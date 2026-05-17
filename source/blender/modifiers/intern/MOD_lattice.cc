/* SPDX-FileCopyrightText: 2005 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup modifiers
 */

#include <cstring>

#include "BLI_math_matrix.h"
#include "BLI_utildefines.h"

#include "BLT_translation.hh"

#include "DNA_lattice_types.h"
#include "DNA_object_types.h"
#include "DNA_screen_types.h"

#include "BKE_lattice.hh"
#include "BKE_lib_query.hh"
#include "BKE_mesh.hh"
#include "BKE_modifier.hh"

#include "BLO_read_write.hh"

#include "UI_interface_layout.hh"
#include "UI_resources.hh"

#include "RNA_prototypes.hh"
#include "RNA_types.hh"

#include "MOD_ui_common.hh"
#include "MOD_util.hh"

namespace blender {

static void init_data(ModifierData *md)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
  INIT_DEFAULT_STRUCT_AFTER(lmd, modifier);
  unit_m4(lmd->object_to_lattice);
  lmd->lattice = BKE_lattice_new_modifier("Lattice");
}

static void copy_data(const ModifierData *md, ModifierData *target, const int flag)
{
  BKE_modifier_copydata_generic(md, target, flag);
  const LatticeModifierData *lmd = reinterpret_cast<const LatticeModifierData *>(md);
  LatticeModifierData *tlmd = reinterpret_cast<LatticeModifierData *>(target);
  tlmd->lattice = BKE_lattice_copy_modifier(lmd->lattice);
}

static void free_data(ModifierData *md)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
  BKE_lattice_free_modifier(lmd->lattice);
  lmd->lattice = nullptr;
}

static void required_data_mask(ModifierData *md, CustomData_MeshMasks *r_cddata_masks)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);

  /* Ask for vertex-groups if we need them. */
  if (lmd->name[0] != '\0') {
    r_cddata_masks->vmask |= CD_MASK_MDEFORMVERT;
  }
}

static bool is_disabled(const Scene * /*scene*/, ModifierData *md, bool /*use_render_params*/)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
  /* Disabled if no geometry source: object-based path (lmd->object) or embedded lattice. */
  return lmd->object == nullptr && lmd->lattice == nullptr;
}

static void foreach_ID_link(ModifierData *md, Object *ob, IDWalkFunc walk, void *user_data)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
  /* Walk the deprecated object pointer so the lib-link pass can resolve it before
   * versioning 502.29 migrates the data to the embedded lattice and nulls it. */
  walk(user_data, ob, reinterpret_cast<ID **>(&lmd->object), IDWALK_CB_NOP);
}

static void deform_verts(ModifierData *md,
                         const ModifierEvalContext *ctx,
                         Mesh *mesh,
                         MutableSpan<float3> positions)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);

  /* if next modifier needs original vertices */
  MOD_previous_vcos_store(md, reinterpret_cast<const float (*)[3]>(positions.data()));

  if (lmd->object) {
    /* Object-based path: lattice parenting / add-to-selected sets lmd->object but not
     * lmd->lattice. Read geometry from the live OB_LATTICE object's data. */
    BKE_lattice_deform_coords_with_mesh(lmd->object,
                                        ctx->object,
                                        reinterpret_cast<float (*)[3]>(positions.data()),
                                        positions.size(),
                                        lmd->flag,
                                        lmd->name,
                                        lmd->strength,
                                        mesh);
  }
  else {
    BKE_lattice_deform_coords_with_mesh_inline(lmd->lattice,
                                               lmd->object_to_lattice,
                                               ctx->object,
                                               reinterpret_cast<float (*)[3]>(positions.data()),
                                               positions.size(),
                                               lmd->flag,
                                               lmd->name,
                                               lmd->strength,
                                               mesh);
  }
}

static void deform_verts_EM(ModifierData *md,
                            const ModifierEvalContext *ctx,
                            const BMEditMesh *em,
                            Mesh *mesh,
                            MutableSpan<float3> positions)
{
  if (mesh->runtime->wrapper_type == ME_WRAPPER_TYPE_MDATA) {
    deform_verts(md, ctx, mesh, positions);
    return;
  }

  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);

  /* if next modifier needs original vertices */
  MOD_previous_vcos_store(md, reinterpret_cast<const float (*)[3]>(positions.data()));

  if (lmd->object) {
    BKE_lattice_deform_coords_with_editmesh(lmd->object,
                                            ctx->object,
                                            reinterpret_cast<float (*)[3]>(positions.data()),
                                            positions.size(),
                                            lmd->flag,
                                            lmd->name,
                                            lmd->strength,
                                            em);
  }
  else {
    BKE_lattice_deform_coords_with_editmesh_inline(lmd->lattice,
                                                   lmd->object_to_lattice,
                                                   ctx->object,
                                                   reinterpret_cast<float (*)[3]>(positions.data()),
                                                   positions.size(),
                                                   lmd->flag,
                                                   lmd->name,
                                                   lmd->strength,
                                                   em);
  }
}

static void blend_write(BlendWriter *writer, const Object * /*ob*/, const ModifierData *md)
{
  const LatticeModifierData *lmd = reinterpret_cast<const LatticeModifierData *>(md);
  if (lmd->lattice) {
    writer->write_struct(lmd->lattice);
    BKE_lattice_write_modifier(writer, lmd->lattice);
  }
}

static void blend_read(BlendDataReader *reader, ModifierData *md)
{
  LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
  BLO_read_struct(reader, Lattice, &lmd->lattice);
  if (lmd->lattice) {
    BKE_lattice_read_modifier(reader, lmd->lattice);
  }
}

static void panel_draw(const bContext * /*C*/, Panel *panel)
{
  ui::Layout &layout = *panel->layout;

  PointerRNA ob_ptr;
  PointerRNA *ptr = modifier_panel_get_property_pointers(panel, &ob_ptr);

  layout.use_property_split_set(true);

  modifier_vgroup_ui(layout, ptr, &ob_ptr, "vertex_group", "invert_vertex_group", std::nullopt);

  layout.prop(ptr, "strength", ui::ITEM_R_SLIDER, std::nullopt, ICON_NONE);

  modifier_error_message_draw(layout, ptr);
}

static void panel_register(ARegionType *region_type)
{
  modifier_panel_register(region_type, eModifierType_Lattice, panel_draw);
}

ModifierTypeInfo modifierType_Lattice = {
    /*idname*/ "Lattice",
    /*name*/ N_("Lattice"),
    /*struct_name*/ "LatticeModifierData",
    /*struct_size*/ sizeof(LatticeModifierData),
    /*srna*/ &RNA_LatticeModifier,
    /*type*/ ModifierTypeType::OnlyDeform,
    /*flags*/ eModifierTypeFlag_AcceptsCVs | eModifierTypeFlag_AcceptsVertexCosOnly |
        eModifierTypeFlag_SupportsEditmode,
    /*icon*/ ICON_MOD_LATTICE,

    /*copy_data*/ copy_data,

    /*deform_verts*/ deform_verts,
    /*deform_matrices*/ nullptr,
    /*deform_verts_EM*/ deform_verts_EM,
    /*deform_matrices_EM*/ nullptr,
    /*modify_mesh*/ nullptr,
    /*modify_geometry_set*/ nullptr,

    /*init_data*/ init_data,
    /*required_data_mask*/ required_data_mask,
    /*free_data*/ free_data,
    /*is_disabled*/ is_disabled,
    /*update_depsgraph*/ nullptr,
    /*depends_on_time*/ nullptr,
    /*depends_on_normals*/ nullptr,
    /*foreach_ID_link*/ foreach_ID_link,
    /*foreach_tex_link*/ nullptr,
    /*free_runtime_data*/ nullptr,
    /*panel_register*/ panel_register,
    /*blend_write*/ blend_write,
    /*blend_read*/ blend_read,
    /*foreach_cache*/ nullptr,
    /*foreach_working_space_color*/ nullptr,
};

}  // namespace blender
