# 2026-09-19 발탄 Stage_3 이동 위치 수정 RESULT

요청: 발탄 `Stage_3` 트리거를 밟으면 Stage_Boss 쪽으로 날아가듯 이동하는 것을 없애고 절벽 위 (100.42, 20.53, -86.95)로 이동하게 한다.
좌표 해석: 사용자가 "100.42, 20,53, -86.95"로 적은 것을 쉼표 오타로 보고 (x=100.42, y=20.53, z=-86.95)로 처리했다.

## 1. 원인 (소스로 확인)

- Debug Server의 발탄 월드는 복도 지름길을 기본으로 켠다. `GameRoom.cpp:105-107`이 `WORLD_ID::VALTAN_ARENA == worldId`를 `Initialize`의 세 번째 인자로 넘기고, `ServerTriggerSystem.cpp:93`이 `_DEBUG`에서만 이것을 `m_bDebugValtanStageBypass`에 저장한다(Release는 96행에서 false).
- 켜져 있으면 `Fires_OnEntry`(315행)는 지름길 표에 있는 트리거를 G 전용으로 만들고, `Run_Trigger`(350행)는 그 트리거의 저작 action을 `Build_ValtanStageBypassMove`가 만든 도약으로 바꿔 실행한다.
- 지름길 표 `DESTINATIONS`에 `Stage_3`이 있었다: 목적지 (126.450, 23.061, -94.750), 0.90초, arc 2.0. 이 자리는 `Stage_Boss` 박스(129.65, 23.02, -96.85) 바로 앞이라 "Stage_3을 밟으면 Stage_Boss로 날아가듯 이동"하는 증상과 일치한다.
- 저작 데이터의 Stage_3은 이미 절벽 이동이었다: `Gameplay.world.json`의 `movePlayer`, 목적지 (100.300285, 20.5463905, -87.2748718), 0.8초, arc 0. 게시본(`VALTAN_ARENA.worldbootstrap` 150행)도 같은 값이다. 즉 데이터가 바뀐 것이 아니라 Debug 지름길이 저작 이동을 덮어쓰고 있었다.
- Client에는 Stage_3을 따로 처리하는 경로가 없다(`Client/`, `Shared/` grep 결과 없음).

## 2. 이력 (git log/show, 조회만)

- 저작 목적지 (100.300285, 20.5463905, -87.2748718)는 `97f70fe6`(KCY, 2026-08-06)에서 들어온 뒤 한 번도 바뀌지 않았다(값 문자열 pickaxe 결과 그 커밋 하나).
- 지름길 표의 Stage_3 행(126.450, 23.061, -94.750, 0.90, 2.0)은 `23007084`(KCY, 2026-08-14, 109-bar arena collapse)에서 처음 들어왔고 값은 그 뒤로 같다. `11b5c267`(KCY, 08-30 wip, main 병합 전 보존)에서 지름길 함수째 빠졌다가 `b5a65545`(KCY, 09-02, KoukuSaton arena slice)에서 같은 값으로 다시 들어왔다. `1d4e517b`(KCY, 09-18)는 Stage_MiniBoss 행을 뺐다.
- 추정(확정 아님): 사용자가 기억하는 "바로 위 절벽으로 이동"은 저작 데이터의 원래 동작이거나 08-30~09-02 사이(지름길 없음)의 동작일 수 있다. Stage_3 저작 값을 바꾼 커밋은 없다.
- 요청 좌표 100.42 / -86.95는 코드·데이터 이력 어디에도 없다. 현재 저작 목적지와 수평 약 0.35 m 차이다.

## 3. 바꾼 것

| 파일 | 전 | 후 |
|---|---|---|
| `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json` Stage_3 `targetPosition` | [100.300285, 20.5463905, -87.2748718] | [100.42, 20.53, -86.95] |
| 같은 파일 `revision` | 645 | 646 |
| `Server/Private/ServerTriggerSystem.cpp` 지름길 표 | `Stage_3` 행 있음 | `Stage_3` 행 삭제(표에는 `Stage_Boss`만 남음), 주석에 이유 추가 |
| `Server/Private/ServerGameplayContractTests_WorldTriggers.cpp` Stage_3 블록 | Debug는 보스 앞 도약(126.45/-94.75) 단언, Release는 저작 이동 단언 | Debug·Release 모두 저작 이동 (100.42, 20.53, -86.95) 단언, `#ifdef` 분기 제거 |
| `.md/GB/gotchas.md` 2570행 | "지금 지름길은 `Stage_3`·`Stage_Boss`뿐" | "`Stage_3`도 표에서 뺐다. 지금 지름길은 `Stage_Boss`뿐" |

- `durationSeconds` 0.8과 `arcHeight` 0은 그대로 뒀다. "절벽 이동" 시절 값이 따로 있었다는 증거가 이력에 없다(저작 값은 08-06 이후 그대로).
- `revision`은 MapTool이 저장할 때 올리는 필드이고 MapTool의 draft 기준 비교와 게시 헤더에 쓰인다. 이 프로젝트의 관례(Bern 552→553)대로 손 편집에도 1 올렸다.
- 데이터 편집은 사용자가 19:45:46에 MapTool로 저장한 파일을 기준으로 했다. 그 저장에서 바뀐 `Stage_Boss_ArenaEntry` 박스 이동과 벽 위치 반올림 6곳은 그대로 보존했다(편집 전후 의미 비교에서 바뀐 placement는 Stage_3 하나).
- 지름길이 `Stage_3`에서 빠졌으므로 Debug에서도 `Fires_OnEntry`가 `AUTO_ENTRY_RULES`로 넘어가고, 발탄 `movePlayer`는 자동 규칙 밖이라 G키로 저작 action이 실행된다. 다른 행(`Stage_Boss`)과 `Stage_Boss`의 낚시 지점 배치는 바꾸지 않았다.

## 4. 검증 (티어 구분)

| 항목 | 티어 |
|---|---|
| Stage_3 `targetPosition`이 (100.42, 20.53, -86.95)로 저작 정본에 저장됨 | 데이터로 직접 확인(JSON 파싱, 편집 전후 diff는 revision과 그 한 줄뿐) |
| 지름길 표에서 `Stage_3` 제거, 다른 행 유지 | 소스 diff로 직접 확인 |
| Debug에서도 Stage_3이 저작 action으로 실행되는 경로 | 소스로 확인(`Fires_OnEntry` → `AUTO_ENTRY_RULES`, `Run_Trigger` → `Run_Action`). 실행하지 않음 |
| 요청 좌표가 발탄 navigation에서 걸을 수 있는 칸 | 게시된 `Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`(2026-09-18 14:06)로 확인: 셀 (212,156), walkable=1, 높이 20.529(요청 y와 0.001 차이). 주변 3×3 셀도 모두 walkable(높이 20.53~20.55) |
| 도착 지점이 다른 활성 트리거 박스 안인지 | 서버 `Contains_Placement` 공식으로 계산: 어느 활성 트리거에도 포함되지 않음 |
| 서버가 이동 목적지를 거절하는지 | `Begin_MovePlayer`는 목적지 걸음·충돌을 검사하지 않고 보간만 한다(소스 확인). 위 navigation 결과가 걸을 수 있음을 보증한다 |
| 테스트 코드 | 구문 검사(`cl /Zs`)만: `ServerTriggerSystem.cpp`, `ServerGameplayContractTests_WorldTriggers.cpp` 모두 Debug·Release exit=0. 실행하지 않음 |
| 게시 | 하지 않음 |
| 빌드 | 하지 않음 |
| 게임 실행·화면 확인 | 하지 않음(사용자 확인 필요) |

## 5. 반영에 필요한 것

- `Publish-WorldGameplay.ps1 -Mode Publish`: 발탄 `worldbootstrap`의 Stage_3 행이 아직 옛 목적지이므로 필수.
- Server 재빌드(`ServerTriggerSystem.cpp` 변경)와 재시작.
- Client는 이 변경으로 바뀌는 것이 없다.

## 6. 알아둘 위험과 결정 사항

- **MapTool draft 덮어쓰기**: Debug Client의 MapTool(Development, World Gameplay)이 지금 열려 있고 메모리에 옛 Stage_3 값을 들고 있으면, 그 상태에서 저장할 때 디스크의 새 값이 옛 값으로 되돌아갈 수 있다. 저장 전에 그 Area 문서를 다시 읽거나(Reload) Stage_3 값을 확인해야 한다.
- 정확한 착지점 (100.42, 20.53, -86.95)는 사용자가 지정한 값이다. 이 값이 원작의 착지점인지는 확인하지 못했다.
- 지름길 표에는 `Stage_Boss`만 남았다. Debug에서 Stage_3 이후 보스까지 가는 길은 지름길 없이 저작 이동(Stage_3 → 절벽)과 `Stage_Boss`, `Stage_Boss_ArenaEntry` 순서로 이어진다. 절벽에서 `Stage_Boss`까지의 걷기·전투 진행이 Debug 지름길 없이 막히지 않는지는 확인하지 못했다.
- 참고(범위 밖, 바꾸지 않음): `GameRoom_WorldEntities.cpp:348`의 빈 아레나 리셋은 `Initialize`를 세 번째 인자 없이 호출해 지름길 플래그가 꺼진다.
- `2026-09-19_TRIGGER_REPEAT_AND_G_KEY_RESULT.md`(다른 fork가 편집 중)의 30·41·117·124행 등은 "Debug에서 Stage_3이 지름길 도약"이라고 적혀 있어 이번 변경 후에는 사실과 다르다. 이 fork는 그 파일을 건드리지 않았다.
