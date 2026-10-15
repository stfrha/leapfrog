"""
ls_booster_tank - the main tank at the launch site.

Body in launch_site.xml: dynamicBoxedSpritePolygonBody "mainTank", texture
ls_booster_tank, width 13.2, height 53. Profile taken from the old drawing:
dark nozzle at the bottom, boat-tail, cylinder, conical nose with a flat tip.

The white parts use the Leapfrog's hex plating. --tile sets how many game
units one texture tile (8 hexes across) covers. On the ship one tile is about
3.9 units (a hex is ~0.49 units wide); the default 7.8 makes the hexes twice
as big as on the ship (~1 unit, ~20 px).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P ls_booster_tank.py
    blender -b --factory-startup -P ls_booster_tank.py -- --samples 16 --name ls_booster_tank_test
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402


def options(p):
    p.add_argument("--name", default="ls_booster_tank", help="texture name (default ls_booster_tank)")
    p.add_argument("--tile", type=float, default=7.8,
                   help="game units per hex texture tile (ship is ~3.9; default 7.8 = hexes twice the ship's size)")
    p.add_argument("--segments", type=int, default=96, help="segments around the tank")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="ortho")

W = 13.2
H = 53.0
R = W / 2               # 6.6, cylinder radius = half the body width

Z_NOZZLE_RIM = 1.9      # top of the straight part of the nozzle
Z_NOZZLE_TOP = 2.85     # where the nozzle meets the tank's underside
Z_CYL_BOTTOM = 6.6      # end of the boat-tail
Z_CYL_TOP = 40.3        # start of the nose (collision polygon has it at 40.2)
R_TIP = 2.0             # flat tip radius

# White hull --------------------------------------------------------------
hull = bmesh.new()
uv = hull.loops.layers.uv.new("UVMap")
seg = args.segments
t = args.tile

# underside ring between nozzle and boat-tail
sk.lathe(hull, [(3.1, Z_NOZZLE_TOP), (3.9, Z_NOZZLE_TOP)], seg, t, uv)
# boat-tail
sk.lathe(hull, [(3.9, Z_NOZZLE_TOP), (R, Z_CYL_BOTTOM)], seg, t, uv)
# cylinder
sk.lathe(hull, [(R, Z_CYL_BOTTOM), (R, Z_CYL_TOP)], seg, t, uv)
# nose cone with flat tip
sk.lathe(hull, [(R, Z_CYL_TOP), (R_TIP, H)], seg, t, uv, cap_top=True)

sk.mesh_object("hull", hull, kit.material("hull_white"), smooth_quads=True)

# Nozzle ------------------------------------------------------------------
nozzle = bmesh.new()
sk.lathe(nozzle, [(4.0, 0.0), (4.0, Z_NOZZLE_RIM)], seg, cap_bottom=True)
sk.lathe(nozzle, [(4.0, Z_NOZZLE_RIM), (3.1, Z_NOZZLE_TOP)], seg)
sk.mesh_object("nozzle", nozzle, kit.material("nozzle"), smooth_quads=True)

# Render ------------------------------------------------------------------
kit.render_ortho(args.name, (-W / 2, 0.0, W / 2, H))
