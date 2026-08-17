# 2026-08-17 발탄 휠윈드·검격 저작 작업 시트

Effect Tool에서 지금 바로 만들 수 있도록, 원본이 각 동작에 실제로 쓴 파티클 시스템과
그 시스템이 참조한 DDS/메시를 실물 존재까지 확인해 정리한다.

조회 정본은 `Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json`
(sourceSystems 193, assets 855)이고, clip 축으로 조인했다. catalog의 clipName은
`Att_Battle_20_03` 표기이고 patternbindings는 `mesh_att_battle_20_03` 표기라
`mesh_` 접두사를 떼고 대소문자 무시로 맞춰야 한다.

## 1. 동작 -> clip -> 원본 파티클 시스템

```text
valtan.attack.whirlwind.windup     mesh_att_battle_20_02   시스템 0개
valtan.attack.whirlwind.active     mesh_att_battle_20_03   시스템 4개
valtan.attack.whirlwind.recovery   mesh_att_battle_20_04   시스템 3개
valtan.attack.swing.active         mesh_att_battle_1_01    시스템 6개
valtan.attack.swing.recovery       mesh_att_battle_1_02    시스템 4개
valtan.attack.four-slash.windup    mesh_att_battle_10_01   시스템 10개
valtan.attack.four-slash.active    mesh_att_battle_10_02   시스템 12개
```

`whirlwind.windup`은 원본에도 파티클이 없다. 휠윈드의 시각 효과는 `active`부터 시작한다.

### 1.1 휠윈드 active — 4 시스템

```text
par_n_rpbf_wwind_01     FX_MN_RPBF_00_N   occ=27    회오리 본체
par_n_mrhg_trail_01     FX_BS_01          occ=72    궤적
par_n_rpbf_dust_01_01   FX_MN_RPBF_00_N   occ=38    지면 먼지
par_mp_light_05_l       FX_CM_02          occ=693   광원(공용)
```

### 1.2 검격 active — 6 시스템

```text
par_o_boeh_trail_01_1   FX_BS_01          occ=221   검격 궤적 본체
par_d_trail_21_loc_int  FX_BS_01          occ=12    보조 궤적
par_s_rpbf_atk_02_1     FX_MN_RPBF_00_S   occ=16    타격
par_d_dust_002_pr       FX_CM_00          occ=19    먼지
par_d_rpbf_sk99_01_loc_int  FX_MN_BOSS_01 occ=6
par_mp_light_05_l       FX_CM_02          occ=693   광원(공용)
```

### 1.3 검격 recovery — 지면 데칼이 여기 있다

```text
par_s_rpbf_stone_01_1   FX_MN_RPBF_00_S   occ=352   지면 파편/데칼
par_s_rpbf_stone_01_2   FX_MN_RPBF_00_S   occ=22
par_s_rpbf_atk_01_1     FX_MN_RPBF_00_S   occ=27
par_o_rpbf_atk_09_01    FX_MN_RPBF_00_O   occ=21
```

`par_s_rpbf_stone_01_1`의 머티리얼 참조에 `fx_d_pa_decmaster_01_tr`(decal master)와
`fx_e_me_rock_01_ma`(rock mesh)가 있다. 요청한 decal은 이 계열이다.

### 1.4 4연참 — 3연 공격의 실제 이름

`valtan.attack.four-slash.*`가 연속 참격이다. `triple-counter`는 카운터 무력화 기믹이라
연속 공격이 아니다. 4연참 active는 시스템 12개로 가장 무겁고,
`par_o_rpbf_atk_01_02 / _03 / _05 / _08`이 타 단계별 참격이다.

## 2. 실제로 바인딩할 DDS — 실물 확인 완료

Resource Library의 `Authoring Category = Valtan`에서 그대로 보인다.
Texture Kind 콤보로 좁히면 후보가 346개에서 20~40개로 준다.

```text
파일                 폴더        크기      포맷    쓰임               Texture Kind
fx_d_atypical_028   FX_TEX_02   128x128   DXT1   휠윈드 본체        Base / Sprite
fx_a_trail_011      FX_TEX_00   256x256   DXT1   검격 궤적 본체     Trail / Beam
fx_d_atypical_009   FX_TEX_02   256x256   DXT1   검격 궤적 보조     Base / Sprite
fx_c_noise_009      FX_TEX_01   256x256   DXT1   궤적 왜곡          Noise / Distortion
fx_a_trail_006      FX_TEX_00   256x256   DXT1   보조 궤적          Trail / Beam
fx_d_atypical_006   FX_TEX_02   128x128   DXT1   보조 궤적          Base / Sprite
fx_e_fluid_007      FX_TEX_03   256x256   DXT5   먼지               Fluid / Water
fx_e_noise_008      FX_TEX_03   256x256   DXT5   먼지 왜곡          Noise / Distortion

fm_m_trail_002.wmodel                             휠윈드 궤적 메시
```

DXT1은 알파가 없으므로 Base에 넣으면 가산 블렌드 전용이다. 먼지 두 장만 DXT5라
알파 블렌드와 Mask/Dissolve에 쓸 수 있다.

## 3. Effect Tool 작업 순서

```text
1  New Effect            valtan.whirlwind.active  (또는 valtan.swing.active)
2  Element Type          Sprite Particle 또는 Trail / Ribbon
3  Create Element
4  Selected Element Resource Set 에서 Base 카드 선택
5  Resource Library      Authoring Category = Valtan
                         Textures + Texture Kind 로 위 표의 파일 선택
6  Bind Selected
7  Noise 카드에 fx_c_noise_009 또는 fx_e_noise_008
8  Effect Detail 에서 Lifetime / Scale / Color 조정
9  Save Changes
```

4~6은 `4872bde1` 이전에는 빈 슬롯 카드가 아예 그려지지 않아 불가능했다.

## 4. 아직 막혀 있는 것 — 게임에서 재생

만든 Effect를 발탄 패턴에 붙이려면
`Data/Animation/Authored/Valtan/Valtan.patterneffects.json`에 binding을 추가해야 하는데,
`CValtanPatternEffectBindingDocument::Parse_Text`의 `Has_ExactProperties`가
binding마다 **정확히 15개 필드**를 요구한다.

```text
본질 3개      patternId, actionId, effectAssetId
Track A 12개  bindingId, semanticStageId, effectDocument, animationBindingDocument,
              sourceCatalogDocument, sourceParticleResourceCatalogDocument,
              sourceEvidence, sourceBranch, modelBoneEvidence,
              sourceOccurrences, failClosedOccurrences, productAdmission
```

`sourceEvidence`는 원본 UPK 추출 receipt의 SHA 핀이라 직접 만든 Effect에는 존재할 수 없다.
현재 binding 1건이 이 필드들 때문에 혼자 약 30 KB다.

이 게이트 제거는 Codex 세션이 진행 중이며 해당 파일
(`Client/Private/AnimationSkillBindingDocument.cpp`, `Client/Private/Effect_Tool.cpp`)이
미커밋 상태라 이 작업 단위에서는 건드리지 않았다.

게이트가 풀리기 전까지는 Effect Tool의 Model View 미리보기로 형태를 확정하는 데까지
진행할 수 있고, 게임 내 재생은 그 뒤다.

## 5. 조회 재현

```text
Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json
  sourceSystems[].clipNames  를 mesh_ 접두사 제거 + 소문자로 정규화해
  Valtan.patternbindings.json 의 clip 과 조인한다.
  assets[].sourceSystems 는 "<package>.<group>.<objectName>" 형식이므로
  마지막 마디로 시스템을 역참조하면 그 시스템이 쓴 텍스처/머티리얼이 나온다.
```
