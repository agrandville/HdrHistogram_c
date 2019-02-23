/**
 * hiccup.c
 * Written by Michael Barker and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#include <sys/types.h>
#include "getopt.h"
#include "time.h"
#pragma warning(disable:4996)
#else
#include <pthread.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <unistd.h>
#endif
 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <hdr_histogram.h>
#include <hdr_histogram_log.h>
#include <hdr_interval_recorder.h>
#include <hdr_time.h>

static int64_t diff(hdr_timespec_t* t0, hdr_timespec_t* t1)
{
    int64_t delta_us = 0;
    delta_us = (t1->tv_sec - t0->tv_sec) * 1000000;
    delta_us += (t1->tv_nsec - t0->tv_nsec) / 1000;

    return delta_us;
}

#ifdef _WINDOWS_
DWORD WINAPI record_hiccups(LPVOID thread_context)
#else
static void* record_hiccups(void* thread_context)
#endif
{

        hdr_timespec_t t0;
        hdr_timespec_t t1;
        struct hdr_interval_recorder* r;

#ifdef _WINDOWS_
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;
#else
	struct itimerspec timeout;
	struct pollfd fd;
	memset(&fd, 0, sizeof(struct pollfd));
#endif
	
	r = thread_context;

    
    memset(&t0, 0, sizeof(hdr_timespec_t));
    memset(&t1, 0, sizeof(hdr_timespec_t));

#ifdef _WINDOWS_
	liDueTime.QuadPart = -1000000LL;
	hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
#else
	memset(&timeout, 0, sizeof(hdr_timespec_t));
	fd.fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    fd.events = POLLIN|POLLPRI|POLLRDHUP;
    fd.revents = 0;
	timeout.it_value.tv_sec = 0;
	timeout.it_value.tv_nsec = 1000000;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#endif

    while (true)
    {
        int64_t delta_us;

#ifdef _WINDOWS_
		SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);

		hdr_gettime(&t0);
		WaitForSingleObject(hTimer, INFINITE);
		hdr_gettime(&t1);
#else
        timerfd_settime(fd.fd, 0, &timeout, NULL);

        hdr_gettime(&t0);
        poll(&fd, 1, -1);
        hdr_gettime(&t1);
#endif

        delta_us = diff(&t0, &t1) - 1000;
        delta_us = delta_us < 0 ? 0 : delta_us;

        hdr_interval_recorder_record_value(r, delta_us);
    }

#ifdef _WINDOWS_
	return 0;
#else
#pragma clang diagnostic pop
	pthread_exit(NULL);
#endif
}

typedef struct config
{
    int interval;
    const char* filename;
} config_t;

const char* USAGE =
"hiccup [-i <interval>] [-f <filename>]\n"
"  interval: <number> Time in seconds between samples (default 1).\n"
"  filename: <string> Name of the file to log to (default stdout).\n";

static int handle_opts(int argc, char** argv, config_t* config)
{
    int c;
    int interval = 1;

    while ((c = getopt(argc, argv, "i:f:")) != -1)
    {
        switch (c)
        {
        case 'h':
            return 0;

        case 'i':
            interval = atoi(optarg);
            if (interval < 1)
            {
                return 0;
            }

            break;
        case 'f':
            config->filename = optarg;
            break;
        default:
            return 0;
        }
    }

    config->interval = interval < 1 ? 1 : interval;
    return 1;
}

int main(int argc, char** argv)
{
    hdr_timespec_t timestamp;
    hdr_timespec_t start_timestamp;
    hdr_timespec_t end_timestamp;
    struct hdr_interval_recorder recorder;
    hdr_log_writer_t log_writer;
    config_t config;
    hdr_histogram_t* inactive = NULL;
#ifdef _WINDOWS_
	HANDLE recording_thread;
#else
	pthread_t recording_thread;
#endif
	
	FILE* output = stdout;

    memset(&config, 0, sizeof(config_t));
    if (!handle_opts(argc, argv, &config))
    {
        printf("%s", USAGE);
        return 0;
    }

    if (config.filename)
    {
        output = fopen(config.filename, "a+");
        if (!output)
        {
            fprintf(
                stderr, "Failed to open/create file: %s, %s", 
                config.filename, strerror(errno));

            return -1;
        }
    }

    if (0 != hdr_interval_recorder_init_all(&recorder, 1, INT64_C(24) * 60 * 60 * 1000000, 3))
    {
        fprintf(stderr, "%s\n", "Failed to init phaser");
        return -1;
    }

#ifdef _WINDOWS_
	recording_thread = CreateThread(
		NULL,			// default security attributes
		0,				// use default stack size  
		record_hiccups,	// thread function name
		&recorder,		// argument to thread function 
		0,				// use default creation flags 
		NULL);			// returns the thread identifier 

#else
	if (pthread_create(&recording_thread, NULL, record_hiccups, &recorder))
    {
        fprintf(stderr, "%s\n", "Failed to create thread");
        return -1;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#endif
    hdr_gettime(&start_timestamp);
    hdr_getnow(&timestamp);
    hdr_log_writer_init(&log_writer);
    hdr_log_write_header(&log_writer, output, "foobar", &timestamp);

    while (true)
    {        
#ifdef _WINDOWS_
		Sleep(config.interval*1000);
#else
		sleep(config.interval);
#endif
        inactive = hdr_interval_recorder_sample(&recorder);

        hdr_gettime(&end_timestamp);
        timestamp = start_timestamp;

        hdr_gettime(&start_timestamp);

        hdr_log_write(&log_writer, output, &timestamp, &end_timestamp, inactive);
        fflush(output);

        hdr_reset(inactive);
    }

#ifdef _WINDOWS_
	return 0;
#else
#pragma clang diagnostic pop
	pthread_exit(NULL);
#endif
}
