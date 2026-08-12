# AGENTS.md

LostArk 팀 저장소에서 사용하는 공통 작업 규칙이다.
프로젝트 구조와 빌드 설명은 `CLAUDE.md`를 따른다. 구현 계획서의 형식과 문체는
아래 `계획서 규칙`의 선택적 규칙 파일 탐색 순서로 결정한다.

## 작업 시작 시 읽을 문서

| 작업 | 추가로 읽을 문서 |
|---|---|
| 모든 세션(예외 없음) | `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, 있으면 `.md/GB/gotchas.local.md`, `.md/TEAM/README.md`, 대응 `*_PLAN.md`와 `*_RESULT.md` |
| 빌드, Prototype/Clone, Level/Layer, Binary Asset, 리소스 배포 | `CLAUDE.md` |
| 계획서, 설계서, 구현 가이드 | 아래 `계획서 규칙`에서 발견한 첫 번째 규칙 파일 |
| G별 H/CPP/struct/변수 설명, 전체 코드 출력 원칙 | `.md/GB/계획서하네스규칙.local.md`가 있으면 먼저 읽고, 없으면 `.md/GB/local.md` |
| 기존 작업 재개 | `.md/GB/<MM-DD>/`의 대응 `*_PLAN.md`, `*_RESULT.md` |
| merge, pull, rebase, 충돌 해결 | `.md/GB/gotchas.md`, 있으면 `.md/GB/gotchas.local.md`, 영향받는 `*_PLAN.md`와 `*_RESULT.md` |
| 팀 담당 인터페이스, Area 데이터 레이어, 신규 팀원 인계 | `.md/TEAM/README.md`에서 현재 정본 순서대로 읽기 |
| LostArk 맵 에셋 검색, UModel 추출, ModelAssetConverter, MapTool 적용 | `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md` |
| 맵 에셋이 생성됐지만 안 보임, diffuse 누락, 스케일 오류, 레거시 런타임 혼선 | `.md/GB/07-29/gotchas.md` |

### 2026-08-20까지 세션 시작 시 팀 LAN 자동 설정

2026-08-20 23:59 KST까지 이 저장소에서 시작하는 모든 에이전트 세션은 다른 작업보다 먼저
다음 명령을 실행한다. 사용자에게 IP를 다시 묻거나 `127.0.0.1`로 임의 복원하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

`Tools/Network/TeamLanEndpoint.json`이 이 기간의 단일 endpoint 정본이다. 스크립트는 모든 PC의
Git 제외 `Client.vcxproj.user`를 `LOSTARK_SERVER_HOST=192.168.200.103`으로 갱신한다.
현재 endpoint 주소를 실제로 가진 PC만 `server-host`로 판정해 `Server.vcxproj.user`의
`--bind-address 0.0.0.0`과 TCP 7777 LocalSubnet 방화벽 규칙도 확인한다. 출력이
`server-host`이면 `Server + Client` profile, `client`이면 Client project를 사용자가 `Ctrl+F5`로
시작할 대상으로 안내한다. 에이전트가 Client나 UI를 자율적으로 실행·조작하지 않는다.
관리자 권한이 있어야 방화벽 규칙을 추가할 수 있으면 그 사실을 즉시 보고한다.
현재 Server가 꺼져 있어 endpoint probe가 `not-listening`이어도 로컬 설정은 완료된 것이므로 작업을
막지 않는다. 만료 뒤에는 `-AllowExpired`로 조용히 우회하지 말고 endpoint 정본과 public 계약을
먼저 갱신한다.

## 정본 문서 역할과 일일 유지 관리

문서는 같은 내용을 복제하지 않고 다음 역할로 나눈다.

| 문서 | 정본 역할 | 갱신 시점 |
|---|---|---|
| `AGENTS.md` | 팀 전체의 금지 경계, 작업 절차, 완료 조건 | 팀 규칙이나 public 경계가 바뀔 때만 |
| `CLAUDE.md` | 실제 프로젝트 구조, 최초 세팅, 빌드·런타임 사용법 | 경로, 명령, 실행 구조가 바뀔 때 |
| `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 담당별 입력·출력 인터페이스와 데이터 정본 | 팀원이 소비하는 public 계약이 바뀔 때 |
| `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` | Area별 optional layer, MapTool과 publisher 지원 범위 | Area 데이터 계약이 바뀔 때 |
| 대응 `*_PLAN.md` | 아직 구현하지 않은 목표와 전체 코드 | 구현 전에 |
| 대응 `*_RESULT.md` | 실제 완료 상태, 검증 증거, 남은 경계 | 검증 후 |

매 작업일 시작에는 `git status --short`, `git fetch`, 현재 브랜치와 대응 PLAN/RESULT를 확인한다. 종료에는 다음 순서를 지킨다.

1. 실제 코드와 데이터의 현재 상태를 다시 확인한다.
2. 바뀐 public 계약만 `AGENTS.md`, `CLAUDE.md`, 팀 사용서에 반영한다.
3. 구현 완료와 미완료를 RESULT에서 분리하고 실행한 검증만 기록한다.
4. 관련 harness와 `ProjectAudit`을 실행하고 `git diff --check`를 확인한다.
5. 빌드·중간 산출물을 제외한 하나의 검증 단위만 commit/push한다.

날짜별 진행 로그나 일시적인 오류를 `AGENTS.md`와 `CLAUDE.md`에 누적하지 않는다. 그런 정보는 해당 RESULT에 기록한다. 문서와 코드가 다르면 코드·데이터·실행 결과를 먼저 조사하고 같은 변경에서 문서를 교정한다.

## 계획서 규칙

### 규칙 파일 탐색 순서

계획서, 설계서, 구현 가이드를 작성할 때 다음 순서로 규칙 파일을 찾는다.

1. `.md/GB/계획서하네스규칙.local.md`: 개인 G별 설명·전체 코드·검증 형식. Git에 커밋하지 않는다.
2. `.md/계획서작성규칙.local.md`: 개인 활성화 파일. Git에 커밋하지 않는다.
3. `.md/GB/local.md`: Git으로 공유하는 LostArk 계획서·대화 출력 규칙.
4. `.md/계획서작성규칙.md`: 팀이 합의했을 때만 두는 선택적 root 공유 규칙.
5. `.md/GB/계획서작성규칙.md`: 전체 코드 계획서의 Git 공유 fallback 규칙.

`.md/GB/계획서하네스규칙.local.md`가 있으면 처음부터 끝까지 읽고 계획서와 대화 출력의
형식·문체에 우선 적용한다. 이 파일 또는 개인 활성화 파일이 다른 규칙 파일을 지시하면 그 문서도
처음부터 끝까지 함께 읽는다.
`.md/GB/코드작성규칙.local.md`는 이전 설명 원문 보존본이며 현재 출력 규칙으로 직접 사용하지 않는다.

- 먼저 발견한 파일 하나를 처음부터 끝까지 읽고 그 형식과 문체를 따른다.
- 위 후보 파일이 모두 없으면 누락을 오류로 처리하거나 작업을 멈추지 않는다. 현재 코드와
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
- ImGui는 authoring/debug 선택과 명령을 전달하는 UI다. 제품 UI 이미지를 ImGui 화면에서
  캡처하거나 ImGui widget 자체를 제품 런타임으로 승격하지 않는다. UI 담당자는 ImGui로
  배치한 결과를 `Data/UI` JSON의 stable slot ID, rect, draw order, Resources-relative image
  asset ID로 저장하고 제품 런타임은 그 계약으로 `CUIObject` 계열 image widget을 만든다.
- UI image asset은 `Client/Bin/Resources/UI/<Domain>/...`에 두고 JSON에는 `UI/...` 상대
  asset ID만 저장한다. UI layout 로드도 `parse -> validate -> stage -> commit`을 따르며
  실패하면 기존 화면을 유지한다.
- 제품 UI picking은 world ray `CPicking`을 재사용하지 않는다. viewport 좌표를 layout의
  reference resolution으로 변환한 뒤 visible/enabled widget을 앞쪽 draw order부터 screen-space
  hit test한다. 최상위 widget 하나만 pointer를 소비하고, 소비한 프레임에는 gameplay mouse
  command를 막는다. 결과는 stable widget/command ID를 통해 `CLobbyCommandService`,
  `CLevelTransitionService`, `IPlayerCommandSink` 중 해당 typed 경계로만 제출한다.
- Catalog는 생성 가능한 정의, placement 문서는 배치 인스턴스의 ID와 Transform을 소유한다.
- 안정적인 asset ID와 placement ID를 저장 계약으로 사용한다. Prototype tag, 포인터, vector index는 저장 ID가 아니다.
- 로드는 `parse -> validate -> stage -> commit` 순서로 처리하고 실패하면 생성 중인 객체를 전부 rollback한다.
- Loader 종료는 협력 취소와 bounded join을 사용한다. `TerminateThread`로 worker만 죽여 process를 계속 실행하지 않는다. 취소와 동기 I/O 중단 후에도 제한 시간을 넘기면 전용 timeout exit code로 process fail-fast하며 하네스는 이를 실패로 처리한다.

## 고정 런타임 계약

- Client 시작 Level은 항상 `LOBBY`다. Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 명령만 제공한다. 별도 시나리오 catalog나 Client 실행 인자로 시작 Level을 바꾸지 않는다.
- `CHARACTER_SELECT`는 Lobby가 선택 class로 `WORLD_ID::CHARACTER_SELECT_ARENA` Server 승인을 받은 뒤에만 `LV_LOBBY_CLASSSELECT_SL00` visual map을 연다. offline Preview와 `Preview / Server Play` mode 분기는 없다. 진입한 Level은 승인된 socket을 one-shot handoff로 소비하고 `CClientReplication -> CPlayerController -> IPlayerCommandSink`를 사용해 우클릭 이동과 class quick-slot 스킬을 Server snapshot으로 반영한다. class 썸네일 선택은 별도 Confirm 없이 typed class-change command를 보내며 Server가 같은 `PlayerId/NetEntityId`와 살아 있는 위치를 유지한 채 action/movement/cooldown/combo/HP/resource/stance를 새 profile로 초기화한다. 사망 중 변경은 원래 Server spawn의 navigation-projected 위치에서 부활한다. snapshot의 지속 class state가 Client presentation을 transactionally 교체하고 실패하면 기존 character와 상태 메시지를 유지한다. Character Select가 직접 connect/send/approval을 반복하지 않는다. Server는 필수이며 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남고 자동 local fallback은 없다. 진입 후 disconnect는 replicated state를 정리하고 Lobby로 복귀한다. Server Arena의 Debug ImGui는 일반 몬스터, `MINIBOSS_LUGARU`, Valtan 중 하나의 stable placement/group ID만 typed command sink로 제출한다. 일반 몬스터와 Lugaru는 Area `SpawnGroups.world.json`을, Valtan은 disabled world template을 Server가 navigation/profile 검증 후 활성화하며 Client local spawn은 금지한다. 즉시 SpawnGroup은 실제 entity commit이 성공한 뒤에만 활성화 결과를 회신하고, 마지막 플레이어가 퇴장하면 동적 audition entity와 SpawnGroup 상태를 초기화한다. Client는 broadcast presentation만 생성하고 Valtan prototype은 batch lazy-load한다. Bern/Valtan도 마지막 Server 승인 class를 `C2S_ENTER_WORLD`로 보내고 `S2C_ENTER_ACCEPTED`를 받은 뒤에만 진입한다.
- 2026-08-20 23:59 KST까지 팀 LAN 검증 Server listener 기본 bind는 `0.0.0.0`이고 Client 기본 endpoint는 `192.168.200.103:7777`이다. endpoint와 만료일 정본은 `Tools/Network/TeamLanEndpoint.json`이다. `Framework.slnLaunch`의 `Server + Client` profile과 공유 x64 debugger 설정도 같은 계약을 사용한다. `LOSTARK_SERVER_HOST`로 Client endpoint를 명시하면 그 값을 우선하며 `0.0.0.0`은 Client 접속 주소로 사용하지 않는다. 이 주소 계약을 바꿀 때는 Server/Client 기본값, 공유 debugger 설정, 팀 사용서와 ProjectAudit을 같은 변경 단위에서 갱신한다.
- 최소 수련장은 `dev.training.ground -> LEVEL::DEVELOPMENT -> LV_DEV_TRAINING_GROUND -> WORLD_ID::TRAINING_GROUND` 계약을 사용한다. 새 `LEVEL::TRAINING`을 만들지 않는다.
- 레벨은 `STATIC, LOADING, LOBBY, CHARACTER_SELECT, BERN, VALTAN_ARENA, DEVELOPMENT`만 사용한다. 새 레벨은 enum, registry, loader, 프로젝트 등록과 실제 Server+Client 진입 검증을 한 변경 단위로 추가한다.
- 제품 맵은 `CLevelRegistry` descriptor의 `MAP_LOAD_SCOPE`로 선언한 진입/전투 범위와 배경만 로드한다. Loader와 runtime placement는 반드시 같은 scope를 소비한다.
- 레벨 전환 요청은 `CLevelTransitionService`로 보낸다. `Change_Level`은 현재 Level update가 끝난 뒤 `CMainApp`만 호출한다. `CLevel_Loading`은 로드 성공 시 activation 요청만 제출한다.
- 공식 전역 기능키는 Debug Developer Tools의 F1과 follow/free camera 전환의 F6뿐이다. F2~F5, F7~F12로 레벨, 맵, 프로파일러, 도구 상태를 바꾸지 않는다. free camera에서는 gameplay command 입력을 보내지 않는다.
- `Client/Bin/Resources`의 최상위 폴더는 `Fonts, Character, Deploy, Effect, Map, UI` 정확히 여섯 개다. `Resources/LostArk` 래퍼와 `SourceData`를 만들지 않는다.
- 런타임 asset ID는 Resources 상대 경로다. 절대 경로, drive-qualified 경로, `..`로 루트를 벗어나는 경로를 거부한다.
- UI와 gameplay 설정은 JSON만 사용한다. `.cfg` 신규 추가와 runtime cfg reader는 금지한다.
- Git 관리 대상 `Data` 원본은 Client 프로젝트의 `96.DataFiles` 아래 `None` 항목으로만 노출한다. 프로젝트별 복사본이나 build output `Content` 항목으로 만들지 않는다.
- MapTool은 `Data/Maps/Authoring`에 저장한다. 검증/publish 도구만 `Client/Bin/DataFiles/Map` 런타임 문서를 교체할 수 있다.

## 팀 인터페이스와 담당 영역

### 기능 단위 작업 모델

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

- 아래 담당 표는 파일 수정 권한표가 아니다. 담당 이름은 작업 시작점, 데이터 정본, 런타임
  권위와 우회하면 안 되는 public interface를 설명한다.
- 기능 담당자는 요청받은 기능의 수직 슬라이스를 끝까지 구현한다. 기능에 Server authority가
  필요하면 `Data -> Shared -> Server -> Client presentation/UI -> harness`의 필요한 파일을
  같은 변경 단위에서 직접 수정한다. Server 폴더가 다른 담당으로 표시됐다는 이유로 Client
  mock, 로컬 우회, 문서만 남기고 중단하지 않는다.
- 교차 영역 수정은 범위 확장이 아니라 기능 완성에 필요한 계약 연결이다. 다만 UI가 socket을
  직접 호출하거나 Character가 damage를 판정하는 등 아래 금지 경계를 우회해서는 안 된다.
- 같은 파일에 다른 팀원의 미커밋 변경이 있으면 덮어쓰지 않고 먼저 현재 diff를 보존·조정한다.
  역할 분리는 merge 충돌을 줄이는 기준이지, 필요한 서버·클라이언트 구현을 금지하는 장벽이 아니다.
- 완료 기준은 한쪽 구현이 아니라 실제 소비자가 연결된 실행 계약이다. 새 command/state/data를
  추가했다면 관련 publisher, protocol/server contract, Client smoke와 실패 경로까지 함께 검증한다.

- UI 담당자는 `CLobbyCommandService`와 `CLevelTransitionService`에 command를 제출하고, 전투 HUD는 `CCombatHUDViewModel`의 읽기 전용 player/boss 상태를 소비한다. UI 코드에서 packet 작성, socket 호출, snapshot 파싱, `Change_Level`을 하지 않는다.
- 입력 담당자는 `CPlayerController -> IPlayerCommandSink` 계약을 사용한다. Controller에서 `CNetworkManager`를 직접 include하지 않는다. Controller는 quick slot 이름과 물리 키만 알고, (class, slot) → skill ID는 `Data/Balance/PlayerSkills.json`의 `inputSlot`을 `CPlayerSkillCatalog`로 조회한다. Controller에 skill ID를 하드코딩하지 않는다.
- Character/Animation 담당자는 `CHARACTER_SPEC`, presentation callback, `CAnimationTargetService`를 사용한다. `Logic_*`에서 DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다. 툴에서 level/layer/part tag/vector index를 추측하지 않는다.
- Character Select와 입장 roster는 `LANCE_MASTER, GUNSLINGER, SLAYER, ARTIST, DIMENSIONMASTER, WARLORD` 여섯 class다. 여섯 class의 quick slot ACTIVE 스킬과 LMB COMBO 평타는 `Data/Balance/PlayerSkills.json`에서 `(characterClass, inputSlot) -> skillId`를 resolve하고 command → Server approval → snapshot → Character presentation 계약을 사용한다. 현재 ACTIVE 슬롯은 Lance Master `Q W E R A S T V ALT_V`, Gunslinger `Q W E R A S D F T V ALT_V`, Slayer `Q W E R A S D F V ALT_V`, Artist `Q W E R A S V ALT_V`, DimensionMaster `Q W E R A S D F T V ALT_V`, Warlord `Q W E R A S D F T X V ALT_V`다. Warlord만 `X`(전장의 방패)와 `Z` 방어 태세 전환을 쓴다. LMB 평타는 각각 `34010/38000/45000/31000/2050010/17000`이고 Server가 `SNAPSHOT_PLAYER::iComboStage`를 확정한다. Client와 Animation Tool은 콤보 단계를 스스로 만들지 않는다.
- playable skill의 Server 수치 정본은 `Data/Balance/PlayerSkills.json`과 `DamageProfiles.json`, presentation 정본은 `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`이다. Animation Tool은 Scene Character의 실제 model clip만 현재 class의 skillId에 순서대로 연결해 저장한다. ACTIVE는 하나 이상의 순차 clip, COMBO는 `comboStages`와 정확히 같은 BA 단계 수를 요구한다. `inputSlot`, `skillId`, `skillKind`, timing, damage는 Animation Tool에서 편집하지 않는다. missing/corrupt presentation 문서는 spawn과 Server gameplay를 막지 않고 해당 action 표현만 격리한다. `.skilltiming/.clipmap/.animnotify/.clipseq`와 `Data/Animation/Reference`는 read-only 저작 참고 자료이며 제품 runtime 정본이 아니다. 이동기·스탠스 전환은 아직 별도 수직 슬라이스 범위다.
- Server 담당자는 `Shared` message와 stable world/entity/archetype ID를 경계로 사용한다. Client GameObject, Prototype tag, asset path를 Server에 전달하지 않는다.
- 플레이어·스킬·damage·boss 수치 정본은 각각 `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, `BossProfiles.json`이다. Server pre-build가 `Publish-GameplayBalance.ps1`로 검증·publish하며 생성된 bootstrap을 직접 편집하지 않는다.
- field-level 공식 근거 정본은 `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`이다. publisher는 5 profile, 88 skill definition, 63 damage profile과 Valtan encounter의 모든 저작 field coverage/result 일치를 검사한다. F1 Balance Tool에서 바뀐 field는 receipt 동기화 단계에서 `PROJECT_TUNED`로 분류하며 공식 basis를 수동 유지하지 않는다. Publish 뒤 Server 재시작 전에는 적용 완료가 아니다.
- 제품 이동과 스킬 이동 보정, Valtan 추적은 `Data/Navigation` authoring에서 publisher가 생성한 Server runtime `.navgrid`를 소비한다. MapTool bake Area는 `<AreaId>.navsource/.navpaint/.navblockers`, 단순 uniform Area는 `<AreaId>.navgrid.json`을 정본으로 사용한다. Client Navigation 결과나 transform을 서버 정답으로 보내지 않는다.
- Map/Encounter 담당자는 catalog 정의와 placement instance를 분리한다. `Gameplay.world.json` authoring은 formatVersion 4이며 actor placement, `triggerBox`, `collisionBox`, gated `destroyable` 구조를 구분한다. 제품 publisher/runtime는 player spawn/NPC/boss, 정확히 하나의 `movePlayer`, `changeLevel`, `activateSpawnGroup`, `activateEncounter` action을 가진 triggerBox, Server 권위 정적 collisionBox를 지원한다. NPC 제품 presentation은 현재 `NPC_BEDA` 한 archetype만 지원한다. `activateSpawnGroup`은 같은 Area의 `SpawnGroups.world.json` stable group ID만 참조하고, `activateEncounter`는 같은 문서의 disabled boss placement ID만 참조한다. movePlayer/changeLevel/activation의 OBB 진입, player collider와 collisionBox의 swept 이동 차단은 Server authority이고 Shared snapshot/spawn/despawn으로 표현한다. changeLevel은 Bern과 Valtan Arena 사이에서만 Server가 source room leave와 target room enter를 확정하고 Client는 `S2C_ENTER_ACCEPTED` 뒤 typed level transition을 제출한다. destroyable과 다른 trigger action은 dynamic navigation·replication·Client presentation이 닫히기 전까지 publisher가 거부하며 authoring parser 존재만으로 제품 지원 완료 처리하지 않는다.
- 수업용 `CMonster`와 `astar/Monster`는 계속 금지한다. 제품 일반 몬스터는 `MonsterCatalog.json` + `MonsterProfiles.json` + Area `SpawnGroups.world.json`을 `Publish-WorldGameplay.ps1`로 publish하고, Server `CSpawnGroupRuntime/CMonsterBrain`과 Shared world entity spawn/snapshot/despawn, Client catalog presentation 경로만 사용한다.
- Valtan 제품 경로의 transform, action, phase, damage 판정은 Server authority다. Client `CValtan`의 로컬 AI는 Development preview 외에 사용하지 않는다.
- 플레이어 스킬, 일반 몬스터, Lugaru, Valtan의 combat overlap은 Engine/PhysX 비의존 Shared XZ primitive 계약을 Server fixed tick에서 평가한다. Client의 character/NPC/Valtan collider는 같은 body radius를 표시하는 Debug mirror이며 hit·damage 권위를 갖지 않는다.
- MapTool gameplay 저장은 `Data/Worlds/<AreaId>/Gameplay.world.json`이 정본이다. `Publish-WorldGameplay.ps1`이 Server bootstrap을 생성하며 생성물을 직접 편집하지 않는다.
- `playerSpawn`은 spawn slot과 transform만 소유하고 `archetypeId`는 `null`이다. 실제 character class는 인증된 session/player selection이 소유한다.

## 사용자 전용 화면 검증 경계

- Artist F, Effect Tool, Character Select와 모든 Client 시각 결과는 사용자가 직접 조작하고 최종 visual fidelity를 판정한다.
- 에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고, 화면을 직접 캡처하거나 스크린샷을 만들지 않으며, visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지를 분석해 달라고 요청하면 에이전트는 반드시 열람·분석하고, 관찰된 결함과 가능한 occurrence 진단을 보고한다. 이 분석은 사용자의 최종 육안 판정을 대체하지 않는다.
- `완성`, `복원`, `보이게 해줘`, `눈으로 검증 가능하게 해줘` 같은 일반 구현 요청은 Client/UI 자율 실행·조작이나 화면 캡처 권한이 아니다.
- 에이전트는 빌드, 구조화된 로그와 수치 진단, 실행 준비까지만 수행한다. 그 뒤 Server CMD와 Client 실행 상태, 사용자가 직접 누를 정확한 경로를 보고하고 멈춘다.
- `manual first pixel`, `eye smoke`, `visual PASS`, occurrence 승인은 사용자의 서면 관찰과 결정이 있어야 한다. 에이전트가 대신 PASS로 기록하지 않는다.
- 사용자가 첨부한 이미지는 요청 시 진단·리뷰 입력으로 사용하되 단독 자동 admission이나 완료 증거로 승격하지 않는다. 에이전트가 직접 만든 캡처나 자동 UI 조작 결과는 증거로 사용하지 않는다.

## AI 코드 하네스

- 코드 작성 전에 실제 호출자, 소유자, 데이터 정본, 실패 소비자를 확인한다. 소비자가 없는 인터페이스와 미래용 placeholder 파일을 추가하지 않는다.
- 함수는 하나의 의미 단위를 소유하고 이름이 그 단위를 설명해야 한다. `Manager`, `Data`, `Handle`, `Temp` 같은 포괄 이름만으로 새 상태를 숨기지 않는다.
- 저장 ID에 pointer, Prototype tag, vector index를 쓰지 않는다. switch fallback으로 모르는 enum/ID를 정상값처럼 처리하지 않는다.
- 파일/네트워크/레벨 로드는 실패 이유를 보존한다. 무한 대기, 부분 commit, silent identity fallback을 금지한다.
- 새 public 계약에는 정상 사례, 잘못된 version/ID/path, 중복, 중간 실패 rollback을 검증하는 harness 또는 audit check를 함께 추가한다.
- 밸런스 파일을 매 프레임 읽거나 Client만 reload하지 않는다. runtime Hot Reload는 revision, Server stage, room tick commit, 진행 중 action 정책, snapshot revision, Client 동기화와 rollback harness가 한 수직 슬라이스로 닫힐 때만 활성화한다.
- 완료 전 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`을 실행한다.

## Git·문서·완료 보고 계약

- `main`에 직접 작업하지 않는다. 사람 작업 브랜치는 팀 명명 규칙을 따르고 Codex 작업 브랜치는 `codex/<topic>`을 사용한다.
- 작업을 시작할 때 `git status --short`로 다른 담당자의 변경과 생성물을 구분한다. 소유권이 불명확한 대규모 dirty worktree에서는 자동 stage/commit하지 않는다.
- 하나의 커밋은 하나의 검증 가능한 계약만 담는다. 코드, 그 코드가 소비하는 JSON/schema, 필요한 project/filter 등록, 대응 harness, PLAN/RESULT 갱신은 같은 변경 단위로 묶는다.
- `Client/Bin/Resources`는 팀장이 직접 관리하는 runtime 입력이다. immutable pack, lock, manifest, hash publish를 완료 조건으로 두지 않는다. build/intermediate 산출물은 커밋하지 않는다.
- 계획/결과 문서는 `.md/GB/<MM-DD>/`에 보관한다. `.md/계획서작성규칙.local.md`와 local gotcha는 개인 파일이므로 커밋하지 않는다.
- 완료 보고 전에 `git diff --check`, JSON/XML parse, 관련 harness, 정본 build/regression을 실행한다. Release에서 의도적으로 제외된 Development tool smoke를 PASS로 기록하지 않는다.
- 문서에 적었다는 이유로 구현을 완료 처리하지 않는다. 구현 상태, 자동 검증 상태, 수동 검증 상태, 다음 단계 항목을 분리해 기록한다.
- 비평 에이전트의 지적은 그대로 결론으로 복사하지 않는다. 실제 코드와 데이터로 재현한 뒤 가능한 항목은 ProjectAudit, protocol harness, smoke의 실행 계약으로 바꾼다.
- 팀원 인계에는 필요한 `Client/Bin/Resources` 상대 asset ID와 물리 폴더 위치를 함께 적는다. ZIP hash나 Hydrate/Verify 결과를 요구하지 않는다.

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
