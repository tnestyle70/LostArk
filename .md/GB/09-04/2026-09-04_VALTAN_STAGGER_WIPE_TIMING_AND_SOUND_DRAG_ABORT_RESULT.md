# 2026-09-04 발탄 마력구 전멸 타이밍·Sound 드래그 abort 수정 결과

## 1. 완료 상태

- Boss Tool 화면의 초록색 `G_Voltan2_Attack25_Shot2 [once]` box는 Pattern Sound cue가 맞다.
- `VALTAN_STAGGER_SLOT`의 `FINAL_ATTACK` 기준 Effect V2 두 occurrence, Sound, Server 권위 전멸 hit를 모두 stage-local `1000ms`로 정렬했다.
- `CHANNEL`이 `12000ms`이므로 전체 타임라인의 실제 동시점은 `13000ms`다.
- Sound body drag 뒤 발생하던 Debug CRT `abort()`의 문자열 수명 오류를 수정했다.
- 정본 JSON과 런타임 생성물을 publish했고 Debug `FullDiagnostic`까지 통과했다.
- 사용자는 값을 다시 옮기거나 Save할 필요가 없다. 새 빌드에서 패턴을 재생해 시청각 결과만 확인하면 된다.

## 2. 실제 변경

### 2.1 타이밍 정본

```text
Data/Valtan/Valtan.gameplay.json
  FINAL_ATTACK hit.schedule.firstOffsetMs = 1000

Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
  boss.valtan.twohand startMs = 1000
  boss.valtan.six.sonic startMs = 1000

Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json
  G_Voltan2_Attack25_Shot2 startMs = 1000
```

재생성 helper와 coverage/alignment oracle도 같은 `1000ms`를 사용하도록 바꿨다. 기존 Sound
binding/occurrence ID의 `impact-2900` 접미사는 stable identity이므로 이름만 유지했다.

### 2.2 abort 근본 원인

`CAnimation_Tool::Patch_ValtanCompositionPatternSound()`는 호출자가 Sound owner vector 안의
`occurrenceId`와 event 문자열을 참조로 넘길 수 있는데, staged document를 owner에 move-commit한
뒤 그 참조를 성공 status에 다시 사용했다. owner 교체로 vector/string 저장소가 무효화된 뒤의
참조 접근이므로 FMOD 문제가 아니라 use-after-invalidation이었다.

함수 진입 시 두 문자열을 값으로 복사하고, 검색·candidate 작성·commit 뒤 status까지 stable copy만
사용하도록 수정했다. source contract는 owner commit 이후 원래 참조가 다시 사용되지 않는 것을
검증한다.

### 2.3 과거 Save 실패 로그와 현재 경로

`Intermediate/Logs/ValtanAuthoringSave/job-47012-1-88569015`는 11:45에 실행된 Client와 이후 writer
script가 섞인 별도 버전 불일치였다. job에는 V2 baseline/candidate만 있고 read-set이 없었으며
`canonicalCommitted=false`, `changedCount=0`이므로 정본을 전혀 바꾸지 않았다. 이 사건은 화면의
`abort()` 원인이 아니다.

현재 소스는 Workbench에서 baseline/candidate/read-set을 함께 전달하고 writer lock 안에서 JSON을
atomic commit한다. 최신 Client의 정상 계약은 `드래그 -> Save -> 정본 JSON 저장`이며 별도 Reload
순서를 요구하지 않는다. 불가능한 구버전 payload가 다시 들어오면 Reload를 오안내하지 않고
Client 재빌드/재시작과 draft 보존을 명시하도록 진단도 교정했다.

## 3. Publish와 검증

- 집중 Sound/atomic-save/timing 계약: PASS (`53/53`).
- Valtan pattern master 단독 회귀: PASS (`70/70`, `387.394s`).
- `Run-FullPipeline.ps1 -DataOnly`: PASS (`30.6s`).
- 최종 `Run-FullPipeline.ps1 -Configuration Debug -Profile FullDiagnostic`: PASS (`701.8s`).
  - Engine, Shared, Server, Client compile/link PASS.
  - Valtan master `70/70` PASS.
  - Server gameplay contract에서 wipe 직전 `966.67ms` 무피해와 `1000ms` 전원 HP 0을 확인했다.
  - Network/Character Select Core 및 Party2/Party4, presentation/render/physics/model harness PASS.
  - compiled shader closure PASS, WARP draw/readback `V1=1352`, `V2=1352`.
  - evidence: `out/BuildPipeline/runs/20260904T144007362Z-debug-fulldiagnostic-ecbb5b94.json`.
- source revision: `704c33cd345e06dc40ec41f142b65d0fe016d0ec01e89d41d7abeae3365ad953`.
- 정본 수치 재파싱: gameplay=`1000`, V2 twohand=`1000`, V2 six.sonic=`1000`, Sound=`1000`, absolute=`13000`; JSON parse PASS.
- `git diff --check`: PASS. 기존 working-copy line-ending 경고만 출력됐다.

동일 CAS/hard-crash suite를 병렬 실행해 writer lock을 선점했던 최초 FullDiagnostic 실패는 코드 결과로
세지 않았다. 해당 프로세스를 종료한 뒤 단독 master와 최종 FullDiagnostic을 다시 실행해 모두
통과했다.

## 4. 이후 타이밍 편집 파이프라인

순수 Effect/Sound timing 편집의 저장 본체는 정본 JSON commit이다. Workbench에서 `Save`하면 바로
정본에 저장되고, `Publish after Save`는 이어서 런타임 배포까지 할지 고르는 옵션이다.

```text
반복 편집: 드래그 -> Save -> focused validation
런타임 반영: Publish after Save 또는 RunFullPipeline.bat -DataOnly -> Server 재시작
최종 변경 묶음: RunFullPipeline.bat -Configuration Debug -Profile FullDiagnostic 한 번
```

JSON을 직접 수정한 경우에도 별도 publisher들을 연속으로 수동 호출하지 않고 `-DataOnly` 한 번을
사용한다. 같은 domain은 receipt cache가 재사용한다. `Product -> Core -> FullDiagnostic`을 각각
연속 실행하지 않고 최종에는 상위 profile 하나만 실행한다. Pattern/Stage/ID/순서를 바꾸지 않은
순수 timing 수정에는 별도 shadow sync가 필요 없다.

이번 변경은 Client C++ abort 수정과 Server hit 의미도 포함하므로 publisher만으로 끝낼 수 없었고
최종 Client 재빌드와 FullDiagnostic이 필요했다.

## 5. 수동 확인 대기와 작업 경계

- LAN 설정은 `server-host`, Server `0.0.0.0:7777`, Client `192.168.0.4:7777`이며 현재 endpoint
  probe의 `not-listening`은 Server가 꺼진 상태일 뿐 설정 실패가 아니다.
- 사용자는 `Server + Client` profile을 직접 시작하고 `Lobby -> Valtan -> F1 -> Boss Tool -> 마력구
  파괴 패턴`을 재생해 전체 `13000ms`에서 이펙트·사운드·전멸이 맞는지만 확인한다.
- Server는 published bootstrap을 읽도록 재시작해야 한다.
- 에이전트는 Client/UI를 실행하거나 시각·청각 PASS를 대신 판정하지 않았다.
- 기존 Kakul/Map/Composition dirty 변경을 보존했으며, 소유권이 섞인 worktree라 stage/commit/push하지 않았다.
