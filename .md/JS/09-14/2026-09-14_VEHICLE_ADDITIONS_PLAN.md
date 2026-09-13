# 2026-09-14 탈것 3종 추가 PLAN (은색 전투 랩터 · 고요한 별빛의 가호 · 레인보우 모코보드)

작성자: JS · 브랜치 `feature/terpeion-vehicle-riding`(로컬 커밋 `40ee3e29`) 위에서 이어서 작업한다.
선행 작업: [황금 테르페이온 PLAN](../09-13/2026-09-13_TERPEION_VEHICLE_RIDING_PLAN.md) / [RESULT](../09-13/2026-09-13_TERPEION_VEHICLE_RIDING_RESULT.md).

## G00. 목표와 실측 근거

테르페이온 탑승 수직 슬라이스를 그대로 확장해 탈것 3종을 추가한다. Server·protocol·H 키 경로는 catalog 기반이라
코드 변경이 없다. 새로 필요한 것은 원작 재질 program 3개, 반투명 forward 패스 일반화, 탈것 모델 3개,
직업별 탑승 자세 애니셋 12개, F1 기본 탈것 선택이다.

### 원본에서 확정한 사실

| 항목 | 은색 전투 랩터 | 고요한 별빛의 가호 | 레인보우 모코보드 |
|---|---|---|---|
| EFTable_Vehicle PK | 7104 | 9370 | 7209 |
| Model(LookInfo) | `EFDLVehi_MN_ISRX_02-4` | `EFDLVehi_MN_PMSSM_00` | `EFDLVehi_MN_PMSMK_00-4` |
| MoveSpeed | 500 (5 m/s) | 500 | 500 |
| RidingMode | 3 `RAPTOR` | 46 `SWING` | 5 `HOVERBOARD` |
| 몸체 SkeletalMesh | `MN_ISRX_02.Mesh.MN_ISRX_02_SK`(60 bone, UV 2세트) | `MN_PMSSM_00.Mesh.MN_PMSSM_00_SK`(19 bone, UV 2세트) | `MN_PMSMK_00.Mesh.MN_PMSMK_00_SK`(9 bone, UV 1세트) |
| AnimSet | `MN_ISRX_01.Ani.MN_ISRX_01_Ani`(본 60개 순서 동일) | `MN_PMSSM_00.Ani.MN_PMSSM_00_Ani` | `MN_PMSHB_00.Ani.MN_PMSHB_00_Ani`(본 9개, 구성 같고 순서 다름) |
| LookInfo MIC 교체 | `MN_ISRX_00-4_MI`, `MN_ISRX_02-4_MI` | `MN_PMSSM_00_A_MI`, `MN_PMSSM_00_B_MI` | `MN_PMSMK_00-4_VFX_MI` |
| 좌석 | 소켓 `Cockpit` = `b_cockpit`, offset 0 | `b_cockpit` | `b_cockpit` |
| 탈것 idle / run | `idle_normal_1` 81f / `run_normal_1` 33f | 101f / 101f | 71f@30 / 51f |
| 탑승자 idle / run | `ride_raptor_*` 81f / 33f (`pc_*_ani`) | `ride_swing_*` 101f / 101f (`pc_*_vehicle_ani`) | `ride_hoverboard_*` 68f@29 / 51f (`pc_*_ani`) |

- 별빛의 가호 8색은 이름에 색이 없다. diffuse 평균 색상 211°·채도 0.57인 9370을 사용자가 `고요한 별빛의 가호`(파란색)로 확인했다.
- 모드 번호→이름은 클립 존재로 교차 확인했다. `pc_ft/wr/sp/sp_m_00_ani` 네 가족에 `ride_raptor` 8개·`ride_hoverboard` 24개가 있고
  스윙은 `pc_*_vehicle_ani` 네 가족에 `ride_swing` 10개가 탈것 클립 10개와 이름·프레임까지 1:1이다.
- 모코보드 idle만 탑승자 68f@29fps(2.345s)와 보드 71f@30fps(2.367s)가 22ms 다르다. 원본 값 그대로 쓰며 루프마다 벌어지는 차이는 사용자 확인 항목이다.
- LookInfo의 파티클(`Par_D_Dust_010_pr`, `Par_L_PMSSM_Default_01`, `Par_G_PMSML_00_Default_Rainbow` 등)과 스폰 이펙트는 이번 범위가 아니다.

### shader map 조사 결과

`build_vehicle_source_material.py extract`로 retail RefShaderCache에서 GPU-skin BasePass / directional light PS를 뽑았다.

| MIC | 부모 · BlendMode | Base PS / Light PS | 결론 |
|---|---|---|---|
| `mn_isrx_02-4_mi` 랩터 몸통 | `monster_base_msk_high` · masked | `b23ac74d…` / `70c5c49c…` | 기존 **program 23** (`monster-d9d6c02905c3.v1`) |
| `mn_isrx_00-4_mi` 랩터 장갑 | `monster_base_msk_high` · masked | `ab0d30ff…` / `9aee034d…` | 기존 **program 25** (`monster-be5bc0ded311.v1`) |
| `mn_pmssm_00_b_mi` 별빛 몸체 | `realpbr_base_msk` · masked | `4c053d1a…` / `eea5aa67…` | 신규 **program 86**, base가 환경 cube(t7) 사용 |
| `mn_pmsmk_00-4_vfx_mi` 모코보드 | `pbr_base_msk_vfx` · masked | `1f0ef7bb…` / `e2192898…` | 신규 **program 87** |
| `mn_pmssm_00_a_mi` 별빛 외피 | `pbr_base_trn_reflection` · **translucent**, one-sided | `e0cd09e5…` / `93add9b0…` | 신규 **program 88**, BLEND forward |

- 비교 기준은 `Shader_SourceCharacterBasePrograms.hlsli`의 `// <family> / source program <base PS id>` 머리주석이다.
- 랩터 MIC는 처음에 `ShaderCache FName index is invalid`로 실패했다. static parameter set의 normal 파라미터가 있는 첫 사례로,
  UE3 `FNormalParameter`(FName 8 + CompressionSettings 1 + bOverride 4 + GUID 16 = 29B)를 공용 oracle이 32B로 읽었다. G01에서 도구 안에서만 보정한다.
- program 86 생성은 `fmaterialuniformexpressionclamp` 미지원으로 packing 블록이 실패하고, cube 샘플은 0으로 생성된다. G01에서 생성기를 보강한다.
- program 86 base의 `t6/s7` 샘플은 uniform texture 목록 밖의 엔진 텍스처다. 설치된 program(1805행 등)도 같은 자리를 0으로 두므로 그대로 따른다.

### 설계 경계

- Server, Shared protocol, VehicleCatalog 파서, H 키 요청 경로는 바꾸지 않는다. 데이터 행만 늘어난다.
- 반투명 forward 패스는 탈것 파츠에서만 쓴다(테르페이온 결정 유지). program 18(양면 헤어)과 88(단면 외피)을 program별 pass로 고른다.
- 탈것 선택은 사용자 결정대로 **F1 Developer Tools에서 기본 탈것 지정**이다. H는 그 탈것으로 탑승하고, 현재 직업의 탑승자 클립이
  없으면 catalog 첫 탈것으로 넘어간다. 제품 선택 UI는 후속이다.
- 탑승자 애니셋은 모드별 파일로 나눈다(`<Class>_RideRaptorAnimSet`, `_RideSwingAnimSet`, `_RideHoverboardAnimSet`). 기존 `RideHorse`와 같은 형식이다.

### 파일 목록

| G | 내용 | 파일 |
|---|---|---|
| G01 | 추출·생성 도구 보강 | `Tools/VehiclePipeline/build_vehicle_source_material.py` |
| G02 | program 86·87·88 설치와 원본 텍스처 | Engine/Client `Shader_SourceCharacter{Base,Light}Programs.hlsli`, `SourceCharacterMaterialParameters.h`, `Engine/Private/Model.cpp`, `Tools/VehiclePipeline/{SilverBattleRaptor,SereneStarlightBlessing,RainbowMokoboard}.texture-map.json`(신규) |
| G03 | 탈것 모델 3개 쿠킹 | Resources `Character/Vehicle/<Name>/<Name>.wmodel` |
| G04 | 탑승자 애니셋 12개 | Resources `Character/<Class>/AnimSets/<Class>_Ride{Raptor,Swing,Hoverboard}AnimSet.wmodel`, `Data/Actors/CharacterCatalog.json` |
| G05 | 데이터 | `Data/Vehicles/VehicleProfiles.json`, `Data/Actors/VehicleCatalog.json` |
| G06 | 반투명 forward 일반화 | `Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl`, `Client/Private/Part_Vehicle.cpp` |
| G07 | F1 기본 탈것 선택 | `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp`, `Client/Private/MainApp.cpp` |
| G08 | 문서·검증 | RESULT, `CLAUDE.md`, `.md/GB/렌더링이펙트복원V2.md`, `.md/GB/gotchas.md` |

---

## G01. 추출·생성 도구 보강

### `Tools/VehiclePipeline/build_vehicle_source_material.py` — normal 파라미터 29B

기준점: `open_cache`의 `oracle.read_fname = numbered_read_fname` 바로 아래. (조사 중 작업 트리에 반영됨)

```python
    parse_static_parameter_set = oracle.parse_static_parameter_set

    def normal_29_byte_static_parameter_set(data, offset, names):
        # UE3 FNormalParameter is FName + BYTE CompressionSettings + UBOOL bOverride + GUID.
        cursor = offset + 16
        for entry_size in (32, 44):
            if cursor + 4 > len(data):
                return parse_static_parameter_set(data, offset, names)
            count = struct.unpack_from('<I', data, cursor)[0]
            cursor += 4 + min(count, 4097) * entry_size
        if cursor + 4 > len(data):
            return parse_static_parameter_set(data, offset, names)
        normals = struct.unpack_from('<I', data, cursor)[0]
        rows_at = cursor + 4
        if normals == 0 or normals > 4096 or rows_at + normals * 29 > len(data):
            return parse_static_parameter_set(data, offset, names)
        widened = bytearray(data[:rows_at])
        for index in range(normals):
            entry = data[rows_at + index * 29:rows_at + (index + 1) * 29]
            widened += entry[:8] + struct.pack('<II', entry[8], struct.unpack_from('<I', entry, 9)[0]) + entry[13:29]
        widened += data[rows_at + normals * 29:]
        decoded = parse_static_parameter_set(bytes(widened), offset, names)
        shrink = normals * 3
        decoded['byteSize'] -= shrink
        decoded['endOffset'] -= shrink
        return decoded
    oracle.parse_static_parameter_set = normal_29_byte_static_parameter_set
    sm.parse_static_parameter_set = normal_29_byte_static_parameter_set
```

공용 `Tools/LevelPlacementExtractor/extract_artist_31470_shader_cache_oracle.py`는 바꾸지 않는다. normal 파라미터가 0개인
기존 MIC는 원래 파서를 그대로 탄다. 이 보정으로 랩터 두 MIC 추출이 성공했다(위 표 PS id).

### `hlsl()` / `cpp()` — clamp uniform expression

기준점: `hlsl()`의 `if kind == 'fmaterialuniformexpressionappendvector':` 블록 바로 아래 추가.

```python
    if kind == 'fmaterialuniformexpressionclamp':
        return ('clamp(' + hlsl(node['input'], time_parameter) + ',' + hlsl(node['minimum'], time_parameter) +
                ',' + hlsl(node['maximum'], time_parameter) + ')')
```

기준점: `cpp()`의 `if kind == 'fmaterialuniformexpressionappendvector':` 블록 바로 아래 추가.
Client packing에는 이미 `bounded(a, lo, hi)`(성분별 min/max)가 있다.

```python
    if kind == 'fmaterialuniformexpressionclamp':
        return 'bounded(' + cpp(node['input']) + ',' + cpp(node['minimum']) + ',' + cpp(node['maximum']) + ')'
```

### `translate()` — 환경 cube 샘플

기준점: `elif 'texturecube' in op:` 와 그 다음 줄 교체. 설치된 realpbr program의 표기
(`g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, …) : 0.f`)와 같은 식이다.
환경 cube 변수는 base pass에만 선언되므로 light stage는 0을 유지한다.

```python
        elif 'texturecube' in op and stage == 'base' and 'sample_l' in op:
            sample = ('(g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel('
                      f'SourceCharacterLookupSampler, ({operand(uv)}).xyz, ({operand(a[4])}).x) : float4(0.0,0.0,0.0,0.0))')
        elif 'texturecube' in op:
            sample = 'float4(0.0,0.0,0.0,0.0)'
```

### 게이트

```powershell
$py = 'C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe'
$tool = 'Tools/VehiclePipeline/build_vehicle_source_material.py'
& $py $tool verify --dump out/VehicleTerpeion20260913/dumps/mn_ppch_00.mat.mn_ppch_00-1_mi.json `
  --family source.character.monster-d621a47e69ad.v1 --program 24
& $py $tool verify --dump out/VehicleTerpeion20260913/dumps/mn_pmstg_01.mat.mn_pmstg_01_mi.json `
  --family source.vehicle.terpeion-body.v1 --program 85
```

두 program 모두 base/light/configure EXACT가 유지돼야 다음 G로 간다(clamp·cube 분기를 타지 않는 기존 생성물 무변화 증명).

---

## G02. program 86·87·88 설치와 원본 텍스처

### 절차

```powershell
$work = 'out/VehicleAdditions20260913'
& $py $tool generate --dump "$work/dumps/mn_pmssm_00.mat.mn_pmssm_00_b_mi.json" `
  --family source.vehicle.starlight-body.v1 --program 86 --out "$work/generated/86"
& $py $tool generate --dump "$work/dumps/mn_pmsmk_00.mat.mn_pmsmk_00-4_vfx_mi.json" `
  --family source.vehicle.mokoboard-vfx.v1 --program 87 --out "$work/generated/87"
& $py $tool generate --dump "$work/dumps/mn_pmssm_00.mat.mn_pmssm_00_a_mi.json" `
  --family source.vehicle.starlight-shell-translucent.v1 --program 88 --out "$work/generated/88"
foreach ($p in @(@('86','source.vehicle.starlight-body.v1'), @('87','source.vehicle.mokoboard-vfx.v1'),
                 @('88','source.vehicle.starlight-shell-translucent.v1'))) {
  & $py $tool install --generated "$work/generated/$($p[0])" --family $p[1] --program $p[0]
}
```

- `install`은 dispatch 기준점을 `case {N-1}u:`로 바꿔 86 → 87 → 88 순서로 넣는다(구현 시 도구 수정, program 85 설치 기준과 동일 결과).
  각 설치 후 `verify … --program <N>`으로 EXACT를 확인한다. 기준점이 없어 실패하면 기존 dispatch 줄을 되돌리지 않고 멈춘다.
- 생성 결과 줄 수(조사 시 생성본): base86 828 / light86 456, base87 706 / light87 701, base88 495 / light88 393.
  G01 보강 후 86은 clamp packing과 cube 샘플이 들어가 줄 수가 달라질 수 있으며 SHA-256은 RESULT에 고정한다.

### `Engine/Private/Model.cpp` — program 상한

기준점: `Apply_MaterialOverrides` 안 `if (source.program == 0u || source.program > 85u || (source.program > 65u && source.program < 80u) ||` 교체.

```cpp
            if (source.program == 0u || source.program > 88u || (source.program > 65u && source.program < 80u) ||
```

86·87·88은 `Shader_SourceCharacterMaterial.hlsli`의 특수 입력 목록(5/6/7/12/18/19/20/22/29/84)과 UV1 요구 목록(5/7/18/19)에 없다.
세 모델 모두 `requiredExtraUVMask`를 쓰지 않는다.

### 텍스처 맵 (신규 3개)

`Tools/VehiclePipeline/SilverBattleRaptor.texture-map.json`

```json
{
  "mn_isrx_00_n": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_00_n.tga",
  "mn_isrx_00_cm": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_00_cm.tga",
  "mn_isrx_00_d": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_00_d.tga",
  "mn_isrx_00_s": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_00_s.tga",
  "mn_isrx_00_m": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_00_m.tga",
  "mn_isrx_02_n": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_n.tga",
  "mn_isrx_02_s_cm": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_s_cm.tga",
  "mn_isrx_02_d": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_d.tga",
  "mn_isrx_02_s": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_s.tga",
  "mn_isrx_02_em": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_em.tga",
  "mn_isrx_02_m": "Character/Vehicle/SilverBattleRaptor/SourceMaterials/mn_isrx_02_m.tga",
  "hdr07_1": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga",
  "statefx_default": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga",
  "brdf_beckmann_spec": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds"
}
```

`Tools/VehiclePipeline/SereneStarlightBlessing.texture-map.json`

```json
{
  "mn_pmssm_00_n": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmssm_00_n.tga",
  "mn_pmssm_00_d": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmssm_00_d.tga",
  "mn_pmssm_00_orm": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmssm_00_orm.tga",
  "mn_pmssm_00_cm": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmssm_00_cm.tga",
  "mn_pmssm_00_e": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmssm_00_e.tga",
  "ta_atypical_2": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/ta_atypical_2.tga",
  "mn_pmsca_00_re": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/mn_pmsca_00_re.tga",
  "pc_mar_av_137_upperc_ef": "Character/Vehicle/SereneStarlightBlessing/SourceMaterials/pc_mar_av_137_upperc_ef.tga",
  "flat_black": "Character/SourceMaterials/efmaster_material_prologue/flat_black.tga",
  "statefx_default": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga"
}
```

`Tools/VehiclePipeline/RainbowMokoboard.texture-map.json`

```json
{
  "mn_pmsmk_00_n": "Character/Vehicle/RainbowMokoboard/SourceMaterials/mn_pmsmk_00_n.tga",
  "mn_pmsmk_00_cm": "Character/Vehicle/RainbowMokoboard/SourceMaterials/mn_pmsmk_00_cm.tga",
  "mn_pmsmk_00-4_d": "Character/Vehicle/RainbowMokoboard/SourceMaterials/mn_pmsmk_00-4_d.tga",
  "mn_pmsmk_00-3_vfx": "Character/Vehicle/RainbowMokoboard/SourceMaterials/mn_pmsmk_00-3_vfx.tga",
  "mn_pmsmk_00_s": "Character/Vehicle/RainbowMokoboard/SourceMaterials/mn_pmsmk_00_s.tga",
  "hdr07_1": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga",
  "statefx_default": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga",
  "brdf_beckmann_spec": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds"
}
```

`hdr07_1`·`statefx_default`·`brdf_beckmann_spec`·`flat_black`은 Resources에 이미 있다. 나머지는 staging에서 복사한다.

```powershell
& 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe' -export -uncook -game=lostark -kr -nameresolve `
  -path='C:\ProgramData\Smilegate\Games\LOSTARK\EFGame' -out="$work/staging" `
  MN_ISRX_00 MN_ISRX_02 MN_PMSSM_00 MN_PMSCA_00 PC_MAR_AV_137 EFMASTER_MATERIAL_PROLOGUE MN_PMSMK_00
foreach ($m in 'SilverBattleRaptor','SereneStarlightBlessing','RainbowMokoboard') {
  & $py $tool textures --texture-map "Tools/VehiclePipeline/$m.texture-map.json" --staging "$work/staging"
}
```

`textures`는 map에 적힌 목적지가 이미 있으면 덮어쓰지 않고 크기·해시만 비교한다(기존 동작). 32bit가 아닌 RLE TGA도 현재 loader가 읽는다.

---

## G03. 탈것 모델 3개 쿠킹

테르페이온과 같은 NPC 파이프라인(`build_npc.py` → `cook_npc.py`)으로 쿠킹한다. LookInfo JSON은 조사 때 만든
`vehicle_lookinfo.py` 출력 형식이다.

```powershell
$scratch = 'C:\Users\95jus\AppData\Local\Temp\claude\C--Users-95jus-Desktop-TeamProject-LostArk\2fbf9388-0d33-47e4-9638-27cf5a326428\scratchpad\vehicles2'
$tables = 'C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data2\EFGame_Extra\ClientData\TableData'
$look = 'C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data4\EFGame_Extra\ClientData\XmlData\LookInfo'
& $py "$scratch\..\..\e805e476-d593-48e8-9b57-70e5000b69e9\scratchpad\vehicle_lookinfo.py" $tables $look "$scratch\psk" "$work\vehicles.json" 7104 9370 7209
$blender = 'C:\Program Files\Blender Foundation\Blender 5.0\blender.exe'
foreach ($v in @(@(7104,'SilverBattleRaptor'), @(9370,'SereneStarlightBlessing'), @(7209,'RainbowMokoboard'))) {
  & $blender --background --factory-startup --python C:\Users\95jus\Desktop\buildScript\build_npc.py -- `
    "$work\vehicles.json" $v[0] "$work\fbx\$($v[1]).fbx"
}
& $py C:\Users\95jus\Desktop\buildScript\cook_npc.py "$work\vehicles.json" "$scratch\psk" "$work\fbx" `
  "Client\Bin\Resources\Character\Vehicle" "Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe" `
  SilverBattleRaptor=7104 SereneStarlightBlessing=9370 RainbowMokoboard=7209
```

검증(쿠킹 직후, 모델마다):

| 검사 | 통과 기준 |
|---|---|
| `validate_wmodel.py` | OK |
| 재질 이름 | `cook_npc.py`는 LookInfo 교체 MIC의 텍스처를 쓰되 슬롯 이름은 메시 기본값을 유지한다(`mn_isrx_02_mi`/`mn_isrx_00_mi`, `mn_pmssm_00_a_mi`/`_b_mi`, `mn_pmsmk_00_mi`). G05 `rows`는 `dump=family@슬롯이름`으로 이 이름을 지정한다(구현 시 도구에 `@` 추가) |
| 좌석 본·클립 | `b_cockpit` 존재, `npc_idle_normal_1`·`npc_run_normal_1` 존재 |
| UV1 | program 86/87/88/23/25는 UV1을 요구하지 않는다. 쿠킹 버전(1.0/1.3)은 그대로 둔다 |

모코보드는 메시와 AnimSet의 본 순서가 달라, `build_npc.py`의 master rig(`MN_PMSHB_00`) 기준 armature로 굽힌 뒤 클립 트랙이 전부 붙는지
빌드 로그의 unresolved track 수 0으로 확인한다.

---

## G04. 탑승자 애니셋 12개

`out/VehicleTerpeion20260913/cook_riders.ps1`과 같은 방식이다. 모드마다 FBX를 따로 굽는다.

```powershell
$riders = "out\VehicleTerpeion20260913\riders"
$ftPsa = 'C:\Users\95jus\AppData\Local\Temp\claude\C--Users-95jus-Desktop-TeamProject-LostArk\2fbf9388-0d33-47e4-9638-27cf5a326428\scratchpad\family_ani\PC_FT_00\AnimSet\pc_ft_00_ani.psa'
$vehicleAni = 'C:\Users\95jus\AppData\Local\Temp\claude\C--Users-95jus-Desktop-TeamProject-LostArk\2fbf9388-0d33-47e4-9638-27cf5a326428\scratchpad\vehicle_ani'
$classes = @(
  @{ Class='LanceMaster'; Rig="$riders\PC_FLM_00\SkeletalMesh3\pc_flm_00_upper_sk_loc_int.psk"; Ani=$ftPsa; Vehicle="$vehicleAni\PC_FT_00\AnimSet\pc_ft_00_vehicle_ani.psa"; Armature='flm'; Carrier='pc_ft_00_sk.001' },
  @{ Class='Warlord'; Rig="$riders\PC_WR_00\SkeletalMesh3\pc_wr_00_sk.psk"; Ani="$riders\PC_WR_00\AnimSet\pc_wr_00_ani.psa"; Vehicle="$vehicleAni\PC_WR_00\AnimSet\pc_wr_00_vehicle_ani.psa"; Armature='wgl'; Carrier='pc_wr_00_sk.001' },
  @{ Class='Artist'; Rig="$riders\PC_SP_00\SkeletalMesh3\pc_sp_00_sk.psk"; Ani="$riders\PC_SP_00\AnimSet\pc_sp_00_ani.psa"; Vehicle="$vehicleAni\PC_SP_00\AnimSet\pc_sp_00_vehicle_ani.psa"; Armature='sdm'; Carrier='pc_sp_00_sk.001' },
  @{ Class='DimensionMaster'; Rig="$riders\PC_SP_M_00\SkeletalMesh3\pc_sp_m_00_sk.psk"; Ani="$riders\PC_SP_M_00\AnimSet\pc_sp_m_00_ani.psa"; Vehicle="$vehicleAni\PC_SP_M_00\AnimSet\pc_sp_m_00_vehicle_ani.psa"; Armature='pc_sp_m_00_sk'; Carrier='pc_sp_m_00_sk.001' })
$modes = @(@('Raptor','raptor','Ani'), @('Swing','swing','Vehicle'), @('Hoverboard','hoverboard','Ani'))
foreach ($c in $classes) { foreach ($m in $modes) {
  $fbx = "out\VehicleAdditions20260913\riders\$($c.Class)_Ride$($m[0]).fbx"
  $wmodel = "Client\Bin\Resources\Character\$($c.Class)\AnimSets\$($c.Class)_Ride$($m[0])AnimSet.wmodel"
  & $blender --background --factory-startup --python C:\Users\95jus\Desktop\buildScript\build_npc_animset.py -- `
    $c.Rig $fbx $c[$m[2]] --armature-name $c.Armature --carrier-name $c.Carrier `
    --clips "ride_$($m[1])_idle_normal_1,ride_$($m[1])_run_normal_1"
  & Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe $fbx -o $wmodel --no-auto-textures
}}
```

차원술사 3개는 쿠킹 결과가 225 bone이므로 테르페이온과 같이 Esther 애니셋(236 bone) 골격으로 스플라이스한다.

```powershell
foreach ($m in 'Raptor','Swing','Hoverboard') {
  $d = 'Client\Bin\Resources\Character\DimensionMaster\AnimSets'
  Copy-Item "$d\DimensionMaster_Ride$($m)AnimSet.wmodel" "out\VehicleAdditions20260913\DimensionMaster_Ride$($m)AnimSet.225.wmodel"
  & $py out\VehicleTerpeion20260913\splice_skeleton.py "$d\DimensionMaster_EstherAnimSet.wmodel" `
    "out\VehicleAdditions20260913\DimensionMaster_Ride$($m)AnimSet.225.wmodel" "$d\DimensionMaster_Ride$($m)AnimSet.wmodel"
}
```

12개 전부 `compare_attach.py <Class>_EstherAnimSet.wmodel <candidate>`가 validate OK, bone table·skeletonHash 일치해야 한다(224/218/239/236).
클립 이름은 `<armature>_ride_<mode>_{idle,run}_normal_1`이다.
단, WModel section 이름은 40바이트라 차원술사 호버보드는 `pc_sp_m_00_sk_ride_hoverboard_idle_norm` / `_run_norma`로 잘린 이름이 런타임 클립 이름이다(구현 시 확인, 두 이름은 서로 달라 충돌 없음).

### `Data/Actors/CharacterCatalog.json`

네 직업의 `animationSetModels` 목록에서 `…_RideHorseAnimSet.wmodel` 줄 뒤에 세 줄을 추가한다(창술사 예시, 나머지 직업도 같은 형식).

```json
        "Character/LanceMaster/AnimSets/LanceMaster_RideHorseAnimSet.wmodel",
        "Character/LanceMaster/AnimSets/LanceMaster_RideRaptorAnimSet.wmodel",
        "Character/LanceMaster/AnimSets/LanceMaster_RideSwingAnimSet.wmodel",
        "Character/LanceMaster/AnimSets/LanceMaster_RideHoverboardAnimSet.wmodel"
```

기존 파일은 CRLF이므로 줄끝을 유지한다.

---

## G05. 데이터

### `Data/Vehicles/VehicleProfiles.json` 전체 코드

```json
{
  "schema": "lostark.vehicle-profiles",
  "formatVersion": 1,
  "vehicles": [
    {
      "vehicleId": 6705,
      "moveSpeed": 5.0,
      "source": { "table": "EFTable_Vehicle", "primaryKey": 6705, "column": "MoveSpeed", "value": 500, "divisor": 100 }
    },
    {
      "vehicleId": 7104,
      "moveSpeed": 5.0,
      "source": { "table": "EFTable_Vehicle", "primaryKey": 7104, "column": "MoveSpeed", "value": 500, "divisor": 100 }
    },
    {
      "vehicleId": 9370,
      "moveSpeed": 5.0,
      "source": { "table": "EFTable_Vehicle", "primaryKey": 9370, "column": "MoveSpeed", "value": 500, "divisor": 100 }
    },
    {
      "vehicleId": 7209,
      "moveSpeed": 5.0,
      "source": { "table": "EFTable_Vehicle", "primaryKey": 7209, "column": "MoveSpeed", "value": 500, "divisor": 100 }
    }
  ]
}
```

`Publish-VehicleProfiles.ps1 -Mode Publish` 후 `Vehicles.bootstrap`은 4행이다. Server 재시작이 필요하다.

### `Data/Actors/VehicleCatalog.json`

`vehicles` 배열의 테르페이온 객체 뒤에 3개를 추가한다. `modelMaterialOverrides`는 `rows` 명령 출력을 그대로 넣는다.

```powershell
& $py $tool rows --model Character/Vehicle/SilverBattleRaptor/SilverBattleRaptor.wmodel `
  --texture-map Tools/VehiclePipeline/SilverBattleRaptor.texture-map.json --out "$work/rows-raptor.json" `
  "$work/dumps/mn_isrx_02.mat.mn_isrx_02-4_mi.json=source.character.monster-d9d6c02905c3.v1@mn_isrx_02_mi" `
  "$work/dumps/mn_isrx_02.mat.mn_isrx_00-4_mi.json=source.character.monster-be5bc0ded311.v1@mn_isrx_00_mi"
& $py $tool rows --model Character/Vehicle/SereneStarlightBlessing/SereneStarlightBlessing.wmodel `
  --texture-map Tools/VehiclePipeline/SereneStarlightBlessing.texture-map.json --out "$work/rows-starlight.json" `
  "$work/dumps/mn_pmssm_00.mat.mn_pmssm_00_b_mi.json=source.vehicle.starlight-body.v1" `
  "$work/dumps/mn_pmssm_00.mat.mn_pmssm_00_a_mi.json=source.vehicle.starlight-shell-translucent.v1"
& $py $tool rows --model Character/Vehicle/RainbowMokoboard/RainbowMokoboard.wmodel `
  --texture-map Tools/VehiclePipeline/RainbowMokoboard.texture-map.json --out "$work/rows-mokoboard.json" `
  "$work/dumps/mn_pmsmk_00.mat.mn_pmsmk_00-4_vfx_mi.json=source.vehicle.mokoboard-vfx.v1@mn_pmsmk_00_mi"
```

추가할 객체의 고정 필드(`modelMaterialOverrides` 값만 위 출력):

```json
    {
      "vehicleId": 7104,
      "archetypeId": "VEHICLE_RAPTOR_SILVER_BATTLE",
      "modelAssetId": "Character/Vehicle/SilverBattleRaptor/SilverBattleRaptor.wmodel",
      "modelPreScale": 0.0001,
      "seatBone": "b_cockpit",
      "vehicleIdleClip": "npc_idle_normal_1",
      "vehicleRunClip": "npc_run_normal_1",
      "riders": [
        { "characterClass": "LANCE_MASTER", "idleClip": "flm_ride_raptor_idle_normal_1", "runClip": "flm_ride_raptor_run_normal_1" },
        { "characterClass": "WARLORD", "idleClip": "wgl_ride_raptor_idle_normal_1", "runClip": "wgl_ride_raptor_run_normal_1" },
        { "characterClass": "ARTIST", "idleClip": "sdm_ride_raptor_idle_normal_1", "runClip": "sdm_ride_raptor_run_normal_1" },
        { "characterClass": "DIMENSIONMASTER", "idleClip": "pc_sp_m_00_sk_ride_raptor_idle_normal_1", "runClip": "pc_sp_m_00_sk_ride_raptor_run_normal_1" }
      ],
      "modelMaterialOverrides": "<rows-raptor.json 배열>",
      "runtimeStatus": "supported"
    },
    {
      "vehicleId": 9370,
      "archetypeId": "VEHICLE_STARLIGHT_BLESSING_SERENE",
      "modelAssetId": "Character/Vehicle/SereneStarlightBlessing/SereneStarlightBlessing.wmodel",
      "modelPreScale": 0.0001,
      "seatBone": "b_cockpit",
      "vehicleIdleClip": "npc_idle_normal_1",
      "vehicleRunClip": "npc_run_normal_1",
      "riders": [
        { "characterClass": "LANCE_MASTER", "idleClip": "flm_ride_swing_idle_normal_1", "runClip": "flm_ride_swing_run_normal_1" },
        { "characterClass": "WARLORD", "idleClip": "wgl_ride_swing_idle_normal_1", "runClip": "wgl_ride_swing_run_normal_1" },
        { "characterClass": "ARTIST", "idleClip": "sdm_ride_swing_idle_normal_1", "runClip": "sdm_ride_swing_run_normal_1" },
        { "characterClass": "DIMENSIONMASTER", "idleClip": "pc_sp_m_00_sk_ride_swing_idle_normal_1", "runClip": "pc_sp_m_00_sk_ride_swing_run_normal_1" }
      ],
      "modelMaterialOverrides": "<rows-starlight.json 배열>",
      "runtimeStatus": "supported"
    },
    {
      "vehicleId": 7209,
      "archetypeId": "VEHICLE_MOKOBOARD_RAINBOW",
      "modelAssetId": "Character/Vehicle/RainbowMokoboard/RainbowMokoboard.wmodel",
      "modelPreScale": 0.0001,
      "seatBone": "b_cockpit",
      "vehicleIdleClip": "npc_idle_normal_1",
      "vehicleRunClip": "npc_run_normal_1",
      "riders": [
        { "characterClass": "LANCE_MASTER", "idleClip": "flm_ride_hoverboard_idle_normal_1", "runClip": "flm_ride_hoverboard_run_normal_1" },
        { "characterClass": "WARLORD", "idleClip": "wgl_ride_hoverboard_idle_normal_1", "runClip": "wgl_ride_hoverboard_run_normal_1" },
        { "characterClass": "ARTIST", "idleClip": "sdm_ride_hoverboard_idle_normal_1", "runClip": "sdm_ride_hoverboard_run_normal_1" },
        { "characterClass": "DIMENSIONMASTER", "idleClip": "pc_sp_m_00_sk_ride_hoverboard_idle_norm", "runClip": "pc_sp_m_00_sk_ride_hoverboard_run_norma" }
      ],
      "modelMaterialOverrides": "<rows-mokoboard.json 배열>",
      "runtimeStatus": "supported"
    }
```

`"<… 배열>"` 문자열은 문서 표기이며 실제 JSON에는 rows 출력 배열 자체를 넣는다. 파서는 객체 필드 수 10개를 검사하므로 필드를 더하지 않는다.
modelPreScale 0.0001과 -90° admission 회전은 NPC 파이프라인 쿠킹본의 기존 계약이며, 크기가 원작과 다르면 사용자 확인 후 조정한다.

---

## G06. 반투명 forward 일반화

### `Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl`

기준점 1: `PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT`의 `if (18u != g_SourceCharacterProgram) discard;` 교체.

```hlsl
    if (18u != g_SourceCharacterProgram && 88u != g_SourceCharacterProgram) discard;
```

기준점 2: 같은 함수의 광원 루프 안 두 줄 교체.

```hlsl
        const SOURCE_CHARACTER_NATIVE_OUTPUT lit = SourceCharacterLight18(
            MakeSourceCharacterForwardLightInput(input, camera, direction, colorExponent.rgb));
```

```hlsl
        const SOURCE_CHARACTER_NATIVE_INPUT lightInput =
            MakeSourceCharacterForwardLightInput(input, camera, direction, colorExponent.rgb);
        SOURCE_CHARACTER_NATIVE_OUTPUT lit;
        if (18u == g_SourceCharacterProgram)
            lit = SourceCharacterLight18(lightInput);
        else
            lit = SourceCharacterLight88(lightInput);
```

기준점 3: technique의 `pass SourceCharacterTranslucentTwoSided` 블록 닫는 `}` 바로 아래 추가.

```hlsl
    // Appended index 10: source translucent one-sided surface, forward lit after scene lighting.
    pass SourceCharacterTranslucentOneSided
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectSourceModelVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT();
    }
```

program 88 base PS도 MRT(o0, o2~o5)를 쓰므로 deferred 합성식(`diffuse × ambient + direct → fog → + indirect`, alpha = `targets[0].a`)을 그대로 적용한다.
light 88의 입력 배치는 program 18과 같은 공통 LIGHT_PASS 배치(특수 목록 밖)다.

### `Client/Private/Part_Vehicle.cpp`

기준점: 익명 namespace 전체 교체.

```cpp
namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_TRANSLUCENT_ONE_SIDED_PASS = 10u;

	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		switch (surface->sourceCharacter.program)
		{
		case 18u: return SOURCE_TRANSLUCENT_TWO_SIDED_PASS;
		case 88u: return SOURCE_TRANSLUCENT_ONE_SIDED_PASS;
		default: return 0u;
		}
	}
}
```

기준점: `Initialize`의 `m_hasTranslucentMeshes |= Is_TranslucentSourceHair(m_pModelCom->Get_MaterialSurface(i));` 교체.

```cpp
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
```

기준점: `Render`의 `if (Is_TranslucentSourceHair(surface))` 교체.

```cpp
		if (0u != Resolve_TranslucentSourcePass(surface))
```

기준점: `Render_Translucent`의 루프 본문 전체 교체.

```cpp
	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
```

별빛의 가호는 외피(A, 반투명)와 몸체(B, 불투명 masked)가 한 모델이다. 한 오브젝트 안의 반투명 submesh는 draw 순서대로 그려진다.

---

## G07. F1 기본 탈것 선택

### `Client/Public/PlayerController.h`

기준점: public `bool_t Request_Revive();` 바로 아래 추가.

```cpp
		/* Debug F1 choice of the vehicle H mounts. Zero, or a vehicle without a
		rider pose for the class, falls back to the first catalog vehicle that has one. */
		static void Set_PreferredVehicleId(std::uint32_t vehicleId) { s_iPreferredVehicleId = vehicleId; }
		static std::uint32_t Get_PreferredVehicleId() { return s_iPreferredVehicleId; }
```

기준점: `bool_t m_wasVehicleKeyDown = false;` 바로 위 추가.

```cpp
		inline static std::uint32_t s_iPreferredVehicleId = 0u;
```

### `Client/Private/PlayerController.cpp`

기준점: `Update_VehicleRiding`의 `if (INVALID_VEHICLE_ID == player.iVehicleId)` 블록 안 `for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())` 루프 교체.

```cpp
		const VEHICLE_ACTOR_ENTRY* preferred = CActorCatalog::Find_Vehicle(s_iPreferredVehicleId);
		if (nullptr != preferred && nullptr != preferred->Find_Rider(character->Get_CharacterClass()))
			requested = preferred->vehicleId;
		for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
		{
			if (INVALID_VEHICLE_ID != requested)
				break;
			if (nullptr != vehicle.Find_Rider(character->Get_CharacterClass()))
				requested = vehicle.vehicleId;
		}
```

### `Client/Private/MainApp.cpp`

기준점: `if (ImGui::CollapsingHeader("Esther Cutin (Debug)"))` 바로 위 추가.

```cpp
	if (ImGui::CollapsingHeader("Vehicle Riding (Debug)"))
	{
		ImGui::TextDisabled(
			"H mounts the selected vehicle. A class without its rider pose uses the first catalog vehicle.");
		const HUD_PLAYER_STATE& ridingPlayer = CCombatHUDViewModel::Get().Get_Player();
		const std::uint32_t preferredVehicleId = CPlayerController::Get_PreferredVehicleId();
		if (ImGui::RadioButton("First available##VehicleRiding", 0u == preferredVehicleId))
			CPlayerController::Set_PreferredVehicleId(0u);
		for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
		{
			const bool_t hasRider = ridingPlayer.isValid &&
				nullptr != vehicle.Find_Rider(ridingPlayer.eCharacterClass);
			const std::string label = vehicle.archetypeId + "  (" +
				std::to_string(vehicle.vehicleId) + ")" + (hasRider ? "" : "  - no rider pose") +
				"##VehicleRiding" + std::to_string(vehicle.vehicleId);
			if (ImGui::RadioButton(label.c_str(), vehicle.vehicleId == preferredVehicleId))
				CPlayerController::Set_PreferredVehicleId(vehicle.vehicleId);
		}
		ImGui::TextDisabled("Riding now: %u", ridingPlayer.iVehicleId);
	}
```

선택은 프로세스 세션 동안만 유지되고 저장하지 않는다. 탑승 중에 바꾸면 다음 H(하차) 뒤 H부터 적용된다.
`MainApp.cpp`는 이미 `ActorCatalog.h`·`CombatHUDViewModel.h`를 include하고 `CPlayerController`를 사용한다.

---

## G08. 문서와 검증

### 문서 갱신

- `CLAUDE.md` 탈것 문단: 탈것 4종, F1 `Vehicle Riding (Debug)` 기본 탈것 선택, program 86/87/88·반투명 88, 모드별 탑승자 애니셋.
- `.md/GB/렌더링이펙트복원V2.md`: 연결 범위 표에 세 탈것 행.
- `.md/GB/gotchas.md`: `FNormalParameter` 29B 항목, 생성기 clamp/cube 지원 전에는 realpbr 계열이 불완전하다는 항목.
- RESULT: `.md/JS/09-14/2026-09-14_VEHICLE_ADDITIONS_RESULT.md`.

### 자동 검증 (에이전트)

| 순서 | 검사 | 통과 기준 |
|---|---|---|
| 1 | verify program 24, 85 | EXACT 유지 |
| 2 | verify program 86, 87, 88 (설치 직후) | EXACT |
| 3 | Engine/Client hlsli SHA-256 | 두 복사본 동일 |
| 4 | 탈것 wmodel 3개 `validate_wmodel` + 재질 이름·`b_cockpit`·idle/run 클립 | 전부 통과 |
| 5 | 탑승자 애니셋 12개 `compare_attach.py` | OK, bone table·skeletonHash 일치 |
| 6 | JSON parse: VehicleCatalog, VehicleProfiles, CharacterCatalog | 성공, override 행 수가 rows 출력과 같음 |
| 7 | `Publish-VehicleProfiles.ps1` Validate → Publish | `Vehicles.bootstrap` 4행 |
| 8 | `fxc /T fx_5_0 Shader_VtxAnimMeshBinary.hlsl` | 성공, 신규 경고 없음 |
| 9 | Product Debug 빌드(Server·Client 종료 후) | Engine/Shared/Server/Client PASS |
| 10 | `Server.exe --vehicle-riding-contract-test` | failures 0 |
| 11 | `git diff --check` | 경고 0 |

### 사용자 확인 (에이전트는 실행·캡처하지 않음)

1. F1 `Vehicle Riding (Debug)`에서 각 탈것 선택 → H: 해당 탈것이 나오고 탑승 자세가 모드에 맞는지(랩터 앉기, 스윙 그네, 모코보드 서기).
2. 이동 시 탈것·탑승자 run 클립과 속도(5 m/s).
3. 은색 전투 랩터 재질(은색·장갑 발광), 별빛의 가호 파란 외피 반투명과 몸체, 모코보드 무지개 VFX.
4. 크기와 좌석 높이가 원작과 비슷한지. 모코보드 idle 루프가 탑승자와 어긋나 보이는지.
5. 직업 변경·발탄 입장 시 하차(기존 동작 유지).

### 남는 경계

- 탈것 LookInfo 파티클·스폰 이펙트·발소리, 탈것 고유 스킬.
- 별빛의 가호 나머지 7색, 다른 모코보드·랩터 색.
- 제품 탈것 선택 UI(원작 F1 창).
- Gunslinger/Slayer 탑승자 애니셋.
