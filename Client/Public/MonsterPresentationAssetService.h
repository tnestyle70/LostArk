#pragma once

// Client 이름 공간과 유효 Level 범위를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr·HRESULT·원형 tag 문자열 타입을 사용한다.
#include "Engine_Defines.h"

// 조합한 모델 prototype tag를 값으로 반환한다.
#include <string>
// 호출자의 archetype ID를 호출 동안만 차용한다.
#include <string_view>

NS_BEGIN(Client)

// 일반 몬스터의 catalog를 검증해 Level의 Prototype 저장소에 표현 자원을 등록한다.
// 정적 준비 기록만 보유하며 실제 원형의 수명은 Engine 저장소가 관리한다.
class CMonsterPresentationAssetService final
{
public:
	// Loader가 재로딩을 시작할 때 해당 Level의 준비 기록만 비운다. 원형을 직접 제거하지 않는다.
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	// 필요 자원을 만들어 등록한다. 같은 ID가 준비되어 있으면 S_FALSE, 새 등록 성공은 S_OK, 입력/준비 실패는 실패 HRESULT다.
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId);
	// archetype ID의 바이트를 wide 문자로 확장해 tag에 붙인다. UTF-8 문장 변환 함수는 아니다.
	static std::wstring Get_ModelPrototypeTag(std::string_view archetypeId);
	// 모든 일반 몬스터 표현이 공유하는 CNpc 원형 tag의 정적 문자열을 반환한다.
	static const wchar_t* Get_GameObjectPrototypeTag();
};

NS_END
