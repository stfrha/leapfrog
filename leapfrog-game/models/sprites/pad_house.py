"""
pad_house - a building at the Orb with a landing deck on its roof, out of
use for now and kept for later in the story. Static scenery at parallax 0.

A squat two-storey house with a flat roof. On the roof, a steel landing deck
stands on a frame, with an external stair tower on the right leading up to
it. Two looks, rendered as two images so the story can switch between them
(e.g. per mission state in the scene):

  pad_house       (default) closed: roller shutters down on the ground floor,
                  dark and broken windows upstairs, barricades and a chain
                  across the deck, its edge lamps dead, the stair door boarded
                  up, a dead sign
  pad_house_open  (--open) in use: shutters up and the windows lit, the deck
                  clear with its edge lamps lit, the stair door lit, and a lit
                  sign (--sign, default OPEN)

The deck is laid out square to the camera (the house itself is not), so its
front edge is level in the image and a plain box body matches it; the camera
aims at the deck's standing line, so 1 unit there is 1 game unit and the
Leapfrog's feet (21.4 apart) land on the foot marks.

Camera: the same as the ore building (yaw 18, pitch 6, distance 95). Night
lighting by default (--day for daylight).

Collision bodies: draw them in the editor. After the render the script prints
where the deck's front edge, standing line and foot marks land in the image
(game units from the image centre, y down). Suggested: the deck as a static
box with behaviour landingPad (active="false" while closed), the house below
it as a static box.

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P pad_house.py -- --samples 16 --name pad_house_test
    blender -b --factory-startup -P pad_house.py
    blender -b --factory-startup -P pad_house.py -- --open
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                          # noqa: E402
import sprite_kit as sk               # noqa: E402


def options(p):
    p.add_argument("--name", default=None, help="texture name (default pad_house, or pad_house_open with --open)")
    p.add_argument("--open", action="store_true", help="the house in use: lit, deck clear")
    p.add_argument("--sign", default="OPEN", help="text of the lit sign with --open (default OPEN)")
    p.add_argument("--yaw", type=float, default=18.0, help="camera angle from the right, degrees (default 18)")
    p.add_argument("--pitch", type=float, default=6.0, help="camera looking down, degrees (default 6)")
    p.add_argument("--distance", type=float, default=95.0, help="camera distance (default 95, as the ore building)")
    p.add_argument("--deck-width", type=float, default=30.0,
                   help="length of the deck along the image (the Leapfrog's feet span 21.4; default 30)")
    p.add_argument("--seed", type=int, default=11, help="randomness for broken windows and clutter (default 11)")
    p.add_argument("--font", default=None, help="TTF/OTF for the sign (default: Blender's font)")
    p.add_argument("--ground-depth", type=float, default=1.8,
                   help="thickness of the ground under the house, 0 for none (default 1.8)")


args = sk.parse_args(options)
if not args.day:
    args.night = True             # the home scene is at night
OPEN = args.open
if args.name is None:
    args.name = "pad_house_open" if OPEN else "pad_house"
kit = sk.Kit(args, rig="persp", res="res_mining.xml")
M = kit.material
rnd = random.Random(args.seed)

YAW = math.radians(args.yaw)
CY, SY = math.cos(YAW), math.sin(YAW)


# Both looks are always built; the other look's parts are hidden but still
# count for the framing (kit_frame_extent), so pad_house and pad_house_open
# get exactly the same frame and can replace each other in the scene.
_rec = None


def rec(ob):
    if _rec is not None:
        _rec.append(ob)
    return ob


def box(name, center, size, role, rot=(0.0, 0.0, 0.0), bev=0.0):
    return rec(sk.box_object(name, center, size, M(role), rotation=rot, bevel_width=bev))


def mesh(name, bm, role, smooth=False):
    return rec(sk.mesh_object(name, bm, M(role), smooth_quads=smooth))


def light(name, loc, color, power, radius):
    return rec(sk.point_light(name, loc, color, power, radius))


def look(is_open, build):
    global _rec
    _rec = []
    build(is_open)
    parts, _rec = _rec, None
    if is_open != OPEN:
        for ob in parts:
            ob.hide_render = True
            if getattr(ob, "type", "MESH") != 'LIGHT':
                ob["kit_frame_extent"] = True


# --- The house (world axes, like the ore building) ----------------------------------
HW, HD = 26.0, 14.0
FRONT = -HD / 2
PLINTH = 0.6
FH = 9.5
ROOF = PLINTH + 2 * FH            # 19.6
UZ = PLINTH + FH + 5.0
box("house", (0, 0, ROOF / 2), (HW, HD, ROOF), "concrete_dirty", bev=0.12)
box("house_plinth", (0, 0, PLINTH / 2), (HW + 0.8, HD + 0.8, PLINTH), "concrete_dirty")
box("house_slab", (0, 0, PLINTH + FH), (HW + 0.5, HD + 0.5, 0.5), "concrete_dirty")
box("house_parapet", (0, 0, ROOF + 0.4), (HW + 0.5, HD + 0.5, 0.8), "concrete_dirty")
GF = ((-7.0, 9.0, 6.0), (5.5, 9.0, 6.0))           # ground floor openings: centre x, width, height
for k, (cx, w, h) in enumerate(GF):
    box("gf_frame_%d" % k, (cx, FRONT - 0.06, PLINTH + h / 2 + 0.2), (w + 0.6, 0.25, h + 0.4), "rusty_metal")
    box("shutter_box_%d" % k, (cx, FRONT - 0.35, PLINTH + h + 0.8), (w + 0.4, 0.7, 0.8), "rusty_metal")
for k in range(4):
    box("uf_frame_%d" % k, (-9.0 + k * 6.0, FRONT - 0.06, UZ), (3.6, 0.25, 3.6), "rusty_metal")
box("sign_board", (-1.0, FRONT - 0.3, PLINTH + FH - 1.4), (10.0, 0.3, 1.8), "frame_main")
UF_KIND = [rnd.choice(["glass_dark", "glass_dark", "void"]) for _ in range(4)]

# Stair tower on the right, up to the deck
TX0 = HW / 2 + 0.2
TWW = 6.0
DECK_Z = ROOF + 3.4               # top of the deck
box("stair_tower", (TX0 + TWW / 2, 1.0, (DECK_Z + 3.0) / 2), (TWW, 7.0, DECK_Z + 3.0), "frame_brace", bev=0.08)
box("stair_roof", (TX0 + TWW / 2, 1.0, DECK_Z + 3.25), (TWW + 0.8, 7.8, 0.5), "rusty_metal")
box("stair_door_frame", (TX0 + TWW / 2, 1.0 - 3.56, PLINTH + 2.7), (2.8, 0.25, 5.4), "frame_main")


# --- Deck on the roof, square to the camera ------------------------------------------------
DECK_X, DECK_Y = -1.6, 0.0          # ends at the stair tower, overhangs the house on the left


def dw(x, y, z):
    return (DECK_X + x * CY - y * SY, DECK_Y + x * SY + y * CY, z)


def dbox(name, center, size, role, bev=0.0, extra=0.0):
    return box(name, dw(*center), size, role, rot=(0.0, 0.0, YAW + extra), bev=bev)


DW = args.deck_width
DD = 12.0
DFRONT = -DD / 2
STAND = DFRONT + 3.5
FEET = 21.4 / 2
dbox("deck", (0, 0, DECK_Z - 0.3), (DW, DD, 0.6), "pad_metal", bev=0.05)
dbox("deck_edge", (0, DFRONT + 0.1, DECK_Z - 0.75), (DW, 0.3, 0.6), "frame_main")
frame = bmesh.new()
for x in (-DW / 2 + 3.0, -4.0, 4.0, DW / 2 - 3.0):        # all standing on the roof
    for y in (DFRONT + 1.0, -DFRONT - 1.0):
        sk.box(frame, dw(x, y, (ROOF + 0.8 + DECK_Z - 0.6) / 2), (0.5, 0.5, DECK_Z - 0.6 - ROOF - 0.8))
for y in (DFRONT + 1.0, -DFRONT - 1.0):
    sk.strut(frame, dw(-DW / 2 + 1.0, y, DECK_Z - 0.8), dw(DW / 2 - 1.0, y, DECK_Z - 0.8), 0.22, 6)
mesh("deck_frame", frame, "frame_main")
for k in range(0, 16, 2):
    x0 = -DW / 2 + DW * k / 16
    dbox("deck_stripe_%d" % k, (x0 + DW / 32, DFRONT + 0.5, DECK_Z + 0.02), (DW / 16 - 0.05, 0.8, 0.06), "hazard_paint")
for sx in (-1, 1):
    for dx, dy, w, d in ((0, -1.0, 3.2, 0.25), (0, 1.0, 3.2, 0.25), (-1.5, 0, 0.25, 2.2), (1.5, 0, 0.25, 2.2)):
        dbox("foot_mark_%d_%g_%g" % (sx, dx, dy), (sx * FEET + dx, STAND + dy, DECK_Z + 0.03), (w, d, 0.05),
             "hazard_paint")


def build_closed(is_open):
    """Shutters down, windows dark and boarded, deck barred, lamps and sign dead."""
    for k, (cx, w, h) in enumerate(GF):
        sl = bmesh.new()
        z = PLINTH + h + 0.2
        while z > PLINTH + 0.3:
            sk.box(sl, (cx, FRONT - 0.12, z - 0.22), (w, 0.15, 0.4))
            z -= 0.45
        mesh("shutter_%d" % k, sl, "rusty_metal")
    for k in range(4):
        cx = -9.0 + k * 6.0
        box("uf_pane_dark_%d" % k, (cx, FRONT - 0.08, UZ), (3.0, 0.25, 3.0), UF_KIND[k])
        if k in (1, 3):
            for a in (0.5, -0.4):
                box("uf_board_%d_%g" % (k, a), (cx, FRONT - 0.25, UZ), (3.8, 0.15, 0.5), "rusty_metal",
                    rot=(0.0, a, 0.0))
    br = bmesh.new()
    for k in range(6):
        sk.box(br, (-4.6 + k * 1.45, FRONT - 0.5, PLINTH + FH - 1.4), (0.15, 0.2, 0.7))
    mesh("sign_brackets", br, "neon_dead")
    box("stair_door_shut", (TX0 + TWW / 2, 1.0 - 3.6, PLINTH + 2.5), (2.2, 0.25, 5.0), "frame_main")
    for a in (0.6, -0.6):
        box("stair_board_%g" % a, (TX0 + TWW / 2, 1.0 - 3.8, PLINTH + 2.5), (2.8, 0.15, 0.45), "rusty_metal",
            rot=(0.0, a, 0.0))
    for k in range(5):
        box("stair_slit_dark_%d" % k, (TX0 + TWW / 2, 1.0 - 3.56, PLINTH + 7.5 + k * 3.4), (1.0, 0.25, 1.6),
            "glass_dark")
    for k in range(7):
        x = -DW / 2 + 1.5 + (DW - 3.0) * k / 6
        dbox("edge_lamp_dead_%d" % k, (x, DFRONT + 0.2, DECK_Z + 0.2), (0.5, 0.4, 0.35), "neon_dead")
    rail_ = bmesh.new()
    deck_rail(rail_)
    mesh("deck_rail_rusty", rail_, "rusty_metal")
    # Two barricades and a sagging chain across the deck, a painted cross
    for sx in (-1, 1):
        cx = sx * 6.0
        b = bmesh.new()
        for dx in (-1.6, 1.6):
            for dy in (-0.5, 0.5):
                sk.strut(b, dw(cx + dx, STAND + 1.5 + dy * 1.6, DECK_Z), dw(cx + dx, STAND + 1.5, DECK_Z + 2.4), 0.08, 6)
        mesh("barricade_legs_%d" % sx, b, "frame_main")
        for k, z in enumerate((1.4, 2.2)):
            dbox("barricade_board_%d_%d" % (sx, k), (cx, STAND + 1.45, DECK_Z + z), (4.0, 0.15, 0.55), "hazard_paint")
    chain = bmesh.new()
    xs = [-DW / 2 + 1.2, -DW / 2 + 9.0, DW / 2 - 9.0, DW / 2 - 1.2]
    for x in xs:
        sk.strut(chain, dw(x, DFRONT + 1.2, DECK_Z), dw(x, DFRONT + 1.2, DECK_Z + 2.0), 0.1, 6)
    for x0, x1 in zip(xs, xs[1:]):
        n = 8
        prev = None
        for k in range(n + 1):
            t = k / n
            p = dw(x0 + (x1 - x0) * t, DFRONT + 1.2, DECK_Z + 1.8 - 0.9 * math.sin(math.pi * t))
            if prev is not None:
                sk.strut(chain, prev, p, 0.05, 5)
            prev = p
    mesh("deck_chain", chain, "frame_main")
    for a in (0.6, -0.6):
        dbox("deck_cross_%g" % a, (0.0, STAND + 1.0, DECK_Z + 0.04), (7.0, 0.6, 0.04), "container_red", extra=a)


def build_open(is_open):
    """Shutters up and the house lit, deck clear with its lamps lit, a lit sign."""
    for k, (cx, w, h) in enumerate(GF):
        box("gf_window_%d" % k, (cx, FRONT - 0.08, PLINTH + h / 2 + 0.2), (w, 0.25, h), "window_lit_warm")
        light("gf_light_%d" % k, (cx, FRONT - 2.5, PLINTH + 3.0), (1.0, 0.65, 0.35), 220.0, 1.5)
    for k in range(4):
        box("uf_pane_lit_%d" % k, (-9.0 + k * 6.0, FRONT - 0.08, UZ), (3.0, 0.25, 3.0),
            "window_lit_warm" if k != 2 else "glass_dark")
    light("uf_light", (0.0, FRONT - 3.0, UZ), (1.0, 0.7, 0.4), 260.0, 2.0)
    if args.sign:
        rec(sk.text_object("sign", args.sign, 1.3, [M("neon_amber")], location=(-1.0, FRONT - 0.5, PLINTH + FH - 1.45),
                           neon=True, tube=0.07, font_path=args.font))
        light("sign_light", (-1.0, FRONT - 2.0, PLINTH + FH - 1.4), (1.0, 0.45, 0.1), 120.0, 0.8)
    box("stair_door_lit", (TX0 + TWW / 2, 1.0 - 3.6, PLINTH + 2.5), (2.2, 0.25, 5.0), "window_lit_warm")
    light("stair_light", (TX0 + TWW / 2, 1.0 - 5.0, PLINTH + 3.0), (1.0, 0.65, 0.35), 120.0, 0.8)
    for k in range(5):
        box("stair_slit_lit_%d" % k, (TX0 + TWW / 2, 1.0 - 3.56, PLINTH + 7.5 + k * 3.4), (1.0, 0.25, 1.6),
            "window_lit_warm")
    for k in range(7):
        x = -DW / 2 + 1.5 + (DW - 3.0) * k / 6
        dbox("edge_lamp_lit_%d" % k, (x, DFRONT + 0.2, DECK_Z + 0.2), (0.5, 0.4, 0.35), "neon_amber")
    light("deck_glow", dw(0.0, DFRONT - 1.0, DECK_Z + 1.0), (1.0, 0.5, 0.15), 200.0, 3.0)
    rail_ = bmesh.new()
    deck_rail(rail_)
    mesh("deck_rail_painted", rail_, "hazard_paint")


def deck_rail(bm):
    for k in range(7):
        x = -DW / 2 + 1.0 + (DW - 2.0) * k / 6
        sk.strut(bm, dw(x, -DFRONT - 0.4, DECK_Z), dw(x, -DFRONT - 0.4, DECK_Z + 2.0), 0.06, 6)
    sk.strut(bm, dw(-DW / 2 + 1.0, -DFRONT - 0.4, DECK_Z + 2.0), dw(DW / 2 - 1.0, -DFRONT - 0.4, DECK_Z + 2.0), 0.07, 6)


look(False, build_closed)
look(True, build_open)

# --- Clutter and ground ---------------------------------------------------------------------------
for k, (x, y) in enumerate(((-15.5, FRONT - 2.0), (-13.8, FRONT - 2.6))):
    bm = bmesh.new()
    sk.lathe(bm, [(0.75, 0.0), (0.75, 2.2)], 18, cap_top=True, cap_bottom=True)
    ob = sk.mesh_object("barrel_%d" % k, bm, M("rusty_metal" if k else "container_blue"), smooth_quads=True)
    ob.location = (x, y, 0.0)
box("crates", (12.0, FRONT - 2.2, 1.0), (3.0, 2.0, 2.0), "rusty_metal", rot=(0.0, 0.0, 0.3))

if args.ground_depth > 0:
    def gw(xp, yp, z):
        return (xp * CY - yp * SY, xp * SY + yp * CY, z)
    bm = bmesh.new()
    nx, ny = 44, 10
    x0, x1, y0, y1 = -24.0, 27.0, -13.0, 12.0
    top = []
    for j in range(ny + 1):
        row = []
        for i in range(nx + 1):
            xp = x0 + (x1 - x0) * i / nx
            yp = y0 + (y1 - y0) * j / ny
            z = rnd.uniform(-0.05, 0.05)
            if j == 0:
                yp += rnd.uniform(-0.4, 0.3)
                z = rnd.uniform(-0.15, 0.0)
            row.append(bm.verts.new(gw(xp, yp, z)))
        top.append(row)
    for j in range(ny):
        for i in range(nx):
            bm.faces.new((top[j][i], top[j][i + 1], top[j + 1][i + 1], top[j + 1][i]))
    front = top[0]
    bottom = [bm.verts.new((v.co.x, v.co.y, -args.ground_depth)) for v in front]
    for i in range(nx):
        bm.faces.new((front[i], front[i + 1], bottom[i + 1], bottom[i]))
    sk.mesh_object("ground", bm, M("ground_dirt"))

# --- Render --------------------------------------------------------------------------------------
kit.render_persp(args.name, target=dw(0.0, STAND, DECK_Z), yaw_deg=args.yaw, pitch_deg=args.pitch,
                 distance=args.distance, ground_z=-args.ground_depth - 0.01)

kit.report_points("pad house: where the bodies go", [
    ("deck front edge, left end", dw(-DW / 2, DFRONT, DECK_Z)),
    ("deck front edge, right end", dw(DW / 2, DFRONT, DECK_Z)),
    ("standing line, left end", dw(-DW / 2, STAND, DECK_Z)),
    ("standing line, right end", dw(DW / 2, STAND, DECK_Z)),
    ("left foot mark", dw(-FEET, STAND, DECK_Z)),
    ("right foot mark", dw(FEET, STAND, DECK_Z)),
    ("house, front left at the ground", (-HW / 2, FRONT, 0.0)),
    ("house, front right at the roof", (HW / 2, FRONT, ROOF)),
])
