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
`Effect/Cooked`에 변환·복사한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
    -File .\Tools\EffectResourceIntake\Cook-SelectedEffectAsset.ps1 `
    -AssetId 'Bard_Skill_01' `
    -MeshPath 'D:\EffectSource\Bard_Skill_01.gltf' `
    -TexturePath 'D:\EffectSource\flare.dds','D:\EffectSource\mask.png'
```

결과는 `Client/Bin/Resources/LostArk/Effect/Cooked/<AssetId>`에 생성된다.
같은 AssetId가 이미 있으면 중단하며, 완성된 결과로 교체할 때만 `-Force`를
추가한다.

`mesh` 변환이 성공하면 `.wmesh`와 Converter가 함께 만든 `.wmat`을 모두
`manifest.json`에 SHA-256과 함께 기록한다. 저장소 상위 경로에 한글이 있어도
Assimp의 narrow-string 절대경로가 깨지지 않도록 Converter에는 저장소 기준
상대경로를 전달한다.

현재 Converter는 `.gltf`와 `.fbx`를 입력으로 사용한다. UModel ActorX 출력인
`.psk`, `.pskx`, `.psa`는 직접 읽지 못하므로, skeletal animation이 필요하면
ActorX를 지원하는 Blender/Noesis 등의 도구로 애니메이션 포함 FBX 또는 glTF로
먼저 변환한 뒤 `mesh`, `skel`, `anim` Cook을 수행해야 한다.

## 생성 위치

`Client/Bin/Resources/LostArk/Effect` 아래에 생성한다.

- `SourceRaw`: 원본 UPK 스냅샷
- `SourceCatalog`: 패키지 Manifest, 전체 ParticleSystem 목록, 후보 CSV
- `SourceExtracted`: UModel이 실제로 내보낼 수 있는 PNG/DDS

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
