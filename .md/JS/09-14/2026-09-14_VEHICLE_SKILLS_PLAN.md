# 2026-09-14 탈것 고유 스킬 PLAN (Space 대시 · Q/W/E · R 하차)

작성자: JS · 브랜치 `feature/vehicle-material-restoration`(로컬 커밋 `0f53f621`) 위에서 이어서 작업한다.
선행: [탈것 3종 추가 RESULT](2026-09-14_VEHICLE_ADDITIONS_RESULT.md), [테르페이온 탑승 RESULT](../09-13/2026-09-13_TERPEION_VEHICLE_RIDING_RESULT.md).

## G00. 목표와 사용자 결정

탑승 중 탈것 7종의 고유 스킬을 원작 데이터로 연결한다.

| 결정 | 내용 |
|---|---|
| Space 대시 | Server가 원본 루트 이동 거리만큼 navigation·충돌 검사를 거쳐 실제로 전진시키고 쿨타임을 관리한다 |
| Q/W/E | 탈것+탑승자 스킬 클립 재생, Server 쿨타임, 재생 중 이동 잠금. 파티클·사운드·랩터 속도 버프는 후속 |
| R | 탑승 중 R은 H와 같은 하차 요청. 탈것 스킬 전용 HUD는 후속 |

### 원본에서 확정한 사실

- `EFTable_Vehicle.MovingSkill`=Space, `SkillId0/1/2`=Q/W/E. 이름·설명은 `GameMsg tip.name/desc.skill_commonaction_<id>`.
- 쿨타임은 `EFTable_Skill.Cooltime`(SecondaryKey 최소 행).
- 스킬→클립 체인은 탈것 `XmlData/Action/<OriginalActionObjectGroupName>.loa`의 `CEFActionObject`(skillId) 아래 `LookInfoAnim` 스테이지다.
  공용 추출기는 `Anim/Stance_Anim`만 읽어 0행이었고, `LookInfoAnim`을 더한 scratch 사본으로 뽑았다. 길이는 `.animnotify`의 `len`.
- COMBO 체인(브레스·스윙·워터점프)은 첫 등장 순서대로 중복 없이 한 번씩 재생한다(추가 키 입력 분기는 후속).
- 루트 이동은 탈것 AnimSet PSA의 `b_root` 키. 좌표는 `extract_rootmotion.py` 규약(forward=0.01·x, lateral=−0.01·y, up=0.01·z).
- 탑승자 클립은 가족 AnimSet의 `ride_<모드>_<탈것 클립에서 npc_를 뺀 이름>`으로 7종 모두 존재한다(`pc_*_ani` 또는 `pc_*_vehicle_ani`).

| 탈것 | 슬롯 | skillId | 탈것 클립 체인 | 길이(ms) | 쿨(ms) | 루트 전진 |
|---|---|---|---|---|---|---|
| 황금 테르페이온 6705 (horse, `MN_PMSHS_00`) | Space | 96030 | sk_dash | 2067 | 10000 | 9.89m |
| | Q | 96000 | sk_victorypose | 2333 | 5000 | 0 |
| | W | 96010 | sk_relaxation | 6333 | 8000 | 0 |
| | E | 96020 | sk_backkick | 1667 | 5000 | 0 |
| 은색 전투 랩터 7104 (raptor, `MN_ISRX_01`) | Q | 95750 | att_battle_1_01 | 3000 | 8000 | 측정 |
| 고요한 별빛의 가호 9370 (swing, `MN_PMSSM_00`) | Space | 98380 | sk_dash | 1200 | 10000 | 10.0m |
| | Q | 98381 | sk_nightmoon | 6000 | 8000 | 0 |
| | W | 98382 | sk_swing_01, 02, 03 | 8767 | 8000 | 0 |
| | E | 98383 | sk_night | 7333 | 8000 | 0 |
| 레인보우 모코보드 7209 (hoverboard, `MN_PMSHB_00`) | Space | 95722 | sk_feelsgoodspeeding | 733 | 8000 | 5.0m |
| 아우프슈텐-R 8302 (heavywalker_bm9, `MN_PMSHE_00`) | Space | 97300 | sk_powershoulder | 1733 | 10000 | 9.55m |
| | Q | 97310 | sk_hifriend | 5000 | 8000 | 0 |
| | W | 97320 | sk_shyness | 4867 | 8000 | 0 |
| | E | 97330 | sk_cheerup | 2667 | 8000 | 0 |
| 바다 유니콘 튜브 8906 (tube, `MN_PMSUT_00`) | Space | 97730 | sk_summerseason | 1333 | 10000 | 10.0m |
| | Q | 97731 | sk_tubespin | 4000 | 5000 | 0 |
| | W | 97732 | sk_waterjump_01, 02, 03 | 6800 | 8000 | 0 |
| | E | 97733 | sk_waterfall_01, 02-2 | 3867 | 8000 | 0 |
| 고대의 신화 9524 (dragon_2, `MN_PMSDZ_00`) | Space | 98520 | sk_dash | 1200 | 10000 | 10.0m |
| | Q | 98521 | sk_roar | 6000 | 8000 | 0 |
| | W | 98522 | sk_breath_01, 02 | 7000 | 8000 | −0.49m |
| | E | 98523 | sk_look | 5333 | 8000 | 0 |

## 설계

### 데이터 계약

`Data/Vehicles/VehicleProfiles.json` formatVersion 2. 각 vehicle에 `skills` 배열을 더한다.

```json
{
  "skillId": 98520,
  "inputSlot": "SPACE",
  "cooldownMs": 10000,
  "actionDurationMs": 1200,
  "source": { "vehicleColumn": "MovingSkill", "skillTable": "EFTable_Skill", "cooldownColumn": "Cooltime", "action": "MN_PMSDZ_00.loa" },
  "rootMotionSamples": [ { "timeMs": 0, "forward": 0.0, "lateral": 0.0, "up": 0.0 } ]
}
```

- `inputSlot`은 `SPACE|Q|W|E`, 한 탈것 안에서 중복 금지. `rootMotionSamples`는 전진이 없으면 빈 배열.
- 샘플은 `Tools/VehiclePipeline/bake_vehicle_skill_root_motion.py`가 PSA에서 생성한다.
- `Publish-VehicleProfiles.ps1`이 bootstrap v2를 쓴다: `VEHICLE id speed skillCount`, `VEHICLESKILL vehicleId skillId slot cooldownMs durationMs sampleCount packedSamples`.

`Data/Actors/VehicleCatalog.json` formatVersion 2. vehicle에 `skills`를 더한다.

```json
{ "skillId": 98520, "inputSlot": "SPACE", "vehicleClips": ["npc_sk_dash"],
  "riders": [ { "characterClass": "WARLORD", "clips": ["wgl_ride_dragon_2_sk_dash"] } ] }
```

### Shared

- `PLAYER_ACTION_STATE::VEHICLE_SKILL`을 `FEAR` 뒤에 추가한다. snapshot의 `iSkillId`는 탈것 skillId, `iActionStartTick`은 시작 tick.
- 새 enum 값이 wire 계약을 바꾸므로 `NETWORK_PROTOCOL_VERSION` 83→84.
- 명령은 기존 `C2S_USE_SKILL`을 그대로 쓴다. Server가 탑승 여부로 분기한다.

### Server

- `CVehicleCatalog`: v2 파서, `SERVER_VEHICLE_SKILL`(slot, cooldownMs, durationMs, RootMotion), `Find_Skill(vehicleId, skillId)`.
- `GameRoom_VehicleRiding.cpp`
  - `Try_StartVehicleSkill`: `Handle_UseSkill`의 탑승 조기 반환 자리에서 호출. 현재 탈것의 스킬인지, `eAction`이 NONE인지, 쿨타임, 탑승 가능 상태를 확인하고
    `eAction=VEHICLE_SKILL`, `iCurrentSkillId`, `iActionStartTick=tick+1`, 이동 목표 해제, 방향=현재 yaw, 쿨타임 기록.
  - `Update_VehicleSkill`: `Update_Players`에서 `m_PlayerSkillSystem.Update` 직전에 호출. 경과 시간을 올리고 루트 이동 한 tick 분을
    `CPlayerSkillSystem::Clamp_StepToWalkable` + `Resolve_PlayerMove`로 적용, 길이가 끝나면 NONE.
  - `Can_RideVehicle`은 `VEHICLE_SKILL`을 허용한다. 하차(요청·강제)는 진행 중인 탈것 스킬을 끝낸다.
- 계약 테스트 `ServerGameplayContractTests_VehicleRiding.cpp`: 탑승 중 탈것 스킬 시작·쿨타임 거부·대시 전진·종료·다른 탈것 스킬 거부·하차 시 종료.

### Client

- `CActorCatalog`: v2 파서, `VEHICLE_SKILL_ENTRY`(skillId, inputSlot, vehicleClips, 직업별 rider clips).
- `CPlayerController`: 탑승 중 Space/Q/W/E 누름 → 현재 탈것 skillId로 `Request_UseSkill`. R 누름 → 하차 요청(H와 같은 경로).
- `CCharacter::Apply_NetworkAction`: `VEHICLE_SKILL` 분기. 행동 나이로 탑승자 체인과 탈것 체인의 현재 클립·시간을 계산해 둘 다 맞춘다.
  행동 종료 시 기존 복귀 목록에 넣어 locomotion으로 돌아간다. `Set_Locomotion`은 탈것 스킬 중 무시.
- `CPart_Vehicle`: `b_root` 수평 루트 이동 억제, `Seek_SkillClip(clip, seconds)`, `Resume_Locomotion()`.

### 리소스

- 탑승자 애니셋 7모드 × 4직업을 idle/run + 스킬 클립으로 다시 굽는다(파일 이름 유지, attach 검사).
- 탈것 wmodel은 이미 `npc_sk_*` 클립을 포함한다(재쿠킹 없음).

## G 목록

| G | 내용 | 파일 |
|---|---|---|
| G01 | 루트 이동 bake 도구와 데이터 v2 | `Tools/VehiclePipeline/bake_vehicle_skill_root_motion.py`(신규), `Data/Vehicles/VehicleProfiles.json`, `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`, `Data/Actors/VehicleCatalog.json` |
| G02 | Shared enum·protocol | `Shared/Public/Network/PacketMessages.h`, `Shared/Public/Network/PacketType.h`, 하네스 기대값 |
| G03 | Server 카탈로그·스킬 실행·계약 테스트 | `Server/Public/VehicleCatalog.h`, `Server/Private/VehicleCatalog.cpp`, `Server/Public/GameRoom.h`, `Server/Private/GameRoom_VehicleRiding.cpp`, `Server/Private/GameRoom_PlayerCommands.cpp`, `Server/Private/GameRoom_PlayerSimulation.cpp`, `Server/Private/ServerGameplayContractTests_VehicleRiding.cpp` |
| G04 | 탑승자 애니셋 재쿠킹 | Resources `Character/<Class>/AnimSets/<Class>_Ride*AnimSet.wmodel` 28개 |
| G05 | Client 카탈로그·입력·연출 | `Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp`, `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp`, `Client/Public/Character.h`, `Client/Private/Character.cpp`, `Client/Public/Part_Vehicle.h`, `Client/Private/Part_Vehicle.cpp` |
| G06 | 문서·검증 | RESULT, `CLAUDE.md` 탈것 문단, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` protocol·입력 표 |

## 검증

| 검사 | 기준 |
|---|---|
| JSON parse, `Publish-VehicleProfiles.ps1` Validate→Publish | bootstrap v2, 7 vehicle, 22 skill |
| 루트 이동 bake | 대시 최종 전진이 위 표와 1cm 이내 |
| 탑승자 애니셋 `compare_attach.py` | 28개 OK, 스킬 클립 존재 |
| Product Debug 빌드 | Engine/Shared/Server/Client PASS |
| `NetworkProtocolHarness` | failures 0 |
| `Server.exe --vehicle-riding-contract-test` | failures 0 |
| 사용자 화면 확인 | 탈것별 Space 전진, Q/W/E 클립, 쿨타임 중 재입력 무반응, R 하차 |

## 범위 밖

- 스킬 파티클·사운드·카메라, 랩터 달리기 속도 버프(Type 5), 튜브 추가 키 입력 분기, 탈것 스킬 HUD.
