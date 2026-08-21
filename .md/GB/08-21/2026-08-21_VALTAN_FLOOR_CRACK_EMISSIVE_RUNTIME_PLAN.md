# 2026-08-21 발탄 아레나 바닥 균열 발광 런타임 계획

## 목표

발탄 아레나의 `VALTAN_FLOOR_BRICK_A/B` 바닥 틈에 이미 저작된 청록색 균열 발광 마스크를 제품 런타임에서 보이게 한다. 바닥 Transform, 충돌, Navigation, 84줄/30줄 붕괴 상태와 Server 권위는 변경하지 않는다.

## 확인된 원인과 정본

- `BG_RAD_VALTAN_FLOOR01A_SM`과 `BG_RAD_VALTAN_FLOOR01B_SM`의 material index 1에는 `bg_rad_valtan_crack_floor01_em_reconstruction.png`가 Emissive로 연결돼 있다.
- 이 마스크는 원본 MIC의 authored emissive가 아니라 영상 대조용 `VIDEO_MATCH_RECONSTRUCTION`이며 목표 강도는 `0.35`다.
- A/B 네 배치와 rail 두 배치는 현재 Deploy authoring/runtime에 존재하고 시작 상태도 `INTACT`다.
- Deploy layer가 먼저 Emissive MRT에 기록한 뒤 일반 Map 불투명 draw가 Target 4를 0으로 덮을 수 있다. 따라서 같은 객체 안에서 단순 재호출하는 방식이 아니라 모든 불투명 draw 뒤, `MRT_GameObject` 종료 직전의 전용 overlay 순서가 필요하다.

## 구현 계약

1. Engine renderer에 `DEFERRED_OVERLAY` 그룹을 추가한다.
   - `NONBLEND` draw가 모두 끝난 뒤 같은 `MRT_GameObject` 안에서 실행한다.
   - `CGameObject::Render_DeferredOverlay()` 기본 구현은 `S_OK`이며 기존 객체 동작은 변하지 않는다.
2. Deploy asset catalog schema를 version 2로 올리고 다음 두 필드를 데이터가 소유한다. Deploy placement schema와 stable placement ID/Transform은 version 1을 그대로 유지한다.
   - `emissiveIntensity`: 유한한 0 이상 값. 기본 일반 asset은 `1.0`, A/B는 `0.35`.
   - `deferredEmissiveOverlay`: `0|1`. A/B만 `1`.
3. `CDeployPropObject`는 overlay가 선언된 정적 A/B가 material index 1의 Emissive를 실제로 가진 경우에만 초기화를 성공시킨다.
   - `INTACT`이고 파괴/억제/숨김 상태가 아닐 때만 overlay queue에 등록한다.
   - 일반 base pass에서는 해당 material의 Emissive만 끄고 diffuse/normal은 유지한다.
   - overlay pass는 같은 World Transform과 material index 1을 사용한다.
4. `Shader_VtxMeshBinary.hlsl`에 pass 15를 추가한다.
   - `SV_TARGET4` RGB만 기록하고 RT 0~3은 쓰지 않는다.
   - 검은 mask 픽셀은 clip하여 다른 Emissive를 지우지 않는다.
   - depth read-only와 작은 음수 depth bias를 사용하고, 강도는 데이터의 `0.35`를 소비한다.
5. 84줄/30줄에서 A/B가 `DESPAWNED`되면 overlay도 즉시 사라진다. 별도 local 상태나 위치 보정은 만들지 않는다.

## 수정 대상

- `Engine/Public/Engine_Enum.h`
- `Engine/Public/GameObject.h`
- `Engine/Private/GameObject.cpp`
- `Engine/Private/Renderer.cpp`
- `Client/Public/DeployPropCatalog.h`
- `Client/Private/DeployPropCatalog.cpp`
- `Client/Public/DeployPropObject.h`
- `Client/Private/DeployPropObject.cpp`
- `Client/Private/DeployPropRuntime.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`
- `Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployassets`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployassets` (publisher 생성물)
- `Tools/LevelPlacementExtractor/build_deployprop_runtime.py`
- `Tools/LevelPlacementExtractor/build_valtan_floor_collapse.py`
- `Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py`
- `Tools/Build/Invoke-BuildAndRegression.ps1`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- 관련 RESULT

## 실패와 Rollback

- schema/version/field count, 비유한 intensity, 잘못된 boolean은 catalog load를 실패시키고 기존 stage를 commit하지 않는다.
- overlay 선언 asset에 material index 1 또는 Emissive texture가 없으면 해당 Deploy prototype 초기화를 실패시킨다.
- renderer overlay draw가 실패해도 MRT는 반드시 종료하고 frame failure를 호출자에게 반환한다.
- 기존 다른 담당자의 Server/Encounter/Balance 미커밋 변경은 수정하거나 되돌리지 않는다.

## 자동 검증

1. `test_valtan_floor_emissive_contract.py`의 Deploy schema/generator, WModel material 1 resource, renderer 순서, shader pass 15, Deploy 상태 계약 6개.
2. HLSL FX 5.0 compile.
3. Engine x64 Debug/Release build 후 `UpdateLib.bat` Debug/Release.
4. ClientFrontendHarness Debug/Release build와 focused floor-emissive contract 실행.
5. Client x64 Debug/Release build.
6. Map publisher validation/publish가 필요한 경우 authoring/runtime Deploy 문서 semantic identity 확인.
7. `git diff --check`.

## 사용자 수동 확인

에이전트는 Client를 실행하거나 visual PASS를 선언하지 않는다. 자동 검증 뒤 사용자가 Lobby → Valtan 진입 후 다음을 확인한다.

- 시작 상태에서 돌 틈에 청록색 선형 발광이 보인다.
- 캐릭터나 벽을 뚫고 발광하지 않고 비스듬한 카메라에서도 심한 깜빡임이 없다.
- 84줄/30줄 바닥 붕괴 후 해당 sector의 발광도 같이 사라진다.
