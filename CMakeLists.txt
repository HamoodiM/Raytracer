cmake_minimum_required(VERSION 3.16)
project(RayTracer VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# GLM: use an installed package if there is one, otherwise the vendored copy
# in third_party/glm (GLM 1.0.1 "light" release, header-only, MIT licence).
find_package(glm QUIET)
if(NOT TARGET glm::glm)
    message(STATUS "Using vendored GLM from third_party/glm")
    add_library(glm_vendored INTERFACE)
    target_include_directories(glm_vendored INTERFACE ${CMAKE_SOURCE_DIR}/third_party/glm)
    add_library(glm::glm ALIAS glm_vendored)
endif()

# The interactive viewer needs GLFW + OpenGL. Without them only the headless
# renderer and the tests are built.
find_package(OpenGL QUIET)
find_package(glfw3 QUIET)
if(OpenGL_FOUND AND glfw3_FOUND)
    set(RAYTRACER_BUILD_VIEWER ON)
else()
    set(RAYTRACER_BUILD_VIEWER OFF)
    message(STATUS "GLFW/OpenGL not found - building raytracer_cli only (no viewer)")
endif()

# Find OpenMP (optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - enabling multi-threading")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    # Try to find OpenMP manually on macOS
    find_library(OPENMP_LIBRARY omp PATHS /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib)
    find_path(OPENMP_INCLUDE_DIR omp.h PATHS /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include)
    
    if(OPENMP_LIBRARY AND OPENMP_INCLUDE_DIR)
        message(STATUS "OpenMP found manually - enabling multi-threading")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lomp")
        include_directories(${OPENMP_INCLUDE_DIR})
    else()
        message(WARNING "OpenMP not found - single-threaded rendering only")
    endif()
endif()

# Include directories
include_directories(src)
include_directories(external)

# Source files
set(CORE_SOURCES
    src/core/ray.cpp
    src/core/camera.cpp
    src/core/scene.cpp
    src/core/utils.cpp
    src/core/light.cpp
    src/core/scene_builder.cpp
    src/core/scene_loader.cpp
    src/core/scene_gallery.cpp
)

set(GEOMETRY_SOURCES
    src/geometry/primitive.cpp
    src/geometry/sphere.cpp
    src/geometry/plane.cpp
    src/geometry/triangle.cpp
    src/geometry/mesh.cpp
)

set(MATERIAL_SOURCES
    src/materials/material.cpp
    src/materials/lambertian.cpp
    src/materials/textured_lambertian.cpp
    src/materials/metal.cpp
    src/materials/dielectric.cpp
    src/materials/emissive.cpp
)

set(TEXTURE_SOURCES
    src/textures/texture.cpp
    src/textures/image_texture.cpp
    src/textures/checker_texture.cpp
    src/textures/normal_map.cpp
)

set(ACCELERATION_SOURCES
    src/acceleration/aabb.cpp
    src/acceleration/bvh.cpp
)

set(RENDERING_SOURCES
    src/rendering/renderer.cpp
    src/rendering/integrator.cpp
    src/rendering/framebuffer.cpp
    src/rendering/progressive_renderer.cpp
    src/rendering/image_io.cpp
)

set(VIEWER_SOURCES
    src/viewer/window.cpp
    src/viewer/controller.cpp
    src/viewer/camera_controller.cpp
)

# Header files
set(HEADERS
    src/core/ray.h
    src/core/camera.h
    src/core/scene.h
    src/core/utils.h
    src/core/light.h
    src/core/scene_builder.h
    src/core/scene_loader.h
    src/core/scene_gallery.h
    src/geometry/primitive.h
    src/geometry/sphere.h
    src/geometry/plane.h
    src/geometry/triangle.h
    src/geometry/mesh.h
    src/materials/material.h
    src/materials/lambertian.h
    src/materials/textured_lambertian.h
    src/materials/metal.h
    src/materials/dielectric.h
    src/materials/emissive.h
    src/textures/texture.h
    src/textures/image_texture.h
    src/textures/checker_texture.h
    src/textures/normal_map.h
    src/acceleration/aabb.h
    src/acceleration/bvh.h
    src/rendering/renderer.h
    src/rendering/integrator.h
    src/rendering/framebuffer.h
    src/rendering/progressive_renderer.h
    src/rendering/image_io.h
    src/viewer/window.h
    src/viewer/controller.h
    src/viewer/camera_controller.h
)

# Everything except the window/viewer code
add_library(raytracer_core STATIC
    ${CORE_SOURCES}
    ${GEOMETRY_SOURCES}
    ${MATERIAL_SOURCES}
    ${TEXTURE_SOURCES}
    ${ACCELERATION_SOURCES}
    ${RENDERING_SOURCES}
)
target_link_libraries(raytracer_core PUBLIC glm::glm)
if(OpenMP_CXX_FOUND)
    target_link_libraries(raytracer_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# Headless command-line renderer (no GLFW/OpenGL needed)
add_executable(raytracer_cli src/cli/headless_main.cpp)
target_link_libraries(raytracer_cli raytracer_core)

# Interactive progressive viewer
if(RAYTRACER_BUILD_VIEWER)
    add_executable(raytracer
        src/main.cpp
        ${VIEWER_SOURCES}
        ${HEADERS}
    )
    target_link_libraries(raytracer raytracer_core OpenGL::GL glfw)
endif()

# BVH correctness test: BVH closest hits must equal brute-force closest hits
enable_testing()
add_executable(bvh_test tests/bvh_test.cpp)
target_link_libraries(bvh_test raytracer_core)
add_test(NAME bvh_matches_brute_force COMMAND bvh_test)

# Brute force vs BVH ray-casting benchmark (not a test; prints a table)
add_executable(bvh_bench tests/bvh_bench.cpp)
target_link_libraries(bvh_bench raytracer_core)

# Set output directory
set_target_properties(raytracer_cli bvh_test bvh_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
if(RAYTRACER_BUILD_VIEWER)
    set_target_properties(raytracer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY scenes DESTINATION ${CMAKE_BINARY_DIR})

# Installation
install(TARGETS raytracer_cli DESTINATION bin)
if(RAYTRACER_BUILD_VIEWER)
    install(TARGETS raytracer DESTINATION bin)
endif()
install(DIRECTORY assets DESTINATION share/raytracer)
install(DIRECTORY scenes DESTINATION share/raytracer)
