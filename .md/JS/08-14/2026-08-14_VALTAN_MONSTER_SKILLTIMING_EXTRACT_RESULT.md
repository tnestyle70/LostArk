# 2026-08-14 발탄 몬스터 skilltiming 추출 RESULT

작성자: JS · 2026-08-14 · branch `feature/root-motion-navigation-clamp`

선행: `.md/JS/08-13/2026-08-13_VALTAN_ORIGINAL_CLIP_EVENT_EXTRACTION_RESULT.md` §3의
"HIT 타이밍 원본 좌표 미해결" 항목.

PLAN 없음. 원작 자료 존재 여부 조사에서 시작해 그 자리에서 산출물까지 만들었다.

## 1. 조사 결과 — 몬스터 키 규약

어제 "`EFTable_SkillEffect`에 몬스터 대역 없음"으로 닫은 것은 **조회 규약이 달라서**였다.

```
플레이어:  SkillEffect.PrimaryKey = variantId × 10  + n   (variant = skillId + 트라이포드)
몬스터:    SkillEffect.PrimaryKey = actionId  × 100 + n
```

어제는 `4206010..4206639`(×10)로 조회해 0행이었다. `×100`으로 보면 발탄 액션 40개에
1,095행이 있다.

한국어 기획자 라벨은 DB가 아니라 **`.loa` 자체**에 있다. `EFTable_Skill.Name`과 `Comment`는
발탄 대역 60행 전부 빈 문자열이지만, `MN_RPBF_00.loa`의 `CEFActionObject` 레코드가
`skillId` 다음에 UTF-16LE display name을 들고 있다. `extract_action_loa.py`가 이미 이것을
읽어 `Valtan.clipmap`에 넣고 있었다(어제 산출물). clipmap은 clip을 키로 하므로 클립이
해소되지 않는 액션은 행이 안 생겨 41개가 안 보였을 뿐이고, loa에는 처음부터 인카운터
63개가 전부 이름과 함께 있었다. `MN_RPBF_01-1.loa`, `MN_RPBF_02-2.loa`는 인카운터 대역
액션이 0개로 이번 건과 무관하다.

## 2. 조사 결과 — `HitTypeTimeMin`은 타격 시각이 아니다

`EFTable_SkillEffect.HitTypeTimeMin`은 플레이어에게는 타격 시각이지만 몬스터는 저작하지
않는다. 실측:

| | 값 분포 |
|---|---|
| 전체 118,344행 | `0` 84.4%, `1814` 6.3%(7,477행), `2151` 3.6%(4,297행), `1691` 1.6% |
| 창술사 404행(대조군) | 25개 값으로 분산, 최다 `1445` 22회 |
| 발탄 전 액션 | **`{0, 1691, 1814, 2151}` 네 값뿐** |
| 발탄 hit 폭 | **989행 전부 정확히 20ms** (플레이어는 87/76/16 등 제각각) |

`1814`를 쓰는 서로 다른 소유자가 2,502개, `2151`이 1,305개, `1691`이 858개다. 서로 다른
40개 모션이 세 값으로 수렴하고 폭이 전부 같은 것은 테이블 상수라는 뜻이다.

같은 행의 **형상은 패턴별로 저작돼 있다.** 지진파 내려찍기는 `area=3 ar=900 aa=360
arem=400`(내반경 400 / 외반경 900 도넛), 휘두르기는 `area=2 ar=600 aa=150` 부채꼴이
오프셋 3개다. 형상은 패턴명과 일치한다.

## 3. 구현 완료

- `buildScript/extract_monster_skilltiming.py` 신설 (저장소 밖, Git 미추적).
  `extract_skilltiming.py`는 손대지 않았다. variant/base 분리가 그 스크립트의 뼈대인데
  몬스터엔 그 축이 없어 확장하면 거의 모든 줄에 분기가 붙는다. `build_npc.py`가
  `build_character_part.py`와 별도인 것과 같은 선례를 따랐다.
- `Data/Animation/Reference/Valtan/Valtan.skilltiming` 생성.
  format `LOSTARK_SKILL_TIMING 3`, 100 액션 / 474 shape 행.
  - 이름: `.loa` `CEFActionObject` → 100개 전부(인카운터 63 포함)
  - 형상: `area/ar/aa/ah/ax/arem`, `hittype`, `fz*`, `push*`, group 구조
  - 교전 기하: `cd/range/approach/turn` (몬스터 전용 컬럼, 버리지 않고 보존)
  - 시각: `tmpl_t`/`tmpl_w`로 격리. 원본 수치는 남기되 저작 타임라인으로 오인되지 않게 한다
- 7자리 스킬 충돌 처리: 액션 `420670~420678`은 7자리 스킬(`4206701~4206786`)이 별도로
  존재해 그 `×10` 대역이 `×100` 대역과 겹친다. 효과 행의 `PK//10`에 Skill row가 있으면
  그 스킬 소유로 판정해 제외한다(6행).
- 엔진 액션 슬롯(`MOVE`/`DIE`/`SPAWN` 등 8개)은 스킬이 아니라 제외.

format 3은 몬스터 헤더를 뜻한다. Animation Tool은 버전 토큰을 읽고 검사하지 않으며
모르는 key/value를 무시하므로 툴 코드 변경 없이 열린다.

## 4. 검증 (실행함)

- `Client/Private/Animation_Tool.cpp:3098~3231` 파서 대조: 버전 미검사 확인, `shape` 줄은
  `Read_Quoted`가 공백 직후 따옴표를 요구하므로 `g=0`에서 실패해 `continue`. `hitset="..."`의
  따옴표를 이름으로 오인하지 않는다. 가짜 행 없음.
- 인코딩·개행: UTF-8 no-BOM, CRLF 575줄, bare LF 0. `LanceMaster.skilltiming`과 동일.
- 형상↔패턴명 정합을 지진파(도넛)·휘두르기(부채꼴)에서 육안 대조.
- `git status --short`로 이 문서 외 변경 없음 확인.

Client/Server 빌드는 돌리지 않았다. 코드 변경이 없고 Reference 문서는 read-only 저작
자료라 런타임 입력이 아니다.

## 5. 남은 것

- **타격 시각 미해결.** 이번에 `HitTypeTimeMin`이 답이 아님을 확정했을 뿐이다. 다음 후보는
  `Valtan.animnotify`의 `ToggleCollision`(161건)·`EmitTriggerSignal`(30건)으로, 어제 RESULT의
  후보와 같다.
- 이 문서는 저작 참고 자료다. 실제 게임 체감은 여기 수치를 `ValtanEncounter.json` /
  `BossProfiles.json`에 저작해 publish할 때 바뀐다. 문서 생성만으로는 런타임이 변하지 않는다.
- 형상을 Debug wire로 그리는 작업(어제 RESULT의 ③)은 이제 재료가 갖춰졌다.
- 커밋에는 생성 문서만 들어간다. 추출 스크립트는 다른 Reference 문서들과 마찬가지로
  저장소 밖 `buildScript/`에 있다.
