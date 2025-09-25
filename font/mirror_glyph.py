#!/usr/bin/env python3
# Font format: 65536 glyphs * 32 bytes; each row is 2 bytes LE; bit0 = leftmost.
#  э (044D) → є (0454): Mirrors Russian "э" to create Ukrainian "є"
#  Э (042D) → Є (0404): Mirrors Russian "Э" to create Ukrainian "Є"
#  I/i → І/і: Copies Latin I/i to Ukrainian І/і
#  Ï/ï → Ї/ї: Copies Latin Ï/ï to Ukrainian Ї/ї
#  Optionally clears unused glyphs (PUA) to save space

import argparse
from pathlib import Path

GLYPH_W = 16
GLYPH_H = 16
BYTES_PER_ROW = 2
BYTES_PER_GLYPH = GLYPH_H * BYTES_PER_ROW          # 32
TOTAL_GLYPHS = 0x10000
EXPECTED_SIZE = TOTAL_GLYPHS * BYTES_PER_GLYPH      # 2,097,152

def load_font(p: Path) -> bytearray:
    data = bytearray(p.read_bytes())
    if len(data) != EXPECTED_SIZE:
        raise SystemExit(f"{p}: size {len(data)} != expected {EXPECTED_SIZE} "
                         f"(did you build with --swap-bytes --bit-lsb-first?)")
    return data

def glyph_offset(cp: int) -> int:
    if not (0 <= cp < TOTAL_GLYPHS):
        raise ValueError("codepoint out of BMP")
    return cp * BYTES_PER_GLYPH

def get_glyph_bitmap(font: bytearray, cp: int):
    """Return 16x16 bool matrix; each row is LE 16-bit, LSB=leftmost pixel."""
    off = glyph_offset(cp)
    g = memoryview(font)[off:off+BYTES_PER_GLYPH]
    bm = []
    for r in range(GLYPH_H):
        lo = g[r*2 + 0]
        hi = g[r*2 + 1]
        row16 = lo | (hi << 8)
        bm.append([bool((row16 >> x) & 1) for x in range(GLYPH_W)])  # x=0 left
    return bm

def put_glyph_bitmap(font: bytearray, cp: int, bm):
    off = glyph_offset(cp)
    g = memoryview(font)[off:off+BYTES_PER_GLYPH]
    for r in range(GLYPH_H):
        row = bm[r]
        row16 = 0
        for x in range(GLYPH_W):
            if row[x]:
                row16 |= (1 << x)  # LSB = leftmost
        g[r*2 + 0] = row16 & 0xFF
        g[r*2 + 1] = (row16 >> 8) & 0xFF

def mirror_horiz(bm):
    return [list(reversed(row)) for row in bm]

def preview(cp, bm):
    print(f"U+{cp:04X}:")
    for r in bm:
        print("".join("█" if p else "·" for p in r))
    print()

def copy_mirrored(font, src_cp, dst_cp, show=False):
    src = get_glyph_bitmap(font, src_cp)
    dst = mirror_horiz(src)
    put_glyph_bitmap(font, dst_cp, dst)
    if show:
        print(f"Copied mirrored U+{src_cp:04X} → U+{dst_cp:04X}")
        preview(src_cp, src)
        preview(dst_cp, dst)

def copy_glyph(font, src_cp, dst_cp, show=False):
    bm = get_glyph_bitmap(font, src_cp)
    put_glyph_bitmap(font, dst_cp, bm)
    if show:
        print(f"Copied U+{src_cp:04X} → U+{dst_cp:04X}")
        preview(src_cp, bm)
        preview(dst_cp, bm)

def zap(font: bytearray, lo, hi):
    """Zero-out glyphs in [lo, hi] inclusive (helps compression if unused)."""
    for cp in range(lo, hi + 1):
        off = glyph_offset(cp)
        font[off:off + BYTES_PER_GLYPH] = b"\x00" * BYTES_PER_GLYPH

def main():
    ap = argparse.ArgumentParser(description="Mirror/copy glyphs for Ukrainian in DBI 16x16 font bin")
    ap.add_argument("in_font", help="input font bin (2,097,152 bytes)")
    ap.add_argument("out_font", help="output font bin")
    ap.add_argument("--preview", action="store_true", help="print glyph previews")
    ap.add_argument("--no-zap", action="store_true", help="do not zero out PUA")
    args = ap.parse_args()

    font = load_font(Path(args.in_font))

    # э (044D) -> є (0454)
    copy_mirrored(font, 0x044D, 0x0454, show=args.preview)
    # Э (042D) -> Є (0404)
    copy_mirrored(font, 0x042D, 0x0404, show=args.preview)
    # Latin I/i & Ï/ï → Cyrillic І/і & Ї/ї
    copy_glyph(font, 0x0049, 0x0406, show=args.preview)  # I  -> І
    copy_glyph(font, 0x0069, 0x0456, show=args.preview)  # i  -> і
    copy_glyph(font, 0x00CF, 0x0407, show=args.preview)  # Ï  -> Ї
    copy_glyph(font, 0x00EF, 0x0457, show=args.preview)  # ï  -> ї

    # Optional: shrink by clearing Private Use Area (only if truly unused)
    if not args.no_zap:
        zap(font, 0xE000, 0xF8FF)

    Path(args.out_font).write_bytes(font)
    print(f"Wrote {args.out_font}")

if __name__ == "__main__":
    main()

