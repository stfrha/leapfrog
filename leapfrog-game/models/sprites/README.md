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
| `--gpu` | render on the GPU |
| `--save-blend` | also save the generated scene to `_blend/<name>.blend` for inspection |
| `--no-render` | build the scene only (with `--save-blend`) |
| `--theme mining` | world theme (palette + lighting), see `themes/` |
| `--hdri <path>` | HDRI to use instead of `models/HDRI/kloetzle_blei_16k.hdr` |

Script-specific options are listed with `-- --help`.

## Conventions

* 1 Blender unit = 1 game unit.
* 20 px per unit, the same as `GlobalConstants.TexturePixelsPerUnit` in the
  editor, so "Set Width and Height from texture pixel size" gives the right size.
  Parts longer than 2048 px are rendered at a lower whole px/unit (with a warning);
  enter their size by hand.
* The image frame is the part's box, not its visible pixels, so the sprite lines
  up with the body's width x height.
* Mechanical parts: orthographic front view (camera looks along +Y, Z up), `kit.render_ortho()`.
* Static scenery (buildings): perspective 3/4 view like the silo, `kit.render_persp()`. The frame
  is fitted around the geometry at 20 px/unit at the object's depth.
* Lighting and materials come from the theme. Scripts ask for material roles
  (`frame_main`, `frame_brace`, ...) and the theme decides what they look like.
* Every file in `data/images` ships with the game (Android packs all of `data/`, iOS has
  `data/images` as a folder reference), and the engine loads what `data/res.xml` lists.
  The kit adds new textures to `res.xml` itself (own atlas, `trim="false" extend="false"`,
  plus `rows`/`cols` for animations). The atlas is a plain `<atlas>` (clamp to edge); only
  textures that tile use `clamp2edge="false"`, and those must be power-of-two or they render
  black on iOS. It skips `*_test` names, custom `--out` and `--no-register`,
  and never changes entries that already exist.
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
