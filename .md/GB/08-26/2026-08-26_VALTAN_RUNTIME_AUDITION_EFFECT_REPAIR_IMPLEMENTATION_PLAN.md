# 발탄 런타임 패턴 재생·Effect 오염 복구 구현 계획

## 1. 목표

현재 실행 화면에서 확인된 다음 결함을 하나의 검증 단위로 닫는다.

- `effect.valtan.sky-axe.active`에 잘못 섞인 도넛 wave Element를 제거해 HIGH_JUMP 독립 도끼 Effect가 도넛 파동을 재생하지 않게 한다.
- 플레이어 대상 도끼는 각 웨이브 생성 시점 좌표에 고정하고, 랜덤 도끼와 같은 STATIC combat-object 의미를 갖게 한다.
- 첫 등장 휠윈드와 일반 Effect-bearing 휠윈드의 identity/cue 차이를 보존하면서 첫 등장 회전에도 정확한 Product Effect를 연결한다.
- 첫 회전 뒤 `VALTAN_DASH_CHARGE`와 opening charge의 Server 이동이 stage 경계 순간이동이 아니라 연속 snapshot 이동으로 보이게 한다.
- 돌진 receiver sweep, exact wall mutation, BREAKING snapshot과 `WALL_CONTACT -> GROGGY`를 같은 fixed tick transaction으로 묶는다.
- HIGH_JUMP는 긴 animation wall 전체에 이동을 늘이지 않고 authored `b_root`에서 실측한 짧은 상승/낙하 subwindow만 Server motion으로 사용한다.
- Server-authoritative Valtan의 X/Y/Z는 snapshot 보간 한 경로만 소유하고, skeleton `b_root` translation을 중복 합성하지 않는다.
- Effect Tool의 전체 패턴 Replay가 HIGH_JUMP와 이동 패턴의 Server motion을 절대 타임라인으로 미러링한다.
- 실제 Valtan root에 누적된 축별 scale drift를 `GAMEPLAY_FOOTPRINT`가 안전하게 정규화해 도넛·4연속 공격·휠윈드·돌진 장판·130줄 전멸기 Effect를 다시 생성한다.
- `VALTAN_DASH_CHARGE`가 아레나의 159벽뿐 아니라 먼저 접촉하는 109 외곽벽에서도 같은 fixed tick에 정지, 벽 파괴와 GROGGY를 commit한다.

## 2. 현재 실측

- sky-axe Effect의 모든 Element attachment는 `follow=false`다. 화면 이동은 Server combat-object snapshot root가 소유한다.
- 현재 sky-axe authored 문서에는 정상 5개 Element 외에 `sprite_particle_8`과 도넛 wave authored copy 두 개가 추가돼 있다.
- 플레이어 도끼는 `bTrackUntilFirstPulse=true`로 stage되어 생성 후 1200ms 동안 매 fixed tick 플레이어 좌표를 따른다. 랜덤 도끼만 false다.
- fresh encounter는 `VALTAN_ENTRANCE_WHIRLWIND`를 먼저 선택하고, 일반 `VALTAN_WHIRLWIND`의 두 Effect cue는 exact action join 때문에 Entrance에 적용되지 않는다.
- `VALTAN_DASH_CHARGE`는 1500ms에 20m를 이동하지만 Client Valtan에는 network transform interpolation이 없어 snapshot 계단이 순간이동처럼 보인다.
- opening charge는 stage의 20m motion 대신 pattern maximumRange 100m를 1500ms에 덮어써 과도한 속도가 발생한다.
- 일반 Dash의 159 wall 10개에는 `COLLISION_IMPACT` exact binding이 있으나, timeline의 `ORDINARY_WALLS_GONE` 이후 남아 첫 접촉이 되는 109 외곽 receiver 30개에는 binding이 없다. 이 상태의 sweep은 body만 정지시키고 CHARGE를 유지하며, generic body contact가 나중에 벽을 파괴한다.
- HIGH_JUMP `b_root`는 TAKEOFF 약 1133ms까지 지상, 1133~1500ms 급상승, AIRBORNE 정점 유지, LAND 시작 후 약 267ms 급하강이다. 기존 Server는 TAKEOFF 1933ms와 LAND 3200ms 전체에 이동을 늘였고 Client는 이 world Y 위에 local `b_root`를 다시 더했다.
- 도넛을 포함한 누락 Effect의 cue ID, catalog row와 authored 문서는 존재하며 load validation도 통과한다. 실제 실패는 Valtan root basis 길이 `(0.997025, 1.0, 0.997025)`를 uniform scale이 아니라는 이유로 `GAMEPLAY_FOOTPRINT` 생성 전에 거부한 런타임 조건이다.

사망 player의 Pattern Audition reset은 Boss Tool의 기존 `Revive Player` 계약을 사용하며 이번 후속 보정에서 별도 replay 경로를 만들지 않는다.

현재 관련 파일은 다른 세션의 staged 변경을 포함한다. 구현은 index를 변경하지 않고 working-tree delta만 추가하며, 다른 변경을 되돌리지 않는다.

## 3. 변경 단위

### G01. Effect 소유권 복구

- sky-axe authored 문서에서 도넛에서 복사된 Element만 제거한다.
- Effect catalog, BossCatalog의 stable join은 변경하지 않는다.

종료 증거는 sky-axe exact Element allowlist, JSON parse와 saved-row focused test다.

### G02. Server combat-object snapshot 의미

- `LOCKED_TARGET_PER_ALIVE_PLAYER`는 각 wave spawn pose snapshot으로 처리한다.
- `LOCKED_TARGET_UNTIL_FIRST_PULSE`만 명시적 추적을 유지한다.
- player association ID는 보존하되 HIGH_JUMP player axe pose는 이동하지 않는다.

종료 증거는 player 이동 뒤 도끼 X/Z 불변, 1200ms hit의 spawn-pose 판정, 2/1/3 player axe와 4/4/4 random axe wave count다.

### G03. 휠윈드와 연속 돌진 Product

- Entrance의 stable pattern/action/wall contract는 보존하고 Entrance 전용 cue identity가 일반 휠윈드 Effect asset을 공유하게 한다.
- DASH_CHARGE의 gameplay motion, stage duration, root-motion suppression과 presentation wall을 대조해 Server 한 owner만 20m/1500ms 연속 이동을 적용하게 한다.
- opening charge도 stage-authored distance를 소비하고 pattern maximumRange로 속도를 덮어쓰지 않는다.
- Client Valtan은 Character와 같은 bounded snapshot interpolation을 사용하며 10m 초과 transfer, tick wrap, stale/equal tick을 명시적으로 처리한다.

종료 증거는 fresh encounter 첫 회전 Effect occurrence, 두 charge의 fixed-tick 중간 위치, Debug/Release Client focused compile과 model composition test다.

### G04. 돌진 receiver·벽 파괴·그로기 transaction

- 159 wall 10개의 기존 binding과 함께 109 외곽 wall 30개 전부에 normal `VALTAN_DASH_CHARGE/CHARGE/COLLISION_IMPACT` binding을 stable receiver/mutation ID로 연결한다.
- swept receiver collision이 발생해도 exact mutation이 새로 commit되지 않으면 body만 정지하고 GROGGY를 만들지 않는다.
- exact mutation이 commit되면 같은 room tick에 BREAKING lifecycle과 `WALL_CONTACT` branch를 적용해 GROGGY로 전환한다.
- 돌진이 먼저 소비한 외곽벽은 이후 109줄 IMPACT batch에서 최종 상태로 유지하고, 나머지 29개만 같은 stage transaction으로 전환한다.

종료 증거는 실제 `ORDINARY_WALLS_GONE` 상태에서 center→109 outer receiver 첫 접촉의 중간 tick swept position, same-tick BREAKING/GROGGY, consumed receiver의 safe stop, Dash 선행 파괴 1개 + 후속 109줄 29개 partial batch와 159+109 총 40개 exact binding World Product projection invariant다.

### G05. HIGH_JUMP Server/Client 위치 단일 소유권

- `serverMotion`에 TAKEOFF와 travel stage의 start/end millisecond subwindow를 추가하고 exact schema, stage bound, bootstrap version과 sealed legacy row를 함께 갱신한다.
- HIGH_JUMP는 1133~1500ms에 정점으로 상승하고 AIRBORNE 동안 유지한 뒤 LAND 0~267ms에 locked target으로 낙하한다.
- Server-authoritative Client Valtan은 `b_root` X/Y/Z를 모두 고정하고 Server snapshot X/Y/Z만 그린다. Tool/non-network preview는 기존 authored vertical root를 유지한다.

종료 증거는 1.0s 지상, 1.2s 상승 중, 1.5s 정점, AIRBORNE 정점 유지, LAND 267ms 이내 정확 착지와 Product/authoring/bootstrap exact join이다.

### G06. GAMEPLAY_FOOTPRINT Effect 생성 복구

- `GAMEPLAY_FOOTPRINT`는 owner root의 축별 basis 길이를 각각 제거하고 authored `worldScale`을 적용하므로, finite·nondegenerate·orthogonal·right-handed 조건을 만족하는 미세한 축별 scale drift를 허용한다.
- shear, reflection, degenerate와 nonfinite owner root는 계속 거부한다.
- cue나 authored Effect를 삭제하거나 새 runtime 경로를 만들지 않고 기존 boss-root cue 생성 경계만 보정한다.

종료 증거는 실제 drift `(0.997025, 1.0, 0.997025)`가 authored `(1.5, 1.5, 1.5)`로 정규화되고 잘못된 행렬은 계속 거부되는 focused regression이다.

### G07. P2/P3 animation과 Server Actual 동등성

- Character Select/Effect Tool local preview와 Server Actual의 clock, transform, root-motion, cue resolver 차이를 문서로
  고정한다.
- 20개 P2/P3 debug intake chain을 manual audition Product의 clip 순서, source offset, mapping basis와 play rate에
  exact-join하고 미분류 chain 또는 clip drift를 PublishV2에서 거부한다.
- `VALTAN_ARENA_BREAK_109`는 TAKEOFF/DROP motion window부터 IMPACT phase/wall commit,
  WIDE_REVEAL roar start/loop와 RECOVERY roar end까지 하나의 contract로 검증한다.

종료 증거는 lineage positive/negative fixture, 중앙점프→포효 exact animation test와 Server room-tick contract다.

## 4. 검증

```powershell
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
python -B Tools/EffectPipeline/test_valtan_model_view_composition.py
python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python -B Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
python -B Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1 -Mode Validate
```

관련 Server contract test, Server/Client x64 build와 `git diff --check`를 실행한다. Client 시각 품질은 사용자가 직접 재생해 최종 판정한다. Server gameplay 또는 combat-object 변경은 Effect Tool Refresh가 아니라 Server 재빌드·재시작 뒤 확인한다.
