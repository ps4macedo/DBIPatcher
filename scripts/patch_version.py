#!/usr/bin/env python3
"""Patch the 20-byte DBI version string based on target language.

Matches the behaviour documented in README and used in CI so that
local builds and GitHub Actions produce identical metadata.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

SIGNATURE = b"{}-{} FW:{}{} SDK:{}"

LANG_MAP = {
    "KR": {"ko", "kr", "kor", "korea", "korean"},
    "JP": {"ja", "jp", "jpn", "japan", "japanese"},
    "ZHCN": {"zhcn", "zh-cn", "cn", "chs", "zhhans", "hans"},
    "ZHTW": {"zhtw", "zh-tw", "tw", "cht", "zhhant", "hant"},
    "EN": {"en", "us", "en-us", "enus", "american"},
    "ENGB": {"uk", "gb", "engb", "en-gb", "british"},
    "FR": {"fr", "french", "france"},
    "FRCA": {"frca", "fr-ca", "canadianfrench"},
    "DE": {"de", "german", "germany", "deu", "ger"},
    "IT": {"it", "italian", "italy", "ita"},
    "NL": {"nl", "dutch", "netherlands", "holland", "nld"},
    "ES": {"es", "spanish", "spain", "espanol", "castellano", "spa"},
    "ES419": {"es419", "es-419", "latamspanish"},
    "PT": {"pt", "portuguese", "portugal", "por"},
    "PTBR": {"ptbr", "pt-br", "brazil", "brazilian", "br"},
    "PL": {"pl", "polish", "poland", "pol"},
    "RU": {"ru", "russian", "russia", "rus"},
    "UA": {"ua", "ukr", "uk-ua", "ukraine"},
}


def resolve_tag(value: str) -> tuple[str, bool]:
    low = value.lower()
    for tag, aliases in LANG_MAP.items():
        if low in aliases:
            return tag, False
    return value, True


def build_replacement(tag: str, fallback: bool) -> bytes:
    if fallback:
        trimmed = tag[:4]
        base = f"{{0}}-{trimmed} FW: {{2}}-{{3}}"
        base = base[:20]
        return base.encode("ascii", "strict").ljust(20, b"\x00")

    length = len(tag)
    if length == 2:
        repl = f"{{0}}-{tag}  FW: {{2}}-{{3}}\x00"
    elif length == 4:
        repl = f"{{0}}-{tag} FW: {{2}}-{{3}}"
    elif length == 5:
        repl = f"{{0}}-{tag} FW:{{2}}-{{3}}"
    else:
        raise ValueError(f"Unsupported tag length {length} for '{tag}'")

    repl_bytes = repl.encode("ascii", "strict")
    if len(repl_bytes) != 20:
        raise ValueError(f"Replacement is {len(repl_bytes)} bytes, expected 20")
    return repl_bytes


def patch_version(path: Path, lang: str) -> None:
    if not path.is_file():
        raise FileNotFoundError(f"File not found: {path}")

    data = path.read_bytes()
    offset = data.find(SIGNATURE)
    if offset == -1:
        raise RuntimeError("Version signature not found in binary")

    tag, fallback = resolve_tag(lang)
    repl = build_replacement(tag, fallback)

    new_data = data[:offset] + repl + data[offset + len(SIGNATURE):]
    path.write_bytes(new_data)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--file", required=True, type=Path, help="Path to DBI NRO")
    parser.add_argument("--lang", required=True, help="Target language code")
    args = parser.parse_args(argv)

    try:
        patch_version(args.file, args.lang)
    except Exception as exc:  # pylint: disable=broad-except
        print(f"patch_version: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
