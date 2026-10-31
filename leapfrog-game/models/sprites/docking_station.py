"""
docking_station - the space docking station: body, arm and clamp.

Bodies in data/docking_station.xml (station coordinates, game y down; here
x is the same and Blender z = -game y):
  stationBase  dock_station_base  22 x 32, centre (0, -1)   -> z -15 .. 17
  dockArm      dock_arm           27.2 x 2.4, centre (16.4, -17) -> x 2.8 .. 30
  dockClamp    dock_clamp         3.2 x 5.4, centre (30, -13.1)
The shoulder (arm pivot) is at (4, -17), on top of the pedestal. The clamp's
key (bottom of its image) seats in the recess along the Leapfrog's nose: a
flat key 3.0 wide narrowing to 2.4, 0.5 deep. The Leapfrog hangs under it,
nose up, 19 units clear of the station body.

Each part is rendered on its own (the others hidden) with the ortho rig at
20 px/unit, framed exactly to its body box. All three are already listed in
res.xml (images/space, placeholders drawn in 2D); a render replaces the
placeholder and keeps the entry.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P docking_station.py
    blender -b --factory-startup -P docking_station.py -- --samples 16 --suffix _test
    blender -b --factory-startup -P docking_station.py -- --only clamp
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to every texture name (e.g. _test: not registered)")
    p.add_argument("--only", choices=["base", "arm", "clamp"], help="render just one part")
    p.add_argument("--tile", type=float, default=3.9,
                   help="game units per hex texture tile on the hull (the Leapfrog's own is ~3.9)")
    p.add_argument("--windows", type=int, default=7, help="lit windows across the window band")
    p.add_argument("--dark-windows", default="2,5", help="window numbers (from 0) that are dark")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res.xml")

parts = {"base": [], "arm": [], "clamp": []}


def add(part, ob):
    parts[part].append(ob)
    return ob


SEG = 96
T = args.tile

# ---------------------------------------------------------------------------
# Station body: a vertical cylinder with flanged end caps, a window band and
# equipment bands, a pedestal on top for the arm's shoulder
# ---------------------------------------------------------------------------
R = 9.6           # hull radius (image half width is 11: flanges reach 10.8)
Z_BOT = -15.0
Z_TOP = 17.0
Z_CAP_TOP = 13.6  # top of the upper flange; the pedestal stands on it

hull = bmesh.new()
uv = hull.loops.layers.uv.new("UVMap")
sk.lathe(hull, [(R, -12.9), (R, -2.2)], SEG, T, uv)
sk.lathe(hull, [(R, 0.8), (R, 11.9)], SEG, T, uv)
add("base", sk.mesh_object("hull", hull, kit.material("hull_white"), smooth_quads=True))

# Flanges (dark) at both ends and two thin equipment bands
fl = bmesh.new()
sk.lathe(fl, [(R - 0.4, Z_BOT), (10.8, Z_BOT + 0.25), (10.8, -13.6), (R, -12.9)], SEG, cap_bottom=True)
sk.lathe(fl, [(R, 11.9), (10.8, 12.6), (10.8, Z_CAP_TOP - 0.25), (R - 0.6, Z_CAP_TOP)], SEG, cap_top=True)
sk.lathe(fl, [(R + 0.15, 6.0), (R + 0.15, 6.5)], SEG)
sk.lathe(fl, [(R + 0.15, -7.5), (R + 0.15, -7.0)], SEG)
add("base", sk.mesh_object("flanges", fl, kit.material("frame_brace"), smooth_quads=True))

# Window band: a dark glass ring with lit windows set into it (game y -1 .. -3
# is z 0.8 .. -2.2 here; the band sits between the two hull sections)
band = bmesh.new()
sk.lathe(band, [(R - 0.25, -2.2), (R - 0.25, 0.8)], SEG)
add("base", sk.mesh_object("window_band", band, kit.material("glass_dark"), smooth_quads=True))

dark = {int(s) for s in args.dark_windows.split(",") if s.strip() != ""}
n = args.windows
for i in range(n):
    a = (i - (n - 1) / 2) * (math.radians(110) / max(1, n - 1))   # spread over the visible front
    x = (R - 0.2) * math.sin(a)
    y = -(R - 0.2) * math.cos(a)
    mat = kit.material("glass_dark" if i in dark else "window_lit_warm")
    add("base", sk.box_object("window_%d" % i, (x, y, -0.7), (1.1, 0.12, 1.6), mat,
                              rotation=(0.0, 0.0, a)))

# Greebles: vertical conduits and a few hatches on the front
for x in (-6.2, 6.2):
    a = math.asin(x / R)
    y = -R * math.cos(a) - 0.18
    add("base", sk.box_object("conduit_%g" % x, (x, y, 5.0), (0.35, 0.35, 13.5), kit.material("frame_main"),
                              rotation=(0.0, 0.0, a)))
for (x, z, w, h) in ((-3.0, -10.5, 2.6, 2.2), (2.5, -10.0, 1.6, 3.0), (0.0, 9.3, 3.4, 1.4)):
    a = math.asin(x / R)
    y = -R * math.cos(a) - 0.05
    add("base", sk.box_object("hatch_%g_%g" % (x, z), (x, y, z), (w, 0.2, h), kit.material("pad_metal"),
                              rotation=(0.0, 0.0, a), bevel_width=0.06))

# Pedestal under the shoulder (station x 4): a short column with a hazard
# striped collar; the hub itself is part of the arm image
ped = bmesh.new()
sk.box(ped, (4.0, 0.0, Z_CAP_TOP + 1.0), (3.6, 3.6, 2.0))
add("base", sk.mesh_object("pedestal", ped, kit.material("frame_main")))
add("base", sk.box_object("pedestal_collar", (4.0, 0.0, Z_CAP_TOP + 2.25), (4.0, 4.0, 0.5),
                          kit.material("hazard_paint"), bevel_width=0.05))
# Upright fork for the shoulder pin, reaching the image top (the arm's hub covers it)
for y in (-1.5, 1.5):
    add("base", sk.box_object("fork_%g" % y, (4.0, y, Z_CAP_TOP + 2.9), (2.0, 0.5, 0.8), kit.material("frame_main")))

# Beacon and antenna on the top cap, left side
add("base", sk.box_object("antenna", (-7.0, 0.0, Z_CAP_TOP + 1.4), (0.18, 0.18, 2.8), kit.material("frame_main")))
beacon = bmesh.new()
sk.lathe(beacon, [(0.0, Z_CAP_TOP + 2.8), (0.35, Z_CAP_TOP + 2.95), (0.35, Z_CAP_TOP + 3.2), (0.0, Z_CAP_TOP + 3.3)],
         16)
b = add("base", sk.mesh_object("beacon", beacon, kit.material("neon_pink"), smooth_quads=True))
b.location.x = -7.0

# Thruster pods on the sides (station keeping)
for sx in (-1, 1):
    pod = bmesh.new()
    sk.box(pod, (sx * 10.2, 0.0, -9.5), (1.4, 2.4, 2.4))
    add("base", sk.mesh_object("pod_%d" % sx, pod, kit.material("frame_main")))
    noz = bmesh.new()
    sk.lathe(noz, [(0.3, 0.0), (0.55, 0.8)], 24)
    nozzle = add("base", sk.mesh_object("pod_nozzle_%d" % sx, noz, kit.material("nozzle"), smooth_quads=True))
    nozzle.location = (sx * 10.6, 0.0, -9.5)
    nozzle.rotation_euler = (0.0, sx * math.pi / 2, 0.0)

# ---------------------------------------------------------------------------
# Arm: a box truss from the shoulder (x 4) to the tip (x 30) at z 17, with a
# hub at the shoulder end and an end plate at the tip. The image starts 1.2
# left of the shoulder so the whole hub fits.
# ---------------------------------------------------------------------------
AZ = 17.0
AX0, AX1 = 4.0, 30.0
HH = 0.95            # chord offset from the arm's centre line (image is 2.4 high)
DY = 0.8             # chord offset in depth
CR = 0.2             # chord radius

tr = bmesh.new()
for sz in (-1, 1):
    for sy in (-1, 1):
        sk.strut(tr, (AX0 + 1.0, sy * DY, AZ + sz * HH), (AX1 - 0.5, sy * DY, AZ + sz * HH), CR, 10)
bays = 13
L = (AX1 - 0.5) - (AX0 + 1.0)
for i in range(bays):
    x0 = AX0 + 1.0 + L * i / bays
    x1 = AX0 + 1.0 + L * (i + 1) / bays
    for sy in (-1, 1):
        sk.strut(tr, (x0, sy * DY, AZ - HH), (x1, sy * DY, AZ + HH) if i % 2 == 0 else (x1, sy * DY, AZ - HH), 0.09, 6)
        sk.strut(tr, (x0, sy * DY, AZ + HH) if i % 2 == 0 else (x0, sy * DY, AZ - HH),
                 (x1, sy * DY, AZ - HH) if i % 2 == 0 else (x1, sy * DY, AZ + HH), 0.09, 6)
        sk.strut(tr, (x1, sy * DY, AZ - HH), (x1, sy * DY, AZ + HH), 0.08, 6)
    sk.strut(tr, (x1, -DY, AZ + HH), (x1, DY, AZ + HH), 0.08, 6)
    sk.strut(tr, (x1, -DY, AZ - HH), (x1, DY, AZ - HH), 0.08, 6)
add("arm", sk.mesh_object("truss", tr, kit.material("frame_brace"), smooth_quads=True))

hub = bmesh.new()
sk.lathe(hub, [(1.1, -1.2), (1.1, 1.2)], 40, cap_top=True, cap_bottom=True)
h = add("arm", sk.mesh_object("hub", hub, kit.material("frame_main"), smooth_quads=True))
h.location = (AX0, 0.0, AZ)
h.rotation_euler = (math.pi / 2, 0.0, 0.0)          # axis along depth (the pivot axis)
cap = bmesh.new()
sk.lathe(cap, [(0.55, -1.3), (0.55, 1.3)], 24, cap_top=True, cap_bottom=True)
c = add("arm", sk.mesh_object("hub_cap", cap, kit.material("hazard_paint"), smooth_quads=True))
c.location = (AX0, 0.0, AZ)
c.rotation_euler = (math.pi / 2, 0.0, 0.0)

add("arm", sk.box_object("tip_plate", (AX1 - 0.35, 0.0, AZ), (0.7, 2.2, 2.3), kit.material("frame_main"),
                         bevel_width=0.05))
# Cable along the lower chord, to the clamp
add("arm", sk.box_object("cable", ((AX0 + AX1) / 2 + 0.5, -DY - 0.15, AZ - HH + 0.2), (AX1 - AX0 - 3.0, 0.12, 0.12),
                         kit.material("hazard_paint")))

# ---------------------------------------------------------------------------
# Clamp: stem from the arm tip down, a crosshead with amber alignment lights
# and the flat key at the bottom (z 10.4 = the grip point)
# ---------------------------------------------------------------------------
CX = 30.0
Z_KEY = 10.4

add("clamp", sk.box_object("stem", (CX, 0.0, 13.6), (1.1, 1.1, 4.4), kit.material("frame_main"), bevel_width=0.05))
add("clamp", sk.box_object("stem_ram", (CX, -0.6, 13.0), (0.45, 0.2, 3.2), kit.material("pad_metal")))
add("clamp", sk.box_object("crosshead", (CX, 0.0, 11.35), (3.2, 1.6, 0.9), kit.material("frame_brace"),
                           bevel_width=0.06))
add("clamp", sk.box_object("crosshead_band", (CX, -0.82, 11.6), (3.0, 0.06, 0.22), kit.material("hazard_paint")))

key = bmesh.new()
top = Z_KEY + 0.5
v = [key.verts.new(p) for p in (
    (CX - 1.5, -0.7, top), (CX + 1.5, -0.7, top), (CX + 1.2, -0.7, Z_KEY), (CX - 1.2, -0.7, Z_KEY),
    (CX - 1.5, 0.7, top), (CX + 1.5, 0.7, top), (CX + 1.2, 0.7, Z_KEY), (CX - 1.2, 0.7, Z_KEY))]
for f in ((0, 1, 2, 3), (5, 4, 7, 6), (4, 5, 1, 0), (3, 2, 6, 7), (0, 3, 7, 4), (1, 5, 6, 2)):
    key.faces.new([v[i] for i in f])
bmesh.ops.recalc_face_normals(key, faces=key.faces[:])
add("clamp", sk.mesh_object("key", key, kit.material("pad_metal")))

for sx in (-1, 1):
    lamp = bmesh.new()
    sk.box(lamp, (CX + sx * 1.15, -0.85, 11.05), (0.35, 0.1, 0.25))
    add("clamp", sk.mesh_object("align_light_%d" % sx, lamp, kit.material("neon_amber")))

# ---------------------------------------------------------------------------
# Render: each part alone, framed to its body box
# ---------------------------------------------------------------------------
frames = {
    "base": ("dock_station_base", (-11.0, Z_BOT, 11.0, Z_TOP)),
    "arm": ("dock_arm", (AX0 - 1.2, AZ - 1.2, AX1, AZ + 1.2)),
    "clamp": ("dock_clamp", (CX - 1.6, Z_KEY, CX + 1.6, Z_KEY + 5.4)),
}

for part, (name, box_xz) in frames.items():
    if args.only and part != args.only:
        continue
    for other, obs in parts.items():
        for ob in obs:
            ob.hide_render = (other != part)
    kit.render_ortho(name + args.suffix, box_xz)
