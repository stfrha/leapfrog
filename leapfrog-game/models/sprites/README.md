# Sprite scripts

Python scripts that model and render Leapfrog sprites in Blender (4.3).
Each `<texture>.py` builds one part and writes `<texture>.png` straight into
`leapfrog-game/data/images/`.

## Run

From this folder:

```
"C:\Program Files\Blender Foundation\Blender 4.3\blender.exe" -b --factory-startup -P ls_tower.py
```

Useful options (after `--`):

| option | |
|---|---|
| `--samples 16` | fast preview render (default 128) |
| `--name ls_tower_test` | write to a different texture name instead of overwriting |
| `--ppu 10` | pixels per game unit (default 20, or the script's own default) |
| `--gpu` | render on the GPU |
| `--save-blend` | also save the generated scene to `_blend/<name>.blend` for inspection |
| `--no-render` | build the scene only (with `--save-blend`) |
| `--theme mining` | world theme (palette + lighting), see `themes/` |
| `--hdri <path>` | HDRI to use instead of `../leapfrog-hdr/kloetzle_blei_4k.exr` (next to the repo, see the main README) |

Script-specific options are listed with `-- --help`.

## Conventions

* 1 Blender unit = 1 game unit.
* 20 px per unit by default, the same as `GlobalConstants.TexturePixelsPerUnit` in the
  editor. Parts longer than 2048 px are rendered at a lower px/unit. The px/unit an image
  was rendered at is written to its resource entry (`ppu="10"`) and updated on re-renders,
  and "Set Width and Height from texture pixel size" in the editor divides by it (and uses
  one frame of an animation strip). oxygine ignores the attribute.
* The image frame is the part's box, not its visible pixels, so the sprite lines
  up with the body's width x height.
* Mechanical parts: orthographic front view (camera looks along +Y, Z up), `kit.render_ortho()`.
* Static scenery (buildings): perspective 3/4 view like the silo, `kit.render_persp()`. The frame
  is fitted around the geometry at 20 px/unit at the object's depth.
* Buildings you land on (the Orb): `workshop.py` (two storeys, the open top floor is the landing
  area between two side walls, tools inside; seen face-on, yaw 0, distance 60), `fuel_station.py`
  (three lying tanks stacked 2 + 1, a pad with dispenser and kiosk, a half-broken FUEL neon) and
  `pad_house.py` (a house with a landing deck on its roof, closed by default, `--open` renders the
  in-use look as `pad_house_open` with exactly the same frame). They render at night by default
  (`--day`). Landing surfaces are laid out square to the camera so their front edge is level in
  the image, and the camera aims at the pad's *standing line* (where the feet come down), so
  1 unit there is 1 game unit and the Leapfrog (feet 21.4 apart) stands on the painted foot marks.
  After the render each script prints where the pad's edges, standing line and foot marks land
  (`kit.report_points`): offsets from the image centre in game units, y down, ready for the
  collision bodies in the editor.
* `gen_landing_pad.py`: the generic landing pad for the mining scenes. The pad is appended from
  `../silo_home.blend` (deck `Cube.003`, collection `PLtform Balks`, footings `Cube.029`/`Cube.030`),
  turned square to the camera and scaled so the deck's front edge is `--deck-width` (default 50);
  only the amber beacon on a mast at one end (`--side`) is new. Two images with the same frame:
  `gen_landing_pad` (lamp on, lighting the deck) and `gen_landing_pad_off`; `--blink` also makes
  `gen_landing_pad_blink` (2 frames, for Animate Period). Night by default (`--day`).
* `kit.image_point(world)` / `kit.report_points(title, [(label, world), ...])`: where world points
  land in the last render_persp/render_axo image. `kit.render_crop(name, objects, frames,
  set_frame)`: an animated overlay cropped to some objects (e.g. a flickering neon sign) with the
  same camera, as a strip `<name>.png`; it prints where to place it on the main sprite. Use it when
  the whole image as a strip would be over 2048 px. `ob["kit_frame_extent"] = True` on a hidden
  object makes render_persp (like render_axo) still frame it, so variants get the same frame.
  Theme role `neon_cyan` (the fuel sign).
* Ambient pieces (small background buildings for parallax layers, named `amb_*`, e.g.
  `amb_hut.py`): `sk.Kit(args, rig="axo", res="res_mining.xml", ambient=True, ppu=14)` and
  `kit.render_axo()`, an orthographic 3/4 view with the same yaw/pitch for every piece, so
  pieces can stand side by side anywhere on a layer. The image bottom is the front of the
  footprint on the ground. They go to `data/images/<world>/ambient/` and share one atlas
  there (oxygine's default extend, i.e. 2 px padding). For a layer with
  `parallaxAmount` a, about 20 × (1 − a) px/unit is enough (0.6 → 8–10, 0.4 → 12–14).
  `scale=` / `--scale` builds the piece bigger than its natural size (the village on the 0.4
  layer uses ×2), and `turn=` / `--turn` turns it about its vertical axis so pieces don't stand
  neatly lined up; the camera stays the same for all pieces. Shared building blocks (windows,
  doors, ribbed and corrugated walls, masts, railings) are in `amb_parts.py`. Village pieces:
  `amb_hut`, `amb_bunkhouse`, `amb_container_home`, `amb_dome`, `amb_water_tower`.
  Mine site pieces (same layer, camera and scale; machines face right and are dead by
  default, `--lit` / `--dark` switch their few lights): `amb_excavator` (tracked excavator;
  `--lift`, `--curl` pose the arm), `amb_haul_truck` (dump truck; `--tip`, `--empty`,
  `--flat`), `amb_crusher` (crusher tower, conveyor and ore stockpile), `amb_headframe`
  (shaft headframe with the winding house; its sheave wheels turn: 4 frames by default,
  `--still` for one image; set Animate Period to what the script prints, about 0.56 s).
  `amb_mine_shaft` is an open shaft (an oval hole with a concrete collar and a fence, rock
  walls into the dark) with a cantilever hoist on a tower beside it lifting a kibble of ore
  out (`--bucket-z`); two strings of lamps run down the far wall one lamp at a time
  (`--all-blink` to blink them together), 6 frames, `--still`/`--dark`. A piece with a hole
  in the ground marks its opening with `ob["kit_catcher_hole"] = True`: the shadow catcher
  gets that hole, and a holdout ring just under the ground hides what is below ground
  outside it.
  `--rusty` makes the excavator or truck a wreck. Their parts (beams, rams, tracks,
  tyres, cabs, heaps, sheaves, trusses, trestles) are in `amb_mine_parts.py`; material
  roles `machine_yellow`, `rubber` and `ore_rock` are in the mining theme.
  Ambient pieces render at night by default: the theme's dark sky and a moon high up
  (`--moon` strength, `--day` for daylight), standing on a shadow catcher so their shadow
  falls on the ground behind them (`--no-shadow`; the frame grows to hold the shadow), with
  haze towards the night sky baked in (`--haze`, default 0.18) and a glow round lit
  windows and lamps (`--glow`, default 1). Signs of life use
  `kit.render_axo_animated(name, frames, set_frame, period)`: the hut, dome and water
  tower lights blink (`--blink 110000`, different phases per piece), the container home
  has stovepipe smoke (`amb_parts.smoke_column`), the headframe's wheels turn. Default 6
  frames per 2.4 s loop; `--still` for one image. Strips may be grids (cols and rows);
  re-rendering keeps the resource entry's cols/rows right.
  Ground for the mine site: `ore_ground.py` renders `ore_ground` (polygon fill) and
  `ore_ground_border` by running `white_gravel.py` with ore stones (`ore_stone` on
  `ore_bed`), their own colour and brightness (no tint needed), 14 px/unit and few big
  stones (1.2 units, 256 px tile = 18.3 units). It prints the polygon and border values.
* Animations (e.g. a blinking neon): render each frame with `frame_only=True` (to `_frames/`,
  not shipped or registered), then `kit.make_strip()` combines them into one image with
  `rows`/`cols` in its resource entry. The editor shows an animation's first frame, so bodies
  are fitted to the animation texture itself; `animatePeriod` (seconds per cycle) on the shape
  makes the game play it.
* Lighting and materials come from the theme. Scripts ask for material roles
  (`frame_main`, `frame_brace`, ...) and the theme decides what they look like.
* Every file in `data/images` ships with the game (Android packs all of `data/`, iOS has
  `data/images` as a folder reference), and the engine loads what `data/res.xml` lists.
  The kit adds new textures to `res.xml` itself (own atlas, `trim="false" extend="false"`,
  plus `rows`/`cols` for animations). The atlas is a plain `<atlas>` (clamp to edge); only
  textures that tile use `clamp2edge="false"`, and those must be power-of-two or they render
  black on iOS. It skips `*_test` names, custom `--out` and `--no-register`,
  and never changes entries that already exist, except their `ppu`.
* Big scenery goes in a per-world resource set instead of `res.xml`, e.g. `res_mining.xml`
  (`kit = sk.Kit(args, res="res_mining.xml")` or `--res`). The game loads a set only while a
  scene that uses it runs; `data/scene_resources.xml` says which scenes use which set.
  New sets are created and added to the iOS/macOS Xcode projects automatically
  (`project_files.py`, which can also be run on its own:
  `python project_files.py add-res res_desert.xml --scenes desert_scene.xml`).
* Images are organised in subfolders matching the resource set: `res_mining.xml` ->
  `data/images/mining/`, `res.xml` -> `data/images/common/` (or `--folder`). Texture names
  are file names without folder, so they must be unique across folders. Each entry the
  kit writes starts with its own `<set path>`, so its position in the file doesn't matter.
  A texture that is already registered is re-rendered to wherever its entry points.
  Move existing textures (file + entry, optionally to another resource file) with
  `python project_files.py move-texture images/mining ls_tower ls_booster_tank --res res_mining.xml`;
  (a moved texture keeps its wrap mode); `python project_files.py where` lists every texture's
  resource file and folder, and `python project_files.py check` lists repeat-mode textures
  that aren't power-of-two.
* Editor-only images (e.g. a single frame for fitting bodies when the game uses an animation)
  go to `leapfrog-game/editor_images/<folder>/`, which no build project includes. The editor reads
  textures from both folders.
* `kit.render_variant(name)` renders the same scene again (e.g. a light switched off) for
  animation frames; pixels that only differ by render noise are copied from the first image.
  `sk.make_strip()` combines frames into one image and prints the `res.xml` line (`cols`/`rows`).

## Sprites

| script | texture | box (units) | notes |
|---|---|---|---|
| `ls_tower.py` | `ls_tower` | 9 x 106.7 | rendered at 19 px/unit (too tall for 2048 at 20) |
| `ls_booster_tank.py` | `ls_booster_tank` | 13.2 x 53 | hex plating twice the ship's hex size (`--tile 7.8`; ship is ~3.9) |
| `ore_building.py` | `ore_building` | ~50 x 42 (perspective) | mission giver's building with the pub "The Ore"; pad states in `PADS` at the top; game uses `ore_building_blink` (2 frames, Beer neon on/off); `ore_building` is editor-only; registered in `res_mining.xml` |
| `docking_station.py` | `dock_station_base`, `dock_arm`, `dock_clamp` | 22 x 32, 27.2 x 2.4, 3.2 x 5.4 | space docking station (`data/docking_station.xml`); each part rendered alone, framed to its body box; in `res.xml`, `data/images/space` (placeholders until rendered); `--only base|arm|clamp`, `--suffix _test` |
