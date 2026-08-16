"""
Generate white_logo.png and black_logo.png from the Aether SVG logo mark.

Reads app/resources/image/logo-mark.svg, renders as a grayscale alpha mask
at 4x resolution for anti-aliasing, then applies the desired stroke color
and composites onto a 120x37 RGBA transparent canvas (centered).
"""

import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw
import os

SVG_PATH = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "app", "resources", "image", "logo-mark.svg",
)
OUT_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "common", "ui", "resources", "image",
)
TARGET_W, TARGET_H = 120, 37
RENDER_SCALE = 4  # render internal mask at 4x for smooth anti-aliasing


# ---------------------------------------------------------------------------
# SVG parsing
# ---------------------------------------------------------------------------

def parse_svg(svg_path):
    """Parse the simple SVG and return (vb_w, vb_h, primitives)."""
    tree = ET.parse(svg_path)
    root = tree.getroot()
    ns = "{http://www.w3.org/2000/svg}"

    viewbox = root.get("viewBox", "0 0 64 64")
    vb = [int(x) for x in viewbox.split()]
    vb_w, vb_h = vb[2], vb[3]

    primitives = []
    for g in root.findall(f"{ns}g"):
        default_stroke = g.get("stroke", "#000000")
        for child in g:
            tag = child.tag.split("}")[-1] if "}" in child.tag else child.tag
            attrs = dict(child.attrib)
            attrs.setdefault("stroke", default_stroke)
            primitives.append((tag, attrs))

    return vb_w, vb_h, primitives


# ---------------------------------------------------------------------------
# Mask rendering (white-on-black grayscale, no color blending)
# ---------------------------------------------------------------------------

def render_alpha_mask(vb_w, vb_h, primitives, scale=1):
    """
    Render the logo shape as a grayscale mask at `scale` resolution.

    Returns a PIL 'L' (grayscale) image where 255 = fully opaque logo
    and 0 = fully transparent background. This cleanly separates the
    anti-aliased shape from the final colour, avoiding dark fringing
    that occurs when down-sampling coloured-on-transparent images.
    """
    size = (vb_w * scale, vb_h * scale)
    mask = Image.new("L", size, 0)        # black (transparent) background
    draw = ImageDraw.Draw(mask)

    for tag, attrs in primitives:
        sw = int(float(attrs.get("stroke-width", 1)) * scale)
        cap = attrs.get("stroke-linecap", "butt")
        join = attrs.get("stroke-linejoin", "miter")

        if tag == "path":
            d = attrs.get("d", "")
            # parse simple M/L path commands
            parts = d.replace(",", " ").split()
            points = []
            cmd = None
            i = 0
            while i < len(parts):
                token = parts[i]
                if token in ("M", "L", "Z", "z"):
                    cmd = token
                    i += 1
                elif cmd in ("M", "L"):
                    x = float(parts[i]) * scale
                    y = float(parts[i + 1]) * scale
                    points.append((x, y))
                    i += 2
                else:
                    i += 1

            if len(points) >= 2:
                for j in range(len(points) - 1):
                    draw.line(
                        [points[j], points[j + 1]],
                        fill=255, width=sw,
                    )
                # Round linecap & linejoin via circles at every vertex
                r = sw // 2
                for pt in points:
                    draw.ellipse(
                        [pt[0] - r, pt[1] - r, pt[0] + r, pt[1] + r],
                        fill=255,
                    )

        elif tag == "line":
            x1 = float(attrs["x1"]) * scale
            y1 = float(attrs["y1"]) * scale
            x2 = float(attrs["x2"]) * scale
            y2 = float(attrs["y2"]) * scale
            w = int(float(attrs.get("stroke-width", 1)) * scale)

            draw.line([(x1, y1), (x2, y2)], fill=255, width=w)

            # Round linecap simulation
            if cap == "round":
                r = w // 2
                for pt in [(x1, y1), (x2, y2)]:
                    draw.ellipse(
                        [pt[0] - r, pt[1] - r, pt[0] + r, pt[1] + r],
                        fill=255,
                    )

    return mask


# ---------------------------------------------------------------------------
# Content bounding box (grayscale version)
# ---------------------------------------------------------------------------

def find_content_bbox_gray(img, threshold=1):
    """Return (left, upper, right, lower) bbox of pixels > threshold."""
    w, h = img.size
    xs, ys = [], []
    for y in range(h):
        for x in range(w):
            if img.getpixel((x, y)) > threshold:
                xs.append(x)
                ys.append(y)
    if not xs:
        return (0, 0, w, h)
    return (min(xs), min(ys), max(xs) + 1, max(ys) + 1)


# ---------------------------------------------------------------------------
# PNG generation
# ---------------------------------------------------------------------------

def generate_logo(svg_path, out_path, color_rgb, target_size=(120, 37)):
    """Render SVG logo with given stroke colour and save as RGBA PNG."""
    vb_w, vb_h, primitives = parse_svg(svg_path)

    # 1) Render alpha mask at high resolution
    mask_hr = render_alpha_mask(vb_w, vb_h, primitives, scale=RENDER_SCALE)

    # 2) Crop to content
    bbox = find_content_bbox_gray(mask_hr)
    content = mask_hr.crop(bbox)
    cw, ch = content.size

    # 3) Scale to fit target while keeping aspect ratio
    target_w, target_h = target_size
    fit = min(target_w / cw, target_h / ch)
    new_w = max(1, int(round(cw * fit)))
    new_h = max(1, int(round(ch * fit)))
    alpha = content.resize((new_w, new_h), Image.LANCZOS)   # 'L' image

    # 4) Build RGBA: use colour_rgb for every pixel, alpha from the mask
    rgba = Image.new("RGBA", (new_w, new_h), (0, 0, 0, 0))
    r, g, b = color_rgb
    for y in range(new_h):
        for x in range(new_w):
            a = alpha.getpixel((x, y))
            if a > 0:
                rgba.putpixel((x, y), (r, g, b, a))

    # 5) Centre on the output canvas
    final = Image.new("RGBA", target_size, (0, 0, 0, 0))
    x_off = (target_w - new_w) // 2
    y_off = (target_h - new_h) // 2
    # NOTE: paste WITHOUT mask — rgba already carries its own alpha channel.
    # Passing the image as mask would composite it against the transparent
    # background and darken the edge pixels.
    final.paste(rgba, (x_off, y_off))

    final.save(out_path, "PNG")
    print(f"  -> {out_path}  ({final.size}, {os.path.getsize(out_path)} bytes)")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    print(f"SVG:  {SVG_PATH}")
    print(f"Out:  {OUT_DIR}")
    print(f"Size: {TARGET_W}x{TARGET_H}px")
    print()

    print("white_logo.png  (colour #FFFFFF) ...")
    generate_logo(
        SVG_PATH,
        os.path.join(OUT_DIR, "white_logo.png"),
        (255, 255, 255),
        (TARGET_W, TARGET_H),
    )

    print("black_logo.png  (colour #212121) ...")
    generate_logo(
        SVG_PATH,
        os.path.join(OUT_DIR, "black_logo.png"),
        (33, 33, 33),          # #212121
        (TARGET_W, TARGET_H),
    )

    print()
    print("Done.")


if __name__ == "__main__":
    main()
