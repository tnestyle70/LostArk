#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include <algorithm>
#include <memory>
#include <cmath>
#include <array>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_GuideAI()
{
 TESTS tests;
 auto source=std::make_shared<CGameRoom>(WORLD_ID::BERN);
 auto target=std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
 tests.Require(source->Is_Ready()&&target->Is_Ready()&&source->m_GuideCatalog.Loaded&&target->m_GuideCatalog.Loaded,"Guide published catalogs load in Bern and Valtan");
 if(!source->Is_Ready()||!target->Is_Ready()||!source->m_GuideCatalog.Loaded||!source->m_iGuideReceptionId)return 1;
 const auto reception=source->m_Players.at(source->m_iGuideReceptionId);
 const auto guideId=reception.iPlayerId;
 tests.Require(reception.Is_Guide()&&!reception.isCombatReady&&reception.iSessionId==INVALID_SESSION_ID&&source->Count_HumanPlayers()==0,"One placed guide has no fake session or human slot");
 tests.Require(std::none_of(target->m_Players.begin(),target->m_Players.end(),[](const auto& p){return p.second.Is_Guide();}),"A raid room contains no guide actor");
 std::vector<std::shared_ptr<CClientSession>> sessions;
 auto drain=[&](){for(auto& session:sessions){session->m_OutboundFrames.clear();session->m_iQueuedOutboundBytes=0;session->m_OutboundMetrics.iCurrentQueuedByteCount=0;session->m_OutboundMetrics.iCurrentQueuedFrameCount=0;}};
 auto hasFrame=[](const auto& session,PACKET_TYPE type){return std::any_of(session->m_OutboundFrames.begin(),session->m_OutboundFrames.end(),[&](const auto& f){return f.ePacketType==type;});};
 auto lastState=[](const auto& session){S2C_GUIDE_STATE result;for(const auto& frame:session->m_OutboundFrames)if(frame.ePacketType==PACKET_TYPE::S2C_GUIDE_STATE){CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};(void)Read_Message(reader,result);}return result;};
 bool joined=true;
 for(unsigned i=0;i<4;++i){auto session=std::make_shared<CClientSession>(99101u+i,INVALID_SOCKET,CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});session->m_isSendRunning.store(true);source->Handle_Register(session);C2S_ENTER_WORLD enter;enter.eWorldId=WORLD_ID::BERN;enter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;enter.strNickName="GuideContract"+std::to_string(i);joined=source->Join(session->Get_SessionId(),enter)&&joined;sessions.push_back(session);drain();}
 tests.Require(joined&&source->Count_HumanPlayers()==4,"Four human admissions remain available beside the single guide");
 if(!joined)return 1;
 auto& leader=source->m_Players.at(sessions[0]->Get_PlayerId());
 leader.fPositionX=reception.fPositionX+2.f;leader.fPositionY=reception.fPositionY;leader.fPositionZ=reception.fPositionZ;
 const auto ownerId=leader.iSessionId;
 auto control=[&](SESSION_ID owner,std::uint32_t sequence,GUIDE_CONTROL_ACTION action){C2S_GUIDE_CONTROL request;request.iRequestSequence=sequence;request.iGuideNetEntityId=reception.iNetEntityId;request.eAction=action;source->Handle_GuideControl(owner,request);};
 const auto actorCount=source->m_Players.size();
 C2S_PARTY_INVITE legacy;legacy.iTargetNetEntityId=reception.iNetEntityId;source->Handle_PartyInvite(ownerId,legacy);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_PartyMembersByPartyId.empty(),"A legacy party invitation cannot create guidance or a hidden party");
 GUIDE_TRIGGER startBox;startBox.Id="guide.contract.start-inside";startBox.Type="SPACE_ENTER";startBox.Category=source->m_GuideCatalog.Categories.at(WORLD_ID::BERN);startBox.PromptId="guide.contract.start-space";
 startBox.Box.fPositionX=leader.fPositionX;startBox.Box.fPositionY=leader.fPositionY+1.f;startBox.Box.fPositionZ=leader.fPositionZ;startBox.Box.fHalfExtentX=startBox.Box.fHalfExtentY=startBox.Box.fHalfExtentZ=.5f;
 source->m_GuideCatalog.Prompts.push_back({startBox.PromptId,{{"Existing contact is not entry",1000u}}});source->m_GuideCatalog.Triggers.push_back(startBox);
 control(ownerId,1,GUIDE_CONTROL_ACTION::START);
 tests.Require(source->m_PersonalGuides.size()==1&&source->m_PersonalGuides.at(ownerId).PlayerId==guideId&&source->m_Players.size()==actorCount&&source->m_PartyMembersByPartyId.empty(),"Typed start binds the placed actor without a clone or internal party");
 if(source->m_PersonalGuides.empty())return 1;
 control(ownerId,2,GUIDE_CONTROL_ACTION::START);
 tests.Require(source->m_Players.size()==actorCount&&source->m_PersonalGuides.at(ownerId).PromptQueue.size()==1,"Duplicate start preserves the actor and one lowercase-category greeting");
 control(sessions[1]->Get_SessionId(),1,GUIDE_CONTROL_ACTION::START);
 control(sessions[1]->Get_SessionId(),2,GUIDE_CONTROL_ACTION::STOP);
 tests.Require(source->m_PersonalGuides.size()==1&&source->m_PersonalGuides.contains(ownerId),"Another human cannot take over or stop an occupied guide");
 tests.Require(lastState(sessions[1]).iOwnerNetEntityId==leader.iNetEntityId,"All Bern observers receive the occupied owner identity");
 source->Update_Guides(.2f);
 tests.Require(hasFrame(sessions[0],PACKET_TYPE::S2C_GUIDE_PROMPT)&&!hasFrame(sessions[1],PACKET_TYPE::S2C_GUIDE_PROMPT),"Prepared greeting is delivered only to the owner");
 const auto firstPrompt=std::find_if(sessions[0]->m_OutboundFrames.begin(),sessions[0]->m_OutboundFrames.end(),[](const auto& f){return f.ePacketType==PACKET_TYPE::S2C_GUIDE_PROMPT;});
 tests.Require(firstPrompt!=sessions[0]->m_OutboundFrames.end()&&std::any_of(sessions[0]->m_OutboundFrames.begin(),firstPrompt,[](const auto& f){return f.ePacketType==PACKET_TYPE::S2C_GUIDE_STATE;}),"Ownership state is queued before the first greeting");
 tests.Require(source->m_PersonalGuides.at(ownerId).InsideBoxes.contains(startBox.Id)&&
  std::none_of(source->m_PersonalGuides.at(ownerId).PromptQueue.begin(),source->m_PersonalGuides.at(ownerId).PromptQueue.end(),[&](const auto& queued){return queued.TriggerId==startBox.Id;}),
  "Starting guidance inside a space seeds contact and emits no invented entry prompt");
 source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Prompts.pop_back();source->m_PersonalGuides.at(ownerId).InsideBoxes.erase(startBox.Id);
 const auto firstEventSequence=source->m_iGuideEventSequence;
 drain();
 auto& state=source->m_PersonalGuides.at(ownerId);
 auto& guide=source->m_Players.at(guideId);
 const auto command=std::find_if(source->m_GuideCatalog.Commands.begin(),source->m_GuideCatalog.Commands.end(),[](const auto& c){return c.Enabled&&!c.Stop;});
 tests.Require(command!=source->m_GuideCatalog.Commands.end(),"Published help command is present");
 if(command!=source->m_GuideCatalog.Commands.end())
 {
  source->Guide_ChatCommand(source->m_Players.at(sessions[1]->Get_PlayerId()),command->Aliases.front());tests.Require(state.ComboId.empty(),"A non-owner cannot command the singleton guide");
  source->Guide_ChatCommand(leader,"prefix "+command->Aliases.front());tests.Require(state.ComboId.empty(),"A substring does not activate a help command");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboId==command->ComboId&&state.ComboStep==0,"Exact owner help command stages its published combo");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboStep==0&&state.PendingComboId.empty(),"Duplicate active combo does not restart or queue");
 }
 const auto* combo=source->m_GuideCatalog.Find_Combo(state.ComboId);
 if(combo&&!combo->Skills.empty())
 {
  SERVER_WORLD_ENTITY enemy;enemy.iNetEntityId=900001;enemy.eKind=WORLD_BOOTSTRAP_KIND::MONSTER;enemy.iCurrentHp=enemy.iMaximumHp=1000;enemy.fPositionX=guide.fPositionX;enemy.fPositionY=guide.fPositionY;enemy.fPositionZ=guide.fPositionZ+1;enemy.eAction=SERVER_ENTITY_ACTION::PATTERN_WINDUP;
  source->m_WorldEntities.push_back(enemy);source->Update_Guides(.2f);
  tests.Require(state.Action==3&&state.ComboStep==1&&guide.iCurrentSkillId==combo->Skills.front()&&guide.eAction==PLAYER_ACTION_STATE::SKILL,"Owner help executes a real skill through the common decision loop");
  source->m_WorldEntities.pop_back();
 }
 source->Reset_PlayerForDebugTeleport(guide);state.ComboId.clear();state.PendingComboId.clear();state.PromptQueue.clear();state.PromptRemaining=100.f;state.HoldElapsed=10.f;
 // Put only the owner collider in a tiny authored-style box, away from the guide.
 const auto ownerPose=std::array{leader.fPositionX,leader.fPositionY,leader.fPositionZ};
 GUIDE_TRIGGER box;box.Id="guide.contract.owner-box";box.Type="SPACE_ENTER";box.Category=source->m_GuideCatalog.Categories.at(WORLD_ID::BERN);box.PromptId=source->m_GuideCatalog.Prompts.front().Id;box.Box.fPositionX=guide.fPositionX+12.f;box.Box.fPositionY=guide.fPositionY+1.f;box.Box.fPositionZ=guide.fPositionZ;box.Box.fHalfExtentX=box.Box.fHalfExtentY=box.Box.fHalfExtentZ=.5f;
 source->m_GuideCatalog.Triggers.push_back(box);leader.fPositionX=box.Box.fPositionX;leader.fPositionZ=box.Box.fPositionZ;source->Update_Guides(.2f);
 tests.Require(state.InsideBoxes.contains(box.Id),"Bern SPACE_ENTER uses the human collider while the guide is outside");
 state.InsideBoxes.erase(box.Id);leader.fPositionX=guide.fPositionX;leader.fPositionZ=guide.fPositionZ;guide.fPositionX=box.Box.fPositionX;source->Update_Guides(.2f);
 tests.Require(!state.InsideBoxes.contains(box.Id),"Guide-only contact cannot trigger the owner's Bern space event");
 source->m_GuideCatalog.Triggers.pop_back();guide.fPositionX=reception.fPositionX;guide.fPositionY=reception.fPositionY;guide.fPositionZ=reception.fPositionZ;leader.fPositionX=ownerPose[0];leader.fPositionY=ownerPose[1];leader.fPositionZ=ownerPose[2];source->Reset_PlayerForDebugTeleport(guide);
 // Decode actual outgoing dialogue for location cancellation, ordering and arrival boundaries.
 {
  const auto savedCatalog=source->m_GuideCatalog;const auto savedState=state;const auto savedGuide=guide;const auto savedOwner=leader;const auto savedTick=source->m_iServerTick;
  source->m_GuideCatalog.Triggers.clear();state.PromptQueue.clear();state.TriggerTicks.clear();state.InsideBoxes.clear();state.ThinkElapsed=0.f;state.PromptRemaining=6.f;state.ReturningOnFoot=false;source->m_iServerTick=100u;
  GUIDE_TRIGGER local=box;local.Id="guide.contract.queued-space";local.PromptId="guide.contract.location";local.CooldownMs=30000;local.Priority=10;
  local.Box.fPositionX=leader.fPositionX+8.f;local.Box.fPositionY=leader.fPositionY+1.f;local.Box.fPositionZ=leader.fPositionZ;
  source->m_GuideCatalog.Prompts.push_back({local.PromptId,{{"Location",1000u}}});source->m_GuideCatalog.Triggers.push_back(local);
  auto step=[&](float seconds){++source->m_iServerTick;source->Update_Guides(seconds);};
  auto enter=[&](){leader.fPositionX=local.Box.fPositionX;leader.fPositionZ=local.Box.fPositionZ;};
  auto leave=[&](){leader.fPositionX=local.Box.fPositionX+3.f;leader.fPositionZ=local.Box.fPositionZ;};
  auto received=[&](){std::vector<S2C_GUIDE_PROMPT> messages;for(const auto& frame:sessions[0]->m_OutboundFrames)if(frame.ePacketType==PACKET_TYPE::S2C_GUIDE_PROMPT){S2C_GUIDE_PROMPT message;CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};if(Read_Message(reader,message))messages.push_back(std::move(message));}return messages;};
  drain();leave();step(.2f);enter();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==local.Id&&!state.TriggerTicks.contains(local.Id),"A waiting location prompt retains its source without spending cooldown");
  leave();step(.2f);
  tests.Require(state.PromptQueue.empty()&&!state.TriggerTicks.contains(local.Id)&&received().empty(),"Leaving a location cancels only its unsaid prompt without sending stale dialogue");
  enter();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().PromptId==local.PromptId,"Re-entering a cancelled location can immediately reserve its dialogue again");
  state.PromptRemaining=0.f;step(.01f);auto messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(local.Id)&&state.TriggerTicks.at(local.Id)==source->m_iServerTick,"The first actual location segment is delivered and starts cooldown");
  drain();leave();step(.2f);enter();step(.2f);
  tests.Require(state.PromptQueue.empty(),"An already spoken location still respects its re-entry cooldown");

  // Moving directly between two shops with shared text replaces the old provenance.
  state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER nextSpace=local;nextSpace.Id="guide.contract.shared-next";nextSpace.Box.fPositionX+=8.f;
  GUIDE_TRIGGER overlap=nextSpace;overlap.Id="guide.contract.shared-overlap";
  source->m_GuideCatalog.Triggers.push_back(nextSpace);source->m_GuideCatalog.Triggers.push_back(overlap);
  enter();source->Queue_GuidePrompt(ownerId,local);leader.fPositionX=nextSpace.Box.fPositionX;source->Queue_GuidePrompt(ownerId,nextSpace);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&!state.TriggerTicks.contains(local.Id),"Same-text entry at a new shop replaces an unsaid departed source before deduplication");
  source->Queue_GuidePrompt(ownerId,overlap);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&state.PromptQueue.front().SpaceTriggerIds.size()==2,"Overlapping current spaces retain both fired sources in only one dialogue");
  source->Seed_GuideSpaceEntries(ownerId,leader);state.PromptRemaining=0.f;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(nextSpace.Id)&&state.TriggerTicks.contains(overlap.Id)&&!state.TriggerTicks.contains(local.Id),"Shared text speaks once and starts cooldown for every valid source that actually fired");
  // Both spaces really fire, then only the chosen source is left before speaking.
  drain();state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();state.InsideBoxes.clear();
  nextSpace.Box.fHalfExtentX=2.f;nextSpace.Priority=40;overlap.Box.fHalfExtentX=2.f;overlap.Box.fPositionX=nextSpace.Box.fPositionX+3.f;
  source->m_GuideCatalog.Triggers[source->m_GuideCatalog.Triggers.size()-2]=nextSpace;source->m_GuideCatalog.Triggers.back()=overlap;
  GUIDE_TRIGGER unentered=overlap;unentered.Id="guide.contract.already-inside-shared";unentered.Priority=250;unentered.Box.fHalfExtentX=10.f;source->m_GuideCatalog.Triggers.push_back(unentered);
  leader.fPositionX=nextSpace.Box.fPositionX-4.f;source->Seed_GuideSpaceEntries(ownerId,leader);step(.2f);leader.fPositionX=nextSpace.Box.fPositionX+1.5f;step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==nextSpace.Id&&state.PromptQueue.front().SpaceTriggerIds.size()==2,"Both actual overlap entry sources are retained without adopting a higher-priority unfired contact");
  leader.fPositionX=overlap.Box.fPositionX+1.5f;step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==overlap.Id&&state.PromptQueue.front().Priority==overlap.Priority&&state.InsideBoxes.contains(overlap.Id),"Leaving the preferred source preserves the already-fired overlapping source without requiring re-entry");
  state.PromptRemaining=0.f;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId&&state.TriggerTicks.contains(overlap.Id)&&!state.TriggerTicks.contains(nextSpace.Id),"The surviving shared source speaks once and the departed unsaid source spends no cooldown");
  source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Triggers.pop_back();source->m_GuideCatalog.Triggers.pop_back();drain();enter();source->Seed_GuideSpaceEntries(ownerId,leader);

  // An unrelated high-priority trigger sharing a prompt must not promote this occurrence.
  state.PromptQueue.clear();state.PromptRemaining=6.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER dormant=local;dormant.Id="guide.contract.unfired-priority";dormant.Priority=250;dormant.Box.fPositionX+=50.f;
  GUIDE_TRIGGER greeting=local;greeting.Id="guide.contract.queue-greeting";greeting.Type="GUIDE_STARTED";greeting.PromptId="guide.contract.greeting";greeting.Priority=100;
  source->m_GuideCatalog.Prompts.push_back({greeting.PromptId,{{"Greeting",1000u}}});source->m_GuideCatalog.Triggers.push_back(dormant);source->m_GuideCatalog.Triggers.push_back(greeting);
  source->Queue_GuidePrompt(ownerId,local);source->Queue_GuidePrompt(ownerId,greeting);
  tests.Require(state.PromptQueue.size()==2&&state.PromptQueue.front().PromptId==greeting.PromptId&&state.PromptQueue.back().Priority==10,"Queue priority belongs to the fired trigger rather than every trigger sharing its prompt");
  state.PromptRemaining=0.f;leave();step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==greeting.PromptId&&state.PromptQueue.empty(),"Cancelling a stale space leaves its greeting deliverable in the same update");

  // Guide-only arrival preserves contact; real owner relocation into a new space still fires.
  drain();state.PromptQueue.clear();state.PromptRemaining=0.f;state.TriggerTicks.clear();enter();source->Seed_GuideSpaceEntries(ownerId,leader);source->Guide_AnchorArrived(leader);
  step(.2f);
  tests.Require(state.InsideBoxes.contains(local.Id)&&state.PromptQueue.empty()&&received().empty(),"Repeated guide arrival preserves owner contact without inventing another entry");
  leave();step(.2f);enter();source->Guide_AnchorArrived(leader,true);step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==local.Id,"Committed local map travel from outside into a new space preserves its real entry event");
  guide.fPositionX=leader.fPositionX+2.f;guide.fPositionY=leader.fPositionY;guide.fPositionZ=leader.fPositionZ;step(.01f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==local.PromptId,"Actual local map travel into a shop delivers its location dialogue once");
  drain();state.PromptRemaining=0.f;state.TriggerTicks.clear();
  GUIDE_TRIGGER returned=greeting;returned.Id="guide.contract.return";returned.Type="RAID_RETURNED";returned.PatternId="VALTAN_ARENA";returned.PromptId="guide.contract.return-text";
  source->m_GuideCatalog.Prompts.push_back({returned.PromptId,{{"Returned",1000u}}});source->m_GuideCatalog.Triggers.push_back(returned);
  state.WaitingForOwner=true;state.InsideBoxes.clear();source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);
  tests.Require(state.InsideBoxes.contains(local.Id)&&state.PromptQueue.size()==1&&state.PromptQueue.front().TriggerId==returned.Id,"World return seeds nearby spaces while reserving its genuine return dialogue");
  leave();guide.fPositionX=leader.fPositionX+2.f;guide.fPositionY=leader.fPositionY;guide.fPositionZ=leader.fPositionZ;step(.2f);messages=received();
  tests.Require(messages.size()==1&&messages.front().strPromptId==returned.PromptId,"Return dialogue is preserved after leaving the arrival space");

  // Once the first segment starts, finishing the authored sentence remains deterministic.
  drain();state.PromptQueue.clear();state.PromptRemaining=0.f;state.TriggerTicks.clear();state.ReturningOnFoot=false;
  GUIDE_TRIGGER multi=local;multi.Id="guide.contract.multisegment";multi.PromptId="guide.contract.multisegment-text";
  source->m_GuideCatalog.Prompts.push_back({multi.PromptId,{{"First segment",1000u},{"Second segment",1000u}}});source->m_GuideCatalog.Triggers.push_back(multi);
  enter();source->Seed_GuideSpaceEntries(ownerId,leader);source->Queue_GuidePrompt(ownerId,multi);step(.01f);const auto speechTick=source->m_iServerTick;
  leave();step(.2f);
  tests.Require(state.PromptQueue.size()==1&&state.PromptQueue.front().NextSegment==1,"Leaving a space does not discard a dialogue whose first segment already started");
  step(1.f);messages=received();
  tests.Require(messages.size()==2&&messages[0].strText=="First segment"&&messages[1].strText=="Second segment"&&state.TriggerTicks.contains(multi.Id)&&state.TriggerTicks.at(multi.Id)==speechTick,"Started multi-segment dialogue finishes in order without restarting its cooldown");
  source->m_GuideCatalog=savedCatalog;state=savedState;guide=savedGuide;leader=savedOwner;source->m_iServerTick=savedTick;drain();
 }
 // The admitted dragon and three-axis common executor remain unchanged.
 C2S_SET_VEHICLE_RIDING riding;riding.eWorldId=WORLD_ID::BERN;riding.iRequestSequence=100;riding.iVehicleId=ANCIENT_SEA_VEHICLE_ID;
 (void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);
 tests.Require(leader.iVehicleId==ANCIENT_SEA_VEHICLE_ID&&guide.iVehicleId==leader.iVehicleId,"Guide mounts the same admitted dragon as the owner");
 leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::FLYING;const float anchorY=leader.fPositionY;leader.fPositionY=guide.fPositionY+2;
 source->Update_Guides(.2f);tests.Require(state.Action==6&&guide.fVehicleFlightInputY>0&&guide.iLastMoveSequence>0,"Airborne guide emits admitted three-axis flight follow input");
 leader.fPositionY=anchorY;leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::GROUNDED;riding.iRequestSequence=200;riding.iVehicleId=INVALID_VEHICLE_ID;(void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);drain();
 // bShipDockValid is set only by the successful server boarding contract.
 const auto pierPose=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
 leader.bShipDockValid=true;leader.iVehicleId=8200;leader.fPositionX+=100.f;guide.hasMoveGoal=true;guide.fMoveGoalX=guide.fPositionX+10.f;
 source->Update_Guides(.2f);source->Update_Players(.5f);
 tests.Require(state.WaitingForShip&&!guide.hasMoveGoal&&guide.iVehicleId!=8200&&guide.fPositionX==pierPose[0]&&guide.fPositionY==pierPose[1]&&guide.fPositionZ==pierPose[2],"Successful ship boarding holds the same guide at the pier without copying the ship");
 leader.bShipDockValid=false;leader.iVehicleId=INVALID_VEHICLE_ID;source->Update_Guides(.2f);
 tests.Require(!state.WaitingForShip&&state.ReturningOnFoot&&guide.fPositionX==pierPose[0],"Disembarking resumes an on-foot approach without relocating the guide");
 leader.fPositionX=ownerPose[0];leader.fPositionZ=ownerPose[2];source->Reset_PlayerForDebugTeleport(guide);state.ReturningOnFoot=false;
 // Local map travel follows the committed owner once; it is not a ship/world departure.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;const auto savedTick=source->m_iServerTick;
  for(const std::uint16_t destination : {std::uint16_t{1u},std::uint16_t{2u},std::uint16_t{3u},WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID})
  {
   source->Reset_PlayerForDebugTeleport(leader);source->Reset_PlayerForDebugTeleport(guide);
   const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
   C2S_USE_SQUAREHOLE travel;travel.iClientSequence=1000u+destination;travel.iSquareHoleId=destination;
   source->Handle_UseSquareHole(ownerId,travel);
   const bool admitted=leader.eAction==PLAYER_ACTION_STATE::SQUAREHOLE_SONG;
   tests.Require(admitted&&guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2],
    "SquareHole and Set Sail preparation preserve the guide until the real song commits");
   if(!admitted)continue;
   const auto duration=(SQUAREHOLE_SONG_DURATION_MS*30u+999u)/1000u+(SQUAREHOLE_BLACKOUT_HOLD_MS*30u+999u)/1000u;
   source->m_iServerTick=leader.iActionStartTick+duration-1u;source->Update_Players(1.f/30.f);
   SERVER_NAV_POINT expected;const bool landed=source->Resolve_SquareHoleDestination(leader,destination,expected)&&
    std::hypot(leader.fPositionX-expected.x,leader.fPositionZ-expected.z)<.01f;
   tests.Require(landed&&guide.iNetEntityId==reception.iNetEntityId&&state.PlayerId==guideId&&state.AnchorId==leader.iPlayerId&&
    !state.WaitingForOwner&&!state.WaitingForShip&&!leader.bShipDockValid&&source->m_PendingWorldTransfers.empty()&&
    std::abs(guide.fPositionY-leader.fPositionY)<=2.f&&std::hypot(guide.fPositionX-leader.fPositionX,guide.fPositionZ-leader.fPositionZ)<=6.01f&&
    source->m_ServerCollisionSystem.Is_PlayerPositionClear(guide.fPositionX,guide.fPositionY,guide.fPositionZ,guide.iNetEntityId)&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY),
    "Committed Bern map travel brings the same owned guide to a validated nearby landing before any ship boarding");
   drain();
  }
  const auto refusedOwner=std::array{leader.fPositionX,leader.fPositionZ},refusedGuide=std::array{guide.fPositionX,guide.fPositionZ};
  C2S_USE_SQUAREHOLE invalid;invalid.iClientSequence=70000u;invalid.iSquareHoleId=65534u;source->Handle_UseSquareHole(ownerId,invalid);
  tests.Require(leader.eAction==PLAYER_ACTION_STATE::NONE&&leader.fPositionX==refusedOwner[0]&&leader.fPositionZ==refusedOwner[1]&&
   guide.fPositionX==refusedGuide[0]&&guide.fPositionZ==refusedGuide[1],"Rejected map travel never relocates its owner or guide");
  leader=savedOwner;guide=savedGuide;state=savedState;source->m_iServerTick=savedTick;drain();
 }
 // Check every heading at the actual harbour destination, including offsets across nav seams.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;
  SERVER_NAV_POINT harbour;const bool resolved=source->Resolve_SquareHoleDestination(leader,WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID,harbour);
  bool connected=resolved;
  if(resolved)for(unsigned heading=0;heading<24;++heading)
  {
   leader.fPositionX=harbour.x;leader.fPositionY=harbour.y;leader.fPositionZ=harbour.z;leader.fYawDegrees=heading*15.f;
   source->Guide_AnchorArrived(leader,true);
   connected=connected&&!state.ReturningOnFoot&&guide.iNetEntityId==reception.iNetEntityId&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY)&&
    source->m_ServerCollisionSystem.Is_PlayerPositionClear(guide.fPositionX,guide.fPositionY,guide.fPositionZ,guide.iNetEntityId);
  }
  tests.Require(connected,"All twenty-four harbour arrival headings keep the guide on collision-clear connected ground");
  leader=savedOwner;guide=savedGuide;state=savedState;
  const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
  guide.hasMoveGoal=true;state.ComboId="guide.contract.preserve";
  leader.fPositionX=100000.f;leader.fPositionZ=100000.f;
  source->Guide_AnchorArrived(leader,true);
  tests.Require(guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2]&&
   guide.hasMoveGoal&&state.ComboId=="guide.contract.preserve",
   "A local arrival without navigation preserves the guide pose, active goal and pending state");
  leader=savedOwner;guide=savedGuide;state=savedState;
 }
 // Real published building triggers must relocate the singleton only on completed motion.
 {
  const auto savedOwner=leader,savedGuide=guide;const auto savedState=state;const auto savedTick=source->m_iServerTick;
  const auto savedTriggers=source->m_ServerTriggerSystem;
  for(const char* id:{"castle","castle.2","library","library.2"})
  {
   leader=savedOwner;guide=savedGuide;state=savedState;source->m_ServerTriggerSystem=savedTriggers;
   source->Reset_PlayerForDebugTeleport(leader);source->Reset_PlayerForDebugTeleport(guide);
   const auto* box=source->Find_Placement(id);
   tests.Require(box&&box->isEnabled&&box->TriggerActions.size()==1u&&
    box->TriggerActions.front().eKind==WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER,
    "Published castle/library entry and exit have their actual authored local travel");
   if(!box||box->TriggerActions.size()!=1u)continue;
   leader.fPositionX=box->fPositionX;leader.fPositionY=box->fPositionY;leader.fPositionZ=box->fPositionZ;
   std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
   source->m_ServerTriggerSystem.Evaluate_Entries(source->m_Players,++source->m_iServerTick,transfers,{},prompts);
   const auto before=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
   const bool started=leader.TriggerMove.isActive&&leader.TriggerMove.strSourcePlacementId==id;
   source->Update_Players(.1f);
   tests.Require(started&&leader.TriggerMove.isActive&&guide.fPositionX==before[0]&&guide.fPositionY==before[1]&&guide.fPositionZ==before[2],
    "Building blackout hold never relocates the guide early");
   for(unsigned step=0;step<120&&leader.TriggerMove.isActive;++step){++source->m_iServerTick;source->Update_Players(1.f/30.f);}
   const auto& move=box->TriggerActions.front();
   tests.Require(started&&!leader.TriggerMove.isActive&&std::hypot(leader.fPositionX-move.fTargetX,leader.fPositionZ-move.fTargetZ)<.01f&&
    !state.ReturningOnFoot&&guide.iNetEntityId==reception.iNetEntityId&&
    source->m_ServerNavigation.Is_PointWalkableExact(guide.fPositionX,guide.fPositionZ,guide.fPositionY)&&
    source->m_ServerNavigation.Has_LineOfSight(leader.fPositionX,leader.fPositionZ,guide.fPositionX,guide.fPositionZ,leader.fPositionY)&&
    std::hypot(guide.fPositionX-leader.fPositionX,guide.fPositionZ-leader.fPositionZ)<=6.01f&&
    !state.WaitingForShip&&!state.WaitingForOwner&&transfers.empty(),
    "Completed castle/library entry and exit bring the existing guide onto connected local ground");
   drain();
  }
  leader=savedOwner;guide=savedGuide;state=savedState;source->m_iServerTick=savedTick;source->m_ServerTriggerSystem=savedTriggers;drain();
 }
 // Existing four-human parties still enter a raid; the guide is outside that transaction.
 for(unsigned i=1;i<4;++i){C2S_PARTY_INVITE invite;invite.iTargetNetEntityId=source->m_Players.at(sessions[i]->Get_PlayerId()).iNetEntityId;source->Handle_PartyInvite(ownerId,invite);C2S_PARTY_INVITE_RESPOND answer;answer.iFromNetEntityId=leader.iNetEntityId;answer.bAccepted=true;source->Handle_PartyInviteRespond(sessions[i]->Get_SessionId(),answer);drain();}
 tests.Require(source->m_PartyMembersByPartyId.size()==1&&source->m_PartyMembersByPartyId.begin()->second.size()==4,"The guide does not occupy or alter a four-human party");
 std::vector<SESSION_ID> batch;for(const auto& session:sessions)batch.push_back(session->Get_SessionId());
 PARTY_TRANSFER_RESULT result;std::string status;
 const auto waitingPose=std::array{guide.fPositionX,guide.fPositionY,guide.fPositionZ};
 const auto queued=state.PromptQueue;const auto nextEntity=source->m_iNextNetEntityId;
 sessions[1]->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
 const bool failed=!source->Transfer_PartyTo(*target,batch,result,status);
 tests.Require(failed&&result==PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY&&source->Count_HumanPlayers()==4&&target->Count_HumanPlayers()==0,"Actual outbound capacity rejection preserves the whole source party");
 tests.Require(!state.WaitingForOwner&&state.PromptQueue==queued&&guide.fPositionX==waitingPose[0]&&guide.iNetEntityId==reception.iNetEntityId&&source->m_iNextNetEntityId==nextEntity,"Failed transfer preserves guide owner, identity, pose and pending prompts");drain();
 const bool moved=source->Transfer_PartyTo(*target,batch,result,status);
 tests.Require(moved&&source->Count_HumanPlayers()==0&&source->m_Players.size()==1&&source->m_PersonalGuides.size()==1&&state.WaitingForOwner&&target->Count_HumanPlayers()==4&&target->m_PersonalGuides.empty(),"Four humans enter while the same single guide waits in Bern");
 if(!moved){std::cout<<"Guide transfer detail: "<<status<<'\n';return 1;}
 tests.Require(std::none_of(target->m_Players.begin(),target->m_Players.end(),[](const auto& p){return p.second.Is_Guide();}),"Successful raid entry creates no raid guide actor or fake participant");
 source->Update_Guides(10.f);source->Update_Players(1.f);
 tests.Require(guide.fPositionX==waitingPose[0]&&guide.fPositionY==waitingPose[1]&&guide.fPositionZ==waitingPose[2]&&!guide.isCombatReady&&state.ComboId.empty()&&state.PromptQueue.empty(),"Last human departure keeps an inactive guide at exactly its committed Bern pose");drain();
 const bool returned=target->Transfer_PartyTo(*source,{ownerId},result,status,"","npc.bern.beda.guide");
 tests.Require(returned,"A committed human raid return re-enters Bern");
 if(!returned){std::cout<<"Guide return detail: "<<status<<'\n';return 1;}
 auto& returnedGuide=source->m_Players.at(guideId);
 tests.Require(!state.WaitingForOwner&&state.ReturningOnFoot&&state.AnchorId==sessions[0]->Get_PlayerId()&&returnedGuide.iNetEntityId==reception.iNetEntityId&&returnedGuide.fPositionX==waitingPose[0]&&returnedGuide.fPositionZ==waitingPose[2],"Raid return rebinds the new human identity while preserving the guide identity and pose");
 const auto returnTrigger=std::find_if(source->m_GuideCatalog.Triggers.begin(),source->m_GuideCatalog.Triggers.end(),[](const auto& t){return t.Enabled&&t.Type=="RAID_RETURNED"&&t.PatternId=="VALTAN_ARENA";});
 tests.Require(returnTrigger!=source->m_GuideCatalog.Triggers.end()&&std::any_of(state.PromptQueue.begin(),state.PromptQueue.end(),[&](const auto& p){return p.PromptId==returnTrigger->PromptId;}),"Actual raid return queues the matching published lowercase-category prompt");
 const auto count=state.PromptQueue.size();source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);tests.Require(state.PromptQueue.size()==count,"A repeated resume notification cannot duplicate the return prompt");
 auto& returnedOwner=source->m_Players.at(sessions[0]->Get_PlayerId());
 const auto returnOwnerPose=std::array{returnedOwner.fPositionX,returnedOwner.fPositionY,returnedOwner.fPositionZ};returnedOwner.hasMoveGoal=false;
 // Make the actual return path fall inside the ordinary follow hysteresis band.
 const float originalResumeDistance=source->m_GuideCatalog.ResumeDistance;
 source->m_GuideCatalog.ResumeDistance=std::hypot(returnedOwner.fPositionX-returnedGuide.fPositionX,returnedOwner.fPositionZ-returnedGuide.fPositionZ)+1.f;
 source->Update_Guides(source->m_GuideCatalog.RecoverDelay+1.f);
 source->m_GuideCatalog.ResumeDistance=originalResumeDistance;
 tests.Require(returnedGuide.fPositionX==waitingPose[0]&&returnedGuide.fPositionZ==waitingPose[2]&&state.ReturningOnFoot&&returnedGuide.hasMoveGoal,"Far return uses the existing nav move goal and never the recovery teleport");
 tests.Require(returnedOwner.fPositionX==returnOwnerPose[0]&&returnedOwner.fPositionY==returnOwnerPose[1]&&returnedOwner.fPositionZ==returnOwnerPose[2]&&!returnedOwner.hasMoveGoal,"Guide approach leaves the returning human's pose and move intent untouched");
 // Stopping discards all future occurrences and does not respawn the singleton.
 tests.Require(!hasFrame(sessions[0],PACKET_TYPE::S2C_GUIDE_PROMPT),"Return speech stays queued until the guide approaches its owner");
 const auto lastSequence=returnedGuide.iLastMoveSequence;control(ownerId,1,GUIDE_CONTROL_ACTION::STOP);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_Players.contains(guideId)&&!returnedGuide.hasMoveGoal&&!returnedGuide.isCombatReady&&lastState(sessions[0]).iOwnerNetEntityId==0,"New-level STOP sequence 1 clears guidance and broadcasts idle while retaining the placed actor");
 source->Resume_PersonalGuide(ownerId,WORLD_ID::VALTAN_ARENA);tests.Require(source->m_PersonalGuides.empty(),"A stopped guide cannot resume from a stale world-return notification");
 control(ownerId,1,GUIDE_CONTROL_ACTION::START);tests.Require(source->m_PersonalGuides.empty(),"A stale START cannot undo a newer STOP");
 returnedOwner.fPositionX=returnedGuide.fPositionX+2.f;returnedOwner.fPositionY=returnedGuide.fPositionY;returnedOwner.fPositionZ=returnedGuide.fPositionZ;
 control(ownerId,2,GUIDE_CONTROL_ACTION::START);source->Update_Guides(.2f);
 tests.Require(source->m_PersonalGuides.size()==1&&returnedGuide.iNetEntityId==reception.iNetEntityId&&source->m_iGuideEventSequence>firstEventSequence&&source->m_PersonalGuides.at(ownerId).Sequence>=lastSequence,"Restart preserves actor identity, monotonic prompts and admitted internal command sequences");
 // Each authoritative return type selects only its own configured event, using the real Join commit.
 for(const auto world:{WORLD_ID::KAKULSAYDON_ARENA,WORLD_ID::MAHARAKA,WORLD_ID::COLOSSEUM}){
  source->Leave(ownerId,PLAYER_DESPAWN_REASON::LEVEL_CHANGED);drain();source->Handle_Register(sessions[0]);C2S_ENTER_WORLD enter;enter.eWorldId=WORLD_ID::BERN;enter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;enter.strNickName="GuideReturn";
  const bool admitted=source->Join(ownerId,enter,{}, {},INVALID_HONOR_TITLE_ID,{}, {},world);
  const char* name=world==WORLD_ID::KAKULSAYDON_ARENA?"KAKULSAYDON_ARENA":world==WORLD_ID::MAHARAKA?"MAHARAKA":"COLOSSEUM";
  const auto event=std::find_if(source->m_GuideCatalog.Triggers.begin(),source->m_GuideCatalog.Triggers.end(),[&](const auto& t){return t.Enabled&&(t.Type=="RAID_RETURNED"||t.Type=="WORLD_RETURNED")&&t.PatternId==name;});
  const auto& current=source->m_PersonalGuides.at(ownerId);
  tests.Require(admitted&&!current.WaitingForOwner&&event!=source->m_GuideCatalog.Triggers.end()&&std::any_of(current.PromptQueue.begin(),current.PromptQueue.end(),[&](const auto& p){return p.PromptId==event->PromptId;}),"Committed return resolves the matching Kouku, island or Colosseum prompt");
 }
 source->Leave(ownerId,PLAYER_DESPAWN_REASON::LEVEL_CHANGED);sessions[0]->Request_Close();source->Update_Guides(.2f);
 tests.Require(source->m_PersonalGuides.empty()&&source->m_Players.size()==1&&source->m_Players.at(guideId).iNetEntityId==reception.iNetEntityId,"Disconnect while away releases the owner but keeps exactly the same singleton guide");
 for(unsigned i=1;i<4;++i)target->Leave(sessions[i]->Get_SessionId(),PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
 // Exercise the actual ServerApp dispatcher: guidance cannot rely on a hidden party
 // to obtain atomic destination admission and reliable initial-frame preparation.
 {
  auto app = std::make_unique<CServerApp>();
  app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
  app->m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, target);
  const SESSION_ID soloId = 99109u;
  auto session = std::make_shared<CClientSession>(soloId, INVALID_SOCKET,
   CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
  session->m_isSendRunning.store(true);
  sessions.push_back(session);
  source->Handle_Register(session);
  C2S_ENTER_WORLD enter{};
  enter.eWorldId = WORLD_ID::BERN;
  enter.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
  enter.strNickName = "GuideSoloTransaction";
  const bool admitted = source->Join(soloId, enter);
  tests.Require(admitted, "Solo guide transaction starts with a real Bern admission");
  if (!admitted) return 1;
  app->m_Sessions.emplace(soloId, session);
  CServerApp::SESSION_GAMEPLAY_BINDING binding{};
  binding.eWorldId = WORLD_ID::BERN;
  binding.pSimulation = source;
  app->m_GameplayBindingBySessionId.emplace(soloId, binding);
  auto& solo = source->m_Players.at(session->Get_PlayerId());
  const auto& placed = source->m_Players.at(guideId);
  const auto heldPose = std::array{placed.fPositionX, placed.fPositionY, placed.fPositionZ};
  solo.fPositionX = placed.fPositionX + 2.f;
  solo.fPositionY = placed.fPositionY;
  solo.fPositionZ = placed.fPositionZ;
  control(soloId, 1u, GUIDE_CONTROL_ACTION::START);
  auto unrelated = std::make_shared<CClientSession>(soloId, INVALID_SOCKET,
   CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
  tests.Require(source->Has_PersonalGuideOwner(session) && !source->Has_PersonalGuideOwner(unrelated) &&
   source->m_PartyMembersByPartyId.empty(), "Guide ownership matches the live session object without creating a solo party");
  if (!source->Has_PersonalGuideOwner(session)) return 1;
  const auto npc = std::find_if(source->m_WorldEntities.begin(), source->m_WorldEntities.end(),
   [](const auto& entity) { return entity.eKind == WORLD_BOOTSTRAP_KIND::NPC && entity.strPlacementId == "npc.bern.beda.guide"; });
  tests.Require(npc != source->m_WorldEntities.end(), "Solo fixture resolves the real raid entrance NPC");
  if (npc == source->m_WorldEntities.end()) return 1;
  solo.fPositionX = npc->fPositionX;
  solo.fPositionY = npc->fPositionY;
  solo.fPositionZ = npc->fPositionZ;
  C2S_CONFIRM_NPC_ENTRY confirm{};
  confirm.iRequestSequence = 81u;
  confirm.strNpcPlacementId = "npc.bern.beda.guide";
  source->Handle_ConfirmNpcEntry(soloId, confirm);
  SERVER_WORLD_TRANSFER_REQUEST entry{};
  const bool staged = source->Try_DequeueWorldTransfer(entry) && entry.iSessionId == soloId &&
   entry.eTargetWorldId == WORLD_ID::VALTAN_ARENA && entry.PartyBatchSessionIds.empty();
  tests.Require(staged, "Typed solo NPC entry remains one human and carries no fabricated party");
  if (!staged) return 1;
  drain();
  const auto entryPlayer = std::make_unique<SERVER_PLAYER>(solo);
  const auto entryPrompts = source->m_PersonalGuides.at(soloId).PromptQueue;
  const auto hasTransferFailure = [&](WORLD_ID destination, std::uint32_t sequence, PARTY_TRANSFER_RESULT reason)
  {
   for (const auto& frame : session->m_OutboundFrames)
   {
    if (frame.ePacketType != PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT || frame.Bytes.size() < PACKET_HEADER_BYTES) continue;
    CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
    S2C_PARTY_TRANSFER_RESULT decoded{};
    if (Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
     decoded.eTargetWorldId == destination && decoded.iRequestSequence == sequence && decoded.eResult == reason) return true;
   }
   return false;
  };
  const auto guidePreserved = [&](bool waiting)
  {
   const auto& actor = source->m_Players.at(guideId);
   const auto owner = source->m_PersonalGuides.find(soloId);
   return owner != source->m_PersonalGuides.end() && owner->second.WaitingForOwner == waiting &&
    actor.iNetEntityId == reception.iNetEntityId && actor.fPositionX == heldPose[0] &&
    actor.fPositionY == heldPose[1] && actor.fPositionZ == heldPose[2];
  };
  const auto playerPreserved = [&](const auto& room, const auto& destination, const SERVER_PLAYER& before)
  {
   const auto player = room->m_Players.find(before.iPlayerId);
   const auto& actualBinding = app->m_GameplayBindingBySessionId.at(soloId);
   return player != room->m_Players.end() && session->Get_PlayerId() == before.iPlayerId &&
    player->second.fPositionX == before.fPositionX && player->second.fPositionY == before.fPositionY &&
    player->second.fPositionZ == before.fPositionZ && player->second.iCurrentHp == before.iCurrentHp &&
    !session->Is_Closing() && actualBinding.eWorldId == room->Get_WorldId() && actualBinding.pSimulation == room &&
    !destination->m_PlayerIdBySessionId.contains(soloId) && !hasFrame(session, PACKET_TYPE::S2C_ENTER_ACCEPTED) &&
    room->m_PartyMembersByPartyId.empty() && destination->m_PartyMembersByPartyId.empty();
  };
  // Both failures pass through Handle_WorldTransfers, including its rejection policy.
  const auto targetNextEntity = target->m_iNextNetEntityId;
  target->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
  source->m_PendingWorldTransfers.push_back(entry);
  app->Handle_WorldTransfers(source);
  target->m_iNextNetEntityId = targetNextEntity;
  tests.Require(playerPreserved(source, target, *entryPlayer) && guidePreserved(false) &&
   source->m_PersonalGuides.at(soloId).PromptQueue == entryPrompts &&
   hasTransferFailure(WORLD_ID::VALTAN_ARENA, 81u, PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED),
   "Solo destination admission failure keeps Bern player, guide and connection and reports typed rejection");
  drain();
  session->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
  source->m_PendingWorldTransfers.push_back(entry);
  app->Handle_WorldTransfers(source);
  tests.Require(playerPreserved(source, target, *entryPlayer) && guidePreserved(false) &&
   source->m_PersonalGuides.at(soloId).PromptQueue == entryPrompts && source->m_PendingPartyTransferResults.contains(soloId),
   "Solo entry FIFO failure preserves source and guide and defers its rejection without disconnecting");
  drain();
  source->Flush_PartyTransferResults();
  tests.Require(hasTransferFailure(WORLD_ID::VALTAN_ARENA, 81u, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY), "Deferred solo entry rejection decodes with the actual destination and sequence after FIFO drains");
  drain();
  CServerApp::SESSION_WORLD_TRANSFER_FAILURE transferFailure{};
  const bool entered = app->Transfer_SessionWorld(source, entry, transferFailure);
  tests.Require(entered && source->Count_HumanPlayers() == 0 && target->Count_HumanPlayers() == 1 &&
   guidePreserved(true) && source->Has_PersonalGuideOwner(session) &&
   source->m_PartyMembersByPartyId.empty() && target->m_PartyMembersByPartyId.empty() &&
   std::none_of(target->m_Players.begin(), target->m_Players.end(), [](const auto& value) { return value.second.Is_Guide(); }) &&
   app->m_GameplayBindingBySessionId.at(soloId).pSimulation == target,
   "Successful atomic solo entry leaves the same guide in Bern and creates no raid guide or hidden party");
  if (!entered) { std::cout << "Solo guide entry detail: " << transferFailure.strContext << '\n'; return 1; }
  drain();
  target->m_bValtanRaidCleared = true;
  C2S_RETURN_TO_BERN returnRequest{};
  returnRequest.iRequestSequence = 82u;
  target->Handle_ReturnToBern(soloId, returnRequest);
  SERVER_WORLD_TRANSFER_REQUEST returning{};
  const bool returnStaged = target->Try_DequeueWorldTransfer(returning) && returning.iSessionId == soloId &&
   returning.eTargetWorldId == WORLD_ID::BERN && returning.PartyBatchSessionIds.empty();
  tests.Require(returnStaged, "The actual solo raid-clear return retains its ordinary typed one-human request");
  if (!returnStaged) return 1;
  const auto raidPlayer = std::make_unique<SERVER_PLAYER>(target->m_Players.at(session->Get_PlayerId()));
  const auto bernNextEntity = source->m_iNextNetEntityId;
  source->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
  target->m_PendingWorldTransfers.push_back(returning);
  app->Handle_WorldTransfers(target);
  source->m_iNextNetEntityId = bernNextEntity;
  const bool returnAdmissionPlayer = playerPreserved(target, source, *raidPlayer);
  const bool returnAdmissionGuide = guidePreserved(true) && source->m_PersonalGuides.at(soloId).PromptQueue.empty();
  const bool returnAdmissionNotice = hasTransferFailure(WORLD_ID::BERN, 82u, PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED);
  if (!returnAdmissionPlayer || !returnAdmissionGuide || !returnAdmissionNotice)
   std::cout << "Solo return admission detail: player=" << returnAdmissionPlayer << " guide=" << returnAdmissionGuide
    << " notice=" << returnAdmissionNotice << " status=" << target->m_strStatus << '\n';
  tests.Require(returnAdmissionPlayer && returnAdmissionGuide && returnAdmissionNotice,
   "Solo Bern admission failure retains the raid player and waiting guide and decodes the actual Bern rejection");
  drain();
  session->m_OutboundFrames.resize(CClientSession::MAX_OUTBOUND_FRAME_COUNT);
  target->m_PendingWorldTransfers.push_back(returning);
  app->Handle_WorldTransfers(target);
  const bool returnFifoPlayer = playerPreserved(target, source, *raidPlayer);
  const bool returnFifoGuide = guidePreserved(true) && source->m_PersonalGuides.at(soloId).PromptQueue.empty();
  const bool returnFifoNotice = target->m_PendingPartyTransferResults.contains(soloId);
  if (!returnFifoPlayer || !returnFifoGuide || !returnFifoNotice)
   std::cout << "Solo return FIFO detail: player=" << returnFifoPlayer << " guide=" << returnFifoGuide
    << " pendingNotice=" << returnFifoNotice << " status=" << target->m_strStatus << '\n';
  tests.Require(returnFifoPlayer && returnFifoGuide && returnFifoNotice,
   "Solo return FIFO failure preserves the raid binding and waiting guide without closing the owner");
  drain();
  target->Flush_PartyTransferResults();
  tests.Require(hasTransferFailure(WORLD_ID::BERN, 82u, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY), "Deferred solo return rejection decodes Bern and the return sequence after FIFO drains");
  drain();
  const bool returnedSolo = app->Transfer_SessionWorld(target, returning, transferFailure);
  tests.Require(returnedSolo && source->Count_HumanPlayers() == 1 && target->Count_HumanPlayers() == 0 &&
   guidePreserved(false) && source->m_PersonalGuides.at(soloId).AnchorId == session->Get_PlayerId() &&
   !source->m_PersonalGuides.at(soloId).PromptQueue.empty() &&
   source->m_PartyMembersByPartyId.empty() && target->m_PartyMembersByPartyId.empty() &&
   app->m_GameplayBindingBySessionId.at(soloId).pSimulation == source,
   "Committed solo return rebinds the original guide and queues its return speech without making a party");
  if (!returnedSolo) { std::cout << "Solo guide return detail: " << transferFailure.strContext << '\n'; return 1; }
  source->Leave(soloId, PLAYER_DESPAWN_REASON::DISCONNECTED);
 }

 // Use one real Kouku room for the human-only Mario admission boundaries.
 {
  auto mario=std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
  tests.Require(mario->Is_Ready(),"Guide Mario policy fixture loads the real product room");
  if(mario->Is_Ready())
  {
   auto& audition=mario->m_KoukuSaydonPatternAudition;
   audition.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
   audition.pProductGeneration=mario->m_GameplayCatalog.Get_ActiveGeneration();
   audition.PinnedGameplayRevision=mario->m_GameplayCatalog.Get_ActiveRevision();
   audition.iPinnedSourceRevision=CKoukuSaydonBrain::Resolve_ProductSourceRevision(*audition.pProductGeneration);
   std::string detail;
   const auto* phase=CKoukuSaydonBrain::Find_AnimationOnlyPattern(*audition.pProductGeneration,"KAKULSAYDON_G1_PATTERN_33",detail);
   const BOSS_PATTERN_MECHANIC_TRIGGER* formation=nullptr;
   if(phase)for(const auto& trigger:phase->MechanicTriggers)if(trigger.eKind==BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS)formation=&trigger;
   tests.Require(formation!=nullptr,"Guide Mario fixture resolves the published phase-2 formation trigger");
   if(formation)
   {
    SERVER_WORLD_ENTITY boss;boss.iNetEntityId=900001;boss.iPatternSequence=7;boss.iPatternStartTick=100;boss.strPatternId=phase->strPatternId;boss.iCurrentHp=10000;
    const auto makePlayer=[](PLAYER_ID id){SERVER_PLAYER p;p.iPlayerId=id;p.iNetEntityId=100+id;p.iSessionId=99000+id;p.iCurrentHp=p.iMaximumHp=100;p.isCombatReady=true;p.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;p.fPositionX=20.f+id;p.fPositionY=1.32f;p.fPositionZ=950.f;return p;};
    for(unsigned humans:{2u,4u})
    {
     mario->m_Players.clear();mario->m_PersonalGuides.clear();mario->m_MarioLayoutRandom.seed(3);
     auto companion=makePlayer(1);companion.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;companion.iSessionId=INVALID_SESSION_ID;mario->m_Players.emplace(1,companion);
     for(unsigned id=2;id<humans+2;++id){auto human=makePlayer(id);if(id==humans+1)human.iMarioStage=1;mario->m_Players.emplace(id,human);}
     const bool committed=mario->Commit_KoukuMarioPhasePlayers(boss,*formation,130);
     tests.Require(committed,"Mario phase-2 formation admits human participants beside a separate guide");
     const auto& after=mario->m_Players.at(1);unsigned humanBound=0;std::vector<float> humanSlots;
     for(const auto& [id,player]:mario->m_Players)if(player.Is_Human()){humanBound+=player.bPatternBound?1u:0u;if(!player.iMarioStage)humanSlots.push_back(player.fPositionX);}
     std::sort(humanSlots.begin(),humanSlots.end());bool exactSlots=humanSlots.size()==humans-1;
     for(std::size_t slot=0;slot<humanSlots.size();++slot)exactSlots=exactSlots&&std::abs(humanSlots[slot]-(formation->fTeleportX+float(slot)*1.25f))<.01f;
     tests.Require(exactSlots&&!after.bPatternBound&&!after.MarioReturnPosition&&humanBound==(humans>=3?1u:0u),"Guide consumes no Mario formation slot, captive target or living-participant threshold");
     if(humans==2)tests.Require(after.fPositionX==companion.fPositionX&&after.fPositionZ==companion.fPositionZ,"Two humans plus guide remain a duo without a prisoner or guide formation teleport");
     else tests.Require(after.fPositionX==companion.fPositionX&&after.fPositionZ==companion.fPositionZ,"Synthetic nonhuman control kind remains outside raid formation without guide runtime");
    }
    mario->m_Players.clear();mario->m_PersonalGuides.clear();
    auto companion=makePlayer(1);companion.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;companion.iSessionId=INVALID_SESSION_ID;mario->m_Players.emplace(1,companion);
    auto entrant=makePlayer(2);entrant.eMadnessForm=PLAYER_MADNESS_FORM::CLOWN;mario->m_Players.emplace(2,entrant);
    CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER member;member.strMemberId="guide.solo-mario";member.iBossEntityId=boss.iNetEntityId;member.iPatternSequence=boss.iPatternSequence;member.PatternIds.push_back(phase->strPatternId);member.MarioEntryAnchor=boss;member.iMarioEntryStartTick=100;member.iMarioEntryStage=1;member.strCompletionChainSuccessPatternId=phase->strPatternId;member.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
    audition.Members.clear();audition.Members.push_back(std::move(member));mario->m_PendingKoukuMarioEntries.push_back({"guide.solo-mario",2,100,1});
    mario->Commit_KoukuMarioEntries();
    const bool entered=!audition.Members.empty()&&audition.Members.front().bMarioEntryConsumed&&mario->m_Players.at(2).iMarioStage==1;
    tests.Require(entered&&audition.Members.front().bMarioSoloReturnRequired,"One human plus guide still requires the real Mario entrant to complete the solo return");
    tests.Require(mario->m_Players.at(1).iMarioStage==0&&!mario->m_Players.at(1).MarioReturnPosition,"Committing the human Mario entry leaves the guide outside the minigame");
    if(!entered)std::cout<<"Guide Mario admission detail: "<<mario->m_strStatus<<'\n';
   }
  }
 }
 std::cout<<"guide AI failures: "<<tests.failures<<'\n';return tests.failures?1:0;
}
