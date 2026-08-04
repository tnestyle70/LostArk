# LostArk UE3 Landscape Extractor

베른성 `Landscape`를 충돌 높이로 대체하지 않고, 원본 UE3 패키지의
`LandscapeProxy`, `LandscapeComponent`, Heightmap, Weightmap, 레이어 할당,
재질 인스턴스를 직접 해독해서 보존하는 도구다.

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

## 현재 한계

- UE3 원본 다층 Landscape 셰이더를 현재 `CMaterial`이 직접 실행하지 못하므로
  런타임 색/노멀은 Weightmap을 이용한 결정적 표시용 베이크다.
- cooked 패키지에 부모 material expression graph의 완전한 연산 연결이 남아 있지 않아
  cliff 혼합 임계값과 projection은 원본 텍스처·파라미터·source normal을 사용하는
  결정적 근사다. specular와 reflection도 실행하지 않으므로 원본 최종 외형과
  동일하다고 주장하지 않는다.
- hole mask는 UE3의 strict `__DataLayer__ > 170` 분류를 보존하고, top-left 샘플이
  소유하는 quad의 두 render 삼각형을 제거한다.
- collision/nav 전용 geometry는 아직 생성하지 않는다.
- 이 catalog는 Landscape 42개만 담은 별도 검증 영역이다. 기존 베른성 정적 에셋
  catalog와 합치면 현재 `MAX_ASSET_COUNT == 512` 제한을 넘기므로 바로 병합하지 않는다.
- `CVIBuffer_Terrain`의 8비트 BMP 경로와
  `LandscapeHeightfieldCollisionComponent` 우회 경로는 사용하지 않는다.
- 베른성 바닥 외형을 함께 구성하는 별도 StaticMesh 바닥, DecalActor, foliage,
  light/shadow map은 이 Landscape 전용 팩의 범위가 아니다.
- 자동 placement 생성은 현재 Debug MapTool 경로다. Release 게임 레벨 연결은
  별도 런타임 배선이 필요하다.
