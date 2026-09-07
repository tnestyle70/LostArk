# Kouku 저작 기능과 main 통합 결과

## G01. 통합 범위

2026-09-08 사용자가 렌더링 후속 작업보다 충돌 해결·PR·merge·pull을 우선하도록 요청했다.
로컬 `13fa34d7` 기반 변경을 `codex/integrate-kouku-authoring-and-materials`에서 재질 복구
`ecdc681f`, 쿠크 번들·월드·이펙트 저작 `6b0d9ba2`로 나누어 커밋한 뒤 `main`의 `fe60c548`을 병합했다.
실제 Git 충돌 20개 파일을 해결했다. Git 쓰기를 막던 전날의 빈 `index.lock`은 실행 중인 Git이
없음을 확인하고 `out/GitIntegration20260908/stale-index.lock`으로 보존했다.

## G02. 충돌 해결 결과

- Shared protocol 68. main의 패킷 72~76과 bundle 77을 함께 유지한다. WORLD operation은
  PLAY=0, REPLAY=1, STOP=2, STOP_OWNER=3이며 필드와 직렬화는 한 번만 존재한다.
- Server 함수·friend·Main의 검사 인자를 통일했다. Replay는 main과 동일하게 책 연출 파티를
  재배치하고, Stop·기존 객체 motion은 위치를 바꾸지 않는다. 실제 Broadcast 회귀 검사로 확인했다.
- Client는 일반 Replay/Stop과 run/member Stop을 구분한다. 독립 객체 소유권, main의 소품·카메라
  정리, Mario·Sequence Viewer·Customizing 흐름, 로컬 Effect CPU 초안·orbit/polar를 함께 유지한다.
- Character Select 광원 소유자를 하나로 통합했다. 사용자가 튜닝한 Point86·Spot99를 유지하고
  중복 원본 광원 및 이미 lightmap에 포함된 광원을 추가하지 않았다. main의 저장된 카메라 조정은 유지했다.
- 쿠크 데이터는 stable ID로 합쳤다. worldsequences revision 400, objectResources 10,
  templates 91, instances 95이며 Gameplay 배치는 67개다. 사용자 저작값과 main의 Mario 추가 항목을 유지했다.
- 프로젝트 등록을 양쪽 모두 유지하고 filters의 U+0004 두 곳을 정상 UI 경로로 고쳤다.
  runtime 문서는 합쳐진 authoring에서 각 domain publisher로 다시 생성했다.

## G03. 자동 검증

| 검증 | 결과 |
|---|---|
| 최종 Debug Product | Engine → Shared → Server → Client 및 SDK·shader·DLL 배포 PASS, missingRuntimeInputs 0 |
| NetworkProtocolHarness | 1,027 assertions PASS, failures 0 |
| Server world playback / bundle / object overlap / support surface | 12 / 23 / 36 / 11 PASS, 합계 82 |
| 기존 Effect·Map·Rendering·WModel 저장 계약 | 37 PASS |
| 유지한 program 0~5 shader WARP | 198 PASS; 이전 공통 180항목 수치 차이 0 |
| JSON/XML parse | 62개 변경 파일 PASS, 프로젝트 중복 등록 없음 |
| Git | origin/main 대비 최종 diff --check PASS, conflict marker 0 |

최종 빌드 증거는 `out/BuildPipeline/runs/20260907T175400952Z-debug-product.json`이다.
첫 통합 빌드 `20260907T175131758Z`도 통과했으며 이후 Replay 보정 때문에 최종 빌드를 다시 수행했다.
세부 명령·출력은 `out/GitIntegration20260908/`에 있다. 초기 model 테스트 수집 실패는 PYTHONPATH를
바로잡아 같은 네 검사로 통과했고, 제품 코드 실패로 분류하지 않는다. 컴파일에는 기존 C4819 경고가 남는다.

## G04. 보존한 작업과 수동 검증 경계

재질은 기존 바닥 6배치와 중앙 링 15배치, program 0~5까지만 포함한다. 전체 맵 재질 계열과
투명·발광·COLOR0 확대는 보류했다. 미완성 shader·parser·vertex 초안은
`out/GitIntegration20260908/pre_integration/`의 원본 208파일 스냅샷과
`out/CharacterSelectFullRestore20260908/`에 보존했다. 전체 맵 복구 완료로 보고하지 않는다.

새 물리 입력은 `Client/Bin/Resources/Map/CHARACTERSELECTMAP/`의 FLOOR12 두 variant,
BRIDGE01E 두 variant와 MAGICFLOOR03D의 `_OVR_43CE9773CA87` 폴더,
`Client/Bin/Resources/Map/Lighting/CharacterSelect/`에 있다. 정확한 Resources-relative ID는
기존 FLOOR_MATERIAL_RECOVERY_RESULT의 G11/G12 및 Git 관리 mapmaterials에서 확인한다.
Resources는 Git에 추가하지 않았으며 Drive 전달은 미수행이다.

Client/UI는 실행·조작하지 않았다. Server는 종료되는 계약 검사 모드만 실행했고 공유 listener를
새로 시작하지 않았다. 사용자 게임 화면·원작 유사도·4인 실제 플레이 검증은 남아 있다.
