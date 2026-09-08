# 2026-09-07 쿠크 Complete Play revision 및 맵 스포트라이트 확인 결과

현재 Product revision은 160이며 최신 결과는 아래 G04를 따른다. 앞선 각도·암전·World Object 수정 시점의 Product revision은 85였다. 아래 82는 최초 오류 복구 시점이며 최종 상태와 빌드·배포 증거는 같은 폴더의 `2026-09-07_KOUKU_PATTERN_STAGING_AND_DARK_SCENE_RESULT.md`를 따른다.

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
