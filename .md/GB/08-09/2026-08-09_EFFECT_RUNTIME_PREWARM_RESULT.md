# 2026-08-09 Effect Runtime Prewarm 결과

## 결론

Character Select 전투에서 제품 Effect cue를 처음 재생할 때 발생하던 동기 shader compile과 model/DDS/vector-field 파일 I/O를 `Spawn` 경로에서 제거했다. 입력 가능 상태가 되기 전에 실제 admitted product cue target만 준비하며, 준비되지 않은 Effect는 동기 로드로 우회하지 않고 생성에 실패한다.

이번 변경은 렌더링 품질이나 이펙트 모양을 자동 이미지로 판정하는 작업이 아니다. 코드, 실행 가능한 C++ 하네스, 정적 계약 테스트와 Debug 빌드/링크까지만 검증했다.

## 원인 증거

입력 capture는 `Client/Bin/ProfilerCaptures/profiler_20260809_041503_288_frame901.json`이다.

- frame 93: `cpuFrameMs=4784.2537`
- frame 322: `cpuFrameMs=4995.4969`
- 기존 경로는 cue crossing 뒤 `Spawn -> EffectObject clone -> renderer Initialize/Stage_Document`에서 매번 core shader 6개를 compile했다.
- DimensionMaster A 문서는 spawn마다 non-animated model reference 20개(고유 4개), DDS binding 104개(고유 13개)를 다시 stage했다.

## 구현된 제품 준비 경계

- `CCharacter::Load_EffectCues`가 검증한 `effectref=asset` cue target만 prewarm한다. Source/Imported 진단 문서나 catalog 전체를 무조건 준비하지 않는다.
- renderer core는 D3D device별 한 번만 만들고, prepared document는 `catalog revision + effectAssetId + resource signature`로 식별한다.
- non-animated model과 DDS SRV는 immutable resource로 공유한다.
- animated Model Cue는 prepared prototype만 공유하고 instance마다 `CModel::Clone`한다.
- particle/trail mutable state와 D3D buffer는 instance별로 유지한다.
- active instance가 `shared_ptr<const PREPARED_DOCUMENT>`와 playback bundle을 보유하므로 catalog revision 교체 뒤에도 이전 bundle 수명이 유지된다.
- `Spawn`은 prepared lookup miss에서 layer insertion 전에 fail closed한다. clone 전후 probe가 shader/model/DDS/vector-field/synchronous-stage counter 불변과 prepared attach 1회를 검사한다.

## Vector field revision 격리

- `Effect_Playback.cpp`의 process-global `assetId -> vector field` map을 제거했다.
- 각 `PREPARED_DOCUMENT`가 immutable `CEffectPlayback::PREPARED_RESOURCES`를 소유한다.
- vector field는 prewarm의 지역 staged map에 전부 로드된 뒤에만 bundle로 publish된다.
- runtime particle update는 instance가 보유한 prepared map만 조회하며 파일을 읽지 않는다.
- 같은 asset ID라도 새 catalog revision은 새 bundle을 만든다. 새 준비가 실패하면 출력 bundle과 전역 prepared catalog를 교체하지 않는다.
- C++ 하네스에서 같은 asset ID의 파일 내용을 revision 사이에 바꾸어 새 instance만 새 sample을 보고, active 이전 instance는 이전 sample을 유지하며, 실패한 다음 준비가 이전 bundle에 residue를 남기지 않음을 실행 검증했다.

## 다중 clip cue와 authoritative clock

- 새 순수 C++ 경계 `CActionPresentationTimeline`이 model duration, `playMs`, `playRate`, sequential clip, HOLD loop epoch를 같은 계산으로 처리한다.
- stage wall time에서 현재 pose clip/source time과 각 cue의 absolute wall offset을 계산한다.
- 늦은 첫 snapshot이 이미 다음 clip을 가리켜도 앞 clip에서 교차한 product cue를 생성하고 `(현재 wall age - cue wall offset) * cue clip playRate`로 첫 sample을 seek한다.
- duplicate key는 schema 변경 없이 runtime occurrence ID `stage/clip/cue/loop epoch`를 사용한다.
- `cue_end`가 backlog 시점에 이미 끝났다면 생성하지 않는다. natural cue는 prepared instance를 생성해 authored overshoot로 seek한다.
- DimensionMaster `2050240`의 실제 두 clip binding과 첫 clip `startms=0` product cue를 하네스 fixture에 결합해, 첫 snapshot age `0.5s`에서 두 번째 clip pose와 앞 clip cue catch-up이 함께 성립함을 검증했다.
- fallback Effect는 현재 stage 어느 clip에든 admitted cue가 있으면 생성하지 않아 catch-up cue와 중복되지 않는다.

## frame order와 anchor 보존

- 순서는 `Engine/Character Update -> Effect anchor sync -> service-owned Effect advance/cleanup`이다.
- 새 Effect는 clone 중 seek하지 않고 pending initial sample만 저장한다.
- post-Character player root/source bone을 동기화한 뒤 pending seek를 정확히 한 번 수행하며, 같은 frame의 `dt`를 다시 더하지 않는다.
- snapshot occurrence도 첫 seek 전에 최신 root를 받고, 0초 occurrence가 현재 root를 명시적으로 capture한다.
- authored cue time을 수정하거나 gameplay delta를 clamp하는 방식은 사용하지 않았다.

## snapshot tick wrap

- `ClientReplication`의 단순 `candidate <= previous` gate를 signed modular delta gate로 교체했다.
- Server가 tick `0`을 예약하므로 `UINT32_MAX -> 1`은 정확히 1 tick, `UINT32_MAX - 1 -> 1`은
  정확히 2 tick으로 계산한다. wrap 뒤 도착한 stale `UINT32_MAX` snapshot은 거부한다.
- action age는 같은 signed modular forward 판정 뒤 예약된 `0`을 세지 않는 64-bit delta를 사용해
  `elapsedTicks / 30Hz`로 계산한다.

## 변경 파일

핵심 런타임:

- `Client/Public/ActionPresentationTimeline.h`
- `Client/Private/ActionPresentationTimeline.cpp`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Public/Effect_Playback.h`
- `Client/Private/Effect_Playback.cpp`
- `Client/Public/Effect_Object.h`
- `Client/Private/Effect_Object.cpp`
- `Client/Public/Effect_PresentationService.h`
- `Client/Private/Effect_PresentationService.cpp`
- `Client/Public/Effect_Catalog.h`
- `Client/Private/Effect_Catalog.cpp`
- `Client/Public/Character.h`
- `Client/Private/Character.cpp`
- `Client/Private/ClientReplication.cpp`
- `Client/Private/MainApp.cpp`
- `Client/Private/Effect_Tool.cpp`

프로젝트/검증:

- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Test-EffectRuntimePrewarm.ps1`
- `Tools/ProjectAudit/Test-CharacterActionPresentationTiming.ps1`

## 자동 검증

- `Test-EffectRuntimePrewarm.ps1`: PASS
- `Test-CharacterActionPresentationTiming.ps1`: PASS
- `Test-EffectToolFinal.ps1`: pre-rollout baseline PASS
  (`code=50, documents=17, resources=335, palette=2667, cues=9`). 이후 네 직업 clip-level
  rollout의 최종 document/cue 수는 대응 Four-Class RESULT의 재검증값을 정본으로 사용한다.
- `ClientFrontendHarness --skill-binding-fast`: PASS, failures 0
- `ClientFrontendHarness --effect-executor-fast`: PASS, failures 0
  - vector-field same-ID revision replacement
  - active old bundle lifetime
  - failed prewarm residue 없음
  - exact clip boundary, `playMs`/rate, HOLD loop epoch, tick wrap, DM `2050240` backlog
- `ClientFrontendHarness --effect-runtime-fast`: PASS, failures 0
- Client/ClientFrontendHarness project와 filters XML parse: PASS
- `Client.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64`: PASS, compile error 0
- `Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64`: PASS, `Client/Bin/Debug/Client.exe` 링크 완료
- 이 pre-rollout checkpoint의 `Invoke-ProjectAudit.ps1`: FAIL. 이번 Effect runtime targeted 검사는
  먼저 PASS했지만 당시 공유 워크트리의 다음 세 항목이 전체 감사를 막았다.
  - `maps.product-editor-visual-scope`: Bern/Valtan/MapTool preview 계약
  - `projects.data-source-visibility`: `expected=822`, `project=583`, `filters=583`
  - `rendering.profile-parser-contract`: authored `globalQuality.fxaaEdgeThreshold`가 `[0.0312, 0.333]` 범위를 벗어남

기존 CP949 `C4819`와 DirectXTK PDB `LNK4099` warning은 남아 있으며 이번 변경의 compile/link 실패는 아니다.

## 남은 수동 검증

1. Character Select Server Play에서 DimensionMaster `2050210`과 `2050240`을 첫 입력/반복 입력한다.
2. 새 JSON profiler capture에서 첫 입력과 반복 입력 모두 shader/model/DDS/vector-field/synchronous stage 증가가 0인지 확인한다.
3. 기존 frame 93/322의 4.8~5.0초 `Client.Update` stall이 제거됐는지 비교한다.
4. `2050240`을 의도적으로 늦은 snapshot 조건에서 실행해 앞 clip cue가 현재 player root와 방향에 붙고, pose보다 한 frame 앞서지 않는지 확인한다.

자동 screenshot 또는 이미지 비교는 수행하지 않았다. 따라서 실제 FPS 개선치와 화면상 cue/anchor 합격은 아직 수동 검증 항목이다.
