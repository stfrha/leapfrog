"""
amb_mine_parts - building blocks for the mine site ambient pieces
(amb_excavator, amb_haul_truck, amb_crusher, amb_headframe).

Same conventions as amb_parts: 1 Blender unit = 1 game unit before the kit's
scale, the camera looks along +Y (fronts face -Y, the right side +X is the
side render_axo sees), pieces stand on z = 0 with the footprint centred on
the origin. Machines face +X (their working end is on the right).

Every helper takes the kit's material function M (kit.material) and builds
separate objects, named after `name`.
"""

import math
import random

import bmesh
import sprite_kit as sk


def beam(name, M, role, p0, p1, width, depth=None, bevel=0.04):
    """
    A straight box beam from p0 to p1 in the X/Z plane (y is taken from p0):
    `width` across the beam in that plane, `depth` along Y (default = width).
    For booms, sticks, trusses' chords and the like.
    """
    x0, y0, z0 = p0
    x1, _y1, z1 = p1
    dx = x1 - x0
    dz = z1 - z0
    length = math.hypot(dx, dz)
    if length < 1e-6:
        raise ValueError("beam %s: p0 and p1 are the same point" % name)
    phi = math.atan2(dz, dx)
    ob = sk.box_object(name, ((x0 + x1) / 2, y0, (z0 + z1) / 2),
                       (length, depth if depth else width, width), M(role),
                       rotation=(0.0, -phi, 0.0), bevel_width=bevel)
    return ob


def hydraulic(name, M, p0, p1, radius=0.14):
    """A hydraulic ram: a thick barrel from p0 two thirds of the way, a shiny thinner rod to p1."""
    x0, y0, z0 = p0
    x1, y1, z1 = p1
    xm = x0 + (x1 - x0) * 0.62
    ym = y0 + (y1 - y0) * 0.62
    zm = z0 + (z1 - z0) * 0.62
    bm = bmesh.new()
    sk.strut(bm, p0, (xm, ym, zm), radius, 10)
    sk.mesh_object(name + "_barrel", bm, M("frame_main"), smooth_quads=True)
    bm = bmesh.new()
    sk.strut(bm, (xm, ym, zm), p1, radius * 0.55, 8)
    sk.mesh_object(name + "_rod", bm, M("frame_brace"), smooth_quads=True)


def track(name, M, role, x_center, y_center, length, height, width, rollers=5):
    """
    A crawler track running along X: a box with round ends, a row of road
    wheels on the outside (-Y) face and a few grouser ribs on top.
    """
    r = height / 2
    zc = r
    straight = max(0.1, length - height)
    sk.box_object(name, (x_center, y_center, zc), (straight, width, height), M(role), bevel_width=0.06)
    ends = bmesh.new()
    for sx in (-1, 1):
        x = x_center + sx * straight / 2
        sk.strut(ends, (x, y_center - width / 2, zc), (x, y_center + width / 2, zc), r, 20)
    sk.mesh_object(name + "_ends", ends, M(role), smooth_quads=True)

    # Idler and sprocket hubs at the ends, road wheels between them
    hubs = bmesh.new()
    face = y_center - width / 2
    for sx in (-1, 1):
        x = x_center + sx * straight / 2
        sk.strut(hubs, (x, face - 0.08, zc), (x, face + 0.02, zc), r * 0.62, 16)
    for i in range(rollers):
        x = x_center - straight / 2 + straight * (i + 0.5) / rollers
        sk.strut(hubs, (x, face - 0.06, r * 0.72), (x, face + 0.02, r * 0.72), r * 0.3, 12)
    sk.mesh_object(name + "_hubs", hubs, M("frame_main"), smooth_quads=True)

    # Track frame between the wheels
    sk.box_object(name + "_frame", (x_center, face - 0.04, zc + r * 0.25), (straight, 0.08, r * 0.5), M("frame_main"))

    # Grouser ribs across the top run
    ribs = bmesh.new()
    n = max(3, int(straight / 0.45))
    for i in range(n):
        x = x_center - straight / 2 + straight * (i + 0.5) / n
        sk.box(ribs, (x, y_center, height + 0.04), (0.12, width * 1.02, 0.08))
    sk.mesh_object(name + "_grousers", ribs, M("frame_main"))


def wheel(name, M, center, radius, width, rim_role="machine_yellow", tread=True):
    """
    A big off-road tyre with its axis along Y, centred at `center`, with a
    painted rim and hub on both faces and chunky tread blocks round it.
    """
    x, y, z = center
    y0 = y - width / 2
    y1 = y + width / 2
    bm = bmesh.new()
    sk.strut(bm, (x, y0, z), (x, y1, z), radius * 0.97, 32)
    sk.mesh_object(name + "_tyre", bm, M("rubber"), smooth_quads=True)

    if tread:
        n = 18
        for i in range(n):
            a = 2.0 * math.pi * i / n
            tx = x + radius * 0.95 * math.cos(a)
            tz = z + radius * 0.95 * math.sin(a)
            size = (radius * 0.16, width * 0.86, radius * 0.11)
            # Blocks lie on the tyre: turned about Y so their local Z points
            # outwards (rotation t about Y takes Z to (sin t, cos t) in X/Z)
            sk.box_object("%s_tread_%d" % (name, i), (tx, y, tz), size, M("rubber"),
                          rotation=(0.0, math.pi / 2 - a, 0.0))

    rims = bmesh.new()
    for face, d in ((y0, -1), (y1, 1)):
        sk.strut(rims, (x, face - d * 0.06, z), (x, face + d * 0.02, z), radius * 0.55, 24)
    sk.mesh_object(name + "_rim", rims, M(rim_role), smooth_quads=True)
    hubs = bmesh.new()
    for face, d in ((y0, -1), (y1, 1)):
        sk.strut(hubs, (x, face - d * 0.16, z), (x, face + d * 0.02, z), radius * 0.2, 12)
    sk.mesh_object(name + "_hub", hubs, M("frame_main"), smooth_quads=True)


def heap(name, M, center, radius, height, role="ore_rock", seed=1, rough=0.12, segments=40, rings=9,
         scale=(1.0, 1.0, 1.0)):
    """
    A heap of loose rock: a cone with a rounded top and an uneven edge,
    built around `center` (on the ground). scale stretches it, e.g. for an
    elongated load in a truck body.
    """
    rnd = random.Random(seed)
    phases = [rnd.uniform(0, 2 * math.pi) for _ in range(4)]
    profile = []
    for k in range(rings + 1):
        t = k / rings
        # Angle of repose cone that rounds over at the top
        r = radius * (1.0 - t) ** 0.85 + radius * 0.06 * (1.0 - t)
        z = height * (1.0 - (1.0 - t) ** 1.6)
        profile.append((max(r, radius * 0.04), z))
    bm = bmesh.new()
    sk.lathe(bm, profile, segments, cap_top=True, cap_bottom=True)
    for v in bm.verts:
        x, y, z = v.co
        r = math.hypot(x, y)
        if r < 1e-6:
            continue
        a = math.atan2(y, x)
        f = 1.0 + rough * (0.5 * math.sin(3 * a + phases[0]) + 0.3 * math.sin(5 * a + phases[1])
                           + 0.2 * math.sin(9 * a + phases[2] + z))
        f += rnd.uniform(-rough, rough) * 0.35
        v.co.x = x * f
        v.co.y = y * f
        if z > 0.01:
            v.co.z = z * (1.0 + rnd.uniform(-rough, rough) * 0.4)
    ob = sk.mesh_object(name, bm, M(role), smooth_quads=True)
    ob.location = center
    ob.scale = scale
    return ob


def cab(name, M, center, size, role="machine_yellow", lit=False, roof_role=None):
    """
    An operator's cab: a box with big windows on the front (-Y) and right
    (+X) faces, a roof that overhangs a little. center is the middle of the
    box. lit puts a warm light behind the glass.
    """
    cx, cy, cz = center
    sx, sy, sz = size
    sk.box_object(name, center, size, M(role), bevel_width=0.06)
    glass = M("window_lit_warm") if lit else M("glass_dark")
    # Front window (most of the face) and side window
    sk.box_object(name + "_win_front", (cx + sx * 0.05, cy - sy / 2 - 0.03, cz + sz * 0.12),
                  (sx * 0.72, 0.06, sz * 0.55), glass)
    sk.box_object(name + "_win_side", (cx + sx / 2 + 0.03, cy, cz + sz * 0.12),
                  (0.06, sy * 0.7, sz * 0.55), glass)
    sk.box_object(name + "_roof", (cx, cy, cz + sz / 2 + 0.05), (sx + 0.2, sy + 0.2, 0.1),
                  M(roof_role or role), bevel_width=0.03)


def sheave(name, M, center, radius, spokes=6, rim=0.14, spoke_r=0.07, role="frame_brace"):
    """
    A headframe sheave wheel in the X/Z plane (axis along Y), with its
    origin at the hub so it can be turned about its own Y axis for
    animation frames. Returns the object.
    """
    bm = bmesh.new()
    n = 48                  # a multiple of 6 x 4, so the default 15 degree frames move the rim by whole segments
    for ring_r in (radius, radius - rim * 1.8):
        pts = [(ring_r * math.cos(2 * math.pi * i / n), 0.0, ring_r * math.sin(2 * math.pi * i / n))
               for i in range(n)]
        for i in range(n):
            sk.strut(bm, pts[i], pts[(i + 1) % n], rim if ring_r == radius else rim * 0.6, 6)
    for i in range(spokes):
        a = 2 * math.pi * i / spokes
        sk.strut(bm, (0.0, 0.0, 0.0), (radius * math.cos(a), 0.0, radius * math.sin(a)), spoke_r, 6)
    sk.strut(bm, (0.0, -0.25, 0.0), (0.0, 0.25, 0.0), radius * 0.12, 12)
    ob = sk.mesh_object(name, bm, M(role))
    ob.location = center
    return ob


def truss(name, M, p0, p1, width, height, bays=None, chord_r=0.09, web_r=0.05):
    """
    A box truss from p0 to p1 (both at the middle of its bottom face, any
    direction in X/Z), `width` across (Y) and `height` deep: four chords,
    verticals and diagonals on both sides. Conveyor galleries, gantries.
    """
    x0, y0, z0 = p0
    x1, _y1, z1 = p1
    length = math.hypot(x1 - x0, z1 - z0)
    if bays is None:
        bays = max(2, int(round(length / 2.0)))
    # Unit vectors along the truss and "up" across it (both in X/Z)
    ux, uz = (x1 - x0) / length, (z1 - z0) / length
    nx, nz = -uz, ux

    def pt(t, side, up):
        s = length * t
        return (x0 + ux * s + nx * up, y0 + side * width / 2, z0 + uz * s + nz * up)

    bm = bmesh.new()
    for side in (-1, 1):
        for up in (0.0, height):
            sk.strut(bm, pt(0, side, up), pt(1, side, up), chord_r, 6)
    web = bmesh.new()
    for i in range(bays + 1):
        t = i / bays
        for side in (-1, 1):
            sk.strut(web, pt(t, side, 0.0), pt(t, side, height), web_r, 6)
        sk.strut(web, pt(t, -1, 0.0), pt(t, 1, 0.0), web_r, 6)
        if i < bays:
            t1 = (i + 1) / bays
            for side in (-1, 1):
                if i % 2 == 0:
                    sk.strut(web, pt(t, side, 0.0), pt(t1, side, height), web_r, 6)
                else:
                    sk.strut(web, pt(t, side, height), pt(t1, side, 0.0), web_r, 6)
    sk.mesh_object(name, bm, M("frame_main"))
    sk.mesh_object(name + "_web", web, M("frame_brace"))


def trestle(name, M, base, top_z, spread=1.4, width=2.0):
    """A pair of A-legs (front and back) standing on `base` up to top_z, braced across."""
    bx, by, bz = base
    bm = bmesh.new()
    for side in (-1, 1):
        y = by + side * width / 2
        sk.strut(bm, (bx - spread / 2, y, bz), (bx, y, top_z), 0.1, 6)
        sk.strut(bm, (bx + spread / 2, y, bz), (bx, y, top_z), 0.1, 6)
        sk.strut(bm, (bx - spread / 4, y, (bz + top_z) / 2), (bx + spread / 4, y, (bz + top_z) / 2), 0.05, 6)
    sk.strut(bm, (bx, by - width / 2, top_z), (bx, by + width / 2, top_z), 0.08, 6)
    sk.strut(bm, (bx, by - width / 2, (bz + top_z) / 2), (bx, by + width / 2, (bz + top_z) / 2), 0.05, 6)
    sk.mesh_object(name, bm, M("frame_main"))
    for side in (-1, 1):
        sk.box_object("%s_foot_%d" % (name, side), (bx, by + side * width / 2, bz + 0.15),
                      (spread + 0.4, 0.5, 0.3), M("concrete_dirty"))


def beacon(name, M, position, lit=False, size=0.3):
    """A small warning light: amber when lit, a dead lamp otherwise."""
    sk.box_object(name, position, (size, size, size * 0.8), M("neon_amber") if lit else M("neon_dead"))
