// 활성 Level의 단독 소유권 교체와 프레임 전달을 구현한다.
#include "Level_Manager.h"

// 이전 Level 슬롯의 객체·프로토타입·물리 자원을 Clear_Resources로 순서대로 정리한다.
#include "GameInstance.h"

CLevel_Manager::CLevel_Manager()
{
}

CLevel_Manager::~CLevel_Manager()
{
}

// CGameInstance가 이미 생성한 새 Level을 넘긴다. 현재 Level의 Update 도중 직접 호출하지 않는 것은
// Client 전환 서비스와 MainApp의 책임이며, 이 함수에는 지연 요청 큐가 없다.
// Clear_Resources 실패 시 이전 Level 포인터는 남지만 이미 정리된 자원까지 복구하지는 않는다.
// nullptr 새 Level을 자체적으로 거부하지 않는 현재 계약도 유지한다.
HRESULT CLevel_Manager::Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel)
{
	// 기존 Level이 있을 때만 그 슬롯의 자원을 정리한다. 중간 실패는 이전 객체/ID를 남기고 새 인자 객체를 해제한다.
	if (nullptr != m_pCurrentLevel)
	{
		if (FAILED(CGameInstance::Get().Clear_Resources(m_iCurrentLevelID)))
			return E_FAIL;
	}

	/* 기존에 할당되어있던 레벨을 삭제한다.*/
	// 자원 정리가 성공한 뒤 이전 Level 객체의 소유권을 해제한다.
	Safe_Reset(m_pCurrentLevel);	

	/* 새로운 레벨로 교체한다. */
	// 새 객체와 자원 슬롯 ID를 같은 성공 경로에서 기록한다.
	m_pCurrentLevel = std::move(pNewLevel);

	m_iCurrentLevelID = iCurrentLevelID;

	return S_OK;
}

// 활성 객체가 없는 동안은 아무 작업도 하지 않는다. Level 내부 상태는 해당 구체 객체가 관리한다.
void CLevel_Manager::Update(f32_t fTimeDelta)
{
	if (nullptr == m_pCurrentLevel)
		return;

	m_pCurrentLevel->Update(fTimeDelta);
}

// 렌더 결과를 CGameInstance에 그대로 전달한다. 활성 Level 부재는 실패로 구분한다.
HRESULT CLevel_Manager::Render()
{
	if (nullptr == m_pCurrentLevel)
		return E_FAIL;

	return m_pCurrentLevel->Render();
}

// CGameInstance가 소유할 관리자를 만든다. 이 시점에는 활성 Level이 없다.
unique_ptr<CLevel_Manager> CLevel_Manager::Create()
{
	return unique_ptr<CLevel_Manager>(new CLevel_Manager());
}
