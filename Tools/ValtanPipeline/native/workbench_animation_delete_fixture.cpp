// Transactional storage doubles around the unmodified production Delete function.
// Failures are injected after earlier owners mutate to verify full rollback.
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
using bool_t = bool;
namespace Client {
struct Clip { std::string clipOccurrenceId; uint32_t playMs = 500; bool operator==(const Clip&) const = default; };
struct VALTAN_PRODUCT_EFFECT_CUE_VIEW {
 std::string strBindingId, strOccurrenceId, strEffectAssetId, strClipOccurrenceId;
 bool bUsesStageClock = false;
 bool operator==(const VALTAN_PRODUCT_EFFECT_CUE_VIEW&) const = default;
};
struct VALTAN_STAGE_VIEW {
 std::string strStageId = "IMPACT", strActionId = "valtan.mechanic.terrain-destruction-3.impact";
 uint32_t durationMs = 1000, hitOffsetMs = 67;
 std::string worldEvent = "worldeventset.valtan.terrain-destruction-3.floor84", motion = "retained";
 std::vector<Clip> clips;
 std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;
 std::string policy = "EXACT";
 uint32_t repeat = 1;
 bool operator==(const VALTAN_STAGE_VIEW&) const = default;
};
struct VALTAN_PATTERN_VIEW {
 std::string strPatternId = "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK";
 std::vector<VALTAN_STAGE_VIEW> Stages;
 bool operator==(const VALTAN_PATTERN_VIEW&) const = default;
};
struct ResourceRow {
 std::string pattern, stage, clip;
 bool operator==(const ResourceRow&) const = default;
};
struct VALTAN_PATTERN_SOUND_CUE_DOCUMENT { std::vector<ResourceRow> rows; bool operator==(const VALTAN_PATTERN_SOUND_CUE_DOCUMENT&) const = default; };
struct ShakeDocument { std::vector<ResourceRow> rows; bool operator==(const ShakeDocument&) const = default; };
struct CValtanPatternShakeCueDocument {
 static bool Remove_ClipDraft(ShakeDocument& draft, const std::string& pattern, const std::string& stage, const std::string& clip, std::string&) {
  std::erase_if(draft.rows, [&](const auto& row) { return row.pattern == pattern && row.stage == stage && row.clip == clip; }); return true;
 }
};
enum class EFFECT_V2_CLOCK_BASIS { STAGE, CLIP_OCCURRENCE };
struct EFFECT_V2_BINDING {
 std::string strPatternId, strStageId, strActionId, strBindingId, strClipOccurrenceId;
 EFFECT_V2_CLOCK_BASIS eClockBasis = EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE;
 bool operator==(const EFFECT_V2_BINDING&) const = default;
};
struct EFFECT_V2_STAGE_BINDING_KEY {
 std::string id;
 static auto From_Binding(const EFFECT_V2_BINDING& row) { return EFFECT_V2_STAGE_BINDING_KEY{row.strBindingId}; }
};
struct Snapshot {
 std::vector<EFFECT_V2_BINDING> rows;
 bool complete = true;
 bool Is_Ready() const { return true; }
 bool Can_MutateBossValtanBindings() const { return complete; }
 const auto& Get_BossValtanBindings() const { return rows; }
};
struct CEffectV2Catalog {
 Snapshot snapshot; bool dirty = true, fail = false; uint64_t revision = 7;
 static auto& Get() { static CEffectV2Catalog value; return value; }
 auto Get_Snapshot() { return std::make_shared<Snapshot>(snapshot); }
 auto Get_Revision() { return revision; }
 bool Apply_BossValtanBindingDraftTransaction(const std::function<bool(std::string&)>& edit, std::string& status) {
  const auto saved = *this; if (edit(status)) return true; *this = saved; return false;
 }
 bool Stage_RemoveBossValtanBindings(const std::vector<EFFECT_V2_STAGE_BINDING_KEY>& keys, std::string& status) {
  for (const auto& key : keys) {
   if (std::erase_if(snapshot.rows, [&](const auto& row) { return row.strBindingId == key.id; }) != 1) return false;
   dirty = true; ++revision;
  }
  if (fail) { status = "injected V2 failure after mutation"; return false; } return true;
 }
};
struct CBalanceTool {
 using ANIMATION_SLOT_EDIT = Clip;
 struct PATTERN_STAGE_EDIT { std::vector<Clip> animationSlots; std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> productCues; std::string animationEndPolicy; uint32_t animationRepeatCount = 1, durationMs = 1000; };
 VALTAN_PATTERN_VIEW pattern; bool dirty = true, failEffect = false, failStage = false; uint64_t generation = 9;
 bool Is_ValtanDraftDirty() const { return dirty; }
 bool Get_ValtanStageDraft(const std::string&, const std::string& stageId, PATTERN_STAGE_EDIT& out, std::string&) {
  for (const auto& stage : pattern.Stages) if (stage.strStageId == stageId) {
   out.animationSlots = stage.clips; out.productCues = stage.ProductCues; out.animationEndPolicy = stage.policy; out.animationRepeatCount = stage.repeat; out.durationMs = stage.durationMs; return true;
  } return false;
 }
 bool Get_ValtanPatternDraft(const std::string&, VALTAN_PATTERN_VIEW& out, std::string&) { out = pattern; return true; }
 bool Apply_ValtanCompositionDraftTransaction(const std::function<bool(std::string&)>& edit, std::string& status) {
  const auto saved = *this; if (edit(status)) return true; *this = saved; return false;
 }
 bool Remove_ValtanStageEffectCue(const std::string&, const std::string& stageId, const std::string&, const std::string&, const std::string& occurrenceId, const std::string&, const std::string&, std::string& status) {
  for (auto& stage : pattern.Stages) if (stage.strStageId == stageId) {
   if (std::erase_if(stage.ProductCues, [&](const auto& cue) { return cue.strOccurrenceId == occurrenceId; }) != 1) return false;
   dirty = true; ++generation;
   if (failEffect) { status = "injected V1 failure after mutation"; return false; } return true;
  } return false;
 }
};
struct CAnimation_Tool {
 VALTAN_PATTERN_SOUND_CUE_DOCUMENT sounds; bool dirty = true, fail = false; uint64_t generation = 11;
 bool Apply_ValtanCompositionPatternSoundDraftTransaction(const std::function<bool(std::string&)>& edit, std::string& status) {
  const auto saved = *this; if (edit(status)) return true; *this = saved; return false;
 }
 bool Stage_ValtanCompositionPatternSoundCascadeForAnimationDelete(const VALTAN_PATTERN_VIEW& pattern, const VALTAN_STAGE_VIEW& stage, const std::string& clip,
  VALTAN_PATTERN_SOUND_CUE_DOCUMENT& previous, bool& wasDirty, uint64_t& next, size_t& count, std::string& status) {
  previous = sounds; wasDirty = dirty;
  count = std::erase_if(sounds.rows, [&](const auto& row) { return row.pattern == pattern.strPatternId && row.stage == stage.strStageId && row.clip == clip; });
  dirty = true; next = ++generation;
  if (fail) { status = "injected Sound failure after mutation"; return false; } return true;
 }
};
struct CValtanActionWorkbench {
 enum class DETAIL_OWNER { GAMEPLAY_STAGE, ANIMATION };
 CAnimation_Tool* m_pAnimationTool; CBalanceTool* m_pBalanceTool;
 bool m_bPatternShakesReady = true, m_bAuthoringDraftDirty = true;
 ShakeDocument m_PatternShakes;
 std::string m_strShakeStatus, m_strSelectedStableId = "clip";
 DETAIL_OWNER m_eDetailOwner = DETAIL_OWNER::ANIMATION;
 uint64_t m_iEffectV2CatalogRevision = 7;
 int invalidations = 0;
 void Invalidate_TimelineCache() { ++invalidations; }
 bool Remove_AnimationOccurrence(const VALTAN_PATTERN_VIEW&, const VALTAN_STAGE_VIEW&, const std::string&, std::string&);
};
}
using namespace Client;
bool ComputeExactAnimationWallMs(const CBalanceTool::PATTERN_STAGE_EDIT& draft, uint32_t& result) { result = 0; for (const auto& clip : draft.animationSlots) result += clip.playMs; return true; }
bool SetValtanStageDraftWithSoundDependencyAdmission(CAnimation_Tool* sound, CBalanceTool* balance, const ShakeDocument* shake,
 const VALTAN_PATTERN_VIEW& pattern, const VALTAN_STAGE_VIEW& source, const CBalanceTool::PATTERN_STAGE_EDIT& draft, std::string& status) {
 const auto retained = [&](const auto& id) { return std::any_of(draft.animationSlots.begin(), draft.animationSlots.end(), [&](const auto& clip) { return clip.clipOccurrenceId == id; }); };
 for (const auto& cue : source.ProductCues) if (!cue.bUsesStageClock && !retained(cue.strClipOccurrenceId)) { status = "orphan V1 cue"; return false; }
 for (const auto& row : sound->sounds.rows) if (row.pattern == pattern.strPatternId && row.stage == source.strStageId && !retained(row.clip)) { status = "orphan Sound"; return false; }
 for (const auto& row : shake->rows) if (row.pattern == pattern.strPatternId && row.stage == source.strStageId && !retained(row.clip)) { status = "orphan Shake"; return false; }
 for (auto& stage : balance->pattern.Stages) if (stage.strStageId == source.strStageId) {
  if (draft.productCues != stage.ProductCues) { status = "joined inventory is read-only"; return false; }
  stage.clips = draft.animationSlots; stage.policy = draft.animationEndPolicy; stage.repeat = draft.animationRepeatCount; balance->dirty = true; ++balance->generation;
  if (balance->failStage) { status = "injected final Stage failure after mutation"; return false; } return true;
 } return false;
}
// @PRODUCTION_ANIMATION_DELETE@
int checks = 0;
void check(bool value, const char* label) { ++checks; if (!value) { std::cerr << "FAIL " << label << '\n'; std::exit(1); } }
struct Fixture {
 CBalanceTool balance; CAnimation_Tool sound; CValtanActionWorkbench tool{&sound, &balance}; std::string status;
 Fixture() {
  CEffectV2Catalog::Get() = CEffectV2Catalog{};
  VALTAN_STAGE_VIEW stage; stage.clips = {{"clip", 500}, {"kept.clip", 500}};
  stage.ProductCues = {{"v1", "v1.1", "effect", "clip", false}, {"stage.v1", "stage.v1.1", "independent", "", true}, {"other.v1", "other.v1.1", "other", "kept.clip", false}};
  balance.pattern.Stages.push_back(stage);
  const auto& patternId = balance.pattern.strPatternId;
  sound.sounds.rows = {{patternId, stage.strStageId, "clip"}, {patternId, stage.strStageId, "kept.clip"}, {"other.pattern", stage.strStageId, "clip"}};
  tool.m_PatternShakes.rows = sound.sounds.rows;
  CEffectV2Catalog::Get().snapshot.rows = {
   {patternId, stage.strStageId, stage.strActionId, "binding.valtan.project-tuned.terrain-destruction-3.impact", "clip"},
   {patternId, stage.strStageId, stage.strActionId, "stage.v2", "", EFFECT_V2_CLOCK_BASIS::STAGE},
   {patternId, stage.strStageId, stage.strActionId, "other.v2", "kept.clip"}};
 }
 bool run() { return tool.Remove_AnimationOccurrence(balance.pattern, balance.pattern.Stages.front(), tool.m_strSelectedStableId, status); }
};
int main() {
 {
  Fixture f; const auto before = f.balance.pattern.Stages[0]; check(f.run(), f.status.c_str()); const auto& stage = f.balance.pattern.Stages[0];
  check(stage.clips.size() == 1 && stage.clips[0].clipOccurrenceId == "kept.clip", "remove only selected Animation");
  check(stage.ProductCues.size() == 2 && stage.ProductCues[0].bUsesStageClock, "cascade V1 but retain Stage and other clip cues");
  check(CEffectV2Catalog::Get().snapshot.rows.size() == 2 && CEffectV2Catalog::Get().snapshot.rows[0].eClockBasis == EFFECT_V2_CLOCK_BASIS::STAGE, "cascade V2 but retain independent scope");
  check(f.sound.sounds.rows.size() == 2 && f.tool.m_PatternShakes.rows.size() == 2, "cascade exact Sound and Shake while retaining other Pattern");
  check(stage.durationMs == before.durationMs && stage.hitOffsetMs == before.hitOffsetMs && stage.worldEvent == before.worldEvent && stage.motion == before.motion, "preserve Server Stage clock hit world motion");
  check(stage.policy == "HOLD_LAST_POSE" && f.tool.m_strSelectedStableId == "kept.clip" && f.tool.invalidations == 1, "retained Animation timing and focus refresh");
 }
 for (int failure = 0; failure != 4; ++failure) {
  Fixture f; f.balance.failEffect = failure == 0; CEffectV2Catalog::Get().fail = failure == 1; f.sound.fail = failure == 2; f.balance.failStage = failure == 3;
  const auto pattern = f.balance.pattern; const auto sounds = f.sound.sounds; const auto shakes = f.tool.m_PatternShakes; const auto bindings = CEffectV2Catalog::Get().snapshot.rows;
  check(!f.run(), "late owner failure rejects whole Delete");
  check(f.balance.pattern == pattern && f.sound.sounds == sounds && f.tool.m_PatternShakes == shakes && CEffectV2Catalog::Get().snapshot.rows == bindings, "restore all prior owner bytes");
  check(f.balance.generation == 9 && f.sound.generation == 11 && CEffectV2Catalog::Get().revision == 7 && f.balance.dirty && f.sound.dirty && CEffectV2Catalog::Get().dirty, "preserve pre-existing dirty state and generations");
  check(f.tool.m_strSelectedStableId == "clip" && f.tool.invalidations == 0 && f.tool.m_iEffectV2CatalogRevision == 7, "failure keeps selection and cache identity");
 }
 {
  Fixture f; auto& stage = f.balance.pattern.Stages[0]; stage.clips.resize(1); stage.ProductCues.pop_back(); f.sound.sounds.rows.erase(f.sound.sounds.rows.begin() + 1); f.tool.m_PatternShakes.rows.erase(f.tool.m_PatternShakes.rows.begin() + 1); CEffectV2Catalog::Get().snapshot.rows.pop_back();
  check(f.run(), f.status.c_str()); check(stage.clips.empty() && stage.policy == "NONE" && stage.repeat == 0 && stage.durationMs == 1000, "last slot becomes NONE without removing gameplay Stage");
  check(f.tool.m_strSelectedStableId == stage.strStageId && f.tool.m_eDetailOwner == CValtanActionWorkbench::DETAIL_OWNER::GAMEPLAY_STAGE, "last slot focuses retained Stage");
 }
 {
  Fixture f; CEffectV2Catalog::Get().snapshot.rows[0].strActionId = "stale.action"; const auto prior = f.balance.pattern;
  check(!f.run() && f.balance.pattern == prior && f.sound.generation == 11, "stale V2 owner refuses before any cascade");
 }
 std::cout << "PASS " << checks << " Animation Delete transaction checks\n";
}
