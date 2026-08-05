# 공식 Balance Provenance · F1 Balance Tool · Map Data v2 RESULT

작성일: 2026-08-05
브랜치: `codex/effect-tool-reboot`

## 1. 결과 요약

다음 계약을 실제 코드와 Data에 반영했다.

1. 5 player profile, 53 skill definition, 54 damage profile, Valtan boss/encounter의 모든 저작
   field를 덮는 field-level provenance receipt를 생성했다.
2. `Publish-GameplayBalance.ps1`이 receipt header/source hash/중복/basis/field count와 현재 JSON
   result를 exact 검증한다.
3. Debug F1에 character/boss 선택형 `Balance Tool`을 추가했다. stats, movement, skill,
   LMB combo stage, Valtan pattern을 편집하고 atomic staging 저장, provenance 동기화, Validate/Publish를
   수행한다.
4. `CClientReplication`이 Server `DAMAGE_EVENT`를 `CCombatHUDViewModel`에 전달하고 Tool이 최근 event를
   표시한다.
5. player defense가 Valtan incoming damage에 실제 적용된다.
6. 네 Area의 `Gameplay.world.json`을 formatVersion 2로 이관하고 `CWorldGameplayDocument`에
   triggerBox/destroyable strict parse/validate/atomic save 구조를 추가했다.

## 2. 공식 provenance

정본:

- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py`
- `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1`

receipt coverage:

```text
playerProfileCount    5
skillDefinitionCount  53
damageProfileCount    54
bossProfileCount      1
encounterPatternCount 1
fieldEntryCount       1058
```

basis count:

```text
OFFICIAL_EXTRACTED 207
OFFICIAL_DERIVED    29
OFFICIAL_SCALED     12
PROJECT_TUNED      810
REFERENCE_ONLY       0
```

`sourceBuildId`, `data2.lpk`, `data3.lpk`, 여섯 EFTable DB, Valtan
`MN_RPBF_01-1.loa`, extractor SHA-256이 저장된다. 개인 절대 경로와 raw payload는 저장하지 않았다.

발탄 basic swing 350, 800/300/1200ms, phase threshold는 계속 `PROJECT_TUNED`다. Valtan action hash가
있다는 이유로 원작 Server pattern timing/damage라고 주장하지 않는다. Balance Tool 변경 field는
자동으로 `balance-tool-authored-override-v1`의 `PROJECT_TUNED`로 바뀐다.

## 3. Balance Tool

진입:

```text
Debug Client -> F1 -> Balance Tool
```

왼쪽은 Players/Bosses 선택, 가운데는 domain별 editor, 오른쪽은 Server live verification이다.
스킬 기준은 level 10이며 원본이 SecondaryKey 1만 가진 fixed basic/awakening definition만 level 1을 사용한다.

- Players: basic stats / movement / slot별 skill / LMB combo stages
- Bosses: base / detection and movement / phase / patterns
- Live: player HP/resource/tick, boss HP/phase/action, 최근 incoming/outgoing damage event
- Save + Validate: exact schema parse → 5 JSON staging/durable write → rollback copy → promote → receipt sync → gameplay+world Validate; 후단 실패도 원본 5종과 receipt 복원
- Publish Server Data: 두 domain을 sibling staging한 뒤 gameplay 1 + world 4 output을 한 rollback set으로 promotion; dirty가 없을 때만 허용, 적용에는 Server restart 필요

Tool이 생성하는 PowerShell process는 `CREATE_NO_WINDOW`, 120초 bounded wait를 사용한다. timeout이면 Tool이
소유한 child만 종료하고 handle을 회수한다. background Server/Client를 만들지 않는다.

## 4. Defense 적용

공식 client payload에는 defense coefficient는 있지만 원작 Server 감산식은 없다. 따라서 다음은
`PROJECT_TUNED` 중앙 계약이다.

```text
raw = max(1, attackPower * damageRatePercent / 100)
applied = max(1, raw * 100 / (100 + playerDefense))
```

`CGameplayCatalog::Apply_Defense`가 유일한 계산점이고 `CValtanBrain`이 target class profile의 defense를
읽는다. contract test에서 raw 350, LanceMaster defense 105가 170으로 적용되고 snapshot damage event도
170인지 확인한다.

## 5. Map Data formatVersion 2

현재 반영:

- 네 Area world JSON version 2 migration
- 기존 playerSpawn/npc/boss 무손실 parse/save
- `triggerBox`: halfExtents, triggerOnce, 최대 32 typed events
- `destroyable`: decimal-string `deployRuntimePlacementId`, initialState
- event: `setCondition(bool)`, `setDestroyableState(INTACT/FRACTURED/DESPAWNED)`
- duplicate placement/deploy binding, invalid stable ID/state, unknown destroyable target, empty trigger Save 거부

64-bit deploy ID는 현재 Valtan 실제 값이 2^53을 넘으므로 JSON number가 아니라 decimal string으로
저장한다.

### 의도적으로 아직 admission하지 않은 것

이번 변경은 Map data 구조와 Client document 계층까지만 반영했다. 다음은 미구현이다.

- MapTool의 trigger/destroyable 저작 widget과 overlay
- world publisher/bootstrap의 신규 kind cook
- Server trigger OBB/typed event authority
- navigation condition에 따른 동적 walkable/path invalidation
- Shared destroyable state replication
- Client deploy presentation state 전환

따라서 기존 `Publish-WorldGameplay.ps1`은 신규 kind를 unknown으로 거부한다. 현재 네 Area는 actor kind만
가지므로 Validate/Publish와 Server 회귀가 통과한다. parser가 생겼다는 이유로 trigger gameplay를 완료로
표기하지 않았다. 후속 구현 정본은
`2026-08-05_MAP_GAMEPLAY_TRIGGER_DESTROYABLE_NAV_EXTENSION_PLAN.md`다.

## 6. 실행 검증

현재까지 실행한 검증:

```text
PASS  python -m py_compile Export-OfficialBalanceReceipt.py
PASS  receipt generation: 5/53/54/1/1, fields 1058
PASS  Update-BalanceProvenanceReceipt.ps1 (changed fields 0)
PASS  Publish-GameplayBalance.ps1 -Mode Validate
PASS  Publish-WorldGameplay.ps1 -Mode Validate (Bern 4, Valtan 5, Training 4, Character Select 5)
PASS  Client Debug compile/link
PASS  Server Debug compile/link
PASS  Server.exe --contract-test, failures : 0
PASS  post-test Client.exe/Server.exe process list empty
PASS  Debug/Release Engine/Shared/Server/Client compile/link
PASS  Debug/Release NetworkProtocolHarness, ClientFrontendHarness, Server contract: failures 0
PASS  Balance runtime set normal publish + promotion 3/5 injected failure full rollback, transaction leftovers 0
PASS  ProjectAudit 신규 Character Select/balance/world/navigation 계약
BLOCKED ProjectAudit 전체 exit: local Resources inventory mismatch + 별도 미완성 Effect Tool G4 audit
```

실제 F1 시각/편집 smoke와 최신 Lobby 승인 Character Select round-trip은 이 세션에서 사람이 실행하지
않았으므로 자동 PASS와 분리한다. `git diff --check`와 PR 결과는 최종 commit 직전에 확인한다.

### 6.1 Windows CRLF provenance 검증 복구

Windows checkout에서 `Export-OfficialBalanceReceipt.py`가 CRLF 391개를 가진 상태였고, receipt는 같은
소스의 LF 바이트 SHA-256을 저장하고 있었다. 기존 publisher가 working file의 raw 바이트를 비교해
`Balance provenance receipt is stale for the current extractor.`로 Server pre-build를 중단했다.

`Publish-GameplayBalance.ps1`은 이제 UTF-8 extractor 텍스트의 `CRLF`와 단독 `CR`을 `LF`로
정규화한 뒤 SHA-256을 비교한다. `.gitattributes`는 신규 checkout의 해당 extractor 한 파일을 LF로
고정한다. Balance JSON, 공식 receipt, 원본 source hash와 생성된 행 계약은 변경하지 않았다.

```text
PASS  working raw hash 13a4c153... / canonical hash 17a4bd65... = receipt hash
PASS  python -m py_compile Export-OfficialBalanceReceipt.py
PASS  Publish-GameplayBalance.ps1 -Mode Validate (5 players, 72 skills, 54 damage, 1 boss)
PASS  Publish-GameplayBalance.ps1 -Mode Publish
PASS  Server Debug pre-build + compile/link, warnings 0, errors 0
PASS  Server.exe --contract-test, failures : 0
BLOCKED ProjectAudit 전체 exit: 기존 asset-lock inventory, Effect Tool G1, 누락 DimensionMaster resource 관련 4건
```

ProjectAudit의 네 실패는 이 줄바꿈 수정 파일과 무관하며, audit 안에서 실행된 gameplay balance
Validate/Publish는 모두 통과했다.

## 7. 담당자 인계

처음 작업하는 담당자는 다음 순서로 읽는다.

1. `.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`
2. `.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md`
3. `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
4. `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
5. 이 RESULT와 연결 MAP PLAN

원본 LPK/DB를 갖지 않은 담당자는 receipt를 수동으로 공식 승격하지 않는다. Balance Tool authoring과
`PROJECT_TUNED` 편집은 가능하며, Save + Validate → Publish → Server restart → live damage event 순서로
검증한다.
