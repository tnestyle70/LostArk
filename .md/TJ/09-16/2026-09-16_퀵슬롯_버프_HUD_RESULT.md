# 퀵슬롯 HUD 보강 결과 — 탈것 6종·특수(Space) 슬롯·버프 바·스킬 표식 (2026-09-16)

브랜치 `feature/quickslot-buff-hud` (feature/nameplate 위, origin/main `e71dd131` 리베이스 상태, protocol 89). 사용자 요청: main에 새로 들어온 탈것에 맞춰 탈것 목록·탈것별 HUD를 갱신하고, 중앙 엠블렘 위의 대쉬(Space) 쿨타임 아이콘, 좌측 슬롯 위의 버프/지속 아이콘, 스킬 아이콘 위의 연계기/키다운 표시를 원작대로 확인해 적용.

## 1. 원작 조사 (quickslot.gfx 전수 · `D:/ClaudeWork/Extracted/HudGfx_quickslot`)

| 항목 | 원작 |
|---|---|
| 특수 슬롯 | `QuickSlotFrame.specialSlotList` = `QuickSlotSpecialSkillBar` at stage (907,869): `QuickSlot_BattleSlot` 2개(slot0/1, 58px 간격, scale 0.9). `QuickSlotSpecialSlotManager.playingCoolDownLength`가 그려진 슬롯 수에 따라 화면 중앙(W/2)에 맞춰 x를 잡는다 — 1개면 그 슬롯이 정중앙. 슬롯 아트는 일반 스킬 슬롯과 같은 48×50(`shape 83`: I50 (656,978) 46² 프레임 + I3F (0,364) 50² 바탕). |
| 버프 바 | `buffList` `BuffSlotManagerEx` at (830,914), `deBuffList` at (1064,914). `perItemWidth/Height 29`, `numColumnsPerPage 10`. 아이템 = components.gfx `Shared_BuffSlot_Common`: `FCSlotBg`(shareimage_i7 858,330 64² × 0.375 = 24) at (1,1), 테두리 28² at (-1,-1) — buff `components_i8` (453,300) 녹색 / debuff (483,300) 적색 / party (363,300) 금색, `Buff_coolDown_102` 42px 파이(검정 a127), `cooldownText` `$YG760` 10px (-3,28) 34폭 중앙 — buff `#9BD979`, debuff `#E2C87A`, `valueAmount`(스택) 12px `#66FF99`/`#FF6600`. |
| 스킬 표식 | `BattleSlot.skillTypeMc` = `Shared_SkillSlotType_Icon` at (3,4): `continueMark`(`SlotSkillIcon_Combo` shareimagev2 i6 (66,1004) 16×15), `holdingMark`(`SlotSkillIcon_Holding` ib (869,1005)), `perfectComboMark`(`SlotSkillIcon_PerfectCombo` ib (887,1005)). `ARKNewSlot.set skillType`: `SKILL_TYPE_CHARGE(3)/HORDING(4)` → holding, `COMBO(5)/CHAIN(6)` → continue, `PERFECT_COMBO(13)` → perfect. |
| 연계 시간 | `BattleSlot.chainSkillTimeEffectMc` (-3,-2) 56px 67프레임 파이(검정 a180), host가 `chainSkillTimeEffect = frame`으로 남은 시간을 넣는다(`SkillSlot.as`). |
| 쿨타임 파이 | `coolDown_HUD_18` 56px 검정 a180(이미 arc shader로 구현돼 있음). 초 텍스트 `cooldownText` 16px, 키 라벨 `keyBind` 12px `#F3FEFF`. |
| 탈것 슬롯 | 원작은 `vehicleSlotList`(우측 64² 1칸)이지만 사용자 결정으로 Q/W/E 자리에 탈것 스킬, Space는 특수 슬롯을 쓴다(쿠크 상호작용 모드와 같은 틀). |

원작 좌표 → HUD 레이아웃(1280×720) 변환은 엠블렘 중심(원작 960 ↔ HUD 673.5)과 Q줄 윗변(원작 974 ↔ HUD 644.7) 기준 2/3 배율.

프로젝트 스킬의 `EFTable_Skill.Type`(테이블 열거값 = AS 상수 + 1): 6=연계 34140·34160(창술사 A/R), 17080(워로드 E), 31210(도화가 R), 45620, 38020/38200/38260, 평타 2050010/38000/31000; 5=홀딩 34590·34610(창술사 S·V), 38290, 45070; 4=차지 17240(워로드 T). 나머지 1(일반)/2(태세)/11(기상).

## 2. 구현

- `Tools/LpkPipeline/build_vehicle_ui.py`: 행을 `Data/Actors/VehicleCatalog.json` 6종(6705 황금 테르페이온, 9370 고요한 별빛의 가호, 7209 레인보우 모코보드, 8302 아우프슈텐-R, 8906 바다 유니콘 튜브, 9524 고대의 신화)으로, 스킬을 **`Data/Vehicles/VehicleProfiles.json skills[]`(Server 계약: SPACE/Q/W/E, cooldownMs)**에서 가져오도록 바꿨다(이전 EFTable 열 기반 Q/W/E/R 제거). 아이콘 `EFTable_Skill Icon/IconIndex → IconInfo`, 이름 `tip.name.skill_CommonAction_<id>`. 창은 6행(높이 510), `CVehicleWindowView` `ROW_COUNT 6`·`HINT_Y/BUTTON_Y` 연동, `VEHICLE_SKILL_UI{slot, skillId, cooldownMs, iconAsset}` + `Find_Skills()`.
- `Tools/LpkPipeline/build_quickslot_hud_ui.py`(신규): 버프 슬롯 아트(`UI/HUD/Buff/`), 상태 아이콘(침묵 `SkillBuff 1015`→`Buff_0`, 속박 1022→`Buff_37`, 공포 13010→`Buff_38`, 워로드 방어 태세 17810 스킬 아이콘), 스킬 표식 3장(`UI/HUD/Common/SkillType_*.png`), 클래스 이동기 아이콘 5장(`UI/Skill/<Class>/<id>_Space.png`), `Data/UI/HUD/SkillSlotMarks.json`(스킬→표식), `Data/UI/HUD/HudBuffSources.json`(복제 상태→아이콘), `HUD_Layout.json`에 슬롯 56개 **append**(Special_Space 4, Buff/Debuff 4×4×2, Skill_X_TypeMark/Chain 10×2). 아이콘 아틀라스 페이지는 umodel로 `EFUI_ICONATLAS_{B,L,G,D,Y,S,A}`, `EFUI_QUICKSLOT`, `EFUI_COMPONENTS` 추출.
- `CMainApp`:
  - `Update_VehicleHud`: Q/W/E = 탈것 스킬 아이콘 + 복제 쿨타임 파이(`HUD_PLAYER_STATE.Cooldowns` × catalog cooldownMs), R·A/S/D/F 잠금 아이콘, T/V 숨김(기존).
  - `Update_SpecialSlot`: 엠블렘 위 Space 슬롯 — 탑승 중이면 탈것 SPACE, 아니면 현재 태세의 클래스 이동기(`Find_BySlot(…, "SPACE", stance)`); 쿨타임 파이·초 텍스트·"Space" 키 라벨. **원작처럼 쿨타임이 도는 동안에만 보이고**(`playingCoolDownLength`), Space 스킬이 없는 클래스/쿠크 상호작용 모드에서는 숨김.
  - `Update_BuffBar`: `HudBuffSources.json` 순서로 **복제 상태만** 표시 — 탈것 탑승(버프, 탈것 아이콘), 워로드 `WARLORD_DEFENSE` 태세(버프), 침묵(디버프, 남은 초·지속 비율 파이), 패턴 속박(디버프, 남은 초), 공포(디버프, 남은 초). 최대 4/4.
  - `Update_SkillSlotMarks`: 슬롯별 표식(`SkillSlotMarks.json` — 프로젝트 `skillKind` 우선: HOLD→키다운(↓), COMBO→연계(>>), 그 외 ACTIVE만 원작 `Type`으로. 09-16 사용자·JS 확인: 처음엔 테이블 `Type`을 AS 상수(5=COMBO)로 읽어 적룡포·적룡질풍격에 >>가 붙었는데, 테이블 열거값은 AS 상수보다 1 크다(4=차지 풀배럴 캐넌, 5=홀딩 적룡포·적룡질풍격, 6=연계 선풍참혼·공의연무·대쉬 어퍼 파이어·필법 콩콩이·평타). 4/5→↓, 6/7→>>로 고쳐 창술사 S·V가 ↓다) + 연계 시간 파이: 실행 중 스킬(`iCurrentSkillId`)의 현재 콤보 단계(`iComboStage`)에 `inputOpenMs..inputCloseMs` 창이 있으면 `iActionStartTick` 기준 남은 비율. 모두 Server 값(단계 표는 `PlayerSkills.json`의 Server 계약). 탑승·쿠크 모드에서는 끔.
  - `RenderSkillCooldownText`: 특수 슬롯·탈것 Q/W/E 초 텍스트, 버프 아래 남은 초(YG760 10px, buff/debuff 색).
- `CCombatHUDViewModel`: `HUD_PLAYER_STATE += iFearEndTick, Cooldowns(원본 복제 목록)`.

## 3. 알고 있는 한계

- Server `PLAYER_SNAPSHOT.Cooldowns`는 최대 16개(`MAX_PLAYER_COOLDOWNS`)로 잘린다. 클래스 스킬 쿨타임이 많이 돌면서 탈것 스킬 4개까지 겹치면 뒤쪽 항목이 빠질 수 있다(Shared/Server 변경 없이는 해결 불가, 이번 범위 밖).
- 버프 바는 Server가 복제하는 상태만 보인다. 일반 스킬 버프/지속 효과 목록은 Server에 없다.
- 특수 슬롯 slot1(원작 두 번째 특수 슬롯)은 프로젝트에 대응 데이터가 없어 두지 않았다.
- 탈것 R(하차)은 원작 아이콘 데이터를 확인하지 못해 잠금 아이콘 그대로.

## 4. 검증

- `cl /Zs`: `MainApp.cpp`, `VehicleWindowView.cpp`, `CombatHUDViewModel.cpp` 오류 0. JSON parse OK, `git diff --check` OK.
- 빌드: Debug Product runner (아래 결과란).
- 화면 확인(사용자): 탈것 창 6행·아이콘, 탑승 시 Q/W/E 아이콘·쿨타임, Space 슬롯(탑승/비탑승), 버프 바(탑승 아이콘, 워로드 Z 방어 태세, 쿠크 침묵/속박), 창술사 A/R·워로드 E·도화가 R의 연계 표식과 연계 창 파이, 워로드 T·창술사 S의 키다운 표식.

## 5. 같은 날 Bern 진입 실패 진단 (참고)

닉네임 입력 → 캐릭터 생성 → Bern 진입이 `Server entry failed / main-app.target-level-create / loading.complete`로 실패했다. Server는 승인했고(`netEntityId` 배정), Client `CLevel_Bern::Initialize`가 E_FAIL을 낸 것이다.

- 원인: main `560741ac`(09-16)가 `LV_BER_BERNCASTLE.mapeffects.json`(LEVEL_ACTIVE/SOURCE_LOOP 물 이펙트 91개)을 추가했고, Bern 초기화의 `CMapEffectPresentationRuntime::Load_AmbientArea → Probe_WorldEffectAdmissions`가 probe를 `iLevelIndex = BERN`으로 스폰하는데 `Spawn_LevelPlacement`는 `iLevelIndex == Get_CurrentLevelID()`(그 시점 LOADING)를 요구해 "Level-placement Effect spawn descriptor is invalid"로 거절 → 맵 이펙트 로드 실패 → 레벨 생성 실패. Bern이 이 probe를 실제로 타는 첫 Area다(Character Select는 mapeffects 없음, Valtan의 1건은 표면 오버레이).
- 수정: main `eca41ee0`(09-16 16:19, tnestyle70)이 probe를 현재 레벨 인덱스로 스폰하고 `Level_Loading`에 Bern 분기, `Level_Bern`에 단계별 `FailActivation` 보고를 추가했다. 이 브랜치는 그 이전 `16d05213` 위에 있어 최신 main으로 다시 리베이스했다(protocol 88 충돌 → 칭호 변경을 89로).
- 함께 확인한 것: Bern 맵 런타임 55개 실물, 조명 v2 문서 파서 규칙 통과, 길찾기 런타임 최신, 물 이펙트 11종의 저작 문서·리소스 31개 로컬 존재, 이펙트 CSO 94개 정상. `LightResources.runtime.json` 추적본은 원본(rev 10)보다 뒤처져(rev 8) 있어 로컬만 재게시했다. `Invoke-BuildDomainOwner -Owner Client`는 composition.presentation에서 `World Sequence source.templates[61] unknown=['colliderTracks']`로 실패한다(main 도구·데이터 불일치, 이 브랜치 범위 밖).
