"""
ls_tower - lattice tower at the launch site.

Body in launch_site.xml: staticBox "leftTower"/"rightTower", texture ls_tower,
width 9, height 106.7. Trolleys are welded to the tower sides at x = +-4.5,
so the image edges must be exactly the outer faces of the legs.

Square lattice tower: four corner legs, X bracing on every face, horizontal
girts at the bottom, top and (optionally) every N panels. Rendered from the
front with an orthographic camera; the back face is visible through the
gaps, which gives the depth the flat drawing lacked.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P ls_tower.py
    blender -b --factory-startup -P ls_tower.py -- --samples 16 --name ls_tower_test
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--name", default="ls_tower", help="texture name (default ls_tower)")
    p.add_argument("--width", type=float, default=9.0)
    p.add_argument("--height", type=float, default=106.7)
    p.add_argument("--panels", type=int, default=23, help="number of X panels up the tower")
    p.add_argument("--girt-every", type=int, default=0,
                   help="horizontal girt every N panels (0 = only bottom and top)")
    p.add_argument("--leg", type=float, default=0.8, help="leg thickness")
    p.add_argument("--brace-radius", type=float, default=0.14)


args = sk.parse_args(options)
kit = sk.Kit(args, rig="ortho")

W = args.width
H = args.height
D = W                        # square cross-section
LEG = args.leg
XI = W / 2 - LEG / 2         # leg centre lines
YI = D / 2 - LEG / 2
PH = H / args.panels         # panel height

# Legs ---------------------------------------------------------------------
legs = bmesh.new()
for sx in (-1, 1):
    for sy in (-1, 1):
        sk.box(legs, (sx * XI, sy * YI, H / 2), (LEG, LEG, H))
legs_ob = sk.mesh_object("legs", legs, kit.material("frame_main"))
sk.bevel(legs_ob, width=0.06)

# Bracing ------------------------------------------------------------------
br = bmesh.new()
R = args.brace_radius

for i in range(args.panels):
    z0 = i * PH
    z1 = (i + 1) * PH
    for y in (-YI, YI):                       # front and back faces
        sk.strut(br, (-XI, y, z0), (XI, y, z1), R)
        sk.strut(br, (XI, y, z0), (-XI, y, z1), R)
    for x in (-XI, XI):                       # side faces
        sk.strut(br, (x, -YI, z0), (x, YI, z1), R)
        sk.strut(br, (x, YI, z0), (x, -YI, z1), R)

girt_levels = {0, args.panels}
if args.girt_every > 0:
    girt_levels.update(range(0, args.panels + 1, args.girt_every))
for i in sorted(girt_levels):
    z = min(max(i * PH, R), H - R)            # keep end girts inside the box
    for y in (-YI, YI):
        sk.strut(br, (-XI, y, z), (XI, y, z), R)
    for x in (-XI, XI):
        sk.strut(br, (x, -YI, z), (x, YI, z), R)

sk.mesh_object("bracing", br, kit.material("frame_brace"), smooth_quads=True)

# Render -------------------------------------------------------------------
kit.render_ortho(args.name, (-W / 2, 0.0, W / 2, H))
