# 발탄 정본 publish·pull·build 가이드

이 문서는 발탄 split authoring을 수정한 작성자와 그 변경을 `git pull`로 받는 팀원이 같은
Product를 보게 만드는 실행 순서의 정본이다. `git pull` 성공과 Product build 성공은 서로 다른
단계다. pull 뒤 계약 하네스가 실패했다면 먼저 아래 생성 세대가 일치하는지 확인한다.

## 정본과 생성물

### 카메라와 이동 anchor 검증

고정 지점 도약 `LEAP_TO_ANCHOR`의 camera lookAt은 WORLD/BOSS_XZ 모두 원본
연출 곡선을 사용할 수 있다. 이동 anchor는 보스 착지점을 소유하며 카메라의
시선까지 고정하지 않는다. 추적 도약 `LEAP_TO_TARGET`의 착지점 시선 검사는
계속 적용된다. 원본 카메라를 게시하기 위해 tracking을 BOSS_XZ로 바꾸면
runtime에서 보스 변위가 추가되므로 원본 WORLD 좌표 계약을 유지한다.

- 저작 정본: `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json`,
  `Valtan.combatobjects.json` 및 같은 폴더의 split 문서
- Effect 정본: `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`과 reachable
  `Authored`, `Groups` 문서
- Git 관리 Product projection: `Data/Encounters/Valtan/ValtanEncounter.json`,
  `ValtanCombatObjects.json`, `Data/Animation/Authored/Valtan/*`
- Git 관리 Composition descriptor: `Data/Compositions/Bosses/Valtan.bosscomposition.json`.
  기존 owner의 경로·coverage·Pattern index만 소유하는 `SHADOW` source manifest다
- Git 관리 실행 생성물: `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`와 참조하는
  `ValtanPresentationGenerations` 문서
- Git 관리 Composition 생성물: `Client/Bin/DataFiles/Compositions/**`. resolved read model과 receipt이며
  직접 편집하지 않고 현재 Client/Server gameplay runtime도 소비하지 않는다

`Gameplay.bootstrap`과 참조 presentation generation은 publisher로 생성하고 정본 변경과 같은
PR에 포함한다. 직접 편집하지 않는다. 받는 PC는 같은 Git snapshot을 사용하며 수신을 위해 다시
게시하지 않는다. Client와 Server는 `Shared/Public/GameplayDataRevision.h`의 공용 format
version을 함께 사용하므로 reader/schema 변경에도 대응 출력과 소비 검증을 포함한다.

## 작성자가 PR 전에 실행할 순서

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2 -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode SyncValtanShadow -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Publish -RepositoryRoot $PWD
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
python Tools/GameplayPipeline/test_valtan_presentation_generation.py
python Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
git diff --check
```

`PublishV2`가 바꾼 Product 파일, Composition 출력과 `Gameplay.bootstrap` 및 참조 generation은
정본 변경과 같은 PR에 포함한다. 위 순서는 발탄 gameplay/presentation 전체를 바꿀 때의 게시
순서이며 변경하지 않은 domain을 매 작업마다 다시 게시할 필요는 없다. CPP/HLSL/schema 소비자를
바꿨으면 정상 증분 Product Build와 해당 기능의 소비 검증을 수행한다. Core/FullDiagnostic은
명시적 광역 진단이고 매 PR·pull의 필수 단계가 아니다. publisher 또는 하네스 기대값만 완화해
서로 다른 데이터 세대를 통과시키지 않는다.
`SyncValtanShadow`가 갱신한 Composition descriptor도 같은 PR에 포함한다. 이 명령은 split join의
Pattern index/coverage가 실제로 달라졌을 때만 descriptor revision을 올린다.

## 다른 PC에서 pull한 뒤 실행할 순서

```powershell
git pull --ff-only origin main
git lfs pull
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

위 pull은 작업 변경을 보존한 뒤 `main`에서 실행한다. 받은 변경이 데이터뿐이고 현재 바이너리가
같은 schema를 지원하면 C++/HLSL 재빌드 없이 해당 runtime reload/재시작 계약을 따른다.
일반 Product Build는 게시 데이터를 재생성하지 않는다. 정본 runner는 Items·Valtan ClearRewards의
`CheckPublished` 결과를 컴파일 성공과 별도로 기록한다. 누락이나 bootstrap version 불일치가
있으면 실제 읽힌 경로·header와 현재 reader를 대조하고 해당 publisher로만 복구한다.
pull한 팀원은 `SyncValtanShadow`를 실행하지 않는다. 작성자가 PR에 포함하지 않은 descriptor drift를
로컬에서 덮어 가릴 수 있기 때문이다. 명시 publish/Core/FullDiagnostic만 `BuildDomains.json`의
`composition.presentation` domain을 실행한다. Resources는 별도 Drive 전달 계약을 따른다.

## 대표 오류의 실제 소유자

| 오류 | 실패한 경계 | 확인할 것 |
|---|---|---|
| `Encounter stage v4 field is invalid` | Client Encounter reader/validator | Product projection의 새 field와 Client parser가 같은 변경에 포함됐는지 |
| `split Counter source requires exactly one COUNTER_HIT and TIMEOUT edge` | Pattern graph validator | `nextPatternId` 교차 패턴 성공과 동일 패턴 TIMEOUT을 reader와 harness가 모두 이해하는지 |
| `Gameplay.bootstrap version or row count is invalid` | presentation generation admission | 공용 bootstrap format version, publisher 출력, Client admission이 같은 값인지 |
| `split authoring Product drift` | Valtan projector | `PublishV2` 결과를 PR에 포함했는지 |
| `Effect V2 binding header/rows are invalid` | presentation publisher/admission | bindings와 groups가 strict formatVersion 2인지, validator가 아직 v1 row를 기대하지 않는지 |
| `pattern index order/closure drift` 또는 `composition coverage drift` | Composition SHADOW descriptor | 작성자가 `PublishV2` 뒤 `SyncValtanShadow` 결과를 PR에 포함했는지 |
| `KakulSaydon ... reference revision/action count drift` | Composition catalog validation | 네 Kakul reference/action/pattern-binding 문서가 같은 reference 세대인지 |

이 오류들은 `git pull`이나 Git merge 자체의 실패가 아니다. pull로 받은 정본과 로컬에서 다시 실행된
publisher/validator/reader 중 하나가 다른 계약 세대를 소비할 때 clean build가 의도적으로 중단한
것이다.

Composition publish는 Valtan `SHADOW`, KakulSaydon `REFERENCE_ONLY`, 두 Arena Sequencer `SHADOW`
문서를 한 catalog로 검증한다. 현재 생성물은 모두 `runtimeEligible=false`다. Sequencer는 source manifest
inspection과 기존 Valtan Workbench 진입만 제공하며 generic Composition Save/Play와 arena runtime은
아직 완료되지 않았다.

## 화면 검증

자동화는 publish, parse, build와 구조화된 runtime 계약까지만 판정한다. Effect, 발탄 패턴 및
Kakul 화면 결과는 사용자가 Client를 직접 실행해 확인한다.
