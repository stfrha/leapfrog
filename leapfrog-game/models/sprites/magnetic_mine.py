"""
magnetic_mine - the magnetic mine seen from above, after an old naval contact
mine: a riveted sphere in weathered teal-grey with flanged detonator horns
(fewer than the real thing), each horn tipped with a small horseshoe magnet,
red with white poles. A short stub of broken chain trails from it, torn from
its mooring (--no-chain to leave it out).

The mines in deep_space_scene.xml (mine1 and the factory's "mine") are
20 x 20 boxes; set their texture to magnetic_mine. Top-down ortho framed to
that box: x, z -10 .. 10, 20 px/unit (400 x 400). The sphere is turned a
little (--tilt) so its riveted seam shows as an ellipse.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P magnetic_mine.py
    blender -b --factory-startup -P magnetic_mine.py -- --samples 16 --suffix _test
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402
import sprite_kit as sk           # noqa: E402
import axe_parts as ax            # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--radius", type=float, default=6.5, help="the sphere's radius (default 6.5)")
    p.add_argument("--horn", type=float, default=1.9, help="horn length (default 1.9)")
    p.add_argument("--rim", type=int, default=8, help="horns around the rim (default 8; five more on the face)")
    p.add_argument("--tilt", type=float, default=30.0, help="the seam's tilt towards the camera, degrees")
    p.add_argument("--no-chain", action="store_true", help="no broken chain stub")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")

R = args.radius
HALF = 10.0

hull = ax.military_paint(kit, "mine_teal", (0.075, 0.12, 0.115), rust=0.35, streak=0.15)
horn_paint = ax.military_paint(kit, "mine_teal_light", (0.10, 0.155, 0.15), rust=0.3, streak=0.15)
red = ax.flat_paint("magnet_red", (0.62, 0.03, 0.02), rough=0.4)
white = ax.flat_paint("magnet_white", (0.85, 0.85, 0.83), rough=0.35)
chain_steel = kit.material("rusty_metal")
rivet_steel = kit.material("strap_steel")


def obj(name, bm, material):
    return sk.mesh_object(name, bm, material, smooth_quads=True)


def ball(bm, centre, radius, segs=12):
    bmesh.ops.create_uvsphere(bm, u_segments=segs, v_segments=max(6, segs // 2), radius=radius,
                              matrix=Matrix.Translation(centre))


def perpendicular(a):
    """A direction across a, in the plane most facing the camera (-Y), so a
    horseshoe drawn in the plane (a, b) shows its U from above."""
    view = Vector((0.0, -1.0, 0.0))
    b = a.cross(view)
    if b.length < 0.2:
        b = a.cross(Vector((0.0, 0.0, 1.0)))
    return b.normalized()


# ---------------------------------------------------------------------------
# The sphere and its riveted seam
# ---------------------------------------------------------------------------
bm = bmesh.new()
ball(bm, Vector((0, 0, 0)), R, segs=64)
obj("sphere", bm, hull)

tilt = math.radians(args.tilt)
axis = Vector((0.0, -math.sin(tilt), math.cos(tilt)))          # the seam's axis
rot = axis.to_track_quat('Z', 'Y').to_matrix().to_4x4()
bm = bmesh.new()
sk.lathe(bm, [(R + 0.14, -0.32), (R + 0.14, 0.32)], 96, cap_top=False, cap_bottom=False)
band = obj("seam", bm, hull)
band.matrix_world = rot
bm = bmesh.new()
u = rot @ Vector((1, 0, 0))
v = rot @ Vector((0, 1, 0))
for k in range(44):
    t = 2 * math.pi * k / 44
    d = (u * math.cos(t) + v * math.sin(t)).normalized()
    ball(bm, d * (R + 0.2), 0.13, segs=8)
obj("seam_rivets", bm, rivet_steel)

# ---------------------------------------------------------------------------
# Horns: flange, horn, cap, magnet
# ---------------------------------------------------------------------------
dirs = []
for k in range(args.rim):
    p = 2 * math.pi * k / args.rim
    dirs.append(Vector((math.cos(p), -0.15, math.sin(p))).normalized())
el = math.radians(50.0)
for k in range(4):
    p = math.radians(22.5 + 90.0 * k)
    dirs.append(Vector((math.cos(p) * math.cos(el), -math.sin(el), math.sin(p) * math.cos(el))).normalized())
dirs.append(Vector((0.08, -1.0, 0.05)).normalized())

flanges = bmesh.new()
horns = bmesh.new()
bolts = bmesh.new()
reds = bmesh.new()
whites = bmesh.new()
for i, a in enumerate(dirs):
    b = perpendicular(a)
    c = a.cross(b).normalized()
    sk.strut(flanges, a * (R - 0.25), a * (R + 0.22), 0.78, 24)
    for k in range(6):
        t = 2 * math.pi * k / 6
        ball(bolts, a * (R + 0.24) + (b * math.cos(t) + c * math.sin(t)) * 0.58, 0.09, segs=6)
    tip = a * (R + args.horn)
    sk.strut(horns, a * (R + 0.1), tip, 0.3, 16)
    ball(horns, tip, 0.31, segs=12)
    # Horseshoe magnet on the tip: the U's bottom on the horn, poles outwards
    rm, tube, leg = 0.36, 0.13, 0.5
    centre = tip + a * (rm + 0.22)
    arc = []
    for k in range(9):
        t = math.pi * k / 8
        arc.append(centre + (b * math.cos(t) - a * math.sin(t)) * rm)
    for p0, p1 in zip(arc, arc[1:]):
        sk.strut(reds, p0, p1, tube, 10)
    for p in arc:
        ball(reds, p, tube, segs=8)
    sk.strut(reds, tip, centre - a * rm, 0.12, 8)                # stem from the horn
    for side in (1, -1):
        foot = centre + b * (side * rm)
        mid = foot + a * (leg * 0.5)
        end = foot + a * leg
        sk.strut(reds, foot, mid, tube, 10)
        sk.strut(whites, mid, end, tube, 10)
        ball(whites, end, tube, segs=8)
obj("flanges", flanges, hull)
obj("flange_bolts", bolts, rivet_steel)
obj("horns", horns, horn_paint)
obj("magnets_red", reds, red)
obj("magnets_white", whites, white)

# ---------------------------------------------------------------------------
# A stub of broken chain, between two rim horns
# ---------------------------------------------------------------------------
if not args.no_chain:
    p = math.radians(360.0 / args.rim * 5.5)
    d = Vector((math.cos(p), 0.0, math.sin(p))).normalized()
    side = Vector((-d.z, 0.0, d.x))
    links = bmesh.new()
    sk.strut(links, d * (R - 0.3), d * (R + 0.45), 0.55, 16)          # the lug
    pos = R + 0.45
    for k in range(3):
        L, Wd, wire = 1.3, 0.8, 0.17
        centre = d * (pos + L / 2 - wire)
        across = side if k % 2 == 0 else Vector((0.0, -1.0, 0.0))  # alternate: flat, then edge on
        pts = []
        for j in range(17):
            t = 2 * math.pi * j / 16
            if k == 2 and 0.35 * math.pi < t < 0.65 * math.pi:     # the last link is broken open
                pts.append(None)
                continue
            pts.append(centre + d * (math.sin(t) * L / 2) + across * (math.cos(t) * Wd / 2))
        for p0, p1 in zip(pts, pts[1:]):
            if p0 is not None and p1 is not None:
                sk.strut(links, p0, p1, wire, 8)
        pos += L - 2 * wire - 0.1
    obj("chain", links, chain_steel)

if not args.no_render:
    kit.render_ortho("magnetic_mine" + args.suffix, (-HALF, -HALF, HALF, HALF))
