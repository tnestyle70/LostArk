# F1 World Scene live host·Deploy picking 구현 결과

## G00. 구현 상태

Bern/Valtan이 Debug `IMapAuthoringHost`를 구현하고 기존 registry에서 발견된다. 정적 map runtime, catalog, placement, batches는 기존 Level-owned 객체 그대로 차용한다. 두 번째 runtime을 생성하지 않았다. generic host는 `Get_MapAuthoringRuntime()`을 제공하며 Character Select 기존 getter에 override를 표시하고 Kouku host에 getter 한 줄을 추가했다.

Bern은 Deploy source pair가 없으므로 Deploy 목록 host getter는 null이다. WorldSequence TARGET_SET에는 예전부터 Level에 있던 빈 Deploy owner를 사용해 기존 complete 조건을 만족한다. entrance camera 및 map replacement preview가 구조를 빌린 동안 변경을 거부하며 자동 종료하지 않는다.

Valtan은 실제 Deploy runtime을 차용한다. source/Workbench/Effect cinematic, map replacement, staged/pending destruction, product flying debris와 각 Deploy animation/physics/debris preview가 활성인 동안 구조 변경을 거부한다. Server phase state, destruction state와 replicated gameplay를 편집하지 않는다.

## G01. 기존 self-motion 소비자 연결

Bern/Valtan은 `Load_SelfMotions(areaId)`와 `Update_SelfMotions(delta)`를 기존 runtime API로 연결했다. 현재 `LV_BER_BERNCASTLE.mapmotions.json`, `LV_LUT_HEARTRB_ED.mapmotions.json`은 source/published 모두 없으며 bound motion은 0이다. 기존 reader가 absent document를 정상 empty로 취급하므로 임의 oscillator, rotation rate, clip 또는 원본 단위를 생성하지 않았다. 문서가 존재하면 기존 schema/placement binding 검증을 적용하며 malformed 입력은 초기화를 실패시킨다. Debug authoring이 active일 때는 self-motion sampling만 멈추며 Server presentation은 계속 기존 owner가 관리한다.

## G02. Deploy 현재 geometry exact pick

Debug `CDeployPropObject::Try_PickInspectionSurface`와 `CDeployPropRuntime::Try_PickInspectionSurface`를 추가했다. 후자는 `DEPLOY_WORLD_MESH_PICK`에 `runtimePlacementId`, mesh index, hit XYZ, area/sourcePlacement/asset/WModel/material identity, model kind와 현재 state를 반환한다.

현재 source render의 DESPAWNED/revealHidden, opacity, base suppression, camera preview suppression 조건을 사용한다. static은 현재 intact/fractured 모델의 실제 triangle과 back-face cull을 사용한다. ANIM은 기존 `CModel::Try_PickCurrentPose`의 현재 skinned geometry로 exact mesh index를 얻는다. 전체 existing entries에서 normalized finite world ray의 최근접 hit를 선택하며 asset/model identity는 현재 catalog와 현재 rendered branch에서 읽는다. 실패는 이전 output snapshot을 변경하지 않는다. pick은 transform/state/animation/파일을 변경하지 않는다.

이 API는 flying debris, texture alpha holes, shader displacement 및 최종 GPU pixel을 선택하는 API가 아니다. ANIM의 기존 current-pose CPU picker는 양면 triangle 검사이며 rendered raster cull과 다를 수 있다. UI는 geometry inspection 경계를 표시해야 한다. 아래 G04에서 기존 Begin/Sample/End seam을 STATIC/bind-pose와 가역 scale/visibility까지 확장했다.

## G03. 실제 검증

- `DeployPropObject.cpp`, `DeployPropRuntime.cpp`, `MapAuthoringHost.cpp`, `Level_Bern.cpp`, `Level_ValtanArena.cpp` 실제 TU 각각 Debug/Release, 총 10회 `/Zs` PASS.
- 현재 VS18 Insiders v143 14.44, current EngineSDK/Client/Shared includes, 기존 forced standard PCH를 사용했다. 일부 기존 CP949 header의 UTF-8 encoding warnings는 있으며 compiler error는 0이다. 그 header를 재작성하지 않았다.
- scoped `git diff --check` PASS. 기존 파일 UTF-8 encoding 보존. 새 C++ 파일 또는 project XML 추가 없음.
- Bern 현재 source master 50,021 placement stable IDs와 published 24 shards 합 50,021 IDs 일치, sourceOnly/runtimeOnly 0. 기존 MapCatalog의 placementCount 50,017 metadata와 실제 현재 rows는 구분한다.

`out/WorldSceneHostDeploy20261004`에 configuration별 실제 `.rsp`/`.log`, `compile_tus.cmd`, `syntax-result.json`을 보존한다. 이 결과는 문법 검증이며 root 담당 정상 증분 Product compile/link 결과와 별개다.

## G04. 가역 Deploy TRS·visibility와 전투 presentation preemption

`Begin_AnimationAuthoringPreview`는 STATIC와 clip 없는 ANIM도 허용한다. 실제 clip이 있는 ANIM만 cursor/loop/pause를 snapshot하며 Sample은 기존 exact clip만 요청한다. `Apply_AnimationAuthoringPose(position,quaternion,positiveUniformScale)`는 절대 양수 균일 scale을 적용하고 기존 2인자 overload는 현재 preview scale을 유지한다. `Get_PlacedRootUniformScale`은 원본 source 값을 읽으며, `Try_GetRenderedRootPose`는 현재 render matrix의 product offset·root pose까지 읽는다. source placement TRS는 쓰지 않는다. signed/비균일 scale은 현재 animated shader normal/cull 계약과 맞지 않아 지원하지 않는다.

`Apply_AnimationAuthoringVisibility(opacity,revealHidden)`는 별도 절대 opacity [0,1] overlay다. source Render·Shadow·geometry pick은 같은 effective opacity를 사용한다. `DEPLOY_SURFACE_PRESENTATION_PACKET`을 저장/복원하지 않으므로 preview 중 갱신된 최신 game packet이 End/preemption 뒤 그대로 적용된다. hidden reveal은 DESPAWNED 표시만 임시 해제하며 다른 owner의 suppression을 변경하지 않는다.

`Set_State`는 invalid enum/target role를 먼저 거절하고 duplicate state는 preview를 유지한다. 실제 state 변경 때만 정상 End로 원본 root/clip과 최신 surface packet을 복원한 뒤 기존 logical role/state를 적용한다. state transaction의 invalid enum도 staging에서 거절한다. Valtan BREAKING은 INTACT와 같을 수 있으므로 authoritative Product debris는 shader/model/bounds/scale 전체 staging 성공 후 commit 직전에 preview를 End한다. alias suppression도 기존 physics/debris/suppressed 검증 후 End한다. invalid event는 preview를 유지하고 MapTool physics/debris 중복 guard는 보존한다. Server 상태 작성자나 source Data 편집 경로를 추가하지 않았다.

실제 Deploy 등록 shader `Shader_VtxAnimMeshBinary.hlsl` pass0/1은 static과 같은 4×4 coverage fade를 사용한다. default `g_DeployPresentationOpacity=1`은 모든 pixel의 기존 coverage를 유지한다. CShader clone은 Effect를 공유하므로 Deploy draw가 실패했을 때도 uniform을 1로 reset한다. 기존 material opacity, 캐릭터/FX의 draw state와 pass index를 변경하지 않는다. legacy `Shader_VtxAnimMesh.hlsl`에는 최종 diff가 없다.

## G05. G04 실제 검증과 증거

- 최신 `DeployPropObject.cpp`, `DeployPropRuntime.cpp`, `Level_Bern.cpp` 실제 TU Debug/Release 총 6회 `/Zs` PASS. wind public 구조가 먼저 변경되고 EngineSDK 배포가 아직 진행 중인 시점의 첫 stale-header 실패를 구분하고, scratch `.rsp`만 최신 `Engine/Public` include 우선으로 검증했다. SDK/제품 바이너리를 이 검사로 교체하지 않았다.
- 실제 Binary full effect `fx_5_0`, `VS_MAIN` `vs_5_0`, `PS_MAIN`/`PS_MAIN_SHADOW` `ps_5_0` 총 4개 compiler 작업 PASS. 기존 include warnings는 있으며 compiler error는 0이다. compile 산출물은 `out/WorldSceneHostDeploy20261004/preemption-scale`에만 저장했다.
- production 함수 본문 18개와 actual header fields를 그대로 가져온 fixture 31 checks, failures 0. model/transform/prototype registry dependencies는 stub이며 shader draw·실제 CModel 원본 pose 성공 증거로 대체하지 않는다. source-body SHA receipt를 함께 보존했다.
- fixture는 STATIC/bind-pose Begin, clip 생성 거부, invalid/duplicate 상태 보존, invalid scale/opacity 보존, current TRS, 기존 pose overload의 scale 보존, byte-exact source root matrix End 복원, latest product packet 보존, target logical clip preemption, invalid product event 보존, 기존 physics/debris guard, 동일 INTACT 상태의 실제 burst preemption 및 authoritative alias suppression을 검증했다.
- 소유 변경 scoped `git diff --check` PASS. source/published Data/Settings 변경 없음. Client/UI 실행과 사용자 화면 확인 없음.

`out/WorldSceneHostDeploy20261004/preemption-scale`의 `.rsp/.cmd/.log`, fixture cpp/source-receipt, shader cso가 검증 정본이다. 이 결과는 root의 정상 증분 Product compile/link와 구분한다.

## G06. 남은 integration·사용자 확인

root 담당 `CWorldSceneTool`/MainApp 및 기존 placement edit session이 이 generic host와 snapshot API를 소비한다. authored draft/visibility, current runtime representation, dirty/save freshness와 publish rollback은 그 session이 소유한다. 기존 single-object `Apply_PlacementTransform`의 visible 반영과 partial-live/dirty-preserving binding 확장은 root가 별도로 담당한다.

통합된 소비자와 최종 Product Debug/Release compile/link 증거는 [World Scene Tool 통합 결과 G06](2026-10-04_F1_WORLD_MESH_INSPECTION_RESULT.md#g06-세션-인계-후-최종-빌드와-실행-준비)을 따른다.

Client/UI를 실행하지 않았다. 사용자 one-click, 현재 skeleton pose 선택, clip preview의 시작/seek/종료 복원, STATIC/ANIM의 가역 TRS/visibility, 실제 전투 이벤트의 preview 종료 및 저장/재진입 확인은 root의 integration/Product Build 후 사용자가 직접 한다. 수동 확인을 PASS로 기록하지 않는다. Deploy preview는 source exact permanent Data를 저장하는 편집 경로가 아니다.
