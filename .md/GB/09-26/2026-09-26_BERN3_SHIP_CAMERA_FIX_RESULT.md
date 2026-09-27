# 배 탑승 카메라가 멀어지지 않던 문제 수정 RESULT

> **2026-09-26 추가 조정:** 사용자가 40 m는 배가 너무 작아 보인다고 해서 `distanceMeters`를
> 40 m → 28 m → **20 m**로 다시 줄였다. 현재 저장값은 20 m이고 원작 소스값은 그대로 17 m다.
> 아래 분석의 40 m는 그 시점의 선택이며, 최신 값과 근거는
> `2026-09-26_BERN3_SHIP_SPEED_TUNING_RESULT.md`에 있다.

작성 2026-09-26 14:55. 브랜치 `codex/main-ship-maharaka-0926`(HEAD = origin/main `a84bbcd4`).
사용자 보고: "배를 타면 원작처럼 멀리서 전체가 보여야 하는데 전혀 변경되지 않았다."
Client와 게임 UI는 실행하지 않았다. 화면 판정은 사용자 몫이다.

## 1. 두 화면 관찰과 측정

**원작 `스크린샷 2026-09-26 143439.png`** (706x1150 세로 영상 크롭)
- 범선 전체(돛대 끝 y≈285 ~ 선체 바닥 y≈530)가 **화면 세로의 약 21%**. 가로는 약 51%.
- 배 전체가 프레임 안에 들어오고 위아래로 바다가 넓다. 뒤 위에서 내려다본다.
- 하단은 원작 항해 HUD("34.8 노트", 정박 안내).

**우리 `스크린샷 2026-09-26 143444.png`** (1282x713)
- 유령선(8204)이 가로 약 44%. 세로는 **돛대가 화면 상단 밖으로 잘려** 있고 선체 바닥이 y≈520.
- 즉 배가 프레임에 다 들어오지 않는다. 세로 점유 **100% 초과**.

측정 결론: 각도는 내려다보지만 **거리가 부족**하다. 원작 21% 대 우리 100%+.

## 2. 왜 안 먹었나 (원인)

### 2.1 코드 경로는 정상이다 [사실]

값이 무시·덮어쓰기·클램프되는 지점을 전부 따라갔고, **결함은 없었다.**

- `Client/Private/Level_Bern.cpp:602` — `Update_ShipCamera()`가 매 프레임 호출된다.
- `Level_Bern.cpp:1435~1437` — `player.iVehicleId`로 `CActorCatalog::Find_Vehicle`을 찾아 `isShip`을 본다.
  `Client/Private/ActorCatalog.cpp:1171,1188~1192`가 JSON `"ship"` 키를 `entry.isShip`으로 읽고,
  `Data/Actors/VehicleCatalog.json`의 8204는 `"ship": true`다. 판정은 성립한다.
- `Client/Private/ArenaCameraProfile.cpp:131~151` — `shipCamera` 블록은 **정확히 7키**를 요구하고
  우리 JSON도 7키(provenance 포함)다. 조용히 무시되지 않는다.
  `:378~383`의 범위 검증도 distanceMeters 0.1~1000 m를 허용하므로 17도 40도 통과한다.
- `Client/Private/Camera_Free.cpp:198~244` `Set_FollowPose` — 유한성과 최소 길이만 보고
  `m_vPositionOffset`, `m_fBaseFovy`, `m_fFovy`에 그대로 넣는다. **거리 클램프가 없다.**
- `Camera_Free.cpp:322` `Update_VehicleOrbitOffsets`는 `ANCIENT_SEA_VEHICLE_ID`(고대의 바다)에서만
  오프셋을 덮어쓴다(`:400`). 배 8200~8208은 해당 없다.
- Bern `Update()` 구간(520~615)에서 `Set_FollowPose`를 부르는 곳은 `Update_ShipCamera` 하나뿐이다.
  매 프레임 도는 `Bind_CameraToLocalCharacter()`(`:919~961`)는 FollowTarget만 바꾸고 pose는 안 건드린다.
- 카메라 far plane은 최소 2000 m(`Level_Bern.cpp:846~848`)라 거리를 늘려도 걸리지 않는다.

### 2.2 진짜 원인: 원작 값이 우리 지도 카메라와 거의 같다 [사실]

| | 거리 | 수직 FOV | 대상 지점의 세로 시야 |
|---|---|---|---|
| 베른 지도 카메라(팀장 튜닝) | 16 m | 32.64° (수평 55°) | **9.37 m** |
| 배 카메라(원작 1001/1 그대로) | 17 m | 35.98° (수평 60°) | **11.04 m** |

**배 카메라는 지도 카메라보다 세로 시야가 18% 넓을 뿐이다.** 사람 눈에 "안 바뀌었다"로 보이는 게 정상이다.

여기에 우리 모델 크기가 겹친다. 유령선 8204 실측(WModel 정점 × preScale 0.01):
**9.4 m × 7.3 m × 3.1 m**로 9척 중 가장 크다. 피치 45°에서 화면 세로로 투영되는 최악값은
`sin45° × (길이 + 높이) = 0.7071 × 16.7 = 11.81 m`. 시야 11.04 m보다 커서 **107%**,
즉 돛대가 화면 밖으로 나간다. 스크린샷과 정확히 일치한다.

9척 실측(길이 × 높이 × 폭, m): 8200 4.9×3.8×2.0 / 8201 4.7×3.5×1.7 / 8202 6.9×4.6×2.6 /
8203 4.4×2.1×1.8 / **8204 9.4×7.3×3.1** / 8205 5.7×4.1×2.4 / 8206 4.5×3.7×1.7 /
8207 6.9×4.4×2.6 / 8208 5.1×3.9×3.1.

### 2.3 원작 값 자체는 E fork가 옳게 읽었다 [사실, 원본 재확인]

`C:\LostArkExtract\MaharakaFunctions20260926\db\...\EFTable_CameraSetting.db`를 직접 열어 확인했다.

- `VoyageShip` 94행 전부 `Camera_Ocean=1001`, `Camera_Booster=1003`, `Camera_Anchor=1002`(유령선만 1004).
- `CameraSetting` 1001/1 「항해 기본뷰(ZoomIn 1단계)」: FOV 60, Pitch −45, ZoomDist 1700, RelativeZ −50, ratio 2.0.
- 참고 1001/2 연안뷰 FOV 60·Pitch −40·1300, 1001/3 선박뷰 FOV 45·Pitch −25·600,
  1002 정박 FOV 60·Pitch −35·Yaw 5·1220, 1004 유령선 정박 FOV 65·1320, 1003 부스터 FOV 70·1700.
- `VoyageShip` 89개 열에 **배 크기·카메라 배율 열은 없다**(카메라 관련은 위 3개뿐).
- 원작 필드 카메라 101/1은 FOV 50·ZoomDist 1600이고, 우리 지도 카메라(16 m)와 정확히 대응한다.
  즉 cm→m·각도 변환 규칙 자체는 검증된다. 원작 내부 비율은 필드 대비 항해가 세로 시야 1.316배다.

### 2.4 확정하지 못한 것 [미확인]

원작 화면에서 배가 21%인데, 원작 값(17 m/FOV 60)에 우리 모델을 넣으면 107%가 된다.
같은 카메라 값에서 이 차이가 나려면 **원작 항해 맵의 배가 우리 모델보다 훨씬 작게 그려져야** 한다.
원작 항해 맵의 배 실제 크기는 확인하지 못했다(원작 해양 맵 데이터 미조사).
세로 영상이 16:9를 좌우로 잘라낸 크롭이라 세로 비율은 보존된다고 **추론**했지만, 영상 제작 방식은 미확인이다.
따라서 **"원작 숫자를 그대로 넣는 것"과 "원작처럼 보이게 하는 것"은 이 프로젝트에서 서로 다른 목표다.**

## 3. 고친 내용

### 3.1 진단 로그 추가 — `Client/Private/Level_Bern.cpp` (21줄)

탑승·하차 때 **한 번씩만** 기존 진단 채널에 남긴다(매 프레임 아님).

```
ship.camera.applied  vehicle=8204 applied=1 distance=40.00 fovX=60.00 fovY=35.98
                     mapDistance=16.00 mapFovY=32.64 pitch=45.00 yaw=90.00 eye=(...)
ship.camera.applied  restored the map camera: distance=16.00 fovY=32.64
```

`Set_FollowPose`의 반환값(`applied=0/1`)까지 남기므로, 다음에 또 "안 바뀐다"는 일이 생기면
**렌즈가 적용조차 안 된 것인지 / 적용됐는데 화면 구도가 마음에 안 드는 것인지**를 로그로 가른다.

### 3.2 거리 17 m → 40 m — `Data/Camera/Bern.camera.json`, `Client/Public/ArenaCameraProfile.h`

각도·FOV·보간은 **원작 1001/1 그대로 두고 거리만** 프로젝트 값으로 바꿨다.
`provenance` 문자열에 원작 17 m와 바꾼 이유를 적어 두어 출처가 흐려지지 않게 했다.

거리별 화면 세로 점유율(피치 45° 최악, FOV 60 고정):

| 거리 | 세로 시야 | 유령선 8204 | 에스톡 8200 | 바크스톰 8203 |
|---|---|---|---|---|
| 17 m (원작) | 11.0 m | **107% (잘림)** | 56% | 42% |
| 30 m | 19.5 m | 61% | 32% | 24% |
| **40 m (적용)** | **26.0 m** | **46%** | **24%** | **18%** |
| 55 m | 35.7 m | 33% | 17% | 13% |
| 83 m | 53.9 m | 22% (원작과 동일) | 11% | 9% |

40 m을 고른 이유: 가장 큰 유령선이 46%로 **전체가 여유 있게 들어오고**, 작은 배들은 18~24%로
**원작 캡처의 21%와 같은 비율**이 된다. 83 m이면 유령선은 원작과 같은 22%가 되지만
작은 배들은 9~11%로 너무 멀어진다.

## 4. 검증

- `cl /Zs` (프로젝트 실제 옵션, `/utf-8` 포함) `Level_Bern.cpp`: **오류 0**.
  로그 `out/ShipCameraFix20260926/logs/zs_Level_Bern.log`. **구문 검사일 뿐 빌드·링크가 아니다.**
  `ArenaCameraProfile.h`는 이 파일이 포함하므로 같이 검사됐다.
- `Bern.camera.json` JSON parse 정상, `shipCamera` 7키 유지, 기존 필드
  (focusDistance 16, fovYDegrees 32.64, rotationDegrees [45,135,0]) **무변경**.
- `Server.exe --vehicle-riding-contract-test`: **77 PASS / 실패 0** (rc=0).
  로그 `out/ShipCameraFix20260926/logs/vehicle-riding.txt`.
  이 테스트는 13:20 빌드된 서버로 돈 것이고, 이번 변경은 Client와 데이터뿐이라 회귀 확인용이다.
- 렌더링 보호 파일(`Data/Rendering`, `Renderer.*`, `ShaderFiles`, `Client/Bin/DataFiles/Rendering`)
  `origin/main` 대비 **diff 0**.
- 인코딩·줄바꿈 보존: Level_Bern.cpp는 LF 유지(CRLF 0), ArenaCameraProfile.h와 Bern.camera.json은
  CRLF 유지(LF 단독 0), BOM 상태 불변.

**빌드는 하지 않았다**(지시대로). 조정자가 Debug 빌드를 한 번 더 돌려야 Client에 반영된다.

## 5. 바꾼 파일 (정확히 3개)

내 백업(`out/ShipCameraFix20260926/backup/*.before-shipcam-fix`) 대비 변경 줄:

- `Client/Private/Level_Bern.cpp` — 21줄 (진단 로그 2곳)
- `Client/Public/ArenaCameraProfile.h` — 4줄 (기본값 17→40 + 주석)
- `Data/Camera/Bern.camera.json` — 4줄 (distanceMeters, provenance)

세 파일 모두 내가 시작하기 **전부터 이미 M 상태**였다(H fork의 배 이식 결과).
그래서 `git status --short`의 파일 목록은 시작·끝이 동일하다.

참고: `git diff --check`가 `Client/Private/MainApp.cpp`의 줄 끝 공백을 경고하는데,
**이건 내 변경이 아니다.** 조정자가 CP949 컴파일 오류를 막으려고 한글 줄 끝 주석 5줄에
일부러 넣은 공백이다.

## 6. 사용자가 빌드 뒤 확인하는 순서

1. 조정자가 Debug Product 빌드 → Server·Client 재시작.
2. Lobby → Bern → Bern3 부두 배 NPC → 배 선택 → 탑승.
3. 바다에서 **배 전체(돛대 끝까지)가 프레임에 들어오고 주위 바다가 넓게 보이는지** 확인.
4. `Client/Default/EffectFailure.user.log`에서 `ship.camera.applied` 줄을 찾는다.
   `distance=40.00 fovY=35.98 applied=1`이 보이면 렌즈가 실제로 적용된 것이다.
   `applied=0`이면 `Set_FollowPose`가 거부한 것이므로 그 줄을 알려 주면 된다.
5. R로 하차 → `restored the map camera` 줄과 함께 원래 지도 카메라로 돌아와야 한다.

## 7. 값이 마음에 안 들 때 (빌드 불필요)

`Data/Camera/Bern.camera.json`의 `shipCamera.distanceMeters` 숫자만 고치고
Level을 다시 들어가면 반영된다. 후보:

- **30** — 유령선 61%. 지금보다 가깝고 배를 크게 보고 싶을 때.
- **40** — 현재 값. 유령선 46%, 작은 배 18~24%.
- **55** — 유령선 33%. 더 넓은 바다.
- **83** — 유령선 22%로 원작 캡처와 같은 비율. 작은 배는 9~11%로 멀어진다.

FOV를 넓히는 방법도 있다(`fovXDegrees`). 원작 부스터 값 70을 쓰면 같은 거리에서 세로 시야가
약 1.27배 넓어진다. 단 광각 왜곡이 커진다.

## 8. 되돌리는 방법

- 데이터만: `distanceMeters`를 17로 되돌리면 원작 숫자 그대로가 된다(빌드 불필요).
- 전부: `out/ShipCameraFix20260926/backup/`의 3개 파일을 제자리에 복사하고 빌드.

## 9. 못 한 것 / 사용자 결정

- **원작 항해 맵의 배 실제 크기 확인**(2.4절). 이것 없이는 "원작 17 m가 왜 원작에서는 넓게 보이는가"를
  확정할 수 없다. 필요하면 원작 해양 맵 데이터 조사를 별도로 해야 한다.
- 줌 단계(1001/2·1001/3), 정박(1002·1004), 부스터(1003) 카메라는 미적용. 원작 값은 2.3절에 있다.
- 원작 Yaw가 월드 고정인지 배 진행 방향 기준인지 여전히 미확인. 지금은 월드 고정 90°다.
  배 방향에 따라 시점이 어색하면 `yawDegrees`를 조정하거나 별도 작업이 필요하다.
- 실제 빌드·링크와 화면 결과는 확인하지 못했다.

## 10. 최종 재확인

- **확인한 것**: 코드 경로 전수(파싱·적용·덮어쓰기·클램프 없음), 원본 EFTable 두 테이블 직접 조회,
  배 9척 WModel 정점 실측, 거리별 점유율 계산, 앵커 1회 매치 패치, 인코딩·줄바꿈 보존,
  `cl /Zs` 오류 0, JSON parse, 탈것 테스트 77 PASS, 렌더링 보호 diff 0, 내 변경 파일 3개.
- **확인하지 못한 것**: 빌드·링크, 실제 화면, 원작 항해 맵의 배 크기, 원작 Yaw 기준.
- 커밋·push·브랜치 변경·게시·빌드는 하지 않았다. 다른 작업(마하라카 지형, 배 이식 파일)은 건드리지 않았다.

SHIP_CAMERA_FIX_DONE
