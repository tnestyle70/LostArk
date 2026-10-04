# LostArk UE3 Landscape Extractor

베른성 `Landscape`를 충돌 높이로 대체하지 않고, 원본 UE3 패키지의
`LandscapeProxy`, `LandscapeComponent`, Heightmap, Weightmap, 레이어 할당,
재질 인스턴스를 직접 해독해서 보존하는 도구다.

## 검증된 원본 레이어 런타임 후보

`build_source_landscape_candidate.py`는 기존 추출기의 `SourceRaw`와 원본 셰이더를
대조한 레이어 계약, 원본 DDS mip 전달 기록으로 `bg-source-landscape-opaque`
재질 행과 source-painted WModel 후보를 만든다. 기존 추출기의 256 베이크와
절벽 side-projection은 이 후보의 레이어 입력으로 사용하지 않는다.

```powershell
python Tools/LandscapeExtractor/build_source_landscape_candidate.py `
  --source-root out/BernTerrainRestore_20260930/source-support/full-cook/SourceRaw `
  --source-contract Tools/LandscapeExtractor/SourceContracts/BernSourceLandscape.v1.json `
  --texture-closure out/BernTerrainRestore_20260930/source-support/native-layer-inputs/texture-closure.json `
  --output out/BernTerrainRestore_20260930/source-support/native-bern42-candidate
```

한 컴포넌트를 먼저 검증하려면 `--only-asset <assetId>`를 추가한다. 출력은
저장소 `out` 아래의 새 폴더만 허용한다. `candidate-manifest.json`이 생성된
후보만 검증 완료 산출물이며, 이 도구는 제품 파일 설치·publish·배치 변경을
수행하지 않는다. `--geometry-only`는 재질 계약 전에 형상만 검증하는 모드다.
이미 검증된 DDS 전달 기록이 없으면 `--texture-closure` 대신
`--package-root <ReleasePC/Packages> --umodel <umodel_lostark_v7.exe>`를 지정한다.
이 경로는 SourceRaw의 원본 dependency metadata와 package hash를 확인하고
기존 `extract_ue3_texture_mips`로 실제 활성 D/N 입력의 전체 압축 mip를 추출한다.
`--source-contract`는 원본 셰이더 검증 결과이며 베른 정본은 Git 관리하는
`SourceContracts/BernSourceLandscape.v1.json`에 보존한다. 일반 Landscape
재추출이나 이 builder는 ShaderCache의 static key, 레이어별 compiled blend mode와
수식을 자동 복원하지 않는다. SourceRaw만 남기고 계약을 삭제한 경우 임의 기본값으로
대체할 수 없으며, 검증된 계약을 복구하거나 원본 ShaderCache 감사를 다시 수행해야 한다.

- 모든 삼각형에 component UV0를 부여하고 원본 위치·packed normal·tangent·
  winding·hole topology와 현재 설치 모델의 위치·topology를 대조한다.
- 원본 Height/Weight BGRA와 모든 기존 mip를 그대로 DDS에 보존한다.
  공통 diffuse/normal은 전달 기록의 원본 압축 DDS hash를 확인하며 재압축하거나
  누락 mip를 생성하지 않는다.
- 재질은 최대 6개 레이어, 2개 weightmap, 1개 heightmap이다. 원본 셰이더에서
  비활성인 normal 입력은 런타임 intensity를 0으로 기록하고 원본 unused scalar는
  manifest에 보존한다. `__DataLayer__`는 기존 hole topology를 유지한다.
- WModel의 `LANDSCAPE_BAKED` 슬롯 이름과 기존 embedded PNG 두 개는 유지한다.
  그 PNG는 변경 없는 모델 의존성이며 native family는 typed 원본 DDS를 소비한다.
  manifest의 `resources`만 새 후보이고 `unchangedDependencies`는 기존 파일이다.
- 이 출력은 수치·리소스 검증 결과다. 런타임 소비 코드, Area publish와 사용자
  화면 확인은 별도로 수행해야 한다.

## 원본 Landscape 조명 후보

`build_source_landscape_lighting_candidate.py`는 현재 원본 42개 component의 native
FLightMap2D와 각 component 자신의 ShadowMap2D를 읽는다. RNM 두 장과 G8 그림자의
원본 전체 mip를 추출하고 기존 `bakedLighting` 및 `placementLighting` 필드만 담은
`mapmaterials.patch.json`을 만든다. geometry, UV0 표면 반복, 배치 TRS는 변경하지 않는다.

inline Texture2D BulkData는 flags 0(raw) 또는 0x80(LZ4)에 따라 디코드한다.
압축 컨테이너 길이가 decoded 길이와 같더라도 flags 0x80이면 UE bulk header와
block table을 해석한다. 길이 비교로 raw를 추정하지 않으며 다른 flags는 거부한다.
Height/Weight BGRA와 G8 shadow 모두 이 동일 decoder에 native flags를 전달한다.

```powershell
python Tools/LandscapeExtractor/build_source_landscape_lighting_candidate.py `
  --source-contract Tools/LandscapeExtractor/SourceContracts/BernSourceLandscape.v1.json `
  --package-root "C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages" `
  --umodel out/BernCliffCoverage20261004/tool/umodel_lostark_v7.exe `
  --uv-proof out/BernLandscapeLightmapUvReview20261004/lc622-lightmap-uv-cpu-proof.json `
  --output out/BernLandscapeLightingCandidate
```

`--uv-proof`는 원본 EFEngine의 LandscapeLightmapScaleBias CPU owner, native shader
binding과 float32 산식을 대조한 전달 기록이다. 현재 engine hash와 각 component의
62/31/2 grid 및 proxy resolution4가 일치해야 한다. 이 범위의 정규화 UV0는
scale0.953125/bias0.015372984111309052를 거친 뒤 각 component의 RNM·shadow atlas와
각각 합성된다. 원본이나 입력이 달라지면 기존 상수를 재사용하지 않고 중단한다.

`--texture-cache`는 DDS와 `.dds.receipt.json`이 함께 있는 이전 추출 폴더를 선택적으로
재사용하며 source serial·package·DDS hash를 다시 확인한다. 출력 `Resources`는
`Map/Lighting/Bern` 상대 경로를 사용한다. `candidate-manifest.json`은 texture hash,
각 component 원본 근거와 합성된 좌표를 보관한다. 현재 shadow의 penumbraWidth0.05와
중심 처리에는 기존 `PROJECT_ADAPTER` 계약을 사용하며 원본 CPU 복원값으로 표기하지 않는다.

geometry·표면까지 다시 만들 때는 위 `build_source_landscape_candidate.py`에
`--lighting-candidate <조명 후보의 candidate-manifest.json>`을 전달한다. 선택한
component의 RNM·shadow resource와 placementLighting을 결과에 함께 보존한다.
이 인자를 생략한 표면 전용 후보로 현재 조명 필드를 덮어쓰지 않는다. 최종 설치는
최신 저작본에서 stable asset/source-placement ID별 해당 필드만 병합한 뒤 공식
Area publisher를 사용한다. 후보 생성은 제품 설치·publish·Client 실행을 수행하지 않는다.

## 출력 계약

- `SourceRaw`가 원본 해독 결과의 정본이다.
  - 모든 Heightmap/Weightmap mip의 무손실 BGRA8
  - 확인용 mip0 PNG
  - 42개 컴포넌트의 SectionBase, Scale/Bias, CachedLocalBox, 레이어 할당
  - Proxy 2개, Component 42개, CollisionComponent 42개,
    로컬 Texture2D 85개 전체 export serial
  - LAND01/LAND02의 Landscape 재질 인스턴스 78개 전체 serial과 tagged property
  - 마스터 Landscape 재질과 연결 텍스처의 DDS/TGA/속성 파일
  - 마스터가 참조하는 Texture2D 13개의 원본 압축 mip serial
- `SourceDerived`는 현재 프레임워크에서 표시하기 위한 파생 결과다.
  - 42개 glTF와 `.bin`
  - 256x256 baked diffuse/normal/hole mask
  - 원본 `layercliff` 텍스처·파라미터와 Heightmap packed normal을 이용한
    결정적 side-projection 절벽 베이크
  - `__DataLayer__ > 170`인 top-left 샘플이 소유하는 quad의 두 삼각형 제거
- `Resources`는 `CModel -> CMaterial` 경로로 로드하는 42개 `.wmodel` 팩이다.
- `DataFiles`는 별도 MapTool 영역
  `LV_BER_BERNCASTLE_LANDSCAPE`의 catalog/placement 문서다.

표시용 베이크는 UE3 Landscape 머티리얼 그래프를 실행한 결과가 아니다.
원본 Weightmap과 재질 파라미터는 항상 `SourceRaw`를 기준으로 판단한다.

## 실행 예시

저장소 루트에서 실행한다. `ModelAssetConverter`는 한글이 포함된 절대 경로를
직접 받지 못하므로 도구가 저장소 내부 경로를 상대 경로로 전달한다.

```powershell
python Tools/LandscapeExtractor/extract_ue3_landscape.py `
  --umodel "C:\Users\USER\Documents\Codex\2026-07-28\c-programdata-smilegate-games-lostark\outputs\UModel_LOSTARK\umodel_lostark_v7.exe" `
  --package-root "C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages" `
  --converter "Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe" `
  --output "_work\BERN_CASTLE_LANDSCAPE_VERIFIED_2026-08-01" `
  --bake-resolution 256 `
  --expect-components 42 `
  LV_BER_BERNCASTLE_T_LAND01 `
  LV_BER_BERNCASTLE_T_LAND02
```

출력 폴더가 이미 있으면 덮어쓰지 않고 중단한다. 생성은 임시 stage에서 수행하고
모든 검증을 통과한 경우에만 최종 출력 폴더로 원자 전환한다.

표시용 베이크 입력은 UModel을 `-dds`로 실행한 `SourceRaw/MasterMaterial/dds`만
사용한다. 기본 TGA 출력은 교차검증 자료로 보존하지만 `umodel.cfg`의
`ExportDdsTexture` 값에 따라 베이크 픽셀이 바뀌지 않도록 입력에서는 제외한다.

## Client 설치 위치

검증된 결과의 런타임 부분은 다음 위치에 설치한다.

```text
Client/Bin/Resources/Map/LV_BER_BERNCASTLE_T/Landscape/
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_LANDSCAPE.mapassets
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_LANDSCAPE.mapplacements
```

현재 작업 중인 맵을 자동으로 바꾸지 않는다. 베른성 Landscape만 MapTool에서
열어 보려면 `Client/Bin/DataFiles/Map/ACTIVE.maparea`를 다음 한 줄로 바꾸고
클라이언트를 다시 시작한다.

```text
LOSTARK_MAP_AREA_SELECTION 1 "LV_BER_BERNCASTLE_LANDSCAPE"
```

기존 맵으로 돌아갈 때는 이 파일의 원래 area ID를 복원한다.

## 검증 항목

- 컴포넌트 42개와 `.wmodel` 42개
- Heightmap 42개, Weightmap 42개
- Landscape 재질 인스턴스 78개, 레이어 할당 139개
- CollisionComponent 42개와 로컬 Texture2D 85개 serial
- Render Heightmap과 CollisionHeightData 166,698개 샘플 불일치 0
- 마스터 재질 Texture2D 의존성 13개, 원본 mip 행 133개
- 컴포넌트 내부 subsection 경계 불일치 0
- LAND01/LAND02 교차 경계를 포함한 70개 인접 경계, 4,410개 높이 샘플 불일치 0
- 4,410개 공유 경계 packed normal 불일치 0
- 7개 원본 빈 grid cell 유지
- 4개 `__DataLayer__` hole mask 보존 및 hole quad render topology 제거
- 모든 `.wmodel`의 WINT/WMOD header, material path, texture pack을 converter `info`로 검증

## 기본 베이크 출력의 한계와 제품 연결

- 기본 추출 명령의 색/노멀 출력은 Weightmap을 이용한 결정적 표시용 베이크다.
  베른의 원본 레이어 소비는 위 후보 builder와 family14를 연결한 `CModel -> CMaterial`
  경로를 사용하며 기본 베이크 자체가 원본 셰이더로 바뀌지는 않는다.
- cooked 패키지에 부모 material expression graph의 완전한 연산 연결이 남아 있지 않아
  cliff 혼합 임계값과 projection은 원본 텍스처·파라미터·source normal을 사용하는
  결정적 근사다. specular와 reflection도 실행하지 않으므로 원본 최종 외형과
  동일하다고 주장하지 않는다.
- hole mask는 UE3의 strict `__DataLayer__ > 170` 분류를 보존하고, top-left 샘플이
  소유하는 quad의 두 render 삼각형을 제거한다.
- collision/nav 전용 geometry는 아직 생성하지 않는다.
- 기본 catalog는 Landscape42개만 담은 독립 검증 영역이다. 제품 베른은 기존
  shard-set의 안정적인 asset ID를 사용하며 native 후보도 배치·shard 구성을 바꾸지 않는다.
- `CVIBuffer_Terrain`의 8비트 BMP 경로와
  `LandscapeHeightfieldCollisionComponent` 우회 경로는 사용하지 않는다.
- 베른성 바닥 외형을 함께 구성하는 별도 StaticMesh 바닥, DecalActor, foliage,
  light/shadow map은 이 Landscape 전용 팩의 범위가 아니다.
- 독립 catalog의 자동 placement 생성과 제품 베른의 기존 배치는 구분한다.
  제품 적용은 기존 Area publisher로 수행하며 stable placement ID와 TRS를 보존한다.
