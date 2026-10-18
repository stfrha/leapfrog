"""
project_files.py - keep the game's data files and build projects in sync.
Plain Python (no Blender), so it can also be run on its own:

    python project_files.py add-res res_mining.xml --scenes landing_scene.xml

How data reaches the builds:
  * Windows runs from leapfrog-game/data.
  * Android packs the whole data/ folder (build.gradle: assets.srcDirs).
  * iOS / macOS: data/images is a folder reference (every file ships), but
    each XML file in data/ is listed in the Xcode project one by one. A new
    resource file therefore has to be added to project.pbxproj; that is
    done here, using the existing res_hud.xml entries as the pattern.
"""

import os
import random
import re
import sys

SPRITES_DIR = os.path.dirname(os.path.abspath(__file__))
GAME_DIR = os.path.dirname(os.path.dirname(SPRITES_DIR))   # leapfrog-game
DATA_DIR = os.path.join(GAME_DIR, "data")
SCENE_MAP = os.path.join(DATA_DIR, "scene_resources.xml")
XCODE_PROJECTS = [
    os.path.join(GAME_DIR, "proj.ios", "Leapfrog_ios.xcodeproj", "project.pbxproj"),
    os.path.join(GAME_DIR, "proj.macosx", "Leapfrog_macosx.xcodeproj", "project.pbxproj"),
]
ANCHOR = "res_hud.xml"


def _read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8")


def _write(path, text):
    with open(path, "wb") as f:
        f.write(text.encode("utf-8"))


def _nl(text):
    return "\r\n" if "\r\n" in text else "\n"


# ---------------------------------------------------------------------------
# Xcode
# ---------------------------------------------------------------------------

def _new_id(text):
    while True:
        i = "".join(random.choice("0123456789ABCDEF") for _ in range(24))
        if i not in text:
            return i


def add_data_file_to_xcode(filename, pbxproj):
    """Add data/<filename> to an Xcode project as a bundled resource. Returns True if changed."""
    if not os.path.exists(pbxproj):
        return False
    text = _read(pbxproj)
    if ("path = ../data/%s;" % filename) in text:
        return False

    a = re.escape(ANCHOR)
    m_build = re.search(r"^(\s*)([0-9A-F]{24}) /\* %s in Resources \*/ = \{isa = PBXBuildFile; "
                        r"fileRef = ([0-9A-F]{24}) /\* %s \*/; \};[ \t]*\r?$" % (a, a), text, re.M)
    if not m_build:
        print("project_files: no %s pattern in %s, add %s to the project by hand" % (ANCHOR, pbxproj, filename))
        return False
    build_id, ref_id = m_build.group(2), m_build.group(3)
    m_ref = re.search(r"^\s*%s /\* %s \*/ = \{isa = PBXFileReference;.*\r?$" % (ref_id, a), text, re.M)
    m_group = re.search(r"^\s*%s /\* %s \*/,[ \t]*\r?$" % (ref_id, a), text, re.M)
    m_phase = re.search(r"^\s*%s /\* %s in Resources \*/,[ \t]*\r?$" % (build_id, a), text, re.M)
    if not (m_ref and m_group and m_phase):
        print("project_files: incomplete %s pattern in %s, add %s by hand" % (ANCHOR, pbxproj, filename))
        return False

    new_build = _new_id(text)
    new_ref = _new_id(text + new_build)

    def clone(match):
        line = match.group(0).rstrip("\r")
        line = line.replace(build_id, new_build).replace(ref_id, new_ref).replace(ANCHOR, filename)
        return line

    # Insert after each anchor line, last position first so offsets stay valid
    inserts = sorted([(m_build.end(), clone(m_build)), (m_ref.end(), clone(m_ref)),
                      (m_group.end(), clone(m_group)), (m_phase.end(), clone(m_phase))],
                     key=lambda t: t[0], reverse=True)
    nl = _nl(text)
    for pos, line in inserts:
        # pos is at the end of the anchor line (before its newline)
        eol = text.find("\n", pos)
        eol = len(text) if eol < 0 else eol + 1
        text = text[:eol] + line + nl + text[eol:]
    _write(pbxproj, text)
    print("project_files: added %s to %s" % (filename, os.path.relpath(pbxproj, GAME_DIR)))
    return True


# ---------------------------------------------------------------------------
# Resource files and the scene map
# ---------------------------------------------------------------------------

def ensure_res_file(res_name):
    """Create data/<res_name> (empty resource set) if missing and add it to the Xcode projects."""
    path = os.path.join(DATA_DIR, res_name)
    if not os.path.exists(path):
        _write(path, '<?xml version="1.0"?>\n<resources>\n\t<set path = "images" />\n</resources>\n')
        print("project_files: created data/%s" % res_name)
    for proj in XCODE_PROJECTS:
        add_data_file_to_xcode(res_name, proj)
    return path


def ensure_scene_map():
    if not os.path.exists(SCENE_MAP):
        _write(SCENE_MAP,
               '<?xml version="1.0" encoding="utf-8"?>\n'
               '<!-- Which resource file each scene loads in addition to res.xml.\n'
               '     The set is loaded when the scene starts and freed when a scene\n'
               '     with another set starts. Scenes not listed use res.xml only. -->\n'
               '<sceneResources>\n'
               '</sceneResources>\n')
        print("project_files: created data/scene_resources.xml")
    for proj in XCODE_PROJECTS:
        add_data_file_to_xcode("scene_resources.xml", proj)


def map_scene(scene_file, res_name):
    """Make scene_file use res_name. Returns True if the map changed."""
    ensure_scene_map()
    text = _read(SCENE_MAP)
    m = re.search(r'<scene\s+file="%s"\s+resources="([^"]*)"\s*/>' % re.escape(scene_file), text)
    if m:
        if m.group(1) == res_name:
            return False
        text = text[:m.start(1)] + res_name + text[m.end(1):]
    else:
        nl = _nl(text)
        end = text.rfind("</sceneResources>")
        line_start = text.rfind("\n", 0, end) + 1
        text = (text[:line_start] + '\t<scene file="%s" resources="%s"/>' % (scene_file, res_name) + nl
                + text[line_start:])
    _write(SCENE_MAP, text)
    print("project_files: %s uses %s" % (scene_file, res_name))
    return True


# ---------------------------------------------------------------------------
# Textures in resource files
# ---------------------------------------------------------------------------
# In oxygine a <set path = "..."/> applies to everything after it in the
# file, and a texture's name is its file name without folder. Every block
# written here starts with its own <set path>, so blocks can be inserted
# anywhere without affecting (or being affected by) the rest of the file.

_SET_RE = re.compile(r'<set\s+path\s*=\s*"([^"]*)"\s*/>')


def res_files():
    """All resource files in data/ (res.xml first)."""
    names = sorted(n for n in os.listdir(DATA_DIR) if n.startswith("res") and n.endswith(".xml"))
    if "res.xml" in names:
        names.remove("res.xml")
        names.insert(0, "res.xml")
    return names


def image_folder(res_name, filename):
    """Folder (relative to data/) that `filename` is loaded from in res_name, or None."""
    path = os.path.join(DATA_DIR, res_name)
    if not os.path.exists(path):
        return None
    text = _read(path)
    pos = text.find('file="%s"' % filename)
    if pos < 0:
        return None
    folder = ""
    for m in _SET_RE.finditer(text, 0, pos):
        folder = m.group(1).strip()
    return folder


def find_texture(filename):
    """(res_name, folder) of the first resource file that lists filename, or (None, None)."""
    for res in res_files():
        folder = image_folder(res, filename)
        if folder is not None:
            return res, folder
    return None, None


def is_shared_folder(folder):
    """Images in a folder named 'ambient' (e.g. images/mining/ambient) share one atlas."""
    return folder.replace("\\", "/").strip("/").split("/")[-1] == "ambient"


def _add_to_shared_atlas(text, folder, image_line, nl):
    """
    Insert image_line into the shared atlas for `folder` (the <atlas> right
    after <set path = "folder" />). Returns the new text, or None if the
    file has no such atlas yet.
    """
    m = re.search(r'<set\s+path\s*=\s*"%s"\s*/>[ \t]*\r?\n[ \t]*<atlas\b[^>]*>(.*?)</atlas>'
                  % re.escape(folder), text, re.S)
    if not m:
        return None
    close = m.end() - len("</atlas>")
    line_start = text.rfind("\n", 0, close) + 1
    return text[:line_start] + image_line + nl + text[line_start:]


def register_texture(res_name, filename, attrs="", folder="images", repeat=False):
    """
    Add an image to data/<res_name> in its own atlas, loaded from data/<folder>.
    Nothing happens if the file already lists it. Returns True if changed.

    repeat=False (default) gives a plain <atlas> (clamp to edge), right for
    sprites. repeat=True writes <atlas clamp2edge="false"> for textures that
    tile; on iOS (GLES2) those must have power-of-two sizes or render black.

    Images in an 'ambient' folder (small background pieces) all go in one
    shared atlas per folder instead, so a village of them is one texture.
    They keep oxygine's default extend (2 px padding with the edge pixels
    repeated), so neighbours in the atlas don't bleed into each other.
    """
    path = os.path.join(DATA_DIR, res_name)
    if res_name != "res.xml":
        ensure_res_file(res_name)
    if not os.path.exists(path):
        print("project_files: %s not found, register %s by hand" % (path, filename))
        return False
    text = _read(path)
    if ('file="%s"' % filename) in text:
        print("project_files: %s is already in %s (unchanged)" % (filename, res_name))
        return False
    nl = _nl(text)
    extra = (" " + attrs.strip()) if attrs and attrs.strip() else ""
    if is_shared_folder(folder) and not repeat:
        image_line = '\t\t<image file="%s"%s trim="false"/>' % (filename, extra)
        shared = _add_to_shared_atlas(text, folder, image_line, nl)
        if shared is not None:
            _write(path, shared)
            print("project_files: added %s to %s (shared atlas, data/%s)" % (filename, res_name, folder))
            return True
        # First image in this folder: a new block, written below like any other
    if is_shared_folder(folder) and not repeat:
        image_line = '\t\t<image file="%s"%s trim="false"/>' % (filename, extra)
    else:
        image_line = '\t\t<image file="%s"%s trim="false" extend="false"/>' % (filename, extra)
    block = ('\t<set path = "%s" />' % folder + nl +
             ('\t<atlas clamp2edge="false">' if repeat else '\t<atlas>') + nl +
             image_line + nl +
             '\t</atlas>' + nl)
    last_atlas = text.rfind("</atlas>")
    if last_atlas >= 0:
        eol = text.find("\n", last_atlas)
        insert_at = len(text) if eol < 0 else eol + 1
    else:
        end = text.rfind("</resources>")
        if end < 0:
            print("project_files: no </resources> in %s, register %s by hand" % (res_name, filename))
            return False
        insert_at = text.rfind("\n", 0, end) + 1
    text = text[:insert_at] + block + text[insert_at:]
    # A block inserted before entries that relied on an earlier <set path>
    # would change their folder; restore it after our block if needed.
    following = text[insert_at + len(block):]
    next_set = _SET_RE.search(following)
    next_entry = re.search(r"<(atlas|image|font)\b", following)
    if next_entry and (not next_set or next_entry.start() < next_set.start()):
        prev_folder = ""
        for m in _SET_RE.finditer(text, 0, insert_at):
            prev_folder = m.group(1).strip()
        restore = '\t<set path = "%s" />' % prev_folder + nl
        text = text[:insert_at + len(block)] + restore + text[insert_at + len(block):]
    _write(path, text)
    print("project_files: added %s to %s (from data/%s)" % (filename, res_name, folder))
    return True


def set_image_attr(filename, attr, value):
    """
    Set (or with value None remove) one attribute on filename's <image> entry,
    in whichever resource file lists it. Used for ppu, which follows the
    image when it is re-rendered at another density. Returns True if changed.
    """
    res, _folder = find_texture(filename)
    if res is None:
        return False
    path = os.path.join(DATA_DIR, res)
    text = _read(path)
    m = re.search(r'<image\s+file="%s"[^>]*?/>' % re.escape(filename), text)
    if not m:
        return False
    tag = m.group(0)
    a = re.search(r'\s%s="[^"]*"' % re.escape(attr), tag)
    if value is None:
        new_tag = tag if not a else tag[:a.start()] + tag[a.end():]
    elif a:
        new_tag = tag[:a.start()] + ' %s="%s"' % (attr, value) + tag[a.end():]
    else:
        f = re.match(r'<image\s+file="[^"]*"', tag)
        new_tag = tag[:f.end()] + ' %s="%s"' % (attr, value) + tag[f.end():]
    if new_tag == tag:
        return False
    _write(path, text[:m.start()] + new_tag + text[m.end():])
    print("project_files: %s in %s: %s" % (filename, res, ('%s="%s"' % (attr, value)) if value is not None
                                           else "removed " + attr))
    return True


def image_attr(filename, attr):
    """Value of one attribute on filename's <image> entry, or None."""
    res, _folder = find_texture(filename)
    if res is None:
        return None
    m = re.search(r'<image\s+file="%s"[^>]*?/>' % re.escape(filename), _read(os.path.join(DATA_DIR, res)))
    a = re.search(r'\s%s="([^"]*)"' % re.escape(attr), m.group(0)) if m else None
    return a.group(1) if a else None


def _atlas_repeats(atlas_tag):
    return re.search(r'clamp2edge\s*=\s*"false"', atlas_tag) is not None


def _remove_entry(text, filename):
    """
    Remove filename's entry; returns (text, other attributes of the entry,
    whether its atlas used repeat mode, i.e. clamp2edge="false").
    """
    f = re.escape(filename)
    single = re.search(r'[ \t]*<set\s+path\s*=\s*"[^"]*"\s*/>[ \t]*\r?\n'
                       r'(?=[ \t]*<atlas[^>]*>[ \t]*\r?\n[ \t]*<image\s+file="%s")' % f, text)
    block = re.search(r'[ \t]*<atlas[^>]*>[ \t]*\r?\n[ \t]*(<image\s+file="%s"[^>]*/>)[^\n]*\r?\n'
                      r'[ \t]*</atlas>[^\n]*\r?\n' % f, text)
    if block:
        tag = block.group(1)
        repeat = _atlas_repeats(re.match(r"[ \t]*(<atlas[^>]*>)", block.group(0)).group(1))
        start = single.start() if (single and single.end() == block.start()) else block.start()
        text = text[:start] + text[block.end():]
    else:
        line = re.search(r'[ \t]*(<image\s+file="%s"[^>]*/>)[^\n]*\r?\n' % f, text)
        if not line:
            return text, None, False
        tag = line.group(1)
        # Shared atlas: its mode is on the nearest <atlas ...> before the line
        opening = None
        for opening in re.compile(r"<atlas\b[^>]*>").finditer(text, 0, line.start()):
            pass
        repeat = _atlas_repeats(opening.group(0)) if opening else False
        text = text[:line.start()] + text[line.end():]
    attrs = re.sub(r'\s(file|trim|extend)="[^"]*"', "", tag[len("<image"):-2]).strip()
    return text, attrs, repeat


def move_texture(name, folder, res_name=None):
    """
    Move data/<old folder>/<name>.png to data/<folder>/ and update its resource
    entry (optionally moving it to another resource file). Returns True if done.
    """
    filename = name if name.endswith(".png") else name + ".png"
    old_res, old_folder = find_texture(filename)
    if old_res is None:
        print("project_files: %s is not in any resource file" % filename)
        return False
    res_name = res_name or old_res
    src = os.path.join(DATA_DIR, old_folder, filename)
    dst_dir = os.path.join(DATA_DIR, folder)
    dst = os.path.join(dst_dir, filename)
    if os.path.normcase(os.path.abspath(src)) == os.path.normcase(os.path.abspath(dst)) and res_name == old_res:
        print("project_files: %s is already in data/%s" % (filename, folder))
        return False
    if not os.path.exists(src):
        print("project_files: %s not found" % src)
        return False
    os.makedirs(dst_dir, exist_ok=True)
    if os.path.abspath(src) != os.path.abspath(dst):
        os.replace(src, dst)
    old_path = os.path.join(DATA_DIR, old_res)
    text, attrs, repeat = _remove_entry(_read(old_path), filename)
    _write(old_path, text)
    # Keep the texture's wrap mode: sprites stay clamped, tiling textures
    # keep repeating.
    register_texture(res_name, filename, attrs or "", folder, repeat)
    print("project_files: moved %s: data/%s -> data/%s" % (filename, old_folder, folder))
    return True


def _png_size(path):
    with open(path, "rb") as f:
        head = f.read(24)
    if head[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    return int.from_bytes(head[16:20], "big"), int.from_bytes(head[20:24], "big")


def _pow2(n):
    return n > 0 and (n & (n - 1)) == 0


def repeat_textures():
    """
    (res file, folder, filename, size, power-of-two) for every image whose
    atlas uses repeat mode (clamp2edge="false").
    """
    found = []
    for res in res_files():
        text = _read(os.path.join(DATA_DIR, res))
        for am in re.finditer(r"(<atlas\b[^>]*>)(.*?)</atlas>", text, re.S):
            if not _atlas_repeats(am.group(1)):
                continue
            for im in re.finditer(r'<image\s+file="([^"]+)"', am.group(2)):
                folder = ""
                for sm in _SET_RE.finditer(text, 0, am.start()):
                    folder = sm.group(1).strip()
                path = os.path.join(DATA_DIR, folder, im.group(1))
                size = _png_size(path) if os.path.exists(path) else None
                ok = size is not None and _pow2(size[0]) and _pow2(size[1])
                found.append((res, folder, im.group(1), size, ok))
    return found


def scenes_using(res_name):
    if not os.path.exists(SCENE_MAP):
        return []
    return re.findall(r'<scene\s+file="([^"]*)"\s+resources="%s"' % re.escape(res_name), _read(SCENE_MAP))


if __name__ == "__main__":
    import argparse
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd")
    a = sub.add_parser("add-res", help="create a resource file, add it to Xcode, map scenes to it")
    a.add_argument("res")
    a.add_argument("--scenes", nargs="*", default=[])
    m = sub.add_parser("move-texture", help="move textures to a folder and update their resource entries")
    m.add_argument("folder", help="folder under data/, e.g. images/mining")
    m.add_argument("names", nargs="+", help="texture names (with or without .png)")
    m.add_argument("--res", default=None, help="also move the entries to this resource file")
    c = sub.add_parser("check", help="list repeat-mode (clamp2edge=\"false\") textures that "
                       "aren't power-of-two; they render black on iOS")
    c.add_argument("--all", action="store_true", help="also list the power-of-two ones")
    w = sub.add_parser("where", help="show which resource file and folder each texture is in")
    w.add_argument("names", nargs="*", help="texture names (default: all)")
    args = p.parse_args()
    if args.cmd == "add-res":
        ensure_res_file(args.res)
        ensure_scene_map()
        for s in args.scenes:
            map_scene(s, args.res)
    elif args.cmd == "move-texture":
        folder = args.folder.replace("\\", "/").strip("/")
        for n in args.names:
            move_texture(n, folder, args.res)
    elif args.cmd == "check":
        bad = 0
        for res, folder, fn, size, ok in repeat_textures():
            if ok and not args.all:
                continue
            bad += 0 if ok else 1
            dims = ("%d x %d" % size) if size else "missing"
            print("%-4s %-32s %-16s %-24s %s" % ("ok" if ok else "BAD", fn, dims, res, "data/" + folder))
        if bad:
            print("%d repeat-mode texture(s) are not power-of-two. Sprites that don't tile should use a"
                  " plain <atlas>; textures that tile need power-of-two sizes." % bad)
        else:
            print("All repeat-mode textures are power-of-two.")
    elif args.cmd == "where":
        names = args.names
        if not names:
            names = []
            for res in res_files():
                names += re.findall(r'<image\s+file="([^"]+)"', _read(os.path.join(DATA_DIR, res)))
        for n in names:
            fn = n if n.endswith(".png") else n + ".png"
            res, folder = find_texture(fn)
            ppu = image_attr(fn, "ppu") if res else None
            print("%-40s %-20s %-32s %s" % (fn, res or "-", ("data/" + folder) if res else "-",
                                            ("ppu " + ppu) if ppu else ""))
    else:
        p.print_help()
        sys.exit(1)
