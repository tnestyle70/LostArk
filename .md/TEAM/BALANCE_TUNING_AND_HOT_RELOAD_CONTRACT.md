# 밸런스 튜닝과 Hot Reload 계약

## 1. 현재 사용할 수 있는 기준선

밸런스 정본은 `Data/Balance/*.json`이다. Visual Studio의 Client 프로젝트에서는
`96.DataFiles/Balance` 필터로 원본 파일을 바로 열 수 있다. 프로젝트에 복사본을
두지 않으며 Server 생성물인 `Gameplay.bootstrap`을 직접 편집하지 않는다.

현재 제품 경로는 다음까지 연결되어 있다.

```text
Data/Balance JSON
-> Publish-GameplayBalance.ps1 parse/validate/stage/commit
-> Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap
-> Server CGameplayCatalog
-> GameRoom 30 Hz 판정
-> S2C_WORLD_SNAPSHOT
-> CCombatHUDViewModel
-> 인게임 HP/resource/cooldown/damage/boss 상태 표시
```

따라서 수치 변경은 JSON에서 시작하고 publisher 검증, Server 재기동, Training smoke
순서로 눈으로 확인한다. Client나 UI에서 damage, cooldown, boss phase를 덮어쓰지 않는다.

## 2. 현재 Hot Reload 상태

실행 중인 Server gameplay 수치를 무중단 교체하는 Hot Reload는 아직 활성화하지 않았다.
파일 감시만 붙이거나 Client HUD만 다시 읽으면 다음 문제가 생긴다.

- Client 표시값과 Server 판정값이 서로 다른 revision이 된다.
- 진행 중인 skill의 hit time, cooldown, movement distance가 중간에 바뀐다.
- active player의 maximum HP/resource 변경 정책이 정의되지 않는다.
- 진행 중인 boss phase와 encounter timing이 중간 정의를 소비한다.

이 상태에서 로컬 JSON 자동 재읽기를 넣는 것은 Hot Reload가 아니라 Server truth 우회다.

## 3. 활성화할 때 반드시 지킬 수직 슬라이스

Hot Reload는 다음을 한 변경 단위로 구현한다.

1. publisher가 content hash 기반의 증가하는 `balanceRevision`을 생성한다.
2. Server가 새 bootstrap을 별도 `CGameplayCatalog`로 parse하고 전체 참조를 검증한다.
3. 실패하면 현재 catalog와 revision을 그대로 유지하고 이유를 Debug tool에 반환한다.
4. 성공한 stage는 `CGameRoom::Tick` 시작 경계에서만 commit한다.
5. 진행 중 action/cooldown/boss pattern은 시작 revision을 끝까지 사용하고, 새 command부터 새 revision을 사용한다.
6. player maximum 값 변경은 현재 비율 유지 또는 clamp 중 하나를 계약으로 고정한다.
7. snapshot에 적용 중인 balance revision을 포함한다.
8. Client HUD definition도 같은 revision일 때만 교체한다.
9. Debug Balance Tool은 reload 요청, 현재/대기 revision, 검증 오류, 실제 Server 수치를 표시한다.
10. 정상 reload, 잘못된 JSON, 참조 누락, 진행 중 action, 중간 실패 rollback, 두 Client revision 일치 하네스를 추가한다.

## 4. 담당자 튜닝 절차

현재 안전한 절차는 다음이다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

그 다음 Server를 재기동하고 `dev.training.ground`에서 확인한다.

- Player: 이동 속도, HP/resource, Q/W cooldown·cost·damage·action timing
- UI: snapshot 수치와 정의의 displayName/damage/cooldown 표시
- Boss: HP, engage distance, move speed, phase threshold, pattern timing
- Character: Server가 승인한 action과 hit/move timing의 시각 일치

Training에서 확인한 뒤 Debug/Release regression, `Publish-BalanceRuntimeSet.ps1 -Mode Validate`, Server contract test를 통과시킨다. 인게임에서
보기 좋다는 이유만으로 JSON validation 범위나 Server authority를 완화하지 않는다.
