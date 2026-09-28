# 2026-09-28 베른 배(항해) 카메라 원본 적용 RESULT

브랜치 `codex/maharaka-map-restoration`. 빌드·실행은 하지 않았다(조정자가 마지막에 한 번 빌드).
구문 검사(`cl /Zs`, 산출물 없음)만 했다. 화면 판정은 사용자 몫이다.

## 1. 사용자 요청

- 마을(항구) 근처 바다의 점 경계 안에서는 카메라가 가깝고 비스듬하고, 벗어나면 더 멀고 높아진다.
- 우리 배 카메라는 전반적으로 너무 가깝다 → 원본 카메라 데이터로 전체 재조정.
- 항해 중 마우스 휠로 가까이/멀리 조절(영상 `로아 배 타는영 상.mp4` 28~48초).

## 2. 원본 근거

### 2.1 카메라 행 — `EFTable_CameraSetting.db` (`C:\LostArkExtract\MaharakaFunctions20260926\db\...`, mode=ro)

`EFTable_VoyageShip` 94행이 전부 `Camera_Ocean=1001`, `Camera_Booster=1003`, `Camera_Anchor=1002`(유령선 계열 1004).

| 행 | 이름 | FOV | Pitch | Yaw | ZoomDist | RelativeZ | InterpolationRatio |
|---|---|---|---|---|---|---|---|
| 1001/1 | 항해 기본뷰(ZoomIn 1단계) | 60 | -45 | 0 | 1700 | -50 | 2.0 |
| 1001/2 | 항해 연안뷰(ZoomIn 2단계) | 60 | -40 | 0 | 1300 | -10 | 2.0 |
| 1001/3 | 항해 선박뷰(ZoomIn 3단계) | 45 | -25 | 0 | 600 | 60 | 1.5 |
| 1002/1·2 | 정박모드 뷰(정박볼륨 외부/내부) | 60 | -35 | 5 | 1220 | 85 | 2.5 |
| 1003/1 | 쾌속운항 뷰(부스터) | 70 | -42 | 0 | 1700 | -50 | 2.0 |

`EFTable_IsometricCamera.db`에도 같은 값이 있다(SourceRow 86~91).
1001의 세 행 이름이 "ZoomIn 1/2/3단계"라서, 휠 줌 단계가 이 세 행이다.

### 2.2 배 메시 표시 배율 — 이전 작업이 못 찾은 값

원본 LookInfo `data4.lpk` `XmlData/LookInfo/Ship/EFDLShip_*.loa`(`out/Bern3Ship20260925/lpk4/...`)의
`CEFData_DefaultMesh` 블록 끝, `200.0`(0x43480000) 바로 앞 float이 배마다 다르다.

| 우리 id | VoyageShip.Model | 메시 | 배율 | LookInfo 오프셋 |
|---|---|---|---|---|
| 8200 | ESTOC_01 | SH_Estoc01_SK | 0.4 | 438 |
| 8201 | WHITEWIND_01 | SH_WindShip01_SK | 0.4 | 437 |
| 8202 | PIRATE_01 | SH_FastShip01_SK_LOC_INT | 0.35 | 443 |
| 8203 | ICEBREAKER_01 | SH_Icebreaker_01_SK | 0.45 | 441 |
| 8204 | GHOST_01 | SH_GhostShip01_SK | 0.35 | 442 |
| 8205 | BRAHMS_01 | SH_Brahms01_SK | 0.4 | 400 |
| 8206 | TRAGON_01 | SH_Tragon01_SK | 0.5 | 400 |
| 8207 | SLOOP_01 | SH_FastShip02_SK_LOC_INT | 0.26 | 441 |
| 8208 | MAGICSHIP_01 | SH_MagicShip01_SK | 0.43 | 367 |

`Tools/ShipPipeline/cook_ships.py`가 같은 메시를 굽는 것을 확인했다(45~62행).

이 값이 배율이라는 검증(영상 실측):
- 영상의 배는 이름표상 "에스토크"(8200)다.
- 30~37초(3단계 600/FOV45)와 38·41~42초(1단계 1700/FOV60)의 배 폭 비율은 약 4.3이다. 원본 값으로 계산한 3.95와 맞아서, ZoomDist가 실제 카메라 거리라는 것이 확인된다.
- 3단계에서 배는 화면 폭의 약 42%를 차지한다. 2×600×tan22.5° = 497 UU에 곱하면 약 209 UU다.
- 우리 에스토크 메시는 486 UU(`out/ShipFix20260925/ship_vertex_bounds.json`)이고, 486 × 0.4 = **194 UU**다. 실측과 맞는다.

**결론:** 원작은 배 메시를 0.26~0.5로 줄여 그리는데 우리는 1로 그린다. 원본 거리를 그대로 쓰면 배가 원작보다 2~4배 크게 보인다. 이전 작업이 17m→40→28→20m로 임의 조정한 원인이 이것이다.

### 2.3 정박볼륨(경계) — 원작 항해 맵 30703

- `leveldata1.lpk`의 `MapData\30703\DeployData.loa`를 추출했다(`$CLAUDE_JOB_DIR/tmp/ocean30703`).
- `VoyageAnchorVolume` ID가 `CEFDeployActor_Prop` 레코드 +100에 있다. 위치는 +0, 존은 +256, 크기(extent)는 +284에 있다.
- **1032005:** 위치 (12681, 16801), extent **(1250, 1250, 128)**, WarpZone 11111(`LV_BER_KronaP`, 크로나 항구).
  - 베른 존 11102 포털(12560, 16650)에서 약 190 UU 떨어져 있다.
  - 다른 항구들도 1152~1280 UU다.
- `LV_OCN_World_PS`에는 `efcoastvolume` 37개도 있다. 반경이 4,600~46,000 UU로 크고, 베른 포털은 연안 볼륨 4번(반경 25,898×40,960) 안에 있다.
- 영상에서는 "정박 가능" 문구가 사라지는 4~5초에 카메라가 멀어지기 시작한다. 12초 이후에도 연안 볼륨 안인데 카메라는 멀리 있다.
  - 그래서 전환 기준은 연안 볼륨이 아니라 **정박볼륨**으로 판단했다(추론).
  - 경계 안(0.5~4초)의 배는 경계 밖(12초 이후)보다 약 1.3배 크고 덜 내려다본다. 1300/-40과 1700/-45의 차이에 해당한다.

## 3. 우리 좌표로의 변환

- **각도:** 2026-09-14 규칙 (x, z, -y)을 따른다. pitch = -Pitch, yaw = Yaw + 90, FOV는 16:9 수평.
- **거리:** ZoomDist ÷ 100 ÷ 배 배율. RelativeZ도 같은 식으로 focus Y에 쓴다. 원작과 같은 화면 구도가 된다.

| 배 | 1단계(기본) | 2단계(연안) | 3단계(선박) |
|---|---|---|---|
| 8200/8201/8205 (0.4) | 42.5 m | 32.5 m | 15.0 m |
| 8202/8204 (0.35) | 48.6 m | 37.1 m | 17.1 m |
| 8203 (0.45) | 37.8 m | 28.9 m | 13.3 m |
| 8206 (0.5) | 34.0 m | 26.0 m | 12.0 m |
| 8207 (0.26) | 65.4 m | 50.0 m | 23.1 m |
| 8208 (0.43) | 39.5 m | 30.2 m | 14.0 m |

- **정박볼륨:** 1250 cm × (1 ÷ 0.4, 기본선 8200 배율) = 31.25 m. 중심은 우리 항만관리인 `npc.bern.ship.harbormaster.1`의 XZ (264.41, -204.03)이고 축 정렬 정사각형이다.
  - 원작 좌표계가 달라서 위치는 옮길 수 없다. 대신 "항구 정박 구역"을 우리 부두로 대응시켰다. 이 매핑은 프로젝트 결정이다.
  - BernSea 바다 칸의 약 12%가 이 안에 들어오고, 출항 지점(265.75, -215.75)도 안에 있다.
- **단계 규칙:** 탑승과 경계 통과 시 안은 2단계(연안뷰), 밖은 1단계(기본뷰)로 바뀐다. 휠 위는 ZoomIn(1→2→3), 휠 아래는 반대다.
- **전환:** 목표 단계의 InterpolationRatio(2.0/1.5)를 지수 감쇠 속도로 써서 거리·각도·FOV·focus를 부드럽게 보간한다. 영상에서도 1~2초에 걸쳐 부드럽게 바뀐다.

## 4. 변경 파일 (인코딩·CRLF 유지, vcxproj 변경 없음)

- `Data/Camera/Bern.camera.json`: `shipCamera` 블록만 새 형식으로 바꿨다. `zoomSteps` 3행은 원본 단위, 나머지는 `openSeaStep`, `anchorStep`, `anchorVolume`, `shipMeshScales` 9척, `followResponse`다. 다른 줄은 바이트 그대로다.
- `Client/Public/ArenaCameraProfile.h`: `ARENA_SHIP_CAMERA_STEP`, `ARENA_SHIP_MESH_SCALE`를 추가하고 `ARENA_SHIP_CAMERA`를 3단계 구조로 바꿨다.
- `Client/Private/ArenaCameraProfile.cpp`: 파싱(7키, 단계 행 6개 값), 저장(같은 형식으로 되돌려 씀), 검증 범위를 고쳤다. JSON 값 제한을 64→128로 올렸다(저장 최악 85개).
- `Client/Public/Camera_Free.h`(CP949, ASCII만 추가)와 `Client/Private/Camera_Free.cpp`: `Set_FollowLens`를 추가했다. 추적 초기화 없이 offset과 FOV만 바꾼다.
- `Client/Public/Level_Bern.h`, `Client/Private/Level_Bern.cpp`: `Update_ShipCamera(fTimeDelta)`를 다시 썼다.
  - 배별 배율 조회, 정박볼륨 판정, 휠 입력, 보간을 한다.
  - 휠은 ImGui·UI가 마우스를 잡았거나 프레젠테이션 중이면 무시한다.
  - 하차 시 지도 카메라 복귀는 그대로다.
  - 진단 `ship.camera.applied`는 탑승 때와 단계가 바뀔 때만 남긴다.

## 5. 적용하지 않은 것 (근거)

- **부스터 1003:** 부스트 상태 `iShipBoostEndTick`이 서버 전용이라 클라이언트로 복제되지 않는다(`2026-09-26_BERN3_SHIP_FOG_AND_BOOST_RESULT.md` 153행). 소비자가 없어 데이터에도 넣지 않았다.
- **정박모드 1002:** 우리 프로젝트에 Z 정박 모드가 없다.
- **원작 휠 한 칸의 정의:** 데이터에 없다. 휠 움직임이 있는 프레임마다 한 단계씩 움직인다.
- 영상의 5~9초 구름은 카메라가 아니라 원작 환경 연출이다.

## 6. 검증

- 원본 값은 sqlite(mode=ro)와 바이너리 오프셋으로 직접 읽었다. 9척 배율은 스크립트로 모두 다시 추출했다.
- `cl /Zs` 3개 파일(Level_Bern, ArenaCameraProfile, Camera_Free): rc=0, 오류 0. 새 경고 없음(기존 C4828만).
- `Bern.camera.json`: JSON parse OK, 1,936 bytes(8,192 제한 이내), 값 70개·깊이 4(파서 제한 128/4 이내), CRLF 57줄 유지.
- 옛 필드(`distanceMeters` 등) 참조 0건. 이 JSON의 다른 소비자 없음(MainApp은 주석만).
- `git diff --check` 통과.
- 원본 백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\ship_camera_backup\`.

## 7. 사용자 확인 순서 (빌드 후 Client 재시작 필요, Server·게시 불필요)

1. 베른 → 항구 NPC → 배 탑승: 부두 앞(정박 구역)에서 약 32 m(에스토크 기준) 거리로 비스듬하게 보이는지.
2. 항구에서 31 m 이상 벗어나기: 1~2초에 걸쳐 약 42 m, 더 위에서 내려다보는 시점으로 바뀌는지.
3. 휠 위: 연안(32 m) → 선박뷰(15 m, 옆에서 보는 각도)로 가까워지는지. 휠 아래: 반대.
4. 항구 구역으로 돌아오면 다시 연안뷰로 바뀌는지. 하차하면 원래 베른 카메라로 복귀하는지.
5. 거리 수치는 `EffectFailure.user.log`의 `ship.camera.applied` 줄(step, anchorVolume, targetDistance)로 확인할 수 있다.
