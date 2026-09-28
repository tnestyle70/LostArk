"""Exercise the actual Pattern-end Effect append transaction without opening Client."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from Tools.ValtanPipeline.test_valtan_sequence_append_native import ROOT, toolchain

PREAMBLE = r'''
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using bool_t=bool;
namespace Client {
enum class EFFECT_FOLLOW_POLICY { FOLLOW };
enum class EFFECT_STOP_POLICY { NATURAL };
enum class VALTAN_PATTERN_EFFECT_SCALE_POLICY { OWNER_RELATIVE };
struct VALTAN_CLIP_OCCURRENCE_VIEW { std::string strClipOccurrenceId, strClipName; uint32_t iSourceStartMs=0; };
struct VALTAN_PRODUCT_EFFECT_CUE_VIEW {
 std::string strBindingId,strOccurrenceId,strPatternId,strStageId,strActionId,strEffectAssetId,strAnchorSlotId,strFollowPolicy,strStopPolicy,strRepeatPolicy,strScalePolicy,strClipOccurrenceId;
 bool bUsesStageClock=false,bHasExplicitScalePolicy=false;uint32_t iStageOffsetMs=0,iSourceStartMs=0,iStageDurationMs=0;
 EFFECT_FOLLOW_POLICY eFollowPolicy{};EFFECT_STOP_POLICY eStopPolicy{};VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy{};
};
struct VALTAN_STAGE_VIEW { std::string strStageId,strActionId,strAnimationEndPolicy="EXACT"; uint32_t iDurationMs=1000; std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues; };
struct VALTAN_PATTERN_VIEW {bool bManualServerAudition=true;std::string strPatternId="P";std::vector<VALTAN_STAGE_VIEW> Stages;};
struct EFFECT_V2_BINDING {std::string strGroupId,strEffectId,strBindingId,strPatternId,strStageId,strActionId;uint32_t iStartMs=0;};
struct EFFECT_DOCUMENT_DESC {uint32_t span=3400;};
struct CEffectCatalog {static inline bool available=true; static inline EFFECT_DOCUMENT_DESC doc;static const EFFECT_DOCUMENT_DESC* Find(const std::string&){return available?&doc:nullptr;}};
struct EFFECT_V2_CATALOG_SNAPSHOT {std::vector<EFFECT_V2_BINDING> rows;bool Is_Ready()const{return true;}bool Can_MutateBossValtanBindings()const{return true;}const auto& Get_BossValtanBindings()const{return rows;}};
struct CEffectV2Catalog {
 std::shared_ptr<EFFECT_V2_CATALOG_SNAPSHOT> snapshot=std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();bool reject=false;int revision=0;
 static CEffectV2Catalog& Get(){static CEffectV2Catalog c;return c;} auto Get_Snapshot(){return snapshot;}
 template<class F> bool Apply_BossValtanBindingDraftTransaction(F fn,std::string&status){auto saved=*snapshot;auto rev=revision;if(fn(status))return true;*snapshot=saved;revision=rev;return false;}
 bool Stage_AppendBossValtanStageBinding(const std::string&id,bool group,const std::string&p,const std::string&s,const std::string&a,uint32_t start,std::string& status){if(reject){status="V2 reject";return false;}EFFECT_V2_BINDING b;b.strPatternId=p;b.strStageId=s;b.strActionId=a;b.iStartMs=start;b.strBindingId="v2."+s;(group?b.strGroupId:b.strEffectId)=id;snapshot->rows.push_back(b);++revision;return true;}
 int Get_Revision()const{return revision;}
};
struct CBalanceTool {
 struct PATTERN_STAGE_EDIT {uint32_t durationMs=0;std::string animationEndPolicy;std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> animationSlots;};
 bool Get_ValtanStageDraft(const std::string&,const std::string&s,PATTERN_STAGE_EDIT&d,std::string&){for(const auto&stage:pattern.Stages)if(stage.strStageId==s){d.durationMs=stage.iDurationMs;d.animationEndPolicy=stage.strAnimationEndPolicy;d.animationSlots=stage.ClipOccurrences;return true;}return false;}
 VALTAN_PATTERN_VIEW pattern;bool rejectCue=false,rejectInsert=false,topology=true;int reads=0,rejectRead=0,generation=0;
 template<class F> bool Apply_ValtanCompositionDraftTransaction(F fn,std::string&status){auto saved=pattern;int g=generation;if(fn(status))return true;pattern=saved;generation=g;return false;}
 bool Get_ValtanPatternDraft(const std::string&id,VALTAN_PATTERN_VIEW&out,std::string&status){if(++reads==rejectRead||id!=pattern.strPatternId){status="draft unavailable";return false;}out=pattern;return true;}
 bool Can_Edit_ValtanManualStageTopology(const std::string&,std::string&){return topology;}
 bool Insert_ValtanManualStageAfter(const std::string&,const std::string&after,const std::string&id,const std::string&action,const std::string&role,uint32_t duration,std::string&){if(rejectInsert||pattern.Stages.size()>=64||after!=pattern.Stages.back().strStageId||role!="ACTIVE")return false;VALTAN_STAGE_VIEW s;s.strStageId=id;s.strActionId=action;s.iDurationMs=duration;pattern.Stages.push_back(s);++generation;return true;}
 bool Add_ValtanStageEffectCue(const std::string&,const std::string&s,const std::string&,const VALTAN_PRODUCT_EFFECT_CUE_VIEW&cue,std::string&){if(rejectCue)return false;for(auto&stage:pattern.Stages)if(stage.strStageId==s){stage.ProductCues.push_back(cue);++generation;return true;}return false;}
 bool Is_ValtanDraftDirty()const{return generation>0;}
};
struct CValtanActionWorkbench {
 enum class EFFECT_RESOURCE_KIND {V1_PATTERN,V2_LEAF,V2_GROUP};enum class DETAIL_OWNER {EFFECT};enum class TIMELINE_LANE {ANIMATION};
 struct ITEM {TIMELINE_LANE eLane{};std::string strPatternId,strStageId,strStableId;uint32_t iStartMs=0,iEndMs=1000;};
 bool m_bPatternShakesReady=true;int m_PatternShakes=0;
 CBalanceTool* m_pBalanceTool=nullptr;void* m_pAnimationTool=nullptr;EFFECT_RESOURCE_KIND m_eEffectAddResourceKind=EFFECT_RESOURCE_KIND::V1_PATTERN;
 std::string m_strEffectAddAssetId="effect.saved",m_strEffectEditIdentity,m_strSelectedPatternId="P",m_strSelectedStageId="STEP_08",m_strSelectedStableId="old.selection";
 bool m_bDetailsWindowVisible=false,m_bEffectAddUsesStageClock=false,m_bAuthoringDraftDirty=false,m_bCompositionResourcesFocused=false;
 uint32_t m_iEffectV2AddStartMs=600000,m_iEffectV2CatalogRevision=0,m_iPlayheadMs=0;int invalidations=0,previews=0;
 std::string m_strResourceTargetPatternId="P",m_strResourceTargetStageId="STEP_01",m_strResourceTargetClipOccurrenceId,m_strEffectAddClipOccurrenceId,m_strSoundAddClipOccurrenceId;
 std::vector<ITEM> m_TimelineItems;
 bool Append_EffectResourceAtPatternEnd(const std::string&,std::string&);
 void Render_ResourcesPane(const VALTAN_PATTERN_VIEW*,const VALTAN_STAGE_VIEW*,bool,bool);
 void Select_Stage(const VALTAN_PATTERN_VIEW&p,const VALTAN_STAGE_VIEW&s,DETAIL_OWNER,const std::string&id){m_strSelectedPatternId=p.strPatternId;m_strSelectedStageId=s.strStageId;m_strSelectedStableId=id;}
 void Invalidate_EffectivePatternCache(){++invalidations;}void Invalidate_TimelineCache(){++invalidations;}
 void Refresh_PatternLocalPreviewAfterMutation(const VALTAN_PATTERN_VIEW*,std::string&){++previews;}
};
}
using namespace Client;
bool SetValtanStageDraftWithSoundDependencyAdmission(void*,CBalanceTool*b,const int*,const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW&s,const CBalanceTool::PATTERN_STAGE_EDIT&d,std::string&){for(auto&stage:b->pattern.Stages)if(stage.strStageId==s.strStageId){stage.iDurationMs=d.durationMs;stage.strAnimationEndPolicy=d.animationEndPolicy;++b->generation;return true;}return false;}
uint32_t ResolveEffectDocumentSpanMs(const EFFECT_DOCUMENT_DESC&d){return d.span;}
struct SPAN {double fMilliseconds=2500;bool bResolved=true,bUnbounded=false;};static SPAN span;
SPAN ResolveEffectV2ResourceSpanMs(const EFFECT_V2_CATALOG_SNAPSHOT&,const EFFECT_V2_BINDING&){return span;}
std::string BuildNextCompositionEffectCueId(const VALTAN_PATTERN_VIEW&p,const VALTAN_STAGE_VIEW&s){return p.strPatternId+"."+s.strStageId+".cue";}
std::string BuildEffectV2BindingStableId(const EFFECT_V2_BINDING&b){return "effect-v2/"+b.strBindingId;}
bool ValidateValtanEffectInvocationClock(void*,const VALTAN_STAGE_VIEW&s,const VALTAN_PRODUCT_EFFECT_CUE_VIEW&c,std::string&){return c.bUsesStageClock&&c.strClipOccurrenceId.empty()&&c.iStageOffsetMs<s.iDurationMs&&c.iStageDurationMs==s.iDurationMs;}
constexpr int ImGuiFocusedFlags_RootAndChildWindows=1;
namespace ImGui {std::string displayed;bool IsWindowFocused(int){return true;}void TextDisabled(const char*,const char*stage){displayed=stage;}}
'''

MAIN = r'''
int main(){
 int checks=0;auto check=[&](bool ok){++checks;if(!ok)throw std::runtime_error("check "+std::to_string(checks));};
 CBalanceTool balance;VALTAN_STAGE_VIEW first;first.strStageId="STEP_08";first.strActionId="action.8";first.iDurationMs=1800;first.ClipOccurrences.push_back({"clip8","clip",15});
 auto last=first;last.strStageId="STEP_10";last.strActionId="action.10";last.iDurationMs=4800;last.ClipOccurrences.front().strClipOccurrenceId="clip10";
 balance.pattern.Stages={first,last};CValtanActionWorkbench wb;wb.m_pBalanceTool=&balance;std::string status;
 wb.Render_ResourcesPane(&balance.pattern,&first,true,true);
 check(ImGui::displayed=="STEP_08"&&wb.m_strResourceTargetStageId=="STEP_08");
 wb.m_strSelectedStageId="STEP_10";wb.Render_ResourcesPane(&balance.pattern,&first,true,true);
 check(ImGui::displayed=="STEP_10"&&wb.m_strEffectAddClipOccurrenceId=="clip10");
 wb.m_strResourceTargetPatternId="old.pattern";wb.Render_ResourcesPane(&balance.pattern,&first,true,true);check(wb.m_strResourceTargetPatternId=="P");
 wb.m_strSelectedStageId="deleted";wb.Render_ResourcesPane(&balance.pattern,&first,true,true);check(ImGui::displayed=="select a Stage");
 check(ResolveEffectAppendClip(last,"deleted","clip10")->strClipOccurrenceId=="clip10");
 check(ResolveEffectAppendClip(last,"deleted","nothing")->strClipOccurrenceId=="clip10");
 check(wb.Append_EffectResourceAtPatternEnd("P",status));
 check(balance.pattern.Stages.size()==3&&balance.pattern.Stages.back().strStageId=="EFFECT_APPEND_1");
 const auto& added=balance.pattern.Stages.back();check(added.ClipOccurrences.empty()&&added.iDurationMs==3400);
 check(added.ProductCues.size()==1&&added.ProductCues[0].bUsesStageClock&&added.ProductCues[0].strClipOccurrenceId.empty());
 check(added.ProductCues[0].iStageOffsetMs==0&&added.ProductCues[0].strEffectAssetId=="effect.saved");
 check(status.find("6600 ms")!=std::string::npos&&wb.m_iEffectV2AddStartMs==0&&wb.m_bEffectAddUsesStageClock);
 check(wb.m_strSelectedStageId=="EFFECT_APPEND_1"&&wb.previews==1);
 check(balance.pattern.Stages[0].ClipOccurrences[0].strClipOccurrenceId=="clip8"&&balance.pattern.Stages[1].iDurationMs==4800);
 balance.rejectCue=true;auto oldSize=balance.pattern.Stages.size();auto selected=wb.m_strSelectedStageId;auto generation=balance.generation;
 check(!wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==oldSize&&balance.generation==generation&&wb.m_strSelectedStageId==selected);balance.rejectCue=false;
 CEffectCatalog::available=false;check(!wb.Append_EffectResourceAtPatternEnd("P",status)&&balance.pattern.Stages.size()==oldSize);CEffectCatalog::available=true;
 balance.topology=false;check(!wb.Append_EffectResourceAtPatternEnd("P",status)&&balance.pattern.Stages.size()==oldSize);balance.topology=true;
 wb.m_eEffectAddResourceKind=CValtanActionWorkbench::EFFECT_RESOURCE_KIND::V2_GROUP;wb.m_strEffectAddAssetId="boss.valtan.group";wb.m_iEffectV2AddStartMs=999999;
 check(wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==4&&balance.pattern.Stages.back().iDurationMs==2500);
 auto& catalog=CEffectV2Catalog::Get();check(catalog.snapshot->rows.size()==1);check(catalog.snapshot->rows.back().iStartMs==0&&catalog.snapshot->rows.back().strStageId=="EFFECT_APPEND_2");
 check(wb.m_strSelectedStableId=="effect-v2/v2.EFFECT_APPEND_2");
 oldSize=balance.pattern.Stages.size();auto oldRows=catalog.snapshot->rows.size();auto oldRevision=catalog.revision;generation=balance.generation;selected=wb.m_strSelectedStageId;
 balance.rejectRead=balance.reads+3;check(!wb.Append_EffectResourceAtPatternEnd("P",status));
 check(balance.pattern.Stages.size()==oldSize&&catalog.snapshot->rows.size()==oldRows&&catalog.revision==oldRevision&&balance.generation==generation&&wb.m_strSelectedStageId==selected);balance.rejectRead=0;
 catalog.reject=true;check(!wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==oldSize&&catalog.snapshot->rows.size()==oldRows);catalog.reject=false;
 span.bResolved=false;check(!wb.Append_EffectResourceAtPatternEnd("P",status)&&balance.pattern.Stages.size()==oldSize);span.bResolved=true;
 span.bUnbounded=true;check(wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.back().iDurationMs==1000);span.bUnbounded=false;
 wb.m_strSelectedStageId="STEP_08";check(wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.back().strStageId=="EFFECT_APPEND_4");
 balance.pattern.Stages.resize(64,first);check(!wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==64);
 balance.pattern.Stages={first,last};balance.pattern.bManualServerAudition=false;balance.topology=false;
 wb.m_eEffectAddResourceKind=CValtanActionWorkbench::EFFECT_RESOURCE_KIND::V1_PATTERN;wb.m_strEffectAddAssetId="effect.saved";
 check(wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==2);
 check(balance.pattern.Stages.back().strStageId=="STEP_10"&&balance.pattern.Stages.back().iDurationMs==8200);
 check(balance.pattern.Stages.back().strAnimationEndPolicy=="HOLD_LAST_POSE");
 check(balance.pattern.Stages.back().ProductCues.back().iStageOffsetMs==4800);
 check(balance.pattern.Stages.back().ClipOccurrences.front().strClipOccurrenceId=="clip10");
 balance.rejectCue=true;generation=balance.generation;check(!wb.Append_EffectResourceAtPatternEnd("P",status));
 check(balance.pattern.Stages.back().iDurationMs==8200&&balance.generation==generation&&balance.pattern.Stages.back().ProductCues.size()==1);balance.rejectCue=false;
 wb.m_eEffectAddResourceKind=CValtanActionWorkbench::EFFECT_RESOURCE_KIND::V2_GROUP;wb.m_strEffectAddAssetId="boss.valtan.group";
 check(wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.size()==2&&balance.pattern.Stages.back().iDurationMs==10700);
 check(catalog.snapshot->rows.back().iStartMs==8200&&catalog.snapshot->rows.back().strStageId=="STEP_10");
 balance.pattern.Stages.back().iDurationMs=600000;check(!wb.Append_EffectResourceAtPatternEnd("P",status));check(balance.pattern.Stages.back().iDurationMs==600000);
 std::cout<<"Effect Pattern-end append: "<<checks<<" checks PASS\n";
}
'''

class TestEffectAppendNative(unittest.TestCase):
    def test_actual_append_and_resource_target_recovery(self):
        source = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(encoding="utf-8")
        start = source.index("bool_t Client::CValtanActionWorkbench::Append_EffectResourceAtPatternEnd(")
        end = source.index("void Client::CValtanActionWorkbench::Render_ResourcesPane(", start)
        append = source[start:end]
        resource_start = end
        resource_end = source.index("\tconst auto ResourceTabFlags", resource_start)
        resources = source[resource_start:resource_end] + "}\n"
        helper_start = source.index("\tconst Client::VALTAN_CLIP_OCCURRENCE_VIEW* ResolveEffectAppendClip(")
        helper_end = source.index("\tvoid InsertEffectResourcePaths", helper_start)
        code = PREAMBLE + source[helper_start:helper_end] + append + resources + MAIN
        with tempfile.TemporaryDirectory(prefix="ValtanEffectAppend-") as directory:
            out = Path(directory)
            cl, environment = toolchain(out)
            probe = out / "probe.cpp"
            probe.write_text(code, encoding="utf-8")
            binary = out / "probe.exe"
            result = subprocess.run([str(cl), "/nologo", "/EHsc", "/std:c++20", "/MDd", str(probe),
                "/Fo" + str(out / "probe.obj"), "/Fe" + str(binary)], env=environment, capture_output=True)
            self.assertEqual(0, result.returncode, (result.stdout + result.stderr).decode(errors="replace"))
            result = subprocess.run([str(binary)], capture_output=True)
            self.assertEqual(0, result.returncode, (result.stdout + result.stderr).decode(errors="replace"))
            self.assertIn(b"checks PASS", result.stdout)
            print(result.stdout.decode().strip())

if __name__ == "__main__":
    unittest.main()
