"""
gen_landing_pad - the generic landing pad for the mining scenes, with a
blinking amber lamp on its side. Static scenery at parallax 0.

The pad itself is not modelled here: it is appended from ../silo_home.blend
(the deck, the truss and legs under it, and the two stone footings), so it
keeps exactly the look of the pad beside the silo. Only the lamp is new: a
mast clamped to one end of the deck with an amber beacon on top. A point
light in the beacon lights the deck's landing surface and the truss around it.

Two images with exactly the same frame, to swap between (or animate):

  gen_landing_pad       the lamp on: amber glass, light on the deck, a soft glow
  gen_landing_pad_off   the lamp off: dark glass, no light

--blink also combines them into gen_landing_pad_blink (rows="2"); give the
sprite box that texture and Animate Period (the script prints a suggestion),
and the engine blinks it.

The deck is turned square to the camera so its front edge is level in the
image, and the camera aims at the standing line (--stand units in from the
front edge), so 1 unit there is 1 game unit, as for the Orb pads. The pad is
scaled so the deck's front edge is --deck-width game units long (default 50,
the size of the pad box at the silo).

In silo_home.blend the deck is "Cube.003", the truss and legs are the
collection "PLtform Balks" and the footings are "Cube.029" and "Cube.030"
(filed under Beam3 there). "Leg1.002" is an exact copy of "Leg1.001" and is
left out.

Night lighting by default, like the home scene (--day for daylight).

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P gen_landing_pad.py -- --samples 16 --name gen_landing_pad_test
    blender -b --factory-startup -P gen_landing_pad.py
    blender -b --factory-startup -P gen_landing_pad.py -- --blink
"""

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                          # noqa: E402
import bpy                            # noqa: E402
from mathutils import Matrix          # noqa: E402
import sprite_kit as sk               # noqa: E402

SILO_BLEND = os.path.join(sk.MODELS_DIR, "silo_home.blend")
PAD_COLLECTION = "PLtform Balks"                 # truss, legs and their struts
PAD_OBJECTS = ["Cube.003", "Cube.029", "Cube.030"]   # deck, two stone footings
DECK = "Cube.003"
DUPLICATE = "Leg1.002"                           # same as Leg1.001, same place
GROUND = 0.0                                     # ground level in silo_home.blend


def options(p):
    p.add_argument("--name", default="gen_landing_pad",
                   help="texture name of the lit image (default gen_landing_pad); the dark one is <name>_off")
    p.add_argument("--deck-width", type=float, default=50.0,
                   help="length of the deck's front edge in game units (default 50, as at the silo)")
    p.add_argument("--side", choices=("left", "right"), default="right", help="which end the lamp is on")
    p.add_argument("--lamp-in", type=float, default=3.0,
                   help="lamp mast this far back from the deck's front edge (default 3)")
    p.add_argument("--mast", type=float, default=5.0, help="lamp height above the deck top (default 5)")
    p.add_argument("--lamp-size", type=float, default=1.6, help="size of the beacon head, 1 = 0.8 units across (default 1.6)")
    p.add_argument("--power", type=float, default=1500.0,
                   help="lamp light in W (default 1500; the moon is 0.7 W/m2)")
    p.add_argument("--color", default="ff7a14", help="lamp colour rrggbb (default ff7a14, amber)")
    p.add_argument("--stand", type=float, default=4.0,
                   help="standing line, units in from the deck's front edge; the camera aims at it (default 4)")
    p.add_argument("--yaw", type=float, default=18.0, help="camera angle from the right, degrees (default 18)")
    p.add_argument("--pitch", type=float, default=6.0, help="camera looking down, degrees (default 6)")
    p.add_argument("--distance", type=float, default=95.0, help="camera distance (default 95, as the ore building)")
    p.add_argument("--blink", action="store_true",
                   help="also make <name>_blink: both images as a 2-frame strip for animatePeriod")
    p.add_argument("--period", type=float, default=1.2,
                   help="with --blink: suggested Animate Period, seconds per on+off (default 1.2)")


args = sk.parse_args(options)
if not args.day:
    args.night = True                 # the home scene is at night
kit = sk.Kit(args, rig="persp", res="res_mining.xml")
M = kit.material
scene = kit.scene

if not os.path.exists(SILO_BLEND):
    raise SystemExit("gen_landing_pad: %s not found" % SILO_BLEND)

c = args.color.lstrip("#")
LAMP_RGB = tuple(int(c[i:i + 2], 16) / 255.0 for i in (0, 2, 4))

YAW = math.radians(args.yaw)
CY, SY = math.cos(YAW), math.sin(YAW)


# --- The pad from silo_home.blend ---------------------------------------------------
# One load, so the deck (requested by name, and the parent of most struts) is
# appended only once.
with bpy.data.libraries.load(SILO_BLEND, link=False) as (src, dst):
    missing = [n for n in PAD_OBJECTS if n not in src.objects]
    if PAD_COLLECTION not in src.collections:
        missing.append("collection " + PAD_COLLECTION)
    if missing:
        raise SystemExit("gen_landing_pad: not in silo_home.blend: " + ", ".join(missing))
    dst.collections = [PAD_COLLECTION]
    dst.objects = list(PAD_OBJECTS)

coll = dst.collections[0]
pad = {ob.name: ob for ob in list(coll.objects) + [o for o in dst.objects if o is not None]}
dup = pad.get(DUPLICATE)
if dup is not None:
    for ob in [dup] + list(dup.children_recursive):
        pad.pop(ob.name, None)
        bpy.data.objects.remove(ob, do_unlink=True)
for ob in pad.values():
    if ob.name not in scene.collection.objects:
        scene.collection.objects.link(ob)
bpy.data.collections.remove(coll)        # the objects stay, linked to the scene
bpy.context.view_layer.update()

# Where the deck is: a unit cube scaled and turned about z. Seen from the silo
# camera, its local -x face is the front and local -y points right in the image.
deck = pad[DECK]
mw = deck.matrix_world.copy()
centre = mw.translation.copy()
ax, ay, az = mw.col[0].xyz, mw.col[1].xyz, mw.col[2].xyz
deck_angle = math.atan2(ax.y, ax.x)
S = args.deck_width / (2.0 * ay.length)

# Deck centre to the origin, ground to z = 0, local -x towards the camera,
# scaled to game units, then turned with the camera so the front edge is level.
T = (Matrix.Rotation(YAW + math.pi / 2 - deck_angle, 4, 'Z') @ Matrix.Scale(S, 4)
     @ Matrix.Translation((-centre.x, -centre.y, -GROUND)))
for ob in pad.values():
    if ob.parent is None or ob.parent.name not in pad:
        ob.matrix_world = T @ ob.matrix_world
bpy.context.view_layer.update()

DW = 2.0 * ay.length * S                  # front edge, = --deck-width
DD = 2.0 * ax.length * S                  # depth
DECK_TOP = (centre.z + az.length - GROUND) * S
DECK_THICK = 2.0 * az.length * S
DFRONT = -DD / 2.0
STAND = DFRONT + args.stand
print("gen_landing_pad: silo pad scaled x%.3f: deck %.1f x %.1f, top at %.2f, %.2f thick"
      % (S, DW, DD, DECK_TOP, DECK_THICK))


def dw(x, y, z):
    """Pad coordinates (x along the front edge, y back from the camera) to world."""
    return (x * CY - y * SY, x * SY + y * CY, z)


# --- The lamp -------------------------------------------------------------------------------
SIDE = 1.0 if args.side == "right" else -1.0
LS = args.lamp_size
END = SIDE * DW / 2.0                       # the deck's end face
LX = END + SIDE * 0.9 * LS                  # mast just outside the deck
LY = DFRONT + args.lamp_in
MAST_BOT = DECK_TOP - DECK_THICK - 1.2
MAST_TOP = DECK_TOP + args.mast
R_MAST = 0.16 * LS

# Mast, clamps and bands are built in pad coordinates and turned with the pad
parts = bmesh.new()
sk.strut(parts, (LX, LY, MAST_BOT), (LX, LY, MAST_TOP), R_MAST, 10)
for z in (DECK_TOP - 0.3 * DECK_THICK, DECK_TOP - 0.8 * DECK_THICK):        # clamp arms to the deck end
    sk.box(parts, ((END + LX) / 2.0, LY, z), (abs(LX - END) + 0.3, 0.35 * LS, 0.3 * LS))
for dy in (-0.5, 0.5):                                                       # clamp plates on the deck end
    sk.box(parts, (END + SIDE * 0.06, LY + dy * LS, DECK_TOP - 0.55 * DECK_THICK),
           (0.12, 0.25 * LS, 0.9 * DECK_THICK))
sk.strut(parts, (LX, LY, DECK_TOP + 1.2), (END - SIDE * 0.3, LY, DECK_TOP + 0.05),
         0.07 * LS, 6)                                                       # brace onto the deck edge
mast_ob = sk.mesh_object("lamp_mast", parts, M("frame_main"), smooth_quads=True)
mast_ob.rotation_euler = (0.0, 0.0, YAW)

bands = bmesh.new()                                                          # yellow bands up the mast
for k in range(3):
    n0 = len(bands.verts)
    sk.lathe(bands, [(R_MAST * 1.1, 0.0), (R_MAST * 1.1, 0.45)], 16)
    z = DECK_TOP + 0.6 + k * (args.mast - 1.6) / 3.0
    for v in list(bands.verts)[n0:]:
        v.co.x += LX
        v.co.y += LY
        v.co.z += z
bands_ob = sk.mesh_object("lamp_mast_bands", bands, M("hazard_paint"), smooth_quads=True)
bands_ob.rotation_euler = (0.0, 0.0, YAW)

HEAD_Z = MAST_TOP                       # bottom of the beacon head


def lathe_at(name, profile, material, z0, cap_top=False, cap_bottom=False):
    bm = bmesh.new()
    sk.lathe(bm, [(r * LS, h * LS) for r, h in profile], 24, cap_top=cap_top, cap_bottom=cap_bottom)
    ob = sk.mesh_object(name, bm, material, smooth_quads=True)
    ob.location = dw(LX, LY, z0)
    ob.rotation_euler = (0.0, 0.0, YAW)
    ob.visible_shadow = False           # the light sits inside the head
    return ob


base = lathe_at("lamp_base", [(0.55, 0.0), (0.55, 0.28), (0.42, 0.34)], M("frame_main"), HEAD_Z,
                cap_top=True, cap_bottom=True)
GLASS_PROFILE = [(0.40, 0.0), (0.40, 0.55), (0.34, 0.78), (0.2, 0.9), (0.0, 0.94)]


def glass_off_material():
    mat = bpy.data.materials.new("lamp_glass_off")
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    dim = tuple(0.18 * ch for ch in LAMP_RGB)
    bsdf.inputs["Base Color"].default_value = (dim[0], dim[1], dim[2], 1.0)
    bsdf.inputs["Roughness"].default_value = 0.22
    return mat


def glass_on_material():
    mat = bpy.data.materials.new("lamp_glass_on")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission")
    em.inputs["Color"].default_value = (LAMP_RGB[0], LAMP_RGB[1], LAMP_RGB[2], 1.0)
    em.inputs["Strength"].default_value = 16.0
    nt.links.new(em.outputs["Emission"], out.inputs["Surface"])
    return mat


GLASS_ON = glass_on_material()
GLASS_OFF = glass_off_material()
glass = lathe_at("lamp_glass", GLASS_PROFILE, GLASS_ON, HEAD_Z + 0.34 * LS, cap_bottom=True)
cage = bmesh.new()                                                          # wire guard over the glass
for k in range(4):
    a = k * math.pi / 2 + math.pi / 4
    p0 = (0.44 * math.cos(a) * LS, 0.44 * math.sin(a) * LS)
    sk.strut(cage, (p0[0], p0[1], 0.0), (p0[0], p0[1], 0.6 * LS), 0.03 * LS, 5)
    sk.strut(cage, (p0[0], p0[1], 0.6 * LS), (0.0, 0.0, 1.0 * LS), 0.03 * LS, 5)
cage_ob = sk.mesh_object("lamp_cage", cage, M("frame_main"))
cage_ob.location = dw(LX, LY, HEAD_Z + 0.34 * LS)
cage_ob.rotation_euler = (0.0, 0.0, YAW)
cage_ob.visible_shadow = False

LIGHT_POS = dw(LX, LY, HEAD_Z + 0.34 * LS + 0.45 * LS)
lamp = sk.point_light("lamp_light", LIGHT_POS, LAMP_RGB, args.power, 0.3 * LS)

# Room above the head for the glow, so it isn't cut off at the image edge
room = sk.box_object("glow_room", dw(LX, LY, HEAD_Z + 1.28 * LS + 0.6), (0.2, 0.2, 0.2), None)
room.hide_render = True
room["kit_frame_extent"] = True

# The kit's glow round the brightest pixels (it does this for ambient pieces);
# no haze for scenery at parallax 0.
kit.haze = 0.0
kit.glow = args.glow if args.glow is not None else 1.0
kit._post_axo = kit.glow > 0.0

# --- Render ----------------------------------------------------------------------------------------
on_path = kit.render_persp(args.name, target=dw(0.0, STAND, DECK_TOP), yaw_deg=args.yaw,
                           pitch_deg=args.pitch, distance=args.distance, ground_z=0.0)

kit.report_points("gen_landing_pad: where the bodies go", [
    ("deck front edge, left end", dw(-DW / 2, DFRONT, DECK_TOP)),
    ("deck front edge, right end", dw(DW / 2, DFRONT, DECK_TOP)),
    ("standing line, left end", dw(-DW / 2, STAND, DECK_TOP)),
    ("standing line, right end", dw(DW / 2, STAND, DECK_TOP)),
    ("deck underside, front middle", dw(0.0, DFRONT, DECK_TOP - DECK_THICK)),
    ("lamp", LIGHT_POS),
])

glass.data.materials[0] = GLASS_OFF
lamp.hide_render = True
off_name = args.name + "_off"
off_path = kit.render_variant(off_name)

if on_path and off_path:
    # The dark image goes in the same folder as the lit one, wherever that is listed
    rel = os.path.relpath(os.path.dirname(on_path), sk.DEFAULT_OUT).replace("\\", "/")
    if not rel.startswith(".."):
        kit.folder = rel
    ppu = kit._last[3]
    kit.register(off_name + ".png", ppu=ppu, frames=(1, 1))
    if args.blink:
        kit.make_strip([on_path, off_path], args.name + "_blink")
        print("sprite_kit: %s_blink: Animate Period %.2f s (on and off %.2f s each)"
              % (args.name, args.period, args.period / 2.0))
