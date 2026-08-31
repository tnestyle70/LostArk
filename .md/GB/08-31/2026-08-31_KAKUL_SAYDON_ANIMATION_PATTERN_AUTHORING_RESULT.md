# 쿠크·세이튼 Animation Pattern Authoring 구현 결과

## 결론

Animation Tool에서 쿠크·세이튼 네 기획 프로필의 액션을 원본 `actionreference.json` 순서와
`displayName` 그대로 나열하고 선택할 수 있게 했다. 선택 액션 아래에는 모든 stage/slot의 실제
WModel clip occurrence를 원본 순서대로 평탄화해 표시하며, 개별 clip 또는 액션 전체를 preview할
수 있다.

`Create Pattern`은 선택 액션의 유효 occurrence를 별도 `REFERENCE_ONLY` 문서로 복사한다. 생성한
패턴은 전체 순차 재생, Pattern Duplicate/Delete, clip 선택 preview, Duplicate/Delete, Move Up/Down,
atomic Save/Reload를 지원한다. 이 결과를 아직 존재하지 않는 쿠크·세이튼 Server Product 전투
Pattern으로 위장하거나 Valtan writer/backend에 연결하지 않았다.

## 실제 반영 범위

### 1. 기획 이름·순서와 전체 clip 목록

- `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCT_07`, `MN_RPCZ_00`의 `actions[]` vector 순서와
  `displayName`을 그대로 `Planner Actions`에 표시한다.
- 현재 데이터는 액션 349개, stage 4,072개, 실제 WModel slot 3,692개다.
- 중복 display name을 구분할 수 있도록 `sourceActionId`를 함께 표시하고 exact
  `(sourceActionId, stageId, slotId)` join을 유지한다.
- 선택 액션의 모든 stage/slot을 한 목록에 평탄화해 stage/slot identity, runtime clip, timing,
  play rate와 source 상태를 표시한다.
- sparse `actionbindings.json` override가 있으면 immutable reference의 slot 위치는 유지한 채
  effective clip/timing을 표시하고 복사한다.
- HOLDOUT 액션은 원본 목록에서 숨기지 않지만 전체 액션 preview와 Pattern 생성·저장은 거부한다.

### 2. clip과 액션 preview

- 선택 clip 행은 기존 Animation Tool의 실제 model clip 선택 경로로 단일 preview한다.
- `Preview Action`은 선택 액션의 유효 occurrence 전체를 source timing 순서대로 재생한다.
- 각 occurrence는 `sourceStartMs`, `playMs`, `playRate`와 명시적 `endPolicy`를 사용한다.
  - native source window 안: `EXACT`
  - non-loop source window가 native clip보다 김: `HOLD_LAST_POSE`
  - loop slot: `LOOP_TO_WINDOW`
- preview는 target generation과 현재 model clip을 매 frame 확인한다. target/profile 변경, Tool 비활성,
  외부 clip 교체, 잘못된 timing에서는 중단하고 idle, 1배속, loop 상태를 복원한다.

### 3. Create Pattern과 CRUD

- `Create Pattern`은 선택 액션의 effective occurrence를 source 순서 그대로 새 Pattern으로 복사한다.
- `nextPatternOrdinal`과 Pattern별 `nextOccurrenceOrdinal`을 단조 증가시켜 삭제 뒤에도 stable ID를
  재사용하지 않는다.
- Pattern 목록은 filter/select, `Play Pattern`, `Duplicate Pattern`, `Delete Pattern`을 제공한다.
- Pattern clip 목록은 선택 window preview, `Duplicate Clip`, `Delete Clip`, `Move Up`, `Move Down`을
  제공한다.
- Pattern은 하나 이상의 clip을 가져야 하므로 마지막 clip 삭제는 거부하고 Pattern 삭제를 안내한다.
- mutation은 후보 문서를 먼저 validate한 뒤에만 현재 draft에 commit한다.

### 4. 별도 저장 문서와 rollback

다음 네 문서를 추가했다.

```text
Data/Animation/Authored/KakulSaydon/MN_RPCT_05.patternbindings.json
Data/Animation/Authored/KakulSaydon/MN_RPCT_06.patternbindings.json
Data/Animation/Authored/KakulSaydon/MN_RPCT_07.patternbindings.json
Data/Animation/Authored/KakulSaydon/MN_RPCZ_00.patternbindings.json
```

문서 계약은 `lostark.kakul-animation-pattern-bindings` formatVersion 1,
`authority=REFERENCE_ONLY`다. 각 문서는 현재 profile의 exact `referenceRevision`에 고정된다.

- Load: `parse -> validate -> stage -> commit`
- Save: `validate -> sibling temporary durable write -> strict reparse/exact compare -> reference CAS ->
  atomic replace`
- fail-close: schema/version/profile/revision/authority, path escape, HOLDOUT source, 중복/비단조 ID,
  source tuple/clip/timing/end policy 불일치
- action reference, sparse action binding, pattern document 세 문서가 모두 유효할 때만 Tool 상태를
  함께 교체한다.

### 5. 프로젝트와 extraction check

- `KakulAnimationPatternDocument.h/.cpp`를 Client project와 기존 Animation 물리 filter에 등록했다.
- 새 contract harness는 네 empty canonical 문서, 정상 non-empty fixture, 잘못된 version/profile/
  revision/source/clip/timing/ID/path, HOLDOUT source 거부와 atomic save 구조를 검사한다.
- extraction generator의 `--check`는 Windows checkout의 CRLF와 canonical LF만 다른 경우에는 같은
  의미 문서로 인정하되 실제 JSON 내용 drift는 계속 실패하도록 교정했다.

## 자동 검증 결과

### Kakul/Valtan focused contract

```text
python -B Tools/KakulSaydonPipeline/build_kakul_animation_reference.py --check
-> PASS: 4 profiles / 349 actions / 4,072 stages / 3,692 slots

python -B -m unittest
  Tools.KakulSaydonPipeline.test_build_kakul_animation_reference
  Tools.KakulSaydonPipeline.test_kakul_animation_action_document_contract
  Tools.KakulSaydonPipeline.test_kakul_animation_pattern_document_contract
  Tools.ValtanPipeline.test_action_composition_sequence_identity_contract
  Tools.ValtanPipeline.test_action_composition_sequencer_occurrence_timing_contract
  Tools.ValtanPipeline.test_action_composition_workbench_regression_oracles
-> 78/78 PASS
```

네 Pattern JSON과 `Client.vcxproj`, `Client.vcxproj.filters` strict JSON/XML parse도 PASS했다.

### 표준 빌드

```text
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
-> PASS
```

- Engine, Shared, Server, Client compile/link: PASS
- Kakul map/world, Valtan Product, gameplay balance, navigation과 관련 공용 publisher: PASS
- Product compiled shader closure와 WARP readback: PASS
- build evidence:
  `out/BuildPipeline/runs/20260831T113101011Z-debug-product-387e055a.json`
- 변경 파일 `git diff --check`: PASS

Client/UI는 저장소 경계에 따라 에이전트가 자율 실행하지 않았다. 자동 검증은 구조·저장·컴파일·링크
완료 증거이며 화면의 visual fidelity를 대신 판정하지 않는다.

## 사용자 수동 검증 대기

1. Server + Client profile을 사용자가 시작하고 Lobby에서 F1 → Animation Tool을 연다.
2. 쿠크·세이튼 profile을 열어 기획 이름과 `sourceActionId` 순서가 보이는지 확인한다.
3. 액션을 선택해 `Selected Action Clips`에 전체 clip이 나열되고 행 선택 시 해당 clip이 재생되는지
   확인한다.
4. `Preview Action`으로 액션 전체가 순서대로 재생되는지 확인한다.
5. `Create Pattern`을 누르고 `Created Local Patterns`에서 생성 결과를 선택한다.
6. `Play Pattern`, Pattern Duplicate/Delete, clip Duplicate/Delete/Move와 `Save Patterns` 후 Reload를
   확인한다.
7. profile 또는 target을 바꿨을 때 이전 body가 idle/1배속으로 복원되는지 확인한다.

현재 PC는 팀 LAN sync에서 `server-host`로 판정됐지만 TCP 7777 LocalSubnet 방화벽 규칙이
누락되었거나 오래된 상태다. 외부 팀 Client까지 붙여 검증하려면 관리자 PowerShell에서 팀 LAN sync
스크립트를 한 번 실행해 규칙을 갱신해야 한다. 로컬 코드·데이터·빌드 검증은 이 상태와 무관하게
완료했다.

## 의도적으로 완료라고 부르지 않는 경계

- 쿠크·세이튼 Product Server Pattern, damage/collider, Effect/Sound cue, replication은 만들지 않았다.
- HOLDOUT source를 추측으로 살리거나 다른 clip으로 대체하지 않았다.
- `MN_RPCT_07`의 별도 physical body를 만들지 않고 검증된 `MN_RPCT_05` alias를 유지했다.
- 사용자 화면 확인 전에는 visual PASS 또는 원작 동작 충실도 완료로 기록하지 않는다.
