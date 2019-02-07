/**
 * hdr_writer_reader_phaser.h
 * Written by Michael Barker and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef HDR_WRITER_READER_PHASER_H
#define HDR_WRITER_READER_PHASER_H 1

#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#include "hdr_thread.h"

HDR_ALIGN_PREFIX(8) 
typedef struct hdr_writer_reader_phaser
{
    int64_t start_epoch;
    int64_t even_end_epoch;
    int64_t odd_end_epoch;
    hdr_mutex_t* reader_mutex;
} hdr_writer_reader_phaser_t;
HDR_ALIGN_SUFFIX(8);

#ifdef __cplusplus
extern "C" {
#endif

    int hdr_writer_reader_phaser_init(hdr_writer_reader_phaser_t* p);

    void hdr_writer_reader_phaser_destroy(hdr_writer_reader_phaser_t* p);

    int64_t hdr_phaser_writer_enter(hdr_writer_reader_phaser_t* p);

    void hdr_phaser_writer_exit(
    hdr_writer_reader_phaser_t* p, int64_t critical_value_at_enter);

    void hdr_phaser_reader_lock(hdr_writer_reader_phaser_t* p);

    void hdr_phaser_reader_unlock(hdr_writer_reader_phaser_t* p);

    void hdr_phaser_flip_phase(
    hdr_writer_reader_phaser_t* p, int64_t sleep_time_ns);

#ifdef __cplusplus
}
#endif

#endif
