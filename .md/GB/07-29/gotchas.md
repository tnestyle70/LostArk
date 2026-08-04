# LostArk 맵 에셋 추출·런타임 Gotchas

작성일: 2026-07-29  
적용 범위: UModel `lostark_v7`, `ModelAssetConverter`, `.wmodel`, `CModel`, `CMaterial`, MapTool

이 문서는 에셋이 카탈로그와 Hierarchy에는 생성됐지만 화면에 보이지 않았던 문제의 근본 원인과 재발 방지 규칙을 고정한다.

## Runtime map 폴더를 rename하면 WModel도 재쿠킹해야 한다

`Map/<AreaId>`를 `Map/CHARACTERSELECTMAP`처럼 바꾸면서 catalog와 폴더만 변경하면 안 된다.
WModel material section에는 `Resource/Map/<AreaId>/...` texture path가 저장되어 있으므로,
최종 runtime namespace로 material remap을 다시 주어 재쿠킹해야 한다. 완료 검증은 catalog
경로뿐 아니라 모든 WModel 내부 texture reference가 같은 `runtimeAssetRoot` 아래 실제 파일로
해석되는지까지 포함한다.

## 1. 런타임 경계: 반드시 CModel로 통합한다

- `CCookedModel`과 이를 직접 소비하는 `CBinaryAssetObject`는 **레거시 검증 경로**다.
- 신규 에셋 기능, MapTool, 정적 맵 오브젝트, 보스 모델은 `CCookedModel`을 사용하거나 확장하지 않는다.
- 정본 경로는 다음 하나다.

```text
CLoader
  -> CModel::Create(... .fbx 또는 .wmodel ...)
  -> CModel::Ready_BinaryModel(.wmodel이면 CWModelDecoder 사용)
  -> CMesh + CMaterial + CBone + CAnimation
  -> CMapAssetObject 또는 기존 GameObject가 CModel Component를 사용
```

`CCookedModel`에 fallback이나 새 렌더 기능이 있더라도 신규 경로의 근거로 삼지 않는다. 공통 수정은 `CModel`, `CMaterial`, `CMesh` 계층에 반영한다.

## 2. “배치는 됐는데 안 보임”의 근본 원인

문제가 난 환경 에셋은 다음 두 옵션을 연속으로 사용했다.

```text
UModel: -notex
ModelAssetConverter: --no-auto-textures
```

이 조합의 결과는 다음과 같다.

1. `-notex`가 UModel의 머티리얼·텍스처 dependency 로드를 막았다.
2. glTF에는 `dummy_material_N`과 `baseColorFactor = [0.3, 0.3, 0.3, 1]`만 남고 image/texture 참조가 없었다.
3. `--no-auto-textures`가 컨버터의 파일명 기반 텍스처 검색도 막았다.
4. 현재 WMaterial v2는 텍스처 경로를 저장하지만 glTF의 `baseColorFactor` 같은 상수 색을 저장하지 않는다.
5. 따라서 `.wmodel`의 diffuse 경로가 비었고, 기존 `CModel -> CMaterial`에서 diffuse bind가 실패해 오브젝트가 생성됐어도 렌더되지 않았다.

중요: UModel/Assimp가 보여 주는 0.3 회색은 **추출된 diffuse 이미지가 아니라 뷰어용 상수 색 fallback**이다. 이를 “텍스처 추출 성공”으로 판단하면 안 된다.

기존 발탄 17개가 보였던 이유도 자동 복원이 아니다. 기존 추출 스크립트가 Material Instance를 조사한 뒤 `dummy_material_N=실제 텍스처`를 `--material-remap`으로 명시했기 때문이다.

## 3. 런타임 안전망과 한계

`CMaterial::Initialize(const MODEL_MATERIAL_DATA&)`는 diffuse와 emissive 경로가 모두 비어 있으면 1×1 `0.3` 회색 diffuse SRV를 만든다.

- 목적: 텍스처 누락 에셋도 MapTool에서 형상과 크기를 확인할 수 있게 한다.
- 위치: `CModel -> CMaterial` 통합 경로.
- 금지: 이 fallback을 최종 머티리얼 복원으로 간주하지 않는다.
- 최종 배포: 실제 Material Instance가 지정한 diffuse를 추출하고 `.wmodel`에 경로를 기록해야 한다.

## 4. 올바른 object 단위 재추출

geometry 전수 감사만 할 때는 기존 `-notex` raw를 재사용한다. 실제 런타임 머티리얼을 복원할 때는 `-notex`를 빼고 **한 object만** 추출한다.

```powershell
$UModel = 'C:\Users\user\Desktop\Resource_LostArk\06_Tools\UEViewerLostArk_runtime\umodel_lostark_v7.exe'
$Packages = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages'
$Package = Join-Path $Packages '<physical-package.upk>'

& $UModel `
  -export -game=lostark -kr -nameresolve -gltf -dds `
  "-path=$Packages" `
  "-out=<fresh-output>" `
  "-obj=<static-mesh-object-name>" `
  $Package
```

주의사항:

- 검증된 object 선택 문법은 positional object/class가 아니라 단일 인자 `-obj=<name>`이다.
- package 전체를 텍스처 포함 export하지 않는다. dependency 폭증, 장경로, 디스크 고갈을 일으킨다.
- UModel이 만든 Material Instance `.props.txt`의 `ParameterName`과 `ParameterValue`를 읽는다.
- 슬롯은 `texture_diffuse`, `texture_normal`, `texture_emissive`, `texture_orm`처럼 **명시된 파라미터만** 매핑한다.
- 이름이 `_p`라는 이유만으로 ORM이라고 추측하지 않는다. 원본 파라미터가 `texture_orm`일 때만 ORM으로 기록한다.
- 메시 이름과 비슷한 텍스처를 임의 선택하지 않는다. direct Material Instance dependency를 따른다.

## 5. UModel glTF 스케일 계약

UModel glTF는 Unreal centimeter 좌표를 meter로 이미 변환한다. 반면 현재 맵 Loader는 기존 FBX/WModel centimeter 자산을 위해 `0.01f` PreTransform을 적용한다.

따라서 UModel glTF를 그대로 cook하면 런타임에서 다시 `0.01`이 적용되어 100배 작아진다. 다음처럼 cook 시 meter를 legacy centimeter로 되돌린다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  '<source.gltf>' `
  -o '<output.wmodel>' `
  --pretransform `
  --no-auto-textures `
  --scale 100 `
  --material-remap '<material-name>=<diffuse.dds>' `
  --normal-remap '<material-name>=<normal.dds>' `
  --emissive-remap '<material-name>=<emissive.dds>' `
  --orm-remap '<material-name>=<orm.dds>'
```

최종 크기 계약:

```text
UModel glTF meters × converter 100 × Loader 0.01 = 게임 월드 meters
```

캐릭터의 `0.0001f`와 맵의 `0.01f`를 섞지 않는다.

## 6. 유지 파이프라인

현재 이 PC에서 사용하는 스크립트:

```text
C:/Users/user/Desktop/Final_LostArk/Tools/AssetPipeline/build_environment_library.py
```

현재 런타임 선택을 다시 만들 때:

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py audit
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py extract-materials --scope runtime --workers 8
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py cook --scope runtime --workers 8 --force
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope runtime
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py install-runtime
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope runtime
```

`extract-materials`는 object 단위 UModel export, Material Instance props 파싱, 텍스처 해시와 슬롯 매니페스트 생성을 수행한다. `cook`은 실제 슬롯 remap과 `--scale 100`을 적용한다.

## 7. 배포 전 필수 검증

헤더 `WINT/WMOD`만 확인해서는 부족하다. 다음을 모두 확인한다.

1. 선택한 모든 `.wmodel`이 decode된다.
2. 모든 머티리얼의 base/diffuse 경로가 비어 있지 않다.
3. 경로가 가리키는 texture 파일이 runtime root에 실제로 존재한다.
4. 추출 매니페스트의 SHA-256과 설치 파일이 일치한다.
5. `.wmodel` bounds가 raw glTF bounds의 약 100배다.
6. Loader의 `0.01f` 적용 후 raw glTF meter 크기로 돌아온다.
7. `.mapassets` header count와 실제 행 수가 일치한다.
8. 발탄 전용 17개와 명시적 allowlist 선택 수의 합이 Catalog 행 수와 일치한다.
9. F2 → F1에서 선택, 월드 클릭 배치, Save/Reload를 실제 확인한다.

2026-07-29 최신 런타임 복구 실측:

```text
Valtan direct model            17
allowlisted environment         0
catalog                    17 + 0 = 17
missing model file              0
placement missing asset ID      0
```

과거 복구한 환경 모델과 텍스처는 조사 원본으로 남아 있지만 현재 Catalog에는 설치하지 않는다.
예를 들어 `ENV_C6F63AE8B4_BG_RAD_KAZEROTH_WALLMERGE03_SM`은 재질 슬롯 복구 검증에는
사용할 수 있어도, 실제 발탄 레벨 참조 근거가 확인되기 전에는 런타임 후보가 아니다.

## 8. 발탄 직명 17개와 실제 HeartRB 레벨을 혼동하지 않는다

- `BG_RAD_VALTAN_A`에서 직접 export된 StaticMesh는 정확히 17개다. 이것은 한 mesh package의 ExportTable 결과이지 발탄 level dependency 목록이 아니다.
- 발탄 DB chain은 `MN_RPBF_01 -> ClassifyIndex 3705100 -> content 37051 -> ZoneBase.LevelFileName LV_LUT_HeartRB_ED_PS`다.
- 실제 조사 범위는 `LV_LUT_HEARTRB_ED_PS`와 `SL00`~`SL05`, 총 일곱 UPK다.
- 일곱 exact ImportTable의 StaticMesh entry는 `6 / 86 / 142 / 122 / 130 / 81 / 142`, 합계 709다. 중복 제거한 object name은 254개다.
- 254 이름을 전체 raw StaticMesh glTF 29,519개에 대조했고 unresolved는 0이다.
- source duplicate를 보존해 exact card는 293개이며, direct 17개 비교군을 더한 gallery는 310개다. ISO/TOP는 620장, render failure는 0이다.
- direct 17개의 object name은 HeartRB exact StaticMesh ImportTable과 교집합이 0이다. current Catalog 17개와 actual level exact set을 같은 것으로 부르지 않는다.
- `bg_rad_valtan_crack_wall03*`, `crackcolumn01*`는 재질·텍스처이고 벽·기둥 mesh 이름이 아니다.
- 과거 환경 256개 선택은 이름 점수 후보 111개와 `BG_SHS_RCARENA_A~D` 고정 145개를 합친 결과였다. 실제 HeartRB 레벨 참조 증거가 아니므로 runtime Catalog에서 제거했다.
- `LUTOMB`, `RCARENA`, Kazeroth, Illiakan 등 분위기가 비슷한 package를 이름이나 육안만으로 발탄 후보에 고정하지 않는다.
- `LV_RAD_SHATTEREDBS*`는 이름 때문에 조사한 오답 family다. DB가 연결한 실제 발탄 family는 `LV_LUT_HEARTRB_ED`다.
- 현재 Catalog 정본은 `BG_RAD_VALTAN_A` direct 17개뿐이며 explicit allowlist는 0개다.
- 환경 후보는 `valtan_candidate_allowlist.json`에 안정적인 asset ID, group, evidence를 명시한 경우에만 설치한다.
- allowlist의 중복 ID, inventory에 없는 ID, 잘못된 schema/area는 즉시 실패해야 한다.
- 실제 level의 actor material override가 확인되면 mesh ID와 함께 `materialOverride` 근거를 저장한다. 공용 geometry의 원본 재질만 cook하면 발탄 균열 벽의 색과 발광이 재현되지 않을 수 있다.
- 추출된 `ENVIRONMENT_CURATED/ENV_*` 폴더는 Catalog에 없으면 로드되지 않는다. 조사 원본이므로 명시적인 정리 작업 전에는 물리 삭제하지 않는다.

## 9. UE3 level ImportTable 복구 Gotchas

### 9.1 ExportTable과 ImportTable은 질문이 다르다

```text
mesh package ExportTable = 이 package가 소유한 mesh
level package ImportTable = 이 level의 actor/component가 외부에서 참조한 object
```

발탄에 어떤 mesh가 쓰였는지 찾을 때 `BG_RAD_VALTAN_A` export만 세면 다시 17개에서 멈춘다. logical level을 먼저 찾고 level ImportTable을 봐야 한다.

### 9.2 NameTable은 참조 증거가 아니다

NameTable은 package 안에서 사용될 수 있는 이름 사전이다. `trh_wall08_sm`이나 그럴듯한 mesh 이름이 NameTable에 보인다는 사실만으로 `StaticMeshActor`가 그 mesh를 import했다고 말할 수 없다. 정확 참조라고 표기하려면 최소한 complete ImportTable row에서 다음이 모두 있어야 한다.

```text
className == StaticMesh
objectName == target
fullPath가 valid negative PackageIndex chain으로 구성됨
resolvedStringCount == header ImportCount
```

초기 `heart_rb_memory_meshes.json`의 plausible name 231개는 방향 탐색 자료이고 exact proof가 아니다.

### 9.3 “그럴듯한 앞부분”을 전체 table로 착각하지 않는다

PS package는 처음에 130개짜리 Class-heavy segment가 잡혔지만 header `Imports`는 138이었다. 후보 scoring에서 Class prefix보다 `138/138 resolved`를 우선한 뒤 완전한 table을 찾았다.

- UModel log의 target `Names`, `Imports`를 먼저 고정한다.
- NameTable 후보는 target `Names` 수와 맞아야 한다.
- ImportTable 후보는 target `Imports` rows를 전부 읽어야 한다.
- class/object pointer가 모두 decoded name으로 resolve되어야 한다.
- `PackageIndex`가 table bounds에 맞아야 한다.
- 한 row라도 부족하면 exact 결과로 승격하지 않는다.

### 9.4 import row가 항상 Class block부터 시작한다고 가정하지 않는다

package마다 table 배열 순서가 다를 수 있다. 일부는 첫 row가 Class import가 아니다. 또한 negative `PackageIndex`가 현재 row보다 뒤쪽 import를 outer로 가리키는 실측 사례가 있었다. 다음 가정을 hard validation으로 쓰지 않는다.

```text
잘못된 가정 1: table 첫 row는 반드시 Class
잘못된 가정 2: outer import는 반드시 child보다 앞 row
잘못된 가정 3: StaticMesh object name만 있으면 full package path는 필요 없음
```

outer chain은 `-PackageIndex - 1`로 따라가되 bounds와 cycle guard를 둔다.

### 9.5 메모리 주소와 preliminary anchor JSON은 재사용하지 않는다

ASLR과 process allocation 때문에 UModel 주소는 실행마다 달라진다. `startAddress`, string pointer, PID를 다음 실행의 입력으로 고정하지 않는다. 매 실행에서 readable region, ASCII anchors, pointer array를 새로 찾는다.

메모리 scanner는 분석 도구 UModel에만 `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ`로 붙었다. game client나 anti-cheat를 대상으로 하지 않고 write/patch/injection/hook도 하지 않는다.

### 9.6 ImportTable 부재의 의미를 과장하지 않는다

serialized StaticMeshActor/Component dependency를 찾는 데 ImportTable은 강한 근거다. 그러나 runtime script가 문자열로 `LoadObject`하는 동적 object까지 절대적으로 부정하지는 못한다. 따라서 direct 17개의 exact 교집합 0은 “HeartRB serialized StaticMesh import가 아니다”라는 강한 음성 증거로 기록하되 모든 동적 가능성을 0이라고 단정하지 않는다.

## 10. exact mesh 목록과 완전한 맵 복원을 혼동하지 않는다

### 10.1 중앙 원형을 안 근거

`bg_pvp_retown_floor01_sm`은 다음 사슬로 확인됐다.

```text
LV_LUT_HEARTRB_ED_SL00 exact ImportTable
  -> pvp_retown_a.mesh.bg_pvp_retown_floor01_sm
  -> PVP_RETOWN_A raw glTF
  -> HRT_885CC3ECA0A0 ISO/TOP render
  -> screenshot 중앙 원형 구조 대응
```

source package가 `PVP`라는 이유로 탈락시키면 안 된다. level은 다른 domain의 공용 mesh를 import할 수 있다.

### 10.2 254, 293, 310은 각각 다른 수다

| 수치 | 의미 |
|---:|---|
| 709 | 일곱 level ImportTable의 StaticMesh row 합계; level 간 중복 포함 |
| 254 | 중복 제거한 exact object name |
| 293 | 같은 object name의 여러 raw source copy를 보존한 exact gallery card |
| 310 | exact 293 + direct Valtan 비교군 17 |
| 620 | 310 card × ISO/TOP 2 views |

숫자가 다르다고 누락이라고 판단하지 않는다. 반대로 310 cards를 310개의 서로 다른 level import 이름이라고 부르지도 않는다.

### 10.3 ImportTable은 placement가 아니다

ImportTable이 주는 것은 dependency class/name/path다. 다음은 별도로 level ExportTable의 actor/component property를 복구해야 한다.

- location, rotation, scale
- actor hierarchy와 streaming visibility
- per-actor material override/lightmap
- Terrain/Landscape
- Particle/fog/post-process/water material
- InterpActor/destructible/skeletal/moving actor
- collision/nav/trigger/gameplay volume

멀리 떠 있는 거대한 폐성채는 단일 mesh가 아니라 church pillar/tower, fortress, chain 등을 Transform으로 조립한 composite다. “큰 성 한 덩어리 mesh”만 찾는 방식으로는 완성되지 않는다.

### 10.4 stale report와 현재 runtime을 구분한다

```text
environment_inventory.json      전체 archive 검색 정본
environment_runtime_111.json    과거 heuristic 보존본
environment_summary.json        과거 audit 시점 runtimeAssets 411 포함; stale
environment_runtime.json        current explicit allowlist selection; 현재 0
valtan_candidate_allowlist.json current 승인 입력; 현재 0
BG_RAD_VALTAN_A.mapassets       current installed catalog; 현재 17
```

파일명이나 summary field 하나로 current state를 판단하지 않는다. `allowlist -> environment_runtime.json -> mapassets`의 생성 순서와 timestamp를 함께 본다.

### 10.5 gallery는 검수 UI이지 자동 설치기가 아니다

`index.html` 체크박스와 selection JSON은 사용자가 고르기 위한 단계다. 선택 결과를 stable runtime asset ID에 연결하고 allowlist에 evidence를 쓴 뒤 `audit -> extract-materials -> cook -> verify -> install-runtime`을 실행해야 MapTool에 들어간다. 310개를 통째로 카탈로그에 넣지 않는다.

현재 scanner/helper는 다음 조사 경로에 있다.

```text
C:/Users/user/Desktop/Final_LostArk/_work/valtan_probe_20260729/scan_umodel_imports.py
C:/Users/user/Desktop/LostArk/.codex_tmp/valtan_heart_rb_gallery_20260729/
  recover_import_table_from_anchor.py
  build_gallery.py
```

이들은 현재 재현 가능한 research tools지만 production pipeline CLI로 승격된 것은 아니다. `.codex_tmp`를 지우기 전에는 결과를 보존하고, 팀 공용화할 때 process lifecycle, exact count gate, receipt, timeout, cleanup을 정식 구현한다.

## 11. 빠른 판별표

| 증상 | 원인 후보 | 확인/해결 |
|---|---|---|
| Hierarchy에는 있는데 화면에 없음 | diffuse 경로 공백, bind 실패 | `ModelAssetConverter info`, 실제 texture 파일 확인 |
| 모두 같은 회색 | `-notex` raw 또는 CMaterial 안전 fallback | object 단위 `extract-materials` 후 재cook |
| 100배 작음 | UModel glTF에 Loader `0.01`을 중복 적용 | cook에 `--scale 100` |
| 잘못된 표면 이미지 | 파일명 추측 매핑 | Material Instance `.props.txt`의 명시 슬롯 사용 |
| UModel 출력이 폭증 | package 전체 dependency export | `-obj=<name>` 단일 object export |
| 발탄 검색 결과가 17개뿐 | mesh package ExportTable만 조사 | DB로 HeartRB level을 식별하고 exact ImportTable/gallery 사용 |
| PVP/BG_FAT source라서 발탄이 아니라고 보임 | source package와 consuming level 혼동 | level ImportTable full path를 provenance로 사용 |
| NameTable mesh 이름이 많음 | 이름 사전을 참조 목록으로 오해 | complete ImportTable에서 `className == StaticMesh` filter |
| import 후보가 header count보다 짧음 | table 중간 segment/prefix를 잡음 | target ImportCount 전부 resolve될 때까지 폐기 |
| exact 이름 254인데 card 310 | source duplicate와 direct 비교군 포함 | 254/293/310의 집합 정의 확인 |
| 멀리 보이는 성채 한 덩어리가 없음 | 여러 actor/mesh의 composite | level actor Transform/export property 복구 |
| summary에는 runtime 411인데 UI에는 17 | stale audit summary | allowlist 0, environment_runtime 0, mapassets 17 확인 |
| `git status`에 runtime이 없음 | `Client/Bin/Resources`는 ignore/Drive pack | 정상. 카탈로그와 코드만 Git, pack은 별도 공유 |
| 새 구현이 `CCookedModel`을 참조 | 레거시 경로 재도입 | `CModel -> CMaterial`로 이동 |

## 12. 2026-07-30 범용 맵 exact pipeline에서 확정된 추가 함정

범용 정본 문서는 `.md/GB/07-30/맵추출파이프라인.md`와
`C:/Users/user/Desktop/Final_LostArk/Tools/AssetPipeline/맵추출파이프라인.md`다.

### 12.1 정상 raw root가 있으면 `_COUNT_*` 복구 사본보다 우선한다

HeartRB exact 260경로를 다시 resolve할 때 `BG_BER_COMMON_A` 3개 object는 정상 root와
`_COUNT_MISSING` 사본의 payload hash가 달랐다. 복구 사본을 같은 우선순위로 놓으면 exact
ImportTable을 가지고도 다른 geometry를 고를 수 있다.

```text
정상 logical package/object 후보 존재
  -> 정상 후보끼리 payload hash 검증
  -> _COUNT_MISSING/_COUNT_MISMATCH는 감사 기록으로만 보존

정상 후보 없음
  -> recovery bucket 후보 검토
  -> 서로 다른 payload가 둘 이상이면 fail
```

### 12.2 UModel staging 경로에 긴 asset ID를 넣지 않는다

긴 `MAP_<hash>_<object>` ID를 staging leaf로 사용했을 때 UModel은 exit 0을 반환하면서도
target glTF를 만들지 않은 object가 3개 있었다. Win32 legacy path 길이 문제였다. staging
leaf를 12자리 UUID로 줄인 뒤 260/260이 성공했다.

### 12.3 WModel magic은 파일 첫 4바이트 `WMOD`가 아니다

실제 계약은 `WINT` container 안의 `WMOD` section이다.

```text
offset 0x00 = WINT
offset 0x10 = WMOD
minimum size = 64 bytes
```

converter stdout의 `wrote`만 확인하거나 첫 4바이트를 `WMOD`로 검사하면 각각 false
positive와 false negative가 생긴다.

### 12.4 UModel `-save`는 복호화된 Actor payload가 아니다

SL00의 `-list`가 가리키는 StaticMeshActor serial offset을 물리 UPK와 `-save` 결과에서
읽으면 0/난독화 데이터다. special UModel은 내부 `FUE3ArchiveReader`에서 읽을 때만 이
구간을 복호화한다. `-save` 파일을 일반 UE3 parser에 넣는 것을 placement 복구로 부르면
안 된다.

### 12.5 `-list`와 ImportTable의 역할이 다르다

- `-list`: export index, class, object name, decoded serial offset/size
- ImportTable: 외부 package/object reference와 exact full path
- Actor/Component payload: mesh reference, owner 관계, Transform, visibility

셋 중 하나만으로 placement JSON을 완성하지 않는다.

### 12.6 object name 254와 exact path 260은 모두 맞는 수치다

동일 object name이 서로 다른 root package 경로에 존재하는 6건 때문에 HeartRB는 unique
name 254개, unique exact full path 260개다. production asset ID는 object name이 아니라
full path를 hash한다.

### 12.7 실행 중인 UModel을 이미지 이름으로 일괄 종료하지 않는다

다른 조사 세션이 같은 `umodel_lostark_v7.exe`를 사용할 수 있다. `Stop-Process -Name`,
`taskkill /IM`처럼 process image 전체를 종료하지 않는다. production tool은 자신이 만든
child PID만 `terminate -> bounded wait -> kill fallback` 순서로 정리한다.

### 12.8 HeartRB exact asset pack과 placement는 별도 완료 조건이다

2026-07-30 exact asset pack은 다음 상태로 완료됐다.

```text
unique exact paths           260
material manifests           260
assets with textures         187
texture files                461
cooked WModels               260
runtime installed WModels    260
```

SL00~SL05의 12,949 StaticMeshComponent와 PS의 142개를 더한 13,091 placement도 이후
원본 UPK의 Lost Ark chunk를 AES-256 ECB + LZ4로 메모리 복원해 추출했다. 단, 다음 완료
조건은 계속 분리한다.

```text
asset pack PASS                 260/260 WModel와 runtime 파일
UE3-native placement PASS       13,091 Transform, property 오류 0
Client coordinate contract OPEN axis/handedness/scale 런타임 검증 전
full map reconstruction OPEN    hidden/collision/DeployData/nav/dynamic actor 전
```

### 12.9 물리 UPK serial offset을 바로 읽지 않는다

`UModel -list`의 offset/size는 logical package stream 기준이다. 물리 UPK나 `-save` 사본의
같은 offset을 읽으면 0 또는 난독화 데이터를 얻게 된다. PackageSummary의 20-byte chunk
descriptor를 읽고, block별 앞 4,096바이트를 AES-256 ECB로 해제하고 LZ4를 복원한 logical
stream에서 ExportTable의 serial 범위를 읽는다.

### 12.10 memory ImportTable 32바이트와 on-disk ImportTable 28바이트를 섞지 않는다

live UModel scanner가 관찰한 row는 string pointer를 담은 32바이트 helper 구조다. 복원된 UE3
package의 on-disk ImportTable row는 `FName ClassPackage + FName ClassName + int32 Outer +
FName ObjectName`, 총 28바이트다. 한 layout을 다른 입력에 적용하지 않는다.

### 12.11 Actor property 시작을 고정 offset으로 가정하지 않는다

Actor export에는 UnrealScript stack frame이 존재할 수 있다. `NetIndex 뒤 4바이트` 같은 고정
규칙으로 자르면 일부 actor가 어긋난다. 유효한 property name/type FName 쌍을 찾은 뒤 전체
stream이 `None` terminator에 도달하는 후보만 채택한다.

### 12.12 일반 Actor와 StaticMeshCollectionActor의 Transform 정본이 다르다

- `StaticMeshActor`, `InterpActor`: Actor의 `Location/Rotation/DrawScale/DrawScale3D`
- `StaticMeshCollectionActor`: Component의 `Translation/Rotation/Scale/Scale3D`

HeartRB 전수 감사에서 actor-source 2,282개, component-source 10,809개였고, 일반 Actor의
Component local transform은 전부 identity, CollectionActor의 Actor transform도 전부 identity였다.
원시 Actor/Component 값을 둘 다 JSON에 보존하고 `transform.source`로 선택 근거를 남긴다.

### 12.13 음수 scale을 invalid placement로 버리지 않는다

HeartRB 원본 13,091개 중 5,042개에 적어도 한 축의 음수 scale이 있다. 이는 mirror/reflection
배치일 수 있다. 현재 MapTool의 `scale > 0` validator에 바로 넣으면 원본의 약 38.5%가 탈락한다.
좌표계 변환 시 determinant와 winding/culling 영향을 검증하고, 음수 scale을 지원하거나
geometry bake로 명시적으로 흡수하기 전에는 placement 문서를 commit하지 않는다.

### 12.14 `propertyErrorCount == 0`을 완전한 맵 복원으로 과장하지 않는다

0은 대상 Actor/Component의 tagged property stream을 전부 읽었다는 뜻이다. hidden/editor-only,
collision-only, per-actor material override, Terrain, Particle, SkeletalMesh, 파괴 상태, DeployData,
navigation까지 분류했다는 뜻이 아니다.
