# Map authoring and publish contract

맵 데이터는 정의, visual placement, gameplay placement를 분리한다.

| 데이터 | 정본 | 소비자 |
|---|---|---|
| Area 목록과 source/runtime 경로 | `Data/Maps/MapCatalog.json` | audit, toolchain |
| 추출 맵/에셋 정의와 shard 기준 | `Data/Maps/Imported/<AreaId>/` | publish tool |
| visual placement 작성본 | `Data/Maps/Authoring/<AreaId>/<AreaId>.mapplacements` | publish tool |
| visual map runtime | `Client/Bin/DataFiles/Map/*` | Loader, MapTool, `CMapPlacementRuntime` |
| gameplay placement | `Data/Worlds/<AreaId>/Gameplay.world.json` | world publisher, Server |

`Data`가 유일한 편집·추출 정본이다. `Imported`는 재추출 결과와 shard 분배 기준,
`Authoring`은 MapTool이 수정하는 현재 placement를 소유한다. `Client/Bin/DataFiles`는
publisher가 원자적으로 교체하는 실행 산출물이며 직접 편집하지 않는다. gameplay
`Save Gameplay`은 stable `placementId`, `archetypeId`, transform을 JSON에 저장하며 현재
kind는 `playerSpawn`, `npc`, `boss`뿐이다. 수업용 Monster 계약은 없다.

## visual publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId <AreaId> -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId <AreaId> -Mode Check
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId <AreaId> -Mode Publish
```

| Mode | 동작 | runtime 변경 |
|---|---|---|
| `Validate` | 입력 검증 후 expected output bytes를 메모리에 구성 | 없음. runtime 폴더도 생성하지 않음 |
| `Check` | 같은 expected bytes와 현재 선언된 output 전부를 비교. 누락·불일치 시 실패 | 없음 |
| `Publish` | 같은 expected bytes를 기존 file-set transaction으로 교체 | 해당 Area의 선언된 output만 교체 |

기존 호출과 호환되도록 생략 시 `Publish`다. 세 모드는 모두 UTF-8(BOM 없음), LF와 마지막
줄바꿈으로 정규화한 동일한 output을 사용한다. `Validate`/`Check`는 파일이나 폴더를 쓰지
않으며 runtime hash를 계산하지 않는다. 다른 Area의 파일과 현재 expected output에 포함되지
않은 이전 파일은 비교·삭제하지 않는다. `FailureAfterPromote`는 격리 fixture의 rollback
검증용이며 `Publish`에서만 허용한다.

single catalog는 `Imported/<AreaId>/<AreaId>.mapassets`와 authoring placement를 함께
publish한다. shard-set은 `Imported`의 `.mapset`, shard별 `.mapassets`와 baseline
`.mapplacements`를 읽어 기존 placement ID의 shard 소속을 보존하고 신규 항목을 해당
asset을 가진 shard에 결정적으로 배치한다. 모든 catalog, placement, `.mapset`, optional
deploy pair, light/effect/water output은 검증을 모두 마친 뒤 staging하고, 승격 중 실패하면
해당 transaction이 만든 파일을 제거하고 기존 파일을 rollback한다.

single과 shard 모두 같은 placement parser로 행, finite float transform, ID domain,
중복 placement/source ID, catalog asset 참조를 검사한다. catalog의 version/Area/count,
중복 asset ID와 Resources 상대 model 경로를 검사하며 절대·drive·parent 경로를 거부한다.
shard는 baseline count/asset 참조와 중복 ID, `Imported` 밖 shard 경로도 commit 전에
거부한다. model/material 전체 schema와 물리 model 파일 존재 검증을 새로 대체하지는 않는다.

격리 temp fixture에서 publisher 실행 계약을 검증한다. 실제 Desktop runtime은 변경하지 않는다.

```powershell
python -B -m unittest Tools.MapPipeline.test_map_effect_presentation_contract
```

## gameplay publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
```

두 제품 world를 먼저 모두 stage/validate한 뒤 Server bootstrap 세트를 원자 교체한다. `.worldbootstrap`은 생성물이므로 직접 편집하지 않는다.
