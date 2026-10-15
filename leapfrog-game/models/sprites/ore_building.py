"""
ore_building - the run-down apartment block where the first mission giver lives.

Three floors, six apartments (three on each side). Each apartment has a
landing pad on the side wall instead of a balcony. The pub "The Ore" takes
the bottom-right slot, with a "Beer" neon in the window (second e dead).
The mission giver lives top right: blue light from the window and the door
to his pad. The whole building leans and is dirty and broken.

Static scenery, so it uses the perspective (3/4) rig like the silo. The
frame is fitted to the building at 20 px/unit at the building's depth, so
"Set Width and Height from texture pixel size" in the editor gives its size.

Pads are part of the image; draw their collision bodies in the editor.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P ore_building.py
    blender -b --factory-startup -P ore_building.py -- --samples 16 --name ore_building_test
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bpy                            # noqa: E402
import bmesh                          # noqa: E402
from mathutils import Vector          # noqa: E402
import sprite_kit as sk               # noqa: E402


# --- What each pad slot looks like ---------------------------------------
# (side, level): level 0 = bottom. States: intact, tilted, fallen, none.
PADS = {
    ("left", 0): "fallen",       # lies on the ground next to the building
    ("left", 1): "intact",       # the one that works
    ("left", 2): "tilted",
    ("right", 0): "none",        # the pub is here
    ("right", 1): "tilted",
    ("right", 2): "intact",      # the mission giver
}
MISSION_GIVER = ("right", 2)
PUB = ("right", 0)


def options(p):
    p.add_argument("--name", default="ore_building", help="texture name (default ore_building)")
    p.add_argument("--lean", type=float, default=4.0, help="lean of the building in degrees (+ = right)")
    p.add_argument("--yaw", type=float, default=18.0, help="camera angle from the right, degrees")
    p.add_argument("--pitch", type=float, default=6.0, help="camera looking down, degrees")
    p.add_argument("--distance", type=float, default=95.0, help="camera distance (less = more perspective)")
    p.add_argument("--seed", type=int, default=7, help="randomness for broken windows, rubble, cracks")
    p.add_argument("--font", default=None, help="TTF/OTF for the signs (default: Blender's font)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="persp")
rnd = random.Random(args.seed)

M = kit.material

# --- Dimensions (game units) ---------------------------------------------
CW = 22.0           # core width
CD = 12.0           # core depth
PLINTH = 1.0
FH = 11.5           # floor height
LEVELS = 3
WALL_TOP = PLINTH + LEVELS * FH          # 35.5
PARAPET = 1.2

PL = 14.0           # pad length (out from the wall), the ship is 11.2 wide
PD = 9.0            # pad depth
PT = 0.7            # pad thickness
TILT = 20.0         # degrees for pads without support

FRONT = -CD / 2     # front face (towards the camera)

building = []       # everything that leans with the building
ground = []         # things lying on the ground (don't lean)


def side_sign(side):
    return -1.0 if side == "left" else 1.0


def floor_z(level):
    return PLINTH + level * FH


# --- Core -------------------------------------------------------------------
core = sk.box_object("core", (0, 0, WALL_TOP / 2), (CW, CD, WALL_TOP), M("concrete_dirty"), bevel_width=0.15)
building.append(core)
building.append(sk.box_object("plinth", (0, 0, PLINTH / 2), (CW + 0.8, CD + 0.8, PLINTH), M("concrete_dirty")))
for i in range(1, LEVELS + 1):
    z = floor_z(i) if i < LEVELS else WALL_TOP
    building.append(sk.box_object("slab_%d" % i, (0, 0, z), (CW + 0.5, CD + 0.5, 0.55), M("concrete_dirty")))

# Parapet
for name, c, s in (("front", (0, FRONT + 0.25, 0), (CW + 0.5, 0.5, PARAPET)),
                   ("back", (0, CD / 2 - 0.25, 0), (CW + 0.5, 0.5, PARAPET)),
                   ("left", (-CW / 2 + 0.25, 0, 0), (0.5, CD, PARAPET)),
                   ("right", (CW / 2 - 0.25, 0, 0), (0.5, CD, PARAPET))):
    building.append(sk.box_object("parapet_" + name, (c[0], c[1], WALL_TOP + PARAPET / 2), s, M("concrete_dirty")))


# --- Windows and doors ------------------------------------------------------
def front_window(name, cx, cz, w, h, kind):
    building.append(sk.box_object(name + "_frame", (cx, FRONT - 0.06, cz), (w + 0.6, 0.25, h + 0.6), M("rusty_metal")))
    pane_mat = {"lit": "window_lit_blue", "dark": "glass_dark", "broken": "void"}[kind]
    building.append(sk.box_object(name + "_pane", (cx, FRONT - 0.08, cz), (w, 0.25, h), M(pane_mat)))
    building.append(sk.box_object(name + "_sill", (cx, FRONT - 0.3, cz - h / 2 - 0.42), (w + 1.0, 0.6, 0.25),
                                  M("concrete_dirty")))
    if kind == "broken":
        # A few glass shards left in the corners
        bm = bmesh.new()
        y = FRONT - 0.215
        x0, x1 = cx - w / 2, cx + w / 2
        z0, z1 = cz - h / 2, cz + h / 2
        corners = [((x0, z1), (1, -1)), ((x1, z1), (-1, -1)), ((x0, z0), (1, 1)), ((x1, z0), (-1, 1))]
        for (px, pz), (dx, dz) in rnd.sample(corners, rnd.randint(1, 3)):
            a = rnd.uniform(0.6, w * 0.6)
            b = rnd.uniform(0.6, h * 0.5)
            v1 = bm.verts.new((px, y, pz))
            v2 = bm.verts.new((px + dx * a, y, pz))
            v3 = bm.verts.new((px + dx * rnd.uniform(0.0, a * 0.4), y, pz + dz * b))
            bm.faces.new((v1, v2, v3))
        building.append(sk.mesh_object(name + "_shards", bm, M("glass_dark")))


def side_door(name, s, z0, kind):
    x = s * (CW / 2)
    building.append(sk.box_object(name + "_frame", (x + s * 0.06, 0, z0 + 2.5), (0.25, 3.2, 5.0), M("rusty_metal")))
    mat = {"lit": "window_lit_blue", "dark": "glass_dark", "broken": "void"}[kind]
    building.append(sk.box_object(name + "_panel", (x + s * 0.08, 0, z0 + 2.3), (0.25, 2.6, 4.5), M(mat)))


for side in ("left", "right"):
    s = side_sign(side)
    for level in range(LEVELS):
        z0 = floor_z(level)
        slot = (side, level)
        if slot == PUB:
            continue
        if slot == MISSION_GIVER:
            kinds = ["lit", "lit"]
        else:
            kinds = [rnd.choice(["broken", "broken", "dark"]) for _ in range(2)]
        for k, dx in enumerate((-2.4, 2.4)):
            front_window("win_%s%d_%d" % (side, level, k), s * CW / 4 + dx, z0 + 5.8, 3.0, 4.2, kinds[k])
        if PADS[slot] != "none":
            door = "lit" if slot == MISSION_GIVER else ("dark" if PADS[slot] == "intact" else "broken")
            side_door("door_%s%d" % (side, level), s, z0, door)

# Mission giver's light spilling out of the window and onto his pad
mg_z = floor_z(MISSION_GIVER[1])
mg_s = side_sign(MISSION_GIVER[0])
building.append(sk.point_light("mg_window_light", (mg_s * CW / 4, FRONT - 1.4, mg_z + 5.8),
                               (0.35, 0.6, 1.0), 220.0, 1.0))
building.append(sk.point_light("mg_door_light", (mg_s * (CW / 2 + 1.3), 0, mg_z + 2.5),
                               (0.35, 0.6, 1.0), 180.0, 0.8))


# --- The pub "The Ore" (bottom right) ---------------------------------------
pz = floor_z(PUB[1])
ps = side_sign(PUB[0])
door_x = ps * 2.4
window_x = ps * 7.4
# Door
building.append(sk.box_object("pub_door_frame", (door_x, FRONT - 0.06, pz + 2.7), (3.2, 0.25, 5.6), M("rusty_metal")))
building.append(sk.box_object("pub_door", (door_x, FRONT - 0.1, pz + 2.6), (2.6, 0.25, 5.2), M("pad_metal")))
# Big window, mostly dark inside
building.append(sk.box_object("pub_window_frame", (window_x, FRONT - 0.06, pz + 3.8), (6.4, 0.25, 4.6), M("rusty_metal")))
building.append(sk.box_object("pub_window", (window_x, FRONT - 0.08, pz + 3.8), (5.8, 0.25, 4.0), M("glass_dark")))
# "Beer" neon in the window, second e dead
beer = sk.text_object("neon_beer", "Beer", 1.9, [M("neon_pink"), M("neon_dead")],
                      location=(window_x, FRONT - 0.35, pz + 3.8), neon=True, tube=0.07, font_path=args.font)
beer.data.body_format[2].material_index = 1
building.append(beer)
building.append(sk.point_light("neon_beer_light", (window_x, FRONT - 1.6, pz + 3.8), (1.0, 0.15, 0.5), 90.0, 0.6))
# Sign board with "The Ore"
sign_x = ps * 4.9
sign_z = pz + 8.4
building.append(sk.box_object("pub_sign_board", (sign_x, FRONT - 0.2, sign_z), (9.4, 0.3, 2.2), M("rusty_metal"),
                              rotation=(0.0, math.radians(1.5), 0.0)))
ore = sk.text_object("neon_the_ore", "The Ore", 1.5, [M("neon_amber")],
                     location=(sign_x, FRONT - 0.45, sign_z), rotation=(math.pi / 2, math.radians(1.5), 0.0),
                     neon=True, tube=0.06, font_path=args.font)
building.append(ore)
building.append(sk.point_light("neon_ore_light", (sign_x, FRONT - 1.8, sign_z), (1.0, 0.45, 0.1), 90.0, 0.6))


# --- Landing pads -----------------------------------------------------------
def pad_mesh(name, s):
    """Pad with its origin on the hinge line at the wall, top surface at z = 0."""
    bm = bmesh.new()
    sk.box(bm, (s * PL / 2, 0, -PT / 2), (PL, PD, PT))
    pad = sk.mesh_object(name, bm, M("pad_metal"))
    sk.bevel(pad, 0.05)
    # Worn hazard stripe along the outer edge
    bm = bmesh.new()
    sk.box(bm, (s * (PL - 0.9), 0, 0.02), (0.8, PD - 0.4, 0.06))
    stripe = sk.mesh_object(name + "_stripe", bm, M("hazard_paint"))
    stripe.parent = pad
    # Low kerb at the back of the pad
    bm = bmesh.new()
    sk.box(bm, (s * PL / 2, PD / 2 - 0.2, 0.25), (PL, 0.4, 0.5))
    kerb = sk.mesh_object(name + "_kerb", bm, M("pad_metal"))
    kerb.parent = pad
    return pad, [stripe, kerb]


def strut_points(s, z0, y):
    a = Vector((s * CW / 2, y, z0 - 6.0))                 # on the wall
    b = Vector((s * (CW / 2 + PL - 1.0), y, z0 - PT))     # under the outer end of the pad
    return a, b


def struts(name, s, z0, broken):
    bm = bmesh.new()
    for y in (-PD / 2 + 1.2, PD / 2 - 1.2):
        a, b = strut_points(s, z0, y)
        # bracket plate on the wall
        sk.box(bm, (a.x + s * 0.1, y, a.z), (0.2, 1.0, 1.4))
        if broken:
            end = a + (b - a).normalized() * rnd.uniform(1.5, 2.8)
            sk.strut(bm, a, end, 0.22)
        else:
            sk.strut(bm, a, b, 0.22)
    return sk.mesh_object(name, bm, M("rusty_metal"), smooth_quads=True)


for (side, level), state in PADS.items():
    if state == "none":
        continue
    s = side_sign(side)
    z0 = floor_z(level)
    name = "pad_%s%d" % (side, level)

    if state == "intact":
        pad, parts = pad_mesh(name, s)
        pad.location = (s * CW / 2, 0, z0)
        building += [pad] + parts
        building.append(struts(name + "_struts", s, z0, broken=False))

    elif state == "tilted":
        pad, parts = pad_mesh(name, s)
        pad.location = (s * CW / 2, 0, z0)
        pad.rotation_euler = (math.radians(rnd.uniform(-3, 3)), s * math.radians(TILT + rnd.uniform(-3, 3)), 0.0)
        building += [pad] + parts
        building.append(struts(name + "_stubs", s, z0, broken=True))

    elif state == "fallen":
        # Torn-off stubs left on the wall
        building.append(struts(name + "_stubs", s, z0 + 6.5, broken=True))
        # The pad itself on the ground, inner end resting on rubble
        pad, parts = pad_mesh(name, s)
        lift = 1.4
        pad.location = (s * (CW / 2 + 1.5), -1.0, PT + lift)
        pad.rotation_euler = (math.radians(4.0), s * math.asin(lift / PL), math.radians(-s * 10.0))
        ground += [pad] + parts
        # Its support beams lying next to it
        bm = bmesh.new()
        for k in range(2):
            x0 = s * (CW / 2 + 2.0 + k * 3.0)
            sk.strut(bm, (x0, -PD / 2 - 1.0 - k, 0.22), (x0 + s * 7.5, -PD / 2 + 0.5 - k * 2, 0.22), 0.22)
        ground.append(sk.mesh_object(name + "_fallen_struts", bm, M("rusty_metal"), smooth_quads=True))


# --- Roof clutter -----------------------------------------------------------
tank_x, tank_y = 5.5, 2.0
tank_base = WALL_TOP + 1.3
bm = bmesh.new()
sk.lathe(bm, [(2.0, tank_base), (2.0, tank_base + 3.0)], 32, cap_top=True, cap_bottom=True)
building.append(sk.mesh_object("water_tank", bm, M("rusty_metal"), smooth_quads=True))
building[-1].location = (tank_x, tank_y, 0)
bm = bmesh.new()
for dx, dy in ((-1.4, -1.4), (1.4, -1.4), (-1.4, 1.4), (1.4, 1.4)):
    sk.strut(bm, (tank_x + dx, tank_y + dy, WALL_TOP), (tank_x + dx * 0.9, tank_y + dy * 0.9, tank_base), 0.12)
building.append(sk.mesh_object("water_tank_legs", bm, M("rusty_metal"), smooth_quads=True))

mast_base = Vector((-6.0, 2.0, WALL_TOP))
mast_top = Vector((-5.0, 2.0, WALL_TOP + 6.5))        # a bit bent
bm = bmesh.new()
sk.strut(bm, mast_base, mast_top, 0.15)
for t in (0.45, 0.7, 0.9):
    p = mast_base.lerp(mast_top, t)
    sk.strut(bm, p + Vector((-1.4, 0, 0)), p + Vector((1.4, 0, 0.1 if t != 0.7 else -0.6)), 0.07)
building.append(sk.mesh_object("antenna", bm, M("rusty_metal"), smooth_quads=True))


# --- Cracks in the facade ---------------------------------------------------
def crack(name, x, z_top, length):
    bm = bmesh.new()
    p = Vector((x, FRONT - 0.03, z_top))
    for _ in range(int(length / 0.9)):
        q = p + Vector((rnd.uniform(-0.7, 0.7), 0, -rnd.uniform(0.6, 1.1)))
        d = q - p
        mid = (p + q) / 2
        ang = math.atan2(d.x, d.z)
        bm_seg = bmesh.new()
        sk.box(bm_seg, (0, 0, 0), (0.12, 0.05, d.length + 0.08))
        ob = sk.mesh_object(name, bm_seg, M("void"))
        ob.location = mid
        ob.rotation_euler = (0.0, ang, 0.0)
        building.append(ob)
        p = q


crack("crack_a", -1.2, WALL_TOP - 0.3, 9.0)
crack("crack_b", 9.8, floor_z(1) + 1.0, 6.0)


# --- Rubble on the ground ---------------------------------------------------
for k in range(26):
    sx = rnd.choice((-1, 1))
    x = sx * rnd.uniform(CW / 2 - 2, CW / 2 + 6) if rnd.random() < 0.6 else rnd.uniform(-CW / 2, CW / 2)
    size = rnd.uniform(0.3, 1.3)
    ob = sk.box_object("rubble_%d" % k, (x, FRONT - rnd.uniform(0.5, 3.0), size * 0.35),
                       (size, size * rnd.uniform(0.6, 1.2), size * rnd.uniform(0.4, 0.9)),
                       M("concrete_dirty" if rnd.random() < 0.7 else "rusty_metal"),
                       rotation=(rnd.uniform(-0.4, 0.4), rnd.uniform(-0.4, 0.4), rnd.uniform(0, math.pi)))
    ground.append(ob)


# --- Lean and render --------------------------------------------------------
lean = bpy.data.objects.new("lean", None)
bpy.context.scene.collection.objects.link(lean)
top_level = [o for o in building if o.parent is None]
sk.parent_keep_transform(top_level, lean)
lean.rotation_euler = (0.0, math.radians(args.lean), 0.0)

kit.render_persp(args.name, target=(0.0, 0.0, WALL_TOP / 2 + 2.0),
                 yaw_deg=args.yaw, pitch_deg=args.pitch, distance=args.distance, ground_z=0.0)
