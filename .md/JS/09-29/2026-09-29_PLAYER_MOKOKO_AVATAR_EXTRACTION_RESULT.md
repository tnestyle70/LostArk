# 2026-09-29 플레이어 5직업 모코코 아바타(AV_036) 모델 추출 RESULT

## 배경

- 상점 구매 → 인벤토리 → 착용 흐름의 아바타 후보로 모코코 아바타 패밀리를 준비한다. 원작 아바타는
  전직업 공용 모델이 아니라 **베이스 클래스별 별도 메시**이므로 5직업 각각의 모델을 추출해야 한다.
- 이번 범위는 추출·쿠킹까지다. `EquipmentPresentationCatalog.json` / `ItemCatalog.json` /
  `CharacterCatalog.json` 재질 행 연결과 상점·인벤토리 UI는 후속이다.
- 5직업은 창술사·워로드·도화가·차원술사·가디언나이트다. 기존 창술사 `mokoko_pleasant_*`(036-1) 자산은
  건드리지 않았다.

## 원본 좌표

| 직업 | `EFTable_PC` | 베이스 클래스(아바타 메시 접두) | 아이템 클래스 플래그 |
|---|---|---|---|
| 창술사 LanceMaster | 305 `PC_FLM` | 301 Fighter → `pc_ft_av_*` | `ForLanceMaster` |
| 워로드 Warlord | 104 `Gunlancer` `PC_WGL` | 101 Warrior → `pc_wr_av_*` | `ForWarlord` |
| 도화가 Artist | 602 `YinYangShi` `PC_SDM` | 601 Specialist → `pc_sp_av_*` | `ForYinyangshi` |
| 차원술사 DimensionMaster | 612 `PC_SWP_M` | 611 Specialist_Male → `pc_sp_m_av_*` | `ForDimensionMaster` |
| 가디언나이트 GuardianKnight | 702 `DragonKnight` `PC_DDK` | 701 DragonHuman → `pc_dk_av_*` | `ForDragonKnight` |

아이템은 `EFTable_Item.Model = EFDLItem_PC_<BASE>_AV_036<VAR>_<Head|Dress>` 15변형 × 2파츠 = 30행/클래스,
5직업 모두 `ClassCheckEnable=1`이고 해당 클래스 플래그가 1이다. 한글 이름은 `EFTable_GameMsg.db`
(`KEY = tip.name.item_<PK>`, cp949)에서 읽었다.

| VAR | 이름(머리/의상 공통) | 메시 | 비고 |
|---|---|---|---|
| `` | 강인한 모코코 | `av_036_head/dress` | 초록 기본 |
| `-1` | 기분 좋은 모코코 | `av_036-1_head/dress` | 창술사 기존 `mokoko_pleasant_*`와 같은 원본 |
| `-2` | 늘푸른 모코코 | `av_036-2_head1/dress1`(일반) / `av_036-2_head/dress`(vfx 재질) | `-2-1/-2-2/-2-3/-2-5` MIC 변형 |
| `-3` `-4` `-5` | 벚꽃 / 오로라 / 레인보우 모코코 | 별도 메시 없음 | `-2` 패키지의 MIC·dye 변형으로 추정, 메시는 `-2` 계열 |
| `A` `A-1` `A-2` | 여름밤 / 꿈꾸는 / 해질녘 모코코 | `av_036a_head/dress` | 은하 계열, 색 변형은 `pc_gn_av_036a-1/-2_*` MIC |
| `B` `B-1` `B-2` | 눈꽃 / 하늘 / 별빛 산타 모코코 | `av_036b_head/dress` | 색 변형은 `pc_dl_av_036b-1/-2_*` MIC |
| `C1` `C2` `C3` | 아프로 / 썬그리 / 스노쿨 모코코 | `av_036c1/c2/c3_head`, 의상은 `av_036c1_dress` 공용 | |

이미지 3~4행의 흰 곰 아바타는 모코코가 아니라 `AV_040` **곰탈 아바타**(시크한/댄디한/큐트한/나홀로 ×
화이트/브라운)이며 원작에 **Fighter(창술사)·Warrior(워로드) 아이템만 존재**한다. 도화가·차원술사·
가디언나이트 베이스에는 `AV_040` 아이템 행이 없다(메시도 `040a` 미참조 메시만 있음).

## 패키지·재질 구조

- 메시 패키지는 `PC_<BASE>_AV_036`(base·-1), `_AV_036-2`, `_AV_036A`, `_AV_036B`, `_AV_036C`. umodel 와일드카드
  `PC_FT_AV_036*`로 잡힌다. 가디언나이트의 base/-1/-2/A 패키지는 이름 해석이 안 돼 해시 파일
  `VC2NJW2NY12N0L6F08EHM7EH`, `VC2NJW2NY12N0L6YF88EHM7E`, `WD3OKX3OZ23O1M730FGUMPUFM`를 직접 지정했고,
  `-1_dress`, `-2_dress1`은 패키지 단위 export가 `TArray index out of range`로 중단되어 오브젝트 단위로 다시
  export했다(psk 청크 정상, 검증 통과).
- 머리 재질은 클래스 패키지에 없고 **Warrior/Gunner/Delain 그룹 MIC를 공유**한다
  (`pc_wr_av_036_head_mi`, `pc_gn_av_036a_head_mi`, `pc_dl_av_036b_head_mi`, `pc_wr_av_036c*_head_mi`).
  `PC_WR_AV_036*` export가 이를 함께 내보내므로 `.mat` 누락은 0이다.
- 머리 메시의 두 번째 슬롯은 클래스 기본 머리카락(`pc_ft_15_hair_helmet_mi` 등)이다. 모자 아래 머리카락
  submesh로 원본 자산에 포함된 것이며 그대로 쿠킹했다.
- `C3`(스노쿨) 머리는 `mn_isms_*`/`mn_petsf_00` 몬스터 재질을 빌려 쓴다. `-2`·`A`·`C1` 계열의 `_vfx` 슬롯은
  diffuse만 연결되며 원본 vfx 효과는 미복원이다.
- FT의 `A/B/C` 메시는 `b_upper_skirt_*_03/04`, `b_weapon_l/r` 본 8~10개를 더 갖지만 **가중치가 0**이라
  master 재바인딩에서 버려도 변형이 없다(`build_avatar_part.py`가 master 밖 가중치 합을 검사한다).

## 파이프라인

```text
umodel_lostark_v7 -export -psk -uncook -noanim  ->  _export_av036_psk (psk 94+ / tga 287+)
  -> build_avatar_part.py (Blender 5.0, 클래스 master 아마추어 재바인딩, 메시 1개 FBX)
  -> cook_av036.py: ModelAssetConverter --no-auto-textures + .mat Diffuse/Normal/Specular 명시 remap
     -> validate_wmodel.check -> 설치 본체 wmodel과 본 팔레트 index 대조
```

스크립트는 `C:\Users\95jus\Desktop\buildScript\`(Git 밖)의 `build_avatar_part.py`, `cook_av036.py`다.

| 직업 | master psk | armature 이름 | 설치 본체 팔레트 |
|---|---|---|---|
| LanceMaster | `PC_FLM_00/pc_flm_00_upper_sk_loc_int.psk` 221본 | `flm` | 224 (`RootNode, flm, …, mesh.001`) |
| Warlord | `PC_WR_00/pc_wr_00_sk.psk` 215본 | `wgl` | 218 (`RootNode, mesh.001, wgl, …`) |
| Artist | `PC_SP_00/pc_sp_00_sk.psk` 236본 | `sdm` | 239 (`RootNode, mesh.001, sdm, …`) |
| DimensionMaster | `PC_SP_M_00/pc_sp_m_00_sk.psk` 222본 | `pc_sp_m_00_sk` | 236 (224 + hair skelcontrol 11) |
| GuardianKnight | `PC_DDK_00/pc_ddk_00_upper_sk.psk` 282본 | `ddk` | 285 |

`CPart_Equipment`는 자기 본이 없는 스킨 파츠에 **본체 팔레트를 index 그대로** 바인딩하므로
(`Part_Equipment.cpp:282`), 파츠 wmodel의 본 순서가 본체와 같아야 한다. Blender FBX는 오브젝트 이름순으로
노드를 쓰기 때문에 armature 이름을 본체와 동일하게 줘야 mesh 노드 위치(1번 또는 마지막)가 맞는다.
`cook_av036.py`가 모든 파츠를 본체와 index별로 대조한다(mesh 노드 `.001` 슬롯만 예외).

## 출력

`Client/Bin/Resources/Character/<Class>/Equipment/mokoko_av036<VAR>_<head|outfit>[1]/<head|dress>.wmodel`
(+ `textures/*.tga`). `-2`의 `head1/dress1`은 폴더 뒤에 `1`이 붙는다. Resources는 Git 비추적이므로 Drive 전달
대상이다.

## 결과

| 항목 | 값 |
|---|---|
| AV_036 쿠킹 | 5직업 × 16파츠(머리 9·의상 7) = **80/80 성공**, 재질 슬롯 누락 경고 0 |
| AV_040 곰탈 | 창술사·워로드 × (head, dress) = 4/4 성공, `Equipment/bear_av040_{head,outfit}/` |
| 팔레트 대조 | 84파츠 모두 설치 본체와 index 일치(LM 224 / WL 218 / AT 239 / DM 236 대비 225 / GK 285) |
| 디코더 검증 | `validate_wmodel.check` 84/84 통과 |
| 용량 | 84폴더 1,079 MB(tga 655장, 폴더별 복제) |
| 리포트 | 스크래치 `av036_report.json`, `av040_report.json`, FBX `umodel_win32\_fbx_av036\<Class>\` |

폴더 목록(클래스마다 동일): `mokoko_av036_head/outfit`, `mokoko_av036-1_head/outfit`, `mokoko_av036-2_head/outfit`,
`mokoko_av036-2_head1/outfit1`, `mokoko_av036a_head/outfit`, `mokoko_av036b_head/outfit`, `mokoko_av036c1_head/outfit`,
`mokoko_av036c2_head`, `mokoko_av036c3_head`.

## 2차: 아이템 변형별 폴더 재배치와 원본 재질 행 (같은 날)

### 아이템 look 정본

`LPK/data4/EFGame_Extra/ClientData/XmlData/LookInfo/Item/EFDLItem_PC_<BASE>_AV_036<VAR>_<Head|Dress>.loa`가
아이템별 메시·MIC 참조를 그대로 갖는다(`out/MokokoAvatar20260929/look.json`, 150건). 추정으로 잡았던
매핑을 이 정본으로 교체했다. 확인된 사실:

- `-2` 늘푸른 = `head1/dress1` 메시 + `-2_*_mi`, `-3` 벚꽃 = 같은 메시 + `-2-1_*_mi`,
  `-4` 오로라 = `head/dress`(vfx) 메시 + `-2-2_vfx_*` + `-2-3_mi` + `-2-2_*`, `-5` 레인보우 = 같은 메시 + `-2-5_vfx_*` + `-2-3_mi` + `-2-3_head`/`-2-2_dress`.
- `A-1/A-2`, `B-1/B-2`는 `_036a-n_*` / `_036b-n_*` MIC 치환. `C2/C3` 의상은 `c1_dress` 메시에 `c2_/c3_dress_vfx`, `c2_/c3_dress2_mi`.
- 차원술사(SP_M)의 `-2` 계열은 `pc_gn_av_036-2*` MIC를 쓴다(umodel 와일드카드 export에는 빠져 있었고 패키지엔 있음).

### 폴더 재배치

`Client/Bin/Resources/Character/<Class>/Equipment/mokoko_av036<var>_<head|outfit>/` 로 **아이템 변형 기준** 30폴더/클래스,
5직업 150폴더. 메시를 공유하는 변형(-3, -5, a-1/a-2, b-1/b-2, c2/c3 outfit 등 70개)은 원본 폴더의 NTFS hard link라
디스크 추가 사용은 없다(Drive로 복사하면 실체 파일이 된다). 이전 `-2_head1/outfit1` 폴더는 `-2_*`로, vfx 메시 폴더는 `-4_*`로 이름을 바꿨다.

### 원본 재질 추출

`build_vehicle_source_material.py extract`로 MIC 147개 dump(`out/MokokoAvatar20260929/dumps`). 한 MIC당 약 80초라
7개 워커로 병렬화했다. 병렬 실패 원인 두 가지를 기록한다.
- Python이 쓴 목록 파일의 CRLF `
`이 target 끝에 붙어 `object is missing`으로 실패한다(로그엔 안 보임). `tr -d '
'` 필수.
- 워커마다 umodel `-list`를 부르지 않도록 `buildScript/extract_av036_par.py`가 `pkgmap.txt`(이름→upk)로 resolver를 대체한다.

셰이더 맵 그룹 18개: 기존 program 재사용 4(2, 11, 112, 1528), 신규 11(905~915, family
`source.character.mokoko-av036-<n>.v1`, `install`이 registry cohort 896~959에 자동 등록), 생성기 미지원 3
(`pbr_base_msk_vfx` flow/flicker 시간식 8개: B 의상 `dress_mi`, A 의상 `dress_vfx`, C 의상 `dress_vfx`).
`verify` 게이트 program 2 EXACT, 설치 후 907/912 EXACT, `test_source_character_program_groups.py` 7 OK.

### 재질 행

`CharacterCatalog.json` `modelMaterialOverrides`에 357행 추가(461 → 818, 삭제 0). 150모델 × 슬롯 중 65슬롯은 행 없이
쿠킹된 baked 텍스처로 남는다: 위 미지원 vfx 의상 슬롯과 C3 스노쿨의 `mn_isms_*` 몬스터 액세서리 슬롯(추출 실패).
텍스처는 `Character/SourceMaterials/<pkg>/<tex>.tga`로 44장 새로 복사(`Tools/VehiclePipeline/MokokoAvatar.texture-map.json`, 144항목).
행 생성 스크립트는 `buildScript/{group,match,plan,rows}_av036*.py`.

### 미완료

- **빌드**: Product Debug PASS(Engine 85.7s, Client 134s, Client OBJ 1 / CSO 3, 오류 0). 신규 program 11개가 `Engine/Bin/ShaderFiles/*Group896.hlsli`와 `SourceCharacterMaterialParameters_Generated.inl`을
  바꿨으므로 Product Debug 빌드가 필요하다. 화면 확인도 없다.
- EquipmentPresentationCatalog visualSet / ItemCatalog 아이템은 아직 없다(행은 `g_EquipmentModelMaterials`에 asset ID로 실려 visualSet 등록 즉시 적용된다).

## 3차: 테스트 경로 — F1 Equipment Authoring Tool

- 캐릭터 선택 화면 → F1 → `Equipment Authoring Tool` → 클래스·슬롯(HEAD/UPPER) 선택 → visualSet 선택 → Apply.
  `CEquipmentAuthoringTool`이 `EquipmentPresentationCatalog.json`의 visualSet만 나열하므로 모코코 150개
  (`character.<class>.mokoko_av036<var>.head|outfit`)를 등록했다(238 → 388). Client 상한 256 → 512
  (`EquipmentPresentationCatalog.cpp`).
- **도구 크래시 수정**: `EquipmentAuthoringTool.h`의 `CLASS_COUNT`가 6인데 `CLASS_OPTIONS`는 가디언나이트 포함 7개라,
  `EquipmentLoadoutPresets.json`의 GUARDIANKNIGHT 행을 읽는 `Reload_Presets`가 `m_Loadouts[6]`을 만져 Debug
  `std::array` 범위 assert로 열자마자 종료됐다. 7로 올리고 `static_assert(CLASS_OPTIONS.size() == CLASS_COUNT)`를 추가했다.
- `test_equipment_catalogs.py` 4 OK. `test_equipment_authoring_tool_contract.py`는 13 중 1 실패
  (`Logic_LanceMaster.cpp` 파츠 수 8 vs 6 — 이 브랜치의 기존 불일치, 이번 변경과 무관).
- **탕후루 광택 수정(09-30)**: 사용자가 F1 도구로 입혀 본 결과 사탕처럼 번들거렸다. 원인은 팀장 09-27 결과와 같다 —
  반사 lookup `hdr07_1`/`brdf_beckmann_spec`를 mip 1단계 `.dds`로 참조하면 roughness LOD `SampleLevel`이 항상 mip0을
  샘플해 거울 반사가 된다. 모코코 행 357개의 공유 prologue 텍스처 4종(hdr07_1, brdf_beckmann_spec, flat_black,
  statefx_default) 참조를 mip 10단계가 생성되는 `.tga`로 바꿨다(1,116 필드). `rows_av036.py`도 `.tga` 우선으로 고쳤다.
  **사용자 확인(09-30 01:15)**: F1 Equipment Authoring Tool로 입혀 본 결과 광택이 정상으로 돌아왔다. 변형별 색 판정은 계속 사용자 확인 범위.

## 미완료 / 다음 단계 (1차 시점)

- `EquipmentPresentationCatalog.json` visualSet 행, `ItemCatalog.json` 아바타 아이템, `CharacterCatalog.json`
  `modelMaterialOverrides`(원본 SourceCharacter program·dye 틴트) 연결.
- `-3/-4/-5`, `A-1/A-2`, `B-1/B-2` 색 변형은 메시 없이 MIC/dye만 다르다. 재질 행으로 표현하려면 해당 MIC의
  dye 틴트와 `_cm` 마스크를 `modelMaterialOverrides`로 실어야 한다.
- 곰탈(AV_040)은 창술사·워로드 원본만 있다. 5직업 공통 후보로는 부적합하다.
- 사용자 화면 확인은 아직 없다. 상점·착용 UI 연결 뒤 실제 착용 화면으로 판정한다.
