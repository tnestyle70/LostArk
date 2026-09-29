# 발탄·쿠크 패턴과 Movie 수정 통합 결과

## G00. 진행 상태와 사용자 정정

기능 통합, 공식 게시와 Debug/Release 최종 제품 빌드를 완료했다. 실제 Server 계약의
통과 범위와 광역 검사 미통과는 G07에 분리한다. PR #483으로 전달한다.
최신 사용자 요청의 높이·빨간 장판·중앙 착지 대상은 피자가 아니라
추적 도끼다. 피자에 준비했던 이번 변경은 철회하고 기존 저장 상태를 보존한다.
3회 구르기 후 돌진은 정상 source cue/lifetime을 유지하며 벽 충돌 GROGGY에서 종료한다.

## G01. 창술사와 기존 Movie 저장

사용자 반영 승인 뒤 최신 ClassSelection.cinematics에서 Intro/Loop의 face 트랙 두 곳만
materialName/family/parameters를 정상 donor에 연결했다. 실제 Product parser와
WorldSequencePlayer/CMaterial을 사용하는 창 없는 WARP 검사 27개가 통과했다.
원래 오류와 이름만 수정했을 때의 program mismatch를 재현하고 최종 교정을 확인했다.
Save writer lock·hash 재확인·백업·ReplaceFileW를 사용했고 설치 후 SHA는
`865b6154fb550852414efc3008ecbd8107a5bcb6983241d599cee579711b7258`이다.
워로드25/도화가12/차원술사13개의 Movie 제외 목록과 모든 다른 필드는 보존했다.
상세 근거는 09-27 WORLD_MOVIE_HAIR_GUARDIAN_EYES_IMPLEMENTATION_RESULT G11이다.

## G02. 리소스와 기존 Effect 변경 검증

설치된 창술사 Appearance WModel4개는 GBResources와 이미 SHA가 같았다. 실제 WModel
material section에서 texture 참조를 읽어 donor 모델까지 26개 파일을 대조했고, 전달 폴더에
없던 기존 donor/texture22개(27,109,926 bytes)를 같은 상대 경로에 추가했다. 모델의 geometry나
원본 texture 내용은 바꾸지 않았다. Movie가 참조하도록 변경된 hdr07_1.tga와
brdf_beckmann_spec.tga도 전달본과 SHA 일치를 확인했다.
증거는 `out/RaidMovieIntegration20260929/lance-resource-delivery.json`에 있다.

현재 변경된 Effect8개에 공식 validator의 material color space, native particle,
module override, attachment orientation과 resource closure 검사를 실행했다.
참조244개/27,128,624 bytes를 읽어 PASS했다. 전체 Effect repository validator를
통과한 것으로 기록하지 않는다. 기존 다른 문서의 v15 carrier 문제는 기존 Mario RESULT에
기록돼 있으며 이번 변경 문서는 모두 v13이다.

## G03. 네트워크 검증

마리오 목표 색과 matching-ball 진행도 전달이 포함된 protocol121의
NetworkProtocolHarness를 Debug/Release 각각 증분 Build하고 실행했다. 두 구성 모두
failures0이다. 로그는 `out/RaidMovieIntegration20260929/protocol-*-build.log`와
`protocol-*-run.log`다. 실제 LAN과 Client 화면을 실행한 검사가 아니다.

## G04. 실제 반영한 전투 동작

발탄 HIGH_JUMP는 LEAP_TO_ANCHOR, apex 30m와 아레나 중앙
`[156.03, 22.99751, -122.06]`을 사용한다. 빨간 장판은 같은 landing snapshot을 소비한다.
SEQUENCE_FOUR는 target/aim 정책과 패턴 선택 전 nearest-target 회전을 함께 차단한다.
STRUGGLING STEP04는 진입 방향을 유지하며 0/180/270/90도 공격 네 번을 실행한다.
3시·9시 지형 파괴와 발악의 세 rock event는 반경을 6.3639610307에서
5.8639610307m로 줄였다. 돌 geometry 크기는 유지한다.

DASH_CHARGE의 WALL_CONTACT만 recovery/GROGGY로 분기하며, 정상 TIMEOUT은 종료한다.
Client는 같은 sequence의 charge→recovery에서 해당 action의 pending/active Effect와
preserved tail을 함께 종료한다. source cue 318.3~1606.7ms와 Effect lifetime 10초는 유지한다.
TRIPLE_COUNTER FAIL_3 및 STAGGER_SLOT FINAL_ATTACK은 기존 encounter wipe 경로로
인간 플레이어 전원을 처치하고 Guide는 제외한다. 유령 전환 중 사망 연출은 ending을
재생하지 않으며 primary Valtan의 gameplayPhase3 실제 DEAD만 최종 ending을 요청한다.
피날레의 주기 삼각 돌진은 portal interval 0으로 껐고 5초마다 random auxiliary ghost는
유지한다. 버러지들 source subtitle 두 곳은 UPPER로 올렸다.

쿠크 DANCE는 플레이어 HUD를 표시하고 보스 HUD만 숨긴다. giant Saydon의 combat camera1은
플레이어를 표시하고 기존 Server 승인 picking/movement 경로를 유지한다. 2관문 160~0줄의
일곱 일반 반복 모두 P106→P105→P85, 즉 저글링→나팔→슈퍼바주카가 되었다. Mario 비행 공의
접촉 피해는 최대 HP 비율 대신 1320 고정값을 사용하며 기존 contact 중복 방지와 CC는 유지한다.
다른 세션의 Mario 목표 색·matching-ball 진행도 protocol121을 함께 보존했다.

## G05. 공식 게시와 저장본 보존

Kouku projector publish(revision2469, pattern114, stage577, bundle9), Valtan PublishV2,
Composition SyncValtanShadow/Publish, Valtan Area WorldSequences publish 및
GameplayBalance Publish를 모두 완료했다. Gameplay source revision은
`60832e1bc10f87c301b9f19f73da444d0b258e9ddcd5309542a195997f84142a`이며 생성된
bootstrap은 109347 rows, 32,254,967 bytes다. publisher의 clip-template parity,
Valtan hit alignment와 player hit coverage95/95도 통과했다.

사용자 Retail 수치, ClassSelection, Kouku HUD 저장본은 게시 직전 별도 baseline과 대조해
요청한 변경 외 값을 보존했다. Mario FXAA 등 rendering option은 변경하지 않았다.
변경 JSON27개 parse 및 `git diff --check`가 통과했다. 공식 게시 로그와 보존 근거는
`out/RaidMovieIntegration20260929/`에 있다. 파일 게시가 실행 중 Server나 도구의 메모리
draft까지 갱신했다는 의미는 아니다.

## G06. 최종 Product 빌드와 교차 검토

`Invoke-BuildAndRegression.ps1 -Configuration Debug/Release -Profile Product`를 순서대로
실행해 Engine/Shared/Server/Client 컴파일·링크·runtime 배포가 두 구성 모두 PASS했다.
누락/무효 runtime input은 각각 0개다. 기존 C4819/C4828 및 DirectXTK PDB 경고는
남지만 컴파일·링크 오류는 없다. 빌드 영수증은 다음 두 파일이다.

- `out/BuildPipeline/runs/20260929T015328182Z-debug-product.json`
- `out/BuildPipeline/runs/20260929T015730011Z-release-product.json`

최종 fixture 교정 뒤 Server만 Debug/Release 최소 Build를 추가해 두 구성 모두 성공했다.
로그는 `out/RaidMovieIntegration20260929/server-final-debug-build.log`와
`server-final-release-build.log`다. 제품 구현과 게시 데이터는 최초 전체 빌드 이후 같다.

발탄 generation Python13개, 창술사 실제 material 경로27개, 발탄 Client native34개,
실제 reader admission6개/거부·rollback104개와 쿠크 정책106개 집중 검사를 통과했다.
각 기능의 상세 검증은 연결된 기능별 RESULT와 out 로그에서 구분한다.

검토한 구현 커밋은 `fc2f573f320add3968501406e67ac592cc058608`이다.
다른 담당자가 발탄, 쿠크/protocol121,
Movie 편집/Release 준비를 교차 검토하여 추가 P1/P2를 발견하지 못했다.
main `2ca7dff8a` 병합 후 `819d0540f3f9296a5d3f2a929acb3be662fa8a51`의 제품 파일은
검토 커밋과 동일하다. 읽기 검토는 실제 GPU 화면이나 다인 플레이의 검증을 대신하지 않는다.

## G07. 실제 Server 계약과 기존 광역 검사 경계

Release의 Kouku object-overlap 및 raid 계약은 각각 failures0이다. 최대 HP13200/26400에서
Mario 비행 공 피해1320을 유지하는 검사를 포함한다. 발탄 전체 검사에서도 no-wall TIMEOUT와
같은 tick WALL 우선, 독립/발악 4방향 yaw 고정, 셋째 counter/무력화 실패의 true wipe,
30m HIGH_JUMP와 중앙 landing, 0.5m 안쪽 rock volley, phase3 random ghost와 terminal clear의
실제 소비 경로를 확인했다. 정적 값 비교만으로 위 실행 결과를 대신하지 않았다.

처음 Release lifecycle의 10개 실패는 기존 fixture가 벽 충돌 없는 TIMEOUT에도 GROGGY를
기대하고, 연속 4인 성공 시나리오를 invulnerability로 무력화 실패 전멸까지 넘기던 전제에서
발생했다. 실제 WALL_CONTACT와 Apply_PlayerHit의 health-threshold/counter 성공 입력으로
교정했다. 생존·순서·snapshot·실제 최종 스킬 처치·MVP 검사는 유지하고 성공 입력5회/2회까지
추가했다. 제품 코드·Data는 이 재검증 중 변경하지 않았다.

GenerationRetention의 오래된 정밀 기대값 세 곳도 현재 source에 맞췄다. FAIL_3는1704ms,
FOUR_SLASH는 slash1667/2221/3008와 spin600/1640/3000ms다. STRUGGLING의 STEP04는 고정
4방향 cone이고 rock volley는 STEP08에 있다. stable hit ID·shape·각도·생성 반경을 함께
검사하며 단순 count 완화로 넘기지 않았다.

named-reactive 검사에 남았던 저작 gauge30/100은 Retail 배율400 적용 후의 게시값
12000/40000으로 맞췄다. publisher가 생성한 TIMEOUT 분기 검사는 유지한다. 최종 소스와
byte-identical한 TU를 실제 Release product objects에 연결한 기존 GenerationRetention
집중 실행은14 PASS/0 FAILURE, exit0이다. `retention-corrected-focused.log`에 기록했다.

Debug lifecycle/Next 묶음은 교정 후에도 HIGH_JUMP 이동 관찰 조건 한 곳에서 failures1을
기록했다. LandingEvidence의 snapshot/airborne/land/recovery/coalescing은 모두1이고
옛 boss target-cache를 사용한 moved만0이었다. 중앙 착지에 맞춰 실제 player 시작 대비
이동 및 anchor 거리로 검사하고, 중앙 좌표와 apex30 조건을 추가했다. 기존 snapshot,
sequence/revision/start tick, lethal landing과3200/400ms 종료 검사는 그대로 유지했다.
수정 후 실제 Debug `--valtan-presentation-contract-test`는55 PASS/0 FAILURE, exit0이다.
로그는 `server-debug-presentation-final.log`다. 전체 Debug lifecycle/Next 묶음을 다시 실행한
것으로 설명하지 않는다. 같은 CLI의 Release는 `_DEBUG` 제외로 assertion을 실행하지 않으므로
출력된 failures0을 별도의 기능 검증으로 계산하지 않는다.

교정 전 광역 Release `--contract-test`는 failures86, 마지막 완료 실행은 failures82였다.
서로 실행에서 결과가 달라진 일부 collider assertion도 있어 숫자 차이 전체를 fixture
교정의 효과로 설명하지 않는다. 이 결과를 전체 PASS로 기록하지
않는다. 요청 밖 오래된 기대값을 모두 교정하는 별도 작업으로 확장하지 않았다. 다음 근거는
시작 SHA `d6a9cc224`의 코드/정본과 현재 값을 비교한 것이며, 과거 SHA를 별도로 빌드·실행하여
동일한 실패 수를 재현했다는 뜻은 아니다.

| 기존 광역 fixture | d6와 현재에서 확인한 실제 계약 | 이번 변경과의 관계 |
|---|---|---|
| Bind5000ms | source stage/event는4107ms | 이번 변경 밖 |
| armor opening159줄 자동 시작 | AUDITION_ONLY, trigger0 | 기존 자동 시작 기대값 불일치 |
| Dash 다른 벽98개 격리 | collision box96개에서 선택벽 제외95개 | 격리 준비가 생략되며 CHARGE 첫 tick 검사라 TIMEOUT 수정과 무관 |
| SkyAxe ownerY0와 생성Y 동일 | FIXED_AREA는 실제 floor Y를 사용 | 직접 ENTER 소비자는 이번 serverMotion kind/apex를 읽지 않음 |
| protocol101, stand-up3000ms | d6 protocol120, 현재121; stand-up30000ms | 오래된 protocol/timing 기대값 |
| Warlord BA2 repeat3, Lance advance470ms | repeat1, advance547ms | 양쪽 정본 동일 |
| skill34120 collider3/BOX | health collider4/FAN | 양쪽 정본 동일 |
| parry/orb gauge30/100 | Retail400을 적용한 게시값12000/40000 | 최종 집중 fixture에서 교정·실행14개 통과 |

SKILL 계열 게시914행은 시작 커밋과 동일하다. AP23000→230000은 함께 반영하도록 승인한
실제 수치 변경이며 고정 HP/피해량 fixture에 영향을 줄 수 있다. 반면 rate만 합산하거나
AP1000을 전제한 기대값은 시작 커밋의 coefficient/addend/AP23000 소비와도 맞지 않았다.
독립 collider·health-bar fixture는 navigation=nullptr이므로 이번 이동 후 navigation 재검사
수정 경로에 들어가지 않는다. Bern·wall-climb의 관련 source/reader/navigation/게시 world는
시작 커밋과 같지만 그 실패군의 전체 실행 원인까지 확정하지 않았다. 따라서 광역 실패 전부가
기존 문제라거나 모든 제품 동작이 검증됐다고 주장하지 않는다.

관련 로그는 `out/RaidMovieIntegration20260929/server-*.log`다. 오래된 Debug 광역 실행은
fixture 교정과 중복을 피하기 위해 이 세션이 시작한 정확한 PID만 확인해 중단했으며 PASS로
계산하지 않는다. Debug는 교정된 Valtan lifecycle에 집중하고 Release는 전체 계약을 실행한다.

## G08. PR과 남은 확인 경계

PR은 https://github.com/tnestyle70/LostArk/pull/483 이다. 최종 merge 상태와 merge commit은
이 PR의 GitHub 기록을 정본으로 삼는다. 세 개의 기존 untracked backup/retired 파일은
PR에서 제외하고 원래 바이트를 보존했다. main 동기화 전 safety stash도 유지한다.
Client/UI는 자율 실행하지 않았으며 화면·음향·실제 다인 플레이는 사용자 확인 경계다.

## G09. Release 4인 검증 전 추가 재감사

이 절부터는 G08의 기존 통합 이후, HEAD3a55be17c에서 시작한 추가 요청 결과다.
현재 변경을 기존 PR483에 포함된 것으로 설명하지 않는다. 사용자가 확대·저장한 발탄
장판 및 AIRBORNE7984ms, 작업 중 저장한 KoukuHudModes의 머리 위 표시 offset과
기존 backup/retired 파일을 보존했다. Mario1~4의 FXAA와 다른 rendering option은
변경하지 않았으며 Resources 추가도 없다.

HIGH_JUMP의 빨간 원은 원본 지름17.5m와 occurrence XZ1.4로 지름24.5m다.
LAND 기본 shape와 hit.valtan.high-jump.final-landing contact를 반경12.25m로 맞췄다.
30m 상승, 아레나 중앙 landing/장판, 피해·접촉 시점은 기존 계약을 유지했다.
사자후는 이미 STEP10+900ms에 CIRCLE100m 판정이 있고 네 돌의 cover와 이후 폭발도
Server에서 소비하므로 중복 collider를 추가하지 않았다. 상세는 같은 날짜
VALTAN_HEALTH_ROTATION_AND_ANCHOR_RESULT G22와 struggling-audit.md에 있다.

돌진 aura는 CHARGE의 실제 stage 시간에서 시작 offset을 뺀 시간만큼 재생한다.
벽 충돌 GROGGY, 정상 종료, 패턴 교체, 죽음 모두 승인된 action 종료에서 해당 aura의
pending/active/tail을 정리한다. 원본 particle lifetime10초를 줄이지 않았다.
고정4방향의 target/aim 및 사전 자동회전 제외, 유령 phase의 실제 사망에만 ending,
피날레 삼각 반복 interval0과 random ghost5000ms, 버러지 자막 UPPER를 재확인했다.
쿠크 Dance의 player HUD 표시/보스 HP 숨김과 giant camera1의 player 표시 및
Server 승인 picking 이동도 기존 실제 소비 경로와 일치했다.

## G10. 마리오 참가자 커튼과 비행 공

네 Client가 동일한 P33을 계속 실행하면서 마리오에 들어간 local player의 커튼만
숨길 수 있다. 기존 suppression은 Product reader에 없는 MARIO_PHASE2_PLAYERS
LogicOccurrences를 검색하므로 제품 실행에서 조건이 성립하지 않았다. projector가
해당 logic을 가진 패턴의 정확한 boss.kouku.curtain_1 occurrence에만
suppressLocalMario=true를 투영하고, 실제 reader가 타입과 leaf를 검증하여 읽는다.
Sample은 local iMarioStage1~4와 이 값을 사용해 기존 handle을 정리한다. 바깥 세 명,
P33의 나머지 동작, 같은 leaf를 쓰는 P6 Dance 및 Preview는 유지한다. 최종 게시본에서
이 metadata를 가진 occurrence는 한 개다.

비행 공은 기존 Server가 피해/중복 ledger만 기록하고 Client의 동일 시계 재생을
종료하지 않던 누락을 수정했다. 실제 접촉이 승인되면 기존 reliable world sequence
STOP을 해당 마리오 참가자에게 전달한다. stable instance, 정확한 birth start tick과
contact tick으로 Client가 그 공만 종료하고 기존 effect.kouku.mario.flyingball.hit를
접촉 위치에서 한 번 재생한다. 피해1320과 기존 CC를 유지하며 protocol은 바꾸지 않았다.
NOT_ADMITTED는 소비하지 않고 유효한 흡수 접촉은 소비한다.

최초 stage snapshot보다 먼저 도착한 접촉도 처리한다. 자연 수명 종료 뒤 늦게 도착한
STOP은 피격 시점과 Effect sample age를 검증해 아직 유효한 Effect를 재생하되 다음
세대 공은 지우지 않는다. 중복·과거 신호는 birth별 소비값으로 차단하며 아직 snapshot
시간에 도달하지 않은 신호는 보류한다. 피해 history로 별도 Effect를 만들던 경로는
제거하고 typed 피해 기반 사운드는 유지했다.

| 구간 | 생성기 수 | 생성기별 기존 간격 | 변경 간격 |
|---|---:|---:|---:|
| Mario2 | 3 | 4000ms | 8000ms |
| Mario3 | 3 | 4000ms | 8000ms |
| Mario4 | 1 | 4000ms | 8000ms |

Server와 Client가 같은 Shared 상수·marker seed·birth 함수를 소비한다. 각 생성기의
phase offset이 있으므로 여러 생성기에서 보이는 전체 공 간격이 항상8초라는 뜻은 아니다.
이동속도3m/s, 반경0.36m와 상·하 높이 정책은 유지한다. 공격해서 깨는 색깔 목표 공은
이번 접촉 소멸 대상이 아니다.

## G11. 추가 요청의 게시·Release 검증과 Debug 보류

Valtan PublishV2, Kouku projector publish, Composition Validate/Publish와
GameplayBalance Publish를 완료했다. Kouku source revision2469의114패턴577단계,
9 bundle을 게시했고 projector check도 통과했다. Gameplay bootstrap은109347행,
32,254,969 bytes이며 SHA256은
`0ccd94c4e8b9f0d4ce3cfd52148d0226f516a6b81d6585135ecfdd04f59ee052`다.
Valtan generation은`5ba6db61b2c2d86d9311396954e86ca985cc14837eae35a1412dab9216d12ad6`이며
174개 artifact의 hash/bytes와 정본에서 재투영한9개 Product가 최종 게시 후에도 일치한다.
Data·Client/Bin/DataFiles·Server/Bin/DataFiles·Resources 경로는 Debug/Release가
공유하며 구성별 인접 사본이나 현재 process/user/machine data-root override는 없다.
실행 중 Client/Server의 메모리나 시작 시 환경을 조사한 결과는 아니다.

최종 소스 기준 Release Engine→Shared→Server→Client를 순차 Build하여 컴파일·링크와
runtime 배포를 완료했다. 로그는 out/RaidPatternReview20260929/final-release-*.log다.
일반 runner는 실행 중 Debug도 모든 configuration의 점유로 보는 preflight에서 막혀
같은 MSBuild 도구·x64 Release 설정·제품 순서로 직접 Build했다. runner의 차단 코드는
변경하지 않았다. 기존 C4828/DirectXTK PDB 경고가 있지만 빌드 오류는 없다.
최종 Client 링크는2026-09-29 18:54 KST에 완료됐다. 별도의 -SkipBuild Product runtime
검사는 PASS이며 out/BuildPipeline/runs/20260929T095220630Z-release-product.json에 있다.
이 영수증은 runtime 배치 검사이며 컴파일 증거는 위 final-release 로그다.

실행한 기능 검증은 다음과 같다.

- 최종 Release Server의 --kouku-object-overlap-contract-test failures0. 일곱 생성기의
  두 세대에 대해1320 피해와 실제 reliable FIFO STOP frame14개, 다른 참가자에게 신호가
  전달되지 않음을 검증한다.
- 실제 Client 공 소비 메서드를 사용하는 native20개 통과. 지연·중복·세대 교체·stage
  진입·사망 정리와 Effect 유효 시간을 포함한다. out/MarioBallContact20260929에 있다.
- 커튼 projector 집중3개 및 실제 DataJson/reader/suppression native496개 통과.
  handle sink는 mock이며 GPU·UI 검사가 아니다. 기존 world_companion fixture의
  StopIteration은 원래 HEAD projector에서도 재현되어 전체 projector suite PASS로
  기록하지 않았다. out/KoukuPreFourClientReview20260929/curtain/validation.json에 있다.
- 발탄 stage contacts6개, finale interval5개, stage aim3개, presentation generation13개,
  EffectV2 validator와 Client focused41개 및 reader rejection/rollback104개 통과.
  게시 후 Release lifecycle failures0이며 추가 공 변경 전의 Debug presentation도
  failures0이다. Release presentation CLI는 assertion 제외라 별도 검증에 세지 않았다.
- 최종 변경 JSON10개 parse, 커튼 flag 단일 occurrence와 git diff --check 통과.
  최종 Release binary와 bootstrap hash는 final-review-receipt.json에 기록했다.

사용자는 Debug에서 계속 검토 중이므로 Release만 먼저 검증하도록 명시했다.
Debug Client41820/Server52740을 종료하지 않았고 최종 Debug EXE/DLL 교체는 보류했다.
Debug Server의 ClCompile만 통과한 상태이며 새 공·커튼 코드가 현재 실행 중인 Debug에
반영됐다고 설명하지 않는다. 파일 publish도 실행 중 Server 활성화나 도구 Reload를
뜻하지 않는다. 현재 Release 실행 파일은 모든 추가 소스를 포함하며 실제 사용 시
새 Server 실행 파일도 함께 사용해야 한다. Client/UI 실행, 실제4클라 화면·음향·입력
검증은 수행하지 않았다. G07의 과거 광역 실패를 이 집중 검사로 해소한 것으로 주장하지 않는다.

이후 추가 요청한 갑옷 파괴 표현·배틀 아이템4종·F1 지급과 최종 Release 재빌드 결과는
같은 날짜 VALTAN_ARMOR_AND_BATTLE_ITEM_EFFECTS_RESULT에 기록한다. 기존 Debug 유지 조건은
계속 적용하며 새 protocol122와 Items schema6는 같은 새 Client/Server 조합으로 확인한다.


## G12. Movie·배틀 아이템 통합 Debug/Release 최종 검토

사용자가 모든 변경의 반영과 두 구성 빌드·게시·Release4인 검토를 요청한 뒤 실제 제품
프로세스가 종료된 것을 확인했다. 기존 미커밋 변경, Camera 저장값, rendering 옵션, backup과
retired 파일을 보존했다. 이번 반영은 현재 checkout 전체를 빌드한 것이며 별도 PR/merge를
수행한 것으로 기록하지 않는다.

### 반영과 게시

창술사 지면 grass300개/24material/RNM48DDS를 기존Map 경로에 설치하고 SL03 Area를
809placements/3files로 게시했다. 누락된 Guardian Intro sound2박스를 복구했고 다섯 클래스의
원본 float/bus 기반 음질 교정 WAV8개를 설치했다. WorldSequences 최종source/runtime SHA는
6b8cac11b11face85bb07ab8f4b2876f7b9d761c965317f6846f0c51bb689bf4다. 두 맵의 최종Check도 PASS다.
사운드 원인·DSP 근사 경계와 카메라 기능은09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT G14~G16,
원본 지면/가디언 추출은09-25 FOUR_CLASS_SELECTION_MOVIES_IMPLEMENTATION_RESULT G13~G14를 따른다.

Movie sound에는 source-in 앞부분 trim과 실제FMOD cursor의 drift 교정을 연결했다. 카메라의
Use free cam pos는 선택 key의 Eye/LookAt/Up만 바꾸며 기존 보간/시간/FOV와 수동Delete를 유지한다.
World 및 Camera의 기존 저작 키를 자동으로 줄이거나2.7초 보정값을 임의 저장하지 않았다.

Composition Publish와 GameplayBalance Validate/Publish를 완료했다. Item CheckPublished는
베른 물약 상점의4종 각각161SILVER를 포함한 현재 정본과 unchanged로 일치한다. 회수의HP피해0,
ceil(최대무력화/3), 성부/시정, 인벤토리·HUD입력은 기존 배틀아이템 구현과 함께 빌드했다.
Game bootstrap은109347행/32254969bytes다. publisher 로그는 out/MovieRaidFinal20260929에 있다.

### 두 구성의 실제 빌드

정상 Invoke-BuildAndRegression -Profile Product를 Debug/Release 각각 실행해 Engine→Shared→
Server→Client compile/link와 EngineSDK·DLL·CSO 배포까지 완료했다. SkipBuild가 아니다.

- Debug: out/BuildPipeline/runs/20260929T115048190Z-debug-product.json,20:50:48 KST.
- Release: out/BuildPipeline/runs/20260929T115318016Z-release-product.json,20:53:18 KST.

두 영수증은 PASS이며 runtime 존재·Navigation 참조·Item/Valtan reward catalog 일치 검사도
통과했다. C4819/C4828·X4000·DirectXTK PDB LNK4099 등 기존 경고는 남아 있다. 경고0 빌드라는
뜻은 아니다. Client/Server EXE와 Engine DLL6개 및 새5312CSO, 게시물의 최종bytes/SHA는 final-verification.json에
기록했다. 제품프로세스를 자동 실행하거나 사용자프로세스를 종료하지 않았다.

### 실제 실행한 기능·네트워크 검증

| 검사 | 결과 | 확인한 경계 |
|---|---|---|
| Release Valtan lifecycle |92 PASS/0fail|4인 연속20492tick,7개체력기믹,유령사망,클리어/MVP/보상,4session snapshot일치|
| Release Kouku raid |1747 PASS/0fail|1~4참가자 관문승인,READY,실제flow,빙고prefix/tail·ending/clear계약|
| Debug/Release Battle Items |각38 PASS/0fail|승인·소비·중복·쿨다운·양보스회수3회·HP불변·성부/시정|
| Release NetworkProtocol |1387 PASS/0fail|protocol122·각packet의왕복/잘못된입력보존|
| Release Party4 live |PASS|실제Server TCP4연결,Bern→Valtan,정확human명단/파티장/명령유지|
| Product Movie sound fixture |56 PASS|실제Engine/FMOD채널,source-in/drift/loop/복구·float재생|
| Camera CPU fixture |41 PASS|실제sampler,포즈교체/거부,시간·FOV·보간·cut보존/키삭제|

Protocol 검사의 최초11FAIL은 현재122 대신121을 요구하는 고정값이었다. packet번호/shape검사는
유지하고 현재version 기대값을 교정해 재실행했다. Party4의 최초timeout은 GUIDE_AI를 다섯째
사람으로 세던 검사 문제다. human목록을 별도로 비교하고 private-world누출검사는 전체players를
계속 검사하도록 교정한 뒤 실제 live scenario를 통과했다. 제품의인원수·guide동작은 바꾸지 않았다.

### 광역 실패와 최종 판정 한계

Release --contract-test의 현재 실행은1436PASS/84FAIL이다. 이 결과를 위 집중검사로 전체PASS로
바꾸어 기록하지 않는다. 두 독립 감사를 합친 정적 분류는 fixture불일치확정56/부분원인6/미확정22다.
수정하지 않은 fixture의 재실행 성공을 주장하지 않는다. 예를 들어Bind4107ms와피격가능상태,
115줄전환,7984msAIRBORNE,삭제된rotation,실제피해보다낮은fixtureHP,현재protocol과 옛기대값이
달랐다. 준비실패·후속연쇄실패를 별도로 구분했다.

Bern 계단은 actualServerNavigation OBJ probe로 양쪽Find_Path,37개경로segment,최종높이와
도착Sample이 PASS다. 가장자리rawSample의 실패는실제메시밖cell중심이며 허공walkable을추가하지
않았다. TrackMove5실패도현v11의9필드에옛fixture6필드를넣어동작검사전에실패했다. 실제게시
181samples/6000ms는확인했다. 전체22미확정항목을이근거만으로정상이라고판정하지 않는다.
감사는 audit/AUDIT.md,VALTAN-AUDIT.md,failures-combined.json,summary-combined.json을 따른다.

Movie58개(복원float8+RNM48+보존Guardian원본2)와기존BattleItems163개,총221개44,194,360bytes를
프로젝트Resources와Desktop/GBResources에서독립hash/byte대조해전부일치했다. 변경JSON34개와
vcxproj/filters XML4개parse도PASS다. source/runtime전용맵포맷은publisher로검증했고최종
git diff --check를통과했다. source/tool/project변경과컴파일·중간산출물은구분했다.

실제Client4개를띄운화면·입력·GPU·음향과사람의연속전투완주는아직미확인이다. 서버자동검사는
그사용자판정을대체하지않는다. 현재소스로확인한범위는두구성빌드/게시와위정량·네트워크계약까지다.
새기능은같은새빌드의Client/Server를함께사용한다. Debug와Release의Data/Resources는공유하지만
실행중도구의메모리draft를자동Reload한것으로설명하지않는다.


## G13. Visual Studio Server 시작 항목 복구 (2026-09-29)

사용자 첨부 화면의 Server(언로드됨)과 시작 프로젝트 목록 누락을 대조했다. Framework.sln의
Server 프로젝트 및 x64 Debug/Release 구성, 실제 Server.vcxproj와 두 구성 Server.exe는 존재한다.
현재 MSBuild의 Debug x64 project 평가도 Server와 올바른 TargetPath를 반환했다. 실행 테스트나
Visual Studio 프로젝트 다시 로드 성공을 의미하지 않는다.

Framework.slnLaunch의 Server + Client 프로필에는 Client만 남아 있었다. 최신 파일을 다시 읽고
원래 Server/Default/Server.vcxproj의 Start 항목을 Client 앞에 복구했다. 나머지 프로필은 보존했고,
현재 Git 정본과 같은 시작 설정이다. 교체 전 바이트 확인과 백업은
out/ServerStartupRecovery20260929/receipt.json에 남겼다. .suo 또는 IDE 상태를 변경하지 않았다.

언로드 동작의 발생 원인을 확정할 최근 Visual Studio ActivityLog는 없었다. LAN sync는 debugger
.user 설정과 방화벽만 다루며 프로젝트 언로드를 수행하지 않는다. 실제 IDE의 Server 다시 로드는
사용자가 Server(언로드됨) 우클릭 → 프로젝트 다시 로드로 수행한다. Movie 반복 정책 변경은
사용자의 긴급 Server 확인 요청으로 보류했으며 아직 제품 동작에 적용하지 않았다.


G13 원인 확정 및 복구: 이후 사용자 오류창은 ServerGameplayContractTests_KoukuSupportSurface.cpp가
ClCompile에 두 번 포함됐다고 명시했다. 실제 XML에는 이 파일과 Navigation.cpp, PlayerSkillFixtures.h의
동일 metadata 등록이 각각 두 번 존재했다. 뒤의 중복 3건만 제거했으며 각 원래 등록과 bigobj,
BattleItems의 추가 등록은 보존했다. Server/Client의 vcxproj 및 filters 4개에서 XML parse와
정규화된 (ItemType, Include) 중복0을 확인했다. 이전의 단순 MSBuild project 평가만으로는
이 Visual Studio 로드 오류를 검출하지 못했다.

복구 후 표준 Debug Product build/deploy는 PASS다. Engine/Shared/Server/Client 모두 성공했으며
out/BuildPipeline/runs/20260929T121156908Z-debug-product.json에 기록했다. 새 Client.exe와 DLL,
shader 배포 및 기본 런타임 존재 검사를 완료했고 이번 작업에서 Data publish는 하지 않았다.
Movie 반복 UI의 미완성 추가는 out/MovieRepeatHold20260929에 후보 patch로 보존한 뒤 정확한
수정 전 SHA로 복귀했다. 기존 자유 카메라 캡처·사운드·배틀 아이템 변경은 그대로다.


Debug Server를 VS와 같은 Server/Default 작업 디렉터리에서 hidden/headless로 실행했다.
격리 포트17778, bind0.0.0.0, 실제 LAN 주소192.168.0.22 TCP 접속 및 자동 종료(exit0)가 PASS다.
준비 시간은16938ms였다. 첫10초 probe는 초기 데이터 로딩 전에 끝나 접속을 못했지만 서버는
그 뒤 Listening에 도달해 정상 종료했다. 충분한 초기화 대기 후 재검사한 최종 접속 성공은
out/ServerStartupRecovery20260929/server-smoke-final.json과 stdout에 기록했다. 실제 게임
Client는 실행하지 않았으며 Ctrl+F5의 IDE 다시 로드·화면 검증은 사용자에게 남아 있다.

## G14. Server 복구 후 Movie 완료 정지·창술사 catalog 수정

사용자가 Server 프로젝트 복구 성공을 확인하고 EXE를 종료한 뒤 후속 수정을 진행했다.
G13에서 보류했던 Repeat Movie UI와 완료 상태를 반영했다. 다섯 클래스는 Intro를 한 번 재생한 뒤
마지막 유효 source frame의 카메라·모델·Effect를 유지하고 해당 Movie 소리를 종료한다.
원래 Loop 데이터는 보존하며 Play All로 처음부터 다시 재생한다. parser/camera CPU1256개와
Product FMOD NOSOUND 종료21개 검사는 통과했다. 실제 Client Tick·GPU·청감 확인과 구분한다.
세부 계약은09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT G17을 따른다.

창술사 SL03 로드 실패는 추가 잔디24개 catalog 행의 uvScale.x=0 때문이었다. 해당 필드만1로
교정한 뒤 Area Validate/Publish/Check(809배치·3파일), 실제 C++ source/runtime161개 catalog 및
재질 admission을 통과했다. source/runtime의 경로 계약을 각각 Load_Source/Load_Area로 확인했다.
추가 Resources221개는 Desktop/GBResources와 SHA/bytes가 모두 일치한다.

최종 표준 Product Debug/Release build/deploy는 모두 PASS(SkipBuild=false)다. 증거는
out/BuildPipeline/runs/20260929T123342593Z-debug-product.json 및
out/BuildPipeline/runs/20260929T123546398Z-release-product.json이다. 최신 저장 SL00 WorldSequences
게시와 SL03 Area 게시도 완료했다. 이는 이전 광역 Server fixture84개 실패를 해결했다는 뜻이
아니며, 실제4Client 화면·연속 전투 완주 상태는 G12의 미확인 경계를 그대로 유지한다.

마지막 SL00 WorldSequences Check도 PASS다. final-integration-check.json에서 다섯 repeat=false,
SL00 source/runtime 및 SL03 교체 SHA, Client/Server 프로젝트 XML4개의 중복0, Resources221개를
다시 확인했다. 기록과 Check 로그는 out/MovieRepeatHold20260929에 있으며 git diff --check도 통과했다.

## G15. 사용자 재확인 후 남은 두 결함의 구분

G14 이후 사용자는 창술사 Play 자체는 되지만 잔디가 없고 첫 연출 뒤 카메라도 계속 이동한다고
확인했다. 새 EXE는 이미 실행되고 있었다. 카메라는 종료 요구를 전체 Intro로 잘못 해석한 문제이며,
실제 Intro에는 뒤쪽 대기 카메라 box도 포함됐다. holdAfterCameraId로 첫 box의 끝을 지정하는
후속 수정과 실제 Update 검증은09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT G18을 따른다.

잔디는 UV admission과 별개로 셰이더의 필수 g_SourceFoliageWindProgram 누락 때문에 material bind가
E_FAIL을 반환해 draw를 제출하지 못했다. 설치 Debug/Release FX에서 같은 실패를 재현했다.
공용 HLSL의 변수 계약을 복구하고 Binary28개 변형도 재컴파일한다. 해당 수정과 CShader의 실제
bind/pass 검사 근거는09-25 FOUR_CLASS_SELECTION_MOVIES_IMPLEMENTATION_RESULT G13 후속을 따른다.
G14의 CPU catalog PASS는 이 렌더링 결함 해결이나 최종 화면 성공을 의미하지 않았다.
