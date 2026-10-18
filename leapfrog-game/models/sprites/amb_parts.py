"""
amb_parts - small building blocks shared by the ambient piece scripts (amb_*.py).

Conventions (as in sprite_kit): 1 Blender unit = 1 game unit before the kit's
scale, the camera looks along +Y, so a building's front wall faces -Y and its
right side (+X) is the side render_axo's camera also sees. Pieces stand on
the ground at z = 0 with the footprint centred on the origin, so --turn and
--scale work about the middle of the piece.

Each helper builds its parts as separate objects and takes the kit's
material function M (kit.material).
"""

import math
import random

import bmesh
import sprite_kit as sk


def window(name, M, center, size, facing="front", lit=True, warm=True, frame=0.15):
    """
    A window on a wall: dark frame plus a pane, both standing slightly proud
    of the wall. center is the point on the wall surface; facing is 'front'
    (the wall at -Y) or 'right' (the wall at +X).
    """
    w, h = size
    pane = M("window_lit_warm" if warm else "window_lit_blue") if lit else M("glass_dark")
    x, y, z = center
    if facing == "front":
        sk.box_object(name + "_frame", (x, y - 0.04, z), (w + 2 * frame, 0.08, h + 2 * frame), M("frame_main"))
        sk.box_object(name, (x, y - 0.09, z), (w, 0.04, h), pane)
    else:
        sk.box_object(name + "_frame", (x + 0.04, y, z), (0.08, w + 2 * frame, h + 2 * frame), M("frame_main"))
        sk.box_object(name, (x + 0.09, y, z), (0.04, w, h), pane)


def door(name, M, center, size=(1.2, 2.3), facing="front", frame_role="hazard_paint"):
    """A door with a contrasting frame; center is at the bottom middle of the door on the wall."""
    w, h = size
    x, y, z = center
    if facing == "front":
        sk.box_object(name + "_frame", (x, y - 0.05, z + h / 2 + 0.1), (w + 0.4, 0.1, h + 0.2), M(frame_role))
        sk.box_object(name, (x, y - 0.11, z + h / 2), (w, 0.08, h), M("frame_main"))
    else:
        sk.box_object(name + "_frame", (x + 0.05, y, z + h / 2 + 0.1), (0.1, w + 0.4, h + 0.2), M(frame_role))
        sk.box_object(name, (x + 0.11, y, z + h / 2), (0.08, w, h), M("frame_main"))


def window_row(prefix, M, x0, x1, y, z, count, size=(1.0, 0.8), rnd=None, lit_share=0.6, warm=True):
    """count windows evenly between x0 and x1 on the front wall at y; a random share is lit."""
    rnd = rnd or random.Random(1)
    for i in range(count):
        x = x0 + (x1 - x0) * (i + 0.5) / count
        window("%s_%d" % (prefix, i), M, (x, y, z), size, "front", lit=rnd.random() < lit_share, warm=warm)


def side_window_row(prefix, M, y0, y1, x, z, count, size=(1.0, 0.8), rnd=None, lit_share=0.6, warm=True):
    """Like window_row, on the right side wall at x."""
    rnd = rnd or random.Random(2)
    for i in range(count):
        y = y0 + (y1 - y0) * (i + 0.5) / count
        window("%s_%d" % (prefix, i), M, (x, y, z), size, "right", lit=rnd.random() < lit_share, warm=warm)


def mast(name, M, base, height, light=True, dark=False):
    """Thin mast with two stays and an optional light on top."""
    bx, by, bz = base
    bm = bmesh.new()
    sk.strut(bm, (bx, by, bz), (bx, by, bz + height), 0.09)
    sk.strut(bm, (bx - 0.6, by, bz), (bx, by, bz + height * 0.45), 0.05)
    sk.strut(bm, (bx + 0.6, by, bz), (bx, by, bz + height * 0.45), 0.05)
    sk.strut(bm, (bx - 0.45, by, bz + height * 0.85), (bx + 0.45, by, bz + height * 0.85), 0.05)
    sk.mesh_object(name, bm, M("frame_main"))
    if light:
        sk.box_object(name + "_light", (bx, by, bz + height + 0.12), (0.3, 0.3, 0.24),
                      M("neon_dead") if dark else M("neon_amber"))


def ribbed_box(name, M, center, size, role, rib_role="frame_main", rib_every=2.0, ribs_on=("front", "right"),
               rib_w=0.25, rib_d=0.12, bevel=0.05):
    """A wall box with vertical ribs on the visible faces (prefab panels, containers)."""
    cx, cy, cz = center
    sx, sy, sz = size
    sk.box_object(name, center, size, M(role), bevel_width=bevel)
    bm = bmesh.new()
    if "front" in ribs_on:
        n = max(1, int(round(sx / rib_every)))
        for i in range(n + 1):
            x = cx - sx / 2 + sx * i / n
            sk.box(bm, (x, cy - sy / 2 - rib_d / 2, cz), (rib_w, rib_d, sz))
    if "right" in ribs_on:
        n = max(1, int(round(sy / rib_every)))
        for i in range(n + 1):
            y = cy - sy / 2 + sy * i / n
            sk.box(bm, (cx + sx / 2 + rib_d / 2, y, cz), (rib_d, rib_w, sz))
    sk.mesh_object(name + "_ribs", bm, M(rib_role))


def corrugated_box(name, M, center, size, role, pitch=0.3, depth=0.06):
    """
    A box whose front and right faces are corrugated (shipping containers,
    sheds): thin vertical bars at `pitch` in the wall's own material on top
    of a plain box. At ambient sizes this reads as corrugation.
    """
    cx, cy, cz = center
    sx, sy, sz = size
    sk.box_object(name, center, size, M(role), bevel_width=0.03)
    bm = bmesh.new()
    n = max(1, int(sx / pitch))
    for i in range(n):
        x = cx - sx / 2 + (i + 0.5) * sx / n
        sk.box(bm, (x, cy - sy / 2 - depth / 2, cz), (pitch * 0.45, depth, sz * 0.92))
    n = max(1, int(sy / pitch))
    for i in range(n):
        y = cy - sy / 2 + (i + 0.5) * sy / n
        sk.box(bm, (cx + sx / 2 + depth / 2, y, cz), (depth, pitch * 0.45, sz * 0.92))
    sk.mesh_object(name + "_corr", bm, M(role))


def cylinder(name, M, role, base, radius, height, segments=32, cone=0.0):
    """Upright cylinder standing on base=(x, y, z), with an optional conical roof `cone` high."""
    bm = bmesh.new()
    sk.lathe(bm, [(radius, 0.0), (radius, height)], segments, cap_bottom=True, cap_top=(cone <= 0))
    if cone > 0:
        sk.lathe(bm, [(radius * 1.04, height), (radius * 0.08, height + cone)], segments, cap_top=True)
    ob = sk.mesh_object(name, bm, M(role))
    ob.location = base
    return ob


def plinth(M, w, d, h=0.6):
    """Concrete slab the piece stands on, from z = 0 to h."""
    sk.box_object("plinth", (0.0, 0.0, h / 2), (w, d, h), M("concrete_dirty"), bevel_width=0.08)
    return h


def railing(name, M, p0, p1, height=1.0, post_every=1.5):
    """Hand rail with posts along the line p0 -> p1 (both at the walkway surface)."""
    bm = bmesh.new()
    x0, y0, z0 = p0
    x1, y1, z1 = p1
    length = math.dist((x0, y0), (x1, y1))
    n = max(1, int(round(length / post_every)))
    for i in range(n + 1):
        t = i / n
        x = x0 + (x1 - x0) * t
        y = y0 + (y1 - y0) * t
        z = z0 + (z1 - z0) * t
        sk.strut(bm, (x, y, z), (x, y, z + height), 0.04, 6)
    sk.strut(bm, (x0, y0, z0 + height), (x1, y1, z1 + height), 0.05, 6)
    sk.strut(bm, (x0, y0, z0 + height / 2), (x1, y1, z1 + height / 2), 0.03, 6)
    sk.mesh_object(name, bm, M("frame_brace"))
