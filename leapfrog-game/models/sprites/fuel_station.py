"""
fuel_station - the fuel station at the Orb: fuel for credits. Static scenery
at parallax 0, landing on its pad.

Three big fuel tanks lie like toppled silos, two on concrete cradles and one
on top between them, strapped down, with valves and a manifold at their
front ends and a pipe along the ground to the pad. The landing pad is a
steel deck on a concrete pedestal, with hazard stripes on its front edge,
foot marks where the Leapfrog's feet go (21.4 apart), a dispenser with a lit
display and the price (5 CR, the pad's default fuelPrice), a hose on a
holster, a ladder, and a floodlight. A small kiosk with a lit window stands
by the pad. On top of the tanks a FUEL sign in cyan neon is half broken: the
U flickers and the L is dead.

Two images:
  fuel_station.png       the station (the sign as in the first animation
                         frame: F, U and E lit)
  fuel_station_neon.png  the sign alone, cropped from the same camera, as an
                         animation strip of --frames frames. Place it on top
                         of the station (higher zLevel) at the offset the
                         script prints, and give it animatePeriod (default
                         2.4 s for 12 frames). The whole station as a
                         12-frame strip would be far over 2048 px.

Camera: the same as the ore building (seen from the right, yaw 18, pitch 6,
distance 95), so they look alike side by side. The pad is laid out square to
the camera, so its front edge is level in the image and a plain box body
matches it; the camera aims at the pad's standing line, so 1 unit there is 1
game unit and the Leapfrog's feet land on the marks. The tanks keep the
world axes, so we see their round ends at an angle.

Lighting: night by default, like the rest of the home scene (--day for
daylight).

Collision bodies: draw them in the editor. After the render the script
prints where the pad's front edge, its standing line and the foot marks land
in the image, as offsets from the image centre in game units (x right, y
down). Suggested: the pad as a static box with behaviour landingPad and
fuel="sell", its top on the standing line; the tanks as scenery only.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P fuel_station.py -- --samples 16 --name fuel_station_test
    blender -b --factory-startup -P fuel_station.py
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                          # noqa: E402
import sprite_kit as sk               # noqa: E402


def options(p):
    p.add_argument("--name", default="fuel_station", help="texture name (default fuel_station)")
    p.add_argument("--yaw", type=float, default=18.0, help="camera angle from the right, degrees (default 18)")
    p.add_argument("--pitch", type=float, default=6.0, help="camera looking down, degrees (default 6)")
    p.add_argument("--distance", type=float, default=95.0, help="camera distance (default 95, as the ore building)")
    p.add_argument("--pad-width", type=float, default=30.0,
                   help="length of the pad along the image (the Leapfrog's feet span 21.4; default 30)")
    p.add_argument("--frames", type=int, default=12, help="frames of the neon animation (default 12)")
    p.add_argument("--flicker", default="111110101111",
                   help="the U per frame, 1 = lit (default 111110101111); repeats if shorter than --frames")
    p.add_argument("--neon-power", type=float, default=180.0, help="watts of cyan light from the sign (default 180)")
    p.add_argument("--no-neon-strip", action="store_true", help="only the station image, no animated sign")
    p.add_argument("--seed", type=int, default=5, help="randomness for clutter (default 5)")
    p.add_argument("--font", default=None, help="TTF/OTF for the neon (default: Blender's font)")
    p.add_argument("--ground-depth", type=float, default=1.8,
                   help="thickness of the ground under the station, 0 for none (default 1.8)")


args = sk.parse_args(options)
if not args.day:
    args.night = True             # the home scene is at night
kit = sk.Kit(args, rig="persp", res="res_mining.xml")
M = kit.material
rnd = random.Random(args.seed)

YAW = math.radians(args.yaw)
CY, SY = math.cos(YAW), math.sin(YAW)


def box(name, center, size, role, rot=(0.0, 0.0, 0.0), bev=0.0):
    return sk.box_object(name, center, size, M(role), rotation=rot, bevel_width=bev)


# --- Pad frame: square to the camera -------------------------------------------
# Pad coordinates (x along the image, y into the picture) to world
PAD_X, PAD_Y = 9.0, -1.0            # pad centre in the world


def pw(x, y, z):
    return (PAD_X + x * CY - y * SY, PAD_Y + x * SY + y * CY, z)


def pbox(name, center, size, role, bev=0.0):
    return box(name, pw(*center), size, role, rot=(0.0, 0.0, YAW), bev=bev)


PADW = args.pad_width
PADD = 11.0
PZ = 4.5                            # top of the deck
PFRONT = -PADD / 2
STAND = PFRONT + 3.5                # standing line: where the feet come down
FEET = 21.4 / 2

pbox("pad_pedestal", (0, 0.3, (PZ - 0.6) / 2), (PADW - 2.4, PADD - 1.6, PZ - 0.6), "concrete_dirty", bev=0.1)
pbox("pad_deck", (0, 0, PZ - 0.3), (PADW, PADD, 0.6), "pad_metal", bev=0.05)
pbox("pad_edge", (0, PFRONT + 0.1, PZ - 0.75), (PADW, 0.3, 0.6), "frame_main")
# Support legs under the deck's overhang
legs = bmesh.new()
for x in (-PADW / 2 + 0.6, PADW / 2 - 0.6):
    for y in (PFRONT + 0.6, -PFRONT - 0.6):
        sk.box(legs, pw(x, y, (PZ - 0.6) / 2), (0.5, 0.5, PZ - 0.6))
sk.mesh_object("pad_legs", legs, M("frame_main"))
# Hazard stripes along the front edge
n = 18
for k in range(0, n, 2):
    x0 = -PADW / 2 + PADW * k / n
    pbox("pad_stripe_%d" % k, (x0 + PADW / n / 2, PFRONT + 0.5, PZ + 0.02), (PADW / n - 0.05, 0.8, 0.06), "hazard_paint")
# Foot marks
for sx in (-1, 1):
    for dx, dy, w, d in ((0, -1.0, 3.2, 0.25), (0, 1.0, 3.2, 0.25), (-1.5, 0, 0.25, 2.2), (1.5, 0, 0.25, 2.2)):
        pbox("foot_mark_%d_%g_%g" % (sx, dx, dy), (sx * FEET + dx, STAND + dy, PZ + 0.03), (w, d, 0.05), "hazard_paint")
# Ladder up the pad's left end
lad = bmesh.new()
lx = -PADW / 2 - 0.4
for dy in (-0.5, 0.5):
    sk.strut(lad, pw(lx, PFRONT + 2.2 + dy, 0.0), pw(lx, PFRONT + 2.2 + dy, PZ + 1.4), 0.07, 6)
for k in range(1, int(PZ / 0.6) + 1):
    sk.strut(lad, pw(lx, PFRONT + 1.7, k * 0.6), pw(lx, PFRONT + 2.7, k * 0.6), 0.05, 6)
sk.mesh_object("pad_ladder", lad, M("frame_brace"))
# Railing along the back of the pad (not over the landing area)
rail = bmesh.new()
for k in range(7):
    x = -PADW / 2 + 1.0 + (PADW - 2.0) * k / 6
    sk.strut(rail, pw(x, -PFRONT - 0.4, PZ), pw(x, -PFRONT - 0.4, PZ + 2.0), 0.06, 6)
sk.strut(rail, pw(-PADW / 2 + 1.0, -PFRONT - 0.4, PZ + 2.0), pw(PADW / 2 - 1.0, -PFRONT - 0.4, PZ + 2.0), 0.07, 6)
sk.strut(rail, pw(-PADW / 2 + 1.0, -PFRONT - 0.4, PZ + 1.0), pw(PADW / 2 - 1.0, -PFRONT - 0.4, PZ + 1.0), 0.05, 6)
sk.mesh_object("pad_rail", rail, M("hazard_paint"))

# Dispenser at the back left of the pad
DX, DY = -PADW / 2 + 3.2, -PFRONT - 1.6
pbox("dispenser_base", (DX, DY, PZ + 0.2), (2.6, 1.9, 0.4), "frame_main")
pbox("dispenser", (DX, DY, PZ + 2.6), (2.2, 1.5, 4.4), "container_blue", bev=0.06)
pbox("dispenser_top", (DX, DY, PZ + 5.0), (2.6, 1.8, 0.5), "frame_main")
pbox("dispenser_display", (DX, DY - 0.77, PZ + 3.7), (1.5, 0.05, 0.9), "window_lit_blue")
price = sk.text_object("dispenser_price", "5 CR", 0.55, [M("neon_amber")],
                       location=pw(DX, DY - 0.82, PZ + 2.75), rotation=(math.pi / 2, 0.0, YAW),
                       neon=False, extrude=0.02, font_path=args.font)
pbox("dispenser_holster", (DX + 1.2, DY - 0.3, PZ + 2.4), (0.3, 0.5, 0.9), "frame_main")
hose = bmesh.new()
hp = [(DX + 1.1, DY, PZ + 4.2), (DX + 1.7, DY - 0.2, PZ + 3.4), (DX + 1.9, DY - 0.4, PZ + 1.6),
      (DX + 1.6, DY - 0.5, PZ + 0.5), (DX + 1.35, DY - 0.4, PZ + 1.9)]
for a_, b_ in zip(hp, hp[1:]):
    sk.strut(hose, pw(*a_), pw(*b_), 0.12, 8)
sk.mesh_object("dispenser_hose", hose, M("rubber"), smooth_quads=True)
sk.point_light("display_light", pw(DX, DY - 2.0, PZ + 3.6), (0.4, 0.6, 1.0), 40.0, 0.4)

# Floodlight mast at the back right of the pad
MX, MY = PADW / 2 - 1.2, -PFRONT - 0.9
mast = bmesh.new()
sk.strut(mast, pw(MX, MY, PZ), pw(MX, MY, PZ + 11.0), 0.18, 8)
sk.strut(mast, pw(MX, MY, PZ + 10.6), pw(MX - 2.2, MY - 0.6, PZ + 10.9), 0.1, 6)
sk.mesh_object("flood_mast", mast, M("frame_main"))
pbox("flood_head", (MX - 2.4, MY - 0.7, PZ + 10.6), (1.4, 1.0, 0.6), "frame_main")
pbox("flood_glass", (MX - 2.4, MY - 0.7, PZ + 10.27), (1.1, 0.8, 0.06), "window_lit_warm")
sk.point_light("flood_light", pw(MX - 3.5, STAND, PZ + 8.0), (1.0, 0.85, 0.65), 900.0, 0.8)

# --- Kiosk by the pad --------------------------------------------------------------
KX, KY = PADW / 2 + 5.0, -0.5
pbox("kiosk", (KX, KY, 3.0), (6.0, 5.6, 6.0), "concrete_dirty", bev=0.08)
pbox("kiosk_roof", (KX, KY - 0.4, 6.25), (7.2, 7.0, 0.5), "rusty_metal")
pbox("kiosk_window_frame", (KX - 0.9, KY - 2.86, 3.6), (3.2, 0.2, 2.4), "rusty_metal")
pbox("kiosk_window", (KX - 0.9, KY - 2.9, 3.6), (2.8, 0.2, 2.0), "window_lit_warm")
pbox("kiosk_door", (KX + 1.8, KY - 2.86, 2.4), (1.6, 0.2, 4.6), "frame_brace")
pbox("kiosk_counter", (KX - 0.9, KY - 3.2, 2.45), (3.4, 0.7, 0.2), "frame_main")
sk.point_light("kiosk_light", pw(KX - 0.9, KY - 4.2, 3.6), (1.0, 0.7, 0.45), 120.0, 0.8)
ant = bmesh.new()
sk.strut(ant, pw(KX + 2.0, KY + 1.5, 6.5), pw(KX + 2.3, KY + 1.5, 10.0), 0.07, 6)
sk.strut(ant, pw(KX + 1.4, KY + 1.5, 9.2), pw(KX + 3.0, KY + 1.5, 9.3), 0.05, 6)
sk.mesh_object("kiosk_antenna", ant, M("frame_main"))

# --- Tanks ------------------------------------------------------------------------
R = 3.8
TL = 18.0                           # tank length (along the world y axis)
TY0, TY1 = -7.5, -7.5 + TL
CRADLE = 0.9
GAP = 2 * R + 0.2
TX = -23.0
bottom_z = R + CRADLE
top_z = bottom_z + math.sqrt(GAP ** 2 - (GAP / 2) ** 2)
TANKS = [("tank_left", TX - GAP / 2, bottom_z, "frame_brace"),
         ("tank_right", TX + GAP / 2, bottom_z, "frame_brace"),
         ("tank_top", TX, top_z, "rusty_metal")]


def tank(name, x, z, role):
    bm = bmesh.new()
    sk.strut(bm, (x, TY0, z), (x, TY1, z), R, 36)
    sk.mesh_object(name, bm, M(role), smooth_quads=True)
    ends = bmesh.new()
    for y, d in ((TY0, -1), (TY1, 1)):
        sk.strut(ends, (x, y, z), (x, y + d * 0.55, z), R * 0.86, 32)
        sk.strut(ends, (x, y + d * 0.5, z), (x, y + d * 0.85, z), R * 0.55, 28)
    sk.mesh_object(name + "_ends", ends, M(role), smooth_quads=True)
    bands = bmesh.new()
    for y in (TY0 + 0.4, TY0 + TL * 0.5, TY1 - 0.4):
        sk.strut(bands, (x, y - 0.12, z), (x, y + 0.12, z), R + 0.06, 36)
    sk.mesh_object(name + "_bands", bands, M("frame_main"), smooth_quads=True)
    # A stencil band and a manway hatch on top
    sk.box_object(name + "_hatch", (x, TY0 + TL * 0.3, z + R + 0.1), (1.6, 1.6, 0.4), M("frame_main"))
    return name


for name, x, z, role in TANKS:
    tank(name, x, z, role)
# Hazard band on the front end of the top tank
band = bmesh.new()
sk.strut(band, (TX, TY0 - 0.6, top_z), (TX, TY0 - 0.5, top_z), R * 0.86 + 0.02, 32)
sk.mesh_object("tank_top_band", band, M("hazard_paint"), smooth_quads=True)

# Concrete cradles under the bottom tanks
for name, x, z, role in TANKS[:2]:
    for y in (TY0 + 2.5, TY1 - 2.5):
        box("%s_cradle_%g" % (name, y), (x, y, (CRADLE + 1.4) / 2), (2 * R - 0.4, 1.4, CRADLE + 1.4),
            "concrete_dirty", bev=0.06)
# Straps over the top tank, bolted to the bottom tanks
straps = bmesh.new()
for y in (TY0 + 3.5, TY1 - 3.5):
    pts = []
    for k in range(13):
        a = math.radians(-30 + 240 * k / 12.0)
        pts.append((TX + (R + 0.08) * math.cos(a), y, top_z + (R + 0.08) * math.sin(a)))
    for a_, b_ in zip(pts, pts[1:]):
        sk.strut(straps, a_, b_, 0.1, 6)
sk.mesh_object("straps", straps, M("frame_main"))

# Valves and a manifold at the front ends, a pipe along the ground to the pad
pipes = bmesh.new()
valves = bmesh.new()
MZ = 1.2
for name, x, z, role in TANKS:
    y = TY0 - 0.9
    sk.strut(pipes, (x, y, z - R * 0.4), (x, y - 0.8, z - R * 0.4), 0.22, 10)
    sk.strut(pipes, (x, y - 0.8, z - R * 0.4), (x, y - 0.8, MZ), 0.22, 10)
    sk.strut(valves, (x - 0.1, y - 1.25, z - R * 0.4 - 0.9), (x + 0.1, y - 1.25, z - R * 0.4 - 0.9), 0.55, 14)
sk.strut(pipes, (TX - GAP / 2 - 0.5, TY0 - 1.7, MZ), (TX + GAP / 2 + 0.5, TY0 - 1.7, MZ), 0.3, 12)
end = pw(-PADW / 2 + 1.2, 0.0, MZ)
sk.strut(pipes, (TX + GAP / 2 + 0.5, TY0 - 1.7, MZ), (end[0] - 0.4, TY0 - 1.7, MZ), 0.26, 12)
sk.strut(pipes, (end[0] - 0.4, TY0 - 1.7, MZ), end, 0.26, 12)
sk.strut(pipes, end, (end[0], end[1], PZ - 0.8), 0.26, 12)
for k in range(4):
    x = TX + GAP / 2 + 2.0 + k * 3.2
    sk.box(pipes, (x, TY0 - 1.7, MZ / 2), (0.5, 0.7, MZ))          # pipe stands
sk.mesh_object("fuel_pipes", pipes, M("rusty_metal"), smooth_quads=True)
sk.mesh_object("valve_wheels", valves, M("container_red"), smooth_quads=True)

# --- The FUEL sign on top of the tanks -------------------------------------------------
SIGN_Z = top_z + R + 3.6
SX, SYW = TX, TY0 + 1.5
posts = bmesh.new()
for dx in (-3.5, 3.5):
    sk.strut(posts, (SX + dx, SYW, top_z + R * 0.8), (SX + dx * CY, SYW + dx * SY, SIGN_Z - 1.6), 0.15, 8)
sk.mesh_object("sign_posts", posts, M("frame_main"))
sign_board = box("sign_board", (SX, SYW + 0.3, SIGN_Z), (12.0, 0.4, 3.6), "frame_main", rot=(0.0, 0.0, YAW), bev=0.05)
fuel = sk.text_object("neon_fuel", "FUEL", 2.6, [M("neon_cyan"), M("neon_dead")],
                      location=(SX, SYW - 0.05, SIGN_Z - 0.05), rotation=(math.pi / 2, 0.0, YAW),
                      neon=True, tube=0.09, font_path=args.font)
fuel.data.body_format[3].material_index = 1          # the L is dead
neon_light = sk.point_light("neon_fuel_light", (SX, SYW - 3.4, SIGN_Z - 0.8), (0.2, 0.8, 1.0), args.neon_power, 1.2)

frames = max(1, args.frames)
pattern = (args.flicker * frames)[:frames] if args.flicker else "1" * frames


def set_frame(i):
    u_on = pattern[i % len(pattern)] == "1"
    fuel.data.body_format[1].material_index = 0 if u_on else 1
    neon_light.energy = args.neon_power * (3.0 if u_on else 2.0) / 3.0


set_frame(0)

# --- Clutter ----------------------------------------------------------------------------
for k, (x, y, role) in enumerate(((-12.5, -9.0, "container_red"), (-11.0, -9.6, "rusty_metal"),
                                  (-27.8, -8.6, "container_blue"))):
    bm = bmesh.new()
    sk.lathe(bm, [(0.75, 0.0), (0.75, 2.2)], 18, cap_top=True, cap_bottom=True)
    ob = sk.mesh_object("barrel_%d" % k, bm, M(role), smooth_quads=True)
    ob.location = (x, y, 0.0)
box("hose_crate", pw(PADW / 2 + 2.0, PFRONT - 2.4, 0.6), (2.2, 1.6, 1.2), "rusty_metal", rot=(0.0, 0.0, YAW + 0.2))

# --- Ground: a level patch, front edge square to the camera -------------------------------------
if args.ground_depth > 0:
    def gw(xp, yp, z):
        return (xp * CY - yp * SY, xp * SY + yp * CY, z)
    bm = bmesh.new()
    nx, ny = 56, 10
    x0, x1, y0, y1 = -38.0, 38.0, -15.0, 14.0
    top = []
    for j in range(ny + 1):
        row = []
        for i in range(nx + 1):
            xp = x0 + (x1 - x0) * i / nx
            yp = y0 + (y1 - y0) * j / ny
            z = rnd.uniform(-0.05, 0.05)
            if j == 0:
                yp += rnd.uniform(-0.4, 0.3)
                z = rnd.uniform(-0.15, 0.0)
            row.append(bm.verts.new(gw(xp, yp, z)))
        top.append(row)
    for j in range(ny):
        for i in range(nx):
            bm.faces.new((top[j][i], top[j][i + 1], top[j + 1][i + 1], top[j + 1][i]))
    front = top[0]
    bottom = [bm.verts.new((v.co.x, v.co.y, -args.ground_depth)) for v in front]
    for i in range(nx):
        bm.faces.new((front[i], front[i + 1], bottom[i + 1], bottom[i]))
    sk.mesh_object("ground", bm, M("ground_dirt"))

# --- Render -------------------------------------------------------------------------------
# The camera aims at the pad's standing line, so 1 unit there is 1 game unit
kit.render_persp(args.name, target=pw(0.0, STAND, PZ), yaw_deg=args.yaw, pitch_deg=args.pitch,
                 distance=args.distance, ground_z=-args.ground_depth - 0.01)

kit.report_points("fuel station: where the bodies go", [
    ("pad front edge, left end", pw(-PADW / 2, PFRONT, PZ)),
    ("pad front edge, right end", pw(PADW / 2, PFRONT, PZ)),
    ("standing line, left end", pw(-PADW / 2, STAND, PZ)),
    ("standing line, right end", pw(PADW / 2, STAND, PZ)),
    ("left foot mark", pw(-FEET, STAND, PZ)),
    ("right foot mark", pw(FEET, STAND, PZ)),
    ("pad bottom (ground), left end", pw(-PADW / 2, PFRONT, 0.0)),
    ("pad bottom (ground), right end", pw(PADW / 2, PFRONT, 0.0)),
])

if not args.no_neon_strip and frames > 1:
    kit.render_crop(args.name + "_neon", [sign_board, fuel], frames, set_frame)
    print("fuel_station: neon %d frames; in the editor set Animate Period to %.2f s" % (frames, frames * 0.2))
