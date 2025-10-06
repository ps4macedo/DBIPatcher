#!/usr/bin/env bash
set -euo pipefail

DBI_DIR="${1:-/tmp/DBI_821}"
PATCHER="${2:-./bin/dbipatcher}"

OUTDIR="debug_rec"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

echo "[*] Probing rec0..rec9 in: $DBI_DIR"
for i in $(seq 0 9); do
  IN="$DBI_DIR/rec${i}.bin"
  [ -f "$IN" ] || continue

  echo "[*] rec${i}"
  cp "$IN" "$OUTDIR/rec${i}.raw.bin" || true
  # raw strings (para referência)
  strings -a "$IN" | head -n 200 > "$OUTDIR/rec${i}.raw.strings.txt" || true

  # Tentativa de conversão via dbipatcher (apenas log)
  "$PATCHER" --convert "$IN" --output "$OUTDIR/rec${i}.ru.txt" > "$OUTDIR/rec${i}.convert.log" 2>&1 || true

  # zstd
  if zstd -q -d -c "$IN" > "$OUTDIR/rec${i}.zstd.bin" 2>/dev/null; then
    strings -a "$OUTDIR/rec${i}.zstd.bin" | head -n 200 > "$OUTDIR/rec${i}.zstd.strings.txt" || true
  else
    rm -f "$OUTDIR/rec${i}.zstd.bin"
  fi

  # gzip
  if gzip -q -d -c "$IN" > "$OUTDIR/rec${i}.gzip.bin" 2>/dev/null; then
    strings -a "$OUTDIR/rec${i}.gzip.bin" | head -n 200 > "$OUTDIR/rec${i}.gzip.strings.txt" || true
  else
    rm -f "$OUTDIR/rec${i}.gzip.bin"
  fi

  # lz4
  if lz4 -q -d -c "$IN" > "$OUTDIR/rec${i}.lz4.bin" 2>/dev/null; then
    strings -a "$OUTDIR/rec${i}.lz4.bin" | head -n 200 > "$OUTDIR/rec${i}.lz4.strings.txt" || true
  else
    rm -f "$OUTDIR/rec${i}.lz4.bin"
  fi

  # zlib
  python3 - "$IN" "$OUTDIR/rec${i}.zlib.bin" << 'PY' || true
import sys, zlib, pathlib
try:
    data = pathlib.Path(sys.argv[1]).read_bytes()
    out = zlib.decompress(data)
    pathlib.Path(sys.argv[2]).write_bytes(out)
    sys.exit(0)
except Exception:
    sys.exit(1)
PY
  if [ -f "$OUTDIR/rec${i}.zlib.bin" ]; then
    strings -a "$OUTDIR/rec${i}.zlib.bin" | head -n 200 > "$OUTDIR/rec${i}.zlib.strings.txt" || true
  fi
done

echo "[*] Probe finished. See $OUTDIR/ for results."
