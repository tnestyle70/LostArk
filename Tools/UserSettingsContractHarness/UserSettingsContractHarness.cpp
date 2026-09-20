#include <string>
#include "UserSettingsDocument.h"
#include "RuntimeAssetRoot.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
HINSTANCE g_hInst = nullptr;
HWND g_hWnd = nullptr;
std::filesystem::path Client::CRuntimeAssetRoot::Resolve(const std::filesystem::path&) { return {}; }
namespace {
std::filesystem::path base;
unsigned assertions = 0;
void Check(bool value, const char* description) {
    if (!value) throw std::runtime_error(description);
    ++assertions; std::cout << "PASS " << description << "\n";
}
void Select(const wchar_t* name) {
    auto path=base/name; std::filesystem::create_directories(path);
    if (!SetEnvironmentVariableW(L"LOCALAPPDATA",path.c_str())) throw std::runtime_error("SetEnvironmentVariable");
}
std::string Read() { std::ifstream in(CUserSettings::Get_SettingsPath(),std::ios::binary); return {std::istreambuf_iterator<char>(in),{}}; }
void Write(const std::string& text) { auto p=CUserSettings::Get_SettingsPath(); std::filesystem::create_directories(p.parent_path()); std::ofstream out(p,std::ios::binary|std::ios::trunc); out << text; out.flush(); if(!out) throw std::runtime_error("Write fixture"); }
USER_SETTINGS Candidate(const CUserSettings& settings) { auto v=settings.Get_Settings(); v.Display.width=1600; v.Display.height=900; v.Values["unknown.future\"\nrow"]=12.25f; v.Values[SystemOptionRowId::MASTER_VOLUME]=73.f; return v; }
void Accept(CUserSettings& settings) { settings.Set_DisplayApplyCallback([](const USER_DISPLAY_SETTINGS&,std::string& status){status="test display applied";return true;}); }
void NoTemps() { for(const auto& file:std::filesystem::directory_iterator(CUserSettings::Get_SettingsPath().parent_path())) Check(file.path().extension()!=L".tmp","staged temporary removed"); }
}
int wmain(int argc,wchar_t** argv) {
 try {
    if(argc!=3) return 2;
    const auto allowed=std::filesystem::weakly_canonical(std::filesystem::path(argv[1])/L"out").wstring()+L"\\";
    const auto requested=std::filesystem::weakly_canonical(std::filesystem::path(argv[2])).wstring();
    if(requested.size()<=allowed.size() || _wcsnicmp(requested.c_str(),allowed.c_str(),allowed.size())!=0)
        throw std::runtime_error("Fixture output must remain inside repository out");
    base=std::filesystem::path(requested)/(std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(base);
    std::string status;
    Select(L"roundtrip");
    CUserSettings original; original.Set_Default("unknown.default",4.f);
    Check(original.Load_Persisted(status),"missing file loads defaults");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"load does not create file");
    Accept(original); auto selected=Candidate(original);
    Check(original.Commit(selected,status),"first save succeeds");
    const auto firstBytes=Read();
    Check(!firstBytes.empty(),"first save creates real JSON");
    CUserSettings reload; Check(reload.Load_Persisted(status),"new instance reload succeeds");
    Check(reload.Get_Settings().Has_SameValues(selected),"reload preserves display and all numeric rows");
    reload.Set_Default("unknown.future\"\nrow",999.f);
    Check(reload.Get_Settings().Get("unknown.future\"\nrow",0.f)==12.25f,"default seeding preserves loaded unknown row");
    auto next=selected; next.Display.width=1280; next.Display.height=720;
    Check(original.Commit(next,status),"second save succeeds");
    bool matchingBackup=false;
    for(const auto& file:std::filesystem::directory_iterator(CUserSettings::Get_SettingsPath().parent_path())) if(file.path().extension()==L".bak") {std::ifstream in(file.path(),std::ios::binary); std::string text{std::istreambuf_iterator<char>(in),{}}; matchingBackup |= text==firstBytes;}
    Check(matchingBackup,"backup exactly preserves previous file bytes");
    NoTemps();
    for(const auto mode:{USER_WINDOW_MODE::WINDOWED,USER_WINDOW_MODE::BORDERLESS,USER_WINDOW_MODE::FULLSCREEN}) { next.Display.mode=mode; Check(original.Commit(next,status),"window mode serializes"); CUserSettings other; Check(other.Load_Persisted(status) && other.Get_DisplaySettings()==next.Display,"window mode reload matches"); }

    Select(L"preview"); CUserSettings preview; Check(preview.Load_Persisted(status),"preview initial load"); Accept(preview); auto previewDraft=Candidate(preview);
    int calls=0; preview.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
    const auto initial=preview.Get_Settings();
    Check(preview.Preview(previewDraft,status),"live preview accepted");
    Check(calls==0 && preview.Get_DisplaySettings()==initial.Display,"preview never invokes display or changes committed display");
    Check(preview.Get_Settings().Values==previewDraft.Values,"preview exposes non-display values");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"preview never writes disk");
    Check(preview.Preview(initial,status) && preview.Get_Settings().Has_SameValues(initial),"cancel preview restores snapshot without file");

    Select(L"malformed");
    const std::vector<std::string> invalid={"{", "[]", "{}", R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920.5,"height":1080,"mode":"WINDOWED"},"values":{}})",R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"BAD"},"values":{}})",R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"WINDOWED"},"values":{"bad":"text"}})"};
    for(const auto& bytes:invalid) {Write(bytes); CUserSettings broken; broken.Set_Default("preserved",22.f); const auto before=broken.Get_Settings(); Check(!broken.Load_Persisted(status),"malformed/schema-invalid file rejected"); Check(broken.Get_Settings().Has_SameValues(before) && Read()==bytes,"invalid load preserves memory and file"); Accept(broken); Check(!broken.Commit(Candidate(broken),status) && Read()==bytes,"invalid source cannot be overwritten by Apply");}

    Select(L"callback_failure"); CUserSettings failure; Check(failure.Load_Persisted(status),"callback initial load"); Accept(failure); Check(failure.Commit(failure.Get_Settings(),status),"callback baseline save"); auto baselineBytes=Read(); const auto baseline=failure.Get_Settings();
    failure.Set_DisplayApplyCallback([&](const auto&,auto& s){s="injected callback failure";++calls;return false;});
    Check(!failure.Commit(Candidate(failure),status),"display callback failure rejects commit");
    Check(Read()==baselineBytes && failure.Get_Settings().Has_SameValues(baseline),"callback failure preserves exact disk and memory"); NoTemps();
    auto bad=baseline; bad.Values["nan"]=std::numeric_limits<float>::quiet_NaN();
    Check(!failure.Commit(bad,status) && Read()==baselineBytes,"nonfinite staged value rejected before mutation");

    Select(L"external_before"); CUserSettings stale; Check(stale.Load_Persisted(status),"external initial load"); Accept(stale); Check(stale.Commit(stale.Get_Settings(),status),"external baseline save");
    auto external=Read()+"\n "; Write(external); calls=0; stale.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
    Check(!stale.Commit(Candidate(stale),status) && calls==0 && Read()==external,"external prior edit rejects before display apply and preserves edit");

    Select(L"external_during"); CUserSettings race; Check(race.Load_Persisted(status),"race initial load"); Accept(race); Check(race.Commit(race.Get_Settings(),status),"race baseline save"); const auto raceOld=race.Get_Settings(); external=Read()+"\n\n"; calls=0; USER_DISPLAY_SETTINGS actual=raceOld.Display;
    race.Set_DisplayApplyCallback([&](const auto& target,auto&){actual=target; ++calls; if(calls==1) Write(external); return true;});
    Check(!race.Commit(Candidate(race),status),"external edit during callback rejects final freshness check");
    Check(calls==2 && actual==raceOld.Display && race.Get_Settings().Has_SameValues(raceOld),"race rollback restores display and retains memory");
    Check(Read()==external,"race leaves external bytes untouched"); NoTemps();

    Select(L"replace_failure"); CUserSettings locked; Check(locked.Load_Persisted(status),"locked initial load"); Accept(locked); Check(locked.Commit(locked.Get_Settings(),status),"locked baseline save"); const auto lockedOld=locked.Get_Settings(); baselineBytes=Read(); calls=0; actual=lockedOld.Display;
    locked.Set_DisplayApplyCallback([&](const auto& target,auto&){actual=target;++calls;return true;});
    HANDLE file=CreateFileW(CUserSettings::Get_SettingsPath().c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    Check(file!=INVALID_HANDLE_VALUE,"fixture holds target without delete sharing");
    const bool committed=locked.Commit(Candidate(locked),status); CloseHandle(file);
    Check(!committed,"real Win32 file lock causes atomic replacement failure");
    Check(calls==2 && actual==lockedOld.Display,"disk failure invokes previous display rollback");
    Check(Read()==baselineBytes && locked.Get_Settings().Has_SameValues(lockedOld),"disk failure preserves exact disk and memory"); NoTemps();
    Check(locked.Commit(Candidate(locked),status),"retry after lock release succeeds");
    std::cout << "SUCCESS assertions=" << assertions << " fixtures=" << base.string() << "\n";
    return 0;
 } catch(const std::exception& ex) {std::cerr<<"FAIL "<<ex.what()<<"\n"; return 1;}
}
