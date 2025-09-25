#!/usr/bin/env python3
"""Helper to build DBI translations locally.

This script mirrors the behaviour used in CI and loops over translation
text files to produce patched NROs in ``out/dbi`` (or a custom
``OUT_DIR``). It is invoked from ``make translate`` but can also be used
manually.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List
import shutil
from datetime import datetime

DEFAULT_LOG_DIR = Path("/tmp")


def env_value(name: str, *, required: bool = True) -> str:
    value = os.environ.get(name)
    if value:
        return value
    if required:
        raise SystemExit(f"{name} environment variable is required")
    return ""


def discover_languages() -> List[str]:
    translations = Path("translate")
    langs = {p.stem.split(".")[-1] for p in translations.glob("rec6.*.txt")}
    if not langs:
        raise SystemExit("No translation files found in translate/rec6.*.txt")
    return sorted(langs)


def run_make(lang: str, make_cmd: str, log_path: Path) -> None:
    env = os.environ.copy()
    env["DBI_TARGET"] = lang
    cmd = [make_cmd, "--no-print-directory", "translate-core"]
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=env) as proc, log_path.open("w", encoding="utf-8", errors="replace") as log_file:
        assert proc.stdout is not None
        for line in proc.stdout:
            sys.stdout.write(line)
            log_file.write(line)
        ret = proc.wait()
        if ret:
            raise subprocess.CalledProcessError(ret, cmd)


def copy_artifact(tmpdir: Path, out_dir: Path, ver: str, lang: str) -> Path:
    src = tmpdir / "bin" / "DBI.nro"
    if not src.is_file():
        raise SystemExit(f"Expected artifact not found: {src}")
    dest = out_dir / f"DBI.{ver}.{lang}.nro"
    out_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dest)
    return dest


def copy_base_artifact(base_file: Path, out_dir: Path, ver: str, lang: str) -> Path:
    if not base_file.is_file():
        raise SystemExit(f"Base DBI file not found: {base_file}")
    dest = out_dir / f"DBI.{ver}.{lang}.nro"
    out_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(base_file, dest)
    return dest


def build_languages(langs: Iterable[str]) -> None:
    ver = env_value("DBI_VER")
    base_lang = env_value("DBI_LANG")
    base_file = Path(env_value("DBI_BASE"))
    tmpdir = Path(env_value("DBI_TMPDIR"))
    out_dir = Path(os.environ.get("OUT_DIR", "out/dbi"))
    make_cmd = os.environ.get("MAKE", "make")
    log_dir = Path(os.environ.get("LOG_DIR", str(DEFAULT_LOG_DIR)))

    langs = list(langs)
    if not langs:
        print("No languages requested", file=sys.stderr)
        return

    for lang in langs:
        lang_norm = lang.strip()
        if not lang_norm:
            continue
        print(f"\n=== Building {lang_norm} ===")
        log_path = log_dir / f"dbi_translate_{ver}_{lang_norm}.log"
        if lang_norm == base_lang:
            dest = copy_base_artifact(base_file, out_dir, ver, lang_norm)
            log_path.write_text(f"{datetime.now():%H:%M:%S}  [N] reused original {base_file}\n")
            print(f"Copied base artifact to {dest}")
            continue

        run_make(lang_norm, make_cmd, log_path)
        dest = copy_artifact(tmpdir, out_dir, ver, lang_norm)
        print(f"Stored {dest}")


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lang", action="append", help="Build only the specified language (can be repeated)")
    parser.add_argument("--all", action="store_true", help="Build every language found in translate/rec6.*.txt (default)")
    return parser.parse_args(argv)


def main(argv: List[str]) -> int:
    args = parse_args(argv)

    if args.lang:
        langs = args.lang
    else:
        langs = discover_languages() if (args.all or not args.lang) else []

    try:
        build_languages(langs)
    except subprocess.CalledProcessError as exc:
        print(f"translate-core failed with exit code {exc.returncode}", file=sys.stderr)
        return exc.returncode
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
