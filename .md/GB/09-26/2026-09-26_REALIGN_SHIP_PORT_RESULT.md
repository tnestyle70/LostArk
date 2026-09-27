# 새 main 위로 배 탑승 작업 이식 (2026-09-26)

사용자 지시: "git main에 올라온 것들 전부 pull 받는데 내가 수정한 것들 싹 다 밀어버리고 main 기준으로 맞추는데 배 탑승이랑 마하라카 두 개만 빼고 하라는 거야." 이 문서는 그중 **배 탑승** 쪽(임무 H)의 결과다. 마하라카는 다른 fork가 `2026-09-26_REALIGN_MAHARAKA_PORT_RESULT.md`에 적는다.

작성: 2026-09-26. 이 fork는 Client와 Game UI를 실행하지 않았고, 빌드와 서버 테스트를 하지 않았고, 커밋·push도 하지 않았다. 화면에서 배가 어떻게 보이는지는 확인하지 못했다(사용자 몫).

## 1. 결론

- 새 main(`a84bbcd4`) 위에 **배 탑승 관련 변경만** 다시 얹었다. 이식한 파일 59개: 백업 스냅샷에서 통째로 가져온 것 35개, main도 바꾼 파일 11개는 배 hunk만 골라 3-way 병합, 게시로 다시 만든 것 11개(바뀐 4 + 새 7), 내 문서 2개.
- 병합 충돌 0건. 병합한 11개 파일은 새 main 대비 변경이 배 hunk 크기와 정확히 같다(main의 기존 내용 보존).
- 게시 3종(바다 네비, 탈것, 베른 월드)을 다시 돌렸다. 바뀐 게시 파일 4개와 새 게시 파일 7개가 **전부 배 관련**이고, 나머지 152개는 바이트 동일하다. Bern/Bern2/Bern3 네비 게시본 27개도 동일하다.
- 베른 월드 게시는 처음에 **main 자체 결함**으로 실패했다(`trackBombs`, 7절). 팀장 파일은 고치지 않고 임시 사본으로 게시한 뒤 사본을 삭제했다.
- 구문 검사(`cl /Zs`, 링크·빌드 아님): Server 5개, Client 12개 모두 오류 0(Client는 `/utf-8`와 표준 PCH 강제 포함 옵션).
- **빌드는 하지 않았다.** 조정자가 Debug Product Build를 돌린 뒤 확인해야 할 항목은 10절에 있다.

## 2. 출발 상태와 방식

- 브랜치 `codex/main-ship-maharaka-0926`, HEAD = `origin/main` = `a84bbcd4`. 지난 병합 기준 main은 `ca02c873`(main은 그 뒤 새 커밋 10개, 파일 257개).
- 내 쪽 최종 상태 S = 스냅샷 커밋 `135772ae`(브랜치 `backup/pre-realign-0926-worktree`, 부모 `cd58d12b`). 내 변경 = `git diff ca02c873 135772ae` + 미추적 파일.
- 방식 (a) **main이 안 바꾼 파일**: 파일마다 `git rev-parse ca02c873:<경로>`와 `HEAD:<경로>`가 같음을 먼저 확인(35개 전부 통과)하고, 있던 파일은 `out/Realign20260926/ship/backup/`에 복사한 뒤 `git restore --source=135772ae -- <정확한 경로>`로 가져왔다.
- 방식 (b) **main도 바꾼 파일**: 도구 `port_ship_merge.py`(`C:\Users\USER\.claude\jobs\46aea322\tmp\`)가 파일마다 base(`ca02c873`)와 S의 차이를 hunk로 나눠 배 관련 hunk만 고르고(포함/제외 표식 문자열로 선택), 그 hunk만 base에 적용해 "theirs"를 만든 뒤 `git merge-file`로 새 main(ours)과 합쳤다. blob 단위라 CRLF가 그대로 보존된다. 선택된 hunk는 `out/Realign20260926/ship/<파일>.selected.patch`에 남겼다. 적용 전에 check 모드로 충돌 0을 확인했다.
- 게시 산출물은 S에서 복사하지 않고 이식한 authoring에서 **다시 생성**했다.

## 3. 통째 이식한 파일 35개 (main이 안 바꾼 파일)

Data / Tools
- `Data/Actors/NpcCatalog.json`: 배 NPC archetype 2행(`NPC_SHIP_SHIPWRIGHT`, `NPC_SHIP_HARBORMASTER`)
- `Data/Actors/VehicleCatalog.json`: 배 9종(8200~8208) 행과 `ship`/`seatOffset`/`modelYawDegrees`/`modelLiftMeters` 필드
- `Data/Vehicles/VehicleProfiles.json`: 배 속도 행 9개(원본 `EFTable_VoyageShip.MoveSpeed`)
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`: 배 NPC 배치 3개(`npc.bern.ship.shipwright.1/.2`, `npc.bern.ship.harbormaster.1`), revision 921 → 922
- `Data/Camera/Bern.camera.json`: `shipCamera` 블록(원작 `EFTable_CameraSetting` 1001/1)
- `Data/UI/Vehicle/VehicleUiCatalog.json`: 배 목록 창 문구
- `Data/Navigation/LV_BER_BERNCASTLE.BernSea.navsource`(새 파일, 296×212, 셀 0.5 m, 높이 10.95), `Data/Navigation/LV_BER_BERNCASTLE.navregions`(영역 3 → 4, `REGION "BernSea" 1` 한 줄만)
- `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`: 배 속도 출처 테이블 허용
- `Tools/ShipPipeline/`(새): `add_ship_npcs.py`, `build_sea_nav.py`(`SHIP_DRAFT_M -0.15`, `CELL 0.5`), `build_ship_data.py`, `cook_npc.py`, `cook_ships.py`, `make_distribution.py`
- `.md/TEAM/NPC_OWNER_HANDOFF.md`(NPC 모델 크기 계약 절), 배 문서 3개(`2026-09-25_BERN3_SHIP_NPC_VISIBILITY`, `..._ORIGINAL_BOARDING`, `..._SAILING`)

Client
- `ActorCatalog.h/.cpp`(배·좌석 오프셋·모델 회전·들어올림 파싱), `ArenaCameraProfile.h/.cpp`(`shipCamera` 파싱·검증), `Part_Vehicle.h/.cpp`(좌석 오프셋), `VehiclePresentationAssetService.cpp`(`modelLiftMeters`·yaw 사전 변환과 진단), `VehicleWindowView.h/.cpp`(배 목록 모드), `WorldPlayerNameplateView.cpp`(배 탑승자 이름표 숨김), `Level_Bern.h`(배 NPC 상호작용·배 카메라 선언)

Server
- `GameRoom_VehicleRiding.cpp`(탑승하면 `BernSea`로 이동, 하차하면 부두 복귀, `SHIP_SEA_NAV_LEVEL_Y = 10.95f`), `ServerNavigation.h/.cpp`(`Is_PointWalkableInRegion`), `GameRoom_WorldEntities.cpp`(`[ShipNpc]` 로그), `ServerGameplayContractTests_VehicleRiding.cpp`(배 시나리오와 `SEA_LEVEL = 10.95f`)

검증: 통째 이식 코드·데이터 파일은 S와 내용이 같다(추적 파일은 `git diff 135772ae`가 비었고, 새 파일 10개는 바이트 비교로 같음). JSON 6개와 python 6개는 parse/compile 실패 0.

## 4. main도 바꾼 파일 11개: hunk 선택 병합 (충돌 0)

표식 = 그 hunk의 +/- 줄에 그 문자열이 들어 있으면 이식, 제외 문자열이 있으면 버림. 번호는 `git diff -U1 ca02c873 135772ae -- <파일>`의 hunk 순서.

- `Server/Public/ServerPlayer.h`: hunk 1 전부(배 부두 위치 필드 5개) 이식.
- `Server/Public/GameRoom.h`: hunk 1(`Begin_ShipVoyage`/`End_ShipVoyage`) 이식. **hunk 2 버림**: `Enter_KoukuRaidCombat(gate, tick)` — 쿠크.
- `Server/Private/GameRoom_PlayerCommands.cpp`: 1줄(`staged.bShipDockValid = false`) 이식.
- `Client/Public/Character.h`: hunk 2(`Is_ShipPresentation`), 4(`m_isShipPresentation`) 이식. **버림**: hunk 1(`Apply_MarioPresentation(..., marioStage)`, `Get_MarioStage`), hunk 3(`m_iMarioStage`) — 마리오 표현 범위.
- `Client/Private/Character.cpp` (14 hunk 중 4개 이식: 1, 11, 13, 14): 1 = `EffectFailureDiagnostic.h` include, 11 = `desc.vSeatOffset = pVehicle->seatOffset`(탈것 좌석 오프셋, `Part_Vehicle`과 한 세트), 13 = 배 탑승자 숨김 블록(`ship.rider.hidden` 로그), 14 = `Late_Update`의 배만 그리기. **버림 10개**(2~10, 12): `CombatHUDViewModel.h` include, `Is_CueScopedOut`과 그 호출 지점들(이펙트·사운드·탈것 스킬 사운드·이동 사운드·탈것 이벤트), `Apply_MarioPresentation` 본문, 에스더 시전 사운드 국소화 — 전부 마리오 표현 범위.
- `Client/Private/ClientReplication.cpp` (8 hunk 중 5개 이식: 1, 3, 4, 5, 6): `EffectFailureDiagnostic.h` include와 배 NPC 진단(`npc.presentation.unavailable`, `npc.ship.spawned`). **버림 3개**(2, 7, 8): 마리오 단계 인자 전달, 에스더 사운드 국소화.
- `Client/Private/MainApp.cpp`: hunk 1(`Open_ShipWindow`) 이식. **hunk 2 버림**: 마리오 단계 데미지 숫자 국소화. `Client/Public/MainApp.h`: `Open_ShipWindow` 선언 이식.
- `Client/Private/PlayerController.cpp`(7 hunk 전부): 바다 클릭 평면(뿌리 − 0.15 m), `vehicle.riding` 진단.
- `Client/Private/NpcPresentationAssetService.cpp`(4 hunk 전부): 배 NPC 모델 크기(`Resolve_NpcModelPreScale`).
- `Client/Private/Level_Bern.cpp`(4 hunk 전부): 배 NPC 상호작용, `Update_ShipCamera`(원작 값).

병합 후 새 main 대비 변경 줄 수(= 배 hunk의 크기와 같아야 함): ServerPlayer.h +4, GameRoom.h +6, GameRoom_PlayerCommands.cpp +1, Character.h +3, Character.cpp +20, ClientReplication.cpp +20, MainApp.cpp +6, MainApp.h +2, PlayerController.cpp +33/−1, NpcPresentationAssetService.cpp +53/−1, Level_Bern.cpp +208. main이 이 파일들에 넣은 새 내용은 전부 그대로 남아 있다.

**이식하지 않은 배 관련 의심 파일**: 없다. `CombatHUDViewModel.h`(마리오 국소화 39줄)는 배와 무관해 통째로 버렸다.

## 5. 게시 3종

잠금 `C:\Users\USER\.claude\jobs\46aea322\tmp\build.lock`을 잡고 순서대로 실행. 게시 전 대상 출력을 `out/Realign20260926/ship/backup/published/`(그리고 월드 폴더는 `published_before_world/`)에 백업했고, 게시 전후 전체 hash를 `hash_before.txt`(156개)와 `hash_after.txt`(163개)로 비교했다.

1. 바다 네비: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate/Publish -AreaId LV_BER_BERNCASTLE` 둘 다 rc=0.
2. 탈것: `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1 -Mode Validate/Publish` 둘 다 rc=0.
3. 베른 월드: `Publish-WorldGameplay.ps1 -Mode Validate -WorldId BERN`은 rc=1로 실패(7절). 임시 사본으로 Validate/Publish 둘 다 rc=0.

게시 전후 비교 결과:
- 바뀐 파일 4개: `Client/Bin/DataFiles/Navigation/LV_BER_BERNCASTLE.navregions`, `Server/.../Navigation/LV_BER_BERNCASTLE.navregions`(둘 다 개수 3 → 4와 `REGION "BernSea" 1` 한 줄), `Server/.../Vehicles/Vehicles.bootstrap`(개수 32 → 41, 배 9행 8200~8208), `Server/.../World/BERN.worldbootstrap`(리비전 921 → 922, 개수 64 → 67, 배 NPC 3행).
- 새 파일 7개: BernSea `navblockers`/`navgrid`/`navpolicy`(Client 3, Server 3)와 Server `navsurface`. 사라진 파일 0.
- 바이트 동일 152개. Bern/Bern2/Bern3/기본 네비 게시본 27개 전부 동일. `Gameplay.bootstrap`, 다른 월드 bootstrap, Client World 파일도 `git status`에 변경 없음.

## 6. main의 게시 도구 결함 (팀장님께 전달)

- 증상: `Publish-WorldGameplay.ps1 -Mode Validate -WorldId BERN`이 `ENCOUNTER_KAKULSAYDON_G1 pattern has missing or unknown fields`로 거부(`Assert-ExactProperties`, 스크립트 약 114행). actual에만 `trackBombs`가 더 있다. Compositions의 `shape` 문제와 같은 종류.
- 증거 (모두 확인함):
  - 내가 main 대비 바꾼 것은 `Data/Encounters`, `Data/Worlds`, `Data/Balance`, `Tools/WorldPipeline` 중 베른 `Gameplay.world.json`(배 NPC 행)뿐이다. 쿠크 인카운터 문서와 게시 스크립트는 `origin/main`과 동일하다(`git diff` 비어 있음).
  - `trackBombs`를 인카운터 문서에 넣은 main 커밋: `e612f4f6 "koukusaydon pattern"`. 문서의 `trackBombs` 1건.
  - main의 `Publish-WorldGameplay.ps1`에는 `trackBombs`가 0번 나온다. 이 필드를 쓰는 코드는 `Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1`, `project_kouku_saydon_composition.py`, `test_showtime_track_bomb_bootstrap.py`.
  - 월드 게시 검증은 -WorldId와 무관하게 쿠크 인카운터 패턴을 검사하므로, main 그대로도 실패한다.
- 처리: 팀장 파일은 고치지 않았다. `Tools/WorldPipeline/Publish-WorldGameplay.shipport.tmp.ps1`(같은 폴더의 임시 사본)에서만 패턴 허용 필드에 `trackBombs`를 통과시키는 5줄을 더해 Validate/Publish를 돌리고, **사본은 삭제했고 삭제를 확인했다**(잠금도 해제). 원본 스크립트는 `git diff`가 비어 있다.
- 팀장님께 넘길 patch: `out/Realign20260926/ship/patches/publish_worldgameplay_trackbombs.patch`(원본 스크립트에 적용하는 5줄 추가 한 블록).
- 이 게시 스크립트가 함께 다시 쓰는 다른 파일은 없었다(위 hash 비교로 확인). 그래서 `git restore --source=origin/main`으로 되돌릴 파일이 없었다.

## 7. 구문 검사 (`cl /Zs`, 링크·빌드 아님, 산출물 없음)

명령은 `Client/Default/x64/Debug/Client.tlog/CL.command.1.tlog`와 `Server/Intermediate/x64/Debug/Server.tlog/CL.command.1.tlog`의 실제 옵션을 따랐다(스크립트 `zs_check.ps1`, 로그 `out/Realign20260926/ship/logs/zs_*`). main이 Engine 공개 헤더 3개(`Mesh.h`, `Model.h`, `Sound_Manager.h`)를 바꿨으므로 `Engine\Public`을 `EngineSDK\Inc`보다 먼저 포함했다.

- Server 5개 오류 0: `GameRoom_VehicleRiding`, `ServerNavigation`, `GameRoom_PlayerCommands`, `GameRoom_WorldEntities`, `ServerGameplayContractTests_VehicleRiding`.
- Client 12개 오류 0(`/utf-8`와 표준 PCH 강제 포함 옵션): `Level_Bern`, `Character`, `ClientReplication`, `PlayerController`, `NpcPresentationAssetService`, `MainApp`, `ActorCatalog`, `ArenaCameraProfile`, `Part_Vehicle`, `VehiclePresentationAssetService`, `VehicleWindowView`, `WorldPlayerNameplateView`.
- **알아둘 점**: 처음 검사(`/utf-8` 없이)에서 `MainApp.cpp`만 오류 18줄(C2065 12, C2737 6)이 나왔다. **main 원본 `MainApp.cpp`(`git show HEAD:...`)로도 같은 18줄**이 나와서 이식한 hunk와 무관하다. 원인은 UTF-8 소스(`// 사망하였습니다` 같은 한글 줄 끝 주석)를 CP949 코드 페이지로 읽으면 줄 끝 리드 바이트가 줄바꿈을 삼켜 다음 줄이 주석이 되는 것이다(그 주석만 지운 사본은 18 → 15줄, `/utf-8`를 주면 0줄). 같은 주석이 지난번에 정상 빌드된 `ca02c873`에도 있었으므로 실제 빌드가 왜 통과하는지는 이 검사 환경과 무엇이 다른지 확정하지 못했다. 실제 판정은 조정자의 정식 빌드다.

## 8. 렌더링 보호와 사용자 소유 네비

- `git diff --stat origin/main`: `Engine/Private/Renderer.cpp`, `Engine/Public/Renderer.h`, `Client/Bin/ShaderFiles`, `Data/Rendering`, `Client/Bin/DataFiles/Rendering`, `Client/Private/UI_Sprite.cpp`, `LevelRegistry.cpp`, `GameInstance.*` → 변경 없음.
- `Data/Navigation`의 Bern/Bern2/Bern3 정본(`navsource`/`navpaint`/`navblockers` 등) → main 대비 변경 없음. `navregions`의 변경 줄은 사용자가 승인한 BernSea 한 줄과 개수 3 → 4뿐이다.
- 이 fork는 `Publish-RenderingProfiles.ps1`을 실행하지 않았다.

## 9. `git diff --name-only origin/main` (추적 파일) 각 파일의 이유

이 fork의 파일(추적 변경): 3절 코드·데이터·도구·문서 중 main에 이미 있던 파일 + 4절 11개 + 5절 게시 4개. 각 이유는 3·4·5절에 적었다.

이 fork의 파일이 아닌 추적 변경 12개는 전부 마하라카 fork(I)의 것이다: `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets`/`.mapplacements`, `Client/Private/Level_Development.cpp`, `Client/Public/Level_Development.h`, `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/...mapplacements`, `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/`의 receipt 2개·`mapassets`·`mapplacements`·`renderprofiles.json`, `Data/Maps/MapCatalog.json`, `Tools/LevelPlacementExtractor/README.md`. 새 파일(미추적)도 이 fork 것은 배 파일 외에 없고, 마하라카 데이터·문서·도구와 이전 fork의 문서(`2026-09-25_MERGE_FAILURES_FIX_RESULT.md` 등)가 있다. 전체 목록: `out/Realign20260926/ship/inventory.txt`.

## 10. 빌드 뒤 확인 순서 (빌드는 조정자·사용자)

1. Visual Studio가 닫혀 있는지 확인하고 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`(정상 증분 Build, Rebuild·Clean 금지). main이 셰이더와 C++를 넓게 바꿨으므로 이번 빌드는 길 수 있다(추정).
2. `Server\Bin\Debug\Server.exe --vehicle-riding-contract-test`: 기대 **77 PASS / 실패 0**. `--navigation-contract-test`: 기대 **34 PASS**.
3. Server와 Client를 재시작하고 Debug로 Bern에서 확인(화면 판정은 사용자): 배 NPC 3명이 보이는지, NPC에게 말을 걸어 배를 고르고 탑승했을 때 배가 바다로 이동하고 선체 바닥이 수면 위(0.15 m)에 뜨는지, 탑승자가 안 보이는지, 카메라가 17 m 거리 −45도로 내려다보는지(원작 값), 우클릭으로 바다에서 이동되는지, 하차하면 부두로 돌아오는지, F1 `Show Navigation`으로 바다 격자가 0.5 m로 촘촘한지. `Client/Default/EffectFailure.user.log`의 `ship.rider.hidden`, `npc.ship.spawned`, `vehicle.riding`, `vehicle.model` 줄로 원인을 볼 수 있다.
4. 빌드에서 컴파일 오류가 나면 이 fork가 건드린 파일부터 본다. `MainApp.cpp`가 이상하면 7절의 `/utf-8` 이야기를 참고하되 main 원본에서도 나는 오류라는 점을 기억한다.

## 11. 되돌리는 방법

- 예전 상태 전체: 브랜치 `backup/pre-realign-0926-head`(`cd58d12b`), `backup/pre-realign-0926-worktree`(`135772ae`, 저장 안 했던 추적 수정 포함), `out/Realign20260926/`(patch, 미추적 파일 복사본).
- 이 fork의 수정 전 파일: `out/Realign20260926/ship/backup/`(있던 파일 원본), `.../backup/published/`와 `published_before_world/`(게시 전 출력).
- 이식만 취소하려면 정확한 경로를 `git restore --source=origin/main -- <경로>`로 되돌리고 새 파일은 삭제한다.

## 12. 확정하지 못한 것

- 빌드(링크)와 서버 테스트, 화면 결과, 원작 흘수(배가 수면에서 얼마나 잠기는지)는 확인하지 못했다. "수면 위 0.15 m"는 이전 fork가 정한 값이다.
- `MainApp.cpp` 구문 검사 환경과 실제 빌드가 왜 다르게 동작하는지(7절).
- 배 관련 hunk 판정은 표식 문자열과 코드 읽기에 근거한 것이다. `Character.cpp`의 `vSeatOffset` hunk는 좌석 오프셋 기능(배는 좌석 본이 수면 위에 없어 갑판 오프셋을 쓴다는 `ActorCatalog.h` 주석)의 일부로 보고 이식했다. 배 외 탈것에 영향을 줄 수 있는지는 카탈로그에서 `seatOffset`이 배 행에만 있다는 것으로 판단했다(추론).

## 13. 최종 재확인

- 확인한 것: 35개 통째 이식 전 main 무변경 검사, 11개 병합의 충돌 0과 main 대비 변경량, 게시 3종의 rc와 게시 전후 hash 비교(바뀐 4 + 새 7 전부 배 관련, 152 동일, Bern 계열 27 동일), 게시본 4개의 내용 diff, 월드 게시 실패의 main 자체 결함 증거(diff·커밋·소비 코드), 임시 사본 삭제와 잠금 해제, 구문 검사 Server 5 + Client 12, 렌더링 보호·사용자 네비 무변경, 통째 이식 파일의 S 일치.
- 확인하지 못한 것: 빌드·링크, 서버 계약 테스트, 화면, `MainApp.cpp` 실제 빌드 통과 여부.

SHIP_PORT_DONE
