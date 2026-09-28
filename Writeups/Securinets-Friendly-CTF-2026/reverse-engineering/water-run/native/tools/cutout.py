#!/usr/bin/env python3
"""Turn a white-background concept render into a billboard sprite with alpha.

Flood-fills transparency inward from the image border only, so white *inside* the
character (shirt logos, sneakers, bottle highlights) survives. A plain
"every pixel near white becomes transparent" pass eats those and leaves holes.
"""
import argparse
import pathlib
from collections import deque

from PIL import Image


def cutout(src: pathlib.Path, dst: pathlib.Path, tol: int, feather: int) -> None:
    img = Image.open(src).convert("RGBA")
    w, h = img.size
    px = img.load()

    def is_bg(x: int, y: int) -> bool:
        r, g, b, _ = px[x, y]
        return r >= 255 - tol and g >= 255 - tol and b >= 255 - tol

    seen = bytearray(w * h)
    q = deque()
    for x in range(w):
        for y in (0, h - 1):
            if is_bg(x, y) and not seen[y * w + x]:
                seen[y * w + x] = 1
                q.append((x, y))
    for y in range(h):
        for x in (0, w - 1):
            if is_bg(x, y) and not seen[y * w + x]:
                seen[y * w + x] = 1
                q.append((x, y))

    while q:
        x, y = q.popleft()
        px[x, y] = (255, 255, 255, 0)
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < h and not seen[ny * w + nx] and is_bg(nx, ny):
                seen[ny * w + nx] = 1
                q.append((nx, ny))

    # Soften the cut edge so the sprite does not read as a paper cutout in 3D.
    if feather:
        alpha = img.getchannel("A")
        from PIL import ImageFilter
        img.putalpha(alpha.filter(ImageFilter.GaussianBlur(feather)))

    img = img.crop(img.getbbox())
    dst.parent.mkdir(parents=True, exist_ok=True)
    img.save(dst)
    print(f"cutout: {dst.name} {img.size[0]}x{img.size[1]}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("dst")
    ap.add_argument("--tol", type=int, default=18)
    ap.add_argument("--feather", type=int, default=1)
    args = ap.parse_args()
    cutout(pathlib.Path(args.src), pathlib.Path(args.dst), args.tol, args.feather)


if __name__ == "__main__":
    main()
