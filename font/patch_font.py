#!/usr/bin/env python3
import sys, pathlib
from zstandard import ZstdCompressor

def main(nro_in, raw_font_bin, nro_out, hex_offset, max_size):
    off = int(hex_offset, 16)
    limit = int(max_size)

    nro = bytearray(pathlib.Path(nro_in).read_bytes())
    raw = pathlib.Path(raw_font_bin).read_bytes()

    comp = ZstdCompressor(level=22).compress(raw)  # match original “max-ish” level
    if len(comp) > limit:
        print(f"ERROR: compressed {len(comp)} > slot limit {limit}")
        sys.exit(1)

    nro[off:off+len(comp)] = comp
    # Optional cosmetic zeroing of the rest of the slot:
    # nro[off+len(comp):off+limit] = b"\x00" * (limit - len(comp))

    pathlib.Path(nro_out).write_bytes(nro)
    print(f"Patched {nro_out}: wrote {len(comp)} bytes at 0x{off:X} (limit {limit})")

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: patch_font.py DBI.nro font_2MiB.bin OUT.nro 0x7555E0 596378")
        sys.exit(1)
    main(*sys.argv[1:])