# 차원술사 F 유리·물방울 시각 canary 구현 결과

## 현재 결론

차원술사 F(`2050230`, `pc_sp_m_00_sk_sk_chronorecoil`) 한 번에 기존 F 전체, 차원술사 W clip2/clip3 전체,
발탄 유체 리소스를 이용한 6발 물방울 burst가 겹쳐 재생되도록 구현했다. 원본 F/W authored effect와 발탄 donor는
수정하지 않았고, F animation occurrence에서 네 개의 독립 effect cue를 합성했다.

소스, authored 데이터, 새 Debug particle CSO와 compiled-shader closure까지 반영됐다. 다만 결과 작성 시점에는 사용자가
실행 중인 `Client/Bin/Debug/Client.exe`가 출력 파일을 점유하고 있어 최종 Client 링크만 완료되지 않았다. 따라서 현재
`Client.exe`는 이전 바이너리이며, 실행 중인 Client를 사용자가 닫은 뒤 Product 증분 빌드를 한 번 더 통과해야 실제
EXE 기반 수동 확인이 가능하다. 에이전트는 Client를 종료하거나 UI를 조작하지 않았다.

## 실제 합성 타임라인

```text
F 입력
  -> chronorecoil action presentation
  -> 0 ms   : 기존 F unified 전체
  -> 450 ms : W clip3 unified 전체
               내부 Glasshole 250 ms + cue 450 ms = 전역 700 ms
  -> 590 ms : W clip2 unified 전체
               내부 core 110 ms + cue 590 ms = 전역 700 ms
  -> 700 ms : 발탄 유체 리소스 기반 water burst 6발
```

W의 두 핵심 레이어와 새 물방울 burst를 F 타격점인 0.7초에 맞췄다. 각 effect는 독립 lifecycle을 가지므로 기존 F를
덮어쓰지 않고 동시에 렌더된다.

## 물방울 표현 계약

- stable asset: `effect.dimensionmaster.skill.2050230.water-burst`
- stable element: `project-tuned.water-burst.2050230.01`
- typed opcode: `1003`, `PROJECT_TUNED_APPROX`
- 발탄 donor source node: `source.68619daf746949ce5bee`
- Valtan noise: `Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_003.dds`
- Valtan fluid mask: `Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_006.dds`
- burst: 6발, 비 local-space, 위쪽 초기 속도와 아래쪽 중력

셰이더는 두 noise octave로 UV를 흐트러뜨리고 fluid mask, 카드 경계 feather, 반구 billboard normal, Fresnel rim과
얇은 foam band를 조합한다. RT0에는 청록 body와 흰 rim을 제한된 peak로 출력하고 RT1에는 coverage를 곱한 작은 signed
distortion만 출력한다. 발탄 원본 vertex RGB의 큰 값은 버리고 alpha만 carrier로 사용해 흰색 포화를 방지한다.

C++ renderer는 이 opcode를 정확히 위 asset/element 한 occurrence에만 허용한다. texture lane, sampler, scalar/vector
개수와 mask, carrier, motion 값이 계약과 다르면 generic fallback으로 숨기지 않고 staging을 거부한다.

## 변경 파일

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.water-burst.effect.json
Data/Effects/EffectCatalog.json
Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents
Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl
Client/Private/Effect_DocumentCodec.cpp
Client/Private/Effect_DocumentRenderer.cpp
Tools/EffectPipeline/test_dimensionmaster_2050230_glass_water_visual_canary.py
.md/GB/08-31/2026-08-31_DIMENSIONMASTER_F_GLASS_WATER_VISUAL_CANARY_IMPLEMENTATION_PLAN.md
.md/GB/08-31/2026-08-31_DIMENSIONMASTER_F_GLASS_WATER_VISUAL_CANARY_IMPLEMENTATION_RESULT.md
```

Product가 현재 소비하지 않는 dormant material-program registry는 수정하지 않았다. 새 opcode는 기존 Product 정본인
authored inline material execution 경로에만 연결했다.

## 원본 보존 증거

focused contract가 원본 authored 문서의 raw SHA-256을 고정한다.

- F unified: `afab680bd36b4efcc4baf654c4848a1f3571a29cbc338b0ed58fec940de60e09`
- W clip2: `bf09a2f1b87789081a94458e71214ec71a9a433823f8ba8e0bb4d0f3f2aaaf7a`
- W clip3: `f0492a1563f7d14ea43778bcc973c4ab373d6da764ef0eee966fa7de6dd2fc1d`
- Valtan donor canonical element: `9e37907097226da2276c78e73b01eb0780d18aa9f2383e59303fd633f77348c3`

## 자동 검증

| 검증 | 결과 |
|---|---|
| 새 authored asset와 EffectCatalog JSON parse | PASS |
| focused F/W/water contract | 8 tests / PASS |
| `Validate-EffectSources.ps1` | PASS: direct 172, unbound 0, resources 991, generated 0 |
| scoped `git diff --check` | PASS, line-ending warning만 존재 |
| Debug Engine / Shared / Server Product build | PASS |
| Debug particle HLSL compile | PASS, 새 `Shader_VtxEffectParticle.cso` 생성 |
| Debug compiled shader closure | PASS: active producers 23, Client consumers 22, V1/V2 WARP pixels 1352/1352 |
| Debug Client final link | PENDING: 실행 중 `Client.exe` 점유로 `LNK1104` |
| Client visual fidelity | PENDING: 사용자 수동 판정 필요 |

산출물 실측 시각은 다음과 같다.

- 이전 `Client.exe`: 2026-08-31 01:22:07 KST
- 새 `Shader_VtxEffectParticle.cso`: 2026-08-31 01:22:40 KST
- 점유 프로세스: PID 42168, 같은 Debug `Client.exe`

## EXE 반영 후 사용자 수동 확인

1. 현재 실행 중인 Client를 사용자가 직접 종료한다.
2. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를 다시 실행해 Client link까지 통과시킨다.
3. `Framework.sln`의 Debug `Server + Client` profile을 사용자가 `Ctrl+F5`로 실행한다.
4. Lobby에서 Character Select에 들어가 DimensionMaster를 선택한다.
5. F를 한 번 눌러 기존 F, W 유리 레이어, 6발 물방울 burst가 겹쳐 보이는지 확인한다.
6. 검은 사각형, UV seam, 과도한 흰색 포화, 지나친 굴절, 타격점 불일치 여부를 알려준다.

사용자의 관찰 전에는 `visual PASS`, first pixel 승인, 최종 크기·밝기·타이밍 완료를 주장하지 않는다.

## Git 인계 상태

현재 브랜치는 `codex/action-composition-workbench`이며 다른 작업의 대규모 미커밋 변경이 함께 존재한다. 이번 변경은
stage, commit, push하지 않았고 무관한 파일을 정리하거나 되돌리지 않았다.
