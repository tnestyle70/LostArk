# Bern3 배 흘수(침몰 표시) 수정과 바다 네비 0.5 m 재생성 RESULT

작성: 2026-09-25 22:35~22:55 (fork D). 사용자 요청: 배를 타고 바다에 있으면 배가 물에 잠겨 보이니 배 바닥이 수면 위에 오게 고치고, 바다 네비 `BernSea`를 0.5 m 격자로 촘촘하게 다시 깔 것.
빌드는 이 fork가 하지 않았다(조정자가 뒤에 Debug Product Build 실행). Client와 게임은 실행하지 않았으므로 화면에서 배가 어떻게 보이는지는 확정하지 못한다. 최종 화면 판정은 사용자 몫이다.

## 1. 실측 (사실)

배가 그려지는 높이는 네 요소의 합이다. 각 값을 코드·데이터에서 직접 읽었다.

- 서버가 탑승 후 플레이어를 두는 높이: `Server/Private/GameRoom_VehicleRiding.cpp`의 `SHIP_SEA_NAV_LEVEL_Y` = **9.3 m**(수정 전). BernSea 격자의 셀 높이도 9.3(생성기 `SEA_SURFACE_Y 10.8 − SHIP_DRAFT_M 1.5`).
- Client가 배 모델을 두는 위치: `CPart_Vehicle`의 부모 행렬은 플레이어 Transform(발 위치, 서버 y) 그대로다(`Character.cpp` `desc.pParentMatrix = &m_VehicleRootMatrix`, `Update_PresentationRootMatrix`). `seatOffset`은 탑승자를 좌석 본 위로 올리는 값이지 배를 올리지 않는다(`Try_Get_SeatWorldPosition`, `Try_Get_PresentationRootMatrix`). 모델 사전 변환은 `modelPreScale 0.01`과 `modelYawDegrees`뿐(`VehiclePresentationAssetService.cpp` 70~71행).
- 배 WModel 정점 y 범위(모델 단위 cm, `modelPreScale 0.01` 적용 후 m). 측정 도구 `Tools/ModelAssetConverter/verify_dimensionmaster_summon_bind_pose.read_wmodel`, 결과 `out/ShipFix20260925/ship_vertex_bounds.json`:
  - 8200 Estoc: 최저 −0.167 m, 최고 3.61 m
  - 8201 Whitewind: 최저 −0.193 m, 최고 3.32 m
  - 8202 Astray: 최저 −0.548 m, 최고 4.08 m
  - 8203 Barkstorm: 최저 −0.362 m, 최고 1.70 m
  - 8204 Ghost: 최저 −0.658 m, 최고 6.64 m
  - 8205 Brahms: 최저 −0.244 m, 최고 3.87 m
  - 8206 Tragon: 최저 −0.316 m, 최고 3.36 m
  - 8207 Pneuma: 최저 −0.548 m, 최고 3.87 m
  - 8208 Luminous: 최저 −0.198 m, 최고 3.66 m
  즉 아홉 척 모두 모델 원점이 용골(선체 최저점) 바로 위에 있다(원점이 갑판이나 수면선이 아니다). 이전 가정 "원점 = 용골"은 맞았고, 실제 최저점은 원점보다 0.17~0.66 m 아래다.
- 수면 높이: 항구 정박 배 `COMMON_SHIP01` 7개 배치 y 10.78~10.97, 바다 평면 `LV_MODULE_WATER02_512` 배치 y 10.71/10.92 (`Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements`). 이전 작업과 같은 근거로 **10.8 m**를 유지한다. 정적 맵 WModel(정박 배·물 평면)은 이 저장소의 python 읽기 도구가 지원하지 않는 형식(WINT 1.2 정적)이라 정점 범위를 재지 못했다(미확인).

계산된 잠김 깊이(수정 전): 선체 최저점 = 9.3 + 최저 y = 8.64~9.13 m → 수면 10.8보다 **1.67~2.16 m 아래**. 갑판(좌석) 높이도 9.3 + 0.45~0.9 = 9.75~10.2 m로 수면 아래다. 사용자가 본 "침몰"과 일치한다(수치 추론; 화면은 확인하지 못함).

## 2. 원인

- 사실: 이전 작업이 정적 `ESTOCSHIP01`(맵 소품, y 9.09)을 근거로 흘수 1.5 m를 가정했는데, 탑승용 배 WModel은 원점이 용골이라 원점을 수면 1.5 m 아래에 두면 선체 전체와 갑판까지 잠긴다.
- 추론: 정적 소품 모델은 원점 위치가 다르거나(측정 불가) 원래부터 다른 높이 기준일 수 있다.
- 사용자 요구 목표: 선체 바닥이 수면 위. 이 문서의 목표값은 "선체 최저점 = 수면 + 0.15 m".

## 3. 바꾼 것과 근거

한 값으로 아홉 척을 맞출 수 없어서(최저점이 0.17~0.66 m로 배마다 다름) 두 단계로 나눴다.

1. 서버·네비 높이(모든 배 공통): 배 뿌리(플레이어 발) = 수면 + 0.15 = **10.95 m**.
   - `Tools/ShipPipeline/build_sea_nav.py`: `SHIP_DRAFT_M 1.5 → -0.15`(음수 = 뿌리가 수면 위), `CELL 1.0 → 0.5`, 문서 문자열 갱신.
   - `Server/Private/GameRoom_VehicleRiding.cpp`: `SHIP_SEA_NAV_LEVEL_Y 9.3f → 10.95f`, 주석 갱신. 허용 오차 0.6 m는 그대로.
   - `Client/Private/PlayerController.cpp`: 바다 우클릭 평면 상수 `SHIP_WATER_ABOVE_KEEL_M 1.5f`(뿌리 + 1.5)를 `SHIP_WATER_BELOW_ROOT_M 0.15f`(뿌리 − 0.15)로 바꿨다(수면이 뿌리보다 0.15 m 아래).
2. 배별 모델 들어올림(Client 표현): 새 선택 필드 `modelLiftMeters`를 카탈로그에 추가하고, 모델 프로토타입 사전 변환에 +Y 이동으로 더했다. 값 = 최저 y의 절댓값을 cm 단위로 올림.
   - `Data/Actors/VehicleCatalog.json`(formatVersion 4 유지, 배 9행에 한 줄씩): 8200 0.17, 8201 0.20, 8202 0.55, 8203 0.37, 8204 0.66, 8205 0.25, 8206 0.32, 8207 0.55, 8208 0.20.
   - `Client/Public/ActorCatalog.h`: `VEHICLE_ACTOR_ENTRY::modelLiftMeters`(기본 0).
   - `Client/Private/ActorCatalog.cpp`: 선택 키 `modelLiftMeters` 파싱(숫자·유한·절댓값 20 이하), 정확 키 개수 검사에 포함.
   - `Client/Private/VehiclePresentationAssetService.cpp`: 사전 변환 `Scale × RotY × Translation(0, modelLiftMeters, 0)`. Engine `CModel`은 사전 변환을 뿌리 본의 부모로도 쓰므로(`Model.cpp` 483행) 메시와 좌석 본이 함께 올라간다. 탑승자 좌석 높이는 배와 같이 움직이므로 `seatOffset`은 바꾸지 않았다.
   - `Tools/ShipPipeline/build_ship_data.py`: 생성기도 같은 필드를 cook receipt의 bbox에서 내보내도록 한 줄 추가(`import math` 포함).
   결과 예상: 선체 최저점 = 10.95 + lift + 최저 y ≈ 10.95~10.96 m, 즉 수면보다 0.15 m 위. 갑판은 11.4~11.85 m.

Server bootstrap(`Vehicles.bootstrap`)에는 이 필드가 들어가지 않으며 `Publish-VehicleProfiles.ps1`은 카탈로그를 비행 검증에만 읽으므로 탈것 게시는 다시 돌리지 않았다. Client는 `Data/Actors/VehicleCatalog.json`을 직접 읽는다(`ActorCatalog.cpp` 1364행).

## 4. 바다 네비 0.5 m 재생성

- 생성 명령: `python Tools/ShipPipeline/build_sea_nav.py --write` (출력 `out/ShipFix20260925/logs`에는 없음, 콘솔 결과를 아래에 옮김).
- 전: 148×106 셀, 셀 1.0 m, 걸을 수 있는 셀 9,802개 = 9,802 m², 높이 9.3.
- 후: 296×212 셀, 셀 0.5 m, 걸을 수 있는 셀 39,230개 = 9,808 m², 높이 10.95. 원점(192, −262)과 범위(148×106 m)는 같다. 정적 keep-out 117개, 열린 구성 요소 6개 중 가장 큰 것 채택. 면적이 6 m² 늘어난 것은 경계가 촘촘해진 만큼이고 영역이 넓어진 것이 아니다.
- 게시: `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_BER_BERNCASTLE` → 성공, 이어서 `-Mode Publish` → 성공(22:47, 빌드 잠금 사용, 6초). 결과 `out/ShipFix20260925/logs/nav_validate.txt`, `nav_publish.txt`.
- 게시본: `Client/Bin/DataFiles/Navigation/LV_BER_BERNCASTLE.BernSea.navgrid`와 Server 쪽이 296×212, 셀 0.5, 313,780 bytes(전 78,460)로 바뀌었고 Client/Server 파일이 바이트 동일. `BernSea.navblockers`, Server `BernSea.navsurface`도 게시자가 다시 썼다. `BernSea.navpolicy`는 내용이 같아 변경 없음.
- 사용자 소유 영역: `LV_BER_BERNCASTLE.navgrid`, `.Bern2.navgrid`, `.Bern3.navgrid` 게시본은 게시 전후 SHA-256이 같다(`out/ShipFix20260925/nav_hashes_before.txt`, `nav_hashes_after.txt`). `Data/Navigation`의 Bern/Bern2/Bern3 `navsource/navpaint`와 `navregions`는 `git status`에 나타나지 않는다(무변경).
- 겹침 높이 검사: Validate/Publish가 BernSea(10.95)와 Bern3의 XZ 겹침을 통과했다. 바다 셀은 육지·부두 셀에서 6 m 이상 떨어져 있어 같은 XZ에서 둘 다 걸을 수 있는 셀이 없다.

## 5. 검증

- 구문 검사(`cl.exe /Zs`, 프로젝트 include·`/std:c++20`·`/FI CppStandardPch.h`, Client는 `UNICODE` 정의 포함): `ActorCatalog.cpp`, `VehiclePresentationAssetService.cpp`, `PlayerController.cpp`, `GameRoom_VehicleRiding.cpp` 모두 오류 0. 링크·전체 빌드가 아니다. 로그 `out/ShipFix20260925/logs/cl_zs_*.log`.
- `python -m py_compile` 두 도구 통과. `VehicleCatalog.json` parse 통과.
- `Server.exe --navigation-contract-test`(22:29 빌드본): 34 PASS / 실패 0 (`logs/navigation_test.txt`).
- `Server.exe --vehicle-riding-contract-test`(22:29 빌드본, 서버 상수는 아직 9.3인 옛 바이너리): 69 PASS / 실패 8 (`logs/vehicle_riding_test_prebuild.txt`). 실패 8건은 전부 배 탑승 연쇄(탑승 속도, 부두 위치 보존, 바다로 이동, 바다 클릭 경로, 배 교체 2건, 하차 속도, 재탑승)이고 원인 줄은 `[ShipBoard] ... refused: no open sea within 100 m`다. 옛 바이너리는 바다 셀을 9.3±0.6 m에서 찾는데 게시된 격자는 10.95 m라 탑승이 거부된다. 즉 빌드 전 상수 불일치가 원인이며 새 격자·데이터의 결함 증거가 아니다. 빌드 뒤 재실행이 필요하다(6절).
- `git diff --check` 공백 오류 없음. 렌더링 보호 파일(`Renderer.*`, `ShaderFiles`, `Data/Rendering`, `DataFiles/Rendering`) 변경 0.

## 6. 빌드 뒤 확인이 필요한 것

- 서버 상수 10.95는 빌드 전에는 테스트에 반영되지 않는다. 빌드 뒤 `Server\Bin\Debug\Server.exe --vehicle-riding-contract-test`를 다시 돌려 77 PASS / 실패 0인지 확인한다.
- Client 파서 변경(`modelLiftMeters`)은 빌드 뒤 Bern 진입 시 카탈로그 로드가 성공해야 한다. 실패하면 `[Client][Character] Vehicle ... presentation isolated` 로그가 남는다.

## 7. 사용자가 화면에서 볼 순서

1. Debug Server와 Client를 다시 시작한다(둘 다 이번 빌드본).
2. Lobby → Bern → Bern3 부두의 조선공 NPC에게 말을 걸어 배를 골라 탑승한다.
3. 바다 위에서 배를 옆에서 본다. 선체 바닥이 수면보다 약간(0.15 m) 위에 있어야 한다. 잠겨 있으면 얼마나 잠겼는지(갑판까지인지, 바닥만인지)와 배 종류를 알려 주면 `modelLiftMeters`나 `SHIP_DRAFT_M`을 조정한다. 반대로 너무 떠 있으면 `SHIP_DRAFT_M`을 0에 가깝게 올린다.
4. 바다를 우클릭해 이동한다. 클릭 표시가 수면에 찍히고 배가 그 지점으로 가야 한다(클릭 평면을 뿌리 − 0.15로 바꿨다).
5. F1 → `Show Navigation`으로 바다 격자가 0.5 m로 촘촘해졌는지 본다(전보다 셀이 4배 많다).
6. 부두로 돌아와 하차(R)하면 부두 위에 서야 한다.

## 8. 되돌리는 방법

- 코드·카탈로그·도구 원본: `out/ShipFix20260925/backup/`(파일별 사본). 네비 원본과 게시본(1.0 m, 9.3)도 같은 폴더.
- 네비만 되돌리려면 `build_sea_nav.py`의 `SHIP_DRAFT_M`과 `CELL`을 원래대로 두고 `--write` 후 `Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_BER_BERNCASTLE`. 서버 상수도 함께 되돌려야 한다.

## 9. 사용자 결정이 필요한 것

- 목표 높이 0.15 m(수면 위 여유)는 이 문서가 정한 값이다. 원작처럼 선체가 조금 잠긴 모습을 원하면 `SHIP_DRAFT_M`을 0.2~0.4로 두고(뿌리가 수면 아래) 서버 상수와 함께 바꾸면 된다. 그 경우 배별 `modelLiftMeters`는 그대로 두는 편이 낫다(원점이 용골로 통일된 상태).
- 원작 마하라카·베른 바다에서 실제 흘수가 얼마인지는 이 저장소에 근거가 없어 확인하지 못했다.

## 10. 최종 재확인

확인한 것: 배 9종 정점 범위 실측, Client 부착 경로(부모 행렬·seatOffset 의미)와 Engine 사전 변환이 본에 적용됨을 코드로 확인, 패치 8개 파일 앵커 1회 매치 적용, 구문 검사 4파일 오류 0, 네비 재생성 수치, Bern Area Validate/Publish 성공, 사용자 영역 게시본 hash 동일, navigation 테스트 34 PASS, 렌더링 보호 파일 무변경, 시작/끝 git status 차이로 변경 파일 14개 확인.
확인하지 못한 것: 화면에서 배가 실제로 수면 위에 보이는지, 빌드(링크) 성공 여부, 빌드 뒤 vehicle-riding 테스트, 정적 맵 모델(정박 배·물 평면)의 정점 범위, 원작 흘수.

## 빌드 뒤 확인과 정정 (조정자, 2026-09-25 22:51~23:02)

- 2차 Debug Product Build(22:51~22:55, PASS, 248초): Server OBJ 1개, Client OBJ 91개(`ActorCatalog.h` 변경으로 포함 파일 재컴파일), 셰이더 0, 오류 0. `Server.exe` 22:51:37, `Client.exe` 22:55:38 갱신. 결과 `out/BuildPipeline/runs/20260925T135541881Z-debug-product.json`, 로그 `out/FinalBuild20260925/product_debug_build_2.log`.
- 새 서버로 `--vehicle-riding-contract-test` 1차 재실행(22:56): 75 PASS / 실패 2. 빌드 전 8건 중 6건은 사라졌고(탑승·속도·복귀·재탑승 정상, `[ShipBoard] ... sea=(267.454, 10.95, -217.695)`), 남은 2건("Boarding a ship carries the player out onto open sea navigation, away from the pier", "A click on the water finds a path on the sea region from the departure point")의 원인은 **테스트 파일 `Server/Private/ServerGameplayContractTests_VehicleRiding.cpp` 358행의 상수 `SEA_LEVEL = 9.3f`**였다. 서버가 배를 10.95 m에 두므로 `|seaY - 9.3| < 0.01` 검사와 경로 목표 높이가 어긋났다. 이 fork(D)의 변경 범위에 이 테스트 파일이 빠져 있었던 누락이며 코드 결함이 아니다. 결과 `out/FinalBuild20260925/vehicle_riding_after_build.txt`.
- 정정: 조정자가 358행을 `SEA_LEVEL = 10.95f`로 한 줄 수정(UTF-8 BOM·CRLF 보존, 백업 `out/ShipFix20260925/backup/ServerGameplayContractTests_VehicleRiding.cpp.before-sea-level`). 서버 상수 `SHIP_SEA_NAV_LEVEL_Y`와 같은 값이며, 두 값과 `build_sea_nav.py`의 `SEA_SURFACE_Y - SHIP_DRAFT_M`은 함께 바꿔야 한다.
- 3차 Debug Product Build(22:59, PASS, 7초): Server OBJ 1개(`ServerGameplayContractTests_VehicleRiding.obj`)와 링크만. `Server.exe` 22:59:15. 결과 `20260925T135919113Z-debug-product.json`.
- `--vehicle-riding-contract-test` 2차 재실행(새 서버): **77 PASS / 실패 0**(23:00:02~23:01:26, rc=0). 이전 2건 모두 PASS. 결과 `out/FinalBuild20260925/vehicle_riding_after_build_2.txt`.
- 화면 확인(배 선체가 수면 위인지, 0.5 m 격자)은 여전히 사용자 몫이다.

SHIP_FIX_DONE
