# 발탄 원본 컷신 — Map Tool Camera Shots 재배치 결과

작성일: 2026-09-17. 범위: 잘못 넣은 Camera Tool 문서 원복 + Map Tool `Camera Shots` 문서 신규 작성.

## 왜 옮겼나

사용자 목적은 팀장이 **F1 → Map Tool → Area: Valtan → Camera 탭의 `Camera Shots` 목록**에서
원본 컷신을 이름으로 알아보고, 골라서 미리보기·편집하고, 데이터를 가져가는 것이다.
앞선 작업은 원본 컷신을 `Data/Encounters/Valtan/ValtanCinematicCamera.json`(별개 도구인
`Camera Tool` 이 읽는 문서)에 넣어 목적지를 잘못 잡았다. Map Tool Camera 탭은 그 문서를
읽지 않는다.

## 걷어낸 것 (작업 A)

`git checkout` 으로 HEAD 복원한 6개:

| 파일 | 되돌린 내용 |
|---|---|
| `Data/Valtan/Valtan.gameplay.json` | `VALTAN_SOURCE_PREVIEW_*` 패턴 5개 |
| `Data/Valtan/Valtan.presentation.json` | 같은 패턴 5개의 presentation |
| `Data/Encounters/Valtan/ValtanEncounter.json` | 투영본 패턴 70 → 65 |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | `DORMANT_PREVIEW_PATTERN_PREFIX` 예외 |
| `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | PublishV2 부작용 182줄 (`valtan.source-preview.*`) |
| `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | PublishV2 부작용 (`patterns.length` 65↔70) |

뒤의 2개는 지시에 명시되지 않았으나 세션 시작 시 clean 이었고 diff 내용이 각각
`valtan.source-preview.*` 추가와 `patterns.length` 70 한 건뿐이어서 같은 PublishV2 의
부작용임을 확인하고 함께 되돌렸다.

`Data/Encounters/Valtan/ValtanCinematicCamera.json` 은 백업 `...prepreview` 로 복원해
**cue 13개만 제거하고 deathCue 교체(23000ms/키 28)는 남겼다**. 그 변경은 별개 작업이다.

검증: `Project-ValtanPatternMaster.ps1 -Mode Validate` → `"ok": true`.

## 새로 넣은 것 (작업 B)

신규 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json`
(`lostark.camera-shots` v1, areaId `LV_LUT_HEARTRB_ED`, revision 1, 샷 13개 / 키 91개).

원본 정본은 `out/ValtanCameraReplace20260917/ORIGINAL_CAMERAS_V2.json` 이고
`keySamples[].eye` / `lookAt15m` 은 이미 클라 좌표(m), `fovY16x9` 는 이미 수직 FOV(도)라
그대로 사용했다. `isGameplayView` 인 dirgroup 컷은 카메라가 아니므로 제외했다.

| shotId | displayName | 길이 | 키 |
|---|---|---|---|
| `source.gate1-entrance.cut01` | 원본 1관문 입장 (0.0~13.0초) | 13000ms | 8 |
| `source.entrance.cut01` | 원본 발탄 등장 1/3 (0.0~13.1초) | 13059ms | 5 |
| `source.entrance.cut02` | 원본 발탄 등장 2/3 (13.1~19.0초) | 5907ms | 12 |
| `source.entrance.cut03` | 원본 발탄 등장 3/3 (19.0~23.4초) | 4420ms | 2 |
| `source.finale.cut01` | 원본 발탄 최후 1/4 (0.0~3.6초) | 3650ms | 2 |
| `source.finale.cut02` | 원본 발탄 최후 2/4 (3.6~7.8초) | 4130ms | 4 |
| `source.finale.cut03` | 원본 발탄 최후 3/4 (7.8~15.4초) | 7630ms | 23 |
| `source.finale.cut04` | 원본 발탄 최후 4/4 (15.4~23.0초) | 7590ms | 2 |
| `source.trash.cut01` | 원본 버러지 1/2 (0.0~1.1초) | 1073ms | 6 |
| `source.trash.cut02` | 원본 버러지 2/2 (1.1~5.7초) | 4663ms | 13 |
| `source.roar.cut01` | 원본 발탄 포효 1/3 (0.0~1.5초) | 1461ms | 6 |
| `source.roar.cut02` | 원본 발탄 포효 2/3 (1.5~3.6초) | 2160ms | 5 |
| `source.roar.cut03` | 원본 발탄 포효 3/3 (3.6~5.0초) | 1379ms | 3 |

### 저절로 재생되지 않게 한 처리

**13개 전부 `activation: "PATTERN_ONLY"`** 다. `AUTO` 로 두면 플레이어가 박스에 들어갈 때
실제로 재생되어 09-16 팀장 보류 결정을 어긴다. `Apply_CutsceneCameraTrack`
(`MapTool_Cutscenes.cpp:46`)이 `shot.patternOnly && !editingOneShot` 이면 건너뛴다.

`box` 는 파서 필수 필드이나 PATTERN_ONLY 라 발동 판정에 쓰이지 않는다. 우연한 진입을
막으려고 각 샷 첫 키 위치에 `halfExtents [0.25, 0.25, 0.25]` 로 아주 작게 두었다.

### 원본과 다른 점

없다. FOV 는 Map Tool 로드 경로에 하한이 없어(`fovYDegrees` 가 수치이기만 하면 통과)
**원본 값을 보정 없이 그대로 넣었다** — 포효 2/3 의 최소 9.6107도 포함. 롤은 이 형식에
필드가 없어 재현되지 않는다(사용자가 이미 롤 포기를 선택).

## MapCatalog 선언

Map Tool 이 읽는 데는 선언이 **필요 없다**. `MapTool_Area.cpp:356` 이 areaId 규칙으로
`Maps/Authoring/<AreaId>/<AreaId>.camerashots.json` 경로를 직접 조립한다.

그러나 `Publish-MapAuthoring.ps1` 이 *선언 없는 저작 소스* 를 거부한다
(`Camera shot source exists without a MapCatalog declaration`). 그래서 `MapCatalog.json`
발탄 항목에 `sourceCameraShots` / `cameraShots` 2줄을 쿠크와 같은 자리에 추가했다.

이것이 팀장 보류를 어기지 않는 근거: **발탄 Level 에는 카메라샷 소비자가 없다.**
`Level_ValtanArena.cpp` / `.h` 의 `CameraShot` grep 결과 0건(쿠크는 104건). 09-16 에
해당 C++ 을 전부 원복했기 때문이다. 즉 게시하더라도 런타임이 읽지 않는다.

## 검증 (실제 실행한 것만)

- Map Tool 로드 코드(`MapTool_CameraShots.cpp:596-780`) 규칙 전수 재현 검사 → 실패 0건.
  헤더 3종, 필수 필드(`shotId`/`sequenceInstanceId`/`box.center`/`box.halfExtents`/`eye`/`lookAt`),
  선택 필드 범위(`displayName` ≤128, `defaultHoldMs` 0~600000 정수, `activation` 열거,
  `transitionEasing` 열거), `cameraTrack` 4필드, 키 시각 강한 증가·첫 키 0·끝 키 = durationMs,
  `sceneId` 유일, `eye != lookAt` 전부 통과.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Mode Validate` → 종료 0,
  **FileCount 23** (09-16 기록의 "카메라샷 포함 시 23" 과 일치, 미포함일 때는 22).
- `Project-ValtanPatternMaster.ps1 -Mode Validate` → `"ok": true`.
- `git diff --check` 통과. Valtan 패턴/투영/파이프라인/바인딩/영수증 5개가 HEAD 와 동일함을 확인.

미실시: 빌드(작업 내내 `devenv.exe` PID 23600 실행 중), Client 실행, 화면 확인, commit/push.

## 사용자 확인 경로

`F1 → Map Tool → Area: Valtan → Camera` 탭 → `Camera Shots` 목록.
13개가 `원본 …` 이름으로 뜬다. 샷을 고르면 그 상세 안에서 `Keys: N` 아래
`Render_CameraTrackTimeline` 이 **무조건 호출**되므로(`MapTool_CameraShots.cpp:1121`)
타임라인이 열리고 키를 드래그로 재배치할 수 있다.

**빌드 불필요.** C++ 은 한 줄도 바꾸지 않았고 데이터만 추가했다. Map Tool 에서
`Reload Shots` 를 누르거나 Area 를 다시 선택하면 읽힌다.

## 남은 경계

- 아레나 안 카메라 미리보기(`Apply_CutsceneCameraTrack`)는 `sequenceInstanceId` 를
  `m_ArenaRisePlayer` 가 알아야 시계가 돌아간다. 발탄에는 대응 World Sequence 가 없어
  `world.sequence.instance.valtan.source-preview.<slug>` 는 실재하지 않는 ID다.
  **목록 표시와 타임라인 편집은 되지만, 그 경로를 통한 아레나 재생은 확인되지 않았다.**
  화면 동작은 사용자가 직접 판정해야 한다.
- `ValtanCinematicCamera.json` 의 deathCue 는 여전히 원본 23초로 교체된 상태다.
  전투 중 보스 사망 시 재생되므로 되돌리려면
  `git checkout -- Data/Encounters/Valtan/ValtanCinematicCamera.json`.
