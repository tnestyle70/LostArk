# 발탄 참조 사진 기반 아레나 시각 복구 결과

> 기준일: 2026-08-20 KST  
> 사용자 입력: 탑다운 아레나 1장, 외곽 성 전경 1장  
> 판정 경계: 자동 구조 검증 완료, 사용자 Client 육안 검증 미완료

## 실제 완료 상태

이번 변경은 기존 발탄 Server 패턴, 벽/바닥 파괴, collision/navigation과 다른 담당자의 dirty
변경을 유지한 채 다음 항목을 반영했다.

1. 메인 HeartRB placement에 누락됐던 원본 Landscape component 6개를 복구했다.
   authoring과 runtime은 모두 `275 assets / 13,192 placements`이며 Landscape 행은 6개다.
2. `scene.valtan.cool-low-key.v1`을 기준 사진에 가까운 청회색 저조도 시작값으로 조정했다.
   노출 `1.55`, bloom 배수 `0.85`, ambient `[0.16,0.21,0.28]`이며 shadow focus는 실제
   아레나 중심 `[156,24,-122]`다.
3. Debug Client의 기존 F1 발탄 audition panel에 다음 버튼을 추가했다.
   - `Reference Top Down`
   - `Reference Exterior`
   - `Restore Camera`
4. `Reference Top Down`은 기존 `VALTAN_PHASE_SPACEHOLE` proxy 3개를 stable sourceLevel로
   표시한다. 다른 기준 뷰, Restore, F6, Server cinematic, disconnect와 level exit에서는 각
   placement의 authored `visible=false`로 복원한다.
5. 기준 카메라는 기존 `CCamera_Free` presentation override만 사용한다. Server 상태, packet,
   Valtan action, player transform, destruction state는 변경하지 않는다.

## 변경 파일

- `Tools/LevelPlacementExtractor/merge_maptool_landscape.py`
- `Tools/LevelPlacementExtractor/test_merge_maptool_landscape.py`
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements`
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

### Landscape

- `test_merge_maptool_landscape.py`: `13/13 PASS`
- Python compile: PASS
- 병합 전 check: `0 assets / 6 placements` 추가 예정
- 병합 후 재실행: `0 / 0`, 멱등 PASS
- Map publisher: `275 assets / 13,192 placements` PASS
- source/runtime placement SHA-256:
  `9CFE16E9...DB754`, 양쪽 동일
- Landscape WModel 6개와 diffuse/normal texture 12개 존재

08-16 붕괴 바닥 6개는 Deploy runtime에 그대로 있고 제거했던 map overlay는 복원하지 않았다.
따라서 Landscape 복구가 파괴 바닥을 이중 렌더하지 않는다.

### Rendering profile

- `Publish-RenderingProfiles.ps1 -Mode Validate`: PASS
- `Publish-RenderingProfiles.ps1 -Mode Publish`: PASS
- authoring과 runtime의 Valtan profile 값 일치

### Reference camera

- ClientFrontendHarness x64 Debug build: PASS
- `ClientFrontendHarness --valtan-camera-fast`: `24/24 PASS`, failures 0
- Client x64 Debug 전체 build/link: PASS
- ClientFrontendHarness x64 Release build: PASS
- Release `ClientFrontendHarness --valtan-camera-fast`: `24/24 PASS`, failures 0
- Client x64 Release 전체 build/link: PASS
- targeted `git diff --check`: PASS, 기존 line-ending 안내만 존재

Release에는 Debug reference UI와 state가 컴파일되지 않으며, 기존 C4819/C4828 인코딩 및
DirectXTK LNK4099 PDB 경고만 있고 오류는 없다.

## 원본 atmosphere 조사와 남은 경계

현재 source-exact baseline은 원본 CloudPlane 2개와 map point light 22개다. 사진의 두꺼운
구름바다와 탑의 가시 불꽃은 이것만으로 완전히 같아지지 않는다.

- SL04 `par_c_ul_volumecloud_01` 27개의 component transform과 alpha/color/spawn 입력은 복구할
  수 있다.
- 그러나 원본 cooked Material은 35 expression 중 distortion scalar 하나만 남은
  `COOKED_PARTIAL`이고 diffuse/emissive/opacity topology가 없다.
- 현재 strict Effect runtime에도 `VOLUME_CLOUD` evaluator가 없다.
- q_firesh 핵심 texture 3개는 exact hash로 있지만 3-layer 중 dissolve/basic material 2개의 strict
  evaluator가 없다.

따라서 이번 변경에는 임의 cloud/fire quad, 대체 texture, generic translucent material 또는 빈
`mapambientfx` loader를 추가하지 않았다. 자동 extraction/material/resource/cook 검사 16개는
PASS했다. 후속은 strict shader/evaluator와 texture cooker receipt를 먼저 닫은 뒤 world-owned
persistent Effect runtime으로 진행해야 한다.

## 사용자가 직접 확인할 절차

1. Debug Server가 `127.0.0.1:7777`에서 실행 중인지 확인한다.
2. Visual Studio에서 Client를 `Ctrl+F5`로 실행한다.
3. Lobby에서 `Valtan`을 선택해 발탄 아레나에 들어간다.
4. `F1`을 눌러 Developer Tools의 발탄 audition 영역을 연다.
5. `Reference Exterior`를 눌러 두 번째 사진과 외곽 성, 다리, 탑, 구름바다 구도를 비교한다.
6. `Reference Top Down`을 눌러 첫 번째 사진과 원형 바닥, 외벽, 중앙 SpaceHole 구도를 비교한다.
7. `Restore Camera` 또는 `F6`으로 원래 follow/free camera로 돌아오는지 확인한다.
8. 색이 너무 어둡거나 파랗거나, 구도가 잘리거나, Landscape가 겹쳐 보이면 같은 위치의
   스크린샷을 보내 다음 수치 조정을 진행한다.

## 수동 판정 상태

- 에이전트 Client/UI 자율 실행: 하지 않음
- 사용자 screenshot 비교: 대기
- visual fidelity PASS: 미판정
- 사진과 완전히 동일하다는 주장: 하지 않음

현재 자동으로 확정할 수 있는 완료선은 누락 Landscape 복구, 냉색 profile publish, Debug 기준
카메라와 proxy visibility transaction까지다. 원본 particle atmosphere와 최종 카메라/색 수치는
사용자 화면 확인 뒤 이어서 조정한다.
