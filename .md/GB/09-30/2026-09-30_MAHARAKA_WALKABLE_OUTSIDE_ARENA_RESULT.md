# 2026-09-30 마하라카 아레나 밖으로 걸어 나가지 못하는 문제 RESULT

빌드와 Client/Server 실행은 하지 않았다(사용자가 마지막에 VS에서 한 번 빌드). 커밋도 하지 않았다. 실제 이동 확인은 사용자 몫이다.

## 1. 사진 분석 (스크린샷 2026-09-30 031121 / 031127)

- 두 장 모두 Debug F1 `Show Navigation` 오버레이가 켜진 마하라카(창 제목 `LostArk Maharaka Paradise`)다.
- 아레나 보라색 링 바깥의 풀장 바닥과 무지개 띠까지는 **촘촘한 격자(0.25m)**, 주황 링 바깥의 모래 해변(선베드 쪽)은 **성긴 격자(1m)**다. 둘 다 초록(걷기 가능)으로 그려진다.
- 두 번째 사진 하단 중앙의 발광 마커는 모래 쪽에 찍은 클릭 이동 목적지다. 즉 촘촘한 격자 안에서 성긴 격자 쪽으로 나가려다 못 나가는 상황이다. 보이지 않는 벽이나 물 위가 아니라 **격자 경계**가 경계선이다.

## 2. 원인 (코드 + 수치 근거)

- 서버는 질의의 **첫 점을 담는 영역의 격자로만** 판정한다(`Server/Private/ServerNavigation.cpp` `Select_Region`, `Find_Path`의 `Resolve_Cell(goal)`). 첫 점이 영역 안이면 목적지가 영역 밖일 때 그 격자에서 셀을 못 찾아 실패한다.
- 마하라카의 `WaterpangEntry` 영역은 **x 64~86, z -995~-973의 22m 정사각형(88x88, 0.25m)** 뿐이었다. 풀장과 아레나가 이 안에 있고, 그 밖은 기본 격자(160x160, 1m)다.
- 출구 트리거 `island.exit.to.bern`(62.7, 20.48, -975.6)과 플레이어 스폰 4개(61.18 / 68.36 / 63.81 / 57.55)는 모두 이 22m 창 **바깥**에 있다. 그래서 창 안(풀장)에서 시작한 이동은 출구도 스폰도 모래도 못 간다. 바깥에서 창 안으로는 들어와지는 한 방향 함정이었다.
- `collisionBox`는 마하라카 문서에 없고(`Gameplay.world.json` revision 278: triggerBox 8개뿐), 내비 `navblockers`도 0행이다. 트리거 상태나 Client 예측 문제가 아니다.
- 수정 전 검산(게시된 navgrid로 서버 규칙 재현, `Tools/MapPipeline/test_maharaka_walkable_outside.py --report`):

```text
pool -> island exit trigger          BLOCKED
pool -> spawn (61.18,-975.7)         BLOCKED
pool -> spawn (68.36,-968.82)        BLOCKED
pool -> spawn (57.55,-982.72)        BLOCKED
pool -> east sand (95,-984)          BLOCKED
pool -> south pool (75,-1003)        BLOCKED
sand -> pool                         WALK
exit trigger -> pool                 WALK
```

## 3. 섬 지형 실측 (시각 지오메트리가 비어 있는지)

- 지형은 `LAND01` 타일 16개(4x4, 각 39.68m)로 x 0~158.7, z -1071.4~-912.6을 덮는다. 기본 격자(x 0~160, z -1072~-912)와 같은 범위이고 **비어 있는 곳은 없다**(추출 누락 서브레벨은 이 지형과 무관).
- 평지(모래)는 높이 20.48이고, 풀장 바닥은 지형 기준 19.84다. 지형 높이 20.0 이상 평지는 x 28.5~140, z -1043.5~-941.5이며 그 밖은 바다 바닥(15.4 전후)이다.
- 기존 기본 격자는 160x160 전체가 20.48 평지로 걷기 가능이다(바다 위도 걷는다). 이번에 이 성질은 바꾸지 않았다(아래 6절).

## 4. 해결

`WaterpangEntry` 영역을 **기본 격자와 같은 160m 정사각형(640x640, 0.25m)** 으로 넓혔다. 서버가 첫 점 영역 하나로만 판정해도 섬 어디로든 걸어 나갈 수 있다.

- 아레나 창(x 64~86, z -995~-973)의 88x88 값은 **기존과 완전히 동일**하다(기존 영역과 값 불일치 0, 타일·원판·통 메시 21개로 굽은 22.4 부근 높이 포함). 창 밖은 기존 기본 격자와 같은 20.48 평지, 전부 걷기 가능이다.
- 덱(22.4)은 이전처럼 20.48 바닥과 높이차가 커서 걸어서는 못 오르고 jump1/2/3으로만 올라간다(검산 `test_deck_stays_a_separate_height_layer`).
- 영역이 섬 전체가 되면 "영역 안에서 걷기 가능"이 아레나 판정이 아니게 된다. 이 조건을 근거로 쓰던 세 곳에 **원래 22m 창 검사**를 함께 붙였다(Shared `Is_MaharakaWaterpangArenaFootprint`).
  - 캐논·물벼락 위험 요소: `GameRoom_PartyWorld.cpp` `Update_MaharakaWaterpangHazards`
  - 물총 무장 판정: `GameRoom_PartyWorld.cpp` `Is_MaharakaWaterpangArmed`
  - 경기 중 밀려나 아레나를 떠날 수 있는 밀림: `GameRoom_PlayerSimulation.cpp` `waterpangPush`
  - 그래서 모래를 걸어 다니는 플레이어는 캐논·물벼락을 맞지 않고 물총도 들지 않는다. 풀장·덱·부두(22m 창 안)의 동작은 이전과 같다.

## 5. 바뀐 파일

| 파일 | 내용 |
|---|---|
| `Data/Navigation/LV_OCN_EVENTIS_MHP.WaterpangEntry.navsource` | 88x88 → 640x640(원점 0, -1072). 아레나 창 값 보존. 백업 `C:\Users\USER\.claude\jobs\46aea322\tmp\mhp_walk\WaterpangEntry.navsource.before` |
| `Client/Bin/DataFiles/Navigation/LV_OCN_EVENTIS_MHP.WaterpangEntry.{navgrid,navblockers}` | `Publish-ServerNavigation.ps1 -Mode Publish`로 재생성(navgrid 2,048,020 B) |
| `Server/Bin/DataFiles/Navigation/LV_OCN_EVENTIS_MHP.WaterpangEntry.{navgrid,navblockers,navsurface}` | 같음. Client navgrid와 바이트 동일, navsurface는 원래 Server 전용 |
| `Tools/MapPipeline/configure_maharaka_waterpang_entry.py` | 다시 실행해도 같은 640x640 결과를 내도록 생성부 수정(창 sampling은 그대로, 전체 배열에 끼워 넣음) |
| `Tools/MapPipeline/test_maharaka_walkable_outside.py` | 신규. 서버 영역 선택 규칙 재현 검산 5건 |
| `Shared/Public/Gameplay/MaharakaWaterpangContract.h` | 아레나 창 상수 4개 + `Is_MaharakaWaterpangArenaFootprint` |
| `Server/Private/GameRoom_PartyWorld.cpp`, `GameRoom_PlayerSimulation.cpp` | 위 세 곳에 창 검사 추가(+3줄, +1줄) |
| `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`, `.md/GB/gotchas.md` | 영역 범위와 함정 재발 방지 기록 |

줄 끝은 그대로 유지했다(GameRoom_*.cpp CRLF, 헤더·파이썬 LF). 영역 파일에 새 파일 등록이나 vcxproj 변경은 없다.

## 6. 검증 (실행한 것만)

- `Publish-ServerNavigation.ps1 -Mode Validate` / `-Mode Publish` 성공(각 약 40초). 마하라카: 기본 격자 160x160 walkable 25,600, `WaterpangEntry 640x640, cellSize=0.25, walkable=409600`. 베른(BernSea 865,329 등)과 다른 Area의 walkable 수는 이전과 같다. 다른 Area 산출물에는 추가 변경이 없다.
- 수정 후 검산(게시본): 위 8개 경로가 전부 WALK. `test_maharaka_walkable_outside` 5건 통과(Client==Server 격자, 풀장→출구/스폰 3곳/동쪽 모래/남쪽 풀장 왕복, 덱이 별도 층으로 유지, jumpN_1 도착 지반 22.3~22.5, 영역이 기본 격자를 덮음).
- `PYTHONUTF8=1 python -m unittest test_maharaka_waterpang_entry test_maharaka_walkable_outside` 8건 중 7건 통과. 실패 1건(`test_installer_is_idempotent...`)은 초기 백업의 `jump1_1`/`jump2_1` 위치가 나중에 서로 맞바뀐 이력(revision 275/276) 때문이며 이번 변경과 무관하다. 인코딩 없이 `read_text()`를 쓰는 기존 검사는 Windows 기본 cp949에서 실패하므로 UTF-8 모드로 돌려야 한다.
- 서버 소스 두 개 `cl /Zs`(C++20, `Server\Public` `Shared\Public`) 오류 0. 오류를 실제로 잡는지 음성 대조(`error C2660`)로 확인했다. 새 헤더 함수는 `static_assert` 14건(경계 반열림, 캐논·모코코·jump 도착 3곳 안, 출구·스폰·모래·남쪽 풀장 밖)으로 검산했다. 제품 빌드가 아니다.
- `git diff --check` 문제 없음(기존 CRLF 경고만).
- Debug `Show Navigation` 오버레이는 이미 그리는 셀 4,000개와 후보 반경(셀 크기x70m)을 제한하므로 40만 셀 영역에서도 부담이 늘지 않는다(`LevelNavigationDebug.cpp`).

## 7. 걷기 가능해진 곳과 그대로인 것

- 걷기 가능해진 곳: 풀장과 무지개 띠에서 모래 해변, 출구 트리거, 스폰, 남쪽 풀장까지 섬 전체(기본 격자 범위).
- 그대로인 것 1: 덱(22.4)은 jump로만 오른다.
- 그대로인 것 2: 바다 위도 걷기 가능하다(원래 기본 격자의 성질). 해안선으로 좁히려면 지형 타일(LAND01) 높이로 다시 굽는 별도 작업이 필요하다. 이번 요청("다 걸어다닐 수 있게")에 맞춰 기존 범위를 줄이지 않았다.
- 그대로인 것 3: NPC 몸체나 소품 충돌은 내비에 없다(navblockers 0행).

## 8. 재빌드와 재시작

- **서버 C++이 바뀌었으므로 Server 재빌드와 재시작이 필요하다.** Shared 헤더도 바뀌어 Client도 다시 컴파일되지만 프로토콜(122)과 Client 동작은 바뀌지 않았다.
- 내비 데이터만 새로 반영하려면 Server를 다시 시작하면 된다(새 영역 파일은 이미 게시됨). 다만 아레나 위험 요소 판정을 이전과 같게 하려면 위 코드 변경이 함께 빌드되어야 한다. 코드 없이 데이터만 반영하면 섬 전체가 아레나로 판정되어 모래 위 플레이어도 캐논에 맞는다.

## 9. 사용자 확인

1. 풀장 바닥에서 모래 해변으로 클릭 이동이 되는지, 출구(`기에나의 바다 [G]`) 근처까지 걸어가지는지.
2. 경기 중 캐논이 돌 때 모래 위에서는 맞지 않고, 풀장과 덱에서는 이전처럼 맞는지. 물총도 모래에서는 들지 않고 풀장·덱에서는 드는지.
3. jump1/2/3 발판, 덱 오르기(jump), 낙사 후 되살리기가 이전과 같은지.
4. `Show Navigation`을 켜면 섬 전체가 촘촘한 격자(0.25m)로 보인다.
