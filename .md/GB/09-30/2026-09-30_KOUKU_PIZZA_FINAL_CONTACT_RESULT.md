# 쿠크 피자 마지막 내려찍기 고정 판정 결과

## G00. 확인한 현재 연결

`KAKULSAYDON_G1_PATTERN_26`(대형세이튼_피자)의 마지막 clip은 원본 action4221819
stage007 `mn_rpct_06_sk.ao_att_battle_8_03`2600ms이며, 현재 저작 시계에서15000ms에 시작한다.
현재 충격 Effect 시작16962ms, 사운드16966ms, 기존 판정 시작16997ms를 확인했다.
피해·넉백 연결 자체가 전혀 없는 상태는 아니었다. P86(두번내려치기)의 collider.283과
Logic507(KNOCKBACK)→522를 재사용하지만 P26도 WEAPON/b_rpct_01을 따라가고 있었다.

기존 게시 bone track의 첫 위치를 현재 boss spawn[10.2399998,10,317.75]와 yaw226.5로 계산하면
[4.04345946,9.71257928,322.95778150]이다. 노란 예고의 저장 중심과 수평0.329m 차이다.
이 값만으로 기존 피해 누락 전체의 원인이 무기 본이었다고 단정하지 않는다. 사용자가 요청한
고정 위치와 시각을 명확히 일치시키는 데이터 교정이다.

노란 원은 MAP 중심[4.340000152587891,10,323.1000061035156],15577ms 시작2066ms 길이여서
17643ms까지 남았다. 기존16997~17097ms 타격 창보다 예고가546~646ms 더 늦게 끝났다.

## G01. 반영한 변경

Composition source revision2496→2497이다. P26.presentation.7의 위치를 노란 MAP 원과 동일하게
저장하고 anchorKind=MAP,followBoss=false,bone="",boneTarget=BODY로 변경했다.
기존 source collider resource·16997ms 시작·100ms 창·scale·피해/넉백값은 그대로다.
projector 결과는 WORLD 고정 cylinder이며 center는 저장한 맵 좌표, radius4.899999916553497m,
half-height1.25m이고 bone worldTrack이 없다. 보스·무기 본의 이후 이동에 판정이 끌려가지 않는다.

P26.presentation.2의 노란 예고 길이만2066→1420ms로 줄여16997ms 타격 시작에 끝나게 했다.
기존 타격 Effect와 사운드의 시각·배치·크기·색은 보존했다. 공통 Logic507→522는 P86과 같으며
최대HP10%,10m/1500ms,AWAY_FROM_CONTACT,forcePush,pushCanLeaveArena,pushBallistic을 유지한다.
중복 Collider를 새로 추가하지 않아 한 번의 공격에서 피해를 두 번 발생시키지 않는다.

새 C++/schema/프로젝트 등록·렌더러는 없다. 두번내려치기와 다른 모든 Pattern·Flow·resource·Logic,
Rendering/FXAA 옵션은 변경하지 않았다. 기존 MAP Collider→WORLD region→ENTER_AREA
→MAX_HP_PERCENT_DAMAGE/강제넉백의 Server 권위 경로를 그대로 사용한다.

## G02. 반영 절차와 검증

최신 저장본을 writer lock으로 보호하고 두 stable occurrence와 revision만 byte 범위로 교정했다.
교체 직전 freshness, ReplaceFileW 백업·원자 교체, 반영 뒤 bytes 검사를 수행했고 rollback은 없었다.
변경한 두 occurrence와 revision을 원래 값으로 되돌린 구조는 이전 JSON과 완전히 같았다.

- JSON parse와 변경 파일 diff check PASS.
- 실제 projector `_project_collider_regions`로 WORLD 좌표·반경·높이·worldTrack 부재를 확인했다.
- 기존 두번내려치기와 trigger/result stable ID가 같고 모든 피해·넉백 수치가 같은 것을 확인했다.
- 노란 예고 끝과 Collider/Logic 시작16997ms가 같으며 Collider와 Logic 끝17097ms도 일치한다.
- 공식 Kouku projector 게시 결과는 다음 항목에 기록한다. Gameplay 게시와 제품 빌드는 통합 담당이 한다.
- 에이전트는 새 Server 실전 입력·실제 피해·넉백 재생·Client/UI 화면을 실행하지 않았다. 구조 검사를 실제 플레이
  성공으로 확대하지 않는다. 같은 binary를 재사용하는 데이터 변경이며 최종 게시 후 Server 재시작과
  Client의 최신 게시 데이터 사용이 필요하다.

증거는 `out/KoukuPizzaFinal20260930`의 candidate-baseline.json, candidate-validation.json,
install-receipt.json, source-action-4221819.json, 기존 게시 pattern snapshot과 projector-publish.log다.

## G03. 전달

Workbench에서 `쿠크피자_묶음`의 `대형세이튼_피자`를 열면 기존 Collider 박스를 그대로 편집한다.
해당 박스의 MAP 위치와16.997초 시작·0.1초 길이를 조정할 수 있다. 공유 resource/Logic을 변경하면
두번내려치기도 함께 영향을 받으므로 이번 수정은 P26 occurrence에 한정했다.


공식 projector `--mode publish`는 exit0으로 완료했다. sourceRevision2497,
Product114 Pattern/573 Stage/9 Bundle, 저장121 Pattern/11 Bundle, output2다.
게시된 Encounter의 실제 P26 region도 WORLD·동일 MAP 중심·worldTrack 없음,
16997ms/100ms·최대HP10%·넉백10m임을 다시 확인했다. GameplayPublish는 이 완료 직후
통합 담당에게 넘겼으며 하위 작업에서 동시에 실행하지 않았다.

## G04. 사용자 화면 확인

2026-09-30 사용자가 “쿠크 피자 쪽 콜라이더 깔끔하게 들어간 거 확인”이라고 확인했다.
쿠크 피자 마지막 공격의 콜라이더 반영은 사용자 실제 화면 확인까지 완료했다.
