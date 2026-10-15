// 공개 descriptor와 레벨 생성·준비 조회 계약을 구현한다.
#include "LevelRegistry.h"

// 베른 Level의 실제 생성 함수를 등록한다.
#include "Level_Bern.h"
// 캐릭터 생성 Level의 실제 생성 함수를 등록한다.
#include "Level_CharacterSelect.h"
// 수련장·마하라카·콜로세움 공용 Level의 실제 생성 함수를 등록한다.
#include "Level_Development.h"
// 쿠크 아레나 Level의 실제 생성 함수를 등록한다.
#include "Level_KoukuSaydonArena.h"
// 로비 Level의 실제 생성 함수를 등록한다.
#include "Level_Lobby.h"
// 발탄 아레나 Level의 실제 생성 함수를 등록한다.
#include "Level_ValtanArena.h"
// 정적 준비 adapter가 CLevelRegistry의 friend 권한으로 Ready_For 함수를 호출한다.
#include "Loader.h"

// Level enum과 일치하는 정적 등록 항목을 찾는다.
#include <algorithm>
// 등록 목록을 크기가 고정된 정적 배열로 보관한다.
#include <array>
// 전체 맵 범위를 표현하는 부동소수점 최솟값·최댓값을 사용한다.
#include <limits>

namespace
{
	using namespace Client;
	using namespace Engine;

	// 제품 Level의 전체 맵 범위를 만든다. 선택적인 제외 그룹만 덧붙이며 저작 원본을 변경하지 않는다.
	MAP_LOAD_SCOPE MakeFullMapScope(
		const char_t* excludedAssetGroupId = nullptr)
	{
		MAP_LOAD_SCOPE scope
		{
			true,
			true,
			(std::numeric_limits<f32_t>::lowest)(),
			(std::numeric_limits<f32_t>::lowest)(),
			(std::numeric_limits<f32_t>::max)(),
			(std::numeric_limits<f32_t>::max)()
		};
		if (nullptr != excludedAssetGroupId)
			scope.excludedAssetGroupId = excludedAssetGroupId;
		return scope;
	}

	// 베른의 큰 지도 범위에 사용할 frustum 여유와 거부 지연을 정적 descriptor에 담는다.
	MAP_LOAD_SCOPE MakeBernMapScope()
	{
		MAP_LOAD_SCOPE scope = MakeFullMapScope();
		// 베른의 원거리 평면은 max(2000, span * 8)로 정해져 이 맵에서는 약 18837까지 닿는다.
		// 과거 모서리 기반 frustum 평면은 float32 상쇄 오차로 실제 가시 지형을 잘랐으나,
		// 현재는 view-projection 행렬에서 평면을 구하므로 아래 값은 일반적인 보수 여유다.
		// diagnostics를 켜면 거부 배치마다 평면 거리와 화면 NDC를
		// Diagnostics/BernFrustumCulling.log에 기록하고 flush하므로 평상시에는 끈다.
		scope.frustumCulling.bypass = false;
		scope.frustumCulling.diagnostics = false;
		scope.frustumCulling.baseMargin = 0.25f;
		scope.frustumCulling.largeObjectRadiusThreshold = 4.f;
		scope.frustumCulling.largeObjectAbsoluteMargin = 2.f;
		scope.frustumCulling.largeObjectRelativeMargin = 0.12f;
		scope.frustumCulling.rejectHysteresisFrames = 3u;
		return scope;
	}

	// descriptor의 동일한 생성 시그니처를 로비 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateLobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Lobby::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 베른 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateBern(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Bern::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 캐릭터 생성 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateCharacterSelect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_CharacterSelect::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 발탄 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateValtanArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_ValtanArena::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 쿠크 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateKoukuSaydonArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_KoukuSaydonArena::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 수련장 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateDevelopment(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		return CLevel_Development::Create(pDevice, pContext);
	}

	// descriptor의 동일한 생성 시그니처를 마하라카 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateMaharaka(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		// 섬은 Development의 맵·추적 카메라·Server 복제·플레이어 제어를 재사용한다.
		// LEVEL::MAHARAKA를 넘겨 Map Editor 분기와 구분한다.
		return CLevel_Development::Create(pDevice, pContext, LEVEL::MAHARAKA);
	}

	// descriptor의 동일한 생성 시그니처를 콜로세움 Level의 Create에 연결하고 생성 결과 소유권을 반환한다.
	unique_ptr<CLevel> CreateColosseum(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext)
	{
		// 같은 Development 구현에 콜로세움 Level을 전달한다. 경기·복제의 실제 동작은 해당 Level 구현이 담당한다.
		return CLevel_Development::Create(pDevice, pContext, LEVEL::COLOSSEUM);
	}

}

// Loader와 MainApp이 공유하는 등록 정본이다. static 배열 수명은 프로세스 전체이며 반환 포인터는 읽기 전용이다.
const CLIENT_LEVEL_DESCRIPTOR* CLevelRegistry::Find(
	const LEVEL eLevel)
{
	static const std::array<CLIENT_LEVEL_DESCRIPTOR, 8> levels =
	{{
		{
			LEVEL::LOBBY,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.lobby",
			nullptr,
			"scene.lobby.neutral.v1",
			{},
			CreateLobby,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_Lobby(); }
		},
		{
			LEVEL::CHARACTER_SELECT,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.character-select",
			"LV_LOBBY_CLASSSELECT_SL00",
			"scene.character-select.warm-high-key.v1",
			MakeFullMapScope(),
			CreateCharacterSelect,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_CharacterSelect(); },
			"LV_LOBBY_CLASSSELECT_SL10",
			MakeFullMapScope()
		},
		{
			LEVEL::BERN,
			CLIENT_LEVEL_KIND::PRODUCT,
			"world.bern",
			"LV_BER_BERNCASTLE",
			"scene.bern.neutral-day.v1",
			// 42개 landscape component가 접근로 전체의 지면을 구성하므로 제품 맵은 모두 로드한다.
			// Map Editor의 "Show Bern Landscape"는 별도의 가역 저작 표시 옵션이다.
			MakeBernMapScope(),
			CreateBern,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_Bern(); }
		},
		{
			LEVEL::VALTAN_ARENA,
			CLIENT_LEVEL_KIND::PRODUCT,
			"raid.valtan.arena",
			"LV_LUT_HEARTRB_ED",
			"scene.valtan.cool-low-key.v1",
			MakeFullMapScope(),
			CreateValtanArena,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_ValtanArena(); }
		},
		{
			LEVEL::KOUKUSAYDON_ARENA,
			CLIENT_LEVEL_KIND::PRODUCT,
			"raid.kouku-saydon.arena",
			"LV_LUT_MIDNIGHTC_ED",
			"scene.koukusaydon.g1.base.v1",
			MakeFullMapScope(),
			CreateKoukuSaydonArena,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_KoukuSaydonArena(); }
		},
		{
			LEVEL::DEVELOPMENT,
			CLIENT_LEVEL_KIND::DEVELOPMENT,
			"dev.training.ground",
			"LV_DEV_TRAINING_GROUND",
			"scene.development.neutral.v1",
			{ true, false, -20.f, -20.f, 20.f, 20.f },
			CreateDevelopment,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_Development(); }
		},
		{
			LEVEL::MAHARAKA,
			CLIENT_LEVEL_KIND::PRODUCT,
			"world.maharaka",
			"LV_OCN_EVENTIS_MHP",
			// 마하라카는 섬 원본 환경 프로필을 사용하며 다른 Area 프로필과 독립이다.
			"scene.maharaka.source-day.v1",
			MakeFullMapScope(),
			CreateMaharaka,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_Maharaka(); }
		},
		{
			LEVEL::COLOSSEUM,
			CLIENT_LEVEL_KIND::PRODUCT,
			"world.colosseum",
			"LV_PVP_COLOSSEUM",
			"scene.colosseum.source-day.v1",
			MakeFullMapScope(),
			CreateColosseum,
			[](CLoader& loader) -> HRESULT { return loader.Ready_For_Colosseum(); }
		}
	}};

	const auto iter = std::find_if(
		levels.begin(),
		levels.end(),
		[eLevel](const CLIENT_LEVEL_DESCRIPTOR& descriptor)
		{
			return descriptor.eLevel == eLevel;
		});
	return levels.end() == iter ? nullptr : &(*iter);
}

// 활성화할 Level 객체만 생성한다. 실제 현재 Level 교체와 실패 복구는 MainApp이 소유한다.
unique_ptr<CLevel> CLevelRegistry::Create_Level(
	const LEVEL eLevel,
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	const CLIENT_LEVEL_DESCRIPTOR* pDescriptor = Find(eLevel);
	if (nullptr == pDescriptor || nullptr == pDescriptor->pCreate)
		return nullptr;

	return pDescriptor->pCreate(
		std::move(pDevice),
		std::move(pContext));
}

// Loader의 worker가 등록된 준비 경로를 호출한다. 로드 작업과 rollback의 수명은 전달된 Loader가 관리한다.
HRESULT CLevelRegistry::Execute_Load(
	const LEVEL eLevel,
	CLoader& loader)
{
	const CLIENT_LEVEL_DESCRIPTOR* pDescriptor = Find(eLevel);

	if (nullptr == pDescriptor || nullptr == pDescriptor->pLoad)
		return E_INVALIDARG;

	return pDescriptor->pLoad(loader);
}
