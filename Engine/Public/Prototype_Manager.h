#pragma once

// 기존 공개 include 묶음으로 모델 원형 타입을 함께 노출한다. 관리자 자체의 저장/복제는 구체 타입 대신 CPrototype 경계를 쓴다.
#include "Model.h"
// 셰이더 원형 타입을 노출한다. GameInstance를 포함하는 소비 코드에도 이 기존 include 경계가 이어진다.
#include "Shader.h"
// 텍스처 원형의 공개 타입을 같은 경계로 제공한다. 관리자 안에서 텍스처를 직접 로드하지는 않는다.
#include "Texture.h"
// 충돌체 원형의 타입을 제공한다. 실제 형상 준비와 복제는 해당 Component의 구현에 남긴다.
#include "Collider.h"
// Transform과 초기화 descriptor의 기존 공개 경계다. GameObject의 기본 Transform은 등록 원형 복제와 별도로 생성한다.
#include "Transform.h"
// Navigation Component의 공개 타입을 제공한다. 이동 판정은 이 원형 저장소가 수행하지 않는다.
#include "Navigation.h"
// 사각형 정점 버퍼 원형을 제공한다. 이 묶음에서 얻는 공통 기반/컨테이너 선언도 기존 포함 순서를 유지한다.
#include "VIBuffer_Rect.h"

/* 원형객체를 레벨별로 모아서 보관한다. */
// Level별 tag로 생성 원형을 소유한다. 활성화된 GameObject의 갱신 목록과는 별개다.

NS_BEGIN(Engine)

class CPrototype_Manager final
{
private:
	CPrototype_Manager();
public:
	~CPrototype_Manager();

public:
	// CGameInstance 초기화가 Level 수를 전달한다. 이미 준비된 저장소는 다시 초기화하지 않는다.
	HRESULT Initialize(uint32_t iNumLevels);	
	// CGameInstance가 전달한 원형의 unique_ptr 소유권을 받는다. 같은 Level의 중복 tag는 거부한다.
	HRESULT Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, 
		unique_ptr<class CPrototype> pPrototype);
	// 묶음의 tag를 전부 검사한 뒤 원형을 임시 map으로 옮겨 등록한다.
	// rvalue 참조는 진입만으로 입력을 이동하지 않는다. 초기 검증 실패는 입력을 보존하지만 이동 단계의 예외는 이미 옮긴 항목을 복구하지 않는다.
	HRESULT Add_Prototypes(
		uint32_t iLevelIndex,
		vector<pair<wstring_t, unique_ptr<class CPrototype>>>&& prototypes);
	// 원형을 빌려 Clone을 호출한다. 등록 원형의 소유권은 이 관리자에 남는다.
	shared_ptr<CPrototype> Clone_Prototype(uint32_t iLevelIndex, 
		const wstring_t& strPrototypeTag, void* pArg);
	// 해당 Level의 원형을 해제한다. 이미 반환한 복제 객체를 직접 삭제하는 함수는 아니다.
	HRESULT Clear(uint32_t iClearLevelID);

private:
	uint32_t			m_iNumLevels = {};


private:

	// 배열 index는 Level, map key는 그 Level 안의 prototype tag다.
	// 내부 동기화가 없으므로 호출자가 등록·조회·해제의 동시 접근을 조율해야 한다.
	unique_ptr<map<const wstring_t, unique_ptr<class CPrototype>>[]>	m_pPrototypes = { nullptr };
	typedef map<const wstring_t, unique_ptr<class CPrototype>>			PROTOTYPES;



private:
	// 반환 포인터는 조회용 차용이다. 호출자가 유효한 Level과 준비된 저장소를 먼저 확인한다.
	class CPrototype* Find_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag);


public:
	// CGameInstance가 Level별 빈 저장소를 준비한다. 성공한 관리자만 소유 반환하며 new의 할당 예외는 HRESULT로 변환하지 않는다.
	static unique_ptr<CPrototype_Manager> Create(uint32_t iNumLevels);


};

NS_END
