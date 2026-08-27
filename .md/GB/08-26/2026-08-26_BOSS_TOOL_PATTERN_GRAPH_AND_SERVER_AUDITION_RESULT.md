# Boss Tool 패턴 연결 그래프·서버 반복 검증 결과

## 0. 결론

구현과 자동 검증은 완료했다. 사용자 육안 검증은 아직 수행하지 않았으므로 visual PASS는 아니다.

Debug Client의 `F1 -> Boss Tool`이 Valtan Server pattern 반복 검증, revive, live graph의 주 화면이다.
Effect Tool은 같은 공용 service에 stable pattern ID를 one-shot 제출하는 `Play Server` shortcut만 제공한다.
선택 Pattern은 기존 stable-ID Server audition으로만 실행되며 Client local animation/effect/spawn 경로를
만들지 않았다. 완료 뒤 automatic 25-step sequence로 새지 않고 IDLE HOLD하며, `Repeat`는 같은 stable ID를
Server reset 뒤 다시 제출한다.

첫 화면은 Winters Engine ImGui 규칙에 따라 다음 정보와 행동만 기본 노출한다.

```text
Live Server Pattern / Stage / time / phase / HP / freshness
Play Selected / Repeat / Stop After Current / 죽었을 때만 Revive Player
선택 Stage의 Animation / Effect / Camera / Hit·Motion / World / Next
```

raw occurrence, Effect Element/resource DDS, camera keyframe, Server 수치는 하나의 접힌
`Why / Advanced diagnostics`에만 둔다.

## 1. 구현 상태

### 1.1 Boss Tool

- `CMainApp::DEBUG_TOOL::BOSS` lazy-create와 F1 hub 버튼을 추가했다.
- `CValtanPatternTree` strict joined view를 그대로 사용한다.
- All Effects와 같은 공용 selector inventory인 Core 6 + animator manual 20만 같은 순서로 표시한다.
- 전체 graph의 나머지 Pattern은 live/owner 진단에 유지하되 선택·Play·Repeat·Follow Live 승격을 막는다.
- 초기 Pattern을 임의 선택하지 않고 empty state를 표시한다.
- Pattern을 직접 선택해 재생하면 live-follow를 켜 실제 Server actionId에 exact join한 Stage를 따라간다.
- Effect summary는 boss-root cue, pattern Effect binding, independent Effect, combat-object visual을 합친다.
- camera invocation/cue, pattern/stage motion, hit/damage, world action, branch/fallthrough를 표시한다.
- Effect resource owner 검색은 사용자가 `Find Owner`를 누를 때만 authored Effect 문서를 읽고 load 실패
  문서 수를 별도로 보고한다.
- current authored Effect와 `CEffectCatalog::Find_Loaded()` next-spawn catalog document를 canonical 비교한다.
  동일한 owner row는 `NEXT-SPAWN MATCHED - replay required`, catalog 미로드·불일치 row는
  `LOCAL UNVERIFIED`로 표시한다. active occurrence는 이전 shared document를 유지할 수 있으므로 검증됐다고
  표시하지 않는다.
- source timestamp 또는 catalog shared document가 바뀌면 owner 검색 generation이 달라져 기존 결과를
  `STALE`로 숨기고 `Find Owner` 재실행을 요구한다.
- gameplay gate/hit/motion/world/next는 current local authoring이며 pinned Server gameplay generation과의
  동일성이 아직 증명되지 않았음을 선택 화면에 명시한다.

### 1.2 단일 실행 서비스와 역할별 UI

- Arena legacy audition panel의 자동 render를 제거했다.
- Balance Tool의 Server Pattern replay와 revive UI/dependency를 제거했다.
- Boss Tool과 Effect Tool의 one-shot shortcut은 같은 `CValtanPatternAuditionService::Submit()`만 호출한다.
- Effect Tool은 Pattern과 독립 Effect owner의 stable ID만 제출하며 local Model View timeline을 초기화하지
  않는다.
- Repeat/Stop과 typed `Request_RevivePlayer()`는 Boss Tool만 소유한다.
- verdict/lifecycle queue는 공용 service 하나가 소비하므로 서로 다른 Tool이 결과를 빼앗지 않는다.

### 1.3 완료 HOLD와 freshness

- Server가 stable-ID audition의 자연 완료를 확인하면 automatic sequence override/hold를 유지한다.
- 다음 stable-ID request는 기존 full reset을 거쳐 HOLD를 풀고 새 occurrence를 시작한다.
- pinned revision `available`만으로 현재 workspace graph를 verified 처리하지 않는다.
- world-entry immutable core presentation artifact baseline과 current workspace source를 byte 비교한다.
- Graph reload 뒤 다르면 `workspace changed; restart/publish`와 `Connections are unverified`를 표시한다.

### 1.4 확인한 기존 회귀

- `VALTAN_HIGH_JUMP` target axe combat-object visual은
  `effect.valtan.sky-axe.active`가 정본이다. 잘못 연결됐던 boss TAKEOFF Effect를 원래 연결로 복구했다.
- `fx_e_decal_007_2.dds`는 도넛 owner가 아니며 floor-wipe second smash, swing/sweep,
  four-pillars target cone 계열 authored Effect에서 소유한다. Boss Tool diagnostics로 owner를 역추적할 수 있다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| Client x64 Debug C++ compile | PASS, 최신 exact inventory 변경 포함 |
| Client x64 Debug canonical link | 실행 중인 `Client.exe` 잠금 해제 후 재실행 필요 |
| Boss Tool contract | PASS, 12 tests |
| Valtan Pattern Tree contract | PASS, 18 tests |
| Balance exclusion + Boss/Effect shared-service contract | PASS, 25 tests |
| Valtan Pattern Master V2 | PASS, 46 tests |
| Client pre-build Valtan V2 publisher validation | PASS, managed 27 / world members 30 / projected artifacts 9 |
| Client project/filter XML parse | PASS |
| 전체 dirty worktree `git diff --check` + 신규 파일 trailing whitespace 검사 | PASS |
| Server isolated Debug build + focused contract | PASS |
| Winters ImGui 독립 재검토 | PASS, 잔존 P0/P1 없음 |
| Effect cache/owner lifecycle 독립 코드 재검토 | PASS, 잔존 P0/P1 없음 |

Server 전체 contract의 별도 실행은 이미 실행 중인 표준 Debug Server process가 mutex를 소유해 해당 기존
중복 실행 항목 1건만 실패했다. 실행 중인 Server를 종료하지 않았고, 추가한 stable-ID completion HOLD 계약은
독립 실행에서 통과했다.

## 3. 사용자 육안 검증 순서

기준은 사용자가 실행한 Debug Server + Debug Client의 Valtan Arena다.

1. `F1 -> Boss Tool`을 연다.
2. Effect Tool의 독립 도넛에서 `Play Server Owner`를 누른다.
   - Boss Tool 상단에는 `VALTAN_FIST_IN_OUT [live only; outside All Effects list]`가 보여야 한다.
   - Boss Tool 선택 목록에는 이 ID와 이전 legacy 행이 없어야 한다.
   - 독립 도넛 Effect 1회 뒤 다른 Pattern과 바닥 decal이 이어지면 실패다.
3. `VALTAN_HIGH_JUMP`를 선택한다.
   - `AIRBORNE` Effect에 `effect.valtan.sky-axe.active [combat object]`가 보여야 한다.
   - 추적 도끼와 `LAND` 원형 착지 Effect가 서로 다른 Stage로 보여야 한다.
4. `Why / Advanced diagnostics -> Find resource owner`에 `fx_e_decal_007_2.dds`를 넣는다.
   - Pattern / Stage / Effect / Element / slot owner가 나와야 한다.
   - `NEXT-SPAWN MATCHED - replay required`는 다음 spawn의 catalog가 current source와 같다는 뜻이다.
     이미 진행 중인 occurrence의 증거가 아니므로 그 표시를 본 뒤 owner Pattern을 새로 재생한다.
   - `LOCAL UNVERIFIED`이면 해당 owner Pattern을 먼저 재생해 catalog load를 유도하거나 Effect hot
     replacement/restart 뒤 다시 검색한다. `STALE`이면 `Find Owner`를 다시 누른다.
5. `VALTAN_ARENA_BREAK_109`를 선택한다.
   - `TAKEOFF -> DROP -> IMPACT -> IMPACT_HOLD -> WIDE_REVEAL -> RECOVERY`를 live-follow해야 한다.
   - `IMPACT`의 wall event와 camera cue, 이후 원래 camera 복귀를 눈으로 확인한다.
6. 전멸로 player가 죽으면 같은 창에 나타나는 `Revive Player`를 누른다.
   - Server HP snapshot이 복구된 뒤 Repeat가 같은 선택을 이어 가야 한다.
7. 3연속 돌진, 잡기/날리기, 바닥 파괴 Pattern도 선택해 반복한다.

상단 freshness가 `workspace changed; restart/publish`이면 현재 실행 중인 Client가 로드한 core
presentation과 화면의 current workspace graph가 다르므로 visual 판정을 중단하고 publish/restart해야 한다.
Hit/Motion/World/Next 행은 current local gameplay authoring이라는 경계를 유지하므로 이 행만으로 pinned
Server 수치 동일성을 확정하지 않는다.

## 4. 남은 경계

- 현재 완료 단위는 `Valtan Boss Tool — read-only connections + Server audition`이다.
- 연결 추가/교체/삭제는 여러 정본의 staged validation/rollback이 필요한 후속 transactional authoring이다.
  첫 화면에 동작하지 않는 Link/Unlink placeholder를 넣지 않았다.
- Camera 값은 읽을 수 있지만 전용 Camera authoring Tool/deep link는 후속 범위다.
- 사용자 관찰 전에는 layout, clipping, Effect/camera fidelity, occurrence visual을 PASS로 기록하지 않는다.
- PR/commit/push는 수행하지 않았다.
