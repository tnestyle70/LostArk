# 2026-08-14 발탄 Animation Tool 참조 워크플로와 타격 시각 원본 좌표 RESULT

작성자: JS · 2026-08-14 · branch `feature/root-motion-navigation-clamp`

선행: `2026-08-14_VALTAN_MONSTER_SKILLTIMING_EXTRACT_RESULT.md` §5의 "타격 시각 미해결",
`.md/JS/08-13/2026-08-13_VALTAN_ORIGINAL_CLIP_EVENT_EXTRACTION_RESULT.md` §3.

PLAN 없음. Animation Tool에서 발탄 패턴을 확인하는 작업 중 사용자 보고를 따라
원인을 좁혀가며 그 자리에서 구현했다.

## 1. 핵심 발견 — 타격 시각은 `.loa`에 있었다

**"몬스터 타격 시각 미상" 결론(08-13·08-14 RESULT)은 폐기한다.**

`.loa`의 `CEFActionNotify_Effect` 레코드는 시각 효과가 아니라 판정 트리거이고,
본문(+28 시각 필드 뒤) **+39 오프셋에 `EFTable_SkillEffect.PrimaryKey` int32**가 있다.
발탄 실측 3,412건 중 3,411건이 살아 있는 DB PK로 해석됐다(오프셋 히스토그램 +39 단일).
notify의 t = 해당 스테이지 클립 로컬의 판정 발사 시각이다.

검증 사례 — 지진파 내려찍기(420615), `mesh_att_battle_15_01`(1.5s):
t=0.400에 PK 42061500/42061530/42061531 발사(임팩트 파티클 0.393과 일치),
t=1.400에 다음 스테이지 전이. 휘두르기(420601)는 t=1.57/1.80에 다단 PK.

같은 조사에서 확정한 부속 사실:

- 액션은 **스테이지 상태기계**다. `MonsterMoveNextStage`(+`Condition{Probability,
  StatusEffect,SkillEffectHit,ChangeTarget,CheckArea}`) notify가 스테이지 전이
  시각을 저작한다(발탄 1,094+@건). `CounterAttack` notify(409건 중 발탄 클립 대역
  8건)가 카운터 가능 창(t, d)을 저작한다.
- 한 클립에 HIT 행이 많은 것은 원작 저작 밀도다: 다단 틱은 notify 반복(대쉬 돌진
  0.2s×5), 한 타격은 HitType별 행 분리(42060401 hittype6 + 42060451 hittype1/경직325,
  형상 동일), 장판 패턴은 발사당 여러 형상 PK(마력구 6개×3회), 값까지 동일한
  tier 중복 PK(42061706=42061732)도 있다.
- 기각된 가설: SkillEffect `Key` ↔ 스테이지 그룹 번호(40액션 대조 불일치),
  Effect notify 본문 +4 정수(키 없는 액션에서도 등장), 7자리 서브스킬 경로
  (86스킬에 SkillEffect 6행뿐, 시각은 또 상수).

## 2. 추출기 확장 (`buildScript/`, 저장소 밖 Git 미추적)

새 스크립트를 만들지 않고 기존을 확장했다. `.loa` 파싱·스테이지 연결·클립
해석·4종 문서 기록이 전부 `extract_action_loa.py`에 이미 있고, 바뀐 것은 notify
클래스 매핑과 PK 해석뿐이라 skilltiming 때(뼈대가 안 맞아 신설)와 반대 판단이다.

- `extract_action_loa.py`
  - 선택적 6번째 인자 `<TABLE_DIR>`. 주어지면 `EFTable_SkillEffect`에서 PK별
    판정성(area/freeze/push/repeat)을 읽어, Effect notify를 **판정형 `HIT`
    (asset=PK)** / 부기형 `EFFECT`(asset=PK)로 분리한다. 인자 생략 시(플레이어
    경로) 동작 불변.
  - KIND에 `CounterAttack -> COUNTER`(윈도우), `MonsterMoveNextStage`+조건 5종
    `-> STAGE`(점) 추가. STAGE 행 label에 전이 조건(`next`/`probability`/...)을 쓴다.
- `extract_monster_skilltiming.py`
  - shape 행에 `pks="42061500,42061510,..."` — 같은 파라미터를 공유하는 원본 PK
    전체 목록. tier 중복은 이전처럼 한 행으로 접되 PK 목록으로 대응 관계를 보존한다.

재추출 결과(`Data/Animation/Reference/Valtan/`, 이번 커밋 포함):

- `Valtan.animnotify` 108클립 1,218행 = HIT 408 / EFFECT 405 / SOUND 261 /
  STAGE 75 / SHAKE 61 / COUNTER 8. 기존 EFFECT 813 중 408이 판정 트리거였다.
- `Valtan.animevents` 1,173건(툴 v3 미러).
- `Valtan.skilltiming` 100액션 474 shape 행에 `pks=` 추가.
- `Valtan.clipmap`/`Valtan.clipseq`는 재생성해도 바이트 불변(확장 무영향 증거).

## 3. Animation Tool 변경 (`Animation_Tool.h/.cpp`, `AnimationPreviewAssets.h`)

발단: 발탄 프리뷰에서 Chain이 안 뜨던 문제. 에셋명 `"Boss_Valtan" -> "Valtan"`
수정(전 세션분 유지)만으로는 부족했고, 재생 전용(`bPlaybackOnly`) 분기가 참조 문서
로드와 Chain 렌더 자체를 하지 않던 것이 실제 원인이었다.

- 클립 목록: animset에 `mesh_` 중복이 있는 body 클립 24종을 숨기고 안내 문구 표시
  (이름 대조 기반, 발탄 하드코딩 없음). clipseq 소속 클립에 `<chain xN>` 표시
  (로드 시 1회 집계).
- 재생 전용 분기에 clipmap/clipseq/animnotify/skilltiming 로드와 Chain 패널,
  아래 두 패널, 상태 문구를 추가했다. 저작(이벤트/스킬바인딩) 비활성은 유지.
- **Original Notifies 패널**(신설 `Render_NotifyReference`): 현재 클립의 원본
  notify를 시간순 표시(프레임/종류/구간/asset), 행 클릭 = 일시정지 + 해당 시각
  seek, 재생 중 활성 구간 하이라이트. HIT 행 클릭 시 `Bind_ReferenceWire`가
  asset의 PK를 skilltiming `pks=`와 토큰 대조해 **해당 형상 wire를 자동 선택**한다.
- Skill Timing Reference: 읽기 전용 모드에서는 stamp 대신 형상 행이 wire 토글이
  된다(선택 형상을 하늘색 와이어로 상시 표시, 라벨에 대표 PK 표기). 플레이어
  full 경로의 stamp UI는 불변.
- `Render_HitAreaWires`: 기준 transform을 CCharacter 전용에서
  `CAnimationTargetService::Resolve_RootTransform`(프리뷰 모델 우선)으로 교체해
  보스 프리뷰에도 그려진다. 형상 그리기를 `Draw_Area` 람다로 추출해 authored
  이벤트(적색/황색)와 참조 형상(하늘색)이 공유한다. 플레이어 경로는 같은 값이라
  동작 불변.
- `EVENT_KIND`에 `COUNTER`(윈도우, Is_Window 순서 준수)·`STAGE`(점) 추가,
  `Kind_Name`/notify 파서/이벤트 문서 로더에 등록.

## 4. 검증

- 자동: Client x64 Debug 빌드 exit 0 (변경 반복마다 총 5회). `git diff --check`
  통과. clipmap/clipseq 바이트 불변 확인. Effect notify PK 유효율 3,411/3,412,
  `Key↔그룹`·`+4 정수` 가설은 전 액션 상관 대조로 기각 기록.
- ProjectAudit: 123건 중 29건 실패 — 변경 전 기준선과 동일 목록(effect 24건 +
  기존 5건). 이번 변경으로 새로 깨진 검사 없음.
- 수동(사용자, 2026-08-14): 발탄 프리뷰에서 body 중복 27종 숨김 확인, Chain 패널
  표시 확인, 참조 형상 하늘색 와이어 표시 확인. **Original Notifies의 HIT 행
  클릭 -> seek + wire 자동 바인딩은 화면 확인 전이다.**

## 5. 남은 경계

- 프리뷰와 제품 스폰의 외형 차이는 모델이 아니라 조립이다: 프리뷰는
  `MN_RPBF_01.wmodel` body만이고 제품은 `ValtanWeapon.wmodel` 파츠를 소켓한다
  (`BossCatalog.json` 대조, Loader가 bodyModel 일치를 검증). 프리뷰 무기 조립은
  별도 작업으로 보류.
- 루프 스테이지의 notify t는 스테이지 로컬이라 클립 길이를 넘을 수 있다
  (`att_battle_17_loop` t=3.5/7.0). 원본 의미 그대로 둔다.
- 420600 "AI 조립용" 공용 풀을 참조하는 cross-action PK는 소유 액션의 skilltiming
  행에서 접힌다. wire 바인딩은 전 액션을 검색하므로 동작하지만, 표시상 다른
  스킬 행이 선택될 수 있다.
- 동일 (t, PK) 완전 중복 notify는 원본 그대로 표시한다. 접기/묶음 표시는 사용자
  결정 대기.
- `MN_RPBF_02`(2페이즈 망령화)는 어떤 경로에도 미배선. 이 발견을 서버
  `ValtanEncounter`/`BossProfiles` 저작에 반영하는 것은 다음 수직 슬라이스다.
- 추출 스크립트 2종의 변경은 저장소 밖 `buildScript/`에 있다(관례 유지).

## 6. 산출물 위치

- 코드: `Client/Public/Animation_Tool.h`, `Client/Private/Animation_Tool.cpp`,
  `Client/Public/AnimationPreviewAssets.h`
- 데이터: `Data/Animation/Reference/Valtan/Valtan.{animnotify,animevents,skilltiming}`
- 스크립트: `C:\Users\95jus\Desktop\buildScript\extract_action_loa.py`,
  `extract_monster_skilltiming.py` (Git 미추적)
- 원본: `C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data3\...\XmlData\Action\
  MN_RPBF_00.loa`, `data2\...\TableData\EFTable_SkillEffect.db`
