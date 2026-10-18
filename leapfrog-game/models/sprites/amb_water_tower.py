"""
amb_water_tower - ambient piece: the village water tower for the 0.4
parallax layer.

A rusty tank on four splayed legs with X bracing, a conical roof, a hazard
band, a ladder up the front and a pipe down the middle. A beacon on top.
Tall and thin, it gives the village a skyline.

Like amb_hut: 14 px/unit, built at natural size and scaled x2, orthographic
3/4 view (render_axo). Turned 30 degrees by default so the legs and bracing
don't line up with the camera (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_water_tower.py -- --samples 16 --name amb_water_tower_test
    blender -b --factory-startup -P amb_water_tower.py
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_water_tower", help="texture name (default amb_water_tower)")
    p.add_argument("--height", type=float, default=8.0, help="height of the legs (default 8)")
    p.add_argument("--dark", action="store_true", help="beacon off")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=30.0)
M = kit.material

HT = args.height
B = 1.8                    # half spread of the legs at the ground
T = 1.2                    # half spread at the top
TR = 2.3                   # tank radius
TH = 3.0                   # tank height

# Footings
for sx in (-1, 1):
    for sy in (-1, 1):
        sk.box_object("footing_%d_%d" % (sx, sy), (sx * B, sy * B, 0.2), (0.7, 0.7, 0.4), M("concrete_dirty"))

# Legs and bracing
legs = bmesh.new()
corners_lo = [(-B, -B), (B, -B), (B, B), (-B, B)]
corners_hi = [(-T, -T), (T, -T), (T, T), (-T, T)]
for (x0, y0), (x1, y1) in zip(corners_lo, corners_hi):
    sk.strut(legs, (x0, y0, 0.4), (x1, y1, HT), 0.14, 8)
sk.mesh_object("legs", legs, M("frame_main"))

brace = bmesh.new()
LEVELS = 3
for k in range(LEVELS):
    z0 = 0.4 + (HT - 0.4) * k / LEVELS
    z1 = 0.4 + (HT - 0.4) * (k + 1) / LEVELS
    f0 = k / LEVELS
    f1 = (k + 1) / LEVELS

    def at(i, f):
        (xa, ya), (xb, yb) = corners_lo[i], corners_hi[i]
        return (xa + (xb - xa) * f, ya + (yb - ya) * f)

    for i in range(4):
        j = (i + 1) % 4
        a0, a1 = at(i, f0), at(i, f1)
        b0, b1 = at(j, f0), at(j, f1)
        sk.strut(brace, (a0[0], a0[1], z0), (b1[0], b1[1], z1), 0.05, 6)
        sk.strut(brace, (b0[0], b0[1], z0), (a1[0], a1[1], z1), 0.05, 6)
        sk.strut(brace, (a1[0], a1[1], z1), (b1[0], b1[1], z1), 0.06, 6)
sk.mesh_object("bracing", brace, M("frame_brace"))

# Tank, roof and band
sk.box_object("deck", (0.0, 0.0, HT + 0.1), (2 * T + 1.2, 2 * T + 1.2, 0.2), M("pad_metal"))
ap.cylinder("tank", M, "rusty_metal", (0.0, 0.0, HT + 0.2), TR, TH, 40, cone=0.9)
ap.cylinder("band", M, "hazard_paint", (0.0, 0.0, HT + 0.2 + TH * 0.62), TR + 0.04, 0.45, 40)

# Pipe down the middle
pipe = bmesh.new()
sk.strut(pipe, (0.0, 0.0, 0.0), (0.0, 0.0, HT + 0.2), 0.22, 10)
sk.mesh_object("pipe", pipe, M("rusty_metal"))

# Ladder up the front right leg, then up the tank
lad = bmesh.new()
lx, ly = T * 0.2, -TR - 0.25
for dx in (-0.3, 0.3):
    sk.strut(lad, (lx + dx, ly, 0.0), (lx + dx, ly, HT + TH + 0.3), 0.04, 6)
n = int((HT + TH) / 0.45)
for i in range(1, n):
    z = i * (HT + TH + 0.2) / n
    sk.strut(lad, (lx - 0.3, ly, z), (lx + 0.3, ly, z), 0.03, 6)
for z in (HT * 0.5, HT + 0.1):
    sk.strut(lad, (lx, ly, z), (lx, -T, z), 0.04, 6)          # ties back to the tower
sk.mesh_object("ladder", lad, M("frame_brace"))

# Beacon on the roof
sk.box_object("beacon", (0.0, 0.0, HT + 0.2 + TH + 1.0), (0.35, 0.35, 0.3),
              M("neon_dead") if args.dark else M("neon_amber"))

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
