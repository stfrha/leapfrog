"""
stripper_mantis - the stripper seen from above: a small robot praying mantis.
A thin body (triangular head with glowing eyes, long neck, thorax, a long
segmented abdomen under folded wing covers) with its legs cut off above the
knees as stubby exhaust pipes, three a side, and two heavy raptorial arms: a
thick spiked femur in shell colour and a broad steel blade with a hook. Built as a salvage drone in faded construction yellow (the Baxters'
old trade); --shell picks another theme colour.

Two frames, stacked (texture "stripper", rows="2"): in each one arm stabs
forward while the other is folded back, swapped in the other frame, so
playing them makes the arms jab in turn. stripper.xml plays them slowly in
flight and fast when latched (stripper flyingAnimatePeriod /
latchedAnimatePeriod).

Body in data/stripper.xml: mainBody 8 x 4.2, nose to the left (noseAngle
180). Top-down ortho framed to the body: x -4 .. 4, z -2.1 .. 2.1, 20 px/unit
(160 x 84 per frame).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P stripper_mantis.py
    blender -b --factory-startup -P stripper_mantis.py -- --samples 16 --suffix _test
    blender -b --factory-startup -P stripper_mantis.py -- --still     (frame 0 only)
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import sprite_kit as sk           # noqa: E402
import axe_parts as ax            # noqa: E402


def options(p):
    p.add_argument("--suffix", default="", help="added to the texture name (e.g. _test: not registered)")
    p.add_argument("--shell", default="machine_yellow", help="theme material of the shell (default machine_yellow)")
    p.add_argument("--arm", type=float, default=2.0, help="arm thickness, times the first version's (default 2)")


args = sk.parse_args(options)
if not args.folder:
    args.folder = "space"
kit = sk.Kit(args, rig="ortho", res="res_space.xml")

W, H = 8.0, 4.2
shell = kit.material(args.shell)
dark = kit.material("frame_main")
blade_steel = ax.hone_steel("mantis_blade")
wing = kit.material("glass_dark")
eye = ax.emission("mantis_eye", (1.0, 0.08, 0.03), 18.0)
pipe_steel = kit.material("strap_steel")
exhaust = ax.emission("mantis_exhaust", (1.0, 0.45, 0.1), 6.0)
T = args.arm                     # arm thickness factor

# ---------------------------------------------------------------------------
# Body: head, neck, thorax, abdomen with bands, wing covers
# ---------------------------------------------------------------------------
head = [(-3.35, 0.0), (-2.95, 0.58), (-2.7, 0.62), (-2.55, 0.22), (-2.55, -0.22), (-2.7, -0.62), (-2.95, -0.58)]
ax.slab("head", head, -0.45, 0.15, kit.material("pad_metal"))      # dark, so the yellow arms stand out
ax.slab("jaw", [(-3.35, 0.0), (-3.1, 0.18), (-2.95, 0.0), (-3.1, -0.18)], -0.5, -0.4, dark)
for sz in (1, -1):
    ax.lathe_up("eye_%d" % sz, [(0.22, 0.0), (0.2, 0.12), (0.12, 0.22), (0.001, 0.26)], -2.82, sz * 0.5, -0.38, eye)
    ax.rod("antenna_%d" % sz, [(-3.2, sz * 0.2), (-3.6, sz * 0.75), (-3.75, sz * 1.5)], 0.045, -0.4, dark)
ax.lathe_x("neck", [(0.18, 0.0), (0.22, 0.8), (0.2, 1.6)], -2.6, shell, flatten=0.8)
ax.lathe_x("thorax", [(0.25, 0.0), (0.46, 0.35), (0.48, 0.8), (0.3, 1.25)], -1.0, shell, flatten=0.7)
ax.lathe_x("abdomen", [(0.3, 0.0), (0.6, 0.6), (0.7, 1.4), (0.6, 2.4), (0.4, 3.2), (0.1, 3.8)], 0.15, shell, flatten=0.6)
for k, x in enumerate((0.95, 1.65, 2.35, 3.05)):
    r = {0: 0.71, 1: 0.71, 2: 0.6, 3: 0.44}[k]
    ax.lathe_x("band_%d" % k, [(r, 0.0), (r, 0.12)], 0.15 + x - 0.06, dark, flatten=0.62)
for sz in (1, -1):
    ax.slab("wing_%d" % sz, [(-0.2, sz * 0.04), (0.7, sz * 0.58), (2.6, sz * 0.52), (3.3, sz * 0.12), (2.5, -sz * 0.06),
                              (0.6, -sz * 0.06)], -0.52 - (0.02 if sz > 0 else 0), -0.4, wing)
    ax.rod("wing_vein_%d" % sz, [(0.1, sz * 0.25), (1.6, sz * 0.42), (3.0, sz * 0.18)], 0.03, -0.55, dark)

# ---------------------------------------------------------------------------
# Legs cut off above the knees: stubby exhaust pipes, three a side
# ---------------------------------------------------------------------------
STUBS = (((-0.6, 0.32), (-0.4, 1.2)),      # (root, cut end) for the upper side
         ((0.15, 0.32), (0.7, 1.28)),
         ((1.1, 0.45), (1.75, 1.15)))
for sz in (1, -1):
    for k, ((rx, rz), (cx, cz)) in enumerate(STUBS):
        root, cut = (rx, sz * rz), (cx, sz * cz)
        dx, dz = cut[0] - root[0], cut[1] - root[1]
        L = math.hypot(dx, dz)
        ux, uz = dx / L, dz / L
        collar = (cut[0] - ux * 0.22, cut[1] - uz * 0.22)
        mouth = (cut[0] + ux * 0.08, cut[1] + uz * 0.08)
        ax.rod("pipe_%d_%d" % (sz, k), [root, collar], 0.14, -0.15, dark)
        ax.rod("pipe_collar_%d_%d" % (sz, k), [collar, cut], 0.21, -0.15, pipe_steel)
        ax.rod("pipe_mouth_%d_%d" % (sz, k), [cut, mouth], 0.13, -0.15, exhaust)

# ---------------------------------------------------------------------------
# Raptorial arms, one per side, in two poses each
# ---------------------------------------------------------------------------
POSES = {"stab": ((-3.0, 1.38), (-3.98, 0.6)),       # elbow, blade tip (z for the upper arm)
         "fold": ((-3.2, 1.3), (-1.9, 1.5))}         # cocked: the blade folded back along the femur
SHOULDER = (-2.35, 0.22)


def arm(sz, pose):
    (ex, ez), (tx, tz) = POSES[pose]
    s = (SHOULDER[0], sz * SHOULDER[1])
    e = (ex, sz * ez)
    t = (tx, sz * tz)
    obs = []
    # centred low enough that its top stays just under the head (-0.45)
    obs.append(ax.rod("arm_%d_%s_femur" % (sz, pose), [s, e], 0.17 * T, -0.43 + 0.17 * T, shell))
    # spikes along the femur's inner side
    dx, dz = e[0] - s[0], e[1] - s[1]
    L = math.hypot(dx, dz)
    ux, uz = dx / L, dz / L
    nx, nz = -uz, ux
    side = -1.0 if (nz * sz) > 0 else 1.0                  # towards the body's centre line
    for k in range(3):
        f = 0.3 + 0.2 * k
        bx, bz = s[0] + dx * f, s[1] + dz * f
        reach = 0.17 * T + 0.22 * T                      # out of the femur's side
        tip = (bx + side * nx * reach + ux * 0.12 * T, bz + side * nz * reach + uz * 0.12 * T)
        obs.append(sk.prism("arm_%d_%s_spike_%d" % (sz, pose, k),
                            [(bx - ux * 0.09 * T, bz - uz * 0.09 * T), (bx + ux * 0.09 * T, bz + uz * 0.09 * T), tip],
                            -0.4, 0.12, dark))
    # the blade: wide at the elbow, a point at the tip, a hook bent inwards
    bdx, bdz = t[0] - e[0], t[1] - e[1]
    bl = math.hypot(bdx, bdz)
    vx, vz = bdx / bl, bdz / bl
    px, pz = -vz, vx
    w = 0.18 * T
    blade = [(e[0] + px * w, e[1] + pz * w), (t[0] - px * w * 0.25, t[1] - pz * w * 0.25), (t[0], t[1]),
             (e[0] - px * w, e[1] - pz * w)]
    obs.append(sk.prism("arm_%d_%s_blade" % (sz, pose), blade, -0.42, 0.12, blade_steel))
    hook_side = -1.0 if (pz * sz) > 0 else 1.0
    hk = 0.32 * (1 + T) / 2
    hook = [(t[0] - vx * 0.3 * T, t[1] - vz * 0.3 * T), (t[0], t[1]),
            (t[0] - vx * 0.15 * T + hook_side * px * hk, t[1] - vz * 0.15 * T + hook_side * pz * hk)]
    obs.append(sk.prism("arm_%d_%s_hook" % (sz, pose), hook, -0.42, 0.12, blade_steel))
    obs.append(ax.stud("arm_%d_%s_elbow" % (sz, pose), e[0], e[1], -0.38, 0.15 * T, dark, height=0.14))
    return obs


ax.stud("shoulder_1", SHOULDER[0], SHOULDER[1], -0.3, 0.17 * T, dark, height=0.14)
ax.stud("shoulder_-1", SHOULDER[0], -SHOULDER[1], -0.3, 0.17 * T, dark, height=0.14)
arms = {(sz, pose): arm(sz, pose) for sz in (1, -1) for pose in ("stab", "fold")}


def set_frame(i):
    """Frame 0: the upper arm (+z) stabs, the lower is folded; frame 1 the other way round."""
    stabbing = 1 if i == 0 else -1
    for (sz, pose), obs in arms.items():
        show = (pose == "stab") == (sz == stabbing)
        for ob in obs:
            ob.hide_render = not show


box = (-W / 2, -H / 2, W / 2, H / 2)
name = "stripper" + args.suffix
set_frame(0)
if not args.no_render:
    if args.still:
        kit.render_ortho(name, box)
    else:
        paths = [kit.render_ortho(name + "_f0", box, frame_only=True)]
        set_frame(1)
        paths.append(kit.render_variant(name + "_f1"))
        if all(paths):
            kit.make_strip(paths, name)
