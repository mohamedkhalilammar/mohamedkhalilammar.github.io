#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

build_variant() {
  local variant=$1
  local label=$2
  CG_VARIANT="$variant" ./native/build.sh
  godot --headless --path game1 --export-release "Windows Desktop"
  cp game1/build/WaterRun.exe "game1/build/WaterRun-$label.exe"
  cp game1/build/cgchallenge.dll "game1/build/cgchallenge-${variant}.dll"
  zip -j -FS "game1/build/WaterRun-$label-windows-x64.zip" \
    "game1/build/WaterRun-$label.exe" game1/build/cgchallenge.dll
}

build_variant beginner Beginner
build_variant advanced Advanced
sha256sum game1/build/WaterRun-{Beginner,Advanced}-windows-x64.zip
