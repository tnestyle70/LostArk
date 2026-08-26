# Map Tool 베른 Area 전환 결과

작업일: 2026-08-26

## 1. 결론

로비 `Test` -> Map Tool의 Area 콤보에서 `Bern`을 골라도 들어가지지 않던 실제 원인은
authoring water 문서의 경로 결함이었다. `CMapAssetCatalog::Load_WaterPresentation`의
editor 분기가 `Data/Maps/Authoring/<AreaId>/` 대신 `Data/Maps/<AreaId>/`를 보고 있었고,
베른은 WATER asset을 하나 가지므로 문서를 찾지 못한 순간 catalog 로드가 실패했다.
전환은 그 자리에서 중단되고 Area는 Character Select로 남았다.

같은 작업에서 Area 전환의 모델 admission을 프레임 예산으로 나누고 진행률을 표시하도록
바꿨다. 이쪽은 위 실패의 원인이 아니라, 경로를 고친 뒤에야 실제로 도달하는 1.8 GB
동기 구간을 응답 가능한 상태로 만들기 위한 별개의 개선이다.

## 2. 확인한 증거

사용자 화면의 workspace status:

```text
Map water document is missing for a WATER asset:
C:\Users\USER\source\...\LostArk\Data\Maps\LV_BER_BERNCASTLE\LV_BER_BERNCASTLE.mapwater.json
Level: 6 | Area: LV_LOBBY_CLASSSELECT_SL00 | Catalog: READY
```

경로 규약을 실측하면 `Get_MapAuthoringRoot()`는 `Data/Maps`를 돌려주고 호출자가
`Authoring` 세그먼트를 직접 붙인다.

```text
MapAssetCatalog.cpp:862  Get_MapAuthoringRoot() / "Editor" / "ACTIVE.maparea"      정상
MapAssetCatalog.cpp:876  Get_MapAuthoringRoot() / "Authoring" / <Area> / .mapplacements  정상
MapAssetCatalog.cpp:669  Get_MapAuthoringRoot() / <Area> / .mapwater.json          Authoring 누락
```

정본 위치와 publisher 계약도 같은 결론이다.

```text
실제 파일            Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapwater.json
MapCatalog sourceWater  Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapwater.json
Publish-MapAuthoring.ps1:21,397  Data\Maps\Authoring\$AreaId\$AreaId.mapwater.json
```

runtime 분기는 `Client/Bin/DataFiles/Map/<AreaId>.mapwater.json`을 보며 그 파일은 존재한다.
제품 베른 Level이 멀쩡하고 Map Tool만 실패한 이유가 이 갈래 차이다.

water 문서 내용 자체는 정상이다.

```text
source .mapassets의 WATER asset   1개  MAP_5387B1504BDD_LV_BER_BERNCASTLE_WATER01_SM
authoring .mapwater.json의 water 행 1개  같은 asset ID          양방향 일치
```

베른 authoring 데이터의 나머지도 모두 정상임을 함께 확인했다.

```text
.mapset 13개 shard 선언 개수 == 실제 파일 헤더        일치
catalog 3021행이 가리키는 .wmodel 누락                0개
placement 50,017행의 미해결 asset ID                  0개
Area 간 prototype 태그 충돌                           없음
SpawnGroups.world.json 부재                           설계상 정상
```

Area별 payload 실측(프레임 예산 변경의 근거):

```text
Character Select  모델   55  DDS  153   116 MB
Valtan            모델  260  DDS  532   407 MB
Bern              모델 1003  DDS 3329  1842 MB + placement 50,017
```

## 3. 반영한 수정

- `Client/Private/MapAssetCatalog.cpp`
  - `Load_WaterPresentation`의 editor 분기 경로에 누락된 `Authoring` 세그먼트를 넣었다.
    runtime 분기는 그대로다.
- `Client/Public/MapTool.h`
  - `EDITOR_AREA_PRELOAD_STATE`(대상 descriptor index, 모델 catalog, 다음 entry index)와
    `Admit_AuthoringPrototype`, `Begin_EditorAreaSwitch`, `Update_EditorAreaPreload`,
    `Report_EditorAreaPreloadProgress`를 추가했다.
- `Client/Private/MapTool.cpp`
  - `Admit_AuthoringPrototype`로 모델 한 개의 admission을 분리했다. 이 도구가 이미 admission한
    태그는 `m_PrototypeModelPaths` fingerprint로 판정하며, 예전처럼 `Clone_Prototype`으로 깊은
    복제를 만들었다 버리지 않는다. 태그가 다른 모델을 가리키면 그대로 실패한다.
  - `Ensure_AuthoringPrototypes`는 같은 헬퍼를 순회하는 형태로 줄었고 호출자 계약은 같다.
  - `Begin_EditorAreaSwitch`는 guard와 source 존재 검사, 모델 catalog `Load_Source`까지만 하고
    admission 대기 상태를 세운다. 나머지 authoring 문서는 여전히 원자적 `Switch_EditorArea`
    트랜잭션 안에서 로드된다.
  - `Update_EditorAreaPreload`가 매 프레임 `EDITOR_AREA_ADMISSION_FRAME_BUDGET`(8ms)만큼
    admission하고 진행률을 갱신한 뒤, 끝나면 기존 `Switch_EditorArea`를 호출한다. 중간 실패는
    대기 상태만 버리고 실패한 asset ID를 status에 남기며 현재 Area를 보존한다.
  - Area 콤보와 미저장 확인 modal의 `Switch_EditorArea` 호출을 `Begin_EditorAreaSwitch`로 바꿨다.
  - admission 중에는 Area 콤보를 잠그고 `ImGui::ProgressBar`를 그리며 `Update_WorldInteraction`을
    비활성으로 넘겨 전환 도중 편집이 시작되지 않게 했다.
  - `Handle_LevelTransition`이 `m_PrototypeModelPaths`를 비울 때 대기 상태도 함께 버린다.
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
  - 베른 Area 전환 항목을 현재 동작으로 교정했다.

Map Tool 진입 시의 `Switch_EditorArea(0)`(Character Select, 116 MB)은 기존 동기 경로를 유지했다.

## 4. 자동 검증

```text
Client.vcxproj x64 Debug 전체 빌드 및 링크              PASS (exit 0)
  Client/Bin/Debug/Client.exe 갱신                      2026-08-26 15:35
Valtan tuning pipeline VALIDATE (Client pre-build)      ok: true, errors: []
git diff --check                                        PASS
MapTool.cpp/.h, MapAssetCatalog.cpp 인코딩·개행 보존     UTF-8 유지, CRLF 유지
```

기존 C4819와 DirectXTK LNK4099는 경고이며 오류는 0개다.

## 5. 남은 경계와 수동 검증

- 모델 admission이 끝난 뒤 실행되는 `Switch_EditorArea` 트랜잭션은 여전히 한 프레임이다.
  베른의 placement 50,017행 parse/stage/commit 구간에서 짧은 멈춤이 남는다. 이 구간까지
  나누려면 원자적 rollback 계약을 함께 재설계해야 하므로 이번 범위에 넣지 않았다.
- 에이전트는 Client를 실행하거나 조작하지 않았다. 아래는 사용자가 직접 확인한다.

```text
1. Ctrl+F5로 Server + Client 실행
2. 로비에서 Test 진입 (Character Select Area가 먼저 열린다)
3. Map Tool 상단 Area 콤보에서 Bern 선택
4. water 오류 없이 Preparing Bern: N / 1003 진행률이 올라가는지 확인
5. 진행률이 끝난 뒤 workspace status가 commit되고 베른 맵이 보이는지 확인
6. 전환 도중 Area 콤보가 잠기고 끝난 뒤 다시 열리는지 확인
```

진행률 도중 `Map authoring model admission failed: <assetId>`가 뜨면 그 asset이 실제 실패
지점이며 이전 Area는 그대로 유지된다.
