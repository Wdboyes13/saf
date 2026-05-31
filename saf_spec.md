# SAF File Format
Version 2.0.0  
by Wdboyes13  

## Abstract

Simple Audio Format (SAF) is a file format which contains a short header containing  
data pertaining to the encoding, format, and size of the contained data. The contained  
data is in an interleaved PCM format, with further information specified  
within the header. This format is intended to provied a simple  
way to parse, create, and share audio data.

## Copyright

Copyright (c) 2026 Wdboyes13.  
This document is licensed under the MIT License, see last section for more details.  

## Introduction

This specification documents the SAF Audio File Format.  
SAF is an acronym meaning "Simple Audio Format", a format developed to make  
audio file processing simple. The SAF format stores lossless interleaved  
PCM audio data, either compressed or uncompression. The header  
contains data which dictates the format of the data following it.  

## Format

A SAF file MUST be structured as a HEADER followed by DATA.  
All data SHOULD stored in little-endian format.  

## Header

The SAF header is defined by the C structure:

```c
#define SAF_MAG 0x53414600 // SAF\0
#define SAF_VERSION_PACK(MAG, MIN, PATCH) (((major) << 22) | ((minor) << 12) | (patch))
#define SAF_VERSION_UNPACK(VER, PMAG, PMIN, PPATCH) \
    {                                               \
        *PMAG = (VER >> 22) & 0x3FF;                \
        *PMIN = (VER >> 12) & 0x3FF;                \
        *PPATCH = (VER) & 0xFFF                     \
    }

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
 uint32_t size;
 uint32_t nsamples;
} saf_header_t;
```

A SAF file MUST begin with this header and all fields within.

### Header Fields

The `mag` field is "magic bytes" used to identify this as a SAF file.  
It contains the NULL-terminated ASCII string "SAF" or simple the  
hexadecimal number 0x53414600. This is the same as SAF_MAG.  

The `version` field is a breaking changed introduced in this version.  
It contains the current SAF version encoded using the SAF_VERSION_PACK macro  
and decoded using the SAF_VERSION_UNPACK macro.  

The `sample_rate` field contains the PCM audio data "sample rate" or the  
number of samples per second in units of Hertz (Hz).  

The `channels` field contains the number of audio channels in the data.  

The `bits` field contains the number of bits each PCM sample is.  

The `sample_type` field is a breaking changge introduced in this version.  
This contains the type of samples that are contained. The values are the following:  
- SAMPLE_TYPE_SINT for signed integers.
- SAMPLE_TYPE_UINT for unsigned integers.
- SAMPLE_TYPE_FLOAT for floating point numbers.  

The `cmpr` field is set to either 0 (UNCOMPRESSED) or 1 (COMPRESSED)  
to indicate if the data is compressed (see section 4).  

The `size` field indicates the total size of the PCM data.  

The `nsamples` filed indicates the number of samples contained in the data section of the file.  

## Data Section

The data section of a SAF file contains `bits` wide interleaved PCM audio data.  
If the `cmpr` field of the header is set, this data is compressed using the Zstandard format.  

## License

MIT License

Copyright (c) 2026 Wdboyes13

Permission is hereby granted, free of charge, to any person obtaining a copy of this specification
and associated source code files (the "Specification"), to deal in the Specification without  
restriction, including without limitation the rights to use, copy, modify, merge, publish,  
distribute, sublicense, and/or sell copies of the Specification, and to permit persons to whom  
the Specification is furnished to do so, subject to the following conditions:  
  
The above copyright notice and this permission notice (including the next paragraph) shall be  
included in all copies or substantial portions of the Specification.  

THE SPECIFICATION IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND  
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   
OUT OF OR IN CONNECTION WITH THE SPECIFICATION OR THE USE OR OTHER DEALINGS IN THE SPECIFICATION'.  