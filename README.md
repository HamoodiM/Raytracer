# Advanced C++ Raytracer

A physically-based raytracer implemented in C++ featuring path tracing, multiple materials, and BVH acceleration structures.

## Features

- **Path Tracing**: Monte Carlo integration with importance sampling
- **Materials**: Lambertian (diffuse), Metal (reflective), Dielectric (refractive), Emissive (light sources)
- **Geometry**: Spheres, planes, triangles, and triangle meshes
- **Acceleration**: BVH (Bounding Volume Hierarchy) for fast ray-primitive intersection
- **Multi-threading**: OpenMP support for parallel rendering
- **Cross-platform**: Builds on macOS and Windows

## Dependencies

- **CMake** (3.16+)
- **GLFW3** (windowing)
- **GLM** (OpenGL Mathematics)
- **OpenMP** (optional, for multi-threading)

### macOS Installation

```bash
# Install dependencies via Homebrew
brew install cmake glfw glm

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows Installation

```bash
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install

# Install dependencies
./vcpkg install glfw3 glm

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Usage

```bash
# Run the raytracer
./bin/raytracer 1

# The program will render a scene and save the result as output.ppm
```
test scenes range between 1 and 4

## Project Structure

```
RayTracer/
├── src/
│   ├── core/           # Core raytracing components (Ray, Camera, Scene)
│   ├── geometry/       # Primitive shapes (Sphere, Plane, Triangle, Mesh)
│   ├── materials/      # Material types (Lambertian, Metal, Dielectric, Emissive)
│   ├── acceleration/   # BVH acceleration structure
│   ├── rendering/      # Renderer, integrator, framebuffer
│   └── viewer/         # Window management and controls
├── scenes/             # JSON scene files
├── assets/             # Textures and other assets
└── external/           # Third-party headers
```

## Development Status

This project is currently in **Sprint 1** - Foundation & Basic Rendering.

### Completed
- Project structure and CMake build system
- Core math classes (Ray, Vec3, Color)
- Camera with configurable FOV and positioning
- Sphere primitive with ray intersection
- Lambertian material (diffuse reflection)
- Basic renderer with framebuffer
- GLFW window setup

### In Progress
- Basic scene rendering with spheres

### Planned
- [ ] Metal and Dielectric materials
- [ ] Path tracing integrator
- [ ] Additional primitives (Plane, Triangle, Mesh)
- [ ] BVH acceleration structure
- [ ] Multi-threading support
- [ ] Interactive camera controls
- [ ] Scene file loading

## Technical Details

### Ray-Primitive Intersection
- **Spheres**: Analytical solution using quadratic formula
- **Planes**: Ray-plane intersection with normal calculation
- **Triangles**: Möller-Trumbore algorithm

### Path Tracing Algorithm
- Monte Carlo integration with cosine-weighted hemisphere sampling
- Russian roulette for path termination
- Gamma correction and tone mapping

### BVH Construction
- Surface Area Heuristic (SAH) for optimal partitioning
- Binning method for efficient partition selection

## License

This project is for educational and portfolio purposes.
