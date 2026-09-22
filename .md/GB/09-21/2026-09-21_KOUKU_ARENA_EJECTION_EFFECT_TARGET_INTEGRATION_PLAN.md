# 2026-09-21 쿠크 아레나 공중 이탈·효과·조준 통합 계획

## G00. 목적

사용자 저장본 Composition을 기준으로 Gate2 쿠크/대형 세이튼의 밀려남을 nav 경로가
가로채지 않는 ballistic push와 FALLING/dead-zone 수명주기로 연결한다. 같은 변경에서
대형 세이튼 바람·노란 시선 effect, 쿠크 중앙 이동 yaw, 레이저 target tracking을 stable
저작 ID와 Server authority 경계에 맞춰 publish한다.

## G01. 적용 파일

- `Server/Private/GameRoom_PlayerSimulation.cpp`: ballistic gap 이후 FALLING 전환과 상부
  nav deck 착지 방지.
- `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`: pushBallistic 결과,
  P99 wind/collider, P21 tracking, P25 yaw.
- `Data/Effects/Authored/*bigsaydon*full.restore.effect.json`와 effect catalog/tree:
  두 원본 후보의 직접 authored 등록.
- `Client/Default/Client.vcxproj(.filters)`: 두 Data effect 문서 등록.
- generated Encounter/bootstrap: composition and gameplay publishers 출력.

## G02. 불변식

- Server가 hit, push, target, floor/dead-zone을 소유한다. Client collider/debug는 판정 권위가
  아니다.
- ENTER_AREA는 플레이어 좌표 overlap을 계속 검사하고 BOSS_TRACK_TARGET은 보스 방향만
  Server target에 맞춘다. 전역 Gate2 모델 yaw 보정은 반전하지 않는다.
- `logic.103` grounded teleport은 yaw를 만들지 않으므로 P25의 explicit reset yaw만 바꾼다.
- 1관문 무력화와 파1빨2 effect/데이터는 보존한다. 사용자 저장 partial P11 occurrence도
  stable ID로 보존한다.
- Gate3 상부 nav는 authored entry terrace이므로 BLOCKED mass edit을 하지 않는다.

## G03. 검증 순서

1. 저장본 hash/revision을 재확인하고 stable field 단위로 Composition을 병합한다.
2. Composition JSON parse와 `project_kouku_saydon_composition.py --mode publish/validate`를
   실행한다.
3. `Publish-GameplayBalance.ps1 -Mode Publish`로 Encounter/bootstrap을 다시 생성한다.
4. 대상 bootstrap rows, effect catalog/tree, project/filter 등록을 확인한다.
5. 변경 JSON 전체 parse, `git diff --check`, Server Product Build를 실행한다.
6. 사용자 EXE 종료 후 사용자가 Debug Build와 실제 Client 아레나 화면을 확인한다.
