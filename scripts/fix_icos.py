"""Regenerate ICO files with proper multi-resolution support."""
import struct
import sys
import os
from PIL import Image

# Add project root to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from scripts.generate_icos import render_logo

SIZES = [(16, 16), (32, 32), (48, 48), (256, 256)]
OUTPUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'app', 'resources', 'image')


def create_ico(images, sizes, output_path):
    """Create ICO file with multiple embedded frames."""
    num_images = len(images)
    header_size = 6 + 16 * num_images
    offset = header_size

    directory_entries = []
    icon_data_list = []

    for img, (w, h) in zip(images, sizes):
        img_rgba = img.convert('RGBA')
        pixels = list(img_rgba.getdata())

        # BITMAPINFOHEADER (40 bytes)
        # biHeight = h * 2 for ICO: XOR mask + AND mask
        bih = struct.pack('<IiiHHIIiiII',
            40,          # biSize
            w,           # biWidth
            h * 2,       # biHeight (XOR + AND)
            1,           # biPlanes
            32,          # biBitCount
            0,           # biCompression (BI_RGB)
            0,           # biSizeImage
            0, 0, 0, 0   # biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImportant
        )

        # XOR mask: BGRA pixels, bottom-up rows, padded to 4 bytes
        xor_data = b''
        for y in range(h - 1, -1, -1):
            row = b''
            for x in range(w):
                r, g, b, a = pixels[y * w + x]
                row += struct.pack('BBBB', b, g, r, a)
            while len(row) % 4:
                row += b'\x00'
            xor_data += row

        # AND mask: 1-bit transparency, bottom-up, padded
        and_data = b''
        for y in range(h - 1, -1, -1):
            acc = 0
            bit = 0
            for x in range(w):
                _, _, _, a = pixels[y * w + x]
                if a < 128:
                    acc |= (1 << (7 - bit))
                bit += 1
                if bit == 8:
                    and_data += struct.pack('B', acc)
                    acc = 0
                    bit = 0
            if bit > 0:
                and_data += struct.pack('B', acc)
            while len(and_data) % 4:
                and_data += b'\x00'

        image_dib = bih + xor_data + and_data

        # Directory entry (0 means 256 for width/height)
        icon_w = 0 if w >= 256 else w
        icon_h = 0 if h >= 256 else h
        entry = struct.pack('<BBBBHHII',
            icon_w, icon_h,
            0,    # bColorCount
            0,    # bReserved
            1,    # wPlanes
            32,   # wBitCount
            len(image_dib),
            offset
        )
        directory_entries.append(entry)
        icon_data_list.append(image_dib)
        offset += len(image_dib)

    with open(output_path, 'wb') as f:
        f.write(struct.pack('<HHH', 0, 1, num_images))  # ICONDIR
        for entry in directory_entries:
            f.write(entry)
        for data in icon_data_list:
            f.write(data)

    # Verify
    verify = Image.open(output_path)
    nf = getattr(verify, 'n_frames', 1)
    print(f'  {output_path}: {nf} frames, {os.path.getsize(output_path)} bytes')
    for i in range(nf):
        verify.seek(i)
        print(f'    Frame {i}: {verify.size}')


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    images = [render_logo(w) for w, _h in SIZES]

    print('Regenerating short_logo.ico...')
    create_ico(images, SIZES, os.path.join(OUTPUT_DIR, 'short_logo.ico'))

    print('Regenerating app_splash.ico...')
    create_ico(images, SIZES, os.path.join(OUTPUT_DIR, 'app_splash.ico'))

    print('Done.')


if __name__ == '__main__':
    main()
