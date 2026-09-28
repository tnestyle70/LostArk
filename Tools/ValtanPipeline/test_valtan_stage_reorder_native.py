"""Run the production Earlier/Later selection operation with transactional owner doubles.

This is a deterministic orchestration check, not a Client/UI or runtime test.
Run from a C++20 developer shell (cl, clang++, g++, or CXX).
"""
from pathlib import Path
import os
import shlex
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def production_stage_reorder():
    text = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(encoding="utf-8")
    start = text.index("bool_t Client::CValtanActionWorkbench::Reorder_SelectedStages(")
    end = text.index("\n}\n", start) + len("\n}")
    return text[start:end]


class ValtanStageReorderNativeTests(unittest.TestCase):
    def test_stage_animation_selection_one_step_reorder_and_rollback(self):
        configured = os.environ.get("CXX")
        command = ([configured.strip('"')] if configured and Path(configured.strip('"')).is_file()
                   else shlex.split(configured) if configured else [])
        if not command:
            compiler = next((shutil.which(name) for name in ("cl", "clang++", "g++") if shutil.which(name)), None)
            if compiler is None:
                self.skipTest("A C++20 developer shell is required for the native reorder regression")
            command = [compiler]
        fixture = (Path(__file__).parent / "native/workbench_stage_reorder_fixture.cpp").read_text(encoding="utf-8")
        self.assertEqual(fixture.count("// @PRODUCTION_STAGE_REORDER@"), 1)
        fixture = fixture.replace("// @PRODUCTION_STAGE_REORDER@", production_stage_reorder())
        with tempfile.TemporaryDirectory(prefix="valtan-stage-reorder-") as folder:
            folder = Path(folder)
            source = folder / "stage_reorder.cpp"
            executable = folder / ("stage_reorder.exe" if os.name == "nt" else "stage_reorder")
            source.write_text(fixture, encoding="utf-8")
            msvc = Path(command[0]).name.lower() in ("cl", "cl.exe", "clang-cl", "clang-cl.exe")
            arguments = (["/nologo", "/EHsc", "/std:c++20", str(source), "/Fe:" + str(executable)] if msvc
                         else ["-std=c++20", str(source), "-o", str(executable)])
            compiled = subprocess.run(command + arguments, cwd=folder, capture_output=True, text=True, errors="replace", timeout=90)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(executable)], cwd=folder, capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("stage reorder checks", result.stdout)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
