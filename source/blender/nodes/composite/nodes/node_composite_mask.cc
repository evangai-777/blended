/* SPDX-FileCopyrightText: 2012 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "MEM_guardedalloc.h"

#include "BLI_math_base.hh"
#include "BLI_string_utf8.h"

#include "DNA_mask_types.h"
#include "DNA_node_types.h"

#include "BKE_mask.hh"

#include "BLO_read_write.hh"

#include "UI_interface.hh"
#include "UI_interface_layout.hh"

#include "COM_cached_mask.hh"
#include "COM_node_operation.hh"

#include "node_composite_util.hh"

namespace blender::nodes::node_composite_mask_cc {

NODE_STORAGE_FUNCS(NodeCompositeMask)

static const EnumPropertyItem size_source_items[] = {
    {0, "SCENE", 0, "Scene Size", ""},
    {CMP_NODE_MASK_FLAG_SIZE_FIXED, "FIXED", 0, N_("Fixed"), N_("Use pixel size for the buffer")},
    {CMP_NODE_MASK_FLAG_SIZE_FIXED_SCENE,
     "FIXED_SCENE",
     0,
     N_("Fixed/Scene"),
     N_("Pixel size scaled by scene percentage")},
    {0, nullptr, 0, nullptr, nullptr},
};

static void node_declare(NodeDeclarationBuilder &b)
{
  b.use_custom_socket_order();

  b.add_output<decl::Float>("Mask"_ustr).structure_type(StructureType::Dynamic);

  b.add_input<decl::Menu>("Size Source"_ustr)
      .default_value(MenuValue(0))
      .static_items(size_source_items)
      .optional_label()
      .description("The source where the size of the mask is retrieved");
  b.add_input<decl::Int>("Size X"_ustr)
      .default_value(256)
      .min(1)
      .usage_by_menu("Size Source"_ustr,
                     {CMP_NODE_MASK_FLAG_SIZE_FIXED, CMP_NODE_MASK_FLAG_SIZE_FIXED_SCENE})
      .description("The resolution of the mask along the X direction");
  b.add_input<decl::Int>("Size Y"_ustr)
      .default_value(256)
      .min(1)
      .usage_by_menu("Size Source"_ustr,
                     {CMP_NODE_MASK_FLAG_SIZE_FIXED, CMP_NODE_MASK_FLAG_SIZE_FIXED_SCENE})
      .description("The resolution of the mask along the Y direction");
  b.add_input<decl::Bool>("Feather"_ustr)
      .default_value(true)
      .description("Use feather information from the mask");

  PanelDeclarationBuilder &motion_blur_panel =
      b.add_panel("Motion Blur"_ustr).default_closed(true);
  motion_blur_panel.add_input<decl::Bool>("Motion Blur"_ustr)
      .default_value(false)
      .panel_toggle()
      .description("Use multi-sampled motion blur of the mask");
  motion_blur_panel.add_input<decl::Int>("Samples"_ustr, "Motion Blur Samples"_ustr)
      .default_value(16)
      .min(1)
      .max(64)
      .description("Number of motion blur samples");
  motion_blur_panel.add_input<decl::Float>("Shutter"_ustr, "Motion Blur Shutter"_ustr)
      .default_value(0.5f)
      .subtype(PROP_FACTOR)
      .min(0.0f)
      .max(1.0f)
      .description("Exposure for motion blur as a factor of FPS");
}

static void node_label(const bNodeTree * /*ntree*/,
                       const bNode *node,
                       char *label,
                       int label_maxncpy)
{
  const NodeCompositeMask *data = static_cast<const NodeCompositeMask *>(node->storage);
  const char *name = (data && data->mask) ? data->mask->id.name + 2 : IFACE_("Mask");
  BLI_strncpy_utf8(label, name, label_maxncpy);
}

static void node_init(bNodeTree * /*ntree*/, bNode *node)
{
  NodeCompositeMask *data = MEM_new<NodeCompositeMask>(__func__);
  data->mask = BKE_mask_new_nodetree("Mask");
  node->storage = data;
}

static void node_free(bNode *node)
{
  NodeCompositeMask *data = static_cast<NodeCompositeMask *>(node->storage);
  if (data) {
    BKE_mask_free_nodetree(data->mask);
    MEM_delete(data);
  }
}

static void node_copy(bNodeTree * /*dest_ntree*/, bNode *dest_node, const bNode *src_node)
{
  const NodeCompositeMask *src = static_cast<const NodeCompositeMask *>(src_node->storage);
  NodeCompositeMask *dst = MEM_new<NodeCompositeMask>(__func__);
  dst->mask = BKE_mask_copy_nodetree(src ? src->mask : nullptr);
  dest_node->storage = dst;
}

static void node_blend_write(const bNodeTree & /*ntree*/,
                             const bNode &node,
                             BlendWriter &writer)
{
  const NodeCompositeMask *data = static_cast<const NodeCompositeMask *>(node.storage);
  if (data) {
    BKE_mask_write_layers(&writer, data->mask);
  }
}

static void node_blend_read(bNodeTree & /*ntree*/, bNode &node, BlendDataReader &reader)
{
  NodeCompositeMask *data = static_cast<NodeCompositeMask *>(node.storage);
  if (data) {
    data->mask = BKE_mask_new_nodetree("Mask");
    BKE_mask_read_layers(&reader, data->mask);
  }
}

using namespace blender::compositor;

class MaskOperation : public NodeOperation {
 public:
  using NodeOperation::NodeOperation;

  void execute() override
  {
    if (!this->get_mask()) {
      this->allocate_default_remaining_outputs();
      return;
    }

    const Domain domain = compute_domain();
    Result &cached_mask = context().cache_manager().cached_masks.get(
        this->context(),
        this->get_mask(),
        domain,
        this->get_aspect_ratio(),
        this->get_use_feather(),
        this->context().get_frame_number(),
        this->get_motion_blur_samples(),
        this->get_motion_blur_shutter(),
        false);

    Result &output_mask = this->get_result("Mask");
    output_mask.share_data(cached_mask);
  }

  Domain compute_domain() override
  {
    if (this->get_flags() & CMP_NODE_MASK_FLAG_SIZE_FIXED) {
      return Domain(this->get_size());
    }

    if (this->get_flags() & CMP_NODE_MASK_FLAG_SIZE_FIXED_SCENE) {
      return Domain(this->get_size() * this->context().get_render_percentage());
    }

    return this->context().get_compositing_domain();
  }

  int2 get_size()
  {
    return int2(math::max(1, this->get_input("Size X").get_single_value_default<int>()),
                math::max(1, this->get_input("Size Y").get_single_value_default<int>()));
  }

  float get_aspect_ratio()
  {
    if (this->is_fixed_size()) {
      return 1.0f;
    }

    return this->context().get_render_data().yasp / this->context().get_render_data().xasp;
  }

  bool is_fixed_size()
  {
    return this->get_flags() &
           (CMP_NODE_MASK_FLAG_SIZE_FIXED | CMP_NODE_MASK_FLAG_SIZE_FIXED_SCENE);
  }

  bool get_use_feather()
  {
    return this->get_input("Feather").get_single_value_default<bool>();
  }

  int get_motion_blur_samples()
  {
    const int samples = math::clamp(
        this->get_input("Motion Blur Samples").get_single_value_default<int>(), 1, 64);
    return this->use_motion_blur() ? samples : 1;
  }

  float get_motion_blur_shutter()
  {
    return math::clamp(
        this->get_input("Motion Blur Shutter").get_single_value_default<float>(), 0.0f, 1.0f);
  }

  bool use_motion_blur()
  {
    return this->get_input("Motion Blur").get_single_value_default<bool>();
  }

  CMPNodeMaskFlags get_flags()
  {
    return CMPNodeMaskFlags(
        this->get_input("Size Source").get_single_value_default<MenuValue>().value);
  }

  Mask *get_mask()
  {
    const NodeCompositeMask *data = static_cast<const NodeCompositeMask *>(this->node().storage);
    return data ? data->mask : nullptr;
  }
};

static NodeOperation *get_compositor_operation(Context &context, const bNode &node)
{
  return new MaskOperation(context, node);
}

static void node_register()
{
  static bke::bNodeType ntype;

  cmp_node_type_base(&ntype, "CompositorNodeMask"_ustr, CMP_NODE_MASK);
  ntype.ui_name = "Mask";
  ntype.ui_description = "Input mask, node-owned and embedded in the compositor node tree";
  ntype.enum_name_legacy = "MASK";
  ntype.nclass = NODE_CLASS_INPUT;
  ntype.declare = node_declare;
  ntype.labelfunc = node_label;
  ntype.initfunc = node_init;
  ntype.get_compositor_operation = get_compositor_operation;
  ntype.blend_write_storage_content = node_blend_write;
  ntype.blend_data_read_storage_content = node_blend_read;
  bke::node_type_storage(ntype, "NodeCompositeMask", node_free, node_copy);

  bke::node_register_type(ntype);
}
NOD_REGISTER_NODE(node_register)

}  // namespace blender::nodes::node_composite_mask_cc
