# Runtime resource pack operations

`Client/Bin/Resources`는 Git 작업 폴더가 아니라 외부 런타임 팩의 hydrate 대상이다. 최상위는 `Character`, `Deploy`, `Effect`, `Fonts`, `Map`, `UI` 여섯 개만 허용한다. `LostArk`, `SourceData`, raw 추출물, 실행 파일은 팩에 넣지 않는다.

정본은 다음 두 파일이다.

- `Data/AssetPacks.lock.json`: 팀이 받아야 할 pack ID와 version
- `Data/AssetManifests/<pack>-<version>.manifest.json`: 파일 목록, 크기, SHA-256

## 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 -Mode Verify
```

`Verify`는 디렉터리 구조, 파일 집합, 크기, hash를 lock과 대조한다. 통합 전에는 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1 -DeepAssetHash`도 실행한다.

## 새 버전 고정과 publish

```powershell
powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 `
  -Mode Snapshot -Version <new-version>

powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 `
  -Mode Publish -PackRoot <external-pack-root>
```

기존 manifest version은 immutable이다. 파일이 바뀌면 version을 올린다. Snapshot은 manifest 생성과 lock 교체를 한 트랜잭션으로 취급하며 lock commit 전 실패하면 새 manifest와 임시 파일을 제거한다. Publish는 외부 `.staging`에서 payload 전체를 검증한 뒤 immutable version 폴더로 승격한다.

## hydrate

```powershell
powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 `
  -Mode Hydrate -PackRoot <external-pack-root>
```

Hydrate는 새 payload를 별도 staging에 복사해 전체 검증한 뒤 `Resources`를 교체한다. 기존 폴더는 `Resources.previous.<UTC timestamp>`로 남겨 rollback 근거를 보존한다. 새 팩 검증이 끝나기 전에 이 backup을 지우지 않는다.

## 금지 사항

- `Client/Bin/Resources` payload를 Git/LFS에 stage하지 않는다.
- absolute path, drive path, `..`가 포함된 asset ID를 만들지 않는다.
- raw UModel/SourceData를 runtime pack에 넣지 않는다.
- lock만 수정하거나 manifest만 복사하지 않는다.
- 이미 publish된 version 폴더를 덮어쓰지 않는다.
