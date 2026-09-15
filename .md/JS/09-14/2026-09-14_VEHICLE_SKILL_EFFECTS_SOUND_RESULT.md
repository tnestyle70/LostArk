# 2026-09-14 탈것 스킬 파티클·사운드 RESULT

작성자: JS · 브랜치 `feature/vehicle-skill-effects-sound`(main `500671fc`에서 분기) · 작업일 2026-09-15
계획: [PLAN](2026-09-14_VEHICLE_SKILL_EFFECTS_SOUND_PLAN.md)

## 상태

| G | 상태 |
|---|---|
| G01 원본 closure | 완료 |
| G02 socket 계약·cue 해석 | 완료 (원본 공백 오타 부착 이름 1건은 G04 결정) |
| G03 native 재질 | 완료. 113 program 설치, Debug Product 빌드 PASS. 4재질 13 emitter 보류 |
| G04 프로젝션·설치 | 완료. 문서 24개·요소 368개 설치, 탈것 문서 검사·리소스 closure PASS. 저장소 전체 validator는 기존 쿠크/차원술사 문서 때문에 원래 실패 |
| G05 카탈로그 v3 | 완료. effectCues 24·soundCues 30 기록, Client 파서 v2/v3 수용, `ActorCatalog.cpp` 단일 컴파일 PASS. 전체 빌드는 G06과 함께 |
| G06 런타임 연결 | 구현 완료, Debug Product 빌드 PASS. 사용자 화면 확인 대기 |
| G07~G08 | 미착수 |

## G01. 원본 closure

### 구현

`Tools/VehiclePipeline/build_vehicle_skill_effects.py --acquire-only`. 쿠크 `build_kouku_gate1_full_restore.acquire()`를 수정 없이 import하고
모듈 입력만 주입한다.

| 입력 | 쿠크 원래 값 | 탈것 주입 값 |
|---|---|---|
| `ACTION` / `SELECTED` | CanonicalSource action JSON | `out/VehicleSkillEffects20260914/actions/*.action-effects.json`에서 만든 `selected_source_actions.json`. 합성 actionId = `skillId*10+stageIndex`, Particle notify만 |
| `GRAPH` | CanonicalSource particle-graph | 없는 경로 → `acquire()`의 external resolver 분기 |
| `source_package('Shared', engine/efgame)` | CanonicalSource 패키지 | 게임 `ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u`(ENGINE), `NU1V7NCQ4YAE9ZPJVNOQS.u`(EFGAME). 이름은 `deobfuscate_names.decode`로 확인 |
| external resolver | `C:/LostArkExtract/Tooling/...umodel` | `C:/Users/95jus/Downloads/umodel_win32/umodel_lostark_v7.exe -nameresolve` |
| `decode_typed_payload` | 실패 시 중단 | 실패를 `source_cue_decode_errors.json`에 기록하고 계속(cue 해석은 G02/G04 몫) |

### 계획과 달라진 사실: 스킨 선택

계획의 "기본 스킨 첫 참조" 전제가 틀렸다. `EFTable_Vehicle.Model`이 6종 중 5종에서 변형 스킨을 가리킨다.

| 탈것 | `Model` | notify 선택 |
|---|---|---|
| 황금 테르페이온 6705 | `EFDLVehi_MN_PMSTG_00-3` | 스킨 system 14, 기본 5 |
| 고요한 별빛의 가호 9370 | `EFDLVehi_MN_PMSSM_00` | 기본 8 |
| 레인보우 모코보드 7209 | `EFDLVehi_MN_PMSMK_00-4` | 스킨 system 1 (`Par_G_PMSML_00-03_Run`) |
| 아우프슈텐-R 8302 | `EFDLVehi_MN_PMSHE_00-2` | 기본 40 (`-2` modifier 없음) |
| 바다 유니콘 튜브 8906 | `EFDLVehi_MN_PMSUT_00-6` | 기본 17 |
| 고대의 신화 9524 | `EFDLVehi_MN_PMSDZ_00-1` | 스킨 system 17, 기본 7 |

선택 규칙: labels를 `CEFParticleDataModifier` 단위로 나눠 `Model`과 같은 이름의 구간에 system이 있으면 그것, 없으면 기본 system.
행별 결과는 `source_skin_selection.json`.

### 결과 (`out/VehicleSkillEffects20260914/`)

| 항목 | 값 |
|---|---:|
| Particle notify | 109 |
| 고유 ParticleSystem | 61 |
| first-LOD emitter | 367 |
| 해소 안 된 module 참조 (`first_lod_module_closure_validation.json`) | **0** |
| LOD 0개 emitter | 0 |
| 게임 UPK에서 추가로 연 외부 패키지 | 17 (`external_module_closure.json`) |
| 스크립트 패키지 파싱 오류 | 0 |
| 원본 재질 | 116 |
| renderer shape | sprite 310, mesh 39, light 10, ribbon 4, decal 2, screenPost 2 |
| 쿠크 라이브러리에 이미 있는 system | 8 (계획 목록과 동일) |
| AnimationTrail | 0 (baked edge history 불필요) |

소스 패키지: `fx_cm_00/01/02`, `fx_post`, `fx_vt_00`, `fx_vt_pmsdz_00`, `fx_vt_pmshe_00`, `fx_vt_pmsmk_00`, `fx_vt_pmssm_00`, `fx_vt_pmsut_00`.
스킬별 emitter 수와 notify 수는 `source_occurrences.json`/`source_notifies.json`의 합성 actionId로 집계한다. 98383(별빛 E)은
PlayCameraParticleEffect만 있어 대상이 없다.

G01 첫 실행에서는 cue payload 19건(신화 17, 테르페이온 `Gold` 2)을 해석하지 못했다. G02에서 해결했다.

## G02. socket 계약과 cue 해석

같은 생성기의 `--acquire-only`가 G01 전에 socket 계약을 만들고, 쿠크 `acquire()`의 `decode_typed_payload`를 탈것용 해석으로 감싼다.
공용 디코더(`build_action_cue_recipe.py`)는 수정하지 않았다.

### socket 계약 (`sockets/<Vehicle>.socket-contract.json`)

게임 UPK의 몸체 SkeletalMesh `Sockets` 배열 → `SkeletalMeshSocket` export를 직접 읽는다(UModel props 텍스트 불필요).
단위는 쿠크 `extract_ue3_skeletal_mesh_sockets`와 같다(위치 ×0.01, Rotator ×360/65536).

| 탈것 | 원본 mesh | socket | 설치 wmodel 본 | socket 본 누락 |
|---|---|---:|---:|---:|
| Terpeion | `MN_PMSTG_01.mesh.mn_pmstg_01_sk` | 27 | 150 | 0 |
| SereneStarlightBlessing | `MN_PMSSM_00.mesh.mn_pmssm_00_sk` | 4 | 22 | 0 |
| RainbowMokoboard | `MN_PMSMK_00.mesh.mn_pmsmk_00_sk` | 2 | 12 | 0 |
| Aufstehen | `MN_PMSHE_00.mesh.mn_pmshe_00_sk` | 12 | 63 | 0 |
| SeaUnicornTube | `MN_PMSUT_00.mesh.mn_pmsut_00_sk` | 2 | 12 | 0 |
| AncientMyth | `MN_PMSDZ_00.mesh.mn_pmsdz_00_sk` | 16 | 91 | 0 |

### 원본 payload에서 새로 확인한 구조

| 구조 | 내용 | 처리 |
|---|---|---|
| 빈 기본 system | 기본 `CEFParticleData`가 아무것도 재생하지 않으면 클래스 이름 뒤 +12의 ParticleSystem FString 길이가 0 | 선택 system 참조를 그 자리에 넣은 사본을 공용 디코더에 넘긴다 |
| 두 부착 배열 | system 참조 끝 +52에 **본 이름 배열**, 바로 뒤에 **socket 이름 배열**. 공용 디코더는 하나만 읽고 빈 두 번째 배열의 4바이트를 layout 차이로 처리해 왔다 | 두 번째 배열이 채워진 2건(신화 98520 `FX_State_01`, 98523 손·발 socket 4)은 사본에서 비우고 socket으로 따로 해소해 `runtimeAnchors` 뒤에 붙인다 |
| `CEFParticleDataModifier` | 클래스, 스킨 이름, system FString, int32 flag, int32 parameter 수, parameter 레코드(기본 테이블과 같은 형식), int32 | 선택 스킨 modifier에 parameter가 있으면 그 테이블이 `parameterOverrides`, 기본 테이블은 `baseParameterOverrides`로 보존. flag는 parameter 유무와 무관(0/1 모두 관찰)해 기록만 한다 |

### 결과

| 항목 | 값 |
|---|---:|
| cue 해석 실패 | **0** / 109 |
| 부착 anchor | 102 |
| socket 해소 (`EXACT_SOURCE_SOCKET`) | 49 |
| 본 해소 (`EXACT_SOURCE_BONE`, 설치 wmodel 본 이름으로 확정) | 52 |
| 해소 실패 | 1 |
| anchor 없는 root snapshot notify | 31 |
| 스킨 modifier parameter 사용 notify | 21 |
| transform scale | 1.0 86건, 0.7 11, 0.3 6, 1.5 4, 기타 2 |

해소 실패 1건: 아우프슈텐 97330 `Floor-react`의 부착 이름이 원본에 `b_effectroot `(끝 공백)로 들어 있다. 설치 wmodel에는 `b_effectroot`가 있다.
원작 엔진이 공백을 포함한 이름으로 본을 찾지 못해 다른 위치에 붙였을 가능성이 있어 여기서 임의로 보정하지 않았다. G04 프로젝션에서
공백 제거 해소로 둘지 결정하고 그 판단을 기록한다.

행별 해소는 `source_anchor_resolution.json`, 두 번째 배열 사용 notify는 `source_summary.json`의 `secondAnchorLists`.

## G03. native 재질 program

### 사용자 결정 (2026-09-15)

| 항목 | 결정 |
|---|---|
| program 번호 | 쿠크 범위 안의 빈 칸이 아니라 **3712 이후 탈것 전용 구간**. native 상한을 3711 → 3967로 넓혀 3712..3967(64×4)을 탈것에 쓴다 |
| PIL | Blender 5.0 번들 Python에 `pip install pillow`(12.3.0) |

### 구현

`build_vehicle_skill_effects.py`에 두 단계를 추가했다.

| 명령 | 내용 |
|---|---|
| `--native-first 3712 --native-last 3967` | 쿠크 `build_kouku_pattern_native.prepare()`를 import해 원본 MaterialMap/DXBC → native HLSL 생성. 프로세스 안에서만 UModel 경로, `startup` 패키지 해석, 텍스처 설치 위치(`Effect/Vehicle/FullRestore/Textures`), D3D 역어셈블러 DLL을 바꾼다. 결과는 `out/VehicleSkillEffects20260914/material/` |
| `--install-native` | `install_kouku_gate1_native_shaders.install()`과 같은 base/distortion case를 만들되, 설치된 쿠크 블록을 그대로 입력으로 넘기고 탈것 program만 이어 붙인다. 표는 `install_kouku_gate1_native_materials.install()`로 설치 |

재질 입력에서 원본 material이 null인 sprite emitter 1개(모코보드 `seed_003`, EventGenerator 씨앗)는 제외하고 `native_input_exclusions.json`에 남겼다.

### 원본과 다른 환경

공유 역어셈블러(`extract_artist_31470_main_ref_shader_cache.D3DDisassembler`)는 SDK 10.0.22621의 `d3dcompiler_47.dll`(SHA `ce013eb1…`)만 허용한다.
이 PC에는 그 버전이 없어 10.0.26100 x64 복사본(SHA `7d4271d6…`)으로 이 프로세스의 기대값을 바꿨다. 공유 코드는 수정하지 않았다.
역어셈블 텍스트 형식 차이가 생성 HLSL에 영향을 줬는지는 기준 DLL이 없어 대조하지 못했다.

### 범위 확장으로 바꾼 공유 파일

`2304..3711` 고정값을 `2304..3967`로 바꿨다(바이트 단위 치환, 인코딩 보존). runtime profile ID는 생성기 domain `kouku`를 그대로 써
`effect.ue3.kouku-37xx-native.v1`이 된다. `Effect_ArtistMaterial.h`, `Effect_Playback.cpp` 등의 `effect.ue3.kouku-` 접두사 분기를 건드리지 않기 위해서다.

| 파일 | 변경 |
|---|---|
| `Client/Private/Effect_NativeScreenPostMaterial.cpp` | 상한 2곳 |
| `Client/Private/Effect_DocumentRenderer_MaterialBinding.cpp` | 상한 2곳 |
| `Client/Private/Effect_DocumentRenderer_Geometry.cpp`, `Effect_DocumentRenderer_Particles.cpp` | 상한 각 1곳 |
| `Shader_EffectMeshFamilyCarrier.hlsli`, `Shader_EffectParticleFamilyCarrier.hlsli` | 상한 각 2곳 |
| `Shader_VtxEffectDecal.hlsl`, `Shader_VtxEffectTrail.hlsl`, `Shader_VtxEffectNativeScreenPost.hlsl` | 상한 각 1곳 |
| `Tools/EffectPipeline/install_kouku_gate1_native_shaders.py` | 범위 assert 3곳, carrier 재작성 상한 1곳 |
| `Tools/EffectPipeline/install_kouku_gate1_native_materials.py` | `FIRST,LAST` |

설치기가 쓴 파일: `Effect_ArtistMaterial_Tables.inl`(+113 program), `Effect_ShaderFamily.h`(90 → 94행), `Shader_EffectArtistNative.hlsli`(include 4줄),
신규 `Shader_EffectKoukuNativeGroup3712/3776.hlsli`, `Shader_EffectArtistNativeDispatchKoukuNativeCases3712/3776.hlsli`,
`Shader_VtxEffect{Mesh,Particle}Kouku3712/3776.hlsl`, `Client.vcxproj(.filters)` 등록. 기존 쿠크 group/case 파일은 바이트 변화가 없다.

### 결과

| 항목 | 값 |
|---|---:|
| 입력 emitter (light 10, 제외 1 뺀 VF 판정) | 355 |
| 선택된 원본 material×VF program | 116 |
| 생성·설치 program | **113** (3712..3827) |
| 원본 distortion 누적 패스 | 49 |
| 필요한 원본 텍스처 | 187 (모두 기존 Resources 재사용, 그중 14개는 이번에 `Effect/Vehicle/FullRestore/Textures`로 복사) |
| 설치 후 native 전체 | 985 program, 18 group, 36 carrier |
| 설치 재실행 | 표 `changed=False`, shader 파일 변화 없음 |
| Debug Product 빌드 | Engine/Shared/Server/Client PASS. Client 943초, OBJ 64·CSO 104·binary 1 (`out/BuildPipeline/runs/20260915T045140937Z-debug-product.json`) |

### 보류 4재질 (13 emitter)

| program | 원본 재질 | emitter | 원인 |
|---|---|---:|---|
| 3765 | `bg_att_eichmannl_c.mat.ehm_pot_01_mi` (아우프슈텐 Q/W/E mesh) | 6 | 원본 VS가 TEXCOORD7을 쓰며 mesh carrier adapter가 없다 |
| 3776 | `bfx_m_mi_00.bfx_mi.bfx_c_pa_volcloud_01_1_tr` (아우프슈텐 Q/W/E sprite) | 5 | TEXCOORD7 sprite adapter 없음 |
| 3810 | `fx_post.fx_mi.fx_c_pa_zoomblur_01_tr` (신화 Q screenPost) | 1 | 원본 PS `3fc4c0de…`의 binding 배열 후보가 0개 |
| - | `itr_01118.mat.itr_01118_mi` (모코보드 Space) | 1 | 텍스처 expression index 3이 parent 참조 배열 밖이고 이름 있는 TextureParameter 후보도 없다 |

TEXCOORD7 adapter는 공유 생성기의 shader ID별 분기라 이번에 추가하지 않았다. G04는 이 13 emitter를 문서에서 빼고 목록을 남긴다.
전체 목록은 `material/reviewed/native_deferred_programs.json`.

## G04. V1 문서 프로젝션과 설치

`build_vehicle_skill_effects.py --project [--install]`. G01 acquire를 다시 돌려 source index를 만든 뒤 쿠크 `project()`와
`project_light_occurrences()`, `bind_source_providers()`, `patch_simulation_providers()`를 재사용한다.

### 결정

| 항목 | 결정 |
|---|---|
| 스테이지 → 클립 | 탈것 COMBO 체인은 클립을 첫 등장 순서로 한 번씩 재생한다(탈것 스킬 PLAN 계약). 그래서 각 클립은 **첫 source stage**만 쓴다. 제외 4 stage: 별빛 98382 stage 3·4·5(swing_02/03 반복·화려한 분기, notify 5), 신화 98522 stage 2(breath_02 두 번째 분기, notify 2) |
| 문서 단위·ID | 클립 하나 = 문서 하나. `effect.vehicle.<archetypeId 소문자>.<skillId>.<clipIndex>.full.restore` |
| native 재질 적용 | 공유 `project()`에 재질 patch를 넘기면 메쉬 경로를 쿠크 폴더로 가정해 검사한다. patch 없이 투영한 뒤 같은 규칙(재질 복사, UV 초기화, emissive 1)을 탈것 쪽에서 적용한다 |
| root snapshot basis | 쿠크·도화가·차원술사와 같은 `snapshotRootSourceBasisYawDegrees=-90` 유지 |
| `b_effectroot ` (G02 보류) | 공백을 떼고 설치 본 `b_effectroot`로 해소, anchor에 `sourceNameNormalization=TRAILING_WHITESPACE_STRIPPED` 기록. 원작이 이 이름으로 본을 못 찾았을 가능성은 사용자 화면 확인에서 판단 |
| 반복 광원 | 신화 98522 breath_02의 `Par_MP_Light_02_sine`는 emitter loop가 1이 아니어서 공유 광원 스케줄러가 거부한다. 해당 광원 1개만 빼고 문서는 설치 |

### 메쉬 (`geometry/installation.json`)

쿠크 full restore와 같은 규약(UModel glTF → converter `--scale 100` → `cook_wmodel_geometry_contract`, preScale 0.01)을 쓴다.
사용 메쉬 17개 중 8개는 `Effect/KoukuSaydon/FullRestore/Meshes`를 경로 그대로 재사용, 9개는 게임 패키지에서 새로 쿠킹해
`Effect/Vehicle/FullRestore/Meshes`에 넣었다. 다른 직업 폴더의 같은 이름 메쉬는 스케일 규약이 달 수 있어 재사용하지 않았다.

### 결과

| 탈것 | 문서 (clipIndex) | 요소 |
|---|---|---:|
| 황금 테르페이온 | 96030, 96000, 96010, 96020 | 27 / 24 / 8 / 20 |
| 고요한 별빛의 가호 | 98380, 98381, 98382.1 | 7 / 11 / 1 |
| 레인보우 모코보드 | 95722 | 9 |
| 아우프슈텐-R | 97300, 97310, 97320, 97330 | 17 / 26 / 37 / 47 |
| 바다 유니콘 튜브 | 97730, 97731, 97732.0·1·2, 97733.0·1 | 3 / 6 / 10·10·9 / 5·15 |
| 고대의 신화 | 98520, 98521, 98522.0·1, 98523 | 7 / 24 / 6·13 / 26 |

- 문서 24개, 요소 368개(particle 348, light 11, trail 6, decal 2, screenPost 1). follow 부착 269, root snapshot 99.
- 별빛 98383(E)은 PlayCameraParticleEffect만 있어 문서가 없고, 98382는 swing_02 한 클립만 파티클이 있다.
- notify 분모: 원본 109 → 제외 stage 7 → 대상 102 → 문서 요소가 참조하는 notify 100. 나머지 2는 G03 보류 zoomblur(신화 Q)와 위 반복 광원.
- 문서 안에서 빠진 emitter 14개: G03 보류 13 + null 재질 씨앗 1. 문서별 목록은 `projection/installation.json`의 `deferredEmitters`.

### 설치

- `Data/Effects/Authored/effect.vehicle.*.effect.json` 24개(v13 21, v15 3), `Data/Effects/EffectCatalog.json` 행 24개 추가.
- `Client.vcxproj(.filters)`에 `96.DataFiles` None 항목 24개 추가.

### 검증

| 검사 | 결과 |
|---|---|
| 탈것 문서 24개에 validator 문서 단위 검사(재질 color space, native sprite 옵션, module override, 부착 orientation, v15 extension) | PASS |
| 참조 리소스 200개 closure(`_validate_runtime_resource_closure`) | PASS (200 파일, 16.5MB, 모두 Git 비추적 로컬) |
| `Validate-EffectSources.ps1` 전체 | **FAIL, 기존 원인**. 기존 쿠크 v15 문서 10개(`effect.kouku.gate1.blade-dance.*.impact` 등 runtime carrier 없음)와 차원술사 audition hash 불일치에서 먼저 멈춘다. 탈것 문서와 무관 |

G05 주의: validator의 `catalog Effect IDs have no runtime consumer` 규칙은 VehicleCatalog를 소비자로 모른다. 전체 validator가 그 단계까지
가게 되면 탈것 24개가 걸리므로, G05에서 `VehicleCatalog.json`의 `effectCues`를 reachability 입력에 더한다.

## G05. VehicleCatalog formatVersion 3

### 데이터

`build_vehicle_skill_effects.py --catalog`가 각 skill에 두 배열을 쓴다.

| 필드 | 출처 | 값 |
|---|---|---|
| `effectCues[]` `{clipIndex, effectAssetId, startMs, stopPolicy}` | G04 `projection/installation.json`의 설치 문서 | 24개, 모두 `startMs 0`, `NATURAL` (notify 시각은 문서 요소가 가진다) |
| `soundCues[]` `{clipIndex, event, startMs}` | 원본 Action의 `AKEvent` notify, G04와 같은 첫 stage 선택 | 30개. `event`는 AkEvent 경로에서 패키지를 뗀 이름(`S_Vehicle_TrisionHorse_Dash1`) |

- 원본 AKEvent 32 중 제외 stage의 2건(별빛 `Skill5`, 신화 `Skill2_2` 두 번째 분기)은 넣지 않았다. `AkEventSwitchFloorMaterial` 발소리 2건은 범위 밖.
- 별빛 98383(E)은 effectCue 없이 soundCue 1개만 있다.
- 기존 `build_vehicle_skills.py`가 다시 돌아도 cue 배열과 formatVersion 3을 지우지 않도록, 같은 skillId의 기존 cue를 옮겨 쓰고 `write_catalog`에 두 배열 출력을 더했다. 현재 파일을 읽어 다시 쓰면 바이트가 같다.

### Client 파서 (`Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp`)

- `VEHICLE_SKILL_EFFECT_CUE {clipIndex, effectAssetId, startMs, bStopAtCueEnd}`, `VEHICLE_SKILL_SOUND_CUE {clipIndex, event, startMs}`를 `VEHICLE_SKILL_ENTRY`에 추가.
- `ParseVehicles`는 formatVersion 2(skill 필드 4개)와 3(필드 6개, 두 배열 필수)을 모두 받는다.
- `ParseVehicleSkillCues`: 배열 형식이 틀리면 카탈로그 전체를 거부하고, cue 하나가 틀리면(필드 수, clipIndex ≥ 클립 수, Effect ID 형식, stopPolicy) 그 cue만 버린다.
  Effect ID 검사는 validator와 같은 `[a-z0-9][a-z0-9._-]{0,127}`이다(기존 `IsStableId`는 64자 제한이라 69자 ID가 걸린다). 없는 effectAssetId는 G06의 prepared catalog 조회에서 거부된다.

### validator

`validate_effect_sources.py`의 runtime 소비자 목록에 `Data/Actors/VehicleCatalog.json`의 `effectCues[].effectAssetId`를 더했다.

### 검증

| 검사 | 결과 |
|---|---|
| VehicleCatalog JSON parse, formatVersion 3, skill별 cue 수 | PASS |
| `write_catalog` 재기록 바이트 동일 | PASS |
| `Client.vcxproj` `ClCompile SelectedFiles=ActorCatalog.cpp` (Debug x64) | PASS (기존 C4819 경고만) |
| v2 입력 호환·잘못된 cue 단일 거부의 실행 확인 | 미실행. 하네스를 추가하지 않았고 G06 실행에서 확인한다 |

## G06. 런타임 연결

### 탈것 본 크기 실측

탈것 wmodel 스켈레톤의 bind 결합 행렬을 admission preScale 0.0001과 곱해 basis 길이를 쟀다.
AncientMyth `b_root`/`b_r_wing_02`/`bip001-spine`/`b_effectroot`/`b_cockpit`, Terpeion `b_root`/`bip001-l-finger0`/`b_effectroot`,
SeaUnicornTube `b_effectroot` 모두 **0.01**이다. 워로드 Q·창술사 34610/34630/34650이 이미 쓰는 계약(admission 0.0001 × rig root 100 = 0.01,
translation은 미터)과 같아서 같은 정규화 경로를 쓴다. 정규화하지 않으면 bone 부착 요소가 1/100 크기로 붙는다.

### 변경

| 파일 | 내용 |
|---|---|
| `Client/Public/Effect_PresentationService.h` | `EFFECT_SPAWN_DESC::bVehicleModelAnchors` |
| `Client/Private/Effect_PresentationService.cpp` | `EFFECT_OWNER_VIEW::bVehicleModel`: 켜지면 `Get_Model()`은 탈것 파츠 모델, `Try_Get_PresentationRoot()`는 탈것 파츠 결합 월드. `ACTIVE_EFFECT`에 같은 플래그를 보존해 follow 샘플도 탈것을 쓴다. 탈것이 없으면 spawn 거부(body 대체 없음), world-root 조합 거부. `Requires_SourceBoneImportScaleNormalization`에 `effect.vehicle.` 접두사 |
| `Client/Public/Part_Vehicle.h`, `Client/Private/Part_Vehicle.cpp` | `Get_Model`, `Get_CombinedWorldMatrix`, `Try_Get_SkillClipWindow`(Seek_SkillChain과 같은 clip 길이로 체인 내 시작·길이) |
| `Client/Public/Character.h`, `Client/Private/Character.cpp` | `Get_VehicleModel`, `Try_Get_VehicleWorldMatrix`. VEHICLE_SKILL 분기에서 `Update_VehicleSkillCues`: 이전 action age < cue 시각 ≤ 현재 age인 cue를 한 번 spawn(`fInitialSampleTimeSeconds`=지연, 준비된 문서 길이를 넘긴 늦은 cue는 생략). 사운드는 `CSoundCueCatalog::Find_Variants("Vehicle", event)`로 찾고 0.25초 넘게 지난 cue는 재생하지 않는다. 새 action start tick과 탑승 교체 때 커서 초기화. 탈것 파츠 부착 직후 그 탈것의 effectAssetId 전체를 `Queue_ProductTargets_Priority`에 넣는다 |

- VEHICLE_SKILL 분기는 snapshot마다(30Hz) 호출되므로 cue 판정 간격은 최대 1 tick이고, 늦은 만큼은 초기 샘플 시간으로 맞춘다.
- 첫 탑승 직후에는 준비 큐가 한 프레임에 문서 하나씩 준비하므로, 준비 전에 쓴 스킬의 이펙트는 기존 계약대로 거부된다.
- 사운드는 G07에서 `Vehicle` 클래스 카탈로그가 생기기 전까지 소리가 없다(빈 variant는 조용히 건너뜀).

### 검증

| 검사 | 결과 |
|---|---|
| Debug Product 빌드 (G05 파서 포함) | Engine/Shared/Server/Client PASS. Client 99초, OBJ 93·binary 1 (`out/BuildPipeline/runs/20260915T060321320Z-debug-product.json`) |
| 사용자 화면 확인 (탈것 6종 Space/Q/W/E 파티클 위치·크기·시각) | 고대의 신화 진행 중 (아래) |

### 사용자 피드백 반영 (2026-09-15, 고대의 신화)

| 피드백 | 원인 | 조치 | 사용자 확인 |
|---|---|---|---|
| E·Space는 원작과 큰 차이 없음 | - | - | 확인 |
| W 불뿜기가 완전히 없음 | Client 디버그 출력(DBWIN 리스너)으로 `98522.1` 준비 실패 확인: 불 emitter 7개의 `particlemodulevelocitycone`을 portable runtime이 허용하지 않아 문서 전체 거부 | 공용 런타임에 VelocityCone 구현. `Effect_DocumentCodec_PortableRuntime.cpp` 모듈·distribution(`angle`,`velocity`) 허용, `Effect_Playback.cpp` 생성 속도: UE3처럼 `Direction` 축에서 `Angle`도 떨어진 lathe 무작위 방향 × `Velocity`, 좌표·local/world 처리는 Velocity 모듈과 같다. 실패 사유를 남기도록 `Character.cpp` 로그에 준비 실패 receipt 추가 | 불 나옴 |
| 불을 짧게 뿜음 (원작은 머리 방향으로 계속) | 공유 importer가 직렬화되지 않은 `EmitterLoops`를 1로 채움. UE3 기본값은 0(무한) | 탈것 프로젝션에서 원본에 `emitterloops`가 없으면 0(World marker builder와 같은 규칙). notify 창(3.2초) 동안 방출 | 잘 쏨 |
| 불 쏠 때 디스토션이 너무 셈 | 위 수정으로 디스토션 emitter(`fx_d_pa_master_01_011_ds_tr`, `fx_d_pa_glow_02_50_dt200_ad`)도 3.2초 누적 | 사용자 결정: 원본 distortion pass가 있는 program의 요소는 반복 기본값을 적용하지 않고 1회 방출. 전체 탈것 문서에서 무한 반복 요소 73 → 60 | 괜찮음 |
| Q에 zoomblur가 들어가야 함 | G03 보류 program(3810) | 같은 원본 재질·screenPost의 설치된 쿠크 program `kouku-3328`을 재사용(`projection/reused_installed_native_materials.json`). 신화 Q 요소 24 → 25 | 조금 보임 |
| (아우프슈텐) 돌진·스킬 부스터가 뒤가 아니라 앞에 붙음 | 탈것 WModel은 Blender psk importer를 거쳐 본 로컬 Y가 원본과 반대. `FX_R/L_Engine_01`(bip002-spine +67cm Y)가 raw로는 몸통 앞(+x 6560), Y 반전 시 `b_engine_00/01`과 같은 뒤(−x 6832) | 탈것 socket 계약의 runtime 위치 Y 부호 반전, 회전 yaw·roll 부호 반전(`EXPLICIT_SOCKET_PROPERTIES_BONE_Y_MIRRORED`). 탈것 6종 전체 적용 | 잘 나옴 |
| (아우프슈텐) 부스터 지속시간이 너무 김 | 원본 emitter `EmitterDuration 10 × Loops 1`, notify 창 0.73 s(대시)/0.5 s(E). 런타임은 duration×loops로만 방출을 끊음 | notify 길이가 있고 `delay + duration × loops`가 요소 수명(notify 창)보다 긴 요소는 loops 0으로 두어 창에서 방출 종료. EmitterTime은 첫 loop 안이라 그대로. 106개 요소 적용 | 잘 나옴 |
| (테르페이온) Q·W·Space 날개가 안 나옴 | 원본 `PlaySkeletalMesh` notify(날개 메쉬 `MN_PMSTG_00_Parts1_SK` + 애니 + 날개 socket 파티클)를 프로젝션이 다루지 않음 | 날개 WModel `Effect/Vehicle/Terpeion/TerpeionWing/TerpeionWing.wmodel`(npc_sk_* 32클립) cook. 스킨 -3 재질 `mn_pmstg_00-2_aa_mi`를 native 3828(skeletal model cue, `Shader_EffectVehicleModelNative.hlsli`)로 설치: 이 PS의 engine CB0는 [0].x opacity, [23..25] sky/ambient라 생성기에 전용 adapter 추가. 문서 96000/96010/96030에 modelCue `terpeion.wing`(clip victorypose/relaxation/dash, notify 시각·길이, 탈것과 같은 pre-transform 0.0001·yaw −90). 날개 notify의 `CEFAN_Particle` 항목을 합성 Particle notify로 투영하고 `modelCueId`로 날개 본에 부착, 날개 본 basis 0.01을 socket 위치·scale ×100으로 상쇄. 날개 트레일 ribbon program 3829/3830 추가. 전체 요소 368 → 391. 날개 사라짐 fade(`PlaySkeletalMeshMaterialParam`)는 미적용 | 날개 나옴 |
| (테르페이온) 날개가 페이드 없이 사라짐 | 원작은 `AnimEvent_MaterialParamterScalar` `dead`(1 투명)를 PlaySkeletalMesh 기준(나타남)·PlaySkeletalMeshMaterialParam 기준(사라짐) 구간으로 보간. payload: flag, start, duration, name, from, to | V1 ModelCue에 선택 필드 `materialParameterTracks`(SCALAR, cue-local 초) 추가. 모델큐 전용 바인딩·샘플 함수만 `Effect_ArtistMaterial.h`에 추가하고 기존 요소 트랙 함수·codec은 그대로. 매 프레임 native 파라미터 배열에 적용. 키: Q (0,1)(1.0,0)(1.10,0)(1.90,1), W (0,1)(0.5,0)(6.33,0)(6.43,1), Space (1.6,0)(2.07,1). Q 이벤트의 미상 값 0.4는 해석 보류, 선형 보간 | 잘 작동 |
| (테르페이온) 날개가 원작보다 어두움 | 모델큐 native 경로가 base pass(sky-light) PS만 쓰고 sky 입력 0·장면 주변광만 받음. 탈것 몸체는 원본 light PS로 방향광을 받음 | `build_vehicle_source_material.py extract`로 날개 MIC의 방향광 PS `1a678e80…`(base는 3828과 같은 `5f33bef7…`) 추출, `--wing-light`가 같은 uniform expression 기준으로 3828 packing을 재사용해 `ArtistNative3828Light` 생성. `Shader_VtxAnimMeshBinary.hlsl` 3828 분기에서 program 88과 같이 forward 주변광 + 조명별 light PS 합산, 렌더러가 3828일 때 forward 조명 바인딩. 그림자 없는 policy(NoStaticShadowing) | 좋음 |

위 C++ 변경은 Debug Product 빌드 PASS 후 실행했다. 문서 재생성은 데이터만 바뀌어 Client 재시작으로 반영했다.

### 하지 않은 확인

- 계획 4절의 "쿠크 `4219801` closure를 같은 resolver로 다시 뽑아 기존 `source_occurrences.json`과 비교"는 이 PC에
  CanonicalSource와 기존 쿠크 evidence(`out/KoukuGate1FullRestore20260911`)가 모두 없어 실행하지 못했다.
- 빌드, 런타임, 화면 확인: G01 범위 밖.
