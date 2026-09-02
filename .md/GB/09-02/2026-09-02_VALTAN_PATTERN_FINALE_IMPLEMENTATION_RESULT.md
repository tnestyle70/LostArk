# 발탄 패턴 마무리 구현 결과

## 구현 상태

- `VALTAN_SILENCE_SLOT`은 사자후 종료 뒤 별도 5초 hold stage에서 Server silence를 적용하고,
  Client HUD는 R 슬롯 하나에만 기존 아이콘을 유지한 채 상태 표시를 한다.
- `VALTAN_BIND_SLOT`은 랜덤 생존자를 Server 권위로 Y +5m에 고정하고 정확히 5초 동안 이동,
  스킬과 Esther 입력을 막은 뒤 원래 navigation-valid 위치로 복구한다.
- 마력구 channel은 발탄을 기준 Y에서 +3m 올리고 확정 HP damage만 1000까지 누적한다.
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
- All Effects는 `CEffectResourceCatalog` facade가 노출하는 owner-kind/stable-ID로 V1과 V2
  resource를 한 목록에 표시한다. Action Composition Workbench는 기존 V1/V2 picker와
  저장 backend을 분리해 유지한다.
- MainApp의 Effect Tool 진입을 하나로 합치고 기존 renderer/backend는 유지해 migration 중인
  V1과 typed V2를 모두 조회·append·preview할 수 있게 했다.
- presentation generation admission은 BOSS_VALTAN animation binding과 BossCatalog combat-object
  visual owner에서 실제 도달 가능한 group/leaf만 transactional closure로 고정한다.

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

- Gameplay publisher Publish: PASS (Valtan 65 patterns, 279 stages, 52 timeline rows)
- Server x64 Debug build/link: PASS
- `Server.exe --contract-test`: PASS, failures 0
- Effect Tool V2 Python suite: PASS, 76 tests
- Effect V2 repository/resource validator: PASS (110 authored, 111 bindings, 8 groups,
  4 independent, 70 textures)
- Valtan presentation generation Python suite: PASS, 9 tests
- PatternTree focused contracts: PASS, 28 tests
- Valtan canonical graph: PASS, 65 patterns / 279 stages
- Valtan Pattern Audition 전체 실행형 harness: PASS (canonical 6/6,
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
Silence R 표시, Bind Y +5m 5초 고정, 마력구 999/1000 damage 분기, 세 counter 구간, 유령 재배치와
5초 사각 portal을 직접 확인해야 한다.

## 2026-09-02 main 후속 구현

- `VALTAN_DASH_CHARGE_GROGGY` 독립 pattern과 manual audition을 제거했다.
  `VALTAN_DASH_CHARGE`가 `WINDUP -> CHARGE -> GROGGY` 세 stage를 직접 소유하고,
  `WALL_CONTACT/TIMEOUT`은 같은 pattern의 recovery action으로 진행한다. Groggy 중
  `PART_DESTROYED`만 `VALTAN_PART_BREAK`로 전환하며 일반 timeout은 다음 pattern으로 끝난다.
  제거 ID는 projection rollback/cleanup용 retired tombstone에만 남는다.
- counter 조건을 실제로 소유하는 `VALTAN_TRIPLE_COUNTER` 표시명을
  `3연속 내려치기 - 카운터`로 변경했다. 별도 비-counter `VALTAN_THREE`는 유지했다.
- Ground Roar의 사용자 저작 돌+폭발 6-element Effect를 하나의 composition으로 유지하고,
  Server가 boss-relative `radiusM=4.9497475`, 시작각 45도, 90도 간격의 네 root에
  전체 composition을 생성하도록 했다. boss yaw 0도 기준 X/Z 배치는
  `(3.5,3.5)`, `(3.5,-3.5)`, `(-3.5,-3.5)`, `(-3.5,3.5)`다.
- Pattern Flow Save adapter가 합법적인 cross-pattern Counter target을 거부하던 구형
  same-pattern 전용 검사를 교정했다. 첫 candidate apply 중 두 번째 Save가 발생하면 최신
  candidate를 deferred queue에 보존하고 첫 exact terminal 뒤 자동 제출해 두 번째 Restart gate가
  영구히 닫히지 않게 했다.
- Composition Patterns는 첫 진입 시 Balance draft가 dirty여도 draft를 버리지 않고 저장된
  canonical Product를 read-only `STALE_PRESERVED` snapshot으로 표시한다. 창 상단에서 admission,
  status 원문과 `Reload Canonical`을 확인할 수 있다.

## 후속 자동 검증

- Valtan split projection Validate: PASS, managed 42 / encounter 65
- Gameplay publisher Publish: PASS, 65 patterns / 279 stages / 52 timeline rows
- Server x64 Debug build와 `Server.exe --contract-test`: PASS, failures 0
- All Effects + Ground Roar combat-object Effect focused: PASS, 53 tests
- Pattern Flow/Tuning native 회귀: PASS, Audition 30/30, Flow 13/13, Tuning 11/11
- Client 전체 실행과 화면 fidelity는 자동 판정하지 않았다. 최종 Product build와 사용자의
  F1/서버 재생 확인 결과를 아래 수동 경계와 분리한다.

## 2026-09-02 strict join 및 조건 분기 최종 폐쇄

- Boss Tool, All Effects, Composition Patterns는 strict split join이 실패하더라도 같은
  `CValtanCanonicalProductReadAdmission` 안에서 검증된 generated Product를 읽기 전용으로
  stage/commit한다. writer window나 admission 실패에는 원본 파일을 우회해서 열지 않고
  last-good만 유지하며 Save, Restart, Play와 authoring mutation은 차단한다.
- Logic Flow의 Sequence 표시 바로 옆 `COUNTER` 배지는 이름 추측이 아니라 해당 Stage의 실제
  `COUNTER_HIT` branch에서 파생한다. strict graph와 read-only Product fallback 모두 같은 표시
  계약을 사용한다.
- `VALTAN_STAGGER_SLOT`은 발탄을 Y +3m로 올리고 confirmed HP damage 1000 이상이면 별도 Groggy
  follow-up으로 전환한다. 미달이면 2900ms 지점 전멸을 가진 3000ms 마지막 공격 Stage를 끝낸 뒤
  base Y로 복귀한다.
- `VALTAN_SILENCE_SLOT`은 2633ms 사자후 뒤 100ms 비가시 적용 Stage에서 5000ms 침묵 deadline을
  부여하고 occurrence를 닫는다. 다음 queued Pattern은 침묵이 남은 동안 시작하며, 침묵은 이전
  occurrence receipt가 아니라 player deadline에서만 해제된다.
- Catch `defaultNextActionId` drift, invalid Bind event, independent Effect owner 불일치, 중복 visual
  identity, stale Product revision 등 목록 전체가 비던 과거 원인과 재발 방지 절차를
  `.md/GB/gotchas.md`에 기록했다.

## Pattern replay와 Flow replay 분리

- 기존 Boss Verification의 `Restart Saved Pattern (Fresh Arena)`는
  `Restart_SavedFlow(true)`를 호출하는 one-slot Flow alias였다. 따라서 saved slot이 하나일 때
  `Restart Flow`와 wire/runtime/reset 의미가 같았고, Pattern restart처럼 보이는 이름만 달랐다.
  이 alias와 one-slot 전용 overload를 제거했다.
- `Play Selected Pattern (Keep Arena)`는 기존 `PLAY_PATTERN_ID`를 사용한다. Server는 Valtan만
  boss-only reset하고 현재 arena destruction/prop/collision/Nav 상태를 유지한다. 교체되는 boss-source
  combat object만 취소하고 player-source object는 유지한 채 선택 Pattern 하나를 첫 Stage부터 재생한다.
- `Restart Active Pattern (Keep Arena)`는 기존 `RESTART_PATTERN_ID` exact predecessor CAS를
  사용한다. 이 Tool이 소유한 ACTIVE/COMPLETED occurrence와 현재 선택 ID가 일치할 때만 활성화되며,
  같은 Pattern 하나를 첫 Stage부터 교체 재생한다. saved Flow는 읽지 않는다.
- Pattern Flow의 버튼은 `Restart Saved Flow (Fresh Arena)`로 명시했다. disk의 전체
  `scriptedSequence`를 reload하고 authoritative arena를 복구한 뒤 Pattern 01부터 saved
  Next/order/Wait를 실행한다.
- focused Python contract 125 tests, native Audition 30/30와 Flow 13/13, Client Debug x64
  `ClCompile`이 통과했다. Client 화면 문구와 실제 arena 차이는 사용자가 새 Debug EXE에서 확인한다.

## 현재 변경 기준 자동 검증

- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: PASS, changed 1 / artifacts 7
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, errors 0 / managed 42 / projected artifacts 9
- Ground Roar combat-object focused: PASS, 17/17
- Action Composition Workbench regression oracle: PASS, 55/55
- Action presentation focused: PASS, 47/47
- Effect V2 optional BossCatalog owner 회귀: PASS, 29/29. 제품 저장소에 BossCatalog가
  존재하면 Valtan combat-object group owner를 strict 검증하고, 독립 Effect fixture에 문서가
  없으면 owner lane 자체가 없는 것으로 처리한다.
- Silence source/Product reader parity: PASS. `SILENCE_APPLY` 100ms에서 ENTER-only 5000ms
  deadline을 승인하고 generic paired-EXIT lifetime closure에서는 제외했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core`: PASS.
  Network failures 0, Valtan canonical 6/6, Action Composition 10/10,
  Effect cue 11/11, presentation admission PASS, Character Select live isolation failures 0.
  Build evidence는
  `out/BuildPipeline/runs/20260902T085728666Z-debug-core-264bbae0.json`이다.
- Client 화면과 Effect visual fidelity는 자동 판정하지 않았으며 아래 인게임 항목은 사용자가
  새 Debug EXE에서 직접 확인한다.

## 최종 범위 정리

- 탐색 중 추가했던 `EncounterDataset`, `BossPatternGraphRuntime`,
  `ValtanStageActionSemantics`, presentation mutation coordinator와 Effect catalog slice는 현재
  Valtan Flow 저장/재시작 완료에 필요한 소비자를 닫지 못했으므로 제거했다.
  이 RESULT의 완료 구조로 주장하지 않는다.
- 현재 Valtan은 기존 canonical source/Product, inline Client/Server reader, 기존 writer와
  `VALTAN_VIEW_ADMISSION`을 사용한다. All Effects의 resource facade와 Workbench의 명시적
  V1/V2 picker는 서로의 저장 backend을 혼합하지 않는다.
- Ground Roar 새 기하는 `PublishV2` 재투영과 focused combat-object/Workbench 회귀로
  검증했다. Client의 실제 네 위치, 크기, 폭발 timing은 사용자 육안 검증 대상이며
  visual PASS로 기록하지 않았다.
