# 쿠크 시작 구역 일반 몬스터 추가 결과 (2026-09-19)

Area `LV_LUT_MIDNIGHTC_ED`(`WORLD_ID::KAKULSAYDON_ARENA`). 이 문서는 소스·데이터에서 직접 확인한 것과 확인하지 못한 것을 분리해 기록한다.
Client/Server 실행, 화면 확인, 빌드, Publish는 하지 않았다(작업 중 Visual Studio가 켜져 있었다).

## 상태 요약

| 항목 | 상태 |
|---|---|
| 원본 배치 데이터 발견 | 원본에서 직접 확인 |
| 몬스터 4종 모델·애니메이션 쿠킹, Resources 설치 | 구조 검사 완료. 화면 확인 안 함 |
| SpawnGroup·카탈로그·프로필·트리거 데이터 반영 | 저작 데이터 파일에 적용, `Publish-WorldGameplay.ps1 -Mode Validate` 통과. 게시(Publish) 안 함 |
| 서버 자동 발동 규칙 1행 추가 | 구문 검사(`cl /Zs`)만 통과. 빌드·실행 안 함 |
| 게임에서 몬스터가 나오는지 | **확인하지 않음** |

Publish와 Server 재빌드·재시작 전에는 게임에서 바뀌지 않는다.

## 1단계 배치 데이터 (있음)

원본 `leveldata1.lpk` `\Common_Extra\MapData\37081\DeployData.loa`의 `CEFDeployActor_NPC` 레코드 343개와
`TriggerMapData.loa`의 `CEFSequenceTriggerUnit` 160개(그중 `CEFSeqTN_Action_SpawnNPC` 노드)가 배치와 스폰 조건을 소유한다.
파서는 Mario 작업의 `inspect_source.py`와 같은 오프셋(위치 float3 +0, 회전 int32×3 +12, actorId +48, npcId +100)을 사용했다.

시작 구역의 원본 흐름(unit 번호, 라벨은 원본 그대로):

| unit | 조건 | 동작 |
|---|---|---|
| 101 시작지점_1 | 시작 볼륨(3.89, 8.64, -14.44) 진입 | 1번책 몬스터 8마리 + 2번책 폭탄 12개 스폰 |
| 111 1번책_이동 | 1번책 볼륨(1.49, 0.16, -29.71) 진입 | 1번책 몬스터 14마리 추가 스폰, 기존 8마리 진영 변경 |
| 112 | 1번책 몬스터 전멸 | 당기는 종이 프랍 01 활성화 |
| 121 2번책 이동 | 2번책 볼륨(-3.42, 0.09, -53.23) 진입 | 2번책 몬스터 15마리 스폰 |
| 122 | 위 15마리 전멸 | 2번책 몬스터 10마리 추가 스폰 |
| 123 | 2번책 몬스터 전멸 | 당기는 종이 프랍 02 활성화 |

## 2단계 몬스터 종류와 수량

시작 구역(X -7~14, Z -64~-28) NPC 액터: MN_CMDUP_02(NPC 480702) 30, MN_RHCN_01(480718, 폭탄) 12,
MN_REUP_04(480701) 8, MN_RHKP_06(480703) 8, MN_CMDGR_03(480704, 준보스 등급) 1. 폭탄을 뺀 47마리를 배치했다.
1번책 22마리(unit 101의 8 + unit 111의 14), 2번책 1파 15마리, 2파 10마리.

## 3단계 내 리소스 확인

`MN_RHCN_01`만 `Character/KoukuSaton/MN_RHCN_01`에 있다(폭탄, 이번 범위 밖). 나머지 4종은 없었다.

## 4단계 원본 리소스에서 추출

| 몬스터 | 메쉬 패키지 | AnimSet | 무기 | 무기 소켓 |
|---|---|---|---|---|
| MN_CMDUP_02 | `9G1MB9ITU1GZDEF4QHMT63S.upk` | MN_CMDUP_00 | WP_MN_RHKP_07 | bip001-prop1, pitch 90° |
| MN_REUP_04 | `9G1M8PTU1BZRE7JUMT64S8P.upk` | MN_REUP_01 | WP_MN_REUP_01 | bip001-prop1, pitch 90° |
| MN_RHKP_06 | `9G1M8AVU1BZ5E7JX0T64SYP.upk` | MN_RHKP_02 | WP_MN_RHKP_06 | b_wp_1, roll -90° |
| MN_CMDGR_03 | `9G1MB9I381GZKEF4QHKJ63S.upk` | MN_CMDGR_00 | WP_MN_RHKP_07 | bip001-prop1, (90°, 180°, -180°) |

AnimSet과 무기는 원본 LookInfo(`XmlData\LookInfo\Monster\EFDLChar_<모델>.loa`)에서 읽었다.
무기 소켓은 몸체 메쉬의 `SkeletalMeshSocket` export를 저장소 UPK 파서로 읽었고, 같은 방법이 Mario 모델
(REUP_05 `bip001-prop1`+pitch 90°, RHKP_07 `b_wp_1`+roll -19.995°)의 기존 하드코딩 값과 일치함을 먼저 확인했다.
추출물은 `C:\LostArkExtract\KMon_20260919`, 쿠킹 스크립트는 `out/KoukuMonsterImport/cook_kouku_monsters.py`(Mario `cook_monsters.py` 일반화).
원본 설치 폴더에는 쓰지 않았다.

## 5~7단계 배치·애니메이션

- 4종 모두 메쉬 스켈레톤과 PSA 본 집합이 정확히 일치했다(52/51/124/52본).
- 카탈로그가 요구하는 클립 `idle_battle_1`, `run_battle_1`, `dmg_idle_1`, `dead_1`, `att_battle_*`가 각 wmodel 안에 들어 있다. CMDUP는 공격 클립 이름에 `_01`이 없다(`att_battle_1`, `att_battle_2`).
- 산출물: wmodel 4개(메쉬 3/2/3/3개, 애니메이션 31/33/29/43개)와 텍스처 30개.
  wmodel 안의 텍스처 참조가 모두 `textures/` 아래 실제 파일과 연결된다.
- 설치 위치: `Client/Bin/Resources/Character/Monster/NPC_480701_MN_REUP_04`, `NPC_480702_MN_CMDUP_02`, `NPC_480703_MN_RHKP_06`, `NPC_480704_MN_CMDGR_03`.

## 바꾼 파일

| 파일 | 변경 |
|---|---|
| `Data/Actors/MonsterCatalog.json` | 4개 archetype 추가(`MONSTER_KOUKU_CMDUP_02`, `_REUP_04`, `_RHKP_06`, `_CMDGR_03`) |
| `Data/Balance/MonsterProfiles.json` | 위 4개 프로필 추가 |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/SpawnGroups.world.json` | 앵커 47개, `spawn.kouku.book1`(2 wave 16+6), `spawn.kouku.book2`(2 wave 15+10), revision 195→196 |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | `Book1_Monsters`, `Book2_Monsters` triggerBox 2개를 placements 끝에 추가, revision 8797→8798 |
| `Server/Private/ServerTriggerSystem.cpp` | `AUTO_ENTRY_RULES`에 `{KAKULSAYDON_ARENA, ACTIVATE_SPAWN_GROUP, "Book"}` 1행과 주석 1줄 추가 |
| `Resource_Distribution_2026-09-19_KoukuMonsters.txt`, `Copy_ResourceDistribution_2026-09-19_KoukuMonsters.ps1` | 팀 배포 목록(34개 파일, 41.5 MB)과 복사 스크립트 |
| `out/KoukuMonsterImport/` | 쿠킹·스테이징 스크립트와 백업(`backup-before-apply`), Git 제외 |

## 활성화 흐름

1. 플레이어가 시작 구역에서 `jump.1~3`(G 키)로 내려가 1번책에 착지한다.
2. 1번책 앞의 `Book1_Monsters` 상자(자동 발동)에 들어가면 서버가 `spawn.kouku.book1`을 활성화한다. 16마리가 즉시, 6마리가 0.2초 뒤에 스폰된다.
3. 종이 다리(`paper.1`) 뒤 2번책 입구의 `Book2_Monsters` 상자에 들어가면 `spawn.kouku.book2` 1파 15마리가 스폰되고, 모두 처치하면 2파 10마리가 스폰된다.
4. 트리거는 `triggerOnce=false`이고, 기존 `Activate_Repeat`로 진행 중이면 거절, 완료되고 몬스터가 없으면 다시 시작한다.

## 가정과 값의 근거

- 좌표 변환은 Mario 가져오기와 같다: 위치 `[x, z, -y] × 0.01`, 앵커 yaw = UE yaw × 360 / 65536. 앵커 47개 모두 현재 게시된 쿠크 `LV_LUT_MIDNIGHTC_ED.navgrid`의 걸을 수 있는 칸 위에 있고 높이 차이는 ±0.09 m 이내다.
- 원본에서는 시작 볼륨 진입 때 1번책 몬스터 8마리가 먼저 스폰되고, 1번책 진입 때 그 8마리에 `CEFSeqTN_Action_ChangeNPCFaction` 노드가 적용된다. 그 노드의 정확한 의미(중립→적대 전환인지)는 노드 이름에서 추정한 것이지 확인하지 못했다. 이번 구현은 1번책 진입 시점에 22마리를 함께 스폰하고 진영 전환은 구현하지 않았다.
- 트리거 볼륨 크기는 원본 `CEFDeployActor_Prop` 레코드의 +276/+280/+284 float(512/256/128 cm 등)를 **반크기**로 해석했다. 기존에 저작된 `2-1Stage_Move`(반크기 [3,1,3])와 256 cm 원본 볼륨의 관계를 근거로 한 추정이다. 회전은 원본 yaw를 앵커와 같은 방식으로 변환했고 부호는 확인하지 못했다.
- 서버 프로필(`MonsterProfiles.json`, `basis: PROJECT_TUNED`)
  - 원본 EFTable_Npc에서 가져온 값: 이동속도(215 / 108 cm/s → 2.15 / 1.08 m/s), 시야 800 cm → `engageRange` 8 m, 추적 2000 cm → `targetReleaseRange` 20 m, 공격 주기 ≤ 1.5초.
  - 원본에서 가져오지 **않은** 값(프로젝트 임시값): `maxHp`(일반 900, CMDGR 5000), `attackPower`(70/70/30/35), `defense`, 공격 판정 타이밍, 충돌 반지름, 가감속. 원본 NpcBalance는 레벨 스케일 비율(Hp 865, AttackPower 25/60, CMDGR Hp 430×5줄)이라 이 프로젝트의 데미지 수치와 직접 비교되지 않는다.
- 카탈로그는 Mario 원본 몬스터와 같은 형식(`modelScale` 0.01, `modelYawDegrees` -90)이다. 추적 클립은 원본 이동속도에 맞춰 `run_battle_1`을 썼다(Mario는 순찰이라 `walk_normal_1`).

## 실행한 검증

- 원본 데이터: unit 101/111/112/121/122/123 노드와 SpawnNPC 액터 목록, 볼륨 Prop 위치·크기를 바이트 단위로 디코딩했다.
- 앵커 47개 네비 확인(`out/KoukuMonsterImport/nav_report.json`).
- 쿠킹 4종 성공, wmodel 클립·텍스처 참조 검사, 설치본과 쿠킹 산출물 해시 동일.
- 적용 데이터를 Client `ActorCatalog::ParseMonsters` 규칙(키 9개, 클립 4종, 공격 클립 중복 없음, 범위)과 서버 publisher 규칙에 대해 점검했다. `Publish-WorldGameplay.ps1 -Mode Validate` 종료 코드 0(쿠크 배치 114개, spawn group 7개).
- 최종 교차 검사 스크립트 64개 항목 통과, 실패 0.
- `ServerTriggerSystem.cpp` `cl /Zs` 구문 검사 Debug·Release 통과, `git diff --check` 통과.
- 배포 스크립트를 임시 폴더로 실행해 34개 파일(43,532,470 bytes)이 복사됨을 확인했다.

## 하지 않은 것 / 알려진 한계

- **폭탄 12개**(MN_RHCN_01, 2번책 위)는 배치하지 않았다. 이동하지 않는 폭발형 NPC라 별도 메커니즘이 필요하다.
- 몬스터 전멸 → 종이 프랍 활성화 연결은 하지 않았다. 지금 `paper.1/2`는 기존처럼 밟으면 시퀀스가 재생된다.
- 서버 규칙 행에 대한 테스트 단언은 추가하지 않았다(같은 테스트 파일을 다른 작업이 편집 중). 기존 서버 테스트의 기대와는 코드를 읽어 확인한 범위에서 충돌하지 않는다.
- 쿠크 Loader는 몬스터 모델을 미리 준비하지 않는다. 첫 스폰 때 `ClientReplication`이 동기 로드하므로 첫 등장 순간에 프레임 정지가 있을 수 있다(RHKP_06 wmodel 11.7 MB). 필요하면 쿠크 Loader에 4개 archetype만 미리 준비하는 호출을 넣을 수 있다.
- 몬스터 외형·무기 방향·애니메이션 재생·전투 강도는 화면에서 확인하지 않았다. 사용자가 직접 확인해야 한다.

## VS를 끈 뒤 실행할 명령

```powershell
# 1) 월드 게시 (Server·Client의 worldbootstrap / spawngroupsbootstrap 생성)
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
# 2) Server 재빌드 (ServerTriggerSystem.cpp 변경)
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

그다음 Server를 재시작하고 Client는 다시 실행한다(카탈로그는 Client 시작 때 읽는다). 팀원은 `Copy_ResourceDistribution_2026-09-19_KoukuMonsters.ps1 -Source <받은 Resources> -Destination <Client/Bin/Resources>`로 리소스를 복사해야 한다.

## 추가 수정: 애니메이션 재생 속도 (2026-09-19 밤)

- **증상(사용자 관찰):** 몬스터의 걷기와 공격 모션이 제대로 나오지 않는다.
- **원인(측정으로 확정):** 4종 wmodel의 모든 clip(33·31·29·43개)이 1000 ticks/s로 저장되어 있었다. 엔진은 저장된 rate를 무시하고 30 ticks/s로 재생하므로(`Engine/Public/Animation.h:24`, `Engine/Private/Animation.cpp:51-52`) 실제로 33.3배 느렸다. 예: `run_battle_1`이 0.8초여야 하는데 26.7초, `att_battle_2_01`(RHKP_06)이 2.5초여야 하는데 83.3초. 서버 공격 구간은 1.5초라 공격 clip의 3~5%만 재생됐다.
- **놓친 이유:** 앞의 5~7단계 검증은 clip 이름이 wmodel 안에 있는지만 확인했다. 09-10 마리오 몬스터에서 같은 결함을 고친 절차(`retime_wmodel_ticks.py`)가 그 RESULT에만 있어 쿠킹 뒤에 실행되지 않았다.
- **수정:** `Tools/ActorXAssetCooker/retime_wmodel_ticks.py --ticks-per-second 30 --expect-ticks-per-second 1000`을 wmodel 4개에 실행했다. 원본은 `out/KoukuMonsterImport/backup_before_retime_20260919/`에 백업했다.
- **확인한 것(측정):** 4개 파일 136개 clip 전부 rate 30, 엔진 재생 시간이 저작 시간과 일치(오차 0.000000초), clip 이름·순서·파일 크기 불변. 코드, 데이터, 서버는 바꾸지 않았고 빌드와 게시도 필요 없다.
- **확인하지 못한 것:** 화면에서 걷기·공격이 자연스러운지는 사용자만 판정한다.
- **남은 후보(바꾸지 않음):**
  - 서버 공격 구간(windup+active+recovery)보다 긴 공격 clip은 서버 액션이 바뀔 때 잘린다. CMDUP_02는 1.5초 대비 1.57·1.73초, RHKP_06은 2.0·2.5초, CMDGR_03은 서버 1.95초 대비 2.13·4.0초다. REUP_04는 맞는다. 수정한다면 `MonsterCatalog.json`의 `attackPresentations[].playbackRate`(Client 전용)나 `MonsterProfiles.json`의 `attackRecoveryMs`(Server, 게시 필요)를 조정한다.
  - `CMDGR_03`의 이동 속도는 1.08 m/s인데 0.8초 주기의 `run_battle_1`을 쓴다. 같은 모델에 1.6초 주기의 `walk_normal_1`이 있다.
- **팀 배포:** 파일 크기는 같고 내용만 다르므로 이미 받은 팀원은 wmodel 4개를 다시 복사해야 한다. 안내와 새 해시는 `Resource_Distribution_2026-09-19_KoukuMonsters.txt` 끝에 적었다.
