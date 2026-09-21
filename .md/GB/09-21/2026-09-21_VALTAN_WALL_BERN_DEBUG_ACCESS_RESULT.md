# 발탄 벽타기·베른 디버그 접근 결과

## 확인된 원인

- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`의 `Stage_3`에는 181개 샘플, 6초 `wallClimb` 저작 데이터가 이미 있었다.
- 기존 `Server/Bin/DataFiles/World/VALTAN_ARENA.worldbootstrap`에는 이전 0.8초 직선 `movePlayer` payload만 있었다. 따라서 실행 중인 Server는 새 경로를 읽지 못했다.
- 발탄 F1 웨이브 버튼의 소스는 있었으나 접힐 수 있는 섹션 안에 있었다. Debug Client에서 발탄일 때 항상 보이는 행으로 변경했다.
- 베른의 `Move Player`는 UI, Client world 허용 목록, free-camera placement enable 세 곳에서 Bern이 제외되어 있었다.
- 베른 디버그 트리거 표시는 `changeLevel` 상자만 필터링했지만, 현재 켜진 상자는 `castle`, `castle.2`, `library`, `library.2`의 `movePlayer` 상자 네 개다. 기존 필터는 0개를 표시했다.

## 반영한 변경

- 발탄 월드 publisher를 실행하여 Server runtime bootstrap을 갱신했다. `Stage_3`의 published payload는 `WALL_CLIMB`, 6초, 181개 샘플이다.
- 베른 navigation publisher를 실행하여 `Bern`, `Bern2`, `Bern3` detail region을 포함한 Server runtime navgrid를 갱신했다.
- F1의 `Valtan Arena / Wave Monsters` 행은 이제 접을 수 없으며 `Normal Monster 1`, `Normal Monster 2` 버튼을 항상 표시한다.
- Bern에서도 `F6` 자유시점 → `Move Player` → 화면의 보이는 바닥 클릭이 Server-authoritative navigation projection을 요청한다. Client는 Bern을 허용하고, Server는 이미 현재 room world가 Bern인지와 navigation/collision/height를 검사한다.
- Bern Debug trigger layer는 모든 enabled `triggerBox`를 표시한다. MapTool/world pick이 시작되면 Bern placement pick도 취소해 한 클릭을 두 도구가 소비하지 않는다.
- Server teleport contract test의 normal-world matrix에 Bern을 추가했다.

## 검증

- `Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate -WorldId VALTAN_ARENA` 통과.
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_BER_BERNCASTLE` 통과. Bern3: 4,764 walkable cells.
- 발행 뒤 runtime `Stage_3`에 `WALL_CLIMB`와 6초 payload가 있음을 확인.
- `git diff --check` 통과.

## 아직 필요한 실행 검증

- Client와 Server가 실행 중이고 Visual Studio도 열려 있어 C++ 빌드·링크와 계약 테스트 실행은 하지 않았다.
- Debug Server와 Debug Client를 새로 빌드한 뒤 모두 재시작해야 protocol v101 wall-climb parser와 Bern UI 변경이 실제로 적용된다.
- 사용자가 직접 확인할 항목은 (1) Stage_3에서 6초 이동, (2) F1 발탄 웨이브 버튼 표시/재소환, (3) Bern F6 Move Player와 네 개 이동 trigger box 표시다.
