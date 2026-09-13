# 쿠크·세이튼 전체 이펙트 편집 결과

## G05 후속 — 2026-09-14 Sequence World 위치 버튼

첨부 화면의 `KAKULSAYDON_G1_PATTERN_4.presentation.35`는 원본 이펙트 문제가 아니라
`WORLD`, 빈 `worldId`, `followBoss=true`인 occurrence였다. 연결 resource의 기본값도 WORLD라
Append가 오브젝트 identity 없는 연결을 만들 수 있었고, Player/Mouse 버튼은 MAP에만 표시됐다.

`KoukuSaydonActionWorkbench.cpp/.h`의 기존 배치 경로를 수정했다. 새 Sequence Effect는 MAP으로
시작하고 명시 선택한 World box 연결만 유지한다. 기존 WORLD에서도 Player/Mouse 버튼을
제공하며, 마우스 요청·취소는 원래 박스를 유지하고 유효한 hit에서만 MAP/좌표/참조 해제를
함께 stage한다. 회전·크기·시각을 보존한다. 이미 배치한 Effect의 Resources Preview도
기존 좌표·시각을 유지하도록 신규 초기화와 구분했다. 실행 중 Composition 파일은 수정하지 않았다.

수정 TU의 실제 OBJ 컴파일은 Product와 같은 문자 집합 옵션으로 통과했다. 기존 MAP projection
회귀 1건(V1_EFFECT/V1_ELEMENT/GROUP 및 부착 참조 거절)과 수정 파일 `git diff --check`도 통과했다.
증거는 `out/EffectWorldPick20260914/compile.log`다. 실행 중 Client PID 9404와 Server PID 58992를
확인했고 종료·UI 조작·화면 캡처는 하지 않았다. 실행 중인 Client는 이전 EXE이며 최종 링크와
사용자 화면 확인은 별도다.

Production 피킹 함수 본문을 추출한 창 없는 CPU 검사 26건도 통과했다
(`out/EffectWorldPick20260914/focused_pick_result.log`). WORLD 요청·취소·MAP 전환,
회전/크기/시각 보존, selection/edit/generation 변경의 stale 요청 거절, NaN hit 거절 후 retry를
확인했다. geometry preview의 전달 sink까지만 검사했으므로 실제 Save/화면 재생 증거로 삼지 않는다.

## G00. 범위와 실행 상태

전체 Kouku 저작 목록·관문별 트리, 불뿜기 보스 앵커 미리보기, 애니메이션 preview 지팡이, 트럼펫 레이저·문양 장판과 회전 카드 그룹을 다룬다. 트럼펫 폭발은 사용자가 기존 알비온 네 방향 이펙트로 추가한 항목을 유지한다.

사용자가 Client와 Server를 실행한 채 편집 중이라고 답했다. 공유 EXE/DLL을 교체하는 Product Build와 재시작은 수행하지 않았다. 소스 컴파일·문서 검사·창 없는 모델/리소스 검증은 out의 독립 산출물로 수행한다. Client/UI 실행·조작·화면 캡처와 visual PASS는 수행하지 않는다.

## G01. 전체 저작 목록과 분류

- `Effect_Tool_ResourceBrowser.cpp`: Catalog, 실제 Authored 헤더, Tree 참조를 합친다. 표시 이름·분류 경로를 검색하며 미등록 원본도 canonical Authored 경로에서 연다. 등록된 원본의 기존 identity/provenance 검사는 유지한다.
- `Effect_Tool_Workspace.cpp`: Resources에서 선택한 Kouku Preview도 기존 통합 source-model 재생 준비를 사용한다. dirty 전환의 Save/Discard/Cancel과 실패 시 기존 문서 보존을 유지한다.
- `sync_kouku_effect_tree.py`: 기존 분류를 보존하며 현재 Composition 사용 관계와 원본 Action/Sequence 근거로 빠진 문서를 추가한다. Product admission이나 실행 가능한 gameplay Pattern을 합성하지 않는다.
- 최초 동기화에서 누락 556개와 분류 노드 170개를 추가했다. 동일 입력 재실행은 변경 0개다. 이전 행 순서·내용을 보존했다.
- production `Read_V1Organization`/`Read_V1Inventory` 코드의 native 검사에서 635개, 트리 누락 0개, 헤더 오류 0개, 129ms를 확인했다. 불뿜기의 이름과 `1관문/패턴/세이튼/세이튼_불뿜기(원본0-1)_FullRestore` 경로가 일치한다.

증거: `out/KoukuEffectLibrary20260913/Tree/validation.json`, `out/KoukuTreeInventory20260913/inventory_probe.result.json`. Native probe에서 사용하는 production 코드 구간과 hash는 `inventory-production-spans.json`에 남겼다.

## G02. 실제 보스와 불뿜기 앵커

`EffectAuthoringSequencer.cpp`는 모든 Kouku V1을 sourceModelPreview → 유일한 Composition 사용 pattern → 사용자의 명시적인 Model View pattern 선택 순서로 resolve한다. 분류 트리나 자동 Append 선택을 보스로 해석하지 않는다. 문맥이 없거나 유효하지 않으면 구체적인 선택 경로를 표시하고 기존 재생을 보존한다. 장면 플레이어의 위치·방향은 preview 배치에만 사용한다.

`EffectCompositionModelPreview.cpp`의 격리 projection에도 V1 resource와 occurrence의 최소 stable ID 관계를 보존해 unrelated Composition 항목의 오류 때문에 보스 연결이 사라지지 않도록 했다.

불뿜기 `effect.kouku.gate1.4219801.full.restore`의 실제 설치 MN_RPCT_05 CModel, preScale 0.017, 원본 애니메이션과 MidControl/FX_Prj_01/FX_Prj_02를 사용했다. 57요소, 665개의 60Hz·경계 샘플에서 finite/clip 경계/rewind 검사에 통과했다. peak CPU rows는 114다. 위치·크기·색의 화면 승인은 사용자 확인으로 남긴다.

증거: `out/KoukuTrumpet20260913/CPU/firebreath_actual_model_probe.json`.

## G03. 애니메이션 미리보기 지팡이

`CharacterPreviewPanel.cpp`의 대형 세이튼 망치 한정 분기를 actor가 resolve한 BossCatalog 무기로 확장했다. 세이튼·쿠크세이튼의 기존 WP_MN_RPCT_05 모델·native 재질·preScale/preRotation을 실제 b_wp_1에 부착한다. 기존 대형 세이튼 WP_MN_RPCT_06과 무기 skeleton/clip·bind pose 정책은 유지한다. 실패하면 staged 객체를 rollback한다.

실제 설치 WModel의 body/clip/time 36조합과 지팡이 정점 15,172개를 합성했다. grip basis 길이 오차는 최대 1.61e-6 이하다. 이는 CPU 조립 검사이며 실제 손에 보이는지의 최종 판정은 아니다.

증거: `out/KoukuPreviewStaff20260913/staff_assembly_validation.json`.

## G04. 신규 그룹 통합

다음 세 문서를 설치하고 Catalog·Tree·Composition 전역 리소스와 프로젝트 None에 등록했다. 트리의 `1관문/패턴/세이튼` 아래 해당 기존 패턴 이름으로 구분한다. 리소스는 다른 관문에서도 재사용할 수 있다.

| 이펙트 | Asset ID | 요소·길이 |
|---|---|---|
| 트럼펫_8방향레이저 | `effect.kouku.common.trumpet.radial.lasers` | 40개, 3,350ms |
| 트럼펫_카드문양장판(원본) | `effect.kouku.common.trumpet.suit.floor` | 20개, 2,700ms |
| 세이튼_빙글빙글돌며카드던지기 | `effect.kouku.common.spinning.card.throw` | 224개, 20,460ms |

트럼펫은 Action 4219807의 Sk_06_8 레이저와 Sk_06_9 장판을 분리했다. 원본 레이저 notify의 FRotator는 `[0,0,0]`, `[0,8192,0]`으로 45도 차이다. 네 방향 묶음 두 개로 8방향을 구성했다. 실제 설치 모델의 notify 시작 소켓 위치 `[-0.000136,6.115392,0.452431]m`와 약 1.7 배율을 고정하고, 독립 편집 그룹의 방향은 수평 방사형으로 구성했다. 이 독립 방향 배치는 원본 손 소켓의 전체 회전을 그대로 복제했다는 뜻이 아니다. 장판의 source emitter 크기·지연·수명을 유지했다. Sk_06_2 폭발은 포함하지 않았다. 두 그룹 모두 원본 보스·clip의 sourceModelPreview를 제공하며 사용자 애니메이션 타임라인을 변경하지 않는다.

카드는 Action 4219819/Projectile 421981901의 준비·회전·종료 두 차례를 포함한다. 매 회전 구간에서 0.7/1.0/1.3/1.6초에 세 장씩 발사하며 총 24장이다. 원본 카드 mesh/native material, TypeData 90도와 MeshRotationRate 3turn/s를 보존했다. source missile 속도 15m/s와 8~15m 거리 범위 중 독립 미리보기는 최대 15m의 직선 경로를 선택했다. 대상에 따른 보스 이동·명중 callback·damage를 합성하지 않았다. 전체 24 trail의 운영 예약은 실제 0.4초·60Hz를 수용하는 27점씩, 총 648점이다. 원본 TypeData의 500점 필드와 재질·레시피는 유지한다.

카드의 실제 CModel CPU 재생 1,230샘플, 실제 anchor 2개, peak 631행을 검증했다. 첫 카드 이동은 14.999816~15.000164m/s, 회전은 1079.983~1080.017도/s이며 위치 오차 최대 0.00000407m다. source track의 절대 시각과 startDelay가 중복되지 않는다. 첫 카드는 4.4333초까지 있고 4.45초에는 없어 비행 끝에서 남아 회전하지 않는다. 트럼펫은 레이저 202샘플·peak 50, 장판 163샘플·peak 35를 검사했다. 세 문서 모두 native codec/CPU와 창·draw 없는 WARP Stage_Document에 통과했다. 이는 리소스 준비 검사이며 사용자 화면 승인이 아니다.

설치 당시 Composition revision 428→429, 새 resource 3개 append만 수행했다. 기존 resource prefix와 모든 기존 패턴·애니메이션·폭발·TRS를 보존했다. 실행 중 EXE에 append-only 외부 리소스 병합 경로가 포함된 것을 확인했으며 Reload나 종료를 요구하지 않았다. 설치 뒤 모든 Kouku 저작 문서 638개의 트리 참조가 존재하고 신규 원본 SHA가 검증 후보와 일치한다. 새 binary Resources는 만들지 않았다.

생성기: `Tools/EffectPipeline/build_kouku_trumpet_groups.py`, `build_kouku_spinning_card_groups.py`. 근거는 `out/KoukuTrumpetGroups20260913/installation.json`, `out/KoukuSpinningCards20260913/native_validation_receipt.json`, `out/KoukuTrumpet20260913/CPU/spinning_card_motion_analysis.json`, `out/KoukuEffectLibrary20260913/InstalledGroups/preservation.json`에 남겼다.

## G05. Resources 이름·트리와 World 위치 UI

Pattern/Sequence가 공유하는 `KoukuSaydonActionWorkbench`의 V1 Resources를 All Effects와 같은 EffectResourceTree로 구성했다. owner 9개 옵션은 `EffectAuthoringResourceTree.h`로 옮겨 두 화면이 같은 배열을 소비한다. 기존 옵션 순서·이름을 유지하며 Composition 브라우저의 All/Other로 미분류 항목도 접근할 수 있다. 이름·ID·분류를 검색하고 V1 Element의 실제 원본 선택·Preview/Create/Append를 유지한다.

Created Resources도 원본 분류를 사용하되 Composition의 별칭을 보존한다. 선택한 항목에서 Source/Asset/Category를 확인하고 `Use Source Name`으로 현재 Composition 이름을 원본과 맞추거나 `Locate Source`로 원본 목록을 찾을 수 있다. 새로운 이름 입력은 Composition 이름임을 표시한다. 이는 Effect Detail의 원본 또는 모든 별칭을 자동 수정하는 기능이 아니다.

Effect Box Detail의 Anchor는 Boss/World로 표시한다. World의 Fixed position은 기존 저장 MAP, Follow world object는 기존 저장 WORLD를 소비한다. 빈 WORLD를 고정 위치처럼 표시하던 항목을 없앴다. 고정 World 전환은 실제 player 위치를 우선하고 없으면 해당 Pattern의 정확한 boss spawn을 사용하며 실패하면 기존 입력을 보존한다. 기존 Use Player Position/Use Mouse Position/Focus와 staged preview·Apply·Save 경로를 유지한다. 현재 실행 중 Composition을 일괄 변환하지 않았다.

Workbench 단일 TU 컴파일이 통과했다. 증거: `out/KoukuWorkbenchAnchorResources20260913/Compile/compile.log`. 화면에서의 선택·피킹·이름 변경·저장은 새 EXE에서 사용자가 직접 확인해야 한다.

마지막 독립 검토에서 미등록 V1의 Workbench 원본 child/Preview 연결이 아직 Catalog::Find를 사용한다는 P2를 확인했다. 현재 Kouku 638개 중 Catalog 미등록 522개는 목록에는 노출되지만 이 Workbench 경로에서 요소 확장·미리보기가 실패할 수 있다. All Effects의 직접 원본 Open Editor 경로와 이번에 정식 등록한 새 3개 그룹은 이 미등록 경계와 다르다. 사용자 요청으로 여기서 마무리했으며, authoring 전용 canonical source loader를 실제 preview stage까지 연결하는 작업은 미완료로 남긴다. 이를 해결하려고 모든 원본을 제품 Catalog에 일괄 등록하지 않았다.

## G06. 빌드와 사용자 확인 경계

ResourceBrowser, Workspace, Sequencer, CompositionModelPreview, CharacterPreviewPanel, KoukuSaydonActionWorkbench의 수정 TU를 out에 개별 컴파일했고 모두 통과했다. 공통 owner header 이동 뒤 ResourceBrowser/Workspace를 다시 컴파일했다. 개별 컴파일은 제품 링크 완료가 아니다. 최신 코드를 실행하는 Client EXE는 아직 만들지 않았다.

새 EXE 적용 후 사용자가 `F1 → Effect Tool → KoukuSaydon → 1관문 → 패턴 → 세이튼`에서 원하는 이펙트를 `Open Editor → Play All`로 열고 실제 보스·무기·앵커를 확인한다. sourceModelPreview가 없는 공유 원본은 `Model View → Pattern source → KoukuSaydon Patterns`에서 사용할 보스 패턴을 먼저 선택한다. Current Effect의 요소/그룹 Transform에서 위치·회전·크기를 조절한다.

기존 편집을 저장하고 Client/Server를 종료한 뒤 `Tools/Build/Invoke-BuildAndRegression.ps1 -Profile Product -Configuration Debug`의 정상 증분 빌드를 수행해야 새 코드가 실행파일에 반영된다. 이 종료·빌드 단계와 최종 시각 확인은 아직 남아 있다.

위 소스·데이터 변경은 이후 사용자 커밋 `2e2f62be`와 PR #375에 포함됐다. 아래 G07은 사용자가 요청한 main 병합 충돌 해결과 마지막 편집 보존 결과다. 제품 EXE 재빌드와 사용자 화면 확인 경계는 그대로 남아 있다.


## G07. PR #375 병합 충돌과 마지막 편집 보존

`kouku-pattern3-sequence`의 `2e2f62be`와 main `461224f9`는 같은 기준점에서 별도로 진행됐고 Composition·Sequence·World Sequences의 원본/생성물 4개 경로가 충돌했다. 마지막 push 이후 main에 별도 변경이 들어와 생긴 충돌이며, 사용자의 마지막 push가 누락된 상태는 아니었다.

처음 원래 작업 폴더에서 수행한 병합이 실행 중 도구의 저장 기준본을 바꿔 Save freshness 거절을 유발했다. 그 병합을 abort하고 안전 stash 및 원시 바이트 백업으로 복구했다. 이후 격리 worktree에서만 충돌을 정리했다. 원본/미저장 편집 보호 검사를 제거하거나 도구를 Reload·종료하지 않았다.

- Pattern Composition은 사용자의 마지막 저장 revision 433을 바이트 그대로 보존했다. `쿠크세이튼_무지개댄스` P38의 최신 18개 stage, 다른 47개 패턴과 150개 리소스를 보존했다. main 쪽 차이는 revision뿐임을 3way로 확인했다. native parse·validate·정확한 typed roundtrip이 통과했다.
- Sequence는 같은 stable ID의 폭죽 항목을 중복 생성하지 않았다. main에 추가된 항목과 기존 stage·animation을 유지하면서, 후속 원본 복원 근거와 사용자 편집이 있는 World 고정 앵커, 12,239ms 원본 tail, 24,785ms 박스 및 최신 배치를 선택했다. 113개 내부 참조와 기존 ID 보존 검사가 통과했다.
- World Sequences는 main의 오브젝트 그룹·갈고리/칼날·재질 변경을 유지하고 이 브랜치의 `세이튼_1관문_연출` 오브젝트 별칭과 template 이름을 함께 반영했다. revision은 1737이다. runtime 문서는 canonical publisher의 WorldSequences 범위로 생성하고 `Mode Check` 바이트 일치 검사를 통과했다. 격리 worktree의 긴 Windows 경로는 짧은 junction으로 접근했으며, 리소스 검사를 생략하지 않았다.
- 변경 JSON/XML 22개 parse, source/runtime 내용 일치, unmerged path 0개와 `git diff --cached --check`를 확인했다. 자동 병합 C++ 호출부·프로젝트 등록 검사에서 기존 이펙트 도구/지팡이 변경과 main 기능의 등록·선언이 함께 보존됨을 확인했다.

안전 branch는 `codex/pr375-safety-20260913`, 사용자 최종 편집 안전 stash는 `0039a14c`다. 원시 백업과 의미 비교는 원래 작업 폴더의 `out/PR375Merge20260913/`, native 및 최종 parse 증거는 격리 worktree의 같은 out 경로에 보존했다. 실행 중 Client/Server와 사용자 UI는 조작하지 않았다.

병합된 `MainApp.cpp`와 `WorldObjectTool.cpp`를 새 worktree의 헤더로 Debug x64 개별 컴파일했고 exit 0, error 0으로 통과했다. 기존 헤더 인코딩 C4828 경고는 남았다. 증거는 격리 worktree의 `out/PR375Merge20260913/Compile/compile.log`다. 이 검사는 제품 EXE 링크·재빌드나 화면 확인을 대신하지 않는다.
