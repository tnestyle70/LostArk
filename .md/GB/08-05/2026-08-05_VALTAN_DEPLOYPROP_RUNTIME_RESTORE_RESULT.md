# Valtan DeployProp 생성 경로 복구 결과

## G00. 결론

기존 Deploy 리소스를 다시 추출하거나 재쿠킹하지 않고, 끊겨 있던 생성 경로를 현재 프레임워크에 맞게 복구했다.

- `CDeployPropRuntime`이 catalog/placement parse 결과를 임시 객체로 전부 stage한 뒤 한 번에 commit한다.
- MapTool Valtan Area는 `Data/Maps/MapCatalog.json`의 `sourceDeployCatalog`와 `sourceDeployPlacements`를 읽기 전용으로 소비한다.
- 제품 `LEVEL::VALTAN_ARENA`는 published `Client/Bin/DataFiles/Map/<Area>.deployassets/.deployplacements`를 소비한다.
- 두 소비자는 같은 `CDeployPropRuntime` 생성·rollback·조회 경로를 사용한다.

## G01. 실제 변경

1. `CDeployPropObject::DEPLOY_PROP_DESC`에 `prototypeLevelIndex`를 추가해 `LEVEL::DEVELOPMENT` 하드코딩을 제거했다.
2. `CDeployPropObject::Set_State`가 성공/실패를 반환해 animation clip 누락을 조용히 무시하지 않게 했다.
3. `CDeployPropRuntime`을 추가해 85개 생성, stable runtime placement ID 조회, 전체 상태 transaction, clear/rollback을 소유하게 했다.
4. Loader가 Valtan Deploy 모델 9종과 `Prototype_GameObject_DeployProp`을 `LEVEL::VALTAN_ARENA`에 등록한다.
5. Development Map Editor는 animated mesh shader와 DeployProp GameObject prototype을 준비하고, Valtan Area 전환 transaction 안에서 Deploy 85개를 함께 stage한다.
6. `CLevel_ValtanArena`가 일반 맵 로드 다음 Deploy runtime을 로드하고 이후 camera/replication 초기화 실패 시 함께 rollback한다.

## G02. 데이터 증거

```text
deploy asset rows       9
deploy placement rows  85
unique runtime IDs     85
missing WModel files    0
WModel files           17
ITR_02326 animations    4
```

현재 `ITR_02326` clip은 다음 네 개다.

```text
itr_02326_sk.ao_hit1_1
itr_02326_sk.ao_off
itr_02326_sk.ao_on
itr_02326_sk.ao_spawn
```

따라서 77개 static placement는 intact/fractured 모델 교체, 8개 animated placement는 on/off non-loop 전환을 사용할 수 있다.

## G03. 자동 검증

- Client x64 Debug build: PASS
- Debug 전체 Engine/Shared/NetworkProtocolHarness/ClientFrontendHarness/Server/Client build: PASS
- Debug NetworkProtocolHarness: failures 0
- Debug ClientFrontendHarness: PASS
- Debug `Server.exe --contract-test`: failures 0
- Release 전체 build와 동일 harness/contract test: PASS
- XML project/filter parse: PASS
- 관련 파일 `git diff --check`: PASS

`ProjectAudit -DeepAssetHash`는 이번 변경 이전부터 존재한 Resources payload와 `Data/AssetPacks.lock.json` manifest의 파일 집합 불일치로 FAIL했다. 이번 변경은 `Client/Bin/Resources`와 asset pack lock/manifest를 수정하지 않았다.

## G04. 아직 완료하지 않은 경계

- 실제 창에서 `Lobby -> Test -> F1 -> Map Tool -> Valtan` 선택 후 85개 시각 확인
- Server+Client로 Valtan 입장 후 발탄 보스와 Deploy 85개 동시 확인
- `WORLD_ENTITY_KIND::DESTROYABLE`, Server trigger 판정, 상태 snapshot, late join 복제
- `SET_CONDITION`과 Server dynamic navigation 경로 무효화

즉 이번 결과는 **Deploy 생성 경로 복구 완료**이며, **Server 권위 파괴 동작 전체 완료**는 아니다.
