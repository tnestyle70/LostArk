#pragma once

// 등록 가능한 LEVEL 값과 Client 공통 타입을 사용한다.
#include "Client_Defines.h"
// HRESULT와 그래픽 장치 ComPtr 등 생성·준비 함수의 공통 타입을 사용한다.
#include "Engine_Defines.h"
// Loader와 실제 배치가 함께 사용할 Area 로드 범위를 값으로 보관한다.
#include "MapLoadScope.h"

// 생성한 Level의 단독 소유권을 unique_ptr로 호출자에게 전달한다.
#include <memory>

NS_BEGIN(Engine)
// 헤더에서는 생성 함수의 반환 소유권만 선언하므로 Level 구현 전체를 포함하지 않는다.
class CLevel;
NS_END

NS_BEGIN(Client)

// 준비 함수는 Loader를 참조로 차용한다. 실제 멤버 호출은 CPP에서 정의한다.
class CLoader;

// 제품 경로와 개발용 경로를 descriptor에서 구분한다. Level enum과 별개의 분류다.
enum class CLIENT_LEVEL_KIND
{
	PRODUCT,
	DEVELOPMENT
};

// 레벨 생성·자원 준비·맵 범위·렌더링 프로필을 연결하는 읽기 전용 등록 정보다.
// 문자열 포인터와 함수 포인터는 CPP의 정적 등록 테이블이 제공하며 호출자가 해제하지 않는다.
struct CLIENT_LEVEL_DESCRIPTOR final
{
	using CREATE_FUNCTION = unique_ptr<Engine::CLevel>(*)(
		ComPtr<ID3D11Device>,
		ComPtr<ID3D11DeviceContext>);
	// MSVC는 전방 선언한 CLoader의 멤버 함수 포인터 표현을 include 순서에 따라 달리 정할 수 있다.
	// 일반 함수 포인터를 사용하여 공개 descriptor의 메모리 배치를 고정한다.
	using LOAD_FUNCTION = HRESULT (*)(CLoader&);

	// eLevel은 런타임 선택값, pStableId는 고정 식별자다. Area와 프로필이 없는 경로는 nullptr을 사용할 수 있다.
	LEVEL eLevel = LEVEL::END;
	CLIENT_LEVEL_KIND eKind = CLIENT_LEVEL_KIND::PRODUCT;
	const char_t* pStableId = nullptr;
	const char_t* pMapAreaId = nullptr;
	const char_t* pRenderingProfileId = nullptr;
	MAP_LOAD_SCOPE MapLoadScope{};
	CREATE_FUNCTION pCreate = nullptr;
	LOAD_FUNCTION pLoad = nullptr;
	// 같은 Level에서 추가로 보여 줄 선택적 Area다. Loader와 런타임 배치가 같은 범위를 사용한다.
	const char_t* pPresentationMapAreaId = nullptr;
	MAP_LOAD_SCOPE PresentationMapLoadScope{};
};

// MainApp의 Level 생성과 Loader의 준비 경로가 같은 정적 descriptor를 소비하도록 연결한다.
class CLevelRegistry final
{
public:
	// 등록된 Level만 반환한다. 포인터는 정적 테이블을 차용하며 미등록이면 nullptr이다.
	static const CLIENT_LEVEL_DESCRIPTOR* Find(LEVEL eLevel);
	// MainApp 활성화에서 호출한다. 생성 실패는 nullptr이며 현재 Level 교체는 호출자가 결정한다.
	static unique_ptr<Engine::CLevel> Create_Level(
		LEVEL eLevel,
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// Loader 작업이 해당 Level의 준비 함수를 실행한다. 미등록 또는 함수 누락은 E_INVALIDARG다.
	static HRESULT Execute_Load(
		LEVEL eLevel,
		CLoader& loader);
};

NS_END
