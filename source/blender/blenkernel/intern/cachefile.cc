/* SPDX-FileCopyrightText: 2016 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bke
 */

#include <cstring>

#include "DNA_cachefile_types.h"
#include "DNA_object_types.h"

#include "BLI_listbase.h"
#include "BLI_mutex.hh"
#include "BLI_path_utils.hh"
#include "BLI_string.h"
#include "BLI_utildefines.h"

#include "BKE_cachefile.hh"

#include "MEM_guardedalloc.h"

#ifdef WITH_ALEMBIC
#  include "ABC_alembic.h"
#endif

#ifdef WITH_USD
#  include "usd_api_modifier.hh"
#endif

namespace blender {

#if defined(WITH_ALEMBIC) || defined(WITH_USD)
/* TODO: make this per cache file to avoid global locks. */
static Mutex cache_mutex;
#endif

void BKE_cachefile_reader_open(CacheFile *cache_file,
                               CacheReader **reader,
                               Object *object,
                               const char *object_path)
{
#if defined(WITH_ALEMBIC) || defined(WITH_USD)

  BLI_assert(cache_file->id.tag & ID_TAG_COPIED_ON_EVAL);

  if (cache_file->handle == nullptr) {
    return;
  }

  switch (cache_file->type) {
    case CACHEFILE_TYPE_ALEMBIC:
#  ifdef WITH_ALEMBIC
      /* Open Alembic cache reader. */
      *reader = CacheReader_open_alembic_object(
          cache_file->handle, *reader, object, object_path, cache_file->is_sequence);
#  endif
      break;
    case CACHEFILE_TYPE_USD:
#  ifdef WITH_USD
      /* Open USD cache reader. */
      *reader = io::usd::CacheReader_open_usd_object(
          cache_file->handle, *reader, object, object_path);
#  endif
      break;
    case CACHE_FILE_TYPE_INVALID:
      break;
  }

  /* Multiple modifiers and constraints can call this function concurrently. */
  std::lock_guard lock(cache_mutex);
  if (*reader) {
    /* Register in set so we can free it when the cache file changes. */
    if (cache_file->handle_readers == nullptr) {
      cache_file->handle_readers = MEM_new<CacheFileHandleReaderSet>("CacheFile.handle_readers");
    }
    cache_file->handle_readers->add(reader);
  }
  else if (cache_file->handle_readers) {
    /* Remove in case CacheReader_open_alembic_object free the existing reader. */
    cache_file->handle_readers->remove(reader);
  }
#else
  UNUSED_VARS(cache_file, reader, object, object_path);
#endif
}

void BKE_cachefile_reader_free(CacheFile *cache_file, CacheReader **reader)
{
#if defined(WITH_ALEMBIC) || defined(WITH_USD)
  /* Multiple modifiers and constraints can call this function concurrently, and
   * cachefile_handle_free() can also be called at the same time. */
  std::lock_guard lock(cache_mutex);
  if (*reader != nullptr) {
    if (cache_file) {
      BLI_assert(cache_file->id.tag & ID_TAG_COPIED_ON_EVAL);

      switch (cache_file->type) {
        case CACHEFILE_TYPE_ALEMBIC:
#  ifdef WITH_ALEMBIC
          ABC_CacheReader_free(*reader);
#  endif
          break;
        case CACHEFILE_TYPE_USD:
#  ifdef WITH_USD
          io::usd::USD_CacheReader_free(*reader);
#  endif
          break;
        case CACHE_FILE_TYPE_INVALID:
          break;
      }
    }

    *reader = nullptr;

    if (cache_file && cache_file->handle_readers) {
      cache_file->handle_readers->remove(reader);
    }
  }
#else
  UNUSED_VARS(cache_file, reader);
#endif
}

double BKE_cachefile_time_offset(const CacheFile *cache_file, const double time, const double fps)
{
  const double time_offset = double(cache_file->frame_offset) / fps;
  const double frame = (cache_file->override_frame ? double(cache_file->frame) : time);
  return cache_file->is_sequence ? frame : frame / fps - time_offset;
}

double BKE_cachefile_frame_offset(const CacheFile *cache_file, const double time)
{
  const double time_offset = double(cache_file->frame_offset);
  const double frame = cache_file->override_frame ? double(cache_file->frame) : time;
  return cache_file->is_sequence ? frame : frame - time_offset;
}

CacheFileLayer *BKE_cachefile_add_layer(CacheFile *cache_file, const char filepath[FILE_MAX])
{
  for (CacheFileLayer &layer : cache_file->layers) {
    if (STREQ(layer.filepath, filepath)) {
      return nullptr;
    }
  }

  const int num_layers = BLI_listbase_count(&cache_file->layers);

  CacheFileLayer *layer = MEM_new<CacheFileLayer>("CacheFileLayer");
  STRNCPY(layer->filepath, filepath);

  BLI_addtail(&cache_file->layers, layer);

  cache_file->active_layer = char(num_layers + 1);

  return layer;
}

CacheFileLayer *BKE_cachefile_get_active_layer(CacheFile *cache_file)
{
  return static_cast<CacheFileLayer *>(
      BLI_findlink(&cache_file->layers, cache_file->active_layer - 1));
}

void BKE_cachefile_remove_layer(CacheFile *cache_file, CacheFileLayer *layer)
{
  cache_file->active_layer = 0;
  BLI_remlink(&cache_file->layers, layer);
  MEM_delete(layer);
}

}  // namespace blender
