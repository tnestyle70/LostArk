# ModelAssetConverter 사용법

`ModelAssetConverter`는 FBX/glTF를 미리 조리(cook)해서, LostArk 엔진의 `CModel`이 읽는 단일 모델 파일인 `.wmodel`을 만드는 도구입니다.

이 도구는 Winters의 Assimp 임포트와 WINT 바이너리 writer를 기반으로 만들었지만, LostArk에서는 결과물을 하나의 `.wmodel`로 묶고 `CModel::Create`에 경로 하나만 등록하도록 변경했습니다.

## 핵심 구조

```text
FBX
  -> ModelAssetConverter(개발 도구에서만 Assimp 사용)
  -> MyAsset.wmodel + textures/*
  -> CModel(런타임에서는 Assimp 없이 바이너리 해석)
  -> 기존 CMesh / CMaterial / CBone / CAnimation
  -> 기존 DirectX 11 렌더링
```

`.wmodel` 안에는 mesh, material, skeleton, animation 데이터가 들어갑니다. 이미지 자체는 `.wmodel`에 넣지 않고, 옆의 `textures` 폴더에 외부 파일로 둡니다. 컨버터가 찾은 텍스처는 출력 폴더의 `textures`로 복사하고 바이너리에는 `textures/파일명` 상대 경로를 기록합니다.

### 런타임 경계

- 신규 런타임의 정본은 `CModel -> CMesh / CMaterial / CBone / CAnimation`입니다.
- `CCookedModel`과 `CBinaryAssetObject`는 제거된 경로이며 새 기능에서 사용하거나 되살리지 않습니다.
- `.wmodel`의 WMaterial v2는 텍스처 경로를 보존하지만 glTF의 `baseColorFactor` 같은 상수 색은 저장하지 않습니다.
- diffuse와 emissive 경로가 모두 없는 경우 `CMaterial`이 1×1 회색 diffuse를 제공하지만, 이는 형상 확인용 안전망입니다. 최종 에셋은 실제 diffuse 경로를 가져야 합니다.

## 실행 파일

```text
Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe
```

같은 폴더의 Assimp 관련 DLL도 함께 있어야 합니다. Windows에는 Visual C++ 2015-2022 x64 재배포 패키지가 설치되어 있어야 합니다.

## 1. 일반 FBX 하나를 `.wmodel`로 만들기

애니메이션 모델:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  "C:\Asset\Character\Hero.fbx" `
  -o ".\Client\Bin\Resources\Character\Hero\Hero.wmodel" `
  --texture-root "C:\Asset\Character\Textures"
```

정적 맵 메시:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  "C:\Asset\Map\Rock.fbx" `
  -o ".\Client\Bin\Resources\Map\Rock\Rock.wmodel" `
  --pretransform `
  --texture-root "C:\Asset\Map\Textures"
```

FBX 하나 안에 mesh/material/skeleton/animation이 모두 있으면 이 명령 한 번으로 끝납니다. FBX에 애니메이션 클립이 없으면 컨버터도 없는 애니메이션을 만들어낼 수 없습니다.

## 2. 분리된 애니메이션 조각을 하나로 묶기

발탄처럼 모델 FBX와 애니메이션 FBX가 분리되어 있어 이미 `.wmesh/.wskel/.wanim`을 만든 경우에는 `pack`을 사용합니다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe pack `
  ".\MN_RPBF_01.wmesh" `
  -o ".\MN_RPBF_01.wmodel" `
  --material ".\MN_RPBF_01.wmat" `
  --skeleton ".\MN_RPBF_01.wskel" `
  --anim-dir ".\anims"
```

발탄의 현재 `.wmodel`은 이 방식으로 27개 `.wanim`을 하나로 묶었습니다.

## 3. 재질만 WMA2로 다시 만들기

기존 mesh/skeleton/animation을 유지하고 재질만 다시 만들 때 사용합니다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe material `
  "C:\Asset\Character\Hero.fbx" `
  -o ".\Hero.wmat" `
  --texture-root "C:\Asset\Character\Textures"
```

LostArk 캐릭터의 `hero_mi`, `hero_1_mi` 같은 재질명과 `hero_d`, `hero-1_n` 같은 텍스처명은 구분자를 제거해 서로 매칭합니다.

## 지원 재질 슬롯

새 WMA2 재질은 다음 경로를 서로 구분해 보존합니다.

| 슬롯 | 대표 이름 | 런타임 CMaterial 슬롯 |
|---|---|---|
| Base color | `_d`, diffuse, albedo | `aiTextureType_DIFFUSE` |
| Normal | `_n`, normal | `aiTextureType_NORMALS` |
| Specular/mask | `_s`, specular | `aiTextureType_SPECULAR` |
| Emissive | `_e`, `_em`, emissive | `aiTextureType_EMISSIVE` |
| Opacity | opacity, alpha | `aiTextureType_OPACITY` |
| Packed ORM | `_orm`, `_mra`, `_rma` | `aiTextureType_UNKNOWN` |
| Metallic | metallic, metalness | `aiTextureType_METALNESS` |
| Roughness | roughness | `aiTextureType_DIFFUSE_ROUGHNESS` |
| AO | `_ao`, occlusion | `aiTextureType_AMBIENT_OCCLUSION` |

주의: LostArk의 `_s`가 어떤 채널을 무엇으로 쓰는지는 원본 머티리얼 규칙에 따라 다를 수 있으므로 임의로 표준 ORM이라고 가정하지 않습니다. `_s`는 원본 specular/mask 슬롯으로 그대로 보존합니다.

현재 제품은 기존 `CModel → CMaterial`에서 SourceCharacter와 map material의 native surface·lighting 경로를 사용합니다. 지원하는 원본 family에서는 specular/mask, roughness, emissive, IBL 반사와 BRDF 입력을 실제로 소비합니다. WMA2에 일반 텍스처 경로를 넣는 것과 그 native 재질 descriptor를 연결하는 것은 별도 단계입니다. 컨버터만 실행하면 원본 shader·환경 반사가 자동 복구되는 것은 아닙니다. emissive를 diffuse로 대체하는 호환 처리는 native 재질 복원 결과가 아닙니다.

## 원본 재질과 환경 반사까지 적용하기

전체 절차의 정본은 [렌더링·이펙트 복원 공통 절차](../../.md/GB/렌더링이펙트복원V2.md#오브젝트-추출에서-재질환경-입력을-보존하는-공통-절차)입니다. 모델을 새로 추출하거나 정적·애니메이션 변형을 만들 때 함께 읽습니다.

- 원본 mesh의 material array와 component override, MIC 상속·static switch를 확인합니다. `.props.txt`의 D/N/S 이름만으로 native family, mask 채널, roughness나 환경 반사식을 추정하지 않습니다.
- 이미 승인된 원본 모델 재질은 `CActorCatalog::Build_ModelLoadDescription`이 실제 생성하는 `CModel`까지 전달합니다. 파생 World Object는 같은 slot의 원본 모델을 `materialSourceModelAssetId`로 지정하고, 배경 재질은 해당 Area의 `mapMaterialBindings`로 연결합니다. 정확한 field와 실패 정책은 [Area 가이드](../../.md/TEAM/AREA_DATA_LAYER_GUIDE.md)를 따릅니다.
- native descriptor가 요구하는 IBL texture/cube, BRDF LUT, parameter와 색 공간, 추가 UV를 보존합니다. 원본에 없는 반사·금속도를 일괄 추가하지 않습니다. 맵 ambient와 배치 lightmap은 모델 재질과 별도 입력입니다.
- 실제 사용 slot에 대한 재질 연결과 파일 존재를 검사합니다. `info`의 geometry/material section 성공만으로 환경 반사 복원을 완료 처리하지 않습니다.

이미 식별한 원본 재질을 파생 World Object에 연결하는 공통 명령은 다음과 같습니다. `--object-id`를 반복하면 같은 source를 사용하는 여러 객체를 한 번에 stage합니다. 새 후보를 먼저 쓰며 `--apply`를 지정했을 때만 검증과 source 변경 비교 뒤 저작 파일을 교체합니다. runtime 파일은 기존 Area publisher가 생성합니다.

```powershell
python Tools/ModelAssetConverter/apply_world_object_material_source.py `
  --document 'Data/Maps/Authoring/<AreaId>/<AreaId>.worldsequences.json' `
  --object-id '<stable-object-id>' `
  --source-model 'Character/<original-model-path>.wmodel' `
  --output 'out/material-source-candidate.json' `
  --apply
```

원본 catalog override, source/target의 실제 material slot과 texture가 있어야 합니다. source를 모를 때 이름이 비슷한 모델을 대신 지정하지 않습니다. `World Object Tool`에서는 `Material Source Model` → `Apply Material Source` → `Save`로 같은 목적의 검증·게시를 수행합니다.

## 자동 매칭이 불확실한 맵 메시

맵 FBX의 재질명이 `dummy_material_0`이면 파일명만으로 어느 텍스처가 몇 번 재질인지 확정할 수 없습니다. 이 경우 자동 탐색을 끄고 슬롯을 명시합니다.

UModel을 `-notex`로 실행하면 glTF의 0.3 회색 `baseColorFactor`만 남을 수 있습니다. 이것은 diffuse 이미지가 아닙니다. 여기에 `--no-auto-textures`까지 사용하면 명시적 remap 없이는 textureless `.wmodel`이 만들어집니다.

UModel `lostark_v7`의 실제 머티리얼을 복구할 때는 `-notex`를 빼고 `-obj=<mesh-name>`으로 한 object만 export합니다. 생성된 Material Instance `.props.txt`에서 `texture_diffuse`, `texture_normal`, `texture_emissive`, `texture_orm`을 읽어 각 remap에 전달합니다. 파일명만 보고 `_p`를 ORM으로 추측하지 않습니다.

Diffuse 맵 예시:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  "C:\Asset\Map\bg_rad_valtan_crystal01a_sm_khb.fbx" `
  -o ".\Client\Bin\Resources\Map\BG_RAD_VALTAN_A\CrystalKHB\CrystalKHB.wmodel" `
  --pretransform `
  --no-auto-textures `
  --material-remap "dummy_material_0=C:\Asset\Textures\bg_rad_valtan_crystal01_d_khb.png"
```

Emissive 전용 맵 예시:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  "C:\Asset\Map\bg_rad_valtan_crystal01a_sm.fbx" `
  -o ".\Client\Bin\Resources\Map\BG_RAD_VALTAN_A\CrystalEmissive\CrystalEmissive.wmodel" `
  --pretransform `
  --no-auto-textures `
  --emissive-remap "dummy_material_0=C:\Asset\Textures\bg_rad_valtan_crystal01_em_pcs.png"
```

수동 옵션은 다음과 같습니다.

```text
--material-remap
--normal-remap
--specular-remap
--emissive-remap
--opacity-remap
--orm-remap
--metallic-remap
--roughness-remap
--ao-remap
```

UModel glTF는 좌표가 meter입니다. 현재 맵 Loader가 기존 centimeter 자산에 `0.01f`를 적용하므로 UModel glTF는 cook할 때 `--scale 100`을 함께 사용합니다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  '<umodel-source.gltf>' `
  -o '<output.wmodel>' `
  --pretransform --no-auto-textures --scale 100 `
  --material-remap '<material>=<diffuse.dds>' `
  --normal-remap '<material>=<normal.dds>'
```

크기 계약은 `glTF meters × 100 × Loader 0.01 = 게임 월드 meters`입니다.

## 결과 검사

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe info ".\MyAsset.wmodel"
```

section 수, skeleton 유무, animation 수, 재질 버전과 첫 재질의 텍스처 슬롯을 출력합니다. 정적 모델은 보통 `sections=2`, 애니메이션 모델은 mesh/material/skeleton과 animation 수만큼 section이 나옵니다.

맵 에셋은 header 검사만으로 배포하지 않습니다. 사용 재질이 요구하는 실제 texture와 native 입력이 runtime root에 존재하는지, material slot과 descriptor가 일치하는지, 추가 UV가 보존됐는지 검사합니다. UModel glTF 입력의 bounds는 약 100배로 cook하고 runtime preScale과 합쳐 월드 미터가 되는지 확인합니다. 의도적으로 diffuse를 쓰지 않는 원본 재질에 임의 diffuse를 넣어 검사를 우회하지 않습니다.

## 게임에 Prototype 등록

애니메이션 모델:

```cpp
CModel::Create(
    m_pDevice,
    m_pContext,
    MODEL::ANIM,
    "../Bin/Resources/Character/Hero/Hero.wmodel",
    XMMatrixScaling(0.01f, 0.01f, 0.01f));
```

정적 맵 모델은 `MODEL::NONANIM`으로 등록합니다.

위 코드는 모델 로드의 최소 예시입니다. 제품 actor·World Object는 해당 catalog가 만든 material override를 포함한 기존 load descriptor를 사용해야 합니다. `.wmodel` 경로만 새로 등록해 원본 native 재질 연결을 생략하지 않습니다.

## 이미지 포맷

런타임은 DDS, PNG, JPG, BMP와 LostArk 추출물에서 자주 나오는 24/32-bit TGA(비압축 또는 RLE)를 읽습니다. TGA는 `CMaterial`의 전용 로더에서 BGRA를 RGBA로 변환해 DirectX 11 texture/SRV를 만듭니다.

## 협업 규칙

```text
Client/Bin/Resources/
├─ Character/<AssetName>/<AssetName>.wmodel
├─ Map/<Area>/<AssetName>/<AssetName>.wmodel
└─ ...각 모델 옆 textures/
```

- 코드와 `Tools/ModelAssetConverter`는 Git/LFS로 공유합니다.
- 대용량 `Client/Bin/Resources` 결과물은 asset pack으로 배포하며, 최상위에는 `Fonts`, `Character`, `Deploy`, `Effect`, `Map`, `Sound`, `UI`만 둡니다.
- 팀원은 각자 같은 컨버터와 같은 WMA2/WMODEL 포맷을 사용합니다.
- `.wmodel` 이름은 달라도 상관없습니다. 내부 magic/section 포맷이 같으면 모두 같은 `CModel` 파이프라인으로 렌더됩니다.
- 해당 domain publisher, 변경한 생성·로더의 최소 컴파일과 실제 재질 입력 검사를 수행합니다. Client/UI 화면 확인은 사용자가 직접 합니다. 관련 없는 전체 하네스를 리소스 적용의 선행조건으로 붙이지 않습니다.
