# Character Select Lobby 승인 Server Arena·Animation 입력·Valtan Lazy Spawn RESULT

- 작성일: 2026-08-05
- 상태: 구현 및 자동 검증 완료, 실제 UI 조작 smoke 대기
- 대응 계획: `.md/GB/08-05/2026-08-05_CHARACTER_SELECT_SERVER_GAMEPLAY_NAVIGATION_PLAN.md`

## 1. 완료 결과

Character Select 최초 진입은 socket 없는 Preview다. class를 고르고 `Enter Test`를 누르면 tokenized TEST가
Lobby로 전달되고, Lobby가 `S2C_ENTER_ACCEPTED`의 protocol/world/player/entity를 검증한 뒤 기존 socket과
queued snapshot을 one-shot handoff한다. 같은 visual map으로 다시 열린 Server Arena는 직접 connect/send/
approval을 반복하지 않고 local replicated character를 gameplay actor와 Animation Tool target으로 사용한다.
실패·disconnect는 Preview local fallback이 아니라 Lobby 복귀다.

Server Arena에서 Q/W/E/R/A/S/D/F/T/V, LMB와 ALT+V는 `CPlayerController -> IPlayerCommandSink -> Server
approval -> snapshot -> Character presentation` 경로를 그대로 사용한다. ImGui가 keyboard를 capture해도
Character Select gameplay key는 raw physical state로 읽고, `WantTextInput` 중에는 명령을 막으면서 edge
shadow는 계속 갱신한다. 따라서 F1 Animation Tool을 띄운 채 key별 저장 animation을 검증할 수 있고,
텍스트 편집 종료 시 누르고 있던 key가 새 입력으로 오인되지 않는다.

F6 free camera에서는 camera만 이동하고 gameplay command를 제출하지 않는다. Controller는 disabled 동안에도
물리 key/mouse edge를 동기화하므로 follow 복귀 순간 held key를 새 press로 오인하지 않는다.

## 2. Valtan lazy spawn

`Summon Valtan (Lazy)`는 Server Arena에서만 보인다. Client는 `ValtanPresentationAssetService`로 catalog와
model/prototype을 stage하고 Engine batch 등록이 성공한 뒤 stable placement ID를 typed
`IWorldEntityCommandSink`에 제출한다. Server는 disabled world template, archetype, encounter, navigation과
boss profile을 검증하고 entity를 생성해 broadcast한다.

protocol v9는 `SPAWNED`, `ALREADY_EXISTS`, `REJECTED` 결과를 돌려준다. 같은 placement의 중복 요청은 새
Valtan을 만들지 않고 기존 entity ID와 `ALREADY_EXISTS`를 반환한다. Client presentation 준비나
replication 적용이 실패하면 불완전한 local fallback을 만들지 않고 Lobby로 통제 복귀한다.

## 3. 데이터와 navigation

- gameplay 정본: `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/Gameplay.world.json`
- navigation authoring: `Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource`, `.navpaint`
- Server runtime: publisher가 생성하는 `Server/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid`
- Client runtime: `Client/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid`

Character Select world revision 2에는 player spawn 네 개와 disabled Valtan template 한 개가 있다.
navigation publish 결과는 62x62, cell 0.5, walkable 2176, max step 0.474029541015625로 검증됐다.
disabled boss template도 publish 시 navigation projection 검사를 받는다.

## 4. 애니메이션 담당 작업 흐름

1. Preview에서 class를 선택하고 F1 Animation Tool을 연다.
2. `PlayerSkills.json`에서 나온 key/skill row에 원하는 clip을 지정하고 Save한다.
3. 저장 정본 `Data/Animation/Authored/<Class>/<Class>.skillbindings.json`을 확인한다.
4. `Enter Test` → Lobby 승인 → 같은 visual map Server Arena 재진입 뒤 실제 key를 눌러 snapshot 재생을 확인한다.
5. ACTIVE는 해당 row의 clip을, COMBO는 Server가 내려준 `iComboStage`의 BA row를 재생한다.
6. animation만 수정할 때는 clip/순서만 바꾸고, key·skillId·combo 단계 수 변경은 Gameplay data와 Server
   계약을 먼저 갱신한다.

## 5. 검증 증거

| 검증 | 결과 |
|---|---|
| World/Navigation/Gameplay Balance publisher validate/publish | PASS |
| Debug 정본 BuildAndRegression: Engine, UpdateLib, Shared, Server, Client | PASS |
| Release 정본 BuildAndRegression: Engine, UpdateLib, Shared, Server, Client | PASS |
| NetworkProtocolHarness Debug/Release | 0 failures |
| ClientFrontendHarness Debug/Release | 0 failures |
| `Server.exe --contract-test` Debug/Release | 0 failures |
| Debug Server+Client startup, Client working directory `Client/Default` | 8초 생존 및 응답 PASS |
| 이전 raw TCP Character Select/Valtan spawn/중복 요청 | transport/server 계약 PASS; 최신 Lobby→Level handoff UI smoke 증거는 아님 |
| ProjectAudit의 신규 handoff/data 코드 계약 | PASS |

ProjectAudit 전체 종료 코드는 현재 immutable Resources payload와 `Data/AssetPacks.lock.json` inventory가
불일치해 실패한다. 실측 payload는 10,256 files / 5,507,131,395 bytes다. 이 작업은 Resources를 수정하거나
lock을 재생성하지 않았으므로 code build/harness 성공과 resource hydration blocker를 분리한다.

## 6. 남은 수동 확인

자동 검증으로 network와 contract는 닫혔다. 최종 visual 인증은 실제 실행 화면에서 다음만 확인하면 된다.

1. Preview에서 F1 Animation Tool의 저장 mapping이 보이는지
2. Server Arena에서 Q/W/E/R/A/S/D/F/T/V와 COMBO가 같은 mapping으로 재생되는지
3. F6 free camera에서 skill command가 차단되고 follow 복귀 뒤 새 press만 동작하는지
4. 최초 Valtan 소환 시 잠깐의 asset 준비 뒤 한 개만 생성되고 combat/HUD가 연결되는지
5. disconnect에서 replicated state가 정리되고 Lobby로 복귀하는지

Resources lock이 다시 hydrate·verify되기 전에는 DeepAssetHash와 전체 visual smoke를 PASS로 기록하지 않는다.
