# 2026-09-26 마하라카 복원 G00 — 실행 작업본 고정 결과

설계서 `.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md`의 G00을 수행한 기록이다.
이 단계에서 코드·데이터·Resources·실행 상태를 변경하지 않았다.

## 1. 실제 바꾼 것

없다. 새 후보 폴더 하나만 만들었다.

`out/MaharakaContinuation_20260926_193455/`

기존 `out/MaharakaReaudit20260926`, `out/MaharakaFunctions20260926`과 모든 백업을 건드리지 않았다.

## 2. 실행 checkout 실측

| 항목 | 값 |
|---|---|
| 저장소 | `C:\Users\USER\source\졸업팀폴\LostArk` |
| branch | `codex/main-ship-maharaka-0926` |
| HEAD | `a84bbcd45ff995c70bf847c683f1d159cf1821a6` |
| origin/main | `e234827fa72519cc4b9f9434012551b17363e524` |
| `git status --short` 항목 수 | 161 |
| `git diff --stat` | 75 files changed, 10356 insertions(+), 121 deletions(-) |

origin/main이 HEAD보다 앞서 있다. 설계서 조사 시점과 같은 상태다. 이 복원 작업을 이유로
pull·rebase·reset을 하지 않았고 이전 작업본으로 강제 되돌리지도 않았다.

`C:/Users/USER/.codex/worktrees/7395/LostArk`는 이번 실행 작업본이 아니다. 거기서 빌드하지 않는다.

## 3. 보호 대상 (미커밋 변경)

같은 작업본에 다른 작업의 미커밋 변경이 섞여 있다. 전부 보존한다.

- 선박: `Data/Vehicles/VehicleProfiles.json`, `Data/Camera/Bern.camera.json`,
  `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`,
  `Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap`,
  `Server/Private/ServerGameplayContractTests_VehicleRiding.cpp`
- 베른/UI/NPC: `Client/Private/Level_Bern.cpp`, `MainApp.cpp`, `Character.cpp`,
  `ClientReplication.cpp`, `PlayerController.cpp`, `NpcPresentationAssetService.cpp`,
  `Data/Actors/NpcCatalog.json`, `.md/TEAM/NPC_OWNER_HANDOFF.md`
- 마하라카 기존 설치: `Data/Maps/Imported|Authoring/LV_OCN_EVENTIS_MHP/*`,
  `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.*`, `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`
- 문서: `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/렌더링이펙트복원V2.md`,
  `Tools/LevelPlacementExtractor/README.md`, `Tools/WorldPipeline/Publish-WorldGameplay.ps1`

`reset`, `checkout --`, 전체 덮어쓰기, 자동 stash, `git add .`, 자동 merge를 하지 않는다.
같은 함수가 이미 수정되어 있으면 현재 diff를 읽어 보존한다.

## 4. 프로세스 상태

| 프로세스 | 상태 |
|---|---|
| `Client.exe` | 없음 |
| `Server.exe` | 없음 |
| `devenv.exe` | **실행 중 (pid 30616)** |

Visual Studio가 열려 있다. 같은 작업 폴더에서 VS와 자동화 빌드를 겹쳐 실행하지 않는다는
규칙에 따라 G01~G04의 조사·데이터 구간에서는 빌드를 돌리지 않는다. G11의 정본 Product 빌드가
필요해지면 그 시점에 사용자에게 VS 상태를 확인하고 진행한다.

## 5. 기존 설치 검증

```
python -B out/MaharakaReaudit20260926/verify_installed.py
PASS: 32 installed terrain files/backups, 2 models/6 material rows/textures;
      all prior NPC and world placements unchanged
EXIT=0
```

이 검사는 09-26 설치 직후 snapshot 기준이다. NPC 마지막 두 행·world 총 6배치·과거 NPC 행
불변을 assert하므로 이후의 정당한 변경에도 실패할 수 있다. 현재는 통과했고, 이후 검증은
stable ID와 해당 작업의 변경 전후 값을 기준으로 작성한다. 이 옛 검사를 통과시키려고 새 NPC나
배치를 삭제하지 않는다.

## 6. 세 흐름의 현재 상태 실측

### 6.1 action 4225601 흐름

| 입력 | 상태 |
|---|---|
| `Tools/LevelPlacementExtractor/extract_action_effect_notifies.py` | 있음 |
| `out/MaharakaReaudit20260926/MN_ISMP_00.Action.loa` | 있음 |
| `out/MaharakaReaudit20260926/MokoAction/MN_ISMP_00.action-effects.json` | 있음 |
| `Client/Private|Public/NpcActionEffectCueDocument.*` | 있음 |
| 마하라카 `npcactioncues` 저작 문서 | **없음** |

`Data/Effects/NpcActionCues/`에는 `NPC_58700`, `NPC_59030`, `NPC_59060`, `NPC_59504`,
`NPC_59620` 5개만 있다. 모코모코·워터캐논 문서는 아직 만들어지지 않았다.

### 6.2 워터팡 무대 흐름

조사 입력이 모두 존재한다.

| 파일 | 크기 |
|---|---|
| `out/MaharakaReaudit20260926/prop-table-57009.json` | 12,703 bytes |
| `out/MaharakaReaudit20260926/prop-table-57011.json` | 18,595 bytes |
| `out/MaharakaReaudit20260926/audit_props.py` | 1,241 bytes |
| `out/MaharakaReaudit20260926/archive_index.json` | 28,559,314 bytes |
| `out/MaharakaReaudit20260926/IstmAction/MN_ISTM_00.action-effects.json` | 4,749,998 bytes |

모델 정체성은 미확정이다. `MN_ISTM_00`은 `MN_Empty_00_SK`를 쓰는 controller이므로 무대
모델로 쓰지 않는다.

### 6.3 렌더링 흐름

설계서 G03-01의 수치가 현재 코드·데이터와 일치한다.

| 항목 | 베른 | 마하라카 |
|---|---|---|
| `LevelRegistry.cpp` scene profile | `scene.bern.neutral-day.v1` (146행) | `scene.development.neutral.v1` (193행) |
| runtime mapmaterials 행 | 23,153 | 329 |
| 그중 `bakedLighting` 보유 | 21,321 | 0 |

이 수치는 문서 행 수이고 채워야 할 목표가 아니다. 마하라카 component의 원본 입력을 찾는다.

## 7. 실행한 명령과 결과

| 명령 | 결과 |
|---|---|
| `git branch --show-current` / `rev-parse HEAD` / `rev-parse origin/main` | 위 2절 값 |
| `git status --short`, `git diff --stat` | 161 항목 / 75 files, +10356 −121 |
| `Get-CimInstance Win32_Process` (Client/Server/devenv) | devenv pid 30616만 실행 |
| `New-Item` 후보 폴더 | `out/MaharakaContinuation_20260926_193455` 생성 |
| `python -B out/MaharakaReaudit20260926/verify_installed.py` | PASS, exit 0 |

## 8. 사용자 확인이 필요한 항목 (아직 진행하지 않음)

G05의 마하라카 전용 scene profile은 `Data/Rendering/Authored/RenderingProfiles.json`과
`Publish-RenderingProfiles.ps1`을 건드린다. 이 두 경로는 팀장 정본이며 이 세션의 상시 규칙상
에이전트가 수정·게시하지 않는다. 설계서도 기존 조율값 변경은 diff와 이유로 승인받으라고 한다.
따라서 G05는 **신규 ID 추가 후보와 diff를 만들어 사용자 승인을 받은 뒤에만** 정본에 적용한다.
그 전까지는 후보 폴더 안의 사본으로만 작업한다.

## 9. 다음 단계

G01(action 4225601 연결표), G02(무대 정체성), G03/G04(렌더링 소비자 비교·원본 RNM 연결)를
독립 작업으로 동시에 시작했다. G02가 막혀도 G03~G08은 멈추지 않는다. 각 G의 결과는
`.md/GB/09-26/2026-09-26_MAHARAKA_G0*_*_RESULT.md`에 남긴다.
