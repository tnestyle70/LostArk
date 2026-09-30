# 2026-09-29 워터팡 물총 Q/W/E/R 스킬 RESULT

## 요청

마하라카 워터팡 경기 중(Server가 무장시킨 동안) Q/W/E/R이 직업 스킬 대신 물총 전용 스킬로 동작하고, 이동을 잠그지 않은 채 쏠 수 있어야 한다. 경기 밖, 다른 월드, 다른 키의 기존 동작은 바뀌면 안 된다.

## 원작에서 확인한 것 (사실)

출처는 모두 `C:/LostArkExtract/MaharakaFunctions20260926/db`(복호화된 EFTable)와 `data1.lpk`의 `XMLData/Projectile`, 기존 `GADGET.loa` 추출이다.

- 물총 프롭 `EFTable_Prop 15000`(워터 프로 MK-1)은 기본 공격 `DefaultSkillId 56930`과 스킬 3개 `SkillId0/1/2 = 56900/56910/56920`을 가진다. `MoveSkillEnable = 1`이라 이동 중 사용이 원작 설정이다. MK-2(15010)는 `57030 / 57000 / 57010 / 57020`이며 모델 ITR_02165는 설치돼 있지 않다.
- 이름과 설명(GameMsg): 56900 연발 샷("강력한 수압의 물총을 연발로 발사한다"), 56910 물 폭탄("전방에 물 폭탄을 투하 한다"), 56920 이동속도 증가("이동속도가 일시적으로 상승한다"). 56930은 이름이 없는 기본 공격이다.
- `EFTable_Skill`: 56900 쿨타임 3000ms·사거리 7m, 56910 쿨타임 8000ms·사거리 7m, 56920 쿨타임 7000ms(자기 버프 569200), 56930 쿨타임 없음·사거리 4m.
- GADGET.loa의 `[마하라카]` 변형 액션(스킬 id + 2): 56902(Q스킬 연발 발사, 클립 Att_2, 1.8s), 56912(W스킬 전방 물폭탄, 클립 Att_5, 1.0s), 56932(단발형 평타, 클립 Att_1, 1.0s). 발사체를 만드는 "Effect" 알림 시각은 각각 0.694s, 0.200s, 0.402s이다. 56920에는 GADGET 액션이 없다(즉시 버프).
- 소리 알림: 56932 Cast1 t0, Attack3_Shot1 t0.2 / 56902 Cast1 t0, Attack1_Shot1 t0.55 / 56912 Enviska1_Attack1_Shot1 t0.1.
- 발사체(`Projectile 569020 / 569320`)는 직선 미사일, 반경 75cm, 수명 1.5s, 속도 420cm/s(연발)와 300cm/s(평타)다. 물 폭탄 `569120`은 수류탄형이며 속도 800cm/s다.
- 피격 행(SkillEffect): 연발 569021·평타 569321은 반경 100cm, 밀림 16~20cm/5ms. 물 폭탄 569121은 반경 105cm, 밀림 15~25cm/150~250ms.
- 버프 569200(이동속도 증가)은 5초, `PassiveOptionValue 3000`이고 설명문 매크로가 값을 100으로 나누므로 +30%다.

## 원작에 있으나 적용하지 않은 것

- 피격 행에는 HP 피해(연발 30000, 평타 7500~8500, 물 폭탄 약 15만)와 "냉기" 중첩(버프 569302: 중첩마다 이동 속도 감소, 6~7중첩 동결, 8중첩 사망)이 있다. 플레이어 최대 HP는 Retail 프로필에서 132000이라 이 수치를 그대로 쓰면 물 폭탄 한 발로 즉사한다. 원작의 워터팡 스탯 보정(PvP 조정)은 추출 데이터에 없어서 **피해와 냉기 중첩은 구현하지 않았다.** 지금 피격은 원작 행의 밀림과 경직(기존 `Apply_WorldToPlayer` 경로)만 한다. 냉기 중첩과 8중첩 사망은 새 플레이어 상태와 복제가 필요한 별도 작업이고 사용자 결정이 필요하다.
- 물 폭탄의 날아가는 물덩이·폭발 파티클(`Par_L_WaterGun01_Sk_02`, `_02_1`)과 연발/평타의 발사체·피격 파티클(`Par_L_WaterGun01_Sk_01_1/_2/_3`, `_03_1/_2`)은 아직 복원되지 않았다. 지금 화면에 보이는 것은 총구 이펙트 4종(이전 작업)뿐이다. 물 폭탄 발사에는 이펙트가 없다.
- 발사체 피격음(`ProjExp1`, `Attack2_Proj1`, `Attack2_ProjExp1`)은 wav로 추출만 해 두고 연결하지 않았다(피격 시점을 클라이언트로 복제하는 이벤트가 없다).
- HUD의 퀵슬롯 아이콘과 쿨타임 표시는 바꾸지 않았다. 무장 중에도 Q/W/E/R 자리에 직업 스킬 아이콘이 보인다.

## 슬롯 배치 (추론)

원작은 기본 공격(클릭) 1개와 스킬 3개다. 요청이 Q/W/E/R 네 칸이라 원작 순서를 따라 Q 연발 샷, W 물 폭탄, E 이동속도 증가, R 기본 사격으로 배정했다. 마우스 왼쪽 클릭은 지금처럼 직업 기본 공격이다(바꾸지 않았다). 바꾸려면 `MaharakaWaterpangContract.h`의 표에서 `cInputSlot`만 고치면 된다.

## 구현

수직 슬라이스: Shared 표 → protocol 122 → Server 판정 → Client 입력·표현. 새 런타임 경로는 만들지 않았고 기존 `C2S_USE_SKILL`을 그대로 쓴다.

- `Shared/Public/Gameplay/MaharakaWaterpangContract.h`: 물총 스킬표(`MAHARAKA_WATERGUN_SKILLS` 4행), 조회 함수, tick 변환. 원작 수치는 모두 이 표에 있고 이유 주석이 붙어 있다.
- `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`: `PLAYER_SNAPSHOT.iWaterGunSkillId`, `iWaterGunCastTick`(기존 `isWaterpangArmed` 바로 뒤). 둘은 함께 0이거나 함께 0이 아니어야 한다는 검증을 추가했다.
- `Shared/Public/Network/PacketType.h`: `NETWORK_PROTOCOL_VERSION` 121 → **122**.
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`: 프로토콜 번호 검사 11곳을 121u → 122u로 수정(문구 문자열은 그대로).
- `Server/Public/ServerPlayer.h`: 마지막 발사, 발사 잠금 끝, 속도 버프 끝과 배율.
- `Server/Public/GameRoom.h`, `Server/Private/GameRoom_PartyWorld.cpp`: `Is_MaharakaWaterpangArmed`(스냅샷에 인라인이던 무장 규칙을 함수로 뺌), `Try_StartMaharakaWaterGunSkill`, `Update_MaharakaWaterGunShots`, 발사체 저장소.
  - 발사: 무장·생존·`eAction == NONE`·시퀀스·쿨타임·이전 발사 액션 길이를 검사한다. **이동 목표는 지우지 않고 `eAction`도 바꾸지 않는다.** 서 있으면 조준 방향으로 돌고, 걷는 중이면 이동 방향을 유지한다.
  - 발사체: 원작 "Effect" 시각에 캐스터의 그 시점 방향으로 출발한다. 연발·평타는 직선(첫 몸에 소진), 물 폭탄은 조준 지점(최대 7m)에 떨어질 때 원형으로 터진다. 무장 중인 다른 플레이어만 맞는다.
  - 피격: `Apply_WorldToPlayer`로 원작 밀림·경직만 준다(피해 0).
  - E: 버프 5초, 이동속도 ×1.3(`Resolve_PlayerMoveSpeed` 한 곳이라 스냅샷 `fMoveSpeed`와 이동이 함께 바뀐다).
- `Server/Private/GameRoom_PlayerCommands.cpp`: `Execute_PlayerSkill`에서 마하라카·무장 상태일 때만 물총 표의 skill id는 물총 경로로 보내고, 직업 스킬 중 Q/W/E/R은 거부한다. 다른 키(A S D F T V ALT_V Z X, 마우스 왼쪽 클릭)와 다른 월드·비무장 상태는 기존 경로 그대로다. 직업 변경 초기화에 물총 상태도 넣었다.
- `Server/Private/GameRoom_Replication.cpp`: 무장 함수 호출과 새 필드 채움.
- `Server/Private/GameRoom_VehicleRiding.cpp`: 속도 버프 배율(마하라카에서만).
- `Server/Private/GameRoom.cpp`: 매 tick `Update_MaharakaWaterGunShots` 호출. `GameRoom_WorldEntities.cpp`: 빈 방 리셋에서 발사체 비우기.
- `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp`: `isWaterpangArmed`를 로컬 플레이어 상태에 넣음.
- `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp`: `Poll_WaterGunSlots`(Q/W/E/R → 표의 skill id, 조준은 기존 지면 피킹). 무장 중에는 직업 퀵슬롯 표에서 Q/W/E/R 네 칸만 건너뛴다.
- `Client/Public/Character.h`, `Client/Private/Character.cpp`: `Play_WaterGunAttack`(디버그 전용이던 발사 미리보기를 릴리스에서도 쓰도록 일반화, 미리보기는 이를 호출), `Apply_NetworkWaterGunCast`(새 cast tick마다 한 번만, 이미 지난 발사는 다시 재생하지 않음), `Update_WaterGunCast`(발사 클립이 끝나면 이동 자세로 복귀, 원작 소리 알림 지연 재생). 발사 클립이 도는 동안 이동 정지/시작 edge가 클립을 끊지 않도록 `Commit_Locomotion`에 조건을 하나 추가했다.
- `Client/Private/ClientReplication.cpp`: 스냅샷마다 `Apply_NetworkWaterGunCast` 호출.
- `CLAUDE.md`: 물총 Q/W/E/R과 protocol 122 한 문단.

## 리소스 (Git 추적 없음)

`Tools/SoundPipeline/render_events.py --filter PC_COMMON`으로 원작 Wwise 이벤트 4개를 wav로 추출해 각각 3종 변형(총 12개)을 설치했다.

- `Client/Bin/Resources/Sound/Maharaka/WaterGun/Gadget_WaterPistol1_Attack1_Cast1.variant01~03.wav`
- `.../Gadget_WaterPistol1_Attack1_Shot1.variant01~03.wav`
- `.../Gadget_WaterPistol1_Attack3_Shot1.variant01~03.wav`
- `.../Gadget_Enviska1_Attack1_Shot1.variant01~03.wav`
- 같은 상대 경로로 `C:\Users\USER\OneDrive\바탕 화면\CY_Resources\Sound\Maharaka\WaterGun\`에도 복사했다. **팀원 PC에는 이 폴더 전달이 필요하다.** 없으면 발사 소리만 나지 않는다(클립·이펙트·판정은 영향 없음).
- `Data/Sound/CharacterSoundCatalog.json`은 **수정하지 않았다.** 이 파일의 해시가 발탄 프레젠테이션 세대(`Client/Private/ValtanPresentationGenerationAdmission.cpp`, `valtan_presentation_generation.py`)의 고정 입력이라, 고치면 발탄 게시 세대가 어긋난다. 소리 경로는 Resources 상대 ID로 `Character.cpp`에 두었다(물총 모델 경로와 같은 방식).

## 실행한 검증

- 구문 검사(`cl /Zs`, 출력물 없음, MSBuild·Product runner 아님): Server 6개 파일(`GameRoom_PartyWorld`, `_PlayerCommands`, `_Replication`, `_VehicleRiding`, `GameRoom.cpp`, `_WorldEntities`), Client 4개 파일(`Character`, `PlayerController`, `ClientReplication`, `CombatHUDViewModel`), `Shared/Private/Network/PacketMessages.cpp`, `NetworkProtocolHarness.cpp` 모두 오류 0. Client는 `UNICODE`, 표준 PCH, `EngineSDK/Inc`를 프로젝트 설정에 맞춰 넣었다.
- 수정한 파일 전체 `git diff --check`: 공백 오류 없음.
- 파일별 원래 인코딩·줄바꿈 보존: 모든 패치가 앵커 일치 1건일 때만 적용되는 바이트 패치이며 CRLF/LF를 유지했다(`PlayerController.cpp`는 원래 LF).
- 패치 중 실수 1건과 수정: 도구 입력에서 백슬래시가 줄어 `'\0'`이 실제 NUL 바이트로 들어갔다. 발견 즉시 `chr(92)`로 바로잡고 NUL 0개를 확인했다.

## 실행하지 않은 것

- Engine/Client/Server 빌드, 링크: 하지 않았다(사용자가 VS에서 한 번에 빌드).
- Client/Server 실행, 화면 확인, 소리 청취: 하지 않았다. **visual PASS가 아니다.**
- `NetworkProtocolHarness`, `Server.exe --contract-test` 실행: 하지 않았다(구문 검사만).
- publisher: 이번 변경은 게시 대상 데이터(`Data/Balance`, `Data/Animation`, `Data/Sound`, `Data/Effects`)를 바꾸지 않아 **실행할 publisher가 없다.** 판정 표는 Shared 헤더에 컴파일되어 있으므로 Server 실행 파일과 Client 실행 파일 재빌드만 필요하다.

## 사용자 확인 필요

1. 마하라카 워터팡 경기 시작 후 Q/W/E/R을 눌러 서서 쏘기, 달리면서 쏘기, 물 폭탄, 이동속도 증가(달리기 속도 +30% 5초)가 되는지.
2. 직업 A/S/D/F 등 다른 키가 그대로 동작하는지, 경기 밖에서 Q/W/E/R이 직업 스킬로 돌아오는지.
3. 발사 소리 타이밍(원작 알림 시각을 그대로 썼다).
4. 다른 플레이어를 맞췄을 때 짧은 경직·밀림만 생기는 것이 원하는 결과인지. HP 피해와 냉기 중첩을 넣을지는 별도 결정이다.

## 다음 명령

Server와 Client는 protocol 122라서 **둘 다 다시 빌드하고 다시 시작**해야 한다. VS를 닫고 Debug 제품 빌드를 한 번 돌린다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

(또는 VS에서 `Debug / x64` 솔루션 빌드 후 `Server + Client` 실행.) 다른 PC에는 `CY_Resources/Sound/Maharaka/WaterGun`을 같은 경로로 전달한다.
