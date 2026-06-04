/* SPDX-FileCopyrightText: Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bke
 */

#include <cstring>

#include "DNA_collection_types.h"
#include "DNA_light_types.h"
#include "DNA_lightprobe_types.h"
#include "DNA_object_types.h"

#include "BLI_math_base.h"
#include "BLI_string_utf8.h"
#include "BLI_string_utils.hh"
#include "BLI_utildefines.h"

#include "MEM_guardedalloc.h"

#include "BLI_listbase.h"

#include "BKE_idtype.hh"
#include "BKE_lib_id.hh"
#include "BKE_lightprobe.h"
#include "BKE_main.hh"

#include "BLO_read_write.hh"

namespace blender {

void BKE_lightprobe_type_set(LightProbe *probe, const short lightprobe_type)
{
  probe->type = lightprobe_type;

  switch (probe->type) {
    case LIGHTPROBE_TYPE_VOLUME:
      probe->distinf = 0.3f;
      probe->falloff = 1.0f;
      probe->clipsta = 0.01f;
      break;
    case LIGHTPROBE_TYPE_PLANE:
      probe->distinf = 0.1f;
      probe->falloff = 0.5f;
      probe->clipsta = 0.001f;
      break;
    case LIGHTPROBE_TYPE_SPHERE:
      probe->attenuation_type = LIGHTPROBE_SHAPE_ELIPSOID;
      break;
    default:
      BLI_assert_msg(0, "LightProbe type not configured.");
      break;
  }
}

LightProbe *BKE_lightprobe_add(Main *bmain, const char *name)
{
  LightProbe *probe = MEM_new<LightProbe>("LightProbe");
  BKE_libblock_runtime_ensure(probe->id);
  *(reinterpret_cast<short *>(probe->id.name)) = ID_LP;
  probe->id.us = 1;
  {
    ListBaseT<ID> *lb = which_libbase(bmain, ID_LP);
    BKE_main_lock(bmain);
    BLI_addtail(lb, probe);
    /* ID_LP is deregistered — BKE_id_new_name_validate indexes namemap at -1 → crash. */
    BLI_strncpy_utf8(probe->id.name + 2, name, sizeof(probe->id.name) - 2);
    BLI_uniquename(reinterpret_cast<const ListBase *>(lb),
                   probe,
                   name,
                   '.',
                   offsetof(ID, name) + 2,
                   sizeof(probe->id.name) - 2);
    bmain->is_memfile_undo_written = false;
    BKE_main_unlock(bmain);
  }
  BKE_lib_libblock_session_uid_ensure(&probe->id);
  return probe;
}

void BKE_lightprobe_type_apply_to_light(Light *la, int probe_type)
{
  switch (probe_type) {
    case LIGHTPROBE_TYPE_SPHERE:
      la->type = LA_PROBE_SPHERE;
      la->probe_attenuation_type = LIGHTPROBE_SHAPE_ELIPSOID;
      break;
    case LIGHTPROBE_TYPE_PLANE:
      la->type = LA_PROBE_PLANAR;
      la->probe_distinf = 0.1f;
      la->probe_falloff = 0.5f;
      la->probe_clipsta = 0.001f;
      break;
    case LIGHTPROBE_TYPE_VOLUME:
      la->type = LA_PROBE_VOLUME;
      la->probe_distinf = 0.3f;
      la->probe_falloff = 1.0f;
      la->probe_clipsta = 0.01f;
      break;
    default:
      BLI_assert_msg(0, "Unknown LightProbe type.");
      break;
  }
}

void BKE_lightprobe_drain_from_bmain(Main *bmain)
{
  LightProbe *probe = static_cast<LightProbe *>(bmain->lightprobes.first);
  while (probe) {
    LightProbe *next = static_cast<LightProbe *>(probe->id.next);
    BLI_remlink(&bmain->lightprobes, probe);
    BKE_libblock_free_data(&probe->id, false);
    MEM_delete(probe);
    probe = next;
  }
}

static void lightprobe_grid_cache_frame_blend_write(BlendWriter *writer,
                                                    const LightProbeGridCacheFrame *cache)
{
  writer->write_struct_array(cache->block_len, cache->block_infos);

  int64_t sample_count = BKE_lightprobe_grid_cache_frame_sample_count(cache);

  writer->write_float3_array(sample_count, reinterpret_cast<float *>(cache->irradiance.L0));
  writer->write_float3_array(sample_count, reinterpret_cast<float *>(cache->irradiance.L1_a));
  writer->write_float3_array(sample_count, reinterpret_cast<float *>(cache->irradiance.L1_b));
  writer->write_float3_array(sample_count, reinterpret_cast<float *>(cache->irradiance.L1_c));

  writer->write_float_array(sample_count, cache->visibility.L0);
  writer->write_float_array(sample_count, cache->visibility.L1_a);
  writer->write_float_array(sample_count, cache->visibility.L1_b);
  writer->write_float_array(sample_count, cache->visibility.L1_c);

  writer->write_int8_array(sample_count, reinterpret_cast<int8_t *>(cache->connectivity.validity));
}

static void lightprobe_grid_cache_frame_blend_read(BlendDataReader *reader,
                                                   LightProbeGridCacheFrame *cache)
{
  if (!ELEM(
          cache->data_layout, LIGHTPROBE_CACHE_ADAPTIVE_RESOLUTION, LIGHTPROBE_CACHE_UNIFORM_GRID))
  {
    /* Do not try to read data from incompatible layout. Clear all pointers. */
    *cache = LightProbeGridCacheFrame{};
    return;
  }

  BLO_read_struct_array(reader, LightProbeGridCacheFrame, cache->block_len, &cache->block_infos);

  int64_t sample_count = BKE_lightprobe_grid_cache_frame_sample_count(cache);

  /* Baking data is not stored. */
  cache->baking.L0 = nullptr;
  cache->baking.L1_a = nullptr;
  cache->baking.L1_b = nullptr;
  cache->baking.L1_c = nullptr;
  cache->baking.virtual_offset = nullptr;
  cache->baking.validity = nullptr;
  cache->surfels = nullptr;
  cache->surfels_len = 0;

  BLO_read_float3_array(reader, sample_count, reinterpret_cast<float **>(&cache->irradiance.L0));
  BLO_read_float3_array(reader, sample_count, reinterpret_cast<float **>(&cache->irradiance.L1_a));
  BLO_read_float3_array(reader, sample_count, reinterpret_cast<float **>(&cache->irradiance.L1_b));
  BLO_read_float3_array(reader, sample_count, reinterpret_cast<float **>(&cache->irradiance.L1_c));

  BLO_read_float_array(reader, sample_count, &cache->visibility.L0);
  BLO_read_float_array(reader, sample_count, &cache->visibility.L1_a);
  BLO_read_float_array(reader, sample_count, &cache->visibility.L1_b);
  BLO_read_float_array(reader, sample_count, &cache->visibility.L1_c);

  BLO_read_int8_array(
      reader, sample_count, reinterpret_cast<int8_t **>(&cache->connectivity.validity));
}

void BKE_lightprobe_cache_blend_write(BlendWriter *writer, LightProbeObjectCache *cache)
{
  if (cache->grid_static_cache != nullptr) {
    writer->write_struct(cache->grid_static_cache);
    lightprobe_grid_cache_frame_blend_write(writer, cache->grid_static_cache);
  }
}

void BKE_lightprobe_cache_blend_read(BlendDataReader *reader, LightProbeObjectCache *cache)
{
  if (cache->grid_static_cache != nullptr) {
    BLO_read_struct(reader, LightProbeGridCacheFrame, &cache->grid_static_cache);
    lightprobe_grid_cache_frame_blend_read(reader, cache->grid_static_cache);
  }
}

template<typename T> static void spherical_harmonic_free(T &data)
{
  MEM_SAFE_DELETE(data.L0);
  MEM_SAFE_DELETE(data.L1_a);
  MEM_SAFE_DELETE(data.L1_b);
  MEM_SAFE_DELETE(data.L1_c);
}

template<typename DataT, typename T> static void spherical_harmonic_copy(T &dst, T &src)
{
  dst.L0 = MEM_dupalloc(src.L0);
  dst.L1_a = MEM_dupalloc(src.L1_a);
  dst.L1_b = MEM_dupalloc(src.L1_b);
  dst.L1_c = MEM_dupalloc(src.L1_c);
}

LightProbeGridCacheFrame *BKE_lightprobe_grid_cache_frame_create()
{
  LightProbeGridCacheFrame *cache = MEM_new<LightProbeGridCacheFrame>("LightProbeGridCacheFrame");
  return cache;
}

LightProbeGridCacheFrame *BKE_lightprobe_grid_cache_frame_copy(LightProbeGridCacheFrame *src)
{
  LightProbeGridCacheFrame *dst = MEM_dupalloc(src);
  dst->block_infos = MEM_dupalloc(src->block_infos);
  spherical_harmonic_copy<float[3]>(dst->irradiance, src->irradiance);
  spherical_harmonic_copy<float>(dst->visibility, src->visibility);
  dst->connectivity.validity = MEM_dupalloc(src->connectivity.validity);
  /* NOTE: Don't copy baking since it wouldn't be freed nor updated after completion. */
  dst->baking.L0 = nullptr;
  dst->baking.L1_a = nullptr;
  dst->baking.L1_b = nullptr;
  dst->baking.L1_c = nullptr;
  dst->baking.virtual_offset = nullptr;
  dst->baking.validity = nullptr;
  dst->surfels = nullptr;
  return dst;
}

void BKE_lightprobe_grid_cache_frame_free(LightProbeGridCacheFrame *cache)
{
  MEM_SAFE_DELETE(cache->block_infos);
  spherical_harmonic_free(cache->baking);
  spherical_harmonic_free(cache->irradiance);
  spherical_harmonic_free(cache->visibility);
  MEM_SAFE_DELETE(cache->baking.validity);
  MEM_SAFE_DELETE(cache->connectivity.validity);
  MEM_SAFE_DELETE_VOID(cache->surfels);
  MEM_SAFE_DELETE(cache->baking.virtual_offset);

  MEM_SAFE_DELETE(cache);
}

void BKE_lightprobe_cache_create(Object *object)
{
  BLI_assert(object->lightprobe_cache == nullptr);

  object->lightprobe_cache = MEM_new<LightProbeObjectCache>("LightProbeObjectCache");
}

LightProbeObjectCache *BKE_lightprobe_cache_copy(LightProbeObjectCache *src_cache)
{
  BLI_assert(src_cache != nullptr);

  LightProbeObjectCache *dst_cache = MEM_dupalloc(src_cache);

  if (src_cache->grid_static_cache) {
    dst_cache->grid_static_cache = BKE_lightprobe_grid_cache_frame_copy(
        src_cache->grid_static_cache);
  }
  return dst_cache;
}

void BKE_lightprobe_cache_free(Object *object)
{
  if (object->lightprobe_cache == nullptr) {
    return;
  }

  LightProbeObjectCache *cache = object->lightprobe_cache;

  if (cache->shared == false) {
    if (cache->grid_static_cache != nullptr) {
      BKE_lightprobe_grid_cache_frame_free(cache->grid_static_cache);
    }
  }

  MEM_SAFE_DELETE(object->lightprobe_cache);
}

int64_t BKE_lightprobe_grid_cache_frame_sample_count(const LightProbeGridCacheFrame *cache)
{
  if (cache->data_layout == LIGHTPROBE_CACHE_ADAPTIVE_RESOLUTION) {
    return cache->block_len * cube_i(cache->block_size);
  }
  /* LIGHTPROBE_CACHE_UNIFORM_GRID */
  return int64_t(cache->size[0]) * cache->size[1] * cache->size[2];
}

}  // namespace blender
