"""
workshop - the workshop at the Orb, where the Leapfrog gets repaired and
upgraded. Static scenery at parallax 0, with landing on its top floor.

Two storeys. The ground floor is a garage and store: a roller door left half
open with light spilling out under it, a side door, a window, and a WORKSHOP
neon on the fascia between the floors. The top floor is the workshop itself:
it has no roof and no front wall, so we look straight in and the Leapfrog
lands on its floor between the two side walls. The inside of the walls has
the tools: a red tool cabinet, a pegboard and a welding cart on the left;
a shelf rack and a spare thruster hanging from a jib hoist on the right; a
workbench, a tool board and a lit doorway on the back wall. Two work lamps
on the wall tops light the floor. Two painted foot marks on the floor show
where the Leapfrog's feet go (they are 21.4 apart).

Sizes (game units, 1 unit as in the scene): the opening between the side
walls is 35 wide (--inside), so there is about 6.8 on each side of the
Leapfrog's feet. The floor is 16 deep. The side walls are 12 high above the
floor (--wall), the top floor's floor is at 11.4 above the ground.

Camera: unlike the other scenery (seen from the right), the workshop is seen
face-on (yaw 0) and from closer (--distance 60), so both inner walls and
their tools show. It is its own sprite, so the different angle doesn't jar.
The camera aims at the standing line, 4 units in from the floor's front
edge, and the image's px/unit holds at that depth: the Leapfrog, drawn flat
at game scale, stands on the foot marks exactly.

Lighting: night by default, like the rest of the home scene (--day for the
daylight rig the ore building was rendered with).

Collision bodies: draw them in the editor. After the render the script
prints where the floor's front edge, a standing line 4 units in from the
front, and the wall tops land in the image, as offsets from the image centre
in game units (x right, y down). The sprite box's position is the image
centre, so those offsets go straight into the bodies' positions.
Suggested bodies:
  - the floor: a static box (behaviour landingPad) with its top on the
    standing line, from the left wall's inner face to the right wall's
  - each side wall: a static box from the floor up to the wall top
  - the ground floor: a static box under the floor, down to the ground

Run from leapfrog-game\\models\\sprites:
    blender -b --factory-startup -P workshop.py -- --samples 16 --name workshop_test
    blender -b --factory-startup -P workshop.py
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bmesh                          # noqa: E402
import sprite_kit as sk               # noqa: E402


def options(p):
    p.add_argument("--name", default="workshop", help="texture name (default workshop)")
    p.add_argument("--inside", type=float, default=35.0,
                   help="opening between the side walls (the Leapfrog's feet span 21.4; default 35)")
    p.add_argument("--wall", type=float, default=12.0, help="height of the side walls above the floor (default 12)")
    p.add_argument("--depth", type=float, default=16.0, help="depth of the building (default 16)")
    p.add_argument("--yaw", type=float, default=0.0, help="camera angle from the right, degrees (default 0)")
    p.add_argument("--pitch", type=float, default=8.0, help="camera looking down, degrees (default 8)")
    p.add_argument("--distance", type=float, default=60.0,
                   help="camera distance; less shows more of the inner walls (default 60)")
    p.add_argument("--lamp-power", type=float, default=1100.0, help="watts of each work lamp (default 1100)")
    p.add_argument("--no-marks", action="store_true", help="no painted foot marks on the floor")
    p.add_argument("--door-open", type=float, default=0.45,
                   help="how far the roller door is open, 0..1 (default 0.45)")
    p.add_argument("--seed", type=int, default=3, help="randomness for clutter (default 3)")
    p.add_argument("--font", default=None, help="TTF/OTF for the neon (default: Blender's font)")
    p.add_argument("--ground-depth", type=float, default=1.8,
                   help="thickness of the ground under the building, 0 for none (default 1.8)")


args = sk.parse_args(options)
if not args.day:
    args.night = True             # the home scene is at night
kit = sk.Kit(args, rig="persp", res="res_mining.xml")
M = kit.material
rnd = random.Random(args.seed)

# --- Dimensions -------------------------------------------------------------
IN = args.inside              # opening between the side walls
WT = 1.4                      # wall thickness
OW = IN + 2 * WT              # outer width
D = args.depth
FRONT = -D / 2
BACK = D / 2
PLINTH = 0.5
SLAB = 0.8
FZ = 11.4                     # top of the workshop floor
WH = args.wall                # side wall height above the floor
TOP = FZ + WH
XI = IN / 2                   # inner faces of the side walls at +-XI
XO = OW / 2
YB = BACK - WT                # inner face of the back wall
FEET = 21.4 / 2               # the Leapfrog's feet at +-FEET


def box(name, center, size, role, rot=(0.0, 0.0, 0.0), bev=0.0):
    return sk.box_object(name, center, size, M(role), rotation=rot, bevel_width=bev)


def cylinder(name, role, base, radius, height, segments=24, cap=True):
    bm = bmesh.new()
    sk.lathe(bm, [(radius, 0.0), (radius, height)], segments, cap_top=cap, cap_bottom=cap)
    ob = sk.mesh_object(name, bm, M(role), smooth_quads=True)
    ob.location = base
    return ob


# --- Ground floor -----------------------------------------------------------
box("plinth", (0, 0, PLINTH / 2), (OW + 1.0, D + 1.0, PLINTH), "concrete_dirty", bev=0.08)
GF_TOP = FZ - SLAB
DX0, DX1 = -13.0, 1.5         # roller door opening
DOOR_TOP = 7.6
RECESS = 3.0                  # depth of the garage behind the door that we can see into
FW = 0.8                      # front wall thickness
# The ground floor's body stands back from the front by RECESS; the front
# wall is built around the door opening, so the lit garage shows through it
box("ground_floor", (0, (FRONT + RECESS + BACK) / 2, (PLINTH + GF_TOP) / 2), (OW, BACK - FRONT - RECESS,
    GF_TOP - PLINTH), "concrete_dirty", bev=0.1)
for name, x0, x1, z0, z1 in (("left", -XO, DX0, PLINTH, GF_TOP), ("right", DX1, XO, PLINTH, GF_TOP),
                             ("lintel", DX0, DX1, DOOR_TOP, GF_TOP)):
    box("front_wall_" + name, ((x0 + x1) / 2, FRONT + 0.2 + FW / 2, (z0 + z1) / 2), (x1 - x0, FW, z1 - z0),
        "concrete_dirty")
# The garage behind the opening: side walls, a ceiling, and a lit back
# (what light falls on inside), with shelves as dark shapes against it
for x in (DX0, DX1):
    box("garage_side_%g" % x, (x, FRONT + (RECESS + FW) / 2, (PLINTH + DOOR_TOP) / 2), (0.3, RECESS, DOOR_TOP - PLINTH),
        "concrete_dirty")
box("garage_ceiling", ((DX0 + DX1) / 2, FRONT + (RECESS + FW) / 2, DOOR_TOP + 0.1), (DX1 - DX0, RECESS, 0.2),
    "concrete_dirty")
# Corner columns
for sx in (-1, 1):
    box("gf_column_%d" % sx, (sx * (XO - 0.6), FRONT + 0.2, (PLINTH + GF_TOP) / 2), (1.3, 1.0, GF_TOP - PLINTH),
        "frame_main")

# Fascia between the floors with the WORKSHOP neon
FASCIA_Z0 = 8.4
box("fascia", (0, FRONT - 0.1, (FASCIA_Z0 + FZ) / 2), (OW + 0.4, 1.2, FZ - FASCIA_Z0), "frame_main", bev=0.05)
box("fascia_board", (0, FRONT - 0.75, (FASCIA_Z0 + FZ) / 2 - 0.1), (17.0, 0.2, 2.1), "rusty_metal")
sk.text_object("neon_workshop", "WORKSHOP", 1.5, [M("neon_amber")],
               location=(0, FRONT - 0.95, (FASCIA_Z0 + FZ) / 2 - 0.15), neon=True, tube=0.07, font_path=args.font)
sk.point_light("neon_workshop_light", (0, FRONT - 3.0, (FASCIA_Z0 + FZ) / 2), (1.0, 0.45, 0.1), 160.0, 0.8)

# Roller door, partly open, with the lit garage behind
door_w = DX1 - DX0
opening = (DOOR_TOP - PLINTH) * max(0.0, min(1.0, args.door_open))
box("garage_inside", ((DX0 + DX1) / 2, FRONT + RECESS - 0.05, (PLINTH + DOOR_TOP) / 2), (door_w, 0.1, DOOR_TOP - PLINTH),
    "window_lit_warm")
box("garage_floor", ((DX0 + DX1) / 2, FRONT + RECESS / 2, PLINTH + 0.02), (door_w, RECESS, 0.04), "concrete_dirty")
# Silhouettes inside: a shelf with boxes and a drum
inside = bmesh.new()
gy = FRONT + RECESS - 0.4
for x in (-11.8, -7.6):
    sk.box(inside, (x, gy, PLINTH + 2.2), (0.15, 0.4, 4.4))
for z in (1.2, 3.0):
    sk.box(inside, (-9.7, gy, PLINTH + z), (4.4, 0.6, 0.12))
sk.box(inside, (-10.6, gy, PLINTH + 1.7), (1.4, 0.5, 1.0))
sk.box(inside, (-8.8, gy, PLINTH + 3.5), (1.0, 0.5, 0.9))
sk.mesh_object("garage_shelf", inside, M("frame_main"))
cylinder("garage_drum", "rusty_metal", (-3.0, FRONT + RECESS - 0.9, PLINTH), 0.7, 2.0, 16)
for sx, x in ((-1, DX0), (1, DX1)):
    box("door_guide_%d" % sx, (x - sx * 0.25, FRONT - 0.15, (PLINTH + DOOR_TOP) / 2), (0.5, 0.5, DOOR_TOP - PLINTH),
        "frame_main")
# Slats from the top of the door down to the opening
slats = bmesh.new()
z = DOOR_TOP
while z > PLINTH + opening + 0.05:
    h = min(0.42, z - (PLINTH + opening))
    sk.box(slats, ((DX0 + DX1) / 2, FRONT + 0.75, z - h / 2), (door_w, 0.18, h - 0.05))
    z -= 0.45
sk.mesh_object("roller_door", slats, M("pad_metal"))
box("roller_door_bar", ((DX0 + DX1) / 2, FRONT + 0.65, PLINTH + opening + 0.12), (door_w, 0.3, 0.25),
    "hazard_paint")
box("roller_box", ((DX0 + DX1) / 2, FRONT - 0.35, DOOR_TOP + 0.45), (door_w + 0.6, 0.9, 0.9), "rusty_metal")
sk.point_light("garage_light", ((DX0 + DX1) / 2, FRONT + RECESS - 1.0, PLINTH + 2.0), (1.0, 0.6, 0.3), 420.0, 2.0)
sk.point_light("garage_spill", ((DX0 + DX1) / 2, FRONT - 1.6, PLINTH + 0.4), (1.0, 0.6, 0.3), 160.0, 2.0)

# Side door and window on the right
box("side_door_frame", (6.2, FRONT + 0.14, PLINTH + 2.8), (3.0, 0.25, 5.8), "rusty_metal")
box("side_door", (6.2, FRONT + 0.1, PLINTH + 2.65), (2.4, 0.25, 5.3), "frame_brace")
box("gf_window_frame", (12.4, FRONT + 0.14, PLINTH + 4.3), (4.8, 0.25, 3.4), "rusty_metal")
box("gf_window", (12.4, FRONT + 0.11, PLINTH + 4.3), (4.2, 0.25, 2.8), "glass_dark")
box("gf_window_sill", (12.4, FRONT - 0.1, PLINTH + 2.45), (5.2, 0.6, 0.25), "concrete_dirty")
# A lamp over the side door
box("door_lamp", (6.2, FRONT - 0.25, PLINTH + 6.4), (0.9, 0.6, 0.4), "window_lit_warm")
sk.point_light("door_lamp_light", (6.2, FRONT - 1.2, PLINTH + 6.0), (1.0, 0.7, 0.4), 90.0, 0.4)

# --- Workshop floor -----------------------------------------------------------
box("floor_slab", (0, 0, FZ - SLAB / 2), (OW + 0.2, D, SLAB), "concrete_dirty", bev=0.05)
# Steel deck plates with seams
box("deck", (0, -0.4, FZ + 0.03), (IN, D - WT - 0.8, 0.06), "pad_metal")
seams = bmesh.new()
for k in range(1, 7):
    x = -XI + IN * k / 7
    sk.box(seams, (x, -0.4, FZ + 0.065), (0.07, D - WT - 0.8, 0.02))
sk.box(seams, (0, FRONT + 5.4, FZ + 0.065), (IN, 0.07, 0.02))
sk.mesh_object("deck_seams", seams, M("frame_main"))
# Hazard stripe along the front edge
stripes = bmesh.new()
n = 22
for k in range(n):
    x0 = -XI + IN * k / n
    if k % 2 == 0:
        sk.box(stripes, (x0 + IN / n / 2, FRONT + 0.45, FZ + 0.08), (IN / n - 0.05, 0.8, 0.04))
sk.mesh_object("edge_stripes", stripes, M("hazard_paint"))
box("edge_angle", (0, FRONT + 0.05, FZ - 0.05), (IN, 0.2, 0.3), "frame_main")
# Painted foot marks where the Leapfrog stands
if not args.no_marks:
    marks = bmesh.new()
    for sx in (-1, 1):
        cx = sx * FEET
        cy = FRONT + 4.0
        for dx, dy, w, d in ((0, -1.0, 3.2, 0.25), (0, 1.0, 3.2, 0.25), (-1.5, 0, 0.25, 2.2), (1.5, 0, 0.25, 2.2)):
            sk.box(marks, (cx + dx, cy + dy, FZ + 0.075), (w, d, 0.02))
    sk.mesh_object("foot_marks", marks, M("hazard_paint"))
    # Oil stains and scuffs
    for k in range(4):
        r = rnd.uniform(0.5, 1.0)
        bm = bmesh.new()
        sk.lathe(bm, [(r, 0.0)], 20, cap_top=True)
        st = sk.mesh_object("stain_%d" % k, bm, M("frame_main"))
        st.location = (rnd.uniform(-XI + 4, XI - 4), rnd.uniform(FRONT + 2, YB - 2), FZ + 0.07)
        st.scale = (1.0, rnd.uniform(0.5, 0.9), 1.0)

# --- Walls of the workshop ------------------------------------------------------
for sx in (-1, 1):
    xc = sx * (XI + WT / 2)
    box("side_wall_%d" % sx, (xc, 0, FZ + WH / 2), (WT, D, WH), "concrete_dirty", bev=0.08)
    box("side_cap_%d" % sx, (xc, 0, TOP + 0.15), (WT + 0.3, D + 0.2, 0.3), "frame_main")
    box("side_column_%d" % sx, (xc, FRONT + 0.4, FZ + WH / 2), (WT + 0.3, 0.9, WH + 0.1), "frame_main")
    # Hazard band on the front end of the wall (where the Leapfrog's legs pass)
    for k in range(4):
        z = FZ + 1.0 + k * 1.2
        box("wall_band_%d_%d" % (sx, k), (xc, FRONT - 0.07, z), (WT + 0.32, 0.1, 0.55), "hazard_paint",
            rot=(0.0, sx * 0.35, 0.0))
box("back_wall", (0, BACK - WT / 2, FZ + WH / 2), (IN, WT, WH), "concrete_dirty", bev=0.06)
box("back_cap", (0, BACK - WT / 2, TOP + 0.15), (OW + 0.3, WT + 0.3, 0.3), "frame_main")

# --- Back wall: workbench, tool board, doorway ------------------------------------------
BENCH_Z = FZ + 3.0
box("bench_top", (-1.5, YB - 0.9, BENCH_Z), (12.0, 1.8, 0.3), "rusty_metal")
legs = bmesh.new()
for x in (-7.2, 4.2):
    for y in (YB - 1.6, YB - 0.2):
        sk.box(legs, (x, y, (FZ + BENCH_Z) / 2), (0.3, 0.3, BENCH_Z - FZ))
sk.box(legs, (-1.5, YB - 0.9, FZ + 0.8), (11.4, 1.4, 0.12))          # lower shelf
sk.mesh_object("bench_legs", legs, M("frame_main"))
box("vice", (2.6, YB - 1.5, BENCH_Z + 0.45), (0.9, 0.7, 0.7), "machine_yellow")
box("bench_toolbox", (-5.0, YB - 0.9, BENCH_Z + 0.5), (2.2, 1.0, 0.8), "container_red")
cylinder("bench_can", "hazard_paint", (-2.6, YB - 0.8, BENCH_Z + 0.15), 0.35, 0.9, 16)
# Tool board with tool shapes
box("tool_board", (-1.5, YB - 0.08, FZ + 6.2), (11.0, 0.15, 4.0), "pad_metal")
tools = bmesh.new()
for k in range(11):
    x = -6.5 + k * 1.0
    kind = k % 4
    if kind == 0:                                                       # spanner
        sk.box(tools, (x, YB - 0.2, FZ + 6.4), (0.18, 0.08, 2.0))
        sk.box(tools, (x, YB - 0.2, FZ + 7.5), (0.5, 0.08, 0.3))
    elif kind == 1:                                                     # hammer
        sk.box(tools, (x, YB - 0.2, FZ + 6.0), (0.15, 0.08, 2.2))
        sk.box(tools, (x, YB - 0.2, FZ + 7.2), (0.8, 0.12, 0.35))
    elif kind == 2:                                                     # screwdrivers
        for d in (-0.2, 0.2):
            sk.box(tools, (x + d, YB - 0.2, FZ + 6.6), (0.1, 0.08, 1.4))
    else:                                                               # saw
        sk.box(tools, (x, YB - 0.2, FZ + 5.8), (0.7, 0.06, 1.6))
sk.mesh_object("tools_back", tools, M("frame_main"))
box("bench_lamp", (-1.5, YB - 0.5, FZ + 8.9), (2.4, 0.8, 0.35), "window_lit_warm")
sk.point_light("bench_light", (-1.5, YB - 1.6, FZ + 8.0), (1.0, 0.85, 0.65), 260.0, 0.6)
# Doorway to the office and the stairs, lit
box("office_door_frame", (-14.2, YB - 0.05, FZ + 2.9), (3.4, 0.2, 5.8), "frame_main")
box("office_doorway", (-14.2, YB - 0.1, FZ + 2.75), (2.8, 0.2, 5.5), "window_lit_warm")
sk.point_light("office_spill", (-14.2, YB - 1.5, FZ + 2.4), (1.0, 0.7, 0.45), 160.0, 0.8)
# Sign on the back wall
box("sign_board", (8.8, YB - 0.1, FZ + 7.4), (6.0, 0.15, 1.6), "hazard_paint")
sk.text_object("sign_text", "NO SMOKING", 0.75, [M("void")], location=(8.8, YB - 0.2, FZ + 7.35),
               neon=False, extrude=0.03, font_path=args.font)

# --- Left wall: tool cabinet, pegboard, welding cart ----------------------------------------
LX = -XI
box("cabinet", (LX + 0.95, 4.5, FZ + 3.3), (1.9, 3.2, 6.6), "container_red", bev=0.05)
drawers = bmesh.new()
for k in range(7):
    sk.box(drawers, (LX + 1.92, 4.5, FZ + 0.7 + k * 0.85), (0.04, 2.9, 0.06))
    sk.box(drawers, (LX + 1.95, 4.5, FZ + 1.05 + k * 0.85), (0.08, 0.8, 0.12))      # handles
sk.mesh_object("cabinet_drawers", drawers, M("frame_main"))
box("pegboard", (LX + 0.06, -1.8, FZ + 5.6), (0.12, 5.6, 4.2), "pad_metal")
peg = bmesh.new()
for k in range(9):
    y = -4.2 + k * 0.6
    if k % 3 == 0:
        sk.box(peg, (LX + 0.18, y, FZ + 5.4), (0.08, 0.16, 2.0))
        sk.box(peg, (LX + 0.18, y, FZ + 6.5), (0.1, 0.45, 0.3))
    elif k % 3 == 1:
        sk.box(peg, (LX + 0.18, y, FZ + 5.8), (0.08, 0.14, 1.5))
    else:
        sk.box(peg, (LX + 0.2, y, FZ + 4.6), (0.12, 0.5, 0.5))
sk.box(peg, (LX + 0.6, -1.8, FZ + 7.3), (1.0, 0.25, 0.25))                           # hose reel hub
sk.mesh_object("pegboard_tools", peg, M("frame_main"))
reel = bmesh.new()
sk.strut(reel, (LX + 0.15, -1.8, FZ + 7.3), (LX + 0.9, -1.8, FZ + 7.3), 0.9, 20)
sk.mesh_object("hose_reel", reel, M("machine_yellow"), smooth_quads=True)
# Welding cart with two gas bottles
box("cart", (LX + 1.8, -5.6, FZ + 1.0), (2.2, 1.6, 0.25), "frame_main")
cart = bmesh.new()
for x in (LX + 0.8, LX + 2.8):
    sk.strut(cart, (x, -5.6, FZ + 0.4), (x, -5.6, FZ + 1.0), 0.4, 12)                # wheels (seen end on)
sk.mesh_object("cart_wheels", cart, M("rubber"), smooth_quads=True)
cylinder("bottle_a", "container_blue", (LX + 1.3, -5.6, FZ + 1.1), 0.45, 3.6)
cylinder("bottle_b", "container_red", (LX + 2.3, -5.4, FZ + 1.1), 0.45, 3.2)
box("welder", (LX + 1.6, -6.5, FZ + 1.6), (1.6, 0.8, 1.0), "machine_yellow")

# --- Right wall: shelf rack, jib hoist with a spare thruster -------------------------------------
RX = XI
rack = bmesh.new()
for y in (0.0, 5.0):
    for x in (RX - 1.7, RX - 0.1):
        sk.box(rack, (x, y, FZ + 3.6), (0.15, 0.15, 7.2))
for z in (0.4, 2.6, 4.8, 7.0):
    sk.box(rack, (RX - 0.9, 2.5, FZ + z), (1.8, 5.2, 0.12))
sk.mesh_object("rack", rack, M("frame_main"))
for k, (y, z, w, h, role) in enumerate(((0.9, 0.5, 1.4, 1.2, "rusty_metal"), (2.8, 0.5, 1.9, 1.6, "container_blue"),
                                        (4.2, 0.5, 0.9, 0.9, "rusty_metal"), (1.2, 2.7, 1.8, 1.4, "pad_metal"),
                                        (3.4, 2.7, 1.2, 1.0, "container_red"), (0.8, 4.9, 1.2, 1.1, "container_blue"),
                                        (2.6, 4.9, 1.6, 1.5, "rusty_metal"), (4.3, 4.9, 0.8, 1.0, "pad_metal"))):
    box("crate_%d" % k, (RX - 0.9, y, FZ + z + h / 2), (1.5, w, h), role)
# Jib hoist on top of the right wall, swung back against the back wall
JY = YB - 2.2
box("jib_post", (RX + WT / 2, JY, TOP + 1.4), (0.5, 0.5, 2.8), "frame_main")
box("jib_arm", (RX - 1.6, JY, TOP + 2.6), (4.8 + WT, 0.4, 0.45), "machine_yellow")
chain = bmesh.new()
sk.strut(chain, (RX - 3.4, JY, TOP + 2.4), (RX - 3.4, JY, FZ + 6.6), 0.06, 6)
sk.mesh_object("hoist_chain", chain, M("frame_main"))
box("hoist_block", (RX - 3.4, JY, TOP + 1.9), (0.6, 0.5, 0.8), "hazard_paint")
# The spare thruster bell hanging from it
bell = bmesh.new()
sk.lathe(bell, [(1.4, 0.0), (1.05, 1.0), (0.6, 2.2), (0.45, 2.8)], 28, cap_top=True)
ob = sk.mesh_object("spare_thruster", bell, M("nozzle"), smooth_quads=True)
ob.location = (RX - 3.4, JY, FZ + 3.8)

# Air compressor by the right wall at the front, and a fuse box with cables
comp = bmesh.new()
sk.strut(comp, (RX - 1.1, -6.6, FZ + 1.2), (RX - 1.1, -3.4, FZ + 1.2), 0.85, 20)       # tank, lying along the wall
sk.mesh_object("compressor_tank", comp, M("machine_yellow"), smooth_quads=True)
box("compressor_motor", (RX - 1.1, -5.0, FZ + 2.45), (1.2, 1.6, 0.9), "frame_main")
wheels = bmesh.new()
for y in (-6.2, -3.8):
    sk.strut(wheels, (RX - 1.95, y, FZ + 0.35), (RX - 0.25, y, FZ + 0.35), 0.35, 12)
sk.mesh_object("compressor_wheels", wheels, M("rubber"), smooth_quads=True)
hose = bmesh.new()
pts = [(RX - 1.1, -4.4, FZ + 2.9), (RX - 0.3, -4.0, FZ + 4.2), (RX - 0.15, -5.0, FZ + 5.6), (RX - 0.15, -6.2, FZ + 5.0),
       (RX - 0.15, -6.6, FZ + 3.9)]
for p0, p1 in zip(pts, pts[1:]):
    sk.strut(hose, p0, p1, 0.09, 6)
sk.box(hose, (RX - 0.2, -5.0, FZ + 5.7), (0.3, 0.3, 0.3))                                # hook
sk.mesh_object("air_hose", hose, M("container_red"), smooth_quads=True)
box("fuse_box", (RX - 0.25, -2.0, FZ + 7.6), (0.5, 1.6, 2.0), "pad_metal")
box("fuse_box_door_mark", (RX - 0.52, -2.0, FZ + 8.0), (0.04, 0.5, 0.5), "hazard_paint")
cables = bmesh.new()
for k, y in enumerate((-2.5, -2.0, -1.5)):
    sk.strut(cables, (RX - 0.3, y, FZ + 6.6), (RX - 0.3, y - 0.2 * k, FZ + 0.2), 0.06, 6)
sk.strut(cables, (RX - 0.3, -2.0, FZ + 8.6), (RX - 0.3, -2.0, TOP), 0.08, 6)
sk.mesh_object("fuse_cables", cables, M("rubber"))

# --- Work lamps on the wall tops ------------------------------------------------------
for sx in (-1, 1):
    x = sx * (XI + WT / 2)
    box("lamp_post_%d" % sx, (x, -1.0, TOP + 1.0), (0.3, 0.3, 2.0), "frame_main")
    box("lamp_arm_%d" % sx, (x - sx * 1.1, -1.0, TOP + 2.0), (2.6, 0.25, 0.25), "frame_main")
    box("lamp_head_%d" % sx, (x - sx * 2.3, -1.0, TOP + 1.7), (1.2, 0.9, 0.5), "frame_main",
        rot=(0.0, sx * 0.5, 0.0))
    box("lamp_glass_%d" % sx, (x - sx * 2.35, -1.0, TOP + 1.42), (0.95, 0.7, 0.06), "window_lit_warm",
        rot=(0.0, sx * 0.5, 0.0))
    sk.point_light("work_lamp_%d" % sx, (x - sx * 3.5, -1.0, TOP - 0.5), (1.0, 0.82, 0.6), args.lamp_power, 0.8)

# --- Clutter on the ground in front -------------------------------------------------------
for k, (x, y) in enumerate(((4.0, FRONT - 1.8), (5.4, FRONT - 2.4), (17.4, FRONT - 1.6))):
    cylinder("barrel_%d" % k, "rusty_metal" if k != 1 else "container_blue", (x, y, 0.0), 0.75, 2.2, 18)
tyre = bmesh.new()
sk.strut(tyre, (-16.4, FRONT - 1.8, 0.95), (-16.4, FRONT - 2.8, 0.95), 0.95, 22)
sk.mesh_object("tyre", tyre, M("rubber"), smooth_quads=True)
box("pallet", (-18.6, FRONT - 2.0, 0.2), (2.6, 2.2, 0.4), "rusty_metal")
box("pallet_load", (-18.6, FRONT - 2.0, 1.1), (2.2, 1.8, 1.4), "frame_brace")

# --- Ground -----------------------------------------------------------------------------------
if args.ground_depth > 0:
    bm = bmesh.new()
    nx, ny = 40, 8
    x0, x1, y0, y1 = -XO - 5.0, XO + 5.0, FRONT - 5.0, BACK + 3.0
    top = []
    for j in range(ny + 1):
        row = []
        for i in range(nx + 1):
            x = x0 + (x1 - x0) * i / nx
            y = y0 + (y1 - y0) * j / ny
            z = rnd.uniform(-0.05, 0.05)
            if j == 0:
                y += rnd.uniform(-0.4, 0.3)
                z = rnd.uniform(-0.15, 0.0)
            row.append(bm.verts.new((x, y, z)))
        top.append(row)
    for j in range(ny):
        for i in range(nx):
            bm.faces.new((top[j][i], top[j][i + 1], top[j + 1][i + 1], top[j + 1][i]))
    front = top[0]
    bottom = [bm.verts.new((v.co.x, v.co.y, -args.ground_depth)) for v in front]
    for i in range(nx):
        bm.faces.new((front[i], front[i + 1], bottom[i + 1], bottom[i]))
    sk.mesh_object("ground", bm, M("ground_dirt"))

# --- Render --------------------------------------------------------------------------------
# The camera aims at the standing line (4 units in from the floor's front
# edge), so the frame's px/unit holds there: the Leapfrog, drawn flat at game
# scale, then fits the floor and the foot marks exactly.
STAND_Y = FRONT + 4.0
kit.render_persp(args.name, target=(0.0, STAND_Y, FZ), yaw_deg=args.yaw, pitch_deg=args.pitch,
                 distance=args.distance, ground_z=-args.ground_depth - 0.01)

kit.report_points("workshop: where the bodies go", [
    ("floor front edge, left wall", (-XI, FRONT, FZ)),
    ("floor front edge, right wall", (XI, FRONT, FZ)),
    ("standing line (4 in), left wall", (-XI, STAND_Y, FZ)),
    ("standing line (4 in), right wall", (XI, STAND_Y, FZ)),
    ("left foot mark", (-FEET, STAND_Y, FZ)),
    ("right foot mark", (FEET, STAND_Y, FZ)),
    ("left wall top, inner front", (-XI, FRONT, TOP)),
    ("left wall top, outer front", (-XO, FRONT, TOP)),
    ("right wall top, inner front", (XI, FRONT, TOP)),
    ("right wall top, outer front", (XO, FRONT, TOP)),
    ("ground, front left corner", (-XO, FRONT, 0.0)),
    ("ground, front right corner", (XO, FRONT, 0.0)),
])
