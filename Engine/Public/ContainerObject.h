#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CContainerObject abstract : public CGameObject
{
public:
	typedef struct tagContainerObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}CONTAINEROBJECT_DESC;

protected:
	CContainerObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CContainerObject();
public:
	shared_ptr<CComponent> Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	virtual HRESULT Render();

protected:
	using PART_OBJECT_MAP =
		map<const wstring_t, shared_ptr<class CPartObject>>;
	PART_OBJECT_MAP			m_PartObjects;

protected:
	HRESULT Add_PartObject(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strPartObjectTag, void* pArg = nullptr);
	HRESULT Clone_PartObject(
		uint32_t iPrototypeLevelIndex,
		const wstring_t& strPrototypeTag,
		void* pArg,
		shared_ptr<class CPartObject>& pOutPartObject);
	/* Builds the complete next map before swapping it in. Candidate tags must all
	share the prefix; an empty candidate map removes the group. */
	HRESULT Replace_PartObjectGroup(
		const wstring_t& strPartObjectTagPrefix,
		PART_OBJECT_MAP&& CandidatePartObjects);
	class CPartObject* Find_PartObject(const wstring_t& strPartObjectTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;


};

NS_END
