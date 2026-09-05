# 밸런스 튜닝과 Hot Reload 계약

## 1. 일반 밸런스 기준선

Player, skill, damage, boss 기본 수치 정본은 `Data/Balance/*.json`이다. Visual Studio Client 프로젝트의
`96.DataFiles/Balance` 필터는 이 원본을 직접 보여 줄 뿐 복사본을 만들지 않는다. Server 생성물인
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 직접 편집하지 않는다.

```text
Data/Balance JSON
-> Publish-GameplayBalance.ps1 parse/validate/stage/commit
-> Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap
-> Server CGameplayCatalog
-> GameRoom 30 Hz 판정
-> S2C_WORLD_SNAPSHOT / damage event
-> CCombatHUDViewModel
```

Valtan 전용 candidate transaction에 포함되지 않는 Player/skill/item/world 일반 변경은 Publish 뒤 Server를
재시작한다. Client HUD나 GameObject만 JSON을 다시 읽어 Server 판정과 다른 값을 표시하지 않는다.

## 2. Valtan 저작 정본

발탄은 다음 물리 정본을 하나의 joined revision으로 다룬다.

| 정본 | 소유 의미 |
|---|---|
| `Data/Valtan/Valtan.gameplay.json` | Server pattern graph, selection, eligibility, stage, hit, motion, event, branch와 inline `scriptedSequence` |
| `Data/Valtan/Valtan.presentation.json` | animation occurrence, managed Effect invocation, camera invocation |

`Valtan.combatobjects.json`, `Valtan.worldeventsets.json`은 각 stable gameplay definition을 소유한다.
`Valtan.pattern.json`은 migration/legacy compatibility fixture일 뿐 새 UI 값이나 draft baseline이 아니다.
pipeline과 `CValtanPatternTree`는 gameplay/presentation의 `patternId/stageId/actionId` closure와 순서를 strict join하고,
한 파일 누락·중복·identity drift에는 기존 Tool/runtime 상태를 유지한다.

일반 패턴 선택 수치는 전역 pattern weight가 아니라 `decisionModel.selectionSets[].candidates[]`가 소유한다.
각 후보는 `weight: 1..100000`과 별도 `enabled`를 가지며, 비활성화해도 마지막 양수 weight를 보존한다.
`compatibilitySelectionWeight`는 아직 managed window가 소유하지 않는 legacy/global fallback 전용 값이므로
selection-set weight 편집으로 함께 바꾸지 않는다. projector는 이 계약을
`ValtanPatternRotations.json` formatVersion 3의 `WINDOW + CANDIDATE` 행으로 내보내고, Server gameplay bootstrap
formatVersion 21이 후보 ordinal 그대로 소비한다. post-109 legacy rotation 여섯 개는 `STEP` 순서와 의도적 중복을
보존하는 read-only Product다.

## 3. 현재 활성화한 Hot Reload 범위

Debug F1 Balance Tool의 Valtan candidate와 Valtan Boss Tool의 canonical sequence Save/Restart는 같은
`CValtanTuningCommandService`와 Server-authoritative Hot Reload 경로를 사용한다. 허용 범위는
**모든 required Client presentation artifact가 해당 Client가 world entry 때 고정한 immutable presentation
baseline과 byte-identical인 gameplay-only diff**다. PREPARE 중 repository disk를 다시 읽어 이미 로드된
presentation 세대인 것처럼 승인하지 않는다.

```text
Validate Draft
-> Save Authoring immutable revision
-> Publish Candidate immutable artifact set
-> Server stage + allowed Valtan diff 검증
-> 영향 Client presentation READY/NACK
-> 모든 shared/private room의 같은 tick boundary에서 global commit 또는 global ABORT
```

- Server는 candidate bootstrap을 별도 immutable `CGameplayCatalog` generation으로 stage한다.
- Publish가 계산한 candidate 전체의 가장 강한 `applyClass`가 `HOT_RELOAD`일 때만 Tool이 PREPARE를 보낸다.
  `ENCOUNTER_RESET`과 `SERVER_RESTART`는 immutable candidate로 남지만 현재 Apply는 요청 경계와 UI에서 모두 거부한다.
- active pattern, pending health-bar mechanic, combat object와 audition은 시작 revision을 끝까지 pin한다.
  Product ordered sequence는 inter-step 및 마지막 idle까지 순서 전체의 revision을 유지한다.
  저장 Flow 적용 후 새 encounter/reset은 새 순서를 사용한다. 중간 cursor에 새 배열을 끼워 넣지 않는다.
- snapshot/world entry는 active와 required pinned revision set을 함께 전달한다.
- Client는 revision identity와 world-entry baseline의 required lane bytes를 검증한 뒤에만 READY한다.
- NACK, timeout, disconnect, stale base, hash/domain mismatch는 모든 room에서 old revision을 유지한다.
- Server decision trace는 후보별 exclusion, authored/effective weight, RNG interval과 선택 revision을 보여 준다.
- 성공 commit은 candidate publish pointer와 별개의 Server runtime-active pointer/journal에 먼저 durable promotion된다.
  다음 Server 시작은 그 pointer가 가리키는 exact immutable candidate를 재검증해 다시 admit하며, missing/corrupt
  candidate를 packaged baseline으로 조용히 대체하지 않는다.

현재 다음 diff는 성공한 Hot Reload로 취급하지 않는다.

| diff | 현재 처리 |
|---|---|
| maximum HP/health bars/AP/collision/engage/move | `ENCOUNTER_RESET required`; live Apply 거부 |
| animation occurrence/Effect/camera invocation 또는 Client combat visual bytes 변경 | byte-identical presentation admission 거부 |
| world-event set membership/placement/mutation/navigation 변경 | G09 이전 candidate 거부 |
| packet/schema/new C++ event/new asset registration | Server/Client restart가 필요한 별도 변경 |
| 109 phase boundary health bar 단독 변경 | 현재 reject; mechanic/window/legacy span을 함께 바꾸는 원자 `SET_PHASE_BOUNDARY` 전까지 read-only |

multi-room controlled encounter reset과 실제 non-byte-identical presentation generation registry가 구현되기 전에는
위 경계를 파일 watcher, Client-only reload, audition reset 호출로 우회하지 않는다.

## 4. transaction 불변식

1. candidate revision은 canonical content hash이며 immutable directory와 hashed parent manifest를 가진다.
2. Save/Validate/Publish는 authoring head CAS를 사용한다. stale editor는 새 head를 덮지 못한다.
3. parse/validate/stage와 모든 Client READY가 성공하기 전에 Server runtime-active pointer를 바꾸지 않는다.
4. commit cohort에는 shared room과 session별 private room을 모두 포함한다.
5. room 등록과 Client join/disconnect는 같은 transaction admission gate를 지난다.
6. commit은 allocation/I/O가 끝난 뒤 global room tick boundary에서만 수행한다.
7. 일반 진행 중 occurrence는 old generation, 다음 선택은 new generation을 사용한다. Product ordered sequence는
   실행 전체를 pin하며 다음 encounter/reset부터 새 generation을 사용한다.
8. 실패는 staged generation/presentation만 버리고 old Server/Client state를 보존한다.
9. Release Client는 현재 Debug Valtan presentation alias transaction을 READY하지 않는다.
10. 화면에서 보이는 revision과 Server snapshot revision이 다르면 성공으로 표시하지 않는다.
11. phase-changing arena mechanic bar는 마지막 phase-1 managed window의 끝 및 첫 post-phase legacy span의
    시작과 정확히 같은 경계여야 한다. authoring pipeline, Product publisher, Server admission이 각각 독립 검증한다.

## 5. 담당자 절차

일반 gameplay balance는 다음을 사용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

발탄 joined source/candidate는 다음으로 자동 검증한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Test-ValtanTuningRuntimeSet.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
Server/Bin/Debug/Server.exe --contract-test
```

Server가 중지된 상태에서만 Debug packaged baseline으로 runtime-active pointer를 복구할 수 있다.

```powershell
Server/Bin/Debug/Server.exe --reset-valtan-runtime-to-packaged
```

이 명령은 verified packaged bootstrap을 새 pointer로 원자 기록한다. 없어진 예전 candidate에서 빠져나오는 운영
escape hatch일 뿐, 손상되거나 모호한 pointer/journal을 추측해 덮지 않는다. Release Server는 이 명령과 Hot Reload를
명시적으로 거부한다. canonical Product는 rotation v3/bootstrap v21이며, v18은 offline migration fixture일 뿐
v21 Server의 live admission 대상이 아니다.

실제 사용은 Server와 Debug Client를 사용자가 직접 시작한 뒤
`Valtan Arena -> F1 -> Balance Tool -> Bosses -> Valtan`에서
`Validate Draft -> Save Authoring -> Publish Candidate` 뒤 `Apply class: HOT_RELOAD`를 확인하고
`Apply Hot Reload -> Play Server Pattern` 순서다. reset/restart class는 Apply 성공으로 기록하지 않는다.
자동 검증과 animation/Effect의 사용자 육안 판정은 RESULT에서 분리한다.

순서 편집은 `F1 -> Valtan Boss Tool -> Pattern Flow -> Save Flow`를 사용한다. slot 편집은
`Valtan.gameplay.json > decisionModel.scriptedSequence` typed draft로 저장되며 별도 Flow 파일을 만들지 않는다.
물리 SAVED, candidate PUBLISHED, Server COMMITTED/ALREADY ACTIVE를 구분한다. 저장 뒤 candidate apply가 실패해도
물리 commit 성공을 되돌려 표시하지 않는다. Restart는 최신 saved candidate와 Server-active definition revision이
exact-equal일 때만 그 sequence를 시작하며, 잘못된 slot/pattern ID를 이전 배열로 대체하지 않는다.
