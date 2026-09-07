# Kouku Clown·HUD·패턴 통합 결과

## G00. 범위와 현재 상태

2026-09-06, `GB/koukusaydon-pattern-1-complete`의 Claude 미커밋 구현과 사용자의
Save Tuning 값을 보존하며 연결했다. 시작 HEAD는 `d9b7c2d8a445ca4e1eb1813dd0b5f85d4aef8f34`다.
기존 dirty diff/파일은 `C:/Users/user/AppData/Local/Temp/lostark-kouku-integration-20260906`에 보존했다.
자동 stage/commit/push와 Client/UI 실행·캡처는 하지 않았다.

G01~G06은 source revision 61 / wire 62와 관문 표시 후속의 과거 checkpoint다. 이번 source 75
후속 상태·검증·실행 준비는 G07을 따른다. 과거 checkpoint의 Product/Gameplay/World publish,
관련 최소 컴파일과 격리 Server native 전체 계약 검사(1,181 PASS)는 통과했다.
당시 관문 표시 수정까지 포함한 Debug Product 전체 링크·SDK/shader/runtime DLL 배포도 통과했다.
사용자가 서면으로 확인한 즉사·부활 동작만 수동 PASS이며, 나머지 화면·포즈·효과는 사용자 확인 전이다.
후속 baseline은 `C:/Users/user/AppData/Local/Temp/lostark-kouku-followup-20260906`에 보존했다.

## G01. Clown 리소스와 스킬

변신 본체는 원본 Polymorph 4134에 연결된 `MN_RPCZ_00-1`이다.
원본 ActorX의 100본(변환 root/mesh node 포함 runtime 103 nodes), 23클립을 보존하고
4개 춤과 망치 1개를 오프라인 보정하여 28클립으로 설치했다. 원본 mesh/material/skeleton과
23개 animation의 26개 section payload는 변경 전과 바이트 단위로 동일하다.

| 모드/키 | clip | 길이 |
|---|---|---:|
| Clown Q 폭탄 43340 | `rpcz00p_att_battle_1_01` | 1.5초 |
| Clown W 나팔 43341 | `rpcz00p_att_battle_2_01` | 1.5초 |
| Clown E 배송 43343 | `rpcz00p_att_battle_4_01` | 1초 |
| Mario Q / Card Maze Q 망치 56411 | `rpcz00p_project_tuned_hammer` | 2.5초 |
| Mario W 점프 43342 | `rpcz00p_att_battle_3_01` | 1.5초 |
| Dance Q 양팔 모으기 | `rpcz00p_project_tuned_pose_arms_folded` | 3초 |
| Dance W 슈퍼맨 | `rpcz00p_project_tuned_pose_superman` | 3초 |
| Dance E 양팔 벌리기 | `rpcz00p_project_tuned_pose_arms_open` | 3초 |
| Dance R 한 다리 올리기 | `rpcz00p_project_tuned_pose_one_leg` | 3초 |

네 춤은 보스 165본 모션의 25_04/25_05/25_06/25_03, 망치는 PC_WR_00의 215본
`PR_IT_GSTFP_00_Att_2_01`에서 공통 43개 몸체 본을 사용했다. 대상 본 길이를 보존하며
world-rest 보정한 `project_tuned` 표현이다. 원작 동일 자세나 visual PASS를 주장하지 않는다.
원본 근거와 베이킹 기록은 `C:/LostArkExtract/MN_RPCZ_00-1_Cook_20260906/SourceEvidence`에 있다.

`Clown.interactionbindings.json`의 10개 mode/index binding은 설치본의 실제 clip과 일치한다.
새 JSON은 Client 프로젝트 `96.DataFiles/Animation/Authored/KoukuSaydon`의 None 항목으로 등록했다.
Animation Resources에서 `MN_RPCZ_00-1` 전체 clip을 재생할 수 있다.
Clown body admission scale은 기존 0.017에 0.709를 곱한다. 프라이팬은 본체 submesh가 아니라
이전에 `b_wp_1`에 추가했던 별도 `IT_GSTFP_00` part였다. 최종 ClownSpec에서 장착 part를
제외했다. 본체 mesh와 28개 clip, Mario/Maze의 망치 몸동작은 보존했다.

## G02. HUD·입력·판정·부활

광기 100% / Change to Clown은 Clown HUD, dance 판정 창은 Dance HUD를 선택한다.
네 모드는 Server의 snapshot으로 실제 avatar와 HUD를 함께 전환한다. F1의 Clown 패널에 있는
HUD mode 선택도 같은 typed Debug 명령을 사용한다. Return to Player는 원래 class로 복귀하고
현재 pattern occurrence가 다음 tick에 같은 모드를 다시 강제하지 않도록 억제한다.

`CPlayerController -> IPlayerCommandSink -> Server -> snapshot -> CCharacter` 경로로
스킬 index와 action start tick을 전달한다. interaction 중 class skill/평타를 보내지 않는다.
모든 interaction cooldown은 Server 기준 3초이며 기존 snapshot cooldown 목록의 예약 ID로
HUD 숫자와 arc를 표시한다. 최신 사용자 지시에 따라 UI Preview를 실제 명령으로 바꾸던
후속 수정은 취소했다. 기존 `Kouku UI Preview (Debug)`는 표시 전용 override로 유지한다.

춤은 Q/W/E/R = 양팔 모으기/슈퍼맨/양팔 벌리기/한 다리 올리기로 고정했다.
판정 창의 첫 잘못된 입력은 fail, 아무 유효 입력 없이 창이 끝나면 timeout이다.
기존 각 결과의 HP 처리는 보존하고, 기존 `MADNESS +50%` RESULT를 fail/timeout에 함께 연결했다.
새 증가 수치를 만들지 않고 사용자가 저장한 50% RESULT 정의를 재사용했다.

실제 Mario1의 `Mario1_go`와 `Mario1_Trigger_5` 이동이 도착한 시점에 각각 MARIO/NONE을 적용한다.
movePlayer event의 optional `koukuHudMode`를 Client authoring·publisher·Server까지 연결했다.
카드미로 Debug gate는 사용자 좌표 `(0.09, -0.01, 1351.48)`를 등록했고 보스를 생성하지 않는다.
이 지점의 navigation 수치 확인은 통과했다. Mario2~4의 실제 진입점은 미등록 상태를 유지한다.
현재 F1에서 네 mode의 모델/HUD/스킬을 확인할 수 있다.

쿠크의 사망 화면은 Valtan과 동일한 DeadScene JSON·레이아웃·입력 경로를 사용한다.
부활 버튼은 Server에 요청하고, 사망 XZ가 walkable이면 그 지면 높이에 부활한다.
그 위치가 더 이상 유효하지 않으면 기존 navigation projection을 사용한다.
즉사·부활 동작은 이번 후속 요청에서 사용자의 직접 관찰로 확인됐다.

## G03. 진짜 찾기·분신·Composition 오류

무력화 Pattern1의 Pattern4 followup 오류는 이름 변경이 원인이 아니었다.
행 단위 Parse에서 자기 행만 들어 있는 임시 document로 cross-pattern link를 검증해서 생겼다.
행 자체의 shape는 즉시 검사하고, 다른 Pattern 참조는 전체 문서를 읽은 뒤 검증한다.
존재하지 않는 target 거부와 실패 시 기존 document 보존은 유지한다.

Pattern2의 TRIGGER를 `REAL_GAZE_TELEPORT`로 지정했다. trigger 시작 1988ms에 진짜가
`(-6.36, 1.3, 937.92)`로 이동하고, Pattern5를 재생하는 종속 Saydon 3개를 함께 생성한다.
원래 중심 `(-0.07, 1.32, 942.33)`에서 XZ 반경은 약 7.68196m이다.
사용자 지정 지점을 1시로 삼아 나머지 시각의 상대 각도를 계산하고 모두 중심을 바라본다.

| 위치 | X/Y/Z | 보정 후 논리 yaw / 본체 정면 yaw |
|---|---|---:|
| 진짜 1시 | -6.36 / 1.3 / 937.92 | -35.035° / 54.965° |
| 가짜 4시 | -4.48003 / 1.3 / 948.62000 | 54.965° / 144.965° |
| 가짜 7시 | 6.22000 / 1.3 / 946.74005 | -215.035° / -125.035° |
| 가짜 10시 | 4.34003 / 1.3 / 936.04004 | -125.035° / -35.035° |

실제 published navgrid의 네 점은 모두 walkable이고 지면 Y는 약 1.299005다.
수치 근거는 `out/KoukuIntegration-gaze-navigation-followup.json`이다.
`clockHours=[4,7,10]`과 `faceCenterYawOffsetDegrees=-90`은 +X인 본체 정면을 반영한다.
기존 owner boss graph와 spawn/snapshot/despawn 경로를 사용하며 가짜가 boss HUD를 덮어쓰지 않는다.
F1 Clown 위 시야 패널은 Composition의 GAZE_REAL_BOSS 각도·거리를 저장하고 cone debug를 표시한다.
저장한 판정 값은 Gameplay publish와 Server 재시작 후 적용된다. collider debug는 판정 권위가 아니다.

## G04. 튜닝·WORLD·Collider·효과

G2 Kouku를 포함한 5개 boss body scale, G2 Big Saydon position/yaw를 기존 Save Tuning에 연결했다.
Animation preview는 현재 조절한 body/hammer scale과 hammer 3축 rotation을 소비한다.
사용자가 이미 저장한 값은 보존했다.

커튼 placement 11과 룰렛 placement 40은 원본 위치가 G1 중심에서 약 205m 떨어져 있었다.
WORLD 정의에 `[0.249, 0, 204.799]` positionOffset을 넣어 재생 중에만 G1 중심으로 옮긴다.
Composition local Play도 기존 WorldSequencePlayer로 WORLD clock을 실행하고 Stop에서 원래 상태를 복구한다.
Server WORLD cue의 speed가 Client에서 누락되던 경로도 복구했다. 최신 wire version은 62이며
WORLD offset과 함께 카드 색 및 보스 pattern clock을 복제한다.
배포 worldsequences는 authoring과 parsed 내용이 같고(56 templates/60 instances), mapplacement
3231개도 원본과 일치한다. 룰렛 1개·커튼 11개가 현재 load scope에서 준비되고 관련 모델의
texture 7개와 맵 catalog의 323개 wmodel이 모두 존재한다. offset은 Server bootstrap까지 일치한다.
Dance/Roulette는 `resetBossToSpawn`으로 시작 위치를 복구하고 WORLD도 보스 spawn 앵커를 사용한다.
룰렛 WORLD speed는 `22200/34072`(약 0.6515614), 표시 창은 34072ms다. 원본 22200ms의
hide key를 늘려 세 번째 판정 23879ms 이전에 바닥이 사라지던 시간을 수정했다.

6칸 검토는 실제 UV 재확인으로 대체했다. mesh yaw 0에서 +Z를 기준으로 중심각
`22.5 + 45 × n`인 8칸은 다음과 같다.

| 중심각 | 문양·색 |
|---:|---|
| 22.5° | SPADE RED |
| 67.5° | CLUB BLACK |
| 112.5° | DIAMOND RED |
| 157.5° | HEART BLACK |
| 202.5° | CLUB RED |
| 247.5° | SPADE BLACK |
| 292.5° | HEART RED |
| 337.5° | DIAMOND BLACK |

8개 Collider를 판정 창마다 배치한 총 24 box와 3개 DURATION을 연결했다. 각 Collider가
`regionId/cardSymbol/cardColor`, `anchorKind=WORLD/worldId`, `logicOccurrenceId`를 소유한다.
같은 회차의 8 box는 하나의 DURATION과 기존 success/fail/timeout RESULT 연결을 공유한다.
세 창은 4286..9079, 11724..16479, 18965..23879ms이며 현재 모두 enabled다.
Server는 종료 tick의 플레이어 XZ를 WORLD 변환이 적용된 지역에 대조한다. 카드의 문양과 색이
모두 같으면 success, 다른 카드면 fail, 어느 지역에도 없으면 timeout이다. 공유 부채꼴 경계는
한쪽만 포함하고 둘 이상의 지역에 포함된 잘못된 설정은 fail로 처리한다.
지역별 별도 결과 실행기를 만들지 않고 기존 Logic/RESULT를 재사용한다. 일반 영역도
`AREA_OVERLAP` DURATION과 `ENTER_AREA` TRIGGER에서 같은 연결을 소비한다. `ENTER_AREA`는
source 61 checkpoint에서는 live BOSS 앵커만 지원했고 animated WORLD entry는 publisher가 거부했다.
Client의 Collider는 `CHitAreaWire` debug geometry이며 판정 권위는 Server XZ primitive에 있다.

Composition의 optional `presentationResources/presentationOccurrences`를 Workbench Resources,
Box Detail, Save/Reload, projector 및 새 `CKoukuSaydonPresentationPlayer`로 연결했다.
Client Product의 `patterns[]`를 stage한 뒤 snapshot의 pattern ID/start tick/sequence로 실행하며
Product load 실패를 authoring 직접 읽기나 임의 시작 시각으로 숨기지 않는다. 새 Player h/cpp는
Client vcxproj/filters에 등록했다. Preview Play/seek/pause/Stop과 owner 종료는 자신이 만든
Effect/Sound/Camera handle 및 Scene Profile을 정리한다. WORLD anchor는 source worldId에서
sequence instance를 찾아 실제 map placement TRS를 적용한다. Scene Profile은 즉시 적용·복원하며
blendMs 시간 보간은 이번에 구현하지 않았다.

| 효과 | 최종 배치 |
|---|---|
| 방패 `boss.kouku.disarm.shield_1` | Pattern1 단일 box, 5263..15947ms, yaw +90° |
| 진짜 하트 | Pattern2의 6767/13167/19567ms 시작, 각 2400ms, 3 box |
| 가짜 별 | Pattern5의 6166/12566/18966ms 시작, 각 934ms, 3 box |

방패는 body clip 반복과 독립된 수명으로 한 번 fade-in/hold/dissolve-out한다. box별
fadeInMs/fadeOutMs와 정규화 수명 기준 dissolveStart/End는 runtime clone override이며
전역 V2 디자인을 수정하지 않는다. 방패 occurrence yaw와 Server 반사 normal offset에 각각
+90°를 적용했다. 사용자 회전 보정 범위에 맞춰 보스 body admission 전체 회전은 적용하지 않았다.
이관이 끝난 `MN_RPCT_05.effectv2bindings.json`의 방패·별 row는 제거하여 중복 재생을 막았다.

G1 Saydon 활성 동안 Server가 문양 4종 × RED/BLACK 중 하나를 배정한다. Client는
`boss.kouku.card.{heart,spade,dia,clober}.{red,black}`의 8 group을 머리 위에 표시한다.
`clober`는 실제 asset ID의 철자다. 카드 group 원본의 Z 보정은 메모리 복사본에서만 제거하고
duration 0의 지속 handle을 사용한다. snapshot NONE·G1 종료·퇴장·owner 정리 때 제거한다.
remote effect branch에서 필요한 데이터 29개 경로(26 추가/2 수정/1 삭제)만 통합했으며 다른
feature 코드는 merge하지 않았다. 관련 26 leaf/12 group과 22개 고유 Resources 참조가 존재한다.

## G05. 자동 검증과 실행 준비

다음 표는 과거 checkpoint(source 61 / wire 62)에 대해 실행한 검사만 기록한다. 이후 source 75 후속은 G07에서 별도로 기록한다.

| 확인 | 실제 결과/증거 |
|---|---|
| Composition Product publish | 성공, `out/KoukuIntegration-projector-publish-followup.log` |
| Gameplay / World publish | 성공, `out/KoukuIntegration-gameplay-publish-followup.log`, `out/KoukuIntegration-world-publish-followup.log` |
| Composition projector | 41 tests OK, `out/KoukuIntegration-projector-followup.log` |
| Shared Debug Build / Server·Engine ClCompile | 성공, `out/KoukuIntegration-shared-build-followup.log`, `out/KoukuIntegration-server-compile-followup.log`, `out/KoukuIntegration-engine-compile-followup.log` |
| NetworkProtocolHarness | 674 PASS, failures 0, `out/KoukuIntegration-network-contract-followup.log` |
| 격리 Server native contract | 1,181 PASS, failures 0, `out/KoukuIntegration-server-contract-followup.log` |
| Composition editor native contract | 왕복·격리·링크·Save/Reload·CAS 통과, `out/kouku-presentation-editor-tests.log` |
| Effect runtime / object / PresentationPlayer 최소 C++ 컴파일 | MSVC exit 0, 아래 임시 디렉터리의 `compile.log`, `player_compile.log` |
| 실제 Effect V2 C++ Parse/Serialize/Parse | 26 leaf/12 group 왕복, 물리 참조 및 이관 후 빈 NPC binding parse 통과 |
| JSON/XML | 변경 JSON 40개/XML 4개 parse 통과, 통합 담당 실행 기록 |
| 1/4/7/10시 navigation 수치 | 네 점 walkable, `out/KoukuIntegration-gaze-navigation-followup.json` |
| Card Maze navigation | 요청 좌표 walkable, blocker 0, `out/KoukuIntegration-card-maze-navigation.json` |
| 문서 diff whitespace | 이번 문서 갱신 후 `git diff --check`로 확인 |

Effect 검증은 제품 `EffectV2_Document.cpp`와 기존 JSON/path 소스를 그대로 컴파일한 임시 CLI다.
검증 파일과 로그는 `C:/Users/user/AppData/Local/Temp/LostArk-EffectOccurrence-20260906`에 두었고
새 저장소 하네스를 추가하지 않았다. 실제 C++ Composition의 cross-pattern forward link와
missing target 거부, enabled/trigger/WORLD 보존은 이전 임시 CLI와 이번 editor native contract에서
확인했다. 설치 28클립의 10개 interaction binding은 실제 clip 이름과 일치한다.

### G05-1. 최종 통합 검증

- 마지막 MainApp/Level/Character/CharacterCatalog/ClientReplication/PresentationAssetService 6개
  CPP는 임시 출력 경로로 최소 컴파일해 exit 0을 확인했다.
  로그: `out/KoukuIntegration-client-compile-followup.log`.
- 일시정지 상태의 1~150ms 전방 Seek에서도 AnimationTool과 Effect/Sound를 함께 갱신한다.
  기존 handle을 정리하고 WORLD를 먼저 샘플한 뒤 같은 clock으로 cue를 재생성한다.
  변경한 PresentationPlayer의 최소 컴파일도 exit 0이며,
  `out/KoukuIntegration-presentation-player-compile-followup.log`에 기록했다.
- 기존 Server 프로젝트를 `out/KoukuServerContract`의 격리 출력에 링크한 뒤
  `--contract-test` 실행 결과 1,181 PASS, failures 0이다. listener와 Client/UI는 실행하지 않았다.
- 표준 명령 `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`를
  실제 실행했으나 `ProductOutputGuard.psm1`이 Client PID 31128 / Server PID 61072를 감지해
  컴파일 시작 전 중단했다. 로그: `out/KoukuIntegration-product-build-followup.log`.
- 이후 두 프로세스 종료를 확인한 뒤 같은 표준 명령을 다시 실행해 Debug Product 전체
  Engine → Shared → Server → Client 컴파일·링크·SDK/shader/runtime DLL 배포가 exit 0으로 완료됐다.
  로그: `out/KoukuIntegration-product-build-gate-label.log`.
  공식 receipt: `out/BuildPipeline/runs/20260906T121337447Z-debug-product.json`.
  기존 인코딩·외부 라이브러리 PDB·shader 경고는 남으며 컴파일/링크 오류는 없다.
- Kouku(`MN_RPCZ_00`)와 Large Saydon(`MN_RPCT_06`) Pattern 목록의 상단은 선택 모델에 따라
  2관문으로 표시한다. 고정 Gate 1 안내 문구 두 곳도 제거했다. 저장 ID와 Product 내용은 유지한다.
  방패 source/Product는 5263..15947ms(10684ms), fade-in 200ms/fade-out 450ms,
  dissolve 0.955..1.0의 단일 box로 일치한다. 이 후속은 C++ 표시만 수정해 JSON/XML 재배포는 하지 않았다.
  `git diff --check` 통과, 화면 확인은 사용자 진행이다.

이전 rev60/wire61 checkpoint의 Debug Product 성공과 Server native 1171 PASS 기록은
`out/KoukuIntegration-product-debug-final.log`, `out/KoukuIntegration-server-contract-final.log`에
남아 있다. 이 과거 결과를 위 최신 후속 변경의 전체 검증 결과로 사용하지 않는다.

## G06. 사용자 확인과 Resources 전달

새 Clown runtime 폴더는 현재 PC에 설치했다. Resources는 Git에 추가하지 않았고 Drive 전달은 아직 하지 않았다.

| Resources 상대 폴더 | 물리 위치/내용 |
|---|---|
| `Character/KoukuSaton/MN_RPCZ_00-1/` | `Client/Bin/Resources/Character/KoukuSaton/MN_RPCZ_00-1/`, wmodel + texture 3개 |
| 기존 `Character/KoukuSaton/IT_GSTFP_00/` | wmodel + texture 2개는 물리 폴더에 남아 있으나 최종 Clown의 장착 part에서 제외 |

카드·하트·별·방패의 V2 JSON은 Git 데이터이며, 필요한 실제 mesh/texture 22개 참조는 현재
`Client/Bin/Resources`에서 확인했다. 이번 통합으로 별도 effect binary 팩을 생성하지 않았다.

기존 loader 오류에서 누락됐던 `MAP_CACE4367A0BC_BG_RAD_KOUKUSATON_DECO14_SM_OVR_MOONGLOW01.wmodel`은
리소스 재투입 뒤 현재 물리 위치에서 존재함을 확인했다. Client 화면 진입을 대신 실행해 PASS로 기록하지는 않았다.

팀 LAN 설정은 server-host, TCP 7777 LocalSubnet 준비 상태다. 에이전트가 Client/UI를 실행하거나
조작하지 않았다. 위 관문 표시 checkpoint에서 Debug Product 빌드·배포까지 완료했으며, 그 시점에는
Server listener와 Client를 실행하지 않았다. 이후 사용자 실행 상태와 G07 후속 검증은 아래에서 구분한다.
Visual Studio `Server + Client` profile을 사용자가 Ctrl+F5로 시작한다.
wire 62이므로 양쪽을 함께 재시작한다.
Lobby→KoukuSaydon→F1→Clown에서 Change to Clown, 네 HUD mode와 QWER, Return to Player를 확인한다.
F1→Action Workbench→Boss Saydon→Composition Resources→Physical Animation→Refresh에서
`MN_RPCZ_00-1`을 펼쳐 기본/춤/망치 clip을 누르거나 Play Preview로 재생한다.
Arena Boss Tuning에서 G2 Kouku scale 및 Big Saydon transform/망치 각도를 조절하고 Save Tuning한다.
Composition Patterns에서 `세이튼_댄스타임`/`세이튼_룰렛`을 선택하고 Composition Sequencer의
Play로 WORLD를 확인한다. WORLD box의 Box Detail에는 World position offset (m), Playback speed x가 있다.
실제 Server 판정은 `KoukuSaydon Complete Play (Server Boss Replay)`→Load/Reload KoukuSaydon Inventory→
패턴 선택→Complete Play로 무력화, 진짜 찾기, Dance와 Roulette를 확인한다.
카드미로 gate에서 보스가 생성되지 않는지, G1 카드의 문양·색과 머리 위 표시가 일치하는지,
8칸 debug Collider와 바닥 UV가 겹치는지, 세 판정 창의 success/fail/timeout을 사용자가 확인한다.
방패 단일 fade와 반사 방향, 하트·별의 소유자/시간, Clown 크기·프라이팬 제거·네 포즈도 사용자
관찰 전이다. 즉사·부활 외의 manual first pixel 또는 visual PASS는 기록하지 않았다.

## G07. 룰렛 네 회차·Collider 저작·전체화면 커튼

### G07-1. source 75와 사용자 저장 보존

최종 source는 revision 75다. 네 회차 수치 증거는 revision 74 시점의 timing JSON을 유지하고,
revision 75는 커튼 companion과 명시적 Effect 배치를 추가했다. 사용자 revision 73의 네 번째 Duration `KAKULSAYDON_G1_PATTERN_7.logic.5`를
27487..32231ms(4744ms)로 보존했다. 연결 Logic은 `kakulsaydon.g1.logic.19`이고
OnFail의 `kakulsaydon.g1.logic.6`, `kakulsaydon.g1.logic.15`, 비어 있던 Success/Timeout도 유지했다.
판정 종류만 `AREA_OVERLAP`, `insideOutcome=FAIL`로 명시하고 Circle geometry
`kakulsaydon.g1.presentation.15`와 box `.presentation.25`를 같은 창에 연결했다.
WORLD placement scale 4를 반영한 실제 반경은 8m(원본 radiusM 2)다. 창 끝에 안쪽이면 기존
OnFail, 바깥이면 기존 OnTimeout을 실행하며 카드 문양을 추가 조건으로 만들지 않는다.

앞의 3개 `ROULETTE_CARD_MATCH` Duration과 24개 카드 지역은 유지했다. 네 회차는
`world.sequence.instance.8`의 183개 transform key, 속도 1, 전체 33773ms로 샘플한다.
회전 중 각속도는 288°/s이고 각 회차 끝 1500ms는 선형 감속이다. 현재 수치 정본은
`out/KoukuIntegration-roulette-four-cycle-timing.json`이며 sourceRevision 74다.

| 회차 | 회전 시작 | 감속 시작 | 정지 | 짧은 재회전 |
|---:|---:|---:|---:|---:|
| 1 | 4333ms | 7500ms | 9000ms | 10428..11625ms |
| 2 | 11667ms | 14834ms | 16334ms | 17762..18959ms |
| 3 | 19001ms | 22168ms | 23668ms | 25096..26293ms |
| 4 | 26335ms | 29502ms | 31002ms | 32430..33627ms |

이 변경은 이전 checkpoint의 `22200/34072` 재생 속도를 대체한다. 과거 24 box·3판정 기록과
source 61 검증은 당시 증거로 남기며 현재 4회차 전체 검증으로 올리지 않는다.
`kakulsaydon.g1.sceneprofile.1 -> scene.kakulsaydon.find-true-dark.v1`과 Pattern2의
2007..26134ms(24127ms) 배치는 원래 연결을 유지했다. `blendMs=500`은 기존 저장값이며
실제 Scene Profile 런타임은 여전히 즉시 적용·복원한다.

### G07-2. Collider와 Preview 연결

Collider resource에 `CIRCLE`, box에 optional `debugRender`(기본 true)를 연결했다.
Debug Render는 gameplay 활성 여부가 아니라 debug wire 표시만 바꾼다. `logicOccurrenceId`로
기존 DURATION 또는 ENTER_AREA TRIGGER와 연결하고 해당 box의 Success/Fail/Timeout RESULT를
재사용한다. Trigger의 피해값은 기존 `MAX_HP_PERCENT_DAMAGE` RESULT의 최대 HP %로 저작하며
ENTER_AREA와 함께 원자 생성·재사용한다. 다른 결과 슬롯을 지우지 않는다.

WORLD ENTER_AREA는 같은 pattern의 WORLD 배치와 단일 실제 MAP_PLACEMENT TRS/key를
투영하고 Server tick에서 샘플한다. Client가 보이는 box를 별도 gameplay 정답으로 보내지 않는다.
동일 WORLD의 lifetime 안에 판정 창이 들어가야 하며 알 수 없는 binding/잘못된 transform은 거부한다.

WORLD 정의의 `companionEffectResourceId`와 EFFECT box의 `worldOccurrenceId`는 World
Preview/Append의 동반 재생·배치를 위한 명시적 연결 계약이다. 제품은 저장된 Effect box만 재생하고
연결된 box는 독립 편집할 수 있다. 단독 Collider Preview의 마지막 유효 시점 유지, source draft
generation 변경 시 재stage, Debug Render 즉시 반영 및 World companion dispatcher를 연결했다.
최종 통합 컴파일·계약 검증은 G07-4에 별도 기록한다.

revision 75의 WORLD `kakulsaydon.g1.world.2`는 EFFECT resource
`kakulsaydon.g1.presentation.16`을 companion으로 참조한다. Pattern6의 WORLD `.world.1`과
EFFECT `.presentation.1`은 모두 0..2671ms이며 Effect box의 `worldOccurrenceId`가 WORLD를
가리킨다. WORLD 속도는 `3500/2671`(약 1.31037065)로 native 3500ms를 배치 창에 맞췄다.
Resources의 단독 World Preview는 native 3500ms와 같은 길이의 companion Effect를 재생한다.
기존 맵 커튼 11개도 0/1050/2450/3500ms에 Y offset 18→0→0→18, 마지막 hide를 적용했다.
이 기존 WORLD key와 화면 overlay는 같은 하강·유지·상승 비율을 사용한다.

### G07-3. 원본 커튼 근거와 설치

원본 `Monster_BaseBuff_KoukuSaton_Curtain` LOA는 `CEFPostProcessMaterialEffectStatus`로
`FX_M_MI_D_00.FX_M.FX_D_Po_Curtain`을 참조한다. source는
`C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/CanonicalSource/Sound/Lpk/EFGame_Extra/ClientData/XmlData/ParticleSoundNew/9_EF_PARTICLE_SOUND_DATA_BUFF_FX_Monster_BaseBuff_KoukuSaton_Curtain.loa`다.
재질은 cached `CanonicalSource/Effect/Closure/SourcePackages/fx_m_mi_d_00/ZHJ4TC4PCK4PL4J22HIXEYUX5U.upk`에서 읽었다.

Material native tail 88바이트를 끝까지 해석한 단일 texture reference는
`fx_tex_high_00.fx_d_symbol_100_ycl`이다. 1024×1024 RGBA DDS의 빨간 커튼·광대 문양을 직접 열람했고
기존 추출본을 바이트 변경 없이 설치했다. 35개 expression 중 34개가 null인 `COOKED_PARTIAL`이므로
원본 shader 동작·시간을 완전히 복원했다고 기록하지 않는다.

새 leaf JSON은 Client project/filters의 `96.DataFiles/Effects/V2/Authored`에 등록했다.
새 leaf `boss.kouku.curtain_1`은 기존 Effect V2의 `TexturedOverlay` profile을 통해
`CPresentation_Manager::Add_ScreenOverlay`와 기존 Deferred pass 14를 사용한다. Engine/Shader
수정과 평행 렌더링 경로는 없다. 기본 3500ms 수명의 0..0.3 하강, 0.3..0.7 유지, 0.7..1 상승은
프로젝트 조정값이다. normalized 위치는 시작/끝 `(0.5,-0.54)`, 유지 `(0.5,0.52)`, 크기 `(1,1.08)`이며
원본 하단의 투명 끝이 유지 구간에서 화면 아래로 내려가도록 했다. Effect Tool에서 위치·크기·회전·
진입/퇴장 비율과 opacity를 저장한다. 이 profile은 alpha envelope를 사용하고 texture dissolve는 사용하지 않는다.

필수 Resources 상대 ID는 `Effect/KoukuSaton/Screen/fx_d_symbol_100_ycl.dds`, 물리 위치는
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/KoukuSaton/Screen/fx_d_symbol_100_ycl.dds`다.
1,048,704바이트가 기존 원본 추출과 동일함을 확인했다. Resources는 Git에 추가하지 않았고
**Drive 전달은 미수행**이다. 원본 커튼 사운드 WAV 3종은 기존 폴더에 존재하지만 선택 근거가 없어
하나를 임의 연결하지 않았다. 기존 원형 맵 커튼 11개를 화면 전체 연출의 원본으로 간주하지 않는다.

### G07-4. 이번 후속 검증과 최종 통합 기록

| 검증 대상 | 이번에 확인한 결과 |
|---|---|
| 커튼 C++ 최소 컴파일 | EffectV2_Object.cpp, EffectV2_Document.cpp, Effect_Tool_V2.cpp MSVC exit 0; 기존 SDK 인코딩 경고는 남음 |
| 실제 C++ Effect V2 왕복 | Kouku 27 leaf/12 group Parse→Serialize→Parse 및 child 문서 존재 PASS |
| 커튼 CPU 샘플·실패 보존 | 수명 0/.15/.3/.5/.7/.85/1 위치와 invalid phase·빈 Base 거부, 기존 document 보존 PASS |
| 기존 Python V2 publisher | 새 profile을 동일하게 검증, curtain natural lifetime 3500ms, 잘못된 phase·scale 거부 PASS |
| 커튼 데이터/물리 입력 | JSON parse, Python syntax, 원본 DDS byte 동일 PASS |
| 룰렛 source timing | source 74 timing JSON과 네 번째 Duration/Result 실제 데이터 대조; 사용자 화면 판정 미수행 |
| 문서 whitespace | 네 기존 문서 갱신 후 `git diff --check` 확인 |
| Map publish | source 75 입력으로 exit 0, `out/KoukuIntegration-roulette-map-publish.log` |
| Composition Product publish | sourceRevision 75, Product 6 pattern/66 stage, exit 0, `out/KoukuIntegration-roulette-product-publish.log` |
| Rendering publish | 기존 sceneprofile.1 closure 포함 PASS, `out/KoukuIntegration-roulette-rendering-publish.log` |
| 최종 Gameplay/World gameplay publish | source 75 기준 모두 exit 0; `out/KoukuCollider-gameplay-publish.log`, `out/KoukuCollider-world-publish.log` |
| Preview/companion 통합 최소 컴파일·계약 검증 | MainApp/PresentationPlayer 최소 컴파일와 Workbench/Document 최소 컴파일 exit 0; 기존 editor contract PASS (`out/KoukuIntegration-roulette-preview-compile.log`, `out/kouku-world-companion-editor-tests.log`) |
| 배포본 Python/Server 계약 | Python 44/44 PASS (`out/KoukuCollider-projector-published-tests.log`), source 75 bootstrap Server 1,183 PASS / failures 0 (`out/KoukuCollider-server-published-contract.log`) |
| 사용자 저장 보존 | 작업 전 source 73과 source 75를 직접 대조: Scene Profile 정의·모든 occurrence, 룰렛 9 stage·4 Logic occurrence 동일 PASS |
| 최종 JSON/XML·whitespace | 변경/신규 JSON 41개, project/filter XML 4개 parse PASS; `git diff --check` exit 0 (기존 newline 변환 경고만 존재) |
| 최종 Debug Product 링크·SDK/shader/DLL 배포 | 표준 Product 빌드 exit 0, Engine→Shared→Server→Client 전부 PASS; `out/KoukuIntegration-roulette-four-cycle-product-build.log`, receipt `out/BuildPipeline/runs/20260906T134232445Z-debug-product.json`; missingRuntimeInputs 비어 있음. 기존 SDK 인코딩·외부 PDB 경고는 남음 |

커튼 native CLI와 출력은 저장소 밖 `C:/Users/user/AppData/Local/Temp/LostArk-Curtain-20260906`에
두었다. 새 저장소 하네스는 만들지 않았다. `parse_compile.log`와 `parse.exe`가 실제 제품 C++ parser를
사용한다. 최초 CLI의 resource root 미지정 실패는 `LOSTARK_RESOURCE_ROOT`를 설치 폴더로 지정한 뒤
재실행해 통과했으며 리소스 누락을 fallback으로 숨기지 않았다.

작업 중 사용자 Client PID 48416 / Server PID 61428(2026-09-06 21:18:17 KST 시작)을 읽기 전용으로
확인하고 보존했다. 이후 사용자가 두 프로세스를 종료했으며 문서 최종 갱신 때 두 PID가 없는 것을
재확인했다. 에이전트가 프로세스를 종료하거나 Client/UI를 실행·조작·캡처하지 않았고 새 listener도
시작하지 않았다. source 75의 데이터 배포와 표준 Debug Product 컴파일·링크·SDK/shader/DLL 배포가
완료됐다. 사용자 실행 경로는 Visual Studio `Server + Client` profile의 `Ctrl+F5`이며 이 PC는 LAN
`server-host`다. 사용자의 기존 즉사·부활 관찰 외에 이번 네 회차 회전·판정, Circle debug
토글·Trigger 피해, Scene Profile, 전체화면 커튼 하강·상승의 manual/visual PASS는 아직 없다.

## G08. Collider WORLD pivot 표시 오류 후속 수정

사용자는 G07 반영 뒤에도 Debug Render가 켜진 Box Detail Preview와 Sequencer Play 모두
Collider가 보이지 않으며 진짜 찾기 시야 Collider는 보인다고 보고했다. 두 표시 경로는 같은
CHitAreaWire/ImGui background drawlist를 사용하고 MainApp은 EndFrame 전에 표시하므로
근거 없이 draw layer를 변경하지 않았다.

### G08-1. 실제 원인과 변경

Level_KakulSaydonArena::Try_GetCompositionWorldPivot이 읽던 placement.record는 재생 복구용
원본이다. WorldSequencePlayer는 실제 object/batch에 샘플 TRS를 적용해도 원본 record는 바꾸지
않는다. 현재 룰렛40의 원본은(-.319,1.9,737.531), G1 실제 배치 샘플은(-.07,1.9,942.33)이라
디버그 anchor가 Z방향 약204.799m 떨어져 있었고 회전도 반영하지 않았다. 서버 판정 경로와
이 Client 표시 경로는 별개이며 이번 변경은 Server 계산을 수정하지 않는다.

WorldSequencePlayer가 성공적으로 적용한 map placement sample을 active instance에서 조회하는
Try_GetSampledPlacementRecord를 추가하고 Level pivot 소비자를 연결했다. 원본은 그대로 두며
seek/회전/offset/scale을 반영한 TRS를 사용한다. 첫 sample이 아직 없으면 Collider는 해당 프레임
표시를 보류하고 다음 프레임에 다시 시도한다. 이전처럼 한 번 실패했다고 영구 숨기지 않는다.
단독 Collider Resource Preview는 아레나 플레이어를 우선 기준으로 삼고 기존 Animation/Effect와
패턴 Preview의 target 선택은 보존한다. WORLD anchor는 그 위에 실제 World pivot을 사용한다.

### G08-2. 검증

WorldSequencePlayer/PresentationPlayer 최소 컴파일 exit0:
`out/KoukuCollider-world-pivot-mincompile.log`. 제품 Sample_Track/Compose_SampledRecord/getter
함수의 실제 구현을 발췌해 현재183key 룰렛 데이터와 연결한 저장소 밖 CPU CLI에서 54개 PASS:
`out/KoukuCollider-world-pivot-sample-check.log`. 전체 World runtime/GPU 실행 검사는 아니다.
샘플 위치·회전·반경8m, 원본 보존, 미준비/종료/다른 ID의 거절을 확인했다. 통합 MainApp/Level
최소 컴파일 exit0는 `out/ArenaCamera-Collider-integration-mincompile.log`다.

최종 Debug Product 링크·SDK/shader/DLL 배포 exit0:
`out/ArenaCamera-BigSaydon-final-product-build.log`, receipt
`out/BuildPipeline/runs/20260906T144205124Z-debug-product.json`. Collider 수정 자체에는 새
publisher와 resource 배포가 필요 없다. Client/UI 실행·캡처와 visual PASS는 미수행이며 이전의
3초 Preview 수명 유지 변경을 실제 프레임 정지 원인 규명이나 해결 증거로 사용하지 않는다.

## G09. G2 Big Saydon 사용자 저장 확인과 컴파일 오류 수정

사용자가 2026-09-06 23:28:20 KST에 저장한 원본을 읽고 다음 값을 확인했다.

| 정본 | 저장 값 |
|---|---|
| BossCatalog의 Big Saydon bodyModelPreScale | 0.0692 |
| weaponModelPreScale | 0.0103829 |
| weaponModelPreRotationDegrees | (-96.5, -6.5, 6.5) |
| Gameplay.world의 boss.kakulsaydon.g2.big-saydon position | (10.24, 8.63, 317.75) |
| 같은 placement yawDegrees | 226.5 |

BossCatalog는 Client의 기존 ActorCatalog가 직접 소비한다. World 위치는 생성 bootstrap이
이전 저장본이어서 공식 `Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish`를 실행했다.
`out/ArenaCamera-BigSaydon-world-publish.log` exit0이며 생성된
`Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap`의 실제 placement 행에서도
10.24/8.63/317.75/226.5를 확인했다. 원본 `Data/Actors/BossCatalog.json`과
`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`은 작업 시작 백업과 바이트가 같다.

Server의 Build_WorldEntity가 모든 boss Y를 navigation 지면으로 덮어쓰는 추가 문제를 발견했다.
KAKULSAYDON_ARENA의 BOSS_KAKULSAYDON_G2_BIG_SAYDON만 저장 Y를 유지하도록 수정했고
XZ walkable 검사·필요한 투영은 그대로 수행한다. fSpawnPositionY에도 같은 높이가 남아 패턴의
spawn 복귀가 이 값을 사용한다. 다른 world/archetype은 기존 navigation 높이를 유지한다.
Client local transform 우회나 Shared protocol 변경은 추가하지 않았다.

기존 ServerGameplayContractTests에 실제 entity 생성과 90 fixed tick 동안 Y8.63 유지,
spawn 높이 보존, 일반 세이튼의 navigation 높이 유지를 검사하는 4개 검증을 추가했다.
최종 Product로 만든 Server의 `--contract-test` 결과는 **1,187 PASS / failures 0**, exit0이며
`out/ArenaCamera-BigSaydon-server-contract.log`에 기록했다. listener나 Client를 실행하지 않았다.

같이 보고된 `)` 구문 오류는 ArenaCameraProfile.cpp:39의 `std::numeric_limits<f32_t>::max()`와
Windows `max` 매크로 충돌이었다. 괄호로 함수 이름을 감싸 수정했다. 이전 Temp 최소 컴파일의
`/DNOMINMAX` 옵션 때문에 놓친 오류이며, 실제 Client 프로젝트 빌드로 해결을 검증했다.
최종 표준 Debug Product는 Engine/Shared/Server/Client 전부 PASS, exit0이고
SDK/shader/DLL 배포와 runtime 입력 확인도 완료했다. 로그와 receipt는 G08-2의 최종 빌드와 같다.

최종 변경/신규 JSON43개와 project/filter XML4개 parse, `git diff --check`를 통과했다.
Valtan Level H/CPP 및 ValtanCinematicCamera JSON은 카메라 작업 시작 백업과 바이트가 같다.
사용자가 종료한 Client/Server는 에이전트가 재실행하지 않았다. LAN server-host인 이 PC에서
Visual Studio `Server + Client` profile의 Ctrl+F5로 직접 실행하고 Big Saydon 크기·무기 각도·높이,
카메라 조절과 Collider Preview/Sequencer Play를 확인한다. 해당 화면의 manual/visual PASS는 미수행이다.
