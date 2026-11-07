/**
 * @file image_io.h
 * @brief Dependency-free 8-bit RGB image writers (binary PPM and PNG)
 *
 * The PNG writer uses uncompressed ("stored") deflate blocks, so it needs no
 * zlib. Files are larger than a compressed PNG but valid for every viewer.
 * Only C stdio is used, so this also works on targets without <fstream>
 * (e.g. wasm32-wasi).
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace raytracer {
namespace rendering {

// rgb: width * height * 3 bytes, rows top to bottom
bool write_ppm(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb);
bool write_png(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb);

// Chooses the format from the extension (.png, otherwise PPM)
bool write_image(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb);

} // namespace rendering
} // namespace raytracer
