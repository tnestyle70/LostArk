# 2026-08-01 베른성 Landscape 원본 추출 결과

## 1. 결론

베른성 바닥은 충돌 높이 대체물이 아니라 원본 UE3 Landscape 데이터에서 직접
추출했다. LAND01과 LAND02의 42개 컴포넌트를 모두 해독해 원본 데이터, 표시용
glTF/텍스처, `CModel`용 `.wmodel`, MapTool catalog/placement까지 생성했다.

검증된 최종 팩:

```text
_work/BERN_CASTLE_LANDSCAPE_VERIFIED_2026-08-01/
```

Client 설치 위치:

```text
Client/Bin/Resources/LostArk/Map/LV_BER_BERNCASTLE_T/Landscape/
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_LANDSCAPE.mapassets
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_LANDSCAPE.mapplacements
```

## 2. 사용한 원본

| 논리 패키지 | 물리 패키지 | Landscape 컴포넌트 | 로컬 Landscape 재질 |
|---|---|---:|---:|
| `LV_BER_BERNCASTLE_T_LAND01` | `978TBWF8XBWFNI4MT9W8XT8T94NP6D.upk` | 22 | 44 |
| `LV_BER_BERNCASTLE_T_LAND02` | `978TBWF8XBWFNI4MT9W8XT8T94NP6K.upk` | 20 | 34 |
| 합계 |  | 42 | 78 |

공통 Landscape GUID, Proxy 위치 `(-4032,-4032,0)`, DrawScale3D
`(64,64,128)`, ComponentSizeQuads `62`, SubsectionSizeQuads `31`,
NumSubsections `2`를 실데이터에서 확인했다.

## 3. 구현 내용

추출기는 `Tools/LandscapeExtractor/extract_ue3_landscape.py`에 추가했다.

1. LostArk UPK를 AES/LZ4 경로로 해독한다.
2. Tagged Property에서 Proxy, Component, SectionBase, Heightmap/Weightmap 참조,
   Scale/Bias, 레이어 할당, 재질 인스턴스를 읽는다.
3. Texture2D BulkData의 모든 mip를 BGRA8로 무손실 저장한다.
4. Heightmap의 `height16 = R*256+G`,
   `localZ = (height16-32768)/128` 계약으로 63x63 정점을 만든다.
5. Heightmap B/A의 packed normal을 해독해 UE 법선 `(X,Y,Z)`를 Client
   `(X,Z,-Y)`로 변환한다. 타일별 중앙차분 법선은 사용하지 않는다.
6. 7개 Landscape 레이어 텍스처와 Weightmap으로 표시용 diffuse/normal을 굽는다.
7. glTF를 `ModelAssetConverter --pretransform --scale 100`으로 `.wmodel`로 cook한다.
8. 안정적인 asset ID와 placement ID로 42개 catalog/placement를 만든다.
9. 전체가 통과하기 전에는 최종 출력 폴더를 확정하지 않는다.

`CCookedModel`, `CBinaryAssetObject`, `CVIBuffer_Terrain`을 새 경로로 만들지 않았고,
팀 규칙대로 기존 `CModel -> CMaterial` 통합 경로를 사용한다.

## 4. 원본과 파생 데이터 구분

### 정본: `SourceRaw`

- Heightmap 42개와 Weightmap 42개의 모든 mip `.bgra8`
- 확인용 mip0 PNG와 각 Texture2D 메타데이터
- 42개 Component 문서
- Landscape 재질 인스턴스 78개와 부모/파라미터/텍스처 참조
- 78개 재질의 전체 export serial, 전체 tagged property, static-permutation tail 36개
- Proxy 2개, Component 42개, CollisionComponent 42개, 로컬 Texture2D 85개
  전체 export serial
- 마스터 재질 의존성: DDS 13개, TGA 13개, MAT 4개, 속성 TXT 4개
- 마스터가 참조하는 Texture2D 13개의 원본 PF_DXT1/DXT5/BC5 mip serial
  (총 133개 mip header와 bulk payload)
- 원본 UPK 경로와 SHA-256

### 표시용 파생물: `SourceDerived`, `Resources`

- glTF 42개와 bin 42개
- baked diffuse 42개, baked normal 42개, hole mask 42개
- `.wmodel` 42개와 런타임 texture pack

현재 엔진에는 UE3 Landscape 다층 재질 그래프가 없으므로 baked 재질은 원본과
구조적으로 동일한 셰이더가 아니다. 어떤 레이어와 원본 값이 사용됐는지는
`SourceRaw`가 정본이고, baked 텍스처는 지금 화면에서 지형을 확인하기 위한
결정적 파생물이다.

## 5. 검증 결과

| 항목 | 결과 |
|---|---:|
| Landscape 컴포넌트 | 42 |
| Heightmap / Weightmap | 42 / 42 |
| CollisionComponent / 로컬 Texture2D serial | 42 / 85 |
| 로컬 Landscape 재질 인스턴스 | 78 |
| 원본 static-permutation tail | 36 |
| 마스터 Texture2D 전체 mip serial | 13개 / mip 133개 |
| Weightmap 레이어 할당 | 139 |
| 생성 `.wmodel` | 42 |
| 정점 / 삼각형 | 166,698 / 322,896 |
| 인접 컴포넌트 경계 | 70 |
| 경계 높이 샘플 | 4,410 |
| 경계 높이 불일치 | 0 |
| 경계 packed normal 불일치 | 0 |
| subsection 사용 채널 불일치 | 0 |
| 원본 빈 grid cell | 7개 유지 |
| `__DataLayer__` 컴포넌트 | 4개 유지 |

`WeightmapLayerAllocations`가 사용하지 않는 채널은 원본 패키지에서 임의 바이트를
포함할 수 있다. 따라서 subsection 검증은 Height RG와 실제 할당된 Weight 채널만
검사한다. 사용 채널의 불일치는 0건이다.

모든 `.wmodel`은 WINT/WMOD header, material table, 상대 texture path,
texture pack을 확인했다. 최종 pack과 Client 설치본 210개 파일의 SHA-256도
전부 일치한다.

재질 메타데이터에는 삭제되는 `.stage.*` 경로가 남지 않도록 최종 팩 상대경로만
저장한다. 최종 `SourceRaw` JSON에서 stale stage 경로는 0건이다.

## 6. MapTool에서 확인하는 방법

현재 `ACTIVE.maparea`는 다른 팀 작업을 보호하기 위해 수정하지 않았다.
베른성 바닥만 열 때 다음 파일을 바꾼다.

```text
Client/Bin/DataFiles/Map/ACTIVE.maparea
```

내용:

```text
LOSTARK_MAP_AREA_SELECTION 1 "LV_BER_BERNCASTLE_LANDSCAPE"
```

클라이언트를 다시 시작하고 F1로 MapTool을 연다. 이 영역은 42개 Landscape
placement가 이미 들어 있으므로 별도 수동 배치는 필요 없다. 확인 후에는 기존
area ID를 복원한다.

## 7. 아직 남은 한계

1. baked diffuse/normal은 UE3 원본 Landscape shader 실행 결과가 아니다.
   cliff 경사 혼합, specular, reflection과 부모 material graph는 아직 런타임
   베이크에 반영하지 않았다.
2. `__DataLayer__`의 alpha는 화면 렌더에서 discard되지만 mesh/collision topology는
   잘라내지 않았다. 정확한 이동 충돌은 별도 collision 생성 단계가 필요하다.
3. Landscape 전용 catalog는 기존 정적 베른성 catalog와 아직 합치지 않았다.
   기존 정적 에셋 약 950개와 42개를 합치면 현재 catalog 최대 512개 제한을 넘는다.
4. 본 결과는 베른성 바닥 Landscape만 대상으로 한다. 별도 StaticMesh 바닥,
   35개 DecalActor, foliage, light/shadow map과 건물·소품·물 배치는 포함하지 않는다.
5. 현재 자동 placement 생성은 Debug MapTool 경로다. Release 게임 레벨에서
   바닥을 생성하는 런타임 배선은 별도 작업이다.

## 8. 재현 명령과 상세 사용법

재현 명령, 출력 구조, 활성화 방법은
`Tools/LandscapeExtractor/README.md`를 따른다. 최종 자동 검증 보고서는 다음에 있다.

```text
_work/BERN_CASTLE_LANDSCAPE_VERIFIED_2026-08-01/Reports/extraction_report.json
```
