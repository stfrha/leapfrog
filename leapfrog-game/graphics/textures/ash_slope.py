"""Photo-style, seamless ash-covered hillside texture (no Blender).

The look: rounded slopes under a blanket of pale ash, dark rough rock
showing through on crests and steps, soft hazy light, seen from far away
and low down, so nearer mounds overlap the ones behind them.

How:
  1. Terrain heightfield h(x, d) on a grid that is periodic in x and in depth d.
     The real surface is z = h + slope * d: a hillside rising away from us.
  2. Rock outcrops (mask + rough blocky height) on crests; ash elsewhere.
  3. Shading on the terrain: hazy sun, sky light, cavity AO, cast shadows.
  4. A "voxel space" render (as in Comanche): for each image column, march
     front to back and keep what rises above everything in front. Because the
     terrain is periodic and the slope is constant, the image is periodic too,
     so the PNG tiles in x and y.

Output: data/images/<world>/<name>.png (default data/images/mining/ash_slope.png),
registered in data/<res> (default res_mining.xml) in its own repeat atlas
(clamp2edge="false"; the size must be power-of-two) with ppu, using the sprite
kit's project_files.py. An existing entry is kept; only its ppu is updated.
Names containing "_test" are written but never registered (like the kit).

Run from anywhere:
    python ash_slope.py                          # ash_slope, 1024 x 1024
    python ash_slope.py --name ash_slope_test --ss 1    # quick draft, not registered
    python ash_slope.py --name ash_slope_rocky --rock 0.15 --seed 7

As a polygon fill: textureMeterWidth = width / ppu, textureMeterHeight = height / ppu
(printed after each run).
"""
import os
import sys
import argparse
import numpy as np
from scipy.ndimage import gaussian_filter
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("--w", type=int, default=1024)
ap.add_argument("--h", type=int, default=1024)
ap.add_argument("--seed", type=int, default=4)
ap.add_argument("--slope", type=float, default=0.42, help="mean gradient of the hillside")
ap.add_argument("--elev", type=float, default=4.0, help="camera looks down this many degrees")
ap.add_argument("--mounds", type=float, default=26.0, help="height of the big mounds (px)")
ap.add_argument("--rock", type=float, default=0.09, help="share of the surface that is outcrop")
ap.add_argument("--rough", type=float, default=0.9, help="roughness of the rock")
ap.add_argument("--sun", type=float, nargs=3, default=[0.6, -0.35, 0.7],
                help="light dir x(right) d(toward camera is +) z(up)")
ap.add_argument("--ash", default="aaa39d", help="ash colour rrggbb")
ap.add_argument("--rockcol", default="4a4440", help="rock colour rrggbb")
ap.add_argument("--ss", type=int, default=2, help="supersampling")
ap.add_argument("--debug", action="store_true")
ap.add_argument("--name", default="ash_slope", help="texture name (file <name>.png)")
ap.add_argument("--world", default="mining", help="images go to data/images/<world>/")
ap.add_argument("--res", default="res_mining.xml", help="resource file in data/ to register in")
ap.add_argument("--ppu", type=float, default=10,
                help="px per game unit written to the resource entry (sets the polygon's meter size)")
ap.add_argument("--no-register", action="store_true", help="write the PNG but leave the resource file alone")
ap.add_argument("--out", default=None, help="write the PNG here instead (never registered)")
a = ap.parse_args()

ss = a.ss
NX, NY = a.w * ss, a.h * ss
e = np.radians(a.elev)
fore = np.sin(e) + a.slope * np.cos(e)          # screen rows per unit of depth
dd = 0.5                                         # depth step (grid px)
ND1 = int(round(a.h / (fore * dd)))               # base grid; supersampling refines the same terrain
ND = ND1 * ss
dd = NY / (fore * ND)                            # exact, so one terrain period = NY rows
rng = np.random.default_rng(a.seed)
print(f"terrain grid {ND} x {NX}, depth step {dd:.3f}")

# frequency grid in cycles per 512 output px
FX = np.fft.fftfreq(a.w, d=1.0)[None, :] * 512
FD = np.fft.fftfreq(ND1, d=dd)[:, None] * 512


def fbm(beta, sx=1.0, sd=1.0, lo=1.0, hi=None, seed_off=0):
    """Periodic power-law noise; sx/sd < 1 make features longer along that axis."""
    r = np.random.default_rng(a.seed * 101 + seed_off)
    fx = FX / sx; fd = FD / sd
    f = np.sqrt(fx * fx + fd * fd)
    f[0, 0] = 1
    amp = 1.0 / np.maximum(f, lo) ** beta
    if hi:
        amp = amp * np.exp(-(f / hi) ** 2)
    amp[0, 0] = 0
    spec = (r.normal(size=(ND1, a.w)) + 1j * r.normal(size=(ND1, a.w))) * amp
    if ss > 1:   # zero-pad the spectrum: same terrain, finer grid
        big = np.zeros((ND, NX), complex)
        ky = np.fft.fftfreq(ND1, 1 / ND1).astype(int); kx = np.fft.fftfreq(a.w, 1 / a.w).astype(int)
        big[np.ix_(ky % ND, kx % NX)] = spec
        spec = big
    z = np.real(np.fft.ifft2(spec))
    return (z - z.mean()) / z.std()


def blur(x, s):
    """Gaussian blur, s in output px."""
    return gaussian_filter(x, (s * ss / dd, s * ss), mode="wrap")


def smooth(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1)
    return t * t * (3 - 2 * t)


P = ss  # output px -> grid units (heights are in grid units)

# ---------------------------------------------------------------- 1. terrain
# big rounded mounds, wider than deep, with some sharper crests (ridged term)
h = a.mounds * fbm(3.0, sx=0.7, lo=1.2, seed_off=0)
rid = 1 - np.abs(fbm(2.6, sx=0.6, lo=2, hi=30, seed_off=1))
h += 0.22 * a.mounds * blur(rid - rid.mean(), 6)
h += 0.10 * a.mounds * fbm(2.8, sx=0.6, lo=3, hi=25, seed_off=2)
h = h * P

# ---------------------------------------------------------------- 2. outcrops
conv = blur(h, 6) - blur(h, 30)
patch = 0.9 * conv / conv.std() + 0.8 * fbm(2.4, sx=0.4, lo=2, seed_off=3) + 0.5 * rid
t = np.quantile(patch, 1 - a.rock)
rock = smooth(t - 0.2, t + 0.5, patch + 0.25 * fbm(1.5, lo=15, hi=120, seed_off=4))

blocks = 1 - np.abs(fbm(1.9, sx=0.7, lo=6, hi=90, seed_off=5))          # jagged crests
blocks = blocks ** 2 + 0.3 * (1 - np.abs(fbm(1.6, lo=15, hi=120, seed_off=6)))
blocks += 0.3 * np.abs(fbm(1.5, lo=20, hi=150, seed_off=7))
h += P * (2.0 * rock + 1.8 * a.rough * rock * blocks)

# ash blanket: gentle hummocks, drapes over everything
h += P * 0.12 * fbm(2.4, sx=0.5, lo=8, hi=35, seed_off=8) * (1 - rock)
h = (1 - 0.5 * (1 - rock)) * h + 0.5 * (1 - rock) * blur(h, 1.0)

# ---------------------------------------------------------------- 3. shading
hx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) / 2
hd = (np.roll(h, -1, 0) - np.roll(h, 1, 0)) / (2 * dd)
n = np.stack([-hx, -(hd + a.slope), np.ones_like(h)])      # surface z = h + slope*d
n /= np.linalg.norm(n, axis=0)
sun = np.array([a.sun[0], -a.sun[1], a.sun[2]], float)   # option's d is toward camera
sun /= np.linalg.norm(sun)
lam = np.clip(np.tensordot(sun, n, 1), 0, 1)

shadow = np.ones_like(h)
hor = np.hypot(sun[0], sun[1]); tan_s = sun[2] / hor
D = np.arange(ND)[:, None] * dd
z = h + a.slope * D
for k in range(1, 40):
    L = k * 1.5 * ss
    ix, idd = int(round(sun[0] / hor * L)), int(round(sun[1] / hor * L / dd))
    zk = np.roll(np.roll(h, -idd, 0), -ix, 1) + a.slope * (D + idd * dd)
    shadow = np.minimum(shadow, np.clip(1 - ((zk - z) / L - tan_s) * 1.5, 0, 1))
shadow = 0.3 + 0.7 * (0.6 * blur(shadow, 0.7) + 0.4 * blur(shadow, 3))   # hazy, soft
cav = h - blur(h, 4)
ao = np.clip(0.88 + cav / (cav.std() * 5) * (1 + 1.5 * rock), 0.35, 1.08)
sky = 0.6 + 0.4 * n[2]


def rgb(s):
    return np.array([int(s[i:i + 2], 16) for i in (0, 2, 4)]) / 255.0


ash_c, rock_c = rgb(a.ash), rgb(a.rockcol)
near_rock = np.clip(blur(rock, 3) * 1.5, 0, 1)
grit = smooth(1.4, 2.8, fbm(0.4, lo=200, seed_off=10)) * (0.3 + 1.6 * near_rock)
ash_alb = ash_c * (1 + 0.06 * fbm(2.0, lo=2, seed_off=11) + 0.03 * fbm(0.8, lo=80, seed_off=12))[..., None]
ash_alb = ash_alb * (1 - 0.15 * near_rock * (1 - rock))[..., None]
rock_alb = rock_c * (1 + 0.18 * fbm(1.0, lo=25, seed_off=14) + 0.1 * fbm(2.0, lo=4, seed_off=15))[..., None]
dust = np.clip(n[2] * 2.2 - 1.3, 0, 1) * 0.35               # ash settles on flat-topped rock
rock_alb = rock_alb * (1 - dust[..., None]) + ash_c * 0.92 * dust[..., None]
alb = ash_alb * (1 - rock[..., None]) + rock_alb * rock[..., None]
alb = alb * (1 - 0.4 * grit[..., None] * (1 - rock[..., None]))

sun_col = np.array([1.0, 0.97, 0.92])
sky_col = np.array([0.70, 0.72, 0.80]) * 0.75
light = (lam * shadow)[..., None] * sun_col + (sky * ao)[..., None] * sky_col
col = alb * light * ao[..., None] ** 0.4
col = col / (1 + col * 0.2)

# ---------------------------------------------------------------- 4. render
K = 4                                              # depth periods marched
dsteps = np.arange(K * ND)
ramp = (dsteps * dd * (np.sin(e) + a.slope * np.cos(e))).astype(np.float32)[:, None]
hc = (h * np.cos(e)).astype(np.float32)
u0 = None
img = np.empty((NY, NX, 3), np.float32)
for c0 in range(0, NX, 128):                       # column chunks keep memory low
    M = np.maximum.accumulate(hc[dsteps % ND, c0:c0 + 128] + ramp, axis=0)
    if u0 is None:
        u0 = float(np.cos(e) * h.max()) + 1.2 * NY  # window well behind the nearest occluders
        rows = u0 + NY - 0.5 - np.arange(NY)        # image top = highest u
    for j in range(M.shape[1]):
        img[:, c0 + j] = col[np.searchsorted(M[:, j], rows) % ND, c0 + j]
img = np.clip(img, 0, 1) ** (1 / 1.12)
if ss > 1:
    img = img.reshape(a.h, ss, a.w, ss, 3).mean(axis=(1, 3))
out = (img * 255 + rng.uniform(-0.5, 0.5, img.shape)).clip(0, 255).astype(np.uint8)
# ---------------------------------------------------------------- 5. output
GAME_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # leapfrog-game
folder = "images/" + a.world
png = a.out or os.path.join(GAME_DIR, "data", *folder.split("/"), a.name + ".png")
os.makedirs(os.path.dirname(os.path.abspath(png)), exist_ok=True)
Image.fromarray(out).save(png)
print("wrote %s" % png)

if a.debug:
    def g(v): v = (v - v.min()) / (np.ptp(v) + 1e-9); return (v * 255).astype(np.uint8)
    Image.fromarray(np.hstack([g(h), g(light.mean(2)), g(rock)])).resize((1536, 512)).save(os.path.splitext(png)[0] + "_dbg.png")

o = out.astype(float)
print(f"{a.w}x{a.h} rock {rock.mean():.2f}  seam x: wrap {np.abs(o[:, 0] - o[:, -1]).mean():.2f} vs inside "
      f"{np.abs(np.diff(o, axis=1)).mean():.2f}; y: wrap {np.abs(o[0] - o[-1]).mean():.2f} "
      f"vs inside {np.abs(np.diff(o, axis=0)).mean():.2f}")

# ---------------------------------------------------------------- 6. register
pow2 = lambda v: v > 0 and v & (v - 1) == 0
print("polygon fill: textureMeterWidth %g, textureMeterHeight %g (ppu %g)"
      % (a.w / a.ppu, a.h / a.ppu, a.ppu))
if a.out or a.no_register or "_test" in a.name:
    print("not registered (%s)" % ("--out" if a.out else "--no-register" if a.no_register else "test name"))
elif not (pow2(a.w) and pow2(a.h)):
    print("not registered: %d x %d is not power-of-two, so it can't repeat on iOS" % (a.w, a.h))
else:
    sys.path.insert(0, os.path.join(GAME_DIR, "models", "sprites"))
    import project_files as pf
    fn = a.name + ".png"
    ppu = ("%g" % a.ppu)
    if not pf.register_texture(a.res, fn, attrs='ppu="%s"' % ppu, folder=folder, repeat=True):
        pf.set_image_attr(fn, "ppu", ppu)
        res, where = pf.find_texture(fn)
        if res and where != folder:
            print("note: %s is registered from data/%s in %s, not data/%s" % (fn, where, res, folder))

