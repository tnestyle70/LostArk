# 2026-09-07 에스더 니나브·이난나 모델 추출 RESULT

작성자: JS · branch `feature/effect-v2-kouku-card-dance`
선례: `.md/JS/08-15/2026-08-15_ESTHER_SILIAN_SUMMON_EXTRACTION_RESULT.md`,
`.md/JS/08-20/2026-08-20_ESTHER_WAYE_BAHUNTUR_SLOTS_RESULT.md`

## 1. 원본 근거 (언팩 LPK TableData 실측)

| 항목 | 니나브 | 이난나 |
|---|---|---|
| EpicSkill 세트 | 2010/2011/2020/2021 슬롯1 Action `53203`, `SkillMC=EstherSkill.Ninave` | 같은 세트 슬롯3 Action `53204`, `SkillMC=EstherSkill.Inanna` |
| 소환 NPC | `59504`(모르둠_니나브; 59500 나로크, 59506~ 종막 동일 모델) | `59620`(종막_이난나) |
| 모델 | `EFDLChar_MN_PPNN_00.MN_PPNN_00` + 무기 `WP_MN_PPNN_00`(LookInfo 소켓 `B_WP_08`) | `EFDLChar_MN_SLINN_00.MN_SLINN_00`(무기 없음) |
| 액션 그룹 | `MN_PPNN_00.loa` | `MN_SLINN_00.loa` |
| 연합군 스킬 | `541700` → `Att_Battle_8_01` → `Att_Battle_10` | `543100` → `Att_Battle_1_01` → `1_02` → `1_03` |
| 에스더 일체형 클립 | `sk_parkunas` 145f 4.83s (+ `sk_esthereffect_2`) | `sk_magicshield` 121f 4.03s (+ `sk_esthereffect_2`) — 몬스터 loa 미참조, psa 전수 조사로 확인 |
| 증원 스킬 | `541710` → `Att_Battle_8_03/04/05` | `543110` → `Att_Battle_2_01/02/03` |
| 디스폰 | `541720` (`Idle_Battle_1`, `Dead`) | `543120` (`Idle_Normal_1`) |
| idle | `idle_battle_1` | `idle_normal_1` (battle idle 없음) |

## 2. 구현 완료

- 추출: `umodel_lostark_v7 -groups` → `_export_esther_ni_g` (MN_PPNN_00 127 obj,
  MN_SLINN_00 210 obj, WP_MN_PPNN_00 32 obj, 전부 exit 0).
- `npc_lookinfo.py` 59504/59620 → `esther_ni_npcs.json` (둘 다 self-rigged master).
- `build_npc.py`에 `--wp-socket <name>` 옵션 추가(buildScript, Git 밖). 니나브 body는
  wp 소켓 4개(`b_wp_08/b_wp_01/wp_2/wp_1`)라 개수 불일치로 무기가 붙지 않던 것을
  LookInfo 소켓 `b_wp_08`(bone `b_wp_2`)로 지정해 활 병합.
- `build_npc_animset.py --clips`로 연합군·증원·idle 클립만 쿠킹, `ModelAssetConverter
  --no-auto-textures`로 변환.
- `cook_npc.py` 바디 쿠킹: 59504 8메시 41,470버텍스(무기 포함, 35MB), 59620 13메시
  58,204버텍스(98MB).
- 산출물 로컬 Resources 배치:
  - `Character/NPC/Npc_59504/Npc_59504.wmodel`, `Character/NPC/Npc_59620/Npc_59620.wmodel`
  - `Character/NPC/AnimSets/MN_PPNN_00/MN_PPNN_00.wmodel` 클립 `npc_att_battle_8_01`,
    `npc_att_battle_10`, `npc_att_battle_8_03/04/05`, `npc_idle_battle_1`
  - `Character/NPC/AnimSets/MN_SLINN_00/MN_SLINN_00.wmodel` 클립 `npc_att_battle_1_01/02/03`,
    `npc_att_battle_2_01/02/03`, `npc_idle_normal_1`
- `Data/Actors/NpcCatalog.json` 75→77 entry: `NPC_59504`, `NPC_59620`
  (`shaderProfile=esther`, `esther.strike` 체인 = 연합군 클립 순서, `cutinMovie` 없음).

## 3. 자동 검증 (실행함)

- `validate_wmodel` 4개 RESULT: OK
- `Compare-Skeletons` ALIGNED (108/108, 120/120), `Compare-InverseBind` IDENTICAL(maxDelta 0)
- `NpcCatalog.json` parse OK, `Publish-WorldGameplay.ps1 -Mode Validate` 전 Area 통과
- `git diff --check` clean

## 4. 수동 확인 — 사용자 (미실행)

Client 재시작 후 F1 Effect Tool `Attach...` 목록에서 `NPC_59504`/`NPC_59620` 스폰.
확인할 것: 니나브 활 위치(`b_wp_08`), 이난나 재질(`_trn`/eyelashes 투명 슬롯), 연합군
클립 체인 동작과 idle 자세.

## 5. 범위 제외 / 후속

- 컷인 프레임(`EstherSkill.Ninave/Inanna` MC) 미추출 → `cutinMovie` 없음.
- Server `ESTHER_ROSTER`·Valtan 로더 선로드·F1 Preview 버튼은 기존 3종만. 슬롯 배정은
  스킬 연출 작업에서 결정.
- LookInfo MaterialInstance 재도색은 08-21 규칙으로 cook_npc가 적용(로그 확인 안 함).
  이난나 `tex_2048` 고해상도 폴더는 LookInfo 참조대로만 사용.
- 이펙트(`FX_ESTHER_*` 니나브/이난나 패키지)와 사운드는 미추출.
- `AGENTS.md`의 "현재 75종" 표기는 77종이 됐으나 이번 변경에서 수정하지 않았다.

원본은 Smilegate 저작물 — 스테이징(`_export_esther_ni_g`, `_cook_esther_ni`)과 FBX는
Git에 올리지 않는다.

## 6. 추가: 텍스처 보정과 Effect 리소스 폴더 (2026-09-07 후속)

### 6.1 바디 텍스처 보정
- 니나브 머리: `mn_ppnn_00_hair_d`는 R=음영/B=투톤 마스크 팩. MI 원본값은
  base (1, 0.751, 0.492) 금발 + twotone (0.641, 0.034, 0) 적색이지만 마스크가 연속
  그라데이션이라 lerp 규칙 그대로면 절반이 적색이 된다. 사용자 판정으로 PROJECT_TUNED
  베이지 (0.93, 0.85, 0.72) / (0.66, 0.56, 0.44), 마스크 ×0.5 적용. 후보 파일은
  `_fixed_tex_esther/mn_ppnn_00_hair_d.{blonde,blonde2,silver,beige}.tga`.
  니나브 LookInfo 변형 15개 전부 같은 Hair_MI라 은발 원본 데이터는 없다.
- 니나브 `dress1` emissive 추가. 이난나: 머리 투톤 베이크 (0.966,0.476,0.482)/(0.801,0.301,0.452),
  눈 `pc_mg_00_eyebase_d`+`mn_slinn_00_eyeiris_d` 합성, 슬롯 1~5·trn emissive 6장 추가.
- 함정: PowerShell `Copy-Item -Recurse`가 기존 대상 폴더 안에 중첩 복사해 한 번 구 파일이 배포됐다.
  내용물 복사(`cp -rf src/. dst/`)로 교정.

### 6.2 Effect 리소스 폴더 (`Effect/Esther/Ninave`, `Effect/Esther/Inanna`)
- 원본 FX 패키지: 니나브 `FX_ESTHER_PPNN_00`(3939 export) + `FX_MN_PPNN_00`(3607),
  이난나 `FX_ESTHER_SLINN_00`(6021). 논리명은 `extract_ue3_placements.obfuscate_package_name`으로
  물리 파일명을 역대조해 확정.
- umodel은 ParticleSystem을 export하지 않으므로 패키지 import table에서
  staticmesh/materialinstanceconstant/material 참조(closure)를 뽑아 `-obj=` 단위로
  `-gltf -dds -groups -uncook` export (430/431, `enginematerials.defaultparticle` 제외).
  파라미터가 빈 MIC 42개는 umodel이 무시하므로 부모 Material3를 별도 export해 `.mat`의 텍스처 목록을 사용.
- 메시 변환: `ModelAssetConverter <gltf> -o <wmodel> --pretransform --scale 100 --no-auto-textures`
  (Wei 폴더의 `fm_a_hemisphere_012.wmodel`과 바이트 동일 확인).
- 결과: Ninave 메시 39 / 텍스처 308(dds 307 + tga 1), Inanna 메시 39 / 텍스처 289.
  `Textures/<원본 패키지>/<이름>.dds`, `Meshes/<원본 패키지>/<이름>.wmodel` 레이아웃(Wei와 동일).
  Effect Tool V2 Resource Library 도메인은 `Esther/Ninave`, `Esther/Inanna`로 자동 분리된다.
- 파르쿠나스 에스더 스킬 ParticleSystem 17개 수치 전수: `2026-09-07_NINAVE_PARKUNAS_FX_DETAIL.md` (`dump_parkunas_fx_detail.py`).
- ParticleSystem→emitter→mesh/material/texture 구성표: `2026-09-07_ESTHER_NINAVE_INANNA_FX_EMITTER_TABLE.md`
  (`extract_ue3_particle_graph.py` graph 기반, LOD0 1484 emitter).
- 스크립트: 스크래치패드 `export_esther_fx_closure.py`, `deploy_esther_fx.py`, `build_esther_fx_emitter_table.py`, 스테이징
  `_export_esther_ni_fx` (Git 밖). 마스터 머티리얼 기본 파라미터가 끌어온 무관 텍스처
  (`ENGINE_MI_SHADERS`, `WP_WGDH_01`, `BG_RHD_TREE_A`, `ITR_00511`)가 소량 포함됐다.

### 6.3 니나브 파르쿠나스 Effect V2 1차 저작 (2026-09-07) — 폐기
- 본 궤적 추정 타이밍으로 leaf 16개·바인딩 29행을 만들었으나 사용자 확인 결과 부적합해 전부 삭제했다.
  생성기는 스크래치패드 `author_ninave_parkunas.py`에만 남아 있다.
