"""
amb_hut - first ambient piece: a small habitation module for a village on a
parallax layer (landing scene, parallaxAmount 0.6).

A prefab module on a concrete plinth: ribbed walls, a rusty roof slab with an
overhang, a door on the left, lit windows, a water tank on the right side
(the side the camera sees), a roof vent and a mast with an amber light.

Ambient pieces differ from the foreground sprites:
  * 10 px/unit by default (a layer at 0.6 is drawn at about 0.4 of the
    foreground size when zoomed in, so 20 * 0.4 = 8 px/unit is enough).
    --ppu overrides it, e.g. to try the piece on another layer.
  * Orthographic 3/4 camera (render_axo), the same angles for every piece,
    so pieces can stand side by side anywhere on the layer. The bottom edge
    of the image is the front of the plinth on the ground.
  * Saved in data/images/mining/ambient/ and registered in res_mining.xml in
    the shared ambient atlas, with ppu="10" for the editor.
  * Bold shapes over fine detail: at this size only silhouettes, a few
    contrasting parts and the lit windows read.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_hut.py -- --samples 16 --name amb_hut_test
    blender -b --factory-startup -P amb_hut.py
Variants (as their own textures):
    ... -P amb_hut.py -- --name amb_hut_dark --dark --no-tank
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_hut", help="texture name (default amb_hut)")
    p.add_argument("--width", type=float, default=12.0, help="module width (default 12)")
    p.add_argument("--depth", type=float, default=6.0, help="module depth (default 6)")
    p.add_argument("--height", type=float, default=4.2, help="wall height above the plinth (default 4.2)")
    p.add_argument("--windows", type=int, default=3, help="windows on the front (default 3)")
    p.add_argument("--dark", action="store_true", help="unlit windows and mast light")
    p.add_argument("--no-tank", action="store_true", help="leave out the side tank")
    p.add_argument("--no-mast", action="store_true", help="leave out the mast")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=10)
M = kit.material

W = args.width
D = args.depth
H = args.height
PLINTH = 0.6
Z0 = PLINTH                    # wall bottom
Z1 = PLINTH + H                # wall top
FRONT = -D / 2                 # front wall plane (camera looks along +Y)
RIGHT = W / 2

# Plinth -------------------------------------------------------------------
sk.box_object("plinth", (0.0, 0.0, PLINTH / 2), (W + 1.0, D + 1.0, PLINTH), M("concrete_dirty"), bevel_width=0.08)

# Walls --------------------------------------------------------------------
sk.box_object("walls", (0.0, 0.0, Z0 + H / 2), (W, D, H), M("frame_brace"), bevel_width=0.06)

# Ribs on the front and right side: what makes it read as a prefab module
ribs = bmesh.new()
n_ribs = max(2, int(round(W / 2.0)))
for i in range(n_ribs + 1):
    x = -W / 2 + W * i / n_ribs
    sk.box(ribs, (x, FRONT - 0.08, Z0 + H / 2), (0.28, 0.16, H))
for i in range(4):
    y = -D / 2 + D * i / 3
    sk.box(ribs, (RIGHT + 0.08, y, Z0 + H / 2), (0.16, 0.28, H))
sk.box(ribs, (0.0, FRONT - 0.1, Z0 + 0.15), (W + 0.2, 0.2, 0.3))          # base rail
sk.mesh_object("ribs", ribs, M("frame_main"))

# Roof slab with an overhang, and a darker underside line from its thickness
sk.box_object("roof", (0.0, 0.0, Z1 + 0.2), (W + 0.8, D + 0.8, 0.4), M("rusty_metal"), bevel_width=0.05)

# Door (left) with a hazard frame and a small canopy ------------------------
door_x = -W / 2 + 1.6
sk.box_object("door_frame", (door_x, FRONT - 0.12, Z0 + 1.3), (1.7, 0.1, 2.6), M("hazard_paint"))
sk.box_object("door", (door_x, FRONT - 0.18, Z0 + 1.2), (1.2, 0.1, 2.3), M("frame_main"))
sk.box_object("canopy", (door_x, FRONT - 0.6, Z0 + 2.75), (2.0, 1.0, 0.12), M("pad_metal"))

# Windows ------------------------------------------------------------------
win_mat = M("glass_dark") if args.dark else M("window_lit_blue")
n_win = max(0, args.windows)
if n_win:
    x0 = door_x + 1.8
    x1 = RIGHT - 1.2
    for i in range(n_win):
        x = x0 + (x1 - x0) * (i + 0.5) / n_win
        sk.box_object("win_frame_%d" % i, (x, FRONT - 0.14, Z0 + 2.4), (1.5, 0.08, 1.1), M("frame_main"))
        sk.box_object("win_%d" % i, (x, FRONT - 0.2, Z0 + 2.4), (1.2, 0.06, 0.8), win_mat)

# Roof vent -----------------------------------------------------------------
sk.box_object("vent", (-W / 4, 0.4, Z1 + 0.8), (1.4, 1.4, 0.8), M("pad_metal"), bevel_width=0.04)
sk.box_object("vent_cap", (-W / 4, 0.4, Z1 + 1.28), (1.8, 1.8, 0.16), M("frame_main"))

# Mast with a light ----------------------------------------------------------
if not args.no_mast:
    mast = bmesh.new()
    mx, my = W / 4, D / 4
    sk.strut(mast, (mx, my, Z1 + 0.4), (mx, my, Z1 + 4.0), 0.09)
    sk.strut(mast, (mx - 0.6, my, Z1 + 0.4), (mx, my, Z1 + 2.0), 0.05)
    sk.strut(mast, (mx + 0.6, my, Z1 + 0.4), (mx, my, Z1 + 2.0), 0.05)
    sk.strut(mast, (mx - 0.5, my, Z1 + 3.4), (mx + 0.5, my, Z1 + 3.4), 0.05)
    sk.mesh_object("mast", mast, M("frame_main"))
    sk.box_object("mast_light", (mx, my, Z1 + 4.12), (0.3, 0.3, 0.24),
                  M("neon_dead") if args.dark else M("neon_amber"))

# Side tank (right, the side the camera sees) --------------------------------
if not args.no_tank:
    TR = 1.1
    TH = 3.0
    tx = RIGHT + 0.4 + TR
    ty = 0.6
    tank = bmesh.new()
    sk.lathe(tank, [(TR, 0.0), (TR, TH)], 32, cap_bottom=True)
    sk.lathe(tank, [(TR, TH), (TR * 0.7, TH + 0.35)], 32, cap_top=True)
    ob = sk.mesh_object("tank", tank, M("hazard_paint"))
    ob.location = (tx, ty, PLINTH + 0.5)
    legs = bmesh.new()
    for lx in (-0.7, 0.7):
        sk.box(legs, (tx + lx, ty - 0.5, (PLINTH + 0.5) / 2), (0.2, 0.2, PLINTH + 0.5))
        sk.box(legs, (tx + lx, ty + 0.5, (PLINTH + 0.5) / 2), (0.2, 0.2, PLINTH + 0.5))
    sk.mesh_object("tank_legs", legs, M("frame_main"))
    pipe = bmesh.new()
    sk.strut(pipe, (tx - TR + 0.1, ty - 0.4, PLINTH + 1.2), (RIGHT, ty - 0.4, PLINTH + 1.2), 0.12)
    sk.mesh_object("pipe", pipe, M("rusty_metal"))

kit.render_axo(args.name, yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
