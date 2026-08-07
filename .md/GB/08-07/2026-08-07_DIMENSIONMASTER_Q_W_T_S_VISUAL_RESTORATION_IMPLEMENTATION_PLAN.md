# 2026-08-07 차원술사 Q/W/T/S 시각 복원 구현 계획서

## 1. 목표

이번 변경의 목표는 차원술사 11스킬 전체를 픽셀 완성으로 선언하는 것이 아니다. 08-07 실캡처에서 화면을 판독 불가능하게 만든 공통 오류를 먼저 제거하고, 원본 부모 Material 증거를 런타임 Shader Profile로 전달하는 반복 가능한 복원 단위를 만든다.

1차 대상은 다음과 같다.

| 키 | 정본 스킬 ID | 현재 실측 | 이번 판정 대상 |
|---|---:|---:|---|
| Q | 2050100 | Particle 10 = grouped 7 / blocked 3 | 검격 carrier, Blackline Aura, Local Crack |
| W | 2050120 | Particle 91 = grouped 53 / blocked 32 / 특화 6 | RGBNoise 화면 포화, 판형 Mask/Alpha |
| T | 2050500 | Particle 81 = grouped 57 / blocked 21 / 특화 3 | FilmNoise 화면 포화, 돔/링 골격 보존 |
| S | 2050220 | Particle 26 = grouped 10 / blocked 13 / circle 3 | Shine/SpriteWave 카드 실루엣 |

전체 base11 기준은 Particle 581 = grouped 근사 381 / fallback-blocked 155 / 특화 재구성 45 / runtime exact 0이다. 따라서 현재 단계의 정확한 명칭은 `Material 복원을 시작할 수 있는 진단·안전 기준 확보 후 첫 유한 Profile 적용`이다.

## 2. 정본과 검증 경계

- 데이터 정본: 원본 추출 계약 → Authored v12 → Assembly/WFX → Runtime Catalog
- Publisher와 Effect Tool은 정본 소비자다. 생성된 JSON을 손으로 교정하지 않는다.
- 원본 Size/Transform/Velocity/Timeline/Anchor는 이번 변경에서 수정하지 않는다.
- 미지원 Material은 흰색/보라색 fallback으로 보이지 않게 fail-closed를 유지한다.
- 08-07 W 캡처는 Lance Master가 선택되어 있으므로 Material/화면 포화 진단에만 사용하고 socket/pivot 판정에서 제외한다.
- 현재 S는 2050220이다. 과거 2050550 S 이미지는 A/B 정본으로 사용하지 않으며 2050220 원작 캡처를 새로 확보한다.
- 자동 테스트 PASS는 픽셀 A/B PASS가 아니다. 수동 검증에는 `Active ID + sample time + emitter id + post on/off + DimensionMaster 선택`을 남긴다.

## 3. G별 구현 계획

### G01. Screen Post 의미 교정

대상:

- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`
- `Client/Bin/ShaderFiles/Shader_Deferred.hlsl`

실측 근거:

- W RGBNoise의 `rgb_str=1`, `powerx=5`가 파싱됐다.
- Playback은 `rgb_str → intensity`, `powerx → secondaryIntensity`로 전달한다.
- Shader가 채널 분리 후 `noise * secondaryIntensity`를 SceneColor에 다시 더해 화면 전체를 흑백 노이즈로 포화시킨다.
- T FilmNoise는 DynamicParameter가 없는데 생성기가 임의로 intensity 1을 만들어 0~2.8685초 전체 화면 노이즈를 실행한다.

수정:

- RGBNoise는 채널 offset만 수행하고 `powerx`를 additive noise gain으로 사용하지 않는다.
- `powerx` 원시 값과 provenance는 `sourcePresentation.parameters`에 그대로 보존한다.
- 실행 문서의 `secondaryIntensity`는 0으로 내린다.
- 그래프 근거가 없는 FilmNoise는 `enabled=false`, intensity 0으로 fail-closed한다.

완료 기준:

- RGBNoise source parameter `powerx=5` 보존.
- runtime secondary 0.
- FilmNoise no-dynamic occurrence 비실행.
- W/T full-screen 흑백 노이즈 0.

### G02. 부모 Material 유한 Profile

대상:

- `Tools/LevelPlacementExtractor/build_effect_source_material_contract.py`
- `Tools/LevelPlacementExtractor/test_build_effect_source_material_contract.py`
- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`

첫 Profile은 부모 경로의 완전 일치 또는 dot-boundary suffix 일치만 허용한다.

| Profile | 원본 부모 | 근거 | 목표 |
|---|---|---|---|
| `effect.ue3.shine.v1` | `fx_m.fx_f_pa_shine_01_0_tr` | base/mask/noise, str/power/noise scalars, noisecolor | S의 직사각 카드 외곽 제거와 빛줄기 실루엣 |
| `effect.ue3.blackline-aura.v1` | `fx_m.fx_j_pa_blacklineaura_01_tr` | base/mask/dissolve, diff/mask color vectors | Q 검격의 mask/dissolve와 금빛/암색 층 |
| `effect.ue3.local-crack.v1` | `fx_m.fx_j_me_localcrack_01_tr` | mesh+dissolve, in/out/reflection color vectors | Q와 공통 crack mesh를 흰 판 없이 표시 |

Profile index는 기존 0~6 뒤의 7~9를 사용한다. Profile을 찾지 못하거나 필요한 carrier가 없으면 기존 fallback-blocked를 유지한다.

완료 기준:

- 정확한 부모 경로만 승격.
- Renderer/Codec/Shader가 같은 Profile 집합을 소비.
- Sprite는 alpha/mask/dissolve로 외곽을 자르고 Mesh는 mesh binding을 계속 필수로 요구.
- runtime exact로 과장하지 않고 semanticStatus는 `RECONSTRUCTED_PROFILE` 유지.

### G03. 정본 재생성·발행

1. Source Material contract 단위 테스트
2. Imported presentation 단위 테스트
3. Q/W/T/S 정본 재생성 및 결과 수치 감사
4. Authored 승격
5. 16 Assembly / WFX Component 재컴파일
6. Publisher Validate/Publish 및 rollback fixture

재생성 전후에 다음을 비교한다.

- Particle/Light/ScreenPost/Decal/ModelCue 개수 불변
- malformed source LUT 0
- normal/data texture → Base 오배정 0
- unsupported reflection cubemap → 2D Base 승격 0
- Screen Post enabled/profile/intensity 분포
- 새 Profile occurrence 수와 fallback-blocked 감소량

### G04. 자동 빌드·회귀

- Python focused tests
- Effect executor/runtime/frontend harness
- Client x64 Debug build
- Client startup smoke: `Client/Default` working directory, 12초 생존
- Effect Tool final harness
- ProjectAudit
- `git diff --check`

실행 중인 `Client.exe`가 산출물을 잠근 경우 이 저장소 Debug Client만 확인 후 종료하고 재빌드한다.

### G05. 수동 A/B와 다음 체크포인트

수동 순서:

1. W post off/on: full-screen noise가 post 원인인지 확인
2. T FilmNoise sample: 화면 static 제거, dome/ring 보존 확인
3. S emitter Solo: Shine와 SpriteWave를 구분하고 raw card 외곽 확인
4. Q Blackline/Local Crack Solo: mask/dissolve와 색층 확인
5. E 2050160 `fm_h_box_01_1.wmodel` / `fx_l_me_icesurfacee_01_tr` Solo

E의 보라색 상자는 원작에도 있는 문 코어 geometry 후보이므로 전역 scale을 바꾸지 않는다. 다음 Profile 후보는 `fx_mm_hole_06_tr`, `fx_d_me_master_01_ma`, `fx_d_pa_master_01_tr`이며 실제 모델 내장 MI와 ResourceBinding을 확인한 뒤에만 추가한다.

## 4. 금지 사항

- 581 Particle 수작업 재제작
- 전역 Scale/Pivot 선행 조정
- 부모 이름 일부가 비슷하다는 이유만으로 Profile 승격
- normal/bump를 증거 없이 Base 또는 Noise로 사용
- 미지원 Material을 white fallback으로 표시
- 구 R 2050190, 구 S 2050550, 구 A/D 키 계약을 정본 판정에 사용

## 5. 완료 정의

이번 구현 단위의 자동 완료는 코드·데이터·발행·빌드·스모크가 모두 통과했을 때다. 시각 완료는 별도이며, 새 고정 캡처에서 W/T full-screen noise가 사라지고 Q/S의 대상 emitter가 raw opaque card 없이 보이는 것을 확인한 뒤 기록한다. 그 결과가 실패하면 수치를 숨기지 않고 Profile 식과 해당 emitter ID를 다음 반복의 입력으로 남긴다.
