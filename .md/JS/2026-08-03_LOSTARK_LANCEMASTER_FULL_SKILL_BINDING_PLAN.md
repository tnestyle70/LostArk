# 창술사 전체 스킬 바인딩 — 착수 전 조사와 설계

작성자: JS · 2026-08-03
성격: PLAN + 1차 구현 기록. **작업 시작할 때 이 문서를 먼저 읽는다.**
선행 문서: `.md/JS/2026-08-02_LOSTARK_CLASS_LOGIC_SKILLDATA_STANCE_RESULT.md`
정본 계약: `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

## 0. 진행 상태 (2026-08-03)

**긴 창 9슬롯이 실제로 동작한다.** `Q W E R A S`(스탠스 전용) + `T V ALT_V`(스탠스 무관). 검증: Debug 빌드 전체 통과, protocol harness `failures : 0`, server contract test `failures : 0`, ProjectAudit 61 checks, offline smoke 3/3, network endpoint smoke 3/3, 사용자 실행 확인.

| 항목 | 상태 |
|---|---|
| 입력 바인딩 데이터화 (§10) | **완료** — `CPlayerSkillCatalog` 신규, 컨트롤러에 skill ID 하드코딩 없음 |
| 퍼블리셔 스키마 일반화 (§7) | **부분 완료** — 슬롯 화이트리스트와 `hitTimeMs=0` 허용까지. `skillKind`는 미착수 |
| 긴 창 스킬 데이터 투입 (§4.1) | **9/11** — 평타·탄영 제외 (§6 타이밍 없음) |
| 34060/34100 제거 | **완료** — 테스트용이라 양쪽 목록에 없었다 |
| 다단 히트 `hits[]` (§11) | 미착수 |
| Shared 스탠스 계약 (§9) | 미착수 |
| 짧은 창 바인딩 (§4.2) | 미착수 — §9 선행 |
| 표현 재통합 (§8) | 미착수 |

## 1. 결론 먼저

1. **양 스탠스 바인딩 전부 skill ID가 확정됐다.** §3, §4.
2. **이 바인딩은 잠정이다.** 트라이포드·스킬창 시스템을 구현하면 바인딩은 스킬창에서 퀵슬롯에 올리는 **런타임 플레이어 상태**가 된다. 그래서 `inputSlot`을 스킬 정의에 두면 안 된다. §5. **아직 분리하지 않았다** — 긴 창만 넣는 동안은 슬롯이 클래스당 유일해서 문제가 안 났지만, 짧은 창을 넣는 순간 `LANCE_MASTER:Q`가 두 번 나와 막힌다.
3. **타이밍 데이터는 일부가 없다.** 34010/34510 평타, 34050 풍진격, 34170 청룡진, 34580 절룡세. §6.
4. **데이터 계약을 바꿀 때 퍼블리셔와 서버 파서를 짝으로 봐야 한다.** `hitTimeMs != 0` 검증이 `Publish-GameplayBalance.ps1`과 `Server/Private/GameplayCatalog.cpp` 양쪽에 있었고, 퍼블리셔만 풀었다가 서버 bootstrap 로드가 실패했다. §7.
5. **08-02 스탠스 작업이 통합에서 거의 전부 빠졌다.** 보류였고, 지금 그 선행 조건을 만든다. §8.
6. **스탠스는 서버 권위 상태다.** §9.
7. **입력 엣지 함정이 둘 있고 둘 다 실제로 걸렸다.** 슬롯 인덱스 색인(08-02), 공유 키의 읽기/판정/쓰기 미분리(08-03). 짧은 창의 `Z`에서 재발 가능하다. §10.2.

## 2. 스탠스 전용 / 무관을 가르는 마커

`.clipseq` 이름의 접두사가 정본이다.

- `긴 창_` 접두사 → 긴 창 전용
- `짧은 창_` 접두사 → 짧은 창 전용
- **접두사 없음 → 스탠스 무관** (양쪽에서 같은 키)

접두사 없는 스킬: 34600 `연가창식: 은하유성탄` · 34610 `연가창식: 적룡질풍격` · 34620 `초각성기_연가창식: 은하비섬창` · 34630 `초각성기_연가창식: 마룡합일섬` · 34640 `초각성스킬_맹룡난무` · 34650 `초각성스킬_적룡필살` · 34660 `연가비기`

**스탠스 판별을 ID 범위로 하면 틀린다.** 반례: `34630`은 34500 이상이지만 짧은 창 스킬이 아니다(스탠스 무관). 반드시 접두사로 판별한다.

## 3. 스탠스 무관 슬롯 (양쪽 공통)

| 키 | 이름 | skill ID | `.clipseq` 이름 | 타이밍 |
|---|---|---:|---|---|
| T | 맹룡난무 | 34640 | `초각성스킬_맹룡난무` | base 행 |
| V | 은하유성탄 | 34600 | `연가창식: 은하유성탄` | 변형만 (34601) |
| Alt+V | 은하비섬창 | 34620 | `초각성기_연가창식: 은하비섬창` | base 행 |

## 4. 스탠스별 바인딩

### 4.1 긴 창

| 키 | 이름 | skill ID | `.clipseq` 이름 | 타이밍 |
|---|---|---:|---|---|
| 좌클릭 | 평타 | 34010 | `긴 창_평타` | **없음** |
| Q | 연환섬 | 34120 | `긴 창_연환섬` | base 행 |
| W | 일섬각 | 34080 | `긴 창_일섬각` | 변형만 (34087/34088) |
| E | 회선창 | 34070 | `긴 창_회선창` | base 행 |
| R | 맹룡열파 | 34150 | `긴 창_맹룡열파` | base 행 |
| A | 반월섬 | 34110 | `긴 창_반월섬` | base 행 |
| S | 철량추 | 34090 | `긴 창_철량추` | base 행 |
| D | 풍진격 | 34050 | `긴 창_풍진격` | **없음** |
| F | 청룡진 | 34170 | `긴 창_청룡진` | **없음** |
| Z | 아이덴티티 (짧은 창 전환) | 34000 | `긴 창_짧은 창으로 스탠스 변경` | 없음 (정상) |
| Space | **탄영** (이동기) | 34020 | `긴 창_탄영 [ 이동기 ]` | 없음 (정상) |

### 4.2 짧은 창

| 키 | 이름 | skill ID | `.clipseq` 이름 | 타이밍 |
|---|---|---:|---|---|
| 좌클릭 | 평타 | 34510 | `짧은 창_평타` | **없음** |
| Q | 나선창 | 34540 | `짧은 창_나선창` | 변형만 (34545~34549) |
| W | 사두룡격 | 34550 | `짧은 창_사두룡격` | base 행 |
| E | 굉열파 | 34560 | `짧은 창_굉열파` | base 행 |
| R | 유성강천 | 34570 | `짧은 창_유성강천` | base 행 |
| A | 절룡세 | 34580 | `짧은 창_절룡세` | **없음** |
| S | 적룡포 | 34590 | `짧은 창_적룡포` | base 행 |
| Z | 아이덴티티 (긴 창 전환) | 34500 | `짧은 창_긴 창으로 스탠스 변경` | 없음 (정상) |
| Space | **돌파** (이동기) | 34520 | `짧은 창_돌파 [ 이동기 ]` | 없음 (정상) |

D·F는 짧은 창에 없다. **짧은 창 스킬 9개(34500·34510·34520·34540·34550·34560·34570·34580·34590)가 이 목록으로 전부 소진된다** — 08-02 실측 "긴 창 18스킬 / 짧은 창 9스킬"과 정확히 일치한다.

### 4.3 바인딩되지 않은 긴 창 스킬 (데이터에는 있음)

34030 기상기 · 34040 이연격 · **34060 열공참** · **34100 청룡출수** · 34130 질풍참 · 34140 선풍참혼 · 34160 공의연무
스탠스 무관 미바인딩: 34610 적룡질풍격 · 34630 마룡합일섬 · 34650 적룡필살 · 34660 연가비기
기타: 34900 격돌 · 34910 저스트가드 · 34920 엔딩연출

**34060 열공참과 34100 청룡출수가 현재 유일하게 바인딩된 두 스킬인데 양쪽 목록에 없다.** §12-1 참조.

## 5. 바인딩은 잠정이다 — 트라이포드·스킬창 계획의 함의

원작의 트라이포드 시스템과 스킬창 시스템을 구현할 예정이다. 그렇게 되면 스킬창에서 스킬포인트로 얻은 스킬을 퀵슬롯에 올려 바인딩하는 형태가 되므로, **§3·§4는 고정 계약이 아니라 기본 로드아웃이다.**

### 5.1 그래서 `inputSlot`을 스킬 정의에 두면 안 된다

현재 `Data/Balance/PlayerSkills.json`은 `inputSlot`을 스킬 정의 안에 갖고 있다. 이건 두 가지를 섞은 것이다.

| 개념 | 성격 | 있어야 할 곳 |
|---|---|---|
| 스킬 정의 | 34120이 무엇을 하는가 (쿨·타이밍·데미지·사거리) | `Data/Balance/PlayerSkills.json` |
| 로드아웃 | 어떤 스킬이 어느 슬롯에 올라가 있는가 | 별도 문서 → 나중에 플레이어 진행 상태 |

**이 분리가 stance 중복 문제를 동시에 해결한다.** §7에서 보듯 퍼블리셔의 유일성 키가 `"$class:$slot"`이라, 스탠스를 도입하면 `LANCE_MASTER:Q`가 긴 창·짧은 창 두 번 나와 실패한다. 바인딩을 정의에서 빼면 그 충돌 자체가 사라진다 — 스킬 정의는 skillId로만 유일하면 되고, 슬롯 유일성은 로드아웃 문서가 (스탠스, 슬롯) 단위로 검사한다.

즉 지금 `inputSlot`에 stance 축을 억지로 추가하는 것보다, **바인딩을 밖으로 빼는 편이 더 적은 작업이면서 앞으로 갈 방향과도 맞는다.**

### 5.2 트라이포드 데이터는 이미 추출돼 있다

새로 뽑을 게 없다. 지금 데이터가 트라이포드를 이미 담고 있다.

- `.skilltiming`의 **변형 행이 곧 트라이포드**다. `34087`/`34088`은 `일섬각 [34080 변형]`이고 `base=34080`이다. 툴 `SKILL_TIMING`의 주석이 명시한다 — *"iSkillId는 트라이포드 변형이고 iBaseSkillId는 소속 스킬"*.
- `.clipseq`가 스킬당 여러 `seq` 인덱스를 갖는 이유도 트라이포드다. `Character.h`의 `CLIP_CHAIN` 주석 — *"a skill has several chains because each tripod build takes a different route"*.
- `CCharacter::Play_Skill(int32_t iSkillId, int32_t iSeqIndex)`가 **이미 트라이포드 경로 인자를 받는다.** 현재 호출부가 `iSeqIndex`를 넘기지 않아 기본값 0을 쓰고 있을 뿐이다.

빠진 것은 데이터가 아니라 **선택 상태** 둘이다 — 어떤 트라이포드를 골랐는지, 어떤 스킬을 어느 슬롯에 올렸는지. 둘 다 플레이어 진행 상태이므로 최종적으로 서버가 소유한다.

### 5.3 지금 지킬 것

스킬창을 지금 만들지는 않는다. 다만 **되돌리기 비싼 결정만 미리 옳게 잡는다.**

1. `inputSlot`을 스킬 정의에서 분리한다 (§5.1) — **아직 안 함.** 짧은 창을 넣을 때 강제된다
2. 입력 바인딩 코드가 "슬롯 → skillId"를 **테이블에서 읽게** 한다. 하드코딩하지 않는다 (§10) — **완료**
3. `Play_Skill`에 넘기는 `iSeqIndex`를 0으로 박지 말고 트라이포드 선택이 들어올 자리로 남긴다 — 현재 기본값 0을 쓰는 상태 유지
4. 트라이포드 선택 UI·스킬포인트·습득 상태는 이번 범위 밖 — 소비자 없는 인터페이스를 미리 만들지 않는다

## 6. 타이밍 데이터 현황

`Data/Animation/Reference/LanceMaster/LanceMaster.skilltiming` 실측. 파일은 **UTF-8**이므로 PowerShell에서 `[IO.File]::ReadAllLines(path, [Text.Encoding]::UTF8)`로 읽는다. 기본 읽기는 CP949로 깨진다.

- **base 행 있음** — 34070 · 34090 · 34110 · 34120 · 34150 · 34550 · 34560 · 34570 · 34590 · 34620 · 34640
- **변형 행만 있음** — 34080(34087/34088) · 34540(34545~34549) · 34600(34601). `base=`로 색인하면 값을 얻는다.
- **타이밍 없음 — 정상** — 34000 · 34500 스탠스 전환, 34020 탄영 · 34520 돌파. 데미지 스킬이 아니다.
- **타이밍 없음 — 조사 필요 (5)** — 34010 · 34510 평타, 34050 풍진격, 34170 청룡진, 34580 절룡세. 다섯 개 모두 `.skilltiming`에 언급 0건이다.

`.clipseq`에는 있고 `.skilltiming`에는 없는 것들이라 추출 누락 가능성이 높다. 확인 경로:

1. `.animnotify` / `.animevents`의 HIT 행에서 시점을 얻을 수 있는지
2. 원본 `EFTable_SkillEffect.db` 직접 쿼리 (`SourceData/LPK`의 SQLite. 07-30 문서 참조)
3. 평타는 Skill 테이블이 아닌 별도 테이블일 가능성

### 6.1 `PlayerSkills.json` 값을 뽑은 규칙 (짧은 창에도 그대로 쓴다)

**추출값 — 실측이다.**

| 필드 | 출처 |
|---|---|
| `cooldownMs` | `.skilltiming` base 행의 `cd=`. base 행이 없으면 `base=<id>` 변형 행 |
| `hitTimeMs` | 첫 `timed=1` hit 라인의 `t=`. `hits=""`여도 hit 라인에 `t=`가 있다 |
| `actionDurationMs` | `.clipseq` 체인 clip 길이 합(`.animnotify` 헤더의 `len=`). **마지막 히트를 담을 수 있는 가장 짧은 체인**을 고른다 |
| `movementDistance` | `move=` × 0.01 |

`actionDurationMs`의 체인 선택 규칙이 중요하다. `seq`마다 트라이포드가 달라 길이가 다르고, 최저 `seq`를 쓰면 히트가 밖으로 나간다 — 34090 철량추가 히트 1730ms인데 `seq=0` 길이가 1600ms다. `seq=4`(2100ms)를 골라야 맞는다. **`.skilltiming` 변형 ID와 `.clipseq` seq 인덱스를 잇는 매핑이 데이터에 없어서**, 히트를 담는 최단 체인으로 대신한다.

`movementDistance` 환산 계수 0.01은 34100의 `move=200`→`2.0`, 34060의 `move=0`→`0.0`으로 확인했다. 긴 창 9개는 전부 `move=0`이다.

**기준선 — 데이터 출처가 없어 내가 정했다. 튜닝 대상이다.**

| 필드 | 규칙 | 근거 |
|---|---|---|
| `resourceCost` | `5 + 쿨/1000`, 각성기(쿨 ≥ 50000)는 0 | 삭제된 34060(10000→15)·34100(20000→25)을 정확히 재현 |
| `damage` | `400 + 쿨×0.025`, 상한 2500 | 삭제된 650·900을 정확히 재현. 상한은 발탄 HP 10000의 25% |
| `maximumRange` | 첫 timed hit의 `ar=` × 0.033 | 34060의 `ar=240`→8.0에서 역산 |

`maximumRange`는 기존 두 스킬이 서로 다른 비율(0.0333 / 0.0273)로 손으로 정해져 있어서 34060 기준으로 통일했다. **`ar=`를 그대로 쓰면 안 된다** — raw 게임 단위이고 월드 스케일 환산이 정본으로 정해진 바 없다.

주의: 서버 계약 테스트가 player (151,−129) / boss (151,−122), 즉 **거리 7.0**에서 판정을 기대한다. 34120의 `maximumRange`가 7.3이라 여유가 0.3뿐이다. 이 값을 낮추면 `Apply server-authoritative player damage once`가 깨진다.

**추정값을 지어넣지 않는다.** `AGENTS.md`의 silent fallback 금지에 해당한다.

## 7. 퍼블리셔 스키마 제약 — 데이터 추가를 막던 것들

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1:87-108`이 현재 2스킬 상태에 맞춰 하드코딩돼 있다.

| 줄 | 검증 | 막는 것 |
|---|---|---|
| 88-90 | `Assert-ExactProperties` 고정 목록 | `hits[]`·`skillKind` 등 새 필드 → **즉시 실패** |
| 103 | `characterClass -ne 'LANCE_MASTER'` | 나머지 3클래스 전부 |
| 103 | `inputSlot -notin @('Q','W')` | E R A S D F T Z V ALT_V SPACE LMB 전부 |
| 104 | `cooldownMs -eq 0` | 평타·이동기 (쿨 없음) |
| 105 | `hitTimeMs -eq 0` | 스탠스 전환·이동기 (히트 없음) |
| 105 | `hitTimeMs -gt actionDurationMs` | 정당한 검증, 유지 |
| 106 | `damageIds.Contains(serverDamageProfileId)` | 데미지 없는 스킬도 프로필 필수 |
| 100 | `$inputSlots.Add("$class:$slot")` 유일성 | 스탠스 도입 시 중복 실패 — **§5.1로 해소** |

### 7.1 이번에 푼 것

- `inputSlot` 화이트리스트를 `$playerSkillSlots` 변수로 빼서 `Q W E R A S D F T Z V ALT_V SPACE LMB RMB`를 허용
- `characterClass` 검사를 기존 `$supportedPlayerClasses`(4클래스) 재사용으로 교체 — 새 변수를 만들지 않았다
- `hitTimeMs -eq 0` 검증 제거. 일섬각의 `t=0`이 정당한 값이다. `Assert-ExactProperties`가 필드 누락을 따로 잡으므로 안전망은 남는다

### 7.2 서버 파서에 같은 검증이 짝으로 있었다 (놓쳤던 부분)

퍼블리셔만 풀고 실행했더니 `Server.exe --contract-test`의 `Load gameplay balance bootstrap`이 실패했다. 원인은 `Server/Private/GameplayCatalog.cpp`의 SKILL 행 검증에 **똑같은 `0u == skill.iHitTimeMs`가 있었던 것**이다. 상한 검사(`iHitTimeMs > iActionDurationMs`)만 남기고 제거했다.

**교훈: `Data/Balance` 스키마 제약을 바꿀 때는 항상 두 곳을 짝으로 본다.**

| 층 | 파일 |
|---|---|
| authoring 검증 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` |
| runtime 파싱 | `Server/Private/GameplayCatalog.cpp` |

퍼블리셔만 고치면 `Validate`/`Publish`는 통과하는데 서버가 bootstrap을 못 읽는다. 그 실패가 Client 빌드나 퍼블리셔 출력에는 안 나타나고 contract test에서만 드러난다.

### 7.3 남은 것

1. `inputSlot`을 정의에서 제거 (§5.1). 유일성은 skillId만 — 짧은 창에서 강제된다
2. 데미지·히트·쿨이 없는 스킬 종류 표현 — `skillKind`가 `ATTACK`/`STANCE`/`MOVEMENT`, 종류별 필수 필드가 다르게. 평타·이동기·스탠스 전환이 여기 걸려 아직 못 넣었다
3. `Assert-ExactProperties` 목록 갱신 (위 필드 추가 시)

`AGENTS.md`가 새 data 추가 시 harness까지 요구하므로, 퍼블리셔 실패 경로(잘못된 종류, 중복 skillId, 종류별 필수 필드 누락, 존재하지 않는 damage 참조)도 같이 넣는다. 지금은 카탈로그 쪽에만 중복 슬롯/중복 ID/미지의 damage 참조 검사가 있다(`PlayerSkillCatalog.cpp`).

## 8. 08-02 작업 중 통합에서 빠진 것

08-03 통합(PR #36) 시점 실측.

| 항목 | 현재 상태 |
|---|---|
| `CSkillData` (`Client/{Public,Private}/SkillData.*`) | **삭제됨** — 쿨다운·히트시점 로더 |
| `CCharacter::Set_PartVisible` | **삭제됨** |
| `CPart_Equipment::Set_Visible` | **삭제됨** (현재 `Set_Visible` 2건은 MapAssetObject 쪽, 무관) |
| 스펙의 짧은 창 파츠 `Part_91_Weapon_R_Short` | **삭제됨** — 현재 무기 1개뿐 |
| `CLogic_LanceMaster` 스탠스 상태기계 + 바인드 테이블 | **삭제됨** — `Update_Presentation`이 빈 스텁 |
| `WP_WFLM_00S` 짧은 창 에셋 | **살아 있음** — 리소스 팩 `.3`에 존재 |
| `.animnotify`의 `win=` 분류 | 살아 있음 |
| Engine crossfade | 살아 있음 |
| 무기 preTransform identity 통일 | 통합에서 되돌아갔다가 08-03 재수정 (`06b7fa5`) |

통합 문서가 이유를 밝혀뒀다 — *"DirectInput에서 스킬·스탠스를 로컬 재생하는 구현과 소비자가 없는 `SkillData` 런타임은 통합하지 않았다. 스탠스는 Shared command → Server approval → snapshot 계약이 추가된 뒤 활성화한다."* **삭제가 아니라 보류**이고, 지금 그 선행 조건을 만드는 것이 이번 작업이다.

## 9. 스탠스는 서버 권위 상태다

어느 스탠스인지가 **어떤 스킬이 합법인지**를 정한다. 짧은 창일 때 긴 창 스킬 요청은 거절돼야 하고, 그 판단은 서버가 한다 — `PlayerSkillSystem::Try_Start`가 이미 `skill->eCharacterClass != player.eCharacterClass`를 검사하는 것과 같은 층이다.

필요한 것:

1. `Shared`의 `PLAYER_SNAPSHOT`에 현재 스탠스 필드
2. 전환 경로 — 34000/34500을 일반 스킬로 처리하고 승인 시 서버가 스탠스를 뒤집는 방식이 추가가 가장 적다
3. `SERVER_PLAYER`에 스탠스 상태 + `Try_Start`에 스탠스 검사
4. Client는 snapshot의 스탠스를 받아 표현(무기 파츠 가시성)만 바꾼다

**전환 클립이 끝난 뒤에 뒤집힌다.** 08-02 실측 — 34000/34500은 `flm_mode_identity1/2`(무기를 바꾸는 동작)를 재생하므로 캐스팅 시작 시점에 뒤집으면 손과 무기가 어긋난다. 서버도 action 종료 tick에 뒤집어야 표현과 맞는다.

### 클래스별 아이덴티티가 서로 다르다 — 공용 프레임워크를 만들지 않는다

08-02 실측 결론을 보존한다.

| 클래스 | 아이덴티티 | 전환 |
|---|---|---|
| 창술사 | 2스탠스 토글 | 34000 긴→짧, 34500 짧→긴 |
| 건슬링어 | 3스탠스 링 (샷건 / 라이플 / 권총) | 38160 정방향, 38161 역방향 |
| 도화가 | 없음 | — |
| 슬레이어 | 일시 모드(폭주). 전용 평타 45001, 전용 회피 45011 | 45003 발동 |

토글·링·없음·일시모드다. 공용 추상을 만들면 넷 중 어디에도 안 맞는다. 각 `Logic_*`이 자기 상태와 자기 테이블을 갖고, **공유하는 것은 바인드가 아니라 스킬 타이밍 데이터**다.

## 10. 입력 바인딩을 데이터 기반으로

### 10.1 구현 완료 (2026-08-03)

원래 `PlayerController.cpp`가 `if (isQDown) requestedSkillId = 34060;` 식으로 하드코딩이었다. 이제 두 층으로 갈렸다.

| 층 | 위치 | 내용 |
|---|---|---|
| 클래스 무관 | `PlayerController.cpp`의 `SlotKeys` | 슬롯 이름 → 물리 키. `"Q"→DIK_Q`, `"ALT_V"→DIK_V`+Alt, `"SPACE"→DIK_SPACE` |
| 클래스별 | `Data/Balance/PlayerSkills.json`의 `inputSlot` | `(characterClass, slot) → skillId` |

조회는 `CPlayerSkillCatalog::Find_BySlot(pSpec->eCharacterClass, slot.pInputSlot)`이다. **컨트롤러에 클래스 고유 정보가 없으므로 다른 클래스 스킬을 JSON에 추가하면 코드 변경 없이 바인딩된다.**

클래스는 `CHARACTER_SPEC::eCharacterClass`에서 온다. HUD 스냅샷(`CCombatHUDViewModel::Get_Player().eCharacterClass`)을 쓰지 않은 이유는 **Local Preview에 스냅샷이 아예 없어서** 그 경로로는 클래스를 알 수 없기 때문이다.

`ProjectAudit`의 `gameplay.skill-binding-is-data`가 이 계약을 잠근다 — 컨트롤러가 `Find_BySlot`을 쓰고 5자리 skill ID 리터럴을 갖지 않는지 검사한다.

### 10.2 입력 엣지 함정 둘 — 둘 다 실제로 걸렸다

**함정 1 (08-02): 키 눌림 상태를 슬롯 인덱스로 색인하면 안 된다.** 스탠스가 바뀌면 슬롯 i가 다른 키를 가리켜, 전환 직전에 눌려 있던 키가 새 스탠스에서 한 번 안 먹는다. → DirectInput 키 코드로 색인한다.

**함정 2 (08-03): 한 키에 두 슬롯이 걸리면 "읽기 → 판정 → 쓰기"를 분리해야 한다.** `V`와 `ALT_V`가 `DIK_V`를 공유하는데, 루프 안에서 `m_wasKeyDown[키]`를 바로 갱신했더니 앞 항목이 뒤 항목의 엣지를 먹었다.

```text
Alt+V 누른 프레임
  1) "V"     isDown=1 wasDown=0 -> m_wasKeyDown[DIK_V]=1 쓰고, Alt 때문에 skip
  2) "ALT_V" isDown=1 wasDown=1 (1번이 방금 씀) -> 엣지 없음으로 판단, skip
  => 둘 다 발동하지 않는다
```

증상이 "V는 되는데 Alt+V만 안 먹힘"으로 보여서 modifier 판정 문제로 오인하기 쉽다. 해결은 3단 루프다.

1. 모든 슬롯의 현재 눌림 상태를 지역 배열에 수집
2. `m_wasKeyDown`(이전 프레임)과 비교해 발동 결정
3. `m_wasKeyDown` 커밋 — **modifier로 걸러진 슬롯까지 포함해서**

3번을 걸러진 슬롯까지 도는 게 중요하다. V를 누른 채 Alt만 떼면 다음 프레임에 V 항목이 자격을 얻는데, 그때 `wasDown`이 true여야 새 입력으로 오인하지 않는다.

**짧은 창 바인딩에서 이 함정을 다시 만난다.** `Z`가 긴 창에서 34000(짧은 창 전환), 짧은 창에서 34500(긴 창 전환)이다. 스탠스별로 **테이블 2개를 만들고 한 번에 하나만 순회하면** 한 키가 한 슬롯에만 대응하므로 함정 2가 생기지 않는다. 반대로 `requiresAlt`처럼 **stance 필드를 달아 한 테이블에 다 넣으면 `Z`가 `DIK_Z`를 공유하게 되어 같은 버그가 재발한다.** 후자를 고르려면 3단 루프 구조를 반드시 유지한다.

### 10.3 남은 것

- `LMB`/`RMB` 슬롯 — 평타를 좌클릭에 넣을 때. 현재 우클릭이 이동이라 피킹과의 충돌 확인이 필요하다(§13-6)
- 로드아웃 문서 — §5.1대로 슬롯 바인딩을 스킬 정의에서 분리
- 스탠스별 테이블 교체 — §9의 Shared 스탠스 계약이 선행

## 11. 다단 히트와 콜라이더 설계

### 11.1 원본 데이터는 "타격수"를 세 축으로 나눠 갖고 있다

`Animation_Tool.h`의 `HIT_PARAMS` 주석이 명시한다.

```cpp
/* Hits after the first fire every iRepeatMs; the window is the tolerance
of each hit, not a span the hits are spread across. */
int32_t iRepeatCount = { 1 };
int32_t iRepeatMs = {};
...
int32_t iMaxTargets = {};
```

34060 열공참 실측: `hits="1.295-1.363,1.445-1.521" freeze=0.325 multi=0 interval=0 cd=10000`

| 축 | 데이터 필드 | 34060 값 | 의미 |
|---|---|---|---|
| 몇 시점에 | `hits="a,b"` | 2개 (1.295 / 1.445) | 별개의 타격 |
| 한 시점에서 몇 번 | `multi`/`interval` → `iRepeatCount`/`iRepeatMs` | 1회 | 연타 |
| 한 번에 몇 명 | `iMaxTargets` | — | 다중 타겟 |

`HIT_PARAMS`는 **hit별로 따로** 존재한다. 각 hit이 자기 `iAreaType`(0 none / 1 box / 2 fan / 3 ring), `iAreaRange`, `iAreaAngle`, `iAreaHeight`, `iAreaOffsetX`, `iAreaInner`를 갖는다. 34060의 두 타격은 형상이 서로 다를 수 있다.

### 11.2 콜라이더가 가질 것과 갖지 말 것

**가져도 되는 것** — `iRepeatCount` / `iRepeatMs` / `iMaxTargets`. "살아 있는 한 개 판정 볼륨이 어떻게 행동하는가"라서 콜라이더의 속성이 자연스럽다.

**가지면 안 되는 것** — "이 스킬이 몇 번 때리는가". 그건 스킬의 타임라인이고, 별개 hit window 2개는 **콜라이더 2개**다. 하나에 "2회"로 얹으면 150ms 간격과 hit별 형상 차이를 둘 다 잃는다.

모델: `1 hit event = 시점 + 형상 + (repeatCount, repeatMs) + maxTargets`. 콜라이더는 그 hit event의 형상 역할. 34060은 hit event 2개, 각 repeatCount 1.

**주의** — `.animevents`의 HIT는 window(start~end)지만 그 window는 "타격이 68ms 지속"이 아니라 **그 한 번의 판정 허용 오차**다. 68ms 사는 콜라이더가 매 프레임 틱하면 틀린다. `iRepeatCount`가 말할 때만 틱한다.

### 11.3 지금 서버에는 콜라이더가 아예 없다

`Server/Private/PlayerSkillSystem.cpp:142-161` 실측.

```cpp
if (!player.hasAppliedSkillDamage && player.fActionElapsedSeconds >= hitSeconds)
{
    float closestDistanceSquared = skill->fMaximumRange * skill->fMaximumRange;
    for (SERVER_WORLD_ENTITY& entity : worldEntities)
    {
        if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind || ...) continue;
        // XZ 평면 거리만 비교, 가장 가까운 보스 1마리
    }
}
```

- 형상 없음 (반경 거리만) · 단일 타겟 · 보스만 · 캐스트당 1회

`RESULT §14` 미완 항목 6번이 "동적 collider, projectile, knockback/피격 판정의 Server 계약"이다. 콜라이더는 **아직 계약이 없는 영역을 새로 만드는 일**이다.

### 11.4 2단계로 쪼갠다 (시점과 형상은 독립)

**A단계 — 타임라인만.** `hitTimeMs` → `hits[]`(시점 + damage 참조). 반경 판정 유지. `hasAppliedSkillDamage` bool을 "몇 번째 hit까지 적용했는지"로 교체.
→ 서버 변경이 작고, 애니메이션 쪽이 검증 가능해진다. **툴 → Balance JSON → 서버 경로가 여기서 처음 닫힌다.**

**B단계 — 형상.** `iAreaType`, `iMaxTargets`, `iRepeatCount`, 플레이어를 타겟으로 삼기. 여기서 콜라이더가 실체가 된다.

**형상 데이터 위치는 `Data/Balance/PlayerSkills.json`이다.** 서버가 판정하니 서버가 형상을 알아야 하고, Balance JSON이 서버가 읽는 정본이다. `.animevents`의 `HIT_PARAMS`는 저작 원본이고 툴이 거기서 내보낸다.

## 12. 착수 순서

각 단계가 독립적으로 검증 가능하도록 쪼갠다. 한 커밋 = 한 계약.

**완료 (2026-08-03)**

- ~~입력 바인딩 데이터화 (§10)~~ — `CPlayerSkillCatalog` 분리, 슬롯 테이블 기반
- ~~퍼블리셔 슬롯/`hitTimeMs` 일반화 (§7.1)~~ + 서버 파서 짝 맞추기 (§7.2)
- ~~긴 창 스킬 9개 투입 (§4.1)~~ — 34060/34100 제거

**남은 순서**

1. **`hits[]` 도입** (§11.4 A단계) — 34120 연환섬이 3타, 34070 회선창이 2타인데 지금 첫 타만 들어간다. 검증: 발탄에게 실제 타수만큼.
2. **`skillKind` 도입 + `inputSlot` 분리** (§7.3, §5.1) — 평타·이동기·스탠스 전환을 넣기 위한 선행. 짧은 창의 슬롯 중복도 여기서 풀린다.
3. **Shared 스탠스 계약** (§9) — snapshot 필드, `SERVER_PLAYER` 상태, `Try_Start` 검사, 전환은 action 종료 tick.
4. **짧은 창 바인딩** (§4.2) — 스탠스별 테이블. §10.2 함정 2 주의.
5. **표현 재통합** (§8) — 짧은 창 파츠, `Set_PartVisible`, `Logic_LanceMaster` 스탠스 표현. 이때 `Update_Presentation`에 처음 실제 내용이 들어간다.
6. **타이밍 없는 5개 조사** (§6) — 평타 2개, 풍진격, 청룡진, 절룡세.

## 13. 미결정 (착수 전 확인할 것)

1. ~~34060 / 34100 제거~~ — **결정됨.** 테스트용이라 둘 다 제거했다. 검증 참조는 34120으로 옮겼다(`MainApp.cpp` smoke, `ServerGameplayContractTests.cpp`).
2. **타이밍 없는 스킬을 어디서 얻나?** 34010/34510 평타, 34050 풍진격, 34170 청룡진, 34580 절룡세 (§6)
3. **평타·이동기·스탠스 전환을 `PlayerSkills.json`에 넣나?** 쿨·히트·데미지가 전부 없어 현재 필수 필드와 안 맞는다. `skillKind` 도입이 가장 정직하다 (§7.3).
4. **로드아웃 문서를 어디에 두나?** `Data/Balance/`인지 별도 `Data/Loadouts/`인지. 최종적으로 플레이어 진행 상태가 되므로 서버 소유로 갈 자리를 고려한다 (§5.1).
5. ~~Alt+V 수정키 처리 규칙~~ — **구현됨.** `requiresAlt`가 다르면 서로 배타적이라 Alt를 잡으면 V는 안 나간다. 구현 중 걸린 버그는 §10.2 함정 2.
6. **좌클릭 평타와 우클릭 이동의 충돌.** 현재 우클릭이 이동이고 좌클릭은 미사용이다. 평타를 좌클릭에 넣으면 피킹 UI와 겹치는지 확인이 필요하다.
7. **밸런스 기준선 3개(§6.1)를 그대로 갈지.** `resourceCost`·`damage`·`maximumRange`는 데이터 출처가 없어 내가 규칙을 정한 값이다. 특히 각성기 damage 상한 2500(발탄 HP의 25%)과 34120의 `maximumRange` 7.3(계약 테스트 여유 0.3)이 검토 대상이다.
