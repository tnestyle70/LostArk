# 2026-08-19 발탄 비석(기둥) 사이클 제품 트리거 계획

이 문서는 `Shared/Public/Network/PacketMessages.h:664`와 `Server/Private/GameRoom.cpp:23-35`가
`no product trigger for the shatter is identified yet`로 열어 둔 항목을 다룬다.

**이 문서는 아직 구현 계획이 아니다.** 1-67 clip 목록에서 새로 확인된 근거와, 그 근거로
닫힌 질문·아직 열린 질문을 분리해 기록한다.

## 1. 지금까지 막혀 있던 이유

기둥은 올라오는 것만 제품 경로다.

```text
Server/Private/GameRoom.cpp:28-29
	PILLAR_PATTERN_ID      = "VALTAN_FOUR_PILLARS_105"
	PILLAR_SPAWN_STAGE_ID  = "RECOVERY"
```

부서지는 것은 Debug audition `PLAY_PILLAR_CYCLE`만 예약한다. dwell은 영상 실측
`PILLAR_AUDITION_DWELL_TICKS = 475`(15.82초, 16:42.74 -> 16:58.56)이고 주석이
`No product trigger reads this`라고 못 박아 두었다.

같은 주석이 `The video shows three further cycles, but no product pattern, stage or binding is
identified for them`이라고 적었다. 이 "세 사이클"의 정체가 이번에 확인됐다.

## 2. 1-67 clip 목록이 제공한 근거

정본은 `Data/Animation/Authored/Valtan/Valtan.patternpreview.json`(formatVersion 1, 67 entry)이다.

### 2.1 비석 생성과 폭파의 원본 action ID

| clip | label | sourceActionId |
|---|---|---|
| 26 | 비석 4개와 방향 에너지파 | `420608` |
| 27 | 붉은 도끼와 비석 2개씩 폭파 | `420636` |
| 33 | 비석 4개와 기 방출 넉백 | `420608` |
| 34 | 붉은 도끼와 비석 폭파 | `420636` |
| 49 | 29줄 비석 4개와 기 방출 | `420608` |
| 50 | 붉은 도끼와 비석 폭파 반복 | `420636` |

세 사이클이 완전히 일관된다. 생성은 언제나 `420608`, 폭파는 언제나 `420636`이다.

각 entry의 note가 `비석 생성과 방향 에너지파는 병렬 월드·판정 시스템이다`라고 적었다.
즉 이 두 ID는 **비석 자체가 아니라 그때 본체가 재생하는 몸통 시퀀스**다. 이 구분은 아래
2.4의 결론에 그대로 영향을 준다.

### 2.2 세 사이클이 걸리는 체력바

clip 목록의 순서가 곧 전투 진행 순서다.

```text
25  100줄 중앙 착지와 2페이즈 전환      <- bar 100
26  비석 4개 생성
27  붉은 도끼와 비석 2개씩 폭파

32  84줄 아레나 외곽 절반 붕괴          <- bar 84
33  비석 4개 생성
34  붉은 도끼와 비석 폭파

48  30줄 중앙 착지와 지반 절반 삭제     <- bar 30
49  29줄 비석 4개 생성
50  붉은 도끼와 비석 폭파 반복
```

**비석 사이클은 자유 선택 패턴이 아니라 체력바 100 / 84 / 30 직후에 붙는 고정 시퀀스다.**
이것이 이번에 새로 닫힌 가장 큰 질문이다.

### 2.3 현재 생성 바인딩은 근거상 맞다

`VALTAN_FOUR_PILLARS_105`는 `triggerHealthBar 100`, `sourceActionIds [420610]`이고 stage가
`TAKEOFF / YELLOW_ZONE / TARGET_CONE / RECOVERY`다. clip 25 `100줄 중앙 착지`가 이 패턴이고,
그 꼬리(RECOVERY)에 clip 26 생성을 붙인 현재 바인딩은 **첫 사이클에 한해 시각이 맞는다.**

이름만 보고 고른 바인딩이 아니었다. 다만 사이클 2, 3에는 대응하는 것이 없다.

### 2.4 원본 패턴에 그대로 묶으면 안 된다

`420608`과 `420636`은 제품 패턴에 이미 매핑돼 있다.

```text
420608 -> VALTAN_MAGIC_CHOICE      selectionMode NORMAL, triggerHealthBar 0
420636 -> VALTAN_RED_BLADE_WAVE    selectionMode NORMAL, triggerHealthBar 0
```

둘 다 AI가 가중치로 아무 때나 반복해서 고르는 패턴이다. 여기에 비석 생성/폭파를 묶으면
전투 내내 비석이 오르내린다.

더 결정적인 반례가 clip 목록 안에 있다. `420636`은 비석과 무관한 3페이즈 occurrence에도
등장한다.

```text
57  37줄 무작위 위치 충격파          420636
61  무작위 충격파와 포탈 분신 돌진    420636
65  무작위 위치 충격파 반복          420636
```

이 셋에는 비석이 없다. 따라서 `420636`은 "붉은 검기 계열 몸통 동작"일 뿐이며 폭파의
소유자가 아니다. **비석 사이클의 소유자는 원본 action ID가 아니라 체력바 게이트다.**

### 2.5 기존 제품 매핑은 서로 어긋나 있다

`Data/Encounters/Valtan/ValtanDebugAudition.json`이 occurrence를 제품 패턴에 매핑한다.

```text
25  PRODUCT_CANDIDATE  VALTAN_FOUR_PILLARS_105   bar 100
26  PRODUCT_PARTIAL    VALTAN_MAGIC_CHOICE       bar 0
27  PRODUCT_PARTIAL    VALTAN_RED_BLADE_WAVE     bar 0
32  PRODUCT_CANDIDATE  VALTAN_ARENA_BREAK_84     bar 84
33  PRODUCT_PARTIAL    VALTAN_FOUR_PILLARS_105   bar 0
34  PRODUCT_PARTIAL    VALTAN_RED_BLADE_WAVE     bar 0
48  PRODUCT_CANDIDATE  VALTAN_ARENA_BREAK_33     bar 30
49  PRODUCT_PARTIAL    VALTAN_FOUR_PILLARS_105   bar 29
50  PRODUCT_PARTIAL    VALTAN_RED_BLADE_WAVE     bar 0
```

같은 생성 occurrence인데 26은 `VALTAN_MAGIC_CHOICE`, 33과 49는 `VALTAN_FOUR_PILLARS_105`로
갈라져 있다. 세 occurrence의 원본 `sourceActionId`는 전부 `420608`로 동일하므로 이 분기는
근거가 아니라 매핑 시점의 추정 차이다. 여섯 행 모두 `PRODUCT_PARTIAL`, 즉 작성자 스스로
exact mapping이 아니라고 표시해 두었다.

이 어긋남 자체가 결론을 뒷받침한다. **비석 사이클은 기존 패턴 중 하나를 골라 붙일 대상이
아니라 자기 패턴을 가져야 한다.**

### 2.6 필요한 표현 수단은 이미 있다

`BOSS_PATTERN_DEFINITION::iTriggerOrder`가 존재하고 `CValtanBrain`이 실제로 소비한다.

```text
Server/Private/ValtanBrain.cpp:163-168
	같은 iTriggerHealthBar 안에서는 iTriggerOrder 오름차순으로 큐에 넣는다
```

현재 체력바 패턴 9개는 전부 `triggerOrder 1`이다. 같은 바에 `triggerOrder 2`, `3`을 추가하면
기존 바 패턴 직후에 순서대로 실행된다. 새 엔진 기능이 필요 없다.

```text
bar 100  order 1  VALTAN_FOUR_PILLARS_105    (clip 25)
bar  84  order 1  VALTAN_ARENA_BREAK_84      (clip 32)
bar  30  order 1  VALTAN_ARENA_BREAK_33      (clip 48)
```

## 3. 근거로 닫힌 것과 아직 열린 것

### 3.1 닫힌 것

- 비석 사이클은 정확히 3회이고 체력바 100 / 84 / 30 직후다.
- 각 사이클은 생성(`420608` 몸통) -> 폭파(`420636` 몸통) 두 occurrence다.
- 원본 패턴 `VALTAN_MAGIC_CHOICE` / `VALTAN_RED_BLADE_WAVE`에 직접 묶으면 안 된다.
- 표현 수단은 기존 `triggerHealthBar` + `triggerOrder`로 충분하다.

### 3.2 열린 것 — 사용자 결정 필요

**(1) 생성과 폭파 사이 15.82초를 무엇이 채우는가.**
Debug dwell `475 tick`은 영상 실측이지만, 그 사이에 보스가 무엇을 하는지는 clip 목록이
말해 주지 않는다(26과 27이 인접 occurrence다). `VALTAN_RED_BLADE_WAVE`의 WINDUP은 1000ms라
15.82초를 설명하지 못한다. 생성 패턴이 긴 dwell stage를 갖는지, 폭파 패턴의 windup이 긴지,
아니면 그 사이에 다른 행동이 있는지 영상을 다시 봐야 한다.

**(2) `2개씩`의 의미.**
clip 27만 `비석 2개씩 폭파`라고 적었고 34, 50은 그냥 `비석 폭파`다. 현재
`CEncounterPropRuntime`은 슬롯 4개를 한 트랜잭션으로 함께 올리고 함께 부순다. 2개씩이
`2 + 2 순차`인지, `4개 중 2개만`인지, 아니면 표기 차이인지에 따라 runtime 구조가 달라진다.

**(3) 비석이 통행을 막는가.**
`Server/Private/GameRoom.cpp:392-395`가 `They own no collision or navigation until that gameplay
contract is authored separately`라고 적었다. 지금 비석은 순수 표현이라 부서져도 게임플레이
영향이 0이다. 원작에서 비석이 길을 막거나 피해를 준다면 별도 수직 슬라이스가 된다.

## 4. 결정이 나온 뒤의 예상 작업 형태

참고용 스케치이며 확정이 아니다.

```text
Data/Encounters/Valtan/ValtanEncounter.json
  bar 100 order 2  VALTAN_PILLAR_RAISE_100     src 420608
  bar 100 order 3  VALTAN_PILLAR_SHATTER_100   src 420636
  bar  84 order 2  VALTAN_PILLAR_RAISE_84      src 420608
  bar  84 order 3  VALTAN_PILLAR_SHATTER_84    src 420636
  bar  30 order 2  VALTAN_PILLAR_RAISE_29      src 420608
  bar  30 order 3  VALTAN_PILLAR_SHATTER_29    src 420636

Server/Private/GameRoom.cpp
  PILLAR_PATTERN_ID / PILLAR_SPAWN_STAGE_ID 단일 상수를 패턴-스테이지 표로 교체
  폭파도 같은 표에서 제품 경로로 예약 (지금은 m_bPillarAuditionCycleArmed 전용)

Server/Private/ServerGameplayContractTests.cpp
  세 사이클이 각 바에서 order 순서대로 큐잉되는지
  자유 선택 MAGIC_CHOICE / RED_BLADE_WAVE 가 비석을 건드리지 않는지
```

패턴을 6개 늘리면 `Publish-GameplayBalance.ps1`의 provenance receipt도 함께 갱신해야 한다.
`Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`이 저작 field
coverage를 강제하므로, 새 패턴의 근거를 `PROJECT_TUNED`가 아니라 clip 목록 출처로 남길지
publisher 규칙을 먼저 확인해야 한다.

## 5. 무기 allowlist(작업 3번)에 대한 메모

같은 clip 목록이 벽 파괴를 명시한 occurrence는 다음뿐이다.

```text
 1  160줄 첫 등장 회오리        (note: 회오리 이펙트가 벽 파괴를 함께 보여준다)
 4  회오리 공격과 주변 벽 파괴
21  109줄 점프 착지와 추가 외벽 붕괴
32  84줄 아레나 외곽 절반 붕괴
48  30줄 중앙 착지와 지반 절반 삭제
```

즉 **공격으로 벽을 부수는 것이 눈에 보이는 건 회오리뿐**이고, 현재
`Data/Encounters/Valtan/ValtanWallContactActions.json`의 11개는 그보다 넓다.

다만 이것으로 좁히는 결정을 내리면 안 된다. clip 목록은 **한 번의 플레이 기록**이고, 어떤
동작이 그 판에서 벽을 안 부순 것과 부술 수 없는 것은 다르다. 이번 판에서 그 동작이 벽 근처에서
나오지 않았을 뿐일 수 있다.

따라서 3번은 **실제 화면에서 회오리와 몸통 접촉만으로 109줄 전에 내부 벽 67개가 충분히
사라지는지 확인한 뒤** 넓힐지 좁힐지 정한다. 그 전에 JSON을 고치면 근거 없이 고치는 것이다.
