# Boss Tool Pattern Flow 저작·서버 검증 결과

## 0. 결론

구현과 자동 검증은 완료했다. 사용자 육안 검증은 아직 끝나지 않았으므로 visual PASS는 아니다.

`F1 -> Boss Tool`은 다음 두 탭으로 분리했다.

```text
Boss Verification | Pattern Flow
```

- `Boss Verification`은 기존 All Effects 기준 28개 Valtan Pattern 목록, `Play Selected`, `Repeat`,
  `Stop After Current`, 사망 시 `Revive Player`, 우측 연결·진단 화면을 그대로 소유한다.
- `Pattern Flow`는 같은 28개 inventory에서 슬롯을 추가·이동·삭제하고 저장한 뒤 `Preview Isolated`,
  `Start First`, `Start Here`, `Stop After Current`, `Revive Player`로 실제 Server Pattern 상호작용을 검증한다.
- `Start Here`는 선택 슬롯부터 끝까지의 suffix를 새 phase-1 audition reset에서 실행한다. 이전 슬롯이 만든
  월드 상태를 합성하는 checkpoint가 아니므로, 앞 Pattern의 파괴·HP·상호작용 누적까지 검증할 때는
  `Start First`를 사용한다.

검증 Flow는 `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`에 저장하며 Product 자동 전투 정본인
`Data/Valtan/Valtan.gameplay.json / decisionModel.scriptedSequence`를 수정하지 않는다. 따라서 순서 튜닝은
publish 없이 빠르게 반복할 수 있고, 검증 완료 Flow를 제품 순서로 승격하는 작업만 별도 명시적 publisher
변경 단위로 남는다.

## 1. 구현 상태

### 1.1 Flow 문서와 저장

- schema/version/root property를 exact 검사한다.
- `flowId`와 각 `slotId`는 stable ID이며 vector index를 저장 ID로 쓰지 않는다.
- 최대 32슬롯, 100..10000ms pursuit 범위를 검사한다.
- 같은 Pattern을 여러 슬롯에 넣을 수 있지만 `slotId` 중복은 거부한다.
- All Effects/Boss Verification이 승인한 현재 28개 inventory에 없는 Pattern은 load, reload, save, start에서
  거부한다.
- raw file bytes의 SHA-256을 revision으로 사용한다. Save는 source revision CAS, durable temp flush,
  atomic replace, post-commit verify와 backup rollback을 수행한다.
- Reload/Save 실패는 현재 admitted draft와 baseline을 유지한다.
- Start 직전에 실제 disk revision과 현재 graph inventory를 다시 확인한다. 외부 편집 또는 graph drift가 있으면
  실행하지 않고 Reload를 요구한다.

### 1.2 Tool UI

- 기존 화면을 `Render_BossVerificationTab()`으로 옮겼고 동작과 우측 diagnostics를 제거하지 않았다.
- Flow 탭의 Pattern 추가 목록은 별도 ID 배열이나 Product `scriptedSequence`가 아니라 기존
  `m_AuditionInventory`만 소비한다.
- `Preview Isolated`는 기존 `Play Selected`와 같은 `CValtanPatternAuditionService::Submit()` 경로를 쓴다.
- ordered Flow는 single-pattern 요청을 Client에서 반복하지 않고 `CValtanPatternFlowService`가 저장된 전체
  Flow와 start slot ID를 한 번 typed 제출한다.
- Tool authoring 상태, Server playback 상태, isolated preview 상태를 별도 행으로 표시한다. ordered Flow가
  실행 중일 때는 과거 isolated preview 완료 문구를 숨긴다.
- Flow 실행 중에는 Add/Move/Remove/Save/Reload와 기존 단일 Pattern 재생을 잠가 화면 revision과 Server 요청을
  일치시킨다.
- Stop은 flowId/epoch에 묶인 one-shot 요청이며 verdict timeout과 lifecycle timeout을 분리했다.
- world generation이 바뀌면 이전 room의 늦은 result/lifecycle이 중단된 Flow를 되살리지 못한다.

### 1.3 typed protocol과 Server runtime

- network protocol version을 39로 올리고 start/result/stop/lifecycle packet을 추가했다.
- Start는 boss placement, flow ID, 64자리 source revision, start slot, pursuit와 모든 ordered
  `slotId=patternId`를 포함한다.
- 같은 request sequence의 재전송은 전체 fingerprint가 같을 때만 duplicate다. 일부 identity만 같은 요청은
  stale로 거부한다. Stop도 control sequence + flowId + room epoch exact identity를 사용한다.
- Server는 boss/player/catalog/bait/range/world destruction/encounter prop을 모두 preflight한 뒤 audition reset을
  정확히 한 번 commit한다.
- 선택 start slot inclusive suffix만 임시 `ORDERED_ONCE_THEN_IDLE` sequence로 만들고 기존
  `CValtanBrain` ordered runtime에 override로 전달한다. 별도 Client slot loop나 두 번째 Pattern runtime은 없다.
- Pattern 사이에는 boss HP, player, wall/prop/destruction 상태를 reset하지 않는다.
- death 중 현재 occurrence clock/cursor를 멈추고 revive 뒤 같은 slot을 이어 간다.
- 자연 완료는 `COMPLETED_HOLD`, Stop은 current occurrence 뒤 `STOPPED_HOLD`이며 Product 자동 Pattern으로 새지
  않는다.
- 시작 gameplay revision을 pin하고 lifecycle에 request/flow/revision/start slot/current slot/pattern sequence를
  함께 보낸다.
- Release Server는 packet type은 알지만 Start와 Stop을 명시적으로 거부한다.

### 1.4 Product sequence drift 정리

현재 Product `scriptedSequence`는 Dash를 포함한 28개다. 이전 Boss Tool 하네스의 27개와 3시/9시 고정
index 13/14 가정을 제거하고, 지형 파괴 Pattern은 stable pattern ID로 찾게 했다. Flow 문서는 Product 순서의
count/index 정본이 아니며 현재 Product JSON을 임의 교정하지 않는다.

### 1.5 All Effects 트리 회귀 방지

- 이번 작업은 인계받은 `ValtanPatternTree.h/.cpp`와 pattern-tree contract를 patch하지 않았다.
- 현재 strict join의 `partDamagePolicy`, `counterProxy`, previous-tree transaction 보존을 focused contract에서
  다시 확인했다.
- `.md/GB/gotchas.md`에 `피자 패턴 바닥 이펙트 가이드`에서 확인된 오래된 worktree 전체 파일 덮어쓰기
  회귀, 금지 파일 복사 방식, 필수 회귀 명령과 수동 28개 tree 확인 절차를 기록했다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| Pattern Tree + All Effects + Boss Tool + Pattern Flow 결합 회귀 | PASS, 66 tests |
| Pattern Flow focused document/UI/protocol/Server static contract | PASS, 17 tests |
| NetworkProtocolHarness Debug | PASS, `failures : 0` |
| NetworkProtocolHarness Release | PASS, `failures : 0` |
| Client x64 Debug `ClCompile` | PASS, 최종 UI 정리 포함 |
| Client x64 Release full build/link | PASS, `Client/Bin/Release/Client.exe` |
| Server x64 Debug 별도 출력 build | PASS |
| Server Debug 신규 Flow 실행 계약 2개 | PASS |
| Server Debug 전체 contract | 신규 Flow 외 기존 무관 1건만 실패 |
| Server x64 Release 별도 출력 build + 전체 contract | PASS, `failures : 0` |
| Flow JSON / Client project·filters XML parse | PASS |
| Server owned files 및 전체 dirty worktree `git diff --check` | 오류 없음, 기존 line-ending warning만 존재 |
| Client 독립 코드 재감사 | 이전 P1/P2 다섯 항목 해소, 새 안전성 회귀 없음 |

Debug Server 전체 contract의 기존 무관 실패 문구는 다음과 같다.

```text
Reset a missing or retired candidate to packaged idempotently, reject corrupt durable state without mutation, and refuse a concurrent Server owner
```

Client build에는 기존 source code-page C4819와 DirectXTK PDB LNK4099 warning이 있었지만 compile/link는
성공했다. 실행 중인 `Client/Bin/Debug/Client.exe`와 `Server/Bin/Debug/Server.exe`는 사용자의 다른 수동 검증
세션이므로 종료하거나 새 바이너리로 교체하지 않았다. 이 때문에 최종 Debug full link는 해당 세션 종료 뒤
한 번 더 수행해야 하며, 현재 최종 소스의 Debug C++ compile과 Release full link는 통과했다.

## 3. 사용자 수동 검증 순서

현재 실행 중인 수동 세션을 끝낸 뒤 최신 Debug Server + Debug Client를 사용자가 직접 시작한다.

1. `F1 -> Effect Tool -> All Effects -> Valtan`을 연다.
   - 28개 Pattern과 각 Stage tree가 비지 않고 보여야 한다.
2. `F1 -> Boss Tool -> Boss Verification`을 연다.
   - 기존 목록, `Play Selected`, `Repeat`, `Stop After Current`, 사망 시 `Revive Player`, 우측 diagnostics가
     기존과 같아야 한다.
3. `Pattern Flow` 탭을 연다.
   - Add/Up/Down/Remove, duplicate Pattern 슬롯, Save/Reload를 확인한다.
   - 저장하지 않은 draft는 isolated preview만 되고 Start는 비활성화되어야 한다.
4. 임의 중간 슬롯을 선택해 `Start Here`를 누른다.
   - boss/player가 중앙 검증 위치로 한 번만 reset되고 선택 슬롯부터 마지막까지 이어져야 한다.
5. `Start First`로 파괴·HP 변화가 있는 Flow를 실행한다.
   - 각 Pattern 사이에 boss/player/wall/prop 상태가 유지되고 Product Pattern이 끼지 않아야 한다.
6. Pattern 도중 player가 죽으면 같은 탭의 `Revive Player`를 누른다.
   - 죽은 동안 멈추고 같은 slot/stage에서 한 번만 이어져야 한다.
7. `Stop After Current`를 누른다.
   - 현재 Pattern은 끝나고 다음 슬롯은 시작하지 않은 채 hold해야 한다.

All Effects tree, UI layout/clipping, 돌진과 피자 바닥 Effect, 실제 animation/effect fidelity는 사용자의 육안
확인이 필요하다. 자동 테스트만으로 visual PASS를 기록하지 않는다.

## 4. 남은 경계

- 검증 Flow 저장은 즉시 Server audition에 쓰이며 Product publish를 요구하지 않는다.
- 검증된 Flow를 Product `scriptedSequence`로 승격하는 command는 이번 범위에 넣지 않았다. 두 정본을 자동
  동기화하면 실험 중 순서가 제품 자동 전투에 섞이므로, 승격은 diff/validation이 보이는 별도 publisher가
  소유해야 한다.
- late lifecycle 순서는 현재 service guard와 정적 focused contract로 고정했다. 향후 network queue를 직접
  주입하는 Client service 실행형 harness를 추가하면 회귀 증거를 더 강화할 수 있다.
- 대규모 공동 dirty worktree이므로 commit/push는 수행하지 않았다.
