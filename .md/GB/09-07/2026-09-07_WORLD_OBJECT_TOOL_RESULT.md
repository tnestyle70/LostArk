# 2026-09-07 World Object Tool 구현 결과

대응 계획: [World Object Tool 구현 계획서](2026-09-07_WORLD_OBJECT_TOOL_IMPLEMENTATION_PLAN.md).
구현·자동 검증·데이터 배포를 완료했다. Client 실행, UI 입력·저장 왕복과 아레나 화면 판정은 사용자 확인 전이다.

## G01. 저장 리소스와 상태

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`이다.
formatVersion 3, revision 121에 Object Resources 10개를 저장했다. 모델 리소스 8개와 기존 sequence alias 2개다.
기존 56 templates/60 instances를 보존하고 상태 11개를 추가하여 현재 67 templates/71 instances다.

| 저장 이름 | 실제 입력/상태 |
|---|---|
| 월드오브젝트_카드 | NPC 480642 → MN_RHOC_00 모델·재질, 기본 상태와 `카드_들썩임`, `카드_뒤집힘` |
| 월드오브젝트_조커카드 | NPC 480643 → MN_RHOC_00-1, 같은 카드 골격에 조커 재질 |
| 월드오브젝트_공 | 기존 Effect의 fm_k_ppct_ball_01 메시와 wp_mn_ppct_00_c DDS, 기본 상태와 `공_튀기기` |
| 월드오브젝트_세토 | NPC 480708/480709 → MN_PPCT_00 |
| 월드오브젝트_칼날 | Cutting projectile 421991301 → Par_V_RPCT_Cutting_pjt_01 → fm_o_cngn_01 메시·mn_cngn_00_mi 재질 |
| 월드오브젝트_갈고리 | NPC 480710 → MN_UMAX_00 |
| 월드오브젝트_빙고폭탄 | NPC 480724 → MN_RHCN_01 |
| 월드오브젝트_빙고 | 기존 Map의 LV_LUT_MIDNIGHTC_FLOOR03_SM |
| 월드오브젝트_커튼 | world.sequence.instance.curtain_drop, 기존 11개 Map placement·키 보존 |
| 월드오브젝트_룰렛 | world.sequence.instance.8, 기존 placement 40·33,773ms 상태·키 보존 |

카드 동작 두 개는 실제 모델에 적용할 Transform 키로 저작했다. 공 튀기기는 2초 동안 위쪽 속도 6m/s,
가속도 -6m/s²로 1초에 정점에 도달하고 출발 높이로 돌아오는 초기 저작값이다. 최종 타이밍과 크기는 화면 확인 전이다.
새 상태의 기본 anchor는 WORLD이며 party01 spawn의 Z+2m인 `(3.28999996, 8.64000034, -8.6899996)`에 둔다.
원작 배치 복원값이 아닌 도구 저작 시작 위치다. `Use Current Character Position`으로 현재 위치를 가져올 수 있다.

카드/조커/세토/갈고리/폭탄은 기존 ActorX → CModel cooker로 준비했다. 새 physical package는 6개,
파일 33개, 51,146,900 bytes이며 Resources 파일은 Git에 추가하지 않았다. 모델을 named state마다 복제하지 않는다.
정확한 Resources-relative 경로와 원본 연결 근거는 [리소스 기록](2026-09-07_KOUKU_WORLD_OBJECT_RESOURCE_LEDGER.md)에 있다.
6개 폴더는 현재 PC에 설치했고 Drive로 복사 가능한 상태다. Drive 업로드는 실행하지 않았다.
칼날의 원본 연결은 확인했지만 사용자가 뜻한 3관문 회전 톱날과의 시각 동일성은 승인하지 않았다.

## G02. 도구와 재생

F1 → World → `World Object Tool`을 추가했다. 상단은 저장 Object Resources와 상태 목록,
가운데는 Detail·Sequencer, 하단은 실제 Physical Resources 폴더다. Effect/Map/Deploy/Character의
`.wmodel`/`.dds`를 전체 상대 경로로 검색하므로 Effect catalog에 없는 파일도 선택할 수 있다.
Effect Tool도 같은 물리 검색 함수를 사용하며 같은 이름의 다른 경로를 한 항목으로 합치지 않는다.

Detail은 모델·diffuse·import scale·Object scale, 상태명, World/Character anchor, 위치,
delay/lifetime/speed, velocity/acceleration/self rotation/revolution, count/interval/spread/seed를 편집한다.
CompositionTimeline의 ruler·box를 사용하며 Transform key를 추가/삭제/이동하고 애니메이션 clip 창을 편집한다.
커튼·룰렛의 Detail은 기존 template를 직접 편집한다.

Save는 같은 CWorldSequenceDocument codec으로 stage → validate → readback equivalence → source CAS → atomic replace한다.
source와 Map/Deploy placement가 외부에서 바뀌면 draft와 기존 저장본을 유지한다. Publish Area는 기존 Map publisher를
숨겨진 process로 호출하고 성공 뒤 runtime을 다시 읽는다. Publish 실패는 오류와 로그 경로를 남긴다.
성공한 저장본만 Action Workbench → Resources → World 목록에 공급하며 선택 상태를 Append한다.

실행은 기존 CWorldSequencePlayer를 확장했다. CWorldSequenceObject는 기존 Prototype/Clone/Layer와
CModel/CMaterial을 사용한 표현 객체다. Player가 시간·위치·방향·생성·수명·정리를 소유한다.
모델 prototype와 DDS SRV를 캐시하고 clone별 pose를 샘플한다. 다른 상태의 clip은 캐시가 있어도 다시 검사한다.
DDS 실패를 성공으로 취급하지 않고 원래 material 또는 명시 override를 사용한다.
생성 한도 초과와 샘플 실패는 해당 상태를 정리한다. Stop·종료·Level 퇴장 시 생성 객체를 제거한다.

Character anchor는 살아 있는 복제 플레이어마다 생성한다. 방향은 플레이어 basis를 따르되 모델 scale이
offset을 확대하지 않도록 정규화한다. World anchor는 저장 위치를 사용한다. 복수 생성은 seed와 emission index로
방향을 결정하여 같은 시각의 seek가 같은 결과를 갖는다. template lifetime은 전체 생성 창이다.

World box의 실제 `durationMs`를 projector → Gameplay bootstrap → Server → Shared WORLD packet → Client로 연결했다.
protocol은 63이며 0인 기존 직접 호출은 sequence의 authored duration을 유지한다. box 종료 시 동적 객체를 정리하고
Map/Deploy의 preview 소유권을 반환한다. 툴/Composition preview도 같은 Player를 사용한다.
룰렛 중심 복사는 live effect pivot과 별개인 읽기 전용 원배치 조회를 사용한다.

기존 Effect WORLD anchor는 살아 있는 단일 object의 pose를 참조할 수 있다. 여러 object 각각의 폭발 이벤트와
effect 동반 생성은 아직 저작하지 않았다. collision/damage/패턴 판정은 기존 Server 권위 경계에 남는다.

## G03. 실행한 검증과 배포

검증 로그는 `out/WorldObjectWork/`에 보관하며 커밋 대상이 아니다.

| 검증 | 실제 결과 |
|---|---|
| 최종 Debug Product | Engine → Shared → Server → Client 컴파일·링크, SDK/shader/runtime DLL 배포 PASS. product-debug-complete.log |
| 빌드 receipt | out/BuildPipeline/runs/20260906T161032657Z-debug-product.json. 모든 step PASS, missingRuntimeInputs 없음 |
| 기존 Python 검사 | WorldSequence Map 계약 26개 + Kouku Composition 49개 = 75/75 PASS. composition-world-tests.log |
| NetworkProtocolHarness | Debug x64 빌드·실행 PASS, failures 0. WORLD lifetime 0/2400ms 왕복과 600001ms 거부 포함. network-build.log, network-run.log |
| Map Publish | v3/revision121, 3,231 placements/7 files, Object 10개, 모델·override 파일 누락 0. map-publish.log |
| source/runtime 일치 | WorldSequence JSON semantic equality PASS. map-source-summary.json, map-runtime-check.json |
| 기존 데이터 보존 | 이전 56 templates/60 instances 동일, 새 항목만 append. seed-verification.json |
| 물리 리소스 | 모델 8개, CMaterial texture 참조 30개 및 공 diffuse override 존재, 저장 animation track 7개의 clip resolve PASS. resource-closure-verification.json |
| 모델 animation rate | animated 모델 5개/clip 110개 원본 PSA rate 복원과 설치 byte 일치 PASS. animation-rate-verification.json |
| Composition Publish | revision76, Product pattern6/stage66/output2. composition-publish-final.log |
| Gameplay Publish | WORLD cue의 duration 포함 bootstrap 배포 성공. gameplay-publish-final.log |
| JSON/XML | dirty/new JSON 49개와 프로젝트·filters XML 4개 parse PASS, Client ProjectReference GUID 2개 정상. final-structure-check.json |
| 공백 | 전체 git diff --check PASS. final-diff-check.log |

첫 컴파일에서 신규 Object include 순서 문제를 수정한 뒤 최종 빌드까지 통과했다.
기존 C4819/LNK4099 경고는 남아 있으며 무관한 인코딩·라이브러리를 일괄 변경하지 않았다.
Runtime UI Save/Reload 버튼 왕복, GPU 렌더 결과와 패턴 실제 재생은 실행하지 않았다.

## G04. 사용자 실행과 남은 확인

이 PC는 LAN 검사에서 server-host이며 TCP 7777 LocalSubnet 방화벽과 endpoint `192.168.0.4:7777` 설정을 확인했다.
종료 확인 시 Server/Client process는 실행되어 있지 않다. Visual Studio에서 `Server + Client` profile을 선택해
사용자가 Ctrl+F5로 시작한다. protocol 63으로 양쪽을 다시 실행해야 한다.

1. Lobby → KoukuSaydon → F1 → World → World Object Tool을 연다.
2. Object Resources에서 저장된 10종을 선택한다. 카드의 `카드_들썩임`/`카드_뒤집힘`, 공의 `공_튀기기`를 선택한다.
3. 필요하면 Use Current Character Position을 누르고 Play/Pause/Clock/Stop으로 크기·방향·키·수명을 확인한다.
4. Physical Resources에서 모델/texture 슬롯을 선택하거나 Detail을 튜닝한 뒤 Save → Reload Source로 왕복을 확인한다.
5. Publish Area 후 Action Workbench → Resources → World에서 상태를 선택 → Append한다.
6. box 시작·Lifetime·offset·speed를 설정하고 저작 preview를 확인한다. Save → Publish All PRODUCT 후
   Server를 재시작하고 Play Published Product (Server)로 해당 패턴을 재생한다.
7. 커튼·룰렛 기존 동작, Character anchor 추적, 여러 공의 방향·개수, 종료 시 제거를 확인한다.

사용자 화면 확인 전이므로 visual PASS는 기록하지 않았다. 새 리소스를 각 제품 패턴에 자동 배치하거나
원작의 모든 패턴 타이밍·충돌·폭발 effect를 완성했다고 처리하지 않는다.
기존 대규모 dirty 변경은 보존했으며 자동 stage/commit/push는 하지 않았다.

## G05. PR #333 최신 main 통합 검증

사용자가 커밋·push한 `b56347879e73ce6a50f45a7e923d94505bfe3175`와
main `0f05b7c5dbc33af48756b9b99fbd1eec7fd4eef4`를 병합했다.
10개 충돌 파일은 양쪽 계약을 합쳤다. 마리오 이동·상호작용·카메라·네비 세부 영역,
커스터마이징·무력화 UI와 World Object·조명·HUD 변경을 유지한다.

WorldSequence는 v3/revision147, Object Resources10/templates75/instances79다.
Gameplay는 revision8094/placements64이며 main의 추가32개와 기존 수정, 사용자의 보스 좌표와
Mario1 HUD 값을 보존했다. Map publisher로 7개 파일을 재생성했고 1Mario camera도 runtime revision62로 맞췄다.

main의 상호작용 packet ID와 기능 브랜치의 HUD/Scene ID 충돌을 해소했다. main 기존68개 ID를 유지하고
HUD/Scene4개를 뒤에 추가한 protocol64를 사용한다. WORLD duration codec도 그대로 유지한다.
자동 병합된 커스터마이징 종료 코드의 옛 카메라 함수 두 곳은 현재 ArenaCameraProfile을 소비하도록 수정했다.

Navigation publisher는 이미 Server가 보존하는 Big Saydon의 저작 높이를 지면 높이로 강제해 배포에 실패했다.
정확한 쿠크 Area+boss kind+Big Saydon archetype의 높이 정책을 Server와 맞췄으며
finite XYZ·bounds·walkable 검사는 유지했다. 사용자 저장 `(10.24,8.63,317.75)`는 변경하지 않았다.

| 검증 | 결과 |
|---|---|
| 최종 Debug Product | PASS, out/BuildPipeline/runs/20260907T015810529Z-debug-product.json |
| 관련 Python 검사 | WorldSequence/Composition/Effect binding/World admission 103개 PASS |
| NetworkProtocolHarness | protocol64·WORLD lifetime·interact packet, failures0 |
| Navigation ContractTest | 기존 검사와 높이 정책 정상/거부7경우 PASS |
| Map/World/Navigation Publish | PASS, 네비5개 Area+Mario 세부격자3개, Client/Server25파일 일치 |
| 구조 | 병합 JSON23개/XML2개 parse, 두 기능의 project/filter 등록·GUID 검사 PASS |
| 충돌·공백 | unmerged path0, origin/main 대비 PR diff --check PASS |

통합 검증 로그는 `out/WorldObjectMerge/`, 데이터 보존 감사는 `out/PRConflictMerge/`에 있다.
이전 main 문서·참고 TXT의 기존 공백은 무관한 변경으로 정리하지 않았다.
Client/UI 실행·화면 검증은 하지 않았다. 사용자 요청에 따라 PR merge 뒤 로컬 main을 pull하여 동기화한다.


## G06. 09-07 독립 패널·미리보기·카메라 실시간 반영

작업 브랜치는 codex/world-object-light-workspaces다. 사용자 검증 후 F1 메뉴를 Tools로 옮기고,
왼쪽 Object Resources / 아래 Object Sequencer / 오른쪽 Object Detail을 독립 ImGui 창으로 나눴다.
각 창은 이동·크기 조절·닫기·Windows 메뉴 재열기·Reset layout을 지원한다. Resources의 Map/Character 분류와
Create anchor는 resource.anchorKind(WORLD/PLAYER)에 저장하며, 리소스 anchor 변경은 연결 상태에도 반영한다.
이전 v3 문서의 무필드 리소스는 WORLD로 읽는다. 커튼/룰렛 alias는 Map에 고정한다.
Create는 용량·Map 위치를 먼저 검사하고 리소스와 상태를 함께 commit한다. 실패하면 기존 draft와 이름을 유지하며
팝업에 실패 이유를 표시한다. 어느 Object 창을 클릭해도 다음 Update에서 도구 입력 소유권을 유지한다.

모델 상태 11개는 이전 첫 진입 위치 (3.29,8.64,-8.69)에 저장되어 있었다. Preview at Character를 기본으로 켜서
Map 모델 상태의 debug preview만 현재 캐릭터 위치로 옮긴다. 저장 위치와 커튼/룰렛은 변경하지 않는다.
끄면 authored 위치를 사용한다. 실제 생성 수·첫 위치와 anchor 대기·모델/texture/shader 실패 원인을 상태에 표시한다.
스킨 모델에 animation track이 없을 때도 rest combined bone matrix를 초기화한다.

카드/조커/세토/갈고리/빙고폭탄의 raw vertex만으로 modelPreScale=1이라고 판단한 이전 기록을 교정했다.
실제 CMesh skin 행렬과 idle clip 시작/중간/끝 15 sample에는 skeleton scale100이 남아 있었으므로 5종의
modelPreScale을 .01로 수정했다. 사용자 state key와 scale 배수는 보존했다. 적용 후 bounds는 유한하며
카드 2.825×.047×1.836m, 세토 1.332×2.462×1.548m, 갈고리 .805×9.394×.226m,
폭탄 1.338×1.635×1.338m다. 이 수치는 렌더 전 변환 진단이며 화면 품질 PASS가 아니다.

Player Follow Camera는 Position/Pitch/Yaw/Roll/Focus/FOV/Response 편집 시 현재 Level setter로 즉시 반영한다.
Reload/Reset도 즉시 preview하며 Save만 영구 저장한다. 다른 맵 draft나 실행 중인 camera sequence에는 적용하지 않는다.

| 실행한 검증 | 결과 |
|---|---|
| 최종 Debug Product | PASS, out/BuildPipeline/runs/20260907T030125810Z-debug-product.json; Engine→Shared→Server→Client 및 정상 배포 |
| WorldSequence publisher 계약 | 26/26 PASS; WORLD/PLAYER resource anchor와 잘못된 종류·alias 검사 포함 |
| Map Publish | v3/revision149, 3231 placements/7 files, source/runtime semantic equality |
| skin 수치 진단 | 5종×3 clip 시각의 finite bounds; world-object-skin-bounds-review.json |
| 사용자 저장본 보존 | 작업 시작 snapshot 대비 WorldSequence는 revision과 5개 modelPreScale만 변경; final-data-preservation.json |
| 구조·공백 | 변경 JSON 9개 parse와 git diff --check PASS; 변경 XML 없음 |
| 입력·저장·재생 화면 | 실행하지 않음. 독립 패널 Save/Reload와 실제 GPU 화면은 사용자 확인 대기 |

이번 검증 로그는 out/ObjectLightWorkspace/에 있다. 기존 C4819/C4828/LNK4099 경고는 남고 빌드 오류는 없다.
Client/UI는 실행·조작·캡처하지 않았다. 사용자가 종료했다고 알려준 EXE는 다시 띄우지 않았다.
Server + Client profile을 Ctrl+F5로 시작한 뒤 Lobby → KoukuSaydon → F1 Tools → World Object Tool →
Map 리소스/상태 선택 → Object Sequencer Play로 확인한다. 창 배치가 필요하면 Windows → Reset layout을 사용한다.
기존 사용자 미커밋 파일을 보존했으며 자동 stage/commit/push는 하지 않았다.

## G07. 원본 Animation Resources와 선택한 Object의 Append

왼쪽 Object Resources 상단은 저장한 Object/모션 패턴 트리이고, Create Object에서 이름과 Map/Character
anchor로 새 항목과 빈 기본 패턴을 만든다. 같은 창 아래 Physical Resources에서 WModel 후보를 고르면
Animation Resources가 기존 WModel decoder로 실제 clip 이름과 길이를 읽는다. 선택 후 Append Animation은
선택 Object의 모델·animated 속성과 정확한 animationTracks 항목을 함께 검증·반영한다. 별도 GPU 로드나
두 번째 모델 catalog/runtime은 없다. static 모델은 Assign Model 후 Transform/Motion으로 저작한다.

Append 후 기존 아래 Object Sequencer에서 재생하고 오른쪽 Object Detail에서 튜닝한 뒤 Save한다.
추가 모션은 왼쪽 Additional motion patterns에서 만들 수 있다. 새 미편집 패턴의 첫 clip은 native 길이를
사용하고 기존 저작 key 시각은 바꾸지 않으며 필요한 끝 구간만 마지막 pose로 연장한다. 다른 저장 패턴의
clip이 새 모델에 없다면 변경 전체를 취소한다. Reload는 미저장 모델 후보도 초기화하고 placeholder clip
추가는 제거했다. 리소스 이름 검색은 전체 하위 상태를, 상태 이름/ID 검색은 일치한 하위 상태만 표시한다.

물리 모델 10종에서 원본 clip 110개를 확인했다(카드14/조커14/세토68/갈고리9/폭탄5).
이 110개는 Animation 재료 목록이고 자동 저장 패턴은 아니다. 기존 idle 5개와 요청 카드 4개,
세토 3개·갈고리 4개·폭탄 2개의 대표 모션, 총 18개 native 패턴을 미리 연결했다.
빈 카드의 card_hop/card_flip ID는 보존하고 원본 body clip으로 교체했으며 합성 Transform은 제거했다.
조커는 요청한 별도 두 이름으로 저장했다. 공·칼날·빙고의 Motion 및 커튼/룰렛의 배치 key는 유지했다.
세토 보행은 제자리 모션이고 실제 이동 경로는 별도 저작한다. 돌진과 갈고리 전방 이동은 native root가
움직이므로 기본 Velocity를 더하지 않는다. 세부 clip과 물리 경로는 같은 날짜 Resource Ledger를 따른다.

WorldSequence source/runtime은 v3/revision151, 86 templates/90 instances다. 기존 10 resources와 79 instances,
요청 카드 2개를 제외한 기존 template 73개를 보존했다. 새 추출·Resources 파일 변경·Drive 업로드는 없다.

| 실제 확인 | 결과 |
|---|---|
| Map Publish / 기존 WorldSequence 계약 검사 | PASS, 26개 검사; source/runtime semantic equality |
| 물리 clip 연결 | 설치 WModel 110개 확인, 미리 저장한 native clip/binding 일치 |
| 데이터 보존·parse | 변경 JSON 5개 parse; final-json-preservation.json; 변경 XML 없음 |
| 첫 통합 Debug Product | PASS, 20260907T051604640Z-debug-product.json, Engine→Shared→Server→Client 배포 |
| 최종 Client 증분 컴파일·링크·배포 | PASS, client-final-incremental.log; Diffuse 선택 뒤 첫 clip 수명 보완까지 반영 |
| UI 입력·저장·실제 화면 | 사용자 확인 대기; Client/UI 실행·조작·캡처 없음 |

로그와 수치 근거는 out/WorldObjectMotionAudit에 있다. 새 화면은 Server + Client profile을 Ctrl+F5로 시작한 뒤
Lobby → KoukuSaydon → F1 Tools → World Object Tool에서 확인한다. 실제 입력·Save/Reload 버튼 왕복과
모션 외형은 자동 검증 성공으로 대신 판정하지 않는다.
