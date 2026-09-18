# 2026-09-18 쿠크 선택 지면 장판 연결과 추적 이동 RESULT

브랜치 `codex/shader-build-isolation-20260917`, 시작 HEAD `a198c9406`.
세션 시작 시 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행했고 결과는
`server-host` / bind `0.0.0.0:7777` / endpoint `192.168.0.14:7777` / `reachable`이다.

작업 중 사용자가 Composition을 계속 저장했다. 이 문서의 수치는 실제 디스크 저장본과
실행한 명령의 출력만 기록한다. Client/UI 실행과 화면 판정은 하지 않았다.

## G01. 노란장판펑펑의 칼날 원형을 플레이어 위치에 생성

### 실제로 막혀 있던 지점

`세이튼_공먹고휘리릭_노란장판펑펑`(`KAKULSAYDON_G1_PATTERN_79`)의 네 MAP 행
`presentation.5/6/7/8`(칼날댄스 원형 예고·폭발, 도넛1·2 폭발)은
`selectionGroupId = KAKULSAYDON_G1_PATTERN_79.effectgroup.11`과 절대 좌표
`[-2.2561512, 1.2990054, 731.51434]`를 갖고 있었다.

플레이어 위치 주입 기구 자체는 Server·Client·projector에 이미 전부 있었다.
빠져 있던 것은 데이터 연결 두 곳뿐이다.

| 항목 | 이전 | 이번 |
|---|---|---|
| `kakulsaydon.g1.logic.80.selectedEffectGroupId` | `...effectgroup.9` (문서 어디에도 없는 ID) | `...effectgroup.11` |
| 패턴 79의 SELECT 배치 | 없음 (logic.80은 어떤 패턴에도 놓이지 않음) | `KAKULSAYDON_G1_PATTERN_79.logic.5` / 8883ms / 4083ms / enabled |

`nextLogicOccurrenceOrdinal` 5→6, `revision`을 하나 올려 저장했다. 편집 중이던 Tool이
기존 3-way 병합(`Merge_CompositionDraft`)으로 이 변경을 보존한 것을 이후 저장본
revision 1611에서 확인했다.

### 앵커를 BOSS로 바꾸지 않은 이유

`_showtime_visual_template(..., selection_start_ms=box["startMs"])`가 그룹의 첫 행 좌표를
공통 기준으로 빼고, SELECT 경로에서는 Y까지 뺀다. 네 행의 좌표가 같으므로 상대 위치는
모두 `[0,0,0]`이 되고 Server는 캡처한 navigation ground를 pivot으로 쓴다. 즉 저장된 절대
좌표는 Product로 새어 나가지 않고, 높이는 지면 기준이 된다. BOSS 앵커로 바꾸면 각 행의
서로 다른 `startMs`에서 공중에 뜬 보스 Y를 샘플하므로 채택하지 않았다.

### 실행한 검증

저장본 revision 1612에서 `prepare_publication` + `project_encounter`를 실행했다.

```
KAKULSAYDON_G1_PATTERN_79.logic.5  ALBION_AIRBORNE  start 8883  dur 4083
  airbornePhase=SELECT_PLAYER  airborneTargetPositionPolicy=SELECT
  selectedEffectVisualId=kouku.showtime.fixed.7c317544afe9aa04938e9aa6094fa3a40ca38a43ad78a87f22e68d64a4f92e6a
  selectedEffectLifetimeMs=4083
blade-circle rows still in the plain MAP lane: none (owned by the SELECT template)
```

template의 네 occurrence는 상대 시각 24 / 1779 / 2540 / 3095ms, 상대 좌표 전부 `[0,0,0]`,
anchorKind `MAP`이다. 원래 MAP lane에서는 사라져 중복 재생되지 않는다.

## G02. 공굴리기 카운터의 대상 지정·회전·이동

### 실측한 현재 상태

`세이튼_공굴리기_카운터`(`KAKULSAYDON_G1_PATTERN_81`)의 STAGE_2~STAGE_9는
`rpct00_att_battle_26_02`를 533ms씩 8회 반복한다. 이 구간의 Server 루트 모션을 실제로
구워 측정한 결과 수평 이동이 없다.

| 구간 | forward | lateral | up |
|---|---|---|---|
| STAGE_2~9 각각 | -9.4e-12 | 0.000 | -0.065 ~ +0.034 |

따라서 지금까지 보스는 이 구간에서 회전도 이동도 하지 않았고, 새 이동이 기존 루트 모션과
속도를 이중으로 더하지 않는다.

기존 `BOSS_TRACK_TARGET`은 대상 선정과 yaw 보간만 소유하고
`if (rotateOnly) continue;`로 위치를 건드리지 않았다. 새 13번째 trigger kind를 만들지 않고
이 judgement에 선택 이동 값을 붙였다.

### 계층별 변경

| 파일 | 변경 |
|---|---|
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | `followSpeedScale`을 SHOWTIME 전용에서 분리해 `BOSS_TRACK_TARGET`도 소유. 0은 회전 전용, 그 외 .01~10 검사. serializer는 0이 아닐 때만 기록해 기존 저장본을 그대로 둔다 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | 해당 judgement에 `Follow speed x player` 입력과 설명 두 줄 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 허용 키 집합에 추가, 정의 검증 분기, mechanic trigger 투영 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | optional 속성 수용·범위 검사와 `PATTERNTRACKMOVE` 행 |
| `Server/Private/GameplayCatalog.cpp` | `PATTERNTRACKMOVE` 파서. 이미 적재된 `BOSS_TRACK_TARGET` trigger를 stable ID로 찾고 중복·charge 겹침·절대 bossMotion을 거절 |
| `Server/Private/KoukuSaydonBrain.cpp` | 해당 judgement의 "다른 mechanic 값 금지" 목록에서 follow 값만 허용 범위로 교체 |
| `Server/Private/GameRoom_BossSimulation.cpp` | `Update_KoukuPlayerTargets`의 rotate-only 분기에서 실제 이동 |
| `Client/Public/KoukuSaydonPresentationPlayer.h` / `.cpp` | 미리보기가 같은 tick 계산을 재현 |

Server 한 tick은 회전을 먼저 쓰고 그 yaw를 따라
`Resolve_PlayerMoveSpeed(대상) * scale / 30` 만큼 전진한다. 기존 돌진과 같은 순서로
`Resolve_TraversalStep` → `Resolve_CircleMove` → `Resolve_TraversalStep`을 통과해야 하며
실패하면 그 tick의 pose를 그대로 둔다. stage 루트 모션이 captured origin에서 절대 위치를
다시 계산하므로 이동량만큼 `fPatternStageOrigin*`와 `fPatternStageRootGroundY`를 함께 옮겨
다음 sample이 이 전진을 되돌리지 않게 했다. 마지막에 `Update_BlockingBody`로 충돌 body를
따라 옮긴다.

첫 구현은 돌진처럼 traversal이 돌려준 지면 Y를 그대로 썼는데, 검토에서 두 가지 오류를
찾아 고쳤다. 그 방식은 해당 tick의 클립 상하 진동을 지면으로 눌러 버리고,
`originY - rootGroundY` 불변식을 깨뜨려 다음 sample의 높이를 틀리게 만든다. 지금은 이동 전
`지면 위 높이`를 먼저 재고 이동 후 새 지면에 그 높이를 더한다. 그러면 origin과 ground base가
같은 양만큼 움직여 불변식이 유지된다. 평지에서는 수직 변화가 정확히 0이다.

미리보기는 tick마다 대상 위치와 함께 `PLAYER_SNAPSHOT.fMoveSpeed`를 고정해 두고 같은 식으로
XZ offset을 누적한다. seek해도 같은 입력을 재생한다. 높이·navigation·충돌은 Server 권위이며
미리보기는 저작 높이를 유지한다.

### 저작한 데이터

| 항목 | 값 |
|---|---|
| `kakulsaydon.g1.logic.84` | `세이튼_공굴리기_추적이동`, DURATION / `BOSS_TRACK_TARGET`, `followSpeedScale` 1 |
| `KAKULSAYDON_G1_PATTERN_81.logic.2` | 3000ms ~ 7264ms (STAGE_2~9 전체), enabled |

`followSpeedScale` 1은 대상 플레이어 자신의 이동 속도와 같은 속도다. 불뿜기
`presentation.1`은 BOSS 앵커·`followBoss`·bone 없음이므로 보스 root 행렬을 그대로 상속한다.
`Effect_PivotSampler`가 `strAnchorKind == "BOSS"`이고 bone이 비면 root 4x4를 pivot으로 쓰므로
회전이 함께 적용된다. 즉 몸이 도는 방향으로 불뿜기도 돈다.

## 실행한 검증과 하지 않은 것

실행한 것.

- Server `ClCompile` (Debug x64): `GameRoom_BossSimulation.cpp`, `GameplayCatalog.cpp`,
  `KoukuSaydonBrain.cpp` 재컴파일, 오류 0.
- Client `ClCompile` (Debug x64): 변경 TU와 헤더 의존 TU 재컴파일, 오류 0, exit 0.
  `EngineSDK\Inc\Level.h`의 C4828은 이번 변경과 무관한 기존 경고다.
- `project_kouku_saydon_composition.py` import와 `LOGIC_KIND_VALUE_KEYS` 확인.
- 저장본 revision 1612의 `prepare_publication` + `project_encounter`: 90개 저장 패턴 중
  75개가 게시 가능, 패턴 79의 SELECT trigger와 template이 위 값으로 투영.
- `PATTERNTRACKMOVE` 투영 경로: 메모리 사본에서 패턴 81의 미완성 카운터 행만 비활성화해
  `BOSS_TRACK_TARGET / start 3000 / dur 4264 / followSpeedScale 1.0` 투영을 확인.
  저장 문서는 바꾸지 않았다.
- PowerShell 구문 검사와 `git diff --check`.
- `Publish-GameplayBalance.ps1 -Mode Validate`: exit 1. 사유는
  `KoukuSaydon composition validate failed: projected Product is stale: Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json`이며
  이번 변경과 무관하다. 게시된 Product가 저장본보다 오래됐기 때문이고, 사용자가 계속 저장 중이라
  이 상태는 `Publish All Patterns` 전까지 유지된다.

하지 않은 것.

- 링크와 제품 전체 빌드. Client(PID 38260)와 Server(PID 43668)가 실행 중이라 EXE/DLL 출력물이
  점유돼 있다. 컴파일만 검증했다.
- domain publish. 사용자가 계속 저장 중이고 `Request_SelectedServerPlay`는 게시 revision과
  저장 revision이 정확히 같아야 하므로, 최종 저장 뒤 사용자의 `Publish All Patterns`가 맞다.
- Client 실행·조작·화면 캡처와 visual 판정.

## 남은 경계

1. 패턴 81은 이번 변경과 무관한 기존 저작 공백 때문에 아직 게시 후보가 아니다.
   `prepare_publication`의 거절 사유는
   `PRODUCT pattern owns a DURATION logic box without a judgementKind: KAKULSAYDON_G1_PATTERN_81.logic.1`이며,
   그 다음 단계에서
   `PRODUCT pattern wires a RESULT logic without an outcomeKind: ... -> kakulsaydon.g1.logic.51`이 나온다.
   `kakulsaydon.g1.logic.82`(`카운터`)와 `kakulsaydon.g1.logic.51`(`결과_카운터`)는 패턴 80
   `세이튼_돌진_카운터`와 공유한다. 기존 선례는 `kakulsaydon.g1.logic.29`
   (`COUNTER_WINDOW`, `endsPatternOnSuccess` true)다. 두 패턴의 카운터 성립 규칙은 저작
   결정이므로 이번에 대신 정하지 않았다. 미리보기 재생은 이 공백과 무관하다.
2. `PATTERNTRACKMOVE` 행의 실제 publish와 Server 파싱은 패턴 81이 게시 가능해진 뒤에만
   실행된다. 현재는 투영 단계까지만 확인했다. 퍼블리셔의 PowerShell 행 생성 경로도
   아직 한 번도 실행되지 않았다. `Publish-GameplayBalance.ps1`은 Kouku projector를
   실제 저장소 경로로 먼저 validate하므로 격리 overlay로도 이 게이트를 우회할 수 없다.
3. 미리보기는 지면 높이·navigation·충돌을 재현하지 않는다. 최종 위치 권위는 Server다.
4. 저작 문서에서 Effect 선택 그룹 ID가 재할당돼도 `selectedEffectGroupId`를 따라 고치는
   경로는 없다. 이번 `.9 -> .11` 드리프트가 그 결과다. Duplicate의 `cloneLogic`은 다른 여섯
   참조 필드만 remap한다.
5. `플레이어 지정`을 사용자가 고르는 UI는 없다. 두 경로 모두 Server가 고른다.
   SELECT_PLAYER는 `Select_BossRandomAliveTarget(..., "albion.airborne.player", ...)`의
   결정적 seed 랜덤이고, `BOSS_TRACK_TARGET`은 `iPatternTargetEntityId` →
   `iTargetEntityId` → 같은 랜덤 선택 순서로 해결한다. 1인 테스트에서는 살아 있는
   플레이어가 하나라 항상 그 플레이어다.
6. 캡처한 장판은 따라오지 않는다. `KoukuSaydonPresentationPlayer.cpp`의 재배치 게이트가
   `(inserted || box.bFollowBoss || row.waitingForAnchor)`이고 `showtime.fixed` 시각은
   `followBoss`가 false라 pivot을 한 번만 계산하고 고정한다. 앵커 문자열이 아니라
   loop=false template의 성질이다.

## 사용자가 이어서 할 순서

1. Tool에서 Composition을 `Reload`한다. 에이전트가 디스크 저장본을 바꿨고 revision이 올라갔다.
2. 패턴 81을 Server Play로도 쓰려면 `카운터` Logic에 judgement kind를, `결과_카운터`에
   outcome kind를 지정한다.
3. `Publish All Patterns`.
4. Client/Server를 같은 소스로 다시 빌드하고 재시작한다. G02의 C++가 실행 파일에 들어가야
   하고, G01의 게시 데이터는 Server 재시작 뒤 적용된다.
5. 실제 아레나에서 확인한다. 노란장판펑펑은 선택된 플레이어 발밑에서 칼날 원형이 시작하는지,
   공굴리기 카운터는 세이튼이 대상을 향해 돌면서 그 방향으로 굴러가고 불뿜기가 함께 도는지 본다.
