"""
shield_power_arcs - the blue sparks inside the shield power unit's glass.

Plain numpy, no Blender, so it can be tried on its own. Each frame is one or
more waveforms between the electrodes: a mix of wavelengths chosen by the
frame's seed, a few short sparks off them, and a blue glow, in the look of the
Leapfrog's shield (data/images/shield_exciter.png): thin white lines in deep
blue. shield_power.py lays the result over its renders.

Images are arrays with row 0 at the top, values 0..1.
"""

import math
import random

import numpy as np


def box_blur(a, r):
    """Box blur of a 2D array with radius r in pixels (edges clamped)."""
    r = int(r)
    if r <= 0:
        return a
    for axis in (0, 1):
        pad = [(0, 0), (0, 0)]
        pad[axis] = (r + 1, r)
        c = np.cumsum(np.pad(a, pad, mode="edge"), axis=axis)
        hi = np.take(c, range(2 * r + 1, c.shape[axis]), axis=axis)
        lo = np.take(c, range(0, c.shape[axis] - 2 * r - 1), axis=axis)
        a = (hi - lo) / (2 * r + 1)
    return a


def soft_blur(a, sigma):
    """About a Gaussian: three box blurs."""
    r = int(round(max(sigma, 0.0) * 0.87))
    for _ in range(3):
        a = box_blur(a, r)
    return a


def _smooth_noise(rng, n, step):
    points = [rng.uniform(-1.0, 1.0) for _ in range(n // step + 3)]
    return np.interp(np.arange(n), np.arange(len(points)) * step, points)


def _plot(image, xs, ys, value):
    """Marks a dense polyline in image (keeps the brighter value)."""
    h, w = image.shape
    n = max(2, int(np.abs(np.diff(xs)).sum() + np.abs(np.diff(ys)).sum()) * 2)
    t = np.linspace(0.0, len(xs) - 1.0, n)
    px = np.interp(t, np.arange(len(xs)), xs).astype(int)
    py = np.interp(t, np.arange(len(ys)), ys).astype(int)
    keep = (px >= 0) & (px < w) & (py >= 0) & (py < h)
    image[py[keep], px[keep]] = np.maximum(image[py[keep], px[keep]], value)


def _waveform(rng, x0, x1, cy, wavelengths, amplitude, jag, ss):
    """Points of one waveform, x0..x1 in pixels (at ss times the size)."""
    n = int((x1 - x0) * ss)
    t = np.linspace(0.0, 1.0, n)
    px = x0 + (x1 - x0) * t
    env = np.sin(math.pi * t) ** 0.55             # pinned at the electrodes
    v = np.zeros(n)
    for wavelength, weight in wavelengths:
        v += weight * np.sin(2.0 * math.pi * (px - x0) / wavelength + rng.uniform(0.0, 6.3))
    v /= sum(weight for _, weight in wavelengths)
    v = v * amplitude * env + _smooth_noise(rng, n, max(2, int(0.25 * ss * amplitude))) * jag * env
    return px * ss, (cy + v) * ss


def frame_layers(w, h, seed, x0, x1, cy, height, ppu):
    """
    One frame's sparks in an image of w x h pixels: (glow, core), each h x w.
    x0, x1: the electrode tips in pixels; cy: the axis; height: the largest
    swing from the axis in pixels; ppu: pixels per unit, the scale of the lines.
    """
    rng = random.Random(seed)
    ss = 2
    core = np.zeros((h * ss, w * ss), np.float32)
    glow = np.zeros((h * ss, w * ss), np.float32)
    s = ppu / 20.0

    arcs = rng.choice([1, 2, 2, 3])
    base = rng.choice([26, 38, 54, 76, 110]) * s
    last = None
    for i in range(arcs):
        wavelength = base * rng.uniform(0.7, 1.5) * (1.0 if i == 0 else rng.choice([0.5, 0.33, 1.6]))
        mix = [(wavelength, 1.0), (wavelength * rng.uniform(0.28, 0.42), rng.uniform(0.2, 0.5))]
        xs, ys = _waveform(rng, x0, x1, cy, mix, height * rng.uniform(0.3, 0.85), rng.uniform(1.5, 4.0) * s, ss)
        _plot(core, xs, ys, 1.0 if i == 0 else 0.75)
        _plot(glow, xs, ys, 1.0)
        last = (xs, ys)

    # Short sparks off the last waveform
    xs, ys = last
    for _ in range(rng.randint(3, 6)):
        k = rng.randint(len(xs) // 8, len(xs) - len(xs) // 8 - 1)
        px, py = [xs[k]], [ys[k]]
        a = rng.uniform(0.0, 6.3)
        for _ in range(rng.randint(2, 4)):
            a += rng.uniform(-0.9, 0.9)
            d = rng.uniform(5.0, 12.0) * s * ss
            px.append(px[-1] + math.cos(a) * d)
            py.append(py[-1] + math.sin(a) * d * 0.8)
        _plot(core, np.array(px), np.array(py), 0.7)
        _plot(glow, np.array(px), np.array(py), 0.75)

    line = soft_blur(core, 1.0 * s)
    line = np.clip(line / max(1e-6, float(np.percentile(line[line > 0.01], 92))), 0.0, 1.0)
    wide = soft_blur(glow, 14.0 * s)
    near = soft_blur(glow, 4.5 * s)
    light = np.clip(wide * 14.0 + near * 5.0, 0.0, 1.6)

    def down(a):
        return a.reshape(h, ss, w, ss).mean(axis=(1, 3))

    return down(light), down(line)


def light_level(glow):
    """How much of the blue light on the caps a frame's sparks give, 0..1."""
    return min(1.0, 0.3 + 1.8 * float(glow.mean()))


def glass_mask(w, h, x0, x1, cy, radius, straps=()):
    """
    Where sparks may show: the glass between x0 and x1 (pixels), soft towards
    the walls; straps: (x, half width) of bands in front of the glass, which
    the sparks are behind.
    """
    xx = np.arange(w)[None, :]
    yy = np.arange(h)[:, None]
    n = np.clip((yy - cy) / float(radius), -1.0, 1.0)
    across = np.sqrt(1.0 - n * n)
    mask = ((xx >= x0) & (xx < x1) & (np.abs(yy - cy) < radius)).astype(np.float32)
    fade = 0.35 + 0.65 * across
    for x, half in straps:
        mask = mask * np.where(np.abs(xx - x) < half, 0.2, 1.0)
    return mask, fade


def compose(base, lit, glow, line, level, mask, fade):
    """
    One finished frame. base: the render with the sparks' light off (h x w x 4);
    lit: with it on; level: how much of that light this frame has. Screen
    blend, so the sparks stay inside 0..1.
    """
    out = base.copy()
    out[..., :3] = base[..., :3] + (lit[..., :3] - base[..., :3]) * level
    add = np.zeros(base.shape[:2] + (3,), np.float32)
    g = glow * fade * mask
    add[..., 0] = g * 0.03 + line * line * mask * 0.95
    add[..., 1] = g * 0.10 + line * line * mask * 0.97
    add[..., 2] = g * 0.95 + line * mask
    out[..., :3] = 1.0 - (1.0 - np.clip(out[..., :3], 0.0, 1.0)) * (1.0 - np.clip(add, 0.0, 1.0))
    return out
