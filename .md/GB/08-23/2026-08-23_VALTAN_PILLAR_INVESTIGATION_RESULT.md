# 2026-08-23 발탄 비석(4주 비석) 미출현 조사 — 결과

담당: 맵/발탄. 최종 화면 판정자: 사용자.

## 1. 결론

서버의 비석 raise 경로는 정상이다. 제품 틱 루프를 끝까지 태우는 신규 계약 테스트로 증명했다.
비석이 화면에 없던 이유는 코드 결함이 아니라 **두 개의 실제 게이트**를 넘지 못했기 때문이다.

1. 비석은 보스가 **160줄 중 100줄**로 내려가야 처음 생성된다. 22,500 데미지가 필요하다.
2. 그 순간 **살아 있는 플레이어가 교전 범위 안에 있어야** 한다. 없으면 보스가 교전을 풀고
   큐에 들어간 100줄 기믹이 실행되지 않은 채 대기한다.

조사 중 세운 "모델이 3.2 cm로 그려진다"는 가설은 틀렸고, 그 가설로 넣었던 수정은 전부 되돌렸다.

## 2. 오진과 정정

`ao_on`의 아마추어 노드 `itr_02326_sk.ao`가 **scale 100과 90도 회전**을 애니메이션 채널에
싣고 있다. 로더의 `0.01` pre-transform이 이를 정확히 되돌린다.

| 에셋 | 메시 span | 아마추어 | pre-transform | 실제 |
|---|---:|---:|---:|---:|
| 비석 `DEPLOY_ITR_02326` | 3.24 | x100 | x0.01 | **3.24 m** |
| 발탄 캐릭터 | 292.10 | x100 | x0.0001 | 2.92 m |

비석은 처음부터 3.24 m로 그려지고 있었다. `modelScale=1`을 넣었다면 324 m가 됐을 것이다.
`DeployPropCatalog.h/.cpp`, `Loader.cpp`, `Publish-MapAuthoring.ps1`, 소스 카탈로그를 원복하고
카탈로그를 version 2로 재 publish했다(placement sha256 `8945e9e7...` 유지).
`MapTool.cpp`은 다른 담당자의 미커밋 `COLLIDER_CONTACT` 변경이 섞여 있어 통째로 되돌리지 않고
내 hunk만 손으로 제거했다.

무효가 된 하위 근거 두 개도 함께 적는다. 메시 bone의 inverse-bind 축 길이 `1.0000`은 스키닝
행렬이지 아마추어 노드의 스케일이 아니다. 엄폐 반경 0.9와 XY 반경 0.91의 일치는 기존 `x0.01`에서
이미 성립하던 값이라 새 가설의 근거가 되지 못한다.

`Play_Animation(0.f)`이 델타 0에서도 `Update_TransformationMatrix`와 전 본의
`Update_CombinedTransformationMatrix`를 수행하므로 아마추어 x100은 INTACT 정지 포즈에서도
적용된다. 원복 판단의 두 번째 독립 근거다.

## 3. 실측으로 확인한 전체 체인

읽어서 확인한 링크는 모두 정상이었다.

| 링크 | 확인 내용 |
|---|---|
| 부트스트랩 | `VALTAN_ARENA.encounterpropsbootstrap` 헤더 7필드 유효, PROPSET 1 + PROPSLOT 4 |
| 경로 해석 | `Resolve_DataRoot()`가 `<exe>/../DataFiles`, 즉 `Server/Bin/DataFiles`. GameplayCatalog와 동일 |
| 슬롯 -> 배치 | 슬롯 4개 XZ가 `deployplacements`의 `DEPLOY_ITR_02326` 4행과 정확히 일치 |
| 하드코딩 ID | `VALTAN_PILLAR_SLOT_PLACEMENT_IDS` 4개가 그 4행의 runtimePlacementId와 일치 |
| 카탈로그 | `DEPLOY_ITR_02326` ANIM, intact 모델 경로 존재 |
| 패턴 | `VALTAN_FOUR_PILLARS_105` 4스테이지, 분기 takeoff -> yellow-zone -> target-cone -> recovery 선형 |
| 서버 훅 | `Apply_EncounterPropStageEntry`가 `GameRoom.cpp:5818` 틱 루프 stageChanged에서 호출 |
| 와이어 | 서버 `Send_EncounterPropSync`, Shared 코덱, 클라 `NetworkManager` -> `ClientReplication` 전부 존재 |
| 렌더 게이트 | `m_State != DESPAWNED && !Is_BasePresentationSuppressed()`. 비석 8개는 파괴 그룹에 없어 suppression 무관 |
| 입장 동기화 | JOIN 시 `GameRoom.cpp:1089`가 4슬롯 HIDDEN을 즉시 전송 |

## 4. 두 번의 침묵

이 결함이 로그 한 줄 없이 진행된 이유다. 원인은 아니지만 진단을 어렵게 만든 구조다.

- `Send_EncounterPropSync`: `!Is_Initialized()`이면 패킷 없이 `return true`.
- `Apply_EncounterPropPresentation`: `props.Slots.empty()`이면 그대로 `return true`.

두 침묵이 맞물리면 "아무 일도 안 일어나고 아무도 불평하지 않는" 모양이 된다. 이번 건은
초기화가 정상이라 해당하지 않았지만, 같은 증상을 다시 만나면 여기를 먼저 본다.

## 5. 실제 게이트

### 5.1 100줄 = 22,500 데미지

`BossProfiles.json`의 `BOSS_VALTAN`은 `maximumHp 60000`, `maximumHealthBars 160`이다.
1바 = 375 HP이므로 160 -> 100줄은 **22,500 데미지**다.

`VALTAN_FOUR_PILLARS_105`는 `selectionMode HEALTH_BAR`, `triggerHealthBar 100`이고
`QueueCrossedHealthBarPatterns`는 `iLastEvaluatedHealthBar > 100`이면서 현재 바가 100 이하일 때만
큐에 넣는다. 초기 `iLastEvaluatedHealthBar`는 160이라 조건 자체는 성립한다.

비석을 부수는 `VALTAN_RED_BLADE_WAVE`는 `selectionMode NORMAL`, `minimumHealthBar 1`,
`maximumHealthBar 100`, weight 9다. 즉 100줄 아래에서만 눈에 띈다. 사용자가 저작한
"비석 2개씩" 순서는 `PATTERNSTAGEPROPBREAK` 4행에 그대로 있다.

```
VALTAN_RED_BLADE_WAVE  stage 1 PROJECTILE  -> slot00, slot02
VALTAN_RED_BLADE_WAVE  stage 2 RECOVERY    -> slot01, slot03
```

### 5.2 살아 있는 타깃이 없으면 큐가 대기한다

`CValtanBrain::Update`는 `iCurrentHp == 0`, `!isCombatReady`, `DEAD`, `FALLING` 플레이어를
타깃 후보에서 제외한다. 후보가 없으면 `FinishPattern` 후 즉시 return하므로 패턴 선택 함수에
도달하지 않는다. 큐에 든 100줄 기믹은 `PendingPatternIds`에 남아 대기한다.

이 동작 자체는 정상이다(보스가 교전을 푼다). 다만 솔로로 발탄을 깎다가 사망하면 그 사이에는
100줄 기믹이 실행되지 않는다는 뜻이다.

이 게이트는 F1 Debug의 `Reset + Play Sky + Pillar Cycle`에도 똑같이 적용된다. 그 버튼은 보스를
100줄로 강제하고 큐에 넣을 뿐, 실행은 같은 타깃 조건을 거친다.

## 6. 신규 계약 테스트

`Server/Private/ServerGameplayContractTests.cpp`에 제품 경로를 끝까지 태우는 테스트를 추가했다.

```
Raise the four stele by damaging the boss across the authored 100-bar boundary
and running the product tick loop
```

`CGameRoom{VALTAN_ARENA}` -> `Activate_Encounter` -> 살아 있는 플레이어 1명 배치 ->
보스 HP를 `Resolve_HealthBarHp(boss, 100)`으로 설정 -> 실제 `room.Tick(1/30)` 반복.
큐 -> 선택 -> RECOVERY 진입 -> 4슬롯 INTACT까지 관측한다. 기존 테스트가
`Apply_EncounterPropStageEntry`를 직접 호출했던 것과 달리 이 테스트는 스테이지로 몰아넣지 않는다.

이 테스트는 처음에 실패했고, 진단을 붙여 원인이 **테스트 플레이어의 사망**임을 확인했다.

```
pattern=0 recovery=0 raised=0 pending=2 playerHp=0 playerAction=4(DEAD)
```

플레이어를 살려두자 통과했다.

```
pattern=1 recovery=1 raised=1 slots=4 pending=0 curStage=RECOVERY
```

`pending=2`는 큐잉은 됐는데 실행이 안 된 상태를 그대로 보여준다. 5.2의 근거다.

### 6.1 Debug 버튼 경로도 함께 닫았다

사용자가 실제로 누르는 `Reset + Play Sky + Pillar Cycle`은 지금까지 end-to-end 검증이 없었다.
두 번째 테스트를 추가했다.

```
Raise the four stele from the Debug pillar-cycle button through the real tick loop
```

`Evaluate_ValtanAudition(PLAY_PILLAR_CYCLE)`를 실제로 제출하고 그 뒤 `room.Tick(1/30)`만 돌려
4슬롯 INTACT까지 관측한다. 이 버튼은 `iTargetHealthBar`를 싣지 않는 barless 요청이라
`Resolve_HealthBarHp(boss, 0)`이 0을 돌려줄 위험이 있었는데, 서버가 `GameRoom.cpp:2951`에서
`targetHealthBar = authored->iTriggerHealthBar`로 패턴에서 100을 끌어온다. 테스트가 보스가
죽지 않고 살아남는지(`survivedTheEdge`)도 함께 검사한다.

이로써 비석 계약 6건이 모두 실행으로 닫혔다.

```
Queue the 100-bar stele mechanic when Valtan crosses its trigger bar
Start the 100-bar stele mechanic from the queue
Reach the stele RECOVERY stage that raises the four pillars
Raise the four stele from the room stage entry the tick loop calls
Raise the four stele by damaging the boss across the authored 100-bar boundary ...
Raise the four stele from the Debug pillar-cycle button through the real tick loop
```

## 7. 자동 검증

| 검증 | 결과 |
|---|---|
| Server x64 Debug 빌드 | PASS |
| `Server.exe --contract-test` | failures 0 |
| 신규 end-to-end 비석 테스트(데미지 경로) | PASS |
| 신규 Debug 버튼 비석 테스트(PLAY_PILLAR_CYCLE) | PASS |
| 비석 계약 6건 전체 | PASS |
| `NetworkProtocolHarness` | failures 0 |
| `git diff --check` | 오류 없음 |

## 8. 사용자 확인 절차

전투 없이 30초 안에 판별할 수 있는 순서다.

1. 발탄에 진입하고 F1 -> Valtan 패널을 연다.
2. `Pillars:` 줄을 읽는다. Debug 출력에도 동기화가 적용될 때마다 한 줄이 나온다.

```
[Level_ValtanArena][EncounterProps] sync epoch E tick T:
4 slots, HIDDEN 4 INTACT 0 BREAKING 0
```

   이 줄이 아예 없으면 서버 동기화가 도착하지 않은 것이고, 있으면 서버가 보낸 실제 슬롯
   상태를 그대로 읽을 수 있다. 상태가 바뀔 때만 찍히므로 프레임마다 쏟아지지 않는다.
   - `Pillars: 4 slots | HIDDEN 4 | INTACT 0 | BREAKING 0` -> 서버-클라 연결 정상.
     비석은 아직 안 올라온 것이 맞다. 3번으로 간다.
   - `Pillars: no encounter prop state received yet.` -> 입장 동기화가 안 온 것이다.
     이 경우만 새 결함이며 그때 다시 조사한다.
3. `Reset + Play Sky + Pillar Cycle`을 누른다. **누를 때 캐릭터가 살아 있어야 한다.**
4. 비석 4개가 동서남북(정확히는 중심에서 대각 5.1 m) 위치에 올라오는지 본다.
   Debug 출력에 다음 4줄이 함께 나온다.

```
[Level_ValtanArena][EncounterProps] pillar.valtan.slotNN raised at
(x, y, z) height 3.2m radius 0.9m
```

5. 로그가 나오는데 화면에 없으면 가려짐/재질/컬링/카메라 쪽이다.
   `height`가 3 m대가 아니면 스케일을 다시 본다.

## 9. 남은 경계

- 슬롯 4개는 중심 `(156.03, -122.06)`에서 대각선 방향 5.1 m에 있다. 사용자가 말한 "동서남북"과
  45도 어긋난다. 배치 의도를 바꾸려면 `EncounterProps.world.json`의 슬롯 좌표를 옮기고
  `Publish-WorldGameplay.ps1`로 다시 publish한다. 이번에는 변경하지 않았다.
- 100줄 기믹이 큐에서 대기 중일 때 이를 알려주는 표시가 없다. 필요하면 F1 패널에
  `PendingPatternIds`를 노출하는 것이 가장 싸다.
- `Apply_EncounterPropPresentation`에 적용 동기화 요약 로그를 추가했다(Debug 한정).
  서버 raise와 클라 수신을 로그 두 종류로 분리해 관찰할 수 있다.
- 화면 판정은 사용자 전용이다. 위 8번 전에는 visual PASS로 기록하지 않는다.
