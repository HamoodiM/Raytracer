#!/usr/bin/env bash
# Builds the headless renderer and the tests as wasm32-wasi modules with zig's
# clang, for machines where native executables cannot be built or run.
#
#   ZIG_CXX="python -m ziglang c++" tools/wasi/build.sh     (from the repo root)
#   node tools/wasi/run.mjs build-wasm/raytracer_cli.wasm --scene ... --out ...
#
# ZIG_CXX defaults to "zig c++". OpenMP is not available on WASI, so these
# builds are single-threaded (the #pragma omp loops are compiled out).
set -euo pipefail
cd "$(dirname "$0")/../.."

CXX=${ZIG_CXX:-"zig c++"}
OUT=build-wasm
mkdir -p "$OUT"

FLAGS=(-std=c++17 -O2 -DNDEBUG -target wasm32-wasi -Isrc -Iexternal -Ithird_party/glm
       -Wl,-z,stack-size=8388608)

CORE=(src/core/ray.cpp src/core/camera.cpp src/core/scene.cpp src/core/utils.cpp
      src/core/light.cpp src/core/scene_builder.cpp src/core/scene_loader.cpp
      src/core/scene_gallery.cpp
      src/geometry/*.cpp src/materials/*.cpp src/textures/*.cpp src/acceleration/*.cpp
      src/rendering/renderer.cpp src/rendering/integrator.cpp src/rendering/framebuffer.cpp
      src/rendering/progressive_renderer.cpp src/rendering/image_io.cpp
      tools/wasi/exception_stubs.cpp)

build() {  # build <name> <main.cpp>
  echo "building $OUT/$1.wasm"
  $CXX "${FLAGS[@]}" "${CORE[@]}" "$2" -o "$OUT/$1.wasm"
}

build raytracer_cli src/cli/headless_main.cpp
build bvh_test tests/bvh_test.cpp
