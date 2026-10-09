#include <string>
#include "UserSettingsDocument.h"
#include "RuntimeAssetRoot.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
float EffectiveDefaultForHarness(const std::string& rowId, float authoredDefault);
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
RENDER_QUALITY_SETTINGS Video(const CUserSettings& settings, bool profileEffects=true) {
    RENDER_QUALITY_SETTINGS quality;
    quality.fGamma=1.73f;
    quality.bBloomEnabled=quality.bFXAAEnabled=quality.bSSAOEnabled=profileEffects;
    settings.Apply_Video(quality);
    return quality;
}
bool SameOtherVideo(const RENDER_QUALITY_SETTINGS& left, const RENDER_QUALITY_SETTINGS& right) {
    return left.fGamma==right.fGamma && left.bBloomEnabled==right.bBloomEnabled &&
        left.bFXAAEnabled==right.bFXAAEnabled && left.bSSAOEnabled==right.bSSAOEnabled &&
        left.iColorFilterType==right.iColorFilterType && left.fColorFilterStrength==right.fColorFilterStrength;
}
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

#ifdef _DEBUG
    constexpr uint32_t textureDefault=0u;
#else
    constexpr uint32_t textureDefault=0u;
#endif
    const float uiTextureDefault=EffectiveDefaultForHarness(SystemOptionRowId::TEXTURE_QUALITY,0.f);
    Check(CUserSettings::Get_DefaultTextureQuality()==float(textureDefault),"configuration texture default matches Debug and Release best");
    Check(uiTextureDefault==float(textureDefault),"actual UI default used by initial seed and Reset matches configuration");
    Check(EffectiveDefaultForHarness("unrelated.row",7.f)==7.f &&
        EffectiveDefaultForHarness(SystemOptionRowId::BATTLE_FONT_SIZE,-1.f)==1.f,
        "texture default leaves unrelated authored defaults unchanged");
    Select(L"texture_default_missing_file"); CUserSettings seeded;
    Check(Video(seeded).iTextureMinMip==textureDefault,"pre-UI missing row uses build texture default");
    seeded.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(seeded.Load_Persisted(status) && Video(seeded).iTextureMinMip==textureDefault &&
        seeded.Get_Settings().Get(SystemOptionRowId::TEXTURE_QUALITY,-1.f)==uiTextureDefault,
        "new settings and initial UI seed agree with build texture default");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"default initialization does not save personal settings");
    Select(L"texture_default_missing_row");
    const std::string missingTexture=R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"WINDOWED"},"values":{"unknown.preserved":42}})";
    Write(missingTexture); CUserSettings earlyDefault; earlyDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(earlyDefault.Load_Persisted(status) && Video(earlyDefault).iTextureMinMip==textureDefault &&
        earlyDefault.Get_Settings().Get("unknown.preserved",0.f)==42.f,
        "missing persisted texture row merges defaults seeded before load");
    CUserSettings lateDefault;
    Check(lateDefault.Load_Persisted(status) && Video(lateDefault).iTextureMinMip==textureDefault,
        "missing persisted texture row uses fallback before UI seed");
    lateDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(lateDefault.Get_Settings().Has_SameValues(earlyDefault.Get_Settings()) && Read()==missingTexture,
        "seeding after load agrees and preserves exact saved bytes");
    Select(L"texture_quality"); CUserSettings texture; Check(texture.Load_Persisted(status),"texture initial load"); Accept(texture);
    Check(Video(texture).iTextureMinMip==textureDefault,"missing texture row uses configuration default");
    auto textureBaseline=texture.Get_Settings();
    textureBaseline.Values[SystemOptionRowId::TEXTURE_QUALITY]=2.f;
    textureBaseline.Values[SystemOptionRowId::BRIGHTNESS]=72.f;
    textureBaseline.Values[SystemOptionRowId::COLOR_FILTER_TYPE]=2.f;
    textureBaseline.Values[SystemOptionRowId::COLOR_FILTER_VALUE]=63.f;
    Check(texture.Commit(textureBaseline,status),"texture baseline save");
    for(uint32_t choice=0;choice!=4;++choice) {
        const auto snapshot=texture.Get_Settings();
        const auto savedBytes=Read();
        const auto before=Video(texture);
        const auto disabledBefore=Video(texture,false);
        auto draft=snapshot; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=static_cast<float>(choice);
        calls=0; texture.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
        Check(texture.Preview(draft,status),"texture choice previews");
        Check(Video(texture).iTextureMinMip==choice,"texture choice reaches effective minimum mip");
        Check(calls==0 && Read()==savedBytes,"texture preview does not apply display or save");
        Check(SameOtherVideo(Video(texture),before),"texture choice preserves effective gamma bloom FXAA SSAO and color filter");
        const auto disabledAfter=Video(texture,false);
        Check(SameOtherVideo(disabledAfter,disabledBefore) && !disabledAfter.bBloomEnabled &&
            !disabledAfter.bFXAAEnabled && !disabledAfter.bSSAOEnabled,"texture choice preserves disabled scene effects");
        Check(texture.Preview(snapshot,status) && Video(texture).iTextureMinMip==before.iTextureMinMip &&
            texture.Get_Settings().Has_SameValues(snapshot) && Read()==savedBytes,"texture cancel restores previous mip and exact snapshot without saving");
        Check(texture.Commit(draft,status),"texture choice commits");
        CUserSettings textureReload; textureReload.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
        Check(textureReload.Load_Persisted(status),"texture saved choice reloads");
        textureReload.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
        Check(textureReload.Get_Settings().Has_SameValues(draft) && Video(textureReload).iTextureMinMip==choice,
            "texture reload retains selected mip and unrelated settings");
        auto resetDraft=textureReload.Get_Settings(); const auto resetBefore=resetDraft;
        const auto resetVideoBefore=Video(textureReload); const auto resetBytes=Read();
        resetDraft.Values[SystemOptionRowId::TEXTURE_QUALITY]=EffectiveDefaultForHarness(SystemOptionRowId::TEXTURE_QUALITY,0.f);
        Check(textureReload.Preview(resetDraft,status) && Video(textureReload).iTextureMinMip==textureDefault &&
            SameOtherVideo(Video(textureReload),resetVideoBefore) && Read()==resetBytes,
            "Reset texture preview uses actual UI configuration default without saving or changing other video rows");
        Check(textureReload.Preview(resetBefore,status) && Video(textureReload).iTextureMinMip==choice && Read()==resetBytes,
            "cancel after Reset preserves explicit saved texture choice");
    }
    const auto textureSaved=texture.Get_Settings();
    const auto textureBytes=Read();
    for(const float value:{-1.f,4.f,0.5f}) {
        auto draft=textureSaved; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=value;
        Check(texture.Preview(draft,status) && Video(texture).iTextureMinMip==0u,"out-of-range or fractional texture choice safely uses mip zero");
        Check(Read()==textureBytes && texture.Preview(textureSaved,status),"invalid texture preview remains unsaved and cancellable");
    }
    for(const float value:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
        auto draft=textureSaved; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=value;
        Check(!texture.Preview(draft,status) && texture.Get_Settings().Has_SameValues(textureSaved) && Read()==textureBytes,
            "nonfinite texture preview is rejected without changing memory or file");
        CUserSettings invalidDefault; invalidDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,value);
        Check(invalidDefault.Get_Settings().Values.count(SystemOptionRowId::TEXTURE_QUALITY)==0 && Video(invalidDefault).iTextureMinMip==textureDefault,
            "nonfinite texture default is ignored and missing-row build default remains");
    }
    auto oversized=textureSaved; oversized.Values[SystemOptionRowId::TEXTURE_QUALITY]=std::numeric_limits<float>::max();
    Check(!texture.Preview(oversized,status) && texture.Get_Settings().Has_SameValues(textureSaved) && Read()==textureBytes,
        "oversized texture preview is rejected without changing memory or file");
    CUserSettings oversizedDefault; oversizedDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,std::numeric_limits<float>::max());
    Check(Video(oversizedDefault).iTextureMinMip==0u,"oversized finite texture default safely uses mip zero");

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
