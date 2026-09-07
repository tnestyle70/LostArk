# 쿠크 월드 오브젝트 물리 리소스 기록 — 2026-09-07

## 저장 상태

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 formatVersion 3, revision 121이다. objectResources 10개(물리 모델 8개 + 기존 sequence alias 2개), 새 template/instance 각 11개를 추가했다. 기존 template 56개와 instance 60개는 원문 배열에 append하여 보존했다.

새 instance의 기본 anchor는 `WORLD`다. `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의 `player.spawn.kakul.party01` 위치 `(3.28999996, 8.64000034, -10.6899996)`에서 월드 +Z 2m를 더한 `(3.28999996, 8.64000034, -8.6899996)`를 저작 시작점으로 사용했다. 이 +2m는 PROJECT_TUNED 저작 편의를 위한 값이며 원본 패턴 위치라고 주장하지 않는다.

카드 추가 상태는 `카드_들썩임`(1.2초, 최대 0.12m 높이, Z축 ±4도), `카드_뒤집힘`(1초, Y축 180도)이다. `공_튀기기`는 2초, velocity `(0,6,0)`m/s, acceleration `(0,-6,0)`m/s²로 한 번 상승·하강한다. 기본 count=1, intervalMs=200, seed=1이고 공 중심은 반지름 0.47080475m 높이에 둔다. 이 상태 값도 PROJECT_TUNED이며 원본 연출 시간 복원값이 아니다.

## 실제 리소스와 식별 근거

모든 경로는 `Client/Bin/Resources` 기준이다. 같은 표의 `textures/...`는 해당 WModel이 있는 폴더 기준이며, WModel의 CMaterial binding에 저장되어 있다.

### 월드오브젝트_카드

- 저장 ID: `world.object.kouku.card`
- 모델/alias: `Character/KoukuSaton/MN_RHOC_00/MN_RHOC_00.wmodel`
- 근거: Npc 480642 `뒤집혀진 빈 카드`; LookInfo `EFDLChar_MN_RHOC_00.MN_RHOC_00` → MN_RHOC_00_SK / MN_RHOC_00_MI / MN_RHOC_00_Ani.
- 단위/배수: modelPreScale=1, scale=2.
- 애니메이션: 14; `mn_rhoc_00_sk.ao_idle_normal_1`.
- 텍스처: `textures/mn_rhoc_00_{d,n,s}.dds`

### 월드오브젝트_조커카드

- 저장 ID: `world.object.kouku.joker_card`
- 모델/alias: `Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel`
- 근거: Npc 480643 `뒤집혀진 조커 카드`; LookInfo `EFDLChar_MN_RHOC_00-1.MN_RHOC_00-1` → 같은 카드 mesh/AnimSet + MN_RHOC_00-1_MI. 재질 variant를 실제 별도 WModel로 cook했다.
- 단위/배수: modelPreScale=1, scale=2.
- 애니메이션: 14; `mn_rhoc_00_sk.ao_idle_normal_1`.
- 텍스처: `textures/mn_rhoc_00-1_{d,n,s}.dds`

### 월드오브젝트_공

- 저장 ID: `world.object.kouku.ball`
- 모델/alias: `Effect/KakulSaydon/Meshes/fx_sm_01/fm_k_ppct_ball_01.wmodel`
- 근거: `FX_MN_RPCT_05_G.par_g_rpct_05_circusball_01_01` 등 TypeDataMesh → `fx_sm_01.fm_k_ppct_ball_01`; 원본 `wp_mn_ppct_00_mi.props.txt`의 texture_diffuse → wp_mn_ppct_00_c.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 0; transform/motion.
- 텍스처: WModel 자체 texture가 비어 있어 `Effect/KakulSaydon/Textures/WP_MN_PPCT_00/tex/wp_mn_ppct_00_c.dds`를 명시 diffuse override한다.

### 월드오브젝트_세토

- 저장 ID: `world.object.kouku.seto`
- 모델/alias: `Character/KoukuSaton/MN_PPCT_00/MN_PPCT_00.wmodel`
- 근거: Npc 480708/480709 `카드미로 세토 로밍형/돌진형`; LookInfo MN_PPCT_00 → MN_PPCT_00_SK_LOC_INT / MN_PPCT_00_Ani. 4 material slot의 실제 diffuse/normal/specular와 두 emissive를 연결했다.
- 단위/배수: modelPreScale=1, scale=1.
- 애니메이션: 68; `Seto_idle_normal_1`.
- 텍스처: `textures/mn_ppct_00_1_{c,n,s}_loc_int.dds`, `mn_ppct_00_1_em.dds`, `mn_ppct_00_2_{c,n,s,em}.dds`, `mn_ppct_00_{c,n,s}.dds`

### 월드오브젝트_칼날

- 저장 ID: `world.object.kouku.cutting_blade`
- 모델/alias: `Effect/KakulSaydon/WorldObjects/CuttingBlade/CuttingBlade.wmodel`
- 근거: MN_ISTM_00-4 Action 4222009 `투명NPC_칼날 발사` → notify SkillEffect 421991302 → Key 12 / ValueA 421991301 → data1 Projectile 421991301.loa → `FX_MN_RPCT_07_V.Par_V_RPCT_Cutting_pjt_01` → TypeDataMesh `fx_sm_01.fm_o_cngn_01`, Required material `mn_cngn_00.mat.mn_cngn_00_mi`. 두 swing mesh는 trail이므로 본체로 등록하지 않았다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 0; transform/motion.
- 텍스처: `textures/mn_cngn_00_{d,n,s,e}.dds`

### 월드오브젝트_갈고리

- 저장 ID: `world.object.kouku.hook`
- 모델/alias: `Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel`
- 근거: Npc 480710 `갈고리`; LookInfo MN_UMAX_00 → MN_UMAX_00_SK / MN_UMAX_00_MI / MN_UMAX_00_Ani.
- 단위/배수: modelPreScale=1, scale=1.15.
- 애니메이션: 9; `Hook_idle_normal_1`.
- 텍스처: `textures/mn_umax_00_d_loc_int.dds`, `mn_umax_00_{n,s}.dds`

### 월드오브젝트_빙고폭탄

- 저장 ID: `world.object.kouku.bingo_bomb`
- 모델/alias: `Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel`
- 근거: Npc 480724 `해골이 그려져있는 폭탄_빙고` → EFDLChar_MN_RHCN_01.MN_RHCN_01; 실제 MN_RHCN_01_SK / MN_RHCN_01_MI / MN_RHCN_01_Ani.
- 단위/배수: modelPreScale=1, scale=2.
- 애니메이션: 5; `Bomb_idle_normal_1`.
- 텍스처: `textures/mn_rhcn_01_d_loc_int.dds`, `mn_rhcn_01_{n,s}.dds`

### 월드오브젝트_빙고

- 저장 ID: `world.object.kouku.bingo`
- 모델/alias: `Map/LV_LUT_MIDNIGHTC_ED/MAP_5F1286085DD9_LV_LUT_MIDNIGHTC_FLOOR03_SM/MAP_5F1286085DD9_LV_LUT_MIDNIGHTC_FLOOR03_SM.wmodel`
- 근거: 기존 LV_LUT_MIDNIGHTC_FLOOR03_SM geometry와 SLOT_000_lv_lut_midnightc_bingofloor01_mi 재질을 재사용한다. 정점 실측 한 타일 304×8×304cm. Npc 480653 이후 빙고 제어용 MN_ISTM_00-4는 투명 actor라 화면용 모델로 등록하지 않았다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 0; transform.
- 텍스처: `textures/1e970b708b4d_lv_lut_midnightc_bingofloor01_d.dds`, `9934688136d4_spec.dds`, `b2378a8f80d6_t_tds_specular04.dds`

### 월드오브젝트_커튼

- 저장 ID: `world.object.kouku.curtain`
- 모델/alias: `world.sequence.instance.curtain_drop`
- 근거: 기존 instance `world.sequence.instance.curtain_drop`, template `sequence.LV_LUT_MIDNIGHTC_ED.curtain_drop`의 11개 curtain placement를 그대로 참조한다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 기존 11 track/key 보존.
- 텍스처: 기존 Map placement/CModel 재질 경로 유지

### 월드오브젝트_룰렛

- 저장 ID: `world.object.kouku.roulette`
- 모델/alias: `world.sequence.instance.8`
- 근거: 기존 instance `world.sequence.instance.8`, template `sequence.LV_LUT_MIDNIGHTC_ED.5` (`roulette`)를 그대로 참조한다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 기존 183 transform key 보존.
- 텍스처: 기존 Map roulette CModel 재질 경로 유지

## 추출 및 CModel 소비

추출 원본은 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/WorldObjectExtraction-20260907`에 있다. 설치된 게임의 `EFGame/data1.lpk` Projectile, `data2.lpk` Npc/SkillEffect, `data3.lpk` Action, `data4.lpk` LookInfo를 기존 `LpkReader`와 `LostArk.Archive.Core.Lpk.LpkDbDecryptor`로 읽었다. 원본 `.loa`와 SQLite DB는 리소스 식별 참고용으로 외부 추출 폴더에만 보존하며 제품 runtime으로 배포하지 않는다.

UModel CLI의 `-export`, 저장소 `Cook-ActorXWModel.ps1`, `ModelAssetConverter.exe`, `retime_wmodel_from_psa.py`를 사용했다. 카드/조커/세토/갈고리/폭탄은 skeleton+clip을 유지하고, 발사 칼날은 원본 glTF 정적 geometry를 CModel WModel로 cook했다. Effect Tool의 particle 런타임을 두 번째 모델 경로로 추가하지 않는다. 새 skinned cook은 기본 ActorX scale-down으로 정점이 m 단위이므로 modelPreScale=1이고, 기존 공/빙고 및 scale100 정적 칼날은 cm WModel이므로 .01이다. 카드/조커2, 갈고리1.15, 빙고폭탄2 배수는 해당 Npc ModelSize의 200/115/200에서 가져왔다.

세토는 긴 armature 이름에 의한 40-byte clip 이름 잘림을 피하려고 실제 cooker의 `ArmatureExportName=Seto`를 사용했다. Hook/Bomb도 짧은 prefix를 쓴다. 원본 PSA의 혼합 29.999998/30Hz를 30Hz로 bake한 뒤 원래 clip rate를 복원했다. source rate를 바꾼 상태로 배포하지 않았다.

## 물리 위치와 Drive 전달 준비

새로 설치한 파일은 다음 6개 폴더의 WModel과 참조 texture만이다. FBX, PSA, PSK, 원본 DB, cook 중간 산출물은 Resources에 복사하지 않았다.

- `Client/Bin/Resources/Character/KoukuSaton/MN_RHOC_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_RHOC_00-1`
- `Client/Bin/Resources/Character/KoukuSaton/MN_PPCT_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_UMAX_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_RHCN_01`
- `Client/Bin/Resources/Effect/KakulSaydon/WorldObjects/CuttingBlade`

6개 폴더 총 33파일, 51,146,900 bytes가 현재 PC의 runtime Resources에 설치되어 Drive 전달을 위해 폴더 단위로 복사 가능한 상태다. Drive 업로드는 실행하지 않았다. 공/빙고/커튼/룰렛은 기존 리소스 팩의 위 경로를 재사용한다. Resources는 Git 제외 입력이며 binary force-add를 하지 않았다.

## 확인한 결과와 남은 확인

- 변경 JSON parse 성공, stable ID 중복 없음, 기존 template 56/instance 60 semantic equality 확인.
- 새 animated object에서 사용한 7개 animation track의 clip이 각 WModel에 실제 존재한다.
- 8개 모델의 사용 material slot과 CMaterial 내부 texture 경로를 읽고 30개 참조 texture 및 공의 명시 diffuse override 파일 존재를 확인했다.
- 새 6개 WModel은 ModelAssetConverter info가 정상이며 animation 수는 카드14/조커14/세토68/갈고리9/폭탄5/칼날0이다.
- 5개 animated model의 원본 PSA rate 복원 receipt 검증과 실제 설치 WModel byte 일치 확인도 PASS다 (`out/WorldObjectWork/animation-rate-verification.json`).
- scoped `git diff --check`가 통과했다.
- `out/WorldObjectWork/seed-verification.json`, `resource-closure-verification.json`, `installed-resources.json`은 이번 실행의 로컬 증거다. 별도 제품 하네스나 Resource manifest 완료 조건을 추가하지 않았다.
- root 보고 기준 Map source v3 Validate/Publish, Gameplay Publish, 최종 Debug Product 빌드가 PASS다. 상세 빌드 receipt는 root 담당 결과 문서에 기록한다. Client/UI는 실행·조작·캡처하지 않았으며 화면 방향, 재질 표현, 연출 만족도는 사용자 확인 전이다.
- 칼날은 위 source join으로 Cutting projectile 본체임을 확인했다. 사용자가 말한 칼날 축제와 동일 연출인지에 대한 사용자 육안 확인은 아직 받지 않았다.
