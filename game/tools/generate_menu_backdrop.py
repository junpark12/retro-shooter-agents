#!/usr/bin/env python3
"""Original procedural Galaxy Storm menu art, dedicated to CC0 1.0.

Requires Pillow. Render at 240x320 and enlarge with nearest-neighbor sampling.
Run without arguments to regenerate the asset; --check compares without writing.
"""

import argparse
from io import BytesIO
import math
from pathlib import Path
import random
import sys

from PIL import Image, ImageDraw


WIDTH, HEIGHT = 240, 320
SEED = 0x6A1A7
DEFAULT_OUTPUT = (
    Path(__file__).resolve().parents[1]
    / "assets/sprites/ui/menu_backdrop.png"
)


def render():
    """Return the opaque RGBA backdrop with a local, fixed-seed random stream."""
    rng = random.Random(SEED)
    image = Image.new("RGB", (WIDTH, HEIGHT))
    pixels = image.load()

    # Low-amplitude grain breaks up quantization without visible gradient bands.
    for y in range(HEIGHT):
        for x in range(WIDTH):
            upper_haze = math.exp(
                -((x - 56) / 105) ** 2 - ((y + 10) / 63) ** 2
            )
            edge_haze = math.exp(
                -((x - 234) / 49) ** 2 - ((y - 220) / 125) ** 2
            )
            grain = rng.choice((-1, 0, 0, 0, 1))
            pixels[x, y] = (
                int(5 + 4 * upper_haze + 3 * edge_haze) + grain,
                int(8 + 8 * upper_haze + 2 * edge_haze) + grain,
                int(19 + 14 * upper_haze + 8 * edge_haze) + grain,
            )

    draw = ImageDraw.Draw(image)
    for _ in range(235):
        x, y = rng.randrange(7, WIDTH - 7), rng.randrange(4, HEIGHT - 14)
        quiet = 28 < x < 212 and 44 < y < 286
        if quiet and rng.random() < 0.76:
            continue
        value = rng.randrange(11, 24) if quiet else rng.randrange(24, 61)
        draw.point((x, y), fill=(value // 2, value, value + 13))
        if not quiet and value > 55 and y < 43:
            draw.line((x - 1, y, x + 1, y), fill=(24, 43, 62))
            draw.line((x, y - 1, x, y + 1), fill=(24, 43, 62))
            draw.point((x, y), fill=(81, 120, 140))

    # The planet and its tilted orbital ring stay above the primary text area.
    cx, cy, radius = 209, 4, 39

    def orbit(front):
        last = None
        for step in range(361):
            angle = step * math.pi / 180
            point = (
                round(cx + 54 * math.cos(angle)),
                round(cy + 12 * math.sin(angle) + 15 * math.cos(angle)),
            )
            visible = math.sin(angle) >= 0
            if last is not None and visible == front:
                draw.line((last, point), fill=(31, 56, 73) if front else (17, 30, 47))
            last = point

    orbit(False)
    for y in range(max(0, cy - radius), min(HEIGHT, cy + radius + 1)):
        for x in range(max(0, cx - radius), min(WIDTH, cx + radius + 1)):
            nx, ny = (x - cx) / radius, (y - cy) / radius
            distance = nx * nx + ny * ny
            if distance > 1:
                continue
            nz = math.sqrt(1 - distance)
            light = max(0, -0.70 * nx - 0.45 * ny + 0.40 * nz)
            clouds = (
                math.sin(ny * 25 + nx * 4 + math.sin(nx * 11))
                + math.sin(ny * 51 - nx * 9) * 0.35
            )
            grain = rng.choice((-1, 0, 0, 1))
            rim = max(0, 1 - nz * 7) * max(0, -nx) * 23
            pixels[x, y] = (
                max(0, int(9 + light * (13 + clouds * 3) + rim * 0.3) + grain),
                max(0, int(15 + light * (31 + clouds * 5) + rim) + grain),
                max(0, int(29 + light * (39 + clouds * 6) + rim * 1.3) + grain),
            )
    orbit(True)
    draw.line((164, 0, 165, 5, 171, 9), fill=(34, 66, 80))
    draw.rectangle((167, 6, 170, 8), fill=(62, 108, 121))

    # A distant dock silhouette balances the planet, rather than a second focal point.
    draw.polygon(
        [(0, 0), (68, 0), (68, 5), (54, 5), (48, 11),
         (33, 11), (33, 17), (16, 17), (16, 30), (0, 39)],
        fill=(10, 17, 30),
    )
    draw.line((5, 34, 13, 29, 13, 14, 31, 14, 31, 8, 47, 8, 53, 2, 65, 2),
              fill=(32, 48, 65))
    draw.line((22, 16, 22, 26, 39, 26), fill=(21, 37, 51))
    draw.rectangle((36, 24, 42, 27), fill=(30, 45, 61))
    draw.line((39, 27, 39, 34), fill=(33, 53, 66))
    draw.point((39, 35), fill=(77, 116, 133))
    for x in (19, 24, 29):
        draw.rectangle((x, 10, x + 1, 11), fill=(47, 83, 96))
    draw.line((51, 6, 74, 6), fill=(22, 34, 49))
    draw.rectangle((70, 5, 73, 7), fill=(39, 43, 65))

    # Hard-edged foreground plates are confined to the safe outer perimeter.
    for right in (False, True):
        def points(coords):
            return [(WIDTH - 1 - x if right else x, y) for x, y in coords]

        draw.polygon(
            points([(0, 41), (5, 46), (5, 111), (11, 122), (11, 197),
                    (7, 207), (7, 274), (25, 295), (25, 319), (0, 319)]),
            fill=(11, 17, 30),
        )
        draw.line(points([(4, 54), (4, 112), (10, 123), (10, 196),
                          (6, 207), (6, 275), (26, 299)]),
                  fill=(27, 39, 56))
        draw.line(points([(0, 118), (6, 125), (6, 190), (2, 200)]),
                  fill=(17, 27, 43))
        accent = (72, 38, 78) if right else (26, 70, 84)
        draw.line(points([(7, 137), (7, 157)]), fill=accent)
        draw.line(points([(7, 164), (7, 169)]), fill=accent)
        draw.line(points([(4, 239), (4, 257)]), fill=accent)
        for y in (128, 181, 215, 267):
            draw.point(points([(3, y)])[0], fill=(49, 59, 75))

    draw.polygon(
        [(0, 294), (21, 294), (39, 311), (92, 311), (99, 319), (0, 319)],
        fill=(17, 24, 39),
    )
    draw.polygon(
        [(239, 291), (222, 291), (200, 311), (148, 311), (140, 319), (239, 319)],
        fill=(17, 23, 38),
    )
    draw.line((0, 294, 21, 294, 39, 311, 92, 311), fill=(39, 53, 69))
    draw.line((239, 291, 222, 291, 200, 311, 148, 311), fill=(39, 47, 66))
    draw.line((42, 315, 88, 315), fill=(23, 47, 60))
    draw.line((154, 315, 197, 315), fill=(45, 29, 56))
    for x in (8, 14, 20):
        draw.line((x, 301, x + 5, 307), fill=(34, 45, 59))
    for x in (219, 225, 231):
        draw.line((x, 301, x - 5, 307), fill=(34, 41, 57))
    draw.line((28, 301, 35, 307), fill=(33, 91, 104))
    draw.line((215, 302, 209, 308), fill=(102, 47, 94))
    draw.rectangle((7, 313, 22, 316), fill=(6, 11, 21))
    draw.rectangle((216, 313, 231, 316), fill=(6, 11, 21))
    for x in (9, 13, 17):
        draw.point((x, 314), fill=(42, 103, 114))
    for x in (219, 223, 227):
        draw.point((x, 314), fill=(85, 42, 82))

    return image.resize((480, 640), Image.Resampling.NEAREST).convert("RGBA")


def png_bytes():
    buffer = BytesIO()
    render().save(buffer, format="PNG", optimize=False, compress_level=9)
    return buffer.getvalue()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT,
                        help="PNG destination (default: game/assets/sprites/ui/menu_backdrop.png)")
    parser.add_argument("--check", action="store_true",
                        help="verify that the destination matches, without writing anything")
    args = parser.parse_args()
    expected = png_bytes()
    if args.check:
        try:
            actual = args.output.read_bytes()
        except OSError as error:
            print(f"Cannot check {args.output}: {error}", file=sys.stderr)
            return 1
        if actual != expected:
            print(f"Backdrop is out of date: {args.output}", file=sys.stderr)
            return 1
        print(f"Backdrop matches deterministic output: {args.output}")
        return 0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(expected)
    print(f"Wrote 480x640 RGBA backdrop: {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
