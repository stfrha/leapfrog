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
  GlobalConstants.TexturePixelsPerUnit in leapfrog-editor; a script can set
  its own default, e.g. 10 for ambient pieces). A part whose longest side
  would exceed --max-px is rendered at the largest PPU that fits, and a
  warning is printed.
* The PPU an image was rendered at is written to its resource entry
  (ppu="10"), and updated when it is re-rendered at another density. The
  editor's "Set Width and Height from texture pixel size" reads it; oxygine
  ignores it.
* The image frame is exactly the box you pass to render_ortho(), not the
  visible pixels, so the sprite lines up with the width x height body box.
* Output: <name>.png straight into leapfrog-game/data/images/, and the
  texture is added to data/res.xml if it isn't listed yet. Every file in
  data/images ships with the game (Android packs all of data/, iOS has
  data/images as a folder reference).
* Animations: render each frame with frame_only=True (it goes to
  sprites/_frames/, neither shipped nor registered), then make_strip()
  combines them into the shipped, registered image with rows/cols. The
  editor shows an animation's first frame, so no editor-only image is
  needed. (editor_only=True still writes to leapfrog-game/editor_images/,
  which no build project includes.)
* Ambient pieces (small buildings etc. for parallax layers, named amb_*)
  use Kit(..., ambient=True): they go to data/images/<world>/ambient/ and
  share one atlas there, and are rendered with render_axo() so pieces can
  be placed side by side.
* Ambient pieces are rendered at night by default (the theme's moon and
  dark sky; --day for the old daylight), on a shadow catcher so they cast
  their shadow on the ground they will stand on (--no-shadow), with a
  little haze towards the night sky baked in (--haze 0..1) and a soft glow
  round lit windows and lamps (--glow). Animated pieces (smoke, blinking
  lights, turning wheels) use Kit.render_axo_animated(); --still renders one
  frame without animation.
* Repeating textures (polygon fills, polygon borders) use rig="tile" and
  render_tile(): a front view of one period of a scene the script builds
  to repeat, rendered with overscan and cropped, power-of-two, registered
  in its own <atlas clamp2edge="false">. tile_tools.py has the numpy
  post-processing (crop, neutral grey, levels, fades); see white_gravel.py.
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
EDITOR_OUT = os.path.join(GAME_DIR, "editor_images")      # editor-only images, not shipped
RES_XML = os.path.join(GAME_DIR, "data", "res.xml")
BLEND_OUT = os.path.join(KIT_DIR, "_blend")
FRAMES_OUT = os.path.join(KIT_DIR, "_frames")            # single frames before make_strip (not shipped)

DEFAULT_PPU = 20.0       # keep equal to GlobalConstants.TexturePixelsPerUnit
DEFAULT_MAX_PX = 2048    # safe texture / atlas size on older phones

if KIT_DIR not in sys.path:
    sys.path.insert(0, KIT_DIR)

import project_files  # noqa: E402  (data files and Xcode projects)


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def parse_args(add_extra=None):
    """Parse the options after '--'. add_extra(parser) can add script options."""
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser(prog="blender -b -P <sprite>.py --")
    p.add_argument("--theme", default="mining", help="world theme in themes/ (default: mining)")
    p.add_argument("--out", default=DEFAULT_OUT, help="output folder (default: leapfrog-game/data/images)")
    p.add_argument("--turn", type=float, default=None,
                   help="ambient pieces: turn the piece this many degrees about its vertical axis "
                        "(+ = counter-clockwise seen from above; default: the script's own)")
    p.add_argument("--scale", type=float, default=None,
                   help="ambient pieces: build size x this (default: the script's own, usually 1)")
    p.add_argument("--ppu", type=float, default=None,
                   help="pixels per game unit (default 20, or the script's own default)")
    p.add_argument("--max-px", type=int, default=DEFAULT_MAX_PX, help="max image side in pixels (default 2048)")
    p.add_argument("--samples", type=int, default=128, help="Cycles samples (default 128, use ~16 for previews)")
    p.add_argument("--gpu", action="store_true", help="render on the GPU if one is available")
    p.add_argument("--hdri", default=None, help="path to an HDRI, overrides the theme's default")
    p.add_argument("--save-blend", action="store_true", help="also save the generated scene to sprites/_blend/")
    p.add_argument("--no-render", action="store_true", help="build the scene only (use with --save-blend)")
    p.add_argument("--folder", default=None,
                   help="image subfolder under data/images and editor_images (default: from the resource "
                        "file, res_mining.xml -> mining, res.xml -> common)")
    p.add_argument("--res", default=None,
                   help="resource file to register textures in (default set by the script, else res.xml). "
                        "Scenery belongs in a per-world set like res_mining.xml")
    p.add_argument("--no-register", action="store_true",
                   help="don't add new textures to data/res.xml (never done for *_test names)")
    p.add_argument("--night", action="store_true", help="night lighting (the default for ambient pieces)")
    p.add_argument("--day", action="store_true", help="daylight, also for ambient pieces")
    p.add_argument("--moon", type=float, default=None, help="night: moon strength (default: the theme's, 0.7)")
    p.add_argument("--haze", type=float, default=None,
                   help="ambient pieces: blend this much towards the night sky colour, 0..1 (default 0.18 at night)")
    p.add_argument("--glow", type=float, default=None,
                   help="ambient pieces: glow round lit windows and lamps (default 1 at night, 0 = none)")
    p.add_argument("--no-shadow", action="store_true", help="ambient pieces: no shadow on the ground")
    p.add_argument("--still", action="store_true", help="animated pieces: one image, no animation")
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


def make_strip(frame_paths, out_path, layout="auto", max_px=DEFAULT_MAX_PX):
    """
    Combine equally sized frames into one image for an oxygine animation
    (<image file="..." cols="C" rows="R"/> in res.xml; frames run left to
    right, then top to bottom). Frame 0 is top left. layout: "cols" (one
    row), "rows" (one column), or "auto": the arrangement, including grids,
    whose longest side is smallest (so it fits within max_px if any does).
    Returns (cols, rows).
    """
    import numpy as np
    frames = [_load_pixels(p) for p in frame_paths]
    w, h = frames[0][1], frames[0][2]
    if any((f[1], f[2]) != (w, h) for f in frames):
        raise ValueError("make_strip: frames have different sizes")
    n = len(frames)
    if layout == "cols":
        cols, rows = n, 1
    elif layout == "rows":
        cols, rows = 1, n
    else:
        options = [(c, n // c) for c in range(1, n + 1) if n % c == 0]
        cols, rows = min(options, key=lambda cr: (max(w * cr[0], h * cr[1]), w * cr[0] * h * cr[1]))
    # Blender's pixel rows run bottom-up, so the top row of frames goes last
    bands = []
    for r in range(rows):
        bands.append(np.concatenate([frames[r * cols + c][0] for c in range(cols)], axis=1))
    data = np.concatenate(list(reversed(bands)), axis=0)
    sh, sw = data.shape[0], data.shape[1]
    img = bpy.data.images.new("strip", sw, sh, alpha=True)
    img.pixels.foreach_set(data.astype(np.float32).ravel())
    img.filepath_raw = out_path
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)

    print("")
    print("sprite_kit: %s, %d frames (%d x %d), %d x %d px" % (out_path, n, cols, rows, sw, sh))
    if max(sw, sh) > max_px:
        print("sprite_kit: WARNING the animation image is larger than %d px" % max_px)
    print("sprite_kit:   resource entry: <image file=\"%s\" %s/>" % (os.path.basename(out_path), strip_attrs(cols, rows)))
    print("")
    return cols, rows


def strip_attrs(cols, rows):
    """cols="C" rows="R" for an animation's resource entry (only those above 1)."""
    parts = []
    if cols > 1:
        parts.append('cols="%d"' % cols)
    if rows > 1:
        parts.append('rows="%d"' % rows)
    return " ".join(parts)


def register_texture(filename, attrs="", res_xml=RES_XML, folder="images"):
    """Add an image to a resource file (see project_files.register_texture)."""
    return project_files.register_texture(os.path.basename(res_xml), filename, attrs, folder)


# ---------------------------------------------------------------------------
# Kit
# ---------------------------------------------------------------------------

class Kit:
    """Owns the scene for one sprite script."""

    def __init__(self, args, rig="ortho", res=None, ambient=False, ppu=None, scale=None, turn=None):
        """res: resource file this script's textures belong in (default res.xml).
        Big scenery should go in a per-world set (e.g. res_mining.xml) that is
        only loaded while a scene using it runs; see data/scene_resources.xml.

        ambient: small background pieces for parallax layers. They go in the
        world's ambient/ subfolder and share one atlas (project_files).
        ppu: the script's default pixels per unit (--ppu overrides it).
        scale: render_axo scales the whole piece by this before framing
        (--scale overrides it). A piece on a far layer can be modelled at a
        natural size and still come out big enough on screen, with enough
        pixels, instead of being stretched in the editor.
        turn: render_axo turns the piece this many degrees about the vertical
        axis through the origin (--turn overrides it). The camera stays the
        same for every piece, so a turned house just shows more of one side,
        as if it was put down without lining it up with its neighbours.
        For a parallax layer with parallaxAmount a, about 20 * (1 - a) is
        enough (0.6 -> 8-10)."""
        self.args = args
        self.rig = rig
        self.ambient = ambient
        self.scale = args.scale if args.scale else (scale if scale else 1.0)
        self.turn = args.turn if args.turn is not None else (turn if turn else 0.0)
        if args.ppu is None:
            args.ppu = float(ppu) if ppu else DEFAULT_PPU
        self.res = args.res or res or "res.xml"
        if args.folder:
            self.folder = args.folder.replace("\\", "/").strip("/")
        elif self.res.startswith("res_") and self.res.endswith(".xml"):
            self.folder = self.res[len("res_"):-len(".xml")]
        else:
            self.folder = "common"
        if ambient and not args.folder:
            self.folder += "/ambient"
        self.night = (args.night or (ambient and rig == "axo")) and not args.day
        reset_scene()
        self.scene = bpy.context.scene
        setup_render(self.scene, args)
        self.theme = importlib.import_module("themes." + args.theme)
        if self.night:
            self.theme.setup_world(self.scene, MODELS_DIR, hdri=args.hdri, night=True)
            self.theme.setup_lights(self.scene, rig, night=True, moon=args.moon)
        else:
            self.theme.setup_world(self.scene, MODELS_DIR, hdri=args.hdri)
            self.theme.setup_lights(self.scene, rig)
        self._materials = {}
        self._post_axo = False          # render_axo: haze and glow after each render
        self._raw_base = None           # unprocessed first frame, for render_variant
        if ambient and rig == "axo":
            self.haze = args.haze if args.haze is not None else (0.18 if self.night else 0.0)
            self.glow = args.glow if args.glow is not None else (1.0 if self.night else 0.0)
            self.shadow = not args.no_shadow
        else:
            self.haze = args.haze or 0.0
            self.glow = args.glow or 0.0
            self.shadow = False

    def material(self, role):
        """Material for a role (e.g. 'frame_main'), as defined by the theme."""
        if role not in self._materials:
            self._materials[role] = self.theme.make_material(role, MODELS_DIR)
        return self._materials[role]

    def render_persp(self, name, target, yaw_deg=18.0, pitch_deg=6.0, lens=50.0, distance=90.0,
                     ground_z=0.0, margin_px=2, editor_only=False, frame_only=False):
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
            print("sprite_kit: NOTE %s too large for %d px, using %g px/unit "
                  "(written to its resource entry for the editor)." % (name, self.args.max_px, ppu))
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
                            "size %.2f x %.2f units at the target's depth" % (res_x / ppu, res_y / ppu),
                            editor_only=editor_only, frame_only=frame_only)

    def _custom_out(self):
        return os.path.normcase(os.path.abspath(self.args.out)) != os.path.normcase(os.path.abspath(DEFAULT_OUT))

    def _out_dir(self, name, editor_only, frame_only=False):
        """
        Where <name>.png goes:
          editor-only images      editor_images/<folder>/
          custom --out            that folder
          already registered      wherever its resource entry points (so re-renders never break it)
          new                     data/images/<folder>/
        """
        if frame_only:
            return FRAMES_OUT
        if editor_only:
            return os.path.join(EDITOR_OUT, self.folder)
        if self._custom_out():
            return self.args.out
        res, folder = project_files.find_texture(name + ".png")
        if res is not None:
            if folder.replace("\\", "/").strip("/") != "images/" + self.folder:
                print("sprite_kit: NOTE %s.png stays in data/%s (listed in %s). To move it:" % (name, folder, res))
                print("sprite_kit:   python project_files.py move-texture images/%s %s --res %s"
                      % (self.folder, name, self.res))
            return os.path.join(GAME_DIR, "data", folder)
        return os.path.join(DEFAULT_OUT, self.folder)

    def _may_register(self, name):
        # Test renders are never registered, including images derived from
        # them (e.g. ore_building_test_blink from make_strip)
        if self.args.no_register or name.endswith("_test") or "_test_" in name:
            return False
        if self._custom_out():
            return False
        return True

    def register(self, filename, attrs="", ppu=None, repeat=False, frames=None):
        """
        Add a shipped texture to the script's resource file (skipped for test
        renders / custom --out). ppu is written as an attribute; for a texture
        that is already listed only its ppu is updated. repeat=True puts it
        in its own <atlas clamp2edge="false"> (textures that tile on polygons
        and polygon borders; they must be power-of-two).
        frames=(cols, rows): also keep an existing entry's cols/rows right,
        e.g. when a piece that was a single image becomes an animation, or
        the other way round ((1, 1) removes them).
        """
        if not self._may_register(os.path.splitext(filename)[0]):
            return
        ppu_text = ("%g" % ppu) if ppu else None
        if ppu_text:
            attrs = ('ppu="%s" ' % ppu_text) + (attrs or "")
        if self.res != "res.xml":
            project_files.ensure_res_file(self.res)
            if not project_files.scenes_using(self.res):
                print("sprite_kit: NOTE no scene uses %s yet. Map scenes to it with:" % self.res)
                print("sprite_kit:   python project_files.py add-res %s --scenes <scene>.xml" % self.res)
            if os.path.exists(RES_XML) and ('file="%s"' % filename) in open(RES_XML, encoding="utf-8").read():
                print("sprite_kit: NOTE %s is also listed in res.xml; remove it there so it isn't "
                      "loaded for the whole game" % filename)
        if project_files.find_texture(filename)[0] is None:
            project_files.register_texture(self.res, filename, attrs, "images/" + self.folder, repeat=repeat)
        else:
            if ppu_text:
                project_files.set_image_attr(filename, "ppu", ppu_text)
            if frames is not None:
                cols, rows = frames
                project_files.set_image_attr(filename, "cols", str(cols) if cols > 1 else None)
                project_files.set_image_attr(filename, "rows", str(rows) if rows > 1 else None)

    def make_strip(self, frame_paths, name, layout="auto"):
        """Combine frames into <name>.png (shipped) and register it as an animation."""
        out_dir = self._out_dir(name, False)
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, name + ".png")
        cols, rows = make_strip(frame_paths, out_path, layout, self.args.max_px)
        ppu = self._last[3] if getattr(self, "_last", None) else self.args.ppu
        self.register(name + ".png", strip_attrs(cols, rows), ppu=ppu, frames=(cols, rows))
        return out_path

    def _finish(self, name, res_x, res_y, ppu, size_text, editor_only=False, frame_only=False):
        if self.args.save_blend:
            os.makedirs(BLEND_OUT, exist_ok=True)
            path = os.path.join(BLEND_OUT, name + ".blend")
            bpy.ops.wm.save_as_mainfile(filepath=path)
            print("sprite_kit: saved %s" % path)

        if self.args.no_render:
            return None

        out_dir = self._out_dir(name, editor_only, frame_only)
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, name + ".png")
        self.scene.render.filepath = out_path
        bpy.ops.render.render(write_still=True)
        self._last = (out_path, res_x, res_y, ppu, size_text)
        if self._post_axo:
            self._raw_base = self._keep_raw(out_path, name)
            self._post_process(out_path, ppu)

        print("")
        print("sprite_kit: %s%s" % (out_path, "  (frame, combined by make_strip)" if frame_only else
                                    ("  (editor only, not shipped)" if editor_only else "")))
        print("sprite_kit:   %s, %d x %d px, %g px/unit" % (size_text, res_x, res_y, ppu))
        print("")
        if not editor_only and not frame_only:
            self.register(name + ".png", ppu=ppu, frames=(1, 1))
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
        out_path = os.path.join(os.path.dirname(base_path), name + ".png")
        self.scene.render.filepath = out_path
        bpy.ops.render.render(write_still=True)
        if keep_unchanged:
            # Compare with the first frame as rendered, before haze and glow
            merge_unchanged(self._raw_base or base_path, out_path)
        if self._post_axo:
            self._post_process(out_path, ppu)
        print("")
        print("sprite_kit: %s (variant of %s)" % (out_path, os.path.basename(base_path)))
        print("sprite_kit:   %s, %d x %d px, %g px/unit" % (size_text, res_x, res_y, ppu))
        print("")
        return out_path

    def render_ortho(self, name, box_xz, editor_only=False, frame_only=False):
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
            print("sprite_kit: NOTE %s is too large for %d px at %g px/unit, "
                  "using %g px/unit instead (written to its resource entry for the editor)."
                  % (name, self.args.max_px, self.args.ppu, ppu))

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

        return self._finish(name, res_x, res_y, ppu, "size %.3f x %.3f units" % (w, h),
                            editor_only=editor_only, frame_only=frame_only)

    def render_axo(self, name, yaw_deg=10.0, pitch_deg=12.0, ground_z=0.0, margin_px=2, editor_only=False,
                   frame_only=False):
        """
        Orthographic 3/4 view for ambient pieces (render with rig="axo").

        The camera is turned yaw_deg to the right (seeing the right side) and
        pitched pitch_deg down, but has no perspective, so every piece rendered
        with the same angles has the same projection and pieces can stand side
        by side anywhere on a layer. Keep yaw small: with yaw 0 the front edge
        of the footprint is a level line in the image.

        Framing: tight around all geometry above ground_z, with margin_px on
        the top, left and right. The bottom edge of the image is the lowest
        ground point (the front of the footprint), so pieces placed on the
        same line in the editor share a ground line.
        """
        scene = self.scene
        if self.scale != 1.0 or self.turn:
            self._transform_scene(self.scale, self.turn)
        if self.shadow:
            self._add_shadow_catcher(ground_z)
        self._post_axo = (self.haze > 0.0) or (self.glow > 0.0)
        yaw = math.radians(yaw_deg)
        pitch = math.radians(pitch_deg)

        cam_data = bpy.data.cameras.new(name + "_cam")
        cam_data.type = 'ORTHO'
        cam_data.sensor_fit = 'AUTO'
        cam_data.clip_start = 0.1
        cam_data.clip_end = 2000.0
        cam = bpy.data.objects.new(name + "_cam", cam_data)
        scene.collection.objects.link(cam)
        cam.rotation_euler = (math.pi / 2 - pitch, 0.0, yaw)
        forward = Vector((-math.sin(yaw) * math.cos(pitch), math.cos(yaw) * math.cos(pitch), -math.sin(pitch)))
        cam.location = -forward * 500.0
        scene.camera = cam
        bpy.context.view_layer.update()

        # Project every vertex above ground onto the camera plane, and where
        # it casts its shadow on the ground (so the shadow isn't cut off)
        to_cam = cam.matrix_world.inverted()
        light = self._key_light_direction() if self.shadow else None
        dg = bpy.context.evaluated_depsgraph_get()
        umin = vmin = float("inf")
        umax = vmax = float("-inf")
        shadow_v = []
        for ob in scene.objects:
            if ob.type not in ('MESH', 'CURVE', 'FONT') or ob.get("kit_no_frame"):
                continue
            if ob.hide_render and not ob.get("kit_frame_extent"):
                continue
            ev = ob.evaluated_get(dg)
            me = ev.to_mesh()
            mw = ob.matrix_world
            for v in me.vertices:
                w = mw @ v.co
                if w.z < ground_z - 1e-4:
                    continue
                c = to_cam @ w
                umin = min(umin, c.x)
                umax = max(umax, c.x)
                vmin = min(vmin, c.y)
                vmax = max(vmax, c.y)
                if light is not None and w.z > ground_z + 1e-3:
                    g = w + light * ((w.z - ground_z) / -light.z)
                    shadow_v.append(to_cam @ g)
            ev.to_mesh_clear()
        for c in shadow_v:
            # Shadows fall behind the piece; they may widen the frame and
            # raise its top, but the bottom stays the front of the footprint
            umin = min(umin, c.x)
            umax = max(umax, c.x)
            vmax = max(vmax, c.y)
        if umin == float("inf"):
            raise RuntimeError("render_axo: no geometry above ground_z")

        uspan = umax - umin
        vspan = vmax - vmin
        ppu = self.args.ppu
        res_x = uspan * ppu + 2 * margin_px
        res_y = vspan * ppu + margin_px
        if max(res_x, res_y) > self.args.max_px:
            ppu = math.floor((self.args.max_px - 2 * margin_px) / max(uspan, vspan) * 10) / 10.0
            print("sprite_kit: NOTE %s too large for %d px, using %g px/unit "
                  "(written to its resource entry for the editor)." % (name, self.args.max_px, ppu))
        res_x = int(math.ceil(uspan * ppu + 2 * margin_px))
        res_y = int(math.ceil(vspan * ppu + margin_px))

        # Frame in camera units: margins on top/left/right, the rounding slack
        # split left/right and put on top, bottom exactly at vmin.
        fw = res_x / ppu
        fh = res_y / ppu
        u0 = (umin + umax) / 2 - fw / 2
        v0 = vmin
        cam_data.ortho_scale = max(fw, fh)
        # Move the camera sideways so the frame centre is on its axis
        uc = u0 + fw / 2
        vc = v0 + fh / 2
        right = cam.matrix_world.to_3x3() @ Vector((1.0, 0.0, 0.0))
        up = cam.matrix_world.to_3x3() @ Vector((0.0, 1.0, 0.0))
        cam.location = cam.location + right * uc + up * vc

        scene.render.resolution_x = res_x
        scene.render.resolution_y = res_y

        return self._finish(name, res_x, res_y, ppu,
                            "size %.2f x %.2f units" % (fw, fh), editor_only=editor_only, frame_only=frame_only)

    def render_tile(self, name, origin_xz, size_px, overscan_px=16, post=None):
        """
        Front view (orthographic, looking along +Y) of a rectangle in the X/Z
        plane for a texture that repeats: polygon fills and polygon borders.
        Render with rig="tile".

        origin_xz: bottom-left corner of the rectangle in Blender units.
        size_px:   (width, height) of the image; both must be powers of two
                   (repeat-mode textures render black on iOS otherwise).
        The rectangle is size_px / PPU units. The script must build the
        scene so that it repeats with that period (e.g. the same geometry
        copied one period left, right, up and down); the frame is rendered
        overscan_px larger on every side and cropped, so the denoiser sees
        the neighbours of the edge pixels as well.

        post(img) -> img gets the cropped image as a numpy array (rows top
        down, RGBA 0..1, see tile_tools) and returns what is saved.
        The texture is registered in its own repeating atlas
        (clamp2edge="false") with its ppu. Returns the image path.
        """
        import numpy as np
        pw, ph = size_px
        for n in (pw, ph):
            if n <= 0 or (n & (n - 1)):
                raise ValueError("render_tile: %s must be a power of two in both directions, got %d x %d"
                                 % (name, pw, ph))
        ppu = self.args.ppu
        w = pw / ppu
        h = ph / ppu
        m = int(overscan_px)
        res_x = pw + 2 * m
        res_y = ph + 2 * m

        cam_data = bpy.data.cameras.new(name + "_cam")
        cam_data.type = 'ORTHO'
        cam_data.ortho_scale = max(res_x, res_y) / ppu
        cam_data.sensor_fit = 'AUTO'
        cam_data.clip_start = 0.1
        cam_data.clip_end = 1000.0
        cam = bpy.data.objects.new(name + "_cam", cam_data)
        self.scene.collection.objects.link(cam)
        cam.location = (origin_xz[0] + w / 2, -300.0, origin_xz[1] + h / 2)
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

        # Render with overscan to _frames/ (not shipped), then crop
        os.makedirs(FRAMES_OUT, exist_ok=True)
        raw_path = os.path.join(FRAMES_OUT, name + "_overscan.png")
        self.scene.render.filepath = raw_path
        bpy.ops.render.render(write_still=True)

        import tile_tools
        px, _w, _h = _load_pixels(raw_path)
        img = px[::-1]                                   # top row first
        img = tile_tools.crop(img, m, m, pw, ph)
        if post is not None:
            img = post(img)

        out_dir = self._out_dir(name, False)
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, name + ".png")
        out = bpy.data.images.new(name, pw, ph, alpha=True)
        out.pixels.foreach_set(np.ascontiguousarray(img[::-1]).astype(np.float32).ravel())
        out.filepath_raw = out_path
        out.file_format = 'PNG'
        out.save()
        bpy.data.images.remove(out)
        sx, sy = tile_tools.seam_error(img)
        self._last = (out_path, pw, ph, ppu, "tile %.2f x %.2f units" % (w, h))

        print("")
        print("sprite_kit: %s" % out_path)
        print("sprite_kit:   repeating texture, %d x %d px, %g px/unit = %g x %g units per repeat"
              % (pw, ph, ppu, w, h))
        print("sprite_kit:   seam check (1 = seams like the rest of the image): x %.2f, y %.2f" % (sx, sy))
        print("")
        self.register(name + ".png", ppu=ppu, repeat=True)
        return out_path

    def render_axo_animated(self, name, frames, set_frame, period=None, yaw_deg=10.0, pitch_deg=12.0,
                            ground_z=0.0):
        """
        An ambient piece with an animation (smoke, a blinking lamp, turning
        wheels): set_frame(i) puts the scene in frame i of `frames`; frame 0
        is rendered with render_axo, the others with render_variant (same
        camera; pixels that only differ by noise are taken from frame 0),
        and make_strip makes <name>.png with cols/rows. period: seconds per
        loop, printed as the Animate Period to set in the editor.
        With --still, or frames < 2, set_frame(None) asks for the still look
        (lamps on, smoke as in frame 0) and a single image is rendered.
        """
        if self.args.still or frames < 2:
            set_frame(None)
            return self.render_axo(name, yaw_deg=yaw_deg, pitch_deg=pitch_deg, ground_z=ground_z)
        set_frame(0)
        paths = [self.render_axo(name + "_f0", yaw_deg=yaw_deg, pitch_deg=pitch_deg, ground_z=ground_z,
                                 frame_only=True)]
        for i in range(1, frames):
            set_frame(i)
            paths.append(self.render_variant(name + "_f%d" % i))
        if not all(paths):
            return None
        out = self.make_strip(paths, name)
        if period:
            print("sprite_kit: %s: %d frames; in the editor set Animate Period to %.2f s" % (name, frames, period))
        return out

    def _key_light_direction(self):
        """Direction the key light (the sun or moon, an object named Sun) travels, or None."""
        sun = self.scene.objects.get("Sun")
        if sun is None or sun.type != 'LIGHT':
            return None
        d = sun.matrix_world.to_3x3() @ Vector((0.0, 0.0, -1.0))
        if d.z > -0.05:
            return None
        return d.normalized()

    def _add_shadow_catcher(self, ground_z):
        """
        A large ground plane that is only visible where shadows fall on it (Cycles shadow catcher).

        A piece with a hole in the ground (a mine shaft) marks an object with
        ob["kit_catcher_hole"] = True (hidden, e.g. a flat outline of the
        opening): the plane then gets a hole of that outline, seen from
        above, so it doesn't cover what is below ground. The outline must be
        star-shaped around its centre (an oval is fine); one hole per piece.
        """
        holes = [o for o in self.scene.objects if o.type == 'MESH' and o.get("kit_catcher_hole")]
        if len(holes) > 1:
            raise RuntimeError("shadow catcher: only one kit_catcher_hole object is supported")
        bm = bmesh.new()
        if not holes:
            bmesh.ops.create_grid(bm, x_segments=1, y_segments=1, size=500.0)
        else:
            bpy.context.view_layer.update()
            mw = holes[0].matrix_world
            pts = [(mw @ v.co) for v in holes[0].data.vertices]
            cx = sum(p.x for p in pts) / len(pts)
            cy = sum(p.y for p in pts) / len(pts)
            pts.sort(key=lambda p: math.atan2(p.y - cy, p.x - cx))
            # A ring of quads from the outline out to a circle 500 units away
            inner = [bm.verts.new((p.x, p.y, 0.0)) for p in pts]
            outer = []
            for p in pts:
                a = math.atan2(p.y - cy, p.x - cx)
                outer.append(bm.verts.new((cx + 500.0 * math.cos(a), cy + 500.0 * math.sin(a), 0.0)))
            n = len(pts)
            for i in range(n):
                j = (i + 1) % n
                bm.faces.new((inner[i], outer[i], outer[j], inner[j]))      # facing up
            # The same ring as a holdout just under the ground, so whatever is
            # below ground outside the hole (the outside of the shaft walls)
            # can't be seen past the edge of the piece
            hold = bpy.data.materials.new("_ground_holdout")
            hold.use_nodes = True
            hold.node_tree.nodes.clear()
            hn = hold.node_tree.nodes.new("ShaderNodeHoldout")
            ho = hold.node_tree.nodes.new("ShaderNodeOutputMaterial")
            hold.node_tree.links.new(hn.outputs["Holdout"], ho.inputs["Surface"])
            hb = bm.copy()
            hob = mesh_object("_ground_holdout", hb, hold)
            hob.location = (0.0, 0.0, ground_z - 0.02)
            hob.visible_shadow = False
            hob["kit_no_frame"] = True
        ob = mesh_object("_shadow_catcher", bm)
        ob.location = (0.0, 0.0, ground_z)
        ob.is_shadow_catcher = True
        ob["kit_no_frame"] = True

    def _keep_raw(self, path, name):
        """A copy of a render as it came out of Blender (in _frames/), before post-processing."""
        import shutil
        os.makedirs(FRAMES_OUT, exist_ok=True)
        raw = os.path.join(FRAMES_OUT, name + "_raw.png")
        if os.path.abspath(raw) != os.path.abspath(path):
            shutil.copyfile(path, raw)
        return raw

    def _post_process(self, path, ppu):
        """Haze towards the night sky, then a glow round the brightest pixels (lit windows, lamps)."""
        import numpy as np
        px, w, h = _load_pixels(path)
        rgb = px[:, :, :3]
        a = px[:, :, 3]

        if self.haze > 0.0:
            c = np.array(getattr(self.theme, "NIGHT_HAZE_COLOR", (0.5, 0.55, 0.6)), dtype=np.float32)
            rgb = rgb * (1.0 - self.haze) + c[None, None, :] * self.haze

        if self.glow > 0.0:
            lum = (rgb * np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)).sum(axis=2)
            bright = np.clip((lum - 0.62) / 0.25, 0.0, 1.0) * a
            r = max(2, int(round(3.0 * ppu / 14.0)))
            glow = np.empty_like(rgb)
            for ch in range(3):
                src = rgb[:, :, ch] * bright
                glow[:, :, ch] = _box_blur(_box_blur(src, r), r) * 1.6 * self.glow
            prem = rgb * a[:, :, None] + glow
            a = np.maximum(a, np.clip(glow.max(axis=2), 0.0, 1.0))
            rgb = prem / np.maximum(a, 1e-6)[:, :, None]

        out = np.empty_like(px)
        out[:, :, :3] = np.clip(rgb, 0.0, 1.0)
        out[:, :, 3] = np.clip(a, 0.0, 1.0)
        img = bpy.data.images.new("post", w, h, alpha=True)
        img.pixels.foreach_set(out.astype(np.float32).ravel())
        img.filepath_raw = path
        img.file_format = 'PNG'
        img.save()
        bpy.data.images.remove(img)
        print("sprite_kit: %s: haze %.2f, glow %.2f" % (os.path.basename(path), self.haze, self.glow))

    def _transform_scene(self, factor, turn_deg=0.0):
        """
        Scale everything built so far by `factor` about the origin (the
        ground stays at z = 0) and turn it turn_deg about the Z axis. Point
        lights keep their look (energy x f^2, size x f); the sun and the
        world are not affected.
        """
        root = bpy.data.objects.new("_scale_root", None)
        self.scene.collection.objects.link(root)
        parts = [ob for ob in self.scene.objects
                 if ob is not root and ob.parent is None and ob.type not in ('CAMERA',)
                 and not (ob.type == 'LIGHT' and ob.data.type == 'SUN')]
        parent_keep_transform(parts, root)
        root.scale = (factor, factor, factor)
        root.rotation_euler = (0.0, 0.0, math.radians(turn_deg))
        for ob in self.scene.objects:
            if ob.type == 'LIGHT' and ob.data.type != 'SUN':
                ob.data.energy *= factor * factor
                ob.data.shadow_soft_size *= factor
        bpy.context.view_layer.update()
        print("sprite_kit: piece scaled x%g, turned %g deg" % (factor, turn_deg))
