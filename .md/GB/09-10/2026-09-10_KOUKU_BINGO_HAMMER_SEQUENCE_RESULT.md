# 쿠크 빙고 거대망치를 World Sequence로 옮김 — RESULT (2026-09-10)

사용자 요청: "거대망치소환을 시퀀스로 만들어서 볼 수 있게 해줘".

## 1. 옮기기 전 상태

망치는 시퀀스가 아니었다. 서버가 위상과 시계를 갖고 클라가 매 프레임 C++로 자세를 조립했다.

- 서버 `CKoukuBingoRuntime::Advance_Hammer`: `RAISED 2000ms → DESCENDING 1400ms → SWEEPING 1600ms`,
  앵커는 20개(행·열 10줄 × 양끝) 중 하나를 `Handle_DebugBingoHammer`가 굴린다. 대각선은 쓸지 않는다.
- 프로토콜 75가 `BINGO_HAMMER_SNAPSHOT`(앵커, 위상, 위상 시작/끝 틱)을 복제한다.
- 클라 `CKoukuSaydonPresentationPlayer`가 그 위상을 받아 아래 식으로 pivot을 만들고
  Effect V2 그룹 `bingo.hammer`를 그 자리에 붙였다.

```
pivot = RotationZ(-lean) * RotationY(atan2(-dz, dx)) * Translation(x, y, z)
```

즉 낙하 높이 9m, 기울기 40도, 기울기가 차오르는 구간 30% 같은 값이 전부 Shared 상수와 C++ 보간에
박혀 있었다. 바꾸려면 매번 리빌드해야 했다.

**중요: 스윕에는 게임플레이 판정이 없다.** `Advance_Hammer`는 위상만 넘기고 끝나며 `Is_Safe`는
계약 테스트 외에 소비자가 없다. 순수 연출이므로 표현 경로를 바꿔도 판정이 어긋날 것이 없다.

## 2. 계약 조사 — 왜 시퀀스로 옮길 수 있는가

옮기기 전에 다음을 코드에서 확인했다.

- `S2C_WORLD_SEQUENCE_PLAY`는 `PLAY/REPLAY/STOP/STOP_OWNER/FINISH_OWNER`와 런타임 placement,
  `iStartTick`, `iDurationMs`, `fPlaybackSpeed`를 싣는다.
- 방 수준 송신 경로 `CGameRoom::Broadcast_WorldSequencePlay(instanceId, ...)`가 이미 있다.
  PLAY 때 `Place_PartyForCutscene`이 걸리지만 그 함수는 `world.sequence.instance.original_kouku`
  하나에만 반응하므로 망치에는 부작용이 없다.
- 클라 `Consume_WorldSequencePlays` 루프는 `iRunEpoch == 0`이고 target이 비면
  `Start_ServerRequestedSequence`로 보내며, 거기에 instance allow-list가 없다.
- World Object 인스턴스는 `position`은 갖지만 **회전 필드가 없다**. 따라서 스윕 방향은 인스턴스가
  아니라 템플릿(트랙 키 쿼터니언)에 들어가야 한다.
- 오브젝트 월드 행렬 합성은 `Scale(resource.scale × key.scale) · Rot(key.quat) · Translate(instance.position + key.offset)`이며,
  이는 위 참조 식과 형태가 같다. 그래서 키에 그대로 구울 수 있다.
- 키 쿼터니언은 단위이고 `w >= 0`이어야 한다. 보간은 `XMQuaternionSlerp`라 최단호를 타므로
  반구가 갈려도 문제가 없다.
- 상한: 템플릿 256, 인스턴스 2048, 키 256. 현재 149/185이므로 여유가 있다.

## 3. 저작한 것

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` (revision 423 → 425)

- **오브젝트 리소스 1개** `world.object.kouku.bingo_hammer`
  - 모델 `Character/KoukuSaton/MN_UMAC_01/MN_UMAC_01.wmodel`, diffuse `mn_umac_01_d.dds`
  - `modelPreScale 0.01`, `scale 1.8`, `animated false`
  - 실측: 본 0개·stride 48의 **정적 메시**이고 크기 260.6 × 1568.6 × 178.6 units.
    ×0.01×1.8 하면 4.69 × 28.24 × 3.21 m다. Y가 −0.03에서 시작하므로 **머리가 원점이고 사슬이
    위로 28m 뻗는다** — 클라 주석이 말하던 그대로다. 배율은 기존 Effect V2 문서와 같은 값이다.
- **템플릿 4개** `sequence.kouku.bingo.hammer.{x_plus,x_minus,z_plus,z_minus}`
  - 스윕이 실제로 향하는 월드 방향은 네 가지뿐이라 방향을 템플릿에 굽는다.
  - `durationMs 5000`, `interpolation LINEAR`, 트랙 1개, 키 36개.
  - 키는 위 참조 식을 그 시각에 그대로 평가해 뽑았다. 직선 구간은 키 두 개면 되고,
    스무스스텝인 낙하와 기울기 구간만 촘촘히(낙하 20등분, 기울기 12등분) 샘플했다.
  - 마지막 키(5000ms)는 `visible false`로 인스턴스를 끝낸다.
- **인스턴스 20개** `world.sequence.instance.kouku.bingo.hammer.anchor.<0..19>`
  - 앵커의 시작 좌표를 `position`에 넣고, 그 앵커의 진행 방향 템플릿을 참조한다.
  - 시작·도착 좌표는 `Kouku_BingoHammerPath`를 그대로 옮겨 계산했다. 스윕 거리 18.24m.

## 4. 바꾼 코드

| 파일 | 내용 |
|---|---|
| `Server/Private/GameRoom.cpp` | `Handle_DebugBingoHammer`가 `Start_Hammer` 성공 시 굴린 앵커의 인스턴스를 `Broadcast_WorldSequencePlay`로 쏜다 |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | 손으로 pivot을 만들던 망치 블록(61줄) 제거 |
| `Client/Public/KoukuSaydonPresentationPlayer.h` | 쓰이지 않게 된 `m_BingoHammer` 핸들 제거 |

서버의 위상 시계는 **그대로 뒀다.** 시퀀스는 그 5초를 보여 주는 표현이고 판정 주체가 아니다.
`bingo.hammer` Effect V2 문서와 그룹은 지우지 않았다(다른 데서 참조하지 않지만 되돌리기 쉽게 남김).

## 5. 검증한 것

- **움직임 동일성**: 저장된 문서를 다시 읽어 엔진과 같은 방식으로 샘플링(키 사이 선형 보간 +
  최단호 slerp + `Rot·Translate` 합성)한 뒤, 클라의 원래 C++ 식과 **20개 앵커 × 5초를 10ms 간격으로**
  대조했다.
  - 최대 위치 오차 **0.01574 m**
  - 최대 회전 오차 **0.00333** (행렬 원소, 약 0.19도)
  - 두 오차 모두 스무스스텝 구간을 직선으로 이은 데서 나오며, 키를 8등분에서 20등분으로
    늘려 0.092m/0.0121에서 위 값으로 줄였다.
- 도착 좌표가 `Kouku_BingoHammerPath`의 끝점과 정확히 일치하고, 마지막 키가 `visible false`다.
- 키 쿼터니언 전부 단위이고 `w >= 0` (위반 0건).
- `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Validate` **exit 0** (쓰기 없는 읽기 전용 경로).
- 바꾼 두 translation unit 구문 검사(`cl /Zs`, 출력물 생성 없음) 오류 0:
  `Server/Private/GameRoom.cpp`, `Client/Private/KoukuSaydonPresentationPlayer.cpp`.

## 6. 아직 하지 않은 것

- **publish는 사용자 빌드가 대신 했다.** 작성 시점에는 Visual Studio가 열려 있어 publisher를 겹쳐
  돌리지 않았는데, 그 사이 사용자가 VS에서 빌드를 돌리면서 pre-build publisher가
  `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json`을 revision 425로 갱신했다
  (11:53). 망치 템플릿 4개와 인스턴스 20개가 런타임에 들어가 있는 것을 확인했다. 같은 빌드가
  11:50에 쿠크 Composition Product도 다시 만들어, 그 전까지 stale이던
  `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`도 해소됐다(sourceRevision 230).
  8장의 폭탄(revision 426)은 아직 런타임에 없으므로 **다시 빌드하면 같은 경로로 함께 올라간다.**
  수동으로 하려면 아래를 쓴다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
```

- **빌드·화면 확인 미실행.** Server와 Client 둘 다 바꿨으므로 함께 빌드하고 재시작해야 한다.
  어떤 항목도 visual PASS가 아니다. 확인 경로는 쿠크 아레나 진입 → F1 → `Bingo_Hammer`.
- 폭탄(머리 위 마크 / 심어진 폭탄)은 이번 범위가 아니다. 머리 위 마크는 World Object의 PLAYER
  앵커가 방 안 플레이어 전원에게 복제되므로 대상 지정 필드 없이는 옮길 수 없다.

## 7. 이제 바꿀 수 있게 된 것

낙하 높이, 낙하 곡선, 체공 시간, 스윕 속도, 기울기 각도와 기울기가 차오르는 구간은 이제
`sequence.kouku.bingo.hammer.*` 템플릿의 트랙 키다. 툴에서 보면서 고칠 수 있고 리빌드가 필요 없다.
다만 **템플릿 전체 길이 5000ms는 서버 위상 합(2000+1400+1600)과 같아야 한다.** 어긋나면 화면과
서버 시계가 따로 논다. 서버 쪽을 바꾸려면 Shared 상수와 템플릿을 같이 고쳐야 한다.

## 8. 심어진 폭탄도 같은 방식으로 옮김

망치를 끝낸 뒤 사용자가 폭탄도 요청해서 같은 세션에서 이어 작업했다.

### 8.1 두 단계 중 하나만 옮긴다

폭탄은 한 물건의 두 단계다.

- **MARKED(3초, 머리 위)** — 옮기지 않았다. 지정된 한 사람을 매 프레임 따라다녀야 하는데 World
  Object의 PLAYER 앵커는 `targets.playerAnchors()`가 돌려주는 **살아 있는 플레이어 전원**에게
  앵커마다 오브젝트를 하나씩 만든다(`WorldSequencePlayer_Objects.cpp` 331행). 큐에도 인스턴스에도
  대상 지정 필드가 없고, 캐리어가 죽으면 서버가 마크를 취소하므로 고정 길이 타임라인과 성격도 맞지
  않는다. 그대로 Effect V2 `bingo.bomb.mark`로 남겼다.
- **PLANTED(도화선 2초)** — 옮겼다. 심는 순간 자리가 고정되고 회전도 필요 없다.

### 8.2 저작

`LV_LUT_MIDNIGHTC_ED.worldsequences.json` (revision 425 → 426)

- 오브젝트 리소스는 **이미 있던 것을 재사용**했다. `world.object.kouku.bingo_bomb`
  (`MN_RHCN_01.wmodel`, `modelPreScale 0.01`, `scale 2.0`, `animated true`)이며 이는 기존 Effect V2
  문서 `bingo.bomb_1`이 쓰던 모델·배율과 같은 값이다. 이 리소스는 그동안 Composition 어디에도
  연결되지 않은 잔재였다.
- **템플릿 1개** `sequence.kouku.bingo.bomb.planted`
  - `durationMs 2000` = `KOUKU_BINGO_BOMB_FUSE_MS`, `interpolation LINEAR`
  - 키 3개: 0ms 원점 visible, 1999ms 원점 visible, 2000ms visible false
  - animationTrack `Bomb_idle_normal_1` 루프 (모델이 가진 클립: `Bomb_att_battle_1_01`,
    `Bomb_dead_1`, `Bomb_idle_battle_1`, `Bomb_idle_normal_1`, `Bomb_respawn_1`)
  - 툴이 만든 기본 모션 `sequence.LV_LUT_MIDNIGHTC_ED.world_object.bingo_bomb`을 그대로 쓰지 않고
    전용 템플릿을 만들었다. 그건 World Object Tool의 기본 모션이라, 누가 툴에서 그걸 고치면 기믹이
    같이 바뀌어 버린다.
- **인스턴스 4개** `world.sequence.instance.kouku.bingo.bomb.planted.slot.<0..3>`
  - 전부 `position [0,0,0]`. 심은 좌표는 서버가 큐의 position offset으로 보내고, 런타임이 그것을
    오브젝트 월드 이동에 더한다(`WorldSequencePlayer_Objects.cpp` 291행).
  - **슬롯마다 인스턴스가 필요하다.** `CWorldSequencePlayer::Play`는 같은 instanceId가 이미 활성이면
    새로 만들지 않고 **제자리에서 재시작**하며 offset을 덮어쓴다. 서버는 폭탄을 최대 4개
    (`KOUKU_BINGO_MAX_BOMBS`) 동시에 들 수 있으므로 인스턴스를 공유하면 두 번째 폭탄이 첫 번째의
    화면을 빼앗는다.

### 8.3 바꾼 코드

| 파일 | 내용 |
|---|---|
| `Server/Private/GameRoom.cpp` | `Update_KoukuBingo`가 `Plant_Bomb` 직후 그 슬롯의 인스턴스를 심은 좌표를 offset으로 실어 `Broadcast_WorldSequencePlay`로 쏜다 |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | 폭탄 루프의 PLANTED 분기 제거, 블록 주석을 현재 계약으로 교정 |

인계는 기존 구조가 알아서 한다. 그 루프는 이번 프레임에 살아 있는 슬롯 목록을 만들고 거기 없는 항목을
정리하는데, PLANTED가 `else continue`로 빠지면서 슬롯이 목록에서 제외되어 머리 위 마크가 자동으로
걷힌다. 같은 틱에 서버 브로드캐스트가 나가므로 마크가 사라지는 프레임에 시퀀스가 시작된다.

### 8.4 검증

- `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Validate` **exit 0** (쓰기 없는 경로)
- 바꾼 두 translation unit 구문 검사(`cl /Zs`) 오류 0

### 8.5 남긴 것과 이제 할 수 있는 것

- Effect V2 문서 `bingo.bomb_1`과 그룹 `bingo.bomb`은 **지우지 않았다.** 이제 참조하는 코드가 없지만
  사용자가 저작한 데이터이고 되돌리기 쉬우므로 삭제는 별도 판단으로 둔다. `bingo.bomb.mark` 쪽은
  계속 사용 중이다.
- 지금은 머리 위에서 폭탄이 사라졌다가 바닥에 나타난다. 마크가 떠 있던 자리와 심기는 자리가 XZ가
  같으므로, 템플릿 첫 키를 `(0, 2.4, 0)`에서 시작해 바닥으로 떨어뜨리면 낙하가 이어진다. 키 두어 개다.
- 폭발 연출도 없다. 모델이 `Bomb_att_battle_1_01`과 `Bomb_dead_1`을 갖고 있고 인스턴스에 `nextMotionId`
  필드가 있으므로 저작으로 이을 수 있다. 이번에는 요청 범위가 아니라 넣지 않았다.
- **템플릿 길이 2000ms는 서버 `KOUKU_BINGO_BOMB_FUSE_MS`와 같아야 한다.** 한쪽만 바꾸면 화면과 서버
  시계가 어긋난다.
