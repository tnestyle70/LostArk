# 2026-09-22 에스더 이난나 이펙트 원작 복원 RESULT

작성자: JS · 선례 `.md/JS/09-20/2026-09-20_ESTHER_SILIAN_SOURCE_EFFECT_RESTORE_RESULT.md`

## 1. 원본 근거

| 항목 | 값 |
|---|---|
| 캐스터 | `COMMONACTION.loa` 53204 (`EstherSkill.Inanna`, `SkillDecoDelayTime3`=0): call_15 이펙트 + 0.5s `CommonActionExcute` 소환 |
| 소환 NPC | `NPC_59620` (종막_이난나, `MN_SLINN_00`), `DestroySkillIndex` 543120 |
| 연합군 스킬 | `MN_SLINN_00.loa` **543100** = `Att_Battle_1_01`(1.2s) → `1_02`(0.8s) → `1_03`(1.6s), PlayParticleEffect 6/2/6 |
| 이전 strike `sk_magicshield` | `MN_SLINN_00.loa`에는 없음. `NP_LPDA_01.loa` 4260115와 `MN_TSLC_03.loa` 4242529가 `PlaySkeletalMesh`로 띄우는 카메오(`MN_SLINN_00_SK_dead` + `SK_MagicShield` 2.83s, 본 부착 sk01_01~04·Light 내장) |

사용자 결정(09-22): 원본 체인 543100으로 strike를 교체한다. magicshield 카메오는 다른 NPC 기준 배치라 채택하지 않는다.

### 543100 notify (stage 기준 시각)

| clip | 시각 | 시스템 | 배치 |
|---|---|---|---|
| 1_01 | 0.00 | `FX_Post FilmNoise_01`, `LRSA Teleport_01`, `ConcaveDis_Z_04`, `Light_05_L`(y 1m) | root snapshot |
| 1_01 | 0.50 | `Par_S_SLINN_Bless_04`(y 0.5) / `Par_D_SLINN_sk01_04` | root / **`B_Effectroot` follow** |
| 1_02 | 0.00 | `Par_S_SLINN_Hand_01` / `Bless_04` | **`Bip001-R-Hand`+`Bip001-L-Hand` follow** / root |
| 1_03 | 0.00 | `Hand_01` | 양손 follow |
| 1_03 | 0.29 | `sk01_03_LOC_INT`(y 1.1) / `ZoomBlur_03`, `FilmNoise_01`, `KZ_Start_01` / `Light_05_L` | `B_Effectroot` follow / root / `B_Root` follow |

## 2. 파이프라인 — `Tools/EffectPipeline/build_esther_inanna_source_effects.py`

실리안 드라이버 복제 + 파라미터(ARCHETYPE/ACTION/CLIPS/모델/메시 루트) 교체. 실행:
`--acquire → --native --native-first 4354 --native-last 4415 → --install-native (같은 범위) → --project --install (같은 범위)`.

| 단계 | 결과 |
|---|---|
| acquire | 3 stage, emitter 32/10/43, 재질 71, 소켓 계약 `mn_slinn_00.mesh.mn_slinn_00_sk` (bone-Y mirror) |
| native | fresh **35** program(4354..), 재사용 18(실리안/탈것 reviewed), 왜곡 companion 22, 보류 1(`fx_d_de_circle_01_ad` decal prefix 미검토), 텍스처 80 전부 기존 설치본 |
| install | group 4352 hlsli/dispatch/selected + **신규 `Shader_VtxEffectMeshKouku4352.hlsl`**(vcxproj/filters 등록) + `Effect_ArtistMaterial_Tables.inl`. 전체 1729 program / 33 group |
| project | `effect.esther.inanna.npc_att_battle_1_0{1,2,3}.full.restore` 32/15/47 element (duration 4000/2801/4690ms), `Data/Effects/NpcActionCues/NPC_59620.npcactioncues.json` 3 cue(0ms), `EffectCatalog.json` +3 |

드라이버가 실리안 대비 추가로 소유한 것 3건:
- 공용 생성기 시그니처 변화 대응: `reuse_native_programs(selected, roots, source_materials)`, `prepare_distortion(..., resource_root)`, `install(..., resource_root)` — 실리안 드라이버 wrapper는 이제 그대로 못 돈다(TypeError).
- `install_bone_resolution()`: 공용 decoder는 `b_*` 또는 소켓이 참조하는 본만 해석한다. `Bip001-R/L-Hand`는 설치 WModel 본 목록으로 해석(`EXACT_SOURCE_BONE`), acquire/project 양쪽 decode에 적용.
- 다중 anchor notify가 만드는 element 사본(`<id>.bip001-l-hand`)의 occurrence 역참조.

program 범위: 3904 그룹 잔여 16개로는 부족(36 필요)해 4352 그룹의 4354..4415를 사용. 4352·4353은 팀장 cohort. 런타임 상한 4607 유지.

### 2.2 최종: 쿠크 원작은 카메오(sk_magicshield)였다 — strike 복귀 + 카메오 세트 (사용자 결정 09-22)

§1의 "543100 체인 = 쿠크 이난나" 판단은 틀렸다. 실측 사슬:
`COMMONACTION.loa 53204` → `EFTable_CommonActionEffect 108`(ValueA NPC **53500**, ValueB skill 535000)
→ `EFTable_Npc 53500` = `MN_ISTM_00`(보이지 않는 공용 소환체, Wei/Ninav도 53200/53400으로 같은 모델,
`SummonLifetime 30000`) → `MN_ISTM_00.loa` 535000 = Idle 1s + Effect notify뿐, 시각 notify 없음.
따라서 보이는 이난나는 다른 캐스터들이 쓰는 PlaySkeletalMesh 카메오(`MN_SLINN_00_SK_dead` +
`SK_MagicShield`)이며, 543100 체인·`KZBuff_01L`(2~3.5m 소형 zone)은 모르둠(카제로스) 레이드용이다.
`KZ_Start_01` 버스트 링이 25~30m인데 KZBuff 루프가 2~3.5m라 사용자가 "엄청 작다"고 본 것이 단서였다.

- 카메오 원본: `NP_LPDA_01.loa` 4260115 stage 2 (`MN_TSLC_03.loa` 4242529, `MN_ISTM_00-26.loa`도 같은 메쉬 카메오).
  mesh notify(1.89s)에 내장된 `CEFAN_Particle` 5개(`sk01_02` Spine1 5s, `sk01_01` Spine1 1s+Color, `Light_01` Spine1,
  `sk01_04` R-Forearm 0.95s, `sk01_03_LOC_INT` Spine1 2.0s)와 그 뒤 월드 세트
  `Tree_02_02`(+1.94s 0.9s), **`Shield_02_01` 돔(+1.94s 11.2s, 스프라이트 5~7m)**, `Buff01_01` 오라(+1.96s 11.2s),
  **`Buff03` 나무데칼 ×4(+1.97s 11.1s, ±6m 십자)**, `Buff01` 나무/5m 데칼(+2.78s 10.3s).
  LPDA 전용(`FX_MN_LPDA_01-3_X.*`, `Shield_Carck`)은 제외.
- 드라이버 `--profile cameo`: `cameo_actions()`가 합성 stage 하나(`SK_MagicShield` 4.033s)를 만든다. 본 부착 5개는
  `vehicle.skeletal_mesh_particles()`로 뽑은 CEFParticleData 블록을 PlayParticleEffect 헤더 뒤에 붙인 합성 notify,
  월드 notify는 −1.89s 재타이밍. mesh 스폰 transform이 payload에 없어(LPDA/TSLC 동일 tail) 돔·오라·나무는
  이난나 루트(0,0,0)로 재중심, Buff03 ×4는 원본 ±6m 유지 — 이 배치는 **추론**이다.
- 결과: `effect.esther.inanna.cameo.npc_sk_magicshield.full.restore` **127 element**(Spine1 follow 20, R-Forearm 7,
  root 100), duration 19113ms. native fresh 32(**4430..4461**, 4398..4415로는 부족), 재사용 16, 보류 1
  (`fx_j_de_ring_01_ad` 링 플래시 데칼 0.5s — 지속 마법진 데칼 `fx_d_de_unlit_01_04_tr` 1m/6s는 포함).
  cue 문서는 magicshield 1개만 참조. 543100 문서 3개는 catalog에 남기되 cue에서 빠져 Loader 준비 대상이 아니다.
- catalog `esther.strike` → `"npc_sk_magicshield"`, Server strikeMs → 4100 복귀.

### 2.3 최종 2차: LPDA 월드 세트 → 쿠크 전용 `d_` zone 세트 (사용자 지적 09-22 "이건 일리아칸 4관문 히든 이난나")

- §2.2의 Shield 돔/나무데칼은 `NP_LPDA_01`(일리아칸 4관문 히든) 연출이었다. 쿠크 에스더 전용으로 남은 미참조 계열은
  `d_` 접두(`par_d_slinn_00-4_teleport_01/02`, `par_d_slinn_buff02_01s/02l/02l2/02l3/03e/03e1`, `par_d_slinn_sk01_*`).
  전수 probe(`out/InannaFX20260922/unused_probe.log`): buff02 = 14m 원형 데칼/스프라이트 루프(11s 수명)+중앙 `fm_d_tree_03`
  나무+잎 start/loop/end 세트. `par_d_esther_link_inanna_01/03/11`(2~7m 잎/나무 버스트)은 역할 미상으로 미사용.
- 드라이버 `KOUKU_WORLD`: 0.0s teleport_01, 2.0s buff02_01s + 02l/02l2/02l3(loop 15s), 4.03s teleport_02, 17.0s 03e/03e1.
  전부 이난나 루트(0,0,0). **loop 15s는 사용자 선택 상수(`KOUKU_ZONE_LOOP_SECONDS`)**, 원본 근거 없음.
  본 부착 5개(§2.2)는 유지. LPDA 월드 notify는 더 이상 복제하지 않는다.
- 결과: **102 element**(root 75, Spine1 20, R-Forearm 7), duration 47000ms. native fresh 7(4463..4469), 재사용 33,
  보류 3: 14m 링 플래시 데칼 `fx_j_de_ring_01_ad`(0.5s), `teleport_01 emitter_5`·`buff02_02l2 emitter_26`
  (원본에 Lifetime 모듈이 없는 무한 수명 sprite — portable 런타임 carrier 규격 위반으로 문서 전체가
  fail-closed됐던 원인. driver가 `SOURCE_EMITTER_HAS_NO_LIFETIME_MODULE`로 제외한다. 이 검사는 installed-material
  reuse 단계 **뒤**에 둬야 한다; 앞에 두면 재사용 매칭이 deferred에서 도로 꺼낸다).
- **사용자 확인(09-22): "이 이펙트가 쿠크에서 쓰는거 맞음".** 마법진 지속 15s는 그대로 튜닝값.

### 2.1 (폐기) 지속 마법진(zone) 추가 — 사용자 지적(09-22) "원작은 마법진을 깔아두고 가는데 터지고 끝난다"

- 원인: 액션 543100 notify에는 시작 버스트 `Par_S_SLINN_KZ_Start_01`(1_03 0.29s, 전부 ~1s 수명)만 있다.
  지속 링 `Par_S_SLINN_KZBuff_01L`(루프)·`KZBuff_01E`(종료)는 어떤 `.loa/.db/.xml`도 참조하지 않으며
  (LPK 전수 grep 0건), `EFTable_SkillBuff` 543100(`Archetype Inanna_001`, `Duration 20000`,
  `BuffFXApply`)의 buff FX archetype(UPK)이 켜는 것으로 판단.
- 처리: 드라이버 `zone_actions()`가 KZ_Start notify를 복제해 payload의 ParticleSystem FString만 바꾼 합성
  notify 2개(`…notify-007-kzloop` 0.29s d=20.0 / `…-kzend` 20.29s)를 stage 2에 추가한다.
  근거는 `out/InannaFX20260922/synthetic_zone_notifies.json`. 배치는 KZ_Start와 같은 root snapshot.
- 결과: 1_03 문서 47 → **71 element**, duration 30290ms. native fresh 8(4390..4397), 재사용 53.
  zone 루프는 level 소유·NATURAL 정책 spawn이라 NPC despawn(3.7s) 뒤에도 root snapshot으로 남는다(코드 구조 확인, 화면은 사용자 확인).
- 20초는 buff 1레벨 duration이다(3레벨 15000). 원작이 zone 링을 다른 길이로 끝내면 `ZONE_BUFF_DURATION_SECONDS`만 조정.

## 3. 런타임 연결

- `Data/Actors/NpcCatalog.json` NPC_59620 `esther.strike`는 최종 `"npc_sk_magicshield"`(§2.2). 체인 `[1_01,1_02,1_03]`은
  `esther.allied`에만 남는다.
- Server `EstherSkillSystem.h` INANNA strikeMs 최종 **4100**(121f). 체인 시도 중에는 3700이었다.
- `Level_Loading.cpp` Esther roster 선준비 목록에 `NPC_59620` 추가(cue target을 Loader worker에 등록해야 첫 재생 거부가 없다).
- `Effect_PresentationService.cpp` `Requires_SourceBoneImportScaleNormalization`에 `effect.esther.inanna.` 추가(NPC 본 0.01 basis).
- NPC_59504(니나브)는 cue 문서가 없어 목록에 넣지 않았다(누락 문서는 Loader isolate).

## 4. 검증

- 실행함: 드라이버 전 단계 exit 0, 투영 문서 감사(§1 표와 시각·attachment·position 일치, 양손 follow ×2 사본 확인), 변경 JSON parse, `git diff --check`.
- 사용자 화면 확인(09-22): 카메오 + `d_` zone 세트가 쿠크 원작과 일치한다고 확인. 마법진 크기/15s는 추가 지적 없음.
- Debug Product 빌드 PASS — 체인 시도 `20260922T080106838Z`, 최종 카메오 `20260922T091850155Z-debug-product.json`
  (Server OBJ 53, Client CSO 6 — 4352/4416 그룹 Mesh/Particle FX, 에러 0).
- 미실행: 사용자 화면 확인. Character Select → F1 `Esther Skill (Debug)` → `Summon Inanna`.

## 5. 남은 경계

- 보류 1: `fx_m_mi_04.fx_m.fx_d_de_circle_01_ad` 바닥 decal(1_03 KZ_Start) — decal PS prefix 미검토, 실리안 Exp_01 decal과 같은 부류.
- 소환 등장(543150 `Par_S_SLINN_Teleport_01/02`, SDSD 포탈)과 퇴장(543120 CNLR despawn)은 NPC spawn/despawn action이라 이번 strike 문서에 포함하지 않았다. 543100 stage 0의 LRSA 텔레포트가 등장을 대신한다.
- 사운드 cue(`EstherActionSoundCues.json`) 미연결.
- 니나브는 별도 작업(FX_ESTHER_PPNN_00, 541700 체인 `Att_Battle_8_01 → 8_10`).
