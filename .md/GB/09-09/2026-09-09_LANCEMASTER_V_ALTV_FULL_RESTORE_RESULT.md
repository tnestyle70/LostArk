# 창술사 전체 스킬·Alt+V 말 복원 결과

## G00. 2026-09-10 실제 구현 상태

사용자가 V clip2 full restore 화면을 승인한 뒤, 두 stance의 LMB부터 현재 `LanceMaster.skillbindings.json`에 연결된 전체 Action으로 확대했다. 기존 unified 저작본과 V3개 full 문서는 보존했다. 신규39개 full 문서603Element와 기존 V/Alt+V7개 문서541Element를 연결할 입력으로 준비했다. 최종46문서에는 총1,144Element와 말12ModelCue가 있다. 실제 공용 Catalog/ResourceTree/animevents 및 제품 빌드는 통합 담당의 RESULT에서 확인한다.

| 기존 문서 | 최종 Element | ModelCue |
|---|---:|---:|
| `effect.lancemaster.skill.34610.clip1.full.restore` | 50 | 0 |
| `effect.lancemaster.skill.34610.clip2.full.restore` | 78 | 0 |
| `effect.lancemaster.skill.34610.clip3.full.restore` | 29 | 0 |
| `effect.lancemaster.skill.34630.clip1.full.restore` | 162 | 0 |
| `effect.lancemaster.skill.34630.clip2.full.restore` | 118 | 4 |
| `effect.lancemaster.skill.34630.clip3.full.restore` | 57 | 4 |
| `effect.lancemaster.skill.34630.clip4.full.restore` | 47 | 4 |

신규39문서와 기존7문서 모두 actual `CEffectDocumentCodec::Load -> Save_Atomic -> Load -> Serialize` 및 개별 Solo 필터/Drawable 검사를 통과했다. 이것은 화면 fidelity 승인이 아니다. 사용자가 직접 판단한 V clip2 이외의 현재 화면은 미확인이다.

## G01. 원본 Action·CDO·발생 선택

설치 `data3.lpk/XmlData/Action/LANCEMASTER.loa`의 현재24skill binding을 기준으로 정확한 clip 이름과 순서로 base Action stage를 선택했다. 같은 clip 이름을 재사용하는 tripod variant는 합치지 않았다. V/Alt+V7단계 외45단계에서 활성 PlayParticleEffect175회, particle/light/decal/post 원본 발생636개를 회수했다. current-native graph의 외부 import gap은0이고 필요한 CDO는207개다. ParticleParameter override는 이름뿐 아니라 scalar/vector 타입이 맞을 때만 적용한다. 다른 타입과 `PSPT_None`은 원본 distribution 기본값을 보존한다.

원본 활성636개에서603개를 새 문서에 설치했다.33개는 native decal VF12, 원본 engine/VS 입력 미완결9, ribbon history8, material 미지정2, Solo sibling provider 필요2다. 원본에 활성 렌더 발생이 없는6단계는 가짜 Element를 만들지 않고 별도 기록했다: stance `Mode_Identity1/2`, `SK_ChestDestruction_03`, `SK_PenetrationLunge_01/04`, `SK_DragonUpfly_03_re`. 구체적인 source cue/emitter/material·disabled/empty Action은 `2026-09-09_LANCEMASTER_ALL_FULL_RESTORE_EXCLUSIONS.json`에 보존한다.

기존 V/Alt+V의 base483발생, 말 child63, camera11, static42, 말12ModelCue의 원본 조사611개는 유지한다. 이번 Alt+V load/Solo 교정에서 추가로 collision2발생과 활성 event-generator/미연결 receiver3발생을 실행 문서에서 제외했다. 원본 V/Alt+V 후보·신규 후보 합계1,247개에서1,144Element+12ModelCue를 설치했으며, 나머지91개는 source 제외 사유가 있다.

## G02. Alt+V 말 애니메이션과 attachment

실제 모델은 `SK_FLM_HOR_00.Mesh.SK_FLM_HOR_00_SK`다. `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0.wmodel`부터 section3까지,152cooked node와 동일한3clip을 사용한다. 원본 source151joint 이름은 PSA/glTF 사이 전단사이고 glTF hierarchy를 유지한다. Action override는 `sk_flm_hor_01.mat.sk_flm_hor_01_01_mi_dead`, `_02_mi_dead`, `_03_dead`, `_04_mi_dead` 네 재질이다.

이번에 말이 멈춰 보일 수 있는 실제 시계 결함을 확인했다.4개 WModel이 duration/key time을1000ticks/s로 저장하지만 Engine은 cooked animation을30ticks/s로 진행했다. duration과 모든 key time253,680개를30/1000으로 정규화하고 stored rate도30으로 저장했다. key pose와 hierarchy·mesh·material은 유지했다. 기존 `retime_wmodel_ticks.py`를 이용하며 반복 실행 시30tick 문서를 다시 축소하지 않는다.

| Clip | Engine30tick에서 실제 길이 | Alt+V 문서 |
|---|---:|---|
| `sk_super_squalllance_02` | 2.2초 | clip2 |
| `sk_super_squalllance_03` | 1초 | clip3 |
| `sk_super_squalllance_04` | 1.366667초 | clip4 |

말 child attachment62Element의 runtime anchor ID에 modelCue owner를 포함했다. character `B_Root`와 horse `B_Root`가 서로 다른 owner인데 같은 runtime slot을 썼던 충돌을 제거했고, 원본 sourceAnchorSlotId와 bone 이름·transform은 보존했다. Solo는 해당 parent ModelCue만 hidden anchor provider로 남기며 독립 검증62dependency가 통과했다. collision2발생과 sibling event3발생은 말 자체의 model/clip을 제거하지 않고 정확히 분리했다.

말 source glTF와 retimed runtime key를 독립 CPU로4sections×3clips×5times 비교했다. 최대 bind local 오차2.45e-6, 최대 pose combined matrix 오차4.75e-7 미만이다. 통합 담당의 실제 CModel경로 `Set_TrackPosition/Play_Animation(0)`도 말4개를 포함한6models/21clips,189sample의 finite palette·정상 wall-clock·모든 clip pose변화를 확인했다. 그 evidence는 `out/ThreeClassFullRestore20260910/summon-models.log`다.

## G03. 재질·shader와 geometry

기존176개 native profile과 승인된 V 프로그램의 함수 본문은 유지했다. 신규 material/VF156조합에서155개의 native RT0 프로그램을 추가했다. 번호는1200~1355중1320을 제외하며 header 총331profile이다. `Shader_EffectLanceMasterVANativeGroup1152/1216/1280/1344.hlsli`에 기존64개단위 family 선택을 확장했다. 새 native1320은 source-world 행렬/VF 입력 미완결로 미설치다.

신규 PS/VS308개를 실제 설치 shader map에서 회수하고 current cache layout을 새로 파싱했다. 원본 uniform 표현식, sampler clamp/sRGB, 실제 MIC override와 texture identity를 보존했다. zero-texture source PS는 byte188의 실제 serializer array와 DXBC/CB범위를 함께 검증했다. `frac` uniform은 source frac연산으로 번역했다. 신규 screenPost1228은RGBsplit,1255는Zoomblur이고,1325sprite는SceneColor를 소비한다. 전체 제품 shader compile은 통합 담당이 수행한다.

원본 particle mesh25개를 추가로 typed CModel geometry로 cook했고 모두 성공했다. 기존 source V용30mesh·static21sections·말4sections는 같은 경로를 유지한다. 이전 GDR01의 source tangent4개 수리는 별도 이전 receipt를 유지하며, 추가 복원에서 그 메쉬나 승인된 V2 geometry를 다시 변경하지 않았다.

## G04. Resources와 전달

이번 추가 설치는25WModel과58DDS, 교체는말4WModel이다. 물리 위치는 `Client/Bin/Resources` 아래이며 Resources binary는 Git에 추가하지 않았다. 새 파일 합계는7005108bytes다. Drive 전달은 팀장의 해당 물리 폴더 배포 대상으로 남아 있다. 새 native 원본, GLTF/cook/texture 기록과 설치 전에 보존한 말4파일은 `out/LanceMasterAllRestore20260910`에 있다. `.md`에는 binary manifest/잠금팩을 완료 조건으로 만들지 않는다.

| 추가·교체 | Resources 상대 ID |
|---|---|
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_a_cylinder_004.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_a_hemisphere_012.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_a_plan_001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_b_circle_009.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_b_halfsphere_001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_cone_006_1.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_helix_011.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_helix_013.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_helix_031.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_hemisphere_001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_hemisphere_002.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_hemisphere_005.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_torus_002.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_d_trail_005.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_f_cylinder001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_f_cylinder_001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_i_swing_02.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_k_helix_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_k_tornado_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_m_ring_001.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_00/fm_o_swing_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_01/fm_k_dragonhead_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_01/fm_k_halfcylinder_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_01/fm_k_wave_01.wmodel` |
| 추가 | `Effect/LanceMaster/Meshes/Native/FX_SM_01/fm_m_trail_002.wmodel` |
| 추가 | `Effect/LanceMaster/Textures/EFMASTER_MATERIAL_PROLOGUE/foam_01.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_P_W_01/fx_c_glow_0001.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_atypical_015_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_atypical_024_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_atypical_028_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_cloud_003.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_cloud_005.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_cloud_016.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_decal_014.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_decal_014_n.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_fragment_005.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_line_003.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_line_010.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_noise_006.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_a_trail_006.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_b_atypical_014.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_00/fx_bg_lightbeam_falloff_03.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_01/fx_c_hit_003.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_cloud_016.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_decal_003.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_decal_043.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_electric_015_1.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_fluid_004.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_fluid_020.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_fragment_016.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_hit_006_1_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_normal_017_1.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_normal_033_1.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_normal_042_1.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_normal_045_1.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_02/fx_d_shockwave_003_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_atypical_004.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_decal_003.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_decal_010_e_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_decal_013.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_decal_013_e.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_fire_005.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_03/fx_e_fluid_021.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_04/fx_f_aura_004.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_04/fx_h_wave_02.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_04/fx_j_exsmokes_01.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_04/fx_j_ylinestream_01_ycl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_k_auraline_01.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_k_auraline_08.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_k_fluidtile_01.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_k_smoke_02.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_m_atypical_008.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_m_magicsymbol_005.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_m_trail_004.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_05/fx_m_trail_015.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_HIGH_00/fx_c_cloud_013_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_HIGH_02/fx_k_tslr_magiccircle_02.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_HIGH_02/fx_l_symbol_08_1_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_NOMIPMAP_00/fx_c_flow_004.dds` |
| 추가 | `Effect/LanceMaster/Textures/FX_TEX_NOMIPMAP_00/fx_j_levelupsymbol_01_cl.dds` |
| 추가 | `Effect/LanceMaster/Textures/MN_VOGC_00/mn_vogc_00_parts1_d.dds` |
| 추가 | `Effect/LanceMaster/Textures/MN_VOGC_00/mn_vogc_00_parts1_n.dds` |
| 추가 | `Effect/LanceMaster/Textures/MN_VOGC_00/mn_vogc_00_parts1_s.dds` |
| 교체 | `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0.wmodel` |
| 교체 | `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section1.wmodel` |
| 교체 | `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section2.wmodel` |
| 교체 | `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section3.wmodel` |

새39문서가 직접 참조하는235개 asset은 모두 safe Resources-relative 경로이고 물리 존재를 검사했다. 기존 V/Alt+V297개 참조의 이전 준비 증거와 구분한다.

## G05. 자동 검증과 실행 준비

| 실행한 검사 | 결과 |
|---|---|
| source exact clip stage/active PPE/CDO/current graph |45단계/175call/636발생/207CDO, import gap0 |
| 새 document JSON parse/stable Element ID/safe Resource path |39문서603Element/235asset 실제 존재 PASS |
| 실제 Codec Load/Save/Reload와 모든 Element Solo |46문서1,144Element, hidden0/locked0/unexpected0, 말 anchor62 PASS |
| `all-codec-solo.log` |44문서1,062Element PASS, 최초34550/AltVclip3 load blocker기록 |
| blocker교정뒤 `fixed-codec-solo.log` |34550 25Element와AltVclip3 57Element,82/82 PASS |
| source geometry25개 |25/25 typed cook PASS |
| 말4개 tick normalization |12clips, key253,680개, source wall-clock 유지 PASS |
| 말 source/runtime pose비교 |60samples, bind/pose 오차위경계 PASS |
| PSPT_None 공용 typed payload 회귀 |`test_build_action_cue_recipe.py`12test PASS |
| 관련 `git diff --check` |PASS |
| Product ClCompile/FXCompile/link |통합 담당의 최신 RESULT로 구분 |
| Client/UI자율실행·조작·캡처 |미실행 |
| 화면 fidelity |V clip2는사용자승인, 이번변경분은사용자확인전 |

`prepare_lancemaster_all_full_restore.py`는 bound Action 원본을 회수하고, `generate_lancemaster_all_full_restore_documents.py`는 native admission·말tick·anchor·독립Solo범위를 적용한다. `install_lancemaster_native_extension.py`는 기존176program을 보존하며 새155program만추가한다. raw회수/native번역 입력은 out의 source evidence를 사용한다. 새로운 C++파일은 없고 shader group,39Data JSON의 프로젝트등록과실제animevents 연결은 root가 처리한다.

## G06. 남은 원본 경계

말 hair sourceUV1 alias, source deferred 환경cube/SH/MRT, 말 own-transform의 미분해 원본필드와 기존 root identity/yaw-90 프로젝트해석은 이전과 같다. source Action의 별도 PostProcessChain, DominantDirectionalLight, PawnMaterialParam, TrailGhost/HidePawn을 이번 Element수에 포함하거나 연결완료로 기록하지 않는다. G18에서 실제 CModel metadata와 CAnimation 소비자를 다시 확인했다. serialized FPS24를 그대로 나눈 1.25배 길이는 제품 재생 시간이 아니다. Engine 고정 runtime30으로 읽은 V 6.433초/T 2.433초가 Server 곡선의 시계와 일치한다. 말 자체의1000→30tick cook 수정과 playable runtime30 계약을 구분한다.

source light의 particle StartSize/Color/Alpha를 component 최종값에 합성하는 의미와 native decal projector/ribbon history/일부 engine VS는 제외목록에 남는다. 이 문서는 실제 source후보, 설치, 구조·실행검증, 사용자화면승인을 구분하며, 아직실행하지않은 제품link나화면을PASS로 기록하지 않는다.

## G07. 새 full 문서와 bound clip

| Bound clip | Effect asset ID | Element |
|---|---|---:|
| `flm_sk_dragoncleave_01` | `effect.lancemaster.skill.34650.clip1.full.restore` | 30 |
| `flm_sk_dragoncleave_03` | `effect.lancemaster.skill.34650.clip2.full.restore` | 74 |
| `flm_att_identity1_1_01` | `effect.lancemaster.skill.34010.ba1.full.restore` | 3 |
| `flm_att_identity1_1_02` | `effect.lancemaster.skill.34010.ba2.full.restore` | 3 |
| `flm_att_identity1_1_03` | `effect.lancemaster.skill.34010.ba3.full.restore` | 3 |
| `flm_att_identity1_1_04` | `effect.lancemaster.skill.34010.ba4.full.restore` | 8 |
| `flm_sk_sharpswing_01` | `effect.lancemaster.skill.34040.clip1.full.restore` | 8 |
| `flm_sk_sharpswing_02` | `effect.lancemaster.skill.34040.clip2.full.restore` | 8 |
| `flm_sk_dragonkick` | `effect.lancemaster.skill.34090.full.restore` | 11 |
| `flm_sk_cycloinclance_01` | `effect.lancemaster.skill.34100.clip1.full.restore` | 13 |
| `flm_sk_cycloinclance_02` | `effect.lancemaster.skill.34100.clip2.full.restore` | 11 |
| `flm_sk_cycloinclance_03_re` | `effect.lancemaster.skill.34100.clip3.full.restore` | 20 |
| `flm_sk_riseup_01` | `effect.lancemaster.skill.34160.ba1.full.restore` | 35 |
| `flm_sk_riseup_02` | `effect.lancemaster.skill.34160.ba2.full.restore` | 12 |
| `flm_sk_chestdestruction_01` | `effect.lancemaster.skill.34140.ba1.clip1.full.restore` | 7 |
| `flm_sk_chestdestruction_02` | `effect.lancemaster.skill.34140.ba1.clip2.full.restore` | 9 |
| `flm_sk_chestdestruction_04` | `effect.lancemaster.skill.34140.ba2.clip2.full.restore` | 37 |
| `flm_sk_threetalonstrike_01` | `effect.lancemaster.skill.34120.clip1.full.restore` | 11 |
| `flm_sk_threetalonstrike_02` | `effect.lancemaster.skill.34120.clip2.full.restore` | 15 |
| `flm_sk_threetalonstrike_03` | `effect.lancemaster.skill.34120.clip3.full.restore` | 26 |
| `flm_sk_crescentsweep` | `effect.lancemaster.skill.34110.full.restore` | 16 |
| `flm_sk_crushingblow` | `effect.lancemaster.skill.34150.full.restore` | 29 |
| `flm_sk_moving_normal_1` | `effect.lancemaster.skill.34020.full.restore` | 3 |
| `flm_sk_standup_normal_1` | `effect.lancemaster.skill.34030.full.restore` | 7 |
| `flm_att_identity2_1_01` | `effect.lancemaster.skill.34510.ba1.full.restore` | 9 |
| `flm_att_identity2_1_02` | `effect.lancemaster.skill.34510.ba2.full.restore` | 9 |
| `flm_att_identity2_1_03` | `effect.lancemaster.skill.34510.ba3.full.restore` | 9 |
| `flm_sk_stab` | `effect.lancemaster.skill.34540.full.restore` | 16 |
| `flm_sk_talonstrike` | `effect.lancemaster.skill.34550.full.restore` | 25 |
| `flm_sk_penetrationlunge_02` | `effect.lancemaster.skill.34560.clip2.full.restore` | 3 |
| `flm_sk_penetrationlunge_03` | `effect.lancemaster.skill.34560.clip3.full.restore` | 20 |
| `flm_sk_dragonupfly_01` | `effect.lancemaster.skill.34570.clip1.full.restore` | 12 |
| `flm_sk_dragonupfly_02_re` | `effect.lancemaster.skill.34570.clip2.full.restore` | 26 |
| `flm_sk_counterattack_01` | `effect.lancemaster.skill.34580.ba1.full.restore` | 9 |
| `flm_sk_counterattack_02` | `effect.lancemaster.skill.34580.ba2.full.restore` | 15 |
| `flm_sk_lastwhisper_start` | `effect.lancemaster.skill.34590.ba1.full.restore` | 11 |
| `flm_sk_lastwhisper_loop` | `effect.lancemaster.skill.34590.ba2.full.restore` | 12 |
| `flm_sk_lastwhisper_end` | `effect.lancemaster.skill.34590.ba3.full.restore` | 27 |
| `flm_sk_moving_normal_2` | `effect.lancemaster.skill.34520.full.restore` | 1 |

## G18. 사용자 승인 A 검격의 BA/Q/W/E/R/S 적용과 V 마지막 돌진 재생

2026-09-10 사용자 후속 요청에 따라 긴 창 BA4단계, Q2clip, W, E3clip, R2단계와 S3clip에 A 첫 반원 검격 4Element를 적용했다. 이는 원본 검격을 그대로 재현했다고 주장하는 변경이 아니라 사용자가 승인한 검격을 사용하는 프로젝트 저작 교체다. A 템플릿은 `effect.lancemaster.skill.34140.ba1.clip1.full.restore`의 `fx_pc_flm_02.par_o_flm_chestdestruction_01_1`이며 native593/1238/595와 기존 mesh/texture/curve를 그대로 소비한다.

| 대상 | 이번 처리 | 현재 Element 합계 |
|---|---|---:|
| LMB34010 BA1..4 | 원래 타격 시각에 A 검격 4개씩 |25|
| Q34040 clip1/2 | 원래 검격 시작 시각에 교체 |12|
| W34090 | 용 꼬리 검격 3개를 A4개로 교체 |12|
| E34100 clip1/2/3 | 첫 구간 A4개 추가, 뒤 구간 검격만 교체 |50|
| R34160 BA1/2 | 첫 구간 5회, 두 번째 1회 시점 유지 |54|
| S34120 clip1/2/3 | 1회+1회+2회, 총4회 검격 |39|
| V34610 clip3 | 첫 시퀀스 spear11개를 마지막 돌진 local0.4초에 추가 |40|

16문서에서 승인된 교체 범위의 기존 검격75개를 제거하고91개를 추가했다. 교체 대상 밖 기존141Element와 각 문서의 비-elements 필드는 모두 이전 사용자 저장본과 deep equality가 일치했다. 새91Element는 ID, sourceNode, 표시명, 비활성 sourcePresentation, 목적지 시작 시각 외 모든 값이 원본 템플릿과 동일했다. `sourceNode=authored-copy:<원본 Element ID>`로 기존 native identity와 source RNG seed 해석을 유지한다. 재생성 도구는 `Tools/EffectPipeline/retune_lancemaster_full_restore_slashes.py`이며 최신 사용자 저장본을 먼저 보존하고 이미 적용된 항목을 다시 덮어쓰지 않는다.

S source-local 시각은 clip1 0.463206초, clip2 0.531423초, clip3 0.433057/0.802980초다. 현재 binding의 clip1 playMs900/playRate1.2, clip2 playMs582, clip3 playRate1.1을 그대로 유지했다. BA의4단계/R의2단계는 기존 Server comboStage 메시지와 실제 binding을 유지하며 클라이언트가 단계를 추가하지 않는다. 변경16문서는 기존 `effectref=asset` animevent가 각각 하나씩 실제 full ID를 참조하고, startMs0/root/follow/natural 계약도 바뀌지 않았다.

V 마지막 spear 재생은 기존 첫 시퀀스 `fx_pc_flm_00.par_k_flm_squalllance_spear_00` 11Element를 재사용한다. clip3 local0.400000006초, 전체 action 약5.266667초이며 원래 clip3의29Element와 사용자 승인 V clip2는 보존했다. 실제 Server는 `PlayerSkillSystem.cpp`의 RootMotion 우선 경로를 사용한다. `movementDistance=0`이어도 published V194개/T74개 곡선 표본이 존재하고 Data 원본과268개 전부 일치했다. V는 약5.33~5.60초에 전방4.08m 이동하며 현재 모델 runtime30 시계와 맞는다. RootMotion/PlayerSkills/Server 코드를 이번 검격 교체에서 변경하지 않았다. 공유 LAN Server가 어떤 배포본을 실행하는지는 이 read-only 검사의 완료 근거에 포함하지 않는다.

| 실행한 검사 | 결과 |
|---|---|
| 실제 제품 Codec Load/Save/Reload, 모든 Element Solo |16문서232Element, Solo232, presentation23, 실패0|
| 기존141Element와 비-elements 필드 보존 |PASS|
| 신규91Element 원본 템플릿 값 대조 |허용5필드 외 전부 동일 PASS|
| 실제 full asset animevent join |16/16 정확히1행 PASS|
| V/T 실제 model runtime30 + published RootMotion |5clip/268curve표본 PASS, 데이터 수정0|
| 수정 JSON parse, scoped git diff --check |PASS|
| Client/UI 실행·조작·화면 캡처 |미실행|
| 화면 fidelity 및 LAN Server 실제 돌진 관찰 |사용자 확인 전|

수치 근거는 `out/LanceMasterSlashFollowup20260910/codec-result.log`, `delta-review.json`, `product-binding-join.json`, `root-motion-clock-evidence.json`, `slash-materialize-result.json`에 있다. 이번 검격 추가로 신규 Resources나 shader compile 항목은 발생하지 않는다.

G18 검격16문서에는 T를 포함하지 않는다. T의 원본 `PlaySkeletalMesh` 연결과 후속 재질 입력 교정은 아래 G19에 기록한다.

## G19. T 용 복원 재개와 투명도 입력 교정

T34650의 현재 full restore는 clip1 30Element, clip2 80Element와2ModelCue다. 원본 `SK_FLM_GDR_01.Mesh.SK_FLM_PMSHB_00_SK`의 두 section, animation `SK_DragonCleave_03`, material `FX_M_MI_T_00.FX_MI.FX_T_Me_Master_02_01_Sk_Dt_Tr`와 child `Par_T_FLM_DragonCleave_01_Cast_01`를 사용한다. clip2의 기존74Element에 본 부착 child6개와 용 cue2개가 추가되어 있고 이번 재개에서 데이터와 시각을 바꾸지 않았다. 용은 clip2 local0.5초부터1.2초 동안 재생한다.

모델이 존재해도 보이지 않는 결함은 skeletal cue의 원본 DynamicParameter가 shader 입력까지 전달되지 않는 데 있었다. native1360의 opacity는 이 값의 x를 사용한다. 기존 zero-init particle 값을 전달하면 noise가 있어도 모든 alpha가0이 된다. `Shader_VtxAnimMeshBinary.hlsl`의1360 분기에서 이미 bind된 원본 재질 parameter8을 사용하도록2줄을 추가했다. 다른 native profile의 입력은 보존했다.

| 실행한 확인 | 결과 |
|---|---|
| 기존 실제 CModel 재실행 | 모델2개, clip2개, 유한 bone palette18개, 움직이는 clip2개, 오류0 |
| 실제 Effect codec 저장·재로드 |80Element/2ModelCue/본 child6개 정확히 일치 |
| 기존 native PS probe 재사용, 실제 DDS4개와 원본 재질 값 |6시각×3depth gap×전후36case, nonfinite0 |
| 수정 전 zero DynamicParameter |18case 모두 alpha-positive pixel0 |
| 수정 후 원본 DynamicParameter |양수 depth gap12case 모두4096/4096 pixel에 비영 alpha, 최대alpha1 |
| depth fade 경계 |gap0의6case는 원본 계산대로 alpha0 유지 |
| scoped git diff --check |PASS |
| 현재 Product CSO 재생성·배포 |Debug Product PASS, 새 Shader_VtxAnimMeshBinary.cso 생성·배포. `out/BuildPipeline/runs/20260910T063843956Z-debug-product.json` |
| Client/UI 실행·조작·캡처와 화면 fidelity |미실행, 사용자 확인 대상 |

CModel/codec 근거는 `out/ArtistWarlordVisualFollowup20260910/LanceT/t_models_codec_probe-resume-20260910.log`, PS 수치 근거는 같은 폴더의 `dynamic_parameter_resume/out/DimensionMasterRound3_20260909/s_native_ps_probe/ps_results.csv`다. 기존 probe를 재사용했으며 새로운 영구 검증 프로젝트는 추가하지 않았다.
