#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION

#include "vendor/dr_libs/dr_flac.h"
#include "vendor/dr_libs/dr_mp3.h"
#include "vendor/dr_libs/dr_wav.h"

#include "saf.h"

#include <zstd.h>

#include <stdio.h>

// always outputs saf f32 compressed
// todo later: make the output format configurable

uint8_t* readfdata(const char* path, size_t* len) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    *len = ftell(f);
    rewind(f);

    uint8_t* data = malloc(*len);
    if (!data) {
        fclose(f);
        return NULL;
    }

    fread(data, *len, 1, f);
    fclose(f);

    return data;
}

int main(int ac, char** av) {
    if (ac < 2) {
        fprintf(stderr, "no file specified\n");
        return 1;
    }

    saf_header_t hdr;
    hdr.mag = SAF_MAG;
    hdr.version = SAF_VERSION;
    hdr.cmpr = 1;
    hdr.bits = 32;
    hdr.sample_type = SAMPLE_TYPE_FLOAT;

    size_t fsz;
    uint8_t* fdata = readfdata(av[1], &fsz);

    drwav wav;
    drmp3 mp3;
    drflac* flac;

    float* samples;
    size_t sz_samples;

    if (drwav_init_memory(&wav, fdata, fsz, NULL)) {
        hdr.sample_rate = wav.sampleRate;
        hdr.channels = wav.channels;
        hdr.nsamples = wav.totalPCMFrameCount;
        samples = malloc(sizeof(float) * hdr.nsamples);
        if (!samples) {
            perror("");
            free(fdata);
            return 1;
        }
        drwav_read_pcm_frames_f32(&wav, hdr.nsamples, samples);
    } else if (drmp3_init_memory(&mp3, fdata, fsz, NULL)) {
        hdr.sample_rate = mp3.sampleRate;
        hdr.channels = mp3.channels;
        hdr.nsamples = mp3.totalPCMFrameCount;
        samples = malloc(sizeof(float) * hdr.nsamples);
        if (!samples) {
            perror("");
            free(fdata);
            return 1;
        }
        drmp3_read_pcm_frames_f32(&mp3, hdr.nsamples, samples);
    } else if ((flac = drflac_open_memory(fdata, fsz, NULL)) != NULL) {
        hdr.sample_rate = flac->sampleRate;
        hdr.channels = flac->channels;
        hdr.nsamples = flac->totalPCMFrameCount;
        samples = malloc(sizeof(float) * hdr.nsamples);
        if (!samples) {
            perror("");
            free(fdata);
            return 1;
        }
        drflac_read_pcm_frames_f32(flac, hdr.nsamples, samples);
    } else {
        fprintf(stderr, "unknown file type\n");
        free(fdata);
        return 1;
    }
    sz_samples = sizeof(float) * hdr.nsamples;
    hdr.size = sz_samples;
    fprintf(stderr, "nsamples: %lu\n", sz_samples);
    free(fdata);

    size_t cbnd = ZSTD_compressBound(sz_samples);
    uint8_t* data = malloc(cbnd);
    if (!data) {
        perror("");
        free(samples);
        return 1;
    }

    size_t res = ZSTD_compress(data, cbnd, samples, sz_samples, ZSTD_defaultCLevel());
    if (ZSTD_isError(res)) {
        fprintf(stderr, "compression error: %s\n", ZSTD_getErrorString(res));
        free(samples);
        return 1;
    }
    free(samples);

    char opath[PATH_MAX + 1];
    snprintf(opath, PATH_MAX + 1, "%s.saf", av[1]);

    FILE* f = fopen(opath, "wb");
    if (!f) {
        perror("");
        free(data);
        return 1;
    }

    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(data, res, 1, f);

    fflush(f);
    fclose(f);
    free(data);

    return 0;
}
