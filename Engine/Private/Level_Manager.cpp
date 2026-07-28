#include "Level_Manager.h"


#include "GameInstance.h"

CLevel_Manager::CLevel_Manager()
{
}

CLevel_Manager::~CLevel_Manager()
{
}

HRESULT CLevel_Manager::Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel)
{
	if (nullptr != m_pCurrentLevel)
	{
		if (FAILED(CGameInstance::Get().Clear_Resources(m_iCurrentLevelID)))
			return E_FAIL;
	}

	/* 기존에 할당되어있던 레벨을 삭제한다.*/
	Safe_Reset(m_pCurrentLevel);	

	/* 새로운 레벨로 교체한다. */
	m_pCurrentLevel = std::move(pNewLevel);

	m_iCurrentLevelID = iCurrentLevelID;

	return S_OK;
}

void CLevel_Manager::Update(f32_t fTimeDelta)
{
	if (nullptr == m_pCurrentLevel)
		return;

	m_pCurrentLevel->Update(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
	if (nullptr == m_pCurrentLevel)
		return E_FAIL;

	return m_pCurrentLevel->Render();
}

unique_ptr<CLevel_Manager> CLevel_Manager::Create()
{
	return unique_ptr<CLevel_Manager>(new CLevel_Manager());
}
