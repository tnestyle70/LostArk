# Action Workbench 선택 복구와 독립 연출 Sequencer 결과

작성일: 2026-09-10. 구현·자동 검증·사용자 화면 확인을 구분한다.

## G00. 저장 데이터와 Preview/Server 위치 원인

현재 Action 원본 revision 229는 적용 완료 당시 requested-composition.staged.json과 바이트가 같다.
Pattern 24, Logic 35, Collider Resource 16개가 남아 있다. 원본이 다른 worktree 파일로 복사됐다는
증거는 발견하지 않았다. 이 비교는 보관한 저장본 밖의 편집 이력이나 미저장 변경까지 보증하지 않는다.

적용 전 사용자 Save 228과 229를 비교하면 파1빨2의 파랑·빨강 Effect 배치 4개는 같다.
레이저 3개는 이전 작업에서 머리 bone을 연결하면서 offset [2.15,1.85,0.30]을 [0,0,0]으로 변경했다.
이는 에이전트가 변경한 값이며 원본 소실로 설명하지 않는다. 이번 수정은 이 offset을 재변경하지 않았다.
대형 잡기 Collider는 보관된 228부터 반경 3m, 반각 45도, occurrence scale [1,1,1]이다.
22개 백업(revision 5~229) 중 다른 P17 크기를 가진 저장본은 찾지 못했다.
세부 증거는 out/ActionSequencer20260910/saved-geometry-history.json에 보관했다.

F1 Boss Tuning은 Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json에 G2 Big Saydon
위치 [10.24,10.0,317.75]를 저장했다. 갱신 전 Server worldbootstrap은 Y=8.63000011이었다.
단일 Pattern Preview의 임시 actor는 현재 저작 위치 Y10을 읽지만 Complete Play는 Server snapshot을
사용한다. F1의 live 표시 offset은 재시작/Reload 뒤 0이므로 배치 게시가 누락되면 1.37m 차이가 난다.
다른 MN_RPCT_06 패턴과 P11의 base Y 처리 분기는 없으며 파1빨2에서만 관측된 이유까지 확정하지 않았다.

## G01. 기존 Action Workbench 재열기

MainApp은 최초 shell 생성 시에만 arena를 확인하던 경로를 고쳤다. 재열기 시 현재 arena와 선택한
보스 계열이 다르면 명시 Open(boss)를 호출한다. 같은 쿠크 계열의 Gate2/3 선택과 dirty 초안은 유지한다.
Open(boss)와 Combo는 같은 Select_Boss를 통해 실제 session의 Gate filter를 맞춘다.
Open은 Patterns/Resources/Box Detail을 다시 표시하고 toolbar를 펼친다. 상단 Physical Animation
참고 트리는 기본 닫힘이며 Windows 메뉴에서 열 수 있다. Action 원본·ID·창 ID는 유지했다.

## G02. 요청한 동일 ImGui의 독립 Sequencer

F1 Action Workbench 버튼 바로 아래 Open Sequencer Benchmark를 추가했다. 기존
CKoukuSaydonActionWorkbench와 CSequencerTool의 별도 인스턴스로 동일 pane renderer와 기본 배치를
사용한다. Patterns 목록 창 이름은 Composition Sequencer이며 Resources/Box Detail/Preview와
시간축의 이동·trim·duration·Rename·Save·Reload를 같은 코드로 제공한다.

저장은 Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json,
compositionId는 boss.composition.kakulsaydon.sequencer다. Action과 교차 Load/Save를 거부하며 기존
atomic CAS 저장을 재사용한다. 초깃값은 팝업북 37,800ms와 1관문 피날레 21,010ms의 기존 World/Camera
참조다. 파티 등장·Server sequence 실행은 사용자가 후속 Summon/Logic 작업으로 미뤘다.
Sequence 모드는 Server Play/Publish 버튼을 숨기고 request API도 거부한다.

두 도구의 window/popup ID, category, draft, 선택과 playhead를 분리했다. 리소스 조회는 한 번 읽어
두 세션에 공급한다. 실제 Animation/World/Effect/Scene/Camera 미리보기 backend는 기존 하나를
사용하고 재생 요청한 세션만 clock·transport·geometry를 소비한다. 단순 창 열기나 비활성 창 닫기는
다른 미리보기를 정지하지 않는다. 실패한 WORLD admission은 stale 이전 WORLD를 정리한다.

팝업북 반복 미리보기는 실제 참조한 Deploy 상태와 겹치는 standing arena 표시 상태를 보관하고
Stop/실패/Level 종료에서 복구한다. 제품 cutscene 상태와 World/Map 저작 JSON은 변경하지 않는다.

## G03. F1 저장 보스 배치의 같은 버튼 게시

Publish All Patterns가 실행하는 Kouku domain owner에 기존 world.gameplay publisher를 연결했다.
-WorldId KAKULSAYDON_ARENA 범위는 쿠크 World만 생성한다. 같은 publisher/codec/transaction을
재사용하며 Product·World·balance 출력과 receipt를 함께 보관하여 뒤 단계 실패 때 함께 복구한다.

정상 owner 실행 결과 product/balance는 재사용했고 쿠크 World를 게시했다. Server worldbootstrap의
G2 Big Saydon Y=10을 확인했다. Action/World 원본과 다른 Server World 파일은 바이트가 같다.
증거: out/ActionSequencer20260910/publish-patterns-and-world.log 및 world-publish-result.json.
실행 중 Server의 world hot reload는 이번 구현에 포함하지 않으며 게시 뒤 Server 재시작이 필요하다.

## G04. Collider 편집 중 발견한 실제 회귀

기존 native editor 검사가 두 Geometry Collider에 피해량을 연결할 때 실패했다. 자동 ENTER_AREA
정의 재사용이 거미카운터의 7m 돌진 Trigger도 후보에 포함한 것이 원인이다. Set_ColliderTriggerDamage의
자동 후보에서 bossChargeDistanceM이 있는 정의를 제외했다. 사용자가 명시적으로 연결한 돌진 Trigger는
유지한다. fixture에서 charge 값을 지워 실패를 숨기지 않고 생성된 피해 Trigger의 charge=0을 검증했다.

## 자동 검증

- Scoped World publisher Validate: 쿠크 placement 105개, spawn group 1개 통과.
- Kouku owner rollback/owner selection 검사 2개 통과.
- Action 재열기·Sequence 창/저장·Server request 차단 검사 4개 통과.
- Native --kouku-sequence-document-contract 빌드/실행 통과: 실제 seed 읽기, 독립 atomic Save/reopen,
  교차 ID 거부, 외부 변경 CAS 실패, Action 원본 바이트 보존.
- 전체 native --kouku-composition-editor-contract 빌드/실행 통과. 자동 피해 Trigger가 돌진을 재사용하지
  않는 검사를 포함해 Save/Reload·복제/삭제·249-stage·Logic/Presentation/CAS 검사를 통과했다.
- Server --kouku-bundle-contract-test 및 --world-playback-contract-test 각각 failures 0.
- 최종 Debug Product(Engine→Shared→Server→Client) 빌드·링크·SDK/Shader/DLL 배포 통과.
  증거: out/ActionSequencer20260910/product-build-verified.log, BuildPipeline run 20260909T211545966Z-debug-product.json.
- 변경 JSON/XML parse와 전체 git diff --check 통과. 기존 경고는 남아 있으며 화면 PASS를 뜻하지 않는다.
- 최종 실행 파일/저장 revision과 보존 결과: out/ActionSequencer20260910/final-verification.json.

첫 Product 시도는 현재 워로드 shader의 float4 배열 초기값 오류, 재시도는 Engine PDB 경합으로 실패했다.
후속 Client 컴파일에서 Effect_DocumentCodec의 CEffectPlayback 선언 include 누락도 확인했다.
Shader_EffectWarlordNativeGroup1088.hlsli의 4개 float4 배열에서 숫자 0을 명시 float4(0,0,0,0)으로
맞췄고, Effect_DocumentCodec.cpp에는 기존 Effect_Playback.h include 한 줄을 추가했다. 수치나 Effect
의미를 변경하지 않았다. 빌드 호출에 /FS를 사용한 최종 Product가 통과했다. 다른 세션의 기존 변경은
되돌리지 않았으며 이 최소 빌드 교정 외 Effect 작업의 완료를 대신 판정하지 않는다.

## 사용자 화면 확인

Client/UI는 실행하거나 캡처하지 않았다. 새 실행 파일로 Server와 Client를 재시작한 뒤 쿠크 아레나에서
F1 → Open Action Workbench로 기존 Logic/Collider를 확인한다. 바로 아래 Open Sequencer Benchmark는
Composition Sequencer 목록과 같은 Resources/Box Detail을 연다. 두 편집기의 선택·Save·Preview를
각각 확인한다. 파1빨2 Complete Play에서 본체 높이와 앵커 Effect를 사용자가 최종 판정한다.
