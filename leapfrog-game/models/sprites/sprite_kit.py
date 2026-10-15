"""
sprite_kit.py - shared helpers for rendering Leapfrog sprites with Blender.

Sprite scripts live next to this file and are run headless, e.g. from
leapfrog-game\\models\\sprites:

    "C:\\Program Files\\Blender Foundation\\Blender 4.3\\blender.exe" -b --factory-startup -P ls_tower.py

Options go after a "--":

    ... -P ls_tower.py -- --samples 32 --save-blend

Conventions
-----------
* 1 Blender unit = 1 game unit (Box2D metre).
* Sprites are viewed from the front: the camera looks along +Y,
  X is right and Z is up in Blender. (The game's y axis points down,
  but that only matters for joints/positions, which are made in the editor.)
* Images are rendered at PPU pixels per unit (default 20, the same as
  GlobalConstants.TexturePixelsPerUnit in leapfrog-editor). A part whose
  longest side would exceed --max-px is rendered at the largest whole PPU
  that fits, and a warning is printed.
* The image frame is exactly the box you pass to render_ortho(), not the
  visible pixels, so the sprite lines up with the width x height body box.
* Output: <name>.png straight into leapfrog-game/data/images/.
"""

import argparse
import importlib
import math
import os
import sys

import bpy
import bmesh
from mathutils import Matrix, Vector

KIT_DIR = os.path.dirname(os.path.abspath(__file__))
MODELS_DIR = os.path.dirname(KIT_DIR)            # leapfrog-game/models
GAME_DIR = os.path.dirname(MODELS_DIR)           # leapfrog-game
DEFAULT_OUT = os.path.join(GAME_DIR, "data", "images")
BLEND_OUT = os.path.join(KIT_DIR, "_blend")

DEFAULT_PPU = 20.0       # keep equal to GlobalConstants.TexturePixelsPerUnit
DEFAULT_MAX_PX = 2048    # safe texture / atlas size on older phones

if KIT_DIR not in sys.path:
    sys.path.insert(0, KIT_DIR)


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def parse_args(add_extra=None):
    """Parse the options after '--'. add_extra(parser) can add script options."""
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser(prog="blender -b -P <sprite>.py --")
    p.add_argument("--theme", default="mining", help="world theme in themes/ (default: mining)")
    p.add_argument("--out", default=DEFAULT_OUT, help="output folder (default: leapfrog-game/data/images)")
    p.add_argument("--ppu", type=float, default=DEFAULT_PPU, help="pixels per game unit (default 20)")
    p.add_argument("--max-px", type=int, default=DEFAULT_MAX_PX, help="max image side in pixels (default 2048)")
    p.add_argument("--samples", type=int, default=128, help="Cycles samples (default 128, use ~16 for previews)")
    p.add_argument("--gpu", action="store_true", help="render on the GPU if one is available")
    p.add_argument("--hdri", default=None, help="path to an HDRI, overrides the theme's default")
    p.add_argument("--save-blend", action="store_true", help="also save the generated scene to sprites/_blend/")
    p.add_argument("--no-render", action="store_true", help="build the scene only (use with --save-blend)")
    if add_extra:
        add_extra(p)
    return p.parse_args(argv)


# ---------------------------------------------------------------------------
# Scene setup
# ---------------------------------------------------------------------------

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def setup_render(scene, args):
    scene.render.engine = 'CYCLES'
    cy = scene.cycles
    cy.samples = args.samples
    cy.use_adaptive_sampling = True
    cy.use_denoising = True
    try:
        cy.denoiser = 'OPENIMAGEDENOISE'
    except TypeError:
        pass

    if args.gpu:
        _enable_gpu(cy)

    r = scene.render
    r.film_transparent = True
    r.resolution_percentage = 100
    r.pixel_aspect_x = 1.0
    r.pixel_aspect_y = 1.0
    r.filter_size = 1.0      # slightly crisper than the default 1.5 at sprite sizes
    r.image_settings.file_format = 'PNG'
    r.image_settings.color_mode = 'RGBA'
    r.image_settings.color_depth = '8'
    r.image_settings.compression = 90

    try:
        scene.view_settings.view_transform = 'Filmic'   # same as the existing ship/silo renders
    except TypeError:
        scene.view_settings.view_transform = 'AgX'
    scene.view_settings.look = 'None'


def _enable_gpu(cy):
    prefs = bpy.context.preferences.addons['cycles'].preferences
    for dev_type in ('OPTIX', 'CUDA', 'HIP', 'ONEAPI', 'METAL'):
        try:
            prefs.compute_device_type = dev_type
        except TypeError:
            continue
        prefs.get_devices()
        devices = [d for d in prefs.devices if d.type == dev_type]
        if devices:
            for d in prefs.devices:
                d.use = (d.type == dev_type)
            cy.device = 'GPU'
            print("sprite_kit: rendering on GPU (%s)" % dev_type)
            return
    print("sprite_kit: no GPU found, rendering on CPU")


# ---------------------------------------------------------------------------
# Geometry helpers (bmesh)
# ---------------------------------------------------------------------------

def box(bm, center, size):
    """Axis aligned box. center=(x,y,z), size=(sx,sy,sz)."""
    m = Matrix.Translation(Vector(center)) @ Matrix.Diagonal(Vector((size[0], size[1], size[2], 1.0)))
    bmesh.ops.create_cube(bm, size=1.0, matrix=m)


def strut(bm, p0, p1, radius, segments=8):
    """Round bar from p0 to p1."""
    p0 = Vector(p0)
    p1 = Vector(p1)
    d = p1 - p0
    rot = d.normalized().to_track_quat('Z', 'Y').to_matrix().to_4x4()
    m = Matrix.Translation((p0 + p1) / 2) @ rot
    bmesh.ops.create_cone(bm, cap_ends=True, segments=segments,
                          radius1=radius, radius2=radius, depth=d.length, matrix=m)


def lathe(bm, profile, segments=64, tile=1.0, uv_layer=None, cap_top=False, cap_bottom=False):
    """
    Surface of revolution around the Z axis. profile = [(radius, z), ...] from
    bottom to top. Edges between profile points are smooth; call lathe() once
    per section to get hard edges between sections.

    UVs (if uv_layer is given) are in texture tiles: u = arc length / tile,
    v = z / tile, so a texture keeps the same size everywhere. u = 0 faces the
    camera (-Y); the seam is at the back (+Y) where it can't be seen.
    """
    rings = []
    for r, z in profile:
        ring = []
        for i in range(segments):
            a = 2.0 * math.pi * i / segments          # 0 = front (-Y), increasing towards +X
            ring.append(bm.verts.new((r * math.sin(a), -r * math.cos(a), z)))
        rings.append(ring)

    faces = []
    for k in range(len(rings) - 1):
        r0 = rings[k]
        r1 = rings[k + 1]
        for i in range(segments):
            j = (i + 1) % segments
            faces.append(bm.faces.new((r0[i], r0[j], r1[j], r1[i])))
    if cap_top and profile[-1][0] > 0:
        faces.append(bm.faces.new(rings[-1]))
    if cap_bottom and profile[0][0] > 0:
        faces.append(bm.faces.new(list(reversed(rings[0]))))

    if uv_layer is not None:
        for f in faces:
            angles = [math.atan2(l.vert.co.x, -l.vert.co.y) for l in f.loops]
            if max(angles) - min(angles) > math.pi:        # face straddles the seam
                angles = [a + 2.0 * math.pi if a < 0 else a for a in angles]
            for l, a in zip(f.loops, angles):
                r = math.hypot(l.vert.co.x, l.vert.co.y)
                l[uv_layer].uv = (a * r / tile, l.vert.co.z / tile)
    return faces


def mesh_object(name, bm, material=None, smooth_quads=False):
    """Turn a bmesh into a linked object. smooth_quads smooths bar sides but not end caps."""
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    if smooth_quads:
        for poly in me.polygons:
            poly.use_smooth = (len(poly.vertices) == 4)
    ob = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(ob)
    if material is not None:
        me.materials.append(material)
    return ob


def box_object(name, center, size, material=None, rotation=(0.0, 0.0, 0.0), bevel_width=0.0):
    """One box as its own object (handy for parts that are rotated or tilted)."""
    bm = bmesh.new()
    box(bm, (0.0, 0.0, 0.0), size)
    ob = mesh_object(name, bm, material)
    ob.location = center
    ob.rotation_euler = rotation
    if bevel_width > 0:
        bevel(ob, bevel_width)
    return ob


def text_object(name, body, size, materials, location=(0, 0, 0), rotation=(math.pi / 2, 0, 0),
                neon=False, tube=0.06, extrude=0.08, font_path=None, align='CENTER'):
    """
    Text, facing the camera (-Y) by default.
    neon=True makes glowing tubes along the letter outlines; otherwise solid letters.
    materials: list of materials; set per-character slots afterwards with
    ob.data.body_format[i].material_index.
    """
    cu = bpy.data.curves.new(name, 'FONT')
    cu.body = body
    cu.size = size
    cu.align_x = align
    cu.align_y = 'CENTER'
    if font_path and os.path.exists(font_path):
        cu.font = bpy.data.fonts.load(font_path, check_existing=True)
    if neon:
        cu.fill_mode = 'NONE'
        cu.extrude = 0.0
        cu.bevel_depth = tube
        cu.bevel_resolution = 3
    else:
        cu.extrude = extrude
        cu.bevel_depth = 0.01
    for m in materials:
        cu.materials.append(m)
    ob = bpy.data.objects.new(name, cu)
    bpy.context.scene.collection.objects.link(ob)
    ob.location = location
    ob.rotation_euler = rotation
    return ob


def point_light(name, location, color=(1, 1, 1), energy=100.0, radius=0.2):
    data = bpy.data.lights.new(name, 'POINT')
    data.color = color
    data.energy = energy
    data.shadow_soft_size = radius
    ob = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(ob)
    ob.location = location
    return ob


def parent_keep_transform(children, parent):
    """Parent objects without moving them (parent should be at its rest transform)."""
    bpy.context.view_layer.update()
    for ch in children:
        mw = ch.matrix_world.copy()
        ch.parent = parent
        ch.matrix_parent_inverse = parent.matrix_world.inverted()
        ch.matrix_world = mw


def bevel(ob, width=0.05, segments=2):
    mod = ob.modifiers.new("Bevel", 'BEVEL')
    mod.width = width
    mod.segments = segments
    mod.limit_method = 'ANGLE'
    return mod


# ---------------------------------------------------------------------------
# Image post-processing
# ---------------------------------------------------------------------------

def _load_pixels(path):
    import numpy as np
    img = bpy.data.images.load(path, check_existing=False)
    w, h = img.size
    px = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(px)
    bpy.data.images.remove(img)
    return px.reshape(h, w, 4), w, h


def _box_blur(a, r):
    """Box blur of a 2D array with radius r (edges clamped), via cumulative sums."""
    import numpy as np
    if r <= 0:
        return a
    for axis in (0, 1):
        pad = [(0, 0), (0, 0)]
        pad[axis] = (r + 1, r)
        p = np.pad(a, pad, mode='edge')
        c = np.cumsum(p, axis=axis)
        n = a.shape[axis]
        if axis == 0:
            a = (c[2 * r + 1:2 * r + 1 + n, :] - c[0:n, :]) / (2 * r + 1)
        else:
            a = (c[:, 2 * r + 1:2 * r + 1 + n] - c[:, 0:n]) / (2 * r + 1)
    return a


def merge_unchanged(base_path, variant_path, t0=0.012, t1=0.05, blur_px=4):
    """
    Rewrite variant_path so that every pixel that differs from base_path by
    less than about t0 (render noise) is taken from base_path. Differences
    above t1 are kept fully; the mask is softened by blur_px pixels so there
    is no visible edge.
    """
    import numpy as np
    base, w, h = _load_pixels(base_path)
    var, w2, h2 = _load_pixels(variant_path)
    if (w, h) != (w2, h2):
        print("sprite_kit: merge_unchanged: size mismatch, skipped")
        return
    diff = np.abs(var - base).max(axis=2)
    mask = np.clip((diff - t0) / (t1 - t0), 0.0, 1.0)
    # Grow and soften the mask so lit areas fade out smoothly
    soft = _box_blur(_box_blur(mask, blur_px), blur_px)
    mask = np.clip(np.maximum(mask, soft * 3.0), 0.0, 1.0)[:, :, None]
    out = base * (1.0 - mask) + var * mask

    img = bpy.data.images.new("merged", w, h, alpha=True)
    img.pixels.foreach_set(out.astype(np.float32).ravel())
    img.filepath_raw = variant_path
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)
    changed = float((mask > 0.01).mean()) * 100.0
    print("sprite_kit: %s differs from %s in %.1f %% of the image"
          % (os.path.basename(variant_path), os.path.basename(base_path), changed))


# ---------------------------------------------------------------------------
# Kit
# ---------------------------------------------------------------------------

class Kit:
    """Owns the scene for one sprite script."""

    def __init__(self, args, rig="ortho"):
        self.args = args
        self.rig = rig
        reset_scene()
        self.scene = bpy.context.scene
        setup_render(self.scene, args)
        self.theme = importlib.import_module("themes." + args.theme)
        self.theme.setup_world(self.scene, MODELS_DIR, hdri=args.hdri)
        self.theme.setup_lights(self.scene, rig)
        self._materials = {}

    def material(self, role):
        """Material for a role (e.g. 'frame_main'), as defined by the theme."""
        if role not in self._materials:
            self._materials[role] = self.theme.make_material(role, MODELS_DIR)
        return self._materials[role]

    def render_persp(self, name, target, yaw_deg=18.0, pitch_deg=6.0, lens=50.0, distance=90.0,
                     ground_z=0.0, margin_px=2):
        """
        Perspective render for static scenery (3/4 view like the silo).

        The camera looks at `target` from `distance` units away, turned yaw_deg
        to the right (seeing the object's right side) and pitched pitch_deg down.
        The frame is fitted tightly around all geometry above ground_z, and the
        resolution is chosen so that one unit at the target's depth is PPU
        pixels. The object's size in the editor then comes out right with
        "Set Width and Height from texture pixel size".
        """
        scene = self.scene
        yaw = math.radians(yaw_deg)
        pitch = math.radians(pitch_deg)

        cam_data = bpy.data.cameras.new(name + "_cam")
        cam_data.type = 'PERSP'
        cam_data.lens = lens
        cam_data.clip_start = 0.5
        cam_data.clip_end = distance * 5.0
        cam = bpy.data.objects.new(name + "_cam", cam_data)
        scene.collection.objects.link(cam)
        cam.rotation_euler = (math.pi / 2 - pitch, 0.0, yaw)
        forward = Vector((-math.sin(yaw) * math.cos(pitch), math.cos(yaw) * math.cos(pitch), -math.sin(pitch)))
        cam.location = Vector(target) - forward * distance
        scene.camera = cam
        bpy.context.view_layer.update()

        # Project every vertex above ground onto the camera's image plane
        # (tangent space: x/-z, y/-z in camera coordinates).
        to_cam = cam.matrix_world.inverted()
        dg = bpy.context.evaluated_depsgraph_get()
        umin = vmin = float("inf")
        umax = vmax = float("-inf")
        for ob in scene.objects:
            if ob.type not in ('MESH', 'CURVE', 'FONT') or ob.hide_render:
                continue
            ev = ob.evaluated_get(dg)
            me = ev.to_mesh()
            mw = ob.matrix_world
            for v in me.vertices:
                w = mw @ v.co
                if w.z < ground_z:
                    continue
                c = to_cam @ w
                if c.z >= -0.01:
                    continue
                u = c.x / -c.z
                vv = c.y / -c.z
                umin = min(umin, u)
                umax = max(umax, u)
                vmin = min(vmin, vv)
                vmax = max(vmax, vv)
            ev.to_mesh_clear()

        uspan = umax - umin
        vspan = vmax - vmin
        depth = distance
        ppu = self.args.ppu
        res_x = uspan * depth * ppu + 2 * margin_px
        res_y = vspan * depth * ppu + 2 * margin_px
        if max(res_x, res_y) > self.args.max_px:
            scale = self.args.max_px / max(res_x, res_y)
            ppu = math.floor(ppu * scale * 10) / 10.0
            res_x = uspan * depth * ppu + 2 * margin_px
            res_y = vspan * depth * ppu + 2 * margin_px
            print("sprite_kit: WARNING %s too large for %d px, using %g px/unit. "
                  "Enter its size by hand in the editor." % (name, self.args.max_px, ppu))
        res_x = int(math.ceil(res_x))
        res_y = int(math.ceil(res_y))

        # Fit the frame: grow the spans by the pixel margin, match the sensor to
        # the larger side, and shift the frame centre onto the geometry.
        uspan_m = uspan * res_x / (res_x - 2 * margin_px)
        vspan_m = vspan * res_y / (res_y - 2 * margin_px)
        uc = (umin + umax) / 2
        vc = (vmin + vmax) / 2
        if res_x >= res_y:
            cam_data.sensor_fit = 'HORIZONTAL'
            cam_data.sensor_width = lens * uspan_m
            big = uspan_m
        else:
            cam_data.sensor_fit = 'VERTICAL'
            cam_data.sensor_height = lens * vspan_m
            big = vspan_m
        cam_data.shift_x = uc / big
        cam_data.shift_y = vc / big

        scene.render.resolution_x = res_x
        scene.render.resolution_y = res_y

        return self._finish(name, res_x, res_y, ppu,
                            "size %.2f x %.2f units at the target's depth" % (res_x / ppu, res_y / ppu))

    def _finish(self, name, res_x, res_y, ppu, size_text):
        if self.args.save_blend:
            os.makedirs(BLEND_OUT, exist_ok=True)
            path = os.path.join(BLEND_OUT, name + ".blend")
            bpy.ops.wm.save_as_mainfile(filepath=path)
            print("sprite_kit: saved %s" % path)

        if self.args.no_render:
            return None

        os.makedirs(self.args.out, exist_ok=True)
        out_path = os.path.join(self.args.out, name + ".png")
        self.scene.render.filepath = out_path
        bpy.ops.render.render(write_still=True)
        self._last = (out_path, res_x, res_y, ppu, size_text)

        print("")
        print("sprite_kit: %s" % out_path)
        print("sprite_kit:   %s, %d x %d px, %g px/unit" % (size_text, res_x, res_y, ppu))
        print("")
        return out_path

    def render_variant(self, name, keep_unchanged=True):
        """
        Render the scene again with the same camera and frame (after the script
        has changed something, e.g. switched a light off) to <name>.png.

        keep_unchanged: pixels that differ from the previous render only by
        render noise are copied from it, so the two images can be swapped as
        animation frames without the rest of the sprite shimmering.
        """
        if self.args.no_render or not getattr(self, "_last", None):
            return None
        base_path, res_x, res_y, ppu, size_text = self._last
        out_path = os.path.join(self.args.out, name + ".png")
        self.scene.render.filepath = out_path
        bpy.ops.render.render(write_still=True)
        if keep_unchanged:
            merge_unchanged(base_path, out_path)
        print("")
        print("sprite_kit: %s (variant of %s)" % (out_path, os.path.basename(base_path)))
        print("sprite_kit:   %s, %d x %d px, %g px/unit" % (size_text, res_x, res_y, ppu))
        print("")
        return out_path

    def render_ortho(self, name, box_xz):
        """
        Render the front view of everything in the scene, framed exactly to
        box_xz = (xmin, zmin, xmax, zmax) in Blender units.
        """
        xmin, zmin, xmax, zmax = box_xz
        w = xmax - xmin
        h = zmax - zmin

        ppu = self.args.ppu
        if max(w, h) * ppu > self.args.max_px:
            ppu = float(math.floor(self.args.max_px / max(w, h)))
            print("sprite_kit: WARNING %s is too large for %d px at %g px/unit, "
                  "using %g px/unit instead. Enter its size by hand in the editor "
                  "(don't use 'set size from texture')." % (name, self.args.max_px, self.args.ppu, ppu))

        res_x = max(1, int(round(w * ppu)))
        res_y = max(1, int(round(h * ppu)))

        cam_data = bpy.data.cameras.new(name + "_cam")
        cam_data.type = 'ORTHO'
        cam_data.ortho_scale = max(res_x, res_y) / ppu   # covers the longer image side
        cam_data.sensor_fit = 'AUTO'
        cam_data.clip_start = 0.1
        cam_data.clip_end = 1000.0
        cam = bpy.data.objects.new(name + "_cam", cam_data)
        self.scene.collection.objects.link(cam)
        cam.location = ((xmin + xmax) / 2, -300.0, (zmin + zmax) / 2)
        cam.rotation_euler = (math.pi / 2, 0.0, 0.0)          # look along +Y, Z up
        self.scene.camera = cam

        self.scene.render.resolution_x = res_x
        self.scene.render.resolution_y = res_y

        if self.args.save_blend:
            os.makedirs(BLEND_OUT, exist_ok=True)
            path = os.path.join(BLEND_OUT, name + ".blend")
            bpy.ops.wm.save_as_mainfile(filepath=path)
            print("sprite_kit: saved %s" % path)

        if self.args.no_render:
            return None

        os.makedirs(self.args.out, exist_ok=True)
        out_path = os.path.join(self.args.out, name + ".png")
        self.scene.render.filepath = out_path
        bpy.ops.render.render(write_still=True)

        print("")
        print("sprite_kit: %s" % out_path)
        print("sprite_kit:   size %.3f x %.3f units, %d x %d px, %g px/unit"
              % (w, h, res_x, res_y, ppu))
        print("")
        return out_path
