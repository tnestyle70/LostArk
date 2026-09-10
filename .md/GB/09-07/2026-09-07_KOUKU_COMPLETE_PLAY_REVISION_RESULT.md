# 2026-09-07 쿠크 Complete Play revision 및 맵 스포트라이트 확인 결과

최신 게시본은 **source revision 243**이며 현재 구현·검증 결과는 G05/G06을 따른다. 앞선 G00~G04의 revision과 재시작 안내는 당시 작업 기록이다.

## G00. 원인과 실제 반영

`GameRoom.cpp`의 Product source revision 검사는 Client 요청과 활성 Server catalog가 다를 때 boss/pattern 검사 전에 거절한다. 조사 당시 Composition은 82, Client encounter/patternbindings는 81, Server bootstrap은 77이었다. 재생 오류는 이 불일치에서 발생한다.

명시 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 82` 실행으로 Composition 정본, encounter Product, patternbindings Product와 Server bootstrap의 쿠크 revision을 모두 **82**로 맞췄다. Product는 기존 **6 patterns / 66 stages**다. 전체 gameplay bootstrap도 해당 publisher를 통해 갱신했다. bootstrap을 직접 편집하지 않았다.

새 `KAKULSAYDON_G1_PATTERN_9`(대형세이튼_세이튼등장)는 MN_RPCT_06의 빈 DRAFT다. 기존 `PATTERN_8`도 DRAFT이며 이 두 항목은 이번 Product 재생 대상에 들어가지 않는다. 이를 삭제하거나 임의로 PRODUCT로 승격하지 않았다. 신규 2관문 패턴이 기존 Product의 구조 검증을 깨뜨린 상태는 아니다.

작업 브랜치는 `codex/kouku-complete-play-revision`이다. 시작 브랜치는 `koukusaydon-arena-light-pattern2`, HEAD는 `13fa34d7df9f37d1c697ddefe1e5e7aebfdafef9`였다. 사용자 저작 파일과 기존 미커밋 변경을 보존했고 자동 stage/commit/push는 하지 않았다. C++ 변경은 없다.

## G01. 월드_1관문스포트라이트 저장 확인

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json`, 런타임은 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.maplights.json`이다. 두 문서의 파싱 결과가 같다.

| 저장 항목 | 현재 값 |
|---|---|
| stable ID / 이름 | `light.LV_LUT_MIDNIGHTC_ED.1` / `월드_1관문스포트라이트` |
| 종류 / 활성화 | SPOT / true |
| 위치 | 약 (-0.17, 35.47, 943.03)m |
| 회전 | (90, 0, 0)도 |
| 범위 / 감쇠 | 약 76.2m / exponent 2 |
| inner / outer cone | 12 / 26도 |
| 색 / 밝기 | 흰색 / 4 |

1관문 세이튼의 저작 spawn은 (-0.07, 1.32, 942.33)m이며 저장 조명은 그 위에 있다. enabled 맵 배치이므로 아레나에 유지되는 조명이다. 패턴 lifetime의 LIGHT box로 중복 추가하지 않았다. 실제 조명 폭과 밝기는 사용자 화면 판정 대상이다.

## G02. 실행한 자동 검증

- Map light v2 기존 validator: PASS. source/runtime JSON semantic equality PASS.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Check`: PASS, 3,231 placements / runtime 7 files. 사용자가 저장·배포한 maplights와 worldsequences를 포함해 현재 runtime이 정본과 일치한다.
- KoukuSaydon Product 및 Gameplay domain Publish: PASS. 로그 `out/KoukuCompletePlayRevision/publish.log`.
- 기존 Composition projector `--mode validate`: PASS, sourceRevision 82 / 6 patterns / 66 stages / 2 outputs.
- Server bootstrap `KOUKUSAYDONPRODUCTREVISION` 82 확인. Product 2개와 정본 revision 일치.
- Composition, maplights, worldsequences의 publisher 실행 전후 원본 bytes 동일: 3/3. 보존 근거 `out/KoukuCompletePlayRevision/source-before.json`.
- patternbindings의 기존 HEAD 대비 semantic 변화는 sourceRevision 81 → 82뿐이다. 사용자 animation/Effect/조명 값을 임의로 변경하지 않았다.
- JSON parse와 `git diff --check`: PASS. C++/XML 변경이 없으므로 컴파일과 XML 검증 대상은 없다.

Gameplay publisher의 기존 일부 class hit-shape coverage 경고는 남았으며 Publish는 성공했다. 새 광역 하네스나 검증 체계를 추가·실행하지 않았다.

## G03. 사용자 실행과 남은 확인

팀 LAN 동기화 결과는 server-host, TCP 7777 LocalSubnet 규칙 정상이다. 마지막 프로세스 조회에서는 Server.exe와 Client.exe가 모두 종료돼 있었다. 새 데이터의 Server 활성화와 실제 Complete Play 화면 결과는 아직 사용자 확인 전이다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다.

1. Visual Studio의 Debug `Server + Client` profile을 `Ctrl+F5`로 시작한다.
2. Lobby → KoukuSaydon → F1 → KoukuSaydon Arena에서 1관문 보스를 올린다.
3. KoukuSaydon Complete Play → Load/Reload KoukuSaydon Inventory → Saved Patterns에서 기존 PRODUCT 패턴을 선택 → Complete Play.
4. 보스 모션과 `월드_1관문스포트라이트`를 확인한다. 새 2관문 DRAFT는 stage 저작과 PRODUCT 전환 전까지 이 재생 목록의 대상이 아니다.

이후 Composition 저장을 재생에 반영하려면 Publish All PRODUCT와 Server 재시작이 필요하다. 현재 runtime hot reload가 없는 계약을 유지했다.


## G04. 최신 조커찾기 PRODUCT와 F1 연결 복구 (2026-09-08)

저장본159와 배포본149 불일치를 확인했다. Workbench publisher의 bare powershell.exe
CreateProcessW는 실제 Win32 error2로 실패했다. 정본 Windows PowerShell 절대경로는 성공했으며,
현재 UI는 그 경로로 publisher를 실행하고 프로세스 생성 실패도 로그에 기록한다. PRODUCT 버튼은
검증 뒤 Save·publish 요청까지 연결하고 성공한 background 배포만 F1 inventory를 갱신한다.
F1 Selected/Bundle/All은 최신 Product를 재조회하고 미저장·배포 중·revision 불일치를 거절한다.
명시 source revision 검증과 Server 권위 재생·재시작 경계는 유지했다.

사용자 source159를 backup하고 CAS로160을 만들었다. 변경은 revision, Bundle3 DRAFT→PRODUCT,
P13.presentation.9의 지원되지 않는 Z회전 -0.25→0도뿐이다. Collider6개의 위치·크기·Yaw와 삭제한
3개, Logic·stage·playAll은 deep diff로 보존했다. Yaw 전용 gameplay Collider UI와 명시 X/Z 오류로
재발을 막는다. P12/P13/P14와 Bundle3, 룰렛P7은 PRODUCT다. 댄스타임P6은 사용자 source159부터
DRAFT이므로 유지했다. 이21stage가 이전11patterns/88stages와 현재10patterns/67stages 차이다.

Invoke-BuildDomainOwner -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision160 PASS다.
Composition160, Encounter160, patternbindings160, Server bootstrap160이 일치한다. projection
validate PASS, 실제 bootstrap의 P13 Collider6개, Bundle3 P12/P13, SUCCESS→P14 FOLLOWUP을
확인했다. headless Server Bundle38/Object overlap38은 failures0·exit0다. 로그는
out/KoukuJoker20260908/product160-publish.log 및 server-*-contract-rev160-20260908-161022.log다.

기존 native focused 회귀는 실제 Workbench Save→hidden PowerShell child→Owner/revision 출력→
Poll 성공 refresh1회, 실패 refresh없음과 gameplay Collider roll Save 거절·원본 보존까지 PASS다.
product-save-launch-build.log / product-save-launch-contract.log가 증거다. 최종 Debug Product
Engine/Shared/Server/Client compile·link·runtime 배포 exit0, receipt `out/BuildPipeline/runs/20260908T071602366Z-debug-product.json`이다.
JSON/XML parse와 scoped git diff --check를 확인했다. 큰 dirty checkout의 무관한 변경과 Resources는
보존했고 자동 stage/commit/push는 하지 않았다.

Server/Client를 새로 시작한 뒤 F1 → KoukuSaydon Complete Play → 조커찾기_동시 → Complete Play로
검증한다. 선택 Pattern은 그 보스 하나, Bundle은 P12/P13 전체를 실행하며 조커 성공은 P14를 따른다.
에이전트는 Client/UI를 실행·조작하지 않았다. 망치·카드 반응과 V2 live P/R/S 화면은 사용자 검증
대상이다. 포커판/중앙링 Map catalog 파서는 앞선 최종 EXE 수정 상태를 유지하며 이번에 Map 입력을
다시 변경하지 않았다.


## G05. 게시 revision admission·공 재질·파1빨2 공포 (2026-09-10)

구현 및 자동 검증을 마쳤다. 브랜치는 codex/kouku-publish-live-update, 시작 HEAD는397aab6c다.
사용자가 마지막 저장·EXE 종료를 알린 뒤 source238을 보존했고 logic36.insideOutcome=FAIL을
추가했다. 이후 휠윈드 설정과 별도 작업에서 이미 반영한 P21/P23 World 정의를 함께 보존해243을 게시했다.
자동 stage/commit/push는 하지 않았다. Resources payload는 이 작업에서 바꾸지 않았다.

Server는 다음 Complete Play 요청의 정확한 published source를 기존 CGameplayCatalog parser로
검증하고 승인된 run이 immutable catalog를 소유한다. 진행 중 run의 Stop/Restart는 원래 pin을
사용하며 새 Complete Play만 새 게시본을 사용한다. 공통 gameplay hash와 쿠크 외 balance는 유지한다.
게시 중 lock, 부정확한 source, 손상, rollback, 다른 balance 변경은 거부하고 이전 상태를 보존한다.
Client는 Server가 승인한 epoch/source에서 presentation을 바꾸며 busy 거절 전에 기존 표현을 지우지 않는다.

파1빨2의 real Saydon이 시야 안이면 Fail→FEAR3000ms, 밖이면 Success다. 기존 진짜 세이튼
찾기의 기본 inside=SUCCESS는 유지한다. 공의 명시 diffuse override 경로는 공유 shader의 이전
character material program/dye 상태를 초기화한다. colorful DDS와 CModel/CMaterial 경로는 그대로다.

## G06. Trigger 연결과 휠윈드 반복 넉백

Resources와 Collider Detail이 공유하던 pending Logic buffer를 분리했다. definition이 실제로
바뀐 commit과 Reload에서만 해당 buffer를 갱신하므로 다른 패널에 의해 선택이 풀리거나 stale 값으로
덮어쓰는 경로를 막는다. Apply/Append-reuse는 선택한 Trigger 값과 Collider의 형태·시간·연결을 함께
반영한다. 첫 연결은 동일 definition의 유일한 미연결 창을 ID 유지한 채 재사용하고 기존 공유 clock을
편집할 때 연결된 Collider 시간을 함께 맞춘다. 실패하면 기존 draft와 저장본을 보존한다.

P24 쿠크_휠윈드는 **Trigger1개 + following Collider1개**다.

| 항목 | 게시243 값 |
|---|---|
| 활성 구간 | 4000~5602ms, 267ms 회전6개 전체 |
| Trigger / Collider | 기존 logic38 / logic.1 / presentation.2, 반경3m BOSS_CURRENT |
| 반복 정책 | repeatAfterKnockback=true, 밀리는 중 추가 피해·넉백 없음 |
| 다음 타격 | 밀기 종료 및242ms 최소 간격 이후에도 내부면 다시 타격, 외부면 없음 |
| Success | 전용 logic39, 최대 HP10%, 쿠크→플레이어2m/242ms |
| 벽·이동 불가 | 발탄과 같은 Server swept collision/navigation 경계 사용 |

기존 rearmOnExit와 기본 one-shot도 보존한다. repeatAfterKnockback PRODUCT는 양수 push를 가진
단일 피해 Success를 요구한다. 공통 Can_ArmPlayerHitReaction을 재사용해 이미 밀리는 중인 피격을
중복 적용하지 않고, 벽에서 일찍 멎어도 설정 간격보다 빠르게 반복하지 않는다.

### 실행한 검증

증거 위치는 `out/KoukuPublishIntegration20260910/`이다.

| 검사 | 결과 / 증거 |
|---|---|
| 표준 Debug Product Engine/Shared/Server/Client compile·link·배포 | PASS, final-contact-product-build.log; receipt out/BuildPipeline/runs/20260910T052159567Z-debug-product.json |
| Kouku domain owner243 publish | PASS, publish243.log (Kouku Product·Map·World·Gameplay) |
| 원본/Encounter/patternbindings/bootstrap243 값 일치 | PASS, published243-contract.json; P24 단일 창·AFTER_KNOCKBACK·10%/2m/242ms 확인 |
| 기존 native preview transport | PASS, final-contact-binding-test.log; Trigger 첫 연결·선택 값·resize·Result·Save/Reload·잘못된 값 rollback·반복 모드 보존 |
| 기존 Server object overlap | failures0, final-contact-server-test.log; 방향·실제2m 이동·밀리는 동안 피해 없음·내부에서 재타격·외부 미타격·벽 최소 간격·window 종료·navigation·gaze/fear |
| 기존 Server bundle contract | failures0, final-contact-bundle-test.log; old200ms→new700ms 실제 tick, publish lock, old Restart/Stop, corrupt/unpublished/rollback/unrelated damage 거부 |
| 최신 contact projector4개 | PASS, contact-projector-tests.log; 두 반복 정책·원본값 검증·기존 bootstrap 행과 보조 행 |
| 앞선 인접 projector 검사 | charge/FEAR/CAPTURE/gaze 및 OBJECT_CONTACT PASS (총8건), inventory6건 PASS |
| JSON/PowerShell parse, git diff --check | 최종 검사 결과는 final-source-checks.log |

최초 Server compile에서 추가 struct field 위치가 기존 aggregate initializer와 충돌했으나 기존
필드 순서를 유지하도록 교정했고 최종 Product 빌드를 통과했다. 기존 C4819 codepage 경고는 남아 있다.

### 실행 준비와 수동 확인

Client/Bin/Debug/Client.exe와 Server/Bin/Debug/Server.exe가 최신 빌드다. 팀 LAN 동기화는
server-host이며192.168.0.14:7777 및 TCP7777 LocalSubnet 설정을 확인했다. 에이전트는 Client/UI를
실행·조작·캡처하지 않았다. 사용자는 Server + Client profile을 Ctrl+F5로 실행하고 Lobby→KoukuSaydon→
F1에서 쿠크_휠윈드를 Complete Play한다. 내부 피격→이동 중 무타격→이동 끝에서 내부면 재피격,
파1빨2에서 바라볼 때 공포, 등장 공 색상은 사용자 화면 확인 항목이다.

이후 Pattern Save→Publish→새 Complete Play는 Server 재시작 없이 다음 source를 승인한다.
현재 active run을 바꾸려면 Stop 후 새 Complete Play를 사용한다. Restart는 시작 당시 source를
반복한다. World placement·쿠크 외 balance 변경과 외부 resource 파일의 전체 immutable pin은
이 Pattern 재생 교체 범위에 포함하지 않는다. 화면 결과는 아직 사용자 확인 전이다.
