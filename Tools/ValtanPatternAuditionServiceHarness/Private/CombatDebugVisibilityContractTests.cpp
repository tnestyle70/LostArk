#include "CombatDebugVisibility.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
	void Require(const bool_t bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	void VerifySafeDefaults()
	{
		const Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT Visibility{};
		Require(!Visibility.bBossBodyCollider,
			"boss body collider must remain opt-in");
		Require(Visibility.bBossPatternHitPulse &&
			Visibility.bBossStageGeometry && Visibility.bCombatObjectHit &&
			Visibility.bCounterProxy &&
			Visibility.bPlayerSkillHitGeometry,
			"existing combat diagnostics or restored Stage geometry defaulted off");
		Require(0u == Visibility.iRevision,
			"an uncommitted visibility value claimed a process revision");
	}

	void VerifySixIndependentVisibilityFlags()
	{
		using SNAPSHOT = Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT;
		using FLAG = bool_t SNAPSHOT::*;
		constexpr std::array<FLAG, 6u> Flags{
			&SNAPSHOT::bBossBodyCollider,
			&SNAPSHOT::bBossPatternHitPulse,
			&SNAPSHOT::bBossStageGeometry,
			&SNAPSHOT::bCombatObjectHit,
			&SNAPSHOT::bCounterProxy,
			&SNAPSHOT::bPlayerSkillHitGeometry };
		const SNAPSHOT Baseline{};
		for (const FLAG Flag : Flags)
		{
			SNAPSHOT Changed = Baseline;
			Changed.*Flag = !(Changed.*Flag);
			Require(!Baseline.Has_SameVisibility(Changed),
				"one diagnostic flag was not an independent revision input");
			Changed.*Flag = Baseline.*Flag;
			Require(Baseline.Has_SameVisibility(Changed),
				"restoring one diagnostic flag did not restore equality");
		}
		SNAPSHOT NewRevision = Baseline;
		NewRevision.iRevision = 91u;
		Require(Baseline.Has_SameVisibility(NewRevision),
			"revision metadata changed the five visibility values");
	}

	void VerifyRevisionSequenceNeverPublishesZero()
	{
		using SNAPSHOT = Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT;
		Require(1u == SNAPSHOT::Next_Revision(0u) &&
			42u == SNAPSHOT::Next_Revision(41u),
			"normal visibility revision did not advance exactly once");
		Require(1u == SNAPSHOT::Next_Revision(
			(std::numeric_limits<std::uint64_t>::max)()),
			"visibility revision overflow published the reserved zero value");
	}

	void VerifyCombatObjectHitClock()
	{
		using CLOCK = Client::COMBAT_OBJECT_HIT_DEBUG_CLOCK;
		Require(CLOCK::Is_Visible(
			true, 400u, 100u, 0u, 1u, 0u),
			"live CONTACT geometry disappeared before object despawn");
		Require(!CLOCK::Is_Visible(
			false, 147u, 100u, 1600u, 1u, 0u) &&
			CLOCK::Is_Visible(
				false, 148u, 100u, 1600u, 1u, 0u) &&
			CLOCK::Is_Visible(
				false, 157u, 100u, 1600u, 1u, 0u) &&
			!CLOCK::Is_Visible(
				false, 158u, 100u, 1600u, 1u, 0u),
			"TIMED geometry did not own its exact 300ms Server-tick pulse window");
		Require(!CLOCK::Is_Visible(
			false, 99u, 100u, 0u, 1u, 0u),
			"an older snapshot was presented as current combat geometry");
	}
}

int Run_CombatDebugVisibilityContractTests()
{
	try
	{
		VerifySafeDefaults();
		VerifySixIndependentVisibilityFlags();
		VerifyRevisionSequenceNeverPublishesZero();
		VerifyCombatObjectHitClock();
		std::cout << "CombatDebugVisibilityContractTests: 4/4 passed\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "CombatDebugVisibilityContractTests: FAIL: " <<
			Error.what() << '\n';
		return 1;
	}
}
