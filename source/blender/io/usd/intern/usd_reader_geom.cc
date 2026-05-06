/* SPDX-FileCopyrightText: 2021 Tangent Animation. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "usd_reader_geom.hh"

#include "BKE_modifier.hh"

#include "BLI_listbase.h"
#include "BLI_string.h"

#include "DNA_cachefile_types.h"
#include "DNA_modifier_types.h"
#include "DNA_object_types.h"

namespace blender::io::usd {

void USDGeomReader::add_cache_modifier()
{
  if (settings_->filepath[0] == '\0') {
    return;
  }

  ModifierData *md = BKE_modifier_new(eModifierType_MeshSequenceCache);
  BLI_addtail(&object_->modifiers, md);
  BKE_modifiers_persistent_uid_init(*object_, *md);

  MeshSeqCacheModifierData *mcmd = reinterpret_cast<MeshSeqCacheModifierData *>(md);

  STRNCPY(mcmd->filepath, settings_->filepath);
  mcmd->is_sequence = settings_->is_sequence;
  mcmd->type = char(CACHEFILE_TYPE_USD);
  mcmd->scale = settings_->scale;
  mcmd->read_flag = import_params_.mesh_read_flag;

  STRNCPY(mcmd->object_path, prim_.GetPath().GetString().c_str());
}

void USDGeomReader::add_subdiv_modifier()
{
  ModifierData *md = BKE_modifier_new(eModifierType_Subsurf);
  BLI_addtail(&object_->modifiers, md);
  BKE_modifiers_persistent_uid_init(*object_, *md);
}

}  // namespace blender::io::usd
