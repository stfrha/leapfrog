"""
amb_dome - ambient piece: an early-settlement habitat dome for the village
on the 0.4 parallax layer.

A low dome on a concrete ring wall, with panel seams, a ring of small
windows and an airlock tube at the front with a door. A mast with a light
on top. The oldest building in the village: the first settlers lived in
domes before anyone bothered with walls.

Like amb_hut: 14 px/unit, built at natural size and scaled x2, orthographic
3/4 view (render_axo). Turned 15 degrees by default so the airlock doesn't
point straight at the camera (--turn).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_dome.py -- --samples 16 --name amb_dome_test
    blender -b --factory-startup -P amb_dome.py
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_dome", help="texture name (default amb_dome)")
    p.add_argument("--radius", type=float, default=5.0, help="dome radius (default 5)")
    p.add_argument("--windows", type=int, default=10, help="windows around the dome (default 10)")
    p.add_argument("--lit", type=float, default=0.5, help="share of lit windows, 0..1 (default 0.5)")
    p.add_argument("--dark", action="store_true", help="no lit windows or mast light")
    p.add_argument("--frames", type=int, default=6, help="animation frames (default 6; --still for one image)")
    p.add_argument("--period", type=float, default=2.4, help="seconds per animation loop (default 2.4)")
    p.add_argument("--blink", default="000011",
                   help="lamp on/off per frame, 1 = on (default 000011; other pieces use other phases)")
    p.add_argument("--seed", type=int, default=5, help="which windows are lit")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=15.0)
M = kit.material
rnd = random.Random(args.seed)
lit = 0.0 if args.dark else args.lit

R = args.radius
RING = 1.0                                   # ring wall height
SEG = 48

# Ring wall
ap.cylinder("ring", M, "concrete_dirty", (0.0, 0.0, 0.0), R + 0.15, RING, SEG)


def dome_profile(r, steps=12, top=0.06):
    """Quarter circle from the ring wall to a small cap at the top."""
    pts = []
    for i in range(steps + 1):
        a = (math.pi / 2) * i / steps
        pts.append((max(r * math.cos(a), r * top), RING + r * math.sin(a) * 0.82))   # slightly flattened
    return pts


bm = bmesh.new()
sk.lathe(bm, dome_profile(R), SEG, cap_top=True)
dome = sk.mesh_object("dome", bm, M("frame_brace"), smooth_quads=True)


def dome_point(az, el, r=R, out=0.0):
    """Point on the dome at azimuth az (0 = front, -Y) and elevation el (radians), `out` above the surface."""
    rr = r + out
    return (rr * math.cos(el) * math.sin(az), -rr * math.cos(el) * math.cos(az), RING + rr * math.sin(el) * 0.82)


# Panel seams: meridians and two rings
seams = bmesh.new()
for k in range(12):
    az = 2 * math.pi * k / 12
    prev = None
    for i in range(13):
        p = dome_point(az, (math.pi / 2) * i / 12 * 0.96, out=0.03)
        if prev:
            sk.strut(seams, prev, p, 0.05, 5)
        prev = p
for el in (math.radians(25), math.radians(52)):
    prev = None
    for k in range(SEG + 1):
        p = dome_point(2 * math.pi * k / SEG, el, out=0.03)
        if prev:
            sk.strut(seams, prev, p, 0.05, 5)
        prev = p
sk.mesh_object("seams", seams, M("frame_main"))

# Windows around the dome, facing outwards (skip the airlock at the front)
EL = math.radians(14)
for k in range(args.windows):
    az = 2 * math.pi * (k + 0.5) / args.windows
    if abs(math.remainder(az, 2 * math.pi)) < math.radians(22):
        continue
    on = rnd.random() < lit
    loc = dome_point(az, EL, out=0.02)
    rot = (-EL, 0.0, az)
    sk.box_object("win_frame_%d" % k, loc, (1.3, 0.12, 0.85), M("frame_main"), rotation=rot)
    loc2 = dome_point(az, EL, out=0.09)
    sk.box_object("win_%d" % k, loc2, (1.0, 0.06, 0.6),
                  M("window_lit_warm") if on else M("glass_dark"), rotation=rot)

# Airlock tube at the front
AR = 1.15
tube = ap.cylinder("airlock", M, "pad_metal", (0.0, -R + 0.8, AR + 0.2), AR, 2.6, 32)
tube.rotation_euler = (math.pi / 2, 0.0, 0.0)            # local Z -> -Y
end_y = -R + 0.8 - 2.6
sk.box_object("airlock_base", (0.0, end_y + 1.3, 0.25), (1.6, 2.6, 0.5), M("concrete_dirty"))
ap.door("airlock_door", M, (0.0, end_y, 0.45), (1.1, 1.75), frame_role="hazard_paint")
for i, y in enumerate((end_y + 0.33, end_y + 1.5)):
    band = ap.cylinder("airlock_band_%d" % i, M, "frame_main", (0.0, y, AR + 0.2), AR + 0.07, 0.18, 32)
    band.rotation_euler = (math.pi / 2, 0.0, 0.0)

# Mast on top
mx, my = R * 0.25, R * 0.2
mz = RING + 0.82 * math.sqrt(R * R - mx * mx - my * my) - 0.1      # just inside the dome surface
mast_light = ap.mast("mast", M, (mx, my, mz), 3.2, dark=args.dark)
life = [] if args.dark else [ap.blink(mast_light, M, args.blink)]

kit.render_axo_animated(args.name, args.frames if life else 1, lambda i: [f(i, args.frames) for f in life], period=args.period,
                        yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
