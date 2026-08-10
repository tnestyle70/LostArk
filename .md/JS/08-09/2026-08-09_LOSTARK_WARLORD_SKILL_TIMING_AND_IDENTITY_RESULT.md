# 워로드 스킬 타이밍과 아이덴티티 게이지 — 결과

작성자: JS · 2026-08-09 · 브랜치 `feature/warlord-skill-timing` (`feature/artist-skill-timing` 위)

같은 날 도화가 작업(`2026-08-09_LOSTARK_ARTIST_SKILL_TIMING_RESULT.md`)에 이어 워로드를 봤다.
E·X·Z 세 슬롯과, 작업 중 사용자가 지적한 이동 회전 문제를 다뤘다. 도화가에서 세운 타격 판별
규칙이 워로드에서 독립 검증된 것이 이번의 가장 큰 소득이다.

## 1. d=0 EFFECT 규칙이 HIT 노티파이로 검증됐다

도화가는 `kind=HIT`이 2개뿐이라 `d=0.0000`인 EFFECT 버스트를 타격 시각으로 썼다. 근거가 도화가
하나뿐이라 다른 클래스에도 맞는지는 미확인으로 남겨뒀는데, 워로드가 그 답을 줬다.

워로드는 `ParticleHit`을 34개 갖고 있고 그 시각이 d=0 버스트와 겹친다.

| 클립 | HIT | d=0 버스트 |
|---|---|---|
| `wgl_att_battle_1_01` | 300 | 330 |
| `wgl_att_battle_1_02` | 200 | 240 |
| `wgl_sk_pierceingspear` | 500 | 549 |
| `wgl_sk_bash` | 300 | 297~328 |
| `wgl_sk_hookchain` | 400 | 367 |

두 신호가 같은 순간을 가리킨다. **`kind=HIT`이 있으면 그것을 쓰고, 없으면 d=0 EFFECT 버스트를
쓴다**가 클래스 공통 규칙으로 쓸 만하다는 뜻이다.

그래서 워로드 `hitTimeMs`는 대부분 손대지 않았다. 저장값이 이미 HIT 노티파이와 일치한다.
어긋나는 W 파이어 불릿(700 vs 325), D 방패 격동(700 vs 590), F 가디언의 낙뢰(1000 vs 307/1200)는
이번 범위에 넣지 않았다.

## 2. E 대쉬 어퍼 파이어는 2단 콤보다

`ACTIVE` 한 덩어리로 클립 두 개를 연달아 재생하고 있었다. 사용자가 원작에서 재입력 2단임을
확인했고, 원본도 그렇게 저작돼 있다.

```text
wgl_sk_dashupperfire_01  len=1.8000  [선콤] t=0.2000 d=0.4500  HIT t=0.2500
wgl_sk_dashupperfire_02  len=1.7000  HIT 없음, d=0 버스트 t=0.1200
```

기준 체인 자체가 `[선콤]`(COMBO_PRE)을 갖고 있다. 스테이지를 나누고 `skillKind`를 COMBO로 바꿨다.

| 스테이지 | dur | hit | 입력 창 |
|---|---|---|---|
| `[dashupperfire_01]` | 1800 | 250 | 200-1800 |
| `[dashupperfire_02]` | 1700 | 120 | — |

**입력 창을 원본의 200-650이 아니라 스테이지 끝까지 열었다.** 처음에 650으로 넣었더니 2단이
안 나갔다. 우리 서버는 창 안에서 버퍼를 잡고 `hasAppliedSkillDamage` 이후 남은 클립을 잘라
전이한다(`cancelsIntoNextStage`). 원본의 창은 "언제까지 입력을 받는가"이고 우리 계약에서는
"언제까지 버퍼링을 허용하는가"라 의미가 다르다. 도화가 콩콩이도 같은 이유로 끝까지 열었다.

## 3. X 전장의 방패는 3클립이다

바인딩이 8클립 15367ms였다. 사용자가 Animation Tool에서 `_01 _02 _03`만 순서대로 재생하면 된다고
확인해 6200ms로 줄였다.

부수 효과로 08-07 RESULT가 남긴 문제가 닫혔다. `up`이 -12.417로 끝나던 것은 하강 클립
(`_03_re`, `_03`)이 둘 들어가 있어서였고, 3클립이 되면서 `_01`이 올린 12.417을 `_03`이 정확히
되돌려 0으로 닫힌다. 전진도 6.322 → 5.508로 정리됐다.

## 4. Z 방어 태세 — 세 갈래

토글 자체는 이미 있었다. `17800`(NORMAL→DEFENSE) / `17810`(DEFENSE→NORMAL)이
`requiredStance`/`setsStance`로 물려 있어 Z 재입력 해제와 "태세 중 다른 스킬 사용"은 그대로
동작했다. 없던 것이 셋이다.

### 4.1 게이지가 무엇의 값인지부터 정했다

아이덴티티를 "워로드 방어 태세 전용"으로 만들면 서버 로직에 클래스 stance enum이 박힌다.
대신 **게이지는 기본 stance가 아닌 곳에 서 있는 비용**으로 정의했다.

```cpp
Is_HoldingGaugedStance = 0u != profile.iMaximumIdentity &&
                         player.eStance != profile.eDefaultStance;
```

워로드는 기본이 `WARLORD_NORMAL`이라 방어 태세가 과금 대상이 되고, 창술사는 게이지가 0이라
단창 전환이 자동으로 제외된다. 이동 배율도 같은 판정 하나를 읽는다.

게이지가 0이 되면 `eStance`를 기본 stance로 되돌린다. 이것이 자동 해제다.

### 4.2 데이터와 서버

`Data/Balance/PlayerProfiles.json`에 네 field를 추가했다. 워로드만 값이 있고 나머지 다섯은 0/1이다.

```text
defenseStanceMoveSpeedScale  0.6
maximumIdentity              1000
identityRegenPerSecond         40
identityDrainPerSecond        120
```

약 8.3초 유지, 25초 완충이다. 전부 튜닝값이다.

게이지는 resource pool과 같은 고정소수점 carry(`iIdentityAccumulator`)를 쓴다. 초당 정확히
`identityDrainPerSecond`만큼 정수로 소모되며 float drift가 없다. `CPlayerSkillSystem::Update_Identity`가
action state와 무관하게 매 tick 돈다 — 게이지는 행동이 아니라 서 있는 것으로 소모되기 때문이다.

publisher는 `maximumIdentity`가 0인데 rate가 0이 아니거나, 게이지가 있는데 drain이 0인 문서를
거부한다. drain 0은 태세가 영원히 열리는 상태이고 그것이 drain을 만든 이유이므로 조용히
통과시키면 안 된다. Server catalog도 같은 조건을 다시 검사한다.

`PLAYER` bootstrap 행이 9열에서 13열이 됐고 `SNAPSHOT_PLAYER`에
`iCurrentIdentity/iMaximumIdentity`가 추가되어 **프로토콜 12 → 13**이다.

### 4.3 수그린 자세

`CHARACTER_SPEC`의 `AnimationClips[]`는 IDLE/RUN/HIT/DEAD 4칸 고정이라 stance 변형 자리가 없었다.
`STANCE_LOCOMOTION_SPEC` 테이블을 optional trailing member로 추가했다. 항목을 갖는 클래스만
IDLE/RUN을 덮어쓰므로 나머지 다섯 spec은 건드리지 않았다.

```cpp
{ PLAYER_STANCE_ID::WARLORD_DEFENSE, "wgl_sk_defence_loop", "wgl_run_defence_1" }
```

`Apply_NetworkStance`가 stance 변화를 감지하면 현재 자세를 즉시 교체한다. `Set_Locomotion`은
이동 시작/정지 edge에서만 불려서, 서 있는 채로 Z를 누르면 그것만으로는 바뀌지 않는다. 스킬
재생 중이면 클립을 유지하고 끝난 뒤에 반영한다.

## 5. 회전이 셀 단위로 끊기던 문제

사용자 지적. 우클릭 이동 중 방향이 뚝뚝 끊어졌다.

원인은 두 겹이다. 서버가 경로 구간마다 `atan2`로 yaw를 다시 계산하는데 격자 경로라 8방향으로
양자화되고, 클라이언트가 그 값을 보간 없이 그대로 꽂고 있었다. 위치는 이미 `XMVectorLerp`로
따라가는데 회전만 즉시 반영이었다(`//G3에서는 위치만 보간하고, Yaw는 서버 최신값을 즉시 반영`).

**클라이언트 표현만 고쳤다.** 초당 720도, 최단호로 서버 yaw를 향해 돈다. 서버에서 회전을 늦추면
yaw가 스킬 조준 방향과 루트 모션 방향을 겸하므로 판정이 바뀐다. 위치가 이미 클라 보간인 것과
같은 경계다.

첫 스냅샷은 보간 없이 그 각도를 그대로 쓴다. 스폰은 회전이 아니라 배치이기 때문이다.

**근본 원인은 남아 있다.** 8방향 격자 경로는 직선으로 갈 자리를 지그재그로 간다. 경로를 펴는
것(string-pulling)은 서버 작업이라 별건이다.

## 6. 검증 (실행함)

```text
Shared / NetworkProtocolHarness / ClientFrontendHarness / Server / Client   빌드 성공
NetworkProtocolHarness      failures : 0
Server.exe --contract-test  failures : 0
Gameplay balance Validate / Publish   6 profiles / 132 skills / 108 damage profiles
루트 모션 재굽기            17080·17820만 변경, 나머지 14개 바이트 단위 동일 재현
git diff --check            통과
```

harness 두 곳이 새 계약을 따라가야 했다.

- `NetworkProtocolHarness`의 World Snapshot Payload Size가 player당 +8바이트를 반영한다.
- `ServerGameplayContractTests`의 `PLAYER` fixture 3개가 13열이 된다.

**인게임 확인(사용자): E 2단 콤보, X 3클립, Z 수그린 자세·감속·자동 해제, 회전 보간 전부 정상.**

세 커밋으로 나눴고 각 커밋이 자체적으로 성립하는지 확인했다. receipt와 `Character.cpp/h`는 세
갈래가 한 파일에 섞여 있어 되돌린 상태에서 검증하고 커밋한 뒤 다시 얹었다.

```text
ec96614  feat(balance): make the Warlord dash a combo and cut the shield to its three clips
e491c4d  feat(gameplay): give the Warlord an identity gauge that pays for its stance
0232149  fix(gameplay): turn the character toward the server yaw instead of snapping to it
```

## 7. 남은 것

- **HUD에 게이지가 안 그려진다.** `CCombatHUDViewModel`까지 값이 오고 팀 사용서에도 적었지만
  화면 표시는 UI layout 작업이다. 인게임 확인은 8.3초 뒤 자동 해제로 했다.
- **W·D·F의 `hitTimeMs`가 원본과 어긋난다.** 각각 700/325, 700/590, 1000/307. 이번 범위 밖.
- **워로드 바인딩에 `playMs`가 없다.** `dashupperfire_01`은 `t=0.6300 d=1.1700 → 1.8000`으로
  클립 컷 마커를 갖고 있다. 재입력하면 서버가 어차피 잘라내므로 안 걸었고, 재입력을 안 했을 때
  1단 뒤끝이 긴지 확인이 남았다.
- **격자 경로 지그재그.** 회전은 매끄러워졌지만 경로 자체는 8방향이다.
- **다단 히트 미구현.** F 가디언의 낙뢰가 1200·1300에 추가 버스트를 갖는다. 전 클래스 공통 제약.
- 수치 0.6 / 1000 / 40 / 120은 전부 감각 튜닝값이며 원본 근거가 없다.
