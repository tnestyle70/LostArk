# Lost Ark UE3 Level Placement Extractor

Lost Ark 레벨 패키지에서 `StaticMeshActor / InterpActor /
StaticMeshCollectionActor -> StaticMeshComponent -> StaticMesh` 연결과 원본
Transform을 JSON으로 복구하는 읽기 전용 도구다.

## 팀 재추출 순서: 가시성·기하·재질을 함께 보존

1. `extract_ue3_placements.py`의 schema3 출력을 사용한다. 실제 actor/component와
   archetype/CDO의 `sourceVisibility`가 visible을 결정한다. `LV_MODULE`, nav, water,
   FX 이름은 진단 힌트이며 숨김 조건이 아니다. navigation 참여와 가시성은 별개다.
2. 기존 Bern/map variant cook은 `cook_wmodel_geometry_contract.py`를 호출해 source
   glTF의 UV1/UV2, tangent.w, COLOR0를 WModel에 보존한다. 원본 UPK→glTF에서 이미
   사라진 채널까지 생성하거나 원본과 동일하다고 인증하는 기능은 아니다. 필요한 채널이
   없으면 source 추출 단계로 돌아가며 UV0 복제로 UV1을 대신하지 않는다.
3. 아래 공용 parameter 추출기와 mip 회수기를 사용한다. source material 전체 경로,
   실제 slot, texture 색공간·hash·원본 mip 수, component RNM·환경 근거를 묶는다.
4. `build_source_map_materials.py`가 기존 Area `mapmaterials` JSON을 생성한다. 생성 receipt를
   `build_maptool_scene.py --source-materials-receipt <receipt> --materials-output <authoring-json> --runtime-asset-root Client/Bin/Resources`에
   전달한다. scene은 입력·Resources·출력 hash와 실제 asset/slot/MIC를 검사하고 동일 compiler로
   재생성해 receipt를 대조한다. legacy 미지원 필드는 정확한 필드별 coverage만 보완한다.
5. 기존 [Area publisher](../MapPipeline/README.md)로 Validate → Publish → Check한다.
   새 재질이 생성된 것, 실제 CModel이 읽은 것, 사용자 화면이 원작과 같은 것은 별도 결과다.

구버전 placement의 가시성 근거 부재, 재질 미지원, 파일 누락은 숨김으로 처리하지 않는다.
`--allow-legacy-visibility`, `--allow-partial-material-preview`는 명시적 geometry 확인용이며
원본 복원 완료를 뜻하지 않는다. 실제 CLI의 `--help`와 아래 지원 범위를 함께 확인한다.

### Source material compiler 입력

```powershell
python Tools\LevelPlacementExtractor\build_source_map_materials.py `
  --input out\MapMaterials\inputs.json `
  --resources-root Client\Bin\Resources `
  --output out\MapMaterials\AREA.mapmaterials.json `
  --receipt out\MapMaterials\build.receipt.json
```

입력 JSON의 `format`은 `lostark-source-map-material-input`, `formatVersion`은1이다.

| 필드 | 내용 |
|---|---|
| `areaId` | 대상 Area stable ID |
| `parameters` | 공용 parameter 추출 결과의 `{path, sha256}` |
| `evidence[]` | 실제 slot/component 근거 파일의 `{path, sha256}`. 하나 이상 필수 |
| `textures[]` | `{sourceObject, assetId, sha256, colorSpace, mipCount, mipEvidence}`. full source object → Resources 상대 DDS |
| `auxiliaryTextures[]` | RNM/cube/BRDF의 같은 texture 항목. componentLighting 대조에 쓰는 RNM은 full `sourceObject`도 지정 |
| `componentLighting` | 공용 component lighting 추출 결과의 `{path,sha256}`. 지정하면 RNM source ID·texture pair·scale/bias를 원본 출력과 대조 |
| `slots[]` | `{assetId, materialName, sourceMaterial, component, textureFallbacks?}`. 동일 asset/slot 중복 거부 |
| `placementLighting[]` | 기존 mapmaterials의 sourcePlacementId·assetId와 coordinateScale/Bias·average/directional RGB scale |
| `projectApproximations[]` | 프로젝트 BRDF, 미확정 환경 각도 등 원작 추출값과 구분할 항목 |

`colorSpace`는 `linear`/`srgb`, `mipEvidence`는 `source-chain`/`source-unmipped`/
`project-generated`다. DDS payload 크기·실제 mip 수·hash를 확인한다. 원본 chain이라고
표시할 때 mip 회수 receipt를 evidence에 포함한다. header의 mip 수만으로 원본 chain
동일성을 증명할 수 없으며 source-unmipped는 원본부터 한 단계인 경우에만 사용한다.

`component`는 `rendering={castsShadow,renderMode,cullMode,sourceFlags?}`,
`lightingEvidence=source-bound|source-absent`, optional `bakedLighting`/`environment`를 가진다.
renderMode는 현재 `deferred`이며 cullMode는 `back`/`front`/`none`이다. PBR은
`environmentEvidence=source-bound|source-absent`, `minimumRoughness`,
`minimumRoughnessEvidence=source-bound|project-authored`도 명시한다. 환경이 미추출된 상태를
source-absent로 쓰지 않는다. RNM과 environment 객체는 기존 Area schema를 그대로 사용한다.
각 source RNM의 atlas pair가 다르면 variant asset ID를 분리하고, 같은 pair의 배치별
scale/bias는 placementLighting에 남긴다. 임의로 모든 인스턴스에 한 tile을 복사하지 않는다.

`rendering.sourceFlags`는 원본 `blendMode`, `twoSided`, `isMasked`, `disableDepthTest`,
`opacityMaskClipValue` 중 실제 확보한 값이다. compiler가 값과 지원 branch를 대조한
항목만 coverage에 넣는다. 이름을 모르는 flag나 읽지 않은 scalar/vector는 해결됐다고
표시하지 않는다. static switch가 OFF여도 다른 shader stage가 parameter를 소비할 수
있으므로 원본 selected permutation 근거 없이 일괄 inactive 처리하지 않는다.

명시적 null texture는 제거하지 않는다. 원본 native uniform expression에서 확인한 fallback만
`textureFallbacks.<parameter>={sourceObject, expressionIndex, evidence:{path,sha256}}`로 연결한다.
evidence JSON의 `sourceMaterial`과 `textures[]`의 expressionIndex/parameterName/sourceObject가
모두 일치해야 한다. leaf 이름이나 다른 모델의 texture를 자동 대입하지 않는다.

지원 surface는 기존 BG base/simple/overlay, 검증된 PBR, foliage/grass의 정적 surface,
snowice carrier다. 미지원 active switch, unresolved switch, foliage vertex wind, PBR normalmap
OFF는 오류로 반환한다. PBR emissive ON/flicker OFF는 `emissive.flicker.mode="none"`과
minimum/speed/phaseOffset=0으로 명시한다. mode 생략 또는 `nested`는 기존 시간식을 유지한다.
PBR metallic-mask diffuse는 양쪽 tint 일치, saturation=1, reflection OFF, 별도 metallic-mask
saturation OFF, 외부 nonmetallic/metallic 배율 일치인 경우에만 기존 두 배율로 정확히 변환한다.
이 조건 밖의 branch를 지원으로 확대하지 않는다. 미지원 branch를 검정 fallback으로 게시하지
않으며 scalar0은 보존한다. 별도로 저작한 프로젝트 밝기 보정은 원본 추출값과 구분한다.

성공 receipt는 `runtimeMaterialInputsComplete=true`와 별도로
`originalVisualFidelityVerified=false`를 가진다. source-only coverage는
`bindings[].materialCoverage[]`의 정확한 slot/MIC/field 목록이다. 원작의 동적 MIC 변경,
native BRDF/SH·hemisphere 입력과 최종 화면 동일성은 이 도구의 성공만으로 완료되지 않는다.
실패하면 기존 출력과 receipt를 보존하며 Resources와 제품 runtime을 직접 설치하지 않는다.

```powershell
python -B -m unittest discover -s Tools\LevelPlacementExtractor -p test_build_source_map_materials.py
```

### 추출 결과와 receipt의 동시 저장

공용 mip·parameter·component 추출기와 source material compiler는
`source_extraction_io.write_pair`를 사용한다. 검증한 출력과 receipt를 모두 stage한 뒤
교체하며 두 번째 교체가 OS 오류로 실패하면 이미 교체한 첫 출력도 원래 bytes 또는
원래의 파일 부재 상태로 되돌린다. 추출 자체가 실패했을 때 receipt만 기록하는 도구는
`write_atomic`을 사용하고 기존 정상 출력을 보존한다. 이는 보고된 I/O 오류의 rollback
계약이며 두 파일에 대한 전원 차단·process crash 복구를 보장하는 저장소는 아니다.

## 원본 component별 RNM·환경 입력 추출

`extract_source_map_component_lighting.py`는 한 원본 map UPK의
StaticMeshComponent를 읽고 `<logical-package>:export:<index0>`를 정본 키로
조명 근거를 만든다. 같은 mesh를 쓰는 배치들의 RNM atlas·UV·계수 차이를 보존한다.

```powershell
python Tools\LevelPlacementExtractor\extract_source_map_component_lighting.py `
  --package C:\source\Packages\756R7S99Q6LG72KKKU7UGR6RK744.upk `
  --logical-package LV_LOBBY_CLASSSELECT_SL00 `
  --area-id LV_LOBBY_CLASSSELECT_SL00 `
  --output out\MapLighting\components.json `
  --receipt out\MapLighting\components.receipt.json

python -B -m unittest discover -s Tools\LevelPlacementExtractor `
  -p test_extract_source_map_component_lighting.py -v
```

출력 `format=lostark-source-map-component-lighting`, `formatVersion=1`의
`components[sourcePlacementId]`는 `lighting`, `environment`, source mesh와
component/actor export index, 원본 serial/tail hash를 갖는다. KR v868의 단일 LOD,
shadow payload 없음, kind2 RNM을 지원한다. 평균/방향 texture full sourceObject,
세 coefficient의 scale, coordinateScale/Bias, baked Light GUID의 원본16byte,
optional BGRA8 vertex color의 count/hash와 정확한 terminal tail 소비를 확인한다.
`textures`에는 local texture export의 package/index/serial offset·hash 또는
external reference 미열람 상태를 기록한다. DDS를 추출·설치하지 않는다.

환경 color·angle과 map override는 태그 존재 여부 및 원본 값을 보존한다. 태그가
없으면 `INSTANCE_PROPERTY_ABSENT`이며 CDO/상속/renderer 기본값을 뜻하지 않는다.
minroughness의 실제 owner인 참조 TextureCube가 다른 package에 있으면
`EXTERNAL_TEXTURE_PROPERTY_NOT_READ`로 기록한다. source absence와 구별하며
임의0.04·색상white·angle0이나 shader CB의 alpha/floor를 주입하지 않는다.
같은 package의 TextureCube 태그는 원본을 직접 읽는다.

정확한4byte zeroLOD는 `NO_LOD_LIGHTING_DATA`다. vertex lightmap, shadow payload,
다중LOD, 세 번째 RNM texture와 미인식 terminal tail은
`UNSUPPORTED_NATIVE_LAYOUT`으로 분리하고 원문 nativeTailHex 및 해석한 prefix를
남긴다. 원본에 조명이 없다는 뜻으로 바꾸지 않는다. 잘못된 count·reference·비유한
수치 등 parse 실패가 있으면 기존 출력은 보존하고 receipt에 실패를 기록한다.

종료 코드는 전체 지원0, parse 실패1, 추출 성공이나 미지원 record 포함2다. 코드2도
구조화된 근거 출력은 생성한다. compiler는 선택한 component의 `status`를 확인해야
하며 미지원 record를 정상 입력으로 소비하면 안 된다. source SL00은803개 중
RNM799·zeroLOD3·vertex lightmap 미지원1이다. 이 도구는 기존 compiler가 사용할
source evidence를 생성하며 새 runtime이나 shader 복원을 추가하지 않는다.

## 명시한 원본 맵 재질의 유효 parameter 추출

`extract_source_map_material_parameters.py`는 full package.object 경로로 지정한
`Material`/`MaterialInstanceConstant`만 추출한다. mesh slot 선택은 기존 variant
manifest가 소유하며 이 도구가 basename이나 폴더 이름으로 추정하지 않는다.

```powershell
python Tools\LevelPlacementExtractor\extract_source_map_material_parameters.py `
  --source-material bg_pcselect15.mat.bg_elg_aryanorb_floor17_mi_ksr_02 `
  --source-material bg_pcselect14.mat.bg_vol_common_arch01a_02mi_feiyi `
  --package-root C:\source\Packages `
  --umodel C:\tools\umodel_lostark_v7.exe `
  --output out\MapMaterials\parameters.json `
  --receipt out\MapMaterials\parameters.receipt.json

python -B -m unittest discover -s Tools\LevelPlacementExtractor `
  -p test_extract_source_map_material_parameters.py -v
```

`--source-material`을 반복하거나 full path 문자열 배열 JSON을
`--source-materials-json`으로 전달한다. 출력은
`format=lostark-source-map-material-parameters`, `formatVersion=1`,
`materials`, `sources`, `failures`를 가진다. 각 Material은 `terminal`, `baseId`,
`values`, `textures`, `switches`, 부모부터 자식까지의 `chain`, parameter별
`parameterSources`, `unresolvedDefaults`와 원본 native static set을 보존한다.
`sources`에는 실제 읽은 package 경로와 SHA-256이 들어간다.

부모 값을 복사한 뒤 자식의 직렬화된 override를 적용한다. 명시적인 scalar 0,
switch false와 null texture는 값이므로 제거하지 않는다. local export reference는
소유 package를 붙이고 import reference는 원래 full path를 유지한다. 기본 Material
expression의 DefaultValue가 미기록이면 0/false를 만들지 않고
`unresolvedDefaults`에 남긴다. 자식의 실제 override나 native effective static set이
같은 parameter를 제공하면 해당 미해결 항목이 해소된다. 소비 compiler는 family에서
필요한 미해결 입력을 검사해야 하며 absent switch를 false로 취급하면 안 된다.

현재 source ABI는 KR v868이다. 원본 map static set의 세 번째
`FStaticNormalParameter`는 BYTE+uint32+GUID의 29-byte record이고 네 번째
terrain record는32byte다. 도구 전용 adapter가 이 layout과 numbered FName을
공유 shader-map parser에 전달하며 원본 byte offset/hash를 유지한다. 공유 parser의
전역 함수나 다른 효과 추출기의 해석은 바꾸지 않는다.

parent cycle, 미지원 class/version, 일부만 해석된 MIC parameter array와 native
static resource 부재·미지원 layout은 명시적인 실패다. 하나라도 실패하면 기존
parameter 출력은 보존하고 receipt에 실패를 기록한다. 성공은 parameter 해석만
뜻하며 shader-map 선택, runtime 연결, 완전한 재질 복원이나 화면 승인이 아니다.

## 원본 Texture2D 전체 mip 회수

`extract_ue3_texture_mips.py`는 기존 DDS의 mip0와 원본의 압축 블록이 같은지
검사하고 원본에 저장된 하위 mip까지 DDS로 기록한다. 일반 mip 생성이나 이미지
재압축을 하지 않는다. source package, source object, 기존 DDS, 출력과 Lost Ark
전용 UModel을 모두 명시한다. 기존 DDS에 전체 mip이 있어도 mip0를 분리해 비교한다.

```powershell
python Tools\LevelPlacementExtractor\extract_ue3_texture_mips.py `
  --source-object efmaster_material.tex.lightbox_cube1 `
  --source-package C:\source\Packages\OV8WELO70A8WLO7GW1DR6O.upk `
  --package-root C:\source\Packages `
  --umodel C:\tools\umodel_lostark_v7.exe `
  --expected-mip0 C:\existing\lightbox_cube1.dds `
  --output out\TextureMips\lightbox_cube1.dds `
  --scratch-root out\TextureMips\scratch

python -B -m unittest discover -s Tools\LevelPlacementExtractor `
  -p test_extract_ue3_texture_mips.py -v
```

지원 입력은 inline bulk를 가진 원본 `Texture2D`와 legacy BC1/DXT1,
BC3/DXT5, BC5/ATI2 DDS다. 원본 ObjectRedirector는 목적 package/object를 resolve해
따라가며 cycle, 불명확한 export, 외부 bulk, 미완성 chain, 다른 압축 형식과 mip0
불일치는 오류로 반환한다. `TextureCube`, DX10 DDS, 일반 Crunch decoder와 임의
normal/색공간 변환은 이 도구의 지원 범위에 포함하지 않는다.

UModel이 원본 mip0만 내보내는 제한 때문에 repo `out` 아래의 지정한 scratch-root에
고유 run 폴더를 만들고, 복원한 package 복사본에서 mip record 순서만 바꿔 각 원본
mip을 회수한다. 원본 packed payload는 바꾸지 않는다. 물리 summary/chunk table과
논리 export offset을 구분하고, 2×2/1×1 mip의 4×4 블록 저장 규칙을 보존한다.
원본 패키지·decoder 폴더를 출력 대상으로 거부하며 원본 파일에 쓰지 않는다.
scratch는 실패 진단을 위해 남긴다. 게임이나 Client를 실행하지 않는다.

모든 mip 검증과 입력 hash 재확인 후 출력 DDS를 원자적으로 교체한다. 추출·검증에
실패하면 기존 출력과 receipt를 보존한다. 기본 `<output>.receipt.json`에는 원본
package·decoder SHA-256, redirect, 원본 속성, mip별 packed/block hash와 mip0 동일
여부를 기록한다. `--receipt`로 경로를 바꿀 수 있다. 색공간·sampler 및 재질 연결은
기존 source metadata 계약을 계속 사용하며 이 도구가 추정하지 않는다. Resources
설치와 map publish는 호출하는 pipeline의 별도 단계다.

## 왜 필요한가

UModel의 일반 메시 export는 메시 파일 자체만 내보낸다. 레벨이 어떤 메시를
몇 개 만들었고 어디에 배치했는지는 레벨 UPK의 ExportTable과 각 export의
UE3 tagged property 안에 별도로 저장된다. 이 도구는 다음 순서로 그 배치표를
복구한다.

1. Lost Ark 전용 UModel의 `-nameresolve`로 논리 패키지명을 실제 난독화 UPK에 연결한다.
2. UPK summary의 Lost Ark 20-byte chunk table을 읽는다.
3. 각 chunk의 앞 4096바이트를 AES-256 ECB로 해제하고 LZ4 block을 복원한다.
4. NameTable, ImportTable, ExportTable을 UE3 규칙으로 파싱한다.
5. Actor와 Component의 tagged property를 읽고 StaticMesh import를 연결한다.
6. MapTool이 소비할 수 있는 안정적인 `placementId`, asset path, UE3-native Transform을 기록한다.

## 실행

```powershell
python Tools\LevelPlacementExtractor\extract_ue3_placements.py `
  --umodel C:\path\to\umodel_lostark_v7.exe `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --output C:\path\to\placements `
  LV_LUT_HEARTRB_ED_PS `
  LV_LUT_HEARTRB_ED_SL00 `
  LV_LUT_HEARTRB_ED_SL01 `
  LV_LUT_HEARTRB_ED_SL02 `
  LV_LUT_HEARTRB_ED_SL03 `
  LV_LUT_HEARTRB_ED_SL04 `
  LV_LUT_HEARTRB_ED_SL05
```

출력은 레벨별 `*.placements.json`과 전체 집계
`placement_manifest.json`이다. 입력 UPK를 수정하거나 중간 복호화 패키지를
디스크에 만들지 않는다.

## Transform 계약

- `actor`와 `component`에는 추출한 원시 속성을 모두 보존한다.
- `transform`은 바로 배치에 사용할 정규화 결과다.
- 일반 `StaticMeshActor / InterpActor`는 Actor의
  `Location / Rotation / DrawScale / DrawScale3D`가 정본이다.
- `StaticMeshCollectionActor`는 Actor가 identity이고 Component의
  `Translation / Rotation / Scale / Scale3D`가 인스턴스별 정본이다.
- 좌표는 아직 `UE3-native`다. Client 좌표계 전환은 MapTool importer의 한
  지점에서만 적용해야 하며 추출 JSON 자체를 파괴적으로 변환하지 않는다.
- Rotator 원시값과 degree 값을 함께 저장한다. 한 바퀴는 65536 units다.

## Lost Ark 전용 주의점

- compressed chunk descriptor는 표준 UE3의 16바이트가 아니라 마지막
  `encrypted_lz4` 필드가 붙은 20바이트다.
- 현재 KR level package의 compression flags는 `0x44`이며 실제 payload는
  AES 해제 뒤 LZ4로 푼다.
- `IntProperty`는 선언된 4바이트 payload 앞에 Lost Ark 전용 8바이트
  descriptor가 추가된다.
- Actor export에는 UnrealScript stack frame이 있을 수 있어 tagged property의
  시작 위치가 고정 4바이트가 아니다. 파서는 유효한 Property FName 쌍과
  `None` terminator를 함께 검증한다.
- 에셋 import 목록만으로는 배치 복구가 되지 않는다. 반드시 ExportTable의
  Actor와 Component 직렬화 데이터를 읽어야 한다.

## HeartRB 실증과 이중 검토

2026-07-30 `PS + SL00~SL05`를 원본 package에서 두 번 독립 실행했다.

```text
placement                         13,091
unique StaticMesh object path        260
duplicate placement ID                 0
asset manifest join missing             0
owner Actor missing                      0
property errors                          0
non-finite Transform                     0
zero scale                               0
negative scale                       5,042
```

기존 결과와 재실행 결과의 placement JSON 7개 및 manifest 1개는 SHA-256이 전부
일치했다. 전수 감사 receipt는 다음 파일이다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/
  LV_LUT_HEARTRB_ED/placements/placement_audit.json
```

`12,949`는 streaming level `SL00~SL05`만의 Component 수이고, `13,091`은
persistent level `PS`의 142개를 더한 값이다.

## DeployData 게임플레이 Prop 복구

`PS/SL`의 StaticMesh placement와 `DeployData`의 Prop placement는 서로 다른
source layer다. 파괴물은 다음 네 정본을 ID로 join한다.

1. `DeployData.loa`: deploy actor ID와 UE3 Transform
2. `EFTable_Prop.db`: LookInfo ID, HP, blocking, hit mesh, state action
3. `EFDLProp_*.loa`: intact/fractured/skeletal mesh와 particle/audio reference
4. `TriggerMapData.loa`: actor별 spawn/despawn/destroy/state/조건 연결

HeartRB zone `37051` 재현 명령은 다음과 같다.

```powershell
python Tools\LevelPlacementExtractor\extract_deploydata_props.py `
  --deploy-data C:\path\to\37051\DeployData.loa `
  --prop-db C:\path\to\EFTable_Prop.db `
  --game-action-db C:\path\to\EFTable_GameAction.db `
  --skill-effect-db C:\path\to\EFTable_SkillEffect.db `
  --lookinfo-dir C:\path\to\37051\LookInfo `
  --static-raw-root C:\path\to\extracted\ITR `
  --trigger-map C:\path\to\37051\TriggerMapData.loa `
  --output C:\path\to\LV_LUT_HEARTRB_ED.deployprops.json `
  --arena-output C:\path\to\LV_LUT_HEARTRB_ED.valtan_arena.deployprops.json `
  --expect-records 169 --expect-arena-records 112
```

실측 결과는 전체 169개, 발탄 아레나 112개다. 아레나 112개 중 시각 모델은
85개이며, 77개는 fractured static mesh가 해석됐고 8개는 skeletal mesh다.
111개는 TriggerMapData 바이너리에서 deploy actor ID의 little-endian 4바이트 패턴이
2~6회 발견됐다. 특정 Trigger node·필드의 직접 참조를 구조 파싱한 수치는 아니다.
이 레코드는 정적
`.mapplacements`에 합쳐 저장하지 않는다. `ChangePropState`, `DestroyHitProp`,
`SpawnProp`, 난이도/페이즈 조건을 해석한 별도 Deploy runtime layer가 소유한다.

`StateOffActionId`는 `GameAction -> SkillEffect`까지 연결한다. HeartRB의
`3705101`~`3705107`은 실제 DB row이며, `3705102`~`3705107`은 250~450 cm 범위의
`SkHit_Claw1` 낙하/피격 효과와 공통 10,000 cm 효과를 가진다. 따라서 상태 액션을
단순히 “mesh 숨김”으로 축약하면 원본 충돌과 연출을 잃는다.

## Floor crack 재구성 도구

원본 crack MaterialInstance에는 diffuse와 normal만 있고 authored emissive slot은
없다. 원작 영상에서 보이는 녹색 틈빛은 exact normal을 유지하면서 영상 근거
재구성 texture로 분리한다.

```powershell
python Tools\LevelPlacementExtractor\build_valtan_crack_emissive.py `
  --input C:\path\to\bg_rad_valtan_crack_floor01_d_lsj.png `
  --output C:\path\to\bg_rad_valtan_crack_floor01_em_reconstruction.png

python Tools\LevelPlacementExtractor\sync_wmodel_pack_manifest.py `
  --pack C:\path\to\BG_RAD_VALTAN_FLOOR01A_SM `
  --material-index 1 `
  --normal bg_rad_valtan_crack_floor01_n_lsj.png `
  --emissive bg_rad_valtan_crack_floor01_em_reconstruction.png `
  --emissive-proof "VIDEO_MATCH reconstruction; no authored emissive slot"
```

에미시브는 diffuse를 밝게 덮는 텍스처가 아니라 주변보다 어두운 국소 골만 골라
만든다. `sync_wmodel_pack_manifest.py`는 v2 WMat 변경 뒤 stale hash가 남지 않게
`asset.manifest.json`과 `.complete.json`을 실제 산출물에서 원자적으로 갱신한다.

## HeartRB 환경·Deploy runtime 생성

정적 환경, phase texture proxy, gameplay Deploy layer는 서로 다른 provenance와 파일을
유지한다.

```powershell
python Tools\LevelPlacementExtractor\build_valtan_environment_runtime.py
python Tools\LevelPlacementExtractor\build_valtan_phase_layers.py
python Tools\LevelPlacementExtractor\build_deployprop_runtime.py
python Tools\LevelPlacementExtractor\build_maptool_scene.py
```

현재 최종 생성 결과는 다음과 같다.

```text
Map Catalog v4 base             269 = exact 260 + overlay 9
Landscape exact                   6
Final Map Catalog                275
Map placement base            13,103 = exact 13,091 + overlay 12
Landscape placement                6
Final Map placement           13,109
render profile                  12
Deploy catalog                   9
Deploy visual placement         85 = static 77 + skeletal 8
```

- 쇠사슬 A/B 13개, CloudPlane 2개, sky mirror 1개는 exact level placement다.
- crack Floor01A/B와 중앙 보강은 reconstruction overlay다.
- spacehole/hugechaosgate 6개 plane은 exact particle texture를 사용하지만 topology와
  debug 전환 timing은 영상 기반 reconstruction이다. 기본 저장 상태는 hidden이다.
- Deploy static 77개는 LookInfo의 FracturedStaticMesh 직접 참조와 같은 package의 intact
  sibling을 쌍으로 조리한다.
- `ITR_02326` 8개는 exact skinned WModel이지만 source glTF에 animation clip이 없어
  현재 bind pose다. `ITR_02326_Ani` AnimSet을 찾기 전에는 animation 완료로 표기하지 않는다.
- MapTool은 환경 phase(Baseline/SpaceHole/ChaosGate)와 Deploy state
  (Intact/Fractured/Despawned)를 수동 검증할 수 있다. 이 radio는 원작 Trigger timing이 아니다.

Catalog v4의 emissive intensity는 asset별 render profile이 소유한다. crack profile의
`0.35`를 모든 맵 자산에 전역 적용하지 않는다. 같은 G-buffer MRT를 쓰는 모든 일반
writer는 emissive target에 0을 기록해 뒤쪽 발광이 전경을 뚫는 현상을 막는다.

## 발탄 후방 타워 source 접합 보존

`LV_LUT_HEARTRB_ED_SL04`의 후방 고딕 타워 상부는 같은 source 조립체의 하부·체인과
원본 높이에서 맞물린다. SL00/SL04 floor 차이 `10.6108742m`를 상부 47개씩에만 적용하면
하부는 제자리에 남고 상부가 공중에 분리된다. 다음 도구는 manifest에 고정된 후방 4개
station의 47개씩, 총 188개를 원본 placement와 exact join하고 잘못된 phase registration을
제거한다. 원본 188개는 source transform에서 `visible=1`, 등록 overlay와 hidden override는
0개, 대응 point light 4개는 source Y `24.734033`을 유지한다. 전방 control station인
`pointlight_11`도 변경하지 않는다.

```powershell
python Tools\LevelPlacementExtractor\sync_valtan_tower_phase_registration.py
python Tools\LevelPlacementExtractor\sync_valtan_tower_phase_registration.py --check-only

powershell -ExecutionPolicy Bypass -File Tools\MapPipeline\Publish-MapAuthoring.ps1 `
  -AreaId LV_LUT_HEARTRB_ED
```

source attachment sync 단독 baseline은 placement `13,186`, 등록 overlay `0`, 원본 visible
`188`이다. `heartrb_valtan_tower_phase_registration.json`에 없는 source placement를 반경이나
asset 이름으로 추측해 추가하지 않는다. 과거 `PROJECT_AUTHORED_RIM` 136개 box나
Landscape 6개를 타워 대용으로 다시 병합하지 않는다.

## MapTool commit 이후에도 남는 검증 경계

정적 Map과 Deploy visual layer의 parse/validate/stage/commit은 구현됐다. 다만 아래
항목까지 자동으로 복구됐다는 뜻은 아니다.

- 원본 placement 5,042개에는 음수 scale이 있다. 현재 MapTool의 양수-only validator로
  버리지 말고 reflection/winding/culling을 검증한다.
- property 오류 0은 tagged property를 읽었다는 뜻이다. hidden, collision-only,
  per-actor material override, Terrain, Particle, DeployData, navigation까지 끝났다는 뜻이 아니다.
- TriggerMap의 node/field 구조, Matinee의 정확한 프레임과 fade timing은 아직 별도 파서가
  필요하다. 지금 phase/state radio는 자동 raid 진행 대체물이 아니라 검증 도구다.

전체 공통 순서와 모든 함정은 다음 두 정본에 기록한다.

```text
C:/Users/user/Desktop/LostArk/.md/GB/07-30/맵추출파이프라인.md
C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/gotchas.md
```

## 발탄 Landscape 정본 병합

발탄 base 생성 결과는 269 assets / 13,103 placements이고 원본 Landscape는 별도
6 assets / 6 placements다. base 문서를 다시 생성한 직후 아래 병합을 반드시 실행해
최종 `LV_LUT_HEARTRB_ED` 단일 문서를 275 / 13,109로 만든다.

Landscape를 reconstruction overlay로 바꾸거나 LFS 문서에 6줄을 손으로 붙이지 않는다.
병합기는 원본 `transformSource=component`, imported ID, Prototype tag, WModel을 검증하며
아래 같은 명령을 두 번 실행해도 중복을 만들지 않으며 두 번째 실행 결과도
275 / 13,109로 유지된다. 최초 통합 receipt는 같은 출력 해시인 동안 덮어쓰지 않아
269 / 13,103에서 6개를 추가한 provenance도 보존한다. 기존 receipt가 다른 출력 문서를
가리키면 정본을 쓰기 전에 실패한다. 통합 전 개수는 receipt에 기록하고 최종 개수만
gate로 둔다.

현재 경로 계약에서는 extractor/병합기의 최종 catalog를
`Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapassets`, placement를
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements`에 반영한다.
`Client/Bin/DataFiles/Map`을 extractor 출력으로 직접 지정하지 않는다. 반영 후 아래 publisher가
catalog, placement와 deploy pair를 한 트랜잭션으로 실행 폴더에 배포한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools\MapPipeline\Publish-MapAuthoring.ps1 `
  -AreaId LV_LUT_HEARTRB_ED
```

### PBR steady emission and constrained metallic-mask diffuse

`build_source_map_materials.py` preserves the source emissive texture, tint, intensity and UV tiling. For PBR with `use_emissive=true` and `use_flicker=false`, it emits `emissive.flicker.mode: "none"` with zero phase inputs. Runtime uses the existing `sourceBgFlicker=3` carrier value only for this explicit PBR mode. Old PBR documents without `mode`, typed zero-initialized surfaces and BG flicker modes keep their previous behavior; `"nested"` explicitly selects the old PBR behavior.

The PBR metallic-mask diffuse branch is admitted only when both diffuse tints match, diffuse saturation is 1, masked saturation and reflection are disabled, and the outer nonmetallic/metallic brightness factors match. Under those conditions the compiler folds the two source diffuse brightnesses into the existing nonmetallic/metallic brightness inputs. Other combinations fail; this is not general metallic-mask graph support. Explicit source AO/brightness zero remains zero. Native null emissive defaults still require an exact referenced-texture expression receipt. These branches do not establish original scene SH, BRDF, dynamic MIC values or visual fidelity.
