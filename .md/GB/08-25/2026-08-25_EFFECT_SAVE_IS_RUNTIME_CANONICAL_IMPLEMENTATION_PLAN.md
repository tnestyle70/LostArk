# 2026-08-25 Effect Save 단일 정본 런타임 전환 구현 계획서

## 0. 목표와 완료 판정

제품 Effect는 `Data/Effects/EffectCatalog.json`과
`Data/Effects/Authored/<EffectAssetId>.effect.json` 한 벌만 입력으로 사용한다. Effect Tool의 Save가
성공하면 같은 Client의 다음 제품 재생과 다음 Client 실행이 같은 authored 파일을 소비한다.

완료 조건은 다음과 같다.

1. 도화가 Q/A decal과 창술사 제품 cue가 generated runtime seal이 아니라 current authored Transform을 읽는다.
2. Save는 파일 교체와 다음-spawn prepared target 교체를 하나의 transaction으로 처리한다.
3. 활성화 실패는 파일을 compare-and-swap으로 이전 bytes로 복원하고 기존 Product target을 보존한다.
4. 실행 중 occurrence는 생성 당시 immutable document를 끝까지 사용하고 다음 occurrence부터 새 revision을 쓴다.
5. 별도 Effect publish, hash seal, runtime catalog, VisualPrograms sidecar 없이 Debug/Release에서 로드된다.
6. 사용자가 Character Select에서 직접 판정하기 전에는 visual PASS로 기록하지 않는다.

## 1. 실측한 회귀 원인

기존 경로는 Editor와 다음 Client 실행이 다른 파일을 읽었다.

```text
Effect Tool Save
  -> Data/Effects/Authored/<ID>.effect.json
  -> 현재 process의 selected hot reload

다음 Client 시작
  -> Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json
  -> Client/Bin/DataFiles/Effect/Authored/<ID>.<sha256>.effect.json
```

도화가 Q/A의 90도 회전과 창술사 이펙트 미반영은 renderer의 추가 좌표 보정이 아니라 두 파일의 Transform
값이 달랐기 때문에 발생했다. publish를 생략하면 현재 process에서 보던 수정본과 다음 실행의 제품본이 다시
갈라졌다. 따라서 publish 단계를 더 자동화하지 않고 두 번째 제품 파일 자체를 제거한다.

## G00. source-only catalog

### 수정 파일과 책임

| 경로 | 책임 |
|---|---|
| `Client/Private/Effect_Catalog.cpp` | source catalog 전체 parse/validate/stage/commit, direct document와 overlay 로드 |
| `Client/Public/Effect_Catalog.h` | version-neutral direct source와 staged replacement 계약 |
| `Client/Private/Effect_DocumentCodec.cpp` | v13/v15 strict parse/validate/serialize |
| `Client/Public/Effect_AuthoringDocument.h` | v15 typed runtime carrier와 baked history 자료형 |
| `Client/Private/Effect_VisualProgramCorpus.cpp` | 같은 document pointer에서 transient adapter projection 생성 |
| `Client/Public/Effect_VisualProgramCorpus.h` | disk corpus가 아닌 in-memory projection 경계 |

`CEffectCatalog::Load()`는 `EffectCatalog.json`의 direct row가 가리키는 exact authored 상대 경로를 읽는다.
경로는 project data root 아래 regular file이어야 하며 절대 경로, drive-qualified 경로, `..`, ID/path 불일치,
중복 ID를 거부한다. 전체 catalog가 성공한 경우에만 staged map을 commit하고 실패하면 실행 중 catalog를
유지한다. Screen Overlay도 source presentation 문서를 직접 stage한다.

direct kind 이름은 특정 문서 version을 경로 이름에 고정하지 않는 `DIRECT_AUTHORED_DOCUMENT`다. v15의 고급
trail/light packet은 `runtimeExtensions.runtimeCarriers`에 typed payload로 저장하고 같은 document pointer에서
`ADAPTER_PACKET_V1` projection을 만든다. disk sidecar나 corpus는 만들지 않는다.

## G01. Save transaction과 next-spawn hot reload

### 수정 파일과 책임

| 경로 | 책임 |
|---|---|
| `Client/Private/Effect_Tool.cpp` | baseline CAS, atomic save, Product stage/commit, 실패 시 disk rollback |
| `Client/Public/Effect_Tool.h` | active baseline과 저장 상태 |
| `Client/Private/Effect_PresentationService.cpp` | 새 document의 drawable/resource/GPU target stage와 commit |
| `Client/Public/Effect_PresentationService.h` | staged prepared target과 rollback 경계 |
| `Client/Private/Effect_Catalog.cpp` | source replacement stage/commit/restore |

실제 흐름은 다음과 같다.

```text
unchanged-baseline 확인
-> candidate document serialize/validate
-> authored 파일 임시 기록 + atomic replace
-> 같은 EffectAssetId의 source document 재parse
-> transient carrier projection과 drawable/resource closure 검증
-> Presentation target GPU prepare
-> Catalog document와 next-spawn prepared target commit
-> 새 baseline 확정
```

중간 실패 시 저장 직전 baseline과 현재 disk bytes가 일치할 때만 이전 bytes를 원자 복원한다. 다른 writer가
파일을 다시 바꿨으면 덮어쓰지 않고 충돌 이유를 보존한다. Catalog와 Presentation commit 전 실패는 기존
pointer/cache를 유지하고, commit 후 restore가 필요한 경우 staged replacement token으로 이전 상태를 복원한다.

## G02. generated Effect publish 제거

### 삭제 대상

- `Client/Bin/DataFiles/Effect/**` tracked generated catalog/seal 전부
- `Data/Effects/VisualPrograms/effect-visual-program-*.json`
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
- hash seal, derived artifact, rollout publish, runtime direct validation에만 쓰이던 builder/test

legacy runtime donor payload는 제품 입력이 아니므로
`Data/Effects/Imported/LegacyRuntimeDonors` 아래 source evidence로 이동한다. `Client/Bin/DataFiles/Effect`는
`.gitignore`에 넣어 build나 오래된 스크립트가 다시 제품 정본처럼 추가하지 못하게 한다.

`Tools/Build/Invoke-BuildAndRegression.ps1`의 Effect gate는 write/publish 대신
`Tools/EffectPipeline/Validate-EffectSources.ps1`를 호출한다. Gameplay, Map, Navigation, Item과 Valtan gameplay
publisher는 Server 형식 변환을 소유하므로 이 변경에서 제거하지 않는다.

## G03. source validator와 프로젝트 등록

### 새 파일

| 경로 | 책임 |
|---|---|
| `Tools/EffectPipeline/validate_effect_sources.py` | catalog 175 direct row와 source closure, v15 carrier/history, retired artifact 부재 검증 |
| `Tools/EffectPipeline/Validate-EffectSources.ps1` | 저장소 root를 확정해 Python validator 실행 |
| `Tools/EffectPipeline/test_validate_effect_sources.py` | 정상/잘못된 version·ID·path·duplicate·carrier/history·generated artifact 회귀 |

validator는 Product membership과 authored file을 일대일로 join한다. catalog row의 version은
`DIRECT_AUTHORED_DOCUMENT`, 문서는 v13 또는 v15여야 한다. v15 carrier의 owner ID, element reference,
sample/history count와 finite numeric closure를 검증한다. `Client/Bin/DataFiles/Effect`, runtime catalog,
VisualPrograms sidecar, active publisher consumer가 있으면 실패한다. 역사적 receipt 안의 과거 경로 문자열은
제품 consumer가 아니므로 삭제 근거로 사용하지 않는다.

`Sync-EffectDataProject.ps1`는 현재 `Data/Effects` 물리 파일만 `Client.vcxproj`와 filters의 Effect generated
block에 등록한다. 새 C++ 파일은 기존 Client project/filter 항목에 명시적으로 등록한다.

## G04. authored data 동결

`Data/Effects/Authored`의 현재 팀 튜닝본을 source-only catalog와 함께 고정한다. 도화가 31950과 발탄
420633은 v15 inline carrier로 이관한다. 도화가 31460 linear reveal 문서를 Product catalog에 연결한다.
창술사, 도화가, 워로드, DimensionMaster와 발탄의 이미 수정된 authored 문서는 generated seal로 다시
축약하지 않는다. Catalog에 연결되지 않은 실험용 generic DimensionMaster BA 문서는 이 변경에 포함하지 않는다.

다른 팀원이 Git pull만으로 같은 화면을 재현하도록 175개 direct Product 문서에서 실제 도달 가능한
`Client/Bin/Resources` DDS/WModel dependency closure만 선별 추적한다. 전체 Resources pack, 미참조 파일,
추출 원본은 포함하지 않고 별도 runtime manifest를 두 번째 정본으로 만들지 않는다.

## G05. 재발 방지 하네스

`Tools/EffectRenderContractHarness`는 repository source root를 입력으로 사용한다.

1. source catalog direct row와 exact authored path/ID identity
2. 도화가 31950 v15 cascade carrier projection
3. 발탄 420633의 trail/light carrier, baked history, sample closure
4. 도화가 decal/linear reveal, 창술사 portable copy, screen overlay와 world mark
5. old runtime catalog/seal/sidecar가 없어도 동일 runtime parse와 WARP draw
6. Save source semantics와 다음 reload가 같은 authored file을 선택하는지

하네스는 Client UI를 실행하지 않고 software/WARP 및 구조화된 수치까지만 판정한다.

## G06. 문서와 검증

현재 계약을 `CLAUDE.md`, Animation 인계서, Gameplay interface handbook, Unified Data architecture,
발탄 인수인계서와 `gotchas.md`에 반영한다. 모든 문서는 Effect publish를 실행 지침으로 남기지 않는다.

검증 순서는 다음과 같다.

1. 변경 JSON/XML/PowerShell parse와 Python unit test
2. `Validate-EffectSources.ps1`
3. `Sync-EffectDataProject.ps1 -Check`
4. Engine Debug/Release와 `UpdateLib.bat`
5. EffectRenderContractHarness Debug/Release build/run
6. Client Debug/Release build
7. `git diff --check`와 retired publish/runtime path 감사
8. 사용자가 직접 Character Select에서 창술사와 도화가 Q/A 확인

정본 main의 다른 domain 회귀는 해당 기능 owner가 forward-fix한다. 이 Effect 변경은 Valtan Server parser,
GameRoom, Profiler, Sound, Character Select, DimensionMaster BA 별도 작업을 포함하지 않는다.
