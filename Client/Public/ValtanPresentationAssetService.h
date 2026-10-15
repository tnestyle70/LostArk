#pragma once

// Client 이름 공간과 유효 Level 범위를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr·HRESULT·원형 tag 문자열 타입을 사용한다.
#include "Engine_Defines.h"

// 호출자가 넘긴 archetype ID를 호출 동안만 차용한다.
#include <string_view>

NS_BEGIN(Client)

// 발탄 계열 보스의 catalog를 검증해 Level의 Prototype 저장소에 표현 자원을 등록한다.
// 정적 준비 기록만 보유하며 실제 원형의 수명은 Engine 저장소가 관리한다.
class CValtanPresentationAssetService final
{
public:
	// Loader가 재로딩을 시작할 때 해당 Level의 준비 기록만 비운다. 원형을 직접 제거하지 않는다.
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	// 필요 자원을 만들어 등록한다. 같은 ID가 준비되어 있으면 S_FALSE, 새 등록 성공은 S_OK, 입력/준비 실패는 실패 HRESULT다.
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId = "BOSS_VALTAN");
	// 해당 Level/archetype의 준비 기록을 mutex 안에서 조회한다.
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		std::string_view archetypeId = "BOSS_VALTAN");
	// 기본 발탄은 기존 tag, 호환 파생 보스는 ID 접미사를 쓴다. 미지원 정의는 빈 문자열이다.
	static wstring_t Get_BodyModelPrototypeTag(std::string_view archetypeId);
	static wstring_t Get_WeaponModelPrototypeTag(std::string_view archetypeId);
};

NS_END
