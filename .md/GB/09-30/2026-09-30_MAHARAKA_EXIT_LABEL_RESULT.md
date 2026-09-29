# 2026-09-30 마하라카 출구 안내 문구 변경 RESULT

## 0. 결론 (이름 확정)

- 적용한 이름: **기에나의 바다**. 사용자가 말한 "기에나의 해협"이 아니다.
- 근거(원본 확인됨): EFTable_GameMsg.db
  - `tip.name.zonebase_30703` = "[대항해] 기에나의 바다" (바다 존 30703, 스크린샷 헤더와 일치)
  - `sys.bm_warp.zonebase_30703` = "기에나의 바다"
  - "기에나의 해협"이라는 문자열은 원본에 없다. "해협"이 든 이름은 안타레스 해협(`que.7046002.01`: "기에나의 바다의 안타레스 해협", `tip.name.ocean_10_jp`)뿐이며 이는 기에나의 바다 안의 다른 지점이다.
- 다른 이름으로 바꾸려면 `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` 의 `island.exit.to.bern` 행 `interactAction` 값 `dock:<이름>` 만 고치고 `Publish-WorldGameplay.ps1 -Mode Publish -WorldId MAHARAKA` 를 실행한다(이름은 32자 이하, 제어문자 불가).

## 1. 문구가 나오는 경로 (실측)

- 화면 문구의 출처는 한 곳뿐이다: `island.exit.to.bern` 트리거의 `interactAction` 데이터(`dock:베른으로 돌아가기`).
- `Client/Private/InteractKeyPromptView.cpp` 가 `dock:` 접두사 뒤 UTF-8 문자열을 그대로 액션 줄로 쓴다(베른 입항 프롬프트 `마하라카 썸머 캠프 [G]` 와 같은 규칙). 코드에 "베른으로" 하드코딩은 없다.
- Publish-WorldGameplay.ps1 은 `^dock:[^\x00-\x1F]{1,32}$` 형식만 허용한다.
- Client/Server/Shared 소스와 로딩·전환 서비스에 "베른으로" 문자열은 없다. 게시본(Client/Server DataFiles)과 Data 전체에서 "베른으로 돌아가기" 잔여 0건.
- 베른/발탄/쿠크 문구와 월드맵·미니맵 라벨(Data/UI)은 바꾸지 않았다.

## 2. 바꾼 것

- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`
  - `island.exit.to.bern.interactAction`: `dock:베른으로 돌아가기` -> `dock:기에나의 바다`
  - `revision`: 277 -> 278 (기존 관례에 맞춘 갱신)
  - 트리거 id, `targetWorldId` BERN, `spawnPlacementId` island.return.sea.landing, 좌표, halfExtents 는 그대로.
  - 앵커 바이트 패치, LF/UTF-8(BOM 없음) 유지. 백업: tmp/Maharaka_Gameplay.world.json.bak_before_exit_label

## 3. 게시와 검증 (실행한 것만)

- `Publish-WorldGameplay.ps1 -Mode Validate -WorldId MAHARAKA` 통과(42 배치), `-Mode Publish -WorldId MAHARAKA` 성공. 약 28초.
  - Server `MAHARAKA.worldbootstrap`, Client `MAHARAKA.npcpresentation.json`, `LV_OCN_EVENTIS_MHP.viewer.world.json` 갱신.
- 게시본 `LV_OCN_EVENTIS_MHP.viewer.world.json` 에서 해당 행을 직접 확인: interactAction `dock:기에나의 바다`, 이벤트 `changeLevel BERN` + `spawnPlacementId island.return.sea.landing` 유지.
- 파일 바이트가 유효한 UTF-8이고 라벨 7글자임을 확인. JSON parse 통과, `git diff --check` 통과(줄 끝 경고 제외).

## 4. 재빌드와 재시작

- 코드 변경 없음. 재빌드 불필요.
- 안내 문구는 Client가 뷰어 문서를 읽어 그리므로 **Client 재시작**이 필요하다. Server bootstrap 도 갱신됐으므로 Server 도 함께 재시작하면 안전하다(트리거 동작은 바뀌지 않았다).

## 5. 사용자 확인 필요

- 마하라카 출구 발판 앞에서 안내 문구가 `기에나의 바다 [G]` 로 나오는지 화면에서 확인한다. 에이전트는 화면을 확인하지 않았다.
