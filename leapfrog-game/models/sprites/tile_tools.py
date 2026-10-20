"""
tile_tools - image post-processing for repeating textures (numpy only, no
Blender needed, so it can be checked with plain Python).

All images here are float arrays of shape (rows, cols, 4), RGBA in 0..1,
straight (not premultiplied) alpha, with row 0 at the TOP of the image.
(Blender's pixel buffers run bottom-up; sprite_kit flips them.)
"""

import numpy as np

LUMA = np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)


def crop(img, margin_x, margin_y, width, height):
    """The width x height block after margin_x columns and margin_y rows."""
    return img[margin_y:margin_y + height, margin_x:margin_x + width, :].copy()


def luminance(img):
    return (img[:, :, :3] * LUMA).sum(axis=2)


def level_gain(img, target=0.97, percentile=99.5, min_alpha=0.99):
    """
    Gain that puts the bright end of the image (the given percentile of the
    luminance of its opaque pixels) at `target`.
    """
    lum = luminance(img)
    mask = img[:, :, 3] >= min_alpha
    values = lum[mask] if mask.any() else lum.ravel()
    top = float(np.percentile(values, percentile))
    return target / top if top > 1e-4 else 1.0


def neutral_grey(img, gain=1.0, keep_color=False):
    """
    Scale by gain and (unless keep_color) make every pixel neutral grey, so
    a shape's color attribute in the editor decides the tint alone.
    """
    out = img.copy()
    if keep_color:
        out[:, :, :3] = np.clip(out[:, :, :3] * gain, 0.0, 1.0)
    else:
        lum = np.clip(luminance(img) * gain, 0.0, 1.0)
        out[:, :, 0] = lum
        out[:, :, 1] = lum
        out[:, :, 2] = lum
    return out


def opaque(img):
    out = img.copy()
    out[:, :, 3] = 1.0
    return out


def fade_bottom(img, solid_to_row, clear_from_row):
    """
    Alpha unchanged down to solid_to_row, then fading smoothly to 0 at
    clear_from_row, and 0 below that (rows counted from the top).
    """
    out = img.copy()
    rows = out.shape[0]
    y = np.arange(rows, dtype=np.float32)
    span = max(1.0, float(clear_from_row - solid_to_row))
    t = np.clip((y - solid_to_row) / span, 0.0, 1.0)
    keep = 1.0 - t * t * (3.0 - 2.0 * t)          # smoothstep 1 -> 0
    out[:, :, 3] *= keep[:, None]
    return out


def clear_rows(img, top=0, bottom=0):
    """
    Make the first `top` and last `bottom` rows fully transparent. A texture
    in a repeating (clamp2edge="false") atlas also wraps vertically, so a
    border strip whose u repeats along an edge would otherwise pick up its
    bottom row along its top edge (and the other way round) through
    filtering.
    """
    out = img.copy()
    if top:
        out[:top, :, 3] = 0.0
    if bottom:
        out[out.shape[0] - bottom:, :, 3] = 0.0
    return out


def seam_error(img):
    """
    How well an image repeats: mean absolute difference across the wrap
    (last column to first, last row to first) divided by the same between
    neighbouring columns/rows inside the image. About 1 means the seams are
    no different from the rest of the image.
    """
    a = img[:, :, :3]
    inner_x = np.abs(a[:, 1:] - a[:, :-1]).mean()
    inner_y = np.abs(a[1:] - a[:-1]).mean()
    wrap_x = np.abs(a[:, 0] - a[:, -1]).mean()
    wrap_y = np.abs(a[0] - a[-1]).mean()
    return wrap_x / max(inner_x, 1e-6), wrap_y / max(inner_y, 1e-6)
