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
    args = p.parse_args()
    if args.cmd == "add-res":
        ensure_res_file(args.res)
        ensure_scene_map()
        for s in args.scenes:
            map_scene(s, args.res)
    else:
        p.print_help()
        sys.exit(1)
