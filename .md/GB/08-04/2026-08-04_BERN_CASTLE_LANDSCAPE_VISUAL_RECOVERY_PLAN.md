# 베른성 Landscape 시각 복구 계획

작성일: 2026-08-04
대상 Area: `LV_BER_BERNCASTLE`

## 1. 목표

베른성 원본 LAND01/LAND02의 Landscape 42개를 현재 `CModel -> CMaterial`
런타임에 맞게 다시 생성한다. 높이와 Transform을 임의로 보정하지 않고 원본
Heightmap, CollisionHeightData, Proxy Scale, Weightmap, 머티리얼 파라미터를 먼저
교차 검증한다.

종료 증거는 다음과 같다.

```text
LandscapeComponent 42
LandscapeHeightfieldCollisionComponent 42
Render/Collision height sample 166,698, mismatch 0
인접 경계 70, height sample 4,410, mismatch 0
원본 layercliff source 사용
__DataLayer__ hole quad topology 제거
CModel용 WModel 42
Resources/Map 설치 파일과 Cook 결과 SHA-256 일치
```

## 2. 수정 파일과 책임

### `Tools/LandscapeExtractor/extract_ue3_landscape.py`

- `LandscapeCollisionComponent`: 충돌 컴포넌트의 SectionBase, 높이, dominant layer를
  원본 export 단위로 보존한다.
- `decode_inline_bulk_array`: uncompressed inline `FUntypedBulkData`를 크기와 절대
  logical offset까지 검증한 뒤 읽는다.
- `parse_landscape_collision_components`: Proxy가 참조하는 충돌 컴포넌트만 읽고
  중복 SectionBase를 거부한다.
- `validate_collision_height_contract`: 같은 SectionBase의 render Heightmap 3,969개와
  collision height 3,969개가 완전히 같은지 검사한다.
- `build_layer_sources`: `layer01`~`layer07`뿐 아니라 원본 `layercliff` diffuse, normal,
  tiling, brightness, desaturation, color, normal intensity를 읽는다.
- `sample_component_height`, `sample_component_source_normal`: 원본 격자 데이터를
  bilinear 보간한다.
- `cliff_blend_weight`: Client up 성분 0.35 이하를 절벽, 0.75 이상을 평면으로 두고
  그 사이만 smoothstep으로 혼합한다.
- `sample_cliff_projection`: X/Z 측면을 높이 포함 world coordinate로 반복 샘플링한다.
  cooked UE3 부모 머티리얼에 완전한 expression 연결이 없으므로 이 projection과
  임계값은 결정적 표시용 근사이며 SourceRaw가 계속 정본이다.
- `quad_is_hole`, `write_component_gltf`: top-left `__DataLayer__ > 170`인 quad의 두
  render 삼각형을 제거한다.
- `build_landscape_pack`: 42개 render/collision 계약을 모두 통과한 뒤에만 glTF,
  WModel, 보고서를 stage에서 commit한다.
- `install_pack`: 현재 리소스 계약인 `Client/Bin/Resources/Map`만 사용한다.
- `main`: 저장소 내부의 짧고 ACL을 상속하는 stage를 사용하고 실패 시 해당 stage만
  정리한다.

### `Tools/LandscapeExtractor/test_extract_ue3_landscape.py`

- strict hole threshold와 top-left 소유권
- collision inline bulk의 값·offset 검증
- cliff blend endpoint와 transition 범위
- 기존 DDS DXT1/DXT5/ATI2 decode 회귀

### `Tools/LandscapeExtractor/README.md`

현재 `Resources/Map` 경로, cliff 표시용 근사, hole topology 계약과 검증 항목을
기록한다.

## 3. 데이터 불변식

- Height16은 `R * 256 + G`, local height는 `(Height16 - 32768) / 128`이다.
- 좌표는 UE `(X,Y,Z)`에서 Client `(X,Z,-Y)`로 한 번만 변환한다.
- Proxy `Location`, `DrawScale`, `DrawScale3D`, component `SectionBase`는 원본 값을
  그대로 사용한다.
- stable asset ID, placement ID와 모델 상대 경로는 바꾸지 않는다.
- Imported/Authoring/runtime placement는 시각 리소스 교체 때문에 재작성하지 않는다.
- `Resources/LostArk` 래퍼와 두 번째 Terrain runtime을 만들지 않는다.
- 로드는 원본 parse, 계약 validate, stage 생성, 최종 commit 순서를 지킨다.

## 4. 검증 순서

```text
py_compile + unit test
-> LAND01/LAND02 42개 no-cook extraction
-> render/collision 166,698 samples exact compare
-> edge/normal/hole report 검증
-> 42개 full WModel cook
-> source/install 210 files SHA-256 compare
-> Debug Server + Client Bern smoke
-> ProjectAudit -DeepAssetHash
-> git diff --check
```

ProjectAudit가 다른 담당 데이터나 locked resource payload 때문에 실패하면 원인을
분리 기록하며, 그 파일을 이번 변경에 섞거나 임의로 새 asset pack을 발급하지 않는다.

## 5. 수동 smoke 실패 후 Map Editor 격리

2026-08-05 수동 smoke에서 42개 Landscape의 초록색 수직 늘어짐이 계속 확인됐다.
원본 placement, imported shard, `.wmodel`을 삭제하거나 50,017개 정본 문서를 재작성하지
않는다. 대신 `CMapTool`의 Bern editor view에서만 catalog `groupId == "landscape"`인
placement를 기본 비표시한다.

- 기본값: Bern Landscape 숨김
- Map Tool 상단 `Show Bern Landscape` 체크로 세션 중 즉시 재표시 가능
- `record.visible`, dirty 상태, Save/Publish 입력은 변경하지 않음
- Bern 제품 Level과 `CMapPlacementRuntime` 공용 경로는 변경하지 않음
- Character Select, Valtan, Training Map 가시성은 변경하지 않음

이 격리는 잘못된 시각 리소스가 마을 확인을 막지 않도록 하는 authoring/debug 조치다.
Landscape 원상 복구 완료를 의미하지 않으며, 42개 리소스는 후속 교정을 위해 보존한다.
