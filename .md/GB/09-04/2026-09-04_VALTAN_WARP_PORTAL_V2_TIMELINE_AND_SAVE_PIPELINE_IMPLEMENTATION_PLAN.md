# 2026-09-04 발탄 Warp 포탈 V2 타임라인·저장 경로 구현 계획

## 1. 목표

- `VALTAN_WARP` 첫 돌진은 포탈 생성 뒤 `300ms`, 반복 돌진은 이전 포탈의 도착 후 잔류 `300ms`와 새 포탈 선행 `300ms`를 합친 `600ms` 동안 발탄 본체를 숨긴다.
- `STEP_02`는 Stage `0ms`, `STEP_03`~`STEP_09`는 Stage `300ms`에 출발 위치와 도착 위치 양쪽의 `boss.valtan.portal` V2 Group을 동시에 `SNAPSHOT_AT_START`로 생성한다.
- 두 포탈은 Server가 Stage 진입 시 잠근 실제 돌진 선분을 공유하고, 돌진 방향에 수직인 yaw와 발탄 몸통 중심 높이를 사용한다.
- 실제 돌진은 `16m / 1,300ms`다. 첫 돌진 Stage는 `1,600ms`, 반복 Stage는 `1,900ms`이며 도착 순간 다음 Stage로 전환해 보이는 제자리걸음 구간을 제거한다.
- 포탈 Group과 두 non-loop leaf의 총 수명은 `1,900ms`다. 생성 후 `1,600ms` 동안 형상을 유지하고 마지막 `300ms`에 dissolve한다. 따라서 도착 뒤에도 이전 포탈이 `300ms` 남는다.
- 포탈 Group child scale은 `[1,1,1]`을 유지하고 authored leaf scale만 원본 대비 정확히 2배로 확대한다. Group scale의 중복 적용과 leaf 위치 Y의 배율 이동을 피한다.
- 마지막 돌진 뒤 `STEP_10`도 `0~300ms` 동안 본체를 숨겨 마지막 포탈의 dissolve와 복귀 동작이 겹쳐 보이지 않게 한다.
- 같은 위치에 V1/V2가 중복 생성되지 않도록 기존 `effect.valtan.project-tuned.sequence.warp.portal` V1 cue를 제거한다.
- Sequencer에서 추가한 V2 binding draft가 Arena Clone Play에 즉시 반영되고, Composition Save가 Reload 시점의 resource read-set을 실제 writer까지 전달하도록 닫는다.
- `VALTAN_HIGH_JUMP`의 플레이어별 도끼 V2 타격 군집을 combat-object pivot으로 되돌리고, TAKEOFF/LAND 바닥 표현은 gameplay 착지 정책을 바꾸지 않은 채 `arena.center` snapshot에 고정한다.

## 2. 실측 원인

- 현재 V2 binding 정본에는 `VALTAN_WARP` 및 `boss.valtan.portal` row가 없다. UI에서 Append한 draft가 Save 실패 뒤 물리 파일에 남지 않은 상태다.
- `Publish-ValtanTuningRuntimeSet.ps1`와 Python canonical writer는 `EffectV2ReadSetPath`를 지원하지만, Client의 owner draft와 `Run-ValtanAuthoringSaveJob.ps1` 경계는 read-set payload를 운반하지 않는다.
- Arena Clone Play는 V2 draft가 있을 때 catalog disk reload를 건너뛰고 immutable authoring snapshot을 `Sync_StageAuthoring`에 전달하는 경로를 이미 가진다. 따라서 이번 현상의 주원인은 렌더 pipeline 우회가 아니라 저장 실패로 binding 정본이 비어 있던 점이며, 해당 preview 경로는 회귀 검증으로 고정한다.
- Local Sequencer sample 경로는 Product의 `bodyVisibility`를 적용하지 않았으므로, 연속 Play에서 Stage별 hidden window를 transactionally stage하고 반개구간 `[from,to)`으로 평가하는 preview 전용 상태가 필요하다.
- Warp motion patch writer는 기존 `bodyVisibility`를 도착 뒤 trailing gap으로 다시 계산한다. 이를 그대로 두면 다음 Workbench Save에서 `0~500ms` 계약이 되돌아간다.
- 도착점은 Stage가 시작된 뒤 현재 pose만 보고 복원할 수 없다. snapshot coalescing 시 첫 Client 관측이 이미 이동 중일 수 있으므로 Server가 잠근 시작점·끝점을 같은 occurrence의 불변 route로 복제해야 한다.
- HIGH_JUMP 도끼 V2 group의 migrated child translation이 leaf의 약 `+4m Z` 오프셋과 합성되어 damage pivot보다 앞에 보였다. gameplay hit 중심은 정상이고 visual group만 재중심화 대상이다.

## 3. 변경 범위

### 3.1 Authoring 데이터

- `Data/Valtan/Valtan.presentation.json`
  - `STEP_02`: `bodyVisibility = { hiddenFromMs: 0, hiddenToMs: 300 }`
  - `STEP_03`~`STEP_09`: `bodyVisibility = { hiddenFromMs: 0, hiddenToMs: 600 }`
  - `STEP_10`: `bodyVisibility = { hiddenFromMs: 0, hiddenToMs: 300 }`
  - Warp V1 portal cue 제거
- `Data/Valtan/Valtan.gameplay.json`
  - `STEP_02`: delay `300ms`, duration `1,600ms`
  - `STEP_03`~`STEP_09`: delay `600ms`, duration `1,900ms`
  - 공통 distance `16m`, speed `12.3076925m/s`, 50ms 간격 hit 26개
- `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`
  - 첫 Stage `0ms`, 반복 Stage `300ms`에 출발/도착 V2 Group binding 두 개씩 추가
  - `portal.rush.start`, `portal.rush.end` virtual slot과 `TARGET_YAW` 사용
- `Data/Effects/V2/Groups/boss.valtan.portal.effectv2group.json`
  - Group/child cap을 1,900ms로 설정
  - 두 child local scale은 `[1, 1, 1]`로 유지
- `Data/Effects/V2/Authored/boss.valtan.portal.*.effectv2.json`
  - lifetime 1.9초, `dissolveInEnd=0`, `dissolveStart=0.84210526`
  - leaf position/Y를 유지하고 black/cyan scale track을 원본 대비 2배로 설정
  - `dissolveStart`는 초가 아니라 lifetime 정규화 비율이므로 `1600/1900`, 즉 생성 후 1,600ms부터 dissolve를 시작한다.
- `Data/Effects/V2/Groups/boss.valtan.axe.effectv2group.json`
  - decal/impact child의 local Z를 leaf offset과 상쇄해 combat-object pivot에 재중심화
- `Data/Valtan/Valtan.presentation.json`
  - HIGH_JUMP TAKEOFF/LAND cue를 `arena.center` + `snapshot`으로 고정
  - gameplay의 `LEAP_TO_TARGET` 착지 정책은 유지

### 3.2 작성·저장 경로

- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
  - portal-rush motion 저장 시 leading retarget delay를 body hidden window로 재생성하고 legacy V1 portal cue를 제거한다.
- `CEffectV2Catalog`
  - Reload에서 유효 catalog resource raw body의 SHA-256 snapshot을 보존한다.
  - Save 준비 시 candidate binding closure에 필요한 row만 골라 canonical V2 read-set JSON을 만든다.
- `CActionCompositionWorkbench -> CBalanceTool -> Run-ValtanAuthoringSaveJob.ps1`
  - Effect V2 baseline/candidate/read-set을 하나의 필수 triplet으로 전달한다.

### 3.3 Server route와 Client V2 anchor

- `CValtanBrain`은 각 Portal target-rush Stage ENTER에서 retarget 결과를 잠근 뒤 시작점·끝점을 확정한다.
- `WORLD_ENTITY_SNAPSHOT`은 유효한 boss Portal rush에만 여섯 float route를 조건부로 운반한다. wire 변경에 맞춰 protocol version은 `54`로 올린다.
- `CValtan`은 동일 Stage 안에서 route가 변하지 않는지 확인하고, `CEffectV2Object`는 두 virtual slot을 동일 route yaw의 world pivot으로 해석한다.
- Local Arena Clone은 authored distance와 현재 preview look으로 같은 두 anchor를 구성해 Product와 동일한 V2 lane을 탄다.
- Local Arena Clone은 Stage별 body visibility window도 별도 preview map에 stage하고, rewind/reset 때 본체 표시 상태로 복원한다.

## 4. 실패·롤백 계약

- Reload 시 resource read-set을 만들 수 없으면 새 catalog snapshot을 commit하지 않는다.
- Effect V2 owner 세 payload 중 하나라도 빠지면 canonical writer를 시작하지 않는다.
- Reload 뒤 참조 leaf/group raw body가 바뀌면 writer lock 안의 current-read-set 비교가 Save를 거부하며 기존 정본을 유지한다.
- V2 binding/group/leaf validation이 실패하면 기존 V1 cue를 fallback으로 되살리지 않는다.

## 5. 검증

1. JSON parse와 Effect V2 validator
2. portal-rush 전용 계약 및 Composition atomic-save/read-set 회귀
3. Valtan authoring/product projection과 domain publisher validation
4. Network protocol route 왕복/잘못된 route fail-closed harness
5. `RunFullPipeline.bat` / Debug FullDiagnostic
6. `git diff --check`

Client/UI는 에이전트가 실행·조작하지 않는다. 자동 검증 뒤 사용자가 직접 Arena Clone의 처음부터 연속 Play와 Server+Client smoke에서 포탈 위치, Stage별 hide, 1.9초 수명, dissolve fidelity를 판정한다. 직접 seek/rewind는 이전 Stage에서 살아 넘어온 NATURAL 포탈을 재구성하지 않으므로 cadence 판정에는 연속 Play를 사용한다.
