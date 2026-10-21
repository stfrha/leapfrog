"""
ore_ground - repeating ore ground for a distant mine site: a polygon fill
and a rugged border strip, in the colours of the ore heaps (ore_rock).

This runs white_gravel.py with ore settings, so everything there applies
(seamless repeat, overscan + crop, the border's rugged top and fade):

  * ore stones (material role ore_stone: each stone between grey rock and
    rust-brown ore) on a dark dirt bed (ore_bed);
  * the render's own colour and brightness (--color --no-level), not the
    neutral grey of white_gravel, so the polygon needs no tint;
  * little detail, for far away: 14 px/unit (the 0.4 layer, like the mine
    pieces), stones 1.2 units across, a 256 x 256 px fill (18.3 units per
    repeat) and a 256 x 64 px border.

Textures: ore_ground and ore_ground_border in data/images/mining/, each in
its own clamp2edge="false" atlas in res_mining.xml. The script prints the
polygon's textureMeterWidth/Height and the <groundBorder ...> values.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P ore_ground.py -- --samples 16 --name ore_ground_test
    blender -b --factory-startup -P ore_ground.py
Any white_gravel option overrides these defaults, e.g. -- --stone 1.6 --px 512.
"""

import os
import runpy
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

DEFAULTS = [
    ("--name", "ore_ground"),
    ("--ppu", "14"),
    ("--stone", "1.2"),
    ("--px", "256"),
    ("--border-px", "64"),
    ("--rugged", "0.9"),
    ("--stone-role", "ore_stone"),
    ("--bed-role", "ore_bed"),
    ("--color", None),
    ("--no-level", None),
]

if "--" not in sys.argv:
    sys.argv.append("--")
given = {a.split("=")[0] for a in sys.argv[sys.argv.index("--") + 1:]}
for option, value in DEFAULTS:
    if option not in given:
        sys.argv.append(option)
        if value is not None:
            sys.argv.append(value)

runpy.run_path(os.path.join(HERE, "white_gravel.py"), run_name="__main__")
