"""Execute the production Animation Delete against transactional owner storage.

The fixture covers dependency cascade and rollback. Source schema/persistence and
the real Balance owner are covered by the canonical Animation NONE tests.
"""
from pathlib import Path
import os
import shlex
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class AnimationDeleteNativeTests(unittest.TestCase):
    def test_clip_dependencies_commit_or_roll_back_together(self):
        configured = os.environ.get("CXX")
        command = ([configured.strip('"')] if configured and Path(configured.strip('"')).is_file()
                   else shlex.split(configured) if configured else [])
        if not command:
            compiler = next((shutil.which(name) for name in ("cl", "clang++", "g++") if shutil.which(name)), None)
            if compiler is None:
                self.skipTest("A C++20 developer shell is required")
            command = [compiler]
        production = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(encoding="utf-8")
        start = production.index("bool_t Client::CValtanActionWorkbench::Remove_AnimationOccurrence(")
        end = production.index("\n}\n", start) + len("\n}")
        fixture = (Path(__file__).parent / "native/workbench_animation_delete_fixture.cpp").read_text(encoding="utf-8")
        self.assertEqual(fixture.count("// @PRODUCTION_ANIMATION_DELETE@"), 1)
        fixture = fixture.replace("// @PRODUCTION_ANIMATION_DELETE@", production[start:end])
        with tempfile.TemporaryDirectory(prefix="valtan-animation-delete-") as folder:
            folder = Path(folder)
            source = folder / "animation_delete.cpp"
            executable = folder / ("animation_delete.exe" if os.name == "nt" else "animation_delete")
            source.write_text(fixture, encoding="utf-8")
            msvc = Path(command[0]).name.lower() in ("cl", "cl.exe", "clang-cl", "clang-cl.exe")
            args = (["/nologo", "/EHsc", "/std:c++20", str(source), "/Fe:" + str(executable)] if msvc
                    else ["-std=c++20", str(source), "-o", str(executable)])
            compile_result = subprocess.run(command + args, cwd=folder, capture_output=True, text=True, errors="replace", timeout=90)
            self.assertEqual(compile_result.returncode, 0, compile_result.stdout + compile_result.stderr)
            result = subprocess.run([str(executable)], cwd=folder, capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("Animation Delete transaction checks", result.stdout)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
