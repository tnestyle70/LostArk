# 발탄 애니메이션 우선 패턴 승격 구현 계획

## 1. 목표

애니메이션 담당자가 저장한 `Valtan.presentation.debug.json`의 체인 순서와 재생 시간을
2·3페이즈 패턴 저작의 시작 정본으로 사용한다. 선택된 체인은 1페이즈와 같은
`patternId -> stageId -> actionId -> clipOccurrenceId` 구조로 split authoring과 Product에
투영하고, All Effects 편집과 Server stable-ID 시험 재생에서 같은 action clock을 소비하게 한다.

현재 20개 체인은 전부 승격하되 피해, 이동, 분기 결과와 자동 선택 조건은 애니메이션만으로
추측하지 않는다. 새 패턴은 `MANUAL_SERVER_AUDITION` 상태로 시작하고 패턴·이펙트 담당자가
gameplay 의미를 확정한 뒤 별도 변경에서 Product rotation으로 승격한다.

## 2. 현재 실측

- debug 문서에는 고유 chain 20개와 고유 occurrence 94개가 있다.
- 94개 중 71개는 `playMs=0`이라 모델 native duration을 사용하고, 23개는 wall time을
  명시한다.
- 명시한 wall time 중 일부는 native clip보다 길다. 현재 Product runtime은 일반 EXACT
  occurrence의 `playMs`를 native remainder로 clamp하므로 평면 chain을 한 stage에 넣으면
  애니메이터가 저장한 8초·6초 hold가 짧아진다.
- debug의 `targetPatternId`와 `targetStageId`는 모두 비어 있고 hit, motion, event, branch,
  rotation을 소유하지 않는다.
- 현재 split authoring은 7개 managed 패턴만 소유하며 Product의 나머지 26개는 legacy
  compatibility로 봉인돼 있다.
- Server stable-ID audition queue는 Product pattern을 직접 실행하지만 normal selector와
  health mechanic selector에서 제외되는 제3의 selection mode는 아직 없다.
- 다른 세션이 수정 중인 `Data/Effects/Authored` 파일은 이 변경의 입력이나 출력이 아니며
  보존한다.

## 3. 정본과 생성 흐름

```text
Valtan.presentation.debug.json
  애니메이터의 chain/occurrence 순서와 요청 wall time
        +
Valtan.animation-chain-promotions.json
  phase, stable pattern ID, display name, 수명 상태
        +
MN_RPBF_01_AnimSet.wmodel
  playMs=0 occurrence의 실제 native duration과 clip identity
        |
        v
promote_valtan_animation_chains.py
  parse -> validate -> stage -> atomic commit
        |
        +-> Valtan.gameplay.json
        +-> Valtan.presentation.json
        +-> promotion receipt
        |
        v
기존 V2 publisher
        +-> ValtanEncounter.json (Server stage duration)
        +-> Valtan.patternbindings.json (Client ordered animation)
        |
        +-> All Effects managed tree
        +-> Server stable-ID audition
```

한 debug occurrence를 한 Server stage로 만든다. native보다 짧거나 같은 occurrence는
`EXACT`, native보다 긴 occurrence는 해당 stage 전체를 사용하는 `LOOP_TO_STAGE_END`로
투영한다. 이에 따라 Server `durationMs`와 Client animation wall budget이 항상 같은 한 값에서
생성된다. clip 이름의 `mesh_` 누락은 WModel에 정확히 하나의 prefixed clip이 존재할 때만
명시적으로 정규화하고 receipt에 남긴다.

## 4. 변경 파일과 역할

- `Data/Valtan/Valtan.animation-chain-promotions.json`
  - 20개 chain의 stable pattern ID, authoring phase와 `MANUAL_SERVER_AUDITION` admission을 소유한다.
  - 이후 3페이즈 chain도 같은 manifest에 추가한다.
- `Tools/ValtanPipeline/promote_valtan_animation_chains.py`
  - debug/manifest/WModel을 strict validate하고 split 문서 후보와 receipt를 transactionally 생성한다.
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
  - `decisionModel.manualAuditions`를 admission하고 신규 managed Product row와 binding을 기존 ordinal
    뒤에 결정적으로 추가한다.
- `Data/Valtan/Valtan.gameplay.json`, `Data/Valtan/Valtan.presentation.json`
  - 20개 패턴과 94개 stage/action/occurrence의 실제 managed 정본이다.
- `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`
  - `AUDITION_ONLY` selection mode를 parse하고 자동 선택용 수치를 모두 0으로 fail-closed 검증한다.
- `Server/Private/ValtanBrain.cpp`와 관련 contract test 지점
  - audition-only 패턴이 자동 선택되지 않고 stable-ID 강제 요청으로만 시작되는 것을 검증한다.
- `Client/Public/ValtanPatternTree.h`, `Client/Private/ValtanPatternTree.cpp`
  - split decision model의 manual audition metadata를 strict join하고 Product의 `AUDITION_ONLY`와
    일치시키며 managed tree에 노출한다.
- `Client/Private/Animation_Tool.cpp`, `Client/Private/BalanceTool.cpp`
  - 정확히 7개라는 초기 migration sentinel을 7개 baseline 포함 + 동적 managed closure로 바꾸고
    manual audition을 로테이션 편집 대상으로 오인하지 않게 표시한다.
- `Tools/ValtanPipeline/test_*.py`, gameplay publisher와 focused harness
  - 신규 admission, 신규 Product append, rollback, 자동 선택 차단, 20/94 closure를 검증한다.
- `.md/TEAM/발탄인수인계서.md`, 대응 RESULT
  - 애니메이션 담당 -> 패턴/이펙트 담당 -> rotation 승격 절차와 자동/수동 검증 경계를 기록한다.

## 5. 구현 순서

### G01. 공용 수명과 Product selection 계약

`decisionModel.manualAuditions`와 `AUDITION_ONLY`를 Data, publisher, Server, Client parser에
수직으로 추가한다. normal/health selector는 이 행을 절대 후보로 사용하지 않고 Debug stable-ID
audition만 기존 Server pattern execution 경로로 통과시킨다.

종료 증거는 잘못된 수치·중복 chain/pattern·decision model 중복 소유가 모두 거부되고,
강제 audition만 허용되는 contract test다.

### G02. 재사용 가능한 animation-first projector

manifest와 promotion tool을 추가한다. 실제 WModel clip table에서 native duration을 읽고
alias를 exact resolve하며, 20개 chain을 94개 one-occurrence stage로 stage한다. Validate 모드는
파일을 바꾸지 않고 Apply는 두 split 문서와 receipt를 한 transaction으로 교체한다.

종료 증거는 20 chain/94 occurrence/0 unresolved clip/0 implicit duration이며 중간 실패 fixture에서
기존 split 파일 hash가 보존되는 것이다.

### G03. All Effects와 Server 연결

생성된 20개 managed 패턴을 기존 7개 뒤에 append하고 Product encounter/bindings를 생성한다.
All Effects는 `[P2 Animation] <chain>` 이름과 stage occurrence를 동적으로 읽는다. Server는 같은
pattern/action/stage duration을 읽어 stable-ID audition을 수행한다.

종료 증거는 split/Product exact join, 동적 managed count, normal rotation 불변과 Server contract다.

### G04. 3페이즈 인계 계약과 문서

새 chain은 debug 저장 -> manifest 한 행 -> projector validate/apply -> gameplay/effect 저작 ->
rotation 승격 순서만 사용하도록 팀 문서를 갱신한다. ghost model variant와 phase-3 gameplay
transition은 현재 base-model 20개 승격과 섞지 않고 별도 수직 슬라이스로 남긴다.

## 6. 검증

1. JSON/XML parse와 `git diff --check`
2. promotion tool Validate 및 transaction rollback fixture
3. `Project-ValtanPatternMaster.ps1 -Mode PublishV2`, `-Mode ValidateV2`
4. Valtan pipeline, pattern tree, Animation Tool, Balance Tool focused tests
5. gameplay publisher validate와 generated bootstrap parse
6. Shared/Server Debug build와 `Server.exe --contract-test`
7. Client Debug compile
8. 사용자가 직접 Server + Client를 실행해 F1 -> Effect Tool -> All Effects -> Valtan에서
   2페이즈 패턴 이름, 순서, stable-ID Server Play와 Effect를 육안 확인

자동 검증은 구조, ID, duration, selector 격리와 실행 준비까지만 판정한다. 애니메이션 연결감과
Effect visual fidelity는 사용자 확인 전 PASS로 기록하지 않는다.
