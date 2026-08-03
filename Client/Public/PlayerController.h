#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <memory>

//local 입력을 gameplay command로 바꾸는 client 입력 경계
//level, character, networkmanager 사이에서 다음 흐름만 조정
//우클릭 -> 월드 목표 계산 -> Send_MoveGoal
namespace Client
{
	class CCharacter;
	class IPlayerCommandSink;

	class CPlayerController final
	{
	public:
		void Set_LocalCharacter(
			const shared_ptr<CCharacter>& character);
		void Set_CommandSink(
			const shared_ptr<IPlayerCommandSink>& commandSink);

		void Update();

	private:
		//실질적인 navigation picking을 통한 이동으로 교체
		bool_t Try_PickGroundPlane(
			f32_t groundY,
			float3_t& outPosition) const;

	private:
		weak_ptr<CCharacter> m_pLocalCharacter;
		shared_ptr<IPlayerCommandSink> m_pCommandSink;

		std::uint32_t m_iNextMoveSequence = 1;
		std::uint32_t m_iNextActionSequence = 1;
		bool_t m_wasRightMouseDown = false;
		bool_t m_wasQDown = false;
		bool_t m_wasWDown = false;
	};
}
