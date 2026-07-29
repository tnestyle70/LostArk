# LostArk 맵 에셋 추출·검색·변환·런타임 적용 가이드

작성일: 2026-07-29  
상태: 현재 로컬 정본 기준 RESULT / OPERATIONS GUIDE  
대상: `lostark_v7`, UModel, `Resource_LostArk`, `ModelAssetConverter`, `CMapAssetCatalog`, `CMapTool`

이 문서는 다른 세션이 발탄 직명 17개만 보고 조사를 끝내거나, 이미 끝난 전체 BG 추출과 `.wmodel` 변환을 처음부터 반복하지 않도록 현재 조사 결과와 재현 절차를 한곳에 고정한다. 특히 **발탄 레벨 식별 → UE3 ImportTable 완전 복구 → 원본 glTF 역매핑 → 갤러리 검수 → 명시적 allowlist → MapTool 설치**의 전체 흐름과 각 단계의 증거 수준을 분리한다.

## 1. 먼저 내릴 결론

1. `BG_RAD_VALTAN_A`라는 이름으로 직접 export된 StaticMesh는 17개가 맞다. 그러나 이것은 **발탄 레벨 전체 사용 에셋 목록이 아니라, 이름이 발탄인 한 BG 패키지의 export 목록**이다.
2. 발탄 Zone의 실제 레벨 family는 DB 교차 조회로 확인한 `LV_LUT_HEARTRB_ED`다. 영속/보조 레벨 `PS`와 지오메트리 스트리밍 레벨 `SL00`~`SL05`를 조사 대상으로 삼았다.
3. 일곱 UPK의 UE3 ImportTable을 UModel 프로세스의 읽기 전용 메모리에서 정확한 header `ImportCount`만큼 전부 복구했다. StaticMesh import entry는 합계 709개, 중복 제거한 object name은 254개다.
4. `bg_pvp_retown_floor01_sm`은 `LV_LUT_HEARTRB_ED_SL00` ImportTable의 정확 참조다. full import path는 `pvp_retown_a.mesh.bg_pvp_retown_floor01_sm`이고, 스크린샷 중앙 원형 구조와 형상이 대응한다. 이름 유사도 후보가 아니다.
5. 복구한 254개 이름을 BG만이 아닌 전체 raw StaticMesh glTF 29,519개와 대조했고 미해결 이름은 0개다. 중복 package source와 비교용 발탄 직명 17개를 포함한 검수 갤러리는 310 cards, ISO/TOP 620장, render failure 0이다.
6. 반대로 `BG_RAD_VALTAN_A` 직명 17개는 `PS`, `SL00`~`SL05`의 exact StaticMesh ImportTable에 하나도 없었다. 이 17개가 현재 MapTool에 있는 이유와 HeartRB 레벨 참조 여부는 별개다.
7. `BG_SHS_RCARENA_*`, `BG_LUT_LUTOMB_*`, Kazeroth/Illiakan 등은 과거 이름·분위기 기반 후보였다. HeartRB exact ImportTable 증거가 없는 항목을 발탄 정답이나 runtime 에셋으로 자동 승격하지 않는다.
8. 전체 BG StaticMesh 19,259개는 이미 glTF 원본과 `.wmodel` 아카이브로 확보했다. 이것은 검색 archive이지 현재 runtime catalog가 아니다.
9. 과거 이름 점수로 고른 111개와 머티리얼 복구 결과는 기술 검증용 historical set이다. 현재 `valtan_candidate_allowlist.json`은 0개이고, 현재 `.mapassets`는 기존 발탄 직명 17개만 가진다.
10. Landscape 높이맵, actor placement/Transform, actor별 material override, collision/nav, effect, movable/destructible/skeletal actor는 ImportTable StaticMesh 목록과 별도 lane이다. ImportTable은 “이 레벨이 무엇을 의존하는가”를 증명하지만 “어디에 어떤 Transform으로 배치했는가”까지 증명하지 않는다.
11. 게임의 MapTool 카탈로그는 최대 512개이며 Level 로딩 때 모든 모델 Prototype을 선등록한다. 19,259개나 310개를 검토 없이 한꺼번에 카탈로그에 넣지 않는다.

## 2. 현재 정본과 수치

| 항목 | 현재 값 |
|---|---:|
| BG StaticMesh | 19,259 |
| 고유 object name | 19,157 |
| logical package | 644 |
| raw glTF | 약 75.2 MB |
| raw buffer | 약 3.18 GB |
| 전체 `.wmodel` | 19,259 / 누락 0 |
| 전체 `.wmodel` 크기 | 2,910,236,494 bytes |
| 1차 curated 목록 | 480 |
| 전체 raw StaticMesh glTF | 29,519 |
| HeartRB 조사 level package | 7 (`PS`, `SL00`~`SL05`) |
| 복구한 ImportTable entry | 4,276 / 문자열 해석 4,276 |
| HeartRB exact StaticMesh import entry | 709 |
| HeartRB exact 고유 StaticMesh object name | 254 |
| exact 이름의 raw glTF 미해결 | 0 |
| 검수 gallery card | 310 |
| ISO/TOP thumbnail | 620 / 실패 0 |
| 과거 heuristic runtime set | 111 (historical, 현재 미설치) |
| 현재 explicit allowlist | 0 |
| 현재 `.mapassets` | 17 (`BG_RAD_VALTAN_A` direct rows only) |

카테고리 1차 분류는 이름 기반 휴리스틱이다.

| category | 수량 | 대표 토큰 |
|---|---:|---|
| wall | 1,337 | wall, rampart, battlement, parapet |
| building | 2,156 | house, castle, temple, tower, roof |
| pillar | 1,090 | pillar, column, buttress |
| gate | 384 | gate, doorway, portal, entrance |
| bridge | 394 | bridge, walkway, platform |
| ruin | 187 | broken, destroy, debris, collapsed |
| stair | 540 | stair, step, ladder, ramp |
| fence | 525 | fence, rail, barricade, barrier |
| cliff | 110 | cliff, terrain, mountain, ridge, cave |
| rock | 888 | rock, stone, boulder, ore |
| foliage | 1,587 | tree, bush, grass, vine, root, moss |
| floor | 2,217 | floor, ground, tile, road, arena |
| decor | 2,133 | statue, altar, torch, crystal, banner |
| other | 5,711 | 위 토큰에 걸리지 않은 나머지 |

`wall` 1,337개 중 287개는 `BG_WALLPAPER_*`가 이름에 `wall`을 포함해서 생긴 오분류다. 이를 제외한 구조물 벽 1차 후보는 약 1,050개다. runtime 선별 점수에서는 `BG_WALLPAPER`에 `-500`을 적용해 제외했다.

## 3. 디렉터리와 소유권

### 3.1 설치 원본과 추출 도구

```text
Lost Ark 설치 원본
C:/ProgramData/Smilegate/Games/LOSTARK

lostark_v7 UModel runtime
C:/Users/user/Desktop/Resource_LostArk/06_Tools/UEViewerLostArk_runtime/
  umodel_lostark_v7.exe
  SDL2_64.dll 또는 해당 bundle의 검증된 SDL DLL

도구 bundle 정본
C:/Users/user/Desktop/Final_LostArk/Tools/ThirdParty/UEViewerLostArk/bundles/
```

설치 원본은 읽기 전용 입력이다. 파일을 교체하거나 수정하지 않는다.

### 3.2 현재 확보된 원본·바이너리·보고서

```text
raw BG glTF 정본
C:/Users/user/Desktop/Resource_LostArk/
  01_Extracted/Map/StaticMesh_Raw_20260729/BG/

과거 runtime 111개 머티리얼·텍스처 재추출 보존본
C:/Users/user/Desktop/Resource_LostArk/
  01_Extracted/Map/Environment_Textured_20260729/<AssetId>/

전체 WModel 아카이브
C:/Users/user/Desktop/Resource_LostArk/
  02_WintersBinary/Final_LostArk/Map/Environment_All/

검색·선별·cook 보고서
C:/Users/user/Desktop/Resource_LostArk/
  05_Reports/EnvironmentLibrary_20260729/
```

보고서 파일 역할:

| 파일 | 역할 |
|---|---|
| `environment_inventory.csv` | 사람이 Excel/PowerShell로 검색하는 전체 목록 |
| `environment_inventory.json` | 파이프라인이 읽는 전체 목록 |
| `environment_summary.json` | 전체 수치와 package 상위 목록 |
| `environment_curated_480.json` | 넓은 시각 검토 후보 |
| `environment_runtime_111.json` | 과거 이름 점수 기반 111개 보존본; 현재 설치 정본 아님 |
| `environment_runtime.json` | 현재 allowlist를 반영한 runtime selection; 현재는 빈 배열 |
| `environment_cook_receipt.json` | 마지막 cook 결과와 각 출력 경로 |

`environment_summary.json`은 20:39에 생성되어 `runtimeAssets: 411`을 담고 있지만, 그 뒤 20:47에 allowlist가 비워지고 20:48에 `environment_runtime.json`이 0개로 다시 생성됐다. 따라서 이 summary의 `runtimeAssets`는 현재 설치 수가 아니라 **이전 audit 시점의 stale selection 수치**다. 현재 설치 정본은 `valtan_candidate_allowlist.json`, `environment_runtime.json`, `.mapassets`를 함께 확인한다.

### 3.3 LostArk 런타임

```text
카탈로그: C:/Users/user/Desktop/LostArk/
  Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets

배치 데이터: C:/Users/user/Desktop/LostArk/
  Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapplacements

runtime WModel: C:/Users/user/Desktop/LostArk/
  Client/Bin/Resources/LostArk/Map/ENVIRONMENT_CURATED/<AssetId>/<AssetId>.wmodel
```

`Client/Bin/DataFiles/**`는 Git/LFS 대상이다. `Client/Bin/Resources/**`와 `*.wmodel`은 `.gitignore` 대상이며 공유 Drive pack으로 배포한다. 카탈로그만 push하고 runtime pack을 공유하지 않으면 다른 팀원은 Catalog validation부터 실패한다.

## 4. 전체 데이터 흐름

```text
LOSTARK 설치 원본의 난독화된 UPK
  -> EFTable_Npc에서 발탄 model MN_RPBF_01의 ClassifyIndex 3705100 확인
  -> IntegrateDungeon/ZoneBase에서 content 37051 -> LV_LUT_HEARTRB_ED_PS 확인
  -> map_level_packages.csv에서 PS, SL00~SL05 logical/physical UPK 매핑
  -> lostark_v7 UModel -view로 package를 정상 decode/load
  -> UModel 프로세스의 읽기 전용 메모리에서 NameTable/ImportTable 완전 복구
  -> className == StaticMesh인 exact import 709개, 고유 object name 254개 추출
  -> 전체 raw StaticMesh glTF 29,519개에 역매핑; unresolved 0
  -> source duplicate와 비교군을 보존한 310-card ISO/TOP 갤러리 생성
  -> 사용자가 확인한 asset ID만 valtan_candidate_allowlist.json에 추가
  -> 선택 object는 UModel -obj=<name>으로 material/texture dependency 재추출
  -> Material Instance .props.txt의 명시 슬롯을 remap
  -> ModelAssetConverter --pretransform --no-auto-textures --scale 100
  -> Resource_LostArk/02_WintersBinary/.../Environment_All
  -> allowlist selection만 runtime으로 복사
  -> BG_RAD_VALTAN_A.mapassets 갱신
  -> CMapAssetCatalog parse/validate
  -> CLoader가 LEVEL::ASSET_TEST에 CModel Prototype 선등록
  -> F2로 AssetTest 진입, F1으로 MapTool, 배치/저장/재로드
```

`Catalog`는 생성 가능한 에셋 정의의 정본이고 `.mapplacements`는 배치 인스턴스와 Transform의 정본이다. `.wmodel` 경로나 Prototype tag를 placement 파일에 중복 저장하지 않는다.

## 5. 원본 package 이름 복원과 UModel 추출

### 5.1 source catalog는 payload extractor가 아니다

다음 도구는 설치 원본의 physical filename을 logical package name으로 복원하고 SQLite/CSV/selection을 만든다.

```powershell
& C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\Invoke-LostArkSourceCatalog.ps1 `
  -SourceRoot C:\ProgramData\Smilegate\Games\LOSTARK `
  -OutputRoot C:\LostArkCatalog\source-v1
```

이 단계는 package 내용을 export하지 않는다. 결과의 `source_catalog.csv`, `catalog.sqlite3`, `logical_name_collisions.json`은 어떤 physical UPK가 어떤 logical package인지 찾는 용도다.

### 5.2 현재 전체 raw를 우선 재사용한다

전체 BG bulk export는 2026-07-29에 완료됐지만 당시 shard/recovery 작업은 `_work`에 남은 조사 산출물이며 유지되는 단일 bulk-export 스크립트가 아니다. 따라서 다음 원칙을 지킨다.

1. 먼저 `environment_inventory.csv`에서 object/package를 찾는다.
2. 해당 `sourceGltf`와 `.bin`이 존재하면 재추출하지 않는다.
3. 텍스처나 누락 dependency가 필요한 소수 object만 새 work root에 단일 export한다.
4. 새로운 전수 export가 필요하면 먼저 별도의 유지 가능한 exporter를 작성하고 package별 receipt, timeout, 용량 한도, 재시작 계약을 넣는다.

### 5.3 단일 StaticMesh 재추출 예

먼저 `-list` 또는 기존 list report로 정확한 package, object name, class를 확인한다. 실제 export는 fresh output 폴더에 한 object씩 수행한다.

```powershell
$UModel = 'C:\Users\user\Desktop\Resource_LostArk\06_Tools\UEViewerLostArk_runtime\umodel_lostark_v7.exe'
$Packages = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages'
$Package = Join-Path $Packages '<physical-package.upk>'
$Output = '<fresh-work-root>\mesh'

& $UModel `
  -export -game=lostark -kr -nameresolve -gltf -dds `
  "-path=$Packages" "-out=$Output" `
  "-obj=<object_name>" `
  $Package
```

성공 조건은 대상 `.gltf`와 local `.bin`, 직접 참조 Material Instance의 `.props.txt`, 해당 props가 가리키는 텍스처만 새 output에 존재하는 것이다. stdout의 `Loading StaticMesh3 ...`, `Loading MaterialInstanceConstant ...`, `Exporting Texture2D ...`를 receipt로 보존한다.

현재 lostark_v7 runtime에서 검증된 object 선택 문법은 positional object/class가 아니라 **한 인자로 전달하는 `-obj=<object_name>`** 이다. `-notex`는 전체 geometry 감사용 raw를 만들 때만 사용하고, 실제 런타임 머티리얼 복구에서는 사용하지 않는다.

특정 누락 텍스처만 다시 뽑을 때는 geometry와 분리해 해당 texture object만 export할 수 있다.

```powershell
& $UModel `
  -export -game=lostark -kr -nameresolve -dds `
  -path=$Packages -out='<fresh-work-root>\dds' `
  $Package '<texture_object_name>' Texture2D
```

한 package 전체를 텍스처와 함께 무제한 export하지 않는다. dependency 폭증, 경로 길이, free space 부족이 다시 발생할 수 있다.

## 6. 발탄 실제 레벨 식별과 UE3 ImportTable 복구

### 6.1 왜 17개만 찾았고, 왜 그 결론이 틀렸는가

처음 조사는 `BG_RAD_VALTAN_A`라는 logical package에서 **export된 StaticMesh**만 찾았다. 그 결과 17개가 나온 것 자체는 정확하다. 틀린 부분은 이 집합을 “발탄 레벨이 사용하는 모든 StaticMesh”로 해석한 것이다.

UE3 레벨 package와 mesh package의 역할은 다르다.

```text
mesh package
  ExportTable: 그 package가 소유하고 export하는 StaticMesh

level package
  ExportTable: Level, WorldInfo, StaticMeshActor, StaticMeshComponent 등
  ImportTable: actor/component가 외부 package에서 가져다 쓰는 StaticMesh, Material 등
```

따라서 `BG_RAD_VALTAN_A` export scan은 “그 이름의 mesh package가 소유한 것”만 답한다. 발탄 레벨이 `PVP_RETOWN_A`, `BG_FAT_*`, `BG_LUT_*` 같은 공용 package의 mesh를 배치했다면 정답은 **레벨 package ImportTable**에 있다. `bg_pvp_retown_floor01_sm`이 바로 이 경우다.

### 6.2 발탄 보스에서 실제 레벨까지 추적한 방법

이름이나 스크린샷 분위기로 레벨을 찍지 않고 게임 table의 식별자를 연결했다.

```text
EFTable_Npc.db
  model MN_RPBF_01 (발탄)
  -> ClassifyIndex 3705100

IntegrateDungeon
  content 37051
  -> commander raid content

ZoneBase
  PK 37051
  -> LevelFileName LV_LUT_HeartRB_ED_PS

map_level_packages.csv
  -> LV_LUT_HEARTRB_ED_PS 및 SL00~SL05의 physical UPK
```

`ClassifyIndex 3705100`의 상위 content key를 `37051`로 연결했고, `ZoneBase`가 준 `LevelFileName`을 source catalog/map inventory의 logical package와 교차 확인했다. 이 체인 때문에 `LV_LUT_HEARTRB_ED`가 발탄 family라는 판정은 이름 추측이 아니다.

조사한 package와 UModel header 실측은 다음과 같다.

| logical package | physical UPK | Names | Exports | Imports | exact StaticMesh imports |
|---|---|---:|---:|---:|---:|
| `LV_LUT_HEARTRB_ED_PS` | `312N3UN2RBQY9N952CQJ2CVG.upk` | 497 | 672 | 138 | 6 |
| `LV_LUT_HEARTRB_ED_SL00` | `534P5WP4TDS0BPB74ESL4EI522.upk` | 707 | 1,871 | 628 | 86 |
| `LV_LUT_HEARTRB_ED_SL01` | `534P5WP4TDS0BPB74ESL4EI529.upk` | 813 | 6,274 | 773 | 142 |
| `LV_LUT_HEARTRB_ED_SL02` | `534P5WP4TDS0BPB74ESL4EI52G.upk` | 754 | 4,018 | 716 | 122 |
| `LV_LUT_HEARTRB_ED_SL03` | `534P5WP4TDS0BPB74ESL4EI52N.upk` | 865 | 5,054 | 803 | 130 |
| `LV_LUT_HEARTRB_ED_SL04` | `534P5WP4TDS0BPB74ESL4EI52U.upk` | 622 | 4,829 | 487 | 81 |
| `LV_LUT_HEARTRB_ED_SL05` | `534P5WP4TDS0BPB74ESL4EI521.upk` | 753 | 3,109 | 731 | 142 |
| 합계 | 7 packages | - | - | 4,276 | 709 |

`PS`는 inventory layer가 `PARTICLE`로 표기되지만 영속/보조 level dependency가 있으므로 함께 조사했다. 실제 공간 geometry의 중심은 여섯 streaming level이다.

### 6.3 왜 UPK 파일을 바로 파싱하지 않고 UModel 메모리를 읽었는가

Lost Ark package는 logical name과 payload가 난독화되어 있고, `lostark_v7` UModel은 이를 정상 해독해 package를 로드한다. `-list` log는 `Names/Exports/Imports` 개수까지 보여 주지만 이 build의 일반 출력만으로는 level ImportTable의 모든 row를 안정적으로 얻지 못했다.

그래서 UModel을 다음 옵션으로 target logical level을 `-view`해 **UModel 자신이 정상 decode한 테이블**을 메모리에 유지시킨 뒤, 별도 scanner가 그 UModel 프로세스를 읽었다.

```text
umodel_lostark_v7.exe
  -view -game=lostark -kr -nameresolve
  -path=<ReleasePC/Packages>
  <logical level package>
```

메모리 접근 경계는 다음과 같다.

- 대상은 게임/클라이언트/안티치트가 아니라 로컬 분석 도구 `umodel_lostark_v7.exe` 프로세스다.
- `OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ)`, `VirtualQueryEx`, `ReadProcessMemory`만 사용했다.
- process memory write, patch, injection, hook은 하지 않았다.
- 주소는 실행마다 달라지므로 결과 JSON의 address는 재사용 계약이 아니다.

### 6.4 NameTable을 찾은 원리

1. UModel log에서 target package의 정확한 `Names` 수를 기록한다.
2. readable memory에서 UE3에 안정적으로 존재하는 ASCII name anchor인 `actor\0`, `package\0`, `staticmesh\0`, `worldinfo\0` 등을 찾는다.
3. writable/private region에서 그 문자열 주소를 가리키는 64-bit pointer들을 찾는다.
4. 주변 pointer array를 확장하고, UModel이 해석한 유효 ASCII name으로 모두 resolve되는지 확인한다.
5. array 길이가 header의 target `Names`와 정확히 맞는 후보를 NameTable로 채택한다.

핵심은 raw UPK에서 암호화된 문자열을 억지로 해독한 것이 아니라, UModel이 이미 해석해 둔 name storage의 pointer array를 찾았다는 점이다. anchor 몇 개나 “그럴듯한 이름 231개”는 방향을 찾는 probe일 뿐 완전성 증거가 아니다.

### 6.5 ImportTable을 복구한 원리

이 UModel 64-bit build에서 관찰한 in-memory import row layout은 32 bytes stride였다.

| offset | 값 | 해석 |
|---:|---|---|
| `+0` | `uint64` | class name 문자열 pointer |
| `+8` | `int32` | UE3 `PackageIndex` |
| `+16` | `uint64` | object name 문자열 pointer |
| `+24` | byte | UModel helper/missing 상태 byte; 식별 근거로만 보조 사용 |

scanner는 NameTable anchor 주변에서 이 구조로 반복되는 후보 segment를 찾는다. 복구 helper는 segment 안의 negative `PackageIndex` 관계로 가능한 table base를 역산하고, 주변 `±8 rows`를 다시 읽어 **header의 정확한 `ImportCount` rows**를 후보별로 decode한다.

후보 점수의 최우선 조건은 다음이다.

```text
resolvedStringCount == target ImportCount
validPackageIndexCount == target ImportCount
```

`Class` row가 앞부분에 많이 보이는지는 보조 점수일 뿐이다. 일부 package는 ImportTable의 첫 row부터 Class block이 아니기 때문이다. 실제로 PS는 처음에 보기 좋은 130개 segment가 잡혔지만 header는 138이었다. “Class-heavy prefix”보다 “138/138 완전 해석”을 우선하도록 바꾼 뒤 전체 table을 찾았다.

UE3 `PackageIndex < 0`은 다음 import row를 outer로 가리킨다.

```text
outerImportIndex = -PackageIndex - 1
```

실측 배열에는 현재 row보다 뒤쪽 index를 가리키는 경우도 있으므로 “outer는 항상 먼저 나온다”를 hard validation으로 쓰지 않았다. bounds/cycle guard를 둔 뒤 negative chain을 따라 `root package -> group -> object`를 조립한다. 예:

```text
pvp_retown_a.mesh.bg_pvp_retown_floor01_sm
```

최종적으로 `className.casefold() == "staticmesh"`인 row만 exact mesh dependency로 채택했다. 일곱 package 모두 `resolvedStringCount == targetImportCount`를 만족했으며, 709 entry에서 중복 object name을 제거해 254개가 됐다.

### 6.6 중앙 원형을 정확히 안 이유

`LV_LUT_HEARTRB_ED_SL00`의 복구된 ImportTable에 다음 row가 실제로 있다.

```text
className  = StaticMesh
fullPath   = pvp_retown_a.mesh.bg_pvp_retown_floor01_sm
objectName = bg_pvp_retown_floor01_sm
level      = LV_LUT_HEARTRB_ED_SL00
```

이 object name을 전체 raw glTF index에서 찾아 다음 source로 연결했다.

```text
PVP/PVP_RETOWN_A__U0U1Q8PMN7G1QXES1JU9GC3/
  PVP_RETOWN_A/StaticMesh3/bg_pvp_retown_floor01_sm.gltf
```

gallery stable ID는 `HRT_885CC3ECA0A0`, dimensions는 `9.2 × 5.287 × 9.2`, vertices/triangles는 `1,542 / 1,458`이다. ISO/TOP render의 네 방향 돌기, 내부 원형 구멍, 원통형 하부가 스크린샷 중앙 구조와 대응한다.

따라서 증거 사슬은 다음과 같다.

```text
발탄 DB 식별자
  -> HeartRB level family
  -> SL00 exact UE3 ImportTable row
  -> PVP_RETOWN_A exact package/object path
  -> raw glTF
  -> ISO/TOP 형상 확인
  -> 스크린샷 중앙 구조 대응
```

ImportTable만으로 actor의 정확한 위치/회전/scale이나 actor별 material override까지 얻은 것은 아니다. 그러나 “이 mesh가 발탄 HeartRB SL00의 serialized dependency다”는 정확히 증명한다.

### 6.7 전수 매핑과 갤러리의 숫자

- 검색 대상은 BG 19,259개만이 아니라 raw StaticMesh 전체 29,519개다. 중앙 원형이 `PVP` package에 있기 때문에 BG만 검색하면 다시 놓친다.
- exact 고유 object name 254개 모두 최소 한 개 raw glTF에 연결됐다. `unresolved_exact_import_mesh_names.txt`는 0 bytes다.
- 동일 object name이 여러 source package copy에 존재하면 어느 copy가 적절한지 눈으로 비교할 수 있도록 모두 보존했다. 그래서 exact card는 254가 아니라 293개다.
- 여기에 현재 MapTool의 `BG_RAD_VALTAN_A` direct 17개 비교군을 합쳐 310 cards다.
- 각 card를 ISO/TOP 두 장씩 렌더해 620 thumbnails이며 render error는 0이다.
- `BG_RAD_VALTAN_A` 17개의 `DirectExact` 교집합은 0이다. 단, UE3 runtime의 동적 string `LoadObject`까지 ImportTable 부재만으로 절대 부정할 수는 없다. serialized level geometry 근거로는 강한 음성 증거다.

검수 산출물:

```text
C:/Users/user/Desktop/LostArk/.codex_tmp/valtan_heart_rb_gallery_20260729/
  index.html
  assets.json
  assets.csv
  thumbs/
  contact_sheets/
  ps_recovered_imports.json
  sl00_recovered_imports.json ... sl05_recovered_imports.json
  unresolved_exact_import_mesh_names.txt
  render_errors.txt
```

`index.html`은 browser에서 체크박스로 고르고 선택 JSON을 export하기 위한 **검수 UI**다. 아직 선택했다고 MapTool catalog에 자동 설치되지는 않는다. 승인한 Gallery ID/source를 stable runtime asset ID로 연결해 `valtan_candidate_allowlist.json`에 명시한 뒤 기존 cook/install pipeline을 탄다.

조사 스크립트:

```text
C:/Users/user/Desktop/Final_LostArk/_work/valtan_probe_20260729/
  scan_umodel_imports.py

C:/Users/user/Desktop/LostArk/.codex_tmp/valtan_heart_rb_gallery_20260729/
  recover_import_table_from_anchor.py
  build_gallery.py
```

현재 이 스크립트들은 재현 가능한 조사 도구지만 `.codex_tmp`/`_work`에 있으므로 production pipeline CLI 정본은 아니다. 팀 공용으로 만들려면 logical level 입력, UModel process lifecycle, header count 자동 수집, exact acceptance gate, receipt schema, timeout/cleanup을 갖춘 별도 승격 작업이 필요하다.

### 6.8 아직 복구하지 않은 것

254개 mesh 목록은 “모든 발탄 맵 데이터”와 동의어가 아니다. 완전한 레벨 복원에는 다음이 추가로 필요하다.

1. `StaticMeshActor`/`StaticMeshComponent` export property와 actor Transform
2. actor별 material override와 lightmap association
3. Terrain/Landscape height/weight data
4. ParticleSystem, fog, post process, water/material animation
5. InterpActor, destructible, skeletal/moving actor와 animation
6. collision, physics volume, nav, trigger/gameplay volume
7. streaming state와 파괴 단계별 layer/visibility

멀리 보이는 거대한 폐성채도 하나의 “BG 한 덩어리”가 아니라 church pillar/tower, fortress, chain 같은 여러 imported mesh를 actor Transform으로 조립한 composite다. 다음 기술 단계는 ImportTable을 더 추측하는 것이 아니라 level ExportTable의 actor/component property를 parse해 placement를 복구하는 것이다.

## 7. 환경 라이브러리 파이프라인

현재 실행 스크립트:

```text
C:/Users/user/Desktop/Final_LostArk/Tools/AssetPipeline/build_environment_library.py
```

현재 스크립트 상단의 `RESOURCE`, `LOSTARK`, `RAW_BG`, `ARCHIVE`, `REPORT`, `CONVERTER`는 이 PC의 절대 경로다. 다른 PC에서 실행할 때는 명령을 바꾸는 것이 아니라 먼저 이 상수들을 그 환경에 맞춰 검토한다.

이 스크립트는 현재 `Final_LostArk` workspace에 있고 `LostArk` Git clone 자체에는 포함되지 않는다. 같은 PC의 다음 세션은 위 경로를 그대로 사용한다. 팀원이나 다른 PC에 넘길 때는 문서만 보내지 말고 `Final_LostArk/Tools/AssetPipeline`도 함께 배포하거나, 별도 작업으로 portable CLI로 옮겨야 한다.

### 7.1 전체 감사와 목록 재생성

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py audit
```

`audit`가 하는 일:

- `BG/**/StaticMesh|StaticMesh3/*.gltf` 전수 탐색
- glTF accessor에서 vertex/index/bounds/material 수집
- object name 키워드로 category 분류
- package affinity, raid, broken, 크기, 정점 수로 score 계산
- 전체 JSON/CSV/summary 작성
- curated 480개와 `valtan_candidate_allowlist.json`에 명시된 runtime selection 결정
- Windows 장경로가 예상되면 archive 경로를 `_long/<2hex>/ENV_<10hex>.wmodel`로 축약

`audit`는 `.wmodel`을 만들지 않는다. raw tree가 바뀌었을 때만 다시 실행한다.

### 7.2 `.wmodel` cook

먼저 실제 머티리얼 dependency를 object 단위로 추출한다.

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py extract-materials --scope runtime --workers 2
```

이 명령은 **현재 `environment_runtime.json`에 있는 selection 각각**의 `manifest.json`을 만들고 실제 Material Instance의 `texture_diffuse`, `texture_normal`, `texture_emissive`, `texture_orm`만 기록한다. 파일명만으로 슬롯을 추측하지 않는다. 현재 selection은 0개이므로 먼저 갤러리에서 확인한 에셋을 explicit allowlist에 넣고 `audit`을 다시 실행해야 한다.

과거 111개에 대해 object 단위 재추출, 276 material slots 중 diffuse 272개와 원본에도 diffuse가 없는 fallback 4개, deep verify 111/111을 완료한 기록은 유효한 **파이프라인 기술 검증**이다. 하지만 그 111개는 HeartRB ImportTable로 선택한 집합이 아니므로 현재 runtime selection은 아니다.

먼저 작은 범위로 검증한다.

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py cook --scope runtime --workers 8
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope runtime
```

전체 변환:

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py cook --scope all --workers 8
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope all
```

정상 `.wmodel`은 재사용하므로 중단 후 같은 명령으로 이어갈 수 있다. `--force`는 전부 다시 변환해야 할 때만 쓴다. worker 범위는 1~32다.

현재 cook 명령은 내부적으로 다음 형태를 사용한다.

```powershell
ModelAssetConverter.exe '<source.gltf>' `
  -o '<temporary.wmodel>' `
  --pretransform `
  --no-auto-textures `
  --scale 100 `
  --material-remap '<material>=<diffuse.dds>' `
  --normal-remap '<material>=<normal.dds>'
```

UModel glTF는 meter이고 현재 맵 Loader는 legacy centimeter 자산에 `0.01f`를 적용한다. 따라서 cook 때 `--scale 100`으로 centimeter로 되돌린다. 임시 파일이 header, material path, texture hash, bounds 검증을 통과한 뒤에만 archive 경로로 원자 교체한다.

### 7.3 archive 중복 정리

먼저 dry run:

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py prune
```

출력이 `missing: 0`인지 확인한 후에만 적용:

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py prune --apply
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope all
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py prune
```

마지막 dry run이 `extras: 0`, `missing: 0`이어야 한다. 2026-07-29 작업에서는 이전 장경로 layout 중복 5,615개, 898,806,272 bytes를 이 절차로 제거했다.

### 7.4 MapTool runtime 설치

```powershell
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py install-runtime
python C:\Users\user\Desktop\Final_LostArk\Tools\AssetPipeline\build_environment_library.py verify --scope runtime
```

`verify --scope runtime`은 archive의 **현재 allowlist selection 수**만 decode하고 material path, source texture hash, raw/cooked bounds까지 심층 검사한다. 설치 후에는 설치 대상 복사본과 카탈로그 행 수도 다음처럼 확인한다.

```powershell
$catalog = Get-Content `
  'C:\Users\user\Desktop\LostArk\Client\Bin\DataFiles\Map\BG_RAD_VALTAN_A.mapassets'
$declared = [int](($catalog[0] -split ' ')[-1])
$runtimeModels = Get-ChildItem `
  'C:\Users\user\Desktop\LostArk\Client\Bin\Resources\LostArk\Map\ENVIRONMENT_CURATED' `
  -Recurse -File -Filter '*.wmodel'
$selection = Get-Content -Raw `
  'C:\Users\user\Desktop\Resource_LostArk\05_Reports\EnvironmentLibrary_20260729\environment_runtime.json' |
  ConvertFrom-Json

if ($declared -ne ($catalog.Count - 1)) { throw 'catalog header/row mismatch' }
if ($runtimeModels.Count -ne @($selection).Count) { throw 'runtime model count mismatch' }
if ($declared -ne (17 + @($selection).Count)) { throw 'direct/runtime catalog mismatch' }
```

`install-runtime`는:

1. current runtime selection의 archive header를 검증한다.
2. `Map/ENVIRONMENT_CURATED/<AssetId>/<AssetId>.wmodel`로 복사한다.
3. 기존 카탈로그에서 `ENV_` 행만 교체한다.
4. 기존 발탄 17개 행은 보존한다.
5. 총 512개를 넘으면 중단한다.
6. `environment_runtime_manifest.json`을 runtime 폴더에 기록한다.

## 8. 네이밍 규칙

### 8.1 원본에서 온 이름

예시:

```text
logicalPackage = BG_RAD_KAZEROTH_W
objectName     = bg_rad_kazeroth_wallmerge06a_sm
```

관찰된 일반 구조:

```text
bg_<domain/region>_<theme>_<semantic><number><variant>_sm_<optional-tag>
```

| 토큰 | 현재 해석 |
|---|---|
| `BG` / `bg` | Background 계열 |
| `RAD` / `rad` | Raid 계열 |
| `KAZEROTH`, `ILLIAKAN`, `ABRELSHUD` 등 | 테마/지역/레이드 이름 |
| `wall`, `pillar`, `gate`, `floor` 등 | 사람이 검색할 semantic token |
| `merge` | 여러 조각이 합쳐진 큰 모델인 경우가 많음 |
| `broken`, `destroy` | 파괴/폐허 변형 |
| `01`, `06` | 계열 번호 |
| `a`, `b`, `c` | 파생 variant |
| `_sm` | StaticMesh 표식 |
| `_old` | 구형 세트 표식 |
| `_KHB`, `_KHG`, `_HEIM`, `_ALCHEMY`, `_ARTREE`, `_LOC_INT` | 제작/재질/내부 variant tag로 보되 근거 없이 의미를 확정하지 않음 |

package 끝의 `A`, `B`, `W` 등은 같은 테마의 서브 패키지 구분이다. LOD 번호라고 가정하지 않는다.

### 8.2 파이프라인이 만든 stable ID

예시:

```text
ENV_71A9A05EB5_BG_RAD_KAZEROTH_WALLMERGE06_SM
```

구성:

```text
ENV_
+ SHA-1(sourceRelative UTF-8)의 앞 10자리 대문자 hex
+ '_'
+ objectName을 [A-Za-z0-9_]로 정규화한 대문자 문자열
```

같은 object name이 다른 package에 중복돼도 source-relative hash가 달라 충돌하지 않는다. asset ID는 한번 placement에 저장한 뒤 임의로 바꾸지 않는다.

### 8.3 extraction folder와 catalog 표시 이름

```text
packageKey:
BG_RAD_KAZEROTH_W__542N9YJ2RWYTQ9ONB2H8F8MP

MapTool label:
[WALL] bg_rad_kazeroth_wallmerge06_sm / BG_RAD_KAZEROTH_W

Prototype tag:
Prototype_Component_Model_Map_ENV_71A9A05EB5_BG_RAD_KAZEROTH_WALLMERGE06_SM
```

`packageKey`의 뒤 physical stem은 추출 원본 식별자다. UI label은 category, 원본 object, logical package를 동시에 보여 검색 가능하게 만든다.

## 9. 현재까지 확인된 발탄/공용 구조물 정보

### 9.1 `BG_RAD_VALTAN_A`: 17개

```text
crystal01 계열
enterprop01 계열
floor01 계열
statue01
toolanvil01 계열
```

이 package 안에 `wall`이나 `pillar`라는 이름의 object는 없다. 더 중요하게, 이 17 object name은 HeartRB `PS`, `SL00`~`SL05` exact StaticMesh ImportTable과의 교집합이 0이다. 현재 MapTool catalog의 `valtan-confirmed` group label은 **direct package export라는 provenance**를 뜻하며 HeartRB level import 확인을 뜻하지 않는다.

### 9.2 `BG_SHS_RCARENA_*`

```text
전체 145
wall 27
pillar 7

bg_shs_rcarena_wall01a_sm_old
bg_shs_rcarena_wall03e_sm_old
bg_shs_rcarena_wall05a_sm_old_loc_int
bg_shs_rcarena_pillar01a_sm_old
bg_shs_rcarena_fence01a_sm_old
```

낡은 경기장형 벽/기둥 이름 때문에 과거 후보로 조사했다. 하지만 “경기장처럼 보인다”는 휴리스틱이며, HeartRB exact ImportTable에 없는 object를 자동 선택하지 않는다. 필요한 경우 310-card exact gallery와 별도의 비교군으로만 본다.

### 9.3 `BG_LUT_LUTOMB_*`

```text
전체 162
wall 18
pillar 18
ruin 7

bg_lut_lutomb_wall01_sm_old
bg_lut_lutomb_wall06_sm_old
bg_lut_lutomb_pillar01_sm_old
bg_lut_lutomb_brokenfloor02_sm_old
bg_lut_lutomb_brokenstair01a_sm_old
bg_lut_lutomb_brokenfence01_sm_old
```

석벽, 묘지/성곽, 파괴 바닥과 계단 조합에 유용할 수 있는 공용 세트다. 이것 역시 이름과 분위기만으로 발탄 source라고 확정하지 않는다.

### 9.4 판정이 끝난 오해

`trh_wall08_sm`은 초기 NameTable probe에 나타났지만 시각 확인 결과 큰 석벽이 아니라 비계/천막 계열이다. 과거 111개 보존본에는 trace로 남아 있어도 현재 runtime에는 설치되지 않았고, “발탄 석벽 정답”으로 다시 주장하지 않는다.

`LV_RAD_SHATTEREDBS_SL01`과 `LV_RAD_SHATTEREDBS2_SL01`도 이름 때문에 먼저 의심했지만 발탄 ZoneBase가 가리키는 level은 `LV_LUT_HEARTRB_ED_PS`였다. 스크린샷과 비슷한 이름보다 DB의 content/level 연결을 우선한다.

### 9.5 과거 heuristic runtime 111개 구성

```text
wall 34, pillar 11, gate 9, building 8, bridge 6,
ruin 8, stair 7, fence 6, cliff 8, rock 5,
foliage 3, floor 3, decor 3
```

대표:

```text
BG_RAD_KAZEROTH_W / wallmerge01~06
BG_RAD_ILLIAKAN_E / wall01~01e
BG_RAD_ILLIAKAN_D / floorwall01~02
BG_RAD_KAZEROTH_T / pillarmerge 계열
BG_RAD_KAZEROTH_J / gate 계열
BG_LUT_LUTOMB_D / broken 계열
BG_KUR_STONE_C / mountain 계열
BG_RAD_NAROK_C / stone_cliff 계열
```

이 집합은 object name, category, 크기, package affinity 점수로 만든 visual search set이다. 머티리얼/텍스처 복구와 deep verify에는 성공했지만 level provenance를 제공하지 않는다. 현재 `environment_runtime_111.json`은 historical artifact이고 `environment_runtime.json`은 explicit allowlist를 반영한 0개다.

### 9.6 HeartRB exact 주요 구조물

시각 정본은 다음 이미지와 사용자 제공 스크린샷이다.

```text
C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Map/ValtanMap.png
```

exact ImportTable과 render를 통해 먼저 볼 주요 object는 다음과 같다.

| 용도 | Gallery ID | object | exact level |
|---|---|---|---|
| 중앙 링/원통 구조 | `HRT_885CC3ECA0A0` | `bg_pvp_retown_floor01_sm` | `SL00` |
| 원형 경기장 석판 | `HRT_EECFF05E322B` | `lv_lut_heartrb_floor01_sm` | `SL00`, `SL04` |
| 굵은 수평 체인 A | `HRT_A468FDDF2117` | `bg_fat_karlajavil_chain01a_sm` | `SL00`, `SL03` |
| 굵은 수평 체인 B | `HRT_34F4E439BFFD` | `bg_fat_karlajavil_chain01b_sm` | `SL03`, `SL04` |
| 연출용 수직 체인 | `HRT_8DAA56582FA4` | `cine_kyg_chain01_sm` | `SL03` |
| 폐성채 큰 벽 조각 | `HRT_3F3E967FA2A1` | `lv_lut_heartrb_churchpillar02_sm` | `SL01`, `SL02`, `SL03`, `SL05` |
| 폐성채 벽 조각 | `HRT_6E3853824496` | `lv_lut_heartrb_churchpillar01_sm` | `SL05` |
| 배경 교회탑 A | `HRT_6384BB0B57D9` | `bg_lut_lucastle_churchtower03_sm_ksh` | `SL04` |
| 배경 교회탑 B | `HRT_58AE22C1F7E8` | `bg_lut_lucastle_churchtower10_sm_ksh` | `SL04` |
| 장거리 성채 기둥 | `HRT_6909FE299586` | `bg_lut_zamount_fortress08_sm_lkj` | `SL04` |
| 장거리 성채 장식 | `HRT_81B4639C3A39` | `bg_lut_zamount_fortressdeco01b_sm_kem` | `SL04` |

스크린샷의 배치는 단일 원형 바닥이나 단일 거대 BG가 아니라 다음 층을 조립한 결과다.

```text
1. 중앙 원형 전투 바닥
2. 파손된 내곽 링과 갈라진 바닥 조각
3. 중심에서 바깥으로 뻗는 방사형 기둥·버트리스·짧은 벽
4. 높고 두꺼운 외곽 성벽·게이트·수직 폐허
5. 외곽 일부가 열린 낭떠러지와 멀리 보이는 배경 지형
```

큰 골격은 원형/방사형으로 읽혀야 하지만 완전한 좌우대칭으로 복제하지 않는다. 벽 높이, 파손 정도, 틈, 잔해는 level actor Transform과 streaming state를 복구해야 정확해진다. 254개 exact mesh dependency도 placement 자체와 동일하지 않다.

## 10. 에셋 검색법

발탄 HeartRB를 찾을 때는 먼저 exact gallery를 사용한다.

```text
C:/Users/user/Desktop/LostArk/.codex_tmp/
  valtan_heart_rb_gallery_20260729/index.html
```

`레벨 정확 참조` filter는 `PS`, `SL00`~`SL05` ImportTable의 exact StaticMesh만 남긴다. 카드의 `level`, `exactImportPaths`, `sourceGltf`, ISO/TOP를 확인하고 체크박스로 선택 JSON을 내보낸다. Gallery ID는 검수 ID이며 MapTool runtime ID로 자동 사용하지 않는다.

HeartRB 외의 전체 archive 탐색은 파일명보다 CSV를 우선한다. 아래 결과는 **후보 검색**이지 level provenance가 아니다.

```powershell
$inventory = Import-Csv `
  'C:\Users\user\Desktop\Resource_LostArk\05_Reports\EnvironmentLibrary_20260729\environment_inventory.csv'

# 이름/패키지에서 wall, arena, tomb 검색
$inventory | Where-Object {
  $_.objectName -match 'wall|arena|tomb' -or
  $_.logicalPackage -match 'RCARENA|LUTOMB'
} | Select-Object assetId,logicalPackage,objectName,category,maxExtent,archiveRelative

# 특정 package 전수
$inventory | Where-Object logicalPackage -like 'BG_SHS_RCARENA*' |
  Sort-Object objectName

# 큰 wall 우선
$inventory | Where-Object {
  $_.category -eq 'wall' -and $_.logicalPackage -notlike 'BG_WALLPAPER*'
} | Sort-Object {[double]$_.maxExtent} -Descending |
  Select-Object -First 100
```

선택한 행의 `sourceGltf`는 원본 형상, `archiveRelative`는 전체 `.wmodel` 아카이브 상대경로다.

검색 증거의 우선순위:

```text
1. exact level ImportTable + raw glTF render
2. level ExportTable actor/component property와 Transform
3. package/object 이름 및 source package affinity
4. 크기/정점 수/category heuristic
5. 분위기가 비슷하다는 육안 추측
```

하위 증거가 상위 증거와 충돌하면 상위를 따른다. 이름 후보를 runtime에 넣으려면 사람이 render를 확인하고 explicit allowlist에 evidence와 함께 기록한다.

## 11. ModelAssetConverter 사용법

실행 파일:

```text
C:/Users/user/Desktop/LostArk/Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe
```

같은 `Bin`의 Assimp/zlib 계열 DLL을 함께 둔다.

### 11.1 정적 glTF/FBX

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  '<source.gltf-or-fbx>' `
  -o '<output.wmodel>' `
  --pretransform `
  --no-auto-textures
```

`--pretransform`은 정적 맵 hierarchy transform을 vertex에 적용한다. skeletal/animation 모델에는 사용하지 않는다.

UModel glTF에는 반드시 `--scale 100`을 추가한다. 일반 FBX에 기계적으로 적용하지 않는다.

### 11.2 텍스처 자동/수동 연결

재질명과 텍스처명이 신뢰 가능하면:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  '<source.fbx>' -o '<output.wmodel>' `
  --pretransform `
  --texture-root '<textures-folder>'
```

`dummy_material_0`처럼 자동 매칭 근거가 없으면 자동 탐색을 끄고 명시한다.

단, UModel을 `-notex`로 export해 생긴 `dummy_material_N`과 0.3 회색 `baseColorFactor`는 실제 diffuse가 아니다. WMaterial v2는 상수 색을 저장하지 않으므로 이 상태로 cook하면 textureless 모델이 된다. 실제 Material Instance를 다시 export하고 `.props.txt`의 명시 슬롯을 사용한다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  '<source.fbx>' -o '<output.wmodel>' `
  --pretransform --no-auto-textures `
  --material-remap 'dummy_material_0=C:\Asset\Textures\asset_d.png' `
  --normal-remap 'dummy_material_0=C:\Asset\Textures\asset_n.png'
```

지원 수동 슬롯:

```text
--material-remap, --normal-remap, --specular-remap,
--emissive-remap, --opacity-remap, --orm-remap,
--metallic-remap, --roughness-remap, --ao-remap
```

현재 renderer가 화면에 직접 사용하는 중심 슬롯은 diffuse와 normal이다. `_s`를 표준 ORM이라고 추측하지 않는다.

### 11.3 분리 W-format pack

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe pack `
  '.\Asset.wmesh' `
  -o '.\Asset.wmodel' `
  --material '.\Asset.wmat' `
  --skeleton '.\Asset.wskel' `
  --anim-dir '.\anims'
```

발탄 `MN_RPBF_01.wmodel`은 이 방식으로 skeleton과 27개 animation을 묶었다.

### 11.4 결과 검사

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe info '<Asset.wmodel>'
```

파이프라인의 최소 header 검증:

```text
offset 0  : WINT
offset 16 : WMOD
file size : 64 bytes 이상
```

`info`로 section, mesh/material, skeleton, animation과 texture slot까지 확인한 뒤 배포한다.

## 12. LostArk 폴더에 적용하는 법

### 12.1 runtime root

`CRuntimeAssetRoot::Get()`의 현재 우선순위:

1. 환경 변수 `LOSTARK_SHARED_ASSET_ROOT`
2. 없으면 `Client.exe` 폴더의 `Resources/LostArk`

기본 로컬 실행에서는 다음이 root다.

```text
C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk
```

다른 Drive pack을 직접 쓰려면 실행 전:

```powershell
$env:LOSTARK_SHARED_ASSET_ROOT = 'D:\SharedLostArk\LostArk'
```

카탈로그에는 이 root 기준 상대경로만 쓴다. 절대경로와 `../Bin`을 넣지 않는다.

### 12.2 `.mapassets` 문법

```text
LOSTARK_MAP_ASSET_CATALOG <version> "<areaId>" <count>
"<assetId>" "<label>" "<relativeWModel>" "<prototypeTag>" <sx> <sy> <sz> <anchor> "<groupId>" "<groupLabel>" "<evidence>"
```

version 1은 metadata 3개가 없는 legacy 문법이고, 현재 version 2는 `groupId`, `groupLabel`, `evidence`를 필수로 읽는다.

현재 예:

```text
LOSTARK_MAP_ASSET_CATALOG 2 "BG_RAD_VALTAN_A" 17
"BG_RAD_VALTAN_CRYSTAL01_SM" "Valtan Crystal 01" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_CRYSTAL01_SM/BG_RAD_VALTAN_CRYSTAL01_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_CRYSTAL01_SM" 1 1 1 BottomCenter "valtan-confirmed" "Valtan Dedicated (17)" "BG_RAD_VALTAN_A direct StaticMesh export"
```

검증 불변식:

- header count와 실제 행 수 일치
- 최대 512행
- asset ID 중복 금지
- Prototype tag 중복 금지
- 상대경로이며 runtime root 밖으로 탈출 금지
- 확장자 `.wmodel`
- scale 유한수이고 각 축 양수
- anchor는 `Origin` 또는 `BottomCenter`
- version 2의 group ID는 영숫자/`_-.`만 허용하고 label/evidence는 비어 있지 않아야 함
- 파일이 실제로 존재
- trailing token 없음

파일 하나라도 없으면 Catalog 전체가 ready 상태가 되지 않고 `LEVEL::ASSET_TEST` 로딩도 실패한다.

### 12.3 Loader와 스케일

`CLoader::Ready_For_Level_AssetTest()`가 카탈로그 전체를 읽어 각 행을 다음 조건으로 등록한다.

```text
MODEL::NONANIM
PreTransform = XMMatrixScaling(0.01f, 0.01f, 0.01f)
Prototype level = LEVEL::ASSET_TEST
```

발탄 캐릭터 `.wmodel`은 별도의 `0.0001f`를 사용한다. map과 character 스케일을 섞지 않는다.

UModel glTF로 새로 cook한 맵은 `--scale 100`을 적용해 archive에 centimeter로 저장해야 한다. 최종 계약은 `glTF meters × 100 × Loader 0.01 = 게임 월드 meters`다.

현재 구조는 eager preload이므로 19,259개 전체나 gallery 310개 전체를 카탈로그에 넣으면 로딩 시간과 메모리가 폭증한다. 전체 아카이브와 gallery는 검색/검수 정본이고, runtime은 explicit allowlist로 승인한 작업 세트다. 현재 작업 세트는 direct 17 + allowlist 0이다.

### 12.4 실행 확인

```text
1. Engine Debug|x64 build
2. UpdateLib.bat Debug
3. Client Debug|x64 build
4. Client 실행
5. Logo에서 F2 -> LEVEL::ASSET_TEST
6. F1 -> MapTool
7. Palette에서 [WALL], [RUIN], package/object 이름 검색
8. 행 선택 -> 월드 클릭 배치
9. Save -> 위치 변경/삭제 -> Load -> 복원 확인
```

F3는 `TEST_LEVEL2` 진입 키다. MapTool 대상 레벨은 F2의 `ASSET_TEST`다.

실행 중인 `Client.exe`가 `Client.exe`와 `Engine.dll`을 잠근다. `LNK1104` 또는 `MSB3021/MSB3027`이 나오면 코드 오류로 오해하지 말고 Client를 종료한 뒤 다시 링크한다.

## 13. 빌드와 배포

```powershell
$MSBuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'

& $MSBuild .\Engine\Default\Engine.vcxproj `
  /m /t:Build /p:Configuration=Debug /p:Platform=x64 /v:minimal

.\UpdateLib.bat Debug

& $MSBuild .\Client\Default\Client.vcxproj `
  /m /t:Build /p:Configuration=Debug /p:Platform=x64 /v:minimal
```

배포 구분:

| 내용 | 배포 |
|---|---|
| C++/HLSL/Markdown/카탈로그 | Git 일반 또는 기존 LFS 규칙 |
| `Tools/ModelAssetConverter` 실행 번들 | Git LFS |
| `Client/Bin/Resources/LostArk/**` | 공유 Drive/zip |
| 전체 2.91 GB 환경 archive | `Resource_LostArk` 보관/별도 공유 |
| `.mapplacements` | 팀이 합의한 Scene 데이터로 Git/LFS |

공유 pack은 runtime root 내부 상대경로를 보존해야 한다.

## 14. 실패 원인 빠른 판별

| 증상 | 먼저 볼 것 |
|---|---|
| Catalog header invalid | header count, version, area ID |
| Catalog validation failed | 해당 ID 파일 존재, 중복 ID/tag, 확장자, root 탈출 |
| AssetTest 로딩 실패 | 현재 catalog header/row와 각 `.wmodel` 존재 확인; 현재는 17행 |
| 모델이 검거나 하얌 | `-notex`, 빈 material path, runtime texture 누락 여부 확인 |
| Hierarchy에는 있는데 화면에 없음 | `ModelAssetConverter info`로 diffuse 경로와 실제 파일 확인. 상세는 `gotchas.md` |
| 크기가 100배/10000배 틀림 | UModel glTF `--scale 100`, map `0.01f`, character `0.0001f` 구분 |
| 이름은 wall인데 구조물이 아님 | Wallpaper/소품 오분류 및 실제 preview 확인 |
| 발탄 검색에 17개만 나옴 | mesh package ExportTable만 본 것; HeartRB level ImportTable exact gallery 확인 |
| exact 후보가 254인데 gallery는 310 | source duplicate를 보존한 exact 293 + direct 비교군 17 |
| ImportTable 후보가 그럴듯하지만 count 부족 | header `ImportCount`와 `resolvedStringCount` 완전 일치 전에는 폐기 |
| 중앙 원형이 PVP package에 있음 | level은 공용 package mesh를 import함; prefix/폴더로 provenance를 판단하지 않음 |
| 거대한 배경 성채 한 덩어리가 안 보임 | 여러 mesh + actor Transform composite; actor placement lane 필요 |
| 벽이 아예 없음 | Terrain/Landscape, level actor, 공용 package, moving/skeletal actor lane 확인 |
| UModel export가 폭증 | package 전체 export 중단, object 단위 allowlist로 재시도 |
| 경로가 너무 길어 cook 실패 | inventory의 `_long/<hash>` archive 경로 사용 |
| git status에 `.wmodel`이 안 보임 | 정상: `.gitignore` 및 Drive pack 대상 |
| 링크가 안 됨 | 실행 중 `Client.exe` 종료 |

## 15. 다음 세션의 시작 순서

```text
1. 이 문서를 읽는다.
2. HeartRB gallery index.html을 열고 `레벨 정확 참조` 254-name/293-card 범위를 먼저 본다.
3. 필요한 카드의 exact level/path와 ISO/TOP를 확인하고 선택 JSON을 내보낸다.
4. 선택 source를 environment inventory/archive와 연결하고 explicit allowlist에 asset ID/group/evidence를 기록한다.
5. audit 후 environment_runtime.json 수가 allowlist와 같은지 확인한다. stale environment_summary의 runtimeAssets를 믿지 않는다.
6. 텍스처가 필요한 object는 `extract-materials --scope runtime` 또는 `-obj=<name>`으로 단일 재추출한다.
7. Material Instance props의 명시 슬롯만 texture remap하고 UModel glTF는 `--scale 100`으로 cook한다.
8. verify --scope runtime 후 install-runtime을 실행한다.
9. 카탈로그 행 수가 direct 17 + current selection이고 runtime model 수도 selection과 같은지 확인한다.
10. F2 -> F1에서 실제 배치/저장/재로드한다.
```

에셋이 생성됐지만 안 보이거나 모두 회색이면 `.md/GB/07-29/gotchas.md`를 먼저 읽는다. `CCookedModel`로 우회하지 말고 `CModel -> CMaterial` 통합 경로에서 해결한다.

하지 말 것:

```text
- BG_RAD_VALTAN_A 17개가 발탄 맵 전체라고 다시 결론내기
- BG/RAD/LUT/PVP 같은 source prefix만 보고 level 사용 여부를 판단하기
- NameTable에 이름이 있다는 사실을 ImportTable exact reference로 부르기
- header ImportCount보다 짧은 그럴듯한 import segment를 완전 복구라고 부르기
- trh_wall08_sm을 스크린샷 석벽이라고 다시 확정하기
- 19,259개를 Loader 카탈로그에 전부 등록하기
- gallery 310개를 사용자 검수 없이 전부 runtime에 등록하기
- textureless bulk WModel을 최종 재질 복원본이라고 부르기
- Catalog만 Git에 올리고 Drive runtime pack 공유를 빼먹기
- Landscape/placement/effect를 StaticMesh 이름 검색만으로 해결됐다고 보기
```

## 16. 관련 문서와 코드

```text
팀 규칙
C:/Users/user/Desktop/LostArk/AGENTS.md
C:/Users/user/Desktop/LostArk/CLAUDE.md

기존 MapTool 계획
C:/Users/user/Desktop/LostArk/.md/GB/07-29/
  2026-07-29_LOSTARK_ASSET_PALETTE_PICKING_RENDER_PLAN.md
  2026-07-29_LOSTARK_SCENE_KIND_PLACEMENT_VALTAN_PLAN.md

HeartRB exact 조사/갤러리
C:/Users/user/Desktop/LostArk/.codex_tmp/valtan_heart_rb_gallery_20260729/
  README.md
  index.html
  assets.json
  ps_recovered_imports.json
  sl00_recovered_imports.json ... sl05_recovered_imports.json

레벨 DB와 UModel 메모리 조사 원본
C:/Users/user/Desktop/Final_LostArk/_work/valtan_probe_20260729/
  EFTable_Npc.db
  tables/EFTable_IntegrateDungeon.db
  tables/EFTable_ZoneBase.db
  scan_umodel_imports.py

맵 level logical/physical inventory
C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/Inventory_20260728/
  map_level_packages.csv

Converter 상세
C:/Users/user/Desktop/LostArk/Tools/ModelAssetConverter/README.md

런타임 코드
C:/Users/user/Desktop/LostArk/Client/Private/RuntimeAssetRoot.cpp
C:/Users/user/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp
C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp
C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp
```

전체 BG archive 수치는 `environment_inventory.json`, HeartRB exact 수치는 gallery `assets.json`과 일곱 `*_recovered_imports.json`, 현재 runtime selection은 `valtan_candidate_allowlist.json`과 `environment_runtime.json`, 현재 설치 행은 `.mapassets`를 정본으로 한다. 바이너리 유효성은 `verify`, 게임 적용 계약은 현재 `MapAssetCatalog.cpp`와 `Loader.cpp`를 따른다. 서로 다르면 파일 timestamp와 생성 순서를 확인하고 최신 정본에 맞춰 이 문서를 즉시 역갱신한다.
