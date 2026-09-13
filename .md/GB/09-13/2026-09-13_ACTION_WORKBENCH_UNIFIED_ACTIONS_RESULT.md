# Action Workbench 통합 Composition Actions 결과

작성일: 2026-09-13. 계획 정본은 `2026-09-13_ACTION_WORKBENCH_UNIFIED_ACTIONS_IMPLEMENTATION_PLAN.md`다.
현재 작업 브랜치는 `codex/kouku-donut-ball-motion`이며 다른 작업의 미커밋 변경을 보존했다.

## G00. 구현 상태와 제품 적용 상태

Composition Actions의 Boss / Character / Object / Sequence 선택을 하나의 Action Workbench 창에
연결했다. Effect V1/V2는 사용자 후속 요청에 따라 G09에서 독립 창으로 분리했다.
캐릭터의 독립 Collider → Logic → Result는 저작 데이터부터
publisher, Server caster/projectile 판정 소비자까지 반영했다.

초기 검증 당시 사용자는 실행 중인 도구에 편집이 있어 계속 작업해 달라고 요청했다. 따라서 기본
Client/Server EXE, 실행 프로세스, gameplay bootstrap을 교체하지 않고 아래 컴파일·링크와 CPU 검사는
모두 `out`의 별도 출력으로 수행했다. 이후 사용자가 직접 재빌드한 EXE의 시작 오류를 요청하여 G08의
정식 데이터 Publish와 Server/Client 실행까지 진행했다. 화면 입력과 시각 결과는 사용자 확인 대상이다.

## G01. 공통 창과 기존 세션

`Client/{Public,Private}/SequencerTool.{h,cpp}`가 공통 창을 소유한다. 표시 이름은
Composition Patterns → Composition Actions이며 ImGui의 기존 window ID는 유지했다.
`CompositionWorkbenchSession.h`의 target과 기존 pane 인터페이스로 다음 세션을 연결했다.

| 선택 | 실제 문서·재생·저장 owner |
|---|---|
| Boss | 기존 ValtanActionWorkbench와 KoukuSaydonActionWorkbench. Valtan 및 Kouku 관문·패턴 트리 |
| Character | 신규 CharacterActionWorkbench. 기존 skillbindings / animevents / HitShapes를 각각의 owner로 편집 |
| Object | 기존 WorldObjectTool, World Sequence 문서, CWorldSequencePlayer와 source Save/Publish |
| Sequence | 기존 독립 Sequence Composition 세션. Boss 편집 문서와 별개로 선택·저장 |
| Effect Tool V1 / V2 (독립) | F1의 개별 Open 버튼. 기존 native Resources, 상세 편집, Attach, Group, Preview, Save |

`MainApp.cpp`는 공통 shell을 한 번 렌더링하고 기존 Object/Sequence 진입 요청을 해당 target으로
연결한다. Effect V1/V2는 각 visibility로 독립 Render를 호출한다. 숨겨진 세션의 Publish 완료 Poll과
background 작업은 계속 소비한다. 대상 변경은 프레임
경계에서 이전 preview를 정리하고 입력 owner를 전환한다. 각 문서·초안·선택은 해당 세션에 남는다.

Boss와 Sequence의 관문 선택은 독립이다. World Level에서 exact box를 여는 typed 요청은 대상·관문을
먼저 정한 뒤 stable box ID를 선택한다. 공통 pane의 ImGui 선택 상태도 해당 target과 관문만 참조한다.

## G02. Character action과 Parent timeline

신규 `Client/{Public,Private}/CharacterActionWorkbench.{h,cpp}`는 PlayerSkills의 실제 여섯 class와
LMB, SPACE, ALT_V, Q/W/E/R 등의 inputSlot을 나열한다. Parent와 combo stage에서 Animation,
Effect, Sound, Camera Shake, Collider, Logic, Result 행을 같은 action 시간축으로 조회한다.
직접 쓴 skill ID나 가짜 combo stage 목록을 사용하지 않는다.

Animation은 실제 모델의 clip을 추가·교체·복제·삭제·재정렬하고 source start/length/rate를 편집한다.
순차 재생 계약을 유지하므로 가운데 drag는 순서를 바꾸고 양 끝 drag는 source 범위를 조절한다.
optional `clipOccurrenceId`는 최초 편집 세션에서 부여하고 저장 이후 유지한다. 기존 Animation Tool의
skillbinding 저장도 이 ID를 보존한다. 원본 모델 metadata와 source window를 검증한 후에만 class와
preview target을 교체하며, 실패하면 이전 문서와 모델을 유지한다.

재생은 기존 `CEffectAuthoringSequencer` backend를 재사용한다. Character는 별도 세션 상태를 가져
Effect sequence 초안을 덮지 않고, 공유 preview 모델의 시계는 기존 독점 소유 계약을 사용한다.
카테고리 전환 전의 clock은 다음 staging 후 유효 범위로 제한해 복원한다. 새 action/stage 선택은
0ms부터 시작한다. Level 변경 뒤에도 같은 Character의 dirty binding/cue owner만 자기 lock을 잠시
해제하여 모델을 복원하고 즉시 다시 잠근다. 다른 도구의 dirty lock은 유지한다.

## G03. 실제 저장 경계와 cue 편집

| 명령 | 저장 대상과 소비자 |
|---|---|
| Save Animation | 기존 `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`; Character presentation |
| Save Combat | 기존 `Data/Animation/HitShapes/<Asset>.hitshapes.json`; Gameplay publisher → Server |
| Edit source cues → 기존 Events Save | 기존 `.animevents`와 Animation Tool의 reader/writer |
| Edit Effect resource → 해당 owner Save | 기존 V1/V2 Effect 문서 |

cue 상세 화면을 보는 것만으로 모델 clip을 바꾸지 않는다. 명시적인 Edit source cues에서 action
preview를 멈추고 source clip을 선택한다. Play/Seek는 action 시계를 다시 사용한다. cue 저장 후
Refresh saved cues로 Parent 행과 재생 입력을 갱신한다.

binding/combat/cue Save는 source baseline과 검증된 임시 출력을 확인한다. 외부 변경이 있으면 저장을
거절하면서 파일과 초안을 보존한다. 서로 다른 파일의 저장을 하나의 원자 Save로 표시하지 않는다.
기존 `.animevents`를 전면 `.animevents.json`으로 바꾸는 이전 설계 전체는 이번 변경에 포함하지 않았다.

## G04. 독립 Collider → Logic → Result와 Server 판정

기존 여섯 HitShapes 문서를 formatVersion 4로 이관했다. 76개 damaging skill의 source hit 166개가
독립 DAMAGE / COUNTER / STAGGER collider 498개가 되었고, colliderId / logicId / resultId는 총
1,494개다. 기존 68개 skill의 caster/projectile hit 153개는 geometry, schedule, 순서, metadata가
같음을 역비교했다. 누락 8개 skill의 13개 hit는 기존 Server maximumRange fallback을 명시적으로
옮겼으며 원본 데이터 복원이라고 기록하지 않았다.

`CharacterActionCombatDocument.{h,cpp}`는 기존 HitShapes에서 행을 읽고 shape/time을 편집·저장한다.
각 Logic은 DURATION / AREA_OVERLAP으로 자기 Collider와 Result를 참조한다. Result 종류와 연결 ID는
현재 문서에서 고정하며, 잘못된 종류·연결 변경·중복 ID·시간 상한·비유한 shape를 거부한다.

`Publish-GameplayBalance.ps1`이 연결을 검증하고 기존 SKILLHIT/SKILLPROJ에 Result kind를 기록한다.
`GameplayCatalog`가 읽고 `PlayerSkillSystem`과 `ServerCombatHitRuntime`이 실제 판정을 수행한다.
DAMAGE만 HP 예산, part damage, push를 소비한다. COUNTER와 STAGGER는 각각 기존 PlayerSkills의
counterPower와 staggerDamage만 사용한다. counter/stagger가 0인 skill에 새 효과나 HP 피해를 만들지
않으며 trait-only hit가 카드미로 즉사를 발생시키지도 않는다. 결과를 세 행으로 나눠도 총 피해량은
기존 값으로 유지한다. 반복 판정 마스크도 세 종류의 상한을 수용하도록 192개로 확장했다.

캐릭터 Combat 연결은 이 세 종류다. BURN은 이번 연결 대상이 아니다. caster 시간은 action/stage 기준,
projectile 시간은 생성 이후 기준이다. CONTACT는 공간 접촉이 발생할 때 판정하므로 고정 적중 시각이
없다. timeline은 이 소유 관계를 표시한다. projectile shape의 편집 wire는 caster root에서 모양을
확인하는 표시이며 투사체 이동·실제 접촉 시뮬레이션을 대신하지 않는다.

Gameplay bootstrap 형식은 34로 올렸고 packet version은 바꾸지 않았다. 새 Client/Server와 v34
bootstrap을 함께 적용해야 한다. authoring Save만으로 실행 중 Server가 바뀌지는 않는다.

## G05. Object와 Sequence 검토 결과

Object Parent는 연결 Motion의 Transform/Animation/Effect overview를 표시한다. 행을 선택하면 기존
Motion 편집으로 들어가며, 연결 Composition과 source baseline을 확인하는 기존 저장 흐름을 유지한다.

Sequence는 기존 session과 runtime을 그대로 공통 창에 연결할 수 있는 구조다. 현재 Animation,
Effect, Sound, Camera, Collider, Light, World, Scene Profile과 Logic을 사용한다. 새로운 중복
Sequence runtime이나 별도 데이터 복사본은 추가하지 않았다. 기존 Complete Play와
enterCombatOnFinish 소비도 유지했다.

이 UI 통합은 연출과 전투에서 동일 Server boss identity를 유지하는 후속 목표의 완료 증거가 아니다.
해당 경계는 `../09-12/2026-09-12_KOUKU_SEQUENCE_COMBAT_HANDOFF_RESULT.md`의 최신 상태를 따른다.
카메라·음향·조명·월드 요소를 추가 편집할 때도 기존 lane과 각 실제 소비자를 확장하면 된다.

## G06. 실행한 검증

| 검사 | 실제 결과와 로그 |
|---|---|
| Client Debug x64 | 영향받는 C++와 header 소비자를 VS2022 Community 14.44 `/MDd`로 별도 컴파일하고 실제 프로젝트 299 TU의 object 집합으로 전체 링크 PASS. `out/UnifiedActions20260913/ClientCompile/`, `LinkedClient/link.log` |
| Server / Shared Debug x64 | Server 81 TU + Shared 7 TU를 전부 fresh compile, 별도 Shared.lib / Server.exe 링크 PASS. 경고·오류 0. `out/UnifiedActionsServer20260913/validation-result.json` |
| 실제 공통 shell CPU 검사 | target 전환, 이전 세션 deactivate, 반복 Open, 독립 Boss/Sequence 관문, exact gate deep link, draft 유지 PASS. `out/UnifiedActions20260913/shell_contract.cpp` |
| Character binding CPU 검사 | 6 class / 94 skill / 183 clip, native 60 checks PASS. legacy 읽기, ID/수치 정밀도 저장·재로드, 순서 변경, 중복 ID·외부 충돌 보존. `out/CharacterActionWorkbench20260913/probe-test.log` |
| 실제 모델 metadata | Engine reader로 14 WModel / 183 source window PASS. `out/CharacterActionWorkbench20260913/metadata-test.log` |
| 복원·cue 시계 회귀 | 실제 production 함수 본문을 추출한 native 26 checks PASS. 같은 owner 복원·다른 owner 거부·실패/예외 lock 복원·clock 유지/clamp·수동 cue 선택을 검사. model/panel은 stub이며 그래픽 실행은 아님. `out/CharacterActionWorkbench20260913/lifecycle-test.log` |
| Combat document CPU 검사 | 6 class load, 원자 저장·ID 재로드·unsupported Result·NaN·늦은 시각·외부 충돌 보존 등 13개 PASS. `out/CharacterActionCombat20260913/editor.run.log` |
| 실제 Server 전투 CPU 검사 | 독립 overlap/miss, zero power, timed/contact projectile, 180회 마스크, 기존 34120 총 3610 피해/3 event, Artist tiger/DimensionMaster nail/Warlord 3타 등 27개 PASS. `out/CharacterActionCombat20260913/focused.run.log` |
| HitShapes Python 계약 | 기존 coverage 계약 5개 PASS. `python -B Tools/CharacterAnimationIntake/test_player_hitshape_coverage_contract.py` |
| Publisher HitShapes 범위 | 실제 publisher 함수와 hit block으로 103 bootstrap row / 1,494 ID 검증, dangling Result 거부 PASS. `out/CharacterActionCombat20260913/publish-focused.ps1` |
| JSON / project 등록 | HitShapes 6개 JSON 및 Client vcxproj/filters parse, 신규 H/CPP 4개 등록 확인. `git diff --check` PASS |

Client 링크에는 기존 DirectXTK 정적 라이브러리의 PDB 부재와 Edit-and-Continue 무시 경고가 있다.
컴파일·링크 성공을 Client 실행이나 화면 판정으로 대신하지 않았다. 마지막 복원/cue 시계 보정은
위 CPU 회귀와 최종 Debug 컴파일·링크로 확인했으며 UI 입력은 사용자가 확인해야 한다.

## G07. 초기 검증 당시 남은 적용과 사용자 확인

전체 `Publish-GameplayBalance.ps1 -Mode Validate`는 Valtan 검증 뒤 다음 기존 불일치로 중단됐다.

```text
KoukuSaydon composition validate failed: projected Product is stale:
Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json
```

로그는 `out/UnifiedActions20260913/gameplay-validate.log`다. 오류는 새 HitShapes 검증에 들어가기 전
Kouku projection의 freshness 검사에서 발생했다. 이 검사를 제거하거나 현재 편집 중인 Kouku 문서를
덮어쓰지 않았다. 전체 Publish, 기본 Product 출력 교체, Server 재시작과 실제 화면 확인은 남아 있다.

현재 Client PID 45900 / Server PID 48036을 유지했다. 사용자 편집 종료 후에는 source baseline과
Kouku projection을 다시 맞추고 Gameplay Validate/Publish 및 정상 Product 빌드를 수행해야 한다.
그 뒤 사용자가 직접 새 Client에서 다음을 확인한다.

1. F1 → Action Workbench → Composition Actions에서 Boss/Character/Object/Sequence 전환.
   Effect는 후속 G09 기준 `Open Effect Tool V1`, `Open Effect Tool V2`에서 확인한다.
2. Character → class → LMB Parent 또는 Q/W/E/R에서 Animation/Effect/Collider/Logic/Result 행과 Play/Seek.
3. Animation 편집·Save, cue의 Edit source cues·Save·Refresh saved cues, Collider shape/time·Save Combat.
4. Object Parent overview와 연결 Motion 편집, Sequence 기존 재생/저장, V1/V2 Attach와 resource 편집.
5. 미저장 상태의 카테고리·Level 전환 후 모델 복원, 여러 clip 재생 중 cue row 선택 시 시계 유지.

Client/UI 자율 실행·조작·화면 캡처나 visual PASS 판정은 수행하지 않았다. 자동 stage/commit/push도
수행하지 않았으며 다른 세션의 미커밋 변경과 이번 변경은 작업 트리에 함께 남아 있다.

## G08. 사용자 재빌드 후 시작 오류 수정과 저장본 보존

사용자가 새 기본 Client/Server를 직접 빌드한 뒤 `Process gameplay generation failed to initialize.
Status=Gameplay bootstrap header is invalid`가 발생했다. 새 코드의 요구 형식은 34지만 설치된
`Gameplay.bootstrap`은 33 / 4,395 rows였다. 오류는 시작 시 생성 데이터의 헤더 검사에서 발생했으며
Composition 저장본을 초기화하는 경로가 아니다. 당시 Client/Server 프로세스가 없음을 확인했다.

먼저 두 Composition 정본, Kouku Product 두 문서와 기존 bootstrap을
`out/GameplayBootstrap34Hotfix20260913/before/`에 보존했다. 이후 다음 정식 게시만 수행했다.

1. `python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish`:
   저장 revision 481에서 Encounter와 patternbindings 두 Product를 동기화했다.
2. `powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish`:
   Valtan/Kouku 검증과 Player HitShapes 76/76 coverage를 통과하고 bootstrap 34 / 4,605 rows를 게시했다.
3. 사용자가 빌드한 기본 EXE를 그대로 사용했다. 명시적인 실행 요청에 따라 Server PID 40420과
   Client PID 47740을 시작했다. Server의 `0.0.0.0:7777` LISTEN, 두 프로세스의 생존과 응답을 확인했다.
   UI 조작·화면 캡처나 visual PASS 판정은 하지 않았다.

로그는 같은 출력 폴더의 `kouku-project.log`, `gameplay-publish.log`, `server.stderr.log`다.
헤더 숫자를 직접 고치거나 버전·freshness 검사를 완화하지 않았다. 새 C++ 변경이나 재빌드는 없었다.

`check_preserved.py`로 오류 수정 전 백업과 현재 저작 파일의 SHA-256을 비교했다.

| 저작 정본 | 보존 확인 |
|---|---|
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | revision 481, 55 patterns, 전체 바이트 동일 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | revision 25, 7 patterns, 전체 바이트 동일 |

`세이튼_1관문연출` P36의 애니메이션 13개, 원본 trim과 재생 길이, Logic 52 블렌딩 occurrence 5개도
저장본에 남아 있다. 이 검사는 저장된 편집의 보존 증거이며 이전 프로세스의 미저장 메모리 초안까지
보존됐다는 뜻은 아니다. 사용자가 새로 요청한 연출·블렌딩·Stop·F6·조명 변경은 이 긴급 수정에
섞지 않았으며 조사 단계다. 현재 실행 중인 편집 세션을 다시 종료하거나 reload하지 않았다.

미저장 복구도 읽기 전용으로 확인했다. Workbench의 `Commit_Candidate`는 `m_Draft`와 dirty 상태만
바꾸며 디스크 쓰기는 명시적인 Save의 `Save_Atomic`에 연결된다. 종료 시 자동저장, 초안 자동백업,
재시작 복구 consumer는 없다. 두 정본 폴더에도 잔여 `.tmp` / `.bak` 파일이 없었다. 종료된 이전
프로세스에서 마지막 Save 뒤에 한 변경의 존재·내용을 현재 디스크로 확정하거나 복구했다고 기록하지 않는다.

## G09. Effect Tool V1/V2 독립 창 복구

사용자가 Effect Tool은 분리해 달라고 요청하여 연출 확장보다 먼저 복구했다. F1에
`Open Effect Tool V1`, `Open Effect Tool V2`를 다시 표시하고 각 버튼이 자기 Effect owner만 열도록
`MainApp.cpp`의 Open, visibility, Render와 input owner 경로를 복구했다. 두 Effect 창은 동시에 표시할
수 있고 Action 창을 닫아도 Effect visibility와 현재 Effect input owner를 유지한다. 직접 Action
카테고리를 선택하면 기존 단일 viewport owner 계약대로 해당 Action이 입력을 가져간다.

`SequencerTool.h/.cpp`, `CompositionWorkbenchSession.h`에서 Effect target/session/selector를 제거했다.
Effect V1/V2의 사용하지 않는 Composition adapter와 embedded 옵션도 제거하여 각 native Render가
기존 창을 한 번만 그린다. native 창 ID, V1 dirty 전환 guard, V2 staged load/초안 복원, Character용
sequencer factory와 typed Effect resource 열기는 유지했다. 이번 분리에서는 Data를 쓰거나 Publish를
실행하지 않았다.

| 검사 | 결과 |
|---|---|
| Client Debug x64 격리 컴파일 | 실제 project 299 TU 중 영향받는 27 TU 재컴파일, 오류 0 |
| Client 전체 링크 | 299 TU object로 링크 성공. 기존 DirectXTK PDB 부재/비증분 Edit-and-Continue 경고만 있음 |
| 실제 라우팅 함수 CPU 검사 | V1/V2 동시 표시, 4 Action 대상 전환/닫기, 각 Effect만 Deactivate, alias, 재열기와 draft 보존 46 checks PASS. stub tool을 사용한 상태 검사이며 UI 실행은 아님 |
| native Effect 어댑터 제거 | adapter 참조 0, 기존 native 함수와 Character factory 보존, 인코딩·줄바꿈 보존 |
| 작업 트리 공백 검사 | `git diff --check` PASS |

로그는 `out/EffectStandaloneRestore20260913/ClientCompile/final-results.json`,
`LinkedClient/link.log`, `RoutingProbe/run.log`와 `source-receipt.json`이다. native 파일별 보존 기록은
`out/EffectToolStandalone20260913/adapter-removal-receipt.json`이다. 이번 C++/UI 분리에는 런타임 데이터
이관이 필요하지 않다. 신규 C++ 파일이나 project/filter 등록은 추가하지 않았다.

사용자 편집 중인 Client PID 47740 / Server PID 40420은 그대로 유지했다. 위 빌드는 `out`의 격리
출력이며 현재 실행 EXE의 적용을 뜻하지 않는다. 실제 Product Build와 사용자 화면 확인은 아직 남아
있고, 두 프로그램을 저장·종료한 뒤 알려 달라고 요청했다.

추가로 사용자가 Animation Play의 이동을 문의했다. 이전 root-motion RESULT는 Server Play만
구현했고 local Preview는 제외했다. 현재 local preview actor는 수평 root를 억제하지만 자동 actor
이동을 소비하지 않는다. P36의 `rpct00_walk_normal_1`은 설치 WModel 49키의 XYZ 이동이 모두 0인
제자리 클립이라 별도 Matinee/BossMotion 연결이 필요하다. 이 구분은 읽기 전용으로 확인했으며
이번 Effect 창 복구에 이동 코드를 섞지 않았다.
