# Action Presentation Workbench 1차 수직 슬라이스 구현 결과

## 1. 완료 상태

구현 완료:

- F1 `Animation Tool` 진입과 창을 `Action Presentation Workbench`로 승격
- `VALTAN_HIGH_JUMP / AIRBORNE`를 포함한 managed Valtan stage의 Server wall/blank timeline 편집
- 동일 `CBalanceTool` draft를 통한 `Validate Joined`, `Save Gameplay Authoring`, `Publish Candidate`, `Apply Revision`
- 선택 pattern의 Animation, Product Effect, Sound event/WAV variant, Server combat-object Effect/hit clock joined lane
- Product Effect stable occurrence의 기존 Effect Tool deep-link
- 기존 `CBossTool`/`CValtanPatternAuditionService`를 통한 `Server Replay / Live`
- 기존 custom chain 창을 `Animation Sequence Intake`로 명확히 분리
- Workbench 구조 계약 5개 추가

자동 구현하지 않은 경계:

- KakulSaydon은 저장소에 Product world/encounter/pattern/Server runtime이 하나도 없어 local mock을 만들지 않았다.
- generated `Valtan.patternsoundcues.json`과 `CharacterSoundCatalog.json`의 저작 저장 경로는 아직 read-only다. 이번 단계는 기존 sound event가 해석하는 실제 WAV asset을 표시·청취한다.
- 새 Animation Intake를 Product pattern으로 자동 승격하지 않는다. 기존 manifest review, WModel duration freeze, gameplay/presentation 원자 projection을 유지한다.
- Client 화면과 음향은 에이전트가 실행·판정하지 않았다.

## 2. 발탄 도끼 투척 실측과 현재 진단

사용자가 말한 도끼 투척은 별도 `VALTAN_AXE_THROW`가 아니라 다음 경로다.

- pattern: `VALTAN_HIGH_JUMP`
- stage: `AIRBORNE`
- animation: `mesh_att_battle_8_01_loop`, `LOOP_TO_STAGE_END`
- Server combat object: `combatobject.valtan.high-jump.target-axe`
- combat-object hit: spawn 후 1,200 ms
- Effect: `effect.valtan.sky-axe.active`

AIRBORNE stage duration을 늘리면 Server wall clock이 늘고 loop animation이 같은 stage 끝까지 반복한다. Workbench는 saved source와 unsaved draft 시간을 동시에 표시한다.

현재 Sound 문서는 TAKEOFF 3개와 LAND 3개를 가지지만 AIRBORNE는 0개다. Workbench joined lane은 combat-object `+1200 ms` hit와 Sound 0개를 같이 보여 주고 다음 오류를 표시한다.

`COVERAGE GAP: Server combat-object hit exists, but this stage has no Sound cue.`

이는 누락을 숨기지 않지만 아직 소리를 임의의 boss clip cue로 잘못 연결하지도 않는다. 정확한 후속 구현은 combat-object hit occurrence가 소유하는 Sound invocation schema와 Client replicated occurrence 소비다.

## 3. 저장과 실행 원리

| 명령 | 결과 |
|---|---|
| `Server wall / blank timeline ms` | 같은 Balance draft의 `SET_STAGE_DURATION(patternId, stageId, durationMs)` 입력 변경 |
| `Validate Joined` | split gameplay/presentation join과 stable-ID patch 검증 |
| `Save Gameplay Authoring` | immutable authoring revision 저장; 성공 뒤 Workbench joined view reload |
| `Publish Candidate` | runtime candidate 생성, active revision 불변 |
| `Apply Revision` | `HOT_RELOAD` candidate만 기존 two-phase Server transaction으로 제출 |
| `Pattern Offline` | Product Valtan animation local sample; gameplay/hit/damage 없음 |
| `Server Replay / Live` | Boss Tool이 current audition inventory에서 pattern을 재해석한 뒤 실제 Server command 제출 |
| `Save Animation Intake` | `Data/Valtan/Valtan.presentation.debug.json` 저장; Product 아님 |

## 4. 변경 파일

- `Client/Private|Public/Animation_Tool.*`
- `Client/Private|Public/BalanceTool.*`
- `Client/Private|Public/BossTool.*`
- `Client/Private/MainApp.cpp`
- `Client/Private|Public/ValtanPatternTree.*`
- `Tools/ValtanPipeline/test_action_presentation_workbench_contract.py`
- `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- 대응 PLAN/RESULT

새 C++ 파일은 없어 vcxproj/filter 등록 변경은 없다.

## 5. 자동 검증

PASS:

- `python -B Tools/ValtanPipeline/test_action_presentation_workbench_contract.py` — 5 tests
- `python -B Tools/ValtanPipeline/test_valtan_boss_tool_contract.py` — 21 tests
- `python -B Tools/ValtanPipeline/test_valtan_balance_tool_contract.py` — 26 tests
- `python -B Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py` — 26 tests
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` — managedPatterns 33, combatObjects 3, errors 0
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` — Engine/Shared/Server/Client build와 compiled shader closure PASS
- `git diff --check` — whitespace error 없음; 기존 line-ending warning만 출력

Product build의 기존 C4819/LNK4099 warning은 남아 있으나 새 compile/link error는 없다.

## 6. EffectRender와 Intermediate 정리 결과

`EffectRenderContractHarness` tracked source는 아직 존재하지만 Product/Core/FullDiagnostic 어디에서도 빌드·실행하지 않아 현재 정상 빌드 시간 비용은 0이다. 삭제 전 이관되지 않은 resource-root, load/prewarm rollback, WModel sample, WARP renderer assertion이 있어 source를 보존했다.

삭제 가능한 무효 출력은 정확히 확인했다.

- `Tools/EffectRenderContractHarness/Bin`: 252,126,138 bytes
- `Tools/EffectRenderContractHarness/Intermediate`: 661,828,258 bytes
- `Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.user`: 711 bytes

호스트 실행 정책이 재귀 삭제 명령을 거부해 실제 삭제는 수행되지 않았다. 제품 파일이나 tracked source도 삭제되지 않았다.

## 7. 사용자 수동 smoke

1. 팀 Server PC에서 Server를 실행한다. 현재 이 PC는 LAN sync 결과 `client`, endpoint `10.207.18.103:7777`은 검사 당시 `not-listening`이었다.
2. 이 PC에서 `Client`를 `Ctrl+F5`, 작업 디렉터리 `Client/Default`로 실행한다.
3. Valtan Arena 진입 후 F1 → `Action Presentation Workbench`를 연다.
4. Valtan target, `VALTAN_HIGH_JUMP`, `AIRBORNE`를 선택한다.
5. `Server wall / blank timeline ms`를 원하는 값으로 늘리고 `Validate Joined` → `Save Gameplay Authoring`을 누른다.
6. 필요하면 `Publish Candidate`; apply class가 `HOT_RELOAD`일 때만 `Apply Revision`을 누른다.
7. `Pattern Offline`에서 loop/landing 연결을 보고, joined lane에서 도끼 Effect와 `+1200 ms` Server hit, AIRBORNE Sound coverage gap을 확인한다.
8. `Server Replay / Live`를 눌러 실제 Arena에서 boss movement, axe spawn/hit/damage와 최종 타이밍을 판정한다.

시각·음향 PASS는 이 사용자 관찰 뒤에만 기록한다.

