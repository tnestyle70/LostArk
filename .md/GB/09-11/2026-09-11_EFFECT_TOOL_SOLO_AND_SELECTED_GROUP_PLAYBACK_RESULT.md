# Effect Tool Solo 즉시 재생과 선택 그룹 반복 재생 결과

작성일: 2026-09-11. 작업 브랜치는 `codex/kouku-gate1-sequence-playback`이다.

## G00. 구현 상태

복원 문서의 Solo와 Timeline Solo가 성공하면 Sequencer를 paused=false로 시작한다. 기존
원본 시작 시각에서 바로 재생하며 준비 작업에 소비된 첫 프레임 delta를 계속 건너뛴다.

Current Effect의 Play All 오른쪽에 Play Group을 추가했다. Ctrl/Shift 클릭으로 marked한
Element ID 목록을 복사하여 단일 transient occurrence로 즉시 반복 재생한다. 일반 클릭으로
A를 고른 뒤 Shift 클릭으로 B를 고르면 A/B가 모두 marked된다. 이후 Ctrl/Shift 클릭은 기존처럼
개별 행을 토글하고 Detail 선택과 draft를 보존한다. 재생 후 marks를 바꿔도 현재 재생 집합은
바뀌지 않는다. Play Group을 다시 누르면 새 선택으로 교체한다.

선택 preview는 기존 `CEffectObject` factory, document projection, Stage_Row, Sample_Row,
Play, Refresh_Effects, Stop을 공유한다. 별도 renderer나 저장 schema는 추가하지 않았다.
선택된 Element와 필요한 source provider를 원본 순서로 보존하고 model cue는 숨긴 anchor로
유지한다. source delay, lifetime, material, resource, attachment와 현재 draft는 그대로 소비한다.

반복 구간은 선택된 drawable Element의 최소 시작부터 최대 종료까지다. 종료 계산은 제품
`CEffectPlayback::Calculate_ElementEndSeconds`를 사용한다. 선택하지 않은 긴 Element,
provider의 긴 lifetime, 전체 model sequence 길이는 선택 반복 구간을 늘리지 않는다.
Refresh가 draft를 다시 투영할 때 시작/종료도 갱신한다. 반복과 Restart는 해당 시작으로 돌아가며
이전 반복의 source anchor history를 재사용하지 않는다.

loop는 임시 row가 소유한다. Stop이나 다른 preview가 row를 해제하면 기존 saved sequence의
loop 설정이 다시 적용된다. 선택 preview는 Append/Save 대상이 아니다. Load 성공, New,
Discard에서 선택 preview를 정리하고 다른 문서의 marks를 이월하지 않는다. 실패한 선택과
Load는 기존 문서와 미리보기를 보존한다.

## G01. 변경 파일과 호출

| 파일 | 변경 책임 |
|---|---|
| `Client/Public/Effect_Tool.h` | 선택 목록 재생, 문서 투영, 재생 구간과 V1 factory 출력 선언 |
| `Client/Private/Effect_Tool.cpp` | Play Group 버튼·marks UX, Solo 공통 진입, 선택 구간 계산, 문서 교체 정리 |
| `Client/Private/Effect_Tool_Workspace.cpp` | draft 적용 후 선택 문서와 시작/종료를 factory에서 반환 |
| `Client/Public/EffectAuthoringSequencer.h` | transient ID 목록·시작 시각·loop 상태 |
| `Client/Private/EffectAuthoringSequencer.cpp` | 즉시 재생 commit, 선택 구간 반복, Refresh와 history 초기화 |
| `Client/Private/EffectAuthoringSequencer_Timeline.cpp` | 임시 loop 제어와 선택 시작 시각 Restart, 목록 label |
| `Client/Private/EffectAuthoringSequencer_Tracks.cpp` | 선택 preview의 Append 금지 유지 |
| `Client/Private/EffectAuthoringSequencer_Resources.cpp` | 선택 목록과 임시 preview 설명 |

`Try_PlayMarkedElementGroup -> Try_PreviewElementsTimeline -> Preview_Elements -> Stage_Row ->
Create_AuthoringOccurrence -> Build_ElementsPreviewDocument/Resolve_ElementsPreviewWindow`가
성공한 뒤 원본 document clock으로 Sample_Row를 실행하고 새 transient를 commit한다.

## G02. 자동 검증

| 실행한 검증 | 결과 |
|---|---|
| 변경 6 CPP Debug 최소 compile | exit 0, 오류 0. OBJ/PDB는 `out/EffectToolSelectedGroup20260911` 전용 |
| 선택 투영·구간 임시 숫자 검사 | exit 0. 원본 순서, 숨긴 cue anchor, 선택 구간, missing/hidden 거부, source provider 보존, 원본 문서 보존 확인 |
| 도화가 31930 실제 provider 의존성 | 선택 dependent와 앞선 simulation-only provider만 유지하고 실제 Playback validator 통과 |
| 쿠크 내려치기 2306/2307 선택 구간 | 930~1830ms. 원본 문서 serialize 전후 일치 |
| 합성 긴 문서의 선택 구간 | 선택 a/b는 2000~6000ms. 선택하지 않은 290000ms 종료 요소가 구간을 늘리지 않음 |
| 인코딩·줄바꿈 | 수정 8개 H/CPP의 UTF-8 BOM 없음과 기존 LF/CRLF 유지 |
| project/filter | 기존 파일 등록 유지, 신규 C++ 파일 없음 |
| JSON/XML | 저장 계약과 해당 입력 파일 변경 없음 |
| `git diff --check` | 성공. 다른 변경의 기존 줄바꿈 변환 경고만 출력 |

컴파일 명령은 `out/EffectToolSelectedGroup20260911/compile.cmd`, 로그는 `compile.log`다.
숫자 검사는 제품의 두 순수 projection/window 함수 본문을 그대로 복사한 임시 probe이며 실제
codec와 Playback helper를 링크했다. 근거는 같은 폴더의 `projection_probe_result.json`과
`projection_probe_source_receipt.json`이다. 이 검사를 실제 Client 입력이나 scene 재생 증거로
표현하지 않는다. Resources 조회에는 `LOSTARK_RESOURCE_ROOT`를 실제 Client/Bin/Resources로
명시했다. 첫 임시 실행의 resource root 누락을 수정한 뒤 성공했다.

## G03. 실행과 사용자 확인 경계

Client가 실행 중이므로 제품 link, 재시작, UI 조작과 화면 캡처는 하지 않았다. root 통합 작업에서
제품 빌드를 마친 뒤 사용자가 F1 → Effect Tool → Current Effect로 확인한다.

1. Solo를 눌러 추가 Play 없이 시작하는지 확인한다.
2. 일반 클릭으로 A, Shift 클릭으로 B를 고르고 Play Group을 누른다.
3. 선택 요소만 원본 상대 타이밍으로 반복하는지 확인한다.
4. Stop, 다른 문서 Load, Play All에서 이전 선택 미리보기가 남지 않는지 확인한다.

실제 버튼 입력과 loop의 화면 결과는 사용자 확인 대기다. commit/push나 다른 세션의 dirty
파일 정리는 하지 않았다.

## G04. 통합 빌드 완료

사용자가 Client/Server를 종료한 뒤 Debug Product의 Engine/Shared/Server/Client compile·link·deploy를
완료했다. `out/BuildPipeline/runs/20260911T073907619Z-debug-product.json`의 모든 build step이
PASS이고 missingRuntimeInputs는0개다. 독립 읽기 전용 코드 검토에서도 즉시 재생·선택 구간·
provider·Stop/문서 교체·저장 sequence 보호의 신규 결함은 발견되지 않았다. 실제 버튼 입력과
화면 반복은 여전히 사용자 확인 대기이며, 실행 경로는
[통합 결과](2026-09-11_KOUKU_GATE1_TWO_PATTERN_FULL_RESTORE_IMPLEMENTATION_RESULT.md)의 마지막 절을 따른다.

## G05. Current Effect 본·root 그룹과 공동 위치 편집 (2026-09-14)

### G05-01. 반영한 동작

Current Effect의 `Group by anchor`는 현재 문서의 attachment enabled/follow, modelCue, runtime bone/binding, orientation, socket 전체 TRS·속도·회전과 transform owner로 그룹을 파생한다. 새 JSON 필드나 저장 그룹 ID는 없다. 알려진 SourceModelPreview actor `MN_RPCT_05`와 source-model attachment일 때만 `Left hand / b_wp_2`와 `Right hand / b_wp_1`를 표시한다. 양손 원본은 각각 11개이며 다른 actor의 같은 본 이름에는 좌우 의미를 붙이지 않는다.

그룹 헤더를 누르면 멤버 stable ID를 함께 표시하고 `Play Group`은 기존 selected Element preview를 사용한다. 펼친 멤버의 개별 Detail/Solo/선택 편집을 유지했다. `Group Center (bone-local m)`는 멤버 로컬 위치의 절대 평균이며, disabled attachment의 독립 root에는 `Group Center (effect-local m)`를 표시한다. WORLD muzzle 원본은 후자 11개 그룹이고 초기 중심은 [0,0,0]이다. 양손 원본은 각 [1.5,0.5,0]이며 중심 0을 원본 복원값으로 취급하지 않는다.

Enter는 새로운 중심과 이전 중심의 차이를 멤버 `Detail.Transform.position` 및 켜진 position lerp endPosition에 함께 더한다. 상대 위치·개별 S/R·source recipe·runtime anchor ID와 입자의 local/world 방출 정책은 보존한다. SourceContract, source transform track, master transform inheritance, RuntimeCarrier 및 서로 다른 particle-system parent basis의 혼합 그룹은 기존 owner에서 편집하도록 사유를 표시한다. 모든 새 위치를 먼저 범위·finite 검사하고 candidate 한 개만 commit한다. 미적용 Detail/ParticleSystem/ModelCue/OccurrenceTransform 입력은 Apply/Revert 전까지 그룹 이동을 차단해 보존한다. `Save Changes`가 기존 authored 저장 경로로 멤버 위치를 저장한다.

변경은 기존 `Effect_Tool_Internal.h`, `Effect_Tool_Helpers.cpp`, `Effect_Tool_Detail.cpp`, `Effect_Tool_ResourceBrowser.cpp`, `Effect_Tool_Playback.cpp`, `Effect_Tool_Workspace.cpp`, `Effect_Tool.h`에 한정했다. 행 UI는 기존 함수를 공통화했고 새 C++ 파일/project 등록은 없다. 사용자 Effect JSON과 Composition은 이 작업에서 변경하지 않았다.

### G05-02. 첫 미리보기 갱신의 문서 선택 수정

기존 `Try_CommitDocument(candidate)`는 active 문서 교체 전에 `Stage_WorldPreview(candidate)`를 호출하지만, sequencer factory가 그 candidate를 받지 않고 이전 active 문서를 다시 읽었다. 따라서 그룹 Enter의 미리보기가 한 번 전 위치를 사용할 수 있었다. 이제 동기 `Refresh_Effects` scope에서 candidate를 명시하고 exact V1 asset key만 갱신한다. factory의 `Resolve_AuthoringOccurrenceDocument`는 matching candidate를 먼저 복사하여 이전 dirty draft를 다시 적용하지 않는다. 일반 Play의 기존 active draft overlay는 그대로 사용한다. 기존 sequencer는 stage/sample 성공 후 객체를 교체하고 이후 `Try_CommitDocument`가 active 문서를 교체한다. 실패·예외에는 RAII가 이전 scope 포인터를 복원한다.

### G05-03. 실행한 검증과 한계

| 검증 | 실제 결과 |
|---|---|
| Helpers/Detail/ResourceBrowser/Playback/Workspace 5 TU 격리 Debug compile | PASS; VS18 Insiders, VC14.44, SDK10.0.26100.0. 기존 EngineSDK 문자셋 경고는 남음 |
| 현재 실제 함수 본문 추출 native probe + 실제 Codec/Playback | 38,329 checks PASS, 최대 수치 위치차 오차 8.58307e-6 m |
| 원본 양손 22개, 서로 다른 Element S/R, numeric anchor scale1.7과 yaw0/90/180 | 모든 22개 입자 위치는 그룹 delta만 변함. count/order/alpha/age/3x3 basis 보존 |
| WORLD muzzle 11개 | 11개 문서 위치·원본 다른 필드 보존. bounded native sample에서 발생한 10개를 yaw0/90/180·움직이는 numeric root로 비교 |
| candidate 우선순위와 failure | 실제 resolver/commit/group wrapper/refresh scope 본문으로 첫 refresh 새 위치, old draft 재적용 방지, dirty 4종 차단, stage 실패·예외의 기존 문서/preview 및 scope 복원 확인 |
| 저장/실패 경계 | 실제 Codec validate, out 전용 Save_Atomic/reopen 일치, NaN/범위초과/누락 그룹/SourceContract/sourceTrack 거절·원본 보존, lerp 전체 경로 delta 보존 |
| 동료 독립 수치 검증 | 6,617 checks PASS, 최대 9.90927e-7 m; `out/ShowtimeDualGroupOffset20260914/run.log` |
| read-only 후속 리뷰/파일 검증 | candidate factory 경계의 신규 결함 없음. 원래 파일별 BOM·줄바꿈 보존, `git diff --check` PASS |

주 근거는 `out/EffectToolBoneGroups20260914/receipt.json`, `probe-run.log`, `group_probe.cpp`, `compile.log`이다. 문서 선택·commit·그룹·scope는 실제 본문을 실행했고 GPU 객체 staging은 실패 주입 가능한 probe 경계다. WORLD `kouku.5.cc9e4867102a29553c83`는 이 sample의 baseline/edited 양쪽에서 입자를 발생시키지 않아 문서 변경까지 확인했다. 이 멤버의 GPU 표시나 모든 상황의 방출을 PASS로 기록하지 않는다. 이 그룹 probe의 anchor는 수치 행렬이며 실제 설치 본·GPU·Client 입력의 증거가 아니다. 실제 모델/총 pivot 검증은 별도 G06의 범위를 따른다.

소스와 격리 컴파일까지 완료했다. 실행 중인 Client/Server의 새 EXE 반영, 제품 통합 빌드와 사용자의 총구 육안 정렬은 상위 작업의 상태를 따른다. Client/UI를 실행·조작·캡처하지 않았다. 사용자는 Current Effect의 `Group by anchor`에서 손 또는 Effect root를 펼치고 공동 중심에 Enter를 입력한 뒤 `Save Changes`를 사용한다. Model View의 총 선택은 G06 미리보기 참조이고, 제품 Append Box의 왼/오른 총 WORLD anchor는 기존 Composition Box Detail에서 선택한다.

## G06. 저장 손 소품 참조와 WORLD Effect 총 앵커 (2026-09-14)

### G06-01. 구현된 연결

`EffectCompositionModelPreview.h/.cpp`는 명시적으로 선택한 저장 Pattern ID의 소품을 읽는다.
`Select_SourceEffect`의 선택 인자가 비어 있으면 기존 source model/animation만 사용한다.
인자가 있으면 같은 actor profile/target인지 확인한 뒤 저장 WORLD occurrence를 추가한다.
양손 원본의 `MN_RPCT_05`, `rpct00_att_battle_28_05_loop_a`, 3167ms source animation은 유지한다.
`Select_WorldEffectContext`는 source attachment가 없는 WORLD Effect에 선택한 Pattern의
기존 actor/animation과 소품을 제공한다. P32/P35의 같은 표시명으로 선택을 추정하지 않는다.

두 경로는 `Read_SavedPropContext`와 `Stage_ActorWorldProps`를 공유한다. Composition은 원문을
typed parser로 읽고 선택한 Pattern의 오류를 검사한다. WORLD 정의는 저장된 Area authoring
문서를 기존 Level validation targets로 검증하여 snapshot으로 보유한다. 같은 actor의
BOSS anchor, 본 이름, 단일 OBJECT_RESOURCE binding, 단일 emission만 소품으로 허용한다.
WORLD/PLAYER anchor, map/deploy, 다른 actor, NEXT motion chain과 내부 Effect track은 제외한다.
누락·유효하지 않은 참조는 실패 이유를 보존하며 다른 Pattern/객체를 대신 선택하지 않는다.

`EFFECT_COMPOSITION_MODEL_PROP`는 occurrence/world/instance stable ID, 표시명, 본과 원래
시작/길이를 제공한다. `Props()`는 Model View의 선택 목록을 위한 값이며 저장 그룹이나 새
런타임 부모가 아니다. 기존 Effect/Sequence JSON과 사용자 WORLD placement는 수정하지 않았다.

`KoukuSaydonPresentationPlayer.h/.cpp`의 opt-in Model Reference만 기존 Bundle WORLD player로
소품을 준비한다. Effect/Logic/Summon/Scene Profile은 제거하여 Effect Tool 재생을 재귀 호출하지
않는다. `Place_ModelReferenceRoot`는 명시 preview position/yaw를 actor와 facing seed에 같이
적용한 뒤 동일 시각의 소품을 갱신한다. 첫 프레임이나 회전에서 authored spawn yaw로 되돌아가지
않는다. `Resolve_ModelReferenceWorldPivot`는 정확한 member/occurrence의 기존
`Try_GetSequencePivot`를 읽으며, 실제 객체가 숨김/비활성 상태면 실패한다.

상위 작업의 `EffectAuthoringSequencer.h/.cpp`는 `Show saved hand props (Play All / Play Group)`,
`Load saved Object anchors`, `Effect preview anchor` 입력을 연결했다. 내부 본 부착 Effect는
`Source model / internal bones`를 사용하고, 중립 WORLD Effect만 특정 총 occurrence를 선택한다.
중립 WORLD의 root는 실제 총 pivot이며 원본 양손+총 WORLD의 중첩 연결은 거절한다.
Object root의 초기 중간 seek도 기존 60Hz model/history 경로를 사용하여 과거 입자 발생 위치를
현재 총 위치로 복제하지 않는다. 선택과 목록 revision은 미리보기 세션에만 남는다.

### G06-02. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| ModelPreview/PresentationPlayer 2 TU 격리 Debug compile | PASS, `out/EffectSourceProps20260914/compile.log` |
| 실제 함수 분기와 설치 MN_RPCT_05 + 저장 양손 총 native 검사 | 1483 checks / 0 failures |
| P35 16 clip × 시작/중간/끝 × 양손 본 | 96 samples, placement 오차 최대 2.38419e-7, root translation 오차 최대 6.10352e-5 |
| props opt-in/default 분리 | 기본 WORLD 없음, 명시 props만 보존, exact ID/span/placement 일치 |
| 실패 입력 | wrong member/Pattern/occurrence, 누락 정의, 다른 actor, NEXT, 내부 Effect, 다중 emission 거절 |
| preview root yaw 0/90/180 | 실제 Place_ModelReferenceRoot와 Facing 본문 통과 후 방향 보존 및 동일 frame 총 pivot 일치 |
| WORLD Effect Tool와 Product WORLD pivot | 현재 총의 uniform scale 2.1에서 기본 및 비영점 local TRS 일치 |
| 2000ms 초기 WORLD seek | actual Record_RowPivot/History와 실제 본: birthError=0, 1000ms 오차=0, 현재 자세와 출생 자세 차이=3.19792, 현재 clock 2000ms 복원 |
| 파일 경계 | 수정 4 H/CPP 원래 UTF-8 BOM 유무/CRLF 유지, 기존 dirty delta 보존, `git diff --check` PASS |
| 저장 자료 | 기존 native gun fixture와 현재 저장 정의 deep equality, 추출한 본/객체 함수가 현재 소스와 일치 |

검증은 `out/EffectSourceProps20260914/native_result.log`, `native-gun-samples.csv`,
`native_props_probe.cpp`, `source-hashes.json`, `final-receipt.json`에 기록했다. 실제 CModel의
설치 본과 기존 World object 합성은 실행했고, preview lifecycle/Level lookup/객체 visibility
조회는 통제한 probe 경계다. 전체 Client UI나 실제 Layer draw를 실행한 증거가 아니다.
상위 작업의 Sequencer TU compile·제품 통합 빌드 결과는 해당 작업의 최종 근거를 따른다.

### G06-03. 사용자 확인 범위

사용자는 Effect Tool Model View에서 `Pattern source > KoukuSaydon Patterns`의 정확한 P35
stable ID를 선택하고 저장 손 소품 옵션을 켠다. `Load saved Object anchors` 뒤 양손 원본은
`Source model / internal bones`로 두고 G05 본 그룹 Position을 편집한다. WORLD용 총구 Effect는
`Effect preview anchor`에서 왼/오른 총 occurrence를 골라 총 기준 Position을 편집한다.
소품의 원래 시작/끝은 유지되므로 선택한 시각에 존재하지 않는 객체는 임의로 연장하지 않는다.

Source model/props 참조 선택은 저장 Sequence의 제품 WORLD anchor로 자동 변환되지 않는다.
제품 배치는 기존 Composition의 정확한 WORLD occurrence 선택 경로를 사용한다. Client/UI
실행·입력·캡처 및 총구의 최종 육안 정렬은 수행하지 않았고 사용자 확인으로 남긴다.

### G06 통합 소스 검증과 제품 반영 상태

EffectAuthoringSequencer의 명시 Pattern/Object 선택, 같은 ID anchor 목록 재로드 시 snapshot revision 갱신, 실제 WORLD root와 60 Hz 과거 pose 연결을 컴파일했다. `out/EffectBoneGroup20260914/sequencer-final-compile.log`가 최종 Sequencer TU 근거다. 관련 C++ UTF-8 해독, 기존 project/filter XML와 세 개 현재 Effect JSON parse 및 git diff 검사는 통과했다. 독립 읽기 검토에서 candidate refresh와 WORLD root/history의 추가 결함은 발견되지 않았다. 저장된 Effect/Composition JSON을 외부에서 변경하지 않았다.

현재 Client 36424와 Server 59268이 실행 중이며 새 Product EXE 빌드·링크는 수행하지 않았다. 실행 파일을 보호하는 정규 빌드 경계에 따라 사용자의 저장·종료를 기다린다. 이 상태는 이전 G04의 09-11 제품 빌드 완료와 구분한다. 정확한 소스 hash와 집중 검사 경로는 `out/EffectBoneGroup20260914/final-source-receipt.json`에 기록했다. WORLD 기본 Append 거절 수정은 손 소품 RESULT G08에 연결된다. 최종 EXE 적용과 Client의 실제 위치·입력·화면 판정은 아직 남아 있다.


### 2026-09-14 후속 실행파일 확인

앞선 제품 빌드 대기는 사용자 Visual Studio 빌드로 해소됐다.16:54:34 Client.exe의 실제
compiler dependency·OBJ·link 입력을 현재9개 관련TU와 대조했고 현재 Shift 선택,
Effect anchor 그룹, saved prop preview, WORLD Append 변경 포함을 확인했다.
17:18:47 사용자 증분 Build 로그도 성공이다. 에이전트의17:19 Product 재빌드는 사용자
재실행으로 output guard가 거절했으므로 그 실행을 PASS로 기록하지 않는다.
증거는 `out/KoukuFlameUnification20260914/user-build-verification.json`과
09-12 KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT의G15-02다.
사용자는 시퀀스 재생을 확인했으며 각 이펙트의 최종 시각 판정은 별도 사용자 확인 범위다.
