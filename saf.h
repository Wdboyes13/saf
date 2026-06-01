#ifndef SAF_H
#define SAF_H

#include <stdint.h>

#define SAF_MAG 0x53414600 // SAF\0
#define SAF_VERSION_PACK(MAJ, MIN, PATCH) (((MAJ) << 22) | ((MIN) << 12) | (PATCH))
#define SAF_VERSION_UNPACK(VER, PMAG, PMIN, PPATCH) \
    {                                               \
        *PMAG = (VER >> 22) & 0x3FF;                \
        *PMIN = (VER >> 12) & 0x3FF;                \
        *PPATCH = (VER) & 0xFFF                     \
    }

#define SAF_VERSION SAF_VERSION_PACK(2, 0, 0)

#define SAMPLE_TYPE_SINT 0
#define SAMPLE_TYPE_UINT 1
#define SAMPLE_TYPE_FLOAT 2

typedef struct {
    uint32_t mag;
    uint32_t version;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bits;
    uint8_t sample_type;
    uint8_t cmpr;
    uint64_t size;
    uint64_t nsamples;
} __attribute__((packed)) saf_header_t;

#endif