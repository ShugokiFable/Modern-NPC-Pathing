#!/usr/bin/env python3
"""Assemble the FOMOD installer from the canonical package/ tree.

Only fomod_src/fomod/*.xml is authored and committed. Everything the installer
ships is copied from package/Data at build time, so the installer can never
drift from the validated plugin, SEQ, MCM config or INI.
"""
from __future__ import annotations

import hashlib
import re
import shutil
import sys
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PACKAGE = ROOT / "package" / "Data"
AUTHORED = ROOT / "fomod_src" / "fomod"
STAGING = ROOT / "dist" / "fomod_staging"
DIST = ROOT / "dist"


def read_version() -> str:
    text = (ROOT / "src" / "version.h").read_text(encoding="utf-8")
    match = re.search(r'String\s*=\s*"([0-9]+\.[0-9]+\.[0-9]+)"', text)
    if not match:
        raise RuntimeError("Could not read semantic version from src/version.h")
    return match.group(1)


def build_staging() -> None:
    if STAGING.exists():
        shutil.rmtree(STAGING)
    (STAGING / "fomod").mkdir(parents=True)

    for xml in AUTHORED.glob("*.xml"):
        shutil.copy2(xml, STAGING / "fomod" / xml.name)

    # core: everything except the INI, which is the variant-selected part.
    core = STAGING / "core"
    for rel in [
        "NPCPathingNG.esp",
        "Seq/NPCPathingNG.seq",
        "SKSE/Plugins/NPCPathingNG.dll",
        "MCM/Config/NPCPathingNG/config.json",
    ]:
        src = PACKAGE / rel
        if not src.is_file():
            raise RuntimeError(f"missing required package file: {rel}")
        dst = core / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)

    # INI variants, derived from the shipping INI so comments stay in sync.
    base = (PACKAGE / "SKSE/Plugins/NPCPathingNG.ini").read_text(encoding="utf-8")
    variants = {
        "ini_recommended": base,
        "ini_evg": base.replace("bEnableEVGTraversal=0", "bEnableEVGTraversal=1"),
        "ini_failsafe": base.replace("bEnableParkour=1", "bEnableParkour=0"),
    }
    for name, text in variants.items():
        dst = STAGING / name / "SKSE" / "Plugins" / "NPCPathingNG.ini"
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_text(text, encoding="utf-8", newline="\r\n")

    # Every variant must actually differ in the way its name claims.
    def flags(name: str) -> tuple[str, str]:
        t = (STAGING / name / "SKSE/Plugins/NPCPathingNG.ini").read_text(encoding="utf-8")
        parkour = re.search(r"^bEnableParkour=(\d)", t, re.M).group(1)
        evg = re.search(r"^bEnableEVGTraversal=(\d)", t, re.M).group(1)
        return parkour, evg

    expected = {"ini_recommended": ("1", "0"), "ini_evg": ("1", "1"), "ini_failsafe": ("0", "0")}
    for name, want in expected.items():
        got = flags(name)
        if got != want:
            raise RuntimeError(f"{name} INI flags {got} != expected {want}")


def validate() -> None:
    cfg = STAGING / "fomod" / "ModuleConfig.xml"
    tree = ET.parse(cfg)
    for elem in tree.iter():
        src = elem.attrib.get("source")
        if src and not (STAGING / src).exists():
            raise RuntimeError(f"ModuleConfig references missing source: {src}")
    # Every flag an option sets must have a matching install rule.
    root = tree.getroot()
    set_flags = {p.find("conditionFlags/flag").text for p in root.iter("plugin")}
    install_flags = {
        pat.find("dependencies/flagDependency").get("value")
        for pat in root.find("conditionalFileInstalls/patterns")
    }
    if set_flags != install_flags:
        raise RuntimeError(f"flag mismatch: options {set_flags} vs installs {install_flags}")


def write_zip(version: str) -> Path:
    out = DIST / f"NPC Pathing NG {version} FOMOD.zip"
    files = sorted(p for p in STAGING.rglob("*") if p.is_file())
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for f in files:
            z.write(f, f.relative_to(STAGING).as_posix())

    names = zipfile.ZipFile(out).namelist()
    if "fomod/ModuleConfig.xml" not in names:
        raise RuntimeError("fomod/ must sit at the archive root")
    if any(n.startswith("Data/") or "/Data/" in n for n in names):
        raise RuntimeError("nested Data/ folder in archive")
    if [n for n in names if n.endswith(".dll")] != ["core/SKSE/Plugins/NPCPathingNG.dll"]:
        raise RuntimeError("unexpected DLL layout in archive")
    return out


def main() -> None:
    version = read_version()
    build_staging()
    validate()
    out = write_zip(version)
    digest = hashlib.sha256(out.read_bytes()).hexdigest()
    print(f"Built {out} ({out.stat().st_size:,} bytes)")
    print(f"SHA256 {digest}")


if __name__ == "__main__":
    sys.exit(main())
