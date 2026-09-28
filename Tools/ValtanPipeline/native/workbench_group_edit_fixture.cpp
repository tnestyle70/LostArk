// Storage-only owner doubles around the actual extracted Workbench group function.
// This verifies orchestration and rollback; runtime/source setter validation has separate tests.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>
using bool_t = bool;
namespace Client {
struct VALTAN_CLIP_OCCURRENCE_VIEW { std::string strClipOccurrenceId; uint32_t iSourceStartMs=0, iAuthoringWallMs=0; float fPlayRate=1; };
struct VALTAN_PRODUCT_EFFECT_CUE_VIEW {
 std::string strPatternId,strStageId,strActionId,strBindingId,strOccurrenceId,strEffectAssetId,strClipOccurrenceId;
 bool bUsesStageClock=false,bHasSourceEnd=false,bHasPlaybackOffset=false;
 uint32_t iSourceStartMs=0,iSourceEndMs=0,iStageOffsetMs=0,iStageDurationMs=0,iPlaybackOffsetMs=0;
};
struct VALTAN_PATTERN_SOUND_CUE {
 std::string strPatternId,strStageId,strActionId,strBindingId,strOccurrenceId,strClipOccurrenceId,strSoundEvent;
 uint32_t iStartMs=0,iPlaybackOffsetMs=0,iPlaybackDurationMs=0; int eRepeatPolicy=0;
 bool bHasPlaybackOffset=false,bHasPlaybackDuration=false;
};
struct VALTAN_PATTERN_SOUND_CUE_ROW_ID {std::string strBindingId,strOccurrenceId;};
struct VALTAN_STAGE_VIEW {std::string strStageId,strActionId; uint32_t iDurationMs=1000; std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences; std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;};
struct VALTAN_PATTERN_VIEW {std::string strPatternId="fixture"; std::vector<VALTAN_STAGE_VIEW> Stages;};
enum class EFFECT_V2_CLOCK_BASIS {STAGE,CLIP_OCCURRENCE};
struct EFFECT_V2_BINDING {std::string strPatternId,strStageId,strActionId,strBindingId,strClipOccurrenceId; EFFECT_V2_CLOCK_BASIS eClockBasis=EFFECT_V2_CLOCK_BASIS::STAGE; uint32_t iStartMs=0;};
struct EFFECT_V2_STAGE_BINDING_KEY {std::string id; static auto From_Binding(const EFFECT_V2_BINDING& x){return EFFECT_V2_STAGE_BINDING_KEY{x.strBindingId};}};
struct EffectSnapshot {std::vector<EFFECT_V2_BINDING> bindings; bool Can_MutateBossValtanBindings()const{return true;} const auto& Get_BossValtanBindings()const{return bindings;}};
struct CEffectV2Catalog {
 std::shared_ptr<EffectSnapshot> snapshot=std::make_shared<EffectSnapshot>(); bool failAppend=false,failMove=false; unsigned revision=1;
 static auto& Get(){static CEffectV2Catalog value;return value;} auto Get_Snapshot(){return snapshot;} unsigned Get_Revision(){return revision;}
 bool Apply_BossValtanBindingDraftTransaction(const std::function<bool(std::string&)>& operation,std::string& status){auto old=*snapshot;auto rev=revision; if(operation(status))return true;*snapshot=old;revision=rev;return false;}
 bool Stage_RemoveBossValtanStageBinding(const EFFECT_V2_STAGE_BINDING_KEY& key,std::string&){auto count=std::erase_if(snapshot->bindings,[&](auto&x){return x.strBindingId==key.id;});revision+=count;return count==1;}
 bool Stage_AppendBossValtanBindings(const std::vector<EFFECT_V2_BINDING>& rows,std::vector<std::string>& ids,std::string& status){if(failAppend){status="injected V2 append failure";return false;}for(auto row:rows){row.strBindingId="new.v2."+std::to_string(++revision);ids.push_back(row.strBindingId);snapshot->bindings.push_back(row);}return true;}
 bool Stage_MoveBossValtanBinding(const EFFECT_V2_STAGE_BINDING_KEY& key,const std::string& stage,const std::string& action,const std::string& clip,uint32_t start,std::string& status){if(failMove){status="injected V2 move failure";return false;}for(auto& x:snapshot->bindings)if(x.strBindingId==key.id){x.strStageId=stage;x.strActionId=action;x.strClipOccurrenceId=clip;x.iStartMs=start;++revision;return true;}return false;}
};
struct CBalanceTool {
 struct PATTERN_STAGE_EDIT {std::string stageId,hitShape="CIRCLE",playerResponse="DAMAGE";uint32_t durationMs=2000;std::vector<int> attackContacts;std::vector<uint32_t> pulses{100,200};bool colliderRemoveAdmitted=true,hasHitActivation=false;};
 VALTAN_PATTERN_VIEW pattern; std::vector<PATTERN_STAGE_EDIT> colliders; bool failAnimationDelete=false; int failAnimationDeleteAt=0, animationDeleteCalls=0; bool dirty=false;
 bool Is_ValtanSaveJobBlockingAuthoring(){return false;}bool Is_ServerRuntimeSetPublishRunning(){return false;}bool Is_ValtanDraftDirty(){return dirty;}
 bool Get_ValtanPatternDraft(const std::string&,VALTAN_PATTERN_VIEW& out,std::string&){out=pattern;return true;}
 bool Get_ValtanStageDraft(const std::string&,const std::string& id,PATTERN_STAGE_EDIT& out,std::string&){for(auto&x:colliders)if(x.stageId==id){out=x;return true;}return false;}
 bool Set_ValtanStageDraft(const std::string&,const std::string& id,const PATTERN_STAGE_EDIT& in,std::string&){for(auto&x:colliders)if(x.stageId==id){x=in;dirty=true;return true;}return false;}
 bool Remove_ValtanStageEffectCue(const std::string&,const std::string& stage,const std::string&,const std::string&,const std::string& occurrence,const std::string&,const std::string&,std::string&){for(auto& s:pattern.Stages)if(s.strStageId==stage){auto n=std::erase_if(s.ProductCues,[&](auto&x){return x.strOccurrenceId==occurrence;});dirty|=n!=0;return n==1;}return false;}
 bool Add_ValtanStageEffectCue(const std::string&,const std::string& stage,const std::string&,const VALTAN_PRODUCT_EFFECT_CUE_VIEW& value,std::string&){for(auto&s:pattern.Stages)if(s.strStageId==stage){s.ProductCues.push_back(value);dirty=true;return true;}return false;}
 bool Apply_ValtanCompositionDraftTransaction(const std::function<bool(std::string&)>& operation,std::string& status){auto old=pattern;auto coll=colliders;auto d=dirty;if(operation(status))return true;pattern=old;colliders=coll;dirty=d;return false;}
};
struct CAnimation_Tool {
 struct SoundDocument {std::vector<VALTAN_PATTERN_SOUND_CUE> Cues;} sounds;bool dirty=false;unsigned next=0;
 bool Is_ValtanCompositionPatternTransactionActive(){return false;}
 const auto* Get_ValtanCompositionPatternSoundDraft(bool& d,std::string&){d=dirty;return &sounds;}
 bool Remove_ValtanCompositionPatternSound(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW&,const VALTAN_PATTERN_SOUND_CUE_ROW_ID& id,std::string&){auto n=std::erase_if(sounds.Cues,[&](auto&x){return x.strOccurrenceId==id.strOccurrenceId;});dirty|=n!=0;return n==1;}
 bool Add_ValtanCompositionPatternSound(const VALTAN_PATTERN_VIEW& p,const VALTAN_STAGE_VIEW& stage,const std::string& clip,const std::string& event,uint32_t at,int repeat,VALTAN_PATTERN_SOUND_CUE_ROW_ID& id,std::string&,std::optional<uint32_t> offset,std::optional<uint32_t> duration){VALTAN_PATTERN_SOUND_CUE v;v.strPatternId=p.strPatternId;v.strStageId=stage.strStageId;v.strActionId=stage.strActionId;v.strClipOccurrenceId=clip;v.strSoundEvent=event;v.iStartMs=at;v.eRepeatPolicy=repeat;v.bHasPlaybackOffset=offset.has_value();v.iPlaybackOffsetMs=offset.value_or(0);v.bHasPlaybackDuration=duration.has_value();v.iPlaybackDurationMs=duration.value_or(0);v.strOccurrenceId="new.sound."+std::to_string(++next);id.strOccurrenceId=v.strOccurrenceId;sounds.Cues.push_back(v);dirty=true;return true;}
 bool Apply_ValtanCompositionPatternSoundDraftTransaction(const std::function<bool(std::string&)>& operation,std::string& status){auto old=sounds;auto d=dirty;auto n=next;if(operation(status))return true;sounds=old;dirty=d;next=n;return false;}
};
enum class COMPOSITION_EDIT_COMMAND {DUPLICATE_SELECTION};
struct CValtanActionWorkbench {
 enum class DETAIL_OWNER{GAMEPLAY_STAGE,ANIMATION,EFFECT,SOUND};enum class TIMELINE_LANE{STAGE,ANIMATION,EFFECT,SOUND,COLLIDER};enum class TIMELINE_GROUP_EDIT{MOVE_BOXES,DELETE_BOXES,DUPLICATE_BOXES};
 struct TIMELINE_SELECTION{std::string strPatternId,strStageId,strStableId;DETAIL_OWNER eOwner{};bool operator==(const TIMELINE_SELECTION&)const=default;};
 struct TIMELINE_ITEM {std::string strPatternId,strStageId,strStableId;DETAIL_OWNER eOwner{};TIMELINE_LANE eLane{};uint32_t iStartMs=0,iEndMs=0;bool bEditable=true,bEffectV2Binding=false,bLoopsToStageEnd=false;std::string strEffectV2ClipOccurrenceId;};
 CBalanceTool* m_pBalanceTool;CAnimation_Tool* m_pAnimationTool;int m_eAdmission=1;std::vector<TIMELINE_ITEM> m_TimelineItems;std::vector<TIMELINE_SELECTION> m_TimelineSelection;std::string m_strSelectedStageId="S",m_strSelectedStableId="A",m_strEffectEditIdentity,m_strEffectV2BindingEditId;DETAIL_OWNER m_eDetailOwner=DETAIL_OWNER::ANIMATION;unsigned m_iEffectV2CatalogRevision=1;int m_PatternShakes=11;bool m_bAuthoringDraftDirty=false;
 static TIMELINE_SELECTION Timeline_SelectionKey(const TIMELINE_ITEM& x){return{x.strPatternId,x.strStageId,x.strStableId,x.eOwner};}
 void Invalidate_TimelineCache(){}void Invalidate_EffectivePatternCache(){}void Refresh_PatternLocalPreviewAfterMutation(const VALTAN_PATTERN_VIEW*,std::string&){}
 bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND,std::string&){return false;}bool Delete_SelectedStages(const VALTAN_PATTERN_VIEW&,std::string&){return false;}bool Move_SelectedStages(const VALTAN_PATTERN_VIEW&,int64_t,std::string&){return false;}
 bool Validate_EffectV2BindingClock(const VALTAN_STAGE_VIEW&,const EFFECT_V2_BINDING&,std::string&){return true;}
 void Ensure_TimelineCache(const VALTAN_PATTERN_VIEW* p){m_TimelineItems.clear();uint32_t stageStart=0;for(auto& stage:p->Stages){m_TimelineItems.push_back({p->strPatternId,stage.strStageId,stage.strStageId,DETAIL_OWNER::GAMEPLAY_STAGE,TIMELINE_LANE::STAGE,stageStart,stageStart+stage.iDurationMs});uint32_t cursor=stageStart;for(auto&clip:stage.ClipOccurrences){m_TimelineItems.push_back({p->strPatternId,stage.strStageId,clip.strClipOccurrenceId,DETAIL_OWNER::ANIMATION,TIMELINE_LANE::ANIMATION,cursor,cursor+clip.iAuthoringWallMs});cursor+=clip.iAuthoringWallMs;}stageStart+=stage.iDurationMs;}}
 bool Remove_AnimationOccurrence(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW& stage,const std::string& id,std::string& status){if(m_pBalanceTool->failAnimationDelete || ++m_pBalanceTool->animationDeleteCalls==m_pBalanceTool->failAnimationDeleteAt){status="injected Animation delete failure";return false;}--m_PatternShakes;for(auto&s:m_pBalanceTool->pattern.Stages)if(s.strStageId==stage.strStageId){std::erase_if(s.ClipOccurrences,[&](auto&x){return x.strClipOccurrenceId==id;});return true;}return false;}
 bool Transfer_AnimationOccurrence(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW& source,const VALTAN_STAGE_VIEW& target,const std::string& id,size_t index,std::string&){auto& stages=m_pBalanceTool->pattern.Stages;std::optional<VALTAN_CLIP_OCCURRENCE_VIEW> slot;for(auto&s:stages)if(s.strStageId==source.strStageId){auto i=std::find_if(s.ClipOccurrences.begin(),s.ClipOccurrences.end(),[&](auto&x){return x.strClipOccurrenceId==id;});if(i==s.ClipOccurrences.end())return false;slot=*i;s.ClipOccurrences.erase(i);}for(auto&s:stages)if(s.strStageId==target.strStageId){s.ClipOccurrences.insert(s.ClipOccurrences.begin()+std::min(index,s.ClipOccurrences.size()),*slot);return true;}return false;}
 bool Apply_SoundTimelinePlacement(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW&,const VALTAN_PATTERN_SOUND_CUE& old,uint32_t at,uint32_t,std::optional<uint32_t>,std::string&,bool){for(auto&x:m_pAnimationTool->sounds.Cues)if(x.strOccurrenceId==old.strOccurrenceId){x.iStartMs=at;return true;}return false;}
 bool Apply_TimelineGroupEdit(const VALTAN_PATTERN_VIEW&,TIMELINE_GROUP_EDIT,int64_t,std::string&);
};
}
using namespace Client;
bool Can_MutateValtanView(int x){return x!=0;}
const EFFECT_V2_BINDING* ResolveEffectV2Binding(const EffectSnapshot& view,const std::string& id,const std::string&){for(auto&x:view.bindings)if("v2/"+x.strBindingId==id)return &x;return nullptr;}
std::string BuildEffectV2BindingStableId(const EFFECT_V2_BINDING& x,const std::string&){return "v2/"+x.strBindingId;}
uint32_t ResolveEffectPlaybackOffsetMs(const VALTAN_PRODUCT_EFFECT_CUE_VIEW& x){return x.iPlaybackOffsetMs;}
std::string BuildNextCompositionEffectCueId(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW& x){return "new.effect."+std::to_string(x.ProductCues.size());}
bool ValidateValtanEffectInvocationClock(CAnimation_Tool*,const VALTAN_STAGE_VIEW&,const VALTAN_PRODUCT_EFFECT_CUE_VIEW&,std::string&){return true;}
void CopyValtanColliderFields(const CBalanceTool::PATTERN_STAGE_EDIT& source,CBalanceTool::PATTERN_STAGE_EDIT& target){target.hitShape=source.hitShape;target.pulses=source.hitShape=="NONE"?std::vector<uint32_t>{}:source.pulses;}
bool ResolveValtanColliderScheduleEndpoints(const CBalanceTool::PATTERN_STAGE_EDIT& x,uint32_t& first,uint32_t& last,std::string&){if(x.pulses.empty())return false;first=x.pulses.front();last=x.pulses.back();return true;}
bool TranslateValtanColliderSchedule(CBalanceTool::PATTERN_STAGE_EDIT& x,int64_t delta,std::string&){for(auto&at:x.pulses)at=uint32_t(at+delta);return true;}
bool BuildValtanColliderPasteDraft(const CBalanceTool::PATTERN_STAGE_EDIT& source,uint32_t at,CBalanceTool::PATTERN_STAGE_EDIT& target,std::string&){target.hitShape=source.hitShape;for(auto pulse:source.pulses)target.pulses.push_back(at+pulse-source.pulses.front());return true;}
bool MoveValtanEffectInvocation(CAnimation_Tool*,CBalanceTool* balance,const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW& source,const VALTAN_PRODUCT_EFFECT_CUE_VIEW& current,const std::vector<CValtanActionWorkbench::TIMELINE_ITEM>&,uint32_t at,std::string& stageId,std::string& occurrence,std::string&,std::optional<uint32_t> end){for(auto&s:balance->pattern.Stages)for(auto&cue:s.ProductCues)if(cue.strOccurrenceId==current.strOccurrenceId){cue.iSourceStartMs=at;cue.iStageOffsetMs=at;if(end)cue.iSourceEndMs=*end;stageId=source.strStageId;occurrence=cue.strOccurrenceId;return true;}return false;}
// @PRODUCTION_GROUP_EDIT@
using WB=CValtanActionWorkbench;using O=WB::DETAIL_OWNER;using L=WB::TIMELINE_LANE;using E=WB::TIMELINE_GROUP_EDIT;
int checks=0;void check(bool value,const char* label){++checks;if(!value){std::cerr<<"FAIL "<<label<<'\n';std::exit(1);}}
struct Fixture {
 CBalanceTool balance;CAnimation_Tool sound;WB tool{&balance,&sound};std::string status;
 Fixture(){auto& v2=CEffectV2Catalog::Get();v2=CEffectV2Catalog{};VALTAN_STAGE_VIEW stage;stage.strStageId="S";stage.strActionId="action.s";stage.iDurationMs=2000;stage.ClipOccurrences={{"A",0,500,1},{"B",0,500,1}};
 VALTAN_PRODUCT_EFFECT_CUE_VIEW fx;fx.strPatternId="fixture";fx.strStageId="S";fx.strActionId="action.s";fx.strBindingId="fx";fx.strOccurrenceId="fx.1";fx.strEffectAssetId="effect";fx.strClipOccurrenceId="A";fx.iSourceStartMs=100;stage.ProductCues.push_back(fx);balance.pattern.Stages.push_back(stage);CBalanceTool::PATTERN_STAGE_EDIT hit;hit.stageId="S";balance.colliders.push_back(hit);
 VALTAN_PATTERN_SOUND_CUE cue;cue.strPatternId="fixture";cue.strStageId="S";cue.strActionId="action.s";cue.strBindingId="sound";cue.strOccurrenceId="sound.1";cue.strClipOccurrenceId="A";cue.iStartMs=150;sound.sounds.Cues.push_back(cue);
 EFFECT_V2_BINDING binding;binding.strPatternId="fixture";binding.strStageId="S";binding.strActionId="action.s";binding.strBindingId="v2";binding.strClipOccurrenceId="A";binding.eClockBasis=EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE;binding.iStartMs=200;v2.snapshot->bindings.push_back(binding);
 tool.Ensure_TimelineCache(&balance.pattern);tool.m_TimelineItems.push_back({"fixture","S","fx.1",O::EFFECT,L::EFFECT,100,350});tool.m_TimelineItems.push_back({"fixture","S","sound.1",O::SOUND,L::SOUND,150,350});WB::TIMELINE_ITEM box{"fixture","S","v2/v2",O::EFFECT,L::EFFECT,200,350};box.bEffectV2Binding=true;box.strEffectV2ClipOccurrenceId="A";tool.m_TimelineItems.push_back(box);tool.m_TimelineItems.push_back({"fixture","S","S/collider",O::GAMEPLAY_STAGE,L::COLLIDER,100,200});
 for(auto& item:tool.m_TimelineItems)if(item.eLane!=L::STAGE&&item.strStableId!="B")tool.m_TimelineSelection.push_back(WB::Timeline_SelectionKey(item));}
 bool run(E edit,int64_t delta){const auto pattern=balance.pattern;return tool.Apply_TimelineGroupEdit(pattern,edit,delta,status);}
};
int main(){
 {Fixture f;check(f.run(E::DELETE_BOXES,0),f.status.c_str());check(f.balance.pattern.Stages[0].ClipOccurrences.size()==1,"delete selected Animation after its cues");check(f.balance.pattern.Stages[0].ProductCues.empty()&&f.sound.sounds.Cues.empty()&&CEffectV2Catalog::Get().snapshot->bindings.empty(),"delete mixed V1 Sound V2");check(f.balance.colliders[0].hitShape=="NONE"&&f.tool.m_TimelineSelection.empty(),"delete Collider and clear selection");}
 {Fixture f;f.balance.failAnimationDelete=true;auto selection=f.tool.m_TimelineSelection;check(!f.run(E::DELETE_BOXES,0),"late Animation rejection rejects whole mixed delete");check(f.balance.pattern.Stages[0].ProductCues.size()==1&&f.sound.sounds.Cues.size()==1&&CEffectV2Catalog::Get().snapshot->bindings.size()==1&&f.balance.colliders[0].hitShape=="CIRCLE","late failure restores V1 Sound V2 Collider");check(f.balance.pattern.Stages[0].ClipOccurrences.size()==2&&f.tool.m_TimelineSelection==selection&&!f.balance.dirty&&!f.sound.dirty&&CEffectV2Catalog::Get().revision==1,"rollback preserves clips selection dirty and V2 revision");}
 {Fixture f;check(f.run(E::MOVE_BOXES,500),f.status.c_str());auto&s=f.balance.pattern.Stages[0];check(s.ClipOccurrences[0].strClipOccurrenceId=="B"&&s.ClipOccurrences[1].strClipOccurrenceId=="A","Animation group snaps to next slot");check(s.ProductCues.size()==1&&s.ProductCues[0].strClipOccurrenceId=="A"&&s.ProductCues[0].iSourceStartMs==100,"moved V1 follows exact selected clip");check(f.sound.sounds.Cues[0].strClipOccurrenceId=="A"&&f.sound.sounds.Cues[0].iStartMs==150,"moved Sound source timing preserved");check(CEffectV2Catalog::Get().snapshot->bindings[0].strClipOccurrenceId=="A"&&CEffectV2Catalog::Get().snapshot->bindings[0].iStartMs==200,"moved V2 source timing preserved");check(f.balance.colliders[0].pulses==std::vector<uint32_t>({600,700})&&f.tool.m_TimelineSelection.size()==5,"Collider and selection use same snapped delta");}
 {Fixture f;CEffectV2Catalog::Get().failAppend=true;auto selection=f.tool.m_TimelineSelection;check(!f.run(E::MOVE_BOXES,500),"late V2 rejection rejects whole mixed move");check(f.balance.pattern.Stages[0].ClipOccurrences[0].strClipOccurrenceId=="A"&&f.balance.pattern.Stages[0].ProductCues[0].strOccurrenceId=="fx.1"&&f.sound.sounds.Cues[0].strOccurrenceId=="sound.1"&&CEffectV2Catalog::Get().snapshot->bindings[0].strBindingId=="v2"&&f.tool.m_TimelineSelection==selection,"late V2 failure restores original IDs and Animation order");}
 {Fixture f;f.balance.colliders[0].colliderRemoveAdmitted=false;check(!f.run(E::DELETE_BOXES,0),"required Collider rejection is atomic");check(f.balance.pattern.Stages[0].ProductCues.size()==1&&f.sound.sounds.Cues.size()==1&&CEffectV2Catalog::Get().snapshot->bindings.size()==1,"required Collider restores earlier removed cues");}
 {Fixture f;std::erase_if(f.tool.m_TimelineSelection,[](const auto&x){return x.strStableId!="S/collider";});check(f.run(E::DELETE_BOXES,0),"single Collider Delete uses same typed path");check(f.balance.colliders[0].hitShape=="NONE"&&f.balance.pattern.Stages[0].ClipOccurrences.size()==2,"single Collider Delete preserves animations");}
 {Fixture f;f.balance.failAnimationDeleteAt=2;f.tool.m_strEffectEditIdentity="effect.detail";f.tool.m_strEffectV2BindingEditId="v2.detail";f.tool.m_TimelineSelection.push_back({"fixture","S","B",O::ANIMATION});auto selection=f.tool.m_TimelineSelection;check(!f.run(E::DELETE_BOXES,0),"second Animation failure rolls back first delete");check(f.tool.m_PatternShakes==11&&f.balance.pattern.Stages[0].ClipOccurrences.size()==2&&f.tool.m_TimelineSelection==selection,"rollback restores cascaded Shake draft and complete selection");check(f.tool.m_strEffectEditIdentity=="effect.detail"&&f.tool.m_strEffectV2BindingEditId=="v2.detail"&&!f.tool.m_bAuthoringDraftDirty&&f.tool.m_iEffectV2CatalogRevision==1,"rollback preserves detail identity and draft indicators");}
 std::cout<<"PASS "<<checks<<" group move/delete transaction checks\n";
}
