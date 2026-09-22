# 2026-09-22 에스더 니나브 이펙트 원작 복원 RESULT

작성자: JS · 선례 `.md/JS/09-22/2026-09-22_ESTHER_INANNA_SOURCE_EFFECT_RESTORE_RESULT.md` §2.2~2.3

## 1. 원본 근거

| 항목 | 값 |
|---|---|
| 캐스터 | `COMMONACTION.loa` 53203 (`EstherSkill.Ninave`) → `EFTable_CommonActionEffect 108` ValueA NPC **53400**(`MN_ISTM_00` 보이지 않는 소환체, 10s), ValueB skill 534000 |
| 소환체 액션 | `MN_ISTM_00.loa` 534000 = Idle 1s + Effect notify, 시각 notify 없음 → 보이는 니나브는 `MN_PPNN_00` `SK_Parkunas` 카메오 |
| 카메오 notify | **없음**. `SK_Parkunas`를 PlaySkeletalMesh로 재생하는 .loa가 없다(`parkunas` 전수 grep 0). 다른 보스 카메오는 `Att_Battle_8_01/9_0x`를 쓴다 |
| 에스더 전용 FX | `FX_ESTHER_PPNN_00`: `par_l_ppnn_spawn_01/spawn_trail_01`, `par_l_ppnn_esther_sk_01`(번개 화살 4m 메쉬·트레일·리본), `_01_1`(임팩트 16m 링·충격파), `_02/_02_1~4`, `_03/_03_1~4`(활 차지 스파이럴), `par_l_ppnn_despawn_01/02` — 어떤 .loa도 참조하지 않음 |
| 참고 배치 | `MN_CNCU_00.loa` 43000011 stage 2: 니나브 카메오가 9.95s에 `Esther_Sk_01_1`을 루트 (0,2.5,0) scale 2로 재생 |
| 카메오 원본(09-22 3차 조사) | `FX_ESTHER_PPNN_00`은 51 시스템이고 `d_`/`o_`/`v_`/`l_` 접두로 나뉜다. 니나브 카메오를 PlaySkeletalMesh로 재생하는 보스 액션은 세 개: `MN_CNCU_00` 43000011(Kazeroth3, `d_teleport_03`+`o_teleport_01` 등장 → `o_battle_*`/`o_wing_*` → 9.95s `d_sk01_01`+`d_sk01_02`+`l_esther_sk_01_1`+`o_battle_11/12`), `MN_CDKCN_00` 4266285(Kamen1, `o_` 세트), `MN_TSLC_01` 4242460(Illiakan2, `d_teleport_01/02`+`v_wing`+`v_arrow/hand_cast/tone_shot`+`d_sk01_01/02`). `MN_PPNN_00.loa` 4189601은 활 `d_sk02_03` 0.37s → 발사 `d_sk01_01`+`d_sk01_06` 0.87s. 이난나의 사용자 확인 쿠크 세트가 `d_`였으므로 `d_` 세트를 같은 시각축에 추가했다(아래 3차) |

09-07 수치 전수: `.md/JS/09-07/2026-09-07_NINAVE_PARKUNAS_FX_DETAIL.md`. probe: `out/NinaveFX20260922/ppnn_probe.log`.

## 1.5 확정 원본 (09-22 4차) — Esther 소환 시퀀스 파일

사용자 "오 이 이펙트 맞아"(`d_` 세트) 뒤 원본 사슬을 끝까지 추적했다.
`EFTable_EpicSkill 2010`(Kouku roster: Action1 53203 니나브 / 53201 웨이 / 53204 이난나) → `EFTable_CommonAction 53203`(NameDev 니나브)
→ `EFTable_CommonActionEffect 108` ValueA NPC 534x0 ValueB skill 534000 → `EFTable_SkillEffect` Key 12(projectile) →
`XMLData/Projectile/<id>.loa` = **`CEFSequenceSummonsProjectileFixArea`**(소환 시퀀스). 니나브 음성(`PC_COMMON_ESTHER.Esther_Ninave1_Attack1_*`)을
가진 순수 니나브 시퀀스는 `531300`(3.3+3.5s 2타)·`531320`·`532300`(3.5s 1타)이며 FX 구성이 같다. `534000.loa`는 이난나+카멘 봉인,
`534100`은 니나브+카멘(`NinaveH2` 히든), `534300000/2`는 몬스터 음성(연합군)이다. `532300`을 읽었다.

시퀀스 구조: Timer body 첫 int = 자식 수, 지연 float은 자식 뒤 다음 형제 액션 −16 byte(DimensionMaster V 디코더와 같은 규약).
CreateFX의 ParticleSystem 토큰 뒤 +76 위치(cm)·+136 scale. SkeletalMeshFX(`SK_Parkunas`)의 CEFAN_Particle 본 부착은
`vehicle.skeletal_mesh_particles`로 그대로 읽는다. 결과(lifetime 7.0s):

| 시각 | 시스템 | 앵커/위치 |
|---|---|---|
| 0.0 | `d_teleport_02` | root |
| 0.7 | `d_teleport_01`(착지·바닥) | root |
| 1.0 | `d_teleport_03`(1.5s) | 소켓 `fx_state_01`→`bip001-spine1` |
| 1.0 | `Par_MP_Light_05_L` | root +3 m 위 |
| 1.2 | `d_wing_02` / `d_wing`(3.0s) | 본 `cb_l_02` / `cb_r_02` |
| 2.0 | `d_arrow`(1.5s) | 소켓 `wp_2`→`b_wp_2` |
| 3.5 | `d_sk01_01`, `d_sk01_02` | root **전방 0.5 m** |
| 3.5 | Light (+ `ConcaveDis_Z_04` scale 3, 미지원 skip) | 전방 5 m |
| 3.65 | Light (+ ConcaveDis skip) | 전방 15 m |

`l_esther_sk_*`·`sk02_03` 활 차지·`sk01_06`·`skhit_01`은 시퀀스에 없어 제거. 사용자 지적 두 가지(착지 데칼이 이르다 = 0.0→0.7s,
임팩트가 멀다 = 8 m→0.5 m)가 모두 원본값으로 해결된다. 화면 왜곡 `fx_cm_01 ConcaveDis`는 restore carrier가 없어 보류.
driver: `sequence_timeline`/`sequence_rows`가 `PROJECTILE`을 읽고 `ninave_actions`가 본/월드 notify를 합성한다. 50 element, 보류 0, 7200ms, 새 program 0.

## 2. 파이프라인 — `Tools/EffectPipeline/build_esther_ninave_source_effects.py`

이난나 driver(`build_esther_inanna_source_effects.py`)를 import해 identity(ARCHETYPE/모델/소켓 메시/텍스처·메시 루트/cue 문서/
reviewed 재사용 루트)만 바꾸고 `cameo_actions`를 `ninave_actions`로 교체한다. 합성 stage(`SK_Parkunas` 4.833s):

| 시각 | 시스템 | 앵커 | 근거 |
|---|---|---|---|
| 0.00 | `spawn_01`, `spawn_trail_01` | root | 등장 |
| 3.00 | `esther_sk_03` (차지) | **`b_wp_2` follow** (템플릿 `MN_PPNN_00` 4189601 `sk02_03`의 `B_WP_08` 소켓) | 수명 0.3~0.6s, 발사 직전 |
| 3.50 | `esther_sk_01` (화살) | root | 기존 V2 저작 thunder/arrow 3.43~3.5s |
| 3.67 | `esther_sk_01_1` (임팩트) | root **전방 8m**(`IMPACT_FORWARD_METERS`) | 기존 V2 impact 3.67s; +x가 전방(실리안 Exp 1/7.5/14m) |
| 4.83 | `despawn_01` | root | 퇴장 |

**09-22 3차(`d_` 세트 추가)**: 사용자 "여전히 빠진 게 있음" → `d_teleport_01`+`d_wing`(0.0), `d_sk02_03`(bow 3.0), `d_arrow`+`d_sk01_01`+`d_sk01_06`(3.5),
`d_sk01_02`+`d_skhit_01`(3.67, 전방 8m), `d_teleport_02/03`(4.83). `sk01_01/02`는 보스 카메오가 쓰는 `FX_ESTHER_PPNN_00` 판을 사용.
196 element, 보류 1, duration 13700ms, fresh program 21(4612..4632), 재사용 49.

**09-22 2차(최대 세트, 사용자 선택)**: 첫 화면에서 "생략된 이펙트가 많다"는 관찰에 따라 패키지의 최상위 변형을 모두 추가했다.
같은 시각에 `esther_sk_03_4`(bow, 차지 20 emitter), `esther_sk_02_4`(root, 화살 32), `arrow_01`(root, 2),
`despawn_02`(root, 15)를 함께 재생한다. `_1~_4` tier는 어떤 .loa도 고르지 않으므로 `_4`가 상위 집합이라는 가정이다.

시각·전방 거리·tier(`_1~_4` 미사용, 기본 3종)는 **전부 추론/튜닝값**이다. 원본 notify가 없다.

| 단계 | 결과 |
|---|---|
| acquire | emitter 67(sprite 51, mesh 12, ribbon 4), 재질 36, 소켓 계약 `mn_ppnn_00.mesh.mn_ppnn_00_sk` |
| native | 1차 fresh **29**(4567..4595), 2차 fresh **13**(4597..4609) + 재사용 35, 보류 1(화살 본체 재질). 4607 상한을 넘어 **그룹 4608(4608..4671)** 신설 |
| 상한 확장 | `install_kouku_gate1_native_shaders.py`(assert·`range(2304, 4672)`·Decal/Trail 정규식), `install_kouku_gate1_native_materials.py` LAST, C++ 7곳(`Effect_DocumentRenderer_Geometry/MaterialBinding/Particles.cpp`, `Effect_NativeScreenPostMaterial.cpp`) 4607→4671, `Shader_VtxEffectDecal/Trail.hlsl` 4671. `Shader_VtxEffectParticleKouku4608.hlsl`·`Shader_EffectKoukuNativeGroup4608.hlsli`·Dispatch/Selected 4608은 설치 스크립트가 생성·vcxproj/filters/`Effect_ShaderFamily.h` 등록 |
| 예산 | 리본 7개 × maxPoints 500 = 3500 > 코덱 상한 2048 → `clamp_trail_budget`가 2048/7=292로 분배(수명 0.35~0.8s·60Hz라 살아 있는 점은 ≤50). 파티클 합 4525 < 8192 |
| 검토 추가 | `generate_artist_native_runtime_shader.py` reviewed pass-constant sid에 `77a224a1…`(화살 메쉬 `fx_l_me_transition_05_8_ma`, CB2[3]/[4] mad 색 override뿐 — Terpeion 날개와 같은 형태) 추가 |
| 보류 2 | 같은 재질의 화살 본체 메쉬 emitter 2개 — `Native TEXCOORD7 has no mesh carrier adapter`(공용 mesh carrier 한계, 팀장 영역). 번개 메쉬·트레일·리본은 포함 |
| provider 바인딩 | `LocationEmitter` 모듈 4개(spawn 리본→`arrow-head` 메쉬, 화살 메쉬→`111` erm_none 시뮬레이션 provider, 화살 리본 2개→`t` = 보류된 본체 메쉬라 `sourceprovidermissing`). 같은 원본 ParticleSystem 안에서만 묶는다 |
| 화살 본체 | 처음엔 "TEXCOORD7 mesh carrier 없음"으로 8개를 보류했으나, 이는 generator `kouku_lit` adapter 표에 PS별 행이 없다는 뜻일 뿐이었다. `77a224a1…`은 설치된 `3a96e00b…`와 같은 LocalVF 계약(row 0 = 파티클 RGBA·W 마스크 임계, TEXCOORD6 view, TEXCOORD7 up, sky row 7~9, row 6.z는 보조 MRT 전용)이라 `('0c1413bd…', 7, 'color', [0, 6, 7, 8, 9])` 한 행을 추가해 program **4611**(blend_masked)로 설치했다. 화살 리본 2쌍의 provider `t`도 이 메쉬로 바인딩됨 |
| project | 3차 `effect.esther.ninave.cameo.npc_sk_parkunas.full.restore` **135 element**(root 110, `b_wp_2` follow 25), 보류 1(Lifetime 없는 emitter), duration 9000ms, `NPC_59504.npcactioncues.json` 1 cue, catalog +1 |

## 3. 런타임 연결

- `Data/Effects/V2/Bindings/NPC_59504.effectv2bindings.json` bindings 11 → 0 (실리안과 같은 처리). 원본은
  `out/NinaveFX20260922/NPC_59504.effectv2bindings.backup.json`. V2 문서 `esther.ninave.*` 10개는 파일로 남는다.
- `Level_Loading.cpp` Esther roster 선준비에 `NPC_59504` 추가, `Effect_PresentationService.cpp` 본 basis 목록에 `effect.esther.ninave.` 추가.
- catalog strike `npc_sk_parkunas`·Server strikeMs 4900은 그대로.

## 4. 검증

- 실행함: driver 전 단계 exit 0, 문서 감사(위 표와 시각·앵커·전방 8m 일치), Lifetime/Required/Spawn cardinality 통과, JSON parse.
- Debug Product 빌드 PASS — `out/BuildPipeline/runs/20260922T114453310Z-debug-product.json`(Client CSO 6, OBJ 3, 에러 0).
- **첫 두 설치본은 Client가 `V1.loading` 빈 reason으로 전체 거부했다.** 원인은 `LocationEmitter` 모듈이 있는
  emitter 4개에 recipe identity(`particleSystemOccurrenceId/emitterName`)와 `runtime.providerelementid`
  바인딩이 없어서 `CEffectPlayback::Validate_SourceParticleProviders`가 메시지 없이 false를 돌려준 것
  (`Effect_Playback.cpp` `if (!HasIdentity || …) return false;`). 이난나·실리안 문서엔 이 모듈이 없어 통과했다.
  함께 잘못 제외했던 `emitter_16`(defaultparticle·`erm_none`)은 화살 메쉬의 시뮬레이션 provider라 복귀시켰다.
  수정: `build_esther_inanna_source_effects.py`의 `bind_providers`(showtime `bind_source_providers`를 notify별로 호출),
  `native_occurrence_excluded`의 defaultparticle 규칙을 erm_normal에만 적용, `audit_document`에 provider 검사 추가.
- 코덱 하네스(`out/EffectCodecHarness`, gitignore 대상): `CEffectDocumentCodec::Load/Validate/Validate_Drawable`를
  `LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`로 실행 → 니나브·이난나·실리안 세 문서 모두 OK.
  provider 검사는 하네스가 stub이라 python audit이 C++ 규칙을 그대로 대조한다.
- 1차(65 element) 화면 확인: 사용자 "나오긴 하는데 생략된 이펙트가 많다" → 최대 세트로 2차 투영.
- 2차 문서: 하네스 Load/Validate/Drawable OK(하네스는 새 `Effect_ArtistMaterial_Tables.inl`로 재빌드해야 4608+ profile을 인정한다).
  trail 예산 초과는 하네스가 먼저 잡았다(`Effect Document exceeds the particle, trail, or after-image budget`).
- 5차(시퀀스 원본 50 element) 하네스 OK, C++/셰이더 변경 없음(데이터만) → Client 재시작. 화면 확인: 사용자(미실행).
- 4차(196 element, `d_` 세트 추가) 하네스 OK, Debug Product 빌드 PASS — `out/BuildPipeline/runs/20260922T134132474Z-debug-product.json`(Client 764s, 에러 0).
  화면 확인: 사용자(미실행). 로컬 Server(127.0.0.1)·Client 재실행 상태로 인계.
- 3차(135 element) 하네스 Load/Validate/Drawable OK. Debug Product 빌드 PASS —
  `out/BuildPipeline/runs/20260922T130714012Z-debug-product.json`(Client 151s, 에러 0, `Shader_VtxEffectMeshKouku4608.cso` 생성).
  화면 확인: 사용자(미실행). 로컬 Server(127.0.0.1)·Client 재실행 상태로 인계.
- 2차 Debug Product 빌드 PASS — `out/BuildPipeline/runs/20260922T125443498Z-debug-product.json`(Client 973s, 에러 0,
  `Shader_VtxEffectParticleKouku4608.cso` 생성). 화면 확인: 사용자(미실행). 로컬 Server(127.0.0.1)·Client 재실행 상태로 인계.

## 5. 남은 경계

- `esther_sk_02_x/03_x` tier 선택(현재 `_4`)과 임팩트 거리·시각은 사용자 관찰로 조정.
- 임팩트는 고정 전방 8m다. Server가 타깃을 주지 않으므로 보스 방향/거리 추적은 없다.
