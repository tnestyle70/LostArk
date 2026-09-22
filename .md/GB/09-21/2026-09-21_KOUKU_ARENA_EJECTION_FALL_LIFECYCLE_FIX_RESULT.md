# 쿠크 아레나 외곽 이탈 낙사 수명 수정 결과

## G00. 완료한 서버 수정

`Begin_PlayerFall`이 FALLING 전환 시 `bArenaEjectionActive`, `bKnockbackBallistic`,
`bKnockbackCanLeaveArena`, ejection owner ID와 공중 속도/발사 높이를 초기화하도록 수정했다.
외곽 이탈과 포물선 gap은 기존 FALLING 적분과 `iFallDeathTick` 경로로 넘어간다.
`Advance_PlayerKnockback`은 발사 높이보다 높은 겹친 nav deck을 착지면으로 선택하지 않도록
막아 Gate3 상부 대기 테라스로 순간 스냅하지 않게 한다.

## G01. 패턴별 Server 입력

- 대형 세이튼 조커 찾기 `KAKULSAYDON_G1_PATTERN_13`: 16m/1500ms, `pushBallistic=true`.
- 쿠크 레이저 `..._21`: 6m/242ms, 각 hit window가 포물선 push를 사용하고
  `BOSS_TRACK_TARGET` 0~6532ms window가 보스 방향을 Server 선택 생존자에게 갱신한다.
- 쿠크 팡파레 `..._23`: 6m/242ms ballistic.
- 쿠크 휠윈드 `..._24`: 2m/242ms ballistic, `forcePush`와 arena 이탈 허용.
- 쿠크 슈퍼바주카 `..._85`: 6m/242ms ballistic.
- 대형 세이튼 두 번·세 번 내려치기 `..._86`/`..._87`: 30m/1500ms ballistic.
- 대형 세이튼 불어날리기 `..._99`: `4221804` 3단계 5735ms, 3534ms contact window에서
  16m/1500ms high ballistic 결과와 전방 sector collider를 사용한다.

## G02. 중앙 이동과 레이저 방향

G2 쿠크 spawn yaw는 216.5도이고 P25의 기존 `resetBossYawDegrees=306.5`가 시계 방향
90도를 만들고 있었다. `KAKULSAYDON_G1_PATTERN_25.resetBossYawDegrees`를 126.5도로
바꿔 반시계 90도로 맞췄다. `logic.103` grounded teleport 자체는 위치와 지면만 갱신하고
yaw를 바꾸지 않으므로 수정하지 않았다.

레이저의 기존 ENTER_AREA 판정은 매 tick 모든 플레이어 좌표와 collider overlap을 검사한다.
다만 시각 레이저의 방향은 시작 시 `RETARGET_RANDOM_ALIVE`가 고른 한 명을 향해 고정됐다.
P21에 `kakulsaydon.g1.logic.65` 추적 window를 추가해 같은 Server 권위 target을 cast 동안
갱신하도록 연결했다.

## G03. 효과와 게시 상태

사용자 범위대로 1관문 무력화와 파1빨2 광선은 건드리지 않고, 다음 두 full restore만 설치했다.

- `effect.kouku.gate2.bigsaydon.wind.full.restore`
- `effect.kouku.gate2.bigsaydon.yellow-gaze.full.restore`

두 문서는 `Data/Effects/Authored`, `EffectCatalog`, `EffectResourceTree`, Client project/filter에
등록했고 최초 설치 때 Composition의 stable resource/occurrence로 연결했다. P11의 기존 저장 partial
occurrences는 보존했다. P99의 잘못된 MAP rectangle은 바람 effect로 교체하고 새 collider는
source hit geometry 입력이 없던 경계에서 6m/35도 project-tuned sector로 명시했다.

09-22 현재 revision 2166 및 PR #440 feature/병합/main 재확인 결과, 노란 시선 full restore의
P11 `.presentation.34`는 이미 없고 해당 resource의 Pattern occurrence 참조도 0개다.
정의·Catalog/Tree 등록이 남아 있는 상태이며 충돌 해결로 재생 연결이 되살아난 것은 아니다.
자세한 현재 상태는 같은 날짜의 `KOUKU_SAYDON_SOURCE_EFFECT_FULL_RESTORE_RESULT.md`
첫 절을 따른다. 아래 revision 2043 게시 수치는 최초 설치 당시 증거다.

Composition revision 2043을 publish/validate했고, generated `Data/Encounters`와
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`도 같은 revision으로 갱신했다.
P99까지 Product sequence에 포함되며, 저장 inventory에서 남은 unavailable rows는 기존
불완전 패턴 3/20/45/51뿐이다.

## G04. 검증 경계

- Composition publish: 통과 (`productPatternCount=89`, `productStageCount=487`).
- Composition validate: 통과.
- Gameplay balance publish: 통과.
- 변경 JSON parse와 `git diff --check`: 최종 점검 예정.
- Server targeted compile은 이전 단계에서 통과했으며, 링크는 실행 중 Server.exe 잠금으로 막혔다.
  사용자가 EXE를 종료한 뒤 Debug Product Build를 실행하면 된다.
- Client 실행·UI 조작·실제 아레나 화면 판정은 수행하지 않았다.
- Gate3 상부 navigation authoring은 잘못된 overhead가 아니라 입장 대기 테라스라는 근거가
  확인되어 nav 데이터를 변경하지 않았다. 전투장 입장은 기존 typed gate-entry 흐름이 담당한다.
