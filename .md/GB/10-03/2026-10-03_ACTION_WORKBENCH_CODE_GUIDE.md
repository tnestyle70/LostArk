# Action Workbench · Sequencer · Composition Resources 코드 조사

[전체 코드 지도](2026-10-03_VISUAL_STUDIO_CODE_ATLAS.md) · [최종 VS 필터 트리](2026-10-03_VISUAL_STUDIO_FILTER_TREE.md)

2026-10-03, origin/main 렌더링 변경 통합 직후 실제 파일을 읽은 조사 메모다. Client 실행·UI 조작·화면 판정은 수행하지 않았다. 이 문서는 구현 계획이 아니라 코드 학습 지도이며 전체 소스를 재수록하지 않는다.

G10은 이후 확보한 `C:/Users/tnest/Desktop/UnrealEngine`의 UE 5.8.3 소스와 대조한 보충이다. 비교 기준 commit은 `396c9f059903aed5fec78ecd3d437a40c6415368`이며 버전 값은 [Build.version](C:/Users/tnest/Desktop/UnrealEngine/Engine/Build/Build.version:2)에서 확인했다. 이 보충 작업에서는 양쪽 소스·publisher를 읽었으며 코드 변경, 빌드, 데이터 게시, Unreal Editor·Client 실행은 하지 않았다. 소스 확보와 실행 가능한 에디터 설치·기동은 별도 상태다.

## G01. Action Workbench의 창과 실제 owner

화면의 Action Workbench는 `CSequencerTool`이라는 공통 ImGui shell이다. 이름에 Sequencer가 있다고 이 클래스가 모든 연출을 재생하는 것은 아니다. `CMainApp`이 실제 세션들을 `unique_ptr`로 소유하고, shell에는 비소유 `ICompositionWorkbenchSession*`를 전달한다. shell은 창 위치·표시 여부·현재 대상·공통 복사/붙여넣기와 물리 애니메이션 브라우저를 관리한다. 저작 문서, dirty draft, 저장, 재생 정책은 세션에 남는다.

근거: [MainApp 생성/연결](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11599), [CSequencerTool 선언](C:/Users/tnest/Desktop/LostArk/Client/Public/SequencerTool.h:20), [공통 세션 인터페이스](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionWorkbenchSession.h:68).

| UI 대상 | 실제 세션 | 핵심 owner |
|---|---|---|
| Boss / Valtan | `CValtanActionWorkbench` | 분할 gameplay/presentation 정본을 join한 view, Balance Tool의 실제 편집 owner |
| Boss / Kouku 각 관문 | `CKoukuSaydonActionWorkbench(false)` | `KoukuSaydonComposition.json`과 `m_Draft` |
| Character | `CCharacterActionWorkbench` | skillbindings, animevents, HitShapes와 전용 `CEffectAuthoringSequencer` |
| Object / Kouku | `CWorldObjectTool` | World Sequence source와 연결 Composition |
| Sequence / Kouku | `CKoukuSaydonActionWorkbench(true)` | 별도 `KoukuSaydonSequenceComposition.json` |
| Object·Sequence / Valtan | 기존 `CValtanActionWorkbench` | 같은 발탄 owner의 해당 작업 공간 |
| World | shell이 소유하는 Class Selection session | Level callback으로 현재 제품 클래스 선택 연출을 제어 |

현재 enum은 `BOSS, CHARACTER, OBJECT, SEQUENCE, WORLD`다. Effect Tool V1/V2는 독립 창이며 이 enum에 없다. World라는 UI 대상과 `CWorldSequencePlayer`라는 객체 재생기는 다른 개념이다. [실제 dispatch](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1276)를 먼저 읽으면 이름 때문에 생기는 혼동을 줄일 수 있다.

공통 frame은 `Render → Selected_Session → Select_WorkbenchTarget → Begin_WorkbenchFrame → 각 pane Render → End_WorkbenchFrame → pending transfer/edit 적용` 순서다. `m_bInsideFrame` 동안 대상 변경은 즉시 실행하지 않고 다음 경계로 보낸다. 여러 pane이 같은 vector 원소를 보고 있을 때 앞 pane이 vector를 바꾸면 뒤 pane의 참조가 무효화될 수 있기 때문이다. Map Tool이 같은 세션을 이미 host하는 frame에는 두 번째 Begin/End를 열지 않는다. 대상 변경 시 이전 세션의 `On_WorkbenchDeactivated`가 preview를 정리하지만 각 문서 초안은 해당 owner에 남는다. [frame 구현](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1652).

이 구조의 핵심 디자인은 UI를 통합하면서 저장 권위까지 하나로 뭉치지 않는 것이다. 따라서 ‘한 창에서 보인다’와 ‘한 번의 Save가 모든 파일을 원자적으로 저장한다’는 별개의 보장이다.

## G02. 쿠크 Composition: 정의와 occurrence

`KOUKU_SAYDON_COMPOSITION_DOCUMENT`는 패턴, Logic 정의, Summon 정의, World 정의, Scene Profile 정의, Presentation Resource, Folder, Bundle, Pattern Flow를 가진 값 문서다. `iRevision`은 저장 세대이며, `iNext*Ordinal`은 stable ID 발급용 다음 번호다. `fixedTickHz=30`은 gameplay clock 계약이다. vector 순서는 편집·실행 순서가 될 수 있지만 ID 대신 저장해서는 안 된다. [문서 구조](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:683).

`PATTERN`은 `patternId/actorProfileId/gateId/targetBossPlacementId`와 여러 lane의 occurrence를 묶는다. Stage는 `stageId/actionId/stageKind/durationMs`와 Animation occurrence를 소유한다. Animation occurrence의 `runtimeClip`은 실제 모델 clip 이름, `sourceStartMs/sourceEndMs`는 원본 구간, `startOffsetMs`는 Stage 안 배치 시각, `playRate/playMs`는 재생 속도·길이다. `referenceRevision`은 원본 행동 참조의 freshness를 연결한다. 원본 clip 시계와 Stage/Pattern 시계를 같은 숫자로 취급하면 trim·효과 발생 시점이 어긋난다. [Animation 구조](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:17), [Pattern 구조](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:563).

Resource는 재사용 가능한 정의이고 occurrence는 시간축에 놓인 한 번의 사용이다. 예를 들어 `presentationResources`의 `resourceId`는 Effect asset, camera, sound 등의 실제 owner를 참조한다. `presentationOccurrences`는 그 resourceId와 start/duration, 위치·회전·크기, bone, anchor kind, 밝기·fade·source skip을 가진다. 박스 하나를 1.5배 확대한다고 공용 Effect leaf/group 자체를 1.5배 바꾸지 않는다. 독립 리소스와 사용 위치의 튜닝이 분리되어 있으므로 같은 원본을 다른 패턴에서 안전하게 재사용할 수 있다. [resource](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:435), [occurrence](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:459).

실제 데이터 예: [쿠크 첫 패턴](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:4065)은 `KAKULSAYDON_G1_PATTERN_1`, 이름 `세이튼_무력화 시작`, duration 15156ms다. Stage 43은 2267ms 동안 `rpct00_att_battle_6_04`를 사용한다. 첫 Logic occurrence는 2263ms부터 10684ms 동안 실행된다. Effect occurrence에는 같은 시각·길이와 `rotationDegrees=[0,90,0]`, `scale=[1.5,1.5,1.5]`, `loopEffectToDuration=true`가 있다. 이 값들은 해당 occurrence의 시간·공간 계약이지 전역 shader 보정이 아니다. 이 패턴의 `authoringStatus`는 조사 시점 `DRAFT`였으므로 데이터 존재만으로 서버 Product 승인 완료라고 설명하지 않는다.

조사 시 저장본 규모는 Composition revision 2498, pattern 121, Logic 210, Presentation Resource 1139였고, 독립 Sequence revision 183, pattern 10이었다. 이 숫자는 현재 파일 관찰값이며 엔진의 고정 제한이 아니다.

`Commit_Candidate`는 후보 값 전체를 받아 parent window를 동기화하고 유효성을 검사한 뒤 draft를 교체한다. 잘못 읽힌 Pattern의 `strLoadError/strPreservedJson`은 편집 상태로 남아 다른 정상 항목의 Save에서 잃지 않도록 한다. 잘못된 Pattern은 repair/reload하거나 삭제하기 전까지 직접 수정하지 못한다. [candidate commit](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:2389).

## G03. 저장·Publish·Reload는 서로 다른 단계

쿠크 `Reload_FromPath`는 local `stagedDocument/stagedReferences`에 읽고 parse/path/validate를 통과한 뒤 `m_LastGood`, baseline bytes, generation을 교체한다. 실패하면 `m_bFresh=false`지만 last-good은 유지한다. Workbench Reload는 dirty draft가 있으면 명시적 discard를 요구한다. [reload](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:5735), [dirty guard](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1685).

`Save_Atomic`은 다음 순서다. [구현](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:5773).

1. last-good baseline과 candidate revision 확인, candidate validation.
2. Composition writer lock 획득, 최신 디스크 bytes 읽기.
3. 디스크가 baseline과 다르면 baseline/candidate/current의 3-way merge 후 재검증. 외부 revision이 증가하지 않았거나 같은 필드 충돌이면 파일·draft 보존.
4. revision 증가, PID/thread/tick을 붙인 임시 파일 기록. fwrite·fflush·_commit·fclose 확인.
5. 임시 파일 재읽기, bytes 비교, parse/validate, 재직렬화 동일성 확인.
6. 교체 직전 디스크를 다시 읽어 CAS 확인.
7. `MoveFileExW(REPLACE_EXISTING | WRITE_THROUGH)`로 교체.
8. 최종 파일 reopen/parse/validate 후 last-good 갱신.

마지막 reopen만 실패하면 `COMMIT_SUCCEEDED_REOPEN_FAILED`라고 구분한다. 이미 성공한 디스크 commit을 실패라고 추측해 오래된 파일로 다시 쓰지 않는 것이 중요하다. Atomic replace는 단일 파일 교체이고, 여러 domain의 서버 snapshot까지 동시에 바뀌었다는 뜻은 아니다.

Workbench의 Save는 pending placement geometry를 candidate에 합쳐 검증하고 owner의 Save_Atomic을 호출한다. 이후 World animation 별도 저장 callback이 실패할 수도 있으므로 UI에 ‘모두 한 transaction’이라고 표현하면 안 된다. [Workbench Save](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1740).

쿠크 Publish는 clean/fresh 저장본을 전제하고 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon` 경로를 실행한다. projected gameplay와 presentation이 갈라진다. `project_encounter`는 서버가 읽을 행동·판정 정의를, `project_presentation`은 clip/effect/world/camera 등의 클라이언트 표현 정의를 만든다. `projected_outputs/publish_outputs`는 생성물 검증과 교체를 담당한다. `Publish-GameplayBalance.ps1`가 Encounter 등을 bootstrap으로 게시하며 서버는 authoring 창의 메모리를 직접 읽지 않는다. [Publish 시작](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1807), [domain owner](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildDomainOwner.ps1:148), [projector](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4929).

독립 Sequence workspace의 `Publish_AllPatterns`는 `m_bSequenceWorkspace`이면 local preview와 Save만 지원한다는 메시지로 거부한다. 이것은 **그 UI 작업 공간의 Publish 명령 제한**이다. `KoukuSaydonSequenceComposition.json` 자체가 전체 제품 게시나 실행에서 사용되지 않는다는 뜻은 아니다. [UI 제한](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1810).

전체 `koukusaydon.product` domain은 Sequence JSON을 입력으로 선언한다. `projected_outputs`는 Boss Composition에서 encounter와 presentation을 만든 뒤, Sequence 파일이 있으면 저장본을 읽어 World catalog를 검증하고 `project_raid_gates(action, sequence)` 결과를 encounter의 `raidGates`에 넣는다. 이 projection은 Sequence composition ID/revision, 관문 intro/clear pattern ID와 길이 등을 기록한다. 따라서 Sequence의 Save는 전체 publisher가 다음에 읽을 입력을 바꾸지만, 그 자체가 게시 완료나 Server의 새 revision 채택은 아니다. [domain 입력](C:/Users/tnest/Desktop/LostArk/Tools/Build/BuildDomains.json:45), [Sequence 저장본을 읽는 projector](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6212), [raid metadata 생성](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/raid_flow_projection.py:112).

실제 Client의 Server raid 연출 경로도 따로 존재한다. `CMainApp`은 준비 시 저장된 Action/Sequence revision 및 Sequence composition ID와 Server가 고정한 값을 대조한다. 일치하면 `m_pKoukuRaidSequenceDocument`에 값 snapshot을 만들고 `m_iKoukuRaidDocumentEpoch`를 기록한다. 재생 진입에서도 snapshot이 없거나 run epoch가 바뀌면 `Resolve_SequencePath()`를 다시 읽고 identity/revision을 검사한다. `state.strSequencePatternId`를 expand한 뒤 `Begin_BundlePreview(..., clockMs, ...)`로 재생 객체를 구성하고 `Sample_ServerSequence(clockMs)`를 호출한다. 여기서 clock은 UI cursor가 아니라 Server start tick과 현재 tick의 차이를 30Hz 기준으로 ms로 바꾼 값이다. 함수 이름에 Preview가 남아 있어도 이 호출자의 시간 권위는 Server다. [준비와 revision 대조](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1849), [run snapshot과 재생 소비자](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2039).

| 확인하는 단계 | 실제로 바뀌는 상태 | 그 성공만으로 보장되지 않는 것 |
|---|---|---|
| Append / Detail Apply | Workbench의 검증된 `m_Draft`, dirty 및 draft generation | 디스크 Save, Product 게시 |
| Save | authoring JSON, 저장 revision과 owner baseline | 실행 중 Server generation, 이미 재생 중인 snapshot |
| 전체 domain Publish | projector 출력과 domain별 게시 파일·receipt | 모든 살아 있는 Client/Server 메모리의 자동 교체 |
| Runtime 준비·Reload·승인 | 해당 소비자가 검증하고 채택한 snapshot, revision 또는 run epoch | 다른 owner의 미저장 draft나 모든 세션의 동시 갱신 |

이 표의 마지막 단계는 하나의 전역 Reload 버튼을 뜻하지 않는다. World Object Tool의 runtime reload, Composition owner의 Reload, Server raid의 revision pin과 준비 승인은 서로 다른 소비자 경계다. 사용자가 편집 중인 draft를 버리는 Reload를 설명 과정에서 자동 실행하지 않는다.

발탄은 저장 구조가 다르다. `CValtanActionWorkbench`는 `CValtanPatternTree`의 canonical view를 그리고 `CBalanceTool`의 gameplay/presentation draft owner를 호출한다. `Save_Reload`는 pinned authoring/canonical revision을 검사하고 Pattern·Sound·EffectV2·Shake·Object Sound의 baseline/candidate/세대를 수집해 owner save transaction으로 보낸다. 저장 job ID와 저장 후 Publish 상태가 따로 있다. `Tool.Composition.SaveReload` profiler scope도 이 경계에 놓였다. [발탄 Save](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:6713). 발탄 데이터 정본은 `Data/Valtan/Valtan.gameplay.json`, `.presentation.json`, `.combatobjects.json`, `.worldeventsets.json`과 개별 cue owner로 나뉜다.

## G04. Character Sequencer와 Product projection

`CCharacterActionWorkbench`는 `CPlayerSkillCatalog`에서 class/skill을 읽고 선택한 실제 CModel과 `CAnimationTargetService` generation을 대조한다. `m_Bindings`는 편집값, `m_Baseline`은 저장 당시 bytes, `m_CompositionBaselineBinding`과 `m_CompositionCueBaseline`은 Composition projection 이전 상태다. `m_Panel/m_Sequencer`는 shared ownership, `m_AnimationTool`은 비소유 연결, `m_BoneEditor/m_ModelEditor`는 unique ownership이다. [선언](C:/Users/tnest/Desktop/LostArk/Client/Public/CharacterActionWorkbench.h:24).

`Open_Composition`은 현재 animevents를 다시 읽고 parse한 뒤 `Stage_CharacterAction`으로 선택한 binding/cue/combat 행을 전용 sequencer에 stage한다. `action.<asset>.skill.<id>[.stage.<N>]`가 arrangement의 stable sequence ID가 된다. stage 단위 편집과 전체 action 편집을 같은 저장값으로 취급하지 않으며, Server stage 순서를 뒤집는 export는 거부한다. [진입](C:/Users/tnest/Desktop/LostArk/Client/Private/CharacterActionWorkbench.cpp:458).

`Save_Composition`은 다음을 수행한다. [구현](C:/Users/tnest/Desktop/LostArk/Client/Private/CharacterActionWorkbench.cpp:483).

1. Sequencer에서 animation binding과 cue 값을 export.
2. retained occurrence의 stage/hold 의미를 보존하며 새 binding 후보 생성.
3. Product binding validate 및 cue save 사전 검증.
4. skillbindings를 baseline 비교로 저장.
5. animevents cue를 저장. 실패하면 방금 기록한 binding을 baseline으로 삼아 조건부 rollback.
6. Product 저장 성공 후 `.effectsequence.json` arrangement 저장.

6번이 실패할 때는 Product animation/cue는 이미 저장되었고 arrangement만 미저장이라는 상태를 명시한다. 이는 다중 파일 crash-atomic transaction이 아니다. 기존 저장 경계의 실제 한계를 포트폴리오에서 숨기지 않고, 어느 단계의 실패까지 복구하는지 설명하는 편이 정확하다.

`Save_Combat`는 별도 HitShapes owner를 저장하고 gameplay publish·Server restart가 필요하다고 표시한다. preview collider 표시와 서버 DAMAGE/COUNTER/STAGGER 판정은 다른 소비자다. [저장 함수](C:/Users/tnest/Desktop/LostArk/Client/Private/CharacterActionWorkbench.cpp:1757).

실제 데이터 예: [Slayer skillbindings](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Slayer/Slayer.skillbindings.json:1)는 formatVersion 3이며 첫 binding `skillId=45050`에 `wbk_sk_updownslash_01`, `wbk_sk_updownslash_02`를 순서대로 연결한다. 배열 순서를 통해 clip chain을 만들지만 skill identity는 45050이다. 저장 형식이 legacy 문자열 clip인지 rich occurrence 객체인지는 binding별로 확인해야 한다.

`CEffectAuthoringSequencer`는 Animation/Effect/Collider/Sound/Camera/Screen Post row를 가진 preview backend다. `Sample`은 모델→root→camera→effect→sound 순서다. `Seek`는 duration으로 clamp하고 pause한 뒤 동일 sample 함수를 사용한다. `Update`는 ms clock을 증가시키되 Play 준비에 소요된 시간이 첫 delta에 포함되지 않도록 한 번 건너뛴다. `Stop`은 효과·sound·camera를 해제하고 모델 identity와 generation이 아직 같을 때만 이전 animation/track position/pause 값을 돌려놓는다. [Sample/Seek/Stop/Update](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectAuthoringSequencer.cpp:2203).

Arrangement의 실제 저장 위치는 `Data/Effects/Sequences/<sequenceId>.effectsequence.json`이다. `Save_Sequence`는 ID·anchor·source window·row 검증, baseline 비교, JSON parse 및 1MiB 제한 뒤 atomic file writer를 호출한다. 저장 직전 current bytes 검사 이후 writer 안까지 동일 baseline이 전달되는 형태는 아니므로 쿠크 Save_Atomic의 최종 CAS 보장과 동일하다고 설명하지 않는다. [sequence Save](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectAuthoringSequencer.cpp:2565).

실제 예: [워로드 17250 sequence](C:/Users/tnest/Desktop/LostArk/Data/Effects/Sequences/effect.warlord.skill.17250.clip1.full.restore.effectsequence.json:1)는 `MODEL_SEQUENCE/Warlord/skill.17250`, `MODEL_ROOT` anchor와 `V1_DOCUMENT` effect ID를 연결한다. Effect occurrence는 0ms부터 6678ms이며 camera keys는 같은 sequence 안에서 별도 lane 값으로 저장된다. 같은 effectId여도 occurrenceId는 그 한 번의 사용을 식별한다.

## G05. World Sequence: 객체 정의·템플릿·인스턴스·활성 재생

World Sequence는 `CWorldSequenceDocument`의 값 문서와 `CWorldSequencePlayer`의 살아 있는 재생 상태가 나뉜다. [문서 선언](C:/Users/tnest/Desktop/LostArk/Client/Public/WorldSequenceDocument.h:94), [player](C:/Users/tnest/Desktop/LostArk/Client/Public/WorldSequencePlayer.h:42).

| 자료구조 | 무엇을 소유하는가 |
|---|---|
| `WORLD_SEQUENCE_OBJECT_RESOURCE` | objectId, model/animation donor asset, material 입력, modelPreScale, 기본 motion |
| `WORLD_SEQUENCE_TEMPLATE` | sequenceId, duration, interpolation과 transform/animation/effect/material/collider/sound/subtitle tracks |
| `WORLD_SEQUENCE_INSTANCE` | instanceId, templateId, slot→대상 binding, 지연·배속, anchor, STOP/HOLD/LOOP/NEXT 정책 |
| `WORLD_SEQUENCE_BINDING` | MAP_PLACEMENT/DEPLOY_PLACEMENT/OBJECT_RESOURCE 중 어느 stable targetId가 slot을 채우는가 |
| player `ACTIVE_INSTANCE` | 현재 elapsed ms, baseline placement, clone 객체, effect/audio handle, 실제 sample 결과 |

Object resource는 생성 가능한 모델 정의이고 instance는 ‘어느 template을 어느 대상에 재생하는가’다. `ACTIVE_INSTANCE`는 디스크에 저장하지 않는 실행 수명이다. object model cache와 prepared clone pool은 반복 재생 비용을 줄이고 prototype 자체는 immutable하게 유지한다.

`Sample_Track`는 정렬된 키에서 `upper_bound`로 양옆 키를 찾는다. 시간 비율을 clamp하고 SMOOTH_STEP이면 `t²(3-2t)`를 적용한다. 위치/scale은 Lerp, 회전은 normalized quaternion Slerp, visibility는 left key의 계단값이다. 키 K개당 구간 검색은 O(log K)이며 보간은 O(1)이다. [구현](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:535).

`Sample_ObjectWorld`는 별도 physics engine 적분이 아니라 현재 age로 다시 계산하는 결정적 sample이다. seed와 emitter ordinal을 섞은 xorshift로 고정 spawn/spread를 얻고 `p = keyOffset + v*t + 0.5*a*t² + orbit(t)-orbit(0)`에 resource/key scale과 회전을 결합한다. authored emission의 yaw/offset, instance/occurrence placement, anchor basis를 정해진 순서로 곱한다. 따라서 Seek로 과거 시각을 지정해도 같은 seed·age이면 같은 위치를 계산할 수 있다. degree→radian, ms→seconds, modelPreScale과 occurrence scale을 다른 단계로 보는 것이 중요하다. [실제 함수](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp:1224).

`Update`는 active instance를 advance한 뒤 `Apply_Instance`의 PLAYING/FINISHED/FAILED에 따라 유지/종료한다. 실패 instance는 자신의 target을 돌려주며 다른 instance까지 전부 지우지 않는다. 정상 완료한 배치 pose는 held 상태로 남을 수 있고, explicit Stop의 restore 정책과 같지 않다. 미디어 tail도 visual lifetime과 별도로 정리한다. [Update](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:1120).

실제 예: [템플릿 paper_2](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json:7179)는 3067ms, SMOOTH_STEP이며 `animated.prop` slot에 `evt2_paperstage_open01` clip을 배속 1, loop true로 연결한다. [instance](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json:321321)는 `world.sequence.instance.1 → sequence.LV_LUT_MIDNIGHTC_ED.2`, slot을 DEPLOY_PLACEMENT `1`에 묶는다. 별도 [카드 object resource](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json:8)는 `Character/KoukuSaton/MN_RHOC_00/MN_RHOC_00.wmodel`, modelPreScale 약 .01, resource scale 2를 가진다.

Object Tool의 Save는 source 및 연결 Composition의 baseline을 확인한다. Publish는 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish`를 실행하고 완료를 Poll한다. 성공 후 Level의 `Reload_WorldObjectRuntime`을 호출하며, 연결 pattern 변경은 별도 publisher 상태로 이어진다. 제품 activation은 `Load_PreparedArea`로 Loader가 준비한 snapshot을 사용하고 준비 실패를 동기 I/O fallback으로 감추지 않는다. [Save](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldObjectTool.cpp:799), [Publish/Poll](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldObjectTool.cpp:1020), [prepared load](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:361).

## G06. Composition Resources: 탐색과 값 복사

`COMPOSITION_RESOURCE_TREE_NODE`는 segment/stablePath, leaf index vector, children, 재귀 leaf 수를 가진 cached tree다. 파일 시스템을 매 frame 탐색하는 tree가 아니다. 명시적으로 읽은 catalog snapshot을 분류한 후 검색 문자열이나 source generation이 바뀔 때 다시 만든다. leaf index는 그 snapshot을 가리키는 UI 인덱스이며 저장 stable ID가 아니다. `InsertResourceTree`는 segment별 자식을 찾거나 생성하고, `FinalizeResourceTree`는 자식을 이름순 정렬하면서 개수를 합산한다. [선언](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionResourceTree.h:19), [구현](C:/Users/tnest/Desktop/LostArk/Client/Private/CompositionResourceTree.cpp:62).

물리 Animation resource의 identity는 clip 이름만이 아니다. `strTargetAssetName`, `strModelAssetId`, `strSourceAssetId`, `strProfileId`, `strRuntimeClip`을 함께 보존한다. 같은 이름 clip이 다른 body/AnimSet에 존재할 수 있기 때문이다. `durationTicks/ticksPerSecond`는 ms로 반올림한 표시 길이와 별도로 원본 정밀도를 지킨다. `strDisplayName`은 label이며 lookup key를 대체하지 않는다. [resource 필드](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionAnimationResource.h:15).

Clipboard와 drag/drop은 `shared_ptr<const COMPOSITION_TRANSFER_SNAPSHOT>`을 사용한다. snapshot에는 authoring 값만 넣고 source row pointer, preview 객체, audio handle을 넣지 않는다. ImGui payload에는 source 객체 주소가 아니라 64bit token만 전달한다. Drop 시 token과 payload type을 확인한 뒤 destination session의 typed Insert/Paste로 들어간다. destination이 자신의 ordinal에서 새 ID를 발급하고 검증·commit한다. 따라서 원본 창을 닫거나 원본 vector를 재정렬해도 복사값이 dangling pointer가 되지 않는다. [transfer/clipboard](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionEditing.h:35), [drag/drop](C:/Users/tnest/Desktop/LostArk/Client/Private/CompositionResourceTree.cpp:14).

공통 resource tab은 Animation/Logic/Summon/World/Scene Profile/Effect/Collider/Sound/Camera/Light/Pattern이다. 탭의 이름이 공통이어도 각 session이 받아들이는 타입과 정책은 다르다. base interface는 모르는 타입을 명시적으로 거부하며, Effect transfer는 V1_EFFECT/LEAF/GROUP과 anchor/timing 의미를 보존해야 한다. 지원하지 않는 속성을 버려서 ‘성공’하는 것은 올바른 호환이 아니다.

## G07. Encounter graph와 실제 전투 clock

발탄 `CActionCompositionGraphModel`은 이미 승인된 `VALTAN_PATTERN_VIEW`를 읽어 node/edge/path/layout/hit-test geometry를 생성하는 순수 projection이다. node key는 pattern/stage/action ID 조합, edge는 outcome과 대상 action/pattern ID다. 정본에 없는 TIMEOUT은 규칙에 따라 derived edge로 만든다. canonical graph와 manual audition chain의 event-entered stage fallback이 다르다. [헤더](C:/Users/tnest/Desktop/LostArk/Client/Public/ActionCompositionGraphModel.h:151), [derived timeout](C:/Users/tnest/Desktop/LostArk/Client/Private/ActionCompositionGraphModel.cpp:440).

Projection은 ID 중복·dangling target·cycle·duration overflow를 검사하고 topology를 구한 뒤 default/선택 outcome path를 계산한다. longest path는 reverse topological order의 동적 계획법으로 계산한다. counter retry는 한 표시 pass에서 끊고 반복임을 표시한다. 사용자가 UI에서 선택한 outcome override는 preview 선택값이며 Server 정본 branch를 바꾸지 않는다. 실패하면 이전 snapshot을 유지한다. [maximum path](C:/Users/tnest/Desktop/LostArk/Client/Private/ActionCompositionGraphModel.cpp:578).

Client의 Product presentation은 서버가 복제한 pattern sequence/start tick을 identity와 clock으로 삼는다. `CKoukuSaydonPresentationPlayer`는 `Try_ResolveActionAgeSeconds(serverTick, patternStartTick, 30.f, ...)`로 action age를 계산한다. 이것은 UI preview의 자유 Seek clock과 구분해야 한다. HP·판정·phase를 타임라인 창이 결정하는 것이 아니라 서버 simulation이 확정하고, 클라이언트가 그 상태의 표현을 재생한다. [Product clock](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3843).

## G08. 이름이 비슷하지만 완료 범위가 다른 facade

`Data/Compositions/Bosses/*.bosscomposition.json`과 `Data/Compositions/Sequences/*.sequencer.json`은 일반화된 facade 문서다. `CBossCompositionDocument`, `CArenaSequencerDocument`, `CCompositionDocumentCatalog`가 strict load/join을 제공한다. `CArenaSequencerDocument` track payload는 variant로 WORLD_SEQUENCE/CAMERA_SHOT/ACTOR_PATTERN 중 하나를 가진다. [정의](C:/Users/tnest/Desktop/LostArk/Client/Public/BossCompositionDocument.h:52).

실제 `Valtan.bosscomposition.json`과 `KoukuSaydonArena.sequencer.json`은 status SHADOW다. 후자는 21010ms에 world instance `world.sequence.instance.circusfinale`와 shot `1Stage.finale`를 참조한다. 현재 Client source 검색에서 이 reader들의 실행 호출자는 reader 구현 밖에서 발견하지 못했다. 그러므로 이 schema 자체를 Action Workbench 제품 재생 경로 또는 완성된 generic cinematic runtime으로 설명하면 안 된다. [실제 facade 데이터](C:/Users/tnest/Desktop/LostArk/Data/Compositions/Sequences/KoukuSaydonArena.sequencer.json:1).

## G09. 포트폴리오에서 설명할 현재 강점과 확인된 부족한 점

강점은 공통 창 아래 typed owner를 유지하는 구조, definition/occurrence 분리, stable ID, JSON validation, last-good 보존, 저장 freshness, preview와 Product clock 분리, 실제 모델의 clip/native clock 재사용, seed 기반 결정적 object sample, graph projection의 실패 보존이다. ‘상용 엔진과 같다’보다 입력 한 건이 어떤 API·데이터·저장·실행 경계를 통과하는지 사례로 보여주는 편이 설득력이 있다.

확인된 제한은 다음과 같다.

1. 모든 Sequencer가 하나의 generic backend가 아니다. 발탄, 쿠크, Character/Effect, World에는 각자의 값 타입과 writer가 있고 shell interface 수준에서 통합되어 있다.
2. 도구 이름과 facade 문서 이름이 실행 owner와 일대일 대응하지 않는다. SHADOW manifest와 실제 Composition을 분리해 가르쳐야 한다.
3. 저장 원자성 수준이 domain마다 다르다. 쿠크의 단일 파일 3-way merge/CAS, 발탄 다중 owner save job, Character의 순차 Product/arrangement 저장, World의 linked save를 같은 보장으로 묶을 수 없다.
4. UI가 값을 저장했다고 실행 중 Server generation까지 교체된 것은 아니다. Save/Publish/runtime reload/Server restart·승인/화면 확인의 증거를 나눠야 한다.
5. `KoukuSaydonActionWorkbench.cpp`, `ValtanActionWorkbench.cpp`는 각각 만 줄을 크게 넘는 domain UI이며 확장과 탐색 비용이 크다. 공통 window interface만으로 모든 편집 알고리즘의 중복이 제거된 것은 아니다.
6. source tree와 ordinal ID는 기능별 규약을 가진다. 두 브랜치가 같은 next ordinal에서 항목을 추가하면 의미 충돌이 날 수 있어 JSON stable ID 기준 merge 검증이 필요하다.
7. 일반 Undo/Redo, process crash recovery, 임의 사용자 track/plugin authoring은 이번 조사에서 연결된 소비자를 입증하지 않았다. 없다라고 단정하기보다 후속 검증 항목으로 두어야 한다.
8. 코드·CPU 검증이 실제 GPU 표시, 제품 화면 완성도, Unreal과의 동등성 증거를 대신하지 않는다.

기존 `.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`에는 과거 시점의 CURRENT/PARTIAL 표가 남아 있다. 학습 순서는 현재 H/CPP와 Data → 현재 소비자 → 대응 RESULT → 역사적 PLAN으로 잡고, 과거 문서의 완료 상태를 최신 코드에 그대로 덮어씌우지 않는다.

## G10. UE 5.8.3 실제 H/CPP와 나란히 읽는 열 가지 경계

아래 비교는 같은 목적을 다루는 코드의 역할을 연결한다. 같은 이름이나 비슷한 화면이 같은 데이터 구조·기능 범위·성능을 뜻하지 않는다. LostArk의 `CSequencerTool`을 UE의 `FSequencer` 하나에 대응시키거나, 우리 Publish를 UE Cook 전체와 같다고 설명하지 않는다. 현재 한 기능의 입력 → 저장 자료 → 실제 평가 소비자 순서로 두 소스를 함께 연다.

### G10-01. SSequencer / FSequencer와 CSequencerTool의 화면·제어 분리

UE의 [SSequencer.h](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/SSequencer.h:139)는 `SSequencer : SCompoundWidget`을 선언한다. [SSequencer::Construct](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/SSequencer.cpp:195)는 전달받은 `FSequencer`와 함께 화면을 구성하고, `ChildSlot` 아래에 `SVerticalBox` 등의 Slate 위젯 트리를 둔다. `SNew`로 조립된 위젯과 저장된 참조가 화면 상태를 유지하는 구조다. `SSequencer`의 트리 생성 코드는 [ChildSlot](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/SSequencer.cpp:404)부터 읽는다.

편집 제어기는 [FSequencerModule::CreateSequencer](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/SequencerModule.cpp:293)가 만든다. 이 함수의 책임은 `FSequencer`와 `FSequencerObjectChangeListener`를 생성하고, 초기화 전 delegate를 알린 뒤 `InitSequencer`에 editor delegate와 초기화 인자를 전달하는 것이다. 성공하면 `TSharedRef<ISequencer>`를 반환한다. [FSequencer::InitSequencer](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/Sequencer.cpp:360)는 읽기 전용 상태, playback context, spawn register 등의 편집 환경을 채택한다.

LostArk에서는 [CSequencerTool::Render_Pane](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1590)가 공통 pane을 그리고 `session.Render_WorkbenchPane(pane)`로 실제 owner에게 위임한다. 쿠크는 [Render_WorkbenchPane](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:16828)에서 Resources/Timeline/Detail 등을 분기한다. UE의 지속 위젯 트리와 우리 ImGui 호출 방식은 다르지만, 화면 shell과 편집·재생 상태를 가진 owner를 구분해서 읽어야 한다는 점은 연결된다.

### G10-02. ISequencerModule의 Track Editor 등록과 작업 세션 인터페이스

UE [ISequencerModule::RegisterTrackEditor](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Public/ISequencerModule.h:250)는 `FOnCreateTrackEditor`와 animated property type 목록을 받는다. 반환값 `FDelegateHandle`은 등록 해제에 쓰는 식별자이며 저장 에셋 ID가 아니다. 실제 [등록 구현](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/SequencerModule.cpp:309)은 생성 delegate를 보관하고, 앞 절의 `CreateSequencer`가 이 목록을 편집기에 전달한다. 따라서 새 track 편집 지원은 기존 화면의 enum 분기에만 의존하지 않는다.

[FSequencerEditorViewModel](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Public/MVVM/ViewModels/SequencerEditorViewModel.h:30)은 selection 조회와 root/outliner/track-area/selection 생성 함수를 나눈다. 이 ViewModel은 화면 선택과 표시 구조의 owner이며 MovieScene 에셋 자체와 동일한 객체가 아니다.

LostArk [ICompositionWorkbenchSession](C:/Users/tnest/Desktop/LostArk/Client/Public/CompositionWorkbenchSession.h:68)은 Boss/Character/Object/Sequence의 서로 다른 owner를 같은 shell에 연결한다. 반면 쿠크 [Render_ResourceTree](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:5892)는 현재 도메인별 함수를 명시적으로 호출한다. 우리 세션 인터페이스는 작업 공간을 교체하는 계약이고, UE track editor registry는 track/property 편집 기능을 등록하는 계약이다. 임의의 새 track을 등록하면 모든 저장·평가가 자동 연결된다는 보장은 현재 우리 구조에서 입증하지 않았다.

### G10-03. IAssetRegistry / FAssetData와 Composition Resources inventory

UE [IAssetRegistry::GetAssets](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AssetRegistry/Public/AssetRegistry/IAssetRegistry.h:363)는 `FARFilter` 또는 compiled filter를 받아 `TArray<FAssetData>`를 채운다. 같은 헤더의 `GetAssetsByPath`, `GetAssetsByClass`는 경로·클래스별 조회 경계다. [FAssetData](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Public/AssetRegistry/AssetData.h:163)는 `PackageName`, `PackagePath`, `AssetName`, `AssetClassPath`를 보유한다. 에셋 목록의 메타데이터와 로드된 runtime 객체를 동일하게 취급하지 않는 것이 핵심이다. 이 인터페이스만으로 UE Content Browser의 모든 동작을 조사했다고 보지는 않는다.

LostArk [Refresh_KoukuPresentationResources](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1478)는 `CEffectV2Catalog::Read_Inventory`의 GROUP/LEAF와 `CEffectCatalog`의 direct-authored V1 항목을 공통 presentation resource 목록에 모은다. Workbench는 [Set_PresentationResources](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:2267)에서 받은 inventory를 보유한다. 선택 ID는 [sourceId](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:12662)의 `kind:resourceKind:assetId[:elementId]`이며 `m_strSelectedPresentationSourceId`가 현재 선택을 가리킨다.

Resources의 Effect 탭 선택은 runtime 이펙트 생성이 아니다. inventory를 version/owner/search로 분류해 보여주고, Append 또는 Play 명령이 뒤따라야 한다. UE Asset Registry와 비교할 수 있는 부분은 메타데이터 검색 경계이며, 우리 목록은 게임의 여러 catalog를 모은 도메인별 view라는 범위를 유지한다.

### G10-04. ULevelSequence → UMovieScene → Track → Section → Channel과 값 문서

UE [ULevelSequence](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/LevelSequence/Public/LevelSequence.h:32)는 `TObjectPtr<UMovieScene> MovieScene`을 갖는다. [UMovieScene](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieScene.h:1299)은 `ObjectBindings`와 scene 수준 `Tracks`를 보관한다. [UMovieSceneTrack::GetAllSections](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieSceneTrack.h:539)는 파생 track이 보유한 section 배열을 노출하고, [UMovieSceneSection::GetChannelProxy](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieSceneSection.h:674)는 section 내부 channel 접근을 제공한다. Section의 [GetRange](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieSceneSection.h:273)는 frame number 범위를 반환한다.

LostArk의 [KOUKU_SAYDON_COMPOSITION_DOCUMENT](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:683)는 C++ 값 구조와 vector로 이루어진 JSON 대응 문서다. `PresentationResources`는 재사용 정의이고, 각 `PATTERN::PresentationOccurrences`는 배치한 사용이다. resource의 `strAssetId`가 실제 Effect 등의 owner를 가리키고 occurrence의 `strResourceId`가 이 resource를 join한다. 모델·이펙트 handle 같은 살아 있는 객체는 이 저작 문서의 저장 계약이 아니다.

따라서 우리의 resource/occurrence 분리를 UE section에 대략 비유할 수는 있지만, occurrence가 UE channel proxy나 UObject 직렬화까지 갖춘 것은 아니다. 두 구조에서 값의 재사용, 시간 범위, 대상 연결을 각각 어디에 두는지 비교한다.

### G10-05. MovieScene binding/section 추가와 Append의 stable ID

UE [UMovieScene::AddTrack](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieScene.h:517)는 track class와 `ObjectGuid`를 받는다. [FMovieSceneBinding](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieSceneBinding.h:164)은 대상 binding의 `FGuid ObjectGuid`와 `Tracks`를 보유한다. property track의 [FindOrAddSection](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieSceneTracks/Private/Tracks/MovieScenePropertyTrack.cpp:339)은 주어진 frame의 section을 찾거나 `CreateNewSection`으로 만들고, 새 section의 `RF_Transactional` 설정을 확인한 뒤 track에 추가한다.

LostArk [Append_PresentationSource](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:11490)는 `m_Draft` 복사본에 선택 source를 stage하고 occurrence를 추가한다. [Stage_PresentationSource](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:11458)는 kind/asset/resourceKind/element가 같은 기존 resource를 재사용하거나 새 resource ID를 발급한다. [Append_PresentationCandidate](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:11503)는 pattern ID와 다음 ordinal로 occurrence ID를 만들고, 시작은 cursor를 패턴 끝 이전으로 clamp하며 길이는 기본 길이와 남은 패턴 길이의 최솟값으로 정한다. 검증 후 `Commit_Candidate`가 draft를 교체하며 이 단계는 Save가 아니다.

UE binding GUID는 대상 객체 연결이고, 우리 resource ID는 저작 정의 연결이며 occurrence ID는 사용 한 건의 identity다. pointer나 vector index를 저장 ID로 쓰지 않는 목적은 공유하지만, 무엇을 식별하는지까지 같지는 않다.

### G10-06. FFrameRate / FMovieSceneFloatChannel과 ms 기반 transform sample

UE [UMovieScene의 TickResolution/DisplayRate](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/MovieScene.h:1327)는 내부 시간 해상도와 화면 표시 프레임률을 분리한다. [FMovieSceneFloatChannel](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/Channels/MovieSceneFloatChannel.h:332)의 `Times`는 `FFrameNumber` 배열, `Values`는 `FMovieSceneFloatValue` 배열이다. 시간과 값을 병렬로 저장하고 [Evaluate](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/Channels/MovieSceneFloatChannel.cpp:248)는 `FFrameTime`을 공통 curve channel 구현에 넘긴다. 같은 CPP의 `AddLinearKey`와 `AddCubicKey`는 선형·곡선 키 추가 경계를 드러낸다.

LostArk [CWorldSequencePlayer::Sample_Track](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:535)은 ms 시각의 양옆 transform key를 `upper_bound`로 찾고 위치·scale은 Lerp, 회전은 normalized quaternion Slerp로 계산한다. SMOOTH_STEP은 보간 계수에 적용한다. 쿠크 gameplay의 fixed 30Hz와 저작 occurrence의 ms, 원본 animation tick은 서로 다른 시간 표현이다.

현재 World Sequence의 transform 보간을 UE의 일반 float channel/tangent 편집 전체와 동등하게 설명하지 않는다. 비교할 핵심은 시간 단위를 어디서 변환하는지, 키 사이 값을 누가 계산하는지, Seek가 동일 sample 경로를 쓰는지다.

### G10-07. ULevelSequencePlayer와 Workbench Preview 요청 소비자

UE [ULevelSequencePlayer::CreateLevelSequencePlayer](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/LevelSequence/Private/LevelSequencePlayer.cpp:51)의 책임은 sequence와 World를 검사하고 `ALevelSequenceActor`를 생성해 sequence/playback settings를 연결한 뒤 초기화된 player를 반환하는 것이다. [ULevelSequencePlayer::Initialize](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/LevelSequence/Private/LevelSequencePlayer.cpp:91)는 World/Level/camera settings를 설정하고 공통 `UMovieSceneSequencePlayer::Initialize`로 이어진다. [PlayInternal](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/MovieSceneSequencePlayer.cpp:310)은 별도 재생 단계다. 에셋 선택, player 생성, Play를 구분한다.

우리 쿠크 Effect 목록의 [Play Effect 버튼](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:12928)은 `Queue_PresentationPreview`에 요청을 넣는다. [CMainApp의 소비](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:3285)가 문서 복사와 임시 `preview.kouku.resource` pattern/occurrence를 구성하고 [Begin_Preview](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:3400)를 호출한다. [PresentationPlayer::Begin_Preview](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:4086)는 유효한 입력을 확인한 뒤 원본 모델 또는 Boss context이면 bundle 경로로, 일반 단일 pattern이면 내부 preview 문서·clock·playing 상태로 채택한다.

Resources의 Play에는 Append가 필수는 아니다. 반대로 이미 배치한 Effect occurrence의 Preview는 [Queue_PresentationPreview의 occurrence 분기](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:12131)에서 원래 패턴의 해당 시작 시간으로 재생을 요청하여 Boss/WORLD의 시간 관계를 유지한다. UI 명령과 실제 객체 생성의 분리라는 목적은 UE와 비교할 수 있지만, 우리 player의 Boss 및 게임 데이터 정책은 도메인 전용이다.

### G10-08. CompiledDataManager / EntitySystemRunner와 직접 occurrence 평가

UE [UMovieSceneCompiledDataManager](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/Compilation/MovieSceneCompiledDataManager.h:108)의 compiled data는 hierarchy, entity component field, track template field를 나누어 가진다. [Compile](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/Compilation/MovieSceneCompiledDataManager.cpp:906)은 entry의 `IsDirty`를 먼저 확인한다. 여기서 Compile은 C++ 컴파일이 아니라 MovieScene의 평가용 데이터 준비 경계다.

편집기 [FSequencer::EvaluateInternal](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/Sequencer.cpp:3856)은 evaluation range와 playback state로 context를 만들고 `RootTemplateInstance.EvaluateSynchronousBlocking`을 호출한다. [EvaluateSynchronousBlocking](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/Evaluation/MovieSceneEvaluationTemplateInstance.cpp:214)은 `Runner->QueueUpdate` 후 `Flush`한다. runtime [UMovieSceneSequencePlayer::UpdateMovieSceneInstance](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/MovieSceneSequencePlayer.cpp:1445)도 runner에 update를 넣고 동기 조건에서 flush한다. [FMovieSceneEntitySystemRunner](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Public/EntitySystem/MovieSceneEntitySystemRunner.h:73)는 QueueUpdate/Flush와 instantiation/evaluation/post-evaluation 단계를 구분한다.

우리 [PresentationPlayer::Sample](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:2743)은 `[startMs, startMs+durationMs)` 안의 occurrence를 평가하고 `(clockMs-startMs)/1000`으로 age를 만든다. V1은 [Spawn_LevelPlacement 호출](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3179), V2는 [Play_Group/Play_Leaf 경로](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3221)로 연결한다. 일반 단일 preview는 MainApp이 WORLD를 먼저 sample한 뒤 [Sample_Preview](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:3697)를 호출하며, bundle은 player가 자신의 member를 평가한다.

이것이 가장 큰 구조 차이다. UE에는 compiled hierarchy와 entity-system runner를 통한 공통 평가 체계가 있고, 조사한 우리 경로는 typed 문서의 occurrence를 도메인 player가 직접 평가한다. 알고리즘·객체 수명·확장 단위의 차이를 설명할 수 있지만 성능 우열은 동일 workload의 측정 없이 주장하지 않는다.

### G10-09. FScopedTransaction / SaveCurrentMovieScene / UPackage::Save와 Save_Atomic

UE의 편집 Undo transaction 예시는 [DeleteSection의 FScopedTransaction](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/Sequencer.cpp:2050)에서 볼 수 있다. 이는 편집 동작의 transaction이며 디스크 저장 성공과 같은 사건이 아니다. [SaveCurrentMovieScene](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/Sequencer/Private/Sequencer.cpp:6681)은 하위 MovieScene의 dirty package를 모으고 `FEditorFileUtils::PromptForCheckoutAndSave`를 호출한 뒤 재평가한다.

package 저장의 기본 API는 [UPackage::SavePackage](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/SavePackage2.cpp:75)이며 `UPackage::Save` 결과가 Success인지 반환한다. 실제 [UPackage::Save](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/SavePackage2.cpp:4212)는 입력 package/asset/filename/args로 `FSaveContext`를 만들고 `BeginSave → InnerSave → EndSave`를 수행한다. [InnerSaveInternal](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/SavePackage2.cpp:3908)은 package를 harvest하고 저장 realm을 처리한다. 이 조사로 UE의 모든 저장 실패가 다중 파일 crash-atomic하다고 보증하지 않는다.

우리 [Save_Atomic](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:5773)은 JSON 값 문서의 last-good/baseline/candidate/current를 다루며 3-way merge, 최종 CAS, 단일 파일 교체와 reopen 검증을 수행한다. [Commit_Candidate](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:2389)는 이보다 앞선 메모리 draft 채택이다. 두 함수의 실패 보존을 설명할 수 있지만, 그것을 UE식 일반 Undo/Redo transaction 구현 증거로 바꾸지는 않는다. 저장 자료와 되돌리기 범위를 나누어 비교한다.

### G10-10. CookSavePackage와 도메인 Publish의 생성물·소비자 경계

UE [UCookOnTheFlyServer::SaveCookedPackage](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/UnrealEd/Private/Cooker/CookSavePackage.cpp:70)는 package를 준비하고 target platform을 순회한다. 각 플랫폼에서 `FArchiveCookData`와 `FSavePackageArgs`를 구성하고 package writer의 인자 갱신을 거친 뒤 `GEditor->Save`로 저장한다. 플랫폼별 결과를 정리하는 `FinishPlatform`과 package 완료 처리가 뒤따른다. [플랫폼 입력과 저장 호출](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/UnrealEd/Private/Cooker/CookSavePackage.cpp:96)을 보면 일반 editor Save와 Cook의 차이를 확인할 수 있다. Cook 저장이 게임 실행이나 live world의 자동 교체까지 수행하는 것은 이 함수의 계약이 아니다.

LostArk의 [Invoke-BuildDomainOwner.ps1](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildDomainOwner.ps1:63)은 `KoukuSaydon` owner에서 `koukusaydon.product`, `map.kakulsaydon`, `world.gameplay`, `gameplay.balance`를 실행한다. [source revision 검사](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildDomainOwner.ps1:32)와 domain 시작/완료 재확인, 실패 rollback은 게시 중 정본 변경과 부분 갱신을 다루는 장치다. 이 owner 목록에는 모든 제품 domain이 들어 있지 않으므로 이를 전체 엔진 패키징이라고 부르지 않는다.

우리 Publish는 저작 입력을 게임별 실행 자료로 검증·projection·배포하는 목적에서 Cook와 비교할 수 있다. UE Cook은 target platform별 package 생성이라는 추가 범위를 갖는다. G03에서 확인했듯 Sequence 창의 Publish 제한과 전체 publisher의 Sequence 입력은 함께 존재한다. Save, 게시 파일, 실행 중 snapshot, Server 승인, 사용자 화면 확인은 각각 별도로 증거를 남긴다.

## G11. 쿠크 망치·anchor·collider·Logic 영상 사례의 준비 범위

쿠크 망치를 이용한 영상은 다음 코드·데이터·화면을 같은 한 건으로 연결하는 후속 사례다. 이번 조사에서는 특정 망치 occurrence의 anchor, collider, Logic 판정과 영상 결과를 끝까지 대조하지 않았다. 따라서 아래 표는 촬영·검증에 필요한 준비 항목이며 구현 완료·동작 성공 기록이 아니다.

| 준비할 장면 | 함께 열어 확인할 근거 | 영상 설명에 쓸 수 있는 시점 |
|---|---|---|
| Resources에서 실제 망치 항목 선택 | 실제 object/resource ID, donor model, template/instance ID와 선택된 catalog 행 | 표시명과 실제 데이터 identity가 일치할 때 |
| Timeline Append와 anchor 변경 | occurrence stable ID, start/duration, anchorKind·anchorPosition·owner 또는 대상 binding, 적용된 Transform 계산 | 원본 모델 basis와 occurrence/anchor 변환 순서를 해당 소비자까지 추적했을 때 |
| Collider 표시와 Logic 배치 | visual collider resource와 gameplay Logic 정의·occurrence·projector의 연결 ID | 화면 collider가 어떤 서버 판정 자료에 연결되는지 확인했을 때 |
| Preview / Save / Publish 비교 | draft, 저장 JSON revision, projector 출력, 게시 revision 및 실제 로드한 consumer | 서로 다른 상태를 같은 완료 문구로 합치지 않고 보여줄 수 있을 때 |
| Server 재생과 결과 | 해당 패턴의 승인·start tick, 판정 결과·snapshot, 사용자가 확인한 화면 | local preview와 Server 권위 결과를 같은 사례로 구분해서 검증했을 때 |

현재 코드·데이터만으로 곧바로 읽어볼 수 있는 작은 비교 사례는 G02의 `세이튼_무력화 시작`이다. [무력화 원본 방패 resource](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3982)는 `V1_EFFECT`의 `effect.kouku.gate1.stagger.shield.full.restore`를 참조하고, [해당 authored effect](C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.kouku.gate1.stagger.shield.full.restore.effect.json:4)에서 실제 asset ID와 payload를 이어 읽을 수 있다. 이 링크 추적은 저장 데이터의 연결 증거이며 GPU 표시·망치 판정·영상 촬영의 성공 증거는 아니다.

G10의 완료 범위는 로컬 UE/LostArk H·CPP·publisher의 심볼과 호출 흐름 대조다. 이 문서 보충에서 C++/HLSL 수정, 컴파일, publisher 실행, 에디터·게임 UI 실행, 화면 캡처는 수행하지 않았다.
