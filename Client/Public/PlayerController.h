#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>

//local 입력을 gameplay command로 바꾸는 client 입력 경계
//level, character, networkmanager 사이에서 다음 흐름만 조정
//우클릭 -> 월드 목표 계산 -> Send_MoveGoal
namespace Client
{
	class CCharacter;
	class IPlayerCommandSink;
	class CSkillGroundTargetPreview;
	struct CHARACTER_SPEC;

	/* Explicit movement/skill commands take ownership at the next server action
	boundary.  A still-held LMB must not immediately auto-resend BA and steal that
	ownership back; only a physical release rearms it.  Kept independent from
	DirectInput so the edge contract is executable in the frontend harness. */
	class CBASIC_ATTACK_RESEND_GATE final
	{
	public:
		constexpr void Reset() { m_suppressUntilRelease = false; }
		constexpr void Suppress_UntilRelease() { m_suppressUntilRelease = true; }
		constexpr bool_t Observe_Button(const bool_t isDown)
		{
			if (!isDown)
				m_suppressUntilRelease = false;
			return isDown && m_suppressUntilRelease;
		}

	private:
		bool_t m_suppressUntilRelease = false;
	};

	/* One physical LMB press submits at most one successful basic-attack command.
	A press that begins while gameplay input is blocked belongs to the blocker and
	must not turn into a delayed attack when capture clears. An eligible press is
	only consumed after the command sink accepts it, so a transient send failure
	can retry while the same physical press remains held. */
	class CBASIC_ATTACK_PRESS_EDGE_GATE final
	{
	public:
		constexpr void Reset()
		{
			m_isCurrentPressActive = false;
			m_isCurrentPressConsumed = false;
		}

		constexpr bool_t Should_Submit(
			const bool_t isPhysicallyDown,
			const bool_t isCommandEligible)
		{
			if (!isPhysicallyDown)
			{
				Reset();
				return false;
			}

			if (!m_isCurrentPressActive)
			{
				m_isCurrentPressActive = true;
				m_isCurrentPressConsumed = !isCommandEligible;
			}
			else if (!isCommandEligible)
			{
				m_isCurrentPressConsumed = true;
			}

			return isCommandEligible && !m_isCurrentPressConsumed;
		}

		constexpr void Commit_Submission()
		{
			if (m_isCurrentPressActive)
				m_isCurrentPressConsumed = true;
		}

	private:
		bool_t m_isCurrentPressActive = false;
		bool_t m_isCurrentPressConsumed = false;
	};

	/* The request callback is the narrow fake-sink boundary used by the compile-time
	contract below and by CPlayerController at runtime. Suppression is therefore
	impossible unless the actual command sink accepted the ground-target cast. */
	template <typename TRequest>
	constexpr bool_t Try_Commit_GroundTargetConfirmation(
		CBASIC_ATTACK_RESEND_GATE& basicAttackGate,
		const TRequest& request)
	{
		if (!request())
			return false;
		basicAttackGate.Suppress_UntilRelease();
		return true;
	}

	namespace PlayerInputGateContract
	{
		struct CONFIRM_REQUEST_STUB final
		{
			bool_t accepted = false;

			constexpr bool_t operator()() const { return accepted; }
		};

		constexpr bool_t Validate_BasicAttackPressTransitions()
		{
			CBASIC_ATTACK_PRESS_EDGE_GATE gate{};

			/* A press owned by UI/capture remains consumed until raw release. */
			if (gate.Should_Submit(true, false) || gate.Should_Submit(true, true))
				return false;
			if (gate.Should_Submit(false, false))
				return false;

			/* A failed sink call does not commit, so the same held press retries. */
			if (!gate.Should_Submit(true, true) ||
				!gate.Should_Submit(true, true))
			{
				return false;
			}

			/* Acceptance consumes the press; release alone rearms the next one. */
			gate.Commit_Submission();
			if (gate.Should_Submit(true, true) ||
				gate.Should_Submit(false, false) ||
				!gate.Should_Submit(true, true))
			{
				return false;
			}
			return true;
		}

		constexpr bool_t Validate_GroundTargetConfirmTransaction()
		{
			CBASIC_ATTACK_RESEND_GATE gate{};
			if (gate.Observe_Button(true))
				return false;

			/* Rejection keeps BA available; acceptance owns LMB through release. */
			if (Try_Commit_GroundTargetConfirmation(
					gate, CONFIRM_REQUEST_STUB{ false }) ||
				gate.Observe_Button(true))
			{
				return false;
			}
			if (!Try_Commit_GroundTargetConfirmation(
					gate, CONFIRM_REQUEST_STUB{ true }) ||
				!gate.Observe_Button(true))
			{
				return false;
			}
			return !gate.Observe_Button(false) && !gate.Observe_Button(true);
		}

		static_assert(Validate_BasicAttackPressTransitions(),
			"basic-attack press edge transaction regressed");
		static_assert(Validate_GroundTargetConfirmTransaction(),
			"ground-target confirm LMB ownership regressed");
	}

	/* Pure state contract for a two-step ground-target skill. DirectInput,
	 networking, navigation and rendering remain outside, which lets the focused
	 frontend harness pin clamp/cancel/confirm behavior without launching Client. */
	class CGROUND_TARGETING_STATE final
	{
	public:
		bool_t Begin(
			LostArk::Shared::SKILL_ID skillId,
			f32_t maximumRange)
		{
			if (LostArk::Shared::INVALID_SKILL_ID == skillId ||
				!std::isfinite(maximumRange) || maximumRange <= 0.f)
			{
				return false;
			}
			m_iSkillId = skillId;
			m_fMaximumRange = maximumRange;
			m_TargetPosition = {};
			m_hasCursor = false;
			m_isWalkable = false;
			m_isActive = true;
			return true;
		}

		void Cancel()
		{
			*this = {};
		}

		bool_t Update_Cursor(
			const float3_t& casterPosition,
			const float3_t& rawCursorPosition)
		{
			if (!m_isActive || !std::isfinite(casterPosition.x) ||
				!std::isfinite(casterPosition.y) ||
				!std::isfinite(casterPosition.z) ||
				!std::isfinite(rawCursorPosition.x) ||
				!std::isfinite(rawCursorPosition.z))
			{
				if (m_isActive)
				{
					m_hasCursor = false;
					m_isWalkable = false;
				}
				return false;
			}
			float deltaX = rawCursorPosition.x - casterPosition.x;
			float deltaZ = rawCursorPosition.z - casterPosition.z;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			const float rangeSquared = m_fMaximumRange * m_fMaximumRange;
			if (distanceSquared > rangeSquared && distanceSquared > 0.f)
			{
				const float scale = m_fMaximumRange /
					std::sqrt(distanceSquared);
				deltaX *= scale;
				deltaZ *= scale;
			}
			m_TargetPosition = {
				casterPosition.x + deltaX,
				casterPosition.y,
				casterPosition.z + deltaZ };
			m_hasCursor = true;
			m_isWalkable = false;
			return true;
		}

		bool_t Apply_WalkableSample(const float3_t& sampledPosition)
		{
			if (!m_isActive || !m_hasCursor ||
				!std::isfinite(sampledPosition.x) ||
				!std::isfinite(sampledPosition.y) ||
				!std::isfinite(sampledPosition.z) ||
				std::abs(sampledPosition.x - m_TargetPosition.x) > 0.001f ||
				std::abs(sampledPosition.z - m_TargetPosition.z) > 0.001f)
			{
				m_isWalkable = false;
				return false;
			}
			m_TargetPosition = sampledPosition;
			m_isWalkable = true;
			return true;
		}

		void Invalidate_Cursor(const float3_t& casterPosition)
		{
			if (!m_isActive)
				return;
			m_TargetPosition = casterPosition;
			m_hasCursor = false;
			m_isWalkable = false;
		}

		bool_t Is_Active() const { return m_isActive; }
		bool_t Can_Confirm() const
		{
			return m_isActive && m_hasCursor && m_isWalkable;
		}
		LostArk::Shared::SKILL_ID Get_SkillId() const { return m_iSkillId; }
		f32_t Get_MaximumRange() const { return m_fMaximumRange; }
		const float3_t& Get_TargetPosition() const { return m_TargetPosition; }

	private:
		LostArk::Shared::SKILL_ID m_iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		f32_t m_fMaximumRange = 0.f;
		float3_t m_TargetPosition{};
		bool_t m_hasCursor = false;
		bool_t m_isWalkable = false;
		bool_t m_isActive = false;
	};

	class CPlayerController final
	{
	public:
		void Set_LocalCharacter(
			const shared_ptr<CCharacter>& character);
		void Rebind_LocalCharacter(
			const shared_ptr<CCharacter>& character);
		void Set_CommandSink(
			const shared_ptr<IPlayerCommandSink>& commandSink);
		bool_t Initialize_TargetingPreview(uint32_t levelIndex);
		void Set_AllowCapturedKeyboardInput(bool_t allow)
		{
			m_allowCapturedKeyboardInput = allow;
		}

		void Update(bool_t gameplayCommandsEnabled);

		/* Cursor -> world ray unprojected against the groundY plane. Public+static
		because it touches no member state (screen cursor, viewport and view/proj
		come straight from CGameInstance) -- Level_Bern reuses this same math to
		test a right-click against a stationary NPC's position instead of
		duplicating the unproject/plane-intersect code a second time. */
		static bool_t Try_PickGroundPlane(
			f32_t groundY,
			float3_t& outPosition);

	private:
		//실질적인 navigation picking을 통한 이동으로 교체
		bool_t Should_SendMoveGoal(
			bool_t wasRightMouseDown,
			f32_t characterX,
			f32_t characterZ,
			const float3_t& goal) const;

		/* Walks the quick slots once and reports the first newly pressed one that
		the character's class has a skill bound to. Only writes outSkillId when it
		is still INVALID_SKILL_ID, so one frame submits at most one skill. */
		void Poll_SkillSlots(
			bool_t isKeyboardBlocked,
			bool_t useRawKeyboard,
			const shared_ptr<CCharacter>& character,
			LostArk::Shared::SKILL_ID& outSkillId,
			LostArk::Shared::SKILL_ID& outReleaseSkillId);

		/* Left mouse is not a key code, so it is polled after the slot table and
		only fills a slot the keyboard left empty. */
		void Poll_BasicAttack(
			const CHARACTER_SPEC* pSpec,
			LostArk::Shared::PLAYER_STANCE_ID stance,
			LostArk::Shared::SKILL_ID& outSkillId,
			bool_t commandSuppressed);

		/* Ctrl+Z/X/C are the raid Esther slots (1..3). Returns 0 when no slot
		was newly pressed this frame. Ctrl frames never reach the quick-slot
		table, so Warlord's plain Z/X stance keys stay unaffected. */
		std::uint8_t Poll_EstherSlot(
			bool_t isKeyboardBlocked,
			bool_t useRawKeyboard);
		void Cancel_GroundTargeting();

	private:
		weak_ptr<CCharacter> m_pLocalCharacter;
		shared_ptr<IPlayerCommandSink> m_pCommandSink;

		std::uint32_t m_iNextMoveSequence = 1;
		std::uint32_t m_iNextActionSequence = 1;
		bool_t m_wasRightMouseDown = false;
		std::chrono::steady_clock::time_point m_LastMoveGoalSentAt{};
		float3_t m_LastSentMoveGoal{};
		/* Edge state indexed by DirectInput key code, not by binding position: a
		slot that is later re-pointed at another skill must not make a key that is
		already held read as a fresh press. */
		std::array<bool_t, 256> m_wasKeyDown{};
		LostArk::Shared::SKILL_ID m_iHeldSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		uint8_t m_byHeldKeyCode = 0;
		std::chrono::steady_clock::time_point m_LastSkillAimSentAt{};
		float3_t m_LastSentSkillAim{};
		/* The Server owns the combo stage/window. The Client submits exactly one
		command for each physical LMB press and never predicts a continuation from
		a held button. */
		LostArk::Shared::SKILL_ID m_iHeldBasicAttackSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		CBASIC_ATTACK_PRESS_EDGE_GATE m_BasicAttackPressEdgeGate;
		CBASIC_ATTACK_RESEND_GATE m_BasicAttackResendGate;
		bool_t m_allowCapturedKeyboardInput = false;
		/* Esther edges are tracked apart from m_wasKeyDown: the quick-slot
		table commits Z and X every frame, and a Ctrl press must not read as
		fresh just because the plain-slot pass was ruled out that frame. */
		std::array<bool_t, 3> m_wasEstherKeyDown{};
		CGROUND_TARGETING_STATE m_GroundTargeting;
		shared_ptr<CSkillGroundTargetPreview> m_pGroundTargetPreview;
		bool_t m_wasTargetingLeftMouseDown = false;
		bool_t m_wasTargetingRightMouseDown = false;
	};
}
