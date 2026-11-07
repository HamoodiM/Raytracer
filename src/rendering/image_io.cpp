/**
 * @file image_io.cpp
 * @brief PPM and stored-deflate PNG writers
 */

#include "image_io.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace raytracer {
namespace rendering {

namespace {

uint32_t crc32_update(uint32_t crc, const uint8_t* data, size_t len) {
    static uint32_t table[256];
    static bool initialized = false;
    if (!initialized) {
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[n] = c;
        }
        initialized = true;
    }
    crc = ~crc;
    for (size_t i = 0; i < len; ++i) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

void put_u32_be(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>(v >> 24));
    out.push_back(static_cast<uint8_t>(v >> 16));
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v));
}

void write_chunk(std::FILE* f, const char type[4], const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buf;
    put_u32_be(buf, static_cast<uint32_t>(data.size()));
    buf.insert(buf.end(), type, type + 4);
    buf.insert(buf.end(), data.begin(), data.end());
    uint32_t crc = crc32_update(0, buf.data() + 4, buf.size() - 4);  // type + data
    put_u32_be(buf, crc);
    std::fwrite(buf.data(), 1, buf.size(), f);
}

bool ends_with(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin(),
                      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

} // namespace

bool write_ppm(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb) {
    std::FILE* f = std::fopen(filename.c_str(), "wb");
    if (!f) return false;
    std::fprintf(f, "P6\n%d %d\n255\n", width, height);
    std::fwrite(rgb.data(), 1, rgb.size(), f);
    return std::fclose(f) == 0;
}

bool write_png(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb) {
    std::FILE* f = std::fopen(filename.c_str(), "wb");
    if (!f) return false;

    static const uint8_t signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    std::fwrite(signature, 1, 8, f);

    std::vector<uint8_t> ihdr;
    put_u32_be(ihdr, static_cast<uint32_t>(width));
    put_u32_be(ihdr, static_cast<uint32_t>(height));
    ihdr.push_back(8);  // bit depth
    ihdr.push_back(2);  // colour type: truecolour RGB
    ihdr.push_back(0);  // compression: deflate
    ihdr.push_back(0);  // filter method
    ihdr.push_back(0);  // no interlace
    write_chunk(f, "IHDR", ihdr);

    // Raw scanlines, each prefixed with filter type 0 (none)
    const size_t row_bytes = static_cast<size_t>(width) * 3;
    std::vector<uint8_t> raw;
    raw.reserve((row_bytes + 1) * height);
    for (int y = 0; y < height; ++y) {
        raw.push_back(0);
        raw.insert(raw.end(), rgb.begin() + y * row_bytes, rgb.begin() + (y + 1) * row_bytes);
    }

    // zlib stream: header, stored deflate blocks (<= 65535 bytes each), Adler-32
    std::vector<uint8_t> z;
    z.reserve(raw.size() + raw.size() / 65535 * 5 + 16);
    z.push_back(0x78);
    z.push_back(0x01);
    size_t pos = 0;
    do {
        size_t len = std::min<size_t>(65535, raw.size() - pos);
        bool final_block = pos + len == raw.size();
        z.push_back(final_block ? 1 : 0);  // BFINAL, BTYPE = 00 (stored)
        z.push_back(static_cast<uint8_t>(len & 0xFF));
        z.push_back(static_cast<uint8_t>(len >> 8));
        z.push_back(static_cast<uint8_t>(~len & 0xFF));
        z.push_back(static_cast<uint8_t>((~len >> 8) & 0xFF));
        z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + len);
        pos += len;
    } while (pos < raw.size());
    uint32_t a = 1, b = 0;
    for (uint8_t byte : raw) {
        a = (a + byte) % 65521;
        b = (b + a) % 65521;
    }
    put_u32_be(z, (b << 16) | a);
    write_chunk(f, "IDAT", z);
    write_chunk(f, "IEND", {});

    return std::fclose(f) == 0;
}

bool write_image(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb) {
    if (ends_with(filename, ".png")) return write_png(filename, width, height, rgb);
    return write_ppm(filename, width, height, rgb);
}

} // namespace rendering
} // namespace raytracer
