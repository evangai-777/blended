/* SPDX-FileCopyrightText: 2025 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup blenloader
 */

#define DNA_DEPRECATED_ALLOW

#include "MEM_guardedalloc.h"

#include "NOD_geometry_nodes_srna.hh"

#include "DNA_ID.h"
#include "DNA_brush_types.h"
#include "DNA_curve_types.h"
#include "DNA_light_types.h"
#include "DNA_lightprobe_types.h"
#include "DNA_lattice_types.h"
#include "DNA_mask_types.h"
#include "DNA_sequence_types.h"
#include "DNA_vfont_types.h"
#include "DNA_modifier_types.h"
#include "DNA_node_tree_interface_types.h"
#include "DNA_node_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "DNA_screen_types.h"

#include "BLI_listbase_iterator.hh"
#include "BLI_math_matrix.h"
#include "BLI_string.h"
#include "BLI_string_utils.hh"
#include "BLI_sys_types.h"

#include "BKE_animsys.h"
#include "BKE_colortools.hh"
#include "BKE_curves.hh"
#include "BKE_idprop.hh"
#include "BKE_lib_id.hh"
#include "BKE_light.h"
#include "BKE_lightprobe.h"
#include "BKE_lattice.hh"
#include "BKE_mask.hh"
#include "BKE_main.hh"
#include "BKE_mesh_legacy_convert.hh"
#include "BKE_node.hh"
#include "BKE_node_legacy_types.hh"
#include "BKE_node_runtime.hh"
#include "BKE_report.hh"

#include "SEQ_iterator.hh"
#include "SEQ_sequencer.hh"

#include "BLO_read_write.hh"
#include "readfile.hh"

#include "versioning_common.hh"

// #include "CLG_log.h"

namespace blender {

// static CLG_LogRef LOG = {"blend.doversion"};

static void version_geometry_nodes_properties(FileData &fd,
                                              Main &bmain,
                                              Object &object,
                                              NodesModifierData &nmd)
{
  const IDProperty *old_props = nmd.settings_legacy.properties;
  if (!old_props) {
    /* Versioning has already been done, this check makes the function idempotent. */
    return;
  }
  if (!nmd.node_group) {
    IDP_FreeProperty(nmd.settings_legacy.properties);
    nmd.settings_legacy.properties = nullptr;
    BLO_reportf_wrap(fd.reports,
                     RPT_WARNING,
                     "Modifier '%s' from Object '%s' is missing its Geometry Node Group, its "
                     "settings will be lost (reset to default).",
                     nmd.modifier.name,
                     BKE_id_name(object.id));
    return;
  }
  if (ID_MISSING(&nmd.node_group->id)) {
    /* Keeping the old idproperties is not an option, and not really useful, since if the
     * blend-file is saved in this current state, it won't be re-versioned here later anyway.
     *
     * Furthermore, the whole remaining part of the code expects this to be nullptr, and keeping it
     * at runtime actually causes weird issues in depsgraph nodes building phase.
     *
     * So all in all, it's simpler and safer to also just lose these values here - if file is not
     * saved in this state, next loading will do the versioning if the node-group is available
     * again, otherwise that data is lost.
     */
    IDP_FreeProperty(nmd.settings_legacy.properties);
    nmd.settings_legacy.properties = nullptr;
    BLO_reportf_wrap(
        fd.reports,
        RPT_WARNING,
        "Modifier '%s' from Object '%s' is using a missing linked Geometry Node Group, its "
        "settings will be lost (reset to default) if the file is saved in this state.",
        nmd.modifier.name,
        BKE_id_name(object.id));
    return;
  }
  const bNodeTree &ntree = *nmd.node_group;
  ntree.ensure_interface_cache();

  IDProperty *system_props = bke::idprop::create_group("NodesModifierProperties").release();

  IDProperty *inputs = bke::idprop::create_group("inputs").release();
  IDP_AddToGroup(system_props, inputs);

  const std::string inputs_path_prefix = fmt::format("modifiers[\"{}\"]", nmd.modifier.name);
  for (const bNodeTreeInterfaceSocket *input : ntree.interface_inputs()) {
    const StringRefNull identifier = input->identifier;
    IDProperty *old_value_prop = IDP_GetPropertyFromGroup(old_props, identifier);
    if (!old_value_prop) {
      continue;
    }

    IDProperty *group = bke::idprop::create_group(identifier).release();
    IDP_AddToGroup(inputs, group);

    if (input->flag & NODE_INTERFACE_SOCKET_LAYER_SELECTION) {
      IDP_AddToGroup(
          group, bke::idprop::create("type", int(nodes::GeometryNodesInputType::Layer)).release());
      const StringRefNull layer_name = [&]() {
        const IDProperty *layer_name = IDP_GetPropertyFromGroup(old_props, identifier);
        if (layer_name) {
          return StringRefNull(IDP_string_get(layer_name));
        }
        return StringRefNull();
      }();
      IDP_AddToGroup(group, bke::idprop::create("layer_name", layer_name).release());
      continue;
    }

    IDProperty *new_value_prop = IDP_CopyProperty(old_value_prop);
    STRNCPY(new_value_prop->name, "value");
    IDP_AddToGroup(group, new_value_prop);

    const std::string old_value_path = fmt::format("[\"{}\"]", identifier);
    const std::string new_value_path = fmt::format(".properties.inputs.{}.value", identifier);
    BKE_animdata_fix_paths_rename_all_ex(&bmain,
                                         &object.id,
                                         inputs_path_prefix.c_str(),
                                         old_value_path.c_str(),
                                         new_value_path.c_str(),
                                         0,
                                         0,
                                         false,
                                         false);

    if (IDOverrideLibrary *override_library = object.id.override_library) {
      for (IDOverrideLibraryProperty &prop : override_library->properties) {
        const StringRef path = prop.rna_path;
        const int64_t i = path.find(inputs_path_prefix);
        if (i == StringRef::not_found) {
          continue;
        }
        if (path.drop_known_prefix(inputs_path_prefix) != old_value_path) {
          continue;
        }
        MEM_delete(prop.rna_path);
        prop.rna_path = BLI_sprintfN("%s%s", inputs_path_prefix.c_str(), new_value_path.c_str());
      }
    }

    bool use_attribute = false;
    if (const IDProperty *use_attribute_prop = IDP_GetPropertyFromGroup(
            old_props, identifier + "_use_attribute"))
    {
      /* This property changed to an enum property and animation is not versioned. */
      if (use_attribute_prop->type == IDP_INT) {
        use_attribute = bool(IDP_int_get(use_attribute_prop));
      }
      else {
        use_attribute = bool(IDP_bool_get(use_attribute_prop));
      }
    }

    const auto input_type = use_attribute ? nodes::GeometryNodesInputType::Attribute :
                                            nodes::GeometryNodesInputType::Value;
    IDP_AddToGroup(group, bke::idprop::create("type", int(input_type)).release());
    const StringRefNull attribute_name = [&]() {
      const IDProperty *attribute_name = IDP_GetPropertyFromGroup(old_props,
                                                                  identifier + "_attribute_name");
      if (attribute_name) {
        return StringRefNull(IDP_string_get(attribute_name));
      }
      return StringRefNull();
    }();
    IDP_AddToGroup(group, bke::idprop::create("attribute_name", attribute_name).release());
  }

  IDProperty *outputs = bke::idprop::create_group("outputs").release();
  IDP_AddToGroup(system_props, outputs);
  for (const bNodeTreeInterfaceSocket *output : ntree.interface_outputs()) {
    const StringRef identifier = output->identifier;
    IDProperty *old_name_prop = IDP_GetPropertyFromGroup(old_props,
                                                         identifier + "_attribute_name");
    if (!old_name_prop) {
      continue;
    }
    IDProperty *group = bke::idprop::create_group(identifier).release();
    IDP_AddToGroup(outputs, group);

    IDProperty *new_value_prop = IDP_CopyProperty(old_name_prop);
    STRNCPY(new_value_prop->name, "attribute_name");
    IDP_AddToGroup(group, new_value_prop);
  }

  if (nmd.modifier.system_properties) {
    IDP_FreeProperty(nmd.modifier.system_properties);
  }
  nmd.modifier.system_properties = system_props;
  IDP_FreeProperty(nmd.settings_legacy.properties);
  nmd.settings_legacy.properties = nullptr;
}

static void sanitize_node_tree_interface_socket_identifiers(bNodeTree &node_tree)
{
  node_tree.ensure_interface_cache();
  Set<StringRef> all_identifiers;
  for (bNodeTreeInterfaceItem *item : node_tree.interface_items()) {
    if (item->item_type == NODE_INTERFACE_PANEL) {
      continue;
    }
    auto &socket = *bke::node_interface::get_item_as<bNodeTreeInterfaceSocket>(item);
    /* Socket identifiers are required to be valid RNA identifiers and unique. */
    if (!RNA_validate_identifier(socket.identifier, true)) {
      RNA_identifier_sanitize(socket.identifier, true);
      if (all_identifiers.contains(socket.identifier)) {
        std::string new_identifier = BLI_uniquename_cb(
            [&](StringRef name) { return all_identifiers.contains(name); },
            '_',
            socket.identifier);
        MEM_SAFE_DELETE(socket.identifier);
        socket.identifier = BLI_strdup(new_identifier.c_str());
      }
    }
    all_identifiers.add(socket.identifier);
  }
}

/* Saving file extension is now a property of the File Output node. So inherit this
 * setting from the active scene to restore the old behavior.
 * Note: One limitation is that node groups containing file outputs that are not part of any
 * scene are not affected by versioning. */
static void do_version_file_output_use_file_extension_recursive(bNodeTree &node_tree,
                                                                const Scene &scene)
{
  for (bNode &node : node_tree.nodes) {
    if (node.type_legacy == CMP_NODE_OUTPUT_FILE) {
      NodeCompositorFileOutput *data = static_cast<NodeCompositorFileOutput *>(node.storage);
      data->use_file_extension = (scene.r.scemode & R_EXTENSION) != 0;
    }
    else if (node.type_legacy == NODE_GROUP) {
      bNodeTree *ngroup = id_cast<bNodeTree *>(node.id);
      if (ngroup) {
        do_version_file_output_use_file_extension_recursive(*ngroup, scene);
      }
    }
  }
}

static void version_clear_strip_linear_modifier_flag(Main &bmain)
{
  for (Scene &scene : bmain.scenes) {
    Editing *ed = seq::editing_get(&scene);
    if (ed != nullptr) {
      seq::foreach_strip(&ed->seqbase, [&](Strip *strip) {
        constexpr int flag_linear_modifiers = 1 << 23;
        strip->flag &= ~flag_linear_modifiers;
        return true;
      });
    }
  }
}

static void fix_single_point_curves_custom_knots(Main *bmain)
{
  /* Fix corrupted flagu/flagv values created by older versions of the Curve Pen tool.
   * The tool could create loose vertices with invalid flag values (e.g. -2), where
   * CU_NURB_CUSTOM was set alongside other flags and knotsu/knotsv was left null,
   * causing a crash when opening these files in newer versions. */
  for (Curve &cu : bmain->curves) {
    for (Nurb *nu = static_cast<Nurb *>(cu.nurb.first); nu != nullptr; nu = nu->next) {
      if (nu->knotsu == nullptr && (nu->flagu & CU_NURB_CUSTOM)) {
        nu->flagu &= (CU_NURB_CYCLIC | CU_NURB_BEZIER | CU_NURB_ENDPOINT);
      }
      if (nu->knotsv == nullptr && (nu->flagv & CU_NURB_CUSTOM)) {
        nu->flagv &= (CU_NURB_CYCLIC | CU_NURB_BEZIER | CU_NURB_ENDPOINT);
      }
    }
  }
}

void do_versions_after_linking_520(FileData *fd, Main *bmain)
{
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 2)) {
    for (Scene &scene : bmain->scenes) {
      bNodeTree *node_tree = version_get_scene_compositor_node_tree(bmain, &scene);
      if (node_tree == nullptr) {
        continue;
      }
      do_version_file_output_use_file_extension_recursive(*node_tree, scene);
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 16)) {
    for (Object &object : bmain->objects) {
      for (ModifierData &md : object.modifiers) {
        if (md.type == eModifierType_Nodes) {
          version_geometry_nodes_properties(
              *fd, *bmain, object, reinterpret_cast<NodesModifierData &>(md));
        }
      }
    }
  }

  /**
   * Always bump subversion in BKE_blender_version.h when adding versioning
   * code here, and wrap it inside a MAIN_VERSION_FILE_ATLEAST check.
   *
   * \note Keep this message at the bottom of the function.
   */
}

void blo_do_versions_520(FileData *fd, Library * /*lib*/, Main *bmain)
{
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 1)) {
    for (Scene &scene : bmain->scenes) {
      scene.r.mode |= R_SAVE_OUTPUT;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 4)) {
    for (Brush &brush : bmain->brushes) {
      if (brush.gpencil_settings != nullptr) {
        brush.blend = 0;
      }
    }
  }
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 5)) {
    FOREACH_NODETREE_BEGIN (bmain, node_tree, id_owner) {
      for (bNode &node : node_tree->nodes) {
        if (node.type_legacy == FN_NODE_INPUT_VECTOR) {
          auto &data = *static_cast<NodeInputVector *>(node.storage);
          data.vector[3] = 0.0f;
          data.dimensions = 3;
        }
      }
    }
    FOREACH_NODETREE_END;
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 6)) {
    for (Scene &scene : bmain->scenes) {
      SequencerToolSettings *sequencer_tool_settings = seq::tool_settings_ensure(&scene);
      sequencer_tool_settings->snap_flag |= SEQ_SNAP_TO_ALL_CHANNEL_STRIPS;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 7)) {
    for (Scene &scene : bmain->scenes) {
      scene.r.anisotropic_filter = 2;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 9)) {
    for (Mesh &mesh : bmain->meshes) {
      bke::mesh_freestyle_marks_to_generic(mesh);
    }
  }

  /* Convert H.264 codec value for older files (2.79), see #155775. */
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 10)) {
    for (Scene &scene : bmain->scenes) {
      if (scene.r.ffcodecdata.codec == 28) {
        scene.r.ffcodecdata.codec = 27;
      }
    }
  }

  /* Disable "unified" flags for Grease Pencil Draw mode. */
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 11)) {
    for (Scene &scene : bmain->scenes) {
      if (scene.toolsettings->gp_paint) {
        UnifiedPaintSettings &settings =
            scene.toolsettings->gp_paint->paint.unified_paint_settings;
        settings.flag &= ~(UNIFIED_PAINT_SIZE | UNIFIED_PAINT_ALPHA | UNIFIED_PAINT_COLOR);
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 12)) {
    for (bScreen &screen : bmain->screens) {
      for (ScrArea &area : screen.areabase) {
        for (SpaceLink &space : area.spacedata) {
          if (space.spacetype == SPACE_NODE) {
            SpaceNode *space_node = reinterpret_cast<SpaceNode *>(&space);
            space_node->overlay.flag |= SN_OVERLAY_SHOW_RENDER_REGION;
            space_node->overlay.passepartout_alpha = 0.5f;
          }
        }
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 13)) {
    version_clear_strip_linear_modifier_flag(*bmain);
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 14)) {
    fix_single_point_curves_custom_knots(bmain);
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 15)) {
    for (Scene &scene : bmain->scenes) {
      scene.r.scemode |= R_USE_TEXTURE_CACHE;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 16)) {
    for (Brush &brush : bmain->brushes) {
      if (brush.gpencil_settings != nullptr) {
        brush.gpencil_settings->curve_type = CURVE_TYPE_POLY;
        brush.gpencil_settings->conversion_threshold = 0.001f;
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 17)) {
    for (Material &materials : bmain->materials) {
      if (materials.gp_style != nullptr) {
        materials.gp_style->placement_mode = GP_MATERIAL_PLACEMENT_COUNT;
        materials.gp_style->placement_count = 1;
        materials.gp_style->placement_density = 10.0f;
        materials.gp_style->placement_radius_spacing = 100.0f;
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 18)) {
    for (Scene &scene : bmain->scenes) {
      if (scene.toolsettings->sculpt) {
        Sculpt &sculpt = *scene.toolsettings->sculpt;
        MeshAutomaskingSettings *settings = MEM_new<MeshAutomaskingSettings>(__func__);
        settings->flags = sculpt.automasking_flags;
        settings->boundary_edges_propagation_steps =
            sculpt.automasking_boundary_edges_propagation_steps;
        settings->cavity_blur_steps = sculpt.automasking_cavity_blur_steps;
        settings->cavity_factor = sculpt.automasking_cavity_factor;
        settings->start_normal_limit = sculpt.automasking_start_normal_limit;
        settings->start_normal_falloff = sculpt.automasking_start_normal_falloff;
        settings->view_normal_limit = sculpt.automasking_view_normal_limit;
        settings->view_normal_falloff = sculpt.automasking_view_normal_falloff;
        settings->cavity_curve = BKE_curvemapping_copy(sculpt.automasking_cavity_curve);
        settings->cavity_curve_op = BKE_curvemapping_copy(sculpt.automasking_cavity_curve_op);

        scene.toolsettings->sculpt->paint.mesh_automasking_settings = settings;
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 19)) {
    for (bNodeTree &tree : bmain->nodetrees) {
      sanitize_node_tree_interface_socket_identifiers(tree);
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 20)) {
    for (Brush &brush : bmain->brushes) {
      if (brush.ob_mode != OB_MODE_SCULPT) {
        continue;
      }

      brush.mesh_automasking_settings = MEM_new<MeshAutomaskingSettings>(__func__);
      brush.mesh_automasking_settings->flags = brush.automasking_flags;
      brush.mesh_automasking_settings->boundary_edges_propagation_steps =
          brush.automasking_boundary_edges_propagation_steps;
      brush.mesh_automasking_settings->cavity_blur_steps = brush.automasking_cavity_blur_steps;
      brush.mesh_automasking_settings->cavity_factor = brush.automasking_cavity_factor;
      brush.mesh_automasking_settings->start_normal_falloff =
          brush.automasking_start_normal_falloff;
      brush.mesh_automasking_settings->start_normal_limit = brush.automasking_start_normal_limit;
      brush.mesh_automasking_settings->view_normal_falloff = brush.automasking_view_normal_falloff;
      brush.mesh_automasking_settings->view_normal_limit = brush.automasking_view_normal_limit;
      brush.mesh_automasking_settings->cavity_curve = BKE_curvemapping_copy(
          brush.automasking_cavity_curve);
      brush.mesh_automasking_settings->cavity_curve_op = nullptr;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 21)) {
    for (Material &materials : bmain->materials) {
      if (materials.gp_style != nullptr) {
        materials.gp_style->random_size_factor = 0.0f;
        materials.gp_style->random_strength_factor = 0.0f;
        materials.gp_style->random_rotation_factor = 0.0f;
        materials.gp_style->random_hue_factor = 0.0f;
        materials.gp_style->random_saturation_factor = 0.0f;
        materials.gp_style->random_value_factor = 0.0f;
        materials.gp_style->random_noise_scale = 1.0f;
      }
    }
  }
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 22)) {
    /* BRUSH_STROKE_CURVE (value 6) was removed in Blended 0.4.0 when PaintCurve (ID_PC) was
     * cut. Remap any persisted value to DOTS so 5.2.x files load with a valid stroke mode. */
    for (Brush &brush : bmain->brushes) {
      if (brush.stroke_method == BRUSH_STROKE_CURVE) {
        brush.stroke_method = BRUSH_STROKE_DOTS;
      }
    }
  }
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 23)) {
    /* OB_SPEAKER (value 12) was removed in Blended 0.4.0 when ID_SPK was cut.
     * Convert speaker objects to empty objects so files load without dangling data pointers. */
    for (Object &object : bmain->objects) {
      if (object.type == 12 /* OB_SPEAKER */) {
        object.type = OB_EMPTY;
        object.data = nullptr;
      }
    }
  }
  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 24)) {
    /* Bucket 3 VFont permanent home (Blended 0.7.0): populate font filepath fields on Curve
     * from the VFont ID pointer. Runtime code will be migrated to read filepath directly;
     * the vfont/vfontb/vfonti/vfontbi pointers will be deprecated in a later layer. */
    for (Object &object : bmain->objects) {
      if (object.type == OB_FONT && object.data != nullptr) {
        Curve *cu = static_cast<Curve *>(static_cast<void *>(object.data));
        if (cu->font_filepath[0] == '\0' && cu->vfont != nullptr) {
          STRNCPY(cu->font_filepath, cu->vfont->filepath);
        }
        if (cu->font_bold_filepath[0] == '\0' && cu->vfontb != nullptr) {
          STRNCPY(cu->font_bold_filepath, cu->vfontb->filepath);
        }
        if (cu->font_italic_filepath[0] == '\0' && cu->vfonti != nullptr) {
          STRNCPY(cu->font_italic_filepath, cu->vfonti->filepath);
        }
        if (cu->font_bold_italic_filepath[0] == '\0' && cu->vfontbi != nullptr) {
          STRNCPY(cu->font_bold_italic_filepath, cu->vfontbi->filepath);
        }
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 25)) {
    /* Bucket 3 Palette permanent home (Blended 0.7.0): palette data is now embedded in
     * Brush::palette. Clear the deprecated Paint::palette pointer so it does not point
     * into the just-drained bmain->palettes listbase after file load. */
    for (Scene &scene_ref : bmain->scenes) {
      Scene *scene = &scene_ref;
      ToolSettings *ts = scene->toolsettings;
      if (ts) {
        auto clear_palette = [](Paint *paint) {
          if (paint) {
            paint->palette = nullptr;
          }
        };
        clear_palette(&ts->imapaint.paint);
        if (ts->sculpt) {
          clear_palette(&ts->sculpt->paint);
        }
        if (ts->vpaint) {
          clear_palette(&ts->vpaint->paint);
        }
        if (ts->wpaint) {
          clear_palette(&ts->wpaint->paint);
        }
        if (ts->gp_paint) {
          clear_palette(&ts->gp_paint->paint);
        }
        if (ts->gp_vertexpaint) {
          clear_palette(&ts->gp_vertexpaint->paint);
        }
        if (ts->gp_sculptpaint) {
          clear_palette(&ts->gp_sculptpaint->paint);
        }
        if (ts->gp_weightpaint) {
          clear_palette(&ts->gp_weightpaint->paint);
        }
        if (ts->curves_sculpt) {
          clear_palette(&ts->curves_sculpt->paint);
        }
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 26)) {
    /* Bucket 3 LightProbe permanent home (Blended 0.7.0): OB_LIGHTPROBE objects become
     * OB_LAMP objects with an LA_PROBE_* light type. Copy all probe fields from the legacy
     * LightProbe data-block into a new Light, then retarget ob->data. */
    for (Object &ob_ref : bmain->objects) {
      Object *ob = &ob_ref;
      if (ob->type != OB_LIGHTPROBE) {
        continue;
      }
      const LightProbe *probe = static_cast<const LightProbe *>(static_cast<const void *>(ob->data));
      Light *la = BKE_light_add(bmain, ob->id.name + 2);
      BKE_lightprobe_type_apply_to_light(la, probe ? probe->type : LIGHTPROBE_TYPE_SPHERE);
      if (probe) {
        la->probe_flag = probe->flag;
        la->probe_attenuation_type = probe->attenuation_type;
        la->probe_parallax_type = probe->parallax_type;
        la->probe_grid_flag = probe->grid_flag;
        la->probe_distinf = probe->distinf;
        la->probe_distpar = probe->distpar;
        la->probe_falloff = probe->falloff;
        la->probe_clipsta = probe->clipsta;
        la->probe_clipend = probe->clipend;
        la->probe_vis_bias = probe->vis_bias;
        la->probe_vis_bleedbias = probe->vis_bleedbias;
        la->probe_vis_blur = probe->vis_blur;
        la->probe_intensity = probe->intensity;
        la->probe_grid_resolution_x = probe->grid_resolution_x;
        la->probe_grid_resolution_y = probe->grid_resolution_y;
        la->probe_grid_resolution_z = probe->grid_resolution_z;
        la->probe_grid_bake_samples = probe->grid_bake_samples;
        la->probe_grid_surface_bias = probe->grid_surface_bias;
        la->probe_grid_escape_bias = probe->grid_escape_bias;
        la->probe_grid_normal_bias = probe->grid_normal_bias;
        la->probe_grid_view_bias = probe->grid_view_bias;
        la->probe_grid_facing_bias = probe->grid_facing_bias;
        la->probe_grid_validity_threshold = probe->grid_validity_threshold;
        la->probe_grid_dilation_threshold = probe->grid_dilation_threshold;
        la->probe_grid_dilation_radius = probe->grid_dilation_radius;
        la->probe_grid_clamp_direct = probe->grid_clamp_direct;
        la->probe_grid_clamp_indirect = probe->grid_clamp_indirect;
        la->probe_grid_surfel_density = probe->grid_surfel_density;
        la->probe_visibility_grp = probe->visibility_grp;
        la->probe_data_display_size = probe->data_display_size;
      }
      if (ob->data) {
        id_us_min(static_cast<ID *>(ob->data));
      }
      ob->type = OB_LAMP;
      ob->data = static_cast<ID *>(static_cast<void *>(la));
      id_us_plus(&la->id);
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 27)) {
    /* Bucket 3 Mask permanent home (Blended 0.7.0): CMP_NODE_MASK nodes used to reference a
     * Mask ID from bmain->masks via node->id.  Migrate the mask data into per-node storage
     * (NodeCompositeMask) so that masks are owned by the compositor NodeTree, not bmain.
     * Sequencer strip->mask and StripModifierData::mask_id references are nullified
     * (Category A data loss: VSE mask input is rare; data cannot be safely forwarded). */
    FOREACH_NODETREE_BEGIN (bmain, ntree, id_owner) {
      if (ntree->type != NTREE_COMPOSIT) {
        continue;
      }
      for (bNode &node : ntree->nodes) {
        if (node.type_legacy != CMP_NODE_MASK) {
          continue;
        }
        if (!node.id) {
          /* Already migrated or never assigned — ensure storage exists. */
          if (!node.storage) {
            NodeCompositeMask *data = MEM_new<NodeCompositeMask>(__func__);
            data->mask = BKE_mask_new_nodetree("Mask");
            node.storage = data;
          }
          continue;
        }
        const Mask *src_mask = reinterpret_cast<const Mask *>(node.id);
        NodeCompositeMask *data = MEM_new<NodeCompositeMask>(__func__);
        data->mask = BKE_mask_copy_nodetree(src_mask);
        /* Populate metadata from the migrated mask so the struct is fully initialised. */
        data->sfra = src_mask->sfra;
        data->efra = src_mask->efra;
        data->flag = src_mask->flag;
        data->masklay_act = src_mask->masklay_act;
        STRNCPY(data->name, src_mask->id.name + 2);
        node.storage = data;
        /* Release the ID reference; bmain->masks drain in readfile.cc will free it. */
        id_us_min(node.id);
        node.id = nullptr;
      }
    }
    FOREACH_NODETREE_END;

    /* Null sequencer mask references — their Mask blocks will be drained from bmain->masks. */
    for (Scene &scene_ref : bmain->scenes) {
      Scene *scene = &scene_ref;
      if (!scene->ed) {
        continue;
      }
      seq::foreach_strip(&scene->ed->seqbase, [](Strip *strip) -> bool {
        strip->mask = nullptr;
        StripModifierData *mod = static_cast<StripModifierData *>(strip->modifiers.first);
        while (mod) {
          StripModifierData *next = static_cast<StripModifierData *>(mod->next);
          mod->mask_id = nullptr;
          mod = next;
        }
        return true;
      });
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 29)) {
    /* Bucket 3 Lattice permanent home (Blended 0.7.0): standard LatticeModifier used to
     * reference an OB_LATTICE object via lmd->object. Migrate Lattice geometry into per-modifier
     * embedded storage (lmd->lattice) and null lmd->object.
     *
     * object_to_lattice is set to identity — a Category A limitation: deformation may appear
     * shifted for setups where the OB_LATTICE had a non-identity transform relative to the
     * modified object. Users should adjust control points manually if needed.
     *
     * OB_LATTICE objects are converted to OB_EMPTY so their ob->data pointers are safely nulled
     * before bmain->lattices is drained. GreasePencilLatticeModifier object references are nulled
     * (Category A: GP lattice deformation silently drops for legacy GP lattice modifiers). */

    /* Step 1: migrate each standard LatticeModifier lmd->object → lmd->lattice. */
    for (Object &ob : bmain->objects) {
      for (ModifierData *md = static_cast<ModifierData *>(ob.modifiers.first); md;
           md = static_cast<ModifierData *>(md->next))
      {
        if (md->type != eModifierType_Lattice) {
          continue;
        }
        LatticeModifierData *lmd = reinterpret_cast<LatticeModifierData *>(md);
        if (lmd->object && lmd->object->type == OB_LATTICE && !lmd->lattice) {
          const Lattice *src_lt = reinterpret_cast<const Lattice *>(lmd->object->data);
          lmd->lattice = src_lt ? BKE_lattice_copy_modifier(src_lt) :
                                  BKE_lattice_new_modifier("Lattice");
        }
        if (!lmd->lattice) {
          lmd->lattice = BKE_lattice_new_modifier("Lattice");
        }
        unit_m4(lmd->object_to_lattice);
        lmd->object = nullptr;
      }
      /* Step 2: null GreasePencilLatticeModifier object refs so they don't dangle after
       * OB_LATTICE → OB_EMPTY conversion (Category A: GP lattice deformation silently drops). */
      for (ModifierData *md = static_cast<ModifierData *>(ob.modifiers.first); md;
           md = static_cast<ModifierData *>(md->next))
      {
        if (md->type == eModifierType_GreasePencilLattice) {
          reinterpret_cast<GreasePencilLatticeModifierData *>(md)->object = nullptr;
        }
      }
    }

    /* Step 3: convert OB_LATTICE → OB_EMPTY so ob->data doesn't dangle after the drain. */
    for (Object &ob : bmain->objects) {
      if (ob.type == OB_LATTICE) {
        ob.type = OB_EMPTY;
        ob.data = nullptr;
        ob.empty_drawtype = OB_PLAINAXES;
      }
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 30)) {
    /* Bucket 3 Brush permanent home (Blended 0.7.0): mark all pre-existing brushes as
     * project-local so they survive the post-read drain. Brushes without BRUSH_PROJECT_LOCAL
     * are treated as transient defaults regenerated by paint-mode init on demand and are freed
     * from bmain->brushes on every file load. Legacy files had all brushes as project data,
     * so we preserve that by setting the flag unconditionally here. */
    for (Brush &brush : bmain->brushes) {
      brush.flag2 |= BRUSH_PROJECT_LOCAL;
    }
  }

  if (!MAIN_VERSION_FILE_ATLEAST(bmain, 502, 31)) {
    /* OB_MBALL (value 5) was removed in Blended 0.4.0 when ID_MB was cut.
     * bmain->metaballs was fully removed — no Scar 2 rescue — so ob->data is already
     * nullptr on any MetaBall object from a legacy file. Convert to OB_EMPTY to prevent
     * null-deref crashes in draw/eval dispatch. */
    int mball_count = 0;
    for (Object &object : bmain->objects) {
      if (object.type == 5 /* OB_MBALL */) {
        object.type = OB_EMPTY;
        object.data = nullptr;
        mball_count++;
      }
    }
    if (mball_count > 0 && fd && fd->reports) {
      fd->reports->count.mball_converted += mball_count;
    }
  }

  /**
   * Always bump subversion in BKE_blender_version.h when adding versioning
   * code here, and wrap it inside a MAIN_VERSION_FILE_ATLEAST check.
   *
   * \note Keep this message at the bottom of the function.
   */
}

}  // namespace blender
