# 베른3 배 탑승 카메라 원작 복원 RESULT (2026-09-26)

작성: 2026-09-26 02:25~02:35 (fork E; 00:19 세션 한도로 끊겼다가 02:25 재개, 저장소는 재개 시점에 시작 상태 그대로였음을 `git status`와 대상 파일 diff로 확인).
사용자 요청: "원작에서 배를 탈 때 카메라가 좀 더 위에서 보는 시점으로 바뀌는데, 원작 데이터에서 그 값을 찾아 적용하라."
빌드는 하지 않았다(조정자가 정본 러너로 수행). Client 실행·화면 판정은 하지 않았다(사용자 몫).

## 1. 스크린샷 관찰 (`스크린샷 2026-09-26 000304.png`)

- 베른 항구 앞 바다에서 유령선 계열 배(검은 돛, 붉은 깃발)가 수면 위에 떠 있다. 선체 아래 수면이 보이므로 침몰 수정(09-25)이 화면에 반영된 상태다.
- 카메라는 일반 캐릭터 추적 카메라(Bern 저장값: focusDistance 16 m, fovY 32.64°=수평 55°, pitch 45°, yaw 135°)라 배가 화면 폭의 절반 이상을 채우고, 배를 남서쪽 위에서 내려다본다. 넓은 바다가 아니라 배 자체가 주인공인 구도다.
- 원작 항해는 더 멀고(17 m) 더 넓은 화각(수평 60°)으로 배와 주변 바다를 함께 본다. 아래 원작 값 참조.

## 2. 원작 값과 출처 (사실)

DB: `out/Bern3Ship20260925/lpk/EFGame_Extra/ClientData/TableData/` (data2.lpk를 `Tools/LpkPipeline/unpack_lpk.py`로 복호화한 09-25 산출물 재사용, 796개 DB). 덤프 원문: `out/ShipCamera20260926/source_camera_dump.txt`.

### 2.1 배 → 카메라 프리셋 연결: `EFTable_VoyageShip.db`, 테이블 `VoyageShip`(94행, 89열)
- 카메라 열 3개: `Camera_Ocean`, `Camera_Booster`, `Camera_Anchor`.
- 이 프로젝트의 배 9종(PrimaryKey 8200~8208) 값: `Camera_Ocean=1001`, `Camera_Booster=1003`, `Camera_Anchor=1002`. 8204(유령선)만 `Camera_Anchor=1004`.
- 전체 94행에서 `Camera_Ocean`은 전부 1001, `Camera_Booster`는 전부 1003이다. 즉 원작의 모든 배는 항해 중 같은 카메라 1001을 쓴다.

### 2.2 프리셋 값: `EFTable_CameraSetting.db`, 테이블 `CameraSetting`(244행)
열: `PrimaryKey, SecondaryKey, Name, CameraMode, FOV, Pitch, Yaw, ZoomDist, RelativeX, RelativeY, RelativeZ, Isometric_InterpolationRatio, …`. 단위: ZoomDist·Relative* = cm, FOV·Pitch·Yaw = 도.

- 1001/1 `항해 기본뷰(ZoomIn 1단계)`: FOV 60, Pitch −45, Yaw 0, ZoomDist 1700, RelativeZ −50, InterpolationRatio 2.0 ← **이번에 적용한 값**
- 1001/2 `항해 연안뷰(ZoomIn 2단계)`: FOV 60, Pitch −40, Yaw 0, ZoomDist 1300, RelativeZ −10, ratio 2.0
- 1001/3 `항해 선박뷰(ZoomIn 3단계)`: FOV 45, Pitch −25, Yaw 0, ZoomDist 600, RelativeZ 60, ratio 1.5
- 1002/1,2 `항해 정박모드 뷰(정박볼륨 외부/내부)`: FOV 60, Pitch −35, Yaw 5, ZoomDist 1220, RelativeZ 85, ratio 2.5
- 1003/1 `항해 쾌속운항 뷰(부스터)`: FOV 70, Pitch −42, Yaw 0, ZoomDist 1700, RelativeZ −50, ratio 2.0
- 1004/1,2 `… 정박모드 뷰_유령선`: FOV 65, Pitch −35, Yaw 5, ZoomDist 1320, RelativeZ 85, ratio 2.5
- 참고(같은 표의 기본 필드 카메라 101/1 `전사(평상시) 기본카메라`): FOV 50, Pitch −45, Yaw 45, ZoomDist 1600, RelativeZ −10, ratio 2.5 → 이 프로젝트 Bern 저장값(수평 55°/16 m/pitch 45/yaw 135)의 원본과 같은 계열. 원작 배 카메라는 필드 카메라보다 FOV +10°, 거리 +1 m, 초점 −40 cm(더 아래), yaw −45°다.

### 2.3 열어봤지만 이번에 쓰지 않은 것
- `EFTable_Vehicle.db`(593행): `Fov`, `RidingCamera`, `RideonKismetCamera`, `DismountKismetCamera` 열이 있으나 배 ID 8200~8208 행은 없다(배는 Vehicle이 아니라 VoyageShip 테이블). 말 탈것의 `RidingCamera`=180000(FOV 50, pitch −45, yaw 45, 1700 cm)은 이 프로젝트 말 탑승 카메라와 무관하게 참고만.
- `EFTable_CameraContentsSetting.db`(14행): 콜로세움·전장 등 콘텐츠별 카메라 가산값. 항해 행 없음.
- `EFTable_IsometricCamera.db`(141행): 클래스별 기본 카메라(101/1 등). 항해 행 없음.
- `EFTable_ZoneBase.db`: 카메라 열 없음. 항해 존 기본 카메라는 이 DB들에서 찾지 못했다(**미확인**). UE3 아카타입은 열지 않았다.

### 2.4 환산식 (2026-09-14 카메라 복원과 같은 기저)
- UE cm → m: ÷100. ZoomDist 1700 → 17 m, RelativeZ −50 → −0.5 m.
- UE (x, y, z) → 런타임 (x, z, −y): 원작 Pitch −45(내려다봄) → 런타임 pitch +45(양수가 내려다봄, `ARENA_CAMERA_PROFILE` 주석), 원작 Yaw 0 → 런타임 yaw 90 (필드 카메라의 원작 Yaw 45 → 런타임 135와 같은 규칙, 9-14 RESULT 42행).
- 원작 FOV는 수평각. F1 패널과 같은 식으로 16:9 수직각으로 바꿔 `Set_FollowPose`에 넘긴다: fovY = 2·atan(tan(60°/2)·9/16) = 36.87°.
- `Isometric_InterpolationRatio` 2.0 → `followResponse` 2.0 (기존 `Update_ShipCamera`가 쓰던 대응 그대로; 단위 동일성은 **추론**).
- RelativeZ(원작 상하축) −50 cm → 초점점 y −0.5 m (플레이어 발 기준). RelativeX/Y는 0.

## 3. 적용 (Client 표현만; Server·protocol 변경 없음)

병합 커밋 `cd58d12b` 이후의 기존 `CLevel_Bern::Update_ShipCamera`는 이미 "배를 타면 카메라 전환"을 하고 있었지만, 원작 값 중 거리 17 m와 FOV 60°만 반영하고 pitch/yaw는 지도 카메라(45/135)를 그대로 썼으며 초점 오프셋(RelativeZ)은 없었고 값이 C++ 상수에 박혀 있었다. 이번 변경:

1. `Data/Camera/Bern.camera.json`: 선택 블록 `shipCamera` 추가(기존 11개 필드는 바이트 그대로). 값: fovXDegrees 60, pitchDegrees 45, yawDegrees 90, distanceMeters 17, focusOffsetYMeters −0.5, followResponse 2, provenance 문자열(출처·환산식).
2. `Client/Public/ArenaCameraProfile.h`: `struct ARENA_SHIP_CAMERA`(기본값 = 위 값) 추가, `ARENA_CAMERA_PROFILE`에 `shipCamera`, `hasShipCamera` 멤버 추가(끝에 추가; 집합 초기화 사용처 없음 확인).
3. `Client/Private/ArenaCameraProfile.cpp`: Parse가 `shipCamera`를 선택 필드로 받음(Bern 맵에서만 허용, 7필드 정확히, 숫자 finite), Validate에 범위 검사 추가(FOV 10..150, pitch −89..89, yaw −180..180, 거리 0.1..1000 m, 초점 ±100 m, response 0..60), Serialize는 `hasShipCamera`일 때만 블록을 다시 씀(현재 `Save` 호출자는 없음 → 기존 저장 경로가 블록을 지울 위험 없음).
4. `Client/Private/Level_Bern.cpp` `Update_ShipCamera`: 상수 3개 대신 `base.shipCamera`를 읽어 pitch/yaw로 forward 벡터를 만들고 눈 = 초점(0, −0.5, 0) − forward×17. 진입 판정(`iVehicleId`의 카탈로그 `isShip`)과 하차 시 저장 프로필로 복귀(`Set_FollowPose(base…)`)는 기존 그대로.

전환·복귀: 탑승 snapshot이 오면 다음 프레임 `Update_ShipCamera`가 한 번 `Set_FollowPose`로 배 카메라를 적용하고, 카메라의 followResponse 2로 눈·초점이 지수 보간된다(`CCamera_Free::Update_FollowCamera`). 하차하면 저장된 지도 프로필(followResponse 0 = 즉시)로 되돌린다. 원작의 줌 단계(1001/2,3)·정박 카메라(1002/1004)·부스터(1003)는 **적용하지 않았다**(휠 줌 단계와 정박 볼륨이 이 프로젝트에 없음).

검산: 지도 카메라(pitch 45/yaw 135, 거리 17)는 눈 (−8.5, 12.0, 8.5)로 저장 JSON의 positionOffset(−8, 11.2, 8)과 같은 방향 → 변환 규칙이 맞다. 배 카메라(45/90)는 눈 (−12.0, 12.0, 0): 플레이어 정서쪽 12 m·높이 12 m에서 동쪽을 내려다본다. `out/ShipCamera20260926/`에 계산 스크립트 출력 없음(대화 중 numpy로 계산).

바다 우클릭 이동: `PlayerController.cpp`의 `Try_PickGroundPlane(groundY − 0.15)`은 커서 광선과 수평 평면의 교점이라 카메라 각도와 무관하다(코드로 확인, 실행 확인은 아님).

## 4. 검증

- `cl /Zs`(구문 검사만, 링크·빌드 아님, `/std:c++20 /permissive- /EHsc /utf-8`): `ArenaCameraProfile.cpp` EXITCODE 0·오류 0, `Level_Bern.cpp` EXITCODE 0·오류 0. 로그 `out/ShipCamera20260926/logs/cl_zs_*.log`.
- `Data/Camera/Bern.camera.json` JSON parse 정상(필드 11개, `shipCamera` 7필드).
- `Server\Bin\Debug\Server.exe --vehicle-riding-contract-test`(22:59 빌드, Server 변경 없음): **77 PASS / 실패 0**, rc=0 (02:26:41~02:28:09). `out/ShipCamera20260926/logs/vehicle_riding_test.txt`.
- `git diff --check` 공백 오류 0. 렌더링 보호 파일(`Renderer.*`, `ShaderFiles`, `Data/Rendering`, `DataFiles/Rendering`) `cd58d12b` 대비 diff 0.
- 인코딩·줄바꿈: 4파일 모두 UTF-8(BOM 없음) 유지. `Level_Bern.cpp`는 원래 CRLF 1436/LF 236 혼재이며 수정 구간이 LF 구간이라 LF로 넣었다(CRLF 수 1436 그대로). 백업 `out/ShipCamera20260926/backup/*.before-ship-camera`.
- 팀장 Bern 저장값(수평 55°/16 m/pitch 45/yaw 135, positionOffset, classSizeMultipliers)은 비탑승 상태에 그대로 쓰이며 JSON의 기존 필드는 바이트 변경 없음(diff는 블록 9줄 추가뿐).

## 5. 사용자 확인 순서 (빌드 뒤)

1. 조정자 빌드(Debug Product) 완료 후 Server·Client 재시작.
2. Lobby → Bern → Bern3 부두의 배 NPC에게 배 선택 → 탑승. 바다로 나가면 카메라가 지도 카메라보다 **멀고(17 m) 넓게(수평 60°)**, 플레이어 정서쪽 위에서 동쪽을 내려다보는 시점으로 부드럽게(response 2) 바뀌어야 한다. 배가 화면 절반을 채우던 구도에서 배와 주변 바다가 함께 보이는 구도로.
3. R로 하차(부두 복귀) 시 카메라가 원래 지도 카메라로 즉시 돌아와야 한다.
4. 바다에서 우클릭 이동이 되는지. 안 되면 클릭 평면 문제이며 카메라 변경과는 별개.
5. 시점이 마음에 안 들면 `Data/Camera/Bern.camera.json`의 `shipCamera` 숫자만 고치면 된다(빌드 불필요, Level 재진입 시 반영). 원작 대안값: 연안뷰 1001/2 = fovX 60, pitch 40, yaw 90, 13 m, 초점 −0.1; 선박뷰 1001/3 = fovX 45, pitch 25, yaw 90, 6 m, 초점 +0.6; 정박뷰 1002 = fovX 60, pitch 35, yaw 95, 12.2 m, 초점 +0.85.

## 6. 되돌리는 방법

- 데이터만: JSON에서 `shipCamera` 블록을 지우면 코드 기본값(같은 원작 값)이 쓰인다. 기존(지도 pitch/yaw 유지) 동작으로 돌리려면 `out/ShipCamera20260926/backup/`의 4파일을 복사해 되돌리고 빌드.

## 7. 확정하지 못한 것 / 사용자 결정

- **원작 yaw의 기준**: 원작 항해 카메라 Yaw 0이 월드 고정인지 배 진행 방향 기준인지 DB만으로는 알 수 없다(**미확인**). 이 프로젝트 팔로우 카메라는 월드 고정 yaw만 지원하므로 필드 카메라와 같은 규칙(원작 Yaw+90)으로 월드 고정 90°를 썼다. 원작이 배 방향을 따라 도는 카메라였다면 다르게 보일 수 있다.
- 줌 단계(휠)와 정박 카메라(항구 근처 1002/1004), 부스터 카메라(1003)는 미적용. 필요하면 별도 작업.
- `Isometric_InterpolationRatio`→`followResponse` 대응은 기존 코드 관행을 따랐고 단위 동일성은 추론.
- 화면 결과 전부(사용자 몫).

## 8. 바꾼 파일 (시작·끝 `git status` 차이; `out/ShipCamera20260926/git_status_{start,end}.txt`)

- `Client/Public/ArenaCameraProfile.h` (+18줄)
- `Client/Private/ArenaCameraProfile.cpp` (+51/−2)
- `Client/Private/Level_Bern.cpp` (+15/−14, `Update_ShipCamera`만)
- `Data/Camera/Bern.camera.json` (+9줄, `shipCamera` 블록)
새 파일: 이 문서. 커밋·push·빌드·게시 없음.

## 9. 최종 재확인

- 확인한 것: 원작 값의 테이블·행·열(2절)과 덤프 원문; 4파일 패치가 앵커 1회 매치로 적용되고 CRLF/BOM 보존; JSON parse; 구문 검사 2파일 오류 0; 탈것 계약 테스트 77/0; 렌더링 보호 파일·팀장 Bern 저장값 무변경; `Save` 호출자 없음; 변경 파일이 정확히 4개.
- 확인하지 못한 것: 실제 컴파일·링크(빌드는 조정자), 화면에서 카메라가 어떻게 보이는지, 원작 yaw 기준(월드/배 방향), 항해 존 기본 카메라(DB에 없음).

SHIP_CAMERA_DONE
