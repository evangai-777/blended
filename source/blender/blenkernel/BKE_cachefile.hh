/* SPDX-FileCopyrightText: 2016 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup bke
 */

namespace blender {

struct CacheFile;
struct CacheFileLayer;
struct CacheReader;
struct Object;

double BKE_cachefile_time_offset(const CacheFile *cache_file, double time, double fps);
double BKE_cachefile_frame_offset(const CacheFile *cache_file, double time);

/* Modifiers open and free readers through these (until inline migration complete). */
void BKE_cachefile_reader_open(CacheFile *cache_file,
                               CacheReader **reader,
                               Object *object,
                               const char *object_path);
void BKE_cachefile_reader_free(CacheFile *cache_file, CacheReader **reader);

CacheFileLayer *BKE_cachefile_add_layer(CacheFile *cache_file,
                                        const char filepath[/*FILE_MAX*/ 1024]);

CacheFileLayer *BKE_cachefile_get_active_layer(CacheFile *cache_file);

void BKE_cachefile_remove_layer(CacheFile *cache_file, CacheFileLayer *layer);

}  // namespace blender
