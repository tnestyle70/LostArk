#pragma once

// Level별로 소유할 Layer와 객체/Component 조회·단계별 갱신 계약을 사용한다.
#include "Layer.h"

/* 사본객체들을 레벨별로 그룹(CLayer)지어서 보관한다. */
/* 보관하고 있는객체들의 반복적인 업데이트 호출을 담당한다. */
// Level 슬롯마다 layer tag로 CLayer를 소유한다. CLayer가 복제 GameObject의 shared_ptr를 보유한다.
// CGameInstance의 프레임 순서를 각 Layer에 전달하며, 원형 저장과 복제 구현은 Prototype_Manager에 맡긴다.
// 저장소 자체에는 잠금이 없다. 호출자는 객체 순회와 등록/제거가 충돌하지 않게 해야 한다.

NS_BEGIN(Engine)

class CObject_Manager final
{
private:
	CObject_Manager();
public:
	~CObject_Manager();

public:
	// iIndex는 현재 Layer 목록의 순서다. 영구 ID가 아니며 삭제 이후 같은 객체를 가리킨다는 보장이 없다.
	shared_ptr<CGameObject> Get_GameObject(uint32_t iLevelIndex, const wstring_t& strLayerTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		const wstring_t& strComponentTag,
		uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		const wstring_t& strPartTag,
		const wstring_t& strComponentTag,
		uint32_t iIndex);


public:
	// Create가 Level 수만큼 빈 map을 준비한다. 이미 준비된 배열은 E_FAIL로 보존하며 0개 Level을 별도 거부하지 않는다.
	HRESULT Initialize(uint32_t iNumLevels);
	// 복제와 Layer 등록이 성공한 뒤에만 선택적 출력 포인터를 채운다. 실패 시 기존 출력값은 유지한다.
	HRESULT Add_GameObject_to_Layer(
		uint32_t iPrototypeLevelIndex,
		const wstring_t& strPrototypeTag,
		uint32_t iLayerLevelIndex,
		const wstring_t& strLayerTag,
		void* pArg,
		shared_ptr<CGameObject>* pOutGameObject = nullptr);
	// 기존 Layer에 제거를 위임한다. 잘못된 Level·없는 Layer·미등록 객체는 실패이며 다른 Layer를 탐색하지 않는다.
	HRESULT Remove_GameObject_from_Layer(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		const shared_ptr<CGameObject>& pGameObject);
	// GameInstance의 프레임 순서를 모든 등록 Level 슬롯에 전달한다. 현재 활성 Level 하나만 순회하는 방식은 아니다.
	void Priority_Update(f32_t fTimeDelta);
	void Update(f32_t fTimeDelta);
	void Post_Physics_Update(f32_t fTimeDelta);
	void Late_Update(f32_t fTimeDelta);
	// 카메라가 확정된 뒤 Layer별 CPU 준비와 최종 렌더 제출을 요청한다.
	void Submit_FinalCamera();
	// 지정 Level의 Layer 소유 참조만 해제한다. 외부 shared_ptr가 있으면 객체 수명은 더 길 수 있다.
	HRESULT Clear(uint32_t iClearLevelID);



private:
	uint32_t			m_iNumLevels = {};

	// 배열 인덱스는 Level, 각 map의 key는 Layer 이름이다. Find_Layer의 raw pointer는 이 저장소를 빌린다.
	unique_ptr<map<const wstring_t, shared_ptr<CLayer>>[]>		m_pLayers = { nullptr };
	typedef map<const wstring_t, shared_ptr<CLayer>>			LAYERS;

private:
	// 범위와 저장소를 확인하고 map 소유 Layer의 주소만 빌린다. 조회가 새 Layer를 만들지는 않는다.
	class CLayer* Find_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag);



public:
	// GameInstance의 수명에 속할 관리자를 준비한다. Level별 map과 Layer/객체 소유 참조는 이 관리자 종료 때 정리된다.
	static unique_ptr<CObject_Manager> Create(uint32_t iNumLevels);

};

NS_END
