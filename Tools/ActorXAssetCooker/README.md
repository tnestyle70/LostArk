# ActorXAssetCooker

`ActorXAssetCooker`는 UModel에서 분리 추출한 스켈레탈 메시 `.psk`와 애니메이션 `.psa`를 다음 검증 경로로 조립하는 오프라인 도구다.

```text
PSK + PSA
  -> Blender 3.0 headless
  -> skeleton과 모든 Action을 포함한 FBX
  -> 기존 ModelAssetConverter
  -> CModel이 읽는 staging WModel
```

새 WModel 포맷이나 두 번째 런타임 로더를 만들지 않는다. 백룸 Hound와 LostArk 창술사에서 성공한 `PSK + PSA -> Blender -> FBX -> ModelAssetConverter` 경로를 범용화한 것이다.

## 파일

```text
Tools/ActorXAssetCooker/
├─ build_actorx_fbx.py       Blender 안에서 실행되는 범용 조립기
├─ Cook-ActorXWModel.ps1     Blender, Converter, info 검증을 연결하는 안전 wrapper
├─ render_action_preview.py  Action 대표 프레임 PNG/JSON 렌더러
└─ README.md
```

이 도구는 오프라인 Cook 전용이다. 런타임 C++ 파일, 프로젝트 파일, 원본 리소스를 수정하지 않는다.

## 준비물

- Blender 3.0. 백룸 성공 사례와 같은 버전을 우선 사용한다.
- `io_import_scene_unreal_psa_psk_280.py` PSK/PSA importer
- `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe`
- 같은 Skeleton에서 추출한 PSK와 PSA

도구는 다음 순서로 Blender를 찾는다.

1. `-BlenderPath`
2. `BLENDER_EXE` 환경 변수
3. `PATH`의 `blender.exe`
4. `C:\Program Files\Blender Foundation\Blender 3.0\blender.exe`
5. 설치된 다른 Blender 버전

Importer는 다음 순서로 찾는다.

1. `-AddonPath`
2. `ACTORX_BLENDER_ADDON` 환경 변수
3. `%APPDATA%\Blender Foundation\Blender\<version>\scripts\addons`

검증된 importer는 module 수준에 `pskimport`와 `psaimport` 함수를 제공해야 한다. API가 다른 최신 addon은 경로가 발견되더라도 명시적으로 거부한다.

검증 사례의 구형 importer는 PSA의 위치·회전만 Action으로 만들고
`SCALEKEYS`를 읽지 않는다. 조립기는 PSA `BONENAMES`, `ANIMINFO`,
`ANIMKEYS`, `SCALEKEYS`를 별도로 읽고, PSK Armature와 뼈 이름·순서가
완전히 같은지 확인한 뒤 누락된 Bone Scale FCurve를 직접 주입한다.

## 권장 실행 방법

PowerShell wrapper는 결과를 런타임 폴더가 아닌 별도 staging 패키지에 만든다.

```powershell
.\Tools\ActorXAssetCooker\Cook-ActorXWModel.ps1 `
  -PskPath 'C:\LostArkExtract\ITR_02326\itr_02326_sk.psk' `
  -PsaPath 'C:\LostArkExtract\ITR_02326\itr_02326_ani.psa' `
  -StagingDirectory 'C:\LostArkExtract\ITR_02326_Cook' `
  -AssetName 'DEPLOY_ITR_02326_INTACT' `
  -TextureRoot 'C:\LostArkExtract\ITR_02326\Textures' `
  -RequiredActionName off,on,spawn,hit1_1 `
  -ExpectedAnimationCount 4 `
  -KeepBlend
```

원본 Material Instance의 `texture_diffuse`, `texture_normal`,
`texture_emissive` 근거가 있으면 자동 탐색에 맡기지 말고 재질별 remap을 함께 넘긴다.
같은 텍스처를 여러 재질이 공유해도 각 재질 이름을 따로 적는다.

```powershell
-MaterialRemap @(
  'itr_02326_01_mi=C:\Extract\diffuse.tga',
  'itr_02326_02_mi=C:\Extract\diffuse.tga'
) `
-NormalRemap @(
  'itr_02326_01_mi=C:\Extract\crystal_normal.tga',
  'itr_02326_02_mi=C:\Extract\forest_normal.tga'
) `
-EmissiveRemap @(
  'itr_02326_01_mi=C:\Extract\crystal_emissive.tga',
  'itr_02326_02_mi=C:\Extract\crystal_emissive.tga'
)
```

지원하는 wrapper remap은 `-MaterialRemap`, `-NormalRemap`,
`-SpecularRemap`, `-EmissiveRemap`, `-OpacityRemap`, `-OrmRemap`,
`-MetallicRemap`, `-RoughnessRemap`, `-AoRemap`이다. 오른쪽 텍스처 경로는
실행 전에 실제 파일인지 검사되고, Converter에는 절대 경로로 전달된다. 하나
이상의 명시적 remap을 사용하면 `--no-auto-textures`도 함께 전달해 다른 슬롯의
자동 오매칭을 막는다. 이때 원본 재질이 사용하는 모든 슬롯을 명시해야 한다.

## 멀티 rate·bone 순서·노드 이름 옵션

기본 동작은 바뀌지 않는다. 아래 네 옵션은 명시적으로 지정할 때만 켜지며, 지정하지 않으면
기존 자산의 Cook 결과는 그대로다. 2026-08-16 발탄 146클립 AnimSet 복원에서 추가했다.

```powershell
-BakeFrameRate 30.0 `
-AllowBoneOrderRemap `
-ArmatureExportName 'mesh' `
-MeshExportName 'mesh.001'
```

`-BakeFrameRate`는 `모든 PSA sequence가 하나의 AnimRate를 공유해야 한다`는 검사를 대신해
지정한 rate 하나로 굽는다. frame span은 그대로 보존되므로 clip별 원본 rate는 반드시
`Tools/ModelAssetConverter/retime_wmodel_from_psa.py`로 되돌린다. 이 도구는 `--psa`를 여러 번
받아 여러 PSA로 조립한 AnimSet도 처리한다. 옵션을 쓰지 않으면 multi-rate PSA는 지금처럼 실패한다.

`-AllowBoneOrderRemap`은 PSA `BONENAMES`가 PSK bone의 순열일 때만 통과시킨다. 이름 집합과
개수가 정확히 같아야 하며 누락·추가·중복이 하나라도 있으면 기존처럼 실패한다. PSA importer와
scale curve 주입이 이미 이름으로 bone을 찾으므로 순서만 다른 것은 같은 애니메이션이다.

`-ArmatureExportName`과 `-MeshExportName`은 FBX export 직전 Blender object 이름을 고정한다.
Converter가 Armature 이름을 runtime clip 접두사로 쓰고, Armature와 Mesh object가 둘 다
skeleton node가 되며, runtime `skeletonHash`가 그 node 이름까지 포함한다. 따라서 body model에
Attach할 AnimSet은 body와 같은 node 이름을 재현해야 한다. Blender는 이름이 겹치면 조용히
`.001`을 붙이므로 도구가 적용 결과를 다시 확인하고 다르면 실패한다.

Attach 대상 body model의 실제 node 이름은 추측하지 말고 body `.wmodel`의 skeleton section에서
읽어 확인한다. 발탄의 경우 `RootNode` / `mesh`(Armature) / PSK bone 84개 / `mesh.001`(Mesh)로
87개였다.

또한 scale key 수 검사는 `원본과 정확히 같은 수`가 아니라 `원본보다 적으면 실패`다. Assimp가
곡선을 재샘플해 fractional tick에 key를 몇 개 더 쓰는 경우가 있는데 이는 데이터 손실이 아니며,
bone별 min/max 극값 비교가 원본 값 보존을 계속 증명한다.

PSA가 여러 개면 배열로 전달한다.

```powershell
-PsaPath @(
  'C:\LostArkExtract\Monster\idle.psa',
  'C:\LostArkExtract\Monster\walk.psa',
  'C:\LostArkExtract\Monster\attack.psa'
)
```

성공하면 다음 패키지가 생긴다.

```text
C:\LostArkExtract\ITR_02326_Cook\DEPLOY_ITR_02326_INTACT\
├─ DEPLOY_ITR_02326_INTACT.fbx
├─ DEPLOY_ITR_02326_INTACT.actorx.json
├─ DEPLOY_ITR_02326_INTACT.blend             -KeepBlend를 사용한 경우
├─ DEPLOY_ITR_02326_INTACT.wmodel
├─ DEPLOY_ITR_02326_INTACT.wmodel.info.txt
└─ textures\...
```

Wrapper는 다음 조건을 모두 통과해야 패키지를 최종 staging 이름으로 이동한다.

- Blender 종료 코드가 0이고 FBX와 JSON report가 존재한다.
- PSK에서 Armature 한 개, Bone 한 개 이상, 가중치가 있는 Mesh가 생성된다.
- PSA `BONENAMES`와 PSK Armature의 Bone 이름·개수·순서가 정확히 같다.
- 모든 PSA sequence의 raw-frame 범위가 연속이고, `ANIMKEYS`가 모든
  frame·bone을 빠짐없이 포함한다. `SCALEKEYS` chunk가 있으면 같은 조건으로
  검증한다.
- 현재 지원하는 ActorX 규칙인 `TrackTime == RawFrameCount`와 Scale Key의
  네 번째 float `1.0`이 아니면 해석을 추측하지 않고 실패한다.
- 모든 PSA에서 Action이 한 개 이상 생성된다.
- `SCALEKEYS`가 있는 Action은 Bone마다 Scale FCurve 3개와 모든 frame의
  key를 가진다.
- `-RequiredActionName`이 모두 정확한 이름으로 존재한다.
- JSON의 Action 수와 `ModelAssetConverter info`의 animation 수가 같다.
- `skeleton=yes`다.
- section 수가 `mesh + material + skeleton + animations`보다 작지 않다.
- PSA `AnimRate`, Blender effective FPS, WModel `ticksPerSecond`가 같다.
- Action frame span과 WModel `durationTicks`가 같다.
- WModel WSKL에서 PSA Bone의 순서가 유지되고, 모든 WANM의 해당 Bone
  scale key 수와 최소·최댓값이 PSA 원본과 허용 오차 안에서 같다.
- WMA2의 모든 비어 있지 않은 텍스처 경로가 staging 패키지 내부의 실제 파일을 가리킨다.
- 명시한 재질 remap은 각 재질 슬롯에서 원본과 SHA-256까지 같다.

ITR_02326의 예상 결과는 다음과 같다.

```text
actions=4
sections=7
animations=4
skeleton=yes
```

Blender Action 이름은 `off`, `on`, `spawn`, `hit1_1`로 보존된다. 다만 현재 FBX/Assimp Converter는 WModel 안에서 다음처럼 Armature 접두어를 붙일 수 있다.

```text
itr_02326_sk.ao_hit1_1
itr_02326_sk.ao_off
itr_02326_sk.ao_on
itr_02326_sk.ao_spawn
```

따라서 런타임에서 사용할 최종 이름은 추측하지 말고 함께 생성된 `.wmodel.info.txt`의 `section type=4 ... name=` 값을 정본으로 삼는다. Wrapper는 이 이름도 `WModelAnimations` 결과로 반환한다.

## 덮어쓰기와 복구

- 저장소 전체와 `Client/Bin/Resources` 아래를 staging 경로로 지정하면 항상 중단한다.
- staging 경로 또는 기존 패키지의 상위 경로에 junction/symbolic link가 있으면 중단한다.
- 같은 이름의 staging 패키지가 있으면 기본적으로 중단한다.
- 정말 다시 Cook해야 할 때만 `-OverwriteStagingOutput`을 명시한다.
- 덮어쓸 기존 폴더 안에 같은 Asset 이름의 유효한 ActorX JSON marker가 없으면 중단한다.
- 긴 Cook 도중 같은 이름의 폴더가 새로 생겨도 최종 교체 직전에 overwrite 옵션과 marker를 다시 검증한다.
- 이 스위치를 사용해도 기존 패키지를 삭제하지 않는다. 검증된 새 패키지가 준비된 뒤 기존 패키지를 `.actorx-backup-날짜` 이름으로 옮기고 교체한다.
- Blender나 Converter가 실패하면 불완전한 작업 폴더를 삭제하지 않고 `.actorx-work-*` 이름으로 남긴다.

검증된 WModel도 자동으로 런타임 리소스를 덮어쓰지 않는다. JSON report, `info` 결과, 애니메이션 시각 판정, Bounds와 Material을 확인한 다음 팀 배포 절차로 별도 반영한다.

## Blender 조립기만 실행

FBX까지만 확인하려면 Blender에서 Python 파일을 직접 실행할 수 있다.

```powershell
& 'C:\Program Files\Blender Foundation\Blender 3.0\blender.exe' `
  --background --factory-startup `
  --python '.\Tools\ActorXAssetCooker\build_actorx_fbx.py' `
  -- `
  --psk 'C:\LostArkExtract\ITR_02326\itr_02326_sk.psk' `
  --psa 'C:\LostArkExtract\ITR_02326\itr_02326_ani.psa' `
  --output-fbx 'C:\LostArkExtract\ITR_02326_Cook\preview.fbx' `
  --report 'C:\LostArkExtract\ITR_02326_Cook\preview.actorx.json' `
  --addon 'C:\Users\USER\AppData\Roaming\Blender Foundation\Blender\3.0\scripts\addons\io_import_scene_unreal_psa_psk_280.py' `
  --output-blend 'C:\LostArkExtract\ITR_02326_Cook\preview.blend'
```

직접 실행도 기존 FBX, report, blend를 덮어쓰지 않는다. staging 결과를 다시 만들 때만 `--overwrite`를 명시한다.

여러 PSA에 같은 clip 이름이 있을 때만 `--prefix-actions-with-source` 또는 wrapper의 `-PrefixActionsWithSource`를 사용한다. ITR_02326처럼 원래 이름 `off`, `on`, `spawn`, `hit1_1`을 저장해야 한다면 prefix를 사용하지 않는다.

## Action 미리보기 렌더

`render_action_preview.py`는 Cook 때 저장한 `.blend`에서 Action 하나와 비교할
프레임을 선택해, 동일한 고정 카메라로 PNG와 JSON 증거를 만든다.

```powershell
& 'C:\Program Files\Blender Foundation\Blender 3.0\blender.exe' `
  --background --factory-startup `
  --python '.\Tools\ActorXAssetCooker\render_action_preview.py' `
  -- `
  --blend 'C:\LostArkExtract\Cook\Asset\Asset.blend' `
  --action-name off `
  --frames 0 55 57 61 `
  --output-dir 'C:\LostArkExtract\Cook\Asset\Preview\off'
```

정확한 Action 이름을 모르면 `--action-suffix`를 사용할 수 있지만, 반드시 하나의
Action으로만 해석되어야 한다. 기존 PNG/report를 교체할 때만 `--overwrite`를
명시한다. Action의 모든 PoseBone이 선택된 Armature에 존재하고 Mesh가 그
Armature에 실제 parent/modifier로 연결돼 있지 않으면 미리보기 생성도 실패한다.

## 중요한 검증 규칙

1. 스켈레탈 FBX에는 `--pretransform`을 사용하지 않는다.
2. PSK와 PSA에는 동일한 `bScaleDown` 설정을 적용한다. 기본값은 검증 사례와 같은 `true`다.
3. Blender 기본 24fps를 그대로 사용하지 않는다. PSA `ANIMINFO`의 `AnimRate`로 scene FPS를 설정하며, 하나의 FBX 안에서 서로 다른 rate가 발견되면 조용히 리샘플하지 않고 실패한다. clip마다 rate가 다른 자산은 `-BakeFrameRate`로 굽는 rate를 명시하고 반드시 `retime_wmodel_from_psa.py`로 clip별 원본 rate를 되돌린다.
4. FBX exporter는 `add_leaf_bones=false`, `bake_anim_use_all_actions=true`로 고정한다.
5. Action 하나를 Armature에 실제로 할당한 뒤 export한다. 할당하지 않으면 FBX take가 빠지는 Blender/Assimp 조합이 있다.
6. clip 순서를 저장 계약으로 사용하지 않는다. 런타임 연결은 `.wmodel.info.txt`에서 검증한 최종 WModel animation 이름을 사용한다.
7. `off`, `on`, `spawn`, `hit1_1`이라는 이름만으로 파괴 의미를 단정하지 않는다. `.blend`에서 각 Action을 시각 확인하고 원본 Trigger 근거와 함께 판정한다.
8. PSK 경로와 PSA 경로는 같은 Skeleton이어야 한다. 도구가 Bone 이름·개수·순서를 대소문자까지 정확히 비교하며, 일부 Bone만 일치하는 PSA는 실패 처리한다.
9. PSA `SCALEKEYS`의 네 번째 float는 이 사례에서 전부 `1.0`이므로 frame 번호로 사용하지 않는다. frame 위치는 `FirstRawFrame + local frame` 배열 인덱스로 계산하고, scale x/y/z에는 `bScaleDown`을 적용하지 않는다.
10. Action 수나 scale key 수만 확인해서는 안 된다. FBX bake가 원본 scale을 잃어도 상수 `1.0` key를 만들 수 있으므로 WModel Bone별 최소·최댓값까지 비교한다.
11. 기존 Bind Pose WModel과 새 WModel의 Bounds, Material 슬롯, texture 경로, 게임 월드 scale을 비교하기 전 런타임 파일을 교체하지 않는다.

## 문제 해결

### importer를 찾지 못함

```powershell
-AddonPath 'C:\...\io_import_scene_unreal_psa_psk_280.py'
```

또는 환경 변수를 설정한다.

```powershell
$env:ACTORX_BLENDER_ADDON = 'C:\...\io_import_scene_unreal_psa_psk_280.py'
```

### Converter가 절대경로 파일을 읽지 못함

현재 Converter는 일부 Unicode 절대경로에서 `failed to read file`이 발생할 수 있다. Wrapper는 Converter를 staging 작업 폴더에서 실행하고 FBX/WModel에는 ASCII 상대 이름을 전달한다. staging과 원본 추출 경로도 `C:\LostArkExtract`처럼 짧은 ASCII 경로를 권장한다.

### animation 수가 Action 수보다 적음

- Armature에 Action 하나가 선택돼 있는지 report의 `selected_action`을 확인한다.
- `actions` 배열에 모든 PSA sequence가 있는지 확인한다.
- PSK/PSA Bone 이름이 실제로 일치하는지 확인한다.
- `bake_anim_use_all_actions=true`인지 report의 `settings`를 확인한다.

### 파괴 애니메이션을 정할 수 없음

`-KeepBlend`로 생성한 `.blend`를 열고 Action Editor에서 각 clip을 재생한다. 이름이나 프레임 수만으로 결정하지 않는다. 시각 판정이 끝난 뒤에만 `CDeployPropObject` 상태와 clip 이름을 연결한다.
