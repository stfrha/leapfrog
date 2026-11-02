"""
stash_case - the raiders' stash case in mission 1: a heavy olive transport
case with steel straps and corners, a brass latch and hazard stripes.

Body in data/stash_case.xml: stashCaseBody, 14 x 9. Front view, ortho rig, 20
px/unit, framed exactly to the body, so x -7 .. 7 and z -4.5 .. 4.5. The
texture is already listed in res.xml (images/space); a render replaces the 2D
placeholder and keeps the entry.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P stash_case.py
    blender -b --factory-startup -P stash_case.py -- --samples 16 --suffix _test
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--strap-x", type=float, default=4.1, help="the straps' distance from the centre")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res.xml")

W = 14.0
H = 9.0
SEAM_Z = 1.9        # between lid and base

olive = kit.material("case_olive")
steel = kit.material("strap_steel")
dark = kit.material("frame_main")
brass = kit.material("latch_brass")
yellow = kit.material("sign_yellow")
black = kit.material("sign_black")


def rivet(name, x, z, y=-0.55, radius=0.2):
    bm = bmesh.new()
    sk.strut(bm, (x, y + 0.2, z), (x, y, z), radius, segments=16)
    return sk.mesh_object(name, bm, steel)


# ---------------------------------------------------------------------------
# The shell: lid and base, a dark gap between them, ribs along both edges
# ---------------------------------------------------------------------------
sk.box_object("gap", (0, 1.5, SEAM_Z), (W - 1.0, 2.0, 0.5), kit.material("void"))

base_h = SEAM_Z - 0.1 + (H / 2 - 0.25)
base_z = -(H / 2 - 0.25) + base_h / 2
sk.box_object("base", (0, 1.5, base_z), (W - 0.5, 3.0, base_h), olive, bevel_width=0.18)

lid_h = (H / 2 - 0.25) - SEAM_Z - 0.1
lid_z = SEAM_Z + 0.1 + lid_h / 2
sk.box_object("lid", (0, 1.5, lid_z), (W - 0.5, 3.0, lid_h), olive, bevel_width=0.18)

for i, z in enumerate((3.75, -3.85)):
    sk.box_object("rib_%d" % i, (0, -0.02, z), (W - 3.4, 0.14, 0.26), olive, bevel_width=0.05)

# ---------------------------------------------------------------------------
# Steel: corners and straps
# ---------------------------------------------------------------------------
for i, (sx, sz) in enumerate(((-1, -1), (1, -1), (-1, 1), (1, 1))):
    x = sx * (W / 2 - 0.82)
    z = sz * (H / 2 - 0.82)
    sk.box_object("corner_%d" % i, (x, 0.2, z), (1.64, 1.2, 1.64), steel, bevel_width=0.2)
    rivet("corner_rivet_%d" % i, x, z, y=-0.55, radius=0.17)

for i, sx in enumerate((-1, 1)):
    x = sx * args.strap_x
    sk.box_object("strap_%d" % i, (x, 0.0, 0), (1.0, 0.7, H - 0.1), steel, bevel_width=0.1)
    for j, z in enumerate((-3.2, 0.0, 3.2)):
        rivet("strap_rivet_%d_%d" % (i, j), x, z, y=-0.5)

# ---------------------------------------------------------------------------
# The latch across the seam, with its keyhole
# ---------------------------------------------------------------------------
sk.box_object("latch_back", (0, -0.05, SEAM_Z), (1.75, 0.3, 1.75), dark, bevel_width=0.12)
sk.box_object("latch", (0, -0.3, SEAM_Z), (1.3, 0.3, 1.3), brass, bevel_width=0.1)
sk.box_object("keyhole", (0, -0.46, SEAM_Z), (0.34, 0.06, 0.6), kit.material("void"))

# ---------------------------------------------------------------------------
# Markings: hazard stripes and a torn label
# ---------------------------------------------------------------------------
for i in range(6):
    x = -2.75 + i * 0.5
    sk.prism("hazard_%d" % i,
             [(x - 0.2, -1.5), (x + 0.03, -1.5), (x + 0.43, -0.45), (x + 0.2, -0.45)], -0.01, 0.02, yellow)

sk.prism("label", [(0.65, -1.45), (2.2, -1.25), (2.25, -0.3), (1.2, -0.38), (0.7, -0.62)],
         -0.01, 0.02, kit.material("label_paper"))
sk.prism("label_slash", [(0.75, -0.42), (0.95, -0.42), (2.2, -1.35), (2.0, -1.35)],
         -0.03, 0.02, kit.material("label_red"))

if not args.no_render:
    kit.render_ortho("stash_case" + args.suffix, (-W / 2, -H / 2, W / 2, H / 2))
