# 2026-09-20 에스더 실리안 이펙트 원작 복원 RESULT

작성자: JS · branch `feature/player-stance-cancel-commit`

목표: 손으로 저작한 실리안 이펙트(V2, 44 leaf)를 쿠크·플레이어 복원과 같은
원본 기반 복원으로 교체한다.

**현재 상태: 완료. 원본 notify 기준 full-restore 문서 2개가 첫 검격부터 재생되고 위치·방향·검
소켓 follow까지 사용자 확인(09-21). 남은 결함은 §6.**

## 1. 확정한 원본 근거

| 항목 | 값 |
|---|---|
| NPC | `NPC_59030` (모르둠_실리안), 애니 그룹 `NP_LRSA_00`, 몸체 `np_lrsa_00.mesh.np_lrsa_00_sk` |
| 스킬/Action | 542600 "모르둠 연합군 스킬 실리안" — `XmlData/Action/NP_LRSA_00.loa` |
| 클립 | stage 2 `npc_evt1_sk_swordofchampion_bk` 5.2s / stage 0 `npc_att_battle_7_01` 3.13s |
| FX 패키지 | `FX_ESTHER_LRSA_00` + 공용 `FX_CM_01` / `FX_CM_02` / `FX_POST` |

### 원작 notify (stage 2, PlayParticleEffect 29행)

| 시각 | 시스템 | 배치 |
|---|---|---|
| 0.0 | `Sword_01_loop` (4.5s) | **`R_Weapon` 소켓 follow** (→ `b_weapon_00`, roll 90°) |
| 0.8 | `FilmNoise_01`, `Teleport_01`, `ConcaveDis_Z_04`(×1.3), `Light_05_L`(y 1m) | 루트 snapshot |
| 1.2 / 2.0 | `Sword_02_loop` (1.85s) / `Sword_03_loop` (1.0s) | `R_Weapon` follow |
| 2.9 | `SwordSwing_01` | 전방 1m, 높이 2.5m |
| 3.10/3.15/3.20 | `Exp_01` ×3 | **전방 1 / 7.5 / 14m** (+왜곡 3/10/17m ×3, 광원 y 1m) |
| 4.20/4.25/4.30 | `Exp_02` + `Exp_03_dust` ×3 | 전방 1 / 7.5 / 14m (dust y 0.3) |

폭발은 50ms 간격으로 전방 세 지점을 훑는다. 손저작본이 빈약해 보이던 이유가 이 구조다.

## 2. 파이프라인 — 원본 Action → kouku full-restore

`Tools/EffectPipeline/build_esther_silian_source_effects.py` (`--acquire → --native →
--install-native → --project [--install]`). 전부 기존 도구 조합이며 드라이버가 소유한 것은
evidence root, program 구간, 소켓 계약, 클립 표, 두 문서·cue·catalog 행뿐이다.

| 단계 | 재사용한 도구 | 산출 |
|---|---|---|
| 원본 notify | `extract_action_effect_notifies.py --action-id 542600` | `out/SilianFX20260920/actions/NP_LRSA_00.action-effects.json` (PlayParticleEffect 58) |
| 소켓 계약 | 탈것 `socket_contracts`와 같은 규약(bone-Y mirror) | `source_socket_contract.json` — 15 socket, `r_weapon → b_weapon_00` |
| acquire | `build_kouku_gate1_full_restore.acquire` (stage별 `SELECTED`) | 클립당 215 emitter / 47 재질, `stages/<clip>/` |
| native | `build_vehicle_skill_effects.native_environment` + `build_kouku_pattern_native.prepare` | program 39 신규(3912..3951) + 탈것 6 재사용, 왜곡 companion 16 |
| install | `build_vehicle_skill_effects.install_native` | `Effect_ArtistMaterial_Tables.inl`, group 3904 HLSLI 4개 |
| project | `build_kouku_gate1_full_restore.project` (attachment·local transform 그대로) | `Data/Effects/Authored/effect.esther.silian.<clip>.full.restore.effect.json` ×2, 각 212 element |

드라이버 보정 4건 (전부 evidence에 기록):
- `exact_texture_reuse`: 탈것 3763(`fx_d_pa_master_01_019_dt_tr`)은 09-14 도구가 MIC 자체
  native 텍스처 표를 안 읽어 부모 기본 `fx_c_cloud_015`로 굳음. 현재 도구는 `fx_d_noise_009`.
  텍스처가 다르면 재사용 거부 → 새 program. 탈것 설치본은 손대지 않음.
- 재사용 program은 왜곡 pass를 다시 내리지 않는다(`prepare_distortion` 래핑). 재실행 시 새
  program 번호는 reviewed 최대치 다음부터(`first` 자동 상향) — 3912 충돌 방지.
- candidate 헤더 설치는 보류 program을 뺀 사본으로(탈것도 같은 assert에서 죽은 채 넘어갔었다).
- `efparticlemodulelocationonground`를 같은 객체로 두 번 참조한 emitter(Exp_01 1개) → 1개로.
  portable 런타임 상한 1.

### 결과 수치

| 항목 | 값 |
|---|---|
| 문서 | 2개 × 212 element (snapshot-root 191 · `b_weapon_00` follow 21), version 15 |
| cue | `Data/Effects/NpcActionCues/NPC_59030.npcactioncues.json` — 클립당 1 cue(0ms) |
| catalog | `EffectCatalog.json` +2행 (이전 11행 제거) |
| 보류 | `fx_d_de_unlit_01_05_tr` decal ×3 occurrence — PS `c478df27…` "Unreviewed source decal prefix" |
| 텍스처 84 / 메시 9 | 전부 기존 설치본 재사용(메시는 `Effect/Esther/Thirain/Meshes/FX_SM_0x/`). Drive 전달 없음 |
| native 전체 | 1235 program / 23 group / 46 carrier, 쿠크 구간 여유 3952..3967 (16) |

### 그 밖의 변경

- `Data/Effects/V2/Bindings/NPC_59030.effectv2bindings.json` bindings 44 → 0 (사용자 지시).
  원본은 `out/SilianFX20260920/NPC_59030.effectv2bindings.backup.json`.
- Client 런타임 (첫 세션): `CNpcActionEffectCueDocument`(신규), `CNpc::Arm/Update_ActionEffectCues`,
  `CEffectV2Runtime::Resolve_ArchetypeId` 노출.
- Client 런타임 (09-21, C++ 2건):
  1. `Level_Loading.cpp` — Character Select/Valtan/Kouku 진입 시 Esther roster 3종의 cue 문서
     target을 player cue와 같은 Loader worker에 등록. 첫 검격이 "not prepared"로 거부되던 문제.
  2. NPC 소켓 follow — `EFFECT_LEVEL_PLACEMENT_SPAWN_DESC::pAnchorOwner`(weak `CNpc`) →
     `EFFECT_SPAWN_DESC/ACTIVE_EFFECT::pNpcAnchorOwner` → `EFFECT_OWNER_VIEW::pNpcAnchors`.
     owner(예산·수명)는 계속 level이고 NPC는 anchor 모델만 공급한다. `Synchronize_FollowAnchors`의
     world-root 분기에서 NPC anchor가 있으면 `Resolve_SourceAnchors`를 수행한다.
     `Requires_SourceBoneImportScaleNormalization`에 `effect.esther.silian.` 추가(NPC 본 0.01 basis).
     `CNpc::Update_ActionEffectCues`가 `desc.pAnchorOwner = shared_from_this()`.

## 3. 첫 세션이 틀린 것

1. "`extract_ue3_material_shader_maps.py`에 디스커버리 모드가 없다" — `build_kouku_pattern_native.py`가
   재질→cache→DXBC→HLSL을 한 번에 한다. 매니페스트 손작성 불필요.
2. MIC static set(offset 61)의 `fbffffa5…`를 base ID로 써 cache 0 hit. base ID는 **부모 Material
   export tail[16:32]**.
3. 시스템별 라이브러리 문서(showtime 방식) + 29 cue — notify의 local transform/소켓/basis −90을
   전부 버려 NPC 루트에 겹쳤다. 원본 Action 파일로 full-restore 투영이 정답.

## 4. 진단 교훈

- element의 `material.sourceProfile`이 비면 drawable 실패 → "Product target is not prepared".
  텍스처가 디스크에 있는 것과 문서가 참조하는 것은 별개.
- cue target 등록이 클립 시작 시점이면 첫 재생은 거부된다(문서당 준비 1~4초). Loader 등록 필수.
- 검증된 몬스터 클립종료 이펙트는 attachment `snapshot root, basis −90`를 유지한다. 라이브러리
  방식으로 attachment를 끄면 basis가 사라진다.

## 5. 설계 결정과 경계

사용자가 B안(NPC action에 V1 cue 경로 추가)을 선택했다. 새 런타임을 만들지는 않았다 — 기존
`Spawn_LevelPlacement` 훅을 클립당 문서 하나로 쓰고 anchor 모델만 NPC에서 읽는다. cue 문서가
없는 archetype은 종전대로 V2만 탄다. AGENTS.md 단일 런타임 경로 규칙과 `effect-v2-runtime-option-b`
팀장 합의 대기 건은 그대로다.

## 6. 알려진 결함 / 후속

- Exp_01 바닥 데칼 ×3 보류(decal PS prefix 미검토).
- 탈것 3763 텍스처 불일치(§2). 탈것 문서 재투영 여부는 별도.
- 소켓 계약의 bone-Y mirror는 탈것 규약을 그대로 가정했다. 검 발광 방향이 틀리면 여기부터.
- Sword loop의 `d=4.5s`는 클립(5.2s)보다 짧고 문서 duration 9.5s는 폭발 여운 포함.

## 7. 실행한 검증 / 하지 않은 것

- 실행함: 드라이버 전 단계 exit 0, 후보 문서 감사(212 element·attachment 분포·위치 표 §1과 일치),
  변경 JSON parse, `git diff --check`.
- 사용자 확인(09-21, 데이터만 반영·V2 off·C++ 이전): 두 번째 검격부터 복원본 재생, **위치·방향
  원본과 일치**. 검 발광(21)은 미표시 예상.
- Debug Product 빌드 (C++ 2건 포함) PASS — `out/BuildPipeline/runs/20260920T153930574Z-debug-product.json`,
  `Client/Bin/Debug/Client.exe` 09-21 00:39. (직전 시도는 Server.exe 실행 중이라 output-lock
  preflight에서 FAIL — 로컬 Server를 내리고 재실행.)
- 사용자 확인(09-21, C++ 2건 반영 빌드): "잘 나와" — 첫 검격 즉시 재생과 검 발광 follow 포함 승인.
- `Validate-EffectSources.ps1`은 이 변경 이전부터 쿠크 문서로 실패한다(팀장 영역).
