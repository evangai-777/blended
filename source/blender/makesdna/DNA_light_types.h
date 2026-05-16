/* SPDX-FileCopyrightText: 2001-2002 NaN Holding BV. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup DNA
 */

#pragma once

#include "BLI_math_constants.h"

#include "DNA_ID.h"
#include "DNA_defs.h"
#include "DNA_lightprobe_types.h"

namespace blender {

#ifndef MAX_MTEX
#  define MAX_MTEX 18
#endif

struct AnimData;
struct bNodeTree;

/** #Light::flag */
enum {
  LA_DS_EXPAND = 1 << 0,
  /**
   * NOTE: this must have the same value as #MA_DS_SHOW_TEXS,
   * otherwise anim-editors will not read correctly.
   */
  LA_DS_SHOW_TEXS = 1 << 2,
};

/** #Light::type */
enum {
  LA_LOCAL = 0,
  LA_SUN = 1,
  LA_SPOT = 2,
  // LA_HEMI = 3, /* Deprecated. */
  LA_AREA = 4,
  /** Light probe objects — permanent homes for Bucket 3 LightProbe fold-down (0.7.0). */
  LA_PROBE_SPHERE = 5,
  LA_PROBE_PLANAR = 6,
  LA_PROBE_VOLUME = 7,
};

/** #Light::mode */
enum {
  LA_SHADOW = 1 << 0,
  // LA_HALO = 1 << 1, /* Deprecated. */
  // LA_LAYER = 1 << 2, /* Deprecated. */
  // LA_QUAD = 1 << 3, /* Deprecated. */
  // LA_NEG = 1 << 4, /* Deprecated. */
  // LA_ONLYSHADOW = 1 << 5, /* Deprecated. */
  // LA_SPHERE = 1 << 6, /* Deprecated. */
  LA_SQUARE = 1 << 7,
  // LA_TEXTURE = 1 << 8, /* Deprecated. */
  // LA_OSATEX = 1 << 9, /* Deprecated. */
  // LA_DEEP_SHADOW = 1 << 10, /* Deprecated. */
  // LA_NO_DIFF = 1 << 11, /* Deprecated. */
  // LA_NO_SPEC = 1 << 12, /* Deprecated. */
  LA_SHAD_RAY = 1 << 13, /* Deprecated, cleaned. */
  /**
   * YAFRAY: light shadow-buffer flag, soft-light.
   * Since it is used with LOCAL light, can't use LA_SHAD.
   */
  // LA_YF_SOFT = 1 << 14, /* Deprecated. */
  // LA_LAYER_SHADOW = 1 << 15, /* Deprecated. */
  // LA_SHAD_TEX = 1 << 16, /* Deprecated. */
  LA_SHOW_CONE = 1 << 17,
  // LA_SHOW_SHADOW_BOX = 1 << 18,
  // LA_SHAD_CONTACT = 1 << 19, /* Deprecated. */
  LA_CUSTOM_ATTENUATION = 1 << 20,
  LA_USE_SOFT_FALLOFF = 1 << 21,
  /** Use absolute resolution clamping instead of relative. */
  LA_SHAD_RES_ABSOLUTE = 1 << 22,
  LA_SHADOW_JITTER = 1 << 23,
  LA_USE_TEMPERATURE = 1 << 24,
  LA_UNNORMALIZED = 1 << 25,
};

/** #Light::falloff_type */
enum {
  LA_FALLOFF_CONSTANT = 0,
  LA_FALLOFF_INVLINEAR = 1,
  LA_FALLOFF_INVSQUARE = 2,
  LA_FALLOFF_CURVE = 3,
  LA_FALLOFF_SLIDERS = 4,
  LA_FALLOFF_INVCOEFFICIENTS = 5,
};

/** #Light::area_shape */
enum {
  LA_AREA_SQUARE = 0,
  LA_AREA_RECT = 1,
  // LA_AREA_CUBE = 2, /* Deprecated. */
  // LA_AREA_BOX = 3,  /* Deprecated. */
  LA_AREA_DISK = 4,
  LA_AREA_ELLIPSE = 5,
};

struct Light {
#ifdef __cplusplus
  DNA_DEFINE_CXX_METHODS(Light)
  /** See #ID_Type comment for why this is here. */
  static constexpr ID_Type id_type = ID_LA;
#endif

  ID id;
  /** Animation data (must be immediately after id for utilities to use it). */
  struct AnimData *adt = nullptr;

  /* Type and flags. */
  short type = 0, flag = 0;
  int mode = LA_SHADOW | LA_USE_SOFT_FALLOFF;

  /* Color, temperature and energy. */
  float r = 1.0f, g = 1.0f, b = 1.0f;
  float temperature = 6500.0f;
  float energy = 10.0f;
  float exposure = 0;

  /* Point light. */
  float radius = 0;

  /* Spot Light. */
  float spotsize = DEG2RADF(45.0f);
  float spotblend = 0.15f;

  /* Area light. */
  short area_shape = 0;
  short _pad1 = {};
  float area_size = 0.25f;
  float area_sizey = 0.25f;
  float area_sizez = 0.25f;
  float area_spread = DEG2RADF(180.0f);

  /* Sun light. */
  float sun_angle = DEG2RADF(0.526f);

  /* Nodes. */
  short pr_texture = 0;
  DNA_DEPRECATED short use_nodes = 0;

  /* Eevee */
  float clipsta = 0.05f;
  float clipend_deprecated = 0;

  float cascade_max_dist = 200.0f;
  float cascade_exponent = 0.8f;
  float cascade_fade = 0.1f;
  int cascade_count = 4;

  float diff_fac = 1.0f;
  float spec_fac = 1.0f;
  float transmission_fac = 1.0f;
  float volume_fac = 1.0f;

  float att_dist = 40.0f;
  float shadow_filter_radius = 1.0f;
  float shadow_maximum_resolution = 0.001f;
  float shadow_jitter_overblur = 10.0f;

  /* Preview */
  struct PreviewImage *preview = nullptr;

  /* Nodes */
  struct bNodeTree *nodetree = nullptr;

  /* Light probe (type == LA_PROBE_SPHERE, LA_PROBE_PLANAR, or LA_PROBE_VOLUME).
   * Fields migrated from LightProbe ID — permanent homes for Bucket 3 (0.7.0). */
  /** LIGHTPROBE_FLAG_* bits (show influence/parallax/clip/data, invert group, etc.). */
  char probe_flag = LIGHTPROBE_FLAG_SHOW_INFLUENCE;
  /** LIGHTPROBE_SHAPE_ELIPSOID / LIGHTPROBE_SHAPE_BOX — sphere attenuation shape. */
  char probe_attenuation_type = 0;
  /** LIGHTPROBE_SHAPE_ELIPSOID / LIGHTPROBE_SHAPE_BOX — sphere parallax shape. */
  char probe_parallax_type = 0;
  /** LIGHTPROBE_GRID_CAPTURE_* flags for volume grid. */
  char probe_grid_flag = LIGHTPROBE_GRID_CAPTURE_INDIRECT | LIGHTPROBE_GRID_CAPTURE_EMISSION;
  /** Influence radius (sphere) or z-axis attenuation distance (planar). */
  float probe_distinf = 2.5f;
  /** Custom parallax sphere radius (sphere only). */
  float probe_distpar = 2.5f;
  /** Influence falloff weight (0=hard edge, 1=full fade). */
  float probe_falloff = 0.2f;
  /** Near clip plane for probe captures. */
  float probe_clipsta = 0.8f;
  /** Far clip plane for probe captures. */
  float probe_clipend = 20.0f;
  float probe_vis_bias = 1.0f, probe_vis_bleedbias = 0;
  float probe_vis_blur = 0.2f;
  /** Intensity multiplier applied to captured lighting. */
  float probe_intensity = 1.0f;
  /** Irradiance grid sample resolution (volume probe). */
  int probe_grid_resolution_x = 4;
  int probe_grid_resolution_y = 4;
  int probe_grid_resolution_z = 4;
  /** Number of hemisphere samples per grid cell during baking. */
  int probe_grid_bake_samples = 2048;
  float probe_grid_surface_bias = 0.05f;
  float probe_grid_escape_bias = 0.1f;
  float probe_grid_normal_bias = 0.3f;
  float probe_grid_view_bias = 0.0f;
  float probe_grid_facing_bias = 0.5f;
  float probe_grid_validity_threshold = 0.40f;
  float probe_grid_dilation_threshold = 0.5f;
  float probe_grid_dilation_radius = 1.0f;
  float probe_grid_clamp_direct = 0.0f;
  float probe_grid_clamp_indirect = 10.0f;
  /** Scene-surface surfel density for volume probe baking. */
  int probe_grid_surfel_density = 20;
  char _pad3[4] = {};
  /** Visibility group: objects included/excluded from probe captures. */
  struct Collection *probe_visibility_grp = nullptr;
  float probe_data_display_size = 0.1f;
  char _pad4[4] = {};

  /* Deprecated. */
  DNA_DEPRECATED float energy_deprecated = 10.0f;
  float _pad2 = 0.0f;
};

}  // namespace blender
