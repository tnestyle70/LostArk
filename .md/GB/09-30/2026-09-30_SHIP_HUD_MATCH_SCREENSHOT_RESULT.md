# 2026-09-30 배(항해) HUD를 원본 스크린샷과 같게 다시 만들기 RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 VS에서 마지막에 한 번 빌드). 화면 판정은 사용자 몫이다.
C++ 파일이 바뀌었으므로 **재빌드가 필요하다.** 게시(Publish)는 필요 없다(HUD 문서와 카탈로그는 Data 정본을 직접 읽는다).

## 1. 사진 분석 (사실)

사용자가 준 사진 두 장은 **같은 배 HUD의 두 화면**이다.

- `스크린샷 2026-09-30 030649.png` (702x225 크롭): **원본 클라이언트** 1920x1080의 HUD 크롭. 크롭 원점은 stage (568, 852)다
  (돔 중심 stage x 960이 크롭 x 392, 선체 하단 stage 1075가 크롭 y 223에 놓이는 것으로 확정).
- `스크린샷 2026-09-30 030655.png`, `030807.png`: **우리 프레임워크** 화면(이전 결과물).

원본 HUD의 요소(크롭 좌표 → stage px):

| 요소 | 사진에서 본 것 | 원본 무비 근거 |
|---|---|---|
| 조타륜 + 돔 | 조타륜 한가운데 파란 물이 찬 유리 돔, 안에 흰 글씨 `2275/3500` | OceanRudderFrame: 조타륜 1017, 테두리 1026, 돔 바닥 980(어두운 구), 물 984(파란 구), 글자 targetText |
| 선박 아이콘 + 속도 | 돔 아래 배 아이콘, 그 아래 `22.3 노트`(베이지) | suppliesIcon_mc 1028, 텍스트 필드(값은 네이티브가 채움) |
| 슬롯 8개 | Q W E R / A S D F. 프레임은 어두운 사각형+가는 테두리+아래 노치, Q W E는 아이콘 | skillSlotList slot0-7, 프레임 386 + 탭 860 |
| 우측 버튼 | T(나침반 포트홀), Z(파란 링 닻), SPACE(톱니바퀴+노란 링+화살표), C(뿔피리/굴뚝+파이프), 빈 사각 슬롯 | cruise_btn(64+92), boatParking_btn(49+318), boatBooster(294 톱니, 225 어두운 링, 227 노란 링, 134 화살표), boatHorn_btn(369), eventProgress(363 병, 325 슬롯) |
| 오른쪽 끝 | 'M 자동 항로'(초록 지구본) | autoCruiseBtn 67 |
| HP/마나 바, 아이템 행 | **없음** | 이전 결과가 이미 숨김 |

**두 화면의 차이**: 우리 결과에는 조타륜 테두리+선체 틀+자물쇠 슬롯 8개만 있었다. 돔 게이지와 숫자, 속도, 슬롯 프레임 모양,
우측 T/Z/SPACE/C 버튼, 병, 톱니, 뿔피리 쪽 장식이 통째로 빠져 있었다.

### 정정: 돔은 '내구도'가 아니라 '보급(supply)'이다

- 이전 결과 문서는 돔을 선박 내구도라고 적었다. 틀렸다. 원본 무비 이름은 `OceanSupplieGauge`(보급 게이지)이고,
  숫자의 분모는 `EFTable_VoyageShip.MaxSupply`다: 8200 레벨1 = **3500**(사진 `2275/3500`), 8203 = **3900**(이전 스크린샷 `3850/3900`).
- 다른 배의 MaxSupply(레벨1): 8201 3700 / 8202 3100 / 8204 4200 / 8205 3600 / 8206 3700 / 8207 3100 / 8208 3150.

## 2. "배 종류마다 HUD가 달라지나?"에 대한 근거

- **틀(프레임/조타륜/버튼)은 배마다 같다.** oceanhud 무비는 하나이고, 무비 안의 모든 프레임 라벨(disabled/up/over/down/out/selected_*/normal/
  hide/show/progress/… 25종)에 배 종류를 가리키는 것이 없다. `EFTable_VoyageShip` 컬럼에도 HUD·UI·조타륜·아이콘을 가리키는 열이 없다.
- **값은 배마다 다르다**: 돔 분모(MaxSupply), Q/W/E 탈것 스킬, 속도(MoveSpeed 180~266).
- 조타륜 `BoatRudder`는 158프레임(조타각)이라 배의 회전에 따라 돌아가는 애니메이션이지 배 종류별 변형이 아니다.

## 3. 만든 것

빌더 `Tools/LpkPipeline/build_ocean_hud_ui.py`(신규). 각 조각은 파싱된 무비의 sub-image 사각형과 대조한 뒤에만 저장한다.

- 리소스 55개(`Client/Bin/Resources/UI/HUD/Voyage/`, `CY_Resources` 같은 경로에 바이트 동일 복사; 복사 스크립트는 만들지 않음):
  테두리, 돔 41프레임(0~100%, 2.5% 간격 — 어두운 구 위에 파란 구를 수면 높이만큼 잘라 얹고 수면 광채를 얹음), 선박 아이콘, 병, 이벤트 슬롯,
  슬롯 프레임(+노치, 얇은 테두리), T/Z/SPACE/C/M 버튼 조각, 톱니, 노란/어두운 링(반쪽 두 개를 거울로 합쳐 원으로).
- `Data/UI/HUD/HUD_Layout.json`: 슬롯 39개를 `Ship_Hud_Wheel` 바로 뒤에 삽입(문서 순서 = 그리기 순서 = 원본 무비 depth 순서).
  문서는 `json.dumps(indent=2)`와 바이트 단위로 왕복하는 것을 확인한 뒤 통째로 다시 썼고 CRLF를 유지했다.
- **슬롯 위치는 사진을 정답으로 삼았다.** 무비의 frame 1은 슬롯 간격이 47px인데, 원본 클라이언트 사진에서 잰 값은 간격 44.3px,
  Q x=699.0 / A x=721.3, 행 y=972.8 / 1018.8이다(실행 시 스크립트가 다시 배치하는 것으로 보임). 나머지 조각(조타륜, 돔, 버튼)은 무비 좌표와 사진이 3px 안에서 일치한다.
- `Data/Actors/VehicleCatalog.json`: 배 9종에 `"maxSupply"`를 삽입(`Tools/ShipPipeline/add_ship_max_supply.py`, EFTable_VoyageShip 레벨1 값, 텍스트 삽입).
- C++ (모두 UTF-8/ASCII + CRLF 유지, 앵커가 정확히 1번 매칭될 때만 적용):
  - `ActorCatalog.h/.cpp`: `VEHICLE_ACTOR_ENTRY::iMaxSupply`, 선택 필드 `maxSupply`(1~100000 정수) 파싱.
  - `CombatHUDViewModel.h/.cpp`: `HUD_PLAYER_STATE::fMoveSpeed`, `hasMoveGoal`(서버가 이미 복제하는 값).
  - `MainApp.h/.cpp`: 배 탑승 중 `Ship_*` 슬롯 41개를 켜고(다른 탈것은 끔), 클래스 퀵슬롯(Skill_Q~F 계열, TypeMark/Chain)과
    Special_Space를 숨기고, Q/W/E 탈것 스킬 아이콘과 쿨다운 파이를 `Ship_Slot_*`로 옮기고, SPACE 부스트 링을 쿨다운에 따라 시계방향으로 채운다.
    돔 프레임을 보급 비율로 고르고, `RenderShipHudTexts`가 돔 숫자, 노트, 슬롯 글자, 버튼 글자(T/Z/SPACE/C/M/자동 항로)를 그린다.

## 4. 서버 값이 없는 것 — 처리 방침과 근거

| 항목 | 처리 | 근거 |
|---|---|---|
| 돔 숫자 `현재/최대` | **최대 = MaxSupply(카탈로그), 현재 = 최대(가득)** | 프로젝트에 보급 소모 시스템이 없다. 서버에 없는 값을 지어내지 않고 '소모 없음 = 가득'으로 표시. 소모를 넣으려면 서버 복제 값과 protocol 변경이 필요하다(하지 않음) |
| 노트 속도 | `이동 목표가 있을 때 fMoveSpeed x 5.575`, 없으면 0.0 | **추정.** 사진 표본 1개(8200, 22.3노트, MoveSpeed 200)를 기준으로 한다. 이 프로젝트의 배는 원본 속도의 x2로 튜닝돼 있어(8200 = 4.0 m/s) 22.3 / 4.0 = 5.575. 사진의 배가 부스트 중이었는지 알 수 없어 확정하지 않는다. 원본 문구 키의 계산식은 어떤 무비에도 없다 |
| 버튼(T 나침반, Z 닻, C 뿔피리, M 자동 항로) | **그림과 글자만.** 눌림은 연결하지 않음 | 해당 동작이 프로젝트에 없다. 표시 전용 HUD는 hit test none 원칙 |
| SPACE 부스트 링 | 기존 SPACE 부스트 스킬의 쿨다운으로 시계방향 채움 | 원본은 지속시간에 따라 반쪽 링 두 개를 회전 마스크로 채운다. 파이 마스크로 근사 |

## 5. 원본과 정확히 같지 않은 부분 (숨김 없이)

- 병(eventProgress)은 몸통 그림만 그린다. 진행 게이지와 빛나는 상태(365 계열)는 그리지 않았다.
- 조타륜의 158프레임 회전(조타각)은 그리지 않았다. 정지 그림이다.
- 돔 수면의 물결 애니메이션(80프레임)은 그리지 않았다. 수면 광채는 정지 그림이다.
- 사진의 S 슬롯 노란 아이콘(원본에서 장착한 것)은 우리 프로젝트에 대응 동작이 없어 빈 슬롯이다.
- 버프/디버프 목록, 낚시 미끼 슬롯, 특수 슬롯(ultraSlot)은 무비에 있지만 사진에 없고 프로젝트에도 없어 그리지 않았다.
- 슬롯 글자와 버튼 글자의 위치와 크기는 사진에서 잰 값이다(±2px). 폰트는 무비 폰트 `YG760`이지만 크기는 사진 대조가 아니라 추정이다.
- 슬롯 프레임의 테두리 밝기는 사진 확대 화면에서 눈으로 맞춘 근사다.
- 사진은 1920x1080에서 찍었다. 우리 HUD는 1280x720 기준 좌표로 투영하며, 조타륜 중심을 안장 엠블럼 중심(HUD x 673.5)에 맞추는
  기존 규칙을 유지했다. 그래서 화면 정중앙이 아니라 약 33 HUD px 오른쪽에 놓인다(원본은 정중앙).

## 6. 실행한 검증 (실제로 돌린 것만)

- 빌더 실행: PNG 55개 저장, 무비 sub-image 사각형과 대조(불일치 시 중단), 레이아웃 39슬롯 삽입.
- `HUD_Layout.json`: JSON parse, slot.id 292개 중복 0, `Ship_*` 41슬롯의 이미지 참조 65개 전부 존재, CY_Resources 복사본 바이트 동일, CRLF 유지(bare LF 0).
- `VehicleCatalog.json`: JSON parse, `maxSupply` 9개.
- 실제 레이아웃 JSON과 PNG로 HUD를 합성해 원본 사진과 나란히 비교했다(`C:\LostArkExtract\VoyageHud20260930\analysis\compare_final.png`).
  조타륜, 돔, 아이콘, 슬롯 위치, T/Z/SPACE, 병, 뿔피리 위치가 사진과 겹친다. 이것은 합성 이미지이며 게임 화면이 아니다.
- 구문 검사(`cl /Zs`, UNICODE, `/utf-8`, 출력물 없음): `MainApp.cpp`, `ActorCatalog.cpp`, `CombatHUDViewModel.cpp`, `ClientReplication.cpp` 오류 0.
  Product 빌드가 아니다.
- 줄 끝: 패치한 C++ 6개 파일 모두 CRLF 유지, BOM 없음.
- `git diff --check` 통과.
- 실행하지 않은 것: Product 빌드, Client/Server 실행, 화면 확인.

## 7. 사용자 확인

1. 재빌드 후 Server와 Client를 재시작한다. 프로토콜과 서버 코드는 바뀌지 않았다.
2. 베른에서 H로 배(8200~8208)에 탄다. 하단에 원본 사진과 같은 HUD가 나오는지: 돔의 파란 물, 흰 숫자(`3500/3500` 등 배별 최대치), 배 아이콘, `0.0 노트`, 슬롯 8칸(Q W E에 탈것 스킬 아이콘), T/Z/SPACE/C/M.
3. 이동하면 노트가 오르는지(추정 환산). SPACE로 부스트를 쓰면 링이 비었다가 다시 차는지. Q/W/E 쿨다운 파이와 초 표시.
4. 말 같은 다른 탈것은 예전 모습 그대로인지, 마하라카 물총 HUD가 그대로인지.
5. 위치나 크기가 사진과 어긋나면 조정 후보: `build_ocean_hud_ui.py`의 `SLOT_POS`/`SLOT_PITCH`/`BOOST_CENTER`, `MainApp.cpp`의 `SHIP_KNOTS_PER_METER_PER_SECOND`와 `RenderShipHudTexts`의 글자 좌표.
   빌더를 다시 돌리면 `Ship_*` 슬롯만 교체되고 다른 슬롯은 그대로다.

## 8. 바꾼 파일

- 새 파일: `Tools/LpkPipeline/build_ocean_hud_ui.py`, `Tools/ShipPipeline/add_ship_max_supply.py`
- 데이터: `Data/UI/HUD/HUD_Layout.json`(슬롯 39개), `Data/Actors/VehicleCatalog.json`(`maxSupply` 9개)
- C++: `Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp`, `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp`,
  `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`
- 리소스(Git 미추적): `Client/Bin/Resources/UI/HUD/Voyage/`(+`Dome/` 41프레임), `CY_Resources` 같은 경로
- 커밋은 하지 않았다.
