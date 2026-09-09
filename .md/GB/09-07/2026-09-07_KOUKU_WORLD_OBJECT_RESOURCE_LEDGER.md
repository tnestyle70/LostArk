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
- 단위/배수: modelPreScale=0.01, scale=2.
- 애니메이션: 14; `mn_rhoc_00_sk.ao_idle_normal_1`.
- 텍스처: `textures/mn_rhoc_00_{d,n,s}.dds`

### 월드오브젝트_조커카드

- 저장 ID: `world.object.kouku.joker_card`
- 모델/alias: `Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel`
- 근거: Npc 480643 `뒤집혀진 조커 카드`; LookInfo `EFDLChar_MN_RHOC_00-1.MN_RHOC_00-1` → 같은 카드 mesh/AnimSet + MN_RHOC_00-1_MI. 재질 variant를 실제 별도 WModel로 cook했다.
- 단위/배수: modelPreScale=0.01, scale=2.
- 애니메이션: 14; `mn_rhoc_00_sk.ao_idle_normal_1`.
- 텍스처: `textures/mn_rhoc_00-1_{d,n,s}.dds`

### 월드오브젝트_공

- 저장 ID: `world.object.kouku.ball`
- 모델/alias: `Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel`
- 근거: 사용자 지정 MN_RHCN_00 텍스처에 맞춰 revision 367에서 기존 세토 계열 서커스 공 연결을 교정했다. `FX_MN_RPCZ_00_U.par_u_rpcz_ballshoot_ball_01/02`의 TypeDataMesh와 mn_rhcn_00_mi 참조가 이 모델이다. 원본 MN_RHCN_00_SK와 위치 보정 후 전체 716 vertex shape가 일치한다. 이름의 fm_d만으로 파편이라고 분류하지 않는다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 0; transform/motion.
- 텍스처: WModel material0 `mn_rhcn_00_mi`에 원본 MIC의 `Effect/KoukuSaydon/Textures/MN_RHCN_00/tex/mn_rhcn_00_{d,n,s,e}.dds` 네 입력을 연결했다. 기존 diffuse override도 같은 D를 사용한다. 실제 CModel의 단일 mesh/material0에서 D/N/S/E 네 texture 로드를 확인했다. mesh section과 dummy material, Transform은 보존했다. 원본 PBR·emissive 시간 공식 전체 복원을 뜻하지 않는다.
- 원본 상하 이동은 위 particle의 local 위치 곡선 21개 표본에서 -20→150→-20cm로 확인했다. 모델 자체 clip은 0개다. 현재 revision412에서 `KAKULSAYDON_G1_PATTERN_8 → world.object.instance.kouku.ball_bounce → world.object.kouku.ball` 소비를 확인했고, 사용자 저작 Motion과 occurrence 타이밍은 바꾸지 않았다. 근거 `out/KoukuPatternStaging/ball-mn-rhcn-00-source-audit.json`, `out/ThreeClassFullRestore20260909/kouku-ball-textures.json`.

### 월드오브젝트_세토

- 저장 ID: `world.object.kouku.seto`
- 모델/alias: `Character/KoukuSaton/MN_PPCT_00/MN_PPCT_00.wmodel`
- 근거: Npc 480708/480709 `카드미로 세토 로밍형/돌진형`; LookInfo MN_PPCT_00 → MN_PPCT_00_SK_LOC_INT / MN_PPCT_00_Ani. 4 material slot의 실제 diffuse/normal/specular와 두 emissive를 연결했다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 68; `Seto_idle_normal_1`.
- 텍스처: `textures/mn_ppct_00_1_{c,n,s}_loc_int.dds`, `mn_ppct_00_1_em.dds`, `mn_ppct_00_2_{c,n,s,em}.dds`, `mn_ppct_00_{c,n,s}.dds`

### 월드오브젝트_칼날

- 저장 ID: `world.object.kouku.cutting_blade`
- 모델/alias: `Effect/KoukuSaydon/WorldObjects/CuttingBlade/CuttingBlade.wmodel`
- 근거: MN_ISTM_00-4 Action 4222009 `투명NPC_칼날 발사` → notify SkillEffect 421991302 → Key 12 / ValueA 421991301 → data1 Projectile 421991301.loa → `FX_MN_RPCT_07_V.Par_V_RPCT_Cutting_pjt_01` → TypeDataMesh `fx_sm_01.fm_o_cngn_01`, Required material `mn_cngn_00.mat.mn_cngn_00_mi`. 두 swing mesh는 trail이므로 본체로 등록하지 않았다.
- 단위/배수: modelPreScale=0.01, scale=1.
- 애니메이션: 0; transform/motion.
- 텍스처: `textures/mn_cngn_00_{d,n,s,e}.dds`

### 월드오브젝트_갈고리

- 저장 ID: `world.object.kouku.hook`
- 모델/alias: `Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel`
- 근거: Npc 480710 `갈고리`; LookInfo MN_UMAX_00 → MN_UMAX_00_SK / MN_UMAX_00_MI / MN_UMAX_00_Ani.
- 단위/배수: modelPreScale=0.01, scale=1.15.
- 애니메이션: 9; `Hook_idle_normal_1`.
- 텍스처: `textures/mn_umax_00_d_loc_int.dds`, `mn_umax_00_{n,s}.dds`

### 월드오브젝트_빙고폭탄

- 저장 ID: `world.object.kouku.bingo_bomb`
- 모델/alias: `Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel`
- 근거: Npc 480724 `해골이 그려져있는 폭탄_빙고` → EFDLChar_MN_RHCN_01.MN_RHCN_01; 실제 MN_RHCN_01_SK / MN_RHCN_01_MI / MN_RHCN_01_Ani.
- 단위/배수: modelPreScale=0.01, scale=2.
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

UModel CLI의 `-export`, 저장소 `Cook-ActorXWModel.ps1`, `ModelAssetConverter.exe`, `retime_wmodel_from_psa.py`를 사용했다. 카드/조커/세토/갈고리/폭탄은 skeleton+clip을 유지하고, 발사 칼날은 원본 glTF 정적 geometry를 CModel WModel로 cook했다. Effect Tool의 particle 런타임을 두 번째 모델 경로로 추가하지 않는다. skinned cook의 raw vertex는 m 단위지만 실제 CMesh skin 행렬에는 skeleton의 scale100이 남아 있다. 09-07 재검증에서 bind/clip 행렬까지 적용한 bounds로 확인하여 이 5종의 modelPreScale을 .01로 교정했다. 기존 공/빙고 및 scale100 정적 칼날도 .01을 유지한다. 카드/조커2, 갈고리1.15, 빙고폭탄2의 저작 배수는 보존했다.

세토는 긴 armature 이름에 의한 40-byte clip 이름 잘림을 피하려고 실제 cooker의 `ArmatureExportName=Seto`를 사용했다. Hook/Bomb도 짧은 prefix를 쓴다. 원본 PSA의 혼합 29.999998/30Hz를 30Hz로 bake한 뒤 원래 clip rate를 복원했다. source rate를 바꾼 상태로 배포하지 않았다.

## 물리 위치와 Drive 전달 준비

새로 설치한 파일은 다음 6개 폴더의 WModel과 참조 texture만이다. FBX, PSA, PSK, 원본 DB, cook 중간 산출물은 Resources에 복사하지 않았다.

- `Client/Bin/Resources/Character/KoukuSaton/MN_RHOC_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_RHOC_00-1`
- `Client/Bin/Resources/Character/KoukuSaton/MN_PPCT_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_UMAX_00`
- `Client/Bin/Resources/Character/KoukuSaton/MN_RHCN_01`
- `Client/Bin/Resources/Effect/KoukuSaydon/WorldObjects/CuttingBlade`

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

## 09-07 native 모션 전수 조사와 상태 연결

설치된 10종의 모델 및 배치 sequence를 다시 읽었다. 새로운 추출이나 Resources 수정은 하지 않았다.
카드/조커 14개씩, 세토 68개, 갈고리 9개, 빙고폭탄 5개로 총 110개 clip이 실제 WModel에 들어 있다.
원본 110개는 선택 모델의 Animation Resources에서 읽는 재료 목록이다. Create Object → 원본 모델·클립 선택 → Append → Save로
사용할 모션만 저장한다. 기존 5개 idle 상태를 재사용하고 카드 합성 상태 2개를 원본으로 교체했으며,
조커 카드 2개·세토 3개·갈고리 4개·폭탄 2개, 총 11개 대표 상태를 추가했다. 저장된 native 패턴은
18개이며 모델 resource는 기존 10개다. 원본 클립마다 모델이나 저장 패턴을 자동 복제하지 않는다.

| 요청 또는 대표 상태 | 원본 clip | 저장 길이 | 이동 방식 |
|---|---|---:|---|
| 월드오브젝트_빈카드들썩임 | mn_rhoc_00_sk.ao_att_battle_2_01 | 500ms | body bone |
| 월드오브젝트_빈카드뒤집힘 | mn_rhoc_00_sk.ao_att_battle_1_start | 667ms | body bone |
| 월드오브젝트_조커카드들썩임 | mn_rhoc_00_sk.ao_att_battle_2_01 | 500ms | body bone |
| 월드_오브젝트_조커카드뒤집힘 | mn_rhoc_00_sk.ao_att_battle_1_start | 667ms | body bone |
| 월드오브젝트_세토걷기_제자리 | Seto_walk_normal_1 | 1334ms | 실제 경로는 Motion/Transform |
| 월드오브젝트_세토달리기_제자리 | Seto_run_battle_1 | 600ms | 실제 경로는 Motion/Transform |
| 월드오브젝트_세토돌진 | Seto_att_battle_21_02 | 3000ms | root bone +X 23.5853m |
| 월드오브젝트_갈고리전방이동 | Hook_att_battle_1_01 | 7000ms | root bone +X 21.25m |
| 월드오브젝트_빙고폭탄낙하 | Bomb_respawn_1 | 2000ms | body bone -Y 23.4997m |
| 월드오브젝트_빙고폭탄흔들림 | Bomb_att_battle_1_01 | 1000ms | body bone; 폭발 Effect는 별도 |

카드 네 상태는 MN_RHOC_00.loa의 빈카드/조커카드 들썩이기·뒤집어지기 Action을 직접 대조했다.
뒤집힘은 Start의 666.6667ms를 667ms로 올림하고 마지막 pose를 유지한다. 뒤따르는 Loop/End는
뒤집힌 pose가 정지한 clip이므로 이 상태에 반복해서 붙이지 않았다. 기존 card_hop/card_flip ID와
모든 배치는 유지하고 합성 위치/회전 키는 identity로 교체해 bone 움직임이 중복되지 않게 했다.
기존 objectScale 2는 보존되므로 카드 움직임 크기에도 그 배수가 적용된다.

세토 Npc 480708/480709는 카드미로 로밍형/돌진형이며, 돌진형 Action 4193890이 Att_Battle_21_02를
참조한다. Walk/Run은 root 이동이 0인 제자리 모션이다. 사용자 지정 이동 경로는 별도로 저작해야 한다.
돌진·갈고리 전방 이동은 native root가 움직이므로 기본 Velocity를 더하지 않는다. 이 bone 이동은
World Transform에 누적되지 않으므로 개별 이동 clip은 one-shot으로 저장한다. 원본 분기와
MoveNext 타이밍을 무시한 여러 clip의 단순 연결을 원작 돌진 패턴으로 저장하지 않았다.

공·칼날·빙고 타일에는 native animation이 없다. 공은 기존 공_튀기기의 Velocity [0,6,0],
Acceleration [0,-6,0], 2000ms 상태를 유지하며, 수평 방향과 거리는 해당 Motion에서 조절한다.
커튼 44개·룰렛 183개의 기존 Transform key도 보존했다. 폭탄은 원본에서도 Effect와 HidePawn이
별도 동작이므로 bone clip만으로 폭발 이펙트까지 연결됐다고 보지 않는다.

저장한 패턴의 clip 대응은 authoring worldsequences의 animationTracks에 있다. 나머지 원본 클립은
기존 WModel decoder가 설치된 모델에서 직접 읽으며 별도 catalog JSON에 중복 저장하지 않는다.
로컬 수치 근거는 out/WorldObjectMotionAudit의 selected-pattern-save-verification.json, all-models-inventory.json,
card-native-animation-audit.json,
props-complete-inventory.json, props-native-motion.json이다. 실제 화면의 방향·크기·만족도는 사용자 확인 전이다.
