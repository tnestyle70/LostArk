# NPC 배치 툴 뼈대 — PLAN

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool`

베른성(및 다른 Area)에 NPC를 배치하는 MapTool 확장이다. 새 툴을 만들지 않고
기존 `TOOL_MODE::WORLD_GAMEPLAY`의 NPC placement 경로를 다중 archetype으로 일반화한다.

## 0. 실측 결과 (2026-08-12 기준)

- NPC 4종 `.wmodel`이 `Client/Bin/Resources/Character/NPC/{Npc_Aylara,Npc_Beda,Npc_Forman,Npc_Schmidt}/`에 존재.
  전부 바디+머리 합본, 클립 접두 `npc_`, 4종 모두 `npc_idle_normal_1` 보유
  (`.md/JS/2026-08-02_LOSTARK_NPC_GROUP_RESOLUTION_RESULT.md`).
- `Data/Actors/NpcCatalog.json`: `NPC_BEDA` 1종만 등록. 파서(`ActorCatalog.cpp::ParseNpcs`)는
  entry당 정확히 5필드, `runtimeStatus=="supported"` 강제, archetype/presentation 중복 거부.
- `Publish-WorldGameplay.ps1::Get-ActorIds`가 npc placement의 archetype을 NpcCatalog
  supported 목록으로 검증한다. **catalog에 추가만 하면 publisher는 자동 통과.**
- Server는 archetype 문자열 passthrough (`WorldBootstrap.cpp`, `GameRoom.cpp::To_NetworkKind`).
  Server 코드 수정 불필요.
- Client 하드코딩 2곳이 실제 차단 지점:
  - `NpcPresentationAssetService.cpp`: `SUPPORTED_ARCHETYPE="NPC_BEDA"`,
    presentation id 및 모델 태그 `Prototype_Component_Model_Npc_Beda` 고정.
  - `ClientReplication.cpp:409-426`: beda presentation id 검사 + 모델 태그 하드코딩.
- MapTool `Render_WorldGameplayPanel` 986-993행: NPC kind 선택 시 `NPC_BEDA` 강제 입력.
  Arm→픽킹 클릭→배치(`Try_PlaceWorldGameplay`)와 placement 테이블/편집/삭제는 완성 상태.
- 에디터 프리젠테이션 패턴: `TRIGGER_BOX_ENTRY` + `Stage_WorldTriggerBoxes`/
  `Remove_WorldTriggerBoxes` 스테이지-스왑. Development 에디터 레벨에
  `Prototype_Component_Shader_VtxAnimMeshBinary` 등록됨(`Loader.cpp:382`).
- `CNpc::NPC_DESC`(모델 태그·클립·위치·Yaw)와 `Set_Animation(clip, loop)`이 이미 툴
  재사용을 전제로 설계되어 있음. `CModel::Get_NumAnimations()/Get_AnimationName(i)`로 클립 열거 가능.

## 1. 범위

| G | 내용 | 소비자 |
|---|---|---|
| G1 | NpcCatalog 4종 등록 + `CActorCatalog::Get_Npcs()` 열람 API | MapTool 콤보, publisher |
| G2 | `CNpcPresentationAssetService` 다중 archetype 일반화 | `CClientReplication`(제품 Bern 스폰), MapTool 미리보기 |
| G3 | MapTool: NPC archetype 콤보, 배치 시 실제 모델 미리보기, 선택 placement 클립 미리보기 | 에디터 사용자 |

이번에 안 하는 것 (다음 수직 슬라이스):
- **per-placement 애니메이션 저장** — 현재 저장 계약은 catalog `idleClip`(archetype당 1개).
  placement별 클립은 Gameplay.world.json schema 확장 + Client가 snapshot entity를 placement로
  역매핑하는 계약이 필요하고, Server에 클립을 알리지 않는 경계를 지켜야 한다.
- **머리/몸통 분리 커스텀** — 현재 쿠킹이 합본을 만들고 `CNpc`는 단일 모델 전제.
  분리 파츠는 cook 파이프라인과 `CNpc` 파츠 지원이 같이 필요하다.
- 미리보기 NPC의 표시/숨김 토글(`CNpc`에 authoring visible 개념 없음), NpcCatalog를 툴에서
  저장하는 writer.

## 2. G1 — NpcCatalog 4종 + 열람 API

### 2.1 `Data/Actors/NpcCatalog.json` — 전체 교체

```json
{
  "schema": "lostark.npc-catalog",
  "formatVersion": 1,
  "npcs": [
    {
      "archetypeId": "NPC_BEDA",
      "clientPresentationId": "npc.beda.client.v1",
      "modelAssetId": "Character/NPC/Npc_Beda/Npc_Beda.wmodel",
      "idleClip": "npc_idle_normal_1",
      "runtimeStatus": "supported"
    },
    {
      "archetypeId": "NPC_AYLARA",
      "clientPresentationId": "npc.aylara.client.v1",
      "modelAssetId": "Character/NPC/Npc_Aylara/Npc_Aylara.wmodel",
      "idleClip": "npc_idle_normal_1",
      "runtimeStatus": "supported"
    },
    {
      "archetypeId": "NPC_FORMAN",
      "clientPresentationId": "npc.forman.client.v1",
      "modelAssetId": "Character/NPC/Npc_Forman/Npc_Forman.wmodel",
      "idleClip": "npc_idle_normal_1",
      "runtimeStatus": "supported"
    },
    {
      "archetypeId": "NPC_SCHMIDT",
      "clientPresentationId": "npc.schmidt.client.v1",
      "modelAssetId": "Character/NPC/Npc_Schmidt/Npc_Schmidt.wmodel",
      "idleClip": "npc_idle_normal_1",
      "runtimeStatus": "supported"
    }
  ]
}
```

### 2.2 `Client/Public/ActorCatalog.h`

작업: 추가 · 기준점: `static const NPC_ACTOR_ENTRY* Find_Npc(std::string_view archetypeId);`
바로 아래 · 대상: 함수 선언 · 정의: `ActorCatalog.cpp`의 `Find_Npc` 정의 바로 뒤 ·
이유: MapTool NPC 콤보가 supported archetype 전체를 열거해야 한다 ·
연결: `CMapTool::Render_WorldGameplayPanel`.

```cpp
	static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();
```

### 2.3 `Client/Private/ActorCatalog.cpp`

`Find_Npc` 정의(353-361행) 바로 뒤에 추가. Initialize 실패 시 빈 vector가 반환되어
호출자는 "등록된 NPC 없음"으로 자연 처리된다.

```cpp
const std::vector<Client::NPC_ACTOR_ENTRY>& Client::CActorCatalog::Get_Npcs()
{
	Initialize();
	return g_Npcs;
}
```

## 3. G2 — NpcPresentationAssetService 다중 archetype

모델 프로토타입 태그는 `Prototype_Component_Model_` + modelAssetId 파일 stem으로
파생한다. Beda의 stem이 `Npc_Beda`이므로 기존 태그
`Prototype_Component_Model_Npc_Beda`와 정확히 일치해 회귀가 없다.
`Prototype_GameObject_Npc`는 레벨당 1회만 등록한다(2번째 archetype에서 중복 Add 방지).

### 3.1 `Client/Public/NpcPresentationAssetService.h` — 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string_view>

NS_BEGIN(Client)

class CNpcPresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId);
	static bool_t Is_Ready(
		uint32_t iLevelIndex,
		std::string_view archetypeId);

	/* Stable per-archetype CModel prototype tag, derived from the catalog
	modelAssetId stem. Empty when the archetype is not in the catalog. */
	static wstring_t Get_ModelPrototypeTag(std::string_view archetypeId);
};

NS_END
```

### 3.2 `Client/Private/NpcPresentationAssetService.cpp` — 전체 코드

```cpp
#include "NpcPresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	std::mutex g_NpcAssetMutex;
	/* level index -> archetypes whose model prototypes are committed there */
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyArchetypes;
	/* levels that already committed the shared CNpc GameObject prototype */
	std::unordered_set<uint32_t> g_NpcObjectReadyLevels;

	wstring_t Derive_ModelTag(const std::string& modelAssetId)
	{
		const std::filesystem::path assetPath(modelAssetId);
		const std::wstring stem = assetPath.stem().wstring();
		if (stem.empty())
			return {};
		return wstring_t(TEXT("Prototype_Component_Model_")) + stem;
	}
}

void Client::CNpcPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	g_ReadyArchetypes.erase(iLevelIndex);
	g_NpcObjectReadyLevels.erase(iLevelIndex);
}

HRESULT Client::CNpcPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) || archetypeId.empty())
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_NpcAssetMutex };
	const auto readyLevel = g_ReadyArchetypes.find(iLevelIndex);
	if (readyLevel != g_ReadyArchetypes.end() &&
		readyLevel->second.contains(archetypeId))
	{
		return S_FALSE;
	}

	const NPC_ACTOR_ENTRY* actor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == actor || actor->runtimeStatus != "supported" ||
		actor->clientPresentationId.empty())
	{
		return E_FAIL;
	}
	const wstring_t modelTag = Derive_ModelTag(actor->modelAssetId);
	if (modelTag.empty())
		return E_FAIL;
	const std::filesystem::path modelPath =
		CRuntimeAssetRoot::Resolve(actor->modelAssetId);
	if (modelPath.empty())
		return E_FAIL;

	const matrix_t preTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));
	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(2u);
	staged.emplace_back(
		modelTag,
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			modelPath.string().c_str(),
			preTransform));
	const bool_t needsObjectPrototype =
		!g_NpcObjectReadyLevels.contains(iLevelIndex);
	if (needsObjectPrototype)
	{
		staged.emplace_back(
			TEXT("Prototype_GameObject_Npc"),
			CNpc::Create(pDevice, pContext));
	}
	for (const auto& [tag, prototype] : staged)
	{
		(void)tag;
		if (nullptr == prototype)
			return E_FAIL;
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(
		iLevelIndex, std::move(staged))))
	{
		return E_FAIL;
	}

	g_ReadyArchetypes[iLevelIndex].insert(std::string(archetypeId));
	if (needsObjectPrototype)
		g_NpcObjectReadyLevels.insert(iLevelIndex);
	return S_OK;
}

bool_t Client::CNpcPresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	const auto readyLevel = g_ReadyArchetypes.find(iLevelIndex);
	return readyLevel != g_ReadyArchetypes.end() &&
		readyLevel->second.contains(archetypeId);
}

wstring_t Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	const NPC_ACTOR_ENTRY* actor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == actor)
		return {};
	return Derive_ModelTag(actor->modelAssetId);
}
```

### 3.3 `Client/Private/ClientReplication.cpp` — NPC 분기 교체

기준점: 409행 `if (WORLD_ENTITY_KIND::NPC == spawned.eKind)` 블록 앞부분.
beda presentation id 검사를 제거하고(검증은 서비스가 수행) 모델 태그를 서비스에서 얻는다.
아래 블록이 409-426행(`desc.strShaderTag` 직전까지)을 교체한다.

```cpp
	if (WORLD_ENTITY_KIND::NPC == spawned.eKind)
	{
		const NPC_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Npc(spawned.strArchetypeId);
		const wstring_t modelTag =
			CNpcPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		if (nullptr == actor || modelTag.empty() ||
			FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice,
				m_Desc.pContext,
				m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			return false;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
```

이후 `desc.strShaderTag = ...`부터의 기존 코드는 그대로 유지한다.

## 4. G3 — MapTool: 콤보 + 미리보기 + 클립 선택

### 4.1 `Client/Public/MapTool.h`

1. 전방 선언 추가 — 기준점: `class CTrigger_Box;`(30행) 바로 아래:

```cpp
class CNpc;
```

2. struct 추가 — 기준점: `TRIGGER_BOX_ENTRY` struct 닫는 `};`(122행) 바로 아래:

```cpp
	struct NPC_PREVIEW_ENTRY
	{
		std::string placementId;
		std::string archetypeId;
		shared_ptr<CNpc> object;
	};
```

3. 함수 선언 추가 — 기준점: `void Remove_WorldTriggerBoxes(vector<TRIGGER_BOX_ENTRY>& entries);`(269행) 바로 아래:

```cpp
	bool_t Stage_WorldNpcPreviews(
		const CWorldGameplayDocument& document,
		vector<NPC_PREVIEW_ENTRY>& outEntries);
	void Remove_WorldNpcPreviews(vector<NPC_PREVIEW_ENTRY>& entries);
	void Sync_WorldNpcPreviews();
```

4. 멤버 추가 — 기준점: `vector<TRIGGER_BOX_ENTRY> m_WorldTriggerBoxes;`(370행) 바로 아래:

```cpp
	vector<NPC_PREVIEW_ENTRY> m_WorldNpcPreviews;
	int32_t m_iWorldNpcArchetypeIndex = 0;
```

### 4.2 `Client/Private/MapTool.cpp`

include 추가(기존 include 블록 말미): `ActorCatalog.h`, `Npc.h`,
`NpcPresentationAssetService.h`, `Model.h` 중 미포함분.

1. **NPC kind UI 교체** — 986-993행의 하드코딩 블록을 catalog 콤보로 교체:

```cpp
		else if (WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind)
		{
			const std::vector<NPC_ACTOR_ENTRY>& npcs = CActorCatalog::Get_Npcs();
			if (npcs.empty())
			{
				m_WorldArchetypeId[0] = '\0';
				ImGui::TextDisabled(
					"NpcCatalog has no supported archetype.");
			}
			else
			{
				if (m_iWorldNpcArchetypeIndex < 0 ||
					m_iWorldNpcArchetypeIndex >=
						static_cast<int32_t>(npcs.size()))
				{
					m_iWorldNpcArchetypeIndex = 0;
				}
				if (ImGui::BeginCombo("NPC Archetype",
					npcs[m_iWorldNpcArchetypeIndex].archetypeId.c_str()))
				{
					for (int32_t index = 0;
						index < static_cast<int32_t>(npcs.size()); ++index)
					{
						const bool_t isSelected =
							index == m_iWorldNpcArchetypeIndex;
						if (ImGui::Selectable(
							npcs[index].archetypeId.c_str(), isSelected))
							m_iWorldNpcArchetypeIndex = index;
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				strcpy_s(m_WorldArchetypeId,
					npcs[m_iWorldNpcArchetypeIndex].archetypeId.c_str());
				ImGui::TextDisabled(
					"Archetypes come from Data/Actors/NpcCatalog.json (runtimeStatus=supported).");
			}
			m_WorldEncounterId[0] = '\0';
		}
```

2. **미리보기 스테이지/제거/동기화** — `Remove_WorldTriggerBoxes` 정의 바로 뒤에 세 함수 추가.
   미리보기는 표현 전용이므로 잘못된 placement 하나가 전체 로드를 막지 않도록 per-placement로
   격리하고 상태 문자열에만 남긴다(문서 조작의 rollback 대상이 아니다).

```cpp
bool_t Client::CMapTool::Stage_WorldNpcPreviews(
	const CWorldGameplayDocument& document,
	vector<NPC_PREVIEW_ENTRY>& outEntries)
{
	outEntries.clear();
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		return false;
	}

	size_t skippedCount = 0;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind)
			continue;

		const NPC_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Npc(placement.archetypeId);
		const wstring_t modelTag =
			CNpcPresentationAssetService::Get_ModelPrototypeTag(
				placement.archetypeId);
		if (nullptr == actor || modelTag.empty() ||
			FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
				m_pDevice, m_pContext,
				m_iAuthoringLevelIndex, placement.archetypeId)))
		{
			++skippedCount;
			continue;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_iAuthoringLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pIdleClip = actor->idleClip.c_str();
		desc.vPosition = placement.position;
		desc.fYawDegree = placement.yawDegrees;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_Npc"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_NpcPreviews"),
			&desc,
			&gameObject)))
		{
			++skippedCount;
			continue;
		}
		shared_ptr<CNpc> npc = dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == npc)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_NpcPreviews"),
				gameObject);
			++skippedCount;
			continue;
		}
		outEntries.push_back(
			{ placement.placementId, placement.archetypeId, std::move(npc) });
	}
	if (0 < skippedCount)
	{
		m_WorldGameplayStatus =
			"NPC preview skipped " + std::to_string(skippedCount) +
			" placement(s): unknown archetype or asset admission failed";
	}
	return true;
}

void Client::CMapTool::Remove_WorldNpcPreviews(
	vector<NPC_PREVIEW_ENTRY>& entries)
{
	for (const NPC_PREVIEW_ENTRY& entry : entries)
	{
		if (nullptr != entry.object &&
			m_iAuthoringLevelIndex < ETOUI(LEVEL::END))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_NpcPreviews"),
				static_pointer_cast<CGameObject>(entry.object));
		}
	}
	entries.clear();
}

void Client::CMapTool::Sync_WorldNpcPreviews()
{
	vector<NPC_PREVIEW_ENTRY> staged;
	if (!Stage_WorldNpcPreviews(m_WorldGameplayDocument, staged))
		return;
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_WorldNpcPreviews = std::move(staged);
}
```

3. **동기화 호출 지점** — 트리거 박스 스테이지-스왑이 성공하는 지점마다 뒤에 1줄 추가.

| 위치 | 기준점 |
|---|---|
| placement 편집 commit | 1329행 `m_WorldTriggerBoxes = std::move(stagedBoxes);` 다음 |
| placement 삭제 | 1357행 동일 패턴 다음 |
| `Load_WorldGameplay` 성공 | 1904행 `m_WorldTriggerBoxes = std::move(stagedBoxes);` 다음 |
| `Try_PlaceWorldGameplay` 성공 | 2126행 동일 패턴 다음 |
| workspace Area 로드 | 3082행 스테이지 성공 스왑 다음 |

호출 코드는 모두 `Sync_WorldNpcPreviews();` 1줄이다. 트리거 박스를 제거하는 teardown
지점(`Remove_WorldTriggerBoxes(m_WorldTriggerBoxes)`가 문서를 버리는 자리)에는
`Remove_WorldNpcPreviews(m_WorldNpcPreviews);`를 짝으로 추가한다.

4. **선택 placement 클립 미리보기** — 1313-1316행 `else { Enabled 체크박스 }` 분기를
   NPC 전용 분기로 확장:

```cpp
		else if (WORLD_PLACEMENT_KIND::NPC == staged.eKind)
		{
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
			const auto preview = std::find_if(
				m_WorldNpcPreviews.begin(), m_WorldNpcPreviews.end(),
				[&](const NPC_PREVIEW_ENTRY& entry)
				{
					return entry.placementId == staged.placementId;
				});
			if (m_WorldNpcPreviews.end() != preview &&
				nullptr != preview->object &&
				nullptr != preview->object->Get_Model())
			{
				ImGui::SeparatorText("NPC Preview Animation");
				const shared_ptr<CModel> model = preview->object->Get_Model();
				const uint32_t clipCount = model->Get_NumAnimations();
				const char_t* currentClip =
					model->Get_AnimationName(model->Get_CurrentAnimIndex());
				if (ImGui::BeginCombo("Preview Clip",
					nullptr != currentClip ? currentClip : "<none>"))
				{
					for (uint32_t clipIndex = 0;
						clipIndex < clipCount; ++clipIndex)
					{
						const char_t* clipName =
							model->Get_AnimationName(clipIndex);
						if (nullptr == clipName)
							continue;
						const bool_t isSelected =
							clipIndex == model->Get_CurrentAnimIndex();
						if (ImGui::Selectable(clipName, isSelected))
							preview->object->Set_Animation(clipName, true);
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::TextDisabled(
					"Preview only. The product idle clip stays in NpcCatalog.json idleClip.");
			}
		}
```

## 5. 검증

1. `git diff --check`, Client x64 Debug 빌드 오류 0.
2. Debug 실행 → Lobby `Test` → F1 Map Tool → Bern Area 로드 → World Gameplay 모드:
   - NPC kind에서 4종 콤보 확인, 각 archetype 배치 → 즉시 해당 모델이 클릭 위치에 표시.
   - placement 선택 → Preview Clip 콤보에서 클립 변경 → 미리보기 재생 확인.
   - 삭제/Reload 시 미리보기 정리 확인. Save → `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`.
3. `Publish-WorldGameplay.ps1` 실행 → 새 archetype placement가 bootstrap 생성 통과.
4. Server+Client 로컬 루프백 → Bern 진입 → 배치한 NPC들이 제품 경로(snapshot)로 표시.
5. `Server.exe --contract-test`, `NetworkProtocolHarness`, ProjectAudit 회귀 없음 확인.

## 6. 완료 후 RESULT에 기록할 것

- 구현/자동 검증/수동 검증 분리.
- 미지원으로 남긴 항목(1장) 재확인.
