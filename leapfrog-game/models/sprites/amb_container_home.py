"""
amb_container_home - ambient piece: a home made from two shipping containers
for the village on the 0.4 parallax layer.

A red container on concrete blocks, a blue one on top turned across it and
sticking out at the front on a post. A window cut into each, a door with a
lean-to roof on poles, a stovepipe and a ladder up the right side. Somebody
built this from what was lying around.

Like amb_hut: 14 px/unit, built at natural size and scaled x2, orthographic
3/4 view (render_axo). Turned -22 degrees by default (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_container_home.py -- --samples 16 --name amb_container_home_test
    blender -b --factory-startup -P amb_container_home.py
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_container_home", help="texture name (default amb_container_home)")
    p.add_argument("--dark", action="store_true", help="no lit windows")
    p.add_argument("--no-smoke", action="store_true", help="no smoke from the stovepipe")
    p.add_argument("--frames", type=int, default=6, help="animation frames (default 6; --still for one image)")
    p.add_argument("--period", type=float, default=2.4, help="seconds per animation loop (default 2.4)")
    p.add_argument("--lower", default="container_red", help="material role of the lower container")
    p.add_argument("--upper", default="container_blue", help="material role of the upper container")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=-22.0)
M = kit.material

L, Wd, H = 6.0, 2.4, 2.6          # container length, width, height
BLOCK = 0.4
z0 = BLOCK                        # lower container floor

# Concrete blocks under the lower container
for x in (-L / 2 + 0.5, L / 2 - 0.5):
    for y in (-Wd / 2 + 0.4, Wd / 2 - 0.4):
        sk.box_object("block_%g_%g" % (x, y), (x, y, BLOCK / 2), (0.7, 0.7, BLOCK), M("concrete_dirty"))

# Lower container, lengthwise
ap.corrugated_box("lower", M, (0.0, 0.0, z0 + H / 2), (L, Wd, H), args.lower)
front = -Wd / 2 - 0.06
ap.door("lower_door", M, (-L / 2 + 1.2, front, z0), (1.0, 2.1), frame_role="frame_main")
ap.window("lower_win", M, (0.9, front, z0 + 1.5), (1.6, 0.8), lit=not args.dark)

# Lean-to over the door, on two poles
poles = bmesh.new()
for x in (-L / 2 + 0.2, -L / 2 + 2.3):
    sk.strut(poles, (x, front - 1.6, 0.0), (x, front - 1.6, z0 + 2.3), 0.06, 6)
sk.mesh_object("leanto_poles", poles, M("frame_main"))
sk.box_object("leanto", (-L / 2 + 1.25, front - 0.85, z0 + 2.45), (2.6, 1.8, 0.06), M("rusty_metal"),
              rotation=(-0.12, 0.0, 0.0))

# Upper container across the lower one, sticking out at the front
ux = L / 2 - Wd / 2 - 0.3
uz = z0 + H
ap.corrugated_box("upper", M, (ux, -0.9, uz + H / 2), (Wd, L, H), args.upper)
ufront = -0.9 - L / 2 - 0.06
ap.window("upper_win", M, (ux, ufront, uz + 1.4), (1.2, 0.9), lit=not args.dark)
post = bmesh.new()
sk.strut(post, (ux - Wd / 2 + 0.2, ufront + 0.3, 0.0), (ux - Wd / 2 + 0.2, ufront + 0.3, uz), 0.1, 6)
sk.strut(post, (ux + Wd / 2 - 0.2, ufront + 0.3, 0.0), (ux + Wd / 2 - 0.2, ufront + 0.3, uz), 0.1, 6)
sk.mesh_object("upper_posts", post, M("frame_main"))

# Ladder up the right side of the lower container
lx = L / 2 + 0.25
ladder = bmesh.new()
for dy in (-0.3, 0.3):
    sk.strut(ladder, (lx, 0.5 + dy, 0.0), (lx, 0.5 + dy, uz + 0.8), 0.04, 6)
for i in range(1, 12):
    z = i * (uz + 0.6) / 12
    sk.strut(ladder, (lx, 0.2, z), (lx, 0.8, z), 0.03, 6)
sk.mesh_object("ladder", ladder, M("frame_brace"))

# Stovepipe on the upper container
pipe = bmesh.new()
sk.strut(pipe, (ux + 0.5, 0.6, uz + H), (ux + 0.5, 0.6, uz + H + 1.6), 0.12, 10)
sk.mesh_object("stovepipe", pipe, M("rusty_metal"))
sk.box_object("stovepipe_cap", (ux + 0.5, 0.6, uz + H + 1.7), (0.45, 0.45, 0.08), M("frame_main"))

# Smoke from the stovepipe: somebody is home
life = [] if args.no_smoke else [ap.smoke_column("smoke", M, (ux + 0.5, 0.6, uz + H + 1.8))]

kit.render_axo_animated(args.name, args.frames if life else 1, lambda i: [f(i, args.frames) for f in life], period=args.period,
                        yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
