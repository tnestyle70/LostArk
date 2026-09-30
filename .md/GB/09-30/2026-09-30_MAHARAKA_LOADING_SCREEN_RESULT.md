# 2026-09-30 마하라카 진입·이탈 로딩화면 RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 마지막에 한 번 빌드). 화면 판정은 사용자 몫이다.

## 0. 결론

- 마하라카(`LEVEL::MAHARAKA`)가 베른성 이미지를 쓴 이유: `CLevel_Loading`이 목적 Level별 분기를 Valtan / Character Select / Kouku에만 갖고, 나머지는 전부 `LoadingLayout.json`의 기본 배경(베른성)과 베른 제목·팁을 쓴다. 마하라카는 분기가 없어서 기본값(베른)이 나왔다.
- 원작에는 마하라카 전용 로딩 이미지가 **있다**. 설치 클라이언트의 `ExtRes/Loading/ZONE`에 `ISLAND_57009`(2021 워터팡 섬)가 있고, 복호해서 확인했다.
- 마하라카에서 기에나의 바다로 나올 때(목적 Level은 `BERN`)도 같은 이유로 베른성 이미지가 나왔다. 원작 바다 로딩 이미지 `VOYAGE_COMMON_0/1/2` 3장(랜덤)을 찾아 연결했다.
- 이번 수정은 **C++ 변경이라 재빌드가 필요하다.** 프로토콜(122)과 Server는 바뀌지 않았다.

## 1. 우리 로딩화면 구조 (코드 실측)

| 항목 | 위치 | 사실 |
|---|---|---|
| 배경·크롬 | `Data/UI/Loading/LoadingLayout.json` | 슬롯 15개(reference 1280x720). `Background` 슬롯 기본 이미지는 `UI/Loading/Loading_Background.png`(베른성). |
| 목적 Level별 배경 교체 | `Level_Loading.cpp` `Ready_Layer_Chrome()` | `Background` 슬롯의 경로만 `m_eNextLevelID`로 바꾼다. 기존: `VALTAN_ARENA`→`Loading_Background_Valtan.png`, `CHARACTER_SELECT`→`_Prologue.png`, `KAKULSAYDON_ARENA`→`_Kouku.png`. |
| 제목·라벨·팁 | `Level_Loading.cpp` `Initialize()` | 같은 Level 분기. 나머지는 `베른 성` + `정보` + 베른 placeholder 팁. |
| 텍스처 프로토타입 | `MainApp.cpp` `Ready_Prototype_For_LoadingChrome()` | 레이아웃 JSON의 layers를 스캔해 등록하고, Level별 배경은 JSON에 안 나오므로 배열로 따로 등록한다. 빠지면 clone이 실패해 그 슬롯만 생략된다. |
| 해상도·종횡비 | `CUI_Sprite`(referenceResolution 1280x720) | 기존 이미지와 동일. 새 이미지도 1280x720 PNG로 맞췄다. |
| 전환 경로 | `LevelTransitionService::Pump_ServerApprovedWorldTransfer(currentLevel)` → `Request_Load(target, "server.trigger.change-level")` → `CMainApp`이 `LEVEL::LOADING`으로 전환 → `CLevel_Loading::Initialize(next)` | 베른→마하라카(G 입항)와 마하라카→베른(출구)이 **같은 경로**다. 출발 Level은 `currentLevel` 인자로만 존재했고 Loading에는 전달되지 않았다. |

"마하라카에서 나올 때 로딩창이 없다"는 관찰에 대해: 위 경로에서 로딩 Level을 건너뛰는 분기는 코드에서 찾지 못했다. 로딩 Level은 항상 거치며, 목적 Level이 `BERN`이라 **베른성 이미지·베른 제목**이 나온다. 실제로 화면에 아예 안 뜨는지는 사용자 관찰로 확인해야 한다.

## 2. 원작 사슬 (실측)

```
EFTable_ZoneBase.LoadingImageGroupID  -> EFTable_LoadingImageGroup.PrimaryKey
EFTable_LoadingImageGroup.LoadingImage = 이름(예 BASECAMP_57037)
  -> ReleasePC/Packages/ExtRes/Loading/ZONE/<난독화 이름>.ipk   (Tools/MoviePipeline/deobfuscate_names.py decode)
```

| 존 | 표시 이름(GameMsg tip.name.zonebase_*) | LoadingImageGroupID | LoadingImageGroup 행 | ExtRes 파일 |
|---|---|---|---|---|
| 57009 (2021) | 마하라카 파라다이스 | 57009 | **없음**(2021 이벤트 종료로 제거) | `ISLAND_57009.ipk` 있음 |
| 57025 (2023) | 마하라카 파라다이스 | 57025 | 없음 | `ISLAND_57025.ipk` 있음 |
| 57037 (2026 리턴즈) | 마하라카 썸머 캠프 | 57037 | `BASECAMP_57037` | `BASECAMP_57037.ipk`, `BASECAMP_57037_01.ipk` |
| 30703 (바다) | [대항해] 기에나의 바다 | 30700 | `VOYAGE_COMMON_0/1/2` (SelectionFactor 100씩) | `VOYAGE_COMMON_0/1/2.ipk` |

- 우리 마하라카 레벨은 `LV_OCN_EVENTIS_MHP`(2021 워터팡 섬)이고 존 57009이다. 존 57009의 표 행은 사라졌지만 원본 이미지 파일은 남아 있었다. 복호한 이미지가 우리 워터팡 아레나와 일치한다(보라·노랑 원판이 있는 풀장, 침몰선, 미끄럼틀, 풍선, 초록 도마뱀).
- 2023판(`ISLAND_57025`)은 빨강 원판·줄무늬 해적선, 2026판(`BASECAMP_57037`)은 섬 전경(MAHARAKA SUMMER CAMP 아치·부두)이라 우리 레벨과 다르다. 그래서 2021판을 채택했다. 2026 섬 전경은 바다 위 섬 모델(`ISL_00072`)과 같은 시점의 그림이라 참고로만 남긴다.
- 존 57009의 HintGroup 행도 사라졌다. 팁은 GameMsg에 남은 마하라카 전용 힌트 `sys.hint.zone_island_323`(색 태그 제거)를 썼다. **추론이다**: 원작이 이 존에 이 힌트를 붙였는지는 확인하지 못했다.
- 라벨은 `정보`로 했다. 추론이다(Type 0 필드 존 힌트는 `정보`로 보이는 기존 베른 관례를 따랐다).

## 3. 이미지 복호 (새로 풀린 부분)

`.md/TJ/09-10/2026-09-10_쿠크_입장로딩화면_RESULT.md`는 "ExtRes/Loading의 ipk는 복호하지 못했다"고 적었다. 이번에 풀었다.

- 컨테이너: **JPEG(EXIF 포함)** 을 48바이트 반복 XOR로 암호화한 것이다(무비 ipk와 같은 방식).
- 키 구조: 모든 파일 키 = SMELT 무비 키(`bink_key_solver`로 복원, `5b5d0fed…`) ⊕ 파일별 16바이트 델타(주기 16).
- 델타 복원: 평문 앞부분 `FFD8 FFE1 <len> "Exif\0\0" "MM\0*"` 14바이트를 crib로 쓰고, 나머지 2바이트(APP1 길이)는 다음 마커 위치가 유효한 후보를 전부 시도해 Pillow로 끝까지 디코드되는 것을 선택했다.
- 결과: `BASECAMP_57037`·`BASECAMP_57037_01` 3840x2160, `ISLAND_57009`·`ISLAND_57025`·`VOYAGE_COMMON_0/1/2`·`BERN_COMMON_1` 1920x1080 전부 정상 디코드.
- 도구는 저장소가 아니라 작업 폴더(`C:\Users\USER\.claude\jobs\46aea322\tmp\loading_mhp\ipk_loading_decrypt.py`, `decode_batch.py`)에 있다. 저장소 도구로 승격하지 않았다.
- 이 방식으로 `ExtRes/Loading`의 633개 ZONE 이미지를 모두 풀 수 있을 가능성이 높지만, 검증한 것은 위 8개뿐이다.

## 4. 적용한 것

리소스(Git 비추적, `Client/Bin/Resources/UI/Loading/`에 설치, `CY_Resources/UI/Loading/`에 같은 경로 복사본, 바이트 동일 확인, 복사 스크립트 없음):

| 파일 | 원본 | 용도 |
|---|---|---|
| `Loading_Background_Maharaka.png` | `ISLAND_57009` (1920x1080 → 1280x720 LANCZOS) | `LEVEL::MAHARAKA` 진입 |
| `Loading_Background_Sea_0.png` | `VOYAGE_COMMON_0` | 마하라카에서 나올 때(랜덤 1/3) |
| `Loading_Background_Sea_1.png` | `VOYAGE_COMMON_1` | 〃 |
| `Loading_Background_Sea_2.png` | `VOYAGE_COMMON_2` | 〃 |

코드(전부 CRLF·UTF-8 유지, 앵커 패치, 한글은 기존 관례대로 wide `\x` 이스케이프):

- `Client/Private/Level_Loading.cpp`: 제목·라벨·팁 분기 2개와 배경 교체 2개를 추가했다.
  - `MAHARAKA`: 제목 `마하라카 파라다이스`, 라벨 `정보`, 팁 `마하라카 섬의 다양한 놀이에 참여하면 마하라카 잎새를 얻을 수 있습니다.`, 배경 `Loading_Background_Maharaka.png`.
  - `BERN` 이면서 직전 세계 이동 출발지가 `MAHARAKA`: 제목 `기에나의 바다`, 팁 `각종 주화를 통해 더 좋은 선원을 획득할 수 있습니다.`(`sys.hint.zone_voyage_005`, 존 30703의 HintGroup 항목), 배경은 `GetTickCount64() % 3`으로 Sea 3장 중 하나(원작의 가중치 동일 랜덤을 흉내).
- `Client/Private/MainApp.cpp`: 텍스처 프로토타입 등록 배열에 새 경로 4개를 추가했다.
- `Client/Public/LevelTransitionService.h`, `Client/Private/LevelTransitionService.cpp`: `Get_LastWorldTransferOrigin()`을 추가했다. `Pump_ServerApprovedWorldTransfer`가 요청을 성공시킨 직후 `currentLevel`을 기록하고, 새 `Request()`마다 초기화한다. 그래서 로비·NPC·항구 등 일반 베른 진입은 출발지가 `END`이고 기존 베른성 로딩이 그대로 나온다. 프로토콜·Server 변경은 없다.

## 5. 실행한 검증

- `cl /Zs` 구문 검사(Debug/x64 옵션): `LevelTransitionService.cpp` rc=0, `Level_Loading.cpp` rc=0. 기존 인코딩 경고(C4819)만 나왔다. `MainApp.cpp`는 배열 4줄 추가뿐이라 별도 컴파일하지 않았고 diff로 구문을 확인했다.
- 한글 문자열의 wide 이스케이프를 diff에서 코드포인트로 대조: `마하라카 파라다이스`, `기에나의 바다` 일치.
- 4개 파일 줄 끝 CRLF 유지(bare LF/CR 0), `git diff --check` 문제 없음.
- 새 PNG 4개 헤더·1280x720 확인, `CY_Resources` 복사본 바이트 동일.

## 6. 남은 것·사용자 확인

- **재빌드 필요.** 빌드 후 Server와 Client를 함께 다시 시작한다(프로토콜 122 그대로).
- 확인할 것
  1. 베른에서 G 입항 → 마하라카 로딩: 배경이 워터팡 섬 그림이고 제목이 `마하라카 파라다이스`인지.
  2. 마하라카 출구 G → 로딩: 바다 그림 3장 중 하나가 나오고 제목이 `기에나의 바다`인지(여러 번 반복하면 그림이 바뀐다).
  3. 로비·NPC·항구로 들어가는 일반 베른 진입은 기존 베른성 로딩 그대로인지.
  4. 창 크기를 바꿔도 배경이 화면에 맞는지(기존 배경과 같은 방식).
- 근사·미확인
  - 마하라카 팁·라벨은 추론이다(존 57009의 힌트 행이 원작 표에 남아 있지 않다).
  - 바다 이미지는 표의 세 장을 모두 넣었지만, 원작에서 이 로딩이 실제로 몇 초 표시되는지·마하라카→바다 전환에서 원작이 바다 로딩을 쓰는지는 확인하지 못했다(존 30703 표만 근거).
  - 이미지를 1920x1080에서 1280x720으로 줄였다. 원본 해상도를 그대로 쓰려면 레이아웃 참조 해상도 쪽 변경이 필요하다.
  - 원본 로딩 이미지 위에는 로딩 크롬(비네트, 진행바)이 얹힌다. 원작 로딩 레이아웃과의 1:1 대조는 하지 않았다.
- 이 fork는 커밋하지 않았다. 복호 도구는 저장소에 넣지 않았다(필요하면 `Tools/MoviePipeline/`으로 승격 가능).
