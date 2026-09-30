# 발탄 검격 회전 중심·불어날리기·원본 상공 이펙트 결과

## G00. 반영한 범위

기존 디스크 저장본의 피자 STEP_11 offset과 Atk_08_04 emitter13 삭제를 보존했다.
이 문서의 검증은 Client/UI를 실행하지 않은 데이터·CPU·컴파일 검사다. 최종 화면은 사용자 확인 영역이다.

### 회전 중심

`effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_04`의 남은7개 Element에 공통 local X+0.75를
적용했다. 원본 emission origin envelope의 X범위[-3,1.5] 중심-0.75를 local0으로 옮긴 것이다.
이는 움직이는 전체 렌더링 AABB를 매 프레임 따라가는 중심이 아니라 고정 검격 발사 중심이다.
원본 StartLocation·속도·TypeData·source recipe·재질·크기·timing은 그대로다.

같은 공유 자산의 세 occurrence를 모두 역보정했다.

| occurrence | 이전 local X | 현재 local X |
|---|---:|---:|
| VALTAN_SIX_PIZZA_106 / STEP_11 | -2.9200000762939453 | -3.6700000762939453 |
| VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK / COMBO_STEP_11 | 0 | -0.75 |
| VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK / COMBO_STEP_11 | 0 | -0.75 |

세 occurrence의 현재 rotation0·scale1을 반영한 보정이다. 피자의 사용자 local Y0.6600000262260437은
유지했다. 기존 표시 위치를 유지하면서 rotation은 새 고정 중심을 사용한다. native playback의
0.01/0.05/0.1/0.25/0.5/0.75/0.99/1.2초에서 이전·현재40개 particle world matrix를 비교해 최대차0을
확인했다. 사용자 저장본과 새 source 문서에서 X0.75만 되돌린 구조 비교도 완전히 같았다.

### 뒤 잡기 후 불어날리기

`VALTAN_CATCH_BREATH/STEP_04`의 `cue.valtan.catch-breath.source.step-04`만 yaw0→90으로 변경했다.
자산은 `effect.valtan.action.420623.stage004.full.restore`다. source asset과 다른 catch stage는 유지했다.

## G01. 원본 상공 블랙홀 복원

사용자가 기억한6개 Element라는 수를 맞추기 위해 원본을 합치거나 임의 삭제하지 않았다.
현재 원본 package에서 확인한 두 ParticleSystem은 다음과 같다.

| 원본 | Product source | 원본 Element | 실제 spawn |
|---|---|---:|---:|
| bfx_high_01.valhatron.par_d_spacehole_03 | effect.valtan.sky.source.spacehole | 2 (별도 empty3 제외) | 2 |
| bfx_high_00.chaosgate.par_d_hugechaosgate_01 | effect.valtan.sky.source.chaosgate | 12 | 11 |

chaosgate particlespriteemitter_0은 원본 particlemodulespawn_0의 Rate/RateScale distribution이 null이고
BurstList도 없어서 입자를 생성하지 않는다. 문서에는 원본 그대로 보존했다. 기존 unbound V2
`boss.valtan.blackhole_1` 단일-texture proxy를 이번 source 원본으로 사용하지 않았다.

원본은 설치된 LOSTARK ReleasePC package에서 기존 UModel 기반 common source extractor를 통해
다시 읽었다. 이전 map audit의 후보를 실제 export와 대조했다. 원본 material10개 permutation과
5개 distortion companion을 기존 native 생성기로 복원했으며 deferred0이다. shader program5363~5372와
기존 group5312 mesh/particle carrier를 사용한다. 별도 렌더러나 global rendering option 변경은 없다.

### 실제 맵 입력

| 원본 actor | runtime 위치 | rotation degrees | scale | warmup |
|---|---|---|---|---:|
| lv_lut_heartrb_ed_ps.theworld.persistentlevel.emitter_18 | [156.574375,60.043125,-121.97296875] | [180,0,0] | [0.359999991953373,0.479999989271164,0.359999991953373] | 3000ms |
| lv_lut_heartrb_ed_sl00.theworld.persistentlevel.emitter_1 | [158.42,138.4,-125.12] | [0,0,0] | [0.5,0.5,0.5] | 8000ms |

spacehole component의 `color_emissive=[2.799999952316284,0.10000000149011612,0.05000000074505806]`,
`color=[0,0,0]`, `cloud_brightness=2.5` instance parameter도 기존 source parameter projection으로 적용했다.
base asset만 보면 파란색으로 오인할 수 있으나 맵 인스턴스는 이 원본 override를 가진다.

`VALTAN_SIX_PIZZA_106/STEP_01`에 map anchor·snapshot·ARENA_ABSOLUTE cue2개를 연결했다.
source warmup은 playbackOffsetMs3000/8000으로 seek하며, stage0~20400ms 표시 후 cue_end로 정리한다.
20.4초는 현재 피자 STEP1~7의 첫 착지까지를 연결한 저작 시각이다. 원작의 정확한 activation/비행
시각 복원이 완료됐다는 뜻은 아니다. 별도의 발탄 비행 motion·camera·Server leap는 변경하지 않았다.
사용자는 해당 두 cue의 map transform·표시 구간을 기존 pattern authoring에서 조정할 수 있다.

## G02. 코드·데이터·배포 경계

- Data/Effects/Authored에 sky source2개, EffectCatalog/EffectResourceTree에 stable ID2개를 추가했다.
- Client.vcxproj와 filters의96.DataFiles에 새 JSON2개를 None으로 등록했다.
- 기존 native installer가 Effect_ArtistMaterial_Tables.inl, Shader_EffectKoukuNativeGroup5312.hlsli,
  Shader_EffectArtistNativeDispatchKoukuNativeCases5312.hlsli,
  Shader_EffectArtistNativeSelectedGroup5312.hlsli의 해당 program만 추가했다.
- effect source36개가 아니라, 이번3개 source 문서가 참조하는 물리 resource는 합쳐서36개다.
  그중 sky2문서의 closure는21DDS+3WModel=24개이며 모두 현재 PC에 존재한다.
- 새로 설치한 sky 전용 DDS는 Effect/Valtan/Sky/Textures/fx_tex_02/fx_d_atypical_126.dds다.
  나머지20DDS는 검증한 기존 exact texture 경로를 재사용한다. 원본 geometry3개를 기존 공용
  Effect/KoukuSaydon/FullRestore/Meshes에 cook해 기존 projector와 동일한 경로를 사용한다.
- Resources는 AGENTS의 Drive 관리 입력이므로 Git에 force-add하지 않는다. 다른 PC는 아래 물리
  자산을 팀 Drive 동기화로 받아야 한다. 별도 immutable pack/manifest를 완료 조건으로 만들지 않았다.
- 통합 담당이 Valtan·Composition·Gameplay publisher와 Debug/Release Product build를 수행한다.
  이 문서의 단독 compile만으로 배포된 EXE/CSO가 최신이라고 주장하지 않는다.

### sky가 참조하는 Resources-relative 물리 경로

```text
Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_atypical_073.dds
Effect/Esther/Inanna/Textures/FX_TEX_02/fx_d_shockwave_002_2_ycl.dds
Effect/Esther/Ninave/Textures/FX_TEX_02/fx_d_electric_016.dds
Effect/KoukuSaydon/FullRestore/Meshes/bfm_d_supercell_02.wmodel
Effect/KoukuSaydon/FullRestore/Meshes/fm_d_electric_02.wmodel
Effect/KoukuSaydon/FullRestore/Meshes/fm_d_supercell_07.wmodel
Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_00/fx_a_noise_012_n.dds
Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_00/fx_d_normal_070.dds
Effect/KoukuSaydon/Textures/FX_TEX_01/fx_c_line_004_ycl.dds
Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_019.dds
Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_noise_009.dds
Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_noise_030.dds
Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_shockwave_001_ycl.dds
Effect/KoukuSaydon/Textures/FX_TEX_04/fx_f_rectangle_001.dds
Effect/KoukuSaydon/Textures/FX_TEX_04/fx_i_environment_001.dds
Effect/KoukuSaydon/Textures/FX_TEX_04/fx_i_noise_05.dds
Effect/KoukuSaydon/Textures/FX_TEX_05/fx_k_cloudtilie_01.dds
Effect/KoukuSaydon/Textures/FX_TEX_05/fx_k_electile_02.dds
Effect/KoukuSaydon/Textures/FX_TEX_05/fx_k_fluidtile_01.dds
Effect/KoukuSaydon/Textures/FX_TEX_05/fx_m_wave_001_ycl.dds
Effect/Valtan/Sky/Textures/fx_tex_02/fx_d_atypical_126.dds
Effect/Valtan/Textures/FX_TEX_02/fx_d_cloud_031.dds
Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_030.dds
Effect/Warlord/Textures/FX_TEX_03/fx_e_cloud_009.dds
```

## G03. 실행한 검증과 남은 확인

| 검증 | 실제 결과 |
|---|---|
| 변경 JSON6개와 vcxproj/filter XML parse | PASS |
| 해당 source3개의 element 이름·색 공간·particle option·module override·attachment·resource 경로 검사 | PASS,36개 물리 resource 존재 |
| 실제 CEffectDocumentCodec Load와 CEffectPlayback Stage | sky2개와 기존/현재 검격 PASS |
| native CPU sky playback | warmup 뒤0/0.25/1/5/10/15/20초,157개 particle packet, 모든 World/SourceEmitterWorld finite |
| sky emitter coverage | source spawn 가능한2+11개 전부 관측, dormant emitter1개는 원본 근거 확인 |
| pivot 위치 보존 |40개 world matrix 최대차0, source의 다른 필드 구조 동일 |
| 새 Effect_ArtistMaterial C++ compile | PASS; 공용 Engine_Enum.h 기존 C4819 경고 |
| group5312 mesh/particle FXC fx_5_0 O1 | 둘 다 PASS; 공용 X4000 및 effect deprecation 경고 |
| 전체 Validate-EffectSources.ps1 | 기존 valtan.trail.7fcde5bbca0fbc103367e216의 v15 baked history clamp/sample closure 오류로 중단 |
| 해당 파일 git diff --check | PASS |
| Client/UI 실행·GPU 표시·체감 방향·실제 fly-through | 미실행, 사용자 확인 필요 |

전체 source validator 실패는 숨기지 않았으며 해당 trail은 이번 source 수정 대상이 아니다.
변경3개 source에 같은 validator의 적용 가능한 함수들과 실제 native Codec 검사를 직접 실행해 분리 확인했다.
원본 blue/red 혼동, spawn0 emitter, 공유 자산 소비자 누락 방지는 rendering guide와 gotchas에 반영했다.

증거는 `out/ValtanEffectCorrections20260930`의 source_module_inputs.json(하위 source),
source_occurrences.json, sky-map-placement.json, sky-installation.json, material/native·reviewed,
source/geometry/installation.json, native-probe.log, validate-effect-sources.log와 원본 저장본 backup이다.
이 경로는 검증 산출물이며 runtime이나 Git 배포 정본이 아니다.


## G04. 후속 요청: 발악 끝에서 바로 망령 부활

현재 저작 clip과 원본 reference를 대조해 추가 연출의 소속을 확인했다. STRUGGLING 끝은
STEP_10 `mesh_att_battle_5_01_end`4433ms, STEP_11 `mesh_att_battle_19_05`2333ms,
STEP_12 `mesh_att_battle_19_06`333ms다. reference `Valtan.clipseq`의 원본420624 seq1~3도
19_05→19_06 꼬리를 가지며 `valtan.cinematic.finale`는 그 발악 원본 clip 목록에 없다.

수정 전 STEP_12 TIMEOUT은 별도 `VALTAN_GHOST_DEATH_AUDITION`으로 이동했다. 그 패턴의
현재 단일 clip은 `valtan.cinematic.finale`23000ms이고 원본 Matinee blended animation을 가리킨다.
이후에야 `VALTAN_GHOST_RESPAWN_AUDITION`의 `mesh_respawn_1`3000ms를 실행했다.
따라서 추가23초가 발악 내부 clip이 아니라 별도 사망 패턴이라는 점은 데이터로 확인됐다.
앉는 정확한 frame의 영상 판정은 Client 실행 없이 수행하지 않았다.

사용자 후속 지시에 따라 `Valtan.gameplay.json`의 STRUGGLING/STEP_12/TIMEOUT
`nextPatternId` 한 필드만 `VALTAN_GHOST_RESPAWN_AUDITION`으로 바꿨다.
DEATH 라이브러리·독립 재생·명시적 PlayAll 저장 목록은 유지했다. 이후 자동 HP기믹은 발악이
완료되면 기존 generic follow-up으로 부활을 바로 선택한다. ValtanBrain의 ApplyStageBranch는
nextPatternId를 pinned pending receipt로 보관하며 다음 tick에 pursuit보다 먼저 선택한다.
STRUGGLING→DEATH를 별도로 강제하는 production C++ 경로는 검색에서 없었고 변경하지 않았다.
Respawn ENTER의 phase3와 완료 후 기존40줄 ghost loop activation은 그대로 소비한다.

기존 연속 레이드 contract test의 revival 기대값만 DEATH+RESPAWN에서 RESPawn 하나로 바꿨다.
독립 DEATH→RESPAWN 검증은 보존했다. 이 하위 작업에서는 해당 대규모 레이드 검사를 실행하지 않았다.
JSON parse, 직전 백업과의 구조 비교에서 정확히 한 필드 차이, 해당 diff check를 확인했다.
최종 게시·통합 빌드 및 가능한 Server 검증 결과는 통합 담당의 RESULT에 기록한다.

## G05. 공유 검격 모양 반시계90도와 발탄 정면 이동

사용자는 모양을 반시계90도로 돌렸을 때 오른쪽으로 흐르는 이동을 발탄 정면으로 고치고,
피자와 동일 이펙트를 쓰는 지형 파괴에도 함께 적용하도록 승인했다. 삭제한 emitter13은
복구하지 않았다. 기존 Atk_08_04 공유 source의 남은7개 Element를 유지했고 별도 파생
이펙트나 cue를 만들지 않았다.

### 원인과 실제 변경

발탄 visual root와 owner yaw는 기존 이동 경로에 이미 들어간다. Particle System yaw를
-90도로만 바꾸면 초기 속도도 같은 방향으로 돌아가므로 모양이 바뀐 뒤 옆으로 흐른다.
기존 directionYawDegrees를+90도로 설정해 초기 속도에서 이 회전을 상쇄했다.
원본 EF velocity-over-life는 XYZ 공통 배율이므로 같은 전방 이동을 유지한다.

실제 필드 변경은 아래4개다.

| 필드 | 이전 | 반영 |
|---|---:|---:|
| particleSystem.yawOffsetDegrees | 0 | -90 |
| particleSystem.directionYawDegrees | 0 | 90 |
| emitter36 detail.sprite.followEmitterAxisRotation | 미지정(false) | true |
| emitter43 detail.sprite.followEmitterAxisRotation | 미지정(false) | true |

후자의2개는 source fixed-axis sprite의 최종 quad가 같은 emitter basis를 소비하도록 한다.
5개 mesh와7개 Element의 기존 position/rotation/scale, sourceRecipe, 재질, 색, timing,
사용자 삭제를 보존했다. 이전에 재기준화한 고정 발사 중심0도 유지했다.
이는 이동하는 전체 화면 AABB의 중심을 매 프레임 추적하는 방식은 아니다.

다음3개 occurrence의 stable ID, 위치, 회전, scale, anchor/follow, 시각은 직전 저장본과 같다.

- VALTAN_SIX_PIZZA_106 / STEP_11
- VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK / COMBO_STEP_11
- VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK / COMBO_STEP_11

### 설치·게시와 검증

최신 source SHA를 후보 기준과 대조한 뒤 백업·교체 직전 SHA 검사·원자 교체를 수행했다.
설치 영수증은 `out/ValtanEffectCorrections20260930/installed-20260930T100125514248Z/receipt.json`이다.
설치 SHA256은 `7ef33f2a2a6fd9bac1c19f5167fdac0c0733ceb30edfecd9d3bb3cb9d463ac6d`다.

| 검사 | 실제 결과 |
|---|---|
| JSON parse와 원본 대비 semantic diff | 정확히4개 필드,7개 Element 유지 |
| 해당 문서 공통 validator5개와 Resources closure | PASS, 누락0 |
| 설치본 실제 native Codec/Stage | PASS,1문서 실패0, resource boundary12개 통과 |
| native Playback/Geometry, owner yaw0/45/90/180/270·시각8개 | PASS,200개 particle sample |
| 회전 전후 이동 속도 최대차 | 0.0000076, 기존 전방 이동 유지 |
| 모양 반시계90도 basis 최대오차 | 0.000000179 |
| 시간에 따른 이동 차이 최대오차 | 0.00000285, 회전한 초기 배치 offset만 유지 |
| canonical/생성 cue 소비자 비교 |3개 모두 동일 공유 asset, 기존 cue 보존 |
| Publish-GameplayBalance.ps1 -Mode Publish | 성공,109863 rows·32324771 bytes |
| 게시 presentation generation·bootstrap 참조 |176개 artifact hash와 현재 파일 일치 |
| Client/UI·GPU·실제 Play Preview | 미실행, 사용자 화면 확인 영역 |

새 generation은 `7d30d19f522e2a28499207fc650aea7c141b36bf063c39785fe4f2f211c9c77b`이며
Gameplay.bootstrap의 PATTERNPRESENTATIONGENERATION이 이를 참조한다.
Effect ID·cue·split 문서는 이번 변경에서 바뀌지 않아 해당 projection을 다시 쓰지 않았고,
정상 publisher의 split 정합성 검사를 통과한 뒤 direct authored effect hash를 갱신했다.
이번4필드 수정에는 C++·shader·Resources 변경과 제품 재빌드가 필요 없다.
파일 설치·게시를 실행 중 도구의 draft나 Server 메모리 갱신으로 간주하지 않는다.

증거는 같은 out 폴더의 `shared-candidate-validation.json`, `visual-rotation-summary.json`,
`facing-rotated-geometry.jsonl`, `gameplay-publish.log`, `shared-installed-verification.json`이다.
