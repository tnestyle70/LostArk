# LostArk 창술사(LanceMaster) 애니메이션 이벤트 — 언팩된 LPK에서 추출

작성자: JS · 2026-07-30

풀린 LPK(`C:\Users\95jus\Downloads\SourceData\SourceData\LPK`)에서 창술사 애니메이션
이벤트(공격 타이밍·사운드·효과)를 실제로 추출했다. **`.lpk`의 base16 벽은 이 데이터로
사실상 우회됐다** — 누군가 이미 복호화해 SQLite로 풀어놨다.

## 1. 풀린 LPK 실측

```text
config/ data1/ data2/ data3/ data4/ font/ leveldata1/ leveldata2/
manifest.json (56MB), sqlite_validation.json (4.8MB)
파일: .db 792개, .loa 111,770개, .epf 1,364개
```

- **`.db` 792개 = SQLite** (`data2/EFGame_Extra/ClientData/TableData/EFTable_*.db`).
  `sqlite_validation.json`으로 PRAGMA 검증까지 돼 있다. 앞 문서의 레시피(per-table key
  + AES-256 CBC + zero-IV/1024B)가 실제로 통한 결과물이다.
- `.loa`는 NPCFunction/Projectile/Quest/Collision 바이너리 시퀀스(플레이어 애니 이벤트
  아님).

## 2. 창술사 특정

`EFTable_PC.db` → **PrimaryKey 305 = LanceMaster**:

```text
Name=LanceMaster, LookInfo=EFDLChar_PC_FLM.PC_FLM,
SkillCamLevelFileName=Standard_SkillCam_LanceMaster
skill IDs: WeaponAttack 34010, HandAttack 34009, Move 34020/34520,
           Getup 34030, Identity 34000/34500/34001/..., Confront 34900, Avoid 34910
```

→ 창술사 스킬은 **PrimaryKey 34000~34999** 대역이다(`EFTable_Skill.db`, 총 42,715행).

## 3. 애니메이션 이벤트가 사는 곳 — LOA 데이터 모델

LOA는 이벤트를 애니 에셋에 심지 않고 **데이터 테이블로 구동**한다. 창술사 기준:

| 원하는 것 | 어디 | 필드/값 |
|---|---|---|
| **공격 타이밍** | `EFTable_SkillEffect.db` | `HitTypeTimeMin/Max` = 애니 시작 기준 타격 판정 윈도우(ms) |
| 히트스톱/경직 | 〃 | `FreezeTime` (예 325~600ms) |
| 넉백/밀림 | 〃 | `PushMinTime/MaxTime` (예 110~200ms) |
| 다단히트 | 〃 | `MultiHitCount`, `MultiHitTime`(간격 ms) |
| **사운드/피격 이펙트** | 〃 | `HittedSetSkillKey` (예 `SkHit_Normal1`) = 히트 사운드/FX 세트 참조 |
| 조건부 효과/사운드 | `EFTable_CombatEffect.db` | Condition/Action 시스템(11,334행) |
| **궁극기 연출 타임라인** | `Standard_SkillCam_LanceMaster.upk` | 애니 클립·이벤트·셰이크 시각(별도 추출 완료) |

`EFTable_SkillEffect`의 효과 PrimaryKey = **스킬ID×10 + 변형**(예 340440 → 스킬 34044).
창술사 효과 대역 340000~349999에 **5,553행, 그중 타이밍 채워진 1,981행**.

**시간 단위는 밀리초.** (freeze 325 = 0.325s, push 110 = 0.11s)

## 4. 추출 결과 (창술사 스킬 50개, 히트 타이밍)

`scratchpad/lancemaster_skill_timing.json`에 스킬별로 저장. 예:

```text
skill 34044: 첫 타격 판정 1.657~1.744s, freeze 0.45s, 효과행 20개
skill 34048: 첫 타격 0.30~0.316s, push 0.20s, freeze 0.45s, 효과행 60개(다단)
skill 34090: 첫 타격 1.73~1.821s, freeze 0.45s
skill 34100: 첫 타격 1.39~1.463s, freeze 0.325s
```

즉 "이 스킬 애니는 시작 후 몇 초에 타격이 들어가고, 얼마나 경직/넉백을 주는가"를
스킬 50개에 대해 **실측 값으로** 확보했다. 사운드는 `SkHit_Normal1` 히트셋으로 연결된다.

## 5. 무엇이 있고 무엇이 없나 (정직하게)

- **있다**: 스킬별 타격 판정 시각(ms), 경직/넉백/다단 타이밍, 피격 사운드/FX 세트,
  그리고 궁극기 3종의 정밀 연출 타임라인(SkillCam UPK).
- **단일 "프레임별 파티클 스폰 노티파이"는 없다.** 일반 스킬의 시전 이펙트 시각은
  하나의 컬럼이 아니라 애니 시스템 + SkillEffect + 히트셋이 런타임에 합성한다.
  프레임 단위 par_* 스폰이 명시된 곳은 **궁극기 SkillCam 타임라인**뿐이다(이미 추출).
- 즉 처음 가정한 "애니마다 박힌 노티파이 목록"은 이 게임 구조상 존재하지 않고,
  **데이터 구동형**이다. 우리 엔진에 이식할 땐 SkillEffect의 hit window/freeze/push를
  전투 타이밍으로, SkillCam을 궁극기 연출로 쓰면 원본 값 그대로 재현된다.

## 6. 산출물

```text
scratchpad/lancemaster_skill_timing.json     창술사 50개 스킬 히트 타이밍(ms→s)
scratchpad/lancemaster_ultimate_timeline.json 궁극기 3종 연출 타임라인(앞 작업)
언팩 원본: C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data2\...\EFTable_*.db
```

조회 방법: `sqlite3`(파이썬 표준 라이브러리)로 `.db` 직접 쿼리. 원본은 Smilegate
저작물이며 언팩본·추출본은 Git에 올리지 않는다.
