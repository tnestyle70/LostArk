# Product Effect 증분 prewarm 구현 계획

기준일: 2026-08-16

기준 브랜치: `codex/effect-incremental-prewarm`

## 0. 현재 실제 반영 상태와 이번 경계

현재 `CCharacter::Load_EffectCues()`는 animevent의 clip, cue timing, anchor와 Product admission을
검증한 직후 `CEffectPresentationService::Prepare_ProductCues()`를 동기 호출한다. 이 함수는 이미
준비된 전역 target과 새 character의 모든 cue target을 합친 뒤 `CEffectCatalog::Find()`와
`CEffectDocumentRenderer::Prepare_VisualProgramCatalog()`를 한 번에 실행한다.

현재 runtime catalog는 `Get-ActiveProductEffectIds()`가 네 class animevents에서 선택한
`effectref=asset` target만 publish하고 direct-authored Effect document는 lazy load한다. 따라서 첫 Lance Master 생성은
클래스의 JSON 문서 parse, drawable validation, DDS/WModel/shader와 prepared renderer 생성이 모두
Client 메인 스레드의 character clone 안에서 끝날 때까지 다음 frame으로 진행하지 못한다.

이 브랜치의 current `main`은 cue별 Product approval sidecar를 이미 폐기했다. publisher가
`$hasProductCuePolicy = $false`로 고정하므로 정상 unified cue도 `pProductAdmissionToken == nullptr`이다.
따라서 token은 완성 여부가 아니며, 완성된 Product target의 현재 정본은 publisher가 선택해 runtime
catalog에 commit한 Effect ID membership이다. Effect Tool의 authoring-only 문서는 runtime catalog에
없으므로 Character Select queue에 들어오지 않는다.

이번 변경은 다음 네 계약만 닫는다.

1. Character 초기화는 cue/binding/anchor metadata만 검증·commit하고 Effect document나 GPU resource를
   동기 준비하지 않는다.
2. publisher-selected runtime catalog에 있는 고유 Effect ID만 process-global 준비 큐에 등록한다.
3. 첫 화면 update 한 번을 양보한 뒤 `CMainApp::Update()`의 전용 main-thread seam이 frame당 target
   하나만 parse → validate → resource stage → prepared-cache commit한다. 실패 target은 revision 단위로
   latch하고 이미 성공한 target과 다음 target은 보존한다.
4. Product spawn은 asset ID로 prepared cache를 먼저 조회한다. miss는 catalog document `Find()`나
   shader/model/DDS/vector-field 준비 없이 fail-closed한다.

Effect Tool의 명시적 `Publish + Reload Product Test`, authored 문서 편집, 기존 batch reprepare와
prepared instance의 수명은 유지한다. Client/UI 실행과 화면 판정은 사용자 경계로 남긴다.

## 1. 수정 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_DocumentRenderer.h` | 증분 prewarm session과 target 단위 commit 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | frame 간 immutable asset cache 재사용, target 단위 rollback/merge |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Catalog.h` | spawn이 사용하는 cache-only document 조회 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Catalog.cpp` | lazy parse를 하지 않는 `Find_Loaded()` 구현 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_ProductPrewarmQueue.h` | revision별 dedupe/yield/FIFO/success/failure 상태 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_ProductPrewarmQueue.cpp` | dependency-free 증분 queue 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_PresentationService.h` | 동기 prepare API를 metadata-only queue API로 교체 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp` | Product-only dedupe queue, one-frame grace, frame당 1 target, 실패 latch와 prepared-only spawn |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp` | 동기 prewarm 제거, Product ID queue 등록 뒤 cue/anchor/HIT 문서 즉시 commit |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | 첫 등록 frame 양보 뒤 frame당 한 target을 소비하는 main-thread seam |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | 증분 success/duplicate/invalid rollback/기존 target 보존 실행 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/CLAUDE.md` | 현재 Product 준비·spawn 사용법 정본 갱신 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | Character/Animation 담당자가 소비하는 public 준비 계약 갱신 |
| 추가 | `C:/Users/user/Desktop/LostArk/.md/GB/08-16/2026-08-16_EFFECT_INCREMENTAL_PRODUCT_PREWARM_RESULT.md` | 실제 diff, 자동 검증, 사용자 수동 검증과 남은 경계 분리 |

| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 queue H/CPP 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 기존 Effect 물리 폴더 filter에 새 queue H/CPP 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj` | production queue CPP를 focused harness에 연결 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters` | harness link 등록 |

새 queue H/CPP는 UTF-8 BOM 없이 추가하고, 기존 C++ 파일의 ASCII 또는 UTF-8 BOM 인코딩을 그대로
유지한다.

## G00. Character metadata commit과 Product-only 등록

`CCharacter::Load_EffectCues()`의 binding owner와 skeleton anchor 검증은 그대로 둔다. 마지막 동기
`Prepare_ProductCues()` 호출은 metadata-only `Queue_ProductCues()`로 교체한다. queue 함수는
animevents loader가 `effectref=asset`으로 골랐고 현재 runtime catalog membership 검증을 통과한 cue만
현재 catalog revision의 Product target으로 받는다. token은 optional legacy compatibility 정보일 뿐
queue membership 기준으로 사용하지 않는다.

queue 검증 실패는 기존처럼 staged cue document commit을 막고 이전 상태를 보존한다. queue 등록 성공은
resource 준비 성공을 뜻하지 않는다. `m_EffectCueDocument`는 즉시 commit되어 action timing과 anchor
metadata를 계속 제공한다. runtime catalog에 publish되지 않은 authoring-only 문서는
`CAnimationEffectCueDocument::Load()`의 `Contains()` 검증에서 Character Select 경로로 들어오지 못한다.

## G01. Presentation service의 frame 단위 준비 상태

process-global queue 상태는 현재 catalog revision, pending FIFO, pending ID set, prepared target set과
failed target set을 소유한다. renderer incremental session은 같은 Device/Context와 revision의 shared
asset cache를 별도로 소유한다.

새 target 등록 흐름은 다음과 같다.

```text
Character cue metadata
→ runtime catalog membership과 optional legacy admission 검증
→ prepared / pending / failed revision set으로 dedupe
→ pending FIFO commit
→ 첫 Update 한 번은 준비하지 않고 화면 진행
```

그 다음 각 `CMainApp::Update()`의 전용 `Advance_ProductCuePreparation()` 호출은 FIFO 앞 target 하나만
처리한다.

```text
Effect ID
→ current revision 재검증
→ CEffectCatalog::Find 한 문서 lazy parse
→ visual projection identity와 scene budget stage
→ renderer target 단위 준비
→ 성공 시 prepared target + budget receipt commit
→ 실패 시 해당 ID만 failed set에 latch하고 다음 frame에 다음 target 진행
```

같은 revision의 두 번째 Character 진입은 prepared/pending/failed set으로 중복 작업하지 않는다. catalog
revision이 바뀌면 queue/session/target receipt를 새 revision으로 초기화한다. 명시적 batch reprepare 성공은
해당 target들을 prepared로 commit하고 같은 ID의 pending/failure 상태를 제거한다.

## G02. Renderer target 단위 transaction

`CEffectDocumentRenderer`에는 opaque incremental session을 추가한다. session은 같은 Device/Context와
catalog revision에서 준비된 WModel/DDS immutable resource map을 frame 사이에 유지한다. target 하나를
준비할 때 session cache를 먼저 stage-copy하고 `Build_PreparedDocument()` 성공 뒤에만 session cache와
전역 prepared index를 commit한다. 중간 실패는 이전 prepared catalog와 shared resource session을 모두
보존한다.

전역 prepared catalog의 기존 document-pointer index를 유지한다. batch prepare는 기존처럼 전체 index를
한 transaction으로 교체하고, incremental prepare는 같은 revision의 기존 index를 보존한 채 target
하나를 merge한다. 잘못된 target, duplicate identity, stale revision은 commit 전에 거부한다.

## G03. Product spawn의 no-I/O miss

`CEffectPresentationService::Spawn()`과 `Spawn_Immediate()`는 다음 순서를 사용한다.

```text
optional Product admission / reconstructed boundary / spawn descriptor 검증
→ revision별 prepared target 확인
→ CEffectCatalog::Find_Loaded cache-only lookup
→ document identity prepared lookup
→ miss면 즉시 fail-closed
→ hit이면 이미 load된 catalog document와 visual projection 사용
→ budget receipt 확인
→ EffectObject clone / prepared attach
```

따라서 아직 FIFO 차례가 오지 않았거나 준비에 실패한 cue를 눌러도 spawn 경로는 direct-authored JSON을
읽거나 GPU resource를 만들지 않는다. 이미 준비된 target의 기존 no-I/O/no-GPU-allocation probe는
그대로 유지한다.

## G04. 실행 검증과 문서 갱신

새 dependency-free queue seam은 duplicate coalescing, 첫 update yield, frame당 한 target, 실패 latch와
revision reset을 실행 검증한다. focused Effect runtime mode의 실제 WARP renderer fixture에서 target을
하나씩 준비한다.
첫 target 뒤 두 번째 target을 commit했을 때 첫 target lookup이 유지되는지, duplicate 재호출이 document
build를 늘리지 않는지, invalid target과 revision 0 실패가 기존 cache/count를 바꾸지 않는지 확인한다.

자동 검증 순서는 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate

msbuild Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --effect-incremental-prewarm-fast

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release

git diff --check
```

에이전트는 Client를 실행하지 않는다. merge 뒤 이 PC는 `client` 역할이므로 사용자가 Visual Studio에서
Client project를 `Ctrl+F5`로 실행하고 Lobby → Character Select → Lance Master 첫 진입의 frame 진행,
준비 중 즉시 스킬의 fail-closed, 준비 완료 뒤 Effect 표시와 두 번째 진입을 직접 확인한다. 사용자의
서면 관찰 전에는 visual PASS나 cold-entry timing PASS를 RESULT에 기록하지 않는다.
