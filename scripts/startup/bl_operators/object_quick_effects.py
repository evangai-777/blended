# SPDX-FileCopyrightText: 2011-2023 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

from mathutils import Vector
import bpy
from bpy.types import Operator
from bpy.props import (
    BoolProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
)
from bpy.app.translations import (
    pgettext_rpt as rpt_,
    pgettext_data as data_,
)


def object_ensure_material(obj, mat_name):
    """ Use an existing material or add a new one.
    """
    mat = mat_slot = None
    for mat_slot in obj.material_slots:
        mat = mat_slot.material
        if mat:
            break
    if mat is None:
        mat = bpy.data.materials.new(mat_name)
        mat.node_tree.nodes.clear()
        if mat_slot:
            mat_slot.material = mat
        else:
            obj.data.materials.append(mat)
    return mat


class ObjectModeOperator:
    @classmethod
    def poll(cls, context):
        return context.mode == 'OBJECT'


class QuickFur(ObjectModeOperator, Operator):
    """Add a fur setup to the selected objects"""
    bl_idname = "object.quick_fur"
    bl_label = "Quick Fur"
    bl_options = {'REGISTER', 'UNDO'}

    density: EnumProperty(
        name="Density",
        items=(
            ('LOW', "Low", ""),
            ('MEDIUM', "Medium", ""),
            ('HIGH', "High", ""),
        ),
        default='MEDIUM',
    )
    length: FloatProperty(
        name="Length",
        min=0.001, max=100,
        soft_min=0.01, soft_max=10,
        default=0.1,
        subtype='DISTANCE',
    )
    radius: FloatProperty(
        name="Hair Radius",
        min=0.0, max=10,
        soft_min=0.0001, soft_max=0.1,
        default=0.001,
        subtype='DISTANCE',
    )
    view_percentage: FloatProperty(
        name="View Percentage",
        min=0.0, max=1.0,
        default=1.0,
        subtype='FACTOR',
    )
    apply_hair_guides: BoolProperty(
        name="Apply Hair Guides",
        default=True,
    )
    use_noise: BoolProperty(
        name="Noise",
        default=True,
    )
    use_frizz: BoolProperty(
        name="Frizz",
        default=True,
    )

    def execute(self, context):
        import os
        from collections import namedtuple

        mesh_objects = [obj for obj in context.selected_objects if obj.type == 'MESH']
        if not mesh_objects:
            self.report({'ERROR'}, "Select at least one mesh object")
            return {'CANCELLED'}

        if self.density == 'LOW':
            count = 1000
        elif self.density == 'MEDIUM':
            count = 10000
        elif self.density == 'HIGH':
            count = 100000

        asset_library_filepath = os.path.join(
            bpy.utils.system_resource('DATAFILES'),
            "assets",
            "nodes",
            "procedural_hair_node_assets.blend",
        )

        # Create a named tuple that stores attributes for the node-group names.
        attr_name_pairs = [
            ("generate", "Generate Hair Curves"),
            ("interpolate", "Interpolate Hair Curves"),
            ("radius", "Set Hair Curve Profile"),

        ]
        if self.use_noise:
            attr_name_pairs.append(("noise", "Hair Curves Noise"))
        if self.use_frizz:
            attr_name_pairs.append(("frizz", "Frizz Hair Curves"))

        NodeGroupData = namedtuple("NodeGroupData", tuple(v for v, _ in attr_name_pairs))

        with bpy.data.libraries.load(
                asset_library_filepath,
                link=True,
                pack=True,
                set_fake=False,
        ) as (data_src, data_dst):
            # The values are assumed to exist, no inspection of the source is needed.
            del data_src
            data_dst.node_groups.extend([name for _, name in attr_name_pairs])

        # For convenient name lookups.
        node_groups_name_map = {id.name: id for id in data_dst.node_groups}
        node_groups = NodeGroupData(*(node_groups_name_map[name] for _, name in attr_name_pairs))
        del node_groups_name_map

        material = bpy.data.materials.new(data_("Fur Material"))

        mesh_with_zero_area = False
        mesh_missing_uv_map = False
        modifier_apply_error = False

        for mesh_object in mesh_objects:
            mesh = mesh_object.data
            if len(mesh.uv_layers) == 0:
                mesh_missing_uv_map = True
                continue

            with context.temp_override(active_object=mesh_object):
                bpy.ops.object.curves_empty_hair_add()
            curves_object = context.active_object
            curves = curves_object.data
            curves.materials.append(material)

            area = 0.0
            for poly in mesh.polygons:
                area += poly.area
            if area == 0.0:
                mesh_with_zero_area = True
                density = 10
            else:
                density = count / area

            generate_modifier = curves_object.modifiers.new(name=data_("Generate"), type='NODES')
            generate_modifier.node_group = node_groups.generate
            generate_modifier.properties.inputs.Input_2.value = mesh_object
            generate_modifier.properties.inputs.Input_18.attribute_name = curves.surface_uv_map
            generate_modifier.properties.inputs.Input_12.value = True
            generate_modifier.properties.inputs.Input_20.value = self.length
            generate_modifier.properties.inputs.Input_22.value = material
            generate_modifier.properties.inputs.Input_15.value = density * 0.01

            radius_modifier = curves_object.modifiers.new(name=data_("Set Hair Curve Profile"), type='NODES')
            radius_modifier.node_group = node_groups.radius
            radius_modifier.properties.inputs.Input_3.value = self.radius

            interpolate_modifier = curves_object.modifiers.new(name=data_("Interpolate Hair Curves"), type='NODES')
            interpolate_modifier.node_group = node_groups.interpolate
            interpolate_modifier.properties.inputs.Input_2.value = mesh_object
            interpolate_modifier.properties.inputs.Input_18.attribute_name = curves.surface_uv_map
            interpolate_modifier.properties.inputs.Input_12.value = True
            interpolate_modifier.properties.inputs.Input_15.value = density
            interpolate_modifier.properties.inputs.Input_17.value = self.view_percentage
            interpolate_modifier.properties.inputs.Input_24.value = True

            if self.use_noise:
                noise_modifier = curves_object.modifiers.new(name=data_("Hair Curves Noise"), type='NODES')
                noise_modifier.node_group = node_groups.noise

            if self.use_frizz:
                frizz_modifier = curves_object.modifiers.new(name=data_("Frizz Hair Curves"), type='NODES')
                frizz_modifier.node_group = node_groups.frizz

            if self.apply_hair_guides:
                with context.temp_override(object=curves_object):
                    try:
                        bpy.ops.object.modifier_apply(modifier=generate_modifier.name)
                    except Exception:
                        modifier_apply_error = True

            curves_object.modifiers.move(0, len(curves_object.modifiers) - 1)

        if mesh_with_zero_area:
            self.report({'WARNING'}, "Mesh has no face area")
        if mesh_missing_uv_map:
            self.report({'WARNING'}, "Mesh UV map required")
        if modifier_apply_error and not mesh_with_zero_area:
            self.report({'WARNING'}, "Unable to apply \"Generate\" modifier")

        return {'FINISHED'}


def obj_bb_minmax(obj, min_co, max_co):
    for i in range(0, 8):
        bb_vec = obj.matrix_world @ Vector(obj.bound_box[i])

        min_co[0] = min(bb_vec[0], min_co[0])
        min_co[1] = min(bb_vec[1], min_co[1])
        min_co[2] = min(bb_vec[2], min_co[2])
        max_co[0] = max(bb_vec[0], max_co[0])
        max_co[1] = max(bb_vec[1], max_co[1])
        max_co[2] = max(bb_vec[2], max_co[2])


def grid_location(x, y):
    return (x * 200, y * 150)


class QuickSmoke(ObjectModeOperator, Operator):
    """Use selected objects as smoke emitters"""
    bl_idname = "object.quick_smoke"
    bl_label = "Quick Smoke"
    bl_options = {'REGISTER', 'UNDO'}

    style: EnumProperty(
        name="Smoke Style",
        items=(
            ('SMOKE', "Smoke", ""),
            ('FIRE', "Fire", ""),
            ('BOTH', "Smoke & Fire", ""),
        ),
        default='SMOKE',
    )

    show_flows: BoolProperty(
        name="Render Smoke Objects",
        description="Keep the smoke objects visible during rendering",
        default=False,
    )

    def execute(self, context):
        if not bpy.app.build_options.fluid:
            self.report({'ERROR'}, "Built without Fluid modifier")
            return {'CANCELLED'}

        mesh_objects = [
            obj for obj in context.selected_objects
            if obj.type == 'MESH'
        ]
        min_co = Vector((100000.0, 100000.0, 100000.0))
        max_co = -min_co

        if not mesh_objects:
            self.report({'ERROR'}, "Select at least one mesh object")
            return {'CANCELLED'}

        for obj in mesh_objects:
            fluid = obj.modifiers.new(name=data_("FLUID"), type='FLUID')
            fluid.fluid_type = 'FLOW'

            # set type
            fluid.flow_settings.flow_type = self.style

            # set flow behavior
            fluid.flow_settings.flow_behavior = 'INFLOW'

            # use some surface distance for smoke emission
            fluid.flow_settings.surface_distance = 1.0

            if not self.show_flows:
                obj.display_type = 'WIRE'

            # store bounding box min/max for the domain object
            obj_bb_minmax(obj, min_co, max_co)

        # add the smoke domain object
        bpy.ops.mesh.primitive_cube_add()
        obj = context.active_object
        obj.name = data_("Smoke Domain")

        # give the smoke some room above the flows
        obj.location = 0.5 * (max_co + min_co) + Vector((0.0, 0.0, 1.0))
        obj.scale = 0.5 * (max_co - min_co) + Vector((1.0, 1.0, 2.0))

        # setup smoke domain
        fluid = obj.modifiers.new(name=data_("FLUID"), type='FLUID')
        fluid.fluid_type = 'DOMAIN'
        # The default value leads to unstable simulations (see #126924).
        fluid.domain_settings.cfl_condition = 4.0
        if self.style == {'FIRE', 'BOTH'}:
            fluid.domain_settings.use_noise = True

        # ensure correct cache file format for smoke
        if bpy.app.build_options.openvdb:
            fluid.domain_settings.cache_data_format = 'OPENVDB'

        # Setup material

        # Cycles and EEVEE.
        bpy.ops.object.material_slot_add()

        mat = bpy.data.materials.new(data_("Smoke Domain Material"))
        obj.material_slots[0].material = mat

        # Set node variables and clear the default nodes
        tree = mat.node_tree
        nodes = tree.nodes
        links = tree.links

        nodes.clear()

        # Create shader nodes

        # Material output
        node_out = nodes.new(type='ShaderNodeOutputMaterial')
        node_out.location = grid_location(6, 1)

        # Add Principled Volume
        node_principled = nodes.new(type='ShaderNodeVolumePrincipled')
        node_principled.location = grid_location(4, 1)
        links.new(node_principled.outputs["Volume"], node_out.inputs["Volume"])

        node_principled.inputs["Density"].default_value = 5.0

        if self.style in {'FIRE', 'BOTH'}:
            node_principled.inputs["Blackbody Intensity"].default_value = 1.0

        return {'FINISHED'}


class QuickLiquid(Operator):
    """Make selected objects liquid"""
    bl_idname = "object.quick_liquid"
    bl_label = "Quick Liquid"
    bl_options = {'REGISTER', 'UNDO'}

    show_flows: BoolProperty(
        name="Render Liquid Objects",
        description="Keep the liquid objects visible during rendering",
        default=False,
    )

    def execute(self, context):
        if not bpy.app.build_options.fluid:
            self.report({'ERROR'}, "Built without Fluid modifier")
            return {'CANCELLED'}

        mesh_objects = [
            obj for obj in context.selected_objects
            if obj.type == 'MESH'
        ]
        min_co = Vector((100000.0, 100000.0, 100000.0))
        max_co = -min_co

        if not mesh_objects:
            self.report({'ERROR'}, "Select at least one mesh object")
            return {'CANCELLED'}

        # set shading type to wireframe so that liquid particles are visible
        for area in bpy.context.screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        space.shading.type = 'WIREFRAME'

        for obj in mesh_objects:
            fluid = obj.modifiers.new(name=data_("FLUID"), type='FLUID')
            fluid.fluid_type = 'FLOW'

            # set type
            fluid.flow_settings.flow_type = 'LIQUID'

            # set flow behavior
            fluid.flow_settings.flow_behavior = 'GEOMETRY'

            # use some surface distance for smoke emission
            fluid.flow_settings.surface_distance = 0.0

            if not self.show_flows:
                obj.display_type = 'WIRE'

            # store bounding box min/max for the domain object
            obj_bb_minmax(obj, min_co, max_co)

        # add the liquid domain object
        bpy.ops.mesh.primitive_cube_add(align='WORLD')
        obj = context.active_object
        obj.name = data_("Liquid Domain")

        # give the liquid some room above the flows
        obj.location = 0.5 * (max_co + min_co) + Vector((0.0, 0.0, -1.0))
        obj.scale = 0.5 * (max_co - min_co) + Vector((1.0, 1.0, 2.0))

        # setup liquid domain
        fluid = obj.modifiers.new(name=data_("FLUID"), type='FLUID')
        fluid.fluid_type = 'DOMAIN'
        # set all domain borders to obstacle
        fluid.domain_settings.use_collision_border_front = True
        fluid.domain_settings.use_collision_border_back = True
        fluid.domain_settings.use_collision_border_right = True
        fluid.domain_settings.use_collision_border_left = True
        fluid.domain_settings.use_collision_border_top = True
        fluid.domain_settings.use_collision_border_bottom = True

        # ensure correct cache file formats for liquid
        if bpy.app.build_options.openvdb:
            fluid.domain_settings.cache_data_format = 'OPENVDB'
        fluid.domain_settings.cache_mesh_format = 'BOBJECT'

        # change domain type, will also allocate and show particle system for FLIP
        fluid.domain_settings.domain_type = 'LIQUID'

        # set color mapping field to show phi grid for liquid
        fluid.domain_settings.color_ramp_field = 'PHI'

        # perform a single slice of the domain
        fluid.domain_settings.use_slice = True

        # set display thickness to a lower value for more detailed display of phi grids
        fluid.domain_settings.display_thickness = 0.02

        # make the domain smooth so it renders nicely
        bpy.ops.object.shade_smooth()

        # create a ray-transparent material for the domain
        bpy.ops.object.material_slot_add()

        mat = bpy.data.materials.new(data_("Liquid Domain Material"))
        obj.material_slots[0].material = mat

        # Set node variables and clear the default nodes
        tree = mat.node_tree
        nodes = tree.nodes
        links = tree.links

        nodes.clear()

        # Create shader nodes

        # Material output
        node_out = nodes.new(type='ShaderNodeOutputMaterial')
        node_out.location = grid_location(6, 1)

        # Add Glass
        node_glass = nodes.new(type='ShaderNodeBsdfGlass')
        node_glass.location = grid_location(4, 1)
        links.new(node_glass.outputs["BSDF"], node_out.inputs["Surface"])
        node_glass.inputs["IOR"].default_value = 1.33

        # Add Absorption
        node_absorption = nodes.new(type='ShaderNodeVolumeAbsorption')
        node_absorption.location = grid_location(4, 2)
        links.new(node_absorption.outputs["Volume"], node_out.inputs["Volume"])
        node_absorption.inputs["Color"].default_value = (0.8, 0.9, 1.0, 1.0)

        return {'FINISHED'}


classes = (
    QuickFur,
    QuickSmoke,
    QuickLiquid,
)
