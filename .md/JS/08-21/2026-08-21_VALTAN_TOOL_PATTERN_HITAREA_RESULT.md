# 2026-08-21 애니메이션 툴 발탄 시퀀스 재생 패턴 히트 와이어 RESULT

요청: 발탄 아레나에서 패턴 사용 시 나오는 패턴 콜라이더(08-20 작업)를, 애니메이션
툴에서 발탄 선택 시 뜨는 시퀀스 참조 창의 버튼으로 재생할 때도 표시한다.

## 0. 결론

`_DEBUG` Client의 Animation Tool에서 발탄 시퀀스를 재생하면(시퀀스 참조 창 버튼,
1-67 패턴 프리뷰 모두), 재생 중인 source action을 `ValtanEncounter.json` 패턴과
조인해 현재 경과 시각의 스테이지 히트 셰이프를 씬 발탄 위에 아레나와 동일한
와이어로 그린다. 그리기는 기존 `CValtan::Draw_PatternHitAreaDebug` 하나를 그대로
재사용하고, 툴은 스테이지 actionId와 스테이지-로컬 시계를 주입만 한다. 판정
코드·Server·protocol 변경 없음.

## 1. 구조 (조사 결과)

- 아레나 와이어(08-20)는 snapshot `m_strServerActionId`/`m_fServerActionAgeSeconds`
  기준으로 encounter 스테이지 셰이프를 그린다. 툴 재생에는 서버 시계가 없다.
- 툴 재생(`CAnimation_Tool`)은 playlist 항목마다 `iSourceActionId`(원본 액션 PK,
  예: 420633)와 source stage cut 길이를 갖고 로컬 시계로 clip을 체인 재생한다.
- `ValtanEncounter.json` 패턴은 `sourceActionIds` 배열을 저작하고 있으나
  `CEncounterPatternReference`가 검증만 하고 버리고 있었다. 스테이지에는
  `iStartOffsetMs`(로드 시 파생)와 `iDurationMs`가 있어 시각→스테이지 조회가 된다.
- 툴의 발탄 대상은 `CAnimationTargetService::Bind_Preview(CValtan)`로 바인드된
  씬 발탄 객체다(접근자는 없었음).

## 2. 실제 변경 (전부 Client)

- `EncounterPatternReference.h/.cpp`: `ENCOUNTER_PATTERN_REFERENCE`에
  `sourceActionIds` 보존(검증 술어는 기존 `Is_FiniteNumberArray` 그대로).
- `AnimationTargetService.h/.cpp`: `Resolve_Boss()` 추가 — 바인드된 보스 프리뷰
  객체 반환, 보스 대상이 아니면 null.
- `Valtan.h/.cpp` (`_DEBUG`):
  - `Set_PatternHitAreaPreview(stageActionId, ageSeconds)` /
    `Clear_PatternHitAreaPreview()` — 프리뷰 시계 주입. 첫 사용 시 히트 셰이프
    맵을 lazy load(기존에는 서버 권위 인스턴스만 스폰 시 로드).
  - `Draw_PatternHitAreaDebug`: 프리뷰 시계가 설정돼 있으면 서버 snapshot 대신
    그것을 사용. 셰이프 매핑·300ms 최소 표시창 규칙은 기존 코드 그대로.
- `Animation_Tool.h/.cpp`:
  - 재생 Update마다 `Update_ValtanPatternHitAreaPreview()`: 현재 항목의
    `iSourceActionId` → encounter 패턴(`sourceActionIds` 포함 여부) → 전체 패턴
    시계(`base + 현재 항목 경과`)로 스테이지 조회 → 보스에 주입. 패턴 마커,
    조인 실패, 스테이지 범위 밖이면 Clear.
  - 전체 패턴 시계: `Advance`에서 직전 항목이 같은 (pattern, sourceAction,
    sequence, repeat) 타임라인이면 그 재생 길이를 base에 누적, 아니면 0으로 리셋.
    Start/Stop/Reset/타겟 교체 시 리셋 및 보스 와이어 Clear.
  - encounter 참조 문서는 lazy 1회 로드, 실패 시 조용히 비활성(재생은 계속).

새 파일 없음(vcxproj/filters 변경 불필요), Engine/Shared/Server/Data 변경 없음.

## 3. 실행한 검증

```text
Client x64 Debug build                   PASS
ClientFrontendHarness x64 Debug build    PASS
ClientFrontendHarness (변경 후)          failures 45 — 변경 전 오늘 main dd382bf1
  기준선과 동수(전부 Effect/DimensionMaster/Source Authoring 기존 버킷). encounter
  소비 테스트(99 Unique Encounter-Qualified Bindings, Valtan Camera Loads Every
  Exact Encounter Tuple 등) 전부 PASS.
git diff --check                         PASS
```

## 3.1 "일부 시퀀스에서 와이어가 안 나옴" 조사 (2026-08-21, 사용자 A안 확정)

사용자 확인에서 일부 시퀀스는 와이어가 나오고 일부는 안 나왔다. 75개 레이드
시퀀스를 encounter와 대조한 실측:

- 57개: 히트 스테이지 조인 성공 — 와이어 표시.
- 12개: 헬 버전·밈 섬·7주년 등 encounter 33패턴 밖 변형 — 조인 자체가 없어
  미표시(정상).
- 6개(마력구 폭발 420617/618, 부위 파괴·오프닝 420627/628/654/655): 패턴 조인은
  되지만 저작된 히트 스테이지 0. 원본 판정이 발탄 본체 스윙이 아니라 **Effect
  notify가 나르는 SkillEffect PK**(예: 42061702~32, 42060xxx — 마력구 오브젝트/
  장판 폭발)에 달려 있고, `Valtan.animnotify`에 `kind=HIT src=Effect asset=<PK>`로
  이미 추출돼 있다.

결정(A): 와이어는 서버 판정 미러라는 계약 유지 — 서버가 해당 패턴 판정을 갖게
되면 자동 표시된다. Effect PK 기반 참고 와이어(B)와 encounter 히트 저작(C)은
하지 않음(C는 보스/밸런스 저작 몫).

## 4. 남은 것 / 경계

- 사용자 육안 확인 필요: `F1 → Animation Tool → (Development에서 발탄 선택) →
  Valtan Pattern Reference 창의 시퀀스 버튼 재생` 시 와이어 표시.
- 시각 정합의 한계: 툴 시계는 source stage cut 길이, encounter 스테이지는 저작
  durationMs 기준이라 둘이 어긋난 패턴은 와이어 시각이 밀릴 수 있다. 둘 다 원본
  유래라 대체로 일치하지만 스테이지 다듬기 작업에서 재확인.
- 히트 없는 스테이지(WINDUP 등)는 아레나와 동일하게 그리지 않는다(텔레그래프는
  별도 슬라이스). encounter 패턴에 없는 source 시퀀스(비 420xxx 등)는 그리지
  않는다.
