#pragma once

// 칭호 ID와 무효 값 등 룸/패킷 소비자가 공유하는 형식에 맞춰 허용 목록을 저장한다.
#include "Network/PacketMessages.h"

// 로드 결과 진단 문장을 catalog가 소유하고 호출자에게 읽기 참조로 제공한다.
#include <string>
// 게시된 칭호 ID의 중복을 검사하고 장착·복원·전환 시 membership을 조회한다.
#include <unordered_set>

namespace LostArk::Server
{
	// Tools/GameplayPipeline/Publish-HonorTitles.ps1이 Data/Titles/HonorTitles.json에서 만든
	// HonorTitles/HonorTitles.bootstrap의 허용 ID 집합이다. 표시 이름과 설명은 Client catalog의 책임이다.
	// 칭호는 외형 상태이며 업적 획득 이력 대신 게시 목록 포함 여부로 장착을 허용한다.
	// GameRoom이 값으로 소유하고 초기화 때 로드한 뒤 룸 worker에서 조회한다. 자체 잠금은 없다.
	class CHonorTitleCatalog final
	{
	  public:
		// GameRoom 초기화가 호출한다. header/행/중복을 임시 집합에서 검사한 뒤 성공 시에만 허용 목록을 교체한다.
		// false 경로는 이전 목록을 유지하고 실패 이유만 Get_Status에 남긴다.
		bool Load();
		// 장착 요청·캐릭터 복원·월드 전환 입장이 게시 목록 포함 여부를 읽는다.
		// INVALID_HONOR_TITLE_ID는 파일에서 거부되므로 이 조회도 false이며 칭호 해제 허용은 호출자가 별도로 판정한다.
		bool Has_Title(LostArk::Shared::HONOR_TITLE_ID titleId) const;
		// 최근 Load 진단의 비소유 참조다. catalog 수명을 넘겨 보관하지 않으며 다음 Load가 내용을 바꿀 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	  private:
		// 성공한 로드만 교체하는 칭호 ID 정본이며 이름/업적/플레이어 장착 상태는 보관하지 않는다.
		std::unordered_set<LostArk::Shared::HONOR_TITLE_ID> m_Titles;
		std::string m_strStatus;
	};
}
