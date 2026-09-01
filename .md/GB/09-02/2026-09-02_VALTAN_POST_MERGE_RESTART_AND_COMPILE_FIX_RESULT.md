# 발탄 저장 Flow 재시작·교차 패턴·main 컴파일 복구 결과

작성일: 2026-09-02

대응 계획서:
`../08-30/2026-08-30_VALTAN_RESTART_FLOW_EFFECT_ANCHOR_SEQUENCER_IMPLEMENTATION_PLAN.md`

## 1. 실제 완료 상태

이번 변경은 직전 통합 뒤 드러난 발탄 저장·재시작 회귀와 `main`의 Client 컴파일 차단을 다음 범위로
수정했다.

- `Restart Saved Flow`는 화면에 남은 이전 draft가 아니라 disk의 canonical saved baseline을 먼저
  다시 읽은 뒤 한 개 저장 패턴 여부와 Server-active revision을 검사한다.
- 미확정 재시도는 기존 pending request가 현재 저장된 exact one-slot Flow와 같은 경우에만 허용한다.
- Server는 Boss Tool 기본 Flow가 Server-active `scriptedSequence`의 mode, pursuit 간격, 패턴 수와
  순서를 정확히 투영한 경우에만 arena reset과 Pattern 01 시작을 commit한다. 재정렬되거나 오래된
  요청은 reset 전에 거부한다.
- managed Counter loader는 `COUNTER_HIT` 후속을 같은 패턴 안의 뒤쪽 Stage뿐 아니라 같은 Encounter의
  별도 follow-up 패턴 첫 Stage로도 해석한다. 따라서 `VALTAN_COUNTER/STEP_02 ->
  VALTAN_COUNTER_GROGGY`와 dash groggy/part-break 분리 패턴을 canonical bootstrap에서 수용한다.
- Client master reader는 cross-pattern follow-up의 unknown/self/cycle/depth 초과와 일반 selector에
  들어가는 잘못된 follow-up 대상을 거부한다. follow-up은 untargeted, health bar 0/0,
  selection weight 0의 `AUDITION_ONLY` 패턴이어야 한다.
- Effect Tool에서 Independent Effect를 삭제하고 Save했을 때 direct-authored Product를 다시 읽는 조건을
  실제 EffectCatalog 등록 여부로 통일했다. combat object의 spawn/hit Effect도 Product mapping index에
  포함한다.
- 직전 Kakul PR `b5a65545`가 존재하지 않는 `WorldSequencePlayer.h/.cpp`를 프로젝트와 Level 소비부에만
  등록해 `main` Client 빌드를 끊은 사실을 확인했다. Git 전체 이력, remote/local branch, 등록 worktree와
  디스크 어디에도 정본 구현이 없으므로 빈 stub을 추가하지 않고 그 미완성 Client 소비부만 직전 컴파일
  가능 상태로 되돌렸다. stage teleport, Shared/Server protocol, publisher와 world-sequence 저작 데이터는
  유지했다.

피자 회전/플레이어 anchor, 3페이즈 망령화와 사각 포탈 반복은 현재 정본 데이터와 Server runtime에 이미
구현되어 있어 이번 변경에서 다시 작성하지 않았다. V1 decal normal-cut도 기존 receiver geometric normal
수정이 정본에 있으므로 shader를 추가 변경하지 않았다.

## 2. 검증 결과

실행한 정본 빌드:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Debug -Profile Product
```

결과:

- Engine, Shared, Server, Client Debug x64 compile/link PASS
- `Client/Bin/Debug/Client.exe` 생성 PASS
- Product compiled-shader closure PASS
- Effect WARP readback V1 1,352 pixels, V2 1,352 pixels
- build receipt:
  `out/BuildPipeline/receipts/product.debug.receipt.json`

추가 자동 검증:

```text
발탄/Effect focused Python suite                         118/118 PASS
World Sequence authoring unittest                       25/25 PASS
Project-ValtanPatternMaster.ps1 -Mode Validate          PASS, errors 0
Validate-EffectSources.ps1 -AllowLocalResources         PASS, direct 177 / unbound 0
Client Debug x64 단독 compile/link                       PASS
Client.vcxproj / Client.vcxproj.filters XML parse       PASS
git diff --check                                         PASS
```

`Server.exe --contract-test`는 이번 교차 Counter/Dash 로드와 관련 항목까지 진행·통과했지만 전체 종료값은
기존 다른 작업 범위의 6개 WIP assertion 때문에 아직 0이 아니다. 내역은 audition-only weight 1건,
spawn-group 3건, audition lifecycle 1건, Flow Stop 1건이다. 이번 Product compile/link 성공과 관련된
새 오류는 아니다.

## 3. 저장·재시작 계약

`Restart Saved Pattern`은 다음 순서로 동작한다.

1. unresolved pending request가 있으면 현재 저장본과 exact match인지 확인한다.
2. 새 요청이면 disk의 canonical Flow를 reload한다.
3. one-slot 저장본과 exact Server-active gameplay revision을 확인한다.
4. Server가 active `scriptedSequence`와 동일한 요청인지 다시 검증한다.
5. 검증이 모두 끝난 뒤에만 fresh arena reset과 저장 Pattern 01 시작을 commit한다.

따라서 화면 draft 삭제/재정렬만으로 오래된 Flow를 재생하거나, 불일치 요청이 벽·HP·cooldown을 먼저
초기화하는 경로는 차단된다.

## 4. 수동 검증 상태와 남은 경계

- Client 실행·UI 조작·화면 캡처: 미실행
- `Restart Saved Flow`, Independent Effect 삭제 후 Save, decal/피자 visual fidelity: 사용자 확인 필요
- Kakul world-sequence 제품 player는 이번 hotfix에서 새로 구현하지 않았다. 현재 저작 문서와 protocol은
  보존됐지만 실제 runtime player는 별도 수직 슬라이스에서 parse/validate/stage/commit, 동시 재생,
  seek/final pose와 rollback harness까지 함께 구현해야 한다.
- 빌드가 생성한 untracked runtime `LV_LUT_MIDNIGHTC_ED.worldsequences.json`은 정본 source가 아니므로
  commit 대상에서 제외했다.
