# 콜로세움 매치 로딩 초상 구도 보정 2차 RESULT (2026-09-30)

상태는 맨 아래 "적용 상태"에 적는다. 이 문서의 화면 판정은 전부 사용자 몫이고, 여기 적은 수치는 스크린샷 픽셀 측정과 코드 계산이다.

## 1. 원본에 "이 3D 캐릭터를 어떻게 띄운다"는 데이터가 있었나

없었다. 이번에 빠뜨렸던 ActionScript(DoABC)까지 전부 읽고 나온 결론이다.

- 무비 `colosseumloadings3.gfx`의 DoABC(552,820바이트, 문자열 8,311개)를 처음으로 읽었다. 이 화면 전용 클래스는 `ColosseumLoadingS3Frame`(필드 topTitleLabel, leftLineupList, rightLineupList, progressBar, leftInfoPanel, rightInfoPanel, loadingTipMc, texture_L_0, texture_R_0, observerGroup; 메서드 `setLoadingTip`, `setProgress`, `StopProgress`), `...InfoGroup`, `...InfoPanel`, `...ObserverGroup`, `...Progress`뿐이다. 캐릭터 모델, 카메라, 포즈, 조명, 줌을 정하는 함수나 문자열이 없다. 나머지는 Scaleform CLIK/ARK 공용 라이브러리 코드다.
- `texture_L_0`(59,128), `texture_R_0`(1092,128)은 이름만 있는 빈 자리다. 심볼 107/108은 배치 하나(클래스 `ColosseumLoadingS3Texture_L_0/R_0`)뿐이고 크기 정보가 없다. 캐릭터는 엔진 네이티브 코드가 렌더 타깃으로 이 자리에 넣는다(추론).
- EFTable 796개(`C:\LostArkExtract\MaharakaFunctions20260926\db`)를 열이름(Portrait, Preview, Camera, Pose, Zoom, Fov, Stage…)과 문자열(콜로세움 로딩, 주목, 섬멸전)로 전수 검색했다. 이 로딩 아바타의 카메라나 포즈를 정하는 표와 행은 없다. 가까운 것만 적는다.
  - `EFTable_Colosseum`(30201 등): `Pose_Intro_*`, `Pose_Win_*`, `RedTeamCamera`. 아레나 안 입장 연출과 승리 포즈용이다. 로딩 화면이 아니다.
  - `EFTable_CommonAction`: `MVPPoseNormal`, `MVPPosePvP`, `PoseMotion`. MVP 결과 화면 포즈용이다(38행). 로딩 아바타와 같다는 근거는 없다.
  - `EFTable_MvpCameraOffset`: MVP 카메라 오프셋. 이 화면 것이 아니고, MVP 쪽에서도 이미 "표 값을 우리 카메라로 옮기는 규칙을 모른다"고 기록되어 있다(`MvpResultView.cpp` 주석).
  - `EFTable_GameMsg`의 `sys.colosseum.loading_character_watch` = "주목할 캐릭터" 제목 문자열뿐.
- 결론: 위치, 텍스처, 문구, 슬롯 구조는 원본 무비 데이터다. 3D 초상의 카메라 거리·높이·FOV·자세·조명은 원본 데이터로 정해진 값이 없어서 이 프로젝트가 스크린샷으로 맞출 수밖에 없다. 지난번에는 그 맞추기를 눈대중으로 했고 측정하지 않았다.

## 2. 지난 보정이 안 먹은 정확한 원인

보정은 들어간 빌드의 결과였다. 오히려 그 보정 자체가 틀렸다.

- 증거: 스크린샷 `190414`(19:04:15)의 닉네임이 `Test-42540`이고, 그 Client는 `ClientStartup.user.log`상 pid 42540, 시작 18:57:41이다. `Client/Default/x64/Debug/MainApp.obj`는 18:57:03에 컴파일됐고 MainApp.cpp는 18:54:16에 저장됐다. 따라서 그 화면은 보정 코드가 들어간 빌드다.
- 원인: 지난 보정은 카메라 높이를 "캐릭터 키 1.7m × Get_PresentationScale"로 가정했다(`1.30 × scale`, 화면 세로 1.3 × scale). 이 프로젝트의 캐릭터 몸은 그보다 훨씬 작다. `CustomizingView.cpp` 주석의 리그 눈 뼈 높이는 창술사 1.040 m, 차원술사 1.010 m, 워로드 0.917 m, 도화가 0.862 m이다(월드 값 = 이 값 × 프레젠테이션 스케일: 도화가 1.5 × 클래스 배율 0.7 = 1.05). 눈이 1 m 안팎이면 키는 1.2 m 안팎이다.
- 계산으로 확인: 보정 코드(S = L = 1.3 × 스케일)에서 정수리가 프레임 위에서 몇 % 내려오는지 계산하면 창술사 57.7 %, 워로드 67.2 %, 도화가 72.0 %다. 스크린샷 실측은 슬롯 세로의 67.3 %(리본 포함, 리본 +5 cm를 넣으면 도화가 68.3 %, 워로드 63.3 %)로 도화가·워로드 크기와 맞는다. 예측과 실측이 1 % 안팎으로 일치한다.
- 슬롯 밖으로 삐져나온 것처럼 보이는 것은 오해였다. 스크린샷은 실제 창(2560×1440 추정)의 일부를 2523×1276으로 잘라 낸 것이라(텍스트 좌표 회귀: `y_screenshot ≈ 1.332·y_1080 − 18`), 내 초록 사각형이 어긋났다. 캐릭터는 렌더 타깃(746×648) 안에 있고 바닥이 렌더 타깃 아래로 잘린 것이다.
- 사이즈와 별개로 다음도 확인했다.
  - FOV는 세로(`XMMatrixPerspectiveFovLH`의 FovY), 카메라는 캐릭터 LOOK 방향 앞 `fDistance`, `fEyeHeight`/`fLookHeight`는 캐릭터 발 위치 기준 세계 미터다. 프레젠테이션 스케일은 모델 루트 행렬에만 곱해지고 카메라 값에는 곱해지지 않는다.
  - 렌더 타깃은 슬롯 사각형 전체에 늘려 그려진다(`UI_Sprite`가 텍스처 전체를 슬롯 rect에 매핑). UV 부분 사용이 없다.
- 로그가 `EffectFailure.user.log`에 없던 이유: 그 로그 줄은 `OutputDebugStringA`로만 나가는 코드였다. 파일 로그가 아니라 디버거 출력이라 처음부터 안 찍히게 되어 있었다. 이번 패치에서 `Write_EffectFailureDiagnostic`으로도 남긴다(채널 `Colosseum.MatchLoading`).

## 3. 이번 카메라 산출 근거

원본 스크린샷 `171625`(1710×957)에서 좌측 초상 슬롯 사각형(=layout의 `MatchLoading_TeamA_Portrait`, 499×433 px)을 잡고 잰 값이다.

| 항목 | 실측 |
|---|---|
| 머리카락 꼭대기 | 슬롯 위에서 10.5 % |
| 눈 | 25.4 % |
| 턱 | 약 34 % |
| 머리 전체(머리카락~턱) | 약 24 % |
| 벨트 | 약 75 % |
| 얼굴 중심 가로 | 72 % |
| 몸 | 슬롯 아래에서 허리쯤 잘림, 시선은 정면에서 조금 튼 3/4 |

수식(E = 캐릭터의 월드 눈 높이, S = 프레임이 보여주는 세로 미터, L = 카메라 시선 높이, FOV 30°):

- 머리카락 꼭대기~눈 = 15 %S, 이 모델에서 크라운은 눈 + 0.16 m(`WorldPlayerNameplateView`의 `EYE_TO_CROWN`) → S = 0.16 / 0.15 ≈ 1.07 m. 머리 전체 24 %S = 0.26 m → S ≈ 1.08 m. E = 1.04 m 기준으로 S ≈ 1.03 E.
- 눈이 위에서 25.4 %이므로 L = E − (0.5 − 0.254) S = 0.75 E.
- 거리 = S / (2 tan 15°). 가로 이동 = −0.22 × S × (RT 폭/높이)(캐릭터가 슬롯 가로 72 %에 오도록 카메라를 왼쪽으로).
- 몸 방향: 원본은 가슴이 시청자 왼쪽으로 약 20° 돈다. 카메라는 캐릭터 LOOK을 Y축으로 −18° 돌린 위치(캐릭터의 왼쪽 앞)에 둔다. 이 부호는 `XMMatrixRotationY`와 왼손 좌표계 계산으로 정했고 화면으로 확인하지 못했다.

이 식은 `CCharacterPortraitRenderer::Try_Measure_EyeHeight`가 실제 눈 뼈(`b_fc_l_eye_ani`, `b_fc_r_eye_ani`)를 읽은 월드 눈 높이로 채운다. 클래스마다 몸이 달라도 같은 구도가 나온다. 눈 뼈가 없으면 1.04 m × 스케일로 대체한다.

## 4. 클래스별 예상 프레이밍 (계산, 화면 확인 전)

새 카메라(S = 1.03 E, L = 0.75 E, FOV 30°, RT 746×648) 기준. 원본 측정은 크라운 10.5 %(머리카락 포함), 눈 25.4 %.

| 클래스 | E(월드 눈) | S(m) | L(m) | 거리(m) | 크라운 위% | 눈 위% | 프레임 아래(m) |
|---|---|---|---|---|---|---|---|
| 창술사 | 1.040 | 1.071 | 0.780 | 2.00 | 10.8 | 25.7 | 0.244 |
| 차원술사 | 1.060 | 1.092 | 0.795 | 2.04 | 11.1 | 25.7 | 0.249 |
| 워로드 | 0.917 | 0.945 | 0.688 | 1.76 | 8.8 | 25.7 | 0.215 |
| 도화가 | 0.905 | 0.932 | 0.679 | 1.74 | 8.6 | 25.7 | 0.213 |
| 총잡이, 슬레이어 | 미측정 | 런타임에서 눈 뼈로 계산 | | | | | |

눈 위치는 구성상 모든 클래스가 같고, 크라운은 원본 10.5 %에 8.6~11.1 %로 맞는다(머리카락 볼륨은 모델마다 다르다). 총잡이와 슬레이어는 눈 뼈 실측값을 이 PC 문서에서 찾지 못해 표에 넣지 않았다.

이전 코드(S = 1.3 × 스케일)의 크라운 위치는 창술사 57.7 %, 차원술사 60.6 %, 워로드 67.2 %, 도화가 72.0 %였다.

## 5. 자세 문제

- 원인 후보(코드): 화면의 팔 든 자세는 베른 캐릭터가 그 순간 재생 중인 클래스 전투 대기 클립이다. `CHARACTER_ANIM::IDLE`은 클래스의 battle idle이고, 캐릭터 생성 화면은 별도 `CUSTOMIZING_IDLE`(`Set_Animation`이 `m_isCreationPreviewActive`일 때만 대체)로 서 있는 자세를 쓴다.
- 지난번 "캡처 프레임에는 강제할 수 없다"는 결론은 맞다. 전환 요청이 걸린 프레임의 Render에서 `Set_Animation`을 부르면 포즈는 다음 Update에 반영되는데, 그 Update 첫머리에서 `Change_Level`이 일어난다.
- 대안(적용함, 화면 미검증): 콜로세움 입장 확인창이 열리는 시점에 `Level_Bern.cpp`에서 로컬 캐릭터에 `CUSTOMIZING_IDLE` 클립을 재생하면 서버 응답을 기다리는 동안 포즈가 바뀌어 있다. `Level_Bern.cpp`의 콜로세움 확인창 여는 분기에 8줄을 추가했다. 스냅샷마다 로코모션 로직이 클립을 다시 덮어쓰는지는 코드만으로 확정하지 못했다. 안 먹으면 이 8줄만 지우면 된다.
- 원본 포즈가 이 `CUSTOMIZING_IDLE`인지는 데이터로 확인하지 못했다(원본 표에 지정 없음). 스크린샷 대조상 팔을 내린 서 있는 자세라 후보일 뿐이다.

## 6. 마스코트 아이콘

- 우리 화면의 VS 아래 빨강·파랑 아이콘은 우리 레이아웃 슬롯이 아니다. `MatchLoading_Layout.json`에 그 자리를 그리는 슬롯이 없고, `join_icon.png`(24×18 관전자 아이콘)는 회색 눈 모양이라 다르다. 모양(왼쪽 위로 뾰족한 삼각 화살, 금테)으로 보아 이 게임의 마우스 커서 이미지가 화면에 잡힌 것으로 판단한다(추론). 원본 스크린샷에도 새싹 왼쪽 위에 금색 화살 커서가 같이 찍혀 있다.
- 원본의 초록 새싹은 24×20 `joinIcon`보다 훨씬 크고 다른 그림이다. 텍스처 30장과 무비 심볼에서 찾지 못했다(`EFUI_LocalResource` 등 임포트 무비 쪽일 수 있음, 미확인). 이번에 교체하지 않았다. 제거할 것도 없다(우리 화면에서 그 자리에 그려지는 슬롯이 없다).

## 7. 적용 상태

- 적용 완료(2026-09-30, Release 빌드 종료 확인 후): `Client/Private/MainApp.cpp`(카메라 블록, 로그, `EffectFailureDiagnostic.h` include 1줄), `Client/Private/CharacterPortraitRenderer.cpp`, `Client/Public/CharacterPortraitRenderer.h`(`fLateralOffset`, `Try_Measure_EyeHeight`), `Client/Private/Level_Bern.cpp`(자세 8줄). 다른 fork가 MainApp.cpp에 넣은 호출은 최신 파일을 다시 읽고 앵커 치환했으므로 보존됐다(제거된 줄은 내 옛 카메라·로그 10줄뿐).
- 검증: 세 파일 CRLF와 LF 개수 동일, U+FFFD 0, `git diff --check` 경고 0, 실제 트리 대상 `cl /Zs` 세 파일 rc=0. 빌드와 실행은 하지 않았다.
- Local Release 묶음은 만들지 않기로 했으므로 어떤 묶음에도 이 보정은 들어 있지 않다.
- 조정 상수: `MainApp.cpp`의 `PORTRAIT_SPAN_PER_EYE`(1.03, 작을수록 크게), `PORTRAIT_LOOK_PER_EYE`(0.75, 클수록 캐릭터가 아래로), `PORTRAIT_YAW_DEGREES`(-18, 부호가 반대면 +18), `PORTRAIT_SHIFT_RIGHT`(0.22), `PORTRAIT_FOV_DEGREES`(30).
- 사용자 확인 순서: 재빌드 → 베른 → 콜로세움 NPC 확인 → 로딩 화면의 좌측 초상(프레임 크기, 위치, 자세) → 로그 `Client/Default/EffectFailure.user.log`의 `channel=Colosseum.MatchLoading eyeWorld=...`로 실제 눈 높이 확인. 화면 판정은 사용자 몫이고 PASS로 기록하지 않았다.
