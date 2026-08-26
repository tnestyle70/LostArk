# Bern Castle exact StaticMesh pipeline

베른성 레벨 배치가 참조하는 StaticMesh를 원본 UPK의 정확한
`package + object`에서 다시 추출하고, `CModel -> CMaterial` 런타임용
`.wmodel`로 조리하는 재개 가능한 파이프라인이다.

핵심 계약은 다음과 같다.

- 입력 정본은 `*.placements.json`의 `asset.objectPath`다.
- UModel은 반드시 `-obj=<exact object>`로 실행한다.
- 같은 이름이 다른 패키지에 있는 glTF는 대체품으로 인정하지 않는다.
- 머티리얼 슬롯은 glTF 재질명과 UModel `.props.txt`의
  `TextureParameterValues`를 연결한다. 파일명 접미사를 보고 추측하지 않는다.
- 일반 diffuse/normal 이름이 없는 UE3 vertex-blend material은 채널 parameter를 읽고,
  현재 단일 texture lane 런타임에서는 red -> green -> blue -> alpha 순으로 선택한다.
  일반 parameter가 있으면 언제나 channel fallback보다 우선한다.
- 정적 glTF는 `--pretransform --scale 100`으로 조리한다.
- 각 단계는 임시 디렉터리에서 검증한 뒤 asset 단위로 commit한다. 중간 실패는
  이미 검증된 asset pack을 망가뜨리지 않는다.
- `source.receipt.json`, `runtime.receipt.json`의 SHA-256이 맞으면 다음 실행에서
  해당 asset을 건너뛴다.

## 전체 실행

경로에 한글이 포함되면 현재 `ModelAssetConverter`의 Assimp 입력이 실패할 수 있다.
따라서 `--output-root`는 `C:\LostArkExtract`처럼 ASCII 경로를 사용한다.

```powershell
$Python = 'C:\Users\USER\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$UModel = 'C:\path\to\umodel_lostark_v7.exe'
$Packages = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages'

& $Python Tools\BernCastlePipeline\build_bern_castle_assets.py all `
  --placements-dir C:\LostArkExtract\bern\placements `
  --placements-dir C:\LostArkExtract\bern\placements_rest `
  --output-root C:\LostArkExtract\bern_full `
  --umodel $UModel `
  --package-root $Packages `
  --converter Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  --workers 2
```

기본 검증값은 베른성 본편(`LV_BER_BERNCASTLE_T_*`) 기준이다.

```text
StaticMesh asset       950
source placement    32,324
```

`LV_BER_BERNCASTLE_FAV_*` 326건은 별도 변형 레벨이라 기본 범위에서 빠진다.
그 7개 asset은 본편 950종 안에 모두 포함된다.

visible placement가 diffuse/emissive 없는 WModel을 참조해 불투명 회색 fallback을 쓰는지
검사하려면 다음 focused audit를 실행한다.

```powershell
& $Python Tools\BernCastlePipeline\audit_bern_runtime_materials.py
```

## 단계별 실행

원본 전체 export corpus의 정확도를 먼저 감사하려면 `inventory`를 사용한다.

```powershell
& $Python Tools\BernCastlePipeline\build_bern_castle_assets.py inventory `
  --placements-dir C:\LostArkExtract\bern\placements `
  --placements-dir C:\LostArkExtract\bern\placements_rest `
  --corpus-root C:\path\to\LOSTARK_EFFECT_EXPORT_2026-07-29 `
  --output-root C:\LostArkExtract\bern_full
```

`extract`는 exact glTF, buffer, 재질 속성, 참조 DDS를 asset pack으로 만든다.
`cook`은 기존 inventory와 source pack을 읽어 WModel을 만든다.

```powershell
& $Python Tools\BernCastlePipeline\build_bern_castle_assets.py cook `
  --inventory C:\LostArkExtract\bern_full\manifests\bern_castle_assets.json `
  --output-root C:\LostArkExtract\bern_full `
  --converter Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  --workers 2
```

`--force`는 해당 단계의 검증된 asset 디렉터리만 교체한다. 출력 루트 밖의
경로는 삭제하지 않는다. 전체 프로세스 이름을 찾아 종료하지 않으며, timeout은
파이프라인이 직접 시작한 자식 프로세스에만 적용된다.

## 결과

```text
<output-root>/
  manifests/
    bern_castle_assets.json
    bern_castle_runtime_assets.json
  source/<assetId>/
    <object>.gltf
    <object>.bin
    materials/*.props.txt
    textures/*.(dds|tga|png)
    source.receipt.json
  runtime/<assetId>/
    <assetId>.wmodel
    textures/*.(dds|tga|png)
    converter.info.txt
    runtime.receipt.json
```

`bern_castle_assets.json`은 `build_maptool_scene.py`의 asset manifest,
`bern_castle_runtime_assets.json`은 runtime manifest로 사용한다.

## MapTool용 13개 shard 생성

베른성 950종은 물리 catalog 제한 512개를 넘으므로 BASE, LANDSCAPE,
SL00~SL10의 13개 문서로 나눈다. builder는 모든 child를 임시 디렉터리에서
검증한 다음 `.mapset`을 마지막 commit marker로 설치한다.

```powershell
& $Python Tools\LevelPlacementExtractor\build_bern_castle_shards.py `
  --asset-manifest C:\LostArkExtract\bern_full\manifests\bern_castle_assets.json `
  --runtime-manifest C:\LostArkExtract\bern_full\manifests\bern_castle_runtime_assets.json `
  --runtime-root C:\LostArkExtract\bern_full\runtime `
  --placements-dir C:\LostArkExtract\bern\placements `
  --placements-dir C:\LostArkExtract\bern\placements_rest `
  --landscape-catalog Client\Bin\DataFiles\Map\LV_BER_BERNCASTLE_LANDSCAPE.mapassets `
  --landscape-placements Client\Bin\DataFiles\Map\LV_BER_BERNCASTLE_LANDSCAPE.mapplacements `
  --output-dir Client\Bin\DataFiles\Map
```

정상 기본 gate는 static asset 950, static placement 32,324, Landscape 42,
통합 placement 32,366, 고유 asset 992다. 생성된 shard set은 원본 대량 배치를
보호하기 위해 MapTool에서 read-only다.

외부 runtime pack은 다음 위치로 팀 리소스 동기화한다. 이 경로는 Git 추적 대상이
아니다.

```text
C:\LostArkExtract\bern_full\runtime\<assetId>\
  -> Client\Bin\Resources\LostArk\Map\LV_BER_BERNCASTLE\<assetId>\
```

Landscape 42개는 별도로 다음 경로를 유지한다.

```text
Client\Bin\Resources\LostArk\Map\LV_BER_BERNCASTLE_T\Landscape\
```
