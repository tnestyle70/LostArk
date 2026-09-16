# Bern Castle exact StaticMesh pipeline

베른성 레벨 배치가 참조하는 StaticMesh를 원본 UPK의 정확한
`package + object`에서 다시 추출하고, `CModel -> CMaterial` 런타임용
`.wmodel`로 조리하는 재개 가능한 파이프라인이다.

핵심 계약은 다음과 같다.

- 입력 정본은 `*.placements.json`의 `asset.objectPath`다.
- inventory는 placement schema v1/v2/v3를 읽는다. v3 `sourceVisibility`는
  scene compiler와 같은 원본 instance/archetype/CDO 검증을 통과해야 한다.
  숨겨진 placement도 asset 추출 대상에 남으며 이름으로 배치를 제거하지 않는다.
  원본 evidence chain은 hash로 고정한 placement 입력에 보존하고, inventory에는
  입력별 schema와 visible/hidden/unrecorded 수를 기록한다. v1/v2는
  `legacy-unrecorded`이며 visible로 승격하지 않는다.
- UModel은 반드시 `-obj=<exact object>`로 실행한다.
- 같은 이름이 다른 패키지에 있는 glTF는 대체품으로 인정하지 않는다.
- 머티리얼 슬롯은 glTF 재질명과 UModel `.props.txt`의
  `TextureParameterValues`를 연결한다. 파일명 접미사를 보고 추측하지 않는다.
- 일반 diffuse/normal 이름이 없는 UE3 vertex-blend material은 채널 parameter를 읽고,
  현재 단일 texture lane 런타임에서는 red -> green -> blue -> alpha 순으로 선택한다.
  일반 parameter가 있으면 언제나 channel fallback보다 우선한다.
- 정적 glTF는 `--pretransform --scale 100`으로 조리한 뒤 기존
  `cook_wmodel_geometry_contract.py`로 같은 기하인지 검사하고 WMSH를 교체한다.
  원본 glTF에 있는 tangent.w, `COLOR_0`, `TEXCOORD_1/2`를 보존한다.
- 각 단계는 임시 디렉터리에서 검증한 뒤 asset 단위로 commit한다. 중간 실패는
  이미 검증된 asset pack을 망가뜨리지 않는다.
- source receipt의 glTF·buffer·texture hash를 검사한다. runtime receipt v2의
  output hash와 source receipt·converter·geometry helper hash까지 같으면 재개한다.
  예전 legacy cook receipt는 자동 재사용하지 않는다.

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
  --package-root $Packages `
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
    geometry.inputs.json
    geometry.legacy-cook.receipt.json
    geometry.receipt.json
    runtime.receipt.json
```

`bern_castle_assets.json`은 `build_maptool_scene.py`의 asset manifest,
`bern_castle_runtime_assets.json`은 runtime manifest로 사용한다.

### 공통 geometry cook의 근거와 실패 처리

`cook_one`과 material variant cook은 `preserve_cooked_geometry`를 공유한다.
이 함수는 receipt에 고정된 원본 glTF/buffer와 조리 입력을 대조한다. variant의
material 슬롯 이름 변경은 허용하지만 정점·추가 채널 변경은 거부한다. legacy converter의
삼각형과 최종 payload가 같은 기하인지, UV·정점 색·접선 부호와 bounds가 보존됐는지
검증한 뒤 호출자가 소유한 staging 파일만 교체한다. 실패하면 기존 runtime pack을 유지한다.

필수 position/normal/UV0/tangent/index가 없거나, primitive별 optional 채널 유무가 섞이거나,
UV2만 있고 UV1이 없으면 명시적으로 실패한다. 없는 채널을 UV0 복제·흰색 정점·임의 접선으로
채우거나 legacy 형식으로 조용히 되돌아가지 않는다. geometry와 별개인 native 재질·환경·
배치별 RNM 연결은 Area material 계약에 따라 검증해야 한다.

새 source receipt는 UModel이 보고한 실제 `physicalPackagePath`를 기록한다. 이전 receipt는
`cook --package-root`로 기록된 상대 physical package를 resolve한다. 둘 다 불가능하면
원본 package 근거 부재로 실패한다. package와 converter hash는 `OBSERVED_UNBOUND`,
생성한 geometry input manifest는 `OBSERVED_GENERATED_COOK_INPUTS_CANONICAL_LF`다.
glTF→WModel 보존을 package→glTF 무손실·원본 pivot·화면 복원의 증명으로 승격하지 않는다.
`geometry.receipt.json`은 기존 helper의 `runtimeProductAdmission=false`와 미확정 경계를 유지한다.

검사 명령:

```powershell
python -m unittest discover -s Tools/BernCastlePipeline -p 'test_*.py'
python -m unittest discover -s Tools/ModelAssetConverter -p test_cook_wmodel_geometry_contract.py
```

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
  -> Client\Bin\Resources\Map\LV_BER_BERNCASTLE\<assetId>\
```

Landscape 42개는 별도로 다음 경로를 유지한다.

```text
Client\Bin\Resources\Map\LV_BER_BERNCASTLE_T\Landscape\
```
