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
