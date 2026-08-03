# AGENTS.md

LostArk 팀 저장소에서 사용하는 공통 작업 규칙이다.
프로젝트 구조와 빌드 설명은 `CLAUDE.md`를 따른다. 구현 계획서의 형식과 문체는
아래 `계획서 규칙`의 선택적 규칙 파일 탐색 순서로 결정한다.

## 작업 시작 시 읽을 문서

| 작업 | 추가로 읽을 문서 |
|---|---|
| 빌드, Prototype/Clone, Level/Layer, Binary Asset, 리소스 배포 | `CLAUDE.md` |
| 계획서, 설계서, 구현 가이드 | 아래 `계획서 규칙`에서 발견한 첫 번째 규칙 파일 |
| 기존 작업 재개 | `.md/GB/<MM-DD>/`의 대응 `*_PLAN.md`, `*_RESULT.md` |
| LostArk 맵 에셋 검색, UModel 추출, ModelAssetConverter, MapTool 적용 | `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md` |
| 맵 에셋이 생성됐지만 안 보임, diffuse 누락, 스케일 오류, 레거시 런타임 혼선 | `.md/GB/07-29/gotchas.md` |

## 정본 문서 역할과 일일 유지 관리

문서는 같은 내용을 복제하지 않고 다음 역할로 나눈다.

| 문서 | 정본 역할 | 갱신 시점 |
|---|---|---|
| `AGENTS.md` | 팀 전체의 금지 경계, 작업 절차, 완료 조건 | 팀 규칙이나 public 경계가 바뀔 때만 |
| `CLAUDE.md` | 실제 프로젝트 구조, 최초 세팅, 빌드·런타임 사용법 | 경로, 명령, 실행 구조가 바뀔 때 |
| `TEAM_GAMEPLAY_INTERFACE_HANDBOOK` | 담당별 입력·출력 인터페이스와 데이터 정본 | 팀원이 소비하는 public 계약이 바뀔 때 |
| 대응 `*_PLAN.md` | 아직 구현하지 않은 목표와 전체 코드 | 구현 전에 |
| 대응 `*_RESULT.md` | 실제 완료 상태, 검증 증거, 남은 경계 | 검증 후 |

매 작업일 시작에는 `git status --short`, `git fetch`, 현재 브랜치와 대응 PLAN/RESULT를 확인한다. 종료에는 다음 순서를 지킨다.

1. 실제 코드와 데이터의 현재 상태를 다시 확인한다.
2. 바뀐 public 계약만 `AGENTS.md`, `CLAUDE.md`, 팀 사용서에 반영한다.
3. 구현 완료와 미완료를 RESULT에서 분리하고 실행한 검증만 기록한다.
4. 관련 harness와 `ProjectAudit`을 실행하고 `git diff --check`를 확인한다.
5. Resources payload와 빌드·중간 산출물을 제외한 하나의 검증 단위만 commit/push한다.

날짜별 진행 로그나 일시적인 오류를 `AGENTS.md`와 `CLAUDE.md`에 누적하지 않는다. 그런 정보는 해당 RESULT에 기록한다. 문서와 코드가 다르면 코드·데이터·실행 결과를 먼저 조사하고 같은 변경에서 문서를 교정한다.

## 계획서 규칙

### 규칙 파일 탐색 순서

계획서, 설계서, 구현 가이드를 작성할 때 다음 순서로 규칙 파일을 찾는다.

1. `.md/계획서작성규칙.local.md`: 개인 작업 스타일. Git에 커밋하지 않는다.
2. `.md/계획서작성규칙.md`: 팀이 합의했을 때만 두는 선택적 공유 규칙.

- 먼저 발견한 파일 하나를 처음부터 끝까지 읽고 그 형식과 문체를 따른다.
- 두 파일이 모두 없으면 누락을 오류로 처리하거나 작업을 멈추지 않는다. 현재 코드와
  요청에 맞는 합리적인 형식으로 계획서를 작성한다.
- 규칙 파일이 없다는 이유만으로 새 규칙 파일을 자동 생성하거나 사용자에게 질문하지 않는다.
- 개인/공유 규칙은 문서의 형식과 문체만 재정의한다. 이 `AGENTS.md`의 팀 작업 경계,
  구현 원칙, 빌드·검증 규칙은 항상 우선한다.

### 공통 최소 계약

- 기본 위치는 `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_PLAN.md`이다.
- 같은 작업의 계획서가 있으면 새 파일을 만들지 않고 기존 문서를 갱신한다.
- 문서의 섹션 순서, 상세도, 전체 코드 포함 여부는 발견한 규칙 파일을 따른다. 규칙
  파일이 없으면 작업 규모와 독자가 검증하기 쉬운 형태를 작성자가 선택한다.
- 새 C++ 파일을 제안하면 `.vcxproj`와 `.vcxproj.filters` 등록 필요 여부와 검증 방법을
  빠뜨리지 않는다.
- 불확실한 내용은 계획서 안에 표식으로 미루지 않는다. 먼저 실제 코드와 데이터를 조사하고, 그래도 사용자 결정이 필요하면 계획서 작성 전에 질문한다.

## 팀 작업 경계

- `main`은 정본이며 기능 작업은 별도 브랜치와 PR을 사용한다.
- 다른 팀원의 미커밋 변경과 무관한 파일을 되돌리거나 정리하지 않는다.
- 변경한 모든 줄은 현재 요청과 직접 연결되어야 한다.
- `Engine/`, `Client/` 기존 C++ 파일은 파일별 인코딩을 감지해 유지한다. 새 C++ 파일은 UTF-8(BOM 없음), Markdown 문서는 UTF-8로 저장한다.
- 물리 폴더가 소스 구조의 정본이다. `.vcxproj`와 `.filters`는 필요한 항목만 추가하고 기존 필터를 재배치하지 않는다.
- 빌드 산출물, `EngineSDK`, `.vs`, `imgui.ini`는 소스 커밋에 섞지 않는다.

## 구현 원칙

- 추측보다 현재 코드와 데이터 실측을 우선한다.
- 기존 Prototype/Clone/Layer/CModel 경로를 확장하고 같은 역할의 두 번째 런타임 경로를 만들지 않는다.
- `CCookedModel`과 `CBinaryAssetObject` 경로는 제거됐다. 신규 기능과 MapTool 에셋은 반드시 `CModel -> CMaterial` 통합 경로를 사용하며 동등한 두 번째 모델 런타임을 만들지 않는다.
- Engine은 범용 기능, Client는 LostArk의 Level, GameObject, 에디터 흐름과 Scene 데이터를 소유한다.
- ImGui는 선택과 명령을 전달하는 UI다. 매 프레임 파일을 읽거나 모델을 다시 디코드하지 않는다.
- Catalog는 생성 가능한 정의, placement 문서는 배치 인스턴스의 ID와 Transform을 소유한다.
- 안정적인 asset ID와 placement ID를 저장 계약으로 사용한다. Prototype tag, 포인터, vector index는 저장 ID가 아니다.
- 로드는 `parse -> validate -> stage -> commit` 순서로 처리하고 실패하면 생성 중인 객체를 전부 rollback한다.
- Loader 종료는 협력 취소와 bounded join을 사용한다. `TerminateThread`로 worker만 죽여 process를 계속 실행하지 않는다. 취소와 동기 I/O 중단 후에도 제한 시간을 넘기면 전용 timeout exit code로 process fail-fast하며 하네스는 이를 실패로 처리한다.

## 고정 런타임 계약

- Client 시작 씬은 항상 `LOBBY`다. 실제 실행 시나리오는 `Data/Levels/LevelCatalog.json`의 stable ID로 선택한다.
- 최소 수련장은 `dev.training.ground -> LEVEL::DEVELOPMENT -> LV_DEV_TRAINING_GROUND -> WORLD_ID::TRAINING_GROUND` 계약을 사용한다. 새 `LEVEL::TRAINING`을 만들지 않는다.
- 레벨은 `STATIC, LOADING, LOBBY, BERN, VALTAN_ARENA, DEVELOPMENT`만 사용한다. 새 레벨은 catalog, registry, loader, smoke 검증을 한 변경 단위로 추가한다.
- 제품 맵은 `mapLoadBounds`로 선언한 진입/전투 범위와 배경만 로드한다. `dev.map.active`만 전체 맵을 열 수 있다. Loader와 runtime placement는 반드시 같은 `MAP_LOAD_SCOPE`를 소비한다.
- 레벨 전환 요청은 `CSceneTransitionService`로 보낸다. `CMainApp`과 `CLevel_Loading` 외에는 `Change_Level`을 직접 호출하지 않는다.
- Debug 전역 도구 단축키는 F1뿐이다. F2~F12로 레벨, 맵, 카메라, 프로파일러, 도구 상태를 바꾸지 않는다.
- `Client/Bin/Resources`의 최상위 폴더는 `Fonts, Character, Deploy, Effect, Map, UI` 정확히 여섯 개다. `Resources/LostArk` 래퍼와 `SourceData`를 만들지 않는다.
- 런타임 asset ID는 Resources 상대 경로다. 절대 경로, drive-qualified 경로, `..`로 루트를 벗어나는 경로를 거부한다.
- UI와 gameplay 설정은 JSON만 사용한다. `.cfg` 신규 추가와 runtime cfg reader는 금지한다.
- MapTool은 `Data/Maps/Authoring`에 저장한다. 검증/publish 도구만 `Client/Bin/DataFiles/Map` 런타임 문서를 교체할 수 있다.

## 팀 인터페이스와 담당 영역

- UI 담당자는 `CLobbyCommandService`와 `CSceneTransitionService`에 command를 제출하고, 전투 HUD는 `CCombatHUDViewModel`의 읽기 전용 player/boss 상태를 소비한다. UI 코드에서 packet 작성, socket 호출, snapshot 파싱, `Change_Level`을 하지 않는다.
- 입력 담당자는 `CPlayerController -> IPlayerCommandSink` 계약을 사용한다. Controller에서 `CNetworkManager`를 직접 include하지 않는다. 현재 Q/W는 stable skill ID `34060`/`34100`을 `C2S_USE_SKILL`로 제출한다.
- Character/Animation 담당자는 `CHARACTER_SPEC`, presentation callback, `CAnimationTargetService`를 사용한다. `Logic_*`에서 DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다. 툴에서 level/layer/part tag/vector index를 추측하지 않는다.
- 스킬 `34060`/`34100`은 command → server approval → snapshot → Character presentation 계약이 닫혔다. 새 스킬은 `Data/Balance` 정의, Shared command/snapshot, Server 판정, Client presentation, protocol/server harness를 함께 추가할 때만 활성화하며 로컬 우회 재생하지 않는다.
- Server 담당자는 `Shared` message와 stable world/entity/archetype ID를 경계로 사용한다. Client GameObject, Prototype tag, asset path를 Server에 전달하지 않는다.
- 플레이어·스킬·damage·boss 수치 정본은 각각 `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, `BossProfiles.json`이다. Server pre-build가 `Publish-GameplayBalance.ps1`로 검증·publish하며 생성된 bootstrap을 직접 편집하지 않는다.
- 제품 이동과 스킬 이동 보정, Valtan 추적은 `Data/Navigation/<AreaId>.navgrid.json` authoring에서 publisher가 생성한 Server runtime `.navgrid`를 소비한다. Client Navigation 결과나 transform을 서버 정답으로 보내지 않는다.
- Map/Encounter 담당자는 catalog 정의와 placement instance를 분리한다. 현재 지원하는 player spawn/NPC/boss placement는 map static placement와 별도 gameplay 문서로 저장한다.
- 수업용 `CMonster`와 그 전제를 새 월드 계약으로 승격하지 않는다. Monster runtime/catalog/schema는 현재 통합 범위 밖이며, 실제 요구와 별도 계획·하네스가 승인되기 전에는 placeholder enum이나 빈 catalog도 추가하지 않는다.
- Valtan 제품 경로의 transform, action, phase, damage 판정은 Server authority다. Client `CValtan`의 로컬 AI는 Development preview 외에 사용하지 않는다.
- MapTool gameplay 저장은 `Data/Worlds/<AreaId>/Gameplay.world.json`이 정본이다. `Publish-WorldGameplay.ps1`이 Server bootstrap을 생성하며 생성물을 직접 편집하지 않는다.
- `playerSpawn`은 spawn slot과 transform만 소유하고 `archetypeId`는 `null`이다. 실제 character class는 인증된 session/player selection이 소유한다.

## AI 코드 하네스

- 코드 작성 전에 실제 호출자, 소유자, 데이터 정본, 실패 소비자를 확인한다. 소비자가 없는 인터페이스와 미래용 placeholder 파일을 추가하지 않는다.
- 함수는 하나의 의미 단위를 소유하고 이름이 그 단위를 설명해야 한다. `Manager`, `Data`, `Handle`, `Temp` 같은 포괄 이름만으로 새 상태를 숨기지 않는다.
- 저장 ID에 pointer, Prototype tag, vector index를 쓰지 않는다. switch fallback으로 모르는 enum/ID를 정상값처럼 처리하지 않는다.
- 파일/네트워크/레벨 로드는 실패 이유를 보존한다. 무한 대기, 부분 commit, silent identity fallback을 금지한다.
- 새 public 계약에는 정상 사례, 잘못된 version/ID/path, 중복, 중간 실패 rollback을 검증하는 harness 또는 audit check를 함께 추가한다.
- 완료 전 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`을 실행한다. 리소스 변경이면 `-DeepAssetHash`를 추가한다.

## Git·문서·완료 보고 계약

- `main`에 직접 작업하지 않는다. 사람 작업 브랜치는 팀 명명 규칙을 따르고 Codex 작업 브랜치는 `codex/<topic>`을 사용한다.
- 작업을 시작할 때 `git status --short`로 다른 담당자의 변경과 생성물을 구분한다. 소유권이 불명확한 대규모 dirty worktree에서는 자동 stage/commit하지 않는다.
- 하나의 커밋은 하나의 검증 가능한 계약만 담는다. 코드, 그 코드가 소비하는 JSON/schema, 필요한 project/filter 등록, 대응 harness, PLAN/RESULT 갱신은 같은 변경 단위로 묶는다.
- `Client/Bin/Resources` payload와 build/intermediate 산출물은 커밋하지 않는다. 리소스 버전 변경은 `Data/AssetPacks.lock.json`, immutable manifest, 운영 문서만 Git에 둔다.
- 계획/결과 문서는 `.md/GB/<MM-DD>/`에 보관한다. `.md/계획서작성규칙.local.md`와 local gotcha는 개인 파일이므로 커밋하지 않는다.
- 완료 보고 전에 `git diff --check`, JSON/XML parse, 관련 harness, 정본 build/regression을 실행한다. Release에서 의도적으로 제외된 Development tool smoke를 PASS로 기록하지 않는다.
- 문서에 적었다는 이유로 구현을 완료 처리하지 않는다. 구현 상태, 자동 검증 상태, 수동 검증 상태, 다음 단계 항목을 분리해 기록한다.
- 비평 에이전트의 지적은 그대로 결론으로 복사하지 않는다. 실제 코드와 데이터로 재현한 뒤 가능한 항목은 ProjectAudit, protocol harness, smoke의 실행 계약으로 바꾼다.
- 팀원 인계는 Git commit만으로 끝나지 않는다. `Data/AssetPacks.lock.json`이 가리키는 immutable resource ZIP의 위치·SHA-256과 `Hydrate -> Verify` 결과를 함께 공유한다.

## 빌드·검증

```text
1. Engine x64 Debug/Release
2. UpdateLib.bat Debug/Release
3. Shared + NetworkProtocolHarness x64 Debug/Release, harness 실행
4. Server x64 Debug/Release, `Server.exe --contract-test` 실행
5. Client x64 Debug/Release
6. Client smoke: lobby, Bern, Valtan, 각 Development scenario
7. ProjectAudit + 필요한 deep asset hash
```

- 정본 자동화 명령은 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration <Debug|Release>`다. 수동 smoke에서도 Client 작업 디렉터리는 `Client/Default`여야 한다.

- Engine public header를 바꿨다면 `UpdateLib.bat` 뒤 Client까지 검증한다.
- 실행 중인 `Client.exe`가 출력물을 점유하면 종료한 뒤 다시 링크한다.
- 에디터 기능은 빌드만으로 끝내지 않고 실행 레벨, 단축키, 생성, 저장, 재로드, 실패 시 기존 상태 보존까지 확인한다.
