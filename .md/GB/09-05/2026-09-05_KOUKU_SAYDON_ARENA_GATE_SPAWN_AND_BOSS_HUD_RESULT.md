# 2026-09-05 KoukuSaydon Arena 관문별 스폰·플레이어 배치·보스 HUD 결과

> 대응 계획: [구현 계획서](2026-09-05_KOUKU_SAYDON_ARENA_GATE_SPAWN_AND_BOSS_HUD_IMPLEMENTATION_PLAN.md) (§7에 검토 후 추가 반영)
>
> 브랜치: `codex/kouku-scale-1p7`. 같은 worktree의 다른 세션 hunk(Logic/Summon Workbench, composition revision 28, MN_RPCT_03 등록)는 보존했다.
>
> 자동 검증 범위와 현재 DRAFT/PRODUCT 구분은 아래 재검토 항목을 따른다. Client 실행과 Boss Tool 화면 재생은 미검증이며 사용자가 수행한다.

## 사용자 결정 반영

| 항목 | 값 |
|---|---|
| 대형 세이튼 `bodyModelPreScale` | **0.1** (계획 1.0에서 10배 축소) |
| 빙고 뿅망치 `weaponModelPreScale` | **0.01** (로드 시 1/100이 빠져 있었음) |
| HUD | KoukuSaydon Arena에서 플레이어 HUD·보스 체력바·보스 제목 표시 |
| 관문 버튼 | 누른 관문은 비활성, 다른 관문을 누르면 그 보스의 HUD(이름·체력)로 전환. 이전 teleport 응답 대기 중에는 모든 관문 버튼 비활성 |
| 패턴 재생 대상 | **관문 버튼이 올린 보스**. 1관문 세이튼(`boss.kakulsaydon.g1.saydon`), 2관문 쿠크, 3관문 세이튼, 빙고 세이튼. Despawn 뒤에는 시작 지점 쿠크로 복귀 |
| Workbench Stage 이동 | Stage와 animation box는 묶여 있음. `< Earlier` / `Later >` 버튼 또는 Left/Right 키로 한 칸 이동 |
| 대형 세이튼 튜닝 | F1 `KoukuSaydon Arena` 하단 임시 슬라이스(live 배율·offset·뿅망치 배율, Save/Reload). 값 확정 뒤 제거 |

## 반영 파일

### 데이터·publisher

| 파일 | 변경 |
|---|---|
| `Data/Actors/BossCatalog.json` | `BOSS_KAKULSAYDON_G1_SAYDON`(MN_RPCT_05, 0.017), `G2_BIG_SAYDON`(MN_RPCT_06, 0.1), `G2_KOUKU`(MN_RPCZ_00, 0.017), `G3_SAYDON`(MN_RPCT_05, 0.017), `BINGO_SAYDON`(MN_RPCT_05 + `WP_MN_RPCT_06` 0.01) |
| `Data/Balance/BossProfiles.json` | 같은 5 archetype, `ENCOUNTER_KAKULSAYDON_G1`, HP 600000 / 160줄, attackPower 100, collisionRadius 1.0(대형 세이튼 4.0), displayName `세이튼 / 대형 세이튼 / 쿠크 / 세이튼 / 앵콜을 외친 쿠크세이튼` |
| `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | updater 실행 결과(55 field `PROJECT_TUNED`, `bossProfileCount 8`) |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | disabled boss placement 5개, revision 1868 |
| `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`, `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` | projector publish, sourceRevision 28. PIZZA에 `bossArchetypeIds [G1_KOUKU, G2_KOUKU]` |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | `arena_boss_archetypes_by_profile(root)`가 BossCatalog `bodyModel` join으로 actor body → archetype 목록을 만든다. `validate_document`는 순수 검증을 유지하고(파일 I/O 없음), `validate_publishable(document, root)`가 PRODUCT actor body가 arena boss body인지 검사, `project_encounter(document, root)`가 pattern별 `bossArchetypeIds`를 투영 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | 임시 root fixture에 BossCatalog 복사, `test_actor_without_arena_boss_body_cannot_publish`(순수 검증 통과·publish 시 거부·투영 목록 확인) |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | Kouku family weapon 규칙, pattern `bossArchetypeIds`(1~8, 같은 encounter의 `BOSS_KAKULSAYDON_*` profile) 검증과 `PATTERNBOSS` 행 emit |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | encounter별 boss archetype 집합 join(BossProfiles encounterId), Kouku pattern `bossArchetypeIds` optional property |
| `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1` | Kouku audition note를 `boss:BOSS_KAKULSAYDON_*`로 |
| `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | weapon null 집합 5개, 빙고 hammer, `bossProfileCount 8`, world revision 1868 |
| `CLAUDE.md` | 관문 보스 placement·F1 gate 명령 순서·`PATTERNBOSS`/`bossArchetypeIds`·Debug 전용 경계 한 단락 |

### Server

| 파일 | 변경 |
|---|---|
| `Server/Public/KoukuSaydonBrain.h`, `Server/Private/KoukuSaydonBrain.cpp` | `KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX`, `Is_ArenaBoss`, `Is_ArenaBossPlacement` |
| `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | `BOSS_PATTERN_DEFINITION::AuditionBossArchetypeIds`, `PATTERNBOSS` 행 parse(중복·8개 상한), finalize에서 항목이 같은 encounter의 arena boss인지 검사. `isKoukuSaydonGateOne`을 `ENCOUNTER_KAKULSAYDON_G1` + `BOSS_KAKULSAYDON_*` family로 정의해 부위 없음·MECHANIC audition·phase action 0 검사 3곳이 새 보스에도 적용(Codex P1) |
| `Server/Private/GameRoom.cpp` | `Update_WorldEntities`: arena boss는 audition이 다른 보스를 소유 중이면 pin만, 아니면 `Update_KoukuSaydonBoss`(brain 없이 IDLE 대기). `Handle_SpawnWorldEntity`/`Handle_DespawnAllWorldEntities`: KAKULSAYDON은 Debug 전용(Codex P2), disabled `Is_ArenaBossPlacement`만 admission, `Despawn_KoukuSaydonArenaDebugEntities`가 disabled placement 출신 entity(+dependent)만 제거하고 제거 보스가 audition 소유자면 ABORTED lifecycle 후 audition 정리. `Evaluate_KoukuSaydonPatternAudition`: scope placement가 arena boss placement(또는 enabled 시작 쿠크)면 `Find_KoukuSaydonArenaBoss`로 대상 결정, pattern의 `AuditionBossArchetypeIds`에 대상 archetype이 없으면 `REJECTED_UNSUPPORTED_PATTERN`(Play All은 sequence 전 pattern 검사), 빈 목록은 legacy 시작 쿠크만 |
| `Server/Public/GameRoom.h` | `Despawn_KoukuSaydonArenaDebugEntities`, `Find_KoukuSaydonArenaBoss` |
| `Server/Private/ServerGameplayContractTests.cpp` | 4건 추가: disabled placement admission, gate boss 90 tick(`m_iServerTick` 증가) IDLE·pattern 없음·audition boss 아님, Saydon 대상 PIZZA audition `REJECTED_UNSUPPORTED_PATTERN`, revert 후 시작 쿠크만 잔존 |

### Client

| 파일 | 변경 |
|---|---|
| `Client/Private/ActorCatalog.cpp` | 무기 없는 보스 허용을 `BOSS_KAKULSAYDON_*` + `boss.kakulsaydon.*.client.v1` family로 확장(Codex P1: 이전에는 첫 세이튼 row에서 전체 catalog 초기화가 실패) |
| `Client/Public|Private/Npc.*` | optional socketed weapon(rest pose, `socket bone × world`), Debug 튜닝 hook(`Set_DebugPresentationScale/Offset`, `Set_DebugWeaponScale`) |
| `Client/Public|Private/KoukuSaydonPresentationAssetService.*` | family archetype prototype, weapon prototype(`weaponModelPreScale` pre-transform), Product binding을 archetype별로 로드하며 body에 있는 clip만 채택, `Try_Resolve_Action(archetype, action)` |
| `Client/Private/ClientReplication.cpp`, `Client/Public/ClientReplication.h` | `Is_KoukuSaydonArenaBoss` helper로 spawn/snapshot/생존/despawn 일반화, weapon desc, despawn 시 HUD clear, `Find_ArenaBossNpc` |
| `Client/Public|Private/CombatHUDViewModel.*` | boss focus archetype, `Clear_BossIfArchetype` |
| `Client/Public|Private/KoukuSaydonPatternAuditionService.*` | `Set_TargetBoss`/getter, 요청 scope를 `m_RequestScope`에 보관해 결과·lifecycle 대조, 거부 문구 갱신 |
| `Client/Public|Private/PlayerController.*` | `Request_DebugTeleportToPosition(world, x, y, z)` |
| `Client/Public|Private/Level_KakulSaydonArena.*` | 관문 table(HUD focus + audition placement), `Debug_ActivateGate`(pending teleport 사전 거부 → despawn → spawn → teleport → HUD focus → audition target → 활성 관문 기록), `Debug_DespawnArenaBosses`(focus·target·활성 관문 reset), spawn result 소비, `Debug_FindArenaBossNpc` |
| `Client/Public|Private/MainApp.*` | HUD gate 4곳, 보스 제목 displayName, `Authoring Sources` 제거, `KoukuSaydon Arena`(활성 관문·pending 비활성·HUD focus·audition target·상태), `Big Saydon / Hammer Tuning (temporary)`(live multiplier, Save는 JSON 숫자만 patch 후 parse 재검증, Reload), `KoukuSaydon Complete Play`(관문 콤보, Saved Patterns, Complete Play / Complete Play All, audition target 표시) |
| `Client/Public|Private/KoukuSaydonBossTool.*` | accessor 공개, `Play_All` public, 라벨 `Play Isolated` / `Start Full Pattern` |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | PRODUCT actor 사전 검사를 `Is_ArenaBossBodyProfile`(MN_RPCZ_00 / MN_RPCT_05 / MN_RPCT_06)로 완화. 정본 join은 projector |
| `Client/Public|Private/KoukuSaydonActionWorkbench.*` | `Move_SelectedStage`, Selected Box `< Earlier` / `Later >`, timeline Left/Right 키 |

## 자동 검증

| 검사 | 결과 |
|---|---|
| `Update-BalanceProvenanceReceipt.ps1` | 55 field PROJECT_TUNED |
| `Publish-ServerNavigation.ps1 -Mode Validate` | 5 area succeeded (새 boss placement 5개 walkable·높이 통과) |
| `Publish-WorldGameplay.ps1 -Mode Publish` | succeeded, `KAKULSAYDON_ARENA.worldbootstrap`에 disabled boss 5행 |
| projector `--mode publish` | sourceRevision 28, PIZZA `bossArchetypeIds` 2개 |
| `Publish-GameplayBalance.ps1 -Mode Publish` | succeeded, bootstrap에 `PATTERNBOSS` 2행 |
| Kouku python 3 모듈 | 53 tests OK |
| Debug Product 빌드 (`Invoke-BuildAndRegression.ps1 -Configuration Debug`) | PASS (`out/BuildPipeline/runs/20260905T132510281Z-debug-product.json`, `out/KoukuArena/product-build-2.log`, error 0) |
| `Server.exe --contract-test` | `failures : 0` (`out/KoukuArena/contract-test-2.log`). 새 검사 4건 PASS: disabled placement admission, gate boss 90 tick IDLE, Saydon 대상 PIZZA 거부, revert 후 시작 쿠크 잔존. 첫 실행(`contract-test-1.log`)은 Codex P1 지적대로 "Boss has no part definitions"로 실패했고 로더 family 일반화 뒤 통과 |
| `git diff --check` | PASS |
| 편집 C++/ps1/py 인코딩·줄바꿈 | 원래 UTF-8·CRLF 유지. receipt JSON만 updater 도구 출력 그대로(git이 LF로 정규화) |

## 사용자 확인 절차 (미실행)

1. `Framework.slnLaunch`의 `Server + Client`로 새 빌드를 시작한다(Server가 새 bootstrap을 읽어야 한다).
2. Lobby → KoukuSaydon → F1 → `KoukuSaydon Arena` → `1관문 - 세이튼`. 버튼이 비활성(active)이 되고 상태창에 `boss.kakulsaydon.g1.saydon: spawned`와 `Server moved player to (-2.84, 1.32, 941.02)`, HUD `세이튼` 600000 / X 160, 스킬로 HP 감소를 확인한다.
3. `2관문 - 대형 세이튼, 쿠크`: 대형 세이튼(0.1)·쿠크, HUD `쿠크`. 하단 튜닝 슬라이스에서 대형 세이튼 배율·offset·(빙고 관문에서) 뿅망치 배율을 맞춘 뒤 `Save Tuning` → BossCatalog/world JSON에 기록. 위치는 `Publish-WorldGameplay.ps1` + Server 재시작, 배율은 다음 Client 실행에 적용.
4. `3관문`, `1마리오`, `빙고`(뿅망치 든 세이튼, HUD `앵콜을 외친 쿠크세이튼`) 확인. 빙고 플레이어 지점이 거부되면 상태창 사유를 알려 준다.
5. 관문 세이튼을 올린 상태에서 Workbench에서 세이튼 패턴(예: DANCE_TIME)을 PRODUCT로 두고 `Publish All PRODUCT` → `Publish-GameplayBalance.ps1` → Server 재시작 → Boss Tool `Play Isolated`/`Start Full Pattern` 또는 F1 Complete Play. PIZZA(쿠크 body)를 세이튼에 재생하면 `REJECTED_UNSUPPORTED_PATTERN`이 정상이다.
6. Workbench Sequencer에서 Stage(또는 그 animation box) 하나를 선택하고 Left/Right 또는 `< Earlier`/`Later >`로 순서를 옮긴 뒤 Save → Reload로 순서가 남는지 확인한다.

## 남은 경계

- Product를 세이튼 patterns로 publish하려면 해당 pattern이 Logic/Summon box 없이 PRODUCT여야 한다(무력화 Logic은 별도 계획).
- 튜닝 슬라이스는 값 확정 뒤 제거한다. 관문 전환은 "이전 teleport 응답 전 거부"로 순서 불일치를 막지만 Server 측 원자 트랜잭션은 아니다(spawn 성공 + teleport navigation 거부는 상태창에 각각 표시된다).
- MN_RPCT_03(무채색 본체)은 Workbench Resources에 물리 clip으로 표시될 뿐 arena boss body가 아니므로 Pattern actor/PRODUCT로는 쓰지 않는다.

## 2026-09-05 Codex 재검토와 수정

### 실제 저장 상태와 검증 범위

현재 composition revision은 28이며 Server Product는 `KAKULSAYDON_G1_PIZZA` 1개다.
사용자가 만든 `세이튼_무력화 시작`(7 Stage, Logic 1), `세이튼_진짜세이튼찾기`(13 Stage, Logic 1, Summon 1),
`세이튼_댄스타임`(21 Stage)은 모두 DRAFT다. 무력화성공·가짜세이튼 보조 패턴도 DRAFT다.
이번 검토는 이 authoring 상태나 사용자 JSON을 변경·publish하지 않았다. Boss Tool을 열어 실제 세이튼 패턴을
재생한 검증은 수행하지 않았다. Server 재생 회귀는 현재 Product인 피자로 검사했다.
`Start Full Pattern`은 기존 코드에서 `Play_All()`을 호출하며 새 보스별 메뉴를 추가한 것이 아니다.

### 수정한 실행·저장 경로

- Server: 시작 쿠크의 별도 update가 다른 관문 보스 audition을 중단하던 분기를 family 경로로 통합했다.
  Play All은 기존 저작 순서에서 대상 body에 맞는 패턴만 고르며, 재생 중 despawn은 실제 Stage를 ABORTED에 남긴다.
- Publisher와 Server: RAW clip만 있는 Kouku Product의 빈 sourceActionIds 및 참조 action 0을 허용한다.
  발탄의 기존 조건과 중복 source 행 거부는 유지한다.
- Client audition: Level 변경 시 이전 대상·request scope를 초기화하고 관문 전환 중 새 Play 요청을 거절한다.
- 관문: 모든 spawn 응답과 teleport 성공을 받은 뒤 active 표시·HUD·재생 대상을 확정한다.
  실패한 관문은 다시 선택할 수 있으며 대기 중 Despawn도 막는다. 1마리오는 보스 HUD를 숨긴다.
- 튜닝: 반복 Save의 배율·offset 누적을 제거했다. Reload는 이 Client가 로드한 prototype 배율을 기준으로 한다.
  외부 파일 변경은 거부하고 두 파일을 먼저 stage한다. 교체 실패 시 복원하고, 복원이 실패한 backup 경로는 보존해 안내한다.
  ReplaceFile 오류 1177의 원본 이동 후 실패도 복구한다. 디버그 body 배율·offset은 Server collider mirror에 적용하지 않는다.
- Stage 재정렬은 기존 구현을 검토했으며 추가 변경하지 않았다. animation의 Stage 상대시간과 Logic/Summon의 Pattern 절대시간을 보존한다.

### 재검증 증거

- Server contract: `out/KoukuArenaReview/followup-server-contract.log`, failures 0.
  새 G2 피자 재생 완료, 혼합 body sequence 선택, 활성 occurrence despawn, action 0 및 중복 거부 검사를 포함한다.
- Python: `out/KoukuArenaReview/followup-python.log`, 55 tests OK.
- 튜닝 저장: 실제 저장 helper를 임시 native check로 실행했다. 두 파일 저장, 동일 bytes 재저장, 외부 수정 보존,
  두 번째 파일 잠금 시 첫 파일 rollback, 임시 파일 정리, 오류 1177 후 복원 모두 통과했다.
  로그: `out/KoukuArenaReview/followup-tuning-io.log`. Client 화면 조작 테스트는 아니다.
- 변경 JSON 8개 parse, git diff --check 통과. 최종 Debug Product 결과는 `out/KoukuArenaReview/followup-final-product.log` 참조.
- 세이튼 DRAFT 3종의 Server gameplay 및 Boss Tool 화면 재생은 미검증이다. Logic/Summon Server consumer는 기존 보류 범위다.

관문 Server 명령 전체가 하나의 원자적 작업으로 합쳐진 것은 아니다. spawn 성공 후 navigation에서 teleport가 거부되면
오류를 표시하고 재시도할 수 있지만 이전 보스 그룹을 자동 복원하지는 않는다. 사용자 요청에 따라 최종 컴파일 확인 뒤 종료한다.

### 최종 빌드 상태와 종료 경계

재검토 첫 Product 빌드(`followup-product.log`)는 통과했다. 이후 Client 전환 guard와 파일 복원 보완까지 포함한
마지막 Product 빌드(`followup-final-product.log`)는 Client의 `cl error D8040`(자식 프로세스 생성·통신 실패) 및 `C1083`(.obj Permission denied)로 종료됐다.
소스 문법 오류라고 단정하지 않지만 최종 변경 전체의 빌드 PASS도 기록하지 않는다.
사용자가 별도로 빌드를 진행 중이라고 알려 추가 빌드를 시작하지 않았다. 사용자 빌드 결과가 최종 확인으로 남는다.
Server contract failures 0, Python 55 tests OK, 튜닝 파일 저장 검사는 위 기록대로 유효하다.
커밋·push·PR·merge는 하지 않았다.
