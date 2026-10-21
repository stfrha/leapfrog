"""
amb_headframe - ambient piece for a distant mine site: the headframe over
the mine shaft, with turning sheave wheels, the 0.4 parallax layer.

A braced steel tower over the shaft opening in a concrete collar, two
sheave wheels on top, the inclined back legs braced back to the tower, and
the winding house tucked under the back legs with the hoist ropes running
from its roof up over the wheels and down into the shaft. The beacon on
top and a window in the winding house are lit (--dark turns them off): the
lift still runs, whatever else at the mine has stopped.

Animation: the wheels turn (Kit.render_axo_animated: frame 0 with
render_axo, the others with render_variant, then make_strip registers the
image with cols="N"). In the editor set the sprite box's Animate Period to
what the script prints. --still renders one image without animation.

Like the village pieces: 14 px/unit, built at natural size and scaled x2,
orthographic 3/4 view (render_axo). Turned 25 degrees by default so the
two wheels don't hide each other (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_headframe.py -- --samples 16 --name amb_headframe_test --still
    blender -b --factory-startup -P amb_headframe.py
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
    p.add_argument("--name", default="amb_headframe", help="texture name (default amb_headframe)")
    p.add_argument("--height", type=float, default=13.0, help="height of the tower top (default 13)")
    p.add_argument("--frames", type=int, default=4, help="animation frames (default 4)")
    p.add_argument("--spokes", type=int, default=6, help="spokes per wheel (default 6)")
    p.add_argument("--dark", action="store_true", help="beacon and window off")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=25.0)
M = kit.material

H = args.height
LX, LY = 2.0, 1.4        # half spread of the tower legs
COLLAR = 0.5             # top of the concrete collar
R = 1.6                  # sheave radius
WHEEL_Z = H + 1.15       # sheave centres
WHEEL_Y = 0.75           # sheaves at y = +-WHEEL_Y
BACK_FOOT_X = -8.2       # back legs reach the ground here

# Collar and shaft ---------------------------------------------------------------------
sk.box_object("collar", (0.0, 0.0, COLLAR / 2), (2 * LX + 2.6, 2 * LY + 2.4, COLLAR), M("concrete_dirty"),
              bevel_width=0.08)
sk.box_object("shaft", (0.0, 0.0, COLLAR + 0.01), (2 * LX - 0.6, 2 * LY - 0.4, 0.04), M("void"))

# Tower ------------------------------------------------------------------------------
corners = [(-LX, -LY), (LX, -LY), (LX, LY), (-LX, LY)]
legs = bmesh.new()
for cx, cy in corners:
    sk.strut(legs, (cx, cy, COLLAR), (cx, cy, H), 0.17, 8)
sk.mesh_object("tower_legs", legs, M("frame_main"))
brace = bmesh.new()
LEVELS = 4
for k in range(LEVELS):
    z0 = COLLAR + (H - COLLAR) * k / LEVELS
    z1 = COLLAR + (H - COLLAR) * (k + 1) / LEVELS
    for i in range(4):
        (ax, ay), (bx, by) = corners[i], corners[(i + 1) % 4]
        sk.strut(brace, (ax, ay, z0), (bx, by, z1), 0.06, 6)
        sk.strut(brace, (bx, by, z0), (ax, ay, z1), 0.06, 6)
        sk.strut(brace, (ax, ay, z1), (bx, by, z1), 0.08, 6)
sk.mesh_object("tower_bracing", brace, M("frame_brace"))

# Back legs from the top back down to the ground on the left, tied to the
# tower at each bracing level
back = bmesh.new()
ties = bmesh.new()
for cy in (-LY, LY):
    top = (-LX, cy, H)
    foot = (BACK_FOOT_X, cy * 1.15, COLLAR)
    sk.strut(back, top, foot, 0.2, 8)
    for k in range(1, LEVELS):
        z = COLLAR + (H - COLLAR) * k / LEVELS
        t = (H - z) / (H - COLLAR)
        px = top[0] + (foot[0] - top[0]) * t
        py = top[1] + (foot[1] - top[1]) * t
        sk.strut(ties, (px, py, z), (-LX, cy, z), 0.07, 6)
        if k < LEVELS - 1:
            z2 = COLLAR + (H - COLLAR) * (k + 1) / LEVELS
            sk.strut(ties, (px, py, z), (-LX, cy, z2), 0.05, 6)
sk.strut(back, (BACK_FOOT_X, -LY * 1.15, COLLAR + 0.3), (BACK_FOOT_X, LY * 1.15, COLLAR + 0.3), 0.1, 6)
sk.mesh_object("back_legs", back, M("frame_main"))
sk.mesh_object("back_ties", ties, M("frame_brace"))
for cy in (-LY, LY):
    sk.box_object("back_footing_%d" % (cy > 0), (BACK_FOOT_X, cy * 1.15, COLLAR / 2), (1.0, 1.0, COLLAR),
                  M("concrete_dirty"))

# Top deck with the bearing blocks, a railing and the beacon -------------------------------------
sk.box_object("top_deck", (0.0, 0.0, H + 0.12), (2 * LX + 0.8, 2 * LY + 0.8, 0.25), M("pad_metal"))
for wy in (-WHEEL_Y, WHEEL_Y):
    for dy in (-0.32, 0.32):
        sk.box_object("bearing_%g_%g" % (wy, dy), (0.0, wy + dy, (H + WHEEL_Z) / 2 + 0.1),
                      (0.5, 0.14, WHEEL_Z - H), M("frame_main"))
ap.railing("top_rail_front", M, (-LX - 0.3, -LY - 0.35, H + 0.25), (LX + 0.3, -LY - 0.35, H + 0.25))
mp.beacon("beacon", M, (LX + 0.1, -LY - 0.1, H + 1.4), lit=not args.dark)
mast = bmesh.new()
sk.strut(mast, (LX + 0.1, -LY - 0.1, H + 0.25), (LX + 0.1, -LY - 0.1, H + 1.2), 0.05, 6)
sk.mesh_object("beacon_mast", mast, M("frame_main"))

# Sheave wheels (turned for the animation frames)
wheels = [mp.sheave("sheave_%d" % i, M, (0.0, wy, WHEEL_Z), R, spokes=args.spokes)
          for i, wy in enumerate((-WHEEL_Y, WHEEL_Y))]

# Winding house under the back legs ---------------------------------------------------------
WH_X = -4.6
WH = (3.4, 4.0, 3.6)
ap.corrugated_box("winding_house", M, (WH_X, 0.0, WH[2] / 2), WH, "frame_brace")
sk.box_object("winding_roof", (WH_X, 0.0, WH[2] + 0.12), (WH[0] + 0.4, WH[1] + 0.4, 0.25), M("rusty_metal"))
ap.window("winding_win", M, (WH_X + 0.6, -WH[1] / 2 - 0.06, 2.2), (1.2, 0.8), lit=not args.dark)
ap.door("winding_door", M, (WH_X - 0.9, -WH[1] / 2 - 0.06, 0.0), (1.0, 2.2))
# Rope exits on the roof
for wy in (-WHEEL_Y, WHEEL_Y):
    sk.box_object("rope_exit_%d" % (wy > 0), (WH_X + 1.0, wy, WH[2] + 0.4), (0.6, 0.5, 0.5), M("frame_main"))

# Hoist ropes: from the winding house up to the top of each wheel (left
# side), and from the right side of each wheel straight down the shaft
ropes = bmesh.new()
for wy in (-WHEEL_Y, WHEEL_Y):
    a = math.radians(120.0)                   # leave the wheel up on its left
    sk.strut(ropes, (WH_X + 1.0, wy, WH[2] + 0.6), (R * math.cos(a), wy, WHEEL_Z + R * math.sin(a)), 0.06, 6)
    sk.strut(ropes, (R, wy, WHEEL_Z), (R, wy, COLLAR), 0.06, 6)
sk.mesh_object("ropes", ropes, M("frame_main"))

# Render ------------------------------------------------------------------------------
# With S spokes the wheels look the same every 360/S degrees, so N frames of
# (360/S)/N degrees each loop seamlessly. A rope speed of about 3 units/s
# at natural size looks like a hoist running at a steady pace.
n = args.frames
step = (360.0 / max(1, args.spokes)) / max(1, n)          # degrees per frame
period = math.radians(step) * R * n / 3.0


def set_frame(i):
    for w in wheels:
        w.rotation_euler = (0.0, math.radians(step * (i or 0)), 0.0)


kit.render_axo_animated(args.name, n, set_frame, period=period,
                        yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
