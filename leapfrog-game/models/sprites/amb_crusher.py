"""
amb_crusher - ambient piece for a distant mine site: the primary crusher
with its conveyor and ore stockpile, the 0.4 parallax layer.

On the left a braced steel tower on a concrete pad, with the crusher house
inside it and a big square hopper on top. A conveyor gallery on two trestles
climbs from the crusher to the right and ends over a tall cone of ore. A
small control shed at the tower's foot has the only light (--dark turns it
off). Rusty and partly derelict: the "strange machinery" of the mine.

Like the village pieces: 14 px/unit, built at natural size and scaled x2,
orthographic 3/4 view (render_axo). Turned 8 degrees by default (--turn).
At about 19 x 10 units (38 x 20 after scaling) it is the widest mine piece.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_crusher.py -- --samples 16 --name amb_crusher_test
    blender -b --factory-startup -P amb_crusher.py
Variants (as their own textures):
    ... -P amb_crusher.py -- --name amb_crusher_small --no-pile
For a more distant layer (0.6), render with --ppu 10.
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402
import amb_mine_parts as mp       # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_crusher", help="texture name (default amb_crusher)")
    p.add_argument("--dark", action="store_true", help="no lit window in the control shed")
    p.add_argument("--no-pile", action="store_true", help="leave out the stockpile (and end the conveyor shorter)")
    p.add_argument("--empty", action="store_true", help="no ore heaped in the hopper")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=8.0)
M = kit.material

TX = -7.0               # tower centre
TS = 1.8                # half spread of the tower legs
PAD_Z = 0.4
DECK_Z = 7.0

# Pad and tower -------------------------------------------------------------------
sk.box_object("pad", (TX, 0.0, PAD_Z / 2), (2 * TS + 2.6, 2 * TS + 2.4, PAD_Z), M("concrete_dirty"), bevel_width=0.08)
corners = [(-TS, -TS), (TS, -TS), (TS, TS), (-TS, TS)]
legs = bmesh.new()
for cx, cy in corners:
    sk.strut(legs, (TX + cx, cy, PAD_Z), (TX + cx, cy, DECK_Z), 0.16, 8)
sk.mesh_object("tower_legs", legs, M("frame_main"))
brace = bmesh.new()
LEVELS = 3
for k in range(LEVELS):
    z0 = PAD_Z + (DECK_Z - PAD_Z) * k / LEVELS
    z1 = PAD_Z + (DECK_Z - PAD_Z) * (k + 1) / LEVELS
    for i in range(4):
        (ax, ay), (bx, by) = corners[i], corners[(i + 1) % 4]
        sk.strut(brace, (TX + ax, ay, z0), (TX + bx, by, z1), 0.06, 6)
        sk.strut(brace, (TX + bx, by, z0), (TX + ax, ay, z1), 0.06, 6)
        sk.strut(brace, (TX + ax, ay, z1), (TX + bx, by, z1), 0.07, 6)
sk.mesh_object("tower_bracing", brace, M("frame_brace"))

# Crusher house inside the tower, with a chute down to the conveyor
ap.corrugated_box("crusher_house", M, (TX, 0.0, 3.6), (3.0, 3.0, 2.8), "rusty_metal")
sk.box_object("chute", (TX + 1.5, 0.0, 1.7), (1.2, 1.2, 1.2), M("pad_metal"), rotation=(0.0, -0.5, 0.0),
              bevel_width=0.04)

# Deck, railing and hopper on top -----------------------------------------------------
sk.box_object("deck", (TX, 0.0, DECK_Z), (2 * TS + 0.8, 2 * TS + 0.8, 0.25), M("pad_metal"))
ap.railing("deck_rail_front", M, (TX - TS - 0.3, -TS - 0.35, DECK_Z + 0.12), (TX + TS + 0.3, -TS - 0.35, DECK_Z + 0.12))
ap.railing("deck_rail_right", M, (TX + TS + 0.35, -TS - 0.3, DECK_Z + 0.12), (TX + TS + 0.35, TS + 0.3, DECK_Z + 0.12))

# Square hopper: a 4-sided lathe (its corners point front/right/back/left),
# turned 45 degrees so its faces face the camera
HOP_TOP = DECK_Z + 2.7
hop = bmesh.new()
sk.lathe(hop, [(1.0, DECK_Z + 0.15), (3.4, HOP_TOP)], 4, cap_bottom=True)
ob = sk.mesh_object("hopper", hop, M("rusty_metal"))
ob.rotation_euler = (0.0, 0.0, math.pi / 4)
ob.location = (TX, 0.0, 0.0)
band = bmesh.new()
sk.lathe(band, [(3.0, HOP_TOP - 0.55), (3.45, HOP_TOP + 0.02)], 4)
ob = sk.mesh_object("hopper_band", band, M("hazard_paint"))
ob.rotation_euler = (0.0, 0.0, math.pi / 4)
ob.location = (TX, 0.0, 0.0)
if not args.empty:
    mp.heap("hopper_ore", M, (TX, 0.0, HOP_TOP - 0.9), 2.1, 1.4, seed=3, rough=0.18)

# Conveyor ---------------------------------------------------------------------
TAIL = (TX + 2.4, 0.0, 1.3)
HEAD = (4.2, 0.0, 9.2) if not args.no_pile else (0.5, 0.0, 6.0)
mp.truss("conveyor", M, TAIL, HEAD, 1.5, 0.9)
length = math.hypot(HEAD[0] - TAIL[0], HEAD[2] - TAIL[2])
ux, uz = (HEAD[0] - TAIL[0]) / length, (HEAD[2] - TAIL[2]) / length
nx, nz = -uz, ux
belt0 = (TAIL[0] + nx * 0.95, 0.0, TAIL[2] + nz * 0.95)
belt1 = (HEAD[0] + nx * 0.95, 0.0, HEAD[2] + nz * 0.95)
mp.beam("belt", M, "rubber", belt0, belt1, 0.12, 1.2, bevel=0.0)
# A line of ore on the belt
mp.beam("belt_ore", M, "ore_rock", (belt0[0] + ux * 0.6, 0.0, belt0[2] + uz * 0.6 + 0.12),
        (belt1[0] - ux * 0.8, 0.0, belt1[2] - uz * 0.8 + 0.12), 0.2, 0.7, bevel=0.06)
# Head end: drive house over the pulley, discharge chute
sk.box_object("head_house", (HEAD[0] + 0.3, 0.0, HEAD[2] + 1.2), (1.8, 2.0, 1.5), M("rusty_metal"),
              bevel_width=0.05)
sk.box_object("head_chute", (HEAD[0] + 0.8, 0.0, HEAD[2] - 0.2), (0.9, 1.3, 1.0), M("pad_metal"),
              rotation=(0.0, 0.35, 0.0))
# Tail pulley housing at the bottom
sk.box_object("tail_house", (TAIL[0] - 0.2, 0.0, TAIL[2] + 0.5), (1.4, 1.9, 1.2), M("frame_main"), bevel_width=0.05)

# Trestles under the gallery
for t in (0.36, 0.72):
    x = TAIL[0] + (HEAD[0] - TAIL[0]) * t
    z = TAIL[2] + (HEAD[2] - TAIL[2]) * t
    mp.trestle("trestle_%d" % int(t * 100), M, (x, 0.0, 0.0), z, spread=1.6 + 2.0 * t, width=1.5)

# Stockpile under the head end ---------------------------------------------------------
if not args.no_pile:
    mp.heap("stockpile", M, (HEAD[0] + 1.3, 0.3, 0.0), 4.6, HEAD[2] - 1.1, seed=11, rough=0.1)

# Control shed at the tower's foot, the one light ------------------------------------------
shed_x, shed_y = TX + 3.4, -2.6
ap.corrugated_box("shed", M, (shed_x, shed_y, 1.35), (2.6, 2.0, 2.3), "container_blue")
sk.box_object("shed_roof", (shed_x, shed_y, 2.6), (2.9, 2.3, 0.2), M("rusty_metal"))
ap.window("shed_win", M, (shed_x + 0.3, shed_y - 1.06, 1.6), (1.1, 0.7), lit=not args.dark)
ap.door("shed_door", M, (shed_x - 0.8, shed_y - 1.06, 0.2), (0.8, 1.9), frame_role="frame_main")

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
