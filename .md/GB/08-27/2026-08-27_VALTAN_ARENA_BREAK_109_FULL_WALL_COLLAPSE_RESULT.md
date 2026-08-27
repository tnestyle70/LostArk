# Valtan 109 Arena Break Full Wall Collapse Result

## 완료 상태

발탄 2페이즈 진입 컷신 `VALTAN_ARENA_BREAK_109 / IMPACT`가 외곽 링 30개만 무너뜨리던 것을,
아레나 위에 남아 있는 내부 벽 67개까지 같은 tick에 함께 무너뜨리도록 연결했다.

데이터·툴·publisher 단계의 자동 검증은 통과했다. C++ 빌드가 필요한 하네스와 실제 화면
판정은 사용자 몫으로 남아 있다. 아래 `남은 검증`을 따른다.

## 실제 구현

### 저작 데이터

- `Data/Encounters/Valtan/ValtanWorldEvents.json`의 휴면 STAGE 바인딩 67개를 활성화했다.
  `wall` 57 + `wall159` 10이며, 이 67개가 문서에 존재하던 유일한 `false` 값이었다.
  전부 `VALTAN_ARENA_BREAK_109 / IMPACT / STAGE_ENTER`, `offsetMs 0`,
  `receiverCollisionId ""`이고 각각 서로 다른 group을 가리킨다.
- `Data/Valtan/Valtan.worldeventsets.json`의
  `worldeventset.valtan.arena-break-109.outer-wall`에 member 67개를 추가해 30 → 97이 됐다.
  set ID는 저장 계약이므로 이름을 바꾸지 않았다.
- 새 group·mutation·placement·debris recipe를 만들지 않았다. 이미 저작되어 있던 계약만 켰다.

### 파이프라인과 publisher

- `valtan_tuning_pipeline._world_member_id`가 `outerwall109` 외에 `wall`, `wall159`
  계열의 member ID도 만들 수 있게 일반화했다.
- `validate_world_event_sets`의 group closure 검사에서 빈 `navigationRegionIds`를
  허용했다. 내부 벽 57개 중 2개가 nav region을 갖지 않는데, 이는
  `Publish-ValtanWorldDestruction.ps1:951`이 문서화한 정상 상태다. placement가 없는 경우만
  치명으로 남겼다.
- `Publish-ValtanWorldDestruction.ps1`에 내부 벽 계열 접두사와
  `$expectedInterior109BindingCount = 67`을 추가하고, 내부 바인딩을 세기만 하던 자리에서
  계열 검사와 group 중복 검사를 하도록 바꿨다. `interior109BindingCount -ne 0` 게이트는
  정확히 67개를 요구하는 검사로 교체했다.
- 음성 계약 테스트 `interior group rejoining the 109 batch`는 이제 정상 입력이므로
  `interior wall dropping out of the 109 batch`와 `floor sector riding the 109 collapse`
  두 개로 교체했다.

### Shared 상한

97개를 한 tick에 처리하려다 실제 fail-close 지점 두 곳을 만났다.

- `MAX_WORLD_DESTRUCTION_EVENTS`를 64에서 128로 올렸다. `GameRoom.cpp:8536`이
  `eventCount > MAX_WORLD_DESTRUCTION_EVENTS`에서 false를 반환해 97개 batch는
  `Apply_WorldDestructionStageEntry` 자체가 실패했다. 형제 상수 두 개가 이미 128이라
  같은 값으로 맞췄고, 전체 group이 105개이므로 이 상한이 다시 걸리는 일은 없다.
- 이 변경으로 예전 Client가 97개짜리 delta를 거부하므로 `NETWORK_PROTOCOL_VERSION`을
  39에서 40으로 올렸다. `CLAUDE.md`와 `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 v39 표기도
  함께 고쳤다.

### Client 표현

- `MAX_ACTIVE_ACTORS`를 `OUTER_RING_EMITTERS * 12`(360)에서
  `(OUTER_RING_EMITTERS + INTERIOR_WALL_EMITTERS) * 12`(1,164)로 올렸다. 원래 주석이 세운
  불변식("부분 recipe를 넣으면 벽의 2/3가 사라지고, 뒤 벽을 굶기면 통째로 잔해 없이
  사라진다")을 그대로 유지하려면 batch 크기를 따라가야 한다. debris recipe 8종은 모두
  정확히 12조각이라 조각 수는 손대지 않았다.
- 잔해는 `Find_Group(groupId)`로 조회하고 `BREAKING` 상태 전이에서 재생하므로,
  presentation 문서의 `bindingId`가 `.contact`에서 `.preview` / `.arena-break-109`로 바뀐
  것은 표시용 메타데이터 변화이고 재생 경로에 영향이 없다.
- `Level_ValtanArena`의 Debug 패널이 내부 group의 상태 변화를 빨간 오류로 보고하던 것을
  정상 진행 표시로 바꾸고, "outer ring only / interior groups stay dormant" 안내 문구를
  현재 계약으로 고쳤다.

### 계약 테스트

- `WorldDestructionBootstrapContractTests.cpp`: 바인딩 행 157 → 224, 109 batch transition과
  binding application 30 → 97, placement 합계 60 → 135으로 올리고, 모든 transition이
  outer/wall159/wall 세 계열 중 하나이며 그중 정확히 30개가 외곽 링임을 검사하도록 바꿨다.
- `ServerGameplayContractTests.cpp`: live-event batch를 30 → 97, 다음 sequence 31 → 98,
  `firstEvents` 30 → 97로 올렸다. 내부 벽이 67개 반응하는지와, 세 계열 밖의 group이 하나도
  반응하지 않는지를 함께 검사한다.
- 부분 파괴 batch 계약(`Release-safe partial 109 batch`)은 outer가 아닌 group이 전부
  그대로여야 한다고 요구했다. 이 검사는 event 상한 때문에 batch 전체가 실패하는 동안 공허하게
  통과하고 있었다. 면제 대상을 outer/wall159/wall 세 계열로 바꾸고, 실제로 지켜야 할 불변식인
  floor sector와 entrance 벽 보존을 이름과 함께 명시했다.
- `Leave every floor sector INTACT when the 109 outer ring collapses`의 주석과 이름에서
  "outer ring only" 표현을 현재 계약에 맞게 고쳤다. 검사 자체(floor 6개 생존)는 그대로다.

## 부분 파괴 상태에서의 동작

원래 의도는 1페이즈 동안 스킬과 몸통 충돌로 내부 벽이 차례로 무너지고, 2페이즈 진입에
외벽이 날아가는 것이다. 딜이 빨라 내부 벽이 남은 채로 2페이즈에 들어가는 경우에만 남은
벽이 외벽과 함께 날아가야 한다.

이 동작은 새로 만든 것이 아니라 `CWorldDestructionRuntime::Prepare_Trigger`가 이미 가진
admission 규칙이다. `WorldDestructionRuntime.cpp:383`이 바인딩이 가리키는 group이 INTACT가
아니면 `foundNoChange`로 건너뛰고, `:413`이 남은 transition이 하나라도 있으면 `READY`를
돌려준다. 따라서 97개 바인딩을 붙여도 실제로 움직이는 것은 그 시점에 서 있는 벽뿐이다.

발행된 bootstrap에 같은 규칙을 그대로 적용해 확인한 결과다.

```text
1페이즈에서 부순 내부 벽    컷신이 날리는 벽    내역                     결과
  0개                     97               외벽 30 + 남은 내부벽 67   READY
 20개                     77               외벽 30 + 남은 내부벽 47   READY
 50개                     47               외벽 30 + 남은 내부벽 17   READY
 67개 전부                30               외벽 30 + 남은 내부벽  0   READY
```

내부 벽 67개는 `COLLIDER_CONTACT` 바인딩을 그대로 유지하므로 1페이즈에서 스킬로 부수는
기존 경로도 변하지 않는다.

## 검증 증거

실행한 것만 적는다.

```text
python -m Tools.ValtanPipeline.valtan_tuning_pipeline validate
  ok=True errors=[] worldMembers=97

Publish-ValtanWorldDestruction.ps1 -Mode Validate
  groups=105 bindings=224 emitters=105
  outer109=30groups/60placements/30emitters/30aliases/360fragments
  interior109=67 openingImpact159=10 dashImpact159PlusOuter=40 contacts=69

Publish-ValtanWorldDestruction.ps1 -Mode ContractTest   통과
Publish-ValtanWorldDestruction.ps1 -Mode Publish        통과

published bootstrap 실측
  VALTAN_ARENA_BREAK_109 STAGE 바인딩 97
  group 계열  outerwall109 30 / wall 57 / wall159 10
  placement 합계 135 (외곽 60 + 내부 75)

python -m unittest Tools.ValtanPipeline.test_valtan_pattern_master_v2   exit 0
python -m unittest Tools.WorldPipeline.test_valtan_floor_destruction_transition_contract
  Ran 8 tests, OK

git diff --check    깨끗함 (기존 LF/CRLF 경고만)
JSON parse          ValtanWorldEvents / Valtan.worldeventsets /
                    worlddestruction.json / worlddestructionpresentation.json 모두 OK
```

`test_valtan_balance_tool_contract`에 실패 1건이 있으나 이번 변경과 무관한 기존 실패다.
`stageKind -cne 'GROGGY'` 문자열이 `Publish-GameplayBalance.ps1`에 있어야 한다는 검사인데,
그 문자열은 HEAD 버전과 작업 트리 양쪽 모두에 없다. 해당 파일은 다른 담당자의 미커밋
변경이며 이번 작업에서 건드리지 않았다.

## 남은 검증 (사용자)

Shared/Client/Server C++이 바뀌었으므로 빌드가 필요하다. 저장소 규칙대로 Engine →
UpdateLib → Shared/harness → Server → Client 순서를 지킨다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

빌드 뒤 확인할 것.

1. `NetworkProtocolHarness` 실행 — protocol 40과 새 event 상한에서 failures 0
2. `Server.exe --contract-test` — 위에서 고친 destruction 계약 포함 failures 0
3. Visual Studio에서 Client 실행 후 `Lobby -> Valtan` 진입
4. 109 컷신에서 확인할 것
   - 외곽 링과 함께 아레나 위에 남아 있던 벽들이 같이 날아가는지
   - 잔해 없이 그냥 사라지는 벽이 없는지
   - 바닥(84/30 지형 붕괴 담당)이 이 시점에는 그대로 남아 있는지
   - 1,164개 잔해가 동시에 뜨는 순간의 프레임 저하가 받아들일 만한지

4번의 마지막 항목은 이번 변경에서 가장 판단이 필요한 부분이다. 잔해 예산은 "모든 벽이
온전한 12조각 recipe를 갖는다"는 기존 불변식을 유지한 결과이며, 성능이 문제가 되면
`ACTORS_PER_EMITTER`를 줄이거나 벽 계열별로 예산을 나누는 별도 결정이 필요하다. 그 판단은
실제 화면을 본 뒤에 한다.

## 보존한 범위

작업 시작 전부터 있던 다른 담당자의 미커밋 변경(몬스터 슬라이스, Bern/Valtan BGM,
Bern 북문 navigation, `Publish-GameplayBalance.ps1` 등)은 되돌리거나 stage하지 않았다.
이번 변경도 commit/push하지 않았다.
