# 2026-08-21 DimensionMaster T Summon과 Warlord Full Barrel Family 구현 결과

## 0. 완료 상태

차원술사 T summon의 실제 결함은 WModel, bone palette 또는 diffuse slot이 아니라 render group/MRT 불일치였다. exact summon만 캐릭터와 같은 NONBLEND deferred 5RT와 animated-mesh pass 0으로 옮겼고, 같은 EffectObject의 particle/decal/trail은 기존 BLEND 경로를 유지했다. BLEND model-cue 호출은 exact summon을 건너뛰므로 이중 draw도 없다.

워로드 풀배럴 캐넌은 사용자가 부른 V가 아니라 현재 정본상 `T / 17240 / HOLD`다. `ba1/ba2/ba3`은 평타 세 번이 아니라 legacy stable ID로, 의미는 `Start / Charge / Release`다. Server 단계와 세 Product document는 그대로 두고 All Effects에서 하나의 HOLD family로만 묶었다.

사용자 손튜닝 중인 차원술사 2050100/2050220/2050230 문서는 수정·stage·publish하지 않았다.

## G01. DimensionMaster T exact character-surface lane

### 구현

- `CEffectDocumentRenderer::Has_NonBlendModelCues`가 exact `2050500 / dimension_summon / WModel / clip / MASKED` identity만 선택한다.
- `CEffectObject::Late_Update`가 해당 object를 NONBLEND와 BLEND에 같은 shared lifetime으로 등록한다.
- 첫 NONBLEND 호출은 `Render_NonBlendModelCues`로 summon만 그린다.
- 후속 BLEND 호출은 exact summon을 제외하고 나머지 effect occurrence를 그린다.
- summon은 `Shader_VtxAnimMeshBinary.hlsl` pass 0, deferred material input, bone matrices를 플레이어 character body와 동일하게 사용한다.
- OPAQUE pass 2, TRANSLUCENT pass 4와 다른 model cue는 변경하지 않았다.

### 원인 증거

기존 EffectObject는 BLEND에만 등록됐다. BLEND는 lighting 이후 SceneHDR 구간인데 animated-character shader는 5개 GBuffer MRT 출력을 전제로 한다. 따라서 기존 경로는 raw diffuse에 가까운 출력을 잘못된 단계에 기록했다.

WModel 자체는 다음 계약을 통과했다.

```text
SHA-256: 87186351b63a6c9ee939da2c6f0c13576d667cd56b24f6d3e861974ea31622b6
mesh sections: 4
cooked vertices: 13,806, single influence
runtime bones: 23
animation: 131 ticks @ 30 TPS
maximum normalized bind identity error: 5.684341886109623e-14
```

stale `bindPoseGate` schema를 읽던 Python test는 현재 receipt의 `bindPose`, `animations`, top-level topology 필드로 교정했다.

## G02. Warlord Full Barrel HOLD family

### 현재 exact runtime

```text
Start   : stage 0, clip 01, ba1.unified
Charge  : stage 1, clip 02(300ms) -> 03(300ms) -> 04(loop), ba2.unified
Release : stage 2, clip 07, ba3.unified
```

### 구현

- multi-stage HOLD의 phase role을 `Hold Start / Hold Charge / Hold Release`로 resolve하는 공통 pure contract를 추가했다.
- All Effects는 exact stage마다 Product cue가 하나씩 있을 때만 `HOLD Product Family`를 연다.
- duplicate/missing stage cue는 family로 오분류하지 않는다.
- Saved document와 Product cue child는 phase-local stable effect ID, Open Editor, Play Full Effect와 Save/hot reload 경로를 그대로 사용한다.
- COMBO는 기존 BA label, ACTIVE single-stage는 Effect label을 유지한다.
- 현재 공통 HOLD family 대상은 Warlord 17240과 LanceMaster 34590이며 두 문서 모두 start/loop/end 3단계가 정확하다.
- Warlord JSON, skillbinding, animevents와 authored effect 문서는 수정하지 않았다.

## G03. 자동 검증

### PASS

```text
ClientFrontendHarness Debug build

--effect-tool-preview-fast
  9/9 PASS
  Warlord T/HOLD/3 phase, clip chain, labels, three phase-local cues

--effect-dm-t-isolation-fast
  failures 0
  exact NONBLEND: 5 RT, 6,801 pixels, 13,689 IA primitives, 28,112 PS invocations
  exact BLEND exclusion: 0 pixels, 0 primitives
  generic character MASKED: same shader, 6,801 pixels, 13,689 primitives
  Full Play All: 6,801 pixels, 13,689 primitives, one submit
  elements-only t=0: 0 pixels, 0 primitives

test_verify_dimensionmaster_summon_bind_pose.py
  3/3 PASS

Publish-Effects.ps1 -Mode Validate
  exit 0
  195 catalog entries, visual programs 14 / rows 135
  productMutation=false

Test-EffectPipeline.ps1
  exit 0
  v12 grouped source, v8 particle, v7 model cue, compatibility,
  rejection matrix and rollback PASS

Client x64 Debug isolated full compile/link
  PASS, %TEMP%/LostArkDmTSummonWarlordFamilyDebug/Client.exe

Client x64 Release isolated full compile/link
  PASS, %TEMP%/LostArkDmTSummonWarlordFamilyRelease/Client.exe
  current Engine import library를 temporary output에서 우선 연결했으며 workspace EngineSDK는 변경하지 않음

git diff --check
  PASS, existing LF/CRLF conversion warnings only
```

### 제한 또는 별도 선행 실패

- canonical Debug link는 실행 중인 `Client.exe` PID 42904가 잠가 LNK1104였고 프로세스를 종료하지 않았다. 같은 source를 격리 OutDir에서 full link했다.
- FourClass representative mode는 DM-T verifier에 들어가기 전 기존 `Field-aware Decal reimport ownership changed.` preflight에서 종료된다. T 전용 WARP mode가 더 직접적인 5RT/pass/double-submit 검증을 통과했다.

## G04. 수동 검증 대기

에이전트는 Client/UI를 실행·조작하지 않았다. 사용자는 새 build에서 다음을 확인해야 한다.

1. All Effects > DimensionMaster > T > summon 단독 Play Full Effect
2. 실제 T 스킬
3. summon surface가 character처럼 lighting되고 기존 raw/파쇄 형상이 사라졌는지
4. All Effects > Warlord > T Full Barrel Cannon > HOLD Product Family
5. Start, Charge loop, Release가 각 phase에서 한 번씩 재생되는지

화면 fidelity는 사용자 확인 전까지 수동 검증 대기다.
