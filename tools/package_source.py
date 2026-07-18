#!/usr/bin/env python3
"""Build a deterministic, dependency-free source archive."""

from __future__ import annotations

import hashlib
import importlib.util
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT / "dist"
FIXED_TIMESTAMP = (2026, 7, 18, 0, 0, 0)
EXCLUDED_DIRS = {
    ".git", ".vs", ".vscode", ".idea", "build", "dist", "out",
    "cmake-build-debug", "cmake-build-release", "vcpkg_installed", "__pycache__",
}
EXCLUDED_SUFFIXES = {
    ".dll", ".pdb", ".obj", ".lib", ".exp", ".ilk", ".pch", ".tlog",
    ".zip", ".7z", ".pyc", ".log",
}


def release_module():
    spec = importlib.util.spec_from_file_location("package_release", ROOT / "tools/package_release.py")
    if not spec or not spec.loader:
        raise RuntimeError("Could not import package_release.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def included_files() -> list[Path]:
    files: list[Path] = []
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file():
            continue
        rel = path.relative_to(ROOT)
        if any(part in EXCLUDED_DIRS for part in rel.parts):
            continue
        if rel.parts and rel.parts[0] == "extern" and rel != Path("extern/.gitkeep"):
            continue
        if path.suffix.lower() in EXCLUDED_SUFFIXES:
            continue
        files.append(rel)
    return files


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    version = release_module().read_version()
    DIST.mkdir(parents=True, exist_ok=True)
    output = DIST / f"Modern NPC Pathing {version} Source.zip"
    output.unlink(missing_ok=True)

    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for rel in included_files():
            info = zipfile.ZipInfo(rel.as_posix(), FIXED_TIMESTAMP)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, (ROOT / rel).read_bytes())

    digest = sha256(output)
    print(f"Built {output} ({output.stat().st_size:,} bytes)")
    print(f"SHA256 {digest}")


if __name__ == "__main__":
    main()
