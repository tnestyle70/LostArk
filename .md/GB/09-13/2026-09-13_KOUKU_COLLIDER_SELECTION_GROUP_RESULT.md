# 쿠크 콜라이더 선택 그룹 결과

## G00. 구현 상태

Kouku Action Workbench에 Set Group/Ungroup, 그룹 전체 선택과 공통 중심 이동·Y 회전을 반영했다. 선택 그룹은 BOSS 기준 Collider 두 개 이상이며 동일 bone/boneTarget/follow를 요구한다. Follow=false면 고정 기준 시각도 같아야 한다. 사용자가 배치한 무지개댄스 Collider 5개는 BOSS/follow=true여서 대상이다. 원본 Data의 박스나 그룹은 에이전트가 직접 변경하지 않았다.

## G01. 문서·선택·공동 변환

- `KoukuSaydonCompositionDocument.h/.cpp`: optional `selectionGroupId`를 sparse 저장한다. 기존 aggregate 초기화 순서를 유지하도록 필드 끝에 추가했다. collider-only, stable ID, Pattern별 2+ 멤버와 동일 기준 검증을 연결했다.
- `KoukuSaydonActionWorkbench.h/.cpp`: Set Group/Ungroup, 한 멤버 클릭·Ctrl 토글·마키의 그룹 확장, 평균 중심 기준 XYZ 이동·Y 회전을 연결했다. 회전은 각 중심의 XZ offset과 yaw에 적용하고 시간·수명·크기·Logic을 유지한다. 그룹 선택 중 개별 박스 시간 drag는 하지 않으며, 개별 편집은 Ungroup 후 수행한다.
- 복제는 그룹 전체를 새 stable group ID로 복사하고 기존 ID와 충돌을 검사한다. 명시 삭제 목록의 의미는 유지하면서 한 멤버만 남은 그룹은 해제한다.
- group geometry 전체를 먼저 검사하고 작은 row overlay만 stage한다. invalid/overflow에서 앞 멤버만 이동하지 않는다. 기존 Preview queue가 batch를 소비하고 Save는 기존 단일 candidate와 원자 저장을 사용한다.
- Python projector는 optional authoring 필드를 검증하고 제품 presentation projection에서는 제외한다. Parent 반복·잘림의 파생행에서도 제거하여 편집 그룹이 runtime 참조나 잘린 singleton으로 남지 않는다.

기존 단일 박스 편집의 원자 저장·CAS·외부 변경 거절을 완화하지 않았다. 별도 그룹 파일, 부모 Transform, 중첩 그룹 또는 Server 충돌 runtime은 추가하지 않았다.

## G02. 비용 검토

최초 조사 시점 원본은 572,190 bytes, 48 Pattern, presentation occurrence 242개와 Collider 94개였다. 사용자가 계속 저장 중이므로 고정된 최종 파일 크기는 아니다. 실제 무지개 5개에 짧은 그룹 ID를 저장하는 추가량은 약 350 bytes, 조사 시점 전체의 약 0.06%다.

저장은 기존 전체 문서 validate/serialize/temp-reparse/CAS/reopen 비용을 유지하고 그룹 검증은 Pattern별 행 순회와 ID map으로 수행한다. 드래그마다 멤버별 전체 문서 commit이나 파일 저장은 하지 않는다. 5개 그룹 때문에 구조가 크게 복잡해지거나 저장 I/O 병목이 새로 생길 근거는 없다. 실제 사용 중 Save 지연 시간 자체를 계측한 결과는 아니므로 latency 개선으로 기록하지 않는다.

## G03. 검증

- 변경된 Workbench·Document와 기존 native editor test의 Debug 컴파일·격리 test EXE 링크 성공. 정본 제품 출력은 사용하지 않고 `out/KoukuColliderGroups20260913` 아래의 IntDir/OutDir을 사용했다.
- 기존 native harness의 `--kouku-collider-group-contract` 성공: 5개 Set Group, 한 멤버 클릭 뒤 공통 중심 90도 회전과 이동, Save/Reopen, 크기·시각·기타 필드 보존, 복제 그룹 분리, Ungroup, 삭제 후 singleton 정리, NaN 입력 거절, 외부 저장 충돌 시 draft/overlay/디스크 보존.
- Python 신규 그룹 관련 4개와 관련 기존 2개 테스트 성공. 전체 module 154개 실행은 실패 11/error 59였고 대표 기존 실패 3개는 변경 전 HEAD에서도 같은 원인으로 재현했다. 현재 Data와 기존 광역 fixture 기대값 불일치를 이 기능 수정으로 덮지 않았다.
- native 테스트의 live Effect geometry 기대값은 이미 바뀐 Effect placement 계약(geometry와 anchor를 함께 stage)을 반영하도록 교정했다. 이펙트의 bone을 무조건 미적용 필드로 보던 기존 기대값이 현재 구현과 달랐다. 별도 native fixture의 live inventory/world 조회는 기존 CPU-only fail guard 방식으로 차단했다.
- 기대값 교정 후 기존 `--kouku-preview-transport-contract` 전체도 성공했다. 기존 단일 Collider/Effect geometry, Apply/Save/CAS, 선택 동기화, 시간 유지, 다른 lane 복제 회귀를 함께 확인했다. 증거는 `native-tests-final.log`다.

증거는 `out/KoukuColliderGroups20260913/build-final.log`, `native-groups.log`에 있다. 제품 Client EXE 링크와 사용자의 실제 화면 판정은 이 격리 검사에 포함되지 않는다.

## G04. 사용자 확인

사용자가 직접 컴파일 중이고 Client/Server도 실행 중이므로 에이전트는 종료·Reload·제품 출력 교체를 하지 않았다. 새 제품 빌드 실행 후 `F1 → Action Composition Workbench → KoukuSaydon → 무지개댄스 → Collider 5개 Ctrl/마키 선택 → Box Detail → Set Group → 한 멤버 클릭 → Group center offset / Group Y rotation → Save`로 확인한다. 다시 열었을 때 그룹 선택이 유지되고 개별 시작 시각과 간격·크기가 유지되는지 확인한다.

이번 회전 입력은 기존 Box Detail 숫자 드래그 방식이다. 별도 화면 회전 gizmo는 추가하지 않았다. 최종 시각·입력 판정은 사용자 확인 전으로 남긴다.

## G05. Effect 선택 그룹과 시간 이동 (2026-09-14)

`selectionGroupId`를 Effect에 확장했다. Box Detail의 `Set Group`/`Ungroup`, 한 멤버 클릭·Ctrl/마키의 전체 그룹 선택, Save/Reopen, 복제 그룹 ID 분리와 삭제 후 singleton 정리를 기존 presentation 계약으로 사용한다. Effect는 서로 다른 왼 총·오른 총 anchor, bone/follow, offset/rotation/scale을 유지한다. Collider의 BOSS 공통 기준과 공간 변환 제약은 바꾸지 않았다.

Effect 그룹 및 임시 Effect 다중 선택의 가운데 드래그는 모든 시작 시각에 같은 delta를 더한다. 전체 멤버를 기준으로 Pattern 시작·끝에 clamp하고 상대 간격·수명·geometry를 보존한다. 그룹 ID와 selection/draft generation이 바뀌면 원래 시각을 유지한다. Effect 이동은 부모 World와 Logic을 이동시키지 않는다. Collider의 연결 Logic 동시 이동·외부 공유 거절은 기존대로 유지한다.

WORLD Effect의 `worldOccurrenceId`는 참조이므로 Effect만 복제할 때 총/Object를 자동 복제하던 ownership 확장을 수정했다. Effect 그룹 복제는 각 기존 총 anchor를 그대로 사용한다. World 박스를 명시 선택하면 World와 부착 Effect를 함께 복제하고 새 World occurrence로 remap한다. WORLD anchor가 아닌 companion의 owner 포함 복제는 유지한다.

자동 검증:

- Workbench·Document·확장 native test의 Debug 컴파일과 격리 native EXE 링크 성공. `out/KoukuEffectGroups20260914/build-ready.log`에 기록했다. 링크의 기존 live UI/catalog 의존성 7개는 같은 테스트 파일의 기존 CPU guard 방식처럼 호출 시 실패하도록 명시했다. 제품 runtime을 대체하지 않으며 실제 editor draft API를 호출한다.
- 기존 `--kouku-collider-group-contract` 확장 실행 성공. 서로 다른 좌우 WORLD anchor와 follow의 Effect Set Group, invalid/singleton 실패 보존, Save/Reopen, 한 멤버 입력으로 전체 그룹 복제, 총 개수 불변과 새 group ID, Ungroup, singleton 정리, 명시 World 복제의 부착 Effect remap을 확인했다. 기존 Collider의 공통 중심 회전·Save CAS 보존도 같은 실행에서 성공했다. `native-group.log`가 증거다.
- 실제 `Prepare_PresentationTimelineMove`, `Move_PresentationTimelineSelection`, row layout과 기존 Collider 공간 변환 함수 본문을 사용한 CPU 검증 95개 성공. grouped/ungrouped/multiple-group Effect 이동, 다른 총 anchor/TRS 보존, 부모 World/Logic 불변, 혼합 선택 거절, 양끝 clamp와 stale/partial/shared-Logic 실패를 확인했다. Commit/preview I/O만 stub이며 GPU/ImGui 입력 검증은 아니다. `group_timeline_probe.receipt.json`과 `.run.log`에 기록했다.
- Python 6개 성공: Effect 독립 총 anchor Save/reparse, runtime projection 동일, singleton/mixed-kind/invalid ID 거절, 기존 Collider 공통 frame/frozen-time 제약과 parent clipping metadata 제거. `selectionGroupId`는 sparse optional 상태를 유지한다.
- 변경 파일의 `git diff --check` 성공. 기존 파일의 UTF-8/BOM/줄바꿈을 유지하고 C++/project 항목을 추가하지 않았다. 사용자 Composition JSON과 실행 중 Client/Server는 조작하지 않았다.

제품 링크/재실행은 상위 작업의 Product 빌드와 사용자 조작에 연결한다. 사용자 확인 경로는 Effect 박스 둘 이상 Ctrl/마키 선택 → Box Detail → Set Group → 멤버 가운데 드래그 → Save → Reload이며, Effect 반복 복제 뒤 World 총 박스가 증가하지 않는지도 확인한다. 최종 화면/실제 ImGui 드래그 판정은 사용자 확인 전이다.

추가 회귀 확인: 기존 `--kouku-preview-transport-contract` 전체도 성공했다(`out/KoukuEffectGroups20260914/native-preview.log`). All-lane segment copy, World companion ownership, pending placement, parent 반복/잘림, Save/Reload와 실패 rollback 경로를 함께 확인했다. 최종 소스 hash와 검증 요약은 `effect-group-receipt.json`에 있다.

상위 작업에서 사용자의 Save 및 Server/Client 종료를 확인한 뒤 정규 Debug Product의
Engine/Shared/Server/Client 빌드·링크·배포를 완료했다. 최종 빌드 근거는
`out/BuildPipeline/runs/20260914T061704289Z-debug-product.json`이며 OBJ80개와 Client binary1개를
작성했다. 이 기록은 위 제품 링크 대기를 갱신하며 실제 ImGui 입력·화면 판정은 사용자에게 남긴다.

## G06. Shift 클릭 다중 선택 지원 (2026-09-14)

Composition Sequencer에서 Shift+박스 클릭을 기존 Ctrl+클릭과 같은 toggle 선택으로 연결했다. Stage/Animation/Logic/Summon/World/Scene Profile/child Pattern/Effect 8개 클릭 입력과 modifier 중 drag 차단, Effect 선택 유지, 빈 공간 marquee의 선택 초기화가 같은 `additiveSelection = io.KeyCtrl || io.KeyShift`를 사용한다. Ctrl/Shift+빈 영역 drag는 기존 선택에 박스를 추가한다. Collider/Effect Box Detail과 Timeline 안내 문구를 갱신했다. 여러 Effect를 Shift로 선택한 뒤 기존 `Set Group` → `Duplicate`/Ctrl+D 경로를 사용한다.

`KoukuSaydonActionWorkbench.cpp` 한 파일을 VS18 Insiders/VC14.44/SDK10.0.26100.0 Debug x64로 out에 격리 컴파일하여 성공했다. `out/CompositionShiftSelection20260914/compile.log`, `receipt.json`, `selection-only.diff`가 근거다. 18개 modifier 소비 지점과 8개 lane click 연결을 확인했고 Ctrl+D/방향키 분기부터 파일 끝까지 원래 바이트와 동일하여 기존 WORLD-default Append 수정과 다른 작업 변경은 보존했다. Set Group/복제/저장 구현은 이번 변경에서 수정하지 않았으므로 이미 기록한 native group 회귀를 반복하거나 별도 테스트 framework를 추가하지 않았다. BOM 없음/CRLF와 `git diff --check`를 확인했다.

위 이전 Product 빌드 기록에는 이 Shift 별칭이 포함되지 않는다. Client/Server가 실행 중이어서 이번 작업은 소스·TU 컴파일까지 완료했고 새 EXE 반영은 상위 작업의 저장·종료 후 빌드에 연결한다. 사용자 JSON과 Client/UI는 조작하지 않았다. 사용자가 새 빌드에서 Shift로 두 Effect를 선택 → Box Detail Set Group → Duplicate를 확인하며 실제 ImGui 입력 판정은 남아 있다.


### 2026-09-14 후속 실행파일 확인

앞선 제품 빌드 대기는 사용자 Visual Studio 빌드로 해소됐다.16:54:34 Client.exe의 실제
compiler dependency·OBJ·link 입력을 현재9개 관련TU와 대조했고 현재 Shift 선택,
Effect anchor 그룹, saved prop preview, WORLD Append 변경 포함을 확인했다.
17:18:47 사용자 증분 Build 로그도 성공이다. 에이전트의17:19 Product 재빌드는 사용자
재실행으로 output guard가 거절했으므로 그 실행을 PASS로 기록하지 않는다.
증거는 `out/KoukuFlameUnification20260914/user-build-verification.json`과
09-12 KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT의G15-02다.
사용자는 시퀀스 재생을 확인했으며 각 이펙트의 최종 시각 판정은 별도 사용자 확인 범위다.

### G06.1. Effect 그룹 공간 이동과 같은 시각 복제 후속 (2026-09-14)

사용자가 쇼타임의 바닥 고정 조준점·공 낙하·충돌 폭발·화염 장판을 함께 이동하고 여러
위치에 복제·저장할 수 있도록 기존 Workbench CPP와 헤더를 확장했다. 저장본에서
P35.effectgroup.182는 낙하178/충돌179/화염180 세 MAP 박스이며, fixed target91도 MAP다.
사용자가 계속 편집 중이므로 이 관찰값을 덮어쓰거나 외부에서 그룹을 자동 재지정하지 않았다.

Box Detail의 Effect selection에 `Group position (world m)` 또는 공통 anchor의
`Group center offset (m)`를 추가했다. 현재 staged placement를 포함한 모든 멤버 중심의
평균을 표시하고, 새 위치와 차이만 모든 occurrence의 PositionOffset에 적용한다.
전체 후보를 먼저 검증하므로 잘못된 수치에서 일부 멤버만 바꾸지 않는다. 회전·크기·anchor·
시각·수명은 유지한다. MAP는 서로 다른 시작 시각도 같은 세계좌표이며 BOSS/WORLD는
같은 anchor/bone/target/world occurrence/emission/Follow 기준을 요구한다. frozen BOSS/WORLD는
시작 시각도 같아야 한다. 다른 기준이 섞이면 위치 입력에 사유를 표시하고 기존 선택·시간
이동·복제는 계속 유지한다.

`Duplicate Group (same time)`은 기존 Duplicate_TimelineSelection의 기본 false 옵션
atOriginalTime을 사용한다. 소유 참조 확장 뒤 Effect만 남은 경우 insertMs=first/delta=0으로
현재 시각·수명·전체 Pattern 길이를 보존한다. 기존 old→new occurrence 매핑으로 미저장
위치까지 복사하고 새 group ID를 발급해 복제본을 선택한다. 원본과 기존 WORLD 참조는
유지된다. 기존 Ctrl+D는 후속 시간 복제 그대로다. `Save Composition`/`Save Sequence`는
기존 문서 Save를 호출해 모든 미저장 문서 편집과 staged geometry를 함께 저장한다.
별도 group Transform/schema/runtime/리소스 원본 수정은 없다.

독립 검토에서 새 그룹 복제 후 기존 preview snapshot이 새 occurrence ID를 모르는 경로를
찾아 수정했다. 같은 시각 복제 성공 뒤 pending→현재 동일 Pattern 재생→cursor 순으로
clock/paused를 보존하고 Request_PatternPreview를 다시 요청한다. 이 경로는 전체 staged
geometry를 포함한 새 문서를 검증·확장한 뒤 준비하므로, 이후 복제 그룹 위치 입력을 받을 수
있다. preview 준비 실패 시 복제 draft와 이전 preview는 유지하고 unavailable 상태를 표시한다.
최종 재검토에서는 추가 결함을 발견하지 않았다.

위치 preview는 기존 Workbench queue → MainApp → Player의 V1/V2 anchor/history 경로를
사용한다. MainApp의 최종 Sample_Preview/Player.Status/Set_PreviewState가 Player 오류를
전달한다. 같은 프레임의 순차 geometry 소비를 runtime 전체 rollback이 보장되는 batch로
기록하지 않는다. 저장은 기존 Save_Atomic의 검증·freshness/CAS·원본 보존 계약을 유지한다.

최종 Workbench CPP의 out 격리 컴파일이 Product 문자 집합 설정으로 통과했다. 파일의
UTF-8 BOM 없음/CRLF와 git diff --check를 확인했다. 이전 양손 자동 소품과 마커 관련10개
소스는 직전 제품 빌드의 hash와 그대로 일치하며 Resources 캐시를 포함한 Workbench 위에
이번 변경만 추가했다. 근거는 `out/EffectGroupPlacement20260914/compile_receipt.json`,
`compile.log`와 변경 직전 `.before` 파일들이다. 새 C++ 파일/project 등록은 없다.

최종 Collect/Translate/Duplicate 함수 본문을 추출한 native focused probe는179검사에서
실패0이다. 실제 P35 MAP3개와 서로 다른 시작 시각의 공통 이동, 상대 위치·회전·크기·시각
보존, invalid/overflow/mixed frame 거절, staged 위치 복제, 새 occurrence/group ID,
pending/live/cursor의 preview clock·paused 보존, 새 ID snapshot 뒤 위치 편집을 확인했다.
실제 occurrence codec 행을 직렬화·재해독해 position/selectionGroupId를 포함한 행을
대조했다. Workbench Save와 overlay 함수 본문은 변경 전과 같음을 확인했다. 전체
Save_Atomic 파일 트랜잭션과 실제 GPU/UI 입력을 실행한 검사는 아니다. 증거는
`out/EffectGroupPlacement20260914/placement_probe.run.log`, `placement_probe.receipt.json`,
`placement_rows.roundtrip.json`이다.

사용자가 **편집 중이므로 EXE 빌드를 잠시 보류**하도록 요청했다. Client6236/Server51620의
실행파일은 교체하지 않았고, 이번 기능은 소스·격리 컴파일·집중 검사까지 완료된 상태다.
제품 반영은 사용자의 후속 빌드 지시 뒤 진행한다. 그 뒤 Effect4박스 선택 → Set Group →
Group position XYZ → Duplicate Group (same time) → 복제본 위치 변경 → Save Composition →
재열람의 실제 입력과 화면 배치는 사용자가 확인한다. 에이전트가 사용자 Effect/Composition
JSON을 수정하거나 Client/UI를 실행·조작·캡처하지 않았다.


## G06.2. 후속 빌드 승인과 제품 반영 — 2026-09-14 23:26 KST

사용자의 “전부 복원 시킨 다음에, 빌드까지 깔끔하게 돌려줘” 지시로 위의 빌드 보류가
해제됐다. 실행 중인 Client/Server가 없음을 확인하고 Debug Product 빌드와 배포를 마쳤다.
그룹 XYZ 이동·같은 시각 복제·Save와 기존 Resources 캐시·양손 소품·마커 최적화가 포함된다.
후속 V1 Append 수명 수정도 포함되며 기존 그룹 세 함수와 저장 계약은 유지한다.

최종 실행 파일은 Client/Bin/Debug/Client.exe, 23:26:07 KST, 58,626,048 bytes,
SHA256 d5f0fdfb776b34ced5620a51ecff3529cf77ae3b987b4b7b8d03fa360a5e4fad다.
정본 Product receipt는 out/BuildPipeline/runs/20260914T142608661Z-debug-product.json이다.
32개 입력의 빌드 전후 hash 동일, 그룹 복제·캐시·마커·native 3607 EXE 문자열 포함은
out/KoukuShowtimeRectangle20260914/final_product_receipt.json에 기록했다.
컴파일·링크와 필수 runtime input은 PASS이며 기존 문자 집합 경고는 남아 있다.
Client/Server는 에이전트가 실행하지 않았다. 사용자 저장·재열람·그룹 위치의 실제 화면
확인은 위 G06.1 절차로 진행하며, 빌드 성공을 사용자 visual PASS로 기록하지 않는다.
