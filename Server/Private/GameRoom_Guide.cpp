#include "GameRoom.h"
#include "ClientSession.h"
#include "ServerCombatGeometry.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

using namespace LostArk::Server;
using namespace LostArk::Shared;
namespace
{
 constexpr float PI = 3.14159265359f;
 float distance(const SERVER_PLAYER& a,const SERVER_PLAYER& b){return std::hypot(a.fPositionX-b.fPositionX,a.fPositionZ-b.fPositionZ);}
 std::uint32_t ticks(std::uint32_t ms){return (ms*30u+999u)/1000u;}
 bool isMinigameAnchor(const SERVER_PLAYER& player){return player.iMarioStage!=0||player.eCardMazeRole!=CARD_MAZE_ROLE::NONE||player.eKoukuHudMode==KOUKU_HUD_MODE::MARIO||player.eKoukuHudMode==KOUKU_HUD_MODE::DANCE||player.eKoukuHudMode==KOUKU_HUD_MODE::MAZE;}
 std::string trim(const std::string& value){auto a=value.find_first_not_of(" \t\r\n");auto b=value.find_last_not_of(" \t\r\n");return a==std::string::npos?std::string{}:value.substr(a,b-a+1);}
}

std::size_t CGameRoom::Count_HumanPlayers() const
{
 return std::count_if(m_Players.begin(),m_Players.end(),[](const auto& p){return p.second.Is_Human();});
}

bool CGameRoom::Find_GuideLanding(const SERVER_PLAYER& guide,float x,float y,float z,SERVER_NAV_POINT& point) const
{
 // Deterministic rings share the normal navigation and actual collision admission.
 for(unsigned sample=0;sample<49;++sample)
 {
  const float radius=sample?(.75f+static_cast<float>((sample-1)/12)*.75f):0.f;
  const float angle=static_cast<float>(sample%12)*PI/6.f;
  SERVER_NAV_POINT candidate{x+std::sin(angle)*radius,y,z+std::cos(angle)*radius};
  if(m_ServerNavigation.Is_Loaded()&&(!m_ServerNavigation.Sample_Position(candidate.x,candidate.z,candidate,y)||std::abs(candidate.y-y)>2.f))continue;
  if(!m_ServerCollisionSystem.Is_PlayerPositionClear(candidate.x,candidate.y,candidate.z,guide.iNetEntityId))continue;
  bool overlap=false;
  for(const auto& [id,other]:m_Players)if(id!=guide.iPlayerId&&id!=m_iGuideReceptionId&&other.iCurrentHp&&std::abs(other.fPositionY-candidate.y)<1.5f&&std::hypot(other.fPositionX-candidate.x,other.fPositionZ-candidate.z)<.75f){overlap=true;break;}
  if(!overlap){point=candidate;return true;}
 }
 return false;
}

bool CGameRoom::Build_GuidePlayer(PLAYER_ID playerId,NET_ENTITY_ID entityId,float x,float y,float z,SERVER_PLAYER& out) const
{
 const auto* profile=m_GameplayCatalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER);
 if(!m_GuideCatalog.Loaded||!profile||!playerId||!entityId||(!m_Players.contains(playerId)&&m_Players.size()>=MAX_WORLD_SNAPSHOT_PLAYERS))return false;
 SERVER_PLAYER guide;
 guide.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;guide.iPlayerId=playerId;guide.iNetEntityId=entityId;
 guide.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;guide.strNickName=m_GuideCatalog.Name;
 guide.strSpawnPlacementId=m_GuideCatalog.PlacementId;guide.eStance=profile->eDefaultStance;
 guide.iCurrentHp=guide.iMaximumHp=profile->iMaximumHp;guide.iCurrentResource=guide.iMaximumResource=profile->iMaximumResource;
 guide.iMaximumIdentity=profile->iMaximumIdentity;guide.fMoveSpeed=profile->fMoveSpeed;guide.fYawDegrees=m_GuideCatalog.Yaw;
 CPlayerSkillSystem::Reset_Gauges(guide,m_GameplayCatalog);guide.isCombatReady=true;
 SERVER_NAV_POINT position;
 if(!Find_GuideLanding(guide,x,y,z,position))return false;
 guide.fPositionX=position.x;guide.fPositionY=position.y;guide.fPositionZ=position.z;
 out=std::move(guide);return true;
}

void CGameRoom::Initialize_Guide()
{
 if(m_eWorldId!=WORLD_ID::BERN&&m_eWorldId!=WORLD_ID::VALTAN_ARENA&&m_eWorldId!=WORLD_ID::KAKULSAYDON_ARENA&&m_eWorldId!=WORLD_ID::COLOSSEUM)return;
 std::string status;
 if(!m_GuideCatalog.Load(status)){std::cout<<"[Guide] "<<status<<'\n';return;}
 // Class/slot references are checked again against the actual installed Server generation.
 for(const auto& combo:m_GuideCatalog.Combos)
 {
  if(combo.Skills.size()!=combo.Slots.size()){m_GuideCatalog.Loaded=false;std::cout<<"[Guide] unresolved combo "<<combo.Id<<'\n';return;}
  for(std::size_t i=0;i<combo.Skills.size();++i){const auto* skill=m_GameplayCatalog.Find_Skill(combo.Skills[i]);if(!skill||skill->eCharacterClass!=CHARACTER_CLASS_ID::DIMENSIONMASTER||skill->strInputSlot!=combo.Slots[i]){m_GuideCatalog.Loaded=false;std::cout<<"[Guide] published skill binding mismatch "<<combo.Id<<'\n';return;}}
 }
 if(m_eWorldId!=WORLD_ID::BERN)return;
 SERVER_PLAYER reception;
 if(!Build_GuidePlayer(m_iNextGuidePlayerId,m_iNextNetEntityId,m_GuideCatalog.Position[0],m_GuideCatalog.Position[1],m_GuideCatalog.Position[2],reception)){std::cout<<"[Guide] reception position is unavailable\n";return;}
 reception.isCombatReady=false;m_iGuideReceptionId=reception.iPlayerId;
 m_PlayerIdByEntityId.emplace(reception.iNetEntityId,reception.iPlayerId);m_Players.emplace(reception.iPlayerId,std::move(reception));++m_iNextGuidePlayerId;++m_iNextNetEntityId;
}

bool CGameRoom::Start_Guide(const SERVER_PLAYER& inviter,const NET_ENTITY_ID target)
{
 const auto targetId=m_PlayerIdByEntityId.find(target);
 if(targetId==m_PlayerIdByEntityId.end())return false;
 const auto actor=m_Players.find(targetId->second);
 if(actor==m_Players.end()||!actor->second.Is_Guide())return false;
 if(!inviter.Is_Human()||!inviter.iCurrentHp||m_eWorldId!=WORLD_ID::BERN||!m_GuideCatalog.Loaded)return true;
 const auto ownerSessionId=inviter.iSessionId;
 auto ownerSession=Find_Session(ownerSessionId);
 if(!ownerSession||ownerSession->Is_Closing()||!ownerSessionId)return true;
 if(!m_PersonalGuides.empty()){Broadcast_GuideOwnership();return true;}
 if(targetId->second!=m_iGuideReceptionId||distance(inviter,actor->second)>10.f)return true;
 auto& guide=actor->second;
 // Restarting with a grounded owner uses the existing validated flight landing.
 if(guide.eVehicleFlightPhase!=VEHICLE_FLIGHT_PHASE::GROUNDED&&inviter.iVehicleId!=ANCIENT_SEA_VEHICLE_ID){End_VehicleSkill(guide);if(guide.eAction==PLAYER_ACTION_STATE::FALLING)return true;}
 Reset_PlayerForDebugTeleport(guide);guide.isCombatReady=true;
 GUIDE_RUNTIME runtime;runtime.PlayerId=guide.iPlayerId;runtime.AnchorId=inviter.iPlayerId;runtime.OwnerSession=ownerSession;runtime.OwnerNetEntityId=inviter.iNetEntityId;
 runtime.Sequence=(std::max)({guide.iLastMoveSequence,guide.iLastSkillSequence,guide.LastVehicleRidingResult.iRequestSequence});
 // Bind the already placed actor. Starting guidance never creates a player or a party.
 m_PersonalGuides.emplace(ownerSessionId,std::move(runtime));
 Seed_GuideSpaceEntries(ownerSessionId,inviter);
 Broadcast_GuideOwnership();
 const auto category=m_GuideCatalog.Categories.find(WORLD_ID::BERN);
 for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&(trigger.Type=="GUIDE_STARTED"||trigger.Type=="PARTY_JOINED"))Queue_GuidePrompt(ownerSessionId,trigger);
 return true;
}

void CGameRoom::Handle_GuideControl(SESSION_ID sessionId,const C2S_GUIDE_CONTROL& request)
{
 if(m_eWorldId!=WORLD_ID::BERN||request.iRequestSequence==0)return;
 auto& last=m_GuideControlSequences[sessionId];
 if(last&&static_cast<std::int32_t>(request.iRequestSequence-last)<=0)return;
 last=request.iRequestSequence;
 const auto binding=m_PlayerIdBySessionId.find(sessionId);
 if(binding==m_PlayerIdBySessionId.end())return;
 const auto owner=m_Players.find(binding->second),actor=m_Players.find(m_iGuideReceptionId);
 if(owner==m_Players.end()||actor==m_Players.end()||!owner->second.Is_Human()||request.iGuideNetEntityId!=actor->second.iNetEntityId)return;
 if(request.eAction==GUIDE_CONTROL_ACTION::START)(void)Start_Guide(owner->second,request.iGuideNetEntityId);
 else if(request.eAction==GUIDE_CONTROL_ACTION::STOP&&m_PersonalGuides.contains(sessionId))Remove_Guide(sessionId);
}

void CGameRoom::Broadcast_GuideState(const S2C_GUIDE_STATE& message)
{
 CPacketWriter writer;if(!Write_Message(writer,message))return;
 for(const auto& [id,human]:m_Players)if(human.Is_Human())if(auto session=Find_Session(human.iSessionId);session&&!session->Is_Closing()&&!session->Send_Frame(PACKET_TYPE::S2C_GUIDE_STATE,writer.Get_Buffer()))session->Request_Close();
}

void CGameRoom::Broadcast_GuideOwnership()
{
 const auto actor=m_Players.find(m_iGuideReceptionId);if(actor==m_Players.end())return;
 S2C_GUIDE_STATE state;state.iGuideNetEntityId=actor->second.iNetEntityId;state.iRevision=m_GuideCatalog.Revision;state.iServerTick=m_iServerTick?m_iServerTick:1;
 if(!m_PersonalGuides.empty()){const auto& runtime=m_PersonalGuides.begin()->second;state.iOwnerNetEntityId=runtime.OwnerNetEntityId;state.iAction=runtime.WaitingForOwner||runtime.WaitingForShip?8:1;state.strReason="Guidance is reserved for its current owner";}
 else state.strReason="Ready to start guidance";
 Broadcast_GuideState(state);
}

void CGameRoom::Remove_Guide(SESSION_ID ownerSessionId,bool publish)
{
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end())return;
 auto actor=m_Players.find(found->second.PlayerId);
 if(actor!=m_Players.end()){
  Reset_PlayerForDebugTeleport(actor->second);actor->second.isCombatReady=false;
  actor->second.fVehicleFlightInputX=actor->second.fVehicleFlightInputY=actor->second.fVehicleFlightInputZ=0.f;
  actor->second.fVehicleFlightVelocityX=actor->second.fVehicleFlightVelocityY=actor->second.fVehicleFlightVelocityZ=0.f;
 }
 // Releasing guidance discards every pending occurrence, while the same actor remains placed.
 m_PersonalGuides.erase(found);
 if(publish)Broadcast_GuideOwnership();
}

void CGameRoom::Suspend_PersonalGuide(SESSION_ID ownerSessionId)
{
 if(m_eWorldId!=WORLD_ID::BERN)return;
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end())return;
 auto& state=found->second;
 auto actor=m_Players.find(state.PlayerId);
 if(actor!=m_Players.end()){
  Reset_PlayerForDebugTeleport(actor->second);
  actor->second.isCombatReady=false;
  actor->second.fVehicleFlightInputX=actor->second.fVehicleFlightInputY=actor->second.fVehicleFlightInputZ=0.f;
  actor->second.fVehicleFlightVelocityX=actor->second.fVehicleFlightVelocityY=actor->second.fVehicleFlightVelocityZ=0.f;
 }
 // Called inside a transfer commit: no packets or callbacks to outbound queues.
 state.WaitingForOwner=true;state.ReturningOnFoot=false;state.ComboId.clear();state.PendingComboId.clear();state.Reason.clear();
 state.PromptQueue.clear();state.PromptRemaining=0.f;state.InsideBoxes.clear();state.FarElapsed=0.f;
}

void CGameRoom::Resume_PersonalGuide(SESSION_ID ownerSessionId,WORLD_ID sourceWorld)
{
 if(m_eWorldId!=WORLD_ID::BERN)return;
 const auto found=m_PersonalGuides.find(ownerSessionId);if(found==m_PersonalGuides.end()||!found->second.WaitingForOwner)return;
 auto& state=found->second;const auto owner=state.OwnerSession.lock();
 const auto binding=m_PlayerIdBySessionId.find(ownerSessionId);
 if(!owner||owner->Is_Closing()||owner!=Find_Session(ownerSessionId)||binding==m_PlayerIdBySessionId.end())return;
 const auto actor=m_Players.find(state.PlayerId);if(actor==m_Players.end())return;
 state.AnchorId=binding->second;state.OwnerNetEntityId=m_Players.at(binding->second).iNetEntityId;state.WaitingForOwner=false;state.ReturningOnFoot=true;state.FarElapsed=0.f;
 actor->second.isCombatReady=actor->second.iCurrentHp!=0;
 state.Reason="Owner returned; approaching on the existing navigation path";
 Seed_GuideSpaceEntries(ownerSessionId,m_Players.at(binding->second));
 const char* type=nullptr;const char* world=nullptr;
 switch(sourceWorld){
 case WORLD_ID::VALTAN_ARENA:type="RAID_RETURNED";world="VALTAN_ARENA";break;
 case WORLD_ID::KAKULSAYDON_ARENA:type="RAID_RETURNED";world="KAKULSAYDON_ARENA";break;
 case WORLD_ID::MAHARAKA:type="WORLD_RETURNED";world="MAHARAKA";break;
 case WORLD_ID::COLOSSEUM:type="WORLD_RETURNED";world="COLOSSEUM";break;
 default:break;
 }
 // This runs only after successful entry commit. Actual sending stays in Update_Guides.
 const auto category=m_GuideCatalog.Categories.find(WORLD_ID::BERN);
 if(type)for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&trigger.Type==type&&trigger.PatternId==world)Queue_GuidePrompt(ownerSessionId,trigger);
}

void CGameRoom::Seed_GuideSpaceEntries(SESSION_ID ownerSessionId,const SERVER_PLAYER& anchor)
{
 const auto guide=m_PersonalGuides.find(ownerSessionId);if(guide==m_PersonalGuides.end())return;
 auto& inside=guide->second.InsideBoxes;inside.clear();
 const auto category=m_GuideCatalog.Categories.find(m_eWorldId);if(category==m_GuideCatalog.Categories.end())return;
 // Starting or arriving while already inside a box is not an outside-to-inside edge.
 for(const auto& trigger:m_GuideCatalog.Triggers)
  if(trigger.Enabled&&trigger.Category==category->second&&trigger.Type=="SPACE_ENTER"&&CServerTriggerSystem::Contains_Placement(trigger.Box,anchor))inside.insert(trigger.Id);
}

void CGameRoom::Prune_GuideSpacePrompts(SESSION_ID ownerSessionId)
{
 const auto guide=m_PersonalGuides.find(ownerSessionId);if(guide==m_PersonalGuides.end())return;
 const auto anchor=m_Players.find(guide->second.AnchorId);const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
 // Retain only sources whose entry really fired; never adopt an unfired nearby box.
 auto& queue=guide->second.PromptQueue;
 for(auto occurrence=queue.begin();occurrence!=queue.end();){
  auto& queued=*occurrence;
  if(!queued.IsSpaceEnter||queued.NextSegment>0){++occurrence;continue;}
  const GUIDE_TRIGGER* best=nullptr;
  std::erase_if(queued.SpaceTriggerIds,[&](const auto& id){
   const auto source=std::find_if(m_GuideCatalog.Triggers.begin(),m_GuideCatalog.Triggers.end(),[&](const auto& row){return row.Id==id;});
   if(anchor==m_Players.end()||source==m_GuideCatalog.Triggers.end()||!source->Enabled||source->Type!="SPACE_ENTER"||source->PromptId!=queued.PromptId||category==m_GuideCatalog.Categories.end()||source->Category!=category->second||!CServerTriggerSystem::Contains_Placement(source->Box,anchor->second))return true;
   if(!best||source->Priority>best->Priority)best=&*source;
   return false;
  });
  if(!best){occurrence=queue.erase(occurrence);continue;}
  queued.TriggerId=best->Id;queued.Priority=best->Priority;++occurrence;
 }
 auto first=queue.begin();if(first!=queue.end()&&first->NextSegment>0)++first;
 std::stable_sort(first,queue.end(),[](const auto& a,const auto& b){return a.Priority>b.Priority;});
}

void CGameRoom::Queue_GuidePrompt(SESSION_ID ownerSessionId,const GUIDE_TRIGGER& trigger)
{
 auto guide=m_PersonalGuides.find(ownerSessionId);const auto* prompt=m_GuideCatalog.Find_Prompt(trigger.PromptId);
 if(guide==m_PersonalGuides.end()||!prompt)return;
 // A same-text occurrence at a new location must not inherit a departed source.
 Prune_GuideSpacePrompts(ownerSessionId);
 auto& queue=guide->second.PromptQueue;
 const auto duplicate=std::find_if(queue.begin(),queue.end(),[&](const auto& queued){return queued.PromptId==trigger.PromptId;});
 if(duplicate!=queue.end()){
  if(duplicate->NextSegment>0)return;
  if(duplicate->IsSpaceEnter&&trigger.Type=="SPACE_ENTER"){
   if(std::find(duplicate->SpaceTriggerIds.begin(),duplicate->SpaceTriggerIds.end(),trigger.Id)==duplicate->SpaceTriggerIds.end())duplicate->SpaceTriggerIds.push_back(trigger.Id);
   Prune_GuideSpacePrompts(ownerSessionId);return;
  }
  if(duplicate->Priority>=trigger.Priority)return;
  queue.erase(duplicate);
 }
 if(queue.size()>=16)return;
 auto place=queue.begin();if(place!=queue.end()&&place->NextSegment>0)++place;
 // Rank the occurrence that actually fired, not an unrelated trigger sharing its text.
 while(place!=queue.end()&&place->Priority>=trigger.Priority)++place;
 queue.insert(place,{trigger.PromptId,trigger.Id,0,trigger.Priority,trigger.Type=="SPACE_ENTER",trigger.Type=="SPACE_ENTER"?std::vector<std::string>{trigger.Id}:std::vector<std::string>{}});
}

void CGameRoom::Guide_ChatCommand(const SERVER_PLAYER& sender,const std::string& line)
{
 if(!sender.Is_Human())return;
 auto guide=m_PersonalGuides.find(sender.iSessionId);if(guide==m_PersonalGuides.end()||guide->second.WaitingForOwner||guide->second.WaitingForShip)return;
 const auto input=trim(line);auto& state=guide->second;
 for(const auto& command:m_GuideCatalog.Commands)
 {
  if(!command.Enabled||std::find(command.Aliases.begin(),command.Aliases.end(),input)==command.Aliases.end())continue;
  const auto commandTick=m_iServerTick?m_iServerTick:1;
  const auto lastCommand=state.CommandTicks.find(command.Id);
  if(!command.Stop&&lastCommand!=state.CommandTicks.end()&&commandTick-lastCommand->second<ticks(command.CooldownMs))return;
  state.CommandTicks[command.Id]=commandTick;
  std::string comboId=command.ComboId;int comboPriority=(std::numeric_limits<int>::min)();
  const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
  for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&trigger.Type=="HELP_COMMAND"&&trigger.PatternId==command.Id&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second&&!trigger.ComboId.empty()&&trigger.Priority>comboPriority){comboId=trigger.ComboId;comboPriority=trigger.Priority;}
  if(command.Stop){state.ComboId.clear();state.PendingComboId.clear();state.ComboStep=0;state.Reason="Assistance stopped by owner command";}
  else if(state.ComboId!=comboId)
  {
   auto actor=m_Players.find(state.PlayerId);
   if(actor!=m_Players.end()&&actor->second.eAction==PLAYER_ACTION_STATE::SKILL)state.PendingComboId=comboId;
   else {state.ComboId=comboId;state.ComboStep=0;state.ComboElapsed=state.StepElapsed=0;}
  }
  for(const auto& trigger:m_GuideCatalog.Triggers)if(trigger.Enabled&&trigger.Type=="HELP_COMMAND"&&trigger.PatternId==command.Id&&category!=m_GuideCatalog.Categories.end()&&trigger.Category==category->second)Queue_GuidePrompt(sender.iSessionId,trigger);
  return;
 }
}

void CGameRoom::Guide_AnchorArrived(const SERVER_PLAYER& anchor, const bool localMapTravel)
{
 if(!anchor.Is_Human()||isMinigameAnchor(anchor))return;
 const auto found=m_PersonalGuides.find(anchor.iSessionId);
 if(found==m_PersonalGuides.end()||found->second.WaitingForOwner)return;
 auto& state=found->second;
 if(state.AnchorId!=anchor.iPlayerId)return;
 auto actor=m_Players.find(state.PlayerId);if(actor==m_Players.end())return;
 Reset_PlayerForDebugTeleport(actor->second);
 state.ComboId.clear();state.PendingComboId.clear();state.FarElapsed=0.f;
 // Preserve owner contact: an actual relocation into a new space still has an entry edge.
 state.ReturningOnFoot=true;state.Reason="Following committed owner arrival on foot";
 // An admitted Bern map-travel is a shared local arrival, before ship boarding.
 // The existing actor follows that relocation; cross-world returns keep their on-foot approach.
 if(localMapTravel&&m_eWorldId==WORLD_ID::BERN&&!anchor.bShipDockValid)
 {
  const float yaw=anchor.fYawDegrees*PI/180.f;
  SERVER_NAV_POINT landing;
  if(Find_GuideLanding(actor->second,
      anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance,anchor.fPositionY,
      anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance,landing))
  {
   actor->second.fPositionX=landing.x;actor->second.fPositionY=landing.y;actor->second.fPositionZ=landing.z;
   actor->second.fYawDegrees=anchor.fYawDegrees;actor->second.isCombatReady=actor->second.iCurrentHp!=0u;
   state.ReturningOnFoot=false;state.WaitingForShip=false;
  }
 }
}

void CGameRoom::Update_Guides(float seconds)
{
 if(!m_GuideCatalog.Loaded||m_eWorldId!=WORLD_ID::BERN)return;
 m_fGuideOwnershipElapsed+=seconds;
 if(m_fGuideOwnershipElapsed>=m_GuideCatalog.ThinkSeconds&&(m_PersonalGuides.empty()||m_PersonalGuides.begin()->second.WaitingForOwner)){m_fGuideOwnershipElapsed=0.f;Broadcast_GuideOwnership();}
 for(auto it=m_PersonalGuides.begin();it!=m_PersonalGuides.end();)
 {
  const auto ownerSessionId=it->first;auto& state=it->second;
  const auto ownerSession=state.OwnerSession.lock();
  if(!ownerSession||ownerSession->Is_Closing()){++it;Remove_Guide(ownerSessionId);continue;}
  auto guideIt=m_Players.find(state.PlayerId);
  if(guideIt==m_Players.end()){++it;Remove_Guide(ownerSessionId);continue;}
  if(state.WaitingForOwner){++it;continue;}
  const auto binding=m_PlayerIdBySessionId.find(ownerSessionId);
  if(binding==m_PlayerIdBySessionId.end()){++it;continue;}
  state.AnchorId=binding->second;
  auto anchorIt=m_Players.find(state.AnchorId);if(anchorIt==m_Players.end()){++it;continue;}
  const bool anchorSuppressed=isMinigameAnchor(anchorIt->second);
  auto& guide=guideIt->second;const auto& anchor=anchorIt->second;
  auto send=[&](PACKET_TYPE kind,const auto& message){CPacketWriter writer;if(!Write_Message(writer,message))return false;if(!ownerSession->Send_Frame(kind,writer.Get_Buffer())){ownerSession->Request_Close();return false;}return true;};
  float evaluatedThreat=0.f;bool survivalOverride=false;
  auto publishTrace=[&](){S2C_GUIDE_STATE trace;trace.iGuideNetEntityId=guide.iNetEntityId;trace.iOwnerNetEntityId=anchor.iNetEntityId;trace.iRevision=m_GuideCatalog.Revision;trace.iServerTick=m_iServerTick?m_iServerTick:1;trace.iContext=state.ComboId.empty()?0:1;trace.iAction=state.Action;trace.fFollowScore=state.FollowScore;trace.fEvadeScore=state.EvadeScore;trace.fCombatScore=state.CombatScore;trace.fThreat=evaluatedThreat;trace.fAnchorDistance=distance(guide,anchor);trace.fHpRatio=guide.iMaximumHp?std::clamp(static_cast<float>(guide.iCurrentHp)/guide.iMaximumHp,0.f,1.f):0.f;trace.bSurvivalOverride=survivalOverride;trace.strReason=state.Reason;trace.strComboId=state.ComboId;trace.iComboStep=static_cast<std::uint32_t>(state.ComboStep);Broadcast_GuideState(trace);};
  // Ships leave the guide at the pier; the dragon still uses the shared flight path below.
  if(anchor.bShipDockValid){
   if(!state.WaitingForShip){Reset_PlayerForDebugTeleport(guide);guide.isCombatReady=false;state.WaitingForShip=true;state.ComboId.clear();state.PendingComboId.clear();}
   state.Action=8;state.FarElapsed=0.f;state.Reason="Waiting at the pier while the owner sails";publishTrace();++it;continue;
  }
  if(state.WaitingForShip){state.WaitingForShip=false;state.ReturningOnFoot=true;guide.isCombatReady=true;}
  // Prune even while another prompt is playing so a fresh re-entry can queue it again.
  Prune_GuideSpacePrompts(ownerSessionId);
  const bool waitingToSpeak=state.ReturningOnFoot&&distance(guide,anchor)>m_GuideCatalog.MaximumDistance;
  if(!waitingToSpeak)state.PromptRemaining-=seconds;
  // Ownership precedes every new prompt, including a world-return first frame.
  if(!waitingToSpeak&&state.PromptRemaining<=0&&!state.PromptQueue.empty())publishTrace();
  if(!waitingToSpeak&&state.PromptRemaining<=0&&!state.PromptQueue.empty())
  {
   auto& queued=state.PromptQueue.front();const auto* prompt=m_GuideCatalog.Find_Prompt(queued.PromptId);
   if(prompt&&queued.NextSegment<prompt->Segments.size()){
    const auto& segment=prompt->Segments[queued.NextSegment];S2C_GUIDE_PROMPT message;message.iGuideNetEntityId=guide.iNetEntityId;message.iEventSequence=++m_iGuideEventSequence;if(!message.iEventSequence)message.iEventSequence=++m_iGuideEventSequence;message.iRevision=m_GuideCatalog.Revision;message.strPromptId=prompt->Id;message.strText=segment.Text;message.iDurationMs=segment.DurationMs;
    if(send(PACKET_TYPE::S2C_GUIDE_PROMPT,message)){
     // Cancelled/overflowed reservations never spend the source trigger's cooldown.
     if(queued.NextSegment==0){
      if(queued.IsSpaceEnter)for(const auto& id:queued.SpaceTriggerIds)state.TriggerTicks[id]=m_iServerTick;
      else if(!queued.TriggerId.empty())state.TriggerTicks[queued.TriggerId]=m_iServerTick;
     }
     ++queued.NextSegment;state.PromptRemaining=segment.DurationMs/1000.f;
    }
   }
   if(!prompt||queued.NextSegment>=prompt->Segments.size())state.PromptQueue.pop_front();
  }
  state.ThinkElapsed+=seconds;state.HoldElapsed+=seconds;if(!state.ComboId.empty())state.ComboElapsed+=seconds;
  if(state.ThinkElapsed<m_GuideCatalog.ThinkSeconds){++it;continue;}
  const float elapsed=state.ThinkElapsed;state.ThinkElapsed=0;state.FollowScore=state.EvadeScore=state.CombatScore=0;
  if(anchorSuppressed){guide.hasMoveGoal=false;guide.MovePath.clear();state.Action=8;state.ComboId.clear();state.PendingComboId.clear();state.Reason="Waiting outside the human-only minigame";publishTrace();++it;continue;}
  if(!anchor.iCurrentHp||anchor.eAction==PLAYER_ACTION_STATE::DEAD){state.Action=7;guide.hasMoveGoal=false;guide.MovePath.clear();state.ComboId.clear();state.PendingComboId.clear();state.Reason="Waiting for the owner to revive";publishTrace();++it;continue;}
  if(!guide.iCurrentHp)
  {
   state.Action=4;
   SERVER_NAV_POINT center{anchor.fPositionX,anchor.fPositionY,anchor.fPositionZ};float yaw=0;
   if(m_eWorldId==WORLD_ID::VALTAN_ARENA){if(const auto* p=Find_Placement("boss.valtan.center"))center={p->fPositionX,p->fPositionY,p->fPositionZ};}
   else if(m_eWorldId==WORLD_ID::KAKULSAYDON_ARENA)(void)Resolve_KoukuRevivePosition(guide,center,yaw);
   SERVER_PLAYER revived;if(Build_GuidePlayer(guide.iPlayerId,guide.iNetEntityId,center.x,center.y,center.z,revived)){m_CombatObjectRuntime.Cancel_Source(guide.iNetEntityId);guide=std::move(revived);state.ComboId.clear();state.PendingComboId.clear();state.Reason="Guide revived at an admitted arena position";}
   publishTrace();++it;continue;
  }
  const auto category=m_GuideCatalog.Categories.find(m_eWorldId);
  for(const auto& trigger:m_GuideCatalog.Triggers)
  {
   if(!trigger.Enabled||category==m_GuideCatalog.Categories.end()||trigger.Category!=category->second)continue;
   bool fire=false;
   if(trigger.Type=="SPACE_ENTER"){const bool inside=CServerTriggerSystem::Contains_Placement(trigger.Box,anchor);const bool was=state.InsideBoxes.contains(trigger.Id);if(inside)state.InsideBoxes.insert(trigger.Id);else state.InsideBoxes.erase(trigger.Id);fire=inside&&!was;}
   else if(trigger.Type=="BOSS_PATTERN_STARTED")for(const auto& boss:m_WorldEntities)if(boss.iCurrentHp&&boss.strPatternId==trigger.PatternId&&boss.iPatternSequence&&state.PatternSequences[boss.iNetEntityId]!=boss.iPatternSequence){fire=true;break;}
   auto last=state.TriggerTicks.find(trigger.Id);if(fire&&(last==state.TriggerTicks.end()||m_iServerTick-last->second>=ticks(trigger.CooldownMs))){Queue_GuidePrompt(ownerSessionId,trigger);}
  }
  for(const auto& boss:m_WorldEntities)if(!boss.strPatternId.empty())state.PatternSequences[boss.iNetEntityId]=boss.iPatternSequence;
  SERVER_WORLD_ENTITY* enemy=nullptr;float enemyDistance=100000;
  for(auto& candidate:m_WorldEntities)if((candidate.eKind==WORLD_BOOTSTRAP_KIND::BOSS||candidate.eKind==WORLD_BOOTSTRAP_KIND::MONSTER)&&candidate.iCurrentHp&&!candidate.isEstherSummon){const float d=std::hypot(candidate.fPositionX-guide.fPositionX,candidate.fPositionZ-guide.fPositionZ);if(d<enemyDistance){enemy=&candidate;enemyDistance=d;}}
  const bool battle=enemy&&enemyDistance<35.f&&(enemy->eAction!=SERVER_ENTITY_ACTION::IDLE||!state.ComboId.empty());
  const float anchorDistance=distance(guide,anchor);
  if(state.ReturningOnFoot&&anchorDistance<=m_GuideCatalog.MaximumDistance)state.ReturningOnFoot=false;
  state.FarElapsed=!state.ReturningOnFoot&&anchorDistance>m_GuideCatalog.RecoverDistance&&!battle?state.FarElapsed+elapsed:0.f;
  if(state.FarElapsed>=m_GuideCatalog.RecoverDelay){state.Action=4;Guide_AnchorArrived(anchor);publishTrace();++it;continue;}
  if(guide.bPatternBound||guide.TriggerMove.isActive||guide.eAction==PLAYER_ACTION_STATE::GRABBED||guide.eAction==PLAYER_ACTION_STATE::FALLING||guide.fKnockbackRemainingSeconds>0){state.Action=5;state.Reason="Contact reaction owns the guide movement";publishTrace();++it;continue;}
  if(anchor.iVehicleId!=guide.iVehicleId&&guide.eAction==PLAYER_ACTION_STATE::NONE){C2S_SET_VEHICLE_RIDING command;command.eWorldId=m_eWorldId;command.iRequestSequence=++state.Sequence;command.iVehicleId=anchor.iVehicleId;(void)Apply_SetVehicleRiding(guide,command);}
  if(guide.iVehicleId==ANCIENT_SEA_VEHICLE_ID)
  {
   if(((anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING||anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::TAKEOFF)&&guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::GROUNDED)||((anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::GROUNDED||anchor.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::LANDING)&&guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING)){const auto* vehicle=m_VehicleCatalog.Find_Vehicle(guide.iVehicleId);if(vehicle){for(const auto& skill:vehicle->Skills)if(skill.eSlot==VEHICLE_SKILL_SLOT::E){C2S_USE_SKILL command;command.iClientSequence=++state.Sequence;command.iSkillId=skill.iSkillId;command.fAimX=anchor.fPositionX;command.fAimZ=anchor.fPositionZ;(void)Try_StartVehicleSkill(guide,command);break;}}}
   if(guide.eVehicleFlightPhase==VEHICLE_FLIGHT_PHASE::FLYING){const float yaw=anchor.fYawDegrees*PI/180.f;float dx=anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance-guide.fPositionX,dz=anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance-guide.fPositionZ;float length=std::hypot(dx,dz);C2S_MOVE move;move.iClientSequence=++state.Sequence;move.eIntent=PLAYER_MOVE_INTENT::VEHICLE_FLIGHT;move.fGoalX=length>.5f?dx/(std::max)(1.f,length):0;move.fGoalZ=length>.5f?dz/(std::max)(1.f,length):0;move.fVerticalInput=std::clamp(anchor.fPositionY-guide.fPositionY,-1.f,1.f);Execute_PlayerMove(guide,move);state.Action=6;state.Reason="Following replicated dragon flight";publishTrace();++it;continue;}
  }
  if(guide.eAction==PLAYER_ACTION_STATE::SKILL)if(const auto* running=m_GameplayCatalog.Find_Skill(guide.iCurrentSkillId);running&&running->eSkillKind==PLAYER_SKILL_KIND::HOLD&&guide.fActionElapsedSeconds>=1.f){C2S_RELEASE_SKILL release;release.iClientSequence=++state.Sequence;release.iSkillId=guide.iCurrentSkillId;m_PlayerSkillSystem.Release(guide,release,m_GameplayCatalog);}
  if(!state.PendingComboId.empty()&&guide.eAction==PLAYER_ACTION_STATE::NONE){state.ComboId=std::move(state.PendingComboId);state.PendingComboId.clear();state.ComboStep=0;state.ComboElapsed=state.StepElapsed=0;}
  auto combo=m_GuideCatalog.Find_Combo(state.ComboId);if(combo&&state.ComboElapsed*1000>=combo->TimeoutMs){state.ComboId.clear();combo=nullptr;state.Reason="Combo total deadline reached";}
  // Predict actual published combat-object shapes at bounded future samples.
  auto danger=[&](float x,float z){float risk=Predict_GuideContactRisk(guide,x,z);for(const auto& object:m_CombatObjectRuntime.Get_LiveObjects()){if(object.eSourceKind==SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER)continue;const auto& pose=object.LiveState.CurrentPose;if(std::abs(pose.fPositionY-guide.fPositionY)>3.f)continue;for(const auto& hit:object.Hits){if(hit.iEndMs&&object.fElapsedMilliseconds>hit.iEndMs)continue;if(hit.iAtMs>object.fElapsedMilliseconds+m_GuideCatalog.HorizonSeconds*1000)continue;for(int sample=0;sample<3;++sample){const float t=m_GuideCatalog.HorizonSeconds*sample*.5f;float ox=pose.fPositionX+pose.fDirectionX*object.fSpeedMps*t,oz=pose.fPositionZ+pose.fDirectionZ*object.fSpeedMps*t;if(CServerCombatGeometry::Overlaps_Pose(hit.Shape,ox,oz,pose.fDirectionX,pose.fDirectionZ,{x,z,.5f})){risk+=(hit.bInstantDeath?5.f:1.f);break;}}}}return risk;};
  auto pathSafe=[&](const SERVER_NAV_POINT& goal,float currentRisk){
   std::vector<SERVER_NAV_POINT> path;
   if(m_ServerNavigation.Is_Loaded()){
    if(!m_ServerNavigation.Find_Path(guide.fPositionX,guide.fPositionZ,goal.x,goal.z,path,guide.fPositionY))return false;
    m_ServerNavigation.Smooth_Path(guide.fPositionX,guide.fPositionZ,goal.x,goal.z,path,guide.fPositionY);
   }else path.push_back(goal);
   SERVER_NAV_POINT from{guide.fPositionX,guide.fPositionY,guide.fPositionZ};unsigned samples=0;
   for(const auto& to:path){const float length=std::hypot(to.x-from.x,to.z-from.z);const unsigned count=(std::max)(1u,static_cast<unsigned>(std::ceil(length/.5f)));if(samples+count>128)return false;for(unsigned i=1;i<=count;++i){float t=float(i)/count;if(danger(from.x+(to.x-from.x)*t,from.z+(to.z-from.z)*t)>currentRisk+.001f)return false;}samples+=count;from=to;}return true;
  };
  const float risk=danger(guide.fPositionX,guide.fPositionZ);const auto& weight=combo?m_GuideCatalog.AssistWeights:m_GuideCatalog.FollowWeights;
  const bool needsFollow=(state.ReturningOnFoot&&anchorDistance>m_GuideCatalog.MaximumDistance)||anchorDistance>m_GuideCatalog.ResumeDistance||(guide.hasMoveGoal&&anchorDistance>m_GuideCatalog.MaximumDistance)||anchorDistance<m_GuideCatalog.MinimumDistance;
  const float distanceError=anchorDistance<m_GuideCatalog.MinimumDistance?m_GuideCatalog.MinimumDistance-anchorDistance:anchorDistance-m_GuideCatalog.MaximumDistance;
  const float followScore=(needsFollow?weight.Follow:0.f)*std::clamp(distanceError/(std::max)(1.f,m_GuideCatalog.ResumeDistance),0.f,1.f);
  const float evadeScore=weight.Avoid*std::clamp(risk,0.f,1.f);const float combatScore=combo&&enemy?weight.Attack:0;
  const bool lethal=risk>=5||(risk>0&&guide.iCurrentHp<guide.iMaximumHp*m_GuideCatalog.LethalHpFraction);evaluatedThreat=risk;survivalOverride=lethal;
  std::uint8_t action=lethal||evadeScore>(std::max)(followScore,combatScore)?2:combatScore>followScore?3:1;
  const float scores[]={0,followScore,evadeScore,combatScore};if(!lethal&&action!=state.Action&&state.Action>=1&&state.Action<=3&&state.HoldElapsed<m_GuideCatalog.MinimumHoldSeconds&&scores[action]<scores[state.Action]+m_GuideCatalog.SwitchMargin)action=state.Action;
  if(action!=state.Action){state.Action=action;state.HoldElapsed=0;}
  if(action==2){float bestRisk=risk;SERVER_NAV_POINT best{};bool found=false;for(unsigned i=0;i<16;++i){const float angle=i*PI/8.f;SERVER_NAV_POINT p;if(!Find_GuideLanding(guide,guide.fPositionX+std::sin(angle)*3.f,guide.fPositionY,guide.fPositionZ+std::cos(angle)*3.f,p))continue;const float candidateRisk=danger(p.x,p.z);if(candidateRisk<bestRisk&&pathSafe(p,risk)){bestRisk=candidateRisk;best=p;found=true;}}if(found){C2S_MOVE move;move.iClientSequence=++state.Sequence;move.fGoalX=best.x;move.fGoalZ=best.z;Execute_PlayerMove(guide,move);state.Reason=lethal?"Lethal contact predicted; survival override":"Evade has the highest weighted score";}else state.Reason="Danger detected; no lower-risk navigation candidate";}
  else if(action==3&&combo&&enemy)
  {
   if(state.ComboStep>=combo->Skills.size()){if(combo->Repeat){state.ComboStep=0;state.StepElapsed=0;}else{state.ComboId.clear();state.Reason="Combo completed";}}
   else if(guide.eAction==PLAYER_ACTION_STATE::NONE){const auto* skill=m_GameplayCatalog.Find_Skill(combo->Skills[state.ComboStep]);const float range=skill?(std::max)(2.f,skill->fMaximumRange):2.f;if(enemyDistance>range){C2S_MOVE move;move.iClientSequence=++state.Sequence;const float stop=(std::max)(1.f,range*.75f);move.fGoalX=enemy->fPositionX+(guide.fPositionX-enemy->fPositionX)*stop/enemyDistance;move.fGoalZ=enemy->fPositionZ+(guide.fPositionZ-enemy->fPositionZ)*stop/enemyDistance;Execute_PlayerMove(guide,move);state.Reason="Approaching the next combo skill range";}
    else {C2S_USE_SKILL command;command.iClientSequence=++state.Sequence;command.iSkillId=combo->Skills[state.ComboStep];command.eTargetIntent=skill?skill->eTargetIntent:SKILL_TARGET_INTENT_KIND::AIM_POINT;command.fAimX=enemy->fPositionX;command.fAimZ=enemy->fPositionZ;if(Execute_PlayerSkill(guide,command)){++state.ComboStep;state.StepElapsed=0;state.Reason="Skill admitted by the shared player executor";}else {state.StepElapsed+=elapsed;state.Reason="Skill waiting: cooldown, resource or current status";if(state.StepElapsed*1000>=combo->StepWaitMs){state.ComboId.clear();state.Reason="Unavailable skill exceeded its wait deadline";}}}}
   else state.Reason="Current skill is still executing";
  }
  else if(needsFollow){const float yaw=anchor.fYawDegrees*PI/180.f;C2S_MOVE move;move.iClientSequence=++state.Sequence;move.fGoalX=anchor.fPositionX-std::sin(yaw)*m_GuideCatalog.DesiredDistance;move.fGoalZ=anchor.fPositionZ-std::cos(yaw)*m_GuideCatalog.DesiredDistance;Execute_PlayerMove(guide,move);state.Reason="Maintaining the configured anchor distance";}
  else {if(guide.eAction==PLAYER_ACTION_STATE::NONE){guide.hasMoveGoal=false;guide.MovePath.clear();}state.Reason="Within the configured following band";}
  state.FollowScore=followScore;state.EvadeScore=evadeScore;state.CombatScore=combatScore;publishTrace();
  ++it;
 }
}
