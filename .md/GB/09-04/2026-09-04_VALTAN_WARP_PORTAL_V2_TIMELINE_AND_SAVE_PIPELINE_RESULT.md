# 2026-09-04 발탄 Warp 포탈 V2 타임라인·저장 경로 구현 결과

## 1. 완료 상태

- 첫 돌진 포탈 쌍은 `STEP_02` `0ms`에 생성되고, 발탄은 `0~300ms` 숨은 뒤 `300~1,600ms`에 16m를 돌진한다.
- 반복 포탈 쌍은 `STEP_03`~`STEP_09`의 `300ms`에 생성된다. 발탄은 `0~600ms` 숨고 `600~1,900ms`에 돌진하므로, 이전 포탈의 도착 후 잔류 `300ms`와 새 포탈 선행 `300ms`가 연속된다.
- 두 포탈은 Server가 Stage ENTER에서 잠근 출발점·도착점과 같은 route yaw를 사용한다. 따라서 포탈 면은 실제 돌진 방향에 수직이고, 두 endpoint가 같은 방향 기준을 공유한다.
- 포탈 Group/leaf 총 수명은 `1,900ms`다. 생성 후 `1,600ms`까지 유지되고 마지막 `300ms`에 dissolve한다. 반복 Stage 경계를 넘어 살아 있는 이전 포탈은 다음 Stage `300ms`에 자연 종료된다.
- 포탈 Group child scale을 `[1,1,1]`로 원복하고 leaf scale track만 원본 대비 정확히 2배로 확대했다. 이에 따라 Group scale 중복 적용 없이 leaf position Y=3 중심을 유지한다.
- 도착 순간 Stage가 끝나도록 motion clock을 맞춰 발탄의 보이는 제자리걸음을 제거했고, 마지막 `STEP_10`도 `0~300ms` 숨겨 마지막 포탈 잔류와 복귀를 가린다.
- 기존 Warp V1 portal cue는 제거해 V1/V2 중복 재생을 막았다.
- Sequencer/Arena Clone의 V2 draft preview와 Composition Save의 Effect V2 baseline/candidate/read-set 전달을 연결했다. Local Play도 Product와 같은 body visibility window를 적용하고 reset 시 표시 상태로 복원한다.
- HIGH_JUMP 도끼 V2 impact/decal 군집은 combat-object pivot으로 재중심화했다.
- HIGH_JUMP TAKEOFF/LAND V1 바닥 표현은 `arena.center` snapshot으로 고정했다. gameplay 착지는 계속 `LEAP_TO_TARGET`이므로 발탄은 기존처럼 잠근 플레이어 위치로 착지한다.

## 2. 구현 계약

### 2.1 Portal route authority

- Server가 retarget 이후 Portal rush 시작점과 끝점을 한 번 잠그고, 해당 Stage 동안 같은 route를 유지한다.
- `WORLD_ENTITY_SNAPSHOT`에 유효한 boss Portal rush에만 조건부 route payload(시작 XYZ + 끝 XYZ)를 추가했다.
- wire 변경에 따라 network protocol version을 `54`로 올렸다.
- Client는 route를 Valtan presentation state에 보존하고 `portal.rush.start` / `portal.rush.end` virtual anchor로 노출한다.
- Effect V2의 `TARGET_YAW`는 두 virtual anchor에 동일한 route yaw를 적용한다.
- snapshot coalescing으로 첫 관측이 이동 도중이어도 Client 현재 pose에서 도착점을 재추정하지 않는다.

### 2.2 V2 authoring과 저장

- 여덟 Warp Stage에 start/end Group binding 16개를 추가했다.
- 각 binding은 `GROUP`, `STAGE`, `ONCE`, `SNAPSHOT_AT_START`, `TARGET_YAW`, `NATURAL`, `timeMs=0` 계약을 사용한다.
- 첫 Stage의 두 binding은 `0ms`, 반복 Stage의 두 binding은 `300ms`에 동시에 생성된다. 도착점 포탈도 출발점 포탈과 같은 시각에 생성된다.
- leaf의 `dissolveStart`는 초가 아닌 정규화된 lifetime 비율이다. 이를 `0.84210526`으로 설정해 생성 후 1,600ms까지 유지한 뒤 마지막 300ms에만 dissolve하도록 했다.
- `NATURAL` stop policy를 유지해 이전 Stage의 포탈이 다음 Stage까지 300ms 살아 있도록 했다. 실제 종료는 finite/non-loop leaf의 object-local `1.9s` lifetime이 담당한다.
- Reload 시 참조 resource body SHA-256 read-set을 보존하고, Save candidate closure가 참조하는 V2 leaf/group만 canonical writer에 전달한다.
- baseline/candidate/read-set 중 하나라도 없으면 Save를 시작하지 않으며, Reload 뒤 참조 resource가 바뀌면 writer lock 안에서 기존 정본을 유지한 채 거부한다.

### 2.3 HIGH_JUMP visual 보정

- migrated axe group child translation을 leaf local offset과 상쇄해 telegraph/impact 군집을 combat-object pivot으로 옮겼다.
- `arena.center` 고정 cue는 `LEAP_TO_TARGET` 또는 `LEAP_TO_ANCHOR`에서 허용하되, `.facing` / `.target-follow` 파생 anchor는 기존처럼 `LEAP_TO_ANCHOR + moveToAnchorBeforeTakeoff`만 허용한다.

## 3. 자동 검증

- 최종 1.9초 cadence와 leaf 2배 scale 반영 뒤 재검증했다.
  - portal-rush 전용 계약: PASS (`8/8`).
  - Pattern Tree 계약: PASS (`27/27`).
  - Valtan pattern master: PASS (`70/70`).
  - Effect V2 validator: PASS (`131` authored, `155` bindings, `16` groups, `13` independent, `84` textures).
  - Effect V2 단위 회귀: validator `31/31`, binding `20/20`, catalog `15/15`, product `4/4`, occurrence runtime `5/5`.
  - Composition atomic save/read-set 및 Animation Tool 집중 회귀: `7/7`, `6/6`, `4/4`, `14/14` PASS.
  - RootMotion 재생성/check PASS. STEP_02=`1600ms/50 samples`, STEP_03~09=`1900ms/59 samples`.
  - Project PublishV2/Validate, Composition Sync/Validate/Publish, Gameplay Validate/Publish, Valtan runtime-set Validate: PASS.
  - Debug Product build: PASS. Engine/Shared/Server/Client compile/link와 compiled shader closure PASS.
  - 최신 evidence: `out/BuildPipeline/runs/20260904T065404034Z-debug-product-3c1a82d2.json`.
- RootMotion unit의 기존 exact multi-clip allowlist가 이미 HEAD의 17개 binding을 7개로 기대하던 누락을 확인했다. Product 데이터를 바꾸지 않고 테스트 목록만 현재 committed binding inventory와 일치시켜 `5/5` PASS로 복구했다.

- 사용자 첫 육안 확인 뒤 portal hold/scale을 보정했다.
  - portal-rush 전용 계약: PASS (`8/8`).
  - Effect V2 validator: PASS (`131` authored, `155` bindings, `16` groups, `13` independent, `84` textures).
  - Effect V2 binding: PASS (`20/20`), catalog: PASS (`15/15`), product: PASS (`4/4`), occurrence runtime: PASS (`5/5`).
  - Effect V2 전체 discover는 `95/96` PASS. 남은 1건은 다른 동시 작업으로 Developer Tools focus option이 12개가 됐지만 기존 UI entry 테스트가 10개를 기대하는 불일치이며 portal 데이터·runtime과 무관하다.
  - Valtan pattern master Validate: PASS.
  - JSON parse 및 `git diff --check`: PASS (`git diff --check`는 기존 working copy의 LF→CRLF 경고만 출력).
- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: PASS, projection 최신 상태.
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS.
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS.
- Effect V2 validator: PASS (`131` authored, `155` bindings, `16` groups, `13` independent, `84` textures).
- Effect V2 binding suite: PASS (`20/20`), axe pivot 회귀 포함.
- Valtan master suite: PASS (`70/70`).
- Debug Product build: PASS.
  - Engine, Shared, Server, Client compile/link PASS.
  - compiled shader closure PASS.
  - WARP draw/readback: `V1=1352`, `V2=1352` pixels.
  - evidence: `out/BuildPipeline/runs/20260904T052910907Z-debug-product-0d312a15.json`
- `RunFullPipeline.bat` Debug/FullDiagnostic를 실행했다.
  - 모든 authoring/data domain, Product build, Valtan master, render/physics/model harness까지 PASS.
  - 최초 실행은 NetworkProtocolHarness의 route 유무 비교가 서로 다른 pattern/action 문자열 payload 크기를 직접 비교해 한 항목에서 중단됐다.
  - 비교 기준을 같은 Warp ID의 route 없는 payload로 교정한 뒤 NetworkProtocolHarness를 재빌드·재실행했고 `Portal Rush Route Snapshot Round Trip` 및 잘못된 route 거부 사례 모두 PASS, `failures: 0`을 확인했다.
  - 사용자가 자동 광역 재실행보다 수동 눈 검증 진행을 선택해 full entry 전체 재실행은 생략했다.

## 4. 수동 검증 대기

에이전트는 Client/UI를 실행하거나 visual fidelity를 대신 판정하지 않았다. 다음 항목은 사용자 눈 검증이 필요하다.

1. Arena Clone/Sequencer에서 `VALTAN_WARP` `STEP_02`~`STEP_09` 중 하나를 Play한다.
2. Stage `0ms`에 출발점과 도착점 포탈이 동시에 보이는지 확인한다.
3. 포탈 면이 돌진 선분에 수직이고 발탄 몸통 중간을 통과하는 높이인지 확인한다.
4. 첫 다리는 `0~300ms`, 반복 다리는 `0~600ms`에 발탄이 숨고 각각 그 직후 정확히 1.3초 돌진하는지 확인한다.
5. 이전 포탈이 도착 뒤 300ms 더 dissolve한 다음, 같은 순간 새 포탈 쌍이 나타나고 300ms 뒤 다음 돌진이 시작되는지 확인한다.
6. 두 포탈의 leaf 화면상 크기가 원본 대비 약 2배이며, 중심 높이는 기존 Y=3에서 올라가지 않았는지 확인한다.
7. HIGH_JUMP에서 플레이어별 도끼 원/타격이 실제 damage pivot과 일치하고, TAKEOFF/LAND 바닥 표현은 아레나 중앙에 고정되는지 확인한다.

Cadence는 `VALTAN_WARP` 처음부터 연속 Play로 판정한다. 직접 seek/rewind는 이전 Stage에서 넘어온 NATURAL 포탈의 과거 occurrence를 재구성하지 않는다.

포탈의 leaf local X/Y는 현재 asset과 코드 기준의 초기 배치다. 높이·가로 중심의 미세 조정은 사용자 관찰값을 받은 뒤 V2 leaf transform만 좁게 조정한다.

## 5. 작업 경계

- 기존 Rock effect V1, 3페이즈 발악/발구르기 후 사자후/피자 패턴 변경은 보존했다.
- `test_action_composition_workbench_regression_oracles.py`의 동시 작업 내용은 수정하거나 되돌리지 않았다.
- 워킹트리에 사용자/Claude 변경이 함께 있어 stage/commit/push하지 않았다.

## 6. 사용자 육안 검증

- 2026-09-04 사용자가 최신 Warp 패턴을 직접 재생해 확인했고, 결과가 "깔끔하고 괜찮다"고 승인했다.
- 따라서 현재 Warp 포탈의 V2 group, leaf 2배 크기, Y=3 중심, `0.0s 생성 → 0.3s 돌진 시작 → 1.6s 도착 → 1.9s 완전 소멸` cadence를 후속 포탈 작업의 수동 PASS 기준선으로 고정한다.
- HIGH_JUMP 도끼·아레나 중앙 표현은 이번 사용자 메시지에서 별도로 관찰 결과를 받지 않았으므로 해당 항목의 수동 판정은 이 승인에 포함하지 않는다.
