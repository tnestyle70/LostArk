# LanceMaster Effect 리소스 이름 해석 (원본 패키지 기준)

작성 2026-08-22. 근거는 `Data/Effects/Imported/LanceMaster/LanceMaster.resource-source-manifest.json`(umodel 추출 당시 원본 UPK의 `패키지.그룹.오브젝트` 경로, 역할, 참조 스킬, 참조 ParticleSystem)과 `Data/Balance/PlayerSkills.json`의 스킬 이름이다. 현재 프로젝트 Authored 문서가 그 리소스를 어떻게 쓰는지는 여기서 판단하지 않는다.

## 1. 이름 규칙 요약

### 원본 패키지(어디서 왔나)

| 패키지 | 내용 | 비고 |
|---|---|---|
| `fx_tex_00`~`fx_tex_06`, `fx_tex_high_xx`, `fx_tex_nomipmap_00` | 전 클래스 공용 FX 텍스처 라이브러리 | `high`=고해상도, `nomipmap`=밉맵 없음 |
| `fx_sm_00`~`fx_sm_03` | 전 클래스 공용 FX 스태틱 메시 라이브러리(`fm_*`) | 창술사 전용 아님 |
| `fx_m`, `fx_mastermaterial`/`fx_mm`, `fx_mi`, `bfx_m` | 마스터 머티리얼(부모) | `_ad/_tr` 블렌드 모드가 이름에 박힘 |
| `fx_m_mi_00`~`05`, `fx_m_mi_<letter>_00`, `bfx_m_mi_00` | 머티리얼 인스턴스(텍스처 바인딩 + 파라미터) | 리소스 폴더에는 없음(JSON 문서로 변환됨) |
| `FX_PC_FLM_00`~`07` | 창술사(FLM = Female Lance Master) ParticleSystem 패키지 | 스킬 의미는 **여기** `Par_<letter>_FLM_<스킬영문명>_NN`에 있음 |
| `sk_flm_gdr_00`, `sk_flm_ydr_00`, `fx_tex_high_03` 일부 | 창술사 전용 용(dragon) 모델 텍스처 | 34150 귀룡연참(ydr), 34600/34620/34640 각성·가이볼그(gdr) |
| `lv_dynamicsky`, `lv_matte` | 레벨 하늘 텍스처/메시 | 34620 SuperGaBolg 마티니 배경 |

### 텍스처 파일명 `fx_<letter>_<category>_<NNN>[_variant][_suffix].dds`

- `fx_` = FX 텍스처. `<letter>`(a~x)는 **라이브러리 볼륨/제작 배치**이지 의미 분류가 아니다. 같은 letter가 같은 `fx_tex_NN` 패키지에 몰린다(a,b→tex_00 / c→tex_01 / d→tex_02 / e→tex_03 / f,h,i,j→tex_04 / k,l,m→tex_05 / w,x→tex_06).
- `<category>`가 실제 내용이다: `atypical`(비정형 잡 형태), `noise`, `decal`(바닥 장판), `normal`(노멀맵), `glow`, `cloud`, `trail`, `hit`, `ring`, `line`, `fragment`(파편), `fluid`, `environ(ment)`, `shockwave`, `flow`, `wave`, `symbol`, `fire`, `ice`, `smoke`, `spark`, `electric`, `aura*`, `grid`…
- `_NNN`은 일련번호, `_1/_2`는 파생본.
- 접미사: `_cl` = UV clamp(양축), `_xcl`/`_ycl` = X/Y 한 축만 clamp(트레일·링처럼 한 방향으로 늘리는 텍스처), `_xyclamp` 동일 의미 풀네임, `_n` = normal map, `_d` = diffuse, `_e`/`_em` = emissive, `_cm` = 용 모델용 마스크(추정: color/control mask), `_h` = height/highlight(1건), `r` 단독(`fx_e_hit_009r`) = 반전/리버스 변형(추정).
- `flat_gray.dds`는 원본 FX 텍스처가 아니라 `efmaster_material_prologue`의 기본 회색이다.

### 메시 파일명 `fm_<letter>_<shape>_<NNN>.wmodel`

- `fm_` = FX mesh(`fn_`도 동일 계열 1건). `<letter>`는 역시 배치 구분이며 전부 `fx_sm_00/01`(공용 라이브러리)에서 왔다.
- `<shape>`가 실제 의미: `cylinder`, `hemisphere`/`halfsphere`, `sphere`, `circle`, `ring`, `torus`, `cone`, `helix`, `screw`, `sprial(=spiral)`, `plane/plan`, `square`, `stone`, `trail`, `swing`(검격 호), `wave`, `tornado`, `dragonhead`.
- 창술사 전용 메시는 `fm_n_flm_ydr_00_sm`(34150 귀룡연참 황룡), `sk_flm_gdr_00_sk`(34600 각성 캐스팅 청룡, 스켈레탈), `fm_x_flm_gdr_01[_dragon]`(34620/34640 가이볼그 용), `fm_k_dragonhead_01`(34590 체후폭발… `ChehuExp`), `sky_seamless_sm`(34620 마티니 하늘)뿐이다. 나머지 64개는 형태만 의미가 있는 공용 메시다.

### 머티리얼 이름(파일은 없지만 텍스처·메시를 어떻게 쓰는지 알려줌)

- `fx_<letter>_<pa|me|de|po>_<용도>_<NN>_<blend>`: `pa`=particle(스프라이트), `me`=mesh, `de`=decal, `po`=post process.
- 블렌드: `_ad`=Additive, `_tr`=Translucent, `_ma`=Masked, `_op`=Opaque, `_dt`/`_ddt`=depth-test/dissolve 변형(추정), `_ts`=two-sided(추정), `_unlit`, `_nodep`=no depth.
- 예: `fx_m.fx_d_me_flow_02_tr` = 메시에 흐르는(flow) 텍스처를 반투명으로 흘리는 마스터. 이런 머티리얼이 `noise`/`flow`/`normal` 텍스처를 distortion 입력으로 쓴다.

### ParticleSystem 이름 `FX_PC_FLM_NN.Par_<letter>_FLM_<SkillEnglish>_<NN>`

- 스킬 의미는 텍스처/메시가 아니라 여기서 나온다. 예: `Par_V_FLM_HurricaneSwing_01_3`(34050), `Par_N_FLM_SpearAura_03`(34550), `Par_M_FLM_KakSungCast_01`(34600 각성 캐스팅), `Par_X_FLM_SuperGabolg_*`(34620/34640), `Par_O_FLM_DownwardHit_02_5`, `Par_N_FLM_Ark_04`(34150).
- 한 텍스처/메시가 여러 스킬에 재사용되므로 "이 dds가 어느 스킬 것"이 아니라 "이 스킬의 어느 ParticleSystem이 이 dds를 어느 머티리얼로 쓰는가"로 봐야 한다. 아래 표가 그 역인덱스다.

## 2. 스킬 ID ↔ 이름 (manifest가 참조하는 스킬 21개)

| skillId | 슬롯 | 이름 |
|---|---|---|
| 34010 | LMB | 긴 창_평타 |
| 34020 | SPACE | 탄영 |
| 34050 |  | (PlayerSkills 미등록) |
| 34070 |  | (PlayerSkills 미등록) |
| 34080 |  | (PlayerSkills 미등록) |
| 34090 | W | 철량추 |
| 34110 | D | 반월섬 |
| 34120 | S | 연환섬 |
| 34150 | F | 맹룡열파 |
| 34170 |  | (PlayerSkills 미등록) |
| 34510 | LMB | 평타 |
| 34520 | SPACE | 돌파 |
| 34540 | Q | 나선창 |
| 34550 | W | 사두룡격 |
| 34560 | E | 굉열파 |
| 34570 | R | 유성강천 |
| 34580 | A | 절룡세 |
| 34590 | S | 적룡포 |
| 34600 |  | (PlayerSkills 미등록) |
| 34620 |  | (PlayerSkills 미등록) |
| 34640 |  | (PlayerSkills 미등록) |

## 3. Meshes (69개)

| 파일 | 원본 경로 | 패키지 | 사용 스킬 | 참조 ParticleSystem |
|---|---|---|---|---|
| `fm_a_cylinder_002.wmodel` | `fx_sm_00.fm_a_cylinder_002` | fx_sm_00 | 34080 (PlayerSkills 미등록) | FX_PC_FLM_03.Par_N_FLM_FickAtt_01 |
| `fm_a_cylinder_003.wmodel` | `fx_sm_00.fm_a_cylinder_003` | fx_sm_00 | 34050 (PlayerSkills 미등록) | FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_3, FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_4 |
| `fm_a_cylinder_004.wmodel` | `fx_sm_00.fm_a_cylinder_004` | fx_sm_00 | 34570 유성강천, 34600 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_GaebolgCastBody_Impact_01, FX_PC_FLM_01.Par_M_FLM_SuBody_Impact_01, FX_PC_FLM_01.Par_M_FLM_dust_03, FX_PC_FLM_06.Par_V_FLM_dust_03 |
| `fm_a_hemisphere_012.wmodel` | `fx_sm_00.fm_a_hemisphere_012` | fx_sm_00 | 34510 평타, 34540 나선창 | FX_PC_FLM_00.Par_K_FLM_ShortNormal_Atk_01, FX_PC_FLM_04.Par_S_FLM_Stab_Shoulder_01_1 |
| `fm_a_hemisphere_015.wmodel` | `fx_sm_00.fm_a_hemisphere_015` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_31, FX_PC_FLM_03.Par_N_FLM_SpearAura_03 |
| `fm_a_plan_001.wmodel` | `fx_sm_00.fm_a_plan_001` | fx_sm_00 | 34080 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_D_FLM_Dash_01 |
| `fm_a_sprial_003.wmodel` | `fx_sm_00.fm_a_sprial_003` | fx_sm_00 | 34050 (PlayerSkills 미등록) | FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_3, FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_4 |
| `fm_a_stone_001.wmodel` | `fx_sm_00.fm_a_stone_001` | fx_sm_00 | 34070 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬, 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_Sasun_UpImpact_01, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_5, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_6, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_7 … 외 11 |
| `fm_a_stone_004.wmodel` | `fx_sm_00.fm_a_stone_004` | fx_sm_00 | 34150 맹룡열파 | FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_03_11, FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_03_12 |
| `fm_b_circle_009.wmodel` | `fx_sm_00.fm_b_circle_009` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_01, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_02, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_31, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_32 … 외 4 |
| `fm_b_cylinder_002.wmodel` | `fx_sm_00.fm_b_cylinder_002` | fx_sm_00 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_WPCast_RE_00_00 |
| `fm_b_halfcylinder_001.wmodel` | `fx_sm_00.fm_b_halfcylinder_001` | fx_sm_00 | 34070 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_5, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_6, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_7, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_8 … 외 1 |
| `fm_b_halfsphere_001.wmodel` | `fx_sm_00.fm_b_halfsphere_001` | fx_sm_00 | 34580 절룡세 | FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_00, FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_05, FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_06 |
| `fm_b_screw_007.wmodel` | `fx_sm_00.fm_b_screw_007` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_01, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_31 |
| `fm_b_sphere_001.wmodel` | `fx_sm_00.fm_b_sphere_001` | fx_sm_00 | 34110 반월섬, 34120 연환섬, 34540 나선창, 34590 적룡포, 34640 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_Cast_01, FX_PC_FLM_01.Par_M_FLM_Chehu_Custom_01, FX_PC_FLM_04.Par_S_FLM_Stab_Atk_01_1, FX_PC_FLM_04.Par_S_FLM_Stab_Atk_03_1 … 외 5 |
| `fm_c_square_001.wmodel` | `fx_sm_00.fm_c_square_001` | fx_sm_00 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_00, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_02, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_03, FX_PC_WGL_07.Par_S_SuperAwake_ExMove_01 |
| `fm_c_square_003.wmodel` | `fx_sm_00.fm_c_square_003` | fx_sm_00 | 34540 나선창 | FX_PC_FLM_04.Par_S_FLM_Stab_EdgeAtk_01_1 |
| `fm_d_cone_003.wmodel` | `fx_sm_00.fm_d_cone_003` | fx_sm_00 | 34590 적룡포 | FX_PC_FLM_06.Par_V_FLM_ChehuSphereEnd_01, FX_PC_FLM_06.Par_V_FLM_ChehuSphereEnd_68 |
| `fm_d_cone_005.wmodel` | `fx_sm_00.fm_d_cone_005` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_31, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_32, FX_PC_FLM_03.Par_N_FLM_SpearAura_03, FX_PC_FLM_03.Par_N_FLM_SpearAura_04 |
| `fm_d_cone_006_1.wmodel` | `fx_sm_00.fm_d_cone_006_1` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_01, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_02, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_31, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_32 … 외 4 |
| `fm_d_cone_007_1.wmodel` | `fx_sm_00.fm_d_cone_007_1` | fx_sm_00 | 34050 (PlayerSkills 미등록), 34110 반월섬 | FX_PC_FLM_01.Par_M_FLM_ComCustomizing_02 |
| `fm_d_cylinder_019.wmodel` | `fx_sm_00.fm_d_cylinder_019` | fx_sm_00 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGabolg_Back_00_00 |
| `fm_d_helix_011.wmodel` | `fx_sm_00.fm_d_helix_011` | fx_sm_00 | 34590 적룡포 | FX_PC_FLM_06.Par_V_FLM_ChehuExp_01, FX_PC_FLM_06.Par_V_FLM_ChehuExp_04, FX_PC_FLM_06.Par_V_FLM_ChehuExp_47, FX_PC_FLM_06.Par_V_FLM_ChehuExp_SS_67 … 외 3 |
| `fm_d_helix_013.wmodel` | `fx_sm_00.fm_d_helix_013` | fx_sm_00 | 34080 (PlayerSkills 미등록), 34550 사두룡격, 34600 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_KakSungCast_01, FX_PC_FLM_02.Par_D_FLM_Dash_01, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_02, FX_PC_FLM_03.Par_N_FLM_SpearAuraRad_Re_32 … 외 2 |
| `fm_d_helix_015.wmodel` | `fx_sm_00.fm_d_helix_015` | fx_sm_00 | 34540 나선창, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_BS_04.State.Par_J_ExMove_01, FX_PC_FLM_04.Par_S_FLM_Stab_Atk_03_1, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Wind_00_00, FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_RE_ExMove_00_00 |
| `fm_d_helix_015_1.wmodel` | `fx_sm_00.fm_d_helix_015_1` | fx_sm_00 | 34010 긴 창_평타, 34110 반월섬, 34550 사두룡격, 34570 유성강천, 34590 적룡포, 34640 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_CastStart_01, FX_PC_FLM_01.Par_M_FLM_Cast_01, FX_PC_FLM_01.Par_M_FLM_ChehuCaststart_01, FX_PC_FLM_01.Par_M_FLM_Chehu_Custom_01 … 외 12 |
| `fm_d_helix_031.wmodel` | `fx_sm_00.fm_d_helix_031` | fx_sm_00 | 34640 (PlayerSkills 미등록) | FX_PC_WGL_07.Par_S_SuperAwake_ExMove_01 |
| `fm_d_hemisphere_001.wmodel` | `fx_sm_00.fm_d_hemisphere_001` | fx_sm_00 | 34170 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_O_FLM_Pulverize_R_01_01, FX_PC_FLM_02.Par_O_FLM_Pulverize_R_01_02 |
| `fm_d_hemisphere_001_1.wmodel` | `fx_sm_00.fm_d_hemisphere_001_1` | fx_sm_00 | 34070 (PlayerSkills 미등록), 34590 적룡포 | FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_6, FX_PC_FLM_02.Par_O_FLM_DownwardHit_02_8, FX_PC_FLM_06.Par_V_FLM_ChehuSphereEnd_01, FX_PC_FLM_06.Par_V_FLM_ChehuSphereEnd_68 |
| `fm_d_hemisphere_001_2.wmodel` | `fx_sm_00.fm_d_hemisphere_001_2` | fx_sm_00 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Spike_00_02 |
| `fm_d_hemisphere_002.wmodel` | `fx_sm_00.fm_d_hemisphere_002` | fx_sm_00 | 34540 나선창 | FX_PC_FLM_04.Par_S_FLM_Stab_Shoulder_01_1 |
| `fm_d_hemisphere_008.wmodel` | `fx_sm_00.fm_d_hemisphere_008` | fx_sm_00 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Spike_Dust_00_00 |
| `fm_d_hemisphere_009_1.wmodel` | `fx_sm_00.fm_d_hemisphere_009_1` | fx_sm_00 | 34080 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_D_FLM_FKick_Atk_41 |
| `fm_d_plane_003.wmodel` | `fx_sm_00.fm_d_plane_003` | fx_sm_00 | 34510 평타, 34540 나선창, 34560 굉열파, 34580 절룡세 | FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_00, FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_05, FX_PC_FLM_00.Par_K_FLM_CounterAttack_Atk_06, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_04 … 외 5 |
| `fm_d_ring_008.wmodel` | `fx_sm_00.fm_d_ring_008` | fx_sm_00 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Dust_00_01, FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Spike_Dust_00_00 |
| `fm_d_torus_002.wmodel` | `fx_sm_00.fm_d_torus_002` | fx_sm_00 | 34540 나선창, 34570 유성강천 | FX_PC_FLM_01.Par_M_FLM_Att_02, FX_PC_FLM_04.Par_S_FLM_Stab_Shoulder_01_1, FX_PC_FLM_06.Par_V_FLM_Att_02, FX_PC_FLM_06.Par_V_FLM_Stab_Shoulder_01_1 |
| `fm_d_trail_005.wmodel` | `fx_sm_00.fm_d_trail_005` | fx_sm_00 | 34560 굉열파 | FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_00, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_02, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_04, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_24 |
| `fm_e_screw_002.wmodel` | `fx_sm_00.fm_e_screw_002` | fx_sm_00 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Jump_00_01, FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_ULTON_Jump_00_01 |
| `fm_e_screwicorn_001.wmodel` | `fx_sm_00.fm_e_screwicorn_001` | fx_sm_00 | 34080 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_D_FLM_Dash_01 |
| `fm_f_cone002.wmodel` | `fx_sm_00.fm_f_cone002` | fx_sm_00 | 34050 (PlayerSkills 미등록) | FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_3, FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_4 |
| `fm_f_cylinder001.wmodel` | `fx_sm_00.fm_f_cylinder001` | fx_sm_00 | 34590 적룡포 | FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_00, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_04, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_06, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_47 … 외 2 |
| `fm_f_cylinder_001.wmodel` | `fx_sm_00.fm_f_cylinder_001` | fx_sm_00 | 34540 나선창 | FX_PC_FLM_04.Par_S_FLM_Stab_Atk_01_1, FX_PC_FLM_04.Par_S_FLM_Stab_Atk_03_1 |
| `fm_h_halfsphere_01_1.wmodel` | `fx_sm_00.fm_h_halfsphere_01_1` | fx_sm_00 | 34550 사두룡격 | FX_PC_FLM_03.Par_N_FLM_SpearAura_04 |
| `fm_h_swing_02.wmodel` | `fx_sm_00.fm_h_swing_02` | fx_sm_00 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_00, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_03 |
| `fm_h_swing_03.wmodel` | `fx_sm_00.fm_h_swing_03` | fx_sm_00 | 34070 (PlayerSkills 미등록), 34150 맹룡열파, 34640 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_1, FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_2, FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_3, FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_4 … 외 3 |
| `fm_h_swing_05.wmodel` | `fx_sm_00.fm_h_swing_05` | fx_sm_00 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_00, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_02 |
| `fm_i_cone_02.wmodel` | `fx_sm_00.fm_i_cone_02` | fx_sm_00 | 34050 (PlayerSkills 미등록) | FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_3, FX_PC_FLM_06.Par_V_FLM_HurricaneSwing_01_4 |
| `fm_i_cylinder_01.wmodel` | `fx_sm_00.fm_i_cylinder_01` | fx_sm_00 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_00 |
| `fm_i_swing_02.wmodel` | `fx_sm_00.fm_i_swing_02` | fx_sm_00 | 34150 맹룡열파 | FX_PC_FLM_03.Par_N_FLM_DragonSwing_02, FX_PC_FLM_03.Par_N_FLM_DragonSwing_03, FX_PC_FLM_03.Par_N_FLM_HurricaneSwing_Re_010 |
| `fm_k_dragonhead_01.wmodel` | `fx_sm_01.fm_k_dragonhead_01` | fx_sm_01 | 34590 적룡포 | FX_PC_FLM_06.Par_V_FLM_ChehuExp_01, FX_PC_FLM_06.Par_V_FLM_ChehuExp_04, FX_PC_FLM_06.Par_V_FLM_ChehuExp_47, FX_PC_FLM_06.Par_V_FLM_ChehuExp_SS_67 … 외 3 |
| `fm_k_halfcylinder_01.wmodel` | `fx_sm_01.fm_k_halfcylinder_01` | fx_sm_01 | 34540 나선창, 34560 굉열파, 34590 적룡포 | FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_00, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_02, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_04, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_24 … 외 7 |
| `fm_k_helix_01.wmodel` | `fx_sm_00.fm_k_helix_01` | fx_sm_00 | 34150 맹룡열파 | FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_01_04, FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_01_05, FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_01_06, FX_PC_FLM_03.Par_N_FLM_EarthQS_Trail_03_11 |
| `fm_k_tornado_01.wmodel` | `fx_sm_00.fm_k_tornado_01` | fx_sm_00 | 34540 나선창, 34590 적룡포, 34640 (PlayerSkills 미등록) | FX_PC_FLM_04.Par_S_FLM_Stab_Atk_01_1, FX_PC_FLM_04.Par_S_FLM_Stab_Atk_03_1, FX_PC_FLM_06.Par_V_FLM_ChehuExp_01, FX_PC_FLM_06.Par_V_FLM_ChehuExp_04 … 외 8 |
| `fm_k_wave_01.wmodel` | `fx_sm_01.fm_k_wave_01` | fx_sm_01 | 34590 적룡포 | FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_00, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_04, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_06, FX_PC_FLM_06.Par_V_FLM_red_decal_Atk_S_47 … 외 2 |
| `fm_m_helix_006.wmodel` | `fx_sm_00.fm_m_helix_006` | fx_sm_00 | 34560 굉열파 | FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_00, FX_PC_FLM_00.Par_K_FLM_PenetrationLunge_Atk_02 |
| `fm_m_helix_008.wmodel` | `fx_sm_01.fm_m_helix_008` | fx_sm_01 | 34600 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_GaebolgTarget_01 |
| `fm_m_helix_010.wmodel` | `fx_sm_01.fm_m_helix_010` | fx_sm_01 | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Spike_00_02 |
| `fm_m_ring_001.wmodel` | `fx_sm_00.fm_m_ring_001` | fx_sm_00 | 34010 긴 창_평타, 34110 반월섬 | FX_PC_FLM_01.Par_M_FLM_PyungMTrail_01, FX_PC_FLM_01.Par_M_FLM_Sasun_MTrail_01 |
| `fm_m_sphere_006.wmodel` | `fx_sm_01.fm_m_sphere_006` | fx_sm_01 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Dust_00_01, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Wind_00_00, FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Spike_00_02, FX_PC_FLM_07.Par_X_FLM_SuperGabolg_Dust_00_05 |
| `fm_m_trail_002.wmodel` | `fx_sm_01.fm_m_trail_002` | fx_sm_01 | 34090 철량추, 34110 반월섬, 34120 연환섬, 34150 맹룡열파, 34640 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_MTrail_03, FX_PC_FLM_01.Par_M_FLM_Sasun_MTrail_01, FX_PC_FLM_01.Par_M_FLM_Sasun_MTrail_02, FX_PC_FLM_01.Par_M_FLM_Sasun_MTrail_04 … 외 8 |
| `fm_n_flm_ydr_00_sm.wmodel` | `fx_sm_01.fm_n_flm_ydr_00_sm` | fx_sm_01 | 34150 맹룡열파 | FX_PC_FLM_03.Par_N_FLM_Ark_04, FX_PC_FLM_03.Par_N_FLM_Ark_05, FX_PC_FLM_03.Par_N_FLM_Ark_07 |
| `fm_o_swing_01.wmodel` | `fx_sm_00.fm_o_swing_01` | fx_sm_00 | 34090 철량추 | FX_PC_FLM_04.Par_S_FLM_Dragon_Trail_01_1, FX_PC_FLM_04.Par_S_FLM_Dragon_Trail_01_2, FX_PC_FLM_04.Par_S_FLM_Dragon_Trail_01_3 |
| `fm_o_swing_02.wmodel` | `fx_sm_00.fm_o_swing_02` | fx_sm_00 | 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34150 맹룡열파, 34640 (PlayerSkills 미등록) | FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_3, FX_PC_FLM_02.Par_O_FLM_DownwardHit_01_4, FX_PC_FLM_03.Par_N_FLM_DragonSwing_02, FX_PC_FLM_03.Par_N_FLM_DragonSwing_03 … 외 5 |
| `fm_t_trail_001.wmodel` | `fx_sm_02.fm_t_trail_001` | fx_sm_02 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Swing_00_02 |
| `fm_x_flm_gdr_01.wmodel` | `fx_sm_03.fm_x_flm_gdr_01` | fx_sm_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_WP_00_00, FX_PC_FLM_07.Par_X_FLM_SuperGabolg_WPCast_00_01, FX_PC_FLM_07.Par_X_FLM_SuperGabolg_WP_00_01_RE |
| `fm_x_flm_gdr_01_dragon.wmodel` | `fx_sm_03.fm_x_flm_gdr_01_dragon` | fx_sm_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_WP_00_00, FX_PC_FLM_07.Par_X_FLM_SuperGabolg_WPCast_00_01, FX_PC_FLM_07.Par_X_FLM_SuperGabolg_WP_00_01_RE |
| `fn_m_screw_001.wmodel` | `fx_sm_01.fn_m_screw_001` | fx_sm_01 | 34640 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_DragonSweep_Wind_00_00, FX_PC_FLM_07.Par_X_FLM_DragonSweep_Wind_00_01 |
| `sk_flm_gdr_00_sk.wmodel` | `fx_sm_01.sk_flm_gdr_00_sk` | fx_sm_01 | 34600 (PlayerSkills 미등록) | FX_PC_FLM_01.Par_M_FLM_KakSungCast_01 |
| `sky_seamless_sm.wmodel` | `lv_matte.mesh.sky_seamless_sm` | lv_matte | 34620 (PlayerSkills 미등록) | FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Re_Matinee_BG_00_01, FX_PC_FLM_07.Par_X_FLM_SuperGaBolg_Re_Matinee_Moon_00_01 |

## 4. Textures (287개)

| 파일 | 원본 경로 | 패키지 | 사용 스킬 | 참조 ParticleSystem(머티리얼 경유) |
|---|---|---|---|---|
| `flat_gray.dds` | `efmaster_material_prologue.tex.flat_gray` | efmaster_material_prologue | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_002_cl.dds` | `fx_tex_00.fx_a_atypical_002_cl` | fx_tex_00 | 34110 반월섬, 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_003.dds` | `fx_tex_00.fx_a_atypical_003` | fx_tex_00 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_018.dds` | `fx_tex_00.fx_a_atypical_018` | fx_tex_00 | 34150 맹룡열파, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_019_1.dds` | `fx_tex_00.fx_a_atypical_019_1` | fx_tex_00 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_024_cl.dds` | `fx_tex_00.fx_a_atypical_024_cl` | fx_tex_00 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_atypical_048.dds` | `fx_tex_00.fx_a_atypical_048` | fx_tex_00 | 34070 (PlayerSkills 미등록), 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_cloud_005.dds` | `fx_tex_00.fx_a_cloud_005` | fx_tex_00 | 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_cloud_022.dds` | `fx_tex_00.fx_a_cloud_022` | fx_tex_00 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34110 반월섬, 34170 (PlayerSkills 미등록), 34550 사두룡격, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_cloud_031.dds` | `fx_tex_00.fx_a_cloud_031` | fx_tex_00 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_decal_008.dds` | `fx_tex_00.fx_a_decal_008` | fx_tex_00 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_decal_008_n.dds` | `fx_tex_00.fx_a_decal_008_n` | fx_tex_00 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_decal_010.dds` | `fx_tex_00.fx_a_decal_010` | fx_tex_00 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_decal_014.dds` | `fx_tex_00.fx_a_decal_014` | fx_tex_00 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_decal_014_2.dds` | `fx_tex_00.fx_a_decal_014_2` | fx_tex_00 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_environ_001.dds` | `fx_tex_00.fx_a_environ_001` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_environ_002.dds` | `fx_tex_00.fx_a_environ_002` | fx_tex_00 | 34070 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬, 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_environ_002_n.dds` | `fx_tex_00.fx_a_environ_002_n` | fx_tex_00 | 34070 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬, 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fire_003.dds` | `fx_tex_00.fx_a_fire_003` | fx_tex_00 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fire_007.dds` | `fx_tex_00.fx_a_fire_007` | fx_tex_00 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fragment_001.dds` | `fx_tex_00.fx_a_fragment_001` | fx_tex_00 | 34110 반월섬, 34510 평타, 34580 절룡세, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fragment_002.dds` | `fx_tex_00.fx_a_fragment_002` | fx_tex_00 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34560 굉열파, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fragment_004.dds` | `fx_tex_00.fx_a_fragment_004` | fx_tex_00 | 34110 반월섬, 34120 연환섬, 34550 사두룡격, 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fragment_005.dds` | `fx_tex_00.fx_a_fragment_005` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_fragment_007.dds` | `fx_tex_00.fx_a_fragment_007` | fx_tex_00 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬, 34120 연환섬, 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34510 평타, 34540 나선창, 34550 사두룡격, 34560 굉열파, 34570 유성강천, 34580 절룡세, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_glow_001.dds` | `fx_tex_00.fx_a_glow_001` | fx_tex_00 | 34170 (PlayerSkills 미등록), 34570 유성강천, 34590 적룡포, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_glow_003.dds` | `fx_tex_00.fx_a_glow_003` | fx_tex_00 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34110 반월섬, 34540 나선창, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_glow_004.dds` | `fx_tex_00.fx_a_glow_004` | fx_tex_00 | 34570 유성강천, 34580 절룡세, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_glow_05_cl.dds` | `fx_tex_00.fx_a_glow_05_cl` | fx_tex_00 | 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_hit_005.dds` | `fx_tex_00.fx_a_hit_005` | fx_tex_00 | 34540 나선창, 34580 절룡세, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_hit_008.dds` | `fx_tex_00.fx_a_hit_008` | fx_tex_00 | 34050 (PlayerSkills 미등록), 34110 반월섬, 34570 유성강천, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_ice_003.dds` | `efmaster_material_prologue.tex.fx_a_ice_003` | efmaster_material_prologue | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_line_003.dds` | `fx_tex_00.fx_a_line_003` | fx_tex_00 | 34170 (PlayerSkills 미등록), 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_line_010.dds` | `fx_tex_00.fx_a_line_010` | fx_tex_00 | 34540 나선창, 34560 굉열파, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_line_010_ycl.dds` | `fx_tex_00.fx_a_line_010_ycl` | fx_tex_00 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_002.dds` | `fx_tex_00.fx_a_noise_002` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34110 반월섬, 34150 맹룡열파, 34540 나선창, 34550 사두룡격, 34570 유성강천, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_003.dds` | `fx_tex_00.fx_a_noise_003` | fx_tex_00 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_008_n.dds` | `fx_tex_00.fx_a_noise_008_n` | fx_tex_00 | 34580 절룡세, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_009.dds` | `fx_tex_00.fx_a_noise_009` | fx_tex_00 | 34070 (PlayerSkills 미등록), 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_010.dds` | `fx_tex_00.fx_a_noise_010` | fx_tex_00 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_noise_011.dds` | `fx_tex_00.fx_a_noise_011` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34120 연환섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_a_ring_001_cl.dds` | `fx_tex_00.fx_a_ring_001_cl` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34540 나선창, 34570 유성강천, 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_trail_004.dds` | `fx_tex_00.fx_a_trail_004` | fx_tex_00 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_trail_005_cl.dds` | `fx_tex_00.fx_a_trail_005_cl` | fx_tex_00 | 34050 (PlayerSkills 미등록), 34550 사두룡격, 34590 적룡포, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_a_trail_007.dds` | `fx_tex_00.fx_a_trail_007` | fx_tex_00 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_b_atypical_004.dds` | `fx_tex_00.fx_b_atypical_004` | fx_tex_00 | 34050 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34550 사두룡격, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_b_atypical_006.dds` | `fx_tex_00.fx_b_atypical_006` | fx_tex_00 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_b_atypical_007.dds` | `fx_tex_00.fx_b_atypical_007` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34550 사두룡격, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_b_atypical_014.dds` | `fx_tex_00.fx_b_atypical_014` | fx_tex_00 | 34120 연환섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_b_cloud_016.dds` | `fx_tex_00.fx_b_cloud_016` | fx_tex_00 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_b_glow_001_cl.dds` | `fx_tex_00.fx_b_glow_001_cl` | fx_tex_00 | 34080 (PlayerSkills 미등록), 34150 맹룡열파, 34540 나선창, 34570 유성강천, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_bg_dustpanner_01.dds` | `fx_tex_00.fx_bg_dustpanner_01` | fx_tex_00 | 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_bg_waterspray_01.dds` | `fx_tex_00.fx_bg_waterspray_01` | fx_tex_00 | 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_c_cloud_012.dds` | `fx_tex_01.fx_c_cloud_012` | fx_tex_01 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_decal_001_1.dds` | `fx_tex_01.fx_c_decal_001_1` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_decal_001_n.dds` | `fx_tex_01.fx_c_decal_001_n` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_decal_002.dds` | `fx_tex_01.fx_c_decal_002` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_flow_004.dds` | `fx_tex_nomipmap_00.fx_c_flow_004` | fx_tex_nomipmap_00 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_0001.dds` | `fx_p_w_01.tex.fx_c_glow_0001` | fx_p_w_01 | 34570 유성강천, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_001.dds` | `fx_tex_01.fx_c_glow_001` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_004_cl.dds` | `fx_tex_01.fx_c_glow_004_cl` | fx_tex_01 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_007.dds` | `fx_tex_high_00.fx_c_glow_007` | fx_tex_high_00 | 34050 (PlayerSkills 미등록), 34110 반월섬, 34150 맹룡열파, 34540 나선창, 34560 굉열파, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_008.dds` | `fx_tex_high_00.fx_c_glow_008` | fx_tex_high_00 | 34110 반월섬, 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34540 나선창, 34550 사두룡격, 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_glow_010.dds` | `fx_tex_high_00.fx_c_glow_010` | fx_tex_high_00 | 34150 맹룡열파, 34600 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_hit_003.dds` | `fx_tex_01.fx_c_hit_003` | fx_tex_01 | 34080 (PlayerSkills 미등록), 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_c_line_003_xcl.dds` | `fx_tex_01.fx_c_line_003_xcl` | fx_tex_01 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_c_line_006_cl.dds` | `fx_tex_01.fx_c_line_006_cl` | fx_tex_01 | 34570 유성강천, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_noise_001.dds` | `fx_tex_01.fx_c_noise_001` | fx_tex_01 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_c_noise_001_n.dds` | `fx_tex_01.fx_c_noise_001_n` | fx_tex_01 | 34150 맹룡열파, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_c_noise_005.dds` | `fx_tex_01.fx_c_noise_005` | fx_tex_01 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_002_1.dds` | `fx_tex_02.fx_d_atypical_002_1` | fx_tex_02 | 34170 (PlayerSkills 미등록), 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_002_1_ycl.dds` | `fx_tex_02.fx_d_atypical_002_1_ycl` | fx_tex_02 | 34070 (PlayerSkills 미등록), 34120 연환섬, 34540 나선창, 34590 적룡포, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_006.dds` | `fx_tex_02.fx_d_atypical_006` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34550 사두룡격, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_006_1.dds` | `fx_tex_02.fx_d_atypical_006_1` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34540 나선창, 34570 유성강천, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_006_1_xcl.dds` | `fx_tex_02.fx_d_atypical_006_1_xcl` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_006_ycl.dds` | `fx_tex_02.fx_d_atypical_006_ycl` | fx_tex_02 | 34050 (PlayerSkills 미등록), 34540 나선창, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_008.dds` | `fx_tex_02.fx_d_atypical_008` | fx_tex_02 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_009.dds` | `fx_tex_02.fx_d_atypical_009` | fx_tex_02 | 34550 사두룡격, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_014.dds` | `fx_tex_02.fx_d_atypical_014` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_015.dds` | `fx_tex_02.fx_d_atypical_015` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_016.dds` | `fx_tex_02.fx_d_atypical_016` | fx_tex_02 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_027.dds` | `fx_tex_02.fx_d_atypical_027` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_028.dds` | `fx_tex_02.fx_d_atypical_028` | fx_tex_02 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34090 철량추, 34110 반월섬, 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34540 나선창, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_031.dds` | `fx_tex_02.fx_d_atypical_031` | fx_tex_02 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_032.dds` | `fx_tex_02.fx_d_atypical_032` | fx_tex_02 | 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_034.dds` | `fx_tex_02.fx_d_atypical_034` | fx_tex_02 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_034_cl.dds` | `fx_tex_02.fx_d_atypical_034_cl` | fx_tex_02 | 34090 철량추, 34120 연환섬, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_042_1.dds` | `fx_tex_02.fx_d_atypical_042_1` | fx_tex_02 | 34550 사두룡격, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_043.dds` | `fx_tex_02.fx_d_atypical_043` | fx_tex_02 | 34540 나선창, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_054.dds` | `fx_tex_02.fx_d_atypical_054` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_055.dds` | `fx_tex_02.fx_d_atypical_055` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34550 사두룡격, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_055_2.dds` | `fx_tex_02.fx_d_atypical_055_2` | fx_tex_02 | 34560 굉열파, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_055_2_cl.dds` | `fx_tex_02.fx_d_atypical_055_2_cl` | fx_tex_02 | 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_055_ycl.dds` | `fx_tex_02.fx_d_atypical_055_ycl` | fx_tex_02 | 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_060_cl.dds` | `fx_tex_02.fx_d_atypical_060_cl` | fx_tex_02 | 34540 나선창, 34550 사두룡격, 34570 유성강천, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_069_ycl.dds` | `fx_tex_02.fx_d_atypical_069_ycl` | fx_tex_02 | 34050 (PlayerSkills 미등록), 34110 반월섬, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_072.dds` | `fx_tex_02.fx_d_atypical_072` | fx_tex_02 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_atypical_076_1_cl.dds` | `fx_tex_02.fx_d_atypical_076_1_cl` | fx_tex_02 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_006.dds` | `fx_tex_high_03.fx_d_cloud_006` | fx_tex_high_03 | 34110 반월섬, 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_027.dds` | `fx_tex_02.fx_d_cloud_027` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_033.dds` | `fx_tex_02.fx_d_cloud_033` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_051.dds` | `fx_tex_high_00.fx_d_cloud_051` | fx_tex_high_00 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_052.dds` | `fx_tex_high_00.fx_d_cloud_052` | fx_tex_high_00 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_cloud_053.dds` | `fx_tex_high_00.fx_d_cloud_053` | fx_tex_high_00 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_decal_003.dds` | `fx_tex_02.fx_d_decal_003` | fx_tex_02 | 34090 철량추 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_decal_018.dds` | `fx_tex_02.fx_d_decal_018` | fx_tex_02 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_decal_033.dds` | `fx_tex_02.fx_d_decal_033` | fx_tex_02 | 34090 철량추, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_decal_045.dds` | `fx_tex_02.fx_d_decal_045` | fx_tex_02 | 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_decal_078.dds` | `fx_tex_02.fx_d_decal_078` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_environ_035.dds` | `fx_tex_02.fx_d_environ_035` | fx_tex_02 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_fluid_020.dds` | `fx_tex_02.fx_d_fluid_020` | fx_tex_02 | 34110 반월섬, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_fragment_003.dds` | `fx_tex_02.fx_d_fragment_003` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_fragment_013.dds` | `fx_tex_02.fx_d_fragment_013` | fx_tex_02 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_grid_002.dds` | `fx_tex_02.fx_d_grid_002` | fx_tex_02 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_hit_004.dds` | `fx_tex_02.fx_d_hit_004` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34090 철량추 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_hit_005_cl.dds` | `fx_tex_02.fx_d_hit_005_cl` | fx_tex_02 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_hit_006_1_cl.dds` | `fx_tex_02.fx_d_hit_006_1_cl` | fx_tex_02 | 34550 사두룡격, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_002.dds` | `fx_tex_02.fx_d_noise_002` | fx_tex_02 | 34050 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34510 평타, 34540 나선창, 34560 굉열파, 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_003.dds` | `fx_tex_02.fx_d_noise_003` | fx_tex_02 | 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34540 나선창, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_004_1.dds` | `fx_tex_02.fx_d_noise_004_1` | fx_tex_02 | 34080 (PlayerSkills 미등록), 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_006.dds` | `fx_tex_02.fx_d_noise_006` | fx_tex_02 | 34560 굉열파, 34590 적룡포, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_009.dds` | `fx_tex_02.fx_d_noise_009` | fx_tex_02 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34120 연환섬, 34170 (PlayerSkills 미등록), 34540 나선창, 34550 사두룡격, 34560 굉열파, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_014.dds` | `fx_tex_02.fx_d_noise_014` | fx_tex_02 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_014_1.dds` | `fx_tex_02.fx_d_noise_014_1` | fx_tex_02 | 34020 탄영 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_021.dds` | `fx_tex_02.fx_d_noise_021` | fx_tex_02 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_030.dds` | `fx_tex_02.fx_d_noise_030` | fx_tex_02 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34110 반월섬, 34120 연환섬, 34170 (PlayerSkills 미등록), 34540 나선창, 34550 사두룡격, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_031.dds` | `fx_tex_02.fx_d_noise_031` | fx_tex_02 | 34540 나선창, 34550 사두룡격, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_033.dds` | `fx_tex_02.fx_d_noise_033` | fx_tex_02 | 34050 (PlayerSkills 미등록), 34550 사두룡격, 34590 적룡포, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_037.dds` | `fx_tex_02.fx_d_noise_037` | fx_tex_02 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_041.dds` | `fx_tex_01.fx_d_noise_041` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_noise_043.dds` | `fx_tex_02.fx_d_noise_043` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_016.dds` | `fx_tex_02.fx_d_normal_016` | fx_tex_02 | 34110 반월섬, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_016_1.dds` | `fx_tex_02.fx_d_normal_016_1` | fx_tex_02 | 34560 굉열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_018.dds` | `fx_tex_02.fx_d_normal_018` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_018_1.dds` | `fx_tex_02.fx_d_normal_018_1` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_028.dds` | `fx_tex_02.fx_d_normal_028` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_028_1.dds` | `fx_tex_02.fx_d_normal_028_1` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_033.dds` | `fx_tex_02.fx_d_normal_033` | fx_tex_02 | 34090 철량추, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_033_1.dds` | `fx_tex_02.fx_d_normal_033_1` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_041_1.dds` | `fx_tex_02.fx_d_normal_041_1` | fx_tex_02 | 34010 긴 창_평타 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_043.dds` | `fx_tex_02.fx_d_normal_043` | fx_tex_02 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_043_1.dds` | `fx_tex_02.fx_d_normal_043_1` | fx_tex_02 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_045.dds` | `fx_tex_02.fx_d_normal_045` | fx_tex_02 | 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_045_1.dds` | `fx_tex_02.fx_d_normal_045_1` | fx_tex_02 | 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_084_1.dds` | `fx_tex_02.fx_d_normal_084_1` | fx_tex_02 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_normal_085.dds` | `fx_tex_01.fx_d_normal_085` | fx_tex_01 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_shockwave_003_cl.dds` | `fx_tex_02.fx_d_shockwave_003_cl` | fx_tex_02 | 34120 연환섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_d_trail_002_cl.dds` | `fx_tex_02.fx_d_trail_002_cl` | fx_tex_02 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_d_uvturbulence_001.dds` | `fx_tex_nomipmap_00.fx_d_uvturbulence_001` | fx_tex_nomipmap_00 | 34020 탄영 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_002.dds` | `fx_tex_03.fx_e_atypical_002` | fx_tex_03 | 34010 긴 창_평타, 34110 반월섬, 34590 적룡포, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_003.dds` | `fx_tex_03.fx_e_atypical_003` | fx_tex_03 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_003_1.dds` | `fx_tex_03.fx_e_atypical_003_1` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_004.dds` | `fx_tex_03.fx_e_atypical_004` | fx_tex_03 | 34020 탄영 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_006.dds` | `fx_tex_03.fx_e_atypical_006` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_011.dds` | `fx_tex_03.fx_e_atypical_011` | fx_tex_03 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_017_cl.dds` | `fx_tex_03.fx_e_atypical_017_cl` | fx_tex_03 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_028.dds` | `fx_tex_03.fx_e_atypical_028` | fx_tex_03 | 34540 나선창, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_atypical_031.dds` | `fx_tex_03.fx_e_atypical_031` | fx_tex_03 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_cloud_002.dds` | `fx_tex_03.fx_e_cloud_002` | fx_tex_03 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_cloud_004_cl.dds` | `fx_tex_03.fx_e_cloud_004_cl` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_decal_002.dds` | `fx_tex_03.fx_e_decal_002` | fx_tex_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_decal_003.dds` | `fx_tex_03.fx_e_decal_003` | fx_tex_03 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_decal_013.dds` | `fx_tex_03.fx_e_decal_013` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_decal_013_h.dds` | `fx_tex_03.fx_e_decal_013_h` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_fluid_003_1.dds` | `fx_tex_03.fx_e_fluid_003_1` | fx_tex_03 | 34080 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_fluid_006.dds` | `fx_tex_03.fx_e_fluid_006` | fx_tex_03 | 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34540 나선창, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_fluid_007.dds` | `fx_tex_03.fx_e_fluid_007` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34090 철량추, 34120 연환섬, 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_fluid_026.dds` | `fx_tex_03.fx_e_fluid_026` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_glow_001.dds` | `fx_tex_03.fx_e_glow_001` | fx_tex_03 | 34570 유성강천, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_glow_002_cl.dds` | `fx_tex_03.fx_e_glow_002_cl` | fx_tex_03 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_hit_006.dds` | `fx_tex_03.fx_e_hit_006` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34090 철량추, 34120 연환섬, 34170 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_hit_007_1.dds` | `fx_tex_03.fx_e_hit_007_1` | fx_tex_03 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_hit_009.dds` | `fx_tex_03.fx_e_hit_009` | fx_tex_03 | 34590 적룡포, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_hit_009r.dds` | `fx_tex_03.fx_e_hit_009r` | fx_tex_03 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_ice_001.dds` | `fx_tex_03.fx_e_ice_001` | fx_tex_03 | 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_line_011_1_ycl.dds` | `fx_tex_03.fx_e_line_011_1_ycl` | fx_tex_03 | 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_noise_001.dds` | `fx_tex_03.fx_e_noise_001` | fx_tex_03 | 34050 (PlayerSkills 미등록), 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_noise_002.dds` | `fx_tex_03.fx_e_noise_002` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_noise_003.dds` | `fx_tex_03.fx_e_noise_003` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34550 사두룡격, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_noise_008.dds` | `fx_tex_03.fx_e_noise_008` | fx_tex_03 | 34510 평타, 34560 굉열파, 34580 절룡세, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_ring_001.dds` | `fx_tex_03.fx_e_ring_001` | fx_tex_03 | 34070 (PlayerSkills 미등록), 34150 맹룡열파, 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_ring_003.dds` | `fx_tex_03.fx_e_ring_003` | fx_tex_03 | 34590 적룡포, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_ring_005.dds` | `fx_tex_03.fx_e_ring_005` | fx_tex_03 | 34050 (PlayerSkills 미등록), 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_e_ring_040.dds` | `fx_tex_03.fx_e_ring_040` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_symbol_014.dds` | `fx_tex_03.fx_e_symbol_014` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_e_symbol_053.dds` | `fx_tex_03.fx_e_symbol_053` | fx_tex_03 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_f_aura_004.dds` | `fx_tex_04.fx_f_aura_004` | fx_tex_04 | 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_f_blrot_001.dds` | `fx_tex_04.fx_f_blrot_001` | fx_tex_04 | 34050 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_f_ring_001.dds` | `fx_tex_04.fx_f_ring_001` | fx_tex_04 | 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34110 반월섬, 34570 유성강천, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_h_atypical_01_1.dds` | `fx_tex_04.fx_h_atypical_01_1` | fx_tex_04 | 34010 긴 창_평타, 34090 철량추, 34110 반월섬, 34120 연환섬, 34150 맹룡열파, 34540 나선창, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_h_hit_01.dds` | `fx_tex_04.fx_h_hit_01` | fx_tex_04 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_h_noise_001.dds` | `fx_tex_04.fx_h_noise_001` | fx_tex_04 | 34110 반월섬, 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_h_trail_01_1.dds` | `fx_tex_04.fx_h_trail_01_1` | fx_tex_04 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_h_wave_04.dds` | `fx_h_w_01.fx_tex.fx_h_wave_04` | fx_h_w_01 | 34070 (PlayerSkills 미등록), 34170 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_atypical_02.dds` | `fx_tex_04.fx_i_atypical_02` | fx_tex_04 | 34070 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_atypical_03_1.dds` | `fx_tex_04.fx_i_atypical_03_1` | fx_tex_04 | 34070 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_atypical_03_2_xcl.dds` | `fx_tex_04.fx_i_atypical_03_2_xcl` | fx_tex_04 | 34050 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_environment_001.dds` | `fx_tex_04.fx_i_environment_001` | fx_tex_04 | 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_environment_002.dds` | `fx_tex_04.fx_i_environment_002` | fx_tex_04 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_noise_02_ycl.dds` | `fx_tex_04.fx_i_noise_02_ycl` | fx_tex_04 | 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_i_noise_03.dds` | `fx_tex_04.fx_i_noise_03` | fx_tex_04 | 34070 (PlayerSkills 미등록), 34560 굉열파, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_noise_04.dds` | `fx_tex_04.fx_i_noise_04` | fx_tex_04 | 34540 나선창, 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_i_noise_05.dds` | `fx_tex_04.fx_i_noise_05` | fx_tex_04 | 34070 (PlayerSkills 미등록), 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_i_rainbowring_01.dds` | `fx_tex_04.fx_i_rainbowring_01` | fx_tex_04 | 34170 (PlayerSkills 미등록), 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_shockwave_01_ycl.dds` | `fx_tex_04.fx_i_shockwave_01_ycl` | fx_tex_04 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_shockwave_02.dds` | `fx_tex_04.fx_i_shockwave_02` | fx_tex_04 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_i_shockwave_02_ycl.dds` | `fx_tex_04.fx_i_shockwave_02_ycl` | fx_tex_04 | 34010 긴 창_평타, 34070 (PlayerSkills 미등록), 34580 절룡세, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_i_smoke_01.dds` | `fx_tex_04.fx_i_smoke_01` | fx_tex_04 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_i_thunder_01_cl.dds` | `fx_tex_high_02.fx_i_thunder_01_cl` | fx_tex_high_02 | 34070 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_chormaticring_01.dds` | `fx_tex_04.fx_j_chormaticring_01` | fx_tex_04 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_flare_02_cl.dds` | `fx_tex_04.fx_j_flare_02_cl` | fx_tex_04 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_flowsmoke_01_cl.dds` | `fx_tex_04.fx_j_flowsmoke_01_cl` | fx_tex_04 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_levelupsymbol_01_cl.dds` | `fx_tex_nomipmap_00.fx_j_levelupsymbol_01_cl` | fx_tex_nomipmap_00 | 34010 긴 창_평타, 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_mirnoise_01.dds` | `fx_tex_04.fx_j_mirnoise_01` | fx_tex_04 | 34050 (PlayerSkills 미등록), 34070 (PlayerSkills 미등록), 34080 (PlayerSkills 미등록), 34150 맹룡열파, 34170 (PlayerSkills 미등록), 34540 나선창, 34580 절룡세, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_mirnoise_02.dds` | `fx_tex_04.fx_j_mirnoise_02` | fx_tex_04 | 34540 나선창, 34560 굉열파, 34580 절룡세, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_risingcolor_01.dds` | `fx_tex_04.fx_j_risingcolor_01` | fx_tex_04 | 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_j_risingforce_01.dds` | `fx_tex_04.fx_j_risingforce_01` | fx_tex_04 | 34120 연환섬, 34550 사두룡격 | (없음: 머티리얼 경유로만 참조) |
| `fx_j_smoke_01_cl.dds` | `fx_tex_04.fx_j_smoke_01_cl` | fx_tex_04 | 34560 굉열파, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_j_ylinestream_01.dds` | `fx_tex_04.fx_j_ylinestream_01` | fx_tex_04 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_j_ylinestream_01_ycl.dds` | `fx_tex_04.fx_j_ylinestream_01_ycl` | fx_tex_04 | 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_01.dds` | `fx_tex_05.fx_k_auraline_01` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_02.dds` | `fx_tex_05.fx_k_auraline_02` | fx_tex_05 | 34170 (PlayerSkills 미등록), 34540 나선창, 34560 굉열파, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_02_ycl.dds` | `fx_tex_05.fx_k_auraline_02_ycl` | fx_tex_05 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_05_ycl.dds` | `fx_tex_05.fx_k_auraline_05_ycl` | fx_tex_05 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34110 반월섬, 34170 (PlayerSkills 미등록), 34590 적룡포, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_07.dds` | `fx_tex_05.fx_k_auraline_07` | fx_tex_05 | 34050 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_08.dds` | `fx_tex_05.fx_k_auraline_08` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_14.dds` | `fx_tex_05.fx_k_auraline_14` | fx_tex_05 | 34560 굉열파, 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auraline_16.dds` | `fx_tex_05.fx_k_auraline_16` | fx_tex_05 | 34510 평타 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auratile_01.dds` | `fx_tex_05.fx_k_auratile_01` | fx_tex_05 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_auratile_02.dds` | `fx_tex_05.fx_k_auratile_02` | fx_tex_05 | 34170 (PlayerSkills 미등록), 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_bubbletile_01.dds` | `fx_tex_05.fx_k_bubbletile_01` | fx_tex_05 | 34070 (PlayerSkills 미등록), 34120 연환섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_caustictile_01.dds` | `fx_tex_05.fx_k_caustictile_01` | fx_tex_05 | 34540 나선창, 34560 굉열파, 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_electile_02.dds` | `fx_tex_05.fx_k_electile_02` | fx_tex_05 | 34070 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_electric_01.dds` | `fx_tex_05.fx_k_electric_01` | fx_tex_05 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_k_electric_02.dds` | `fx_tex_05.fx_k_electric_02` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_k_turtlespec_01.dds` | `fx_tex_05.fx_k_turtlespec_01` | fx_tex_05 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_l_environment_001.dds` | `fx_tex_05.fx_l_environment_001` | fx_tex_05 | 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_l_symbol_08_1_cl.dds` | `fx_tex_high_02.fx_l_symbol_08_1_cl` | fx_tex_high_02 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_atypical_007.dds` | `fx_tex_05.fx_m_atypical_007` | fx_tex_05 | 34050 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_atypical_008.dds` | `fx_tex_05.fx_m_atypical_008` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_atypical_012.dds` | `fx_tex_05.fx_m_atypical_012` | fx_tex_05 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_caustic_001.dds` | `fx_tex_05.fx_m_caustic_001` | fx_tex_05 | 34510 평타, 34540 나선창, 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_decal_01_1.dds` | `fx_tex_05.fx_m_decal_01_1` | fx_tex_05 | 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_decal_01_2.dds` | `fx_tex_05.fx_m_decal_01_2` | fx_tex_05 | 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_dis_001.dds` | `fx_tex_05.fx_m_dis_001` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_flow_02_n.dds` | `fx_tex_05.fx_m_flow_02_n` | fx_tex_05 | 34150 맹룡열파, 34540 나선창 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_flow_03_n.dds` | `fx_tex_05.fx_m_flow_03_n` | fx_tex_05 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_flow_04_n.dds` | `fx_tex_05.fx_m_flow_04_n` | fx_tex_05 | 34590 적룡포, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_magicsymbol_005.dds` | `fx_tex_05.fx_m_magicsymbol_005` | fx_tex_05 | 34170 (PlayerSkills 미등록), 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_magicsymbol_007.dds` | `fx_tex_05.fx_m_magicsymbol_007` | fx_tex_05 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_noise_001.dds` | `fx_tex_05.fx_m_noise_001` | fx_tex_05 | 34010 긴 창_평타, 34110 반월섬, 34550 사두룡격, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_noise_002.dds` | `fx_tex_05.fx_m_noise_002` | fx_tex_05 | 34050 (PlayerSkills 미등록), 34110 반월섬, 34550 사두룡격, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_noise_003.dds` | `fx_tex_05.fx_m_noise_003` | fx_tex_05 | 34010 긴 창_평타, 34110 반월섬, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_noise_008.dds` | `fx_tex_05.fx_m_noise_008` | fx_tex_05 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_ring_001.dds` | `fx_tex_05.fx_m_ring_001` | fx_tex_05 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_spark_001.dds` | `fx_tex_05.fx_m_spark_001` | fx_tex_05 | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_spatter_001_xyclamp.dds` | `fx_tex_05.fx_m_spatter_001_xyclamp` | fx_tex_05 | 34050 (PlayerSkills 미등록), 34510 평타, 34560 굉열파, 34580 절룡세 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_001_cl.dds` | `fx_tex_05.fx_m_trail_001_cl` | fx_tex_05 | 34070 (PlayerSkills 미등록), 34090 철량추, 34150 맹룡열파, 34540 나선창, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_002.dds` | `fx_tex_05.fx_m_trail_002` | fx_tex_05 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_003.dds` | `fx_tex_05.fx_m_trail_003` | fx_tex_05 | 34590 적룡포 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_004.dds` | `fx_tex_05.fx_m_trail_004` | fx_tex_05 | 34570 유성강천, 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_004_cl.dds` | `fx_tex_05.fx_m_trail_004_cl` | fx_tex_05 | 34010 긴 창_평타, 34050 (PlayerSkills 미등록), 34110 반월섬, 34170 (PlayerSkills 미등록), 34570 유성강천, 34590 적룡포, 34600 (PlayerSkills 미등록), 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_006.dds` | (manifest 미등재) | | | |
| `fx_m_trail_007.dds` | `fx_tex_05.fx_m_trail_007` | fx_tex_05 | 34010 긴 창_평타, 34110 반월섬 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_trail_008.dds` | `fx_tex_05.fx_m_trail_008` | fx_tex_05 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_m_wave_001.dds` | `fx_tex_05.fx_m_wave_001` | fx_tex_05 | 34170 (PlayerSkills 미등록), 34510 평타 | (없음: 머티리얼 경유로만 참조) |
| `fx_m_wave_001_ycl.dds` | `fx_tex_05.fx_m_wave_001_ycl` | fx_tex_05 | 34050 (PlayerSkills 미등록), 34110 반월섬, 34170 (PlayerSkills 미등록), 34540 나선창, 34550 사두룡격, 34570 유성강천, 34590 적룡포, 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_n_atypical_02.dds` | `fx_tex_04.fx_n_atypical_02` | fx_tex_04 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_n_decal_03.dds` | `fx_tex_05.fx_n_decal_03` | fx_tex_05 | 34110 반월섬, 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_n_trail_001_cl.dds` | `fx_tex_nomipmap_00.fx_n_trail_001_cl` | fx_tex_nomipmap_00 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_w_decal_004.dds` | `fx_tex_06.fx_w_decal_004` | fx_tex_06 | 34570 유성강천 | (없음: 머티리얼 경유로만 참조) |
| `fx_x_decal_019_cl.dds` | `fx_tex_06.fx_x_decal_019_cl` | fx_tex_06 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `fx_x_glow_001_cl.dds` | `fx_tex_06.fx_x_glow_001_cl` | fx_tex_06 | 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `lv_dynamicskypack06_06_d.dds` | `lv_dynamicsky.tex.lv_dynamicskypack06_06_d` | lv_dynamicsky | 34620 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_00_d.dds` | `sk_flm_gdr_00.tex.sk_flm_gdr_00_d` | sk_flm_gdr_00 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_00_em.dds` | `sk_flm_gdr_00.tex.sk_flm_gdr_00_em` | sk_flm_gdr_00 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_00_n.dds` | `sk_flm_gdr_00.tex.sk_flm_gdr_00_n` | sk_flm_gdr_00 | 34600 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_01_cm.dds` | `fx_tex_high_03.sk_flm_gdr_01_cm` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_01_d.dds` | `fx_tex_high_03.sk_flm_gdr_01_d` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_01_e.dds` | `fx_tex_high_03.sk_flm_gdr_01_e` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_01_n.dds` | `fx_tex_high_03.sk_flm_gdr_01_n` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_02_cm.dds` | `fx_tex_high_03.sk_flm_gdr_02_cm` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_02_d.dds` | `fx_tex_high_03.sk_flm_gdr_02_d` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_02_e.dds` | `fx_tex_high_03.sk_flm_gdr_02_e` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_gdr_02_n.dds` | `fx_tex_high_03.sk_flm_gdr_02_n` | fx_tex_high_03 | 34620 (PlayerSkills 미등록), 34640 (PlayerSkills 미등록) | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_ydr_00_e.dds` | `sk_flm_ydr_00.tex.sk_flm_ydr_00_e` | sk_flm_ydr_00 | 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |
| `sk_flm_ydr_00_n.dds` | `sk_flm_ydr_00.tex.sk_flm_ydr_00_n` | sk_flm_ydr_00 | 34150 맹룡열파 | (없음: 머티리얼 경유로만 참조) |

## 5. manifest에 없는 파일

- `fx_m_trail_006.dds`
