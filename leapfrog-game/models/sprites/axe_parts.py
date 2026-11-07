"""
axe_parts - shared parts for the military axe ships (hammer_ship.py,
hammer_interceptor.py): an axe blade seen from above, after a double-bitted
battle axe. The blade's cheek is armour in military paint; its curved edge is
bright ground steel, a sloped bevel with grind marks running along the edge
and a narrow polished hone at the very edge, so the "sharpened" front reads
clearly apart from the rest of the hull.

Top-down view for the ortho rig: the ship lies in the x-z plane (x forward
is to the left, as the game's sprites: the front is -x), its top faces the
camera at -Y, and heights are negative y.
"""

import math

import bpy                        # noqa: F401
import bmesh
from mathutils import Matrix      # noqa: F401

import sprite_kit as sk


# ---------------------------------------------------------------------------
# Materials
# ---------------------------------------------------------------------------

def _principled(name):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    return m, m.node_tree, m.node_tree.nodes.get("Principled BSDF")


def ground_steel(name="axe_ground_steel", streaks=9.0):
    """Bright ground steel: grind marks along the edge (UV u runs along it,
    v across the bevel), varying brightness and roughness."""
    m, nt, bsdf = _principled(name)
    bsdf.inputs["Metallic"].default_value = 1.0
    tc = nt.nodes.new("ShaderNodeTexCoord")
    mp = nt.nodes.new("ShaderNodeMapping")
    mp.inputs["Scale"].default_value = (0.12, streaks, 1.0)
    nz = nt.nodes.new("ShaderNodeTexNoise")
    nz.inputs["Scale"].default_value = 4.0
    nz.inputs["Detail"].default_value = 8.0
    nt.links.new(tc.outputs["UV"], mp.inputs["Vector"])
    nt.links.new(mp.outputs["Vector"], nz.inputs["Vector"])
    col = nt.nodes.new("ShaderNodeValToRGB")
    col.color_ramp.elements[0].position = 0.35
    col.color_ramp.elements[0].color = (0.50, 0.51, 0.53, 1.0)
    col.color_ramp.elements[1].position = 0.65
    col.color_ramp.elements[1].color = (0.86, 0.87, 0.89, 1.0)
    nt.links.new(nz.outputs["Fac"], col.inputs["Fac"])
    nt.links.new(col.outputs["Color"], bsdf.inputs["Base Color"])
    rough = nt.nodes.new("ShaderNodeMapRange")
    rough.inputs["To Min"].default_value = 0.34
    rough.inputs["To Max"].default_value = 0.16
    nt.links.new(nz.outputs["Fac"], rough.inputs["Value"])
    nt.links.new(rough.outputs["Result"], bsdf.inputs["Roughness"])
    return m


def hone_steel(name="axe_hone"):
    """The polished hone along the very edge."""
    m, nt, bsdf = _principled(name)
    bsdf.inputs["Metallic"].default_value = 1.0
    bsdf.inputs["Base Color"].default_value = (0.95, 0.95, 0.96, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.07
    return m


def military_paint(kit, name, paint, rust=0.15, streak=0.12, fallback="frame_main"):
    """The theme's weathered paint in another colour (for the cheek and hull)."""
    wp = getattr(kit.theme, "_weathered_paint", None)
    if wp is None:
        return kit.material(fallback)
    return wp(name, paint=paint, rust=rust, streak_scale=streak)


def flat_paint(name, color, rough=0.75):
    m, nt, bsdf = _principled(name)
    bsdf.inputs["Base Color"].default_value = (color[0], color[1], color[2], 1.0)
    bsdf.inputs["Roughness"].default_value = rough
    bsdf.inputs["Metallic"].default_value = 0.0
    return m


# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------

def bezier(p0, c, p1, n=12):
    """Quadratic Bezier from p0 to p1 (n segments, p0 excluded, p1 included)."""
    out = []
    for i in range(1, n + 1):
        t = i / n
        a, b, d = (1 - t) ** 2, 2 * (1 - t) * t, t * t
        out.append((a * p0[0] + b * c[0] + d * p1[0], a * p0[1] + b * c[1] + d * p1[1]))
    return out


def arc_circle(front, horn):
    """The circle through the edge's front point (on the x axis) and a horn
    tip: (centre x, radius, horn angle in radians, measured from +x)."""
    fx, hx, hz = front, horn[0], horn[1]
    cx = (hx * hx + hz * hz - fx * fx) / (2.0 * (hx - fx))
    r = abs(fx - cx)
    return cx, r, math.atan2(hz, hx - cx)


def slab(name, outline, top, bottom, material):
    """A flat slab cut to an outline in x-z, its top at y = top (towards the
    camera), its bottom at y = bottom."""
    return sk.prism(name, outline, top, bottom - top, material)


def stud(name, x, z, top, radius, material, height=0.18):
    bm = bmesh.new()
    sk.strut(bm, (x, top + 0.05, z), (x, top - height, z), radius, 12)
    return sk.mesh_object(name, bm, material, smooth_quads=True)


def disc(name, x, z, radius, top, material, height=0.1, segments=32):
    """A disc lying on a surface at y = top (seen from above as a circle)."""
    bm = bmesh.new()
    sk.lathe(bm, [(radius, 0.0), (radius, height)], segments, cap_top=True, cap_bottom=True)
    ob = sk.mesh_object(name, bm, material)
    ob.location = (x, top + height * 0.5, z)
    ob.rotation_euler = (math.pi / 2, 0.0, 0.0)     # lathe axis (Z) towards the camera (-Y)
    return ob


def lathe_up(name, profile, x, z, base_y, material, segments=48):
    """A lathe whose height grows towards the camera from y = base_y."""
    bm = bmesh.new()
    sk.lathe(bm, profile, segments, cap_top=True, cap_bottom=True)
    ob = sk.mesh_object(name, bm, material, smooth_quads=True)
    ob.location = (x, base_y, z)
    ob.rotation_euler = (math.pi / 2, 0.0, 0.0)
    return ob


# ---------------------------------------------------------------------------
# The blade
# ---------------------------------------------------------------------------

class Blade:
    """
    An axe blade seen from above, facing -x (sx = 1) or +x (sx = -1, the
    back blade of a double axe).

    front     x of the edge's foremost point (on the centre line), for sx = 1
    horn      (x, z) of the upper horn tip (the lower one is mirrored)
    join      (x, z) where the back edge meets the body (upper; mirrored)
    bite      control point of the concave back edge from horn to join
    band      width of the ground bevel at the middle of the edge
    hone      width of the polished edge
    top       y of the cheek's top (negative: towards the camera)
    """

    def __init__(self, prefix, front, horn, join, bite, band, hone, top, sx=1.0, segments=64):
        self.prefix = prefix
        self.sx = sx
        self.top = top
        self.cx, self.r, a = arc_circle(front, horn)
        self.a0, self.a1 = a, 2.0 * math.pi - a            # upper horn .. lower horn through the front
        self.band, self.hone = band, hone
        self.n = segments
        self.horn, self.join, self.bite = horn, join, bite

    def _p(self, x, z):
        return (self.sx * x, z)

    def _dir(self, t):
        return math.cos(t), math.sin(t)

    def width(self, t):
        """Bevel width: full in the middle, tapering to a sliver at the horns."""
        taper = math.radians(28.0)
        d = min(t - self.a0, self.a1 - t)
        return self.band * max(0.12, min(1.0, d / taper))

    def angles(self):
        return [self.a0 + (self.a1 - self.a0) * i / self.n for i in range(self.n + 1)]

    def point(self, t, inset):
        c, s = self._dir(t)
        return (self.cx + (self.r - inset) * c, (self.r - inset) * s)

    def cheek_outline(self):
        """The armoured cheek: inside the bevel, back edges curving to the body."""
        pts = [self.point(t, self.width(t)) for t in self.angles()]
        lo_h = (self.horn[0], -self.horn[1])
        lo_j = (self.join[0], -self.join[1])
        lo_b = (self.bite[0], -self.bite[1])
        pts.append(lo_h)
        pts += bezier(lo_h, lo_b, lo_j)
        pts.append(self.join)
        pts += bezier(self.join, self.bite, self.horn)
        return [self._p(x, z) for x, z in pts]

    def build(self, cheek_material, ground_material, hone_material, cheek_bottom=0.3):
        obs = [slab(self.prefix + "_cheek", self.cheek_outline(), self.top, cheek_bottom, cheek_material)]
        obs += self._bevel(ground_material, hone_material, cheek_bottom)
        return obs

    def _bevel(self, ground_material, hone_material, bottom):
        """The ground bevel (cheek top down to the hone) and the polished hone
        (down to the edge), as two sloped strips with UVs along the edge."""
        top = self.top
        mid = top * 0.28                                   # where the bevel meets the hone
        rows = []
        for t in self.angles():
            w = self.width(t)
            h = min(self.hone, w * 0.5)
            rows.append((t, [self.point(t, w), self.point(t, h), self.point(t, 0.0)]))
        out = []
        for (name, i0, y0, i1, y1, material) in ((self.prefix + "_ground", 0, top, 1, mid, ground_material),
                                                 (self.prefix + "_hone", 1, mid, 2, 0.0, hone_material)):
            bm = bmesh.new()
            uv = bm.loops.layers.uv.new("UVMap")
            verts = []
            for t, ring in rows:
                a = self._p(*ring[i0])
                b = self._p(*ring[i1])
                verts.append((bm.verts.new((a[0], y0, a[1])), bm.verts.new((b[0], y1, b[1])), t))
            # the edge's own underside: down to the bottom, so the silhouette is solid
            for k in range(len(verts) - 1):
                (a0, b0, t0), (a1, b1, t1) = verts[k], verts[k + 1]
                f = bm.faces.new((a0, b0, b1, a1))
                for loop in f.loops:
                    tt = t0 if loop.vert in (a0, b0) else t1
                    v = 0.0 if loop.vert in (a0, a1) else 1.0
                    loop[uv].uv = (tt * self.r, v)
            if i1 == 2:
                lows = []
                for (a, b, t) in verts:
                    co = b.co.copy()
                    co.y = bottom
                    lows.append(bm.verts.new(co))
                for k in range(len(verts) - 1):
                    bm.faces.new((verts[k][1], lows[k], lows[k + 1], verts[k + 1][1]))
            bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
            out.append(sk.mesh_object(name, bm, material, smooth_quads=False))
        return out

    def rivets(self, material, inset=0.4, spacing=0.75, radius=0.11, every=1):
        """A row of rivets just inside the bevel."""
        obs = []
        arc = (self.a1 - self.a0) * self.r
        n = max(2, int(arc / spacing))
        for i in range(1, n):
            if i % every:
                continue
            t = self.a0 + (self.a1 - self.a0) * i / n
            x, z = self.point(t, self.width(t) + inset)
            x, z = self._p(x, z)
            obs.append(stud("%s_rivet_%d" % (self.prefix, i), x, z, self.top, radius, material))
        return obs

    def panel_lines(self, material, angles_deg, inner, line=0.07):
        """Thin dark lines from the bevel inwards (plate joints on the cheek)."""
        obs = []
        for k, deg in enumerate(angles_deg):
            t = math.radians(deg)
            w = self.width(t) + 0.12
            c, s = self._dir(t)
            nx, nz = -s * line, c * line
            p0 = self.point(t, w)
            p1 = self.point(t, inner)
            pts = [(p0[0] - nx, p0[1] - nz), (p0[0] + nx, p0[1] + nz), (p1[0] + nx, p1[1] + nz), (p1[0] - nx, p1[1] - nz)]
            obs.append(sk.prism("%s_seam_%d" % (self.prefix, k), [self._p(x, z) for x, z in pts],
                                self.top - 0.01, 0.02, material))
        return obs


def lathe_x(name, profile, x0, material, flatten=1.0, z=0.0, segments=32):
    """A lathe lying along +x from x0 (profile: (radius, distance along x)),
    squashed towards the camera by flatten (1 = round)."""
    bm = bmesh.new()
    sk.lathe(bm, profile, segments, cap_top=True, cap_bottom=True)
    ob = sk.mesh_object(name, bm, material, smooth_quads=True)
    ob.location = (x0, 0.0, z)
    ob.rotation_euler = (0.0, math.pi / 2, 0.0)       # lathe axis (Z) along +x; local y stays y
    ob.scale = (1.0, flatten, 1.0)
    return ob


def rod(name, points, radius, y, material, segments=10):
    """A thin bent rod through points (x, z) at height y: legs, antennae."""
    bm = bmesh.new()
    for (x0, z0), (x1, z1) in zip(points, points[1:]):
        sk.strut(bm, (x0, y, z0), (x1, y, z1), radius, segments)
    return sk.mesh_object(name, bm, material, smooth_quads=True)


def emission(name, color, strength):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (color[0], color[1], color[2], 1.0)
    em.inputs["Strength"].default_value = strength
    nt.links.new(em.outputs["Emission"], out.inputs["Surface"])
    return m
