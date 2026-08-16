# Product Effect 증분 prewarm 결과

## 결론

- Character Select에서 선택 class의 Product Effect 문서를 `CCharacter::Initialize()` 안에서 한꺼번에
  parse·검증·GPU 준비하던 동기 경로를 제거했다.
- Character는 animevent의 cue, anchor와 HIT metadata를 검증한 뒤 runtime-published Effect ID만
  process-global queue에 등록하고 전체 metadata 문서를 즉시 commit한다.
- 등록 frame은 resource 작업을 양보한다. 다음 main-thread frame부터 FIFO target을 최대 하나씩
  JSON parse → drawable/budget 검증 → DDS/WModel/GPU 준비 → prepared-cache commit한다.
- 준비 성공 target만 spawn 가능하다. 준비 전·실패 target의 `Spawn`과 `Spawn_Immediate`는 cache-only
  lookup으로 fail-closed하며 direct-authored JSON을 처음 읽거나 GPU resource를 동기 생성하지 않는다.
- Lobby/Level clear는 prepared queue와 renderer cache를 유지한다. process shutdown 또는 catalog revision
  교체만 상태를 초기화하므로 같은 revision의 두 번째 진입은 이미 준비된 target을 다시 만들지 않는다.

## Product 정본 교정

현재 publisher는 `ProductCueApprovals.json` 정책을 비활성화하고 네 class animevent의
`effectref=asset` 집합만 runtime catalog에 publish한다. 실측 runtime set은 99개이며 Lance Master
41개를 포함한다. 따라서 null `pProductAdmissionToken`은 미완성 판정이 아니다.

이번 queue admission은 runtime catalog membership과 optional legacy admission을 사용한다. Effect Tool의
authoring-only 문서는 runtime membership이 없으므로 Character Select queue에 들어오지 않는다. 최종
drawable validation에 성공한 ID만 prepared set과 scene-budget receipt에 commit한다.

## 구현된 경계

### Character와 frame scheduler

- `CCharacter::Load_EffectCues()`의 class-wide `Prepare_ProductCues()`를 metadata-only
  `Queue_ProductCues()`로 교체했다.
- queue는 revision별 desired/pending/prepared/failed ID를 분리하고 duplicate를 coalesce한다.
- 새 target이 등록된 frame에는 `YIELDED`를 반환해 첫 화면이 resource work보다 먼저 진행될 수 있게 했다.
- `CMainApp::Update()`는 Engine/Character update 뒤, pending Effect spawn commit 전에
  `Advance_ProductCuePreparation()`을 한 번 호출한다.
- 한 target 실패는 같은 revision에서만 latch하고 다음 frame의 다음 target과 기존 prepared target을
  보존한다. revision 변경 시 현재 runtime catalog에 남은 desired ID만 다시 queue한다.

### Renderer transaction과 Tool rollback

- `CEffectDocumentRenderer::Prepare_VisualProgramTarget()`이 같은 Device/Context/revision의 immutable
  DDS/WModel shared-asset session을 frame 사이에 유지하면서 target 하나를 기존 prepared index에 merge한다.
- target build는 shared cache와 prepared index를 stage-copy한다. 성공과 generation identity가 모두
  확인된 뒤에만 maps/session/probe를 한 번에 commit하므로 invalid target과 중간 실패는 이전 identity를
  보존한다.
- 기존 `Prepare_VisualProgramCatalog()` batch transaction은 Effect Tool의 명시적 Publish/Reload에
  남겼다. batch 성공 뒤에만 queue와 budget receipt를 교체하며 catalog rollback 시 이전 revision 전체를
  다시 준비한다.

### Spawn no-I/O

- `CEffectCatalog::Find_Loaded()`를 추가했다. 이 함수는 loaded map만 조회하고 sealed JSON parse나 catalog
  status mutation을 수행하지 않는다.
- `Spawn()`과 `Spawn_Immediate()`는 admission/descriptor, revision별 prepared ID, cache-only document,
  exact document/projection identity와 budget receipt 순서로 검사한다.
- prepared miss에서는 layer insertion, shader/model/DDS/vector-field load와 synchronous document stage가
  발생하지 않는다.

## 자동 검증

- `Publish-Effects.ps1 -Mode Validate -ResourceRoot Client/Bin/Resources`: PASS, runtime Effect 99개.
- `Test-EffectPipeline.ps1`: PASS, publish/validate/fault-injection rollback 계약 통과.
- `Sync-EffectDataProject.ps1 -Check`: PASS, files 1,596 / filters 188.
- Client와 ClientFrontendHarness `.vcxproj/.filters` XML parse: PASS.
- `ClientFrontendHarness --effect-incremental-prewarm-fast` Debug: PASS, failures 0.
  - registration frame yield, duplicate coalesce, 한 front 단위 success/failure, revision reset
  - 실제 runtime 99개 membership과 authoring-only draft 제외
  - cache-only lookup의 first-use JSON load 금지
  - 실제 Lance 두 target의 WARP incremental prepare
  - duplicate build/commit 없음, invalid target rollback, 두 번째 commit 뒤 첫 identity 보존
- Client x64 Debug build/link: PASS.
- Client x64 Release build/link: PASS.
- ClientFrontendHarness x64 Debug build/link: PASS.
- ClientFrontendHarness x64 Release build/link와 focused mode: PASS, failures 0.
- `git diff --check`: PASS, line-ending notice만 존재.

전체 인자 없는 ClientFrontendHarness와 기존 `--effect-runtime-fast`도 시도했지만 이번 diff가 건드리지
않은 DimensionMaster/legacy admission·authored fixture에서 여러 failure를 출력해 전체 PASS로 기록하지
않았다. 현재 publisher가 폐기한 explicit Product admission sidecar를 요구하는 fixture도 포함되어 있다.
관련 증분 계약은 현재 runtime membership에 맞춘 독립 focused mode로 실행 검증했다.

## 남은 경계와 사용자 검증

- 이 구현의 bound는 문서 하나/frame이다. 15 MiB급 단일 문서 하나의 parse/GPU build를 element 단위나
  시간 budget으로 중단·재개하지 않으므로 해당 한 frame은 여전히 길어질 수 있다.
- 준비 전에 발생한 cue는 fail-closed하고 Character cue cursor가 진행된다. 준비 완료 뒤 occurrence를
  소급 재생하지 않는다.
- 에이전트는 Client/UI를 실행하지 않았고 visual 또는 cold-entry timing PASS를 판정하지 않았다.
- 사용자는 Client project를 `Ctrl+F5`로 실행한 뒤 Lobby → Character Select → Lance Master 첫 진입에서
  UI/frame 진행, 준비 중 즉시 스킬의 fail-closed, 준비 완료 뒤 Effect 표시와 같은 revision 두 번째 진입을
  직접 확인해야 한다.
