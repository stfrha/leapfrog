"""
Materials shared by all worlds (things that belong to the Leapfrog's own
technology, like the white hex plating of the ship).
"""

import os

import bpy

METAL_PLATE_DIR = os.path.join("Materials", "By texture", "Metal Plate")


def _image(models_dir, folder, filename, non_color):
    path = os.path.join(models_dir, folder, filename)
    if not os.path.exists(path):
        raise FileNotFoundError("texture not found: %s" % path)
    img = bpy.data.images.load(path, check_existing=True)
    if non_color:
        img.colorspace_settings.name = 'Non-Color'
    return img


def hex_plate(models_dir, name="hex_plate", uv_map="UVMap", normal_strength=1.0, bump_strength=0.3):
    """
    The white hexagonal plating of the Leapfrog (Metal_Plate_029).
    Needs UVs in texture tiles (one tile = 8 hexes across); the scale is
    chosen by the geometry's UVs, e.g. sprite_kit.lathe(tile=...).
    """
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    n = nt.nodes
    l = nt.links
    n.clear()

    out = n.new("ShaderNodeOutputMaterial")
    bsdf = n.new("ShaderNodeBsdfPrincipled")
    l.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    bsdf.inputs["Metallic"].default_value = 0.0

    uv = n.new("ShaderNodeUVMap")
    uv.uv_map = uv_map

    def tex(filename, non_color):
        t = n.new("ShaderNodeTexImage")
        t.image = _image(models_dir, METAL_PLATE_DIR, filename, non_color)
        t.interpolation = 'Cubic'
        l.new(uv.outputs["UV"], t.inputs["Vector"])
        return t

    base = tex("Metal_Plate_029_basecolor.jpg", False)
    rough = tex("Metal_Plate_029_roughness.jpg", True)
    normal = tex("Metal_Plate_029_normal.jpg", True)
    height = tex("Metal_Plate_029_height.png", True)

    l.new(base.outputs["Color"], bsdf.inputs["Base Color"])
    l.new(rough.outputs["Color"], bsdf.inputs["Roughness"])

    nmap = n.new("ShaderNodeNormalMap")
    nmap.uv_map = uv_map
    nmap.inputs["Strength"].default_value = normal_strength
    l.new(normal.outputs["Color"], nmap.inputs["Color"])

    bump = n.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = bump_strength
    l.new(height.outputs["Color"], bump.inputs["Height"])
    l.new(nmap.outputs["Normal"], bump.inputs["Normal"])
    l.new(bump.outputs["Normal"], bsdf.inputs["Normal"])

    return mat


def burnt_metal(name="burnt_metal"):
    """Dark, slightly heat-tinted metal for nozzles and engine parts."""
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    n = nt.nodes
    l = nt.links
    n.clear()

    out = n.new("ShaderNodeOutputMaterial")
    bsdf = n.new("ShaderNodeBsdfPrincipled")
    l.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    bsdf.inputs["Metallic"].default_value = 0.85

    coord = n.new("ShaderNodeTexCoord")
    noise = n.new("ShaderNodeTexNoise")
    noise.inputs["Scale"].default_value = 3.0
    noise.inputs["Detail"].default_value = 6.0
    l.new(coord.outputs["Object"], noise.inputs["Vector"])

    ramp = n.new("ShaderNodeValToRGB")
    ramp.color_ramp.elements[0].position = 0.35
    ramp.color_ramp.elements[0].color = (0.035, 0.035, 0.04, 1.0)
    ramp.color_ramp.elements[1].position = 0.75
    ramp.color_ramp.elements[1].color = (0.12, 0.09, 0.08, 1.0)
    l.new(noise.outputs["Fac"], ramp.inputs["Fac"])
    l.new(ramp.outputs["Color"], bsdf.inputs["Base Color"])

    rough = n.new("ShaderNodeMapRange")
    rough.inputs["To Min"].default_value = 0.35
    rough.inputs["To Max"].default_value = 0.6
    l.new(noise.outputs["Fac"], rough.inputs["Value"])
    l.new(rough.outputs["Result"], bsdf.inputs["Roughness"])

    return mat
