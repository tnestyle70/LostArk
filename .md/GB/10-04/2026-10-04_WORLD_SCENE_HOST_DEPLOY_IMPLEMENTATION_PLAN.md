# F1 World Scene 도구의 live host·Deploy picking 구현 계획서

## G00. 목표와 현재 실측

사용자가 MapTool과 별개인 F1 world model 도구에서 실제 Level 모델을 선택·조작하고 쿠크/발탄의 맵 애니메이션을 확인하도록 요청했다. 새 UI/session은 root 담당이며 이 변경은 기존 Level-owned runtime의 차용 인터페이스와 Deploy exact pick만 확장한다. 두 번째 map/Deploy runtime을 만들거나 Server Deploy state를 편집하지 않는다.

Character Select/Kouku만 현재 `IMapAuthoringHost`를 구현한다. Bern/Valtan은 이미 Debug runtime getter를 갖지만 host registry에서 빠져 있다. Bern의 빈 Deploy runtime은 기존 TARGET_SET 완전성용 실제 Level owner다. Valtan에는 실제 Deploy props와 source cinematic/destruction preview가 존재한다.

## G01. Bern·Valtan host

`Level_Bern.h/.cpp`, `Level_ValtanArena.h/.cpp`, `MapAuthoringHost.h/.cpp`에서 Debug host를 추가한다. catalog, placement, static batches, Deploy는 기존 Level owner의 실제 컨테이너를 반환한다. sequence/physics/debris preview 및 phase/floor override가 빌린 runtime은 구조 변경을 거부하고 자동 종료하지 않는다. edit session의 authored draft와 self motion baseline은 기존 Rebase API를 사용한다.

Bern/Valtan의 optional `Load_SelfMotions`/Update는 기존 검증된 `.mapmotions.json` 소비자만 연결한다. 현재 두 Area 문서는 없으므로 새 oscillator/rate를 만들지 않으며 current bound count는 0이다. authoring active 동안 self motion sampling을 중지한다. phase-driven visibility와 Server destruction presentation은 계속 Level이 소유한다.

## G02. Deploy exact surface picking

`DeployPropObject.h/.cpp`에 Debug `Try_PickInspectionSurface`를 추가한다. Render와 동일한 source visibility 조건을 사용하고 static은 현재 intact/fractured 모델의 실제 triangle, ANIM은 기존 `CModel::Try_PickCurrentPose`로 현재 골격 pose와 mesh index를 얻는다. alpha holes, shader displacement와 flying debris는 이 triangle geometry inspection의 별도 경계다.

`DeployPropRuntime.h/.cpp`는 normalized finite world ray로 기존 entries 전체의 최근접 surface를 비교하고 stable runtimePlacementId, sourcePlacementId, assetId, 현재 표시 WModel, mesh index, material name, hit XYZ를 `DEPLOY_WORLD_MESH_PICK` snapshot으로 반환한다. pick은 state/transform/animation/저장 파일을 변경하지 않는다.

## G03. 검증·소유권

기존 UTF-8 인코딩을 유지하고 root의 동시 picker/session 변경은 보존한다. 새 C++ 파일·project XML은 추가하지 않는다. scoped diff/헤더 소비자·Debug/Release guard를 확인하고 root의 정상 증분 Product Build에 함께 포함한다. 별도 Client/UI 실행이나 자동 Reload는 수행하지 않는다. 결과 문서에는 구현과 실제 빌드, 사용자 입력/애니메이션 복원 확인을 구분한다.

## G04. Deploy 가역 root·visibility와 authoritative preemption

`DeployPropObject.h/.cpp`의 기존 Begin/Sample/End seam을 확장한다. Begin은 STATIC와 clip이 없는 ANIM에도 pose-only preview를 허용하며 저장된 animation이 있을 때만 cursor/loop/pause를 snapshot한다. Sample은 ANIM의 실제 exact clip만 소비한다. UI가 clip 없는 경우 재생 clip을 만들지 않고 root 조작만 요청해야 한다.

`Apply_AnimationAuthoringPose(position, quaternion, positiveUniformScale)` overload는 절대 양수 균일 root scale을 검증 후 기존 presentation transform lane에 적용한다. 기존 2인자 caller는 현재 preview scale을 유지한다. source placement `uniformScale`은 쓰지 않으며 End는 원본 scale로 같은 transform을 다시 계산한다. `Try_GetRenderedRootPose`는 actual current matrix를 분해해 product root offset까지 반영한 현재 pose/scale을 읽는다. signed/비균일 scale은 기존 animated normal/cull 소비자와 맞지 않아 이 범위에서 허용하지 않는다.

`Apply_AnimationAuthoringVisibility(opacity,revealHidden)`는 별도 preview overlay를 소유한다. Render·Shadow·현재 surface pick은 같은 effective opacity를 사용하고 `DEPLOY_SURFACE_PRESENTATION_PACKET` 자체를 수정하지 않는다. End는 overlay를 제거하므로 preview 중 최신 product packet을 복원 값으로 덮어쓰지 않는다. authoring UI는 이전 packet opacity 전체 저장/복원을 없애고 이 API를 소비한다.

`Set_State`는 enum과 실제 target role 검증 후 실제 state 변경 직전에 정상 End를 호출한다. duplicate/invalid state는 preview를 유지한다. Valtan BREAKING은 Deploy INTACT와 같을 수 있으므로 `Begin_DestructionDebrisPresentation`은 shader/model/bounds/scale 전체 staging이 성공한 commit 직전 preview를 End하고, authoritative alias suppression도 기존 physics/debris/suppression guard 통과 후 End한다. 실패한 이벤트는 preview를 중단하지 않는다. MapTool physics/debris guard와 Server state의 owner는 유지한다.

실제 등록 shader는 `Shader_VtxAnimMeshBinary.hlsl`이다. pass0/1에 기존 static carrier와 같은 4×4 opacity coverage를 적용한다. `g_DeployPresentationOpacity`는 default 1이며 Deploy가 draw 뒤 반드시 1로 되돌린다. `CShader::Clone`은 Effect를 공유하므로 실패 mesh 경로도 reset을 통과한다. 다른 캐릭터/FX 소비자, 원본 material opacity와 pass index는 유지한다.

검증은 실제 affected TU Debug/Release `/Zs`, full FX와 actual VS/PS compiler, scoped diff, production 함수 본문/현재 header fields를 그대로 추출한 dependency-stub fixture로 나눈다. fixture는 state/preemption·validation/rollback·source root restoration을 검증하고 actual CModel/GPU 렌더링 증거로 대체하지 않는다. 최종 Product link와 사용자 UI 확인은 root integration 결과에서 구분한다.
