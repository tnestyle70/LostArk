// Actual Workbench routing, deterministic source owners. Native clip cut validation
// and physical persistence are covered separately by sequence/source tests.
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using bool_t = bool;
namespace Client {
struct VALTAN_CLIP_OCCURRENCE_VIEW { std::string strClipOccurrenceId; uint32_t playMs=411; bool loop=false; bool operator==(const VALTAN_CLIP_OCCURRENCE_VIEW&) const = default; };
struct VALTAN_STAGE_VIEW { std::string strStageId,strActionId; uint32_t iDurationMs=1000; std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences; bool operator==(const VALTAN_STAGE_VIEW&) const = default; };
struct VALTAN_PATTERN_VIEW { std::string strPatternId="fixture"; bool bManualServerAudition=true; std::vector<VALTAN_STAGE_VIEW> Stages; bool operator==(const VALTAN_PATTERN_VIEW&) const = default; };
struct COMPOSITION_ANIMATION_RESOURCE { std::string strTargetAssetName="Valtan",strModelAssetId="Character/Valtan.model",strSourceAssetId="Character/Valtan.anim",strRuntimeClip="clip"; uint32_t iDurationMs=500; };
struct CAnimation_Tool { bool blocked=false; bool Is_ValtanCompositionPatternTransactionActive() const { return blocked; } };
struct CBalanceTool {
    VALTAN_PATTERN_VIEW pattern; bool blocked=false,publishing=false,dirty=false,failInsert=false;
    unsigned generation=0,transactions=0,appends=0; mutable std::string lastCanStage;
    bool Is_ValtanSaveJobBlockingAuthoring() const{return blocked;} bool Is_ServerRuntimeSetPublishRunning()const{return publishing;}
    bool Is_ValtanDraftDirty()const{return dirty;} unsigned Get_ValtanDraftGeneration()const{return generation;}
    bool Get_ValtanAuthoringState(std::string& revision,bool& changed,std::string&)const{revision="authoring";changed=dirty;return true;}
    bool Get_ValtanCanonicalSourceRevision(std::string& revision,std::string&)const{revision="canonical";return true;}
    bool Get_ValtanPatternDraft(const std::string& id,VALTAN_PATTERN_VIEW& output,std::string&)const{if(id!=pattern.strPatternId)return false;output=pattern;return true;}
    bool Can_Edit_ValtanManualStageTopology(const std::string& id,std::string&)const{return id==pattern.strPatternId&&pattern.bManualServerAudition;}
    bool Insert_ValtanManualStageAfter(const std::string& patternId,const std::string& anchor,const std::string& id,const std::string& action,const std::string&,uint32_t duration,std::string& status){
        if(failInsert||pattern.Stages.size()>=64){status="stage limit/owner failure";return false;}
        auto at=std::find_if(pattern.Stages.begin(),pattern.Stages.end(),[&](const auto& row){return row.strStageId==anchor;});
        if(patternId!=pattern.strPatternId||at==pattern.Stages.end())return false;
        pattern.Stages.insert(at+1,{id,action,duration,{}});dirty=true;++generation;return true;
    }
    bool Can_AppendValtanAnimationClip(const std::string& patternId,const std::string& stageId,const std::string&,uint32_t duration,bool asNewStage,std::string&)const{
        lastCanStage=stageId;return !blocked&&!publishing&&pattern.Stages.size()<64&&patternId==pattern.strPatternId&&stageId==pattern.Stages.back().strStageId&&asNewStage==pattern.bManualServerAudition&&duration>0&&(asNewStage||std::none_of(pattern.Stages.back().ClipOccurrences.begin(),pattern.Stages.back().ClipOccurrences.end(),[](const auto& row){return row.loop||row.playMs==0;}));
    }
    bool Append_ValtanAnimationClip(const std::string& patternId,const std::string& stageId,const std::string&,uint32_t duration,bool asNewStage,VALTAN_PATTERN_VIEW& output,std::string& newStage,std::string& occurrence,std::string& status){
        if(!Can_AppendValtanAnimationClip(patternId,stageId,"clip",duration,asNewStage,status))return false;
        newStage=asNewStage?"RAW_"+std::to_string(++appends):stageId;occurrence=newStage+"-clip-added";
        if(asNewStage&&!Insert_ValtanManualStageAfter(patternId,stageId,newStage,newStage+"-action","ACTIVE",duration,status))return false;
        if(!asNewStage){pattern.Stages.back().iDurationMs+=duration;dirty=true;++generation;}
        pattern.Stages.back().ClipOccurrences.push_back({occurrence,duration,false});output=pattern;return true;
    }
    bool Apply_ValtanCompositionDraftTransaction(const std::function<bool(std::string&)>& operation,std::string& status){
        ++transactions;auto baseline=pattern;const auto oldDirty=dirty;const auto oldGeneration=generation;
        if(operation(status))return true;pattern=baseline;dirty=oldDirty;generation=oldGeneration;return false;
    }
};
class CValtanActionWorkbench {
public:
    enum class DETAIL_OWNER{NONE,GAMEPLAY_STAGE,ANIMATION};
    struct TIMELINE_SELECTION{std::string strPatternId,strStageId,strStableId;DETAIL_OWNER eOwner=DETAIL_OWNER::NONE;bool operator==(const TIMELINE_SELECTION&)const=default;};
    struct PENDING_RESOURCE_APPEND{COMPOSITION_ANIMATION_RESOURCE Resource;bool bAsNewStage=false;std::string strPatternId,strStageId;};
    struct Sequence{std::string strStableId="sequence";std::vector<unsigned> Clips={600,700,800};};
    CBalanceTool* m_pBalanceTool=nullptr;CAnimation_Tool* m_pAnimationTool=nullptr;int m_eAdmission=1;
    std::vector<Sequence> m_AnimationSequences={Sequence{}};
    std::string m_strSelectedSequenceStableId="sequence",m_strSelectedPatternId="fixture",m_strSelectedStageId="A",m_strSelectedStableId="A",m_strStatus,m_strEffectEditIdentity="prior";
    std::string m_strPinnedAuthoringSourceRevision="authoring",m_strPinnedCanonicalSourceRevision="canonical";
    DETAIL_OWNER m_eDetailOwner=DETAIL_OWNER::GAMEPLAY_STAGE;
    std::vector<TIMELINE_SELECTION> m_TimelineSelection={{"fixture","A","A",DETAIL_OWNER::GAMEPLAY_STAGE}};
    std::vector<int> m_BossPatternOutcomeOverrides={1};unsigned m_iBossPatternRouteGeneration=1,refreshes=0;
    bool m_bAuthoringDraftDirty=false,m_bWorkbenchFrameActive=false,failSequence=false;
    std::optional<PENDING_RESOURCE_APPEND> m_PendingResourceAppend;
    VALTAN_PATTERN_VIEW m_EffectivePatternCache;
    std::string m_strEffectivePatternCachePatternId,m_strEffectivePatternCacheCanonicalRevision;
    unsigned m_iEffectivePatternCacheDraftGeneration=0;bool m_bEffectivePatternCacheReady=false,m_bCompositionResourceDraftReady=false;
    void Invalidate_EffectivePatternCache(){} void Invalidate_TimelineCache(){}
    void Refresh_PatternLocalPreviewAfterMutation(const VALTAN_PATTERN_VIEW*,std::string&){++refreshes;}
    bool Apply_SelectedSequenceToStage(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW& stage,bool append){
        if(failSequence){m_strStatus="native clip validation failure";return false;}
        if(!append)throw std::runtime_error("Sequence must append");
        if(std::any_of(stage.ClipOccurrences.begin(),stage.ClipOccurrences.end(),[](const auto& row){return row.loop||row.playMs==0;})){m_strStatus="Unbounded final clip";return false;}
        auto& added=m_pBalanceTool->pattern.Stages.back();added.iDurationMs=0;for(const auto& clip:added.ClipOccurrences)added.iDurationMs+=clip.playMs;
        for(const auto duration:m_AnimationSequences.front().Clips){added.ClipOccurrences.push_back({stage.strStageId+"-clip"+std::to_string(added.ClipOccurrences.size()),duration,false});added.iDurationMs+=duration;}
        m_strSelectedStableId=added.ClipOccurrences.front().strClipOccurrenceId;m_eDetailOwner=DETAIL_OWNER::ANIMATION;
        ++m_pBalanceTool->generation;return true;
    }
    bool_t Append_SelectedSequenceToPattern(const VALTAN_PATTERN_VIEW&);
    bool Can_AppendCompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE&,bool,std::string&)const;
    bool Append_CompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE&,bool,std::string&);
    bool Apply_CompositionResourceAppend(const PENDING_RESOURCE_APPEND&,std::string&);
};
}
using namespace Client;
bool Can_MutateValtanView(int admission){return admission!=0;}
struct PreviewAsset{const char* pAssetName;const char* pModelAssetId;const char* pAnimationSetAssetId;};
constexpr std::array<PreviewAsset,1> ANIMATION_PREVIEW_ASSETS={PreviewAsset{"Valtan","Character/Valtan.model","Character/Valtan.anim"}};
// @SEQUENCE_END@
// @RAW_CAN@
// @RAW_APPEND@
// @RAW_APPLY@
unsigned checks=0;void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
struct Fixture{
    CBalanceTool balance;CAnimation_Tool animation;CValtanActionWorkbench ui;
    Fixture(){balance.pattern.Stages={{"A","action.A",1000,{{"A-clip",1000,false}}},{"LOOP","action.LOOP",3000,{{"last-loop",0,true}}}};ui.m_pBalanceTool=&balance;ui.m_pAnimationTool=&animation;}
    bool append(){const auto snapshot=balance.pattern;return ui.Append_SelectedSequenceToPattern(snapshot);}
};
int main(){try{
    std::string status;Fixture sequence;const auto before=sequence.balance.pattern;
    check(sequence.append(),sequence.ui.m_strStatus);
    check(sequence.balance.pattern.Stages.size()==3&&sequence.balance.pattern.Stages[0]==before.Stages[0]&&sequence.balance.pattern.Stages[1]==before.Stages[1],"Sequence ignores early selection and leaves final loop intact");
    check(sequence.balance.pattern.Stages.back().iDurationMs==2100&&sequence.balance.pattern.Stages.back().ClipOccurrences.size()==3,"Sequence owns exact end Stage with all clips");
    check(sequence.ui.m_strSelectedStageId==sequence.balance.pattern.Stages.back().strStageId&&sequence.ui.m_TimelineSelection.size()==4,"Appended Stage and clips selected together");
    const auto firstEnd=sequence.balance.pattern.Stages.back().strStageId;
    sequence.ui.m_strSelectedStageId="A";sequence.ui.m_strSelectedStableId="A";
    check(sequence.append()&&sequence.balance.pattern.Stages.size()==4&&sequence.balance.pattern.Stages[2].strStageId==firstEnd,"Repeated append uses latest end regardless focus");
    check(sequence.ui.refreshes==2&&sequence.balance.transactions==2,"One preview and transaction per Sequence append");
    Fixture failure;failure.ui.failSequence=true;const auto selected=failure.ui.m_TimelineSelection;
    check(!failure.append()&&failure.balance.pattern==before&&!failure.balance.dirty&&failure.balance.generation==0,"Native failure rolls back new end Stage");
    check(failure.ui.m_strSelectedStageId=="A"&&failure.ui.m_strSelectedStableId=="A"&&failure.ui.m_TimelineSelection==selected&&failure.ui.refreshes==0,"Failed append preserves UI selection");
    Fixture empty;empty.ui.m_AnimationSequences.clear();check(!empty.append()&&empty.balance.generation==0,"Absent Sequence does not create empty Stage");
    Fixture canonicalLoop;canonicalLoop.balance.pattern.bManualServerAudition=false;check(!canonicalLoop.append()&&canonicalLoop.balance.generation==0,"Canonical unbounded final loop preserves source");
    Fixture canonical;canonical.balance.pattern.bManualServerAudition=false;canonical.balance.pattern.Stages.back().ClipOccurrences.front()={"finite-tail",3000,false};
    check(canonical.append()&&canonical.balance.pattern.Stages.size()==2&&canonical.balance.pattern.Stages.back().iDurationMs==5100,"Canonical finite tail appends Sequence without adding a Stage");
    check(canonical.ui.m_TimelineSelection.size()==3&&canonical.ui.m_TimelineSelection.front().eOwner==CValtanActionWorkbench::DETAIL_OWNER::ANIMATION,"Canonical append selects only new Animation boxes");
    COMPOSITION_ANIMATION_RESOURCE canonicalResource;
    check(canonical.ui.Append_CompositionAnimationResource(canonicalResource,false,status)&&canonical.balance.pattern.Stages.size()==2&&canonical.balance.pattern.Stages.back().iDurationMs==5600,"Canonical raw append reuses final Stage and preserves topology");
    Fixture maximum;while(maximum.balance.pattern.Stages.size()<64)maximum.balance.pattern.Stages.push_back({"x"+std::to_string(maximum.balance.pattern.Stages.size()),"",1,{}});
    check(!maximum.append()&&maximum.balance.pattern.Stages.size()==64&&maximum.balance.generation==0,"64 Stage limit preserves source");
    Fixture blocked;blocked.balance.blocked=true;check(!blocked.append()&&blocked.balance.generation==0,"Active Save blocks Sequence append");
    Fixture raw;COMPOSITION_ANIMATION_RESOURCE resource;
    check(raw.ui.Can_AppendCompositionAnimationResource(resource,false,status)&&raw.balance.lastCanStage=="LOOP","Raw preflight resolves final Stage even for default row action");
    check(raw.ui.Append_CompositionAnimationResource(resource,false,status)&&raw.balance.pattern.Stages.back().strStageId=="RAW_1",status);
    check(raw.balance.pattern.Stages[0]==before.Stages[0]&&raw.balance.pattern.Stages[1]==before.Stages[1],"Raw append never extends selected or looping existing Stage");
    check(raw.ui.m_TimelineSelection.size()==2&&raw.ui.refreshes==1,"Raw end Stage and clip selected and preview refreshed");
    Fixture queued;queued.ui.m_bWorkbenchFrameActive=true;
    check(queued.ui.Append_CompositionAnimationResource(resource,false,status)&&queued.balance.generation==0&&queued.ui.m_PendingResourceAppend.has_value(),"Frame-active raw append queues without mutating");
    const auto command=*queued.ui.m_PendingResourceAppend;queued.ui.m_PendingResourceAppend.reset();
    queued.balance.pattern.Stages.push_back({"NEW_END","new.end",900,{}});
    check(queued.ui.Apply_CompositionResourceAppend(command,status)&&queued.balance.pattern.Stages[2].strStageId=="NEW_END"&&queued.balance.pattern.Stages[3].strStageId=="RAW_1","Deferred command resolves end after another edit");
    Fixture changed;changed.ui.m_strSelectedPatternId="other";
    check(!changed.ui.Apply_CompositionResourceAppend(command,status)&&changed.balance.generation==0,"Changed Pattern cancels stale queued command");
    Fixture foreign;resource.strTargetAssetName="Kouku";check(!foreign.ui.Append_CompositionAnimationResource(resource,false,status)&&foreign.balance.generation==0,"Foreign resource cannot append");
    std::cout<<checks<<" pattern end append checks passed\n";
}catch(const std::exception& error){std::cerr<<"After "<<checks<<" checks: "<<error.what()<<'\n';return 1;}}
