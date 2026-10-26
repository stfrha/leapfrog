#!/bin/bash
# Builds and runs the mission tester on macOS (or Linux).
#
#   ./build.sh            build, then run all tests
#   ./build.sh norun      build only
#   ./build.sh -- <args>  build, then run with these arguments
#                         (e.g. -- --filter "Bruno" --verbose)
#
# Needs the Lua sources next to the repo (../../../lua from here), the same
# checkout the game builds with. The executable goes to build/.

set -e
cd "$(dirname "$0")"

LUA_DIR="../../../lua"
if [ ! -f "$LUA_DIR/onelua.c" ]; then
   echo "build.sh: no Lua sources at $LUA_DIR (expected onelua.c)" >&2
   exit 2
fi

mkdir -p build
EXTRA=""
if [ "$(uname)" = "Linux" ]; then
   EXTRA="-lm"
fi
cc -O2 -std=c99 -w -DLUA_USE_POSIX -I"$LUA_DIR" -o build/mission_tester main.c $EXTRA

if [ "$1" = "norun" ]; then
   echo "Built build/mission_tester"
   exit 0
fi

if [ "$1" = "--" ]; then
   shift
fi

./build/mission_tester "$@"
