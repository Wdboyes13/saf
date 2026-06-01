#define MINIAUDIO_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <zstd.h>
#include "saf.h"
#include "vendor/miniaudio.h"

typedef struct {
    void* data;
    uint8_t bits;
    uint8_t type;
    size_t cframe;
    size_t tframes;
} stream_data;

#define FILL_DATA                                                                                              \
    for (ma_uint32 i = 0; i < ftcpy * pDevice->playback.channels; ++i) {                                       \
        out[i] = in[sd->cframe * pDevice->playback.channels + i];                                              \
    }                                                                                                          \
    for (ma_uint32 i = ftcpy * pDevice->playback.channels; i < frameCount * pDevice->playback.channels; ++i) { \
        out[i] = 0;                                                                                            \
    }

#define SAMPLE_DT(TYPE)             \
    {                               \
        TYPE* out = (TYPE*)pOutput; \
        TYPE* in = (TYPE*)sd->data; \
        FILL_DATA                   \
    }

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    stream_data* sd = (stream_data*)pDevice->pUserData;
    if (sd == NULL) {
        return;
    }

    ma_uint32 ftcpy = frameCount;
    if (sd->cframe + frameCount > sd->tframes) {
        ftcpy = (ma_uint32)(sd->tframes - sd->cframe);
    }

    switch (pDevice->playback.format) {
        case ma_format_f32:
            SAMPLE_DT(float)
            break;
        case ma_format_u8:
            SAMPLE_DT(uint8_t)
            break;
        case ma_format_s16:
            SAMPLE_DT(uint16_t)
            break;
        case ma_format_s32:
            SAMPLE_DT(uint32_t)
            break;
        default:
            break;
    }

    sd->cframe += ftcpy;
}

int main(int ac, char** av) {
    if (ac < 2) {
        fprintf(stderr, "no file specified\n");
        return 1;
    }

    FILE* f = fopen(av[1], "rb");
    if (!f) {
        perror("");
        return 1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        perror("");
        fclose(f);
        return 1;
    }

    long fsz = ftell(f);
    rewind(f);

    uint8_t* data = malloc(fsz);
    if (!data) {
        perror("");
        fclose(f);
        return 1;
    }

    fread(data, fsz, 1, f);
    fclose(f);

    saf_header_t* hdr = (saf_header_t*)data;

    if (hdr->mag != SAF_MAG) {
        fprintf(stderr, "bad magic\n");
        free(data);
        return 1;
    }

    void* sample_data;
    bool spd_allocd;
    if (hdr->cmpr) {
        sample_data = malloc(hdr->size);
        if (!sample_data) {
            perror("");
            free(data);
            return 1;
        }

        size_t sz = ZSTD_decompress(
            sample_data,
            hdr->size,
            &data[sizeof(*hdr)],
            fsz - sizeof(*hdr));

        if (ZSTD_isError(sz)) {
            fprintf(stderr, "decompression error: %s (%d)\n", ZSTD_getErrorString(sz), ZSTD_getErrorCode(sz));
            fprintf(stderr, "Destination capacity: %u\nEstimated compressed size: %lu\n", hdr->size, fsz - sizeof(*hdr));
            exit(1);
        }
    } else {
        sample_data = &data[sizeof(*hdr)];
    }

    stream_data stream;
    stream.bits = hdr->bits;
    stream.data = sample_data;
    stream.cframe = 0;
    stream.type = hdr->sample_type;
    stream.tframes = hdr->nsamples;

    ma_device_config devcfg = ma_device_config_init(ma_device_type_playback);

    devcfg.playback.channels = hdr->channels;
    devcfg.sampleRate = hdr->sample_rate;
    devcfg.pUserData = &stream;
    devcfg.dataCallback = data_callback;

    if (stream.type == SAMPLE_TYPE_FLOAT && stream.bits == 32) {
        devcfg.playback.format = ma_format_f32;
    } else if (stream.type == SAMPLE_TYPE_SINT && stream.bits == 32) {
        devcfg.playback.format = ma_format_s32;
    } else if (stream.type == SAMPLE_TYPE_SINT && stream.bits == 16) {
        devcfg.playback.format = ma_format_s16;
    } else if (stream.type == SAMPLE_TYPE_UINT && stream.bits == 8) {
        devcfg.playback.format = ma_format_u8;
    } else {
        fprintf(stderr, "unknown sample format\n");
        if (spd_allocd) {
            free(sample_data);
        }
        free(data);
    }

    ma_device dev;
    if (ma_device_init(NULL, &devcfg, &dev) != MA_SUCCESS) {
        fprintf(stderr, "failed to initialize audio device\n");
        if (spd_allocd) {
            free(sample_data);
        }
        free(data);
        return 1;
    }

    ma_device_start(&dev);

    printf("press enter to stop streaming...\n");
    getchar();

    ma_device_uninit(&dev);
    if (spd_allocd) {
        free(sample_data);
    }
    free(data);
    return 0;
}