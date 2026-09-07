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
