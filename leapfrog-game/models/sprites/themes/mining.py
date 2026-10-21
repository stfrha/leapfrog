"""
Theme: the home mining planet (launch site, silo).

Lighting follows silo_home.blend / leapfrog.blend: the kloetzle_blei HDRI for
soft grey ambient light. Instead of a point light (which lights each part from
a different angle depending on where it is), the key light is a sun, so every
part is lit identically wherever it is in the scene.

Materials are procedural and selected by role:
    frame_main   - heavy structural steel (tower legs), dark grey paint, weathered
    frame_brace  - lighter bracing steel, mid grey paint, weathered
    hull_white   - the Leapfrog's white hex plating (shared.hex_plate, needs UVs)
    nozzle       - dark burnt metal (shared.burnt_metal)
    gravel_white - white stone chips, neutral grey-white (per-stone
                   'stone_rand' attribute), for repeating gravel textures
    gravel_bed   - the dark ground showing between gravel stones
    machine_yellow - mining machines' faded yellow paint, weathered and rusty
    rubber       - big off-road tyres, dusty black
    ore_rock     - loose ore in heaps and truck bodies, rust-brown and grey
"""

import math
import os

import bpy
from mathutils import Vector

from . import shared

# Relative to leapfrog-game/models. Not in git: download from
# https://polyhaven.com/a/kloetzle_blei (see the README).
HDRI_CANDIDATES = [
    os.path.join("..", "..", "..", "leapfrog-hdr", "kloetzle_blei_4k.exr"),   # leapfrog-hdr/ next to the repo
    os.path.join("HDRI", "kloetzle_blei_16k.hdr"),                           # older place, leapfrog-game/models/HDRI
]
HDRI_STRENGTH = 0.6

SUN_DIRECTION = Vector((0.45, 0.75, -0.5))   # light travels right, into the scene, downwards
SUN_STRENGTH = 3.0
SUN_ANGLE_DEG = 8.0                          # soft-ish shadow edges


# ---------------------------------------------------------------------------
# World and lights
# ---------------------------------------------------------------------------

def setup_world(scene, models_dir, hdri=None):
    world = bpy.data.worlds.new("mining")
    scene.world = world
    world.use_nodes = True
    nt = world.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputWorld")
    bg = nt.nodes.new("ShaderNodeBackground")
    nt.links.new(bg.outputs["Background"], out.inputs["Surface"])
    bg.inputs["Strength"].default_value = HDRI_STRENGTH

    candidates = [hdri] if hdri else [os.path.join(models_dir, c) for c in HDRI_CANDIDATES]
    candidates = [os.path.normpath(c) for c in candidates if c]
    path = next((p for p in candidates if os.path.exists(p)), None)
    if path:
        env = nt.nodes.new("ShaderNodeTexEnvironment")
        env.image = bpy.data.images.load(path, check_existing=True)
        nt.links.new(env.outputs["Color"], bg.inputs["Color"])
        print("mining theme: HDRI %s" % path)
    else:
        # Neutral overcast grey, close to the HDRI's average colour.
        bg.inputs["Color"].default_value = (0.42, 0.43, 0.45, 1.0)
        bg.inputs["Strength"].default_value = 1.0
        print("mining theme: HDRI not found (looked for %s), using flat grey sky" % candidates)


def setup_lights(scene, rig):
    sun_data = bpy.data.lights.new("Sun", 'SUN')
    if rig in ("persp", "axo", "tile"):
        # Scenery, ambient pieces and repeating ground textures: softer and a
        # bit dimmer, like the big area light over the silo.
        sun_data.energy = SUN_STRENGTH * 0.75
        sun_data.angle = math.radians(25.0)
    else:
        sun_data.energy = SUN_STRENGTH
        sun_data.angle = math.radians(SUN_ANGLE_DEG)
    sun = bpy.data.objects.new("Sun", sun_data)
    scene.collection.objects.link(sun)
    # A sun shines along its local -Z axis.
    sun.rotation_euler = SUN_DIRECTION.normalized().to_track_quat('-Z', 'Y').to_euler()


# ---------------------------------------------------------------------------
# Materials
# ---------------------------------------------------------------------------

def make_material(role, models_dir):
    if role == "frame_main":
        return _weathered_paint("frame_main", paint=(0.10, 0.10, 0.105), rust=0.35, streak_scale=0.10)
    if role == "frame_brace":
        return _weathered_paint("frame_brace", paint=(0.30, 0.30, 0.31), rust=0.25, streak_scale=0.12)
    if role == "hull_white":
        return shared.hex_plate(models_dir)
    if role == "nozzle":
        return shared.burnt_metal()
    if role == "concrete_dirty":
        return _dirty_concrete("concrete_dirty")
    if role == "pad_metal":
        return _weathered_paint("pad_metal", paint=(0.16, 0.16, 0.17), rust=0.55, streak_scale=0.3)
    if role == "rusty_metal":
        return _weathered_paint("rusty_metal", paint=(0.12, 0.08, 0.06), rust=0.8, streak_scale=0.2)
    if role == "hazard_paint":
        return _weathered_paint("hazard_paint", paint=(0.55, 0.40, 0.03), rust=0.4, streak_scale=0.5)
    if role == "glass_dark":
        return _simple("glass_dark", base=(0.015, 0.02, 0.025), rough=0.12, metallic=0.0)
    if role == "void":
        return _simple("void", base=(0.004, 0.004, 0.005), rough=1.0, metallic=0.0)
    if role == "window_lit_blue":
        return _emission("window_lit_blue", (0.30, 0.55, 1.0), 6.0)
    if role == "neon_pink":
        return _emission("neon_pink", (1.0, 0.12, 0.45), 25.0)
    if role == "neon_amber":
        return _emission("neon_amber", (1.0, 0.45, 0.08), 14.0)
    if role == "window_lit_warm":
        return _emission("window_lit_warm", (1.0, 0.58, 0.25), 5.0)
    if role == "container_red":
        return _weathered_paint("container_red", paint=(0.30, 0.07, 0.04), rust=0.45, streak_scale=0.25)
    if role == "container_blue":
        return _weathered_paint("container_blue", paint=(0.05, 0.12, 0.19), rust=0.45, streak_scale=0.25)
    if role == "ground_dirt":
        return _ground_dirt("ground_dirt")
    if role == "gravel_white":
        return _gravel_stone("gravel_white")
    if role == "gravel_bed":
        return _simple("gravel_bed", base=(0.10, 0.10, 0.10), rough=0.95, metallic=0.0)
    if role == "machine_yellow":
        return _weathered_paint("machine_yellow", paint=(0.50, 0.33, 0.035), rust=0.5, streak_scale=0.3)
    if role == "rubber":
        return _simple("rubber", base=(0.028, 0.027, 0.026), rough=0.88, metallic=0.0)
    if role == "ore_rock":
        return _ground_dirt("ore_rock", dark=(0.07, 0.045, 0.035), light=(0.33, 0.20, 0.13), stone_scale=1.6)
    if role == "neon_dead":
        return _simple("neon_dead", base=(0.10, 0.07, 0.08), rough=0.25, metallic=0.0)
    raise KeyError("mining theme has no material role '%s'" % role)


def _simple(name, base, rough, metallic):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (base[0], base[1], base[2], 1.0)
    bsdf.inputs["Roughness"].default_value = rough
    bsdf.inputs["Metallic"].default_value = metallic
    return mat


def _emission(name, color, strength):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (color[0], color[1], color[2], 1.0)
    em.inputs["Strength"].default_value = strength
    nt.links.new(em.outputs["Emission"], out.inputs["Surface"])
    return mat


def _dirty_concrete(name):
    """
    Worn concrete: mottled grey-beige, dark rain streaks running down,
    dirt splashed up from the ground and a rough, uneven surface.
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

    coord = n.new("ShaderNodeTexCoord")

    # Large mottling
    mottle = n.new("ShaderNodeTexNoise")
    mottle.inputs["Scale"].default_value = 0.35
    mottle.inputs["Detail"].default_value = 6.0
    l.new(coord.outputs["Object"], mottle.inputs["Vector"])
    mottle_ramp = n.new("ShaderNodeValToRGB")
    mottle_ramp.color_ramp.elements[0].color = (0.20, 0.19, 0.17, 1.0)
    mottle_ramp.color_ramp.elements[1].color = (0.42, 0.40, 0.36, 1.0)
    l.new(mottle.outputs["Fac"], mottle_ramp.inputs["Fac"])

    # Rain streaks: noise stretched along Z
    smap = n.new("ShaderNodeMapping")
    smap.inputs["Scale"].default_value = (1.6, 1.6, 0.06)
    l.new(coord.outputs["Object"], smap.inputs["Vector"])
    streaks = n.new("ShaderNodeTexNoise")
    streaks.inputs["Scale"].default_value = 2.0
    streaks.inputs["Detail"].default_value = 4.0
    l.new(smap.outputs["Vector"], streaks.inputs["Vector"])
    streak_ramp = n.new("ShaderNodeValToRGB")
    streak_ramp.color_ramp.elements[0].position = 0.45
    streak_ramp.color_ramp.elements[0].color = (0.35, 0.33, 0.30, 1.0)
    streak_ramp.color_ramp.elements[1].position = 0.65
    streak_ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)
    l.new(streaks.outputs["Fac"], streak_ramp.inputs["Fac"])

    streaked = n.new("ShaderNodeMix")
    streaked.data_type = 'RGBA'
    streaked.blend_type = 'MULTIPLY'
    _socket(streaked.inputs, "Factor_Float").default_value = 1.0
    l.new(mottle_ramp.outputs["Color"], _socket(streaked.inputs, "A_Color"))
    l.new(streak_ramp.outputs["Color"], _socket(streaked.inputs, "B_Color"))

    # Dirt near the ground (world Z, so it follows the ground even on a tilted building)
    geo = n.new("ShaderNodeNewGeometry")
    sep = n.new("ShaderNodeSeparateXYZ")
    l.new(geo.outputs["Position"], sep.inputs["Vector"])
    dirt_ramp = n.new("ShaderNodeMapRange")
    dirt_ramp.inputs["From Min"].default_value = 0.0
    dirt_ramp.inputs["From Max"].default_value = 4.0
    dirt_ramp.inputs["To Min"].default_value = 0.85
    dirt_ramp.inputs["To Max"].default_value = 0.0
    l.new(sep.outputs["Z"], dirt_ramp.inputs["Value"])

    dirty = n.new("ShaderNodeMix")
    dirty.data_type = 'RGBA'
    l.new(dirt_ramp.outputs["Result"], _socket(dirty.inputs, "Factor_Float"))
    l.new(_socket(streaked.outputs, "Result_Color"), _socket(dirty.inputs, "A_Color"))
    _socket(dirty.inputs, "B_Color").default_value = (0.08, 0.06, 0.045, 1.0)
    l.new(_socket(dirty.outputs, "Result_Color"), bsdf.inputs["Base Color"])

    bsdf.inputs["Roughness"].default_value = 0.92

    # Uneven surface
    fine = n.new("ShaderNodeTexNoise")
    fine.inputs["Scale"].default_value = 9.0
    fine.inputs["Detail"].default_value = 8.0
    l.new(coord.outputs["Object"], fine.inputs["Vector"])
    bump = n.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = 0.35
    l.new(fine.outputs["Fac"], bump.inputs["Height"])
    l.new(bump.outputs["Normal"], bsdf.inputs["Normal"])

    return mat


def _ground_dirt(name, dark=(0.07, 0.06, 0.05), light=(0.24, 0.21, 0.17), stone_scale=2.5):
    """Dusty grey-brown dirt with small stones, for ground under scenery.
    dark / light are the patch colours; stone_scale is Voronoi cells per
    unit (smaller = bigger stones, e.g. loose ore)."""
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
    bsdf.inputs["Roughness"].default_value = 0.96

    coord = n.new("ShaderNodeTexCoord")

    # Large patches of darker and lighter dirt
    mottle = n.new("ShaderNodeTexNoise")
    mottle.inputs["Scale"].default_value = 0.25
    mottle.inputs["Detail"].default_value = 6.0
    l.new(coord.outputs["Object"], mottle.inputs["Vector"])
    mottle_ramp = n.new("ShaderNodeValToRGB")
    mottle_ramp.color_ramp.elements[0].color = (dark[0], dark[1], dark[2], 1.0)
    mottle_ramp.color_ramp.elements[1].color = (light[0], light[1], light[2], 1.0)
    l.new(mottle.outputs["Fac"], mottle_ramp.inputs["Fac"])

    # Small stones: Voronoi cells, darker in the gaps between them
    stones = n.new("ShaderNodeTexVoronoi")
    stones.inputs["Scale"].default_value = stone_scale
    l.new(coord.outputs["Object"], stones.inputs["Vector"])
    stone_ramp = n.new("ShaderNodeValToRGB")
    stone_ramp.color_ramp.elements[0].position = 0.0
    stone_ramp.color_ramp.elements[0].color = (1.0, 1.0, 1.0, 1.0)
    stone_ramp.color_ramp.elements[1].position = 0.6
    stone_ramp.color_ramp.elements[1].color = (0.55, 0.55, 0.55, 1.0)
    l.new(stones.outputs["Distance"], stone_ramp.inputs["Fac"])

    color = n.new("ShaderNodeMix")
    color.data_type = 'RGBA'
    color.blend_type = 'MULTIPLY'
    _socket(color.inputs, "Factor_Float").default_value = 1.0
    l.new(mottle_ramp.outputs["Color"], _socket(color.inputs, "A_Color"))
    l.new(stone_ramp.outputs["Color"], _socket(color.inputs, "B_Color"))
    l.new(_socket(color.outputs, "Result_Color"), bsdf.inputs["Base Color"])

    # Stones raised, plus fine grit
    bump_stones = n.new("ShaderNodeBump")
    bump_stones.invert = True
    bump_stones.inputs["Strength"].default_value = 0.4
    l.new(stones.outputs["Distance"], bump_stones.inputs["Height"])
    grit = n.new("ShaderNodeTexNoise")
    grit.inputs["Scale"].default_value = 12.0
    grit.inputs["Detail"].default_value = 8.0
    l.new(coord.outputs["Object"], grit.inputs["Vector"])
    bump_grit = n.new("ShaderNodeBump")
    bump_grit.inputs["Strength"].default_value = 0.3
    l.new(grit.outputs["Fac"], bump_grit.inputs["Height"])
    l.new(bump_stones.outputs["Normal"], bump_grit.inputs["Normal"])
    l.new(bump_grit.outputs["Normal"], bsdf.inputs["Normal"])

    return mat


def _gravel_stone(name):
    """
    White stone chips (marble / quartz gravel). Neutral, so the editor's
    color attribute tints it. Each stone's brightness comes from its
    'stone_rand' attribute (0..1, set per stone by the script), so stones
    differ a little from each other; texture coordinates are the object's,
    so repeated copies of a tile look the same.
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
    _try_input(bsdf, "Subsurface Weight", 0.12)
    _try_input(bsdf, "Subsurface Scale", 0.04)

    coord = n.new("ShaderNodeTexCoord")
    attr = n.new("ShaderNodeAttribute")
    attr.attribute_type = 'GEOMETRY'
    attr.attribute_name = "stone_rand"

    # Per-stone brightness (linear albedo)
    per_stone = n.new("ShaderNodeMapRange")
    per_stone.inputs["To Min"].default_value = 0.55
    per_stone.inputs["To Max"].default_value = 0.86
    l.new(attr.outputs["Fac"], per_stone.inputs["Value"])

    # Cloudy veins inside a stone
    cloud = n.new("ShaderNodeTexNoise")
    cloud.inputs["Scale"].default_value = 5.0
    cloud.inputs["Detail"].default_value = 4.0
    l.new(coord.outputs["Object"], cloud.inputs["Vector"])
    cloud_map = n.new("ShaderNodeMapRange")
    cloud_map.inputs["To Min"].default_value = 0.88
    cloud_map.inputs["To Max"].default_value = 1.08
    l.new(cloud.outputs["Fac"], cloud_map.inputs["Value"])

    # A few dark specks
    speck = n.new("ShaderNodeTexNoise")
    speck.inputs["Scale"].default_value = 45.0
    speck.inputs["Detail"].default_value = 2.0
    l.new(coord.outputs["Object"], speck.inputs["Vector"])
    speck_map = n.new("ShaderNodeMapRange")
    speck_map.inputs["From Min"].default_value = 0.66
    speck_map.inputs["From Max"].default_value = 0.74
    speck_map.inputs["To Min"].default_value = 1.0
    speck_map.inputs["To Max"].default_value = 0.6
    l.new(speck.outputs["Fac"], speck_map.inputs["Value"])

    v1 = n.new("ShaderNodeMath")
    v1.operation = 'MULTIPLY'
    l.new(per_stone.outputs["Result"], v1.inputs[0])
    l.new(cloud_map.outputs["Result"], v1.inputs[1])
    v2 = n.new("ShaderNodeMath")
    v2.operation = 'MULTIPLY'
    v2.use_clamp = True
    l.new(v1.outputs["Value"], v2.inputs[0])
    l.new(speck_map.outputs["Result"], v2.inputs[1])
    l.new(v2.outputs["Value"], bsdf.inputs["Base Color"])

    # Some stones a bit shinier than others
    rough = n.new("ShaderNodeMapRange")
    rough.inputs["To Min"].default_value = 0.42
    rough.inputs["To Max"].default_value = 0.72
    l.new(attr.outputs["Fac"], rough.inputs["Value"])
    l.new(rough.outputs["Result"], bsdf.inputs["Roughness"])

    # Pitted, chipped surface
    pits = n.new("ShaderNodeTexNoise")
    pits.inputs["Scale"].default_value = 16.0
    pits.inputs["Detail"].default_value = 6.0
    l.new(coord.outputs["Object"], pits.inputs["Vector"])
    bump = n.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = 0.35
    bump.inputs["Distance"].default_value = 0.02
    l.new(pits.outputs["Fac"], bump.inputs["Height"])
    l.new(bump.outputs["Normal"], bsdf.inputs["Normal"])

    return mat


def _try_input(node, name, value):
    if name in node.inputs:
        node.inputs[name].default_value = value


def _socket(sockets, identifier):
    return next(s for s in sockets if s.identifier == identifier)


def _weathered_paint(name, paint, rust, streak_scale):
    """
    Painted steel with rust running down in vertical streaks.
    rust: 0..1, how much of the surface is rusty.
    streak_scale: smaller = longer vertical streaks.
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

    coord = n.new("ShaderNodeTexCoord")

    # Vertical streaks: squash the noise along Z.
    streak_map = n.new("ShaderNodeMapping")
    streak_map.inputs["Scale"].default_value = (1.0, 1.0, streak_scale)
    l.new(coord.outputs["Object"], streak_map.inputs["Vector"])

    streak_noise = n.new("ShaderNodeTexNoise")
    streak_noise.inputs["Scale"].default_value = 1.2
    streak_noise.inputs["Detail"].default_value = 8.0
    streak_noise.inputs["Roughness"].default_value = 0.6
    l.new(streak_map.outputs["Vector"], streak_noise.inputs["Vector"])

    rust_ramp = n.new("ShaderNodeValToRGB")
    lo = 0.68 - 0.25 * rust
    rust_ramp.color_ramp.elements[0].position = lo
    rust_ramp.color_ramp.elements[1].position = lo + 0.12
    l.new(streak_noise.outputs["Fac"], rust_ramp.inputs["Fac"])

    # Paint vs rust colour.
    mix = n.new("ShaderNodeMix")
    mix.data_type = 'RGBA'
    l.new(rust_ramp.outputs["Color"], _socket(mix.inputs, "Factor_Float"))
    _socket(mix.inputs, "A_Color").default_value = (paint[0], paint[1], paint[2], 1.0)
    _socket(mix.inputs, "B_Color").default_value = (0.22, 0.085, 0.03, 1.0)

    # Fine grime darkens the whole surface a little.
    grime = n.new("ShaderNodeTexNoise")
    grime.inputs["Scale"].default_value = 6.0
    grime.inputs["Detail"].default_value = 4.0
    l.new(coord.outputs["Object"], grime.inputs["Vector"])
    grime_ramp = n.new("ShaderNodeValToRGB")
    grime_ramp.color_ramp.elements[0].color = (0.75, 0.75, 0.75, 1.0)
    grime_ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)
    l.new(grime.outputs["Fac"], grime_ramp.inputs["Fac"])

    darken = n.new("ShaderNodeMix")
    darken.data_type = 'RGBA'
    darken.blend_type = 'MULTIPLY'
    _socket(darken.inputs, "Factor_Float").default_value = 1.0
    l.new(_socket(mix.outputs, "Result_Color"), _socket(darken.inputs, "A_Color"))
    l.new(grime_ramp.outputs["Color"], _socket(darken.inputs, "B_Color"))
    l.new(_socket(darken.outputs, "Result_Color"), bsdf.inputs["Base Color"])

    # Rust is rough and dull, paint a bit glossy.
    rough = n.new("ShaderNodeMapRange")
    rough.inputs["To Min"].default_value = 0.42
    rough.inputs["To Max"].default_value = 0.85
    l.new(rust_ramp.outputs["Color"], rough.inputs["Value"])
    l.new(rough.outputs["Result"], bsdf.inputs["Roughness"])
    bsdf.inputs["Metallic"].default_value = 0.15

    # Slight surface unevenness.
    bump = n.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = 0.15
    l.new(grime.outputs["Fac"], bump.inputs["Height"])
    l.new(bump.outputs["Normal"], bsdf.inputs["Normal"])

    return mat
