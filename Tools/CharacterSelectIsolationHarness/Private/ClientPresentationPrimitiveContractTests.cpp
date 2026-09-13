#include "ClientReplicationEvent.h"
#include "LocalMovePrediction.h"
#include "MouseButtonReleaseGate.h"
#include "PartyTransferNotice.h"
#include "ReplicatedPlayerHealth.h"
#include "Sound/TrackedSoundChannel.h"

#include <iostream>
#include <limits>

namespace
{
	bool Require(const bool condition, const char* message)
	{
		if (!condition)
			std::cerr << "ClientPresentationPrimitiveContracts: " << message << '\n';
		return condition;
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
		second.iNetEntityId = 202u;
		second.iCurrentHp = 0u;
		second.iMaximumHp = 200u;
		snapshot.Players = { second, first };
		if (!Require(!health.Find(101u).hasSnapshot && health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f &&
			health.Find(202u).hasSnapshot && health.Find(202u).Get_Ratio() == 0.f &&
			!health.Find(999u).hasSnapshot,
			"HP join fabricated data, used row order, or hid zero HP"))
		{
			return false;
		}
		snapshot.Players[1].iCurrentHp = 90u;
		if (!Require(health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f,
			"duplicate tick replaced current HP"))
		{
			return false;
		}
		snapshot.iServerTick = 9u;
		if (!Require(health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f,
			"older tick replaced current HP"))
		{
			return false;
		}
		snapshot.iServerTick = 11u;
		snapshot.Players.push_back(first);
		if (!Require(!health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f,
			"duplicate entity partially committed HP"))
		{
			return false;
		}
		snapshot.Players.pop_back();
		snapshot.Players[1].iMaximumHp = 0u;
		if (!Require(!health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f,
			"invalid HP partially committed the snapshot"))
		{
			return false;
		}
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
			!Require(NearMove(reconciled.pose.position.x, 0.89f),
				"acknowledged movement failed to advance from its authoritative snapshot") ||
			!Require(NearMove(capped.pose.position.x, 1.1f),
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
		return Require(NearMove(latencyCapped.pose.position.x, 0.94f) &&
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
		if (!Require(prediction.SubmitMove(2u, 0.03, visual),
			"a quick second click could not replace the first prediction"))
		{
			return false;
		}
		visual = { { 0.08f, 0.f, 0.04f }, -90.f, true };
		(void)prediction.Update(0.04, 0.01f, visual);
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

	bool VerifyMovePredictionTimeout()
	{
		MovePrediction prediction;
		MovePrediction::Pose visual{};
		(void)prediction.ApplySnapshot(MakeMoveSnapshot(10u), 0.0, visual);
		(void)prediction.SubmitMove(1u, 0.01, visual);
		visual = { { 0.4f, 0.f, 0.f }, 90.f, true };
		(void)prediction.Update(0.1, 0.09f, visual);
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
	if (!VerifyMousePressOwnership() || !VerifyIndependentLoopedSound() ||
		!VerifyReplicatedPartyHealth() || !VerifyPartyTransferNotice() ||
		!VerifyImmediateMovePrediction() || !VerifyAcknowledgedMoveCorrection() ||
		!VerifyMoveSequenceOrdering() || !VerifyMoveRejectionAndForcedState() ||
		!VerifyMovePredictionTimeout() || !VerifyMoveCorrectionSpeedAndContinuousSnapshots() ||
		!VerifyLatestOppositeHeadingAndGroundHeight() || !VerifyAcknowledgedBearingHasOneSmoothingOwner())
	{
		return 1;
	}
	std::cout << "Client presentation primitive contracts: PASS\n";
	return 0;
}
