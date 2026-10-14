#!/bin/bash
# Leapfrog iOS build - runs on the Mac.
# Usage: build-ios.sh [Debug|Release] [pull|nopull] [install|noinstall] [device] [scheme]
set -uo pipefail

CONFIG="${1:-Release}"
PULL="${2:-pull}"
INSTALL="${3:-noinstall}"
DEVICE="${4:-}"
SCHEME="${5:-Leapfrog_ios_debug}"

REPO="$HOME/Documents/GitHub3/leapfrog"
PROJ="$REPO/leapfrog-game/proj.ios/Leapfrog_ios.xcodeproj"
DD="$HOME/leapfrog-build"
LOG="$DD/last-build.log"

mkdir -p "$DD"

if [ "$PULL" = "pull" ]; then
  echo "== git pull"
  git -C "$REPO" pull --ff-only || { echo "git pull failed (local changes on the Mac?)"; exit 1; }
fi

echo "== Unlocking login keychain (needed for code signing over SSH)"
security unlock-keychain "$HOME/Library/Keychains/login.keychain-db" || exit 1

echo "== Building $SCHEME ($CONFIG) for iOS device"
xcodebuild -project "$PROJ" -scheme "$SCHEME" -configuration "$CONFIG" \
  -destination 'generic/platform=iOS' -derivedDataPath "$DD" \
  -allowProvisioningUpdates -quiet build 2>&1 | tee "$LOG" \
  | grep -E "error:|Undefined symbol|symbol\(s\) not found|BUILD (SUCCEEDED|FAILED)" --line-buffered
STATUS=${PIPESTATUS[0]}

if [ "$STATUS" -ne 0 ]; then
  echo "== BUILD FAILED (full log on the Mac: $LOG)"
  exit "$STATUS"
fi
echo "== BUILD SUCCEEDED"

APP=$(find "$DD/Build/Products/$CONFIG-iphoneos" -maxdepth 1 -name '*.app' | head -n 1)
echo "App: $APP"

if [ "$INSTALL" = "install" ]; then
  if [ -z "$DEVICE" ]; then
    xcrun devicectl list devices --json-output /tmp/lf-devices.json >/dev/null 2>&1
    DEVICE=$(plutil -extract result.devices.0.identifier raw /tmp/lf-devices.json 2>/dev/null)
  fi
  if [ -z "$DEVICE" ]; then
    echo "No iPhone found. Connect it to the Mac (USB, or paired over Wi-Fi) and try again."
    exit 1
  fi
  BUNDLE=$(/usr/libexec/PlistBuddy -c 'Print CFBundleIdentifier' "$APP/Info.plist")
  echo "== Installing on device $DEVICE"
  xcrun devicectl device install app --device "$DEVICE" "$APP" || exit 1
  echo "== Launching $BUNDLE"
  xcrun devicectl device process launch --device "$DEVICE" "$BUNDLE"
fi
