# 쿠크 재생 복구와 입구 표식 수정 결과

## G00. 적용 범위와 완료 경계

현재 브랜치 `codex/character-select-200fps`에서 기존 성능·이펙트 작업을 보존했다.
Complete Play의 게시 불일치, Sequencer 초기 WORLD 준비와 시작 시계, G2 fade/light 앵커,
입구 이동 표식의 소유자, mouse_click 두 LocalDecal의 시작 공백을 수정했다.
Client/UI 실행·조작·캡처를 수행하지 않았다. 화면 미출력의 유일한 원인을 재현했다고 기록하지
않으며, 사용자의 실제 시퀀스·전투·화면 확인은 남아 있다. 최종 Debug Product 빌드·배포는 PASS다.

## G01. Complete Play와 Composition Sequencer

| 실제 확인 | 수정 |
|---|---|
| 저장332와 게시331 차이로 Complete Play가 거부됨 | 공식 Owner KoukuSaydon 게시로 Composition/Encounter/patternbindings/Server bootstrap332 일치 |
| 변경은 빈 G3 `세이튼_쇼타임` P32 초안이며 기존 실행 내용은 같음 | 초안 보존, 실행27패턴·202stage·8bundle에 빈 초안은 포함하지 않음 |
| 열려 있는 깨끗한 Workbench도 이전 revision을 가질 수 있음 | MainApp 재생 준비에서 실제 저장 Composition을 Reload해 게시 revision과 비교 |
| F1 재생 패널의 게시 안내가 다른 도구로 이동을 요구함 | 같은 패널에 Publish Saved Patterns 버튼 추가, 기존 typed one-shot와 Workbench publisher 사용 |
| 독립 WORLD마다 같은 전체 문서와 mapplacement를 읽고 검증함 | `Set_DocumentBatch`로 요청당1회 검증, G2 17회→1회; 각 player의 문서 복사·리소스·시계는 독립 유지 |
| Begin이 Presentation Update 뒤에 실행돼 다음 dt에 동기 준비 시간이 섞임 | own-clock의 첫 pivot-ready Update에서만 준비 delta 제외; 두 번째 이후 delta는 그대로 사용 |
| G2 fade/light가 빈 WORLD 앵커를 찾아 계속 기다림 | 독립 Sequence revision5의 두 occurrence와 defaultAnchorKind를 MAP으로 수정; 생성 도구도 동일 |
| 재생 실패가 Timeline에서 잘 보이지 않음 | Sequence/Bundle Play 아래에 Preview status 표시 |

새 batch API는 null/중복 destination과 미완성 target을 거부한다. 전체 Validate와 모든 문서
복사가 끝나기 전에 기존 player를 바꾸지 않는다. 예외로 복사가 실패해도 기존 player가 유지된다.
수정 호출자는 Level의 단일 Sequence와 PresentationPlayer의 Bundle WORLD 준비다.

own-clock 시작 플래그는 Begin/Bundle에서 설정하고 첫 유효 Update와 Stop에서 해제한다.
Seek/Pause/Resume 자체는 플래그를 바꾸지 않으며, 외부 Animation Tool 시계와 ModelReference
샘플링은 유지한다. 모든 frame delta를 임의로 제한하는 방식이 아니다.

게시 명령과 검증 증거:

- `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 332`
- `out/KoukuSaydon/publish332-20260912.log`: product/world.gameplay/gameplay.balance PASS, map REUSED.
- `out/KoukuSaydon/publish332-check-20260912.log`: 기존 projector `--check` PASS.
- `out/KoukuSaydon/publish332-semantic-check-20260912.json`: 기존 실행 내용과 PlayAll 보존, 빈 초안 제외.

진행 중 Server에332가 실제 승인됐다는 검사는 수행하지 않았다. Client/Server를 임의 종료하거나
재시작하지 않았다. 현재 데이터에 맞는 새 실행에서 Server 권위 재생 확인이 필요하다.

## G02. festival·책 재질·아레나 전환 재검토

현재 저장된 독립 Sequence의 G1은 `연출_팝업북`37.8초와 `연출_1관문 피날레`21.01초 두 개다.
각각 WORLD7+CAMERA1, WORLD1+CAMERA1이며 festival/fireworks EFFECT나 companionEffect 직접
연결은 없다. Effect Tool의 source-model preview 추가 diff도 기존 G1 카메라/clock 경로를
대체하지 않는다. festival 추가를 카메라 중단 원인으로 단정할 근거는 발견하지 못했다.

G1 카메라2개와 WORLD8개를 authored/runtime에서 대조했다. instance enabled/template 연결이
유효하다. 팝업북 WMA2의 materialName3개는 mapMaterialBindings와 일치하고, 복원 donor2개와
재질3행이 게시 catalog에도 존재한다. 필요한 surface DDS 입력7개 역할의 파일/header/크기를
확인했다. SOURCE_BG 재질은 실제 animated pass6 경로로 전달된다.

`Has_MaterialTexture(DIFFUSE)`가 surfaceDiffuse를 직접 보지 않는다는 후보도 조사했지만,
CMaterial은 그 전에 legacy diffuse 또는 solid fallback을 생성한다. 이 차이만으로 준비를
거부하는 결함은 아니므로 해당 검사나 재질을 임의로 바꾸지 않았다.

WORLD 준비/샘플링 실패 시 전체 Composition을 중단하는 경로는 유지한다. 부분 맵만 보이며
카메라만 진행하도록 오류를 숨기지 않는다. 실제 실패 문구는 이제 Play 하단에서 확인할 수 있다.
책 모델과 전투 아레나의 기존 표시 전환·플레이어 이동 계약은 이번 수정에서 변경하지 않았다.
재질/조명 동일성이나 원본과의 시각 일치에 대한 사용자 승인은 아직 없다.

## G03. 우클릭과 입구 표식

`CClickMoveEffect`는 명령 제출 성공 뒤 `effect.world.mouse_click`만 재생한다.
`move_destination`의 클릭 retarget/도착 감지 상태를 제거하고, 기존 Loader 준비는 유지했다.
입구 표식은 `CLevel_KakulSaydonArena`가 별도 prepared handle5개를 소유한다.

- `jump.1`, `jump.2`, `jump.3`, `paper.1`, `paper.2`의 실제 Gameplay.world.json position을 읽는다.
- Trigger_Box가 사용하는 collider 중심과 같은 좌표다. 좌표 복제나 바닥 높이 추정은 없다.
- disabled trigger는 제외하고 기존 Seek_WorldRoot로7초 창을 반복한다.
- 일회용 paper는 Server PLAY/REPLAY sequence 이벤트를 수신하면 제거하고, jump는 반복한다.
- Level 퇴장/파괴 시 모든 handle을 정리한다. 실패는 해당 장식에 격리한다.

지속적인 trigger hasFired late-join snapshot이 현재 없으므로, 새 Client의 초기 표식 eligibility는
enabled Data 기준이다. 이미 실행된 paper의 이력을 새 접속자가 받는 기능은 이번에 추가하지
않았다. 수신한 Server 이벤트 이후의 제거는 연결돼 있다.

## G04. mouse_click 두 LocalDecal

실제 Playback CPU 샘플에서 두 decal은 첫 fixed tick16.67ms에 age0으로 생성된다.
첫 decal이 늦어 보이는 원인은 native material의 환형 마스크와 실제 DDS가 초기 source alpha=.5에서
겹치지 않는 구간이었다. 두 번째 decal에는 별도로 burst0.2초도 저장돼 있었다.
이 재질의 alpha는 일반 투명도 대신 파동 반경 입력이므로 Life만 줄여도 시작 공백은 남는다.

`Data/Effects/Authored/effect.world.mouse_click.effect.json`의 변경은 다음3줄이다.

- particlespriteemitter_3의 `detail.color.multiply.w`:1→.4.
- particlespriteemitter_4의 같은 값:1→.5.
- particlespriteemitter_4의 첫 burst 시간:.2→0.

사용자 `sourceScale.lifeTime=.5` 두 개, native source module curve, shader와 공용 Playback/Distribution
구현은 그대로 보존했다. 현재 첫 tick의 실제 alpha는 각각.20/.25, size는.8/1이다.
이는 원본 파일 변경이 아닌 두 요소의 프로젝트 저작 튜닝이며 최종 보이는 크기·깊이는 사용자 확인 대상이다.

## G05. 검증과 실행 준비

- 마커 CPP4개 격리 컴파일 PASS: `out/WorldMarkers20260912/compile.log`.
- 현행 실제 Playback 재컴파일 CPU 검사 PASS: `cpu_probe_run.log`, `decal_samples.tsv`.
- 실제 trigger root5개 × .25/6.99/7초 wrap 직후 샘플의 위치 오차≤1mm, normalized age 차≤.0001.
- 실제 DDS와 native mask의 수치 비교: `out/WorldMarkers20260912/decal_radial_mask_samples.json`.
- 마커 전체 근거: `out/WorldMarkers20260912/result.json`.
- 변경 JSON/XML8개 parse, Gate2 생성 Python syntax, 현재 Sequence Effect anchor 검사 PASS.
- 전체 `git diff --check` PASS. 기존 LF/CRLF 안내는 유지하고 무관한 파일을 정규화하지 않았다.
- 최종 Engine/Shared/Server/Client Debug Product 컴파일·기본 경로 배포 PASS: `out/KoukuSequencerPlay20260912/product-build.log`.
- 빌드 영수증: `out/BuildPipeline/runs/20260911T165536712Z-debug-product.json`.
- 실제 함수 본문 기반 batch/clock OUT 컴파일·실행 PASS: `out/KoukuSequencerPlay20260912/world_batch_probe.run.log`.
  17개 player의 Validate1회, copy 중 실패/invalid destination/원본 alias 보존을 검사했다.
  첫60초 준비 delta는0초를 유지하고, 후속 .5초는500ms로 진행하며, 그다음 큰 delta는 정상 종료한다.
  Seek/Pause/Resume/paused Begin/pivot 지연/external clock/Bundle/Stop/ModelReference도 검사했다.
  이 검사는 실제 본문과 stand-in 상태를 사용하며 GPU resource 준비 시간의 측정은 아니다.
- 기존 비UI Sequence document contract에 현재 revision5 입력을 넣어 PASS: `sequence-document.run.log`.
  실제 seed·문서 분리·원자 Save/Reload·CAS·Complete Sequence 순서/0초 시작/일시정지/실패 취소를 검사한다.
  해당 도구의 기존 실행 파일은 문서 검증에 사용했고, 제품 변경 CPP는 위 새 Product 빌드로 확인했다.

새 C++ 파일이나 project/filter 등록은 추가하지 않았다. 기존 사용자/다른 작업 diff를 자동 stage,
commit, push하지 않았다. 준비된 자료를 GPU pixel 성공이나 사용자 visual PASS로 기록하지 않는다.

사용자가 직접 확인할 순서:

1. Server + Client profile로 새 Debug 실행을 시작해 Lobby → KoukuSaydon으로 입장한다.
2. F1 → Composition Sequencer에서 G1 팝업북 Play, 이어 피날레 Play를 확인한다.
3. G2 진입 컷신의 카메라/맵/암전·조명을 확인한다. 실패하면 Play 바로 아래 상태 문구를 확인한다.
4. F1 → KoukuSaydon Complete Play → Reload KoukuSaydon Inventory 후 Selected Pattern,
   Composition Play All 또는 Complete Play - Sequences + Pattern Flow를 확인한다.
5. 입구 우클릭에는 mouse_click만 나오고, 이동/종이 트리거 중심에는 금색 표식이 반복되는지 확인한다.
6. 두 LocalDecal 링의 첫 표시와 Stop/Level 퇴장 후 정리 상태를 확인한다.
