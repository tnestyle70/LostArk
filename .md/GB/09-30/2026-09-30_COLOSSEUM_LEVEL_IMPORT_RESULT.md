# 증명의 전장(콜로세움) 레벨 도입 RESULT

작성일 2026-09-30. 원본 `lv_pvp_colosseum`(zone 30201) → AreaId `LV_PVP_COLOSSEUM`, World `COLOSSEUM`, `LEVEL::COLOSSEUM`.
범위는 "Debug 로비에서 들어가 걸어 다니기"까지다. 3v3, 카운트다운, 점수, 승패는 만들지 않았다.

## 구현 완료
- 추출·쿠킹·설치: 에셋 103종, 배치 1,294개. 런타임 리소스는 `Client/Bin/Resources/Map/LV_PVP_COLOSSEUM/`(Git 비추적, 205MB).
- 데이터: `Data/Maps/Imported|Authoring/LV_PVP_COLOSSEUM/`, `MapCatalog.json` 행, `Data/Navigation/LV_PVP_COLOSSEUM.navsource`(80x76, 셀 0.5m, 걸을 수 있는 셀 2216), `Data/Worlds/LV_PVP_COLOSSEUM/Gameplay.world.json`(playerSpawn 6개: A팀 3, B팀 3, `archetypeId` null).
- 게시(공식 publisher 실행): Area, ServerNavigation, WorldGameplay(COLOSSEUM). 산출물은 `Client|Server/Bin/DataFiles/{Map,Navigation,World}`.
- publisher 등록: `Publish-ServerNavigation.ps1`(AreaId), `Publish-WorldGameplay.ps1`(WorldId + 분기), `BuildDomains.json`(필수 출력 2개).
- Shared: `WORLD_ID::COLOSSEUM = 7`, `Is_Known_World_Id`, protocol 125 → 126.
- Server: `WorldBootstrap`/`SpawnGroupBootstrap` 이름 표, `ServerApp` 공유 시뮬레이션 기동, `GameRoom` 내비 필수 목록.
- Client: `LEVEL::COLOSSEUM`, `LOBBY_STAGE::COLOSSEUM`, `CLevelRegistry` 행(Development 셸 재사용), `CLoader::Ready_For_Colosseum`, 로비 ImGui 버튼 "콜로세움"(Debug 전용, Release는 case 자체가 없다), 세계 이동 수락 → 레벨 매핑, 캐릭터 선택 상태, 클릭 이동 마커, 체력바·저장·Debug 카메라 창의 레벨 목록, `WorldGameplayDocument` 이름 파싱.
- 로딩 화면: 제목 `증명의 전장`, 원본 `PVP_LUTERAN_30201`(1920x1080 → 1280x720) → `UI/Loading/Loading_Background_Colosseum.png`. 팁 문구는 근거가 없어 넣지 않았다.
- NetworkProtocolHarness의 protocol 기대값 4곳을 126으로 갱신.

## 검증(실행한 것만)
- 변경한 Client/Server C++ 11개 파일 `cl /Zs`: 오류 0 (제품 빌드 아님).
- `MapCatalog.json`, `BuildDomains.json` JSON parse 통과. 인코딩: 전부 UTF-8 무BOM, U+FFFD 없음, 줄바꿈은 기존 방식 유지(혼합 파일 2개는 삽입 줄만 CRLF).
- `CY_Resources`: 774개(213MB) sha1 일치.
- 빌드, Client/Server 실행, 화면 확인은 하지 않았다.

## 미복원·주의
- 쿠킹 제외 메시 2종: `herostatue05`(배치 6), `castledeco01`(배치 2). 노말/탄젠트 평행으로 geometry 계약 실패, 마하라카 선례처럼 제외.
- 텍스처 슬롯이 불완전한 변형 7개, `mapmaterials` 없음(부분 재질 미리보기).
- 수면 평면 `lv_module_water02_512`는 일반 지오메트리로 처리. BSP 7, 데칼 30, 반투명 볼륨 4는 미처리. 십자검 메달리온 바닥은 별도 확인 못 함.
- 내비: 팔각 경기장 바닥(높이 12.64m)만 걷는다. 구덩이 링(5.0m)과 우리·관중석은 걷지 못한다. 원본에 PvP 존 내비 데이터가 없어 직접 계산했다.
- 스폰 yaw는 전부 0.0(팀이 마주 보게 조정하지 않음).
- `CY_Resources` 폴더가 바탕화면에 없어(다른 작업이 이름을 바꾼 것으로 보임) 새로 만들어 콜로세움 경로만 넣었다. `CY_Resources2`, `CY_Resources_보류_이미배포추정`은 건드리지 않았다. 기존 폴더와 합칠지는 사용자가 정한다.
- 마하라카 전용 분기(물벼락·물총·닻·트리거 마커)는 COLOSSEUM에 적용되지 않는다.

## 사용자가 할 일
1. VS에서 Debug/x64 빌드(Shared 변경이므로 Server와 Client 함께). protocol 126이라 양쪽을 함께 재시작한다.
2. 로비 오른쪽 위 ImGui 패널의 "콜로세움" 버튼 → 입장, 걷기, 바닥·경계 확인.
3. 화면 확인은 사용자 몫이다.
