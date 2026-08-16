# 2026-08-16 플레이어 스킬 히트 셰이프 원작 데이터 자동 반영 RESULT

작성자: JS · 2026-08-16 · branch `feature/skill-hit-shape-official-import`

PLAN 없음. 사용자 요청("Q를 쓰면 바인딩된 스킬의 원작 히트 콜라이더가 저작 없이 기본으로
보이게")을 조사하면서 그 자리에서 구현했다. 이전 세션이 `X`로 종료되어 이 세션이 이어받았다.

## 1. 요청과 결론

- 요청: 창술사 등 플레이어 스킬 클립의 HIT 이벤트(box/circle/fan)를 손으로 저작하지 않고
  원작 데이터에서 채우고, 게임 중 스킬을 쓰면 그 콜라이더가 캐릭터에 시각적으로 나오게.
- 결론: (1) `.skilltiming` 누락 스킬 재추출 → (2) Animation Tool Import가 셰이프를 자동
  채움 → (3) 네 클래스 `.animevents` HIT 행을 스크립트로 일괄 채움 → (4) 런타임 `CCharacter`가
  `.animevents` HIT 행을 읽어 활성 구간에 와이어를 그림(Debug, 기본 ON). 버튼 없이 동작한다.

## 2. 데이터

### 2.1 `Data/Animation/Reference/<Class>/<Class>.skilltiming` (Task ①, 이전 세션)

원본 `EFTable_SkillEffect.db`를 다시 쿼리해 07-31 추출에서 빠진 base 스킬 행을 추가했다.
순수 추가 diff이며 기존 행은 바이트 불변이다.

| 파일 | 행 | 추가 스킬 |
|---|---|---|
| LanceMaster | 50 → 59 | 34000 34010 34040 34140 34160 34510 34540 34580 34610 |
| Artist | +20줄 | LMB/ACTIVE base 행 |
| DimensionMaster | +11줄 | 〃 |
| Warlord | +17줄 | 〃 |

GunSlinger/Slayer는 사용자 지시로 제외. 회피(34020/34520 등 이동기)는 원작에도 히트 행이 없다.

### 2.2 `Data/Animation/Authored/<Class>/<Class>.animevents` (Task ③)

`Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py`가 Reference `.animnotify`의
HIT notify로 `src=orig` HIT 행을 재생성하고 `.skilltiming` 셰이프를 찍는다. 재생성 전
기존 `src=orig` HIT 행과 (clip, startms, endms) 집합이 네 클래스 모두 정확히 일치함을 확인했다
(Import 로직 미러 검증). 헤더 row count 불변, 제자리 교체, 두 번째 실행은 `unchanged`.

| 클래스 | HIT 행 | 셰이프 채움 |
|---|---|---|
| LanceMaster | 166 | 166 |
| Artist | 1 | 1 |
| DimensionMaster | 19 | 13 |
| Warlord | 32 | 30 |

셰이프 매핑 규칙(Import와 동일): clip → `.clipmap` owner skill → `.skilltiming`의 같은 ID 행,
없으면 첫 `base=` 변형 행. k번째 HIT notify ← 참조 k번째 hit(k = `.clipseq` 첫 체인에서 앞
클립들의 distinct HIT 수 + 클립 내 순번). 참조가 부족하면 마지막 hit 반복, `area=0`이면 안 채움.

## 3. 코드

| 파일 | 변경 |
|---|---|
| `Client/Public/HitAreaWire.h`, `Client/Private/HitAreaWire.cpp` (신규) | `HIT_AREA_SHAPE` + `CHitAreaWire::Draw(root, shape, rgba)`. Animation Tool의 Draw_Area 람다를 공용화(ImGui background draw list, 100 units = 1 m). |
| `Client/Private/Animation_Tool.cpp` | `Render_HitAreaWires`가 공용 drawer 사용. `Import_Notifies(clip, rate, iShapedHits&)`: HIT 노티에 참조 셰이프 자동 복사, 상태줄에 `N HIT shape(s) from skill timing reference`. `Find_ReferenceRow`, `Count_PrecedingChainHits`, `Count_DistinctHitNotifies` 추가. |
| `Client/Public/Animation_Tool.h` | 위 선언. |
| `Client/Public/AnimationEffectCueDocument.h/.cpp` | `ANIMATION_HIT_CUE`(clip, start/end, rep/repms, shape) + `Hits` 벡터. `.animevents` HIT 행 파싱(clip/time/shape fail-closed). |
| `Client/Public/Character.h`, `Client/Private/Character.cpp` | `_DEBUG` `m_isSkillHitAreaDebugVisible = true`, `Draw_SkillHitAreaDebug()`: body model 현재 클립·로컬 ms로 활성 HIT 행(rep/repms 포함)을 와이어로 그림. Effect prewarm 실패 시에도 Hits는 유지. |
| `Client/Public/ClientReplication.h/.cpp` | `Set_SkillHitAreaDebugVisible` 전파, spawn 시 적용(기본 true). |
| `Client/Public/Level_CharacterSelect.h/.cpp` | Debug 패널 `Show Skill Hit Areas` 체크박스(기본 ON). |
| `Client/Default/Client.vcxproj(.filters)` | HitAreaWire 등록(`02.GameObjects\00. Character`). |

## 4. 검증

- 자동: `git diff --check` 통과. fill 스크립트 미러 검증(§2.2), 재실행 idempotent. **빌드는
  사용자 규칙대로 실행하지 않았다.**
- 수동(사용자, 미실행): Client Debug 빌드 → Character Select 진입 → 창술사 Q/W/E/LMB 사용 시
  붉은 와이어(fan/circle/box)가 히트 구간 동안 캐릭터 발밑에 그려지는지. Debug 패널
  `Show Skill Hit Areas`로 끌 수 있다. Animation Tool에서 `Import original`을 누르면 셰이프가
  자동으로 채워지는지(상태줄 문구).

## 5. 남은 경계 / 조사 메모

- 표시는 Client Debug 와이어일 뿐 Server 판정은 여전히 `PlayerSkills.json maximumRange`
  원형이다(`PlayerSkillSystem.cpp:518`). 셰이프를 Server 판정으로 올리는 것은 별도 수직 슬라이스.
- 플레이어 `.animnotify`의 HIT는 `ParticleHit` notify만 HIT로 분류돼 있다. Artist(1클립),
  DimensionMaster(10), Warlord(26)는 HIT notify 자체가 적어 셰이프가 붙어도 표시 대상 클립이
  적다. `extract_action_loa.py`에 TableData 인자를 주면 `CEFActionNotify_Effect`의 SkillEffect PK가
  플레이어에서도 해석된다(LanceMaster 94행, Warlord 51행 확인 — 예: 34010 → PK 340100/1/2). 이
  경로로 재추출하고 `.skilltiming` hit 행에 `pks=`를 붙이면 순번 추정 대신 PK 정확 매칭이 된다.
  단 Artist(`YINYANGSHI.loa`, 현재 프리픽스로 12클립만 해석)와 DimensionMaster(0클립)는 원래
  추출 명령/모델이 문서에 없어 재현하지 못했다. 다음 작업 후보.
- 순번 매핑은 노티 수 ≠ 참조 hit 수인 스킬(34580 노티1/참조4, 34550 노티4/참조2)에서 추정이다.
- Animation Tool이 열려 있으면 툴 와이어와 캐릭터 와이어가 같은 자리에 겹쳐 그려진다(같은 색).

## 6. 산출물 위치

- 코드: §3 표.
- 데이터: `Data/Animation/Reference/{LanceMaster,Artist,DimensionMaster,Warlord}/*.skilltiming`,
  `Data/Animation/Authored/{LanceMaster,Artist,DimensionMaster,Warlord}/*.animevents`
- 스크립트: `Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py`
  (`<blender-python> fill_animevents_hit_shapes.py LanceMaster Artist DimensionMaster Warlord [--check]`)
- skilltiming 재추출기(이전 세션 작성): `C:\Users\95jus\Desktop\buildScript\extract_player_skilltiming_v2.py`
  (Git 미추적, `validate <Class>` / `fill <Class>` 모드)
