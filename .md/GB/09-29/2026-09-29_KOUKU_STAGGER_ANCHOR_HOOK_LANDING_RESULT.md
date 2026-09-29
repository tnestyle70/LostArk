# 쿠크 무력화 머리 추종과 갈고리 하차·회피 경계 수정 결과

## G00. 확인한 원인과 반영 범위

2026-09-29 `codex/kouku-release-sequence-ready`에서 사용자의 첨부 화면을 열람했다.
무력화 바는 화면 고정 rect를 사용했고, 작은 HP는 실제 presentation 머리를 투영했다.
갈고리 이동 경로가 틀렸다는 초기 추정을 사용자가 정정했다. 하차 순간 Space 사용은
추정 입력이며 이번에 실제 Client 입력을 재현하거나 화면을 캡처하지 않았다.

현재 코드에서 확정한 결함은 skill root motion의 collision 이후 최종 지면 검사 누락이다.
실제 collision body의 접선 이동이 사전 navigation clamp 바깥으로 나오는 CPU 사례를
재현했다. 갈고리의 정상 source 종단 외 해제도 현재 공중 XYZ를 그대로 풀 수 있었다.

## G01. 무력화 게이지

MainApp, CombatHUDViewModel과 WorldHealthBarView의 기존 경로를 연결했다. 보스 snapshot의
정확한 entity ID로 현재 actor를 찾고, 작은 HP와 같은 Try_GetHeadAnchor 및 화면 투영을
최종 카메라 갱신 뒤 매 프레임 사용한다. 보스·카메라 이동/순간이동/대상 교체를 즉시 반영하고
actor 부재·화면 밖·projection 실패·기믹 종료는 게이지만 숨긴다. 시작 위치 delta 방식은
사용하지 않는다. 기존 frame/fill, 서버 게이지 값, 너비·두께는 그대로 소비한다.

F1 Stagger head X / Y는 optional mechanicHeadOffsetX/Y를 사용하며 기본0이다. 기존
mechanicOffsetX/Y는 화면 고정 보정값이라 새 기준으로 재해석하지 않고 JSON에 보존한다.
Save는 기존 최신 필드 병합·CAS·backup·원자 교체를 사용한다. 사용자의 실제 HUD JSON은
이 작업에서 저장·교체하지 않았다. public 계약은 TEAM_GAMEPLAY_INTERFACE_HANDBOOK에 반영했다.

## G02. 갈고리 하차와 회피

PlayerSkillSystem은 collision이 바꾼 최종 이동 segment를 다시 Clamp_StepToWalkable로
검증한다. 잘린 결과는 commit하지 않고 직전 좌표를 유지하며 스킬 시계는 계속 진행한다.
정상 Space 거리·cooldown·공격 수치와 navigation 없는 기존 실행은 유지한다.
충돌 후 지면 Y가 달라지는 보정은 별도 수직 sweep과 최종 player volume 검사를 통과해야 한다.
최종 양끝이 비어 있어도 사이의 collider를 관통하는 보정은 거절한다. 같은 높이에서 이미
겹친 body를 조금씩 벗어나는 기존 collision 동작은 유지한다.

GameRoom의 공통 Release_PlayerAttachment는 GATE3의 살아 있는 비마리오 WORLD_HOOK_TIP만
가까운 실제 walkable floor·동일 grid·높이/수평 거리1.8m·충돌을 확인한 뒤 입력을 푼다.
근처 하차점이 없으면 기존 관문 시작점의 바닥을 검증해 복귀시킨다. 이 지면도 없으면
GRABBED/input lock을 유지한다. owner 소멸·deadline·명시 중단도 같은 경로를 사용하며
해제 시 잔여 낙하/강제이동·pending command를 정리한다. 사망과 Mario 내부 규칙은 유지한다.
GameRoom_KoukuRaidFlow의 cinematic 시작도 현재 관문에서 모든 살아 있는 hook의 해제를
복사본에 먼저 stage한다. 하나라도 실패하면 owner 제거·gate scope 변경·일괄 action reset
전에 거절한다. cinematic 종료/중단 뒤 안전 해제 재시도가 사라지는 우회를 막는다.

## G03. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| Debug Server Build | Shared 참조 포함 정상 증분 컴파일·링크 PASS. server-debug-build-final.log |
| Debug Client Build | Engine/Shared 참조, Client 컴파일·링크와 runtime 배포 PASS. client-debug-build.log |
| --skill-stages-contract-test | 80 PASS, failures0. 실제 unsafe tangent 재현과 거절, 정상 Space/안전 slide/높이 경계/off-grid/navigation 부재, 높이 보정의 최종 겹침·중간 관통 거절과 기존 겹침 탈출 보존 포함. skill-stages-debug-final.log |
| --kouku-object-overlap-contract-test | 835 PASS, failures0. 기존66개 source hook과 새13개 하차/Space/중단/미확보 지면 재시도/사망 보존/cinematic 입장 검사 포함. hook-overlap-debug-final.log |
| HUD 실제 함수 CPU probe | 45 checks, failures0. 실제 MainApp 함수·DataJson·DirectXMath 투영, actor/camera/viewport/교체/숨김/비누적/신규키 기본값/legacy 보존/atomic Save/CAS 검사 |
| 형식 | 변경 C++ UTF-8 BOM없음/CRLF 보존, 해당 파일 git diff --check PASS |

빌드·실행 로그는 out/KoukuStaggerHook20260929, HUD 추출 본문·manifest·fixture는
out/KoukuMechanicFollow20260929에 있다. HUD probe의 actor/head/HUD/game-instance는
통제된 협력 객체다. 실제 애니메이션 본·GPU·입력·네트워크 화면 검증을 대신하지 않는다.
기존 코드페이지 경고는 남았고 컴파일·링크 오류는 없었다. 새 C++/project/filter/schema
파일과 이번 작업의 데이터 publisher 실행은 없다.
같은 요청의 추가 뿅망치 수정과 그 뒤 최종 Debug Client 빌드는
`2026-09-29_MARIO_HELD_HAMMER_RESULT.md`에 분리했다.

## G04. 마리오 실패 뒤 진행에 대한 답

현재 저장 Flow는 155줄 P88(1마리오),125줄 P91(2마리오),90줄 쇼타임,80줄 P92(3마리오),
55줄 P93(4마리오)다. 낙사·사망 복귀 및 미입장은 해당 기믹 실패 완료로 소비하므로 P33을
생략한다. 이후125줄 이하에서 현재 일반 패턴 종료 후 P91의 명시 stage2를 사용한다.
실패한1마리오 또는 그2페이즈를 재개하는 것은 아니다. 이 진행 정책은 이번에 변경하지 않았다.

## G05. 남은 사용자 확인

새 Debug Client/Server로 보스·카메라 이동 중 무력화 바 추종과 갈고리 하차 직후 Space를
확인한다. 에이전트는 제품 Client/UI를 실행·조작하지 않았다. 실행 중 기존 process가 새
바이너리로 자동 갱신되는 계약은 없으며 실제 사용자의 입력과 최종 화면 판정은 남아 있다.


## G05~G07. 댄스 HUD·일반 나팔·대형 세이튼·마리오 공 후속 반영

2026-09-29 후속 사용자 승인에 따라 아래 소스 변경을 완료했다. 기존 무력화 head anchor,
갈고리 안전 하차, Release 준비와 다른 세션의 마리오 색상·진행도 변경은 보존했다.

- `MainApp.cpp/.h`: MAZE만 제품 HUD 전체를 숨긴다. DANCE는 기존 QWER 등 제품 HUD를
  표시하고 `Is_KoukuBossHealthBarHidden`으로 보스 sprite와 제목·줄수 text를 함께 숨긴다.
  `KoukuHudModes.json`의 사용자 rect/scale/key/icon 저장값은 이 후속 작업에서 수정하지 않았다.
- `Level_KakulSaydonArena.cpp/.h`: 대형 세이튼 bundle.1(P8/P9)의
  `camera.kouku.pattern.1`만 캐릭터 표시와 gameplay input을 허용한다.
  `Is_CinematicInputBlocked`를 MainApp capture와 PlayerController 입력이 함께 소비한다.
  카메라·surroundings·fog 재생과 다른 cinematic의 표시/차단은 기존 경로를 유지한다.
  free camera, pending sequence, Server의 raid CINEMATIC 차단은 해제하지 않는다.
- `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`: revision 2468→2469,
  Gate2 entries 78→85. 140→125, 125→110, 110→95, 95→80, 80→55, 55→25, 25→0
  일곱 일반 구간 모두 P106 저글링→P105 나팔→P85 슈퍼바주카가 된다. 새 나팔 wait는
  기존 일반 패턴과 같은 1000ms이며 원래 entry ID·값·13개 group 경계는 그대로다.
- `GameRoom_KoukuMiniGames.cpp`: `Update_MarioBombContacts`의 FLYING_BALL 접촉은
  고정 1320 피해다. 기존 세대별 1회 판정, 충돌·knockdown·4m 이동·2m 포물선은 유지했다.
  `ServerGameplayContractTests_KoukuOverlap.cpp`의 실제 7개 marker×2세대 검사는
  최대 HP 13200/26400 양쪽에서 1320 피해를 기대하도록 바꿨다. 타겟공 색상 masking·
  matching count·망치 파괴 판정은 이 후속 변경에서 수정하지 않았다.

정본 병합은 최신 디스크를 `.writer.lock`으로 보호하고 기존 bytes에 7개 entry와 revision만
삽입했다. 교체 직전 freshness, `ReplaceFileW` 백업과 실제 백업 내용, 교체 후 bytes를
검사했으며 rollback은 필요하지 않았다. 근거는
`out/KoukuHudRotation20260929/install-receipt.json`이다. 원본 SHA-256은
`3ad2a3b35f020eed8d8dcc22fbb5b78648132b46b6a33ec33d7d4b58f8ea3faf`, 반영본은
`31f1f2d39766eec294721c092a6acbfbd6fd7c9c923ad3269b6dbddab3c97495`다.

## G08. 후속 집중 검증과 남은 실행 경계

실제 소스에서 두 MainApp HUD 함수, cinematic active/input/visibility 함수 및 shot helper를
추출한 C++20 CPU probe가 `/W4 /WX` 컴파일과 106개 검사를 통과했다. DANCE의 HUD 표시와
보스 HP 숨김, MAZE/다른 Level/invalid HUD, 대형 세이튼 camera1 허용, 다른·알 수 없는 shot
차단, 카메라 소유권 해제, sequence pending 차단을 확인했다. 보스 sprite/text와 UI/input
소비자가 같은 정책을 호출하는 것도 확인했다. 이 검사는 GPU 표시나 실제 picking 성공을
대신하지 않는다. 근거는 `out/KoukuHudRotation20260929/actual_policy_probe.cpp`와
`policy-source-manifest.json`이다.

flow 후보 검사에서 7개 순서, 기존 row/group 보존, 두 번째 적용 no-op, 무관한 동시 변경
보존을 확인했다. 최신 반영본에서 추가 7행과 revision을 제거하면 이전 JSON과 값이 완전히
같다. 변경 C++ 인코딩/BOM·CRLF를 유지했고 `git diff --check`를 통과했다.

게시 전에 실행한 projector `--check`는 source2469와 이전 projected Product의 차이 때문에
`projected Product is stale`을 반환했다. 이 결과를 게시 완료로 취급하지 않는다. parent가
공식 projector/publisher, Debug/Release 통합 빌드와 실제 Server overlap/raid 계약을 진행한다.
게시 후 read-only 검증 결과와 빌드 결과는 아래 또는 통합 RESULT에 기록한다. Client/UI는
실행하지 않았고 댄스 HUD, 대형 세이튼 카메라 중 캐릭터·우클릭 이동, 실제 공 피해 화면은
사용자의 실행 화면 확인 경계다. 추가 Resources는 필요하지 않았다.


parent의 공식 projector 게시 완료 후 최신 `project_kouku_saydon_composition.py --check`는
PASS했다(sourceRevision2469, productPattern114, productStage577, productBundle9, output2).
별도로 projected Encounter의 Gate2 entries와 entryGroups가 저작 정본과 일치하고 모든
7개 일반 반복의 시작 세 항목이 P106→P105→P85임을 확인했다. 첫 group 표시명은 기존
`140→125줄`을 유지하지만 진입 상한 HP를 제한하는 값이 아니므로 160줄 시작에서도
첫 반복부터 나팔을 포함한다. threshold와 PATTERN_END 전환값은 보존했다.
