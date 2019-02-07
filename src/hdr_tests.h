#ifndef HDR_TESTS_H
#define HDR_TESTS_H

/* These are functions used in tests and are not intended for normal usage. */

#include "hdr_histogram.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t counts_index_for(const hdr_histogram_t* h, int64_t value);
int hdr_encode_compressed(hdr_histogram_t* h, uint8_t** compressed_histogram, size_t* compressed_len);
int hdr_decode_compressed(uint8_t* buffer, size_t length, hdr_histogram_t** histogram);
void hdr_base64_decode_block(const char* input, uint8_t* output);
void hdr_base64_encode_block(const uint8_t* input, char* output);

#ifdef __cplusplus
}
#endif

#endif
