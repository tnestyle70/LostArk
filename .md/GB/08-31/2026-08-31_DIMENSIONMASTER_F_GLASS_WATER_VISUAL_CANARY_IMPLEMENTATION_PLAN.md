# 차원술사 F 유리·물방울 시각 canary 구현 계획

## 1. 목표

- 차원술사 F(`2050230`, `pc_sp_m_00_sk_sk_chronorecoil`) 한 번으로 다음 세 시각 레이어 전체를 함께 발생시킨다.
  - 현재 Product의 차원술사 F 전체
  - 현재 Product의 차원술사 W clip2/clip3 전체
  - 발탄 내려찍기에서 가져온 유체 텍스처와 6발 운동을 사용하는 물방울 레이어
- 원본 binary 계산식 복원 여부가 아니라 실제 화면에서 유리 균열/굴절과 물방울의 부피·테두리·burst가 구분되는지를 목표로 한다.
- 원본 F, W, 발탄 authored 문서는 보존하고 F animation occurrence에서 조합한다.
- 최종 visual fidelity 판정은 사용자가 Client에서 직접 수행한다.

## 2. 현재 실측 사실

- F Product effect는 `effect.dimensionmaster.skill.2050230.unified`이며 8개 source element를 가진다.
- F의 기존 animevent는 `chronorecoil` 시작 0ms에 위 effect 하나를 재생한다.
- W Product effect는 clip2와 clip3의 21개 요소 전체를 사용한다. clip3의 Glasshole은 `effect.ue3.glasshole-02.v1` profile 29로 RT0 색과 RT1 distortion을 함께 출력한다.
- 발탄 down-smash의 `source.68619daf746949ce5bee`는 `fx_d_noise_003.dds`와 `fx_e_fluid_006.dds`를 사용하며 6발 원형 burst, 위쪽 속도, 중력을 가진다.
- 기존 `PROJECT_TUNED` opcode 1001/1002는 한 장짜리 coverage 셰이더이므로 두 텍스처 기반 물방울 표면식에는 별도 typed opcode가 필요하다.
- F 문서에 행을 직접 추가하지 않고 기존 F cue, W clip2/clip3 cue, F-owned water cue 네 개로 구성한다.

## 3. 변경 파일

- `Data/Effects/Authored/effect.dimensionmaster.skill.2050230.water-burst.effect.json`
  - 발탄 water donor의 sourceRecipe와 두 유체 텍스처를 쓰는 authored asset 한 개를 추가한다.
- `Data/Effects/EffectCatalog.json`
  - 위 direct-authored asset을 identity-derived path로 등록한다.
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`
  - `chronorecoil`에 기존 F를 유지하면서 W clip2/clip3와 water cue를 추가한다.
- `Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli`
  - 새 물방울 packet validator와 RT0 물 표면 계산식을 추가한다.
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`
  - 새 opcode를 sprite particle 경로에서 명시적으로 dispatch한다.
- `Client/Private/Effect_DocumentCodec.cpp`
  - 새 opcode를 `PROJECT_TUNED_APPROX`의 허용된 typed opcode로 추가한다.
- `Client/Private/Effect_DocumentRenderer.cpp`
  - 새 asset/element/lane/scalar/vector/carrier 계약을 fail-closed로 검증한다.
- `Tools/EffectPipeline/`
  - F-only composition과 물방울 ABI의 정상/변조/중복/범위 이탈을 검사하는 focused test를 추가한다.

## 4. 실행 흐름

```text
F 입력
  -> Server 승인 snapshot
  -> chronorecoil action presentation
  -> 동일 clip의 Product effect cues
       1) 기존 F unified
       2) W clip3 unified @ 450ms: 내부 Glasshole 250ms -> 전역 700ms
       3) W clip2 unified @ 590ms: 내부 core 110ms -> 전역 700ms
       4) F water burst @ 700ms
  -> 네 effect가 각각 lifecycle을 소유
  -> W Glasshole과 water의 RT1을 Deferred resolve에서 scene 굴절로 합성
```

물방울은 작은 signed RT1 굴절을 함께 출력한다. 물 느낌은 다음 RT0/RT1 식으로 만든다.

1. 두 octave의 noise로 mask UV를 작게 흐트러뜨린다.
2. fluid mask를 coverage로 사용하고 카드 가장자리를 feather 처리한다.
3. billboard 중심을 반구 법선으로 근사해 프레넬 rim을 계산한다.
4. 청록 body, 흰색 rim, 얇은 foam band를 합성한다.
5. carrier RGB는 버리고 alpha만 정규화해 발탄 원본의 큰 vertex color가 색을 폭주시키지 않게 한다.
6. signed flow에 coverage를 곱해 제한된 RT1 굴절을 출력한다.

## 5. 구현 단계

### G1. 조합 계약

- F/W 기존 effectref와 timing을 실측한다.
- F 원본 cue를 유지하고 W clip3 450ms, W clip2 590ms, water 700ms cue를 F clip에 추가한다.
- W 두 문서의 내부 핵심 발생과 물방울 burst가 F의 0.7초 타격에 맞도록 cue offset을 고정한다.

### G2. 물방울 authored asset

- 발탄 donor의 두 텍스처와 6발 burst 운동을 F root 좌표계로 retarget한다.
- 별도 stable asset/element identity를 부여하고 donor sourceRecipe를 보존한다.
- 모든 execution lane과 튜닝 상수를 이름·packed index·값까지 고정한다.

### G3. 셰이더·런타임 ABI

- opcode 1003을 `PROJECT_TUNED_APPROX`로 등록한다.
- HLSL과 C++가 동일한 texture/scalar/vector/mask receipt를 검사한다.
- 패킷이 다르면 generic shader로 떨어지지 않고 해당 element를 거부한다.
- Product가 소비하지 않는 dormant material-program registry는 부분적으로 되살리지 않고, 현재 정본인 authored inline execution 경로에서 계약을 닫는다.

### G4. 회귀 방지

- 1003은 F water element 한 occurrence에서만 허용한다.
- 발탄 donor 문서와 F/W 원본 문서의 hash/핵심 identity가 변하지 않았음을 검사한다.
- F clip에서 기존 F, W clip2/clip3, water 네 cue가 정확히 한 번씩만 연결되는지 검사한다.

## 6. 검증

- JSON parse 및 focused Python contract test
- `Validate-EffectSources` validation-only
- compiled shader Product closure를 포함한 Product build
- `git diff --check`
- 수동 확인은 Server + Client를 사용자가 `Ctrl+F5`로 실행하고 Character Select에서 DimensionMaster를 선택한 뒤 F를 한 번 눌러 수행한다.
- 수동 판정 항목은 기존 F 보존, W 유리 레이어 가시성, 물방울 6발 burst, 과도한 흰색 포화/검은 사각형/UV seam 부재다.

## 7. 완료 경계

- 자동 검증 통과는 실행 준비 완료를 뜻한다.
- `visual PASS`, first pixel 승인, 최종 크기·밝기·타이밍 승인은 사용자의 서면 관찰 전까지 미완료로 남긴다.
