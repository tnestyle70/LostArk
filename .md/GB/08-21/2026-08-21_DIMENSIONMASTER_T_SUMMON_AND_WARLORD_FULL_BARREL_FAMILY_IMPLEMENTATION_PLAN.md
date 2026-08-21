# 2026-08-21 DimensionMaster T Summon과 Warlord Full Barrel Family 구현 계획서

## 0. 목표와 현재 실측

이번 변경은 서로 다른 두 문제를 한 검증 단위로 닫는다.

1. 차원술사 T `effect.dimensionmaster.skill.2050500.unified`의 `dimension_summon`만 플레이어 캐릭터와 같은 animated character mesh pass로 렌더링한다.
2. 워로드 풀배럴 캐넌을 하나의 저작 family로 읽기 쉽게 묶되, Server HOLD 단계와 세 Product Effect 문서는 합치지 않는다. label resolver 자체는 같은 계약의 multi-stage HOLD에 공통 적용한다.

첨부 화면의 차원술사 summon은 단순 sprite alpha 문제가 아니라 skinned model section이 갈라져 보이는 형태다. 현재 코드를 실측하면 summon은 이미 `CModel`, bone palette, deferred material input, `Shader_VtxAnimMeshBinary.hlsl`을 사용한다. 플레이어 `CPart_Body`도 같은 shader의 pass 0을 사용하지만, T의 exact identity만 `Effect_DocumentRenderer.cpp`에서 별도 MASKED pass 3으로 강제된다.

pass 3과 pass 0의 alpha cutoff 차이만으로는 첨부 화면을 설명하지 못한다. 실제 렌더 단계 차이는 `CEffectObject::Late_Update`가 모든 effect를 BLEND group에만 넣는 데 있다. BLEND는 deferred lighting 뒤의 `MRT_SceneHDR` 2RT 구간인데 animated character shader는 5개의 deferred MRT 출력을 전제로 한다. 그 결과 summon은 본/geometry가 정상이어도 raw diffuse에 가까운 출력을 SceneHDR에 쓰며 캐릭터 lighting 합성을 우회한다. 따라서 새 renderer나 diffuse-only translucent 경로를 만들지 않고 exact summon만 NONBLEND character 단계에서 pass 0으로 그린다. 같은 EffectObject의 particle과 다른 model cue는 기존 BLEND 단계에 남긴다.

WModel 실측은 4 section, 13,806 single-influence vertices, 23 runtime bones, 131 ticks @ 30 TPS이며 normalized bind identity error는 약 `5.68e-14`다. 즉 palette와 bind pose는 내부적으로 정상이고 clock section의 움직임은 authored animation이다. 이번 runtime 원인은 모델 binary가 아니라 렌더 group/MRT 계약이다.

사용자가 워로드 V라고 부른 풀배럴 캐넌의 데이터 정본은 실제로 다음과 같다.

```text
inputSlot: T
skillId: 17240
skillKind: HOLD
stage 1: Start, clip 01
stage 2: Charge, clip 02 -> 03 -> 04(loop)
stage 3: Release, clip 07
Product documents: ba1 / ba2 / ba3
```

여기서 `ba1/ba2/ba3`은 과거 staged-effect stable ID 이름이고 세 번의 LMB BA를 뜻하지 않는다. HOLD는 누르는 시간과 놓는 시점이 Server 권위라 한 문서로 합치면 loop가 중복되거나 release effect가 누락된다. 따라서 runtime data는 보존하고 All Effects에서만 `Full Barrel Cannon -> Start / Charge / Release` family로 표시한다.

현재 사용자가 별도로 손튜닝 중인 다음 문서는 이 변경에서 읽기만 하며 stage/commit하지 않는다.

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050100.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json
```

## G00. 기준선과 실패 경계 고정

### 변경하지 않는 계약

- T authored document의 element, transform, timing, model cue와 animation clip은 자동 rewrite하지 않는다.
- 다른 model cue의 OPAQUE/MASKED/TRANSLUCENT 선택은 바꾸지 않는다.
- 워로드 `PlayerSkills.json`, skillbinding, animevents와 세 authored effect document는 바꾸지 않는다.
- Client/UI는 에이전트가 실행·조작하지 않는다. 자동 검증 뒤 사용자가 직접 T와 풀배럴 캐넌을 확인한다.

### 실패 시 보존

- T model staging, clip lookup, bone palette 또는 material binding 실패는 해당 cue만 fail-close하고 다른 effect occurrence를 오염시키지 않는다.
- Tool family grouping이 owner/phase를 정확히 resolve하지 못하면 기존 individual effect node를 유지하며 임의 문서 합치기나 fallback 이름 변경을 하지 않는다.

## G01. 차원술사 T summon character animated-mesh pass

### 수정 파일

```text
Client/Private/Effect_DocumentRenderer.cpp
Client/Public/Effect_DocumentRenderer.h
Client/Private/Effect_Object.cpp
Client/Public/Effect_Object.h
Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
Tools/ModelAssetConverter/test_verify_dimensionmaster_summon_bind_pose.py
```

`Effect_DocumentRenderer`는 exact T `dimension_summon` identity predicate와 NONBLEND 전용 model-cue render entry를 소유한다. 이 entry는 summon만 플레이어 body와 같은 pass 0으로 그린다. 기존 BLEND `Render_ModelCues`는 같은 exact cue를 건너뛰어 중복 submit을 막는다. OPAQUE pass 2와 TRANSLUCENT pass 4 및 다른 cue는 기존 계약을 유지한다.

`CEffectObject`는 exact summon이 현재 frame에 visible할 때만 NONBLEND와 BLEND 두 group에 자신을 등록한다. NONBLEND 호출은 summon model만, BLEND 호출은 particle/decal/trail 및 summon을 제외한 model cue만 소비한다. active object, playback clock과 prepared document는 하나로 유지하며 두 번째 EffectObject나 별도 runtime을 만들지 않는다.

새 shader, 새 CModel 경로, diffuse-only translucent 우회는 만들지 않는다. character mesh와 summon이 같은 VS skinning, bone matrices, deferred material binding, masked pixel pass를 사용한다는 것이 종료 불변식이다.

기존 `Test_DimensionMasterTRetimedIsolation`은 다음으로 교정한다.

- exact T summon과 generic character MASKED witness가 같은 selected pixel shader를 사용한다.
- summon은 NONBLEND deferred MRT 단계에서만 한 번 submit되고 BLEND model pass에서는 제외된다.
- model-only와 full Play All의 primitive/VS/PS invocation이 정확히 같다.
- elements-only t=0 draw가 0이고 model cue가 중복 submit되지 않는다.
- OPAQUE witness와는 pass/coverage가 분리된다.
- WModel 4 mesh section, 23 runtime bones, animation clip, finite combined matrices와 bind-pose identity를 유지한다.

bind-pose verifier가 현재 receipt의 `bindPose` schema를 읽도록 교정해 stale `bindPoseGate` key 때문에 검증이 무의미하게 실패하지 않게 한다. 이 수정은 asset binary를 재생성하지 않는다.

## G02. 워로드 Full Barrel Cannon Tool family

### 수정 파일

```text
Client/Public/Effect_Tool.h
Client/Private/Effect_Tool.cpp
Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
```

All Effects의 현재 Product cue ownership을 바탕으로 multi-stage HOLD를 공통 semantic family로 묶고, skill 17240은 다음 exact 결과를 가져야 한다.

```text
Warlord
  Full Barrel Cannon | Input T | HOLD
    Start   | clip 01 | effect.warlord.skill.17240.ba1.unified
    Charge  | clip 02 | effect.warlord.skill.17240.ba2.unified
    Release | clip 07 | effect.warlord.skill.17240.ba3.unified
```

stage 2의 clip 03/04는 같은 Charge presentation chain이며 새 Product document를 만들지 않는다. 세 phase node의 Open Editor, Play Full Effect, Save/selected hot reload는 기존 exact Product cue를 그대로 사용한다. label만 의미에 맞게 바꾸고 stable effect asset ID와 disk format은 유지한다. 현재 데이터에서 같은 공통 분기에 들어오는 다른 대상은 LanceMaster 34590 S이며, 이 역시 start/loop/end 세 Product cue가 정확히 대응한다.

focused helper/harness는 다음을 고정한다.

- 17240이 T/HOLD/3 Server stages임을 확인한다.
- phase leader가 01/02/07이고 문서가 ba1/ba2/ba3에 exact 대응한다.
- stage 2 chain 02/03/04를 Charge 하나로 표시한다.
- ACTIVE/COMBO skill은 이 family label 규칙에 들어오지 않고 COMBO의 BA label을 유지한다.
- family grouping 뒤에도 선택 node가 정확한 원본 Product cue/document를 연다.

## G03. 빌드와 자동 검증

다음 순서로 실행하고 실제 결과만 RESULT에 기록한다.

1. Python AST 및 summon bind-pose verifier
2. `ClientFrontendHarness` x64 Debug build
3. `--effect-dm-t-isolation-fast`
4. Warlord Full Barrel family focused mode
5. `Publish-Effects.ps1 -Mode Validate -ResourceRoot Client/Bin/Resources`
6. `Test-EffectPipeline.ps1`
7. Client x64 Debug isolated full build/link
8. scoped `git diff --check`

실행 중인 canonical Client/Server는 종료하거나 교체하지 않는다. 격리 산출물로 링크한 뒤 사용자가 새 빌드를 직접 실행할 수 있는 상태와 정확한 수동 확인 경로를 보고한다.

## G04. 사용자 수동 확인

자동 검증 뒤 사용자가 직접 확인한다.

1. All Effects에서 DimensionMaster T를 열고 summon만 Play Full Effect한다.
2. summon이 플레이어 캐릭터처럼 하나의 정상 skinned surface로 보이는지, 기존 분리/파쇄 형상이 사라졌는지 확인한다.
3. 실제 T 스킬에서도 animation과 model section이 같은지 확인한다.
4. Warlord T Full Barrel Cannon family에서 Start/Charge/Release 세 child를 각각 열고 재생한다.
5. hold 유지 시 Charge loop, release 시 Release effect가 한 번만 나오는지 확인한다.

화면 fidelity는 이 수동 관찰 전에는 PASS로 기록하지 않는다.

## G05. 완료 단위

다음이 모두 성립할 때 이 변경을 하나의 commit으로 완료한다.

- T summon만 character animated-mesh pass를 사용하고 다른 cue pass가 불변이다.
- Warlord runtime 3단계와 세 authored document는 유지되며 Tool에서만 의미 있는 family로 통합된다.
- focused harness와 Debug build가 통과한다.
- publisher validation과 diff check가 통과한다.
- 세 사용자 dirty DimensionMaster 문서가 stage/commit에서 제외된다.
- RESULT가 자동 검증, 사용자 수동 검증 대기, 남은 경계를 분리한다.
