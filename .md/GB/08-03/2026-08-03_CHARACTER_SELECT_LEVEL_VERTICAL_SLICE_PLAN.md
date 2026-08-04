# Character Select Map Loader와 Enter-to-Test 수직 슬라이스 PLAN

작성: 2026-08-03
전면 갱신: 2026-08-04
상태: 구현 전 전체 코드 정답지

## 0. 이번 목표와 비목표

```text
Lobby의 Character Select 클릭
-> Loading Level
-> Character Select 전용 map asset + 다섯 playable character asset 준비
-> LEVEL::CHARACTER_SELECT 활성화
-> LV_LOBBY_CLASSSELECT_SL00 visual map + preview Character + ImGui 표시
-> class 선택
-> Enter 키 또는 Enter Test 버튼
-> 선택 class commit + LOBBY_STAGE::TEST command 예약
-> Lobby Loading
-> 기존 Lobby가 C2S_ENTER_WORLD(TRAINING_GROUND, selected class) 전송
-> S2C_ENTER_ACCEPTED 확인
-> DEVELOPMENT Loading
-> Server spawn class로 실제 Character/HUD 생성
-> CPlayerController의 우클릭 이동과 Q/W skill command 사용
```

이번 변경에서 하지 않는 것:

```text
새 LEVEL::TRAINING 추가
Character Select Level에서 socket 직접 호출
Character Select preview Character를 실제 player로 승격
GameRoom/Shared packet/Server tick의 두 번째 경로 추가
LV_LOBBY_CLASSSELECT_SL00를 Server navigation world로 사용
trainingTarget/monster catalog/새 boss placement 추가
MapTool navigation authoring 확장
제품 HUD를 ImGui 구현으로 확정
```

Character Select를 거치지 않아도 Lobby에서 Test/Bern/Valtan을 누를 수 있다. 명시적으로 선택한 class가 없으면 이번 process의 entry class는 `LANCE_MASTER`로 commit하고 기존 Server 승인 경로를 사용한다.

## 1. C1~C8 관점

```text
C1 기준계          중요: Character Select 원본 placement 좌표와 preview/camera 좌표를 같은 world space로 사용한다.
C2 이동>계산       Loader가 map/model prototype을 준비하고 Level은 staged resource를 commit한다.
C3 공유는 비싸다   다섯 class preview는 선택 화면에서만 모두 준비하며 제품 world는 승인된 class만 준비한다.
C4 수명은 선언된다 중요: preview/map은 CHARACTER_SELECT Level 수명, 실제 player는 DEVELOPMENT Level 수명이다.
C5 이산화와 오차   placement 실측 범위와 preview foot height를 고정한다.
C6 가지치기        Character Select main cluster scope만 Loader가 admission한다.
C7 권위와 정합성   중요: class 선택은 Client session state, 실제 entity/action/HUD 수치는 Server snapshot이 정본이다.
C8 검증이 병목     map resource 실재, Loader rollback, Server approval, HUD/이동/스킬 runtime을 분리 검증한다.
```

## 2. 문제 해결 ①~⑤

```text
① 문제·제약: Character Select map resource는 추출됐지만 LevelRegistry/Loader/MapRuntime에 연결되지 않았고 Confirm은 Lobby 복귀만 수행한다.
② 단순 해법의 문제: preview Character를 그대로 움직이거나 Character Select에서 socket을 열면 server entity와 local entity라는 두 gameplay 경로가 생긴다.
③ 해결 방식: Character Select는 map+preview+선택만 소유하고 Enter가 기존 Lobby Test command를 예약한다. 실제 gameplay는 승인 후 DEVELOPMENT에서 시작한다.
④ 비교: Lobby를 다시 거치지만 입장 state machine, timeout, endpoint, world mapping, rollback을 복제하지 않는다.
⑤ 대가: Character Select -> Lobby -> Development로 Loading이 두 번 보인다. 추후 공용 entry coordinator가 실제 요구가 될 때만 별도 수직 슬라이스로 옮긴다.
```

## 3. 현재 데이터 실측과 고정값

```text
MapCatalog area                  LV_LOBBY_CLASSSELECT_SL00
mapassets header                LOSTARK_MAP_ASSET_CATALOG 4 "LV_LOBBY_CLASSSELECT_SL00" 55
mapplacements header            LOSTARK_MAP_PLACEMENTS 2 "LV_LOBBY_CLASSSELECT_SL00" 803
runtime root                    Client/Bin/Resources/Map/CHARACTERSELECTMAP
runtime manifest assetCount     55
main cluster placement count    779
main cluster X                  -790.837422 .. -751.900781
main cluster Y                  -250.255215 .. -102.6
main cluster Z                   159.164424 .. 216.79291
central floor reference         approximately (-772.017, -142.7, 197.538)
branch HEAD lock                lostark-resources 2026.08.03.4
working-tree lock               lostark-resources 2026.08.03.3
working-tree deleted manifest   lostark-resources-2026.08.03.4.manifest.json
working-tree lock fileCount     7922
current Resources fileCount     10158
current extra / missing / size  2236 / 0 / 0
next immutable pack version     2026.08.04.1
```

현재 로컬 Resources에는 Character Select 215개 파일이 `Map/CHARACTERSELECTMAP` 아래에 있고 runtime manifest는 model 55개를 모두 가리킨다. 에셋은 없는 상태가 아니다. 문제는 working-tree lock `2026.08.03.3`보다 Character/Effect/Map intake 2236개가 앞서 있어 팀원이 같은 payload를 Hydrate할 수 없고, 다른 세션이 branch HEAD의 immutable `.4` manifest를 삭제 표시한 점이다. 이 계획은 그 삭제나 lock downgrade를 소유하지 않는다. 담당 세션이 lock diff를 commit 또는 복구하고 기존 immutable manifest 삭제가 0개인 상태에서만 `2026.08.04.1`을 Snapshot → Verify → Publish → Hydrate로 고정한다. 기존 `.1`~`.4`를 덮어쓰거나 삭제하지 않고 새 manifest/lock/ZIP/SHA-256을 RESULT에 기록한다.

정확한 Level load scope:

```cpp
{ true, true, -792.f, 158.f, -750.f, 218.f }
```

이 좌표는 전체 803 placement의 main cluster를 실측해서 잡은 값이다. 인게임에서 카메라/바닥이 맞지 않으면 숫자를 감으로 수정하지 않고 MapTool에서 실제 placement와 camera transform을 관찰한 뒤 같은 descriptor와 preview 상수를 함께 갱신한다.

## 4. 파일 변경 지도

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_CharacterSelect.h` | map runtime, preview, Enter 계약 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_CharacterSelect.cpp` | map commit, preview 교체, Enter-to-Test command |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_Lobby.h` | default entry가 가능한 상태 문구와 기존 approval state 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Lobby.cpp` | explicit selection 또는 Lance Master default로 기존 Server 입장 |
| 객체 교체 | `C:/Users/user/Desktop/LostArk/Data/Maps/MapCatalog.json` | Character Select visual area를 제품 frontend admission으로 승격 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Data/Balance/PlayerSkills.json` | 검증된 기존 네 class가 각자 소유하는 Q/W Server skill 계약. DimensionMaster는 실제 ID 확보 전 비워 둔다. |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Data/Balance/DamageProfiles.json` | 신규 여섯 skill의 명시적 training damage profile |
| 블록 교체 | `C:/Users/user/Desktop/LostArk/Server/Private/ServerGameplayContractTests.cpp` | 기존 네 class Q/W catalog와 Server 승인, DimensionMaster profile 검증 |
| 함수 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp` | Character Select map area와 load scope 등록 |
| 함수 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | Character Select map + camera + 다섯 class prototype stage |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/LobbyCommandService.h` | stage command와 취소 가능한 handoff token 계약 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/LobbyCommandService.cpp` | token 발급·단일 pending·정확한 취소 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/LevelTransitionService.h` | Level request가 Lobby command token을 운반 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/LevelTransitionService.cpp` | token이 붙은 Lobby load/activation 검증 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_Loading.h` | Loader 구간의 token ownership 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp` | load 실패 취소와 activation ownership transfer |
| 함수 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | load 시작/activation 실패에서 token 취소 |
| 새 프로젝트 | `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness` | Lobby command token ledger의 정상·중복·정확 취소·stale 단위 검증 |
| 블록 추가 | `C:/Users/user/Desktop/LostArk/Framework.sln`, `Tools/Build/Invoke-BuildAndRegression.ps1` | 새 harness를 정본 regression에 포함 |
| 도구 생성물 | `C:/Users/user/Desktop/LostArk/Data/AssetPacks.lock.json`, `Data/AssetManifests/lostark-resources-2026.08.04.1.manifest.json` | 현재 6-root Resources immutable snapshot |
| 블록 추가/교체 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | map/resource/entry 경계 정적 검사 |
| 문단 교체 | `C:/Users/user/Desktop/LostArk/AGENTS.md` | fixed runtime contract 교정 |
| 문단 교체 | `C:/Users/user/Desktop/LostArk/CLAUDE.md` | 실행/Loader 사용법 교정 |
| 문단 교체 | `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 팀 소비 계약 교정 |

Client runtime의 새 H/CPP는 없다. `Client.vcxproj`와 `Client.vcxproj.filters` 등록 변경도 없다. 새 `ClientFrontendHarness.cpp`는 전용 `.vcxproj`, `.vcxproj.filters`, `Framework.sln`, build regression에 등록한다.

다른 세션이 현재 수정 중인 다음 파일은 이 구현에서 덮어쓰지 않는다.

```text
Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Tools/ProjectAudit/Invoke-ProjectAudit.ps1의 unrelated effect/animation audit 변경
```

`Data/Maps/MapCatalog.json`은 다른 세션의 나머지 Area 변경을 그대로 보존하고 `LV_LOBBY_CLASSSELECT_SL00` 객체만 아래 코드로 교체한다. 현재 dirty mapassets 55개 row와 `Map/CHARACTERSELECTMAP` root는 다른 세션의 추출 결과이므로 재생성하거나 되돌리지 않고 ProjectAudit에서 catalog/manifest set 일치를 검증한다.

## 5. G1 — Character Select visual map과 다섯 class Loader

### 5-1. `C:/Users/user/Desktop/LostArk/Client/Public/Level_CharacterSelect.h`

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "Network/PacketType.h"

#include <array>

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;

class CLevel_CharacterSelect final : public CLevel
{
private:
	CLevel_CharacterSelect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_CharacterSelect();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_Preview(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool_t Select_Preview(size_t index);
	bool_t Enter_TrainingGround();
	void Render_SelectionPanel();

private:
	static constexpr std::array<
		LostArk::Shared::CHARACTER_CLASS_ID, 5> SUPPORTED_CLASSES =
	{
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
		LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
		LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
		LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER
	};

	CMapPlacementRuntime m_MapRuntime;
	size_t m_iPreviewIndex = 0;
	shared_ptr<CCharacter> m_pPreviewCharacter = { nullptr };
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	string m_strStatus =
		"Choose a class, then press Enter to join the Test stage.";

public:
	static unique_ptr<CLevel_CharacterSelect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### 5-2. `C:/Users/user/Desktop/LostArk/Client/Private/Level_CharacterSelect.cpp`

변경 종류: 전체 교체

```cpp
#include "imgui.h"

#include "Level_CharacterSelect.h"

#include "AnimationTargetService.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "LobbyCommandService.h"
#include "Transform.h"

#include <algorithm>

namespace
{
	constexpr f32_t PREVIEW_POSITION_X = -772.017f;
	constexpr f32_t PREVIEW_POSITION_Y = -142.55f;
	constexpr f32_t PREVIEW_POSITION_Z = 197.538f;

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		default:
			return "Unknown";
		}
	}
}

CLevel_CharacterSelect::CLevel_CharacterSelect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_CharacterSelect::~CLevel_CharacterSelect()
{
	CAnimationTargetService::Unbind(m_pPreviewCharacter);
	m_MapRuntime.Clear();
}

HRESULT CLevel_CharacterSelect::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::CHARACTER_SELECT),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_CharacterSelect] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	LostArk::Shared::CHARACTER_CLASS_ID initialClass =
		SUPPORTED_CLASSES.front();
	if (CCharacterSelectionState::Try_Get_SelectedClass(initialClass))
	{
		const auto selected = std::find(
			SUPPORTED_CLASSES.begin(),
			SUPPORTED_CLASSES.end(),
			initialClass);
		if (SUPPORTED_CLASSES.end() == selected)
			return E_INVALIDARG;
		m_iPreviewIndex = static_cast<size_t>(
			std::distance(SUPPORTED_CLASSES.begin(), selected));
	}

	if (FAILED(Ready_Preview(initialClass)) || FAILED(Ready_Camera()))
		return E_FAIL;

	return S_OK;
}

void CLevel_CharacterSelect::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_CharacterSelect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	Render_SelectionPanel();
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Lights()
{
	return CMapPlacementRuntime::Ensure_DefaultLight();
}

HRESULT CLevel_CharacterSelect::Ready_Camera()
{
	if (nullptr == m_pPreviewCharacter ||
		nullptr == m_pPreviewCharacter->Get_Transform())
	{
		return E_FAIL;
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + 2.4f,
		PREVIEW_POSITION_Z - 6.f);
	cameraDesc.vAt = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + 1.f,
		PREVIEW_POSITION_Z);
	cameraDesc.fFovy = 45.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 8.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = m_pPreviewCharacter->Get_Transform();
	cameraDesc.vPositionOffset = float3_t(0.f, 2.4f, -6.f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.f, 0.f);
	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = true;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_Camera"),
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_Camera"),
			gameObject);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Preview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return E_INVALIDARG;
	}

	const CHARACTER_SPEC* pSpec = CCharacterCatalog::Find_Spec(characterClass);
	if (nullptr == pSpec)
		return E_FAIL;

	CCharacter::CHARACTER_DESC characterDesc{};
	characterDesc.iPrototypeLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	characterDesc.pSpec = pSpec;
	characterDesc.pNavigationPrototypeTag = nullptr;
	characterDesc.fSpeedPerSec = 0.f;
	characterDesc.fRotationPerSec = 90.f;
	characterDesc.vPosition = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y,
		PREVIEW_POSITION_Z);
	characterDesc.strNickName = Get_CharacterClassName(characterClass);
	characterDesc.isLocallyControlled = false;

	shared_ptr<CGameObject> stagedObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_PreviewCharacter"),
		&characterDesc,
		&stagedObject)))
	{
		return E_FAIL;
	}

	const shared_ptr<CCharacter> stagedCharacter =
		dynamic_pointer_cast<CCharacter>(stagedObject);
	if (nullptr == stagedCharacter ||
		nullptr == stagedCharacter->Get_Transform() ||
		!stagedCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			stagedObject);
		return E_FAIL;
	}

	stagedCharacter->Get_Transform()->Rotation(0.f, 180.f, 0.f);

	const shared_ptr<CCharacter> previousCharacter = m_pPreviewCharacter;
	m_pPreviewCharacter = stagedCharacter;
	if (nullptr != previousCharacter)
		CAnimationTargetService::Unbind(previousCharacter);
	CAnimationTargetService::Bind(m_pPreviewCharacter);

	if (nullptr != m_pCamera)
	{
		m_pCamera->Set_FollowTarget(m_pPreviewCharacter->Get_Transform());
		m_pCamera->Set_FollowEnabled(true);
	}

	if (nullptr != previousCharacter)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			previousCharacter);
	}

	return S_OK;
}

bool_t CLevel_CharacterSelect::Select_Preview(const size_t index)
{
	if (index >= SUPPORTED_CLASSES.size())
		return false;
	if (index == m_iPreviewIndex && nullptr != m_pPreviewCharacter)
		return true;

	if (FAILED(Ready_Preview(SUPPORTED_CLASSES[index])))
	{
		m_strStatus =
			"The new preview could not be created. The previous preview was kept.";
		return false;
	}

	m_iPreviewIndex = index;
	m_strStatus = std::string("Previewing ") +
		Get_CharacterClassName(SUPPORTED_CLASSES[index]) +
		". Press Enter to join Test.";
	return true;
}

bool_t CLevel_CharacterSelect::Enter_TrainingGround()
{
	if (m_iPreviewIndex >= SUPPORTED_CLASSES.size() ||
		nullptr == m_pPreviewCharacter)
	{
		m_strStatus = "There is no valid preview to enter with.";
		return false;
	}

	if (!CCharacterSelectionState::Select(
		SUPPORTED_CLASSES[m_iPreviewIndex]))
	{
		m_strStatus = "The selected class is not supported.";
		return false;
	}

	LOBBY_COMMAND_TOKEN commandToken = INVALID_LOBBY_COMMAND_TOKEN;
	if (!CLobbyCommandService::Request(
		LOBBY_STAGE::TEST,
		commandToken))
	{
		m_strStatus = CLobbyCommandService::Get_Status();
		return false;
	}

	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"character-select.enter-test",
		commandToken))
	{
		CLobbyCommandService::Cancel(
			commandToken,
			"Lobby load request was rejected");
		m_strStatus = CLevelTransitionService::Get_Status();
		return false;
	}

	m_strStatus =
		"Selection committed. Lobby will request the Test world from Server.";
	return true;
}

void CLevel_CharacterSelect::Render_SelectionPanel()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_Always);
	}

	if (!ImGui::Begin(
		"Character Select",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Choose a playable class");
	for (size_t index = 0; index < SUPPORTED_CLASSES.size(); ++index)
	{
		const bool_t isSelected = index == m_iPreviewIndex;
		if (ImGui::Selectable(
			Get_CharacterClassName(SUPPORTED_CLASSES[index]),
			isSelected))
		{
			Select_Preview(index);
		}
	}

	ImGui::Separator();
	const bool_t transitionPending = CLevelTransitionService::Is_Pending();
	const bool_t enterPressed =
		ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
		ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
	ImGui::BeginDisabled(transitionPending);
	if (ImGui::Button("Enter Test") ||
		(!transitionPending && enterPressed))
		Enter_TrainingGround();
	ImGui::SameLine();
	if (ImGui::Button("Back"))
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"character-select.back"))
		{
			m_strStatus = CLevelTransitionService::Get_Status();
		}
	}
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"Enter: join Test  |  Back: keep the previous committed selection");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

unique_ptr<CLevel_CharacterSelect> CLevel_CharacterSelect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_CharacterSelect>(
		new CLevel_CharacterSelect(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
```

### 5-3. `C:/Users/user/Desktop/LostArk/Data/Maps/MapCatalog.json`

변경 종류: `LV_LOBBY_CLASSSELECT_SL00` 객체 전체 교체

```json
{
  "id": "LV_LOBBY_CLASSSELECT_SL00",
  "kind": "product",
  "catalogType": "single",
  "catalog": "Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets",
  "placements": "Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements",
  "placementCount": 803,
  "assetCount": 55,
  "runtimeAssetRoot": "Map/CHARACTERSELECTMAP"
}
```

### 5-4. `C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp`

변경 종류: `CLevelRegistry::Find` 함수 전체 교체

```cpp
const CLIENT_LEVEL_DESCRIPTOR* CLevelRegistry::Find(
	const LEVEL eLevel)
{
	static const std::array<CLIENT_LEVEL_DESCRIPTOR, 5> levels =
	{{
		{
			LEVEL::LOBBY,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.lobby",
			nullptr,
			{},
			CreateLobby,
			&CLoader::Ready_For_Lobby
		},
		{
			LEVEL::CHARACTER_SELECT,
			CLIENT_LEVEL_KIND::PRODUCT,
			"front.character-select",
			"LV_LOBBY_CLASSSELECT_SL00",
			{ true, true, -792.f, 158.f, -750.f, 218.f },
			CreateCharacterSelect,
			&CLoader::Ready_For_CharacterSelect
		},
		{
			LEVEL::BERN,
			CLIENT_LEVEL_KIND::PRODUCT,
			"world.bern",
			"LV_BER_BERNCASTLE",
			{ true, true, -50.f, -50.f, 50.f, 50.f },
			CreateBern,
			&CLoader::Ready_For_Bern
		},
		{
			LEVEL::VALTAN_ARENA,
			CLIENT_LEVEL_KIND::PRODUCT,
			"raid.valtan.arena",
			"LV_LUT_HEARTRB_ED",
			{ true, true, 120.5f, -157.5f, 191.5f, -86.f },
			CreateValtanArena,
			&CLoader::Ready_For_ValtanArena
		},
		{
			LEVEL::DEVELOPMENT,
			CLIENT_LEVEL_KIND::DEVELOPMENT,
			"dev.training.ground",
			"LV_DEV_TRAINING_GROUND",
			{ true, false, -20.f, -20.f, 20.f, 20.f },
			CreateDevelopment,
			&CLoader::Ready_For_Development
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
```

### 5-5. `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp`

변경 종류: `CLoader::Ready_For_CharacterSelect` 함수 전체 교체

```cpp
HRESULT CLoader::Ready_For_CharacterSelect()
{
	using LostArk::Shared::CHARACTER_CLASS_ID;
	constexpr std::array CHARACTER_CLASSES =
	{
		CHARACTER_CLASS_ID::LANCE_MASTER,
		CHARACTER_CLASS_ID::GUNSLINGER,
		CHARACTER_CLASS_ID::SLAYER,
		CHARACTER_CLASS_ID::ARTIST
	};

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::CHARACTER_SELECT));
	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Set_Status(TEXT("CHARACTER SELECT: visual map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("CHARACTER SELECT: playable classes"));
	if (FAILED(Ready_Character_Rendering(
			ETOUI(LEVEL::CHARACTER_SELECT),
			CHARACTER_CLASSES)) ||
		FAILED(Ready_AnimationPreviewModels(
			ETOUI(LEVEL::CHARACTER_SELECT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Character Select loading complete"));
	rollback.Commit();
	return S_OK;
}
```

## 6. G2 — 실패해도 남지 않는 Enter-to-Test ticket

Character Select의 `Enter`는 Lobby가 소비할 `TEST` command와 Lobby Level 전환을 하나의 handoff로 묶는다. command를 먼저 예약한 뒤 load 시작, Loader, Lobby activation 중 하나라도 실패하면 같은 token의 command만 취소한다. 이미 소비됐거나 더 새로운 command는 오래된 실패가 취소할 수 없다.

### 6-1. `C:/Users/user/Desktop/LostArk/Client/Public/LobbyCommandService.h`

변경 종류: 전체 교체

```cpp
#pragma once

#include "Engine_Defines.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

enum class LOBBY_STAGE
{
	TEST,
	CHARACTER_SELECT,
	VALTAN,
	BERN,
	END
};

using LOBBY_COMMAND_TOKEN = std::uint64_t;
inline constexpr LOBBY_COMMAND_TOKEN INVALID_LOBBY_COMMAND_TOKEN = 0;

struct LOBBY_COMMAND final
{
	LOBBY_STAGE eStage = LOBBY_STAGE::END;
	LOBBY_COMMAND_TOKEN iToken = INVALID_LOBBY_COMMAND_TOKEN;
};

class CLobbyCommandService final
{
public:
	static bool_t Request(LOBBY_STAGE eStage);
	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_TOKEN& outToken);
	static bool_t Cancel(
		LOBBY_COMMAND_TOKEN token,
		const char_t* pReason);
	static bool_t Try_Consume(LOBBY_COMMAND& outCommand);
	static std::string Get_Status();
};

NS_END
```

### 6-2. `C:/Users/user/Desktop/LostArk/Client/Private/LobbyCommandService.cpp`

변경 종류: 전체 교체

```cpp
#include "LobbyCommandService.h"

#include <limits>
#include <mutex>
#include <optional>
#include <utility>

namespace
{
	std::mutex g_CommandMutex;
	std::optional<Client::LOBBY_COMMAND> g_PendingCommand;
	Client::LOBBY_COMMAND_TOKEN g_iNextToken = 1;
	std::string g_Status = "No lobby command is pending.";
}

bool_t Client::CLobbyCommandService::Request(const LOBBY_STAGE eStage)
{
	LOBBY_COMMAND_TOKEN ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;
	return Request(eStage, ignoredToken);
}

bool_t Client::CLobbyCommandService::Request(
	const LOBBY_STAGE eStage,
	LOBBY_COMMAND_TOKEN& outToken)
{
	outToken = INVALID_LOBBY_COMMAND_TOKEN;
	if (LOBBY_STAGE::END == eStage)
	{
		std::scoped_lock lock{ g_CommandMutex };
		g_Status = "Rejected invalid lobby command.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	if (g_PendingCommand.has_value())
	{
		g_Status = "A lobby command is already pending.";
		return false;
	}
	if ((std::numeric_limits<LOBBY_COMMAND_TOKEN>::max)() == g_iNextToken)
	{
		g_Status = "Lobby command token space is exhausted.";
		return false;
	}

	outToken = g_iNextToken++;
	g_PendingCommand = LOBBY_COMMAND{ eStage, outToken };
	g_Status = "Lobby command staged with token " +
		std::to_string(outToken) + ".";
	return true;
}

bool_t Client::CLobbyCommandService::Cancel(
	const LOBBY_COMMAND_TOKEN token,
	const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == token ||
		nullptr == pReason || '\0' == *pReason)
	{
		std::scoped_lock lock{ g_CommandMutex };
		g_Status = "Rejected invalid lobby command cancellation.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	if (!g_PendingCommand.has_value() ||
		g_PendingCommand->iToken != token)
	{
		g_Status = "Lobby command cancellation token did not match.";
		return false;
	}

	g_PendingCommand.reset();
	g_Status = std::string("Lobby command cancelled: ") + pReason;
	return true;
}

bool_t Client::CLobbyCommandService::Try_Consume(
	LOBBY_COMMAND& outCommand)
{
	std::scoped_lock lock{ g_CommandMutex };
	if (!g_PendingCommand.has_value())
		return false;

	outCommand = *g_PendingCommand;
	g_PendingCommand.reset();
	g_Status = "Lobby command consumed.";
	return true;
}

std::string Client::CLobbyCommandService::Get_Status()
{
	std::scoped_lock lock{ g_CommandMutex };
	return g_Status;
}
```

### 6-3. `C:/Users/user/Desktop/LostArk/Client/Public/LevelTransitionService.h`

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"

#include <string>

NS_BEGIN(Client)

enum class LEVEL_TRANSITION_PHASE
{
	LOAD,
	ACTIVATE
};

struct LEVEL_TRANSITION_REQUEST final
{
	LEVEL_TRANSITION_PHASE ePhase = LEVEL_TRANSITION_PHASE::LOAD;
	LEVEL eTargetLevel = LEVEL::END;
	std::string strSource;
	LOBBY_COMMAND_TOKEN iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
};

class CLevelTransitionService final
{
public:
	static bool_t Request_Load(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	static bool_t Request_Activation(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	static bool_t Try_Consume(LEVEL_TRANSITION_REQUEST& outRequest);
	static bool_t Is_Pending();
	static std::string Get_Status();
	static void Report_LoadFailure(HRESULT result);
	static bool_t Try_ConsumeLoadFailure(HRESULT& outResult);

private:
	static bool_t Request(
		LEVEL_TRANSITION_PHASE ePhase,
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
};

NS_END
```

### 6-4. `C:/Users/user/Desktop/LostArk/Client/Private/LevelTransitionService.cpp`

변경 종류: 전체 교체

```cpp
#include "LevelTransitionService.h"

#include "LevelRegistry.h"

#include <mutex>
#include <optional>
#include <utility>

namespace
{
	std::mutex g_TransitionMutex;
	std::optional<Client::LEVEL_TRANSITION_REQUEST> g_PendingRequest;
	std::optional<HRESULT> g_LoadFailure;
	std::string g_Status = "No level transition is pending.";
}

bool_t Client::CLevelTransitionService::Request_Load(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::LOAD,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

bool_t Client::CLevelTransitionService::Request_Activation(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::ACTIVATE,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

bool_t Client::CLevelTransitionService::Try_Consume(
	LEVEL_TRANSITION_REQUEST& outRequest)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_PendingRequest.has_value())
		return false;

	outRequest = std::move(*g_PendingRequest);
	g_PendingRequest.reset();
	g_Status = "Level transition request consumed.";
	return true;
}

bool_t Client::CLevelTransitionService::Is_Pending()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_PendingRequest.has_value();
}

std::string Client::CLevelTransitionService::Get_Status()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_Status;
}

void Client::CLevelTransitionService::Report_LoadFailure(
	const HRESULT result)
{
	std::scoped_lock lock{ g_TransitionMutex };
	g_LoadFailure = result;
	g_Status = "Level loading failed with HRESULT " +
		std::to_string(static_cast<long>(result)) + ".";
}

bool_t Client::CLevelTransitionService::Try_ConsumeLoadFailure(
	HRESULT& outResult)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_LoadFailure.has_value())
		return false;

	outResult = *g_LoadFailure;
	g_LoadFailure.reset();
	return true;
}

bool_t Client::CLevelTransitionService::Request(
	const LEVEL_TRANSITION_PHASE ePhase,
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	const bool_t hasLobbyCommand =
		INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken;
	if (nullptr == CLevelRegistry::Find(eTargetLevel) ||
		nullptr == pSource || '\0' == *pSource ||
		(hasLobbyCommand && LEVEL::LOBBY != eTargetLevel))
	{
		std::scoped_lock lock{ g_TransitionMutex };
		g_Status = "Rejected invalid level transition request.";
		return false;
	}

	std::scoped_lock lock{ g_TransitionMutex };
	if (g_PendingRequest.has_value())
	{
		g_Status =
			"Rejected level transition while another request is pending.";
		return false;
	}

	g_PendingRequest = LEVEL_TRANSITION_REQUEST{
		ePhase,
		eTargetLevel,
		pSource,
		lobbyCommandToken
	};
	if (LEVEL_TRANSITION_PHASE::LOAD == ePhase)
		g_LoadFailure.reset();
	g_Status = "Level transition request staged by " +
		g_PendingRequest->strSource + ".";
	return true;
}
```

### 6-5. `C:/Users/user/Desktop/LostArk/Client/Public/Level_Loading.h`

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"

NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Loading();

public:
	virtual HRESULT Initialize(
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Recover_FromFailure(HRESULT result);
	void Cancel_LobbyCommand(const char_t* pReason);
	void Retry_LobbyLoad();

private:
	LEVEL m_eNextLevelID = LEVEL::END;
	LOBBY_COMMAND_TOKEN m_iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
	unique_ptr<class CLoader> m_pLoader = { nullptr };
	bool_t m_isActivationRequested = { false };
	bool_t m_isFailureReported = { false };
	bool_t m_isRetryRequested = { false };

public:
	static unique_ptr<CLevel_Loading> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
};

NS_END
```

### 6-6. `C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp`

변경 종류: 전체 교체

```cpp
#include "imgui.h"

#include "Level_Loading.h"

#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "Loader.h"
#include "NetworkManager.h"

CLevel_Loading::CLevel_Loading(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
}

HRESULT CLevel_Loading::Initialize(
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken &&
		LEVEL::LOBBY != eNextLevelID)
	{
		return E_INVALIDARG;
	}

	m_eNextLevelID = eNextLevelID;
	m_iLobbyCommandToken = lobbyCommandToken;
	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);
	return nullptr == m_pLoader ? E_FAIL : S_OK;
}

void CLevel_Loading::Update(const f32_t fTimeDelta)
{
	if (m_isRetryRequested)
	{
		m_isRetryRequested = false;
		Retry_LobbyLoad();
		return;
	}

	if (nullptr == m_pLoader)
		return;

	if (m_pLoader->Failed())
	{
		Recover_FromFailure(m_pLoader->Get_Result());
		return;
	}

	if (m_pLoader->Finished() && !m_isActivationRequested)
	{
		if (CLevelTransitionService::Request_Activation(
			m_eNextLevelID,
			"loading.complete",
			m_iLobbyCommandToken))
		{
			m_isActivationRequested = true;
			m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
		}
		else
		{
			OutputDebugStringA(
				"[Level_Loading] Activation request was rejected; retrying.\n");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (m_isFailureReported && LEVEL::LOBBY == m_eNextLevelID)
	{
		ImGui::SetNextWindowPos(ImVec2(24.f, 24.f), ImGuiCond_Always);
		if (ImGui::Begin(
			"Loading recovery",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped(
				"Lobby resources could not be loaded. Partial resources were rolled back.");
			if (ImGui::Button("Retry Lobby"))
				m_isRetryRequested = true;
		}
		ImGui::End();
	}

#ifdef _DEBUG
	if (nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif
	return S_OK;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	Cancel_LobbyCommand("target level loading failed");
	CLevelTransitionService::Report_LoadFailure(result);
	CNetworkManager::Get().Close_ServerConnection();

	if (FAILED(CGameInstance::Get().Clear_Resources(
		ETOUI(m_eNextLevelID))))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to clear partial target resources.\n");
	}

	OutputDebugStringA(
		"[Level_Loading] Load failed; session closed and partial resources rolled back.\n");

	if (LEVEL::LOBBY != m_eNextLevelID)
		Retry_LobbyLoad();
}

void CLevel_Loading::Cancel_LobbyCommand(const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == m_iLobbyCommandToken)
		return;

	CLobbyCommandService::Cancel(m_iLobbyCommandToken, pReason);
	m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
}

void CLevel_Loading::Retry_LobbyLoad()
{
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"loading.recovery"))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to stage Lobby recovery.\n");
	}
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	auto instance = unique_ptr<CLevel_Loading>(
		new CLevel_Loading(pDevice, pContext));
	if (FAILED(instance->Initialize(eNextLevelID, lobbyCommandToken)))
		return nullptr;
	return instance;
}
```

### 6-7. `C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h`

변경 종류: include와 선언 교체

```cpp
#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
```

```cpp
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
```

### 6-8. `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp`

변경 종류: include 추가

```cpp
#include "LobbyCommandService.h"
```

변경 종류: `Start_Level`, `Apply_LevelRequest` 함수 전체 교체

```cpp
HRESULT CMainApp::Start_Level(
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	if (nullptr == CLevelRegistry::Find(eTargetLevel))
		return E_INVALIDARG;

	unique_ptr<CLevel_Loading> loading =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			eTargetLevel,
			lobbyCommandToken);
	if (nullptr == loading)
		return E_FAIL;

	return CGameInstance::Get().Change_Level(
		ETOUI(LEVEL::LOADING),
		move(loading));
}

void CMainApp::Apply_LevelRequest()
{
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;

	if (LEVEL_TRANSITION_PHASE::LOAD == request.ePhase)
	{
		const HRESULT result = Start_Level(
			request.eTargetLevel,
			request.iLobbyCommandToken);
		if (FAILED(result))
		{
			if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
			{
				CLobbyCommandService::Cancel(
					request.iLobbyCommandToken,
					"target level loading could not start");
			}
			CLevelTransitionService::Report_LoadFailure(result);
		}
		return;
	}

	unique_ptr<CLevel> nextLevel = CLevelRegistry::Create_Level(
		request.eTargetLevel,
		m_pDevice,
		m_pContext);
	if (nullptr != nextLevel && SUCCEEDED(CGameInstance::Get().Change_Level(
		ETOUI(request.eTargetLevel),
		move(nextLevel))))
	{
		return;
	}

	if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
	{
		CLobbyCommandService::Cancel(
			request.iLobbyCommandToken,
			"target level activation failed");
	}
	CGameInstance::Get().Clear_Resources(ETOUI(request.eTargetLevel));
	CNetworkManager::Get().Close_ServerConnection();
	CLevelTransitionService::Report_LoadFailure(E_FAIL);
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"main-app.activation-failure"))
	{
		OutputDebugStringA(
			"[MainApp] Failed to stage Lobby recovery after activation failure.\n");
	}
}
```

일반 Level 전환은 invalid token이므로 `Cancel`을 호출하지 않는다. Character Select handoff failure에서만 유효한 token으로 정확히 해당 `TEST`를 제거한다.

## 7. G3 — optional direct entry와 기존 Lobby Server approval

### 7-1. `C:/Users/user/Desktop/LostArk/Client/Public/Level_Lobby.h`

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "Network/PacketType.h"

#include <chrono>

NS_BEGIN(Client)

class CLevel_Lobby final : public CLevel
{
private:
	enum class ENTRY_STATE
	{
		IDLE,
		WAITING_FOR_APPROVAL
	};

private:
	CLevel_Lobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Lobby();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	bool_t Begin_StageRequest(LOBBY_STAGE eStage);
	bool_t Begin_NetworkEntry(
		LostArk::Shared::WORLD_ID eWorldId,
		LEVEL eTargetLevel);
	bool_t Resolve_Stage(
		LOBBY_STAGE eStage,
		LostArk::Shared::WORLD_ID& outWorldId,
		LEVEL& outTargetLevel) const;
	void Consume_EnterAccepted();
	void Cancel_PendingEntry(const string& reason);
	void Render_StagePanel();

private:
	ENTRY_STATE m_eEntryState = ENTRY_STATE::IDLE;
	LostArk::Shared::WORLD_ID m_ePendingWorldId =
		LostArk::Shared::WORLD_ID::END;
	LEVEL m_ePendingLevel = LEVEL::END;
	std::chrono::steady_clock::time_point m_ApprovalDeadline{};
	string m_strStatus =
		"Choose a stage directly or open Character Select to change class.";

public:
	static unique_ptr<CLevel_Lobby> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### 7-2. `C:/Users/user/Desktop/LostArk/Client/Private/Level_Lobby.cpp`

변경 종류: 전체 교체

```cpp
#include "imgui.h"

#include "Level_Lobby.h"

#include "CharacterSelectionState.h"
#include "LevelTransitionService.h"
#include "NetworkManager.h"

namespace
{
	constexpr char_t DEFAULT_SERVER_HOST[] = "127.0.0.1";
	constexpr char_t SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	constexpr std::uint16_t SERVER_PORT = 7777;
	constexpr char_t PLAYER_NICKNAME[] = "Player";
	constexpr LostArk::Shared::CHARACTER_CLASS_ID DEFAULT_ENTRY_CLASS =
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;

	string Resolve_ServerHost()
	{
		char_t configuredHost[64]{};
		const DWORD configuredLength = ::GetEnvironmentVariableA(
			SERVER_HOST_ENVIRONMENT,
			configuredHost,
			static_cast<DWORD>(std::size(configuredHost)));
		if (0 == configuredLength ||
			configuredLength >= std::size(configuredHost) ||
			"0.0.0.0" == string_view{ configuredHost })
		{
			return DEFAULT_SERVER_HOST;
		}

		return configuredHost;
	}

	string Describe_ServerEndpoint()
	{
		return Resolve_ServerHost() + ":" + to_string(SERVER_PORT);
	}

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		default:
			return "Not selected";
		}
	}

	bool_t Resolve_EntryCharacterClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass,
		bool_t& outUsedDefault)
	{
		outUsedDefault = false;
		if (CCharacterSelectionState::Try_Get_SelectedClass(
			outCharacterClass))
		{
			return true;
		}

		if (!CCharacterSelectionState::Select(DEFAULT_ENTRY_CLASS))
		{
			outCharacterClass =
				LostArk::Shared::CHARACTER_CLASS_ID::END;
			return false;
		}

		outCharacterClass = DEFAULT_ENTRY_CLASS;
		outUsedDefault = true;
		return true;
	}
}

CLevel_Lobby::CLevel_Lobby(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Lobby::~CLevel_Lobby()
{
}

HRESULT CLevel_Lobby::Initialize()
{
	return __super::Initialize();
}

void CLevel_Lobby::Update(const f32_t fTimeDelta)
{
	LOBBY_COMMAND command{};
	if (CLobbyCommandService::Try_Consume(command))
		Begin_StageRequest(command.eStage);

	Consume_EnterAccepted();

	if (ENTRY_STATE::WAITING_FOR_APPROVAL == m_eEntryState)
	{
		if (!CNetworkManager::Get().Is_Connected())
		{
			Cancel_PendingEntry(
				"Server disconnected before approving entry. Lobby remains active.");
		}
		else if (std::chrono::steady_clock::now() >= m_ApprovalDeadline)
		{
			Cancel_PendingEntry(
				"Server entry approval timed out after 5 seconds. Lobby remains active.");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Lobby::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	Render_StagePanel();
	return S_OK;
}

bool_t CLevel_Lobby::Begin_StageRequest(const LOBBY_STAGE eStage)
{
	if (ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "Another entry or level transition is already pending.";
		return false;
	}

	if (LOBBY_STAGE::CHARACTER_SELECT == eStage)
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::CHARACTER_SELECT,
			"lobby.character-select"))
		{
			m_strStatus = CLevelTransitionService::Get_Status();
			return false;
		}

		m_strStatus = "Opening Character Select.";
		return true;
	}

	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	LEVEL targetLevel = LEVEL::END;
	if (!Resolve_Stage(eStage, worldId, targetLevel))
	{
		m_strStatus = "The selected stage is not registered.";
		return false;
	}

	return Begin_NetworkEntry(worldId, targetLevel);
}

bool_t CLevel_Lobby::Begin_NetworkEntry(
	const LostArk::Shared::WORLD_ID eWorldId,
	const LEVEL eTargetLevel)
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool_t usedDefaultClass = false;
	if (!Resolve_EntryCharacterClass(characterClass, usedDefaultClass))
	{
		m_strStatus = "The default entry class could not be committed.";
		return false;
	}

	CNetworkManager& networkManager = CNetworkManager::Get();
	networkManager.Close_ServerConnection();
	const string serverHost = Resolve_ServerHost();
	if (!networkManager.Connect_To_Server(serverHost, SERVER_PORT))
	{
		m_strStatus = "Server connection failed for " +
			serverHost + ":" + to_string(SERVER_PORT) + " (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		return false;
	}

	if (!networkManager.Send_EnterWorld(
		eWorldId,
		characterClass,
		PLAYER_NICKNAME))
	{
		m_strStatus = "C2S_ENTER_WORLD send failed (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		networkManager.Close_ServerConnection();
		return false;
	}

	m_eEntryState = ENTRY_STATE::WAITING_FOR_APPROVAL;
	m_ePendingWorldId = eWorldId;
	m_ePendingLevel = eTargetLevel;
	m_ApprovalDeadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_strStatus = usedDefaultClass ?
		"No class was selected. Lance Master was committed and entry approval is pending." :
		"C2S_ENTER_WORLD sent. Waiting for server approval.";
	return true;
}

bool_t CLevel_Lobby::Resolve_Stage(
	const LOBBY_STAGE eStage,
	LostArk::Shared::WORLD_ID& outWorldId,
	LEVEL& outTargetLevel) const
{
	using LostArk::Shared::WORLD_ID;
	outWorldId = WORLD_ID::END;
	outTargetLevel = LEVEL::END;

	switch (eStage)
	{
	case LOBBY_STAGE::TEST:
		outWorldId = WORLD_ID::TRAINING_GROUND;
		outTargetLevel = LEVEL::DEVELOPMENT;
		return true;
	case LOBBY_STAGE::VALTAN:
		outWorldId = WORLD_ID::VALTAN_ARENA;
		outTargetLevel = LEVEL::VALTAN_ARENA;
		return true;
	case LOBBY_STAGE::BERN:
		outWorldId = WORLD_ID::BERN;
		outTargetLevel = LEVEL::BERN;
		return true;
	default:
		return false;
	}
}

void CLevel_Lobby::Consume_EnterAccepted()
{
	LostArk::Shared::S2C_ENTER_ACCEPTED accepted{};
	CNetworkManager& networkManager = CNetworkManager::Get();
	if (!networkManager.Try_Consume_EnterAccepted(accepted))
		return;

	if (ENTRY_STATE::WAITING_FOR_APPROVAL != m_eEntryState)
	{
		Cancel_PendingEntry("Unexpected server approval was rejected.");
		return;
	}

	if (accepted.eWorldId != m_ePendingWorldId)
	{
		Cancel_PendingEntry("Server approved a different world. Entry was rejected.");
		return;
	}

	const LEVEL approvedLevel = m_ePendingLevel;
	if (!CLevelTransitionService::Request_Load(
		approvedLevel,
		"lobby.enter-accepted"))
	{
		Cancel_PendingEntry(CLevelTransitionService::Get_Status());
		return;
	}

	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ApprovalDeadline = {};
	m_strStatus = "Server approved the world. Loading the stage.";
}

void CLevel_Lobby::Cancel_PendingEntry(const string& reason)
{
	CNetworkManager::Get().Close_ServerConnection();
	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ApprovalDeadline = {};
	m_strStatus = reason;
}

void CLevel_Lobby::Render_StagePanel()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_Always);
	}

	if (!ImGui::Begin(
		"LostArk Lobby",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	LostArk::Shared::CHARACTER_CLASS_ID selectedClass =
		DEFAULT_ENTRY_CLASS;
	const bool_t hasExplicitSelection =
		CCharacterSelectionState::Try_Get_SelectedClass(selectedClass);
	ImGui::Text(
		"Entry character: %s%s",
		Get_CharacterClassName(selectedClass),
		hasExplicitSelection ? "" : " (default)");
	const string serverEndpoint = Describe_ServerEndpoint();
	ImGui::TextDisabled("Server: %s", serverEndpoint.c_str());
	ImGui::Separator();

	const bool_t isBusy = ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Test"))
		CLobbyCommandService::Request(LOBBY_STAGE::TEST);
	ImGui::SameLine();
	if (ImGui::Button("Character Select"))
		CLobbyCommandService::Request(LOBBY_STAGE::CHARACTER_SELECT);
	ImGui::SameLine();
	if (ImGui::Button("Valtan"))
		CLobbyCommandService::Request(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Bern"))
		CLobbyCommandService::Request(LOBBY_STAGE::BERN);
	ImGui::EndDisabled();

	if (!hasExplicitSelection)
	{
		ImGui::TextDisabled(
			"Direct entry commits Lance Master. Character Select changes it.");
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

unique_ptr<CLevel_Lobby> CLevel_Lobby::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_Lobby>(
		new CLevel_Lobby(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
```

## 8. G4 — 기존 네 class Q/W 데이터와 DimensionMaster 스킬 미승격 경계

현재 `PlayerSkills.json`에는 Lance Master 9개와 Gunslinger/Slayer/Artist Q/W가 있다. 다섯 class 모두 Character Select, Server profile, spawn, HUD class identity, 이동과 IDLE/RUN을 사용한다. 검증된 Q/W는 `LANCE_MASTER 34120/34080`, `GUNSLINGER 38020/38050`, `SLAYER 45050/45060`, `ARTIST 31210/31230`이다. DimensionMaster reference 문서는 정식 헤더와 0 row만 있으므로 실제 stable skill ID와 timing을 확보할 때까지 skill row를 만들지 않으며 Lance Master skill로 대체하지 않는다.

추가 Q/W의 skill ID와 animation chain은 다음 실측 정본을 사용한다.

```text
GUNSLINGER Q 38020  Data/Animation/Reference/GunSlinger/GunSlinger.clipseq
GUNSLINGER W 38050  Data/Animation/Reference/GunSlinger/GunSlinger.clipseq
SLAYER      Q 45050  Data/Animation/Reference/Slayer/Slayer.clipseq
SLAYER      W 45060  Data/Animation/Reference/Slayer/Slayer.clipseq
ARTIST      Q 31210  Data/Animation/Reference/Artist/Artist.clipseq
ARTIST      W 31230  Data/Animation/Reference/Artist/Artist.clipseq
```

damage/resource/range는 원작 밸런스라고 주장하지 않는 수련장 baseline이다. stable class/skill identity와 추출 animation은 실제 값이고, 수치는 `Data/Balance`에서 담당자가 튜닝한다.

### 8-1. `C:/Users/user/Desktop/LostArk/Data/Balance/DamageProfiles.json`

변경 종류: 전체 교체

```json
{
  "schema": "lostark.damage-profiles",
  "formatVersion": 1,
  "profiles": [
    { "damageProfileId": "damage.player.34120", "amount": 650 },
    { "damageProfileId": "damage.player.34080", "amount": 700 },
    { "damageProfileId": "damage.player.34070", "amount": 750 },
    { "damageProfileId": "damage.player.34150", "amount": 1000 },
    { "damageProfileId": "damage.player.34110", "amount": 850 },
    { "damageProfileId": "damage.player.34090", "amount": 650 },
    { "damageProfileId": "damage.player.34640", "amount": 1650 },
    { "damageProfileId": "damage.player.34600", "amount": 2500 },
    { "damageProfileId": "damage.player.34620", "amount": 2500 },
    { "damageProfileId": "damage.player.38020", "amount": 600 },
    { "damageProfileId": "damage.player.38050", "amount": 800 },
    { "damageProfileId": "damage.player.45050", "amount": 750 },
    { "damageProfileId": "damage.player.45060", "amount": 650 },
    { "damageProfileId": "damage.player.31210", "amount": 550 },
    { "damageProfileId": "damage.player.31230", "amount": 700 },
    { "damageProfileId": "damage.valtan.basic-swing", "amount": 350 }
  ]
}
```

### 8-2. `C:/Users/user/Desktop/LostArk/Data/Balance/PlayerSkills.json`

변경 종류: 전체 교체

```json
{
  "schema": "lostark.player-skills",
  "formatVersion": 1,
  "skills": [
    {
      "skillId": 34120,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "Q",
      "displayName": "연환섬",
      "actionId": "lancemaster.skill.34120",
      "cooldownMs": 10000,
      "actionDurationMs": 2266,
      "hitTimeMs": 1295,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 7.3,
      "serverDamageProfileId": "damage.player.34120"
    },
    {
      "skillId": 34080,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "W",
      "displayName": "일섬각",
      "actionId": "lancemaster.skill.34080",
      "cooldownMs": 12000,
      "actionDurationMs": 1366,
      "hitTimeMs": 0,
      "resourceCost": 17,
      "movementDistance": 0.0,
      "maximumRange": 4.0,
      "serverDamageProfileId": "damage.player.34080"
    },
    {
      "skillId": 34070,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "E",
      "displayName": "회선창",
      "actionId": "lancemaster.skill.34070",
      "cooldownMs": 14000,
      "actionDurationMs": 2000,
      "hitTimeMs": 300,
      "resourceCost": 19,
      "movementDistance": 0.0,
      "maximumRange": 9.9,
      "serverDamageProfileId": "damage.player.34070"
    },
    {
      "skillId": 34150,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "R",
      "displayName": "맹룡열파",
      "actionId": "lancemaster.skill.34150",
      "cooldownMs": 24000,
      "actionDurationMs": 2266,
      "hitTimeMs": 1345,
      "resourceCost": 29,
      "movementDistance": 0.0,
      "maximumRange": 9.2,
      "serverDamageProfileId": "damage.player.34150"
    },
    {
      "skillId": 34110,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "A",
      "displayName": "반월섬",
      "actionId": "lancemaster.skill.34110",
      "cooldownMs": 18000,
      "actionDurationMs": 3200,
      "hitTimeMs": 1857,
      "resourceCost": 23,
      "movementDistance": 0.0,
      "maximumRange": 9.9,
      "serverDamageProfileId": "damage.player.34110"
    },
    {
      "skillId": 34090,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "S",
      "displayName": "철량추",
      "actionId": "lancemaster.skill.34090",
      "cooldownMs": 10000,
      "actionDurationMs": 2100,
      "hitTimeMs": 1730,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 9.2,
      "serverDamageProfileId": "damage.player.34090"
    },
    {
      "skillId": 34640,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "T",
      "displayName": "맹룡난무",
      "actionId": "lancemaster.skill.34640",
      "cooldownMs": 50000,
      "actionDurationMs": 3166,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 10.6,
      "serverDamageProfileId": "damage.player.34640"
    },
    {
      "skillId": 34600,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "V",
      "displayName": "은하유성탄",
      "actionId": "lancemaster.skill.34600",
      "cooldownMs": 300000,
      "actionDurationMs": 5600,
      "hitTimeMs": 1390,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 12.2,
      "serverDamageProfileId": "damage.player.34600"
    },
    {
      "skillId": 34620,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "ALT_V",
      "displayName": "은하비섬창",
      "actionId": "lancemaster.skill.34620",
      "cooldownMs": 300000,
      "actionDurationMs": 6300,
      "hitTimeMs": 1930,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 18.2,
      "serverDamageProfileId": "damage.player.34620"
    },
    {
      "skillId": 38020,
      "characterClass": "GUNSLINGER",
      "inputSlot": "Q",
      "displayName": "퀵 스텝",
      "actionId": "gunslinger.skill.38020",
      "cooldownMs": 10000,
      "actionDurationMs": 3533,
      "hitTimeMs": 1000,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 5.8,
      "serverDamageProfileId": "damage.player.38020"
    },
    {
      "skillId": 38050,
      "characterClass": "GUNSLINGER",
      "inputSlot": "W",
      "displayName": "심판의 시간",
      "actionId": "gunslinger.skill.38050",
      "cooldownMs": 30000,
      "actionDurationMs": 1767,
      "hitTimeMs": 1000,
      "resourceCost": 25,
      "movementDistance": 0.0,
      "maximumRange": 4.0,
      "serverDamageProfileId": "damage.player.38050"
    },
    {
      "skillId": 45050,
      "characterClass": "SLAYER",
      "inputSlot": "Q",
      "displayName": "퓨리 블레이드",
      "actionId": "slayer.skill.45050",
      "cooldownMs": 12000,
      "actionDurationMs": 4533,
      "hitTimeMs": 1100,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 3.5,
      "serverDamageProfileId": "damage.player.45050"
    },
    {
      "skillId": 45060,
      "characterClass": "SLAYER",
      "inputSlot": "W",
      "displayName": "와일드 러시",
      "actionId": "slayer.skill.45060",
      "cooldownMs": 14000,
      "actionDurationMs": 4600,
      "hitTimeMs": 500,
      "resourceCost": 18,
      "movementDistance": 0.0,
      "maximumRange": 5.0,
      "serverDamageProfileId": "damage.player.45060"
    },
    {
      "skillId": 31210,
      "characterClass": "ARTIST",
      "inputSlot": "Q",
      "displayName": "필법 : 콩콩이",
      "actionId": "artist.skill.31210",
      "cooldownMs": 16000,
      "actionDurationMs": 5334,
      "hitTimeMs": 1445,
      "resourceCost": 18,
      "movementDistance": 0.0,
      "maximumRange": 2.5,
      "serverDamageProfileId": "damage.player.31210"
    },
    {
      "skillId": 31230,
      "characterClass": "ARTIST",
      "inputSlot": "W",
      "displayName": "묵법 : 옹달샘",
      "actionId": "artist.skill.31230",
      "cooldownMs": 24000,
      "actionDurationMs": 1500,
      "hitTimeMs": 1445,
      "resourceCost": 22,
      "movementDistance": 0.0,
      "maximumRange": 6.5,
      "serverDamageProfileId": "damage.player.31230"
    }
  ]
}
```

`Publish-GameplayBalance.ps1 -Mode Publish`가 이 두 JSON과 기존 player/boss profiles를 검증한 뒤 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`을 다시 생성한다. 생성물은 직접 편집하지 않는다.

### 8-3. `C:/Users/user/Desktop/LostArk/Server/Private/ServerGameplayContractTests.cpp`

변경 종류: 표준 include 추가

```cpp
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
```

변경 종류: `catalog.Load()` 직후의 기존 단일 Lance Master skill resolve assertion을 아래 블록으로 교체

```cpp
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");

	struct EXPECTED_QUICK_SKILL final
	{
		SKILL_ID iSkillId;
		CHARACTER_CLASS_ID eCharacterClass;
		const char* pInputSlot;
		const char* pName;
	};
	constexpr std::array EXPECTED_QUICK_SKILLS =
	{
		EXPECTED_QUICK_SKILL{
			34120, CHARACTER_CLASS_ID::LANCE_MASTER, "Q", "Lance Master Q" },
		EXPECTED_QUICK_SKILL{
			34080, CHARACTER_CLASS_ID::LANCE_MASTER, "W", "Lance Master W" },
		EXPECTED_QUICK_SKILL{
			38020, CHARACTER_CLASS_ID::GUNSLINGER, "Q", "Gunslinger Q" },
		EXPECTED_QUICK_SKILL{
			38050, CHARACTER_CLASS_ID::GUNSLINGER, "W", "Gunslinger W" },
		EXPECTED_QUICK_SKILL{
			45050, CHARACTER_CLASS_ID::SLAYER, "Q", "Slayer Q" },
		EXPECTED_QUICK_SKILL{
			45060, CHARACTER_CLASS_ID::SLAYER, "W", "Slayer W" },
		EXPECTED_QUICK_SKILL{
			31210, CHARACTER_CLASS_ID::ARTIST, "Q", "Artist Q" },
		EXPECTED_QUICK_SKILL{
			31230, CHARACTER_CLASS_ID::ARTIST, "W", "Artist W" }
	};
	for (const EXPECTED_QUICK_SKILL& expected : EXPECTED_QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(expected.iSkillId);
		const std::string resolveName =
			std::string("Resolve ") + expected.pName + " skill binding";
		tests.Require(
			nullptr != skill &&
			expected.eCharacterClass == skill->eCharacterClass &&
			skill->strInputSlot == expected.pInputSlot,
			resolveName.c_str());

		SERVER_PLAYER probePlayer{};
		probePlayer.eCharacterClass = expected.eCharacterClass;
		probePlayer.iCurrentResource = 100;
		probePlayer.iMaximumResource = 100;
		C2S_USE_SKILL probeCommand{};
		probeCommand.iClientSequence = 1;
		probeCommand.iSkillId = expected.iSkillId;
		probeCommand.fAimX = 0.f;
		probeCommand.fAimZ = 1.f;
		CPlayerSkillSystem probeSystem;
		const std::string approvalName =
			std::string("Approve ") + expected.pName +
			" through server skill authority";
		tests.Require(
			probeSystem.Try_Start(
				probePlayer,
				probeCommand,
				catalog,
				10),
			approvalName.c_str());
	}
```

기존 player profile, navigation, damage-once, cooldown, Valtan brain assertions는 이 블록 뒤에 그대로 둔다. 이 교체는 기존 test를 삭제하지 않는다.

다음 기존 단일 runtime 경로를 검증한다.

```text
Character Select Enter
-> CCharacterSelectionState::Select
-> CLobbyCommandService::Request(TEST)
-> Lobby Begin_NetworkEntry
-> CNetworkManager::Send_EnterWorld(TRAINING_GROUND, selected class)
-> ServerApp::On_SessionFrame
-> CGameRoom::Join
-> S2C_ENTER_ACCEPTED + S2C_PLAYER_SPAWNED
-> DEVELOPMENT Loader가 Get_LocalCharacterClass()로 selected class prototype 준비
-> CClientReplication::Create_Character
-> CNetObjectRegistry local handle
-> CCombatHUDViewModel class-specific player/skill rows
-> CPlayerController
-> right click C2S_MOVE / Q,W C2S_USE_SKILL
-> GameRoom 30 Hz movement/skill update
-> S2C_WORLD_SNAPSHOT
-> Character locomotion/action presentation + HUD cooldown
```

G4에서 수정하지 않는 C++ 파일:

```text
Shared/*
Server/Public/GameRoom.h
Server/Private/GameRoom.cpp
Client/Public/Level_Development.h
Client/Private/Level_Development.cpp
Client/Public/ClientReplication.h
Client/Private/ClientReplication.cpp
Client/Public/PlayerController.h
Client/Private/PlayerController.cpp
Client/Public/CombatHUDViewModel.h
Client/Private/CombatHUDViewModel.cpp
```

generic `CPlayerSkillCatalog`, `CGameplayCatalog`, `CPlayerSkillSystem`, `CCharacter::Play_Skill`은 이미 class와 stable skill ID를 데이터로 소비하므로 새 switch를 추가하지 않는다. 이 파일 중 하나가 runtime 검증에서 실패할 때만 최초 실패 원인을 재현하고 별도 diff로 최소 수정한다. PLAN에 미리 중복 Manager나 placeholder를 추가하지 않는다.

## 9. G5 — token service 단위 harness와 integration ProjectAudit

### 9-1. `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

변경 종류: 새 파일 전체

```cpp
#include "LobbyCommandService.h"

#include <cstddef>
#include <iostream>
#include <string>

namespace
{
	struct TEST_RUNNER final
	{
		void Require(const bool_t condition, const char_t* pName)
		{
			if (condition)
			{
				std::cout << "[PASS] " << pName << '\n';
				return;
			}

			++iFailureCount;
			std::cout << "[FAILURE] " << pName << '\n';
		}

		std::size_t iFailureCount = 0;
	};

	void Require_NoPendingCommand(
		TEST_RUNNER& runner,
		const char_t* pName)
	{
		Client::LOBBY_COMMAND command{};
		runner.Require(
			!Client::CLobbyCommandService::Try_Consume(command),
			pName);
	}

	void Test_NormalHandoff(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Normal Handoff Stages Tokenized Test Command");

		LOBBY_COMMAND_TOKEN duplicateToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(
				LOBBY_STAGE::BERN,
				duplicateToken) &&
			INVALID_LOBBY_COMMAND_TOKEN == duplicateToken,
			"Duplicate Enter Does Not Replace Pending Command");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::TEST == command.eStage &&
			token == command.iToken,
			"Lobby Consumes Exact Handoff Command Once");
		Require_NoPendingCommand(
			runner,
			"Consumed Handoff Leaves No Stale Command");
	}

	void Test_ExactCancellation(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token),
			"Cancellation Fixture Stages Test Command");
		runner.Require(
			CLobbyCommandService::Cancel(token, "handoff owner failed"),
			"Exact Token Cancels Pending Command");
		Require_NoPendingCommand(
			runner,
			"Exact Cancellation Leaves No Stale Command");
	}

	void Test_StaleTokenCannotCancelNewCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN oldToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, oldToken) &&
			CLobbyCommandService::Cancel(oldToken, "old handoff cancelled"),
			"Old Handoff Is Cancelled");

		LOBBY_COMMAND_TOKEN newToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::BERN, newToken) &&
			newToken > oldToken,
			"New Handoff Receives New Token");
		runner.Require(
			!CLobbyCommandService::Cancel(oldToken, "stale failure"),
			"Stale Failure Cannot Cancel New Handoff");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::BERN == command.eStage &&
			newToken == command.iToken,
			"New Handoff Survives Stale Cancellation");
	}

	void Test_InvalidRequestsPreservePendingCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(LOBBY_STAGE::END, token) &&
			INVALID_LOBBY_COMMAND_TOKEN == token,
			"Invalid Stage Is Rejected");

		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::VALTAN, token),
			"Valid Command Is Staged After Invalid Stage");
		runner.Require(
			!CLobbyCommandService::Cancel(
				INVALID_LOBBY_COMMAND_TOKEN,
				"invalid token"),
			"Invalid Token Is Rejected");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::VALTAN == command.eStage &&
			token == command.iToken,
			"Invalid Cancellation Preserves Pending Command");
	}
}

int main()
{
	TEST_RUNNER runner{};

	Test_NormalHandoff(runner);
	Test_ExactCancellation(runner);
	Test_StaleTokenCannotCancelNewCommand(runner);
	Test_InvalidRequestsPreservePendingCommand(runner);

	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
```

### 9-2. `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`

변경 종류: 새 파일 전체

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{78406C04-3D55-4F36-B6D1-B5180A48F521}</ProjectGuid>
    <RootNamespace>ClientFrontendHarness</RootNamespace>
    <ProjectName>ClientFrontendHarness</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)..\Bin\$(Configuration)\</OutDir>
    <IntDir>$(ProjectDir)..\Intermediate\$(Platform)\$(Configuration)\</IntDir>
    <TargetName>ClientFrontendHarness</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Client\Public;$(ProjectDir)..\..\..\EngineSDK\Inc;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Client\Public;$(ProjectDir)..\..\..\EngineSDK\Inc;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\ClientFrontendHarness.cpp" />
    <ClCompile Include="..\..\..\Client\Private\LobbyCommandService.cpp">
      <Link>Client\LobbyCommandService.cpp</Link>
    </ClCompile>
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
```

### 9-3. `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`

변경 종류: 새 파일 전체

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Private">
      <UniqueIdentifier>{1FE078EF-A16A-4934-8D2C-A6949CCB5BAC}</UniqueIdentifier>
    </Filter>
    <Filter Include="Client">
      <UniqueIdentifier>{BC0E9108-26E5-43E7-9D73-73A11EB40BFC}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\ClientFrontendHarness.cpp">
      <Filter>Private</Filter>
    </ClCompile>
    <ClCompile Include="..\..\..\Client\Private\LobbyCommandService.cpp">
      <Filter>Client</Filter>
    </ClCompile>
  </ItemGroup>
</Project>
```

### 9-4. `C:/Users/user/Desktop/LostArk/Framework.sln`

변경 종류: project block 추가

```text
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "ClientFrontendHarness", "Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj", "{78406C04-3D55-4F36-B6D1-B5180A48F521}"
EndProject
```

변경 종류: `ProjectConfigurationPlatforms`에 추가

```text
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x64.ActiveCfg = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x64.Build.0 = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x86.ActiveCfg = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x64.ActiveCfg = Release|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x64.Build.0 = Release|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x86.ActiveCfg = Release|x64
```

### 9-5. `C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1`

변경 종류: build 목록에서 NetworkProtocolHarness 다음에 추가

```powershell
        Invoke-MSBuildProject $msbuild `
            'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
```

변경 종류: protocol harness 실행 다음에 추가

```powershell
    $frontendHarness = Join-Path $repoRoot `
        "Tools\ClientFrontendHarness\Bin\$Configuration\ClientFrontendHarness.exe"
    & $frontendHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'ClientFrontendHarness failed.'
    }
```

### 9-6. `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

변경 종류: 기존 `maps.training-area-contract` 바로 다음에 아래 블록 추가

```powershell
	$characterSelectArea = @($mapCatalog.areas |
		Where-Object id -eq 'LV_LOBBY_CLASSSELECT_SL00')
	$characterSelectAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectRuntimeRoot = if ($characterSelectArea.Count -eq 1) {
		[string]$characterSelectArea[0].runtimeAssetRoot
	} else { '' }
	$characterSelectManifestPath = if ([string]::IsNullOrWhiteSpace($characterSelectRuntimeRoot)) {
		''
	} else {
		Join-Path 'Client\Bin\Resources' `
			(Join-Path $characterSelectRuntimeRoot 'map_asset_runtime_manifest.json')
	}
	$characterSelectManifest = if (-not [string]::IsNullOrWhiteSpace($characterSelectManifestPath) -and
		(Test-Path -LiteralPath $characterSelectManifestPath)) {
		Read-Json $characterSelectManifestPath
	} else { $null }
	Add-Check 'maps.character-select-area-contract' (
		$characterSelectArea.Count -eq 1 -and
		$characterSelectArea[0].kind -eq 'product' -and
		$characterSelectArea[0].catalogType -eq 'single' -and
		$characterSelectArea[0].assetCount -eq 55 -and
		$characterSelectArea[0].placementCount -eq 803 -and
		$characterSelectAssetRows -eq 55 -and
		$characterSelectPlacementRows -eq 803 -and
		$null -ne $characterSelectManifest -and
		$characterSelectManifest.areaId -eq 'LV_LOBBY_CLASSSELECT_SL00' -and
		$characterSelectManifest.assetCount -eq 55 -and
		@($characterSelectManifest.assets).Count -eq 55) "assets=$characterSelectAssetRows placements=$characterSelectPlacementRows manifest=$($characterSelectManifest.assetCount)"
```

변경 종류: 기존 `levels.character-select-contract` Add-Check 전체 교체

```powershell
	$characterSelectLoaderFunction = [regex]::Match(
		$loaderSource,
		'HRESULT CLoader::Ready_For_CharacterSelect\(\)[\s\S]*?(?=HRESULT CLoader::Ready_For_Bern\(\))').Value
	$lobbyCommandHeaderSource = Get-Content -LiteralPath 'Client\Public\LobbyCommandService.h' -Raw
	$lobbyCommandSource = Get-Content -LiteralPath 'Client\Private\LobbyCommandService.cpp' -Raw
	$transitionHeaderSource = Get-Content -LiteralPath 'Client\Public\LevelTransitionService.h' -Raw
	$loadingSource = Get-Content -LiteralPath 'Client\Private\Level_Loading.cpp' -Raw
	$frontendHarnessSource = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp' -Raw
	$frontendHarnessProject = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj' -Raw
	$buildRegressionSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw

	Add-Check 'levels.character-select-contract' (
		$levelRegistrySource -match 'LEVEL::CHARACTER_SELECT' -and
		$levelRegistrySource -match 'LV_LOBBY_CLASSSELECT_SL00' -and
		$levelRegistrySource -match '\{ true, true, -792\.f, 158\.f, -750\.f, 218\.f \}' -and
		$levelRegistrySource -match 'Ready_For_CharacterSelect' -and
		$characterSelectLoaderFunction -match 'Ready_MapArea\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_Camera_Prototype\(' -and
		$characterSelectLoaderFunction -match 'Ready_Character_Rendering\(' -and
		$characterSelectLoaderFunction -match 'Ready_AnimationPreviewModels\(' -and
		$characterSelectLoaderFunction -match 'CHARACTER_CLASS_ID::LANCE_MASTER' -and
		$characterSelectLoaderFunction -match 'CHARACTER_CLASS_ID::GUNSLINGER' -and
		$characterSelectLoaderFunction -match 'CHARACTER_CLASS_ID::SLAYER' -and
		$characterSelectLoaderFunction -match 'CHARACTER_CLASS_ID::ARTIST' -and
		$characterSelectSource -match 'm_MapRuntime\.Load_Area' -and
		$characterSelectSource -match 'CCharacterSelectionState::Select' -and
		$characterSelectSource -match 'CLobbyCommandService::Request\([\s\S]{0,120}LOBBY_STAGE::TEST[\s\S]{0,120}commandToken' -and
		$characterSelectSource -match 'character-select\.enter-test' -and
		$characterSelectSource -match 'ImGuiKey_Enter' -and
		$characterSelectSource -notmatch 'NetworkManager|Connect_To_Server|Send_EnterWorld' -and
		$lobbySource -match 'DEFAULT_ENTRY_CLASS' -and
		$lobbySource -match 'Resolve_EntryCharacterClass' -and
		$lobbySource -match 'Send_EnterWorld\(' -and
		$lobbySource -match '"Test"' -and
		$lobbySource -match '"Character Select"' -and
		$lobbySource -match '"Valtan"' -and
		$lobbySource -match '"Bern"') 'Character Select loads one map path and Enter reuses the Lobby server-authorized Test path'

	Add-Check 'levels.character-select-handoff-ticket' (
		$lobbyCommandHeaderSource -match 'LOBBY_COMMAND_TOKEN' -and
		$lobbyCommandHeaderSource -match 'Cancel\(' -and
		$lobbyCommandSource -match 'g_iNextToken' -and
		$lobbyCommandSource -match 'g_PendingCommand->iToken != token' -and
		$transitionHeaderSource -match 'iLobbyCommandToken' -and
		$loadingSource -match 'Request_Activation\([\s\S]{0,160}m_iLobbyCommandToken' -and
		$loadingSource -match 'Cancel_LobbyCommand\("target level loading failed"\)' -and
		$mainAppSource -match 'target level loading could not start' -and
		$mainAppSource -match 'target level activation failed' -and
		$frontendHarnessSource -match 'Exact Cancellation Leaves No Stale Command' -and
		$frontendHarnessSource -match 'Stale Failure Cannot Cancel New Handoff' -and
		$frontendHarnessProject -match 'Client\\Private\\LobbyCommandService\.cpp' -and
		$buildRegressionSource -match 'ClientFrontendHarness') 'token service has an executable unit harness and integration failure hooks are statically admitted'

	$playerSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	foreach ($className in @('LANCE_MASTER','GUNSLINGER','SLAYER','ARTIST')) {
		foreach ($slotName in @('Q','W')) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			if ($bindings.Count -ne 1) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0) "missing=$($missingQuickSlots -join ',')"

	$quickSkillAnimationContracts = @(
		[pscustomobject]@{ Class = 'LANCE_MASTER'; Asset = 'LanceMaster'; Skills = @(34120, 34080) },
		[pscustomobject]@{ Class = 'GUNSLINGER'; Asset = 'GunSlinger'; Skills = @(38020, 38050) },
		[pscustomobject]@{ Class = 'SLAYER'; Asset = 'Slayer'; Skills = @(45050, 45060) },
		[pscustomobject]@{ Class = 'ARTIST'; Asset = 'Artist'; Skills = @(31210, 31230) }
	)
	$quickSkillAnimationErrors = [Collections.Generic.List[string]]::new()
	foreach ($contract in $quickSkillAnimationContracts) {
		$sequencePath = "Data\Animation\Reference\$($contract.Asset)\$($contract.Asset).clipseq"
		$clipMapPath = "Data\Animation\Reference\$($contract.Asset)\$($contract.Asset).clipmap"
		if (-not (Test-Path -LiteralPath $sequencePath) -or
			-not (Test-Path -LiteralPath $clipMapPath)) {
			$quickSkillAnimationErrors.Add("$($contract.Class):missing animation document")
			continue
		}
		$sequenceSource = Get-Content -LiteralPath $sequencePath -Raw
		$clipMapSource = Get-Content -LiteralPath $clipMapPath -Raw
		foreach ($skillId in $contract.Skills) {
			if ($sequenceSource -notmatch "(?m)^$skillId\s") {
				$quickSkillAnimationErrors.Add("$($contract.Class):$skillId missing clipseq")
			}
			if ($clipMapSource -notmatch "skill=$skillId(?:\s|$)") {
				$quickSkillAnimationErrors.Add("$($contract.Class):$skillId missing clipmap")
			}
		}
	}
	$characterRuntimeSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	Add-Check 'gameplay.playable-qw-animation-contract' (
		$quickSkillAnimationErrors.Count -eq 0 -and
		$characterRuntimeSource -match 'Load_ClipChains\(\)' -and
		$characterRuntimeSource -match 'filesystem::path\(assetName \+ "\.clipseq"\)' -and
		$characterRuntimeSource -match 'Play_Skill\(static_cast<int32_t>\(skillId\)\)') "errors=$($quickSkillAnimationErrors -join ',')"
```

## 10. G6 — public 문서와 RESULT의 정확한 교체 문단

### 10-1. `C:/Users/user/Desktop/LostArk/AGENTS.md`

변경 종류: `고정 런타임 계약`의 Character Select 문단 전체 교체

```markdown
- `CHARACTER_SELECT`는 socket을 직접 열지 않는 3D 선택 Level이다. Lobby에서 진입하면 Loader가 `LV_LOBBY_CLASSSELECT_SL00` visual map과 다섯 playable class를 준비하고, Level은 같은 map 위에 preview Character와 ImGui 선택 panel을 만든다. Enter는 선택 class를 commit하고 tokenized `TEST` command를 예약한 뒤 Lobby로 돌아가 기존 `C2S_ENTER_WORLD -> S2C_ENTER_ACCEPTED` 경로를 재사용한다. token은 Lobby load 시작, Loader, activation 실패에서 같은 command만 취소하므로 재시도 때 stale 자동 진입이 없다. Character Select를 거치지 않은 직접 Test/Bern/Valtan 진입은 `LANCE_MASTER`를 명시적 process selection으로 commit한다. Server 승인 전에는 제품 Level로 전환하지 않으며, 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남고 진입 후 disconnect는 replicated state를 정리하고 Lobby로 복귀한다. Preview Character를 실제 player로 승격하거나 Character Select에서 packet/socket을 직접 호출하는 우회 경로는 존재하지 않는다.
```

변경 종류: Character/Animation 담당 bullet 전체 교체

```markdown
- 검증된 기존 네 playable class의 Q/W는 각각 `LANCE_MASTER 34120/34080`, `GUNSLINGER 38020/38050`, `SLAYER 45050/45060`, `ARTIST 31210/31230`으로 `Data/Balance/PlayerSkills.json`에 연결한다. 입력은 `(class, inputSlot)`으로 stable skill ID를 찾고 command → server approval → snapshot → Character presentation을 통과한다. 수치는 수련장 baseline이며 원작 수치라고 주장하지 않는다. DimensionMaster는 선택·spawn·HUD class identity·IDLE/RUN·Animation Tool 모델 열람까지만 이번 계약이며, 스킬 ID와 timing row가 0이므로 Q/W를 활성화하지 않는다.
```

### 10-2. `C:/Users/user/Desktop/LostArk/CLAUDE.md`

변경 종류: `최소 수련장 Area`의 첫 문단 전체 교체

```markdown
`dev.training.ground`는 새 Engine Level이 아니라 기존 `LEVEL::DEVELOPMENT`를 사용하는 Test 진입이다. map area는 `LV_DEV_TRAINING_GROUND`, world ID는 `TRAINING_GROUND`다. `LEVEL::CHARACTER_SELECT` 진입 Loader는 `LV_LOBBY_CLASSSELECT_SL00` visual map과 다섯 class preview resource를 준비하지만 socket은 열지 않는다. class 선택 후 Enter는 tokenized `LOBBY_STAGE::TEST`를 예약하고 Lobby의 기존 Server 승인 state machine을 거쳐 DEVELOPMENT로 진입한다. Lobby load 또는 activation 실패 시 같은 token의 command를 취소한다. Character Select를 거치지 않고 Test/Bern/Valtan을 누르면 `LANCE_MASTER`를 process selection으로 commit한 뒤 같은 Server 승인 경로를 사용한다. Client host는 process-local `LOSTARK_SERVER_HOST`를 읽으며 값이 없거나 `0.0.0.0`이면 `127.0.0.1`을 사용한다. 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남고, 진입 후 연결이 끊기면 replicated state를 제거한 뒤 Lobby로 복귀한다. Local Preview나 자동 offline gameplay 우회 경로는 없다.
```

변경 종류: runtime validation bullet 전체 교체

```markdown
- runtime validation: `Framework.slnLaunch`로 실제 Server와 Client를 함께 실행하고 Lobby → Loading → Character Select map/preview → Enter → Lobby approval → Test 진입, Lobby direct Test/Bern/Valtan 진입, disconnect 복귀를 확인
```

### 10-3. `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

변경 종류: Lobby/Character Select 계약 문단 전체 교체

```markdown
Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 명령만 제공한다. Character Select는 socket 없는 3D preview Level이며 Loader가 `LV_LOBBY_CLASSSELECT_SL00` visual map과 다섯 playable class를 준비한다. Enter는 선택 class를 commit하고 tokenized Lobby `TEST` command를 예약한다. load/activation 실패는 같은 token만 취소하며 실제 Test 진입은 Lobby의 `C2S_ENTER_WORLD -> S2C_ENTER_ACCEPTED` 경로를 그대로 통과한다. Character Select를 생략한 direct entry는 `LANCE_MASTER`를 명시적 process selection으로 commit한다. Test/Bern/Valtan은 실제 Server 승인을 받은 뒤에만 진입하고, 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남으며 진입 후 disconnect는 replicated state를 정리하고 Lobby로 복귀한다. Preview Character를 actual player로 승격하거나 UI/Character Select가 packet과 socket을 직접 호출하는 경로는 금지한다.
```

변경 종류: 최소 성공 증거의 Character Select bullet 전체 교체

```markdown
- 실제 Server+Client에서 Lobby → Character Select map/다섯 class preview → Enter → Test 진입과 다섯 class Character/HUD/이동을 확인한다. Q/W는 기존 네 class만 확인하며 DimensionMaster는 skill data가 생기기 전 비활성임을 확인한다.
- Character Select를 생략한 direct Test/Bern/Valtan entry와 연결 실패/timeout/진입 후 disconnect 복귀 확인
```

변경 종류: class quick-slot 표를 아래 행으로 교체

```markdown
| Class | Q | W | 수치 정본 |
|---|---:|---:|---|
| Lance Master | `34120` | `34080` | `Data/Balance/PlayerSkills.json` |
| Gunslinger | `38020` | `38050` | `Data/Balance/PlayerSkills.json` |
| Slayer | `45050` | `45060` | `Data/Balance/PlayerSkills.json` |
| Artist | `31210` | `31230` | `Data/Balance/PlayerSkills.json` |
```

### 10-4. `C:/Users/user/Desktop/LostArk/.md/GB/08-03/2026-08-03_CHARACTER_SELECT_LEVEL_VERTICAL_SLICE_RESULT.md`

변경 종류: 구현 시작 전에 맨 아래 추가

```markdown
## 11. 2026-08-04 후속 수직 슬라이스 상태

계획서 정본은 같은 이름의 PLAN으로 전면 갱신했다. 새 범위는 Character Select visual map admission, Enter-to-Test token handoff, direct Lance Master entry, 다섯 class 선택/입장, 기존 네 playable class Q/W balance 연결과 ClientFrontendHarness다.

현재 상태는 계획 완료, 구현 전이다. 이 절을 추가한 시점에는 새 범위의 C++/balance/harness를 반영하지 않았고 build, 자동 harness, Server+Client 수동 진입도 실행하지 않았다. 기존 2026-08-03 결과와 새 목표를 혼동해 완료 처리하지 않는다.
```

구현 후에는 위 절 아래에 실제 실행한 Debug/Release build, `ClientFrontendHarness failures : 0`, ProjectAudit, resource `Hydrate -> Verify`, 다섯 class runtime 결과와 미검증 항목을 사실대로 추가한다. 실행하지 않은 검증을 성공으로 미리 적지 않는다.

## 12. 프로젝트 등록

```text
Client.vcxproj 변경 없음
Client.vcxproj.filters 변경 없음
DataFiles filter 변경 없음
Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp 신규
Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj 신규
Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters 신규
Framework.sln project/config 등록
Tools/Build/Invoke-BuildAndRegression.ps1 build/run 등록
```

현재 다른 세션의 `Client.vcxproj`, `.filters`, mapassets 변경을 이 작업에서 재정렬하거나 덮어쓰지 않는다.

## 13. 적용 순서

1. `MapCatalog.json`의 해당 Area와 `LevelRegistry.cpp` descriptor를 교체한다.
2. `Loader.cpp::Ready_For_CharacterSelect`를 map → class → animation preview 순서로 교체한다. `Ready_MapArea`가 camera prototype을 준비하므로 다시 호출하지 않는다.
3. `LobbyCommandService`, `LevelTransitionService`, `Level_Loading`, `MainApp`에 token ownership과 failure cancellation을 반영한다.
4. `Level_CharacterSelect.h/.cpp`, `Level_Lobby.h/.cpp`를 최종 코드로 교체한다.
5. 기존 네 class Q/W balance와 damage profile을 publish하고 DimensionMaster skill row는 비어 있는지 검사한다.
6. ClientFrontendHarness project/source와 build regression을 등록한다.
7. 현재 dirty `ProjectAudit`의 unrelated 변경을 보존하며 지정 audit 블록만 병합한다.
8. AGENTS/CLAUDE/팀 handbook/RESULT의 지정 문단만 교체한다.
9. 다른 세션의 Character/Effect/Map intake가 끝난 같은 Resources를 새 immutable pack으로 Snapshot/Publish한다.
10. 새 pack을 Hydrate/Verify하고 diff·build·audit·runtime 순으로 검증한다.

## 14. 빌드·자동 검증 명령

```powershell
Set-Location C:\Users\user\Desktop\LostArk

if ([string]::IsNullOrWhiteSpace($env:LOSTARK_PACK_ROOT)) {
	throw 'LOSTARK_PACK_ROOT must point to the external immutable pack root.'
}
$assetContractStatus = @(
	git status --short -- 'Data/AssetPacks.lock.json' 'Data/AssetManifests')
if ($assetContractStatus.Count -ne 0) {
	$assetContractStatus | ForEach-Object { Write-Host $_ }
}
$deletedImmutableManifests = @(
	git diff --name-only --diff-filter=D -- 'Data/AssetManifests/*.manifest.json'
	git diff --cached --name-only --diff-filter=D -- 'Data/AssetManifests/*.manifest.json') |
	Sort-Object -Unique
if ($deletedImmutableManifests.Count -ne 0) {
	throw "Resolve immutable manifest deletions before Snapshot: $($deletedImmutableManifests -join ', ')"
}
$dirtyAssetLock = @(
	git diff --name-only -- 'Data/AssetPacks.lock.json'
	git diff --cached --name-only -- 'Data/AssetPacks.lock.json'
	git ls-files --others --exclude-standard -- 'Data/AssetPacks.lock.json') |
	Sort-Object -Unique
if ($dirtyAssetLock.Count -ne 0) {
	throw 'The asset lock has an unresolved owner diff. Commit or restore that diff before Snapshot.'
}
$packVersion = '2026.08.04.1'
$packRoot = [IO.Path]::GetFullPath($env:LOSTARK_PACK_ROOT)
$zipPath = Join-Path $packRoot "lostark-resources-$packVersion.zip"
if (Test-Path -LiteralPath $zipPath) {
	throw "Immutable ZIP already exists: $zipPath"
}
powershell -ExecutionPolicy Bypass -File .\Tools\AssetPipeline\Manage-ResourcePack.ps1 `
	-Mode Snapshot -Version $packVersion
powershell -ExecutionPolicy Bypass -File .\Tools\AssetPipeline\Manage-ResourcePack.ps1 `
	-Mode Verify
powershell -ExecutionPolicy Bypass -File .\Tools\AssetPipeline\Manage-ResourcePack.ps1 `
	-Mode Publish -PackRoot $packRoot
tar.exe -a -cf $zipPath -C $packRoot "lostark-resources/$packVersion"
Get-FileHash -LiteralPath $zipPath -Algorithm SHA256
powershell -ExecutionPolicy Bypass -File .\Tools\AssetPipeline\Manage-ResourcePack.ps1 `
	-Mode Hydrate -PackRoot $packRoot
powershell -ExecutionPolicy Bypass -File .\Tools\AssetPipeline\Manage-ResourcePack.ps1 `
	-Mode Verify

powershell -ExecutionPolicy Bypass -File .\Tools\GameplayPipeline\Publish-GameplayBalance.ps1 `
	-Mode Validate
powershell -ExecutionPolicy Bypass -File .\Tools\GameplayPipeline\Publish-GameplayBalance.ps1 `
	-Mode Publish

powershell -ExecutionPolicy Bypass -File .\Tools\Build\Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File .\Tools\Build\Invoke-BuildAndRegression.ps1 -Configuration Release

.\Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
.\Tools\ClientFrontendHarness\Bin\Debug\ClientFrontendHarness.exe
.\Tools\ClientFrontendHarness\Bin\Release\ClientFrontendHarness.exe
.\Server\Bin\Debug\Server.exe --contract-test

powershell -ExecutionPolicy Bypass -File .\Tools\ProjectAudit\Invoke-ProjectAudit.ps1 -DeepAssetHash

git diff --check
```

실행 경로가 현재 산출물 구조와 다르면 `CLAUDE.md`의 정본 출력 경로를 먼저 확인하고 같은 binary를 중복 실행하지 않는다.

## 15. 필수 런타임 검증

### 15.1 Character Select resource/preview

```text
1. Server + Client 실행
2. Client가 Lobby에서 시작
3. Character Select 클릭
4. Loading text에 CHARACTER SELECT: visual map 표시
5. LV_LOBBY_CLASSSELECT_SL00 main cluster가 보임
6. Lance Master/Gunslinger/Slayer/Artist 선택 시 이전 preview를 잃지 않고 교체
7. Back은 이전 committed class를 바꾸지 않음
```

### 15.2 Enter-to-Test

```text
1. 각 class를 선택
2. Enter 키 또는 Enter Test 버튼
3. Lobby가 TEST command를 한 번만 소비
4. C2S_ENTER_WORLD world=TRAINING_GROUND, class=선택 class
5. 승인 전 DEVELOPMENT로 가지 않음
6. S2C_ENTER_ACCEPTED 후 DEVELOPMENT Loading
7. 실제 local Character가 선택 class
8. HUD class 이름/HP/resource/skill row가 선택 class
9. 우클릭 이동이 Server snapshot으로 반영
10. Q/W가 C2S_USE_SKILL -> Server approval -> action/cooldown snapshot으로 반영
```

### 15.3 Character Select 생략

```text
1. 새 Client process에서 Character Select를 열지 않음
2. Lobby의 Test 클릭 가능
3. LANCE_MASTER가 process selection으로 commit
4. Server 승인 후 DEVELOPMENT 진입
5. Bern/Valtan도 같은 default selection으로 승인 가능
```

### 15.4 실패와 rollback

```text
Character Select mapassets/placement/model 하나 누락
-> Loader FAILED
-> CHARACTER_SELECT Level resource rollback
-> 부분 map/preview commit 없음

preview class 교체 실패
-> 이전 preview 유지

Enter 시 Lobby command 예약 후 Level request 실패
-> 방금 발급한 token의 TEST command 취소
-> 나중에 Lobby에서 의도치 않은 자동 입장 없음

Lobby Loader 실패 또는 Lobby activation 실패
-> ownership 중인 token의 TEST command 취소
-> Retry Lobby 뒤 stale 자동 입장 없음

오래된 failure callback이 더 새 command token을 취소 시도
-> token mismatch로 거부
-> 새 command 유지

Server 없음/connection 거부/5초 timeout/world mismatch
-> Lobby 유지
-> socket close
-> 제품 Level 전환 없음

진입 후 disconnect
-> replication/HUD reset
-> Lobby 복귀
```

## 16. 이번 계획의 완료 판정

```text
구현 완료:
- 위 코드가 실제 파일에 반영되고 build 성공

자동 검증 완료:
- Debug/Release build regression 성공
- Protocol Harness failures 0
- ClientFrontendHarness Debug/Release failures 0
- Server contract failures 0
- ProjectAudit 전 항목 성공
- immutable resource pack Hydrate/Verify와 DeepAssetHash 성공
- git diff --check 성공

수동 검증 완료:
- Character Select map이 실제로 보임
- 다섯 class preview 교체
- Enter와 버튼 모두 Test 진입
- selected class actual Character/HUD 일치
- 이동/Q/W server snapshot 반영
- direct default entry
- failure/timeout/disconnect rollback

후속 별도 수직 슬라이스:
- MapTool에서 수련장 navigation paint/project/publish
- trainingTarget stable kind와 Server damage contract
- 잡몹 catalog/spawn trigger/runtime
- 추가 boss placement/brain/encounter
- 제품 CUIObject Character Select/HUD layout
```

문서에 코드가 있다는 사실은 구현 완료가 아니다. Character Select map은 추출·catalog·manifest까지 존재하지만 이 PLAN 작성 시점에는 인게임 표시가 검증되지 않았다.
