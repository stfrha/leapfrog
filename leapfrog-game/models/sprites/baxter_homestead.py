"""
baxter_homestead - the Baxters' homestead ship, a big background sprite for
the space scenes (a parallax layer, no body). Seen from the side, nose to
the right, a little from above and in front.

A family ship for 25+ people, built up over the years from whatever they
could get, so the parts don't match. From the stern forwards:
  engines     one big main bell and two smaller ones on a block, radiator
              fins above and below, exhaust plumes when they burn
  tanks       two fuel tanks on top of the spine
  hangars     three hangars for one-man fighters hanging under the spine,
              each painted differently: the first open (lit, a fighter
              inside), the second shut with BXTR sprayed over the doors,
              the third half open. Cargo containers lashed on top.
  habitat     a long drum with rows of cabin windows (living quarters),
              a glass dome on top with a garden and the mess/pub
              underneath it (recreation), a comms dish, BAXTER CORP stencilled
              on the side
  main deck   the bow: a hard wedge with a ram, raked armour scales, a low
              armoured bridge with slit windows, a twin gun and a shark
              mouth; an old salvage arm folded under the keel and the
              faded BAXTER SALVAGE & CONSTRUCTION name show what the family
              once was (--old-bow: the first bow)

One image with two frames (c_animateSprite picks one):
  frame 0  engines burning (arriving, leaving)
  frame 1  engines cold (parked)
Both have exactly the same frame, so they can be swapped.

1 Blender unit = 1 game unit: the ship is ~300 long (the Leapfrog is 24
wide). It's drawn far smaller than that in the game, on a far parallax layer
(0.9 in deep_space_scene.xml), so it renders at a low px/unit: 6 by default
(the layer needs about 20 x (1 - 0.9) = 2, the rest is margin).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P baxter_homestead.py
    blender -b --factory-startup -P baxter_homestead.py -- --samples 16 --suffix _test
    blender -b --factory-startup -P baxter_homestead.py -- --still      (engines burning only)
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bpy                        # noqa: E402
import bmesh                      # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--yaw", type=float, default=-14.0,
                   help="ship turned about its vertical axis, degrees (- = bow towards the camera)")
    p.add_argument("--pitch", type=float, default=9.0, help="ship tipped towards the camera, degrees (shows the top)")
    p.add_argument("--sun", type=float, default=2.6, help="sunlight strength (default 2.6)")
    p.add_argument("--plume", type=float, default=1.0, help="exhaust plume length x this (0 = none)")
    p.add_argument("--seed", type=int, default=7, help="which cabin windows are dark")
    p.add_argument("--dark", type=float, default=0.3, help="share of dark cabin windows")
    p.add_argument("--bow-scale", type=float, default=0.9, help="size of the bow (default 0.9)")
    p.add_argument("--old-bow", action="store_true", help="the first, friendlier bow")


args = sk.parse_args(options)
if not args.ppu:
    args.ppu = 6.0
if not args.day:
    args.night = True             # dim sky light; the sun is set below
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")
rnd = random.Random(args.seed)

parts = []           # everything that belongs to the ship (parented to the root)


def add(ob):
    parts.append(ob)
    return ob


def mat(role):
    return kit.material(role)


# ---------------------------------------------------------------------------
# Own materials (not in the theme)
# ---------------------------------------------------------------------------

def emission(name, color, strength):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (color[0], color[1], color[2], 1.0)
    em.inputs["Strength"].default_value = strength
    nt.links.new(em.outputs["Emission"], out.inputs["Surface"])
    return m


def see_through_glow(name, color, strength, opacity):
    """Emission mixed with transparency: exhaust plumes."""
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    tr = nt.nodes.new("ShaderNodeBsdfTransparent")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (color[0], color[1], color[2], 1.0)
    em.inputs["Strength"].default_value = strength
    mix = nt.nodes.new("ShaderNodeMixShader")
    mix.inputs[0].default_value = opacity
    nt.links.new(tr.outputs[0], mix.inputs[1])
    nt.links.new(em.outputs[0], mix.inputs[2])
    nt.links.new(mix.outputs[0], out.inputs["Surface"])
    return m


def paint(name, color, rough=0.7, metallic=0.2):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (color[0], color[1], color[2], 1.0)
    bsdf.inputs["Roughness"].default_value = rough
    bsdf.inputs["Metallic"].default_value = metallic
    return m


def glass(name):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (0.7, 0.85, 0.8, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.05
    for key in ("Transmission Weight", "Transmission"):       # 4.x / 3.x name
        if key in bsdf.inputs:
            bsdf.inputs[key].default_value = 0.85
            break
    return m


ENGINE_GLOW = emission("engine_glow", (0.55, 0.75, 1.0), 40.0)
ENGINE_COLD = paint("engine_cold", (0.02, 0.02, 0.025), rough=0.6, metallic=0.5)
PLUME_CORE = see_through_glow("plume_core", (0.65, 0.82, 1.0), 12.0, 0.6)
PLUME_OUTER = see_through_glow("plume_outer", (0.35, 0.55, 1.0), 5.0, 0.25)
NAV_RED = emission("nav_red", (1.0, 0.06, 0.04), 30.0)
NAV_GREEN = emission("nav_green", (0.1, 1.0, 0.25), 30.0)
NAV_WHITE = emission("nav_white", (1.0, 1.0, 1.0), 30.0)
GARDEN = emission("garden", (0.25, 0.75, 0.2), 1.6)
GRAFFITI = paint("graffiti_pink", (0.9, 0.12, 0.45), rough=0.8, metallic=0.0)
STENCIL = paint("stencil_white", (0.75, 0.73, 0.68), rough=0.8, metallic=0.0)
HANGAR_GREEN = paint("hangar_green", (0.10, 0.17, 0.09), rough=0.75, metallic=0.3)
DOME_GLASS = glass("dome_glass")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def cylinder_x(name, x0, x1, r, center_yz, material, segments=48, caps=True, uv_tile=None):
    """A cylinder along X from x0 to x1."""
    bm = bmesh.new()
    uv = bm.loops.layers.uv.new("UVMap") if uv_tile else None
    sk.lathe(bm, [(r, 0.0), (r, x1 - x0)], segments, uv_tile or 1.0, uv, cap_top=caps, cap_bottom=caps)
    ob = add(sk.mesh_object(name, bm, material, smooth_quads=True))
    ob.location = (x0, center_yz[0], center_yz[1])
    ob.rotation_euler = (0.0, math.pi / 2, 0.0)     # lathe axis (Z) along +X
    return ob


def tank_x(name, x0, x1, r, center_yz, material):
    """A tank along X with rounded ends."""
    L = x1 - x0
    prof = []
    for i in range(7):
        a = math.pi / 2 * (1 - i / 6)
        prof.append((r * math.cos(a) + 0.001, r - r * math.sin(a)))
    for i in range(7):
        a = math.pi / 2 * i / 6
        prof.append((r * math.cos(a) + 0.001, L - r + r * math.sin(a)))
    bm = bmesh.new()
    sk.lathe(bm, prof, 32)
    ob = add(sk.mesh_object(name, bm, material, smooth_quads=True))
    ob.location = (x0, center_yz[0], center_yz[1])
    ob.rotation_euler = (0.0, math.pi / 2, 0.0)
    return ob


def bell(name, x_throat, z, r0, r1, length):
    """An engine bell opening aft (-X), with a glowing disc in its mouth.
    Returns the glow disc (its material is swapped for the cold frame)."""
    bm = bmesh.new()
    prof = [(r0 * 1.1, 0.0)]
    for i in range(1, 9):
        t = i / 8
        prof.append((r0 + (r1 - r0) * (t ** 1.6), length * t))
    sk.lathe(bm, prof, 40)
    b = add(sk.mesh_object(name, bm, mat("nozzle"), smooth_quads=True))
    b.location = (x_throat, 0.0, z)
    b.rotation_euler = (0.0, -math.pi / 2, 0.0)     # lathe axis (Z) along -X
    # The glow, a little inside the mouth
    dbm = bmesh.new()
    sk.lathe(dbm, [(r1 * 0.93, length * 0.9)], 40, cap_top=True)
    d = add(sk.mesh_object(name + "_glow", dbm, ENGINE_GLOW))
    d.location = b.location
    d.rotation_euler = b.rotation_euler
    return d


def plume(name, x_mouth, z, r, length):
    """Exhaust behind a bell: a bright core in a fainter, longer sheath."""
    obs = []
    for (rr, ll, m, tag) in ((r * 0.8, length * 0.55, PLUME_CORE, "core"), (r * 1.0, length, PLUME_OUTER, "outer")):
        bm = bmesh.new()
        sk.lathe(bm, [(rr, 0.0), (rr * 0.75, ll * 0.4), (rr * 0.3, ll * 0.85), (0.05, ll)], 32, cap_bottom=True)
        ob = add(sk.mesh_object("%s_%s" % (name, tag), bm, m, smooth_quads=True))
        ob.location = (x_mouth, 0.0, z)
        ob.rotation_euler = (0.0, -math.pi / 2, 0.0)
        obs.append(ob)
    return obs


def windows_on_drum(prefix, x0, x1, step, rows_z, R, size, lit_role, dark_share, skip=None):
    """Rows of windows on the front (-Y) of a drum along X with radius R."""
    out = []
    lit = mat(lit_role)
    dark = mat("glass_dark")
    x = x0
    i = 0
    while x <= x1:
        for z in rows_z:
            if skip and skip(x, z):
                continue
            phi = math.asin(z / R)
            y = -R * math.cos(phi) - 0.05
            m = dark if rnd.random() < dark_share else lit
            out.append(add(sk.box_object("%s_%d_%g" % (prefix, i, z), (x, y, z), (size[0], 0.3, size[1]), m,
                                         rotation=(-phi, 0.0, 0.0))))
        x += step
        i += 1
    return out


# ---------------------------------------------------------------------------
# Spine: the keel everything hangs on, from the engine block to the bow
# ---------------------------------------------------------------------------
add(sk.box_object("spine", (-30.0, 0.0, 0.0), (180.0, 7.0, 7.0), mat("frame_main"), bevel_width=0.3))
spine = bmesh.new()
for sy in (-1, 1):
    for sz in (-1, 1):
        sk.strut(spine, (-110.0, sy * 4.2, sz * 4.2), (52.0, sy * 4.2, sz * 4.2), 0.5, 8)
add(sk.mesh_object("spine_pipes", spine, mat("pad_metal"), smooth_quads=True))

# ---------------------------------------------------------------------------
# Engines: a block at the stern with one big bell and two smaller ones,
# radiator fins above and below
# ---------------------------------------------------------------------------
EX = -128.0          # the bells' throats (aft face of the block)
add(sk.box_object("engine_block", (-118.0, 0.0, 0.0), (20.0, 26.0, 44.0), mat("frame_main"), bevel_width=0.6))
add(sk.box_object("engine_collar", (-107.0, 0.0, 0.0), (3.0, 22.0, 36.0), mat("hazard_paint"), bevel_width=0.3))
for z in (-15.0, -5.0, 5.0, 15.0):
    add(sk.box_object("engine_rib_%g" % z, (-118.0, -13.2, z), (19.0, 0.6, 1.2), mat("frame_brace")))

glows = []
glows.append(bell("bell_main", EX, 0.0, 6.0, 10.5, 17.0))
glows.append(bell("bell_upper", EX, 16.0, 3.5, 6.0, 10.0))
glows.append(bell("bell_lower", EX, -16.0, 3.5, 6.0, 10.0))

plumes = []
if args.plume > 0:
    plumes += plume("plume_main", EX - 17.0, 0.0, 10.0, 34.0 * args.plume)
    plumes += plume("plume_upper", EX - 10.0, 16.0, 5.6, 20.0 * args.plume)
    plumes += plume("plume_lower", EX - 10.0, -16.0, 5.6, 20.0 * args.plume)

engine_lights = []
for (z, e) in ((0.0, 4000.0), (16.0, 1500.0), (-16.0, 1500.0)):
    engine_lights.append(add(sk.point_light("engine_light_%g" % z, (EX - 22.0, -6.0, z),
                                            color=(0.55, 0.75, 1.0), energy=e, radius=3.0)))

# Radiators: big flat fins in the side plane, on struts
for sz in (-1, 1):
    zc = sz * 31.0
    add(sk.box_object("radiator_%d" % sz, (-114.0, 0.0, zc), (26.0, 0.8, 16.0), mat("rusty_metal")))
    for k in range(6):
        add(sk.box_object("radiator_rib_%d_%d" % (sz, k), (-125.0 + k * 4.4, -0.5, zc), (0.5, 0.4, 15.0),
                          mat("frame_main")))
    rad = bmesh.new()
    sk.strut(rad, (-120.0, 0.0, sz * 22.0), (-117.0, 0.0, sz * 23.5), 0.6, 8)
    sk.strut(rad, (-110.0, 0.0, sz * 22.0), (-111.0, 0.0, sz * 23.5), 0.6, 8)
    add(sk.mesh_object("radiator_struts_%d" % sz, rad, mat("frame_main"), smooth_quads=True))

# ---------------------------------------------------------------------------
# Fuel tanks on top of the spine, strapped
# ---------------------------------------------------------------------------
for sy in (-1, 1):
    tank_x("tank_%d" % sy, -104.0, -74.0, 5.5, (sy * 6.0, 9.5), mat("pad_metal"))
    for x in (-98.0, -89.0, -80.0):
        cylinder_x("tank_strap_%d_%g" % (sy, x), x, x + 1.2, 5.75, (sy * 6.0, 9.5), mat("hazard_paint"), 32)

# ---------------------------------------------------------------------------
# Hangars: three boxes under the spine for the one-man fighters. Doors on
# the side facing the camera. Each from a different yard, so each painted
# differently.
# ---------------------------------------------------------------------------
HW, HH, HD = 24.0, 16.0, 24.0          # length, height, depth
HZ = -3.5 - HH / 2 - 0.5              # hangs just under the spine
hangars = [(-86.0, mat("container_red"), "open"),
           (-60.0, HANGAR_GREEN, "shut"),
           (-34.0, mat("container_blue"), "half")]
front = -HD / 2


def fighter(name, x, z, y):
    """A small one-man fighter, side on, nose right: a wedge with a canopy."""
    pts = [(-6.0, -1.2), (-6.5, 1.6), (-3.5, 1.8), (1.0, 1.0), (6.5, -0.2), (5.5, -1.3)]
    f = add(sk.prism(name, [(x + px, z + pz) for px, pz in pts], y, 3.0, mat("frame_brace")))
    add(sk.box_object(name + "_canopy", (x + 1.5, y - 0.05, z + 0.8), (2.4, 0.2, 0.9), mat("window_lit_blue")))
    add(sk.box_object(name + "_engine", (x - 6.6, y + 0.2, z + 0.1), (0.4, 0.4, 2.0), mat("neon_amber")))
    return f


for i, (hx, hmat, state) in enumerate(hangars):
    add(sk.box_object("hangar_%d" % i, (hx, 0.0, HZ), (HW, HD, HH), hmat, bevel_width=0.4))
    # Frame round the door
    add(sk.box_object("hangar_%d_lintel" % i, (hx, front - 0.3, HZ + HH / 2 - 1.0), (HW - 1.0, 0.6, 1.4),
                      mat("frame_main")))
    add(sk.box_object("hangar_%d_sill" % i, (hx, front - 0.3, HZ - HH / 2 + 0.9), (HW - 1.0, 0.6, 1.2),
                      mat("hazard_paint")))
    for sx in (-1, 1):
        add(sk.box_object("hangar_%d_jamb_%d" % (i, sx), (hx + sx * (HW / 2 - 1.5), front - 0.3, HZ),
                          (1.2, 0.6, HH - 1.0), mat("frame_main")))
    door_w = HW - 4.0
    door_h = HH - 4.2
    door_z = HZ + 0.1
    if state in ("open", "half"):
        # The bay behind the opening: dark, with a lit ceiling strip
        open_h = door_h if state == "open" else door_h * 0.5
        bay_z = door_z - door_h / 2 + open_h / 2
        # (on the hangar's face: the box is solid, so the bay is a dark plate)
        add(sk.box_object("hangar_%d_bay" % i, (hx, front - 0.15, bay_z), (door_w, 0.2, open_h), mat("void")))
        add(sk.box_object("hangar_%d_bay_light" % i, (hx, front - 0.35, bay_z + open_h / 2 - 0.5),
                          (door_w - 1.0, 0.3, 0.5), mat("window_lit_warm")))
        add(sk.point_light("hangar_%d_light" % i, (hx, front - 1.5, bay_z + open_h / 2 - 1.5),
                           color=(1.0, 0.8, 0.55), energy=400.0, radius=1.0))
        if state == "open":
            fighter("fighter_%d" % i, hx - 1.0, door_z - door_h / 2 + 2.0, front - 0.9)
        else:
            # The door, half way up
            add(sk.box_object("hangar_%d_door" % i, (hx, front - 0.2, door_z + door_h / 4),
                              (door_w, 0.4, door_h / 2), mat("pad_metal"), bevel_width=0.1))
    else:
        # Two door leaves, shut, with BXTR sprayed across them
        for sx in (-1, 1):
            add(sk.box_object("hangar_%d_leaf_%d" % (i, sx), (hx + sx * door_w / 4, front - 0.2, door_z),
                              (door_w / 2 - 0.2, 0.4, door_h), mat("pad_metal"), bevel_width=0.1))
        add(sk.text_object("hangar_%d_graffiti" % i, "BXTR", 7.5, [GRAFFITI],
                           location=(hx + 0.5, front - 0.45, door_z - 0.5), rotation=(math.pi / 2, 0.12, 0.0),
                           extrude=0.02))
    # Landing lights either side of the door
    for sx in (-1, 1):
        add(sk.box_object("hangar_%d_lamp_%d" % (i, sx), (hx + sx * (HW / 2 - 1.1), front - 0.35, HZ - HH / 2 + 2.2),
                          (0.7, 0.3, 0.7), mat("neon_amber")))

# Containers lashed on top of the spine above the hangars
for (x, z, w, role) in ((-64.0, 5.5, 12.0, "container_red"), (-51.0, 5.5, 12.0, "container_blue"),
                        (-57.5, 10.8, 12.0, "machine_yellow"), (-38.0, 5.5, 12.0, "rusty_metal"),
                        (-25.0, 5.5, 9.0, "container_red")):
    add(sk.box_object("container_%g_%g" % (x, z), (x, 0.0, z), (w, 5.0, 5.0), mat(role), bevel_width=0.15))

# ---------------------------------------------------------------------------
# Habitat: the drum with the cabins, the dome with the garden and the mess
# ---------------------------------------------------------------------------
R = 18.0
DX0, DX1 = -16.0, 52.0
cylinder_x("habitat", DX0, DX1, R, (0.0, 0.0), mat("hull_white"), 72, uv_tile=3.9)
for x in (DX0, DX1 - 1.5):                                      # end flanges
    cylinder_x("habitat_flange_%g" % x, x, x + 1.5, R + 0.8, (0.0, 0.0), mat("frame_main"), 72)
for x in (5.0, 27.0):                                           # bands between sections
    cylinder_x("habitat_band_%g" % x, x, x + 1.0, R + 0.35, (0.0, 0.0), mat("frame_brace"), 72)
# A patched-on section of other plates
add(sk.box_object("habitat_patch", (40.0, -R * math.cos(math.asin(-9.0 / R)) - 0.1, -9.0), (8.0, 0.25, 6.0),
                  mat("rusty_metal"), rotation=(-math.asin(-9.0 / R), 0.0, 0.0)))


def stencil_area(x, z):
    return 30.0 <= x <= 50.0 and z < -4.0 or 36.0 <= x <= 45.0 and -12.0 <= z <= -6.0


windows_on_drum("cabin", DX0 + 3.0, DX1 - 3.0, 3.2, (-11.0, -5.0, 1.0, 7.0), R, (1.3, 1.9),
                "window_lit_warm", args.dark, skip=lambda x, z: (4.0 <= x <= 7.0 or 26.0 <= x <= 29.0
                                                                 or stencil_area(x, z)))
# BAXTER CORP stencilled under the cabin rows
phi = math.asin(-12.0 / R)
add(sk.text_object("stencil", "BAXTER CORP", 2.6, [STENCIL],
                   location=(16.0, -R * math.cos(phi) - 0.15, -12.5), rotation=(math.pi / 2 - phi, 0.0, 0.0),
                   extrude=0.02))

# The dome: glass over a garden, the mess under it with a lit window band
DOME_X, DOME_R = 14.0, 11.0
add(sk.box_object("dome_deck", (DOME_X, 0.0, R - 0.5), (DOME_R * 2 + 4.0, 16.0, 3.0), mat("frame_main"),
                  bevel_width=0.3))
add(sk.box_object("mess_windows", (DOME_X, -8.05, R - 0.4), (DOME_R * 2 + 1.0, 0.2, 1.2), mat("window_lit_warm")))
dome = bmesh.new()
prof = [(DOME_R * math.cos(math.pi / 2 * i / 10) + 0.001, DOME_R * 0.75 * math.sin(math.pi / 2 * i / 10))
        for i in range(11)]
sk.lathe(dome, prof, 48)
d = add(sk.mesh_object("dome", dome, DOME_GLASS, smooth_quads=True))
d.location = (DOME_X, 0.0, R + 1.0)
ribs = bmesh.new()
for k in range(-3, 4):
    a = k * math.pi / 8
    pts = [(DOME_X + DOME_R * math.cos(math.pi / 2 * i / 8) * math.sin(a),
            -DOME_R * math.cos(math.pi / 2 * i / 8) * math.cos(a),
            R + 1.0 + DOME_R * 0.75 * math.sin(math.pi / 2 * i / 8)) for i in range(9)]
    for p0, p1 in zip(pts, pts[1:]):
        sk.strut(ribs, p0, p1, 0.15, 6)
add(sk.mesh_object("dome_ribs", ribs, mat("frame_brace"), smooth_quads=True))
garden = bmesh.new()
for (gx, gy, gz, gr) in ((-6.0, -2.0, 3.0, 2.6), (-2.0, 1.5, 4.0, 3.4), (3.0, -1.0, 3.2, 2.8),
                         (6.5, 2.0, 2.6, 2.2), (0.5, -4.0, 2.0, 1.8)):
    bmesh.ops.create_uvsphere(garden, u_segments=16, v_segments=10, radius=gr,
                              matrix=Matrix.Translation((DOME_X + gx, gy, R + 1.0 + gz)))
add(sk.mesh_object("garden", garden, GARDEN, smooth_quads=True))
add(sk.point_light("dome_light", (DOME_X, -2.0, R + 7.0), color=(1.0, 0.9, 0.7), energy=600.0, radius=2.0))

# Comms dish and masts on the aft end of the drum
dish = bmesh.new()
sk.lathe(dish, [(0.3, 0.0)] + [(5.0 * i / 6, 0.12 * (5.0 * i / 6) ** 2) for i in range(1, 7)], 32)
dob = add(sk.mesh_object("dish", dish, mat("hull_white"), smooth_quads=True))
dob.location = (-6.0, 0.0, R + 6.0)
dob.rotation_euler = (0.0, math.radians(-35.0), 0.0)
add(sk.box_object("dish_mast", (-6.0, 0.0, R + 2.5), (0.8, 0.8, 6.0), mat("frame_main")))
add(sk.box_object("mast_aft", (-12.0, 0.0, R + 6.0), (0.35, 0.35, 12.0), mat("frame_main")))
add(sk.box_object("mast_aft_lamp", (-12.0, 0.0, R + 12.3), (0.8, 0.8, 0.8), NAV_WHITE))

# ---------------------------------------------------------------------------
# Main deck: the bow (2026-11-07, from the chosen mockup "A + stowed arm").
# A hard wedge with a ram under the nose, raked armour scales, a low
# armoured bridge with slit windows, a dorsal blade, a twin gun on the
# foredeck and a shark mouth. Underneath, the family's past: an old
# salvage arm folded under the keel in faded construction yellow, and
# BAXTER SALVAGE & CONSTRUCTION stencilled on the side, half covered by
# a newer plate. Built at full size and scaled by --bow-scale (0.9) about
# where it meets the habitat. --old-bow builds the first bow instead.
# ---------------------------------------------------------------------------
add(sk.box_object("neck", (54.0, 0.0, 0.0), (6.0, 14.0, 16.0), mat("frame_main"), bevel_width=0.3))
BY = -12.0
bow_parts = []


def badd(ob):
    bow_parts.append(ob)
    return add(ob)


_paint = getattr(kit.theme, "_weathered_paint", None)


def weathered(name, paint, rust, streak, fallback):
    return _paint(name, paint=paint, rust=rust, streak_scale=streak) if _paint else mat(fallback)


ARMOUR_A = weathered("armour_light", (0.25, 0.25, 0.26), 0.25, 0.12, "frame_brace")
ARMOUR_B = weathered("armour_dark", (0.19, 0.19, 0.20), 0.3, 0.12, "pad_metal")
MOUTH = weathered("mouth_red", (0.24, 0.02, 0.015), 0.25, 0.3, "container_red")
TEETH = weathered("teeth_white", (0.62, 0.60, 0.54), 0.3, 0.3, "hull_white")
OLD_STENCIL = weathered("old_stencil", (0.30, 0.26, 0.11), 0.6, 0.3, "machine_yellow")
SLIT = emission("slit_amber", (1.0, 0.55, 0.12), 12.0)
BRIDGE_RED = emission("bridge_red", (0.8, 0.12, 0.06), 6.0)


def flat(name, pts, y, material, depth=0.05):
    """Paint or a thin plate on the near side (pts as x, z)."""
    return badd(sk.prism(name, pts, y, depth, material))


if not args.old_bow:
    # Ram under the nose
    badd(sk.prism("bow_ram", [(143.5, -5.5), (179.6, -6.0), (181.0, -7.0), (173.0, -8.5), (143.5, -11.6)],
                  -4.0, 8.0, mat("frame_main")))
    # The wedge and its keel
    badd(sk.prism("bow", [(56.0, 18.2), (82.9, 20.5), (103.7, 17.1), (143.5, 7.0), (166.9, -2.4), (162.9, -5.8),
                          (141.8, -11.6), (103.7, -16.6), (56.0, -16.0)], BY, 24.0, mat("frame_brace")))
    badd(sk.prism("bow_keel", [(56.0, -12.2), (103.7, -13.3), (141.8, -8.9), (162.9, -5.8), (165.0, -6.7),
                               (143.2, -13.9), (103.7, -18.3), (56.0, -17.7)], BY - 0.4, 24.8, mat("frame_main")))
    # Raked armour scales, overlapping towards the nose, with rivet rows
    scales = [[(60.3, 16.5), (86.3, 15.5), (80.4, 1.6), (54.4, 1.6)],
              [(82.9, 17.1), (108.9, 16.1), (103.0, 0.3), (77.0, 0.3)],
              [(105.4, 13.8), (129.7, 7.6), (123.8, -1.4), (99.5, -1.4)],
              [(126.2, 8.7), (147.0, 3.4), (141.1, -2.4), (120.3, -2.4)]]
    for i, pts in enumerate(scales):
        pts = [(max(x, 56.5), z) for x, z in pts]
        y = BY - 0.6 - i * 0.3
        flat("bow_scale_%d" % i, pts, y, ARMOUR_B if i % 2 else ARMOUR_A, depth=0.6)
        for (a, b) in ((pts[0], pts[1]), (pts[3], pts[2])):
            for k in range(9):
                t = (k + 0.5) / 9
                badd(sk.box_object("bow_scale_%d_rivet_%g_%d" % (i, a[1], k),
                                   (a[0] + (b[0] - a[0]) * t, y - 0.08, a[1] + (b[1] - a[1]) * t + (-0.5 if a is pts[0] else 0.5)),
                                   (0.3, 0.16, 0.3), mat("pad_metal")))
    # Rusty patch, seams, vents
    flat("bow_rust_patch", [(67.3, -2.4), (82.9, -1.8), (81.5, -8.9), (66.6, -9.5)], BY - 0.5, mat("rusty_metal"), 0.4)
    for (x0, z0, x1, z1) in ((72.5, -0.8, 68.7, -14.9), (95.0, -0.8, 91.2, -14.9), (117.5, -0.8, 113.7, -14.9)):
        flat("bow_seam_%g" % x0, [(x0, z0), (x0 + 0.22, z0), (x1 + 0.22, z1), (x1, z1)], BY - 0.03, mat("frame_main"))
    for i, x in enumerate([88.1, 89.8, 91.5, 93.3, 95.0, 96.7, 98.5, 112.3, 114.1, 115.8, 117.5, 119.3]):
        z = -3.8 if x < 110 else -4.5
        flat("bow_vent_%d" % i, [(x, z), (x + 0.69, z), (x + 0.35, z - 3.0), (x - 0.35, z - 3.0)], BY - 0.04, mat("void"))
    # The old company name, faded, half covered by a newer plate
    badd(sk.text_object("bow_old_stencil", "BAXTER SALVAGE & CONSTRUCTION", 1.44, [OLD_STENCIL],
                             location=(59.6, BY - 0.04, -11.5), rotation=(math.pi / 2, 0.0, 0.0), extrude=0.01))
    flat("bow_cover_plate", [(88.1, -8.9), (103.7, -8.5), (102.6, -13.3), (87.4, -13.6)], BY - 0.5, ARMOUR_B, 0.4)
    for k in range(7):
        t = (k + 0.5) / 7
        badd(sk.box_object("bow_cover_rivet_%d" % k, (88.6 + 14.6 * t, BY - 0.58, -9.3 + 0.4 * t), (0.3, 0.16, 0.3),
                           mat("pad_metal")))
    # Hull number
    badd(sk.text_object("bow_number", "BC-01", 1.52, [STENCIL], location=(112.3, BY - 0.62, 3.8),
                        rotation=(math.pi / 2, math.radians(8.0), 0.0), extrude=0.01))
    # Shark mouth above the ram, hazard stripes under it
    flat("bow_mouth", [(137.6, -3.1), (160.2, -4.5), (156.0, -6.5), (137.6, -8.9)], BY - 0.05, MOUTH, 0.04)
    top = [(137.6, -3.1), (140.1, -3.3), (142.6, -3.4), (145.1, -3.6), (147.6, -3.7), (150.1, -3.9), (152.6, -4.0),
           (155.2, -4.2), (157.7, -4.3), (160.2, -4.5)]
    bot = [(137.6, -8.9), (139.9, -8.6), (142.2, -8.3), (144.5, -8.0), (146.8, -7.7), (149.1, -7.4), (151.4, -7.1),
           (153.7, -6.8), (156.0, -6.5)]
    for i, (a, b) in enumerate(zip(top, top[1:])):
        flat("bow_tooth_up_%d" % i, [a, b, ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2 - 1.52)], BY - 0.07, TEETH, 0.03)
    for i, (a, b) in enumerate(zip(bot, bot[1:])):
        flat("bow_tooth_lo_%d" % i, [a, ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2 + 1.35), b], BY - 0.07, TEETH, 0.03)
    flat("bow_hazard", [(143.2, -9.9), (153.1, -9.2), (151.8, -10.9), (142.8, -11.9)], BY - 0.12, mat("hazard_paint"), 0.08)
    for i, x in enumerate((144.4, 146.8, 149.2, 151.4)):
        flat("bow_hazard_stripe_%d" % i, [(x, -9.75), (x + 1.0, -9.68), (x - 0.2, -11.4), (x - 1.2, -11.5)],
             BY - 0.14, mat("void"), 0.02)
    # Dorsal blade, swept back
    badd(sk.prism("bow_blade", [(62.1, 19.0), (58.3, 30.7), (63.5, 30.0), (78.5, 20.2)], -1.0, 2.0, mat("frame_main")))
    # Bridge: low, armoured, set back; slit windows and a red strip
    badd(sk.prism("bow_bridge", [(75.9, 19.9), (79.1, 26.3), (93.3, 28.3), (104.5, 25.3), (112.3, 14.9), (103.7, 17.1),
                                 (82.9, 20.5)], -6.0, 12.0, mat("frame_main")))
    flat("bow_bridge_plate", [(79.7, 25.6), (92.9, 27.3), (95.0, 23.6), (81.1, 22.2)], -6.6, ARMOUR_B, 0.6)
    flat("bow_bridge_strip", [(81.1, 24.6), (92.2, 25.9), (92.2, 25.5), (81.1, 24.2)], -6.7, BRIDGE_RED, 0.05)
    for k, (x, z) in enumerate([(96.0, 26.9), (97.8, 26.5), (99.5, 26.0), (101.2, 25.5), (103.0, 25.1)]):
        flat("bow_slit_%d" % k, [(x, z), (x + 1.21, z - 0.34), (x + 1.04, z - 0.85), (x - 0.17, z - 0.51)], -6.1, SLIT)
    # Antennas raked back, strobe on the tallest
    for i, (x, h) in enumerate(((88.1, 7.1), (91.2, 4.7), (100.2, 3.7))):
        badd(sk.prism("bow_antenna_%d" % i, [(x, 27.8), (x - 3.8, 27.8 + h), (x - 3.3, 27.8 + h), (x + 0.9, 27.8)],
                      -0.3, 0.6, mat("frame_main")))
    badd(sk.box_object("nav_bow_top", (84.3, 0.0, 35.3), (0.9, 0.9, 0.9), NAV_WHITE))
    # Twin gun on the foredeck
    badd(sk.prism("bow_turret", [(122.0, 12.4), (123.8, 14.8), (130.7, 15.3), (133.1, 9.7)], -4.0, 8.0, mat("frame_main")))
    for i, z in enumerate((14.2, 13.0)):
        g = cylinder_x("bow_barrel_%d" % i, 129.7, 143.5, 0.3, (-0.8 + i * 1.6, z), mat("frame_main"), 16)
        bow_parts.append(g)
    # Portholes, mostly dark
    for k in range(12):
        x = 61.4 + k * 2.08
        m = mat("window_lit_warm") if rnd.random() < 0.3 else mat("glass_dark")
        badd(sk.box_object("bow_porthole_%d" % k, (x + 0.43, BY - 0.08, -0.7), (0.87, 0.16, 0.68), m))
    badd(sk.box_object("nav_bow", (164.3, BY - 0.2, -4.3), (1.2, 0.4, 1.2), NAV_RED))
    # The old salvage arm, folded under the keel (faded construction yellow)
    badd(sk.prism("arm_mount", [(108.2, -17.3), (116.5, -17.0), (115.8, -20.0), (108.9, -20.4)], -3.0, 6.0, mat("frame_main")))
    badd(sk.prism("arm_boom", [(112.3, -18.3), (142.1, -15.6), (143.2, -17.1), (113.4, -20.4)], -2.5, 5.0,
                  mat("machine_yellow")))
    badd(sk.prism("arm_piston", [(119.3, -17.7), (134.9, -16.3), (135.2, -17.0), (119.6, -18.5)], -3.2, 0.7,
                  mat("strap_steel")))
    badd(sk.prism("arm_wrist", [(141.4, -14.9), (144.2, -14.6), (144.9, -17.7), (141.8, -18.0)], -3.0, 6.0, mat("frame_main")))
    badd(sk.prism("arm_finger_up", [(144.2, -14.9), (148.4, -14.4), (150.1, -15.6), (147.7, -15.8)], -1.5, 3.0,
                  mat("machine_yellow")))
    badd(sk.prism("arm_finger_lo", [(144.2, -17.3), (148.0, -17.7), (149.8, -16.5), (147.3, -18.3)], -1.5, 3.0,
                  mat("machine_yellow")))
    flat("arm_rust", [(122.7, -17.1), (129.7, -16.5), (129.8, -17.7), (123.1, -18.3)], -2.9, mat("rusty_metal"), 0.3)

    # Scale the whole bow about where it meets the habitat
    bow_root = bpy.data.objects.new("bow_root", None)
    bpy.context.scene.collection.objects.link(bow_root)
    bow_root.location = (56.0, 0.0, 0.0)
    sk.parent_keep_transform(bow_parts, bow_root)
    bow_root.scale = (args.bow_scale, args.bow_scale, args.bow_scale)
    bpy.context.view_layer.update()
else:
    bow = [(56.0, -14.0), (56.0, 15.0), (92.0, 17.0), (106.0, 22.0), (122.0, 22.0), (132.0, 14.0),
           (144.0, 3.0), (142.0, -6.0), (128.0, -13.0), (90.0, -16.0)]
    add(sk.prism("bow", bow, BY, 24.0, mat("frame_brace")))
    add(sk.prism("bow_keel", [(58.0, -14.0), (90.0, -16.0), (128.0, -13.0), (142.0, -6.0), (142.5, -8.5),
                              (129.0, -15.2), (90.0, -18.0), (58.0, -16.0)], BY - 0.2, 24.4, mat("frame_main")))
    add(sk.box_object("bridge_windows", (117.0, BY - 0.1, 19.2), (12.0, 0.3, 2.0), mat("window_lit_blue")))
    for k in range(5):
        x = 124.0 + k * 1.9
        z = 18.5 - k * 1.55
        add(sk.box_object("bridge_window_%d" % k, (x, BY - 0.1, z), (1.4, 0.3, 1.7), mat("window_lit_blue")))
    for k in range(14):
        x = 64.0 + k * 4.2
        m = mat("glass_dark") if rnd.random() < 0.2 else mat("window_lit_warm")
        add(sk.box_object("deck_window_%d" % k, (x, BY - 0.1, 4.0), (2.4, 0.3, 1.5), m))
    for x in (75.0, 98.0, 118.0):
        add(sk.box_object("seam_%g" % x, (x, BY - 0.08, -3.0), (0.3, 0.2, 18.0), mat("frame_main")))
    add(sk.box_object("airlock", (86.0, BY - 0.15, -8.0), (5.0, 0.3, 5.5), mat("hazard_paint"), bevel_width=0.1))
    add(sk.box_object("airlock_door", (86.0, BY - 0.3, -8.0), (3.6, 0.2, 4.2), mat("pad_metal")))
    for (x, h) in ((108.0, 9.0), (112.0, 5.0), (116.0, 7.0)):
        add(sk.box_object("antenna_%g" % x, (x, 0.0, 22.0 + h / 2), (0.3, 0.3, h), mat("frame_main")))
    add(sk.box_object("nav_bow", (141.5, BY - 0.2, -3.0), (1.2, 0.4, 1.2), NAV_RED))
    add(sk.box_object("nav_bow_top", (108.0, 0.0, 31.3), (0.9, 0.9, 0.9), NAV_WHITE))
add(sk.box_object("nav_stern", (-118.0, -13.4, 20.5), (1.2, 0.4, 1.2), NAV_GREEN))

# ---------------------------------------------------------------------------
# Turn the ship to the camera, light it, frame it
# ---------------------------------------------------------------------------
root = bpy.data.objects.new("ship_root", None)
bpy.context.scene.collection.objects.link(root)
sk.parent_keep_transform(parts, root)
root.rotation_euler = (math.radians(args.pitch), 0.0, math.radians(args.yaw))
bpy.context.view_layer.update()

# Sunlight from the upper left, in front; the theme's night sky stays as a
# faint fill (space has no sky, but the shadow side shouldn't be black)
sun = bpy.data.objects.get("Sun")
if sun is not None:
    sun.data.energy = args.sun
    sun.data.color = (1.0, 0.96, 0.9)
    sun.data.angle = math.radians(1.0)
    sun.rotation_euler = Vector((0.55, 0.7, -0.45)).normalized().to_track_quat('-Z', 'Y').to_euler()


def frame_box(obs, margin):
    dg = bpy.context.evaluated_depsgraph_get()
    xs, zs = [], []
    for ob in obs:
        if ob.type not in ('MESH', 'FONT', 'CURVE'):
            continue
        ev = ob.evaluated_get(dg)
        for c in ev.bound_box:
            w = ev.matrix_world @ Vector(c)
            xs.append(w.x)
            zs.append(w.z)
    return (min(xs) - margin, min(zs) - margin, max(xs) + margin, max(zs) + margin)


box_xz = frame_box(parts, 1.0)        # with the plumes, so both frames are the same
print("baxter_homestead: frame x %.1f .. %.1f, z %.1f .. %.1f (%.1f x %.1f units)"
      % (box_xz[0], box_xz[2], box_xz[1], box_xz[3], box_xz[2] - box_xz[0], box_xz[3] - box_xz[1]))


def set_engines(on):
    for g in glows:
        g.data.materials[0] = ENGINE_GLOW if on else ENGINE_COLD
    for p in plumes:
        p.hide_render = not on
    for lt in engine_lights:
        lt.hide_render = not on


name = "baxter_homestead" + args.suffix
set_engines(True)
if args.still:
    kit.render_ortho(name, box_xz)
else:
    paths = [kit.render_ortho(name + "_f0", box_xz, frame_only=True)]
    set_engines(False)
    paths.append(kit.render_variant(name + "_f1"))
    if all(paths):
        kit.make_strip(paths, name)
        print("baxter_homestead: frame 0 engines burning, frame 1 cold (c_animateSprite(shape, 0, frame))")
