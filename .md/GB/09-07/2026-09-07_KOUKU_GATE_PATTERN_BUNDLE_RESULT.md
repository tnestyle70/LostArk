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

기존 물리 자산 `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel`과 같은 폴더의 `WP_MN_RPCT_06.wmodel`을 읽었다. 새 binary/Resource를 생성하거나 이동하지 않았으며 Drive 추가 전달 대상은 없다. 실제 끝부분 Bone 선택과 offset의 시각 확인은 사용자가 한다. 구체적인 편집 순서는 팀 Animation Tool 인계 문서 17.7을 따른다.

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
