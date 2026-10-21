"""
amb_excavator - ambient piece for a distant mine site: a big tracked
hydraulic excavator, the 0.4 parallax layer.

Two crawler tracks, a turntable, the upper house with the engine and a
heavy counterweight at the back (left), the cab front left, and the boom,
stick and bucket reaching out to the right with their hydraulic rams. By
default it is dead: unlit cab, dead beacon, the bucket resting on the
ground in front of it, as if it was left mid-shift when the mine closed.

Like the village pieces: 14 px/unit, built at natural size and scaled x2,
orthographic 3/4 view (render_axo), the same camera for every piece.
Turned -12 degrees by default so it isn't square to the camera (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_excavator.py -- --samples 16 --name amb_excavator_test
    blender -b --factory-startup -P amb_excavator.py
Variants (as their own textures):
    ... -P amb_excavator.py -- --name amb_excavator_working --lit --lift 25
    ... -P amb_excavator.py -- --name amb_excavator_wreck --rusty --lift -4
For a more distant layer (0.6), render with --ppu 10.
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_mine_parts as mp       # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_excavator", help="texture name (default amb_excavator)")
    p.add_argument("--lift", type=float, default=0.0,
                   help="raise the whole arm this many degrees about the boom foot (default 0: bucket on the ground)")
    p.add_argument("--curl", type=float, default=0.0, help="turn the bucket this many degrees (+ = curled up)")
    p.add_argument("--lit", action="store_true", help="lit cab and beacon (default: dead)")
    p.add_argument("--rusty", action="store_true", help="rust instead of yellow paint (a wreck)")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=-12.0)
M = kit.material
PAINT = "rusty_metal" if args.rusty else "machine_yellow"

# Undercarriage ---------------------------------------------------------------
TL, TH, TW = 5.6, 1.1, 0.8          # track length, height, width
TY = 1.3                            # track centres at y = +-TY
for side, y in (("front", -TY), ("back", TY)):
    mp.track("track_" + side, M, "frame_main", 0.0, y, TL, TH, TW)
sk.box_object("carbody", (0.0, 0.0, 0.85), (2.2, 2.0, 0.6), M("frame_main"), bevel_width=0.06)

# Turntable and upper house ------------------------------------------------------
Z_DECK = 1.35                       # top of the turntable, bottom of the upper house
ring = bmesh.new()
sk.lathe(ring, [(1.15, 1.15), (1.15, Z_DECK)], 32, cap_top=True, cap_bottom=True)
sk.mesh_object("turntable", ring, M("frame_main"))

HOUSE_H = 1.6
sk.box_object("house", (-0.9, 0.0, Z_DECK + HOUSE_H / 2), (2.8, 2.6, HOUSE_H), M(PAINT), bevel_width=0.08)
sk.box_object("counterweight", (-2.7, 0.0, Z_DECK + 0.65), (0.9, 2.7, 1.3), M(PAINT), bevel_width=0.25)
# Engine grille on the side the camera sees, and the walkway rail on top
sk.box_object("grille", (-1.4, -1.33, Z_DECK + 0.9), (1.4, 0.06, 0.8), M("frame_main"))
grille = bmesh.new()
for i in range(7):
    x = -2.05 + i * 0.22
    sk.box(grille, (x, -1.37, Z_DECK + 0.9), (0.06, 0.04, 0.7))
sk.mesh_object("grille_slats", grille, M("frame_brace"))
rail = bmesh.new()
for x in (-2.2, -1.2, -0.2):
    sk.strut(rail, (x, -1.2, Z_DECK + HOUSE_H), (x, -1.2, Z_DECK + HOUSE_H + 0.8), 0.035, 6)
sk.strut(rail, (-2.2, -1.2, Z_DECK + HOUSE_H + 0.8), (-0.2, -1.2, Z_DECK + HOUSE_H + 0.8), 0.04, 6)
sk.mesh_object("rail", rail, M("frame_brace"))
# Exhaust stack
stack = bmesh.new()
sk.strut(stack, (-1.0, 0.6, Z_DECK + HOUSE_H), (-1.0, 0.6, Z_DECK + HOUSE_H + 0.9), 0.1, 10)
sk.mesh_object("exhaust", stack, M("rusty_metal"))

# Cab front left, on the deck beside the boom ------------------------------------------
CAB = (1.3, 1.1, 1.8)
cab_center = (1.15, -0.72, Z_DECK + CAB[2] / 2)
mp.cab("cab", M, cab_center, CAB, role=PAINT, lit=args.lit)
mp.beacon("beacon", M, (cab_center[0], cab_center[1], Z_DECK + CAB[2] + 0.28), lit=args.lit)

# Boom foot block, right of the cab
BOOM_Y = 0.35
sk.box_object("boom_foot", (1.05, BOOM_Y, Z_DECK + 0.5), (1.3, 1.0, 1.0), M(PAINT), bevel_width=0.08)

# Arm: boom, stick and bucket ----------------------------------------------------------
# Points in the X/Z plane for the default pose (bucket on the ground in
# front of the tracks); --lift turns them all about the boom foot pin,
# --curl turns the bucket about the stick tip.
FOOT = (1.2, Z_DECK + 0.75)
KNEE = (3.5, 5.6)
ELBOW = (5.6, 5.3)
STICK_TOP = (5.4, 6.0)
TIP = (6.9, 1.35)


def turn2d(p, about, deg):
    a = math.radians(deg)
    x, z = p[0] - about[0], p[1] - about[1]
    return (about[0] + x * math.cos(a) - z * math.sin(a), about[1] + x * math.sin(a) + z * math.cos(a))


def P(p, y=BOOM_Y):
    q = turn2d(p, FOOT, args.lift)
    return (q[0], y, q[1])


# Boom in two pieces with a kink, the stick, and a cross pin at each joint
mp.beam("boom_1", M, PAINT, P(FOOT), P(KNEE), 0.75, 0.6)
mp.beam("boom_2", M, PAINT, P(KNEE), P(ELBOW), 0.65, 0.6)
mp.beam("stick", M, PAINT, P(STICK_TOP), P(TIP), 0.5, 0.45)
pins = bmesh.new()
for pt in (FOOT, KNEE, ELBOW, TIP):
    x, _y, z = P(pt)
    sk.strut(pins, (x, BOOM_Y - 0.42, z), (x, BOOM_Y + 0.42, z), 0.13, 10)
sk.mesh_object("pins", pins, M("frame_main"))

# Hydraulic rams: two boom rams from the house front, the stick ram on top
# of the boom, the bucket ram down the stick
for dy in (-0.32, 0.32):
    mp.hydraulic("boom_ram_%d" % (dy > 0), M, P((1.75, Z_DECK + 0.35), BOOM_Y + dy), P((2.75, 4.35), BOOM_Y + dy), 0.16)
mp.hydraulic("stick_ram", M, P((3.0, 6.15)), P(STICK_TOP), 0.14)
mp.hydraulic("bucket_ram", M, P((5.75, 5.0), BOOM_Y - 0.3), P((6.55, 2.35), BOOM_Y - 0.3), 0.11)

# Bucket at the stick tip: back plate, floor, side plates and teeth.
# Built in its own frame about the tip, opening to the left (towards the
# machine), then turned by --curl and --lift.

def B(bx, bz, y=BOOM_Y):
    q = turn2d((TIP[0] + bx, TIP[1] + bz), TIP, args.curl)
    q = turn2d(q, FOOT, args.lift)
    return (q[0], y, q[1])


BW = 1.5                            # bucket width (Y)
mp.beam("bucket_back", M, PAINT, B(0.15, 0.0), B(0.55, -1.15), 0.18, BW)
mp.beam("bucket_floor", M, PAINT, B(0.55, -1.15), B(-0.75, -1.2), 0.18, BW)
for side in (-1, 1):
    mp.beam("bucket_side_%d" % (side > 0), M, PAINT, B(0.35, -0.55, BOOM_Y + side * BW / 2),
            B(-0.45, -0.95, BOOM_Y + side * BW / 2), 0.95, 0.08)
teeth = bmesh.new()
for i in range(5):
    y = BOOM_Y - BW / 2 + BW * (i + 0.5) / 5
    a = B(-0.75, -1.2, y)
    b = B(-1.05, -1.2, y)
    sk.strut(teeth, a, b, 0.07, 6)
sk.mesh_object("teeth", teeth, M("frame_main"))

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
