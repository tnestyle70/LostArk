"""Compile the actual editor Save/Load helpers with the real JSON codec, without Client/UI."""
import os
import subprocess
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


class BossWeaponSaveTests(unittest.TestCase):
    def test_save_merge_conflict_and_failed_commit(self):
        vcvars = next(Path(os.environ.get("ProgramFiles", "C:/Program Files")).glob(
            "Microsoft Visual Studio/*/*/VC/Auxiliary/Build/vcvars64.bat"), None)
        if vcvars is None:
            self.skipTest("MSVC is required for the Windows atomic-save contract")
        output = ROOT / "out/ValtanAxeEditorTests"
        output.mkdir(parents=True, exist_ok=True)
        header = (ROOT / "Client/Public/DataJson.h").read_text(encoding="utf-8")
        header = header.replace('#include "Client_Defines.h"', '').replace('#include "Engine_Defines.h"', '')
        codec = (ROOT / "Client/Private/DataJson.cpp").read_text(encoding="utf-8").replace('#include "DataJson.h"', '')
        editor = (ROOT / "Client/Private/MainApp_ValtanAxe.cpp").read_text(encoding="utf-8")
        helpers = editor[editor.index("namespace\n{"):editor.index("void CMainApp::RenderValtanAxeEditor()")]
        catalog = (ROOT / "Client/Private/ActorCatalog.cpp").read_text(encoding="utf-8")
        validation = catalog[catalog.index("bool_t Client::CActorCatalog::Validate_BossWeaponSocketTransform"):
                             catalog.index("bool_t Client::CActorCatalog::Set_ValtanWeaponSocketTransform")]
        source = r'''
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
using bool_t = bool;
using f32_t = float;
struct float3_t { float x = 0, y = 0, z = 0; };
#define NS_BEGIN(x) namespace x {
#define NS_END }
namespace Client {
struct BOSS_WEAPON_SOCKET_TRANSFORM { float3_t positionMeters{}, rotationDegrees{}; };
class CActorCatalog { public:
    static bool Validate_BossWeaponSocketTransform(const BOSS_WEAPON_SOCKET_TRANSFORM&);
    static bool Set_ValtanWeaponSocketTransform(const char*, const BOSS_WEAPON_SOCKET_TRANSFORM&, std::string&) { return true; }
};
static std::filesystem::path testData;
class CProjectDataRoot { public:
    static std::filesystem::path Resolve(const std::filesystem::path& path) { return testData / path; }
};
}
'''
        source += header + "\n" + codec + "\n" + validation + "\n" + helpers + r'''
int main(int argc, char** argv) {
    using namespace Client;
    assert(argc == 2);
    const auto root = std::filesystem::path(argv[1]) / std::to_string(GetCurrentProcessId());
    testData = root / "Data";
    std::filesystem::create_directories(testData / "Actors");
    const auto file = testData / "Actors/BossCatalog.json";
    const char* owner = "BOSS_VALTAN";
    auto write = [&](const std::string& text) { std::ofstream output(file, std::ios::binary); output << text; };
    auto text = [&]() { std::ifstream input(file, std::ios::binary); return std::string(std::istreambuf_iterator<char>(input), {}); };
    const std::string legacy = R"({"schema":"lostark.boss-catalog","formatVersion":8,"bosses":[{"archetypeId":"BOSS_VALTAN","untouched":17},{"archetypeId":"BOSS_VALTAN_GHOST","untouched":29}]})";
    write(legacy);
    AXE_DRAFT draft;
    assert(Load(owner, draft) && Equal(draft.value, {}));
    draft.value.positionMeters.x = .125f;
    assert(Save(owner, draft));
    assert(text() != legacy && std::filesystem::is_regular_file(file.wstring() + L".axe.previous.bak"));
    AXE_DRAFT external;
    assert(Load(owner, external));
    draft.value.positionMeters.x = .25f;
    external.value.positionMeters.y = .5f;
    assert(Save(owner, external));
    // Unrelated JSON edits made after the draft was opened must survive too.
    std::string changed = text();
    auto location = changed.find("\"untouched\": 29"); assert(location != std::string::npos);
    changed.replace(location, std::string("\"untouched\": 29").size(), "\"untouched\": 31"); write(changed);
    assert(Save(owner, draft) && draft.value.positionMeters.x == .25f && draft.value.positionMeters.y == .5f);
    assert(text().find("\"untouched\": 31") != std::string::npos);
    assert(Load(owner, external));
    external.value.positionMeters.x = .75f; assert(Save(owner, external));
    draft.value.positionMeters.x = 1.f;
    const std::string conflict = text();
    assert(!Save(owner, draft) && text() == conflict && draft.value.positionMeters.x == 1.f);
    assert(Load(owner, draft));
    draft.value.rotationDegrees.z = 45.f;
    {
        WRITER_LOCK busy; std::string status; assert(busy.Acquire(root, status));
        assert(!Save(owner, draft) && text() == conflict);
    }
    // A locked target rejects final replacement after staging; disk and draft survive.
    HANDLE held = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(held != INVALID_HANDLE_VALUE);
    assert(!Save(owner, draft) && text() == conflict && draft.value.rotationDegrees.z == 45.f);
    CloseHandle(held);
    assert(Save(owner, draft));
    assert(Load(owner, external) && external.value.rotationDegrees.z == 45.f);
    const std::string saved = text();
    draft.value.positionMeters.x = std::numeric_limits<float>::infinity();
    assert(!Save(owner, draft) && text() == saved);
    for (const auto& entry : std::filesystem::directory_iterator(file.parent_path()))
        assert(entry.path().filename().string().find(".tmp.") == std::string::npos);
    std::cout << "PASS actual Save/Load: legacy, unrelated axis+boss merge, same-axis conflict, writer lock, failed atomic commit, retry, nonfinite\n";
}
'''
        cpp = output / "actual_save_test.cpp"
        cpp.write_text(source, encoding="utf-8")
        script = output / "build.cmd"
        script.write_text(f'@echo off\ncall "{vcvars}" >nul\ncl /nologo /std:c++20 /EHsc /utf-8 /W3 "{cpp}" /Fe:"{output / "actual_save_test.exe"}" /Fo:"{output / "actual_save_test.obj"}"\n', encoding="utf-8")
        built = subprocess.run(["cmd", "/c", str(script)], capture_output=True, text=True)
        self.assertEqual(built.returncode, 0, built.stdout + built.stderr)
        result = subprocess.run([str(output / "actual_save_test.exe"), str(output / "fixtures")], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
