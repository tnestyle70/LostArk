# 발탄 애니메이션 우선 패턴 승격 결과

## 1. 완료 범위

애니메이션 담당자의 `Valtan.presentation.debug.json` 20개 chain, 94개 occurrence를
2·3페이즈 패턴 저작의 시작 정본으로 승격했다. 한 occurrence를 한 Server stage로 고정해
다음 stable ID 사슬을 split authoring과 Product가 같이 소비한다.

`patternId -> stageId -> actionId -> clipOccurrenceId -> durationMs`

20개 중 19개는 2페이즈, 망령화 형태를 참고한 마지막 1개는 `P3 Candidate`로 표시했다.
3페이즈 모델 교체와 실제 gameplay transition은 아직 확정하지 않았으며 별도 수직 슬라이스로
남겼다.

## 2. 애니메이션 저작 승격 계약

- `Valtan.animation-chain-promotions.json`이 debug chain과 stable pattern ID, phase,
  admission 상태의 명시적 매핑을 소유한다.
- `promote_valtan_animation_chains.py`가 debug 문서, promotion manifest와 실제 WModel clip
  table을 strict validate한 뒤 split authoring 두 문서와 receipt를 transactionally 생성한다.
- `playMs=0`인 71개 occurrence는 현재 WModel native duration을 receipt와 split source에
  고정했다.
- native clip보다 긴 명시 duration은 `LOOP_TO_STAGE_END`로 투영해 Server stage clock과
  Client animation wall time을 일치시켰다.
- debug 이름에 `mesh_`가 빠진 6개 clip은 manifest에 정확한 alias를 명시했고 암묵적
  fallback은 두지 않았다.
- 기존 live 33개 패턴과 131개 stage의 의미는 유지하면서 Product에는 총 53개 패턴과
  225개 stage를 생성한다.

## 3. All Effects와 Server 실행 경계

신규 20개 패턴은 Product에서 `AUDITION_ONLY`로 publish된다. hit, damage, motion, event,
branch와 자동 rotation 수치를 애니메이션만 보고 추측하지 않았고 모두 fail-closed 값으로
시작한다.

- All Effects는 신규 행을 `Manual Audition | P2/P3`와 `[P2 Animation]` 또는
  `[P3 Candidate]` 이름으로 표시한다.
- Effect Tool은 같은 managed pattern/stage/action/occurrence를 읽으므로 pattern 담당자가
  곧바로 cue와 Effect를 저작할 수 있다.
- Server normal/ordered/weighted/health selector는 `AUDITION_ONLY`를 후보로 사용하지 않는다.
- Debug의 stable pattern ID 강제 요청은 기존 Server pattern execution 경로로 그대로 실행한다.
- 패턴 담당자가 gameplay 의미를 확정한 뒤에만 normal 또는 health-bar admission과 실제
  피해·이동·event를 별도 검증 변경에서 승격한다.

Balance Tool은 split authoring이 소유한 27개만 managed로 취급하고 기존 Product-only 26개는
read-only legacy로 유지한다. manual audition의 Server Play와 hit/cue 저작은 열어 두되,
selection/repeat/range와 animation occurrence가 소유하는 stage duration은 UI와 draft에서
fail-closed로 잠근다.

## 4. 애니메이션 담당 후속 절차

애니메이션 담당자는 Animation Tool의 `Valtan Custom Chain`에서 새 3페이즈 chain을 저장한다.
그 뒤 promotion manifest에 phase, stable pattern ID, display name과 reviewed debug/model hash를
추가하고 projector Validate/Apply를 실행한다. 기존 1·2페이즈 pattern ID를 재사용하거나
Product JSON을 직접 편집하지 않는다.

promotion 수량은 manifest/debug의 reviewed closure에서 계산되므로 다음 chain 추가를 현재
20/94 고정 수치가 막지 않는다. 반대로 현재 `manualAuditions`에서 제거된 pattern이 이전
Product에 남아 있으면 projection 전에 실패하고, projection 결과의 `AUDITION_ONLY` ID도 현재
manual inventory와 exact join해야 한다.

## 5. 자동 검증

- animation-chain promotion fixture와 rollback: 6/6 PASS
- Valtan V2 pipeline 전체 회귀: 39/39 PASS
- Valtan pattern tree: 17/17 PASS
- Animation Tool Valtan contract: 8/8 PASS
- gameplay publisher Validate: 53 patterns / 225 stages,
  그중 20 audition patterns / 94 audition stages PASS
- Server alternate Debug build와 신규 audition focused contract: PASS
- 원래 worktree Client 전체 `ClCompile` target: PASS
- 최종 clean source에서 `Animation_Tool.cpp`, `BalanceTool.cpp` compile: PASS
- Client project/filter XML parse, Python compile, JSON parse와 `git diff --check`: PASS

사용자가 실행 중인 `Client.exe`가 원래 worktree의 Debug 출력 파일을 점유해 Client의 최종 link는
`LNK1104`로 중단됐다. 실행 중인 Client는 종료하지 않았다. 최종 clean worktree의 전체
`ClCompile` 재실행은 변경 TU 두 개가 통과한 뒤 빠른 PR 인계 요청에 따라 중단했다.
같은 이유로 실행 중인 user-owned Server의 concurrent-owner sentinel을 우회하거나 Server를
종료해 전체 contract를 다시 실행하지 않았다.

전체 Effect occurrence inventory check에는 이번 신규 94개 행과 무관한 기존
`VALTAN_FLOOR_WIPE_130` reviewed clip-name mismatch가 남아 있다. 신규 audition occurrence와
기존 131개 binding의 보존 검사는 통과했다.

Client 화면의 애니메이션 연결감, Effect 타이밍과 visual fidelity는 사용자가 직접 확인하기
전까지 PASS로 기록하지 않는다.

## 6. 보존한 로컬 변경

원래 worktree에서 동시에 진행 중인 `Data/Effects/Authored` Effect JSON과 HIGH_JUMP mixed-axe
코드·데이터·계획서는 이 변경의 입력·출력이 아니다. 최신 main의 별도 clean worktree에서
이번 수직 슬라이스만 재구성해 commit과 PR에서 모두 제외했다.
