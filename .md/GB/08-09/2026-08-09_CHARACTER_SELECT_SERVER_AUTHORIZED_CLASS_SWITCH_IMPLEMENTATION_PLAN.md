# Character Select Server 권위 통합·런타임 클래스 변경 구현 계획서

작성일: 2026-08-09
대상 브랜치: `codex/character-select-server-unification`

## 목표

Character Select의 socket 없는 `Preview`와 Lobby 승인 `Server Play`를 하나의 Server 권위 Level로 통합한다.
Lobby의 `Character Select` 명령은 `WORLD_ID::CHARACTER_SELECT_ARENA` 입장 승인을 받은 뒤에만
`LEVEL::CHARACTER_SELECT`를 연다. Level 안에서는 기존 class 썸네일을 누르는 즉시 선택 class 변경
의도를 typed player command 경계로 제출하고, Server가 같은 player/entity의 class와 전투 상태를
교체한 snapshot을 보낸 뒤에만 Client presentation을 commit한다.

이번 수직 슬라이스의 실행 목표는 Client 한 대에서 여섯 class를 연속 변경하고 각 class의 quick-slot/LMB
스킬을 Server approval → snapshot → Character presentation 경로로 시험하는 것이다.

## 인터뷰에서 확정한 요구사항

- socket 없는 Preview와 `Preview / Server Play` mode 선택 UI를 제거한다.
- Server 연결 실패·입장 거부·5초 승인 timeout은 기존처럼 Lobby에 남는다. local fallback은 없다.
- class 썸네일 선택은 별도 Confirm 없이 즉시 Server class 변경 요청을 제출한다.
- 살아 있는 player는 같은 world 위치와 같은 `PlayerId/NetEntityId`를 유지한다.
- 사망 player는 기존 `strSpawnPlacementId`의 enabled `PLAYER_SPAWN`을 검증하고 최초 입장과 동일한
  navigation projection을 거친 Server spawn transform으로 복귀한다.
- class 변경 성공 시 이동, trigger move, action, skill, combo/hold, stance, cooldown, HP/resource를 새
  class profile 기준으로 초기화한다.
- action 중이거나 사망 중이어도 Character Select Arena에서는 변경을 허용한다.
- 요청 거부 또는 요청 전 새 class asset admission 실패는 기존 character를 유지하고 상태 문자열을 남긴다.
- 마지막 Server 승인 class는 Character Select에서 Bern/Valtan으로 나갈 때 다음 `C2S_ENTER_WORLD` class가 된다.
- 일반 몬스터, Valtan spawn 확장, multi-client 전용 동시성·late-join 정책은 이번 범위가 아니다.
- 자동/수동 완료 목표는 Client 한 대의 연속 class 변경과 변경 후 스킬 사용이다.

## 현재 코드 실측

- `CLevel_Lobby::Begin_StageRequest`는 `LOBBY_STAGE::CHARACTER_SELECT`만 network admission을 건너뛰고
  곧바로 Level을 연다.
- `CLevel_CharacterSelect::MODE`는 `PREVIEW / CONNECTING / SERVER_ARENA / RETURNING_TO_LOBBY`로
  분리돼 있고 `Select_Preview`는 `PREVIEW`에서만 동작한다.
- `CCharacterSelectionState`의 `CHARACTER_TEST_ENTRY_MODE`는 Preview → TEST → Server Arena one-shot
  handoff를 구분하기 위해 존재한다.
- Shared protocol version은 12이고 class는 `C2S_ENTER_WORLD`와 `S2C_PLAYER_SPAWNED`에만 있다.
  `PLAYER_SNAPSHOT`에는 class가 없어 같은 entity의 런타임 class 교체를 표현할 수 없다.
- Server `SERVER_PLAYER::eCharacterClass`는 입장 때 고정되고 `CPlayerSkillSystem`이 매 command에서 class와
  skill definition 일치를 검사한다.
- `CClientReplication`은 spawn 때 class별 `CCharacter`를 만들고 이후 snapshot에서는 같은 객체에
  transform/action만 적용한다.
- `CLoader::Ready_For_CharacterSelect`는 선택 class 하나를 준비한 뒤 Debug에서
  `Ready_AnimationPreviewModels`로 여섯 class를 다시 선로드한다. 제품 Area loader의 선택 class 우선
  admission 계약과 맞지 않는다.
- 기존 worktree에는 다른 Effect 작업의 대규모 미커밋 변경이 있어 최신 `origin/main`에서 독립 worktree와
  Codex 브랜치를 만들었다. 이번 변경은 원래 worktree를 수정하지 않는다.

## 범위 밖

- Valtan/일반 몬스터 spawn UI와 Server runtime 변경
- Character Select 밖 Bern/Valtan Arena에서 class를 변경하는 기능
- multi-client 전용 request arbitration, 준비 완료 handshake, 별도 reconnect/late-join 최적화
- 새 UI schema, `CUIObject` command binding, Confirm 버튼 기능 추가
- 모든 class asset 선로드나 두 번째 character/model runtime

## 전체 호출 흐름

```text
Lobby Character Select
-> CLobbyCommandService::Request(CHARACTER_SELECT)
-> CLevel_Lobby::Resolve_Stage
-> CNetworkManager::Send_EnterWorld(CHARACTER_SELECT_ARENA, selected class)
-> Server S2C_ENTER_ACCEPTED
-> LEVEL::CHARACTER_SELECT load
-> queued spawn/snapshot을 CClientReplication이 소비
-> Server local CCharacter + Controller + camera bind
```

```text
class thumbnail click
-> CLevel_CharacterSelect::Request_CharacterClassChange
-> target class CPlayableCharacterAssetService admission
-> IPlayerCommandSink::Request_ChangeCharacterClass
-> CNetworkManager::Send_ChangeCharacterClass
-> C2S_CHANGE_CHARACTER_CLASS
-> CServerApp -> ROOM_COMMAND::CHANGE_CHARACTER_CLASS
-> CGameRoom::Handle_ChangeCharacterClass
-> staged SERVER_PLAYER reset/commit
-> S2C_CHARACTER_CLASS_CHANGE_RESULT
-> 다음 S2C_WORLD_SNAPSHOT의 PLAYER_SNAPSHOT.eCharacterClass
-> CClientReplication이 새 CCharacter stage
-> registry same-slot atomic replace + old layer object 제거
-> 실패 시 old record/character rollback
-> CLevel_CharacterSelect가 Controller/camera/AnimationTarget를 새 character에 재bind
-> CCharacterSelectionState에 Server 승인 class commit
```

## G00. Shared protocol v13 class 변경 계약

### 수정 파일

- `Shared/Public/Network/PacketType.h`
- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`

### 변경 계약

- `NETWORK_PROTOCOL_VERSION`을 12에서 13으로 올린다.
- packet type에 `C2S_CHANGE_CHARACTER_CLASS`, `S2C_CHARACTER_CLASS_CHANGE_RESULT`를 추가한다.
- `C2S_CHANGE_CHARACTER_CLASS`는 0이 아닌 `clientSequence`와 요청 `CHARACTER_CLASS_ID`만 가진다.
  PlayerId/NetEntityId는 보내지 않고 Server가 인증 session으로 player를 찾는다.
- `CHARACTER_CLASS_CHANGE_RESULT`는 `ACCEPTED`, `REJECTED_WRONG_WORLD`,
  `REJECTED_STALE_SEQUENCE`, `REJECTED_UNSUPPORTED_CLASS`, `REJECTED_SAME_CLASS`,
  `REJECTED_STATE`를 명시한다.
- `S2C_CHARACTER_CLASS_CHANGE_RESULT`는 요청 sequence, result, requested class, 현재 Server active class를
  돌려준다. 거부에서도 Client가 기존 class를 명확히 유지할 수 있다.
- `PLAYER_SNAPSHOT`에 `eCharacterClass`를 추가한다. class는 edge event가 아니라 Server의 지속 상태이므로
  모든 snapshot과 late state가 같은 정답을 가진다.
- writer/reader는 local temporary에 전부 decode하고 sequence, enum 범위, supported active class와 trailing
  byte를 검증한 뒤에만 output을 commit한다.

### 종료 증거

- 새 C2S/S2C round-trip
- DESTROYER처럼 protocol enum에는 있지만 playable admission이 아닌 요청의 wire round-trip
- zero sequence, `END` class/result, truncated/trailing payload 거부와 destination 보존
- class가 포함된 world snapshot round-trip과 기존 size/count gate 통과

## G01. Server 권위 class 변경과 상태 초기화

### 수정 파일

- `Server/Public/ServerPlayer.h`
- `Server/Public/RoomCommand.h`
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerApp.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`

### H 계약

- `SERVER_PLAYER::iLastClassChangeSequence`가 session player별 replay/stale request 경계를 소유한다.
- `ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS`와 `ROOM_COMMAND::ChangeCharacterClass`가 transport thread에서
  room tick으로 새 intent를 전달한다.
- `CGameRoom::Handle_ChangeCharacterClass`는 world/session/sequence/target 검증과 result 전송을 소유한다.
- `CGameRoom::Apply_CharacterClassChange`는 `SERVER_PLAYER` 전체 복사본에 새 profile과 reset을 stage하고,
  모든 검증 성공 뒤 한 번만 원본 player를 교체한다.
- `Send_CharacterClassChangeResult`는 요청 session 한 곳에 typed 결과만 전송한다.

### Server reset 불변식

- `WORLD_ID::CHARACTER_SELECT_ARENA`에서만 승인한다.
- `iSessionId`, `iPlayerId`, `iNetEntityId`, nickname, spawn placement ID와 alive player 위치/yaw는 유지한다.
- dead player만 `strSpawnPlacementId`의 position/yaw로 돌아간다. spawn이 사라졌으면 기존 player를 유지하고
  `REJECTED_STATE`를 보낸다.
- 새 profile의 max/current HP, max/current resource, move speed, default stance를 적용한다.
- 이동 목표/path, trigger move, action/skill/timing, combo/hold flag, damage flag, cooldown, regen carry를
  초기화한다.
- 기존 move/skill/revive sequence와 새 class-change sequence는 session replay 경계이므로 reset하지 않는다.
- 성공 뒤 `CServerTriggerSystem::Remove_Player`로 이전 enter/trigger 상태를 제거한다.
- 같은 tick의 다음 snapshot부터 새 class와 초기화 state를 broadcast한다.

### 종료 증거

- alive class 변경: identity/position 유지, profile/state/cooldown 초기화
- dead class 변경: 기존 spawn transform 복귀와 full HP/resource
- unsupported/same/stale/wrong-world 요청: 기존 player byte 의미 상태 보존과 명시 result
- 변경 직후 새 class skill은 승인되고 이전 class skill은 Server skill gate에서 거부됨

## G02. Client transport와 transactional presentation 교체

### 수정 파일

- `Client/Public/PlayerCommandSink.h`
- `Client/Public/NetworkPlayerCommandSink.h`
- `Client/Private/NetworkPlayerCommandSink.cpp`
- `Client/Public/NetworkManager.h`
- `Client/Private/NetworkManager.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`

### H 계약

- `IPlayerCommandSink::Request_ChangeCharacterClass`가 Level/UI intent와 socket을 분리한다.
- `CNetworkPlayerCommandSink`만 이를 `CNetworkManager::Send_ChangeCharacterClass`로 번역한다.
- `CNetworkManager`는 class change result queue를 소유하고 main thread의
  `Try_Consume_CharacterClassChangeResult`에 전달한다.
- `CClientReplication::Replace_CharacterClass`는 snapshot class mismatch를 발견했을 때 새 presentation의
  stage/commit/rollback을 소유한다.

### Client 교체 불변식

- local thumbnail 처리에서 target class prototype을 먼저 admission한다. 실패하면 packet을 보내지 않고
  기존 character/status를 유지한다.
- snapshot class가 registry record와 같으면 기존 fast path를 사용한다.
- 다르면 snapshot transform/yaw와 기존 nickname/identity로 새 `CCharacter`를 먼저 layer에 stage한다.
- stage 성공 뒤 `CNetObjectRegistry::Replace`가 같은 handle/generation과 지원 class를 검증한 다음
  같은 NetEntityId slot의 record/character를 한 번에 교체한다.
- registry replace 뒤 old layer 제거가 실패하면 같은 atomic replace 경계로 old record/character를 복원하고
  새 object를 제거한다.
- local handle은 commit된 새 handle로만 교체한다. Server class를 Client가 낙관적으로 먼저 적용하지 않는다.
- accepted result는 NetworkManager의 local class metadata를 갱신하지만 실제 Character/HUD commit은 class가
  들어 있는 snapshot을 기준으로 한다.

### 종료 증거

- snapshot class 동일 시 object 교체 없음
- class mismatch 성공 시 identity 유지 + 새 spec/skill/HUD 적용
- prototype/stage/register/layer 중간 실패 시 기존 record/character 유지
- rollback 성공은 recoverable presentation 결과로 Level에 노출해 socket을 닫지 않고 상태 메시지를 표시한다.
  기존 `Update()==false`는 rollback도 불가능한 fatal replication 실패에만 사용한다.
- result decode 실패나 rejection은 presentation을 바꾸지 않음

## G03. Character Select 단일 Server Level과 썸네일 변경

### 수정 파일

- `Client/Public/CharacterSelectionState.h`
- `Client/Private/CharacterSelectionState.cpp`
- `Client/Private/Level_Lobby.cpp`
- `Client/Public/Level_CharacterSelect.h`
- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Private/Loader.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

### 진입 계약

- `CHARACTER_TEST_ENTRY_MODE`와 stage/consume/clear API를 삭제한다.
- Lobby의 Character Select special local-load 분기를 삭제하고 `Resolve_Stage`에서
  `CHARACTER_SELECT -> CHARACTER_SELECT_ARENA / LEVEL::CHARACTER_SELECT`로 해석한다.
- Lobby는 기존 class 또는 명시적인 Lance Master 기본값으로 `C2S_ENTER_WORLD`를 보내며 승인 실패 시
  Lobby에 남는다.
- Character Select Level은 항상 `CONNECTING`으로 시작하고 queued spawn/snapshot의 local character가
  생긴 뒤 `SERVER_ARENA`로 commit한다. local preview object는 만들지 않는다.
- camera는 초기 arena 기준 eye/at으로 먼저 생성하고 local replicated character가 준비되면 follow target을
  bind한다.

### class UI 계약

- `Preview / Server Play` radio와 Preview 복귀 문구/command를 제거한다.
- 기존 ImGui class 목록과 `Render_ClassList` 썸네일은 `SERVER_ARENA` 안정 상태에서 활성화한다.
- 여섯 class 모두 accordion/thumbnail hit target을 제공한다. 현재 전용 illustration asset이 없는
  Gunslinger/Slayer는 Common fallback tile과 class label을 사용하고 전용 asset rollout은 별도 UI 범위로 남긴다.
- 현재 Server 확정 class와 pending target index를 분리한다. pending 중에는 중복 요청을 막고 기존 class를
  계속 highlight/present한다.
- target asset admission 성공 뒤 Level이 자체 class-change sequence를 증가시키고 typed sink에 요청한다.
- rejection/5초 result timeout은 pending만 해제하고 기존 character를 유지한다.
- snapshot 교체를 관찰하면 `CAnimationTargetService`, camera, `CPlayerController`를 새 local character에
  재bind하고 `CCharacterSelectionState`에 확정 class를 commit한다.
- 동일 entity presentation 재bind는 move/skill sequence를 보존하고 held/release input edge만 초기화한다.
  일반 Level 진입의 sequence reset과 분리된 `CPlayerController::Rebind_LocalCharacter`를 사용한다.
- result는 pending sequence와 requested class가 모두 일치할 때만 pending을 변경한다. wire reader는
  `ACCEPTED`와 `REJECTED_SAME_CLASS`에서 requested/active class 일치도 검증한다.
- Bern/Valtan 버튼은 기존 Lobby 재승인 경계를 사용하고 확정 class를 다음 entry에 전달한다.
- disconnect/replication fatal failure는 기존처럼 state를 정리하고 Lobby로 복귀한다.

### Loader 계약

- Character Select loader는 입장 class 하나만 `Ready_Character_Rendering`으로 준비한다.
- Character Select에서 `Ready_AnimationPreviewModels` 전체 선로드를 제거한다. 변경 class는
  `CPlayableCharacterAssetService` 한 경로로 선택 시 admission한다.
- Development의 Animation/Effect preview tool용 `Ready_AnimationPreviewModels` 경로는 유지한다.

### 종료 증거

- Frontend harness에서 Character Select command가 tokenized gameplay command로 한 번 소비됨
- stale Preview/Server handoff state와 mode 문자열/API 0건
- Character Select Level 내부 `Connect_To_Server`, `Send_EnterWorld`, `Try_Consume_EnterAccepted` 0건
- Lobby가 Character Select도 Server approval 대상으로 resolve함

## G04. Audit와 public 문서 정본 갱신

### 수정 파일

- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `AGENTS.md`
- `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`

### 변경 계약

- `levels.character-select-contract`를 offline Preview/Server Play 분리 검사가 아니라 Lobby 승인 단일 진입,
  typed class change command, snapshot class, transactional presentation 교체, selected-class-only loader 검사로
  바꾼다.
- camera audit 문구를 Preview/Server 공유가 아닌 initial arena framing과 replicated target rebind 계약으로
  바꾼다.
- 고정 런타임 문서에서 socket 없는 Preview, tokenized TEST 왕복, Preview 복귀 설명을 제거한다.
- Animation 담당 절차는 Character Select 진입 직후 Server character를 Scene Character target으로 사용하고,
  class 썸네일 변경 뒤 같은 Level에서 새 target으로 교체되는 흐름을 설명한다.
- Area guide는 Character Select Arena가 항상 gameplay world이며 Area loader는 입장 class 하나만 준비하고
  다른 class는 실제 선택/snapshot에서 lazy admission한다고 기록한다.
- 일반 몬스터/Valtan과 multi-client 확장은 별도 경계로 남긴다.

## 프로젝트 등록

새 C++ 파일은 만들지 않는다. Client 제품 project 등록은 바뀌지 않는다. 다만 same-entity registry 교체의
실행 rollback 증거를 위해 기존 `Client/Private/NetObjectRegistry.cpp`를 `ClientFrontendHarness.vcxproj`와
`.vcxproj.filters`에 추가 등록하고 Debug/Release 하네스 빌드로 검증한다.

Client rollback은 `CNetObjectRegistry::Replace`의 validate-before-commit seam으로 분리하고 성공 교체와
invalid replacement의 기존 record/object/handle 보존을 FrontendHarness에서 실행 검증한다. layer 제거 실패
rollback은 제품 코드의 same seam 재사용과 audit로 연결하되 실제 Engine layer 실패 주입은 수동 미검증으로 남긴다.

## 구현 순서

1. Shared packet type/message/snapshot layout과 protocol harness
2. ServerApp room command dispatch와 GameRoom staged class reset/result
3. NetworkManager/sink result transport
4. ClientReplication snapshot class replacement/rollback
5. Lobby admission과 Character Select Level/UI 단일화
6. CharacterSelectionState/Loader/Frontend harness의 stale Preview 경계 제거
7. ProjectAudit와 public 문서 갱신
8. Debug compile/harness부터 오류를 수정한 뒤 Release 전체 회귀

## 자동 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

최소 기대 결과:

- NetworkProtocolHarness Debug/Release `failures : 0`
- ClientFrontendHarness Debug/Release `failures : 0`
- `Server.exe --contract-test` Debug/Release `failures : 0`
- Shared, Server, Client Debug/Release compile/link 성공
- ProjectAudit 신규 Character Select class-change 계약 PASS
- JSON/XML parse 오류 없음

## 수동 Server+Client smoke

Client 작업 디렉터리는 `Client/Default`를 사용한다.

1. `Framework.slnLaunch`의 `Server + Client` profile로 시작한다.
2. Lobby에서 `Character Select`를 누르고 Server 승인 뒤에만 Level이 열리는지 확인한다.
3. Lance Master에서 다른 class 썸네일을 연속 선택한다.
4. 각 변경이 승인될 때 같은 위치·같은 player identity에서 모델/HUD/스킬 row가 교체되는지 확인한다.
5. 각 class에서 quick-slot 또는 LMB를 입력해 Server snapshot action과 실제 clip 재생을 확인한다.
6. action 도중 다른 class를 선택해 action/cooldown/resource가 초기화되고 새 class skill이 동작하는지 확인한다.
7. Server를 종료해 replicated state가 정리되고 Lobby로 복귀하는지 확인한다.

Valtan 소환, 일반 몬스터, 두 Client 동시 class 변경은 이번 RESULT의 PASS 조건으로 기록하지 않는다.

## RESULT 기록 기준

`2026-08-09_CHARACTER_SELECT_SERVER_AUTHORIZED_CLASS_SWITCH_RESULT.md`에는 계획이 아니라 실제 diff와 실행
증거만 기록한다. 구현 완료, 자동 검증, 수동 검증, 남은 multi/monster/Valtan 경계를 분리하고 실행하지 않은
smoke를 PASS로 쓰지 않는다.
