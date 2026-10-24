#!/bin/bash
# Leapfrog macOS build - runs on the Mac itself (for working on the road).
# Usage: build-macos.sh [Debug|Release] [pull|nopull] [run|norun]
#
# Expects the same folder layout as the iOS build: next to the leapfrog
# repo there must be oxygine-framework, SDL (branch leapfrog), Box2D and lua.
# The app is built to ~/leapfrog-build-macos/Products/<config>/Leapfrog_macosx.app
# with everything from leapfrog-game/data copied into Contents/Resources.
# "run" starts it from the terminal, so the game's log shows up here.
set -uo pipefail

CONFIG="${1:-Debug}"
PULL="${2:-nopull}"
RUN="${3:-run}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$SCRIPT_DIR/../.." && pwd)"
PARENT="$(dirname "$REPO")"
PROJ="$REPO/leapfrog-game/proj.macosx/Leapfrog_macosx.xcodeproj"
BD="$HOME/leapfrog-build-macos"
LOG="$BD/last-build.log"

mkdir -p "$BD"

MISSING=0
for d in oxygine-framework SDL Box2D lua; do
  if [ ! -d "$PARENT/$d" ]; then
    echo "Missing $PARENT/$d (must sit next to the leapfrog repo)"
    MISSING=1
  fi
done
[ "$MISSING" -eq 0 ] || exit 1

if [ "$PULL" = "pull" ]; then
  for d in "$REPO" "$PARENT/oxygine-framework" "$PARENT/SDL"; do
    echo "== git pull in $d"
    git -C "$d" pull --ff-only || { echo "git pull failed in $d (local changes?)"; exit 1; }
  done
fi

echo "== Building Leapfrog_macosx ($CONFIG, $(uname -m))"
xcodebuild -project "$PROJ" -target Leapfrog_macosx -configuration "$CONFIG" \
  SYMROOT="$BD/Products" OBJROOT="$BD/Intermediates" \
  -quiet build 2>&1 | tee "$LOG" \
  | grep -E "error:|Undefined symbol|symbol\(s\) not found|BUILD (SUCCEEDED|FAILED)" --line-buffered
STATUS=${PIPESTATUS[0]}

if [ "$STATUS" -ne 0 ]; then
  echo "== BUILD FAILED (full log: $LOG)"
  exit "$STATUS"
fi
echo "== BUILD SUCCEEDED"

APP="$BD/Products/$CONFIG/Leapfrog_macosx.app"
echo "App: $APP"

if [ "$RUN" = "run" ]; then
  echo "== Running (close the window or Cmd+Q to quit)"
  "$APP/Contents/MacOS/Leapfrog_macosx"
fi
