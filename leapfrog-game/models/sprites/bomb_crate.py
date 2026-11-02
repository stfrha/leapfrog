"""
bomb_crate - the explosive crate: a nailed wooden crate with crossed braces
and a yellow warning sign bolted to it, showing a blast.

Bodies in data/landing_scene_state2.xml and deep_space_scene.xml: bombBody and
magenticMineBody, 20 x 20. Front view, ortho rig, 20 px/unit, framed exactly to
the body, so x and z -10 .. 10. It replaces a downloaded picture: nothing here
comes from an image, the wood and paint are procedural. The texture is already
listed in res.xml (images); a render replaces the file and keeps the entry.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P bomb_crate.py
    blender -b --factory-startup -P bomb_crate.py -- --samples 16 --suffix _test
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--sign", type=float, default=6.5, help="the sign's half diagonal")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="ortho", res="res.xml")

S = 10.0            # half the crate's side
RAIL = 2.9          # width of the frame's planks
OPENING = S - RAIL  # half the opening inside the frame

wood = kit.material("wood_crate")
steel = kit.material("strap_steel")
yellow = kit.material("sign_yellow")
black = kit.material("sign_black")


def plank(name, center, length, width, depth, angle=0.0, bevel_width=0.07):
    """A plank of its own, grain along its length; angle turns it, seen from the front."""
    return sk.box_object(name, center, (length, depth, width), wood,
                         rotation=(0.0, math.radians(angle), 0.0), bevel_width=bevel_width)


def nail(name, x, z, y, radius=0.17):
    bm = bmesh.new()
    sk.strut(bm, (x, y + 0.12, z), (x, y, z), radius, segments=12)
    return sk.mesh_object(name, bm, steel)


# ---------------------------------------------------------------------------
# Boards behind everything, a dark inside showing through the gaps
# ---------------------------------------------------------------------------
sk.box_object("inside", (0, 1.6, 0), (2 * OPENING + 1, 0.4, 2 * OPENING + 1), kit.material("void"))

BOARDS = 5
pitch = 2 * OPENING / BOARDS
for i in range(BOARDS):
    x = -OPENING + pitch * (i + 0.5)
    plank("board_%d" % i, (x, 1.0, 0), 2 * OPENING + 1.0, pitch - 0.07, 0.8, angle=90, bevel_width=0.04)

# ---------------------------------------------------------------------------
# Crossed braces, one lapped over the other, cut off by the frame in front
# ---------------------------------------------------------------------------
brace_length = 2 * OPENING * math.sqrt(2) - 0.2
plank("brace_0", (0, 0.45, 0), brace_length, 2.4, 0.4, angle=45)
plank("brace_1", (0, 0.05, 0), brace_length, 2.4, 0.4, angle=-45)

# ---------------------------------------------------------------------------
# The frame: rails across, stiles between them, a nail row at each joint
# ---------------------------------------------------------------------------
for i, sz in enumerate((-1, 1)):
    plank("rail_%d" % i, (0, -0.2, sz * (S - RAIL / 2)), 2 * S, RAIL, 0.8)
for i, sx in enumerate((-1, 1)):
    plank("stile_%d" % i, (sx * (S - RAIL / 2), -0.2, 0), 2 * OPENING, RAIL, 0.8, angle=90)

for i, sx in enumerate((-1, 1)):
    for j, sz in enumerate((-1, 1)):
        x = sx * (S - RAIL / 2)
        z = sz * (OPENING - 1.1)
        for k, dx in enumerate((-0.75, 0.75)):
            nail("nail_%d_%d_%d" % (i, j, k), x + dx, z, -0.75)

# Steel corners
for i, (sx, sz) in enumerate(((-1, -1), (1, -1), (-1, 1), (1, 1))):
    x = sx * (S - 1.15)
    z = sz * (S - 1.15)
    sk.box_object("corner_%d" % i, (x, -0.6, z), (2.3, 0.3, 2.3), steel, bevel_width=0.1)
    for j, (dx, dz) in enumerate(((-0.65, -0.65), (0.65, -0.65), (-0.65, 0.65), (0.65, 0.65))):
        nail("corner_nail_%d_%d" % (i, j), x + dx, z + dz, -0.85, radius=0.2)


# ---------------------------------------------------------------------------
# The warning sign: rounded diamonds on top of each other, and a blast
# ---------------------------------------------------------------------------
def diamond(half, corner, steps=6):
    """A diamond with rounded corners; half = half diagonal before rounding."""
    points = []
    centre = half - corner * math.sqrt(2)
    for i in range(4):
        a0 = math.radians(90 * i - 45)
        cx = centre * math.cos(math.radians(90 * i))
        cz = centre * math.sin(math.radians(90 * i))
        for s in range(steps + 1):
            a = a0 + math.radians(90) * s / steps
            points.append((cx + corner * math.cos(a), cz + corner * math.sin(a)))
    return points


def burst(radii, turn=0.0):
    """A jagged star: radii are the distances of its points, in turn out and in."""
    n = len(radii)
    return [(r * math.cos(math.radians(turn + 360.0 * i / n)),
             r * math.sin(math.radians(turn + 360.0 * i / n))) for i, r in enumerate(radii)]


R = args.sign
sk.prism("sign", diamond(R, 0.7), -0.72, 0.1, yellow)
sk.prism("sign_ring", diamond(R - 0.45, 0.55), -0.74, 0.02, black)
sk.prism("sign_face", diamond(R - 0.75, 0.45), -0.76, 0.02, yellow)

sk.prism("blast", burst([4.1, 2.0, 3.3, 1.8, 4.3, 2.1, 3.0, 1.9, 4.0, 1.9, 3.4, 2.1,
                         4.2, 1.8, 3.1, 2.0, 3.9, 1.9, 3.3, 2.0], turn=8.0), -0.78, 0.02, black)
sk.prism("blast_core", burst([1.75, 0.95, 1.5, 0.9, 1.8, 1.0, 1.45, 0.9, 1.7, 1.0, 1.5, 0.95],
                             turn=20.0), -0.80, 0.02, yellow)

for i in range(4):
    a = math.radians(90 * i)
    nail("sign_bolt_%d" % i, (R - 0.72) * math.cos(a), (R - 0.72) * math.sin(a), -0.95, radius=0.26)

if not args.no_render:
    kit.render_ortho("bomb_crate" + args.suffix, (-S, -S, S, S))
