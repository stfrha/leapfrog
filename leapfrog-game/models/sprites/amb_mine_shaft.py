"""
amb_mine_shaft - ambient piece for a distant mine site: an open shaft in the
ground with a cantilever hoist over it, the 0.4 parallax layer.

An oval hole with a concrete collar and a light fence round it, on a ring of
trodden dirt. Its rock walls go down into the dark (a black floor far below).
To the right a braced steel tower on a concrete pad carries a truss boom out
over the hole, held by stays from a king post; a winch on the boom pulls the
rope over a sheave at the tip and down to a kibble (a bucket) loaded with
ore, just coming up out of the hole. A spoil heap behind the hole on the
left.

Signs of life: two strings of lamps on the far wall of the shaft run
downwards, one lamp at a time (a chase), the right string half a loop after
the left; the beacon on the king post blinks. The lamps light the rock round
them. --all-blink makes all lamps blink together instead, --dark turns
everything off (no animation), --still renders one image with all lamps on.

The camera looks only 12 degrees down, so it sees about 5 units down the far
wall; the lamps sit in that band. The hole is longer front to back (--hole-y)
than across (--hole-x) so more of the far wall shows.

The ground's shadow catcher gets a hole of the shaft's outline (the object
marked kit_catcher_hole), otherwise it would cover the hole.

Like the other mine pieces: 14 px/unit, built at natural size (1 unit = 1 m)
and scaled x2, orthographic 3/4 view (render_axo), same camera.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P amb_mine_shaft.py -- --samples 16 --name amb_mine_shaft_test --still
    blender -b --factory-startup -P amb_mine_shaft.py
For a more distant layer (0.6), render with --ppu 10.
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import sprite_kit as sk           # noqa: E402
import amb_parts as ap            # noqa: E402
import amb_mine_parts as mp       # noqa: E402


def options(p):
    p.add_argument("--name", default="amb_mine_shaft", help="texture name (default amb_mine_shaft)")
    p.add_argument("--hole-x", type=float, default=9.0, help="half width of the hole across (default 9)")
    p.add_argument("--hole-y", type=float, default=13.0, help="half length of the hole front to back (default 13)")
    p.add_argument("--height", type=float, default=11.0, help="height of the hoist tower (default 11)")
    p.add_argument("--bucket-z", type=float, default=-0.5,
                   help="bottom of the bucket; 0 = ground, lower is down the hole (default -0.5)")
    p.add_argument("--lamps", type=int, default=4, help="visible lamps per string (default 4)")
    p.add_argument("--lamp-watts", type=float, default=25.0,
                   help="light from each lamp onto the rock, at natural size (default 25)")
    p.add_argument("--all-blink", action="store_true", help="all lamps blink together instead of running down")
    p.add_argument("--no-fence", action="store_true", help="no fence round the hole")
    p.add_argument("--no-heap", action="store_true", help="no spoil heap")
    p.add_argument("--dark", action="store_true", help="lamps and beacon off (no animation)")
    p.add_argument("--frames", type=int, default=6, help="animation frames (default 6; --still for one image)")
    p.add_argument("--period", type=float, default=2.4, help="seconds per animation loop (default 2.4)")
    p.add_argument("--blink", default="110000",
                   help="beacon on/off per frame, 1 = on (default 110000)")
    p.add_argument("--yaw", type=float, default=10.0, help="camera yaw, keep equal for all pieces (default 10)")
    p.add_argument("--pitch", type=float, default=12.0, help="camera pitch, keep equal for all pieces (default 12)")


args = sk.parse_args(options)
kit = sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14, scale=2.0, turn=0.0)
M = kit.material
rnd = random.Random(7)

A = args.hole_x          # hole half axes (x across, y front to back)
B = args.hole_y
LIP_W = 1.0              # concrete collar: width, top above ground, how far it reaches down the hole
LIP_Z = 0.45
COLLAR_DEPTH = 0.8
DEPTH = 30.0             # shaft floor, far out of sight
N = 120                  # segments round the oval


def oval(a, b, t):
    """Point on an oval with half axes a (x) and b (y); t = 0 at the front (-Y), increasing towards +X."""
    return a * math.sin(t), -b * math.cos(t)


def oval_ring(bm, a, b, z, n=N, offset=None):
    """A ring of n verts round an oval at height z; offset(t) widens it locally."""
    ring = []
    for i in range(n):
        t = 2.0 * math.pi * i / n
        d = offset(t) if offset else 0.0
        x, y = oval(a + d, b + d, t)
        ring.append(bm.verts.new((x, y, z)))
    return ring


def bridge(bm, r0, r1):
    """Quads between two rings of the same length."""
    n = len(r0)
    for i in range(n):
        j = (i + 1) % n
        bm.faces.new((r0[i], r0[j], r1[j], r1[i]))


def rock(t, z):
    """How far the rock wall stands back from the collar line at angle t and height z (z < 0)."""
    depth = -z - COLLAR_DEPTH
    rough = min(1.0, max(0.0, depth / 1.5))
    return 0.15 + rough * (0.22 * math.sin(3 * t + 1.3 + 0.35 * z) + 0.16 * math.sin(7 * t + 0.8 * z + 0.4)
                           + 0.10 * math.sin(13 * t + 2.1 - 0.6 * z) + 0.06 * math.sin(29 * t + 1.7 * z))


# Shaft --------------------------------------------------------------------------------------
# Rock walls from below the collar down to the floor, rough, with rings close
# together where the camera can see and further apart below.
wall = bmesh.new()
levels = [-COLLAR_DEPTH - 0.4 * k for k in range(16)] + [-8.0, -11.0, -15.0, -20.0, -DEPTH]
rings = [oval_ring(wall, A, B, z, offset=lambda t, z=z: rock(t, z)) for z in levels]
for r0, r1 in zip(rings, rings[1:]):
    bridge(wall, r1, r0)                       # facing into the hole
sk.mesh_object("shaft_wall", wall, M("ore_rock"), smooth_quads=True)

floor = bmesh.new()
floor_ring = oval_ring(floor, A, B, -DEPTH, offset=lambda t: rock(t, -DEPTH))
floor.faces.new(floor_ring)
sk.mesh_object("shaft_floor", floor, M("void"))

# Concrete collar: top, inner face down into the shaft, outer face down to the ground,
# and a ledge under it so the rock meets it
collar = bmesh.new()
c_in_top = oval_ring(collar, A, B, LIP_Z)
c_in_bot = oval_ring(collar, A, B, -COLLAR_DEPTH)
c_ledge = oval_ring(collar, A + 0.15, B + 0.15, -COLLAR_DEPTH)
c_out_top = oval_ring(collar, A + LIP_W, B + LIP_W, LIP_Z)
c_out_bot = oval_ring(collar, A + LIP_W, B + LIP_W, 0.0)
bridge(collar, c_in_top, c_out_top)            # top
bridge(collar, c_in_bot, c_in_top)             # inner face
bridge(collar, c_ledge, c_in_bot)              # ledge
bridge(collar, c_out_top, c_out_bot)           # outer face
sk.mesh_object("collar", collar, M("concrete_dirty"))

# The shadow catcher's hole: the opening, a little under the collar
hole = bmesh.new()
hole.faces.new(oval_ring(hole, A + 0.4, B + 0.4, 0.0, n=72))
hole_ob = sk.mesh_object("catcher_hole", hole)
hole_ob.hide_render = True
hole_ob["kit_catcher_hole"] = True
hole_ob["kit_no_frame"] = True

# Ring of trodden dirt round the collar, slightly mounded, with an uneven outer edge
phases = [rnd.uniform(0, 2 * math.pi) for _ in range(3)]


def apron_w(t):
    return 2.2 + 0.5 * math.sin(2 * t + phases[0]) + 0.35 * math.sin(5 * t + phases[1]) \
        + 0.2 * math.sin(11 * t + phases[2])


apron = bmesh.new()
a0 = oval_ring(apron, A + LIP_W - 0.05, B + LIP_W - 0.05, 0.12)
a1 = oval_ring(apron, A + LIP_W, B + LIP_W, 0.12, offset=lambda t: apron_w(t) * 0.45)
a1_z = [0.12 + 0.12 * (0.5 + 0.5 * math.sin(4 * (2 * math.pi * i / N) + 1.1)) for i in range(N)]
for v, z in zip(a1, a1_z):
    v.co.z = z
a2 = oval_ring(apron, A + LIP_W, B + LIP_W, 0.0, offset=apron_w)
bridge(apron, a0, a1)
bridge(apron, a1, a2)
sk.mesh_object("apron", apron, M("ground_dirt"))

# A few loose stones on the apron
for k in range(14):
    t = rnd.uniform(0, 2 * math.pi)
    d = LIP_W + rnd.uniform(0.4, 0.9) * apron_w(t)
    x, y = oval(A + d, B + d, t)
    s = rnd.uniform(0.25, 0.55)
    sk.box_object("stone_%d" % k, (x, y, 0.1 + s * 0.3), (s, s * rnd.uniform(0.7, 1.2), s * 0.6), M("ore_rock"),
                  rotation=(rnd.uniform(-0.3, 0.3), rnd.uniform(-0.3, 0.3), rnd.uniform(0, math.pi)),
                  bevel_width=0.06)

# Fence round the collar: posts, a top rail and a sagging middle rail
if not args.no_fence:
    fence = bmesh.new()
    rails = bmesh.new()
    perim = math.pi * (3 * (A + B) - math.sqrt((3 * A + B) * (A + 3 * B)))   # Ramanujan
    posts = max(8, int(perim / 2.8))
    tops = []
    for i in range(posts):
        t = 2.0 * math.pi * (i + 0.5) / posts
        x, y = oval(A + LIP_W * 0.6, B + LIP_W * 0.6, t)
        sk.strut(fence, (x, y, LIP_Z), (x, y, LIP_Z + 1.1), 0.05, 6)
        tops.append((x, y))
    for i in range(posts):
        (x0, y0), (x1, y1) = tops[i], tops[(i + 1) % posts]
        sk.strut(rails, (x0, y0, LIP_Z + 1.05), (x1, y1, LIP_Z + 1.05), 0.035, 5)
        xm, ym = (x0 + x1) / 2, (y0 + y1) / 2
        sk.strut(rails, (x0, y0, LIP_Z + 0.6), (xm, ym, LIP_Z + 0.48), 0.025, 5)
        sk.strut(rails, (xm, ym, LIP_Z + 0.48), (x1, y1, LIP_Z + 0.6), 0.025, 5)
    sk.mesh_object("fence_posts", fence, M("frame_main"))
    sk.mesh_object("fence_rails", rails, M("hazard_paint"))

# Hoist tower on its pad, right of the hole ------------------------------------------------------
H = args.height
TX = A + LIP_W + 2.2         # tower centre
TL = 1.4                     # half spread of the legs
PAD_Z = 0.35
sk.box_object("tower_pad", (TX + 1.2, 0.0, PAD_Z / 2), (6.4, 5.2, PAD_Z), M("concrete_dirty"), bevel_width=0.08)

corners = [(TX - TL, -TL), (TX + TL, -TL), (TX + TL, TL), (TX - TL, TL)]
legs = bmesh.new()
for cx, cy in corners:
    sk.strut(legs, (cx, cy, PAD_Z), (cx, cy, H), 0.16, 8)
    sk.box(legs, (cx, cy, PAD_Z + 0.08), (0.6, 0.6, 0.16))            # base plates
sk.mesh_object("tower_legs", legs, M("frame_main"))
brace = bmesh.new()
LEVELS = 4
for k in range(LEVELS):
    z0 = PAD_Z + (H - PAD_Z) * k / LEVELS
    z1 = PAD_Z + (H - PAD_Z) * (k + 1) / LEVELS
    for i in range(4):
        (ax, ay), (bx, by) = corners[i], corners[(i + 1) % 4]
        sk.strut(brace, (ax, ay, z0), (bx, by, z1), 0.06, 6)
        sk.strut(brace, (bx, by, z0), (ax, ay, z1), 0.06, 6)
        sk.strut(brace, (ax, ay, z1), (bx, by, z1), 0.08, 6)
sk.mesh_object("tower_bracing", brace, M("frame_brace"))
# Ladder up the right side
lad = bmesh.new()
for dy in (-0.3, 0.3):
    sk.strut(lad, (TX + TL + 0.35, dy, PAD_Z), (TX + TL + 0.35, dy, H + 0.2), 0.035, 5)
for k in range(1, int((H - PAD_Z) / 0.5)):
    z = PAD_Z + k * 0.5
    sk.strut(lad, (TX + TL + 0.35, -0.3, z), (TX + TL + 0.35, 0.3, z), 0.025, 5)
sk.mesh_object("ladder", lad, M("frame_brace"))

# Boom: a truss on top of the tower, from the counterweight end out over the hole
BX = 1.2                     # where the rope comes down (the bucket's centre)
SHEAVE_R = 0.55
BOOM_Z = H + 0.05
BOOM_H = 1.0
BACK_X = TX + 4.4
TIP_X = BX - 0.2
mp.truss("boom", M, (TIP_X, 0.0, BOOM_Z), (BACK_X, 0.0, BOOM_Z), 1.1, BOOM_H)    # p0 -> p1 along +X keeps it upright
sk.box_object("tower_cap", (TX, 0.0, H - 0.05), (2 * TL + 0.5, 2 * TL + 0.5, 0.2), M("pad_metal"))
sk.box_object("counterweight", (BACK_X - 0.9, 0.0, BOOM_Z - 0.75), (1.6, 1.5, 1.5), M("concrete_dirty"),
              bevel_width=0.05)
ap.railing("boom_rail", M, (TX - TL, -0.65, BOOM_Z + BOOM_H), (BACK_X, -0.65, BOOM_Z + BOOM_H), height=0.8)

# Winch on top of the boom over the tower: drum (axis along Y) and motor
WX = TX + 1.6
DRUM_R = 0.45
drum_z = BOOM_Z + BOOM_H + 0.15 + DRUM_R
drum = bmesh.new()
sk.strut(drum, (WX, -0.5, drum_z), (WX, 0.5, drum_z), DRUM_R, 20)
sk.strut(drum, (WX, -0.56, drum_z), (WX, -0.5, drum_z), DRUM_R + 0.12, 20)
sk.strut(drum, (WX, 0.5, drum_z), (WX, 0.56, drum_z), DRUM_R + 0.12, 20)
sk.mesh_object("winch_drum", drum, M("rusty_metal"), smooth_quads=True)
for dy in (-0.62, 0.62):
    sk.box_object("winch_cheek_%d" % (dy > 0), (WX, dy, BOOM_Z + BOOM_H + 0.4), (1.2, 0.08, 0.8), M("frame_main"))
sk.box_object("winch_motor", (WX + 1.3, 0.0, BOOM_Z + BOOM_H + 0.45), (1.2, 0.9, 0.8), M("machine_yellow"),
              bevel_width=0.05)
sk.box_object("winch_base", (WX + 0.6, 0.0, BOOM_Z + BOOM_H + 0.05), (2.6, 1.3, 0.1), M("pad_metal"))

# King post on the tower with stays to the tip and the back end
KP_Z = BOOM_Z + BOOM_H + 3.0
stays = bmesh.new()
post = bmesh.new()
for dy in (-0.45, 0.45):
    sk.strut(post, (TX - 0.4, dy, BOOM_Z + BOOM_H), (TX - 0.4, dy * 0.3, KP_Z), 0.1, 6)
    sk.strut(stays, (TX - 0.4, dy * 0.3, KP_Z - 0.1), (TIP_X + 0.6, dy, BOOM_Z + BOOM_H), 0.04, 5)
    sk.strut(stays, (TX - 0.4, dy * 0.3, KP_Z - 0.1), (BACK_X - 0.2, dy, BOOM_Z + BOOM_H), 0.04, 5)
sk.mesh_object("king_post", post, M("frame_main"))
sk.mesh_object("stays", stays, M("frame_brace"))
beacon = sk.box_object("beacon", (TX - 0.4, 0.0, KP_Z + 0.2), (0.3, 0.3, 0.25),
                       M("neon_dead") if args.dark else M("neon_amber"))

# Tip sheave in cheek plates, the rope's left side drops straight down at BX
SX = BX + SHEAVE_R
SZ = BOOM_Z + BOOM_H + 0.35
mp.sheave("tip_sheave", M, (SX, 0.0, SZ), SHEAVE_R, spokes=5, rim=0.08, spoke_r=0.04)
for dy in (-0.3, 0.3):
    sk.box_object("tip_cheek_%d" % (dy > 0), (SX, dy, SZ - 0.2), (1.3, 0.06, 1.1), M("frame_main"))

# Bucket (kibble) with its bail, hook block and a load of ore ---------------------------------------
BZ = args.bucket_z
KH = 2.0
kib = bmesh.new()
sk.lathe(kib, [(0.9, 0.0), (1.2, KH)], 32, cap_bottom=True)
sk.lathe(kib, [(1.25, KH - 0.15), (1.25, KH + 0.05)], 32)               # rim bead
k_ob = sk.mesh_object("kibble", kib, M("rusty_metal"))
k_ob.location = (BX, 0.0, BZ)
load = mp.heap("kibble_ore", M, (BX, 0.0, BZ + KH - 0.3), 1.12, 0.7, seed=5, rough=0.1)
HOOK_Z = BZ + KH + 1.7
bail = bmesh.new()
for sx in (-1, 1):
    sk.strut(bail, (BX + sx * 1.22, 0.0, BZ + KH - 0.3), (BX, 0.0, HOOK_Z), 0.05, 6)
    sk.box(bail, (BX + sx * 1.24, 0.0, BZ + KH - 0.3), (0.12, 0.3, 0.3))       # trunnions
sk.mesh_object("kibble_bail", bail, M("frame_main"))
sk.box_object("hook_block", (BX, 0.0, HOOK_Z + 0.3), (0.4, 0.35, 0.6), M("hazard_paint"), bevel_width=0.04)

rope = bmesh.new()
sk.strut(rope, (BX, 0.0, HOOK_Z + 0.6), (BX, 0.0, SZ), 0.045, 6)                       # down to the bucket
sk.strut(rope, (SX, 0.0, SZ + SHEAVE_R), (WX, 0.0, drum_z + DRUM_R), 0.045, 6)          # back to the winch
sk.mesh_object("rope", rope, M("frame_main"))

# Spoil heap behind the hole on the left ----------------------------------------------------------
if not args.no_heap:
    mp.heap("spoil", M, (-A * 0.55, B + LIP_W + 2.4, 0.0), 3.4, 2.6, seed=11, rough=0.14, scale=(1.3, 0.7, 1.0))

# Lamps down the far wall ---------------------------------------------------------------------
# Two strings on cables hung from the collar, on the far wall left and right
# of the bucket. Each lamp lights the rock round it (a point light in front of it).
LAMP_Z = [-1.3 - 1.0 * k for k in range(max(1, args.lamps))]


def wall_point(t, z, inset):
    """Point on the rock wall at angle t and height z, `inset` towards the middle of the hole."""
    d = rock(t, z) - inset
    x, y = oval(A + d, B + d, t)
    return x, y


strings = []
for side, x_at in (("L", -3.5), ("R", 4.2)):
    # Angle on the far side (t near pi) where the wall is at x = x_at
    t = math.pi - math.asin(max(-0.95, min(0.95, x_at / A)))
    cable = bmesh.new()
    pts = [(*wall_point(t, -COLLAR_DEPTH, 0.05), LIP_Z)]
    pts.append((*wall_point(t, -COLLAR_DEPTH, 0.05), -COLLAR_DEPTH))
    for z in LAMP_Z:
        pts.append((*wall_point(t, z, 0.08), z + 0.2))
    pts.append((*wall_point(t, LAMP_Z[-1] - 1.5, 0.08), LAMP_Z[-1] - 1.5))
    for p0, p1 in zip(pts, pts[1:]):
        sk.strut(cable, p0, p1, 0.03, 5)
    sk.mesh_object("lamp_cable_" + side, cable, M("frame_main"))

    lamps = []
    for k, z in enumerate(LAMP_Z):
        x, y = wall_point(t, z, 0.2)
        cx, cy = wall_point(t, z, 0.0)
        sk.box_object("lamp_bracket_%s%d" % (side, k), ((x + cx) / 2, (y + cy) / 2, z + 0.1), (0.12, 0.25, 0.1),
                      M("frame_main"))
        lamp = sk.box_object("lamp_%s%d" % (side, k), (x, y, z), (0.32, 0.26, 0.26),
                             M("neon_dead") if args.dark else M("neon_amber"))
        lx, ly = wall_point(t, z, 0.7)
        light = sk.point_light("lamp_light_%s%d" % (side, k), (lx, ly, z), color=(1.0, 0.55, 0.2),
                               energy=args.lamp_watts, radius=0.12)
        light.hide_render = args.dark
        lamps.append((lamp, light))
    strings.append(lamps)

# Animation -------------------------------------------------------------------------------
n = args.frames
on_mat = M("neon_amber")
off_mat = M("neon_dead")


def lamp_on(string, k, i):
    """Is lamp k of a string on in frame i?"""
    if args.all_blink:
        return i % 2 == 0
    phase = 0 if string == 0 else n // 2
    return (i - phase) % n == k                # one lamp at a time, running down; the rest of the loop dark


life = []
if not args.dark:
    life.append(ap.blink(beacon, M, args.blink))

    def lamps_frame(i, n_):
        for s, lamps in enumerate(strings):
            for k, (lamp, light) in enumerate(lamps):
                lit = (i is None) or lamp_on(s, k, i)
                lamp.data.materials[0] = on_mat if lit else off_mat
                light.hide_render = not lit

    life.append(lamps_frame)

kit.render_axo_animated(args.name, n if life else 1, lambda i: [f(i, n) for f in life], period=args.period,
                        yaw_deg=args.yaw, pitch_deg=args.pitch, ground_z=0.0)
