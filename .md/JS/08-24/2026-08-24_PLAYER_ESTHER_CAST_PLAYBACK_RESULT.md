# 2026-08-24 플레이어 에스더 스킬 캐스팅 애니메이션 재생 연결 RESULT

작성자: JS · 2026-08-24

## 1. 목표

플레이어가 에스더 스킬(실리안/웨이/바훈투르 어느 슬롯이든)을 사용하면 캐스터가
어제 부착한 `act_estherskill_1` 캐스팅 애니메이션(45프레임@30fps, 1.5초)을
재생한다. Server 권위 계약 유지: Client는 snapshot으로만 표현한다.

## 2. 설계

에스더 호출은 balance 스킬이 아니므로(스킬 ID·쿨다운·데미지 정본 없음, 게이지가
정본) `PLAYER_ACTION_STATE`에 전용 상태 `ESTHER_CAST`를 추가했다. FALLING 추가와
같은 wire 규칙(마지막에 append, skillId 없음, actionStartTick 필수)이다.

- Server: `Handle_UseEstherSkill` 수락 시 캐스터를 `ESTHER_CAST`로 잠그고 조준
  방향으로 회전. `Update_Players`가 `ESTHER_CAST_DURATION_MS`(1500ms=45틱) 경과 후
  NONE으로 해제. 캐스트 중 이동(`Handle_Move`의 기존 eAction != NONE 거부)과 스킬
  (`Try_StartInternal`의 NONE 요구), 에스더 재사용(신규 NONE guard)이 모두 차단된다.
  넉백/넉다운/사망은 기존 경로가 그대로 캐스트를 중단시킨다.
- Client: `CHARACTER_ANIM::ESTHER_CAST` 추가, 6클래스 spec의 `AnimationClips`에
  클립명 등록(4직업은 애니셋 클립, Gunslinger/Slayer는 nullptr → 표현만 격리).
  `CCharacter::Apply_NetworkAction`에 ESTHER_CAST 분기: `Start_Clip`으로 0프레임부터
  재생(재사용 시 되감기 보장), 클립 없으면 locomotion 유지. NONE 복귀 시 기존
  SKILL 복귀 분기를 공유해 idle/run으로 돌아온다. `Commit_Locomotion`에 캐스트 중
  지연 idle commit이 클립을 덮지 않는 guard 추가.

## 3. 변경 파일

| 파일 | 내용 |
|---|---|
| `Shared/Public/Network/PacketType.h` | `NETWORK_PROTOCOL_VERSION` 32 → 33 |
| `Shared/Public/Network/PacketMessages.h` | `PLAYER_ACTION_STATE::ESTHER_CAST` append |
| `Shared/Private/Network/PacketMessages.cpp` | snapshot 검증: ESTHER_CAST는 skillId 없음 + startTick 필수 |
| `Server/Public/EstherSkillSystem.h` | `ESTHER_CAST_DURATION_MS = 1500` |
| `Server/Private/GameRoom.cpp` | 수락 시 캐스트 잠금·조준 회전, NONE guard, `Update_Players` 만료 해제, `ESTHER_CAST_TICKS` |
| `Client/Public/CharacterSpec.h` | `CHARACTER_ANIM::ESTHER_CAST` |
| `Client/Private/Logic_{LanceMaster,Warlord,Artist,DimensionMaster}.cpp` | 캐스팅 클립명 (`flm/wgl/sdm/pc_sp_m_00_sk` + `_act_estherskill_1`) |
| `Client/Private/Logic_{GunSlinger,Slayer}.cpp` | nullptr (애니셋 미부착) |
| `Client/Private/Character.cpp` | ESTHER_CAST 재생 분기, NONE 복귀, locomotion guard |
| `Tools/NetworkProtocolHarness/...` | V33 pin, ESTHER_CAST roundtrip + 2 거부 케이스 |
| `Server/Private/ServerGameplayContractTests.cpp` | 캐스트 잠금·조준 회전·캐스트 중 재사용 거부·만료 해제 assert, 연속 사용 사이 해제 스캐폴딩 |

새 파일 없음 → vcxproj/filters 변경 없음.

## 4. 자동 검증 (실행함)

- Shared, NetworkProtocolHarness, Server, Client x64 Debug 빌드 exit 0.
- `NetworkProtocolHarness.exe` failures 0. 신규 PASS: `Esther Cast Player Snapshot
  Round Trip`, `Reject An Esther Cast That Carries A Skill`, `Reject An Esther Cast
  Without A Start Tick`, `World Destruction Packet Types At Protocol V33`.
- `Server.exe --contract-test` failures 0. 신규 PASS: `Lock the caster into the
  Esther call turned to the aim`, `Reject an Esther use while the caster is still
  casting`, `Release the caster to NONE once the call clip has run out`.
- `git diff --check` 통과.

## 5. 수동 검증 (사용자)

Valtan Arena에서 게이지 충전 후 에스더 슬롯 사용 → 캐스터가 조준 방향을 보며 손을
드는 캐스팅 1.5초 재생, 그동안 이동/스킬 잠김, 이후 idle 복귀와 소환 확인.
다른 파티원 화면에서도 같은 캐스팅이 보여야 한다(스냅샷 브로드캐스트).

## 6. 남은 것

- Gunslinger/Slayer 에스더 애니셋 미쿠킹(내 4직업 범위 밖) — 캐스트 시 자세 유지로 격리됨.
- 손 이펙트(`Par_D_Esther_call_11`, `FX_R_Hand`, ~0.35s)와 사운드(`PC_COMMON_ESTHER`)는
  이펙트 담당 인계 정보 그대로 미구현.
- protocol v33: 팀원 전원 재빌드 필요(구버전 Client는 접속 거부).
