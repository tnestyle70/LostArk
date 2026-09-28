"""Execute production Pattern-end append routing with deterministic typed owner storage."""
from pathlib import Path
import os
import shlex
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ValtanPatternEndAppendNativeTests(unittest.TestCase):
    def test_sequence_and_raw_append_ignore_current_stage_and_rollback(self):
        configured = os.environ.get("CXX")
        command = ([configured.strip('"')] if configured and Path(configured.strip('"')).is_file()
                   else shlex.split(configured) if configured else [])
        if not command:
            compiler = next((shutil.which(name) for name in ("cl", "clang++", "g++") if shutil.which(name)), None)
            if compiler is None:
                self.skipTest("A C++20 developer shell is required")
            command = [compiler]
        source = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(encoding="utf-8")
        fixture = (Path(__file__).parent / "native/workbench_pattern_end_append_fixture.cpp").read_text(encoding="utf-8")
        methods = (
            ("SEQUENCE_END", "bool_t Client::CValtanActionWorkbench::Append_SelectedSequenceToPattern("),
            ("RAW_CAN", "bool Client::CValtanActionWorkbench::Can_AppendCompositionAnimationResource("),
            ("RAW_APPEND", "bool Client::CValtanActionWorkbench::Append_CompositionAnimationResource("),
            ("RAW_APPLY", "bool Client::CValtanActionWorkbench::Apply_CompositionResourceAppend("),
        )
        for marker, signature in methods:
            start = source.index(signature)
            end = source.index("\n}\n", start) + len("\n}")
            placeholder = "// @" + marker + "@"
            self.assertEqual(fixture.count(placeholder), 1)
            fixture = fixture.replace(placeholder, source[start:end])
        with tempfile.TemporaryDirectory(prefix="valtan-pattern-end-") as folder:
            folder = Path(folder)
            cpp = folder / "pattern_end.cpp"
            executable = folder / ("pattern_end.exe" if os.name == "nt" else "pattern_end")
            cpp.write_text(fixture, encoding="utf-8")
            msvc = Path(command[0]).name.lower() in ("cl", "cl.exe", "clang-cl", "clang-cl.exe")
            arguments = (["/nologo", "/EHsc", "/std:c++20", str(cpp), "/Fe:" + str(executable)] if msvc
                         else ["-std=c++20", str(cpp), "-o", str(executable)])
            compiled = subprocess.run(command + arguments, cwd=folder, capture_output=True, text=True, errors="replace", timeout=90)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(executable)], cwd=folder, capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("pattern end append checks", result.stdout)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
