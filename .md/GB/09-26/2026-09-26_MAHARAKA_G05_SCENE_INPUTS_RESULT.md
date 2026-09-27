# 2026-09-26 마하라카 복원 G05 전반부 — 원본 scene 입력 추출과 후보 준비 결과

설계서 `.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md` G05의 전반부다.
**정본 `Data/Rendering/**`과 `Client/Bin/DataFiles/Rendering/**`에 한 바이트도 쓰지 않았다.**
후보와 diff만 만들었고 적용·연결은 사용자 승인 후 별도 단계다.

## 1. 이번 G에서 실제 바꾼 파일

제품·저작 파일 변경 **0건**. 새로 만든 것은 후보 폴더와 이 문서뿐이다.

```
out/MaharakaContinuation_20260926_193455/G05/
  enumerate_exports.py            export class 전수 열거기
  decode_environment.py           PostProcessSettings 등 raw struct 디코더
  build_scene_candidate.py        후보·diff·근거표 생성기
  ExportInventory/                class 목록 + 환경 export dump 3개
  Environment/environment-inputs.json
  ambient-emitters.json
  RenderingProfiles.candidateA.json / .diff
  RenderingProfiles.candidateB.json / .diff
  scene-field-provenance.json
  candidate-summary.json
```

무변경 증거 (G00 baseline 해시와 동일):

| 파일 | SHA-256 | 상태 |
|---|---|---|
| `Data/Rendering/Authored/RenderingProfiles.json` | `39add24e044a7ff1922e91ea08eb2e9ac52d15e016096de655f3d75dd6a4f251` | 불변 |
| `Client/Private/LevelRegistry.cpp` | `dfd20fccc830eb2740d204b324b8874f56a3b447fc43ee300c90cef98591e37c` | 불변 |

`git diff --stat -- Data/Rendering Client/Bin/DataFiles/Rendering` → 빈 출력.
`Publish-RenderingProfiles.ps1`을 `-Mode Publish`로 실행하지 않았다. revision 85 그대로,
`qualityOverride` 20키 그대로, 기존 27 profile 전부 그대로다.
`git status --short` 항목 수는 시작 165 → 이 문서 추가로 166이 된다.

## 2. 원본 근거

### 2.1 환경 액터는 `_PS` package에 있다 (전수 열거로 확정)

세 레벨 package의 export **25,119개**를 class별로 전수 열거했다.

| 논리 package | 물리 UPK | export | 고유 class | 버전 |
|---|---|---|---|---|
| `LV_OCN_EVENTIS_MHP_SL01` | `645QRFK5UT4TKQLJ5FDEY5KJ63A.upk` | 8,977 | 39 | 868 |
| `LV_OCN_EVENTIS_MHP_LAND01` | `867STHM7WV6VMSNL7HFG07M83MO5C.upk` | 15,033 | 26 | 868 |
| `LV_OCN_EVENTIS_MHP_PS` | `423OPDI3SR2RIOJH3DBCW3IWH.upk` | 1,109 | 40 | 868 |

세 UPK 모두 `ReleasePC/Packages/` 아래에 실재한다. 환경 액터는 배치가 아니라 다음 export다.

| class | 위치 | 개수 |
|---|---|---|
| `exponentialheightfog` + `component` | PS export 106 / 107 | 각 1 |
| `dominantdirectionallight` + `component` | PS export 44 / 45 | 각 1 |
| `efenvironmentinfodata` | PS export 53 | 1 |
| `efenvironmentinfovolume` | PS export 54 | 1 |
| `efselfcameraenvironmentinfo` | PS export 101 | 1 |
| `worldinfo` | PS 1108, SL01, LAND01 | 각 1 |
| `lightmassimportancevolume` | PS export 220 / 221 | 2 |
| `winddirectionalsource` | PS export 1106 | 1 |
| `skylightcomponent` | SL01 export 3633 `skylight_1_lc` | 1 |
| `pointlightcomponent` / `spotlightcomponent` | SL01 | 32 / 1 |
| `emitter` + `particlesystemcomponent` | SL01 | 각 105 |
| `eftranslucentvolume` | SL01 export 138~143 | 6 |
| `landscapecomponent` | LAND01 | 46 |

`lightmassimportancevolume` 두 개가 y≈9,747 cm와 y≈98,835 cm에 있다. 섬 범위 규칙
(source y > 50,000 cm)이 레이싱 트랙과 섬을 나눈다는 것과 일치한다.

### 2.2 `PostProcessSettings`를 실제로 디코드했다

공용 reader(`extract_ue3_placements.py:1010`)는 `postprocesssettings`를 허용 목록에 갖고 있지
않아 32바이트 hex로 잘라낸다. 그래서 property stream을 직접 걸어 payload 전체를 보관하고
`parse_tagged_properties_at(payload, names, 0, "postprocesssettings")`로 중첩 파싱했다.
**네 struct 모두 잔여 바이트 0으로 소비**됐다 — 디코드가 맞다는 강한 증거다.

| struct | 위치 | 크기 | 필드 | 잔여 |
|---|---|---|---|---|
| `WorldInfo.defaultPostProcessSettings` | PS 1108 | 431 B | 15 | 0 |
| `efEnvironmentInfoVolume.settings` | PS 54 | 356 B | 12 | 0 |
| `efSelfCameraEnvironmentInfo.ppSettings` | PS 101 | 540 B | 20 | 0 |
| `efSelfCameraEnvironmentInfo.heightFogSettings` | PS 101 | 276 B | 9 | 0 |
| `efSelfCameraEnvironmentInfo.lightShaftSettings` | PS 101 | 228 B | 7 | 0 |

작성 중 자기 버그를 하나 잡았다. 처음 walker는 파싱에 성공하는 **첫** offset을 받아 export 101
에서 중첩 struct 내부를 outer 필드로 오인했다. 공용 파서의 권위 key set과 일치하는 offset만
받도록 고친 뒤 정상화됐다. 이 오류는 결과에 남지 않았다.

### 2.3 런타임 권위는 `efEnvironmentInfoData`다 (PS export 53)

`ehf_override = true`, `ddl_override = true` 둘 다 켜져 있으므로 이 블록이 component 값을
덮는다. **component와 값이 다르다** — 이 구분이 중요하다.

| 필드 | EnvironmentInfoData (권위) | Component (PS 107) |
|---|---|---|
| fog density | 0.10000000149011612 | 0.10000000149011612 |
| fog heightFalloff | **3.0** | 0.699999988079071 |
| fog maxOpacity | 0.20000000298023224 | 0.20000000298023224 |
| fog startDistance | 1600.0 cm | 1600.0 cm |
| opposite brightness | **2.0** | 0.5 |
| opposite color | **RGB(122,182,255) a=255** | RGB(119,213,255) a=3 |
| inscattering color | **RGB(195,101,245) a=0** | RGB(126,246,255) a=3 |
| inscattering brightness | (없음) | 0.5 |
| 태양 brightness | 1.399999976158142 | — |
| 태양 color | RGB(255,254,203) a=0 | — |

fog `startDistance`가 1600 cm = **16 m**로, 엔진 `fog.startDistance` 기본 계약값과 정확히 같다.
component의 `fogheight` 1906.4862060546875 cm만 override 블록에 없어 component에서 가져왔다.

### 2.4 색 보정 LUT가 이미 설치돼 있었다

`WorldInfo.defaultPostProcessSettings.colorgrading_lookuptable`과
`efSelfCameraEnvironmentInfo.ppSettings.colorgrading_lookuptable`이 모두 import `-90`을 가리키고,
그 import는 `lv_ocn_dookyis.tex.lv_ocn_dookyis_lut`이다.

설치본 `Client/Bin/Resources/Map/Lighting/Maharaka/lv_ocn_dookyis_lut.dds`는 16,512 바이트,
**256×16, mip 1, 무압축**이다. 엔진 `sourcePostProcess.colorGradingLut` 계약(256×16 linear 8bit)과
일치한다. G03/G04가 "lightmap이 아닌 LUT 하나만 있다"고 남긴 것의 정체가 이것이다.
`worldpostprocesschain`은 `efpostprocess.postprocesschain.defaultscenepostprocess`다.

### 2.5 태양·하늘

- `DominantDirectionalLight` (PS 44): location (12495.0, 8892.0009765625, 1924.84423828125) cm,
  rotation pitch −51.04248046875°, yaw 209.1357421875°, roll −417.89794921875°.
- component (PS 45)는 serial 836,217 바이트인데 **property stream을 앞 4,096 바이트에서 찾지
  못했다**. 밝기·색·채널 flag를 읽지 못했으므로 `ddl_brightness`/`ddl_lightcolor`를 사용했다.
- `SkyLightComponent` (SL01 3633 `skylight_1_lc`): `lowerbrightness` 0.30000001192092896,
  `lowercolor` RGB(223,236,255) a=0. **상단 반구 brightness/lightcolor는 직렬화되지 않았다** →
  class default이며 읽지 않았다. unresolved다.

## 3. 원본 광원은 전부 lightmap에 베이크돼 있다 (이중 가산 위험 확정)

설계서가 경고한 항목의 실측 답이다. SL01 광원 component **34개 전수**가
built-into-lightmap 플래그를 true로 갖고 있다.

**저장된 property 이름은 소문자 `bhaslighteverbeenbuiltintolightmap`이다.** package name table
자체가 cooker에 의해 전부 소문자이고(앞 400개 중 대소문자 혼용 0건),
`extract_ue3_placements.parse_fname`은 `names[index]`를 그대로 돌려주므로 reader가 접은 것이
아니다. 산문에서 쓰는 CamelCase `bHasLightEverBeenBuiltIntoLightMap`은 UE3 `ULightComponent`의
관례 철자를 내가 복원한 표기이며 **이 package가 증명하는 철자가 아니다.**
대소문자 구분 grep은 0건이 되므로 검색할 때 소문자를 쓴다.

| class | 값 | 개수 | export index |
|---|---|---|---|
| `pointlightcomponent` | true | 32 | 500~531 |
| `skylightcomponent` | true | 1 | 3633 `skylight_1_lc` |
| `spotlightcomponent` | true | 1 | 3634 `spotlight_0_lc` |

전부 `LV_OCN_EVENTIS_MHP_SL01` (`645QRFK5UT4TKQLJ5FDEY5KJ63A.upk`)이다.
34개 모두 `lightmapguid`를 갖고, `lightmasssettings`는 skylight만 없어 33개가 갖는다.
brightness 분포는 0.3(1, skylight), 0.5(1), 1.5(4), 2.0(12), 3.0(6), 3.5(9), 5.0(1, spotlight)이다.

근거 파일과 재현 명령:

- export별 전체 표: `out/MaharakaContinuation_20260926_193455/G05/baked-light-evidence.json`
- 원본 dump: 같은 폴더 `ExportInventory/LV_OCN_EVENTIS_MHP_SL01.env-exports.json`

```
python -c "import json,collections;d=json.load(open(r'out/MaharakaContinuation_20260926_193455/G05/ExportInventory/LV_OCN_EVENTIS_MHP_SL01.env-exports.json',encoding='utf-8'));L=[e for e in d['exports'] if e['class'] in ('pointlightcomponent','spotlightcomponent','skylightcomponent')];print(len(L),collections.Counter((e.get('properties') or {}).get('bhaslighteverbeenbuiltintolightmap',{}).get('value') for e in L))"
```

출력: `34 Counter({True: 34})`

**결론:** G04가 RNM lightmap 729 component를 설치하면 이 34개 광원의 기여가 lightmap에 이미
들어 있다. 현재 설치된 33개 maplights를 동적 광원으로 그대로 함께 켜면 **같은 빛을 두 번
더한다**. RNM 설치와 33 dynamic maplights는 같은 원본 광원에 대해 상호 배타적 기여다.
어느 pass에서 한 번 적용할지 정하는 것이 G05 후반부의 선결 조건이다.
`lightingchannels`는 34개 중 1개만 직렬화됐다(나머지는 class default).

## 4. 상시 맵 particle — 105개, 3중으로 끊겨 있다

SL01의 `emitter` 105개를 actor → `ParticleSystemComponent` → `ParticleSystem` template으로
조인했다. **105개 전부 섬 범위 안**(source y > 50,000 cm)이고, 1개만 template 조인에 실패했다.

| template | 개수 |
|---|---|
| `bfx_low_06.water.par_b_wave_001` | 17 |
| `bfx_low_02.glow.par_b_glow_param_001` | 17 |
| `bfx_low_06.water.par_d_fall_w2_h2_001` | 13 |
| `bfx_low_06.water.par_d_geyser_01` | 9 |
| `bfx_low_06.water.par_b_lutwater_004` | 9 |
| `bfx_low_06.ocn.par_d_ocn_wave_01` | 9 |
| `bfx_low_06.water.par_b_lut_waterfall_001` | 6 |
| `bfx_low_06.water.par_a_h_waterwave_001` | 5 |
| `bfx_low_02.fly.par_g_butterfly_001` | 5 |
| `bfx_low_12.fire.par_g_candle_001` | 3 |
| `bfx_low_06.water.par_b_mysticwater_002` | 2 |
| `bfx_low_06.water.par_d_fallmist_w3_002` | 2 |
| `bfx_low_06.water.par_d_fallsplash_w3_006` | 2 |
| `bfx_low_06.water.par_d_sahill_risewater_02` | 2 |
| `bfx_low_06.water.par_d_fall_w3_h3_001` | 2 |
| `bfx_low_03.lensflare.par_c_lightflare_07` | 1 |
| 조인 실패 | 1 |

물 계열이 78/105이고 나머지는 glow 17, 나비 5, 촛불 3, 렌즈플레어 1이다. 원본 package는
`bfx_low_02`, `bfx_low_03`, `bfx_low_06`, `bfx_low_12` 공용 BFX다(마하라카 전용 아님).

**`bAutoActivate`를 직렬화한 emitter가 하나도 없다.** 따라서 "상시 켜짐"은 UE3 `Emitter`
class default에 의한 추론이며 source-explicit flag가 아니다. 그대로 기록했다.

이것은 level-active 환경 효과다. action-triggered인 `FX_MN_ISMP_00`(얼굴·물줄기·지면·종료)는
NPC occurrence 소유이며 G06 범위다. 여기에 넣지 않는다.

### 4.1 끊긴 지점 3개와 연결 소유자

| 단절 | 확인 방법 | 결과 |
|---|---|---|
| Level 호출 경로 | `grep Load_AmbientArea Client/Private/Level_Development.cpp` | **0건** |
| MapCatalog 선언 | `LV_OCN_EVENTIS_MHP` 키 목록 | `effects`/`sourceEffects` **없음** (Bern은 둘 다 있음) |
| 저작 문서 | mapeffects 문서 | **없음** |

Bern의 기존 소유자(그대로 재사용할 대상):

| 단계 | 위치 |
|---|---|
| Prepare/Commit | `Client/Private/Level_Bern.cpp:409~411` — `CMapEffectPresentationRuntime` 생성 + `Load_AmbientArea` |
| Update | 같은 파일 `:522` — `Update_LevelPresentation(fTimeDelta)` |
| Clear | 같은 파일 `:349` — `Clear()` |

Area는 `LV_OCN_EVENTIS_MHP`만 선택한다. 코드 연결은 이번에 하지 않았다.

## 5. scene profile 후보 두 개와 diff

정본은 엔진 자체 serialiser 형식(CRLF, 2-space, inline array)이다. `json.dumps`로 다시 쓰면
1,289줄이 재포맷돼 diff가 오염된다(실제로 첫 시도에서 발생). 그래서 **정본 텍스트를 그대로 두고
profiles 배열 끝에 텍스트로 삽입**하는 방식으로 바꿨다.

| 후보 | 내용 | 추가 | 삭제 | profile 수 | Validate |
|---|---|---|---|---|---|
| A | 원본 fog + light만, `qualityOverride` 없음 | +46줄 | **0줄** | 28 | PASS |
| B | A + 원본 post-process·bloom·LUT `qualityOverride` | +68줄 | **0줄** | 28 | PASS |

**두 후보 모두 삭제 0줄 — 기존 값이 한 줄도 바뀌지 않았다.** revision 85 유지, 기존 27 profile
불변. `Publish-RenderingProfiles.ps1 -Mode Validate -SourcePath <후보>`로 둘 다 PASS했고,
같은 명령으로 정본도 여전히 PASS한다.

후보 A의 `scene.maharaka.source-day.v1` 실제 값:

```
light.direction  [-0.549188205, -0.777612341, 0.306123117, 0]
light.diffuse    [1.39999998, 1.39450978, 1.11450978, 1]
light.ambient    [0.262352952, 0.27764707, 0.300000012, 1]
fog.enabled      true
fog.color        [0.956862745, 1.42745098, 2, 1]
fog.density      0.100000001
fog.heightFalloff 3
fog.topHeight    19.0648621
fog.startDistance 16
fog.maximumOpacity 0.200000003
fog.sourceExponential.inscatteringColor [0.382352941, 0.198039216, 0.480392157, 1]
fog.sourceExponential.lightDirection    [0.549188205, 0.777612341, -0.306123117, 0.707106781]
```

후보 B가 더하는 것: `bloomThreshold` 0.4, `bloomIntensity` 1, `bloomTint`
[1, 0.698039216, 0.623529412, 1], `ssaoPower` 1.20000005,
`sourcePostProcess.colorize` [1, 1, 1.15], `midtones` [0.9, 1, 1],
`colorGradingLut` `Map/Lighting/Maharaka/lv_ocn_dookyis_lut.dds`.

### 5.1 필드별 근거 (`scene-field-provenance.json`)

**source-exact** — fog enabled/density/heightFalloff/maximumOpacity, bloom threshold·scale·tint,
`scene_colorize`, `scene_midtones`, LUT 경로, `ssaoPower`.

**converted (공식 명기)** — `fog.startDistance`·`fog.topHeight`는 cm × UNIT_SCALE 0.01,
`light.direction`은 UE3 forward `(cos p cos y, cos p sin y, sin p)` 뒤 저장소 `(x, z, −y)` swap
(`extract_ue3_map_lights.py`와 같은 공식), 색은 RGB/255 × brightness.

**unresolved** — 아래 8항목.

1. **색 밝기 배율 규약.** 엔진 `fog.color`/`light.diffuse`는 범위 없는 float4이고 기존 Bern
   profile은 `fog.color`를 6.0까지 저장한다. RGB/255 × brightness 외에 추가 배율이 규약일
   가능성이 있으며 마하라카 원본만으로는 도출되지 않는다.
2. `sourcePostProcess.toneScale` — `boverride_scene_tonemapperscale = true`인데 값이 직렬화되지
   않았다(class default 미독). 후보는 1로 두었다.
3. `sourcePostProcess.toneToe` — 같은 사유. 후보는 1.
4. `sourcePostProcess.toneRange` — 두 struct 어디에도 대응 UE3 필드가 없다. 후보는 8.
5. `sourcePostProcess.desaturation` — 직렬화 없음. 후보는 0.
6. `sourcePostProcess.highlights` — `boverride_scene_highlights = false`이므로 원본이 override
   하지 않는다. 후보는 [1,1,1].
7. SkyLight 상단 반구 brightness/color — 직렬화 없음.
8. `dominantdirectionallightcomponent` (PS 45, 836,217 B) property stream 미발견.

**현재 profile에서 가져온 것** — `scene.development.neutral.v1`의 `shadow.*`,
`light.specular`, fog의 drift·wind·patch 9필드, 세 multiplier. 마하라카는 **지금 이 profile을
쓰고 있으므로** 그 project-only 필드를 유지하는 것이 진짜 최소 변경이다. Bern 복사도, 기본값
채우기도 아니다. drift/wind/patch 9필드는 UE3 `ExponentialHeightFog`에 대응 필드가 없다.

**이 후보의 어떤 값도 bern profile이나 bern region에서 복사하지 않았다.**

### 5.2 아직 미결정인 두 번째 환경 세트

`efSelfCameraEnvironmentInfo`(PS 101)가 `boverrideppsettings = true`,
`boverrideheightfogsettings = true`, `buselightshaft = true`와 함께 **두 번째 값 세트**를 갖는다.

| struct | 주요 값 |
|---|---|
| `ppSettings` | bloom_threshold 0.4, dof_maxfarbluramount 0.8, dof_focusinnerradius 4000, LUT override true |
| `heightFogSettings` | density 0.01, heightFalloff 0.5, maxOpacity 0.5, startDistance 1600, oppositeBrightness **3.0**, oppositeColor RGB(161,216,255), inscatteringBrightness **2.0**, inscatteringColor RGB(195,101,245), lightTerminatorAngle 40 |
| `lightShaftSettings` | rotation pitch −18.41° yaw 275.18°, bloomScale 40, bloomThreshold 0.5, bloomTint RGB(255,244,230), occlusionDepthRange 1200, occlusionMaskDarkness 0.5 |

`heightFogSettings`의 density 0.01은 `efEnvironmentInfoData`의 0.1과 **10배 다르다**.
**어느 쪽이 언제 적용되는지 확정하지 못했다.** 후보에는 `efEnvironmentInfoData`를 썼다.
카메라 기반 활성화 조건을 확인하기 전에는 이 선택을 "원작과 동일"로 기록하지 않는다.

## 6. 상태 구분

| 항목 | 상태 |
|---|---|
| 원본 추출 | 완료 — export 25,119 열거, 환경 struct 5개 잔여 0 디코드 |
| 후보 생성 | 완료 — 후보 A/B + 순수 추가 diff + 근거표 |
| live 설치 | **없음** |
| 게시 | **없음** (`-Mode Publish` 미실행) |
| 빌드 | **없음** (VS pid 30616 실행 중이라 금지 준수) |
| 제품 소비자 연결 | **없음** (`LevelRegistry.cpp` 불변) |
| 사용자 화면 확인 | **없음** (화면에 나타나는 변경을 하지 않았다) |

## 7. 실행한 명령과 결과

| 명령 | exit | 결과 |
|---|---|---|
| `enumerate_exports.py --output .../ExportInventory` | 0 | 3 package, export 25,119, 환경 후보 dump 261 |
| `decode_environment.py --output .../Environment` | 0 | PS 7 export, SL01 sky/volume 9, emitter측 210 |
| `build_scene_candidate.py --output .../G05` | 0 | 후보 A +46/−0, B +68/−0 |
| `Publish-RenderingProfiles.ps1 -Mode Validate -SourcePath 후보A` | 0 | PASS |
| `Publish-RenderingProfiles.ps1 -Mode Validate -SourcePath 후보B` | 0 | PASS |
| `Publish-RenderingProfiles.ps1 -Mode Validate` (정본) | 0 | PASS |
| `extract_ue3_map_lights.py --help` | 0 | `--package --area-id --output --aes-key`만 지원 |

`extract_ue3_map_lights.py`는 **광원 component만** 처리한다(`"light" not in class_name`이면
skip). fog·sky·post-process·WorldInfo를 추출하지 않으며, 출력 document 전체에 blanket
`provenance: "PROJECT_AUTHORED"`를 붙인다(필드별 표시가 아니다). 이 blanket 라벨을 원본
정확도 PASS로 승격하지 않았다.

## 8. 사용자 승인이 필요한 diff 요약

**결정할 것: 후보 A만 적용할지, 후보 B까지 적용할지, 아니면 미해결 8항목을 먼저 풀지.**

- 두 후보 모두 **기존 줄 삭제 0**이다. `RenderingProfiles.json`에 profile 하나를 추가할 뿐
  기존 27 profile·`globalQuality`·`qualityOverride` 20키·revision 85를 건드리지 않는다.
- 후보 A는 `qualityOverride`가 아예 없어 팀장 품질 설정과 접점이 0이다. 가장 보수적이다.
- 후보 B는 마하라카 전용 `qualityOverride`를 **새로** 만든다. 기존 값을 바꾸지는 않지만
  마하라카 Level의 SSAO/bloom/FXAA/gamma 기준을 이 새 블록이 고정하게 된다.
- 어느 쪽이든 실제 화면 적용에는 `LevelRegistry.cpp:193`의 MAHARAKA scene profile ID 교체와
  `-Mode Publish`가 추가로 필요하다. **둘 다 하지 않았다.**
- 미해결 8항목(특히 색 밝기 배율 규약과 두 번째 카메라 환경 세트) 때문에 후보의 밝기·색이
  원작과 같다고 보장할 수 없다. 화면 판정은 사용자 몫이다.

## 9. 미해결과 다음 조사 위치

| 항목 | 다음 조사 위치 |
|---|---|
| 색 밝기 배율 규약 | 엔진 fog/light 소비 경로의 실제 곱셈 순서, 또는 Bern 원본 `efEnvironmentInfoData`와 Bern profile `fog.color` 6.0의 대조 |
| `scene_tonemapperscale` / `toefactor` 실값 | `PostProcessSettings` CDO 또는 `efpostprocess.postprocesschain.defaultscenepostprocess` |
| `dominantdirectionallightcomponent` (PS 45) | 836,217 B export의 preamble 구조. 앞 4,096 바이트 밖에서 property stream 탐색 범위 확대 |
| 두 환경 세트 중 런타임 적용 조건 | `efselfcameraenvironmentinfo`의 `activatedelay` 배열과 `efenvironmentinfovolume`의 brush 경계(위치 (7488, 98432, 8704) cm) |
| SkyLight 상단 반구 | `SkyLightComponent` class default |
| emitter 1개 template 조인 실패 | `ambient-emitters.json`의 `template: null` 행 |
| RNM 대 dynamic maplights 선택 | G04의 RNM 설치 계획과 같은 결정 단위로 묶어야 한다 |
| 물 재질 (G05-03) | **이번에 착수하지 않았다.** SL01 `eftranslucentvolume` 6개(섬 좌표)가 translucent sort 입력 후보다 |

## 10. 다음 G를 시작해도 되는 근거

G05 전반부의 산출물은 후보와 근거표뿐이고 제품에 아무 영향이 없다. 따라서 G06·G07·G08은
이 결과와 무관하게 진행할 수 있다. 다만 두 항목은 순서 의존이 있다.

- **G04의 RNM 설치는 3절의 이중 가산 결정 없이 진행하면 안 된다.** 34개 광원이 이미 lightmap에
  베이크돼 있다.
- **G05-03 물 재질과 G05-02 상시 particle 코드 연결은 이 문서의 목록을 입력으로** 시작할 수 있다.

`_PS` package 조사는 G02와 중복되지 않았다. G02 RESULT에서 `_PS` 언급은 admission 제외
집계 한 줄뿐이고 환경 액터·fog·skylight·post-process는 조사하지 않았다(grep 확인).
