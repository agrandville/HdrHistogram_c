/**
 * hdr_interval_recorder.h
 * Written by Michael Barker and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef HDR_INTERVAL_RECORDER_H
#define HDR_INTERVAL_RECORDER_H 1

#include "hdr_writer_reader_phaser.h"
#include "hdr_histogram.h"

HDR_ALIGN_PREFIX(8)
typedef struct hdr_interval_recorder
{
    hdr_histogram_t* active;
    hdr_histogram_t* inactive;
    hdr_writer_reader_phaser_t phaser;
} hdr_interval_recorder_t;
HDR_ALIGN_SUFFIX(8);

#ifdef __cplusplus
extern "C" {
#endif

int hdr_interval_recorder_init(hdr_interval_recorder_t* r);

int hdr_interval_recorder_init_all(
    hdr_interval_recorder_t* r,
    int64_t lowest_trackable_value,
    int64_t highest_trackable_value,
    int significant_figures);

void hdr_interval_recorder_destroy(hdr_interval_recorder_t* r);

int64_t hdr_interval_recorder_record_value(
    hdr_interval_recorder_t* r,
    int64_t value
);

int64_t hdr_interval_recorder_record_values(
    hdr_interval_recorder_t* r,
    int64_t value,
    int64_t count
);

int64_t hdr_interval_recorder_record_corrected_value(
    hdr_interval_recorder_t* r,
    int64_t value,
    int64_t expected_interval
);

int64_t hdr_interval_recorder_record_corrected_values(
    hdr_interval_recorder_t* r,
    int64_t value,
    int64_t count,
    int64_t expected_interval
);

hdr_histogram_t* hdr_interval_recorder_sample_and_recycle(
    hdr_interval_recorder_t* r,
    hdr_histogram_t* inactive_histogram);

hdr_histogram_t* hdr_interval_recorder_sample(hdr_interval_recorder_t* r);

#ifdef __cplusplus
}
#endif

#endif
