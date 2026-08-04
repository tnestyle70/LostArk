# 베른성 배치 오염 긴급 복구 결과

작성일: 2026-08-04
대상 Area: `LV_BER_BERNCASTLE`

## 1. 완료 상태

베른성의 후속 지형·재질 검증을 방해하던 배치 정본 문제 두 가지를 복구했다.

1. `EFMotionStaticMeshActor`를 누락한 추출기가 원점 기본 Transform을 기록한 131개 placement를 원본 UPK에서 다시 추출했다.
2. 일반 베른성에서 사용하면 안 되는 `LV_BER_BERNCASTLE_T_EVENT01`과 `LV_BER_BERNCASTLE_T_SCENE03E`를 삭제하지 않고 기본 비표시로 보존했다.
3. Imported 13개 shard, Authoring 50,017행, Client runtime shard를 공식 생성기와 Publisher로 다시 만들었다.
4. Windows staging ACL이 생성 파일로 전파되던 문제와 `Publish-MapAuthoring.ps1`의 기본 `ProjectRoot` 해석 실패를 함께 수정했다.

이번 변경은 Landscape 높이·Weightmap·재질 복원 작업이 아니다. 스크린샷의 초록색 수직 늘어짐은 별도 Landscape 파이프라인 문제로 남아 있다.

## 2. 원인과 수정

### 2-1. 131개 원점 배치

원본의 해당 `StaticMeshComponent` owner는 `EFMotionStaticMeshActor`였지만 기존 추출기는 다음 세 actor만 허용했다.

- `StaticMeshActor`
- `InterpActor`
- `StaticMeshCollectionActor`

owner를 인식하지 못한 component도 오류로 중단하지 않고 actor 기본값 `(0, 0, 0)`, identity rotation, scale 1로 계속 기록한 것이 직접 원인이었다.

`extract_ue3_placements.py`에 `EFMotionStaticMeshActor`를 정식 placement actor로 추가했다. 지원하지 않는 owner나 actor property 해석 실패는 `unresolvedPlacements`에 근거와 함께 기록하고, 한 건이라도 있으면 CLI가 실패하도록 바꿨다. 더 이상 미해석 placement를 정상 원점 배치로 위장하지 않는다.

복구 분포는 다음과 같다.

| 원본 level | 복구 수 |
|---|---:|
| `LV_BER_BERNCASTLE_T_EVENT01` | 80 |
| `LV_BER_BERNCASTLE_T_SL02` | 17 |
| `LV_BER_BERNCASTLE_T_SL04` | 2 |
| `LV_BER_BERNCASTLE_T_SL05` | 6 |
| `LV_BER_BERNCASTLE_T_SL06` | 26 |
| 합계 | 131 |

재추출 결과는 `unresolved placement 0`, `property error 0`, non-finite Transform 0이다.

### 2-2. 이벤트·컷신 배치 혼입

정확한 source level은 유지하고 기본 렌더 상태만 끄는 방식으로 처리했다.

| source level | exact | overlay | 최종 hidden |
|---|---:|---:|---:|
| `LV_BER_BERNCASTLE_T_EVENT01` | 1,127 | 6 | 1,133 |
| `LV_BER_BERNCASTLE_T_SCENE03E` | 45 | 0 | 45 |

기존에도 `SCENE03E` 한 행은 hidden이었으므로 기존 Authoring 대비 실제 visibility 변경 행은 1,177개다. 최종 기본 hidden 계약 수는 1,178개다.

### 2-3. 생성기와 Publisher 안전성

`build_bern_castle_shards.py`는 다음을 검증한 뒤에만 정본 파일을 교체한다.

- placement 50,017
- unique asset 1,003
- any-negative scale 7,990
- reflected scale 7,986
- default-hidden placement 1,178
- stable placement ID 집합 유지
- unresolved source 0

Windows에서 `tempfile.mkdtemp`로 만든 private ACL이 `os.replace`를 통해 최종 파일에 남는 문제는 출력 폴더 아래에 일반 상속 stage 디렉터리를 만들도록 수정했다. stage -> validate -> commit과 rollback 구조는 유지된다.

`Publish-MapAuthoring.ps1`은 parameter default 평가 시 비어 있는 `$PSScriptRoot`를 사용하지 않도록, param block 이후에 기본 `ProjectRoot`를 계산하게 수정했다. 이제 문서의 기본 명령을 그대로 실행할 수 있다.

## 3. 데이터 반영 결과

생성 경로는 다음 정본 계약을 따랐다.

```text
원본 UPK
-> extract_ue3_placements.py
-> C:/LostArkExtract/bern/placements*
-> build_bern_castle_shards.py
-> Data/Maps/Imported/LV_BER_BERNCASTLE
-> Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements
-> Publish-MapAuthoring.ps1
-> Client/Bin/DataFiles/Map runtime
```

기존 Authoring 50,017행이 변경 전 Imported shard를 순서까지 포함해 완전히 그대로 합친 baseline임을 먼저 확인했다. 별도 MapTool 편집이 없다는 전제 검증이 통과한 뒤에만 새 Imported shard로 Authoring을 재생성했다.

변경 전후 비교 결과는 다음과 같다.

```text
old placement count:                 50,017
new placement count:                 50,017
added stable ID:                          0
removed stable ID:                        0
source/level/kind/asset contract change: 0
transform changed:                     131
visibility changed:                  1,177
both changed:                           80
unique changed row:                  1,228
```

최종 교차 검증 결과:

- Authoring 행과 새 Imported 13개 shard 결합: exact match
- Client runtime placement shard와 Imported: exact match
- Client runtime catalog shard와 Imported: SHA-256 match
- Client runtime mapset과 Imported: SHA-256 match
- `EVENT01`: 1,133 / 1,133 hidden
- `SCENE03E`: 45 / 45 hidden

## 4. 실행한 검증

### PASS

- 원본 UPK 5개 표적 재추출
- 추출기 property error 0 / unresolved placement 0
- Bern shard full regeneration gate PASS
- `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE` PASS
- Python `py_compile` PASS
- `test_extract_ue3_placements.py`
- `test_build_bern_castle_shards.py`
- `test_build_maptool_scene.py`
- 총 26개 Python test PASS
- `git diff --check` PASS

### BLOCKED / 수동 확인 필요

- 전체 `ProjectAudit`는 이번 변경과 무관한 기존 누락 파일 `Data/Effects/SourceCatalog/dimensionist_admission.json`을 읽는 단계에서 중단됐다.
- Client Bern 진입 후 실제 화면 smoke는 자동화하지 못했다. `EVENT01` 크리스마스 오브젝트와 `SCENE03E` 컷신 오브젝트가 기본 화면에서 사라졌는지, 131개 오브젝트가 원점에 몰리지 않는지는 Debug Client에서 수동 확인이 필요하다.

## 5. 남은 문제와 다음 우선순위

이번 복구 뒤에도 다음 문제는 남아 있다.

1. 42개 Landscape의 초록색 수직 늘어짐과 거대한 절벽 모양
2. Landscape height/section 좌표, XY/Z scale, component seam 검증
3. Weightmap layer와 원본 Landscape material의 올바른 결합
4. StaticMesh 재질의 Opaque/Back 일괄 fallback 정리
5. 바닥 누락 구역을 원본 source level/BSP/decal 기준으로 분류

다음 작업은 임의 벽돌을 배치하는 것이 아니라, Landscape component 1개를 골라 원본 height sample, section base, world transform, weight layer를 수치 대조하는 검증부터 시작해야 한다.
