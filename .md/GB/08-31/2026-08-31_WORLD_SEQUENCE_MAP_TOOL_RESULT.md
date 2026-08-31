# World Sequence MapTool Result

## 1. 구현 결과

Development MapTool에 `World Sequence` 저작 모드를 추가했다. 정적 맵 오브젝트의 원래
placement를 기준으로 위치, 회전, 크기, 표시 상태를 keyframe으로 편집하고 실제 맵 위치에서
재생할 수 있다.

- `Sequence List`: 한 번 만들고 여러 곳에서 재사용하는 template 목록
- `Placed Instances`: template을 현재 Area의 placement에 연결한 목록
- `Map Objects`: stable placement ID와 asset ID로 검색하는 대상 목록
- 여러 부품용 `Target Track`과 slot별 placement binding
- 시작/끝 key 자동 생성, 중간 key 추가·삭제, 시간/transform/visibility 편집
- Linear/Smooth 보간, start delay, instance별 playback speed
- Play/Pause/Stop/Loop/time scrub preview
- 영어 조작명과 한국어 hover 도움말

저장 정본은 visual placement와 같은 authoring 폴더의
`<AreaId>.worldsequences.json`이다. 쿠크 Area에서 처음 Save하면 다음 파일이 생성된다.

```text
Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/
  LV_LUT_MIDNIGHTC_ED.worldsequences.json
```

## 2. 데이터·실패 안전성

- schema `lostark.world-sequences`, formatVersion `1`의 exact JSON만 받는다.
- load는 parse -> schema/reference validation -> stage -> commit이며 실패하면 현재 Area를 유지한다.
- template/instance/slot ID 중복, 잘못된 key 시간, quaternion, scale, playback speed,
  사라진 placement를 거부한다.
- 한 instance에서 같은 placement를 둘 이상의 target slot에 연결하지 못하게 한다.
- 카메라가 transform을 소유하는 `BACKGROUND` asset은 목록에서 unavailable로 표시하고,
  UI와 document validator 양쪽에서 binding을 거부한다.
- preview는 authoring record를 수정하지 않는다. 실제 runtime visibility까지 캡처해 Stop,
  tab/Area 변경, tool close와 오류 시 원래 상태로 복구한다.
- map placement와 world sequence를 함께 저장할 때 양쪽 원본을 백업하고, 저장 뒤 다시 읽어
  의도한 내용과 정확히 같은지 검증한다. 어느 한쪽이 실패하면 두 파일을 함께 rollback한다.
- 저장 도중 process가 종료되어 pending marker가 남으면 다음 Area load 전에 양쪽 백업을
  복구한다.
- 같은 Area를 여러 MapTool이 동시에 읽거나 저장하지 못하도록 Area별 exclusive lock을 사용한다.
- Reload 이후 외부에서 map/sequence 원본이 바뀌었으면 byte baseline 비교에서 Save를 거부하고
  Reload를 요구하므로 오래된 편집 창이 새 내용을 덮어쓰지 않는다.
- sequence JSON은 파일 전체를 할당하기 전에 16 MiB 한도를 검사하고, read 중 크기가 바뀐 파일도
  거부한다. 표시 이름과 category는 유효한 UTF-8만 받는다.

## 3. 사용 순서

1. Lobby에서 `Test`로 Development Map Editor Workspace에 들어간다.
2. `F1 -> Open Map Tool`에서 `KoukuSaton / MidnightC ED`를 고르고 Area 준비가 끝날 때까지
   기다린다.
3. `World Sequence` 탭을 누른다.
4. `New Sequence`를 누르고 `Display Name`, `Category`, `Duration`, `Motion`을 정한다.
5. `Map Objects`에서 움직일 오브젝트를 선택한 뒤 `Apply to Selected Object`를 누른다.
6. `START`와 `END` key의 `Position Offset`, `Rotation Offset`, `Scale Multiplier`,
   `Visible`을 편집한다. 필요한 순간으로 time slider를 옮기고
   `Add Key at Preview Time`으로 중간 key를 만든다.
7. 종이를 여러 rigid 부품으로 구성했다면 `Add Target Track`을 누르고, 각 track에서 서로 다른
   Map Object를 선택해 `Bind Selected`로 연결한다.
8. `Play`, `Pause`, `Stop / Restore`, `Loop`, `Time`으로 실제 맵에서 확인한다.
9. `Validate` 후 `Save`를 누른다. 이 Save는 map placement와 sequence 연결을 함께 검사·저장한다.
10. `Reload`로 저장 파일을 다시 읽어 template과 instance가 유지되는지 확인한다.

## 4. 자동 검증

- World Sequence source/project integration guard: `16/16 PASS`
- Kakul placement quaternion save/read exact stability: `2951/2951 PASS`
  - unit tolerance 밖 입력 `0`, G9 저장·재파싱 뒤 component exact 변화 `0`
- Debug Product build: `PASS`
  - Engine, Shared, Server, Client compile/link
  - Client `WorldSequenceDocument.cpp`, `WorldSequenceToolPanel.cpp`, MapTool integration compile
  - compiled shader closure PASS
- build receipt:
  `out/BuildPipeline/receipts/product.debug.receipt.json`
- build evidence:
  `out/BuildPipeline/runs/20260831T044649089Z-debug-product-ade56d86.json`

## 5. 아직 제품 기능이 아닌 경계

이번 완료 범위는 MapTool authoring과 Development preview다. 다음 항목은 구현했다고 처리하지
않는다.

- 레버/trigger가 sequence를 시작하는 Server command와 world state
- Shared replication과 제품 Client의 sequence playback
- 완료 시 collision/nav 길 개방 transaction
- `book01a` 한 장 메시 자체가 실제로 말렸다 펴지는 bone/morph/procedural deformation

현재 generic sequence로는 여러 rigid 종이 부품을 순서대로 회전·이동해 펼침을 저작할 수 있다.
원본처럼 한 장이 연속 곡률로 말리는 화면은 대응 segmented/rigged/morph asset이 추가로 필요하다.

## 6. 수동 확인 상태

Client/UI는 에이전트가 실행하지 않았다. 최종 위치, 속도, 종이 부품의 pivot과 원작 대비 시각 품질은
사용자가 위 순서로 직접 확인해야 한다. 이 PC는 팀 endpoint 설정상 `client`이며, Test 입장에 필요한
공유 Server `192.168.0.14:7777`은 작업 시작 시 probe에서 `not-listening`이었다.

## 7. Animated Props (Deploy ANIM) 이어붙이기 — 2026-08-31

`World Sequence` 모드 안에 `Animated Props (Deploy ANIM)` 섹션을 추가해, sequence의
animation track이 binding할 Deploy ANIM 배치를 MapTool에서 직접 만들 수 있게 했다.
이전 상태는 `CMapTool`에 메서드 8개가 선언만 되어 있고 정의가 하나도 없었으며, 어디에서도
호출되지 않아 화면에 뜨지 않았다. 같은 변경에서 `CWorldSequenceToolPanel`의 v2 시그니처가
요구하는 `CDeployPropRuntime` 인자가 MapTool 호출부 15곳에 빠져 있던 것도 함께 맞췄다.

### 7.1 새로 구현한 것

| 메서드 | 책임 |
|---|---|
| `Get_SelectedDeployAsset` | 선택 asset ID를 Deploy catalog에서 조회 |
| `Get_SelectedAnimatedProp` | 선택 `runtimePlacementId`의 runtime entry 조회 |
| `Allocate_AnimatedPropPlacementId` | editor ID domain(`<= 0x7fffffffffffffff`) 안에서 다음 ID 할당 |
| `Try_PlaceSelectedDeploy` | 뷰포트 depth pick 위치에 `PROJECT_AUTHORED` 배치 추가 |
| `Sync_AnimatedPropTransformDraft` | 선택 배치 ↔ position/rotation/scale draft 동기화 |
| `Apply_AnimatedPropTransform` | draft를 검증해 `Update_ProjectAuthoredPlacement`로 반영 |
| `Remove_SelectedAnimatedProp` | `Remove_ProjectAuthoredPlacement` + sequence 참조 검사 |
| `Render_AnimatedPropsAuthoring` | 자산 목록 / 배치 목록 / Place·Focus·Transform·Remove·Save UI |
| `Commit_DeployCatalog` | 수정 catalog를 stage → commit하고 Deploy runtime 재구성 |
| `Save_DeployPlacements` | `.deployplacements` 원자 저장 + 사후 재읽기 검증 |

연결한 곳은 다음과 같다.

- `Render_WorldSequencePanel`이 패널 위에서 `Render_AnimatedPropsAuthoring()`을 호출한다.
- `Update_WorldInteraction`의 `WORLD_SEQUENCE` 분기에서 armed 상태의 좌클릭만
  `Try_PlaceSelectedDeploy()`로 간다. 일반 map placement로 fall-through하지 않는다.
- `ConsumesWorldLeftMouse`, Esc 취소, 모드 전환, Area 전환, Level 전환이 armed 상태와
  선택/draft를 정리한다.
- `Has_UnsavedAuthoring`과 `Save_AllAuthoring`이 `m_bDeployDirty`를 포함한다.
- `Switch_EditorArea`의 sequence 문서 load를 `Stage_DeployProps` 뒤로 옮겼다. animation
  track이 Deploy placement를 참조하므로 staged Deploy runtime 없이는 reference 검증을 할 수
  없고, 실패 시 staged placement/deploy runtime을 모두 rollback한다.

### 7.2 데이터 계약

- placement 문서는 format version 2이고 마지막 열이 provenance token이다.
- 추출 원본 행은 `SOURCE_EXACT`로 남으며 UI와 catalog 양쪽에서 읽기 전용이다.
- project 행은 `deployActorId`/`propDefinitionId`/`stateOffActionId`/
  `triggerBinaryOccurrenceCount`가 모두 `0`이어야 하고, `runtimePlacementId`는 map placement와
  같은 editor ID domain 규칙(`<= 0x7fffffffffffffff`)을 따른다.
- 저장은 temp 파일 + `MoveFileExW` 원자 교체 뒤 같은 pair를 다시 읽어 행 단위로 정확히 같은지
  비교하고, 그때만 dirty를 해제한다.
- 편집/삭제는 catalog 복사본을 검증한 뒤 Deploy runtime을 통째로 다시 stage한다. 그 전에
  sequence preview와 destruction preview seam을 반납하고, 반납이 실패하면 commit을 거부한다.
- sequence animation track이 참조 중인 배치를 지우려 하면 sequence validator가 거부하고 이전
  catalog로 되돌린 뒤 dirty 상태까지 원래대로 복구한다.
- `Save All`은 Deploy 문서를 먼저 저장하고 성공한 뒤에만 map placement + world sequence 연결
  트랜잭션을 실행한다. 순서가 반대면 저장된 sequence가 없는 placement를 참조할 수 있다.

### 7.3 source / cooked 구분

`CDeployPropCatalog::Load`가 asset의 `.wmodel` 실재를 검사하므로 조리 전 원본은 catalog에
들어오지 못한다. UI는 asset hover에 조리된 Resources 상대 경로와 clip role을, 배치 hover에
실제 model에서 읽은 clip 이름과 길이를 보여 준다. 원본 glTF/PSK/PSA는 목록에 나오지 않는다.

쿠크세이튼 Area의 현재 ANIM asset 두 개는 이미 조리되어 있다.

```text
Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/
  DEPLOY_ITR_02283/DEPLOY_ITR_02283.wmodel
    bone 11 / clip go_off 2000ms, go_on 1333.333ms, off 33.333ms, on 33.333ms
  DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE/DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE.wmodel
    bone 13 / clip evt2_paperstage_open01 3066.667ms
```

종이 펼침은 정적 메시의 rigid transform이 아니라 skinned clip이므로, sequence에서도 transform
track이 아니라 animation track으로 재생한다. 길이는 `3100ms`가 아니라 `3066.667ms`다.

계획서가 블로커로 적어 둔 ActorX/Blender 애드온 경로는 실제로는 필요 없었다.
`Tools/ActorXAssetCooker/build_umodel_gltf_psa.py`가 UModel glTF + PSA에서 직접 조리했고
`.cook.json`에 source SHA-256과 clip duration이 남아 있다.

### 7.4 MapTool.cpp의 Korean UI 문자열

`Client.vcxproj`는 `WorldSequenceToolPanel.cpp`에만 `/utf-8`을 준다. `MapTool.cpp`는 BOM 없는
UTF-8이지만 컴파일러가 시스템 코드페이지(CP949)로 읽으므로, 이 파일에 한글 문자열 리터럴을
그대로 넣으면 깨진다. vcxproj를 건드리지 않기 위해 이 섹션의 hover 도움말 5개는 UTF-8 바이트를
`\xHH` escape로 직접 적고 위에 영어 뜻을 주석으로 남겼다. 실제 컴파일 결과 바이트가 의도한
한글과 같은지 probe 프로그램으로 확인했고, guard test가 escape 형태와 디코드 가능성을 계속 검사한다.

### 7.5 자동 검증

- `Tools/MapPipeline/test_world_sequence_authoring_contract.py`: `25/25 PASS`
  - v2 계약 변경에 맞춰 stale 기대값 2건 갱신
    (`availablePlacements.find(binding.placementId)` → typed `targetId` + deploy 분기,
     `Validate(catalog, placements, validation)` → `deployRuntime` 인자 포함)
  - `AnimatedPropAuthoringContractTests` 9건 신규: 선언/정의 일치, 패널에서의 호출,
    armed 클릭 경로, editor ID domain, 원자 저장 + 재읽기 검증, dirty/저장 순서,
    binding된 배치 삭제 rollback, commit 전 preview seam 반납, escape UTF-8 도움말
- `cl /Zs` front-end 컴파일: 변경된 10개 TU 전부 `EXIT=0`
  - `MapTool.cpp`, `WorldSequenceToolPanel.cpp`(`/utf-8`), `WorldSequenceDocument.cpp`,
    `DeployPropCatalog.cpp`, `DeployPropObject.cpp`, `DeployPropRuntime.cpp`,
    `MapPlacementDocument.cpp`, `MapPlacementRuntime.cpp`, `MapStaticBatchObject.cpp`,
    `Camera_Free.cpp`
- `MapTool.h` 선언 대비 정의 누락: `0`
- `git diff --check`: whitespace 오류 없음
- 인코딩: `MapTool.cpp` bare LF `0`, lone CR `0`, U+FFFD `116`(HEAD와 동일한 기존 손상분)

### 7.6 아직 하지 않은 것

- **Client x64 Debug 정본 build + link은 실행하지 않았다.** Visual Studio(`devenv`)가 같은
  solution을 연 상태였고, 빌드는 사용자가 직접 하는 것이 이 저장소의 규칙이다. 위 `/Zs`는
  front-end 전체를 통과시키지만 링크는 검증하지 않는다.
- 화면 검증은 전부 사용자 몫이다. 아래 순서를 그대로 눌러 확인한다.

  1. Lobby `Test` → Development Map Editor Workspace
  2. `F1 -> Open Map Tool` → `KoukuSaton / MidnightC ED` 선택 후 Area 준비 대기
  3. `World Sequence` 탭 → `Animated Props (Deploy ANIM)` 헤더
  4. `Catalog Assets`에서 레버 또는 종이 펼침을 고르고 `Place In Viewport` → 뷰포트 지면 클릭
  5. `Placed Animated Props`에서 방금 배치를 선택 → `Focus`로 카메라 프레이밍 확인
  6. `Position`/`Rotation (deg)`/`Uniform Scale` 편집 후 `Apply Transform`
  7. `Save Animated Props` → `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/`
     `LV_LUT_MIDNIGHTC_ED.deployplacements`가 갱신되는지 확인
  8. `Reload Animated Props`로 다시 읽어 배치가 유지되는지 확인
  9. 아래 `Sequence List`에서 animation track template을 만들고 그 배치에 binding해
     `evt2_paperstage_open01` 재생 확인
- 레버 클릭 패킷, Server 권위 `CLOSED -> OPENING -> OPEN`, 동적 collision/navigation 개방,
  Shared replication은 이번 범위가 아니다. 5절의 경계가 그대로 유효하다.

## 8. 배치한 프롭이 화면에 안 보이던 원인 — 2026-08-31

사용자가 7절 순서대로 레버를 배치했는데 월드에 아무것도 보이지 않았다. 원인은 두 개였고
둘 다 MapTool 코드가 아니라 에셋 조리 쪽이었다.

### 8.1 스케일: 프롭이 7.7mm였다

`Ensure_DeployAuthoringPrototypes`와 제품 `Loader`는 모든 Deploy 모델에
`XMMatrixScaling(0.01f, 0.01f, 0.01f)`을 건다. 기존 Deploy 자산이 전부 센티미터라서 그렇다.
그런데 이번 두 프롭은 UModel glTF 그대로 **미터**로 조리돼 있었다.

```text
레버   source glTF span 0.4200 x 0.5390 x 0.7677
       cooked wmodel    동일 (ratio 1.000)
       월드 크기 = 0.0042 x 0.0054 x 0.0077 m
종이   월드 크기 = 0.0555 x 0.0005 x 0.0455 m
```

`.md/GB/07-29/gotchas.md`의 3단 계약(`UModel glTF meters x converter 100 x Loader 0.01`)에서
converter 단계가 빠진 상태였다.

### 8.2 `--scale 100`은 스킨드 자산에 쓰면 안 된다

먼저 `Cook-KakulInteractionProps.ps1`의 ModelAssetConverter 호출에 `--scale 100`을 넣어
프로브로 조리하고 실측했다.

```text
mesh vertices        ratio 100.000
bone bind transform  ratio   1.000
anim translation키   ratio   1.000
```

메시만 커지고 스켈레톤과 PSA 이동 키는 그대로였다. 컨버터에는 스켈레톤까지 스케일하는 옵션이
없다(`--scale`, `--pretransform` 모두 아니다). 기존에 `--scale 100`을 쓰는 곳은
`Tools/BernCastlePipeline`의 **정적** 메시뿐이라 이 한계가 드러난 적이 없었다.
이 조합은 되돌렸다.

`Engine/Private/Bone.cpp:67`은 PreTransform을 루트 본에만 곱해 계층으로 전파하고,
`Mesh.cpp:34`는 `NONANIM`일 때만 정점에 굽는다. 즉 ANIM 모델에서 PreTransform은 메시/피벗/
애니메이션 이동을 통째로 균일 변환한다. 원본 자산은 내부적으로 일관됐고 100배 작았을 뿐이다.

### 8.3 채택한 수정: 스테이징 빌더가 전부 함께 스케일한다

`Tools/ActorXAssetCooker/build_umodel_gltf_psa.py`에 `--scale`을 추가했다. 새 함수
`scale_bind_geometry`가 한 인자로 다음을 모두 곱한다.

- 모든 primitive의 `POSITION` accessor 값과 그 `min`/`max`
- skin의 `inverseBindMatrices` 각 행렬의 translation 열(column-major 12~14)
- 모든 node의 `translation`
- `build_animations`가 PSA에서 만드는 translation 키(`0.01 * scale`)

`interleaved(byteStride)` accessor와 `matrix` 형식 node는 거부한다. Cook 스크립트는 컨버터가
아니라 **빌더**에 `--scale 100`을 넘긴다. 결과는 센티미터 자산이라 다른 Deploy 자산과 같은
규약을 따르고 `uniformScale`은 1 그대로다. C++ 변경도 catalog 포맷 변경도 없다.

재조리 후 실측:

```text
                     mesh span   bone bind T   anim transl   runtime(x0.01)
DEPLOY_ITR_02283      x100.000     x100.000      x100.000        0.768 m
..._PAPERSTAGE        x100.000     x100.000      x100.000        5.547 m
```

정점/본/클립 수는 그대로다(레버 2981/11/4, 종이 24/13/1).

### 8.4 cook 스크립트가 팀 PC에서 실행되지 않던 문제

재조리하려다 이 스크립트가 Windows PowerShell 5.1에서 아예 못 도는 것을 발견했다. 이 PC는
`5.1.26100.9168`이고 `pwsh` 7은 없다. 네 가지를 고쳤다.

1. BOM 없는 UTF-8이라 한글 경로 리터럴이 CP949로 깨졌다(`'_레버_ITR_02283'` ->
   `'_?덈쾭_ITR_02283'`). UTF-8 BOM을 붙였다.
2. 여러 줄 Python을 `python -c`로 넘기면 5.1의 native 인자 처리에서 깨진다. 임시 `.py`로
   써서 실행하는 `Invoke-PythonSource`를 추가했다(`sys.argv` 인덱스는 동일).
3. Python이 쓴 UTF-8 `stage.json`을 `Get-Content`가 ANSI로 읽어 `ConvertFrom-Json`이
   실패했다. `-Encoding UTF8`을 지정하고 `PYTHONIOENCODING`/콘솔 출력도 UTF-8로 고정했다.
4. `[IO.Path]::GetRelativePath`는 .NET Core 전용이라 5.1에 없다. `Get-RelativeResourcePath`로
   대체했다.

### 8.5 애니메이션 타이밍: 이 Area 자산만 30tps로 재작성

`.wmodel` 파일 자체는 정확했다(`go_off` durationTicks 2000 / ticksPerSecond 1000 = 2.0초).
문제는 `Engine/Private/Animation.cpp:51`이 `.wmodel` 경로에서 파일 값을 버리고
`COOKED_TICK_RATE = 30.f`를 하드코딩한다는 점이다. 그래서 런타임은 2000/30 = 66.667초로
재생했고 Tool의 `Duration (ms)`도 66667로 표시됐다.

기존 자산 37개 2114클립을 조사하면 이 하드코딩의 영향은 이번 프롭만이 아니다.

```text
ticksPerSecond=30    clips=1076
ticksPerSecond=24    clips=1032   <- Artist 등 캐릭터. 지금 25% 빠르게 재생 중
ticksPerSecond=1000  clips=5      <- 이번 두 프롭
ticksPerSecond=28.71 clips=1
```

`Animation.cpp`를 파일 값 존중으로 고치면 24tps 클립 1032개가 20% 느려진다. 팀 전체 모션
타이밍이 바뀌므로 이번 슬라이스에서 하지 않는다. 대신 **이 Area의 두 프롭만** 엔진이 가정하는
30tps로 재작성했다. 24tps에 맞추는 것은 답이 아니다 — 엔진이 30으로 나누므로 48틱으로 저장하면
48/30 = 1.6초가 되어 팀원 자산과 같은 방식으로 20% 빨라진다.

`Tools/ActorXAssetCooker/retime_wmodel_ticks.py`를 추가했다. WANM 섹션마다 duration,
저장된 rate, 그리고 position/rotation/scale 모든 키의 시간 필드를 같은 비율로 다시 쓴다.
섹션 크기·키 값·채널 표는 건드리지 않아 다른 reader와 바이트 호환을 유지하고, temp + `os.replace`
원자 교체 뒤 다시 읽어 벽시계 길이가 보존됐는지 검증한다. event 레코드를 가진 clip은 레이아웃을
해석하지 않으므로 fail-closed로 거부한다(이번 두 자산은 event 0개).

Cook 스크립트가 컨버터 직후 이 도구를 `--ticks-per-second 30 --expect-ticks-per-second 1000`으로
호출하고, clip 검증의 기대 rate도 1000에서 30으로 바꿨다. 결과 durationTicks는 원본 프레임 수와
정확히 일치한다.

```text
DEPLOY_ITR_02283      go_off 60틱, go_on 40틱, off/on 1틱   @30tps
..._PAPERSTAGE        evt2_paperstage_open01 92틱          @30tps
```

엔진 재생 시간: 2.000 / 1.333 / 0.033 / 0.033초, 종이 펼침 3.067초. 원본과 같다.
지오메트리는 리타이밍 전후 동일하다(mesh span, 정점/본 수 불변).

**팀원 자산에는 영향이 없다.** `Engine/`, `Shared/`, `Server/` 변경 0건이고, 이 두 `.wmodel`을
참조하는 문서는 쿠크 Area의 `deployassets` 하나뿐이며, 이 쿠커를 호출하는 다른 파이프라인도 없다.
팀원 클립이 25% 빠른 상태는 그대로 남으며 이번 변경이 그것을 만들지도 악화시키지도 않는다.

### 8.6 자동 검증

- `Tools/ActorXAssetCooker/test_build_umodel_gltf_psa.py`: `8/8 PASS`
  - 신규 2건: `--scale`이 position/inverse bind/node translation/PSA 키를 같은 배수로
    움직이는지, 0 이하 배수를 거부하는지
- `Tools/ActorXAssetCooker/test_retime_wmodel_ticks.py`: `5/5 PASS`
  - 벽시계 길이 보존과 모든 키 시간 재작성, 패키지 내 전 clip 적용, 예상과 다른 source rate
    거부, event를 가진 clip 거부, 0 이하 목표 rate 거부. 거부 경로는 파일 바이트 불변을 확인한다.
- `Tools/MapPipeline/test_world_sequence_authoring_contract.py`: `25/25 PASS`
- Cook 실행: `KAKUL_INTERACTION_PROPS_COOK_OK assets=2` (Windows PowerShell 5.1)
- 교체 전 자산은 scratchpad에 SHA-256 백업 후 교체했고, 카탈로그가 선언한 두 경로가 모두
  해석되는 것을 확인했다.

### 8.7 사용자 확인 사항

Client를 재시작해야 새 `.wmodel`이 admission된다(프로토타입은 프로세스 내 tag 캐시).
재시작 뒤 7절 순서로 배치하면 레버 약 0.77m, 종이 펼침 약 5.55m로 보여야 한다.
직전 세션의 배치는 저장되지 않았으므로(`Placements: 0`) 새로 배치하면 된다.
`New Animation Sequence`의 `Duration (ms)`는 이제 `go_off` 2000, 종이 펼침 3067로 잡히고
`Play`가 실시간 속도로 재생된다.
