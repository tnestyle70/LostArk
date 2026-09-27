# Bern3 배 탑승·항해 기능 — 조사와 진행 기록

작성 시작: 2026-09-25. 이 문서는 진행 중인 작업의 누적 기록이다. 마지막 절 "현재 상태와 남은 작업"이 이어받기 기준이다.
정직 표기: **사실**(파일·표에서 직접 읽음), **추론**, **미확인**을 구분한다. 화면·소리·카메라 느낌은 사용자가 판정한다.

## 1. 원작 데이터 위치와 추출 방법 (사실)

- 원본 게임은 이 PC에 설치되어 있다: `C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\` (`data2.lpk` 표 DB, `data4.lpk` LookInfo, `ReleasePC\Packages` upk).
- 표(DB)는 저장소의 `Tools/LpkPipeline/unpack_lpk.py`로 복호화했다: `python Tools/LpkPipeline/unpack_lpk.py <data2.lpk> --out out/Bern3Ship20260925/lpk --filter EFTable_` (797개, 약 50초). LookInfo는 `data4.lpk`에 `--filter EFDLShip_`.
- 메시·재질·텍스처는 `C:\LostArkExtract\umodel\umodel_lostark_v7.exe`, 애니메이션은 같은 도구의 AnimSet export(PSA)로 얻는다(`-gltf`와 같이 주면 AnimSet이 빠지므로 `-obj=<animset>`을 따로 실행).
- 한글 문자열은 `EFTable_GameMsg.db`에 있고 행마다 UTF-8 또는 CP949가 섞여 있다(UTF-8을 먼저 시도하고 실패하면 CP949).
- 작업 산출물(git 제외): `out/Bern3Ship20260925/` (`lpk/`, `lpk4/`, `raw/`, `survey_ships.json`, `ship_lookinfo_refs.json`).

## 2. 원작 배 종류 (사실: `EFTable_VoyageShip.db` 94행, `EFTable_VoyageShipAvatar.db` 13행)

기본 배 9종(각 11단계, 루미나스만 6단계, 1~11단계에서 이동 속도·탑재 물량 등이 오르고 일부 단계에서 모델/재질이 바뀜):

| ID | 한글 이름 | 기본 모델(LookInfo) | 단계별 모델 변화 | 카메라(바다/부스터/정박) |
|---|---|---|---|---|
| 8200 | 에스토크 | ESTOC_01 | 4단계 ESTOC_02, 8단계 ESTOC_05 (메시 동일, 재질만 변경) | 1001/1003/1002 |
| 8201 | 풍백 | WHITEWIND_01 | 4단계 _03, 8단계 _05 (메시 동일) | 1001/1003/1002 |
| 8202 | 아스트레이 | PIRATE_01 | 8단계부터 PIRATE_02 (메시 SH_FastShip03) | 1001/1003/1002 |
| 8203 | 바크스톰 | ICEBREAKER_01 | 8단계부터 _02 (메시 Icebreaker_02) | 1001/1003/1002 |
| 8204 | 에이번의 상처 | GHOST_01 | 8단계부터 _02 | 1001/1003/**1004** |
| 8205 | 브람스 | BRAHMS_01 | 8단계부터 _02 (재질만) | 1001/1003/1002 |
| 8206 | 트라곤 | TRAGON_01 | 8단계부터 _02 (재질만) | 1001/1003/1002 |
| 8207 | 프뉴마 | SLOOP_01 | 없음 | 1001/1003/1002 |
| 8208 | 루미나스 | MAGICSHIP_01 | 없음(6단계) | 1001/1003/1002 |

외형 변경(아바타) 배 13종(721003~721016, 카메라 정박 1005~1007 등): WACHSTUM, ASTRAL, WAR, PR_ISKW_02 계열 6종(범고래), TURTLE, DraculaShip, PumpkinShip, LeafFenuma 등. 이번 범위는 기본 9종이다(아바타는 미포함, 아래 "미완료" 참조).

이동 속도(표 `MoveSpeed`, 1단계): 에스토크 200, 풍백 190, 아스트레이 220, 바크스톰 180, 에이번 183, 브람스 185, 트라곤 180, 프뉴마 210, 루미나스 200. 부스트 속도 75~100, 부스트 지속 5초. 충돌 X/Z 100. `PathfindingType` 2. 원본 단위(cm/s 또는 uu/s)는 **미확인**: 기존 탈것(`EFTable_Vehicle.MoveSpeed`)은 cm/s를 100으로 나누어 m/s로 쓴다. 배에 같은 규칙을 쓰면 2m/s 안팎이 되어 원작 감각과 다를 수 있다(추론). 최종 값은 구현 단계에서 사용자 플레이어 걷기 속도와 비교해 정한다.

## 3. 배 모델·애니메이션 (사실: LookInfo `data4.lpk`, umodel 목록)

- 모든 배는 `EFDLShip_*` LookInfo가 SkeletalMesh + AnimSet + AnimTree(`Ship_Common.AnimTree.Common_AnimTree`) + MaterialInstanceConstant + 물결 파티클을 가리킨다.
- 에스토크 실측(`SH_Estoc01`): SkeletalMesh 1개(본 135, primitive 2), AnimSet `SH_Estoc01_Ani`의 클립 7개, 모두 30fps: `run_battle_1`(31프레임=1.03초), `idle_battle_1`(31), `run_battle_2`(25), `run_battle_3`(19), `dead_1`(31), `idle_normal_1`(31), `walk_normal_1`(31). 소켓 18개(`skeletalmeshsocket_*`), 텍스처 `sh_estoc01_da_loc_int`(diffuse), `sh_estoc01_n_loc_int`(normal), `sh_estoc01_mk`(마스크), 돛 무늬/상징 `sh_pattern*`·`sh_simbol*`.
- 다른 배의 AnimSet 이름(LookInfo): `SH_WindShip01_Ani`, `SH_FastShip01_Ani`(아스트레이·프뉴마·LeafFenuma 공유), `SH_Icebreaker_01_Ani`/`_02_Ani`, `SH_GhostShip01_Ani`, `SH_Brahms01_Ani`, `SH_Tragon01_Ani`, `SH_MagicShip01_Ani`. 추출해 확인한 각 배의 클립 목록은 9절에 있다(대부분 `idle_normal_1`, `idle_battle_1`, `run_battle_1~3`).
- 재질은 돛 전용 마스터(`preset_ch_shipsail_msk`)를 쓰며 돛 무늬/상징/바람 흔들림 파라미터가 있다. 프로젝트의 기존 skinned 재질 경로는 diffuse+normal이므로 **돛 무늬·바람 흔들림은 1차 범위에서 복원되지 않는다**(미복원으로 보고).

## 4. 탑승 카메라 (사실: `EFTable_CameraSetting.db`, 표 안의 이름은 CP949)

| ID | 용도 | FOV | Pitch | ZoomDist | 상대 Z | 보간 |
|---|---|---|---|---|---|---|
| 1001-1 | 항해 기본(확대 1단계) | 60° | -45° | 1700 | -50 | 2.0 |
| 1001-2 | 확대 2단계 | 60° | -40° | 1300 | -10 | 2.0 |
| 1001-3 | 확대 3단계 | 45° | -25° | 600 | 60 | 1.5 |
| 1002 | 정박 | 60° | -35° | 1220 | 85 | 2.5 (Yaw 5) |
| 1003 | 부스터 | 70° | -42° | 1700 | -50 | 2.0 |
| 1004~1007 | 일부 배의 정박 변형 | 60~65° | -35° | 1220~1320 | 65~85 | 2.5 |

ZoomDist는 cm 단위로 보인다(1700 = 17m, 추론: 프로젝트 Follow camera 기본이 16m). 프로젝트의 `Data/Camera/*.camera.json` 계약(FOV·거리·pitch)에 대응시킨다.

## 5. 배 관련 NPC (사실/추론)

- 원작 NPC 상호작용 기능 표(`EFTable_NpcInteractionFunction`)에 `sys.interaction.fla_sail`(기능 5, 출항)이 있고, 이름 표에 `[아스트레이 조선공]`(`npcfunction_shipmaker_ars`) 기능이 있다.
- NPC 이름 예(GameMsg): 조선소 장인 페펜(15004)/코작(15005)/바시르(15006), 조선공 란첸베르크(18545), 선장 베로나(16008), 항해사 로사(19446) 등.
- 조사 시점의 `NpcCatalog.json`에는 배/항구 NPC가 없었다. 이후 원작 모델로 `NPC_SHIP_SHIPWRIGHT`(조선공 19991)와 `NPC_SHIP_HARBORMASTER`(NP_LRKK_01)를 추가했다(9절).

## 6. Bern3와 베른의 배 오브젝트 (사실: 게시 navgrid 실측)

- 네비 영역: `LV_BER_BERNCASTLE` = Bern, Bern2, Bern3 세 영역. **Bern3는 사용자 저작 정본이므로 수정하지 않는다.**
- Bern3 격자 191×110, 셀 0.5m, 원점 (206.5, -220.0), 걸을 수 있는 셀 4,764개(약 1,190㎡), X 212.5~296.0, Z -219.5~-165.0, 높이 7.4~22.4m(대부분 12~14m). 모양은 부두·갑판 띠 형태다.
- 베른 맵의 정적 배 모형(SL02): `COMMON_SHIP01` 7개(예 (257.8,11.0,-184.0), (241.8,10.8,-181.0)), `ESTOCSHIP01`/`01B` 2쌍((278.6,9.1,-201.1), (225.4,9.1,-204.7)), `COMMON_SHIP04` 7개(예 (266.7,12.5,-191.1)). 모두 Bern3 범위 안이다.
- 사용자가 말한 "커다란 배 오브젝트"의 정확한 대상은 **미확인**이다. 가장 큰 배로 보이는 `ESTOCSHIP01`(에스토크형) 근처를 NPC 후보 위치로 삼되 확정은 좌표 실측 뒤에 한다.

## 7. 프로젝트 안의 재사용 가능한 부분 (사실)

- 탈것 시스템: `Data/Actors/VehicleCatalog.json`(Client 표현), `Data/Vehicles/VehicleProfiles.json`(Server 속도), `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`, `Server/Private/GameRoom_VehicleRiding.cpp`, `Client/Private/Part_Vehicle.cpp`, `VehiclePresentationAssetService`, `VehicleWindowView`(원작 탈것 창 UI). H 키 `C2S_SET_VEHICLE_RIDING`.
- 모델 조립 정본 경로(저장소 안): `UModel glTF + PSA` → `Tools/ActorXAssetCooker/build_umodel_gltf_psa.py --scale 100` → `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe` → `retime_wmodel_ticks.py`(30틱) → `verify_dimensionmaster_summon_bind_pose.py`. 예시 wrapper: `Cook-KoukuSaydonInteractionProps.ps1`. Converter는 한글 절대경로를 못 읽을 수 있어 ASCII 작업 폴더에서 실행한다.
- NPC 배치: `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`(kind=npc) → `Publish-WorldGameplay.ps1`. 이전 NPC 상호작용(`GUIDE_NPC_PLACEMENT_IDS`, `Request_ConfirmNpcEntry`)은 코드 하드코딩 방식이다(현재 상태는 구현 단계에서 재확인).

## 8. 설계 결정 (사실 + 결정 근거)

- 배는 기존 탈것 시스템의 새 항목으로 만든다. 근거: Server가 `C2S_SET_VEHICLE_RIDING`의 vehicleId로 탑승을 승인하고 이동 속도를 `Vehicles.bootstrap`에서 읽으며, 우클릭 이동은 이미 그 속도로 동작한다. 새 protocol이나 Server C++ 변경이 필요 없다(protocol 105 유지, main 번호 충돌 위험 없음).
- 배 ID는 원작 `EFTable_VoyageShip.PrimaryKey`(8200~8208)를 그대로 vehicleId로 쓴다. 기존 탈것 ID와 겹치지 않는다.
- Bern3는 사용자 정본이라 수정하지 않는다. 배는 플레이어가 서 있는 영역의 네비 위에서 배 속도로 움직인다(지금 Bern3 걷기 가능 셀은 부두와 갑판이다).
- 카메라는 Bern follow camera 프로파일을 바탕으로 거리와 FOV만 원작 값으로 바꾼다.

## 9. 진행 상황 (2026-09-25 03:55 기준, 실제 확인한 것만)

완료(근거 포함):

- **원작 조사**: 배 종류·모델·LookInfo·카메라·NPC 이름 표(위 1~6절). 추출 도구 `Tools/ShipPipeline/cook_ships.py`.
- **배 9종 모델**: `Client/Bin/Resources/Character/Vehicle/Ship_<Name>/`에 wmodel + diffuse/normal DDS 설치. 재확인(03:52): 9종 모두 조립 결과물과 바이트 동일, 클립 30틱, 본 수 28~136. 클립은 원작 AnimSet 그대로 `idle_normal_1`, `idle_battle_1`, `run_battle_1~3` 등(에스토크·유령선은 `dead_1` 포함, 루미나스는 `walk_normal_1` 포함).
- **NPC 2종 모델**(원작): 조선공(EFTable_Npc 19991, MN_RHKP_02-2, 머리 MN_Head_MA04_012 + 몸 MN_RHKP_00) = `Character/NPC/Npc_MN_RHKP_02_2/`, 여객선 계열(NP_LRKK_01, 머리 Head_MA02_001 + 몸 NP_LRKK_00) = `Character/NPC/Npc_NP_LRKK_01/`. 머리를 몸 메시에 합치는 도구 `Tools/ShipPipeline/cook_npc.py`를 새로 만들었다(머리가 실제로 쓰는 뼈의 바인드 자세 차이가 1cm 이내일 때만 합침. 여객선의 실측 차이는 4.3mm이고, 조선공은 처음의 0.1cm 허용치도 통과했으므로 그 이하이지만 값은 따로 기록하지 않았다). 무기는 붙이지 않았다.
- **데이터**: `VehicleCatalog.json`(배 9항목), `VehicleProfiles.json`(9항목, 속도 출처 `EFTable_VoyageShip`), `VehicleUiCatalog.json`(9행 + `shipTitle`), 아이콘 `UI/Vehicle/Icons/ship_<id>.png` 9개, `NpcCatalog.json`(`NPC_SHIP_SHIPWRIGHT`, `NPC_SHIP_HARBORMASTER`), `Gameplay.world.json`(배 NPC 2개 배치, 리비전 921→922).
- **Client 코드**: 카탈로그 선택 키 `ship`·`seatOffset`·`modelYawDegrees`, 탈것 창 배 전용 목록과 휠 스크롤, `CMainApp::Open_ShipWindow`, Bern 배 NPC 우클릭·걷기·창 열기, 탑승 카메라. protocol과 Server C++은 바꾸지 않았다.
- **게시(03:52, 잠금 보유 중, 막는 프로세스 0)**: `Publish-VehicleProfiles.ps1 -Mode Publish`(탈것 16종, 스킬 25개) → `Vehicles.bootstrap` 헤더 32→41행, 배 9행 추가. `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN` → `BERN.worldbootstrap`(66 placements) 갱신, `BERN.npcpresentation.json`은 바이트 동일(배 NPC는 카탈로그를 쓰므로 변경 없음). 게시 전 백업: `out/Bern3Ship20260925/backup/published/`(해시 `before.sha256`). 두 게시 도구의 `-Mode Validate`도 통과.
- **월드 게시 도구 차단 해결**: 이 브랜치의 `Publish-WorldGameplay.ps1`은 이미 커밋된 쿠크 인카운터 데이터(커밋 ff868f0a `codex/kouku-gate3-bingo-flow-0923`)의 `parentPatternSequence` 필드를 거부했다. origin/main(커밋 7cea970b)이 같은 필드를 받는 방식을 그대로 이식했다: 새 파일 `Tools/KoukuSaydonPipeline/KoukuParentSequenceContract.ps1`(main 바이트 그대로 4,182 bytes)과 게시 도구 5줄 추가. 안전성 증거: 모든 월드를 스크래치 폴더에 게시해 라이브와 비교했더니 19개 산출물 중 `BERN.worldbootstrap` 하나만 달랐고 그 차이는 배 NPC 2개와 리비전뿐이었다.
- **Debug Product 빌드**: PASS(잠금 보유 중). 이번 실행은 새로 컴파일한 것이 0개였다(다른 작업의 03:49~03:50 빌드가 이미 내 소스 변경을 컴파일). 실행 파일에 내 코드가 들어 있음을 확인했다: Server.exe(03:50)에 배 계약 테스트 문자열, Client.exe(03:49)에 `NPC_SHIP_`·`seatOffset`·`shipTitle` 문자열이 있고, 두 exe는 내 마지막 수정(03:41)보다 새것이다. 실행 데이터 검사 36건 누락·손상 0.
- **서버 계약 테스트** `Server.exe --vehicle-riding-contract-test`: 59건 통과, 실패 0건, 종료 코드 0. 새 케이스: 배 9종의 속도가 게시본에서 원작 표 값(2.0/1.9/2.2/1.8/1.83/1.85/1.8/2.1/2.0 m/s)으로 읽힘, Bern에서 탑승 승인과 속도, 배 교체, 하차 후 걷기 속도 복구, 사망 시 강제 하차, 레이드 아레나 거절. 기존 탈것 케이스도 모두 통과.
- **팀원 전달 목록**: 저장소 루트 `Resource_Distribution_2026-09-25_Bern3Ship.txt` + `Copy_ResourceDistribution_2026-09-25_Bern3Ship.ps1`(새 파일 46개, 약 57.7 MB, 목록의 파일이 모두 실제로 있고 두 목록이 같음, 복사 스크립트는 실행하지 않음). Resources는 Git 비추적이다.

미완료/한계(정직 표기):

- NPC 대화창(인사말·기능 버튼)의 원본 UI는 복원하지 않았다. 배 NPC를 우클릭해 걸어가면 배 선택 창이 바로 열린다.
- 배 선택 창은 원작 탈것 창(vehicleWnd) 크롬을 재사용한 목록이며 원작 항해 창(voyage)이 아니다. 제목 "선박"과 스크롤 안내는 원본 표로 확인한 문구가 아니다. 아이콘은 `Voyage_Ship_1_<정렬순서-1>`로 정렬 순서에 따른 추정 대응이다.
- 재질은 diffuse+normal만이다. 돛 무늬·상징·바람 흔들림·물결 파티클은 복원하지 않았다.
- 배 속도는 원본 `MoveSpeed`를 100으로 나눈 값(1.8~2.2 m/s)이다. 원본 단위가 확정되지 않았고 걷기 속도(약 2.8 m/s)보다 느리다.
- 카메라 확대 2·3단계, 정박(1002)·부스터(1003) 카메라는 구현하지 않았다.
- 갑판 높이(`seatOffset`)는 정점 분포로 추정한 값, 배 앞방향은 다른 탈것과 같은 규칙(모델 +X를 -90도)으로 가정한 값이라 화면에서 조정이 필요할 수 있다(카탈로그의 `seatOffset`, `modelYawDegrees`만 바꾸면 된다). 배에서 캐릭터가 서는 자세는 직업 전투 대기 클립(`<직업>_idle_battle_1`)이며 원작의 항해 자세인지는 확인하지 못했다.
- 배가 물 위에서 움직이는 이동 영역은 사용자가 저작한 Bern3 네비를 그대로 쓴다(수정하지 않음). Bern3의 걷기 가능 셀은 부두와 갑판 띠 모양이라 "바다를 항해"하는 넓은 이동은 그 네비가 있는 범위로 제한된다.
- 배 NPC 위치는 사용자가 말한 "커다란 배 오브젝트"를 확정하지 못하고 `ESTOCSHIP01` 두 개 근처 걷기 가능 셀 3~5m로 정했다(가정).
- 기본 9종의 상위 등급(4·8단계) 모델 변경과 외형 변경 13종은 이번 범위에 넣지 않았다. NPC 무기 부착도 하지 않았다.
- **Client 실행·화면·소리는 아무도 확인하지 못했다**(에이전트는 Client를 실행하지 않는다). 클라이언트 쪽 배 표시·탑승 자세·카메라·NPC 창은 컴파일과 데이터 검사까지만 확인했다.

## 10. 남은 작업(이어받기)

1. 사용자 화면 확인(Debug Server와 Client를 함께 재시작해야 게시본이 반영됨): Bern → Bern3 항구의 배 NPC 2명을 우클릭 → 배 선택 창(9종, 휠 스크롤) → 한 척 탑승 → 우클릭 이동 → 카메라 → 탑승 해제(창의 하차 버튼).
2. 화면에서 어긋나는 값 조정: 배 앞방향(`modelYawDegrees`), 갑판 높이(`seatOffset`), 배 크기(`modelPreScale`), 이동 속도. 모두 `VehicleCatalog.json`/`VehicleProfiles.json`(속도는 `Publish-VehicleProfiles.ps1` 재게시 필요)의 값 수정이다.
3. main 병합 시: main은 이미 protocol 110이다. 이 작업은 protocol을 바꾸지 않았으므로 번호 충돌은 없다. 다만 `Publish-WorldGameplay.ps1`의 5줄과 새 파일 `KoukuParentSequenceContract.ps1`은 main에 이미 있는 것과 같은 내용이라 병합에서 겹칠 수 있다(main 것을 채택하면 됨). `Publish-VehicleProfiles.ps1`의 속도 출처 허용은 이 작업이 추가한 것이다.
4. 새 큰 기능(원작 NPC 대화창, 항해 창 복원, 카메라 3단계 확대·부스터·정박 카메라, 상위 등급 모델, 외형 변경 13종, 돛·파티클 재질)은 착수하지 않았다.

## 11. 사용자 요청으로 배 NPC 2명 제거 (2026-09-25)

사용자가 게임에서 이 NPC를 찾지 못했다고 알려 와서, 요청대로 배 NPC 2명만 제거했다. 이 NPC가 화면에 렌더링되는 것은 한 번도 확인된 적이 없다(파일·서버 테스트 수준까지만 확인했었다).

지운 것:

- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 placement 2개: `npc.bern.ship.shipwright`, `npc.bern.ship.harbormaster` (배치 66 → 64).
- `Data/Actors/NpcCatalog.json`의 archetype 행 2개: `NPC_SHIP_SHIPWRIGHT`, `NPC_SHIP_HARBORMASTER` (128 → 126행). 지운 뒤 이 파일은 HEAD와 바이트까지 같다.
- 게시본 `Server/Bin/DataFiles/World/BERN.worldbootstrap`의 두 행. 직접 편집하지 않고 `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN`으로 다시 게시했다(다른 월드는 게시하지 않음). 게시 전후로 다른 World 출력 22개는 바이트가 같고, 바뀐 것은 이 파일 하나다.

남긴 것(건드리지 않음):

- 배 탈것 데이터·코드·리소스 전부(`VehicleCatalog.json`, `VehicleProfiles.json`, `VehicleUiCatalog.json`, `Vehicles.bootstrap`, 배 9종 모델·아이콘, Client 배 창·카메라 코드).
- NPC 모델 리소스 폴더 2개(`Character/NPC/Npc_MN_RHKP_02_2`, `Npc_NP_LRKK_01`)와 팀원 전달 목록 파일.
- 참고: NPC가 없으므로 지금은 배 선택 창을 여는 방법이 없다. 배 탑승 기능은 NPC 없이는 게임에서 시작할 수 없는 상태다.

참고 사항:

- 월드 문서의 `revision`은 배 작업 때 올라간 922 그대로 두었다(카운터이며 되돌리지 않았다). 게시본 헤더도 922다.
- Client 게시본 `BERN.npcpresentation.json`에는 배 NPC 문자열이 처음부터 없었다(게시 전후 내용 동일). 이 임무는 원인 조사가 아니라 제거였으므로 원인은 확인하지 않았다.
- 백업: `out/Bern3ShipNpcRemoval20260925/backup/`(편집 전 JSON 2개, 게시 전 World 출력 전체, sha256 목록).

## 12. 배 NPC 재배치와 안 보이던 원인 (사용자 좌표)

사용자가 지정한 좌표에 배 NPC 3개를 다시 배치하고 게시했다: 조선공 (271.59, 12.43, -196.70), (229.61, 12.47, -196.50),
항만 관리인 (264.41, 11.79, -204.03). 안 보이던 원인은 배 NPC 모델 두 개가 NPC 크기 계약(RootNode 아래 x100 노드)을
지키지 않아 클라이언트 배율 0.0001에서 약 1.3cm가 된 것이었다. 로더가 그런 센티미터 모델을 0.01로 불러오게 고쳤고,
클라이언트·서버 진단 로그를 넣었다. 상세 근거와 확인 순서는 `2026-09-25_BERN3_SHIP_NPC_VISIBILITY_RESULT.md`.
화면 확인은 아직 없다.

## 13. 배 탑승을 원작 방식으로 변경 (2026-09-25)

스크린샷에서 배가 부두 위에 올라앉고 캐릭터가 갑판에 보이던 것은 이 문서 앞부분의 초기 구현(탑승해도 위치가 그대로)이 원인이었다. 원작에서는 마을에서 바다 구역으로 이동한 뒤 배만 보인다는 조사 결과에 따라 다음과 같이 바꿨다. 상세 근거·가정·검증은 `2026-09-25_BERN3_SHIP_ORIGINAL_BOARDING_RESULT.md`가 정본이다.

- 배를 고르면 Server가 플레이어를 항구 앞 바다 영역 `LV_BER_BERNCASTLE.BernSea`(새 평평한 네비 상세 영역)의 가장 가까운 열린 셀로 옮기고 부두 위치를 기억한다. 하차·사망 등 강제 하차에서 그 위치로 돌아온다. 100m 안에 열린 바다가 없거나 Bern이 아닌 월드면 탑승을 거부한다.
- 배를 탄 동안 Client는 캐릭터 몸·장비·이름표를 그리지 않고 배만 그린다. 우클릭 목표는 물 표면 평면(뿌리 높이 + 1.5m)과의 교점이다.
- 앞 절의 "배가 물 위에서 움직이는 이동 영역은 사용자가 저작한 Bern3 네비를 그대로 쓴다"는 더 이상 맞지 않는다. 바다 이동은 `BernSea`에서 하고 Bern3 네비는 수정하지 않았다.
- protocol 105 불변, 새 리소스 없음, Navigation 게시 파일 BernSea 7개 추가와 `navregions` 2개 수정. Server와 Client 재시작 필요. 화면 확인은 아직 없다.
