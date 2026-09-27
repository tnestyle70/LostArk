#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include <algorithm>
#include <memory>
#include <cmath>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_GuideAI()
{
 TESTS tests;
 auto source=std::make_unique<CGameRoom>(WORLD_ID::BERN);
 auto target=std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
 tests.Require(source->Is_Ready()&&target->Is_Ready()&&source->m_GuideCatalog.Loaded&&target->m_GuideCatalog.Loaded,"Guide published catalogs load in Bern and Valtan");
 if(!source->Is_Ready()||!target->Is_Ready()||!source->m_GuideCatalog.Loaded||!source->m_iGuideReceptionId)return 1;
 const auto reception=source->m_Players.at(source->m_iGuideReceptionId);
 tests.Require(reception.Is_Guide()&&!reception.isCombatReady&&reception.iSessionId==INVALID_SESSION_ID&&source->Count_HumanPlayers()==0,"Reception guide has no fake session or human slot");
 std::vector<std::shared_ptr<CClientSession>> sessions;
 auto drain=[&](){for(auto& session:sessions){session->m_OutboundFrames.clear();session->m_iQueuedOutboundBytes=0;session->m_OutboundMetrics.iCurrentQueuedByteCount=0;session->m_OutboundMetrics.iCurrentQueuedFrameCount=0;}};
 bool joined=true;
 for(unsigned i=0;i<4;++i){auto session=std::make_shared<CClientSession>(99101u+i,INVALID_SOCKET,CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});session->m_isSendRunning.store(true);source->Handle_Register(session);C2S_ENTER_WORLD enter;enter.eWorldId=WORLD_ID::BERN;enter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;enter.strNickName="GuideContract"+std::to_string(i);joined=source->Join(session->Get_SessionId(),enter)&&joined;sessions.push_back(session);drain();}
 tests.Require(joined&&source->Count_HumanPlayers()==4,"Four human admissions remain available beside the reception guide");
 if(!joined)return 1;
 auto& leader=source->m_Players.at(sessions[0]->Get_PlayerId());
 for(unsigned i=1;i<4;++i){C2S_PARTY_INVITE invite;invite.iTargetNetEntityId=source->m_Players.at(sessions[i]->Get_PlayerId()).iNetEntityId;source->Handle_PartyInvite(leader.iSessionId,invite);C2S_PARTY_INVITE_RESPOND answer;answer.iFromNetEntityId=leader.iNetEntityId;answer.bAccepted=true;source->Handle_PartyInviteRespond(sessions[i]->Get_SessionId(),answer);drain();}
 C2S_PARTY_INVITE invitation;invitation.iTargetNetEntityId=reception.iNetEntityId;source->Handle_PartyInvite(leader.iSessionId,invitation);
 tests.Require(source->m_Guides.size()==1&&source->m_PartyMembersByPartyId.begin()->second.size()==4&&source->Count_HumanPlayers()==4,"Guide invitation auto accepts without consuming any of four human party slots");
 if(source->m_Guides.empty())return 1;
 const auto partyId=source->m_Guides.begin()->first,guideId=source->m_Guides.begin()->second.PlayerId;
 const auto actorCount=source->m_Players.size();source->Handle_PartyInvite(leader.iSessionId,invitation);
 tests.Require(source->m_Players.size()==actorCount&&source->m_Guides.at(partyId).PromptQueue.size()==1,"Duplicate invite preserves one companion and one greeting");
 source->Update_Guides(.2f);
 tests.Require(std::any_of(sessions[0]->m_OutboundFrames.begin(),sessions[0]->m_OutboundFrames.end(),[](const auto& frame){return frame.ePacketType==PACKET_TYPE::S2C_GUIDE_PROMPT;}),"Accepted invitation emits the reliable prepared greeting");
 drain();
 auto& state=source->m_Guides.at(partyId);
 const auto command=std::find_if(source->m_GuideCatalog.Commands.begin(),source->m_GuideCatalog.Commands.end(),[](const auto& c){return c.Enabled&&!c.Stop;});
 tests.Require(command!=source->m_GuideCatalog.Commands.end(),"Published help command is present");
 if(command!=source->m_GuideCatalog.Commands.end())
 {
  source->Guide_ChatCommand(leader,"prefix "+command->Aliases.front());tests.Require(state.ComboId.empty(),"A substring does not activate a help command");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboId==command->ComboId&&state.ComboStep==0,"Exact party help command stages its published combo");
  source->Guide_ChatCommand(leader,command->Aliases.front());tests.Require(state.ComboStep==0&&state.PendingComboId.empty(),"Duplicate active combo does not restart or queue");
 }
 // Drive the actual decision loop against a world enemy after the exact help command.
 auto& guide=source->m_Players.at(guideId);const auto* combo=source->m_GuideCatalog.Find_Combo(state.ComboId);
 if(combo&&!combo->Skills.empty())
 {
  SERVER_WORLD_ENTITY enemy;enemy.iNetEntityId=900001;enemy.eKind=WORLD_BOOTSTRAP_KIND::MONSTER;enemy.iCurrentHp=enemy.iMaximumHp=1000;enemy.fPositionX=guide.fPositionX;enemy.fPositionY=guide.fPositionY;enemy.fPositionZ=guide.fPositionZ+1;enemy.eAction=SERVER_ENTITY_ACTION::PATTERN_WINDUP;
  source->m_WorldEntities.push_back(enemy);source->Update_Guides(.2f);
  tests.Require(state.Action==3&&state.ComboStep==1&&guide.iCurrentSkillId==combo->Skills.front()&&guide.eAction==PLAYER_ACTION_STATE::SKILL,"Exact help command executes the next real skill through the automatic decision loop");
  source->m_WorldEntities.pop_back();
 }
 const auto rescue=std::find_if(source->m_GuideCatalog.Commands.begin(),source->m_GuideCatalog.Commands.end(),[&](const auto& c){return c.Enabled&&!c.Stop&&c.ComboId!=state.ComboId;});
 if(rescue!=source->m_GuideCatalog.Commands.end()){source->Guide_ChatCommand(leader,rescue->Aliases.front());tests.Require(state.PendingComboId==rescue->ComboId,"A different rescue command is admitted immediately despite the previous command cooldown");const auto pending=state.PendingComboId;source->Guide_ChatCommand(leader,rescue->Aliases.front());tests.Require(state.PendingComboId==pending,"Repeating the same rescue command is idempotent during its own cooldown");}
 source->m_iServerTick+=200;auto stop=std::find_if(source->m_GuideCatalog.Commands.begin(),source->m_GuideCatalog.Commands.end(),[](const auto& c){return c.Enabled&&c.Stop;});if(stop!=source->m_GuideCatalog.Commands.end()){source->Guide_ChatCommand(leader,stop->Aliases.front());tests.Require(state.ComboId.empty()&&state.PendingComboId.empty(),"Stop command cancels future combo steps");}
 guide.iCurrentHp=0;guide.eAction=PLAYER_ACTION_STATE::DEAD;source->Update_Guides(.2f);tests.Require(guide.iCurrentHp==guide.iMaximumHp&&guide.Is_Guide(),"A dead guide revives through a validated landing while humans live");drain();
 // Human-only minigames never pull the companion into the participant field.
 const float guideX=guide.fPositionX,guideZ=guide.fPositionZ;
 leader.iMarioStage=1;source->Guide_AnchorArrived(leader);
 tests.Require(guide.fPositionX==guideX&&guide.fPositionZ==guideZ,"A Mario participant arrival cannot teleport the guide into the minigame");
 for(const auto& session:sessions)source->m_Players.at(session->Get_PlayerId()).iMarioStage=1;
 source->Update_Guides(.2f);tests.Require(state.Action==8&&!guide.hasMoveGoal&&state.ComboId.empty(),"The guide waits outside when every human is in a minigame");
 for(const auto& session:sessions)source->m_Players.at(session->Get_PlayerId()).iMarioStage=0;
 // Riding uses the same permission path, and three-axis follow uses the common move executor.
 C2S_SET_VEHICLE_RIDING riding;riding.eWorldId=WORLD_ID::BERN;riding.iRequestSequence=100;riding.iVehicleId=ANCIENT_SEA_VEHICLE_ID;
 (void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);
 tests.Require(leader.iVehicleId==ANCIENT_SEA_VEHICLE_ID&&guide.iVehicleId==leader.iVehicleId,"Guide mounts the same admitted dragon as the anchor");
 leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::FLYING;const float anchorY=leader.fPositionY;leader.fPositionY=guide.fPositionY+2;
 source->Update_Guides(.2f);tests.Require(state.Action==6&&guide.fVehicleFlightInputY>0&&guide.iLastMoveSequence>0,"Airborne guide emits admitted three-axis flight follow input");
 leader.fPositionY=anchorY;leader.eVehicleFlightPhase=guide.eVehicleFlightPhase=VEHICLE_FLIGHT_PHASE::GROUNDED;riding.iRequestSequence=200;riding.iVehicleId=INVALID_VEHICLE_ID;(void)source->Apply_SetVehicleRiding(leader,riding);source->Update_Guides(.2f);drain();
 std::vector<SESSION_ID> batch;for(const auto& session:sessions)batch.push_back(session->Get_SessionId());
 PARTY_TRANSFER_RESULT result;std::string status;const auto revision=target->m_GuideCatalog.Revision;target->m_GuideCatalog.Revision^=1;
 tests.Require(!source->Transfer_PartyTo(*target,batch,result,status)&&source->m_Guides.size()==1&&source->Count_HumanPlayers()==4&&target->Count_HumanPlayers()==0,"Guide generation failure keeps the whole source party and guide unchanged");target->m_GuideCatalog.Revision=revision;
 const bool moved=source->Transfer_PartyTo(*target,batch,result,status);
 tests.Require(moved&&source->Count_HumanPlayers()==0&&source->m_Guides.empty()&&target->Count_HumanPlayers()==4&&target->m_Guides.size()==1,"Four humans and one guide transfer in the same transaction");
 if(!moved)std::cout<<"Guide transfer detail: "<<status<<'\n';
 if(moved){const auto& companion=target->m_Guides.begin()->second;tests.Require(companion.ComboId.empty()&&companion.PromptQueue.empty()&&target->m_Players.at(companion.PlayerId).iSessionId==INVALID_SESSION_ID,"Transfer clears old combo and dialogue occurrences without a guide session");drain();for(const auto& session:sessions)target->Leave(session->Get_SessionId(),PLAYER_DESPAWN_REASON::LEVEL_CHANGED);tests.Require(target->m_Guides.empty()&&target->Count_HumanPlayers()==0,"Last human departure despawns the companion");}
 // Direct NPC admission must preserve a singleton party's separate companion.
 auto solo=sessions.front();source->Handle_Register(solo);C2S_ENTER_WORLD reenter;reenter.eWorldId=WORLD_ID::BERN;reenter.eCharacterClass=CHARACTER_CLASS_ID::DIMENSIONMASTER;reenter.strNickName="GuideSolo";
 const bool rejoined=source->Join(solo->Get_SessionId(),reenter);
 if(rejoined){auto& owner=source->m_Players.at(solo->Get_PlayerId());source->Handle_PartyInvite(owner.iSessionId,invitation);const auto* npc=source->Find_Placement("npc.bern.beda.guide");if(npc){owner.fPositionX=npc->fPositionX;owner.fPositionY=npc->fPositionY;owner.fPositionZ=npc->fPositionZ;}C2S_CONFIRM_NPC_ENTRY entry;entry.iRequestSequence=1;entry.strNpcPlacementId="npc.bern.beda.guide";source->Handle_ConfirmNpcEntry(owner.iSessionId,entry);SERVER_WORLD_TRANSFER_REQUEST request;const bool staged=source->Try_DequeueWorldTransfer(request)&&request.PartyBatchSessionIds.size()==1;
  tests.Require(staged,"Direct NPC entry batches one human with its separate companion");drain();const bool soloMoved=staged&&source->Transfer_PartyTo(*target,request.PartyBatchSessionIds,result,status,request.strRaidReturnNpcPlacementId,request.strSpawnPlacementOverrideId);tests.Require(soloMoved&&target->m_Guides.size()==1&&target->Count_HumanPlayers()==1,"Singleton NPC transfer commits the human and guide together");
  if(soloMoved){target->m_bValtanRaidCleared=true;C2S_RETURN_TO_BERN back;back.iRequestSequence=2;target->Handle_ReturnToBern(solo->Get_SessionId(),back);SERVER_WORLD_TRANSFER_REQUEST returning;const bool backStaged=target->Try_DequeueWorldTransfer(returning)&&returning.PartyBatchSessionIds.size()==1;drain();const bool returned=backStaged&&target->Transfer_PartyTo(*source,returning.PartyBatchSessionIds,result,status,returning.strRaidReturnNpcPlacementId,returning.strSpawnPlacementOverrideId);tests.Require(returned&&source->m_Guides.size()==1&&source->Count_HumanPlayers()==1&&target->m_Guides.empty(),"Cleared raid returns the singleton companion transactionally to Bern");if(!returned)std::cout<<"Guide return detail: "<<status<<'\n';}
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
     mario->m_Players.clear();mario->m_Guides.clear();mario->m_MarioLayoutRandom.seed(3);
     auto companion=makePlayer(1);companion.eControlKind=PLAYER_CONTROL_KIND::GUIDE_AI;companion.iSessionId=INVALID_SESSION_ID;mario->m_Players.emplace(1,companion);
     for(unsigned id=2;id<humans+2;++id){auto human=makePlayer(id);if(id==humans+1)human.iMarioStage=1;mario->m_Players.emplace(id,human);}
     if(humans==4){CGameRoom::GUIDE_RUNTIME runtime;runtime.PlayerId=1;runtime.AnchorId=2;mario->m_Guides.emplace(91,std::move(runtime));}
     const bool committed=mario->Commit_KoukuMarioPhasePlayers(boss,*formation,130);
     tests.Require(committed,"Mario phase-2 formation admits human participants beside a separate guide");
     const auto& after=mario->m_Players.at(1);unsigned humanBound=0;std::vector<float> humanSlots;
     for(const auto& [id,player]:mario->m_Players)if(player.Is_Human()){humanBound+=player.bPatternBound?1u:0u;if(!player.iMarioStage)humanSlots.push_back(player.fPositionX);}
     std::sort(humanSlots.begin(),humanSlots.end());bool exactSlots=humanSlots.size()==humans-1;
     for(std::size_t slot=0;slot<humanSlots.size();++slot)exactSlots=exactSlots&&std::abs(humanSlots[slot]-(formation->fTeleportX+float(slot)*1.25f))<.01f;
     tests.Require(exactSlots&&!after.bPatternBound&&!after.MarioReturnPosition&&humanBound==(humans>=3?1u:0u),"Guide consumes no Mario formation slot, captive target or living-participant threshold");
     if(humans==2)tests.Require(after.fPositionX==companion.fPositionX&&after.fPositionZ==companion.fPositionZ,"Two humans plus guide remain a duo without a prisoner or guide formation teleport");
     else {const auto& anchor=mario->m_Players.at(2);tests.Require(mario->m_Guides.at(91).Reason=="Following committed anchor arrival"&&std::hypot(after.fPositionX-anchor.fPositionX,after.fPositionZ-anchor.fPositionZ)<7.f,"A real companion follows only the committed human anchor through separate safe landing");}
    }
    mario->m_Players.clear();mario->m_Guides.clear();
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
