"""
amb_haul_truck - ambient piece for a distant mine site: a rigid mining dump
truck, the 0.4 parallax layer.

Huge tyres (single at the front, twin at the back), the chassis, the upper
deck with the radiator grille and the cab on the left, a ladder up the
front, and the dump body with its canopy reaching over the cab. By default
it is parked loaded with ore and dead (no lights). The truck faces right.

Like the village pieces: 14 px/unit, built at natural size and scaled x2,
orthographic 3/4 view (render_axo). Turned 14 degrees by default (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_haul_truck.py -- --samples 16 --name amb_haul_truck_test
    blender -b --factory-startup -P amb_haul_truck.py
Variants (as their own textures):
    ... -P amb_haul_truck.py -- --name amb_haul_truck_tipping --tip 35
    ... -P amb_haul_truck.py -- --name amb_haul_truck_empty --empty --lit
    ... -P amb_haul_truck.py -- --name amb_haul_truck_wreck --rusty --empty --flat
For a more distant layer (0.6), render with --ppu 10.
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bpy                        # noqa: E402
import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_mine_parts as mp       # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_haul_truck", help="texture name (default amb_haul_truck)")
    p.add_argument("--tip", type=float, default=0.0, help="tip the body up this many degrees (default 0)")
    p.add_argument("--empty", action="store_true", help="no load of ore in the body")
    p.add_argument("--lit", action="store_true", help="lit cab, head lights and beacon (default: dead)")
    p.add_argument("--rusty", action="store_true", help="rust instead of yellow paint (a wreck)")
    p.add_argument("--flat", action="store_true", help="a flat front tyre: the truck sags at the front left")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=14.0)
M = kit.material
PAINT = "rusty_metal" if args.rusty else "machine_yellow"

R = 1.5                 # tyre radius
TW = 1.0                # tyre width
FRONT_X = 3.0
REAR_X = -2.7
TRACK_Y = 2.3           # outer tyres at y = +-TRACK_Y

# Tyres --------------------------------------------------------------------------
front_r = R * (0.86 if args.flat else 1.0)
mp.wheel("tyre_fl", M, (FRONT_X, -TRACK_Y, front_r), front_r, TW, rim_role=PAINT)
mp.wheel("tyre_fr", M, (FRONT_X, TRACK_Y, R), R, TW, rim_role=PAINT)
for side in (-1, 1):
    for k, dy in enumerate((0.0, 1.06)):
        y = side * (TRACK_Y - dy)
        mp.wheel("tyre_r%s%d" % ("l" if side < 0 else "r", k), M, (REAR_X, y, R), R, TW, rim_role=PAINT)

# Chassis, axles, deck, grille ------------------------------------------------------
sk.box_object("chassis", (0.2, 0.0, 1.75), (8.4, 1.8, 0.9), M(PAINT), bevel_width=0.1)
axles = bmesh.new()
sk.strut(axles, (FRONT_X, -TRACK_Y + 0.4, R), (FRONT_X, TRACK_Y - 0.4, R), 0.25, 12)
sk.strut(axles, (REAR_X, -TRACK_Y + 1.5, R), (REAR_X, TRACK_Y - 1.5, R), 0.45, 16)
sk.mesh_object("axles", axles, M("frame_main"), smooth_quads=True)
# Front wheel arches / fenders over the front tyres
for side in (-1, 1):
    sk.box_object("fender_%d" % (side > 0), (FRONT_X - 0.3, side * TRACK_Y, 3.08), (3.2, 1.3, 0.16), M(PAINT),
                  bevel_width=0.05)

DECK_Z = 3.2
sk.box_object("deck", (3.7, 0.0, DECK_Z), (2.3, 5.4, 0.25), M(PAINT), bevel_width=0.04)
sk.box_object("engine", (4.0, 0.0, 2.35), (1.7, 2.4, 1.5), M("frame_main"), bevel_width=0.08)
sk.box_object("grille", (4.9, 0.0, 2.45), (0.3, 2.5, 1.4), M("frame_main"))
slats = bmesh.new()
for i in range(8):
    z = 1.9 + i * 0.16
    sk.box(slats, (5.07, 0.0, z), (0.06, 2.3, 0.06))
sk.mesh_object("grille_slats", slats, M("frame_brace"))
sk.box_object("bumper", (5.15, 0.0, 1.55), (0.4, 4.0, 0.45), M(PAINT), bevel_width=0.06)

# Head lights on the bumper ends
for side in (-1, 1):
    sk.box_object("headlight_%d" % (side > 0), (5.37, side * 1.6, 1.6), (0.06, 0.4, 0.25),
                  M("window_lit_warm") if args.lit else M("glass_dark"))

# Cab on the deck, left side (towards the camera)
CAB = (1.7, 1.6, 1.6)
cab_center = (3.5, -1.7, DECK_Z + 0.12 + CAB[2] / 2)
mp.cab("cab", M, cab_center, CAB, role=PAINT, lit=args.lit)
mp.beacon("beacon", M, (cab_center[0] - 0.4, cab_center[1], cab_center[2] + CAB[2] / 2 + 0.28), lit=args.lit)

# Ladder up the front, diagonally across the grille
lad = bmesh.new()
for dy in (-0.32, 0.32):
    sk.strut(lad, (5.4, -1.0 + dy, 0.5), (4.7, -2.3 + dy, DECK_Z), 0.04, 6)
for i in range(1, 8):
    t = i / 8
    x = 5.4 + (4.7 - 5.4) * t
    y = -1.0 + (-2.3 + 1.0) * t
    z = 0.5 + (DECK_Z - 0.5) * t
    sk.strut(lad, (x, y - 0.32, z), (x, y + 0.32, z), 0.03, 6)
sk.mesh_object("ladder", lad, M("frame_brace"))
rail = bmesh.new()
sk.strut(rail, (2.6, -2.65, DECK_Z), (2.6, -2.65, DECK_Z + 1.0), 0.04, 6)
sk.strut(rail, (4.8, -2.65, DECK_Z), (4.8, -2.65, DECK_Z + 1.0), 0.04, 6)
sk.strut(rail, (2.6, -2.65, DECK_Z + 1.0), (4.8, -2.65, DECK_Z + 1.0), 0.05, 6)
sk.mesh_object("deck_rail", rail, M("frame_brace"))

# Dump body ---------------------------------------------------------------------
# Built level, hinged at the back (HINGE); --tip turns it up about the hinge.
HINGE = (-5.0, 3.2)
BODY_L = 7.6            # from the hinge forward to the front wall
BODY_W = 5.4
FLOOR_Z = 3.35          # above the rear tyres (their tops are at 2 R = 3.0)
parts = []
parts.append(sk.box_object("body_floor", (HINGE[0] + BODY_L / 2, 0.0, FLOOR_Z), (BODY_L, BODY_W, 0.3), M(PAINT),
                           bevel_width=0.05))
for side in (-1, 1):
    parts.append(sk.box_object("body_side_%d" % (side > 0), (HINGE[0] + BODY_L / 2 + 0.1, side * BODY_W / 2, FLOOR_Z + 1.0),
                               (BODY_L - 0.2, 0.25, 2.0), M(PAINT), bevel_width=0.05))
    # Stiffening ribs on the outside of the side walls
    ribs = bmesh.new()
    for i in range(5):
        x = HINGE[0] + 0.8 + i * (BODY_L - 1.6) / 4
        sk.box(ribs, (x, side * (BODY_W / 2 + 0.15), FLOOR_Z + 1.0), (0.22, 0.12, 2.0))
    parts.append(sk.mesh_object("body_ribs_%d" % (side > 0), ribs, M(PAINT)))
front_x = HINGE[0] + BODY_L
parts.append(sk.box_object("body_front", (front_x, 0.0, FLOOR_Z + 1.35), (0.3, BODY_W, 2.7), M(PAINT),
                           bevel_width=0.05))
# Canopy over the cab, sloping up a little at the front
parts.append(sk.box_object("canopy", (front_x + 1.2, 0.0, FLOOR_Z + 2.75), (2.6, BODY_W + 0.2, 0.2), M(PAINT),
                           rotation=(0.0, -0.06, 0.0), bevel_width=0.04))
# Rock lip at the back of the floor, sloping up
parts.append(mp.beam("tail_lip", M, PAINT, (HINGE[0], 0.0, FLOOR_Z), (HINGE[0] - 0.9, 0.0, FLOOR_Z + 0.6), 0.25, BODY_W))
if not args.empty and args.tip < 20.0:
    parts.append(mp.heap("load", M, (HINGE[0] + BODY_L / 2 + 0.2, 0.0, FLOOR_Z + 0.15), 2.4, 1.9, seed=7,
                         scale=(1.45, 1.0, 1.0)))

if args.tip:
    hinge = bpy.data.objects.new("body_hinge", None)
    kit.scene.collection.objects.link(hinge)
    hinge.location = (HINGE[0], 0.0, HINGE[1])
    sk.parent_keep_transform(parts, hinge)
    hinge.rotation_euler = (0.0, -math.radians(args.tip), 0.0)
    # The hoist rams from the chassis up to the body
    bpy.context.view_layer.update()
    for side in (-1, 1):
        y = side * 1.0
        # Body point the rams push on, turned with the body
        bx, bz = -0.6, FLOOR_Z - 0.15
        a = math.radians(args.tip)
        rx = HINGE[0] + (bx - HINGE[0]) * math.cos(a) - (bz - HINGE[1]) * math.sin(a)
        rz = HINGE[1] + (bx - HINGE[0]) * math.sin(a) + (bz - HINGE[1]) * math.cos(a)
        mp.hydraulic("hoist_%d" % (side > 0), M, (-0.2, y, 1.9), (rx, y, rz), 0.18)

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
