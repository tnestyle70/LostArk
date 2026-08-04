# 베른성 Landscape 시각 복구 결과

작성일: 2026-08-04
대상 Area: `LV_BER_BERNCASTLE`

## 1. 구현 결과

베른성 Landscape의 높이와 크기를 임의 수정하지 않고 원본 데이터로 교차 검증했다.
42개 컴포넌트의 render Heightmap과
`LandscapeHeightfieldCollisionComponent` 높이 166,698개가 모두 일치했다. 따라서
스크린샷의 초록색 수직면은 height endian이나 임의 Scale 오류가 아니라, 기존 표시용
베이크가 원본 `layercliff`를 누락해 평면용 grass/road 텍스처를 절벽에도 사용한 것이
직접 원인이었다.

다음 내용을 구현했다.

- 원본 collision inline BulkData 해독과 render/collision height 전수 검증
- 원본 `layercliff` diffuse, normal, tiling, color, brightness, desaturation,
  normal intensity 로드
- source packed normal과 높이 포함 world coordinate를 사용하는 절벽 측면 projection
- `__DataLayer__ > 170`인 top-left 샘플이 소유하는 463개 quad의 926개 render
  삼각형 제거
- 현재 규칙에 맞는 `Resources/Map` 출력과 짧은 ACL 상속 stage
- CModel용 `.wmodel` 42개 재생성

원본 cooked 부모 머티리얼에는 완전한 expression 연결이 남아 있지 않다. 따라서 cliff
projection과 normal-up 0.35/0.75 혼합 임계값은 원본 텍스처·파라미터·source normal을
사용하는 결정적 표시용 근사다. 원본과 동일한 material graph라고 주장하지 않는다.

## 2. 런타임 통합

새 Cook 결과를 다음 위치에 설치했다.

```text
Client/Bin/Resources/Map/LV_BER_BERNCASTLE_T/Landscape
```

기존 42개 런타임 리소스는 다음 위치에 보존했다.

```text
_work/BERN_CASTLE_LANDSCAPE_RUNTIME_BACKUP_2026-08-04
```

새 출력 210개 파일과 설치된 210개 파일은 상대 경로와 SHA-256이 모두 일치한다.
stable asset ID, placement ID, 모델 상대 경로는 그대로이므로 Imported,
Authoring 50,017개, runtime placement 문서는 재작성하지 않았다.

전체 추출 결과는 다음 위치에 있다.

```text
_work/BERN_CASTLE_LANDSCAPE_VISUAL_FIXED_2026-08-04
```

## 3. 자동 검증

```text
py_compile: PASS
LandscapeExtractor unit tests: 23 PASS
LandscapeComponent: 42
CollisionComponent: 42
Render/Collision height samples: 166,698
Render/Collision mismatch: 0
Adjacent edges: 70
Adjacent edge samples: 4,410
Height mismatch: 0
Packed normal mismatch: 0
Subsection seam mismatch: 0
Original missing grid cells preserved: 7
Hole quad count: 463
WModel count: 42
Installed resource files: 210
Installed/source hash differences: 0
Resources top-level: Character, Deploy, Effect, Fonts, Map, UI
```

## 4. 전체 Audit와 수동 검증 상태

`ProjectAudit -DeepAssetHash`는 현재 Resources의 파일 집합이 locked `.4` manifest와
이미 다르기 때문에 중단됐다. 현재 payload에는 manifest 기준 추가 1,363개와 누락
1,258개가 있으며 Dimensionist와 Character Select 등 다른 담당 리소스가 포함된다.
이번 Landscape만 검증하지 않은 다른 리소스까지 새 팩으로 묶지 않기 위해
`Data/AssetPacks.lock.json`과 manifest는 수정하지 않았다.

일반 ProjectAudit는 이 작업과 무관하게 누락된 다음 파일에서 중단됐다.

```text
Data/Effects/SourceCatalog/dimensionist_admission.json
```

Debug Server와 Client는 실행했다. 실제 Lobby에서 Bern 진입 후 화면 비교는 사용자가
클라이언트에서 확인해야 하므로 자동 PASS로 기록하지 않는다.

## 5. 남은 경계

- 화면에서 초록색 수직 늘어짐과 hole이 교정됐는지 Bern 수동 smoke를 완료한다.
- 수동 smoke 통과 후 리소스 담당자가 다른 로컬 payload를 분리하고
  `Snapshot -> Verify -> Publish -> Hydrate`로 새 immutable pack을 발급한다.
- Decal, foliage, water, 조명과 별도 StaticMesh 바닥은 Landscape 전용 복구 범위가
  아니며 각 Area layer 정본에서 별도로 복원한다.
