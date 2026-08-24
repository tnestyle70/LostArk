# Effect Even Ring Orientation 및 Mesh Ring Fill 구현 결과

## 1. 완료 상태

`origin/main`에서 분기한 `codex/effect-even-ring-orientation`에 Effect Tool, authored document codec, particle playback, mesh effect shader, publisher validation과 실행형 harness를 연결했다.

이번 변경은 다음 계약을 실제 런타임까지 닫는다.

- Sprite Particle의 fixed burst cohort를 링 둘레에 균등 배치한다.
- 각 Sprite를 발탄 중심 기준 방사 바깥/안쪽 또는 접선 시계/반시계 방향으로 독립 회전한다.
- manual generic Mesh Particle의 raw carrier `TEXCOORD0.y`를 이용해 고정 크기 링을 안쪽에서 바깥쪽 또는 반대 방향으로 공개한다.
- Ring Fill progress를 Element-local Timing Life에 따라 선형 보간해 완료 wave 경계에서 정확히 1에 도달시킨다.
- 완료 wave는 별도 element의 Start Delay로 분리 저작한다.

사용자가 작업 중인 `Data/Effects/Authored` 문서와 runtime catalog는 이번 변경에서 수정하거나 Publish하지 않았다.

## 2. 구현 내용

### 2.1 Even Ring 및 개별 Sprite 자세

- `spawnShape.distribution`에 `random`과 `even`을 추가했다.
- full 360도는 `i / N`으로 끝점 중복 없이 배치한다.
- partial arc는 `i / (N - 1)`로 양 끝점을 포함한다.
- `particle.initialOrientation`에 다음 모드를 추가했다.
  - `fixed`
  - `groundRadialOutward`
  - `groundRadialInward`
  - `groundTangentClockwise`
  - `groundTangentCounterClockwise`
- 위치에서 각도를 재추정하지 않고 spawn sample의 원래 ring azimuth를 보존해 initial-position offset과 자세를 분리했다.
- non-fixed 자세는 direct-authored Sprite Particle, Ring, Local Space ON, Billboard OFF에서만 admission한다.

### 2.2 Mesh Ring Fill

- `detail.mesh.ringFill`에 enabled, progress, direction, feather, invert를 추가했다.
- `detail.linearLerp`에 ringFillProgress와 endRingFillProgress를 추가했다.
- `Shader_VtxEffectMeshPreview.hlsl`이 변형 전 raw `TEXCOORD0`을 별도 semantic으로 pixel shader에 전달한다.
- `Shader_EffectCommon.hlsli`가 raw V를 기준으로 coverage를 계산한다.
- progress 0은 완전 비표시, progress 1은 완전 표시이며 중간 경계에 feather를 적용한다.
- Feather는 `progress - feather`부터 `progress + feather`까지의 대칭 경계를 사용한다.
- manual Ring Fill carrier에는 기존 암시적 particle alpha fade를 생략한다. 완료 alpha는 Color/Lerp가 소유하고 particle lifetime을 Timing Life보다 길게 두면 짧은 hold가 가능하다.
- Alpha/Additive가 모두 SrcAlpha blend이므로 coverage는 SceneColor alpha와 Distortion에만 곱해 framebuffer에서 feather가 이중 감쇠되지 않게 했다.
- direct-authored `effect.standard` Mesh Particle, non-Opaque profile에서만 활성화한다. SourceRecipe, SourceMaterial, reconstructed/runtime material에는 neutral disabled constant를 바인딩한다.

### 2.3 Effect Tool UI

- `Ring Distribution`: Random / Even (Fixed Burst)
- `Initial Sprite Orientation`: Fixed / Ground Radial Outward / Ground Radial Inward / Ground Tangent Clockwise / Ground Tangent Counter Clockwise
- `Orientation Offset Degrees`
- `Mesh Ring Fill (Raw Radial V)`
  - Enable Mesh Ring Fill
  - Ring Fill Progress
  - Ring Fill Direction
  - Ring Fill Feather
  - Invert Carrier Radial V
- `Linear Lerp`
  - Lerp Ring Fill Progress
  - Ring Fill Progress End

### 2.4 Manual Mesh/Mesh Particle Scale 손튜닝 회귀 수정

- `Model Import Scale`은 WModel을 생성하는 `CModel` pre-transform이다. authoring live restage의 resource signature에 `Detail.Mesh.fModelPreScale`이 빠져 있어 값과 JSON만 바뀌고 이미 로드된 모델을 계속 쓰던 원인을 수정했다.
- 새 manual Mesh/Mesh Particle은 Valtan 저작 계약과 같은 다음 기준으로 생성한다.
  - Model Import Scale `0.01`
  - Transform Scaling `(1, 1, 1)`
  - Mesh Particle Start/End Size `(1, 1)`
- Transform Scaling과 Start/End Size는 GPU model resource를 다시 로드하지 않고 매 sample particle world matrix에 곱한다.
- Model Import Scale을 바꿀 때만 prepared document와 WModel resource를 다시 만든다. 같은 Import Scale에서 Transform/Size만 다시 바꾸면 준비된 모델을 재사용한다.
- 기존 authored document는 자동 변환하지 않았다. 현재 사용자 scratch 문서에 이미 저장된 `modelPreScale=0.01`과 `transform.scale=0.01` 조합도 보존했다.

## 3. 검증

### 3.1 G01~G03 PASS

- Client x64 Debug Rebuild 및 링크
  - 출력: `Client/Bin/Debug/Client.exe`
  - 최종 증분 링크 시각: 2026-08-24 12:09:26 KST
- `Publish-Effects.ps1 -Mode Validate`
  - 163 Effect catalog entries
  - 171 material-program bindings
  - 5 registry-bound audition effects
- `EffectRenderContractHarness` x64 Debug
  - expected/actual binding count 171/171
  - codec default omission 및 round-trip
  - Even Ring 16개 반경, 중심, 중복 없음, 22.5도 간격과 결정성
  - 4개 orientation mode와 offset
  - Ring Fill 범위 및 unsupported carrier rejection
  - 완료 시점 authored alpha 유지와 Element Timing Life 기반 0 → 1 clock
  - WARP frame `generic-mesh-particle-ring-fill-0.5s`: submitted 1, failed 0, shader pass 1, draw 1
- `git diff --check`: 오류 없음. 기존 line-ending 경고만 존재한다.

### 3.2 G04 Manual Scale PASS / 실행 바이너리 갱신 대기

- EffectRenderContractHarness x64 Debug build: PASS
- EffectRenderContractHarness x64 Debug 실행: PASS
  - expected/actual material binding `171/171`
  - nonuniform Transform Scaling과 Start/End Size가 하나의 Mesh Particle world matrix에 정확히 합성됨
  - Transform/Size restage는 prepared model build와 model disk load를 증가시키지 않으며 submitted world hash를 변경함
  - Model Import Scale `0.01 -> 0.02` restage는 prepared document build와 model disk load를 각각 정확히 1회 증가시킴
  - 같은 Import Scale에서 Transform을 재조정하면 WModel resource를 다시 로드하지 않음
- `test_effect_tool_valtan_saved_rows.py::test_effect_detail_has_one_working_owner_per_manual_tuning_axis`: PASS
  - 새 manual mesh carrier가 Import Scale `0.01`, identity Transform, Mesh Particle Size `1` 기준을 유지함
- `Publish-Effects.ps1 -Mode Validate`: PASS
  - 163 Effect catalog entries
  - 171 material-program bindings
  - 5 registry-bound audition effects
- 관련 파일 `git diff --check`: 오류 없음. 기존 harness line-ending 경고만 존재한다.
- Client x64 Debug Build는 변경된 `Effect_DocumentRenderer.cpp`와 `Effect_Tool.cpp` 컴파일까지 PASS했다. 현재 사용자가 실행 중인 `Client.exe` PID 60664가 출력 파일을 점유해 최종 링크는 `LNK1104`로 중단됐다. 실행 중 Client를 종료한 뒤 같은 Build를 다시 실행해야 G04 수정이 실제 EXE에 들어간다.

### 3.3 전체 EffectPipeline 상태

PowerShell publisher fixture 본체와 이번 신규 Even Ring/orientation/Ring Fill 정상·거부 사례는 PASS했다. 이어 실행된 Python 전체 217 tests는 기존 정본 및 동시 작업 중 데이터 불일치로 6 failures, 6 errors, 1 skipped 상태다.

- 기존 EffectCatalog 정렬 기대 불일치: 3건
- Artist F raw-byte golden 불일치: 2건
- Valtan pattern 실제 34개와 테스트 기대 33개 불일치: 4건
- action binding 실제 139개와 테스트 기대 137개 불일치: 1건
- 작업 중인 FIST_IN_OUT source/runtime payload 불일치: 1건
- 작업 중인 watertrail materializer/ledger 불일치: 1건

어떤 실패도 `distribution`, `initialOrientation`, `mesh.ringFill`, `ringFillProgress`를 원인으로 보고하지 않았다. 이 범위 밖의 기존/동시 작업 데이터를 이번 변경에서 되돌리거나 교정하지 않았다.

### 3.4 2026-08-24 origin/main 재동기화 검증

- 실제 정본 폴더를 PR #203이 포함된 `origin/main` `ef1aa54d`까지 fast-forward한 뒤 이 변경을 다시 적용했다.
- `Client.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64`: PASS
  - `Effect_Tool.cpp`를 포함한 변경 소스가 최신 main 위에서 컴파일됐다.
- `EffectRenderContractHarness` x64 Debug build 및 실행: PASS
  - expected/actual material binding `171/171`
  - submitted `1`, failed `0`, shader pass `1`, draw `1`
- 별도 clean verification worktree의 `Publish-Effects.ps1 -Mode Validate`: PASS
  - 162 Effect catalog entries
  - 171 material-program bindings
  - 5 registry-bound audition effects
- 실제 정본 폴더의 publisher는 사용자가 별도로 저장 중인 `effect.dimensionmaster.skill.2050180.unified` Product 문서와 기존 V1 audition의 source seal 불일치를 정확히 거부했다. 새 Product를 예전 V1 파생본이라고 오인하도록 hash만 바꾸지 않았으며, 이는 이번 Ring 기능과 분리된 authored/V1 계보 정리 항목이다.
- clean worktree의 전체 `Test-EffectPipeline.ps1` fixture는 `Effect/Artist/Textures/fx_k_auratile_02.dds` 물리 리소스가 없는 별도 worktree ResourceRoot에서 중단됐다. 중단 전 이번 변경의 PowerShell 정상·거부 fixture는 통과했다.
- 실제 정본의 최종 Client/Server 링크는 실행 중인 `Client.exe` PID 50728과 `Server.exe` PID 27504를 사용자가 종료한 뒤 수행한다. 에이전트는 실행 중인 사용자 프로그램을 임의 종료하지 않았다.

## 4. 수동 시각 검증 경계

G04 변경 소스는 최신 main 기준 Client compile을 통과했지만 실행 중인 Client가 출력 EXE를 점유해 최신 링크는 대기 중이다. 사용자가 Client와 Server를 종료하고 Debug Build를 다시 실행한 뒤 Effect Tool에서 실제 Valtan resource와 authored effect를 연결한 최종 외형, Import/Transform/Particle Size scale, fill UV 방향, 색, feather와 완료 wave를 육안 판정한다. 자동 visual PASS로 기록하지 않는다.
