# 2026-09-07 쿠크 Gate·부모 분류·재생 묶음 구현 결과

> 상태: 기존 Gate·묶음 배포와 G08~G11 후속 코드·최종 Product 빌드·저장/서버 계약 검증 완료. 후속 작업은 사용자 저작 데이터를 자동 publish하지 않았으며 사용자 화면 재생은 미확인.
> 계획: [Gate·묶음 구현 계획](2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_IMPLEMENTATION_PLAN.md)
> 브랜치: `codex/kouku-gate-pattern-bundles`. 선행 `Fix KoukuSaydon Complete Play` 작업의 미커밋 변경을 보존하고 이어서 구현했다.

## 반영한 기능

| 영역 | 실제 구현 |
|---|---|
| Composition Patterns | Gate → 부모 폴더 → 재생 묶음 → 자식 패턴과 Gate 직속 독립 패턴. Model View는 필터이며 묶음의 다른 모델 자식도 표시한다. |
| 생성·연결 | Create Parent, Create Bundle, Target Boss·목적지를 고르는 Create Pattern, Link Existing Pattern. 같은 stable pattern ID를 참조하며 클립을 복제하지 않는다. |
| 선택·Sequencer | 부모는 분류 안내, 묶음은 자식별 요약 행과 공통 Camera/Scene Profile, 자식은 기존 상세 Sequencer. 이전 선택의 타임라인을 유지하지 않는다. |
| 묶음 편집 | 자식 시작 offset, 이동할 부모, DRAFT/PRODUCT, 연결 제거, 공통 연출 Append·시간·삭제. 공통 Camera는 `BOSS` 비결합 anchor와 `followBoss=false`를 사용한다. |
| 저작 Preview | 각 모델의 기존 CNpc/CModel 경로와 독립 WORLD player를 사용한다. 묶음 공통 시계로 Play/Pause/Seek/Stop하고 자식 선택 시에는 해당 패턴만 재생한다. |
| F1 Complete Play | 기존 Gate 선택을 유지하고 게시된 PRODUCT의 폴더·묶음·자식을 표시한다. 묶음 전체 또는 자식 하나만 typed request로 제출한다. 기존 Play All은 PRODUCT 원본 순서의 순차 재생이다. |
| Server | 모든 대상의 admission과 Begin 준비 후 같은 tick에서 commit. offset은 30Hz tick으로 올림한다. member별 Logic ledger·후속 패턴·완료를 관리하고 모든 member 완료 후 부모를 종료한다. |
| 중지·재시작 | 원래 run epoch를 명시한다. 이전 epoch의 Stop이 새 실행을 중단하지 못한다. 시작 실패 시 일부 보스만 reset하거나 재생하지 않는다. |
| 복제·시계 | 같은 방과 늦게 입장한 Client에 run/member 상태·공통 시작 tick을 전달한다. WORLD와 애니메이션은 수신 지연만큼 cursor를 맞춘다. |
| 실행 소유권 | WORLD run/member/activation/source occurrence를 구분한다. 정확한 motion 대상과 STOP_OWNER를 연결하고 형제의 WORLD를 보존한다. MAP/DEPLOY의 같은 물리 target 중복 사용은 거부하며 Object Resource clone은 독립 생성한다. |
| 리비전 | 새 run에서 서버 pinned source revision에 맞는 Product와 모델 action binding을 준비한다. 다른 revision의 이전 캐시를 새 실행의 애니메이션으로 소비하지 않는다. |
| 전역 연출 충돌 | 서로 다른 owner의 겹치는 Camera/Scene Profile을 게시 단계에서 거부한다. 시각을 확정할 수 없는 FOLLOWUP은 같은 family를 공유하는 다른 owner와의 조합을 거부한다. Client도 실제 동시 활성화 충돌을 격리한다. |

## 저장·배포 계약

- Composition은 기존 경로 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 **v3, revision 87**이다. Gate 이름과 저장 경로를 혼동하지 않는다.
- 기존 9개 패턴의 Stage·Animation·Logic·연출·튜닝 필드는 이전 저장값과 동일함을 구조 비교했다. Gate/target 관계만 추가했다.
- `kakulsaydon.folder.1`의 `세이튼등장` 아래 `kakulsaydon.bundle.1`의 `세이튼등장_동시`를 만들고 기존 `_8` 쿠크와 `_9` 거대세이튼을 연결했다. 두 패턴은 원래 클립 없는 DRAFT이며 임의 모션·타이밍을 추가하지 않았다.
- 이번 publish 결과는 기존 PRODUCT 6개, 66 stages이다. 빈 DRAFT 묶음은 F1 Product 목록에 나오지 않는다. 자식 저작과 PRODUCT 전환 후 묶음도 PRODUCT로 저장·publish해야 한다.
- Encounter v4와 patternbindings v1은 기존 경로에서 source revision 87을 소비한다. bootstrap에는 `PATTERNTARGET`, `PATTERNBUNDLE`, `PATTERNBUNDLEMEMBER`를 추가했다.
- Shared protocol은 **66**이다. Server와 Client는 같은 소스·생성 데이터·protocol로 함께 빌드하고 재시작해야 한다.
- `Invoke-BuildDomainOwner.ps1`의 source revision 검사도 Composition v3를 허용한다. CAS 저장과 parse → validate → stage → commit, 잘못된 항목의 원문 보존을 유지했다.
- 물리 Resources는 새로 추출·이동·추가하지 않았다. 기존 Drive 리소스와 Resources 상대 경로를 그대로 사용한다.

## 자동 검증

| 검증 | 결과 및 증거 |
|---|---|
| Domain publish | `-Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 87` PASS. Product와 Gameplay bootstrap 생성. `out/KoukuGateBundles/publish.log` |
| Composition projector | 69개 PASS. Gate/관계/원문 보존/offset/공통 연출/실제 PS bootstrap 검증. `composition-tests.log` |
| Client 기존 계약 | 18개 PASS. 변경된 Complete Play 버튼과 명시적 Create 후 Append 계약을 반영했다. 기존 registry의 현재 scene ID에 맞춰 오래된 기대값도 교정했다. `client-contract-tests.log` |
| Server 묶음 계약 | 19개 assertion PASS, failures 0. 동일 tick, 67ms→3 ticks, 지연·부분 실패, 단독·순차 회귀, 후속·전체 완료, restart/Stop, WORLD 형제 보존. `server-bundle-tests.log` |
| Shared wire | 묶음·WORLD 관련 33개 assertion PASS. `protocol-tests.log` |
| Client 최소 빌드 | 컴파일·링크·DLL/shader 배포 PASS. `client-build.log` |
| 전체 Product 빌드 | 최종 **PASS**. Engine → Shared → Server → Client와 DLL/shader 배포, 누락 runtime input 0. `out/BuildPipeline/runs/20260907T082219028Z-debug-product.json`, `product-build-final.log`. 첫 시도는 병렬 Server 링크의 `Server.ilk` 점유로 실패했으며 해당 링크 종료 후 순차 재실행했다. |
| 기존 native editor | **PASS, exit 0**. 기존 harness에서 실제 Workbench/Document를 컴파일하고 Gate 2 폴더·빈 묶음·자식 ID 생성, Serialize→Parse 정확 일치, Save→Reload, 중복 target 후보의 저장 거부와 기존 파일 보존을 확인했다. 기존 249-stage·Append·quarantine·CAS 검사도 통과했다. `native-editor-final.log`. 처음 실패했던 v2/자동 Pattern 생성 fixture는 v3/명시 Create 계약으로 갱신했다. |
| JSON·XML·원본 보존 | 변경한 Composition/Encounter/patternbindings parse PASS, 기존 패턴별 필드 구조 비교 PASS. 기존 Client/Shared/Server/editor harness project XML parse PASS. 새 C++ 파일과 project 등록 변경은 없다. |
| 게시물 최신성 | `project_kouku_saydon_composition.py --mode validate` PASS, source revision 87. |
| diff | `git diff --check` 오류 없음. 기존 파일의 Git 줄바꿈 정규화 경고는 별도이며 기능 오류로 계산하지 않았다. |

## 사용자 화면 확인

Client/UI는 실행·조작하지 않았고 캡처나 visual PASS를 기록하지 않았다. 실행 준비가 끝나면 현재 PC의 `Server + Client` profile을 사용자가 `Ctrl+F5`로 시작한다.

1. Lobby → KoukuSaydon → F1 → Tools → Action Workbench를 연다. Composition Patterns에서 Gate `2관문`, Model View `All`을 고른다.
2. `세이튼등장` → `세이튼등장_동시` → 쿠크/거대세이튼을 각각 선택해 부모 안내·묶음 요약·자식 상세 Sequencer가 바뀌는지 확인한다.
3. 기존 빈 자식에 실제 해당 모델 클립을 Append하고 저장한다. 묶음을 선택해 두 모델을 Play/Pause/Seek/Stop한다. 자식 offset과 표시되는 실제 시작 tick도 확인한다.
4. 실제 서버 조건 검증 시 자식과 묶음을 PRODUCT로 저장하고 Publish한다. Server 재시작 후 F1에서 같은 Gate의 묶음 Complete Play와 자식 Complete Play를 각각 확인한다.
5. Camera/Scene Profile은 묶음에서 공통 행을 튜닝한다. 겹치는 자식 전역 연출이나 조건부 FOLLOWUP 충돌은 메시지를 보고 해당 owner의 구성을 정리한다.

기존 2관문 등장 연출 자체의 클립 제작·공 패턴 최종 타이밍·최종 화면 품질 판정은 이번 구조 구현의 자동 검증 결과에 포함하지 않는다.

## Git 상태

선행 작업에서 넘어온 대규모 미커밋 변경의 기준점은 `out/KoukuGateBundles/baseline/index.json`과 해당 파일 사본에 보존했다. 사용자·선행 작업 변경을 자동 stage/commit하거나 PR에 묶지 않았다. 이번 작업의 파일 변경과 자동 검증 결과를 확인할 수 있으며 PR/merge는 수행하지 않았다.

## G08. Composition Sequencer 선택 묶음 편집 후속

Duplicate는 선택된 owner Stage의 원래 순서대로 사본을 준비하고 가장 오른쪽 선택 owner Stage 뒤에 한 번 삽입한다. 원본과 사본이 번갈아 끼워지지 않는다. Stage와 자식 중복 선택은 한 번만 처리한다. Animation만 선택한 경우 owner별 선택 구간을 새 Stage로 복제하고 offset을 그 구간 시작으로 맞춘다. 원본 Stage의 미선택 tail은 유지되므로 그 tail 뒤에 사본이 놓인다. 사본 stable ID를 선택해 다음 Duplicate/Earlier/Later가 같은 묶음을 소비한다.

Earlier/Later와 좌우 방향키는 선택된 Stage 및 Animation owner의 중복을 제거해 한 후보에서 재배열한다. 선택 순서와 전체 선택을 보존하며 이동 방향의 Pattern 경계에 선택이 닿으면 전체를 거절한다. Logic/World/Effect 등 Pattern 시각 소유 lane을 자동 변경하지 않는다.

- Client Debug ClCompile: 오류 0, `out/CompositionSelectionCatalog/client-compile.log`. 저장소 기존 인코딩 경고가 있으며 이번 수정은 UTF-8/기존 줄바꿈을 유지했다.
- 기존 native editor 회귀: `--kouku-composition-editor-contract` PASS. 선택 순서·중복 선택·연속/비연속 Duplicate·Earlier/Later·실패 시 문서/선택/ordinal/저장 보존을 확인했다. `out/CompositionSelectionCatalog/editor-parent-test.log`.
- 프로젝트 XML 3개와 현재 WorldSequence/Composition JSON parse 통과. 코드 수정에서 저작 JSON·리소스·자동 Append는 수행하지 않았다.
- Client EXE 링크·배포: 사용자 종료 후 후속 Product 빌드로 반영했다. 최종 증거는 아래 G10/G11을 따른다. UI 입력·화면·실제 배치는 미검증이며 사용자 전용이다.
- 대규모 기존 dirty 변경은 보존했고 자동 stage/commit/push하지 않았다.


## G09. Parent 직속 Pattern 생성·기존 Pattern 분류 변경

Create Pattern에 Parent와 Bundle을 별도 선택하는 UI를 연결했다. Parent를 지정하고 Bundle=None으로 생성하면 optional `patterns[].folderId`에 Parent를 저장한다. Parent=None/Bundle=None은 기존 Gate 직속이다. Bundle을 지정해 생성하면 기존 member 참조만 추가한다. 선택 Parent와 다른 Parent의 Bundle, 다른 Gate 또는 오류 Parent는 거절하고 이전 생성 목적지를 유지한다.

기존 Pattern Detail에도 Parent 선택을 추가했다. `대형세이튼_조커찾기_성공`처럼 이미 만든 Pattern을 다시 만들지 않고 같은 Gate Parent로 분류하거나 Gate root로 되돌릴 수 있다. Stage·Animation·Logic·기존 Bundle 참조는 변경하지 않는다. Parent 직속 항목은 Model View, Parent tree/Detail, Product F1, Effect saved Pattern 목록에서 같은 분류를 소비한다. 자식 Pattern이 있는 Parent는 삭제를 막고, 잘못된 다른 Gate Parent를 참조하는 격리 Pattern은 원래 Gate 목록에서 숨기지 않는다. Bundle의 Parent를 옮기면 해당 Bundle로 향하던 Create 목적지도 동기화한다.

Document JSON reader/writer와 Product projector, Boss Tool reader, Gameplay publisher의 optional field 검사를 연결했다. 동일 Product Encounter를 읽는 일반 WorldGameplay publisher에도 optional folders/bundles 및 Gate/Actor/Parent ID의 기존 타입·stable ID 검사를 연결했다. Object Tool/Kouku Publish의 직접 경로와 별개인 일반 Client/Server 게시 소비자를 위한 호환이며 실제 WorldGameplay publish는 실행하지 않았다. folderId 누락은 기존 문서와 호환된다. 존재하지 않거나 다른 Gate Parent는 해당 Pattern의 원문을 보존해 격리한다. Parent 분류는 기존 Pattern ID·Bundle member 실행 계약을 바꾸지 않으며 Server bootstrap 행과 protocol 변경은 없다.

### 이번 후속 구현의 최종 자동 검증

| 검증 | 실제 결과 |
|---|---|
| 최종 Client 최소 컴파일 | Debug ClCompile exit 0. Parent consumer/Document H 변경 후 컴파일과 후속 Workbench 재컴파일 모두 성공. `out/CompositionSelectionCatalog/client-parent-compile.log`, `client-final-compile.log`. 기존 C4819 인코딩 경고는 남아 있으며 변경 파일 인코딩/줄바꿈은 유지했다. |
| 기존 native editor build/run | Build exit 0, `--kouku-composition-editor-contract` PASS. Parent+Bundle None 생성, 정상 Bundle 생성, 기존 Pattern의 Parent 변경/해제, clip·Bundle 불변, Save/Reload, 잘못된 Parent/Bundle 실패 보존과 기존 249-stage/선택 편집/CAS 검사를 실행했다. `editor-parent-build.log`, `editor-parent-test.log`. |
| native harness 연결 | 기존 Workbench의 카메라 API 참조로 발생하던 unresolved symbols는 기존 test CPP에 harness define 안의 throwing seam으로 해결했다. 카메라 호출 시 성공을 위장하지 않고 테스트가 실패하며 Client 구현이나 새 harness는 추가하지 않았다. |
| Parent projector/publisher | 신규 focused 5개 PASS. Python 전체 78개는 69개 성공, 현재 live authoring/Product에 의존하는 기존 9개 실패. 이번 projector 추가만 메모리에서 제거한 기준에서도 같은 9개 실패를 재현했다. `parent-projector-test-summary.json`. 사용자 데이터 변경으로 fixture를 맞추지 않았다. |
| 구조 검사 | 현재 Composition/World JSON read-only parse, Client project/filters 및 native harness project XML parse, Gameplay publisher PowerShell parse, git diff --check PASS. `final-check-summary.json`. |
| EXE 반영/화면 | 사용자 Client 종료 후 후속 Product 빌드로 링크·배포했다. 최종 증거는 G10/G11을 따른다. Client/UI 자율 실행·조작·화면 검증은 하지 않았다. |

이번 후속 작업은 사용자 Composition/World authoring을 저장·publish하거나 Append·배치하지 않았다. 사용자 저장이 진행되는 기존 dirty worktree를 보존했으며 자동 stage/commit/push도 하지 않았다. 사용자는 새 EXE 반영 후 Create Pattern에서 Parent=조커찾기, Bundle=None으로 생성하거나, 기존 성공 Pattern Detail의 Parent=조커찾기로 바꾸고 Composition Save한다.

## G10. 카드별 접촉 Trigger와 Shared Logic 실제 소비자

`TRIGGER / OBJECT_CONTACT`와 카드 WORLD occurrence ID 목록(1~64개), 카드 반경, Contact Group/Priority를 연결했다. 같은 저장 Motion으로 만든 카드도 occurrence와 실행 cue로 구분한다. `PLAY_CONTACT_WORLD_OBJECT_MOTION` Result의 각 대상별 saved Motion map으로 닿은 카드만 반응한다. 같은 타격 Group/시각에서 높은 Priority가 먼저 반응을 소비하며, 다른 시각에는 Group을 재사용할 수 있다. 중앙과 가장자리를 각각 별도 Trigger 창에 연결하고 중앙 Priority를 높이는 입력을 지원한다.

전체 제한시간은 `DURATION / EXTERNAL_SIGNAL`을 사용한다. 접촉 Success의 `COMPLETE_LOGIC_WINDOW`가 optional 조커 occurrence 조건에 따라 이 창에 성공을 전하고 기존 성공 Pattern Result를 실행한다. 성공 없이 마감하면 기존 Timeout Result를 실행한다. contact를 timeout보다 먼저 평가하고, Brain의 마지막 stage가 신규 contact/signal deadline 전에 끝나던 한 tick 경계도 수정했다. 기존 중간 stage action-start clock은 보존한다.

Source reader/writer/validation, Workbench 정의·Result·Shared Logic 선택/시간 동기화, projector/publisher, Server catalog/Brain admission/LogicRuntime, GameRoom의 정확한 WORLD cue 대상, 기존 Client Motion 소비자까지 연결했다. Bone/CONTACT를 parser에만 추가하고 실제 Play admission에서 거절하던 누락도 같은 변경에서 수정했다. 기존 `OBJECT_OVERLAP`은 하나의 카드 대상 계약으로 유지하며, 같은 saved instance의 여러 배치는 새 CONTACT를 사용하도록 거절 이유를 보존한다. Pattern WORLD 최대 수는 128개로 맞췄다.

단일 Pattern 저작 Preview도 WORLD box별 player를 소유하도록 바꿨다. 같은 saved instance를 두 번 놓으면 각 box가 독립 생성·seek·stop되며, 연동 Collider가 정확한 occurrence를 찾는다. Product는 기존 run/member/cue별 clone 경로를 그대로 사용한다. Shared wire를 새로 만들거나 사용자 Logic/Pattern/카드 배치를 자동으로 다시 저장하지 않았다.

## G11. 실제 뿅망치 Bone Anchor와 서버 수치 궤적

Collider 상세의 BOSS 아래 BODY/WEAPON target과 실제 모델 Bone 목록을 추가했다. 별도 무기 모델을 AnimationTargetService의 typed view로 연결하고, 단일 Preview/Bundle/Product가 같은 실제 무기 부착 행렬을 사용한다. 제품 CNpc에서도 body clip과 대응되는 hammer clip을 같은 source 시간에 재생한다. Bone 이름이 없으면 보스 원점으로 대체하지 않는다. Source optional `boneTarget`은 누락 시 BODY로 호환되며 named WEAPON anchor는 BOSS/followBoss Collider에 한정한다.

Publisher는 기존 WModel reader/sampler와 BossCatalog의 scale/weapon attachment를 사용해 body 또는 weapon Bone 끝점의 boss-local 궤적을 생성한다. Server는 기존 worldTrack의 수치 위치에 현재 boss transform을 합성한다. Bone pitch/roll은 끝점 위치에 반영하지만 형상/저작 offset은 기존 TARGET_YAW 계약을 유지한다. 타격 활성화는 짧은 Trigger 창이 소유하며 높이 자체로 접촉을 켜지 않는다. named-bone CONTACT가 활성화된 Pattern에만 derived `unblendedBoneContact` binding을 출력하여 제품 전환 50ms blend와 baked pose의 차이를 없앤다.

기존 물리 자산 `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel`과 별도 무기 폴더의 `Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel`을 읽었다. 새 binary/Resource를 생성하거나 이동하지 않았으며 Drive 추가 전달 대상은 없다. 실제 끝부분 Bone 선택과 offset의 시각 확인은 사용자가 한다. 구체적인 편집 순서는 팀 Animation Tool 인계 문서 17.7을 따른다.

### 자동 검증과 실행 경계

- 기존 native editor build와 `--kouku-composition-editor-contract` **PASS, exit 0**. 선택 Duplicate/Earlier/Later, Parent+Bundle None, contact 다중 대상/Result/Shared window, WEAPON Bone Set→Save→Reload, 잘못된 Bone target/anchor/follow 및 mapping 실패 시 기존 저장 보존을 함께 확인했다. `out/CompositionSelectionCatalog/editor-bone-build-final.log`, `editor-bone-test.log`.
- 기존 UI 없는 editor harness가 새 Bone picker의 service symbols를 참조한 링크 오류는 기존 test CPP의 throwing stubs로 해결했다. UI service를 실제 호출하면 실패하며 동작 성공을 위장하지 않는다. EngineSDK는 Client의 정본 PrepareEngineSdk로 갱신했다.
- Server Debug Build와 기존 object/bundle contract **PASS, failures 0**. 실제 catalog→GameRoom→Brain→contact/signal 실행, 맞은 occurrence만 Motion, 미접촉 마감 Timeout, 마지막 tick 성공, fractional tick Bone 및 boss transform 합성까지 검사했다. `server-bone-build.log`, `server-bone-contact-test.log`, `server-bone-bundle-test.log`.
- 실제 무기에서 bake한 105개 key/2002ms Trigger와 publisher 118행을 temp DataRoot에 합성한 기존 native catalog/Room admission **PASS**. 잘못된 Bone track scale은 거절하고 fixture 원문을 복구했다. `server-bone-generated-admission.log`, `server-bone-generated-rejection.log`. 제품 Bin/Data를 교체하지 않았다.
- 최종 Product runner **PASS**. Engine→Shared→Server→Client compile/link, SDK/shader/DLL 배포를 완료했다. `out/BuildPipeline/runs/20260907T135526985Z-debug-product.json`, `out/CompositionSelectionCatalog/product-bone-build-final.log`. Client EXE 2026-09-07 22:55:26 KST, Engine DLL 22:54:30 KST, Server EXE 22:48:19 KST. 기존 C4819/LNK4099 경고는 있으며 오류 0이다. Client/UI는 실행·조작·캡처하지 않았다.
- 실제 WModel Bone/CONTACT/기존 overlap focused **15개 PASS**, 실패 0. 마지막 pose 유지 구간의 계속되는 source clock, native clip 끝 clamp, 분수 tick 양쪽 key 일치, 잘못된 asset/Bone/clip/gap 거절, enabled Bone CONTACT에만 unblended binding 출력까지 확인했다. `bone-projector-focused-tests.log`, `bone-projector-focused-tests.summary.json`.
- body root suppression 기준은 실제 MN_RPCT_06 skeleton/idle/34개 clip 첫 pose에서 X/Y 0(최대 오차 5.36e-9 모델 단위)으로 일치했다. 미대응 weapon clip은 immutable rest를 사용한다. 망치 8_01/02/03의 unkeyed root bone들은 모든 clip에서 비애니메이션 bone이다. `bone-channel-coverage.json` 및 실제 모델 분석을 근거로 기존 native sampler와 행렬·channel 소비 순서를 검토했다.
- Native bone 좌표 9개 샘플의 직접 실행 비교는 **미실행**이다. 기존 native 검사 도구에는 실제 CModel pose 비교 옵션이 없으며 이를 위해 새 harness나 Client UI를 실행하지 않았다. single Preview는 기존 원본 root 이동을 표시하므로 8_03의 1초 표본에서 Product의 root suppression과 약 9.3mm 차이가 있다. 각 경로의 Collider는 그 경로에서 실제 표시하는 Bone을 따른다. 최종 화면과 타격 위치는 사용자 확인으로 남긴다.
- 변경 계약의 JSON 3개 및 project/filter XML 5개 parse, Gameplay/WorldGameplay publisher PowerShell 2개 parse, `git diff --check` **PASS**. Git의 기존 줄바꿈 정규화 경고는 공백 오류와 구분했다.

이번 후속에서 Composition revision115, World revision398과 BossCatalog를 read-only parse했다. 사용자 저작 JSON을 publish하거나 Append·배치하지 않았고 기존 dirty worktree를 보존했다. 사용자는 카드/Trigger/Result/Bone을 연결해 Save/Publish하고 Server 재시작 후 직접 Complete Play로 검증한다. 자동 검증은 사용자의 화면 관찰이나 최종 Bone 부위 판정을 대신하지 않는다.

## G12. World Object 배치 후속

사용자 재개 요청으로 Object 단일 목록·카드 Append·개별 절대 Map Transform과 Save 자동 적용을 완료했다. Shared protocol67, 실제 Server/Client 배치 소비 및 Debug EXE 배포의 정본 결과는 [World Object Tool RESULT G10](2026-09-07_WORLD_OBJECT_TOOL_RESULT.md)이다. 기존 CONTACT/Bone Logic 저작은 유지하며 사용자 Collider/Logic을 자동 연결하지 않았다.


## G13. Logic 겹침 표시 — Debug EXE 배포 완료

2026-09-08 Workbench Render_Timeline의 Logic 단일 행을 interval 행 배정으로 교체했다. 같은 시점에
겹친 후순위 Logic은 24px씩 위에 표시한다. 22px 박스와 최소 8px 표시 폭을 충돌 기준에 사용하고,
canvas 및 아래 트랙도 필요한 만큼 확장한다. InvisibleButton과 DrawBox는 동일 logicBoxY를 사용한다.
저장된 시점·수명·ID·Logic 연결·기존 drag commit 경로는 변경하지 않았다. 새 파일/프로젝트 등록은 없다.

동일 시점 2개·3중 겹침·맞닿은 끝/시작·비중복·축소 최소 폭의 수치 5사례와 source 검토를 수행했다.
Client 최종 ClCompile 및 git diff --check PASS. 로그는 out/CompositionCardLivePreview에 있다.
기존 native editor 검사는 카드 편집/Logic 저장 계약이 PASS이며 실제 UI layout을 실행한 검사는 아니다.
사용자 종료 확인 후 최종 Debug Product 링크/배포는 PASS이며, 실제 위 행 박스 선택·드래그 화면 확인은 사용자 확인 대기다.
카드 Position 후속은 World Object Tool RESULT G11을 따른다.


## G14. 이름만 생성한 Trigger 표시와 카드 접촉 설정

Composition revision121에서 사용자 Trigger22/23은 이름과 TRIGGER 종류만 있었고 triggerKind는
비어 있었다. Publish 실패가 아니라 기존 compatible-only UI 필터가 해당 정의를 숨긴 것이 원인이다.
Collider Logic definition에 name-only TRIGGER를 `(set Trigger kind)`와 함께 표시하도록 수정했다.
Use as Card Contact Trigger 버튼은 기존 Logic 입력 draft에 OBJECT_CONTACT/radius1을 stage한다.
기존 Render_LogicDefinitionValues를 재사용해 대상 카드를 체크하고 Apply Values로 검증·commit한다.
카드 대상을 자동 지정하거나 최소1개 대상 검증을 완화하지 않는다. Apply 실패 상태는 같은 영역에
즉시 표시하고 Collider의 미저장 Transform 입력을 보존한다. helper commit 뒤 무효화된 Pattern/box
참조를 사용하지 않도록 정의 복사본으로 호출한 뒤 즉시 반환한다.

Shared Logic은 설정된 해당 Pattern의 ENTER_AREA/OBJECT_CONTACT 창만 표시하며 빈 이유를 설명한다.
명시 ENTER_AREA 선택 전에는 damage 생성 UI를 표시하지 않는다. 저장되지 않은 이름만으로 접촉
판정이 실행되거나 Collider를 Result에 직접 연결하는 별도 경로를 만들지 않았다. 사용자 Logic/
Outcomes/Anchor/배치 JSON은 수정하지 않았다.

최종 Debug Product compile/link/deploy PASS, 20260907T161917966Z-debug-product.json.
기존 native editor의 OBJECT_CONTACT 생성·여러 Collider 연결·대상별 Motion/Save/Reload와 카드
Position 회귀가 PASS이며, 새 버튼의 실제 UI 입력/표시는 사용자 확인 대기다. C++ 인코딩/CRLF
및 git diff --check PASS, 이번 UI 후속의 변경 JSON/XML 없음. 세부 최종 빌드와 EXE 시각은
World Object Tool RESULT G11을 따른다. 다른 Codex 작업에 메시지를 보내지 않았다.


## G15. 공통 트랙 겹침 배치 — Debug EXE 배포 완료

2026-09-08 사용자 첨부 화면에서 Logic만 위쪽 숫자 행으로 분리되고 World가 한 행에 겹치는 차이를
확인했다. Render_Timeline의 가족별 packing을 한 개의 TIMELINE_INTERVAL/TIMELINE_LANE 경로로
통합했다. Animation/Logic/Summon/World/Scene Profile 및 Effect/Sound/Camera/Collider/Light의
10개 박스 트랙이 같은 시작 시각·최소8px 표시 폭 기준으로 겹침을 나눈다. 트랙 이름은 한 번만
표시하고 추가 박스는 그 트랙 내부에서 아래로 쌓인다. G13의 Logic2/3 및 Animation Overlap 숫자
이름과 Logic만 위로 배치하던 특수 처리는 제거했다. World의 같은 시점 여러 카드도 각각 보인다.

lanes의 실제 행 수를 누적해 다음 트랙 시작과 canvas 전체 높이를 계산한다. laneY/boxY를 그리기와
InvisibleButton에 같이 사용하고 Animation marquee, playhead, 세로 스크롤의 범위도 같은 결과를
소비한다. Camera return tail은 실제 본문 폭+blendOut만큼 겹침 구간에 포함한다. 양수 duration을
누적하는 Stages는 순차 한 행을 유지한다. 시각 배치 변경이며 저장 ID/시점/수명/Logic/drag commit과
Append 생성·개별 Transform 동작은 바꾸지 않았다. 새 C++/project/filter/JSON 변경은 없다.

| 확인 | 실제 결과 |
|---|---|
| 독립 source 검토 | 10개 가족 공통 배정, draw/hit/marquee/camera tail/height 연결 및 동률 저장 순서 보존 확인 |
| 구조·인코딩 | structure-check.json: 공통 allocator1개, 숫자 label 제거, hit/draw 좌표 일치, UTF-8/CRLF 보존 |
| 사용자 저장본 | source revision125의 SHA-256이 작업 전후 동일. 사용자가 지운 카드/Logic을 자동 복원하지 않음 |
| 최종 Debug Product | PASS, out/BuildPipeline/runs/20260907T165943653Z-debug-product.json. Engine→Shared→Server→Client compile/link 및 SDK/shader/DLL 배포 |
| 공백 검사 | git diff --check PASS |
| UI 화면·선택 | 에이전트 실행·조작·캡처 없음. 재추가 후 실제 화면/선택은 사용자 확인 대기 |

로그는 out/CompositionUnifiedLanes에 있다. Client EXE는 2026-09-08 01:59:43 KST, Engine.dll은
01:56:51, Server EXE는 01:57:13이다. 기존 인코딩/셰이더/PDB 경고는 남고 컴파일·링크 오류는 0이다.
사용자 종료 확인 뒤 배포했으며 Server+Client profile Ctrl+F5로 실행해 World/Logic 박스를 같은
시점에 여러 개 Append하고 각각 선택한다. 별도 하네스나 광역 진단을 추가·실행하지 않았다.
다른 작업에 메시지를 보내거나 사용자 데이터를 publish·Git stage/commit/push하지 않았다.


## G16. 쿠크 본체 구간 이동과 도착점 유지 (2026-09-08)

구현한 `bossMotion`은 PATTERN_8 본체의 월드 시작점 `(2.04, 10.56, 316.95)`과 도착점 `(11.79, 10.56, 326.79)` 사이를 패턴 시각 `1870~5780ms`에 선형 보간하고 이후 도착점을 유지한다. yaw는 `314.7368`이다. Composition parser/validator/serializer 및 Pattern Detail의 `Move boss during Pattern`, 시작/종료 시각, XZ·baseY·yaw 편집이 같은 값을 소비한다. spawn reset 또는 REAL_GAZE_TELEPORT와 동시 사용은 거부하며 실패한 candidate는 기존 저장본을 교체하지 않는다.

Server Gameplay publisher는 `PATTERNBOSSMOTION` 행을 생성하고 catalog가 시간·좌표·동시 writer를 검증한다. GameRoom audition은 전체 participant를 stage하기 전에 시작점·끝점·경로의 navigation을 검증한다. Kouku brain은 시작 staged entity를 시작점으로 초기화하고 매 tick 절대 패턴 시각으로 위치와 yaw를 대입한다. Logic이 현재 위치를 읽기 전에 같은 샘플을 적용하며 단독 Brain Update에도 연결했다. 30Hz의 구간 경계는 최대 한 tick 뒤에 관측된다. 자연 완료는 Transform을 reset하지 않으므로 도착점이 유지된다. 기존 `Build_WorldEntitySpawnedPayload`와 snapshot의 XYZ/yaw가 이를 전달한다.

Client `CNpc`의 기존 `b_root` local X/Y 억제와 source Z 보존은 유지했다. 이 asset의 source Z는 변환 후 월드 Y pose이므로 원본 점프·상하 움직임이 남고, 별도 arc/height 곡선은 추가하지 않았다. Server body의 기준 Y는 10.56이며 mesh의 시각적 높이는 animation pose가 소유한다. Play Bundle은 공용 `Sample_KoukuSaydonBossMotion`을 사용하고 Model Reference는 기존 제자리 비교를 유지한다. Product binding은 bossMotion 및 BOSS_SPAWN World Object occurrence의 `worldEmissionAnchors`를 투영하여 root가 연결한 emission birth anchor callback에 정확한 패턴 시각과 offset을 제공한다.

| 검증 | 결과 |
|---|---|
| 새 projector focused tests 2개 | PASS: motion의 Server/presentation 투영, 복사 격리, emission anchor, 잘못된 시간/Y/NaN/yaw/reset 거부 |
| Gameplay/World publisher PowerShell parser | PASS, 구문 오류 0 |
| 소유 파일 git diff --check | PASS. 전체 작업의 기존 사용자 ArenaCameraProfile.cpp:53 탭은 보존 |
| Server native focused tests | 정상 Room 이동, 1870 직전 정지, 5780 이후 도착/완료 유지, tick wrap, 실패 시 보존 케이스 추가. 실행은 root 담당 |
| 최소 컴파일 | root Shared/Server/Client ClCompile PASS. 최종 Product link/deploy와 native focused 실행은 root 통합 검증 기록 참조 |
| Product publish | root source revision133 Composition projector 및 GameplayBalance publish PASS, World/최종 Product 통합 검증 진행 |
| 실제 9시 방향·점프·공 생성 | 사용자 Play Bundle/Server 재생 관찰 대기 |

focused Python 원문은 `out/KoukuBossMotion/boss-motion-projector-tests.log`에 있다.

Composition JSON의 최종 revision/CAS와 P8/P9·Bundle1 PRODUCT 승격은 root가 소유했다. product pattern의 authored order와 playAllPatternIds 일치 검증을 유지했다. 새 C++ 파일·Shared packet·대체 모델 런타임은 추가하지 않았다. 필요한 runtime model은 기존 Resources-relative `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel`과 동시 재생하는 `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel`을 그대로 사용한다. 물리 위치는 `Client/Bin/Resources/` 아래이며 새 binary나 Drive 전달 팩을 만들지 않았다.

## G17. 조커찾기 망치 Collider 앵커와 선택 Preview 수정 (2026-09-08)

기존 revision133의 PATTERN_13 Collider 네 개는 BOSS/BODY와 빈 Bone을 사용했다. 실제 망치 끝이 연결된 상태가 아니었다. revision134에서 presentation.4/.5/.6/.7을 WEAPON/b_rpct_01로 연결했다. occurrence ID, 시작/수명, debugRender, 크기, offset, Logic과 다른 Pattern은 보존했다. 중앙/주변은 현재 offset0과 2m 크기가 같아 활성 시간이 겹치면 같은 wire처럼 보일 수 있다. 상세 위치·크기 조정은 사용자가 한다.

BossCatalog가 사용하는 몸 모델은 Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel, 별도 무기는 Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel이다. 실제 무기 8개 Bone 중 b_root는 원점, b_rpct_01/b_rpct_03은 양쪽 말단이다. ao_att_battle_1_03과 대응 무기 clip의 pose를 수치로 읽어 내려치는 쪽 b_rpct_01을 기본 연결했다. 최종 표면상의 부위와 offset은 사용자 육안 확인 대상이다. 기존 G11의 잘못 적힌 무기 폴더 설명도 교정했다. Resources 물리 위치는 Client/Bin/Resources 아래이며 새 binary/Drive 추가 전달 대상은 없다.

선택 Collider 상세의 Preview는 animation 없는 독립 resource 경로로 전달되어 플레이어 body/pivot를 사용하고, 세이튼의 WEAPON view가 누락되면 Collider가 실패 상태로 격리됐다. Request_ColliderBoxPreview는 기존 Pattern Preview를 재사용하여 원본 보스, body/weapon clip, 패턴 시각을 유지하고 해당 Collider Start에서 재생한다. 선택한 Box의 미적용 Detail만 임시 Pattern 사본에 반영하며 원본 draft/저장본은 바꾸지 않는다. 다른 Collider도 각자의 활성 시간에 같은 망치를 따른다. 잘못된 owner/resource/anchor/시간/transform은 이전 pending 요청과 draft를 보존한다. 문법상 유효한 Bone의 실제 존재 여부는 기존 runtime 모델 소비자가 확인한다.

Request_PatternPreview는 이전 단독 presentation 요청을 정리하여 MainApp의 뒤쪽 소비가 새 Pattern 요청을 덮어쓰지 않게 했다. bossMotion이 있는 Pattern의 기존 임시 Bundle Preview 분기도 요청된 Pattern 사본을 사용하여 Detail 편집값을 보존한다. 별도 모델 runtime이나 새 C++ 파일/project 등록은 추가하지 않았다.

| 실제 확인 | 결과 |
|---|---|
| 기존 native editor harness Debug Build 및 --kouku-composition-editor-contract | PASS, exit0. 새 WEAPON Collider 요청의 actor/전체 clip/시각/다른 Box/편집 복사본 보존과 잘못된 요청 10종 거부, 기존 Save/Reload/CAS 검사 포함. 임시 DataRoot에서 실행 |
| 실제 WModel Bone projector focused 3개 | PASS. body/hammer/TARGET_YAW, quantized stage/tick bracket, 잘못된 Bone/clip/asset/sample gap 거부 |
| 저장된 네 Collider의 실제 pose 수치 | PASS. 각각 48/50/53/53 key가 유한하고 망치 animation에 따라 이동. saved-collider-pose-check.json. Client 화면 실행 결과는 아님 |
| KoukuSaydon domain publish | PASS, expected sourceRevision134. Composition/patternbindings/encounter revision 일치와 GameplayBalance publish 확인. PATTERN_13은 기존 DRAFT 유지 |
| 최종 Client Debug Build | PASS, exit0. 참조 Engine 포함 compile/link, SDK 및 shader/runtime DLL 배포. client-build.log |
| JSON/XML | 변경 JSON 3개 및 기존 Client project/filter, 기존 harness project XML parse PASS. source revision과 네 Bone 연결 외 JSON 의미가 기준본과 같음을 확인 |
| 공백/인코딩 | 변경 범위 git diff --check PASS. 전체 dirty tree에서는 기존 ArenaCameraProfile.cpp:53 탭만 실패하며 보존. 기존 C++ UTF-8 및 파일별 줄바꿈 유지 |
| 독립 코드 검토 | 추가 범위에서 재현 가능한 P1/P2 없음. 실제 animation sample→weapon sync→같은 모델 WEAPON view 소비 확인. dirty WIP의 최종 승인 판정은 아님 |
| Client/UI 조작·화면 | 미실행. 위치·크기·말단과 실제 wire 이동은 사용자 확인 대기 |

로그는 out/KoukuHammerAnchorPreview에 있다. 최종 Client EXE는 2026-09-08 12:15:30 KST, 배포 Engine.dll은 12:15:24 KST다. Server EXE는 이번 Client 수정으로 다시 빌드하지 않았으며 기존 11:55:37 KST 파일을 유지한다. 최종 확인 시 Server/Client는 종료 상태이고 listener는 꺼져 있다. 기존 인코딩/PDB 경고가 있으며 최종 컴파일·링크 오류는0이다. 대규모 기존 dirty 변경을 보존했고 Git stage/commit/push하지 않았다.

사용자는 Server + Client profile을 Ctrl+F5로 실행하고 KoukuSaydon Arena → F1 → KoukuSaydon Action Workbench → 2관문 → 조커찾기 → 대형세이튼_조커찾기를 선택한다. Collider 중앙/주변 Box에서 Debug Render와 WEAPON/b_rpct_01을 확인하고 Position Offset, Width / height / depth를 조정한 뒤 Preview한다. Apply → Save는 조정값을 저장하고 Play Pattern은 적용된 전체 Pattern을 현재 커서에서 재생한다. 상세 Preview의 위치 입력은 임시 사본에만 적용된다.

G17 당시 완료 범위는 앵커 연결과 로컬 저작 재생이었고 P13 카드 WORLD와 Collider Logic 연결은 비어 있었다. 이후 사용자 카드 배치·중앙/주변 튜닝을 보존한 실제 서버 타깃 지정과 뒤집기/들썩임 연결은 아래 G21에 기록한다.


## G18. 패턴별 root 수직 높이 0.8배 (2026-09-08)

G2 쿠크의 실제 MN_RPCZ_00.wmodel과 BossCatalog의 bodyModelPreScale 0.012053 / presentationScale 1을 읽었다. rpcz00_att_battle_7_01은 30Hz, 222 tick, 7400ms다. b_root의 source local Z가 변환 후 월드 Y가 되며, 5433.333ms의 원본 최고 상승량은 17.846225335m다. PATTERN_8의 animationRootVerticalScale 0.8은 최고 상승량을 14.276980268m로 낮춘다. Server 기준 Y 10.56을 더한 root 최고 위치는 24.836980268이다. 이는 머리나 mesh 상단 높이가 아닌 root translation 실측이다. 수평 bossMotion은 5780ms에 도착하지만 원본 수직 pose는 약6100ms에 착지한다. 두 시각과 모델 크기, animation 재생시간을 변경하지 않았다.

Composition의 optional animationRootVerticalScale은 유한한 0..1만 허용하고 누락은1이다. Pattern Detail의 Animation jump height 입력과 parse/validate/Save/Reload, Product pattern presentation 및 해당 pattern의 action binding, Client action reader와 CNpc Play_NetworkAction까지 연결했다. Server의 위치·body Y·snapshot 계약은 그대로다. publisher가 Bone Collider track을 bake할 때 같은 body root 배율을 사용하여 bone 위치와 표시 pose가 일치하도록 했으며, weapon 자체의 local pose와 다른 pattern의 binding은 변경하지 않는다. root가 관리한 원본 source revision135의 PATTERN_8에0.8이 저장되어 있다.

Engine CModel은 기존 root suppression의 보존축에 rest + (원본 sample - rest) × scale을 적용한다. 실제 animation 평가와 비파괴 bone sampler가 같은 helper를 사용한다. 배율 적용 전 root translation을 보관하고 다음 unkeyed fallback과 blend 시작에서 복구하여 프레임마다0.8이 반복 곱해지는 일을 막는다. clone은 배율과 원본 cache를 값으로 복사한다. 기본1에서는 cache 복구와 해당 축의 배율 산술을 건너뛰므로 기존 외부 BoneLocal 수정과 원래 값이 유지된다. 1에서 다른 배율로 바꿀 때는 현재 원본 local을 저장하고, 다른 배율에서1로 돌아갈 때는 그 원본을 복원한다.

Server action edge 및 late-join seek 전에 배율을 적용하며 다음 일반 action·idle·death는 기본1로 돌아간다. Preview는 actor별로 배율을 적용하고 정상 종료·중단·해제 시1로 복구한다. 배율만 지정된 단독 Pattern도 기존 actor 소유 임시 Bundle 경로를 사용한다. Model Reference는1을 유지한다. 실제 Product action 전환은 Set_AnimTrackPosition / Skip_Blend / Update_Animation(0)으로 새 pose를 평가한다. 임의 Engine blend 도중 배율을 부드럽게 애니메이션하는 추가 계약은 도입하지 않았다.

| 검증 | 실제 결과 |
|---|---|
| 기존 Python projector focused 3개 | PASS, 3 tests / 2.069s / exit0. pattern별 binding 투영, 유한 범위 거절, 기본값 생략, 실제 WModel의 원본·0.8·0배율·6100ms 착지, 배율별 cache 격리 및 기존 body/hammer TARGET_YAW bake 확인 |
| 실제 최고 상승량 | 원본17.846225335m → 0.8배14.276980268m. imported 회전 basis의 약0.0000006m 수평 성분 차이는 수치 오차 범위이며 수평 이동 계약은 보존 |
| 기존 native editor Save/Reload | Debug build 및 --kouku-composition-editor-contract exit0. 0 / 0.8 / 1 Save→Reload와 잘못된 음수·1초과·NaN·무한대에서 LastGood/원문 보존 PASS |
| live/비파괴 pose·blend·cleanup | 실제 호출 경로와 공용 helper를 독립 검토. live CModel 실행 비교는 미실행이며 Python 수치 PASS와 구분 |
| 소유 C++/projector 공백 검사 | git diff --check PASS. 다른 사용자의 기존 ArenaCameraProfile.cpp:53 탭은 보존 |
| Product compile/link 및 publisher | 마지막 Model.cpp 기본1 보존 보완 포함 Debug Product PASS(20260908T034213853Z-debug-product.json). Map/Composition135/GameplayBalance publish PASS. 상세 통합 증거는 Effect Composition RESULT G21 |
| Client/UI 조작·최종 점프 높이 | 미실행. 사용자의 실제 Play Bundle / Server 재생 확인 대상 |

Python 원문 명령·출력은 out/KoukuBossMotion/root-vertical-scale-tests.log에 있다. 실행한 unittest는 test_animation_root_vertical_scale_projects_only_its_actions, test_animation_root_vertical_scale_matches_original_rise_and_cache_isolation, test_bone_contact_bakes_real_hammer_and_body_with_target_yaw다. 기존 source와 effect/camera/망치 Preview 수정은 보존했고 새 하네스나 C++ 파일, project 항목, Shared packet을 만들지 않았다. runtime model은 Client/Bin/Resources/Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel을 그대로 사용하며 신규 binary나 Drive 전달 대상은 없다.


## G19. Stop 포즈 유지와 정지 Scrub 최종 통합 (2026-09-08)

Workbench의 Stop/Stop Bundle은 표시된 clock을 다시 샘플한 뒤 Pause한다. Pattern/Bundle의
기존 Pause 버튼도 같은 clock 요청을 사용하므로 Update 한 프레임만큼 더 진행한 위치에서
멈추지 않는다. Reset/Reset Bundle은 기존 해제와0ms 초기화를 수행한다. 내부 선택 변경·Reload
cleanup과 Server Stop 계약은 유지한다. stopped/inactive ruler drag는 paused preview를 시작하고,
이미 재생 중이면 해당 시각으로 Seek한다. 끝시각은 pose를 유지하며 Play/Resume은0ms부터 시작한다.

같은 Pattern을 정지한 뒤 Collider Detail Preview를 누르면 정지 clock, 임시 Box 편집값과 원래
BODY/WEAPON animation을 함께 샘플한다. 망치 b_rpct_01 앵커와 실제 [start,end) 활성 창은 유지한다.
AnimationTool 및 PresentationPlayer 실제 소비 경로를 검토했고 actor/Effect 정지 및 끝시각 유지가
기존 preview runtime으로 연결됨을 확인했다. 새 C++ 파일/project/filter 항목은 없다.

| 검증 | 실제 결과 |
|---|---|
| Debug Product compile/link/deploy | 최종 PASS, out/BuildPipeline/runs/20260908T044302127Z-debug-product.json. Engine/Shared/Server/Client 모두 PASS, runtime 입력 누락 없음 |
| 기존 native harness build 및 focused transport 실행 | PASS, --kouku-preview-transport-contract exit0. cold/live/paused/end Pattern·Bundle Seek, 표시 clock Pause, 끝에서 Play 재시작, paused Collider 수정본/시각, one-shot 및 source/dirty/generation 보존 |
| 단일 Pos 검사 | CameraTrackContractTests 3개 PASS, 기존 Valtan Camera Tool contract PASS. 상세는 Effect Composition RESULT G23 |
| domain publish | Map Area 8파일, Composition source136, GameplayBalance publish 모두 exit0 |
| JSON/XML | Camera71·World402 source/runtime 동일성, P1 원본 pose 보존, 공 scale1.5 외 원본 보존, Composition136와 Product sourceRevision 일치, 기존 project/filter XML3 parse PASS |
| 공백 검사 | 요청 범위 git diff --check PASS. 사용자의 기존 ArenaCameraProfile.cpp:53 탭은 보존·제외 |
| 실제 Client/UI 입력·망치 wire/카메라/공 화면 | 에이전트 실행·조작·캡처 없음. 새 EXE의 시각 결과는 사용자 확인 대상 |

원문 로그와 수치는 out/KoukuPausedScrub20260908의 product-final-build.log,
transport-harness-build.log, transport-harness-test.log, map-publish.log,
composition-publish.log, gameplay-publish.log, final-data-validation.json에 있다.
이 PC는 LAN server-host이며 사용자는 Server + Client profile을 Ctrl+F5로 시작한다.
Composition → 2관문 → 조커찾기 Bundle → 대형세이튼 패턴에서 Play 후 Stop,
내려치는 시각으로 ruler drag → Collider Detail 값 조절 → Preview → Save 순서로 확인한다.
Stop은 preview를 남기므로 완전히 정리하려면 Reset을 누른다.

## G20. 조커찾기 Bundle의 대형 세이튼 중복 표시 정리 (2026-09-08)

실제 source revision135에서 kakulsaydon.bundle.3은 PATTERN_12 쿠크_조커찾기와 PATTERN_13 대형세이튼_조커찾기를 이미 멤버로 가지고 있었다. PATTERN_13의 folderId가 kakulsaydon.folder.3으로도 지정되어 같은 항목이 조커찾기 Parent 직속과 Bundle 자식에 두 번 표시됐다. revision136에서 PATTERN_13의 folderId 한 줄을 제거했다. 기존 멤버 ID, startOffsetMs, animation/Collider/Logic/World/WEAPON b_rpct_01, PATTERN_8 공·점프높이·카메라 등 다른 원본 값은 모두 보존했다. 새 Pattern이나 animation을 만들거나 병합·삭제하지 않았다.

PATTERN_14 대형세이튼_조커찾기_성공은 다른 2333ms clip을 가진 별도 성공 표현이므로 유지했다. 기존 Bundle은 하나의 boss placement에 두 멤버를 허용하지 않는다. 이번 변경은 중복 표시된 PATTERN_13의 소속만 정리한 것이며, 성공 결과의 Logic 연결이나 성공 animation 자동 연속 재생을 추가하지 않았다.

| 검증 | 실제 결과 |
|---|---|
| 기존 Composition validator | projector.validate_document PASS |
| 원본 CAS와 JSON parse | revision135 원문 확인 후 교체, revision136 parse PASS |
| semantic 비교 | revision135→136과 PATTERN_13 folderId 제거만 변경. Bundle 멤버/P13 본문/P14/다른 원본 값 동일 |
| 공백·줄바꿈 | 소유 Source/C++/PLAN git diff --check PASS. 원본 JSON의 기존 혼합 줄바꿈을 정규화하지 않고 유지 |
| domain publish·Client hierarchy 화면 | Composition136/GameplayBalance publish PASS. 실제 hierarchy 표시는 사용자 확인 대상이며 Client/UI 실행 없음 |

검증 수치는 out/KoukuBossMotion/joker-bundle-source-check.json에 있다. G19 끝시각 정지 보완으로 PresentationPlayer는 paused 상태에서 자동 종료하지 않으며, 끝시각 Seek는 Pause를 유지한다. 이후 Resume은 기존 Seek_Preview(0)를 재사용해 첫 pose와 cue 상태를 다시 평가한다. 정상 재생 중 자연 완료와 Collider의 [start,end) 활성 구간은 유지하며, 최종 Debug Product compile/link PASS이며 증거는 G19에 기록했다. C++ 기존 UTF-8/no-BOM·CRLF를 유지했다.


## G21. 조커찾기 Collider 실행 연결과 Stage 방향 통합 (2026-09-08)

Collider Detail의 Logic definition 선택은 UI 값에만 남고 Apply Values는 정의만 저장해 실제
logicOccurrenceId가 비어 있던 경로를 수정했다. Set_ColliderLogicValues가 정의값·Logic 시간창·
Collider link를 한 candidate로 검증하고 Apply Values와 새 정의를 연결하는 Apply가 이 경로를 사용한다. 같은 정의의
정확한 start/duration만 재사용하며 후속 타격은 별도 창을 만든다. 명시 Shared 선택은 그 창의
시간과 결과 연결을 유지한다. 미완성 Trigger·잘못된 target은 draft/ordinal/generation/원문을 보존한다.

Stage의 optional retargetOnEnter는 strict bool, 기본false다. 저장·기본 equality·Stage Detail
checkbox와 기존 RETARGET_RANDOM_ALIVE ENTER 투영을 연결했다. 잘못된 타입과 fixed-yaw
bossMotion 동시 사용은 거부한다. Server는 Stage 진입 때 alive player 위치·yaw를 한 번 저장하고
다음 지정 Stage까지 유지한다. Preview는 같은 actor의 Bundle 경로와 Stage별 yaw cache를 사용한다.
되감기는 cache를 재사용하고 처음 방문한 미래 Stage의 seek는 요청 시점 player를 표본으로 삼는다.
명시 Model Reference는 retarget 없이 제자리 비교를 유지한다. 추가 실측에서 대형세이튼의
얼굴·눈·망치 전방이 actor-local +X임을 확인해 BIG_SAYDON retarget의 +Z 기준 atan2 결과에만
-90도를 더하는 Server/Preview 보정을 추가했다. 다른 archetype·모델 pre-yaw·geometry는 보존했다.
source149 published bootstrap으로 -90도 보정 Server 재빌드와 실제 actor-local +X 축 정렬 검사까지 통과했다.

정본 revision149에서 P13 STAGE_3/8/16만 retarget을 켰다. 사용자 카드 WORLD .5~11의 절대 TRS를
보존하고 수명은0~23157ms로 연결했다. 중앙·양옆 Collider는 모두 WEAPON/b_rpct_01을 사용한다.

| 타격 | 중앙 / 양옆 Shared Logic occurrence | 함께 사용하는 창 |
|---|---|---|
| 첫 번째 | .2 / .3 | 4858~5902ms |
| 두 번째 | .17 / .14 | 11244~12289ms |
| 세 번째 | .18 / .19 | 20134~20484ms, 350ms. authored 최저20191ms와 Server 최저20300ms 포함 |

Logic22/23은 같은 joker.hammer.contact 그룹의 OBJECT_CONTACT이며 우선순위는 중앙200/양옆100이다.
중앙은 일반/조커 flip Motion의 HOLD, 양옆은 hop 뒤 NEXT Idle을 실행한다. 같은 카드에 적용한
높은 motion priority를 Pattern ledger에 남겨 후속 양옆 타격이 뒤집힌 카드를 Idle로 되돌리지 않는다.
조커 WORLD .11의 중앙 접촉만 Result26으로 전체기한 Logic .6을 완료하고 Result21→P14를 실행한다.
전체기한4573~20824ms와 기존 Timeout Result2를 유지하며 짧은 접촉 창에는 Fail/Timeout을 넣지 않았다.
P12/P13/P14와 조커찾기 Bundle3은 PRODUCT다. P6/P7의 PRODUCT 변경도 같은 revision149에 포함된다.

Patterns 목록의 Set Pattern to PRODUCT는 기존 Set_PatternAuthoringStatus를 재사용한다. 선택한
Pattern을 검증해 draft 상태를 바꾸며 Save 필요와 Server 재시작을 안내한다. 기존 Save/publish를
생략하거나 별도 실행 경로를 만들지 않는다. 새 C++ 파일·project/filter·Shared packet은 없다.

| 자동 검증 | 실제 결과 |
|---|---|
| 기존 native harness Debug x64 / focused transport | PASS, BuildProjectReferences=false 및 --kouku-preview-transport-contract exit0. pause/scrub/Collider Preview와 atomic Apply·두 타격·Shared·실패보존·Save/Reload·Stage bool 검사 |
| 기존 projector focused | PASS, retarget action 투영·yaw 충돌 거부와 실제 body/hammer TARGET_YAW bake, 2 tests / exit0 |
| Server Debug x64 / --kouku-bundle-contract-test | PASS, failures0. 실제 Room Stage 진입 표본·유지·다음 표본과 malformed/중복/fixed-yaw 충돌 거부, 최종 tick 접촉·성공/Timeout 순서 |
| Server --kouku-object-overlap-contract-test | PASS, failures0. exact owned card·중앙 우선순위·다음 타격 재사용·낮은 priority의 flip 복구 금지 및 millisecond bone clock |
| Composition source/publish | revision149 JSON parse 및 projector publish PASS, PRODUCT11개/88 stages, output2개 |
| 공백·인코딩 | 변경 소유 범위 git diff --check PASS, 기존 C++ UTF-8/CRLF 보존 |

로그는 out/KoukuJoker20260908의 g21-harness-build.log, g21-harness-focused.log,
server-bundle-contract.log, server-object-overlap-contract.log, composition-connect.log,
composition-publish.log와 out/KoukuBossMotion/g21-direction-projector-tests.log에 있다.
이 기록 시점의 최종 Product compile/link/deploy는 별도 통합 검증을 기다린다.
최종 축 보정은 target+X→yaw0, target+Z→yaw-90과 변환한 local+X ray의 dot>0.999999로 확인했다. Client/UI 입력·화면은 에이전트 미실행이며
방향·접촉 위치·카드 반응의 최종 아레나 검증은 사용자 확인으로 남긴다.

### Gameplay publisher 재타겟 Stage action 연결

source revision149 projector가 STAGE_3/8/16에 생성한 optional actions를 기존 Kouku strict-field 검사가 거절하던 누락을 수정했다. 액션은 정확히 한 개의 ENTER/RETARGET_RANDOM_ALIVE/boss.target.pattern/value=1/durationMs=0만 허용하며, BossMotion 병용은 기존 Brain과 동일하게 거절한다. 기존 10-field PATTERNSTAGEACTION과 ordinal0을 사용하므로 새 Server row/runtime은 없다.

PowerShell AST parse, 액션 없음/정상 액션 admission 2종, 종류·trigger·target·value·duration·다중·빈 배열·BossMotion·extra field 거절 9종을 실제 검사 블록으로 확인했다. Publish-GameplayBalance.ps1 -Mode Publish는 source149에서 exit0이며, 생성 Gameplay.bootstrap의 P13 stage.3/8/16에 정확히 세 retarget 행이 있다. 로그는 out/KoukuJoker20260908/gameplay-publish-retarget.log다. scoped git diff --check PASS. Product 빌드나 Client/UI 실행은 이 publisher 보완에서 수행하지 않았다.


## G22. 세이튼 룰렛 PRODUCT와 보스 보행면 (2026-09-08)

P7은 source149에서 PRODUCT이며 기존 룰렛 WORLD의 보행면을 사용한다. Server는 활성 표면에서
정지한 플레이어와 일반 Kouku 보스의 Y를 함께 갱신하고 WORLD 종료·Stop 때 원래 지면으로 복구한다.
원판 밖 보스와 강제 이동·scripted root motion의 수직 권위는 유지한다. 카드 보행면이나 navigation
bake를 추가하지 않았다.

Server Debug 재빌드와 --kouku-support-surface-contract-test는 최종16개 검사/failures0으로 통과했다.
최초 실패3개는 fixture에 있던 기존 bootstrap 보스를 잘못 선택한 검사였으며 격리 fixture를 수정한
뒤 통과했다. 정지 보스 상승·종료·Stop 복구, 원판 밖 위치 보존, 강제 이동/root motion 보존과 기존
플레이어/A-star/line-of-sight 계약을 확인했다. 원문은
out/KoukuJoker20260908/server-support-surface-contract.log다. 최종 Product 빌드와 사용자의 실제
아레나 보행면 확인은 아직 별도 완료 기록이 필요하다.


### Bootstrap 부모 행 정렬 보완

실제 Server load에서 contact mapping이 그 부모 outcome보다, World placement가 그 부모 sequence보다 먼저 기록되어 strict parser가 거절하는 두 결함을 확인했다. Get-BootstrapRowSortKey에서 동일 Logic window/slot의 OUTCOME 전체→CONTACTMOTION→SIGNAL, World SEQUENCE→PLACEMENT→SUPPORT 순서만 추가했다. 출력 행 내용, 종류 내부 순서, 숫자 자연 정렬과 공용 Stage action/volley ordinal 정렬은 유지하고 parser는 완화하지 않았다.

source149 Gameplay publish exit0 후 실제 Gameplay.bootstrap 3201행의 저장 순서로 검증했다. outcome44/dependent45, World occurrence10/child8 모두 부모 선행과 slot dense ordinal을 만족한다. 모든 행 multiset, Stage action/volley94개 순서와 Stage owner76개 순서도 보존됐다. 기존 LOGIC→REGION→REGIONWORLD→REGIONWORLDKEY 및 BUNDLE→MEMBER 즉시 lookup은 현재 종류 문자열 순서로 이미 부모가 선행함을 확인했다.

검사는 Git 제외 out/KoukuJoker20260908/check-bootstrap-logic-order.ps1 -RequirePublishedOrder를 사용했고 결과는 bootstrap-logic-order-check.json, 최종 배포 로그는 gameplay-publish-parent-order.log다. 통합 Server 테스트 담당자가 같은 source149 bootstrap 실제 load를 포함한 Bundle 38 checks, Object overlap 38 checks, Support 16 checks를 재실행하여 모두 failures0/exit0 PASS를 확인했다. 최신 원문은 같은 out 폴더의 server-*-contract.log다. 이 정렬 보완에서는 새 하네스·제품 C++ 수정·Product 빌드·Client/UI 실행을 하지 않았다.

최종 Server 검사는 Bundle38/Object overlap38/Support16개 모두 failures0/exit0이다. 요약은
out/KoukuJoker20260908/server-contract-final.json이며 실제 전방축 실측은
out/KoukuBossMotion/g21-actual-forward-axis.json에 있다. Preview는 동일 보정 코드를 사용하지만
최종 Product 컴파일과 사용자의 실제 화면 확인은 Server 수치 검사와 별도로 기록한다.

## G23. 최종 Product 배포와 맵 카탈로그 입장 오류 확인 (2026-09-08)

최종 Debug Product 빌드는 Engine → Shared → Server → Client 전 단계 PASS/exit0이며
185852ms가 걸렸다. EXE, Engine SDK/DLL, shader와 runtime DLL 배포까지 끝났다.
컴파일 오류는0건이며 기존 C4819 및 Effect shader 경고는 남아 있다. 실행 증거는
`out/BuildPipeline/runs/20260908T062920624Z-debug-product.json`과
`out/KoukuJoker20260908/product-build.log`다. G21/G22와 Effect RESULT G24/G25의
Product 빌드 대기 항목은 이 결과로 완료됐다.

사용자가 중간에 실행한 Client13:46:32는14:57 배포된 Mirror/emissive 재질 입력을 지원하지 않았다.
session34428은 쿠크2회와 Character Select1회 모두 입장 승인 뒤 `CLIENT_LOAD_FAILED`와
`[Loader] Map: explicit area catalog`를 기록했다. 구EXE에는 새 parser 문자열이 없고,
MapAssetCatalog.obj만 갱신된 상태라 최종 링크가 되지 않은 이전 실행 파일이었다.
최종EXE에는 새 Mirror/emissive parser 및 즉시 geometry Preview 문자열이 모두 존재함을 확인했다.

기존 catalog native probe의 catalog-only 경로로 현재 CMapAssetCatalog를 컴파일해 검사했다.
쿠크 runtime/source323개, Character Select runtime/source63개 모두 로드 PASS/exit0이다.
분리된 probe 폴더에 현재 runtime6파일을 byte동일 복사했고 원본12파일은 검사 전후 동일했다.
GPU/Device/GameInstance/Client/UI를 실행하지 않았다. 원문은
`out/KoukuJoker20260908/catalog-only-receipt.json`, `old-client-map-parser-evidence.json`이다.

최종 정본은 Composition149(PRODUCT11개/88 stages), World402, Camera71이다. 댄스타임 +90도와
clap14개, 룰렛 radius2.5m 저작 보행면, P13 retarget3/8/16 및 Collider9개 연결을 재확인했다.
WORLD 카드에는 보행면이 없으며 hop→NEXT Idle/flip→HOLD를 유지한다. 중앙 조커 접촉 시 flip cue와
Success 결과가 발생하고 기존 P14로 넘어간다. 원래 카드 TRS와 사용자 Collider geometry는 보존했다.
Composition/Gameplay publish, 쿠크 Map3231 placement·8파일과 CS Map803 placement·4파일 publish가
통과했다. changed JSON/XML parse 및 소유 범위 diff check를 확인했다. 기존 다른 변경의
ArenaCameraProfile.cpp trailing whitespace는 건드리지 않고 scoped check에서 분리했다.

Server focused 검사92개(Bundle38/Object overlap38/Support16), 기존 편집기 transport·atomic
Apply/Save/Reload·즉시 geometry 검사, static/instance/skinned shader3개 컴파일이 통과했다.
세부 로그는 `out/KoukuJoker20260908`에 있다. 대형 dirty checkout의 다른 작업과 binary Resources는
보존했으며 자동 stage/commit/push는 하지 않았다.

사용자 실행 단계는 Server + Client profile의 Ctrl+F5 → KoukuSaydon 입장 → F1 Composition에서
댄스타임/룰렛과 조커찾기 Bundle의 Server Play다. Collider Detail은 Stop 또는 Pause 상태에서
위치·회전·크기를 조절하면 현재 시각에 즉시 반영되고, Apply/Save로 값을 유지한다.
에이전트는 Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 포커판/중앙 링의 최종 모습,
두 맵 실제 재입장과 망치·카드 반응·장판·Collider drag 화면은 새 EXE로 사용자 확인이 남아 있다.
