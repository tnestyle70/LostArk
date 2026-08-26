# 발탄 Authoring Preview와 Server Actual 재생 동등성 계약

## 1. 결론

Character Select의 Server Play와 Valtan Arena는 서로 다른 발탄 클래스를 쓰지 않는다. 두 Level 모두
`CClientReplication`이 `isServerAuthoritative=true`인 `CValtan`을 만들고 같은 snapshot/presentation 경로를
소비한다.

실제로 달랐던 경로는 다음 두 가지다.

| 실행 모드 | 시간·위치 정답 | Effect 선택 | 완료 증거 |
|---|---|---|---|
| Local Authoring Preview | Tool의 local clip clock과 authored `b_root` | 편집 중인 후보 문서 또는 Debug alias | 저작 참고만 가능 |
| Server Actual | Server fixed-tick stage/action clock과 world snapshot | canonical Product cue | 자동 contract와 사용자 육안 확인 |

Effect Tool이나 local preview에서 맞춘 결과가 그대로 Arena에 나타나려면, 같은 모델을 쓴다는 사실만으로는 부족하다.
`patternId/stageId/actionId/clipOccurrenceId`, stage wall, Server motion, root 축 소유자와 Product effect asset이 모두
같은 joined revision에 봉인되어야 한다.

## 2. 이번 결함의 근본 원인

### 2.1 총 시간만 맞고 이동 곡선은 달랐다

기존 HIGH_JUMP는 TAKEOFF/LAND의 총 stage duration만 animation wall과 맞췄다. 실제 `b_root`는 TAKEOFF의
`1133~1500ms`에 급상승하고 LAND의 `0~267ms`에 급하강하지만 Server는 stage 전체를 선형 비율로 이동했다.
따라서 Preview의 빠른 상승·대기·빠른 낙하가 Arena에서는 느린 상승·느린 낙하로 바뀌었다.

해결 계약은 `serverMotion`이 총 duration 외에 다음 네 값을 소유하는 것이다.

- `takeoffStartMs`, `takeoffEndMs`
- `travelStartMs`, `travelEndMs`

publisher, Client parser, Balance Tool, Server bootstrap과 `CValtanBrain`은 이 네 값을 exact field로 소비한다.
첫 stage는 `TAKEOFF`, travel stage는 그 뒤에 있어야 하며 각 window가 소유 stage duration을 벗어나면 publish를
거부한다.

### 2.2 Server world transform과 skeleton root가 위치를 중복 소유했다

Server가 Valtan의 world X/Y/Z를 snapshot으로 보낸 상태에서 Client model의 `b_root` Y도 남아 있었다. body,
socket part와 root-follow Effect가 Server 이동 위에 authored root 이동을 한 번 더 합성할 수 있는 구조였다.

현재 축 소유권은 다음으로 고정한다.

- Local Preview/Tool: authored vertical `b_root`를 유지한다.
- Server-authoritative `CValtan`: `b_root` X/Y/Z를 모두 잠그고 Server snapshot만 world 위치를 소유한다.
- collider debug mirror는 보간된 Server presentation transform을 따라가며 hit 권위를 갖지 않는다.

### 2.3 돌진 snapshot을 즉시 좌표로 덮어썼다

20m/1500ms 돌진을 30Hz snapshot마다 즉시 적용하면 약 0.44m 단위 계단 이동이 보인다. 현재 Client는 bounded
sample queue, 2-tick presentation delay, drift correction, 10m transfer reset과 tick wrap/stale/equal-tick 계약으로
X/Y/Z와 yaw를 보간한다. 실제 충돌과 벽 파괴는 계속 Server fixed tick이 소유한다.

향후 moving P2/P3 pattern은 transform만 local로 보정하거나 clip root motion으로 재이동시키지 않는다. Product
승격 전 Server Actual에서 action clock과 owner transform의 체감 동기화를 확인해야 하며, 확인 전 row는
`MANUAL_SERVER_AUDITION/AUDITION_ONLY`를 벗어날 수 없다.

### 2.4 첫 휠윈드는 일반 휠윈드와 다른 exact identity였다

fresh encounter의 첫 회전은 `VALTAN_ENTRANCE_WHIRLWIND/SWEEP`이고 일반 회전은
`VALTAN_WHIRLWIND/SPIN`이다. runtime은 의도적으로 pattern, stage index, action과 clip occurrence를 exact-match하므로
일반 휠윈드 cue가 intro에 자동 적용되지 않는다. 기존 결함은 exact join이 느슨해서가 아니라 실제 첫 경로에 전용
cue가 없어서 발생했다.

현재 intro는 자기 stable cue 두 개를 갖고 검토된 일반 회전 Effect asset을 공유한다. 이후 selector 또는 phase-entry가
별도 pattern identity를 만들면 기존 cue를 암묵적으로 상속하지 않고 그 실제 identity의 cue coverage를 함께 추가한다.

### 2.5 Tool 후보 Effect와 Product Effect는 같다고 가정할 수 없다

Effect Tool은 Debug/V1 alias 또는 현재 편집 문서를 미리 볼 수 있지만 Server Actual은 canonical Product cue의
`effectAssetId`를 사용한다. Tool에서 보였다는 이유로 alias를 Product 승인으로 간주하지 않는다. Product Preview와
Server Actual이 같은 stable asset ID를 resolve하는 검증과 사용자 육안 판정을 분리한다.

`VALTAN_FIST_IN_OUT` 도넛 자체의 재생 결함은 사용자 지시에 따라 이번 변경에서 제외한다.

## 3. 단일 occurrence 계약

Arena에서 재생 가능한 occurrence의 식별자는 다음 묶음이다.

```text
(revision, patternId, stageId, actionId, clipOccurrenceId)
```

이 identity에 다음 항목이 함께 닫혀야 한다.

1. clip source range, play rate, loop/hold policy와 stage wall
2. Server motion kind, subwindow와 X/Y/Z 축 소유자
3. canonical Product Effect cue, source time, anchor/follow/stop/repeat policy
4. Server hit, collider, world mutation과 branch outcome
5. snapshot에서 재생할 pattern sequence, stage index와 action start tick

generated `ValtanEncounter.json`, `Valtan.patternbindings.json` 또는 cue JSON을 직접 고쳐 한 lane만 맞추지 않는다.
`Valtan.gameplay.json + Valtan.presentation.json` joined source를 publish한다.

## 4. 중앙점프 후 포효 계약

`VALTAN_ARENA_BREAK_109`는 점프와 포효를 따로 승인하지 않고 다음 하나의 Server trace로 검증한다.

```text
TAKEOFF 900ms (0..900ms 상승)
  -> DROP 700ms (0..700ms 중앙 anchor 낙하)
  -> IMPACT (phase 2 + outer-wall event commit)
  -> IMPACT_HOLD
  -> WIDE_REVEAL (12_03 tail -> roar start -> roar loop)
  -> RECOVERY (roar end)
```

Server contract는 authored 중앙 anchor, apex, 네 subwindow 값, stage별 중간 Y와 IMPACT의 정확한 착지를 검사한다.
Animation Tool contract는 같은 stage/action 순서와 열 개 clip occurrence 전체를 검사한다. phase 2 commit과 wall event는
IMPACT ENTER 한 곳에만 존재해야 한다.

## 5. 이번에 받은 P2/P3 animation admission

현재 animation intake 20개는 `19개 P2 + 1개 P3 candidate`이며 모두
`MANUAL_SERVER_AUDITION`에서 Product `AUDITION_ONLY`로만 재생된다. 이름에 `success`, `fail`, `jump`, `dead`가 있어도
Server motion, hit, branch 또는 phase event를 추측하지 않는다.

`Project-ValtanPatternMaster.ps1 -Mode ValidateV2`는 다음 lineage를 검사한다.

- debug intake의 모든 `sourceChainId`가 manual audition에 정확히 한 번 분류됨
- 각 chain의 occurrence 수와 Product occurrence 수가 동일함
- `att_* -> mesh_att_*`로 승인된 exact alias 외에는 fuzzy clip fallback 없음
- clip 순서, source offset, mapping basis와 play rate가 joined Product와 동일함
- target pattern/stage를 기입했다면 둘이 함께 존재하고 실제 Product stage에 exact-join함
- Product stage의 explicit wall/loop/hold budget이 Server duration과 일치함

새 chain이 분류되지 않았거나 한 clip이라도 다른 이름을 가리키면 publish가 실패한다. 이 admission은 animation
lineage만 승인한다. gameplay를 채우고 실제 Server audition과 사용자의 시각 확인을 끝낸 뒤에만 phase selection 또는
mechanic으로 승격한다.

## 6. 필수 검증과 수동 경계

자동 검증은 최소 다음을 포함한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python -B Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py
python -B Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
Server.exe --contract-test
```

자동 PASS는 화면 품질 PASS가 아니다. Agent는 Client를 실행·조작하지 않는다. Server와 Client를 재빌드·재시작한 뒤
사용자가 첫 휠윈드, 연속 돌진, 벽 충돌 즉시 파괴/그로기, HIGH_JUMP, 중앙점프→포효와 P2/P3 manual Server audition을
직접 확인한다.
