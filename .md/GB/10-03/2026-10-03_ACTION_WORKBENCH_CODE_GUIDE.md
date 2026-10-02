# Action Workbench · Sequencer · Composition Resources 코드 조사

[전체 코드 지도](2026-10-03_VISUAL_STUDIO_CODE_ATLAS.md) · [최종 VS 필터 트리](2026-10-03_VISUAL_STUDIO_FILTER_TREE.md)

2026-10-03, origin/main 렌더링 변경 통합 직후 실제 파일을 읽은 조사 메모다. Client 실행·UI 조작·화면 판정은 수행하지 않았다. 이 문서는 구현 계획이 아니라 코드 학습 지도이며 전체 소스를 재수록하지 않는다.

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

독립 Sequence workspace의 `Publish_AllPatterns`는 현재 명시적으로 local preview와 Save만 지원한다고 거부한다. 같은 클래스·lane을 쓴다는 사실만으로 Boss Product와 같은 서버 publish 기능이라고 설명하지 않는다. 독립 연출의 별도 진입·완료 처리 경로는 해당 소비자 단위로 추가 추적해야 한다.

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
