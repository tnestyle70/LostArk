# Effect Resource Intake

이 도구는 설치된 한국 Lost Ark 클라이언트에서 다음 원본 후보를
팀 프레임워크의 로컬 리소스 영역으로 안전하게 수집한다.

- 바드 `FX_PC_MBD_00` ~ `08`
- 창술사 `FX_PC_FLM_00` ~ `10`
- 공용 선택/이동 표시 `FX_BS_03`
- 공용 대시 `FX_BS_04`
- 공용 지형별 발걸음 `FX_BS_06`
- 마우스 커서 `EFUI_CURSOR`, `EFUI_CURSOREFFECT`
- 창술사 무기 Trail 원본 `LANCEMASTER_ANIMNOTIFY_TRAILS`

## 실행

저장소 루트에서 실행한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
    -File .\Tools\EffectResourceIntake\Build-EffectSourceCatalog.ps1
```

UModel 또는 게임 설치 위치가 기본 위치와 다르면 명시한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
    -File .\Tools\EffectResourceIntake\Build-EffectSourceCatalog.ps1 `
    -UModelPath 'D:\Tools\UModel\umodel_lostark_v7.exe' `
    -PackageRoot 'D:\LOSTARK\EFGame\ReleasePC\Packages'
```

## 선택 리소스 Cook

80GB 원본 전체를 등록하지 않고, 실제 사용할 메시와 텍스처만 AssetId 단위로
`Effect/Effect_Tool` 아래로 변환·복사한다. 이 폴더가 팀에 전달하는 배포 대상이다.

`-Category`로 클래스 폴더를 지정한다. ASCII 이름만 허용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
    -File .\Tools\EffectResourceIntake\Cook-SelectedEffectAsset.ps1 `
    -AssetId 'glaivier-dash-01' `
    -Category 'Glaivier' `
    -TexturePath 'D:\EffectSource\flare.dds','D:\EffectSource\mask.png'
```

결과는 `Client/Bin/Resources/LostArk/Effect/Effect_Tool/<Category>/<AssetId>`에
생성된다. `-Category`를 생략하면 `Effect_Tool/<AssetId>`에 만든다.
같은 AssetId가 이미 있으면 중단하며, 완성된 결과로 교체할 때만 `-Force`를
추가한다.

텍스처 전용 Cook은 Converter를 호출하지 않으므로 정상 동작한다. 원본과 결과의
SHA-256을 `manifest.json`에 기록한다. 저장소 상위 경로에 한글이 있어도 Assimp의
narrow-string 절대경로가 깨지지 않도록 Converter에는 저장소 기준 상대경로를
전달한다.

### Mesh Cook은 현재 사용할 수 없다

`-MeshPath`를 넘기는 경로는 지금 실패한다. 기본 `-ConverterPath`가 삭제된
`Tools/WintersAssetConverter/Bin/WintersAssetConverter.exe`를 가리킨다.

| | 구형 (이 스크립트가 기대하는 것) | 현재 팀 표준 |
|---|---|---|
| 실행 파일 | `WintersAssetConverter.exe` (없음) | `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe` |
| 호출 | `mesh <입력> -o <출력>` | `<입력> -o <출력> --scale 100` |
| 출력 | `<AssetId>.wmesh` + `.wmat` | `<AssetName>.wmodel` + 옆의 `textures/` |
| 런타임 | `CCookedModel` (레거시) | `CModel` (정본) |

`CLAUDE.md`와 `AGENTS.md`는 신규 기능에서 `CCookedModel`과 `.wmesh`를 쓰지
않도록 정하고 있다. 따라서 이 스크립트와 `Client/Private/Effect_Runtime.cpp`를
`.wmodel` 경로로 옮기는 작업이 먼저 필요하다. 그때까지는 `-TexturePath`만
사용한다.

UModel glTF는 meter 단위이므로 `.wmodel`로 옮긴 뒤에는 `--scale 100`을 함께
써야 한다. 크기 계약은 `glTF meters × 100 × Loader 0.01 = 게임 월드 meters`다.
상세는 `Tools/ModelAssetConverter/README.md`를 따른다.

UModel ActorX 출력인 `.psk`, `.pskx`, `.psa`는 어느 Converter도 직접 읽지
못한다. skeletal animation이 필요하면 ActorX를 지원하는 Blender/Noesis 등의
도구로 애니메이션 포함 FBX 또는 glTF로 먼저 변환한다.

## 생성 위치

`Client/Bin/Resources/LostArk/Effect/Effect_Tool` 아래에 생성한다.

- `SourceRaw`: 원본 UPK 스냅샷
- `SourceCatalog`: 패키지 Manifest, 전체 ParticleSystem 목록, 후보 CSV
- `SourceExtracted`: UModel이 실제로 내보낼 수 있는 PNG/DDS
- `<Category>/<AssetId>`: Cook한 배포 대상 에셋

`Effect_Tool`은 팀에 전달하는 선별 묶음이고, 같은 레벨의
`LOSTARK_EFFECT_EXPORT_2026-07-29`(약 80GB)는 로컬 검색용 보관본이다.
전자만 공유하고 후자는 공유하지 않는다.

이전 기본값으로 만들어진 `Effect/SourceRaw`가 남아 있다. 새 기본값은
`Effect_Tool` 아래이므로 그 폴더는 더 이상 스크립트가 갱신하지 않는다.
지우기 전에 Manifest 참조 여부를 확인한다.

이 경로는 저장소 `.gitignore` 정책에 따라 Git에 올라가지 않는다.
팀 공유가 필요하면 기존 리소스 공유 드라이브로 전달한다.

## 중요한 제한

현재 Lost Ark용 UModel은 `ParticleSystem` 객체 이름과 패키지 구조는
열거하지만 Lost Ark 커스텀 Cascade 그래프를 복원하지 못한다.
따라서 CSV의 이름은 검색 후보이며 다음 항목을 증명하지 않는다.

- 실제 스킬 한 번에 호출되는 ParticleSystem 순서
- 이미터, 모듈, Curve, Burst, Lifetime의 원본 값
- ParticleSystem별 Mesh/Material/Texture의 정확한 의존 관계
- 원본 셰이더 그래프

이 자료는 Cascade형 Effect Tool에서 원본 효과를 재구성하기 위한
소스 카탈로그다. `ParticleSystem 개수 = 완성 이펙트 개수`로 취급하면
안 된다.
