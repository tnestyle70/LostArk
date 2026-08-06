# Effect Tool 기능 완성 결과

## 완료 상태

렌더링 품질 고도화와 원작 시각 튜닝을 제외한 Effect Tool 저작 수직 슬라이스를 현재 working tree의 기존 `Effect Document -> Playback -> Renderer -> CEffectObject` 경로 위에서 연결했다. 제품 runtime의 `CEffectCatalog -> CEffectPresentationService -> CEffectObject` 소비 경로는 유지했고 저작 UI에서만 Published 목록과 중복 Load 명령을 제거했다.

완료한 기능은 다음과 같다.

- `PlayerSkills.json + Data/Effects/Authored`를 staged load하여 `class -> input slot/완성 스킬 -> Kind -> Element` All Effects 트리를 구성한다. 검색과 다섯 playable class 필터를 제공하고 하나라도 잘못된 문서가 있으면 이전 트리를 유지한다.
- `Data/Effects/Imported`가 실제로 존재할 때만 Imported 문서를 Data Files에 표시한다. Warlord 또는 다른 비-playable import를 runtime roster에 등록하지 않는다.
- Element Add/Delete/Clear와 트리 선택은 미적용 Detail draft를 보존하며 Apply/Revert 전에는 선택/구조 변경을 거부한다.
- 선택 Element에 Mesh Shape 카드와 Material Template이 선언한 input 카드를 표시한다. 현재 `effect.standard`는 Base/Noise/Mask/Emissive/Dissolve 다섯 input만 등록한다.
- DDS는 실제 SRV를 표시하고 WModel은 기존 `CModel`과 `Shader_VtxMeshPreview.hlsl`을 사용해 128x128 offscreen thumbnail을 생성한다. thumbnail은 프레임당 4개 load와 최대 192개 cache로 제한한다.
- Resource Browser는 선택 stable slot의 file kind와 category/search로 필터하며 선택, Bind Selected, Clear Slot을 제공한다.
- Detail Apply/Revert에 Element visible을 포함했고 visible=false Element는 Playback simulation/draw와 duration에서 제외된다.
- Complete Effect, 선택 Element Solo, 선택 Element Mute는 active 문서를 변경하지 않은 복사본을 같은 `CEffectObject::Stage_Document` 경로로 stage한다.
- Data Files는 New/Save/Save As/Reload/Discard와 Authored/Imported 목록을 제공한다. Save와 Save As는 atomic replace를 사용하고 Save As는 기존 ID 덮어쓰기를 거부한다. Imported source는 중복 ID를 만들지 않도록 고유한 Authored ID로 Save As만 허용한다.
- Model View의 기존 class/weapon/animation/socket/bone/world anchor와 완성 Effect preview를 유지했다.
- authoring format v6에 Element `displayName/groupId/sourceNode/visible`, Material `templateId`, resource `slotId` 경계를 추가했다. v5는 호환 load 후 memory에서 v6으로 승격하고 다음 Save에서 v6으로 기록한다.
- Material Template은 stable slot ID, semantic, 표시 이름, HLSL binding 이름, resource kind, 현재 runtime slot을 함께 선언한다. 근거 없는 custom shader/slot은 등록하지 않았으며 publisher도 미등록 Template을 거부한다.
- runtime catalog는 embedded v5와 v6 header/version 일치를 검사해 둘 다 소비하며 ID/path/dependency hash/필수 binding/rollback 검증을 유지한다.

## 주요 변경 파일

- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Public/Effect_ThumbnailCache.h`
- `Client/Private/Effect_ThumbnailCache.cpp`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Private/Effect_Playback.cpp`
- `Client/Private/Effect_Catalog.cpp`
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `CLAUDE.md`

## 실행한 자동 검증

### 빌드

- Git LFS pointer 상태였던 Debug third-party library만 `git lfs pull --include=...`로 hydrate했다. 해당 파일은 Git dirty 변경으로 남지 않았다.
- `Engine/Default/Engine.vcxproj`, x64 Debug `Build`: PASS. DirectXTK/Effects PDB 부재 경고만 발생했다.
- `UpdateLib.bat Debug`: PASS. Engine header/lib/runtime과 shader를 배포했다.
- `Client/Default/Client.vcxproj`, x64 Debug `Build`: PASS. 새 Effect shader 다섯 개와 기존 Mesh Preview shader compile, `Client.exe` link, runtime DLL 배포를 확인했다.
- 별도 `Client.vcxproj /t:ClCompile`: PASS. Effect Tool, codec, renderer, playback, thumbnail을 포함한 Client 전체 C++ compile을 확인했다.
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`, x64 Debug `Build`: PASS.

### Effect 데이터와 하네스

- `Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe`: PASS, failures 0.
  - 30/60/144 FPS playback 결정성
  - particle/trail tail duration
  - visible=false Element duration/simulation 제외
  - invalid stage rollback
  - `PlayerSkills.json`의 비어 있지 않은 모든 `effectId`와 실제 Authored 문서의 staged codec join
  - valid non-drawable draft 구분
  - 미등록 Material Template 거부
  - v6 metadata/Template atomic save-reload
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`: PASS.
  - v6 정상 publish
  - 미등록 Template, version, path, kind, duplicate ID, missing resource, hash, budget, required binding 거부
  - promote 실패 시 기존 runtime catalog 보존
- `Publish-Effects.ps1 -Mode Validate` + 실제 Resources root: PASS, 11 Effects.
- `Publish-Effects.ps1 -Mode Publish` + 임시 output: PASS, runtime formatVersion 1, Effects 11, embedded v5 11개.
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1` + 실제 Resources root: PASS, `code=50, documents=11, resources=4, cues=14`.
- XML parse: Client project/filter와 ClientFrontendHarness project PASS.
- JSON parse: `Data/Effects` 12개 PASS.
- PowerShell parser: publisher, pipeline test, Effect final audit, ProjectAudit PASS.
- `git diff --check`: PASS. 기존 line-ending 변환 경고만 있고 whitespace 오류는 없다.

### 원본 worktree 전체 ProjectAudit

`Invoke-ProjectAudit.ps1 -ResourceRoot <실제 Resources>`를 실행했다. Effect 관련 두 항목은 모두 PASS였다.

- `effect.g09-authoring-world-runtime-boundary`: PASS
- `effect.g09-cross-document-contract`: PASS

전체 audit는 이번 작업과 무관한 기존 4개 항목 때문에 최종 exit 1이었다.

- `maps.extracted-area-runtime-roots`
- `maps.training-area-contract`
- `maps.character-select-area-contract`
- `actors.dimensionmaster-runtime-animation`

보고서는 `.codex_tmp/ProjectAudit.effect-tool.json`에 생성했다.

### 실제 작업 공간 재반영 검증

다른 세션 종료 후 `C:\Users\user\Desktop\LostArk`에 완료 bundle을 파일별 hunk로 반영했다.
같은 파일의 더 최신 변경은 없었으며 추출 세션이 만든 `Data/Effects/Imported` 네 파일은
보존했다. 첫 전체 audit가 이 네 파일의 project visibility 누락을 정확히 검출했으므로
`Client.vcxproj/.filters`의 `96.DataFiles\Effects\Imported`에 `None` 항목으로 등록했다.
Warlord는 README 표시 경계만 추가됐고 playable roster/runtime onboarding은 추가하지 않았다.

- 실제 작업 공간 `ClientFrontendHarness` x64 Debug build/run: PASS, `failures : 0`.
- 실제 작업 공간 Effect pipeline v6/template/rollback harness: PASS.
- 실제 작업 공간 최종 Effect audit: PASS, `code=50, documents=11, resources=4, cues=14`.
- 실제 작업 공간 `Client.vcxproj` x64 Debug 전체 build/link: PASS.
- 실제 작업 공간 전체 ProjectAudit 재실행: PASS, 69 checks.
  보고서는 `.codex_tmp/ProjectAudit.effect-tool-actual.json`에 생성했다.
- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug`: PASS.
  Engine -> UpdateLib -> Shared -> NetworkProtocolHarness -> ClientFrontendHarness -> Server ->
  Client build와 두 harness, `Server.exe --contract-test`, ProjectAudit 69 checks를 모두 통과했다.
  보고서는 `.codex_tmp/regression/Debug/ProjectAudit.json`에 생성했다.
- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Release`: PASS.
  같은 build/contract 순서와 ProjectAudit 69 checks를 모두 통과했다.
  보고서는 `.codex_tmp/regression/Release/ProjectAudit.json`에 생성했다.
- 정본 자동화는 완료 메시지에서 GUI level validation을 `Framework.slnLaunch`로 안내할 뿐
  Client GUI를 직접 조작하지 않는다. 따라서 Release Development smoke나 F1 육안 확인을
  자동 PASS로 기록하지 않았다.

## 실행하지 못했거나 수동 확인이 남은 항목

- 빌드한 Debug `Client.exe`를 `Client/Default` working directory와 실제 `LOSTARK_RESOURCE_ROOT`로 실행했다. 프로세스는 살아 있고 `Responding=True`였지만 이 실행 환경에서 최상위 HWND를 만들지 않아 F1 키와 ImGui 클릭을 전달할 수 없었다. 프로세스는 종료했으며 이 시도를 F1 smoke PASS로 기록하지 않는다.
- 따라서 실제 화면에서 다음 항목은 수동 확인이 남는다.
  - F1 -> Effect Tool 진입과 ASCII UI 배치
  - All Effects class/skill/Kind/Element 선택
  - DDS/WModel thumbnail 시각 확인
  - 임시 새 Effect의 Bind -> Apply -> Save -> Reload -> Discard 조작
  - Complete/Solo/Mute 전환
  - Model View weapon/bone anchor follow
  - 저장한 Effect를 publish한 뒤 실제 월드 cue에서 재생되는 화면 확인
- 원작 추출 결과의 graph node, Material Instance parameter, direct dependency, Unsupported/Unresolved receipt 및 Imported 초안은 병행 추출 세션 범위이므로 생성하거나 추측하지 않았다.

## 인수인계 경계

- 기존 11개 Authored 문서는 그대로 v5이며 현재 codec/publisher/runtime가 읽는다. Tool에서 저장한 문서부터 v6이 된다. 일괄 변환은 하지 않았다.
- `effect.standard` 외 Template을 추가하려면 추출 근거, stable slot/semantic 정의, 실제 HLSL binding, publisher validation, renderer mapping, save/reload harness를 같은 변경 단위로 닫아야 한다.
- `CEffectCatalog`는 제품 runtime 소비자 때문에 유지한다. All Effects 저작 트리는 이 catalog를 목록 source로 사용하지 않는다.
- 대규모 기존 dirty working tree를 보존했으며 stage/commit/push하지 않았다.

## 2026-08-06 실제 화면 실측 후속 수정 결과

사용자가 제공한 실제 F1 화면을 기준으로 다음 문제를 수정했다.

- All Effects의 112 px 고정 child를 제거했다. 창 최소 크기는 360x380이고 기본 높이는
  420이며, tree는 하단 상태 문구를 제외한 남은 높이를 모두 사용한다.
- Effect Tool 상단의 오해를 부르는 `CreateEffect` 버튼을 제거했다. 기존 데이터 확인은
  All Effects 또는 Data Files의 Authored 행을 로드하는 흐름이고, `Add Element`는 저작
  구조 변경에만 사용한다.
- `Load Selected` 성공 시 Complete filter, sample time 0, loop PLAYING 상태를 즉시
  적용한다. Timeline에 `World Preview: PLAYING/PAUSED`와 `Restart + Play`를 표시한다.
- 기본 preview pivot을 world 원점에서 현재 animation target의 player root로 바꿨다.
  로드 직후에도 root world를 즉시 반영하고 이후 프레임마다 기존
  `CAnimationTargetService -> CEffectObject` 경로로 follow한다.
- `PlayerSkills.json`의 `effectId` 소유 skill을 찾아 playable class target으로 전환하고,
  기존 `<Class>.skillbindings.json`의 첫 clip을 재생한다. Dimension Core의
  `sk_super_instance`가 남아 있던 경우 preview-only prop을 DimensionMaster playable
  body로 교체하고, 2050240은 `pc_sp_m_00_sk_sk_telekinesisthrust_01`부터 재생한다.
- Effect Tool의 target 목록에는 playable 5 class body만 표시한다. Dimension Core와
  Dimension Summon 두 보조 대상은 Animation Tool 전용 목록에는 유지하되 Effect Tool에서
  숨겨, DimensionMaster 아래 두 항목을 Effect class/animation으로 오인하지 않게 했다.
- 추출 세션의 `.imported-effect-draft.json`과 Warlord unbound draft index는 실제 파일이
  존재할 때 Data Files에 `[Imported Draft]`로 표시한다. 이 파일은
  `NOT_PUBLISHABLE_SOURCE_CONVERSION_DRAFT` 경계이므로 Load 버튼을 비활성화하고 matching
  Authored 문서를 사용하라는 문구를 표시한다. Warlord runtime roster는 추가하지 않았다.
- 추출 세션이 최종적으로 남긴 manifest/receipt/catalog/index 54개 중 기존 4개를 제외한 50개를
  `Client.vcxproj/.filters`의 `96.DataFiles\Effects\Imported` None 항목으로 등록했다.
- `ClientFrontendHarness`가 Effect resource 실파일 검증을 할 때 정본 Resources root를
  보도록 `Invoke-BuildAndRegression.ps1`이 harness 실행 구간에만
  `LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`를 설정하고 기존 process 값을 복원한다.

### 후속 자동 검증

- `Client.vcxproj` x64 Debug 전체 build/link: PASS.
- `Client.vcxproj` x64 Release 전체 build/link: PASS.
- `ClientFrontendHarness` Debug: PASS, failures 0. 11개 PlayerSkills effectId와 실제
  Authored 문서/리소스 join을 포함한다.
- `Test-EffectToolFinal.ps1`: PASS,
  `code=50, documents=11, resources=4, cues=14`. 가변 All Effects 높이, load autoplay,
  player-root pivot, skill animation sync, reference-only Imported draft 경계를 추가 검사한다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -SkipBuild`: PASS.
  최신 Debug Client build 뒤 NetworkProtocolHarness, ClientFrontendHarness, Server contract,
  ProjectAudit 69 checks를 통과했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Release`: PASS.
  Engine, UpdateLib, Shared, 두 harness, Server, Client 전체 build와 Server contract,
  ProjectAudit 69 checks를 한 번에 통과했다.
- playable-only Effect target 필터를 최종 반영한 뒤 Debug/Release Client를 각각 다시
  build/link했고, Debug/Release `-SkipBuild` 회귀도 각각 PASS했다.
- Project/filter XML parse와 현재 Git 관리/미추적 Data 164개 대조:
  `expected=164, project=164, filters=164`, missing/extra 0.

### 후속 수동 확인

- Debug Client를 `Client/Default`에서 실행해 HWND 생성, `Responding=True`까지 확인했다.
  자동 데스크톱 포커스/캡처는 다른 최상위 창과 경쟁해 Effect Tool 조작 증거로 채택하지
  않았다. 사용자가 직접 화면 검증하기로 했으므로 다음 항목은 수동 확인으로 남긴다.
  - DimensionMaster 2050240 Authored 행 선택 후 `Load Selected`
  - `World Preview: PLAYING`, `Complete Effect`, sample time 증가 확인
  - animation clip이 `telekinesisthrust_01`로 바뀌고 Dimension Core
    `sk_super_instance` preview가 남지 않는지 확인
  - 현재 캐릭터/preview body root 주변에서 Effect가 loop 재생되는지 확인
  - All Effects tree가 창 남은 높이를 사용하며 이전 112 px 높이로 축소되지 않는지 확인

## 2026-08-06 FPS, All Effects 선택, Live Detail 후속 결과

- F1 `LostArk Developer Tools`의 Diagnostics에 `FPS`와 최근 `Frame ms`를 항상 표시한다.
  FPS는 ImGui rolling average이고 frame time은 최근 프레임의 `DeltaTime`이다. 기존 Profiler
  체크박스는 CPU/GPU 상세 overlay와 JSON capture 경로로 유지했다.
- All Effects 스킬 행은 active 문서를 선택 색으로 표시한다. 다른 스킬은 Data Files와 같은
  Authored load 경로를 사용하고, 현재 active인 스킬을 다시 눌러도 Complete filter로 restage한 뒤
  animation과 Effect를 0초부터 다시 재생한다. 펼침 화살표는 탐색만 하고 스킬 label 클릭이 재생
  명령을 소유한다.
- All Effects와 Data Files는 같은 완성 Effect Document의 의미 탐색/물리 파일 수명이라는 두
  진입점이다. All Effects가 여러 Effect Document를 중첩하지 않는다. 한 스킬을 구성하는 Mesh,
  Sprite, Particle, Decal, Trail은 동일 Document의 여러 Element다.
- 현재 11개 DimensionMaster Authored 문서는 총 13 Element다. 2050010/2050110/2050150/
  2050190/2050200/2050210/2050220/2050240/2050510은 각 1개, 2050500과 2050540만 각 2개다.
  따라서 현재 All Effects에 한 스킬 아래 모든 Kind가 풍부하게 보이지 않는 것은 UI load 실패가
  아니라 Authored 기능 검증본의 실제 구성이다. Imported 추출 초안은 reference-only이며 검수 후
  여러 Element를 가진 Authored 문서로 저장되어야 Complete Effect 재생에 들어온다.
- Effect Detail의 Transform/Velocity, Color, UV, Timing, Lerp와 Kind별 연속 수치를 bounded
  ImGui drag control로 교체했다. drag 중에는 Detail draft를 복사한 임시 Document만 기존
  `CEffectObject::Stage_Document`에 넣고 GPU resource signature가 같으면 renderer resource를
  재사용한다. Apply만 active Document를 바꾸며 Revert는 active Document preview를 다시 stage한다.
  잘못된 수치나 stage 실패는 이전 preview와 active Document를 유지한다.
- 추출 세션 종료 뒤 실제로 나타난 Imported Effect draft 43개와 Warlord action particle catalog
  1개를 `Client.vcxproj/.filters`의 `96.DataFiles\Effects\Imported` None 항목에 추가했다.
  Warlord playable/runtime onboarding은 추가하지 않았다.

### 자동 검증

- `Client.vcxproj` x64 Debug build/link: PASS.
- `Client.vcxproj` x64 Release build/link: PASS. 기존 code-page/DirectXTK PDB warning만 있으며
  Effect Tool 변경 파일 compile과 link는 성공했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -SkipBuild`: PASS.
  두 harness, Server contract와 ProjectAudit 69 checks를 통과했다.
- `Invoke-BuildAndRegression.ps1 -Configuration Release -SkipBuild`: PASS.
  같은 harness/contract/ProjectAudit 회귀를 통과했다.
- standalone `Invoke-ProjectAudit.ps1`: PASS, 69 checks. 보고서는
  `.codex_tmp/ProjectAudit.effect-resource-pacing.json`에 기록했다.
- `Client.vcxproj` x64 Release build/link: PASS.
- `Test-EffectToolFinal.ps1`: PASS,
  `code=50, documents=11, resources=4, cues=14`. FPS/frame time, 같은 스킬 Complete restart,
  active selection 강조와 Detail live-stage/Apply/Revert 경계를 추가 검사한다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -SkipBuild`: PASS.
  NetworkProtocolHarness, ClientFrontendHarness failures 0, Server contract, ProjectAudit 69 checks PASS.
- `Invoke-BuildAndRegression.ps1 -Configuration Release -SkipBuild`: PASS.
  같은 두 harness, Server contract, ProjectAudit 69 checks PASS.
- 현재 Git 관리/미추적 Data와 project/filter 대조:
  `expected=212, project=212, filters=212`, missing/extra 0.

### 수동 확인

사용자가 직접 F1 화면을 검증하기로 했으므로 다음 시각/조작 항목은 자동 PASS로 기록하지 않는다.

- Diagnostics의 FPS와 frame ms를 확인하고 끊기는 순간의 두 값을 기록
- All Effects에서 active 스킬 label을 다시 눌렀을 때 선택 강조, animation restart,
  `World Preview: PLAYING`과 sample time 0초 재시작 확인
- 다른 스킬 label을 눌렀을 때 Data Files `Load Selected`와 같은 완성 Document가 열리는지 확인
- Effect Detail에서 Velocity/Scaling/UV Speed/Lerp end 값을 좌우 drag할 때 Apply 전에도
  world preview가 즉시 바뀌며, Revert가 원값으로 돌아오고 Apply 뒤 Save/Reload가 값을 보존하는지 확인

## 2026-08-06 Resource Browser frame pacing 후속 결과

- 기존 Resource Browser는 2,599개 catalog에서 category 집합과 filtered entry를 매 frame 각각
  다시 만들고 각 entry마다 `filesystem::path`와 문자열을 생성했다. 이제 category는 catalog refresh
  때 한 번 계산하며, visible index는 catalog revision/slot file kind/filter/category가 바뀔 때만
  staged rebuild하고 평상시 frame에는 재사용한다.
- thumbnail cache miss는 기존 frame당 최대 4개 동기 생성에서 2 frame당 최대 1개로 분산했다.
  DDS는 실제 이미지 SRV를 유지하면서 최대 256 dimension mip를 선택하고, WModel은 기존 128x128
  CModel render thumbnail 경로를 유지한다.
- Effect Tool 제목 아래에 FPS와 최신 frame ms를 직접 표시해 F1 허브가 다른 tool 창 뒤에 가려져도
  frame pacing을 확인할 수 있다.
- Profiler JSON에는 `EffectTool.Render`와 `EffectTool.ResourceGrid` scope가 추가되어 전체 Client.Render
  중 tool과 resource grid가 차지한 CPU 시간을 분리할 수 있다.

### 자동 검증

- `Test-EffectToolFinal.ps1`: PASS,
  `code=50, documents=11, resources=4, cues=14`. revision cache, thumbnail budget,
  Effect Tool FPS와 profiler scope 경계를 추가 검사한다.
- `Client.vcxproj` x64 Debug build/link: PASS.

### 수동 확인

- Resource Browser를 열고 고정한 상태, category 변경, 빠른 scroll 세 경우의 FPS/frame ms를 비교한다.
- `F1 -> Diagnostics -> Profiler`를 켜고 CPU/GPU overlay를 확인한 뒤 5~10초 재현 capture를
  `Client/ProfilerCaptures`에 저장한다.
- 실제 GUI frame pacing 비교는 사용자가 직접 검증하기로 했으므로 자동 PASS로 기록하지 않는다.
