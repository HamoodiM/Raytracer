"""Generate the procedural brick textures used by the demo scenes.

Writes assets/textures/bricks_albedo.png and assets/textures/bricks_normal.png.
Only needs numpy and the standard library (PNG is written with zlib).

Normal map convention: tangent-space, OpenGL style (+green = image up),
encoded as rgb = (n + 1) / 2, so a flat surface is (128, 128, 255).
"""
import os
import struct
import sys
import zlib

import numpy as np

W, H = 1024, 512          # 2:1 so it maps nicely onto a sphere's (u, v)
ROWS, COLS = 12, 12       # bricks (2:1 bricks on a 2:1 image)
MORTAR = 0.10             # mortar width as a fraction of brick height
BEVEL = 0.12              # bevel width as a fraction of brick height
STRENGTH = 6.0            # height-to-normal scale


def write_png(path, rgb):
    h, w, _ = rgb.shape
    raw = b"".join(b"\x00" + rgb[y].tobytes() for y in range(h))

    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def main(out_dir):
    rng = np.random.default_rng(7)
    y, x = np.mgrid[0:H, 0:W].astype(np.float64)
    bh = H / ROWS
    bw = W / COLS
    row = np.floor(y / bh)
    xs = x + (row % 2) * (bw / 2)              # running bond: offset every other row
    col = np.floor(xs / bw)
    fy = (y - row * bh) / bh                   # 0..1 inside the brick (vertical)
    fx = (xs - col * bw) / bw                  # 0..1 inside the brick (horizontal)

    # distance to the nearest brick edge, in units of brick height
    d = np.minimum(np.minimum(fy, 1 - fy), np.minimum(fx, 1 - fx) * (bw / bh))
    m = MORTAR / 2
    height = np.clip((d - m) / BEVEL, 0.0, 1.0)
    height = height * height * (3 - 2 * height)  # smoothstep bevel
    # small per-pixel roughness on the brick faces
    noise = rng.normal(0, 1, (H, W))
    for _ in range(2):                           # cheap blur
        noise = (noise + np.roll(noise, 1, 0) + np.roll(noise, 1, 1) + np.roll(noise, -1, 0) + np.roll(noise, -1, 1)) / 5
    height = height + 0.04 * noise * (height > 0.5)

    # gradients: d/dx along columns, d/drow along rows (image down)
    dhdx = (np.roll(height, -1, 1) - np.roll(height, 1, 1)) * 0.5
    dhdrow = (np.roll(height, -1, 0) - np.roll(height, 1, 0)) * 0.5
    # height field h(x, y_up) with y_up = -row: n ~ (-dh/dx, -dh/dy_up, 1) = (-dh/dx, +dh/drow, 1)
    n = np.stack([-dhdx * STRENGTH, dhdrow * STRENGTH, np.ones_like(height)], axis=-1)
    n /= np.linalg.norm(n, axis=-1, keepdims=True)
    normal_rgb = np.clip((n + 1) * 0.5 * 255 + 0.5, 0, 255).astype(np.uint8)

    # albedo: per-brick tint, mortar is light grey
    brick_id = (row * 1000 + col).astype(np.int64)
    tint = np.random.default_rng(3).uniform(0.75, 1.15, size=int(brick_id.max()) + 1)
    base = np.array([0.62, 0.24, 0.16])
    brick = base[None, None, :] * tint[brick_id][..., None] * (1 + 0.15 * noise[..., None])
    mortar = np.array([0.70, 0.68, 0.64])[None, None, :]
    w = np.clip((d - m) / 0.02, 0, 1)[..., None]
    albedo = mortar * (1 - w) + brick * w
    albedo_rgb = np.clip(albedo * 255 + 0.5, 0, 255).astype(np.uint8)

    os.makedirs(out_dir, exist_ok=True)
    write_png(os.path.join(out_dir, "bricks_normal.png"), normal_rgb)
    write_png(os.path.join(out_dir, "bricks_albedo.png"), albedo_rgb)
    print("wrote", out_dir)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join("assets", "textures"))
