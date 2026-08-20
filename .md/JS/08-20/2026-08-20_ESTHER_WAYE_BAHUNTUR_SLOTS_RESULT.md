# 2026-08-20 에스더 웨이·바훈투르 슬롯 활성화 RESULT

작성자: JS · branch `feature/esther-cutin`
PLAN: `2026-08-20_ESTHER_WAYE_BAHUNTUR_SLOTS_PLAN.md`

## 1. 구현 완료

PLAN 계약 그대로. 요점:

- 추출·쿠킹: `_export_esther_wb_g` 3패키지 exit 0, `_cook_esther_wb`에서
  - `Npc_58700.wmodel` 8메시(맨손), `Npc_59060.wmodel` 9메시(무기
    `WP_MN_YOBR_00` 병합, 67k 버텍스)
  - `AnimSets/NP_DPWI_00.wmodel` 클립 `npc_sk_dochul`+`npc_idle_battle_1`,
    `AnimSets/MN_YOBR_00.wmodel` 클립 `npc_att_battle_09_01`+`npc_sk_esthereffect_1`
  - 전부 `validate_wmodel` OK, Compare-Skeletons ALIGNED(86/86, 105/105),
    Compare-InverseBind IDENTICAL(maxDelta 0). Resources 물리 폴더 배치 완료.
- `NpcCatalog.json` 68→70 entry: `NPC_58700`/`NPC_59060` + `actionClips
  esther.strike` — 컷인·월드 클립 전환이 데이터로 함께 연결됨.
- Server: `ESTHER_ROSTER[]`{1:NPC_59030/3200, 2:NPC_58700/7100, 3:NPC_59060/5100},
  `Try_Consume(slot, outArchetype, outStrikeMs)`, `SERVER_WORLD_ENTITY.iEstherStrikeMs`,
  `Spawn_EstherSummon(strikeMs)` — strike 단계 길이가 슬롯별 클립 길이를 따름.
  전역 `ESTHER_STRIKE_MS` 제거. protocol 변경 없음(v 유지, 슬롯 필드 기존).
- Client: Valtan 로더 에스더 선로드 3종 확장(실패는 해당 archetype만 격리),
  READY 라벨 `Ctrl+Z/X/C`, `EstherUI.json` 슬롯 2·3 잠금 레이어 제거.

## 2. 자동 검증 (실행함)

- Server x64 Debug 빌드 exit 0, `Server.exe --contract-test` **failures 0** —
  신규: 슬롯2 소모(NPC_58700·7100ms·게이지 0), 슬롯3 소모(NPC_59060·5100ms),
  roster 밖 슬롯(4) 거부·게이지 보존, 실리안 summon의 `iEstherStrikeMs 3200` 확인.
  기존 에스더 타임라인/재사용 거부/빈 방 리셋 전부 유지.
- Client x64 Debug 빌드 exit 0.
- `NpcCatalog.json`(70)/`EstherUI.json`(12) parse OK,
  `Publish-WorldGameplay.ps1 -Mode Validate` 전 Area 통과.
- `git diff --check` clean. Validate 실행이 EOL만 다시 쓴
  `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction*.json` 2건은
  내용 무변경 확인 후 restore.

## 3. 수동 검증 — 사용자 확인 필요 (미실행)

로컬 Server+Client 실행 준비 완료. 발탄 진입 → 게이지 충전 후:

1. **Ctrl+X** → 웨이 소환(sk_dochul 7.1s) + 우하단 컷인 동반, 종료 시 상승 소멸.
2. **Ctrl+C** → 바훈투르 소환(att_battle_09 5.0s) + 컷인, 무기 표시 확인.
3. 등장(0.8s)/퇴장 구간의 대기 자세 — 바훈투르는 `sk_esthereffect_1` 대체라
   자세가 이상하면 클립 교체 검토.
4. 게이지 라벨 `Ctrl+Z/X/C`와 슬롯 2·3 잠금 아이콘 제거 확인.

## 3.1 1차 확인 피드백 반영 (2026-08-20)

- 웨이 컷인 정상, 바훈투르는 컷인 미표시 + 월드 소환수도 잠깐 나왔다 사라짐
  (사용자 관찰) — 원인 2건을 실측·수정:
  1. **컷인 프레이밍**: 실리안 기준 고정 카메라(거리 3.4m)가 거인 모델 내부로
     들어가 백페이스만 보임 → 컷인 시작 시 본 팔레트 최대 Y로 모델 높이를 실측해
     카메라를 높이 비례 자동 프레이밍(기존 구도 비율 유지, far 60, 폴백 2.2m).
  2. **클립 앵커 오프셋**: psa 실측 결과 바훈투르 공격 클립은 루트 이동이
     `b_root`가 아니라 **`bip001`에 무대 앵커 오프셋(-25m 낙하→-10m 유지)**으로
     저작돼 있어 strike 전환 순간 소환 위치에서 10m 밖으로 이동. `NpcCatalog`에
     optional `rootMotionLockBone`(NPC_59060=`bip001`)을 추가하고 CNpc(월드)와
     컷인 모두 해당 본 translation을 rest로 잠금. 다른 archetype은 필드 없음 —
     파서는 필드 수 검증에 optional 2종 반영.
- 부작용: 잠금으로 바훈투르의 낙하 인트로(~1.2s)가 제자리 동작이 됨(하단 후속).

## 3.2 2차 확인 피드백 반영 — 바훈투르 클립 교정 (2026-08-20)

- 사용자 관찰(원작: 점프→망치→구르기 2번→내리찍기, 14.7초는 과함)이 맞았다.
  `MN_YOBR_00-1.loa` 542800 스테이지를 파서로 실측한 결과 연합군은 3스테이지 체인
  `Att_Battle_9_01(2.0s) → 9_02(1.2s) → 9_03(0.7s)` ≈ 4.0초이며, 이 클립들은
  MN_YOBR_00 패키지가 아니라 **`UM_MA01\ani_0600\um_ma01_0600_bat_ani.psa`**(원형
  archetype 전투 셋, LookInfo가 가리키던 미추출 애니셋)에 있다. 처음 구운
  `att_battle_09_*`(5.0s/9.7s, bip001 무대 앵커 오프셋 -25m→-10m)는 다른 연출용
  좌표계 클립이라 폐기.
- 애니셋 재쿠킹: `MN_YOBR_00.wmodel` = `npc_att_battle_9_01/02/03` +
  `npc_idle_battle_1`(진짜 idle). validate OK, ALIGNED 105/105, InverseBind
  IDENTICAL. 새 클립은 루트 이동이 b_root에 정상 저작(점프 3.65u 낙하·구르기 소폭
  전진)이라 월드는 원작 그대로 재생.
- **actionClips 체인 지원**: 카탈로그 값이 단일 문자열 또는 순서 배열.
  `WORLD_ENTITY_PRESENTATION`에 `strActiveActionId`/`iActionClipIndex`를 추가해
  액션 시작 시 chain[0], 클립 종료(`Get_AnimationProgress` duration 도달) 시 다음
  클립으로 전환. 컷인도 같은 체인을 받아 순차 재생(요청 struct가 Clips 배열).
  컷인 세대 트리거는 클립 전환이 아니라 **액션 전환**에서 1회만 발화.
- 3.1의 `rootMotionLockBone` 카탈로그 필드/CNpc/replication 배선은 소비자가
  없어져 제거(컷인의 `b_root` 기본 잠금은 유지). Server slot 3 strikeMs
  5100 → **4000**, contract test 갱신 — failures 0. Server/Client 빌드 OK.

## 3.3 3차 확인 피드백 반영 — 웨이 디테일 (2026-08-20)

- **머리색**: 원작(검은머리+앞 흰 브릿지)과 다른 붉은 톤 — 원인은 미적용 LookInfo
  Hair MI. 실측: hair MI는 `npc_hair_trn_autosorting` twotone 계약으로
  `haircolor_base (0.077,0.056,0.047)` + `twotonecolor (0.635,0.571,0.509)`를
  텍스처에 곱하며, `np_dpwi_00_hair_d.tga`는 R=음영, **B=브릿지 이진 마스크**
  (76%/20% bimodal), A=알파 채널 패킹. 셰이더 확장 대신 **틴트를 tga에 베이크**
  (`lerp(base, twotone, B) × min(1, R×1.8)`)해 배포 텍스처만 교체 — wmodel은
  textures/ 폴더 참조라 재쿠킹 불필요. hair2(단색 base MI)도 동일 베이크.
- **타임라인**: sk_dochul이 등장·공격·퇴장 일체형이라 roster를
  `{slot, archetype, appearMs, strikeMs, leaveMs}`로 확장 —
  실리안 800/3200/1500, **웨이 0/7100/0**(스폰 즉시 STRIKE, 종료 즉시 despawn,
  상승 없음), 바훈투르 800/4000/1500 유지. `Try_Consume`은 roster entry 포인터
  출력, entity가 appear/leave ms 보유, leave 0이면 상승 스킵.
- contract test: 웨이 소모 타임라인 검증 + 핸들러 경로 2건(무 appear 스폰,
  클립 종료 즉시 despawn) 추가 — failures 0. Server 빌드 OK.
- 베이크 근사(R×1.8 게인)는 원본 셰이더 미확보 상태의 추정 — 화면 확인 후 조정.

## 3.4 4차 확인 피드백 반영 — 실리안 연출 교체 (2026-08-20)

- 사용자 관찰: 원작 실리안도 웨이처럼 상승하며 퇴장 — 코드 상승은 임의 연출.
- 실측: `NP_LRSA_00` 애니셋 전수 조사 결과 전용 등장/퇴장 클립은 없지만, 연합군
  액션(542600)의 두 번째 체인이 참조하는 **`evt1_sk_swordofchampion_bk`(157f,
  5.23s)**가 b_root 궤적상 **16m 상공 낙하 → 착지 대베기 → 마지막 도약 퇴장**의
  일체형 클립. 기존 `att_battle_7_01`은 같은 액션의 다른 체인(점프 베기, 착지로
  끝남 — 퇴장 없음).
- 애니셋 재쿠킹(`att_battle_7_01`+`idle_battle_1`+`evt1_sk_swordofchampion_bk`
  3클립), validate OK, Compare-Skeletons ALIGNED 104/104.
  Compare-InverseBind는 08-15 산출물과 달리 maxDelta 1.58e-3 DIFFERS —
  런타임 `Attach_AnimationSet`의 skeletonHash는 **본 이름 해시만** 사용
  (`WSkeletonReader.cpp` FNV over nameHash)이라 attach에는 무관, 시각 오차
  ~1.6mm 수준으로 무해 판단. 7_01은 A/B 비교용으로 애니셋에 유지(카탈로그 미참조).
- `NpcCatalog` esther.strike → `npc_evt1_sk_swordofchampion_bk`, roster 실리안
  **0/5300/0**(즉시 strike, 상승 로직 미사용, 종료 즉시 despawn). contract test를
  새 타임라인으로 재작성하고 appear/leave/rise 커버리지는 바훈투르 핸들러
  테스트로 이전 — failures 0.

## 3.5 5차 확인 — 실리안 육안 PASS, 바훈투르 클립 교정 (2026-08-20)

- 실리안 `evt1_sk_swordofchampion_bk` — **사용자 확인: "원작 애니메이션이랑 똑같음"**.
  한글 스킬명 대조: 실리안=완성된 패자의 검(SwordofChampion), 웨이=각성 도철
  (Dochul), 바훈투르=아크투르스의 숨결(BreathofArcturus) — 클립명이 스킬명 직역.
- 바훈투르 strike를 **`sk_breathofarcturus`(121f, 4.03s)**로 교체 — b_root 실측:
  3m 낙하 등장 → 전진 도약 강타(+3.3m) → 마지막 12.6m 상승 퇴장의 일체형.
  9_01/02/03 체인은 다른 연출(A/B용으로 애니셋에 유지, 카탈로그 미참조).
- 세 에스더 모두 일체형 클립로 확정되어 **서버 appear/leave 단계·상승 로직 전면
  제거**: roster `{slot, archetype, strikeMs}`(5300/7100/4100), 스폰 즉시
  `esther.strike` + PATTERN_ACTIVE, strikeMs 경과 시 sweep despawn.
  `ESTHER_ACTION_APPEAR/LEAVE`, `ESTHER_LEAVE_RISE_PER_SECOND`,
  entity appear/leave ms 삭제. contract test 재작성 — failures 0.
- actionClips 체인(배열) 지원은 현재 소비 데이터가 없지만 원본 loa 다단 스테이지
  계약의 일반형이라 유지.
- **후속(사용자 지시)**: 컷인 표시 타이밍을 원작처럼 — 실리안이 공중에 뜨는
  시점부터 컷인 표시 시작, 베고 착지하는 시점에 컷인 종료(클립 전체 동안 표시가
  아니라 구간 표시). 다음 작업.

## 3.6 6차 확인 — 스폰 1프레임 깜빡임 수정 (2026-08-20)

- 사용자 관찰: 세 에스더 모두 소환 순간 플레이어 위치에 1프레임쯤 보였다가
  깜빡이며 클립 시작 위치로 이동. 원인: `S2C_WORLD_ENTITY_SPAWNED`에 action이
  없어 다음 스냅샷(최대 1틱)까지 idle 포즈로 서 있다가 strike 클립 첫 프레임
  (공중 좌표)으로 점프.
- **protocol v25 → v26**: spawn 메시지에 `strActionId`(스폰 시점 action, 스냅샷과
  같은 id 공간, 빈 값 = idle 스폰) 추가. writer/reader validation은 스냅샷과 동일
  `Is_Valid_StableId(…, true)`.
- Server `Send_WorldEntitySpawned`가 entity action을 실어 보내고, Client NPC 스폰
  경로가 catalog 매핑 시 **스폰 즉시** 체인 첫 클립 설정 + `Play_Animation(0)`으로
  본 팔레트 선충전 + 컷인 트리거(스냅샷 경로는 같은 action이라 재트리거 없음).
- NetworkProtocolHarness: spawn 왕복에 actionId 추가, 불안정 actionId 거부·빈
  actionId 허용 2건 신설, 버전 핀 V26 갱신 — **failures 0**. Shared/Server/Client
  빌드 exit 0, contract test failures 0.
- Server pre-build publish가 EOL만 다시 쓴 worlddestruction 런타임 JSON 2건은
  내용 무변경 확인 후 restore(반복 발생 — publish의 개행 정책은 별도 이슈).

## 3.7 7차 — 컷인 표시 구간 데이터화 (2026-08-20)

- 원본 조사: 컷인 시작 지연은 `EFTable_EpicSkill.SkillDecoDelayTime1/2/3` =
  0 / 2800 / 2000ms(전 레벨 동일, 캐스트 시작 기준; 원작 소환 발동은 캐스트
  +500ms의 CommonActionExcute). 실리안 0은 사용자도 원작 재확인으로 승인
  (착시 원인은 원작 카메라 각도). 컷인 길이·모션은 `epicskill.gfx` MC 소유인데
  이 파일은 SWF가 아닌 전용 난독 컨테이너("GFX"+0x90, UI_DATA/BRANCH 문자열
  파편)라 로컬 미해석 — 종료 시점은 육안 계약으로 튜닝. 카메라 연출 데이터는
  없음(소환 액션 notify는 ViewShake뿐, 원작 컷인은 일러스트 MC) — 3D 컷인
  카메라는 프로젝트 자체 연출로 확정.
- 참고 실측: 실리안 BK 클립 b_root 궤적 = 0~0.8s 상공 하강, 1~2s 지상 차지,
  2.2~3.6s 공중 베기, 3.8s 착지, 5.2s 도약 퇴장.
- `NpcCatalog`에 optional `cutinWindow {startMs, endMs?}`(strike 시작 기준,
  endMs 생략 = 체인 끝) 추가 — actionClips 없는 entry 거부, endMs ≤ startMs
  거부. 시드: 실리안 {0, 3400}, 웨이 {2300}, 바훈투르 {1500}
  (원작 지연 − 500ms 캐스트 오프셋).
- 컷인 서비스: 클립 시계는 표시 창 밖에서도 계속 진행(월드와 동기 유지), 그리기만
  [startMs, endMs) 게이트, endMs 도달 시 조기 종료. Client 빌드 OK, catalog
  parse OK.
- **사용자 확인 완료 (2026-08-20)**: 세 슬롯 컷인 구간 육안 PASS
  (실리안 {0,3400} / 웨이 {2300} / 바훈투르 {1500}).

## 3.8 8차 — 컷인 카메라 런타임 튜닝 패널 (2026-08-20)

- 원작에 컷인 카메라 각도 데이터가 없음을 재확인(일러스트 MC) — 카메라는 프로젝트
  자체 연출이므로 F1 Developer Tools 허브에 `Esther Cutin (Debug)` 섹션 추가
  (_DEBUG 전용, 기존 허브 관례대로 탭·단축키 신설 없음).
- `ESTHER_CUTIN_TUNING`(yaw, eye X/Y per-height, distance, target Y, FOV,
  720p 기준 rect)을 서비스가 _DEBUG에서 소유, Release는 기존 상수 그대로.
  Reset, 값 표시(확정 시 상수로 베이크용), 슬롯별 Preview 버튼(게이지 없이
  `Apply_EstherCutinAction` 재생 — 순수 클라 표현이라 서버 무관, 프로토타입이
  로드된 레벨(발탄)에서만 동작).
- Client 빌드 OK. 확정값이 나오면 상수 베이크는 수동 후속.

## 4. 알려진 편차와 후속

- 웨이·바훈투르 모두 LookInfo MaterialInstance 재도색 미적용(실리안과 같은
  cook_npc 한계) — 기본 재질로 쿠킹, "not applied" 로그 확인함.
- 바훈투르 idle은 3.2에서 `UM_MA01` 셋의 진짜 `npc_idle_battle_1`로 교체 완료.
- 컷인 크기: 본 높이 기반 자동 프레이밍이라 웅크린 체형(바훈투르)은 작게 보일 수
  있음 — 사용자 육안 튜닝 대상.
- 소환 이펙트/사운드 없음(실리안과 동일, 이펙트 담당 인계).
- strike 수치·게이지 상수는 계속 서버 코드 소유(Data/Balance 승격은 기존 후속).
