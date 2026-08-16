# Character Select Effect 로딩 준비와 All Effects 목록 복구 구현 계획

기준일: 2026-08-16

기준 브랜치: `codex/effect-binary-preload-all-effects` (`origin/main` `d176c6e8`)

## 0. 실측 원인과 이번 변경 경계

Invisible prune은 All Effects 목록 정본이나 Effect Tool 코드를 바꾸지 않았다. 현재 source
`EffectCatalog.json`에는 `DIRECT_AUTHORED_DOCUMENT_V13` 101개가 있고 모두 실제 authored unified
문서 및 `PlayerSkills.json` owner와 일치한다. 목록이 비는 원인은 Effect Tool이 이 source catalog를
이미 읽고도 별도 UI에서는 폐기된 `FourClassCombat.track-a-authored-import-batch.json`과 runtime
`m_AllEffects` join을 필수 정본으로 사용하기 때문이다.

Product spawn은 이미 `Find_Loaded()`만 사용하므로 좌클릭이 직접 JSON을 읽지는 않는다. 그러나 현재
main frame은 gameplay input 직후 `Advance_ProductCuePreparation()`에서 Lance 문서 한 개의 read,
SHA, JSON decode, drawable validation, DDS/WModel 및 D3D 준비를 모두 동기 실행한다. 한 문서/frame은
시간 제한이 아니므로 5~6 MiB 문서 프레임과 좌클릭이 겹치면 클릭이 원인인 것처럼 멈춘다.

Winters의 `.wfx`도 text JSON이며 새 literal WFX loader나 두 번째 Effect runtime은 만들지 않는다.
이번 변경은 authoring JSON과 기존 unified catalog/renderer를 유지하면서 다음 계약을 닫는다.

1. All Effects의 saved unified 정본을 source Effect catalog direct-authored row로 교체한다.
2. ImGui 목록 렌더는 lazy `Find()`를 호출하지 않는다.
3. Character Select activation 전에 Loading Level에서 선택 class Product cue를 우선 등록하고 queue가
   끝날 때까지 기존 main-thread 증분 준비를 소비한다.
4. 준비된 Product attach는 prewarm에서 확정한 immutable identity를 재사용해 canonical JSON 재직렬화와
   전체 document deep copy를 반복하지 않는다.

새 cooked binary 포맷은 이번 변경에 넣지 않는다. 위 네 단계 뒤 계측으로 JSON decode가 남은 병목임이
확인될 때만 기존 catalog 내부의 versioned cooked representation으로 별도 설계한다. DDS/WModel 및 GPU
생성 비용은 binary로 없어지지 않는다.

## 1. 수정 파일과 역할

| 파일 | 역할 |
|---|---|
| `Client/Private/Effect_Tool.cpp`, `Client/Public/Effect_Tool.h` | source catalog 기반 saved unified 목록, optional Product annotation, render-time cache-only lookup |
| `Client/Private/Effect_DirectAuthoredSourceIndex.cpp`, `Client/Public/Effect_DirectAuthoredSourceIndex.h` | Tool과 harness가 함께 쓰는 source direct row/owner/canonical path transactional index |
| `Client/Private/Effect_Catalog.cpp`, `Client/Public/Effect_Catalog.h` | passive Tool render용 cache-only visual projection 조회, harness 전용 짧은 runtime fixture 경로 |
| `Client/Private/AnimationEffectCueDocument.cpp`, `Client/Public/AnimationEffectCueDocument.h` | live `CModel` 없이 선택 class 전체 clip의 cue metadata를 로딩 단계에서 검증하는 API |
| `Client/Private/Effect_ProductPrewarmQueue.cpp`, `Client/Public/Effect_ProductPrewarmQueue.h` | 현재 선택 class target을 pending 앞에 재우선화하고 probe를 노출 |
| `Client/Private/Effect_PresentationService.cpp`, `Client/Public/Effect_PresentationService.h` | priority 등록, loading completion 판단, prepared attach identity 재사용 |
| `Client/Private/Level_Loading.cpp`, `Client/Public/Level_Loading.h` | Loader 완료 후 Character Select Effect 준비를 등록하고 pending 종료 뒤 activation 요청 |
| `Client/Private/ClientReplication.cpp`, `Client/Public/ClientReplication.h` | Character Select의 local class-change snapshot을 최신 generation으로 보류하고 준비 뒤 명시 commit |
| `Client/Private/Level_CharacterSelect.cpp`, `Client/Public/Level_CharacterSelect.h` | 내부 class 변경도 priority 준비와 global queue drain 뒤 presentation 교체 |
| `Client/Private/Effect_Playback.cpp`, `Client/Public/Effect_Playback.h` | prepared Product document의 canonical identity와 shared immutable document를 재사용 |
| `Client/Private/Effect_DocumentRenderer.cpp`, `Client/Public/Effect_DocumentRenderer.h` | exact prepared projection/document identity에서 signature 재계산과 document copy 제거 |
| `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | source direct index, queue priority/loading gate, prepared attach no-rebuild 회귀 검증 |
| `Client/Default/Client.vcxproj`, `.filters`, `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`, `.filters` | cue scanner와 source-index helper를 제품/harness 빌드에 연결 |
| `CLAUDE.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 바뀐 Product loading 및 Effect Tool public 계약 |
| `.md/GB/08-16/2026-08-16_EFFECT_LOADING_PREWARM_AND_ALL_EFFECTS_RESULT.md` | 실제 구현·검증·수동 경계 기록 |

source-index helper는 새 C++ 파일로 분리해 Effect Tool과 focused harness가 동일 production builder를
호출한다. Loading cue scanner도 실제 호출하도록 `AnimationEffectCueDocument.cpp`를 harness에 등록한다.

## G00. Source catalog 기반 All Effects

`Refresh_DirectAuthoredEditableIndex()`가 validation을 통과시킨 101개 direct-authored entry를 ordered saved
unified index로 commit한다. class/skill owner는 stable Effect asset ID와 `PlayerSkills.json`을 join한다.
폐기된 Track-A batch는 Product 이전 관계를 보여 주는 optional annotation으로만 남기거나 제거하며, 누락,
invalid row, runtime Product tree refresh 실패가 saved authored 목록 전체를 지우지 못한다.

목록 렌더에서는 path/ID/owner metadata만 사용한다. 문서 decode는 사용자가 특정 saved Effect의 Open 또는
Play를 명시했을 때만 실행한다. Source Presets와 검색도 `CEffectCatalog::Find_Loaded()` 및 이미 준비된
visual program만 조회한다.

## G01. Loading Level의 선택 class Product 준비

`CAnimationEffectCueDocument`는 animevents의 모든 Effect row가 참조하는 clip 이름을 먼저 수집한 뒤 기존
`Load()` validation을 재사용해 선택 class cue set을 만든다. Loading worker가 target level resource load를
끝낸 뒤 main thread에서만 다음 흐름을 실행한다.

```text
selected class spec/animation asset
→ cue/anchor metadata load
→ selected class IDs를 prewarm queue 앞에 priority 등록
→ MainApp frame seam이 기존대로 target 최대 1개 준비
→ 선택 class와 기존 background pending/failed가 모두 settle
→ Character Select activation 요청
```

등록 실패나 개별 Effect 준비 실패는 presentation-only 오류로 기록하고 level 진입을 무한 차단하지 않는다.
동일 revision에서 성공/실패가 settle된 target은 Character 생성 시 재등록돼도 resource work를 반복하지
않는다. 다른 class의 오래된 pending 작업은 현재 선택 class 뒤쪽으로 밀되, 그것까지 모두 끝나기 전에는
Character Select를 활성화하지 않아 interactive frame에서 resource 준비가 다시 실행되지 않는다.

Character Select 안에서 Server가 class 변경 snapshot을 승인한 경우에도 즉시 Character를 교체하지 않는다.
stable entity/class generation의 최신 snapshot을 보류하고 같은 priority 준비/global drain gate를 통과한 뒤
기존 replacement transaction을 commit한다. 준비 중에는 gameplay/class/create/stage 입력을 막고 기존
presentation을 유지한다. 교체 transaction 자체가 실패하면 입력이 영구 정지하지 않도록 Lobby로 복귀한다.

Loading progress/status는 target 수를 표시하되 실제 Client 화면 PASS는 사용자가 판정한다.

## G02. Prepared Product attach의 반복 작업 제거

prewarm prepared record가 catalog revision, document pointer, projection pointer, canonical SHA와 resource
signature를 소유하는 기존 identity를 Product attach의 authority로 사용한다. exact identity가 일치하면
Playback과 Renderer는 document를 다시 serialize/hash하거나 `EFFECT_DOCUMENT_DESC` 전체를 복사하지 않고
prepared projection이 소유한 immutable document를 참조한다. Tool의 editable/non-Product staging은 기존
owned-copy validation 경로를 유지한다.

prepared miss, revision drift, projection/document identity mismatch는 disk/GPU fallback 없이 기존처럼
fail-closed한다. instance별 mutable playback state와 element runtime state만 clone한다.

## G03. 실행 검증

focused harness는 다음을 자동 확인한다.

- source direct-authored saved unified 101개와 PlayerSkills owner 101/101, 폐기 batch 비의존
- source direct index가 orphan/authoring-only 물리 문서를 Product 목록으로 추측하지 않음
- priority enqueue가 현재 class를 기존 pending 앞으로 옮기고 duplicate/revision 계약을 보존
- Loading과 내부 class change가 같은 readiness predicate로 stale revision/background pending을 거부
- loading cue scan에서 Lance Product cue 41개를 정확히 수집
- cache-only document/projection 조회가 first-use JSON load를 일으키지 않음
- prepared Product Playback/Renderer attach가 resource build, model/texture/vector-field I/O 또는 synchronous
  stage를 늘리지 않고, 내용이 같아도 주소 identity가 다른 문서는 거부함
- 기존 renderer identity, failed target isolation과 incremental rollback 계약 보존

검증 명령은 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate

msbuild Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --effect-loading-prewarm-fast

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release

git diff --check
```

에이전트는 Client/UI를 실행하지 않는다. merge 뒤 사용자는 Client project를 `Ctrl+F5`로 실행해 Loading
화면의 Effect 준비 상태, Character Select 첫 좌클릭, class 변경, All Effects의 saved unified 101개 목록과
Open/Play를 직접 확인한다. 사용자 관찰 전에는 visual/timing PASS를 기록하지 않는다.
