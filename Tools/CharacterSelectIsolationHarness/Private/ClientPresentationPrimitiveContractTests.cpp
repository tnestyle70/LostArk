#include "ClientReplicationEvent.h"
#include "MouseButtonReleaseGate.h"
#include "PartyTransferNotice.h"
#include "ReplicatedPlayerHealth.h"
#include "Sound/TrackedSoundChannel.h"

#include <iostream>

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
}

int Run_ClientPresentationPrimitiveContractTests()
{
	if (!VerifyMousePressOwnership() || !VerifyIndependentLoopedSound() ||
		!VerifyReplicatedPartyHealth() || !VerifyPartyTransferNotice())
	{
		return 1;
	}
	std::cout << "Client presentation primitive contracts: PASS\n";
	return 0;
}
