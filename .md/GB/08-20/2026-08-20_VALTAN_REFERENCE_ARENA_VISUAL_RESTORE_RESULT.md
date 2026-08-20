# 발탄 참조 사진 기반 아레나 시각 복구 결과

> 기준일: 2026-08-21 KST
>
> 사용자 입력: 탑다운 아레나 1장, 외곽 성 전경 1장, 상부 탑 분리 회귀 실행 화면 1장
>
> 판정 경계: tower 데이터 sync/publish와 C++ Debug/Release build/focused harness 검증 완료, 사용자 Client 육안 검증 미완료

## 실제 완료 상태

이번 변경은 기존 발탄 Server 패턴, 벽/바닥 파괴, collision/navigation과 다른 담당자의 dirty
변경을 유지한 채 다음 항목을 반영했다.

1. 처음 메인 HeartRB placement에 병합했던 Landscape component 6개가 사용자 실행 화면에서
   검은색·갈색 판으로 바닥과 폐허를 덮는 회귀를 일으킨 사실을 확인했다. 이 6개만 즉시
   제거해 `275 assets / 13,186 source placements` baseline을 복구했다. 잘못된 철탑 phase
   registration overlay 188개도 제거한 현재 authoring/runtime은 모두
   `275 assets / 13,186 placements`이며, 메인 Landscape 행은 0개다. 별도 Landscape 추출
   문서와 WModel은 조사 자료로만 유지된다.
2. `scene.valtan.cool-low-key.v1`을 기준 사진에 가까운 청회색 저조도 시작값으로 조정했다.
   노출 `1.55`, bloom 배수 `0.85`, ambient `[0.16,0.21,0.28]`이며 shadow focus는 실제
   아레나 중심 `[156,24,-122]`다.
3. Debug Client의 기존 F1 발탄 audition panel에 다음 버튼을 추가했다.
   - `Reference Top Down`
   - `Reference Exterior (Arena Towers)`
   - `Restore Camera`
   사용자 1280px 화면의 오른쪽 약 160px 패널에서 기존 180px 버튼 두 개를 `SameLine`으로
   놓아 Exterior가 화면 밖으로 잘린 회귀도 확인했다. 세 버튼은 이제 각자 한 행에서 현재
   content 폭을 사용하므로 좁은 패널에서도 Exterior를 누를 수 있다.
4. `Reference Top Down`은 기존 `VALTAN_PHASE_SPACEHOLE` proxy 3개를 stable sourceLevel로
   표시한다. 다른 기준 뷰, Restore, F6, Server cinematic, disconnect와 level exit에서는 각
   placement의 authored `visible=false`로 복원한다.
5. 기준 카메라는 기존 `CCamera_Free` presentation override만 사용한다. Server 상태, packet,
   Valtan action, player transform, destruction state는 변경하지 않는다.
6. 사진에서 파란색 1~4로 표시한 높은 탑은 SL04 source 조립체의 상부, 하부 받침과 체인이
   원본 transform에서 함께 맞물려야 한다. 이전 구현은 후방 4 station의 상부 47개씩, 총
   188개만 `+10.6108742m` 이동하고 하부와 체인을 원본 위치에 남겼다. 2026-08-21 사용자 실행
   화면에서 이 두 부분 사이에 약 `11.54m`의 수직 seam이 생겨 상부 첨탑이 공중에 분리된 회귀를
   확인했다. 이를 되돌려 source 188개는 original transform과 `visible=1`을 유지하고,
   `VALTAN_TOWER_REGISTERED` overlay는 0개로 만들었다. tower를 숨기는 environment override도
   0개이며 전체 override는 기존 atmosphere 2개뿐이다. core overlay는 기존 phase proxy 6개만
   남는다.
7. 후방 4 station과 전방 우측 `pointlight_11`의 point light는 모두 source 위치
   `Y=24.734033m`를 유지한다. maplight provenance는
   `SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED`이고 전체 point-light 수는 22개다.

## 변경 파일

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements`
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.maplights.json`
- `Data/Maps/MapCatalog.json`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.maplights.json`
- `Tools/LevelPlacementExtractor/heartrb_valtan_tower_phase_registration.json`
- `Tools/LevelPlacementExtractor/sync_valtan_tower_phase_registration.py`
- `Tools/LevelPlacementExtractor/test_sync_valtan_tower_phase_registration.py`
- `Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json`
- `Tools/LevelPlacementExtractor/heartrb_environment_runtime.json`
- `Data/Rendering/Authored/RenderingProfiles.json`
- `Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`
- `Client/Public/MapPlacementRuntime.h`
- `Client/Private/MapPlacementRuntime.cpp`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `.md/GB/07-30/2026-07-30_LOSTARK_HEARTRB_EXACT_STATIC_MAP_RECONSTRUCTION_PLAN.md`
- `.md/GB/08-20/2026-08-20_VALTAN_REFERENCE_ARENA_VISUAL_RESTORE_RESULT.md`

`Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`에는 이 작업 전부터 다른 발탄
playlist 변경이 있었다. 이 작업은 reference camera focused test만 좁게 추가했고 기존 hunk를
되돌리거나 재작성하지 않았다.

## 자동 검증 증거

- `test_sync_valtan_tower_phase_registration.py`: `6/6 PASS`
  - station/asset/source ID drift 거부
  - source 188개 original transform / visible 복원과 registration overlay 0개
  - rear 4개 및 `pointlight_11` light의 source Y/provenance 유지
  - 중간 commit 실패 rollback과 check-only stale 거부
- `test_build_maptool_scene.py`: `16/16 PASS`
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED`:
  `275 assets / 13,186 placements` PASS
- authoring/runtime placement header: 각각 `13,186`
- 복구 결과: rear source original/visible 188개, registered overlay 0개, tower visibility override
  0개, core phase proxy 6개, 전체 environment override 2개

이번 source attachment 복구 뒤 다음 C++ 검증을 완료했다.

- `PointLightFalloffContractHarness`: Debug/Release build 및 실행 PASS
- `ClientFrontendHarness`: Debug/Release build PASS, 각 configuration을 `Client/Default`에서
  `--valtan-camera-fast`로 실행해 각각 `45/45 PASS`, `failures 0`, exit 0
- 전체 Client: Debug/Release build PASS

Client/UI는 에이전트가 실행하지 않았으며 visual fidelity는 사용자 화면 확인 대기 상태다.
아직 수행하지 않은 사용자 육안 검증을 PASS로 기록하지 않는다.

## 원본 atmosphere 조사와 남은 경계

현재 atmosphere baseline은 원본 CloudPlane 2개와 map point light 22개 구성이다. 후방 tower
light 4개를 포함해 22개 모두 source 위치와 provenance를 유지한다.
사진의 두꺼운 구름바다와 탑의 가시 불꽃은 이것만으로 완전히 같아지지 않는다.

- SL04 `par_c_ul_volumecloud_01` 27개의 component transform과 alpha/color/spawn 입력은 복구할
  수 있다.
- 그러나 원본 cooked Material은 35 expression 중 distortion scalar 하나만 남은
  `COOKED_PARTIAL`이고 diffuse/emissive/opacity topology가 없다.
- 현재 strict Effect runtime에도 `VOLUME_CLOUD` evaluator가 없다.
- q_firesh 핵심 texture 3개는 exact hash로 있지만 3-layer 중 dissolve/basic material 2개의 strict
  evaluator가 없다.

따라서 이번 변경에는 임의 cloud/fire quad, 대체 texture, generic translucent material 또는 빈
`mapambientfx` loader를 추가하지 않았다. 이 atmosphere 항목은 이번 tower sync/publish 검증 범위에
포함하지 않았으며, 후속은 strict shader/evaluator와 texture cooker receipt를 먼저 닫은 뒤
world-owned persistent Effect runtime으로 진행해야 한다.

## 사용자가 직접 확인할 절차

1. Debug Server가 `127.0.0.1:7777`에서 실행 중인지 확인한다.
2. Visual Studio에서 Client를 `Ctrl+F5`로 실행한다.
3. Lobby에서 `Valtan`을 선택해 발탄 아레나에 들어간다.
4. `F1`을 눌러 Developer Tools의 발탄 audition 영역을 연다.
5. 세로로 보이는 `Reference Exterior (Arena Towers)`를 눌러 두 번째 사진과 외곽 성, 다리,
   파란색 1~4 위치의 후방 탑을 비교한다. 특히 네 상부 첨탑이 하부 받침과 체인에 끊김 없이
   다시 붙었는지 확인한다. 같은 계열 다섯째 station도 source 위치를 유지한다.
6. `Reference Top Down`을 눌러 첫 번째 사진과 원형 바닥, 외벽, 중앙 SpaceHole 구도를 비교한다.
7. `Restore Camera` 또는 `F6`으로 원래 follow/free camera로 돌아오는지 확인한다.
8. 색이 너무 어둡거나 파랗거나, 구도가 잘리거나, 검은색·갈색 판이 다시 보이면 같은 위치의
   스크린샷을 보내 다음 수치 조정을 진행한다.

## 수동 판정 상태

- 에이전트 Client/UI 자율 실행: 하지 않음
- 2026-08-21 분리 회귀 screenshot 확인: 완료
- source attachment 복구 뒤 사용자 screenshot 재확인: 대기
- visual fidelity PASS: 미판정
- 사진과 완전히 동일하다는 주장: 하지 않음

현재 자동으로 확정할 수 있는 완료선은 잘못 병합된 Landscape 6개가 없는 baseline 위에
188 source original/visible, registration overlay 0개, source light Y와 provenance를 동기화하고
`275 assets / 13,186 placements`로 publish한 데이터 계약, PointLight Debug/Release 검증,
Client Debug/Release build와 ClientFrontend Debug/Release focused harness까지다. 외곽 탑의 최종
attachment 화면, 원본 particle atmosphere 및 최종 카메라/색 수치는 사용자 화면 재확인과 별도
source 대조로 이어서 조정한다.
