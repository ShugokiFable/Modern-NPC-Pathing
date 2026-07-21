from __future__ import annotations

import ast
import importlib.util
import json
import re
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION = "2.4.4"
EXPECTED_LOCAL_IDS = {
    0x800, 0x801, 0x802, 0x803, 0x804, 0x805, 0x806, 0x807,
    0x808, 0x809, 0x80A, 0x80B, 0x80C, 0x80D, 0x80E, 0x80F,
    0x810, 0x811, 0x812,
}


def read_subrecord(data: bytes, offset: int) -> tuple[bytes, bytes, int]:
    tag = data[offset:offset + 4]
    size = struct.unpack_from("<H", data, offset + 4)[0]
    start = offset + 6
    return tag, data[start:start + size], start + size


class ReleaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.tmp = tempfile.TemporaryDirectory()
        cls.esp = Path(cls.tmp.name) / "NPCPathingNG.esp"
        subprocess.run(["python", str(ROOT / "generate_esp.py"), str(cls.esp)], check=True, cwd=ROOT)
        cls.data = cls.esp.read_bytes()

    @classmethod
    def tearDownClass(cls) -> None:
        cls.tmp.cleanup()

    def test_version_metadata_is_synchronized(self) -> None:
        self.assertIn("VERSION 2.4.4", (ROOT / "CMakeLists.txt").read_text())
        self.assertEqual(json.loads((ROOT / "vcpkg.json").read_text())["version-semver"], VERSION)
        version_header = (ROOT / "src/version.h").read_text()
        self.assertIn('String = "2.4.4"', version_header)
        self.assertIn('(static_cast<std::uint32_t>(Major) << 24)', version_header)
        self.assertTrue((ROOT / "README.md").read_text().startswith("# NPC Pathing NG v2.4.4"))
        self.assertTrue((ROOT / "package/README.md").read_text().startswith("# NPC Pathing NG v2.4.4"))

    def test_esp_header_and_form_ids(self) -> None:
        self.assertEqual(self.data[:4], b"TES4")
        record_size, flags = struct.unpack_from("<II", self.data, 4)
        self.assertEqual(flags & 0x200, 0x200, "ESP must remain ESL-flagged")
        tag, payload, _ = read_subrecord(self.data, 24)
        self.assertEqual(tag, b"HEDR")
        version, record_count, next_object_id = struct.unpack("<fII", payload)
        self.assertAlmostEqual(version, 1.71, places=5)
        self.assertEqual(record_count, 19)
        self.assertEqual(next_object_id, 0x813, "HEDR next object ID must be local, not load-order-prefixed")
        self.assertEqual(record_size + 24, self.data.find(b"GRUP"))

        form_ids: set[int] = set()
        offset = self.data.find(b"GRUP")
        while offset < len(self.data):
            self.assertEqual(self.data[offset:offset + 4], b"GRUP")
            group_size = struct.unpack_from("<I", self.data, offset + 4)[0]
            pos = offset + 24
            end = offset + group_size
            while pos < end:
                size = struct.unpack_from("<I", self.data, pos + 4)[0]
                form_ids.add(struct.unpack_from("<I", self.data, pos + 12)[0] & 0x00FFFFFF)
                pos += 24 + size
            self.assertEqual(pos, end)
            offset = end
        self.assertEqual(form_ids, EXPECTED_LOCAL_IDS)

    def test_seq_points_to_start_game_quest(self) -> None:
        seq = self.esp.parent / "Seq/NPCPathingNG.seq"
        self.assertEqual(struct.unpack("<I", seq.read_bytes())[0], 0x01000810)

    def test_mcm_global_references_exist(self) -> None:
        config = json.loads((ROOT / "package/Data/MCM/Config/NPCPathingNG/config.json").read_text())
        refs: set[int] = set()
        for page in config["pages"]:
            for item in page.get("content", []):
                source = item.get("valueOptions", {}).get("sourceForm")
                if source:
                    refs.add(int(source.split("|")[1], 16))
        self.assertTrue(refs)
        self.assertTrue(refs.issubset(EXPECTED_LOCAL_IDS))
        self.assertNotIn(0x810, refs, "The quest is not a GlobalValue")

    def test_regressions_are_guarded_in_source(self) -> None:
        pathing = (ROOT / "src/pathing.cpp").read_text()
        parkour = (ROOT / "src/npc_parkour.cpp").read_text()
        self.assertIn("CancelParkourJobs(true)", pathing)
        self.assertIn("(settings->enableParkour || settings->enableEvgTraversal)", pathing)
        self.assertIn("(isEvg && !settings->enableEvgTraversal)", pathing)
        self.assertIn("it = playerEvents.erase(it)", pathing)
        self.assertIn("OnParkourEnd(actor, timedOut)", pathing)
        self.assertIn("OnParkourEnd(actor, true);", pathing)
        self.assertIn("TryTeleportBypass(a_actor))", pathing)
        self.assertIn("pathBlocked", pathing)
        self.assertIn("destinationBlocked", pathing)
        self.assertIn("IsActorHit(ground)", pathing)
        self.assertGreaterEqual(parkour.count("SetGraphVariableBool(SkyParkourGraph::VarLowerBody, false)"), 2)

    def test_shipping_dll_has_required_skse_exports(self) -> None:
        dll_path = ROOT / "package/Data/SKSE/Plugins/NPCPathingNG.dll"
        if not dll_path.exists():
            self.skipTest("DLL is produced by the Windows release build and is not committed")
        spec = importlib.util.spec_from_file_location("package_release", ROOT / "tools/package_release.py")
        self.assertIsNotNone(spec)
        self.assertIsNotNone(spec.loader)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        dll = dll_path.read_bytes()
        self.assertTrue({"SKSEPlugin_Load", "SKSEPlugin_Query", "SKSEPlugin_Version"}.issubset(module.read_pe_exports(dll)))

    def test_settings_bindings_match_generated_globals(self) -> None:
        generator = ast.parse((ROOT / "generate_esp.py").read_text())
        globs = None
        for node in generator.body:
            if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == "GLOBS" for t in node.targets):
                globs = ast.literal_eval(node.value)
                break
        self.assertIsNotNone(globs)
        generated_ids = {row[0] for row in globs}
        settings = (ROOT / "src/settings.cpp").read_text()
        bound_ids = {int(value, 16) for value in re.findall(r"= lookup\((0x[0-9A-Fa-f]+)\);", settings)}
        self.assertEqual(bound_ids, generated_ids)

    def test_build_presets_and_ci_are_portable(self) -> None:
        presets = json.loads((ROOT / "CMakePresets.json").read_text())
        configure = presets["configurePresets"][0]
        self.assertEqual(configure["generator"], "Visual Studio 17 2022")
        workflow = (ROOT / ".github/workflows/build.yml").read_text()
        self.assertIn("actions/setup-python@v5", workflow)
        self.assertIn("VCPKG_INSTALLATION_ROOT", workflow)
        self.assertIn("VCPKG_ROOT=$root", workflow)

    def test_documented_default_matches_runtime(self) -> None:
        readme = (ROOT / "README.md").read_text()
        self.assertIn("climb height **130**", readme)
        self.assertNotIn("climb height **250** (max)", readme)


if __name__ == "__main__":
    unittest.main()
