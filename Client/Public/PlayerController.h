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
		void Set_CommandSink(
			const shared_ptr<IPlayerCommandSink>& commandSink);
		void Set_GameplayInputEnabled(bool_t isEnabled)
		{
			m_isGameplayInputEnabled = isEnabled;
		}

		void Update();

	private:
		//실질적인 navigation picking을 통한 이동으로 교체
		bool_t Try_PickGroundPlane(
			f32_t groundY,
			float3_t& outPosition) const;

		/* Walks the quick slots once and reports the first newly pressed one that
		the character's class has a skill bound to. Only writes outSkillId when it
		is still INVALID_SKILL_ID, so one frame submits at most one skill. */
		void Poll_SkillSlots(
			bool_t isKeyboardBlocked,
			const shared_ptr<CCharacter>& character,
			LostArk::Shared::SKILL_ID& outSkillId);

		/* Left mouse is not a key code, so it is polled after the slot table and
		only fills a slot the keyboard left empty. */
		void Poll_BasicAttack(
			const CHARACTER_SPEC* pSpec,
			LostArk::Shared::SKILL_ID& outSkillId);

	private:
		weak_ptr<CCharacter> m_pLocalCharacter;
		shared_ptr<IPlayerCommandSink> m_pCommandSink;

		std::uint32_t m_iNextMoveSequence = 1;
		std::uint32_t m_iNextActionSequence = 1;
		bool_t m_wasRightMouseDown = false;
		/* Edge state indexed by DirectInput key code, not by binding position: a
		slot that is later re-pointed at another skill must not make a key that is
		already held read as a fresh press. */
		std::array<bool_t, 256> m_wasKeyDown{};
		bool_t m_isGameplayInputEnabled = true;
		/* A held basic attack has to keep asking: the combo buffer only takes a
		press inside a window the server owns and the client is not told about, so
		the press is repeated at a fixed rate instead of being predicted. */
		bool_t m_wasLeftMouseDown = false;
		std::chrono::steady_clock::time_point m_LastBasicAttackSentAt{};
	};
}
