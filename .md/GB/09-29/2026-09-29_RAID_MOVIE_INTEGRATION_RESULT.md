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
