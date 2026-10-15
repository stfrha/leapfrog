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
* A new texture must also be listed in `data/res.xml` for the game to load it.
* `kit.render_variant(name)` renders the same scene again (e.g. a light switched off) for
  animation frames; pixels that only differ by render noise are copied from the first image.

## Sprites

| script | texture | box (units) | notes |
|---|---|---|---|
| `ls_tower.py` | `ls_tower` | 9 x 106.7 | rendered at 19 px/unit (too tall for 2048 at 20) |
| `ls_booster_tank.py` | `ls_booster_tank` | 13.2 x 53 | hex plating twice the ship's hex size (`--tile 7.8`; ship is ~3.9) |
| `ore_building.py` | `ore_building` | ~50 x 42 (perspective) | mission giver's building with the pub "The Ore"; pad states in `PADS` at the top; also renders `ore_building_beer_off` for a blinking neon |
