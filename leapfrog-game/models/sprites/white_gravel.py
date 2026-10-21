"""
white_gravel - repeating white gravel for polygon fills, and a matching
rugged border strip for the polygon's edges.

Two textures, rendered one after the other:

  white_gravel         512 x 512, opaque, repeats both ways.
                       Polygon: textureMeterWidth = textureMeterHeight = 25.6
  white_gravel_border  512 x 64, repeats along the edge. The top is the
                       outside: stones stand up to about 1 unit above the
                       edge line with clear gaps between them, so the edge
                       looks rugged; below the line it is solid gravel that
                       fades out into the fill.
                       <groundBorder texture="white_gravel_border"
                           horisontalMeterOffset="1.25"
                           textureMeterWidth="25.6" textureMeterHeight="3.2" />
                       (the script prints the values for other sizes)

Both are neutral grey-white: set the colour on the polygon in the editor
(color / addColor). Borders get the polygon's tint automatically. Fill and
border use the same stones, material, light and levels, so they match.

How the repeat works: the stones are laid out by dart throwing with
distances that wrap around the tile (gravel_layout.py). The tile is built
once as one mesh and copied one period to each side (and up/down for the
fill), so stones crossing the edge, their shadows and the light between
them are the same on both sides of the seam. The frame is rendered a bit
larger and cropped (sprite_kit.render_tile). The images are made neutral
grey and levelled so the brightest stones are near white; the border uses
the fill's level so the two match (saved in sprites/_frames/).

At 20 px/unit (the foreground density) a stone is about 0.6 units = 12 px
across. For a polygon on a parallax layer, render at a lower --ppu (and
--name it differently), e.g. --ppu 12 gives a 42.7-unit tile.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P white_gravel.py -- --samples 16 --name white_gravel_test
    blender -b --factory-startup -P white_gravel.py
Only one of them:
    ... -P white_gravel.py -- --only fill
    ... -P white_gravel.py -- --only border
Options: --stone 0.6 (mean stone size, units), --seed 1, --px 512 (fill
size, power of two), --border-px 64 (border height, power of two),
--rugged 0.8 (height of the humps along the edge), --color (keep the
render's own colour instead of neutral grey), --stone-role / --bed-role
(material roles, default gravel_white / gravel_bed), --no-level (keep the
render's brightness). ore_ground.py runs this script with ore settings.
"""

import json
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                      # noqa: E402
import bpy                        # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

import gravel_layout              # noqa: E402
import sprite_kit as sk           # noqa: E402
import tile_tools                 # noqa: E402


def options(p):
    p.add_argument("--name", default="white_gravel",
                   help="texture name (default white_gravel; the border is <name>_border)")
    p.add_argument("--only", choices=("fill", "border"), default=None, help="render only one of the two")
    p.add_argument("--px", type=int, default=512, help="fill texture size, power of two (default 512)")
    p.add_argument("--border-px", type=int, default=64, help="border height in px, power of two (default 64)")
    p.add_argument("--stone", type=float, default=0.6, help="mean stone size in units (default 0.6)")
    p.add_argument("--seed", type=int, default=1, help="random seed for the layout (default 1)")
    p.add_argument("--rugged", type=float, default=0.8,
                   help="border: height of the humps along the edge, units (default 0.8)")
    p.add_argument("--color", action="store_true", help="keep the render's colour (default: neutral grey)")
    p.add_argument("--stone-role", default="gravel_white", help="material role of the stones (default gravel_white)")
    p.add_argument("--bed-role", default="gravel_bed", help="material role of the bed between them (default gravel_bed)")
    p.add_argument("--no-level", action="store_true",
                   help="keep the render's brightness (default: level so the brightest stones are near white)")


args = sk.parse_args(options)
if args.ppu is None:
    args.ppu = sk.DEFAULT_PPU
PPU = args.ppu
SIZE = args.stone
LEVELS = os.path.join(sk.FRAMES_OUT, args.name + "_levels.json")


# ---------------------------------------------------------------------------
# Stones
# ---------------------------------------------------------------------------

def add_stone(bm, s, rand_layer):
    """One white chip: a squashed sphere with a few flat facets cut off."""
    rng = random.Random(s.seed)
    sub = 2 if s.r > 0.15 * SIZE else 1
    made = bmesh.ops.create_icosphere(bm, subdivisions=sub, radius=1.0)
    verts = made["verts"]

    # Chipped: clip the unit sphere against a few random planes
    cuts = []
    for _ in range(rng.randint(3, 6)):
        n = Vector((rng.gauss(0, 1), rng.gauss(0, 1), rng.gauss(0, 1)))
        if n.length < 1e-6:
            continue
        cuts.append((n.normalized(), rng.uniform(0.55, 0.85)))

    elong = rng.uniform(1.0, 1.5)
    a = s.r                              # along its length (in the picture)
    b = s.r / elong                      # across
    c = s.r * rng.uniform(0.55, 0.85)    # depth, towards the camera
    scale = Matrix.Diagonal(Vector((a, c, b, 1.0)))
    rot = (Matrix.Rotation(rng.uniform(-0.35, 0.35), 4, 'X') @
           Matrix.Rotation(rng.uniform(-0.35, 0.35), 4, 'Z') @
           Matrix.Rotation(rng.uniform(0.0, 2.0 * math.pi), 4, 'Y'))
    # Sunk into the bed (y = 0) by a varying amount; the camera is at -Y
    y = -c * rng.uniform(0.15, 0.6)
    m = Matrix.Translation(Vector((s.x, y, s.z))) @ rot @ scale

    value = rng.random()
    for v in verts:
        p = v.co.copy()
        for n, d in cuts:
            over = p.dot(n) - d
            if over > 0.0:
                p -= n * over
        v.co = m @ p
        v[rand_layer] = value


def build_tile_mesh(name, stones, bed_quads):
    """
    All stones plus the bed behind them in one mesh (material 0 stone,
    1 bed). bed_quads: list of 4 (x, z) corners, counter-clockwise seen from
    the camera.
    """
    bm = bmesh.new()
    rand_layer = bm.verts.layers.float.new("stone_rand")
    for s in stones:
        add_stone(bm, s, rand_layer)
    stone_faces = len(bm.faces)
    for quad in bed_quads:
        vs = [bm.verts.new((x, 0.0, z)) for x, z in quad]
        f = bm.faces.new(vs)
        f.material_index = 1
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    me.polygons.foreach_set("use_smooth", [True] * stone_faces + [False] * (len(me.polygons) - stone_faces))
    me.materials.append(kit.material(args.stone_role))
    me.materials.append(kit.material(args.bed_role))
    return me


def place_copies(name, me, offsets):
    """The same mesh at each (dx, dz): one period around the tile."""
    for i, (dx, dz) in enumerate(offsets):
        ob = bpy.data.objects.new("%s_%d" % (name, i), me)
        bpy.context.scene.collection.objects.link(ob)
        ob.location = (dx, 0.0, dz)


def save_levels(gain):
    os.makedirs(sk.FRAMES_OUT, exist_ok=True)
    with open(LEVELS, "w") as f:
        json.dump({"gain": gain}, f)


def load_levels():
    try:
        with open(LEVELS) as f:
            return float(json.load(f)["gain"])
    except (OSError, ValueError, KeyError):
        return None


def check_pow2(n, what):
    if n <= 0 or n & (n - 1):
        raise SystemExit("white_gravel: %s must be a power of two, got %d" % (what, n))


# ---------------------------------------------------------------------------
# Fill: repeats both ways
# ---------------------------------------------------------------------------

def render_fill():
    check_pow2(args.px, "--px")
    T = args.px / PPU
    stones = gravel_layout.place_stones(T, T, SIZE, seed=args.seed, wrap_x=True, wrap_z=True)
    print("white_gravel: fill %.2f x %.2f units, %d stones" % (T, T, len(stones)))
    me = build_tile_mesh(args.name, stones, [[(0, 0), (T, 0), (T, T), (0, T)]])
    place_copies(args.name, me, [(dx * T, dz * T) for dx in (-1, 0, 1) for dz in (-1, 0, 1)])

    def post(img):
        gain = 1.0 if args.no_level else tile_tools.level_gain(img)
        post.gain = gain
        if not args.name.endswith("_test"):
            save_levels(gain)       # for a later --only border
        img = tile_tools.neutral_grey(img, gain, keep_color=args.color)
        return tile_tools.opaque(img)

    post.gain = None
    kit.render_tile(args.name, (0.0, 0.0), (args.px, args.px), post=post)
    print("white_gravel: polygon textureMeterWidth=\"%g\" textureMeterHeight=\"%g\"" % (T, T))
    return post.gain


# ---------------------------------------------------------------------------
# Border: repeats along the edge
# ---------------------------------------------------------------------------

def render_border(gain=None):
    check_pow2(args.px, "--px")
    check_pow2(args.border_px, "--border-px")
    T = args.px / PPU                    # same period as the fill
    H = args.border_px / PPU             # strip height
    margin = 3.0 / PPU                   # clear rows at the top
    rugged = args.rugged
    # Edge line: low enough that the tallest stones stay below the clear
    # rows at the top, i.e. ups and downs plus half a big stone above it
    top_room = rugged + 0.55 * 0.62 * SIZE + 0.1
    above = min(H * 0.6, top_room + margin)
    z_edge = H - above
    h = gravel_layout.periodic_profile(T, rugged, seed=args.seed + 1, min_wave=1.2 * SIZE, power=1.6)

    def profile(x):
        # Never below the edge line: a dip below it would show the fill's
        # straight edge through the gap
        return z_edge + 0.08 + h(x)

    z_low = -0.8                         # stones continue below the image

    def accept(x, z, r):
        z += z_low
        return z + 0.45 * r <= profile(x)

    stones = gravel_layout.place_stones(T, H - z_low, SIZE, seed=args.seed, wrap_x=True, wrap_z=False,
                                        accept=accept)
    for s in stones:
        s.z += z_low
    print("white_gravel: border %.2f x %.2f units, %d stones, edge %.2f units below the top"
          % (T, H, len(stones), H - z_edge))

    # The bed: solid up to the edge line, and behind the lower part of the
    # stones above it, but not up to their tops so the gaps there stay clear
    segs = 256
    quads = []
    for i in range(segs):
        x0 = T * i / segs
        x1 = T * (i + 1) / segs
        t0 = max(z_edge + 0.05, profile(x0) - 0.35)
        t1 = max(z_edge + 0.05, profile(x1) - 0.35)
        quads.append([(x0, z_low - 0.5), (x1, z_low - 0.5), (x1, t1), (x0, t0)])
    name = args.name + "_border"
    me = build_tile_mesh(name, stones, quads)
    place_copies(name, me, [(dx * T, 0.0) for dx in (-1, 0, 1)])

    if gain is None:
        gain = load_levels()
        if gain is None:
            print("white_gravel: NOTE no fill levels saved yet; levelling the border on its own "
                  "(render the fill first to match them exactly)")

    rows = args.border_px
    edge_row = (H - z_edge) * PPU        # rows from the top

    def post(img):
        g = gain if gain is not None else (1.0 if args.no_level else tile_tools.level_gain(img))
        img = tile_tools.neutral_grey(img, g, keep_color=args.color)
        # Solid down to 0.6 units below the edge line, then fade into the fill
        img = tile_tools.fade_bottom(img, edge_row + 0.6 * PPU, rows - 2)
        return tile_tools.clear_rows(img, top=2, bottom=2)

    kit.render_tile(name, (0.0, 0.0), (args.px, args.border_px), post=post)
    print("white_gravel: <groundBorder texture=\"%s\" horisontalMeterOffset=\"%.3g\" "
          "textureMeterWidth=\"%g\" textureMeterHeight=\"%g\" />" % (name, H - z_edge, T, H))


# ---------------------------------------------------------------------------

fill_gain = None
if args.only != "border":
    kit = sk.Kit(args, rig="tile", res="res_mining.xml")
    fill_gain = render_fill()
if args.only != "fill":
    kit = sk.Kit(args, rig="tile", res="res_mining.xml")
    render_border(fill_gain)
