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

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Find OpenMP (optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - enabling multi-threading")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(WARNING "OpenMP not found - single-threaded rendering only")
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
    src/materials/metal.cpp
    src/materials/dielectric.cpp
    src/materials/emissive.cpp
)

set(ACCELERATION_SOURCES
    src/acceleration/aabb.cpp
    src/acceleration/bvh.cpp
)

set(RENDERING_SOURCES
    src/rendering/renderer.cpp
    src/rendering/integrator.cpp
    src/rendering/framebuffer.cpp
)

set(VIEWER_SOURCES
    src/viewer/window.cpp
    src/viewer/controller.cpp
)

# Header files
set(HEADERS
    src/core/ray.h
    src/core/camera.h
    src/core/scene.h
    src/core/utils.h
    src/geometry/primitive.h
    src/geometry/sphere.h
    src/geometry/plane.h
    src/geometry/triangle.h
    src/geometry/mesh.h
    src/materials/material.h
    src/materials/lambertian.h
    src/materials/metal.h
    src/materials/dielectric.h
    src/materials/emissive.h
    src/acceleration/aabb.h
    src/acceleration/bvh.h
    src/rendering/renderer.h
    src/rendering/integrator.h
    src/rendering/framebuffer.h
    src/viewer/window.h
    src/viewer/controller.h
)

# Create executable
add_executable(raytracer
    src/main.cpp
    ${CORE_SOURCES}
    ${GEOMETRY_SOURCES}
    ${MATERIAL_SOURCES}
    ${ACCELERATION_SOURCES}
    ${RENDERING_SOURCES}
    ${VIEWER_SOURCES}
    ${HEADERS}
)

# Link libraries
target_link_libraries(raytracer
    OpenGL::GL
    glfw
    glm::glm
)

# Link OpenMP if available
if(OpenMP_CXX_FOUND)
    target_link_libraries(raytracer OpenMP::OpenMP_CXX)
endif()

# Set output directory
set_target_properties(raytracer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY scenes DESTINATION ${CMAKE_BINARY_DIR})

# Installation
install(TARGETS raytracer DESTINATION bin)
install(DIRECTORY assets DESTINATION share/raytracer)
install(DIRECTORY scenes DESTINATION share/raytracer)
