#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>

//local 입력을 gameplay command로 바꾸는 client 입력 경계
//level, character, networkmanager 사이에서 다음 흐름만 조정
//우클릭 -> 월드 목표 계산 -> Send_MoveGoal
namespace Client
{
	class CCharacter;
	class IPlayerCommandSink;
	struct CHARACTER_SPEC;

	class CPlayerController final
	{
	public:
		void Set_LocalCharacter(
			const shared_ptr<CCharacter>& character);
		void Rebind_LocalCharacter(
			const shared_ptr<CCharacter>& character);
		void Set_CommandSink(
			const shared_ptr<IPlayerCommandSink>& commandSink);
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
		/* A held basic attack has to keep asking: the combo buffer only takes a
		press inside a window the server owns and the client is not told about, so
		the press is repeated at a fixed rate instead of being predicted. */
		bool_t m_wasLeftMouseDown = false;
		std::chrono::steady_clock::time_point m_LastBasicAttackSentAt{};
		bool_t m_allowCapturedKeyboardInput = false;
		/* Esther edges are tracked apart from m_wasKeyDown: the quick-slot
		table commits Z and X every frame, and a Ctrl press must not read as
		fresh just because the plain-slot pass was ruled out that frame. */
		std::array<bool_t, 3> m_wasEstherKeyDown{};
	};
}
