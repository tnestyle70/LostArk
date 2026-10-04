# Character Select 원본 렌더링 입력·UI 누락 복원 결과

원본으로 확인한 PNG 10개, 식생 재질 18행, 배치별 입력 62행을 설치했습니다.
회전 장식 2개는 원본 fMotionVel 단위와 초기 구동 근거가 확인되지 않아 설치하지 않았습니다.
이는 복원이 끝난 항목과 원본 근거가 남은 항목을 분리한 결과입니다.

1. UI 원본 10개
EFUI_LOBBY / EFUI_CREATECHARACTER / EFUI_SHAREIMAGE 원본 package에서 atlas 5개를 새로 추출했습니다.
원본 GFX 11개 movie를 검사하고 실제 사용하는 3개 movie의 subimage와 합성 위치를 재대조했습니다.
option/rename normal·hover, CreateCharacterCenterButton normal·hover, TrialV2Button normal·hover,
TrialModeBand, TrialModePlate 총 10개가 현재 설치되어 있습니다.
현재 파일 SHA와 decode한 RGBA SHA·크기가 후보와 10/10 일치합니다.
기존 원본 기반 crop/alpha 합성 레시피를 사용했으며 layout나 카드 그림은 변경하지 않았습니다.
TrialModeBand의 1228-pixel viewport는 기존 정수 crop 레시피를 보존했습니다. 원본 SWF의 최종
viewport/raster sampling까지 완전히 동일하다고 주장하지 않습니다. 추측 이미지·대체 그림은 만들지 않았습니다.

2. 원본 식생 shader 입력
원본 MIC 7개 → 재질 18행(E4FE 13행, 1C39 5행), asset 13개, 물리 모델 5개입니다.
배치 62개는 StaticMeshActor 53개와 StaticMeshCollectionActor component 9개입니다.
각 배치의 원본 Actor.location과 component/actor 복합 matrix를 따로 연결했습니다.
원본 Scene wind 하나의 Strength 0.699999988079071 / Speed 2.0과 원본 Actor local X에 근거하여
wind vector4는 [0.699999988079071, 0, 0, 2]입니다.
원본 FBoxSphereBounds TransformBy의 최대 column norm과 Static UpdateBounds의 +1 cm를 확인했습니다.
extent/radius 각각 +1 cm 후 BoundsScale을 곱했으며 원본 BoundsScale은 62개 모두 1입니다.
재질 18행의 foliageWind와 루트 placementWind 62행만 추가·정정했습니다.
저작/게시 두 문서는 현재 SHA d5d8858b0c99924f6af235605e507e25ca10cbeb631d24cafde322ec3f492078입니다.
기존의 material field, baked lighting, TRS, visibility, 사용자가 OFF한 Bloom/Fog/FXAA를 보존했습니다.
원본 package 10개와 최신 target hash를 재검사하고 exact backup 및 원자적 교체를 수행했습니다.

3. geometry 검증 경계
물리 모델 5개의 원본 LOD0 vertex count와 현재 WModel vertex count가 모두 일치합니다.
전체 POSITION+UV0 vertex의 원본↔현재 양방향 nearest coverage를 검사했습니다.
대응 vertex의 position 오차 최대는 4.57763671875e-05 cm,
UV0 오차 최대는 0입니다.
이는 nearest coverage이며 topology 또는 vertex identity의 일대일 대응을 주장하지 않습니다.
cm→m preScale 및 [x,y,z]→[x,z,-y] 변환을 중복하지 않았습니다.
이 검증은 전체 topology/tangent/color와 native userdata 0x8000 특수 draw branch의 동일성까지 증명하지 않습니다.
현재 Renderer 시간 연결 또한 원본 게임의 시간 owner 전체를 확인했다는 의미는 아닙니다.

4. 회전 장식 2개와 기존 mapmotions 소비자
sourcePlacement export 307/308에 EFActorMotionRotationAcyclic / AXIS_Z / fMotionVel +5/-5가 존재합니다.
.u의 관련 class/default에는 실행 method bytecode가 없고, native binary 36개의 export search에서도
관련 구현 symbol을 찾지 못했습니다. 이 scalar를 임의로 degree/sec로 변환하지 않았습니다.
native 속도 단위, 축 합성, wrap, 시간 owner와 constructor/BeginPlay/Kismet 초기 구동은 남은 근거입니다.
bMotionToggle 직렬화 누락만으로 초기 OFF라고 판정하지 않습니다. 후보 2행은 설치하지 않았습니다.
기존에 검증된 mapmotions는 CS main과 성공적으로 로드한 background에서 Load/Update하도록 연결했습니다.
현재 존재하지 않는 선택적 mapmotions 파일로 Area 로딩을 실패시키지 않습니다.

5. 독립 F1 WorldSceneTool animation module
기존 Deploy/CModel animation preview 경로로 clip 선택·play/pause/seek/loop/speed를 연결했습니다.
STATIC 또는 bind-pose 모델은 clip Sample 없이 root position/rotation offset/positive uniform scale을 미리봅니다.
초기값은 실제 renderer matrix에서 읽어 초기 preview 클릭 시 pose가 뛰지 않게 했습니다.
opacity/reveal은 별도의 가역 presentation overlay를 사용하며 authoritative surface packet을 수정하지 않습니다.
성공한 Begin만 소유하며 외부 state preemption·object/context 변경·Hide 시 미리보기 소유를 내려놓습니다.
map self-motion은 기존 clock/paused/rate를 캡처합니다. host/level/area/runtime generation이 모두 같을 때만
이전 clock을 복원하여 같은 Area reload의 새 runtime에 오래된 시간을 적용하지 않습니다.
자기 Deploy preview와 map motion preview는 UI 및 Begin 함수에서 동시에 시작하지 못하게 했습니다.
외부 Object/Composition/arena presentation이 map 소유권을 얻으면 map motion preview를 종료합니다.
이 경우 이전 time으로 Seek하여 외부 pose를 덮지 않고 저장한 paused/rate만 복원합니다.
현재 self-motion 소비자는 게시 Map 폴더가 아니라 Data/Maps/Authoring/<Area>/<Area>.mapmotions.json입니다.
Kouku에는 기존 authored 126행(CYCLIC 114 / ACYCLIC 12, nonzero range 114)이 있습니다.
이는 이번에 새로 복원한 행이 아니며 이 보고서에서 native 속도 단위·활성화까지 새로 검증했다고 주장하지 않습니다.
Bern/CS/Valtan에는 현재 이 authored motion 파일이 없습니다. CS 원본 후보 2행도 여전히 설치하지 않았습니다.
Area/sourcePlacementId/assetId/WModel/exact mesh/material/hit XYZ와 Copy source selection을 표시합니다.
목록에서 선택한 placement는 exact mesh pick과 구별하고 Pick in world 안내를 표시합니다.

6. 검증
현재 UI 10/10 파일·RGBA 일치, 저작/게시 wind 18+62 일치, native shader/bounds/owner 근거 일치입니다.
읽기 전용 publisher: Validate=PASS; Check=PASS
WorldSceneTool_Animation.cpp /Zs PASS, 현재 SHA 782d410713d979d414e1f56a360311328b26f7cecbf56be70086401a57fcda6d입니다.
Product Debug/Release 통합 빌드의 최종 상태와 로그는 [World Scene Tool 통합 결과 G06](2026-10-04_F1_WORLD_MESH_INSPECTION_RESULT.md#g06-세션-인계-후-최종-빌드와-실행-준비)에 기록합니다.
Client, 원작 게임, UI를 실행하지 않았으며 최종 화면이 원작과 완전히 같다는 판정은 하지 않았습니다.

## 근거 파일

- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\outputs\character-select-restoration-result.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\outputs\character-select-ui-restoration.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\outputs\character-select-wind-restoration.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\outputs\character-select-motion-staged-evidence.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\work\character-select\restore\wind-placement-source-evidence.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\work\character-select\restore\ui-fresh-gfx-verification.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\work\character-select\restore\publisher-check.json`
- `C:\Users\user\Documents\Codex\2026-10-04\review-recent-gameplay-fixes-character-select\work\character-select\restore\WorldSceneTool_Animation.syntax.json`
