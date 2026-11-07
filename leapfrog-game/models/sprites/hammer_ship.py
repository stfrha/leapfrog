"""
hammer_ship - the military "hammer" fighter seen from above, replacing the
hand-drawn hammer.png. One side of a battle axe: a half-moon blade at the
front whose curved edge is bright ground steel (a sloped bevel with grind
marks along it and a polished hone at the very edge), the cheek behind it
armour in dark military paint with rivets and plate joints, a raised spine
with the gun, and a slim fuselage to the engine with swept tail fins.

Body in deep_space_scene.xml (hammer1, hammer2): mainBody 24 x 12.6, front
to the left (gun at x -10 firing left, booster flame at x +12). Top-down
ortho, framed exactly to the body: x -12 .. 12, z -6.3 .. 6.3, 20 px/unit
(480 x 252). The texture "hammer" is already in res_space.xml; a render
replaces the drawing (it's in git) and keeps the entry. stripper.xml uses
the same texture as a tinted placeholder.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P hammer_ship.py
    blender -b --factory-startup -P hammer_ship.py -- --samples 16 --suffix _test
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import sprite_kit as sk           # noqa: E402
import axe_parts as ax            # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--band", type=float, default=1.25, help="width of the ground bevel (default 1.25)")
    p.add_argument("--number", default="07", help="painted on the upper cheek (empty: none)")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")

W, H = 24.0, 12.6
TOP = -0.7                       # the cheek's top (towards the camera)

paint = ax.military_paint(kit, "mil_grey", (0.075, 0.085, 0.095), rust=0.12)
paint_light = ax.military_paint(kit, "mil_grey_light", (0.13, 0.14, 0.15), rust=0.1)
seam = kit.material("frame_main")
steel = kit.material("strap_steel")
dark = kit.material("void")
red = ax.flat_paint("mil_red", (0.45, 0.03, 0.02))
white = ax.flat_paint("mil_white", (0.75, 0.75, 0.72))

# ---------------------------------------------------------------------------
# The blade: half-moon edge from horn to horn through the front
# ---------------------------------------------------------------------------
blade = ax.Blade("blade", front=-11.85, horn=(-7.4, 6.2), join=(-1.5, 2.1), bite=(-4.0, 2.9),
                 band=args.band, hone=0.18, top=TOP)
blade.build(paint, ax.ground_steel(), ax.hone_steel())
blade.rivets(steel, inset=0.42, spacing=0.8)
blade.panel_lines(seam, (128, 152, 208, 232), inner=4.2)

# Spine down the middle of the cheek, carrying the gun
spine = [(-9.9, 0.0), (-7.2, 0.75), (-1.0, 0.95), (-1.0, -0.95), (-7.2, -0.75)]
ax.slab("spine", spine, TOP - 0.35, 0.2, paint_light)
gun = sk.box_object("gun_housing", (-7.6, TOP - 0.55, 0.0), (2.6, 0.5, 1.0), seam, bevel_width=0.1)
import bmesh                     # noqa: E402
bm = bmesh.new()
sk.strut(bm, (-10.25, TOP - 0.6, 0.0), (-7.0, TOP - 0.6, 0.0), 0.2, 16)
sk.mesh_object("gun_barrel", bm, steel, smooth_quads=True)
sk.box_object("gun_muzzle", (-10.15, TOP - 0.6, 0.0), (0.35, 0.55, 0.55), seam)

# ---------------------------------------------------------------------------
# Fuselage to the engine, canopy, tail fins
# ---------------------------------------------------------------------------
body = [(-3.0, 2.0), (4.0, 1.9), (8.0, 1.5), (10.6, 1.2), (11.4, 1.0), (11.4, -1.0), (10.6, -1.2), (8.0, -1.5),
        (4.0, -1.9), (-3.0, -2.0), (-3.6, 0.0)]
ax.slab("fuselage", body, TOP - 0.3, 0.3, paint)
ax.slab("fuselage_ridge", [(-2.4, 0.9), (7.5, 0.6), (9.8, 0.0), (7.5, -0.6), (-2.4, -0.9)], TOP - 0.5, 0.0, paint_light)
canopy = [(0.4, 0.0), (1.2, 0.75), (3.6, 0.75), (4.5, 0.0), (3.6, -0.75), (1.2, -0.75)]
ax.slab("canopy_frame", [(x * 1.06 - 0.12, z * 1.25) for x, z in canopy], TOP - 0.6, TOP - 0.3, seam)
ax.slab("canopy", canopy, TOP - 0.75, TOP - 0.4, kit.material("glass_dark"))
ax.slab("canopy_glint", [(1.4, 0.5), (3.2, 0.5), (3.0, 0.3), (1.6, 0.3)], TOP - 0.77, TOP - 0.74,
        ax.flat_paint("glint", (0.55, 0.62, 0.7), rough=0.2))
for sz in (1, -1):
    fin = [(7.6, sz * 1.45), (10.1, sz * 4.6), (11.35, sz * 4.6), (10.9, sz * 1.15)]
    ax.slab("fin_%d" % sz, fin, TOP + 0.05, 0.25, paint)
    ax.slab("fin_tip_%d" % sz, [(10.1, sz * 4.6), (11.35, sz * 4.6), (11.3, sz * 4.25), (9.85, sz * 4.25)],
            TOP + 0.04, 0.25, red)
    for k, x in enumerate((-1.5, 1.0, 5.5)):
        ax.stud("body_rivet_%d_%d" % (sz, k), x, sz * 1.55, TOP - 0.3, 0.1, steel)
sk.box_object("engine", (11.55, TOP - 0.1, 0.0), (0.9, 1.0, 1.7), kit.material("nozzle"), bevel_width=0.08)
sk.box_object("engine_throat", (11.95, TOP - 0.1, 0.0), (0.12, 0.6, 1.2), dark)

# Markings: a red band round the fuselage, a number on the cheek
ax.slab("band_red", [(5.2, 1.85), (5.9, 1.8), (5.9, -1.8), (5.2, -1.85)], TOP - 0.52, TOP - 0.2, red)
if args.number:
    sk.text_object("number", args.number, 1.5, [white], location=(-6.4, TOP - 0.02, 2.35),
                   rotation=(math.pi / 2, 0.0, 0.0), extrude=0.01)

if not args.no_render:
    kit.render_ortho("hammer" + args.suffix, (-W / 2, -H / 2, W / 2, H / 2))
