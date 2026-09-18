# Map Tool 컷신 비교 버튼 · 통합 컷신 뷰 Area 세션 교정 결과

작성일: 2026-09-17

두 가지를 같은 변경 단위로 처리했다. 서로 다른 작업이므로 아래에서 분리해 적는다.

- (1) 발탄 컷신 원본/현재 비교 재생 버튼 추가 — 사용자 요청
- (2) 통합 컷신 편집 뷰가 발탄에서 쿠크 세션을 붙이던 버그 수정

빌드는 하지 않았다. 작업 시점에 Visual Studio(`devenv.exe`)가 실행 중이어서 같은 작업 폴더에
자동화 빌드를 겹쳐 돌리지 않는다는 규칙을 따랐다. **컴파일 확인은 사용자가 VS에서 수행해야 한다.**
화면 판정도 사용자가 한다.

## 1. 컷신 비교 재생 버튼

### 목적

발탄 최후를 원본으로 교체할지 결정하기 전에, 원본 매티니의 컷과 지금 프로젝트가 쓰는 컷을
각각 눈으로 보고 비교하기 위한 것이다. 카메라만 움직이며 저장 데이터는 바뀌지 않는다.

### 사용 경로

`F1 → Map Tool → Area: Valtan → Camera 탭 → 컷신 비교`

- `비교 데이터 불러오기` 를 한 번 누르면 양쪽 트랙을 읽는다.
- `오리진 1` / `내거 1` — 발탄 등장(부활)
- `오리진 2` / `내거 2` — 발탄 최후
- 재생 중에는 `정지` 버튼과 `경과 / 전체` 초가 표시된다.
- 다른 버튼을 누르면 이전 재생을 정리하고 새로 시작한다.
- Camera 탭을 벗어나거나 Map Tool을 닫으면 카메라를 자동으로 돌려준다.

발탄 Area에서만 나타난다. 쿠크 등 다른 Area에서는 이 섹션 자체가 그려지지 않는다.

### 데이터

| 쪽 | 출처 | 내용 |
|---|---|---|
| 오리진 | `Data/Encounters/Valtan/Reference/ValtanSourceCameraCuts.reference.json` (신규) | 컷신 2개 / 컷 7개 / 키프레임 52개 |
| 내거 | `Data/Encounters/Valtan/ValtanCinematicCamera.json` (기존, 무변경) | 등장 cue 3개(8600+5800+4467ms), deathCue 1개(2090ms) |

참고 문서는 `Tools/ValtanPipeline/build_valtan_source_camera_reference.py`(신규)가
`out/ValtanCameraReplace20260917/ORIGINAL_CAMERAS_V2.json`에서 생성한다. `--check`로 재생성 없이
디스크 내용과 대조할 수 있다.

**이 참고 문서는 저작 참고용 읽기 전용이다.** `Data/Animation/Reference`와 같은 성격이며
Map Tool만 읽는다. `Level_ValtanArena`나 publisher는 읽지 않고, 두 번째 카메라 정본이 아니다.

원본 컷 구간(ms):

- 발탄 등장(부활) 23,386 — `c` 0~13,059 / `c1` 13,059~18,966 / `c` 18,966~23,386
- 발탄 최후 23,000 — `cam1` 0~3,650 / `cam2` 3,650~7,780 / `cam3` 7,780~15,410 / `cam6` 15,410~23,000

### 재생 방식과 한계

- 컷마다 자기 클럭과 키 목록을 갖는다. 원본의 하드컷을 그대로 보여 주기 위해 컷 경계를
  가로질러 보간하지 않는다.
- 키프레임 포즈는 원본 키 시각의 실측값이므로 **각 키에서의 구도는 정확하다.**
  키와 키 사이는 편집기의 CATMULL_ROM 샘플러가 채우며, 이는 원본의 `cim_curveautoclamped`
  곡선과 같지 않다. 즉 **구도는 정확하고 이징은 근사다.**
- 원본 쪽은 이징을 추가하지 않는다(easing LINEAR). 현재 쪽은 문서에 적힌
  `interpolation`/`easing`을 그대로 써서 지금 나가는 그대로 재생한다.
- 롤(up)은 재생하지 않는다. 현재 포맷이 키프레임에 up 필드를 허용하지 않기 때문이다.
  발탄 등장 `c`(약 10도)와 `c1`(약 20도)에만 롤이 있으므로 그 두 컷은 기울기가 빠진 채 보인다.

### 수정한 파일

| 파일 | 내용 |
|---|---|
| `Client/Public/MapTool.h` | `CUTSCENE_COMPARE_CUT` / `CUTSCENE_COMPARE_TRACK` 중첩 구조체, 메서드 5개 선언, 멤버 5개 |
| `Client/Private/MapTool_Cutscenes.cpp` | `Load_CutsceneCompareTracks` / `Start_CutsceneCompare` / `Stop_CutsceneCompare` / `Apply_CutsceneCompareTrack` 구현, `Apply_CutsceneCameraTrack` 선점 가드 |
| `Client/Private/MapTool_CameraShots.cpp` | `Render_CutsceneComparePanel` 구현과 Camera 탭 연결 |
| `Client/Private/MapTool.cpp` | `Update`에서 매 프레임 `Apply_CutsceneCompareTrack` 구동 |

새 카메라 런타임이나 샘플러는 만들지 않았다. 기존 `Sample_ShotCameraTrack`(제품 시네마틱
샘플러로 마샬링하는 경로)과 `CCamera_Free::Begin/Apply/End_PresentationOverride`를 그대로 쓴다.
카메라 소유권은 기존 `CAMERA_SHOT_PREVIEW_OWNER_ID`와 `m_bCutsceneCameraHeld`를 공유하므로
`End_CutsceneCameraTrack()` 한 곳에서 정리된다.

## 2. 통합 컷신 편집 뷰의 Area별 세션 선택

### 증상과 원인

발탄 Area에서 `통합 컷신 편집`을 켜도 발탄 컷신이 나오지 않았다.

`Client/Private/MainApp.cpp`가 Map Tool 통합 뷰에 `m_pSequenceActionWorkbench`를 한 번만
주입하는데, 이 멤버의 타입은 `unique_ptr<CKoukuSaydonActionWorkbench>`이고
`Compositions/Sequences/KoukuSaydonSequenceComposition.json`만 읽는다. 쿠크 전용 세션이라
발탄 데이터가 나올 수 없었다.

### 수정

호스팅할 세션을 **현재 편집 중인 Area에 따라 매 프레임 고른다.**

- `LV_LUT_HEARTRB_ED`(발탄) → `m_pValtanActionWorkbench`
- `LV_LUT_MIDNIGHTC_ED`(쿠크) → 기존 `m_pSequenceActionWorkbench`
- 그 외 Area 또는 세션이 없는 경우 → 빈 화면 대신 이유를 한국어로 표시하고 반환

`CValtanActionWorkbench`는 이미 `ICompositionWorkbenchSession`을 구현하므로 같은 통합 뷰에
그대로 붙는다.

곁들여 고친 것:

- `CSequencerTool::Suppress_SessionFrameThisFrame`에 넘기는 대상을 고정된 쿠크 세션이 아니라
  **실제로 그 프레임에 연 세션**(`Get_HostedCompositionSession()`)으로 바꿨다. Area를 바꿔도
  같은 프레임에 두 곳에서 그리는 일이 없다.
- 통합 뷰의 `Save Changes`는 쿠크 composition 전용 API(`Save`/`Is_Dirty`)에 묶여 있다.
  발탄이 호스팅된 프레임에는 `sequenceDirty`를 false로 두어 **엉뚱한 문서를 저장하지 않게**
  했다. 발탄의 split source 저장은 계속 Valtan 워크벤치가 소유한다.

Begin/End_WorkbenchFrame과 view request 소비는 한 프레임 안에서 선택된 세션 하나에 대해
짝이 맞으므로, Area가 바뀌어도 이전 세션은 이미 그 프레임을 닫은 상태다.

### 남은 제한

**발탄을 호스팅한 상태에서는 통합 뷰의 `Save Changes`가 Composition을 저장하지 않는다.**
Object 저장은 그대로 동작한다. 발탄 Composition 저장에 통합 뷰를 쓰려면 Valtan 워크벤치에
`Save(std::string&)` / `Is_Dirty()`에 해당하는 공개 계약을 맞추는 별도 작업이 필요하다.
이번 범위에서는 저장 owner를 바꾸지 않기 위해 의도적으로 하지 않았다.

## 검증 상태

| 항목 | 상태 |
|---|---|
| 참고 문서 생성 | 완료. `--check` 재실행으로 소스 추출본과 일치 확인 |
| 변경 파일 범위 | 확인. 내 수정은 C++ 4개 + 헤더 1개, 신규 2개. `ValtanCinematicCamera.json`·`Data/Valtan/`·`.vcxproj`·`.filters` 무변경 |
| 다른 작업 보존 | 확인. 기존 미커밋 변경 19개 그대로 |
| `git diff --check` | 통과 |
| C++ 컴파일 | **미실행.** VS 실행 중이라 빌드하지 않음. 사용자가 VS에서 Build 필요 |
| 화면 동작 | **미검증.** 사용자 확인 필요 |
| commit/push | 하지 않음 |
