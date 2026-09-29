# 2026-09-30 배(항해) HUD와 워터팡 물총 HUD RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 VS에서 마지막에 한 번 빌드). 화면 판정은 사용자 몫이다.
C++ 파일(`MainApp.cpp/.h`)이 바뀌었으므로 **재빌드가 필요하다.** 게시(Publish)는 필요 없다.

## 1. 우리 HUD가 만들어지는 방식 (코드로 확인)

- 정본은 `Data/UI/HUD/HUD_Layout.json`(`lostark.ui-layout` v1, reference 1280x720)이다. `slot.id`, `ownerClass`,
  `rect`, `layers[].path`(Resources 상대 `UI/...`)를 저장하고 배열 순서가 그리기 순서다.
- `CHUDRuntimeView`가 문서를 한 번 읽어 매 프레임 슬롯을 그린다. `Set_ActiveOwnerClass`가 소유 클래스가 다른
  슬롯을 숨기고, `Set_SlotVisible / Set_SlotTexture / Set_SlotArcRatio / Set_SlotTint`로 값을 덮는다.
- `CMainApp::Update_CombatHUD`가 매 프레임 모든 슬롯을 켠 뒤 클래스 소유 필터를 적용하고, 그 뒤
  `Update_KoukuHudMode → Update_VehicleHud → (새) Update_WaterGunHud → Update_SpecialSlot` 순서로 덮어쓴다.
- 값은 `CCombatHUDViewModel`(서버 복제값)에서만 읽는다. 쿨다운 파이는 `Resolve_HudCooldownRatio`가 복제된
  쿨다운 목록에서 계산한다. HP/마나 숫자와 키 글자는 슬롯이 아니라 C++이 rect를 읽어 따로 그린다.
- 탑승 HUD는 이미 있었다: `VehicleRiding` 클래스, 안장 엠블럼(`Vehicle_Hud_Emblem`), Q/W/E에 탈것 스킬 아이콘,
  A/S/D/F는 잠금 아이콘. 이번에 배(ship)일 때만 원본 항해 HUD 틀로 바꿨다.

## 2. 원본 근거

### 2-1. 배 HUD = `oceanhud` 무비

- 무비: `oceanhud` (EFSwfMovie, 패키지 `OVSG0AMAOWF9SHDR2YW8WM.upk`, 논리 패키지 `EFUI_OCEANHUD`), 스테이지 1920x1080.
  UModel은 텍스처 4장(`oceanhud_i1/i12/i23/i26`)만 내보내고 무비는 내보내지 못해서, 패키지 export의 165바이트
  오프셋에서 시작하는 `GFX` 바이트(길이 722,222)를 직접 꺼내 태그를 파싱했다(ffdec/Java 없음).
- 루트 인스턴스: `oceanRudderFrame`(sprite 1031)와 `oceanHudFrame`(sprite 974).
- 사용한 조각(무비가 이름 붙인 sub-image 사각형 그대로):

| 조각 | 무비 위치 | 아틀라스 `oceanHud_I1` | 스테이지 배치 |
|---|---|---|---|
| 선체 프레임 | OceanRudderFrame 첫 도형 shape 1024 | 0,230 - 712,349 (712x119) | (652, 956) |
| 조타륜 | BoatRudderComponent → BoatRudder → sprite 1018 → shape 1017 | 247,0 - 433,165 (186x165) | 중심 (960, 975), 좌상단 (867, 892.5) |

- 같은 movie의 나머지 구성(사용하지 않음): 보급/내구도 구(`OceanSupplieGauge`, 텍스트 `targetText` '3850/3900'),
  전투 슬롯 8개(`OceanHudSkillSlotList` slot0-7, Q W E R / A S D F), 부스터·닻·뱃고동·자동항로 버튼,
  버프 목록. 조타륜 `BoatRudder`는 158프레임(조타각).
- 원본 메시지 키 `sys.voyage.hud_speed_indicator` = `<font size='16' color='#e1d29d'>{0} 노트</font>`,
  `sys.voyage.hud_shipdurability_desc`(선박 내구도)가 둥근 게이지의 정체다.

### 2-2. 물총 HUD = `quickslot` 무비의 소품 집기 모드

- `quickslot`(EFSwfMovie, 패키지 `PWTH1B17THBVF2NME7646CM.upk`)에 `INTERACTION_TYPE_PICKUP_PROP`,
  `ST_Hud_PickUpHudShow`, 엠블럼 `quickSlot_fla.interaction_pickUp_00_68`(sprite 1556)이 있다. 엠블럼은
  `QuickSlot_I50.tga` 782,454 - 940,607(158x153) 한 장이다. 탈것 모드 엠블럼(`interaction_vehicle_00_67`,
  sprite 1553, 161x161)과 같은 구조다.
- 이 소품 집기 모드가 쿠크 뿅망치의 HUD와 같은 모드라서, 프로젝트에 이미 있는 `Kouku_Emblem_Pickup`
  (`UI/KoukuSaydon/Hud/emblem_interaction_pickup_gear.png`, 158x153으로 위 크롭과 크기가 일치)과
  쿠크 상호작용 레이아웃(Q..F 슬롯, T/V 없음)을 그대로 재사용했다. 새 레이아웃을 만들지 않았다.
- 물총 스킬 아이콘: `EFTable_Skill` 56900/56910/56920/56930의 Icon/IconIndex =
  `Vehicle_212 / Vehicle_210 / Vehicle_211 / Vehicle_3` → `IconInfo.loa` → `EFUI_ICONATLAS_V` 페이지
  `Vehicle_0`(896,64 / 768,64 / 832,64) · `Vehicle_1`(192,0), 64x64. 이미지로 확인한 모양: 물줄기 분사(Q),
  물 폭탄(W), 달리는 사람(E 이속), 푸른 폭발(R).
- 스크린샷 검색: 원본 클라이언트 스크린샷 폴더의 최근 사진은 베른 일반 HUD와 월드맵뿐이라 물총 HUD 화면은
  찾지 못했다. 그래서 모드 구조(무비)와 기존 쿠크 HUD 재사용으로 확정했고, 화면 1:1 대조는 하지 않았다.

## 3. 연결한 것

### 배 HUD (Update_VehicleHud 확장)

- 탑승한 탈것이 배(`VEHICLE_ACTOR_ENTRY::isShip`)일 때만:
  - `Ship_Hud_Plate`(선체 프레임)와 `Ship_Hud_Wheel`(조타륜)을 켜고 안장 엠블럼을 끈다. 두 슬롯은
    `HUD_Layout.json` 맨 앞에 넣어(소유 `VehicleRiding`) 퀵슬롯 아래에 그려진다. 좌표는
    `build_quickslot_hud_ui.py`와 같은 변환(스테이지 960↔HUD 673.5, 974↔644.702, 2/3)이라 조타륜 중심이
    안장 엠블럼 중심과 같고 프레임은 Q 슬롯 열에서 시작한다.
  - 원본 HUD에 없는 HP/마나 바, 아이템 행, 클래스 특수 슬롯, 차지 게이지 슬롯을 숨기고
    (`SHIP_HIDDEN_SLOTS` 30개) HP/마나 숫자와 그 키 글자(1-4, 5-0)도 `m_bShipHudActive`로 함께 막는다.
  - Q/W/E 탈것 스킬 아이콘·쿨다운 파이, Space 부스트 슬롯, 버프 목록은 기존 그대로다.
- 다른 탈것(말 등)은 안장 엠블럼과 기존 모습 그대로다.

### 물총 HUD (Update_WaterGunHud 신규)

- 조건: 마하라카 레벨 + 서버가 무장 상태로 복제(`isWaterpangArmed`) + 도보. 마하라카는 원래 HUD가 없던 레벨이라
  `Is_WaterGunHudLevel`로 그 조건에서만 전투 HUD를 열었다(Update_CombatHUD, HP/마나 글자,
  쿨다운 글자, 키 글자의 레벨 조건 4곳). 무장이 아닐 때 마하라카는 예전처럼 HUD가 없다.
- 표시: 클래스 정체성 블록과 T/V를 숨기고 소품 집기 엠블럼을 켠다. Q/W/E/R이 물총 스킬 아이콘,
  쿨다운 파이는 서버가 스킬 시전 때 채우는 복제 쿨다운 목록(`Cooldowns`)에서 읽고
  (Shared 표 `MAHARAKA_WATERGUN_SKILLS`의 쿨다운 3000/8000/7000ms, R은 없음), A/S/D/F는 빈 슬롯이다.
  클래스 스킬 아이콘이 물총 스킬 자리에 보이던 문제가 이 HUD로 대체된다.
- 값은 전부 서버 복제값이다. 클라이언트가 쿨다운이나 판정을 만들지 않는다.

## 4. 연결하지 못한 것 (사유)

- 배 내구도 구와 '3850/3900' 숫자, 위험 해역 게이지: 서버에 선박 내구도·보급 값이 없어 그리지 않았다.
- `0.0 노트` 속도 표시: 서버는 `SNAPSHOT_PLAYER.fMoveSpeed`(m/s)를 복제하지만, 원본 문구
  `sys.voyage.hud_speed_indicator`의 `{0}` 값 공식은 어떤 무비에도 없고(전체 무비 510개 패키지의 바이트에서
  키를 찾지 못했다) 실행 파일 네이티브 코드가 채운다. 공식을 확인하지 못해 환산해 표시하지 않았다.
  `EFTable_VoyageShip`의 `MoveSpeed`(예: 8200 = 200)에도 노트 환산 열이 없다.
- 닻(Z)·자동항로(M)·뱃고동(T)·부스터 버튼 미술과 조타륜 회전(158프레임): 프로젝트에 해당 동작이 없어 그리지 않았다.
- 배 HUD 전용 전투 슬롯 프레임: 원본은 파란 닻 원형 프레임이지만 기존 퀵슬롯 프레임을 그대로 썼다.
- RenderDamageNumbers: 마하라카에서 데미지 숫자를 열지 않았다(요청 범위 밖).

## 5. 바꾼 파일

- `Tools/LpkPipeline/build_voyage_hud_ui.py`(신규): 이미지 크롭 + 레이아웃 삽입(멱등, 원본 무비와 대조 후 저장).
- `Tools/LpkPipeline/dump_upk_movie.py`, `gfx_native_parse.py`, `gfx_native_tree.py`(신규): ffdec 없이 GFX를 읽는 도구.
- `Data/UI/HUD/HUD_Layout.json`: 맨 앞에 `Ship_Hud_Plate`, `Ship_Hud_Wheel` 2개 추가(다른 변경 없음).
- `Client/Bin/Resources/UI/HUD/Voyage/{ship_plate,ship_wheel}.png`, `UI/HUD/Waterpang/skill_{56900,56910,56920,56930}.png`
  (Git 미추적 Resources; `CY_Resources`에 같은 상대 경로로 복사했다. 복사 스크립트는 만들지 않았다).
- `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`: 선언 2개(`Update_WaterGunHud`, `m_bShipHudActive`),
  `Is_WaterGunHudLevel`, `SHIP_HIDDEN_SLOTS`, `Update_VehicleHud` 확장, `Update_WaterGunHud`, 레벨 조건 4곳,
  키 글자 조건 2곳. 인코딩(UTF-8, CRLF)을 유지했고 앵커가 각각 정확히 1번 매칭된 경우에만 적용했다.
- `.md/GB/gotchas.md`: 재발 방지 4항목.
- 추출 입력 보관: `C:\LostArkExtract\VoyageHud20260930`(oceanhud 아틀라스·파싱 결과, 아이콘 페이지).

## 6. 실행한 검증 (실제로 돌린 것만)

- 빌더 실행: 6개 PNG 저장, 레이아웃 2슬롯 삽입, 원본 무비 대조(shape 1024/1017의 sub-image 사각형 일치).
- `HUD_Layout.json` JSON parse, slot.id 중복 0, 새 슬롯 이미지 경로 실재, 물총 엠블럼 파일 실재(158x153).
- `MainApp.cpp` 구문 검사(`cl /Zs`, 출력물 없음): 오류 0, 기존 파일의 C4819 인코딩 경고만 있다.
  이것은 Product 빌드가 아니다.
- 줄 끝 확인(CRLF 유지), `git diff --check` 문제 없음.
- 실행하지 않은 것: 빌드, Client/Server 실행, 화면 확인.

## 7. 사용자 확인

- 재빌드 후 Server·Client 재시작(프로토콜은 바뀌지 않음).
- 배: 베른에서 H로 배에 타면 하단 중앙에 선체 프레임과 조타륜이 나오고, HP/마나 바·아이템 행이 사라지는지,
  Q/W/E 탈것 스킬과 Space 부스트 쿨다운은 그대로인지. 프레임 위치가 Q 슬롯 열과 맞는지.
- 물총: 워터팡 경기에서 물총을 들면 HUD가 나타나고 소품 집기 엠블럼, Q/W/E/R 물총 아이콘이 보이는지,
  쏠 때 쿨다운 파이와 초가 도는지(Q 3초, W 8초, E 7초), 경기 밖에서는 HUD가 없는지.
- 화면이 원본과 다르면 조정 후보: 프레임 위치(`build_voyage_hud_ui.py`의 스테이지 좌표), 숨김 목록(`SHIP_HIDDEN_SLOTS`).
