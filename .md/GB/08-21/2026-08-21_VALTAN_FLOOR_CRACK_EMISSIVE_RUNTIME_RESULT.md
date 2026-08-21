# 2026-08-21 발탄 아레나 바닥 균열 발광 런타임 결과

## 결론

발탄 아레나의 `VALTAN_FLOOR_BRICK_A/B`가 이미 가진 균열 Emissive를 일반 불투명 Map draw 뒤에 다시 기록하는 전용 deferred overlay 경로를 구현했다. focused 계약, catalog publish/parity, shader와 renderer 계약, Engine Debug/Release, UpdateLib Debug/Release 및 변경된 Client translation unit의 Debug object compile은 통과했다. 전체 Client Debug build는 이번 변경이 아닌 기존 dirty `Valtan.h`, Balance Tool, Encounter Pattern Reference 계열 문법 오류로 exit 1이어서 blocked 상태다. Release Client, ClientFrontendHarness 전체 회귀, Client/UI 실행과 사용자 육안 확인은 실행하지 않았으므로 최종 visual PASS로 기록하지 않는다.

## 리소스와 원인 실측

- A/B WModel의 material index 1은 `bg_rad_valtan_crack_floor01_d_lsj.png`, `bg_rad_valtan_crack_floor01_n_lsj.png`, `bg_rad_valtan_crack_floor01_em_reconstruction.png`를 연결한다.
- 원본 MIC에는 diffuse와 normal만 있고 authored emissive는 없다. 현재 발광 mask는 영상 대조로 만든 `VIDEO_MATCH_RECONSTRUCTION`이며 catalog 강도는 `0.35`다.
- A/B 네 배치와 rail 두 배치의 authoring/runtime placement는 이미 존재했다. 위치를 새로 만들거나 Y를 올리는 작업은 하지 않았다.
- 기존 Deploy draw가 먼저 Target Emissive를 쓴 뒤 다른 Map 불투명 draw가 같은 target을 0으로 덮을 수 있었다. 바닥 geometry가 거의 같은 깊이에 있는 점도 이 현상을 악화시켰다.

## 구현된 계약

### Deploy catalog와 placement

- `LOSTARK_DEPLOY_PROP_CATALOG`을 version 2로 올렸다.
- 각 asset row에 `emissiveIntensity`와 `deferredEmissiveOverlay`를 추가했다.
- `VALTAN_FLOOR_BRICK_A/B`만 `0.35 1`, 나머지 Deploy asset은 `1 0`을 사용한다.
- `LOSTARK_DEPLOY_PROP_PLACEMENTS`는 version 1을 유지한다. stable placement ID, Transform, 84줄/30줄 mutation 연결은 바뀌지 않았다.
- 비유한 값, 음수 intensity, `0|1` 이외의 overlay 값, animated asset의 overlay 선언은 catalog stage에서 fail-closed한다.

### 마지막 MRT 발광 overlay

- Engine `RENDERGROUP`에 `DEFERRED_OVERLAY`를 추가하고 모든 `NONBLEND` draw 뒤, `MRT_GameObject`를 닫기 전에 처리한다.
- renderer는 기존 `NONBLEND` 객체 loop의 호출·실패 처리 의미를 유지하고, 그 뒤에 별도 overlay loop만 추가하는 최소 구조를 사용한다.
- `CGameObject::Render_DeferredOverlay()`의 기본 구현은 `S_OK`여서 기존 객체 동작은 유지된다.
- `CDeployPropObject`는 static, `INTACT`, base presentation 비억제 상태이며 catalog flag가 켜진 A/B만 overlay queue에 넣는다.
- 일반 base pass에서는 A/B material 1의 Emissive만 억제하고 diffuse/normal 렌더는 유지해 중복 발광을 막는다.
- overlay는 같은 intact model, material index 1, World Transform을 사용한다. A/B가 `DESPAWNED`되거나 파괴 preview가 base를 억제하면 overlay도 제출하지 않는다.

### Shader pass 15

- `DeferredEmissiveOverlayPass`는 `SV_TARGET4` RGB만 기록하고 G-buffer target 0~3은 보존한다.
- 검은 mask 픽셀은 clip해 앞서 기록된 다른 Emissive를 지우지 않는다.
- depth는 read-only이며 작은 음수 depth bias를 사용한다. authored Transform을 움직이지 않는다.
- intensity는 catalog의 `0.35`를 소비한다.

## 변경하지 않은 경계

- 바닥 및 rail placement Transform
- player/Valtan collision과 navigation
- 84줄/30줄 Server 권위와 mutation state
- Shared protocol, Server gameplay 또는 damage 판정
- 외곽 벽·철탑 배치와 리소스

## 자동 검증 상태

| 검증 | 상태 | 증거 |
|---|---|---|
| `python Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py` | PASS | focused contract 6/6 |
| Python 변경 파일 `py_compile` | PASS | generator와 focused contract 문법 검사 성공 |
| source/runtime Deploy catalog parity | PASS | version 2, 12행 semantic parity와 A/B `0.35/1` 확인 |
| 변경 PowerShell script parse | PASS | parser 오류 없음 |
| `Shader_VtxMeshBinary.hlsl` FX 5.0 compile | PASS | pass 15 포함 FX compile 성공 |
| shader/renderer source contract | PASS | pass 15 Target 4 전용 기록과 opaque 뒤·MRT 종료 전 호출 순서 확인 |
| Engine x64 Debug/Release rebuild | PASS | Engine public API와 renderer 변경 compile/link 성공 |
| `UpdateLib.bat` Debug/Release | PASS | 두 구성의 Engine public header/lib/runtime 전달 성공 |
| DeployProp Client translation unit Debug object compile | PASS | `DeployPropCatalog.cpp`, `DeployPropObject.cpp`, `DeployPropRuntime.cpp` 실제 compile 성공 |
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED` | PASS | runtime deploy catalog version 2, A/B `0.35/1` publish 확인 |
| World Destruction `ContractTest` | PASS | 기존 destruction/Deploy 연동 계약 회귀 통과 |
| 전역 `git diff --check` | PASS | whitespace 오류 없음 |
| 전체 Client x64 Debug build | BLOCKED | 기존 unrelated dirty `Valtan.h`/Balance Tool/Encounter Pattern Reference 계열 문법 오류로 exit 1; 이번 Deploy object 3개는 별도 compile PASS |
| 전체 Client x64 Release build | PENDING | 미실행/미확정 |
| ClientFrontendHarness Debug/Release 및 전체 build regression | PENDING | 미실행 |
| Client/UI runtime 및 사용자 visual smoke | PENDING | 에이전트와 사용자 모두 미실행 |

## 사용자 수동 확인 대기

전체 Client build의 unrelated blocker가 정리되고 실행 준비가 끝난 뒤 사용자가 Lobby에서 Valtan으로 진입해 다음을 직접 판정해야 한다.

1. 시작 상태의 돌 틈에 청록색 균열선이 보이는지 확인한다.
2. 비스듬한 카메라에서 심한 깜빡임이나 바닥 밖 발광이 없는지 확인한다.
3. 84줄/30줄 바닥 붕괴 후 사라진 A/B sector의 균열 발광도 함께 사라지는지 확인한다.

사용자 관찰이 기록되기 전에는 first pixel, eye smoke, visual fidelity PASS 또는 원작 동일을 완료로 선언하지 않는다.
