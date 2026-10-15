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
"""

import math
import os

import bpy
from mathutils import Vector

from . import shared

HDRI_CANDIDATES = [
    os.path.join("HDRI", "kloetzle_blei_16k.hdr"),        # leapfrog-game/models/HDRI (not in git)
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
    path = next((p for p in candidates if p and os.path.exists(p)), None)
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
    raise KeyError("mining theme has no material role '%s'" % role)


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
