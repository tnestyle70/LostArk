# 2026-08-05 전 class 스킬 바인딩과 평타 콤보 PLAN

> **2026-08-05 구현 현행화.** 이 문서의 초기 재산정 계획은 실제 반영 뒤 다음 계약으로
> 대체됐다. `PlayerSkills.json`은 53개의 class skill 정의를 갖고 publisher는 COMBO stage를
> 펼친 72 runtime skill row와 54 damage profile을 생성한다. DimensionMaster는 사용자 확정대로
> `Q W E R A S D F T V + LMB(4단)`이며 `Z`와 `ALT_V`, candidate `2050550`은 없다.
> 다섯 class 입력은 모두 Server command → approval → snapshot 경로를 사용한다. Character runtime은
> 더 이상 `.clipseq`를 직접 소비하지 않고
> `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`의 skillId → ordered model clip을
> 소비한다. Animation Tool은 PlayerSkills에서 row를 data-driven으로 열거해 ACTIVE sequence와
> COMBO BA stage를 저장한다. 아래 G00~G05의 이전 기준선 수치와 "런타임 무변경" 문장은 역사적
> 재산정 기록으로만 보존하며 현재 구현 판단에는 이 현행화 절과 대응 RESULT를 우선한다.

현재 종료 증거는 다음과 같다.

| 계약 | 구현/검증 |
|---|---|
| 전 class quick slot + LMB Server 계약 | balance Validate `5 profiles, 72 skills, 54 damage profiles, 1 boss`; Server Debug/Release `failures : 0` |
| key/skill animation Save | strict parse/validate/stage/atomic replace, 5문서 53/53 coverage, ClientFrontendHarness Debug/Release `failures : 0` |
| runtime presentation | missing/corrupt 문서 nonfatal, action-boundary pending reload, Server combo stage direct jump, ACTIVE final pose hold |
| F6 | camera mode만 변경하고 gameplay command gate 없음; ProjectAudit PASS |
| 전체 EXE smoke | Debug/Release Client build PASS. 정확한 locked Resources pack `2026.08.04.1` 부재로 Lobby/Character Select 육안 검증은 BLOCKED |

실패했던 이전 접근의 원인은 Server skill contract와 animation presentation authority를 분리하지 않고
reference `.clipseq`/기존 Q/W 연결만으로 전 class가 동작할 것이라 가정한 데 있다. DimensionMaster에는
Server skill definition 자체가 없었고, 다른 class도 일부 슬롯만 존재했으며, local preview 재생은
Server snapshot의 `skillId/iComboStage`를 대신할 수 없었다. 이번 구현은 먼저 Server 계약을 전 class로
완성하고, 그 stable skillId에 animator-authored model clip 문서를 join하는 순서로 이 문제를 제거한다.

다섯 class 전부에 창술사와 동일한 구조로 quick slot 스킬과 좌클릭 평타 다단 콤보
(`skillKind=COMBO`)를 `Data -> publisher -> Server 판정 -> snapshot -> Client
presentation -> harness` 수직 슬라이스로 연결한다.

- **차원술사(DIMENSIONMASTER)**: `Q W E R A S D F T V` 10슬롯 + 평타 2050010 4단 콤보.
  (본 문서 초판의 `Z` 슬롯은 사용자 지시로 `V`로 교체했고, T=2050510/V=2050540로 재배치했다.)
- **건슬링어(GUNSLINGER)**: 기존 Q/W에 `E R A S D F T V ALT_V` 9슬롯 + 평타 38000 **3단** 콤보 추가.
- **슬레이어(SLAYER)**: 기존 Q/W에 `E R A S D F V ALT_V` 8슬롯 + 평타 45000 4단 콤보 추가.
- **아티스트(ARTIST)**: 기존 Q/W에 `E R A S V ALT_V` 6슬롯 + 평타 31000 4단 콤보 추가.
- **창술사(LANCE_MASTER)**: 9슬롯 + 평타 34010이 이미 닫혀 있으므로 **무변경**.

이 계획의 실측·검증은 **마이그레이션 이전 기준선**에서 수행했다. G01의 JSON 두 개는
**한 번 실제로 적용해 `Publish-GameplayBalance.ps1 -Mode Validate`가
`5 player profiles, 73 skills, 55 damage profiles, 1 bosses`로 통과하는 것을 확인한 뒤
`git checkout`으로 되돌렸다**(그 시점 `git status --short -- Data/Balance`는 적용 전후
모두 clean). G02 이후의 빌드·contract test·audit·런타임 검증은 미실행이다.

> **⚠ 기준선 변경 경고 (2026-08-05 비평 검증 중 확인).** 이 계획서 작성·검증 도중 다른
> 세션이 원작 밸런스 마이그레이션(`2026-08-05_OFFICIAL_BALANCE_COOLDOWN_DAMAGEFONT_PLAN.md`
> 슬라이스)을 worktree에 적용하기 시작했다. 비평 시점 실측: DamageProfiles가
> `formatVersion 2` + `damageRatePercent`로 바뀌고, 자원 풀이 100→1000 경제로 재산정되고
> (기존 Q/W cost 276~521), `CGameplayCatalog::Find_Damage`가 `Find_DamageRatePercent`로
> 개명되는 등 `Data/Balance` 4개, publisher, Server 9개 파일이 미커밋 수정 중이다.
> **이 계획에서 애니메이션 유도 구조 — 슬롯 편성, skillId, comboStages 타이밍, 콤보 창,
> audit 목록, 주석 교정, G00.7 발견 사항 — 는 밸런스 경제와 무관하므로 그대로 유효하다.**
> 반면 경제 수치 — G01의 damage 스키마·resourceCost baseline, G02.1의 `Find_Damage`
> 호출·기대값·자원 상수, 기존 행 인용값 — 는 마이그레이션이 커밋된 뒤 G01.0의 재산정
> 계약으로 다시 끊어야 한다. G01/G02 전문은 검증된 이전 기준선의 정본으로 보존한다.

## 0. 목표와 종료 증거

| 목표 | 종료 증거 |
|---|---|
| 다섯 class 전부의 quick slot ACTIVE 스킬 + LMB COMBO 평타 Server 계약 | `Publish-GameplayBalance.ps1 -Mode Validate` PASS(73 skills/55 damage), `Server.exe --contract-test` `failures : 0` |
| 각 class에서 LMB 꾹 누름 → 평타 단계 연쇄, 창 안 입력 시 다음 단계 연결 | Server + Client 실행에서 class별 평타 연쇄(3단/4단)와 단절 시 idle 복귀 확인 |
| 각 class quick slot 키 입력 → Server 승인 → snapshot → clipseq 클립 재생 + HUD 쿨다운 | 실행에서 class별 전 슬롯 발동·쿨다운 타일 확인 |

이번 작업은 **Client/Server/Shared 런타임 로직을 수정하지 않는다.** 기존 34010 COMBO
슬라이스가 전부 데이터 구동으로 닫혀 있기 때문이다. 변경 파일은 다음이 전부다.

| 파일 | 변경 내용 |
|---|---|
| `Data/Balance/DamageProfiles.json` | damage profile 38개 추가 (G01.1) |
| `Data/Balance/PlayerSkills.json` | 스킬 38개 추가 (G01.2) |
| `Server/Private/ServerGameplayContractTests.cpp` | QUICK_SKILLS 41항목 + damage resolve 4건 + 평타 콤보 블록 4개 (G02.1) |
| `Server/Private/PlayerSkillSystem.cpp` | 사실이 아니게 되는 주석 1건 교정 (G02.2) |
| `Client/Private/PlayerController.cpp` | 최소 콤보 창 수치 주석 2건 교정 224→183 (G02.3) |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | quick-slot·animation 계약 확장 (G03) |
| `AGENTS.md`, `CLAUDE.md`, 팀 사용서 | 닫힌 계약 문장 교체 (G05) |

새 C++ 파일이 없으므로 `.vcxproj`/`.vcxproj.filters` 등록도 없다.

## G00. 실측과 설계 결정

### G00.1 수정하지 않는 파일과 그 근거 (실측)

| 파일 | 근거 |
|---|---|
| `Client/Private/PlayerController.cpp` (주석 2건 제외) | `SlotKeys[]`에 `Q W E R A S D F T Z V ALT_V SPACE` 13항목이 이미 있고 class-불문이다. `Poll_BasicAttack`은 skillId를 하드코딩하지 않고 `CPlayerSkillCatalog::Find_BySlot(pSpec->eCharacterClass, "LMB")`로 조회하며(222-223행) 100ms 간격 재전송을 한다. 이 간격은 전 class 최소 콤보 창(183ms, 아티스트 3단)보다 좁다 |
| `Client/Private/PlayerSkillCatalog.cpp` | `ParseCharacterClass`가 5개 class 전부 파싱. 스킬은 local vector에 stage 후 전부 성공 시에만 `g_Skills = std::move(skills)`로 commit — 손상 문서는 기존 카탈로그 보존 |
| `Client/Private/CombatHUDViewModel.cpp` | `Build_PlayerSkills`가 class 필터 + slot 문자열 사전순 정렬로 카탈로그를 그대로 타일화. COMBO는 "쿨다운이 없으니 타일을 차지하지 않는다" 주석과 함께 제외 |
| `Client/Private/Character.cpp` | `Load_ClipChains`가 `CProjectDataRoot::Resolve("Animation/Reference/<pAssetName>/<pAssetName>.clipseq")`를 읽는다. `Play_Skill(skillId)`이 체인을 시작하고 `Advance_ComboStage`가 서버 `iComboStage`로만 단계를 넘긴다 |
| `Client/Private/Logic_*.cpp` | 다섯 Spec의 `pAssetName`이 `"LanceMaster" "GunSlinger" "Slayer" "Artist" "DimensionMaster"`로 `Data/Animation/Reference/` 디렉터리명과 대소문자까지 일치 |
| `Server/Private/PlayerSkillSystem.cpp` (주석 1건 제외) | `Try_Start`의 class 가드(`skill->eCharacterClass != player.eCharacterClass`), 콤보 창 버퍼링, `hasAppliedSkillDamage` 게이트 후 cancel 전진, 쿨다운 0 허용까지 전부 데이터 구동 |
| `Server/Private/GameplayCatalog.cpp` | bootstrap `SKILL`(13필드)/`SKILLSTAGE` 행 파서와 `ParseCharacterClass`의 `DIMENSIONMASTER` 분기가 이미 있다 |
| `Shared/Public/Network/*` | `C2S_USE_SKILL`, `PLAYER_SNAPSHOT::iComboStage`, `PLAYER_SKILL_KIND`가 class-불문 계약. worktree는 08-04 슬라이스의 미커밋 v7 bump + `WORLD_ID::CHARACTER_SELECT_ARENA` 상태(HEAD는 v6). `NetworkProtocolHarness` 변경 없음. `MAX_PLAYER_COOLDOWNS = 8`(PacketMessages.h:155) 절단 한계는 7장 참조 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `$supportedPlayerClasses`에 5개 class, `$playerSkillSlots`에 `Q W E R A S D F T Z V ALT_V SPACE LMB RMB` 전부 존재. COMBO 2..8단, 비최종 stage `open < close <= duration`, 최종 stage `0/0`, `resourceCost <= 100`, `hitTimeMs <= actionDurationMs` 검증 존재 |

호출 흐름(이번 슬라이스가 데이터로 연결하는 기존 경로):

```text
LMB hold / quick slot press
→ CPlayerController::Poll_BasicAttack(100ms 재전송) / Poll_SkillSlots
→ IPlayerCommandSink::Request_UseSkill → C2S_USE_SKILL
→ CGameRoom::Handle_UseSkill → CPlayerSkillSystem::Try_Start
   (액션 중이면: 같은 COMBO 스킬 + 창 안 press만 hasBufferedComboInput 버퍼)
→ CPlayerSkillSystem::Update
   (hit 적용 후 버퍼가 있으면 cancel 전진: ++iComboStage, iActionStartTick 갱신)
→ S2C_WORLD_SNAPSHOT(PLAYER_SNAPSHOT::iComboStage)
→ CClientReplication → CCharacter::Apply_NetworkAction
   (stage 1: Play_Skill(평타 ID) → clipseq COMBO 체인 시작,
    stage n>1: Advance_ComboStage(n) → clips[n-1] 재생,
    action NONE 엣지: idle/run 복귀)
```

### G00.2 수치 유도 규칙

창술사 34010과 차원술사 표 전체를 독립 재유도로 역검증한 기존 규칙에, 이번 실측에서
필요해진 확장 규칙 세 개(별표)를 더한다. 반올림은 전부 half-up이다(2050150 range
8.25→8.3, 2050220 21.45→21.5가 이 규칙으로만 재현된다).

| 필드 | 유도 규칙 (출처 파일) |
|---|---|
| `cooldownMs` | `.skilltiming` 자기 행 `cd=`. 자기 행이 없으면 같은 `base=` 가족의 **가장 낮은 변형 skillId 행**의 cd |
| `hitTimeMs` (ACTIVE) | `.skilltiming` `hits=` 첫 구간 시작 ×1000 |
| `actionDurationMs` (ACTIVE) | `.clipseq` 최저 seq 체인 clip들의 `.animnotify` `len=` 합 ×1000 |
| combo stage `actionDurationMs` | 해당 clip `len=` ×1000, 단 `강제종료`(win=OTHER) notify가 있으면 그 t (차원술사 1단 4000→1400이 유일 사례) |
| combo stage `hitTimeMs` | 해당 clip HIT notify `t=` ×1000 반올림. **★HIT notify가 없는 stage는 전 class 공용 임팩트 라이트 `FX_CM_02.Light.Par_MP_Light_01`의 첫 `t` ×1000을 쓴다** (건슬링어 1단에서 실제 HIT 121ms와 라이트 136ms가 근접해 교차 검증됨. 건슬링어 2단, 슬레이어 4단 전부, 아티스트 4단 전부가 이 규칙 대상) |
| combo stage `inputOpen/CloseMs` | `[선콤]`/`콤보선입력`(win=COMBO_PRE) 창 `t`/`t+d` ×1000. COMBO_PRE가 없으면 무라벨 `src=InputTiming` 창 (차원술사 3단 200~1067, 아티스트 3단 1150~1333) |
| `maximumRange` (ACTIVE) | `.skilltiming` timed(`timed=1`) hit 행 `ar=` ×0.033 소수 1자리 half-up 반올림. **★첫 timed 행이 `ar=0` 유틸 행이면 `ar>0`인 첫 timed 행을 쓴다** (건슬링어 38050/38100/38120/38140/38180 등) |
| `maximumRange` (COMBO 평타) | 첫 hit 행(1단 진입 타격) `ar=` ×0.033. **★skilltiming에 평타 행이 아예 없으면 창술사 평타 3.5를 팀 선택값으로 쓴다** (슬레이어 45000이 유일 — skilltiming이 45050부터 시작) |
| `movementDistance` | `.skilltiming` `move=`. 단 45070(move=300)은 단위 환산 계약이 없어 창술사 선례(전부 0.0)대로 0.0을 저장하고 이동 보정은 7장 범위 밖 |
| `resourceCost`, damage | 원작 참조 파일에 없다. 팀 baseline: 신규 바인딩 중 cd<50s는 cd 오름차순 15/17/19/21/23/25, cd≥50s·각성기·평타는 0. damage는 class별 cd 오름차순 800~1400 밴드, T tier 2000, V/ALT_V 2500, 평타 100. 전부 밸런스 튜닝 대상 |
| 변형 행 선택 | 같은 `base=` 가족에 변형 행이 여럿이고 값이 갈리면 가장 낮은 변형 skillId 행 (45620: 45621 hit=130 채택, 45622 hit=1500 미채택) |

skillId는 반드시 `.clipseq`의 스킬 ID를 그대로 쓴다. `Play_Skill`이 snapshot의 skillId로
체인을 조회하므로 `.skilltiming`의 변형 ID를 저장하면 클립이 재생되지 않는다.

### G00.3 슬롯 편성 규칙과 class별 확정 표

편성 규칙(창술사 관례의 명문화): 기존 Q/W 계약은 불변. 신규 유효 후보를 **cd 오름차순
(동률은 낮은 skillId 먼저)으로 `E R A S D F`에 채우고**, cd 50s~120s의 `초각성 스킬`
tier는 `T`, cd 300000의 각성기 tier는 `V`, 초각성기 tier는 `ALT_V`에 둔다(창술사
T=34640 50s / V=34600 300s / ALT_V=34620 300s와 동일 구조). tier에 후보가 여럿이면
낮은 skillId를 채택하고 나머지는 7장에 기록한다.

**GUNSLINGER** (신규 9 ACTIVE + LMB 3단; cd 오름차순 상위가 전부 핸드건 계열이라 평타
무기와 시각적으로 일치):

| slot | skillId | displayName | cd | dur | hit | range | cost | damage |
|---|---|---|---:|---:|---:|---:|---:|---:|
| Q | 38020 | 퀵 스텝 (기존) | 10000 | 3533 | 1000 | 5.8 | 15 | 600 |
| W | 38050 | 심판의 시간 (기존) | 30000 | 1767 | 1000 | 4.0 | 25 | 800 |
| E | 38120 | 류탄 | 6000 | 1200 | 1000 | 4.0 | 15 | 800 |
| R | 38200 | 민첩한 사격 | 6000 | 3333 | 1000 | 8.2 | 17 | 850 |
| A | 38140 | 플라즈마 불릿 | 8000 | 1233 | 1000 | 3.0 | 19 | 900 |
| S | 38180 | 나선의추적자 | 8000 | 1000 | 1000 | 5.6 | 21 | 950 |
| D | 38210 | 썸머솔트샷 | 9000 | 1600 | 1000 | 3.3 | 23 | 1000 |
| F | 38260 | 피스키퍼 | 12000 | 6267 | 1000 | 15.8 | 25 | 1100 |
| T | 38290 | 프리즌 불릿 | 50000 | 3267 | 1730 | 17.2 | 0 | 2000 |
| V | 38250 | 황혼의 눈 | 300000 | 3600 | 1746 | 20.1 | 0 | 2500 |
| ALT_V | 38320 | 데드 엔드 | 300000 | 6467 | 1451 | 33.0 | 0 | 2500 |
| LMB | 38000 | 핸드건 평타 | 0 | 1000 | 121 | 14.5 | 0 | 100 |

**SLAYER** (신규 8 ACTIVE + LMB 4단; 50s tier 후보가 없어 T는 비운다 — 45750/45760이
그 tier지만 최저 seq가 COMBO라 제외):

| slot | skillId | displayName | cd | dur | hit | range | cost | damage |
|---|---|---|---:|---:|---:|---:|---:|---:|
| Q | 45050 | 퓨리 블레이드 (기존) | 12000 | 4533 | 1100 | 3.5 | 15 | 750 |
| W | 45060 | 와일드 러시 (기존) | 14000 | 4600 | 500 | 5.0 | 18 | 650 |
| E | 45620 | 그라운드 스매시 | 15000 | 3933 | 130 | 6.3 | 15 | 800 |
| R | 45210 | 플라잉 스트라이크 | 16000 | 2200 | 500 | 8.6 | 17 | 900 |
| A | 45300 | 페이탈 소드 | 22000 | 2667 | 500 | 6.6 | 19 | 1000 |
| S | 45070 | 허리케인 소드 | 24000 | 7000 | 1600 | 7.3 | 21 | 1100 |
| D | 45190 | 파이널 블로 | 24000 | 2433 | 1250 | 5.9 | 23 | 1200 |
| F | 45600 | 마운틴 클리브 | 27000 | 2533 | 1000 | 16.5 | 25 | 1300 |
| V | 45810 | 라그나 브레이크 | 300000 | 4000 | 2200 | 29.7 | 0 | 2500 |
| ALT_V | 45820 | 레이지 슬래셔 | 300000 | 13000 | 1500 | 16.5 | 0 | 2500 |
| LMB | 45000 | 버서커 평타 | 0 | 1800 | 360 | 3.5 | 0 | 100 |

**ARTIST** (신규 6 ACTIVE + LMB 4단; 유효 후보가 6개뿐이라 D F T는 비운다):

| slot | skillId | displayName | cd | dur | hit | range | cost | damage |
|---|---|---|---:|---:|---:|---:|---:|---:|
| Q | 31210 | 필법 : 콩콩이 (기존) | 16000 | 5334 | 1445 | 2.5 | 18 | 550 |
| W | 31230 | 묵법 : 옹달샘 (기존) | 24000 | 1500 | 1445 | 6.5 | 22 | 700 |
| E | 31510 | 필법 : 올려치기 | 16000 | 3167 | 1445 | 8.2 | 15 | 800 |
| R | 31470 | 필법 : 한획긋기 | 25000 | 2833 | 1657 | 19.8 | 17 | 1000 |
| A | 31410 | 묵법 : 해우물 | 30000 | 1833 | 1657 | 10.7 | 19 | 1200 |
| S | 31490 | 묵법 : 범가르기 | 30000 | 1967 | 1654 | 5.0 | 21 | 1400 |
| V | 31900 | 절기 : 진경산수 | 300000 | 5300 | 1876 | 13.2 | 0 | 2500 |
| ALT_V | 31920 | 신수도 : 봉황 | 300000 | 5333 | 1654 | 16.5 | 0 | 2500 |
| LMB | 31000 | 평타 | 0 | 1233 | 276 | 7.6 | 0 | 100 |

**DIMENSIONMASTER** (11 ACTIVE + LMB 4단; 표 전체를 독립 재유도로 재검증해 전 셀 일치.
초판 대비 변경: Z 슬롯 폐기, T=2050510(120s tier)/V=2050540(300s 각성 tier) 재배치,
ALT_V=2050550 추가 — 2050540/2050550 모두 `[초각성기]` cd 300000으로 창술사 V/ALT_V
구조와 동일):

| slot | skillId | displayName | cd | dur | hit | range | cost | damage |
|---|---|---|---:|---:|---:|---:|---:|---:|
| Q | 2050110 | 예고 | 16000 | 2100 | 1451 | 5.0 | 15 | 800 |
| W | 2050150 | 공간 베기 | 25000 | 1933 | 1451 | 8.3 | 19 | 1000 |
| E | 2050220 | 일점 관통 | 40000 | 1667 | 1451 | 21.5 | 23 | 1300 |
| R | 2050190 | 진공 | 48000 | 2400 | 1789 | 13.2 | 27 | 1450 |
| A | 2050240 | 경계 돌파 | 50000 | 2000 | 1451 | 16.5 | 0 | 1500 |
| S | 2050210 | 분광 | 55000 | 2767 | 1451 | 9.9 | 0 | 1550 |
| D | 2050200 | 공간 절단 | 60000 | 2633 | 1639 | 13.2 | 0 | 1650 |
| F | 2050500 | 업의 경계 | 120000 | 4367 | 1730 | 9.9 | 0 | 2000 |
| T | 2050510 | 일념 | 120000 | 3000 | 1730 | 33.0 | 0 | 2000 |
| V | 2050540 | 무간의 옥 | 300000 | 5100 | 1730 | 16.5 | 0 | 2500 |
| ALT_V | 2050550 | 찰나 | 300000 | 5000 | 1730 | 39.6 | 0 | 2500 |
| LMB | 2050010 | 기본 공격 | 0 | 1400 | 100 | 7.3 | 0 | 100 |

차원술사 cd/hit의 변형 행 출처(재검증 완료): 2050153(W), 2050221(E), 2050194(R),
2050501(F), 평타 cd/ar은 2050001(`base=2050000`, 1타 ar=220→7.3). 2050550은 자기 행
(cd=300000, hits=1.73, timed ar=1200)에서 직접 유도. 신규 55개 skillId 전부가 각
`.clipseq` 행과 `.clipmap` `skill=<id>` 항목 양쪽에 존재함을 스크립트로 확인했다.

### G00.4 평타 comboStages 유도 — class별 실측 표

`{ dur, hit, open, close }`는 ×1000 half-up 반올림 값이다. hit의 `(라이트)`는 HIT notify
부재로 G00.2의 임팩트 라이트 규칙을 적용했다는 뜻이다.

**GUNSLINGER 38000** — `gdh_att_battle_1_01..03` 3단:

| stage | clip | len | HIT | COMBO_PRE | 유도 결과 |
|---|---|---:|---|---|---|
| 1 | _01 | 1.0000 | 0.1212 | 0.0746+0.2849 | { 1000, 121, 75, 360 } |
| 2 | _02 | 1.6000 | 없음 → 라이트 0.2600 | 0.3712+0.3472 | { 1600, 260, 371, 718 } |
| 3 | _03 | 1.7667 | 0.7613 | 없음 (최종) | { 1767, 761, 0, 0 } |

**SLAYER 45000** — `wbk_att_battle_2_01..04` 4단 (전 stage HIT notify 부재 → 라이트 규칙):

| stage | clip | len | 라이트 | COMBO_PRE | 유도 결과 |
|---|---|---:|---:|---|---|
| 1 | _01 | 1.8000 | 0.3600 | 0.2+0.27 | { 1800, 360, 200, 470 } |
| 2 | _02 | 1.6000 | 0.2776 | 0.2+0.22 | { 1600, 278, 200, 420 } |
| 3 | _03 | 1.5667 | 0.4293 | 0.2+0.40 | { 1567, 429, 200, 600 } |
| 4 | _04 | 1.8667 | 0.5005 | 없음 (최종) | { 1867, 501, 0, 0 } |

**ARTIST 31000** — clipseq 문서 순서가 `_03, _02, _01, _04`다. **정렬하지 않고 이
순서가 stage 순서다** (전 stage HIT notify 부재 → 라이트 규칙):

| stage | clip | len | 라이트 | 입력 창 | 유도 결과 |
|---|---|---:|---:|---|---|
| 1 | _03 | 1.2333 | 0.2755 | COMBO_PRE 0.1567+0.2777 | { 1233, 276, 157, 434 } |
| 2 | _02 | 1.2333 | 0.2691 | COMBO_PRE 0.2+0.34 | { 1233, 269, 200, 540 } |
| 3 | _01 | 1.3333 | 0.4939 | 무라벨 InputTiming 1.15+0.1833 | { 1333, 494, 1150, 1333 } |
| 4 | _04 | 1.2667 | 0.1927 | 없음 (최종) | { 1267, 193, 0, 0 } |

**DIMENSIONMASTER 2050010** — `pc_sp_m_00_sk_att_battle_1_01..04` 4단 (초판 표 그대로,
독립 재유도로 재검증 완료):

| stage | clip | len | 강제종료 | HIT t | COMBO_PRE | 유도 결과 |
|---|---|---:|---:|---:|---|---|
| 1 | _01 | 4.0000 | 1.400 | 0.1002 | 0.100+0.410 | { 1400, 100, 100, 510 } |
| 2 | _02 | 1.5000 | 없음 | 0.0425 | 0.000+0.410 | { 1500, 43, 0, 410 } |
| 3 | _03 | 1.0667 | 없음 | 0.0282 | 무라벨 0.200+0.8667 | { 1067, 28, 200, 1067 } |
| 4 | _04 | 1.7000 | 없음 | 0.0328/0.3350 | 없음 (최종) | { 1700, 335, 0, 0 } |

유도에서 내린 결정:

1. 차원술사 1단 duration은 len 4.0이 아니라 `강제종료`(win=OTHER, t=1.400)를 쓴다.
   1.4초 이후는 idle 대기 꼬리다. 다른 세 class 평타에는 `강제종료` notify가 없어
   전부 len을 그대로 쓴다.
2. 차원술사 4단은 HIT가 두 개(0.0328 선행 베기, 0.3350 내려찍기)다. 사운드·이펙트
   클러스터가 몰린 주 타격 0.3350을 채택한다(재검증에서 클러스터 위치로 재확인).
3. hit이 입력 창보다 앞서는 stage(차원술사 3단 28<200, 건슬링어 2단 260<371,
   아티스트 3단 494<1150)는 `hasAppliedSkillDamage` 게이트가 damage 유실을 막는다.
   G02.2의 주석 교정이 이 사실을 코드에 반영한다.
4. 전 class 콤보 창 최소 폭: 창술사 224ms(2단), 건슬링어 285ms(1단), 슬레이어
   220ms(2단), 아티스트 **183ms(3단)**, 차원술사 410ms. 전부 Client 재전송 간격
   100ms보다 넓어 hold 입력이 단계를 놓치지 않는다. 최솟값이 224→183으로 바뀌므로
   `PlayerController.cpp` 주석 2건을 G02.3에서 교정한다.

### G00.5 선정 제외 목록 (전부 실측)

**공통 계약 제외**: 회피기·기상기(각 class x0010~x0030 대역), 격돌(맞대결)(x0900/31700),
저스트가드(x0910/31710), 엔딩연출(x0920/31720), 스탠스 전환(38160/38161), 이동기(38150).
이동기·스탠스 전환은 AGENTS.md 고정 계약대로 어느 kind에도 넣지 않는다.

**최저 seq가 mode=COMBO인 다단 입력 스킬** (평타와 같은 구조의 별도 수직 슬라이스):
차원술사 2050120/2050160/2050170/2050250/2050530, 건슬링어 38002/38040/38070/38190/
38240/38340, 슬레이어 45001(폭주 평타)/45700/45710/45720/45730/45740/45750/45760/45800,
아티스트 31450/31500.

**mode=HOLD 차지·홀드형** (재생 계약 미정): 건슬링어 38060/38080/38270, 슬레이어
45080, 아티스트 31150.

**cd 유도 불가** (skilltiming에 자기 행·base 가족 변형 행 모두 없음): 차원술사
2050100/2050130/2050140/2050180/2050230, 슬레이어 45003/45004/45100/45220, 아티스트
31050/31051/31121/31131/31141/31145/31200/31220/31400/31420/31430/31910/31940/31950과
`_Test` 변형 전부.

**개별 사유**:

- `2050520 시간의 굴레` — 첫 hit 4130ms > clip 2533ms. 클립 루프 재생 계약이 먼저 필요.
- `45610 스피닝 소드` — hit 2100ms > dur 1900ms. publisher 제약 위반.
- `45110 플래시 블레이드` — `hits=""` 반복 다중타(rep/repms) 구조라 단일 hitTime 유도 불가.
- `31460 호접몽`, `31480 두루미나래` — 같은 `hits=""` 다중타 구조.
- `38110 마탄의사수` — timed hit 행이 전부 ar=0(발사체)이라 range 규칙 미정.
- `31440 먹오름`, `31060 아크패시브 파죽`, `31489 두루미나래_Test` — clipmap에 자기
  skillId 항목이 없음(clip이 다른 스킬로 매핑). clipseq+clipmap 양쪽 존재 조건 미충족.
- `31110/31060 아이덴티티·아크패시브` — cd 1~2s로 자원 게이지 체계 전제. 일반 quick
  slot 계약이 아님.
- `45061` — 45060과 같은 clip 체인의 변형 ID. clipmap이 전부 45060으로 매핑.
- 슬롯 초과 잔여 유효 후보(후속 슬롯 확장 1순위): 건슬링어 38220 이퀄리브리엄(16s),
  38100 메테오스트림(20s), 38090 데스파이어(24s), 38170 샷건연사(36s), 38230
  최후의만찬(36s), 38300 세븐 샷건(60s), 38310 불스 아이(70s), 38280 폭발 샷건(각성),
  38330 아토믹 익스플로전(초각성), 슬레이어 45830 라그나 블레이드(초각성), 아티스트
  31930 몽중백화원(초각성).

### G00.6 불변식

1. `skillId`는 clipseq 스킬 ID와 동일하다. 변형 ID·별칭 ID를 저장하지 않는다.
2. COMBO `comboStages` 개수 == clipseq 해당 스킬 COMBO 체인 clip 개수
   (건슬링어 3==3, 나머지 4==4). stage 순서는 clipseq `clips=` 문서 순서다.
3. ACTIVE 스킬의 clipseq 최저 seq 체인 mode는 SEQUENCE 또는 ONESHOT이다.
4. `hitTimeMs ≤ actionDurationMs`, 비최종 stage `inputOpen < inputClose ≤ duration`,
   최종 stage `0/0` — publisher가 강제하고 G01 Validate 실행으로 이미 확인했다.
5. 콤보 단계 정본은 `SNAPSHOT_PLAYER::iComboStage`다. Client는 단계를 세지 않는다.
6. 기존 Q/W 15행과 창술사 전체 바인딩은 값 하나도 바꾸지 않는다.
7. V는 각성기 tier(cd 300000), ALT_V는 초각성기 tier, T는 50s~120s `초각성 스킬` tier다.
8. `effectId`는 전부 빈 문자열 유지 — candidate-only effect는 이번 슬라이스에서
   활성화하지 않는다.

### G00.7 실측 중 발견한 기존 상태 (이번 변경에서 손대지 않음)

1. **아티스트 Q 31210의 최저 seq(3) mode가 COMBO다.** 현재 ACTIVE로 바인딩되어 있어
   `Play_Skill`의 lowest-chain fallback이 COMBO 체인을 시작하면 서버가 ACTIVE에는
   `iComboStage`를 보내지 않으므로 첫 clip(1.867s) 이후 5.334s 액션이 끝날 때까지
   표현이 정지할 수 있다. 닫힌 계약이므로 보고만 남긴다(별도 슬라이스 후보).
2. 기존 Q/W의 `maximumRange` 5건(38020 5.8, 38050 4.0, 45050 3.5, 45060 5.0, 31210
   2.5, 31230 6.5)은 이번 유도 규칙(ar×0.033)으로 재현되지 않는 팀 선택값이다.
   cd/dur/hit/move는 전부 유도값과 일치했다. 기존 값을 유지한다.

### G00.8 종료 판정

G00은 문서 장이므로 코드 변경이 없다. G01~G05의 실측 근거가 본 장과 다르게 판명되면
구현을 멈추고 이 표부터 교정한다.

## G01. Data — 전 class balance 정의

### G01.0 원작 밸런스 마이그레이션 위 재산정 계약

마이그레이션이 커밋된 뒤 G01.1/G01.2를 적용할 때 아래 필드만 그 시점 스키마로 변환한다.
슬롯·skillId·displayName·actionId·skillKind·comboStages·actionDurationMs·hitTimeMs는
애니메이션 유도값이므로 그대로 쓴다.

| 필드 | 이전 기준선 (본문 전문) | 마이그레이션 후 재산정 규칙 |
|---|---|---|
| damage 스키마 | `formatVersion 1`, `amount` | `formatVersion 2`, `damageRatePercent`. 신규 38개 rate는 OFFICIAL_BALANCE 계획 G00의 join(`EFTable_SkillEffect.PrimaryKey = skillId*10+0`, `SecondaryKey=10`(레벨), `ValueA`)으로 유도. 평타 4종은 창술사 34010 선례(rate 100) |
| `resourceCost` | 15~27 / 0 baseline | `EFTable_Skill.CostMp`(레벨 10), 풀 1000 경제. 각성기·평타 0 유지 |
| `cooldownMs` | `.skilltiming` `cd=` | 원칙적으로 동일 출처지만 `EFTable_Skill.Cooltime`(레벨 10)과 대조해 다르면 EFTable 값 채택 |
| `maximumRange` | `ar × 0.033` | 마이그레이션이 기존 행 range를 재산정했으므로 그 규칙(`EFTable_Skill.MaxRange` 기반)과 대조해 같은 규칙으로 통일 |
| G02.1 damage resolve | `Find_Damage` == 절대값 | `Find_DamageRatePercent` == rate 값으로 이름·기대값 교체 |
| G02.1 자원 상수 | `iCurrentResource = 100` | 마이그레이션 후 풀 값(실측 1000)으로 교체 — 기존 Q/W cost 276~521이 100 풀에서는 전부 거부된다 |

기존 15행(창술사 9 + 네 class Q/W 6 + 34010)의 값은 마이그레이션 세션이 소유한다.
이 계획은 그 행을 읽기만 하고 절대 덮어쓰지 않는다.

### G01.1 `Data/Balance/DamageProfiles.json` — 적용 후 전문 (이전 기준선 정본)

38개 profile을 추가한 파일 전체다. 그대로 교체한다.

```json
{
  "schema": "lostark.damage-profiles",
  "formatVersion": 1,
  "profiles": [
    { "damageProfileId": "damage.player.34120", "amount": 650 },
    { "damageProfileId": "damage.player.34080", "amount": 700 },
    { "damageProfileId": "damage.player.34070", "amount": 750 },
    { "damageProfileId": "damage.player.34150", "amount": 1000 },
    { "damageProfileId": "damage.player.34110", "amount": 850 },
    { "damageProfileId": "damage.player.34090", "amount": 650 },
    { "damageProfileId": "damage.player.34640", "amount": 1650 },
    { "damageProfileId": "damage.player.34600", "amount": 2500 },
    { "damageProfileId": "damage.player.34620", "amount": 2500 },
    { "damageProfileId": "damage.player.38020", "amount": 600 },
    { "damageProfileId": "damage.player.38050", "amount": 800 },
    { "damageProfileId": "damage.player.45050", "amount": 750 },
    { "damageProfileId": "damage.player.45060", "amount": 650 },
    { "damageProfileId": "damage.player.31210", "amount": 550 },
    { "damageProfileId": "damage.player.31230", "amount": 700 },
    { "damageProfileId": "damage.valtan.basic-swing", "amount": 350 },
    { "damageProfileId": "damage.player.34010", "amount": 100 },
    { "damageProfileId": "damage.player.38120", "amount": 800 },
    { "damageProfileId": "damage.player.38200", "amount": 850 },
    { "damageProfileId": "damage.player.38140", "amount": 900 },
    { "damageProfileId": "damage.player.38180", "amount": 950 },
    { "damageProfileId": "damage.player.38210", "amount": 1000 },
    { "damageProfileId": "damage.player.38260", "amount": 1100 },
    { "damageProfileId": "damage.player.38290", "amount": 2000 },
    { "damageProfileId": "damage.player.38250", "amount": 2500 },
    { "damageProfileId": "damage.player.38320", "amount": 2500 },
    { "damageProfileId": "damage.player.38000", "amount": 100 },
    { "damageProfileId": "damage.player.45620", "amount": 800 },
    { "damageProfileId": "damage.player.45210", "amount": 900 },
    { "damageProfileId": "damage.player.45300", "amount": 1000 },
    { "damageProfileId": "damage.player.45070", "amount": 1100 },
    { "damageProfileId": "damage.player.45190", "amount": 1200 },
    { "damageProfileId": "damage.player.45600", "amount": 1300 },
    { "damageProfileId": "damage.player.45810", "amount": 2500 },
    { "damageProfileId": "damage.player.45820", "amount": 2500 },
    { "damageProfileId": "damage.player.45000", "amount": 100 },
    { "damageProfileId": "damage.player.31510", "amount": 800 },
    { "damageProfileId": "damage.player.31470", "amount": 1000 },
    { "damageProfileId": "damage.player.31410", "amount": 1200 },
    { "damageProfileId": "damage.player.31490", "amount": 1400 },
    { "damageProfileId": "damage.player.31900", "amount": 2500 },
    { "damageProfileId": "damage.player.31920", "amount": 2500 },
    { "damageProfileId": "damage.player.31000", "amount": 100 },
    { "damageProfileId": "damage.player.2050110", "amount": 800 },
    { "damageProfileId": "damage.player.2050150", "amount": 1000 },
    { "damageProfileId": "damage.player.2050220", "amount": 1300 },
    { "damageProfileId": "damage.player.2050190", "amount": 1450 },
    { "damageProfileId": "damage.player.2050240", "amount": 1500 },
    { "damageProfileId": "damage.player.2050210", "amount": 1550 },
    { "damageProfileId": "damage.player.2050200", "amount": 1650 },
    { "damageProfileId": "damage.player.2050500", "amount": 2000 },
    { "damageProfileId": "damage.player.2050510", "amount": 2000 },
    { "damageProfileId": "damage.player.2050540", "amount": 2500 },
    { "damageProfileId": "damage.player.2050550", "amount": 2500 },
    { "damageProfileId": "damage.player.2050010", "amount": 100 }
  ]
}
```

### G01.2 `Data/Balance/PlayerSkills.json` — 삽입 블록과 정확한 위치

현재 파일의 마지막 항목은 34010(LMB COMBO)이며 다음으로 끝난다.

```json
      "comboStages": [
        { "actionDurationMs": 1633, "hitTimeMs": 470, "inputOpenMs": 329, "inputCloseMs": 658 },
        { "actionDurationMs": 1367, "hitTimeMs": 356, "inputOpenMs": 330, "inputCloseMs": 554 },
        { "actionDurationMs": 1533, "hitTimeMs": 451, "inputOpenMs": 396, "inputCloseMs": 752 },
        { "actionDurationMs": 1567, "hitTimeMs": 500, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    }
  ]
}
```

위 조각의 `    }`(34010 객체 닫기)와 `  ]` 사이에 아래 38개 항목을 삽입한다. 34010 객체
닫기 뒤에 쉼표가 추가되는 것에 주의한다. 순서는 건슬링어 10 → 슬레이어 9 → 아티스트 7 →
차원술사 12다. 교체 후 34010의 `comboStages` 이후 파일 끝까지 전체는 다음과 같다.

```json
      "comboStages": [
        { "actionDurationMs": 1633, "hitTimeMs": 470, "inputOpenMs": 329, "inputCloseMs": 658 },
        { "actionDurationMs": 1367, "hitTimeMs": 356, "inputOpenMs": 330, "inputCloseMs": 554 },
        { "actionDurationMs": 1533, "hitTimeMs": 451, "inputOpenMs": 396, "inputCloseMs": 752 },
        { "actionDurationMs": 1567, "hitTimeMs": 500, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    },
    {
      "skillId": 38120,
      "characterClass": "GUNSLINGER",
      "inputSlot": "E",
      "displayName": "류탄",
      "actionId": "gunslinger.skill.38120",
      "skillKind": "ACTIVE",
      "cooldownMs": 6000,
      "actionDurationMs": 1200,
      "hitTimeMs": 1000,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 4.0,
      "serverDamageProfileId": "damage.player.38120",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38200,
      "characterClass": "GUNSLINGER",
      "inputSlot": "R",
      "displayName": "민첩한 사격",
      "actionId": "gunslinger.skill.38200",
      "skillKind": "ACTIVE",
      "cooldownMs": 6000,
      "actionDurationMs": 3333,
      "hitTimeMs": 1000,
      "resourceCost": 17,
      "movementDistance": 0.0,
      "maximumRange": 8.2,
      "serverDamageProfileId": "damage.player.38200",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38140,
      "characterClass": "GUNSLINGER",
      "inputSlot": "A",
      "displayName": "플라즈마 불릿",
      "actionId": "gunslinger.skill.38140",
      "skillKind": "ACTIVE",
      "cooldownMs": 8000,
      "actionDurationMs": 1233,
      "hitTimeMs": 1000,
      "resourceCost": 19,
      "movementDistance": 0.0,
      "maximumRange": 3.0,
      "serverDamageProfileId": "damage.player.38140",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38180,
      "characterClass": "GUNSLINGER",
      "inputSlot": "S",
      "displayName": "나선의추적자",
      "actionId": "gunslinger.skill.38180",
      "skillKind": "ACTIVE",
      "cooldownMs": 8000,
      "actionDurationMs": 1000,
      "hitTimeMs": 1000,
      "resourceCost": 21,
      "movementDistance": 0.0,
      "maximumRange": 5.6,
      "serverDamageProfileId": "damage.player.38180",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38210,
      "characterClass": "GUNSLINGER",
      "inputSlot": "D",
      "displayName": "썸머솔트샷",
      "actionId": "gunslinger.skill.38210",
      "skillKind": "ACTIVE",
      "cooldownMs": 9000,
      "actionDurationMs": 1600,
      "hitTimeMs": 1000,
      "resourceCost": 23,
      "movementDistance": 0.0,
      "maximumRange": 3.3,
      "serverDamageProfileId": "damage.player.38210",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38260,
      "characterClass": "GUNSLINGER",
      "inputSlot": "F",
      "displayName": "피스키퍼",
      "actionId": "gunslinger.skill.38260",
      "skillKind": "ACTIVE",
      "cooldownMs": 12000,
      "actionDurationMs": 6267,
      "hitTimeMs": 1000,
      "resourceCost": 25,
      "movementDistance": 0.0,
      "maximumRange": 15.8,
      "serverDamageProfileId": "damage.player.38260",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38290,
      "characterClass": "GUNSLINGER",
      "inputSlot": "T",
      "displayName": "프리즌 불릿",
      "actionId": "gunslinger.skill.38290",
      "skillKind": "ACTIVE",
      "cooldownMs": 50000,
      "actionDurationMs": 3267,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 17.2,
      "serverDamageProfileId": "damage.player.38290",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38250,
      "characterClass": "GUNSLINGER",
      "inputSlot": "V",
      "displayName": "황혼의 눈",
      "actionId": "gunslinger.skill.38250",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 3600,
      "hitTimeMs": 1746,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 20.1,
      "serverDamageProfileId": "damage.player.38250",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38320,
      "characterClass": "GUNSLINGER",
      "inputSlot": "ALT_V",
      "displayName": "데드 엔드",
      "actionId": "gunslinger.skill.38320",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 6467,
      "hitTimeMs": 1451,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 33.0,
      "serverDamageProfileId": "damage.player.38320",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38000,
      "characterClass": "GUNSLINGER",
      "inputSlot": "LMB",
      "displayName": "핸드건 평타",
      "actionId": "gunslinger.skill.38000",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1000,
      "hitTimeMs": 121,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 14.5,
      "serverDamageProfileId": "damage.player.38000",
      "effectId": "",
      "comboStages": [
        { "actionDurationMs": 1000, "hitTimeMs": 121, "inputOpenMs": 75, "inputCloseMs": 360 },
        { "actionDurationMs": 1600, "hitTimeMs": 260, "inputOpenMs": 371, "inputCloseMs": 718 },
        { "actionDurationMs": 1767, "hitTimeMs": 761, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    },
    {
      "skillId": 45620,
      "characterClass": "SLAYER",
      "inputSlot": "E",
      "displayName": "그라운드 스매시",
      "actionId": "slayer.skill.45620",
      "skillKind": "ACTIVE",
      "cooldownMs": 15000,
      "actionDurationMs": 3933,
      "hitTimeMs": 130,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 6.3,
      "serverDamageProfileId": "damage.player.45620",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45210,
      "characterClass": "SLAYER",
      "inputSlot": "R",
      "displayName": "플라잉 스트라이크",
      "actionId": "slayer.skill.45210",
      "skillKind": "ACTIVE",
      "cooldownMs": 16000,
      "actionDurationMs": 2200,
      "hitTimeMs": 500,
      "resourceCost": 17,
      "movementDistance": 0.0,
      "maximumRange": 8.6,
      "serverDamageProfileId": "damage.player.45210",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45300,
      "characterClass": "SLAYER",
      "inputSlot": "A",
      "displayName": "페이탈 소드",
      "actionId": "slayer.skill.45300",
      "skillKind": "ACTIVE",
      "cooldownMs": 22000,
      "actionDurationMs": 2667,
      "hitTimeMs": 500,
      "resourceCost": 19,
      "movementDistance": 0.0,
      "maximumRange": 6.6,
      "serverDamageProfileId": "damage.player.45300",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45070,
      "characterClass": "SLAYER",
      "inputSlot": "S",
      "displayName": "허리케인 소드",
      "actionId": "slayer.skill.45070",
      "skillKind": "ACTIVE",
      "cooldownMs": 24000,
      "actionDurationMs": 7000,
      "hitTimeMs": 1600,
      "resourceCost": 21,
      "movementDistance": 0.0,
      "maximumRange": 7.3,
      "serverDamageProfileId": "damage.player.45070",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45190,
      "characterClass": "SLAYER",
      "inputSlot": "D",
      "displayName": "파이널 블로",
      "actionId": "slayer.skill.45190",
      "skillKind": "ACTIVE",
      "cooldownMs": 24000,
      "actionDurationMs": 2433,
      "hitTimeMs": 1250,
      "resourceCost": 23,
      "movementDistance": 0.0,
      "maximumRange": 5.9,
      "serverDamageProfileId": "damage.player.45190",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45600,
      "characterClass": "SLAYER",
      "inputSlot": "F",
      "displayName": "마운틴 클리브",
      "actionId": "slayer.skill.45600",
      "skillKind": "ACTIVE",
      "cooldownMs": 27000,
      "actionDurationMs": 2533,
      "hitTimeMs": 1000,
      "resourceCost": 25,
      "movementDistance": 0.0,
      "maximumRange": 16.5,
      "serverDamageProfileId": "damage.player.45600",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45810,
      "characterClass": "SLAYER",
      "inputSlot": "V",
      "displayName": "라그나 브레이크",
      "actionId": "slayer.skill.45810",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 4000,
      "hitTimeMs": 2200,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 29.7,
      "serverDamageProfileId": "damage.player.45810",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45820,
      "characterClass": "SLAYER",
      "inputSlot": "ALT_V",
      "displayName": "레이지 슬래셔",
      "actionId": "slayer.skill.45820",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 13000,
      "hitTimeMs": 1500,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 16.5,
      "serverDamageProfileId": "damage.player.45820",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45000,
      "characterClass": "SLAYER",
      "inputSlot": "LMB",
      "displayName": "버서커 평타",
      "actionId": "slayer.skill.45000",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1800,
      "hitTimeMs": 360,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 3.5,
      "serverDamageProfileId": "damage.player.45000",
      "effectId": "",
      "comboStages": [
        { "actionDurationMs": 1800, "hitTimeMs": 360, "inputOpenMs": 200, "inputCloseMs": 470 },
        { "actionDurationMs": 1600, "hitTimeMs": 278, "inputOpenMs": 200, "inputCloseMs": 420 },
        { "actionDurationMs": 1567, "hitTimeMs": 429, "inputOpenMs": 200, "inputCloseMs": 600 },
        { "actionDurationMs": 1867, "hitTimeMs": 501, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    },
    {
      "skillId": 31510,
      "characterClass": "ARTIST",
      "inputSlot": "E",
      "displayName": "필법 : 올려치기",
      "actionId": "artist.skill.31510",
      "skillKind": "ACTIVE",
      "cooldownMs": 16000,
      "actionDurationMs": 3167,
      "hitTimeMs": 1445,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 8.2,
      "serverDamageProfileId": "damage.player.31510",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31470,
      "characterClass": "ARTIST",
      "inputSlot": "R",
      "displayName": "필법 : 한획긋기",
      "actionId": "artist.skill.31470",
      "skillKind": "ACTIVE",
      "cooldownMs": 25000,
      "actionDurationMs": 2833,
      "hitTimeMs": 1657,
      "resourceCost": 17,
      "movementDistance": 0.0,
      "maximumRange": 19.8,
      "serverDamageProfileId": "damage.player.31470",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31410,
      "characterClass": "ARTIST",
      "inputSlot": "A",
      "displayName": "묵법 : 해우물",
      "actionId": "artist.skill.31410",
      "skillKind": "ACTIVE",
      "cooldownMs": 30000,
      "actionDurationMs": 1833,
      "hitTimeMs": 1657,
      "resourceCost": 19,
      "movementDistance": 0.0,
      "maximumRange": 10.7,
      "serverDamageProfileId": "damage.player.31410",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31490,
      "characterClass": "ARTIST",
      "inputSlot": "S",
      "displayName": "묵법 : 범가르기",
      "actionId": "artist.skill.31490",
      "skillKind": "ACTIVE",
      "cooldownMs": 30000,
      "actionDurationMs": 1967,
      "hitTimeMs": 1654,
      "resourceCost": 21,
      "movementDistance": 0.0,
      "maximumRange": 5.0,
      "serverDamageProfileId": "damage.player.31490",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31900,
      "characterClass": "ARTIST",
      "inputSlot": "V",
      "displayName": "절기 : 진경산수",
      "actionId": "artist.skill.31900",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 5300,
      "hitTimeMs": 1876,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 13.2,
      "serverDamageProfileId": "damage.player.31900",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31920,
      "characterClass": "ARTIST",
      "inputSlot": "ALT_V",
      "displayName": "신수도 : 봉황",
      "actionId": "artist.skill.31920",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 5333,
      "hitTimeMs": 1654,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 16.5,
      "serverDamageProfileId": "damage.player.31920",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31000,
      "characterClass": "ARTIST",
      "inputSlot": "LMB",
      "displayName": "평타",
      "actionId": "artist.skill.31000",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1233,
      "hitTimeMs": 276,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 7.6,
      "serverDamageProfileId": "damage.player.31000",
      "effectId": "",
      "comboStages": [
        { "actionDurationMs": 1233, "hitTimeMs": 276, "inputOpenMs": 157, "inputCloseMs": 434 },
        { "actionDurationMs": 1233, "hitTimeMs": 269, "inputOpenMs": 200, "inputCloseMs": 540 },
        { "actionDurationMs": 1333, "hitTimeMs": 494, "inputOpenMs": 1150, "inputCloseMs": 1333 },
        { "actionDurationMs": 1267, "hitTimeMs": 193, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    },
    {
      "skillId": 2050110,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "Q",
      "displayName": "예고",
      "actionId": "dimensionmaster.skill.2050110",
      "skillKind": "ACTIVE",
      "cooldownMs": 16000,
      "actionDurationMs": 2100,
      "hitTimeMs": 1451,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 5.0,
      "serverDamageProfileId": "damage.player.2050110",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050150,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "W",
      "displayName": "공간 베기",
      "actionId": "dimensionmaster.skill.2050150",
      "skillKind": "ACTIVE",
      "cooldownMs": 25000,
      "actionDurationMs": 1933,
      "hitTimeMs": 1451,
      "resourceCost": 19,
      "movementDistance": 0.0,
      "maximumRange": 8.3,
      "serverDamageProfileId": "damage.player.2050150",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050220,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "E",
      "displayName": "일점 관통",
      "actionId": "dimensionmaster.skill.2050220",
      "skillKind": "ACTIVE",
      "cooldownMs": 40000,
      "actionDurationMs": 1667,
      "hitTimeMs": 1451,
      "resourceCost": 23,
      "movementDistance": 0.0,
      "maximumRange": 21.5,
      "serverDamageProfileId": "damage.player.2050220",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050190,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "R",
      "displayName": "진공",
      "actionId": "dimensionmaster.skill.2050190",
      "skillKind": "ACTIVE",
      "cooldownMs": 48000,
      "actionDurationMs": 2400,
      "hitTimeMs": 1789,
      "resourceCost": 27,
      "movementDistance": 0.0,
      "maximumRange": 13.2,
      "serverDamageProfileId": "damage.player.2050190",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050240,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "A",
      "displayName": "경계 돌파",
      "actionId": "dimensionmaster.skill.2050240",
      "skillKind": "ACTIVE",
      "cooldownMs": 50000,
      "actionDurationMs": 2000,
      "hitTimeMs": 1451,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 16.5,
      "serverDamageProfileId": "damage.player.2050240",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050210,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "S",
      "displayName": "분광",
      "actionId": "dimensionmaster.skill.2050210",
      "skillKind": "ACTIVE",
      "cooldownMs": 55000,
      "actionDurationMs": 2767,
      "hitTimeMs": 1451,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 9.9,
      "serverDamageProfileId": "damage.player.2050210",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050200,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "D",
      "displayName": "공간 절단",
      "actionId": "dimensionmaster.skill.2050200",
      "skillKind": "ACTIVE",
      "cooldownMs": 60000,
      "actionDurationMs": 2633,
      "hitTimeMs": 1639,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 13.2,
      "serverDamageProfileId": "damage.player.2050200",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050500,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "F",
      "displayName": "업의 경계",
      "actionId": "dimensionmaster.skill.2050500",
      "skillKind": "ACTIVE",
      "cooldownMs": 120000,
      "actionDurationMs": 4367,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 9.9,
      "serverDamageProfileId": "damage.player.2050500",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050510,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "T",
      "displayName": "일념",
      "actionId": "dimensionmaster.skill.2050510",
      "skillKind": "ACTIVE",
      "cooldownMs": 120000,
      "actionDurationMs": 3000,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 33.0,
      "serverDamageProfileId": "damage.player.2050510",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050540,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "V",
      "displayName": "무간의 옥",
      "actionId": "dimensionmaster.skill.2050540",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 5100,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 16.5,
      "serverDamageProfileId": "damage.player.2050540",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050550,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "ALT_V",
      "displayName": "찰나",
      "actionId": "dimensionmaster.skill.2050550",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 5000,
      "hitTimeMs": 1730,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 39.6,
      "serverDamageProfileId": "damage.player.2050550",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 2050010,
      "characterClass": "DIMENSIONMASTER",
      "inputSlot": "LMB",
      "displayName": "기본 공격",
      "actionId": "dimensionmaster.skill.2050010",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1400,
      "hitTimeMs": 100,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 7.3,
      "serverDamageProfileId": "damage.player.2050010",
      "effectId": "",
      "comboStages": [
        { "actionDurationMs": 1400, "hitTimeMs": 100, "inputOpenMs": 100, "inputCloseMs": 510 },
        { "actionDurationMs": 1500, "hitTimeMs": 43, "inputOpenMs": 0, "inputCloseMs": 410 },
        { "actionDurationMs": 1067, "hitTimeMs": 28, "inputOpenMs": 200, "inputCloseMs": 1067 },
        { "actionDurationMs": 1700, "hitTimeMs": 335, "inputOpenMs": 0, "inputCloseMs": 0 }
      ]
    }
  ]
}
```

### G01.3 G01 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

기대 출력 (현재 기준선은 `5 player profiles, 20 skills, 17 damage profiles, 1 bosses`):

```text
Gameplay balance Validate succeeded: 5 player profiles, 73 skills, 55 damage profiles, 1 bosses.
```

skills 카운트는 `SKILL` 54행 + `SKILLSTAGE` 19행(34010 4 + 38000 3 + 45000 4 + 31000 4 +
2050010 4)이다. **이 명령과 기대 출력은 계획 단계에서 실제로 한 번 실행해 확인한 뒤
되돌렸다.** Validate 실패는 어느 항목이든 이 G의 실패이며 JSON을 되돌린다. 추가로
`git diff --check`가 공백 오류 0이어야 한다.

## G02. Server 계약 테스트와 주석 교정

`Server/Private/ServerGameplayContractTests.cpp`의 변경은 **현재 worktree 기준** 세
갈래다: `QUICK_SKILLS`에 신규 41슬롯 계약 추가(창술사의 기존 미커버 7슬롯 포함 —
데이터는 무변경이고 G03 audit의 전 슬롯 계약과 대칭을 맞춘다), class별 damage profile
resolve 4건 추가,
34010 콤보 블록 뒤에 차원술사 콤보 행동 블록과 건슬링어/슬레이어/아티스트 콤보 데이터
블록 추가. 콤보 메커니즘 자체(중복 press 무시, hit 전 hold 등)는 34010 블록이 이미
검증하고 차원술사 블록이 두 번째 class에서 반복 검증하므로, 나머지 세 class는
**데이터**(kind·스테이지 수·창 경계·1단 승인)만 검증한다.

적용 방식 주의:

1. 이 파일의 현재 worktree본에는 08-04 Character Select Arena 슬라이스의 **미커밋**
   블록(`WORLD_ID::CHARACTER_SELECT_ARENA` world/navigation/skill 검증, `<cmath>`
   include 포함)이 있고, 그 블록은 미커밋 `Shared/Public/Network/PacketType.h`(v7 bump +
   enum)에 의존한다. 아래 전문은 그 블록을 보존한 worktree 기준이다. 커밋 선후 조건은
   6장을 따른다.
2. 대상 `.cpp`는 CRLF 파일이고 이 계획서 markdown은 LF다. 전문을 통째로 붙여넣지 말고
   삽입 지점만 제자리 편집한다(붙여넣을 경우 CRLF로 저장). 그렇지 않으면 파일 전체가
   줄바꿈 diff로 바뀐다.
3. **기준선 변경 경고의 적용**: 이 파일은 원작 밸런스 마이그레이션 세션도 수정 중이다.
   마이그레이션 커밋 후 `Find_Damage` 4회 신규 호출은 `Find_DamageRatePercent`와 rate
   기대값으로, `quickPlayer.iCurrentResource = 100`은 그 시점 풀 값(실측 1000)으로 바꿔
   적용한다(G01.0). 보존 구간의 기대값(34120 damage, 9350 HP 등)은 마이그레이션 세션의
   소유이므로 그쪽 diff를 그대로 따른다.

### G02.1 `Server/Private/ServerGameplayContractTests.cpp` — 적용 후 전문

```cpp
#include "ServerGameplayContractTests.h"

#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>

namespace
{
	struct TESTS
	{
		void Require(const bool condition, const char* name)
		{
			std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
			if (!condition)
				++failures;
		}
		int failures = 0;
	};

	struct QUICK_SKILL_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		const char* inputSlot;
	};

	constexpr std::array QUICK_SKILLS
	{
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34120, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34080, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38020, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38050, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45050, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31210, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31230, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34070, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34150, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34110, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34090, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34640, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34600, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34620, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38120, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38200, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38180, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38210, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38260, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38290, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38250, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38320, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45620, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45300, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45070, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45190, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45600, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45810, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45820, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31510, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31470, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31410, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31490, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31900, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31920, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050110, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050150, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050220, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050190, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050240, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050210, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050200, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050500, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050510, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050540, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050550, "ALT_V" }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable Q/W skill binding");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		quickPlayer.iCurrentResource = 100;
		quickPlayer.iMaximumResource = 100;
		C2S_USE_SKILL quickCommand{};
		quickCommand.iClientSequence = 1;
		quickCommand.iSkillId = contract.skillId;
		quickCommand.fAimX = 1.f;
		quickCommand.fAimZ = 0.f;
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1),
			"Approve playable Q/W skill command");
	}
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::GUNSLINGER),
		"Resolve Gunslinger player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::SLAYER),
		"Resolve Slayer player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST),
		"Resolve Artist player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER),
		"Resolve DimensionMaster player profile");
	tests.Require(nullptr == catalog.Find_Player(CHARACTER_CLASS_ID::DESTROYER),
		"Reject unsupported Destroyer player profile");
	tests.Require(650u == catalog.Find_Damage("damage.player.34120"),
		"Resolve player damage profile");
	tests.Require(800u == catalog.Find_Damage("damage.player.2050110"),
		"Resolve DimensionMaster damage profile");
	tests.Require(800u == catalog.Find_Damage("damage.player.38120"),
		"Resolve GunSlinger damage profile");
	tests.Require(800u == catalog.Find_Damage("damage.player.45620"),
		"Resolve Slayer damage profile");
	tests.Require(800u == catalog.Find_Damage("damage.player.31510"),
		"Resolve Artist damage profile");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(navigation.Find_Path(152.f, -137.f, 151.f, -122.f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

	CWorldBootstrap trainingWorld;
	CServerNavigation trainingNavigation;
	tests.Require(trainingWorld.Load(WORLD_ID::TRAINING_GROUND) &&
		trainingWorld.Get_AreaId() == "LV_DEV_TRAINING_GROUND" &&
		std::all_of(
			trainingWorld.Get_Placements().begin(),
			trainingWorld.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty();
			}),
		"Load class-neutral training player spawns");
	tests.Require(trainingNavigation.Load("LV_DEV_TRAINING_GROUND"),
		"Load training server navigation");
	SERVER_NAV_POINT trainingPoint{};
	tests.Require(trainingNavigation.Project_Point(0.f, -4.f, trainingPoint),
		"Project training spawn to walkable cell");
	tests.Require(!trainingNavigation.Project_Point(16.01f, 0.f, trainingPoint),
		"Reject training point beyond arena navigation bounds");

	CWorldBootstrap characterSelectWorld;
	CServerNavigation characterSelectNavigation;
	const bool characterSelectWorldLoaded =
		characterSelectWorld.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
	const auto& characterSelectSpawns =
		characterSelectWorld.Get_Placements();
	tests.Require(
		characterSelectWorldLoaded &&
		characterSelectWorld.Get_AreaId() ==
			"LV_LOBBY_CLASSSELECT_SL00" &&
		characterSelectSpawns.size() == 4 &&
		std::all_of(
			characterSelectSpawns.begin(),
			characterSelectSpawns.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty() &&
					placement.isEnabled;
			}),
		"Load class-neutral Character Select arena player spawns");
	tests.Require(
		characterSelectNavigation.Load("LV_LOBBY_CLASSSELECT_SL00"),
		"Load Character Select arena server navigation");
	bool characterSelectSpawnsOnNavigation =
		characterSelectWorldLoaded && characterSelectSpawns.size() == 4;
	SERVER_NAV_POINT characterSelectPoint{};
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : characterSelectSpawns)
	{
		SERVER_NAV_POINT projected{};
		characterSelectSpawnsOnNavigation =
			characterSelectSpawnsOnNavigation &&
			characterSelectNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		characterSelectSpawnsOnNavigation,
		"Project all Character Select spawns to baked navigation");
	if (!characterSelectSpawns.empty())
	{
		characterSelectNavigation.Project_Point(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectPoint);
	}
	std::vector<SERVER_NAV_POINT> characterSelectPath;
	tests.Require(
		characterSelectSpawns.size() >= 2 &&
		characterSelectNavigation.Find_Path(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectSpawns[1].fPositionX,
			characterSelectSpawns[1].fPositionZ,
			characterSelectPath) &&
		characterSelectPath.size() >= 2 &&
		std::adjacent_find(
			characterSelectPath.begin(),
			characterSelectPath.end(),
			[](const SERVER_NAV_POINT& left, const SERVER_NAV_POINT& right)
			{
				return std::abs(left.y - right.y) > 0.6f;
			}) == characterSelectPath.end(),
		"Find Character Select arena navigation path");
	SERVER_NAV_POINT characterSelectOutside{};
	tests.Require(
		!characterSelectNavigation.Project_Point(
			-787.6f,
			197.5f,
			characterSelectOutside),
		"Reject point beyond Character Select arena navigation bounds");

	SERVER_PLAYER arenaSkillPlayer{};
	arenaSkillPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	arenaSkillPlayer.iCurrentHp = 1000;
	arenaSkillPlayer.iMaximumHp = 1000;
	arenaSkillPlayer.iCurrentResource = 100;
	arenaSkillPlayer.iMaximumResource = 100;
	arenaSkillPlayer.fPositionX = characterSelectPoint.x;
	arenaSkillPlayer.fPositionY = characterSelectPoint.y;
	arenaSkillPlayer.fPositionZ = characterSelectPoint.z;
	C2S_USE_SKILL arenaSkillCommand{};
	arenaSkillCommand.iClientSequence = 1;
	arenaSkillCommand.iSkillId = 34120;
	arenaSkillCommand.fAimX = characterSelectPoint.x + 3.f;
	arenaSkillCommand.fAimZ = characterSelectPoint.z;
	CPlayerSkillSystem arenaSkillSystem;
	std::vector<SERVER_WORLD_ENTITY> arenaEntities;
	tests.Require(
		arenaSkillSystem.Try_Start(
			arenaSkillPlayer,
			arenaSkillCommand,
			catalog,
			10) &&
		PLAYER_ACTION_STATE::SKILL == arenaSkillPlayer.eAction &&
		34120u == arenaSkillPlayer.iCurrentSkillId &&
		10u == arenaSkillPlayer.iActionStartTick,
		"Start Character Select arena skill action");
	arenaSkillSystem.Update(
		arenaSkillPlayer,
		arenaEntities,
		catalog,
		&characterSelectNavigation,
		1.f / 30.f,
		11);
	SERVER_NAV_POINT arenaSkillPoint{};
	tests.Require(
		characterSelectNavigation.Project_Point(
			arenaSkillPlayer.fPositionX,
			arenaSkillPlayer.fPositionZ,
			arenaSkillPoint),
		"Keep Character Select skill action position on baked navigation");

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.iCurrentResource = 100;
	player.iMaximumResource = 100;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	boss.fPositionZ = -122.f;
	std::vector<SERVER_WORLD_ENTITY> entities{ boss };
	C2S_USE_SKILL useSkill{};
	useSkill.iClientSequence = 1;
	useSkill.iSkillId = 34120;
	useSkill.fAimX = boss.fPositionX;
	useSkill.fAimZ = boss.fPositionZ;
	CPlayerSkillSystem skills;
	tests.Require(skills.Try_Start(player, useSkill, catalog, 10),
		"Approve valid skill command");
	tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
		"Reject duplicate skill command while action is active");
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, 1.f / 30.f, tick);
	tests.Require(9350u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage once");
	C2S_USE_SKILL cooldownAttempt = useSkill;
	cooldownAttempt.iClientSequence = 2;
	tests.Require(!skills.Try_Start(player, cooldownAttempt, catalog, 70),
		"Reject skill during authoritative cooldown");

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(34010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve LanceMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 34010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve basic attack first stage");

		// 329ms is where stage one opens; 100ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.1f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.4f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer combo input inside the window");

		press.iClientSequence = 4;
		comboSkills.Try_Start(comboPlayer, press, catalog, 15);
		tests.Require(1u == comboPlayer.iComboStage,
			"Ignore a second press inside the same window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 5;
		other.iSkillId = 34120;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 16) &&
			34010u == comboPlayer.iCurrentSkillId,
			"Reject a different skill during a combo");

		/* Stage one is 1633 ms long but its hit lands at 470 ms, so a buffered
		press has to cut in there rather than waiting out the clip. 20 ticks is
		about 667 ms: past the hit, nowhere near the full duration. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the next combo stage once the hit has landed");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(2050010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			100u == combo->ComboStages[0].iInputOpenMs &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve DimensionMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 2050010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve DimensionMaster basic attack first stage");

		// 100ms is where stage one opens; 50ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.05f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject DimensionMaster combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.3f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer DimensionMaster combo input inside the window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 4;
		other.iSkillId = 2050110;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 15) &&
			2050010u == comboPlayer.iCurrentSkillId,
			"Reject a different DimensionMaster skill during the combo");

		/* Stage one forces its end at 1400 ms but its hit lands at 100 ms, so a
		buffered press cuts in on the first update past the hit. */
		comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, 16);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the second DimensionMaster stage once the hit has landed");

		/* Nothing is buffered now, so stage two has to run its whole 1500 ms and
		drop back to idle instead of chaining into stage three. */
		for (std::uint32_t tick = 17; tick < 64; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the DimensionMaster combo when no press was buffered");

		SERVER_PLAYER wrongClass{};
		wrongClass.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		wrongClass.iCurrentHp = 1;
		wrongClass.iMaximumHp = 1;
		wrongClass.iCurrentResource = 100;
		wrongClass.iMaximumResource = 100;
		C2S_USE_SKILL crossClass{};
		crossClass.iClientSequence = 1;
		crossClass.iSkillId = 2050010;
		crossClass.fAimX = 1.f;
		crossClass.fAimZ = 0.f;
		CPlayerSkillSystem crossSkills;
		tests.Require(!crossSkills.Try_Start(wrongClass, crossClass, catalog, 1),
			"Reject DimensionMaster basic attack for another class");
	}

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(38000);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			3u == combo->ComboStages.size() &&
			75u == combo->ComboStages[0].iInputOpenMs &&
			0u == combo->ComboStages[2].iInputCloseMs,
			"Resolve GunSlinger basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::GUNSLINGER;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 38000;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSkills;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve GunSlinger basic attack first stage");
	}

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(45000);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			200u == combo->ComboStages[0].iInputOpenMs &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve Slayer basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 45000;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSkills;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve Slayer basic attack first stage");
	}

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(31000);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			157u == combo->ComboStages[0].iInputOpenMs &&
			1150u == combo->ComboStages[2].iInputOpenMs &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve Artist basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 31000;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSkills;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve Artist basic attack first stage");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	valtan.strDamageProfileId = "damage.valtan.basic-swing";
	valtan.iCurrentHp = 10000;
	valtan.iMaximumHp = 10000;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fPatternMaximumRange = 8.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	valtan.iPatternTelegraphMs = 1;
	valtan.iPatternActiveMs = 300;
	valtan.iPatternRecoveryMs = 1;
	CValtanBrain brain;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 100);
	brain.Update(valtan, players, catalog, navigation, 0.1f, 101);
	tests.Require(650u == players.begin()->second.iCurrentHp,
		"Apply server-authoritative Valtan damage once");
	valtan.iCurrentHp = 5000;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 102);
	tests.Require(2u == valtan.iPhase, "Advance Valtan phase from server HP");

	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
```

새 차원술사 콤보 블록의 tick 산술 근거:

- `Try_Start` 직후 1단, `fActionElapsedSeconds`를 0.05로 두면 1단 창(100~510ms) 이전이므로
  버퍼되지 않고, 0.3이면 창 안이므로 버퍼된다.
- 버퍼된 상태의 `Update` 1회(1/30s)로 elapsed 0.3333 ≥ hit 0.1 → damage 적용 → 버퍼
  cancel 전진으로 2단이 된다. 창술사(hit 470ms)와 달리 1틱이면 충분하다.
- 2단(1500ms)은 버퍼 없이 47틱(1.5667s)을 돌리면 stage 소진 없이 종료되어 `NONE`/stage
  0으로 복귀한다.
- 타 class 도용 거부는 `Try_Start`의 `skill->eCharacterClass != player.eCharacterClass`
  가드가 데이터에 실제로 걸리는지 확인한다.

### G02.2 `Server/Private/PlayerSkillSystem.cpp` — 주석 1건 교정

hit이 자기 입력 창보다 앞서는 stage(차원술사 3단 28<200, 건슬링어 2단 260<371, 아티스트
3단 494<1150)가 들어오면 아래 기존 주석의 전제("모든 단계의 hit는 자기 입력 창 안에
있다")가 데이터 사실이 아니게 된다. 동작은 이미 `hasAppliedSkillDamage` 게이트가
보장하므로 로직 변경 없이 주석만 사실에 맞게 바꾼다. 이 파일도 CRLF이므로 해당 줄만
제자리 편집한다.

기존 (`Update` 안, `cancelsIntoNextStage` 위):

```cpp
	/* A buffered press cancels the rest of the clip once the hit has landed,
	which is what makes a combo read as fast. Every stage's hit time is inside
	its own input window, so cutting here never drops damage. */
```

변경:

```cpp
	/* A buffered press cancels the rest of the clip once the hit has landed,
	which is what makes a combo read as fast. The cancel gates on
	hasAppliedSkillDamage, so cutting here never drops damage even when a
	stage's hit lands before its input window opens. */
```

### G02.3 `Client/Private/PlayerController.cpp` — 최소 콤보 창 주석 2건 교정

전 class 콤보 창 최소 폭이 창술사 2단 224ms에서 아티스트 3단 183ms로 바뀐다. 재전송
간격 100ms는 여전히 더 좁으므로 로직 변경은 없고, 수치를 인용한 주석 두 곳만 교정한다.
CRLF 제자리 편집.

기존 1 (`BASIC_ATTACK_RESEND_INTERVAL` 선언 위, 46~47행):

```cpp
	/* Tighter than the narrowest combo window the balance data declares (224 ms
	on the second basic-attack stage), so a held button cannot skip one. */
```

변경 1:

```cpp
	/* Tighter than the narrowest combo window the balance data declares (183 ms
	on the third stage of the Artist basic attack), so a held button cannot skip one. */
```

기존 2 (`Poll_BasicAttack` 안, 215~217행):

```cpp
	/* The first press goes out immediately; holding repeats on an interval
	narrower than the tightest combo window in the data (224 ms), so a held
	button always lands at least one press inside it. */
```

변경 2:

```cpp
	/* The first press goes out immediately; holding repeats on an interval
	narrower than the tightest combo window in the data (183 ms), so a held
	button always lands at least one press inside it. */
```

### G02.4 G02 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

```bash
MSBuild.exe Server/Default/Server.vcxproj -p:Configuration=Debug -p:Platform=x64 -m
```

```bash
Server/Bin/Debug/Server.exe --contract-test
```

기대 결과: 기존 PASS에 더해 아래 신규 라인이 모두 `[PASS]`이고 `failures : 0`.

```text
[PASS] Resolve playable Q/W skill binding        (총 49회 — 신규 41)
[PASS] Approve playable Q/W skill command        (총 49회 — 신규 41)
[PASS] Resolve DimensionMaster damage profile
[PASS] Resolve GunSlinger damage profile
[PASS] Resolve Slayer damage profile
[PASS] Resolve Artist damage profile
[PASS] Resolve DimensionMaster basic attack combo stages
[PASS] Approve DimensionMaster basic attack first stage
[PASS] Reject DimensionMaster combo input before the window opens
[PASS] Buffer DimensionMaster combo input inside the window
[PASS] Reject a different DimensionMaster skill during the combo
[PASS] Cancel into the second DimensionMaster stage once the hit has landed
[PASS] End the DimensionMaster combo when no press was buffered
[PASS] Reject DimensionMaster basic attack for another class
[PASS] Resolve GunSlinger basic attack combo stages
[PASS] Approve GunSlinger basic attack first stage
[PASS] Resolve Slayer basic attack combo stages
[PASS] Approve Slayer basic attack first stage
[PASS] Resolve Artist basic attack combo stages
[PASS] Approve Artist basic attack first stage
failures : 0
```

Server 빌드는 `BeforeTargets="ClCompile"`로 매 빌드마다 balance Publish를 자동 실행하므로
빌드 직후에는 bootstrap이 stale일 수 없다. stale 위험은 **JSON만 고치고 재빌드 없이 기존
`Server.exe --contract-test`를 다시 돌릴 때**이며, 그 경우 위 Publish를 먼저 수동
실행한다. Release 구성도 같은 순서로 확인한다. G02.3은 Client 파일이므로 Client Debug
빌드까지 확인한다.

## G03. ProjectAudit 확장 — 전 class 스킬 계약을 audit에 등록

현행 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`의 `gameplay.playable-qw-contract`는
`$dimensionmasterSkillRows.Count -eq 0`을 **강제**한다("아직 스킬 계약이 없다"는 과거
상태의 audit화). G01 데이터만 추가하면 이 check가 즉시 실패하므로, 같은 변경 단위에서
audit을 새 계약으로 확장한다. 이 파일은 현재 worktree에 다른 세션의 미커밋 수정(42+/25-,
map-editor-workspace·character-select·effect G1→G4 관련)이 있으나 아래 두 블록은 그
diff와 겹치지 않는다. 두 블록만 제자리 편집하고 주변 diff를 보존한다.

### G03.1 quick-slot 계약 블록 교체

기존 (`$playerSkillDocument` 로드 직후):

```powershell
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	foreach ($className in @('LANCE_MASTER','GUNSLINGER','SLAYER','ARTIST')) {
		foreach ($slotName in @('Q','W')) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			if ($bindings.Count -ne 1) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	$dimensionmasterSkillRows = @($playerSkillDocument.skills |
		Where-Object characterClass -eq 'DIMENSIONMASTER')
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0 -and
		$dimensionmasterSkillRows.Count -eq 0) "missing=$($missingQuickSlots -join ',') dimensionmasterUnverified=$($dimensionmasterSkillRows.Count)"
```

변경:

```powershell
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	$classQuickSlotContracts = [ordered]@{
		'LANCE_MASTER' = @('Q','W','E','R','A','S','T','V','ALT_V','LMB')
		'GUNSLINGER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','LMB')
		'SLAYER' = @('Q','W','E','R','A','S','D','F','V','ALT_V','LMB')
		'ARTIST' = @('Q','W','E','R','A','S','V','ALT_V','LMB')
		'DIMENSIONMASTER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','LMB')
	}
	foreach ($className in $classQuickSlotContracts.Keys) {
		foreach ($slotName in $classQuickSlotContracts[$className]) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			if ($bindings.Count -ne 1) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	$dimensionmasterSkillRows = @($playerSkillDocument.skills |
		Where-Object characterClass -eq 'DIMENSIONMASTER')
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0 -and
		$dimensionmasterSkillRows.Count -eq 12) "missing=$($missingQuickSlots -join ',') dimensionmasterRows=$($dimensionmasterSkillRows.Count)"
```

### G03.2 animation 계약 목록을 전 class 전체 바인딩으로 확장

기존:

```powershell
	$quickSkillAnimationContracts = @(
		[pscustomobject]@{ Class = 'LANCE_MASTER'; Asset = 'LanceMaster'; Skills = @(34120, 34080) },
		[pscustomobject]@{ Class = 'GUNSLINGER'; Asset = 'GunSlinger'; Skills = @(38020, 38050) },
		[pscustomobject]@{ Class = 'SLAYER'; Asset = 'Slayer'; Skills = @(45050, 45060) },
		[pscustomobject]@{ Class = 'ARTIST'; Asset = 'Artist'; Skills = @(31210, 31230) }
	)
```

변경:

```powershell
	$quickSkillAnimationContracts = @(
		[pscustomobject]@{ Class = 'LANCE_MASTER'; Asset = 'LanceMaster'; Skills = @(
			34120, 34080, 34070, 34150, 34110, 34090, 34640, 34600, 34620, 34010) },
		[pscustomobject]@{ Class = 'GUNSLINGER'; Asset = 'GunSlinger'; Skills = @(
			38020, 38050, 38120, 38200, 38140, 38180, 38210, 38260, 38290, 38250, 38320, 38000) },
		[pscustomobject]@{ Class = 'SLAYER'; Asset = 'Slayer'; Skills = @(
			45050, 45060, 45620, 45210, 45300, 45070, 45190, 45600, 45810, 45820, 45000) },
		[pscustomobject]@{ Class = 'ARTIST'; Asset = 'Artist'; Skills = @(
			31210, 31230, 31510, 31470, 31410, 31490, 31900, 31920, 31000) },
		[pscustomobject]@{ Class = 'DIMENSIONMASTER'; Asset = 'DimensionMaster'; Skills = @(
			2050110, 2050150, 2050220, 2050190, 2050240, 2050210,
			2050200, 2050500, 2050510, 2050540, 2050550, 2050010) }
	)
```

이 check는 각 skillId가 `<Asset>.clipseq`의 행(`^<id>\s`)과 `<Asset>.clipmap`의
`skill=<id>` 항목으로 존재하는지 검사한다. **55개 ID 전부 두 파일에 존재함을 계획
단계에서 스크립트로 확인했다** (창술사 기존 10개 포함).

### G03.3 G03 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

기대: `gameplay.playable-qw-contract`(dimensionmasterRows=12),
`gameplay.playable-qw-animation-contract`, `gameplay.balance-publish-contract` PASS.
G01 이전에 이 G만 먼저 적용하면 실패하므로 G01과 같은 변경 단위로만 적용한다. 현재
브랜치에 이미 존재하는 `asset-lock.inventory` 1건 실패(로컬 DimensionMaster payload가
immutable pack에 없음)는 이번 작업과 무관한 기존 상태이며 이번 변경으로 실패 수가
늘어나면 안 된다.

## G04. 런타임 검증 — Server + Client 실측

코드가 아니라 실행 계약을 검증하는 장이다. `Framework.slnLaunch`의 `Server + Client`
profile로 둘을 함께 실행한다 (기본 `127.0.0.1:7777`).

절차와 판정 기준 (각 class 공통, Lobby → `Character Select` → class 선택 → `Enter Test`
→ `CHARACTER_SELECT_ARENA` Server 승인 재진입):

1. **차원술사** — LMB 꾹: `att_battle_1_01→_04` 4단 연쇄 후 idle 복귀. 1단이 4초 클립
   꼬리(1.4s 이후)를 재생하지 않는지 본다. LMB 단발: 1단만 재생 후 서버 1400ms 엣지에서
   idle. `Q W E R A S D F T V ALT_V` 각 1회: 클립 재생·쿨다운 시작·재입력 거부·자원
   소모(Q/W/E/R). V(무간의 옥) 5.1s, ALT_V(찰나) 5.0s 클립 완주 후 idle 복귀.
2. **건슬링어** — LMB 꾹: 핸드건 평타 3단 연쇄. `E R A S D F`(전부 핸드건 계열 6~12s
   쿨) 연속 시전으로 짧은 쿨다운 타일 회전, T(프리즌 불릿)/V(황혼의 눈)/ALT_V(데드 엔드)
   완주 확인.
3. **슬레이어** — LMB 꾹: 버서커 평타 4단 연쇄. S(허리케인 소드)가 7초 클립을 완주하고
   idle로 복귀하는지(이동 보정 없이 제자리 시전) 본다. V/ALT_V 완주 확인.
4. **아티스트** — LMB 꾹: `_03→_02→_01→_04` 순서 4단 연쇄(3단 입력 창이 클립 끝
   1150~1333ms에 몰려 있어 hold가 아니면 잇기 어렵다 — hold 100ms 재전송으로 연쇄되는지
   확인). E R A S V ALT_V 각 1회.
5. **창술사** — 무변경 회귀: Q/W/평타가 기존과 동일하게 동작.
6. **콤보 중 스킬 거부** — 각 class 평타 연쇄 도중 Q를 눌러도 평타가 유지된다(서버 거부).
7. **창 밖 입력** — 수동 재현이 어려우므로 판정은 G02 contract test를 정본으로 하고,
   여기서는 꾹 누르기/단발의 육안 차이만 본다.
8. **원격 표현** — Client 두 개를 띄워 한쪽 평타 연쇄가 다른 쪽에서 같은 단계로
   재생되는지 본다(단계 정본이 snapshot임을 확인).
9. **HUD** — 타일은 slot 문자열 사전순(예: 건슬링어 `A ALT_V D E F Q R S T V W`)으로
   표시된다. 물리 키 배열 순서 표시는 별도 슬라이스(7장)이며 결함으로 판정하지 않는다.
   미만료 쿨다운이 동시에 9개 이상이면 `MAX_PLAYER_COOLDOWNS = 8` 절단으로 일부 타일이
   사용 가능처럼 보일 수 있다(서버 판정은 정확). 건슬링어 11슬롯에서 재현 가능성이
   커졌으므로 7장에 기록하고, 이 검증 절차의 "각 1회" 순차 시전에서는 결함으로 판정하지
   않는다.
10. **disconnect** — gameplay 중 Server 종료 시 replicated state 정리 후 Lobby 복귀.

마지막으로 정본 회귀를 실행한다 (audit 기대 결과는 G03.3과 동일):

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

## G05. 팀 문서 갱신 — 구현·검증 완료 후 같은 변경 단위에서

두 문서 모두 현재 worktree에 다른 세션의 미커밋 수정이 있다. 아래 문장 단위 교체만
수행하고 주변 diff를 건드리지 않는다.

### G05.1 `AGENTS.md` — `팀 인터페이스와 담당 영역` 절

기존 문장:

```text
기존 네 class의 Q/W는 각각 `34120/34080`, `38020/38050`, `45050/45060`, `31210/31230`으로 닫혔다.
```

```text
reference 4종에는 원작 `DIMENSIONMASTER.loa`와 balance DB에서 추출한 clip/notify/chain/timing 데이터가 들어 있으나, 이는 저작·비교용 참조이며 Server 계약이 아니다. Server 계약이 있는 skill ID가 생기기 전에는 DimensionMaster Q/W 또는 candidate-only effect를 활성화하지 않는다.
```

변경 문장:

```text
다섯 class 전부 quick slot 스킬과 좌클릭 평타 `COMBO` 계약이 닫혔다: LanceMaster 9슬롯+34010(4단), GunSlinger 11슬롯(`Q W E R A S D F T V ALT_V`)+38000(3단), Slayer 10슬롯(`Q W E R A S D F V ALT_V`)+45000(4단), Artist 8슬롯(`Q W E R A S V ALT_V`)+31000(4단), DimensionMaster 11슬롯(`Q W E R A S D F T V ALT_V`)+2050010(4단)이다. 각 class의 슬롯-skill ID 정본은 `Data/Balance/PlayerSkills.json`이다.
```

```text
reference 4종에는 원작 `DIMENSIONMASTER.loa`와 balance DB에서 추출한 clip/notify/chain/timing 데이터가 들어 있으며, 위 스킬 계약의 쿨다운·hit·클립·콤보 창 수치가 여기서 유도됐다. runtime 표현은 Client가 `.clipseq`를 직접 소비하므로 PlayerSkills의 skillId는 clipseq 스킬 ID와 동일해야 한다. candidate-only effect는 여전히 활성화하지 않는다.
```

### G05.2 `CLAUDE.md` — `최소 수련장 Area` 절

기존 문장:

```text
기존 네 class의 Q/W는 각각 `34120/34080`, `38020/38050`, `45050/45060`, `31210/31230`으로 Server 계약이 있다. DimensionMaster의 Animation reference 4종에는 원작 `DIMENSIONMASTER.loa`와 balance DB에서 추출한 clip/notify/chain/timing 데이터가 있으나 Server 계약이 있는 skill ID가 아직 없으므로 Q/W와 candidate-only effect는 비활성이다.
```

변경 문장:

```text
다섯 class 전부 Animation reference에서 유도한 quick slot 스킬과 좌클릭 평타 `COMBO`의 Server 계약이 있다(각 class의 슬롯 구성과 skill ID는 `Data/Balance/PlayerSkills.json`이 정본). candidate-only effect는 여전히 비활성이다.
```

### G05.3 `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

class별 스킬 표가 있으면 다섯 class의 슬롯 계약을 같은 형식으로 갱신한다. 구현 시점의
실제 표 구조를 따르고 새 섹션을 만들지 않는다.

### G05.4 G05 검증

`git diff`로 교체 문장 외 변경이 없는지, 다른 세션의 미커밋 수정이 보존됐는지 확인한다.
RESULT 문서는 `.md/GB/08-05/2026-08-05_DIMENSIONMASTER_SKILL_BINDING_BA_COMBO_RESULT.md`로
작성하고 자동 검증·수동 검증·미검증을 분리 기록한다.

## 6. 선행 조건·브랜치·커밋 단위

- **선행 조건**: `Server/Private/ServerGameplayContractTests.cpp`의 현재 worktree본에는
  08-04 Character Select Arena 슬라이스의 미커밋 블록이 있고, 이 블록은 미커밋
  `Shared/Public/Network/PacketType.h`(v7, `WORLD_ID::CHARACTER_SELECT_ARENA`)와
  Character Select world/navigation 데이터에 의존한다. HEAD에서 새 브랜치를 만들어 이
  계획의 전문을 커밋하면 **컴파일이 깨진다.** 08-04 슬라이스가 먼저 커밋·머지된 뒤 그
  위에서 브랜치를 만들거나, 같은 커밋 계열에서 선커밋한다.
- 이번 작업 브랜치는 `codex/all-class-skill-binding`이며 다음 파일을 하나의 검증 단위로
  묶는다: `Data/Balance/DamageProfiles.json`, `Data/Balance/PlayerSkills.json`,
  `Server/Private/ServerGameplayContractTests.cpp`,
  `Server/Private/PlayerSkillSystem.cpp`(주석 1건),
  `Client/Private/PlayerController.cpp`(주석 2건),
  `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`, `AGENTS.md`, `CLAUDE.md`, 팀 사용서,
  PLAN/RESULT 문서.
- `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 publisher 생성물이므로 커밋하지
  않는다.
- **선행 조건 추가**: 원작 밸런스 마이그레이션(OFFICIAL_BALANCE 슬라이스)이 계획 검증
  이후 worktree에 진입했다(검증 시점 `git status --short -- Data/Balance
  Tools/GameplayPipeline` clean → 비평 시점 `Data/Balance` 4개 + publisher + Server 9개
  파일 M, `ServerGameplayContractTests.cpp` 포함). 이 슬라이스는 **그 마이그레이션이
  커밋된 뒤** G01.0 재산정을 거쳐 그 위에서 적용한다. 마이그레이션 중간 상태 위에 G01/G02
  전문을 그대로 적용하면 publisher throw(`formatVersion 1` 거부), 컴파일 실패
  (`Find_Damage` 부재), 타 세션 데이터 파괴가 발생한다.
- 다른 세션이 수정 중인 파일(`AGENTS.md`, `CLAUDE.md`, 팀 사용서,
  `Invoke-ProjectAudit.ps1`, `ServerGameplayContractTests.cpp`, `Data/Balance/*`,
  `Publish-GameplayBalance.ps1`, `PlayerSkillSystem.cpp`)은 stage 전에 현재 diff를
  보존·조정하고, 소유가 불분명하면 커밋을 분리한다. 구현 시작 시 `git status --short`로
  동시 편집 여부를 반드시 재확인하고, diff가 있으면 덮어쓰지 말고 먼저 조정한다.

## 7. 범위 밖 (이번 슬라이스에서 하지 않는 것)

- **다단 입력 COMBO 스킬 승격** (평타 외 mode=COMBO — 별도 PLAN): 차원술사
  2050120/2050160/2050170/2050250/2050530, 건슬링어 38040/38070/38190/38240/38340,
  슬레이어 45700~45760/45800, 아티스트 31450/31500, 슬레이어 폭주 평타 45001.
- **HOLD/차지형 재생 계약**: 건슬링어 38060/38080/38270, 슬레이어 45080. `2050520
  시간의 굴레`도 클립 루프/hold 계약이 먼저 필요하다.
- **스탠스 전환**(38160/38161)과 무기 스왑 표현 — 건슬링어 신규 슬롯은 전부 핸드건
  계열로 뽑아 시각 충돌을 피했지만, 샷건/라이플 스킬(38110 마탄의사수의 range 규칙
  포함)은 스탠스 계약과 함께 별도 슬라이스다.
- **이동기(SPACE)·기상기·스킬 이동 보정** — 45070 허리케인 소드의 `move=300`은 단위
  환산 계약이 없어 0.0으로 저장했다. 이동 보정은 AGENTS.md 고정대로 별도 슬라이스.
- **아이덴티티/아크패시브 자원 체계**: 아티스트 31110/31060, 슬레이어 45003/45004(폭주).
- **다중타 hit 계약**: 45110 플래시 블레이드, 31460 호접몽, 31480 두루미나래처럼
  `hits=""` 반복 타격 구조는 단일 hitTime 계약으로 표현 불가. per-stage damage 분리와
  다중 hit도 창술사 34010과 공유하는 한계.
- **아티스트 Q 31210의 최저 seq COMBO 표현 문제** (G00.7) — 기존 닫힌 계약의 잠재 시각
  결함 보고. 수정은 별도 슬라이스.
- effect 연결 (`effectId` 채우기, candidate `.effect` 승격), animation notify 기반
  사운드/셰이크.
- `MAX_PLAYER_COOLDOWNS = 8` 상향과 결정적 절단(만료 임박 순) — 건슬링어/차원술사
  11슬롯에서 미만료 쿨다운 9개 이상 동시 상태가 이전보다 쉽게 발생한다. Shared 계약과
  protocol harness를 함께 바꾸는 별도 슬라이스다.
- HUD quick slot 타일의 물리 키 배열 순서 표시 — 현재는 slot 문자열 사전순이다.
- resource/damage baseline과 평타 사거리(건슬링어 14.5 유도값) 튜닝 — 팀 baseline이며
  밸런스 담당의 후속 조정 대상. **원작 밸런스 반영 슬라이스가 먼저 worktree에 진입했으므로
  (기준선 변경 경고 참조), 그 슬라이스 커밋 후 이 계획의 G01.0 재산정 계약으로 신규 38개
  스킬의 rate/CostMp/Cooltime을 EFTable에서 끊는다. 측정 확장 책임은 이 슬라이스 쪽에
  있다.**
- 후속 슬롯 확장 후보(G00.5 잔여 유효 후보 11종)의 바인딩.
