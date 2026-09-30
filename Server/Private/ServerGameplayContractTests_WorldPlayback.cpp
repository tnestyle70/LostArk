#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerTriggerSystem.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

int LostArk::Server::CServerGameplayContractRunner::Run_WorldPlayback(TESTS& tests)
{
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
        CCombatObjectRuntime runtime;
        SERVER_PLAYER owner; owner.iPlayerId=1001u; owner.iNetEntityId=1002u;
        owner.fPositionX=10.f; owner.fPositionY=20.f; owner.fPositionZ=30.f; owner.fYawDegrees=90.f;
        std::string status;
        for (const auto skill : {56900u, 56910u, 56930u})
        {
            auto staged=runtime.Begin_Transaction();
            tests.Require(runtime.Stage_WaterGunPresentation(staged,owner,skill,room->m_GameplayCatalog,500u,status),
                "Watergun stages a source-owned projectile");
            if (staged.Objects.empty()) continue;
            const auto id=staged.Objects.front().iCombatObjectId;
            tests.Require(runtime.Get_LiveObjects().empty() && runtime.Commit(std::move(staged)),
                "Staging never exposes a partial projectile");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
            std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
            std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
            runtime.Drain_Lifecycle(spawned,events,despawned);
            const float expectedX=skill==56910u?10.f:10.7f;
            const float expectedY=skill==56910u?20.f:20.75f;
            const float expectedZ=skill==56910u?30.f:skill==56900u?29.8f:29.89f;
            tests.Require(spawned.size()==1u && std::abs(spawned[0].fPositionX-expectedX)<.001f &&
                std::abs(spawned[0].fPositionY-expectedY)<.001f && std::abs(spawned[0].fPositionZ-expectedZ)<.001f,
                "Source forward/right/up launch offset is converted once at yaw90");
            tests.Require(!runtime.Finish_WaterGunPresentation(id,999u,500u,501u,true) &&
                !runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,499u,501u,true) &&
                runtime.Get_LiveObjects().size()==1u,"Wrong owner or cast cannot finish a live watergun object");
            tests.Require(runtime.Set_OwnedVisualPosition(id,owner.iNetEntityId,500u,15.f,20.f,34.f),
                "Watergun flight consumes the authoritative room pose");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> late;
            runtime.Build_LiveSpawnMessages(510u,late);
            tests.Require(late.size()==1u && late[0].iSpawnTick==500u && late[0].iServerTick==510u &&
                late[0].fPositionX==15.f && late[0].fPositionZ==34.f,
                "Late join keeps cast age and current projectile position");
            tests.Require(runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,500u,511u,true) &&
                !runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,500u,512u,true),
                "A projectile impact finishes exactly once");
            runtime.Drain_Lifecycle(spawned,events,despawned);
            tests.Require(events.size()==1u && despawned.size()==1u && runtime.Get_LiveObjects().empty() &&
                events[0].strHitId=="maharaka.watergun.impact" && events[0].fPositionX==15.f &&
                events[0].fPositionZ==34.f && events[0].PinnedDefinitionRevision==room->m_GameplayCatalog.Get_ActiveRevision(),
                "Impact and despawn carry the last authoritative pose and pinned revision");
        }
        auto invalid=runtime.Begin_Transaction();
        tests.Require(!runtime.Stage_WaterGunPresentation(invalid,owner,56920u,room->m_GameplayCatalog,520u,status) &&
            invalid.Objects.empty() && invalid.Spawned.empty(),"Speed buff cannot create a water projectile");
        auto expired=runtime.Begin_Transaction();
        const bool staged=runtime.Stage_WaterGunPresentation(expired,owner,56900u,room->m_GameplayCatalog,530u,status);
        const auto id=staged?expired.Objects.front().iCombatObjectId:0u;
        tests.Require(staged && runtime.Commit(std::move(expired)) &&
            runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,530u,560u,false),
            "A missed watergun projectile expires without impact");
        std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
        std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
        std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
        runtime.Drain_Lifecycle(spawned,events,despawned);
        tests.Require(events.empty() && despawned.size()==1u,"Miss expiry emits no fabricated hit");
        // Independent source fields, rather than the old misread MaxDistance-as-Speed:
        // EFSequenceSummonsProjectile Speed=1000cm/s; Q570020/R569320 distance=330/300cm.
        const auto* fan = Find_MaharakaWaterGunSkillBySlot('Q');
        const auto* bomb = Find_MaharakaWaterGunSkillBySlot('W');
        const auto* single = Find_MaharakaWaterGunSkillBySlot('R');
        tests.Require(fan && bomb && single && fan->iSkillId == 56900u &&
            fan->iAttackClip == 4u && fan->iSpawnMs == 704u && fan->iCooldownMs == 3000u &&
            fan->iProjectileCount == 3u && fan->fSpreadDegrees == 30.f &&
            fan->fSpeedMps == 10.f && fan->fProjectileMaxDistanceM == 3.3f &&
            bomb->fSpeedMps == 10.f && bomb->fProjectileMaxDistanceM == 8.f && bomb->fMaxRangeM == 7.f &&
            single->fSpeedMps == 10.f && single->fProjectileMaxDistanceM == 3.f,
            "Watergun keeps stable input IDs and project cooldown while restoring source fan and projectile units");
        auto invalidRay = runtime.Begin_Transaction();
        tests.Require(!runtime.Stage_WaterGunPresentation(invalidRay,owner,56900u,
            room->m_GameplayCatalog,600u,status,3u) && invalidRay.Objects.empty() && invalidRay.Spawned.empty(),
            "An invalid fan ray is rejected before staging any replicated object");

        // Exercise the actual room authority, not a duplicate trajectory simulation.
        tests.Require(room->Is_Ready(), "Watergun trajectory fixture loads the published Maharaka room");
        if (room->Is_Ready())
        {
            S2C_WORLD_SEQUENCE_PLAY intro;
            intro.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
            intro.strSequenceInstanceId = MAHARAKA_WATERPANG_INTRO_INSTANCE;
            intro.iStartTick = 1000u;
            room->m_MaharakaWaterpangIntro = intro;
            room->m_iServerTick = 1000u;
            owner.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 4.f;
            owner.fPositionY = 22.4f;
            owner.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
            room->m_Players.emplace(owner.iPlayerId, owner);
            auto& shooter = room->m_Players.at(owner.iPlayerId);
            C2S_USE_SKILL command;
            command.iClientSequence = 1u; command.iSkillId = 56900u;
            command.fAimX = shooter.fPositionX - 7.f; command.fAimZ = shooter.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter, command) &&
                room->m_MaharakaWaterGunShots.size() == 3u,
                "One accepted Q queues three independent shots under one cast and cooldown");
            if (room->m_MaharakaWaterGunShots.size() == 3u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                room->Update_MaharakaWaterGunShots(launchTick - 1u);
                tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                    "Q does not emit before its source action notify");
                room->Update_MaharakaWaterGunShots(launchTick);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                bool exactFan = spawned.size() == 3u && room->m_MaharakaWaterGunShots.size() == 3u;
                const std::array<float,3u> yawOffset{0.f,30.f,-30.f};
                std::set<COMBAT_OBJECT_ID> uniqueIds;
                for (std::size_t ray = 0u; exactFan && ray < spawned.size(); ++ray)
                {
                    const auto& message = spawned[ray];
                    const auto& shot = room->m_MaharakaWaterGunShots[ray];
                    uniqueIds.insert(message.iCombatObjectId);
                    exactFan = message.iSpawnTick == launchTick &&
                        std::abs(message.fYawDegrees - shooter.fYawDegrees - yawOffset[ray]) < .001f &&
                        std::abs(message.fPositionX - (shooter.fPositionX - .70f)) < .001f &&
                        std::abs(message.fPositionY - (shooter.fPositionY + .75f)) < .001f &&
                        std::abs(message.fPositionZ - (shooter.fPositionZ + .20f)) < .001f &&
                        std::abs(shot.fTravelM - (10.f / 30.f)) < .001f &&
                        std::abs(shot.fX - message.fPositionX - shot.fDirX * shot.fTravelM) < .001f &&
                        std::abs(shot.fZ - message.fPositionZ - shot.fDirZ * shot.fTravelM) < .001f;
                }
                tests.Require(exactFan && uniqueIds.size() == 3u,
                    "Q fan shares one muzzle but replicates three source yaws and independent authoritative rays");
                for (unsigned step = 1u; step < 10u; ++step)
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.empty() && despawned.size() == 3u,
                    "Q rays expire independently at 3.3m without invented impacts");
            }

            room->m_iServerTick = 2000u;
            command.iClientSequence = 2u; command.iSkillId = 56930u;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter, command) &&
                room->m_MaharakaWaterGunShots.size() == 1u,
                "R retains one basic projectile");
            if (room->m_MaharakaWaterGunShots.size() == 1u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                room->Update_MaharakaWaterGunShots(launchTick);
                tests.Require(std::abs(room->m_MaharakaWaterGunShots.front().fTravelM - 10.f/30.f) < .001f,
                    "R advances at the reflected 10m/s speed");
                for (unsigned step = 1u; step < 9u; ++step)
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && spawned.size() == 1u &&
                    events.empty() && despawned.size() == 1u,
                    "R stops at its independent 3m MaxDistance");
            }

            SERVER_PLAYER target;
            target.iPlayerId = 1003u; target.iNetEntityId = 1004u;
            target.fPositionX = shooter.fPositionX - 3.f;
            target.fPositionY = shooter.fPositionY; target.fPositionZ = shooter.fPositionZ;
            room->m_Players.emplace(target.iPlayerId,target);
            SERVER_PLAYER outside = target;
            outside.iPlayerId = 1005u; outside.iNetEntityId = 1006u; outside.fPositionZ += 2.f;
            room->m_Players.emplace(outside.iPlayerId,outside);
            room->m_iServerTick = 3000u;
            command.iClientSequence = 3u; command.iSkillId = 56910u;
            command.fAimX = target.fPositionX; command.fAimZ = target.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter,command) &&
                room->m_MaharakaWaterGunShots.size() == 1u,"W queues one aimed grenade");
            if (room->m_MaharakaWaterGunShots.size() == 1u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                float peakY = shooter.fPositionY;
                for (unsigned step = 0u; step < 8u; ++step)
                {
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                    const auto& live = room->m_CombatObjectRuntime.Get_LiveObjects();
                    if (!live.empty()) peakY = (std::max)(peakY,live.front().LiveState.CurrentPose.fPositionY);
                }
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(events.empty() && despawned.empty() &&
                    room->m_Players.at(target.iPlayerId).fKnockbackRemainingSeconds == 0.f &&
                    peakY > shooter.fPositionY + .6f && peakY <= shooter.fPositionY + .751f,
                    "W follows its bounded source-height arc and cannot hit during flight");
                room->Update_MaharakaWaterGunShots(launchTick + 8u);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.size() == 1u &&
                    despawned.size() == 1u && std::abs(events[0].fPositionX - target.fPositionX) < .001f &&
                    std::abs(events[0].fPositionY - target.fPositionY) < .001f &&
                    room->m_Players.at(target.iPlayerId).fKnockbackRemainingSeconds > 0.f &&
                    room->m_Players.at(target.iPlayerId).iCurrentHp == target.iCurrentHp &&
                    room->m_Players.at(outside.iPlayerId).fKnockbackRemainingSeconds == 0.f,
                    "W bursts once at authoritative ground arrival, preserves HP, and respects its hit radius");
            }

            room->m_Players.erase(outside.iPlayerId);
            auto& overlapping = room->m_Players.at(target.iPlayerId);
            overlapping = target;
            overlapping.fPositionX = shooter.fPositionX - 1.f;
            overlapping.fPositionZ = shooter.fPositionZ + .20f;
            room->m_iServerTick = 4000u;
            command.iClientSequence = 4u; command.iSkillId = 56900u;
            command.fAimX = shooter.fPositionX - 7.f; command.fAimZ = shooter.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter,command),
                "Q can be cast again after its existing project cooldown");
            if (!room->m_MaharakaWaterGunShots.empty())
            {
                room->Update_MaharakaWaterGunShots(room->m_MaharakaWaterGunShots.front().iSpawnTick);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.size() == 3u &&
                    despawned.size() == 3u && overlapping.iCurrentHp == target.iCurrentHp &&
                    std::abs(overlapping.fKnockbackSpeed - room->m_MaharakaAITuning.fKnockbackRangeM /
                        (float(room->m_MaharakaAITuning.iKnockbackMs) * .001f)) < .001f,
                    "Each nonpiercing fan ray consumes on its first body; overlapping rays replace rather than triple push velocity");
            }
        }

    }
        {
            auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
            tests.Require(room->Is_Ready(), "Waterpang published room loads");
            room->m_iServerTick=100u;
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                0.f,0.f,0.f,0.f,0u,{}) && !room->m_MaharakaWaterpangIntro,
                "Invalid Waterpang packet cannot consume room reservation");
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro &&
                room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "Waterpang reserves exactly ten seconds on the Server clock");
            room->m_iServerTick=250u;
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "A second arena entry cannot reset countdown");
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{},WORLD_SEQUENCE_OPERATION::REPLAY),
                "Replay cannot restart an active Waterpang reservation");
            auto session=std::make_shared<CClientSession>(99001u,INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});
            C2S_ENTER_WORLD enter{}; enter.iProtocolVersion=NETWORK_PROTOCOL_VERSION;
            enter.eWorldId=WORLD_ID::MAHARAKA; enter.eCharacterClass=CHARACTER_CLASS_ID::ARTIST;
            enter.strNickName="WaterpangLateJoin";
            CGameRoom::STAGED_PLAYER_ENTRY admission{}; SESSION_DIAGNOSTIC_REASON reason{}; std::string admissionStatus;
            const bool admitted=room->Stage_PlayerEntry(session,enter,{},admission,reason,admissionStatus) &&
                room->Build_PlayerEntryFrames(admission,std::span<const CGameRoom::STAGED_PLAYER_ENTRY>{&admission,1u},admissionStatus);
            if (!admitted) std::cout << "Waterpang admission diagnostic: " << admissionStatus << '\n';
            bool sameReservation=false;
            for (const auto& frame:admission.Frames)
                if (frame.ePacketType==PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
                {
                    CPacketReader reader(frame.Payload); S2C_WORLD_SEQUENCE_PLAY play;
                    if (Read_Message(reader,play) && play.strSequenceInstanceId==MAHARAKA_WATERPANG_INTRO_INSTANCE)
                        sameReservation=play.iStartTick==400u && play.iServerTick==250u;
                }
            tests.Require(admitted && sameReservation,"Late Maharaka admission receives original start and current Server tick");
            tests.Require(room->Reset_ReplayableArenaWhenEmpty() && !room->m_MaharakaWaterpangIntro,
                "An empty Maharaka room releases Waterpang reservation");

            CServerNavigation navigation; SERVER_NAV_POINT point;
            tests.Require(navigation.Load("LV_OCN_EVENTIS_MHP") &&
                navigation.Sample_Position(73.041f,-979.223022f,point) && point.y>22.3f && point.y<22.5f &&
                navigation.Resolve_TraversalStep(73.041f,-979.223022f,73.1f,-979.4f,point,23.1289997f) && point.y>22.3f,
                "Waterpang landing and next walking step remain on mesh-baked stage floor");

            CServerTriggerSystem entry; entry.Set_WorldId(WORLD_ID::MAHARAKA);
            WORLD_BOOTSTRAP_PLACEMENT start{}; start.strPlacementId="waterpang.arena.start";
            start.eKind=WORLD_BOOTSTRAP_KIND::TRIGGER_BOX; start.isEnabled=true;
            start.fHalfExtentX=start.fHalfExtentY=start.fHalfExtentZ=1.f;
            WORLD_TRIGGER_ACTION action{}; action.eKind=WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
            action.strTargetId=MAHARAKA_WATERPANG_INTRO_INSTANCE; start.TriggerActions.push_back(action);
            std::string status; tests.Require(entry.Initialize({start},status),"Waterpang landing trigger initializes");
            std::map<PLAYER_ID,SERVER_PLAYER> players;
            auto& player=players[1u]; player.iPlayerId=1u; player.iCurrentHp=player.iMaximumHp=100u;
            player.TriggerMove.isActive=true;
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers; std::vector<SERVER_INTERACT_PROMPT_EDGE> edges;
            int fired=0; const auto activate=[&](WORLD_TRIGGER_ACTION_KIND,const std::string&){++fired;return true;};
            entry.Evaluate_Entries(players,1u,transfers,activate,edges);
            tests.Require(fired==0,"Flying through arena does not start countdown");
            player.TriggerMove.isActive=false;
            entry.Evaluate_Entries(players,2u,transfers,activate,edges);
            tests.Require(fired==1,"Landing inside arena starts countdown without an extra re-entry");
            entry.Evaluate_Entries(players,3u,transfers,activate,edges);
            tests.Require(fired==1,"Standing on arena does not repeatedly start countdown");
            CWorldBootstrap authored;
            tests.Require(authored.Load(WORLD_ID::MAHARAKA),"Published Waterpang G jumps load");
            for (const char* name:{"jump1","jump2","jump3"})
            {
                const auto box=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [&](const auto& row){return row.strPlacementId==name;});
                if (box==authored.Get_Placements().end()) { tests.Require(false,"Waterpang jump source missing"); continue; }
                const auto destination=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [&](const auto& row){return row.strPlacementId==std::string(name)+"_1";});
                const bool hasDestination=destination!=authored.Get_Placements().end() && box->TriggerActions.size()==1u;
                tests.Require(hasDestination,"Every published Waterpang jump has its saved landing marker and one action");
                if (!hasDestination) continue;
                const auto& move=box->TriggerActions.front();
                tests.Require(box->isEnabled && box->requiresInteract && !box->isTriggerOnce &&
                    !destination->isEnabled && destination->TriggerActions.empty() &&
                    move.eKind==WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER &&
                    move.fTargetX==destination->fPositionX && move.fTargetY==destination->fPositionY &&
                    move.fTargetZ==destination->fPositionZ,
                    "G jump consumes the current saved marker, never a stale copied destination");
                bool walkable=navigation.Sample_Position(move.fTargetX,move.fTargetZ,point,move.fTargetY) &&
                    point.y>22.3f && point.y<22.5f;
                for (int dx=-1;dx<=1;++dx) for (int dz=-1;dz<=1;++dz)
                    if (dx || dz) walkable=navigation.Resolve_TraversalStep(move.fTargetX,move.fTargetZ,
                        move.fTargetX+.25f*dx,move.fTargetZ+.25f*dz,point,move.fTargetY) &&
                        point.y>22.3f && point.y<22.5f && walkable;
                tests.Require(walkable,"Each saved landing has deck support and eight valid first walking steps");
                CServerTriggerSystem jump; jump.Set_WorldId(WORLD_ID::MAHARAKA);
                tests.Require(jump.Initialize({*box},status),"Waterpang authored jump initializes");
                player.fPositionX=box->fPositionX; player.fPositionY=box->fPositionY; player.fPositionZ=box->fPositionZ;
                jump.Evaluate_Entries(players,10u,transfers,activate,edges);
                tests.Require(!player.TriggerMove.isActive && !edges.empty(),"Waterpang jump offers G without automatic movement");
                const auto activated=jump.Activate_Here(1u,players,11u,transfers,activate);
                const auto target=player.TriggerMove;
                for (unsigned tick=0;tick<40;++tick) jump.Update_PlayerMotion(player,1.f/30.f);
                tests.Require(activated==1u && !player.TriggerMove.isActive &&
                    std::abs(player.fPositionX-target.fTargetX)<.001f &&
                    std::abs(player.fPositionY-target.fTargetY)<.001f &&
                    std::abs(player.fPositionZ-target.fTargetZ)<.001f,
                    "Waterpang G travels to the exact authored destination");
                const auto arena=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [](const auto& row){return row.strPlacementId=="waterpang.arena.start";});
                tests.Require(arena!=authored.Get_Placements().end(),"Published arena start trigger exists");
                if (arena!=authored.Get_Placements().end())
                {
                    CServerTriggerSystem arrival; arrival.Set_WorldId(WORLD_ID::MAHARAKA);
                    tests.Require(arrival.Initialize({*arena},status),"Published arena start trigger initializes");
                    fired=0;
                    arrival.Evaluate_Entries(players,60u,transfers,activate,edges);
                    arrival.Evaluate_Entries(players,61u,transfers,activate,edges);
                    tests.Require(fired==1,"Every authored jump landing activates the arena countdown exactly once");
                }
            }
        }
		CWorldBootstrap bootstrap;
		tests.Require(bootstrap.Load(WORLD_ID::KAKULSAYDON_ARENA) && !bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer loads published Kouku sequence IDs with the world");
		CWorldBootstrap valtanOnly;
		tests.Require(valtanOnly.Load(WORLD_ID::VALTAN_ARENA) && bootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			bootstrap.Get_SequenceInstanceIds() == valtanOnly.Get_SequenceInstanceIds(),
			"Viewer switching to Valtan replaces previous IDs with its published sequences");
		CServerTriggerSystem triggers;
		triggers.Set_HonourTriggerOnce(true);
		WORLD_BOOTSTRAP_PLACEMENT box{};
		box.strPlacementId = "viewer.trigger"; box.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		box.fHalfExtentX = box.fHalfExtentY = box.fHalfExtentZ = 1.f;
		box.isTriggerOnce = true; box.requiresInteract = true;
		WORLD_TRIGGER_ACTION action{}; action.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
		action.strTargetId = "viewer.sequence"; box.TriggerActions.push_back(action);
		std::string status;
		tests.Require(triggers.Initialize({ box }, status), "Viewer test initializes the real trigger system");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players[1u].iPlayerId = 1u; players[1u].iCurrentHp = players[1u].iMaximumHp = 100u;
		players[1u].fPositionX = 50.f;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		int fired = 0;
		const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{ if (kind != WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE || id != "viewer.sequence") return false; ++fired; return true; };
		using R = DEBUG_WORLD_PLAYBACK_RESULT;
		tests.Require(triggers.Debug_Activate(2u, box.strPlacementId, false, players, 1u, transfers, activate) ==
#ifdef _DEBUG
			R::INVALID_PLAYER,
#else
			R::DISABLED,
#endif
			"Viewer rejects missing player without activating a trigger");
#ifdef _DEBUG
		tests.Require(triggers.Debug_Activate(1u, "missing", false, players, 1u, transfers, activate) == R::INVALID_TARGET && fired == 0,
			"Viewer rejects unknown targets without effects");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 1u, transfers, activate) == R::ACCEPTED && fired == 1,
			"Debug viewer uses the authored action outside the G-key box");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 2u, transfers, activate) == R::ALREADY_USED && fired == 1,
			"Play preserves the one-shot latch");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 3u, transfers, activate) == R::ACCEPTED && fired == 2,
			"Replay reuses the same authored action");
		players[1u].iCurrentHp = 0;
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 4u, transfers, activate) == R::INVALID_PLAYER && fired == 2,
			"Dead viewer cannot activate world actions");
		players[1u].iCurrentHp = 100;
		const auto reject = [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return false; };
		tests.Require(triggers.Initialize({ box }, status) &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 5u, transfers, reject) == R::ACTION_REJECTED &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 6u, transfers, activate) == R::ACCEPTED,
			"Failed action does not consume the one-shot trigger");
#endif
		{
			// Exercise the real broadcast boundary: stale bootstrap rows cannot revive
			// the old actor or move the party before the Client rejects the cue.
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			auto& player = room->m_Players[1u];
			player.iPlayerId = 1u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			bool allPreserve = room->Is_Ready();
			bool admissionMatches = room->Is_Ready();
			const WORLD_SEQUENCE_OPERATION operations[] = { WORLD_SEQUENCE_OPERATION::PLAY,
				WORLD_SEQUENCE_OPERATION::REPLAY, WORLD_SEQUENCE_OPERATION::STOP,
				WORLD_SEQUENCE_OPERATION::PLAY, WORLD_SEQUENCE_OPERATION::PLAY };
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				player.fPositionX = 12.f; player.fPositionY = 34.f; player.fPositionZ = 56.f;
				player.hasMoveGoal = true; player.TriggerMove.isActive = true;
				player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
				player.iMarioStage = 1u; player.ePreMarioForm = PLAYER_MADNESS_FORM::NORMAL;
				player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
				const bool accepted = room->Broadcast_WorldSequencePlay(
					scenario == 4u ? "world.sequence.instance.contract_ordinary" : "world.sequence.instance.original_kouku",
					1.f, 0.f, 0.f, 0.f, 0u, scenario == 3u ? "world.existing.target" : "", operations[scenario]);
				admissionMatches = admissionMatches && accepted == (scenario == 2u || scenario == 4u);
				allPreserve = allPreserve &&
					player.fPositionX == 12.f && player.fPositionY == 34.f && player.fPositionZ == 56.f &&
					player.hasMoveGoal && player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
					player.iMarioStage == 1u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && player.iCurrentHp == 100u;
			}
			tests.Require(admissionMatches, "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted");
			tests.Require(allPreserve, "Legacy rejection and ordinary sequence cues preserve all player movement and form state");
		}
		{
			C2S_DEBUG_WORLD_PLAYBACK request{};
			request.iRequestSequence = 17u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
			request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
			request.strOccurrenceId = request.strTargetId + ".logic.51";
			request.iRunEpoch = 9u; request.iRoomPlayerSlot = 3u;
			request.fPositionX = -1.156042f; request.fPositionY = 1.3176255f; request.fPositionZ = 742.512031f;
			CPacketWriter writer;
			tests.Require(Write_Message(writer, request), "Arrival command encodes its run, occurrence, slot and destination");
			CPacketReader reader(writer.Get_Buffer()); C2S_DEBUG_WORLD_PLAYBACK decoded;
			tests.Require(Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 9u && decoded.iRoomPlayerSlot == 3u && decoded.strOccurrenceId == request.strOccurrenceId &&
				decoded.fPositionX == request.fPositionX && decoded.fPositionY == request.fPositionY && decoded.fPositionZ == request.fPositionZ,
				"Arrival packet preserves exact slot coordinates and replay identity");
			auto bytes = writer.Get_Buffer(); bytes.pop_back();
			CPacketReader truncated(bytes); decoded.strOccurrenceId = "sentinel";
			tests.Require(!Read_Message(truncated, decoded) && decoded.strOccurrenceId == "sentinel",
				"Truncated arrival packet does not partially commit decoded intent");
			bool rejects = true;
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				auto bad = request;
				if (scenario == 0u) bad.iRoomPlayerSlot = 4u;
				if (scenario == 1u) bad.iRunEpoch = 0u;
				if (scenario == 2u) bad.fPositionX = std::numeric_limits<float>::quiet_NaN();
				if (scenario == 3u) bad.eWorldId = WORLD_ID::BERN;
				if (scenario == 4u) bad.fPositionZ = 100001.f;
				CPacketWriter invalid; rejects = rejects && !Write_Message(invalid, bad) && invalid.Get_Buffer().empty();
			}
			tests.Require(rejects, "Arrival rejects wrong world, invalid epoch, slot and coordinates before writing");
			S2C_DEBUG_WORLD_PLAYBACK_RESULT receipt{};
			receipt.iRequestSequence = request.iRequestSequence; receipt.eWorldId = request.eWorldId;
			receipt.eOperation = request.eOperation; receipt.strTargetId = request.strTargetId;
			receipt.eResult = R::SKIPPED_PLAYER;
			CPacketWriter replyWriter; const bool wroteReply = Write_Message(replyWriter, receipt);
			CPacketReader replyReader(replyWriter.Get_Buffer()); S2C_DEBUG_WORLD_PLAYBACK_RESULT reply;
			tests.Require(wroteReply && Read_Message(replyReader, reply) && replyReader.Get_RemainingSize() == 0u &&
				reply.iRequestSequence == 17u && reply.eOperation == request.eOperation && reply.eResult == R::SKIPPED_PLAYER,
				"Arrival skip reply uses the existing request-correlated result envelope");
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLAY_SEQUENCE;
			CPacketWriter legacyWriter; const bool wroteLegacy = Write_Message(legacyWriter, request);
			CPacketReader legacyReader(legacyWriter.Get_Buffer());
			tests.Require(wroteLegacy && Read_Message(legacyReader, decoded) && legacyReader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 0u && decoded.strOccurrenceId.empty(), "Ordinary world playback keeps its original payload shape");
		}
#ifdef _DEBUG
		{
			WSADATA winsock{};
			const bool socketReady = WSAStartup(MAKEWORD(2, 2), &winsock) == 0;
			tests.Require(socketReady, "Arrival fixture prepares unconnected session sockets without a listener");
			if (socketReady)
			{
				const std::array<std::array<float, 2>, 4> locations{{ {-3.913588f,739.883125f},
					{-3.290587f,742.070391f}, {-5.324063f,738.528281f}, {-1.156042f,742.512031f} }};
				for (unsigned count = 1u; count <= 4u; ++count)
				{
					auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
					std::vector<std::shared_ptr<CClientSession>> sessions;
					const auto join = [&](PLAYER_ID id)
					{
						const SESSION_ID sessionId = id + 1000u;
						auto connection = std::make_shared<CClientSession>(sessionId, ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
							CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
						sessions.push_back(connection); room->m_Sessions[sessionId] = connection;
						room->m_PlayerIdBySessionId[sessionId] = id;
						auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = sessionId;
						player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
						player.fPositionX = -100.f - static_cast<float>(id); player.fPositionY = 1.3f; player.fPositionZ = 740.f;
						player.hasMoveGoal = true; player.iCurrentSkillId = 34010u; player.eAction = PLAYER_ACTION_STATE::SKILL;
					};
					// Reverse insertion proves that stable PlayerId order, not joins or session order, chooses slots.
					for (unsigned n = count; n > 0u; --n) join(n * 10u);
					if (count == 4u) join(50u);
					bool nativeGround = room->Is_Ready();
					std::array<SERVER_NAV_POINT, 4> ground{};
					for (unsigned slot = 0u; slot < 4u; ++slot)
						nativeGround = nativeGround && room->m_ServerNavigation.Sample_Position(locations[slot][0], locations[slot][1], ground[slot]);
					tests.Require(nativeGround, "Arrival samples all four authored fireworks XZ on actual Server navigation");
					if (!nativeGround) continue;
					C2S_DEBUG_WORLD_PLAYBACK request{};
					request.iRequestSequence = 1u; request.iRunEpoch = 1u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
					request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
					const auto position = [&](unsigned slot)
					{
						request.iRoomPlayerSlot = static_cast<std::uint8_t>(slot);
						request.strOccurrenceId = request.strTargetId + ".logic." + std::to_string(slot + 1u);
						request.fPositionX = ground[slot].x; request.fPositionY = ground[slot].y; request.fPositionZ = ground[slot].z;
					};
					bool movedInOrder = true;
					for (unsigned slot = 0u; slot < 4u; ++slot)
					{
						position(slot); const auto verdict = room->Apply_DebugRoomPlayerArrival(1010u, request);
						movedInOrder = movedInOrder && verdict == (slot < count ? R::ACCEPTED : R::SKIPPED_PLAYER);
						if (slot < count)
						{
							const auto& player = room->m_Players.at((slot + 1u) * 10u);
							movedInOrder = movedInOrder && player.fPositionX == ground[slot].x && player.fPositionY == ground[slot].y &&
								player.fPositionZ == ground[slot].z && !player.hasMoveGoal && player.iCurrentSkillId == INVALID_SKILL_ID && player.iCurrentHp == 100u;
						}
					}
					tests.Require(movedInOrder, "One through four connected players arrive by PlayerId; missing slots are successful skips");
					if (count == 4u)
						tests.Require(room->m_RoomPlayerArrivalRuns.at(1010u).Players.size() == 4u && room->m_Players.at(50u).hasMoveGoal,
							"Arrival roster caps at four and preserves any later connected player");
					position(0u); auto& first = room->m_Players.at(10u); first.fPositionX -= 20.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ALREADY_USED && first.fPositionX == ground[0].x - 20.f,
						"Duplicate arrival occurrence never teleports an already consumed slot twice");
					request.iRunEpoch = 2u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACCEPTED && first.fPositionX == ground[0].x,
						"Explicit new playback epoch permits the same occurrence again");
					first.fPositionX -= 20.f; first.hasMoveGoal = true; first.iCurrentSkillId = 34010u;
					request.iRunEpoch = 1u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::STALE_REQUEST && first.hasMoveGoal && first.iCurrentSkillId == 34010u,
						"An older playback cannot mutate the current run");
					request.iRunEpoch = 2u; request.strOccurrenceId += ".wrongheight"; request.fPositionY += 100.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACTION_REJECTED && first.fPositionX == ground[0].x - 20.f &&
						first.hasMoveGoal && first.iCurrentSkillId == 34010u && first.iCurrentHp == 100u,
						"Rejected destination preserves position, action, movement and health");
					request.eWorldId = WORLD_ID::BERN;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::WRONG_WORLD && first.hasMoveGoal,
						"Arrival cannot cross the requesting room's world boundary");
					request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					if (count > 1u)
					{
						// Epoch 2 already captured PlayerId 20; replacing its room binding cannot retarget that slot.
						room->m_PlayerIdBySessionId.erase(1020u); room->m_Players.erase(20u); join(21u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(21u).hasMoveGoal,
							"Departed roster member is skipped without teleporting its newly joined replacement");
					}
					else
					{
						join(20u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(20u).hasMoveGoal,
							"Joining midway does not fill a slot absent from the playback's fixed roster");
					}
				}
				WSACleanup();
			}
		}
#endif
		std::cout << "World playback contract failures: " << tests.failures << '\n';
		return tests.failures == 0 ? 0 : 1;
	}
