# 2026-09-07 쿠크 Complete Play revision 및 맵 스포트라이트 반영

## G00. 현재 저장 상태와 목표

F1 Saved Patterns의 Complete Play가 `KoukuSaydon audition expected Product source revision is not active`로 거절된다. 현재 Composition revision은 82, 생성된 encounter/patternbindings는 81, Server Gameplay.bootstrap의 KOUKUSAYDONPRODUCTREVISION은 77이다. Server의 exact revision 검사는 이 차이를 거절하며 그대로 유지한다.

`KAKULSAYDON_G1_PATTERN_9`(대형세이튼_세이튼등장)는 MN_RPCT_06의 빈 DRAFT다. 기존 Product 6개에 추가하지 않고 사용자의 저작 상태를 보존한다. 새로운 2관문 패턴의 stage 구현은 이번 복구 범위가 아니다.

## G01. 저장 정본과 반영 경로

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`을 수정하지 않고 명시 KoukuSaydon domain publisher로 encounter, patternbindings, Server gameplay bootstrap을 동일 revision으로 생성한다. publisher의 revision 비교와 실패 시 기존 생성물 보존을 사용한다.

`월드_1관문스포트라이트`는 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json`에 `light.LV_LUT_MIDNIGHTC_ED.1`로 저장됐다. enabled=true인 맵 고정 SPOT이며 runtime maplights에도 같은 값이 있다. 기존 Map publisher로 저장 구조와 runtime 일치를 확인한다. 패턴 LIGHT occurrence로 중복 추가하지 않는다.

기존 dirty Composition, maplights, worldsequences 및 LightResources runtime을 보존한다. C++와 프로젝트/filter 등록 변경은 제안하지 않는다.

## G02. 실행 검증과 사용자 재생

- 기존 map-light validator와 Map publisher의 Check로 저장 값과 배포 일치를 확인한다.
- `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 82`로 두 Product와 Server gameplay 데이터를 생성한다.
- 기존 Composition projector validate와 JSON parse, source/Product/bootstrap revision 및 Product 목록 일치, `git diff --check`를 확인한다.
- C++ 변경이 없으므로 컴파일은 불필요하다. 실행 중인 Server는 이전 catalog를 소유하므로 새 데이터의 활성화는 Server 재시작 뒤에만 완료된다.
- Client/UI는 실행·조작하지 않는다. 사용자가 Server + Client를 재시작하고 Lobby → KoukuSaydon → F1 → KoukuSaydon Arena에서 1관문 보스를 올린 뒤 Saved Patterns → Reload KoukuSaydon Inventory → Complete Play를 확인한다. 스포트라이트 밝기와 실제 보스 재생 판정은 사용자 확인으로 남긴다.


## G04. PRODUCT 버튼·Save 배포·F1 Complete Play 연결 (2026-09-08)

사용자 저장159의 P12/P13/P14는 PRODUCT이나 조커찾기 Bundle3은 P13 편집 때 DRAFT로 내려갔고,
배포 Encounter/patternbindings/bootstrap은149다. Publish_Product의 CreateProcessW는 상대
application name powershell.exe를 resolve하지 못해 Win32 error2를 반환하며 로그도0byte다.
Windows system directory에서 Windows PowerShell 절대경로를 resolve해 기존 publisher를 실행하고
실패 원문을 표시한다. PRODUCT 버튼은 저장과 배포 요청을 연결하되 실제 성공은 background 종료
결과로 구분한다. 포함된 DRAFT Bundle은 자동 승격하지 않고 그 이름을 알린다.

F1은 inventory에서 고른 stable ID를 복사한 뒤 최신 Product를 재조회한다. Workbench의 source
revision을 검증하는 기존 expected revision API는 유지하고, F1/Selected/All의 cached revision
때문에 첫 재생이 거절되거나 이전 데이터를 제출하는 경로를 정리한다. Workbench dirty/publish
진행 중/배포 revision 불일치를 확인한 뒤 재생하며, 성공한 publish는 F1 inventory refresh로
연결한다. Server 활성 revision 검사를 우회하지 않으며 Server 재시작은 여전히 필요하다.

최신 source159는 사용자 변경 Collider6개와 삭제 상태를 보존하고, P13.presentation.9의 지원되지
않는 Z회전 -0.25도만0도로 정규화한다. 조커찾기 Bundle3을 PRODUCT로 설정하고 revision160으로
CAS 갱신한다. 그 외 위치·크기·Yaw·Logic·playAll은 동일해야 한다. 지면 gameplay Collider Detail은
Yaw만 편집하게 해 같은 publisher 거절을 예방한다. 기존 domain publisher로160을 배포하며
생성물은 직접 편집하지 않는다.

기존 native Workbench 회귀, publisher 실제 실행 및 projection 참조 검증, 변경 Client 컴파일과
Product 빌드·JSON/XML parse·scoped diff check를 수행한다. 신규 C++/project/filter/schema는 없다.
Client/UI와 실제 조커 성공 분기·화면 결과는 사용자가 직접 확인한다.
