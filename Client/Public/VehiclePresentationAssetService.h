#pragma once

// Client 이름 공간과 유효 Level 범위를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr·HRESULT·원형 tag 문자열 타입을 사용한다.
#include "Engine_Defines.h"

// catalog vehicle ID의 고정 크기 정수 타입을 사용한다.
#include <cstdint>
// 모델 tag와 마지막 준비 상태 문구를 제공한다.
#include <string>

NS_BEGIN(Client)

// 탈것의 catalog를 검증해 Level의 Prototype 저장소에 표현 자원을 등록한다.
// 정적 준비 기록만 보유하며 실제 원형의 수명은 Engine 저장소가 관리한다.
class CVehiclePresentationAssetService final
{
public:
	// Loader가 재로딩을 시작할 때 해당 Level의 준비 기록만 비운다. 원형을 직접 제거하지 않는다.
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	// 필요 자원을 만들어 등록한다. 같은 ID가 준비되어 있으면 S_FALSE, 새 등록 성공은 S_OK, 입력/준비 실패는 실패 HRESULT다.
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::uint32_t vehicleId);
	// Level/vehicle ID의 준비 기록을 mutex 안에서 조회한다.
	static bool_t Is_Ready(uint32_t iLevelIndex, std::uint32_t vehicleId);
	// vehicle ID를 decimal 문자열로 붙인 모델 tag를 값으로 반환한다.
	static wstring_t Get_ModelPrototypeTag(std::uint32_t vehicleId);
	// 공유 상태 문자열의 차용 참조다. 이 getter는 lock하지 않으므로 동시 갱신과의 안전한 snapshot을 보장하지 않는다.
	static const std::string& Get_Status();
};

NS_END
