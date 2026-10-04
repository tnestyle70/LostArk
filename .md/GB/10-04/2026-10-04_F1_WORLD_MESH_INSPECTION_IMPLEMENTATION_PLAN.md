# F1 World Scene Tool IMPLEMENTATION PLAN

## G00. 목표와 현재 기준점

Bern/Character Select/Valtan/Kouku의 실제 Level-owned map/Deploy를 별도 F1 창에 연결한다.
Map Tool의 authoring attach와 GPU world-position/AABB 추정에 의존하지 않고 기존 CModel
LOD0/static/current-skeletal 삼각형에서 최근접 placement/mesh/material을 식별한다.
선택은 데이터를 변경하지 않으며 source actor/level/asset/WModel/hit 근거를 복사한다.
목록 검색·선택과 Focus를 함께 제공한다. alpha·shader displacement·rendered LOD·animated
raster cull은 CPU geometry와 구분하고 실제 게임 화면 판정은 사용자가 한다.

현재 branch의 기존 gameplay/rendering 작업과 수동 Bloom/Fog/FXAA 등 모든 팀장 옵션을
보존한다. 새로운 모델 runtime, 다른 level, 원본 단위의 추정 animation을 만들지 않는다.
기존 IMapAuthoringHost, CMapPlacementRuntime, CMapPlacementEditSession 및 Deploy
Begin/Sample/End preview 계약을 확장한다.

## G01. 새 파일과 H 계약

`Client/Public/WorldSceneTool.h`는 Debug 창과 선택/편집/preview 상태를 소유한다.
MapPlacementEditSession include는 authored draft/Save 계약, DeployPropRuntime include는
typed mesh snapshot/weak live object, optional/vector/array는 선택·목록·검색 수명에 필요하다.
ROW.id는 stable placement ID이며 deploy lane를 함께 사용해 두 domain의 숫자 충돌을 피한다.
mesh UINT32_MAX는 목록 placement 선택이고 exact mesh/material은 실제 triangle pick만 채운다.

Open/Hide/Update/Render의 호출자는 MainApp이며 Consume_*는 pick/focus/interaction의
one-shot 명령을 전달한다. Complete_MapPick/Complete_DeployPick은 성공 결과만 commit한다.
Host는 현재 level/area의 live owner를 확인하고 Refresh_Rows/Select_*/Render_*는 해당 창의
목록과 inspector를 관리한다. Begin_Editing은 MapCatalog의 source paths로 기존 session을
bind한다. m_Edit는 전체 authored draft, m_Undo/m_PendingUndo는 drag gesture 시작 pose,
m_RuntimeGeneration은 같은 host 객체 내부 reload 경계를 소유한다.

`Client/Private/WorldSceneTool.cpp`는 독립 F1 UI와 검색·선택·static placement edit 흐름이다.
`Client/Private/WorldSceneTool_Animation.cpp`는 기존 Deploy clip/root preview와 self-motion
clock 제어만 소유한다. weak object와 area/level/host/generation이 유효하고 자기 Begin이
성공한 경우에만 Sample/End한다. Begin 전 root/clock을 캡처하고 Stop/Hide에서 복원한다.
source-exact Deploy는 영구 저장하지 않으며 positive uniform scale와 visibility overlay만
가역 적용한다. product surface packet과 Server state는 이 창의 편집 문서가 아니다.

## G02. 기존 파일의 변경 경계

- MainApp.h/.cpp/_WorldLevel.cpp: WORLD_SCENE enum, Ensure/visibility/focus/Render/Update,
  one-shot world ray 입력과 gameplay LB/RB 소비를 연결한다. 다른 피커/Move Player는
  다음 click owner를 하나로 유지한다. UI capture와 Esc/우클릭/Level 변경은 요청을 취소한다.
- MapAuthoringHost 및 네 Level: generic existing runtime reference와 map/Deploy 소유자를
  제공한다. optional source mapmotions만 Load/Update하고 absence는 0 rows다.
- MapAssetObject/MapStaticBatchObject/MapPlacementRuntime: static/batch live visibility,
  suppression/world/cull을 사용하고 foliage도 포함해 exact mesh/material을 반환한다.
  기존 이동 query의 foliage 제외 정책은 보존한다.
- DeployPropObject/Runtime: 현재 intact/fractured 또는 current skeleton pose를 선택한다.
  기존 preview에 static/bind-pose pose-only, positive uniform root scale, opacity/Reveal
  overlay를 연결한다. valid authoritative state/debris/suppression은 preview를 End한 뒤
  적용하며 invalid event와 physics/debris 경계를 보존한다.
- Shader_VtxAnimMeshBinary.hlsl: dedicated Deploy opacity overlay의 coverage를 소비한다.
  기본 1은 기존 draw이며 공유 shader binding은 draw 뒤 복원한다.
- MapPlacementEditSession: 실패 전 draft를 삭제하지 않는다. partial-live bind는 source의
  모든 행을 보존하고 live ID/asset parity를 검증한다. Save는 sampled live pose 대신 authored
  draft를 사용하며 off-scope 행을 보존한다. detached draft 삭제는 명시적 UI 명령이다.
- MapPlacementDocument::Write: optional expected source bytes를 받는다. unique temporary
  staging 뒤 atomic replacement 직전에 비교하고 stale source를 덮어쓰지 않는다.
  parse 시점 source와 baseline bytes도 일치 확인한다.
- MapPlacementRuntime: standalone Visible 적용을 수정하고 Debug clock pause/rate/Seek와
  runtime generation을 제공한다. Clear는 motion/cache/clock을 초기화한다.

## G03. 호출 흐름과 불변식

`F1 → World Scene Tool → Pick in world → current viewport ray → live map/Deploy nearest hit
→ typed source snapshot → inspector/clipboard`. camera depth 밖의 geometry는 클릭 선택하지
않으며 miss는 이전 선택을 유지한다. GPU pixel 판정을 exact CPU geometry와 혼동하지 않는다.

`Enable editing → MapCatalog source paths → parse/validate/stage → source/live IDs bind
→ authored TRS/Visible Apply → Undo/Reset → Save → unique temporary/freshness/atomic source
commit → official Area publisher → publish failure guarded rollback`. phase/backdrop/borrower
경계는 기존 계약을 따른다. duplicate만 Delete하며 원본 행은 Visible로 identity를 유지한다.

`Preview Begin → captured current root/clock → clip or pose/visibility overlay Sample
→ Stop/Hide/authoritative preemption → baseline restore`. 신규 product packet이나 Server
gameplay 상태를 생성하지 않는다. generation 변경 때 새 runtime에 이전 clock을 쓰지 않는다.

## G04. 등록과 종료 증거

새 H/CPP 3개를 Client.vcxproj와 기존 WorldTools filter에 필요한 항목만 등록한다.
기존 UTF-8/CRLF 및 파일별 인코딩을 보존하고 XML parse를 검증한다.
관련 실제 C++ TU 컴파일, Product Debug/Release 정상 증분 build/link, existing movement
regression과 production geometry picker, source write/freshness/scope/draft·Deploy preemption
회귀를 실행한다. JSON/XML parse와 git diff --check를 확인한다. 광역 진단을 commit 조건으로
추가하지 않는다. Client/UI는 에이전트가 실행하지 않으며 최종 viewport/animation/Save 재진입
판정은 사용자 확인으로 RESULT에 분리한다.

## G05. 사용자 종료 보고와 선택 진입 통합

사용자가 World Level Tool의 편집 진입 뒤 EXE 종료를 보고했다. 현재 시점 종료 원인을
기록한 crash dump는 없으므로 OOM으로 확정하지 않는다. 기존 World Level 선택은
전체 재질 문서를 읽는 별도 placement edit session을 선행하며, World Scene의 실제
triangle 선택과 중복된다. World Level의 맵 선택·편집 진입을 World Scene으로 연결하고
이중 session과 GPU world-point 기반 placement 추정을 제거한다. Guide의 위치 선택은
별도 계약으로 유지한다. saved inventory와 composition owner 연결은 보존한다.

World Scene의 scene pick은 편집용 문서를 열지 않고 현재 Level의 map/Deploy를 읽는다.
현재 Area의 inventory 행도 같은 inspector로 열고 Test Level 이동을 요구하지 않는다.
placement 편집을 명시한 때만 source ID·전체 미로드 행·freshness를 검증하는 draft를
준비한다. 이 준비에서 이미 로드된 runtime 재질을 재사용해 대형 JSON DOM 재생성을
피하고 실패 시 기존 draft와 선택을 보존한다. saved source와 live identity를 검증하지
않은 임의 material 생략이나 rendering 옵션 변경은 하지 않는다.

기존 파일만 수정하며 새 프로젝트 등록은 없다. source/runtime identity 실패·부분 로드
저장·draft 보존의 기존 좁은 회귀와 변경 TU 및 Product Debug 컴파일/링크를 확인한다.
실제 사용자 클릭 후 종료 재발 여부와 선택 결과는 사용자의 Client 화면 확인으로 남긴다.
