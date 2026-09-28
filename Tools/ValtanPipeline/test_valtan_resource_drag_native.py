"""Reproduce the Summon search assertion in bundled ImGui without a Client/window."""
from pathlib import Path
import os
import shlex
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ResourceDragNativeTests(unittest.TestCase):
    def test_search_focus_and_resource_drag_keep_valid_item_identity(self):
        configured = os.environ.get("CXX")
        command = ([configured.strip('"')] if configured and Path(configured.strip('"')).is_file()
                   else shlex.split(configured) if configured else [])
        if not command:
            compiler = shutil.which("cl")
            if not compiler:
                self.skipTest("Bundled ImGui DLL config requires a Windows C++20 developer shell")
            command = [compiler]
        if Path(command[0]).name.lower() not in ("cl", "cl.exe", "clang-cl", "clang-cl.exe"):
            self.skipTest("Bundled ImGui DLL config requires an MSVC-compatible compiler")
        source = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(encoding="utf-8")
        supplemental = source.index("void Client::CValtanActionWorkbench::Render_SupplementalResources(")
        begin = source.index("if (domain == RESOURCE_DOMAIN::SUMMON)", supplemental)
        end = source.index("if (domain == RESOURCE_DOMAIN::LOGIC)", begin)
        summon = source[begin:end]
        logic = source[end:source.index("bool_t Client::CValtanActionWorkbench::Append_EffectResourceAtPatternEnd(", end)]
        search = summon[summon.index("ImGui::InputTextWithHint("):summon.index("m_SummonResourceSearch.size());") + len("m_SummonResourceSearch.size());")]

        def rows(block, text):
            selected = [line.strip() for line in block.splitlines() if text in line or
                        'if (ImGui::SmallButton("Copy Resource"))' in line or "Offer_CompositionResourceDrag(" in line]
            self.assertEqual(len(selected), 3)
            # Record the actual button position for headless public mouse events.
            return "\n".join(line + (" copyPosition = {ImGui::GetItemRectMin().x + 5.f, ImGui::GetItemRectMin().y + 5.f};"
                                      if "SmallButton" in line else "") for line in selected)

        shared = (ROOT / "Client/Private/CompositionResourceTree.cpp").read_text(encoding="utf-8")
        start = shared.index("void Client::Offer_CompositionResourceDrag(")
        stop = shared.index("\n}\n", start) + len("\n}")
        fixture = (Path(__file__).parent / "native/workbench_resource_drag_fixture.cpp").read_text(encoding="utf-8")
        replacements = {
            "// @PRODUCTION_DRAG@": shared[start:stop],
            "// @PRODUCTION_SEARCH@": search,
            "// @PRODUCTION_ROWS@": rows(summon, 'ImGui::TextDisabled("Spawn count'),
            "// @PRODUCTION_LOGIC_ROWS@": rows(logic, 'ImGui::TextWrapped("%s | %s [%s]"'),
        }
        for marker, value in replacements.items():
            self.assertEqual(fixture.count(marker), 1)
            fixture = fixture.replace(marker, value)
        with tempfile.TemporaryDirectory(prefix="valtan-resource-drag-") as folder:
            folder = Path(folder)
            source_file, assert_header = folder / "resource_drag.cpp", folder / "test_assert.h"
            source_file.write_text(fixture, encoding="utf-8")
            assert_header.write_text("#pragma once\nvoid TestImGuiAssert(const char*, const char*, int);\n"
                                     "#define IM_ASSERT(expression) ((expression) ? (void)0 : TestImGuiAssert(#expression, __FILE__, __LINE__))\n")
            imgui = ROOT / "Engine/External/imgui"
            executable = folder / "resource_drag.exe"
            sources = [source_file] + [imgui / name for name in ("imgui.cpp", "imgui_draw.cpp", "imgui_tables.cpp", "imgui_widgets.cpp")]
            args = ["/nologo", "/EHsc", "/std:c++20", "/MDd", "/D_DEBUG", "/DENGINE_EXPORTS", "/utf-8",
                    "/FI" + str(assert_header), "/I" + str(imgui), *map(str, sources), "/Fe:" + str(executable), "user32.lib", "imm32.lib"]
            compiled = subprocess.run(command + args, cwd=folder, capture_output=True, text=True, errors="replace", timeout=90)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(executable)], cwd=folder, capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("BASELINE 0 at", result.stdout)
            self.assertIn("PASS real ImGui Summon/Logic search click", result.stdout)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
