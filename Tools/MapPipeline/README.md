# Map authoring and publish contract

맵 데이터는 정의, visual placement, gameplay placement를 분리한다.

| 데이터 | 정본 | 소비자 |
|---|---|---|
| 맵/에셋 정의 | `Data/Maps/MapCatalog.json`, runtime `.mapassets`/`.mapset` | Loader, MapTool |
| visual placement 작성본 | `Data/Maps/Authoring/<AreaId>/<AreaId>.mapplacements` | publish tool |
| visual placement runtime | `Client/Bin/DataFiles/Map/*.mapplacements`, `*.mapset` | Loader, `CMapPlacementRuntime` |
| gameplay placement | `Data/Worlds/<AreaId>/Gameplay.world.json` | world publisher, Server |

MapTool의 visual `Save`는 authoring 파일만 교체한다. 제품 runtime 파일을 직접 덮어쓰지 않는다. gameplay `Save Gameplay`은 stable `placementId`, `archetypeId`, transform을 JSON에 저장하며 현재 kind는 `playerSpawn`, `npc`, `boss`뿐이다. 수업용 Monster 계약은 없다.

## visual publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId <AreaId>
```

single catalog는 하나의 placement 문서를, shard-set은 기존 placement ID의 shard 소속을 보존하면서 신규 항목을 해당 asset을 가진 shard에 결정적으로 배치한다. 모든 shard placement와 `.mapset`은 staging 후 한 번에 승격되며 중간 실패 시 기존 파일 전부를 rollback한다. 잘못된 row, non-finite transform, ID domain 위반, 중복 ID, root 밖 경로는 commit 전에 거부한다.

## gameplay publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
```

두 제품 world를 먼저 모두 stage/validate한 뒤 Server bootstrap 세트를 원자 교체한다. `.worldbootstrap`은 생성물이므로 직접 편집하지 않는다.
