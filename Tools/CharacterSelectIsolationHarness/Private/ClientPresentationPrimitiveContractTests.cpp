#include "ClientReplicationEvent.h"
#include "CombatHUDViewModel.h"
#include "LocalMovePrediction.h"
#include "MouseButtonReleaseGate.h"
#include "PartyTransferNotice.h"
#include "ReplicatedPlayerHealth.h"
#include "Sound/TrackedSoundChannel.h"

#include <iostream>
#include <limits>
#include <vector>

namespace
{
	bool Require(const bool condition, const char* message)
	{
		if (!condition)
			std::cerr << "ClientPresentationPrimitiveContracts: " << message << '\n';
		return condition;
	}

    bool VerifyKoukuVisibleCooldownSlots()
    {
        Client::HUD_KOUKU_GIMMICK_STATE hud;
        hud.eHudMode = Client::HUD_KOUKU_HUD_MODE::MAZE;
        hud.ModeSkillIndexBySlot[0] = 0; hud.ModeSkillIndexBySlot[1] = 1;
        hud.CooldownEndTicks[0] = 140u; hud.CooldownEndTicks[1] = 112u;
        if (!Require(hud.Has_VisibleSkillSlot(0u) && !hud.Has_VisibleSkillSlot(1u) &&
            !hud.Has_VisibleSkillSlot(Client::HUD_KOUKU_SLOT_COUNT),
            "maze LMB wire cooldown appeared as a W icon, ring or countdown")) return false;
        for (const auto mode : {Client::HUD_KOUKU_HUD_MODE::POLYMORPH,
            Client::HUD_KOUKU_HUD_MODE::MARIO, Client::HUD_KOUKU_HUD_MODE::DANCE})
        {
            hud.eHudMode = mode;
            if (!Require(hud.Has_VisibleSkillSlot(0u) && hud.Has_VisibleSkillSlot(1u) &&
                !hud.Has_VisibleSkillSlot(2u), "maze filtering hid another mode's assigned W skill")) return false;
        }
        hud.eHudMode = Client::HUD_KOUKU_HUD_MODE::NONE;
        return Require(!hud.Has_VisibleSkillSlot(0u) && !hud.Has_VisibleSkillSlot(1u) &&
            hud.CooldownEndTicks[0] == 140u && hud.CooldownEndTicks[1] == 112u,
            "interaction HUD visibility altered replicated cooldown deadlines or leaked into the class HUD");
    }

	bool VerifyMousePressOwnership()
	{
		Engine::CMouseButtonReleaseGate left, right;
		if (!Require(right.Observe(true, false), "fresh world RMB was suppressed") ||
			!Require(!right.Observe(true, true) && !left.Observe(true, true),
				"popup opening frame leaked RMB movement/LMB attack"))
		{
			return false;
		}
		for (int frame = 0; frame < 10; ++frame)
		{
			if (!Require(!right.Observe(true, false) && !left.Observe(true, false),
				"closing popup rearmed a still-held button"))
			{
				return false;
			}
		}
		if (!Require(!left.Observe(false, false) && left.Observe(true, false) &&
			!right.Observe(true, false), "LMB release incorrectly rearmed RMB") ||
			!Require(!right.Observe(false, false) && right.Observe(true, false),
				"physical RMB release did not permit the next world click"))
		{
			return false;
		}
		return Require(!left.Observe(true, true) && !left.Observe(true, false),
			"one input consumer cleared another consumer's press ownership");
	}

	struct FAKE_CHANNEL final
	{
		int iStops = 0;
		void stop() { ++iStops; }
	};

	bool VerifyIndependentLoopedSound()
	{
		Engine::CTrackedSoundChannel<FAKE_CHANNEL> music, uiLoop;
		FAKE_CHANNEL bgm, wait, replacement, failedStage;
		const auto start = [](auto& owner, FAKE_CHANNEL& channel)
		{
			return owner.Try_Replace(
				[&](FAKE_CHANNEL*& staged) { staged = &channel; return true; });
		};
		if (!Require(start(music, bgm) && start(uiLoop, wait) && 0 == bgm.iStops,
			"starting UI wait sound stopped BGM") ||
			!Require(!uiLoop.Try_Replace([](FAKE_CHANNEL*&) { return false; }) &&
				0 == wait.iStops && 0 == bgm.iStops,
				"load failure stopped committed audio") ||
			!Require(!uiLoop.Try_Replace([&](FAKE_CHANNEL*& staged)
				{ staged = &failedStage; return false; }) &&
				1 == failedStage.iStops && 0 == wait.iStops && 0 == bgm.iStops,
				"failed stage did not roll back only its staged channel") ||
			!Require(start(uiLoop, replacement) && 1 == wait.iStops && 0 == bgm.iStops,
				"UI loop replacement touched the music owner"))
		{
			return false;
		}
		uiLoop.Stop();
		uiLoop.Stop();
		if (!Require(1 == replacement.iStops && 0 == bgm.iStops,
			"UI cleanup stopped BGM or stopped its channel twice"))
		{
			return false;
		}
		music.Stop();
		return Require(1 == bgm.iStops, "music owner failed to clean up its channel");
	}

	bool VerifyReplicatedPartyHealth()
	{
		using namespace LostArk::Shared;
		Client::CReplicatedPlayerHealth health;
		S2C_WORLD_SNAPSHOT snapshot{};
		snapshot.iServerTick = 10u;
		PLAYER_SNAPSHOT first{}, second{};
		first.iNetEntityId = 101u;
		first.iCurrentHp = 25u;
		first.iMaximumHp = 100u;
		first.iInvulnerabilityZonePulseTick = snapshot.iServerTick;
		first.bRonaunGuard = true;
		first.iRonaunGrantTick = snapshot.iServerTick;
		second.iNetEntityId = 202u;
		second.iCurrentHp = 0u;
		second.iMaximumHp = 200u;
		snapshot.Players = { second, first };
		if (!Require(!health.Find(101u).hasSnapshot && health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 10u &&
			health.Find(101u).bRonaunGuard && health.Find(101u).iRonaunGrantTick == 10u &&
			health.Find(202u).iRonaunGrantTick == 0u &&
			health.Find(202u).iInvulnerabilityZonePulseTick == 0u &&
			health.Find(202u).hasSnapshot && health.Find(202u).Get_Ratio() == 0.f &&
			!health.Find(999u).hasSnapshot,
			"HP join fabricated data, used row order, or hid zero HP"))
		{
			return false;
		}
		snapshot.Players[1].iCurrentHp = 90u;
		snapshot.Players[1].iInvulnerabilityZonePulseTick = 0u;
		snapshot.Players[1].bRonaunGuard = false;
		if (!Require(health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 10u,
			"duplicate tick replaced current HP"))
		{
			return false;
		}
		snapshot.iServerTick = 9u;
		if (!Require(health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 10u,
			"older tick replaced current HP"))
		{
			return false;
		}
		snapshot.iServerTick = 11u;
		snapshot.Players.push_back(first);
		if (!Require(!health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 10u,
			"duplicate entity partially committed HP"))
		{
			return false;
		}
		snapshot.Players.pop_back();
		snapshot.Players[1].iMaximumHp = 0u;
		if (!Require(!health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 10u,
			"invalid HP partially committed the snapshot"))
		{
			return false;
		}
		snapshot.Players[1].iMaximumHp = 100u;
		if (!Require(health.Apply_Snapshot(snapshot) &&
			health.Find(101u).iInvulnerabilityZonePulseTick == 0u &&
			!health.Find(101u).bRonaunGuard && health.Find(101u).iRonaunGrantTick == 10u,
			"new snapshot lost grant text or retained consumed protection"))
		{
			return false;
		}
		snapshot.iServerTick = 12u;
		snapshot.Players = { second };
		if (!Require(health.Apply_Snapshot(snapshot) && !health.Find(101u).hasSnapshot,
			"out-of-world member retained stale HP"))
		{
			return false;
		}
		health.Erase(202u);
		if (!Require(!health.Find(202u).hasSnapshot, "despawn retained HP"))
			return false;
		health.Reset();
		snapshot.iServerTick = 1u;
		if (!Require(health.Apply_Snapshot(snapshot) && health.Find(202u).hasSnapshot,
			"new-world tick origin was rejected after reset"))
		{
			return false;
		}
		health.Reset();
		return Require(!health.Find(202u).hasSnapshot,
			"disconnect reset retained party HP");
	}

	bool VerifyPartyTransferNotice()
	{
		using namespace LostArk::Shared;
		for (const auto result : {
			PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER,
			PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL,
			PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE,
			PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED,
			PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY })
		{
			if (!Require(nullptr != Client::Get_PartyTransferFailureText(result),
				"server transfer failure has no product notice"))
			{
				return false;
			}
		}
		using Event = Client::CLIENT_REPLICATION_EVENT_TYPE;
		return Require(
			nullptr == Client::Get_PartyTransferFailureText(
				static_cast<PARTY_TRANSFER_RESULT>(255)) &&
			!Client::Can_CoalesceAdjacentReplicationEvents(
				Event::WORLD_SNAPSHOT, Event::PARTY_TRANSFER_RESULT) &&
			!Client::Can_CoalesceAdjacentReplicationEvents(
				Event::PARTY_TRANSFER_RESULT, Event::WORLD_SNAPSHOT),
			"unknown result normalized or reliable notice lost ordering barrier");
	}

	using MovePrediction = Client::CLocalMovePrediction;
	using MoveDisposition = MovePrediction::SnapshotDisposition;

	MovePrediction::Snapshot MakeMoveSnapshot(const std::uint32_t tick,
		const std::uint32_t acknowledgedSequence = 0u)
	{
		MovePrediction::Snapshot snapshot{};
		snapshot.serverTick = tick;
		snapshot.processedMoveSequence = acknowledgedSequence;
		snapshot.moveSpeed = 6.f;
		snapshot.canPredictMove = true;
		return snapshot;
	}

	bool NearMove(const float actual, const float expected)
	{
		return std::abs(actual - expected) < 0.0001f;
	}

	bool VerifyImmediateMovePrediction()
	{
		MovePrediction prediction;
		MovePrediction::Pose origin{};
		if (!Require(!prediction.Can_SubmitMove(0.0) &&
			!prediction.SubmitMove(1u, 0.0, origin),
			"movement predicted before an authoritative initial state") ||
			!Require(prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, origin) ==
				MoveDisposition::RESET && prediction.Can_SubmitMove(0.01) &&
				prediction.Get_MoveSpeed() == 6.f &&
				prediction.SubmitMove(1u, 0.01, origin),
				"fresh valid click did not start prediction"))
		{
			return false;
		}
		const MovePrediction::Pose firstStep{ { 0.1f, 0.f, 0.f }, 90.f, true };
		const auto immediate = prediction.Update(0.01, 0.f, firstStep);
		if (!Require(immediate.active && immediate.useLocalPath && immediate.pose.isMoving &&
			NearMove(immediate.pose.position.x, 0.1f),
			"local movement/RUN waited for the first server acknowledgement") ||
			!Require(prediction.ApplySnapshot(MakeMoveSnapshot(11u), 0.03, firstStep) ==
				MoveDisposition::PRESERVE_LOCAL_PATH,
				"an in-flight idle snapshot cancelled the newly clicked path"))
		{
			return false;
		}
		auto invalid = MakeMoveSnapshot(12u);
		invalid.position.x = std::numeric_limits<float>::quiet_NaN();
		if (!Require(prediction.ApplySnapshot(invalid, 0.04, firstStep) ==
			MoveDisposition::IGNORED && prediction.HasPendingMove() &&
			!prediction.SubmitMove(1u, 0.04, firstStep),
			"invalid snapshot or duplicate click discarded the committed path"))
		{
			return false;
		}
		invalid = MakeMoveSnapshot(12u);
		invalid.canPredictMove = false;
		invalid.hasMoveGoal = true;
		if (!Require(prediction.ApplySnapshot(invalid, 0.04, firstStep) ==
			MoveDisposition::IGNORED && prediction.HasPendingMove(),
			"a contradictory locked/moving snapshot replaced committed prediction"))
		{
			return false;
		}
		const MovePrediction::Pose reached{ { 0.2f, 0.f, 0.f }, 90.f, false };
		const auto arrival = prediction.Update(0.05, 0.016f, reached);
		return Require(arrival.useLocalPath && !arrival.pose.isMoving &&
			NearMove(arrival.pose.position.x, 0.2f),
			"an unacknowledged local arrival kept RUN active or rewound the pose");
	}

	bool VerifyAcknowledgedMoveCorrection()
	{
		MovePrediction prediction;
		MovePrediction::Pose visual{};
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, visual);
		(void)prediction.SubmitMove(1u, 0.01, visual);
		visual = { { 0.25f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.05, 0.016f, visual);
		(void)prediction.CompleteLocalPathFrame(0.05, visual);
		auto accepted = MakeMoveSnapshot(12u, 1u);
		accepted.position.x = 0.2f;
		accepted.yawDegrees = 90.f;
		accepted.hasMoveGoal = true;
		accepted.nextWaypoint = { 10.f, 0.f, 0.f };
		if (!Require(prediction.ApplySnapshot(accepted, 0.06, visual) ==
			MoveDisposition::RECONCILE && !prediction.HasPendingMove(),
			"accepted click did not retire its local navigation path"))
		{
			return false;
		}
		const auto edge = prediction.Update(0.06, 0.f, {});
		const auto reconciled = prediction.Update(0.15, 0.09f, {});
		const auto capped = prediction.Update(0.30, 0.15f, {});
		if (!Require(!edge.useLocalPath && edge.pose.isMoving &&
			NearMove(edge.pose.position.x, visual.position.x),
			"small server correction snapped at acknowledgement") ||
			!Require(reconciled.pose.position.x > visual.position.x &&
				reconciled.pose.position.x - visual.position.x <= accepted.moveSpeed * 1.15f * 0.09f,
				"acknowledged movement failed to advance from its authoritative snapshot") ||
			!Require(capped.pose.position.x >= reconciled.pose.position.x &&
				NearMove(prediction.Update(0.34, 0.04f, {}).pose.position.x, capped.pose.position.x),
				"movement extrapolated beyond the bounded 150 ms horizon"))
		{
			return false;
		}

		MovePrediction shortPath;
		auto shortSnapshot = MakeMoveSnapshot(1u);
		shortSnapshot.hasMoveGoal = true;
		shortSnapshot.nextWaypoint = { 0.03f, 0.02f, 0.f };
		(void)shortPath.ApplySnapshot(shortSnapshot, 0.0, {});
		const auto waypoint = shortPath.Update(0.1, 0.1f, {});
		if (!Require(NearMove(waypoint.pose.position.x, 0.03f) &&
			NearMove(waypoint.pose.position.y, shortSnapshot.position.y),
			"extrapolation overshot XZ or invented height from a distant waypoint"))
		{
			return false;
		}

		MovePrediction delayedAck;
		(void)delayedAck.ApplySnapshot(MakeMoveSnapshot(1u), 0.0, {});
		(void)delayedAck.SubmitMove(1u, 0.01, {});
		const MovePrediction::Pose delayedVisual{ { 0.6f, 0.f, 0.f }, 90.f, true };
		accepted.serverTick = 10u;
		(void)delayedAck.ApplySnapshot(accepted, 0.31, delayedVisual);
		const auto latencyCapped = delayedAck.Update(0.35, 0.04f, {});
		const auto sameFrameAgain = delayedAck.Update(0.35, 0.04f, {});
		return Require(std::abs(latencyCapped.pose.position.x - delayedVisual.position.x) <= accepted.moveSpeed * 1.15f * 0.04f &&
			NearMove(sameFrameAgain.pose.position.x, latencyCapped.pose.position.x),
			"large ACK latency or a second frame query advanced prediction twice");
	}

	bool VerifyMoveSequenceOrdering()
	{
		MovePrediction prediction;
		MovePrediction::Pose visual{};
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, visual);
		(void)prediction.SubmitMove(1u, 0.01, visual);
		visual = { { 0.1f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.02, 0.01f, visual);
		(void)prediction.CompleteLocalPathFrame(0.02, visual);
		if (!Require(prediction.SubmitMove(2u, 0.03, visual),
			"a quick second click could not replace the first prediction"))
		{
			return false;
		}
		visual = { { 0.08f, 0.f, 0.04f }, -90.f, true };
		(void)prediction.Update(0.04, 0.01f, visual);
		(void)prediction.CompleteLocalPathFrame(0.04, visual);
		auto firstAck = MakeMoveSnapshot(11u, 1u);
		firstAck.position.x = 0.1f;
		firstAck.hasMoveGoal = true;
		firstAck.nextWaypoint.x = 10.f;
		if (!Require(prediction.ApplySnapshot(firstAck, 0.05, visual) ==
			MoveDisposition::PRESERVE_LOCAL_PATH && prediction.HasPendingMove(),
			"the first click's acknowledgement overwrote a newer direction") ||
			!Require(prediction.ApplySnapshot(MakeMoveSnapshot(10u, 2u), 0.06, visual) ==
				MoveDisposition::IGNORED &&
				prediction.ApplySnapshot(MakeMoveSnapshot(12u, 0u), 0.07, visual) ==
					MoveDisposition::IGNORED && prediction.HasPendingMove(),
				"stale snapshot tick or regressing acknowledgement consumed the new click"))
		{
			return false;
		}
		const auto redirected = prediction.Update(0.08, 0.01f, visual);
		if (!Require(NearMove(redirected.pose.position.z, 0.04f) &&
			NearMove(redirected.pose.yawDegrees, -90.f),
			"re-click direction was replayed from the older path"))
		{
			return false;
		}

		prediction.Reset();
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(0xffffffffu, 0xfffffffeu), 1.0, {});
		if (!Require(prediction.SubmitMove(0xffffffffu, 1.01, {}) &&
			prediction.ApplySnapshot(MakeMoveSnapshot(0u, 0xffffffffu), 1.02, {}) ==
				MoveDisposition::RECONCILE && prediction.SubmitMove(1u, 1.03, {}),
			"server tick or movement sequence wrap broke fresh commands"))
		{
			return false;
		}
		return Require(!prediction.SubmitMove(0u, 1.04, {}) &&
			!prediction.SubmitMove(0xfffffffeu, 1.04, {}) && prediction.HasPendingMove(),
			"reserved zero or old pre-wrap sequence replaced current prediction");
	}

	bool VerifyMoveRejectionAndForcedState()
	{
		MovePrediction prediction;
		MovePrediction::Pose visual{};
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, visual);
		(void)prediction.SubmitMove(1u, 0.01, visual);
		visual = { { 0.3f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.04, 0.03f, visual);
		(void)prediction.CompleteLocalPathFrame(0.04, visual);
		if (!Require(prediction.ApplySnapshot(MakeMoveSnapshot(11u, 1u), 0.05, visual) ==
			MoveDisposition::RECONCILE && !prediction.HasPendingMove(),
			"server rejection did not consume the predicted input"))
		{
			return false;
		}
		const auto rejected = prediction.Update(0.14, 0.09f, {});
		if (!Require(!rejected.pose.isMoving && NearMove(rejected.pose.position.x, 0.f),
			"rejected move kept travelling instead of correcting to the server") ||
			!Require(prediction.SubmitMove(2u, 0.15, rejected.pose),
				"server rejection permanently disabled future movement"))
		{
			return false;
		}
		auto captured = MakeMoveSnapshot(12u, 1u);
		captured.canPredictMove = false;
		captured.position = { 3.f, 2.f, 1.f };
		if (!Require(prediction.ApplySnapshot(captured, 0.16, visual) ==
			MoveDisposition::RESET && !prediction.HasPendingMove() &&
			!prediction.Can_SubmitMove(0.17),
			"death/skill/capture/forced movement retained a pending click"))
		{
			return false;
		}
		const auto locked = prediction.Update(0.16, 0.f, visual);
		if (!Require(!locked.pose.isMoving && NearMove(locked.pose.position.x, 3.f) &&
			NearMove(locked.pose.position.y, 2.f),
			"forced authoritative pose was smoothed through an invalid local position"))
		{
			return false;
		}
		auto teleport = MakeMoveSnapshot(13u, 1u);
		teleport.position.x = -5.f;
		if (!Require(prediction.ApplySnapshot(teleport, 0.2, locked.pose) ==
			MoveDisposition::RESET &&
			NearMove(prediction.Update(0.2, 0.f, {}).pose.position.x, -5.f),
			"a discontinuous server teleport retained the old correction offset"))
		{
			return false;
		}
		const MovePrediction::Pose invalidVisual{ { 20.f, 0.f, 0.f }, 0.f, true };
		teleport.serverTick = 14u;
		if (!Require(prediction.ApplySnapshot(teleport, 0.23, invalidVisual) ==
			MoveDisposition::RESET &&
			NearMove(prediction.Update(0.23, 0.f, {}).pose.position.x, -5.f),
			"large local/server disagreement was hidden in a smooth correction"))
		{
			return false;
		}
		prediction.Reset();
		return Require(!prediction.Update(0.3, 0.1f, visual).active &&
			!prediction.HasPendingMove() && !prediction.Can_SubmitMove(0.3),
			"class/world/disconnect reset carried prediction into another character");
	}

	bool VerifyMoveCorrectionSpeedAndContinuousSnapshots()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(1u);
		snapshot.moveSpeed = 2.95f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		(void)prediction.SubmitMove(1u, 0.01, {});
		const MovePrediction::Pose visual{ { 0.8f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.09, 0.016f, visual);
		(void)prediction.CompleteLocalPathFrame(0.09, visual);
		snapshot.serverTick = 4u; snapshot.processedMoveSequence = 1u;
		if (!Require(prediction.ApplySnapshot(snapshot, 0.1, visual) == MoveDisposition::RECONCILE,
			"ordinary ACK error reset the visual pose")) return false;
		auto previous = prediction.Update(0.1, 0.f, {}).pose;
		if (!Require(NearMove(previous.position.x, visual.position.x),
			"ACK correction changed position on its first frame")) return false;
		for (int frame = 1; frame <= 17; ++frame)
		{
			const auto current = prediction.Update(0.1 + frame / 60.0, 1.f / 60.f, {}).pose;
			if (!Require(previous.position.x - current.position.x <= snapshot.moveSpeed / 60.f + 0.0001f &&
				current.position.x >= -0.0001f && current.position.x <= previous.position.x + 0.0001f,
				"ACK residual correction exceeded locomotion speed or reversed/overshot")) return false;
			previous = current;
		}
		if (!Require(NearMove(previous.position.x, 0.f), "bounded ACK correction never reached its stationary target")) return false;
		const MovePrediction::Pose diverged{ { 3.f, 0.f, 0.f }, 0.f, false };
		snapshot.serverTick = 13u;
		if (!Require(prediction.ApplySnapshot(snapshot, 0.4, diverged) == MoveDisposition::RECONCILE &&
			NearMove(prediction.Update(0.4, 0.f, {}).pose.position.x, 3.f),
			"continuous Server snapshots snapped a visual-only disagreement above 2m")) return false;
		const auto softened = prediction.Update(0.4 + 1.0 / 60.0, 1.f / 60.f, {}).pose;
		if (!Require(3.f - softened.position.x <= snapshot.moveSpeed / 60.f + 0.0001f,
			"large ordinary visual error exceeded the correction speed bound")) return false;
		snapshot.serverTick = 14u; snapshot.position.x = 20.f;
		return Require(prediction.ApplySnapshot(snapshot, 0.45, softened) == MoveDisposition::RESET &&
			NearMove(prediction.Update(0.45, 0.f, {}).pose.position.x, 20.f),
			"true Server teleport was hidden by the larger visual correction threshold");
	}

	bool VerifyLatestOppositeHeadingAndGroundHeight()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(1u);
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		(void)prediction.SubmitMove(1u, 0.01, {});
		const MovePrediction::Pose latest{ { 0.f, 7.f, 0.f }, -90.f, true };
		(void)prediction.SubmitMove(2u, 0.02, latest);
		snapshot.serverTick = 2u; snapshot.processedMoveSequence = 1u;
		snapshot.hasMoveGoal = true; snapshot.nextWaypoint = { 10.f, 40.f, 0.f };
		if (!Require(prediction.ApplySnapshot(snapshot, 0.03, latest) == MoveDisposition::PRESERVE_LOCAL_PATH,
			"old ACK replaced the latest opposite click")) return false;
		// Keep Server positions continuous while its yaw still trails the new goal.
		snapshot.serverTick = 3u; snapshot.processedMoveSequence = 2u;
		snapshot.yawDegrees = 90.f; snapshot.nextWaypoint = { -10.f, 40.f, 0.f };
		const MovePrediction::Pose levelVisual{ { 0.f, 0.f, 0.f }, -90.f, true };
		if (!Require(prediction.ApplySnapshot(snapshot, 0.05, levelVisual) == MoveDisposition::RECONCILE,
			"latest click ACK failed to reconcile")) return false;
		const auto heading = prediction.Update(0.15, 0.1f, {}).pose;
		if (!Require(heading.position.x < 0.f && NearMove(heading.yawDegrees, -90.f) &&
			NearMove(heading.position.y, 0.f),
			"latest goal moved backward against its target yaw or gained distant-waypoint height")) return false;
		MovePrediction elevated;
		snapshot = MakeMoveSnapshot(1u); snapshot.position.y = 7.f;
		snapshot.hasMoveGoal = true; snapshot.nextWaypoint = { 10.f, 40.f, 0.f };
		(void)elevated.ApplySnapshot(snapshot, 0.0, { { 0.f, 7.f, 0.f }, 0.f, false });
		const auto highWaypoint = elevated.Update(0.15, 0.15f, {}).pose;
		return Require(highWaypoint.position.x > 0.f && NearMove(highWaypoint.position.y, 7.f),
			"helper projected a distant waypoint height before Character sampled current ground");
	}

	bool VerifyAcknowledgedBearingHasOneSmoothingOwner()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 2.95f; snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint = { -100.f, 0.f, 0.f };
		snapshot.yawDegrees = 90.f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		for (int ack = 1; ack <= 15; ++ack)
		{
			const double now = ack / 30.0;
			snapshot.serverTick = 100u + ack;
			snapshot.position.x = static_cast<float>(-2.95 * now);
			const MovePrediction::Pose visual{ { snapshot.position.x + 3.f, 0.f, 0.f }, 90.f, true };
			if (!Require(prediction.ApplySnapshot(snapshot, now, visual) == MoveDisposition::RECONCILE,
				"continuous ACK unexpectedly reset the bearing fixture")) return false;
			const auto target = prediction.Update(now, 0.f, {}).pose;
			if (!Require(NearMove(target.position.x, visual.position.x) && NearMove(target.yawDegrees, -90.f),
				"ACK position residual also delayed target bearing before Character yaw smoothing")) return false;
		}
		return true;
	}

	bool VerifyDelayedSnapshotsPreserveNormalMotion()
	{
		for (const std::uint32_t initialTick : { 100u, 0xfffffff0u })
		{
			for (const std::uint32_t gapTicks : { 30u, 53u, 57u })
			{
				MovePrediction prediction;
				auto snapshot = MakeMoveSnapshot(initialTick);
				snapshot.moveSpeed = 2.95f;
				snapshot.hasMoveGoal = true;
				snapshot.nextWaypoint.x = 100.f;
				(void)prediction.ApplySnapshot(snapshot, 0.0, {});
				if (!Require(prediction.SubmitMove(1u, 0.01, {}) &&
					prediction.SubmitMove(2u, 0.02, {}),
					"delayed snapshot fixture did not admit two quick clicks")) return false;
				const MovePrediction::Pose visual{ { 0.295f, 0.f, 0.f }, 90.f, true };
				(void)prediction.Update(0.12, 0.1f, visual);
				(void)prediction.CompleteLocalPathFrame(0.12, visual);
				// The Server continues at its legal speed while the main thread stalls.
				const double elapsed = static_cast<double>(gapTicks) / 30.0;
				snapshot.serverTick += gapTicks;
				snapshot.processedMoveSequence = 2u;
				snapshot.position.x = snapshot.moveSpeed * static_cast<float>(elapsed);
				if (!Require(prediction.ApplySnapshot(snapshot, elapsed, visual) ==
					MoveDisposition::RECONCILE && !prediction.HasPendingMove(),
					"normal Server travel across a delayed snapshot was classified as teleport") ||
					!Require(NearMove(prediction.Update(elapsed, 0.f, {}).pose.position.x, visual.position.x),
						"delayed normal ACK snapped the first resumed visual frame")) return false;
			}
		}

		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 2.95f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		auto invalid = snapshot;
		invalid.serverTick += 0x80000000u;
		invalid.position.x = 20.f;
		if (!Require(prediction.ApplySnapshot(invalid, 1.0, {}) == MoveDisposition::IGNORED,
			"ambiguous half-range Server tick was admitted as elapsed motion")) return false;
		invalid.serverTick = 99u;
		if (!Require(prediction.ApplySnapshot(invalid, 1.0, {}) == MoveDisposition::IGNORED,
			"backward Server tick was admitted as elapsed motion")) return false;
		invalid.serverTick = 101u;
		invalid.position.x = (std::numeric_limits<float>::infinity)();
		if (!Require(prediction.ApplySnapshot(invalid, 1.0, {}) == MoveDisposition::IGNORED,
			"non-finite delayed snapshot was admitted")) return false;
		snapshot.serverTick = 101u;
		snapshot.position.x = 20.f;
		if (!Require(prediction.ApplySnapshot(snapshot, 1.0, {}) == MoveDisposition::RESET &&
			NearMove(prediction.Update(1.0, 0.f, {}).pose.position.x, 20.f),
			"actual one-tick Server teleport was smoothed after removing the time cap")) return false;
		// Even a large forward tick gap cannot hide an excessive visual disagreement.
		snapshot.serverTick += 300u;
		snapshot.position.x = 40.f;
		return Require(prediction.ApplySnapshot(snapshot, 11.0, { { 20.f, 0.f, 0.f }, 0.f, false }) ==
			MoveDisposition::RESET, "large delayed visual disagreement lost its hard reset");
	}

	bool VerifyTotalMovePresentationBudget()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 2.95f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		if (!Require(prediction.SubmitMove(1u, 0.025, {}),
			"bounded movement fixture rejected its first click")) return false;
		MovePrediction::Pose visual{ { 0.07375f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.05, 0.025f, visual);
		(void)prediction.CompleteLocalPathFrame(0.05, visual);
		if (!Require(prediction.SubmitMove(2u, 0.075, visual),
			"bounded movement fixture rejected its second click")) return false;
		visual.position.x = 0.1475f;
		(void)prediction.Update(0.1, 0.025f, visual);
		(void)prediction.CompleteLocalPathFrame(0.1, visual);
		snapshot.serverTick = 106u;
		snapshot.processedMoveSequence = 2u;
		snapshot.position.x = 0.59f;
		snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint.x = 100.f;
		if (!Require(prediction.ApplySnapshot(snapshot, 0.2, visual) == MoveDisposition::RECONCILE &&
			NearMove(prediction.Update(0.2, 0.f, {}).pose.position.x, visual.position.x),
			"double-click ACK changed the presented position immediately")) return false;
		for (unsigned frame = 1u; frame <= 240u; ++frame)
		{
			const double now = 0.2 + frame / 40.0;
			const auto next = prediction.Update(now, 0.025f, {}).pose;
			const auto repeated = prediction.Update(now, 0.025f, {}).pose;
			if (!Require(next.position.x >= visual.position.x - 0.00001f &&
				next.position.x - visual.position.x <= 0.08482f &&
				NearMove(repeated.position.x, next.position.x),
				"40 FPS correction exceeded total travel budget or advanced twice at one time")) return false;
			visual = next;
			snapshot.serverTick = 106u + static_cast<unsigned>(std::floor(frame * 30.0 / 40.0));
			snapshot.position.x = 0.59f + (snapshot.serverTick - 106u) * snapshot.moveSpeed / 30.f;
			if (!Require(prediction.ApplySnapshot(snapshot, now, visual) != MoveDisposition::RESET,
				"bounded catch-up converted ordinary movement into a teleport")) return false;
		}
		if (!Require(std::abs(snapshot.position.x - visual.position.x) < 0.2f,
			"the presentation speed bound left permanent lag behind a moving Server")) return false;
		snapshot.serverTick += 1u;
		snapshot.hasMoveGoal = false;
		(void)prediction.ApplySnapshot(snapshot, 6.225, visual);
		for (unsigned frame = 1u; frame <= 12u; ++frame)
			visual = prediction.Update(6.225 + frame / 40.0, 0.025f, {}).pose;
		if (!Require(NearMove(visual.position.x, snapshot.position.x) && !visual.isMoving,
			"bounded correction failed to settle at the stopped Server position")) return false;

		MovePrediction stalled;
		snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 2.95f;
		snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint.x = 100.f;
		(void)stalled.ApplySnapshot(snapshot, 0.0, {});
		visual = stalled.Update(0.025, 0.025f, {}).pose;
		const auto afterStall = stalled.Update(0.325, 0.3f, {}).pose;
		if (!Require(afterStall.position.x - visual.position.x <= snapshot.moveSpeed * 1.15f * 0.3f + 0.00001f,
			"a frame stall exceeded the total travel budget for its real elapsed time")) return false;
		snapshot.serverTick = 101u;
		snapshot.position.x = 20.f;
		return Require(stalled.ApplySnapshot(snapshot, 0.326, afterStall) == MoveDisposition::RESET &&
			NearMove(stalled.Update(0.326, 0.f, {}).pose.position.x, 20.f),
			"the presentation budget delayed a genuine authoritative teleport");
	}

	bool VerifySlowFramesRetainMoveConvergence()
	{
		for (const unsigned fps : { 5u, 8u })
		{
			MovePrediction prediction;
			auto snapshot = MakeMoveSnapshot(100u);
			snapshot.moveSpeed = 2.95f;
			(void)prediction.ApplySnapshot(snapshot, 0.0, {});
			(void)prediction.SubmitMove(1u, 0.025, {});
			(void)prediction.SubmitMove(2u, 0.05, {});
			snapshot.serverTick = 106u;
			snapshot.processedMoveSequence = 2u;
			snapshot.position.x = 0.59f;
			snapshot.hasMoveGoal = true;
			snapshot.nextWaypoint.x = 100.f;
			MovePrediction::Pose visual{};
			if (!Require(prediction.ApplySnapshot(snapshot, 0.2, visual) == MoveDisposition::RECONCILE,
				"slow-frame fixture did not preserve the delayed double-click ACK")) return false;
			(void)prediction.Update(0.2, 0.f, {});
			const float elapsed = 1.f / fps;
			for (unsigned frame = 1u; frame <= fps * 16u; ++frame)
			{
				const double now = 0.2 + static_cast<double>(frame) / fps;
				const auto next = prediction.Update(now, elapsed, {}).pose;
				if (!Require(next.position.x >= visual.position.x - 0.00001f &&
					next.position.x - visual.position.x <= snapshot.moveSpeed * 1.15f * elapsed + 0.00001f,
					"slow-frame catch-up exceeded its real elapsed-time movement budget")) return false;
				visual = next;
				// The product updates Character before the Level applies the latest snapshot.
				snapshot.serverTick = 100u + static_cast<unsigned>(std::floor(now * 30.0 + 0.000001));
				snapshot.position.x = (snapshot.serverTick - 100u) * snapshot.moveSpeed / 30.f;
				if (!Require(prediction.ApplySnapshot(snapshot, now, visual) == MoveDisposition::RECONCILE,
					"sustained 5/8 FPS accumulated ordinary movement into a hard reset")) return false;
				if (frame >= fps * 4u && !Require(std::abs(snapshot.position.x - visual.position.x) < 0.25f,
					"fresh slow-frame snapshots left increasing or permanent correction lag"))
				{ std::cerr << "slow fps=" << fps << " frame=" << frame << " server=" << snapshot.position.x << " visual=" << visual.position.x << "\n"; return false; }
			}
		}
		return true;
	}
	bool VerifySlowedMoveAndZeroSpeedCorrection()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 0.5f;
		snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint.x = 100.f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		snapshot.serverTick = 118u;
		snapshot.position.x = 0.3f;
		MovePrediction::Pose visual{};
		if (!Require(prediction.ApplySnapshot(snapshot, 0.6, visual) == MoveDisposition::RECONCILE,
			"slowed movement fixture did not preserve its delayed snapshot")) return false;
		(void)prediction.Update(0.6, 0.f, {});
		for (unsigned frame = 1u; frame <= 200u; ++frame)
		{
			const double now = 0.6 + frame / 40.0;
			const auto next = prediction.Update(now, 0.025f, {}).pose;
			if (!Require(next.position.x >= visual.position.x - 0.000001f &&
				next.position.x - visual.position.x <= 0.014375f + 0.000001f,
				"slowed movement correction exceeded 115 percent of the actual move speed")) return false;
			visual = next;
			snapshot.serverTick = 118u + static_cast<unsigned>(std::floor(frame * 30.0 / 40.0));
			snapshot.position.x = 0.3f + (snapshot.serverTick - 118u) * snapshot.moveSpeed / 30.f;
			if (!Require(prediction.ApplySnapshot(snapshot, now, visual) != MoveDisposition::RESET,
				"slowed correction accumulated ordinary movement into a reset")) return false;
		}
		if (!Require(std::abs(snapshot.position.x - visual.position.x) < 0.06f,
			"slowed correction failed to close the initial movement gap")) return false;
		snapshot.serverTick += 1u;
		snapshot.moveSpeed = 0.f;
		snapshot.hasMoveGoal = false;
		snapshot.position.x = visual.position.x - 0.3f;
		if (!Require(prediction.ApplySnapshot(snapshot, 5.625, visual) == MoveDisposition::RECONCILE,
			"zero-speed correction unexpectedly reset its valid stopped position")) return false;
		for (unsigned frame = 1u; frame <= 12u; ++frame)
			visual = prediction.Update(5.625 + frame / 40.0, 0.025f, {}).pose;
		return Require(NearMove(visual.position.x, snapshot.position.x) && !visual.isMoving,
			"zero-speed movement lost the fallback needed to settle its stopped position");
	}
	bool VerifyPresentationClockIgnoresPacketPhase()
	{
		struct Phase final { double receiveDelay; double updateJitter; };
		for (const unsigned fps : { 60u, 40u })
		{
			for (const Phase phase : { Phase{ 0.0, 0.0 }, { 0.008, 0.0 },
				{ 0.012, 0.0 }, { 0.015, 0.0 }, { 0.004, 0.006 } })
			{
				MovePrediction prediction;
				auto snapshot = MakeMoveSnapshot(100u);
				snapshot.moveSpeed = 2.95f;
				snapshot.nextWaypoint.x = 100.f;
				(void)prediction.ApplySnapshot(snapshot, 0.0, {});
				MovePrediction::Pose visual{};
				double firstSentAt = -1.0, secondSentAt = -1.0;
				unsigned firstMoveTick = 0u, receivedTick = 0u;
				const float frameSeconds = 1.f / fps;
				for (unsigned frame = 1u; frame <= fps * 10u; ++frame)
				{
					const double frameAt = static_cast<double>(frame) / fps;
					const double updateAt = frameAt + (frame % 2u ? phase.updateJitter : 0.0);
					const double receiveAt = updateAt + phase.receiveDelay;
					const auto previous = visual;
					auto result = prediction.Update(updateAt, frameSeconds, visual);
					if (result.useLocalPath)
					{
						visual.position.x += snapshot.moveSpeed * frameSeconds;
						visual.isMoving = true;
						result = prediction.CompleteLocalPathFrame(updateAt, visual);
					}
					visual = result.pose;
					const auto repeated = prediction.Update(updateAt, frameSeconds, visual);
					if (!Require(NearMove(repeated.pose.position.x, visual.position.x),
						"a repeated frame query advanced the presentation clock twice")) return false;
					const unsigned tick = receiveAt >= 0.025 ?
						static_cast<unsigned>(std::floor((receiveAt - 0.025) * 30.0 + 0.000001)) : 0u;
					if (tick > receivedTick)
					{
						receivedTick = tick;
						snapshot.serverTick = 100u + tick;
						snapshot.hasMoveGoal = firstMoveTick != 0u && tick >= firstMoveTick;
						snapshot.processedMoveSequence = snapshot.hasMoveGoal ? 1u : 0u;
						if (secondSentAt >= 0.0 && tick / 30.0 + 0.000001 >= secondSentAt + 0.025)
							snapshot.processedMoveSequence = 2u;
						snapshot.position.x = snapshot.hasMoveGoal ?
							(tick - firstMoveTick + 1u) * snapshot.moveSpeed / 30.f : 0.f;
						if (!Require(prediction.ApplySnapshot(snapshot, receiveAt, visual) != MoveDisposition::RESET,
							"normal frame/packet phase differences accumulated into an authoritative reset")) return false;
					}
					if (frame == fps || frame == static_cast<unsigned>(fps * 1.15))
					{
						const unsigned sequence = frame == fps ? 1u : 2u;
						if (!Require(prediction.SubmitMove(sequence, receiveAt, visual),
							"packet-phase fixture rejected a same-direction double click")) return false;
						if (sequence == 1u)
						{
							firstSentAt = receiveAt;
							firstMoveTick = static_cast<unsigned>(std::ceil((firstSentAt + 0.025) * 30.0 - 0.000001));
						}
						else secondSentAt = receiveAt;
					}
					if (frameAt >= 4.0)
					{
						const float step = visual.position.x - previous.position.x;
						if (!Require(step > 0.f && step <= snapshot.moveSpeed * frameSeconds * 1.15f + 0.00001f &&
							std::abs(snapshot.position.x - visual.position.x) < 0.25f,
							"packet or update phase caused stopped frames, excess movement, or increasing lag")) return false;
						if (fps == 60u && !Require(std::abs(step - snapshot.moveSpeed * frameSeconds) < 0.0001f,
							"steady 60 FPS movement alternated speed with the 30 Hz snapshot phase"))
						{
							std::cerr << "phase delay=" << phase.receiveDelay << " jitter=" << phase.updateJitter << " frame=" << frame << " step=" << step << "\n";
							return false;
						}
					}
				}
			}
		}
		return true;
	}
	bool VerifyContinuousReclickPresentation()
	{
		for (const unsigned fps : { 40u, 60u })
		for (const double phase : { 0.0, 0.008, 0.015 })
		for (const unsigned scenario : { 0u, 1u, 2u, 3u, 4u })
		{
			MovePrediction single, repeated;
			MovePrediction::Pose visual[2]{};
			MovePrediction::Vec3 goals[2]{ { 1000.f, 0.f, 0.f }, { 1000.f, 0.f, 0.f } };
			auto snapshot = MakeMoveSnapshot(100u);
			snapshot.moveSpeed = 2.95f;
			(void)single.ApplySnapshot(snapshot, 0.0, {});
			(void)repeated.ApplySnapshot(snapshot, 0.0, {});
			std::vector<double> sentTimes;
			unsigned receivedTick = 0u;
			double nextClick = 0.15;
			float minRatio = 100.f, maxRatio = 0.f, maxDifference = 0.f;
			for (unsigned frame = 1u; frame <= fps * 8u; ++frame)
			{
				const double frameAt = static_cast<double>(frame) / fps;
				const double updateAt = frameAt + (frame % 2u ? 0.001 : 0.0);
				const double receiveAt = updateAt + phase;
				const float dt = 1.f / fps;
				const auto beforeSingle = visual[0];
				for (unsigned i = 0u; i < 2u; ++i)
				{
					auto& prediction = i == 0u ? single : repeated;
					auto result = prediction.Update(updateAt, dt, visual[i]);
					if (result.useLocalPath)
					{
						const double dx = goals[i].x - visual[i].position.x;
						const double dz = goals[i].z - visual[i].position.z;
						const double length = std::sqrt(dx * dx + dz * dz);
						const double step = (std::min)(length, static_cast<double>(snapshot.moveSpeed *
							prediction.Get_LocalPathDeltaSeconds()));
						if (length > 0.00001)
						{
							visual[i].position.x += static_cast<float>(dx * step / length);
							visual[i].position.z += static_cast<float>(dz * step / length);
						}
						visual[i].isMoving = true;
						result = prediction.CompleteLocalPathFrame(updateAt, visual[i]);
					}
					visual[i] = result.pose;
					if (!Require(NearMove(prediction.Update(updateAt, dt, visual[i]).pose.position.x,
						visual[i].position.x), "reclick frame consumed its displacement twice")) return false;
				}
				const float difference = std::sqrt(
					std::pow(visual[0].position.x - visual[1].position.x, 2.f) +
					std::pow(visual[0].position.z - visual[1].position.z, 2.f));
				maxDifference = (std::max)(maxDifference, difference);
				if (!Require(difference < 0.0001f,
					"150 ms reclick changed a displayed frame against the same Server timeline"))
				{
					std::cerr << "fps=" << fps << " phase=" << phase << " scenario=" << scenario
						<< " frame=" << frame << " difference=" << difference << '\n';
					return false;
				}
				const float dx = visual[0].position.x - beforeSingle.position.x;
				const float dz = visual[0].position.z - beforeSingle.position.z;
				const float step = std::sqrt(dx * dx + dz * dz);
				if (!Require(step <= snapshot.moveSpeed * dt * 1.15f + 0.00002f,
					"continuous reclick exceeded the existing presentation speed budget")) return false;
				if (frameAt > 2.0 && frameAt < 2.5)
				{
					minRatio = (std::min)(minRatio, step / (snapshot.moveSpeed * dt));
					maxRatio = (std::max)(maxRatio, step / (snapshot.moveSpeed * dt));
				}
				// Change true delivery latency, add jitter and a 220 ms packet batch.
				const double delay = (scenario == 2u || scenario == 4u) ?
					(0.025 + (frameAt >= 3.0 && frameAt < 4.2 ? 0.20 : 0.0) +
						(frame % 3u == 0u ? 0.012 : 0.0)) : 0.025;
				const unsigned tick = receiveAt >= delay ?
					static_cast<unsigned>(std::floor((receiveAt - delay) * 30.0 + 0.000001)) : 0u;
				if (tick > receivedTick && !((scenario == 2u || scenario == 4u) && frameAt >= 5.0 && frameAt < 5.22))
				{
					receivedTick = tick;
					snapshot.serverTick = 100u + tick;
					const double serverAt = tick / 30.0;
					const double movingAt = (std::max)(0.0, (std::min)(6.0, serverAt) - 0.1);
					snapshot.hasMoveGoal = serverAt >= 0.1 && serverAt < 6.0;
					snapshot.position = { static_cast<float>(movingAt * snapshot.moveSpeed), 0.f, 0.f };
					snapshot.nextWaypoint = { 1000.f, 0.f, 0.f };
					if (scenario == 3u && serverAt >= 3.0)
					{
						// Same route turns 90 degrees, then body collision prevents progress
						// while the Server still owns a live goal.
						snapshot.position.x = 2.9f * snapshot.moveSpeed;
						snapshot.position.z = static_cast<float>((std::min)(1.0, serverAt - 3.0) * snapshot.moveSpeed);
						snapshot.nextWaypoint = { snapshot.position.x, 0.f, 1000.f };
					}
					unsigned ack = 0u;
					while (ack < sentTimes.size() && sentTimes[ack] + 0.025 <= serverAt + 0.000001) ++ack;
					for (unsigned i = 0u; i < 2u; ++i)
					{
						snapshot.processedMoveSequence = i == 0u ? (std::min)(1u, ack) : ack;
						const auto disposition = (i == 0u ? single : repeated).ApplySnapshot(snapshot, receiveAt, visual[i]);
						if (!Require(disposition != MoveDisposition::RESET,
							"normal continuous motion/stop/turn/latency change reset the display")) return false;
					}
				}
				if (frame == 1u || (frameAt >= nextClick && frameAt < 5.8))
				{
					if (frame != 1u) nextClick += 0.15;
					sentTimes.push_back(receiveAt);
					if (scenario == 1u || scenario == 4u) goals[1].x += 10.f;
					if (!Require(repeated.SubmitMove(static_cast<unsigned>(sentTimes.size()), receiveAt,
						visual[1], &goals[1]), "reclick fixture rejected a fresh command")) return false;
					if (frame == 1u && !Require(single.SubmitMove(1u, receiveAt, visual[0], &goals[0]),
						"single-click fixture rejected its command")) return false;
				}
			}
			if (!Require(!visual[0].isMoving && !visual[1].isMoving &&
				NearMove(visual[0].position.x, snapshot.position.x) &&
				NearMove(visual[0].position.z, snapshot.position.z),
				"continuous movement failed to settle at the authoritative final stop")) return false;
			if (!Require(minRatio >= 0.995f && maxRatio <= 1.005f,
				"steady single/reclick speed departed from nominal by more than 0.5 percent")) return false;
			std::cout << "reclick fps=" << fps << " phase=" << phase << " scenario=" << scenario
				<< " ratio=" << minRatio << ".." << maxRatio << " maxDifference=" << maxDifference << '\n';
		}
		return true;
	}
	bool VerifyRetargetCornerAndTickWrap()
	{
		MovePrediction prediction;
		auto snapshot = MakeMoveSnapshot(100u);
		snapshot.moveSpeed = 2.95f;
		snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint.x = 100.f;
		(void)prediction.ApplySnapshot(snapshot, 0.0, {});
		auto visual = prediction.Update(0.025, 0.025f, {}).pose;
		snapshot.serverTick = 101u;
		snapshot.position.x = 0.04f;
		(void)prediction.ApplySnapshot(snapshot, 0.03, visual);
		const MovePrediction::Vec3 turnGoal{ visual.position.x, 0.f, 100.f };
		if (!Require(prediction.SubmitMove(1u, 0.031, visual, &turnGoal),
			"new 90-degree target failed to begin prediction")) return false;
		for (unsigned frame = 0u; frame < 2u; ++frame)
		{
			const double now = 0.05 + frame * 0.025;
			const auto before = visual;
			const auto begin = prediction.Update(now, 0.025f, visual);
			if (!Require(begin.useLocalPath, "old ACK retired the latest actual turn")) return false;
			visual.position.z += snapshot.moveSpeed * prediction.Get_LocalPathDeltaSeconds();
			visual.yawDegrees = 0.f;
			visual = prediction.CompleteLocalPathFrame(now, visual).pose;
			if (!Require(NearMove(visual.position.x, before.position.x) &&
				visual.position.z > before.position.z,
				"an old X-segment residual pulled a new Z turn diagonally")) return false;
			snapshot.serverTick += 1u;
			snapshot.position.x += 0.05f;
			if (!Require(prediction.ApplySnapshot(snapshot, now + 0.001, visual) ==
				MoveDisposition::PRESERVE_LOCAL_PATH,
				"unacknowledged 90-degree turn lost its bounded local path")) return false;
		}
		snapshot.serverTick += 1u;
		snapshot.processedMoveSequence = 1u;
		snapshot.position = visual.position;
		snapshot.nextWaypoint = turnGoal;
		if (!Require(prediction.ApplySnapshot(snapshot, 0.08, visual) == MoveDisposition::RECONCILE &&
			!prediction.UsesLocalPath() && prediction.Update(0.1, 0.025f, {}).pose.position.z > visual.position.z,
			"actual turn ACK failed to restore authoritative continuous motion")) return false;

		MovePrediction corner;
		snapshot = MakeMoveSnapshot(100u);
		snapshot.hasMoveGoal = true;
		snapshot.nextWaypoint = { 0.05f, 0.f, 0.f };
		(void)corner.ApplySnapshot(snapshot, 0.0, {});
		visual = corner.Update(0.003, 0.003f, {}).pose;
		snapshot.serverTick = 101u;
		snapshot.position = { 0.05f, 0.f, 0.05f };
		snapshot.nextWaypoint = { 0.05f, 0.f, 10.f };
		(void)corner.ApplySnapshot(snapshot, 0.034, visual);
		// The new ACK can arrive before any local path frame observes the turn.
		// Retire the old corner from Server segment evidence in that ordering too.
		auto fastAckCorner = corner;
		const MovePrediction::Vec3 fastGoal{ -10.f, 0.f, 0.f };
		(void)fastAckCorner.SubmitMove(1u, 0.035, visual, &fastGoal);
		auto fastSnapshot = snapshot;
		fastSnapshot.serverTick = 102u;
		fastSnapshot.processedMoveSequence = 1u;
		fastSnapshot.position = { 0.02f, 0.f, 0.f };
		fastSnapshot.nextWaypoint = fastGoal;
		(void)fastAckCorner.ApplySnapshot(fastSnapshot, 0.036, visual);
		if (!Require(fastAckCorner.Update(0.04, 0.003f, visual).pose.position.x < visual.position.x,
			"an ACK before the first local turn frame retained an obsolete Server corner")) return false;
		auto redirectedCorner = corner;
		const MovePrediction::Vec3 away{ -10.f, 0.f, 0.f };
		(void)redirectedCorner.SubmitMove(1u, 0.035, visual, &away);
		(void)redirectedCorner.Update(0.05, 0.016f, visual);
		auto awayPose = visual;
		awayPose.position.x -= 0.096f;
		awayPose = redirectedCorner.CompleteLocalPathFrame(0.05, awayPose).pose;
		auto redirectedSnapshot = snapshot;
		redirectedSnapshot.serverTick = 102u;
		redirectedSnapshot.processedMoveSequence = 1u;
		redirectedSnapshot.position = awayPose.position;
		redirectedSnapshot.nextWaypoint = away;
		(void)redirectedCorner.ApplySnapshot(redirectedSnapshot, 0.051, awayPose);
		const auto continuedAway = redirectedCorner.Update(0.066, 0.016f, awayPose).pose;
		if (!Require(continuedAway.position.x < awayPose.position.x &&
			std::abs(continuedAway.position.z) < 0.02f,
			"a cached Server corner pulled a newly acknowledged opposite target back toward the old route")) return false;
		visual = corner.Update(0.05, 0.016f, visual).pose;
		if (!Require(NearMove(visual.position.x, 0.05f) && visual.position.z > 0.f,
			"display movement or its residual cut inside the known Server corner")) return false;

		// GameRoom skips reserved tick zero. A wrap must not add a fictitious tick
		// to the receive clock, relative to an otherwise identical ordinary trace.
		MovePrediction normal, wrapped;
		auto ordinary = MakeMoveSnapshot(100u);
		ordinary.hasMoveGoal = true; ordinary.nextWaypoint.x = 100.f;
		auto wrap = ordinary; wrap.serverTick = 0xfffffffdu;
		(void)normal.ApplySnapshot(ordinary, 0.0, {});
		(void)wrapped.ApplySnapshot(wrap, 0.0, {});
		MovePrediction::Pose normalPose{}, wrapPose{};
		for (unsigned frame = 1u; frame <= 120u; ++frame)
		{
			const double now = frame / 60.0;
			normalPose = normal.Update(now, 1.f / 60.f, normalPose).pose;
			wrapPose = wrapped.Update(now, 1.f / 60.f, wrapPose).pose;
			if (!Require(NearMove(normalPose.position.x, wrapPose.position.x),
				"skipped-zero Server tick wrap changed the continuous movement clock")) return false;
			if (frame % 2u != 0u) continue;
			++ordinary.serverTick;
			if (++wrap.serverTick == 0u) ++wrap.serverTick;
			ordinary.position.x = wrap.position.x = frame * 0.1f;
			(void)normal.ApplySnapshot(ordinary, now + 0.001, normalPose);
			(void)wrapped.ApplySnapshot(wrap, now + 0.001, wrapPose);
		}
		return true;
	}
	struct GroundProof final
	{
		unsigned calls = 0u;
		bool valid = false;
		MovePrediction::Vec3 from{};
		MovePrediction::Vec3 to{};
	};

	bool ValidateGroundStep(const MovePrediction::Vec3& from,
		const MovePrediction::Vec3& to, void* context)
	{
		auto& proof = *static_cast<GroundProof*>(context);
		++proof.calls;
		proof.from = from;
		proof.to = to;
		return proof.valid;
	}

	bool VerifyNavigationProvenGroundContinuity()
	{
		for (const bool proofValid : { false, true })
		{
			MovePrediction prediction;
			auto snapshot = MakeMoveSnapshot(100u);
			snapshot.moveSpeed = 2.95f;
			(void)prediction.ApplySnapshot(snapshot, 0.0, {});
			snapshot.serverTick = 101u;
			snapshot.position = { 0.09f, 1.25f, 0.f };
			GroundProof proof{ 0u, proofValid };
			const auto disposition = prediction.ApplySnapshot(snapshot, 1.0 / 30.0, {},
				ValidateGroundStep, &proof);
			if (!Require(proof.calls == 1u && NearMove(proof.from.x, 0.f) &&
				NearMove(proof.to.y, 1.25f) && disposition ==
					(proofValid ? MoveDisposition::RECONCILE : MoveDisposition::RESET),
				"vertical ground continuity ignored the navigation proof or used another segment")) return false;
		}
		MovePrediction prediction;
		auto initial = MakeMoveSnapshot(100u, 2u);
		initial.moveSpeed = 2.95f;
		(void)prediction.ApplySnapshot(initial, 0.0, {});
		auto step = initial;
		step.serverTick = 101u;
		step.position = { 0.09f, 1.25f, 0.f };
		if (!Require(prediction.ApplySnapshot(step, 1.0 / 30.0, {}) == MoveDisposition::RESET,
			"an unproven vertical discontinuity lost its authoritative reset")) return false;
		prediction.Reset();
		GroundProof proof{ 0u, true };
		(void)prediction.ApplySnapshot(initial, 0.0, {}, ValidateGroundStep, &proof);
		step.position.x = 2.f;
		if (!Require(prediction.ApplySnapshot(step, 1.0 / 30.0, {}, ValidateGroundStep, &proof) ==
			MoveDisposition::RESET && proof.calls == 0u,
			"a navigation callback exempted a genuine horizontal discontinuity")) return false;
		prediction.Reset();
		(void)prediction.ApplySnapshot(initial, 0.0, {});
		step.position.x = 0.09f;
		step.serverTick = 100u;
		if (!Require(prediction.ApplySnapshot(step, 1.0 / 30.0, {}, ValidateGroundStep, &proof) ==
			MoveDisposition::IGNORED && proof.calls == 0u,
			"a duplicate Server tick reached the ground continuity callback")) return false;
		step.serverTick = 101u;
		step.processedMoveSequence = 1u;
		if (!Require(prediction.ApplySnapshot(step, 1.0 / 30.0, {}, ValidateGroundStep, &proof) ==
			MoveDisposition::IGNORED && proof.calls == 0u,
			"a regressing move acknowledgement reached the ground continuity callback")) return false;
		step.processedMoveSequence = 2u;
		const MovePrediction::Pose farVisual{ { 20.f, 0.f, 0.f }, 0.f, false };
		return Require(prediction.ApplySnapshot(step, 1.0 / 30.0, farVisual,
			ValidateGroundStep, &proof) == MoveDisposition::RESET,
			"a navigation proof bypassed the large visual/server disagreement reset");
	}
	bool VerifyMovePredictionTimeout()
	{
		MovePrediction prediction;
		MovePrediction::Pose visual{};
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, visual);
		(void)prediction.SubmitMove(1u, 0.01, visual);
		visual = { { 0.4f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.1, 0.09f, visual);
		(void)prediction.CompleteLocalPathFrame(0.1, visual);
		const auto disconnected = prediction.Update(0.36, 0.26f,
			{ { 100.f, 0.f, 0.f }, 90.f, true });
		const auto stillDisconnected = prediction.Update(10.0, 9.64f, {});
		if (!Require(disconnected.stopLocalPath && !prediction.HasPendingMove() &&
			!disconnected.pose.isMoving && NearMove(disconnected.pose.position.x, 0.4f) &&
			NearMove(stillDisconnected.pose.position.x, 0.4f) &&
			!prediction.Can_SubmitMove(10.0),
			"missing snapshots allowed unbounded local travel or stale-input prediction"))
		{
			return false;
		}

		prediction.Reset();
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(1u), 0.0, {});
		(void)prediction.SubmitMove(1u, 0.01, {});
		(void)prediction.Update(0.1, 0.09f, visual);
		(void)prediction.CompleteLocalPathFrame(0.1, visual);
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(8u), 0.25, visual);
		const auto missingAck = prediction.Update(0.37, 0.12f, visual);
		const auto corrected = prediction.Update(0.46, 0.09f, {});
		if (!Require(missingAck.stopLocalPath && !prediction.HasPendingMove() &&
			!corrected.pose.isMoving && NearMove(corrected.pose.position.x, 0.f),
			"fresh snapshots without a move acknowledgement kept the input alive forever"))
		{
			return false;
		}

		prediction.Reset();
		auto moving = MakeMoveSnapshot(1u);
		moving.hasMoveGoal = true;
		moving.nextWaypoint.x = 10.f;
		(void)prediction.ApplySnapshot(moving, 0.0, {});
		const auto lastMoving = prediction.Update(0.2, 0.2f, {});
		const auto stopped = prediction.Update(0.36, 0.16f, {});
		return Require(NearMove(lastMoving.pose.position.x, 0.9f) &&
			NearMove(stopped.pose.position.x, lastMoving.pose.position.x) &&
			!stopped.pose.isMoving,
			"acknowledged movement extrapolated indefinitely after snapshot loss");
	}
}

int Run_ClientPresentationPrimitiveContractTests()
{
	bool failed = false;
	failed |= !VerifyKoukuVisibleCooldownSlots();
	failed |= !VerifyMousePressOwnership();
	failed |= !VerifyIndependentLoopedSound();
	failed |= !VerifyReplicatedPartyHealth();
	failed |= !VerifyPartyTransferNotice();
	failed |= !VerifyContinuousReclickPresentation();
	failed |= !VerifyRetargetCornerAndTickWrap();
	failed |= !VerifyImmediateMovePrediction();
	failed |= !VerifyAcknowledgedMoveCorrection();
	failed |= !VerifyMoveSequenceOrdering();
	failed |= !VerifyMoveRejectionAndForcedState();
	failed |= !VerifyMovePredictionTimeout();
	failed |= !VerifyMoveCorrectionSpeedAndContinuousSnapshots();
	failed |= !VerifyLatestOppositeHeadingAndGroundHeight();
	failed |= !VerifyAcknowledgedBearingHasOneSmoothingOwner();
	failed |= !VerifyDelayedSnapshotsPreserveNormalMotion();
	failed |= !VerifyTotalMovePresentationBudget();
	failed |= !VerifySlowFramesRetainMoveConvergence();
	failed |= !VerifySlowedMoveAndZeroSpeedCorrection();
	failed |= !VerifyPresentationClockIgnoresPacketPhase();
	failed |= !VerifyNavigationProvenGroundContinuity();
	if (failed)
	{
		return 1;
	}
	std::cout << "Client presentation primitive contracts: PASS\n";
	return 0;
}
