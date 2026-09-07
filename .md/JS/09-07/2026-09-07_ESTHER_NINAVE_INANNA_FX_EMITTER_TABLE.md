# 2026-09-07 니나브·이난나 FX ParticleSystem 구성표

작성자: JS. 근거: `extract_ue3_particle_graph.py`로 디코드한 `FX_ESTHER_PPNN_00`, `FX_MN_PPNN_00`, `FX_ESTHER_SLINN_00` graph와
`_export_esther_ni_fx` 스테이징의 머티리얼 props/.mat. LOD0 emitter 기준. 텍스처 열은 머티리얼(또는 파라미터 없는 MI의 부모 Material)이
참조하는 텍스처 이름이며 `Effect/Esther/<Name>/Textures/<원본 패키지>/<이름>.dds`에 있다. 메시는 `Meshes/<원본 패키지>/<이름>.wmodel`.
| Esther | 패키지 | ParticleSystem | Emitter(LOD0) |
|---|---|---|---|
| Ninave | `FX_ESTHER_PPNN_00` | 51 | 477 |
| Ninave | `FX_MN_PPNN_00` | 45 | 339 |
| Inanna | `FX_ESTHER_SLINN_00` | 72 | 668 |


## Ninave — `FX_ESTHER_PPNN_00` (51 ParticleSystem)

### `par_d_ppnn_arrow`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_27 | sprite |  | fx_k_pa_light_01_01_ad | fx_e_adli, fx_f_feather_001 | acceleration, color, location, orbit, rotationrate, size, velocity, velocityoverlifetime |
| 1 | particlespriteemitter_1 | sprite |  |  |  |  |

### `par_d_ppnn_camparticle_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_2 | sprite |  | fx_d_pa_ringmaster_01_41_dt_ad | fx_f_aura_004_1, fx_m_trail_001_ycl | acceleration, color, location, orbit, rotationrate, size, velocity, velocityoverlifetime |
| 1 | particlespriteemitter_3 | mesh(override mat) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 2 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |

### `par_d_ppnn_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | arrow2-k | mesh(override mat) | fm_a_stone_001 | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 1 | electric-centre | mesh(override mat) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 2 | arrow3 | mesh(override mat) | fm_c_screwfront_001 | fx_d_me_master_01_114_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_i_atypical_03_2_xcl, fx_i_smoke_01 | color, colorscaleoverlife, lifetime, locationdirect, meshrotation, meshrotationrate, size, sizemultiplylife |
| 3 | arrow3 | mesh(override mat) | fm_a_stone_001 | fx_d_de_master_01_77_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_decal_004, fx_d_environ_001, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002, fx_f_decal_007 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 4 | arrow2 | mesh(override mat) | fm_c_screwfront_001 | fx_d_me_master_01_114_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_i_atypical_03_2_xcl, fx_i_smoke_01 | color, colorscaleoverlife, lifetime, locationdirect, meshrotation, meshrotationrate, size, sizemultiplylife |
| 5 | ghost01 | mesh(override mat) | fm_a_stone_001 | fx_d_me_master_01_110_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_114_cl, fx_d_atypical_127_xcl, fx_i_thunder_02 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 6 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 7 | arrow-aura | mesh(override mat) | fm_a_stone_001 | fx_d_pa_ribbonflow_02_01_ad | fx_d_atypical_055_2_cl, fx_d_atypical_060_cl, fx_d_electric_016, fx_d_noise_009, fx_d_normal_085, fx_k_auraline_01_ycl, fx_k_auraline_08 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 8 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 9 | particlespriteemitter_0 | efdecal |  | fx_d_pa_atta_09_17_dt_ad | fx_b_atypical_007, fx_d_atypical_027, fx_d_noise_004_1, fx_e_adba | color, colorscaleoverlife, lifetime, location, rotation, size |
| 10 | particlespriteemitter_6 | sprite (disabled) |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocity |
| 11 | arrow-head | mesh(override mat) | fm_d_rectan3cross_001 | fx_d_me_master_01_111_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_006_1, fx_d_atypical_011, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl | color, colorscaleoverlife, lifetime, locationdirect, meshrotation, meshrotationrate, size, sizemultiplylife |
| 12 | arrow2 | mesh(override mat) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |

### `par_d_ppnn_sk01_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | impact_particle | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 1 | particlespriteemitter_13 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | particlespriteemitter_11 | efdecal |  | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | color, colorscaleoverlife, lifetime, size |
| 3 | particlespriteemitter_25 | sprite |  | fx_d_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002, fx_f_decal_007 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_19 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 5 | dust | efdecal |  | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | color, colorscaleoverlife, lifetime, size |
| 6 | particlespriteemitter_18 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 7 | particlespriteemitter_26 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 8 | particlespriteemitter_21 | sprite (disabled) |  | fx_d_me_master_01_ph_02_msk | fx_a_environ_003, fx_bg_softriver_01_n, fx_d_environ_034 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 9 | rock | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | acceleration, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 10 | dust | sprite (disabled) |  | fx_d_de_simple_01_32_tr | fx_a_decal_014, fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_d_normal_016_1 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |

### `par_d_ppnn_teleport_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | arrow-head | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_20 | sprite |  | fx_d_pa_ring_07_21_tr | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_031_cl, fx_d_noise_009, fx_i_noise_03 | cameraoffset, coloroverlife, lifetime, size, sizemultiplylife |
| 2 | ghost01 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 3 | dust01 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv |
| 4 | particlespriteemitter_18 | sprite |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | ember01 | sprite (disabled) |  | fx_d_pa_glow_01_02_ad |  | color, colorscaleoverlife, lifetime, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 6 | particlespriteemitter_0 | sprite |  | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 7 | ice_twinkle | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, rotation, size, sizemultiplylife, velocity |
| 8 | particlespriteemitter_21 | sprite |  | fx_d_pa_glow_01_02_ad |  | color, colorscaleoverlife, lifetime, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 9 | ember02 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_7 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 11 | dust02 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 12 | particlespriteemitter_17 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_teleport_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_13 | sprite |  | fx_d_pa_twist_04_09_tr | fx_c_cloud_015, fx_d_atypical_006, fx_d_atypical_042_ycl, fx_d_noise_014 | cameraoffset, color, colorscaleoverlife, lifetime, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 1 | ghost01 | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |
| 2 | particlespriteemitter_3 | ribbon |  | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 3 | particlespriteemitter_1 | sprite |  | fx_d_pa_turbulence_01_06_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_004, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, eflocationprimitivecylinderspin_seeded, lifetime, rotation, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 4 | particlespriteemitter_2 | sprite (disabled) |  | fx_d_pa_twist_04_09_tr | fx_c_cloud_015, fx_d_atypical_006, fx_d_atypical_042_ycl, fx_d_noise_014 | cameraoffset, color, colorscaleoverlife, lifetime, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |

### `par_d_ppnn_teleport_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_d_ppnn_wing`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_e_screw_002 | fx_d_me_flow_02_12_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_006_1_cl | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_d_ppnn_wing_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | mesh(override mat) (disabled) | fm_d_hemisphere_001_1 | fx_d_me_master_01_038_ad | fx_c_cloud_015, fx_d_atypical_009, fx_d_atypical_039_1, fx_d_atypical_055_1_cl, fx_d_noise_009, fx_i_shockwave_02_ycl | meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |

### `par_d_skhit_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_77 | mesh(override mat) | fm_d_rectan3cross_001 | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | coloroverlife, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_22 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | color, lifetime, location, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | ring-start | sprite |  | fx_j_pa_rampshape_01_2_ad | fx_d_atypical_043, fx_e_atypical_029, fx_j_flowsmoke_01_cl |  |
| 3 | particlespriteemitter_1 | mesh(override mat) | fm_e_halfsphere_001 | bfx_d_pa_circ_01_01_dt_ad |  | color, parameterdynamic |

### `par_l_ppnn_arrow_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_27 | sprite |  | fx_d_pa_atta_09_04_tr | fx_d_noise_021, fx_e_trba, fx_f_ring_001 | location, sizemultiplylife |
| 1 | particlespriteemitter_0 | sprite |  | fx_c_pa_ring_06_ad |  | location |

### `par_l_ppnn_despawn_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_2 | sprite |  | fx_d_pa_ring_07_21_tr | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_031_cl, fx_d_noise_009, fx_i_noise_03 | location |
| 1 | ember01 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | location |
| 2 | particlespriteemitter_18 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | location |
| 3 | 22222 | sprite |  | fx_j_pa_chromaring_01_ad | fx_e_adli, fx_j_chormaticring_01 | color, location, locationprimitivesphere |
| 4 | particlespriteemitter_0 | sprite |  | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_21 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | colorscaleoverlife, location, rotation_seeded |
| 6 | particlespriteemitter_4 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | location |
| 7 | particlespriteemitter_7 | sprite |  | fx_d_pa_atta_09_04_tr | fx_d_noise_021, fx_e_trba, fx_f_ring_001 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 8 | distortion | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | location |
| 9 | particlespriteemitter_17 | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | color, location |
| 10 | particlespriteemitter_20 | sprite |  | fx_c_pa_lensflare_01_03_ad | fx_c_glow_006, fx_c_glow_007 | color, location |

### `par_l_ppnn_despawn_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_2 | sprite |  | fx_d_pa_glow_01_02_ad |  | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 1 | ghost01 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_18 | sprite |  | fx_d_pa_glow_01_02_ad |  | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 3 | ember01 | sprite |  | fx_j_pa_chromaring_01_ad | fx_e_adli, fx_j_chormaticring_01 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 4 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 5 | ice_twinkle | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 6 | particlespriteemitter_21 | sprite |  | fx_j_pa_chromaring_01_ad | fx_e_adli, fx_j_chormaticring_01 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 7 | particlespriteemitter_4 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 8 | particlespriteemitter_7 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 9 | dust01 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife, subuv |
| 10 | particlespriteemitter_17 | sprite |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 11 | ember02 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 12 | distortion | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, locationdirect, size |
| 13 | arrow-head | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife, subuv |
| 14 | particlespriteemitter_20 | sprite |  | fx_c_pa_lensflare_01_03_ad | fx_c_glow_006, fx_c_glow_007 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |

### `par_l_ppnn_esther_sk_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 1 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |
| 2 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 3 | particlespriteemitter_14 | mesh(override mat) (disabled) | fm_d_hemisphere_002 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 4 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, meshrotation, parameterdynamic, size |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, parameterdynamic, size |
| 6 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_12 | sprite |  | bfx_j_pa_lightdust_01_1_tr | fx_a_atypical_018, fx_d_fragment_003, fx_e_trex | colorscaleoverlife |
| 8 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 9 | particlespriteemitter_1 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 10 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 11 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, locationdirect, parameterdynamic, size |
| 12 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, parameterdynamic, size |
| 13 | distortion | sprite (disabled) |  | bfx_i_pa_backglow_cl_02_tr |  | coloroverlife, size |
| 14 | particlespriteemitter_6 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 15 | color | ribbon |  | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | locationemitter |
| 16 | particlespriteemitter_15 | mesh(override mat) | fm_h_lighting_01_1 | defaultparticle |  | coloroverlife, locationemitter, parameterdynamic, size |
| 17 | 111 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | locationdirect |
| 18 | particlespriteemitter_17 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 19 | line0 | sprite |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | size |

### `par_l_ppnn_esther_sk_01_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | dust | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, efvelocityoverlifetime, size |
| 1 | particlespriteemitter_14 | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | color, colorscaleoverlife, efvelocityoverlifetime, size |
| 2 | 22222 | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | color, colorscaleoverlife, rotation_seeded |
| 3 | 22222 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | colorscaleoverlife, rotation_seeded |
| 4 | ring-start | sprite |  | fx_j_circleshine_01_1_ad | fx_d_atypical_011 | colorscaleoverlife, sizemultiplylife |
| 5 | particlespriteemitter_19 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | colorscaleoverlife, sizemultiplylife |
| 6 | particlespriteemitter_20 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife |
| 7 | particlespriteemitter_21 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 8 | particlespriteemitter_22 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 9 | particlespriteemitter_23 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | size |
| 10 | line0 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | size |
| 11 | line0 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 |  |
| 12 | particlespriteemitter_26 | sprite (disabled) |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 13 | particlespriteemitter_7 | mesh(override mat) (disabled) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 14 | distortion | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | colorscaleoverlife |
| 15 | particlespriteemitter_6 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | colorscaleoverlife |
| 16 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |

### `par_l_ppnn_esther_sk_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 1 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |
| 2 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 3 | particlespriteemitter_19 | mesh(override mat) (disabled) | fm_d_hemisphere_002 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 4 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, meshrotation, parameterdynamic, size |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, parameterdynamic, size |
| 6 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 8 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 9 | particlespriteemitter_1 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 10 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 11 | particlespriteemitter_20 | mesh(override mat) | fm_h_lighting_01_1 | defaultparticle |  | coloroverlife, locationemitter, parameterdynamic, size |
| 12 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, parameterdynamic, size |
| 13 | distortion | sprite (disabled) |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 14 | particlespriteemitter_6 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 15 | color | ribbon |  | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | locationemitter |
| 16 | line0 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | size |
| 17 | 111 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | locationdirect |
| 18 | particlespriteemitter_22 | sprite |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 19 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, locationdirect, parameterdynamic, size |

### `par_l_ppnn_esther_sk_02_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 1 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 2 | line0 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | size |
| 3 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 4 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 6 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 7 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 8 | m | mesh(override mat) | fm_d_sphere_001 | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 9 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 11 | particlespriteemitter_11 | mesh(override mat) | fm_d_hemisphere_002 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 12 | particlespriteemitter_1 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 13 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 14 | particlespriteemitter_23 | mesh(override mat) (disabled) | fm_h_lighting_01_1 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 15 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 16 | distortion | sprite (disabled) |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 17 | particlespriteemitter_6 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 18 | color | ribbon |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | locationemitter |
| 19 | particlespriteemitter_24 | mesh(override mat) | fm_d_sphere_001 | defaultparticle |  | coloroverlife, locationemitter, meshrotation, parameterdynamic, size |
| 20 | 111 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | locationdirect |
| 21 | particlespriteemitter_26 | sprite |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 22 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, location, locationdirect, meshrotation, parameterdynamic, size |

### `par_l_ppnn_esther_sk_02_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | m | mesh(override mat) | fm_d_sphere_001 | defaultparticle |  | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 1 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 2 | line0 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | size |
| 3 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 4 | particlespriteemitter_27 | mesh(override mat) (disabled) | fm_d_hemisphere_002 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 5 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 6 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 7 | 111 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | locationdirect |
| 8 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 9 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 10 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 11 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 12 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 13 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 14 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 15 | particlespriteemitter_1 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 16 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 17 | m | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 18 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 19 | distortion | sprite (disabled) |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 20 | particlespriteemitter_6 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 21 | color | ribbon |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | locationemitter |
| 22 | particlespriteemitter_28 | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, locationemitter, meshrotation, parameterdynamic, size |
| 23 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, location, locationdirect, parameterdynamic, size |
| 24 | particlespriteemitter_32 | sprite |  | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 25 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |

### `par_l_ppnn_esther_sk_02_3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 1 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, locationdirect, parameterdynamic, size |
| 2 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 3 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 4 | m | mesh(override mat) | fm_d_sphere_001 | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 6 | line0 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | size |
| 7 | particlespriteemitter_38 | mesh(override mat) (disabled) | fm_d_hemisphere_002 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 8 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 9 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 10 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 11 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 12 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 13 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 14 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 15 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 16 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 17 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 18 | particlespriteemitter_1 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 19 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 20 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 21 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, parameterdynamic, size |
| 22 | 111 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | locationdirect |
| 23 | particlespriteemitter_6 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 24 | color | ribbon |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | locationemitter |
| 25 | particlespriteemitter_39 | mesh(override mat) | fm_d_sphere_001 | defaultparticle |  | coloroverlife, locationemitter, meshrotation, parameterdynamic, size |
| 26 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |
| 27 | particlespriteemitter_41 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 28 | distortion | sprite (disabled) |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |

### `par_l_ppnn_esther_sk_02_4`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 1 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 2 | particlespriteemitter_4 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 |  |
| 3 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, parameterdynamic, size |
| 4 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 6 | m | mesh(override mat) | fm_d_hemisphere_001_1 | fx_k_pa_shine_01_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 7 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 8 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 9 | line0 | sprite |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | size |
| 10 | particlespriteemitter_48 | mesh(override mat) (disabled) | fm_d_hemisphere_002 | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | colorscaleoverlife, sizemultiplylife |
| 11 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 12 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 13 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 14 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 15 | m | mesh(override mat) | fm_d_sphere_001 | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 16 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 17 | m | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 18 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, location, size, sizemultiplylife, velocity |
| 19 | particlespriteemitter_10 | mesh(override mat) | fm_c_screwfront_001 | bfx_j_pa_lightdust_01_1_tr | fx_a_atypical_018, fx_d_fragment_003, fx_e_trex | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 20 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 21 | particlespriteemitter_1 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, size |
| 22 | particlespriteemitter_10 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, locationprimitivesphere, size, sizemultiplylife, velocity |
| 23 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, location, locationdirect, parameterdynamic, size |
| 24 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, parameterdynamic, size |
| 25 | m | mesh(override mat) | fm_d_sphere_001 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, colorscaleoverlife, location, meshrotation, parameterdynamic, size |
| 26 | 111 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | locationdirect |
| 27 | color | ribbon |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | locationemitter |
| 28 | particlespriteemitter_49 | mesh(override mat) | fm_d_sphere_001 | defaultparticle |  | coloroverlife, locationemitter, meshrotation, parameterdynamic, size |
| 29 | distortion | sprite (disabled) |  | fx_d_me_master_01_111_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_006_1, fx_d_atypical_011, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl | coloroverlife, size |
| 30 | particlespriteemitter_51 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |
| 31 | particlespriteemitter_6 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |

### `par_l_ppnn_esther_sk_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring-start | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 1 | arrow-aura | mesh(override mat) | fm_a_sprial_008 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 2 | dust | sprite |  | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv, velocity |
| 3 | electric-centre | sprite |  | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv, velocity |
| 4 | arrow3 | mesh(override mat) | fm_a_sprial_008 | bfx_c_pa_lightflare_01_ddt_4_ad | fx_c_glow_009, fx_c_glow_010 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_0 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | location |

### `par_l_ppnn_esther_sk_03_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | location |
| 1 | particlespriteemitter_16 | mesh(override mat) | fm_d_hemisphere_008_1 | fx_d_me_ringmaster_01_14_ad | fx_a_noise_008_n, fx_e_fire_005_1, fx_f_aura_004_1 | coloroverlife, location, parameterdynamic, size |
| 2 | particlespriteemitter_0 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, size |
| 3 | particlespriteemitter_1 | sprite |  | fx_b_me_energy_01_tr | fx_b_atypical_004, fx_c_trail_001_cl | location |
| 4 | particlespriteemitter_17 | mesh(override mat) | fm_d_helix_002 | fx_d_me_flow_02_28_tr | fx_b_atypical_004, fx_c_trail_005_cl, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl | color, colorscaleoverlife, location, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_18 | mesh(override mat) | fm_d_sphere_001 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, location, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 6 | ring-start | sprite |  | fx_c_pa_lensflare_01_03_ad | fx_c_glow_006, fx_c_glow_007 | location |

### `par_l_ppnn_esther_sk_03_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | line0 | sprite |  | fx_l_pa_trail_09_tr | fx_d_atypical_035_cl, fx_d_electric_013, fx_d_noise_030, fx_k_electric_01 | size |
| 1 | particlespriteemitter_4 | sprite |  | fx_k_maskedrib_01_05_tr | fx_a_noise_008_n, fx_b_atypical_004, fx_i_atypical_03_ycl |  |
| 2 | particlespriteemitter_0 | sprite |  | fx_m_me_spritewave_01_3_tr | fx_a_line_005, fx_d_atypical_006_ycl, fx_d_noise_009, fx_d_noise_030, fx_m_caustic_001 | color |
| 3 | p1 | ribbon |  | fx_k_pa_fd_01_05_tr | fx_e_noise_008, fx_e_trdi, fx_m_spatter_001_xyclamp | colorscaleoverlife, locationemitter |
| 4 | line0 | sprite |  | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | size |
| 5 | m | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | coloroverlife, location, meshrotation, parameterdynamic, size |
| 6 | particlespriteemitter_9 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, location, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_1 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, size |
| 8 | particlespriteemitter_11 | mesh(override mat) | fm_d_sphere_001 | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | colorscaleoverlife, sizemultiplylife |
| 9 | t | mesh(override mat) | fm_d_sphere_001 | fx_l_me_transition_05_8_ma | fx_c_atypical_016, fx_e_symbol_007, fx_l_arrow_normal_01, fx_l_arrow_spec_01, wp_wgdh_01s_d, wp_wgdh_01s_n, wp_wgdh_01s_s | coloroverlife, colorscaleoverlife, locationdirect, meshrotation, parameterdynamic, size |
| 10 | m | mesh(override mat) | fm_d_hemisphere_002 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, meshrotation, parameterdynamic, size |
| 11 | distortion | sprite (disabled) |  | bfx_h_pa_circle_01_02_ad |  | coloroverlife, sizemultiplylife |
| 12 | m | mesh(override mat) | fm_h_lighting_01_1 | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | coloroverlife, location, meshrotation, parameterdynamic, size |
| 13 | color | ribbon |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | locationemitter |

### `par_l_ppnn_esther_sk_03_3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_14 | mesh(override mat) | fm_h_lighting_01_1 | fx_k_pa_glow_01_ad_dt |  | coloroverlife, size |
| 1 | particlespriteemitter_19 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 |  |
| 2 | particlespriteemitter_20 | mesh(override mat) (disabled) | fm_a_hemisphere_012 | fx_a_pa_cd_01_2_tr | fx_a_environ_001, fx_e_trba | colorscaleoverlife, sizemultiplylife |
| 3 | impact_particle | sprite |  | fx_i_pa_thunder_02_ad | fx_a_noise_014, fx_a_noise_018, fx_d_noise_009, fx_i_environment_001, fx_i_shockwave_02_ycl, fx_i_thunder_02, fx_i_watercaustic_001 | color, locationprimitivesphere, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_1 | mesh(override mat) | fm_h_halfsphere_01_1 | fx_d_pa_atta_09_02_ad | fx_d_atypical_034_cl, fx_e_adba | coloroverlife, parameterdynamic, size |
| 5 | impact_particle | sprite (disabled) |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | coloroverlife, velocity, velocityoverlifetime |
| 6 | particlespriteemitter_18 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | colorscaleoverlife |
| 7 | particlespriteemitter_12 | sprite |  | fx_d_de_simple_01_12_tr | fx_a_decal_014, fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_d_normal_016_1 | color, colorscaleoverlife, sizemultiplylife |
| 8 | start | sprite |  | fx_d_me_master_01_111_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_006_1, fx_d_atypical_011, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl | coloroverlife |
| 9 | particlespriteemitter_15 | sprite |  | fx_a_pa_firework_01_ad | fx_a_atypical_009, fx_a_fire_006 | color |
| 10 | particlespriteemitter_8 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | coloroverlife, location, size, velocity |
| 11 | dust_smoke_02 | sprite |  | fx_e_pa_ht_18_1_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | coloroverlife, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocity |
| 12 | under_crash_01 | efparticlemodulevelocityoverlifetime |  | fx_h_me_fd_01_1_ts_tr | fx_bg_softriver_02_n, fx_c_noise_001, fx_c_ring_004_cl, fx_e_atypical_006, fx_e_electric_002 | coloroverlife |
| 13 | particlespriteemitter_10 | mesh(override mat) | fm_c_screwfront_001 | bfx_j_pa_lightdust_01_1_tr | fx_a_atypical_018, fx_d_fragment_003, fx_e_trex | color, location, parameterdynamic |
| 14 | particlespriteemitter_11 | sprite |  | bfx_i_pa_thunder_02_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, parameterdynamic, sizemultiplylife |
| 15 | particlespriteemitter_17 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | coloroverlife, size |

### `par_l_ppnn_esther_sk_03_4`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring-start | sprite |  | bfx_i_pa_glow_01_ad |  | location |
| 1 | particlespriteemitter_35 | sprite (disabled) |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, locationprimitivesphere, size, velocity |
| 2 | particlespriteemitter_34 | sprite |  | bfx_i_pa_thunder_02_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, location |
| 3 | shine_loop | sprite |  | fx_d_pa_atta_05_20e_tr | fx_b_atypical_026, fx_e_trli | color, colorscaleoverlife, location, size, sizemultiplylife |
| 4 | ggg | sprite |  | fx_d_pa_shine_01_04_dt_ad | fx_d_atypical_028, fx_d_noise_009, fx_e_noise_003 | color, colorscaleoverlife, location, sizemultiplylife |
| 5 | shockwave_small | mesh(override mat) | fm_d_cylinder1500_001 | bfx_i_pa_glow_02_ad |  | coloroverlife, location, parameterdynamic, size, sizemultiplylife |
| 6 | particlespriteemitter_30 | mesh(override mat) (disabled) | fm_d_cylinder1500_001 | fx_m_me_trail_02_18_tr | fx_a_cloud_022, fx_d_noise_030, fx_j_mirnoise_02, fx_m_atypical_004_loc_int, fx_m_flow_04_n, fx_m_line_001, fx_m_trail_002 | coloroverlife, location, orbit, parameterdynamic, size, sizemultiplylife |
| 7 | particlespriteemitter_31 | mesh(override mat) | fm_d_hemisphere_001_1 | fx_a_pa_ht_01_1_ad | fx_a_fragment_002, fx_e_adsi, fx_e_normal | coloroverlife, location, parameterdynamic, size, sizemultiplylife |
| 8 | 1 | sprite |  | fx_j_pa_lbeam_01_1_ad |  | color, lifetime_seeded, location, velocity_seeded |
| 9 | glow03 | sprite (disabled) |  | fx_h_pa_shine_01_ad | fx_d_noise_030 | colorscaleoverlife, location, size, sizemultiplylife |
| 10 | arrow-aura | mesh(override mat) | fm_d_rectan3cross_001 | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, colorscaleoverlife, location, parameterdynamic, size, sizemultiplylife |
| 11 | dust | sprite (disabled) |  | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | color, efvelocityoverlifetime, location, velocity |
| 12 | particlespriteemitter_20 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife, location, size, sizemultiplylife |
| 13 | particlespriteemitter_24 | sprite |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | coloroverlife, efvelocityoverlifetime, location, sizemultiplylife, velocity_seeded |
| 14 | glow03 | sprite (disabled) |  | bfx_i_pa_glow_01_ad |  | color, colorscaleoverlife, location, sizemultiplylife |
| 15 | arrow3 | mesh(override mat) | fm_d_rectan3cross_001 | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | color, colorscaleoverlife, location, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 16 | arrow-head | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | eflocationcirclesurface |
| 17 | glow03 | sprite |  | fx_m_me_trail_02_18_tr | fx_a_cloud_022, fx_d_noise_030, fx_j_mirnoise_02, fx_m_atypical_004_loc_int, fx_m_flow_04_n, fx_m_line_001, fx_m_trail_002 | color, colorscaleoverlife, location, parameterdynamic, size, sizemultiplylife |
| 18 | ggg | sprite |  | fx_m_pa_worldoffset_02_28_tr | fx_a_blankwhite_01, fx_d_noise_002, fx_j_mirnoise_01, fx_k_caustictile_01, fx_k_fluidtile_01, fx_m_flow_04_n, fx_m_noise_001, fx_m_spatter_001_xyclamp | color, colorscaleoverlife, efvelocityoverlifetime, location, orbit, size, sizemultiplylife, velocity |
| 19 | electric-centre | sprite |  | fx_d_me_master_01_038_ad | fx_c_cloud_015, fx_d_atypical_009, fx_d_atypical_039_1, fx_d_atypical_055_1_cl, fx_d_noise_009, fx_i_shockwave_02_ycl | color, colorscaleoverlife, location, size, sizemultiplylife, velocity |

### `par_l_ppnn_spawn_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_20 | sprite |  | fx_d_pa_ring_07_21_tr | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_031_cl, fx_d_noise_009, fx_i_noise_03 |  |
| 1 | ghost01 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 |  |
| 2 | dust01 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | color, eflocationcirclesurface |
| 3 | particlespriteemitter_18 | sprite |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi |  |
| 4 | ember01 | sprite (disabled) |  | fx_d_pa_glow_01_02_ad |  | color, locationprimitivecylinder |
| 5 | ghost01 | sprite |  | fx_o_me_master_01_10_ds_ad | fx_a_atypical_048, fx_c_cloud_015, fx_i_shockwave_02_ycl | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 6 | ice_twinkle | sprite |  | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | color, eflocationcirclesurface |
| 7 | particlespriteemitter_21 | sprite |  | fx_d_pa_glow_01_02_ad |  | color, locationprimitivecylinder |
| 8 | ember02 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, locationprimitivecylinder |
| 9 | particlespriteemitter_7 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 |  |
| 10 | dust02 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | eflocationcirclesurface |
| 11 | particlespriteemitter_17 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 |  |

### `par_l_ppnn_spawn_trail_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | black | sprite |  | fx_o_me_master_01_10_ds_ad | fx_a_atypical_048, fx_c_cloud_015, fx_i_shockwave_02_ycl | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |

### `par_o_battle_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | 1 | sprite |  | fx_o_pa_screwflow_01_10_tr | fx_a_line_011, fx_a_noise_003, fx_c_flow_003 | lifetime, size |
| 1 | ice_twinkle | sprite |  | fx_n_pa_shine_01_01_ad |  | color, colorscaleoverlife, eflocationemitter, lifetime, location, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_13 | sprite |  | fx_o_pa_screwflow_01_10_tr | fx_a_line_011, fx_a_noise_003, fx_c_flow_003 | lifetime, size |
| 3 | particlespriteemitter_39 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, color, colorscaleoverlife, eflocationemitter, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | shine_loop | sprite |  | bfx_d_pa_circ_01_02_ad |  | cameraoffset, color, colorscaleoverlife, eflocationemitter, lifetime, rotation, size, sizemultiplylife |
| 5 | glow | sprite |  | fx_n_pa_shine_01_01_ad |  | color, colorscaleoverlife, eflocationemitter, lifetime, location, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_5 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | particlespriteemitter_11 | sprite |  | fx_e_pa_gl_07_1_ad |  | color, colorscaleoverlife, lifetime_seeded, location, parameterdynamic, rotation, size_seeded, sizemultiplylife, velocity |
| 8 | small-sonicboom | mesh(override mat) | fm_o_trail_02 | fx_j_me_streaming_001_4_tr | fx_a_noise_008_n, fx_d_trail_002_cl | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 9 | particlespriteemitter_30 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, coloroverlife, colorscaleoverlife, eflocationemitter, lifetime, rotation, size, sizemultiplylife |
| 10 | particlespriteemitter_49 | sprite |  | bfx_d_pa_circ_01_02_ad |  | cameraoffset, color, colorscaleoverlife, eflocationemitter, lifetime, rotation, size, sizemultiplylife |
| 11 | glow | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, coloroverlife, colorscaleoverlife, eflocationemitter, lifetime, rotation, size, sizemultiplylife |

### `par_o_battle_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_12 | sprite |  | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | color, colorscaleoverlife, lifetime, locationprimitivesphere, size, sizemultiplylife |
| 1 | ring_in | sprite |  | fx_c_pa_lensflare_01_11_dt5_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, rotationrate, size |
| 2 | particlespriteemitter_1 | sprite |  | defaultparticle |  | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, rotation, size, sizemultiplylife, subuv, velocity |
| 3 | particlespriteemitter_21 | sprite |  | bfx_d_pa_circ_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_24 | sprite |  | fx_n_pa_shine_01_01_ad |  | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife, velocity |
| 5 | particlespriteemitter_5 | sprite |  | defaultparticle |  | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, rotation, size, sizemultiplylife, subuv, velocity |

### `par_o_battle_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | 1 | sprite |  | fx_d_pa_atta_05_14_ad | fx_a_ring_001_cl, fx_e_adli | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, rotation, size, sizemultiplylife, subuv, velocity |
| 1 | particlespriteemitter_0 | sprite |  | fx_d_pa_atta_05_14_ad | fx_a_ring_001_cl, fx_e_adli | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_3 | sprite |  | fx_d_pa_atta_05_14_ad | fx_a_ring_001_cl, fx_e_adli | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |

### `par_o_battle_04`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring_in | mesh(override mat) | fm_d_cracklight_003 | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_26 | sprite (disabled) |  | fx_d_me_master_01_111_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_006_1, fx_d_atypical_011, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, size, sizemultiplylife, velocity |
| 2 | particlespriteemitter_25 | mesh(override mat) | fm_c_planecross_001 | fx_e_pa_gl_07_1_ad |  | color, colorscaleoverlife, lifetime_seeded, locationdirect, orbit, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 3 | after_wind | mesh(override mat) | fm_c_screwfront_001 | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_7 | mesh(override mat) (disabled) | fm_c_screwfront_001 | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 5 | arrow-aura | mesh(override mat) | fm_d_cracklight_003 | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 6 | particlespriteemitter_1 | mesh(override mat) | fm_d_helix_015_1 | bfx_o_me_shine_02_ad |  | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 7 | arrow-aura | mesh(override mat) | fm_d_helix_003 | bfx_o_me_shine_02_ad |  | color, colorscaleoverlife, lifetime_seeded, locationdirect, orbit, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 8 | particlespriteemitter_10 | sprite |  | fx_c_pa_lensflare_01_09_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |

### `par_o_battle_05`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_6 | sprite |  | fx_o_me_makeflow_02_22_tr | fx_d_atypical_009, fx_d_noise_002, fx_e_noise_001, fx_i_atypical_03_1, fx_l_environment_001, fx_m_caustic_001, fx_m_wave_001, fx_m_wave_001_ycl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_11 | sprite |  | fx_o_pa_beam_01_04_tr | fx_a_noise_008_n, fx_j_mirnoise_01, fx_j_ylinestream_01, fx_k_auraline_16, fx_k_caustictile_01 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, size |
| 2 | shine_loop | sprite |  | fx_k_pa_beam_01_02_tr | fx_a_noise_008_n, fx_j_mirnoise_01, fx_j_ylinestream_01, fx_k_auraline_16, fx_k_caustictile_01 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 3 | shine_loop | sprite |  | fx_o_pa_ap_23_2_tr | fx_d_atypical_028, fx_d_atypical_060_cl | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 4 | particlespriteemitter_14 | sprite |  | fx_d_pa_atta_05_03_ad | fx_e_adli, fx_e_hit_007_1 | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife, velocity |
| 5 | emit_loc | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife, velocity |
| 6 | ring-start | sprite |  | fx_d_me_master_01_032_ts_tr | fx_a_trail_005_cl, fx_c_cloud_015, fx_d_atypical_009, fx_d_atypical_042_1, fx_d_hit_006_1_cl | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 7 | helix | sprite |  | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 8 | ice_twinkle | sprite |  | fx_o_me_makeflow_02_22_tr | fx_d_atypical_009, fx_d_noise_002, fx_e_noise_001, fx_i_atypical_03_1, fx_l_environment_001, fx_m_caustic_001, fx_m_wave_001, fx_m_wave_001_ycl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 9 | smoke_tail | mesh(override mat) | fm_b_halfcylinder_001 | bfx_d_pa_smoke_ulit_01_05_tr | fx_d_cloud_033, fx_d_noise_021, fx_d_noise_031 | color_seeded, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife |

### `par_o_battle_06`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | smoke_up | sprite (disabled) |  | bfx_d_pa_smoke_ulit_01_05_tr | fx_d_cloud_033, fx_d_noise_021, fx_d_noise_031 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 1 | 1_head | mesh(override mat) | fm_d_cracklight_003 | fx_o_me_ring_05_03_ts_tr | fx_a_blankwhite_01, fx_d_atypical_002, fx_d_atypical_013, fx_d_noise_005, fx_d_noise_014 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | smoke_up | sprite (disabled) |  | fx_o_me_flowtrail_01_14_tr_ts | fx_c_line_004_ycl, fx_d_atypical_011, fx_d_atypical_028, fx_i_shockwave_02, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_2 | mesh(override mat) | fm_d_cracklight_003 | fx_o_me_ring_05_6_ts_tr | fx_d_atypical_002, fx_d_hit_007, fx_d_hit_007_1, fx_d_noise_014, fx_e_noise_012 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_o_battle_07`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | black | sprite |  | fx_e_pa_ht_01_1_tr | fx_e_atypical_011, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_5 | mesh(override mat) | fm_d_helix_015_1 | fx_k_pa_backglow_cl_02_tr_dt |  | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | particlespriteemitter_2 | mesh(override mat) | fm_d_cracklight_003 | fx_o_me_ring_05_03_ts_tr | fx_a_blankwhite_01, fx_d_atypical_002, fx_d_atypical_013, fx_d_noise_005, fx_d_noise_014 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 3 | 1_head | mesh(override mat) | fm_c_square_001 | fx_s_me_ring_01_2_ts_tr | fx_d_atypical_002, fx_d_atypical_034_1, fx_d_hit_007, fx_d_noise_014, fx_j_mirnoise_01 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, parameterdynamic, size, sizemultiplylife, velocity |
| 4 | 1_head | mesh(override mat) | fm_c_square_001 | fx_s_me_ring_01_2_ts_tr | fx_d_atypical_002, fx_d_atypical_034_1, fx_d_hit_007, fx_d_noise_014, fx_j_mirnoise_01 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, parameterdynamic, size, velocity |
| 5 | 1_head | mesh(override mat) | fm_c_square_001 | fx_o_me_ring_05_02_ts_tr | fx_d_atypical_002, fx_d_hit_007, fx_d_hit_007_1, fx_d_noise_004_1, fx_d_noise_014 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, parameterdynamic, size, velocity |
| 6 | 1_head | sprite |  | fx_o_me_ringmaster_01_10_ad | fx_d_atypical_076_1_cl, fx_f_aura_004_1 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size, velocity |
| 7 | 1_head | mesh(override mat) | fm_d_cracklight_003 | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, meshrotation, parameterdynamic, size, velocity |
| 8 | 1_head | sprite |  | fx_d_me_flow_02_01_ad | fx_a_noise_017, fx_b_atypical_004, fx_d_atypical_035_1_cl, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl | coloroverlife, lifetime, location, locationprimitivecylinder, orbit, rotation, size, sizemultiplylife, subuv, velocity |
| 9 | spark | sprite (disabled) |  | fx_d_me_flow_02_01_ad | fx_a_noise_017, fx_b_atypical_004, fx_d_atypical_035_1_cl, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl | coloroverlife, lifetime, location, locationprimitivecylinder, orbit, rotation, size, sizemultiplylife, subuv, velocity |
| 10 | particlespriteemitter_3 | mesh(override mat) | fm_c_square_001 | fx_d_me_flow_02_01_ad | fx_a_noise_017, fx_b_atypical_004, fx_d_atypical_035_1_cl, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 11 | 1_head | sprite |  | fx_o_me_ring_05_6_ts_tr | fx_d_atypical_002, fx_d_hit_007, fx_d_hit_007_1, fx_d_noise_014, fx_e_noise_012 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size, velocity |

### `par_o_battle_08`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_38 | sprite |  | fx_a_pa_ht_01_2_ad | fx_a_fragment_002, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_39 | sprite |  | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | coloroverlife, lifetime, location, locationprimitivesphere, size, sizemultiplylife, velocityoverlifetime |
| 2 | particlespriteemitter_40 | sprite |  | fx_o_pa_ri_04_ad_2s | fx_e_adba, fx_e_atypical_031 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, rotation, size |
| 3 | particlespriteemitter_41 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_42 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_43 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | ice_twinkle | sprite |  | fx_d_pa_flare_03_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 7 | note | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 8 | dust02 | sprite (disabled) |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |

### `par_o_battle_09`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ggg | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | dust01 | sprite (disabled) |  | fx_c_pa_lensflare_01_09_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv |

### `par_o_battle_10`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | sprite |  | fx_o_pa_sy_09_1_ts_tr | fx_e_normal, fx_e_trsi, fx_o_symbol_01 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | particlespriteemitter_2 | sprite |  | fx_e_pa_gl_03_1_ad | fx_a_noise_002, fx_a_ring_001_cl, fx_b_glow_001_cl | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 2 | dist_concave | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | coloroverlife, lifetime, orientationaxislock, parameterdynamic, size, sizemultiplylife |

### `par_o_battle_11`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_18 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, orientationaxislock, rotation, size, sizemultiplylife |
| 1 | ring-start | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, orientationaxislock, size, sizemultiplylife |
| 2 | particlespriteemitter_23 | sprite (disabled) |  | bfx_d_pa_smoke_ulit_01_05_tr | fx_d_cloud_033, fx_d_noise_021, fx_d_noise_031 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocity |
| 3 | smoke_up | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity |

### `par_o_battle_12`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_5 | mesh(override mat) | fm_d_helix_015_1 | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | cameraoffset, color, colorscaleoverlife, lifetime_seeded, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 1 | line | mesh(override mat) | fm_d_helix_015_1 | fx_d_pa_ribbonflow_02_02_ad | fx_d_atypical_055_2_cl, fx_d_electric_016, fx_d_noise_009, fx_k_auraline_08, fx_k_auraline_13, fx_k_auraline_14 | cameraoffset, color, colorscaleoverlife, lifetime_seeded, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 2 | ring-start | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 3 | particlespriteemitter_4 | sprite |  | fx_b_pa_cd_02_tr | fx_a_atypical_002_cl, fx_b_atypical_006, fx_b_cloud_016, fx_b_cloud_019 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, rotation, size, sizemultiplylife, subuv, velocity |

### `par_o_battle_13`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | arrow-head | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | after_wind | mesh(override mat) | fm_d_rectan3cross_001 | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size_seeded, sizemultiplylife, velocity |
| 2 | particlespriteemitter_12 | sprite |  | bfx_i_pa_glow_01_ad |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 3 | ggg | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 4 | ice_twinkle | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_16 | sprite (disabled) |  | bfx_d_pa_circ_01_02_ad |  | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, size, sizemultiplylife, subuv |
| 6 | particlespriteemitter_9 | sprite |  | fx_j_me_ringrainbow_01_4_ts_tr | fx_c_cloud_015, fx_d_atypical_042_1, fx_d_atypical_055_2_cl, fx_d_noise_009, fx_e_line_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |

### `par_o_ppnn_teleport_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_20 | sprite |  | fx_d_pa_ring_07_21_tr | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_031_cl, fx_d_noise_009, fx_i_noise_03 | cameraoffset, coloroverlife, lifetime, size, sizemultiplylife |
| 1 | ghost01 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 2 | dust01 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, eflocationprimitivecylinderspin, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv |
| 3 | particlespriteemitter_18 | sprite |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | ember01 | sprite (disabled) |  | fx_d_pa_glow_01_02_ad |  | color, colorscaleoverlife, lifetime, locationprimitivecylinder_seeded, orbit, particlespriteemitter, size, sizemultiplylife, subuv, velocity |
| 5 | ee | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |
| 6 | ice_twinkle | sprite |  | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, eflocationcirclesurface, lifetime, rotation, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_21 | sprite |  | fx_d_pa_glow_01_02_ad |  | color, colorscaleoverlife, lifetime, locationprimitivecylinder_seeded, orbit, particlespriteemitter, size, sizemultiplylife, subuv, velocity |
| 8 | ember02 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 9 | particlespriteemitter_7 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 10 | dust02 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 11 | particlespriteemitter_17 | sprite |  | fx_k_pa_turbulence_01_tr | fx_c_atypical_017, fx_c_noise_008, fx_d_atypical_082, fx_d_fluid_026, fx_d_uvturbulence_001, fx_e_atypical_005_cl, fx_e_noise_002, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |

### `par_o_ppnn_wing_loop_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_o_ppnn_wing_loop_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_d_wing_002 | fx_m_pa_master_01_01_tr | fx_c_cloud_015, fx_d_atypical_082, fx_f_feather_001, fx_l_spiderline_01 | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_o_ppnn_wing_start_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | blood_particle_short | sprite |  | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | loc | mesh(override mat) | fm_d_helix_018 | fx_m_pa_master_01_01_tr | fx_c_cloud_015, fx_d_atypical_082, fx_f_feather_001, fx_l_spiderline_01 | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |
| 2 | ee | mesh(override mat) | fm_d_helix_018 | fx_m_pa_master_01_01_tr | fx_c_cloud_015, fx_d_atypical_082, fx_f_feather_001, fx_l_spiderline_01 | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_o_ppnn_wing_start_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | blood_particle_short | sprite |  | fx_m_pa_skull_02_23_tr | fx_d_noise_030, fx_e_ring_001_cl, fx_m_noise_001, fx_m_skull_002_loc_int | cameraoffset, coloroverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, orbit, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 1 | loc | sprite |  | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_22 | sprite |  | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |

### `par_v_ppnn_arrow`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_27 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, colorscaleoverlife, lifetime, location, size |
| 1 | ggg | sprite |  | bfx_d_pa_circ_01_02_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |

### `par_v_ppnn_hand_cast_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_14 | sprite |  | fx_n_me_spritewave_01_22_tr | fx_a_line_005, fx_d_noise_030, fx_h_noise_001, fx_k_caustictile_01, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_008 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | particlespriteemitter_0 | sprite |  | fx_k_pa_makeflow_02_08_tr | fx_d_atypical_006, fx_d_atypical_031_cl, fx_d_noise_002, fx_i_atypical_03_1, fx_l_environment_001 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orbit, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_22 | mesh(override mat) | fm_d_helix_015_1 | fx_n_pa_spritewave_03_4_tr | fx_a_line_005, fx_d_atypical_028, fx_d_noise_030, fx_k_electile_02, fx_m_trail_004_cl | coloroverlife, colorscaleoverlife, lifetime, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 3 | card_01 | sprite |  | fx_o_pa_shorkwave_01_17_tr | fx_a_environ_003_n, fx_d_noise_003, fx_f_electric_005, fx_i_noise_03, fx_m_atypical_002_ycl, fx_m_atypical_n_001 | coloroverlife, lifetime, location, parameterdynamic, rotation_seeded, size |
| 4 | particlespriteemitter_31 | sprite |  | fx_k_pa_ring_05_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_j_mirnoise_01, fx_m_wave_001 | color, colorscaleoverlife, eflocationemitter, lifetime, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 5 | 2 | sprite |  | fx_k_pa_ht_12_01_tr | fx_e_trdi, fx_k_hit_01 | color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_48 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |

### `par_v_ppnn_hit_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | glow | sprite |  | fx_k_pa_fd_18_01_tr | fx_a_decal_013, fx_bg_softriver_02_n, fx_e_atypical_024_1_xcl, fx_e_electric_002_cl, fx_e_fluid_003, fx_e_fluid_021, fx_e_fluid_021_n, fx_e_noise_008 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_1 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, size |
| 2 | glow | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | coloroverlife, eftypedatadecal, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 3 | 22222 | sprite |  | fx_j_circleshine_01_1_ad | fx_d_atypical_011 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotationrate, size |
| 4 | distortion | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size |
| 5 | particlespriteemitter_16 | sprite |  | fx_c_pa_flickline_01_1_ad | fx_f_star_001 | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | 22222 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | particlespriteemitter_7 | mesh(override mat) | fm_a_hemisphere_012 | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 8 | helix3 | sprite |  | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife, subuv, velocity |
| 9 | impact_particle | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |

### `par_v_ppnn_tone_shot_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | wave_gr | sprite |  | fx_n_pa_shine_01_01_ad |  | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, rotation, rotationrate_seeded, size, sizemultiplylife, typedatamesh, velocity |
| 1 | pc_smoke | mesh(override mat) | fm_a_hemisphere_012 | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 2 | helix2 | mesh(override mat) | fm_a_hemisphere_012 | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 3 | after_wind | sprite |  | fx_k_me_makeflow_01_02_tr | fx_b_atypical_004, fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01, fx_k_auraline_04, fx_m_noise_001 | color, colorscaleoverlife, location, parameterdynamic, rotation, sizemultiplylife, velocity |
| 4 | after_wind1 | sprite |  | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | color, colorscaleoverlife, efvortex, lifetime, location, locationprimitivesphere, rotation, rotationrate_seeded, size, sizemultiplylife, typedatamesh, velocity_seeded |
| 5 | after_wind4_dark | mesh(override mat) | fm_k_tornado_01 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 6 | smoke_tail | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 7 | after_wind3 | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |
| 8 | helix4 | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 9 | after_wind2_dark | mesh(override mat) | fm_d_helix_015_1 | fx_c_pa_dist_05_ad | fx_a_noise_002, fx_a_ring_001_cl, fx_b_glow_001_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 10 | spark | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 11 | helix1 | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 12 | after_wind3_dark | mesh(override mat) | fm_b_sphere_001 | fx_k_me_makeflow_02_09_tr | fx_d_noise_002, fx_i_atypical_03_1, fx_j_mirnoise_01, fx_k_auraline_02, fx_l_environment_001, fx_m_caustic_001, fx_m_wave_001 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 13 | after_wind1_dark | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 14 | smoke_tail | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, required, size |
| 15 | particlespriteemitter_18 | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 16 | after_wind0 | sprite |  | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 17 | helix5 | mesh(override mat) | fm_a_hemisphere_012 | bfx_i_pa_glow_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 18 | after_wind2 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | colorscaleoverlife, lifetime, locationdirect, parameterdynamic, size, sizemultiplylife, velocity |
| 19 | ee | ribbon |  |  |  | color_seeded, lifetime, lifetime_seeded, meshrotation_seeded, size_seeded, sizemultiplylife, spawn |
| 20 | dist_concave | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 21 | smoke | mesh(override mat) | fm_d_helix_015_1 | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 22 | glow | sprite |  | fx_m_pa_worldoffset_02_5_tr | fx_a_blankwhite_01, fx_a_fluid_017_n, fx_d_noise_002, fx_j_mirnoise_01, fx_m_caustic_001, fx_m_noise_001, fx_m_spatter_001_xyclamp | cameraoffset, color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 23 | after_wind4 | mesh(override mat) | fm_d_helix_015_1 | fx_n_me_ap_07_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife, velocity |
| 24 | flare | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | collision, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 25 | wave_gr_dark | sprite |  | fx_m_pa_worldoffset_02_25_tr | fx_a_blankwhite_01, fx_d_noise_002, fx_e_noise_001, fx_j_mirnoise_01, fx_j_mirnoise_02, fx_m_flow_04_n, fx_m_noise_001, fx_m_spatter_001_xyclamp | collision, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size, sizemultiplylife |

### `par_v_ppnn_wing_loop_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | sprite |  |  |  |  |

### `par_v_ppnn_wing_loop_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | par_d_ppnn_arrow | sprite |  |  |  |  |

## Ninave — `FX_MN_PPNN_00` (45 ParticleSystem)

### `par_d_gizmo_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, parameterdynamic, size |

### `par_d_ppnn_buff01_01s`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_5 | mesh(override mat) | fm_d_torus_002 | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, meshrotation, size |
| 1 | ring_out | mesh(override mat) | fm_a_plan_004 | fx_k_pa_shinemask_02_ad | fx_c_noise_005, fx_d_atypical_027, fx_d_noise_009 | color_seeded, colorscaleoverlife, parameterdynamic, sizemultiplylife |
| 2 | smoke_tail | sprite |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | color, colorscaleoverlife, sizemultiplylife |
| 3 | particlespriteemitter_26 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | coloroverlife, locationprimitivecylinder, size |
| 4 | particlespriteemitter_3 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | coloroverlife, locationprimitivecylinder, size |
| 5 | particlespriteemitter_35 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | coloroverlife |
| 6 | under_crash_01 | efdecal |  | fx_d_me_master_01_096_ad | fx_c_cloud_015, fx_d_atypical_076_cl, fx_d_atypical_114_cl, fx_e_symbol_064_1_cl | coloroverlife |
| 7 | particlespriteemitter_36 | sprite (disabled) |  | fx_e_pa_et_02_1_tr | fx_e_atypical_003, fx_e_electric_003, fx_e_electric_004, fx_e_noise_002, fx_e_noise_003 |  |
| 8 | particlespriteemitter_37 | sprite |  | fx_d_de_simple_01_32_tr | fx_a_decal_014, fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_d_normal_016_1 | coloroverlife, size |
| 9 | particlespriteemitter_39 | efdecal |  | fx_d_me_master_01_096_ad | fx_c_cloud_015, fx_d_atypical_076_cl, fx_d_atypical_114_cl, fx_e_symbol_064_1_cl | coloroverlife |
| 10 | line | sprite (disabled) |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | color, velocity |
| 11 | particlespriteemitter_2 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | coloroverlife |
| 12 | particlespriteemitter_20 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | color, colorscaleoverlife, parameterdynamic |
| 13 | particlespriteemitter_34 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal |  |
| 14 | particlespriteemitter_12 | sprite |  | fx_a_pa_et_01_2_ad | fx_a_electric_003, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, location, sizemultiplylife, velocity, velocityoverlifetime |

### `par_d_ppnn_buff02_02l`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | parents | sprite |  | fx_c_pa_light_01_1_ad | fx_e_adli, fx_f_star_001 | color, colorscaleoverlife, locationemitter, sizemultiplylife |
| 1 | particlespriteemitter_5 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, location, rotationrate, sizemultiplylife, velocity, velocityoverlifetime |
| 2 | particlespriteemitter_11 | sprite (disabled) |  | bfx_i_pa_thunder_02_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, size, sizemultiplylife, velocityoverlifetime |
| 3 | glow | sprite |  | bfx_i_pa_thunder_02_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, size, sizemultiplylife, velocityoverlifetime |

### `par_d_ppnn_prj01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | head00 | sprite |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 |  |
| 1 | particlespriteemitter_4 | sprite |  | fx_i_pa_thunder_02_ad | fx_a_noise_014, fx_a_noise_018, fx_d_noise_009, fx_i_environment_001, fx_i_shockwave_02_ycl, fx_i_thunder_02, fx_i_watercaustic_001 | coloroverlife, size |
| 2 | particlespriteemitter_6 | sprite |  |  |  |  |
| 3 | glow | sprite |  |  |  |  |
| 4 | particlespriteemitter_10 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, size |
| 5 | particlespriteemitter_12 | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | color, size |
| 6 | trail | ribbon |  |  |  | color, colorscaleoverlife, eflocationemitter, size |
| 7 | head02 | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | orbit |
| 8 | trail | ribbon |  |  |  | color, eflocationemitter, size |
| 9 | particlespriteemitter_21 | sprite |  | bfx_i_pa_backglow_cl_02_tr |  | color, colorscaleoverlife, size, sizemultiplylife |
| 10 | trail | ribbon |  | fx_a_pa_et_01_2_ad | fx_a_electric_003, fx_e_adsi, fx_e_normal | color, eflocationemitter |
| 11 | head03 | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | orbit |
| 12 | trail | ribbon |  | fx_a_pa_et_01_2_ad | fx_a_electric_003, fx_e_adsi, fx_e_normal | color, eflocationemitter |
| 13 | head01 | sprite |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | orbit |
| 14 | blackback | sprite |  | fx_c_pa_lensflare_01_04_ad | fx_c_glow_006, fx_c_glow_007 |  |

### `par_d_ppnn_prj01_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_17 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_12 | sprite |  | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation, size, subuv, velocity |
| 2 | ring_out | mesh(override mat) | fm_a_hemisphere_012 | fx_d_de_simple_01_32_tr | fx_a_decal_014, fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_d_normal_016_1 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 3 | smoke_tail | efdecal |  | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | coloroverlife, lifetime, location, rotation, size |
| 4 | under_crash_01 | sprite (disabled) |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, subuv |
| 5 | particlespriteemitter_18 | sprite (disabled) |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | acceleration, coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 6 | particlespriteemitter_19 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 7 | particlespriteemitter_20 | mesh(override mat) | fm_d_line_019 | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife |
| 8 | line | sprite (disabled) |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 9 | particlespriteemitter_22 | sprite (disabled) |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_23 | sprite (disabled) |  | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | coloroverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 11 | particlespriteemitter_25 | efdecal |  | fx_e_pa_gl_02_2_ad | fx_a_glow_002, fx_e_adsi, fx_e_normal | coloroverlife, colorscaleoverlife, lifetime, location, rotation, size |
| 12 | under_crash_01 | sprite |  | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation, size, subuv, velocity |

### `par_d_ppnn_prj02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_34 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, size |
| 1 | ring_out | mesh(override mat) | fm_a_hemisphere_012 | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | smoke_tail | mesh(override mat) | fm_a_hemisphere_012 | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_37 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, subuv |
| 4 | particlespriteemitter_38 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_39 | sprite (disabled) |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | acceleration, coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 6 | particlespriteemitter_40 | mesh(override mat) | fm_a_hemisphere_012 | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 7 | line | mesh(override mat) | fm_d_torus_002 | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife |
| 8 | particlespriteemitter_42 | sprite (disabled) |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 9 | particlespriteemitter_43 | sprite (disabled) |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_44 | sprite (disabled) |  | fx_s_pa_glow_01_1_ad |  | coloroverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 11 | particlespriteemitter_0 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_14 | sprite |  | fx_d_pa_glow_01_02_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 13 | particlespriteemitter_15 | sprite |  | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 14 | particlespriteemitter_13 | sprite |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 15 | under_crash_01 | sprite |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | cameraoffset, coloroverlife, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |

### `par_d_ppnn_prj02_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring_out | mesh(override mat) | fm_a_hemisphere_012 | fx_k_pa_shinemask_02_ad | fx_c_noise_005, fx_d_atypical_027, fx_d_noise_009 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 1 | smoke_tail | sprite |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 2 | particlespriteemitter_26 | sprite |  | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_3 | sprite |  | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_36 | sprite (disabled) |  | fx_d_pa_ring_07_79_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_normal_041_1, fx_i_shockwave_02_ycl | acceleration, coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 5 | particlespriteemitter_37 | sprite |  | bfx_h_pa_circle_01_02_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_46 | sprite |  | bfx_h_pa_circle_01_02_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | line | sprite (disabled) |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 8 | particlespriteemitter_2 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_20 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, size |
| 10 | particlespriteemitter_35 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, lifetime, location, rotation, size, sizemultiplylife |
| 11 | particlespriteemitter_0 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_47 | sprite (disabled) |  | fx_a_pa_cd_01_3_tr | fx_a_cloud_005, fx_e_normal, fx_e_trsi | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 13 | dust | sprite (disabled) |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | cameraoffset, coloroverlife, lifetime, location, rotation, rotationrate, size, sizemultiplylife, velocity, velocityoverlifetime |
| 14 | impact_particle | sprite |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | color, colorscaleoverlife, lifetime_seeded, meshrotation, parameterdynamic, rotation, size, sizemultiplylife, velocity_seeded |
| 15 | particlespriteemitter_5 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_prj03_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring_out | mesh(override mat) | fm_a_hemisphere_012 | fx_k_pa_shinemask_02_ad | fx_c_noise_005, fx_d_atypical_027, fx_d_noise_009 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 1 | smoke_tail | sprite |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 2 | particlespriteemitter_26 | sprite |  | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 3 | particlespriteemitter_34 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, subuv |
| 4 | particlespriteemitter_35 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_5 | efdecal |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | coloroverlife, lifetime, location, rotation, size |
| 6 | particlespriteemitter_36 | sprite (disabled) |  | fx_e_pa_et_02_1_tr | fx_e_atypical_003, fx_e_electric_003, fx_e_electric_004, fx_e_noise_002, fx_e_noise_003 | acceleration, coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 7 | particlespriteemitter_37 | sprite |  | fx_d_de_master_01_17_tr | fx_a_atypical_039, fx_a_decal_014_n, fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_normal_042_1, fx_d_normal_045_1, fx_e_cloud_008, fx_e_decal_003, fx_e_noise_002 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, orientationaxislock, parameterdynamic, size, velocity |
| 8 | particlespriteemitter_39 | sprite (disabled) |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | coloroverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 9 | line | sprite (disabled) |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 10 | particlespriteemitter_2 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 11 | particlespriteemitter_20 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, size |
| 12 | under_crash_01 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_prj04_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_prj05_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_5 | sprite |  | fx_d_pa_glow_01_02_ad |  | color, sizemultiplylife |
| 1 | ring_out | mesh(override mat) | fm_a_hemisphere_012 | fx_k_pa_shinemask_02_ad | fx_c_noise_005, fx_d_atypical_027, fx_d_noise_009 | color_seeded, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | smoke_tail | sprite |  | fx_e_pa_ht_15_1_tr | fx_e_hit_006, fx_e_trdi | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 3 | particlespriteemitter_26 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_3 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 5 | particlespriteemitter_35 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, lifetime, location, rotation, size, sizemultiplylife |
| 6 | under_crash_01 | efdecal |  | fx_s_pa_glow_01_1_ad |  | coloroverlife, lifetime, location, rotation, size |
| 7 | particlespriteemitter_36 | sprite (disabled) |  | fx_e_pa_et_02_1_tr | fx_e_atypical_003, fx_e_electric_003, fx_e_electric_004, fx_e_noise_002, fx_e_noise_003 | acceleration, coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocityoverlifetime |
| 8 | particlespriteemitter_37 | sprite |  | fx_d_de_simple_01_32_tr | fx_a_decal_014, fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_d_normal_016_1 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, orientationaxislock, parameterdynamic, size, velocity |
| 9 | particlespriteemitter_39 | efdecal |  | fx_s_pa_glow_01_1_ad |  | coloroverlife, lifetime, location, rotation, size |
| 10 | line | sprite (disabled) |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 11 | particlespriteemitter_2 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 12 | particlespriteemitter_20 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, size |
| 13 | particlespriteemitter_34 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, subuv |
| 14 | glow | sprite |  | fx_j_pa_shockwv_01_8_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_062, fx_d_noise_009, fx_e_line_004 | color |

### `par_d_ppnn_prj06`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, parameterdynamic, size |
| 1 | glow | sprite |  | fx_a_me_panning_02_ad | fx_a_atypical_048_cl, fx_a_line_011, fx_a_noise_009, fx_a_noise_011 | color |
| 2 | glow | sprite |  | fx_d_pa_master_01_042_ad | fx_a_hit_007, fx_c_cloud_015, fx_e_atypical_010 | color, colorscaleoverlife, rotationrate |
| 3 | glow | mesh(override mat) | fm_d_line_019 | fx_j_me_ringrainbow_01_3_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_i_atypical_03_1_ycl | coloroverlife, parameterdynamic, size |

### `par_d_ppnn_prj06_exp`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | sprite |  | fx_a_pa_gl_01_2_ad | fx_a_glow_003, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 1 | ring_out | mesh(override mat) | fm_d_helix_015_1 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color_seeded, colorscaleoverlife, parameterdynamic, sizemultiplylife |
| 2 | smoke_tail | sprite |  | fx_j_pa_lightingdetail_01_ad | fx_a_electric_003, fx_e_noise_002 | color, sizemultiplylife |
| 3 | particlespriteemitter_34 | sprite |  | fx_a_pa_gl_01_6_ad | fx_a_glow_001, fx_e_adsi, fx_e_normal |  |
| 4 | particlespriteemitter_35 | sprite |  | fx_e_pa_ht_08_1_ad | fx_a_fragment_001, fx_e_adsi, fx_e_normal | coloroverlife |
| 5 | under_crash_01 | efdecal |  | fx_n_pa_shine_01_01_ad |  | coloroverlife |
| 6 | particlespriteemitter_36 | sprite (disabled) |  | fx_e_pa_et_02_1_tr | fx_e_atypical_003, fx_e_electric_003, fx_e_electric_004, fx_e_noise_002, fx_e_noise_003 |  |
| 7 | particlespriteemitter_37 | sprite |  | fx_d_de_simple_01_33_tr | fx_a_hit_004, fx_c_decal_002_1, fx_c_decal_002_3, fx_e_decal_013, fx_e_decal_013_e | coloroverlife, size |
| 8 | particlespriteemitter_39 | efdecal |  | fx_n_pa_shine_01_01_ad |  | coloroverlife |
| 9 | line | sprite (disabled) |  | bfx_k_pa_radialcolor_01_5_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_d_noise_029, fx_i_atypical_01_ycl, fx_i_environment_001, fx_i_noise_04 | color, velocity |
| 10 | particlespriteemitter_2 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | coloroverlife |
| 11 | particlespriteemitter_20 | sprite |  | fx_k_me_makeflow_01_03_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01 | color, colorscaleoverlife, parameterdynamic |

### `par_d_ppnn_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_4 | sprite |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | color, colorscaleoverlife, lifetime, location, orbit, size, velocity, velocityoverlifetime |
| 1 | particlespriteemitter_9 | sprite (disabled) |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | color, colorscaleoverlife, lifetime, location, orbit, size, velocity, velocityoverlifetime |
| 2 | particlespriteemitter_2 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_3 | sprite |  | fx_a_pa_gl_01_2_ad | fx_a_glow_003, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 4 | bow-string-on | sprite |  | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | color, colorscaleoverlife, eventreceiverspawn, lifetime, orientationaxislock, rotation, size, sizemultiplylife, subuv |

### `par_d_ppnn_sk01_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | bow-string-off | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_10 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_sk01_06`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_7 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_8 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_9 | mesh(override mat) | fm_k_tornado_01 | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_13 | sprite |  | fx_a_pa_et_01_2_ad | fx_a_electric_003, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 4 | particlespriteemitter_11 | mesh(override mat) | fm_b_cylinder_002 | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_15 | mesh(override mat) | fm_b_cylinder_002 | fx_e_me_ap_02_5_ts_tr | fx_e_atypical_002, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 6 | after_wind | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |

### `par_d_ppnn_sk02_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_15 | sprite |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | color, colorscaleoverlife, eventgenerator, lifetime, orientationaxislock, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_14 | sprite |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | cameraoffset, color, colorscaleoverlife, lifetime, location, rotation, size, subuv |

### `par_d_ppnn_sk02_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | bow-string-on | sprite |  | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | color, colorscaleoverlife, eventreceiverspawn, lifetime, orientationaxislock, rotation, size, sizemultiplylife, subuv |

### `par_d_ppnn_sk02_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | bow-string-off | sprite |  | fx_n_pa_shine_01_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, locationprimitivesphere, orientationaxislock, size, sizemultiplylife, subuv, subuvmovie |
| 1 | particlespriteemitter_7 | sprite |  |  |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 2 | particlespriteemitter_8 | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 3 | head06 | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |

### `par_d_ppnn_sk02_11`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | trail | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 1 | head02 | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |
| 2 | trail | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 3 | head03 | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |
| 4 | trail | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 5 | head04 | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 6 | head05 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife, velocity |
| 7 | trail | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationonground, lifetime, size |
| 8 | particlespriteemitter_1 | sprite (disabled) |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife, velocity |
| 9 | trail | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 10 | trail | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |
| 11 | head07 | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |
| 12 | trail | sprite |  | fx_d_pa_master_01_060_tr | fx_c_cloud_015, fx_d_atypical_035_cl, fx_d_atypical_039_1, fx_d_atypical_041_ycl, fx_k_fluidtile_02 | lifetime, location, orbit, velocity, velocityoverlifetime |
| 13 | head01 | ribbon |  | fx_m_pa_shine_02_1_ad | fx_a_noise_011 | color, eflocationemitter, lifetime, size |

### `par_d_ppnn_sk03_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_28 | sprite |  | fx_e_pa_tl_02_2_ad |  | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 1 | particlespriteemitter_29 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_3 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 3 | particlespriteemitter_2 | sprite (disabled) |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_4 | sprite |  | fx_e_pa_tl_02_2_ad |  | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 5 | particlespriteemitter_6 | sprite |  | fx_e_pa_tl_02_2_ad |  | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 6 | particlespriteemitter_27 | sprite |  | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 7 | particlespriteemitter_5 | sprite |  | fx_e_pa_tl_02_2_ad |  | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 8 | glow | ribbon |  | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, lifetime, size |

### `par_d_ppnn_sk07_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_6 | ribbon |  | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, lifetime, size |
| 1 | big-s | sprite (disabled) |  | fx_c_pa_light_01_1_ad | fx_e_adli, fx_f_star_001 | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocityoverlifetime |
| 2 | particlespriteemitter_9 | sprite |  | fx_c_pa_light_01_1_ad | fx_e_adli, fx_f_star_001 | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocityoverlifetime |

### `par_d_ppnn_sk08_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | big-s | sprite (disabled) |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocity, velocityoverlifetime |
| 1 | glow | sprite |  | fx_c_pa_light_01_1_ad | fx_e_adli, fx_f_star_001 | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocityoverlifetime |
| 2 | big-f | sprite |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocity, velocityoverlifetime |

### `par_d_ppnn_sk08_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_21 | mesh(override mat) | fm_o_swing_01 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |
| 1 | glow | sprite (disabled) |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocity, velocityoverlifetime |
| 2 | big-f | sprite |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocity, velocityoverlifetime |
| 3 | big-f | mesh(override mat) | fm_o_swing_01 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |
| 4 | big-s | sprite |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, rotation, rotationrate, size, velocity, velocityoverlifetime |

### `par_g_ppnn_reinforcement_01_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | efdecal |  | fx_d_pa_atta_09_17_dt_ad | fx_b_atypical_007, fx_d_atypical_027, fx_d_noise_004_1, fx_e_adba | color, colorscaleoverlife, lifetime, location, rotation, size |
| 1 | smoke_tail | mesh(override mat) (disabled) | fm_a_stone_001 | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | cameraoffset, color_seeded, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_72 | sprite |  | fx_y_de_master_01_74_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_decal_004, fx_d_environ_001, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002, fx_f_decal_007 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 3 | particlespriteemitter_34 | mesh(override mat) | fm_b_cylinder_002 | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |

### `par_g_ppnn_reinforcement_01_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | impact_particle | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 1 | lightning1 | sprite (disabled) |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_10 | sprite |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 3 | dust | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 4 | under_crash_01 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_25 | sprite |  | fx_y_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_environ_018, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 6 | particlespriteemitter_19 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 7 | particlespriteemitter_31 | sprite |  | fx_l_de_master_04_01_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_decal_002, fx_e_decal_007_1_n, fx_e_decal_007_2, fx_e_decal_018, fx_e_noise_002 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, velocity |
| 8 | particlespriteemitter_16 | sprite |  | fx_o_pa_splitline_02_ad | fx_d_atypical_045 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity_seeded |
| 9 | particlespriteemitter_26 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_21 | sprite (disabled) |  | fx_y_me_master_01_063_ma | fx_a_environ_003, fx_a_fluid_019_n, fx_c_noise_007, fx_d_atypical_054, fx_d_fluid_012, fx_i_environment_001 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 11 | under_crash_01 | efdecal |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, location_seeded, rotation, size |
| 12 | rock | mesh(override mat) (disabled) | fm_b_cylinder_002 | fx_x_pa_flowmask_08_01_tr | fx_d_hit_004, fx_d_noise_002, fx_j_mirnoise_01, fx_k_smoke_02, fx_l_environment_001 | acceleration, collision, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 13 | dust | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 14 | particlespriteemitter_18 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 15 | particlespriteemitter_13 | sprite |  | fx_y_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_environ_018, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 16 | particlespriteemitter_21 | mesh(override mat) | fm_o_swing_01 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |

### `par_g_ppnn_reinforcement_02_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | efdecal |  | fx_d_pa_atta_09_17_dt_ad | fx_b_atypical_007, fx_d_atypical_027, fx_d_noise_004_1, fx_e_adba | color, colorscaleoverlife, lifetime, location, rotation, size |
| 1 | smoke_tail | mesh(override mat) (disabled) | fm_a_stone_001 | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | cameraoffset, color_seeded, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_72 | sprite |  | fx_l_de_master_04_01_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_decal_002, fx_e_decal_007_1_n, fx_e_decal_007_2, fx_e_decal_018, fx_e_noise_002 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 3 | particlespriteemitter_34 | mesh(override mat) | fm_b_cylinder_002 | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |

### `par_g_ppnn_reinforcement_02_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | impact_particle | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 1 | lightning1 | sprite (disabled) |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 2 | under_crash_01 | efdecal |  | fx_y_de_master_01_74_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_decal_004, fx_d_environ_001, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002, fx_f_decal_007 | color, colorscaleoverlife, lifetime, location_seeded, rotation, size |
| 3 | particlespriteemitter_10 | sprite |  | fx_d_pa_ringmaster_01_41_dt_ad | fx_f_aura_004_1, fx_m_trail_001_ycl | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 4 | under_crash_01 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_25 | sprite |  | fx_y_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_environ_018, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 6 | particlespriteemitter_19 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 7 | particlespriteemitter_31 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, velocity |
| 8 | particlespriteemitter_16 | sprite |  | fx_o_pa_splitline_02_ad | fx_d_atypical_045 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity_seeded |
| 9 | particlespriteemitter_26 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_21 | sprite (disabled) |  | fx_y_me_master_01_063_ma | fx_a_environ_003, fx_a_fluid_019_n, fx_c_noise_007, fx_d_atypical_054, fx_d_fluid_012, fx_i_environment_001 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 11 | dust | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 12 | rock | mesh(override mat) (disabled) | fm_d_electric_02 | fx_x_pa_flowmask_08_01_tr | fx_d_hit_004, fx_d_noise_002, fx_j_mirnoise_01, fx_k_smoke_02, fx_l_environment_001 | acceleration, collision, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 13 | dust | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocityoverlifetime |
| 14 | particlespriteemitter_18 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 15 | particlespriteemitter_13 | sprite |  | fx_y_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_environ_018, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 16 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |

### `par_g_ppnn_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | arrow2-k | mesh(override mat) | fm_k_tornado_01 | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 1 | electric-centre | mesh(override mat) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 2 | arrow3 | mesh(override mat) | fm_c_screwfront_001 | fx_d_me_master_01_114_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_i_atypical_03_2_xcl, fx_i_smoke_01 | color, colorscaleoverlife, lifetime, locationemitter, meshrotation, meshrotationrate, size, sizemultiplylife |
| 3 | arrow3 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 4 | arrow2 | mesh(override mat) | fm_c_screwfront_001 | fx_d_me_master_01_114_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_i_atypical_03_2_xcl, fx_i_smoke_01 | color, colorscaleoverlife, lifetime, locationemitter, meshrotation, meshrotationrate, size, sizemultiplylife |
| 5 | ghost01 | mesh(override mat) | fm_k_tornado_01 | fx_d_me_master_01_110_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_114_cl, fx_d_atypical_127_xcl, fx_i_thunder_02 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 6 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 7 | arrow-aura | mesh(override mat) | fm_k_tornado_01 | fx_d_pa_ribbonflow_02_01_ad | fx_d_atypical_055_2_cl, fx_d_atypical_060_cl, fx_d_electric_016, fx_d_noise_009, fx_d_normal_085, fx_k_auraline_01_ycl, fx_k_auraline_08 | color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 8 | thunder | mesh(override mat) (disabled) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 9 | particlespriteemitter_0 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_6 | sprite (disabled) |  | fx_e_me_ap_25_1_ts_ad | fx_a_fragment_005, fx_a_noise_002, fx_e_addi | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, parameterdynamic, size, sizemultiplylife, velocity |
| 11 | arrow-head | mesh(override mat) | fm_d_rectan3cross_001 | fx_d_me_master_01_111_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_006_1, fx_d_atypical_011, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl | color, colorscaleoverlife, lifetime, locationemitter, meshrotation, meshrotationrate, size, sizemultiplylife |
| 12 | arrow2 | mesh(override mat) | fm_d_rectan3cross_001 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |

### `par_y_ppnn_reinforcement_01_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_13 | sprite |  | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, size, sizemultiplylife, velocity_seeded |
| 1 | particlespriteemitter_8 | mesh(override mat) | fm_k_tornado_01 | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | particlespriteemitter_1 | mesh(override mat) | fm_d_helix_015_1 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_9 | sprite |  | fx_y_me_master_01_018_tr | fx_c_cloud_015, fx_d_noise_009, fx_d_noise_044, fx_k_electile_01, fx_k_electile_02, fx_k_electric_01 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 4 | particlespriteemitter_10 | mesh(override mat) | fm_k_tornado_01 | bfx_i_pa_thunder_03_ad | fx_a_noise_017, fx_a_noise_018, fx_i_environment_001, fx_i_thunder_01_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 5 | particlespriteemitter_0 | sprite |  | fx_y_me_master_01_018_tr | fx_c_cloud_015, fx_d_noise_009, fx_d_noise_044, fx_k_electile_01, fx_k_electile_02, fx_k_electric_01 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 6 | arrow2 | mesh(override mat) | fm_k_tornado_01 | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | acceleration, color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 7 | particlespriteemitter_11 | sprite |  | fx_j_pa_sphereline_01_ad | fx_b_atypical_026, fx_e_adli | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 8 | particlespriteemitter_4 | mesh(override mat) | fm_a_sprial_010 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 9 | particlespriteemitter_12 | sprite |  | fx_j_pa_sphereline_01_ad | fx_b_atypical_026, fx_e_adli | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 10 | particlespriteemitter_5 | mesh(override mat) | fm_a_sprial_010 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 11 | after_wind | mesh(override mat) | fm_k_tornado_01 | fx_m_pa_missiletrail_01_3_tr | fx_a_cloud_022, fx_d_noise_021, fx_i_fragment_02_cl, fx_m_atypical_004_loc_int | acceleration, color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 12 | smoke_tail | mesh(override mat) | fm_c_screwfront_001 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | color_seeded, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 13 | particlespriteemitter_7 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 14 | arrow2 | mesh(override mat) | fm_k_tornado_01 | fx_m_pa_missiletrail_01_3_tr | fx_a_cloud_022, fx_d_noise_021, fx_i_fragment_02_cl, fx_m_atypical_004_loc_int | acceleration, color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 15 | particlespriteemitter_3 | mesh(override mat) | fm_k_tornado_01 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 16 | particlespriteemitter_14 | mesh(override mat) (disabled) | fm_c_screwfront_001 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 17 | particlespriteemitter_15 | mesh(override mat) | fm_k_tornado_01 | fx_e_me_ap_02_2_ts_tr | fx_e_atypical_002, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 18 | glow03 | mesh(override mat) | fm_e_arrow_001 | fx_o_me_flowtrail_01_23_tr | fx_c_line_004_ycl, fx_d_atypical_028, fx_j_mirnoise_02, fx_l_environment_001, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation, meshrotationrate, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife, velocity |
| 19 | particlespriteemitter_2 | mesh(override mat) | fm_k_tornado_01 | fx_d_me_master_01_112_ts_fs_dt_ad | fx_c_cloud_015, fx_d_atypical_060_cl, fx_d_atypical_069_ycl, fx_d_atypical_076_1_cl, fx_d_electric_013_1, fx_k_electric_01 | acceleration, color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_y_ppnn_reinforcement_01_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | lightning1 | sprite (disabled) |  | fx_y_de_master_01_74_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_decal_004, fx_d_environ_001, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002, fx_f_decal_007 | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_13 | sprite |  | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_0 | efdecal |  | fx_d_pa_atta_09_17_dt_ad | fx_b_atypical_007, fx_d_atypical_027, fx_d_noise_004_1, fx_e_adba | color, colorscaleoverlife, lifetime, location, rotation, size |
| 3 | particlespriteemitter_14 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 4 | ee | mesh(override mat) | fm_a_stone_001 | fx_a_pa_lightning04_ad | fx_a_atypical_026, fx_a_atypical_046, fx_a_electric_008 | cameraoffset, color, lifetime, location, parameterdynamic, size |

### `par_y_ppnn_reinforcement_01_3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | impact_particle | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 1 | particlespriteemitter_13 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 2 | particlespriteemitter_10 | sprite (disabled) |  | fx_t_de_master_01_03_tr | fx_a_hit_004, fx_a_ice_002, fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_c_ice_002, fx_d_decal_037, fx_d_environ_001, fx_d_normal_037_1, fx_e_cloud_008, fx_e_noise_002, fx_m_flow_04_n | color, colorscaleoverlife, eflocationonground, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 3 | particlespriteemitter_25 | sprite |  | fx_y_de_master_01_73_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_environ_018, fx_d_normal_078, fx_d_normal_078_1, fx_e_cloud_008, fx_e_noise_002 | color, colorscaleoverlife, eflocationonground, lifetime, location, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_19 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 5 | dust | sprite (disabled) |  | fx_o_me_watertrail_01_47_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 6 | particlespriteemitter_18 | efdecal |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 7 | particlespriteemitter_26 | sprite |  | fx_a_pa_firework_01_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity |
| 8 | particlespriteemitter_21 | sprite (disabled) |  | fx_y_me_master_01_063_ma | fx_a_environ_003, fx_a_fluid_019_n, fx_c_noise_007, fx_d_atypical_054, fx_d_fluid_012, fx_i_environment_001 | color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 9 | rock | mesh(override mat) (disabled) | fm_m_sphere_006 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | acceleration, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 10 | dust | sprite (disabled) |  | fx_o_me_watertrail_01_47_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | cameraoffset, color, colorscaleoverlife, eflocationonground, lifetime, location, locationprimitivecylinder, rotation, rotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |

### `par_y_ppnn_reinforcement_02_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_51 | sprite |  | fx_e_pa_gl_03_1_ad | fx_a_noise_002, fx_a_ring_001_cl, fx_b_glow_001_cl | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 1 | ring-start | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 2 | ring-start | sprite (disabled) |  | fx_o_pa_flare_02_03_ad | fx_d_atypical_009 | color, colorscaleoverlife, location, orientationaxislock, rotation, sizemultiplylife |
| 3 | particlespriteemitter_68 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 4 | dist_concave | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_13 | mesh(override mat) (disabled) | fm_d_ring_008 | fx_a_pa_lightning04_ad | fx_a_atypical_026, fx_a_atypical_046, fx_a_electric_008 | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, orbit, parameterdynamic, size, sizemultiplylife, velocity |
| 6 | thunder | mesh(override mat) | fm_a_ice_004 | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 7 | particlespriteemitter_83 | sprite |  | bfx_d_pa_circ_02_ad_dt |  | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocityoverlifetime |
| 8 | particlespriteemitter_85 | sprite (disabled) |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | cameraoffset, coloroverlife, lifetime, location, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_72 | sprite |  | fx_x_pa_flowmask_08_01_tr | fx_d_hit_004, fx_d_noise_002, fx_j_mirnoise_01, fx_k_smoke_02, fx_l_environment_001 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 10 | particlespriteemitter_87 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, subuv, velocity |
| 11 | particlespriteemitter_78 | sprite |  | fx_x_pa_flowmask_08_01_tr | fx_d_hit_004, fx_d_noise_002, fx_j_mirnoise_01, fx_k_smoke_02, fx_l_environment_001 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocityoverlifetime |
| 12 | ice_twinkle | sprite |  | fx_j_pa_ht_01_01_tr | fx_e_hit_006, fx_e_trdi | acceleration, color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, rotation, size, sizemultiplylife, velocity |
| 13 | particlespriteemitter_28 | sprite (disabled) |  | fx_j_pa_ht_01_2_tr | fx_c_hit_002, fx_e_hit_007_1, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 14 | impact_particle | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime_seeded, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 15 | particlespriteemitter_86 | sprite |  | fx_j_lightingdetail_02_ad | fx_e_noise_002, fx_i_thunder_01_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 16 | ice_twinkle | sprite |  | fx_o_pa_ring_11_26_ad |  | acceleration, color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife, velocity |
| 17 | particlespriteemitter_17 | sprite |  | bfx_d_pa_shine_02_ad_inst32535 |  | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 18 | particlespriteemitter_49 | sprite (disabled) |  | fx_o_pa_ringmaster_01_12_tr | fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 19 | particlespriteemitter_29 | sprite |  | fx_a_pa_firework_01_ad | fx_a_atypical_009, fx_a_fire_006 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 20 | particlespriteemitter_11 | mesh(override mat) | fm_d_electric_02 | fx_y_me_master_06_tr | fx_c_cloud_015, fx_d_atypical_054, fx_d_electric_008_1_cl, fx_d_noise_009 | color, colorscaleoverlife, lifetime, location, meshrotationrate, parameterdynamic, size, sizemultiplylife |

### `par_y_ppnn_reinforcement_02_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | under_crash_01 | efdecal |  | fx_y_me_master_01_063_ma | fx_a_environ_003, fx_a_fluid_019_n, fx_c_noise_007, fx_d_atypical_054, fx_d_fluid_012, fx_i_environment_001 | color, colorscaleoverlife, lifetime, rotation, size |
| 1 | particlespriteemitter_63 | mesh(override mat) | fm_b_cylinder_002 | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | color, colorscaleoverlife, eflocationonground, lifetime, location, meshrotation, size, sizemultiplylife |
| 2 | under_crash_01 | efdecal (disabled) |  | fx_y_me_master_01_063_ma | fx_a_environ_003, fx_a_fluid_019_n, fx_c_noise_007, fx_d_atypical_054, fx_d_fluid_012, fx_i_environment_001 | color, colorscaleoverlife, lifetime, location, rotation, size |
| 3 | under_crash_01 | sprite |  | fx_y_de_ground_99_01_tr | fx_a_decal_009, fx_bg_dustpanner_01, fx_d_atypical_055_2_cl, fx_d_fire_017, fx_d_fragment_016, fx_d_normal_033_1, fx_d_symbol_111_cl | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, location, rotation, size, sizemultiplylife, subuv, velocity |
| 4 | particlespriteemitter_84 | sprite (disabled) |  | fx_y_de_ground_99_01_tr | fx_a_decal_009, fx_bg_dustpanner_01, fx_d_atypical_055_2_cl, fx_d_fire_017, fx_d_fragment_016, fx_d_normal_033_1, fx_d_symbol_111_cl | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, location, rotation, size, sizemultiplylife, subuv, velocity |
| 5 | lightning1 | sprite (disabled) |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_0 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 7 | particlespriteemitter_9 | mesh(override mat) (disabled) | fm_a_broken_012 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, orbit, parameterdynamic, size, sizemultiplylife, velocity |
| 8 | 1- | sprite (disabled) |  | fx_d_pa_turbulence_01_19_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_e_atypical_012, fx_i_noise_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, particlespriteemitter, rotation, size, sizemultiplylife, velocity |
| 9 | 1- | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, particlespriteemitter, rotation, size, sizemultiplylife, velocity |
| 10 | under_crash_01 | efdecal |  | fx_d_me_master_01_018_ad | fx_a_atypical_019_1, fx_c_cloud_015, fx_d_environ_035, fx_i_environment_002 | color, colorscaleoverlife, lifetime, rotation, size |

### `par_y_ppnn_reinforcement_02_3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring-start | sprite |  | bfx_d_pa_circ_01_02_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | ring-start | sprite |  | fx_a_pa_lightning04_ad | fx_a_atypical_026, fx_a_atypical_046, fx_a_electric_008 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 2 | ring-start | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | particlespriteemitter_57 | sprite |  | fx_a_pa_lightning04_ad | fx_a_atypical_026, fx_a_atypical_046, fx_a_electric_008 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, subuv, velocity |
| 4 | lightning1 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | cameraoffset, coloroverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 5 | ring-start | sprite |  | bfx_d_pa_flar_01_03_dt_tr | fx_d_atypical_010, fx_d_cloud_035 | color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife, velocity |

### `par_y_ppnn_reinforcement_02_4`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring-start | sprite |  | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | color, colorscaleoverlife, lifetime, size, sizemultiplylife, velocity |
| 1 | particlespriteemitter_21 | mesh(override mat) | fm_o_swing_01 | fx_d_me_electric_03_13_ad | fx_d_electric_011, fx_e_electric_005, fx_e_noise_001 | coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size |

### `par_y_ppnn_reinforcement_02_5`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_d_wing_002 | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |
| 1 | smoke_tail | mesh(override mat) (disabled) | fm_d_wing_002 | fx_m_me_trail_02_19_tr | fx_a_cloud_022, fx_k_electile_02, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002, fx_m_wave_001_ycl | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_72 | sprite |  | fx_d_me_master_01_113_ad | fx_c_cloud_015, fx_l_pabs_00_d, fx_l_pabs_00_s | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotationrate, size, sizemultiplylife |
| 3 | particlespriteemitter_34 | mesh(override mat) | fm_b_cylinder_002 | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | coloroverlife, colorscaleoverlife, lifetime, locationdirect, meshrotation, parameterdynamic, size |

### `par_y_ppnn_reinforcement_03_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ee | mesh(override mat) | fm_d_helix_002 | fx_c_pa_light_01_1_ad | fx_e_adli, fx_f_star_001 | color, colorscaleoverlife, lifetime, meshrotation, size, sizemultiplylife |

### `par_y_ppnn_reinforcement_03_2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | glow | sprite |  | fx_d_me_flow_02_28_tr | fx_b_atypical_004, fx_c_trail_005_cl, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl | cameraoffset, color, colorscaleoverlife, lifetime, rotationrate, size, sizemultiplylife |

### `par_y_ppnn_reinforcement_03_3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_2 | mesh(override mat) | fm_d_hemisphere_008_1 | fx_d_me_ringmaster_01_14_ad | fx_a_noise_008_n, fx_e_fire_005_1, fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, meshrotation_seeded, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_3 | ribbon (disabled) |  | bfx_c_pa_lightflare_01_ddt_4_ad | fx_c_glow_009, fx_c_glow_010 | color, colorscaleoverlife, lifetime, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_4 | sprite (disabled) |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 3 | bow-string-on | sprite |  | fx_d_pa_atta_05_24_ad | fx_d_typical_017_cl, fx_e_adli | color, colorscaleoverlife, eventreceiverspawn, lifetime, orientationaxislock, rotation, size, sizemultiplylife |

### `par_y_ppnn_reinforcement_04_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | bow-string-off | sprite |  | fx_d_pa_master_01_024_ts_ad | fx_c_cloud_015, fx_d_atypical_041_ycl, fx_d_noise_021, fx_i_thunder_03 | color, colorscaleoverlife, lifetime, orientationaxislock, rotation, size, sizemultiplylife, subuv |
| 1 | particlespriteemitter_7 | sprite |  | fx_n_pa_shine_01_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, locationprimitivesphere_seeded, orientationaxislock, size, sizemultiplylife, subuv, typedatamesh |
| 2 | particlespriteemitter_8 | sprite |  | fx_e_pa_tl_02_2_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 3 | glow | particlemodulevelocity |  | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, lifetime, size |

### `par_y_ppnn_reinforcement_05_1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_6 | particlemodulevelocity |  | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, lifetime, size |
| 1 | particlespriteemitter_0 | sprite (disabled) |  | fx_j_pa_smokes_1_tr | fx_a_glow_009, fx_j_bigsmokes_m_01 | coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, orientationaxislock, rotation, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_9 | sprite |  | fx_j_pa_smokes_1_tr | fx_a_glow_009, fx_j_bigsmokes_m_01 | coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, orientationaxislock, rotation, size, sizemultiplylife, subuv, velocity |

### `par_y_ppnn_reinforcement_dust_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, subuv |
| 1 | particlespriteemitter_2 | sprite (disabled) |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, orientationaxislock, rotation, size, sizemultiplylife, subuv, velocity |
| 2 | particlespriteemitter_4 | sprite |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, subuv |
| 3 | particlespriteemitter_3 | sprite (disabled) |  | fx_j_pa_smokes_1_tr | fx_a_glow_009, fx_j_bigsmokes_m_01 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_5 | sprite (disabled) |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 5 | particlespriteemitter_3 | sprite |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv, velocity |

### `par_y_ppnn_reinforcement_dust_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | sprite (disabled) |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv, velocity |

### `par_y_ppnn_reinforcement_dust_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | sprite (disabled) |  | fx_y_de_master_06_02_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_e_cloud_008, fx_e_noise_002, fx_y_decal_01, fx_y_decal_01_e, fx_y_decal_01_n | colorscaleoverlife, eftypedatadecal, efvelocityoverlifetime, lifetime, location_seeded, locationprimitivecylinder_seeded, rotation, size, spawn, subuvmovie, velocity |

### `par_y_ppnn_reinforcement_dust_04`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | par_d_gizmo_01 | sprite |  |  |  |  |
| 1 | particlespriteemitter_5 | efparticlemodulevelocityoverlifetime |  |  |  | color, colorscaleoverlife, eventgenerator, lifetime_seeded, rotation_seeded, size_seeded |
| 2 | under_crash_01 | efparticlemodulevelocityoverlifetime (disabled) |  |  |  | color, colorscaleoverlife, eventgenerator, lifetime_seeded, rotation_seeded, size_seeded |

## Inanna — `FX_ESTHER_SLINN_00` (72 ParticleSystem)

### `par_d_esther_link_inanna_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | mesh | fm_d_tree_02 | fx_d_pa_glow_02_02_dt15_ad |  | colorscaleoverlife, size |
| 1 | particlespriteemitter_10 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_master_01_112_dt_tr | fx_c_cloud_015, fx_d_atypical_076_1_cl, fx_d_typical_008, fx_s_plus_01 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 2 | particlespriteemitter_5 | mesh(override mat) | fm_d_tree_01 | fx_d_pa_glow_02_02_dt15_ad |  | acceleration, color, colorscaleoverlife, locationprimitivesphere, meshrotationrate, size |
| 3 | particlespriteemitter_9 | mesh(override mat) | fm_d_tree_01 | fx_d_pa_glow_01_05_dt_ad |  | acceleration, color, colorscaleoverlife, locationprimitivesphere, meshrotationrate, size |
| 4 | particlespriteemitter_11 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_master_01_112_dt_tr | fx_c_cloud_015, fx_d_atypical_076_1_cl, fx_d_typical_008, fx_s_plus_01 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 5 | particlespriteemitter_12 | sprite (disabled) |  | fx_h_pa_shine_03_1_ad | fx_d_noise_030 | color, eflocationprimitivesphere, efvelocityoverlifetime, sizemultiplylife |
| 6 | particlespriteemitter_3 | sprite |  | fx_d_pa_shine_01_11_dt5_ad |  | color, parameterdynamic |
| 7 | particlespriteemitter_4 | mesh | fm_d_tree_02 | fx_d_pa_glow_02_02_dt15_ad |  | colorscaleoverlife, size |
| 8 | particlespriteemitter_7 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, eflocationprimitivesphere, efvelocityoverlifetime, sizemultiplylife |
| 9 | particlespriteemitter_0 | mesh(override mat) (disabled) | fm_d_ring_009 | fx_d_pa_glow_02_02_dt15_ad |  | size |
| 10 | particlespriteemitter_6 | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color |
| 11 | particlespriteemitter_8 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_master_01_112_dt_tr | fx_c_cloud_015, fx_d_atypical_076_1_cl, fx_d_typical_008, fx_s_plus_01 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 12 | particlespriteemitter_13 | sprite |  | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, sizemultiplylife |
| 13 | particlespriteemitter_14 | sprite |  | fx_d_pa_ringmaster_01_31_dt_ad | fx_f_aura_004_1, fx_i_noise_03, fx_j_mirnoise_01 | color, colorscaleoverlife, size, sizemultiplylife |
| 14 | particlespriteemitter_2 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, parameterdynamic |

### `par_d_esther_link_inanna_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | mesh(override mat) (disabled) | fm_d_ring_009 | fx_d_pa_glow_02_02_dt15_ad |  | size |
| 1 | particlespriteemitter_10 | mesh(override mat) | fm_d_tree_01 | fx_d_pa_glow_02_02_dt15_ad |  | eflocationprimitivecylinderspin |
| 2 | particlespriteemitter_8 | mesh(override mat) | fm_d_tree_01 | fx_d_pa_glow_02_02_dt15_ad |  | eflocationprimitivecylinderspin |
| 3 | particlespriteemitter_12 | sprite |  | fx_m_pa_ringaura_01_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_032, fx_i_noise_03 | color |
| 4 | particlespriteemitter_11 | mesh(override mat) | fm_d_tree_01 | fx_d_pa_glow_02_02_dt15_ad |  | eflocationprimitivecylinderspin |
| 5 | particlespriteemitter_2 | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, parameterdynamic |
| 6 | particlespriteemitter_3 | sprite |  | fx_d_pa_ringmaster_01_31_dt_ad | fx_f_aura_004_1, fx_i_noise_03, fx_j_mirnoise_01 | color, parameterdynamic |
| 7 | particlespriteemitter_4 | mesh(override mat) (disabled) | fm_d_ring_009 | fx_d_pa_glow_02_02_dt15_ad |  | size |

### `par_d_esther_link_inanna_11`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_14 | sprite |  | fx_m_pa_ringaura_01_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_032, fx_i_noise_03 | color |
| 1 | particlespriteemitter_9 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_glow_01_01_dt_ad |  | color, colorscaleoverlife, efvortex, locationprimitivesphere, meshrotationrate, size, velocity |
| 2 | particlespriteemitter_6 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | colorscaleoverlife, eflocationprimitivecylinderspin, location, meshrotation, sizemultiplylife |
| 3 | smoke_tail | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | size |
| 4 | particlespriteemitter_5 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | colorscaleoverlife, eflocationprimitivecylinderspin, location, meshrotation, sizemultiplylife |
| 5 | particlespriteemitter_19 | sprite |  | fx_d_pa_shine_01_11_dt5_ad |  | color, colorscaleoverlife, sizemultiplylife |
| 6 | particlespriteemitter_20 | sprite |  | fx_d_me_flow_02_06_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_006_1_cl | color, colorscaleoverlife, size, sizemultiplylife |
| 7 | particlespriteemitter_3 | mesh | fm_d_tree_02 | fx_f_pa_wind_05_tr | fx_a_atypical_003, fx_a_fire_003, fx_c_atypical_016, fx_c_noise_002, fx_d_noise_003, fx_d_noise_014 | color, meshrotation, parameterdynamic, sizemultiplylife |
| 8 | particlespriteemitter_10 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 9 | particlespriteemitter_8 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | colorscaleoverlife, eflocationprimitivecylinderspin, location, meshrotation, sizemultiplylife |

### `par_d_gizmo_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_16 | sprite |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 1 | glow_fill-l | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_14 | sprite |  | fx_d_pa_turbulence_01_07_tr | fx_a_hit_001, fx_b_atypical_004, fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_i_noise_01 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | note | sprite |  | fx_c_pa_flickline_01_1_ad | fx_f_star_001 | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 4 | particlespriteemitter_17 | sprite |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_19 | sprite |  | fx_c_pa_lensflare_01_01_ad | fx_c_glow_006 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 6 | particlespriteemitter_20 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | coloroverlife, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 7 | particlespriteemitter_24 | sprite |  | fx_k_me_ring_01_ad | fx_a_noise_011, fx_b_atypical_004, fx_d_atypical_011, fx_d_atypical_031, fx_d_hit_008 | coloroverlife, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 8 | particlespriteemitter_25 | sprite |  | fx_k_me_ring_01_ad | fx_a_noise_011, fx_b_atypical_004, fx_d_atypical_011, fx_d_atypical_031, fx_d_hit_008 | color |
| 9 | note | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife |

### `par_d_slinn_00-4_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_27 | sprite |  | fx_c_pa_flickline_01_1_ad | fx_f_star_001 | color |
| 1 | glow_fill-l | sprite (disabled) |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 |  |
| 2 | particlespriteemitter_31 | sprite |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 |  |
| 3 | particlespriteemitter_32 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color |
| 4 | particlespriteemitter_33 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 |  |
| 5 | particlespriteemitter_35 | mesh (disabled) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, locationprimitivesphere, meshrotationrate, size, velocityoverlifetime |
| 6 | particlespriteemitter_36 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_glow_02_02_dt15_ad |  | size |
| 7 | particlespriteemitter_0 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_pa_glow_02_02_dt15_ad |  | size |

### `par_d_slinn_00-4_sk01_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_pa_glow_02_02_dt15_ad |  | size |
| 1 | particlespriteemitter_10 | mesh(override mat) | fm_d_tree_01 | fx_c_pa_ring_06_ad |  | eflocationprimitivecylinderspin |
| 2 | particlespriteemitter_5 | mesh(override mat) | fm_d_tree_01 | fx_c_pa_ring_06_ad |  | eflocationprimitivecylinderspin |
| 3 | distortion | sprite |  | fx_o_pa_circledisort_01_01_ad |  | color |
| 4 | particlespriteemitter_11 | mesh(override mat) | fm_d_tree_01 | fx_c_pa_ring_06_ad |  | eflocationprimitivecylinderspin |
| 5 | particlespriteemitter_2 | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color |
| 6 | particlespriteemitter_3 | sprite |  | fx_d_pa_ringmaster_01_54_dt_ad | fx_f_aura_004_1, fx_m_caustic_001 | color |
| 7 | particlespriteemitter_8 | mesh(override mat) | fm_d_tree_01 | fx_c_pa_ring_06_ad |  | eflocationprimitivecylinderspin |
| 8 | particlespriteemitter_4 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_pa_glow_02_02_dt15_ad |  | size |

### `par_d_slinn_00-4_teleport_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_17 | mesh (disabled) | fm_d_gizmo_01 | fx_d_pa_glow_01_02_ad |  | color, colorscaleoverlife, locationprimitivesphere, meshrotationrate, size, sizemultiplylife, velocityoverlifetime |
| 1 | particlespriteemitter_2 | sprite (disabled) |  | fx_d_pa_shine_01_02_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 |  |
| 2 | ember02 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color |
| 3 | ember02 | sprite (disabled) |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | acceleration, color, locationprimitivesphere |
| 4 | particlespriteemitter_7 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | color |
| 5 | ice_twinkle | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | acceleration, color, locationprimitivesphere |
| 6 | particlespriteemitter_8 | sprite |  | fx_d_pa_glow_01_02_ad |  | color |
| 7 | particlespriteemitter_6 | sprite |  | fx_d_pa_shine_01_02_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 |  |

### `par_d_slinn_00-4_teleport_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | sprite |  | fx_d_me_master_01_011_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_atypical_042_ycl, fx_d_cloud_033 |  |
| 1 | ice_twinkle | mesh(override mat) | fm_d_sphere_003 | fx_d_pa_ring_07_76_ts_ad | fx_b_atypical_004, fx_c_ice_003, fx_d_atypical_031, fx_d_atypical_032, fx_d_atypical_050 | color, lifetime, size |

### `par_d_slinn_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_2 | mesh | fm_d_tree_04 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, size |

### `par_d_slinn_buff01_01s`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |
| 1 | particlespriteemitter_8 | sprite |  | fx_d_me_master_01_011_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_atypical_042_ycl, fx_d_cloud_033 |  |
| 2 | particlespriteemitter_5 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |
| 3 | particlespriteemitter_2 | sprite |  | fx_d_me_master_01_011_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_atypical_042_ycl, fx_d_cloud_033 |  |
| 4 | particlespriteemitter_6 | mesh(override mat) | fm_d_sphere_003 | fx_d_pa_ring_07_76_ts_ad | fx_b_atypical_004, fx_c_ice_003, fx_d_atypical_031, fx_d_atypical_032, fx_d_atypical_050 | color, parameterdynamic, size |
| 5 | particlespriteemitter_7 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |

### `par_d_slinn_buff01_02l`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | mesh | fm_d_tree_04 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, size |
| 1 | particlespriteemitter_8 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |
| 2 | particlespriteemitter_5 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |
| 3 | particlespriteemitter_9 | mesh(override mat) (disabled) | fm_d_sphere_003 | fx_d_pa_ring_07_76_ts_ad | fx_b_atypical_004, fx_c_ice_003, fx_d_atypical_031, fx_d_atypical_032, fx_d_atypical_050 | color, colorscaleoverlife, efvortex, locationprimitivesphere, meshrotationrate, size, sizemultiplylife |
| 4 | particlespriteemitter_6 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | eflocationprimitivecylinderspin |
| 5 | particlespriteemitter_7 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, parameterdynamic, size |
| 6 | particlespriteemitter_2 | mesh | fm_d_tree_04 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, size |

### `par_d_slinn_buff01_03e`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 1 | particlespriteemitter_8 | mesh(override mat) (disabled) | fm_d_helix_024 | fx_d_pa_ringmaster_01_13_ad | fx_d_shockwave_001_ycl, fx_f_aura_004_1, fx_k_fluidtile_01 | color, colorscaleoverlife, locationprimitivesphere, size |
| 2 | particlespriteemitter_5 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 3 | particlespriteemitter_1 | mesh(override mat) | fm_d_helix_024 | fx_d_pa_ringmaster_01_13_ad | fx_d_shockwave_001_ycl, fx_f_aura_004_1, fx_k_fluidtile_01 | color, colorscaleoverlife, locationprimitivecylinder, size |
| 4 | particlespriteemitter_6 | mesh(override mat) | bfm_leaf_002 | fx_d_me_flow_02_03_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_005_cl | color, parameterdynamic, size |
| 5 | particlespriteemitter_7 | mesh(override mat) | fm_d_sphere_001 | fx_d_me_master_01_060_ds_dt_ts_ad | fx_c_cloud_015, fx_d_atypical_055_ycl, fx_h_atypical_01_1, fx_j_mirnoise_01 | colorscaleoverlife, eflocationprimitivecylinderspin |

### `par_d_slinn_buff02_01s`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_22 | sprite |  | fx_d_pa_ring_07_35_tr | fx_b_atypical_004, fx_c_glow_001, fx_d_atypical_031, fx_d_shockwave_001_ycl | parameterdynamic |
| 1 | particlespriteemitter_23 | mesh(override mat) | fn_m_helix_005 | fx_d_pa_glow_01_01_dt_ad |  | color, meshrotation, parameterdynamic, size |
| 2 | particlespriteemitter_20 | sprite |  | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | parameterdynamic |
| 3 | particlespriteemitter_19 | mesh(override mat) (disabled) | fm_d_helix_024 | fx_d_pa_ringmaster_01_13_ad | fx_d_shockwave_001_ycl, fx_f_aura_004_1, fx_k_fluidtile_01 | color, colorscaleoverlife, locationprimitivecylinder, size |
| 4 | particlespriteemitter_24 | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_glow_01_01_dt_ad |  | color, eflocationcirclesurface, parameterdynamic, size, sizemultiplylife |
| 5 | tree-s | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | sizemultiplylife |
| 6 | particlespriteemitter_21 | sprite |  | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | parameterdynamic |

### `par_d_slinn_buff02_02l`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotation, meshrotationrate |
| 1 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotationrate |
| 2 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 3 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 4 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 6 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 7 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 8 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 9 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 10 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 11 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 12 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 13 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |

### `par_d_slinn_buff02_02l2`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_22 | mesh(override mat) | fm_d_tree_02 | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | color, colorscaleoverlife, parameterdynamic, size |
| 1 | particlespriteemitter_7 | sprite |  | fx_d_pa_shine_01_05_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color |
| 2 | particlespriteemitter_4 | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, locationprimitivecylinder, size, sizemultiplylife |
| 3 | particlespriteemitter_6 | efdecal |  | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | color, colorscaleoverlife |
| 4 | particlespriteemitter_8 | sprite |  | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | color, size, sizemultiplylife |
| 5 | particlespriteemitter_9 | mesh(override mat) | bfm_leaf_002 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | coloroverlife, parameterdynamic, size |
| 6 | particlespriteemitter_19 | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, locationprimitivecylinder, size, velocityoverlifetime |
| 7 | under_crash_01 | mesh(override mat) (disabled) | fm_d_ring_013 | fx_k_me_makeflow_01_06_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01, fx_k_electric_01 | coloroverlife, parameterdynamic, size |
| 8 | tree-l | mesh(override mat) | fm_m_sphere_004 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | lifetime, sizemultiplylife |
| 9 | particlespriteemitter_5 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, size |
| 10 | smoke_tail | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | parameterdynamic, size |
| 11 | ice_twinkle | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotation, sizemultiplylife |
| 12 | particlespriteemitter_25 | sprite |  | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, parameterdynamic, size_seeded, sizemultiplylife |
| 13 | particlespriteemitter_26 | sprite |  | fx_j_de_ring_01_ad |  | color, parameterdynamic, size |
| 14 | particlespriteemitter_3 | sprite |  | fx_j_de_ring_01_ad |  | color, parameterdynamic, size |

### `par_d_slinn_buff02_02l3`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 1 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 2 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, sizemultiplylife |
| 3 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic, size, sizemultiplylife |
| 4 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotation, sizemultiplylife |
| 5 | leaf-s | mesh(override mat) | fm_d_tree_02 | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic, size, sizemultiplylife |
| 6 | leaf-s | mesh(override mat) | fm_d_tree_02 | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic, sizemultiplylife |
| 7 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, sizemultiplylife |
| 8 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, sizemultiplylife |

### `par_d_slinn_buff02_03e`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_13 | sprite |  | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | parameterdynamic, size |
| 1 | particlespriteemitter_14 | sprite |  | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | size |
| 2 | particlespriteemitter_15 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 |  |
| 3 | particlespriteemitter_19 | mesh(override mat) | bfm_leaf_002 | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | color, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 4 | particlespriteemitter_21 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | color, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_16 | mesh(override mat) (disabled) | fm_d_helix_016 | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | color, colorscaleoverlife, efvortex, locationprimitivecylinder, size, sizemultiplylife |
| 6 | particlespriteemitter_27 | sprite |  | fx_d_pa_shine_01_05_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | colorscaleoverlife, parameterdynamic, size_seeded, sizemultiplylife |
| 7 | particlespriteemitter_11 | efdecal |  | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | color |
| 8 | particlespriteemitter_12 | sprite |  | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | parameterdynamic, size |
| 9 | particlespriteemitter_17 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | color, size |
| 10 | particlespriteemitter_22 | mesh | fm_d_tree_03 | fx_d_me_flow_02_03_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_005_cl | coloroverlife, parameterdynamic, size |
| 11 | note | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife |
| 12 | particlespriteemitter_28 | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_glow_01_01_dt_ad |  | color, eflocationcirclesurface, meshrotation, parameterdynamic, size, sizemultiplylife |
| 13 | particlespriteemitter_3 | mesh(override mat) (disabled) | fm_m_sphere_004 | fx_d_pa_glow_01_01_dt_ad |  | color, eflocationcirclesurface, meshrotation, parameterdynamic, size, sizemultiplylife |
| 14 | particlespriteemitter_23 | sprite |  | fx_d_me_flow_02_03_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_005_cl | color, colorscaleoverlife, locationprimitivecylinder, size, sizemultiplylife |

### `par_d_slinn_buff02_03e1`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | tree-s | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, locationprimitivecylinder, size, velocity, velocityoverlifetime |
| 1 | branch-start | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | colorscaleoverlife, sizemultiplylife |
| 2 | leaf-s | mesh(override mat) | fn_m_helix_005 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic |
| 3 | branch-start | mesh(override mat) | fn_m_helix_005 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic, size |
| 4 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotation, meshrotationrate |
| 5 | leaf-s | mesh(override mat) | fn_m_helix_005 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, parameterdynamic, size |
| 6 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotationrate |
| 7 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, meshrotationrate |

### `par_d_slinn_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_27 | sprite |  | fx_m_pa_ringaura_01_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_032, fx_i_noise_03 | color |
| 1 | glow_fill-l | sprite |  | fx_d_pa_turbulence_01_07_tr | fx_a_hit_001, fx_b_atypical_004, fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_noise_014_1, fx_d_uvturbulence_001, fx_i_noise_01 | color |
| 2 | particlespriteemitter_29 | sprite |  | fx_c_pa_flickline_01_1_ad | fx_f_star_001 |  |
| 3 | particlespriteemitter_30 | sprite |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 | color |
| 4 | particlespriteemitter_31 | sprite |  | bfx_j_pa_ring_07_08_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_j_mirnoise_02 |  |
| 5 | particlespriteemitter_32 | sprite |  | fx_c_pa_lensflare_01_01_ad | fx_c_glow_006 |  |
| 6 | particlespriteemitter_33 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 |  |
| 7 | particlespriteemitter_34 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 |  |
| 8 | particlespriteemitter_35 | mesh(override mat) (disabled) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, locationprimitivesphere, meshrotationrate, size, velocityoverlifetime |
| 9 | particlespriteemitter_36 | mesh | fm_n_slinn_tree_01 | fx_d_pa_ring_11_09_ts_tr |  | color, colorscaleoverlife, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, subuv |
| 10 | particlespriteemitter_1 | mesh (disabled) | fm_n_slinn_tree_01 | fx_d_pa_ring_11_09_ts_tr |  | color, colorscaleoverlife, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, subuv |

### `par_d_slinn_sk01_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_0 | sprite |  | fx_d_pa_ring_07_73_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_environ_038, fx_f_ray_001 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |

### `par_d_slinn_sk01_03_loc_int`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | glow1 | sprite |  | fx_d_pa_master_01_047_dt_ad | fx_c_cloud_015, fx_c_glow_010 | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_7 | sprite |  | bfx_i_pa_glow_01_ad |  | color, colorscaleoverlife |
| 2 | ice_twinkle | sprite |  | fx_d_pa_glow_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 3 | glow1 | sprite (disabled) |  | fx_d_pa_ring_07_75_ds_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031 | coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_5 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_0 | sprite |  | bfx_i_pa_glow_01_ad |  | color, colorscaleoverlife |
| 6 | glow1 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |

### `par_d_slinn_sk01_04`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 |  |
| 1 | ice_twinkle | sprite (disabled) |  | bfx_e_pa_ht_05_tr | fx_a_fragment_007, fx_e_trba |  |
| 2 | particlespriteemitter_8 | sprite |  | bfx_d_pa_smoke_ulit_01_09_dt_tr | fx_a_cloud_017, fx_d_noise_021, fx_i_noise_03 | coloroverlife |
| 3 | particlespriteemitter_13 | sprite (disabled) |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, velocity, velocityoverlifetime |
| 4 | particlespriteemitter_5 | sprite |  | bfx_d_pa_smoke_ulit_01_09_dt_tr | fx_a_cloud_017, fx_d_noise_021, fx_i_noise_03 | coloroverlife |
| 5 | particlespriteemitter_7 | sprite |  | bfx_d_pa_smoke_ulit_01_09_dt_tr | fx_a_cloud_017, fx_d_noise_021, fx_i_noise_03 | coloroverlife |
| 6 | glow1 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 |  |

### `par_n_slinn_battle_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | bf1 | mesh (disabled) | fm_n_slinn_tree_01 | bfx_d_pa_smoke_ulit_01_09_dt_tr | fx_a_cloud_017, fx_d_noise_021, fx_i_noise_03 | color, colorscaleoverlife, lifetime, locationprimitivecylinder, locationprimitivesphere, orbit, size, sizemultiplylife |
| 1 | tree-s | mesh | fm_n_slinn_tree_01 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 2 | tree-s | mesh | fm_n_slinn_tree_01 | bfx_d_pa_smoke_ulit_01_09_dt_tr | fx_a_cloud_017, fx_d_noise_021, fx_i_noise_03 | color, colorscaleoverlife, lifetime, locationprimitivecylinder, locationprimitivesphere, orbit, size, sizemultiplylife |
| 3 | tree-s | mesh | fm_n_slinn_tree_01 | bfx_g_pa_butterfly_01_tr | fx_a_noise_008_n, fx_g_butterfly_001_cl | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 4 | flame | sprite (disabled) |  | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 5 | tree-s | mesh(override mat) | fm_g_planecross_001 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |

### `par_n_slinn_battle_02_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | tree-s | mesh | fm_n_slinn_tree_01 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 1 | tree-s | mesh | fm_n_slinn_tree_01 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 2 | flame | sprite (disabled) |  | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 3 | tree-s | mesh | fm_n_slinn_tree_01 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 4 | particlespriteemitter_13 | sprite (disabled) |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, velocity, velocityoverlifetime |

### `par_n_slinn_battle_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | tree-s | mesh(override mat) | fm_m_helix_002 | fx_k_me_flowtrail_01_12_ts_tr | fx_a_atypical_002_cl, fx_c_line_004_ycl, fx_d_atypical_028, fx_j_ylinestream_01_ycl, fx_m_atypical_003, fx_m_flow_02_n | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 1 | tree-s | mesh | fm_n_slinn_tree_01 | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, meshmaterial, meshrotation_seeded, size |

### `par_n_slinn_battle_03_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_10 | sprite |  | fx_k_pa_ri_02_ad | fx_b_ring_006, fx_e_adba | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 1 | tree-s | mesh(override mat) | fm_m_sphere_006 | fx_k_pa_radialcolor_03_ad | fx_a_glow_05_cl, fx_a_noise_018, fx_d_atypical_031, fx_d_noise_009, fx_i_environment_001, fx_i_noise_04, fx_j_risingforce_01 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, velocity |

### `par_n_slinn_battle_cast_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | dist_concave | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_6 | sprite |  | fx_s_pa_glow_01_1_ad |  | color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 2 | wave_out_02 | sprite |  | fx_j_pa_smokes_1_tr | fx_a_glow_009, fx_j_bigsmokes_m_01 | cameraoffset, coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | particlespriteemitter_7 | sprite |  | fx_j_pa_smokes_1_tr | fx_a_glow_009, fx_j_bigsmokes_m_01 | cameraoffset, coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_11 | sprite |  | fx_j_pa_ring_11_30_ad | fx_d_noise_009, fx_i_shockwave_02, fx_j_mirnoise_02 | coloroverlife, lifetime, rotation, rotationrate, size, sizemultiplylife |
| 5 | particlespriteemitter_46 | sprite |  | fx_j_pa_simplesphere_01_10_tr | fx_b_atypical_004, fx_d_atypical_009, fx_d_noise_009, fx_f_note_001, fx_j_mirnoise_01 | coloroverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, velocity |
| 6 | particlespriteemitter_12 | sprite |  | fx_c_pa_dist_02_1_ad | fx_a_noise_002, fx_a_ring_001_cl, fx_b_glow_001_cl | color, colorscaleoverlife, lifetime, location, rotation_seeded, size, sizemultiplylife |
| 7 | particlespriteemitter_13 | sprite (disabled) |  | fx_j_circleshine_01_1_ad | fx_d_atypical_011 | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, size |
| 8 | ice_twinkle | sprite |  | fx_k_pa_circlenoise_01_tr | fx_b_atypical_004 | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_3 | sprite |  | bfx_i_pa_aurawave_001_ad | fx_a_tweb_001, fx_c_environ_008, fx_d_atypical_006_ycl, fx_d_noise_006 | acceleration, coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife, velocity |

### `par_n_slinn_battle_exp_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | smoke_tail | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 1 | particlespriteemitter_5 | sprite |  | fx_d_pa_atta_02_01_ts_tr | fx_d_hit_006_cl, fx_d_noise_014, fx_e_trba | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size |
| 2 | particlespriteemitter_13 | sprite |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, velocity, velocityoverlifetime |
| 3 | distortionfast | sprite |  | fx_d_pa_atta_02_01_ts_tr | fx_d_hit_006_cl, fx_d_noise_014, fx_e_trba | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size |
| 4 | decal | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | 22222 | mesh | fm_n_slinn_tree_01 | fx_c_pa_ring_03_ad | fx_a_noise_002, fx_c_ring_002 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 6 | particlespriteemitter_46 | sprite |  | bfx_i_pa_backglow_cl_02_tr |  | coloroverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 7 | particlespriteemitter_22 | sprite |  | fx_f_pa_gl_02_1_ad | fx_e_adba, fx_f_ring_001 | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 8 | wave_black | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_24 | sprite (disabled) |  | fx_k_pa_makeflow_02_09_tr | fx_b_ring_001, fx_d_atypical_031_cl, fx_d_noise_002, fx_i_atypical_03_1, fx_i_environment_001, fx_l_environment_001 | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity |
| 10 | particlespriteemitter_42 | sprite |  | fx_h_de_master_01_2_tr | fx_a_decal_008, fx_a_decal_008_n, fx_a_decal_009, fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_normal_016_1, fx_e_cloud_008, fx_e_noise_002 | coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 11 | particlespriteemitter_9 | sprite |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, parameterdynamic, size, velocity, velocityoverlifetime |
| 12 | particlespriteemitter_20 | sprite |  | fx_e_pa_fd_12_5_tr | fx_bg_softriver_02_n, fx_c_noise_001, fx_e_atypical_006 | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, size, sizemultiplylife, subuv, velocityoverlifetime |
| 13 | particlespriteemitter_44 | sprite |  | fx_m_me_watertrail_01_46_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |

### `par_n_slinn_battle_loop`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | flame | sprite (disabled) |  | fx_i_me_shield_01_tr | fx_a_cloud_022, fx_a_noise_002, fx_a_noise_013, fx_a_noise_014, fx_a_trail_007, fx_e_noise_002, fx_e_noise_006 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 1 | tree-s | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | color, lifetime, location, meshmaterial, meshrotation_seeded, size |
| 2 | particlespriteemitter_18 | sprite |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | cameraoffset, coloroverlife, lifetime, location, locationprimitivecylinder, rotation, size, sizemultiplylife, subuv |

### `par_n_slinn_battlearea_end`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_25 | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, parameterdynamic, size |
| 2 | particlespriteemitter_30 | sprite |  | fx_e_pa_fd_29_1_tr | fx_a_cloud_001, fx_e_normal, fx_e_trsi | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 3 | particlespriteemitter_6 | efdecal |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 4 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | color, colorscaleoverlife, eflocationcirclesurface, lifetime, parameterdynamic, size |
| 5 | particlespriteemitter_18 | sprite (disabled) |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | cameraoffset, coloroverlife, lifetime, location, locationprimitivecylinder, rotation, size, sizemultiplylife, subuv |
| 6 | particlespriteemitter_26 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 7 | particlespriteemitter_29 | sprite |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 8 | under_crash_01 | efdecal |  | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | coloroverlife, lifetime, size, sizemultiplylife |
| 9 | particlespriteemitter_3 | sprite |  | fx_j_de_ring_01_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 10 | particlespriteemitter_31 | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | acceleration, coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 11 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 12 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 13 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 14 | particlespriteemitter_5 | sprite |  | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 15 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 16 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 17 | particlespriteemitter_7 | sprite |  | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 18 | leaf-l | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 19 | particlespriteemitter_9 | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_n_slinn_battlearea_loop`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_25 | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, eflocationcirclesurface, lifetime, parameterdynamic, size |
| 2 | particlespriteemitter_30 | sprite |  | fx_e_pa_fd_29_1_tr | fx_a_cloud_001, fx_e_normal, fx_e_trsi | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 3 | particlespriteemitter_6 | efdecal |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 4 | particlespriteemitter_8 | sprite |  | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 5 | particlespriteemitter_9 | mesh(override mat) | bfm_leaf_002 | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 6 | under_crash_01 | efdecal |  | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | coloroverlife, lifetime, size, sizemultiplylife |
| 7 | particlespriteemitter_18 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 8 | particlespriteemitter_26 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 9 | particlespriteemitter_29 | sprite |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 10 | particlespriteemitter_19 | mesh(override mat) (disabled) | bfm_leaf_002 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 11 | particlespriteemitter_3 | sprite |  | fx_j_de_ring_01_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 12 | particlespriteemitter_31 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | acceleration, coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 13 | particlespriteemitter_32 | sprite (disabled) |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | cameraoffset, coloroverlife, lifetime, location, locationprimitivecylinder, rotation, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 14 | particlespriteemitter_34 | mesh(override mat) (disabled) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | cameraoffset, color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv |
| 15 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, lifetime, meshrotation, parameterdynamic, size |
| 16 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 17 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, lifetime, meshrotation, parameterdynamic, size |
| 18 | particlespriteemitter_5 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 19 | leaf-l | mesh(override mat) | fm_m_sphere_004 | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 20 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, lifetime, meshrotation, parameterdynamic, size |
| 21 | particlespriteemitter_7 | sprite |  | fx_d_pa_shine_01_05_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |

### `par_n_slinn_battlearea_start`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_25 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 1 | branch-start | mesh(override mat) | fm_a_cylinder_004 | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 2 | particlespriteemitter_30 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | acceleration, coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 3 | particlespriteemitter_6 | efdecal |  | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 4 | particlespriteemitter_8 | mesh(override mat) | bfm_leaf_002 | fx_o_beam_01_01_ad | fx_i_environment_001, fx_i_noise_04 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_9 | sprite |  | fx_o_beam_01_01_ad | fx_i_environment_001, fx_i_noise_04 | color, colorscaleoverlife, lifetime, location, rotation, size, velocity |
| 6 | under_crash_01 | efdecal |  | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | coloroverlife, lifetime, size, sizemultiplylife |
| 7 | particlespriteemitter_0 | sprite |  | fx_o_beam_01_01_ad | fx_i_environment_001, fx_i_noise_04 | color, colorscaleoverlife, lifetime, location, rotation, size, velocity |
| 8 | particlespriteemitter_26 | sprite |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 9 | particlespriteemitter_29 | sprite |  | fx_j_de_ring_01_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 10 | particlespriteemitter_19 | efdecal |  | fx_k_pa_ring_06_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_i_shockwave_01_ycl, fx_j_mirnoise_01 | coloroverlife, lifetime, size, sizemultiplylife |
| 11 | particlespriteemitter_3 | sprite |  | fx_j_de_ring_01_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 12 | particlespriteemitter_31 | sprite (disabled) |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | cameraoffset, coloroverlife, lifetime, location, locationprimitivecylinder, rotation, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 13 | particlespriteemitter_32 | mesh(override mat) (disabled) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | cameraoffset, color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv |
| 14 | particlespriteemitter_34 | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 15 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, size |
| 16 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 17 | branch-start | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 18 | particlespriteemitter_5 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 19 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 20 | leaf-l | sprite |  | fx_d_pa_shine_01_05_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 21 | particlespriteemitter_7 | sprite |  | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | color, colorscaleoverlife, lifetime, size, sizemultiplylife |

### `par_n_slinn_battlespawn_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | smoke_tail | sprite (disabled) |  | bfx_i_pa_glow_01_ad |  | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocity |
| 1 | particlespriteemitter_8 | sprite |  | fx_m_pa_shine_02_01_ad | fx_a_noise_009, fx_m_noise_003 | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife, velocity |
| 2 | 22222 | sprite |  | fx_k_pa_makeflow_02_09_tr | fx_b_ring_001, fx_d_atypical_031_cl, fx_d_noise_002, fx_i_atypical_03_1, fx_i_environment_001, fx_l_environment_001 | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_31 | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 4 | particlespriteemitter_21 | sprite |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | blackback | sprite |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 6 | circle_masked | sprite |  | fx_c_pa_glitter_01_tr | fx_a_atypical_003, fx_a_fire_003, fx_c_atypical_016, fx_c_noise_002, fx_d_noise_003, fx_d_noise_014 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | ring-start | sprite |  | fx_o_me_trail_02_22_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 8 | smoke_tail | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_0 | sprite |  | fx_m_pa_shine_02_01_ad | fx_a_noise_009, fx_m_noise_003 | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_16 | sprite |  | fx_o_de_master_01_15_tr | fx_c_decal_001, fx_c_decal_001_n, fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_normal_026_1, fx_d_normal_045_1, fx_e_cloud_008, fx_e_decal_002, fx_e_noise_002 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 11 | particlespriteemitter_1 | eflight (disabled) |  | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, location, size |
| 12 | particlespriteemitter_11 | sprite (disabled) |  | fx_c_pa_ring_06_ad |  | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv, velocity |
| 13 | particlespriteemitter_12 | sprite |  | fx_o_de_master_01_15_tr | fx_c_decal_001, fx_c_decal_001_n, fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_environ_001, fx_d_normal_026_1, fx_d_normal_045_1, fx_e_cloud_008, fx_e_decal_002, fx_e_noise_002 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 14 | particlespriteemitter_19 | sprite |  | fx_o_me_trail_02_22_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 15 | particlespriteemitter_28 | sprite |  | fx_o_me_trail_02_22_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 16 | particlespriteemitter_2 | sprite |  | fx_o_me_trail_02_22_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 17 | decal | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, velocity |

### `par_n_slinn_battlespawn_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | sprite |  | fx_m_pa_shine_02_01_ad | fx_a_noise_009, fx_m_noise_003 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 1 | bb | sprite |  | fx_m_pa_shine_02_01_ad | fx_a_noise_009, fx_m_noise_003 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |

### `par_n_slinn_cast_01_05`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_25 | mesh(override mat) | fm_b_halfcylinder_001 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 1 | bb | sprite |  | fx_d_me_flow_02_03_ad | fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_005_cl | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 2 | bb | mesh(override mat) | fm_b_halfcylinder_001 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_28 | sprite (disabled) |  | fx_m_pa_worldoffset_02_39_tr | fx_a_blankwhite_01, fx_d_noise_002, fx_j_mirnoise_01, fx_k_electile_02, fx_m_noise_001, fx_m_ring_001, fx_m_spatter_001_xyclamp | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, rotation, size, sizemultiplylife, subuv, velocity |
| 4 | particlespriteemitter_11 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, orientationaxislock, parameterdynamic, size, sizemultiplylife, velocity |

### `par_n_slinn_exp_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_92 | sprite |  | fx_y_me_customlfresnel_02_tr | fx_b_atypical_004, fx_c_noise_001_n, fx_i_noise_04, fx_j_mirnoise_02 | acceleration, coloroverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 1 | particlespriteemitter_7 | mesh(override mat) | fm_h_sphere_01_1 | fx_c_pa_vsmoke_01_tr334 | fx_a_cloud_016, fx_c_cloud_002_cl, fx_k_bubblentile_01, fx_k_cloudtilie_01 | color, colorscaleoverlife, lifetime, location, parameterdynamic, size |
| 2 | particlespriteemitter_86 | sprite |  | fx_k_me_flowtrail_01_14_tr_ts | fx_c_line_004_ycl, fx_d_atypical_011, fx_d_atypical_028, fx_i_shockwave_02, fx_m_atypical_003, fx_m_flow_02_n | cameraoffset, coloroverlife, eflocationcirclesurface, eflocationonground, efvelocityoverlifetime, lifetime, rotation, rotationrate, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_12 | sprite (disabled) |  | fx_e_pa_fd_04_1_tr | fx_e_fluid_007, fx_e_noise_008, fx_e_trdi | color, colorscaleoverlife, lifetime, locationprimitivecylinder, orbit, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 4 | particlespriteemitter_22 | efdecal |  | fx_n_worldposfade_me_tr_04 | fx_a_noise_014, fx_a_rainripple_n_001, fx_k_turtlediff_02, fx_k_turtlespec_01 | coloroverlife, lifetime, rotation, size |
| 5 | under_crash_01 | efdecal |  | fx_n_worldposfade_me_tr_04 | fx_a_noise_014, fx_a_rainripple_n_001, fx_k_turtlediff_02, fx_k_turtlespec_01 | coloroverlife, lifetime, rotation, size |
| 6 | particlespriteemitter_13 | sprite |  | fx_o_pa_master_01_31_tr | fx_a_ice_002, fx_a_noise_002, fx_c_cloud_015, fx_d_atypical_098, fx_d_fluid_007 | cameraoffset, color, colorscaleoverlife, eflocationonground, efvelocityoverlifetime, lifetime, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_16 | sprite |  | fx_n_de_crack_03_15_tr | fx_c_decal_002_1, fx_c_decal_002_2, fx_c_decal_002_3, fx_c_decal_002_n, fx_d_atypical_031, fx_d_decal_033, fx_d_environ_001, fx_d_normal_018, fx_d_normal_018_1, fx_e_cloud_008, fx_e_noise_002, fx_k_turtlespec_01 | acceleration, coloroverlife, efvelocityoverlifetime, lifetime, location, locationprimitivecylinder, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv, velocity |
| 8 | particlespriteemitter_87 | mesh(override mat) | fm_n_highpolygon_01 | fx_n_worldposfade_me_tr_09 | fx_a_noise_014, fx_a_rainripple_n_001, fx_k_turtlediff_02, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 9 | particlespriteemitter_89 | mesh(override mat) | fm_n_highpolygon_01 | fx_n_worldposfade_me_tr_09 | fx_a_noise_014, fx_a_rainripple_n_001, fx_k_turtlediff_02, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 10 | particlespriteemitter_91 | mesh(override mat) | fm_n_highpolygon_01 | fx_e_pa_cd_17_tr | fx_c_cloud_001_cl, fx_c_cloud_012, fx_e_cloud_004_cl, fx_e_noise_003 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size |
| 11 | aa | sprite |  | fx_o_pa_master_01_31_tr | fx_a_ice_002, fx_a_noise_002, fx_c_cloud_015, fx_d_atypical_098, fx_d_fluid_007 | cameraoffset, color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |

### `par_n_slinn_expcast_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_74 | sprite |  | fx_d_pa_glow_02_02_dt15_ad |  | acceleration, cameraoffset, coloroverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv, velocity |
| 1 | particlespriteemitter_61 | sprite |  | fx_o_pa_master_01_31_tr | fx_a_ice_002, fx_a_noise_002, fx_c_cloud_015, fx_d_atypical_098, fx_d_fluid_007 | cameraoffset, color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_10 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, eflocationprimitivecylinderspin, lifetime, location, meshrotation, parameterdynamic, size |

### `par_s_slinn_bless_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_9 | mesh(override mat) (disabled) | fm_d_helix_026 | fx_d_pa_glow_01_05_dt_ad |  | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 1 | particlespriteemitter_11 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, eflocationprimitivecylinderspin, lifetime, location, meshrotation, parameterdynamic, size |
| 2 | particlespriteemitter_12 | sprite (disabled) |  | fx_h_pa_shine_03_1_ad | fx_d_noise_030 | cameraoffset, color, colorscaleoverlife, eflocationprimitivesphere, efvelocityoverlifetime, lifetime, location, size, sizemultiplylife, velocity |
| 3 | particlespriteemitter_3 | sprite |  | fx_d_pa_shine_01_11_dt5_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_5 | mesh(override mat) (disabled) | fm_d_helix_026 | fx_d_pa_glow_02_02_dt15_ad |  | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, orbit, size, sizemultiplylife, subuv, velocityoverlifetime |
| 5 | particlespriteemitter_8 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, eflocationprimitivecylinderspin, lifetime, location, meshrotation, parameterdynamic, size |
| 6 | note | sprite (disabled) |  | fx_g_pa_ninjaflow_01_06_tr | fx_b_atypical_004, fx_e_atypical_008, fx_g_rainbowmosaic_01, fx_j_flowsmoke_01_cl, fx_j_smoke_01_cl, fx_k_flowmask_01_d, fx_k_flowmask_01_v | color, colorscaleoverlife, lifetime, location, locationprimitivesphere, size, sizemultiplylife, velocity, velocityoverlifetime |
| 7 | particlespriteemitter_6 | sprite |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |
| 8 | particlespriteemitter_13 | sprite |  | fx_d_me_master_01_224_ts_ad | fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_fluid_060_ycl, fx_m_atypical_002_ycl, fx_m_flow_03_n | color, colorscaleoverlife, lifetime, orientationaxislock, size, sizemultiplylife, velocityoverlifetime |
| 9 | particlespriteemitter_14 | sprite |  | fx_d_pa_ringmaster_01_31_dt_ad | fx_f_aura_004_1, fx_i_noise_03, fx_j_mirnoise_01 | color, colorscaleoverlife, lifetime, orientationaxislock, size, sizemultiplylife, velocityoverlifetime |
| 10 | particlespriteemitter_2 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |

### `par_s_slinn_bless_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | parents | sprite |  | fx_s_pa_ringmaster_11_216_dt_ad | fx_b_atypical_004, fx_d_noise_045, fx_d_normal_085, fx_f_aura_004_1 | eflocationonground, eftypedatadecal, eventgenerator, lifetime |
| 1 | particlespriteemitter_29 | sprite |  | fx_d_me_master_01_224_ts_ad | fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_fluid_060_ycl, fx_m_atypical_002_ycl, fx_m_flow_03_n | acceleration, cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife |
| 2 | distortion | sprite (disabled) |  |  |  | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | test11 | mesh | fm_d_tree_02 | bfx_c_pa_lightflare_01_ddt_4_ad | fx_c_glow_009, fx_c_glow_010 | color, colorscaleoverlife, lifetime, location_seeded, meshrotation, parameterdynamic, size, sizemultiplylife, velocity |
| 4 | aa | mesh(override mat) | fn_m_helix_005 | fx_d_pa_flare_03_ad |  | color, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size |
| 5 | particlespriteemitter_33 | sprite (disabled) |  | fx_d_pa_ring_11_10_ad | fx_d_atypical_043 | color, colorscaleoverlife, lifetime, location, rotation, size, sizemultiplylife |

### `par_s_slinn_bless_03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_13 | sprite |  | fx_d_pa_turbulence_01_101_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_fluid_070_cl, fx_d_fluid_071_cl, fx_d_uvturbulence_001, fx_d_uvturbulence_004, fx_e_noise_002, fx_i_noise_01 | acceleration, cameraoffset, color, colorscaleoverlife, eventreceiverspawn, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocityoverlifetime |
| 1 | particlespriteemitter_39 | mesh(override mat) | fm_d_hemisphere_009_2 | fx_s_pa_ringmaster_01_300_dt_tr | fx_b_atypical_004, fx_d_normal_085, fx_f_aura_004_1, fx_i_shockwave_02_ycl, fx_m_noise_003 | color, colorscaleoverlife, eflocationonground, eventreceiverspawn, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_8 | mesh(override mat) | fm_d_helix_015 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eventreceiverspawn, lifetime, location, meshrotation, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_1 | sprite (disabled) |  | fx_d_pa_ringmaster_01_222_dt_tr | fx_f_aura_004_1 | color, colorscaleoverlife, eventreceiverspawn, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_10 | sprite |  | fx_d_pa_glow_02_03_ad |  | cameraoffset, color, colorscaleoverlife, eventreceiverspawn, lifetime, location, size, sizemultiplylife |
| 5 | particlespriteemitter_40 | sprite |  | fx_s_me_ringmaster_01_109_tr | fx_d_environ_114_ycl, fx_f_aura_004_1, fx_k_electile_02 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 6 | particlespriteemitter_20 | sprite (disabled) |  | fx_d_pa_glow_02_03_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 7 | particlespriteemitter_4 | sprite (disabled) |  | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eventreceiverspawn, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 8 | ring-start | sprite |  | fx_m_pa_spritewave_01_7_ad | fx_a_line_005, fx_d_noise_030, fx_k_auraline_05_ycl, fx_m_noise_008, fx_m_trail_004_cl | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 9 | particlespriteemitter_11 | sprite |  | fx_d_pa_flar_01_02_ad | fx_b_atypical_004 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 10 | crack | sprite |  | fx_d_pa_turbulence_01_101_dt_tr | fx_c_atypical_017, fx_d_atypical_082, fx_d_fluid_026, fx_d_fluid_070_cl, fx_d_fluid_071_cl, fx_d_uvturbulence_001, fx_d_uvturbulence_004, fx_e_noise_002, fx_i_noise_01 | acceleration, cameraoffset, color, colorscaleoverlife, eventreceiverspawn, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocityoverlifetime |
| 11 | particlespriteemitter_2 | sprite |  | fx_s_pa_ring_07_77_ds_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031 | color, colorscaleoverlife, eventreceiverspawn, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 12 | particlespriteemitter_14 | sprite |  | fx_s_pa_ring_07_77_ds_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031 | color, colorscaleoverlife, eventreceiverspawn, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 13 | particlespriteemitter_26 | mesh(override mat) | fm_d_hemisphere_009_2 | fx_s_pa_ringmaster_01_300_dt_tr | fx_b_atypical_004, fx_d_normal_085, fx_f_aura_004_1, fx_i_shockwave_02_ycl, fx_m_noise_003 | color, colorscaleoverlife, eflocationonground, eventreceiverspawn, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |

### `par_s_slinn_bless_04`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_20 | sprite |  | fx_c_pa_glitter_01_tr | fx_a_atypical_003, fx_a_fire_003, fx_c_atypical_016, fx_c_noise_002, fx_d_noise_003, fx_d_noise_014 | cameraoffset, coloroverlife, lifetime, location, parameterdynamic, size, sizemultiplylife, velocity |
| 1 | smoke_tail | sprite |  | fx_j_me_ringrainbow_01_2_ts_tr | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_2_cl, fx_d_noise_003, fx_d_noise_009, fx_e_line_011_1_ycl | cameraoffset, color_seeded, colorscaleoverlife, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, rotationrate, size, sizemultiplylife |
| 2 | particlespriteemitter_22 | mesh(override mat) | fm_d_cracklight_002 | fx_j_rgbsplit_01_1_ad | fx_f_note_001, fx_f_star_002 | cameraoffset, color, colorscaleoverlife, lifetime, locationdirect, meshrotation_seeded, meshrotationrate, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_23 | sprite (disabled) |  | fx_a_pa_gl_01_2_ad | fx_a_glow_003, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, subuv |
| 4 | particlespriteemitter_0 | sprite |  | fx_o_pa_circledisort_01_01_ad |  | cameraoffset, coloroverlife, lifetime, orientationaxislock, rotation, size |

### `par_s_slinn_bless_05`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | smoke_tail | mesh(override mat) | fm_d_cracklight_002 | fx_j_me_shine_02_1_ad | fx_b_atypical_004, fx_d_atypical_028, fx_d_noise_033 | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation_seeded, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_2 | sprite |  | fx_j_me_shine_02_1_ad | fx_b_atypical_004, fx_d_atypical_028, fx_d_noise_033 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size |
| 2 | distortion | sprite |  | bfx_i_pa_backglow_cl_02_tr |  | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 3 | smoke_tail | mesh(override mat) | fm_d_cracklight_002 | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation_seeded, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 4 | bb | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, coloroverlife, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, size, sizemultiplylife, velocity |
| 5 | smoke_tail | sprite |  | fx_j_me_shine_02_1_ad | fx_b_atypical_004, fx_d_atypical_028, fx_d_noise_033 | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity_seeded |
| 6 | 1324 | sprite |  | fx_m_pa_shine_02_01_ad | fx_a_noise_009, fx_m_noise_003 | color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, size, sizemultiplylife, subuv, velocity_seeded |
| 7 | smoke_tail | mesh(override mat) | fm_d_cracklight_002 | fx_j_rgbsplit_01_1_ad | fx_f_note_001, fx_f_star_002 | cameraoffset, color, colorscaleoverlife, lifetime, location, meshrotation_seeded, meshrotationratemultiplylife, parameterdynamic, size, sizemultiplylife |
| 8 | particlespriteemitter_4 | sprite (disabled) |  | fx_j_me_shine_02_1_ad | fx_b_atypical_004, fx_d_atypical_028, fx_d_noise_033 | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder, parameterdynamic, size, sizemultiplylife, velocity_seeded |
| 9 | base | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |

### `par_s_slinn_hand_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_41 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | ice_twinkle | sprite (disabled) |  | fx_o_pa_rampshape_01_20_ad | fx_d_atypical_043, fx_j_flowsmoke_01_cl, fx_o_magicsymbol_01 | acceleration, coloroverlife, eflocationcirclesurface, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 2 | ice_twinkle | sprite |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime, location, locationprimitivesphere, rotation, size, sizemultiplylife |
| 3 | particlespriteemitter_43 | sprite (disabled) |  | fx_d_pa_ring_07_73_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_environ_038, fx_f_ray_001 | coloroverlife, colorscaleoverlife, lifetime, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife, subuv |
| 4 | particlespriteemitter_44 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |

### `par_s_slinn_kz_start_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | glow | sprite (disabled) |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | coloroverlife, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_6 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | color_seeded, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size |
| 2 | particlespriteemitter_61 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | color_seeded, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size |
| 3 | particlespriteemitter_70 | sprite (disabled) |  | fx_m_pa_worldoffset_02_11_tr | fx_a_blankwhite_01, fx_b_atypical_006, fx_d_noise_002, fx_e_ring_023_1, fx_j_mirnoise_01, fx_j_mirnoise_02, fx_m_noise_001, fx_m_noise_003, fx_m_spatter_001_xyclamp | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocity |
| 4 | particlespriteemitter_62 | sprite |  | bfx_d_pa_circ_01_02_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, orientationaxislock, parameterdynamic, rotation_seeded, size_seeded, sizemultiplylife |
| 5 | particlespriteemitter_58 | sprite (disabled) |  | fx_s_pa_ringmaster_01_185_ad | fx_f_aura_004_1, fx_i_noise_04, fx_j_environment_tile_02 | color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, size |
| 6 | ice_twinkle | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | cameraoffset, coloroverlife, lifetime, locationprimitivecylinder, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 7 | leaf-l | mesh(override mat) | fm_d_tree_02 | fx_s_me_master_01_059_gpu_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, parameterdynamic, size |
| 8 | particlespriteemitter_55 | sprite (disabled) |  | fx_t_pa_ringmaster_01_02_ad | fx_f_aura_004_1, fx_m_noise_008 | cameraoffset, coloroverlife, colorscaleoverlife, eflocationonground, lifetime, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 9 | particlespriteemitter_57 | sprite (disabled) |  | fx_d_pa_ringmaster_01_228_ad | fx_b_atypical_004, fx_d_atypical_082_cl, fx_d_noise_045, fx_d_normal_085, fx_f_aura_004_1, fx_i_noise_03, fx_j_risingcolor_01 | color_seeded, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size |
| 10 | ring-start | sprite |  | fx_d_pa_ringmaster_01_34_dt_ad | fx_f_aura_004_1, fx_j_mirnoise_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 11 | particlespriteemitter_68 | mesh(override mat) | fm_d_tree_02 | fx_f_pa_ri_01_3_ad | fx_c_glow_003, fx_f_star_002 | color, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_93 | sprite |  | fx_d_pa_atta_10_06_ad | fx_d_cloud_027, fx_d_noise_014, fx_e_adba | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 13 | ground | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, size, sizemultiplylife, velocity |
| 14 | particlespriteemitter_95 | sprite |  | fx_g_pa_ninjaflow_01_06_tr | fx_b_atypical_004, fx_e_atypical_008, fx_g_rainbowmosaic_01, fx_j_flowsmoke_01_cl, fx_j_smoke_01_cl, fx_k_flowmask_01_d, fx_k_flowmask_01_v | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivecylinder_seeded, parameterdynamic, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 15 | particlespriteemitter_7 | sprite |  | fx_m_pa_worldoffset_02_11_tr | fx_a_blankwhite_01, fx_b_atypical_006, fx_d_noise_002, fx_e_ring_023_1, fx_j_mirnoise_01, fx_j_mirnoise_02, fx_m_noise_001, fx_m_noise_003, fx_m_spatter_001_xyclamp | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, orbit, size, sizemultiplylife, subuv, velocity |
| 16 | particlespriteemitter_71 | sprite (disabled) |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 17 | ring-start | sprite |  | fx_m_pa_worldoffset_02_11_tr | fx_a_blankwhite_01, fx_b_atypical_006, fx_d_noise_002, fx_e_ring_023_1, fx_j_mirnoise_01, fx_j_mirnoise_02, fx_m_noise_001, fx_m_noise_003, fx_m_spatter_001_xyclamp | color, colorscaleoverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 18 | particlespriteemitter_78 | sprite (disabled) |  | fx_s_me_master_01_059_gpu_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 19 | particlespriteemitter_105 | mesh(override mat) (disabled) | fm_d_sphere_003 | fx_d_de_circle_01_ad |  | color, colorscaleoverlife, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocityoverlifetime |
| 20 | leaf-l | mesh(override mat) | fm_d_tree_02 | fx_e_pa_ht_18_1_tr | fx_a_noise_005, fx_a_noise_011, fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, parameterdynamic, size |
| 21 | particlespriteemitter_92 | sprite |  | fx_o_pa_ringmaster_01_10_ts_ad | fx_c_line_004_ycl, fx_e_line_008, fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, locationprimitivecylinder, size, sizemultiplylife, velocity |
| 22 | ice_twinkle | sprite (disabled) |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | acceleration, coloroverlife, eflocationcirclesurface, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 23 | particlespriteemitter_10 | mesh | fm_d_tree_04 | bfx_d_pa_circ_02_04_ad | fx_c_glow_001, fx_d_atypical_072 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 24 | particlespriteemitter_104 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | coloroverlife, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 25 | particlespriteemitter_96 | sprite (disabled) |  | fx_s_me_master_01_059_gpu_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 26 | leaf-l | mesh(override mat) | bfm_leaf_002 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationcirclesurface, eflocationonground, lifetime, meshrotation, parameterdynamic, size |
| 27 | under_crash_01 | efdecal |  | fx_m_pa_shorkwave_01_30_tr | fx_a_environ_003_n, fx_c_noise_009, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl, fx_m_trail_002 | coloroverlife, lifetime, size |

### `par_s_slinn_kzbuff_01e`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | ring-start | mesh(override mat) | fm_d_ring_013 | fx_s_pa_ringmaster_11_216_dt_ad | fx_b_atypical_004, fx_d_noise_045, fx_d_normal_085, fx_f_aura_004_1 | color, parameterdynamic, size |
| 1 | particlespriteemitter_16 | sprite |  | fx_w_me_floorstrm_21_ad | fx_c_cloud_015, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_q_noise_002, fx_w_atypical_004 | color, size |
| 2 | ring-start | sprite |  | fx_w_me_floorstrm_21_ad | fx_c_cloud_015, fx_i_shockwave_02_ycl, fx_j_mirnoise_01, fx_q_noise_002, fx_w_atypical_004 | color, size |
| 3 | particlespriteemitter_17 | mesh(override mat) | fm_d_sphere_003 | fx_d_pa_ring_07_76_ts_ad | fx_b_atypical_004, fx_c_ice_003, fx_d_atypical_031, fx_d_atypical_032, fx_d_atypical_050 | color, meshrotation, size, sizemultiplylife |
| 4 | particlespriteemitter_11 | mesh | fm_d_tree_04 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | colorscaleoverlife, eflocationprimitivecylinderspin |
| 5 | particlespriteemitter_18 | sprite |  | fx_d_me_master_01_011_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_atypical_042_ycl, fx_d_cloud_033 |  |
| 6 | particlespriteemitter_4 | sprite |  | fx_s_pa_ringmaster_11_216_dt_ad | fx_b_atypical_004, fx_d_noise_045, fx_d_normal_085, fx_f_aura_004_1 | color, location, size |
| 7 | particlespriteemitter_12 | mesh(override mat) | fm_d_sphere_001 | fx_d_pa_ring_11_12_dt100_tr |  | colorscaleoverlife, eflocationprimitivecylinderspin |
| 8 | particlespriteemitter_13 | sprite |  | bfx_d_pa_circ_02_04_ad | fx_c_glow_001, fx_d_atypical_072 | color, parameterdynamic, sizemultiplylife |
| 9 | lighttone_out | sprite |  | fx_o_me_floorstrm_02_ad | fx_a_line_010_ycl, fx_c_cloud_015, fx_j_mirnoise_01 | color, location, size |

### `par_s_slinn_kzbuff_01l`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_12 | mesh(override mat) | fm_d_helix_015_1 | fx_d_pa_ring_07_75_ds_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031 | color, parameterdynamic, size |
| 1 | ice_twinkle | sprite |  | fx_d_me_master_01_011_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_atypical_042_ycl, fx_d_cloud_033 | coloroverlife, lifetime, locationprimitivesphere, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_19 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 |  |
| 3 | particlespriteemitter_1 | sprite |  | bfx_d_pa_circ_01_04_fs_dt_ad |  | color, parameterdynamic, sizemultiplylife |
| 4 | particlespriteemitter_5 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | eflocationprimitivecylinderspin |
| 5 | lighttone_out | mesh(override mat) | fm_d_sphere_001 | fx_o_me_floorstrm_02_ad | fx_a_line_010_ycl, fx_c_cloud_015, fx_j_mirnoise_01 | color, meshrotation, size, sizemultiplylife |
| 6 | particlespriteemitter_9 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife |
| 7 | particlespriteemitter_6 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | eflocationprimitivecylinderspin |
| 8 | particlespriteemitter_8 | mesh(override mat) | fm_d_ring_013 | fx_d_pa_ring_11_12_dt100_tr |  | color, colorscaleoverlife, efvortex, locationprimitivesphere, meshrotationrate, size, sizemultiplylife |
| 9 | particlespriteemitter_28 | mesh | fm_d_tree_04 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, size |
| 10 | particlespriteemitter_14 | sprite |  | fx_d_pa_ring_07_76_ts_ad | fx_b_atypical_004, fx_c_ice_003, fx_d_atypical_031, fx_d_atypical_032, fx_d_atypical_050 | color, colorscaleoverlife, parameterdynamic, size, sizemultiplylife |
| 11 | particlespriteemitter_43 | sprite |  | fx_s_pa_ringmaster_01_300_dt_tr | fx_b_atypical_004, fx_d_normal_085, fx_f_aura_004_1, fx_i_shockwave_02_ycl, fx_m_noise_003 | color |
| 12 | particlespriteemitter_2 | sprite |  | fx_a_pa_db_01_2_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | size |
| 13 | particlespriteemitter_3 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | eflocationprimitivecylinderspin |

### `par_s_slinn_kzbuff_02e`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_44 | sprite |  | fx_t_pa_ringmaster_01_02_ad | fx_f_aura_004_1, fx_m_noise_008 | color, sizemultiplylife |
| 1 | particlespriteemitter_47 | sprite |  | bfx_d_pa_circ_01_04_fs_dt_ad |  | lifetime_seeded, parameterdynamic |
| 2 | particlespriteemitter_48 | sprite |  | fx_e_pa_shield_01_tr |  | coloroverlife |
| 3 | particlespriteemitter_0 | sprite |  | fx_s_pa_ringmaster_01_300_dt_tr | fx_b_atypical_004, fx_d_normal_085, fx_f_aura_004_1, fx_i_shockwave_02_ycl, fx_m_noise_003 | size |

### `par_s_slinn_kzbuff_02l`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | field | sprite |  | fx_t_pa_ringmaster_01_02_ad | fx_f_aura_004_1, fx_m_noise_008 | color, sizemultiplylife |
| 1 | particlespriteemitter_36 | sprite |  | fx_d_pa_glow_02_04_dt5_ad |  | location, size |
| 2 | particlespriteemitter_12 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | lifetime_seeded, parameterdynamic |
| 3 | particlespriteemitter_40 | sprite |  | fx_m_pa_shorkwave_01_32_tr | fx_a_environ_003_n, fx_i_noise_03, fx_m_atypical_002_ycl, fx_m_flow_04_n, fx_m_noise_001, fx_m_trail_002 |  |
| 4 | particlespriteemitter_30 | sprite |  | fx_d_pa_fire_01_10_tr | fx_a_fragment_007, fx_e_normal, fx_e_trsi | color |
| 5 | particlespriteemitter_34 | sprite |  | bfx_d_pa_circ_01_09_dt_ad |  | acceleration, color, colorscaleoverlife, efvortex, location, locationprimitivecylinder |
| 6 | ring-start | sprite |  | fx_d_pa_flare_02_101_ts_dt5_ad | fx_d_atypical_009, fx_k_caustictile_01 |  |

### `par_s_slinn_kzheal_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_41 | sprite |  | fx_e_pa_gl_07_1_ad |  |  |
| 1 | ring_in | sprite |  | bfx_i_pa_glow_01_ad |  | color, locationprimitivesphere |
| 2 | ring_in | sprite |  | fx_d_pa_glow_02_02_dt15_ad |  |  |
| 3 | particlespriteemitter_2 | sprite |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_54 | sprite |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |

### `par_s_slinn_kzheal_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_11 | sprite (disabled) |  | fx_m_pa_ap_01_2_tr | fx_d_atypical_028, fx_d_atypical_060_cl | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_49 | sprite (disabled) |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 2 | particlespriteemitter_3 | sprite (disabled) |  | fx_d_pa_ringmaster_01_54_dt_ad | fx_f_aura_004_1, fx_m_caustic_001 | color, colorscaleoverlife, lifetime, location, orientationaxislock, rotation, size, sizemultiplylife |
| 3 | particlespriteemitter_50 | sprite (disabled) |  | fx_d_pa_ring_11_10_ad | fx_d_atypical_043 | cameraoffset, coloroverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_45 | sprite |  | fx_r_me_master_19_06_ts_dt_fs_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_060_1_cl, fx_d_atypical_060_cl, fx_d_normal_085, fx_m_caustic_001 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 5 | 1 | mesh(override mat) | fm_d_helix_024 | fx_d_pa_glow_02_07_dt100_ad |  | color, colorscaleoverlife, lifetime, location, meshrotation_seeded, parameterdynamic, size_seeded, sizemultiplylife |
| 6 | particlespriteemitter_5 | sprite |  | fx_d_pa_ring_11_10_ad | fx_d_atypical_043 | cameraoffset, coloroverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife, velocity |
| 7 | particlespriteemitter_47 | sprite |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size |
| 8 | distortion | sprite (disabled) |  | fx_g_pa_ninjaflow_01_06_tr | fx_b_atypical_004, fx_e_atypical_008, fx_g_rainbowmosaic_01, fx_j_flowsmoke_01_cl, fx_j_smoke_01_cl, fx_k_flowmask_01_d, fx_k_flowmask_01_v | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |

### `par_s_slinn_teleport_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_42 | sprite (disabled) |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | color, colorscaleoverlife, lifetime, locationprimitivesphere, orbit, size, sizemultiplylife |
| 1 | particlespriteemitter_8 | sprite |  | fx_j_pa_circledisort_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife |
| 2 | particlespriteemitter_45 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | acceleration, coloroverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | particlespriteemitter_41 | sprite |  | fx_d_pa_glow_02_03_ad |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_7 | sprite |  | fx_d_pa_master_01_046_dt_ad | fx_c_cloud_015, fx_m_flare_001 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 5 | particlespriteemitter_0 | sprite |  | fx_j_pa_circledisort_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | distort | sprite |  | fx_c_pa_shockwave_01_1_ad | fx_c_cloud_015, fx_c_noise_002, fx_d_noise_004_1, fx_d_noise_021 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, size, velocity |
| 7 | particlespriteemitter_16 | sprite |  | fx_d_pa_glow_02_03_ad |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 8 | ice_twinkle | sprite |  | fx_d_pa_master_01_046_dt_ad | fx_c_cloud_015, fx_m_flare_001 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |

### `par_s_slinn_teleport_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_26 | sprite (disabled) |  | fx_d_pa_master_01_047_dt_ad | fx_c_cloud_015, fx_c_glow_010 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | particlespriteemitter_23 | sprite |  | fx_d_pa_dark_05_02_tr |  | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 2 | black | sprite (disabled) |  | fx_d_pa_master_01_046_dt_ad | fx_c_cloud_015, fx_m_flare_001 | color, colorscaleoverlife, lifetime, size |
| 3 | particlespriteemitter_27 | sprite (disabled) |  | fx_d_me_flow_02_08_ad | fx_a_noise_013, fx_b_atypical_004, fx_d_atypical_076_cl, fx_d_fluid_032_1_cl, fx_d_hit_006_1_cl, fx_d_noise_009 | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_0 | mesh(override mat) | fm_b_sphere_001 | fx_d_pa_ring_12_ad_inst35 |  | color, colorscaleoverlife, lifetime, location, meshrotationratemultiplylife, orbit, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_54 | mesh(override mat) | fm_b_sphere_001 | fx_z_me_hexashield_01_2_ad | fx_b_atypical_012, fx_d_atypical_043, fx_d_noise_033, fx_h_wave_02, fx_i_noise_04, fx_j_hexagone_01 | color, lifetime, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 6 | particlespriteemitter_22 | sprite |  | fx_k_pa_glow_01_01_ad_dt |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 7 | distortion | sprite (disabled) |  | fx_d_pa_flar_01_03_dt_ad | fx_b_atypical_004 | cameraoffset, coloroverlife, lifetime, parameterdynamic, size, sizemultiplylife |

### `par_x_slinn_lpda_shield_02_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_4 | sprite |  | fx_d_pa_ringmaster_01_21_tr | fx_d_atypical_076_1_cl, fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 1 | particlespriteemitter_15 | sprite |  | fx_z_me_hexashield_01_2_ad | fx_b_atypical_012, fx_d_atypical_043, fx_d_noise_033, fx_h_wave_02, fx_i_noise_04, fx_j_hexagone_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 2 | particlespriteemitter_17 | mesh(override mat) | fm_m_sphere_004 | defaultparticle |  | color, colorscaleoverlife, lifetime, location, meshrotationratemultiplylife, orbit, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_8 | sprite |  | fx_f_pa_ring_01_5_tr | fx_d_atypical_008, fx_d_atypical_016, fx_d_atypical_028, fx_d_noise_005, fx_d_noise_009, fx_d_noise_014 | color, colorscaleoverlife, lifetime, parameterdynamic, size |
| 4 | particlespriteemitter_26 | sprite |  | fx_c_pa_ri_04_2_tr |  | color, colorscaleoverlife, lifetime, orbit, parameterdynamic, size, sizemultiplylife |
| 5 | down_big_rock_01 | mesh(override mat) | fm_m_sphere_004 | fx_d_me_crackspace_01_tr | fx_d_symbol_066 | acceleration, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 6 | particlespriteemitter_27 | eflight (disabled) |  | fx_d_pa_ringmaster_01_21_tr | fx_d_atypical_076_1_cl, fx_f_aura_004_1 | color, coloroverlife, lifetime, location, size |
| 7 | particlespriteemitter_11 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | cameraoffset, coloroverlife, lifetime, location, orbit, orientationaxislock, size, sizemultiplylife |
| 8 | particlespriteemitter_30 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 9 | particlespriteemitter_6 | sprite |  | fx_d_pa_ringmaster_01_21_tr | fx_d_atypical_076_1_cl, fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 10 | particlespriteemitter_14 | sprite |  | fx_d_pa_ring_12_ad_inst35 |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 11 | particlespriteemitter_25 | sprite |  | fx_f_pa_ring_01_5_tr | fx_d_atypical_008, fx_d_atypical_016, fx_d_atypical_028, fx_d_noise_005, fx_d_noise_009, fx_d_noise_014 | color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_2 | sprite |  | fx_j_pa_ring_07_90_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_032, fx_d_noise_014, fx_j_mirnoise_01 | cameraoffset, coloroverlife, lifetime, location, orbit, orientationaxislock, size, sizemultiplylife |
| 13 | particlespriteemitter_10 | sprite |  | fx_d_me_crackspace_01_tr | fx_d_symbol_066 | color, colorscaleoverlife, lifetime, orbit, parameterdynamic, size, sizemultiplylife |
| 14 | particlespriteemitter_28 | sprite |  | fx_e_pa_gl_01_1_ad | fx_e_adba, fx_e_glow_001 | color, colorscaleoverlife, lifetime, orbit, parameterdynamic, size, sizemultiplylife |
| 15 | glow02 | sprite |  | fx_c_pa_ri_04_2_tr |  | color, colorscaleoverlife, lifetime, orientationaxislock, size, sizemultiplylife |
| 16 | particlespriteemitter_39 | sprite |  | fx_d_pa_ringmaster_01_17_tr | fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, orbit, parameterdynamic, size |

### `par_x_slinn_lpda_shield_carck_02_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_4 | mesh(override mat) | fm_m_sphere_004 | fx_x_me_break_02_01_ts_ad | fx_d_noise_037, fx_d_normal_084_1, fx_e_addi | color, lifetime, meshrotation, orbit, parameterdynamic, size |
| 1 | particlespriteemitter_42 | mesh(override mat) (disabled) | fm_a_broken_011 | fx_x_me_break_03_01_ts_tr | fx_d_noise_037, fx_d_normal_084_1, fx_e_trdi | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_9 | mesh(override mat) | fm_m_sphere_004 | fx_x_me_break_03_01_ts_tr | fx_d_noise_037, fx_d_normal_084_1, fx_e_trdi | color, colorscaleoverlife, lifetime, meshrotation, orbit, parameterdynamic, size |
| 3 | particlespriteemitter_19 | mesh(override mat) | fm_m_sphere_004 | fx_x_me_break_03_01_ts_tr | fx_d_noise_037, fx_d_normal_084_1, fx_e_trdi | color, colorscaleoverlife, lifetime, meshrotation, orbit, parameterdynamic, size |
| 4 | particlespriteemitter_40 | mesh(override mat) (disabled) | fm_a_sphere_002 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_35 | mesh(override mat) (disabled) | fm_a_sphere_002 | fx_d_me_crackspace_01_tr | fx_d_symbol_066 | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 6 | down_big_rock_01 | mesh(override mat) (disabled) | fm_m_sphere_004 | fx_d_me_crackspace_01_tr | fx_d_symbol_066 | acceleration, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 7 | down_big_rock_01 | mesh(override mat) (disabled) | fm_m_sphere_004 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | acceleration, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 8 | down_big_rock_01 | mesh(override mat) (disabled) | fm_m_sphere_004 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | acceleration, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 9 | down_big_rock_01 | mesh(override mat) | fm_a_sphere_002 | fx_d_me_crackspace_01_tr | fx_d_symbol_066 | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |

### `par_x_slinn_lpda_shield_carck_02_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_44 | mesh (disabled) | fm_d_tree_03 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | color, colorscaleoverlife, lifetime, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_45 | mesh(override mat) | fm_m_sphere_006 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, lifetime, size, sizemultiplylife |
| 2 | tree-l | efdecal |  | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | color, colorscaleoverlife, lifetime, size |
| 3 | particlespriteemitter_20 | mesh(override mat) | fm_a_sphere_002 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, lifetime, meshrotation, orbit, parameterdynamic, size |
| 4 | particlespriteemitter_43 | mesh(override mat) (disabled) | fm_a_sphere_002 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, colorscaleoverlife, lifetime, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |

### `par_x_slinn_lpda_tree_02_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_22 | efdecal (disabled) |  | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | color, colorscaleoverlife, lifetime, size |
| 1 | tree-l | mesh(override mat) | bfm_leaf_002 | fx_c_pa_aura_02_tr | fx_a_cloud_026, fx_a_glow_009 | color, lifetime, size, sizemultiplylife |

### `par_x_slinn_lpda_tree_02_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_22 | efdecal (disabled) |  | fx_k_pa_shockwave_17_ad | fx_a_line_005, fx_c_cloud_015, fx_c_noise_002, fx_d_noise_009, fx_i_noise_03 | color, colorscaleoverlife, lifetime, size |
| 1 | particlespriteemitter_2 | efdecal |  | fx_k_pa_shockwave_17_ad | fx_a_line_005, fx_c_cloud_015, fx_c_noise_002, fx_d_noise_009, fx_i_noise_03 | color, colorscaleoverlife, lifetime, size |
| 2 | particlespriteemitter_1 | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 3 | ring_up | sprite |  | fx_m_me_watertrail_01_46_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_9 | sprite |  | fx_k_pa_backglow_cl_02_tr_dt |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 5 | ring_in | sprite (disabled) |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_7 | sprite |  | fx_d_pa_glow_01_01_dt_ad |  | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 7 | black | sprite |  | fx_e_pa_ht_01_1_tr | fx_e_atypical_011, fx_e_trba | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 8 | particlespriteemitter_3 | sprite |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |

### `par_z_battle_08`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_38 | sprite |  | fx_a_pa_ht_01_2_ad | fx_a_fragment_002, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 1 | particlespriteemitter_39 | sprite |  | fx_a_pa_gl_01_4_ad | fx_a_hit_008, fx_e_adba | coloroverlife, lifetime, location, locationprimitivesphere, size, sizemultiplylife, velocityoverlifetime |
| 2 | particlespriteemitter_40 | sprite |  | fx_o_pa_ri_04_ad_2s | fx_e_adba, fx_e_atypical_031 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, rotation, size |
| 3 | particlespriteemitter_41 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 4 | particlespriteemitter_42 | sprite |  | fx_e_pa_ht_12_3_tr | fx_e_hit_009r, fx_e_trdi | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_43 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, coloroverlife, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | ice_twinkle | sprite |  | fx_d_pa_flare_03_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 7 | note | sprite |  | fx_j_de_ring_01_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 8 | particlespriteemitter_19 | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |

### `par_z_slinn_buff01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_25 | sprite |  | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | cameraoffset, color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 1 | particlespriteemitter_8 | sprite |  | fx_j_de_ring_01_ad |  | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 2 | under_crash_01 | efdecal (disabled) |  | fx_d_pa_ringmaster_01_04_dt_ad | fx_f_aura_004_1, fx_k_fluidtile_01 | coloroverlife, lifetime, size |
| 3 | under_crash_01 | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 4 | tree-l | mesh(override mat) | fm_m_sphere_004 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, lifetime, size, sizemultiplylife |
| 5 | particlespriteemitter_6 | efdecal |  | fx_d_pa_shine_01_05_dt_ad | fx_d_noise_009, fx_d_noise_014, fx_d_noise_021 | color, colorscaleoverlife, lifetime, size |
| 6 | particlespriteemitter_26 | mesh | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity |
| 7 | particlespriteemitter_4 | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, efvortex, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity |

### `par_z_slinn_buff01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_1 | mesh(override mat) | fm_m_sphere_004 | fx_d_me_master_01_133_dt_fn_ts_ds_ad | fx_b_atypical_004, fx_c_cloud_015, fx_d_atypical_055_ycl, fx_d_normal_085, fx_d_shockwave_002_2_ycl | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 1 | smoke_tail | sprite |  | fx_k_me_makeflow_01_06_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01, fx_k_electric_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 2 | particlespriteemitter_7 | sprite (disabled) |  | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, efvelocityoverlifetime, lifetime, location, rotation, size, sizemultiplylife, subuv |
| 3 | particlespriteemitter_1 | mesh | fm_d_tree_03 | fx_d_me_master_01_060_ma | fx_a_environ_003, fx_m_noise_003, itr_00511_d, itr_00511_n | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 4 | particlespriteemitter_26 | sprite |  | fx_m_me_trail_02_28_tr | fx_a_cloud_022, fx_bg_dustpanner_01, fx_j_mirnoise_01, fx_j_mirnoise_02, fx_m_atypical_004_loc_int, fx_m_wave_001_ycl | cameraoffset, color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 5 | particlespriteemitter_3 | sprite |  | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 6 | particlespriteemitter_10 | mesh(override mat) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | color, colorscaleoverlife, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 7 | particlespriteemitter_25 | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 8 | particlespriteemitter_5 | sprite (disabled) |  | fx_k_me_makeflow_01_06_tr | fx_d_noise_006, fx_i_atypical_03_1_ycl, fx_i_environment_001, fx_j_mirnoise_01, fx_k_electric_01 | cameraoffset, color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 9 | particlespriteemitter_11 | mesh(override mat) | fm_b_sphere_001 | fx_z_me_hexashield_01_2_ad | fx_b_atypical_012, fx_d_atypical_043, fx_d_noise_033, fx_h_wave_02, fx_i_noise_04, fx_j_hexagone_01 | color, colorscaleoverlife, lifetime, locationprimitivecylinder, meshrotation, meshrotationrate, size, sizemultiplylife, subuv, velocity |
| 10 | particlespriteemitter_14 | mesh(override mat) | fm_d_ring_013 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |
| 11 | particlespriteemitter_2 | mesh(override mat) (disabled) | fm_d_ring_013 | fx_d_pa_ringmaster_01_17_dt_ad | fx_f_aura_004_1, fx_m_noise_003 | coloroverlife, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size |

### `par_z_slinn_buff01_02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_22 | efdecal (disabled) |  | fx_k_pa_shockwave_17_ad | fx_a_line_005, fx_c_cloud_015, fx_c_noise_002, fx_d_noise_009, fx_i_noise_03 | color, colorscaleoverlife, lifetime, size |
| 1 | tree-s | mesh(override mat) | fm_a_sphere_002 | fx_f_pa_ring_01_4_tr | fx_d_atypical_008, fx_d_atypical_016, fx_d_atypical_028, fx_d_noise_005, fx_d_noise_009, fx_d_noise_014 | color, lifetime, size, sizemultiplylife |
| 2 | tree-l | mesh (disabled) | fm_d_tree_03 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 3 | particlespriteemitter_2 | efdecal |  | fx_k_pa_shockwave_17_ad | fx_a_line_005, fx_c_cloud_015, fx_c_noise_002, fx_d_noise_009, fx_i_noise_03 | color, colorscaleoverlife, lifetime, size |
| 4 | ring_up | sprite |  | fx_m_me_watertrail_01_46_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife |
| 5 | particlespriteemitter_9 | sprite |  | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 6 | ring_in | sprite (disabled) |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | particlespriteemitter_7 | sprite |  | fx_d_pa_glow_01_01_dt_ad |  | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |
| 8 | particlespriteemitter_1 | mesh(override mat) | fm_a_sphere_002 | fx_d_pa_ring_12_ad_inst35 |  | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 9 | particlespriteemitter_3 | sprite |  | fx_c_pa_ring_06_ad |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size, sizemultiplylife |

### `par_z_slinn_buff02`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_30 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | coloroverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 1 | particlespriteemitter_14 | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 2 | particlespriteemitter_15 | sprite |  | fx_z_me_hexashield_01_2_ad | fx_b_atypical_012, fx_d_atypical_043, fx_d_noise_033, fx_h_wave_02, fx_i_noise_04, fx_j_hexagone_01 | color, colorscaleoverlife, eflocationonground, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 3 | particlespriteemitter_17 | mesh(override mat) | fm_d_tree_02 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, colorscaleoverlife, lifetime, location, meshrotationratemultiplylife, orbit, orientationaxislock, parameterdynamic, size, sizemultiplylife |
| 4 | particlespriteemitter_26 | sprite |  | fx_c_pa_ri_04_2_tr |  | color, colorscaleoverlife, eflocationonground, lifetime, location, orbit, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_27 | efparticlemodulevelocityoverlifetime (disabled) |  | fx_d_pa_ringmaster_01_21_tr | fx_d_atypical_076_1_cl, fx_f_aura_004_1 | color, coloroverlife, lifetime, location, size |
| 6 | particlespriteemitter_29 | sprite |  |  |  |  |
| 7 | particlespriteemitter_14 | sprite (disabled) |  | fx_j_pa_ring_07_90_ad | fx_b_atypical_004, fx_d_atypical_031, fx_d_atypical_032, fx_d_noise_014, fx_j_mirnoise_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 8 | particlespriteemitter_20 | mesh(override mat) | fm_e_halfsphere_001 | fx_z_me_break_01_01_ts_ad | fx_d_fragment_015_cl, fx_e_addi, fx_i_noise_05, fx_m_noise_001 | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 9 | particlespriteemitter_24 | mesh(override mat) | fm_b_sphere_001 | defaultparticle |  | color, colorscaleoverlife, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |
| 10 | particlespriteemitter_11 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | cameraoffset, coloroverlife, eflocationonground, lifetime, location, orbit, orientationaxislock, size, sizemultiplylife |
| 11 | particlespriteemitter_25 | sprite |  | fx_f_pa_ring_01_5_tr | fx_d_atypical_008, fx_d_atypical_016, fx_d_atypical_028, fx_d_noise_005, fx_d_noise_009, fx_d_noise_014 | color, colorscaleoverlife, eflocationonground, lifetime, location, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_28 | sprite (disabled) |  | fx_e_pa_gl_01_1_ad | fx_e_adba, fx_e_glow_001 | color, colorscaleoverlife, eflocationonground, lifetime, location, orbit, parameterdynamic, size, sizemultiplylife |
| 13 | glow02 | sprite (disabled) |  | fx_d_pa_ringmaster_01_17_tr | fx_f_aura_004_1 | color, colorscaleoverlife, lifetime, orientationaxislock, size, sizemultiplylife |
| 14 | particlespriteemitter_39 | sprite |  | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |

### `par_z_slinn_buff02_03e`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |

### `par_z_slinn_buff03`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | leaf-l | mesh(override mat) | fm_d_tree_01 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 1 | branch-start | mesh(override mat) | fm_d_tree_01 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 2 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 3 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 4 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 6 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 7 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 8 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 9 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 10 | leaf-s | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size, sizemultiplylife |
| 11 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 12 | leaf-l | mesh(override mat) | fm_d_tree_01 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 13 | branch-start | mesh(override mat) | fm_d_tree_02 | fx_d_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |

### `par_z_slinn_buff03_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | branch-start | mesh(override mat) | fm_d_ring_008 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 1 | branch-start | mesh(override mat) | fm_d_ring_008 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 2 | ring-start | sprite |  | fx_k_me_makeflow_02_31_tr | fx_d_noise_002, fx_e_noise_001, fx_i_atypical_03_1, fx_k_auraline_02, fx_l_environment_001, fx_m_caustic_001, fx_m_wave_001, fx_m_wave_001_ycl | color, colorscaleoverlife, lifetime, location, size |
| 3 | leaf-l | mesh(override mat) | fm_d_tree_01 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |
| 4 | leaf-l | mesh(override mat) | fm_d_tree_01 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eflocationprimitivesphere, lifetime, meshrotation, size |
| 5 | leaf-l | mesh(override mat) | fm_d_tree_01 | bfx_d_pa_circ_01_01_dt_ad |  | color, colorscaleoverlife, eflocationonground, eflocationprimitivesphere, lifetime, meshrotation, size |
| 6 | branch-start | mesh(override mat) | fm_d_ring_008 | fx_d_me_master_01_059_ma | bg_rhd_tree_mapleleaf02_da_ksy, bg_rhd_tree_mapleleaf02_n_ksy, bg_rhd_tree_mapleleaf02_s_ksy, fx_a_environ_003 | color, eflocationonground, eflocationprimitivecylinderspin, lifetime, meshrotation, size |

### `par_z_slinn_buff04`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_33 | sprite |  | fx_d_me_master_01_018_ad | fx_a_atypical_019_1, fx_c_cloud_015, fx_d_environ_035, fx_i_environment_002 | cameraoffset, coloroverlife, lifetime, location, orientationaxislock, size, sizemultiplylife |
| 1 | particlespriteemitter_34 | mesh(override mat) (disabled) | fm_m_sphere_006 | fx_d_pa_atta_12_11_dt_ad | fx_e_adba, fx_f_ring_001 | color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife |
| 2 | particlespriteemitter_35 | sprite |  | fx_o_pa_makeflow_02_12_tr | fx_b_ring_001, fx_d_atypical_031_cl, fx_d_noise_002, fx_i_atypical_03_1, fx_i_environment_001, fx_k_auraline_02, fx_l_environment_001 | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, size_seeded, sizemultiplylife |
| 3 | particlespriteemitter_37 | sprite (disabled) |  | fx_m_me_watertrail_01_46_tr | fx_a_fluid_017_n, fx_d_noise_014_1, fx_d_noise_030, fx_m_fluid_004, fx_m_wave_001_ycl, t_cubemap_01_tex | coloroverlife, lifetime, location, orientationaxislock, parameterdynamic, rotation, size |
| 4 | particlespriteemitter_38 | mesh(override mat) | fm_a_stone_004 | fx_b_pa_cd_02_tr | fx_a_atypical_002_cl, fx_b_atypical_006, fx_b_cloud_016, fx_b_cloud_019 | color, colorscaleoverlife, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | particlespriteemitter_39 | sprite (disabled) |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, color, colorscaleoverlife, eflocationcirclesurface, lifetime, location, parameterdynamic, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 6 | ring-start | sprite |  | fx_d_pa_dark_05_02_tr |  | color, colorscaleoverlife, lifetime, location, orientationaxislock, size |
| 7 | blackback | sprite |  | fx_h_pa_ap_01_1_tr | fx_c_noise_005, fx_e_fluid_026, fx_e_trdi, fx_h_hit_01, fx_h_wave_04 | color, colorscaleoverlife, lifetime, location, size |
| 8 | smoke_tail | mesh(override mat) | fm_d_ring_008 | fx_a_pa_gl_01_3_ad | fx_a_glow_002, fx_e_adba | color_seeded, colorscaleoverlife, eflocationprimitivecylinderspin, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 9 | particlespriteemitter_49 | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, colorscaleoverlife, lifetime, location, size |
| 10 | particlespriteemitter_43 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, parameterdynamic, size_seeded, sizemultiplylife, velocityoverlifetime |
| 11 | ice_twinkle | sprite |  | fx_j_me_icesurfacee_01 | fx_b_atypical_004 | acceleration, color, colorscaleoverlife, eflocationcirclesurface, eflocationprimitivecylinderspin, lifetime, rotation, size, sizemultiplylife, velocity |
| 12 | down_big_rock_01 | mesh(override mat) (disabled) | bfm_leaf_002 | bfx_d_me_leaf_01_01_ts_tr | fx_e_tral, fx_g_leaf_08 | acceleration, coloroverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, meshrotation, meshrotationrate, size, sizemultiplylife, velocity |
| 13 | particlespriteemitter_48 | mesh(override mat) | fm_d_helix_018 | bfx_i_pa_glow_01_ad |  | acceleration, color, colorscaleoverlife, lifetime, locationprimitivesphere, meshrotation, meshrotationrate_seeded, meshrotationrateoverlife, parameterdynamic, size, sizemultiplylife, velocity, velocityoverlifetime |
| 14 | ggg | sprite |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | color, colorscaleoverlife, lifetime, location, size |

### `par_z_slinn_hand_cast_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_14 | sprite |  | fx_n_me_spritewave_01_22_tr | fx_a_line_005, fx_d_noise_030, fx_h_noise_001, fx_k_caustictile_01, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_008 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 1 | particlespriteemitter_26 | sprite |  | fx_e_pa_fd_29_1_tr | fx_a_cloud_001, fx_e_normal, fx_e_trsi | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, parameterdynamic, size |
| 2 | particlespriteemitter_22 | mesh(override mat) | fm_h_lighting_01_1 | fx_o_pa_shorkwave_01_17_tr | fx_a_environ_003_n, fx_d_noise_003, fx_f_electric_005, fx_i_noise_03, fx_m_atypical_002_ycl, fx_m_atypical_n_001 | coloroverlife, colorscaleoverlife, lifetime, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 3 | card_01 | sprite |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | color, colorscaleoverlife, eflocationonground, lifetime, parameterdynamic, rotation_seeded, size, sizemultiplylife |
| 4 | 2 | mesh(override mat) | fm_d_helix_016_1 | fx_m_pa_smoke_01_12_tr | fx_d_normal_009, fx_d_symbol_020_cl, fx_i_noise_03, fx_j_mirnoise_01, fx_m_atypical_001_cl, fx_m_atypical_n_001 | cameraoffset, color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife, subuv |
| 5 | particlespriteemitter_34 | mesh(override mat) | fm_d_tree_01 | fx_k_pa_ring_05_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031, fx_j_mirnoise_01, fx_m_wave_001 | color, colorscaleoverlife, lifetime_seeded, location, meshrotation, parameterdynamic, size |
| 6 | particlespriteemitter_38 | sprite |  | fx_m_pa_wave_01_2_ad | fx_c_noise_005, fx_d_noise_006, fx_j_mirnoise_01, fx_m_caustic_001 | color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 7 | particlespriteemitter_48 | sprite |  | fx_e_pa_gl_01_2_tr | fx_e_glow_001, fx_e_trli | color, colorscaleoverlife, lifetime_seeded, location, orientationaxislock, parameterdynamic, rotation, size_seeded, sizemultiplylife |

### `par_z_slinn_ice_loop`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | branch-start | mesh(override mat) | fm_z_tree_01 | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, lifetime, location, meshrotation, parameterdynamic, size, sizemultiplylife |
| 1 | particlespriteemitter_30 | sprite |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | acceleration, coloroverlife, colorscaleoverlife, lifetime, locationprimitivecylinder, rotation, size, sizemultiplylife, velocity, velocityoverlifetime |
| 2 | particlespriteemitter_31 | sprite |  | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | cameraoffset, coloroverlife, lifetime, location, locationprimitivecylinder_seeded, rotation, size, sizemultiplylife, subuv, velocity, velocityoverlifetime |
| 3 | particlespriteemitter_32 | mesh(override mat) | fm_d_tree_01 | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, lifetime, locationprimitivecylinder, meshrotation, parameterdynamic, size, sizemultiplylife |
| 4 | branch-start | mesh(override mat) | fm_d_tree_01 | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, lifetime, locationprimitivecylinder, meshrotation, parameterdynamic, size, sizemultiplylife |
| 5 | branch-start | mesh(override mat) | fx_p_issa_wood_01 | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, lifetime, locationprimitivecylinder, meshrotation, parameterdynamic, size, sizemultiplylife |
| 6 | branch-start | eflight |  | fx_z_me_master_01_058_ma | fx_a_blankwhite_01, fx_a_environ_003, fx_d_atypical_114_cl, fx_k_turtlediff_02, fx_k_turtlenorm_01, fx_k_turtlespec_01 | color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 7 | particlespriteemitter_6 | mesh(override mat) | fx_p_issa_wood_01 | fx_d_de_unlit_01_04_tr | fx_a_decal_013 | color, lifetime, location, meshrotation, meshrotationrateoverlife, parameterdynamic, size, sizemultiplylife |
| 8 | branch-start | mesh(override mat) | fm_a_cylinder_004 | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | color, lifetime, location, meshrotation, orbit, parameterdynamic, size, sizemultiplylife |

### `par_z_slinn_ice_tree_loop`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | branch-start | sprite |  | fx_m_me_trail_02_24_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |

### `par_z_slinn_ice_tree_loop_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_18 | sprite |  | fx_m_me_trail_02_24_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |

### `par_z_slinn_portal_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | particlespriteemitter_3 | sprite (disabled) |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, rotationrate, size |
| 1 | particlespriteemitter_0 | sprite |  | fx_e_pa_fd_07_2_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 2 | glowl | sprite (disabled) |  | fx_d_pa_atta_12_11_dt_ad | fx_e_adba, fx_f_ring_001 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 3 | ring-start | sprite |  | bfx_f_pa_ringpo_01_ad | fx_b_atypical_004, fx_c_noise_002 | cameraoffset, color, colorscaleoverlife, lifetime, location, size, sizemultiplylife |
| 4 | glowl | sprite |  | fx_d_pa_ring_11_09_ts_tr |  | cameraoffset, coloroverlife, lifetime, locationprimitivesphere, size, sizemultiplylife |
| 5 | particlespriteemitter_9 | sprite |  | fx_d_pa_ring_11_09_ts_tr |  | cameraoffset, coloroverlife, lifetime, locationprimitivesphere, size, sizemultiplylife |
| 6 | particlespriteemitter_20 | mesh(override mat) | fm_h_lighting_01_1 | fx_d_pa_atta_09_02_ad | fx_d_atypical_034_cl, fx_e_adba | coloroverlife, colorscaleoverlife, lifetime_seeded, location, meshrotation_seeded, parameterdynamic, size, sizemultiplylife |
| 7 | particlespriteemitter_19 | sprite (disabled) |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, locationprimitivesphere, size, sizemultiplylife, subuv |
| 8 | particlespriteemitter_21 | mesh(override mat) (disabled) | fm_a_cylinder_004 | bfx_j_pa_lightdust_01_1_tr | fx_a_atypical_018, fx_d_fragment_003, fx_e_trex | cameraoffset, color, colorscaleoverlife, lifetime, meshrotation, parameterdynamic, size, sizemultiplylife, subuv |
| 9 | dust | sprite (disabled) |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv |
| 10 | ring_in | sprite |  | fx_z_bogt_portal_01_01_tr | fx_c_noise_007, fx_c_noise_008, fx_d_atypical_009, fx_d_atypical_043, fx_d_atypical_094_ycl, fx_d_atypical_095, fx_d_noise_009, fx_f_aura_004_1, fx_i_environment_001, fx_i_noise_03 | cameraoffset, color, lifetime, location, size, sizemultiplylife |
| 11 | particlespriteemitter_27 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotation, size |
| 12 | particlespriteemitter_28 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, rotationrate, size |

### `par_z_slinn_sk01_01`

| # | Emitter | 종류 | Mesh | Material | Textures | 주요 모듈 |
|---|---|---|---|---|---|---|
| 0 | dust | sprite (disabled) |  | fx_m_pa_shorkwave_01_8_tr | fx_a_environ_003_n, fx_a_noise_009, fx_d_atypical_002_1_ycl, fx_d_noise_003, fx_i_noise_03, fx_m_atypical_002_ycl | color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, orientationaxislock, size, sizemultiplylife, subuv, velocity_seeded |
| 1 | ice_twinkle | sprite (disabled) |  | fx_d_pa_sqc_02_01_tr | fx_d_cloud_005 | acceleration, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere, required, rotation, sizemultiplylife, subuv |
| 2 | 22222 | sprite |  | fx_j_circleshine_01_1_ad | fx_d_atypical_011 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 3 | ice_twinkle | sprite (disabled) |  | bfx_i_pa_backglow_cl_02_tr |  | coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 4 | particlespriteemitter_46 | sprite (disabled) |  | fx_a_pa_db_01_1_ad | fx_a_fragment_007, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, efvelocityoverlifetime, lifetime, location, locationprimitivesphere_seeded, parameterdynamic, rotation, rotationrate, size, sizemultiplylife, subuv |
| 5 | par_d_esther_link_inanna_01 | sprite |  |  |  |  |
| 6 | glow1 | sprite |  | fx_d_pa_glow_01_01_dt_ad |  | color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 7 | b | sprite |  | fx_m_me_trail_02_4_tr | fx_a_cloud_007, fx_a_cloud_022, fx_d_noise_003, fx_d_noise_030, fx_m_atypical_004_loc_int, fx_m_noise_001 | color_seeded, eventgenerator, lifetime, locationprimitivesphere, particlespriteemitter, rotation_seeded, size, sizemultiplylife, typedatamesh |
| 8 | particlespriteemitter_13 | sprite |  | fx_m_me_trail_02_4_tr | fx_a_cloud_007, fx_a_cloud_022, fx_d_noise_003, fx_d_noise_030, fx_m_atypical_004_loc_int, fx_m_noise_001 | color_seeded, eventgenerator, lifetime, locationprimitivesphere, particlespriteemitter, rotation_seeded, size, sizemultiplylife, typedatamesh |
| 9 | particlespriteemitter_14 | sprite |  | fx_d_pa_master_01_047_dt_ad | fx_c_cloud_015, fx_c_glow_010 | cameraoffset, coloroverlife, efvortex, lifetime, locationprimitivesphere, parameterdynamic, rotation, size, sizemultiplylife, velocity |
| 10 | glow1 | sprite |  | fx_e_pa_fd_07_1_ad | fx_d_noise_003, fx_e_atypical_005_cl, fx_e_fluid_006 | cameraoffset, color, colorscaleoverlife, lifetime, size, sizemultiplylife |
| 11 | particlespriteemitter_48 | sprite |  | bfx_d_pa_circ_01_01_dt_ad |  | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 12 | particlespriteemitter_5 | sprite |  | fx_d_pa_ring_07_75_ds_ad | fx_a_noise_008_n, fx_b_atypical_004, fx_d_atypical_031 | cameraoffset, color, colorscaleoverlife, lifetime, location, parameterdynamic, rotationrate, size |
| 13 | ring-start | sprite |  | fx_j_circleshine_01_1_ad | fx_d_atypical_011 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 14 | particlespriteemitter_52 | sprite |  | bfx_d_pa_flar_02_01_ad | fx_d_atypical_009 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 15 | particlespriteemitter_53 | sprite (disabled) |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 16 | particlespriteemitter_54 | sprite (disabled) |  | fx_c_pa_lensflare_01_05_ad | fx_c_glow_006, fx_c_glow_008 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, size, sizemultiplylife |
| 17 | particlespriteemitter_55 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | cameraoffset, coloroverlife, lifetime, location, size, sizemultiplylife |
| 18 | particlespriteemitter_56 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, rotation, rotationratemultiplylife, size, sizemultiplylife |
| 19 | line0 | sprite |  | fx_a_pa_gl_01_9_ad | fx_a_glow_004, fx_e_adsi, fx_e_normal | cameraoffset, color, colorscaleoverlife, lifetime, rotation, rotationratemultiplylife, size, sizemultiplylife |
| 20 | line0 | sprite |  | fx_m_pa_spritewave_01_48_tr | fx_a_line_005, fx_d_atypical_043, fx_d_noise_030, fx_m_atypical_013_yclamp, fx_m_noise_008 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, rotationratemultiplylife, size, sizemultiplylife |
| 21 | particlespriteemitter_59 | sprite |  | fx_d_pa_atta_05_07_ad | fx_e_adli, fx_f_ring_001 | cameraoffset, color, colorscaleoverlife, lifetime, parameterdynamic, rotation, size, sizemultiplylife |
| 22 | particlespriteemitter_60 | sprite |  | fx_f_pa_ht_02_2_ad | fx_e_adba, fx_e_ring_001 | cameraoffset, color, colorscaleoverlife, lifetime, rotation, size, sizemultiplylife |
| 23 | distortion | sprite (disabled) |  | fx_h_me_lightingdetail_01_1_ad | fx_i_thunder_01_cl, fx_m_noise_001 | cameraoffset, coloroverlife, lifetime, size, sizemultiplylife |
| 24 | particlespriteemitter_62 | particlemodulevelocity (disabled) |  | fx_m_me_trail_02_24_tr | fx_a_cloud_022, fx_d_noise_031, fx_i_shockwave_02_ycl, fx_m_atypical_004_loc_int, fx_m_noise_001, fx_m_noise_002 | cameraoffset, color, colorscaleoverlife, lifetime, meshrotation_seeded, parameterdynamic, size, sizemultiplylife, subuv |
| 25 | particlespriteemitter_63 | mesh(override mat) | fm_h_lighting_01_1 | fx_c_pa_lensflare_01_04_ad | fx_c_glow_006, fx_c_glow_007 | colorscaleoverlife, location, location_seeded, meshrotationrate, parameterdynamic, size, spawn |
| 26 | particlespriteemitter_7 | sprite |  |  |  | cameraoffset, coloroverlife, lifetime, size_seeded |
