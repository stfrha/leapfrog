"""
hammer_interceptor - the small, fast military interceptor seen from above: a
double-bitted battle axe. A half-moon blade at the front and one at the back,
both with bright ground edges like the hammer's, joined by a short spine
through a round cockpit hub, so the outline is almost a circle. Lighter than
the hammer: thinner cheeks with lightening holes, fewer rivets, a single
small gun in front, the engine in a slot in the rear blade.

Body in data/hammer_interceptor.xml: a circle, radius 6.5, front to the left
(gun at x -5.5 firing left, booster flame at x +6.2). Top-down ortho, framed
to the circle: x, z -6.5 .. 6.5, 20 px/unit (260 x 260).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P hammer_interceptor.py
    blender -b --factory-startup -P hammer_interceptor.py -- --samples 16 --suffix _test
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import axe_parts as ax            # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--band", type=float, default=0.9, help="width of the ground bevel (default 0.9)")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")

R_BODY = 6.5
TOP = -0.45                      # thinner cheeks than the hammer's

paint = ax.military_paint(kit, "mil_grey", (0.075, 0.085, 0.095), rust=0.12)
paint_light = ax.military_paint(kit, "mil_grey_light", (0.13, 0.14, 0.15), rust=0.1)
seam = kit.material("frame_main")
steel = kit.material("strap_steel")
dark = kit.material("void")
red = ax.flat_paint("mil_red", (0.45, 0.03, 0.02))
white = ax.flat_paint("mil_white", (0.75, 0.75, 0.72))
ground = ax.ground_steel(streaks=7.0)
hone = ax.hone_steel()

# ---------------------------------------------------------------------------
# Two blades, front and back; their horns almost meet above and below
# ---------------------------------------------------------------------------
for sx, prefix in ((1.0, "front"), (-1.0, "back")):
    b = ax.Blade(prefix, front=-6.45, horn=(-1.95, 5.95), join=(-1.1, 1.55), bite=(-2.4, 3.4),
                 band=args.band, hone=0.14, top=TOP, sx=sx)
    b.build(paint, ground, hone)
    b.rivets(steel, inset=0.32, spacing=0.75, radius=0.09, every=2)
    b.panel_lines(seam, (124, 236), inner=3.2, line=0.06)
    # Lightening holes: rimmed, dark
    for sz in (1, -1):
        x, z = sx * -3.55, sz * 2.45
        ax.disc("%s_hole_rim_%d" % (prefix, sz), x, z, 0.62, TOP - 0.02, steel, height=0.06)
        ax.disc("%s_hole_%d" % (prefix, sz), x, z, 0.46, TOP - 0.05, dark, height=0.06)

# Spine through the hub, blade to blade
spine = [(-5.3, 0.0), (-4.0, 0.55), (4.0, 0.55), (5.3, 0.0), (4.0, -0.55), (-4.0, -0.55)]
ax.slab("spine", spine, TOP - 0.25, 0.2, paint_light)

# ---------------------------------------------------------------------------
# The hub: a round cockpit with a red ring
# ---------------------------------------------------------------------------
ax.lathe_up("hub", [(2.05, 0.0), (2.05, 0.75), (1.85, 0.95)], 0.0, 0.0, 0.2, paint)
ax.lathe_up("hub_ring", [(1.84, 0.955), (1.84, 0.97), (1.45, 0.97)], 0.0, 0.0, 0.2, red)
ax.lathe_up("canopy", [(1.4, 0.9), (1.3, 1.25), (0.95, 1.5), (0.5, 1.65), (0.001, 1.7)], 0.0, 0.0, 0.2,
            kit.material("glass_dark"))
for k in range(6):
    a = math.radians(30 + k * 60)
    ax.stud("hub_rivet_%d" % k, 1.65 * math.cos(a), 1.65 * math.sin(a), 0.2 - 0.97, 0.08, steel, height=0.05)

# Gun: a single short barrel in the front blade
sk.box_object("gun_housing", (-4.3, TOP - 0.42, 0.0), (1.4, 0.35, 0.7), seam, bevel_width=0.06)
bm = bmesh.new()
sk.strut(bm, (-5.55, TOP - 0.45, 0.0), (-3.8, TOP - 0.45, 0.0), 0.13, 12)
sk.mesh_object("gun_barrel", bm, steel, smooth_quads=True)

# Engine in a slot in the rear blade
sk.box_object("engine_slot", (5.05, TOP - 0.02, 0.0), (1.3, 0.2, 1.3), dark)
sk.box_object("engine", (4.75, TOP - 0.3, 0.0), (1.1, 0.6, 1.0), kit.material("nozzle"), bevel_width=0.06)

# Markings: white chevrons on the front cheeks, pointing forward
for sz in (1, -1):
    z0 = sz * 1.2
    sk.prism("chevron_%d" % sz, [(-4.9, z0), (-4.3, z0 + sz * 0.6), (-3.85, z0 + sz * 0.6), (-4.45, z0)],
             TOP - 0.01, 0.02, white)

if not args.no_render:
    kit.render_ortho("hammer_interceptor" + args.suffix, (-R_BODY, -R_BODY, R_BODY, R_BODY))
