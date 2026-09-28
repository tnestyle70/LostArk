"""Run the shared timeline hit test and editing shortcuts without opening Client."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from Tools.ValtanPipeline.test_valtan_sequence_append_native import ROOT, toolchain


PROBE = r'''
#include "CompositionTimeline.h"
#include "CompositionEditing.h"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace Client;
using namespace Client::CompositionTimeline;
int main() {
    int checks = 0;
    auto check = [&](bool ok) { ++checks; if (!ok) throw std::runtime_error("check " + std::to_string(checks)); };
    const ImVec2 lo(100, 50), hi(180, 80);
    for (const auto& [anchor, pointer] : std::vector<std::pair<ImVec2, ImVec2>>{
        {{90,40},{190,90}}, {{190,90},{90,40}}, {{190,40},{90,90}}, {{90,90},{190,40}}})
        check(MarqueeIntersectsBox(anchor, pointer, lo, hi));
    check(MarqueeIntersectsBox({120,60}, {130,70}, lo, hi));
    check(MarqueeIntersectsBox({70,40}, {105,55}, lo, hi));
    check(!MarqueeIntersectsBox({0,0}, {90,90}, lo, hi));
    check(!MarqueeIntersectsBox({181,0}, {200,90}, lo, hi));
    check(!MarqueeIntersectsBox({0,0}, {200,49}, lo, hi));
    check(!MarqueeIntersectsBox({0,81}, {200,100}, lo, hi));
    check(MarqueeIntersectsBox({70,50}, {100,80}, lo, hi));
    check(MarqueeIntersectsBox({700,240}, {710,280}, {706,260}, {710,270}));
    check(!MarqueeIntersectsBox({0,0}, {200,200}, {100,50}, {100,80}));
    check(!MarqueeIntersectsBox({0,0}, {200,200}, {100,80}, {180,50}));
    check(!MarqueeIntersectsBox({std::numeric_limits<float>::quiet_NaN(),0}, {200,200}, lo, hi));
    check(!MarqueeIntersectsBox({0,0}, {200,std::numeric_limits<float>::infinity()}, lo, hi));

    COMPOSITION_EDIT_INPUT input;
    input.focused = input.control = input.copyPressed = true;
    check(Resolve_CompositionShortcut(input) == COMPOSITION_EDIT_COMMAND::COPY);
    input.copyPressed = false; input.pastePressed = true;
    check(Resolve_CompositionShortcut(input) == COMPOSITION_EDIT_COMMAND::PASTE);
    input.pastePressed = false; input.duplicatePressed = true;
    check(Resolve_CompositionShortcut(input) == COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION);
    for (auto field : {&COMPOSITION_EDIT_INPUT::textInput, &COMPOSITION_EDIT_INPUT::activeItem,
        &COMPOSITION_EDIT_INPUT::popup, &COMPOSITION_EDIT_INPUT::dragging}) {
        input.*field = true; check(!Resolve_CompositionShortcut(input)); input.*field = false;
    }
    input.focused = false; check(!Resolve_CompositionShortcut(input)); input.focused = true;
    input.control = false; check(!Resolve_CompositionShortcut(input));

    std::string status;
    auto value = std::make_shared<COMPOSITION_EFFECT_TRANSFER>();
    value->items.push_back({}); value->items.front().resourceId = "saved-effect";
    int pasted = 0, duplicated = 0;
    auto capture = [&](std::string&) -> COMPOSITION_TRANSFER { return value; };
    auto paste = [&](const COMPOSITION_TRANSFER& item, std::string&) { ++pasted; return item == value; };
    auto duplicate = [&](std::string&) { ++duplicated; return true; };
    auto& clipboard = CCompositionClipboard::Get();
    clipboard.Clear();
    check(!Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::PASTE, capture, paste, duplicate, status));
    check(pasted == 0);
    check(Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::COPY, capture, paste, duplicate, status));
    check(clipboard.Read() == value);
    check(!Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::COPY,
        [](std::string&) -> COMPOSITION_TRANSFER { return {}; }, paste, duplicate, status));
    check(clipboard.Read() == value);
    check(Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::PASTE, capture, paste, duplicate, status));
    check(pasted == 1);
    check(Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, capture, paste, duplicate, status));
    check(duplicated == 1 && clipboard.Read() == value);
    check(!Dispatch_CompositionEdit(COMPOSITION_EDIT_COMMAND::PASTE, capture,
        [](const auto&, std::string&) { return false; }, duplicate, status));
    check(clipboard.Read() == value);
    std::cout << "Composition input: " << checks << " checks PASS\n";
}
'''


class TestCompositionInputNative(unittest.TestCase):
    def test_marquee_shortcuts_and_clipboard_failure_preservation(self):
        with tempfile.TemporaryDirectory(prefix="ValtanCompositionInput-") as directory:
            out = Path(directory)
            cl, environment = toolchain(out)
            source = out / "probe.cpp"
            source.write_text(PROBE, encoding="utf-8")
            binary = out / "probe.exe"
            result = subprocess.run([
                str(cl), "/nologo", "/EHsc", "/std:c++20", "/MDd",
                "/I" + str(ROOT / "Client/Public"),
                "/I" + str(ROOT / "Engine/External/imgui"), str(source),
                "/Fo" + str(out / "probe.obj"), "/Fe" + str(binary),
            ], env=environment, capture_output=True)
            self.assertEqual(0, result.returncode, (result.stdout + result.stderr).decode(errors="replace"))
            result = subprocess.run([str(binary)], capture_output=True)
            self.assertEqual(0, result.returncode, (result.stdout + result.stderr).decode(errors="replace"))
            self.assertIn(b"checks PASS", result.stdout)
            print(result.stdout.decode().strip())


if __name__ == "__main__":
    unittest.main()
