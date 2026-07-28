#pragma once

#include "Level.h"

/* 1. 현재 할당된 레벨을 보관한다. */
/* 1_1. 보관하고 있는 레벨의 반복적인 Update, Render를 호출한다. */

/* 2. 교체된 이전 레벨을 삭제한다. */
/* 2_1. 기존 레벨을 위해 할당됐던 다양한 리소스들을 삭제한다. */

NS_BEGIN(Engine)

class CLevel_Manager final
{
private:
	CLevel_Manager();
public:
	~CLevel_Manager();

public:
	HRESULT Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel);
	void Update(f32_t fTimeDelta);
	HRESULT Render();

private:
	uint32_t							m_iCurrentLevelID = { };
	unique_ptr<class CLevel>			m_pCurrentLevel = { nullptr };

public:
	static unique_ptr<CLevel_Manager> Create();

};

NS_END