# 발탄 패턴 마무리 구현 결과

## 구현 상태

- `VALTAN_SILENCE_SLOT`은 사자후 종료 뒤 별도 5초 hold stage에서 Server silence를 적용하고,
  Client HUD는 R 슬롯 하나에만 기존 아이콘을 유지한 채 상태 표시를 한다.
- `VALTAN_BIND_SLOT`은 랜덤 생존자를 Server 권위로 공중에 고정하고 정확히 5초 동안 이동,
  스킬과 Esther 입력을 막은 뒤 원래 navigation-valid 위치로 복구한다.
- 마력구 channel은 발탄을 기준 Y에서 +5m 올리고 확정 HP damage만 1000까지 누적한다.
  성공은 별도 `VALTAN_GROGGY_FOLLOWUP`으로 전환하며, 실패는 마지막 공격 contact frame에서
  wipe를 한 번 발생시킨다. 모든 종료 경로가 기준 Y를 복구한다.
- 3연속 counter는 전방 180도 `BOSS_FORWARD_ARC` 판정을 사용한다. 세 구간 중 최초 성공은
  공용 groggy 후속 패턴으로 전환하고, 끝까지 실패하면 세 번째 공격이 wipe를 발생시킨다.
- phase 3 primary 발탄은 같은 NetEntityId, HP와 damage authority를 유지한 채 유령 표현으로
  전환한다. 저장된 여섯 패턴 사이에 한 tick 숨김/무적과 deterministic random 재배치를 거친다.
- 네 portal combat object는 0→90→180→270→0의 사각형 네 변을 동시에 이동하며 5초 간격으로
  반복한다.
- 기존 피자 패턴의 회전과 후속 decal 계약은 변경하지 않았다.

## Effect authoring 통합

- Valtan Effect V2 bindings와 여덟 group을 strict formatVersion 2 문서로 migration했다.
- `CEffectResourceCatalog` facade가 V1과 V2 resource를 같은 목록 identity로 노출한다.
- MainApp의 Effect Tool 진입을 하나로 합치고 기존 renderer/backend는 유지해 migration 중인
  V1과 typed V2를 모두 조회·append·preview할 수 있게 했다.
- presentation generation admission은 BOSS_VALTAN binding에서 실제 도달 가능한 group/leaf만
  transactional closure로 고정한다.

## 데이터와 팀 pull 계약

- gameplay bootstrap format version을 Shared 정본으로 이동해 Server publisher와 Client
  presentation admission이 같은 세대를 사용한다.
- `nextPatternId`, vertical offset, accumulated health damage outcome과 전방 arc를 authoring,
  Product projection, Server catalog/runtime, Client reader/debug mirror까지 연결했다.
- `.md/TEAM/VALTAN_PUBLISH_PULL_BUILD_GUIDE.md`에 작성자 publish 순서와 다른 PC의 clean pull/build
  순서, 대표 오류의 실제 소유자를 기록했다.
- Kakul #292는 먼저 `main`에 merge하고 이 branch가 해당 revision을 다시 병합한 뒤 Product를
  검증한다.

## 자동 검증

- Gameplay publisher Publish: PASS (Valtan 66 patterns, 279 stages, 52 timeline rows)
- Server x64 Debug build/link: PASS
- `Server.exe --contract-test`: PASS, failures 0
- Effect Tool V2 Python suite: PASS, 76 tests
- Effect V2 repository/resource validator: PASS (110 authored, 111 bindings, 8 groups,
  5 independent, 70 textures)
- Valtan presentation generation Python suite: PASS, 9 tests
- PatternTree focused contracts: PASS, 28 tests
- Valtan canonical graph: PASS, 66 patterns / 279 stages
- Valtan Pattern Audition 전체 실행형 harness: PASS (canonical 4/4,
  Action Composition 10/10, Effect cue 11/11, presentation admission PASS)
- Client x64 Debug 전체 build/link: PASS, compile errors 0
- `git diff --check`: PASS
- Kakul #292 통합 계약: PASS (Client product 8/8, world admission 7/7)
- Debug Product: PASS
- Debug Core: PASS (Network failures 0, Valtan admission PASS, Character Select live
  isolation failures 0)

## Post-merge pull 회귀 수정

- Core가 설정하는 `LOSTARK_RESOURCE_ROOT`가 Effect V2 임시 fixture를 가리키지 않도록 해당
  테스트가 자신의 임시 Resources root를 명시한다. 환경값 설정/해제 양쪽 focused test와
  전체 Debug Core가 통과했다.
- Kakul source map header의 실제 수량 `318 assets / 2971 placements`에 맞춰
  `Data/Maps/MapCatalog.json`의 집계값을 교정했다. live `server-product-level` admission은
  Resource Collection, Development Geometry Preview, Server Product Level을 모두 허용한다.

## 수동 검증 경계

Client 화면과 Effect visual fidelity는 자동 PASS로 기록하지 않는다. 사용자는 merge된 main에서
Silence R 표시, Bind 5초 고정, 마력구 999/1000 damage 분기, 세 counter 구간, 유령 재배치와
5초 사각 portal을 직접 확인해야 한다.
