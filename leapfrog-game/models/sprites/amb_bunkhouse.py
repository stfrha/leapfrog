"""
amb_bunkhouse - ambient piece: a two-storey workers' bunkhouse for the
village on the 0.4 parallax layer.

A long concrete block with a row of rooms on each floor, an outside walkway
along the upper floor and a steel stair up the right side (the side the
camera sees). Some windows lit, most of the building asleep. Air
conditioners and a small water tank on the roof.

Like amb_hut: 14 px/unit, built at natural size and scaled x2, orthographic
3/4 view (render_axo). Turned 8 degrees by default, so it doesn't stand
lined up with its neighbours (--turn to change it).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_bunkhouse.py -- --samples 16 --name amb_bunkhouse_test
    blender -b --factory-startup -P amb_bunkhouse.py
"""

import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_bunkhouse", help="texture name (default amb_bunkhouse)")
    p.add_argument("--width", type=float, default=15.0, help="length of the block (default 15)")
    p.add_argument("--rooms", type=int, default=5, help="rooms (windows) per floor on the front (default 5)")
    p.add_argument("--lit", type=float, default=0.4, help="share of lit windows, 0..1 (default 0.4)")
    p.add_argument("--dark", action="store_true", help="no lit windows")
    p.add_argument("--seed", type=int, default=3, help="which windows are lit")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=8.0)
M = kit.material
rnd = random.Random(args.seed)
lit = 0.0 if args.dark else args.lit

W = args.width
D = 6.0
FLOOR = 3.0
P = ap.plinth(M, W + 0.8, D + 0.8, 0.5)
FRONT = -D / 2
RIGHT = W / 2
WALK = 1.3                                   # walkway depth

# Block: two floors with a band between them
sk.box_object("block", (0.0, 0.0, P + FLOOR), (W, D, 2 * FLOOR), M("concrete_dirty"), bevel_width=0.06)
sk.box_object("band", (0.0, 0.0, P + FLOOR), (W + 0.1, D + 0.1, 0.25), M("frame_brace"))
sk.box_object("roof", (0.0, 0.0, P + 2 * FLOOR + 0.15), (W + 0.4, D + 0.4, 0.3), M("frame_brace"))

# Ground floor: door and windows; upper floor: doors open onto the walkway
door_x = -W / 2 + 1.6
ap.door("door_0", M, (door_x, FRONT, P))
ap.window_row("win_0", M, door_x + 1.4, RIGHT - 0.6, FRONT, P + 1.7, args.rooms - 1, rnd=rnd, lit_share=lit)
for i in range(args.rooms):
    x = -W / 2 + W * (i + 0.5) / args.rooms
    ap.door("door_1_%d" % i, M, (x - 0.9, FRONT, P + FLOOR + 0.12), (0.9, 2.0), frame_role="frame_main")
    ap.window("win_1_%d" % i, M, (x + 0.6, FRONT, P + FLOOR + 1.6), (0.9, 0.7), lit=rnd.random() < lit)
ap.side_window_row("side_0", M, FRONT + 1.0, D / 2 - 1.0, RIGHT, P + 1.7, 2, rnd=rnd, lit_share=lit)
ap.side_window_row("side_1", M, FRONT + 1.0, D / 2 - 1.0, RIGHT, P + FLOOR + 1.6, 2, rnd=rnd, lit_share=lit)

# Upper walkway along the front, on posts
wz = P + FLOOR
sk.box_object("walkway", (0.0, FRONT - WALK / 2, wz), (W, WALK, 0.15), M("pad_metal"))
posts = bmesh.new()
for i in range(5):
    x = -W / 2 + 0.3 + (W - 0.6) * i / 4
    sk.strut(posts, (x, FRONT - WALK + 0.15, 0.0), (x, FRONT - WALK + 0.15, wz), 0.1, 6)
sk.mesh_object("walkway_posts", posts, M("frame_main"))
ap.railing("walk_rail", M, (-W / 2, FRONT - WALK + 0.05, wz), (W / 2 + 1.3, FRONT - WALK + 0.05, wz))

# Stair up the right side, from the back to a landing at the front right corner
STEPS = 12
sx = RIGHT + 0.7
y0 = D / 2 - 0.2                             # foot of the stair (back)
y1 = FRONT + 0.4                             # top of the stair (front)
steps = bmesh.new()
for i in range(STEPS):
    t = (i + 0.5) / STEPS
    sk.box(steps, (sx, y0 + (y1 - y0) * t, wz * t), (1.1, abs(y1 - y0) / STEPS + 0.05, 0.08))
sk.mesh_object("stair_steps", steps, M("pad_metal"))
stringers = bmesh.new()
for dx in (-0.58, 0.58):
    sk.strut(stringers, (sx + dx, y0, 0.0), (sx + dx, y1, wz), 0.07, 6)
sk.mesh_object("stair_stringers", stringers, M("frame_main"))
sk.box_object("landing", (sx, FRONT - WALK / 2 + 0.2, wz), (1.3, WALK + 0.8, 0.15), M("pad_metal"))
ap.railing("stair_rail", M, (sx + 0.6, y0, 0.0), (sx + 0.6, y1, wz), post_every=2.0)

# Roof clutter
rz = P + 2 * FLOOR + 0.3
sk.box_object("ac_0", (-W / 4, 0.8, rz + 0.4), (1.2, 1.0, 0.8), M("frame_brace"), bevel_width=0.04)
sk.box_object("ac_1", (W / 8, 1.2, rz + 0.4), (1.2, 1.0, 0.8), M("frame_brace"), bevel_width=0.04)
ap.cylinder("roof_tank", M, "rusty_metal", (W / 2 - 1.8, 0.8, rz), 0.9, 1.6, cone=0.3)

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
