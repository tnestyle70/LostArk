# 2026-09-30 마하라카 jump1/2/3 트리거 표시 RESULT

요청: 마하라카의 jump1, jump2, jump3 트리거를 쿠크에서 트리거를 표시해 놓은 것과 똑같이 표시.
상태: 코드 적용 완료. 재빌드 필요(Client만). 화면 확인은 사용자 몫이다. 빌드·Client/Server 실행·게시는 하지 않았다.

## 1. 쿠크의 표시 방식 (코드 실측)

- 쿠크는 트리거 상자 중심에 원본 이펙트 `effect.world.move_destination`(약 7초짜리 목적지 표식, 8 m 구 안에 들어가는 고정 크기)을 바닥 마커로 띄운다. 디버그 와이어나 텍스트 라벨이 아니다.
- 구현: `CLevel_KakulSaydonArena::Load_EntranceTriggerMarkers / Update_EntranceTriggerMarkerClocks / Submit_EntranceTriggerMarkers` (`Level_KakulSaydonArena.cpp` 4794~4948행). 발탄은 같은 구조의 `CLevel_ValtanArena::Load_TriggerMarkers / Submit_TriggerMarkers`(`Level_ValtanArena.cpp` 2395~2535행)다.
- 위치: `Gameplay.world.json`의 트리거 상자 `position`이 정확히 중심이다. 좌표를 코드에 넣지 않고 문서에서 읽는다.
- 그리기: `MainApp`이 물체 반복이 끝난 뒤 `Submit_*Markers()`를 부른다. 이펙트는 외부 시계로 7초마다 반복 샘플링하고, 8 m 구가 화면 밖이면 제출하지 않는다.
- 선택 규칙(쿠크): 하드코딩된 23개 ID. 선택 규칙(발탄): 켜진 movePlayer 상자 중 도착 상자를 뺀 것 + 보스 입장 상자.
- 미리 준비: `Level_Loading`이 `CClickMoveEffect::Queue_LevelResources`로 `effect.world.move_destination`을 로딩 중에 준비한다. 이 준비 조건이 쿠크와 발탄에만 열려 있었다. **마하라카는 이 이펙트가 준비되지 않았다.**
- 다른 후보(참고): F1 트리거 박스 와이어, 상호작용 프롬프트(`InteractKeyPromptView`, 마하라카에는 이미 있음), MapTool 트리거 시각화는 쿠크의 "표시"와 다르다. 위 바닥 마커가 쿠크에서 화면에 보이는 표식이라 이것을 택했다.

## 2. 마하라카 jump1/2/3의 실제 데이터

`Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`(revision 276)과 게시본 `Client/Bin/DataFiles/World/LV_OCN_EVENTIS_MHP.viewer.world.json`(revision 276)은 위치·halfExtents·yaw·enabled가 모두 같다.

| 상자 | 위치 (x, y, z) | 도착 지점 | 비고 |
|---|---|---|---|
| jump1 | (66.61, 23.20, -992.67) | (71.03, 23.10, -988.02) | 켜짐, movePlayer, 상호작용(G) |
| jump2 | (83.30, 23.40, -975.67) | (78.84, 23.23, -980.06) | 켜짐, movePlayer, 상호작용(G) |
| jump3 | (71.76, 20.65, -975.37) | (73.04, 23.13, -979.22) | 켜짐, movePlayer, 상호작용(G). 낮은 지면(y 20.65)에서 아레나(y 23.13)로 올라간다 |
| jump1_1 / jump2_1 / jump3_1 | 위 도착 지점과 같음 | - | 꺼진 도착 상자, 이벤트 없음 |

세 도착 지점은 모두 `waterpang.arena.start`(중심 (75.05, 22.65, -984.32), 반크기 5.5) 안에 있다. jump1과 jump2는 그 상자 바깥, jump3는 바깥의 낮은 지면에 있다. halfExtents는 셋 다 [1,1,1]이다. gameplay 파일에 jump3는 실제로 있다(위 표).

## 3. 적용한 내용

방식은 발탄과 같다(켜진 movePlayer 상자에 마커, 도착 상자는 제외). 데이터를 게시된 뷰어 문서에서 읽는다(상호작용 프롬프트와 같은 출처). 마커가 붙는 상자는 `jump1, jump2, jump3` 셋뿐이다.

- `Client/Private/ClickMoveEffect.cpp` (3줄): `effect.world.move_destination` 준비 조건에 마하라카를 추가했다. 이 조건은 이 이펙트의 미리 준비에만 쓰이며 클릭 이동 표시와 무관하다.
- `Client/Public/Level_Development.h` (+19줄): 마커 구조체·목록과 `Submit_TriggerMarkers()` 선언.
- `Client/Private/Level_Development.cpp` (+145줄, 마하라카 전용): `Load_TriggerMarkers`(뷰어 문서 읽기), `Clear_`, `Update_TriggerMarkerClocks`, `Submit_TriggerMarkers`. 초기화의 마하라카 블록에서 로드하고, 갱신에서 시계를 돌리고, 소멸자에서 정리한다.
- `Client/Private/MainApp.cpp` (+3줄): 발탄 옆에서 `CLevel_Development::Get_Active(LEVEL::MAHARAKA)->Submit_TriggerMarkers()`를 부른다.
- 새 파일, `.vcxproj`/`.filters` 변경, 리소스 추가, 게시 변경은 없다. `effect.world.move_destination`의 리소스 5개(`Effect/World/...` wmodel 1, dds 4)는 이 PC에 이미 있다.

## 4. 다른 레벨에 영향이 없는 이유 (코드로 확인)

- 준비 조건: `ClickMoveEffect.cpp`의 조건은 `... && LEVEL::MAHARAKA != level`이라 마하라카만 추가로 준비하고, 베른·수련장·캐릭터 선택은 여전히 건너뛴다.
- 로드·시계·제출: `Load_TriggerMarkers`는 `m_eLevel == LEVEL::MAHARAKA` 블록 안에서만 호출한다. `Submit_TriggerMarkers`는 `LEVEL::MAHARAKA != m_eLevel`이면 바로 반환한다. 다른 레벨의 `m_TriggerMarkers`는 비어 있어 소멸자 정리는 아무 일도 하지 않는다.
- `MainApp`의 호출은 `Get_Active(LEVEL::MAHARAKA)`가 마하라카 인스턴스일 때만 유효하다.
- 서버와 프로토콜은 바뀌지 않는다(프로토콜 122 그대로).

## 5. 실행한 검증 (실행한 것만)

- 앵커 기반 바이트 패치, 파일별 줄 끝(CRLF/LF) 보존. 백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\maharaka_markers\*.bak`. 적용 전후 변경 줄 수: ClickMoveEffect 3, Level_Development.cpp 145, Level_Development.h 19, MainApp 3.
- 구문 검사 `cl /Zs`(출력물 없음, 실제 빌드 아님): `ClickMoveEffect.cpp`, `Level_Development.cpp`, `MainApp.cpp` 오류 0. 기존 인코딩 경고 C4819만 출력됐다.
- `git diff --check`: 공백 오류 없음(줄 끝 경고는 기존 것).
- 마커 선택 시뮬레이션(게시된 뷰어 문서 기준): `['jump1', 'jump2', 'jump3']`.
- 실행하지 않은 것: 빌드, Client/Server 실행, 화면 확인, 게시.

## 6. 사용자가 할 일

- 재빌드 필요: Client(C++ 변경). Server 변경은 없다. 지금 쌓인 다른 변경(물총 스킬 프로토콜 122 등)과 같이 Debug/x64로 빌드하고 Server/Client를 함께 다시 시작한다.
- 화면에서 볼 것: 마하라카(워터팡 아레나 주변)에서 jump1, jump2, jump3 세 발판에 쿠크의 트리거 표식과 같은 바닥 마커가 7초 주기로 반복해서 보이는지. 마커 중심이 상자 중심과 맞는지. `jump*_1` 도착 지점, 베른 출구(`island.exit.to.bern`), 경기 시작 상자(`waterpang.arena.start`)에는 마커가 없어야 한다.
- 마커가 안 나오면 `Client/Default/EffectFailure.user.log`와 출력창의 `[MaharakaTriggerMarker]` 줄을 확인한다.

## 7. 판단이 갈리는 점

- `island.exit.to.bern`(베른으로 나가는 changeLevel 상자)에도 같은 표식을 붙일지는 요청에 없어서 넣지 않았다. 쿠크 규칙(입구 이동 상자에 표시)에 맞춰 넣으려면 `Load_TriggerMarkers`의 조건에 changeLevel 하나를 추가하면 된다.
- 마커 모양·색·크기는 쿠크 것과 같은 이펙트를 그대로 쓴다. yaw는 무시된다(쿠크·발탄과 같음).
