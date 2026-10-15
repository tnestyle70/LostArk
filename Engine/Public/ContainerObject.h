#pragma once

// 부모 객체의 Transform/Component와 Prototype 기반을 상속하고 자식 Part들을 추가로 소유한다.
#include "GameObject.h"

NS_BEGIN(Engine)

// 캐릭터/보스처럼 여러 PartObject를 소유하는 GameObject의 기반이다. Part의 갱신은 이 부모가 전달한다.
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
	// part tag로 자식을 찾은 뒤 그 자식의 Component map을 조회한다.
	shared_ptr<CComponent> Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag);

public:
	// 기반 Container에는 공유 자원 준비가 없어 S_OK다. 구체 캐릭터/보스의 원형 준비는 파생 구현이 담당한다.
	virtual HRESULT Initialize_Prototype() override;
	// GameObject의 Transform 준비를 위임한다. 이 단계에서 Part를 자동 생성하거나 이전 Part 목록을 지우지는 않는다.
	virtual HRESULT Initialize(void* pArg) override;
	// 이 세 단계만 자식 map 순서로 전달한다. Part phase mask를 검사하거나 Post_Physics_Update를 추가 전달하는 구현은 없다.
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	// 기반 Render는 자식을 직접 순회해 그리지 않고 S_OK다. 실제 draw 제출은 구체 Part/Container가 담당한다.
	virtual HRESULT Render();

protected:
	using PART_OBJECT_MAP =
		map<const wstring_t, shared_ptr<class CPartObject>>;
	// key는 부모 안의 part 이름이며 값은 소유 참조다. Layer 목록에 별도 등록하는 계약과 구분한다.
	// 각 값은 유효 Part여야 한다. 갱신 중 map 교체를 막는 잠금은 없으므로 소유 스레드가 편집/순회 시점을 구분한다.
	PART_OBJECT_MAP			m_PartObjects;

protected:
	// 중복 이름을 거부하고 PartObject 타입으로 복제한 뒤 등록한다.
	HRESULT Add_PartObject(
		uint32_t iPrototypeLevelIndex,
		const wstring_t& strPrototypeTag,
		const wstring_t& strPartObjectTag,
		void* pArg = nullptr);
	// 복제만 수행하며 아직 map에 등록하지 않는다. 실패 여부와 무관하게 호출 시작 때 pOutPartObject를 비운다.
	HRESULT Clone_PartObject(
		uint32_t iPrototypeLevelIndex,
		const wstring_t& strPrototypeTag,
		void* pArg,
		shared_ptr<class CPartObject>& pOutPartObject);
	// 같은 접두사 그룹의 다음 map을 모두 준비한 뒤 swap한다. 빈 후보는 해당 그룹 제거를 뜻한다.
	// 실패하면 기존 map은 유지하지만, 이미 move한 후보 항목까지 되돌려 주지는 않는다.
	HRESULT Replace_PartObjectGroup(
		const wstring_t& strPartObjectTagPrefix,
		PART_OBJECT_MAP&& CandidatePartObjects);
	// m_PartObjects가 수명을 보장하는 동안만 유효한 비소유 포인터다.
	class CPartObject* Find_PartObject(const wstring_t& strPartObjectTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;


};

NS_END
