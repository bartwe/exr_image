/* exr_image.h - single-header OpenEXR image helpers

   SPDX-License-Identifier: BSD-3-Clause

   Written by Codex, inspired by TinyEXR and stb_image.h.

   Copyright (c) 2026, Bart van der Werf.
   Portions of the codec implementations are based on OpenEXR/TinyEXR algorithms:
   Copyright (c) 2014 - 2021, Syoyo Fujita and many contributors.
   Copyright (c) 2002, Industrial Light & Magic, a division of Lucas Digital Ltd. LLC.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from this
      software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

   This file follows a single-header implementation pattern:

      #define EXR_IMAGE_IMPLEMENTATION
      #include "exr_image.h"

   before including it in one C or C++ translation unit.
*/

#ifndef EXR_IMAGE_H
#define EXR_IMAGE_H

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef EXRI_NO_STDIO
#include <stdio.h>
#endif
#include <stddef.h>

#define EXRI_VERSION 12

enum
{
   EXRI_default    = 0,
   EXRI_grey       = 1,
   EXRI_grey_alpha = 2,
   EXRI_rgb        = 3,
   EXRI_rgb_alpha  = 4
};

enum
{
   EXRI_LOAD_DEFAULT      = 0,
   EXRI_LOAD_SCRGB_STRICT = 1,
   EXRI_LOAD_SCRGB_ASSUME = 2
};

enum
{
   EXRI_VERSION_FLAG_TILED      = 0x00000200,
   EXRI_VERSION_FLAG_LONG_NAMES = 0x00000400,
   EXRI_VERSION_FLAG_NON_IMAGE  = 0x00000800,
   EXRI_VERSION_FLAG_MULTIPART  = 0x00001000
};

enum
{
   EXRI_WRITE_COMPRESSION_NONE = 0,
   EXRI_WRITE_COMPRESSION_RLE  = 1,
   EXRI_WRITE_COMPRESSION_ZIPS = 2,
   EXRI_WRITE_COMPRESSION_ZIP  = 3,
   EXRI_WRITE_COMPRESSION_PIZ  = 4,
   EXRI_WRITE_COMPRESSION_PXR24 = 5,
   EXRI_WRITE_COMPRESSION_B44  = 6,
   EXRI_WRITE_COMPRESSION_B44A = 7
};

enum
{
   EXRI_WRITE_PIXEL_FLOAT = 0,
   EXRI_WRITE_PIXEL_HALF  = 1,
   EXRI_WRITE_PIXEL_UINT  = 2
};

enum
{
   EXRI_WRITE_LEVEL_ONE    = 0,
   EXRI_WRITE_LEVEL_MIPMAP = 1,
   EXRI_WRITE_LEVEL_RIPMAP = 2
};

enum
{
   EXRI_WRITE_ROUND_DOWN = 0,
   EXRI_WRITE_ROUND_UP   = 1
};

enum
{
   EXRI_SPECTRUM_REFLECTIVE = 0,
   EXRI_SPECTRUM_EMISSIVE   = 1,
   EXRI_SPECTRUM_POLARISED  = 2
};

enum
{
   EXRI_PIXEL_UINT  = 0,
   EXRI_PIXEL_HALF  = 1,
   EXRI_PIXEL_FLOAT = 2
};

typedef unsigned char  exri_uc;

#ifndef EXRI_CALL
#if defined(_WIN32) && defined(EXRI_STDCALL)
#define EXRI_CALL __stdcall
#else
#define EXRI_CALL
#endif
#endif

#ifndef EXRI_CALLBACK
#if defined(_WIN32) && defined(EXRI_STDCALL)
#define EXRI_CALLBACK __stdcall
#else
#define EXRI_CALLBACK
#endif
#endif

#ifndef EXRIDEF
#ifdef EXR_IMAGE_STATIC
#define EXRIDEF static
#elif defined(_WIN32) && defined(EXRI_DLL_EXPORT)
#define EXRIDEF __declspec(dllexport)
#elif defined(_WIN32) && defined(EXRI_DLL_IMPORT)
#define EXRIDEF __declspec(dllimport)
#elif defined(__GNUC__) && defined(EXRI_DLL_EXPORT)
#define EXRIDEF __attribute__((visibility("default")))
#else
#define EXRIDEF extern
#endif
#endif

#ifndef EXRI__PRIVATEDEF
#if defined(__GNUC__) || defined(__clang__)
#define EXRI__PRIVATEDEF static __attribute__((unused))
#else
#define EXRI__PRIVATEDEF static
#endif
#endif

#ifndef EXRI__NOINLINE
#if defined(__GNUC__) && !defined(__clang__)
#define EXRI__NOINLINE __attribute__((noinline))
#else
#define EXRI__NOINLINE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   int  (EXRI_CALLBACK *read) (void *user, void *data, size_t size, size_t *bytes_read);
   int  (EXRI_CALLBACK *skip) (void *user, size_t n);
   int  (EXRI_CALLBACK *eof)  (void *user);
} exri_io_callbacks;

typedef struct
{
   int (EXRI_CALLBACK *write) (void *user, void const *data, size_t size);
} exri_write_callbacks;

typedef struct
{
   char const *name;
   int pixel_type;
} exri_write_channel;

typedef struct
{
   char const *name;
   char const *type;
   exri_uc const *value;
   int value_size;
} exri_write_attribute;

typedef struct
{
   int compression;
   int pixel_type;
   int tiled;
   int tile_size;
   int level_mode;
   int level_rounding;
} exri_write_options;

typedef struct
{
   char const *name;
   int width;
   int height;
   int num_channels;
   float const *data;
   exri_write_channel const *channels;
   exri_write_attribute const *attributes;
   int num_attributes;
   exri_write_options options;
} exri_write_part;

EXRIDEF int EXRI_CALL exri_is_exr_from_memory(exri_uc const *buffer, size_t len);
EXRIDEF int EXRI_CALL exri_is_exr_from_callbacks(exri_io_callbacks const *clbk, void *user);
EXRIDEF int EXRI_CALL exri_version_from_memory(exri_uc const *buffer, size_t len, int *version, int *flags);
EXRIDEF int EXRI_CALL exri_version_from_callbacks(exri_io_callbacks const *clbk, void *user, int *version, int *flags);
EXRIDEF int EXRI_CALL exri_info_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file);
EXRIDEF int EXRI_CALL exri_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file);

EXRIDEF int EXRI_CALL exri_part_count_from_memory(exri_uc const *buffer, size_t len, int *num_parts);
EXRIDEF int EXRI_CALL exri_part_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int *num_parts);
EXRIDEF int EXRI_CALL exri_part_info_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file);
EXRIDEF int EXRI_CALL exri_part_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *channels_in_file);
EXRIDEF int EXRI_CALL exri_part_channel_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_part_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_part_channel_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_part_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_part_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_part_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_part_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_part_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_part_attribute_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_attributes);
EXRIDEF int EXRI_CALL exri_part_attribute_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_part_attribute_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, char *type, int type_size);
EXRIDEF int EXRI_CALL exri_part_attribute_value_size_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, int *value_size);
EXRIDEF int EXRI_CALL exri_part_attribute_value_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, exri_uc *value, int value_size, int *bytes_written);
EXRIDEF int EXRI_CALL exri_part_tiled_level_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_x_levels, int *num_y_levels);
EXRIDEF int EXRI_CALL exri_part_tiled_level_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_x_levels, int *num_y_levels);

EXRIDEF int EXRI_CALL exri_layer_count_from_memory(exri_uc const *buffer, size_t len, int *num_layers);
EXRIDEF int EXRI_CALL exri_layer_name_from_memory(exri_uc const *buffer, size_t len, int layer_index, char *name, int name_size);

EXRIDEF int EXRI_CALL exri_deep_part_info_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels, int *total_samples);
EXRIDEF int EXRI_CALL exri_deep_part_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *num_channels, int *total_samples);
EXRIDEF int EXRI_CALL exri_deep_part_channel_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_deep_part_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_deep_part_channel_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_deep_part_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_load_deep_part_from_memory(float **out_samples, int **out_sample_offsets, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels, int *total_samples);
EXRIDEF int EXRI_CALL exri_load_deep_part_from_callbacks(float **out_samples, int **out_sample_offsets, exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *num_channels, int *total_samples);

EXRIDEF int EXRI_CALL exri_loadf_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_part_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_layer_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_layer_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_channels_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_part_channels_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_channels_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_tiled_level_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_part_tiled_level_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_tiled_level_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_from_callbacks(float **out_pixels, exri_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_part_from_callbacks(float **out_pixels, exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);

EXRIDEF int EXRI_CALL exri_writef_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int comp, float const *data, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int comp, float const *data, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_channels_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_channels_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_multipart_to_memory(exri_uc **out_data, size_t *out_len, exri_write_part const *parts, int num_parts);
EXRIDEF int EXRI_CALL exri_writef_multipart_to_callbacks(exri_write_callbacks const *clbk, void *user, exri_write_part const *parts, int num_parts);

EXRIDEF int EXRI_CALL exri_format_wavelength(char *buffer, int buffer_size, float wavelength_nm);
EXRIDEF int EXRI_CALL exri_make_emissive_spectral_channel_name(char *buffer, int buffer_size, float wavelength_nm, int stokes_component);
EXRIDEF int EXRI_CALL exri_make_reflective_spectral_channel_name(char *buffer, int buffer_size, float wavelength_nm);
EXRIDEF float EXRI_CALL exri_parse_spectral_wavelength(char const *channel_name);
EXRIDEF int EXRI_CALL exri_spectral_stokes_component(char const *channel_name);
EXRIDEF int EXRI_CALL exri_is_spectral_channel_name(char const *channel_name);
EXRIDEF int EXRI_CALL exri_is_spectral_from_memory(exri_uc const *buffer, size_t len);
EXRIDEF int EXRI_CALL exri_is_spectral_from_callbacks(exri_io_callbacks const *clbk, void *user);
EXRIDEF int EXRI_CALL exri_spectrum_type_from_memory(exri_uc const *buffer, size_t len, int *spectrum_type);
EXRIDEF int EXRI_CALL exri_spectrum_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int *spectrum_type);
EXRIDEF int EXRI_CALL exri_spectral_wavelengths_from_memory(exri_uc const *buffer, size_t len, float *wavelengths, int max_wavelengths, int *num_wavelengths);
EXRIDEF int EXRI_CALL exri_spectral_wavelengths_from_callbacks(exri_io_callbacks const *clbk, void *user, float *wavelengths, int max_wavelengths, int *num_wavelengths);
EXRIDEF int EXRI_CALL exri_spectral_units_from_memory(exri_uc const *buffer, size_t len, char *units, int units_size);
EXRIDEF int EXRI_CALL exri_spectral_units_from_callbacks(exri_io_callbacks const *clbk, void *user, char *units, int units_size);

#ifndef EXRI_NO_STDIO
/* Filename helpers open/read/close the named path for each call. Do not use a
   prior metadata call such as exri_info(filename) as a security decision for a
   later load/write of the same path; the file can be replaced between calls.
   Security-sensitive callers should open/authorize/read once, then use the
   memory or callback APIs on that same object. */
EXRIDEF int EXRI_CALL exri_is_exr(char const *filename);
EXRIDEF int EXRI_CALL exri_version(char const *filename, int *version, int *flags);
EXRIDEF int EXRI_CALL exri_info(char const *filename, int *x, int *y, int *channels_in_file);
EXRIDEF int EXRI_CALL exri_part_count(char const *filename, int *num_parts);
EXRIDEF int EXRI_CALL exri_part_info(char const *filename, int part_index, int *x, int *y, int *channels_in_file);
EXRIDEF int EXRI_CALL exri_part_channel_count(char const *filename, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_part_channel_name(char const *filename, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_part_channel_pixel_type(char const *filename, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_part_channel_sampling(char const *filename, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_part_attribute_count(char const *filename, int part_index, int *num_attributes);
EXRIDEF int EXRI_CALL exri_part_attribute_name(char const *filename, int part_index, int attribute_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_part_attribute_type(char const *filename, int part_index, int attribute_index, char *type, int type_size);
EXRIDEF int EXRI_CALL exri_part_attribute_value_size(char const *filename, int part_index, int attribute_index, int *value_size);
EXRIDEF int EXRI_CALL exri_part_attribute_value(char const *filename, int part_index, int attribute_index, exri_uc *value, int value_size, int *bytes_written);
EXRIDEF int EXRI_CALL exri_part_tiled_level_count(char const *filename, int part_index, int *num_x_levels, int *num_y_levels);
EXRIDEF int EXRI_CALL exri_layer_count(char const *filename, int *num_layers);
EXRIDEF int EXRI_CALL exri_layer_name(char const *filename, int layer_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_deep_part_info(char const *filename, int part_index, int *x, int *y, int *num_channels, int *total_samples);
EXRIDEF int EXRI_CALL exri_deep_part_channel_count(char const *filename, int part_index, int *num_channels);
EXRIDEF int EXRI_CALL exri_deep_part_channel_name(char const *filename, int part_index, int channel_index, char *name, int name_size);
EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type(char const *filename, int part_index, int channel_index, int *pixel_type);
EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling(char const *filename, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear);
EXRIDEF int EXRI_CALL exri_load_deep_part(float **out_samples, int **out_sample_offsets, char const *filename, int part_index, int *x, int *y, int *num_channels, int *total_samples);
EXRIDEF int EXRI_CALL exri_loadf(float **out_pixels, char const *filename, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_part(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_layer(float **out_pixels, char const *filename, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_region(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_layer_region(float **out_pixels, char const *filename, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_channels(float **out_pixels, char const *filename, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_part_channels(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_channels_region(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels);
EXRIDEF int EXRI_CALL exri_loadf_tiled_level(float **out_pixels, char const *filename, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_part_tiled_level(float **out_pixels, char const *filename, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_loadf_tiled_level_region(float **out_pixels, char const *filename, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags);
EXRIDEF int EXRI_CALL exri_writef(char const *filename, int w, int h, int comp, float const *data, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_channels(char const *filename, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options);
EXRIDEF int EXRI_CALL exri_writef_multipart(char const *filename, exri_write_part const *parts, int num_parts);
EXRIDEF int EXRI_CALL exri_is_spectral(char const *filename);
EXRIDEF int EXRI_CALL exri_spectrum_type(char const *filename, int *spectrum_type);
EXRIDEF int EXRI_CALL exri_spectral_wavelengths(char const *filename, float *wavelengths, int max_wavelengths, int *num_wavelengths);
EXRIDEF int EXRI_CALL exri_spectral_units(char const *filename, char *units, int units_size);
#endif

EXRIDEF char const * EXRI_CALL exri_failure_reason(void);
EXRIDEF void EXRI_CALL exri_image_free(void *retval_from_exri_load);

#ifdef __cplusplus
}
#endif

#endif /* EXR_IMAGE_H */

#ifdef EXR_IMAGE_IMPLEMENTATION

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4505)
#endif

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined(EXRI_MALLOC) && defined(EXRI_FREE) && defined(EXRI_REALLOC)
/* ok */
#elif !defined(EXRI_MALLOC) && !defined(EXRI_FREE) && !defined(EXRI_REALLOC)
/* ok */
#else
#error "Must define all or none of EXRI_MALLOC, EXRI_FREE, and EXRI_REALLOC."
#endif

#ifndef EXRI_MALLOC
#define EXRI_MALLOC(sz)          malloc(sz)
#define EXRI_REALLOC(p,newsz)    realloc(p,newsz)
#define EXRI_FREE(p)             free(p)
#endif

#ifndef EXRI_MAX_DIMENSIONS
#define EXRI_MAX_DIMENSIONS (1 << 24)
#endif

#ifndef EXRI_MAX_INPUT_SIZE
#define EXRI_MAX_INPUT_SIZE ((size_t) -1)
#endif

#ifndef EXRI_MAX_OUTPUT_SIZE
#define EXRI_MAX_OUTPUT_SIZE ((size_t) -1)
#endif

#ifndef EXRI_MAX_PIXELS
#define EXRI_MAX_PIXELS (((size_t) -1) / (4u * sizeof(float)))
#endif

#ifndef EXRI_THREAD_LOCAL
#if defined(EXRI_NO_THREAD_LOCALS)
#define EXRI_THREAD_LOCAL
#elif defined(_MSC_VER)
#define EXRI_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#define EXRI_THREAD_LOCAL __thread
#else
#define EXRI_THREAD_LOCAL
#endif
#endif

typedef unsigned short exri__uint16;
typedef unsigned int   exri__uint32;
#if defined(_MSC_VER)
typedef unsigned __int64 exri__uint64;
#elif defined(__GNUC__)
typedef unsigned int exri__uint64 __attribute__((mode(DI)));
#else
typedef unsigned long long exri__uint64;
#endif

typedef unsigned char exri__validate_uint16[sizeof(exri__uint16) == 2 ? 1 : -1];
typedef unsigned char exri__validate_uint32[sizeof(exri__uint32) == 4 ? 1 : -1];
typedef unsigned char exri__validate_uint64[sizeof(exri__uint64) == 8 ? 1 : -1];
typedef unsigned char exri__validate_int_size[sizeof(int) >= 4 ? 1 : -1];
typedef unsigned char exri__validate_float32[sizeof(float) == 4 ? 1 : -1];

typedef struct
{
   exri_uc const *data;
   size_t size;
   size_t pos;
} exri__context;

typedef struct
{
   int width;
   int height;
   int channels;
   int num_channel_records;
   int compression;
   int line_order;
   size_t header_end;
   int chunk_count;
   int tiled;
   int tile_width;
   int tile_height;
   int tile_level_mode;
   int tile_rounding_mode;
   int non_image;
   int multipart;
   int data_window_found;
   int channels_found;
   int compression_found;
   int line_order_found;
   int chunk_count_found;
   int tiles_found;
   int chromaticities_found;
   int min_x;
   int min_y;
   int max_x;
   int max_y;
   float chromaticities[8];
} exri__info;

typedef struct
{
   exri_uc const *name;
   int name_len;
   int layer_len;
   int pixel_type;
   int x_sampling;
   int y_sampling;
   int bytes_per_sample;
   int byte_offset;
   int role;
   int p_linear;
} exri__channel;

typedef struct
{
   exri_uc const *name;
   int name_len;
   exri_uc const *type;
   int type_len;
   exri_uc const *value;
   int value_size;
} exri__attribute_ref;

typedef struct
{
   size_t header_start;
   size_t header_end;
   int chunk_count;
   int chunk_count_found;
   int tiled;
   int non_image;
   int name_found;
   int type_found;
} exri__multipart_part_ref;

enum
{
   EXRI__ROLE_UNKNOWN = 0,
   EXRI__ROLE_Y       = 1,
   EXRI__ROLE_R       = 2,
   EXRI__ROLE_G       = 3,
   EXRI__ROLE_B       = 4,
   EXRI__ROLE_A       = 5
};

enum
{
   EXRI__COLOR_ASSUME_SCRGB = 1
};

enum
{
   EXRI__WRITE_COMPRESSION_MASK = 0x00ff,
   EXRI__WRITE_STORAGE_HALF     = 0x0100,
   EXRI__WRITE_TILED            = 0x0200,
   EXRI__WRITE_STORAGE_UINT     = 0x0400,
   EXRI__WRITE_TILED_MIPMAP     = 0x0800,
   EXRI__WRITE_TILED_RIPMAP     = 0x1000,
   EXRI__WRITE_TILED_ROUND_UP   = 0x2000,
   EXRI__WRITE_TILE_SIZE_SHIFT  = 16,
   EXRI__WRITE_TILE_SIZE_MASK   = 0x0fff0000
};

static EXRI_THREAD_LOCAL char const *exri__failure_reason = "";

static int exri__err(char const *reason)
{
   exri__failure_reason = reason;
   return 0;
}

static int exri__err_invalid(void)
{
   return exri__err("invalid EXR");
}

static int exri__check_input_size(size_t len)
{
   if (EXRI_MAX_INPUT_SIZE > 0 && len > EXRI_MAX_INPUT_SIZE)
      return exri__err("input too large");
   return 1;
}

static int exri__has_bytes_at(int pos, int size, int count)
{
   if (pos < 0 || size < 0 || count < 0 || pos > size)
      return 0;
   return (size_t) count <= (size_t) size - (size_t) pos;
}

static int exri__has_file_bytes_at(size_t pos, size_t size, size_t count)
{
   if (pos > size)
      return 0;
   return count <= size - pos;
}

static int exri__has_offset_table_entry(size_t table_pos, int index, size_t size)
{
   size_t entry_end;

   if (index < 0 || (size_t) index > (((size_t) -1) - 8u) / 8u)
      return 0;
   entry_end = (size_t) index * 8u + 8u;
   return exri__has_file_bytes_at(table_pos, size, entry_end);
}

EXRIDEF char const * EXRI_CALL exri_failure_reason(void)
{
   return exri__failure_reason;
}

EXRIDEF void EXRI_CALL exri_image_free(void *retval_from_exri_load)
{
   EXRI_FREE(retval_from_exri_load);
}

static exri__uint32 exri__get32le_at(exri_uc const *p)
{
   return ((exri__uint32) p[0]) |
          ((exri__uint32) p[1] << 8) |
          ((exri__uint32) p[2] << 16) |
          ((exri__uint32) p[3] << 24);
}

static exri__uint16 exri__get16le_at(exri_uc const *p)
{
   return (exri__uint16) (((exri__uint16) p[0]) |
                          ((exri__uint16) p[1] << 8));
}

static exri__uint32 exri__get32be_at(exri_uc const *p)
{
   return ((exri__uint32) p[0] << 24) |
          ((exri__uint32) p[1] << 16) |
          ((exri__uint32) p[2] << 8) |
          ((exri__uint32) p[3]);
}

static void exri__put32le_at(exri_uc *p, exri__uint32 v)
{
   p[0] = (exri_uc) (v & 255u);
   p[1] = (exri_uc) ((v >> 8) & 255u);
   p[2] = (exri_uc) ((v >> 16) & 255u);
   p[3] = (exri_uc) ((v >> 24) & 255u);
}

static void exri__put64le_size_at(exri_uc *p, size_t v)
{
   exri__uint64 value;

   value = (exri__uint64) v;
   exri__put32le_at(p, (exri__uint32) (value & 0xffffffffu));
   exri__put32le_at(p + 4, (exri__uint32) (value >> 32));
}

static int exri__get64le_as_size_at(exri_uc const *p, size_t *out)
{
   exri__uint32 low;
   exri__uint32 high;
   exri__uint64 value;

   low = exri__get32le_at(p);
   high = exri__get32le_at(p + 4);
   value = ((exri__uint64) high << 32) | (exri__uint64) low;

   if (value > (exri__uint64) ((size_t) -1))
      return 0;

   *out = (size_t) value;
   return 1;
}

static float exri__float_from_bits(exri__uint32 bits)
{
   float f;

   memcpy(&f, &bits, sizeof(f));
   return f;
}

static exri__uint32 exri__float_to_bits(float f)
{
   exri__uint32 bits;

   memcpy(&bits, &f, sizeof(bits));
   return bits;
}

static float exri__getf32le_at(exri_uc const *p)
{
   return exri__float_from_bits(exri__get32le_at(p));
}

static float exri__half_to_float(exri__uint16 h)
{
   exri__uint32 s;
   exri__uint32 e;
   exri__uint32 m;
   exri__uint32 out_e;
   exri__uint32 out_m;
   int shift;

   s = ((exri__uint32) h & 0x8000u) << 16;
   e = ((exri__uint32) h >> 10) & 31u;
   m = (exri__uint32) h & 1023u;

   if (e == 0) {
      if (m == 0)
         return exri__float_from_bits(s);

      shift = 0;
      while ((m & 0x400u) == 0) {
         m <<= 1;
         shift += 1;
      }
      m &= 0x3ffu;
      out_e = (exri__uint32) (127 - 14 - shift);
      out_m = m << 13;
      return exri__float_from_bits(s | (out_e << 23) | out_m);
   }

   if (e == 31u)
      return exri__float_from_bits(s | 0x7f800000u | (m << 13));

   out_e = e + (127u - 15u);
   out_m = m << 13;
   return exri__float_from_bits(s | (out_e << 23) | out_m);
}

static int exri__i32_from_u32(exri__uint32 x)
{
   if (x <= 0x7fffffffu)
      return (int) x;

   if (x == 0x80000000u)
      return INT_MIN;

   return -1 - (int) (0xffffffffu - x);
}

static int exri__dimension_from_window(int min_v, int max_v, int *out)
{
   int span;
   int neg_count;
   int pos_count;
   int lo;
   int hi;

   if (max_v < min_v)
      return 0;

   if (min_v >= 0) {
      span = max_v - min_v;
      if (span >= EXRI_MAX_DIMENSIONS)
         return 0;
      *out = span + 1;
      return 1;
   }

   if (max_v < 0) {
      lo = -(min_v + 1);
      hi = -(max_v + 1);
      span = lo - hi;
      if (span >= EXRI_MAX_DIMENSIONS)
         return 0;
      *out = span + 1;
      return 1;
   }

   if (min_v == INT_MIN)
      return 0;

   neg_count = -min_v;
   if (max_v >= EXRI_MAX_DIMENSIONS)
      return 0;
   pos_count = max_v + 1;
   if (neg_count > EXRI_MAX_DIMENSIONS - pos_count)
      return 0;

   *out = neg_count + pos_count;
   return 1;
}

typedef struct
{
   exri_uc const *data;
   int size;
   int pos;
   unsigned int bits;
   int num_bits;
} exri__bits;

typedef struct
{
   unsigned short code[288 + 32];
   unsigned char size[288 + 32];
   int num;
} exri__huffman;

static unsigned int exri__bit_reverse(unsigned int v, int bits)
{
   unsigned int r;
   int i;

   r = 0;
   for (i = 0; i < bits; ++i) {
      r = (r << 1) | (v & 1u);
      v >>= 1;
   }

   return r;
}

static int exri__bits_read(exri__bits *b, int n, unsigned int *out)
{
   unsigned int result;
   int shift;

   if (n < 0 || n > 24)
      return 0;

   result = 0;
   shift = 0;

   while (n > 0) {
      if (b->num_bits == 0) {
         if (b->pos >= b->size)
            return 0;
         b->bits = b->data[b->pos++];
         b->num_bits = 8;
      }

      result |= (b->bits & 1u) << shift;
      b->bits >>= 1;
      b->num_bits -= 1;
      shift += 1;
      n -= 1;
   }

   *out = result;
   return 1;
}

static void exri__bits_align_byte(exri__bits *b)
{
   b->bits = 0;
   b->num_bits = 0;
}

static int exri__huffman_build(exri__huffman *h, unsigned char const *sizes, int num)
{
   int counts[16];
   int next_code[16];
   int code;
   int i;
   int len;

   if (num > (int) (sizeof(h->code) / sizeof(h->code[0])))
      return 0;

   memset(counts, 0, sizeof(counts));
   memset(next_code, 0, sizeof(next_code));
   memset(h->code, 0, sizeof(h->code));
   memset(h->size, 0, sizeof(h->size));

   h->num = num;

   for (i = 0; i < num; ++i) {
      if (sizes[i] > 15)
         return 0;
      counts[sizes[i]] += 1;
   }
   counts[0] = 0;

   code = 0;
   for (len = 1; len <= 15; ++len) {
      code = (code + counts[len - 1]) << 1;
      if (counts[len] && code + counts[len] > (1 << len))
         return 0;
      next_code[len] = code;
   }

   for (i = 0; i < num; ++i) {
      len = sizes[i];
      if (len) {
         h->code[i] = (unsigned short) exri__bit_reverse((unsigned int) next_code[len], len);
         h->size[i] = (unsigned char) len;
         next_code[len] += 1;
      }
   }

   return 1;
}

static int exri__huffman_decode(exri__bits *b, exri__huffman const *h, int *out_symbol)
{
   unsigned int code;
   unsigned int bit;
   int len;
   int i;

   code = 0;
   for (len = 1; len <= 15; ++len) {
      if (!exri__bits_read(b, 1, &bit))
         return 0;
      code |= bit << (len - 1);

      for (i = 0; i < h->num; ++i) {
         if (h->size[i] == len && h->code[i] == code) {
            *out_symbol = i;
            return 1;
         }
      }
   }

   return 0;
}

static int exri__inflate_uncompressed(exri__bits *bits, exri_uc *dst, int dst_len, int *dst_pos)
{
   int len;
   int nlen;

   exri__bits_align_byte(bits);

   if (!exri__has_bytes_at(bits->pos, bits->size, 4))
      return 0;

   len = bits->data[bits->pos] | (bits->data[bits->pos + 1] << 8);
   nlen = bits->data[bits->pos + 2] | (bits->data[bits->pos + 3] << 8);
   bits->pos += 4;

   if ((len ^ 0xffff) != nlen)
      return 0;
   if (!exri__has_bytes_at(bits->pos, bits->size, len))
      return 0;
   if (!exri__has_bytes_at(*dst_pos, dst_len, len))
      return 0;

   memcpy(dst + *dst_pos, bits->data + bits->pos, (size_t) len);
   bits->pos += len;
   *dst_pos += len;
   return 1;
}

static int exri__inflate_fixed_tables(exri__huffman *litlen, exri__huffman *dist)
{
   unsigned char sizes[288];
   unsigned char dsizes[32];
   int i;

   for (i = 0; i <= 143; ++i)
      sizes[i] = 8;
   for (; i <= 255; ++i)
      sizes[i] = 9;
   for (; i <= 279; ++i)
      sizes[i] = 7;
   for (; i <= 287; ++i)
      sizes[i] = 8;

   for (i = 0; i < 32; ++i)
      dsizes[i] = 5;

   return exri__huffman_build(litlen, sizes, 288) &&
          exri__huffman_build(dist, dsizes, 32);
}

static int exri__inflate_dynamic_tables(exri__bits *bits, exri__huffman *litlen, exri__huffman *dist)
{
   static const unsigned char order[19] =
   {
      16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
   };
   unsigned char code_sizes[19];
   unsigned char sizes[288 + 32];
   exri__huffman code_table;
   unsigned int v;
   int hlit;
   int hdist;
   int hclen;
   int total;
   int i;
   int sym;
   int repeat;
   unsigned char fill;

   memset(code_sizes, 0, sizeof(code_sizes));
   memset(sizes, 0, sizeof(sizes));

   if (!exri__bits_read(bits, 5, &v))
      return 0;
   hlit = (int) v + 257;
   if (!exri__bits_read(bits, 5, &v))
      return 0;
   hdist = (int) v + 1;
   if (!exri__bits_read(bits, 4, &v))
      return 0;
   hclen = (int) v + 4;

   if (hlit > 286 || hdist > 32)
      return 0;

   for (i = 0; i < hclen; ++i) {
      if (!exri__bits_read(bits, 3, &v))
         return 0;
      code_sizes[order[i]] = (unsigned char) v;
   }

   if (!exri__huffman_build(&code_table, code_sizes, 19))
      return 0;

   total = hlit + hdist;
   i = 0;
   while (i < total) {
      if (!exri__huffman_decode(bits, &code_table, &sym))
         return 0;

      if (sym < 16) {
         sizes[i++] = (unsigned char) sym;
      } else {
         if (sym == 16) {
            if (i == 0)
               return 0;
            if (!exri__bits_read(bits, 2, &v))
               return 0;
            repeat = (int) v + 3;
            fill = sizes[i - 1];
         } else if (sym == 17) {
            if (!exri__bits_read(bits, 3, &v))
               return 0;
            repeat = (int) v + 3;
            fill = 0;
         } else if (sym == 18) {
            if (!exri__bits_read(bits, 7, &v))
               return 0;
            repeat = (int) v + 11;
            fill = 0;
         } else {
            return 0;
         }

         if (repeat > total - i)
            return 0;
         while (repeat-- > 0)
            sizes[i++] = fill;
      }
   }

   return exri__huffman_build(litlen, sizes, hlit) &&
          exri__huffman_build(dist, sizes + hlit, hdist);
}

static int exri__inflate_huffman_block(exri__bits *bits, exri__huffman *litlen, exri__huffman *dist, exri_uc *dst, int dst_len, int *dst_pos)
{
   static const int length_base[29] =
   {
      3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
      15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
      67, 83, 99, 115, 131, 163, 195, 227, 258
   };
   static const int length_extra[29] =
   {
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
      4, 4, 4, 4, 5, 5, 5, 5, 0
   };
   static const int dist_base[30] =
   {
      1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
      33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
      1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
   };
   static const int dist_extra[30] =
   {
      0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
      4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
      9, 9, 10, 10, 11, 11, 12, 12, 13, 13
   };
   unsigned int v;
   int sym;
   int len;
   int d;
   int i;

   for (;;) {
      if (!exri__huffman_decode(bits, litlen, &sym))
         return 0;

      if (sym < 256) {
         if (*dst_pos >= dst_len)
            return 0;
         dst[(*dst_pos)++] = (exri_uc) sym;
      } else if (sym == 256) {
         return 1;
      } else {
         sym -= 257;
         if (sym < 0 || sym >= 29)
            return 0;
         len = length_base[sym];
         if (length_extra[sym]) {
            if (!exri__bits_read(bits, length_extra[sym], &v))
               return 0;
            len += (int) v;
         }

         if (!exri__huffman_decode(bits, dist, &sym))
            return 0;
         if (sym < 0 || sym >= 30)
            return 0;
         d = dist_base[sym];
         if (dist_extra[sym]) {
            if (!exri__bits_read(bits, dist_extra[sym], &v))
               return 0;
            d += (int) v;
         }

         if (d <= 0 || d > *dst_pos)
            return 0;
         if (len > dst_len - *dst_pos)
            return 0;

         for (i = 0; i < len; ++i) {
            dst[*dst_pos] = dst[*dst_pos - d];
            *dst_pos += 1;
         }
      }
   }
}

static exri__uint32 exri__adler32(exri_uc const *data, int len)
{
   exri__uint32 s1;
   exri__uint32 s2;
   int i;
   int n;
   int remaining;

   s1 = 1;
   s2 = 0;
   i = 0;
   remaining = len;
   while (remaining > 0) {
      n = remaining;
      if (n > 5552)
         n = 5552;
      remaining -= n;
      while (n-- > 0) {
         s1 += data[i++];
         s2 += s1;
      }
      s1 %= 65521u;
      s2 %= 65521u;
   }

   return (s2 << 16) | s1;
}

static int exri__zlib_decode_buffer(exri_uc *dst, int dst_len, exri_uc const *src, int src_len)
{
   exri__bits bits;
   exri__huffman litlen;
   exri__huffman dist;
   unsigned int final_block;
   unsigned int type;
   int dst_pos;
   int cmf;
   int flg;
   int trailer_pos;
   exri__uint32 expected_adler;

   if (dst == NULL || src == NULL || dst_len < 0 || src_len < 6)
      return -1;

   cmf = src[0];
   flg = src[1];
   if ((cmf & 15) != 8)
      return -1;
   if (((cmf << 8) + flg) % 31 != 0)
      return -1;
   if (flg & 32)
      return -1;

   bits.data = src;
   bits.size = src_len - 4;
   bits.pos = 2;
   bits.bits = 0;
   bits.num_bits = 0;
   dst_pos = 0;

   do {
      if (!exri__bits_read(&bits, 1, &final_block))
         return -1;
      if (!exri__bits_read(&bits, 2, &type))
         return -1;

      if (type == 0) {
         if (!exri__inflate_uncompressed(&bits, dst, dst_len, &dst_pos))
            return -1;
      } else if (type == 1 || type == 2) {
         if (type == 1) {
            if (!exri__inflate_fixed_tables(&litlen, &dist))
               return -1;
         } else {
            if (!exri__inflate_dynamic_tables(&bits, &litlen, &dist))
               return -1;
         }

         if (!exri__inflate_huffman_block(&bits, &litlen, &dist, dst, dst_len, &dst_pos))
            return -1;
      } else {
         return -1;
      }
   } while (!final_block);

   trailer_pos = bits.pos;
   if (trailer_pos != src_len - 4)
      return -1;

   expected_adler = exri__get32be_at(src + trailer_pos);
   if (exri__adler32(dst, dst_pos) != expected_adler)
      return -1;

   return dst_pos;
}

static int exri__require(exri__context *s, size_t n)
{
   if (s->pos > s->size)
      return 0;
   if (n > s->size - s->pos)
      return 0;
   return 1;
}

static int exri__read_u32(exri__context *s, exri__uint32 *out)
{
   if (!exri__require(s, 4))
      return 0;
   *out = exri__get32le_at(s->data + s->pos);
   s->pos += 4;
   return 1;
}

static int exri__read_cstring(exri__context *s, char const **out, int *len)
{
   size_t start;
   size_t string_len;

   if (!exri__require(s, 1))
      return 0;

   start = s->pos;
   while (s->pos < s->size) {
      if (s->data[s->pos] == 0) {
         string_len = s->pos - start;
         if (string_len > (size_t) INT_MAX)
            return 0;
         *out = (char const *) (s->data + start);
         *len = (int) string_len;
         s->pos += 1;
         return 1;
      }
      s->pos += 1;
   }

   return 0;
}

static int exri__channel_component_start(exri_uc const *name, int len)
{
   int start;
   int i;

   start = 0;
   i = len - 1;
   while (i >= 0) {
      if (name[i] == '.') {
         start = i + 1;
         break;
      }
      i -= 1;
   }

   return start;
}

static int exri__channel_layer_len(exri_uc const *name, int len)
{
   int start;

   start = exri__channel_component_start(name, len);
   if (start > 0 && start < len)
      return start - 1;
   return 0;
}

static int exri__channel_component_is(exri_uc const *name, int len, char component)
{
   int start;

   start = exri__channel_component_start(name, len);
   return (len - start == 1) && (name[start] == (exri_uc) component);
}

static int exri__channel_role(exri_uc const *name, int len)
{
   if (exri__channel_component_is(name, len, 'R'))
      return EXRI__ROLE_R;
   if (exri__channel_component_is(name, len, 'G'))
      return EXRI__ROLE_G;
   if (exri__channel_component_is(name, len, 'B'))
      return EXRI__ROLE_B;
   if (exri__channel_component_is(name, len, 'A'))
      return EXRI__ROLE_A;
   if (exri__channel_component_is(name, len, 'Y'))
      return EXRI__ROLE_Y;
   return EXRI__ROLE_UNKNOWN;
}

static int exri__bytes_per_pixel_type(exri__uint32 pixel_type)
{
   if (pixel_type == 0)
      return 4;
   if (pixel_type == 1)
      return 2;
   if (pixel_type == 2)
      return 4;
   return 0;
}

static int exri__grow_channels(exri__channel **channels, int *capacity, int needed)
{
   int new_capacity;
   size_t new_capacity_size;
   size_t needed_size;
   void *p;

   if (needed < 0)
      return 0;
   needed_size = (size_t) needed;
   new_capacity = *capacity;
   if (new_capacity <= 0)
      new_capacity = 8;
   new_capacity_size = (size_t) new_capacity;

   while (new_capacity_size < needed_size) {
      if (new_capacity_size > (size_t) INT_MAX / 2u)
         return 0;
      new_capacity_size *= 2u;
   }
   if (new_capacity_size > (size_t) INT_MAX)
      return 0;
   new_capacity = (int) new_capacity_size;

   if ((size_t) new_capacity > ((size_t) -1) / sizeof(**channels))
      return 0;

   p = EXRI_REALLOC(*channels, (size_t) new_capacity * sizeof(**channels));
   if (p == NULL)
      return 0;

   *channels = (exri__channel *) p;
   *capacity = new_capacity;
   return 1;
}

static int exri__parse_channels(exri_uc const *data, int size, int *channels_out, int *num_channel_records, exri__channel **channel_list)
{
   int pos;
   int count;
   int capacity;
   int has_r;
   int has_g;
   int has_b;
   int has_a;
   int has_y;
   int byte_offset;
   int bytes_per_sample;
   int role;
   int name_start;
   int name_len;
   int p_linear;
   exri__uint32 pixel_type;
   exri__uint32 xs;
   exri__uint32 ys;
   exri__channel *channels;

   pos = 0;
   count = 0;
   capacity = 0;
   has_r = has_g = has_b = has_a = has_y = 0;
   byte_offset = 0;
   channels = NULL;

   for (;;) {
      if (pos >= size)
         goto fail;

      name_start = pos;
      while (pos < size && data[pos] != 0)
         pos += 1;
      if (pos >= size)
         goto fail;

      name_len = pos - name_start;
      pos += 1;

      if (name_len == 0)
         break;

      if (size - pos < 16)
         goto fail;

      pixel_type = exri__get32le_at(data + pos);
      pos += 4;
      p_linear = data[pos] ? 1 : 0;
      pos += 4;
      xs = exri__get32le_at(data + pos);
      pos += 4;
      ys = exri__get32le_at(data + pos);
      pos += 4;

      bytes_per_sample = exri__bytes_per_pixel_type(pixel_type);
      if (bytes_per_sample == 0)
         goto fail;
      if (xs == 0 || ys == 0)
         goto fail;
      if (xs > (exri__uint32) INT_MAX || ys > (exri__uint32) INT_MAX)
         goto fail;

      if (count == INT_MAX)
         goto fail;

      role = exri__channel_role(data + name_start, name_len);
      if (role == EXRI__ROLE_R)
         has_r = 1;
      else if (role == EXRI__ROLE_G)
         has_g = 1;
      else if (role == EXRI__ROLE_B)
         has_b = 1;
      else if (role == EXRI__ROLE_A)
         has_a = 1;
      else if (role == EXRI__ROLE_Y)
         has_y = 1;

      if (channel_list) {
         if (!exri__grow_channels(&channels, &capacity, count + 1))
            goto fail;
         channels[count].pixel_type = (int) pixel_type;
         channels[count].name = data + name_start;
         channels[count].name_len = name_len;
         channels[count].layer_len = exri__channel_layer_len(data + name_start, name_len);
         channels[count].x_sampling = (int) xs;
         channels[count].y_sampling = (int) ys;
         channels[count].bytes_per_sample = bytes_per_sample;
         channels[count].byte_offset = byte_offset;
         channels[count].role = role;
         channels[count].p_linear = p_linear;
      }

      count += 1;

      if (byte_offset > INT_MAX - bytes_per_sample)
         goto fail;
      byte_offset += bytes_per_sample;
   }

   if (count <= 0)
      goto fail;

   if (has_r && has_g && has_b)
      *channels_out = has_a ? 4 : 3;
   else if (has_y)
      *channels_out = has_a ? 2 : 1;
   else
      *channels_out = count;

   if (channel_list)
      *channel_list = channels;
   if (num_channel_records)
      *num_channel_records = count;

   return 1;

fail:
   EXRI_FREE(channels);
   if (channel_list)
      *channel_list = NULL;
   return 0;
}

static int exri__parse_header(exri_uc const *buffer, size_t len, exri__info *info, exri__channel **channels)
{
   exri__context s;
   char const *name;
   char const *type;
   int name_len;
   int type_len;
   size_t value_pos;
   int value_size;
   int i;
   exri__uint32 version_field;
   exri__uint32 attr_size;
   exri__uint32 u0;
   exri__uint32 u1;
   exri__uint32 u2;
   exri__uint32 u3;

   if (buffer == NULL || len < 8)
      return exri__err_invalid();

   if (!exri__check_input_size(len))
      return 0;

   if (buffer[0] != 0x76 || buffer[1] != 0x2f || buffer[2] != 0x31 || buffer[3] != 0x01)
      return exri__err_invalid();

   version_field = exri__get32le_at(buffer + 4);
   if ((version_field & 255u) != 2u)
      return exri__err("unsupported EXR version");

   memset(info, 0, sizeof(*info));
   info->compression = -1;
   info->line_order = 0;
   info->tiled = (version_field & 0x00000200u) ? 1 : 0;
   info->non_image = (version_field & 0x00000800u) ? 1 : 0;
   info->multipart = (version_field & 0x00001000u) ? 1 : 0;
   if (channels)
      *channels = NULL;

   s.data = buffer;
   s.size = len;
   s.pos = 8;

   for (;;) {
      if (!exri__read_cstring(&s, &name, &name_len))
         return exri__err_invalid();

      if (name_len == 0)
         break;

      if (!exri__read_cstring(&s, &type, &type_len))
         return exri__err_invalid();
      (void) type;
      (void) type_len;
      if (!exri__read_u32(&s, &attr_size))
         return exri__err_invalid();
      if (attr_size > (exri__uint32) INT_MAX)
         return exri__err_invalid();

      value_size = (int) attr_size;
      if (!exri__require(&s, (size_t) value_size))
         return exri__err_invalid();

      value_pos = s.pos;

      if (strcmp(name, "dataWindow") == 0) {
         if (strcmp(type, "box2i") != 0 || value_size != 16)
            return exri__err_invalid();
         if (info->data_window_found)
            return exri__err_invalid();
         u0 = exri__get32le_at(buffer + value_pos + 0);
         u1 = exri__get32le_at(buffer + value_pos + 4);
         u2 = exri__get32le_at(buffer + value_pos + 8);
         u3 = exri__get32le_at(buffer + value_pos + 12);

         info->min_x = exri__i32_from_u32(u0);
         info->min_y = exri__i32_from_u32(u1);
         info->max_x = exri__i32_from_u32(u2);
         info->max_y = exri__i32_from_u32(u3);
         info->data_window_found = 1;
      } else if (strcmp(name, "channels") == 0) {
         if (strcmp(type, "chlist") != 0)
            return exri__err_invalid();
         if (info->channels_found)
            return exri__err_invalid();
         if (!exri__parse_channels(buffer + value_pos, value_size, &info->channels, &info->num_channel_records, channels))
            return exri__err_invalid();
         info->channels_found = 1;
      } else if (strcmp(name, "compression") == 0) {
         if (strcmp(type, "compression") != 0 || value_size != 1)
            return exri__err_invalid();
         if (info->compression_found)
            return exri__err_invalid();
         info->compression = buffer[value_pos];
         info->compression_found = 1;
      } else if (strcmp(name, "lineOrder") == 0) {
         if (strcmp(type, "lineOrder") != 0 || value_size != 1)
            return exri__err_invalid();
         if (info->line_order_found)
            return exri__err_invalid();
         info->line_order = buffer[value_pos];
         info->line_order_found = 1;
      } else if (strcmp(name, "tiles") == 0) {
         if (strcmp(type, "tiledesc") != 0 || value_size != 9)
            return exri__err_invalid();
         if (info->tiles_found)
            return exri__err_invalid();
         info->tile_width = exri__i32_from_u32(exri__get32le_at(buffer + value_pos + 0));
         info->tile_height = exri__i32_from_u32(exri__get32le_at(buffer + value_pos + 4));
         info->tile_level_mode = buffer[value_pos + 8] & 3;
         info->tile_rounding_mode = (buffer[value_pos + 8] >> 4) & 1;
         info->tiles_found = 1;
      } else if (strcmp(name, "chunkCount") == 0) {
         if (strcmp(type, "int") != 0 || value_size != 4)
            return exri__err_invalid();
         if (info->chunk_count_found)
            return exri__err_invalid();
         info->chunk_count = exri__i32_from_u32(exri__get32le_at(buffer + value_pos));
         info->chunk_count_found = 1;
      } else if (strcmp(name, "chromaticities") == 0) {
         if (strcmp(type, "chromaticities") != 0 || value_size != 32)
            return exri__err_invalid();
         if (info->chromaticities_found)
            return exri__err_invalid();
         for (i = 0; i < 8; ++i)
            info->chromaticities[i] = exri__getf32le_at(buffer + value_pos + i * 4);
         info->chromaticities_found = 1;
      }

      s.pos = value_pos + (size_t) value_size;
   }

   info->header_end = s.pos;

   if (!info->data_window_found || !info->channels_found || !info->compression_found)
      return exri__err_invalid();

   if (info->max_x < info->min_x || info->max_y < info->min_y)
      return exri__err_invalid();

   if (info->compression < 0 || info->compression > 9)
      return exri__err_invalid();
   if (info->line_order_found && (info->line_order < 0 || info->line_order > 2))
      return exri__err_invalid();
   if (info->chunk_count_found && info->chunk_count < 0)
      return exri__err_invalid();
   if (info->tiled) {
      if (!info->tiles_found)
         return exri__err_invalid();
      if (info->tile_width <= 0 || info->tile_height <= 0)
         return exri__err_invalid();
   } else if (info->tiles_found && !info->multipart && !info->non_image) {
      return exri__err_invalid();
   }

   if (!exri__dimension_from_window(info->min_x, info->max_x, &info->width))
      return exri__err("image too large");
   if (!exri__dimension_from_window(info->min_y, info->max_y, &info->height))
      return exri__err("image too large");

   return 1;
}

EXRIDEF int EXRI_CALL exri_is_exr_from_memory(exri_uc const *buffer, size_t len)
{
   exri__info info;
   return exri__parse_header(buffer, len, &info, NULL);
}

EXRIDEF int EXRI_CALL exri_info_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file)
{
   exri__info info;

   if (!exri__parse_header(buffer, len, &info, NULL))
      return 0;

   if (x)
      *x = info.width;
   if (y)
      *y = info.height;
   if (channels_in_file)
      *channels_in_file = info.channels;

   return 1;
}

static int exri__layer_string_len(char const *layer, int *len_out)
{
   int len;

   if (layer == NULL) {
      *len_out = 0;
      return 1;
   }

   len = 0;
   while (layer[len] != 0) {
      if (len == INT_MAX)
         return exri__err("layer name too long");
      len += 1;
   }

   *len_out = len;
   return 1;
}

static int exri__channel_matches_layer(exri__channel const *channel, char const *layer, int layer_len)
{
   if (layer_len == 0)
      return channel->layer_len == 0;

   return channel->layer_len == layer_len &&
          memcmp(channel->name, layer, (size_t) layer_len) == 0;
}

static int exri__channel_index_for_role_layer(exri__channel const *channels, int count, int role, char const *layer, int layer_len)
{
   int i;

   for (i = 0; i < count; ++i) {
      if (channels[i].role == role && exri__channel_matches_layer(channels + i, layer, layer_len))
         return i;
   }

   return -1;
}

static int exri__channels_for_layer(exri__channel const *channels, int count, char const *layer, int layer_len, int *channels_out)
{
   int i;
   int matched;
   int has_r;
   int has_g;
   int has_b;
   int has_a;
   int has_y;

   matched = 0;
   has_r = has_g = has_b = has_a = has_y = 0;
   for (i = 0; i < count; ++i) {
      if (!exri__channel_matches_layer(channels + i, layer, layer_len))
         continue;
      matched += 1;
      if (channels[i].role == EXRI__ROLE_R)
         has_r = 1;
      else if (channels[i].role == EXRI__ROLE_G)
         has_g = 1;
      else if (channels[i].role == EXRI__ROLE_B)
         has_b = 1;
      else if (channels[i].role == EXRI__ROLE_A)
         has_a = 1;
      else if (channels[i].role == EXRI__ROLE_Y)
         has_y = 1;
   }

   if (matched <= 0)
      return exri__err("layer not found");

   if (has_r && has_g && has_b)
      *channels_out = has_a ? 4 : 3;
   else if (has_y)
      *channels_out = has_a ? 2 : 1;
   else
      *channels_out = matched;

   return 1;
}

static int exri__same_layer_name(exri__channel const *a, exri__channel const *b)
{
   return a->layer_len == b->layer_len &&
          a->layer_len > 0 &&
          memcmp(a->name, b->name, (size_t) a->layer_len) == 0;
}

static int exri__layer_seen_before(exri__channel const *channels, int index)
{
   int i;

   for (i = 0; i < index; ++i) {
      if (exri__same_layer_name(channels + i, channels + index))
         return 1;
   }

   return 0;
}

static int exri__count_unique_layers(exri__channel const *channels, int count)
{
   int i;
   int layers;

   layers = 0;
   for (i = 0; i < count; ++i) {
      if (channels[i].layer_len > 0 && !exri__layer_seen_before(channels, i))
         layers += 1;
   }

   return layers;
}

static exri__channel const *exri__layer_by_index(exri__channel const *channels, int count, int layer_index)
{
   int i;
   int layers;

   layers = 0;
   for (i = 0; i < count; ++i) {
      if (channels[i].layer_len <= 0 || exri__layer_seen_before(channels, i))
         continue;
      if (layers == layer_index)
         return channels + i;
      layers += 1;
   }

   return NULL;
}

static int exri__copy_name_to_buffer(exri_uc const *src, int src_len, char *name, int name_size)
{
   int i;

   if (src_len <= 0)
      return exri__err_invalid();

   if (name != NULL) {
      if (name_size <= src_len)
         return exri__err("buffer too small");
      for (i = 0; i < src_len; ++i)
         name[i] = (char) src[i];
      name[src_len] = 0;
   }

   return src_len;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_count_from_memory(exri_uc const *buffer, size_t len, int *num_channels)
{
   exri__info info;
   exri__channel *channels;

   channels = NULL;
   if (num_channels == NULL)
      return exri__err("invalid argument");
   *num_channels = 0;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }

   *num_channels = info.num_channel_records;
   EXRI_FREE(channels);
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_name_from_memory(exri_uc const *buffer, size_t len, int channel_index, char *name, int name_size)
{
   exri__info info;
   exri__channel *channels;
   int result;

   channels = NULL;
   if (channel_index < 0)
      return exri__err("invalid argument");

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   result = exri__copy_name_to_buffer(channels[channel_index].name, channels[channel_index].name_len, name, name_size);
   EXRI_FREE(channels);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int channel_index, int *pixel_type)
{
   exri__info info;
   exri__channel *channels;

   channels = NULL;
   if (channel_index < 0 || pixel_type == NULL)
      return exri__err("invalid argument");
   *pixel_type = -1;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   *pixel_type = channels[channel_index].pixel_type;
   EXRI_FREE(channels);
   return 1;
}

static void exri__clear_channel_sampling(int *x_sampling, int *y_sampling, int *p_linear)
{
   if (x_sampling)
      *x_sampling = 0;
   if (y_sampling)
      *y_sampling = 0;
   if (p_linear)
      *p_linear = 0;
}

static void exri__set_channel_sampling(exri__channel const *channel, int *x_sampling, int *y_sampling, int *p_linear)
{
   if (x_sampling)
      *x_sampling = channel->x_sampling;
   if (y_sampling)
      *y_sampling = channel->y_sampling;
   if (p_linear)
      *p_linear = channel->p_linear;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri__info info;
   exri__channel *channels;

   channels = NULL;
   exri__clear_channel_sampling(x_sampling, y_sampling, p_linear);
   if (channel_index < 0)
      return exri__err("invalid argument");

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   exri__set_channel_sampling(channels + channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(channels);
   return 1;
}

EXRIDEF int EXRI_CALL exri_layer_count_from_memory(exri_uc const *buffer, size_t len, int *num_layers)
{
   exri__info info;
   exri__channel *channels;

   channels = NULL;
   if (num_layers == NULL)
      return exri__err("invalid argument");
   *num_layers = 0;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }

   *num_layers = exri__count_unique_layers(channels, info.num_channel_records);
   EXRI_FREE(channels);
   return 1;
}

EXRIDEF int EXRI_CALL exri_layer_name_from_memory(exri_uc const *buffer, size_t len, int layer_index, char *name, int name_size)
{
   exri__info info;
   exri__channel *channels;
   exri__channel const *layer;
   int i;

   channels = NULL;
   if (layer_index < 0)
      return exri__err("invalid argument");

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }

   layer = exri__layer_by_index(channels, info.num_channel_records, layer_index);
   if (layer == NULL) {
      EXRI_FREE(channels);
      return exri__err("layer not found");
   }

   i = exri__copy_name_to_buffer(layer->name, layer->layer_len, name, name_size);
   EXRI_FREE(channels);
   return i;
}

static int exri__parse_header_for_attributes(exri_uc const *buffer, size_t len, exri__info *info)
{
   exri__channel *channels;

   channels = NULL;
   if (!exri__parse_header(buffer, len, info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   EXRI_FREE(channels);

   if (info->multipart)
      return exri__err("multipart metadata unsupported");

   return 1;
}

static int exri__version_field_from_memory(exri_uc const *buffer, size_t len, exri__uint32 *version_field);
static int exri__is_multipart_memory(exri_uc const *buffer, size_t len, int *is_multipart);
static int exri__multipart_find_part(exri_uc const *buffer, size_t len, int part_index, exri__multipart_part_ref *part, size_t *tables_start, size_t *part_table_start, int *num_parts);

static int exri__attribute_by_index_in_header(exri_uc const *buffer, size_t header_start, size_t header_end, int attribute_index, exri__attribute_ref *attribute, int *count_out)
{
   exri__context s;
   char const *name;
   char const *type;
   int name_len;
   int type_len;
   size_t value_pos;
   int value_size;
   int count;
   exri__uint32 attr_size;

   if (attribute_index < 0 && count_out == NULL)
      return exri__err("invalid argument");
   if (attribute_index >= 0 && attribute == NULL)
      return exri__err("invalid argument");
   if (buffer == NULL || header_end < header_start)
      return exri__err_invalid();

   s.data = buffer;
   s.size = header_end;
   s.pos = header_start;
   count = 0;

   for (;;) {
      if (!exri__read_cstring(&s, &name, &name_len))
         return exri__err_invalid();

      if (name_len == 0) {
         if (count_out)
            *count_out = count;
         if (attribute_index >= 0)
            return exri__err("attribute not found");
         return 1;
      }

      if (!exri__read_cstring(&s, &type, &type_len))
         return exri__err_invalid();
      if (!exri__read_u32(&s, &attr_size))
         return exri__err_invalid();
      if (attr_size > (exri__uint32) INT_MAX)
         return exri__err_invalid();

      value_size = (int) attr_size;
      if (!exri__require(&s, (size_t) value_size))
         return exri__err_invalid();
      value_pos = s.pos;

      if (count == attribute_index) {
         attribute->name = (exri_uc const *) name;
         attribute->name_len = name_len;
         attribute->type = (exri_uc const *) type;
         attribute->type_len = type_len;
         attribute->value = buffer + value_pos;
         attribute->value_size = value_size;
         if (count_out)
            *count_out = count + 1;
         return 1;
      }

      count += 1;
      s.pos = value_pos + (size_t) value_size;
   }
}

static int exri__attribute_by_index(exri_uc const *buffer, size_t header_end, int attribute_index, exri__attribute_ref *attribute, int *count_out)
{
   return exri__attribute_by_index_in_header(buffer, 8u, header_end, attribute_index, attribute, count_out);
}

static int exri__part_attribute_header_bounds(exri_uc const *buffer, size_t len, int part_index, size_t *header_start, size_t *header_end)
{
   exri__uint32 version_field;
   exri__info info;
   exri__multipart_part_ref part;

   if (part_index < 0 || header_start == NULL || header_end == NULL)
      return exri__err("invalid argument");
   *header_start = 0;
   *header_end = 0;

   if (!exri__version_field_from_memory(buffer, len, &version_field))
      return 0;

   if ((version_field & 0x00001000u) == 0) {
      if (part_index != 0)
         return exri__err("part not found");
      if (!exri__parse_header_for_attributes(buffer, len, &info))
         return 0;
      *header_start = 8;
      *header_end = info.header_end;
      return 1;
   }

   if (!exri__multipart_find_part(buffer, len, part_index, &part, NULL, NULL, NULL))
      return 0;
   *header_start = part.header_start;
   *header_end = part.header_end;
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_count_from_memory(exri_uc const *buffer, size_t len, int *num_attributes)
{
   exri__info info;

   if (num_attributes == NULL)
      return exri__err("invalid argument");
   *num_attributes = 0;

   if (!exri__parse_header_for_attributes(buffer, len, &info))
      return 0;

   return exri__attribute_by_index(buffer, info.header_end, -1, NULL, num_attributes);
}

EXRIDEF int EXRI_CALL exri_part_attribute_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_attributes)
{
   size_t header_start;
   size_t header_end;

   if (num_attributes == NULL)
      return exri__err("invalid argument");
   *num_attributes = 0;

   if (!exri__part_attribute_header_bounds(buffer, len, part_index, &header_start, &header_end))
      return 0;

   return exri__attribute_by_index_in_header(buffer, header_start, header_end, -1, NULL, num_attributes);
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_name_from_memory(exri_uc const *buffer, size_t len, int attribute_index, char *name, int name_size)
{
   exri__info info;
   exri__attribute_ref attribute;

   if (attribute_index < 0)
      return exri__err("invalid argument");
   if (!exri__parse_header_for_attributes(buffer, len, &info))
      return 0;
   if (!exri__attribute_by_index(buffer, info.header_end, attribute_index, &attribute, NULL))
      return 0;

   return exri__copy_name_to_buffer(attribute.name, attribute.name_len, name, name_size);
}

EXRIDEF int EXRI_CALL exri_part_attribute_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, char *name, int name_size)
{
   exri__attribute_ref attribute;
   size_t header_start;
   size_t header_end;

   if (attribute_index < 0)
      return exri__err("invalid argument");
   if (!exri__part_attribute_header_bounds(buffer, len, part_index, &header_start, &header_end))
      return 0;
   if (!exri__attribute_by_index_in_header(buffer, header_start, header_end, attribute_index, &attribute, NULL))
      return 0;

   return exri__copy_name_to_buffer(attribute.name, attribute.name_len, name, name_size);
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_type_from_memory(exri_uc const *buffer, size_t len, int attribute_index, char *type, int type_size)
{
   exri__info info;
   exri__attribute_ref attribute;

   if (attribute_index < 0)
      return exri__err("invalid argument");
   if (!exri__parse_header_for_attributes(buffer, len, &info))
      return 0;
   if (!exri__attribute_by_index(buffer, info.header_end, attribute_index, &attribute, NULL))
      return 0;

   return exri__copy_name_to_buffer(attribute.type, attribute.type_len, type, type_size);
}

EXRIDEF int EXRI_CALL exri_part_attribute_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, char *type, int type_size)
{
   exri__attribute_ref attribute;
   size_t header_start;
   size_t header_end;

   if (attribute_index < 0)
      return exri__err("invalid argument");
   if (!exri__part_attribute_header_bounds(buffer, len, part_index, &header_start, &header_end))
      return 0;
   if (!exri__attribute_by_index_in_header(buffer, header_start, header_end, attribute_index, &attribute, NULL))
      return 0;

   return exri__copy_name_to_buffer(attribute.type, attribute.type_len, type, type_size);
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value_size_from_memory(exri_uc const *buffer, size_t len, int attribute_index, int *value_size)
{
   exri__info info;
   exri__attribute_ref attribute;

   if (attribute_index < 0 || value_size == NULL)
      return exri__err("invalid argument");
   *value_size = 0;

   if (!exri__parse_header_for_attributes(buffer, len, &info))
      return 0;
   if (!exri__attribute_by_index(buffer, info.header_end, attribute_index, &attribute, NULL))
      return 0;

   *value_size = attribute.value_size;
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_attribute_value_size_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, int *value_size)
{
   exri__attribute_ref attribute;
   size_t header_start;
   size_t header_end;

   if (attribute_index < 0 || value_size == NULL)
      return exri__err("invalid argument");
   *value_size = 0;

   if (!exri__part_attribute_header_bounds(buffer, len, part_index, &header_start, &header_end))
      return 0;
   if (!exri__attribute_by_index_in_header(buffer, header_start, header_end, attribute_index, &attribute, NULL))
      return 0;

   *value_size = attribute.value_size;
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value_from_memory(exri_uc const *buffer, size_t len, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   exri__info info;
   exri__attribute_ref attribute;

   if (attribute_index < 0 || bytes_written == NULL)
      return exri__err("invalid argument");
   *bytes_written = 0;

   if (!exri__parse_header_for_attributes(buffer, len, &info))
      return 0;
   if (!exri__attribute_by_index(buffer, info.header_end, attribute_index, &attribute, NULL))
      return 0;

   if (value == NULL) {
      *bytes_written = attribute.value_size;
      return 1;
   }
   if (value_size < attribute.value_size)
      return exri__err("buffer too small");
   if (attribute.value_size > 0)
      memcpy(value, attribute.value, (size_t) attribute.value_size);
   *bytes_written = attribute.value_size;
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_attribute_value_from_memory(exri_uc const *buffer, size_t len, int part_index, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   exri__attribute_ref attribute;
   size_t header_start;
   size_t header_end;

   if (attribute_index < 0 || bytes_written == NULL)
      return exri__err("invalid argument");
   *bytes_written = 0;

   if (!exri__part_attribute_header_bounds(buffer, len, part_index, &header_start, &header_end))
      return 0;
   if (!exri__attribute_by_index_in_header(buffer, header_start, header_end, attribute_index, &attribute, NULL))
      return 0;

   if (value == NULL) {
      *bytes_written = attribute.value_size;
      return 1;
   }
   if (value_size < attribute.value_size)
      return exri__err("buffer too small");
   if (attribute.value_size > 0)
      memcpy(value, attribute.value, (size_t) attribute.value_size);
   *bytes_written = attribute.value_size;
   return 1;
}

static int exri__attribute_name_matches(exri__attribute_ref const *attribute, char const *name)
{
   int len;

   if (attribute == NULL || name == NULL)
      return 0;
   len = (int) strlen(name);
   return attribute->name_len == len && memcmp(attribute->name, name, (size_t) len) == 0;
}

static int exri__attribute_type_matches(exri__attribute_ref const *attribute, char const *type)
{
   int len;

   if (attribute == NULL || type == NULL)
      return 0;
   len = (int) strlen(type);
   return attribute->type_len == len && memcmp(attribute->type, type, (size_t) len) == 0;
}

static int exri__find_attribute_by_name_in_header(exri_uc const *buffer, size_t header_start, size_t header_end, char const *name, exri__attribute_ref *attribute)
{
   exri__context s;
   char const *attr_name;
   char const *type;
   int attr_name_len;
   int type_len;
   size_t value_pos;
   int value_size;
   exri__uint32 attr_size;

   if (buffer == NULL || header_end < header_start || name == NULL || attribute == NULL)
      return 0;

   s.data = buffer;
   s.size = header_end;
   s.pos = header_start;

   for (;;) {
      if (!exri__read_cstring(&s, &attr_name, &attr_name_len))
         return 0;
      if (attr_name_len == 0)
         break;
      if (!exri__read_cstring(&s, &type, &type_len))
         return 0;
      if (!exri__read_u32(&s, &attr_size) || attr_size > (exri__uint32) INT_MAX)
         return 0;
      value_size = (int) attr_size;
      if (!exri__require(&s, (size_t) value_size))
         return 0;
      value_pos = s.pos;

      attribute->name = (exri_uc const *) attr_name;
      attribute->name_len = attr_name_len;
      attribute->type = (exri_uc const *) type;
      attribute->type_len = type_len;
      attribute->value = buffer + value_pos;
      attribute->value_size = value_size;
      if (exri__attribute_name_matches(attribute, name))
         return 1;

      s.pos = value_pos + (size_t) value_size;
   }

   return 0;
}

static int exri__find_attribute_by_name(exri_uc const *buffer, size_t header_end, char const *name, exri__attribute_ref *attribute)
{
   return exri__find_attribute_by_name_in_header(buffer, 8u, header_end, name, attribute);
}

static int exri__part_channels_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_channels, exri__channel **channels_out)
{
   exri__info info;
   exri__channel *channels;
   exri__multipart_part_ref part;
   exri__attribute_ref attribute;
   int multipart;
   int channel_kinds;
   int num_records;

   channels = NULL;
   if (num_channels)
      *num_channels = 0;
   if (channels_out)
      *channels_out = NULL;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;

   if (!multipart) {
      if (part_index != 0)
         return exri__err("part not found");
      if (!exri__parse_header(buffer, len, &info, &channels)) {
         EXRI_FREE(channels);
         return 0;
      }
      if (info.non_image) {
         EXRI_FREE(channels);
         return exri__err("unsupported EXR storage");
      }
      if (num_channels)
         *num_channels = info.num_channel_records;
      if (channels_out)
         *channels_out = channels;
      else
         EXRI_FREE(channels);
      return 1;
   }

   if (!exri__multipart_find_part(buffer, len, part_index, &part, NULL, NULL, NULL))
      return 0;
   if (part.non_image)
      return exri__err("unsupported EXR storage");
   if (!exri__find_attribute_by_name_in_header(buffer, part.header_start, part.header_end, "channels", &attribute))
      return exri__err_invalid();
   if (!exri__attribute_type_matches(&attribute, "chlist"))
      return exri__err_invalid();

   channel_kinds = 0;
   num_records = 0;
   if (!exri__parse_channels(attribute.value, attribute.value_size, &channel_kinds, &num_records, &channels)) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }
   (void) channel_kinds;

   if (num_channels)
      *num_channels = num_records;
   if (channels_out)
      *channels_out = channels;
   else
      EXRI_FREE(channels);
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_channel_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_channels)
{
   exri__channel *channels;
   int result;

   channels = NULL;
   if (num_channels == NULL)
      return exri__err("invalid argument");
   *num_channels = 0;

   result = exri__part_channels_from_memory(buffer, len, part_index, num_channels, &channels);
   EXRI_FREE(channels);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, char *name, int name_size)
{
   exri__channel *channels;
   int num_channels;
   int result;

   channels = NULL;
   num_channels = 0;
   if (channel_index < 0)
      return exri__err("invalid argument");
   if (!exri__part_channels_from_memory(buffer, len, part_index, &num_channels, &channels))
      return 0;
   if (channel_index >= num_channels) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   result = exri__copy_name_to_buffer(channels[channel_index].name, channels[channel_index].name_len, name, name_size);
   EXRI_FREE(channels);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *pixel_type)
{
   exri__channel *channels;
   int num_channels;

   channels = NULL;
   num_channels = 0;
   if (channel_index < 0 || pixel_type == NULL)
      return exri__err("invalid argument");
   *pixel_type = -1;
   if (!exri__part_channels_from_memory(buffer, len, part_index, &num_channels, &channels))
      return 0;
   if (channel_index >= num_channels) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   *pixel_type = channels[channel_index].pixel_type;
   EXRI_FREE(channels);
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri__channel *channels;
   int num_channels;

   channels = NULL;
   num_channels = 0;
   exri__clear_channel_sampling(x_sampling, y_sampling, p_linear);
   if (channel_index < 0)
      return exri__err("invalid argument");
   if (!exri__part_channels_from_memory(buffer, len, part_index, &num_channels, &channels))
      return 0;
   if (channel_index >= num_channels) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   exri__set_channel_sampling(channels + channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(channels);
   return 1;
}

static int exri__attribute_is_string(exri__attribute_ref const *attribute)
{
   return attribute != NULL &&
          exri__attribute_type_matches(attribute, "string") &&
          attribute->value != NULL &&
          attribute->value_size > 0;
}

static int exri__attribute_string_len(exri__attribute_ref const *attribute)
{
   int len;

   if (!exri__attribute_is_string(attribute))
      return -1;
   len = 0;
   while (len < attribute->value_size && attribute->value[len] != 0)
      len += 1;
   if (len >= attribute->value_size)
      return -1;
   return len;
}

static float exri__read_sample(exri_uc const *p, int pixel_type)
{
   exri__uint32 bits;
   exri__uint16 h;

   if (pixel_type == 1) {
      h = (exri__uint16) p[0] | (exri__uint16) ((exri__uint16) p[1] << 8);
      return exri__half_to_float(h);
   }

   bits = exri__get32le_at(p);
   if (pixel_type == 2)
      return exri__float_from_bits(bits);

   return (float) bits;
}

static int exri__set_pixel(float *out, int out_comp, float r, float g, float b, float a)
{
   float grey;

   if (out_comp == 1) {
      out[0] = r * 0.299f + g * 0.587f + b * 0.114f;
   } else if (out_comp == 2) {
      grey = r * 0.299f + g * 0.587f + b * 0.114f;
      out[0] = grey;
      out[1] = a;
   } else if (out_comp == 3) {
      out[0] = r;
      out[1] = g;
      out[2] = b;
   } else if (out_comp == 4) {
      out[0] = r;
      out[1] = g;
      out[2] = b;
      out[3] = a;
   } else {
      return 0;
   }

   return 1;
}

static int exri__compute_row_bytes(exri__channel const *channels, int num_channels, int *row_bytes)
{
   int c;
   int bytes;

   bytes = 0;
   for (c = 0; c < num_channels; ++c) {
      if (channels[c].x_sampling != 1 || channels[c].y_sampling != 1)
         return exri__err("subsampled channels unsupported");
      if (channels[c].byte_offset != bytes)
         return exri__err("invalid channel layout");
      if (bytes > INT_MAX - channels[c].bytes_per_sample)
         return 0;
      bytes += channels[c].bytes_per_sample;
   }

   if (bytes <= 0)
      return 0;

   *row_bytes = bytes;
   return 1;
}

static int exri__decode_uncompressed_block(float *out, int out_comp, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int line_y, int num_lines, char const *layer, int layer_len)
{
   int x;
   int v;
   int c;
   int value_count;
   int y_rel;
   int row_bytes;
   int expected_len;
   int r_index;
   int g_index;
   int b_index;
   int y_index;
   int known_color;
   float values[4];
   float r;
   float g;
   float b;
   float a;
   float yv;
   float sample;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (num_lines <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (info->width > INT_MAX / row_bytes)
      return 0;
   row_bytes *= info->width;
   if (num_lines > INT_MAX / row_bytes)
      return 0;
   expected_len = num_lines * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed scanline size");

   y_rel = line_y - info->min_y;
   if (y_rel < 0 || num_lines > info->height - y_rel)
      return exri__err_invalid();

   r_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_R, layer, layer_len);
   g_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_G, layer, layer_len);
   b_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_B, layer, layer_len);
   y_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_Y, layer, layer_len);
   known_color = (r_index >= 0 && g_index >= 0 && b_index >= 0) || (y_index >= 0);

   for (v = 0; v < num_lines; ++v) {
      for (x = 0; x < info->width; ++x) {
         r = g = b = yv = 0.0f;
         a = 1.0f;
         values[0] = values[1] = values[2] = values[3] = 0.0f;
         value_count = 0;

         for (c = 0; c < num_channels; ++c) {
            if (!exri__channel_matches_layer(channels + c, layer, layer_len))
               continue;
            sample_index = (size_t) v * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) info->width +
                           (size_t) x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            sample = exri__read_sample(p, channels[c].pixel_type);
            if (channels[c].role == EXRI__ROLE_R)
               r = sample;
            else if (channels[c].role == EXRI__ROLE_G)
               g = sample;
            else if (channels[c].role == EXRI__ROLE_B)
               b = sample;
            else if (channels[c].role == EXRI__ROLE_A)
               a = sample;
            else if (channels[c].role == EXRI__ROLE_Y)
               yv = sample;

            if (value_count < 4)
               values[value_count] = sample;
            value_count += 1;
         }

         out_index = (((size_t) (y_rel + v) * (size_t) info->width) + (size_t) x) * (size_t) out_comp;
         if (!known_color) {
            for (c = 0; c < out_comp; ++c)
               out[out_index + (size_t) c] = values[c];
         } else {
            if (y_index >= 0 && !(r_index >= 0 && g_index >= 0 && b_index >= 0))
               r = g = b = yv;
            if (!exri__set_pixel(out + out_index, out_comp, r, g, b, a))
               return 0;
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_tile(float *out, int out_comp, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int tile_x, int tile_y, int tile_w, int tile_h, char const *layer, int layer_len)
{
   int x;
   int v;
   int c;
   int value_count;
   int row_bytes;
   int expected_len;
   int dest_x;
   int dest_y;
   int r_index;
   int g_index;
   int b_index;
   int y_index;
   int known_color;
   float values[4];
   float r;
   float g;
   float b;
   float a;
   float yv;
   float sample;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (tile_w <= 0 || tile_h <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (tile_x < 0 || tile_y < 0 || tile_x > info->width - tile_w || tile_y > info->height - tile_h)
      return exri__err_invalid();

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (tile_w > INT_MAX / row_bytes)
      return 0;
   row_bytes *= tile_w;
   if (tile_h > INT_MAX / row_bytes)
      return 0;
   expected_len = tile_h * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed tile size");

   r_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_R, layer, layer_len);
   g_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_G, layer, layer_len);
   b_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_B, layer, layer_len);
   y_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_Y, layer, layer_len);
   known_color = (r_index >= 0 && g_index >= 0 && b_index >= 0) || (y_index >= 0);

   for (v = 0; v < tile_h; ++v) {
      dest_y = tile_y + v;
      for (x = 0; x < tile_w; ++x) {
         dest_x = tile_x + x;
         r = g = b = yv = 0.0f;
         a = 1.0f;
         values[0] = values[1] = values[2] = values[3] = 0.0f;
         value_count = 0;

         for (c = 0; c < num_channels; ++c) {
            if (!exri__channel_matches_layer(channels + c, layer, layer_len))
               continue;
            sample_index = (size_t) v * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) tile_w +
                           (size_t) x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            sample = exri__read_sample(p, channels[c].pixel_type);
            if (channels[c].role == EXRI__ROLE_R)
               r = sample;
            else if (channels[c].role == EXRI__ROLE_G)
               g = sample;
            else if (channels[c].role == EXRI__ROLE_B)
               b = sample;
            else if (channels[c].role == EXRI__ROLE_A)
               a = sample;
            else if (channels[c].role == EXRI__ROLE_Y)
               yv = sample;

            if (value_count < 4)
               values[value_count] = sample;
            value_count += 1;
         }

         out_index = (((size_t) dest_y * (size_t) info->width) + (size_t) dest_x) * (size_t) out_comp;
         if (!known_color) {
            for (c = 0; c < out_comp; ++c)
               out[out_index + (size_t) c] = values[c];
         } else {
            if (y_index >= 0 && !(r_index >= 0 && g_index >= 0 && b_index >= 0))
               r = g = b = yv;
            if (!exri__set_pixel(out + out_index, out_comp, r, g, b, a))
               return 0;
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_block_region(float *out, int out_comp, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int line_y, int num_lines, char const *layer, int layer_len, int region_x, int region_y, int region_w, int region_h)
{
   int x_abs;
   int v;
   int c;
   int value_count;
   int y_rel;
   int row_bytes;
   int expected_len;
   int r_index;
   int g_index;
   int b_index;
   int y_index;
   int known_color;
   int block_y0;
   int block_y1;
   int region_x1;
   int region_y1;
   int decode_y0;
   int decode_y1;
   float values[4];
   float r;
   float g;
   float b;
   float a;
   float yv;
   float sample;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (num_lines <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0)
      return exri__err("invalid region");
   if (region_x > info->width - region_w || region_y > info->height - region_h)
      return exri__err("invalid region");

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (info->width > INT_MAX / row_bytes)
      return 0;
   row_bytes *= info->width;
   if (num_lines > INT_MAX / row_bytes)
      return 0;
   expected_len = num_lines * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed scanline size");

   y_rel = line_y - info->min_y;
   if (y_rel < 0 || num_lines > info->height - y_rel)
      return exri__err_invalid();

   block_y0 = y_rel;
   block_y1 = y_rel + num_lines;
   region_x1 = region_x + region_w;
   region_y1 = region_y + region_h;
   decode_y0 = block_y0 > region_y ? block_y0 : region_y;
   decode_y1 = block_y1 < region_y1 ? block_y1 : region_y1;
   if (decode_y0 >= decode_y1)
      return 1;

   r_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_R, layer, layer_len);
   g_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_G, layer, layer_len);
   b_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_B, layer, layer_len);
   y_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_Y, layer, layer_len);
   known_color = (r_index >= 0 && g_index >= 0 && b_index >= 0) || (y_index >= 0);

   for (v = decode_y0 - block_y0; v < decode_y1 - block_y0; ++v) {
      for (x_abs = region_x; x_abs < region_x1; ++x_abs) {
         r = g = b = yv = 0.0f;
         a = 1.0f;
         values[0] = values[1] = values[2] = values[3] = 0.0f;
         value_count = 0;

         for (c = 0; c < num_channels; ++c) {
            if (!exri__channel_matches_layer(channels + c, layer, layer_len))
               continue;
            sample_index = (size_t) v * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) info->width +
                           (size_t) x_abs * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            sample = exri__read_sample(p, channels[c].pixel_type);
            if (channels[c].role == EXRI__ROLE_R)
               r = sample;
            else if (channels[c].role == EXRI__ROLE_G)
               g = sample;
            else if (channels[c].role == EXRI__ROLE_B)
               b = sample;
            else if (channels[c].role == EXRI__ROLE_A)
               a = sample;
            else if (channels[c].role == EXRI__ROLE_Y)
               yv = sample;

            if (value_count < 4)
               values[value_count] = sample;
            value_count += 1;
         }

         out_index = (((size_t) (block_y0 + v - region_y) * (size_t) region_w) + (size_t) (x_abs - region_x)) * (size_t) out_comp;
         if (!known_color) {
            for (c = 0; c < out_comp; ++c)
               out[out_index + (size_t) c] = values[c];
         } else {
            if (y_index >= 0 && !(r_index >= 0 && g_index >= 0 && b_index >= 0))
               r = g = b = yv;
            if (!exri__set_pixel(out + out_index, out_comp, r, g, b, a))
               return 0;
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_tile_region(float *out, int out_comp, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int tile_x, int tile_y, int tile_w, int tile_h, char const *layer, int layer_len, int region_x, int region_y, int region_w, int region_h)
{
   int x_abs;
   int y_abs;
   int c;
   int value_count;
   int row_bytes;
   int expected_len;
   int local_x;
   int local_y;
   int r_index;
   int g_index;
   int b_index;
   int y_index;
   int known_color;
   int tile_x1;
   int tile_y1;
   int region_x1;
   int region_y1;
   int decode_x0;
   int decode_y0;
   int decode_x1;
   int decode_y1;
   float values[4];
   float r;
   float g;
   float b;
   float a;
   float yv;
   float sample;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (tile_w <= 0 || tile_h <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (tile_x < 0 || tile_y < 0 || tile_x > info->width - tile_w || tile_y > info->height - tile_h)
      return exri__err_invalid();
   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0)
      return exri__err("invalid region");
   if (region_x > info->width - region_w || region_y > info->height - region_h)
      return exri__err("invalid region");
   if (region_x == 0 && region_y == 0 && region_w == info->width && region_h == info->height)
      return exri__decode_uncompressed_tile(out, out_comp, info, channels, num_channels, data, data_len, tile_x, tile_y, tile_w, tile_h, layer, layer_len);

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (tile_w > INT_MAX / row_bytes)
      return 0;
   row_bytes *= tile_w;
   if (tile_h > INT_MAX / row_bytes)
      return 0;
   expected_len = tile_h * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed tile size");

   tile_x1 = tile_x + tile_w;
   tile_y1 = tile_y + tile_h;
   region_x1 = region_x + region_w;
   region_y1 = region_y + region_h;
   decode_x0 = tile_x > region_x ? tile_x : region_x;
   decode_y0 = tile_y > region_y ? tile_y : region_y;
   decode_x1 = tile_x1 < region_x1 ? tile_x1 : region_x1;
   decode_y1 = tile_y1 < region_y1 ? tile_y1 : region_y1;
   if (decode_x0 >= decode_x1 || decode_y0 >= decode_y1)
      return 1;

   r_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_R, layer, layer_len);
   g_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_G, layer, layer_len);
   b_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_B, layer, layer_len);
   y_index = exri__channel_index_for_role_layer(channels, num_channels, EXRI__ROLE_Y, layer, layer_len);
   known_color = (r_index >= 0 && g_index >= 0 && b_index >= 0) || (y_index >= 0);

   for (y_abs = decode_y0; y_abs < decode_y1; ++y_abs) {
      local_y = y_abs - tile_y;
      for (x_abs = decode_x0; x_abs < decode_x1; ++x_abs) {
         local_x = x_abs - tile_x;
         r = g = b = yv = 0.0f;
         a = 1.0f;
         values[0] = values[1] = values[2] = values[3] = 0.0f;
         value_count = 0;

         for (c = 0; c < num_channels; ++c) {
            if (!exri__channel_matches_layer(channels + c, layer, layer_len))
               continue;
            sample_index = (size_t) local_y * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) tile_w +
                           (size_t) local_x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            sample = exri__read_sample(p, channels[c].pixel_type);
            if (channels[c].role == EXRI__ROLE_R)
               r = sample;
            else if (channels[c].role == EXRI__ROLE_G)
               g = sample;
            else if (channels[c].role == EXRI__ROLE_B)
               b = sample;
            else if (channels[c].role == EXRI__ROLE_A)
               a = sample;
            else if (channels[c].role == EXRI__ROLE_Y)
               yv = sample;

            if (value_count < 4)
               values[value_count] = sample;
            value_count += 1;
         }

         out_index = (((size_t) (y_abs - region_y) * (size_t) region_w) + (size_t) (x_abs - region_x)) * (size_t) out_comp;
         if (!known_color) {
            for (c = 0; c < out_comp; ++c)
               out[out_index + (size_t) c] = values[c];
         } else {
            if (y_index >= 0 && !(r_index >= 0 && g_index >= 0 && b_index >= 0))
               r = g = b = yv;
            if (!exri__set_pixel(out + out_index, out_comp, r, g, b, a))
               return 0;
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_channels_block(float *out, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int line_y, int num_lines)
{
   int x;
   int v;
   int c;
   int y_rel;
   int row_bytes;
   int expected_len;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (num_lines <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (info->width > INT_MAX / row_bytes)
      return 0;
   row_bytes *= info->width;
   if (num_lines > INT_MAX / row_bytes)
      return 0;
   expected_len = num_lines * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed scanline size");

   y_rel = line_y - info->min_y;
   if (y_rel < 0 || num_lines > info->height - y_rel)
      return exri__err_invalid();

   for (v = 0; v < num_lines; ++v) {
      for (x = 0; x < info->width; ++x) {
         out_index = (((size_t) (y_rel + v) * (size_t) info->width) + (size_t) x) * (size_t) num_channels;
         for (c = 0; c < num_channels; ++c) {
            sample_index = (size_t) v * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) info->width +
                           (size_t) x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            out[out_index + (size_t) c] = exri__read_sample(p, channels[c].pixel_type);
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_channels_tile(float *out, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int tile_x, int tile_y, int tile_w, int tile_h)
{
   int x;
   int v;
   int c;
   int row_bytes;
   int expected_len;
   int dest_x;
   int dest_y;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (tile_w <= 0 || tile_h <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (tile_x < 0 || tile_y < 0 || tile_x > info->width - tile_w || tile_y > info->height - tile_h)
      return exri__err_invalid();

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (tile_w > INT_MAX / row_bytes)
      return 0;
   row_bytes *= tile_w;
   if (tile_h > INT_MAX / row_bytes)
      return 0;
   expected_len = tile_h * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed tile size");

   for (v = 0; v < tile_h; ++v) {
      dest_y = tile_y + v;
      for (x = 0; x < tile_w; ++x) {
         dest_x = tile_x + x;
         out_index = (((size_t) dest_y * (size_t) info->width) + (size_t) dest_x) * (size_t) num_channels;
         for (c = 0; c < num_channels; ++c) {
            sample_index = (size_t) v * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) tile_w +
                           (size_t) x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            out[out_index + (size_t) c] = exri__read_sample(p, channels[c].pixel_type);
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_channels_block_region(float *out, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int line_y, int num_lines, int region_x, int region_y, int region_w, int region_h)
{
   int x_abs;
   int y_abs;
   int c;
   int y_rel;
   int row_bytes;
   int expected_len;
   int block_y0;
   int block_y1;
   int region_x1;
   int region_y1;
   int decode_y0;
   int decode_y1;
   int local_y;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (num_lines <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0)
      return exri__err("invalid region");
   if (region_x > info->width - region_w || region_y > info->height - region_h)
      return exri__err("invalid region");

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (info->width > INT_MAX / row_bytes)
      return 0;
   row_bytes *= info->width;
   if (num_lines > INT_MAX / row_bytes)
      return 0;
   expected_len = num_lines * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed scanline size");

   y_rel = line_y - info->min_y;
   if (y_rel < 0 || num_lines > info->height - y_rel)
      return exri__err_invalid();

   block_y0 = y_rel;
   block_y1 = y_rel + num_lines;
   region_x1 = region_x + region_w;
   region_y1 = region_y + region_h;
   decode_y0 = block_y0 > region_y ? block_y0 : region_y;
   decode_y1 = block_y1 < region_y1 ? block_y1 : region_y1;
   if (decode_y0 >= decode_y1)
      return 1;

   for (y_abs = decode_y0; y_abs < decode_y1; ++y_abs) {
      local_y = y_abs - block_y0;
      for (x_abs = region_x; x_abs < region_x1; ++x_abs) {
         out_index = (((size_t) (y_abs - region_y) * (size_t) region_w) + (size_t) (x_abs - region_x)) * (size_t) num_channels;
         for (c = 0; c < num_channels; ++c) {
            sample_index = (size_t) local_y * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) info->width +
                           (size_t) x_abs * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            out[out_index + (size_t) c] = exri__read_sample(p, channels[c].pixel_type);
         }
      }
   }

   return 1;
}

static int exri__decode_uncompressed_channels_tile_region(float *out, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *data, int data_len, int tile_x, int tile_y, int tile_w, int tile_h, int region_x, int region_y, int region_w, int region_h)
{
   int x_abs;
   int y_abs;
   int c;
   int row_bytes;
   int expected_len;
   int local_x;
   int local_y;
   int tile_x1;
   int tile_y1;
   int region_x1;
   int region_y1;
   int decode_x0;
   int decode_y0;
   int decode_x1;
   int decode_y1;
   exri_uc const *p;
   size_t sample_index;
   size_t out_index;

   if (tile_w <= 0 || tile_h <= 0)
      return 0;
   if (info->width <= 0 || info->height <= 0)
      return 0;
   if (num_channels <= 0)
      return 0;
   if (tile_x < 0 || tile_y < 0 || tile_x > info->width - tile_w || tile_y > info->height - tile_h)
      return exri__err_invalid();
   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0)
      return exri__err("invalid region");
   if (region_x > info->width - region_w || region_y > info->height - region_h)
      return exri__err("invalid region");
   if (region_x == 0 && region_y == 0 && region_w == info->width && region_h == info->height)
      return exri__decode_uncompressed_channels_tile(out, info, channels, num_channels, data, data_len, tile_x, tile_y, tile_w, tile_h);

   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (tile_w > INT_MAX / row_bytes)
      return 0;
   row_bytes *= tile_w;
   if (tile_h > INT_MAX / row_bytes)
      return 0;
   expected_len = tile_h * row_bytes;
   if (data_len != expected_len)
      return exri__err("unsupported packed tile size");

   tile_x1 = tile_x + tile_w;
   tile_y1 = tile_y + tile_h;
   region_x1 = region_x + region_w;
   region_y1 = region_y + region_h;
   decode_x0 = tile_x > region_x ? tile_x : region_x;
   decode_y0 = tile_y > region_y ? tile_y : region_y;
   decode_x1 = tile_x1 < region_x1 ? tile_x1 : region_x1;
   decode_y1 = tile_y1 < region_y1 ? tile_y1 : region_y1;
   if (decode_x0 >= decode_x1 || decode_y0 >= decode_y1)
      return 1;

   for (y_abs = decode_y0; y_abs < decode_y1; ++y_abs) {
      local_y = y_abs - tile_y;
      for (x_abs = decode_x0; x_abs < decode_x1; ++x_abs) {
         local_x = x_abs - tile_x;
         out_index = (((size_t) (y_abs - region_y) * (size_t) region_w) + (size_t) (x_abs - region_x)) * (size_t) num_channels;
         for (c = 0; c < num_channels; ++c) {
            sample_index = (size_t) local_y * (size_t) row_bytes +
                           (size_t) channels[c].byte_offset * (size_t) tile_w +
                           (size_t) local_x * (size_t) channels[c].bytes_per_sample;
            p = data + sample_index;
            out[out_index + (size_t) c] = exri__read_sample(p, channels[c].pixel_type);
         }
      }
   }

   return 1;
}

static int exri__unzip_exr_block(exri_uc *dst, int uncompressed_size, exri_uc const *src, int src_size)
{
   exri_uc *tmp;
   int decoded;
   int i;
   int p;
   exri_uc const *t1;
   exri_uc const *t2;
   exri_uc *s;
   exri_uc *stop;

   if (uncompressed_size == src_size) {
      memcpy(dst, src, (size_t) uncompressed_size);
      return 1;
   }

   tmp = (exri_uc *) EXRI_MALLOC((size_t) uncompressed_size);
   if (tmp == NULL)
      return exri__err("outofmem");

   decoded = exri__zlib_decode_buffer(tmp, uncompressed_size, src, src_size);
   if (decoded != uncompressed_size) {
      EXRI_FREE(tmp);
      return exri__err("bad zlib data");
   }

   p = tmp[0];
   for (i = 1; i < uncompressed_size; ++i) {
      p = p + tmp[i] - 128;
      tmp[i] = (exri_uc) p;
      p = tmp[i];
   }

   t1 = tmp;
   t2 = tmp + (uncompressed_size + 1) / 2;
   s = dst;
   stop = dst + uncompressed_size;

   for (;;) {
      if (s < stop)
         *s++ = *t1++;
      else
         break;

      if (s < stop)
         *s++ = *t2++;
      else
         break;
   }

   EXRI_FREE(tmp);
   return 1;
}

static int exri__unrle_exr_block(exri_uc *dst, int uncompressed_size, exri_uc const *src, int src_size)
{
   exri_uc *tmp;
   int in_pos;
   int out_pos;
   int code;
   int count;
   int i;
   int p;
   exri_uc const *t1;
   exri_uc const *t2;
   exri_uc *s;
   exri_uc *stop;

   if (uncompressed_size == src_size) {
      memcpy(dst, src, (size_t) uncompressed_size);
      return 1;
   }

   if (src_size < 2)
      return exri__err("bad rle data");

   tmp = (exri_uc *) EXRI_MALLOC((size_t) uncompressed_size);
   if (tmp == NULL)
      return exri__err("outofmem");

   in_pos = 0;
   out_pos = 0;
   while (in_pos < src_size) {
      code = src[in_pos++];
      if (code > 127)
         code -= 256;
      if (code < 0) {
         count = -code;
         if (count > src_size - in_pos || count > uncompressed_size - out_pos) {
            EXRI_FREE(tmp);
            return exri__err("bad rle data");
         }
         memcpy(tmp + out_pos, src + in_pos, (size_t) count);
         in_pos += count;
         out_pos += count;
      } else {
         count = code + 1;
         if (in_pos >= src_size || count > uncompressed_size - out_pos) {
            EXRI_FREE(tmp);
            return exri__err("bad rle data");
         }
         memset(tmp + out_pos, src[in_pos], (size_t) count);
         in_pos += 1;
         out_pos += count;
      }
   }

   if (out_pos != uncompressed_size) {
      EXRI_FREE(tmp);
      return exri__err("bad rle data");
   }

   p = tmp[0];
   for (i = 1; i < uncompressed_size; ++i) {
      p = p + tmp[i] - 128;
      tmp[i] = (exri_uc) p;
      p = tmp[i];
   }

   t1 = tmp;
   t2 = tmp + (uncompressed_size + 1) / 2;
   s = dst;
   stop = dst + uncompressed_size;

   for (;;) {
      if (s < stop)
         *s++ = *t1++;
      else
         break;

      if (s < stop)
         *s++ = *t2++;
      else
         break;
   }

   EXRI_FREE(tmp);
   return 1;
}

static int exri__pxr24_channel_bytes(int pixel_type)
{
   if (pixel_type == 1)
      return 2;
   if (pixel_type == 2)
      return 3;
   return 4;
}

static int exri__unpxr24_exr_block(exri_uc *dst, int uncompressed_size, exri_uc const *src, int src_size, exri__channel const *channels, int num_channels, int width, int num_lines)
{
   exri_uc *tmp;
   exri_uc const *in;
   exri_uc *out;
   exri__uint32 pixel;
   exri__uint32 diff;
   int pxr24_row_bytes;
   int pxr24_size;
   int decoded;
   int line;
   int c;
   int x;
   int channel_bytes;

   if (dst == NULL || src == NULL || channels == NULL || width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("bad pxr24 data");

   pxr24_row_bytes = 0;
   for (c = 0; c < num_channels; ++c) {
      if (channels[c].x_sampling != 1 || channels[c].y_sampling != 1)
         return exri__err("subsampled channels unsupported");
      channel_bytes = exri__pxr24_channel_bytes(channels[c].pixel_type);
      if (width > INT_MAX / channel_bytes)
         return exri__err("image too large");
      if (pxr24_row_bytes > INT_MAX - width * channel_bytes)
         return exri__err("image too large");
      pxr24_row_bytes += width * channel_bytes;
   }
   if (num_lines > INT_MAX / pxr24_row_bytes)
      return exri__err("image too large");
   pxr24_size = num_lines * pxr24_row_bytes;

   tmp = (exri_uc *) EXRI_MALLOC((size_t) pxr24_size);
   if (tmp == NULL)
      return exri__err("outofmem");

   if (pxr24_size == src_size) {
      memcpy(tmp, src, (size_t) pxr24_size);
   } else {
      decoded = exri__zlib_decode_buffer(tmp, pxr24_size, src, src_size);
      if (decoded != pxr24_size) {
         EXRI_FREE(tmp);
         return exri__err("bad pxr24 data");
      }
   }

   in = tmp;
   out = dst;
   for (line = 0; line < num_lines; ++line) {
      for (c = 0; c < num_channels; ++c) {
         if (channels[c].pixel_type == 1) {
            exri_uc const *p0;
            exri_uc const *p1;

            p0 = in;
            p1 = in + width;
            in += width * 2;
            pixel = 0;
            for (x = 0; x < width; ++x) {
               diff = ((exri__uint32) p0[x] << 8) | (exri__uint32) p1[x];
               pixel += diff;
               out[0] = (exri_uc) (pixel & 255u);
               out[1] = (exri_uc) ((pixel >> 8) & 255u);
               out += 2;
            }
         } else if (channels[c].pixel_type == 2) {
            exri_uc const *p0;
            exri_uc const *p1;
            exri_uc const *p2;

            p0 = in;
            p1 = in + width;
            p2 = in + width * 2;
            in += width * 3;
            pixel = 0;
            for (x = 0; x < width; ++x) {
               diff = ((exri__uint32) p0[x] << 24) |
                      ((exri__uint32) p1[x] << 16) |
                      ((exri__uint32) p2[x] << 8);
               pixel += diff;
               exri__put32le_at(out, pixel);
               out += 4;
            }
         } else {
            exri_uc const *p0;
            exri_uc const *p1;
            exri_uc const *p2;
            exri_uc const *p3;

            p0 = in;
            p1 = in + width;
            p2 = in + width * 2;
            p3 = in + width * 3;
            in += width * 4;
            pixel = 0;
            for (x = 0; x < width; ++x) {
               diff = ((exri__uint32) p0[x] << 24) |
                      ((exri__uint32) p1[x] << 16) |
                      ((exri__uint32) p2[x] << 8) |
                      (exri__uint32) p3[x];
               pixel += diff;
               exri__put32le_at(out, pixel);
               out += 4;
            }
         }
      }
   }

   if (out != dst + uncompressed_size || in != tmp + pxr24_size) {
      EXRI_FREE(tmp);
      return exri__err("bad pxr24 data");
   }

   EXRI_FREE(tmp);
   return 1;
}

static exri__uint16 exri__b44_from_ordered(exri__uint16 v)
{
   if (v & 0x8000u)
      return (exri__uint16) (v & 0x7fffu);
   return (exri__uint16) ~v;
}

static exri__uint16 exri__b44_delta(exri__uint16 base, unsigned int code, unsigned int shift, unsigned int bias)
{
   return (exri__uint16) ((unsigned int) base + code * (1u << shift) - bias);
}

static double exri__b44_log_significand(int significand)
{
   double y;
   double y2;
   double term;
   double sum;

   y = ((double) significand - 1024.0) / ((double) significand + 1024.0);
   y2 = y * y;
   term = y;
   sum = term;
   term *= y2; sum += term / 3.0;
   term *= y2; sum += term / 5.0;
   term *= y2; sum += term / 7.0;
   term *= y2; sum += term / 9.0;
   term *= y2; sum += term / 11.0;
   term *= y2; sum += term / 13.0;
   term *= y2; sum += term / 15.0;
   return 2.0 * sum;
}

static exri__uint16 exri__b44_float_to_half_trunc(float f)
{
   exri__uint32 bits;
   exri__uint32 s;
   exri__uint32 e;
   exri__uint32 m;

   bits = exri__float_to_bits(f);
   s = (bits >> 31) & 1u;
   e = (bits >> 23) & 255u;
   m = bits & 0x7fffffu;

   if (e == 0)
      return (exri__uint16) (s << 15);
   if (e == 255u)
      return (exri__uint16) ((s << 15) | 0x7c00u | (m >> 13));
   if (e < 113u) {
      if (e < 103u)
         return (exri__uint16) (s << 15);
      m = (m | 0x800000u) >> (int) (114u - e);
      return (exri__uint16) ((s << 15) | (m >> 13));
   }
   if (e > 142u)
      return (exri__uint16) ((s << 15) | 0x7c00u);

   return (exri__uint16) ((s << 15) | ((e - 112u) << 10) | (m >> 13));
}

static exri__uint16 exri__b44_p_linear_to_half(exri__uint16 h)
{
   int exponent;
   int mantissa;
   int significand;
   double value;

   if ((h & 0x7c00u) == 0x7c00u)
      return 0;
   if (h > 0x8000u)
      return 0;

   exponent = (int) ((h >> 10) & 31u);
   mantissa = (int) (h & 1023u);
   if (exponent == 0) {
      if (mantissa == 0)
         return 0;
      significand = mantissa;
      exponent = -14;
      while (significand < 1024) {
         significand <<= 1;
         exponent -= 1;
      }
   } else {
      significand = 1024 + mantissa;
      exponent -= 15;
   }

   value = 8.0 * (exri__b44_log_significand(significand) + (double) exponent * 0.69314718055994530942);
   return exri__b44_float_to_half_trunc((float) value);
}

static void exri__unpack_b44_flat(exri__uint16 dst[16], exri_uc const *src)
{
   exri__uint16 h;
   int i;

   h = exri__b44_from_ordered((exri__uint16) (((exri__uint16) src[0] << 8) | (exri__uint16) src[1]));
   for (i = 0; i < 16; ++i)
      dst[i] = h;
}

static void exri__unpack_b44_block(exri__uint16 dst[16], exri_uc const *src)
{
   unsigned int shift;
   unsigned int bias;
   exri__uint16 s0;
   exri__uint16 s1;
   exri__uint16 s2;
   exri__uint16 s3;
   exri__uint16 s4;
   exri__uint16 s5;
   exri__uint16 s6;
   exri__uint16 s7;
   exri__uint16 s8;
   exri__uint16 s9;
   exri__uint16 s10;
   exri__uint16 s11;
   exri__uint16 s12;
   exri__uint16 s13;
   exri__uint16 s14;
   exri__uint16 s15;
   int i;

   s0 = (exri__uint16) (((exri__uint16) src[0] << 8) | (exri__uint16) src[1]);
   shift = (unsigned int) (src[2] >> 2);
   bias = 0x20u << shift;

   s4 = exri__b44_delta(s0, (((unsigned int) src[2] << 4) | ((unsigned int) src[3] >> 4)) & 0x3fu, shift, bias);
   s8 = exri__b44_delta(s4, (((unsigned int) src[3] << 2) | ((unsigned int) src[4] >> 6)) & 0x3fu, shift, bias);
   s12 = exri__b44_delta(s8, (unsigned int) src[4] & 0x3fu, shift, bias);

   s1 = exri__b44_delta(s0, (unsigned int) src[5] >> 2, shift, bias);
   s5 = exri__b44_delta(s4, (((unsigned int) src[5] << 4) | ((unsigned int) src[6] >> 4)) & 0x3fu, shift, bias);
   s9 = exri__b44_delta(s8, (((unsigned int) src[6] << 2) | ((unsigned int) src[7] >> 6)) & 0x3fu, shift, bias);
   s13 = exri__b44_delta(s12, (unsigned int) src[7] & 0x3fu, shift, bias);

   s2 = exri__b44_delta(s1, (unsigned int) src[8] >> 2, shift, bias);
   s6 = exri__b44_delta(s5, (((unsigned int) src[8] << 4) | ((unsigned int) src[9] >> 4)) & 0x3fu, shift, bias);
   s10 = exri__b44_delta(s9, (((unsigned int) src[9] << 2) | ((unsigned int) src[10] >> 6)) & 0x3fu, shift, bias);
   s14 = exri__b44_delta(s13, (unsigned int) src[10] & 0x3fu, shift, bias);

   s3 = exri__b44_delta(s2, (unsigned int) src[11] >> 2, shift, bias);
   s7 = exri__b44_delta(s6, (((unsigned int) src[11] << 4) | ((unsigned int) src[12] >> 4)) & 0x3fu, shift, bias);
   s11 = exri__b44_delta(s10, (((unsigned int) src[12] << 2) | ((unsigned int) src[13] >> 6)) & 0x3fu, shift, bias);
   s15 = exri__b44_delta(s14, (unsigned int) src[13] & 0x3fu, shift, bias);

   dst[0] = s0;   dst[1] = s1;   dst[2] = s2;   dst[3] = s3;
   dst[4] = s4;   dst[5] = s5;   dst[6] = s6;   dst[7] = s7;
   dst[8] = s8;   dst[9] = s9;   dst[10] = s10; dst[11] = s11;
   dst[12] = s12; dst[13] = s13; dst[14] = s14; dst[15] = s15;

   for (i = 0; i < 16; ++i)
      dst[i] = exri__b44_from_ordered(dst[i]);
}

static int exri__unb44_exr_block(exri_uc *dst, int uncompressed_size, exri_uc const *src, int src_size, exri__channel const *channels, int num_channels, int width, int num_lines)
{
   exri__uint16 block[16];
   int row_bytes;
   int expected_len;
   int in_pos;
   int c;
   int x;
   int y;
   int bx;
   int by;
   int dx;
   int dy;
   int blocks_x;
   int blocks_y;
   int raw_bytes;
   int pixel_bytes;
   int dst_pos;
   int src_pos;

   if (dst == NULL || src == NULL || channels == NULL || width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("bad b44 data");
   if (!exri__compute_row_bytes(channels, num_channels, &row_bytes))
      return 0;
   if (width > INT_MAX / row_bytes)
      return exri__err("image too large");
   row_bytes *= width;
   if (num_lines > INT_MAX / row_bytes)
      return exri__err("image too large");
   expected_len = num_lines * row_bytes;
   if (expected_len != uncompressed_size)
      return exri__err("bad b44 data");
   if (src_size == uncompressed_size) {
      memcpy(dst, src, (size_t) uncompressed_size);
      return 1;
   }

   in_pos = 0;
   for (c = 0; c < num_channels; ++c) {
      if (channels[c].x_sampling != 1 || channels[c].y_sampling != 1)
         return exri__err("subsampled channels unsupported");

      pixel_bytes = channels[c].bytes_per_sample;
      if (channels[c].pixel_type != 1) {
         if (width > INT_MAX / pixel_bytes)
            return exri__err("image too large");
         raw_bytes = width * pixel_bytes;
         if (num_lines > INT_MAX / raw_bytes)
            return exri__err("image too large");
         raw_bytes *= num_lines;
         if (raw_bytes > src_size - in_pos)
            return exri__err("bad b44 data");
         for (y = 0; y < num_lines; ++y) {
            dst_pos = y * row_bytes + channels[c].byte_offset * width;
            src_pos = in_pos + y * width * pixel_bytes;
            memcpy(dst + dst_pos, src + src_pos, (size_t) (width * pixel_bytes));
         }
         in_pos += raw_bytes;
         continue;
      }

      blocks_x = (width + 3) / 4;
      blocks_y = (num_lines + 3) / 4;
      for (by = 0; by < blocks_y; ++by) {
         for (bx = 0; bx < blocks_x; ++bx) {
            if (src_size - in_pos < 3)
               return exri__err("bad b44 data");
            if (src[in_pos + 2] >= (13 << 2)) {
               exri__unpack_b44_flat(block, src + in_pos);
               in_pos += 3;
            } else {
               if (src_size - in_pos < 14)
                  return exri__err("bad b44 data");
               exri__unpack_b44_block(block, src + in_pos);
               in_pos += 14;
            }
            if (channels[c].p_linear) {
               int bi;
               for (bi = 0; bi < 16; ++bi)
                  block[bi] = exri__b44_p_linear_to_half(block[bi]);
            }

            for (dy = 0; dy < 4; ++dy) {
               y = by * 4 + dy;
               if (y >= num_lines)
                  continue;
               for (dx = 0; dx < 4; ++dx) {
                  x = bx * 4 + dx;
                  if (x >= width)
                     continue;
                  dst_pos = y * row_bytes + channels[c].byte_offset * width + x * 2;
                  dst[dst_pos + 0] = (exri_uc) (block[dy * 4 + dx] & 255u);
                  dst[dst_pos + 1] = (exri_uc) ((block[dy * 4 + dx] >> 8) & 255u);
               }
            }
         }
      }
   }

   if (in_pos != src_size)
      return exri__err("bad b44 data");
   return 1;
}

#define EXRI__PIZ_HUF_ENCBITS 16
#define EXRI__PIZ_HUF_DECBITS 14
#define EXRI__PIZ_HUF_ENCSIZE ((1 << EXRI__PIZ_HUF_ENCBITS) + 1)
#define EXRI__PIZ_HUF_DECSIZE (1 << EXRI__PIZ_HUF_DECBITS)
#define EXRI__PIZ_HUF_DECMASK (EXRI__PIZ_HUF_DECSIZE - 1)
#define EXRI__PIZ_SHORT_ZEROCODE_RUN 59
#define EXRI__PIZ_LONG_ZEROCODE_RUN 63
#define EXRI__PIZ_SHORTEST_LONG_RUN (2 + EXRI__PIZ_LONG_ZEROCODE_RUN - EXRI__PIZ_SHORT_ZEROCODE_RUN)
#define EXRI__PIZ_LONGEST_LONG_RUN (255 + EXRI__PIZ_SHORTEST_LONG_RUN)
#define EXRI__PIZ_USHORT_RANGE (1 << 16)
#define EXRI__PIZ_BITMAP_SIZE (EXRI__PIZ_USHORT_RANGE >> 3)

typedef struct
{
   exri_uc const *data;
   int num_bits;
   int bit_pos;
} exri__piz_bits;

typedef struct
{
   unsigned char len;
   unsigned char has_long;
   exri__uint32 lit;
   exri__uint32 *p;
} exri__piz_huf_dec;

typedef struct
{
   exri__uint16 *start;
   int nx;
   int ny;
   int size;
} exri__piz_channel;

static int exri__piz_bits_read(exri__piz_bits *b, int n, exri__uint64 *out)
{
   exri__uint64 v;
   int i;
   int byte_index;
   int bit_shift;

   if (n < 0 || n > 58)
      return 0;
   if (b->bit_pos < 0 || b->num_bits < 0 || b->bit_pos > b->num_bits - n)
      return 0;

   v = 0;
   for (i = 0; i < n; ++i) {
      byte_index = b->bit_pos >> 3;
      bit_shift = 7 - (b->bit_pos & 7);
      v = (v << 1) | (exri__uint64) ((b->data[byte_index] >> bit_shift) & 1);
      b->bit_pos += 1;
   }

   *out = v;
   return 1;
}

static int exri__piz_bits_peek(exri__piz_bits const *b, int n, exri__uint64 *out)
{
   exri__piz_bits copy;

   copy = *b;
   return exri__piz_bits_read(&copy, n, out);
}

static int exri__piz_huf_length(exri__uint64 code)
{
   return (int) (code & 63u);
}

static exri__uint64 exri__piz_huf_code(exri__uint64 code)
{
   return code >> 6;
}

static int exri__piz_huf_canonical_code_table(exri__uint64 *hcode)
{
   exri__uint64 n[59];
   exri__uint64 c;
   exri__uint64 nc;
   int i;
   int l;

   for (i = 0; i <= 58; ++i)
      n[i] = 0;

   for (i = 0; i < EXRI__PIZ_HUF_ENCSIZE; ++i) {
      if (hcode[i] > 58u)
         return 0;
      n[(int) hcode[i]] += 1;
   }

   c = 0;
   for (i = 58; i > 0; --i) {
      nc = (c + n[i]) >> 1;
      n[i] = c;
      c = nc;
   }

   for (i = 0; i < EXRI__PIZ_HUF_ENCSIZE; ++i) {
      l = (int) hcode[i];
      if (l > 0) {
         if ((n[l] >> l) != 0)
            return 0;
         hcode[i] = (n[l]++ << 6) | (exri__uint64) l;
      }
   }

   return 1;
}

static int exri__piz_huf_unpack_enc_table(exri_uc const *src, int src_len, int im, int iM, exri__uint64 *hcode)
{
   exri__piz_bits bits;
   exri__uint64 v;
   int i;
   int l;
   int zerun;

   if (src_len < 0 || src_len > INT_MAX / 8)
      return 0;

   memset(hcode, 0, (size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(hcode[0]));
   bits.data = src;
   bits.num_bits = src_len * 8;
   bits.bit_pos = 0;

   for (i = im; i <= iM; ++i) {
      if (!exri__piz_bits_read(&bits, 6, &v))
         return 0;

      l = (int) v;
      if (l == EXRI__PIZ_LONG_ZEROCODE_RUN) {
         if (!exri__piz_bits_read(&bits, 8, &v))
            return 0;
         zerun = (int) v + EXRI__PIZ_SHORTEST_LONG_RUN;
         if (zerun > iM - i + 1)
            return 0;
         i += zerun - 1;
      } else if (l >= EXRI__PIZ_SHORT_ZEROCODE_RUN) {
         zerun = l - EXRI__PIZ_SHORT_ZEROCODE_RUN + 2;
         if (zerun > iM - i + 1)
            return 0;
         i += zerun - 1;
      } else {
         if (l > 58)
            return 0;
         hcode[i] = (exri__uint64) l;
      }
   }

   return exri__piz_huf_canonical_code_table(hcode);
}

static int exri__piz_huf_build_dec_table(exri__uint64 const *hcode, int im, int iM, exri__piz_huf_dec *hdecod, exri__uint32 **long_symbols_out)
{
   exri__uint32 *long_symbols;
   exri__uint64 code;
   exri__uint64 c;
   int l;
   int i;
   int j;
   int index;
   int start;
   int repeat;
   int total_long;
   int pos;

   long_symbols = NULL;
   *long_symbols_out = NULL;
   memset(hdecod, 0, (size_t) EXRI__PIZ_HUF_DECSIZE * sizeof(hdecod[0]));

   total_long = 0;
   for (i = im; i <= iM; ++i) {
      code = hcode[i];
      l = exri__piz_huf_length(code);
      if (l == 0)
         continue;
      if (l > 58)
         return 0;

      c = exri__piz_huf_code(code);
      if ((c >> l) != 0)
         return 0;

      if (l > EXRI__PIZ_HUF_DECBITS) {
         index = (int) (c >> (l - EXRI__PIZ_HUF_DECBITS));
         if (index < 0 || index >= EXRI__PIZ_HUF_DECSIZE)
            return 0;
         if (hdecod[index].len)
            return 0;
         if (hdecod[index].lit == 0xffffffffu || total_long == INT_MAX)
            return 0;
         hdecod[index].has_long = 1;
         hdecod[index].lit += 1;
         total_long += 1;
      } else {
         start = (int) (c << (EXRI__PIZ_HUF_DECBITS - l));
         repeat = 1 << (EXRI__PIZ_HUF_DECBITS - l);
         if (start < 0 || repeat <= 0 || start > EXRI__PIZ_HUF_DECSIZE - repeat)
            return 0;
         for (j = 0; j < repeat; ++j) {
            if (hdecod[start + j].len || hdecod[start + j].has_long)
               return 0;
         }
         for (j = 0; j < repeat; ++j) {
            hdecod[start + j].len = (unsigned char) l;
            hdecod[start + j].lit = (exri__uint32) i;
         }
      }
   }

   if (total_long > 0) {
      if ((size_t) total_long > ((size_t) -1) / sizeof(long_symbols[0]))
         return 0;
      long_symbols = (exri__uint32 *) EXRI_MALLOC((size_t) total_long * sizeof(long_symbols[0]));
      if (long_symbols == NULL)
         return exri__err("outofmem");

      pos = 0;
      for (i = 0; i < EXRI__PIZ_HUF_DECSIZE; ++i) {
         if (hdecod[i].has_long) {
            repeat = (int) hdecod[i].lit;
            hdecod[i].p = long_symbols + pos;
            hdecod[i].lit = 0;
            pos += repeat;
         }
      }
   }

   for (i = im; i <= iM; ++i) {
      code = hcode[i];
      l = exri__piz_huf_length(code);
      if (l > EXRI__PIZ_HUF_DECBITS) {
         c = exri__piz_huf_code(code);
         index = (int) (c >> (l - EXRI__PIZ_HUF_DECBITS));
         hdecod[index].p[hdecod[index].lit++] = (exri__uint32) i;
      }
   }

   *long_symbols_out = long_symbols;
   return 1;
}

static int exri__piz_huf_emit(int symbol, int rlc, exri__piz_bits *bits, exri__uint16 *out, int no, int *out_pos)
{
   exri__uint64 v;
   exri__uint16 s;
   int count;
   int i;

   if (symbol == rlc) {
      if (*out_pos <= 0)
         return 0;
      if (!exri__piz_bits_read(bits, 8, &v))
         return 0;
      count = (int) v;
      if (count > no - *out_pos)
         return 0;
      s = out[*out_pos - 1];
      for (i = 0; i < count; ++i)
         out[(*out_pos)++] = s;
      return 1;
   }

   if (symbol < 0 || symbol > 65535)
      return 0;
   if (*out_pos >= no)
      return 0;

   out[(*out_pos)++] = (exri__uint16) symbol;
   return 1;
}

static int exri__piz_huf_decode(exri__uint64 const *hcode, exri__piz_huf_dec const *hdecod, exri_uc const *src, int n_bits, int rlc, int no, exri__uint16 *out)
{
   exri__piz_bits bits;
   exri__uint64 look;
   exri__uint64 code;
   exri__piz_huf_dec const *pl;
   int out_pos;
   int remaining;
   int look_bits;
   int index;
   int l;
   int i;
   int symbol;
   int found;

   if (n_bits < 0 || no < 0)
      return 0;

   bits.data = src;
   bits.num_bits = n_bits;
   bits.bit_pos = 0;
   out_pos = 0;

   while (bits.bit_pos < bits.num_bits) {
      remaining = bits.num_bits - bits.bit_pos;
      look_bits = remaining >= EXRI__PIZ_HUF_DECBITS ? EXRI__PIZ_HUF_DECBITS : remaining;
      if (look_bits <= 0)
         return 0;
      if (!exri__piz_bits_peek(&bits, look_bits, &look))
         return 0;

      index = (int) (look_bits == EXRI__PIZ_HUF_DECBITS ? look : (look << (EXRI__PIZ_HUF_DECBITS - look_bits)));
      if (index < 0 || index >= EXRI__PIZ_HUF_DECSIZE)
         return 0;

      pl = hdecod + index;
      if (pl->len) {
         if ((int) pl->len > remaining)
            return 0;
         bits.bit_pos += (int) pl->len;
         if (!exri__piz_huf_emit((int) pl->lit, rlc, &bits, out, no, &out_pos))
            return 0;
      } else {
         if (pl->p == NULL)
            return 0;

         found = 0;
         for (i = 0; i < (int) pl->lit; ++i) {
            symbol = (int) pl->p[i];
            l = exri__piz_huf_length(hcode[symbol]);
            if (l <= remaining) {
               if (!exri__piz_bits_peek(&bits, l, &code))
                  return 0;
               if (exri__piz_huf_code(hcode[symbol]) == code) {
                  bits.bit_pos += l;
                  if (!exri__piz_huf_emit(symbol, rlc, &bits, out, no, &out_pos))
                     return 0;
                  found = 1;
                  break;
               }
            }
         }
         if (!found)
            return 0;
      }
   }

   return out_pos == no;
}

static int exri__piz_huf_uncompress(exri_uc const *compressed, int n_compressed, exri__uint16 *raw, int n_raw)
{
   exri__uint64 *hcode;
   exri__piz_huf_dec *hdec;
   exri__uint32 *long_symbols;
   exri__uint32 im_u;
   exri__uint32 iM_u;
   exri__uint32 table_length_u;
   exri__uint32 n_bits_u;
   int im;
   int iM;
   int table_length;
   int n_bits;
   int data_bytes;
   int max_bits;
   int ok;

   hcode = NULL;
   hdec = NULL;
   long_symbols = NULL;
   ok = 0;

   if (compressed == NULL || raw == NULL || n_compressed < 20 || n_raw < 0)
      return 0;

   im_u = exri__get32le_at(compressed);
   iM_u = exri__get32le_at(compressed + 4);
   table_length_u = exri__get32le_at(compressed + 8);
   n_bits_u = exri__get32le_at(compressed + 12);

   if (im_u >= (exri__uint32) EXRI__PIZ_HUF_ENCSIZE || iM_u >= (exri__uint32) EXRI__PIZ_HUF_ENCSIZE || im_u > iM_u)
      return 0;
   if (table_length_u > (exri__uint32) INT_MAX || n_bits_u > (exri__uint32) INT_MAX)
      return 0;

   table_length = (int) table_length_u;
   n_bits = (int) n_bits_u;
   if (!exri__has_bytes_at(20, n_compressed, table_length))
      return 0;

   data_bytes = n_compressed - 20 - table_length;
   if (n_bits > INT_MAX - 7)
      return 0;
   if (data_bytes != (n_bits + 7) / 8)
      return 0;
   max_bits = data_bytes > INT_MAX / 8 ? INT_MAX : data_bytes * 8;
   if (n_bits > max_bits)
      return 0;

   im = (int) im_u;
   iM = (int) iM_u;

   hcode = (exri__uint64 *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(hcode[0]));
   hdec = (exri__piz_huf_dec *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_DECSIZE * sizeof(hdec[0]));
   if (hcode == NULL || hdec == NULL) {
      exri__err("outofmem");
      goto done;
   }

   if (!exri__piz_huf_unpack_enc_table(compressed + 20, table_length, im, iM, hcode))
      goto done;
   if (!exri__piz_huf_build_dec_table(hcode, im, iM, hdec, &long_symbols))
      goto done;
   if (!exri__piz_huf_decode(hcode, hdec, compressed + 20 + table_length, n_bits, iM, n_raw, raw))
      goto done;

   ok = 1;

done:
   EXRI_FREE(long_symbols);
   EXRI_FREE(hdec);
   EXRI_FREE(hcode);
   return ok;
}

static int exri__piz_i16_from_u16(exri__uint16 x)
{
   if (x < 0x8000u)
      return (int) x;
   return -1 - (int) (0xffffu - (exri__uint32) x);
}

static int exri__piz_sar1(int x)
{
   if (x >= 0)
      return x >> 1;
   return -(((-x) + 1) >> 1);
}

static int exri__piz_odd(int x)
{
   if (x >= 0)
      return x & 1;
   return (-x) & 1;
}

static exri__uint16 exri__piz_u16_from_i16(int x)
{
   if (x >= 0)
      return (exri__uint16) x;
   return (exri__uint16) (0x10000u - (exri__uint32) (-x));
}

static void exri__piz_wdec14(exri__uint16 l, exri__uint16 h, exri__uint16 *a, exri__uint16 *b)
{
   int ls;
   int hs;
   int ai;
   int bi;

   ls = exri__piz_i16_from_u16(l);
   hs = exri__piz_i16_from_u16(h);
   ai = ls + exri__piz_odd(hs) + exri__piz_sar1(hs);
   bi = ai - hs;

   *a = exri__piz_u16_from_i16(ai);
   *b = exri__piz_u16_from_i16(bi);
}

static void exri__piz_wdec16(exri__uint16 l, exri__uint16 h, exri__uint16 *a, exri__uint16 *b)
{
   int m;
   int d;
   int bb;
   int aa;

   m = (int) l;
   d = (int) h;
   bb = (m - (d >> 1)) & 0xffff;
   aa = (d + bb - 32768) & 0xffff;

   *b = (exri__uint16) bb;
   *a = (exri__uint16) aa;
}

static int exri__piz_wav2_decode(exri__uint16 *in, int nx, int ox, int ny, int oy, exri__uint16 mx)
{
   exri__uint16 *py;
   exri__uint16 *ey;
   exri__uint16 *px;
   exri__uint16 *ex;
   exri__uint16 *p01;
   exri__uint16 *p10;
   exri__uint16 *p11;
   exri__uint16 i00;
   exri__uint16 i01;
   exri__uint16 i10;
   exri__uint16 i11;
   int w14;
   int n;
   int p;
   int p2;
   int oy1;
   int oy2;
   int ox1;
   int ox2;

   if (in == NULL || nx <= 0 || ox <= 0 || ny <= 0 || oy <= 0)
      return 0;

   w14 = mx < (1 << 14);
   n = nx > ny ? ny : nx;
   p = 1;
   while (p <= n && p <= INT_MAX / 2)
      p <<= 1;

   p >>= 1;
   p2 = p;
   p >>= 1;

   while (p >= 1) {
      py = in;
      if (ny - p2 > 0 && oy > INT_MAX / (ny - p2))
         return 0;
      ey = in + oy * (ny - p2);
      if (p > INT_MAX / oy || p2 > INT_MAX / oy || p > INT_MAX / ox || p2 > INT_MAX / ox)
         return 0;
      oy1 = oy * p;
      oy2 = oy * p2;
      ox1 = ox * p;
      ox2 = ox * p2;

      for (; py <= ey; py += oy2) {
         px = py;
         if (nx - p2 > 0 && ox > INT_MAX / (nx - p2))
            return 0;
         ex = py + ox * (nx - p2);

         for (; px <= ex; px += ox2) {
            p01 = px + ox1;
            p10 = px + oy1;
            p11 = p10 + ox1;

            if (w14) {
               exri__piz_wdec14(*px, *p10, &i00, &i10);
               exri__piz_wdec14(*p01, *p11, &i01, &i11);
               exri__piz_wdec14(i00, i01, px, p01);
               exri__piz_wdec14(i10, i11, p10, p11);
            } else {
               exri__piz_wdec16(*px, *p10, &i00, &i10);
               exri__piz_wdec16(*p01, *p11, &i01, &i11);
               exri__piz_wdec16(i00, i01, px, p01);
               exri__piz_wdec16(i10, i11, p10, p11);
            }
         }

         if (nx & p) {
            p10 = px + oy1;
            if (w14)
               exri__piz_wdec14(*px, *p10, &i00, p10);
            else
               exri__piz_wdec16(*px, *p10, &i00, p10);
            *px = i00;
         }
      }

      if (ny & p) {
         px = py;
         ex = py + ox * (nx - p2);

         for (; px <= ex; px += ox2) {
            p01 = px + ox1;
            if (w14)
               exri__piz_wdec14(*px, *p01, &i00, p01);
            else
               exri__piz_wdec16(*px, *p01, &i00, p01);
            *px = i00;
         }
      }

      p2 = p;
      p >>= 1;
   }

   return 1;
}

static exri__uint16 exri__piz_reverse_lut_from_bitmap(exri_uc const *bitmap, exri__uint16 *lut)
{
   int i;
   int k;
   int n;

   k = 0;
   for (i = 0; i < EXRI__PIZ_USHORT_RANGE; ++i) {
      if (i == 0 || (bitmap[i >> 3] & (1 << (i & 7))))
         lut[k++] = (exri__uint16) i;
   }

   n = k - 1;
   while (k < EXRI__PIZ_USHORT_RANGE)
      lut[k++] = 0;

   return (exri__uint16) n;
}

static void exri__piz_apply_lut(exri__uint16 const *lut, exri__uint16 *data, int n)
{
   int i;

   for (i = 0; i < n; ++i)
      data[i] = lut[data[i]];
}

static void exri__put16le(exri_uc *p, exri__uint16 v)
{
   p[0] = (exri_uc) (v & 255u);
   p[1] = (exri_uc) (v >> 8);
}

static int exri__unpiz_exr_block(exri_uc *dst, int uncompressed_size, exri_uc const *src, int src_size, exri__channel const *channels, int num_channels, int width, int num_lines)
{
   exri_uc *bitmap;
   exri__uint16 *lut;
   exri__uint16 *tmp;
   exri__piz_channel *channel_data;
   exri__uint16 max_value;
   exri__uint32 length_u;
   int min_nonzero;
   int max_nonzero;
   int bitmap_count;
   int pos;
   int length;
   int tmp_words;
   int total_words;
   int channel_words;
   int line_words;
   int c;
   int j;
   int y;
   int word;
   int out_pos;
   int ok;

   bitmap = NULL;
   lut = NULL;
   tmp = NULL;
   channel_data = NULL;
   ok = 0;

   if (uncompressed_size == src_size) {
      memcpy(dst, src, (size_t) uncompressed_size);
      return 1;
   }

   if (dst == NULL || src == NULL || channels == NULL || uncompressed_size <= 0 || src_size < 8 || width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("bad piz data");
   if ((uncompressed_size & 1) != 0)
      return exri__err("bad piz data");

   tmp_words = uncompressed_size / 2;

   bitmap = (exri_uc *) EXRI_MALLOC((size_t) EXRI__PIZ_BITMAP_SIZE);
   lut = (exri__uint16 *) EXRI_MALLOC((size_t) EXRI__PIZ_USHORT_RANGE * sizeof(lut[0]));
   tmp = (exri__uint16 *) EXRI_MALLOC((size_t) tmp_words * sizeof(tmp[0]));
   channel_data = (exri__piz_channel *) EXRI_MALLOC((size_t) num_channels * sizeof(channel_data[0]));
   if (bitmap == NULL || lut == NULL || tmp == NULL || channel_data == NULL) {
      exri__err("outofmem");
      goto done;
   }

   memset(bitmap, 0, (size_t) EXRI__PIZ_BITMAP_SIZE);

   min_nonzero = (int) ((exri__uint16) src[0] | (exri__uint16) ((exri__uint16) src[1] << 8));
   max_nonzero = (int) ((exri__uint16) src[2] | (exri__uint16) ((exri__uint16) src[3] << 8));
   pos = 4;

   if (max_nonzero >= EXRI__PIZ_BITMAP_SIZE)
      goto bad;

   if (min_nonzero <= max_nonzero) {
      bitmap_count = max_nonzero - min_nonzero + 1;
      if (bitmap_count > src_size - pos)
         goto bad;
      memcpy(bitmap + min_nonzero, src + pos, (size_t) bitmap_count);
      pos += bitmap_count;
   } else if (!(min_nonzero == EXRI__PIZ_BITMAP_SIZE - 1 && max_nonzero == 0)) {
      goto bad;
   }

   max_value = exri__piz_reverse_lut_from_bitmap(bitmap, lut);

   if (src_size - pos < 4)
      goto bad;
   length_u = exri__get32le_at(src + pos);
   if (length_u > (exri__uint32) INT_MAX)
      goto bad;
   length = (int) length_u;
   pos += 4;
   if (length <= 0 || length != src_size - pos)
      goto bad;

   if (!exri__piz_huf_uncompress(src + pos, length, tmp, tmp_words))
      goto bad;

   total_words = 0;
   for (c = 0; c < num_channels; ++c) {
      if (channels[c].bytes_per_sample != 2 && channels[c].bytes_per_sample != 4)
         goto bad;
      channel_data[c].nx = width;
      channel_data[c].ny = num_lines;
      channel_data[c].size = channels[c].bytes_per_sample / 2;
      if (width > INT_MAX / channel_data[c].size)
         goto bad;
      line_words = width * channel_data[c].size;
      if (num_lines > INT_MAX / line_words)
         goto bad;
      channel_words = line_words * num_lines;
      if (total_words > tmp_words - channel_words)
         goto bad;
      channel_data[c].start = tmp + total_words;
      total_words += channel_words;
   }

   if (total_words != tmp_words)
      goto bad;

   for (c = 0; c < num_channels; ++c) {
      for (j = 0; j < channel_data[c].size; ++j) {
         if (!exri__piz_wav2_decode(channel_data[c].start + j,
                                    channel_data[c].nx,
                                    channel_data[c].size,
                                    channel_data[c].ny,
                                    channel_data[c].nx * channel_data[c].size,
                                    max_value))
            goto bad;
      }
   }

   exri__piz_apply_lut(lut, tmp, tmp_words);

   out_pos = 0;
   for (y = 0; y < num_lines; ++y) {
      for (c = 0; c < num_channels; ++c) {
         line_words = width * channel_data[c].size;
         for (j = 0; j < line_words; ++j) {
            if (!exri__has_bytes_at(out_pos, uncompressed_size, 2))
               goto bad;
            word = y * line_words + j;
            exri__put16le(dst + out_pos, channel_data[c].start[word]);
            out_pos += 2;
         }
      }
   }

   if (out_pos != uncompressed_size)
      goto bad;

   ok = 1;
   goto done;

bad:
   exri__err("bad piz data");

done:
   EXRI_FREE(channel_data);
   EXRI_FREE(tmp);
   EXRI_FREE(lut);
   EXRI_FREE(bitmap);
   return ok;
}

static int exri__compression_supported(int compression)
{
   return compression == 0 || compression == 1 || compression == 2 ||
          compression == 3 || compression == 4 || compression == 5 ||
          compression == 6 || compression == 7;
}

static int exri__scanline_block_lines_for_compression(int compression)
{
   if (compression == 3 || compression == 5)
      return 16;
   if (compression == 4 || compression == 6 || compression == 7)
      return 32;
   return 1;
}

static int exri__decompress_exr_block(exri_uc *dst, int expected_len, exri_uc const *src, int src_size, exri__channel const *channels, int num_channels, int width, int num_lines, int compression)
{
   if (compression == 1)
      return exri__unrle_exr_block(dst, expected_len, src, src_size);
   if (compression == 2 || compression == 3)
      return exri__unzip_exr_block(dst, expected_len, src, src_size);
   if (compression == 4)
      return exri__unpiz_exr_block(dst, expected_len, src, src_size, channels, num_channels, width, num_lines);
   if (compression == 5)
      return exri__unpxr24_exr_block(dst, expected_len, src, src_size, channels, num_channels, width, num_lines);
   if (compression == 6 || compression == 7)
      return exri__unb44_exr_block(dst, expected_len, src, src_size, channels, num_channels, width, num_lines);
   return exri__err("compressed EXR unsupported");
}

static int exri__floor_log2_int(int x)
{
   int r;

   if (x <= 0)
      return -1;
   r = -1;
   while (x > 0) {
      x >>= 1;
      r += 1;
   }
   return r;
}

static int exri__ceil_log2_int(int x)
{
   int f;

   f = exri__floor_log2_int(x);
   if (f < 0)
      return -1;
   if ((x & (x - 1)) != 0)
      f += 1;
   return f;
}

static int exri__round_log2_int(int x, int rounding)
{
   if (rounding)
      return exri__ceil_log2_int(x);
   return exri__floor_log2_int(x);
}

static int exri__tiled_level_size(int top_size, int level, int rounding, int *out_size)
{
   int divisor;
   int size;

   if (top_size <= 0 || level < 0 || level > 30)
      return 0;
   divisor = 1 << level;
   size = top_size / divisor;
   if (rounding && size * divisor < top_size)
      size += 1;
   if (size < 1)
      size = 1;
   *out_size = size;
   return 1;
}

static int exri__tiled_num_levels(exri__info const *info, int *num_x_levels, int *num_y_levels)
{
   int largest;

   if (info->tile_level_mode == 0) {
      *num_x_levels = 1;
      *num_y_levels = 1;
      return 1;
   }

   if (info->tile_level_mode == 1) {
      largest = info->width > info->height ? info->width : info->height;
      *num_x_levels = exri__round_log2_int(largest, info->tile_rounding_mode) + 1;
      *num_y_levels = *num_x_levels;
      return *num_x_levels > 0;
   }

   if (info->tile_level_mode == 2) {
      *num_x_levels = exri__round_log2_int(info->width, info->tile_rounding_mode) + 1;
      *num_y_levels = exri__round_log2_int(info->height, info->tile_rounding_mode) + 1;
      return *num_x_levels > 0 && *num_y_levels > 0;
   }

   return exri__err("unsupported tiled level mode");
}

static int exri__ceil_div_int(int a, int b, int *out)
{
   if (a <= 0 || b <= 0)
      return 0;
   if (a > INT_MAX - (b - 1))
      return 0;
   *out = (a + b - 1) / b;
   return 1;
}

static int exri__tiled_level_available(exri__info const *info, int level_x, int level_y, int *level_w, int *level_h)
{
   int num_x_levels;
   int num_y_levels;

   if (level_x < 0 || level_y < 0)
      return exri__err("tiled level not found");
   if (!exri__tiled_num_levels(info, &num_x_levels, &num_y_levels))
      return 0;

   if (info->tile_level_mode == 0) {
      if (level_x != 0 || level_y != 0)
         return exri__err("tiled level not found");
   } else if (info->tile_level_mode == 1) {
      if (level_x != level_y || level_x >= num_x_levels)
         return exri__err("tiled level not found");
   } else if (info->tile_level_mode == 2) {
      if (level_x >= num_x_levels || level_y >= num_y_levels)
         return exri__err("tiled level not found");
   } else {
      return exri__err("unsupported tiled level mode");
   }

   if (!exri__tiled_level_size(info->width, level_x, info->tile_rounding_mode, level_w) ||
       !exri__tiled_level_size(info->height, level_y, info->tile_rounding_mode, level_h))
      return exri__err_invalid();
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_tiled_level_count_from_memory(exri_uc const *buffer, size_t len, int *num_x_levels, int *num_y_levels)
{
   exri__info info;
   exri__channel *channels;
   int nx;
   int ny;

   channels = NULL;
   if (num_x_levels == NULL || num_y_levels == NULL)
      return exri__err("invalid argument");
   *num_x_levels = 0;
   *num_y_levels = 0;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   EXRI_FREE(channels);

   if (info.multipart || info.non_image)
      return exri__err("unsupported EXR storage");
   if (!info.tiled)
      return exri__err("not tiled");
   if (!exri__tiled_num_levels(&info, &nx, &ny))
      return 0;

   *num_x_levels = nx;
   *num_y_levels = ny;
   return 1;
}

static int exri__loadf_tiled_blocks(float *out, int out_comp, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *buffer, size_t len, char const *layer, int layer_len, int target_level_x, int target_level_y, int region_x, int region_y, int region_w, int region_h)
{
   exri_uc *scratch;
   exri_uc const *pixel_data;
   exri__info level_info;
   int row_sample_bytes;
   int tile_row_bytes;
   int expected_len;
   int total_tiles;
   int num_x_tiles;
   int num_y_tiles;
   int num_x_levels;
   int num_y_levels;
   int lx;
   int ly;
   int level_count_x;
   int level_count_y;
   int offset_index;
   size_t chunk_offset;
   int data_len;
   int tile_x;
   int tile_y;
   int level_x;
   int level_y;
   int pixel_x;
   int pixel_y;
   int tile_w;
   int tile_h;
   int level_w;
   int level_h;
   int decode_level;
   int tx;
   int ty;

   scratch = NULL;

   if (info->tile_width <= 0 || info->tile_height <= 0)
      return exri__err_invalid();
   if (!exri__tiled_num_levels(info, &num_x_levels, &num_y_levels))
      return 0;
   if (!exri__compute_row_bytes(channels, num_channels, &row_sample_bytes))
      return 0;

   total_tiles = 0;
   level_count_y = (info->tile_level_mode == 2) ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = (info->tile_level_mode == 2) ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         if (!exri__tiled_level_size(info->width, info->tile_level_mode == 2 ? lx : ly, info->tile_rounding_mode, &level_w))
            return exri__err_invalid();
         if (!exri__tiled_level_size(info->height, ly, info->tile_rounding_mode, &level_h))
            return exri__err_invalid();
         if (!exri__ceil_div_int(level_w, info->tile_width, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, info->tile_height, &num_y_tiles))
            return exri__err("image too large");
         if (num_x_tiles > INT_MAX / num_y_tiles || total_tiles > INT_MAX - num_x_tiles * num_y_tiles)
            return exri__err("image too large");
         total_tiles += num_x_tiles * num_y_tiles;
      }
   }

   if (info->chunk_count_found && info->chunk_count != total_tiles)
      return exri__err("invalid chunk count");
   if (info->header_end > len || ((size_t) total_tiles) > (len - info->header_end) / 8u)
      return exri__err_invalid();

   offset_index = 0;
   level_count_y = (info->tile_level_mode == 2) ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = (info->tile_level_mode == 2) ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = info->tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         decode_level = (level_x == target_level_x && level_y == target_level_y);

         if (!exri__tiled_level_size(info->width, level_x, info->tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(info->height, level_y, info->tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, info->tile_width, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, info->tile_height, &num_y_tiles)) {
            exri__err_invalid();
            goto fail;
         }

         for (ty = 0; ty < num_y_tiles; ++ty) {
            for (tx = 0; tx < num_x_tiles; ++tx) {
               if (!exri__get64le_as_size_at(buffer + info->header_end + (size_t) offset_index * 8u, &chunk_offset)) {
                  exri__err_invalid();
                  goto fail;
               }
               offset_index += 1;
               if (!exri__has_file_bytes_at(chunk_offset, len, 20)) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_x = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 0));
               tile_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
               if (tile_x != tx || tile_y != ty ||
                   exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 8)) != level_x ||
                   exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 12)) != level_y) {
                  exri__err_invalid();
                  goto fail;
               }
               data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 16));
               if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 20u)) {
                  exri__err_invalid();
                  goto fail;
               }

               if (tile_x > INT_MAX / info->tile_width || tile_y > INT_MAX / info->tile_height) {
                  exri__err_invalid();
                  goto fail;
               }
               pixel_x = tile_x * info->tile_width;
               pixel_y = tile_y * info->tile_height;
               if (pixel_x < 0 || pixel_y < 0 || pixel_x >= level_w || pixel_y >= level_h) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_w = info->tile_width;
               tile_h = info->tile_height;
               if (tile_w > level_w - pixel_x)
                  tile_w = level_w - pixel_x;
               if (tile_h > level_h - pixel_y)
                  tile_h = level_h - pixel_y;
               if (tile_w <= 0 || tile_h <= 0) {
                  exri__err_invalid();
                  goto fail;
               }

               if (tile_w > INT_MAX / row_sample_bytes) {
                  exri__err("image too large");
                  goto fail;
               }
               tile_row_bytes = tile_w * row_sample_bytes;
               if (tile_h > INT_MAX / tile_row_bytes) {
                  exri__err("image too large");
                  goto fail;
               }
               expected_len = tile_h * tile_row_bytes;
               pixel_data = buffer + chunk_offset + 20;

               if (info->compression != 0) {
                  scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
                  if (scratch == NULL) {
                     exri__err("outofmem");
                     goto fail;
                  }
                  if (!exri__decompress_exr_block(scratch, expected_len, pixel_data, data_len, channels, num_channels, tile_w, tile_h, info->compression))
                     goto fail;
                  pixel_data = scratch;
                  data_len = expected_len;
               } else if (data_len != expected_len) {
                  exri__err("unsupported packed tile size");
                  goto fail;
               }

               if (decode_level) {
                  level_info = *info;
                  level_info.width = level_w;
                  level_info.height = level_h;
                  if (!exri__decode_uncompressed_tile_region(out, out_comp, &level_info, channels, num_channels, pixel_data, data_len, pixel_x, pixel_y, tile_w, tile_h, layer, layer_len, region_x, region_y, region_w, region_h))
                     goto fail;
               }

               EXRI_FREE(scratch);
               scratch = NULL;
            }
         }
      }
   }

   return 1;

fail:
   EXRI_FREE(scratch);
   return 0;
}

static int exri__loadf_channels_tiled_blocks(float *out, exri__info const *info, exri__channel const *channels, int num_channels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h)
{
   exri_uc *scratch;
   exri_uc const *pixel_data;
   int row_sample_bytes;
   int tile_row_bytes;
   int expected_len;
   int total_tiles;
   int num_x_tiles;
   int num_y_tiles;
   int num_x_levels;
   int num_y_levels;
   int lx;
   int ly;
   int level_count_x;
   int level_count_y;
   int offset_index;
   size_t chunk_offset;
   int data_len;
   int tile_x;
   int tile_y;
   int level_x;
   int level_y;
   int pixel_x;
   int pixel_y;
   int tile_w;
   int tile_h;
   int level_w;
   int level_h;
   int top_level;
   int tx;
   int ty;

   scratch = NULL;

   if (info->tile_width <= 0 || info->tile_height <= 0)
      return exri__err_invalid();
   if (!exri__tiled_num_levels(info, &num_x_levels, &num_y_levels))
      return 0;
   if (!exri__compute_row_bytes(channels, num_channels, &row_sample_bytes))
      return 0;

   total_tiles = 0;
   level_count_y = (info->tile_level_mode == 2) ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = (info->tile_level_mode == 2) ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         if (!exri__tiled_level_size(info->width, info->tile_level_mode == 2 ? lx : ly, info->tile_rounding_mode, &level_w))
            return exri__err_invalid();
         if (!exri__tiled_level_size(info->height, ly, info->tile_rounding_mode, &level_h))
            return exri__err_invalid();
         if (!exri__ceil_div_int(level_w, info->tile_width, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, info->tile_height, &num_y_tiles))
            return exri__err("image too large");
         if (num_x_tiles > INT_MAX / num_y_tiles || total_tiles > INT_MAX - num_x_tiles * num_y_tiles)
            return exri__err("image too large");
         total_tiles += num_x_tiles * num_y_tiles;
      }
   }

   if (info->chunk_count_found && info->chunk_count != total_tiles)
      return exri__err("invalid chunk count");
   if (info->header_end > len || ((size_t) total_tiles) > (len - info->header_end) / 8u)
      return exri__err_invalid();

   offset_index = 0;
   level_count_y = (info->tile_level_mode == 2) ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = (info->tile_level_mode == 2) ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = info->tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         top_level = (level_x == 0 && level_y == 0);

         if (!exri__tiled_level_size(info->width, level_x, info->tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(info->height, level_y, info->tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, info->tile_width, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, info->tile_height, &num_y_tiles)) {
            exri__err_invalid();
            goto fail;
         }

         for (ty = 0; ty < num_y_tiles; ++ty) {
            for (tx = 0; tx < num_x_tiles; ++tx) {
               if (!exri__get64le_as_size_at(buffer + info->header_end + (size_t) offset_index * 8u, &chunk_offset)) {
                  exri__err_invalid();
                  goto fail;
               }
               offset_index += 1;
               if (!exri__has_file_bytes_at(chunk_offset, len, 20)) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_x = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 0));
               tile_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
               if (tile_x != tx || tile_y != ty ||
                   exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 8)) != level_x ||
                   exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 12)) != level_y) {
                  exri__err_invalid();
                  goto fail;
               }
               data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 16));
               if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 20u)) {
                  exri__err_invalid();
                  goto fail;
               }

               if (tile_x > INT_MAX / info->tile_width || tile_y > INT_MAX / info->tile_height) {
                  exri__err_invalid();
                  goto fail;
               }
               pixel_x = tile_x * info->tile_width;
               pixel_y = tile_y * info->tile_height;
               if (pixel_x < 0 || pixel_y < 0 || pixel_x >= level_w || pixel_y >= level_h) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_w = info->tile_width;
               tile_h = info->tile_height;
               if (tile_w > level_w - pixel_x)
                  tile_w = level_w - pixel_x;
               if (tile_h > level_h - pixel_y)
                  tile_h = level_h - pixel_y;
               if (tile_w <= 0 || tile_h <= 0) {
                  exri__err_invalid();
                  goto fail;
               }

               if (tile_w > INT_MAX / row_sample_bytes) {
                  exri__err("image too large");
                  goto fail;
               }
               tile_row_bytes = tile_w * row_sample_bytes;
               if (tile_h > INT_MAX / tile_row_bytes) {
                  exri__err("image too large");
                  goto fail;
               }
               expected_len = tile_h * tile_row_bytes;
               pixel_data = buffer + chunk_offset + 20;

               if (info->compression != 0) {
                  scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
                  if (scratch == NULL) {
                     exri__err("outofmem");
                     goto fail;
                  }
                  if (!exri__decompress_exr_block(scratch, expected_len, pixel_data, data_len, channels, num_channels, tile_w, tile_h, info->compression))
                     goto fail;
                  pixel_data = scratch;
                  data_len = expected_len;
               } else if (data_len != expected_len) {
                  exri__err("unsupported packed tile size");
                  goto fail;
               }

               if (top_level) {
                  if (!exri__decode_uncompressed_channels_tile_region(out, info, channels, num_channels, pixel_data, data_len, pixel_x, pixel_y, tile_w, tile_h, region_x, region_y, region_w, region_h))
                     goto fail;
               }

               EXRI_FREE(scratch);
               scratch = NULL;
            }
         }
      }
   }

   return 1;

fail:
   EXRI_FREE(scratch);
   return 0;
}

static float *exri__loadf_main_layer(exri_uc const *buffer, size_t len, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int out_comp;
   int layer_len;
   int selected_channels;
   int num_blocks;
   int block_lines;
   int row_bytes;
   int expected_len;
   int num_lines;
   int i;
   int data_len;
   int line_y;
   size_t chunk_offset;
   int block_index;
   int y_rel;
   int seen_count;
   exri_uc *scratch;
   exri_uc *seen_blocks;
   exri_uc const *pixel_data;
   size_t total;

   channels = NULL;
   out = NULL;
   scratch = NULL;
   seen_blocks = NULL;
   layer_len = 0;
   selected_channels = 0;

   if (desired_channels < 0 || desired_channels > 4) {
      exri__err("bad desired_channels");
      return NULL;
   }

   if (!exri__layer_string_len(layer, &layer_len))
      return NULL;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (!exri__channels_for_layer(channels, info.num_channel_records, layer, layer_len, &selected_channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (channels_in_file)
      *channels_in_file = selected_channels;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }

   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }

   out_comp = desired_channels ? desired_channels : selected_channels;
   if (out_comp <= 0 || out_comp > 4) {
      EXRI_FREE(channels);
      exri__err("unsupported channel count");
      return NULL;
   }

   if ((size_t) info.width > ((size_t) -1) / (size_t) info.height) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   total = (size_t) info.width * (size_t) info.height;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (total > ((size_t) -1) / (size_t) out_comp || total * (size_t) out_comp > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) out_comp * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__compute_row_bytes(channels, info.num_channel_records, &row_bytes)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }
   if (info.tiled) {
      if (!exri__loadf_tiled_blocks(out, out_comp, &info, channels, info.num_channel_records, buffer, len, layer, layer_len, 0, 0, 0, 0, info.width, info.height)) {
         EXRI_FREE(out);
         EXRI_FREE(channels);
         return NULL;
      }
      if (x)
         *x = info.width;
      if (y)
         *y = info.height;
      EXRI_FREE(channels);
      return out;
   }
   if (info.width > INT_MAX / row_bytes) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   row_bytes *= info.width;

   block_lines = exri__scanline_block_lines_for_compression(info.compression);
   num_blocks = ((info.height - 1) / block_lines) + 1;
   if (info.chunk_count_found && info.chunk_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("invalid chunk count");
      return NULL;
   }

   if (num_blocks < 0 || info.header_end > len || ((size_t) num_blocks) > (len - info.header_end) / 8u) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   seen_blocks = (exri_uc *) EXRI_MALLOC((size_t) num_blocks);
   if (seen_blocks == NULL) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }
   memset(seen_blocks, 0, (size_t) num_blocks);
   seen_count = 0;

   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info.header_end + (size_t) i * 8u, &chunk_offset)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (!exri__has_file_bytes_at(chunk_offset, len, 8)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      line_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset));
      data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
      if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (line_y < info.min_y || line_y > info.max_y) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      y_rel = line_y - info.min_y;
      if ((y_rel % block_lines) != 0) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      block_index = y_rel / block_lines;
      if (block_index < 0 || block_index >= num_blocks || seen_blocks[block_index]) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      seen_blocks[block_index] = 1;
      seen_count += 1;

      num_lines = block_lines;
      if (num_lines > info.height - y_rel)
         num_lines = info.height - y_rel;

      if (num_lines <= 0 || num_lines > INT_MAX / row_bytes) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      expected_len = num_lines * row_bytes;
      pixel_data = buffer + chunk_offset + 8;

      if (info.compression != 0) {
         scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
         if (scratch == NULL) {
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            exri__err("outofmem");
            return NULL;
         }
         if (!exri__decompress_exr_block(scratch, expected_len, buffer + chunk_offset + 8, data_len, channels, info.num_channel_records, info.width, num_lines, info.compression)) {
            EXRI_FREE(scratch);
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            return NULL;
         }
         pixel_data = scratch;
         data_len = expected_len;
      }

      if (!exri__decode_uncompressed_block(out, out_comp, &info, channels, info.num_channel_records, pixel_data, data_len, line_y, num_lines, layer, layer_len)) {
         EXRI_FREE(scratch);
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         return NULL;
      }

      EXRI_FREE(scratch);
      scratch = NULL;
   }

   if (seen_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(seen_blocks);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   if (x)
      *x = info.width;
   if (y)
      *y = info.height;

   EXRI_FREE(channels);
   EXRI_FREE(seen_blocks);
   return out;
}

static float *exri__loadf_main(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_main_layer(buffer, len, NULL, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_region_main_layer(exri_uc const *buffer, size_t len, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int out_comp;
   int layer_len;
   int selected_channels;
   int num_blocks;
   int block_lines;
   int row_bytes;
   int expected_len;
   int num_lines;
   int i;
   int data_len;
   int line_y;
   size_t chunk_offset;
   int block_index;
   int y_rel;
   int seen_count;
   exri_uc *scratch;
   exri_uc *seen_blocks;
   exri_uc const *pixel_data;
   size_t total;

   channels = NULL;
   out = NULL;
   scratch = NULL;
   seen_blocks = NULL;
   layer_len = 0;
   selected_channels = 0;

   if (desired_channels < 0 || desired_channels > 4) {
      exri__err("bad desired_channels");
      return NULL;
   }

   if (!exri__layer_string_len(layer, &layer_len))
      return NULL;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (!exri__channels_for_layer(channels, info.num_channel_records, layer, layer_len, &selected_channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (channels_in_file)
      *channels_in_file = selected_channels;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }

   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }

   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0 ||
       region_x > info.width - region_w || region_y > info.height - region_h) {
      EXRI_FREE(channels);
      exri__err("invalid region");
      return NULL;
   }

   out_comp = desired_channels ? desired_channels : selected_channels;
   if (out_comp <= 0 || out_comp > 4) {
      EXRI_FREE(channels);
      exri__err("unsupported channel count");
      return NULL;
   }

   if ((size_t) region_w > ((size_t) -1) / (size_t) region_h) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   total = (size_t) region_w * (size_t) region_h;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (total > ((size_t) -1) / (size_t) out_comp || total * (size_t) out_comp > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) out_comp * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__compute_row_bytes(channels, info.num_channel_records, &row_bytes)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }
   if (info.tiled) {
      if (!exri__loadf_tiled_blocks(out, out_comp, &info, channels, info.num_channel_records, buffer, len, layer, layer_len, 0, 0, region_x, region_y, region_w, region_h)) {
         EXRI_FREE(out);
         EXRI_FREE(channels);
         return NULL;
      }
      if (x)
         *x = region_w;
      if (y)
         *y = region_h;
      EXRI_FREE(channels);
      return out;
   }
   if (info.width > INT_MAX / row_bytes) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   row_bytes *= info.width;

   block_lines = exri__scanline_block_lines_for_compression(info.compression);
   num_blocks = ((info.height - 1) / block_lines) + 1;
   if (info.chunk_count_found && info.chunk_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("invalid chunk count");
      return NULL;
   }

   if (num_blocks < 0 || info.header_end > len || ((size_t) num_blocks) > (len - info.header_end) / 8u) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   seen_blocks = (exri_uc *) EXRI_MALLOC((size_t) num_blocks);
   if (seen_blocks == NULL) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }
   memset(seen_blocks, 0, (size_t) num_blocks);
   seen_count = 0;

   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info.header_end + (size_t) i * 8u, &chunk_offset)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (!exri__has_file_bytes_at(chunk_offset, len, 8)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      line_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset));
      data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
      if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (line_y < info.min_y || line_y > info.max_y) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      y_rel = line_y - info.min_y;
      if ((y_rel % block_lines) != 0) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      block_index = y_rel / block_lines;
      if (block_index < 0 || block_index >= num_blocks || seen_blocks[block_index]) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      seen_blocks[block_index] = 1;
      seen_count += 1;

      num_lines = block_lines;
      if (num_lines > info.height - y_rel)
         num_lines = info.height - y_rel;

      if (num_lines <= 0 || num_lines > INT_MAX / row_bytes) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      expected_len = num_lines * row_bytes;
      pixel_data = buffer + chunk_offset + 8;

      if (info.compression != 0) {
         scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
         if (scratch == NULL) {
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            exri__err("outofmem");
            return NULL;
         }
         if (!exri__decompress_exr_block(scratch, expected_len, buffer + chunk_offset + 8, data_len, channels, info.num_channel_records, info.width, num_lines, info.compression)) {
            EXRI_FREE(scratch);
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            return NULL;
         }
         pixel_data = scratch;
         data_len = expected_len;
      }

      if (!exri__decode_uncompressed_block_region(out, out_comp, &info, channels, info.num_channel_records, pixel_data, data_len, line_y, num_lines, layer, layer_len, region_x, region_y, region_w, region_h)) {
         EXRI_FREE(scratch);
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         return NULL;
      }

      EXRI_FREE(scratch);
      scratch = NULL;
   }

   if (seen_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(seen_blocks);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   if (x)
      *x = region_w;
   if (y)
      *y = region_h;

   EXRI_FREE(channels);
   EXRI_FREE(seen_blocks);
   return out;
}

static float *exri__loadf_region_main(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_region_main_layer(buffer, len, NULL, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_channels_main(exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels_out)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int num_blocks;
   int block_lines;
   int row_bytes;
   int expected_len;
   int num_lines;
   int i;
   int data_len;
   int line_y;
   size_t chunk_offset;
   int block_index;
   int y_rel;
   int seen_count;
   exri_uc *scratch;
   exri_uc *seen_blocks;
   exri_uc const *pixel_data;
   size_t total;

   channels = NULL;
   out = NULL;
   scratch = NULL;
   seen_blocks = NULL;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (num_channels_out)
      *num_channels_out = info.num_channel_records;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }

   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }

   if ((size_t) info.width > ((size_t) -1) / (size_t) info.height) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   total = (size_t) info.width * (size_t) info.height;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (info.num_channel_records <= 0 ||
       total > ((size_t) -1) / (size_t) info.num_channel_records ||
       total * (size_t) info.num_channel_records > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) info.num_channel_records * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__compute_row_bytes(channels, info.num_channel_records, &row_bytes)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }

   if (info.tiled) {
      if (!exri__loadf_channels_tiled_blocks(out, &info, channels, info.num_channel_records, buffer, len, 0, 0, info.width, info.height)) {
         EXRI_FREE(out);
         EXRI_FREE(channels);
         return NULL;
      }
      if (x)
         *x = info.width;
      if (y)
         *y = info.height;
      EXRI_FREE(channels);
      return out;
   }

   if (info.width > INT_MAX / row_bytes) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   row_bytes *= info.width;

   block_lines = exri__scanline_block_lines_for_compression(info.compression);
   num_blocks = ((info.height - 1) / block_lines) + 1;
   if (info.chunk_count_found && info.chunk_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("invalid chunk count");
      return NULL;
   }

   if (num_blocks < 0 || info.header_end > len || ((size_t) num_blocks) > (len - info.header_end) / 8u) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   seen_blocks = (exri_uc *) EXRI_MALLOC((size_t) num_blocks);
   if (seen_blocks == NULL) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }
   memset(seen_blocks, 0, (size_t) num_blocks);
   seen_count = 0;

   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info.header_end + (size_t) i * 8u, &chunk_offset)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (!exri__has_file_bytes_at(chunk_offset, len, 8)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      line_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset));
      data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
      if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (line_y < info.min_y || line_y > info.max_y) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      y_rel = line_y - info.min_y;
      if ((y_rel % block_lines) != 0) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      block_index = y_rel / block_lines;
      if (block_index < 0 || block_index >= num_blocks || seen_blocks[block_index]) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      seen_blocks[block_index] = 1;
      seen_count += 1;

      num_lines = block_lines;
      if (num_lines > info.height - y_rel)
         num_lines = info.height - y_rel;

      if (num_lines <= 0 || num_lines > INT_MAX / row_bytes) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      expected_len = num_lines * row_bytes;
      pixel_data = buffer + chunk_offset + 8;

      if (info.compression != 0) {
         scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
         if (scratch == NULL) {
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            exri__err("outofmem");
            return NULL;
         }
         if (!exri__decompress_exr_block(scratch, expected_len, buffer + chunk_offset + 8, data_len, channels, info.num_channel_records, info.width, num_lines, info.compression)) {
            EXRI_FREE(scratch);
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            return NULL;
         }
         pixel_data = scratch;
         data_len = expected_len;
      }

      if (!exri__decode_uncompressed_channels_block(out, &info, channels, info.num_channel_records, pixel_data, data_len, line_y, num_lines)) {
         EXRI_FREE(scratch);
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         return NULL;
      }

      EXRI_FREE(scratch);
      scratch = NULL;
   }

   if (seen_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(seen_blocks);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   if (x)
      *x = info.width;
   if (y)
      *y = info.height;

   EXRI_FREE(channels);
   EXRI_FREE(seen_blocks);
   return out;
}

static float *exri__loadf_channels_region_main(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels_out)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int num_blocks;
   int block_lines;
   int row_bytes;
   int expected_len;
   int num_lines;
   int i;
   int data_len;
   int line_y;
   size_t chunk_offset;
   int block_index;
   int y_rel;
   int seen_count;
   exri_uc *scratch;
   exri_uc *seen_blocks;
   exri_uc const *pixel_data;
   size_t total;

   channels = NULL;
   out = NULL;
   scratch = NULL;
   seen_blocks = NULL;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (num_channels_out)
      *num_channels_out = info.num_channel_records;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }

   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }

   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0 ||
       region_x > info.width - region_w || region_y > info.height - region_h) {
      EXRI_FREE(channels);
      exri__err("invalid region");
      return NULL;
   }

   if ((size_t) region_w > ((size_t) -1) / (size_t) region_h) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   total = (size_t) region_w * (size_t) region_h;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (info.num_channel_records <= 0 ||
       total > ((size_t) -1) / (size_t) info.num_channel_records ||
       total * (size_t) info.num_channel_records > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) info.num_channel_records * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__compute_row_bytes(channels, info.num_channel_records, &row_bytes)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }

   if (info.tiled) {
      if (!exri__loadf_channels_tiled_blocks(out, &info, channels, info.num_channel_records, buffer, len, region_x, region_y, region_w, region_h)) {
         EXRI_FREE(out);
         EXRI_FREE(channels);
         return NULL;
      }
      if (x)
         *x = region_w;
      if (y)
         *y = region_h;
      EXRI_FREE(channels);
      return out;
   }

   if (info.width > INT_MAX / row_bytes) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   row_bytes *= info.width;

   block_lines = exri__scanline_block_lines_for_compression(info.compression);
   num_blocks = ((info.height - 1) / block_lines) + 1;
   if (info.chunk_count_found && info.chunk_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("invalid chunk count");
      return NULL;
   }

   if (num_blocks < 0 || info.header_end > len || ((size_t) num_blocks) > (len - info.header_end) / 8u) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   seen_blocks = (exri_uc *) EXRI_MALLOC((size_t) num_blocks);
   if (seen_blocks == NULL) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }
   memset(seen_blocks, 0, (size_t) num_blocks);
   seen_count = 0;

   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info.header_end + (size_t) i * 8u, &chunk_offset)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (!exri__has_file_bytes_at(chunk_offset, len, 8)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      line_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset));
      data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
      if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u)) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      if (line_y < info.min_y || line_y > info.max_y) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      y_rel = line_y - info.min_y;
      if ((y_rel % block_lines) != 0) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      block_index = y_rel / block_lines;
      if (block_index < 0 || block_index >= num_blocks || seen_blocks[block_index]) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }
      seen_blocks[block_index] = 1;
      seen_count += 1;

      num_lines = block_lines;
      if (num_lines > info.height - y_rel)
         num_lines = info.height - y_rel;

      if (num_lines <= 0 || num_lines > INT_MAX / row_bytes) {
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         exri__err_invalid();
         return NULL;
      }

      expected_len = num_lines * row_bytes;
      pixel_data = buffer + chunk_offset + 8;

      if (info.compression != 0) {
         scratch = (exri_uc *) EXRI_MALLOC((size_t) expected_len);
         if (scratch == NULL) {
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            exri__err("outofmem");
            return NULL;
         }
         if (!exri__decompress_exr_block(scratch, expected_len, buffer + chunk_offset + 8, data_len, channels, info.num_channel_records, info.width, num_lines, info.compression)) {
            EXRI_FREE(scratch);
            EXRI_FREE(out);
            EXRI_FREE(seen_blocks);
            EXRI_FREE(channels);
            return NULL;
         }
         pixel_data = scratch;
         data_len = expected_len;
      }

      if (!exri__decode_uncompressed_channels_block_region(out, &info, channels, info.num_channel_records, pixel_data, data_len, line_y, num_lines, region_x, region_y, region_w, region_h)) {
         EXRI_FREE(scratch);
         EXRI_FREE(out);
         EXRI_FREE(seen_blocks);
         EXRI_FREE(channels);
         return NULL;
      }

      EXRI_FREE(scratch);
      scratch = NULL;
   }

   if (seen_count != num_blocks) {
      EXRI_FREE(out);
      EXRI_FREE(seen_blocks);
      EXRI_FREE(channels);
      exri__err_invalid();
      return NULL;
   }

   if (x)
      *x = region_w;
   if (y)
      *y = region_h;

   EXRI_FREE(channels);
   EXRI_FREE(seen_blocks);
   return out;
}

static float *exri__loadf_tiled_level_main(exri_uc const *buffer, size_t len, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int selected_channels;
   int out_comp;
   int level_w;
   int level_h;
   size_t total;

   channels = NULL;
   out = NULL;
   selected_channels = 0;
   level_w = 0;
   level_h = 0;

   if (desired_channels < 0 || desired_channels > 4) {
      exri__err("bad desired_channels");
      return NULL;
   }

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (!exri__channels_for_layer(channels, info.num_channel_records, NULL, 0, &selected_channels)) {
      EXRI_FREE(channels);
      return NULL;
   }
   if (channels_in_file)
      *channels_in_file = selected_channels;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }
   if (!info.tiled) {
      EXRI_FREE(channels);
      exri__err("not tiled");
      return NULL;
   }
   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }
   if (!exri__tiled_level_available(&info, level_x, level_y, &level_w, &level_h)) {
      EXRI_FREE(channels);
      return NULL;
   }

   out_comp = desired_channels ? desired_channels : selected_channels;
   if (out_comp <= 0 || out_comp > 4) {
      EXRI_FREE(channels);
      exri__err("unsupported channel count");
      return NULL;
   }

   if ((size_t) level_w > ((size_t) -1) / (size_t) level_h) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   total = (size_t) level_w * (size_t) level_h;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (total > ((size_t) -1) / (size_t) out_comp || total * (size_t) out_comp > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) out_comp * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__loadf_tiled_blocks(out, out_comp, &info, channels, info.num_channel_records, buffer, len, NULL, 0, level_x, level_y, 0, 0, level_w, level_h)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }

   if (x)
      *x = level_w;
   if (y)
      *y = level_h;
   EXRI_FREE(channels);
   return out;
}

static float *exri__loadf_tiled_level_region_main(exri_uc const *buffer, size_t len, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri__info info;
   exri__channel *channels;
   float *out;
   int selected_channels;
   int out_comp;
   int level_w;
   int level_h;
   size_t total;

   channels = NULL;
   out = NULL;
   selected_channels = 0;
   level_w = 0;
   level_h = 0;

   if (desired_channels < 0 || desired_channels > 4) {
      exri__err("bad desired_channels");
      return NULL;
   }

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return NULL;
   }

   if (!exri__channels_for_layer(channels, info.num_channel_records, NULL, 0, &selected_channels)) {
      EXRI_FREE(channels);
      return NULL;
   }
   if (channels_in_file)
      *channels_in_file = selected_channels;

   if (info.multipart || info.non_image) {
      EXRI_FREE(channels);
      exri__err("unsupported EXR storage");
      return NULL;
   }
   if (!info.tiled) {
      EXRI_FREE(channels);
      exri__err("not tiled");
      return NULL;
   }
   if (!exri__compression_supported(info.compression)) {
      EXRI_FREE(channels);
      exri__err("compressed EXR unsupported");
      return NULL;
   }
   if (!exri__tiled_level_available(&info, level_x, level_y, &level_w, &level_h)) {
      EXRI_FREE(channels);
      return NULL;
   }
   if (region_x < 0 || region_y < 0 || region_w <= 0 || region_h <= 0 ||
       region_x > level_w - region_w || region_y > level_h - region_h) {
      EXRI_FREE(channels);
      exri__err("invalid region");
      return NULL;
   }

   out_comp = desired_channels ? desired_channels : selected_channels;
   if (out_comp <= 0 || out_comp > 4) {
      EXRI_FREE(channels);
      exri__err("unsupported channel count");
      return NULL;
   }

   if ((size_t) region_w > ((size_t) -1) / (size_t) region_h) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   total = (size_t) region_w * (size_t) region_h;
   if (EXRI_MAX_PIXELS > 0 && total > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }
   if (total > ((size_t) -1) / (size_t) out_comp || total * (size_t) out_comp > ((size_t) -1) / sizeof(float)) {
      EXRI_FREE(channels);
      exri__err("image too large");
      return NULL;
   }

   out = (float *) EXRI_MALLOC(total * (size_t) out_comp * sizeof(float));
   if (out == NULL) {
      EXRI_FREE(channels);
      exri__err("outofmem");
      return NULL;
   }

   if (!exri__loadf_tiled_blocks(out, out_comp, &info, channels, info.num_channel_records, buffer, len, NULL, 0, level_x, level_y, region_x, region_y, region_w, region_h)) {
      EXRI_FREE(out);
      EXRI_FREE(channels);
      return NULL;
   }

   if (x)
      *x = region_w;
   if (y)
      *y = region_h;
   EXRI_FREE(channels);
   return out;
}

static double exri__absd(double x)
{
   return x < 0.0 ? -x : x;
}

static int exri__valid_chromaticity(double x, double y)
{
   if (!(x == x) || !(y == y))
      return 0;
   if (x <= -10.0 || x >= 10.0 || y <= -10.0 || y >= 10.0)
      return 0;
   return exri__absd(y) > 1.0e-12;
}

static int exri__invert_3x3(double const m[9], double inv[9])
{
   double c00;
   double c01;
   double c02;
   double c10;
   double c11;
   double c12;
   double c20;
   double c21;
   double c22;
   double det;

   c00 =  m[4] * m[8] - m[5] * m[7];
   c01 = -(m[3] * m[8] - m[5] * m[6]);
   c02 =  m[3] * m[7] - m[4] * m[6];
   c10 = -(m[1] * m[8] - m[2] * m[7]);
   c11 =  m[0] * m[8] - m[2] * m[6];
   c12 = -(m[0] * m[7] - m[1] * m[6]);
   c20 =  m[1] * m[5] - m[2] * m[4];
   c21 = -(m[0] * m[5] - m[2] * m[3]);
   c22 =  m[0] * m[4] - m[1] * m[3];

   det = m[0] * c00 + m[1] * c01 + m[2] * c02;
   if (exri__absd(det) < 1.0e-20)
      return 0;

   inv[0] = c00 / det;
   inv[1] = c10 / det;
   inv[2] = c20 / det;
   inv[3] = c01 / det;
   inv[4] = c11 / det;
   inv[5] = c21 / det;
   inv[6] = c02 / det;
   inv[7] = c12 / det;
   inv[8] = c22 / det;
   return 1;
}

static void exri__mul_3x3_vec(double const m[9], double const v[3], double out[3])
{
   out[0] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2];
   out[1] = m[3] * v[0] + m[4] * v[1] + m[5] * v[2];
   out[2] = m[6] * v[0] + m[7] * v[1] + m[8] * v[2];
}

static void exri__mul_3x3(double const a[9], double const b[9], double out[9])
{
   int r;
   int c;
   int k;
   double sum;

   for (r = 0; r < 3; ++r) {
      for (c = 0; c < 3; ++c) {
         sum = 0.0;
         for (k = 0; k < 3; ++k)
            sum += a[r * 3 + k] * b[k * 3 + c];
         out[r * 3 + c] = sum;
      }
   }
}

static int exri__chromaticities_to_rgb_to_xyz(double rx, double ry, double gx, double gy, double bx, double by, double wx, double wy, double out[9])
{
   double base[9];
   double inv[9];
   double white[3];
   double scale[3];

   if (!exri__valid_chromaticity(rx, ry) ||
       !exri__valid_chromaticity(gx, gy) ||
       !exri__valid_chromaticity(bx, by) ||
       !exri__valid_chromaticity(wx, wy))
      return 0;

   base[0] = rx / ry;
   base[1] = gx / gy;
   base[2] = bx / by;
   base[3] = 1.0;
   base[4] = 1.0;
   base[5] = 1.0;
   base[6] = (1.0 - rx - ry) / ry;
   base[7] = (1.0 - gx - gy) / gy;
   base[8] = (1.0 - bx - by) / by;

   white[0] = wx / wy;
   white[1] = 1.0;
   white[2] = (1.0 - wx - wy) / wy;

   if (!exri__invert_3x3(base, inv))
      return 0;
   exri__mul_3x3_vec(inv, white, scale);

   out[0] = base[0] * scale[0];
   out[1] = base[1] * scale[1];
   out[2] = base[2] * scale[2];
   out[3] = base[3] * scale[0];
   out[4] = base[4] * scale[1];
   out[5] = base[5] * scale[2];
   out[6] = base[6] * scale[0];
   out[7] = base[7] * scale[1];
   out[8] = base[8] * scale[2];
   return 1;
}

static int exri__scrgb_transform(exri__info const *info, double transform[9])
{
   double src_to_xyz[9];
   double scrgb_to_xyz[9];
   double xyz_to_scrgb[9];

   if (!info->chromaticities_found)
      return exri__err("chromaticities missing");

   if (!exri__chromaticities_to_rgb_to_xyz((double) info->chromaticities[0],
                                           (double) info->chromaticities[1],
                                           (double) info->chromaticities[2],
                                           (double) info->chromaticities[3],
                                           (double) info->chromaticities[4],
                                           (double) info->chromaticities[5],
                                           (double) info->chromaticities[6],
                                           (double) info->chromaticities[7],
                                           src_to_xyz))
      return exri__err("invalid chromaticities");

   if (!exri__chromaticities_to_rgb_to_xyz(0.6400, 0.3300,
                                           0.3000, 0.6000,
                                           0.1500, 0.0600,
                                           0.3127, 0.3290,
                                           scrgb_to_xyz))
      return exri__err("invalid scRGB matrix");
   if (!exri__invert_3x3(scrgb_to_xyz, xyz_to_scrgb))
      return exri__err("invalid scRGB matrix");

   exri__mul_3x3(xyz_to_scrgb, src_to_xyz, transform);
   return 1;
}

static int exri__has_rgb_channels_for_layer(exri__channel const *channels, int num_channels, char const *layer, int layer_len)
{
   int has_r;
   int has_g;
   int has_b;
   int i;

   has_r = has_g = has_b = 0;
   for (i = 0; i < num_channels; ++i) {
      if (!exri__channel_matches_layer(channels + i, layer, layer_len))
         continue;
      if (channels[i].role == EXRI__ROLE_R)
         has_r = 1;
      else if (channels[i].role == EXRI__ROLE_G)
         has_g = 1;
      else if (channels[i].role == EXRI__ROLE_B)
         has_b = 1;
   }

   return has_r && has_g && has_b;
}

static int exri__prepare_scrgb_for_layer(exri_uc const *buffer, size_t len, char const *layer, int desired_channels, int color_flags, int *out_comp, int *apply_transform, double transform[9])
{
   exri__info info;
   exri__channel *channels;
   int selected_channels;
   int layer_len;

   channels = NULL;
   selected_channels = 0;
   layer_len = 0;

   if (out_comp == NULL || apply_transform == NULL)
      return exri__err("invalid argument");
   *out_comp = 0;
   *apply_transform = 0;

   if ((color_flags & ~EXRI__COLOR_ASSUME_SCRGB) != 0)
      return exri__err("invalid color flags");
   if (desired_channels != 0 && desired_channels != 3 && desired_channels != 4)
      return exri__err("scRGB requires RGB output");
   if (!exri__layer_string_len(layer, &layer_len))
      return 0;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (!exri__channels_for_layer(channels, info.num_channel_records, layer, layer_len, &selected_channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (!exri__has_rgb_channels_for_layer(channels, info.num_channel_records, layer, layer_len)) {
      EXRI_FREE(channels);
      return exri__err("scRGB requires RGB channels");
   }

   *out_comp = desired_channels ? desired_channels : (selected_channels == 4 ? 4 : 3);
   if (!info.chromaticities_found) {
      if ((color_flags & EXRI__COLOR_ASSUME_SCRGB) == 0) {
         EXRI_FREE(channels);
         return exri__err("chromaticities missing");
      }
   } else {
      if (!exri__scrgb_transform(&info, transform)) {
         EXRI_FREE(channels);
         return 0;
      }
      *apply_transform = 1;
   }

   EXRI_FREE(channels);
   return 1;
}

static void exri__apply_scrgb_transform(float *pixels, int width, int height, int comp, double const transform[9])
{
   size_t total;
   size_t i;
   float *p;
   double r;
   double g;
   double b;
   double nr;
   double ng;
   double nb;

   total = (size_t) width * (size_t) height;
   for (i = 0; i < total; ++i) {
      p = pixels + i * (size_t) comp;
      r = p[0];
      g = p[1];
      b = p[2];
      nr = transform[0] * r + transform[1] * g + transform[2] * b;
      ng = transform[3] * r + transform[4] * g + transform[5] * b;
      nb = transform[6] * r + transform[7] * g + transform[8] * b;
      p[0] = (float) nr;
      p[1] = (float) ng;
      p[2] = (float) nb;
   }
}

static float *exri__loadf_scrgb_main(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   float *pixels;
   double transform[9];
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   pixels = NULL;
   apply_transform = 0;
   local_x = 0;
   local_y = 0;

   if (!exri__prepare_scrgb_for_layer(buffer, len, NULL, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return NULL;

   pixels = exri__loadf_main(buffer, len, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (pixels == NULL)
      return NULL;

   if (apply_transform)
      exri__apply_scrgb_transform(pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);

   return pixels;
}

static int exri__load_flags_to_color_flags(int load_flags, int *color_flags)
{
   if (color_flags == NULL)
      return exri__err("invalid argument");
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *color_flags = 0;
      return 1;
   }
   if (load_flags == EXRI_LOAD_SCRGB_STRICT) {
      *color_flags = 0;
      return 1;
   }
   if (load_flags == EXRI_LOAD_SCRGB_ASSUME) {
      *color_flags = EXRI__COLOR_ASSUME_SCRGB;
      return 1;
   }
   return exri__err("invalid load flags");
}

static float *exri__loadf_from_memory_ptr(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_main(buffer, len, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_layer_from_memory_ptr(exri_uc const *buffer, size_t len, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_main_layer(buffer, len, layer, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_region_from_memory_ptr(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_region_main(buffer, len, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_layer_region_from_memory_ptr(exri_uc const *buffer, size_t len, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_region_main_layer(buffer, len, layer, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_channels_from_memory_ptr(exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels)
{
   return exri__loadf_channels_main(buffer, len, x, y, num_channels);
}

static float *exri__loadf_channels_region_from_memory_ptr(exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   return exri__loadf_channels_region_main(buffer, len, region_x, region_y, region_w, region_h, x, y, num_channels);
}

static float *exri__loadf_tiled_level_from_memory_ptr(exri_uc const *buffer, size_t len, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_tiled_level_main(buffer, len, level_x, level_y, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_tiled_level_region_from_memory_ptr(exri_uc const *buffer, size_t len, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels)
{
   return exri__loadf_tiled_level_region_main(buffer, len, level_x, level_y, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
}

static float *exri__loadf_scrgb_from_memory_ptr(exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   return exri__loadf_scrgb_main(buffer, len, x, y, channels_in_file, desired_channels, color_flags);
}

EXRIDEF int EXRI_CALL exri_loadf_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   int color_flags;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT)
      *out_pixels = exri__loadf_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels);
   else
      *out_pixels = exri__loadf_scrgb_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels, color_flags);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_layer_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   double transform[9];
   int color_flags;
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *out_pixels = exri__loadf_layer_from_memory_ptr(buffer, len, layer, x, y, channels_in_file, desired_channels);
      return *out_pixels != NULL;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(buffer, len, layer, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return 0;
   *out_pixels = exri__loadf_layer_from_memory_ptr(buffer, len, layer, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (*out_pixels != NULL && apply_transform)
      exri__apply_scrgb_transform(*out_pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   double transform[9];
   int color_flags;
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *out_pixels = exri__loadf_region_from_memory_ptr(buffer, len, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
      return *out_pixels != NULL;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(buffer, len, NULL, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return 0;
   *out_pixels = exri__loadf_region_from_memory_ptr(buffer, len, region_x, region_y, region_w, region_h, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (*out_pixels != NULL && apply_transform)
      exri__apply_scrgb_transform(*out_pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_layer_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   double transform[9];
   int color_flags;
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *out_pixels = exri__loadf_layer_region_from_memory_ptr(buffer, len, layer, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
      return *out_pixels != NULL;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(buffer, len, layer, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return 0;
   *out_pixels = exri__loadf_layer_region_from_memory_ptr(buffer, len, layer, region_x, region_y, region_w, region_h, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (*out_pixels != NULL && apply_transform)
      exri__apply_scrgb_transform(*out_pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_channels_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels)
{
   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   *out_pixels = exri__loadf_channels_from_memory_ptr(buffer, len, x, y, num_channels);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_channels_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   *out_pixels = exri__loadf_channels_region_from_memory_ptr(buffer, len, region_x, region_y, region_w, region_h, x, y, num_channels);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_tiled_level_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   double transform[9];
   int color_flags;
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *out_pixels = exri__loadf_tiled_level_from_memory_ptr(buffer, len, level_x, level_y, x, y, channels_in_file, desired_channels);
      return *out_pixels != NULL;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(buffer, len, NULL, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return 0;
   *out_pixels = exri__loadf_tiled_level_from_memory_ptr(buffer, len, level_x, level_y, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (*out_pixels != NULL && apply_transform)
      exri__apply_scrgb_transform(*out_pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_tiled_level_region_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   double transform[9];
   int color_flags;
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (out_pixels == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT) {
      *out_pixels = exri__loadf_tiled_level_region_from_memory_ptr(buffer, len, level_x, level_y, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels);
      return *out_pixels != NULL;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(buffer, len, NULL, desired_channels, color_flags, &out_comp, &apply_transform, transform))
      return 0;
   *out_pixels = exri__loadf_tiled_level_region_from_memory_ptr(buffer, len, level_x, level_y, region_x, region_y, region_w, region_h, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (*out_pixels != NULL && apply_transform)
      exri__apply_scrgb_transform(*out_pixels, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   return *out_pixels != NULL;
}

static int exri__grow_buffer(exri_uc **buffer, size_t *capacity, size_t needed)
{
   size_t new_capacity_size;
   void *new_buffer;

   new_capacity_size = *capacity;
   if (new_capacity_size == 0)
      new_capacity_size = 4096u;

   while (new_capacity_size < needed) {
      if (new_capacity_size > ((size_t) -1) / 2u) {
         new_capacity_size = needed;
         break;
      }
      new_capacity_size *= 2u;
   }

   new_buffer = EXRI_REALLOC(*buffer, new_capacity_size);
   if (new_buffer == NULL)
      return 0;

   *buffer = (exri_uc *) new_buffer;
   *capacity = new_capacity_size;
   return 1;
}

static int exri__append(exri_uc **buffer, size_t *length, size_t *capacity, exri_uc const *data, int data_len)
{
   size_t needed;

   if (data_len < 0)
      return exri__err_invalid();
   if (*length > ((size_t) -1) - (size_t) data_len)
      return exri__err("input too large");

   needed = *length + (size_t) data_len;
   if (EXRI_MAX_INPUT_SIZE > 0 && needed > EXRI_MAX_INPUT_SIZE)
      return exri__err("input too large");

   if (needed > *capacity) {
      if (!exri__grow_buffer(buffer, capacity, needed))
         return exri__err("outofmem");
   }

   if (data_len > 0)
      memcpy(*buffer + *length, data, (size_t) data_len);
   *length = needed;
   return 1;
}

static int exri__append_output(exri_uc **buffer, size_t *length, size_t *capacity, exri_uc const *data, int data_len)
{
   size_t needed;

   if (data_len < 0)
      return exri__err("invalid argument");
   if (*length > ((size_t) -1) - (size_t) data_len)
      return exri__err("output too large");

   needed = *length + (size_t) data_len;
   if (EXRI_MAX_OUTPUT_SIZE > 0 && needed > EXRI_MAX_OUTPUT_SIZE)
      return exri__err("output too large");

   if (needed > *capacity) {
      if (!exri__grow_buffer(buffer, capacity, needed))
         return exri__err("outofmem");
   }

   if (data_len > 0)
      memcpy(*buffer + *length, data, (size_t) data_len);
   *length = needed;
   return 1;
}

static int exri__append_cstr_output(exri_uc **buffer, size_t *length, size_t *capacity, char const *s)
{
   size_t n;

   n = strlen(s) + 1u;
   if (n > (size_t) INT_MAX)
      return exri__err("output too large");

   return exri__append_output(buffer, length, capacity, (exri_uc const *) s, (int) n);
}

static int exri__append_cstrn_output(exri_uc **buffer, size_t *length, size_t *capacity, char const *s, int n)
{
   exri_uc zero;

   if (s == NULL || n < 0)
      return exri__err("invalid argument");
   if (!exri__append_output(buffer, length, capacity, (exri_uc const *) s, n))
      return 0;
   zero = 0;
   return exri__append_output(buffer, length, capacity, &zero, 1);
}

static int exri__append_u8_output(exri_uc **buffer, size_t *length, size_t *capacity, exri_uc v)
{
   return exri__append_output(buffer, length, capacity, &v, 1);
}

static int exri__append_u32le_output(exri_uc **buffer, size_t *length, size_t *capacity, exri__uint32 v)
{
   exri_uc tmp[4];

   exri__put32le_at(tmp, v);
   return exri__append_output(buffer, length, capacity, tmp, 4);
}

static int exri__append_u64le_size_output(exri_uc **buffer, size_t *length, size_t *capacity, size_t v)
{
   exri_uc tmp[8];

   exri__put64le_size_at(tmp, v);
   return exri__append_output(buffer, length, capacity, tmp, 8);
}

static int exri__append_attr_output(exri_uc **buffer, size_t *length, size_t *capacity, char const *name, char const *type, exri_uc const *data, int data_len)
{
   if (data_len < 0)
      return exri__err("invalid argument");

   if (!exri__append_cstr_output(buffer, length, capacity, name))
      return 0;
   if (!exri__append_cstr_output(buffer, length, capacity, type))
      return 0;
   if (!exri__append_u32le_output(buffer, length, capacity, (exri__uint32) data_len))
      return 0;
   return exri__append_output(buffer, length, capacity, data, data_len);
}

static int exri__version_field_from_memory(exri_uc const *buffer, size_t len, exri__uint32 *version_field)
{
   exri__uint32 v;

   if (buffer == NULL || len < 8 || version_field == NULL)
      return exri__err_invalid();
   if (buffer[0] != 0x76 || buffer[1] != 0x2f || buffer[2] != 0x31 || buffer[3] != 0x01)
      return exri__err_invalid();

   v = exri__get32le_at(buffer + 4);
   if ((v & 255u) != 2u)
      return exri__err("unsupported EXR version");

   *version_field = v;
   return 1;
}

EXRIDEF int EXRI_CALL exri_version_from_memory(exri_uc const *buffer, size_t len, int *version, int *flags)
{
   exri__uint32 version_field;
   exri__uint32 known_flags;

   if (version == NULL || flags == NULL)
      return exri__err("invalid argument");
   *version = 0;
   *flags = 0;

   if (!exri__version_field_from_memory(buffer, len, &version_field))
      return 0;
   known_flags = (exri__uint32) EXRI_VERSION_FLAG_TILED |
                 (exri__uint32) EXRI_VERSION_FLAG_LONG_NAMES |
                 (exri__uint32) EXRI_VERSION_FLAG_NON_IMAGE |
                 (exri__uint32) EXRI_VERSION_FLAG_MULTIPART;
   *version = (int) (version_field & 255u);
   *flags = (int) (version_field & known_flags);
   return 1;
}

static int exri__attr_value_equals_cstr(exri_uc const *value, int value_size, char const *s)
{
   int len;

   if (value == NULL || value_size < 0 || s == NULL)
      return 0;

   len = 0;
   while (s[len] != 0)
      len += 1;

   if (value_size == len)
      return memcmp(value, s, (size_t) len) == 0;
   if (value_size == len + 1 && value[len] == 0)
      return memcmp(value, s, (size_t) len) == 0;
   return 0;
}

static int exri__attr_value_starts_with_cstr(exri_uc const *value, int value_size, char const *s)
{
   int len;

   if (value == NULL || value_size < 0 || s == NULL)
      return 0;

   len = 0;
   while (s[len] != 0)
      len += 1;

   if (value_size < len)
      return 0;
   return memcmp(value, s, (size_t) len) == 0;
}

static int exri__scan_multipart_part_header(exri_uc const *buffer, size_t len, size_t start, exri__multipart_part_ref *ref)
{
   exri__context s;
   char const *name;
   char const *type;
   int name_len;
   int type_len;
   size_t value_pos;
   int value_size;
   exri__uint32 attr_size;

   if (buffer == NULL || ref == NULL || start >= len)
      return exri__err_invalid();

   memset(ref, 0, sizeof(*ref));
   ref->header_start = start;

   s.data = buffer;
   s.size = len;
   s.pos = start;

   for (;;) {
      if (!exri__read_cstring(&s, &name, &name_len))
         return exri__err_invalid();
      if (name_len == 0) {
         ref->header_end = s.pos;
         if (!ref->name_found || !ref->type_found || !ref->chunk_count_found)
            return exri__err_invalid();
         if (ref->chunk_count <= 0)
            return exri__err_invalid();
         return 1;
      }

      if (!exri__read_cstring(&s, &type, &type_len))
         return exri__err_invalid();
      if (!exri__read_u32(&s, &attr_size))
         return exri__err_invalid();
      if (attr_size > (exri__uint32) INT_MAX)
         return exri__err_invalid();

      value_size = (int) attr_size;
      if (!exri__require(&s, (size_t) value_size))
         return exri__err_invalid();
      value_pos = s.pos;

      if (strcmp(name, "chunkCount") == 0) {
         if (strcmp(type, "int") != 0 || value_size != 4 || ref->chunk_count_found)
            return exri__err_invalid();
         ref->chunk_count = exri__i32_from_u32(exri__get32le_at(buffer + value_pos));
         ref->chunk_count_found = 1;
      } else if (strcmp(name, "tiles") == 0) {
         if (strcmp(type, "tiledesc") != 0 || value_size != 9)
            return exri__err_invalid();
         ref->tiled = 1;
      } else if (strcmp(name, "name") == 0) {
         if (strcmp(type, "string") != 0 || value_size <= 0 || ref->name_found)
            return exri__err_invalid();
         ref->name_found = 1;
      } else if (strcmp(name, "type") == 0) {
         if (strcmp(type, "string") != 0 || value_size <= 0 || ref->type_found)
            return exri__err_invalid();
         ref->type_found = 1;
         if (exri__attr_value_equals_cstr(buffer + value_pos, value_size, "tiledimage"))
            ref->tiled = 1;
         else if (exri__attr_value_starts_with_cstr(buffer + value_pos, value_size, "deep"))
            ref->non_image = 1;
         else if (!exri__attr_value_equals_cstr(buffer + value_pos, value_size, "scanlineimage")) {
            if (!exri__attr_value_equals_cstr(buffer + value_pos, value_size, "tiledimage"))
               return exri__err("unsupported EXR storage");
         }
      }

      s.pos = value_pos + (size_t) value_size;
   }
}

static int exri__multipart_find_part(exri_uc const *buffer, size_t len, int part_index, exri__multipart_part_ref *part, size_t *tables_start, size_t *part_table_start, int *num_parts)
{
   exri__uint32 version_field;
   exri__multipart_part_ref current;
   exri__multipart_part_ref selected;
   size_t pos;
   int count;
   int chunks_before;
   int total_chunks;
   int selected_chunks_before;
   int found;

   if (part_index < -1)
      return exri__err("invalid argument");
   if (!exri__version_field_from_memory(buffer, len, &version_field))
      return 0;
   if ((version_field & 0x00001000u) == 0)
      return exri__err("not multipart");

   pos = 8;
   count = 0;
   chunks_before = 0;
   total_chunks = 0;
   selected_chunks_before = 0;
   found = 0;
   memset(&selected, 0, sizeof(selected));

   for (;;) {
      if (pos >= len)
         return exri__err_invalid();
      if (buffer[pos] == 0) {
         pos += 1;
         break;
      }
      if (!exri__scan_multipart_part_header(buffer, len, pos, &current))
         return 0;
      if (part_index >= 0 && count == part_index) {
         selected = current;
         selected_chunks_before = chunks_before;
         found = 1;
      }
      if (current.chunk_count > INT_MAX - total_chunks)
         return exri__err_invalid();
      chunks_before += current.chunk_count;
      total_chunks += current.chunk_count;
      if (count == INT_MAX)
         return exri__err_invalid();
      count += 1;
      pos = current.header_end;
   }

   if (count <= 0)
      return exri__err_invalid();
   if (part_index >= 0 && !found)
      return exri__err("part not found");
   if ((size_t) total_chunks > (((size_t) -1) - pos) / 8u)
      return exri__err_invalid();
   if (pos + (size_t) total_chunks * 8u > len)
      return exri__err_invalid();

   if (part)
      *part = selected;
   if (tables_start)
      *tables_start = pos;
   if (part_table_start)
      *part_table_start = pos + (size_t) selected_chunks_before * 8u;
   if (num_parts)
      *num_parts = count;

   return 1;
}

static int exri__deep_read_chunk_header(exri_uc const *buffer, size_t len, size_t chunk_offset, int *line_y, int *packed_offset_size, int *packed_sample_size, int *unpacked_sample_size, size_t *payload_pos);

static int exri__synthesize_multipart_part_to_memory(exri_uc **out_data, size_t *out_len, exri_uc const *buffer, size_t len, int part_index)
{
   exri__multipart_part_ref part;
   exri_uc *out;
   size_t capacity;
   size_t length;
   size_t tables_start;
   size_t part_table_start;
   int i;
   size_t original_offset;
   size_t chunk_offset;
   int data_len;
   int chunk_total;
   int packed_offsets;
   int packed_samples;
   size_t offsets_pos;
   exri__uint32 version_field;
   exri__uint32 part_number;

   if (out_data == NULL || out_len == NULL || part_index < 0)
      return exri__err("invalid argument");

   *out_data = NULL;
   *out_len = 0;
   out = NULL;
   capacity = 0;
   length = 0;

   if (!exri__multipart_find_part(buffer, len, part_index, &part, &tables_start, &part_table_start, NULL))
      return 0;
   (void) tables_start;

   if (part.non_image && part.tiled)
      return exri__err("unsupported EXR storage");

   version_field = 2u;
   if (part.non_image)
      version_field |= 0x00000800u;
   if (part.tiled)
      version_field |= 0x00000200u;

   if (part.header_end - part.header_start > (size_t) INT_MAX) {
      exri__err("input too large");
      goto fail;
   }
   if (!exri__append_u32le_output(&out, &length, &capacity, 20000630u) ||
       !exri__append_u32le_output(&out, &length, &capacity, version_field) ||
       !exri__append_output(&out, &length, &capacity, buffer + part.header_start, (int) (part.header_end - part.header_start)))
      goto fail;

   offsets_pos = length;
   for (i = 0; i < part.chunk_count; ++i) {
      if (!exri__append_u64le_size_output(&out, &length, &capacity, 0))
         goto fail;
   }

   for (i = 0; i < part.chunk_count; ++i) {
      if (!exri__get64le_as_size_at(buffer + part_table_start + (size_t) i * 8u, &original_offset)) {
         exri__err_invalid();
         goto fail;
      }
      if (!exri__has_file_bytes_at(original_offset, len, 4)) {
         exri__err_invalid();
         goto fail;
      }

      part_number = exri__get32le_at(buffer + original_offset);
      if (part_number != (exri__uint32) part_index) {
         exri__err_invalid();
         goto fail;
      }

      chunk_offset = original_offset + 4;
      if (part.non_image) {
         if (!exri__deep_read_chunk_header(buffer, len, chunk_offset, NULL, &packed_offsets, &packed_samples, NULL, NULL)) {
            goto fail;
         }
         if (packed_offsets > INT_MAX - 28 || packed_samples > INT_MAX - (28 + packed_offsets)) {
            exri__err_invalid();
            goto fail;
         }
         chunk_total = 28 + packed_offsets + packed_samples;
      } else if (part.tiled) {
         if (!exri__has_file_bytes_at(chunk_offset, len, 20)) {
            exri__err_invalid();
            goto fail;
         }
         data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 16));
         if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 20u)) {
            exri__err_invalid();
            goto fail;
         }
         chunk_total = 20 + data_len;
      } else {
         if (!exri__has_file_bytes_at(chunk_offset, len, 8)) {
            exri__err_invalid();
            goto fail;
         }
         data_len = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset + 4));
         if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u)) {
            exri__err_invalid();
            goto fail;
         }
         chunk_total = 8 + data_len;
      }

      exri__put64le_size_at(out + offsets_pos + (size_t) i * 8u, length);
      if (!exri__append_output(&out, &length, &capacity, buffer + chunk_offset, chunk_total))
         goto fail;
   }

   *out_data = out;
   *out_len = length;
   return 1;

fail:
   EXRI_FREE(out);
   *out_data = NULL;
   *out_len = 0;
   return 0;
}

static int exri__single_part_index_ok(exri_uc const *buffer, size_t len, int part_index)
{
   exri__info info;

   if (part_index != 0)
      return exri__err("part not found");
   if (!exri__parse_header(buffer, len, &info, NULL))
      return 0;
   if (info.multipart)
      return exri__err_invalid();
   return 1;
}

static int exri__is_multipart_memory(exri_uc const *buffer, size_t len, int *is_multipart)
{
   exri__uint32 version_field;

   if (is_multipart == NULL)
      return exri__err("invalid argument");
   *is_multipart = 0;
   if (!exri__version_field_from_memory(buffer, len, &version_field))
      return 0;
   *is_multipart = (version_field & 0x00001000u) ? 1 : 0;
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_count_from_memory(exri_uc const *buffer, size_t len, int *num_parts)
{
   int multipart;
   int count;

   if (num_parts == NULL)
      return exri__err("invalid argument");
   *num_parts = 0;

   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, 0))
         return 0;
      *num_parts = 1;
      return 1;
   }

   count = 0;
   if (!exri__multipart_find_part(buffer, len, -1, NULL, NULL, NULL, &count))
      return 0;
   *num_parts = count;
   return 1;
}

EXRIDEF int EXRI_CALL exri_part_info_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_info_from_memory(buffer, len, x, y, channels_in_file);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_info_from_memory(single, single_len, x, y, channels_in_file);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_tiled_level_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_x_levels, int *num_y_levels)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_tiled_level_count_from_memory(buffer, len, num_x_levels, num_y_levels);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_tiled_level_count_from_memory(single, single_len, num_x_levels, num_y_levels);
   EXRI_FREE(single);
   return result;
}

static float *exri__loadf_part_from_memory_ptr(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   float *result;

   if (part_index < 0) {
      exri__err("invalid argument");
      return NULL;
   }
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return NULL;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return NULL;
      return exri__loadf_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return NULL;

   result = exri__loadf_from_memory_ptr(single, single_len, x, y, channels_in_file, desired_channels);
   EXRI_FREE(single);
   return result;
}

static float *exri__loadf_part_channels_from_memory_ptr(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   float *result;

   if (part_index < 0) {
      exri__err("invalid argument");
      return NULL;
   }
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return NULL;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return NULL;
      return exri__loadf_channels_from_memory_ptr(buffer, len, x, y, num_channels);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return NULL;

   result = exri__loadf_channels_from_memory_ptr(single, single_len, x, y, num_channels);
   EXRI_FREE(single);
   return result;
}

static float *exri__loadf_part_tiled_level_from_memory_ptr(exri_uc const *buffer, size_t len, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   float *result;

   if (part_index < 0) {
      exri__err("invalid argument");
      return NULL;
   }
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return NULL;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return NULL;
      return exri__loadf_tiled_level_from_memory_ptr(buffer, len, level_x, level_y, x, y, channels_in_file, desired_channels);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return NULL;

   result = exri__loadf_tiled_level_from_memory_ptr(single, single_len, level_x, level_y, x, y, channels_in_file, desired_channels);
   EXRI_FREE(single);
   return result;
}

static float *exri__loadf_part_tiled_level_scrgb_from_memory_ptr(exri_uc const *buffer, size_t len, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   exri_uc *owned_single;
   exri_uc const *single;
   size_t single_len;
   int multipart;
   float *result;
   double transform[9];
   int apply_transform;
   int out_comp;
   int local_x;
   int local_y;

   if (part_index < 0) {
      exri__err("invalid argument");
      return NULL;
   }
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return NULL;

   owned_single = NULL;
   single = NULL;
   single_len = 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return NULL;
      single = buffer;
      single_len = len;
   } else {
      if (!exri__synthesize_multipart_part_to_memory(&owned_single, &single_len, buffer, len, part_index))
         return NULL;
      single = owned_single;
   }

   apply_transform = 0;
   out_comp = 0;
   local_x = 0;
   local_y = 0;
   if (!exri__prepare_scrgb_for_layer(single, single_len, NULL, desired_channels, color_flags, &out_comp, &apply_transform, transform)) {
      if (multipart)
         EXRI_FREE(owned_single);
      return NULL;
   }

   result = exri__loadf_tiled_level_from_memory_ptr(single, single_len, level_x, level_y, x ? x : &local_x, y ? y : &local_y, channels_in_file, out_comp);
   if (result != NULL && apply_transform)
      exri__apply_scrgb_transform(result, x ? *x : local_x, y ? *y : local_y, out_comp, transform);
   if (multipart)
      EXRI_FREE(owned_single);
   return result;
}

static float *exri__loadf_part_scrgb_from_memory_ptr(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   float *result;

   if (part_index < 0) {
      exri__err("invalid argument");
      return NULL;
   }
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return NULL;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return NULL;
      return exri__loadf_scrgb_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels, color_flags);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return NULL;

   result = exri__loadf_scrgb_from_memory_ptr(single, single_len, x, y, channels_in_file, desired_channels, color_flags);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_part_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   int color_flags;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT)
      *out_pixels = exri__loadf_part_from_memory_ptr(buffer, len, part_index, x, y, channels_in_file, desired_channels);
   else
      *out_pixels = exri__loadf_part_scrgb_from_memory_ptr(buffer, len, part_index, x, y, channels_in_file, desired_channels, color_flags);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_part_channels_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels)
{
   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;
   *out_pixels = exri__loadf_part_channels_from_memory_ptr(buffer, len, part_index, x, y, num_channels);
   return *out_pixels != NULL;
}

EXRIDEF int EXRI_CALL exri_loadf_part_tiled_level_from_memory(float **out_pixels, exri_uc const *buffer, size_t len, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   int color_flags;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;
   if (!exri__load_flags_to_color_flags(load_flags, &color_flags))
      return 0;
   if (load_flags == EXRI_LOAD_DEFAULT)
      *out_pixels = exri__loadf_part_tiled_level_from_memory_ptr(buffer, len, part_index, level_x, level_y, x, y, channels_in_file, desired_channels);
   else
      *out_pixels = exri__loadf_part_tiled_level_scrgb_from_memory_ptr(buffer, len, part_index, level_x, level_y, x, y, channels_in_file, desired_channels, color_flags);
   return *out_pixels != NULL;
}

static int exri__deep_compression_supported(int compression)
{
   return compression == 0 || compression == 1 || compression == 2 || compression == 3;
}

static int exri__deep_decode_payload(exri_uc *dst, int dst_len, exri_uc const *src, int src_len, int compression)
{
   if (dst_len < 0 || src_len < 0 || src == NULL)
      return exri__err_invalid();
   if (dst_len > 0 && dst == NULL)
      return exri__err_invalid();

   if (compression == 0) {
      if (src_len != dst_len)
         return exri__err_invalid();
      if (dst_len > 0)
         memcpy(dst, src, (size_t) dst_len);
      return 1;
   }

   if (dst_len == 0)
      return src_len == 0 ? 1 : exri__err_invalid();

   if (compression == 1)
      return exri__unrle_exr_block(dst, dst_len, src, src_len);

   if (compression == 2 || compression == 3) {
      return exri__unzip_exr_block(dst, dst_len, src, src_len);
   }

   return exri__err("unsupported deep compression");
}

static int exri__deep_prepare(exri_uc const *buffer, size_t len, exri__info *info, exri__channel **channels_out, int *pixel_count, int *num_blocks, int *block_lines, int *sample_size)
{
   exri__attribute_ref attribute;
   exri__channel *channels;
   int c;
   int bytes;
   int blocks;
   int lines_per_block;
   size_t total_pixels;

   channels = NULL;
   if (channels_out == NULL)
      return exri__err("invalid argument");
   *channels_out = NULL;

   if (!exri__parse_header(buffer, len, info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (info->multipart) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }
   if (!info->non_image) {
      EXRI_FREE(channels);
      return exri__err("not deep");
   }
   if (info->tiled) {
      EXRI_FREE(channels);
      return exri__err("unsupported EXR storage");
   }

   if (exri__find_attribute_by_name(buffer, info->header_end, "type", &attribute)) {
      if (!exri__attr_value_equals_cstr(attribute.value, attribute.value_size, "deepscanline")) {
         EXRI_FREE(channels);
         return exri__err("unsupported EXR storage");
      }
   }

   if (!exri__deep_compression_supported(info->compression)) {
      EXRI_FREE(channels);
      return exri__err("unsupported deep compression");
   }

   bytes = 0;
   for (c = 0; c < info->num_channel_records; ++c) {
      if (channels[c].x_sampling != 1 || channels[c].y_sampling != 1) {
         EXRI_FREE(channels);
         return exri__err("subsampled channels unsupported");
      }
      if (channels[c].bytes_per_sample <= 0) {
         EXRI_FREE(channels);
         return exri__err_invalid();
      }
      if (bytes > INT_MAX - channels[c].bytes_per_sample) {
         EXRI_FREE(channels);
         return exri__err("image too large");
      }
      bytes += channels[c].bytes_per_sample;
   }
   if (bytes <= 0) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }

   if ((size_t) info->width > ((size_t) -1) / (size_t) info->height) {
      EXRI_FREE(channels);
      return exri__err("image too large");
   }
   total_pixels = (size_t) info->width * (size_t) info->height;
   if (EXRI_MAX_PIXELS > 0 && total_pixels > (size_t) EXRI_MAX_PIXELS) {
      EXRI_FREE(channels);
      return exri__err("image too large");
   }
   if (total_pixels > (size_t) INT_MAX) {
      EXRI_FREE(channels);
      return exri__err("image too large");
   }

   lines_per_block = exri__scanline_block_lines_for_compression(info->compression);
   if (lines_per_block <= 0) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }
   blocks = ((info->height - 1) / lines_per_block) + 1;
   if (blocks <= 0) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }
   if (info->chunk_count_found && info->chunk_count != blocks) {
      EXRI_FREE(channels);
      return exri__err("invalid chunk count");
   }
   if (info->header_end > len || ((size_t) blocks) > (len - info->header_end) / 8u) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }

   *channels_out = channels;
   if (pixel_count)
      *pixel_count = (int) total_pixels;
   if (num_blocks)
      *num_blocks = blocks;
   if (block_lines)
      *block_lines = lines_per_block;
   if (sample_size)
      *sample_size = bytes;
   return 1;
}

static int exri__deep_read_chunk_header(exri_uc const *buffer, size_t len, size_t chunk_offset, int *line_y, int *packed_offset_size, int *packed_sample_size, int *unpacked_sample_size, size_t *payload_pos)
{
   size_t pos;
   size_t packed_offsets;
   size_t packed_samples;
   size_t unpacked_samples;

   if (!exri__has_file_bytes_at(chunk_offset, len, 28))
      return exri__err_invalid();
   if (!exri__get64le_as_size_at(buffer + chunk_offset + 4, &packed_offsets) ||
       !exri__get64le_as_size_at(buffer + chunk_offset + 12, &packed_samples) ||
       !exri__get64le_as_size_at(buffer + chunk_offset + 20, &unpacked_samples))
      return exri__err_invalid();

   pos = chunk_offset + 28;
   if (packed_offsets > len - pos)
      return exri__err_invalid();
   pos += packed_offsets;
   if (packed_samples > len - pos)
      return exri__err_invalid();

   if ((packed_offset_size && packed_offsets > (size_t) INT_MAX) ||
       (packed_sample_size && packed_samples > (size_t) INT_MAX) ||
       (unpacked_sample_size && unpacked_samples > (size_t) INT_MAX))
      return exri__err_invalid();

   if (line_y)
      *line_y = exri__i32_from_u32(exri__get32le_at(buffer + chunk_offset));
   if (packed_offset_size)
      *packed_offset_size = (int) packed_offsets;
   if (packed_sample_size)
      *packed_sample_size = (int) packed_samples;
   if (unpacked_sample_size)
      *unpacked_sample_size = (int) unpacked_samples;
   if (payload_pos)
      *payload_pos = chunk_offset + 28;
   return 1;
}

static int exri__deep_build_sample_offsets(int **out_offsets, int *out_total_samples, exri_uc const *buffer, size_t len, exri__info const *info, int pixel_count, int num_blocks, int block_lines, int sample_size)
{
   int *offsets;
   exri_uc *seen_blocks;
   exri_uc *offset_bytes;
   int seen_count;
   int i;
   size_t chunk_offset;
   int line_y;
   int y_rel;
   int block_index;
   int num_lines;
   int offset_entries;
   int offset_bytes_len;
   int packed_offsets;
   int packed_samples;
   int unpacked_samples;
   size_t payload_pos;
   int entry;
   int previous;
   int current;
   int count;
   int pixel_index;
   int running;
   int p;
   size_t offset_count;

   if (out_total_samples == NULL || info == NULL || pixel_count < 0 || num_blocks <= 0 || block_lines <= 0 || sample_size <= 0)
      return exri__err("invalid argument");
   *out_total_samples = 0;
   if (out_offsets)
      *out_offsets = NULL;

   if ((size_t) pixel_count >= ((size_t) -1) / sizeof(int))
      return exri__err("image too large");
   offset_count = (size_t) pixel_count + 1u;

   offsets = (int *) EXRI_MALLOC(offset_count * sizeof(int));
   if (offsets == NULL)
      return exri__err("outofmem");
   memset(offsets, 0, offset_count * sizeof(int));

   seen_blocks = (exri_uc *) EXRI_MALLOC((size_t) num_blocks);
   if (seen_blocks == NULL) {
      EXRI_FREE(offsets);
      return exri__err("outofmem");
   }
   memset(seen_blocks, 0, (size_t) num_blocks);

   offset_bytes = NULL;
   seen_count = 0;
   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info->header_end + (size_t) i * 8u, &chunk_offset))
         goto invalid;
      if (!exri__deep_read_chunk_header(buffer, len, chunk_offset, &line_y, &packed_offsets, &packed_samples, &unpacked_samples, &payload_pos))
         goto fail;
      (void) packed_samples;

      if (line_y < info->min_y || line_y > info->max_y)
         goto invalid;
      y_rel = line_y - info->min_y;
      if ((y_rel % block_lines) != 0)
         goto invalid;
      block_index = y_rel / block_lines;
      if (block_index < 0 || block_index >= num_blocks || seen_blocks[block_index])
         goto invalid;
      seen_blocks[block_index] = 1;
      seen_count += 1;

      num_lines = block_lines;
      if (num_lines > info->height - y_rel)
         num_lines = info->height - y_rel;
      if (num_lines <= 0 || info->width > INT_MAX / num_lines)
         goto invalid;
      offset_entries = info->width * num_lines;
      if (offset_entries > INT_MAX / 4)
         goto invalid;
      offset_bytes_len = offset_entries * 4;

      offset_bytes = (exri_uc *) EXRI_MALLOC((size_t) offset_bytes_len);
      if (offset_bytes == NULL) {
         exri__err("outofmem");
         goto fail;
      }
      if (!exri__deep_decode_payload(offset_bytes, offset_bytes_len, buffer + payload_pos, packed_offsets, info->compression))
         goto fail;

      previous = 0;
      for (entry = 0; entry < offset_entries; ++entry) {
         current = exri__i32_from_u32(exri__get32le_at(offset_bytes + entry * 4));
         if (current < previous)
            goto invalid;
         count = current - previous;
         pixel_index = (y_rel + entry / info->width) * info->width + (entry % info->width);
         if (pixel_index < 0 || pixel_index >= pixel_count)
            goto invalid;
         offsets[pixel_index] = count;
         previous = current;
      }

      if (previous > INT_MAX / sample_size)
         goto invalid;
      if (unpacked_samples != previous * sample_size)
         goto invalid;

      EXRI_FREE(offset_bytes);
      offset_bytes = NULL;
   }

   if (seen_count != num_blocks)
      goto invalid;

   running = 0;
   for (p = 0; p < pixel_count; ++p) {
      count = offsets[p];
      offsets[p] = running;
      if (count < 0 || count > INT_MAX - running)
         goto invalid;
      running += count;
   }
   offsets[pixel_count] = running;
   if (EXRI_MAX_PIXELS > 0 && (size_t) running > (size_t) EXRI_MAX_PIXELS) {
      exri__err("image too large");
      goto fail;
   }

   *out_total_samples = running;
   EXRI_FREE(seen_blocks);
   if (out_offsets) {
      *out_offsets = offsets;
   } else {
      EXRI_FREE(offsets);
   }
   return 1;

invalid:
   exri__err_invalid();

fail:
   EXRI_FREE(offset_bytes);
   EXRI_FREE(seen_blocks);
   EXRI_FREE(offsets);
   if (out_offsets)
      *out_offsets = NULL;
   *out_total_samples = 0;
   return 0;
}

static int exri__deep_decode_samples(float *samples, int const *sample_offsets, exri_uc const *buffer, size_t len, exri__info const *info, exri__channel const *channels, int pixel_count, int num_blocks, int block_lines, int sample_size)
{
   exri_uc *offset_bytes;
   exri_uc *sample_bytes;
   int *channel_bases;
   int i;
   int c;
   size_t chunk_offset;
   int line_y;
   int y_rel;
   int num_lines;
   int offset_entries;
   int offset_bytes_len;
   int packed_offsets;
   int packed_samples;
   int unpacked_samples;
   size_t payload_pos;
   int previous;
   int current;
   int count;
   int chunk_samples;
   int entry;
   int pixel_index;
   int global_start;
   int channel_base;
   int s;
   int src_index;
   size_t dst_index;

   if (sample_offsets == NULL || info == NULL || channels == NULL || sample_size <= 0)
      return exri__err("invalid argument");
   if (samples == NULL && sample_offsets[pixel_count] != 0)
      return exri__err("invalid argument");

   offset_bytes = NULL;
   sample_bytes = NULL;
   channel_bases = (int *) EXRI_MALLOC((size_t) info->num_channel_records * sizeof(int));
   if (channel_bases == NULL)
      return exri__err("outofmem");

   for (i = 0; i < num_blocks; ++i) {
      if (!exri__get64le_as_size_at(buffer + info->header_end + (size_t) i * 8u, &chunk_offset))
         goto invalid;
      if (!exri__deep_read_chunk_header(buffer, len, chunk_offset, &line_y, &packed_offsets, &packed_samples, &unpacked_samples, &payload_pos))
         goto fail;

      if (line_y < info->min_y || line_y > info->max_y)
         goto invalid;
      y_rel = line_y - info->min_y;
      if ((y_rel % block_lines) != 0)
         goto invalid;
      num_lines = block_lines;
      if (num_lines > info->height - y_rel)
         num_lines = info->height - y_rel;
      if (num_lines <= 0 || info->width > INT_MAX / num_lines)
         goto invalid;
      offset_entries = info->width * num_lines;
      if (offset_entries > INT_MAX / 4)
         goto invalid;
      offset_bytes_len = offset_entries * 4;

      offset_bytes = (exri_uc *) EXRI_MALLOC((size_t) offset_bytes_len);
      if (offset_bytes == NULL) {
         exri__err("outofmem");
         goto fail;
      }
      if (!exri__deep_decode_payload(offset_bytes, offset_bytes_len, buffer + payload_pos, packed_offsets, info->compression))
         goto fail;

      previous = 0;
      for (entry = 0; entry < offset_entries; ++entry) {
         current = exri__i32_from_u32(exri__get32le_at(offset_bytes + entry * 4));
         if (current < previous)
            goto invalid;
         previous = current;
      }
      chunk_samples = previous;
      if (chunk_samples > INT_MAX / sample_size)
         goto invalid;
      if (unpacked_samples != chunk_samples * sample_size)
         goto invalid;

      if (unpacked_samples > 0) {
         sample_bytes = (exri_uc *) EXRI_MALLOC((size_t) unpacked_samples);
         if (sample_bytes == NULL) {
            exri__err("outofmem");
            goto fail;
         }
      }
      if (!exri__deep_decode_payload(sample_bytes, unpacked_samples, buffer + payload_pos + packed_offsets, packed_samples, info->compression))
         goto fail;

      channel_base = 0;
      for (c = 0; c < info->num_channel_records; ++c) {
         channel_bases[c] = channel_base;
         if (chunk_samples > 0 && channels[c].bytes_per_sample > (INT_MAX - channel_base) / chunk_samples)
            goto invalid;
         channel_base += channels[c].bytes_per_sample * chunk_samples;
      }
      if (channel_base != unpacked_samples)
         goto invalid;

      previous = 0;
      for (entry = 0; entry < offset_entries; ++entry) {
         current = exri__i32_from_u32(exri__get32le_at(offset_bytes + entry * 4));
         count = current - previous;
         pixel_index = (y_rel + entry / info->width) * info->width + (entry % info->width);
         if (pixel_index < 0 || pixel_index >= pixel_count)
            goto invalid;
         global_start = sample_offsets[pixel_index];
         if (sample_offsets[pixel_index + 1] - global_start != count)
            goto invalid;

         for (s = 0; s < count; ++s) {
            for (c = 0; c < info->num_channel_records; ++c) {
               src_index = channel_bases[c] + (previous + s) * channels[c].bytes_per_sample;
               if (src_index < 0 || src_index > unpacked_samples - channels[c].bytes_per_sample)
                  goto invalid;
               dst_index = ((size_t) (global_start + s) * (size_t) info->num_channel_records) + (size_t) c;
               samples[dst_index] = exri__read_sample(sample_bytes + src_index, channels[c].pixel_type);
            }
         }

         previous = current;
      }

      EXRI_FREE(sample_bytes);
      sample_bytes = NULL;
      EXRI_FREE(offset_bytes);
      offset_bytes = NULL;
   }

   EXRI_FREE(channel_bases);
   return 1;

invalid:
   exri__err_invalid();

fail:
   EXRI_FREE(sample_bytes);
   EXRI_FREE(offset_bytes);
   EXRI_FREE(channel_bases);
   return 0;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_info_from_memory(exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels, int *total_samples)
{
   exri__info info;
   exri__channel *channels;
   int pixel_count;
   int num_blocks;
   int block_lines;
   int sample_size;
   int samples;

   channels = NULL;
   samples = 0;
   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &num_blocks, &block_lines, &sample_size))
      return 0;

   if (!exri__deep_build_sample_offsets(NULL, &samples, buffer, len, &info, pixel_count, num_blocks, block_lines, sample_size)) {
      EXRI_FREE(channels);
      return 0;
   }

   if (x)
      *x = info.width;
   if (y)
      *y = info.height;
   if (num_channels)
      *num_channels = info.num_channel_records;
   if (total_samples)
      *total_samples = samples;
   EXRI_FREE(channels);
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_count_from_memory(exri_uc const *buffer, size_t len, int *num_channels)
{
   exri__info info;
   exri__channel *channels;
   int pixel_count;
   int num_blocks;
   int block_lines;
   int sample_size;

   channels = NULL;
   if (num_channels == NULL)
      return exri__err("invalid argument");
   *num_channels = 0;

   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &num_blocks, &block_lines, &sample_size))
      return 0;
   (void) pixel_count;
   (void) num_blocks;
   (void) block_lines;
   (void) sample_size;
   *num_channels = info.num_channel_records;
   EXRI_FREE(channels);
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_name_from_memory(exri_uc const *buffer, size_t len, int channel_index, char *name, int name_size)
{
   exri__info info;
   exri__channel *channels;
   int pixel_count;
   int num_blocks;
   int block_lines;
   int sample_size;
   int result;

   channels = NULL;
   if (channel_index < 0)
      return exri__err("invalid argument");

   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &num_blocks, &block_lines, &sample_size))
      return 0;
   (void) pixel_count;
   (void) num_blocks;
   (void) block_lines;
   (void) sample_size;
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   result = exri__copy_name_to_buffer(channels[channel_index].name, channels[channel_index].name_len, name, name_size);
   EXRI_FREE(channels);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int channel_index, int *pixel_type)
{
   exri__info info;
   exri__channel *channels;
   int pixel_count;
   int num_blocks;
   int block_lines;
   int sample_size;

   channels = NULL;
   if (channel_index < 0 || pixel_type == NULL)
      return exri__err("invalid argument");
   *pixel_type = -1;

   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &num_blocks, &block_lines, &sample_size))
      return 0;
   (void) pixel_count;
   (void) num_blocks;
   (void) block_lines;
   (void) sample_size;
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   *pixel_type = channels[channel_index].pixel_type;
   EXRI_FREE(channels);
   return 1;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri__info info;
   exri__channel *channels;
   int pixel_count;
   int num_blocks;
   int block_lines;
   int sample_size;

   channels = NULL;
   exri__clear_channel_sampling(x_sampling, y_sampling, p_linear);
   if (channel_index < 0)
      return exri__err("invalid argument");

   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &num_blocks, &block_lines, &sample_size))
      return 0;
   (void) pixel_count;
   (void) num_blocks;
   (void) block_lines;
   (void) sample_size;
   if (channel_index >= info.num_channel_records) {
      EXRI_FREE(channels);
      return exri__err("channel not found");
   }

   exri__set_channel_sampling(channels + channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(channels);
   return 1;
}

static int exri__load_deep_from_memory(float **out_samples, int **out_sample_offsets, exri_uc const *buffer, size_t len, int *x, int *y, int *num_channels, int *total_samples)
{
   exri__info info;
   exri__channel *channels;
   int *sample_offsets;
   float *samples;
   int pixel_count;
   int blocks;
   int block_lines;
   int sample_size;
   int samples_count;
   size_t float_count;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   channels = NULL;
   sample_offsets = NULL;
   samples = NULL;
   samples_count = 0;

   if (!exri__deep_prepare(buffer, len, &info, &channels, &pixel_count, &blocks, &block_lines, &sample_size))
      return 0;

   if (!exri__deep_build_sample_offsets(&sample_offsets, &samples_count, buffer, len, &info, pixel_count, blocks, block_lines, sample_size))
      goto fail;

   if (samples_count > 0) {
      if ((size_t) samples_count > ((size_t) -1) / (size_t) info.num_channel_records ||
          (size_t) samples_count * (size_t) info.num_channel_records > ((size_t) -1) / sizeof(float)) {
         exri__err("image too large");
         goto fail;
      }
      float_count = (size_t) samples_count * (size_t) info.num_channel_records;
      samples = (float *) EXRI_MALLOC(float_count * sizeof(float));
      if (samples == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   }
   if (!exri__deep_decode_samples(samples, sample_offsets, buffer, len, &info, channels, pixel_count, blocks, block_lines, sample_size))
      goto fail;

   if (x)
      *x = info.width;
   if (y)
      *y = info.height;
   if (num_channels)
      *num_channels = info.num_channel_records;
   if (total_samples)
      *total_samples = samples_count;
   *out_samples = samples;
   *out_sample_offsets = sample_offsets;
   EXRI_FREE(channels);
   return 1;

fail:
   EXRI_FREE(samples);
   EXRI_FREE(sample_offsets);
   EXRI_FREE(channels);
   *out_samples = NULL;
   *out_sample_offsets = NULL;
   return 0;
}

EXRIDEF int EXRI_CALL exri_deep_part_info_from_memory(exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_deep_info_from_memory(buffer, len, x, y, num_channels, total_samples);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_deep_info_from_memory(single, single_len, x, y, num_channels, total_samples);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_count_from_memory(exri_uc const *buffer, size_t len, int part_index, int *num_channels)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_deep_channel_count_from_memory(buffer, len, num_channels);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_deep_channel_count_from_memory(single, single_len, num_channels);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_name_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_deep_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_deep_channel_name_from_memory(single, single_len, channel_index, name, name_size);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_deep_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_deep_channel_pixel_type_from_memory(single, single_len, channel_index, pixel_type);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling_from_memory(exri_uc const *buffer, size_t len, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   exri__clear_channel_sampling(x_sampling, y_sampling, p_linear);
   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri_deep_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri_deep_channel_sampling_from_memory(single, single_len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(single);
   return result;
}

EXRIDEF int EXRI_CALL exri_load_deep_part_from_memory(float **out_samples, int **out_sample_offsets, exri_uc const *buffer, size_t len, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *single;
   size_t single_len;
   int multipart;
   int result;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   if (part_index < 0)
      return exri__err("invalid argument");
   if (!exri__is_multipart_memory(buffer, len, &multipart))
      return 0;
   if (!multipart) {
      if (!exri__single_part_index_ok(buffer, len, part_index))
         return 0;
      return exri__load_deep_from_memory(out_samples, out_sample_offsets, buffer, len, x, y, num_channels, total_samples);
   }

   single = NULL;
   single_len = 0;
   if (!exri__synthesize_multipart_part_to_memory(&single, &single_len, buffer, len, part_index))
      return 0;

   result = exri__load_deep_from_memory(out_samples, out_sample_offsets, single, single_len, x, y, num_channels, total_samples);
   EXRI_FREE(single);
   return result;
}

static void exri__put_f32le_at(exri_uc *p, float f)
{
   exri__put32le_at(p, exri__float_to_bits(f));
}

static int exri__strlen_checked(char const *s, int allow_empty, int *len_out)
{
   int len;

   if (s == NULL || len_out == NULL)
      return exri__err("invalid argument");

   len = 0;
   while (s[len] != 0) {
      if (len >= 255)
         return exri__err("name too long");
      len += 1;
   }
   if (!allow_empty && len == 0)
      return exri__err("invalid argument");

   *len_out = len;
   return 1;
}

static int exri__same_cstr(char const *a, char const *b)
{
   return strcmp(a, b) == 0;
}

static int exri__reserved_scanline_write_attribute(char const *name)
{
   return exri__same_cstr(name, "channels") ||
          exri__same_cstr(name, "compression") ||
          exri__same_cstr(name, "dataWindow") ||
          exri__same_cstr(name, "displayWindow") ||
          exri__same_cstr(name, "lineOrder") ||
          exri__same_cstr(name, "pixelAspectRatio") ||
          exri__same_cstr(name, "screenWindowCenter") ||
          exri__same_cstr(name, "screenWindowWidth") ||
          exri__same_cstr(name, "tiles") ||
          exri__same_cstr(name, "chunkCount");
}

static int exri__validate_write_attributes(exri_write_attribute const *attributes, int num_attributes)
{
   int i;
   int j;
   int name_len;
   int type_len;

   if (num_attributes < 0)
      return exri__err("invalid argument");
   if (num_attributes > 0 && attributes == NULL)
      return exri__err("invalid argument");

   for (i = 0; i < num_attributes; ++i) {
      if (!exri__strlen_checked(attributes[i].name, 0, &name_len) ||
          !exri__strlen_checked(attributes[i].type, 0, &type_len))
         return 0;
      if (attributes[i].value_size < 0)
         return exri__err("invalid argument");
      if (attributes[i].value_size > 0 && attributes[i].value == NULL)
         return exri__err("invalid argument");
      if (exri__reserved_scanline_write_attribute(attributes[i].name))
         return exri__err("reserved attribute name");
      for (j = 0; j < i; ++j) {
         if (strcmp(attributes[i].name, attributes[j].name) == 0)
            return exri__err("duplicate attribute");
      }
   }

   return 1;
}

static int exri__append_write_attributes(exri_uc **buffer, size_t *length, size_t *capacity, exri_write_attribute const *attributes, int num_attributes)
{
   int i;

   for (i = 0; i < num_attributes; ++i) {
      if (!exri__append_attr_output(buffer, length, capacity, attributes[i].name, attributes[i].type, attributes[i].value, attributes[i].value_size))
         return 0;
   }

   return 1;
}

static int exri__write_pixel_type_valid(int pixel_type)
{
   return pixel_type == EXRI_PIXEL_UINT || pixel_type == EXRI_PIXEL_HALF || pixel_type == EXRI_PIXEL_FLOAT;
}

static int exri__write_pixel_type_bytes(int pixel_type)
{
   if (pixel_type == EXRI_PIXEL_HALF)
      return 2;
   if (pixel_type == EXRI_PIXEL_UINT || pixel_type == EXRI_PIXEL_FLOAT)
      return 4;
   return 0;
}

static int exri__write_channels_bytes_per_pixel(exri_write_channel const *channels, int num_channels, int *out_bytes)
{
   int i;
   int bytes;
   int total;

   if (channels == NULL || out_bytes == NULL || num_channels <= 0)
      return exri__err("invalid argument");
   total = 0;
   for (i = 0; i < num_channels; ++i) {
      bytes = exri__write_pixel_type_bytes(channels[i].pixel_type);
      if (bytes == 0)
         return exri__err("unsupported pixel type");
      if (total > INT_MAX - bytes)
         return exri__err("output too large");
      total += bytes;
   }
   *out_bytes = total;
   return 1;
}

static int exri__write_channels_uniform_pixel_type(exri_write_channel const *channels, int num_channels, int *out_pixel_type)
{
   int i;
   int pixel_type;

   if (channels == NULL || out_pixel_type == NULL || num_channels <= 0)
      return exri__err("invalid argument");
   pixel_type = channels[0].pixel_type;
   if (!exri__write_pixel_type_valid(pixel_type))
      return exri__err("unsupported pixel type");
   for (i = 1; i < num_channels; ++i) {
      if (!exri__write_pixel_type_valid(channels[i].pixel_type))
         return exri__err("unsupported pixel type");
      if (channels[i].pixel_type != pixel_type) {
         *out_pixel_type = -1;
         return 1;
      }
   }
   *out_pixel_type = pixel_type;
   return 1;
}

static int exri__write_channels_match_pixel_type(exri_write_channel const *channels, int num_channels, int pixel_type)
{
   int i;

   if (channels == NULL || num_channels <= 0)
      return exri__err("invalid argument");
   if (!exri__write_pixel_type_valid(pixel_type))
      return exri__err("unsupported pixel type");
   for (i = 0; i < num_channels; ++i) {
      if (channels[i].pixel_type != pixel_type)
         return exri__err("unsupported storage");
   }
   return 1;
}

static int exri__make_named_chlist(exri_write_channel const *channels, int num_channels, exri_uc **out_chlist, int *out_len)
{
   exri_uc *chlist;
   size_t capacity;
   size_t length;
   int i;
   int j;
   int name_len;

   if (out_chlist == NULL || out_len == NULL)
      return exri__err("invalid argument");
   *out_chlist = NULL;
   *out_len = 0;

   if (channels == NULL || num_channels <= 0)
      return exri__err("invalid argument");

   chlist = NULL;
   capacity = 0;
   length = 0;
   for (i = 0; i < num_channels; ++i) {
      if (!exri__write_pixel_type_valid(channels[i].pixel_type)) {
         exri__err("unsupported pixel type");
         goto fail;
      }
      if (!exri__strlen_checked(channels[i].name, 0, &name_len))
         goto fail;
      for (j = 0; j < i; ++j) {
         if (strcmp(channels[i].name, channels[j].name) == 0) {
            exri__err("duplicate channel");
            goto fail;
         }
      }
      if (!exri__append_cstrn_output(&chlist, &length, &capacity, channels[i].name, name_len))
         goto fail;
      if (!exri__append_u32le_output(&chlist, &length, &capacity, (exri__uint32) channels[i].pixel_type))
         goto fail;
      if (!exri__append_u32le_output(&chlist, &length, &capacity, 0u))
         goto fail;
      if (!exri__append_u32le_output(&chlist, &length, &capacity, 1u))
         goto fail;
      if (!exri__append_u32le_output(&chlist, &length, &capacity, 1u))
         goto fail;
   }
   if (!exri__append_u8_output(&chlist, &length, &capacity, 0))
      goto fail;
   if (length > (size_t) INT_MAX) {
      exri__err("output too large");
      goto fail;
   }

   *out_chlist = chlist;
   *out_len = (int) length;
   return 1;

fail:
   EXRI_FREE(chlist);
   *out_chlist = NULL;
   *out_len = 0;
   return 0;
}

static int exri__make_chlist(exri_uc *dst, int comp, int pixel_type, int *out_len)
{
   char const *names[4];
   int count;
   int i;
   int p;

   if (comp == 1) {
      names[0] = "Y";
      count = 1;
   } else if (comp == 2) {
      names[0] = "A";
      names[1] = "Y";
      count = 2;
   } else if (comp == 3) {
      names[0] = "B";
      names[1] = "G";
      names[2] = "R";
      count = 3;
   } else if (comp == 4) {
      names[0] = "A";
      names[1] = "B";
      names[2] = "G";
      names[3] = "R";
      count = 4;
   } else {
      return 0;
   }

   p = 0;
   for (i = 0; i < count; ++i) {
      dst[p++] = (exri_uc) names[i][0];
      dst[p++] = 0;
      exri__put32le_at(dst + p, (exri__uint32) pixel_type); p += 4;
      exri__put32le_at(dst + p, 0u); p += 4;
      exri__put32le_at(dst + p, 1u); p += 4;
      exri__put32le_at(dst + p, 1u); p += 4;
   }
   dst[p++] = 0;

   *out_len = p;
   return 1;
}

static int exri__write_box2i_attr(exri_uc **buffer, size_t *length, size_t *capacity, char const *name, int min_x, int min_y, int max_x, int max_y)
{
   exri_uc value[16];

   exri__put32le_at(value + 0, (exri__uint32) min_x);
   exri__put32le_at(value + 4, (exri__uint32) min_y);
   exri__put32le_at(value + 8, (exri__uint32) max_x);
   exri__put32le_at(value + 12, (exri__uint32) max_y);
   return exri__append_attr_output(buffer, length, capacity, name, "box2i", value, 16);
}

static int exri__write_float_attr(exri_uc **buffer, size_t *length, size_t *capacity, char const *name, float f)
{
   exri_uc value[4];

   exri__put_f32le_at(value, f);
   return exri__append_attr_output(buffer, length, capacity, name, "float", value, 4);
}

static int exri__write_v2f_attr(exri_uc **buffer, size_t *length, size_t *capacity, char const *name, float x, float y)
{
   exri_uc value[8];

   exri__put_f32le_at(value + 0, x);
   exri__put_f32le_at(value + 4, y);
   return exri__append_attr_output(buffer, length, capacity, name, "v2f", value, 8);
}

static int exri__write_int_attr(exri_uc **buffer, size_t *length, size_t *capacity, char const *name, int value)
{
   exri_uc data[4];

   exri__put32le_at(data, (exri__uint32) value);
   return exri__append_attr_output(buffer, length, capacity, name, "int", data, 4);
}

static int exri__write_tiledesc_attr(exri_uc **buffer, size_t *length, size_t *capacity, int tile_width, int tile_height, int level_mode, int rounding_mode)
{
   exri_uc data[9];

   exri__put32le_at(data + 0, (exri__uint32) tile_width);
   exri__put32le_at(data + 4, (exri__uint32) tile_height);
   data[8] = (exri_uc) ((level_mode & 3) | (rounding_mode ? 16 : 0));
   return exri__append_attr_output(buffer, length, capacity, "tiles", "tiledesc", data, 9);
}

static int exri__write_chromaticities_attr(exri_uc **buffer, size_t *length, size_t *capacity)
{
   static float const srgb[8] =
   {
      0.6400f, 0.3300f,
      0.3000f, 0.6000f,
      0.1500f, 0.0600f,
      0.3127f, 0.3290f
   };
   exri_uc value[32];
   int i;

   for (i = 0; i < 8; ++i)
      exri__put_f32le_at(value + i * 4, srgb[i]);

   return exri__append_attr_output(buffer, length, capacity, "chromaticities", "chromaticities", value, 32);
}

static int exri__component_for_written_channel(int comp, int channel_index)
{
   if (comp == 1)
      return 0;
   if (comp == 2)
      return channel_index == 0 ? 1 : 0;
   if (comp == 3)
      return channel_index == 0 ? 2 : (channel_index == 1 ? 1 : 0);
   return channel_index == 0 ? 3 : (channel_index == 1 ? 2 : (channel_index == 2 ? 1 : 0));
}

static int exri__bounded_putc(char *buffer, int buffer_size, int *pos, char c)
{
   if (buffer_size > 0 && *pos < buffer_size - 1)
      buffer[*pos] = c;
   *pos += 1;
   return *pos < buffer_size;
}

static int exri__bounded_puts(char *buffer, int buffer_size, int *pos, char const *s)
{
   while (*s) {
      if (!exri__bounded_putc(buffer, buffer_size, pos, *s++))
         return 0;
   }
   return 1;
}

static int exri__bounded_putu(char *buffer, int buffer_size, int *pos, unsigned int v)
{
   char tmp[10];
   int n;

   n = 0;
   do {
      tmp[n++] = (char) ('0' + (v % 10u));
      v /= 10u;
   } while (v != 0u && n < (int) sizeof(tmp));

   while (n > 0) {
      n -= 1;
      if (!exri__bounded_putc(buffer, buffer_size, pos, tmp[n]))
         return 0;
   }
   return 1;
}

static int exri__finish_bounded_string(char *buffer, int buffer_size, int pos)
{
   if (buffer == NULL || buffer_size <= 0)
      return 0;
   if (pos < buffer_size)
      buffer[pos] = 0;
   else
      buffer[buffer_size - 1] = 0;
   return pos < buffer_size;
}

EXRIDEF int EXRI_CALL exri_format_wavelength(char *buffer, int buffer_size, float wavelength_nm)
{
   int pos;
   int whole;
   int frac;
   int scale;
   float f;

   if (buffer == NULL || buffer_size <= 0)
      return 0;

   pos = 0;
   f = wavelength_nm;
   if (!(f == f)) {
      buffer[0] = 0;
      return 0;
   }
   if (f < 0.0f) {
      if (!exri__bounded_putc(buffer, buffer_size, &pos, '-')) {
         exri__finish_bounded_string(buffer, buffer_size, pos);
         return 0;
      }
      f = -f;
   }

   if (f > 2147480000.0f) {
      buffer[0] = 0;
      return 0;
   }

   whole = (int) f;
   frac = (int) ((f - (float) whole) * 1000000.0f + 0.5f);
   if (frac >= 1000000) {
      whole += 1;
      frac -= 1000000;
   }

   if (!exri__bounded_putu(buffer, buffer_size, &pos, (unsigned int) whole) ||
       !exri__bounded_putc(buffer, buffer_size, &pos, ',')) {
      exri__finish_bounded_string(buffer, buffer_size, pos);
      return 0;
   }

   scale = 100000;
   while (scale > 0) {
      if (!exri__bounded_putc(buffer, buffer_size, &pos, (char) ('0' + (frac / scale) % 10))) {
         exri__finish_bounded_string(buffer, buffer_size, pos);
         return 0;
      }
      scale /= 10;
   }

   return exri__finish_bounded_string(buffer, buffer_size, pos);
}

EXRIDEF int EXRI_CALL exri_make_emissive_spectral_channel_name(char *buffer, int buffer_size, float wavelength_nm, int stokes_component)
{
   char wave[32];
   int pos;

   if (buffer == NULL || buffer_size <= 0 || stokes_component < 0 || stokes_component > 3)
      return 0;
   if (!exri_format_wavelength(wave, (int) sizeof(wave), wavelength_nm)) {
      buffer[0] = 0;
      return 0;
   }

   pos = 0;
   if (!exri__bounded_putc(buffer, buffer_size, &pos, 'S') ||
       !exri__bounded_putc(buffer, buffer_size, &pos, (char) ('0' + stokes_component)) ||
       !exri__bounded_putc(buffer, buffer_size, &pos, '.') ||
       !exri__bounded_puts(buffer, buffer_size, &pos, wave) ||
       !exri__bounded_puts(buffer, buffer_size, &pos, "nm")) {
      exri__finish_bounded_string(buffer, buffer_size, pos);
      return 0;
   }

   return exri__finish_bounded_string(buffer, buffer_size, pos);
}

EXRIDEF int EXRI_CALL exri_make_reflective_spectral_channel_name(char *buffer, int buffer_size, float wavelength_nm)
{
   char wave[32];
   int pos;

   if (buffer == NULL || buffer_size <= 0)
      return 0;
   if (!exri_format_wavelength(wave, (int) sizeof(wave), wavelength_nm)) {
      buffer[0] = 0;
      return 0;
   }

   pos = 0;
   if (!exri__bounded_puts(buffer, buffer_size, &pos, "T.") ||
       !exri__bounded_puts(buffer, buffer_size, &pos, wave) ||
       !exri__bounded_puts(buffer, buffer_size, &pos, "nm")) {
      exri__finish_bounded_string(buffer, buffer_size, pos);
      return 0;
   }

   return exri__finish_bounded_string(buffer, buffer_size, pos);
}

EXRIDEF float EXRI_CALL exri_parse_spectral_wavelength(char const *channel_name)
{
   char const *p;
   double value;
   double place;
   int saw_digit;

   if (channel_name == NULL)
      return -1.0f;

   p = channel_name;
   if (p[0] == 'S' && p[1] >= '0' && p[1] <= '3' && p[2] == '.') {
      p += 3;
   } else if (p[0] == 'T' && p[1] == '.') {
      p += 2;
   } else {
      return -1.0f;
   }

   value = 0.0;
   saw_digit = 0;
   while (*p >= '0' && *p <= '9') {
      value = value * 10.0 + (double) (*p - '0');
      p += 1;
      saw_digit = 1;
   }
   if (!saw_digit)
      return -1.0f;

   if (*p == ',' || *p == '.') {
      p += 1;
      place = 0.1;
      while (*p >= '0' && *p <= '9') {
         value += (double) (*p - '0') * place;
         place *= 0.1;
         p += 1;
      }
   }

   if (p[0] != 'n' || p[1] != 'm' || p[2] != 0)
      return -1.0f;
   return (float) value;
}

EXRIDEF int EXRI_CALL exri_spectral_stokes_component(char const *channel_name)
{
   if (channel_name == NULL)
      return -1;
   if (channel_name[0] != 'S' || channel_name[1] < '0' || channel_name[1] > '3' || channel_name[2] != '.')
      return -1;
   if (exri_parse_spectral_wavelength(channel_name) < 0.0f)
      return -1;
   return channel_name[1] - '0';
}

EXRIDEF int EXRI_CALL exri_is_spectral_channel_name(char const *channel_name)
{
   return exri_parse_spectral_wavelength(channel_name) >= 0.0f;
}

EXRIDEF int EXRI_CALL exri_is_spectral_from_memory(exri_uc const *buffer, size_t len)
{
   exri__info info;
   exri__channel *channels;
   exri__attribute_ref attribute;
   int ok;

   channels = NULL;
   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   EXRI_FREE(channels);
   if (info.multipart)
      return exri__err("multipart metadata unsupported");

   ok = exri__find_attribute_by_name(buffer, info.header_end, "spectralLayoutVersion", &attribute);
   if (!ok)
      return exri__err("not spectral");
   return 1;
}

EXRIDEF int EXRI_CALL exri_spectrum_type_from_memory(exri_uc const *buffer, size_t len, int *spectrum_type)
{
   exri__info info;
   exri__channel *channels;
   exri__attribute_ref attribute;
   int i;
   int has_reflective;
   int has_emissive;
   int has_stokes;

   channels = NULL;
   if (spectrum_type == NULL)
      return exri__err("invalid argument");
   *spectrum_type = -1;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   if (info.multipart) {
      EXRI_FREE(channels);
      return exri__err("multipart metadata unsupported");
   }
   if (!exri__find_attribute_by_name(buffer, info.header_end, "spectralLayoutVersion", &attribute)) {
      EXRI_FREE(channels);
      return exri__err("not spectral");
   }

   has_reflective = 0;
   has_emissive = 0;
   has_stokes = 0;
   for (i = 0; i < info.num_channel_records; ++i) {
      if (channels[i].name_len >= 2 && channels[i].name[0] == (exri_uc) 'T' && channels[i].name[1] == (exri_uc) '.')
         has_reflective = 1;
      else if (channels[i].name_len >= 3 &&
               channels[i].name[0] == (exri_uc) 'S' &&
               channels[i].name[1] >= (exri_uc) '0' &&
               channels[i].name[1] <= (exri_uc) '3' &&
               channels[i].name[2] == (exri_uc) '.') {
         has_emissive = 1;
         if (channels[i].name[1] != (exri_uc) '0')
            has_stokes = 1;
      }
   }

   EXRI_FREE(channels);
   if (has_reflective)
      *spectrum_type = EXRI_SPECTRUM_REFLECTIVE;
   else if (has_stokes)
      *spectrum_type = EXRI_SPECTRUM_POLARISED;
   else if (has_emissive)
      *spectrum_type = EXRI_SPECTRUM_EMISSIVE;
   else
      return exri__err("not spectral");

   return 1;
}

EXRIDEF int EXRI_CALL exri_spectral_wavelengths_from_memory(exri_uc const *buffer, size_t len, float *wavelengths, int max_wavelengths, int *num_wavelengths)
{
   exri__info info;
   exri__channel *channels;
   float *tmp;
   float wl;
   float diff;
   float swap;
   int count;
   int found;
   int i;
   int j;

   channels = NULL;
   tmp = NULL;
   if (num_wavelengths == NULL || max_wavelengths < 0)
      return exri__err("invalid argument");
   *num_wavelengths = 0;

   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   if (info.multipart) {
      EXRI_FREE(channels);
      return exri__err("multipart metadata unsupported");
   }
   if (info.num_channel_records <= 0) {
      EXRI_FREE(channels);
      return exri__err_invalid();
   }

   tmp = (float *) EXRI_MALLOC((size_t) info.num_channel_records * sizeof(tmp[0]));
   if (tmp == NULL) {
      EXRI_FREE(channels);
      return exri__err("outofmem");
   }

   count = 0;
   for (i = 0; i < info.num_channel_records; ++i) {
      wl = exri_parse_spectral_wavelength((char const *) channels[i].name);
      if (wl > 0.0f) {
         found = 0;
         for (j = 0; j < count; ++j) {
            diff = tmp[j] - wl;
            if (diff < 0.0f)
               diff = -diff;
            if (diff < 0.01f) {
               found = 1;
               break;
            }
         }
         if (!found)
            tmp[count++] = wl;
      }
   }

   for (i = 0; i < count - 1; ++i) {
      for (j = i + 1; j < count; ++j) {
         if (tmp[i] > tmp[j]) {
            swap = tmp[i];
            tmp[i] = tmp[j];
            tmp[j] = swap;
         }
      }
   }

   *num_wavelengths = count;
   if (wavelengths != NULL && max_wavelengths < count) {
      EXRI_FREE(tmp);
      EXRI_FREE(channels);
      return exri__err("buffer too small");
   }
   if (wavelengths != NULL) {
      for (i = 0; i < count; ++i)
         wavelengths[i] = tmp[i];
   }

   EXRI_FREE(tmp);
   EXRI_FREE(channels);
   return 1;
}

EXRIDEF int EXRI_CALL exri_spectral_units_from_memory(exri_uc const *buffer, size_t len, char *units, int units_size)
{
   exri__info info;
   exri__channel *channels;
   exri__attribute_ref attribute;
   int unit_len;
   int i;

   channels = NULL;
   if (units != NULL && units_size > 0)
      units[0] = 0;
   if (!exri__parse_header(buffer, len, &info, &channels)) {
      EXRI_FREE(channels);
      return 0;
   }
   EXRI_FREE(channels);
   if (info.multipart)
      return exri__err("multipart metadata unsupported");

   if (!exri__find_attribute_by_name(buffer, info.header_end, "ROOT/units", &attribute)) {
      if (!exri__find_attribute_by_name(buffer, info.header_end, "emissiveUnits", &attribute))
         return exri__err("spectral units not found");
   }

   unit_len = exri__attribute_string_len(&attribute);
   if (unit_len < 0)
      return exri__err_invalid();

   if (units != NULL) {
      if (units_size <= unit_len)
         return exri__err("buffer too small");
      for (i = 0; i < unit_len; ++i)
         units[i] = (char) attribute.value[i];
      units[unit_len] = 0;
   }

   return unit_len;
}

static int exri__rle_compress_bound(int src_size, int *out_bound)
{
   int packets;

   if (src_size < 0 || out_bound == NULL)
      return exri__err("invalid argument");
   packets = src_size / 127 + (src_size % 127 ? 1 : 0);
   if (src_size > INT_MAX - packets)
      return exri__err("output too large");
   *out_bound = src_size + packets;
   return 1;
}

static int exri__compress_rle_exr_block(exri_uc *dst, int dst_capacity, exri_uc *tmp, exri_uc const *src, int src_size, int *out_size)
{
   int p;
   int i;
   int run;
   int lit_start;
   int lit_len;

   p = 0;
   for (i = 0; i < src_size; i += 2)
      tmp[p++] = src[i];
   for (i = 1; i < src_size; i += 2)
      tmp[p++] = src[i];

   for (i = src_size - 1; i > 0; --i)
      tmp[i] = (exri_uc) ((int) tmp[i] - (int) tmp[i - 1] + 128);

   p = 0;
   i = 0;
   while (i < src_size) {
      run = 1;
      while (i + run < src_size && run < 128 && tmp[i + run] == tmp[i])
         run += 1;

      if (run >= 3) {
         if (!exri__has_bytes_at(p, dst_capacity, 2))
            return exri__err("output too large");
         dst[p++] = (exri_uc) (run - 1);
         dst[p++] = tmp[i];
         i += run;
      } else {
         lit_start = i;
         i += run;
         while (i < src_size) {
            run = 1;
            while (i + run < src_size && run < 128 && tmp[i + run] == tmp[i])
               run += 1;
            if (run >= 3 || i - lit_start + run > 128)
               break;
            i += run;
         }

         lit_len = i - lit_start;
         if (lit_len <= 0 || lit_len > 128)
            return exri__err("bad rle data");
         if (!exri__has_bytes_at(p, dst_capacity, lit_len + 1))
            return exri__err("output too large");
         dst[p++] = (exri_uc) (256 - lit_len);
         memcpy(dst + p, tmp + lit_start, (size_t) lit_len);
         p += lit_len;
      }
   }

   *out_size = p;
   return 1;
}

static int exri__zlib_store_bound(int src_size, int *out_bound)
{
   int blocks;
   int bound;

   if (src_size < 0)
      return exri__err("invalid argument");
   blocks = src_size / 65535 + 1;
   if (blocks > (INT_MAX - src_size - 6) / 5)
      return exri__err("output too large");
   bound = 2 + src_size + blocks * 5 + 4;
   *out_bound = bound;
   return 1;
}

static int exri__zlib_store(exri_uc *dst, int dst_capacity, exri_uc const *src, int src_size, int *out_size)
{
   exri__uint32 adler;
   int remaining;
   int src_pos;
   int p;
   int n;
   int final_block;

   if (src_size < 0)
      return exri__err("invalid argument");
   if (dst_capacity < 6)
      return exri__err("output too large");

   p = 0;
   dst[p++] = 0x78;
   dst[p++] = 0x01;

   remaining = src_size;
   src_pos = 0;
   do {
      n = remaining;
      if (n > 65535)
         n = 65535;
      final_block = remaining <= 65535;
      if (!exri__has_bytes_at(p, dst_capacity, n + 5))
         return exri__err("output too large");
      dst[p++] = (exri_uc) (final_block ? 1 : 0);
      dst[p++] = (exri_uc) (n & 255);
      dst[p++] = (exri_uc) ((n >> 8) & 255);
      dst[p++] = (exri_uc) ((~n) & 255);
      dst[p++] = (exri_uc) (((~n) >> 8) & 255);
      if (n > 0)
         memcpy(dst + p, src + src_pos, (size_t) n);
      p += n;
      src_pos += n;
      remaining -= n;
   } while (!final_block);

   if (!exri__has_bytes_at(p, dst_capacity, 4))
      return exri__err("output too large");
   adler = exri__adler32(src, src_size);
   dst[p++] = (exri_uc) ((adler >> 24) & 255u);
   dst[p++] = (exri_uc) ((adler >> 16) & 255u);
   dst[p++] = (exri_uc) ((adler >> 8) & 255u);
   dst[p++] = (exri_uc) (adler & 255u);

   *out_size = p;
   return 1;
}

static int exri__compress_zip_exr_block(exri_uc *dst, int dst_capacity, exri_uc *tmp, exri_uc const *src, int src_size, int *out_size)
{
   int p;
   int i;

   p = 0;
   for (i = 0; i < src_size; i += 2)
      tmp[p++] = src[i];
   for (i = 1; i < src_size; i += 2)
      tmp[p++] = src[i];

   for (i = src_size - 1; i > 0; --i)
      tmp[i] = (exri_uc) ((int) tmp[i] - (int) tmp[i - 1] + 128);

   return exri__zlib_store(dst, dst_capacity, tmp, src_size, out_size);
}

static exri__uint32 exri__float_bits_to_pxr24(exri__uint32 bits)
{
   exri__uint32 s;
   exri__uint32 e;
   exri__uint32 m;
   exri__uint32 i;

   s = bits & 0x80000000u;
   e = bits & 0x7f800000u;
   m = bits & 0x007fffffu;

   if (e == 0x7f800000u) {
      if (m != 0) {
         m >>= 8;
         return (s >> 8) | (e >> 8) | m | (m == 0 ? 1u : 0u);
      }
      return (s >> 8) | (e >> 8);
   }

   i = ((e | m) + (m & 0x00000080u)) >> 8;
   if (i >= 0x7f8000u)
      i = (e | m) >> 8;
   return (s >> 8) | i;
}

static int exri__pxr24_written_bytes_for_type(int pixel_type)
{
   if (pixel_type == EXRI_PIXEL_HALF)
      return 2;
   if (pixel_type == EXRI_PIXEL_UINT)
      return 4;
   return 3;
}

static int exri__pxr24_channel_pixel_type(exri_write_channel const *channels, int uniform_pixel_type, int channel_index)
{
   if (channels != NULL)
      return channels[channel_index].pixel_type;
   return uniform_pixel_type;
}

static int exri__compress_pxr24_exr_block(exri_uc *dst, int dst_capacity, exri_uc *tmp, exri_uc const *src, int width, int num_lines, int num_channels, exri_write_channel const *channels, int uniform_pixel_type, int *out_size)
{
   exri_uc const *in;
   exri_uc *out;
   exri_uc *p0;
   exri_uc *p1;
   exri_uc *p2;
   exri_uc *p3;
   exri__uint32 pixel;
   exri__uint32 previous;
   exri__uint32 diff;
   int line;
   int c;
   int x;
   int pxr24_size;
   int pxr24_bytes_per_sample;
   int pixel_type;

   if (width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("invalid argument");
   pxr24_bytes_per_sample = 0;
   for (c = 0; c < num_channels; ++c) {
      pixel_type = exri__pxr24_channel_pixel_type(channels, uniform_pixel_type, c);
      if (!exri__write_pixel_type_valid(pixel_type))
         return exri__err("invalid argument");
      if (pxr24_bytes_per_sample > INT_MAX - exri__pxr24_written_bytes_for_type(pixel_type))
         return exri__err("output too large");
      pxr24_bytes_per_sample += exri__pxr24_written_bytes_for_type(pixel_type);
   }
   if (width > INT_MAX / pxr24_bytes_per_sample)
      return exri__err("output too large");
   if (num_lines > INT_MAX / (width * pxr24_bytes_per_sample))
      return exri__err("output too large");

   in = src;
   out = tmp;
   for (line = 0; line < num_lines; ++line) {
      for (c = 0; c < num_channels; ++c) {
         pixel_type = exri__pxr24_channel_pixel_type(channels, uniform_pixel_type, c);
         if (pixel_type == EXRI_PIXEL_HALF) {
            p0 = out;
            p1 = out + width;
            out += width * 2;

            previous = 0;
            for (x = 0; x < width; ++x) {
               pixel = (exri__uint32) exri__get16le_at(in);
               in += 2;
               diff = pixel - previous;
               previous = pixel;
               p0[x] = (exri_uc) ((diff >> 8) & 255u);
               p1[x] = (exri_uc) (diff & 255u);
            }
         } else if (pixel_type == EXRI_PIXEL_UINT) {
            p0 = out;
            p1 = out + width;
            p2 = out + width * 2;
            p3 = out + width * 3;
            out += width * 4;

            previous = 0;
            for (x = 0; x < width; ++x) {
               pixel = exri__get32le_at(in);
               in += 4;
               diff = pixel - previous;
               previous = pixel;
               p0[x] = (exri_uc) ((diff >> 24) & 255u);
               p1[x] = (exri_uc) ((diff >> 16) & 255u);
               p2[x] = (exri_uc) ((diff >> 8) & 255u);
               p3[x] = (exri_uc) (diff & 255u);
            }
         } else {
            p0 = out;
            p1 = out + width;
            p2 = out + width * 2;
            out += width * 3;

            previous = 0;
            for (x = 0; x < width; ++x) {
               pixel = exri__float_bits_to_pxr24(exri__get32le_at(in));
               in += 4;
               diff = pixel - previous;
               previous = pixel;
               p0[x] = (exri_uc) ((diff >> 16) & 255u);
               p1[x] = (exri_uc) ((diff >> 8) & 255u);
               p2[x] = (exri_uc) (diff & 255u);
            }
         }
      }
   }

   pxr24_size = (int) (out - tmp);
   return exri__zlib_store(dst, dst_capacity, tmp, pxr24_size, out_size);
}

typedef struct
{
   exri_uc *data;
   int capacity;
   int byte_pos;
   int bit_count;
   int total_bits;
   unsigned int acc;
} exri__piz_bit_writer;

static int exri__piz_bit_writer_write(exri__piz_bit_writer *w, int n, exri__uint64 bits)
{
   int i;

   if (w == NULL || n < 0 || n > 58)
      return 0;
   if (w->total_bits > INT_MAX - n)
      return 0;

   for (i = n - 1; i >= 0; --i) {
      w->acc = (unsigned int) ((w->acc << 1) | (unsigned int) ((bits >> i) & 1u));
      w->bit_count += 1;
      w->total_bits += 1;
      if (w->bit_count == 8) {
         if (w->byte_pos >= w->capacity)
            return 0;
         w->data[w->byte_pos++] = (exri_uc) w->acc;
         w->acc = 0;
         w->bit_count = 0;
      }
   }

   return 1;
}

static int exri__piz_bit_writer_finish(exri__piz_bit_writer *w, int *out_bytes, int *out_bits)
{
   if (w == NULL || out_bytes == NULL || out_bits == NULL)
      return 0;
   if (w->bit_count > 0) {
      if (w->byte_pos >= w->capacity)
         return 0;
      w->data[w->byte_pos++] = (exri_uc) (w->acc << (8 - w->bit_count));
      w->acc = 0;
      w->bit_count = 0;
   }
   *out_bytes = w->byte_pos;
   *out_bits = w->total_bits;
   return 1;
}

static int exri__piz_huf_heap_less(exri__uint64 const *freq, int a, int b)
{
   if (freq[a] != freq[b])
      return freq[a] < freq[b];
   return a < b;
}

static void exri__piz_huf_heap_push(int *heap, int *count, int value, exri__uint64 const *freq)
{
   int i;
   int parent;
   int tmp;

   i = *count;
   heap[i] = value;
   *count = i + 1;

   while (i > 0) {
      parent = (i - 1) >> 1;
      if (!exri__piz_huf_heap_less(freq, heap[i], heap[parent]))
         break;
      tmp = heap[i];
      heap[i] = heap[parent];
      heap[parent] = tmp;
      i = parent;
   }
}

static int exri__piz_huf_heap_pop(int *heap, int *count, exri__uint64 const *freq)
{
   int result;
   int value;
   int i;
   int child;
   int right;

   result = heap[0];
   *count -= 1;
   value = heap[*count];
   if (*count <= 0)
      return result;

   i = 0;
   for (;;) {
      child = i * 2 + 1;
      if (child >= *count)
         break;
      right = child + 1;
      if (right < *count && exri__piz_huf_heap_less(freq, heap[right], heap[child]))
         child = right;
      if (!exri__piz_huf_heap_less(freq, heap[child], value))
         break;
      heap[i] = heap[child];
      i = child;
   }
   heap[i] = value;
   return result;
}

static int exri__piz_huf_build_enc_table(exri__uint64 *hcode, int *im_out, int *iM_out)
{
   int *hlink;
   int *heap;
   int *scode;
   int im;
   int iM;
   int nf;
   int i;
   int mm;
   int m;
   int j;
   int ok;

   hlink = NULL;
   heap = NULL;
   scode = NULL;
   ok = 0;

   if (hcode == NULL || im_out == NULL || iM_out == NULL)
      return 0;

   hlink = (int *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(hlink[0]));
   heap = (int *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(heap[0]));
   scode = (int *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(scode[0]));
   if (hlink == NULL || heap == NULL || scode == NULL) {
      exri__err("outofmem");
      goto done;
   }

   memset(scode, 0, (size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(scode[0]));

   im = 0;
   while (im < EXRI__PIZ_HUF_ENCSIZE && hcode[im] == 0)
      im += 1;
   if (im >= EXRI__PIZ_HUF_ENCSIZE - 1)
      goto done;

   nf = 0;
   iM = im;
   for (i = im; i < EXRI__PIZ_HUF_ENCSIZE - 1; ++i) {
      hlink[i] = i;
      if (hcode[i] != 0) {
         exri__piz_huf_heap_push(heap, &nf, i, hcode);
         iM = i;
      }
   }

   if (iM >= EXRI__PIZ_HUF_ENCSIZE - 1)
      goto done;

   iM += 1;
   hcode[iM] = 1;
   hlink[iM] = iM;
   exri__piz_huf_heap_push(heap, &nf, iM, hcode);

   while (nf > 1) {
      mm = exri__piz_huf_heap_pop(heap, &nf, hcode);
      m = exri__piz_huf_heap_pop(heap, &nf, hcode);
      if (hcode[m] > (((exri__uint64) -1) - hcode[mm]))
         goto done;
      hcode[m] += hcode[mm];
      exri__piz_huf_heap_push(heap, &nf, m, hcode);

      for (j = m;; j = hlink[j]) {
         scode[j] += 1;
         if (scode[j] > 58)
            goto done;
         if (hlink[j] == j) {
            hlink[j] = mm;
            break;
         }
      }

      for (j = mm;; j = hlink[j]) {
         scode[j] += 1;
         if (scode[j] > 58)
            goto done;
         if (hlink[j] == j)
            break;
      }
   }

   for (i = 0; i < EXRI__PIZ_HUF_ENCSIZE; ++i)
      hcode[i] = (exri__uint64) scode[i];

   if (!exri__piz_huf_canonical_code_table(hcode))
      goto done;

   *im_out = im;
   *iM_out = iM;
   ok = 1;

done:
   EXRI_FREE(scode);
   EXRI_FREE(heap);
   EXRI_FREE(hlink);
   return ok;
}

static int exri__piz_huf_pack_enc_table(exri__uint64 const *hcode, int im, int iM, exri_uc *dst, int dst_capacity, int *out_len)
{
   exri__piz_bit_writer w;
   int l;
   int zerun;
   int bits;

   if (hcode == NULL || dst == NULL || out_len == NULL || im < 0 || iM < im)
      return 0;

   w.data = dst;
   w.capacity = dst_capacity;
   w.byte_pos = 0;
   w.bit_count = 0;
   w.total_bits = 0;
   w.acc = 0;

   while (im <= iM) {
      l = exri__piz_huf_length(hcode[im]);
      if (l == 0) {
         zerun = 1;
         while (im < iM && zerun < EXRI__PIZ_LONGEST_LONG_RUN) {
            if (exri__piz_huf_length(hcode[im + 1]) > 0)
               break;
            im += 1;
            zerun += 1;
         }

         if (zerun >= 2) {
            if (zerun >= EXRI__PIZ_SHORTEST_LONG_RUN) {
               if (!exri__piz_bit_writer_write(&w, 6, (exri__uint64) EXRI__PIZ_LONG_ZEROCODE_RUN) ||
                   !exri__piz_bit_writer_write(&w, 8, (exri__uint64) (zerun - EXRI__PIZ_SHORTEST_LONG_RUN)))
                  return 0;
            } else {
               if (!exri__piz_bit_writer_write(&w, 6, (exri__uint64) (EXRI__PIZ_SHORT_ZEROCODE_RUN + zerun - 2)))
                  return 0;
            }
            im += 1;
            continue;
         }
      }

      if (l < 0 || l > 58)
         return 0;
      if (!exri__piz_bit_writer_write(&w, 6, (exri__uint64) l))
         return 0;
      im += 1;
   }

   return exri__piz_bit_writer_finish(&w, out_len, &bits);
}

static int exri__piz_huf_output_code(exri__uint64 code, exri__piz_bit_writer *w)
{
   return exri__piz_bit_writer_write(w, exri__piz_huf_length(code), exri__piz_huf_code(code));
}

static int exri__piz_huf_send_code(exri__uint64 symbol_code, int run_count, exri__uint64 run_code, exri__piz_bit_writer *w)
{
   int symbol_bits;
   int run_bits;
   size_t packed_bits;
   size_t repeated_bits;

   symbol_bits = exri__piz_huf_length(symbol_code);
   run_bits = exri__piz_huf_length(run_code);
   if (run_count < 0)
      return 0;
   packed_bits = (size_t) symbol_bits + (size_t) run_bits + 8u;
   repeated_bits = (size_t) symbol_bits * (size_t) run_count;
   if (packed_bits < repeated_bits) {
      return exri__piz_huf_output_code(symbol_code, w) &&
             exri__piz_huf_output_code(run_code, w) &&
             exri__piz_bit_writer_write(w, 8, (exri__uint64) run_count);
   }

   while (run_count >= 0) {
      if (!exri__piz_huf_output_code(symbol_code, w))
         return 0;
      run_count -= 1;
   }

   return 1;
}

static int exri__piz_huf_encode(exri__uint64 const *hcode, exri__uint16 const *raw, int n_raw, int rlc, exri_uc *dst, int dst_capacity, int *out_len, int *out_bits)
{
   exri__piz_bit_writer w;
   int s;
   int cs;
   int i;

   if (hcode == NULL || raw == NULL || dst == NULL || out_len == NULL || out_bits == NULL || n_raw <= 0)
      return 0;

   w.data = dst;
   w.capacity = dst_capacity;
   w.byte_pos = 0;
   w.bit_count = 0;
   w.total_bits = 0;
   w.acc = 0;

   s = raw[0];
   cs = 0;
   for (i = 1; i < n_raw; ++i) {
      if (s == raw[i] && cs < 255) {
         cs += 1;
      } else {
         if (!exri__piz_huf_send_code(hcode[s], cs, hcode[rlc], &w))
            return 0;
         cs = 0;
      }
      s = raw[i];
   }

   if (!exri__piz_huf_send_code(hcode[s], cs, hcode[rlc], &w))
      return 0;
   return exri__piz_bit_writer_finish(&w, out_len, out_bits);
}

static int EXRI__NOINLINE exri__piz_huf_compress(exri_uc *dst, int dst_capacity, exri__uint16 const *raw, int n_raw, int *out_size)
{
   exri__uint64 *hcode;
   int im;
   int iM;
   int i;
   int table_len;
   int data_len;
   int n_bits;
   int table_capacity;
   int data_start;
   int data_capacity;
   int ok;

   hcode = NULL;
   ok = 0;

   if (dst == NULL || raw == NULL || out_size == NULL || n_raw <= 0 || dst_capacity < 20)
      return 0;

   hcode = (exri__uint64 *) EXRI_MALLOC((size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(hcode[0]));
   if (hcode == NULL) {
      exri__err("outofmem");
      goto done;
   }

   memset(hcode, 0, (size_t) EXRI__PIZ_HUF_ENCSIZE * sizeof(hcode[0]));
   for (i = 0; i < n_raw; ++i)
      hcode[raw[i]] += 1;

   if (!exri__piz_huf_build_enc_table(hcode, &im, &iM))
      goto done;

   if (!exri__has_bytes_at(20, dst_capacity, 0))
      goto done;
   table_capacity = (int) ((size_t) dst_capacity - 20u);
   if (!exri__piz_huf_pack_enc_table(hcode, im, iM, dst + 20, table_capacity, &table_len))
      goto done;
   if (!exri__has_bytes_at(20, dst_capacity, table_len))
      goto done;
   data_start = (int) (20u + (size_t) table_len);
   data_capacity = (int) ((size_t) dst_capacity - (size_t) data_start);

   if (!exri__piz_huf_encode(hcode,
                              raw,
                              n_raw,
                              iM,
                              dst + data_start,
                              data_capacity,
                              &data_len,
                              &n_bits))
      goto done;
   if (data_len < 0 || !exri__has_bytes_at(data_start, dst_capacity, data_len))
      goto done;

   exri__put32le_at(dst + 0, (exri__uint32) im);
   exri__put32le_at(dst + 4, (exri__uint32) iM);
   exri__put32le_at(dst + 8, (exri__uint32) table_len);
   exri__put32le_at(dst + 12, (exri__uint32) n_bits);
   exri__put32le_at(dst + 16, 0u);
   *out_size = (int) ((size_t) data_start + (size_t) data_len);
   ok = 1;

done:
   EXRI_FREE(hcode);
   return ok;
}

static void exri__piz_bitmap_from_data(exri__uint16 const *data, int n, exri_uc *bitmap, int *min_nonzero, int *max_nonzero)
{
   int i;
   int index;

   memset(bitmap, 0, (size_t) EXRI__PIZ_BITMAP_SIZE);
   for (i = 0; i < n; ++i) {
      index = (int) (data[i] >> 3);
      bitmap[index] = (exri_uc) (bitmap[index] | (exri_uc) (1 << (data[i] & 7)));
   }

   bitmap[0] = (exri_uc) (bitmap[0] & ~1u);
   *min_nonzero = EXRI__PIZ_BITMAP_SIZE - 1;
   *max_nonzero = 0;
   for (i = 0; i < EXRI__PIZ_BITMAP_SIZE; ++i) {
      if (bitmap[i]) {
         if (*min_nonzero > i)
            *min_nonzero = i;
         if (*max_nonzero < i)
            *max_nonzero = i;
      }
   }
}

static exri__uint16 exri__piz_forward_lut_from_bitmap(exri_uc const *bitmap, exri__uint16 *lut)
{
   int i;
   int k;

   k = 0;
   for (i = 0; i < EXRI__PIZ_USHORT_RANGE; ++i) {
      if (i == 0 || (bitmap[i >> 3] & (1 << (i & 7))))
         lut[i] = (exri__uint16) k++;
      else
         lut[i] = 0;
   }

   return (exri__uint16) (k - 1);
}

static void exri__piz_wenc14(exri__uint16 a, exri__uint16 b, exri__uint16 *l, exri__uint16 *h)
{
   int as;
   int bs;

   as = exri__piz_i16_from_u16(a);
   bs = exri__piz_i16_from_u16(b);
   *l = exri__piz_u16_from_i16(exri__piz_sar1(as + bs));
   *h = exri__piz_u16_from_i16(as - bs);
}

static void exri__piz_wenc16(exri__uint16 a, exri__uint16 b, exri__uint16 *l, exri__uint16 *h)
{
   int ao;
   int m;
   int d;

   ao = ((int) a + 32768) & 65535;
   m = (ao + (int) b) >> 1;
   d = ao - (int) b;
   if (d < 0)
      m = (m + 32768) & 65535;
   *l = (exri__uint16) m;
   *h = (exri__uint16) (d & 65535);
}

static int exri__piz_wav2_encode(exri__uint16 *in, int nx, int ox, int ny, int oy, exri__uint16 mx)
{
   exri__uint16 *py;
   exri__uint16 *ey;
   exri__uint16 *px;
   exri__uint16 *ex;
   exri__uint16 *p01;
   exri__uint16 *p10;
   exri__uint16 *p11;
   exri__uint16 i00;
   exri__uint16 i01;
   exri__uint16 i10;
   exri__uint16 i11;
   int w14;
   int n;
   int p;
   int p2;
   int oy1;
   int oy2;
   int ox1;
   int ox2;
   int y_span;
   int x_span;

   if (in == NULL || nx <= 0 || ox <= 0 || ny <= 0 || oy <= 0)
      return 0;

   w14 = mx < (1 << 14);
   n = nx > ny ? ny : nx;
   p = 1;
   p2 = 2;

   while (p2 <= n) {
      py = in;
      y_span = p2 < ny ? (int) ((size_t) ny - (size_t) p2) : 0;
      if (y_span > 0 && (size_t) oy > (size_t) INT_MAX / (size_t) y_span)
         return 0;
      ey = in + (int) ((size_t) oy * (size_t) y_span);
      if ((size_t) p > (size_t) INT_MAX / (size_t) oy ||
          (size_t) p2 > (size_t) INT_MAX / (size_t) oy ||
          (size_t) p > (size_t) INT_MAX / (size_t) ox ||
          (size_t) p2 > (size_t) INT_MAX / (size_t) ox)
         return 0;
      oy1 = (int) ((size_t) oy * (size_t) p);
      oy2 = (int) ((size_t) oy * (size_t) p2);
      ox1 = (int) ((size_t) ox * (size_t) p);
      ox2 = (int) ((size_t) ox * (size_t) p2);

      for (; py <= ey; py += oy2) {
         px = py;
         x_span = p2 < nx ? (int) ((size_t) nx - (size_t) p2) : 0;
         if (x_span > 0 && (size_t) ox > (size_t) INT_MAX / (size_t) x_span)
            return 0;
         ex = py + (int) ((size_t) ox * (size_t) x_span);

         for (; px <= ex; px += ox2) {
            p01 = px + ox1;
            p10 = px + oy1;
            p11 = p10 + ox1;

            if (w14) {
               exri__piz_wenc14(*px, *p01, &i00, &i01);
               exri__piz_wenc14(*p10, *p11, &i10, &i11);
               exri__piz_wenc14(i00, i10, px, p10);
               exri__piz_wenc14(i01, i11, p01, p11);
            } else {
               exri__piz_wenc16(*px, *p01, &i00, &i01);
               exri__piz_wenc16(*p10, *p11, &i10, &i11);
               exri__piz_wenc16(i00, i10, px, p10);
               exri__piz_wenc16(i01, i11, p01, p11);
            }
         }

         if (nx & p) {
            p10 = px + oy1;
            if (w14)
               exri__piz_wenc14(*px, *p10, &i00, p10);
            else
               exri__piz_wenc16(*px, *p10, &i00, p10);
            *px = i00;
         }
      }

      if (ny & p) {
         px = py;
         x_span = p2 < nx ? (int) ((size_t) nx - (size_t) p2) : 0;
         if (x_span > 0 && (size_t) ox > (size_t) INT_MAX / (size_t) x_span)
            return 0;
         ex = py + (int) ((size_t) ox * (size_t) x_span);
         for (; px <= ex; px += ox2) {
            p01 = px + ox1;
            if (w14)
               exri__piz_wenc14(*px, *p01, &i00, p01);
            else
               exri__piz_wenc16(*px, *p01, &i00, p01);
            *px = i00;
         }
      }

      p = p2;
      if ((size_t) p2 > (size_t) INT_MAX / 2u)
         break;
      p2 = (int) ((size_t) p2 * 2u);
   }

   return 1;
}

static int exri__piz_write_channel_bytes(exri_write_channel const *channels, int uniform_pixel_type, int channel_index)
{
   int pixel_type;

   pixel_type = channels ? channels[channel_index].pixel_type : uniform_pixel_type;
   return exri__write_pixel_type_bytes(pixel_type);
}

static int exri__compress_piz_exr_block(exri_uc *dst, int dst_capacity, exri_uc const *src, int src_size, int width, int num_lines, int num_channels, exri_write_channel const *channels, int uniform_pixel_type, int *out_size)
{
   exri_uc *bitmap;
   exri__uint16 *lut;
   exri__uint16 *tmp;
   exri__piz_channel *channel_data;
   exri__uint16 max_value;
   int min_nonzero;
   int max_nonzero;
   int words_per_sample;
   int bytes_per_sample;
   int tmp_words;
   int line_words;
   int channel_words;
   int total_words;
   int pos;
   int huf_len;
   int huf_capacity;
   int huf_start;
   int bitmap_len;
   int c;
   int y;
   int j;
   int ok;
   exri_uc const *p;

   bitmap = NULL;
   lut = NULL;
   tmp = NULL;
   channel_data = NULL;
   ok = 0;

   if (dst == NULL || src == NULL || out_size == NULL || src_size <= 0 || (src_size & 1) != 0 ||
       width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("invalid argument");
   if (dst_capacity < src_size)
      return exri__err("output too large");

   tmp_words = src_size / 2;

   bitmap = (exri_uc *) EXRI_MALLOC((size_t) EXRI__PIZ_BITMAP_SIZE);
   lut = (exri__uint16 *) EXRI_MALLOC((size_t) EXRI__PIZ_USHORT_RANGE * sizeof(lut[0]));
   tmp = (exri__uint16 *) EXRI_MALLOC((size_t) tmp_words * sizeof(tmp[0]));
   channel_data = (exri__piz_channel *) EXRI_MALLOC((size_t) num_channels * sizeof(channel_data[0]));
   if (bitmap == NULL || lut == NULL || tmp == NULL || channel_data == NULL) {
      exri__err("outofmem");
      goto done;
   }

   total_words = 0;
   for (c = 0; c < num_channels; ++c) {
      bytes_per_sample = exri__piz_write_channel_bytes(channels, uniform_pixel_type, c);
      if (bytes_per_sample != 2 && bytes_per_sample != 4)
         goto bad;
      words_per_sample = bytes_per_sample / 2;
      if ((size_t) width > (size_t) INT_MAX / (size_t) words_per_sample)
         goto bad;
      line_words = (int) ((size_t) width * (size_t) words_per_sample);
      if ((size_t) num_lines > (size_t) INT_MAX / (size_t) line_words)
         goto bad;
      channel_words = (int) ((size_t) line_words * (size_t) num_lines);
      if (!exri__has_bytes_at(total_words, tmp_words, channel_words))
         goto bad;
      channel_data[c].start = tmp + total_words;
      channel_data[c].nx = width;
      channel_data[c].ny = num_lines;
      channel_data[c].size = words_per_sample;
      total_words = (int) ((size_t) total_words + (size_t) channel_words);
   }
   if (total_words != tmp_words)
      goto bad;

   p = src;
   for (y = 0; y < num_lines; ++y) {
      for (c = 0; c < num_channels; ++c) {
         line_words = width * channel_data[c].size;
         for (j = 0; j < line_words; ++j) {
            channel_data[c].start[(size_t) y * (size_t) line_words + (size_t) j] = exri__get16le_at(p);
            p += 2;
         }
      }
   }

   exri__piz_bitmap_from_data(tmp, tmp_words, bitmap, &min_nonzero, &max_nonzero);
   max_value = exri__piz_forward_lut_from_bitmap(bitmap, lut);
   exri__piz_apply_lut(lut, tmp, tmp_words);

   pos = 4;
   exri__put16le(dst + 0, (exri__uint16) min_nonzero);
   exri__put16le(dst + 2, (exri__uint16) max_nonzero);
   if (min_nonzero <= max_nonzero) {
      if (min_nonzero < 0 || max_nonzero >= EXRI__PIZ_BITMAP_SIZE)
         goto fallback;
      bitmap_len = (int) ((size_t) max_nonzero - (size_t) min_nonzero + 1u);
      if (!exri__has_bytes_at(pos, dst_capacity, bitmap_len))
         goto fallback;
      memcpy(dst + pos, bitmap + min_nonzero, (size_t) bitmap_len);
      pos = (int) ((size_t) pos + (size_t) bitmap_len);
   }

   for (c = 0; c < num_channels; ++c) {
      for (j = 0; j < channel_data[c].size; ++j) {
         if (!exri__piz_wav2_encode(channel_data[c].start + j,
                                    channel_data[c].nx,
                                    channel_data[c].size,
                                    channel_data[c].ny,
                                    (int) ((size_t) channel_data[c].nx * (size_t) channel_data[c].size),
                                    max_value))
            goto bad;
      }
   }

   if (!exri__has_bytes_at(pos, dst_capacity, 4))
      goto fallback;
   huf_start = (int) ((size_t) pos + 4u);
   huf_capacity = (int) ((size_t) dst_capacity - (size_t) pos - 4u);
   if (!exri__piz_huf_compress(dst + huf_start, huf_capacity, tmp, tmp_words, &huf_len)) {
      if (strcmp(exri_failure_reason(), "outofmem") == 0)
         goto done;
      goto bad;
   }
   if (huf_len < 0 || huf_len > huf_capacity)
      goto fallback;
   exri__put32le_at(dst + pos, (exri__uint32) huf_len);
   pos = (int) ((size_t) pos + 4u + (size_t) huf_len);

   if (pos >= src_size) {
fallback:
      memcpy(dst, src, (size_t) src_size);
      *out_size = src_size;
   } else {
      *out_size = pos;
   }
   ok = 1;
   goto done;

bad:
   exri__err("bad piz data");

done:
   EXRI_FREE(channel_data);
   EXRI_FREE(tmp);
   EXRI_FREE(lut);
   EXRI_FREE(bitmap);
   return ok;
}

static exri__uint16 exri__float_to_half_round(float f)
{
   exri__uint32 bits;
   exri__uint32 sign;
   exri__uint32 mantissa;
   int exponent;

   bits = exri__float_to_bits(f);
   sign = (bits >> 16) & 0x8000u;
   exponent = (int) ((bits >> 23) & 255u);
   mantissa = bits & 0x7fffffu;

   if (exponent == 255) {
      if (mantissa != 0) {
         mantissa >>= 13;
         return (exri__uint16) (sign | 0x7c00u | mantissa | (mantissa == 0 ? 1u : 0u));
      }
      return (exri__uint16) (sign | 0x7c00u);
   }

   exponent = exponent - 127 + 15;
   if (exponent >= 31)
      return (exri__uint16) (sign | 0x7c00u);

   if (exponent <= 0) {
      int shift;
      exri__uint32 rounded;

      if (exponent < -10)
         return (exri__uint16) sign;
      mantissa |= 0x800000u;
      shift = 14 - exponent;
      rounded = mantissa >> shift;
      if ((mantissa >> (shift - 1)) & 1u)
         rounded += 1u;
      return (exri__uint16) (sign | rounded);
   }

   mantissa += 0x1000u;
   if (mantissa & 0x800000u) {
      mantissa = 0;
      exponent += 1;
      if (exponent >= 31)
         return (exri__uint16) (sign | 0x7c00u);
   }

   return (exri__uint16) (sign | ((exri__uint32) exponent << 10) | (mantissa >> 13));
}

static exri__uint32 exri__float_to_uint_sample(float f)
{
   double d;

   d = (double) f;
   if (!(d == d) || d <= 0.0)
      return 0u;
   if (d >= 4294967295.0)
      return ~(exri__uint32) 0u;
   return (exri__uint32) d;
}

static exri__uint16 exri__b44_to_ordered(exri__uint16 h)
{
   if ((h & 0x7c00u) == 0x7c00u)
      return 0x8000u;
   if (h & 0x8000u)
      return (exri__uint16) ~h;
   return (exri__uint16) (h | 0x8000u);
}

static int exri__b44_shift_and_round(int x, int shift)
{
   int a;
   int b;

   x <<= 1;
   a = (1 << shift) - 1;
   shift += 1;
   b = (x >> shift) & 1;
   return (x + a + b) >> shift;
}

static int exri__pack_b44_block(exri_uc *out, exri__uint16 const block[16], int flatfields)
{
   int d[16];
   int r[15];
   exri__uint16 t[16];
   exri__uint16 t_max;
   int r_min;
   int r_max;
   int shift;
   int i;

   for (i = 0; i < 16; ++i)
      t[i] = exri__b44_to_ordered(block[i]);

   t_max = 0;
   for (i = 0; i < 16; ++i) {
      if (t_max < t[i])
         t_max = t[i];
   }

   shift = -1;
   do {
      shift += 1;
      for (i = 0; i < 16; ++i)
         d[i] = exri__b44_shift_and_round((int) t_max - (int) t[i], shift);

      r[0] = d[0] - d[4] + 0x20;
      r[1] = d[4] - d[8] + 0x20;
      r[2] = d[8] - d[12] + 0x20;
      r[3] = d[0] - d[1] + 0x20;
      r[4] = d[4] - d[5] + 0x20;
      r[5] = d[8] - d[9] + 0x20;
      r[6] = d[12] - d[13] + 0x20;
      r[7] = d[1] - d[2] + 0x20;
      r[8] = d[5] - d[6] + 0x20;
      r[9] = d[9] - d[10] + 0x20;
      r[10] = d[13] - d[14] + 0x20;
      r[11] = d[2] - d[3] + 0x20;
      r[12] = d[6] - d[7] + 0x20;
      r[13] = d[10] - d[11] + 0x20;
      r[14] = d[14] - d[15] + 0x20;

      r_min = r[0];
      r_max = r[0];
      for (i = 1; i < 15; ++i) {
         if (r_min > r[i])
            r_min = r[i];
         if (r_max < r[i])
            r_max = r[i];
      }
   } while (r_min < 0 || r_max > 0x3f);

   if (r_min == 0x20 && r_max == 0x20 && flatfields) {
      out[0] = (exri_uc) (t[0] >> 8);
      out[1] = (exri_uc) t[0];
      out[2] = 0xfc;
      return 3;
   }

   t[0] = (exri__uint16) (t_max - (exri__uint16) (d[0] << shift));
   out[0] = (exri_uc) (t[0] >> 8);
   out[1] = (exri_uc) t[0];
   out[2] = (exri_uc) ((shift << 2) | (r[0] >> 4));
   out[3] = (exri_uc) ((r[0] << 4) | (r[1] >> 2));
   out[4] = (exri_uc) ((r[1] << 6) | r[2]);
   out[5] = (exri_uc) ((r[3] << 2) | (r[4] >> 4));
   out[6] = (exri_uc) ((r[4] << 4) | (r[5] >> 2));
   out[7] = (exri_uc) ((r[5] << 6) | r[6]);
   out[8] = (exri_uc) ((r[7] << 2) | (r[8] >> 4));
   out[9] = (exri_uc) ((r[8] << 4) | (r[9] >> 2));
   out[10] = (exri_uc) ((r[9] << 6) | r[10]);
   out[11] = (exri_uc) ((r[11] << 2) | (r[12] >> 4));
   out[12] = (exri_uc) ((r[12] << 4) | (r[13] >> 2));
   out[13] = (exri_uc) ((r[13] << 6) | r[14]);
   return 14;
}

static int exri__compress_b44_exr_block(exri_uc *dst, int dst_capacity, exri_uc const *src, int width, int num_lines, int num_channels, int flatfields, int *out_size)
{
   exri__uint16 block[16];
   int row_bytes;
   int blocks_x;
   int blocks_y;
   int out_pos;
   int c;
   int bx;
   int by;
   int dx;
   int dy;
   int x;
   int y;
   int src_x;
   int src_y;
   int sample_pos;

   if (width <= 0 || num_lines <= 0 || num_channels <= 0)
      return exri__err("invalid argument");
   if (width > INT_MAX / (num_channels * 2))
      return exri__err("output too large");
   row_bytes = width * num_channels * 2;
   blocks_x = (width + 3) / 4;
   blocks_y = (num_lines + 3) / 4;
   out_pos = 0;

   for (c = 0; c < num_channels; ++c) {
      for (by = 0; by < blocks_y; ++by) {
         for (bx = 0; bx < blocks_x; ++bx) {
            for (dy = 0; dy < 4; ++dy) {
               y = by * 4 + dy;
               src_y = y >= num_lines ? num_lines - 1 : y;
               for (dx = 0; dx < 4; ++dx) {
                  x = bx * 4 + dx;
                  src_x = x >= width ? width - 1 : x;
                  sample_pos = src_y * row_bytes + c * width * 2 + src_x * 2;
                  block[dy * 4 + dx] = (exri__uint16) ((exri__uint16) src[sample_pos] |
                                                       (exri__uint16) ((exri__uint16) src[sample_pos + 1] << 8));
               }
            }

            if (!exri__has_bytes_at(out_pos, dst_capacity, 14))
               return exri__err("output too large");
            out_pos += exri__pack_b44_block(dst + out_pos, block, flatfields);
         }
      }
   }

   *out_size = out_pos;
   return 1;
}

static int exri__write_tiled_level_counts(int w, int h, int tile_size, int tile_level_mode, int tile_rounding_mode, int *num_x_levels_out, int *num_y_levels_out, int *num_tiles_out)
{
   int num_x_levels;
   int num_y_levels;
   int level_count_x;
   int level_count_y;
   int level_x;
   int level_y;
   int level_w;
   int level_h;
   int num_x_tiles;
   int num_y_tiles;
   int level_tiles;
   int total_tiles;
   int largest;
   int lx;
   int ly;

   if (tile_level_mode == 0) {
      num_x_levels = 1;
      num_y_levels = 1;
   } else if (tile_level_mode == 1) {
      largest = w > h ? w : h;
      num_x_levels = exri__round_log2_int(largest, tile_rounding_mode) + 1;
      num_y_levels = num_x_levels;
   } else if (tile_level_mode == 2) {
      num_x_levels = exri__round_log2_int(w, tile_rounding_mode) + 1;
      num_y_levels = exri__round_log2_int(h, tile_rounding_mode) + 1;
   } else {
      return exri__err("unsupported tiled level mode");
   }

   if (num_x_levels <= 0 || num_y_levels <= 0)
      return exri__err("unsupported tiled level mode");

   total_tiles = 0;
   level_count_y = tile_level_mode == 2 ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = tile_level_mode == 2 ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         if (!exri__tiled_level_size(w, level_x, tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(h, level_y, tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, tile_size, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, tile_size, &num_y_tiles))
            return exri__err("image too large");
         if (num_x_tiles > INT_MAX / num_y_tiles)
            return exri__err("image too large");
         level_tiles = num_x_tiles * num_y_tiles;
         if (total_tiles > INT_MAX - level_tiles)
            return exri__err("image too large");
         total_tiles += level_tiles;
      }
   }

   *num_x_levels_out = num_x_levels;
   *num_y_levels_out = num_y_levels;
   *num_tiles_out = total_tiles;
   return 1;
}

static size_t exri__write_tiled_source_offset(int w, int h, int comp, int level_w, int level_h, int x, int y, int component)
{
   exri__uint64 sx64;
   exri__uint64 sy64;
   exri__uint64 denom;
   int sx;
   int sy;

   if (level_w == w) {
      sx = x;
   } else {
      denom = (exri__uint64) level_w * 2u;
      sx64 = (((exri__uint64) x * 2u) + 1u) * (exri__uint64) w / denom;
      if (sx64 >= (exri__uint64) w)
         sx64 = (exri__uint64) (w - 1);
      sx = (int) sx64;
   }

   if (level_h == h) {
      sy = y;
   } else {
      denom = (exri__uint64) level_h * 2u;
      sy64 = (((exri__uint64) y * 2u) + 1u) * (exri__uint64) h / denom;
      if (sy64 >= (exri__uint64) h)
         sy64 = (exri__uint64) (h - 1);
      sy = (int) sy64;
   }

   return ((size_t) sy * (size_t) w + (size_t) sx) * (size_t) comp + (size_t) component;
}

static int exri__writef_tiled_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int comp, float const *data, int compression, int written_pixel_type, int tile_size, int tile_level_mode, int tile_rounding_mode)
{
   exri_uc *buffer;
   exri_uc *tile;
   exri_uc *rle_tmp;
   exri_uc *rle_data;
   exri_uc *zip_tmp;
   exri_uc *zip_data;
   exri_uc *b44_data;
   exri_uc *payload;
   exri_uc chlist[80];
   exri_uc compression_value;
   size_t capacity;
   size_t length;
   int chlist_len;
   int bytes_per_written_sample;
   int max_tile_row_bytes;
   int max_tile_block_bytes;
   int tile_row_bytes;
   int tile_block_bytes;
   int pxr24_capacity_bytes;
   int pxr24_bytes_per_sample;
   int piz_capacity_bytes;
   int b44_capacity;
   int rle_capacity;
   int zip_capacity;
   size_t offset_table_pos;
   int num_x_levels;
   int num_y_levels;
   int num_x_tiles;
   int num_y_tiles;
   int num_tiles;
   int level_count_x;
   int level_count_y;
   int lx;
   int ly;
   int level_x;
   int level_y;
   int level_w;
   int level_h;
   int tx;
   int ty;
   int tile_index;
   int pixel_x;
   int pixel_y;
   int tile_w;
   int tile_h;
   int line;
   int x;
   int c;
   int component;
   int row_pos;
   int rle_len;
   int zip_len;
   int piz_len;
   int b44_len;
   int payload_len;
   size_t src_index;

   buffer = NULL;
   tile = NULL;
   rle_tmp = NULL;
   rle_data = NULL;
   zip_tmp = NULL;
   zip_data = NULL;
   b44_data = NULL;
   capacity = 0;
   length = 0;

   if (tile_size <= 0)
      tile_size = 64;
   if (tile_size > EXRI_MAX_DIMENSIONS)
      return exri__err("image too large");
   if ((compression == EXRI_WRITE_COMPRESSION_B44 ||
        compression == EXRI_WRITE_COMPRESSION_B44A) &&
       written_pixel_type == EXRI_PIXEL_UINT)
      return exri__err("unsupported storage");
   if (!exri__write_tiled_level_counts(w, h, tile_size, tile_level_mode, tile_rounding_mode, &num_x_levels, &num_y_levels, &num_tiles))
      return 0;

   bytes_per_written_sample = written_pixel_type == EXRI_PIXEL_HALF ? 2 : 4;
   if (tile_size > INT_MAX / (comp * bytes_per_written_sample))
      return exri__err("output too large");
   max_tile_row_bytes = tile_size * comp * bytes_per_written_sample;
   if (tile_size > INT_MAX / max_tile_row_bytes)
      return exri__err("output too large");
   max_tile_block_bytes = max_tile_row_bytes * tile_size;

   pxr24_capacity_bytes = 0;
   pxr24_bytes_per_sample = 0;
   piz_capacity_bytes = 0;
   b44_capacity = 0;
   rle_capacity = 0;
   zip_capacity = 0;

   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      if (!exri__rle_compress_bound(max_tile_block_bytes, &rle_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
      if (!exri__zlib_store_bound(max_tile_block_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      if (max_tile_block_bytes > (INT_MAX - 8192) / 8)
         return exri__err("output too large");
      piz_capacity_bytes = 8192 + max_tile_block_bytes * 8;
   } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
      pxr24_bytes_per_sample = exri__pxr24_written_bytes_for_type(written_pixel_type);
      if (comp > INT_MAX / pxr24_bytes_per_sample)
         return exri__err("output too large");
      if (tile_size > INT_MAX / (comp * pxr24_bytes_per_sample))
         return exri__err("output too large");
      if (tile_size * comp * pxr24_bytes_per_sample > INT_MAX / tile_size)
         return exri__err("output too large");
      pxr24_capacity_bytes = tile_size * tile_size * comp * pxr24_bytes_per_sample;
      if (!exri__zlib_store_bound(pxr24_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      if (tile_size > INT_MAX - 3)
         return exri__err("output too large");
      b44_capacity = ((tile_size + 3) / 4) * ((tile_size + 3) / 4);
      if (b44_capacity > INT_MAX / (comp * 14))
         return exri__err("output too large");
      b44_capacity *= comp * 14;
   }

   if (!exri__make_chlist(chlist, comp, written_pixel_type, &chlist_len))
      return exri__err("invalid argument");

   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x01312f76u))
      goto fail;
   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x00000202u))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "channels", "chlist", chlist, chlist_len))
      goto fail;
   if (comp == 3 || comp == 4) {
      if (!exri__write_chromaticities_attr(&buffer, &length, &capacity))
         goto fail;
   }
   compression_value = (exri_uc) compression;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "compression", "compression", &compression_value, 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "dataWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "displayWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "lineOrder", "lineOrder", (exri_uc const *) "", 1))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "pixelAspectRatio", 1.0f))
      goto fail;
   if (!exri__write_v2f_attr(&buffer, &length, &capacity, "screenWindowCenter", 0.0f, 0.0f))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "screenWindowWidth", 1.0f))
      goto fail;
   if (!exri__write_tiledesc_attr(&buffer, &length, &capacity, tile_size, tile_size, tile_level_mode, tile_rounding_mode))
      goto fail;
   if (!exri__write_int_attr(&buffer, &length, &capacity, "chunkCount", num_tiles))
      goto fail;
   if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
      goto fail;

   offset_table_pos = length;
   for (tile_index = 0; tile_index < num_tiles; ++tile_index) {
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
   }

   tile = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
   if (tile == NULL) {
      exri__err("outofmem");
      goto fail;
   }
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      rle_tmp = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
      rle_data = (exri_uc *) EXRI_MALLOC((size_t) rle_capacity);
      if (rle_tmp == NULL || rle_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) {
      zip_tmp = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) zip_capacity);
      if (zip_tmp == NULL || zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) piz_capacity_bytes);
      if (zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      b44_data = (exri_uc *) EXRI_MALLOC((size_t) b44_capacity);
      if (b44_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   }

   tile_index = 0;
   level_count_y = tile_level_mode == 2 ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = tile_level_mode == 2 ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         if (!exri__tiled_level_size(w, level_x, tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(h, level_y, tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, tile_size, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, tile_size, &num_y_tiles)) {
            exri__err("image too large");
            goto fail;
         }
         for (ty = 0; ty < num_y_tiles; ++ty) {
            for (tx = 0; tx < num_x_tiles; ++tx) {
               pixel_x = tx * tile_size;
               pixel_y = ty * tile_size;
               tile_w = tile_size;
               tile_h = tile_size;
               if (tile_w > level_w - pixel_x)
                  tile_w = level_w - pixel_x;
               if (tile_h > level_h - pixel_y)
                  tile_h = level_h - pixel_y;
               if (tile_w <= 0 || tile_h <= 0) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_row_bytes = tile_w * comp * bytes_per_written_sample;
               tile_block_bytes = tile_row_bytes * tile_h;
               for (line = 0; line < tile_h; ++line) {
                  row_pos = line * tile_row_bytes;
                  for (c = 0; c < comp; ++c) {
                     component = exri__component_for_written_channel(comp, c);
                     for (x = 0; x < tile_w; ++x) {
                        src_index = exri__write_tiled_source_offset(w, h, comp, level_w, level_h, pixel_x + x, pixel_y + line, component);
                        if (written_pixel_type == EXRI_PIXEL_HALF) {
                           exri__put16le(tile + row_pos, exri__float_to_half_round(data[src_index]));
                           row_pos += 2;
                        } else if (written_pixel_type == EXRI_PIXEL_UINT) {
                           exri__put32le_at(tile + row_pos, exri__float_to_uint_sample(data[src_index]));
                           row_pos += 4;
                        } else {
                           exri__put_f32le_at(tile + row_pos, data[src_index]);
                           row_pos += 4;
                        }
                     }
                  }
               }

               if (!exri__has_offset_table_entry(offset_table_pos, tile_index, length)) {
                  exri__err("output too large");
                  goto fail;
               }
               exri__put64le_size_at(buffer + offset_table_pos + (size_t) tile_index * 8u, length);

               payload = tile;
               payload_len = tile_block_bytes;
               if (compression == EXRI_WRITE_COMPRESSION_RLE) {
                  if (!exri__compress_rle_exr_block(rle_data, rle_capacity, rle_tmp, tile, tile_block_bytes, &rle_len))
                     goto fail;
                  if (rle_len < tile_block_bytes) {
                     payload = rle_data;
                     payload_len = rle_len;
                  }
               } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
                  if (!exri__compress_zip_exr_block(zip_data, zip_capacity, zip_tmp, tile, tile_block_bytes, &zip_len))
                     goto fail;
                  if (zip_len < tile_block_bytes) {
                     payload = zip_data;
                     payload_len = zip_len;
                  }
               } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
                  if (!exri__compress_piz_exr_block(zip_data, piz_capacity_bytes, tile, tile_block_bytes, tile_w, tile_h, comp, NULL, written_pixel_type, &piz_len))
                     goto fail;
                  payload = zip_data;
                  payload_len = piz_len;
               } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
                  if (!exri__compress_pxr24_exr_block(zip_data, zip_capacity, zip_tmp, tile, tile_w, tile_h, comp, NULL, written_pixel_type, &zip_len))
                     goto fail;
                  payload = zip_data;
                  payload_len = zip_len;
               } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
                  if (!exri__compress_b44_exr_block(b44_data, b44_capacity, tile, tile_w, tile_h, comp, compression == EXRI_WRITE_COMPRESSION_B44A, &b44_len))
                     goto fail;
                  payload = b44_data;
                  payload_len = b44_len;
               }

               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) tx))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) ty))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) level_x))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) level_y))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) payload_len))
                  goto fail;
               if (!exri__append_output(&buffer, &length, &capacity, payload, payload_len))
                  goto fail;

               tile_index += 1;
            }
         }
      }
   }

   EXRI_FREE(b44_data);
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(tile);
   *out_data = buffer;
   *out_len = length;
   return 1;

fail:
   EXRI_FREE(b44_data);
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(tile);
   EXRI_FREE(buffer);
   *out_data = NULL;
   *out_len = 0;
   return 0;
}

static int exri__writef_channels_tiled_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int compression, int written_pixel_type, int tile_size, int tile_level_mode, int tile_rounding_mode)
{
   exri_uc *buffer;
   exri_uc *tile;
   exri_uc *rle_tmp;
   exri_uc *rle_data;
   exri_uc *zip_tmp;
   exri_uc *zip_data;
   exri_uc *b44_data;
   exri_uc *payload;
   exri_uc *chlist;
   exri_uc compression_value;
   size_t capacity;
   size_t length;
   int chlist_len;
   int bytes_per_written_pixel;
   int max_tile_row_bytes;
   int max_tile_block_bytes;
   int tile_row_bytes;
   int tile_block_bytes;
   int pxr24_capacity_bytes;
   int pxr24_bytes_per_sample;
   int piz_capacity_bytes;
   int b44_capacity;
   int rle_capacity;
   int zip_capacity;
   size_t offset_table_pos;
   int num_x_levels;
   int num_y_levels;
   int num_x_tiles;
   int num_y_tiles;
   int num_tiles;
   int level_count_x;
   int level_count_y;
   int lx;
   int ly;
   int level_x;
   int level_y;
   int level_w;
   int level_h;
   int tx;
   int ty;
   int tile_index;
   int pixel_x;
   int pixel_y;
   int tile_w;
   int tile_h;
   int line;
   int x;
   int c;
   int row_pos;
   int rle_len;
   int zip_len;
   int piz_len;
   int b44_len;
   int payload_len;
   size_t src_index;

   buffer = NULL;
   tile = NULL;
   rle_tmp = NULL;
   rle_data = NULL;
   zip_tmp = NULL;
   zip_data = NULL;
   b44_data = NULL;
   chlist = NULL;
   capacity = 0;
   length = 0;
   chlist_len = 0;

   if (tile_size <= 0)
      tile_size = 64;
   if (tile_size > EXRI_MAX_DIMENSIONS)
      return exri__err("image too large");
   if ((compression == EXRI_WRITE_COMPRESSION_B44 ||
        compression == EXRI_WRITE_COMPRESSION_B44A) &&
       !exri__write_channels_match_pixel_type(channels, num_channels, EXRI_PIXEL_HALF))
      return 0;
   if (!exri__write_tiled_level_counts(w, h, tile_size, tile_level_mode, tile_rounding_mode, &num_x_levels, &num_y_levels, &num_tiles))
      return 0;

   if (!exri__write_channels_bytes_per_pixel(channels, num_channels, &bytes_per_written_pixel))
      return 0;
   if (tile_size > INT_MAX / bytes_per_written_pixel)
      return exri__err("output too large");
   max_tile_row_bytes = tile_size * bytes_per_written_pixel;
   if (tile_size > INT_MAX / max_tile_row_bytes)
      return exri__err("output too large");
   max_tile_block_bytes = max_tile_row_bytes * tile_size;

   pxr24_capacity_bytes = 0;
   pxr24_bytes_per_sample = 0;
   piz_capacity_bytes = 0;
   b44_capacity = 0;
   rle_capacity = 0;
   zip_capacity = 0;

   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      if (!exri__rle_compress_bound(max_tile_block_bytes, &rle_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
      if (!exri__zlib_store_bound(max_tile_block_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      if (max_tile_block_bytes > (INT_MAX - 8192) / 8)
         return exri__err("output too large");
      piz_capacity_bytes = 8192 + max_tile_block_bytes * 8;
   } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
      pxr24_bytes_per_sample = 0;
      for (c = 0; c < num_channels; ++c) {
         if (pxr24_bytes_per_sample > INT_MAX - exri__pxr24_written_bytes_for_type(channels[c].pixel_type))
            return exri__err("output too large");
         pxr24_bytes_per_sample += exri__pxr24_written_bytes_for_type(channels[c].pixel_type);
      }
      if (tile_size > INT_MAX / pxr24_bytes_per_sample)
         return exri__err("output too large");
      if (tile_size * pxr24_bytes_per_sample > INT_MAX / tile_size)
         return exri__err("output too large");
      pxr24_capacity_bytes = tile_size * tile_size * pxr24_bytes_per_sample;
      if (!exri__zlib_store_bound(pxr24_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      if (tile_size > INT_MAX - 3)
         return exri__err("output too large");
      b44_capacity = ((tile_size + 3) / 4) * ((tile_size + 3) / 4);
      if (num_channels > INT_MAX / 14 || b44_capacity > INT_MAX / (num_channels * 14))
         return exri__err("output too large");
      b44_capacity *= num_channels * 14;
   }

   if (!exri__make_named_chlist(channels, num_channels, &chlist, &chlist_len))
      return 0;

   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x01312f76u))
      goto fail;
   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x00000202u))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "channels", "chlist", chlist, chlist_len))
      goto fail;
   compression_value = (exri_uc) compression;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "compression", "compression", &compression_value, 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "dataWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "displayWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "lineOrder", "lineOrder", (exri_uc const *) "", 1))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "pixelAspectRatio", 1.0f))
      goto fail;
   if (!exri__write_v2f_attr(&buffer, &length, &capacity, "screenWindowCenter", 0.0f, 0.0f))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "screenWindowWidth", 1.0f))
      goto fail;
   if (!exri__write_tiledesc_attr(&buffer, &length, &capacity, tile_size, tile_size, tile_level_mode, tile_rounding_mode))
      goto fail;
   if (!exri__write_int_attr(&buffer, &length, &capacity, "chunkCount", num_tiles))
      goto fail;
   if (!exri__append_write_attributes(&buffer, &length, &capacity, attributes, num_attributes))
      goto fail;
   if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
      goto fail;

   offset_table_pos = length;
   for (tile_index = 0; tile_index < num_tiles; ++tile_index) {
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
   }

   tile = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
   if (tile == NULL) {
      exri__err("outofmem");
      goto fail;
   }
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      rle_tmp = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
      rle_data = (exri_uc *) EXRI_MALLOC((size_t) rle_capacity);
      if (rle_tmp == NULL || rle_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) {
      zip_tmp = (exri_uc *) EXRI_MALLOC((size_t) max_tile_block_bytes);
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) zip_capacity);
      if (zip_tmp == NULL || zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) piz_capacity_bytes);
      if (zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      b44_data = (exri_uc *) EXRI_MALLOC((size_t) b44_capacity);
      if (b44_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   }

   tile_index = 0;
   level_count_y = tile_level_mode == 2 ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = tile_level_mode == 2 ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         if (!exri__tiled_level_size(w, level_x, tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(h, level_y, tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, tile_size, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, tile_size, &num_y_tiles)) {
            exri__err("image too large");
            goto fail;
         }
         for (ty = 0; ty < num_y_tiles; ++ty) {
            for (tx = 0; tx < num_x_tiles; ++tx) {
               pixel_x = tx * tile_size;
               pixel_y = ty * tile_size;
               tile_w = tile_size;
               tile_h = tile_size;
               if (tile_w > level_w - pixel_x)
                  tile_w = level_w - pixel_x;
               if (tile_h > level_h - pixel_y)
                  tile_h = level_h - pixel_y;
               if (tile_w <= 0 || tile_h <= 0) {
                  exri__err_invalid();
                  goto fail;
               }

               tile_row_bytes = tile_w * bytes_per_written_pixel;
               tile_block_bytes = tile_row_bytes * tile_h;
               for (line = 0; line < tile_h; ++line) {
                  row_pos = line * tile_row_bytes;
                  for (c = 0; c < num_channels; ++c) {
                     for (x = 0; x < tile_w; ++x) {
                        src_index = exri__write_tiled_source_offset(w, h, num_channels, level_w, level_h, pixel_x + x, pixel_y + line, c);
                        if (channels[c].pixel_type == EXRI_PIXEL_HALF) {
                           exri__put16le(tile + row_pos, exri__float_to_half_round(data[src_index]));
                           row_pos += 2;
                        } else if (channels[c].pixel_type == EXRI_PIXEL_UINT) {
                           exri__put32le_at(tile + row_pos, exri__float_to_uint_sample(data[src_index]));
                           row_pos += 4;
                        } else {
                           exri__put_f32le_at(tile + row_pos, data[src_index]);
                           row_pos += 4;
                        }
                     }
                  }
               }

               if (!exri__has_offset_table_entry(offset_table_pos, tile_index, length)) {
                  exri__err("output too large");
                  goto fail;
               }
               exri__put64le_size_at(buffer + offset_table_pos + (size_t) tile_index * 8u, length);

               payload = tile;
               payload_len = tile_block_bytes;
               if (compression == EXRI_WRITE_COMPRESSION_RLE) {
                  if (!exri__compress_rle_exr_block(rle_data, rle_capacity, rle_tmp, tile, tile_block_bytes, &rle_len))
                     goto fail;
                  if (rle_len < tile_block_bytes) {
                     payload = rle_data;
                     payload_len = rle_len;
                  }
               } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
                  if (!exri__compress_zip_exr_block(zip_data, zip_capacity, zip_tmp, tile, tile_block_bytes, &zip_len))
                     goto fail;
                  if (zip_len < tile_block_bytes) {
                     payload = zip_data;
                     payload_len = zip_len;
                  }
               } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
                  if (!exri__compress_piz_exr_block(zip_data, piz_capacity_bytes, tile, tile_block_bytes, tile_w, tile_h, num_channels, channels, written_pixel_type, &piz_len))
                     goto fail;
                  payload = zip_data;
                  payload_len = piz_len;
               } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
                  if (!exri__compress_pxr24_exr_block(zip_data, zip_capacity, zip_tmp, tile, tile_w, tile_h, num_channels, channels, written_pixel_type, &zip_len))
                     goto fail;
                  payload = zip_data;
                  payload_len = zip_len;
               } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
                  if (!exri__compress_b44_exr_block(b44_data, b44_capacity, tile, tile_w, tile_h, num_channels, compression == EXRI_WRITE_COMPRESSION_B44A, &b44_len))
                     goto fail;
                  payload = b44_data;
                  payload_len = b44_len;
               }

               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) tx))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) ty))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) level_x))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) level_y))
                  goto fail;
               if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) payload_len))
                  goto fail;
               if (!exri__append_output(&buffer, &length, &capacity, payload, payload_len))
                  goto fail;

               tile_index += 1;
            }
         }
      }
   }

   EXRI_FREE(b44_data);
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(tile);
   EXRI_FREE(chlist);
   *out_data = buffer;
   *out_len = length;
   return 1;

fail:
   EXRI_FREE(b44_data);
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(tile);
   EXRI_FREE(chlist);
   EXRI_FREE(buffer);
   *out_data = NULL;
   *out_len = 0;
   return 0;
}

static int exri__write_options_to_flags(exri_write_options const *options, int allow_pixel_type, int *flags_out)
{
   int compression;
   int pixel_type;
   int tiled;
   int tile_size;
   int level_mode;
   int level_rounding;
   int flags;

   if (flags_out == NULL)
      return exri__err("invalid argument");

   compression = EXRI_WRITE_COMPRESSION_NONE;
   pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   tiled = 0;
   tile_size = 0;
   level_mode = EXRI_WRITE_LEVEL_ONE;
   level_rounding = EXRI_WRITE_ROUND_DOWN;

   if (options != NULL) {
      compression = options->compression;
      pixel_type = options->pixel_type;
      tiled = options->tiled;
      tile_size = options->tile_size;
      level_mode = options->level_mode;
      level_rounding = options->level_rounding;
   }

   if (compression < EXRI_WRITE_COMPRESSION_NONE || compression > EXRI_WRITE_COMPRESSION_B44A)
      return exri__err("unsupported compression");

   flags = compression;
   if (pixel_type == EXRI_WRITE_PIXEL_FLOAT) {
      /* default */
   } else if (pixel_type == EXRI_WRITE_PIXEL_HALF) {
      if (!allow_pixel_type)
         return exri__err("unsupported write options");
      flags |= EXRI__WRITE_STORAGE_HALF;
   } else if (pixel_type == EXRI_WRITE_PIXEL_UINT) {
      if (!allow_pixel_type)
         return exri__err("unsupported write options");
      flags |= EXRI__WRITE_STORAGE_UINT;
   } else {
      return exri__err("unsupported pixel type");
   }

   if (tiled != 0) {
      flags |= EXRI__WRITE_TILED;
      if (tile_size < 0)
         return exri__err("invalid argument");
      if (tile_size > 0) {
         if (tile_size > (EXRI__WRITE_TILE_SIZE_MASK >> EXRI__WRITE_TILE_SIZE_SHIFT))
            return exri__err("unsupported write options");
         flags |= tile_size << EXRI__WRITE_TILE_SIZE_SHIFT;
      }
   } else if (tile_size != 0 || level_mode != EXRI_WRITE_LEVEL_ONE || level_rounding != EXRI_WRITE_ROUND_DOWN) {
      return exri__err("unsupported write options");
   }

   if (level_mode == EXRI_WRITE_LEVEL_ONE) {
      /* default */
   } else if (level_mode == EXRI_WRITE_LEVEL_MIPMAP) {
      flags |= EXRI__WRITE_TILED_MIPMAP;
   } else if (level_mode == EXRI_WRITE_LEVEL_RIPMAP) {
      flags |= EXRI__WRITE_TILED_RIPMAP;
   } else {
      return exri__err("unsupported tiled level mode");
   }

   if (level_rounding == EXRI_WRITE_ROUND_DOWN) {
      /* default */
   } else if (level_rounding == EXRI_WRITE_ROUND_UP) {
      if (level_mode == EXRI_WRITE_LEVEL_ONE)
         return exri__err("unsupported write options");
      flags |= EXRI__WRITE_TILED_ROUND_UP;
   } else {
      return exri__err("unsupported tiled rounding mode");
   }

   *flags_out = flags;
   return 1;
}

static int exri__writef_channels_to_memory_flags(exri_uc **out_data, size_t *out_len, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, int compression)
{
   exri_uc *buffer;
   exri_uc *row;
   exri_uc *rle_tmp;
   exri_uc *rle_data;
   exri_uc *zip_tmp;
   exri_uc *zip_data;
   exri_uc *b44_data;
   exri_uc *payload;
   exri_uc *chlist;
   exri_uc compression_value;
   size_t capacity;
   size_t length;
   int chlist_len;
   int row_bytes;
   int bytes_per_written_pixel;
   int compression_code;
   exri__uint32 compression_bits;
   exri__uint32 compression_flags;
   exri__uint32 unsupported_flags;
   int tiled;
   int tile_size;
   int written_pixel_type;
   int block_lines;
   int num_blocks;
   int block_bytes;
   int block_capacity_bytes;
   int piz_capacity_bytes;
   int pxr24_capacity_bytes;
   int pxr24_bytes_per_sample;
   int b44_capacity;
   int rle_capacity;
   int zip_capacity;
   size_t offset_table_pos;
   int block_index;
   int line;
   int num_lines;
   int y;
   int x;
   int c;
   int row_pos;
   int rle_len;
   int zip_len;
   int piz_len;
   int b44_len;
   int payload_len;
   size_t src_index;

   if (out_data == NULL || out_len == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_data = NULL;
   *out_len = 0;

   if (data == NULL || channels == NULL || w <= 0 || h <= 0 || num_channels <= 0)
      return exri__err("invalid argument");
   if (!exri__validate_write_attributes(attributes, num_attributes))
      return 0;
   if (!exri__write_channels_uniform_pixel_type(channels, num_channels, &written_pixel_type))
      return 0;

   compression_bits = (exri__uint32) compression;
   compression_code = (int) (compression_bits & (exri__uint32) EXRI__WRITE_COMPRESSION_MASK);
   compression_flags = compression_bits & ~((exri__uint32) EXRI__WRITE_COMPRESSION_MASK);
   unsupported_flags = compression_flags & ~((exri__uint32) EXRI__WRITE_STORAGE_HALF |
                                             (exri__uint32) EXRI__WRITE_STORAGE_UINT |
                                             (exri__uint32) EXRI__WRITE_TILED |
                                             (exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                                             (exri__uint32) EXRI__WRITE_TILED_RIPMAP |
                                             (exri__uint32) EXRI__WRITE_TILED_ROUND_UP |
                                             (exri__uint32) EXRI__WRITE_TILE_SIZE_MASK);
   if (unsupported_flags != 0)
      return exri__err("unsupported write flags");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_HALF) &&
       (compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT))
      return exri__err("unsupported write flags");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_TILED_MIPMAP) &&
       (compression_flags & (exri__uint32) EXRI__WRITE_TILED_RIPMAP))
      return exri__err("unsupported write flags");
   tiled = (compression_flags & (exri__uint32) EXRI__WRITE_TILED) ? 1 : 0;
   tile_size = (int) ((compression_flags & (exri__uint32) EXRI__WRITE_TILE_SIZE_MASK) >> EXRI__WRITE_TILE_SIZE_SHIFT);
   if (!tiled && (tile_size != 0 ||
                  (compression_flags & ((exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                                        (exri__uint32) EXRI__WRITE_TILED_RIPMAP |
                                        (exri__uint32) EXRI__WRITE_TILED_ROUND_UP)) != 0))
      return exri__err("unsupported write flags");
   if (tiled &&
       (compression_flags & (exri__uint32) EXRI__WRITE_TILED_ROUND_UP) &&
       (compression_flags & ((exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                             (exri__uint32) EXRI__WRITE_TILED_RIPMAP)) == 0)
      return exri__err("unsupported write flags");
   compression = compression_code;

   if (compression != EXRI_WRITE_COMPRESSION_NONE &&
       compression != EXRI_WRITE_COMPRESSION_RLE &&
       compression != EXRI_WRITE_COMPRESSION_ZIPS &&
       compression != EXRI_WRITE_COMPRESSION_ZIP &&
       compression != EXRI_WRITE_COMPRESSION_PIZ &&
       compression != EXRI_WRITE_COMPRESSION_PXR24 &&
       compression != EXRI_WRITE_COMPRESSION_B44 &&
       compression != EXRI_WRITE_COMPRESSION_B44A)
      return exri__err("unsupported compression");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT) &&
       (compression == EXRI_WRITE_COMPRESSION_B44 ||
        compression == EXRI_WRITE_COMPRESSION_B44A))
      return exri__err("unsupported storage");
   if (w > EXRI_MAX_DIMENSIONS || h > EXRI_MAX_DIMENSIONS)
      return exri__err("image too large");
   if (EXRI_MAX_PIXELS > 0 && (size_t) w > (size_t) EXRI_MAX_PIXELS / (size_t) h)
      return exri__err("image too large");
   if ((size_t) w > ((size_t) -1) / (size_t) h ||
       (size_t) w * (size_t) h > ((size_t) -1) / (size_t) num_channels)
      return exri__err("image too large");

   if (compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT) {
      if (!exri__write_channels_match_pixel_type(channels, num_channels, EXRI_PIXEL_UINT))
         return 0;
      written_pixel_type = EXRI_PIXEL_UINT;
   } else if (compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_HALF) {
      if (!exri__write_channels_match_pixel_type(channels, num_channels, EXRI_PIXEL_HALF))
         return 0;
      written_pixel_type = EXRI_PIXEL_HALF;
   }
   if ((compression == EXRI_WRITE_COMPRESSION_B44 ||
        compression == EXRI_WRITE_COMPRESSION_B44A) &&
       !exri__write_channels_match_pixel_type(channels, num_channels, EXRI_PIXEL_HALF))
      return 0;
   if (tiled)
      return exri__writef_channels_tiled_to_memory(out_data, out_len, w, h, num_channels, data, channels, attributes, num_attributes, compression, written_pixel_type, tile_size,
                                                  (compression_flags & (exri__uint32) EXRI__WRITE_TILED_RIPMAP) ? 2 : ((compression_flags & (exri__uint32) EXRI__WRITE_TILED_MIPMAP) ? 1 : 0),
                                                  (compression_flags & (exri__uint32) EXRI__WRITE_TILED_ROUND_UP) ? 1 : 0);
   if (!exri__write_channels_bytes_per_pixel(channels, num_channels, &bytes_per_written_pixel))
      return 0;
   if (w > INT_MAX / bytes_per_written_pixel)
      return exri__err("output too large");

   row_bytes = w * bytes_per_written_pixel;
   if (row_bytes > INT_MAX - 8)
      return exri__err("output too large");
   block_lines = (compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) ? 16 : ((compression == EXRI_WRITE_COMPRESSION_PIZ || compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) ? 32 : 1);
   num_blocks = ((h - 1) / block_lines) + 1;
   if (row_bytes > INT_MAX / block_lines)
      return exri__err("output too large");
   block_capacity_bytes = row_bytes * block_lines;

   piz_capacity_bytes = 0;
   pxr24_capacity_bytes = 0;
   pxr24_bytes_per_sample = 0;
   rle_capacity = 0;
   zip_capacity = 0;
   b44_capacity = 0;
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      if (!exri__rle_compress_bound(row_bytes, &rle_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
      if (!exri__zlib_store_bound(block_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      if (block_capacity_bytes > (INT_MAX - 8192) / 8)
         return exri__err("output too large");
      piz_capacity_bytes = 8192 + block_capacity_bytes * 8;
   } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
      pxr24_bytes_per_sample = 0;
      for (c = 0; c < num_channels; ++c) {
         if (pxr24_bytes_per_sample > INT_MAX - exri__pxr24_written_bytes_for_type(channels[c].pixel_type))
            return exri__err("output too large");
         pxr24_bytes_per_sample += exri__pxr24_written_bytes_for_type(channels[c].pixel_type);
      }
      if (w > INT_MAX / pxr24_bytes_per_sample)
         return exri__err("output too large");
      if (w * pxr24_bytes_per_sample > INT_MAX / block_lines)
         return exri__err("output too large");
      pxr24_capacity_bytes = w * pxr24_bytes_per_sample * block_lines;
      if (!exri__zlib_store_bound(pxr24_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      if (w > INT_MAX - 3 || num_blocks < 0)
         return exri__err("output too large");
      b44_capacity = ((w + 3) / 4) * ((block_lines + 3) / 4);
      if (num_channels > INT_MAX / 14 || b44_capacity > INT_MAX / (num_channels * 14))
         return exri__err("output too large");
      b44_capacity *= num_channels * 14;
   }

   chlist = NULL;
   chlist_len = 0;
   if (!exri__make_named_chlist(channels, num_channels, &chlist, &chlist_len))
      return 0;

   buffer = NULL;
   capacity = 0;
   length = 0;
   row = NULL;
   rle_tmp = NULL;
   rle_data = NULL;
   zip_tmp = NULL;
   zip_data = NULL;
   b44_data = NULL;

   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x01312f76u))
      goto fail;
   if (!exri__append_u32le_output(&buffer, &length, &capacity, 2u))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "channels", "chlist", chlist, chlist_len))
      goto fail;
   compression_value = (exri_uc) compression;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "compression", "compression", &compression_value, 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "dataWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "displayWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "lineOrder", "lineOrder", (exri_uc const *) "", 1))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "pixelAspectRatio", 1.0f))
      goto fail;
   if (!exri__write_v2f_attr(&buffer, &length, &capacity, "screenWindowCenter", 0.0f, 0.0f))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "screenWindowWidth", 1.0f))
      goto fail;
   if (!exri__append_write_attributes(&buffer, &length, &capacity, attributes, num_attributes))
      goto fail;
   if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
      goto fail;

   offset_table_pos = length;
   for (block_index = 0; block_index < num_blocks; ++block_index) {
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
   }

   row = (exri_uc *) EXRI_MALLOC((size_t) block_capacity_bytes);
   if (row == NULL) {
      exri__err("outofmem");
      goto fail;
   }
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      rle_tmp = (exri_uc *) EXRI_MALLOC((size_t) row_bytes);
      rle_data = (exri_uc *) EXRI_MALLOC((size_t) rle_capacity);
      if (rle_tmp == NULL || rle_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) {
      zip_tmp = (exri_uc *) EXRI_MALLOC((size_t) block_capacity_bytes);
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) zip_capacity);
      if (zip_tmp == NULL || zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) piz_capacity_bytes);
      if (zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      b44_data = (exri_uc *) EXRI_MALLOC((size_t) b44_capacity);
      if (b44_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   }

   for (block_index = 0; block_index < num_blocks; ++block_index) {
      y = block_index * block_lines;
      num_lines = block_lines;
      if (num_lines > h - y)
         num_lines = h - y;
      block_bytes = row_bytes * num_lines;

      for (line = 0; line < num_lines; ++line) {
         row_pos = line * row_bytes;
         for (c = 0; c < num_channels; ++c) {
            for (x = 0; x < w; ++x) {
               src_index = ((size_t) (y + line) * (size_t) w + (size_t) x) * (size_t) num_channels + (size_t) c;
               if (channels[c].pixel_type == EXRI_PIXEL_HALF) {
                  exri__put16le(row + row_pos, exri__float_to_half_round(data[src_index]));
                  row_pos += 2;
               } else if (channels[c].pixel_type == EXRI_PIXEL_UINT) {
                  exri__put32le_at(row + row_pos, exri__float_to_uint_sample(data[src_index]));
                  row_pos += 4;
               } else {
                  exri__put_f32le_at(row + row_pos, data[src_index]);
                  row_pos += 4;
               }
            }
         }
      }

      if (!exri__has_offset_table_entry(offset_table_pos, block_index, length)) {
         exri__err("output too large");
         goto fail;
      }
      exri__put64le_size_at(buffer + offset_table_pos + (size_t) block_index * 8u, length);

      payload = row;
      payload_len = block_bytes;
      if (compression == EXRI_WRITE_COMPRESSION_RLE) {
         if (!exri__compress_rle_exr_block(rle_data, rle_capacity, rle_tmp, row, row_bytes, &rle_len))
            goto fail;
         if (rle_len < row_bytes) {
            payload = rle_data;
            payload_len = rle_len;
         }
      } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
         if (!exri__compress_zip_exr_block(zip_data, zip_capacity, zip_tmp, row, block_bytes, &zip_len))
            goto fail;
         if (zip_len < block_bytes) {
            payload = zip_data;
            payload_len = zip_len;
         }
      } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
         if (!exri__compress_piz_exr_block(zip_data, piz_capacity_bytes, row, block_bytes, w, num_lines, num_channels, channels, written_pixel_type, &piz_len))
            goto fail;
         payload = zip_data;
         payload_len = piz_len;
      } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
         if (!exri__compress_pxr24_exr_block(zip_data, zip_capacity, zip_tmp, row, w, num_lines, num_channels, channels, written_pixel_type, &zip_len))
            goto fail;
         payload = zip_data;
         payload_len = zip_len;
      } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
         if (!exri__compress_b44_exr_block(b44_data, b44_capacity, row, w, num_lines, num_channels, compression == EXRI_WRITE_COMPRESSION_B44A, &b44_len))
            goto fail;
         payload = b44_data;
         payload_len = b44_len;
      }

      if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) y))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) payload_len))
         goto fail;
      if (!exri__append_output(&buffer, &length, &capacity, payload, payload_len))
         goto fail;
   }

   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(b44_data);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(row);
   EXRI_FREE(chlist);
   *out_data = buffer;
   *out_len = length;
   return 1;

fail:
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(b44_data);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(row);
   EXRI_FREE(chlist);
   EXRI_FREE(buffer);
   *out_data = NULL;
   *out_len = 0;
   return 0;
}

static int exri__writef_to_memory_flags(exri_uc **out_data, size_t *out_len, int w, int h, int comp, float const *data, int compression)
{
   exri_uc *buffer;
   exri_uc *row;
   exri_uc *rle_tmp;
   exri_uc *rle_data;
   exri_uc *zip_tmp;
   exri_uc *zip_data;
   exri_uc *b44_data;
   exri_uc *payload;
   exri_uc chlist[80];
   exri_uc compression_value;
   size_t capacity;
   size_t length;
   int chlist_len;
   int row_bytes;
   int bytes_per_written_sample;
   int compression_code;
   exri__uint32 compression_bits;
   exri__uint32 compression_flags;
   exri__uint32 unsupported_flags;
   int tiled;
   int tile_size;
   int written_pixel_type;
   int block_lines;
   int num_blocks;
   int block_bytes;
   int block_capacity_bytes;
   int piz_capacity_bytes;
   int pxr24_capacity_bytes;
   int pxr24_bytes_per_sample;
   int b44_capacity;
   int rle_capacity;
   int zip_capacity;
   size_t offset_table_pos;
   int block_index;
   int line;
   int num_lines;
   int y;
   int x;
   int c;
   int component;
   int row_pos;
   int rle_len;
   int zip_len;
   int piz_len;
   int b44_len;
   int payload_len;
   size_t src_index;

   if (out_data == NULL || out_len == NULL) {
      exri__err("invalid argument");
      return 0;
   }

   *out_data = NULL;
   *out_len = 0;

   if (data == NULL || w <= 0 || h <= 0 || comp < 1 || comp > 4)
      return exri__err("invalid argument");
   compression_bits = (exri__uint32) compression;
   compression_code = (int) (compression_bits & (exri__uint32) EXRI__WRITE_COMPRESSION_MASK);
   compression_flags = compression_bits & ~((exri__uint32) EXRI__WRITE_COMPRESSION_MASK);
   unsupported_flags = compression_flags & ~((exri__uint32) EXRI__WRITE_STORAGE_HALF |
                                             (exri__uint32) EXRI__WRITE_STORAGE_UINT |
                                             (exri__uint32) EXRI__WRITE_TILED |
                                             (exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                                             (exri__uint32) EXRI__WRITE_TILED_RIPMAP |
                                             (exri__uint32) EXRI__WRITE_TILED_ROUND_UP |
                                             (exri__uint32) EXRI__WRITE_TILE_SIZE_MASK);
   if (unsupported_flags != 0)
      return exri__err("unsupported write flags");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_HALF) &&
       (compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT))
      return exri__err("unsupported write flags");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_TILED_MIPMAP) &&
       (compression_flags & (exri__uint32) EXRI__WRITE_TILED_RIPMAP))
      return exri__err("unsupported write flags");
   tiled = (compression_flags & (exri__uint32) EXRI__WRITE_TILED) ? 1 : 0;
   tile_size = (int) ((compression_flags & (exri__uint32) EXRI__WRITE_TILE_SIZE_MASK) >> EXRI__WRITE_TILE_SIZE_SHIFT);
   if (!tiled && (tile_size != 0 ||
                  (compression_flags & ((exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                                        (exri__uint32) EXRI__WRITE_TILED_RIPMAP |
                                        (exri__uint32) EXRI__WRITE_TILED_ROUND_UP)) != 0))
      return exri__err("unsupported write flags");
   if (tiled &&
       (compression_flags & (exri__uint32) EXRI__WRITE_TILED_ROUND_UP) &&
       (compression_flags & ((exri__uint32) EXRI__WRITE_TILED_MIPMAP |
                             (exri__uint32) EXRI__WRITE_TILED_RIPMAP)) == 0)
      return exri__err("unsupported write flags");
   compression = compression_code;

   if (compression != EXRI_WRITE_COMPRESSION_NONE &&
       compression != EXRI_WRITE_COMPRESSION_RLE &&
       compression != EXRI_WRITE_COMPRESSION_ZIPS &&
       compression != EXRI_WRITE_COMPRESSION_ZIP &&
       compression != EXRI_WRITE_COMPRESSION_PIZ &&
       compression != EXRI_WRITE_COMPRESSION_PXR24 &&
       compression != EXRI_WRITE_COMPRESSION_B44 &&
       compression != EXRI_WRITE_COMPRESSION_B44A)
      return exri__err("unsupported compression");
   if ((compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT) &&
       (compression == EXRI_WRITE_COMPRESSION_B44 ||
        compression == EXRI_WRITE_COMPRESSION_B44A))
      return exri__err("unsupported storage");
   if (w > EXRI_MAX_DIMENSIONS || h > EXRI_MAX_DIMENSIONS)
      return exri__err("image too large");
   if (EXRI_MAX_PIXELS > 0 && (size_t) w > (size_t) EXRI_MAX_PIXELS / (size_t) h)
      return exri__err("image too large");
   if ((size_t) w > ((size_t) -1) / (size_t) h ||
       (size_t) w * (size_t) h > ((size_t) -1) / (size_t) comp)
      return exri__err("image too large");
   if (compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_UINT)
      written_pixel_type = EXRI_PIXEL_UINT;
   else
      written_pixel_type = ((compression_flags & (exri__uint32) EXRI__WRITE_STORAGE_HALF) || compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) ? EXRI_PIXEL_HALF : EXRI_PIXEL_FLOAT;
   if (tiled)
      return exri__writef_tiled_to_memory(out_data, out_len, w, h, comp, data, compression, written_pixel_type, tile_size,
                                          (compression_flags & (exri__uint32) EXRI__WRITE_TILED_RIPMAP) ? 2 : ((compression_flags & (exri__uint32) EXRI__WRITE_TILED_MIPMAP) ? 1 : 0),
                                          (compression_flags & (exri__uint32) EXRI__WRITE_TILED_ROUND_UP) ? 1 : 0);
   bytes_per_written_sample = written_pixel_type == EXRI_PIXEL_HALF ? 2 : 4;
   if (w > INT_MAX / (comp * bytes_per_written_sample))
      return exri__err("output too large");

   row_bytes = w * comp * bytes_per_written_sample;
   if (row_bytes > INT_MAX - 8)
      return exri__err("output too large");
   block_lines = (compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) ? 16 : ((compression == EXRI_WRITE_COMPRESSION_PIZ || compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) ? 32 : 1);
   num_blocks = ((h - 1) / block_lines) + 1;
   if (row_bytes > INT_MAX / block_lines)
      return exri__err("output too large");
   block_capacity_bytes = row_bytes * block_lines;
   piz_capacity_bytes = 0;
   pxr24_capacity_bytes = 0;
   pxr24_bytes_per_sample = 0;
   rle_capacity = 0;
   zip_capacity = 0;
   b44_capacity = 0;
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      if (!exri__rle_compress_bound(row_bytes, &rle_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
      if (!exri__zlib_store_bound(block_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      if (block_capacity_bytes > (INT_MAX - 8192) / 8)
         return exri__err("output too large");
      piz_capacity_bytes = 8192 + block_capacity_bytes * 8;
   } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
      pxr24_bytes_per_sample = exri__pxr24_written_bytes_for_type(written_pixel_type);
      if (comp > INT_MAX / pxr24_bytes_per_sample)
         return exri__err("output too large");
      if (w > INT_MAX / (comp * pxr24_bytes_per_sample))
         return exri__err("output too large");
      if (w * comp * pxr24_bytes_per_sample > INT_MAX / block_lines)
         return exri__err("output too large");
      pxr24_capacity_bytes = w * comp * pxr24_bytes_per_sample * block_lines;
      if (!exri__zlib_store_bound(pxr24_capacity_bytes, &zip_capacity))
         return 0;
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      if (w > INT_MAX - 3 || num_blocks < 0)
         return exri__err("output too large");
      b44_capacity = ((w + 3) / 4) * ((block_lines + 3) / 4);
      if (b44_capacity > INT_MAX / (comp * 14))
         return exri__err("output too large");
      b44_capacity *= comp * 14;
   }

   if (!exri__make_chlist(chlist, comp, written_pixel_type, &chlist_len))
      return exri__err("invalid argument");

   buffer = NULL;
   capacity = 0;
   length = 0;
   row = NULL;
   rle_tmp = NULL;
   rle_data = NULL;
   zip_tmp = NULL;
   zip_data = NULL;
   b44_data = NULL;

   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x01312f76u))
      goto fail;
   if (!exri__append_u32le_output(&buffer, &length, &capacity, 2u))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "channels", "chlist", chlist, chlist_len))
      goto fail;
   if (comp == 3 || comp == 4) {
      if (!exri__write_chromaticities_attr(&buffer, &length, &capacity))
         goto fail;
   }
   compression_value = (exri_uc) compression;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "compression", "compression", &compression_value, 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "dataWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__write_box2i_attr(&buffer, &length, &capacity, "displayWindow", 0, 0, w - 1, h - 1))
      goto fail;
   if (!exri__append_attr_output(&buffer, &length, &capacity, "lineOrder", "lineOrder", (exri_uc const *) "", 1))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "pixelAspectRatio", 1.0f))
      goto fail;
   if (!exri__write_v2f_attr(&buffer, &length, &capacity, "screenWindowCenter", 0.0f, 0.0f))
      goto fail;
   if (!exri__write_float_attr(&buffer, &length, &capacity, "screenWindowWidth", 1.0f))
      goto fail;
   if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
      goto fail;

   offset_table_pos = length;
   for (block_index = 0; block_index < num_blocks; ++block_index) {
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, 0u))
         goto fail;
   }

   row = (exri_uc *) EXRI_MALLOC((size_t) block_capacity_bytes);
   if (row == NULL) {
      exri__err("outofmem");
      goto fail;
   }
   if (compression == EXRI_WRITE_COMPRESSION_RLE) {
      rle_tmp = (exri_uc *) EXRI_MALLOC((size_t) row_bytes);
      rle_data = (exri_uc *) EXRI_MALLOC((size_t) rle_capacity);
      if (rle_tmp == NULL || rle_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP || compression == EXRI_WRITE_COMPRESSION_PXR24) {
      zip_tmp = (exri_uc *) EXRI_MALLOC((size_t) block_capacity_bytes);
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) zip_capacity);
      if (zip_tmp == NULL || zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
      zip_data = (exri_uc *) EXRI_MALLOC((size_t) piz_capacity_bytes);
      if (zip_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
      b44_data = (exri_uc *) EXRI_MALLOC((size_t) b44_capacity);
      if (b44_data == NULL) {
         exri__err("outofmem");
         goto fail;
      }
   }

   for (block_index = 0; block_index < num_blocks; ++block_index) {
      y = block_index * block_lines;
      num_lines = block_lines;
      if (num_lines > h - y)
         num_lines = h - y;
      block_bytes = row_bytes * num_lines;

      for (line = 0; line < num_lines; ++line) {
         row_pos = line * row_bytes;
         for (c = 0; c < comp; ++c) {
            component = exri__component_for_written_channel(comp, c);
            for (x = 0; x < w; ++x) {
               src_index = ((size_t) (y + line) * (size_t) w + (size_t) x) * (size_t) comp + (size_t) component;
               if (written_pixel_type == EXRI_PIXEL_HALF) {
                  exri__put16le(row + row_pos, exri__float_to_half_round(data[src_index]));
                  row_pos += 2;
               } else if (written_pixel_type == EXRI_PIXEL_UINT) {
                  exri__put32le_at(row + row_pos, exri__float_to_uint_sample(data[src_index]));
                  row_pos += 4;
               } else {
                  exri__put_f32le_at(row + row_pos, data[src_index]);
                  row_pos += 4;
               }
            }
         }
      }

      if (!exri__has_offset_table_entry(offset_table_pos, block_index, length)) {
         exri__err("output too large");
         goto fail;
      }
      exri__put64le_size_at(buffer + offset_table_pos + (size_t) block_index * 8u, length);

      payload = row;
      payload_len = block_bytes;
      if (compression == EXRI_WRITE_COMPRESSION_RLE) {
         if (!exri__compress_rle_exr_block(rle_data, rle_capacity, rle_tmp, row, row_bytes, &rle_len))
            goto fail;
         if (rle_len < row_bytes) {
            payload = rle_data;
            payload_len = rle_len;
         }
      } else if (compression == EXRI_WRITE_COMPRESSION_ZIPS || compression == EXRI_WRITE_COMPRESSION_ZIP) {
         if (!exri__compress_zip_exr_block(zip_data, zip_capacity, zip_tmp, row, block_bytes, &zip_len))
            goto fail;
         if (zip_len < block_bytes) {
            payload = zip_data;
            payload_len = zip_len;
         }
      } else if (compression == EXRI_WRITE_COMPRESSION_PIZ) {
         if (!exri__compress_piz_exr_block(zip_data, piz_capacity_bytes, row, block_bytes, w, num_lines, comp, NULL, written_pixel_type, &piz_len))
            goto fail;
         payload = zip_data;
         payload_len = piz_len;
      } else if (compression == EXRI_WRITE_COMPRESSION_PXR24) {
         if (!exri__compress_pxr24_exr_block(zip_data, zip_capacity, zip_tmp, row, w, num_lines, comp, NULL, written_pixel_type, &zip_len))
            goto fail;
         payload = zip_data;
         payload_len = zip_len;
      } else if (compression == EXRI_WRITE_COMPRESSION_B44 || compression == EXRI_WRITE_COMPRESSION_B44A) {
         if (!exri__compress_b44_exr_block(b44_data, b44_capacity, row, w, num_lines, comp, compression == EXRI_WRITE_COMPRESSION_B44A, &b44_len))
            goto fail;
         payload = b44_data;
         payload_len = b44_len;
      }

      if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) y))
         goto fail;
      if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) payload_len))
         goto fail;
      if (!exri__append_output(&buffer, &length, &capacity, payload, payload_len))
         goto fail;
   }

   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(b44_data);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(row);
   *out_data = buffer;
   *out_len = length;
   return 1;

fail:
   EXRI_FREE(zip_data);
   EXRI_FREE(zip_tmp);
   EXRI_FREE(b44_data);
   EXRI_FREE(rle_data);
   EXRI_FREE(rle_tmp);
   EXRI_FREE(row);
   EXRI_FREE(buffer);
   *out_data = NULL;
   *out_len = 0;
   return 0;
}

EXRIDEF int EXRI_CALL exri_writef_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int comp, float const *data, exri_write_options const *options)
{
   int flags;

   if (out_data == NULL || out_len == NULL)
      return exri__err("invalid argument");
   *out_data = NULL;
   *out_len = 0;
   if (!exri__write_options_to_flags(options, 1, &flags))
      return 0;
   return exri__writef_to_memory_flags(out_data, out_len, w, h, comp, data, flags);
}

EXRIDEF int EXRI_CALL exri_writef_channels_to_memory(exri_uc **out_data, size_t *out_len, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options)
{
   int flags;

   if (out_data == NULL || out_len == NULL)
      return exri__err("invalid argument");
   *out_data = NULL;
   *out_len = 0;
   if (!exri__write_options_to_flags(options, 1, &flags))
      return 0;
   return exri__writef_channels_to_memory_flags(out_data, out_len, w, h, num_channels, data, channels, attributes, num_attributes, flags);
}

typedef struct
{
   exri_uc *data;
   size_t len;
   exri__info info;
   int chunk_count;
   size_t multipart_table_pos;
} exri__multipart_write_part;

static int exri__multipart_write_attributes_valid(exri_write_attribute const *attributes, int num_attributes)
{
   int i;

   if (num_attributes < 0)
      return exri__err("invalid argument");
   if (num_attributes > 0 && attributes == NULL)
      return exri__err("invalid argument");

   for (i = 0; i < num_attributes; ++i) {
      if (attributes[i].name == NULL)
         return exri__err("invalid argument");
      if (exri__same_cstr(attributes[i].name, "name") ||
          exri__same_cstr(attributes[i].name, "type") ||
          exri__same_cstr(attributes[i].name, "chunkCount"))
         return exri__err("reserved attribute name");
   }

   return 1;
}

static int exri__multipart_write_part_name_valid(exri_write_part const *parts, int part_index)
{
   int name_len;
   int i;

   if (!exri__strlen_checked(parts[part_index].name, 0, &name_len))
      return 0;

   for (i = 0; i < part_index; ++i) {
      if (strcmp(parts[i].name, parts[part_index].name) == 0)
         return exri__err("duplicate part name");
   }

   return 1;
}

static int exri__multipart_write_single_part(exri_write_part const *part, exri_uc **out_data, size_t *out_len)
{
   if (part->channels == NULL) {
      if (part->attributes != NULL || part->num_attributes != 0)
         return exri__err("channels required for attributes");
      return exri_writef_to_memory(out_data, out_len, part->width, part->height, part->num_channels, part->data, &part->options);
   }

   if (!exri__multipart_write_attributes_valid(part->attributes, part->num_attributes))
      return 0;
   return exri_writef_channels_to_memory(out_data, out_len, part->width, part->height, part->num_channels, part->data, part->channels, part->attributes, part->num_attributes, &part->options);
}

static int exri__single_part_chunk_count_for_info(exri__info const *info, int *chunk_count)
{
   int block_lines;
   int count;
   int num_x_levels;
   int num_y_levels;
   int level_count_y;
   int level_count_x;
   int lx;
   int ly;
   int level_x;
   int level_y;
   int level_w;
   int level_h;
   int num_x_tiles;
   int num_y_tiles;
   int level_tiles;

   if (info == NULL || chunk_count == NULL)
      return exri__err("invalid argument");

   if (!info->tiled) {
      block_lines = exri__scanline_block_lines_for_compression(info->compression);
      count = ((info->height - 1) / block_lines) + 1;
      if (info->chunk_count_found && info->chunk_count != count)
         return exri__err("invalid chunk count");
      *chunk_count = count;
      return 1;
   }

   if (!exri__tiled_num_levels(info, &num_x_levels, &num_y_levels))
      return 0;

   count = 0;
   level_count_y = (info->tile_level_mode == 2) ? num_y_levels : num_x_levels;
   for (ly = 0; ly < level_count_y; ++ly) {
      level_count_x = (info->tile_level_mode == 2) ? num_x_levels : 1;
      for (lx = 0; lx < level_count_x; ++lx) {
         level_x = info->tile_level_mode == 2 ? lx : ly;
         level_y = ly;
         if (!exri__tiled_level_size(info->width, level_x, info->tile_rounding_mode, &level_w) ||
             !exri__tiled_level_size(info->height, level_y, info->tile_rounding_mode, &level_h) ||
             !exri__ceil_div_int(level_w, info->tile_width, &num_x_tiles) ||
             !exri__ceil_div_int(level_h, info->tile_height, &num_y_tiles))
            return exri__err("image too large");
         if (num_x_tiles > INT_MAX / num_y_tiles)
            return exri__err("image too large");
         level_tiles = num_x_tiles * num_y_tiles;
         if (count > INT_MAX - level_tiles)
            return exri__err("image too large");
         count += level_tiles;
      }
   }

   if (info->chunk_count_found && info->chunk_count != count)
      return exri__err("invalid chunk count");
   *chunk_count = count;
   return 1;
}

static int exri__single_part_chunk_total(exri_uc const *data, size_t len, size_t chunk_offset, int tiled, int *chunk_total)
{
   int data_len;

   if (chunk_total == NULL)
      return exri__err("invalid argument");
   if (tiled) {
      if (!exri__has_file_bytes_at(chunk_offset, len, 20))
         return exri__err_invalid();
      data_len = exri__i32_from_u32(exri__get32le_at(data + chunk_offset + 16));
      if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 20u))
         return exri__err_invalid();
      *chunk_total = 20 + data_len;
      return 1;
   }

   if (!exri__has_file_bytes_at(chunk_offset, len, 8))
      return exri__err_invalid();
   data_len = exri__i32_from_u32(exri__get32le_at(data + chunk_offset + 4));
   if (data_len <= 0 || (size_t) data_len > len - (chunk_offset + 8u))
      return exri__err_invalid();
   *chunk_total = 8 + data_len;
   return 1;
}

static int exri__append_single_header_attrs_for_multipart(exri_uc **buffer, size_t *length, size_t *capacity, exri_uc const *single, size_t single_len)
{
   exri__context s;
   char const *name;
   char const *type;
   int name_len;
   int type_len;
   size_t attr_start;
   size_t value_pos;
   int value_size;
   exri__uint32 attr_size;

   s.data = single;
   s.size = single_len;
   s.pos = 8;

   for (;;) {
      attr_start = s.pos;
      if (!exri__read_cstring(&s, &name, &name_len))
         return exri__err_invalid();
      if (name_len == 0)
         return 1;
      if (!exri__read_cstring(&s, &type, &type_len))
         return exri__err_invalid();
      if (!exri__read_u32(&s, &attr_size))
         return exri__err_invalid();
      if (attr_size > (exri__uint32) INT_MAX)
         return exri__err_invalid();
      value_size = (int) attr_size;
      if (!exri__require(&s, (size_t) value_size))
         return exri__err_invalid();
      value_pos = s.pos;
      s.pos = value_pos + (size_t) value_size;

      if (strcmp(name, "name") != 0 &&
          strcmp(name, "type") != 0 &&
          strcmp(name, "chunkCount") != 0) {
         if (s.pos - attr_start > (size_t) INT_MAX)
            return exri__err("output too large");
         if (!exri__append_output(buffer, length, capacity, single + attr_start, (int) (s.pos - attr_start)))
            return 0;
      }
   }
}

EXRIDEF int EXRI_CALL exri_writef_multipart_to_memory(exri_uc **out_data, size_t *out_len, exri_write_part const *parts, int num_parts)
{
   exri__multipart_write_part *written_parts;
   exri_uc *buffer;
   size_t capacity;
   size_t length;
   int i;
   int j;
   int ok;
   size_t chunk_offset;
   int chunk_total;
   int name_len;
   char const *type_name;

   if (out_data == NULL || out_len == NULL)
      return exri__err("invalid argument");
   *out_data = NULL;
   *out_len = 0;

   if (parts == NULL || num_parts < 2)
      return exri__err("invalid argument");
   if ((size_t) num_parts > ((size_t) -1) / sizeof(written_parts[0]))
      return exri__err("outofmem");

   written_parts = (exri__multipart_write_part *) EXRI_MALLOC((size_t) num_parts * sizeof(written_parts[0]));
   if (written_parts == NULL)
      return exri__err("outofmem");
   memset(written_parts, 0, (size_t) num_parts * sizeof(written_parts[0]));

   buffer = NULL;
   capacity = 0;
   length = 0;
   ok = 0;

   for (i = 0; i < num_parts; ++i) {
      if (!exri__multipart_write_part_name_valid(parts, i))
         goto done;
      if (!exri__multipart_write_single_part(parts + i, &written_parts[i].data, &written_parts[i].len))
         goto done;
      if (!exri__parse_header(written_parts[i].data, written_parts[i].len, &written_parts[i].info, NULL))
         goto done;
      if (written_parts[i].info.multipart || written_parts[i].info.non_image) {
         exri__err("unsupported EXR storage");
         goto done;
      }
      if (!exri__single_part_chunk_count_for_info(&written_parts[i].info, &written_parts[i].chunk_count))
         goto done;
      if (written_parts[i].chunk_count <= 0 ||
          written_parts[i].info.header_end > written_parts[i].len ||
          (size_t) written_parts[i].chunk_count > (written_parts[i].len - written_parts[i].info.header_end) / 8u) {
         exri__err_invalid();
         goto done;
      }
   }

   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x01312f76u))
      goto done;
   if (!exri__append_u32le_output(&buffer, &length, &capacity, 0x00001002u))
      goto done;

   for (i = 0; i < num_parts; ++i) {
      if (!exri__append_single_header_attrs_for_multipart(&buffer, &length, &capacity, written_parts[i].data, written_parts[i].len))
         goto done;
      if (!exri__strlen_checked(parts[i].name, 0, &name_len))
         goto done;
      if (!exri__append_attr_output(&buffer, &length, &capacity, "name", "string", (exri_uc const *) parts[i].name, name_len))
         goto done;
      type_name = written_parts[i].info.tiled ? "tiledimage" : "scanlineimage";
      if (!exri__append_attr_output(&buffer, &length, &capacity, "type", "string", (exri_uc const *) type_name, (int) strlen(type_name)))
         goto done;
      if (!exri__write_int_attr(&buffer, &length, &capacity, "chunkCount", written_parts[i].chunk_count))
         goto done;
      if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
         goto done;
   }
   if (!exri__append_u8_output(&buffer, &length, &capacity, 0))
      goto done;

   for (i = 0; i < num_parts; ++i) {
      written_parts[i].multipart_table_pos = length;
      for (j = 0; j < written_parts[i].chunk_count; ++j) {
         if (!exri__append_u64le_size_output(&buffer, &length, &capacity, 0))
            goto done;
      }
   }

   for (i = 0; i < num_parts; ++i) {
      for (j = 0; j < written_parts[i].chunk_count; ++j) {
         if (!exri__get64le_as_size_at(written_parts[i].data + written_parts[i].info.header_end + (size_t) j * 8u, &chunk_offset)) {
            exri__err_invalid();
            goto done;
         }
         if (!exri__single_part_chunk_total(written_parts[i].data, written_parts[i].len, chunk_offset, written_parts[i].info.tiled, &chunk_total))
            goto done;
         exri__put64le_size_at(buffer + written_parts[i].multipart_table_pos + (size_t) j * 8u, length);
         if (!exri__append_u32le_output(&buffer, &length, &capacity, (exri__uint32) i))
            goto done;
         if (!exri__append_output(&buffer, &length, &capacity, written_parts[i].data + chunk_offset, chunk_total))
            goto done;
      }
   }

   *out_data = buffer;
   *out_len = length;
   buffer = NULL;
   ok = 1;

done:
   EXRI_FREE(buffer);
   for (i = 0; i < num_parts; ++i)
      EXRI_FREE(written_parts[i].data);
   EXRI_FREE(written_parts);
   if (!ok) {
      *out_data = NULL;
      *out_len = 0;
   }
   return ok;
}

EXRIDEF int EXRI_CALL exri_writef_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int comp, float const *data, exri_write_options const *options)
{
   exri_uc *buffer;
   size_t len;

   if (clbk == NULL || clbk->write == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_to_memory(&buffer, &len, w, h, comp, data, options))
      return 0;

   if (!clbk->write(user, buffer, len)) {
      EXRI_FREE(buffer);
      return exri__err("write failed");
   }
   EXRI_FREE(buffer);
   return 1;
}

EXRIDEF int EXRI_CALL exri_writef_channels_to_callbacks(exri_write_callbacks const *clbk, void *user, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options)
{
   exri_uc *buffer;
   size_t len;

   if (clbk == NULL || clbk->write == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&buffer, &len, w, h, num_channels, data, channels, attributes, num_attributes, options))
      return 0;

   if (!clbk->write(user, buffer, len)) {
      EXRI_FREE(buffer);
      return exri__err("write failed");
   }
   EXRI_FREE(buffer);
   return 1;
}

EXRIDEF int EXRI_CALL exri_writef_multipart_to_callbacks(exri_write_callbacks const *clbk, void *user, exri_write_part const *parts, int num_parts)
{
   exri_uc *buffer;
   size_t len;

   if (clbk == NULL || clbk->write == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_multipart_to_memory(&buffer, &len, parts, num_parts))
      return 0;

   if (!clbk->write(user, buffer, len)) {
      EXRI_FREE(buffer);
      return exri__err("write failed");
   }
   EXRI_FREE(buffer);
   return 1;
}

static exri_uc *exri__read_callbacks_to_memory(exri_io_callbacks const *clbk, void *user, size_t *out_len)
{
   exri_uc tmp[4096];
   exri_uc *buffer;
   size_t capacity;
   size_t length;
   size_t n;

   if (clbk == NULL || clbk->read == NULL)
      return NULL;

   buffer = NULL;
   capacity = 0;
   length = 0;

   for (;;) {
      n = 0;
      if (!clbk->read(user, tmp, sizeof(tmp), &n)) {
         EXRI_FREE(buffer);
         return NULL;
      }
      if (n > sizeof(tmp)) {
         exri__err("read failed");
         EXRI_FREE(buffer);
         return NULL;
      }
      if (n == 0)
         break;
      if (n > (size_t) INT_MAX || !exri__append(&buffer, &length, &capacity, tmp, (int) n)) {
         EXRI_FREE(buffer);
         return NULL;
      }
   }

   *out_len = length;
   return buffer;
}

EXRIDEF int EXRI_CALL exri_is_exr_from_callbacks(exri_io_callbacks const *clbk, void *user)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_is_exr_from_memory(buffer, len);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_version_from_callbacks(exri_io_callbacks const *clbk, void *user, int *version, int *flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_version_from_memory(buffer, len, version, flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_info_from_memory(buffer, len, x, y, channels_in_file);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int *num_parts)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_count_from_memory(buffer, len, num_parts);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *channels_in_file)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_info_from_memory(buffer, len, part_index, x, y, channels_in_file);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_tiled_level_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int *num_x_levels, int *num_y_levels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_tiled_level_count_from_memory(buffer, len, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_tiled_level_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_x_levels, int *num_y_levels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_tiled_level_count_from_memory(buffer, len, part_index, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_info_from_memory(buffer, len, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_info_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_info_from_memory(buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_count_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_name_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling_from_callbacks(exri_io_callbacks const *clbk, void *user, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_from_callbacks(float **out_pixels, exri_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;
   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_loadf_from_memory(out_pixels, buffer, len, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_part_from_callbacks(float **out_pixels, exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;
   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_loadf_part_from_memory(out_pixels, buffer, len, part_index, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_load_deep_part_from_callbacks(float **out_samples, int **out_sample_offsets, exri_io_callbacks const *clbk, void *user, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_load_deep_part_from_memory(out_samples, out_sample_offsets, buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_is_spectral_from_callbacks(exri_io_callbacks const *clbk, void *user)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_is_spectral_from_memory(buffer, len);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectrum_type_from_callbacks(exri_io_callbacks const *clbk, void *user, int *spectrum_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_spectrum_type_from_memory(buffer, len, spectrum_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectral_wavelengths_from_callbacks(exri_io_callbacks const *clbk, void *user, float *wavelengths, int max_wavelengths, int *num_wavelengths)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_spectral_wavelengths_from_memory(buffer, len, wavelengths, max_wavelengths, num_wavelengths);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectral_units_from_callbacks(exri_io_callbacks const *clbk, void *user, char *units, int units_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_callbacks_to_memory(clbk, user, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_spectral_units_from_memory(buffer, len, units, units_size);
   EXRI_FREE(buffer);
   return result;
}

#ifndef EXRI_NO_STDIO

static exri_uc *exri__read_file_to_memory(FILE *f, size_t *out_len)
{
   exri_uc tmp[4096];
   exri_uc *buffer;
   size_t capacity;
   size_t length;
   size_t n;

   if (f == NULL)
      return NULL;

   buffer = NULL;
   capacity = 0;
   length = 0;

   for (;;) {
      n = fread(tmp, 1, sizeof(tmp), f);
      if (n > 0) {
         if (n > (size_t) INT_MAX) {
            EXRI_FREE(buffer);
            return NULL;
         }
         if (!exri__append(&buffer, &length, &capacity, tmp, (int) n)) {
            EXRI_FREE(buffer);
            return NULL;
         }
      }
      if (n < sizeof(tmp)) {
         if (ferror(f)) {
            EXRI_FREE(buffer);
            return NULL;
         }
         break;
      }
   }

   *out_len = length;
   return buffer;
}

static exri_uc *exri__read_filename_to_memory(char const *filename, size_t *out_len)
{
   FILE *f;
   exri_uc *buffer;

   if (filename == NULL) {
      exri__err("invalid argument");
      return NULL;
   }

   f = fopen(filename, "rb");
   if (f == NULL) {
      exri__err("can't fopen");
      return NULL;
   }

   buffer = exri__read_file_to_memory(f, out_len);
   fclose(f);
   if (buffer == NULL)
      exri__err("read failed");

   return buffer;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_is_exr_from_file(FILE *f)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_is_exr_from_memory(buffer, len);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_version_from_file(FILE *f, int *version, int *flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_version_from_memory(buffer, len, version, flags);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_info_from_file(FILE *f, int *x, int *y, int *channels_in_file)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_info_from_memory(buffer, len, x, y, channels_in_file);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_count_from_file(FILE *f, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_name_from_file(FILE *f, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_pixel_type_from_file(FILE *f, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_sampling_from_file(FILE *f, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_count_from_file(FILE *f, int *num_parts)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_count_from_memory(buffer, len, num_parts);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_info_from_file(FILE *f, int part_index, int *x, int *y, int *channels_in_file)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_info_from_memory(buffer, len, part_index, x, y, channels_in_file);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_channel_count_from_file(FILE *f, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_channel_name_from_file(FILE *f, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_channel_pixel_type_from_file(FILE *f, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_channel_sampling_from_file(FILE *f, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_tiled_level_count_from_file(FILE *f, int part_index, int *num_x_levels, int *num_y_levels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_tiled_level_count_from_memory(buffer, len, part_index, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_info_from_file(FILE *f, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_info_from_memory(buffer, len, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_part_info_from_file(FILE *f, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_info_from_memory(buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_count_from_file(FILE *f, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_part_channel_count_from_file(FILE *f, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_name_from_file(FILE *f, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_part_channel_name_from_file(FILE *f, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_pixel_type_from_file(FILE *f, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_part_channel_pixel_type_from_file(FILE *f, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_sampling_from_file(FILE *f, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_part_channel_sampling_from_file(FILE *f, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_deep_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF float * EXRI_CALL exri__loadf_from_file_ptr(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri_uc *buffer;
   size_t len;
   float *result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL) {
      exri__err("read failed");
      return NULL;
   }

   result = exri__loadf_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_load_deep_from_file_to(float **out_samples, int **out_sample_offsets, FILE *f, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri__load_deep_from_memory(out_samples, out_sample_offsets, buffer, len, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_load_deep_part_from_file_to(float **out_samples, int **out_sample_offsets, FILE *f, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_load_deep_part_from_memory(out_samples, out_sample_offsets, buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF float * EXRI_CALL exri__loadf_part_from_file_ptr(FILE *f, int part_index, int *x, int *y, int *channels_in_file, int desired_channels)
{
   exri_uc *buffer;
   size_t len;
   float *result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL) {
      exri__err("read failed");
      return NULL;
   }

   result = exri__loadf_part_from_memory_ptr(buffer, len, part_index, x, y, channels_in_file, desired_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF float * EXRI_CALL exri_loadf_scrgb_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   exri_uc *buffer;
   size_t len;
   float *result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL) {
      exri__err("read failed");
      return NULL;
   }

   result = exri__loadf_scrgb_from_memory_ptr(buffer, len, x, y, channels_in_file, desired_channels, color_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF float * EXRI_CALL exri_loadf_part_scrgb_from_file(FILE *f, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int color_flags)
{
   exri_uc *buffer;
   size_t len;
   float *result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL) {
      exri__err("read failed");
      return NULL;
   }

   result = exri__loadf_part_scrgb_from_memory_ptr(buffer, len, part_index, x, y, channels_in_file, desired_channels, color_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_is_exr(char const *filename)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri_is_exr_from_file(f);
   fclose(f);
   return result;
}

EXRIDEF int EXRI_CALL exri_version(char const *filename, int *version, int *flags)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri_version_from_file(f, version, flags);
   fclose(f);
   return result;
}

EXRIDEF int EXRI_CALL exri_info(char const *filename, int *x, int *y, int *channels_in_file)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri_info_from_file(f, x, y, channels_in_file);
   fclose(f);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_count(char const *filename, int *num_channels)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_name(char const *filename, int channel_index, char *name, int name_size)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_pixel_type(char const *filename, int channel_index, int *pixel_type)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_channel_sampling(char const *filename, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_layer_count(char const *filename, int *num_layers)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_layer_count_from_memory(buffer, len, num_layers);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_layer_name(char const *filename, int layer_index, char *name, int name_size)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_layer_name_from_memory(buffer, len, layer_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_count(char const *filename, int *num_attributes)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_count_from_memory(buffer, len, num_attributes);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_name(char const *filename, int attribute_index, char *name, int name_size)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_name_from_memory(buffer, len, attribute_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_type(char const *filename, int attribute_index, char *type, int type_size)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_type_from_memory(buffer, len, attribute_index, type, type_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value_size(char const *filename, int attribute_index, int *value_size)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_value_size_from_memory(buffer, len, attribute_index, value_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value(char const *filename, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_value_from_memory(buffer, len, attribute_index, value, value_size, bytes_written);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_count_from_file(FILE *f, int *num_attributes)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_count_from_memory(buffer, len, num_attributes);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_attribute_count(char const *filename, int part_index, int *num_attributes)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_attribute_count_from_memory(buffer, len, part_index, num_attributes);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_attribute_count_from_file(FILE *f, int part_index, int *num_attributes)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_attribute_count_from_memory(buffer, len, part_index, num_attributes);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_name_from_file(FILE *f, int attribute_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_name_from_memory(buffer, len, attribute_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_attribute_name(char const *filename, int part_index, int attribute_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_attribute_name_from_memory(buffer, len, part_index, attribute_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_attribute_name_from_file(FILE *f, int part_index, int attribute_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_attribute_name_from_memory(buffer, len, part_index, attribute_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_type_from_file(FILE *f, int attribute_index, char *type, int type_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_type_from_memory(buffer, len, attribute_index, type, type_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_attribute_type(char const *filename, int part_index, int attribute_index, char *type, int type_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_attribute_type_from_memory(buffer, len, part_index, attribute_index, type, type_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_attribute_type_from_file(FILE *f, int part_index, int attribute_index, char *type, int type_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_attribute_type_from_memory(buffer, len, part_index, attribute_index, type, type_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value_size_from_file(FILE *f, int attribute_index, int *value_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_value_size_from_memory(buffer, len, attribute_index, value_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_attribute_value_size(char const *filename, int part_index, int attribute_index, int *value_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_attribute_value_size_from_memory(buffer, len, part_index, attribute_index, value_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_attribute_value_size_from_file(FILE *f, int part_index, int attribute_index, int *value_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_attribute_value_size_from_memory(buffer, len, part_index, attribute_index, value_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_attribute_value_from_file(FILE *f, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_attribute_value_from_memory(buffer, len, attribute_index, value, value_size, bytes_written);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_attribute_value(char const *filename, int part_index, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_attribute_value_from_memory(buffer, len, part_index, attribute_index, value, value_size, bytes_written);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_part_attribute_value_from_file(FILE *f, int part_index, int attribute_index, exri_uc *value, int value_size, int *bytes_written)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_part_attribute_value_from_memory(buffer, len, part_index, attribute_index, value, value_size, bytes_written);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_tiled_level_count(char const *filename, int *num_x_levels, int *num_y_levels)
{
   FILE *f;
   exri_uc *buffer;
   size_t len;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "rb");
   if (f == NULL)
      return exri__err("can't fopen");

   buffer = exri__read_file_to_memory(f, &len);
   fclose(f);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_tiled_level_count_from_memory(buffer, len, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_tiled_level_count_from_file(FILE *f, int *num_x_levels, int *num_y_levels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_file_to_memory(f, &len);
   if (buffer == NULL)
      return exri__err("read failed");

   result = exri_tiled_level_count_from_memory(buffer, len, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_count(char const *filename, int *num_parts)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_count_from_memory(buffer, len, num_parts);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_info(char const *filename, int part_index, int *x, int *y, int *channels_in_file)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_info_from_memory(buffer, len, part_index, x, y, channels_in_file);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_count(char const *filename, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_name(char const *filename, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_pixel_type(char const *filename, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_channel_sampling(char const *filename, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_part_tiled_level_count(char const *filename, int part_index, int *num_x_levels, int *num_y_levels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_part_tiled_level_count_from_memory(buffer, len, part_index, num_x_levels, num_y_levels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_info(char const *filename, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_info_from_memory(buffer, len, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_info(char const *filename, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_part_info_from_memory(buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_count(char const *filename, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_channel_count_from_memory(buffer, len, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_count(char const *filename, int part_index, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_part_channel_count_from_memory(buffer, len, part_index, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_name(char const *filename, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_channel_name_from_memory(buffer, len, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_name(char const *filename, int part_index, int channel_index, char *name, int name_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_part_channel_name_from_memory(buffer, len, part_index, channel_index, name, name_size);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_pixel_type(char const *filename, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_channel_pixel_type_from_memory(buffer, len, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_pixel_type(char const *filename, int part_index, int channel_index, int *pixel_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_part_channel_pixel_type_from_memory(buffer, len, part_index, channel_index, pixel_type);
   EXRI_FREE(buffer);
   return result;
}

EXRI__PRIVATEDEF int EXRI_CALL exri_deep_channel_sampling(char const *filename, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_channel_sampling_from_memory(buffer, len, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_deep_part_channel_sampling(char const *filename, int part_index, int channel_index, int *x_sampling, int *y_sampling, int *p_linear)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_deep_part_channel_sampling_from_memory(buffer, len, part_index, channel_index, x_sampling, y_sampling, p_linear);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_load_deep_part(float **out_samples, int **out_sample_offsets, char const *filename, int part_index, int *x, int *y, int *num_channels, int *total_samples)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_samples == NULL || out_sample_offsets == NULL)
      return exri__err("invalid argument");
   *out_samples = NULL;
   *out_sample_offsets = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_load_deep_part_from_memory(out_samples, out_sample_offsets, buffer, len, part_index, x, y, num_channels, total_samples);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf(float **out_pixels, char const *filename, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_from_memory(out_pixels, buffer, len, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_part(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_part_from_memory(out_pixels, buffer, len, part_index, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_layer(float **out_pixels, char const *filename, char const *layer, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_layer_from_memory(out_pixels, buffer, len, layer, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_region(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_region_from_memory(out_pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_layer_region(float **out_pixels, char const *filename, char const *layer, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_layer_region_from_memory(out_pixels, buffer, len, layer, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_channels(float **out_pixels, char const *filename, int *x, int *y, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_channels_from_memory(out_pixels, buffer, len, x, y, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_part_channels(float **out_pixels, char const *filename, int part_index, int *x, int *y, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_part_channels_from_memory(out_pixels, buffer, len, part_index, x, y, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_channels_region(float **out_pixels, char const *filename, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *num_channels)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_channels_region_from_memory(out_pixels, buffer, len, region_x, region_y, region_w, region_h, x, y, num_channels);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_tiled_level(float **out_pixels, char const *filename, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_tiled_level_from_memory(out_pixels, buffer, len, level_x, level_y, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_part_tiled_level(float **out_pixels, char const *filename, int part_index, int level_x, int level_y, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_part_tiled_level_from_memory(out_pixels, buffer, len, part_index, level_x, level_y, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_loadf_tiled_level_region(float **out_pixels, char const *filename, int level_x, int level_y, int region_x, int region_y, int region_w, int region_h, int *x, int *y, int *channels_in_file, int desired_channels, int load_flags)
{
   exri_uc *buffer;
   size_t len;
   int result;

   if (out_pixels == NULL)
      return exri__err("invalid argument");
   *out_pixels = NULL;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_loadf_tiled_level_region_from_memory(out_pixels, buffer, len, level_x, level_y, region_x, region_y, region_w, region_h, x, y, channels_in_file, desired_channels, load_flags);
   EXRI_FREE(buffer);
   return result;
}

static int exri__writef_to_file(FILE *f, int w, int h, int comp, float const *data, exri_write_options const *options)
{
   exri_uc *buffer;
   size_t len;
   size_t written;

   if (f == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_to_memory(&buffer, &len, w, h, comp, data, options))
      return 0;

   written = fwrite(buffer, 1, (size_t) len, f);
   EXRI_FREE(buffer);
   if (written != (size_t) len)
      return exri__err("write failed");

   return 1;
}

EXRIDEF int EXRI_CALL exri_writef(char const *filename, int w, int h, int comp, float const *data, exri_write_options const *options)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "wb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri__writef_to_file(f, w, h, comp, data, options);
   if (fclose(f) != 0 && result)
      return exri__err("write failed");

   return result;
}

static int exri__writef_channels_to_file(FILE *f, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options)
{
   exri_uc *buffer;
   size_t len;
   size_t written;

   if (f == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_channels_to_memory(&buffer, &len, w, h, num_channels, data, channels, attributes, num_attributes, options))
      return 0;

   written = fwrite(buffer, 1, (size_t) len, f);
   EXRI_FREE(buffer);
   if (written != (size_t) len)
      return exri__err("write failed");

   return 1;
}

EXRIDEF int EXRI_CALL exri_writef_channels(char const *filename, int w, int h, int num_channels, float const *data, exri_write_channel const *channels, exri_write_attribute const *attributes, int num_attributes, exri_write_options const *options)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "wb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri__writef_channels_to_file(f, w, h, num_channels, data, channels, attributes, num_attributes, options);
   if (fclose(f) != 0 && result)
      return exri__err("write failed");

   return result;
}

static int exri__writef_multipart_to_file(FILE *f, exri_write_part const *parts, int num_parts)
{
   exri_uc *buffer;
   size_t len;
   size_t written;

   if (f == NULL)
      return exri__err("invalid argument");

   buffer = NULL;
   len = 0;
   if (!exri_writef_multipart_to_memory(&buffer, &len, parts, num_parts))
      return 0;

   written = fwrite(buffer, 1, (size_t) len, f);
   EXRI_FREE(buffer);
   if (written != (size_t) len)
      return exri__err("write failed");

   return 1;
}

EXRIDEF int EXRI_CALL exri_writef_multipart(char const *filename, exri_write_part const *parts, int num_parts)
{
   FILE *f;
   int result;

   if (filename == NULL)
      return exri__err("invalid argument");

   f = fopen(filename, "wb");
   if (f == NULL)
      return exri__err("can't fopen");

   result = exri__writef_multipart_to_file(f, parts, num_parts);
   if (fclose(f) != 0 && result)
      return exri__err("write failed");

   return result;
}

EXRIDEF int EXRI_CALL exri_is_spectral(char const *filename)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_is_spectral_from_memory(buffer, len);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectrum_type(char const *filename, int *spectrum_type)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_spectrum_type_from_memory(buffer, len, spectrum_type);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectral_wavelengths(char const *filename, float *wavelengths, int max_wavelengths, int *num_wavelengths)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_spectral_wavelengths_from_memory(buffer, len, wavelengths, max_wavelengths, num_wavelengths);
   EXRI_FREE(buffer);
   return result;
}

EXRIDEF int EXRI_CALL exri_spectral_units(char const *filename, char *units, int units_size)
{
   exri_uc *buffer;
   size_t len;
   int result;

   buffer = exri__read_filename_to_memory(filename, &len);
   if (buffer == NULL)
      return 0;

   result = exri_spectral_units_from_memory(buffer, len, units, units_size);
   EXRI_FREE(buffer);
   return result;
}

#endif /* EXRI_NO_STDIO */

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif /* EXR_IMAGE_IMPLEMENTATION */
