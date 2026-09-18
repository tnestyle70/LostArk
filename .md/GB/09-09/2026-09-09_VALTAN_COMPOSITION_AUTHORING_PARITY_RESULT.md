# 발탄 Composition 저작 흐름 조사·계획 결과

작성일: 2026-09-09. 갱신: 2026-09-18. 현재 구현 상태는 아래 재개 결과를 따른다.

## 2026-09-18 구현 및 실행 반영

이번 재개는 발탄의 실제 source 애니메이션 transport, 공통 Composition 편집 화면,
Source Save와 Product 분리, 4연속 공격의 Full Restore 연결을 구현했다.
아래 2026-09-09 내용은 조사 당시 기록이다. 당시 계획의 모든 G가 완료됐다는 뜻은 아니다.

### G08. Play·Pause·Seek와 Effect Solo·Group

- Valtan source sequence는 CModel을 pause 상태로 두고 하나의 커서로 pose를 sample한다.
  Play/Pause, Reset, 역방향 seek, clip 경계와 종료 pose, Loop가 같은 clock을 소비한다.
- 공통 Resources transport, Sequencer ruler, 별도 Preview 창이 source/master 소유자를 구분한다.
  source preview는 Pattern ID가 비어 있으므로 Pattern ID 비교보다 source owner를 먼저 처리한다.
- Effect Tool의 `420609 stage008/009 Full Restore` Play All, Solo, Play Group은 실제 원본
  clip occurrence와 source anchor history를 EffectAuthoringSequencer에 전달한다.
  기존 root scale 1과 Valtan bone owner scale 1.4를 구분한다.
- root 이외 세션·창으로 이동하면 이전 preview와 Camera lease를 종료하고, 오래된 PLAYING UI 상태를 남기지 않는다.

### G09. 공통 Resources·Sequencer와 실제 편집 경로

- Saydon과 Valtan이 같은 `COMPOSITION_RESOURCE_CATEGORIES`와 `CompositionTimeline`의
  행 높이 24, 라벨 폭 180, 최소 box 폭 8을 사용한다. Resource tab은 Animation, Logic,
  Summon, World, Scene Profile, Effect, Collider, Sound, Camera, Light, Pattern이다.
- Stage 다음 Animation·Logic·Summon·World·Scene Profile과 Effect·Sound·Camera·Collider·Light
  트랙이 이어진다. Resources의 추가, Box Detail의 수정·삭제, timeline drag를 같은 typed owner에 연결했다.
- Stage body 이동은 앞 Stage 길이를 바꿔 뒤 clock을 함께 이동한다. 첫 Stage의 시작은 0이다.
  Animation은 기존 source clip 순서·구간을, Collider는 실제 hit schedule을 편집한다.
- Camera·Scene Profile·Light는 시간 이동·양끝 trim과 Stage 간 이동을 지원한다.
  실패한 Stage 간 변경은 draft와 dirty generation을 함께 rollback한다.
- Summon은 기존 combat-object resource, spawn count/wave/interval과 공유 archetype lifetime을 편집한다.
  공유 lifetime 변경은 같은 resource의 모든 occurrence에 적용됨을 UI에 표시한다.
- Logic과 World는 Server의 ENTER/EXIT 계약을 유지해 가까운 Stage 경계로 snap한다.
  Counter/Groggy topology와 phase 전환 등 전용 계약은 해당 typed 편집기를 사용한다.
  기존 World set은 단일 invocation 계약 때문에 Resource에서 `Move to Stage`로 이동한다.
- Sound는 시점 이벤트다. 참조·자동 파생 행과 길이가 없는 이벤트에 임의의 duration을 저장하지 않는다.
  새 Scene/Light/Camera 끝과 Summon 마지막 spawn을 넘겨 Stage를 줄이면 변경 전 거부한다.

### G10. Source Save와 Product 소비

- Save는 owner별 최신 baseline/CAS, 구조 parse, atomic writer·rollback을 유지하면서
  전체 Product projection 검증을 일반 Source 저장과 분리한다. Product reopen 실패를
  이미 성공한 Source 저장 실패로 보고하지 않는다.
- Workbench와 Balance가 source inventory를 직접 읽고, 명시적 draft preview는 해당 draft를 소비한다.
  제품은 strict Publish로 만든 snapshot을 계속 사용한다.
- V2와 Sound의 게시된 문서는 `Data/Valtan/Published/` 아래 생성물로 분리한다.
  Source Save만으로 실행 중 또는 재실행한 제품에 미완성 binding이 활성화되지 않는다.
  local V2 preview에는 기존 명시적 authoring snapshot 주입을 유지한다.
- Scene Profile·Light는 stage별 저장, product patternbindings v5와 실제 CValtan stage clock까지 연결했다.
  기존 공통 presentation sampler, rendering profile lease와 frame light provider를 사용한다.
- Camera occurrence도 v5 binding을 소비한다. 제품의 기존 cinematic controller와 local CameraTool이
  같은 stage offset/duration을 사용하며 새 camera renderer는 만들지 않았다.

### G11. 4연속 공격의 Full Restore

- `VALTAN_FOUR_SLASH`의 SLASHES와 SPIN에 `effect.valtan.action.420609.stage008.full.restore`,
  `effect.valtan.action.420609.stage009.full.restore`를 연결했다.
- 기존 stable occurrence ID, timing, source window와 사용자의 scale 1.5를 유지했다.
- source presentation과 generated pattern effect cues를 기존 writer lock/CAS 경로로 함께 반영했다.
  이어 Camera v5와 새 published V2/Sound snapshot을 strict Publish로 생성했다.
- 검격의 sprite 복원·empty alpha key 수정은
  [09-18 검격 복원 결과](../09-18/2026-09-18_VALTAN_FOUR_SLASH_SPRITE_RESTORE_RESULT.md)를 함께 따른다.

### 검증과 사용자 확인 경계

- Effect/Animation transport 변경 TU, Workbench/MainApp, environment/Camera와 Save owner의 최소 컴파일을 수행했다.
- 실제 C++ source transport 함수를 이용한 CPU 검사는 14건을 통과했다.
- 실제 auxiliary drag 함수를 이용한 CPU 검사는 Stage ripple, Camera/환경 Stage 간 이동,
  상태쌍·World 경계, Summon clock과 rollback을 검사했다. 최종 건수는 해당 실행 로그를 따른다.
- strict source/product projection은 42 managed / 25 legacy / 9 combat object / 97 world member로 통과했다.
- 검증 파일: `out/ValtanSequencerTransport20260918/`, `out/ValtanUnifiedSequencer20260918/`.
- Debug Product build는 14:19와 14:21 증분 빌드 모두 PASS. 마지막 증분은 native generation admission의
  게시된 V2/Sound 경로를 포함한다. 증거: `out/BuildPipeline/runs/20260918T052152203Z-debug-product.json`.
- 그 뒤 독립 검토에서 Camera의 지연을 잘못된 trigger로 저장하던 두 UI 지점을 `ENTER + startOffsetMs`로,
  Summon Add가 오래된 prototype lifetime으로 미저장 공유 값을 되돌리던 경로를 최신 typed draft 조회로 고쳤다.
  마지막 Workbench/MainApp/Kouku TU 컴파일은 exit 0이다. 이후 사용자가 직접 빌드하기로 했으므로
  에이전트의 추가 제품 빌드는 수행하지 않았다. 14:25 EXE 갱신과 Server/Client 실행은 읽기 전용으로 확인했다.
- Source 저장 4검사(CAS, 무변경/다른 owner 보존, 미해결 Sound의 Save 허용·Publish 거부,
  두 sidecar 중간 실패의 바이트 단위 rollback)와 실제 PowerShell 비동기 Save wrapper 1검사 PASS.
  Windows PowerShell 5.1의 null backup `File.Replace`가 최종 receipt 쓰기를 실패시키던 문제도
  job 소유 backup 경로를 사용해 고쳤다. 저장됐는데 UI가 실패로 남을 수 있던 실제 경로다.
- generation 검사 3건 PASS. 미완성 source V2/Sound 변경은 제품 generation을 바꾸지 않고,
  게시된 V2/Sound 변경은 generation을 바꾼다.
- auxiliary drag CPU 검사 최종 15건 PASS. native source-only inventory는 42 patterns / 194 stages /
  8 Summon occurrences / 3 World triggers이며 Product/effect reader 호출을 abort sentinel로 차단해 확인했다.
  환경 typed patch 왕복, 8개 잘못된 입력 거부, v5 parser 실패 보존과 Camera offset/duration도 PASS.
- 변경 JSON 22개와 project/filter XML 2개 parse, 새 TU 단일 등록, `git diff --check` PASS.
- 최신 쿠크 저장본 revision 1556에서 generated Encounter/PatternBindings가 stale임을 확인해 공식 projector로
  generated 두 파일만 다시 게시했다. 사용자 authoring 파일은 변경하지 않았다.
- `Publish-GameplayBalance.ps1 -Mode Publish` PASS, `Gameplay.bootstrap`은 14:30:57 갱신됐다.
  145개 presentation artifact로 재계산한 generation
  `9a858a90804166b8fd1bc959078c6d056ab65c084463428930f575867c102b04`가 실제 bootstrap 행과 일치한다.
  게시 로그는 `out/ValtanUnifiedSequencer20260918/gameplay-publish.log`, 재계산은 `generation-final.json`이다.
- 실행 중인 14:25 Server/Client는 게시보다 먼저 시작됐다. 새 제품 데이터 적용은 사용자가 두 프로그램을
  다시 시작한 후 확인한다. 에이전트가 Reload·프로세스 종료를 수행하지 않았다.
- Client/UI를 실행하거나 화면 판정을 대신하지 않았다. 사용자는 새 EXE에서 Valtan Sequencer의
  Play/Pause/Reset/seek, Full Restore Solo/Group, resource 추가·drag·Save 후 다시 열기를 확인한다.
  실행 중 도구의 미저장 draft는 외부 파일 변경으로 자동 교체하지 않는다.

### 이번 재개와 구별할 과거 계획

09-09의 빈 formatVersion 2 Draft Pattern 및 저자가 지속 저장하는 자유 row ID 도입은 이번 변경에
포함되지 않았다. 현재 source schema와 Server의 실제 Stage/액션 계약을 유지한 편집 기능을 구현했다.
Source 저장 성공, strict Publish, EXE 빌드와 사용자 화면 판정은 서로 별개의 완료 단계다.

---

## 2026-09-09 조사 당시 기록



후속 실행 준비에서도 새 저작 기능은 미구현이다. Source Save Python 초안은 UI 소비자와 Product snapshot 분리가
완료되지 않아 `out/ValtanCompositionParity20260909/unfinished_source_save/`에 파일·patch·보존 기록으로 남겼다.
초안 3개만 작업 전 상태로 되돌려 기존 Save를 유지했다. 후속 Product 빌드 PASS는 이 계획의 구현 완료가 아니다.

대응 [구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_IMPLEMENTATION_PLAN.md)에
Source Save, V1/V2 append·box/detail, row와 Draft Pattern, local preview 및 Product Publish의 변경 범위를 정리했다.
쿠크의 좋은 저작 흐름을 기준으로 하되 현재 발탄의 source, Server authority와 재생기를 그대로 확장하는 계획이다.

## 현재 실측

기준 HEAD는 `591012dbebf7eeab0b660baec42852b9396e77d4`, 브랜치는 `codex/dimensionmaster-tool-round3`다.
조사 당시 HEAD와 origin/main은 같았고, 쿠크·이펙트·캐릭터 관련 다른 세션의 미커밋 변경이 있었다.
그 변경을 유지한 현재 working copy의 코드와 Source를 읽었다.

읽기 전용으로 다음을 확인했다.

- `python Tools/ValtanPipeline/valtan_tuning_pipeline.py --repository-root . validate`: PASS.
  managed pattern 42, legacy pattern 25, combat object 9, world member 97, projected artifact 9.
- 실제 Save writer가 호출하는 `validate_and_project`, Sound candidate dependency 검사,
  V2 binding candidate 검사도 현재 데이터로 PASS였다. 파일 commit은 실행하지 않았다.
- BOSS_VALTAN V2 source는 format 2, binding 102개이며 NATURAL 100개, STAGE_END 2개다.
- Save는 기본 Source·descriptor 5개와 조건부 Product 8개, dirty Sound/V2를 포함하면 최대 15개 target 후보를
  구성한다. 실제 쓰기는 바뀐 bytes만 수행한다. “현재 70개 검증이 항상 실패해 모든 Save가 불가능”이라는
  설명은 이 실측과 맞지 않는다.

## 실제 구조적 결합

현재 Save에는 dirty owner/CAS/atomic writer가 있으나 전체 `validate_and_project`, Sound/V2 join,
전체 source manifest와 Product reopen이 일반 Source 저장에도 연결돼 있다.
준비되지 않은 패턴 하나가 전체 editor reopen/저장 허용 상태에 영향을 줄 수 있는 구조다.
과거 2026-09-04 `missing V2 read-set` 오류는 현재 구현에서 고쳐진 이력이므로 현재 실패로 재사용하지 않았다.

V2에는 Save와 별개의 정확한 UI 결함도 있다. `VALTAN_BIND_SLOT`의 shout 세 binding은 고유 binding ID와
`clip.02/03/04`를 가지지만 현재 UI는 convenience stage 필드와 합성 ID를 소비한다.
runtime source-window 식의 올바른 시작은 1400/2300/3200ms인데 0ms의 같은 선택 ID로 충돌할 수 있다.
typed bindingId와 clock basis를 모든 선택·편집·저장·재생에서 동일하게 쓰는 변경을 계획했다.

현재 V2 append는 STAGE/NATURAL 등의 고정 기본값을 주고 detail은 start 중심이며,
V1은 기존 source draft·cue 편집 경로가 존재한다. V1을 저장본 재생만 가능한 기능으로 판정하지 않았다.
표시용 subrow packing은 저자가 저장한 row ID가 아니며, 빈 Draft Pattern 생성도 현재 Product intake와 결합돼 있다.

Source-only Save를 먼저 열기 전에 runtime snapshot 분리가 필요하다.
live `CEffectV2Runtime::Ensure_Bindings`가 authoring catalog revision을 따라가는 경로를 확인했다.
기존 presentation generation receipt의 immutable binding/leaf/group bytes를 제품이 소비하고,
local preview에는 명시 draft snapshot을 전달하도록 계획했다. 새 runtime/manifest는 만들지 않는다.

## 계획으로 정한 구현 단위

| G | 계획한 결과 |
|---|---|
| G01 | 정상 Source inventory와 오류 row 보존, Product snapshot 고정 |
| G02 | Source-only Save, owner별 CAS/atomic rollback, 미완성 Draft schema와 정확한 저장 상태 |
| G03 | V2 bindingId·typed clock·source-window·실제 finite end의 UI/runtime/publisher 일치 |
| G04 | Resources typed append와 V1/V2 Box Detail의 실제 owner mutation 연결 |
| G05 | 빈 Draft Pattern, 지속되는 row ID와 row assignment |
| G06 | 선택 draft local preview, 기존 encounter 전체 atomic Publish 및 Server revision 보존 |
| G07 | 기능별 최소 컴파일·focused 입력/저장 검사와 사용자 아레나 재생 인계 |

발탄 raid의 scripted/cross-pattern 참조는 기존 encounter 전체 Product 단위로 검증·배포한다.
실패한 일부를 이전 Product와 섞어 새 revision 전체가 적용됐다고 하지 않는다.
Product에 연결되지 않은 새 Draft는 정상 저작 Save를 막지 않게 한다.

## 수행 상태

계획서와 이 조사 RESULT만 추가했다. 발탄 C++/Python/Source schema는 이번 요청에서 수정하지 않았다.
새 schema·source-only Save·V2 identity 수정·row/Draft 기능은 모두 미구현이다.
현재 source의 읽기 전용 구조 검증과 문서 검토를 수행했으며 Client compile/link는 실행하지 않았다.
Workbench 실제 버튼 Save/Reopen, local/Server Play, Client 시각 결과는 이번 조사에서 실행하지 않았다.
그 결과는 사용자의 직접 조작과 후속 구현 검증 후에만 기록한다.

PLAN/RESULT의 UTF-8, 모든 링크 대상 존재, 신규 파일을 포함한 공백 검사를 확인했다.
문서 확인 근거는 [documents_verified.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/documents_verified.json)에 있다.
