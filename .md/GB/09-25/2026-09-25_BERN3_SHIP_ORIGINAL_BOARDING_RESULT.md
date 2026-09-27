# Bern3 배 탑승: 원작 동작 조사와 적용 (2026-09-25)

이 문서는 진행 중인 작업의 누적 기록이다. 표기: **사실**(원본 표·문자열·파일에서 직접 읽음), **추론**, **확인 못 함**.
화면·소리·카메라 느낌은 사용자만 판정한다. 에이전트는 Client를 실행하지 않았다.

## 1. 사용자 스크린샷 관찰 (사실)

- 스크린샷 2장(2026-09-25 14:56): 검은 돛의 큰 배가 부두 돌바닥 위에 올라앉았고 플레이어가 갑판 위에 작게 서 있으며 이름표 `Test-35288`이 돛 사이에 떠 있다. 다른 한 장은 작은 회색 배가 같은 부두 위에 놓이고 플레이어가 갑판 위에 서 있고 옆에 NPC가 서 있다.
- 두 경우 모두 배가 바다가 아니라 부두 위, 플레이어가 서 있던 자리에 생겼다.
- 원인(코드 사실): `GameRoom_VehicleRiding.cpp`의 탑승 승인은 `iVehicleId`만 바꾸고 위치를 옮기지 않는다. Client `CPart_Vehicle`은 캐릭터 발 위치를 배 루트로 쓰고 캐릭터를 갑판(seat) 위에 세운다.

## 2. 원작에서 배 탑승이 동작하는 방식

근거 자료: 이 PC의 원본 게임(`C:\ProgramData\Smilegate\Games\LOSTARK\EFGame`)의 표 DB `data2.lpk`(저장소 `Tools/LpkPipeline/unpack_lpk.py`로 복호화, `out/Bern3Ship20260925/lpk`)와 LookInfo `data4.lpk`. 한글 문자열은 `EFTable_GameMsg`.

### (a) 배가 생기는 곳 — 항구 근처 "바다"이고 별도 바다 구역이다
- **사실**: 출항은 육지(마을) 구역에서 바다 구역으로의 이동이다. GameMsg `sys.hint.common_1005`: "대륙에 있는 상태에서 월드 맵의 '출항하기' 버튼을 누르면 즉시 바다로 이동할 수 있습니다." `sys.squarehole.dlg_title_voyage`/`btn_worldmap_voyage`: "출항 준비". NPC 기능 표(`EFTable_NpcInteractionFunction`)의 기능 5 `sys.interaction.fla_sail`이 출항 기능이다(파라미터 없음).
- **사실**: 바다 구역은 `EFTable_ZoneBase`에서 레벨 `LV_OCN_World_PS`(존 30703/30704, Type 6)이고 항해 관련 존 30733~30737(Type 19)도 있다. 베른 성 마을은 `LV_BER_BernCastle_T_PS`(존 11102, Type 1)이다.
- **사실**: 항구는 `EFTable_VoyageAnchorVolume` 45행이다. Type 0이 17행(항구, `WarpZoneId`=입항 때 이동할 마을 존, 입항 조건 레벨/퀘스트), Type 1이 27행(원주민 마을, `AnchorTime` 1200초). 항구 근처 바다에 정박 볼륨이 있고, 볼륨 안에 들어가면 HUD "정박 가능"(`sys.voyage.hud_into_anchor_volume`)이 뜨며 정박 모드에서 선박 관리와 "입항하기"(`ui_anchor_btn_move_port`)를 한다. "출항하기"(`ui_anchor_btn_move_sea`)와 "항해 시작"(`ui_anchor_btn_setsail`) 버튼도 있다.
- **확인 못 함**: 각 항구의 바다 위 출항 좌표(레벨 데이터의 정박 볼륨 액터). 바다 레벨(`LV_OCN_World_PS`)의 upk는 이 PC에 있으나 이번에 추출하지 않았다. 베른 성 마을에서 바로 출항하는 항구 구성인지도 확인하지 못했다(베른 성 존 11102는 `WarpZoneId` 목록에 없다).

### (b) 플레이어 캐릭터는 보이는가 — 배만 보이는 것으로 판단한다
- **사실**: 배의 LookInfo(`EFDLShip_*.loa`)는 `CEFData_DefaultMesh`+`CEFParticleData`이고 SkeletalMesh, MaterialInstance, AnimSet, PhysicsAsset, AnimTree(`Ship_Common.AnimTree.Common_AnimTree`), 꼬리 궤적(`WaterTrail`), 소리(Idle/Move/Run/Turn)를 담는다. 좌석 소켓이나 탑승자 정보가 없다. (몬스터 LookInfo도 같은 `DefaultMesh` 계열이다.)
- **추론**: 배가 조종하는 폰의 기본 메시를 대체한다. 사용자 진술("바다 위에 배만 떠 있고 플레이어는 안 나온다")과 일치한다.
- **사실**: 항해 중에는 스킬 슬롯 변경 불가(`sys.skillbook.alert_cannot_change_quickslot_case_voyage`). 난파 중에는 입항 외 상호작용 불가(`sys.common.result_failure_interaction_ship_wrecked`).
- **확인 못 함**: 선원(crew)이 갑판에 보이는지, 이름표가 항해 중 어떻게 표시되는지.

### (c) 전환 연출 — 대화창은 확인, 나머지는 확인 못 함
- **사실**: "출항 안내"(`sys.voyage.dialog_takeoffnotice_title`) 대화창이 선박 상태(내구도 부족, 선원 미탑승, 내성 포인트 미사용)를 보여 주고 "이대로 출항 하시겠습니까?"를 묻는다. 마을에서 "즉시 바다로 이동"한다.
- **확인 못 함**: 페이드, 사운드, 카메라 전환, 대기 시간.

### (d) 항해 이동 규칙
- **사실**: `EFTable_VoyageShip`(94행, 기본 9종): `MoveSpeed`, `BoostSpeed`/`BoostDuration`(5초), `CollisionX/Z`=100, `PathfindingType`=2, 카메라 `Camera_Ocean/Booster/Anchor`. 카메라 표 1001(항해), 1003(부스터), 1002(정박). 앞선 작업에서 속도·카메라를 이미 적용했다.
- **추론**: `PathfindingType`이 있으므로 목적지를 눌러 경로로 이동하는 방식이다(사용자 진술과 일치). 원본 단위(cm/s)는 확정하지 못했다.
- **확인 못 함**: 가속, 회전 반경, 바다 구역의 이동 가능 영역 규칙.

### (e) 복귀
- **사실**: "입항하기"로 항구의 마을 존(`VoyageAnchorVolume.WarpZoneId`)으로 이동한다.
- **확인 못 함**: 마을 안 정확한 복귀 좌표(NPC 앞 부두의 어느 지점인지).

### (f) 탑승 중 UI와 스킬
- **사실**: 항해 중 스킬 슬롯 변경 불가, 정박 볼륨 밖에서는 선박 변경 불가(`sys.voyage.ui_anchor_failure_not_in_anchorvolume`: "선박에 관한 각종 변경은 항구 근처에서만 할 수 있습니다").

## 3. 이 프로젝트에서의 대응 (설계, 가정 표시)

이 프로젝트에는 별도 바다 레벨이 없다. 원작의 "마을 → 바다 구역 이동"을 **같은 베른 월드 안의 항구 앞 바다 영역으로 서버가 이동시키는 것**으로 근사한다.

1. 배를 고르면 서버가 플레이어를 항구 앞 **바다 위 출항 지점**으로 옮기고(서버 권위, 기존 텔레포트 방식), 원래 서 있던 부두 위치를 기억한다. (원작 좌표 근거 없음 → **가정**: 플레이어가 서 있는 부두에서 가장 가까운 열린 바다 셀.)
2. 배가 **아바타**다. Client는 캐릭터 몸·장비·무기·이름표를 그리지 않고 배 모델만 그린다(배 전용 분기, 다른 탈것은 바뀌지 않는다).
3. 바다 이동은 새 네비 영역 `BernSea`(별도 층)에서 기존 서버 경로/속도 판정으로 처리한다. 사용자 소유 Bern/Bern2/Bern3 영역 파일은 수정하지 않는다.
4. 우클릭 목표는 물이 반투명이라 화면 픽이 바다 밑을 맞히므로, 배를 탄 동안은 커서 ray와 **물 표면 평면**(배 뿌리 높이 + 흘수 1.5m)의 교점을 목표로 쓴다(카드미로 바닥 클릭과 같은 원리, 평면 높이만 다르다).
5. 하차하면 기억해 둔 부두 위치로 되돌린다. 사망 등 강제 하차도 같다.

## 4. 이번에 바꾼 것 (층별)

Data / Tools
- 새 파일 `Data/Navigation/LV_BER_BERNCASTLE.BernSea.navsource` (211,434바이트): 항구 앞 바다를 나타내는 평평한 네비 격자 원본.
- `Data/Navigation/LV_BER_BERNCASTLE.navregions`: 머리줄 영역 수 3 → 4, 마지막에 `REGION "BernSea" 1` 한 줄만 추가(원본은 `out/Bern3ShipSea20260925/backup/nav/`에 보관).
- 새 파일 `Tools/ShipPipeline/build_sea_nav.py`: 위 격자를 만드는 생성기(수면 높이·흘수·범위·여유 거리를 상수로 두었고 `--write`로만 파일을 쓴다).

Server (권위)
- `ServerPlayer.h`: 부두 위치를 기억하는 Server 전용 필드 5개(복제하지 않는다). 월드 이동 staging(`GameRoom_PlayerCommands.cpp`)에서 초기화 1줄.
- `GameRoom_VehicleRiding.cpp`: 배 탑승을 승인하면 `Begin_ShipVoyage`가 플레이어를 부두에서 가장 가까운 `BernSea` 열린 셀로 옮기고 부두 위치·방향을 기억한다. 하차, 다른 탈것으로 변경, 사망 등 강제 하차에서 `End_ShipVoyage`가 기억한 부두 위치로 되돌린다. Bern이 아닌 월드의 배 탑승은 거부, 100m 안에 열린 바다가 없으면 `REJECTED_PLAYER_STATE`로 거부(플레이어 위치 그대로).
- `ServerNavigation.h/.cpp`: 상세 영역이 자기 manifest id를 기억하고 `Is_PointWalkableInRegion(regionId, x, z, hintY)`로 "이 점이 그 이름의 영역 걷기 가능 셀인가"를 답한다. 배 출항 지점이 다른 영역의 비슷한 높이 땅을 바다로 착각하지 않게 하는 데 쓴다.
- `ServerGameplayContractTests_VehicleRiding.cpp`: 항구 부두에서 탑승, 바다 위 출항 지점, 바다 위 경로 탐색, 배 변경 시 위치 유지, 하차·사망 시 부두 복귀, 부두 밖 탑승 거부, 지상 탈것은 위치를 옮기지 않음을 검사한다.

Client (표현)
- `Character.h/.cpp`: 배를 탄 동안 `Is_ShipPresentation()`이 참이고 `Late_Update`가 배 파트만 그리도록 제출한다(몸·장비·무기는 제출하지 않음). 바뀔 때 `ship.rider.hidden` 로그를 남긴다.
- `WorldPlayerNameplateView.cpp`: 배 탄 플레이어의 이름표를 그리지 않는다.
- `PlayerController.cpp`: 배를 탄 동안 우클릭 목표는 물 표면 평면(뿌리 높이 + 1.5m)과 커서 ray의 교점이다(수면이 반투명이라 화면 pick은 바다 밑을 맞힌다). 클릭 표시도 수면에 찍힌다.
- 카메라(`CLevel_Bern::Update_ShipCamera`)와 배 창은 앞선 작업의 것을 그대로 쓴다.

게시(narrow publish)
- `Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_BER_BERNCASTLE` 한 번. 새 게시 파일은 Server 4개(`BernSea.navblockers/.navgrid/.navpolicy/.navsurface`)와 Client 3개(`.navblockers/.navgrid/.navpolicy`), 수정은 양쪽 `navregions` 두 개.

## 5. 바다 이동 영역을 어떻게 풀었나

서버의 이동은 네비 격자 위에서만 계산한다. Bern3 격자는 부두와 갑판 띠만 걷기 가능이고 바다 칸이 없어서, 그대로 두면 배를 바다로 보내도 서버가 이동을 받아 주지 않는다. 원작 바다 레벨(`LV_OCN_World_PS`)은 이 프로젝트에 없고 이번에도 추출하지 않았다. 그래서 같은 Area 안에 **바다 전용 평평한 상세 영역 `BernSea`**를 새로 만들었다. 기존 Bern/Bern2/Bern3 저작 파일은 읽기만 했고 수정하지 않았다.

만든 방식과 근거(전부 가정이며 상수를 고쳐 다시 만들 수 있다):
- 수면 높이 10.8m: 항구 정박 배(COMMON_SHIP01)가 y 10.78~10.97, 넓은 물 평면 두 장이 y 10.71/10.92에 있다(배치 실측).
- 흘수 1.5m: 정적 ESTOCSHIP01이 y 9.09에 서 있고 수면이 약 10.8이다. 배 뿌리(선체 원점)는 캐릭터 발 위치이므로 네비 높이는 10.8 − 1.5 = 9.3m로 했다.
- 범위: x 192~340, z −262~−156, 칸 1m. 기존 세 영역의 걷기 셀에서 6m, 정적 배(ESTOCSHIP/COMMON_SHIP01·04)에서 9m, 바위·절벽·부두·계류 시설류에서 5m를 비웠고 가장자리 2m를 막았다. 열린 조각이 5개 나왔고 가장 큰 4방향 연결 조각(9,802칸, 9,802㎡)만 걷기 가능으로 썼다. 정적 배제 대상은 117개였다.
- 검증: 게시 검증이 통과했고(`BernSea 148x106, 걷기 9802, 단차 0`), 영역이 기존 영역과 서로 다른 높이 층으로 겹치지 않는다는 로드 검사도 통과했다.
- 세 NPC 자리에서 가장 가까운 열린 바다 칸: 조선공 A 16.5m, 조선공 B 10.1m, 항만 관리인 12.5m. 항만 관리인 자리에서 실제 출항하면 서버가 (267.45, 9.3, −217.70)을 골랐고 14m 떨어져 있다(테스트 로그).

서버가 출항 지점을 고르는 규칙: 플레이어 위치에서 4m 반경부터 2m씩 넓히며 100m까지 원을 돌아 `BernSea` 영역의 걷기 가능 셀 중 가장 가까운 것. 이 규칙은 원작 좌표 근거가 없는 **가정**이다.

## 6. 게시와 백업, 바이트 동일 증명

- 게시 전 백업: `out/Bern3ShipSea20260925/backup/`(Server 17개·Client 13개 Bern 네비 게시 파일 전부, 저작 `navregions` 원본 포함).
- 먼저 임시 폴더(`out/Bern3ShipSea20260925/stage`)에 같은 게시를 해서 비교했다. 기존 Bern 게시 파일은 Server 16개, Client 12개가 **바이트 동일**했고 다른 것은 `navregions` 하나뿐이었다(예상된 변경).
- 실제 게시 뒤 게시 파일 37개가 임시 폴더 결과와 모두 같았고, 백업 대비 바뀐 기존 파일은 양쪽 `navregions` 둘뿐이었다.
- `git status` 전후 비교에서 `Server/Bin/DataFiles`·`Client/Bin/DataFiles` 아래 달라진 것은 `LV_BER_BERNCASTLE.navregions` 수정 2개와 BernSea 새 파일 7개뿐이다. 다른 Area, World, Map 게시본은 건드리지 않았다.

## 7. 진단 로그 문구

Server 콘솔(`Server.exe`):
- `[ShipBoard] player=<id> ship=<id> pier=(x, y, z) sea=(x, y, z) yaw=<도> distance=<m>`: 탑승 승인과 바다로 옮긴 위치.
- `[ShipBoard] player=<id> ship=<id> refused: no open sea within 100 m of (x, z)`: 출항 거부.
- `[ShipBoard] player=<id> back to pier=(x, y, z) from sea=(x, z) reason=left the ship | changed to another vehicle | forced dismount`: 부두 복귀.

Client(`Client/Default/EffectFailure.user.log`):
- `ship.rider.hidden`: `local boarded ship <id>: the rider body, gear and nameplate are hidden, only the ship is drawn` / `local left the ship: the rider is drawn again` (원격 플레이어는 `remote`).
- 앞선 작업의 `ship.window.open`, `ship.npc.ready`도 그대로다.

## 8. Debug에서 직접 확인하는 순서 (화면 판정은 사용자만)

1. 이번에 빌드한 Debug Server와 Client를 **둘 다 다시 시작**한다(게시한 `BernSea` 격자는 Server 시작 때 읽힌다).
2. Lobby → Bern → Bern3 부두로 걸어가 조선공 또는 항만 관리인을 우클릭 → 배 선택 창 → 배 한 척 탑승.
3. 예상 결과: 캐릭터 몸과 이름표가 사라지고 배만 부두 앞 바다 위(대략 10~17m 떨어진 곳)에 나타난다. 카메라는 배 시점(FOV 60, 17m)이다.
4. 바다 위를 우클릭하면 그 지점으로 배가 이동한다(1.8~2.2 m/s). 클릭 표시가 수면에 찍힌다. 바다 영역 밖(육지·부두·바위 쪽)을 눌렀을 때 서버가 어떻게 처리하는지(그 자리에 멈춤인지 가장 가까운 점까지인지)는 이번에 화면으로 확인하지 못했다.
5. 탈것 창의 하차 또는 H로 내리면 캐릭터가 다시 보이고 탑승 직전의 부두 위치로 돌아온다.
6. 어긋나면 알려 줄 것: 배가 수면에 잠긴 정도(흘수 1.5m 가정), 배가 나타나는 위치, 이동 가능한 바다 범위(부두 옆 여유 6m 가정), 카메라 거리.

## 9. 빌드와 테스트 결과

- Debug Product 빌드(Engine → Shared → Server → Client): 두 번 모두 PASS. 첫 실행 `out/BuildPipeline/runs/20260925T063126720Z-debug-product.json`(Client OBJ 67개 재컴파일), 서버 보정 뒤 `20260925T063554329Z-debug-product.json`. `missingRuntimeInputs`, `invalidRuntimeInputs`는 둘 다 비어 있다. Client.exe 15:31, Server.exe는 보정 뒤 빌드본이다.
- `Server.exe --vehicle-riding-contract-test`: **77 PASS, 실패 0**. 처음 실행은 1개 실패였다. 도시 시작 지점에서 탑승을 시도하니 서버가 (150.3, 9.28, −35.2)의 Bern 네비 땅(`BernSea` 범위 x 192~340 밖, 도시 시작 지점에서 18m)을 "바다"로 받아들였다(거부됐어야 하는 경우). 원인은 높이만 보고 영역을 확인하지 않은 내 코드였고, `Is_PointWalkableInRegion`을 추가해 `BernSea`만 인정하고 탐색 반경을 140m에서 100m로 줄여 고쳤다. 고친 뒤 재실행에서 통과했고 도시 시작 지점 탑승은 `refused: no open sea within 100 m`로 거부된다.
- `Server.exe --navigation-contract-test`: 34 PASS, 실패 0.
- `Server.exe --debug-teleport-contract-test`: 8,111 PASS, 실패 0(17분 소요, 종료 코드 0).
- `Server.exe --contract-test`: 28분 제한 안에 끝나지 않았다(종료 코드 124, 350 PASS 후 중단). 그 사이 **실패 22건**이 나왔다: 마리오 입장 패턴(chain-free entry Parent, 마리오 stage 진입) 관련 14건(같은 두 항목이 반복 실행되며 실패), KoukuSaydon 관문 보스 Debug 소환·despawn 관련 7건, Ghost Death 1건. 이 실패가 이번 변경 때문인지 확인하려고 **배 탑승 서버 변경만 되돌린 기준 Server**를 별도 폴더(`out/BaselineServer20260925`)에 빌드해 같은 테스트를 같은 게시 데이터로 돌렸고, 실패 22건이 이름까지 똑같이 나왔다(기준 실행은 Ghost Death 항목까지 확인한 뒤 내가 시작한 프로세스만 종료했다). 따라서 이 실패는 이번 배 변경과 무관하게 현재 작업 트리(다른 작업의 미커밋 변경 포함)에 이미 있던 것이다. 원인 조사와 수정은 이 작업 범위가 아니라서 하지 않았다. 나머지 테스트는 끝까지 돌리지 못했다.
- 실행하지 않은 것: Client(에이전트는 실행하지 않는다), 화면·소리·카메라 판정, 다른 Area 게시, Release 패키지 갱신, `NetworkProtocolHarness`(protocol이 바뀌지 않았다).

## 10. protocol과 리소스 영향

- protocol은 105 그대로다(Shared 수정 없음). 새 필드는 Server 전용이라 snapshot에 실리지 않는다. main이 protocol 110이지만 이 작업은 번호를 건드리지 않았다.
- Client 리소스(모델·텍스처)는 추가하지 않았다. 팀원 전달이 필요한 새 물리 리소스는 없다.
- Server와 Client를 함께 다시 빌드하고 다시 시작해야 한다. 바탕화면 Release 폴더는 이 변경을 반영하지 않은 상태다.

## 11. 알 수 없는 것과 가정 (다시 정리)

- 원작에서 배가 나타나는 바다 좌표, 페이드·소리·카메라 전환, 이름표 처리, 선원 표시는 확인하지 못했다.
- `BernSea`의 위치·크기·수면 높이·흘수·여유 거리는 배치 실측을 근거로 한 가정이다. 화면에서 어긋나면 `build_sea_nav.py`의 상수를 고치고 `--write` 후 같은 게시 명령을 다시 실행한다.
- 출항 지점을 "부두에서 가장 가까운 열린 바다 셀"로 정한 것은 가정이다.
- 배 표시 높이(`seatOffset`)와 앞방향(`modelYawDegrees`)은 앞선 작업의 추정값이며 이번에도 화면에서 확인되지 않았다.
- 바다 영역은 정적 배·바위·부두 배치만 피한다. 이후 배치가 늘면 다시 만들어야 한다.

## 12. 함께 갱신한 문서

`CLAUDE.md`의 탈것 절(배 탑승 흐름), `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 배 절, `.md/GB/gotchas.md`의 새 항목, `.md/GB/09-25/2026-09-25_BERN3_SHIP_SAILING_RESULT.md`의 13절.

## 13. 최종 재확인 (완료 보고 직전)

- 다시 확인한 것: 게시 파일 37개가 임시 폴더 결과와 같음, 기존 Bern 네비 게시 파일 바이트 동일(navregions 제외), Debug Product 빌드 PASS 두 번, `--vehicle-riding-contract-test` 77 PASS 0 실패(고친 뒤 빌드본), `--navigation-contract-test` 34 PASS, `--debug-teleport-contract-test` 8,111 PASS.
- Client.exe(15:31)와 Server.exe(15:35)는 수정한 모든 소스(가장 늦은 15:34)보다 나중이고, 두 실행 파일에서 새 로그 문구(`ship.rider.hidden`, `[ShipBoard] player=`, `refused: no open sea within`)를 찾았다.
- 확인하지 못한 것: 실제 화면(배 위치·높이·캐릭터 숨김·이름표·카메라·바다 클릭 이동), 바다 영역 밖을 눌렀을 때의 동작, `--contract-test` 완주. Client와 UI는 실행하지 않았다.
- 아직 커밋하지 않았다. 바탕화면 Release 폴더도 갱신하지 않았다.
