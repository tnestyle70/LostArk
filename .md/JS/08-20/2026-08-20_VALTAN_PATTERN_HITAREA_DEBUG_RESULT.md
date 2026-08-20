# 2026-08-20 발탄 패턴 히트 셰이프 디버그 시각화 RESULT

요청: 창술사 스킬 사용 시 나오는 시각 콜라이더처럼, 발탄 보스도 패턴마다 서버가
판정하는 콜라이더를 시각화한다.

## 0. 결론

`_DEBUG` Client에서 발탄이 패턴 스테이지에 들어가면, 서버가 실제로 판정하는
히트 셰이프(CIRCLE/RING/CONE/BOX/CROSS/SIX_DIRECTIONS)가 히트 시각마다 보스
위치·방향 기준 와이어로 그려진다. 판정은 계속 Server 소유이고 Client는 창술사
스킬 히트 디버그(`CHitAreaWire`)와 같은 미러만 그린다. 사용자 육안 확인(08-20):
**와이어 표시됨**. 다만 보스 패턴 자체가 아직 다듬어지지 않아 셰이프·타이밍의
디테일 정합까지는 판정 보류.

## 1. 구조 (조사 결과)

- Server `ValtanBrain`: 스테이지 진입 시 encounter의 shape/hitCount/hitIntervalMs를
  boss에 싣고, 스테이지 age가 `k * hitIntervalMs`를 넘을 때마다 `ApplyPatternHit`
  (`ContainsPatternHit`의 Shared XZ primitive) — 이것이 판정 정본.
- snapshot(`WORLD_ENTITY_SNAPSHOT`)에는 `strActionId/iActionStartTick/
  iPatternStageIndex`만 있고 셰이프 수치는 없다. protocol은 건드리지 않았다.
- Client에는 이미 encounter 문서를 표시용으로 읽는 `CEncounterPatternReference`가
  있다(Effect Tool/카메라 소비). 이 경로를 확장해 셰이프 수치를 보존시켰다.

## 2. 실제 변경 (전부 Client)

- `EncounterPatternReference.h/.cpp`: `ENCOUNTER_STAGE_REFERENCE`에
  `fHitOuterRadius/fHitInnerRadius/fHitAngleDegrees/fHitLength/fHitHalfWidth/
  iHitCount/iHitIntervalMs` 보존. 검증 술어는 기존과 동일(같은 finite/range 검사를
  하던 값을 버리지 않고 저장만 추가).
- `Valtan.h/.cpp` (`_DEBUG` 전용):
  - 스폰 시(서버 권위 인스턴스만) `ValtanEncounter.json`을 읽어
    `actionId -> PATTERN_HIT_AREA_DEBUG` 맵 구성. 로드 실패는 로그만 남기고 격리.
  - `Late_Update`에서 snapshot `m_strServerActionId`/`m_fServerActionAgeSeconds`
    기준, 각 히트 시각 `k * hitIntervalMs`부터 최소 표시창 300ms(플레이어 스킬
    디버그와 같은 규칙) 동안 `CHitAreaWire::Draw`.
  - 셰이프 매핑: CIRCLE/RING → areaType 1(+inner), CONE → 3(전체 각도),
    BOX → 2(전방 length, 폭 halfWidth*2), CROSS → 중심 스트립
    `[-length,+length]` 2개(0°/90°), SIX_DIRECTIONS → 3개(0°/±60°) —
    Shared `Circle_Intersects*`의 기하와 동일. 미터→cm 변환.
- `ClientReplication.cpp`: 기존 `Set_SkillHitAreaDebugVisible`(Character Select의
  "Show Skill Hit Areas" 체크박스)를 발탄 `Set_PatternHitAreaDebugVisible`로도
  전파, 발탄 스폰 시 현재 토글 적용. 기본값 켜짐 — 체크박스 없는 Valtan Arena
  에서도 보인다.

새 파일 없음(vcxproj/filters 변경 불필요), Engine/Shared/Server/Data 변경 없음.

## 3. 실행한 검증

```text
Client x64 Debug build                   PASS
ClientFrontendHarness x64 Debug build    PASS
ClientFrontendHarness (변경 후)          failures 62 — encounter 소비 테스트는 전부 PASS
  (Valtan Camera Loads Every Exact Encounter Tuple / Rejects Duplicate Tuple /
   99 Unique Encounter-Qualified Bindings 등). 실패는 전부 Effect/DimensionMaster/
   TypeDataMesh 도메인(기존 팀장 몫 버킷).
ClientFrontendHarness (변경 전 baseline) failures 62 — 동수, 이 변경의 회귀 0건
git diff --check                         PASS
```

## 4. 하네스 실패 수 대조

이전 기록상 main 55b843b1의 ClientFrontendHarness 실패는 60건(08-20 오전),
변경 후 62건이 나와 `EncounterPatternReference.h/.cpp`만 stash한 baseline을 같은
바이너리 구성으로 재실행해 대조했다. baseline도 62건 동수 — 이 변경의 회귀는
0건이고, 60→62 증가는 이 작업과 무관한 main 상태(전부 Effect 도메인)다.

## 5. 남은 것 / 경계

- 사용자 육안 확인 완료(08-20): 와이어가 보인다. 셰이프·타이밍 디테일 정합은
  보스 패턴 다듬기(밸런스/연출 작업)와 함께 다시 볼 것.
  Character Select Debug 패널의 `Show Skill Hit Areas`가 토글이다.
- 표시는 서버 판정 시각의 미러다. WINDUP 예고(텔레그래프)는 그리지 않는다 —
  원하면 별도 슬라이스.
- Development preview의 로컬 AI 발탄(비권위)은 서버 타이밍이 없어 그리지 않는다.
