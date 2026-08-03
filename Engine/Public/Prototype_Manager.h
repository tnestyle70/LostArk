#pragma once

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Collider.h"
#include "Transform.h"
#include "Navigation.h"
#include "VIBuffer_Rect.h"

/* 원형객체를 레벨별로 모아서 보관한다. */

NS_BEGIN(Engine)

class CPrototype_Manager final
{
private:
	CPrototype_Manager();
public:
	~CPrototype_Manager();

public:
	HRESULT Initialize(uint32_t iNumLevels);	
	HRESULT Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, 
		unique_ptr<class CPrototype> pPrototype);
	shared_ptr<CPrototype> Clone_Prototype(uint32_t iLevelIndex, 
		const wstring_t& strPrototypeTag, void* pArg);
	HRESULT Clear(uint32_t iClearLevelID);

private:
	uint32_t			m_iNumLevels = {};


private:

	unique_ptr<map<const wstring_t, unique_ptr<class CPrototype>>[]>	m_pPrototypes = { nullptr };
	typedef map<const wstring_t, unique_ptr<class CPrototype>>			PROTOTYPES;

	// map<const wstring_t, unique_ptr<class CPrototype>>		m_Prototypes[LEVEL::END];

private:
	class CPrototype* Find_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag);


public:
	static unique_ptr<CPrototype_Manager> Create(uint32_t iNumLevels);


};

NS_END