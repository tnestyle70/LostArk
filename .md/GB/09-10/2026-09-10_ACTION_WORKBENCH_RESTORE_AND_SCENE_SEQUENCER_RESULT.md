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

## G05. 1관문 두 연출 Complete Play — 2026-09-11

재개 기준 bba47ad00269c91e0b97391c14ce0f2fed660f09의 최신 재질·gameplay를 보존하고,
독립 Sequence의 Complete Play를 구현했다. 선택 Gate의 document order를 stable pattern ID로
확정하여 첫 항목 0ms부터 요청한다. 자연 종료만 다음 항목으로 이동하며 마지막 항목 뒤에는 반복하지
않는다. Pause/Resume, 명시 Stop/Reset, admission 실패, WORLD sample 실패와 owner 교체를
기존 Preview 경계로 연결했다. Action Workbench의 Server Complete Play 의미는 유지한다.

자연 종료 신호는 MainApp이 같은 프레임의 transport를 소비한 뒤 확정한다. 마지막 프레임에서
Pause/Seek를 눌렀을 때 presentation clock이 먼저 해제되어 다른 backend로 명령이 넘어가거나
다음 연출로 진행하지 않게 했다. WORLD sample 오류는 occurrence와 원래 오류 이유를 표시하고
연속 재생과 차용 상태를 정리한다.

Sequence 정본 revision은 2다. 팝업북의 5개 맵 WORLD box는 현재 template의 4,507ms/속도 1을
소비하고, 종료 시 움직이는 복제본을 숨긴 뒤 기존 Level의 standing arena를 표시한다. 역방향 scrub은
다시 연출 배치를 표시한다. 책·Saydon·Camera 37,800ms와 피날레 21,010ms는 유지했다.
Stop/실패/다음 연출 전환은 미리보기 전의 Deploy 상태와 standing arena 표시 상태를 복구한다.

## G06. 실제 1관문 Saydon과 맵 재질 연결

Imported deployassets의 stable ID `DEPLOY_BOSS_MN_RPCT_00`과 placement 5를 유지하면서 모델을
`Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`로 교체하고 Area publisher로 배포했다.
이 ID는 저장 호환용 이름이며 현재 모델이 RPCT00이라는 뜻이 아니다. 실제 Gate1 BossCatalog가
가리키는 RPCT05와 동일한 모델이다. Deploy의 기존 `CActorCatalog::Build_ModelLoadDescription`
→ `CModel` 경로와 source character material bind가 복구된 5개 override를 소비한다.
Resources 바이너리나 두 번째 모델 runtime은 추가하지 않았다.

RPCT00은 3 mesh/material, RPCT05는 5 mesh/material이며 두 모델은 168 bone·249 clip 이름을
공유한다. 실제 skeleton/clip bytes는 다르다. 연출이 소비하는 12개 clip 이름이 RPCT05에 모두 있으며,
복구 override가 참조하는 texture 21개의 존재를 확인했다. 원본 연출 캐스팅을 그대로 재현했다는
의미가 아니라 사용자가 요청한 실제 Gate1 모델의 자체 animation과 복구 재질을 연결한 변경이다.

움직이는 맵은 136 placement·37 model·40 mesh-used material slot이며 현재 BG8 복구 재질과
일치한다. standing arena는 별도 461 placement이고 그중 411개는 RNM 배치 조명을 가진다.
연출 모델 37개 중 36개의 mesh 이름은 standing 쪽에도 있지만 material variant 5종과 연출 전용
중앙 면 `lv_module_mesh03_512`가 다르다. 따라서 연출용 배치와 미복구 맵은 같은 뜻이 아니다.
고정 아레나의 RNM을 움직이는 부품에 억지로 복사하지 않고 Level의 두 배치 표시를 전환한다.

두 번째 `circus_finale`는 현재 23개 맵 placement의 editor-authored 움직임이며 배우 animation
track과 source package/export 연결 정보가 없다. 로컬 원본 map package 17개의 InterpData 조사에서도
현재 21,010ms timeline에 직접 대응하는 원본을 찾지 못했다. 기존 피날레를 이어 재생하도록 유지했으며
원본 배우 연출 복원 완료로 기록하지 않는다. 후속 Summon·Effect·Scene Profile 저작은 사용자 작업이다.

## G07. 2026-09-11 자동 검증과 남은 실행 단계

- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish`와
  같은 범위 `-Mode Check` 성공: placement 3,368개, 출력 8개. 실제 내용 변경은 deployassets의 모델 참조다.
- Client x64 Debug `ClCompile` 성공. 최신 수정된 PresentationPlayer까지 컴파일했고 기존 코드 페이지
  경고는 남아 있다. 로그: `out/gate1-client-compile.log`.
- 기존 native 검사 프로젝트 x64 Debug 빌드·링크 성공 후 실제
  `ValtanPatternAuditionServiceHarness.exe --kouku-sequence-document-contract` 실행 exit 0.
  첫 항목/0ms/문서 순서, queued pause, 잘못된 완료 신호, 실패 이유, 마지막 종료, owner 취소,
  독립 원본·atomic Save/Reload·CAS 보존을 검사했다. 빌드 로그: `out/gate1-sequence-harness-build.log`.
- 기존 Python의 Sequence pane/storage 분리와 Server request 차단 검사 2개 통과.
- 변경 Sequence JSON parse와 `git diff --check` 통과. 이번 변경에 XML/project/filter 추가는 없다.
- Client/UI는 에이전트가 실행·조작·캡처하지 않았다. 사용자가 “현재 사용 중 — 파일 교체 보류”로
  회신하여 최종 링크와 실행 파일 교체를 보류했다. 현재 Client.exe는 14:45:41 빌드이며 마지막
  PresentationPlayer 수정은 14:51:02여서 새 수정 전체가 실행 파일에 포함된 상태는 아니다.
  컴파일 및 검사 EXE 성공을 제품 Client 실행 완료로 기록하지 않는다.

사용자 화면 확인 경로는 Lobby → KoukuSaydon → F1 → Open Sequencer Benchmark →
Saydon/1관문 → Complete Play다. 첫 팝업북의 펼침 뒤 실제 아레나 표시, RPCT05의 재질·animation,
두 번째 저장 피날레 진입과 Stop/재시작 결과는 새 Client 실행 파일에서 사용자가 확인한다.

## G08. 쿠크 로더의 named translucent family 회귀 수정

사용자 Client 11976·24352의 session diagnostic은 Server `entry.accepted` 뒤
`CLIENT_LOAD_FAILED`, `loading.target-resource-load: [Loader] Map: explicit area catalog`를 기록했다.
Deploy 모델 준비 이전의 실패다. 실제 `CMapAssetCatalog::Load_Area`를 UI 없는 CPU probe에서
실행한 수정 전 결과도 `invalid native program inputs`로 실패했으며 처음 거절된 행은
`MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C/SLOT_000_dummy_material_0`다.

공용 dispatcher가 `source.map.translucent-` prefix만 보고 기존 named family까지 숫자형 44~63
handler로 보낸 것이 원인이다. tiled 2행, reflection 2행, bump 1행을 각각 기존 Forward program
34/35/36으로 명시 분기했다. 실패 후 fallback이나 재질 데이터 삭제는 하지 않았다.
Loader의 맵 카탈로그 실패도 기존 ActiveStatus에 원래 이유를 보존해 recovery 진단으로 전달한다.

수정 전 native 실행 증거는 `out/KoukuGate1CatalogNative20260911/baseline.log`와
`catalog-native-before.exe`에 보존했다. 임시 probe는 제품 MapAssetCatalog/DataJson/RuntimeAssetRoot/
ProjectDataRoot C++를 그대로 사용하며 별도 out object에 컴파일했다. Resources와 제품 intermediate는
진단 준비 과정에서 변경하지 않았다. Client 최소 Debug 컴파일은 exit 0이며
`out/gate1-loader-compile.log`에 기록했다. 기존 코드 페이지 경고는 남아 있다.

수정 후 동일 native 실행은 `Catalog ready (v5): 1301`로 성공했다(2,271ms).
실제 쿠크 named 5행은 program 34/35/36, 베른 numbered 525행은 program 44~63을 유지했다.
잘못된 family/부족한 입력 1,060건 거부와 출력 보존, 카탈로그 후속 로드 실패 시 기존
ready/AreaId/entry 보존도 확인했다. 결과는 `out/KoukuGate1CatalogNative20260911/after.log`,
failures 0이다. 이는 catalog admission 검증이며 사용자 Client의 실제 화면 진입 확인은 아직 남아 있다.
