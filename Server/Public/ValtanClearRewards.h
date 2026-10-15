#pragma once

#include "Network/PacketType.h"

#include <array>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// GameRoom이 발탄 클리어 때 읽을 직업별 보상 item ID 목록이다. Data/Valtan/Valtan.clearrewards.json의 게시 결과를 사용한다.
	// 실제 reader 입력은 DataFiles/Valtan/ClearRewards.bootstrap이며 이 클래스는 JSON을 직접 읽거나 아이템을 지급하지 않는다.
	// 목록 순서는 지급 순서로 유지한다. 주 발탄의 DEAD/중복 지급 방지와 실제 지급 성공 여부는 GameRoom이 담당한다.
	class CValtanClearRewards final
	{
	public:
		// GameRoom 초기화에서 호출한다. false 반환이면 이전 직업별 목록을 복원하고 상태 문자열에는 실패 이유를 남긴다.
		// item ID 형식만 검증하며 ItemCatalog에 실제 존재하는지 또는 지급 가능한지까지 여기서 확인하지 않는다.
		bool Load();

		// GameRoom_WorldEntitySimulation.cpp의 Grant_ValtanRaidClearRewards가 목록을 차용한다. 잘못된 직업은 정적인 빈 목록을 반환한다.
		// 유효 직업 목록의 참조는 이 객체가 소유하며 성공한 재로드/파괴 뒤에는 재사용하지 않는다.
		const std::vector<std::string>& Get_ItemIds(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;

		// 최근 로드 결과의 차용 참조다. 다음 Load가 내용을 갱신할 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	private:
		// 알려진 직업 enum을 배열 index로 사용하고 각 vector가 item ID 문자열을 소유한다. 현재 player의 인벤토리는 저장하지 않는다.
		using CLASS_ITEM_IDS = std::array<std::vector<std::string>,
			static_cast<std::size_t>(LostArk::Shared::CHARACTER_CLASS_ID::END)>;
		CLASS_ITEM_IDS m_ItemIds;
		std::string m_strStatus;
	};
}
