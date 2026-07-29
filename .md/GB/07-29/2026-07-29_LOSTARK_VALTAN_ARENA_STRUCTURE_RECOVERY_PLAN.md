# LostArk 발탄 코어 자동 조립 및 간단 맵 마무리 계획

작성일: 2026-07-29
최종 범위: `Floor01`, `Floor01A`, `Floor01B`, `Statue01`의 원본 공통 좌표계를 보존하고 MapTool에서 한 번에 원자적으로 생성한다. 일반 에셋의 연속 배치는 유지한다. Navigation, StageData, 외곽 구조물 역추적은 별도 작업으로 남긴다.

## 1. C1~C8 관점

| 관점 | 결정 |
|---|---|
| C1 기준계 | 네 코어 조각은 glTF 검증에서 원점·회전 0·스케일 1로 자연스럽게 결합됐다. 따라서 개별 바운딩 박스 기준 `BottomCenter` 보정을 제거하고 source-space `Origin`을 사용한다. |
| C2 이동·계산 | 자동 조립은 네 조각 모두 동일한 position `(0,0,0)`, rotation `(0,0,0)`, catalog scale `(1,1,1)`을 사용한다. 런타임에서 버텍스를 재베이크하거나 Blender 결과물을 합치지 않는다. |
| C3 공유·비정형 | 기존 `Prototype_GameObject_MapAsset -> CMapAssetObject -> CModel -> CMaterial` 경로만 사용한다. 네 조각은 독립 객체로 유지해 이후 파괴 상태·재질·가시성을 따로 제어할 수 있게 한다. |
| C4 수명·선언 | Catalog가 앵커 계약을 소유하고 `.mapplacements`가 생성된 네 placement ID와 Transform을 소유한다. 자동 조립 중 생성한 객체는 임시 staging 목록에 두고 전부 성공한 뒤에만 편집기 목록으로 commit한다. |
| C5 내삽·절차 | `Assemble Valtan Core`는 기존 코어 조각이 하나라도 있으면 중복 생성을 거부한다. 일반 `Arm placement`는 한 번 배치한 뒤에도 유지되어 같은 바닥·벽 조각을 연속 배치할 수 있다. |
| C6 가지치기 | 180도 복제, 내부 0.5배 복제, 메시 병합, StageData 배치 복구, Navigation 베이크는 현재 근거가 없거나 다음 단계이므로 구현하지 않는다. |
| C7 권위·정합성 | Blender/UModel 결과는 네 glTF의 버텍스가 공통 source-space 좌표를 보존한다는 검증 근거로만 사용한다. 런타임 정본은 기존 `.wmodel` 네 개와 catalog asset ID다. |
| C8 검증 | Catalog 네 행의 `Origin`, 중복 방지, 4개 전부 성공/실패 rollback, Save/Reload 계약, Client x64 Debug 빌드를 완료 조건으로 둔다. |

## 2. 문제 해결 ①~⑤

① 문제·제약
원본 네 조각은 같은 좌표계에 놓으면 자동으로 결합되지만 MapTool의 `BottomCenter`는 각 모델의 로컬 바운딩 박스 중심과 최저점을 별도로 계산해 world origin을 이동시킨다. 서로 다른 바운딩 박스를 가진 네 조각은 같은 Transform을 주어도 서로 다른 보정량을 받아 도넛처럼 벌어졌다. 사용자가 위치·회전·스케일을 손으로 맞출 문제가 아니었다.

② 단순 해법이 만드는 문제
Blender에서 합쳐 하나의 메시로 베이크하면 지금 당장 모양은 맞지만 네 조각의 독립 재질, 가시성, 파괴 연출 단위를 잃는다. 네 개를 수동 배치하면 공통 좌표를 다시 깨뜨리기 쉽고 저장 데이터도 반복해서 오염된다. 기존 코어 조각 위에 자동 조립을 허용하면 겹침과 중복 렌더링이 생긴다.

③ 해결 방식
Catalog에서 코어 네 행만 `Origin`으로 바꾼다. MapTool에 안정적인 네 asset ID 배열과 `Assemble_ValtanCore` 명령을 추가한다. 명령은 catalog 준비, 기존 코어 배치 부재, placement 한도, 네 asset 존재, 네 앵커가 모두 `Origin`인지 먼저 검증한다. 이후 네 객체를 임시로 생성하고 하나라도 실패하면 임시 객체를 전부 Layer에서 제거한다. 네 개가 모두 생성됐을 때만 `m_Placements`, next placement ID, dirty 상태를 갱신한다.

④ 비교
원본 메시와 통합 런타임을 그대로 사용하므로 새 binary asset 경로가 생기지 않는다. 사용자 조립 오차가 없고 Save/Reload 형식도 바뀌지 않는다. 나머지 13개 에셋은 계속 `BottomCenter`를 사용하므로 일반 소품의 바닥 배치 편의도 보존한다.

⑤ 대가
현재 저장된 `Floor01*` 또는 `Statue01` 수동 배치가 있으면 버튼이 의도적으로 동작하지 않는다. Hierarchy에서 해당 조각만 삭제하거나, 다른 배치를 보존할 필요가 없을 때 `Clear`한 뒤 조립해야 한다. 자동 조립은 world origin에 생성되며 네 조각을 하나의 그룹처럼 함께 이동하는 기능은 이번 범위에 없다.

## 3. 자료구조·알고리즘 핵심

### 3.1 조립 정본

```text
VALTAN_CORE_ASSET_IDS
  1. BG_RAD_VALTAN_FLOOR01_SM
  2. BG_RAD_VALTAN_FLOOR01A_SM
  3. BG_RAD_VALTAN_FLOOR01B_SM
  4. BG_RAD_VALTAN_STATUE01_SM

공통 Transform
  position = (0, 0, 0)
  rotation = (0, 0, 0)
  scale    = catalog defaultScale = (1, 1, 1)
  anchor   = Origin
```

### 3.2 원자적 조립

```text
preflight
  -> catalog ready
  -> 기존 네 core asset placement 없음
  -> placement count / ID 여유
  -> 네 asset 모두 catalog에 존재
  -> 네 asset anchor 모두 Origin

stage
  -> 기존 Prototype_GameObject_MapAsset 경로로 4개 clone
  -> 실패: 지금 만든 객체만 Layer에서 전부 rollback

commit
  -> staged 4개를 m_Placements에 이동
  -> next placement ID + 4
  -> 첫 객체 선택
  -> placement armed 해제
  -> dirty = true
```

### 3.3 일반 연속 배치

`Try_PlaceSelected`는 성공 후 `PLACEMENT_STATE::ARMED`를 유지한다. 같은 바닥을 여러 번 클릭해 배치할 수 있고 `Esc`를 눌렀을 때만 해제한다.

## 4. 추가·수정·삭제 파일 목록

| 구분 | 파일 | 역할 |
|---|---|---|
| 수정 | `Client/Public/MapTool.h` | 코어 존재 검사와 자동 조립 명령 선언 |
| 수정 | `Client/Private/MapTool.cpp` | 조립 ID 정본, 연속 배치, preflight → stage → commit/rollback, ImGui 버튼 |
| 수정 | `Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets` | 코어 네 조각만 `BottomCenter`에서 `Origin`으로 전환 |
| 수정 | `.md/GB/07-29/2026-07-29_LOSTARK_VALTAN_ARENA_STRUCTURE_RECOVERY_PLAN.md` | 현재 구현 가능한 최종 교체 코드 기록 |
| 수정 | `.md/GB/07-29/2026-07-29_LOSTARK_VALTAN_ARENA_STRUCTURE_RECOVERY_RESULT.md` | 구현·검증 결과 기록 |

신규 C++ 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다. `BG_RAD_VALTAN_A.mapplacements`, Navigation, StageData 파일은 수정하지 않는다.

## 5. 파일별 전체 구현 코드

### 5.1 `Client/Public/MapTool.h`

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapAssetPreview.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapTool final
{
private:
    enum class PLACEMENT_STATE
    {
        IDLE,
        ARMED,
    };

    struct PLACED_ENTRY
    {
        uint64_t placementId = {};
        std::string assetId;
        shared_ptr<CMapAssetObject> object;
    };

public:
    ~CMapTool();

    HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

    void Toggle();
    void Update(f32_t fTimeDelta);
    void Render();

    bool IsOpen() const;

private:
    void Handle_LevelTransition(bool_t isAssetTest);
    bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
    bool_t Try_PlaceSelected();
    bool_t Create_Placement(uint64_t placementId, const std::string& assetId,
        const float3_t& position, const float3_t& rotationDegrees,
        const float3_t& scale, bool_t visible, PLACED_ENTRY& outEntry);
    bool_t Remove_Placement(uint64_t placementId);
    void Remove_AllPlacements();
    bool_t Save_Placements();
    bool_t Load_Placements();
    bool_t Has_ValtanCorePlacements() const;
    bool_t Assemble_ValtanCore();

    void Select_Asset(const MAP_ASSET_ENTRY& asset);
    void Arm_SelectedAsset();

    void Render_Toolbar();
    void Render_Palette(f32_t childHeight);
    void Render_Hierarchy(f32_t childHeight);
    void Render_Inspector();
    void Render_AssetPreview();
    void Render_DecoderReport() const;

    PLACED_ENTRY* Find_Placement(uint64_t placementId);
    const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
    bool_t m_bOpen = false;
    bool_t m_bWasInAssetTest = false;
    bool_t m_bPreviousMouseDown = false;
    bool_t m_bDirty = false;
    PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;

    CMapAssetCatalog m_Catalog;
    std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
    std::string m_SelectedAssetId;
    std::string m_Status = "Enter AssetTest with F2";
    char m_Filter[128]{};
    std::unordered_set<std::string> m_FavoriteAssetIds;

    vector<PLACED_ENTRY> m_Placements;
    uint64_t m_iSelectedPlacementId = {};
    uint64_t m_iNextPlacementId = 1;
};

NS_END
```

### 5.2 `Client/Private/MapTool.cpp` include 및 조립 정본

```cpp
#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace
{
    constexpr const char* PLACEMENT_MAGIC = "LOSTARK_MAP_PLACEMENTS";
    constexpr uint32_t PLACEMENT_VERSION = 1;
    constexpr uint32_t MAX_PLACEMENT_COUNT = 10000;
    constexpr const wchar_t* MAP_LAYER_TAG = L"Layer_MapAsset";
    constexpr std::array<const char*, 4> VALTAN_CORE_ASSET_IDS =
    {
        "BG_RAD_VALTAN_FLOOR01_SM",
        "BG_RAD_VALTAN_FLOOR01A_SM",
        "BG_RAD_VALTAN_FLOOR01B_SM",
        "BG_RAD_VALTAN_STATUE01_SM",
    };

    bool_t IsValtanCoreAsset(const std::string& assetId)
    {
        return std::any_of(VALTAN_CORE_ASSET_IDS.begin(),
            VALTAN_CORE_ASSET_IDS.end(),
            [&assetId](const char* pCoreAssetId)
            {
                return assetId == pCoreAssetId;
            });
    }
}
```

실제 파일의 같은 익명 namespace에는 placement 문서 parser와 임시 파일 commit 함수가 이어진다. 위 블록은 기존 상수 블록과 include 블록의 최종 교체 코드다.

### 5.3 `Client/Private/MapTool.cpp::Try_PlaceSelected`

```cpp
bool_t Client::CMapTool::Try_PlaceSelected()
{
    const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
    if (nullptr == pAsset)
    {
        m_Status = "Select an asset before placing";
        return false;
    }

    float3_t position{};
    if (!Try_PickPlacementPosition(position))
    {
        m_Status = "No valid surface under the cursor";
        return false;
    }

    PLACED_ENTRY placed{};
    if (!Create_Placement(m_iNextPlacementId, pAsset->id, position,
        float3_t(0.f, 0.f, 0.f), pAsset->defaultScale, true, placed))
    {
        m_Status = "Failed to clone map object for " + pAsset->id;
        return false;
    }

    m_iSelectedPlacementId = m_iNextPlacementId++;
    m_Placements.push_back(std::move(placed));
    m_bDirty = true;
    m_Status = "Placed " + pAsset->label +
        "; placement remains armed (Esc cancels).";
    return true;
}
```

### 5.4 `Client/Private/MapTool.cpp` 코어 검사 및 자동 조립

```cpp
bool_t Client::CMapTool::Has_ValtanCorePlacements() const
{
    return std::any_of(m_Placements.begin(), m_Placements.end(),
        [](const PLACED_ENTRY& entry)
        {
            return IsValtanCoreAsset(entry.assetId);
        });
}

bool_t Client::CMapTool::Assemble_ValtanCore()
{
    if (!m_Catalog.Is_Ready())
    {
        m_Status = "Assembly failed: the Valtan catalog is not ready.";
        return false;
    }

    if (Has_ValtanCorePlacements())
    {
        m_Status = "Assembly blocked: delete existing Floor01/Floor01A/"
            "Floor01B/Statue01 placements first.";
        return false;
    }

    if (m_Placements.size() >
        static_cast<size_t>(MAX_PLACEMENT_COUNT) - VALTAN_CORE_ASSET_IDS.size() ||
        m_iNextPlacementId > (std::numeric_limits<uint64_t>::max)() -
        VALTAN_CORE_ASSET_IDS.size())
    {
        m_Status = "Assembly failed: the placement limit was reached.";
        return false;
    }

    std::array<const MAP_ASSET_ENTRY*, VALTAN_CORE_ASSET_IDS.size()> assets{};
    for (size_t index = 0; index < VALTAN_CORE_ASSET_IDS.size(); ++index)
    {
        assets[index] = m_Catalog.Find(VALTAN_CORE_ASSET_IDS[index]);
        if (nullptr == assets[index] ||
            MAP_ASSET_ANCHOR::ORIGIN != assets[index]->anchor)
        {
            m_Status = "Assembly failed: all four Valtan core assets must use Origin.";
            return false;
        }
    }

    vector<PLACED_ENTRY> staged;
    staged.reserve(VALTAN_CORE_ASSET_IDS.size());
    uint64_t nextPlacementId = m_iNextPlacementId;
    for (const MAP_ASSET_ENTRY* pAsset : assets)
    {
        PLACED_ENTRY entry{};
        if (!Create_Placement(nextPlacementId, pAsset->id,
            float3_t(0.f, 0.f, 0.f), float3_t(0.f, 0.f, 0.f),
            pAsset->defaultScale, true, entry))
        {
            for (const PLACED_ENTRY& rollback : staged)
            {
                CGameInstance::Get().Remove_GameObject_from_Layer(
                    ETOUI(LEVEL::ASSET_TEST), MAP_LAYER_TAG,
                    static_pointer_cast<CGameObject>(rollback.object));
            }
            m_Status = "Assembly rolled back: could not clone " + pAsset->id;
            return false;
        }

        staged.push_back(std::move(entry));
        ++nextPlacementId;
    }

    const uint64_t firstPlacementId = m_iNextPlacementId;
    m_Placements.reserve(m_Placements.size() + staged.size());
    for (PLACED_ENTRY& entry : staged)
        m_Placements.push_back(std::move(entry));

    m_iSelectedPlacementId = firstPlacementId;
    m_iNextPlacementId = nextPlacementId;
    m_ePlacementState = PLACEMENT_STATE::IDLE;
    m_bDirty = true;
    m_Status = "Assembled Valtan core at shared Origin (4 objects, unsaved).";
    return true;
}
```

### 5.5 `Client/Private/MapTool.cpp::Render_Toolbar`

```cpp
void Client::CMapTool::Render_Toolbar()
{
    if (ImGui::Button("Save"))
        Save_Placements();
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        Load_Placements();
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
        ImGui::OpenPopup("Clear all placements?");
    ImGui::SameLine();
    if (ImGui::Button("Assemble Valtan Core"))
        Assemble_ValtanCore();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(
            "Create Floor01, Floor01A, Floor01B, and Statue01 at the same "
            "Origin / rotation 0 / scale 1. Existing core pieces block assembly.");
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(nullptr == Get_SelectedAsset() ||
        PLACEMENT_STATE::ARMED == m_ePlacementState);
    if (ImGui::Button("Arm placement"))
        Arm_SelectedAsset();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("Objects: %zu%s", m_Placements.size(), m_bDirty ? "  *unsaved" : "");

    if (ImGui::BeginPopupModal("Clear all placements?", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Remove every placed map object from this level?");
        if (ImGui::Button("Clear all"))
        {
            Remove_AllPlacements();
            m_Status = "Cleared all placements (not saved yet)";
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (PLACEMENT_STATE::ARMED == m_ePlacementState)
    {
        ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
            "PLACEMENT ARMED: click the world (Esc cancels)");
    }
    ImGui::Separator();
}
```

### 5.6 `Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets`

Catalog 전체 17행 중 변경되는 최종 데이터 블록은 다음 네 행이다.

```text
"BG_RAD_VALTAN_FLOOR01_SM" "Valtan Floor 01" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_FLOOR01_SM/BG_RAD_VALTAN_FLOOR01_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_FLOOR01_SM" 1 1 1 Origin "valtan-confirmed" "Valtan Dedicated (17)" "shared source-space origin; Valtan core assembly"
"BG_RAD_VALTAN_FLOOR01A_SM" "Valtan Floor 01A" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_FLOOR01A_SM/BG_RAD_VALTAN_FLOOR01A_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_FLOOR01A_SM" 1 1 1 Origin "valtan-confirmed" "Valtan Dedicated (17)" "shared source-space origin; Valtan core assembly"
"BG_RAD_VALTAN_FLOOR01B_SM" "Valtan Floor 01B" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_FLOOR01B_SM/BG_RAD_VALTAN_FLOOR01B_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_FLOOR01B_SM" 1 1 1 Origin "valtan-confirmed" "Valtan Dedicated (17)" "shared source-space origin; Valtan core assembly"
"BG_RAD_VALTAN_STATUE01_SM" "Valtan Statue 01" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_STATUE01_SM/BG_RAD_VALTAN_STATUE01_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_STATUE01_SM" 1 1 1 Origin "valtan-confirmed" "Valtan Dedicated (17)" "shared source-space origin; Valtan core assembly"
```

## 6. 프로젝트 등록과 검증

프로젝트 등록 변경은 없다. 기존 `MapTool.cpp`와 `MapTool.h`만 수정하므로 다음 XML을 추가하지 않는다.

검증 순서:

1. Catalog header count 17과 실제 행 수 17이 일치하는지 확인한다.
2. 코어 네 asset만 `Origin`, 나머지 13개는 `BottomCenter`인지 확인한다.
3. Client x64 Debug를 빌드한다.
4. F2로 AssetTest에 들어가 기존 수동 코어 조각을 Hierarchy에서 삭제한다. 다른 배치가 필요 없으면 `Clear`를 사용할 수 있다.
5. `Assemble Valtan Core`를 한 번 눌러 네 객체가 world origin에서 원본 형태로 결합되는지 확인한다.
6. 버튼을 다시 눌렀을 때 객체 수가 늘지 않고 기존 코어 삭제 안내가 표시되는지 확인한다.
7. 일반 바닥 asset을 Arm한 뒤 월드를 여러 번 클릭해 중복 인스턴스가 연속 생성되는지 확인하고 `Esc`로 해제한다.
8. Save 후 Reload하여 네 asset ID, placement ID, 공통 Transform이 유지되는지 확인한다.
9. 실패 경로에서는 기존 placement 목록과 저장 파일이 보존되는지 확인한다.

Navigation 베이크는 위 1차 맵 조립과 Save/Reload가 확인된 뒤 별도 계획에서 진행한다.
