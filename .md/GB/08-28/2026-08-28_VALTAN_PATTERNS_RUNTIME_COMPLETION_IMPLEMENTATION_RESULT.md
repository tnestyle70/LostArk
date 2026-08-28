# 발탄 패턴 런타임 완성 구현 결과

## G00. 범위와 정본

현재 작업 폴더의 기존 미커밋 변경을 보존한 채 `Data -> publisher -> Shared -> Server ->
Client presentation -> native harness` 수직 경로를 확장했다. 별도 보스 런타임이나 고정 pattern
allowlist는 만들지 않았다. 플레이어 외형 분신은 이번 요청의 구체 동작 범위에 포함하지 않았다.

저장 순서 정본은
`Data/Encounters/Valtan/ValtanBossAuditionFlows.json`의
`flow.valtan.boss-tool.default`다. 현재 22슬롯과 반복 ID, sparse slot ID,
`nextSlotOrdinal=31`을 그대로 보존했다. 확인한 raw SHA-256은
`3a0e831a8454ef09ae05c1023b251d607cce60d0702e3cb732fb7e9b952b79d5`다.
HIGH_JUMP AIRBORNE 6500ms와 Six Pizza 첫 착지 267ms도 유지했다.

## G01. 손 부착과 날리기

Client는 잡힌 순간의 손 상대 거리를 저장하지 않고, 왼손 본의 정규화된 basis에 고정된 손바닥
translation을 합성한다. 실제 본 scale 0.01에서도 유효하며 non-finite·비 affine·퇴화 basis는
이전 attachment를 보존하고 거부한다. Server 포획 동안 기존 입력 gate가 이동·스킬 제출을 막는다.

`VALTAN_CATCH_BREATH`의 release는 `ARENA_EJECTION`, 24m/s, 500ms다. 기존 12m/s 기준을
두 배로 올렸고 최소 이동도 6m에서 12m로 보정한다. nav 밖에서도 유한 시간 뒤 명시적으로
`FALLING`에 들어가며 일반 knockback은 바꾸지 않았다.

## G02. 버러지 네 패턴

`VALTAN_TRASH`, `VALTAN_CATCH_BREATH`, `VALTAN_CHARGE_2`, `VALTAN_STRUGGLING`을
strict joined 패턴으로 유지한다. Trash는 STEP_07의 전방 counter proxy와 counterable flag를
사용하며 성공 시 4433ms GROGGY 뒤 종료한다. Rush는 0~600ms 동안 100ms 간격으로 포획을
계속 평가하고, 정면 navigation이 막히면 그 tick의 마지막 포획 판정을 끝낸 뒤 RUSH_MISS로
종료한다. 포획·미스·전원 포획·카운터 분기는 모두 terminal에 도달하는 유한 graph다.

## G03. 포탈, 중앙 이동과 피자 방향

`VALTAN_WARP`는 8개 leg마다 생존 플레이어를 다시 선택한다. 같은 플레이어의 연속 선택을
허용하며 현재 위치에서 대상을 바라본 뒤 기존 900ms·최종 7.2654m root curve를 Server transform으로
적용한다. 이 이동만 navigation projection과 wall/body contact를 우회한다. 0~850ms를 50ms 간격으로
평가하되 이전 pulse와 현재 pulse 사이를 swept box로 판정하고, leg마다 플레이어 한 명당 한 번만
피해를 준다. 마지막 STEP_10에서 같은 높이의 walkable center로 돌아온 뒤 다음 패턴을 허용한다.

Six Pizza와 3시·9시 지형 파괴는 takeoff 전에 `[156.03, 22.99751, -122.06]` 중앙으로
이동한다. 3시·9시는 `arena.center` snapshot anchor를 사용해 사용자가 저장한 상대 회전과 파괴
placement를 유지한다. Six Pizza는 시작 때 생존 플레이어 최대 4명 중 한 명을 Server에서 잠그고
중앙→대상 yaw를 `arena.center.facing`으로 고정한다. sector 05·04와 착지 facing이 같은 yaw를
소비한다.

## G04. 독립 도넛

`VALTAN_FIST_IN_OUT` foreground 단계는 100ms 뒤 끝나 다음 패턴을 진행한다. 동시에
`combatobject.valtan.fist-in-out.donut`을 정확히 하나 생성한다. 객체는 생성 위치에 2600ms
고정되고 1600ms에 반경 8~16m ring 피해를 한 번 적용한다. 다섯 Effect element는
`localSpace=false`이며 생성 시점의 position·orientation을 고정하지만 element 내부 scale lerp는
계속 진행한다.

## G05. 유령 finale

`VALTAN_GHOST_FINALE`는 `DERIVED_SERVER_PATTERN`이며 저장 Flow를 자동 수정하지 않는다.
이 패턴의 본체 portal만 arena corner 0→1→2→3을 두 번 순회하며 snapshot anchor를 사용한다.
같은 deck의 중앙 반폭
10m, nav와 body clearance를 만족하는 위치에 owner가 지정된 `BOSS_VALTAN_GHOST`를 최대 한 개
spawn한다. 유령은 `VALTAN_WHIRLWIND -> VALTAN_FOUR_SLASH -> VALTAN_SEQUENCE_FOUR`를
순서대로 한 번 실행하고 제거된다.

애니메이션 인수 패턴 `VALTAN_GHOST_RESPAWN_AUDITION`과
`VALTAN_GHOST_DEATH_AUDITION`도 strict manual wrapper로 승격했다. 각각 `mesh_respawn_1`,
`mesh_dead_1` 한 stage를 재생하며 자체 Effect cue는 0개다. 공통 dynamic inventory를 통해 Boss Tool과
All Effects에 노출하지만 finale의 세 공격과 cleanup 순서에는 자동 삽입하지 않는다.

유령은 본체 HUD·BGM·Boss Tool target·player damage target·몸 충돌 blocker가 아니다. spawn wire의
owner는 살아 있는 같은 encounter의 primary이며 무소유·자기 소유·종속 owner·다른 encounter·다른
revision을 거부한다. 초기 복제는 primary를 먼저 보내고 고아·중복 dependent frame을 rollback한다.
본체 HP 0, room reset, Next, Stop은 child와 그 combat object를 함께 정리한다.

## G06. HP 0 사망 표현

protocol v44의 `S2C_WORLD_ENTITY_DESPAWNED`는 `REMOVED`와 `DEAD` reason을 구분한다. Server는
HP 0에서 BOSS authority와 dependent entity·독립 피해를 제거하고 `DEAD`를 reliable broadcast한다.
Client는 HUD와 registry를 즉시 끊은 뒤 해당 actor의 실제 `presentationClips.dead`를 non-loop로
재생한다. 본체와 유령의 현재 clip은 `mesh_dead_1`이며 실제 모델 길이는 110 ticks / 30Hz다.
clip 완료 뒤 Layer에서 제거하고, clip 누락·duration 오류·재생 실패는 즉시 제거한다. DEAD snapshot과
despawn 중복은 재생을 다시 시작하지 않으며 reset·퇴장도 남은 corpse를 정리한다.

## G07. Boss Tool·Flow 확장 통합

`Fix next pattern selection flow`의 G15 계약을 현재 구현 기준으로 반영했다.
`Build_PlayablePatternInventory`는 strict joined `bAuthoringMasterManaged` 전체 정의에서 재생 집합을
동적으로 만든다. Boss Tool, Play/Repeat, Next, All Effects, 저장 Flow와 publisher가 같은 집합을
사용한다. Core/Animator/Derived는 표시 분류이며 Core 8+Animator 20, 28개, 29개 또는 현재 33개를
승인 조건으로 사용하지 않는다. FIST_IN_OUT, ENTRANCE_CINEMATIC, GHOST_FINALE도 포함한다.

등록 pattern 수와 한 Flow의 wire 1~255슬롯 U8 용량은 서로 다른 계약이며 256슬롯은 거부한다.
stable ID·stage/action identity,
manual owner·source chain, 중복과 손상 데이터 rollback은 유지한다. Reload는 slot ID 숫자를 찾지 않고
저장 배열 첫 원소에서 시작하며, Next는 현재 occurrence 종료 뒤 reset 없이 다음 패턴을 재생하고 저장
배열은 바꾸지 않는다. 같은 슬롯의 새 occurrence와 wrap·terminal도 stable ID 하드코딩 없이 sequence로
상관한다.

## G08. 자동 검증

이 문서는 실행한 결과만 PASS로 갱신한다. 현재까지 확인한 항목은 다음과 같다.

| 검사 | 결과 |
|---|---|
| Valtan promotion/publisher validation | PASS, promoted 22 / managed 33 / Product 57 / stages 255 |
| Pattern Master V2 | PASS, 72 tests |
| Promotion / Boss Tool / Pattern Flow / PatternTree / root motion | PASS, 16 + 21 + 27 + 26 + 4 tests |
| Camera contract | PASS |
| 3시·9시 world transition contract | PASS, 8 tests |
| 관련 JSON parse / 저장 Flow SHA 보존 | PASS |
| `git diff --check` | PASS, whitespace error 0; EOL 경고만 존재 |
| Debug Engine/Shared/Server/Client 및 native harness build | PASS |
| NetworkProtocolHarness | PASS, failures 0 |
| Valtan audition / Flow / tuning harness | PASS, 23/23 + 11/11 + 13/13 |
| EffectRenderContractHarness Debug | PASS, native exit 0 |
| Server Debug contract | FAIL, 현재 1건: lethal floor-wipe 사망 cursor hold → revive → terminal 전이 |
| Debug 정본 전체 회귀 | 사용자 마감 요청으로 중복 장시간 validator 구간에서 중단; 전체 PASS 아님 |
| Release 정본 전체 회귀 | 현재 합본으로 미실행; PASS 아님 |

Client/UI는 실행·조작·캡처하지 않았다. 실제 이펙트 회전·스케일·손 위치·사망 clip의 화면 품질은
사용자가 Server + Client profile에서 확인해야 하며 자동 PASS로 기록하지 않는다.

## G09. 남은 P1 코드 경계

1. 사망 중 dash cursor hold와 revive 이후 즉시 재개, terminal IDLE을 검증하는 Server contract 1건을
   실제 cursor/완료 로직에서 고쳐야 한다.
2. Reload Flow는 저장 revision의 Product가 Server active revision으로 확정된 뒤에만 시작해야 한다.
3. FlowDocument Save의 authoring-head CAS는 모든 writer가 공유하는 lock 안에서 precheck와 replace를
   함께 수행해야 한다.
4. Saved Flow publisher의 마지막 CAS 실패 시 이미 교체한 Product 문서 7개도 rollback해야 한다.

위 네 항목을 닫고 Debug/Release Server contract와 Client build를 다시 통과하기 전에는 전체 완료로
간주하지 않는다.

## G10. 남은 사용자 확인

1. Lobby → Valtan → F1 → Boss Tool에서 저장 Flow가 화면 01부터 시작하는지 확인한다.
2. `VALTAN_TRASH`, `CATCH_BREATH`, `WARP`, `SIX_PIZZA_106`, 3시·9시,
   `FIST_IN_OUT`, `GHOST_FINALE`를 각각 Play/Next로 확인한다.
3. sector 05·04와 발탄 착지가 같은 랜덤 대상 방향인지, 중앙 이펙트가 이동 뒤에도 고정되는지 본다.
4. HP 0에서 `mesh_dead_1`이 한 번 재생되고 약 3.667초 뒤 사라지는지 확인한다.

대규모 공유 dirty worktree이므로 자동 stage·commit·push는 하지 않았다.
