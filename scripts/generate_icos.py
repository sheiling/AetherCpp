"""Generate short_logo.ico and app_splash.ico from logo-mark.svg.

Renders the Aether logo mark (teal 'A' shape) directly with Pillow
at multiple resolutions and saves as Windows ICO files.
"""

from PIL import Image, ImageDraw
import os

# Output paths (relative to project root)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
OUTPUT_DIR = os.path.join(PROJECT_ROOT, 'app', 'resources', 'image')

SVG_VIEWBOX = 64  # logo-mark.svg viewBox is 0 0 64 64
TEAL = (0, 188, 212, 255)

# SVG element parameters (in 64x64 viewBox coordinates)
# Path: M 10 56 L 32 10 L 54 56, stroke-width=12, round linecap/join
V_START = (10, 56)
V_APEX = (32, 10)
V_END = (54, 56)
V_STROKE_WIDTH = 12

# Line: x1=20 y1=36 x2=44 y2=36, stroke-width=8, butt linecap
H_START = (20, 36)
H_END = (44, 36)
H_STROKE_WIDTH = 8


def render_logo(size: int) -> Image.Image:
    """Render the Aether logo mark at the given size (width == height)."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    s = size / SVG_VIEWBOX  # scale factor

    def scale_point(pt):
        return (pt[0] * s, pt[1] * s)

    # --- V-shape (round caps, round joint) ---
    vw = max(1, int(V_STROKE_WIDTH * s))

    p1 = scale_point(V_START)
    p2 = scale_point(V_APEX)
    p3 = scale_point(V_END)

    # Draw both legs
    draw.line([p1, p2], fill=TEAL, width=vw)
    draw.line([p2, p3], fill=TEAL, width=vw)

    # Round joint at apex
    r = vw / 2.0
    draw.ellipse([p2[0]-r, p2[1]-r, p2[0]+r, p2[1]+r], fill=TEAL)
    # Round caps at endpoints
    draw.ellipse([p1[0]-r, p1[1]-r, p1[0]+r, p1[1]+r], fill=TEAL)
    draw.ellipse([p3[0]-r, p3[1]-r, p3[0]+r, p3[1]+r], fill=TEAL)

    # --- Horizontal bar (butt caps -> filled rectangle) ---
    hw = max(1, int(H_STROKE_WIDTH * s))
    hp1 = scale_point(H_START)
    hp2 = scale_point(H_END)

    draw.rectangle(
        [hp1[0], hp1[1] - hw/2.0, hp2[0], hp2[1] + hw/2.0],
        fill=TEAL
    )

    return img


def generate_ico(output_path: str, sizes=None):
    """Generate a multi-resolution ICO file from the logo mark SVG.

    Note: Pillow's ICO save uses the first image's dimensions as the cap;
    any size larger than the first image is skipped. So the first image
    must be the largest size (256x256).
    """
    if sizes is None:
        sizes = [(16, 16), (32, 32), (48, 48), (256, 256)]

    # Sort descending so the largest image is first
    sizes_sorted = sorted(sizes, reverse=True)
    images = [render_logo(w) for w, _ in sizes_sorted]

    images[0].save(
        output_path,
        format='ICO',
        sizes=sizes,
        append_images=images[1:]
    )
    print(f'  Created: {output_path} ({os.path.getsize(output_path)} bytes)')


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Source SVG path (for reference only — we render manually)
    svg_path = os.path.join(OUTPUT_DIR, 'logo-mark.svg')
    if not os.path.exists(svg_path):
        print(f'Warning: source SVG not found at {svg_path}')
    else:
        print(f'Source SVG: {svg_path}')

    sizes = [(16, 16), (32, 32), (48, 48), (256, 256)]

    print('\nGenerating short_logo.ico ...')
    generate_ico(os.path.join(OUTPUT_DIR, 'short_logo.ico'), sizes)

    print('\nGenerating app_splash.ico ...')
    generate_ico(os.path.join(OUTPUT_DIR, 'app_splash.ico'), sizes)

    print('\nDone.')


if __name__ == '__main__':
    main()
