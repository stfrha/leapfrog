"""
shield_power - the Baxters' shield power unit: the source that feeds their
shield at Halfway. It sits on the end of the station's boom, outside the shield,
a tough breakable object seen from the side: a glass cylinder between two metal
caps, the connector to the boom on the right cap (data/shield_power.xml; the boom
is dock_boom in docking_station.py). Inside the glass, blue sparks run as a
waveform of changing wavelengths.

One image in images/space (res_space.xml), at 20 px/unit:

  shield_power   26 x 8 units, the body's box. An animation strip, a few
                 frames in apparent chaotic order: Animate Period about 0.5 s.

The unit is rendered twice with the same camera, with the sparks' light off and
on (blue point lights along the axis, on the caps, the electrodes and the back
wall). Each frame mixes those two by how strong its sparks are, and lays the
sparks on top, drawn by shield_power_arcs.py. So a frame needs no render of its
own, and the sparks can be changed with --seed without waiting for Cycles.

The glass is a thin shell of reflections, nothing behind it but a half tube
for a back wall, so the electrodes show through it. The long lit cards in front
of it are what it reflects: the camera doesn't see them.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P shield_power.py -- --samples 16 --suffix _test
    blender -b --factory-startup -P shield_power.py
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import bpy                        # noqa: E402
import shield_power_arcs as arcs  # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to every texture name (e.g. _test: not registered)")
    p.add_argument("--frames", type=int, default=6, help="frames of sparks (default 6)")
    p.add_argument("--seed", type=int, default=7, help="another seed gives other sparks (default 7)")
    p.add_argument("--arc-power", type=float, default=40.0, help="each blue light on the axis, W (default 40)")
    p.add_argument("--card", type=float, default=6.0, help="strength of the cards the glass reflects (default 6)")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")
M = kit.material

WIDTH, HEIGHT = 26.0, 8.0
BOX = (-WIDTH / 2, -HEIGHT / 2, WIDTH / 2, HEIGHT / 2)
SEG = 96

R_CAP = 3.0                   # the caps
R_COLLAR = 3.7                # the collars that hold the glass
R_GLASS = 3.4
GLASS_X = 6.8                 # the glass runs -GLASS_X .. GLASS_X
TIP_X = 4.7                   # the electrode tips
STRAPS = (-4.3, 0.0, 4.3)     # bands round the glass
BLUE = (0.10, 0.22, 1.0)

unit = []                     # everything in the unit's image


def add(ob):
    unit.append(ob)
    return ob


def along(ob, x=0.0):
    """A lathed part (axis Z) laid along x."""
    ob.rotation_euler = (0.0, math.pi / 2, 0.0)
    ob.location = (x, 0.0, 0.0)
    return add(ob)


def turned(name, sections, material, caps=True):
    """Lathed sections, hard edges between them: [[(r, x), ...], ...]"""
    bm = bmesh.new()
    for profile in sections:
        sk.lathe(bm, profile, SEG, cap_top=caps, cap_bottom=caps)
    return along(sk.mesh_object(name, bm, material, smooth_quads=True))


def plain_material(name, color, roughness, metallic=0.0):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color + (1.0,)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = metallic
    return mat


def glass_material():
    """A thin shell: clear, reflecting more towards the edges."""
    mat = bpy.data.materials.new("power_glass")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    mix = nt.nodes.new("ShaderNodeMixShader")
    weight = nt.nodes.new("ShaderNodeLayerWeight")
    clear = nt.nodes.new("ShaderNodeBsdfTransparent")
    shine = nt.nodes.new("ShaderNodeBsdfPrincipled")
    weight.inputs["Blend"].default_value = 0.35
    shine.inputs["Base Color"].default_value = (0.8, 0.88, 1.0, 1.0)
    shine.inputs["Metallic"].default_value = 1.0
    shine.inputs["Roughness"].default_value = 0.07
    nt.links.new(weight.outputs["Fresnel"], mix.inputs[0])
    nt.links.new(clear.outputs[0], mix.inputs[1])
    nt.links.new(shine.outputs[0], mix.inputs[2])
    nt.links.new(mix.outputs[0], out.inputs["Surface"])
    return mat


def stripes_material():
    """Yellow and black diagonal stripes, in world coordinates."""
    mat = plain_material("power_hazard", (0.6, 0.42, 0.05), 0.55)
    nt = mat.node_tree
    bsdf = nt.nodes.get("Principled BSDF")
    geo = nt.nodes.new("ShaderNodeNewGeometry")
    sep = nt.nodes.new("ShaderNodeSeparateXYZ")
    add_xz = nt.nodes.new("ShaderNodeMath")
    add_xz.operation = 'ADD'
    scale = nt.nodes.new("ShaderNodeMath")
    scale.operation = 'MULTIPLY'
    scale.inputs[1].default_value = 1.4
    frac = nt.nodes.new("ShaderNodeMath")
    frac.operation = 'FRACT'
    over = nt.nodes.new("ShaderNodeMath")
    over.operation = 'GREATER_THAN'
    over.inputs[1].default_value = 0.5
    mix = nt.nodes.new("ShaderNodeMixRGB")
    mix.inputs["Color1"].default_value = (0.6, 0.42, 0.05, 1.0)
    mix.inputs["Color2"].default_value = (0.015, 0.015, 0.018, 1.0)
    nt.links.new(geo.outputs["Position"], sep.inputs[0])
    nt.links.new(sep.outputs[0], add_xz.inputs[0])
    nt.links.new(sep.outputs[2], add_xz.inputs[1])
    nt.links.new(add_xz.outputs[0], scale.inputs[0])
    nt.links.new(scale.outputs[0], frac.inputs[0])
    nt.links.new(frac.outputs[0], over.inputs[0])
    nt.links.new(over.outputs[0], mix.inputs["Fac"])
    nt.links.new(mix.outputs["Color"], bsdf.inputs["Base Color"])
    return mat


def emission_material(name, strength):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (0.85, 0.92, 1.0, 1.0)
    em.inputs["Strength"].default_value = strength
    nt.links.new(em.outputs["Emission"], out.inputs["Surface"])
    return mat


# ---------------------------------------------------------------------------
# The caps. Mirrored; only the right one meets the boom.
# ---------------------------------------------------------------------------
for side in (-1.0, 1.0):
    def p(points):
        """A profile from the middle outwards, whichever side it is on."""
        pts = [(r, side * x) for r, x in points]
        return pts if side > 0 else pts[::-1]

    nm = "right" if side > 0 else "left"
    turned(nm + "_cap", [p([(R_CAP, 8.0), (R_CAP, 10.9)]),
                         p([(R_CAP, 10.9), (2.0, 11.5)])], M("frame_main"))
    turned(nm + "_step", [p([(R_COLLAR - 0.2, 7.6), (R_CAP, 8.0)])], M("frame_brace"))
    turned(nm + "_collar", [p([(R_COLLAR, 6.8), (R_COLLAR, 7.6)])], M("pad_metal"))
    turned(nm + "_lip", [p([(R_GLASS + 0.1, 6.6), (R_GLASS + 0.1, 6.8)])], M("frame_brace"))

    # Ribs and the hazard band on the collar
    for k in range(2):
        turned("%s_rib_%d" % (nm, k), [p([(R_COLLAR + 0.04, 6.9 + 0.2 * k), (R_COLLAR + 0.04, 7.0 + 0.2 * k)])],
               M("frame_brace"))

    # Panels, on the front of the cap and tilted round it
    xc = side * 9.45
    for tilt in (-48.0, 0.0, 48.0):
        a = math.radians(tilt)
        where = (xc, -R_CAP * math.cos(a), R_CAP * math.sin(a))
        add(sk.box_object("%s_panel_%g" % (nm, tilt), where, (2.5, 0.12, 1.5), M("pad_metal"),
                          rotation=(-a, 0.0, 0.0), bevel_width=0.04))
        for dx in (-1.05, 1.05):
            for dz in (-0.55, 0.55):
                bx = xc + dx
                bz = dz * math.cos(a)
                by = -R_CAP * math.cos(a) - 0.07
                add(sk.box_object("%s_bolt_%g_%g_%g" % (nm, tilt, dx, dz),
                                  (bx, by, R_CAP * math.sin(a) + bz), (0.14, 0.05, 0.14), M("frame_main")))

    # Vents in the middle panel
    for k in range(5):
        add(sk.box_object("%s_vent_%d" % (nm, k), (xc - 0.9 + 0.45 * k, -R_CAP - 0.08, 0.0), (0.17, 0.05, 0.95),
                          M("glass_dark")))

    # A status lamp
    lamp_z = R_CAP * math.sin(math.radians(32.0))
    lamp_y = -R_CAP * math.cos(math.radians(32.0))
    add(sk.box_object(nm + "_lamp", (xc - 0.7, lamp_y - 0.08, lamp_z + 0.1), (0.3, 0.1, 0.3), M("neon_amber"),
                      bevel_width=0.04))

    # The electrode
    turned(nm + "_electrode", [p([(0.08, TIP_X), (0.45, TIP_X + 0.8), (1.3, GLASS_X + 0.1)])], M("frame_main"),
           caps=False)

# The ends: a plain plug on the left, the connector to the boom on the right
turned("left_plug", [[(2.2, -12.4), (2.2, -11.4)]], M("frame_brace"))
turned("connector", [[(2.0, 11.4), (2.0, 12.0)], [(1.0, 12.0), (1.0, 12.5)]], M("frame_brace"))
turned("cable_gland", [[(0.5, 12.5), (0.5, 13.2)]], M("rubber"))

# The hazard bands
hazard = stripes_material()
turned("left_band", [[(R_COLLAR + 0.02, -7.6), (R_COLLAR + 0.02, -7.25)]], hazard)
turned("right_band", [[(R_COLLAR + 0.02, 7.25), (R_COLLAR + 0.02, 7.6)]], hazard)

# ---------------------------------------------------------------------------
# The glass: a back wall, the straps, and a shell of reflections
# ---------------------------------------------------------------------------
wall = bmesh.new()
sk.lathe(wall, [(R_GLASS - 0.1, -GLASS_X), (R_GLASS - 0.1, GLASS_X)], SEG)
bmesh.ops.delete(wall, geom=[v for v in wall.verts if v.co.y < -0.001], context='VERTS')
along(sk.mesh_object("back_wall", wall, plain_material("power_wall", (0.012, 0.022, 0.07), 0.35),
                     smooth_quads=True))

for x in STRAPS:
    turned("strap_%g" % x, [[(R_GLASS + 0.03, x - 0.08), (R_GLASS + 0.03, x + 0.08)]], M("frame_brace"))

shell = bmesh.new()
sk.lathe(shell, [(R_GLASS, -GLASS_X), (R_GLASS, GLASS_X)], SEG)
shell_ob = along(sk.mesh_object("glass", shell, glass_material(), smooth_quads=True))
shell_ob.visible_shadow = False

# What the glass reflects: not seen by the camera, no shadows
for name, z, height, strength in (("card_top", 9.0, 1.6, args.card), ("card_bottom", -9.0, 0.8, args.card * 0.35)):
    card = add(sk.box_object(name, (0.0, -9.0, z), (16.0, 0.05, height), emission_material(name, strength)))
    card.rotation_euler = (math.atan2(-z, 9.0), 0.0, 0.0)     # facing the glass
    card.visible_camera = False
    card.visible_shadow = False

# The sparks' light
blue = [add(sk.point_light("arc_light_%d" % k, (x, 0.0, 0.0), BLUE, args.arc_power, 0.4))
        for k, x in enumerate((-4.0, -2.0, 0.0, 2.0, 4.0))]


def set_arc_light(on):
    for light in blue:
        light.hide_render = not on


# ---------------------------------------------------------------------------
# Render
# ---------------------------------------------------------------------------
def load_top_down(path):
    pixels, w, h = sk._load_pixels(path)
    return pixels[::-1].copy(), w, h


def save_top_down(path, image):
    import numpy as np
    h, w = image.shape[:2]
    img = bpy.data.images.new("frame", w, h, alpha=True)
    img.pixels.foreach_set(image[::-1].astype(np.float32).ravel())
    img.filepath_raw = path
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)


def render_unit():
    name = "shield_power" + args.suffix
    set_arc_light(False)
    base_path = kit.render_ortho(name + "_base", BOX, frame_only=True)
    set_arc_light(True)
    lit_path = kit.render_variant(name + "_lit", keep_unchanged=False)
    if not base_path or not lit_path:
        return

    base, w, h = load_top_down(base_path)
    lit, _, _ = load_top_down(lit_path)
    ppu = w / WIDTH
    cx, cy = w / 2.0, h / 2.0
    x0, x1 = cx - TIP_X * ppu, cx + TIP_X * ppu
    gx0, gx1 = cx - GLASS_X * ppu, cx + GLASS_X * ppu
    r = R_GLASS * ppu
    mask, fade = arcs.glass_mask(w, h, gx0, gx1, cy, r, [(cx + x * ppu, 0.1 * ppu) for x in STRAPS])

    rng = random.Random(args.seed)
    paths = []
    for k in range(args.frames):
        glow, line = arcs.frame_layers(w, h, rng.randint(0, 1 << 30), x0, x1, cy, r * 0.8, ppu)
        level = arcs.light_level(glow)
        path = os.path.join(os.path.dirname(base_path), "%s_f%d.png" % (name, k))
        save_top_down(path, arcs.compose(base, lit, glow, line, level, mask, fade))
        paths.append(path)

    kit.make_strip(paths, name)
    print("shield_power: Animate Period about %.2f s" % (args.frames * 0.085))


render_unit()
