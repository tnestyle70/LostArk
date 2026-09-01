# 발탄 정본 publish·pull·build 가이드

이 문서는 발탄 split authoring을 수정한 작성자와 그 변경을 `git pull`로 받는 팀원이 같은
Product를 보게 만드는 실행 순서의 정본이다. `git pull` 성공과 Product build 성공은 서로 다른
단계다. pull 뒤 계약 하네스가 실패했다면 먼저 아래 생성 세대가 일치하는지 확인한다.

## 정본과 생성물

- 저작 정본: `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json`,
  `Valtan.combatobjects.json` 및 같은 폴더의 split 문서
- Effect 정본: `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`과 reachable
  `Authored`, `Groups` 문서
- Git 관리 Product projection: `Data/Encounters/Valtan/ValtanEncounter.json`,
  `ValtanCombatObjects.json`, `Data/Animation/Authored/Valtan/*`
- 로컬 생성물: `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`

`Gameplay.bootstrap`은 직접 편집하거나 Git으로 전달하지 않는다. 각 PC의 publisher가 같은 Git
정본으로 다시 생성한다. Client와 Server는 `Shared/Public/GameplayDataRevision.h`의 공용 format
version을 함께 사용한다.

## 작성자가 PR 전에 실행할 순서

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2 -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
python Tools/GameplayPipeline/test_valtan_presentation_generation.py
python Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
git diff --check
```

`PublishV2`가 바꾼 Git 관리 Product 파일은 정본 변경과 같은 PR에 포함한다. publisher 또는 하네스
기대값만 완화해 서로 다른 데이터 세대를 통과시키지 않는다.

## 다른 PC에서 pull한 뒤 실행할 순서

```powershell
git pull --ff-only origin main
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
```

증분 빌드가 통과했더라도 Core의 publisher와 하네스를 생략하지 않는다. 기존 PC에는 이전 bootstrap,
이전 harness EXE 또는 이미 컴파일된 Client object가 남을 수 있어 새 PC의 clean build와 결과가
달라질 수 있다.

## 대표 오류의 실제 소유자

| 오류 | 실패한 경계 | 확인할 것 |
|---|---|---|
| `Encounter stage v4 field is invalid` | Client Encounter reader/validator | Product projection의 새 field와 Client parser가 같은 변경에 포함됐는지 |
| `split Counter source requires exactly one COUNTER_HIT and TIMEOUT edge` | Pattern graph validator | `nextPatternId` 교차 패턴 성공과 동일 패턴 TIMEOUT을 reader와 harness가 모두 이해하는지 |
| `Gameplay.bootstrap version or row count is invalid` | presentation generation admission | 공용 bootstrap format version, publisher 출력, Client admission이 같은 값인지 |
| `split authoring Product drift` | Valtan projector | `PublishV2` 결과를 PR에 포함했는지 |
| `Effect V2 binding header/rows are invalid` | presentation publisher/admission | bindings와 groups가 strict formatVersion 2인지, validator가 아직 v1 row를 기대하지 않는지 |

이 오류들은 `git pull`이나 Git merge 자체의 실패가 아니다. pull로 받은 정본과 로컬에서 다시 실행된
publisher/validator/reader 중 하나가 다른 계약 세대를 소비할 때 clean build가 의도적으로 중단한
것이다.

## 화면 검증

자동화는 publish, parse, build와 구조화된 runtime 계약까지만 판정한다. Effect, 발탄 패턴 및
Kakul 화면 결과는 사용자가 Client를 직접 실행해 확인한다.
