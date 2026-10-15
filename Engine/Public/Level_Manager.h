#pragma once

// 단독 소유하는 CLevel의 완전한 타입과 공통 Update/Render 계약을 사용한다.
#include "Level.h"

/* 1. 현재 할당된 레벨을 보관한다. */
/* 1_1. 보관하고 있는 레벨의 반복적인 Update, Render를 호출한다. */
/* 2. 교체된 이전 레벨을 삭제한다. */
/* 2_1. 기존 레벨을 위해 할당됐던 다양한 리소스들을 삭제한다. */
// 현재 Level 하나를 unique_ptr로 소유하고 갱신과 렌더를 전달한다.
// 교체 요청은 Client 전환 서비스와 MainApp을 거쳐 CGameInstance가 전달한다.
// Level 소유권 교체와 Level별 자원 정리는 여기서 연결하되, 콘텐츠 로드는 호출자가 끝낸다.

NS_BEGIN(Engine)

class CLevel_Manager final
{
private:
	CLevel_Manager();
public:
	~CLevel_Manager();

public:
	// 마지막으로 교체한 자원 슬롯 ID를 반환한다. 생성 직후의 0만으로 활성 객체 존재를 판단하지 않는다.
	uint32_t Get_CurrentLevelID() const
	{
		return m_iCurrentLevelID;
	}
	// 인자로 받은 새 Level의 소유권은 함수 진입 때 넘어온다. 자원 정리에 실패하면 새 Level은 해제된다.
	HRESULT Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel);
	// CGameInstance의 프레임 처리에서 현재 Level로 경과 초를 전달한다. 활성 객체가 없으면 아무 작업도 하지 않는다.
	void Update(f32_t fTimeDelta);
	// 현재 Level의 결과를 전달하며 활성 객체가 없으면 E_FAIL이다. 교체·갱신·그리기의 동시 호출은 보호하지 않는다.
	HRESULT Render();

private:
	// ID는 Level별 자원 저장소의 슬롯을 가리킨다. Level 포인터와 함께 성공 경로에서 교체한다.
	uint32_t							m_iCurrentLevelID = { };
	unique_ptr<class CLevel>			m_pCurrentLevel = { nullptr };

public:
	// CGameInstance가 소유할 관리자를 만든다. Level의 실제 콘텐츠 준비와 진입 판정은 Client가 담당한다.
	static unique_ptr<CLevel_Manager> Create();

};

NS_END
