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
  -AreaId <AreaId>
```

single catalog는 `Imported/<AreaId>/<AreaId>.mapassets`와 authoring placement를 함께
publish한다. shard-set은 `Imported`의 `.mapset`, shard별 `.mapassets`와 baseline
`.mapplacements`를 읽어 기존 placement ID의 shard 소속을 보존하고 신규 항목을 해당
asset을 가진 shard에 결정적으로 배치한다. 모든 catalog, placement, `.mapset`, optional
deploy pair는 staging 후 한 번에 승격되며 중간 실패 시 기존 파일 전부를 rollback한다.
잘못된 row, non-finite transform, ID domain 위반, 중복 ID, `Imported` 밖 shard 경로는
commit 전에 거부한다.

## gameplay publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
```

두 제품 world를 먼저 모두 stage/validate한 뒤 Server bootstrap 세트를 원자 교체한다. `.worldbootstrap`은 생성물이므로 직접 편집하지 않는다.
