"""
gravel_layout - where the stones of a gravel texture go (no Blender needed).

Stones are placed in the image plane (x right, z up, in game units) by dart
throwing: a stone is kept if it doesn't overlap its neighbours by more than
a set amount. Distances can wrap around in x and/or z, so a layout made for
a W x H tile continues seamlessly into the next tile. The Blender script
then builds the stones and repeats the whole tile around itself, so stones
on the tile edges, their shadows and their light are the same on both sides.

Kept free of bpy so the layout can be checked with plain Python.
"""

import math
import random


class Stone:
    __slots__ = ("x", "z", "r", "seed")

    def __init__(self, x, z, r, seed):
        self.x = x          # centre in the image plane
        self.z = z
        self.r = r          # radius in the image plane (the stone's largest)
        self.seed = seed    # for the stone's own shape and colour


# Passes: (radius as a fraction of the mean stone diameter, low, high),
# overlap allowed (distance >= (r1 + r2) * keep), tries per unit area.
# Big stones first, then medium ones in the gaps, then fines.
PASSES = [
    (0.40, 0.62, 0.88, 3.0),
    (0.26, 0.42, 0.80, 6.0),
    (0.12, 0.24, 0.70, 10.0),
]


def place_stones(width, height, size, seed=1, wrap_x=True, wrap_z=True, accept=None, passes=PASSES):
    """
    Stones in [0, width) x [0, height), mean diameter about `size`.

    accept(x, z, r) -> bool can reject positions (e.g. above a border's
    rugged top). With wrap_x / wrap_z the layout tiles in that direction.
    Returns a list of Stone.
    """
    rng = random.Random(seed)
    stones = []
    r_max = max(p[1] for p in passes) * size
    cell = 2.0 * r_max
    nx = max(1, int(width // cell))
    nz = max(1, int(height // cell))
    cw = width / nx
    ch = height / nz
    grid = {}

    def cell_of(x, z):
        return int(x // cw) % nx if wrap_x else min(nx - 1, max(0, int(x // cw))), \
               int(z // ch) % nz if wrap_z else min(nz - 1, max(0, int(z // ch)))

    def neighbours(x, z):
        ix, iz = cell_of(x, z)
        for dx in (-1, 0, 1):
            for dz in (-1, 0, 1):
                jx, jz = ix + dx, iz + dz
                if wrap_x:
                    jx %= nx
                elif not 0 <= jx < nx:
                    continue
                if wrap_z:
                    jz %= nz
                elif not 0 <= jz < nz:
                    continue
                for s in grid.get((jx, jz), ()):
                    yield s

    def distance(ax, az, bx, bz):
        dx = abs(ax - bx)
        dz = abs(az - bz)
        if wrap_x:
            dx = min(dx, width - dx)
        if wrap_z:
            dz = min(dz, height - dz)
        return math.hypot(dx, dz)

    for lo, hi, keep, tries_per_area in passes:
        tries = int(width * height * tries_per_area / (size * size))
        for _ in range(tries):
            r = rng.uniform(lo, hi) * size
            x = rng.uniform(0.0, width)
            z = rng.uniform(0.0, height)
            if accept is not None and not accept(x, z, r):
                continue
            ok = True
            for s in neighbours(x, z):
                if distance(x, z, s.x, s.z) < (r + s.r) * keep:
                    ok = False
                    break
            if not ok:
                continue
            s = Stone(x, z, r, rng.randrange(1 << 30))
            stones.append(s)
            grid.setdefault(cell_of(x, z), []).append(s)
    return stones


def periodic_profile(width, amplitude, seed=2, terms=24, min_wave=None, power=1.0):
    """
    A random wavy line h(x) that repeats every `width`, between 0 and
    `amplitude`: a sum of sines with whole numbers of waves per width (so it
    wraps), longer waves stronger. min_wave: shortest wavelength used.
    power > 1 makes it mostly low with sharper humps. Returns h(x).
    """
    rng = random.Random(seed)
    waves = []
    for k in range(1, terms + 1):
        if min_wave and width / k < min_wave:
            break
        waves.append((k, rng.uniform(0.3, 1.0) / k ** 0.6, rng.uniform(0.0, 2.0 * math.pi)))

    def raw(x):
        return sum(a * math.sin(2.0 * math.pi * k * x / width + p) for k, a, p in waves)

    samples = [raw(width * i / 512.0) for i in range(512)]
    lo = min(samples)
    hi = max(samples)
    span = (hi - lo) or 1.0

    def h(x):
        return min(1.0, max(0.0, (raw(x) - lo) / span)) ** power * amplitude

    return h
