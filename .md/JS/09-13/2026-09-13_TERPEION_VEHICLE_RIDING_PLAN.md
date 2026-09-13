# 2026-09-13 황금 테르페이온 탈것 탑승 수직 슬라이스 PLAN

작성자: JS · 브랜치 `feature/terpeion-vehicle-riding` (origin/main `e6f19ec8`에서 분기)

## G00. 목표와 실측 근거

H 키로 황금 테르페이온(EFTable_Vehicle 6705)에 타고 내린다. 탑승 여부와 이동 속도는 Server가
소유하고, Client는 snapshot을 보고 말 몸체를 붙이고 탑승 자세 클립을 재생한다. 말 몸체는 원작
MIC의 native shader program으로 그린다.

### 원본에서 확정한 사실

| 항목 | 원본 값 | 근거 |
|---|---|---|
| 탈것 행 | `EFTable_Vehicle` PK 6705, Model `EFDLVehi_MN_PMSTG_00-3`, MoveSpeed 500, RidingMode 1 | `data2 EFTable_Vehicle.db` 직접 조회 |
| 몸체·재질 | `MN_PMSTG_01_SK`, MIC `01-1_mi / 01-3_mi / 01_mi` (wmodel 슬롯 순서 동일) | LookInfo `.loa`, psk props, wmodel WMA2 문자열 |
| RidingMode 1 | `ACTION_CONDITION_RIDINGMODE_OUTPUT_HORSE` | `NU1V7NCQ4YAE9ZPJVNOQS.u`의 `ActionConditionRidingModeOutput` enum 순서 (0~50) |
| 탑승자 클립 | `ride_horse_idle_normal_1`(101f), `ride_horse_run_normal_1`(29f) | `PC_FT_00` 가족 `pc_ft_00_ani.psa` ANIMINFO. 말 `idle_normal_1` 101f, `run_normal_1` 29f와 프레임 수 일치 |
| 좌석 | 소켓 `Cockpit` = 본 `b_cockpit`, offset 0 | `mn_pmstg_01_sk.props.txt`, Terpeion.wmodel 본 표 |
| 원작 입력 | F1 탈것 선택 → 우리는 F1이 Debug ImGui라 H 키 토글 | 사용자 결정 |
| 허용 월드 | Bern, Character Select만. 레이드 월드 요청은 typed 거부, 탑승 중 이동하면 새 방 player가 도보로 시작 | 사용자 결정 |
| 탑승 중 스킬 | 전부 거부(탈것 고유 스킬은 후속) | 사용자 결정 |

### shader map 조사 결과

`Tools/VehiclePipeline/build_vehicle_source_material.py extract`가 retail
`EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk`에서 MIC의 static set으로 정확한 material map을 고르고
GPU skin BasePass / directional light PS를 뽑았다.

| MIC | static switch(on) | Base PS / Light PS | 결론 |
|---|---|---|---|
| `mn_pmstg_01_mi` 몸통 | masked, color_variation, ibl_color, state, emissive, flicker, state_noise | `2b41020e…` / `efdfb38a…` | 기존 program에 없음 → **신규 program 85** |
| `mn_pmstg_01-1_mi` 장식 | masked, color_variation, ibl_color, state, state_noise | `0d0f537e…` / `70f1c24c…` | 카드미로 `mn_ppch_00-1_mi`와 PS 동일 → 기존 **program 24** 재사용 |
| `mn_pmstg_01-3_mi` 갈기 | hairtwotone, hairtwotonerange, state, state_noise (`pc_hair_trn`) | `dea8fd54…` / `3b0966d4…` | 도화가 `pc_sp_06_hair_mi`와 PS 동일 → 기존 **program 18** (`hair-two-tone.v1`) 재사용 |

생성기 신뢰성 게이트: 같은 도구로 원본 `mn_ppch_00-1_mi`를 다시 생성하면 설치된
`SourceCharacterBase24/Light24`와 packing 블록이 **바이트 단위로 일치**한다(base 654줄, light 630줄, configure EXACT).
program 21(공)은 literal 표기만 다른 구 생성본이며 값 정규화 비교 시 명령 346/315줄 전부 일치한다.

어제 구운 `textures/*.tga`(diffusecolor를 8bit에 곱해 1에서 잘린 것)는 native 경로에서 쓰지 않는다.
원본 TGA를 `Character/Vehicle/Terpeion/SourceMaterials/`에 두고 MIC 파라미터는 HDR 그대로 셰이더에 넘긴다.

### 설계 경계

- 두 번째 모델 런타임을 만들지 않는다. 말 몸체는 `CModel → CMaterial → SourceCharacter` 기존 경로.
- 탈것은 NPC가 아니다. `Data/Actors/VehicleCatalog.json`이 Client 표현 정본, `Data/Vehicles/VehicleProfiles.json`이
  Server 수치 정본. NpcCatalog의 `VEHICLE_TERPEION_GOLD` 행은 삭제하고 Effect Tool V2 Attach는 VehicleCatalog를 읽는다.
- Server는 vehicleId와 이동 속도만 안다. 클립·모델·좌석 본은 Client catalog만 안다.
- 탑승자는 `CCharacter`에 `CPart_Vehicle` 파츠가 붙은 상태다. 새 GameObject를 따로 레이어에 두지 않아
  보간·예측·nameplate·collider가 한 transform을 공유한다.
- 이번 범위의 탑승자 애니셋은 4직업(창술사·워로드·도화가·차원술사)만 쿠킹한다. 다른 직업은 H를 눌러도 요청하지 않는다.
- protocol 81 → 82. Server와 Client를 같이 빌드·재시작해야 한다.

### 대화형 설명 순서와 파일 목록

| G | 내용 | 파일 |
|---|---|---|
| G01 | 원본 재질 추출·program 85 설치·텍스처 배치 | `Tools/VehiclePipeline/build_vehicle_source_material.py`(신규), `Tools/VehiclePipeline/Terpeion.texture-map.json`(신규), Engine/Client `Shader_SourceCharacter{Base,Light}Programs.hlsli`, `Client/Public/SourceCharacterMaterialParameters.h`, `Engine/Private/Model.cpp` |
| G02 | 탑승자 애니셋 4직업 쿠킹 | Resources `Character/<Class>/AnimSets/<Class>_RideHorseAnimSet.wmodel`, `Data/Actors/CharacterCatalog.json` |
| G03 | Shared protocol 82 | `NetworkIds.h`, `PacketType.h`, `PacketMessages.h/.cpp`, `NetworkProtocolHarness.cpp` |
| G04 | Server 권위 탑승 상태 | `VehicleCatalog.h/.cpp`(신규), `GameRoom_VehicleRiding.cpp`(신규), `ServerPlayer.h`, `RoomCommand.h`, `GameRoom.h/.cpp`, `ServerApp.cpp`, `GameRoom_PlayerCommands.cpp`, `GameRoom_PlayerSimulation.cpp`, `GameRoom_Replication.cpp`, `Main.cpp`, `ServerGameplayContractTests.h`, `ServerGameplayContractTests_VehicleRiding.cpp`(신규), `Server.vcxproj/.filters` |
| G05 | Server 수치 데이터·publisher | `Data/Vehicles/VehicleProfiles.json`(신규), `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`(신규), `BuildDomains.json`, `Invoke-BuildDomainOwner.ps1`, `Invoke-BuildAndRegression.ps1` |
| G06 | Client catalog | `Data/Actors/VehicleCatalog.json`(신규), `Data/Actors/NpcCatalog.json`, `ActorCatalog.h/.cpp` |
| G07 | 탈것 표현 | `Part_Vehicle.h/.cpp`(신규), `VehiclePresentationAssetService.h/.cpp`(신규), `Character.h/.cpp`, `ClientReplication.cpp`, `Loader.cpp`, `Client.vcxproj/.filters` |
| G08 | H 키 명령 | `PlayerCommandSink.h`, `NetworkPlayerCommandSink.h/.cpp`, `NetworkManager.h/.cpp`, `PlayerController.h/.cpp`, `CombatHUDViewModel.h/.cpp` |
| G09 | Effect Tool V2 Attach 이전 | `NpcPresentationAssetService.h/.cpp`, `Effect_Tool_V2.h/.cpp`, `EffectV2_Runtime.cpp` |
| G10 | 문서·검증 | `CLAUDE.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, RESULT |

전체 코드 원칙: 새 파일은 전문을 싣는다. 1000줄을 넘는 기존 파일(`PacketMessages.cpp`, `ClientReplication.cpp`,
`Character.cpp`, `GameRoom.h` 등)은 전문 대신 **현재 파일에 실제로 존재하는 기준 줄**과 교체/추가 블록을 싣는다.
생성 HLSL(1,317줄)은 생성 명령과 SHA-256으로 고정한다.

---

## G01. 원본 재질 추출과 program 85

### 절차

```powershell
$py = 'C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe'
$tool = 'Tools/VehiclePipeline/build_vehicle_source_material.py'
$work = 'out/VehicleTerpeion20260913'

# 1. MIC → shader map → PS/uniform/texture dump (retail 패키지 read-only)
& $py $tool extract `
  --umodel 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe' `
  --d3dcompiler 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\d3dcompiler_47.dll' `
  --out "$work/dumps" `
  mn_ppch_00.mat.mn_ppch_00-1_mi `
  mn_pmstg_01.mat.mn_pmstg_01_mi mn_pmstg_01.mat.mn_pmstg_01-1_mi mn_pmstg_01.mat.mn_pmstg_01-3_mi `
  pc_sp_06_hair.mat.pc_sp_06_hair_mi

# 2. 생성기 게이트: 설치된 program 24를 원본에서 다시 만들어 바이트 일치해야 다음 단계로 간다
& $py $tool verify --dump "$work/dumps/mn_ppch_00.mat.mn_ppch_00-1_mi.json" `
  --family source.character.monster-d621a47e69ad.v1 --program 24

# 3. program 85 생성 → 설치 (Engine 원본 + Client 복사본 + packing 블록)
& $py $tool generate --dump "$work/dumps/mn_pmstg_01.mat.mn_pmstg_01_mi.json" `
  --family source.vehicle.terpeion-body.v1 --program 85 --out "$work/generated"
& $py $tool install --generated "$work/generated" --family source.vehicle.terpeion-body.v1 --program 85

# 4. 원본 TGA 배치 (staging에 없으면 먼저 umodel로 다시 export)
& 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe' -export -uncook -game=lostark -kr -nameresolve `
  -path='C:\ProgramData\Smilegate\Games\LOSTARK\EFGame' -out="$work/staging" MN_PMSTG_00 MN_PMSTG_01 MN_PMSHS_00
& $py $tool textures --texture-map Tools/VehiclePipeline/Terpeion.texture-map.json --staging "$work/staging"

# 5. catalog 행 (G06 VehicleCatalog.json의 modelMaterialOverrides로 붙인다)
& $py $tool rows --model Character/Vehicle/Terpeion/Terpeion.wmodel `
  --texture-map Tools/VehiclePipeline/Terpeion.texture-map.json --out "$work/rows.json" `
  "$work/dumps/mn_pmstg_01.mat.mn_pmstg_01_mi.json=source.vehicle.terpeion-body.v1" `
  "$work/dumps/mn_pmstg_01.mat.mn_pmstg_01-1_mi.json=source.character.monster-d621a47e69ad.v1" `
  "$work/dumps/mn_pmstg_01.mat.mn_pmstg_01-3_mi.json=source.character.hair-two-tone.v1"
```

생성 결과 고정값(2026-09-13 스크래치 생성본):

| 파일 | 줄 | SHA-256 |
|---|---|---|
| `base85.hlsli` | 688 | `1a2b6e20ce931232491020c6ec378c9c2864732f98c7003962d2566acb1b17f1` |
| `light85.hlsli` | 629 | `70e8623c669799cb9df2f2d6f1f58404b873b8b6fe95e8d14c31f925f085a67b` |
| `configure85.h` | 54 | `08415c5f5f64574b3d49a6caad4f4423f73ee0504df103ad466fe37872db921a` |

`install`은 기존 program 번호를 바꾸지 않는다. `Evaluate…` 함수 앞에 함수를 넣고 `case 84u:` 다음 줄에
`case 85u:`를 추가하며 Engine 파일을 Client `Bin/ShaderFiles`에 그대로 복사한다(두 복사본은 현재 바이트 동일).
packing 블록은 `    else return false;` + `    if (staged.program == 80u)` 기준점 바로 위에 들어간다.

### `Engine/Private/Model.cpp` — native program 상한

기준점: `Apply_MaterialOverrides` 안 `const uint32_t mask = source.baseTextureMask | source.lightTextureMask;` 다음 줄. 교체.

```cpp
            if (source.program == 0u || source.program > 85u || (source.program > 65u && source.program < 80u) ||
```

program 85는 `Shader_SourceCharacterMaterial.hlsli`의 특수 입력 목록(5/6/7/12/18/19/20/22/29/84)에 속하지 않으므로 그 파일은 바꾸지 않는다.
baked lighting·UV1 요구도 없다(program 21과 같은 monster PBR 입력).

### `Tools/VehiclePipeline/build_vehicle_source_material.py` 전체 코드

```python
"""Source-exact native material programs and catalog rows for vehicle bodies.

extract    reads each MIC's static shader map from the retail RefShaderCache and
           writes its GPU-skin BasePass/directional-light pixel programs, uniform
           expressions, effective parameters and referenced textures.
verify     regenerates an installed program from its original MIC and requires
           the installed Base/Light functions to match byte for byte.
generate   writes one new Base/Light function pair and its CPU packing block.
install    inserts a generated program into both SourceCharacter program files
           and SourceCharacterMaterialParameters.h without renumbering others.
rows       emits the catalog modelMaterialOverrides rows for extracted MICs.
textures   copies the exact source textures the rows reference into Resources.
"""
from __future__ import annotations

import argparse
import functools
import hashlib
import json
import pathlib
import re
import shutil
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/LevelPlacementExtractor'), str(ROOT / 'Tools/EffectPipeline')]

BASE_TYPE = 'tbasepasspixelshaderfnolightmappolicyskylight'
LIGHT_TYPE = 'tlightpixelshaderfdirectionallightpolicyfnostaticshadowingpolicy'
GPU_SKIN_VF = 'fgpuskinvertexfactory'
LOOKUP_TEXTURES = {'texture_ibl', 'texture_brdf'}
TIME_PARAMETER_ROW = 63
RELEASE = pathlib.Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
BASE_PROGRAMS = ROOT / 'Engine/Bin/ShaderFiles/Shader_SourceCharacterBasePrograms.hlsli'
LIGHT_PROGRAMS = ROOT / 'Engine/Bin/ShaderFiles/Shader_SourceCharacterLightPrograms.hlsli'
CLIENT_SHADER_DIR = ROOT / 'Client/Bin/ShaderFiles'
PARAMETER_HEADER = ROOT / 'Client/Public/SourceCharacterMaterialParameters.h'
RESOURCES = ROOT / 'Client/Bin/Resources'


def fail(message):
    raise SystemExit('build_vehicle_source_material: ' + message)


def open_cache(d3dcompiler: pathlib.Path):
    import extract_artist_31470_shader_cache_oracle as oracle
    import extract_artist_31470_main_ref_shader_cache as ref_cache
    import extract_ue3_material_shader_maps as sm
    read_fname = oracle.read_fname

    def numbered_read_fname(data, offset, names):
        name, number, rest = read_fname(data, offset, names)
        return (f'{number}.{name}' if number else name), 0, rest
    oracle.read_fname = numbered_read_fname
    ref_cache.EXPECTED_D3DCOMPILER['byteSize'] = d3dcompiler.stat().st_size
    ref_cache.EXPECTED_D3DCOMPILER['sha256'] = hashlib.sha256(d3dcompiler.read_bytes()).hexdigest()
    cache = sm.package_tables(RELEASE / 'EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk')
    return sm, cache, sm.parse_shader_code_layout(cache), sm.D3DDisassembler(d3dcompiler)


def command_extract(arguments):
    from extract_ue3_effect_material_closure import (
        LOSTARK_KR_AES_KEY, decode_material_instance, find_export, load_package,
        package_ref_name, package_ref_path, parse_tagged_properties, tagged_value)
    from extract_ue3_placements import resolve_physical_package
    sm, cache, layout, disassembler = open_cache(arguments.d3dcompiler)

    @functools.lru_cache(None)
    def package(name):
        return load_package(resolve_physical_package(arguments.umodel, RELEASE, name, 'kr'), LOSTARK_KR_AES_KEY)

    def full_reference(name, loaded, reference):
        path = package_ref_path(reference, loaded.imports, loaded.exports)
        return name + '.' + path if reference > 0 else path

    @functools.lru_cache(None)
    def export(path):
        name, relative = path.split('.', 1)
        loaded = package(name)
        entry = find_export(loaded, relative)
        serial = loaded.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        properties, end = parse_tagged_properties(serial, loaded.names, loaded.summary.version)
        return dict(package=name, className=package_ref_name(entry.class_index, loaded.imports, loaded.exports),
                    properties=properties, tail=serial[end:])

    @functools.lru_cache(None)
    def material(path):
        current = export(path)
        loaded = package(current['package'])
        if current['className'] == 'materialinstanceconstant':
            parent = material(full_reference(current['package'], loaded, tagged_value(current['properties'], 'parent')))
            static = sm.decode_static_set_from_tail(current['tail'], bytes.fromhex(parent['baseId']), loaded.names,
                                                    sm.POLICY_BLOCK_ABSENT)
            if static.get('status') == sm.STATUS_BLOCKED:
                static = parent['static']
            original = decode_material_instance(loaded, path.split('.', 1)[1])
            scalars, vectors, textures = dict(parent['scalars']), dict(parent['vectors']), dict(parent['textures'])
            for row in original['scalarParameters']:
                scalars[row['name']] = row['value']
            for row in original['vectorParameters']:
                vectors[row['name']] = row['value']
            for row in original['textureParameters']:
                textures[row['name']] = full_reference(current['package'], loaded, row['packageIndex']) if row['packageIndex'] else None
            return dict(parentMaterial=parent['parentMaterial'], baseId=parent['baseId'], static=static,
                        scalars=scalars, vectors=vectors, textures=textures)
        base_id = current['tail'][16:32].hex()
        parsed = sm.parse_static_parameter_set(bytes.fromhex(base_id) + struct.pack('<IIII', 0, 0, 0, 0), 0, loaded.names)
        static = {'staticParameterSet': sm.public_static_set(parsed),
                  'engineEqualityStaticParameterSetSha256': sm.canonical_json_sha256(sm.engine_equivalent_static_parameter_set(parsed))}
        return dict(parentMaterial=path, baseId=base_id, static=static, scalars={}, vectors={}, textures={})

    rows = {target: material(target) for target in arguments.materials}
    scans = sm.scan_base_material_contexts(cache, layout, sorted({row['baseId'] for row in rows.values()}))
    arguments.out.mkdir(parents=True, exist_ok=True)
    for target, row in rows.items():
        equality = row['static']['engineEqualityStaticParameterSetSha256']
        material_map = sm.parse_material_map(cache, layout, sm.select_unique_map_context(scans[row['baseId']], equality), equality)
        factories = [v for v in material_map['vertexFactories'] if v['vertexFactoryType'] == GPU_SKIN_VF]
        if len(factories) != 1:
            fail(f'{target} has no unique GPU-skin vertex factory')
        references = [s for s in factories[0]['shaderReferences'] if s['shaderType'] in (BASE_TYPE, LIGHT_TYPE)]
        if sorted(s['shaderType'] for s in references) != sorted((BASE_TYPE, LIGHT_TYPE)):
            fail(f'{target} lacks its BasePass or directional light pixel shader')
        bytecodes = sm.extract_selected_packed_dxbc(cache, layout, references)
        objects = sm.extract_selected_shader_objects(cache, layout, references, allow_mixed_code_preambles=True)
        programs = {}
        for reference in references:
            bytecode = bytecodes[reference['shaderIdHex']]['_bytecode']
            disassembly = disassembler.disassemble(bytecode)
            closure = sm.parse_dxbc_declaration_closure(disassembly, allow_textureless=True)
            raw = objects['byShaderId'][reference['shaderIdHex']]
            bindings = sm.select_unique_native_binding_arrays(raw['_bytes'], raw['logicalOffset'],
                                                              material_map['uniformExpressionCounts'], closure)
            programs[reference['shaderType']] = dict(shaderId=reference['shaderIdHex'],
                                                     disassembly=dict(declarations=disassembly['declarations'],
                                                                      instructions=disassembly['instructions']),
                                                     bindings=bindings)
        source = export(target)
        loaded = package(source['package'])
        count = struct.unpack_from('<I', source['tail'], 36)[0]
        texture_expressions = material_map['uniformExpressionSet']['pixelTexture2DExpressions']
        if not (len(texture_expressions) <= count <= 64 and len(source['tail']) >= 40 + 4 * count):
            fail(f'{target} referenced texture table is not at the static-resource offset')
        referenced = struct.unpack_from('<' + 'i' * count, source['tail'], 40)
        textures = []
        for index, expression in enumerate(texture_expressions):
            name = expression.get('parameterName')
            path = row['textures'].get(name) if name else None
            if not path:
                path = full_reference(source['package'], loaded, referenced[expression['referencedTextureIndex']])
            srgb = tagged_value(export(path)['properties'], 'srgb')
            textures.append(dict(expressionIndex=index, parameterName=name, sourceObject=path,
                                 srgb=True if srgb is None else bool(srgb)))
        document = dict(sourceMaterial=target, parentMaterial=row['parentMaterial'], mapKey=equality,
                        scalars=row['scalars'], vectors=row['vectors'], textures=textures,
                        uniformExpressionSet={k: material_map['uniformExpressionSet'][k] for k in
                                              ('pixelVectorExpressions', 'pixelScalarExpressions', 'pixelTexture2DExpressions')},
                        programs=programs)
        path = arguments.out / (target + '.json')
        path.write_text(json.dumps(document, indent=1, default=str), encoding='utf8')
        print('extracted', target, programs[BASE_TYPE]['shaderId'], programs[LIGHT_TYPE]['shaderId'])


def split_arguments(text):
    return [x.strip() for x in re.split(r',\s*(?![^()]*\))', text)]


def immediate(text, integer=False):
    numbers = [x.strip() for x in text[2:-1].split(',')]
    numbers += numbers[-1:] * (4 - len(numbers))
    if integer:
        return 'uint4(' + ','.join(((hex(int(n, 16) & 4294967295) if n.lower().startswith('0x') else str(int(n, 10) & 4294967295)) + 'u' for n in numbers)) + ')'

    def lane(n):
        if n.lower().startswith('0x'):
            return 'asfloat(' + n + 'u)'
        return n if '.' in n or 'e' in n.lower() else 'asfloat(' + str(int(n, 10) & 4294967295) + 'u)'
    return 'float4(' + ','.join(lane(n) for n in numbers) + ')'


def operand(text, destination=False):
    text = text.strip()
    if text == 'null':
        return 'unused'
    negative = text.startswith('-')
    text = text[1:] if negative else text
    absolute = text.startswith('|') and text.endswith('|')
    text = text[1:-1] if absolute else text
    if text.startswith('l('):
        text = immediate(text)
    else:
        text = re.sub(r'\bcb0\[', 'source[', text)
        text = re.sub(r'\bcb1\[', 'projection[', text)
        text = re.sub(r'\bcb2\[', 'passValues[', text)
        text = re.sub(r'\bo(\d)\b', r'output.targets[\1]', text)
        text = text.replace('vCoverage.x', '1.0')
        if not destination:
            text = re.sub(r'\.([xyzw])$', lambda m: '.' + m[1] * 4, text)
    if absolute:
        text = 'abs(' + text + ')'
    return '-(' + text + ')' if negative else text


def uint_operand(text):
    return immediate(text.strip(), True) if text.strip().startswith('l(') else 'asuint(' + operand(text) + ')'


def result_mask(destination, value):
    match = re.search(r'\.([xyzw]+)$', destination)
    return operand(destination, True) + ' = (' + value + ').' + (match.group(1) if match else 'xyzw') + ';'


def translate(instruction, texture_map, lookup, stage):
    op, _, tail = instruction.partition(' ')
    a = split_arguments(tail)
    saturate = op.endswith('_sat')
    op = op.removesuffix('_sat')
    if op in ('if_nz', 'if_z'):
        return 'if ((' + uint_operand(a[0]) + ').x' + (' != 0u)' if op == 'if_nz' else ' == 0u)') + ' {'
    if op == 'else':
        return '} else {'
    if op == 'endif':
        return '}'
    if op == 'ret':
        return 'return output;'
    if op == 'discard_nz':
        return 'if ((' + uint_operand(a[0]) + ').x != 0u) { output.discarded = true; return output; }'
    if a and a[0].startswith('oMask'):
        return '// Coverage is owned by the product rasterizer.'
    if op.startswith('sample'):
        destination, uv, texture = a[:3]
        match = re.match(r't(\d+)\.([xyzw]+)', texture)
        register, swizzle = int(match[1]), match[2]
        if register in texture_map:
            index = texture_map[register]
            sampler = 'SourceCharacterLookupSampler' if index in lookup else 'SourceCharacterSampler'
            sample = f'g_SourceCharacterTexture{index}'
            if 'sample_l' in op:
                sample += f'.SampleLevel({sampler}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            elif 'sample_b' in op:
                sample += f'.SampleBias({sampler}, ({operand(uv)}).xy, ({operand(a[4])}).x)'
            else:
                sample += f'.Sample({sampler}, ({operand(uv)}).xy)'
        elif 'texturecube' in op:
            sample = 'float4(0.0,0.0,0.0,0.0)'
        else:
            sample = 'float4(sqrt(saturate(input.shadow)).xxx,1.0)' if stage == 'light' else 'float4(0.0,0.0,0.0,0.0)'
        return result_mask(destination, '(' + sample + ').' + swizzle)
    source = [operand(x) for x in a[1:]]
    if op == 'mov':
        value = source[0]
    elif op in ('add', 'mul', 'div'):
        value = '(' + source[0] + ')' + {'add': '+', 'mul': '*', 'div': '/'}[op] + '(' + source[1] + ')'
    elif op == 'mad':
        value = '(' + source[0] + ')*(' + source[1] + ')+(' + source[2] + ')'
    elif op in ('dp2', 'dp3', 'dp4'):
        mask = {'dp2': 'xy', 'dp3': 'xyz', 'dp4': 'xyzw'}[op]
        value = 'dot((' + source[0] + ').' + mask + ',(' + source[1] + ').' + mask + ').xxxx'
    elif op in ('min', 'max'):
        value = op + '(' + ','.join(source) + ')'
    elif op == 'rcp':
        value = '1.0/(' + source[0] + ')'
    elif op in ('rsq', 'sqrt', 'log', 'exp', 'frc', 'round_ni', 'round_pi'):
        value = {'rsq': 'rsqrt', 'sqrt': 'sqrt', 'log': 'log2', 'exp': 'exp2', 'frc': 'frac', 'round_ni': 'floor', 'round_pi': 'ceil'}[op] + '(' + source[0] + ')'
    elif op in ('lt', 'ge', 'ne', 'eq'):
        value = 'asfloat((uint4)((' + source[0] + ')' + {'lt': '<', 'ge': '>=', 'ne': '!=', 'eq': '=='}[op] + '(' + source[1] + ')) * 0xffffffffu)'
    elif op == 'movc':
        value = '(' + uint_operand(a[1]) + ' != 0u) ? (' + source[1] + ') : (' + source[2] + ')'
    elif op in ('and', 'or', 'xor'):
        value = 'asfloat(' + uint_operand(a[1]) + {'and': ' & ', 'or': ' | ', 'xor': ' ^ '}[op] + uint_operand(a[2]) + ')'
    elif op == 'not':
        value = 'asfloat(~' + uint_operand(a[1]) + ')'
    elif op == 'ftou':
        value = 'asfloat((uint4)(' + source[0] + '))'
    elif op == 'utof':
        value = '(float4)(' + uint_operand(a[1]) + ')'
    elif op == 'itof':
        value = '(float4)(asint(' + source[0] + '))'
    elif op == 'iadd':
        value = 'asfloat(' + uint_operand(a[1]) + ' + ' + uint_operand(a[2]) + ')'
    elif op == 'bfi':
        value = 'SourceCharacterBitInsert(' + ','.join(uint_operand(x) for x in a[1:]) + ')'
    elif op in ('ishl', 'ushr'):
        value = 'asfloat(' + uint_operand(a[1]) + (' << ' if op == 'ishl' else ' >> ') + '(' + uint_operand(a[2]) + ' & 31u))'
    elif op in ('deriv_rtx_coarse', 'deriv_rty_coarse'):
        value = ('ddx_coarse' if 'rtx' in op else 'ddy_coarse') + '(' + source[0] + ')'
    elif op == 'sincos':
        parts = []
        if a[0] != 'null':
            parts.append(result_mask(a[0], 'sin(' + operand(a[2]) + ')'))
        if a[1] != 'null':
            parts.append(result_mask(a[1], 'cos(' + operand(a[2]) + ')'))
        return ' '.join(parts)
    else:
        fail(f'unsupported DXBC instruction: {instruction}')
    if saturate:
        value = 'saturate(' + value + ')'
    return result_mask(a[0], value)


def g9(value):
    return format(float(value), '.9g')


def uses_time(node):
    if not isinstance(node, dict):
        return False
    if node['typeName'] == 'fmaterialuniformexpressiontime':
        return True
    return any(uses_time(v) for v in node.values() if isinstance(v, dict))


def parameter_names(node, found):
    if isinstance(node, dict):
        if node['typeName'] in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
            found.add(node['parameterName'])
        for value in node.values():
            parameter_names(value, found)
    return found


FOLD = {0: ('+', 'add'), 1: ('-', 'subtract'), 2: ('*', 'multiply')}


def hlsl(node, time_parameter):
    kind = node['typeName']
    if kind == 'fmaterialuniformexpressiontime':
        return 'g_SourceCharacterTime.xxxx'
    if kind == 'fmaterialuniformexpressionconstant':
        return 'float4(' + ','.join(g9(v) for v in node['value']) + ')'
    if kind in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
        if node['parameterName'] != time_parameter:
            fail(f'shader-side parameter {node["parameterName"]} has no reserved row')
        return f'source[{TIME_PARAMETER_ROW}].xxxx'
    if kind == 'fmaterialuniformexpressionfoldedmath':
        return '(' + hlsl(node['a'], time_parameter) + FOLD[node['operationOrdinal']][0] + hlsl(node['b'], time_parameter) + ')'
    if kind == 'fmaterialuniformexpressionsine':
        return ('cos(' if node['isCosine'] else 'sin(') + hlsl(node['input'], time_parameter) + ')'
    if kind == 'fmaterialuniformexpressionappendvector':
        return 'SourceCharacterAppend(' + hlsl(node['a'], time_parameter) + ',' + hlsl(node['b'], time_parameter) + ',' + str(node['componentsFromA']) + 'u)'
    fail(f'unsupported uniform expression {kind}')


def cpp_float(value):
    text = g9(value)
    return (text if '.' in text or 'e' in text else text + '.') + 'f'


def cpp(node):
    kind = node['typeName']
    if kind == 'fmaterialuniformexpressiontime':
        return 'Value{}'
    if kind == 'fmaterialuniformexpressionconstant':
        return 'Value{' + ','.join(cpp_float(v) for v in node['value']) + '}'
    if kind in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
        return 'parameter("' + node['parameterName'] + '")'
    if kind == 'fmaterialuniformexpressionfoldedmath':
        return FOLD[node['operationOrdinal']][1] + '(' + cpp(node['a']) + ',' + cpp(node['b']) + ')'
    if kind == 'fmaterialuniformexpressionsine':
        return 'wave(' + cpp(node['input']) + ',' + ('true' if node['isCosine'] else 'false') + ')'
    if kind == 'fmaterialuniformexpressionappendvector':
        return 'append(' + cpp(node['a']) + ',' + cpp(node['b']) + ',' + str(node['componentsFromA']) + 'u)'
    fail(f'unsupported uniform expression {kind}')


def stage_rows(document, program):
    expressions = document['uniformExpressionSet']
    rows = [(b['baseIndex'] // 16, None, expressions['pixelVectorExpressions'][b['expressionIndexOrGroup']])
            for b in program['bindings']['vectors']]
    scalars = expressions['pixelScalarExpressions']
    for binding in program['bindings']['scalarGroups']:
        for lane in range(4):
            index = binding['expressionIndexOrGroup'] * 4 + lane
            rows.append((binding['baseIndex'] // 16, lane, scalars[index] if index < len(scalars) else None))
    return sorted(rows, key=lambda r: (r[0], -1 if r[1] is None else r[1]))


def time_parameter_of(document):
    names = set()
    for stage in (BASE_TYPE, LIGHT_TYPE):
        for _row, _lane, node in stage_rows(document, document['programs'][stage]):
            if node is not None and uses_time(node):
                parameter_names(node, names)
    if len(names) > 1:
        fail(f'time expressions need more than one reserved row: {sorted(names)}')
    return next(iter(names), None)


def texture_map_of(document, program):
    expressions = document['uniformExpressionSet']['pixelTexture2DExpressions']
    mapping, lookup, mask = {}, set(), 0
    for binding in program['bindings']['textures']:
        index = binding['expressionIndexOrGroup']
        mapping[binding['baseIndex']] = index
        mask |= 1 << index
        if expressions[index].get('parameterName') in LOOKUP_TEXTURES:
            lookup.add(index)
    return mapping, lookup, mask


def emit_function(document, family, number, stage):
    program = document['programs'][BASE_TYPE if stage == 'base' else LIGHT_TYPE]
    time_parameter = time_parameter_of(document)
    texture_map, lookup, _mask = texture_map_of(document, program)
    constants = 'g_SourceCharacterBaseConstants' if stage == 'base' else 'g_SourceCharacterLightConstants'
    name = ('SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight') + str(number)
    lines = [f'// {family} / source program {program["shaderId"]}',
             f'SOURCE_CHARACTER_NATIVE_OUTPUT {name}(SOURCE_CHARACTER_NATIVE_INPUT input)', '{',
             '    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;',
             '    float4 source[64];',
             f'    [unroll] for (uint i=0u;i<64u;++i) source[i]={constants}[i];']
    for row, lane, node in stage_rows(document, program):
        if node is None or not uses_time(node):
            continue
        if lane is None:
            lines.append(f'    source[{row}]={hlsl(node, time_parameter)};')
        else:
            lines.append(f'    source[{row}].{"xyzw"[lane]}=({hlsl(node, time_parameter)}).x;')
    if stage == 'light':
        trailing = program['bindings']['constantBufferClosure']['trailingUnownedConstantBuffer0Slots']
        lines.append(f'    source[{trailing[0]}]=float4(input.lightColor,1.0);')
        lines.append(f'    source[{trailing[1]}].x=1.0;')
    lines.append('    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];')
    lines.append('    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};')
    lines.append('    float4 ' + ', '.join(f'v{i} = input.values[{i}]' for i in range(10)) + ';')
    temps = next(int(d.split()[1]) for d in program['disassembly']['declarations'] if d.startswith('dcl_temps'))
    lines.append('    float4 ' + ', '.join(f'r{i}=0.0' for i in range(temps)) + ';')
    for index, instruction in enumerate(program['disassembly']['instructions'], 1):
        lines.append(f'    // {index}: {instruction}')
        lines.append('    ' + translate(instruction, texture_map, lookup, stage))
    lines.append('}')
    return '\n'.join(lines) + '\n'


def emit_configure(document, family, number):
    time_parameter = time_parameter_of(document)
    lines = [f'    else if (family == "{family}")', '    {', f'        staged.program = {number}u;']
    if time_parameter is not None:
        lines.append(f'        staged.baseConstants[{TIME_PARAMETER_ROW}] = vector(parameter("{time_parameter}"));')
        lines.append(f'        staged.lightConstants[{TIME_PARAMETER_ROW}] = vector(parameter("{time_parameter}"));')
    for stage, program_type in (('base', BASE_TYPE), ('light', LIGHT_TYPE)):
        program = document['programs'][program_type]
        lines.append(f'        staged.{stage}TextureMask = {texture_map_of(document, program)[2]}u;')
        grouped = {}
        for row, lane, node in stage_rows(document, program):
            grouped.setdefault(row, []).append((lane, node))
        for row in sorted(grouped):
            entries = grouped[row]
            if entries[0][0] is None:
                lines.append(f'        staged.{stage}Constants[{row}] = vector({cpp(entries[0][1])});')
            else:
                lanes = [(cpp(node) + '[0]') if node is not None else '0.f' for _lane, node in entries]
                lines.append(f'        staged.{stage}Constants[{row}] = float4_t({",".join(lanes)});')
    lines.append('    }')
    return '\n'.join(lines) + '\n'


def read_text(path):
    return path.read_bytes().decode('utf8').replace('\r\n', '\n')


def installed_function(path, name):
    lines = read_text(path).split('\n')
    start = next((i for i, l in enumerate(lines) if l.startswith(f'SOURCE_CHARACTER_NATIVE_OUTPUT {name}(')), None)
    if start is None:
        return None
    head = start - 1 if lines[start - 1].startswith('// source.') else start
    end = next(i for i in range(start, len(lines)) if lines[i] == '}')
    return '\n'.join(lines[head:end + 1]) + '\n'


def installed_configure(family):
    lines = read_text(PARAMETER_HEADER).split('\n')
    start = next((i for i, l in enumerate(lines) if f'family == "{family}"' in l), None)
    if start is None:
        return None
    end = next(i for i in range(start + 2, len(lines)) if lines[i] == '    }')
    body = [l for l in lines[start:end + 1] if not l.strip().startswith('//')]
    return '\n'.join(body) + '\n'


def load_document(path):
    return json.loads(pathlib.Path(path).read_text(encoding='utf8'))


def command_verify(arguments):
    document = load_document(arguments.dump)
    failures = 0
    for stage, path in (('base', BASE_PROGRAMS), ('light', LIGHT_PROGRAMS)):
        name = ('SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight') + str(arguments.program)
        installed = installed_function(path, name)
        generated = emit_function(document, arguments.family, arguments.program, stage)
        exact = installed == generated
        failures += 0 if exact else 1
        print(stage, 'EXACT' if exact else 'MISMATCH', len(generated.splitlines()), 'lines')
    installed = installed_configure(arguments.family)
    generated = emit_configure(document, arguments.family, arguments.program)
    reserved = f'staged.baseConstants[{TIME_PARAMETER_ROW}]', f'staged.lightConstants[{TIME_PARAMETER_ROW}]'
    comparable = ''.join(l + '\n' for l in generated.splitlines() if not l.strip().startswith(reserved))
    exact = installed is not None and installed.replace('    if (family', '    else if (family') in (generated, comparable)
    failures += 0 if exact else 1
    print('configure', 'EXACT' if exact else 'MISMATCH')
    if failures:
        fail(f'reference program {arguments.program} was not reproduced')


def command_generate(arguments):
    document = load_document(arguments.dump)
    arguments.out.mkdir(parents=True, exist_ok=True)
    (arguments.out / f'base{arguments.program}.hlsli').write_text(emit_function(document, arguments.family, arguments.program, 'base'), encoding='utf8')
    (arguments.out / f'light{arguments.program}.hlsli').write_text(emit_function(document, arguments.family, arguments.program, 'light'), encoding='utf8')
    (arguments.out / f'configure{arguments.program}.h').write_text(emit_configure(document, arguments.family, arguments.program), encoding='utf8')
    print('generated', arguments.program, arguments.family)


def write_preserving_newlines(path, text):
    before = path.read_bytes()
    newline = '\r\n' if b'\r\n' in before else '\n'
    after = text.replace('\r\n', '\n').replace('\n', newline).encode('utf8')
    if before != after:
        path.write_bytes(after)


def install_program(path, function, number, stage):
    text = read_text(path)
    prefix = 'SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight'
    name = f'{prefix}{number}'
    existing = installed_function(path, name)
    if existing is not None:
        if existing != function:
            fail(f'{path.name} already holds a different {name}')
        return text
    evaluate = f'\n\n\nSOURCE_CHARACTER_NATIVE_OUTPUT Evaluate{prefix}(SOURCE_CHARACTER_NATIVE_INPUT input)'
    last_case = '    case 84u: return ' + prefix + '84(input);\n'
    if text.count(evaluate) != 1 or text.count(last_case) != 1 or f'case {number}u:' in text:
        fail(f'{path.name} dispatch anchors changed')
    text = text.replace(evaluate, '\n\n' + function.rstrip('\n') + evaluate)
    return text.replace(last_case, last_case + f'    case {number}u: return {name}(input);\n')


def command_install(arguments):
    base = (arguments.generated / f'base{arguments.program}.hlsli').read_text(encoding='utf8')
    light = (arguments.generated / f'light{arguments.program}.hlsli').read_text(encoding='utf8')
    configure = (arguments.generated / f'configure{arguments.program}.h').read_text(encoding='utf8')
    for path, function, stage in ((BASE_PROGRAMS, base, 'base'), (LIGHT_PROGRAMS, light, 'light')):
        write_preserving_newlines(path, install_program(path, function, arguments.program, stage))
        shutil.copyfile(path, CLIENT_SHADER_DIR / path.name)
    header = read_text(PARAMETER_HEADER)
    existing = installed_configure(arguments.family)
    if existing is None:
        anchor = '    else return false;\n    if (staged.program == 80u)\n'
        if header.count(anchor) != 1:
            fail('SourceCharacterMaterialParameters.h family anchor changed')
        header = header.replace(anchor, configure + anchor)
        write_preserving_newlines(PARAMETER_HEADER, header)
    elif existing != configure:
        fail(f'{arguments.family} is already installed with a different packing')
    print('installed program', arguments.program)


def command_rows(arguments):
    resources = json.loads(arguments.texture_map.read_text(encoding='utf8'))
    rows = []
    for spec in arguments.entry:
        dump_path, family, *generated = spec.split('=')
        document = load_document(dump_path)
        block = pathlib.Path(generated[0]).read_text(encoding='utf8') if generated else installed_configure(family)
        if block is None:
            fail(f'{family} is not installed')
        names = sorted(set(re.findall(r'parameter\("([^"]+)"\)', block)))
        defaults = {}

        def collect(node):
            if isinstance(node, dict):
                if node.get('typeName') in ('fmaterialuniformexpressionscalarparameter', 'fmaterialuniformexpressionvectorparameter'):
                    defaults.setdefault(node['parameterName'], node['defaultValue'])
                for value in node.values():
                    collect(value)
            elif isinstance(node, list):
                for value in node:
                    collect(value)
        collect(document['uniformExpressionSet'])
        parameters = {}
        for name in names:
            if name in document['vectors']:
                value = document['vectors'][name]
                parameters[name] = [float(value[k]) for k in ('r', 'g', 'b', 'a')] if isinstance(value, dict) else [float(v) for v in value]
            elif name in document['scalars']:
                parameters[name] = [float(document['scalars'][name])] * 4
            elif name in defaults:
                value = defaults[name]
                parameters[name] = [float(v) for v in value] if isinstance(value, list) else [float(value)] * 4
            else:
                fail(f'{document["sourceMaterial"]} parameter {name} has neither override nor default')
        mask = 0
        for value in re.findall(r'staged\.(?:base|light)TextureMask = (\d+)u', block):
            mask |= int(value)
        textures = []
        for texture in document['textures']:
            if not mask & (1 << texture['expressionIndex']):
                continue
            leaf = texture['sourceObject'].split('.')[-1].lower()
            if leaf not in resources:
                fail(f'{texture["sourceObject"]} has no Resources mapping')
            textures.append(dict(expressionIndex=texture['expressionIndex'], assetId=resources[leaf],
                                 colorSpace='srgb' if texture['srgb'] else 'linear'))
        if sum(1 << t['expressionIndex'] for t in textures) != mask:
            fail(f'{document["sourceMaterial"]} does not supply every required texture')
        material_name = document['sourceMaterial'].partition('.mat.')[2]
        rows.append(dict(modelAssetId=arguments.model, materialName=material_name,
                         sourceMaterial=document['sourceMaterial'], family=family,
                         parameters=parameters, textures=textures))
    arguments.out.write_text(json.dumps(rows, indent=2) + '\n', encoding='utf8')
    print('rows', len(rows), '->', arguments.out)


def command_textures(arguments):
    resources = json.loads(arguments.texture_map.read_text(encoding='utf8'))
    for leaf, asset_id in sorted(resources.items()):
        destination = RESOURCES / asset_id
        if asset_id.startswith('Character/SourceMaterials/'):
            if not destination.is_file():
                fail(f'shared source material is missing: {asset_id}')
            continue
        candidates = [p for p in arguments.staging.rglob(leaf + pathlib.Path(asset_id).suffix)]
        if len(candidates) != 1:
            fail(f'{leaf} must exist exactly once under the staging tree, found {len(candidates)}')
        destination.parent.mkdir(parents=True, exist_ok=True)
        if destination.is_file() and destination.read_bytes() != candidates[0].read_bytes():
            fail(f'{asset_id} already exists with different bytes')
        if not destination.is_file():
            shutil.copyfile(candidates[0], destination)
        print('texture', asset_id)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest='command', required=True)
    extract = commands.add_parser('extract')
    extract.add_argument('--umodel', type=pathlib.Path, required=True)
    extract.add_argument('--d3dcompiler', type=pathlib.Path, required=True)
    extract.add_argument('--out', type=pathlib.Path, required=True)
    extract.add_argument('materials', nargs='+')
    for name in ('verify', 'generate'):
        sub = commands.add_parser(name)
        sub.add_argument('--dump', type=pathlib.Path, required=True)
        sub.add_argument('--family', required=True)
        sub.add_argument('--program', type=int, required=True)
        if name == 'generate':
            sub.add_argument('--out', type=pathlib.Path, required=True)
    install = commands.add_parser('install')
    install.add_argument('--generated', type=pathlib.Path, required=True)
    install.add_argument('--family', required=True)
    install.add_argument('--program', type=int, required=True)
    rows = commands.add_parser('rows')
    rows.add_argument('--model', required=True)
    rows.add_argument('--texture-map', type=pathlib.Path, required=True)
    rows.add_argument('--out', type=pathlib.Path, required=True)
    rows.add_argument('entry', nargs='+')
    textures = commands.add_parser('textures')
    textures.add_argument('--texture-map', type=pathlib.Path, required=True)
    textures.add_argument('--staging', type=pathlib.Path, required=True)
    arguments = parser.parse_args()
    {'extract': command_extract, 'verify': command_verify, 'generate': command_generate,
     'install': command_install, 'rows': command_rows, 'textures': command_textures}[arguments.command](arguments)


if __name__ == '__main__':
    main()
```

### `Tools/VehiclePipeline/Terpeion.texture-map.json` 전체 코드

```json
{
  "mn_pmstg_00_n": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_n.tga",
  "mn_pmstg_00_s": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_s.tga",
  "mn_pmstg_00_e": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_e.tga",
  "mn_pmstg_00_cm": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_cm.tga",
  "mn_pmstg_00-1_n": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_n.tga",
  "mn_pmstg_00-1_s": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_s.tga",
  "mn_pmstg_00-1_cm": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_cm.tga",
  "mn_pmstg_01_d": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01_d.tga",
  "mn_pmstg_01-1_d": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01-1_d.tga",
  "mn_pmstg_01_hair_d": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01_hair_d.tga",
  "mn_pmshs_00_hair_n": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmshs_00_hair_n.tga",
  "hdr07_1": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga",
  "statefx_default": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga",
  "brdf_beckmann_spec": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds"
}
```

색 공간은 원본 Texture2D의 `SRGB` tagged property를 읽는다. `_n`과 BRDF LUT만 linear다.

---

## G02. 탑승자 애니셋 쿠킹

에스더 캐스팅 애니셋(08-24)과 같은 레시피다. 본 해시가 몸체와 같아야 `Attach_AnimationSet`이 통과한다.

| 직업 | 가족 패키지 / psa | rig psk | `--armature-name` | `--carrier-name` | 출력 |
|---|---|---|---|---|---|
| 창술사 | `PC_FT_00` / `pc_ft_00_ani` | `PC_FLM_00` `pc_flm_00_upper_sk_loc_int` | `flm` | `pc_ft_00_sk.001` | `Character/LanceMaster/AnimSets/LanceMaster_RideHorseAnimSet.wmodel` |
| 워로드 | `PC_WR_00` / `pc_wr_00_ani` | `PC_WR_00` `pc_wr_00_sk` | `wgl` | `pc_wr_00_sk.001` | `Character/Warlord/AnimSets/Warlord_RideHorseAnimSet.wmodel` |
| 도화가 | `PC_SP_00` / `pc_sp_00_ani` | `PC_SP_00` `pc_sp_00_sk` | `sdm` | `pc_sp_00_sk.001` | `Character/Artist/AnimSets/Artist_RideHorseAnimSet.wmodel` |
| 차원술사 | `PC_SP_M_00` / `pc_sp_m_00_ani` | `PC_SP_M_00` `pc_sp_m_00_sk` | `pc_sp_m_00_sk` | `pc_sp_m_00_sk.001` | `Character/DimensionMaster/AnimSets/DimensionMaster_RideHorseAnimSet.wmodel` |

```powershell
$umodel = 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe'
$game = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame'
$stage = 'out/VehicleTerpeion20260913/riders'
& $umodel -export -psk -uncook -game=lostark -kr -nameresolve "-path=$game" "-out=$stage" -obj=pc_ft_00_ani PC_FT_00
& $umodel -export -psk -uncook -game=lostark -kr -nameresolve "-path=$game" "-out=$stage" -obj=pc_flm_00_upper_sk_loc_int PC_FLM_00
# 나머지 3직업도 위 표의 패키지/오브젝트로 같은 명령

$blender = 'C:\Program Files\Blender Foundation\Blender 5.0\blender.exe'
& $blender --background --factory-startup --python C:\Users\95jus\Desktop\buildScript\build_npc_animset.py -- `
  "$stage\PC_FLM_00\...\pc_flm_00_upper_sk_loc_int.psk" "$stage\LanceMaster_RideHorse.fbx" `
  "$stage\PC_FT_00\AnimSet\pc_ft_00_ani.psa" `
  --armature-name flm --carrier-name pc_ft_00_sk.001 `
  --clips ride_horse_idle_normal_1,ride_horse_run_normal_1
& .\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe "$stage\LanceMaster_RideHorse.fbx" `
  -o Client/Bin/Resources/Character/LanceMaster/AnimSets/LanceMaster_RideHorseAnimSet.wmodel --no-auto-textures
```

`--clips`에 없는 클립이 psa에 없으면 스크립트가 실패한다. 이 실패가 곧 "이 가족 psa에 ride_horse가 없다"는 판정이므로
그때는 같은 패키지의 `_bk` animset을 `-list`로 확인한 뒤 교체한다(추측으로 다른 클립을 쓰지 않는다).

쿠킹 클립명은 `<armature>_ride_horse_idle_normal_1`이며 G06 `riders[]`가 이 이름을 그대로 쓴다.

### `Data/Actors/CharacterCatalog.json`

각 직업 `animationSetModels` 배열 마지막 항목 뒤에 추가한다.

```json
        "Character/LanceMaster/AnimSets/LanceMaster_RideHorseAnimSet.wmodel"
```
```json
        "Character/Artist/AnimSets/Artist_RideHorseAnimSet.wmodel"
```
```json
        "Character/DimensionMaster/AnimSets/DimensionMaster_RideHorseAnimSet.wmodel"
```
```json
        "Character/Warlord/AnimSets/Warlord_RideHorseAnimSet.wmodel"
```

기존 `…_CustomizingAnimSet.wmodel"` 줄 끝에 `,`를 붙인다. `CPlayableCharacterAssetService`가 선언 순서대로 부착하며
실패하면 그 직업 admission 전체가 실패한다(기존 fail-closed 계약).

검증: 4개 wmodel `validate_wmodel.py` OK, `compare_attach.py`로 본 이름·순서·skeletonHash가 몸체와 일치.

---

## G03. Shared protocol 82

### `Shared/Public/Network/NetworkIds.h`

기준점: `inline constexpr COMBAT_OBJECT_ID INVALID_COMBAT_OBJECT_ID = 0;` 바로 아래, 닫는 `}` 위에 추가.

```cpp

	// EFTable_Vehicle primary key. The Server admits only ids its published
	// vehicle bootstrap carries; zero means the player is on foot.
	using VEHICLE_ID =
		std::uint32_t;

	inline constexpr VEHICLE_ID
		INVALID_VEHICLE_ID = 0;
```

### `Shared/Public/Network/PacketType.h`

기준점: 버전 주석의 `81 adds processed MOVE sequence, … Both peers need 81. */` 줄과 그 다음 상수 줄을 교체.

```cpp
	81 adds processed MOVE sequence, effective speed, prediction permission and
	the next authoritative waypoint to player snapshots.
	82 appends the ridden vehicle to player snapshots and the riding toggle
	request/verdict. Both peers need 82. */
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 82;
```

기준점: `PACKET_TYPE` 마지막 항목 `C2S_DEBUG_BINGO_HAMMER` 교체.

```cpp
		C2S_DEBUG_BINGO_HAMMER,
		// H key riding toggle and its typed verdict. The snapshot carries the
		// ridden vehicle, so the verdict only reports why a request did nothing.
		C2S_SET_VEHICLE_RIDING,
		S2C_SET_VEHICLE_RIDING_RESULT
```

기준점: `Is_Known_Packet_Type`의 `case PACKET_TYPE::C2S_MARIO_MOVE:` 바로 아래 추가.

```cpp
		case PACKET_TYPE::C2S_SET_VEHICLE_RIDING:
		case PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT:
```

### `Shared/Public/Network/PacketMessages.h`

기준점: `S2C_DEBUG_SET_MADNESS_FORM_RESULT& message);`(Read 선언) 바로 아래, `MECHANIC_CARD_SYMBOL` 주석 위에 추가.

```cpp

	/* H key riding toggle. A vehicle id mounts that vehicle and
	INVALID_VEHICLE_ID dismounts. The Server owns the verdict and the snapshot
	presents the result. */
	struct C2S_SET_VEHICLE_RIDING
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		VEHICLE_ID iVehicleId = INVALID_VEHICLE_ID;
	};

	enum class VEHICLE_RIDING_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_WORLD_NOT_ALLOWED,
		REJECTED_UNKNOWN_VEHICLE,
		REJECTED_PLAYER_STATE,
		REJECTED_SAME_STATE,
		END
	};

	struct S2C_SET_VEHICLE_RIDING_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		VEHICLE_RIDING_RESULT eResult = VEHICLE_RIDING_RESULT::REJECTED_SESSION;
		// The vehicle the player rides after this request, accepted or not.
		VEHICLE_ID iActiveVehicleId = INVALID_VEHICLE_ID;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_SET_VEHICLE_RIDING& message);
	bool Read_Message(CPacketReader& reader,
		C2S_SET_VEHICLE_RIDING& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_SET_VEHICLE_RIDING_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_SET_VEHICLE_RIDING_RESULT& message);
```

기준점: `PLAYER_SNAPSHOT`의 `PLAYER_MADNESS_FORM eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;` 바로 아래 추가.

```cpp
		/* The vehicle the Server has this player riding, or INVALID_VEHICLE_ID.
		A ridden vehicle implies a living, idle, normal-form player outside Mario
		and pattern bind; the Server dismounts before any other action is sent. */
		VEHICLE_ID iVehicleId = INVALID_VEHICLE_ID;
```

### `Shared/Private/Network/PacketMessages.cpp`

기준점 1: `Is_Valid_PlayerSnapshot`의 `Is_Valid_PlayerMadnessForm(snapshot.eMadnessForm) &&` 바로 아래 추가.

```cpp
			(LostArk::Shared::INVALID_VEHICLE_ID == snapshot.iVehicleId ||
			 (snapshot.iCurrentHp != 0u &&
			  LostArk::Shared::PLAYER_ACTION_STATE::NONE == snapshot.eAction &&
			  LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL == snapshot.eMadnessForm &&
			  !snapshot.isPatternBound && snapshot.iMarioStage == 0u)) &&
```

기준점 2: `Read_Message(CPacketReader& reader, S2C_DEBUG_SET_MADNESS_FORM_RESULT& message)` 정의의 닫는 `}` 바로 아래 추가.

```cpp

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const C2S_SET_VEHICLE_RIDING& message)
{
	if (0u == message.iRequestSequence || !Is_Known_World_Id(message.eWorldId))
		return false;
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U32(message.iVehicleId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, C2S_SET_VEHICLE_RIDING& message)
{
	C2S_SET_VEHICLE_RIDING decoded{};
	std::uint16_t world = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) || !reader.Read_U16(world) ||
		!reader.Read_U32(decoded.iVehicleId))
		return false;
	decoded.eWorldId = static_cast<WORLD_ID>(world);
	if (0u == decoded.iRequestSequence || !Is_Known_World_Id(decoded.eWorldId))
		return false;
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const S2C_SET_VEHICLE_RIDING_RESULT& message)
{
	if (0u == message.iRequestSequence || !Is_Known_World_Id(message.eWorldId) ||
		message.eResult >= VEHICLE_RIDING_RESULT::END)
		return false;
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	writer.Write_U32(message.iActiveVehicleId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, S2C_SET_VEHICLE_RIDING_RESULT& message)
{
	S2C_SET_VEHICLE_RIDING_RESULT decoded{};
	std::uint16_t world = 0u;
	std::uint8_t result = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) || !reader.Read_U16(world) ||
		!reader.Read_U8(result) || !reader.Read_U32(decoded.iActiveVehicleId))
		return false;
	decoded.eWorldId = static_cast<WORLD_ID>(world);
	decoded.eResult = static_cast<VEHICLE_RIDING_RESULT>(result);
	if (0u == decoded.iRequestSequence || !Is_Known_World_Id(decoded.eWorldId) ||
		decoded.eResult >= VEHICLE_RIDING_RESULT::END)
		return false;
	message = decoded;
	return true;
}
```

기준점 3: `Write_Message(…S2C_WORLD_SNAPSHOT…)`의 player 루프 마지막 `writer.Write_F32(player.fMoveWaypointZ);` 바로 아래 추가.

```cpp
		writer.Write_U32(player.iVehicleId);
```

기준점 4: `Read_Message(…S2C_WORLD_SNAPSHOT…)`의 `!reader.Read_F32(player.fMoveWaypointZ))` 교체.

```cpp
			!reader.Read_F32(player.fMoveWaypointZ) ||
			!reader.Read_U32(player.iVehicleId))
```

### `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`

1. 버전 고정 7곳(`NETWORK_PROTOCOL_VERSION == 81u` 5곳, `81u == NETWORK_PROTOCOL_VERSION` 2곳)의 81u를 82u로, 같은 문장의
   "Protocol 81"/"protocol 81"을 82로 바꾼다(현재 2285, 2712, 2780, 3133, 3250, 6583, 6611행).
   World snapshot 크기 검사의 `playerPredictionBytes` 아래에 `playerVehicleBytes = 4`를 추가하고 `playerFixedBytes` 합에 더한다.
2. 기준점 `void Test_DebugMadnessFormProtocol(TEST_RUNNER& testRunner)` 함수의 닫는 `}` 바로 아래 새 테스트 추가.

```cpp

	void Test_VehicleRidingProtocol(TEST_RUNNER& testRunner)
	{
		C2S_SET_VEHICLE_RIDING request{};
		request.iRequestSequence = 21u;
		request.eWorldId = WORLD_ID::BERN;
		request.iVehicleId = 6705u;
		CPacketWriter writer;
		testRunner.Require(Write_Message(writer, request) && writer.Get_Buffer().size() == 10u,
			"Riding toggle carries sequence, world and vehicle id");
		CPacketReader reader{ writer.Get_Buffer() };
		C2S_SET_VEHICLE_RIDING decoded{};
		testRunner.Require(Read_Message(reader, decoded) && decoded.iRequestSequence == 21u &&
			decoded.eWorldId == WORLD_ID::BERN && decoded.iVehicleId == 6705u &&
			0u == reader.Get_RemainingSize(),
			"Riding toggle round trip");
		request.iVehicleId = INVALID_VEHICLE_ID;
		CPacketWriter dismount;
		testRunner.Require(Write_Message(dismount, request),
			"Dismount is the same request with the invalid vehicle id");
		request.iRequestSequence = 0u;
		CPacketWriter zeroSequence;
		testRunner.Require(!Write_Message(zeroSequence, request) && zeroSequence.Get_Buffer().empty(),
			"Riding toggle refuses the reserved zero sequence");
		for (std::uint8_t reason = 0u;
			reason < static_cast<std::uint8_t>(VEHICLE_RIDING_RESULT::END); ++reason)
		{
			S2C_SET_VEHICLE_RIDING_RESULT result{};
			result.iRequestSequence = 21u;
			result.eWorldId = WORLD_ID::BERN;
			result.eResult = static_cast<VEHICLE_RIDING_RESULT>(reason);
			result.iActiveVehicleId = 6705u;
			CPacketWriter resultWriter;
			testRunner.Require(Write_Message(resultWriter, result) &&
				11u == resultWriter.Get_Buffer().size(),
				"Riding verdict writer accepts every typed reason");
			CPacketReader resultReader{ resultWriter.Get_Buffer() };
			S2C_SET_VEHICLE_RIDING_RESULT read{};
			testRunner.Require(Read_Message(resultReader, read) &&
				read.eResult == result.eResult && read.iActiveVehicleId == 6705u &&
				0u == resultReader.Get_RemainingSize(),
				"Riding verdict echoes correlation, reason and active vehicle");
		}
		auto unknownReason = CPacketWriter{};
		unknownReason.Write_U32(21u);
		unknownReason.Write_U16(static_cast<std::uint16_t>(WORLD_ID::BERN));
		unknownReason.Write_U8(static_cast<std::uint8_t>(VEHICLE_RIDING_RESULT::END));
		unknownReason.Write_U32(0u);
		CPacketReader unknownReader{ unknownReason.Get_Buffer() };
		S2C_SET_VEHICLE_RIDING_RESULT preserved{};
		preserved.iRequestSequence = 99u;
		testRunner.Require(!Read_Message(unknownReader, preserved) && preserved.iRequestSequence == 99u,
			"Riding verdict refuses an unknown reason and preserves caller output");

		S2C_WORLD_SNAPSHOT source{};
		source.iServerTick = 100u;
		source.eWorldId = WORLD_ID::BERN;
		source.ActiveGameplayRevision = Make_GameplayDataRevision(1u);
		PLAYER_SNAPSHOT rider{};
		rider.iNetEntityId = 100u;
		rider.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		rider.fMoveSpeed = 5.f;
		rider.iVehicleId = 6705u;
		source.Players.push_back(rider);
		std::vector<std::uint8_t> payload;
		const bool written = Build_WorldSnapshotPayload(source, payload);
		CPacketReader snapshotReader{ payload };
		S2C_WORLD_SNAPSHOT decodedSnapshot{};
		testRunner.Require(written && Read_Message(snapshotReader, decodedSnapshot) &&
			0u == snapshotReader.Get_RemainingSize() &&
			decodedSnapshot.Players.front().iVehicleId == 6705u,
			"Player snapshot preserves the ridden vehicle");
		for (unsigned scenario = 0u; scenario < 4u; ++scenario)
		{
			auto invalid = source;
			auto& bad = invalid.Players.front();
			switch (scenario)
			{
			case 0u: bad.iCurrentHp = 0u; break;
			case 1u: bad.eAction = PLAYER_ACTION_STATE::KNOCKDOWN; bad.iActionStartTick = 90u; break;
			case 2u: bad.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN; break;
			case 3u: bad.iMarioStage = 1u; break;
			}
			CPacketWriter invalidWriter;
			testRunner.Require(!Write_Message(invalidWriter, invalid),
				"A ridden vehicle cannot accompany death, a forced action, the clown body or Mario");
		}
		testRunner.Require(Is_Known_Packet_Type(PACKET_TYPE::C2S_SET_VEHICLE_RIDING) &&
			Is_Known_Packet_Type(PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT) &&
			static_cast<std::uint16_t>(PACKET_TYPE::C2S_SET_VEHICLE_RIDING) ==
			static_cast<std::uint16_t>(PACKET_TYPE::C2S_DEBUG_BINGO_HAMMER) + 1u &&
			static_cast<std::uint16_t>(PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT) ==
			static_cast<std::uint16_t>(PACKET_TYPE::C2S_SET_VEHICLE_RIDING) + 1u &&
			NETWORK_PROTOCOL_VERSION == 82u,
			"Riding packet identities append without renumbering peers");
	}
```

3. 기준점 `Test_DebugMadnessFormProtocol(testRunner);` 호출 바로 아래 `Test_VehicleRidingProtocol(testRunner);` 추가.

---

## G04. Server 권위 탑승 상태

### 호출 흐름

```text
ServerApp receive C2S_SET_VEHICLE_RIDING
 → Read_Message(잘못된 payload면 세션 종료)
 → ROOM_COMMAND SET_VEHICLE_RIDING enqueue(reliable)
GameRoom::Tick 명령 루프
 → Handle_SetVehicleRiding → Apply_SetVehicleRiding
    world 일치 / sequence 멱등 / 하차 / 허용 월드 / catalog / Can_RideVehicle
    → player.iVehicleId commit + LastVehicleRidingResult 저장
 → S2C_SET_VEHICLE_RIDING_RESULT 전송
Update_Players → 이동 거리 = Resolve_PlayerMoveSpeed(탑승 시 탈것 속도)
Tick 마지막(m_iServerTick commit 직전) Enforce_VehicleRidingState
 → 사망/넉다운/잡기/패턴 속박/광대/마리오/월드 불허면 강제 하차
Broadcast_WorldSnapshot → PLAYER_SNAPSHOT.iVehicleId, fMoveSpeed
Handle_UseSkill / Handle_UseEstherSkill → 탑승 중 거부
Apply_CharacterClassChange → 하차
월드 이동 → 새 방에서 SERVER_PLAYER가 새로 만들어져 도보로 시작
```

### `Server/Public/VehicleCatalog.h` 전체 코드

```cpp
#pragma once

#include "Network/PacketMessages.h"

#include <string>
#include <unordered_map>

namespace LostArk::Server
{
	struct SERVER_VEHICLE_DEFINITION
	{
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		float fMoveSpeed = 0.f;
	};

	class CVehicleCatalog final
	{
	public:
		bool Load();

		const SERVER_VEHICLE_DEFINITION* Find_Vehicle(
			LostArk::Shared::VEHICLE_ID vehicleId) const;

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::unordered_map<LostArk::Shared::VEHICLE_ID, SERVER_VEHICLE_DEFINITION> m_Vehicles;
		std::string m_strStatus;
	};
}
```

### `Server/Private/VehicleCatalog.cpp` 전체 코드

```cpp
#include "VehicleCatalog.h"

#include <Windows.h>

#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace
{
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}
}

bool LostArk::Server::CVehicleCatalog::Load()
{
	using VEHICLE_MAP = decltype(m_Vehicles);
	VEHICLE_MAP staged;

	const std::filesystem::path dataRoot = Resolve_DataRoot();
	const std::filesystem::path path = dataRoot / L"Vehicles" / L"Vehicles.bootstrap";
	std::ifstream input(path, std::ios::binary);
	if (dataRoot.empty() || !input)
	{
		m_strStatus = "Missing vehicle bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Vehicle bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t rowCount = 0u;
	if (3u != header.size() || "LOSTARK_VEHICLE_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 1u != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > 4096u)
	{
		m_strStatus = "Vehicle bootstrap header is invalid";
		return false;
	}

	for (std::uint32_t row = 0u; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Vehicle bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		SERVER_VEHICLE_DEFINITION vehicle{};
		if (3u != fields.size() || "VEHICLE" != fields[0] ||
			!ParseNumber(fields[1], vehicle.iVehicleId) ||
			LostArk::Shared::INVALID_VEHICLE_ID == vehicle.iVehicleId ||
			!ParseNumber(fields[2], vehicle.fMoveSpeed) ||
			!std::isfinite(vehicle.fMoveSpeed) ||
			vehicle.fMoveSpeed <= 0.f || vehicle.fMoveSpeed > 30.f)
		{
			m_strStatus = "Vehicle bootstrap row is invalid";
			return false;
		}
		if (!staged.emplace(vehicle.iVehicleId, vehicle).second)
		{
			m_strStatus = "Duplicate vehicle ID";
			return false;
		}
	}

	if (std::getline(input, line))
	{
		m_strStatus = "Vehicle bootstrap has trailing rows";
		return false;
	}

	m_Vehicles = std::move(staged);
	m_strStatus = "Loaded vehicle bootstrap";
	return true;
}

const LostArk::Server::SERVER_VEHICLE_DEFINITION*
LostArk::Server::CVehicleCatalog::Find_Vehicle(
	const LostArk::Shared::VEHICLE_ID vehicleId) const
{
	const auto iter = m_Vehicles.find(vehicleId);
	return m_Vehicles.end() == iter ? nullptr : &iter->second;
}
```

### `Server/Private/GameRoom_VehicleRiding.cpp` 전체 코드

```cpp
#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <memory>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	bool Is_VehicleRidingWorld(const LostArk::Shared::WORLD_ID worldId)
	{
		return LostArk::Shared::WORLD_ID::BERN == worldId ||
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
	}
}

float LostArk::Server::CGameRoom::Resolve_PlayerMoveSpeed(
	const SERVER_PLAYER& player) const
{
	if (LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId)
	{
		if (const SERVER_VEHICLE_DEFINITION* vehicle =
			m_VehicleCatalog.Find_Vehicle(player.iVehicleId))
		{
			return vehicle->fMoveSpeed;
		}
	}
	return player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player);
}

bool LostArk::Server::CGameRoom::Can_RideVehicle(
	const SERVER_PLAYER& player) const
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp &&
		PLAYER_ACTION_STATE::NONE == player.eAction &&
		!player.bPatternBound &&
		0u == player.iMarioStage &&
		PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm &&
		KOUKU_HUD_MODE::NONE == player.eKoukuHudMode &&
		INVALID_NET_ENTITY_ID == player.iAttachmentOwnerNetEntityId &&
		player.fKnockbackRemainingSeconds <= 0.f &&
		!player.TriggerMove.isActive &&
		0u == player.CardMaze.flags &&
		0u == player.CardMaze.transferStartTick;
}

void LostArk::Server::CGameRoom::Handle_SetVehicleRiding(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = VEHICLE_RIDING_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_SetVehicleRiding(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT
LostArk::Server::CGameRoom::Apply_SetVehicleRiding(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.iActiveVehicleId = player.iVehicleId;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const S2C_SET_VEHICLE_RIDING_RESULT& previous = player.LastVehicleRidingResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
	{
		return previous;
	}
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto commit = [&player, &result](const VEHICLE_RIDING_RESULT reason)
	{
		result.eResult = reason;
		result.iActiveVehicleId = player.iVehicleId;
		player.LastVehicleRidingResult = result;
		return result;
	};
	if (request.iVehicleId == player.iVehicleId)
		return commit(VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE);
	if (INVALID_VEHICLE_ID == request.iVehicleId)
	{
		player.iVehicleId = INVALID_VEHICLE_ID;
		return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
	}
	if (!Is_VehicleRidingWorld(m_eWorldId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED);
	if (nullptr == m_VehicleCatalog.Find_Vehicle(request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE);
	if (!Can_RideVehicle(player))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE);
	player.iVehicleId = request.iVehicleId;
	return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
}

void LostArk::Server::CGameRoom::Enforce_VehicleRidingState()
{
	const bool ridingWorld = Is_VehicleRidingWorld(m_eWorldId);
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::INVALID_VEHICLE_ID == player.iVehicleId)
			continue;
		if (!ridingWorld ||
			nullptr == m_VehicleCatalog.Find_Vehicle(player.iVehicleId) ||
			!Can_RideVehicle(player))
		{
			player.iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		}
	}
}
```

### `Server/Public/ServerPlayer.h`

기준점: `LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT LastDebugMadnessFormResult;` 바로 아래 추가.

```cpp
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT LastVehicleRidingResult;
		/* The ridden vehicle or INVALID_VEHICLE_ID on foot. Only riding worlds
		admit it, and Enforce_VehicleRidingState clears it before the snapshot
		whenever the player can no longer ride. */
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
```

### `Server/Public/RoomCommand.h`

기준점: enum `DEBUG_SET_MADNESS_FORM,` 바로 아래 `SET_VEHICLE_RIDING,` 추가.
기준점: `LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM DebugSetMadnessForm;` 바로 아래 추가.

```cpp
		LostArk::Shared::C2S_SET_VEHICLE_RIDING SetVehicleRiding;
```

### `Server/Public/GameRoom.h`

기준점: `#include "ItemCatalog.h"` 바로 아래 `#include "VehicleCatalog.h"` 추가.

기준점: `friend int Run_ServerKoukuObjectOverlapContractTests();` 바로 아래 추가.

```cpp
		friend int Run_ServerVehicleRidingContractTests();
```

기준점: `Apply_DebugMadnessForm(…);` 선언(두 줄) 바로 아래 추가.

```cpp
		/* H key riding toggle for this session's player. The verdict is sent
		back; the ridden vehicle itself rides the world snapshot. */
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		/* True while nothing the player is doing forbids a vehicle underneath. */
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		/* Metres per second the player walks at: the ridden vehicle's speed, or
		the class speed scaled by its held stance. */
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		/* Dismounts every player the world, catalog or current state no longer
		lets ride. Runs once per tick before the snapshot is committed. */
		void Enforce_VehicleRidingState();
```

기준점: `CItemCatalog m_ItemCatalog;` 바로 아래 `CVehicleCatalog m_VehicleCatalog;` 추가.

### `Server/Private/GameRoom.cpp`

기준점: 생성자의 `if (!m_ItemCatalog.Load())` 블록 닫는 `}` 바로 아래 추가.

```cpp
	if (!m_VehicleCatalog.Load())
	{
		m_strStatus = m_VehicleCatalog.Get_Status();
		return;
	}
```

기준점: `case ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM:` 블록의 `break;` 바로 아래 추가.

```cpp
		case ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING:
			Handle_SetVehicleRiding(command.iSessionId, command.SetVehicleRiding);
			break;
```

기준점: `Tick`의 `m_iServerTick = updateTick;` 바로 위 추가.

```cpp
	Enforce_VehicleRidingState();
```

### `Server/Private/ServerApp.cpp`

기준점: `frame.ePacketType == PACKET_TYPE::C2S_DEBUG_SET_MADNESS_FORM` 분기 블록 닫는 `}` 바로 아래 추가.

```cpp
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_VEHICLE_RIDING)
	{
		C2S_SET_VEHICLE_RIDING request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_VEHICLE_RIDING");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING;
		command.SetVehicleRiding = request;
	}
```

### `Server/Private/GameRoom_PlayerCommands.cpp`

기준점: `Handle_UseSkill`의 `if (0u != playerIter->second.iMarioStage || playerIter->second.bPatternBound ||` 교체.

```cpp
	if (LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId ||
		0u != playerIter->second.iMarioStage || playerIter->second.bPatternBound ||
```

기준점: `Handle_UseEstherSkill`의 `if (playerIter->second.fKnockbackRemainingSeconds > 0.f)` 교체.

```cpp
	if (playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId)
```

기준점: `Apply_CharacterClassChange`의 `staged.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;` 바로 아래 추가.

```cpp
	staged.iVehicleId = INVALID_VEHICLE_ID;
```

### `Server/Private/GameRoom_PlayerSimulation.cpp`

기준점: `Update_Players`의 `player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *` 교체.

```cpp
				Resolve_PlayerMoveSpeed(player) *
```

### `Server/Private/GameRoom_Replication.cpp`

기준점: `snapshot.fMoveSpeed = player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player);` 교체.

```cpp
		snapshot.fMoveSpeed = Resolve_PlayerMoveSpeed(player);
```

기준점: `snapshot.eMadnessForm = player.eMadnessForm;` 바로 아래 추가.

```cpp
		snapshot.iVehicleId = player.iVehicleId;
```

### `Server/Public/ServerGameplayContractTests.h`

기준점: `int Run_ServerKoukuObjectOverlapContractTests();` 바로 아래 `int Run_ServerVehicleRidingContractTests();` 추가.

### `Server/Private/Main.cpp`

기준점: `--bingo-contract-test` 분기 두 줄 바로 아래 추가.

```cpp
	if (2 == argumentCount && std::string_view(arguments[1]) == "--vehicle-riding-contract-test")
		return LostArk::Server::Run_ServerVehicleRidingContractTests();
```

기준점: Usage 문자열 `"--bingo-contract-test | "` 교체.

```cpp
			"--bingo-contract-test | --vehicle-riding-contract-test | "
```

### `Server/Private/ServerGameplayContractTests_VehicleRiding.cpp` 전체 코드

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "VehicleCatalog.h"
#include "WorldBootstrap.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace
{
	constexpr VEHICLE_ID GOLDEN_TERPEION = 6705u;
	constexpr SESSION_ID RIDER_SESSION = 11u;
	constexpr PLAYER_ID RIDER_PLAYER = 1u;

	C2S_SET_VEHICLE_RIDING Make_VehicleRequest(
		const std::uint32_t sequence, const WORLD_ID worldId, const VEHICLE_ID vehicleId)
	{
		C2S_SET_VEHICLE_RIDING request{};
		request.iRequestSequence = sequence;
		request.eWorldId = worldId;
		request.iVehicleId = vehicleId;
		return request;
	}

	bool Near(const float value, const float expected)
	{
		return std::abs(value - expected) < 0.0001f;
	}
}

int LostArk::Server::Run_ServerVehicleRidingContractTests()
{
	TESTS tests;
	auto bern = std::make_unique<CGameRoom>(WORLD_ID::BERN);
	tests.Require(bern->Is_Ready(), "Bern room loads the published vehicle bootstrap");
	if (!bern->Is_Ready())
	{
		std::cout << bern->Get_Status() << '\n';
		return 1;
	}
	const SERVER_VEHICLE_DEFINITION* terpeion =
		bern->m_VehicleCatalog.Find_Vehicle(GOLDEN_TERPEION);
	tests.Require(nullptr != terpeion && Near(terpeion->fMoveSpeed, 5.f),
		"Golden Terpeion carries EFTable_Vehicle MoveSpeed 500 as 5 m/s");

	const auto& placements = bern->m_WorldBootstrap.Get_Placements();
	const auto spawn = std::find_if(placements.begin(), placements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
		});
	tests.Require(placements.end() != spawn, "Bern publishes an enabled player spawn");
	if (placements.end() == spawn)
		return 1;

	SERVER_PLAYER& rider = bern->m_Players[RIDER_PLAYER];
	rider.iPlayerId = RIDER_PLAYER;
	rider.iNetEntityId = 101u;
	rider.iSessionId = RIDER_SESSION;
	rider.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
	rider.iCurrentHp = rider.iMaximumHp = 50000u;
	rider.fMoveSpeed = 2.8f;
	rider.fPositionX = spawn->fPositionX;
	rider.fPositionY = spawn->fPositionY;
	rider.fPositionZ = spawn->fPositionZ;
	bern->m_PlayerIdBySessionId[RIDER_SESSION] = RIDER_PLAYER;

	const S2C_SET_VEHICLE_RIDING_RESULT mounted = bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(1u, WORLD_ID::BERN, GOLDEN_TERPEION));
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == mounted.eResult &&
		GOLDEN_TERPEION == mounted.iActiveVehicleId &&
		GOLDEN_TERPEION == rider.iVehicleId &&
		Near(bern->Resolve_PlayerMoveSpeed(rider), 5.f),
		"Mounting in Bern commits the vehicle and moves at the vehicle speed");

	const S2C_SET_VEHICLE_RIDING_RESULT replayed = bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(1u, WORLD_ID::BERN, INVALID_VEHICLE_ID));
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == replayed.eResult &&
		GOLDEN_TERPEION == rider.iVehicleId,
		"A replayed sequence answers the committed verdict without applying its payload");

	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(2u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult,
		"Mounting the vehicle already ridden is a typed no-op");

	C2S_USE_SKILL attack{};
	attack.iClientSequence = 1u;
	attack.iSkillId = 17000u;
	attack.fAimX = rider.fPositionX + 1.f;
	attack.fAimZ = rider.fPositionZ;
	bern->Handle_UseSkill(RIDER_SESSION, attack);
	tests.Require(PLAYER_ACTION_STATE::NONE == rider.eAction &&
		GOLDEN_TERPEION == rider.iVehicleId && 0u == rider.iLastSkillSequence,
		"A skill request while mounted is refused before the skill system runs");

	rider.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
	bern->Enforce_VehicleRidingState();
	tests.Require(INVALID_VEHICLE_ID == rider.iVehicleId &&
		Near(bern->Resolve_PlayerMoveSpeed(rider), 2.8f),
		"A forced action dismounts before the snapshot and restores the class speed");
	rider.eAction = PLAYER_ACTION_STATE::NONE;

	bern->Handle_UseSkill(RIDER_SESSION, attack);
	tests.Require(0u != rider.iLastSkillSequence,
		"The same skill request reaches the skill system once the player is on foot");
	rider.eAction = PLAYER_ACTION_STATE::NONE;
	rider.iCurrentSkillId = INVALID_SKILL_ID;

	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(3u, WORLD_ID::BERN, 1u)).eResult,
		"An id outside the published bootstrap is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_STALE_SEQUENCE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(2u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult,
		"An older sequence is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_WRONG_WORLD == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(4u, WORLD_ID::VALTAN_ARENA, GOLDEN_TERPEION)).eResult,
		"A request addressed to another world is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(5u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult &&
		VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(6u, WORLD_ID::BERN, INVALID_VEHICLE_ID)).eResult &&
		INVALID_VEHICLE_ID == rider.iVehicleId,
		"Dismount clears the vehicle");

	auto valtan = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
	tests.Require(valtan->Is_Ready(), "Valtan room loads");
	if (valtan->Is_Ready())
	{
		SERVER_PLAYER& raider = valtan->m_Players[RIDER_PLAYER];
		raider.iPlayerId = RIDER_PLAYER;
		raider.iCurrentHp = raider.iMaximumHp = 50000u;
		tests.Require(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED ==
			valtan->Apply_SetVehicleRiding(raider,
				Make_VehicleRequest(1u, WORLD_ID::VALTAN_ARENA, GOLDEN_TERPEION)).eResult &&
			INVALID_VEHICLE_ID == raider.iVehicleId,
			"A raid arena refuses to mount");
		raider.iVehicleId = GOLDEN_TERPEION;
		valtan->Enforce_VehicleRidingState();
		tests.Require(INVALID_VEHICLE_ID == raider.iVehicleId,
			"A raid arena never replicates a mounted player");
	}

	std::cout << "vehicle riding failures: " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
```

### `Server/Default/Server.vcxproj`

기준점: `<ClInclude Include="..\Public\ItemCatalog.h" />` 바로 아래 `<ClInclude Include="..\Public\VehicleCatalog.h" />`.
기준점: `<ClCompile Include="..\Private\ItemCatalog.cpp" />` 바로 아래 `<ClCompile Include="..\Private\VehicleCatalog.cpp" />`.
기준점: `<ClCompile Include="..\Private\GameRoom_KoukuPlayerCommands.cpp" />` 바로 아래 `<ClCompile Include="..\Private\GameRoom_VehicleRiding.cpp" />`.
기준점: `ServerGameplayContractTests_Bingo.cpp` ClCompile 블록(3줄) 바로 아래 추가.

```xml
    <ClCompile Include="..\Private\ServerGameplayContractTests_VehicleRiding.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
```

### `Server/Default/Server.vcxproj.filters`

기준점별로 기존 이웃과 같은 필터를 쓴다.

```xml
	<ClInclude Include="..\Public\VehicleCatalog.h"><Filter>Public</Filter></ClInclude>
	<ClCompile Include="..\Private\VehicleCatalog.cpp"><Filter>Private</Filter></ClCompile>
    <ClCompile Include="..\Private\GameRoom_VehicleRiding.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_VehicleRiding.cpp"><Filter>Private</Filter></ClCompile>
```

(각각 `ItemCatalog.h`, `ItemCatalog.cpp`, `GameRoom_KoukuPlayerCommands.cpp`, `ServerGameplayContractTests_Bingo.cpp` 줄 바로 아래)

---

## G05. Server 수치 데이터와 publisher

`MoveSpeed 500`은 원본 cm/s이고 우리 플레이어 `moveSpeed` 2.95는 m/s다. `value / divisor = 5.0`을 publisher가 검사한다.
탈것 수치는 `PlayerProfiles.json`/provenance receipt와 분리한다(receipt 해시·coverage를 건드리지 않음).

### `Data/Vehicles/VehicleProfiles.json` 전체 코드

```json
{
  "schema": "lostark.vehicle-profiles",
  "formatVersion": 1,
  "vehicles": [
    {
      "vehicleId": 6705,
      "moveSpeed": 5.0,
      "source": {
        "table": "EFTable_Vehicle",
        "primaryKey": 6705,
        "column": "MoveSpeed",
        "value": 500,
        "divisor": 100
      }
    }
  ]
}
```

### `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1` 전체 코드

```powershell
[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Vehicles'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing vehicle document: $RelativePath" }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-JsonInteger([object]$Value, [string]$Context, [long]$Minimum, [long]$Maximum) {
    if (($Value -isnot [int]) -and ($Value -isnot [long])) {
        throw "$Context must be a JSON integer."
    }
    if ([long]$Value -lt $Minimum -or [long]$Value -gt $Maximum) {
        throw "$Context integer is out of range: $Value"
    }
}

function Assert-JsonNumber([object]$Value, [string]$Context) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and ($Value -isnot [double]) -and ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    if ([double]::IsNaN([double]$Value) -or [double]::IsInfinity([double]$Value)) {
        throw "$Context must be finite."
    }
}

$document = Read-JsonDocument 'Data/Vehicles/VehicleProfiles.json'
Assert-ExactProperties $document @('schema', 'formatVersion', 'vehicles') 'vehicle profile document'
if ($document.schema -cne 'lostark.vehicle-profiles' -or $document.formatVersion -ne 1) {
    throw 'Vehicle profile header is invalid.'
}
$vehicles = @($document.vehicles)
if ($vehicles.Count -eq 0 -or $vehicles.Count -gt 4096) {
    throw "Vehicle profile count is out of range: $($vehicles.Count)"
}

$vehicleIds = [Collections.Generic.HashSet[uint32]]::new()
$rows = [Collections.Generic.List[string]]::new()
foreach ($vehicle in $vehicles) {
    Assert-ExactProperties $vehicle @('vehicleId', 'moveSpeed', 'source') 'vehicle profile'
    Assert-JsonInteger $vehicle.vehicleId 'vehicle vehicleId' 1 ([uint32]::MaxValue)
    Assert-JsonNumber $vehicle.moveSpeed "vehicle $($vehicle.vehicleId) moveSpeed"
    Assert-ExactProperties $vehicle.source @('table', 'primaryKey', 'column', 'value', 'divisor') "vehicle $($vehicle.vehicleId) source"
    Assert-JsonInteger $vehicle.source.primaryKey "vehicle $($vehicle.vehicleId) source primaryKey" 1 ([uint32]::MaxValue)
    Assert-JsonInteger $vehicle.source.value "vehicle $($vehicle.vehicleId) source value" 1 100000
    Assert-JsonInteger $vehicle.source.divisor "vehicle $($vehicle.vehicleId) source divisor" 1 100000
    $speed = [double]$vehicle.moveSpeed
    if ($vehicle.source.table -cne 'EFTable_Vehicle' -or $vehicle.source.column -cne 'MoveSpeed' -or
        [uint32]$vehicle.source.primaryKey -ne [uint32]$vehicle.vehicleId -or
        [Math]::Abs(([double]$vehicle.source.value / [double]$vehicle.source.divisor) - $speed) -gt 0.000001 -or
        $speed -le 0.0 -or $speed -gt 30.0) {
        throw "Vehicle $($vehicle.vehicleId) speed does not match its EFTable_Vehicle source."
    }
    if (-not $vehicleIds.Add([uint32]$vehicle.vehicleId)) {
        throw "Duplicate vehicle ID: $($vehicle.vehicleId)"
    }
    $rows.Add((@('VEHICLE', [uint32]$vehicle.vehicleId,
        $speed.ToString('R', [Globalization.CultureInfo]::InvariantCulture)) -join "`t"))
}

if ($Mode -eq 'Validate') {
    Write-Output "Vehicle profile Validate succeeded: $($rows.Count) vehicles."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Vehicle profile OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Vehicle profile OutputRoot escaped the repository.'
}
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_VEHICLE_BOOTSTRAP`t1`t$($rows.Count)")
foreach ($row in $rows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Vehicles.bootstrap'
$transactionId = [Guid]::NewGuid().ToString('N')
$staged = "$destination.staging.$transactionId"
$rollback = "$destination.rollback.$transactionId"
$hadPrevious = $false
try {
    [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
    if ([IO.File]::Exists($destination)) {
        [IO.File]::Move($destination, $rollback)
        $hadPrevious = $true
    }
    [IO.File]::Move($staged, $destination)
    if ($hadPrevious) { [IO.File]::Delete($rollback) }
    Write-Output "Vehicle profile Publish succeeded: $($rows.Count) vehicles -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
```

### `Tools/Build/BuildDomains.json`

기준점: `"id": "items.catalog"` 도메인 객체의 닫는 `},` 바로 아래 추가.

```json
    {
      "id": "vehicles.profiles",
      "kind": "publisher",
      "profiles": ["Core", "FullDiagnostic"],
      "inputs": ["Data/Vehicles/VehicleProfiles.json"],
      "tools": ["Tools/GameplayPipeline/Publish-VehicleProfiles.ps1"],
      "outputs": ["Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap"],
      "requiredOutputPatterns": ["Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap"],
      "action": {
        "executable": "powershell.exe",
        "arguments": [
          "-NoProfile",
          "-ExecutionPolicy",
          "Bypass",
          "-File",
          "Tools/GameplayPipeline/Publish-VehicleProfiles.ps1",
          "-Mode",
          "Publish"
        ]
      }
    },
```

### `Tools/Build/Invoke-BuildDomainOwner.ps1`

기준점: Server 목록의 `'items.catalog',` 바로 아래 `'vehicles.profiles',` 추가.

### `Tools/Build/Invoke-BuildAndRegression.ps1`

기준점: `'Server/Bin/DataFiles/Items/Items.bootstrap',` 바로 아래 `'Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap',` 추가.

`CGameRoom`이 부트스트랩 없이는 준비되지 않으므로 Server PC는 `Invoke-BuildDomainOwner.ps1 -Owner Server`를 한 번 실행해야 한다.

---

## G06. Client VehicleCatalog

### `Data/Actors/VehicleCatalog.json` 전체 코드

`modelMaterialOverrides`는 G01 `rows` 명령의 출력 그대로다(파라미터 115개, 텍스처 18개).

```json
{
  "schema": "lostark.vehicle-catalog",
  "formatVersion": 1,
  "vehicles": [
    {
      "vehicleId": 6705,
      "archetypeId": "VEHICLE_TERPEION_GOLD",
      "modelAssetId": "Character/Vehicle/Terpeion/Terpeion.wmodel",
      "modelPreScale": 0.0001,
      "seatBone": "b_cockpit",
      "vehicleIdleClip": "npc_idle_normal_1",
      "vehicleRunClip": "npc_run_normal_1",
      "riders": [
        { "characterClass": "LANCE_MASTER", "idleClip": "flm_ride_horse_idle_normal_1", "runClip": "flm_ride_horse_run_normal_1" },
        { "characterClass": "WARLORD", "idleClip": "wgl_ride_horse_idle_normal_1", "runClip": "wgl_ride_horse_run_normal_1" },
        { "characterClass": "ARTIST", "idleClip": "sdm_ride_horse_idle_normal_1", "runClip": "sdm_ride_horse_run_normal_1" },
        { "characterClass": "DIMENSIONMASTER", "idleClip": "pc_sp_m_00_sk_ride_horse_idle_normal_1", "runClip": "pc_sp_m_00_sk_ride_horse_run_normal_1" }
      ],
      "modelMaterialOverrides": [
        {
          "modelAssetId": "Character/Vehicle/Terpeion/Terpeion.wmodel",
          "materialName": "mn_pmstg_01_mi",
          "sourceMaterial": "mn_pmstg_01.mat.mn_pmstg_01_mi",
          "family": "source.vehicle.terpeion-body.v1",
          "parameters": {
            "1.use_dyeing_sp": [0.0, 0.0, 0.0, 0.0],
            "1.use_emissive_flickerspeed_fixed": [0.0, 0.0, 0.0, 0.0],
            "beckmannspecular_constant_max": [3.3499999, 3.3499999, 3.3499999, 3.3499999],
            "buffcolor": [0.0, 0.0, 0.0, 1.0],
            "constantoutline": [0.0, 0.0, 0.0, 0.0],
            "constantoutline_blink": [0.5, 0.5, 0.5, 0.5],
            "constantoutline_color": [0.0, 0.0, 0.0, 1.0],
            "diffusecolor": [2.23446631, 1.9383378, 1.65426719, 1.0],
            "diffusecolor_a": [0.0497219265, 0.0247932058, 0.016483631, 1.0],
            "diffusecolor_b": [1.24826443, 1.15407479, 1.02154243, 1.0],
            "diffusecolor_c": [1.0, 1.0, 1.0, 1.0],
            "emissive_color": [1.75723255, 0.586944759, 0.0, 1.0],
            "emissive_flicker_speed": [0.5, 0.5, 0.5, 0.5],
            "emissive_intensity": [2.0, 2.0, 2.0, 2.0],
            "emissive_intensitymin": [1.0, 1.0, 1.0, 1.0],
            "fresnel_radius": [0.949999988, 0.949999988, 0.949999988, 0.949999988],
            "fresnel_rimlightintensity": [0.5, 0.5, 0.5, 0.5],
            "fx_color_desaturation_actiontool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_desaturation_buffsettool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_intensity_actiontool": [0.0, 0.0, 0.0, 1.0],
            "fx_color_intensity_buffsettool": [0.0, 0.0, 0.0, 1.0],
            "hit_color": [0.0, 0.0, 0.0, 0.0],
            "ibl_color_bottom": [1.0, 1.0, 1.0, 1.0],
            "ibl_color_top": [1.0, 1.0, 1.0, 1.0],
            "ibl_exposer": [5.0, 5.0, 5.0, 5.0],
            "ibl_intensity": [1.0, 1.0, 1.0, 1.0],
            "ibl_normal_smooth": [0.699999988, 0.699999988, 0.699999988, 0.699999988],
            "ibl_reflect_lodbias": [80.0, 80.0, 80.0, 80.0],
            "metalicness_power": [0.300000012, 0.300000012, 0.300000012, 0.300000012],
            "normaltex_intensity": [1.0, 1.0, 1.0, 1.0],
            "orennayar": [1.0, 1.0, 1.0, 1.0],
            "orennayar_brightness": [1.0, 1.0, 1.0, 1.0],
            "pbr_specular_intensity": [10.0, 10.0, 10.0, 10.0],
            "pbr_specular_power": [6.0, 6.0, 6.0, 6.0],
            "roughness_power": [5.0, 5.0, 5.0, 5.0],
            "selectioncolor": [0.0, 0.0, 0.0, 1.0],
            "shadowfactor": [0.5, 0.5, 0.5, 0.5],
            "specular_power_limit": [0.920000017, 0.920000017, 0.920000017, 0.920000017],
            "state": [0.0, 0.0, 0.0, 0.0],
            "state_noise": [1.0, 0.0, 0.0, 1.0],
            "trans_rim_hard": [1.0, 1.0, 1.0, 1.0],
            "trans_rim_inradius": [0.0, 0.0, 0.0, 0.0],
            "transcolor": [0.0, 0.0, 0.0, 1.0],
            "transcolor_rimlight ": [1.0, 1.0, 1.0, 1.0]
          },
          "textures": [
            { "expressionIndex": 0, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_n.tga", "colorSpace": "linear" },
            { "expressionIndex": 1, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_cm.tga", "colorSpace": "srgb" },
            { "expressionIndex": 2, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01_d.tga", "colorSpace": "srgb" },
            { "expressionIndex": 3, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_s.tga", "colorSpace": "srgb" },
            { "expressionIndex": 4, "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga", "colorSpace": "srgb" },
            { "expressionIndex": 5, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00_e.tga", "colorSpace": "srgb" },
            { "expressionIndex": 6, "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga", "colorSpace": "srgb" },
            { "expressionIndex": 7, "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds", "colorSpace": "linear" }
          ]
        },
        {
          "modelAssetId": "Character/Vehicle/Terpeion/Terpeion.wmodel",
          "materialName": "mn_pmstg_01-1_mi",
          "sourceMaterial": "mn_pmstg_01.mat.mn_pmstg_01-1_mi",
          "family": "source.character.monster-d621a47e69ad.v1",
          "parameters": {
            "1.use_dyeing_sp": [0.0, 0.0, 0.0, 0.0],
            "beckmannspecular_constant_max": [3.3499999, 3.3499999, 3.3499999, 3.3499999],
            "buffcolor": [0.0, 0.0, 0.0, 1.0],
            "constantoutline": [0.0, 0.0, 0.0, 0.0],
            "constantoutline_blink": [0.5, 0.5, 0.5, 0.5],
            "constantoutline_color": [0.0, 0.0, 0.0, 1.0],
            "diffusecolor": [1.0, 1.0, 1.0, 1.0],
            "diffusecolor_a": [2.03363037, 1.78128707, 1.51681519, 1.0],
            "diffusecolor_b": [12.3806438, 8.18070984, 2.46281385, 1.0],
            "diffusecolor_c": [1.0, 1.0, 1.0, 1.0],
            "fresnel_radius": [0.949999988, 0.949999988, 0.949999988, 0.949999988],
            "fresnel_rimlightintensity": [0.5, 0.5, 0.5, 0.5],
            "fx_color_desaturation_actiontool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_desaturation_buffsettool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_intensity_actiontool": [0.0, 0.0, 0.0, 1.0],
            "fx_color_intensity_buffsettool": [0.0, 0.0, 0.0, 1.0],
            "hit_color": [0.0, 0.0, 0.0, 0.0],
            "ibl_color_bottom": [1.0, 1.0, 1.0, 1.0],
            "ibl_color_top": [1.0, 1.0, 1.0, 1.0],
            "ibl_exposer": [5.0, 5.0, 5.0, 5.0],
            "ibl_intensity": [1.0, 1.0, 1.0, 1.0],
            "ibl_normal_smooth": [0.699999988, 0.699999988, 0.699999988, 0.699999988],
            "ibl_reflect_lodbias": [80.0, 80.0, 80.0, 80.0],
            "metalicness_power": [0.200000003, 0.200000003, 0.200000003, 0.200000003],
            "normaltex_intensity": [1.0, 1.0, 1.0, 1.0],
            "orennayar": [1.0, 1.0, 1.0, 1.0],
            "orennayar_brightness": [1.0, 1.0, 1.0, 1.0],
            "pbr_specular_intensity": [10.0, 10.0, 10.0, 10.0],
            "pbr_specular_power": [6.0, 6.0, 6.0, 6.0],
            "roughness_power": [3.0, 3.0, 3.0, 3.0],
            "selectioncolor": [0.0, 0.0, 0.0, 1.0],
            "shadowfactor": [0.5, 0.5, 0.5, 0.5],
            "specular_power_limit": [0.920000017, 0.920000017, 0.920000017, 0.920000017],
            "state": [0.0, 0.0, 0.0, 0.0],
            "state_noise": [1.0, 0.0, 0.0, 1.0],
            "trans_rim_hard": [1.0, 1.0, 1.0, 1.0],
            "trans_rim_inradius": [0.0, 0.0, 0.0, 0.0],
            "transcolor": [0.0, 0.0, 0.0, 1.0],
            "transcolor_rimlight ": [1.0, 1.0, 1.0, 1.0]
          },
          "textures": [
            { "expressionIndex": 0, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_n.tga", "colorSpace": "linear" },
            { "expressionIndex": 1, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_cm.tga", "colorSpace": "srgb" },
            { "expressionIndex": 2, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01-1_d.tga", "colorSpace": "srgb" },
            { "expressionIndex": 3, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_00-1_s.tga", "colorSpace": "srgb" },
            { "expressionIndex": 4, "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga", "colorSpace": "srgb" },
            { "expressionIndex": 5, "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga", "colorSpace": "srgb" },
            { "expressionIndex": 6, "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds", "colorSpace": "linear" }
          ]
        },
        {
          "modelAssetId": "Character/Vehicle/Terpeion/Terpeion.wmodel",
          "materialName": "mn_pmstg_01-3_mi",
          "sourceMaterial": "mn_pmstg_01.mat.mn_pmstg_01-3_mi",
          "family": "source.character.hair-two-tone.v1",
          "parameters": {
            "buffcolor": [0.0, 0.0, 0.0, 1.0],
            "customshade_shadowdiffuselighting": [0.0, 0.0, 0.0, 0.0],
            "fx_color_desaturation_actiontool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_desaturation_buffsettool": [0.0, 0.0, 0.0, 0.0],
            "fx_color_intensity_actiontool": [0.0, 0.0, 0.0, 1.0],
            "fx_color_intensity_buffsettool": [0.0, 0.0, 0.0, 1.0],
            "hairtwotone_color_switch": [0.0, 0.0, 0.0, 0.0],
            "hit_color": [0.0, 0.0, 0.0, 0.0],
            "rimlight_color": [0.265183598, 0.265183598, 0.265183598, 1.0],
            "rimlight_power": [3.0, 3.0, 3.0, 3.0],
            "selectioncolor": [0.0, 0.0, 0.0, 1.0],
            "specular_intensity": [1.0, 1.0, 1.0, 1.0],
            "specular_offset": [0.0, 0.0, 0.0, 0.0],
            "specular_power": [60.0, 60.0, 60.0, 60.0],
            "state": [0.0, 0.0, 0.0, 0.0],
            "state_noise": [1.0, 0.0, 0.0, 1.0],
            "subspecular_intensity": [0.5, 0.5, 0.5, 0.5],
            "subspecular_power": [60.0, 60.0, 60.0, 60.0],
            "trans_rim_hard": [1.0, 1.0, 1.0, 1.0],
            "trans_rim_inradius": [0.0, 0.0, 0.0, 0.0],
            "transcolor": [0.0, 0.0, 0.0, 1.0],
            "transcolor_rimlight ": [1.0, 1.0, 1.0, 1.0],
            "var_base_haircolor_base_ui": [1.66051006, 1.2359463, 0.649322808, 1.0],
            "var_base_hairspecularintensity_ui": [1.0, 1.0, 1.0, 1.0],
            "var_base_hairspecularpower_ui": [0.800000012, 0.800000012, 0.800000012, 0.800000012],
            "var_base_hairtwotone_bool_ui": [0.0, 0.0, 0.0, 0.0],
            "var_base_hairtwotonecolor_ui": [1.0, 1.0, 1.0, 1.0],
            "var_base_hairtwotonerange_bool_ui": [0.0, 0.0, 0.0, 0.0],
            "var_base_hairtwotonerangea_ui": [0.5, 0.5, 0.5, 0.5],
            "var_base_hairtwotonerangeb_ui": [0.5, 0.5, 0.5, 0.5],
            "var_base_hairtwotonerangeedge_ui": [0.5, 0.5, 0.5, 0.5],
            "var_base_hairtwotonerangehardness_ui": [0.5, 0.5, 0.5, 0.5]
          },
          "textures": [
            { "expressionIndex": 0, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmshs_00_hair_n.tga", "colorSpace": "linear" },
            { "expressionIndex": 1, "assetId": "Character/Vehicle/Terpeion/SourceMaterials/mn_pmstg_01_hair_d.tga", "colorSpace": "srgb" },
            { "expressionIndex": 2, "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga", "colorSpace": "srgb" }
          ]
        }
      ],
      "runtimeStatus": "supported"
    }
  ]
}
```

`modelPreScale 0.0001`과 `-90°` admission 회전은 NPC 파이프라인으로 쿠킹한 이 wmodel의 기존 계약(`CNpcPresentationAssetService`)과 같다.

### `Data/Actors/NpcCatalog.json`

`"archetypeId": "VEHICLE_TERPEION_GOLD"` 객체 전체와 그 앞 객체 끝의 `,`를 삭제한다(`NPC_59620` 객체가 배열 마지막이 된다).

### `Client/Public/ActorCatalog.h`

기준점: `struct MONSTER_ACTOR_ENTRY final` 바로 위에 추가.

```cpp
/* One rider pose set on a vehicle: the body clips the class plays while seated.
The clip names are the cooked "<armature>_ride_<mode>_*" names on the class body. */
struct VEHICLE_RIDER_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string idleClip;
	std::string runClip;
};

/* A rideable vehicle's presentation. vehicleId is the EFTable_Vehicle key the
Server replicates; everything else stays on the Client. */
struct VEHICLE_ACTOR_ENTRY final
{
	std::uint32_t vehicleId = 0u;
	std::string archetypeId;
	std::string modelAssetId;
	f32_t modelPreScale = 0.f;
	std::string seatBone;
	std::string vehicleIdleClip;
	std::string vehicleRunClip;
	std::vector<VEHICLE_RIDER_ENTRY> riders;
	std::string runtimeStatus;

	const VEHICLE_RIDER_ENTRY* Find_Rider(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
	{
		for (const VEHICLE_RIDER_ENTRY& rider : riders)
			if (rider.characterClass == characterClass)
				return &rider;
		return nullptr;
	}
};
```

기준점: `static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();` 바로 아래 추가.

```cpp
	static const VEHICLE_ACTOR_ENTRY* Find_Vehicle(std::uint32_t vehicleId);
	static const VEHICLE_ACTOR_ENTRY* Find_VehicleByArchetype(std::string_view archetypeId);
	static const std::vector<VEHICLE_ACTOR_ENTRY>& Get_Vehicles();
```

### `Client/Private/ActorCatalog.cpp`

기준점: 익명 namespace의 `std::vector<MONSTER_ACTOR_ENTRY> g_Monsters;` 바로 아래 추가.

```cpp
	std::vector<VEHICLE_ACTOR_ENTRY> g_Vehicles;
	ModelMaterials g_VehicleModelMaterials;
```

기준점: `bool_t ParseMonsters(const DATA_JSON_VALUE& root)` 정의의 닫는 `}` 바로 아래(익명 namespace 닫는 `}` 위) 추가.

```cpp

	bool_t ParseVehicles(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* pSchema = root.Find("schema");
		const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
		const DATA_JSON_VALUE* pEntries = root.Find("vehicles");
		if (3u != root.Get_Object().size() ||
			nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.vehicle-catalog" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 1.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::uint32_t> vehicleIds;
		std::set<std::string> archetypes;
		std::vector<VEHICLE_ACTOR_ENTRY> staged;
		ModelMaterials stagedMaterials;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object() || 10u != value.Get_Object().size())
				return false;
			VEHICLE_ACTOR_ENTRY entry;
			const DATA_JSON_VALUE* pRiders = value.Find("riders");
			if (!ReadRequiredU32(value, "vehicleId", entry.vehicleId) || 0u == entry.vehicleId ||
				!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!IsStableId(entry.archetypeId) ||
				!ReadRequiredString(value, "modelAssetId", entry.modelAssetId) ||
				!IsResourceId(entry.modelAssetId) ||
				!ReadRequiredNumber(value, "modelPreScale", entry.modelPreScale) ||
				entry.modelPreScale <= 0.f || entry.modelPreScale > 1.f ||
				!ReadRequiredString(value, "seatBone", entry.seatBone) ||
				!ReadRequiredString(value, "vehicleIdleClip", entry.vehicleIdleClip) ||
				!ReadRequiredString(value, "vehicleRunClip", entry.vehicleRunClip) ||
				!ReadRequiredString(value, "runtimeStatus", entry.runtimeStatus) ||
				entry.runtimeStatus != "supported" ||
				nullptr == pRiders || !pRiders->Is_Array() ||
				pRiders->Get_Array().empty() || pRiders->Get_Array().size() > 16u ||
				nullptr == value.Find("modelMaterialOverrides") ||
				!vehicleIds.insert(entry.vehicleId).second ||
				!archetypes.insert(entry.archetypeId).second)
			{
				return false;
			}
			std::set<LostArk::Shared::CHARACTER_CLASS_ID> riderClasses;
			for (const DATA_JSON_VALUE& riderValue : pRiders->Get_Array())
			{
				VEHICLE_RIDER_ENTRY rider;
				std::string characterClass;
				if (!riderValue.Is_Object() || 3u != riderValue.Get_Object().size() ||
					!ReadRequiredString(riderValue, "characterClass", characterClass) ||
					!ReadRequiredString(riderValue, "idleClip", rider.idleClip) ||
					!ReadRequiredString(riderValue, "runClip", rider.runClip))
				{
					return false;
				}
				rider.characterClass = ParseClass(characterClass);
				if (LostArk::Shared::CHARACTER_CLASS_ID::END == rider.characterClass ||
					!riderClasses.insert(rider.characterClass).second)
				{
					return false;
				}
				entry.riders.push_back(std::move(rider));
			}
			if (!ParseModelMaterialOverrides(value, stagedMaterials))
				return false;
			staged.push_back(std::move(entry));
		}
		for (const auto& [asset, materials] : stagedMaterials)
		{
			(void)materials;
			if (std::none_of(staged.begin(), staged.end(),
				[&asset](const VEHICLE_ACTOR_ENTRY& vehicle) { return vehicle.modelAssetId == asset; }))
			{
				return false;
			}
		}
		g_Vehicles = std::move(staged);
		g_VehicleModelMaterials = std::move(stagedMaterials);
		return !g_Vehicles.empty();
	}
```

기준점: `Initialize()` 본문 전체 교체(`bool_t Client::CActorCatalog::Initialize()`의 여는 `{`부터 닫는 `}`까지).

```cpp
{
	if (g_isInitialized)
		return true;
	DATA_JSON_VALUE characters;
	DATA_JSON_VALUE bosses;
	DATA_JSON_VALUE npcs;
	DATA_JSON_VALUE monsters;
	DATA_JSON_VALUE vehicles;
	if (!ReadDocument(L"Actors/CharacterCatalog.json", characters) ||
		!ReadDocument(L"Actors/BossCatalog.json", bosses) ||
		!ReadDocument(L"Actors/NpcCatalog.json", npcs) ||
		!ReadDocument(L"Actors/MonsterCatalog.json", monsters) ||
		!ReadDocument(L"Actors/VehicleCatalog.json", vehicles) ||
		!ParseCharacters(characters) || !ParseBosses(bosses) ||
		!ParseNpcs(npcs) || !ParseMonsters(monsters) || !ParseVehicles(vehicles))
	{
		g_Characters.clear();
		g_Bosses.clear();
		g_BossModelMaterials.clear();
		g_Npcs.clear();
		g_Monsters.clear();
		g_Vehicles.clear();
		g_VehicleModelMaterials.clear();
		g_Status = "Actor catalog contract mismatch.";
		return false;
	}
	g_isInitialized = true;
	g_Status = "Actor catalogs ready.";
	return true;
}
```

기준점: `Build_ModelLoadDescription`의 `else` 블록(`const auto found = g_BossModelMaterials.find(asset);`를 감싼 블록) 교체.

```cpp
    else
    {
        const auto found = g_BossModelMaterials.find(asset);
        const auto vehicle = g_VehicleModelMaterials.find(asset);
        if (found != g_BossModelMaterials.end() && vehicle != g_VehicleModelMaterials.end())
        {
            outStatus = "Model material ownership is ambiguous: " + asset;
            return false;
        }
        if (found != g_BossModelMaterials.end())
            staged.materialOverrides = found->second;
        else if (vehicle != g_VehicleModelMaterials.end())
            staged.materialOverrides = vehicle->second;
    }
```

기준점: `Get_Npcs()` 정의 닫는 `}` 바로 아래 추가.

```cpp

const Client::VEHICLE_ACTOR_ENTRY* Client::CActorCatalog::Find_Vehicle(
	const std::uint32_t vehicleId)
{
	if (!Initialize())
		return nullptr;
	for (const VEHICLE_ACTOR_ENTRY& entry : g_Vehicles)
		if (entry.vehicleId == vehicleId)
			return &entry;
	return nullptr;
}

const Client::VEHICLE_ACTOR_ENTRY* Client::CActorCatalog::Find_VehicleByArchetype(
	const std::string_view archetypeId)
{
	if (!Initialize())
		return nullptr;
	for (const VEHICLE_ACTOR_ENTRY& entry : g_Vehicles)
		if (entry.archetypeId == archetypeId)
			return &entry;
	return nullptr;
}

const std::vector<Client::VEHICLE_ACTOR_ENTRY>& Client::CActorCatalog::Get_Vehicles()
{
	Initialize();
	return g_Vehicles;
}
```

---

## G07. 탈것 표현

### 좌석 합성

```text
CCharacter::Update
 → Update_NetworkTransform (Server 보간 위치/yaw → m_pTransformCom)
 → Update_PresentationRootMatrix
     m_VehicleRootMatrix = transform world (말 파츠의 부모)
     seatOffset = CPart_Vehicle::Try_Get_SeatWorldPosition() - root 위치  (b_cockpit, 직전 프레임 포즈)
     m_PresentationRootMatrix = scale * world * translation(seatOffset)   (몸통/장비/무기의 부모)
 → CContainerObject::Update → 파츠 Update (말 애니메이션 진행)
```

Server 위치·collider·navigation은 발 위치 그대로다. 좌석 오프셋은 표현 루트에만 들어간다.

### `Client/Public/Part_Vehicle.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

#include <string>

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CPart_Vehicle final : public CPartObject
{
public:
	typedef struct tagPartVehicleDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;
		std::string strIdleClip;
		std::string strRunClip;
		std::string strSeatBone;
	} PART_VEHICLE_DESC;

private:
	CPart_Vehicle(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPart_Vehicle();

public:
	bool_t Set_Moving(bool_t isMoving);
	bool_t Try_Get_SeatWorldPosition(float3_t& outPosition) const;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	std::string m_strIdleClip;
	std::string m_strRunClip;
	std::string m_strSeatBone;
	bool_t m_isMoving = { false };

private:
	HRESULT Ready_Components(const PART_VEHICLE_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();

public:
	static unique_ptr<CPart_Vehicle> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### `Client/Private/Part_Vehicle.cpp` 전체 코드

```cpp
#include "Part_Vehicle.h"
#include "BinaryAsset/ModelAssetData.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"

#include <cmath>

CPart_Vehicle::CPart_Vehicle(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Vehicle::~CPart_Vehicle()
{
}

HRESULT CPart_Vehicle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Vehicle::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_VEHICLE_DESC*>(pArg);
	m_strIdleClip = pDesc->strIdleClip;
	m_strRunClip = pDesc->strRunClip;
	m_strSeatBone = pDesc->strSeatBone;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)) ||
		!m_pModelCom->Has_Bone(m_strSeatBone.c_str()) ||
		!m_pModelCom->Set_Animation(m_strIdleClip.c_str(), true))
	{
		return E_FAIL;
	}
	return S_OK;
}

bool_t CPart_Vehicle::Set_Moving(const bool_t isMoving)
{
	if (nullptr == m_pModelCom)
		return false;
	if (m_isMoving == isMoving)
		return true;
	m_isMoving = isMoving;
	return m_pModelCom->Set_Animation(
		(isMoving ? m_strRunClip : m_strIdleClip).c_str(), true);
}

bool_t CPart_Vehicle::Try_Get_SeatWorldPosition(float3_t& outPosition) const
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom ||
		nullptr == m_pParentMatrix || !m_pModelCom->Has_Bone(m_strSeatBone.c_str()))
	{
		return false;
	}
	const matrix_t seat = m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(m_pParentMatrix);
	float3_t staged{};
	XMStoreFloat3(&staged, seat.r[3]);
	if (!std::isfinite(staged.x) || !std::isfinite(staged.y) || !std::isfinite(staged.z))
		return false;
	outPosition = staged;
	return true;
}

void CPart_Vehicle::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Vehicle::Update(f32_t fTimeDelta)
{
	m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPart_Vehicle::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
}

HRESULT CPart_Vehicle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		uint32_t materialPass = 0u;
		const auto* surface = m_pModelCom->Get_MaterialSurface(i);
		if (surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u ||
			 surface->sourceCharacter.program == 18u || surface->sourceCharacter.program == 19u ||
			 surface->sourceCharacter.program == 20u))
			materialPass = 6u;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(ANIMATED_SHADOW_PASS)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Ready_Components(const PART_VEHICLE_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShadowShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<CPart_Vehicle> CPart_Vehicle::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Vehicle>(new CPart_Vehicle(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartVehicle] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Vehicle::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Vehicle>(new CPart_Vehicle(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartVehicle] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
```

### `Client/Public/VehiclePresentationAssetService.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

class CVehiclePresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::uint32_t vehicleId);
	static bool_t Is_Ready(uint32_t iLevelIndex, std::uint32_t vehicleId);
	static wstring_t Get_ModelPrototypeTag(std::uint32_t vehicleId);
	static const std::string& Get_Status();
};

NS_END
```

### `Client/Private/VehiclePresentationAssetService.cpp` 전체 코드

```cpp
#include "VehiclePresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"

#include <map>
#include <mutex>
#include <set>
#include <string_view>

namespace
{
	std::mutex g_VehicleAssetMutex;
	std::map<uint32_t, std::set<std::uint32_t>> g_ReadyVehiclesByLevel;
	std::string g_VehicleAssetStatus;

	HRESULT Reject(const std::string& reason)
	{
		g_VehicleAssetStatus = reason;
		OutputDebugStringA(("[Client][VehiclePresentation] " + reason + "\n").c_str());
		return E_FAIL;
	}

	bool_t Has_Clip(const Engine::CModel& model, const std::string_view clip)
	{
		for (uint32_t index = 0u; index < model.Get_NumAnimations(); ++index)
		{
			const char_t* name = model.Get_AnimationName(index);
			if (nullptr != name && clip == name)
				return true;
		}
		return false;
	}
}

void Client::CVehiclePresentationAssetService::Begin_LevelLoad(const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_VehicleAssetMutex };
	g_ReadyVehiclesByLevel.erase(iLevelIndex);
}

HRESULT Client::CVehiclePresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::uint32_t vehicleId)
{
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;

	std::scoped_lock lock{ g_VehicleAssetMutex };
	if (g_ReadyVehiclesByLevel[iLevelIndex].contains(vehicleId))
		return S_FALSE;

	const VEHICLE_ACTOR_ENTRY* vehicle = CActorCatalog::Find_Vehicle(vehicleId);
	if (nullptr == vehicle)
		return Reject("Vehicle is not in the catalog: " + std::to_string(vehicleId));

	Engine::MODEL_ASSET_LOAD_DESC load;
	std::string materialStatus;
	if (!CActorCatalog::Build_ModelLoadDescription(vehicle->modelAssetId, load, materialStatus))
		return Reject("Vehicle material input failed: " + materialStatus);

	unique_ptr<Engine::CModel> model = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, load,
		XMMatrixScaling(vehicle->modelPreScale, vehicle->modelPreScale, vehicle->modelPreScale) *
		XMMatrixRotationY(XMConvertToRadians(-90.f)));
	if (nullptr == model || 0u == model->Get_NumMeshes() || !model->Has_Animations())
		return Reject("Vehicle model has no usable animated geometry: " + vehicle->modelAssetId);
	if (!model->Has_Bone(vehicle->seatBone.c_str()) ||
		!Has_Clip(*model, vehicle->vehicleIdleClip) ||
		!Has_Clip(*model, vehicle->vehicleRunClip))
	{
		return Reject("Vehicle model is missing its seat bone or idle/run clips: " + vehicle->modelAssetId);
	}
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex, Get_ModelPrototypeTag(vehicleId), std::move(model))))
	{
		return Reject("Vehicle model prototype registration failed: " + vehicle->modelAssetId);
	}

	g_ReadyVehiclesByLevel[iLevelIndex].insert(vehicleId);
	g_VehicleAssetStatus.clear();
	return S_OK;
}

bool_t Client::CVehiclePresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex, const std::uint32_t vehicleId)
{
	std::scoped_lock lock{ g_VehicleAssetMutex };
	const auto level = g_ReadyVehiclesByLevel.find(iLevelIndex);
	return g_ReadyVehiclesByLevel.end() != level && level->second.contains(vehicleId);
}

wstring_t Client::CVehiclePresentationAssetService::Get_ModelPrototypeTag(
	const std::uint32_t vehicleId)
{
	return wstring_t(TEXT("Prototype_Component_Model_Vehicle_")) + std::to_wstring(vehicleId);
}

const std::string& Client::CVehiclePresentationAssetService::Get_Status()
{
	return g_VehicleAssetStatus;
}
```

### `Client/Public/Character.h`

기준점: `void Apply_NetworkPresentationHidden(bool_t hidden) { m_isNetworkPresentationHidden = hidden; }` 바로 아래 추가.

```cpp
	/* Replication hands over the replicated vehicle. Zero dismounts. A vehicle
	whose presentation is not admitted leaves the character on foot and logs
	once; gameplay truth stays on the Server either way. */
	void Apply_NetworkVehicle(std::uint32_t vehicleId);
```

기준점: `bool_t m_isNetworkPresentationHidden = false;` 바로 아래 추가.

```cpp
	std::uint32_t m_iVehicleId = 0u;
	std::uint32_t m_iRejectedVehicleId = 0u;
	shared_ptr<class CPart_Vehicle> m_pVehiclePart;
	// Vehicle part parent: the ground transform without the seat lift or class scale.
	float4x4_t m_VehicleRootMatrix = {};
	// World-space lift from the ground transform to the vehicle seat bone.
	float3_t m_vVehicleSeatOffset = {};
```

기준점: private `const char_t* Resolve_LocomotionClip(CHARACTER_ANIM eAnim) const;` 바로 아래 추가.

```cpp
	const VEHICLE_RIDER_ENTRY* Find_VehicleRider() const;
```

### `Client/Private/Character.cpp`

기준점: `#include "Part_Equipment.h"` 바로 아래 추가.

```cpp
#include "Part_Vehicle.h"
#include "VehiclePresentationAssetService.h"
```

기준점: `Try_Get_PresentationRootMatrix` 본문의 `XMStoreFloat4x4(pOut, …);` 문장 교체.

```cpp
	XMStoreFloat4x4(pOut,
		XMMatrixScaling(m_fPresentationScale, m_fPresentationScale, m_fPresentationScale) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
		XMMatrixTranslation(m_vVehicleSeatOffset.x, m_vVehicleSeatOffset.y, m_vVehicleSeatOffset.z));
```

기준점: `void CCharacter::Update_PresentationRootMatrix()` 정의 전체 교체.

```cpp
void CCharacter::Update_PresentationRootMatrix()
{
	m_vVehicleSeatOffset = {};
	if (nullptr != m_pTransformCom)
	{
		m_VehicleRootMatrix = *m_pTransformCom->Get_WorldMatrixPtr();
		float3_t seat{};
		if (nullptr != m_pVehiclePart && m_pVehiclePart->Try_Get_SeatWorldPosition(seat))
		{
			m_vVehicleSeatOffset = float3_t(
				seat.x - m_VehicleRootMatrix._41,
				seat.y - m_VehicleRootMatrix._42,
				seat.z - m_VehicleRootMatrix._43);
		}
	}
	Try_Get_PresentationRootMatrix(&m_PresentationRootMatrix);
}
```

기준점: `const char_t* CCharacter::Resolve_LocomotionClip(const CHARACTER_ANIM eAnim) const` 여는 `{` 바로 아래 추가.

```cpp
	if (CHARACTER_ANIM::IDLE == eAnim || CHARACTER_ANIM::RUN == eAnim)
	{
		if (const VEHICLE_RIDER_ENTRY* pRider = Find_VehicleRider())
			return (CHARACTER_ANIM::IDLE == eAnim ? pRider->idleClip : pRider->runClip).c_str();
	}
```

기준점: `Resolve_LocomotionClip` 정의 닫는 `}` 바로 아래 추가.

```cpp

const VEHICLE_RIDER_ENTRY* CCharacter::Find_VehicleRider() const
{
	if (0u == m_iVehicleId || nullptr == m_pVehiclePart)
		return nullptr;
	const VEHICLE_ACTOR_ENTRY* pVehicle = CActorCatalog::Find_Vehicle(m_iVehicleId);
	return nullptr != pVehicle ? pVehicle->Find_Rider(m_eCharacterClass) : nullptr;
}

void CCharacter::Apply_NetworkVehicle(const std::uint32_t vehicleId)
{
	static const wstring_t VEHICLE_PART_TAG = TEXT("Part_Vehicle");
	if (vehicleId == m_iVehicleId || vehicleId == m_iRejectedVehicleId)
		return;
	const auto reject = [this, vehicleId](const std::string& reason)
	{
		m_iRejectedVehicleId = vehicleId;
		OutputDebugStringA(("[Client][Character] Vehicle " + std::to_string(vehicleId) +
			" presentation isolated: " + reason + "\n").c_str());
	};

	PART_OBJECT_MAP candidates;
	shared_ptr<CPart_Vehicle> pPart;
	if (0u != vehicleId)
	{
		const VEHICLE_ACTOR_ENTRY* pVehicle = CActorCatalog::Find_Vehicle(vehicleId);
		if (nullptr == pVehicle || nullptr == pVehicle->Find_Rider(m_eCharacterClass))
			return reject("no rider pose for this class");
		if (!CVehiclePresentationAssetService::Is_Ready(m_iPrototypeLevelIndex, vehicleId))
			return reject("vehicle prototypes are not admitted in this level");
		CPart_Vehicle::PART_VEHICLE_DESC desc{};
		desc.pParentMatrix = &m_VehicleRootMatrix;
		desc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		desc.strModelTag = CVehiclePresentationAssetService::Get_ModelPrototypeTag(vehicleId);
		desc.strShaderTag = m_pSpec->pShaderTag;
		desc.strIdleClip = pVehicle->vehicleIdleClip;
		desc.strRunClip = pVehicle->vehicleRunClip;
		desc.strSeatBone = pVehicle->seatBone;
		shared_ptr<CPartObject> pObject;
		if (FAILED(__super::Clone_PartObject(m_iPrototypeLevelIndex,
				TEXT("Prototype_GameObject_Part_Vehicle"), &desc, pObject)) ||
			nullptr == (pPart = dynamic_pointer_cast<CPart_Vehicle>(pObject)))
		{
			return reject("vehicle part clone failed");
		}
		candidates.emplace(VEHICLE_PART_TAG, pObject);
	}
	if (FAILED(__super::Replace_PartObjectGroup(VEHICLE_PART_TAG, std::move(candidates))))
		return reject("vehicle part group replacement failed");

	m_iVehicleId = vehicleId;
	m_iRejectedVehicleId = 0u;
	m_pVehiclePart = pPart;
	if (nullptr != m_pVehiclePart)
		(void)m_pVehiclePart->Set_Moving(m_isMoving);
	Update_PresentationRootMatrix();
	if (!Is_PlayingSkill())
		Set_Animation(m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE, true);
}
```

기준점: `void CCharacter::Commit_Locomotion(bool_t isMoving)`의 `m_isMoving = isMoving;` 바로 아래 추가.

```cpp
	if (nullptr != m_pVehiclePart)
		(void)m_pVehiclePart->Set_Moving(isMoving);
```

`m_iRejectedVehicleId`는 같은 id를 매 snapshot 다시 시도해 로그를 쏟지 않게 막는다. 다른 id나 0이 오면 다시 시도한다.

### `Client/Private/ClientReplication.cpp`

기준점: `Apply_PlayerSnapshot`의 `character->Apply_NetworkStance(player.eStance);` 바로 아래 추가.

```cpp
	character->Apply_NetworkVehicle(player.iVehicleId);
```

### `Client/Private/Loader.cpp`

기준점: `#include "Part_Equipment.h"` 바로 아래 추가.

```cpp
#include "Part_Vehicle.h"
#include "VehiclePresentationAssetService.h"
```

기준점: `Ready_Character_Shared_Prototypes`의 `Prototype_GameObject_Part_Body` 등록 3줄(`CPart_Body::Create(m_pDevice, m_pContext))) ||`까지) 바로 아래 추가.

```cpp
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Part_Vehicle"),
			CPart_Vehicle::Create(m_pDevice, m_pContext))) ||
```

기준점: `Ready_For_Bern`의 `Set_Status(TEXT("Bern loading complete"));` 바로 위 추가.

```cpp
	Set_Status(TEXT("BERN: vehicle presentation"));
	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::BERN), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] BERN vehicle " +
				std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n").c_str());
		}
	}
```

기준점: `Ready_For_CharacterSelect`의 `Set_Status(TEXT("Character Select loading complete"));` 바로 위에 같은 블록을 `LEVEL::CHARACTER_SELECT`, 로그 접두 `CHARACTER SELECT`로 추가.

### `Client/Default/Client.vcxproj`

기준점 `<ClInclude Include="..\Public\Part_Body.h" />` 아래 `<ClInclude Include="..\Public\Part_Vehicle.h" />`,
`<ClCompile Include="..\Private\Part_Body.cpp" />` 아래 `<ClCompile Include="..\Private\Part_Vehicle.cpp" />`,
`<ClInclude Include="..\Public\NpcPresentationAssetService.h" />` 아래 `<ClInclude Include="..\Public\VehiclePresentationAssetService.h" />`,
`<ClCompile Include="..\Private\NpcPresentationAssetService.cpp" />` 아래 `<ClCompile Include="..\Private\VehiclePresentationAssetService.cpp" />`.

### `Client/Default/Client.vcxproj.filters`

```xml
    <ClCompile Include="..\Private\Part_Vehicle.cpp">
      <Filter>02.GameObjects\00. Character</Filter>
    </ClCompile>
    <ClInclude Include="..\Public\Part_Vehicle.h">
      <Filter>02.GameObjects\00. Character</Filter>
    </ClInclude>
	<ClCompile Include="..\Private\VehiclePresentationAssetService.cpp">
	  <Filter>04. Network</Filter>
	</ClCompile>
	<ClInclude Include="..\Public\VehiclePresentationAssetService.h">
	  <Filter>04. Network</Filter>
	</ClInclude>
```

(각각 `Part_Body.cpp`, `Part_Body.h`, `NpcPresentationAssetService.cpp`, `NpcPresentationAssetService.h` 블록 바로 아래)

---

## G08. H 키 명령

### `Client/Public/PlayerCommandSink.h`

기준점: `Consume_DebugMadnessFormResult(…) = 0;` 두 줄 바로 아래 추가.

```cpp
	/* H key riding toggle: a vehicle id mounts, INVALID_VEHICLE_ID dismounts.
	Sinks without a Server reject it. */
	virtual bool Request_SetVehicleRiding(std::uint32_t, LostArk::Shared::VEHICLE_ID) { return false; }
	virtual bool Consume_VehicleRidingResult(
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT&) { return false; }
```

### `Client/Public/NetworkPlayerCommandSink.h`

기준점: `Consume_DebugMadnessFormResult(…) override;` 두 줄 바로 아래 추가.

```cpp
	bool Request_SetVehicleRiding(
		std::uint32_t requestSequence,
		LostArk::Shared::VEHICLE_ID vehicleId) override;
	bool Consume_VehicleRidingResult(
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT& result) override;
```

### `Client/Private/NetworkPlayerCommandSink.cpp`

기준점: `Consume_DebugMadnessFormResult` 정의 닫는 `}` 바로 아래 추가.

```cpp

bool Client::CNetworkPlayerCommandSink::Request_SetVehicleRiding(
	const std::uint32_t requestSequence,
	const LostArk::Shared::VEHICLE_ID vehicleId)
{
	return CNetworkManager::Get().Send_SetVehicleRiding(requestSequence, vehicleId);
}

bool Client::CNetworkPlayerCommandSink::Consume_VehicleRidingResult(
	LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT& result)
{
	return CNetworkManager::Get().Try_Consume_VehicleRidingResult(result);
}
```

### `Client/Public/NetworkManager.h`

기준점: `Try_Consume_DebugMadnessFormResult(…);` 두 줄 바로 아래 추가.

```cpp
	/* H key riding toggle. The snapshot is the only presentation result. */
	bool Send_SetVehicleRiding(
		std::uint32_t requestSequence, LostArk::Shared::VEHICLE_ID vehicleId);
	bool Try_Consume_VehicleRidingResult(
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT& result);
```

기준점: `m_DebugMadnessFormResults;` 멤버 바로 아래 추가.

```cpp
	std::deque<LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT> m_VehicleRidingResults;
```

### `Client/Private/NetworkManager.cpp`

기준점: `CNetworkManager::Send_DebugSetMadnessForm` 정의 닫는 `}` 바로 아래 추가.

```cpp

bool CNetworkManager::Send_SetVehicleRiding(
	const std::uint32_t requestSequence,
	const LostArk::Shared::VEHICLE_ID vehicleId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected() || !Is_Known_World_Id(m_eWorldId) ||
		INVALID_PLAYER_ID == m_iLocalPlayerId)
		return false;
	C2S_SET_VEHICLE_RIDING message{};
	message.iRequestSequence = requestSequence;
	message.eWorldId = m_eWorldId;
	message.iVehicleId = vehicleId;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	std::vector<std::uint8_t> frame;
	return Build_Packet_Frame(PACKET_TYPE::C2S_SET_VEHICLE_RIDING,
		writer.Get_Buffer(), frame) && Send_All(frame);
}
```

기준점: `CNetworkManager::Try_Consume_DebugMadnessFormResult` 정의 닫는 `}` 바로 아래 추가.

```cpp

bool CNetworkManager::Try_Consume_VehicleRidingResult(
	LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT& result)
{
	if (m_VehicleRidingResults.empty())
		return false;
	result = m_VehicleRidingResults.front();
	m_VehicleRidingResults.pop_front();
	return true;
}
```

기준점: 세션 정리의 `m_DebugMadnessFormResults.clear();` 바로 아래 `m_VehicleRidingResults.clear();` 추가.

기준점: 수신 switch의 `case PACKET_TYPE::S2C_DEBUG_SET_MADNESS_FORM_RESULT:` 블록(`break;` `}`) 바로 아래 추가.

```cpp
	case PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT:
	{
		S2C_SET_VEHICLE_RIDING_RESULT result{};
		if (!Read_Message(reader, result) || 0u != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (result.eWorldId != m_eWorldId)
			break;
		if (m_VehicleRidingResults.size() >= MAX_REVISION_CONTROL_QUEUE)
		{
			Fail_Protocol(WSAENOBUFS);
			return;
		}
		m_VehicleRidingResults.push_back(result);
		break;
	}
```

### `Client/Public/CombatHUDViewModel.h` / `Client/Private/CombatHUDViewModel.cpp`

기준점 `LostArk::Shared::PLAYER_MADNESS_FORM eMadnessForm =` 두 줄 바로 아래:

```cpp
		// The vehicle the Server has this player riding; 0 on foot.
		std::uint32_t iVehicleId = 0u;
```

기준점 `m_Player.eMadnessForm = snapshot.eMadnessForm;` 바로 아래:

```cpp
	m_Player.iVehicleId = snapshot.iVehicleId;
```

### `Client/Public/PlayerController.h`

기준점: public `bool_t Request_Revive();` 바로 아래 추가.

```cpp
		/* Last H key riding verdict, for a status line; empty until one arrives. */
		const std::string& Get_VehicleRidingStatus() const { return m_vehicleRidingStatus; }
```

기준점: private `bool_t Poll_InteractKey(` 선언(세 줄) 바로 위 추가.

```cpp
		/* Consumes riding verdicts and turns an H press into a mount or dismount
		intent. The first catalog vehicle with a rider pose for the class mounts. */
		void Update_VehicleRiding(bool_t inputAllowed, bool_t useRawKeyboard);
```

기준점: `bool_t m_wasInteractKeyDown = false;` 바로 아래 추가.

```cpp
		bool_t m_wasVehicleKeyDown = false;
		std::uint32_t m_nextVehicleRidingSequence = 1u;
		std::uint32_t m_pendingVehicleRidingSequence = 0u;
		std::chrono::steady_clock::time_point m_vehicleRidingSentAt{};
		std::string m_vehicleRidingStatus;
```

### `Client/Private/PlayerController.cpp`

기준점: `#include "Character.h"` 바로 위에 `#include "ActorCatalog.h"` 추가.

기준점: `Set_LocalCharacter`의 `m_wasRightMouseDown = false;` 바로 아래 `m_wasVehicleKeyDown = false;` 추가.

기준점: `Set_CommandSink`의 첫 `if (m_pCommandSink != commandSink)` 블록 교체.

```cpp
	if (m_pCommandSink != commandSink)
	{
		m_iLastMarioMoveDirection = 0;
		m_pendingVehicleRidingSequence = 0u;
	}
```

기준점: `Update`의 `const bool_t marioControlsActive = Update_MarioControls(gameplayCommandsEnabled);` 바로 아래 추가.

```cpp
	{
		const bool_t useRawVehicleKeyboard =
			m_allowCapturedKeyboardInput && GetForegroundWindow() == g_hWnd;
		const bool_t vehicleInputAllowed = gameplayCommandsEnabled && !marioControlsActive &&
			!m_GroundTargeting.Is_Active() &&
			!Is_PlayerControlCaptured(CCombatHUDViewModel::Get().Get_Player()) &&
			GetForegroundWindow() == g_hWnd &&
			!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive() &&
			(useRawVehicleKeyboard || !CGameInstance::Get().IsKeyboardInputBlocked());
		Update_VehicleRiding(vehicleInputAllowed, useRawVehicleKeyboard);
	}
```

기준점: `Update`의 `Poll_SkillSlots(` 호출(`suppressKeyboard || !gameplayCommandsEnabled,` 인자) 바로 위에 추가하고 두 인자를 교체.

```cpp
	const bool_t isMounted = 0u != CCombatHUDViewModel::Get().Get_Player().iVehicleId;
```

```cpp
		suppressKeyboard || !gameplayCommandsEnabled || isMounted,
```

기준점: `const std::uint8_t estherSlot = Poll_EstherSlot(` 다음 줄 교체.

```cpp
		suppressKeyboard || !gameplayCommandsEnabled || isMounted, useRawKeyboard);
```

기준점: `Client::CPlayerController::Poll_InteractKey` 정의 바로 위에 추가.

```cpp
void Client::CPlayerController::Update_VehicleRiding(
	const bool_t inputAllowed,
	const bool_t useRawKeyboard)
{
	using namespace LostArk::Shared;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	while (nullptr != m_pCommandSink && m_pCommandSink->Consume_VehicleRidingResult(result))
	{
		if (0u == m_pendingVehicleRidingSequence ||
			result.iRequestSequence != m_pendingVehicleRidingSequence)
			continue;
		m_pendingVehicleRidingSequence = 0u;
		switch (result.eResult)
		{
		case VEHICLE_RIDING_RESULT::ACCEPTED:
			m_vehicleRidingStatus = 0u == result.iActiveVehicleId ? "Dismounted." : "Mounted."; break;
		case VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED:
			m_vehicleRidingStatus = "Vehicles cannot be ridden in this area."; break;
		case VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE:
			m_vehicleRidingStatus = "Cannot mount right now."; break;
		case VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE:
			m_vehicleRidingStatus = "The Server does not know this vehicle."; break;
		case VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE:
			break;
		default:
			m_vehicleRidingStatus = "The Server rejected the riding request."; break;
		}
	}
	const auto now = std::chrono::steady_clock::now();
	if (0u != m_pendingVehicleRidingSequence &&
		now - m_vehicleRidingSentAt > std::chrono::seconds(5))
	{
		m_pendingVehicleRidingSequence = 0u;
	}

	const int8_t state = m_CaptureInputGate.Is_Blocked(DIK_H) ?
		static_cast<int8_t>(0) :
		(useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(DIK_H) :
			CGameInstance::Get().Get_DIKeyState(DIK_H));
	const bool_t isDown = 0 != (state & 0x80);
	const bool_t pressed = inputAllowed && isDown && !m_wasVehicleKeyDown;
	m_wasVehicleKeyDown = isDown;
	if (!pressed || 0u != m_pendingVehicleRidingSequence || nullptr == m_pCommandSink)
		return;

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const shared_ptr<CCharacter> character = m_pLocalCharacter.lock();
	if (!player.isValid || player.isPreview || nullptr == character)
		return;
	VEHICLE_ID requested = INVALID_VEHICLE_ID;
	if (INVALID_VEHICLE_ID == player.iVehicleId)
	{
		for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
		{
			if (nullptr != vehicle.Find_Rider(character->Get_CharacterClass()))
			{
				requested = vehicle.vehicleId;
				break;
			}
		}
		if (INVALID_VEHICLE_ID == requested)
		{
			m_vehicleRidingStatus = "No vehicle has a riding pose for this class yet.";
			return;
		}
	}
	if (!m_pCommandSink->Request_SetVehicleRiding(m_nextVehicleRidingSequence, requested))
	{
		m_vehicleRidingStatus = "Could not send the riding request.";
		return;
	}
	m_pendingVehicleRidingSequence = m_nextVehicleRidingSequence;
	m_vehicleRidingSentAt = now;
	if (0u == ++m_nextVehicleRidingSequence)
		m_nextVehicleRidingSequence = 1u;
}

```

H는 `SlotKeys`, Esther(Ctrl+Z/X/C), 상호작용 G, Mario 방향키와 겹치지 않는다(`DIK_H` 사용처 0건).

구현 시 변경: `Get_VehicleRidingStatus()`/`m_vehicleRidingStatus`는 소비자가 없어 넣지 않고, 같은 문구를 익명 namespace `Log_VehicleRiding`으로 `OutputDebugStringA`(`[Client][VehicleRiding]`)에 남긴다.

---

## G09. Effect Tool V2 Attach 이전

### `Client/Public/NpcPresentationAssetService.h`

기준점: `static bool_t Is_Ready(` 선언 바로 위 추가.

```cpp
	/* Registers the shared CNpc GameObject prototype once per level for a body
	that is not an NpcCatalog archetype (a vehicle previewed in a tool). */
	static HRESULT Ensure_ObjectPrototype(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex);
```

### `Client/Private/NpcPresentationAssetService.cpp`

기준점: `Client::CNpcPresentationAssetService::Is_Ready` 정의 바로 위 추가.

```cpp
HRESULT Client::CNpcPresentationAssetService::Ensure_ObjectPrototype(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex)
{
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	std::scoped_lock lock{ g_NpcAssetMutex };
	if (g_NpcObjectReadyLevels.contains(iLevelIndex))
		return S_FALSE;
	auto prototype = CNpc::Create(pDevice, pContext);
	if (nullptr == prototype || FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex, TEXT("Prototype_GameObject_Npc"), std::move(prototype))))
	{
		return E_FAIL;
	}
	g_NpcObjectReadyLevels.insert(iLevelIndex);
	return S_OK;
}

```

### `Client/Public/Effect_Tool_V2.h`

기준점: `bool_t Spawn_NpcTarget(const std::string& strArchetypeId, const float3_t& vPosition);` 바로 아래 추가.

```cpp
	bool_t Spawn_VehicleTarget(const std::string& strArchetypeId, const float3_t& vPosition);
```

### `Client/Private/Effect_Tool_V2.cpp`

기준점 `#include "NpcPresentationAssetService.h"` 바로 아래 `#include "VehiclePresentationAssetService.h"` 추가.

기준점: `Spawn_Target`의 `else` + `bSpawned = Spawn_NpcTarget(strArchetypeId, vPosition);` 두 줄 교체.

```cpp
	else if (nullptr != CActorCatalog::Find_VehicleByArchetype(strArchetypeId))
		bSpawned = Spawn_VehicleTarget(strArchetypeId, vPosition);
	else
		bSpawned = Spawn_NpcTarget(strArchetypeId, vPosition);
```

기준점: `Spawn_NpcTarget` 정의 닫는 `}` 바로 아래 추가.

```cpp

bool_t Client::CEffect_Tool_V2::Spawn_VehicleTarget(
	const std::string& strArchetypeId,
	const float3_t& vPosition)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const VEHICLE_ACTOR_ENTRY* pVehicle = CActorCatalog::Find_VehicleByArchetype(strArchetypeId);
	if (nullptr == pVehicle)
	{
		m_strAttachStatus = "Unknown vehicle archetype: " + strArchetypeId;
		return false;
	}
	if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, iLevel, pVehicle->vehicleId)) ||
		FAILED(CNpcPresentationAssetService::Ensure_ObjectPrototype(m_pDevice, m_pContext, iLevel)))
	{
		m_strAttachStatus = "Vehicle presentation prototypes failed: " + strArchetypeId + " " +
			CVehiclePresentationAssetService::Get_Status();
		return false;
	}

	CNpc::NPC_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.strModelTag = CVehiclePresentationAssetService::Get_ModelPrototypeTag(pVehicle->vehicleId);
	Desc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
	Desc.pIdleClip = pVehicle->vehicleIdleClip.c_str();
	Desc.isLoop = true;
	Desc.vPosition = vPosition;
	Desc.fYawDegree = 0.f;
	Desc.fCollisionRadius = 0.f;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Npc"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Target spawn failed: " + strArchetypeId;
		return false;
	}
	const std::shared_ptr<CNpc> pNpc = std::dynamic_pointer_cast<CNpc>(pGameObject);
	if (nullptr == pNpc || nullptr == pNpc->Get_Model())
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	m_Target = EFFECT_V2_TARGET::From_Npc(pNpc);

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pVehicle->modelAssetId, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pNpc->Get_Model()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}
	return true;
}
```

기준점: Attach 창 콤보의 NPC 루프(`for (const NPC_ACTOR_ENTRY& Entry : Npcs)` 블록) 닫는 `}` 바로 아래, `ImGui::EndCombo();` 위 추가.

```cpp
		ImGui::Separator();
		for (const VEHICLE_ACTOR_ENTRY& Entry : CActorCatalog::Get_Vehicles())
		{
			const std::string strLabel = Entry.archetypeId + "  (vehicle " +
				std::to_string(Entry.vehicleId) + ")";
			if (ImGui::Selectable(strLabel.c_str(), Entry.archetypeId == m_strSelectedArchetypeId))
				m_strSelectedArchetypeId = Entry.archetypeId;
		}
```

기준점: `SeparatorText("Target (NPC archetype / Valtan / KoukuSaydon body)")` 문자열을 `"Target (NPC / vehicle / Valtan / KoukuSaydon body)"`로 교체.

### `Client/Private/EffectV2_Runtime.cpp`

기준점: `#include` 목록의 `NpcPresentationAssetService.h` 바로 아래 `#include "VehiclePresentationAssetService.h"` 추가.

기준점: `Resolve_Archetype`의 NPC 태그 루프(`for (const Client::NPC_ACTOR_ENTRY& Entry : …)` 블록) 닫는 `}` 바로 아래 추가.

```cpp
			for (const Client::VEHICLE_ACTOR_ENTRY& Entry : Client::CActorCatalog::Get_Vehicles())
			{
				const wstring_t strTag =
					Client::CVehiclePresentationAssetService::Get_ModelPrototypeTag(Entry.vehicleId);
				if (!g_ModelTagToArchetype.contains(strTag))
					g_ModelTagToArchetype.emplace(strTag, Entry.archetypeId);
			}
```

기존 archetype 이름 `VEHICLE_TERPEION_GOLD`를 그대로 쓰므로 Effect V2 binding 파일 경로가 바뀌지 않는다(현재 해당 binding 0개).

---

## G10. 문서와 검증

### 문서 갱신

- `CLAUDE.md`: protocol v81 표기 3곳 → v82. "새 GameObject"와 무관한 새 절: `### 탈것` — H 토글, Bern/Character Select만 허용,
  탑승 중 스킬 거부, `Data/Actors/VehicleCatalog.json`(표현)·`Data/Vehicles/VehicleProfiles.json`(Server 속도) 정본,
  `Publish-VehicleProfiles.ps1`, 4직업 `…_RideHorseAnimSet.wmodel`, program 85.
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: protocol 81 표기 5곳 → 82, 입력 표에 H 추가.
- `.md/GB/렌더링이펙트복원V2.md`: 연결 범위 표에 "황금 테르페이온 program 85 / 24 / 18" 행.
- RESULT: `.md/JS/09-13/2026-09-13_TERPEION_VEHICLE_RIDING_RESULT.md`.

### 자동 검증 (에이전트)

| 순서 | 명령 | 통과 기준 |
|---|---|---|
| 1 | `build_vehicle_source_material.py verify … --program 24` | base/light/configure EXACT |
| 2 | generate 결과 SHA-256 | G01 표와 일치 |
| 3 | JSON parse: `VehicleCatalog.json`, `VehicleProfiles.json`, `NpcCatalog.json`, `CharacterCatalog.json` | ConvertFrom-Json 성공, override 3행/파라미터 115/텍스처 18 |
| 4 | `Publish-VehicleProfiles.ps1 -Mode Validate` 후 `-Mode Publish` | `Vehicles.bootstrap` 1행 `VEHICLE 6705 5` |
| 5 | 4직업 animset `validate_wmodel.py` + `compare_attach.py` | OK, skeletonHash 일치 |
| 6 | Product Debug 빌드 `Invoke-BuildAndRegression.ps1 -Configuration Debug` | Engine/Shared/Server/Client exit 0, `Shader_Deferred`/Client FX CSO 갱신 |
| 7 | `NetworkProtocolHarness` 빌드·실행 | failures 0 |
| 8 | `Server.exe --vehicle-riding-contract-test` | `vehicle riding failures: 0` |
| 9 | `Server.exe --contract-test` 중 `Run_CharacterAdmission`/`Run_GroundTarget` 구간 | 기존 실패 수 증가 없음 |
| 10 | `git diff --check` | 경고 0 |

### 사용자 확인 (에이전트는 실행·캡처하지 않음)

1. Server/Client 둘 다 새 빌드로 재시작 → Lobby → Bern 입장 → H: 말이 발밑에 나타나고 캐릭터가 등에 앉는지.
2. 우클릭 이동: 말 달리기 + 탑승자 run 클립, 속도가 도보보다 빠른지(5 m/s).
3. 탑승 중 Q/LMB: 아무 것도 나가지 않음. H 다시: 하차.
4. Character Select에서 같은 동작, 직업 썸네일 변경 시 하차.
5. 탑승 상태로 발탄 입장 NPC → 입장 후 도보.
6. 말 재질: 금색 장식, 몸통 IBL 반사, 발광 깜빡임, 갈기 투명 두께가 원작과 가까운지(최종 판정은 사용자).

### 남는 경계 (이번 범위 밖)

- 탈것 고유 스킬 96000/96010/96020/96030(dash 등)과 탑승·하차 클립(`npc_on`, `Ride_Respawn_2/3`), 소환 파티클.
- Gunslinger/Slayer 탑승자 애니셋.
- 여러 탈것 중 선택 UI(현재는 catalog 첫 탈것).
- 탈것 전용 LookInfo 파티클(`Par_D_PMSTG_00-3_Default_01~05`)과 발굽 사운드.
