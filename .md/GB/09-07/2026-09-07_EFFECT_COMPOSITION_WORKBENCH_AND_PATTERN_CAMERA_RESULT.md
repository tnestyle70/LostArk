# Effect Composition Workbench·패턴 Camera 구현 결과

작성일: 2026-09-07

## 구현 상태

현재 배포 대상은 기존 **F1 Effect Tool**의 panel 재사용, Effect Resources Parent/Effect 트리, Model View와 Effect Sequencer, World Object resource 연결, Patterns by Gate Save다. 별도 큰 Effect Composition Workbench 창은 진입점에서 제거했다. 기존 Play All/Family/Element를 유지하고 Current Effect의 편집 버튼 아래 Append만 추가했다. 새 Glass/Energy/Slash material 및 기존 스킬 재질 변경은 사용자의 범위 축소에 따라 이번 배포에서 제외했다.

사용자 최종 Composition revision95는 Parent8/Bundle8/2관문 자식Pattern6을 포함하고 SHA256 `8856242fed4c524bac4911de73207663e50f8002a0904b47f88938cf0bcc1bbc`를 보존한다. WorldSequence397의 변경은 승인된 룰렛 walkableSurface뿐이다. Effect JSON392개와 기존 skillbinding의 생성/교체/튜닝은 수행하지 않는다. Client/UI 입력과 아레나 시각 판정은 사용자 확인 전이다.

G01~G10은 선행 구현·검증 이력이며 아래 G11이 현재 도구 구성의 정본이다.
## G01. 독립 Effect 문서와 편집 창

- 새 `EffectEditingSession`은 CPU 문서·선택 element·Dirty·저장 기준본을 소유한다. Create 시 한글 표시 이름과 별도의 ASCII stable ID를 발급한다. 기존 V2 leaf/group 파일을 그대로 사용한다.
- Create/Load/Save는 모델이나 live GPU preview를 요구하지 않는다. 필수 문서 구조·ID·path 검사는 유지한다. Save는 변경 문서를 먼저 serialize/parse하고 파일 기준본을 확인한다. leaf를 먼저, group을 마지막에 쓰며 쓰기 실패 시 자신이 쓴 bytes를 확인해 rollback한다. 여러 파일 전체를 OS 수준의 crash-atomic transaction으로 보장한 것은 아니다.
- 기존 Effect Tool의 leaf Load/Save도 로드한 한글 displayName과 원본 bytes를 보존한다. 같은 ID가 변경·삭제됐거나 로드하지 않은 기존 ID이면 Save를 거부한다. 새 ID와 성공 후 반복 Save는 허용한다.
- Save 실패 시 Dirty와 기존 파일을 보존한다. Revert도 임시 session에서 읽기에 성공한 뒤 교체하므로 저장 파일 누락·손상 시 현재 draft가 사라지지 않는다.
- 기존 Effect Tool의 resource domain/폴더/썸네일·slot과 type별 튜닝 UI를 재사용한다. 공유 브라우저를 그릴 때 기존 창의 type/slot/bindings를 보존한다. 새 창의 편집값은 자체 draft에 반영한다.
- 오른쪽 Effect Detail / Model Animation 탭, 아래 Model clip / Effect element 행으로 배치했다. Play/Pause/Restart/Stop/Loop/Seek, element 이동·길이·offset·Solo/Mute를 제공한다. Solo/Mute는 preview 상태이며 저장 enabled를 바꾸지 않는다.
- 목록은 metadata 조회, Play는 선택 closure의 parse·resource stage로 처리한다. 새 목록과 Save, Kouku Effect 실행이 Valtan 전체 graph admission을 기다리지 않는다. 잘못된 항목은 오류로 분리하며 실행 중인 occurrence는 기존 snapshot을 유지한다.

신규 파일은 `Client/Public`, `Client/Private`의 EffectEditingSession, EffectCompositionWorkbench, EffectCompositionModelPreview, EffectCompositionWorldResource H/CPP 8개다. `Client/Default/Client.vcxproj`와 filters에 각각 한 번 등록했다. 새 파일은 UTF-8 BOM 없음이며 기존 파일의 인코딩을 유지했다. 미사용 generic facade API는 제거했다.

## G02. World Object 공과 모델 참고

`World Object` category는 부모의 model/base texture/pre-scale과 자식 Motion을 읽는다. 부모 선택은 resource binding, 자식 선택은 현재 Effect draft에 모션을 가져오는 동작이다. 원본 Object/Motion은 수정하지 않는다.

저장된 `공_튀기기`에서 확인한 값:

| 값 | 저장본 |
|---|---|
| 개별 lifetime | 1,700 ms |
| playback speed | 1.5 |
| velocity | (1, 5, 0) |
| acceleration | (0, -6, 0) |
| 기본 생성 개수 / 간격 | 1 / 200 ms |
| native model clip | 없음 |

이 모션은 기존 Mesh Particle로 가져온다. 1회 burst, 1 ms emitter, 1.7초의 개별 입자 수명과 1.5배 재생을 구분한다. 실시간 입자 수명은 약 1.1333초다. Saved Effects의 Particle/Trail leaf를 Append할 때도 playRate 환산과 DEACTIVATE/잔여 lifetime을 유지한다. Detail에서 Emission count / Interval / Max Particles와 초기 rotation 범위, spin 범위를 조절할 수 있다. `Random mesh rotation 0..360`, `Random mesh spin` 버튼도 추가했다.

Model Animation은 저장된 Gate → Folder → Bundle → member와 독립 Pattern을 읽고 기존 순서의 clip을 actor별 Sequencer 행에 표시한다. 묶음 member offset은 기존 30Hz tick 올림과 같은 시각을 사용한다. 빈 DRAFT와 오류는 해당 항목에 표시한다. Clip 편집 정본은 Action Workbench이며 여기서 별도 clip 목록을 저장하지 않는다.

모델 참고는 기존 CNpc/CModel preview actor를 재사용한다. 참고 재생은 root motion과 WORLD gameplay를 실행하지 않는 고정 root다. 실제 보스 이동을 비교하려면 저장 Effect를 Action Workbench에 연결해 Server Complete Play를 사용한다. Product 입자는 보스 root를 birth 시각에 샘플링하고 생성된 world-space 입자는 이후 보스에 끌려가지 않는다.

고정 60Hz simulation과 외부 master cursor의 이중 진행을 막았다. Deactivate는 step 잔여 구간의 마지막 방출을 처리하고 emitter 종료 후 개별 입자 수명을 유지한다. Rewind는 같은 seed와 기록된 root 표본으로 다시 계산한다. 처음 중간 시점에서 모델 참고 Play를 할 때는 고정 root의 알려진 0초 pose를 기록한다. 실제 Server 과거 위치를 만들어 넣지는 않는다.

## G03. Camera와 가짜 세이튼 Light

Action Workbench의 Kouku Camera resource에 이름 기반 Create Camera, 현재 eye/lookAt/FOV의 `Set Camera Pos / Capture view`, Blend in ms / Default hold ms / Return ms / LINEAR·SMOOTHSTEP, Save Camera를 연결했다. 저장된 Camera resource를 Pattern 또는 Bundle에 Append한다. 별도 새 런타임 경로 대신 기존 Camera controller와 Engine override를 확장했다.

Camera는 재생 순간의 view에서 목표 pose로 이동하고, box의 끝에서 현재 이동 중인 player follow pose로 복귀한다. 출발/복귀의 고정 world 좌표 세 개를 저장하지 않는다. Camera box duration은 진입+유지이고 복귀 tail은 별도로 충돌 검사한다. Follow 모드에서는 gameplay 입력을 계속 받는다. 기존 Free Camera의 입력 차단 계약은 유지한다.

저작 Preview는 Save한 camera-shot source를 즉시 읽고, Product Complete Play는 Map publisher의 runtime shot을 새 run에서 읽는다. 새 `patternOnly` shot은 Area 자동 trigger와 구분한다. 기존 Valtan shot의 시간 제한을 임의로 넓히지 않았다.

진짜 세이튼 찾기의 BOSS-follow Spot Light는 살아 있는 같은 archetype·같은 owner boss의 가짜 모델에도 동일 asset/offset/timing으로 적용한다. 같은 Light를 직접 재생하는 가짜는 중복 생성하지 않는다. despawn·box 종료 시 follower handle을 정리한다. ClientReplication이 Server owner boss ID를 보존하도록 연결했다. 저장된 Light 밝기·범위·cone·위치를 바꾸지는 않았다.

## 자동 검증과 배포

| 확인 | 결과 |
|---|---|
| Client 최소 Debug compile/link | 통과 |
| Debug Product (Engine → Shared → Server → Client) | 통과, 마지막 legacy leaf 저장 호환성·Particle Append 수정까지 포함. SDK/shader/runtime DLL 배포 포함 |
| Effect binding / occurrence / catalog tests | 22 / 8 / 16 통과 |
| MainApp entry / resource facade tests | 10 / 11 통과 |
| Camera source contract | 통과 |
| Camera overlap·generic presentation·잘못된 공통 Camera 3개 focused tests | 통과 |
| 변경 JSON parse | 13개 통과 |
| Client project/filter XML parse·신규 8개 등록 | 통과 |
| 원본 저장본 5개 SHA256 비교 | 모두 보존 |
| git diff --check | 통과 |
| Client 입력·저장·아레나 화면 | 사용자 확인 전, 자동 PASS로 기록하지 않음 |

Map publisher를 `-AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish`로 실행해 해당 Area 7개 출력(3,231 placements 포함)을 배포하고 Check를 통과했다. Camera revision 62 / World revision 396이다. 해당 저작 폴더 6개 파일의 hash도 변경되지 않았다.

Kouku domain은 필요한 `koukusaydon.product`만 공식 domain runner로 갱신했다. PatternBindings의 Light 참조 revision 2를 최신 5로 바꾸고 Composition revision 87을 유지했다. Encounter bytes와 Server Product revision 87은 이미 최신이므로 공통 gameplay.balance를 재배포하지 않았다. Rendering Light 5 / Profile 18은 source/runtime deep-equal과 publisher Validate가 통과해 불필요한 Publish를 생략했다.

최종 Product receipt는 `out/BuildPipeline/runs/20260907T094323697Z-debug-product.json`이다. 검증 출력은 `out/EffectCompositionWorkbench/`에 있으며 `source-preservation-check.json`, `runtime-implementation-review.json`, `particle-tail-numerical-review.json`, Map publish/check 로그와 Product 빌드 로그를 남겼다. 수명 수치 검사는 float 누적 경계 계산이며 Client 실행 결과가 아니다. 기존 MainApp source contract의 과거 함수명·고정 tool 개수 assertion은 현재 실제 호출 경로 검사로 갱신했다. source contract와 Python projector test는 실제 GPU/UI 입력 검사를 대신하지 않는다.

## 사용자 재생 경로

1. Visual Studio의 Server + Client profile을 Ctrl+F5로 실행하고 Lobby에서 KoukuSaydon에 입장한다.
2. F1 → Effect Tool → Effect Resources에서 V1/V2 아래 Parent 선택 → 이름 입력 → Create Effect. 저장된 Effect 행을 선택하면 기존 Current Effect에 열린다.
3. Current Effect의 Play All/Family/Element로 미리 보고, Append를 누르면 Effect Sequencer의 현재 시간에 추가된다. Model View에서 모델 선택 → Reload saved ordered clips 또는 KoukuSaydon Patterns에서 저장 Pattern/Bundle 선택 → Sequencer Play로 함께 확인한다.
4. World Object 리소스는 저장 model/texture를 사용한다. V2 World Object Resource에서 공_튀기기 Motion을 선택해 CPU draft로 가져오고 기존 Effect Detail로 튜닝한다. Effect Save와 Sequence Save는 서로 다른 저장 단위다.
5. Action Workbench의 Patterns by Gate 바로 위 Save는 Parent/Bundle을 포함한 전체 Composition을 저장하며 Saved/Unsaved를 표시한다. 자동 저장이 아니다.
6. Camera와 fake Spotlight는 기존 G03 경로를 유지한다. 후속 source149에서 룰렛 P7은 PRODUCT로 설정됐다. 최신 보행면 서버 검증과 남은 Product/아레나 확인은 Gate Pattern Bundle RESULT G22를 따른다.

## 남은 경계

- 공 끝의 터짐·파편 Effect와 창술사 유리 파편의 원작 시각 복원은 아직 저작하지 않았다. 이번 작업은 그것을 제작할 도구와 기존 runtime 연결이다.
- World Object native 모션은 단일 clip 적용이 기본이며 여러 native clip의 순차 재생·복잡한 World transform key·orbit·다음 Motion 자동 전환 전체를 Effect로 복제하지 않는다. 지원되지 않는 변환은 가져오기 상태에 표시한다. 공의 저장된 velocity/acceleration은 지원한다.
- V1/V2는 같은 Effect Tool 안에서 원래 codec을 사용한다. 기존 nested V2 group의 전체 편집 확대와 사용처 역조회는 이번 범위 밖이다.
- root 이동 접선 방향의 별도 orientation mode와 작은 순간이동 전체 검출은 구현하지 않았다. 현재 rotation은 actor root 회전이고 caller는 50m 초과 jump를 discontinuity로 표시한다. 표본이 없는 late join/과거 WORLD·bone emission은 해당 Effect 오류로 격리한다.
- 실제 render pass의 시각 품질, 공의 수량·회전·길 위치, Camera 보간·입력, 가짜 Light 수와 모습은 사용자 아레나 확인이 남아 있다.
- 대규모 공유 dirty worktree의 기존 변경을 보존했으며 자동 stage/commit/push하지 않았다.

## G10. 사용자 최초 Open assertion 후속 수정

사용자가 `Open Effect Composition Workbench` 직후 `Cannot have an empty ID at the root of a window` assertion을 보고했다. 빈 session의 group 표시 이름과 ID가 모두 비었는데 `Render_Timeline`이 부모 `Selectable`을 그린 것이 원인이다. ImGui `ItemAdd`의 root ID 비교 조건과 해당 호출을 소스로 확인했다.

- 빈 draft의 Timeline은 생성/열기 안내만 표시하고 반환한다. 생성 전과 unsaved Revert 후 모두 적용된다.
- 정상 부모 항목은 표시 이름과 별개인 `###EffectCompositionGroup` 위젯 ID를 사용한다.
- 삭제된 Pattern을 참조하는 모델에서 actor 표시 이름이 비는 경우도 있어 Character anchor는 member ID scope와 `###AnchorMember`를 사용한다.
- 이번 변경은 Workbench CPP와 관련 PLAN/RESULT/gotchas뿐이며 Light·Effect·Object/Motion 저작 데이터를 수정하지 않았다.
- Client Debug compile/link 및 기존 SDK/shader/runtime DLL 배포 통과. 로그: `out/EffectCompositionWorkbench/empty-id-client-build.log`. Object 파일 생성 시각이 Character anchor 추가 수정 이후임도 확인해 두 수정의 빌드 포함을 확인했다.
- 기존 MainApp/Effect focused contract 10개와 `git diff --check` 통과. 새 JSON/XML 변경 없음.
- 사용자 최초 Open은 assertion 실패 관찰이며, 수정 후 실제 창 재열기 확인은 아직 사용자 대기다. 에이전트 Client/UI 실행·조작·캡처 없음.

룰렛 보행 높이는 별도로 읽기 전용 조사했다. 현재 WORLD 시퀀스는 Map placement40을 시각적으로 재생하고 Server Navigation은 기존 셀 높이를 소비한다. template은 Y축 회전·마지막 숨김만 가지며 Composition의 룰렛 offset Y는 +0.58이다. 원형 보행면 정의와 동일 WORLD cue 수명의 Server 활성화, 정지 플레이어 높이 재계산, 경로/실제 이동의 공통 높이 조회가 필요한 상태다. 이번 assertion 수정에 Navigation 구현·bake·publish를 섞지 않았다.


## G11. 기존 panel로 재구성한 실제 구현

- `EffectAuthoringResourceTree`는 V1/V2 saved metadata와 사용자 Parent/category reference만 읽고 저장한다. tree 조회에 GPU나 전체 boss validation을 붙이지 않는다. 하나의 손상 항목 때문에 정상 목록을 지우지 않는다.
- `Effect_Tool_Workspace`는 기존 V1 Current Effect/Detail/slot 바인딩과 V2 CPU pane를 선택 owner에 맞춰 연결한다. 저장 leaf는 같은 native ID/파일을 유지하고 명시적인 group 확장만 별도 ID를 만든다. Effect Save 성공과 optional tree 저장 실패를 구분한다.
- `EffectAuthoringSequencer`는 Character skillbindings/Valtan Product/Kouku saved Pattern의 clip 행과 복수 V1/V2 occurrence를 같은 시간으로 샘플링한다. Preview는 임시행이고 Append만 저장행을 만든다. 숨김·owner 변경·level 종료에서 occurrence와 borrowed model을 정리한다. V1 Character source bone은 기존 source-anchor resolver와 고정 시점 기록을 소비한다. Kouku의 root-only adapter에 source bone을 추측해서 붙이지 않는다.
- Model clip 순서는 원본 읽기 전용이다. 별도 `Data/Effects/Sequences/<id>.effectsequence.json`은 stable source 참조와 occurrence 시간·offset만 저장한다.
- Pattern Save 버튼, World source 카테고리의 기존 domain 검사, 복수 draft/selected resource panel의 ImGui ID, typed owner focus와 잘못된 draft의 saved fallback을 수정했다.

최종 컴파일·보존 검증 결과는 아래 완료 증거에 기록한다. 실제 UI 입력·저장·재실행·아레나 재생은 사용자 확인 대상으로 남긴다.

## G14. 기존 스킬 재질 읽기 전용 조사

사용자가 범위를 툴 확장으로 한정한 뒤 기존 Effect의 셰이더·C++ packet·JSON·animation binding은 변경하지 않았다. 차원술사 Q/W/E/F의 현재 source family와 Glasshole translated 식의 변수 대응을 PLAN G14에 기록했다. 미소비 scalar가 다른 원본 texture lane, static permutation 또는 view vector를 요구하는 경우를 구분했다. 계산 의미가 확인된 edge-crack desaturation도 기존 Effect 반영 범위가 아니므로 적용하지 않았다.

현재 문서와 resource 파일 존재를 확인한 결과는 `out/EffectCompositionWorkbench/source-family-current-audit.json`에 있다. 조사한 11개 Effect 문서의 참조 asset 누락은 0건이며 화면 확인은 수행하지 않았다. 제거된 원본 receipt는 Git `4e013459^`에서 읽어 `glasshole-historical-equation-evidence.json`으로만 보관했고 Data에 복원하지 않았다. 과거 문서의 워로드 F56행·창술사 D/F74/168행은 현재 사용자 저장본 4/2/3행과 다르므로 자동 복원 근거로 사용하지 않는다. 도화가 F의 현재 17행 실행 구성은 보존했다.

## 최종 완료 증거 — 패턴 제작 우선 마감

- 최종 Debug Product 빌드 성공, Engine/Shared/Server/Client 링크와 정상 SDK·shader·runtime DLL 배포 완료. receipt `out/BuildPipeline/runs/20260907T114256824Z-debug-product.json`, log `out/EffectCompositionWorkbench/final-pattern-tools-product-build.log`.
- 새 Effect Resources/Sequencer/V2Pane/Workspace와 V1 source-anchor callback까지 컴파일에 포함했다. 추가 Material Model 변경은 빌드 전에 제거했으며 새 HLSL은 생성하지 않았다.
- Nav native 집중 검증 11개 PASS(실패0), Python 및 실제 publisher 함수 사례 5개 PASS. 해당 source397과 runtime WorldSequence JSON deep-equal. 룰렛 DRAFT 상태는 그대로 유지했다.
- Effect binding 기존 테스트22개 PASS(Material 전용 변경/테스트 제거 후). Client project/filter XML2개 parse, git diff --check 통과.
- Data/Effects JSON392개 SHA256 및 Composition95 SHA256 전부 보존. 2관문 Parent8·Bundle8, 전체 참조 확인. `세이튼등장`2명/클립4개, `파1빨2`·`조커찾기`각2명/클립0개, 나머지5개Parent/Bundle은 member0 상태까지 실제 파일과 대조했다. 원본 백업 `out/EffectCompositionWorkbench/pre-rebuild-user-saved-composition-r95.json`.
- 보존 검사 JSON: `out/EffectCompositionWorkbench/final-pattern-tools-preservation.json`. 실제 Effect 생성·스킬 binding·기존 W shader 변경 없음. 미완성 Material 작업은 `out/EffectCompositionWorkbench/deferred-material-20260907/`에 후속 참고로만 보관했다.
- Client/UI 자율 실행·화면 캡처·visual PASS 없음. 사용자가 F1 Effect Tool과 Action Workbench에서 입력·저장·재생을 확인한다. 종료 확인 뒤 EXE만 배포했으며 Client/Server는 자동 실행하지 않았다.

## G17. 저장 GROUP Append 연결 — 2026-09-08

사용자는 `boss.kouku.ball.smoke` GROUP Preview는 정상이며 Append가 보이지 않는다고 보고했다.
실측 결과 통합 Effect Sequencer는 이미 GROUP stable ID를 단일 occurrence로 소비했다. Saved Effects
tree는 PREVIEW/APPEND command와 consumer가 있어도 버튼이 없었고, Current Effect의 Append는 긴
child 목록 아래에 있었다. 쿠크 Action Workbench의 source 직접 Append는 Light/Camera에만 표시됐다.

구현 상태:

- Saved Effects tree에 선택 리소스의 `Preview`와 `Append`를 연결했다. V1/V2 LEAF/GROUP의 owner key를 유지한다.
- V2 Current Effect의 Append를 child 목록 위로 옮겼고, tree command도 현재 선택 draft의 native 수명을 사용한다.
- 쿠크 `Resources > Effect` source에 `Append Effect at Cursor`를 표시한다. 선택한 child Pattern의 cursor에
  하나의 GROUP/LEAF resource reference를 추가한다. bundle/빈 Pattern에는 actor lane 추가를 허용하지 않는다.
- 새 resource 등록과 occurrence 생성은 같은 candidate를 검증한 뒤 한 번 commit한다. 실패하면 기존 draft와
  resource ordinal을 보존한다. 기존 typed asset reference를 찾으면 저장된 resource 설정을 재사용한다.
- Effect runtime, Group/leaf 원본, Composition 저작 데이터는 이 UI 수정에서 변경하지 않았다. 원본 Group을
  leaf로 펼쳐 Pattern에 복제하는 경로도 추가하지 않았다. 관련 17 GROUP 사용처는 PLAN G17-1에 대조했다.

자동 검증과 수동 검증:

- 변경 구간 diff와 기존 actual caller를 대조했다. source→typed key→기존 V2 snapshot/Group runtime 연결을 확인했다.
- 변경 소스 5개와 PLAN/RESULT의 `git diff --check`가 통과했다. 모든 변경 H/CPP가 기존 Client project/filter에 등록되어 있음을 확인했다.
- 독립 diff 검토에서 확인한 범위의 correctness/transaction 결함은 없었다. 전체 caller 검토는 다른 구현의
  agent slot 확보 요청으로 중단했으며 이를 전체 검토 PASS로 기록하지 않는다.
- 최소 컴파일은 전체 기능의 통합 Product 빌드 담당자가 수행한다. 이 하위 작업에서 build/publish/Client 실행은 하지 않았다.
- UI 버튼 클릭, Save/Reload 및 아레나 시각 확인은 사용자 대기다.

## G18. WORLD 정상 완료 신호 — 2026-09-08

최종 공 10개의 마지막 smoke는 7881ms까지 필요하여 7400ms의 actor 완료 후에도 481ms가 남는다.
기존 owner Stop이 이 재생을 자르지 않도록 정상 완료와 취소의 Shared/Server 경계를 분리했다.

- `WORLD_SEQUENCE_OPERATION`은 기존 0~3을 보존하고 `FINISH_OWNER = 4`를 추가했다. packet field
  layout은 유지하며 FINISH도 STOP과 같은 exact run/member identity와 transform 거절 검사를 소비한다.
  Read는 같은 Write 검증을 통과한 뒤에만 destination을 변경한다.
- protocol은 69다. 68 peer는 새 operation을 읽을 수 없어 Server/Client를 함께 갱신·재시작해야 한다.
  기존 packet type 값과 source-pinning 계약은 그대로 유지한다.
- Server `Stop_KoukuWorldOwner(memberId, finished)`의 기본값은 false다. 정상 member 완료와
  `Clear_KoukuSaydonPatternAudition(completed)`만 FINISH를 보내고 취소·실패·재시작은 STOP을 보낸다.
  Server의 완료된 replay/support ownership 정리는 기존대로 유지한다. Client tail 소비자는 통합 담당자가 연결한다.
- 기존 NetworkProtocolHarness에 5종 operation prefix/member round trip, whole-run STOP/FINISH
  round trip, epoch 없는 owner 신호와 placement를 가진 FINISH 거절을 추가했다. 현재 protocol 기대값도 69로 맞췄다.
- source diff와 `git diff --check`는 통과했다. JSON/XML 변경은 없고 새 project 등록도 없다.
  이 하위 작업은 빌드·하네스 실행·publish·Client 실행을 하지 않았다. 통합 시 기존
  `NetworkProtocolHarness --kouku-bundle-only`가 새 사례를 전부 포함한다. 실행 결과와 실제 재생은 별도 기록한다.

## G19. 2026-09-08 쿠크 공·V2 연결 최종 통합 결과

Composition source revision133, WorldSequence source revision401을 사용한다. 사용자가 저장한 최신 revision130의 stage·Logic·기존 occurrence를 보존하고 다음 연결만 추가했다.

| 연결 | 최종 데이터 |
|---|---|
| 쿠크 본체 | PATTERN_8 bossMotion, (2.04,10.56,316.95)→(11.79,10.56,326.79), 1870~5780ms, yaw314.7368, 도착점 유지 |
| 높이 | Server baseY10.56. 기존 CNpc root 수평 억제 유지, sourceZ→worldY animation pose 유지. 추가 포물선 없음 |
| 공 발생 | 기존 world.6 / PATTERN_8.world.1, start1481/duration4933 보존. ball_bounce, BOSS_SPAWN, occurrenceSpeed2/3 × 원본 instanceSpeed1.5 = effective1 |
| 공 개수·궤적 | count10, interval300ms, spread360° 수평. 기존 velocity(1,5,0), acceleration(0,-6,0), lifetime1700ms 보존 |
| 폭발 | 각 공 Motion End에 boss.kouku.ball.smoke GROUP 2000ms. Group 원본 offset을 Effect row (0,-2.3,-1.35)로 보정 |
| 댄스 장판·박수 | PATTERN_6 dance 0~26800ms, clap23300~25300ms |
| 세이튼 찾기 공통 구체 | PATTERN_2 find.core3900~23967ms, PATTERN_5 find.core1833~23833ms. 진짜·가짜 양쪽에 연결 |
| 파랑·빨강 | 현재 저작된 PATTERN_11 attack stage에 medusa.blue0~4000ms, medusa.red6667~10667ms. DRAFT 상태와 gameplay 판정은 그대로 |
| 기존 연결 | card.{clober,dia,heart,spade}.{red,black} 8종은 Server snapshot 문양·색별 플레이어 표현을 그대로 사용. disarm의 튜닝된21개 leaf와 find.heart/star는 중복 추가하지 않음 |

첫 공1481ms, 열 번째4181ms, 마지막 공 이동 종료5881ms, 마지막 smoke 종료7881ms다. 공 출생마다 패턴 시각으로 계산한 world matrix를 고정한다. 플레이어 수와 무관하게 한 번씩 생성하고 이후 쿠크·플레이어를 따라가지 않는다. body와 end Effect는 같은 궤적 sampler·출생 matrix를 사용한다.

Pattern Play의 bossMotion은 기존 Bundle preview player를 임시 한 member로 재사용한다. Play Bundle은 로컬 저작 preview이며 실제 Server Play는 별도 audition command다. 둘은 같은 선형 구간과 시작/도착 유지 계약을 사용한다. P8/P9와 세이튼등장_동시 bundle1을 PRODUCT로 승격하고 publisher 계약에 맞춰 playAllPatternIds 순서도 동기화했다.

자연 완료는 FINISH_OWNER(protocol69)로 새 발생을 닫고 남은 body/tail을 보존한다. Stop·실패는 즉시 정리한다. 새 run epoch가 오면 이전 공·Effect·대기 cue를 정리한다. 초기 입장이나 Product 준비 순서 때문에 수신 cue를 잃지 않도록 최대1024개를 유효 수명 동안 보존하며 sourceRevision이 맞은 뒤 시작한다. Product의 worldEmissionAnchors는 occurrence 시각과 offset을 제공하고 편집 중인 source를 매 프레임 다시 읽지 않는다.

Kouku publisher의 엄격한 JSON 검사에서 기존 MapCatalog의 Character Select 영역에 sourceLights/lights가 각각 두 번 존재함을 발견했다. 완전히 동일한 앞쪽 두 속성만 제거했고 전체 유효 JSON 값이 수정 전과 동일함을 확인했다.

| 실제 실행 검증 | 결과 |
|---|---|
| Shared / Server / Client 최소 ClCompile | PASS. 최초 Client scope 오류 교정 후 재컴파일 PASS |
| Debug Product Engine→Shared→Server→Client | PASS. compile/link, SDK·shader·DLL 정상 배포 |
| Map / Composition / GameplayBalance / World publish | PASS. Composition revision133, Source/Runtime WorldSequence 의미 동일 |
| Server --kouku-bundle-contract-test | PASS, failures0. 이동 시작/도착 유지, 30Hz 경계, tick wrap, navigation 거부·보존, Bundle/Restart/Stop 포함 |
| NetworkProtocolHarness --kouku-bundle-only | PASS. FINISH/STOP owner roundtrip, 유효성 거부, protocol69 |
| 새 bossMotion projector focused2개 | PASS |
| 기존 Object publisher fixture + V2 Group 자연 수명 검사 | PASS,2 tests |
| 변경 JSON strict parse / 변경 XML parse | PASS |
| 공10개 발생·마지막 Effect 시각·published motion/anchor 일치 | PASS, final-schedule.json |
| Client 화면·Append/Save/Reload 실제 UI 입력 | 사용자 확인 대기. 에이전트 실행·조작·캡처 없음 |

최종 빌드 증거는 out/BuildPipeline/runs/20260908T025058075Z-debug-product.json이다. 상세 로그·수치는 out/KoukuPatternEffects20260908에 있다. 기존 C4819/PDB/셰이더 경고는 남으며 최종 컴파일·링크 오류는0이다. git diff --check는 이 작업의 파일에서 PASS이고 작업 전부터 있던 ArenaCameraProfile.cpp:53의 탭만 전체 검사에서 별도로 남는다. 해당 사용자 줄은 수정하지 않았다.

사용자는 새 Server + Client profile을 Ctrl+F5로 실행한다. endpoint는192.168.0.14:7777이다. KoukuSaydon Arena에서 Composition의 세이튼등장_동시를 골라 Play Bundle 또는 서버 재생 명령으로 확인한다. Object Tool → 공 → 공_튀기기 → Effect Rows에 끝 smoke가 보인다. 다른 Group은 Object Resources → V2 Effects에서 고르고 Append Effect at Motion End를 사용한다. Effect Tool의 Saved Effects / Current Effect 상단과 쿠크 Resources → Effect에도 Append가 있다.

사용자 최종 확인은 화면상9시 방향, 원본 상승 높이, 공 발사 위치·속도·smoke 크기/정렬이다. 최종 visual PASS를 대신 기록하지 않는다. 새 Resources binary, vcxproj/filters 소스 등록, Drive 전달 대상은 없으며 Git stage/commit/push는 수행하지 않았다.

## G20. 2026-09-08 Cinematic Camera Tool의 쿠크 Area 저장 연결

기존 Cinematic Camera Tool에 `Source: Kouku Area`를 연결했다. 쿠크 Arena에서 열면 자동 선택하며,
미저장 draft가 있으면 source 전환과 Valtan cue deep link를 거절해 현재 편집을 보존한다.
Valtan source·cue 저장·Complete Play 호출은 기존 경로를 유지한다. 쿠크 모드에서는 Valtan 전용
Complete Play 버튼을 숨기고 Save 뒤 Kouku Action Workbench의 Play Bundle을 사용하도록 안내한다.

- Cut List는 Area의 PATTERN_ONLY shot만 표시하고 AUTO/마리오 shot은 기존 MapTool 소유로 남긴다.
  한글 displayName을 New Cut과 기존 Cut에서 편집할 수 있으며 참조 shotId는 이름 변경으로 바뀌지 않는다.
- Capture Pos, keyframe 편집과 Start는 기존 cue editor와 Sample_Cue/Engine camera override를 재사용한다.
  keyframe은 기존 shot.cameraTrack에 저장한다. 새 Camera runtime·파일·project/filter 등록은 없다.
- Level의 CameraShot_ToCue, Stage_PatternCameraTracks, Save_CameraShotSource가 기존 Area parser와
  MapTool 원자 저장 경계를 연결한다. 전체 candidate를 먼저 검사한 뒤 disk baseline CAS가 성공해야
  source와 Arena 저작 cache를 교체한다. Action Workbench의 미저장 Camera draft도 저장 충돌로 거절한다.
- 변경하지 않은 shot과 기존 defaultHoldMs/transitionEasing/activation을 보존한다. 이름이나 blend만
  바꾸면 기존 static/follow/track pose를 유지하며 실제 keyframe 변경 때만 cameraTrack으로 저장한다.
- 쿠크 진입·복귀는 Area의 0..10000ms 계약을 사용하므로 요청된 1200ms가 보존된다. Valtan의
  Stage_CameraDraft/MAX1000 경로는 쿠크 저장과 preview 검증에서 호출하지 않는다.

사용자 입력 경로는 F1 → Camera Tool → Source Kouku Area → 카메라_2관문_세이튼 등장이다.
Capture Pos는 현재 pose를 추가하고 Pos 선택·keyframe editor에서 각 장면을 조정한다. Save는 Area
authoring을 저장하며, 실제 bundle 연결·진입 1200ms·7400ms 뒤 Follow 복귀는 통합 데이터와 기존
Kouku Action Workbench → 세이튼등장_동시 → Play Bundle 경로에서 확인한다. 전체 카메라 복귀를
볼 때는 F6 Follow 상태에서 재생한다. Camera Tool의 Start는 저작된 path preview다.

자동 검증은 기존 `test_valtan_camera_tool_contract.py` PASS와 변경 소스/문서의 scoped
`git diff --check` PASS다. 기존 H/CPP의 UTF-8 인코딩과 CRLF 줄바꿈을 유지했다. 이 하위 작업은
JSON/XML을 변경하지 않았고 native 컴파일·publisher·Client 실행·UI 저장 입력은 수행하지 않았다.
최소 컴파일과 source/Composition publish는 통합 담당자가 실행하고 실제 Capture/Save/Reload,
카메라 구도와 Follow 복귀의 화면 확인은 사용자에게 남긴다.

## G21. 2026-09-08 사용자 확인 뒤 등장 높이·공 생성 시각·공통 카메라 통합

사용자가 이전 EXE에서 공 생성과 끝 smoke 재생을 확인했고, 상승량이 높고 마지막 공이 도착점에
못 미친다고 보고했다. 아래 변경은 그 후속 조정이다. 새 높이와 카메라 구도의 육안 승인은 아직 아니다.

| 항목 | 이전 | 현재 저장·배포값 |
|---|---|---|
| 쿠크 root 최대 상승 | 17.846225m | 14.276980m, animationRootVerticalScale=0.8 |
| 기준 Y / 최고 root Y | 10.56 / 28.406225 | 10.56 / 24.836980 |
| 수평 이동 / 도착 유지 | 1870~5780ms | 동일, (11.79,10.56,326.79)에 유지 |
| 원본 clip 수직 복귀 | 약 6100ms | 동일; 수평 도착 시각과 구분 |
| 공 생성 | 1481~4181ms | 3080~5780ms, 10개·300ms 간격 유지 |
| 마지막 공 생성점 | (7.802724,10.56,322.765918) | (11.79,10.56,326.79), 이전보다 경로상 5.664945m 전진 |
| 마지막 공 smoke | 5881~7881ms | 7480~9480ms, 정상 WORLD tail 보존 |
| 등장 카메라 | bundle 공통 Camera 없음 | 0~1200 진입, 1200~7400 유지, 7400~8600 현재 Follow로 복귀 |

Composition revision134→135에서 Pattern8의 높이 배율과 WORLD 시간, resource36 CAMERA와 bundle1
presentation.1만 변경했다. 조커찾기 무기 앵커를 포함한 다른 모든 Pattern은134와 의미상 동일하다.
Area camera revision68→69에 camera.kouku.pattern.1만 추가했고 기존 AUTO/마리오 shot은 전부 보존했다.
표시명은 카메라_2관문_세이튼 등장이다. 초기 eye=(-11.085,40.56,339.87),
lookAt=(6.915,15.56,321.87), FOV60의 고정 두 key를 등록했다. 현재 view에서 이 pose로 진입하며
카메라의 최종 구도는 사용자가 Capture Pos와 keyframe editor에서 조정한다.

검증 증거는 out/KoukuIntroTuning20260908 및 out/KoukuBossMotion/root-vertical-scale-tests.log에 있다.

| 검증 | 실제 결과 |
|---|---|
| 최종 Debug Product 컴파일·링크·SDK/DLL 배포 | PASS, out/BuildPipeline/runs/20260908T034213853Z-debug-product.json |
| Map Area publish / Composition135 publish / GameplayBalance publish | PASS |
| 실물 WModel root 최고 높이·6100ms 복귀·배율 cache 격리 / action 투영 / 기존 hammer bake | Python 3개 PASS |
| Camera return tail 충돌 / bundle offset·reference / malformed common Camera | Python 3개 PASS |
| 기존 Valtan Camera Tool 계약 | PASS |
| Server --kouku-bundle-contract-test | failures0 |
| native Composition editor Save/Reload·실패 보존 | Debug build와 --kouku-composition-editor-contract exit0. 높이0/0.8/1 저장·재로드 및 음수/1초과/NaN/무한대 거절·기존 상태 보존 포함 |
| strict JSON5·XML3 parse / source135와 배포본 일치 / 기존 Pattern·shot 보존 | PASS |
| scoped git diff --check | PASS; 작업 전 ArenaCameraProfile.cpp:53의 whitespace만 제외 |

Camera tail 검사에서 기존 합성 bundle fixture가 실제 새 clip으로 stage를 교체하면서 예전 bossMotion과
WORLD 구간을 함께 남겨 검사 목적과 무관한 범위 오류가 먼저 났다. 그 fixture에서 기존 lane과 motion,
공통 Camera/Scene을 비우고 합성 시나리오를 구성하도록 수정했다. 실제 runtime 검증은 완화하지 않았다.

실행은 Server + Client profile을 Ctrl+F5로 시작한다. endpoint는192.168.0.14:7777이고 이 작업에서
Client/UI를 실행하거나 캡처하지 않았다. F1 → Camera Tool → Source Kouku Area에서 이름을 선택해
Capture Pos/Save를 사용한다. 전체 연출은 F6 Follow 상태의 Kouku Action Workbench에서
세이튼등장_동시 → Play Bundle로 확인한다. Camera Tool Start는 컷 경로 미리보기다.
사용자 후속 확인은 새 높이, 마지막 생성점, 카메라 framing 및 현재 플레이어로의 복귀다.
새 Resources binary/Drive 전달물, C++ 파일/project/filter 등록, Git stage/commit/push는 없다.

최종 파일 시각은 Client.exe 12:39:08, Server.exe 12:37:33, 배포 Engine.dll 12:42:12 KST다.
마지막 Model.cpp 수정 12:40:42 이후 Engine을 다시 컴파일·배포한 증거가 위 최종 Product receipt다.
종료 확인에서 Client/Server 프로세스와 TCP7777 listener는 없으며 사용자 Ctrl+F5 실행 대기 상태다.
기존 인코딩/PDB 경고는 남지만 컴파일·링크 오류는0이다. 팀 사용서의 WORLD 수명 설명도 실제
protocol69·FINISH_OWNER tail 보존 계약에 맞췄다.

## G22. 2026-09-08 사용자 P1/P2 목표 높이 배포와 입력 경로 확인

사용자가 Camera Tool에 새 Pos를 추가한 뒤 Save한 Area revision70을 적용했다. 최초 확인 시에는
새 Pos가 미저장 상태여서 이전69만 보였으며, 사용자 Save 뒤 camera.scene.auto.1/auto.2를 확인했다.
P1 Eye=(-3.0489308834,25.3147125244,331.9632873535),
P2 Eye=(-4.0211238861,25.3114490509,331.9855651855)와 사용자의 LookAt/FOV60을 그대로 보존했다.
기존 초기 key.1/key.2는 사용자 저장에서 이미 교체됐으므로 에이전트가 추가 삭제하거나 재작성하지 않았다.

두 key는 0/7400ms의 연출 목표이며 플레이어 시점을 저장한 key가 아니다. Y 차이는0.003263m로
거의 같은 높이이며, 연출 도중에는 사용자가 저장한 약0.972m의 작은 수평 이동을 보간한다.
시작의 현재 카메라에서1200ms 진입, 7400ms에 복귀 시작, 8600ms에 현재 플레이어 Follow로 돌아오는
기존 공통 Camera 구간을 유지했다. 처음/끝 플레이어 pose는 runtime이 취득하므로 P0/P3를 더하지 않는다.

Map Area publish PASS(8 files), source/runtime Camera revision70 동일성 및 JSON3 parse,
Composition135의 bundle1→resource36→camera.kouku.pattern.1 연결과 기존 시간값을 확인했다.
로그와 수치는 out/KoukuIntroTuning20260908/user-camera-map-publish.log 및
user-camera-validation.json에 있다. 데이터 배포만 수행했으므로 C++/XML 변경·재빌드는 없다.
Camera Tool Save는 저작 cache에 이미 반영됐고, Server 재생은 다음 run에서 배포 Camera를 다시 읽는다.

입력은 코드의 실제 소비 경로로 확인했다. Level의 gameplay gate는 Follow 여부와 sequence 연결
Area cameraTrack을 검사한다. 이 PATTERN_ONLY shot의 sequenceInstanceId는 비어 있으므로
Composition Camera는 Follow 상태의 이동·스킬을 막지 않는다. PlayerController는 현재 VIEW/PROJ로
우클릭/스킬 aim ray를 만들고 player groundY 평면을 피킹해 IPlayerCommandSink로 Server에 제출한다.
F6 Free 또는 ImGui가 소비한 마우스·키보드/텍스트 입력은 기존 계약대로 gameplay에서 차단된다.
이는 읽기 전용 호출 경로 확인이며 Client 화면 조작·실제 입력 PASS를 대신 기록하지 않았다.

## G23. 2026-09-08 쿠크 Camera Tool의 단일 Pos

쿠크 Area cameraTrack은 0ms의 P1 한 개만으로 저장·재생할 수 있다. 기존 durationMs를 유지하고
Sample_Cue의 endpoint sampling으로 전 구간 같은 Eye/LookAt/FOV를 사용한다. 처음 현재 화면에서
P1까지의 진입과 마지막 플레이어 Follow 복귀는 기존 Composition Camera blend가 계속 담당한다.
기존 Valtan document parser와 Valtan active controller는 최소 두 key 규칙을 유지한다.

Camera Tool의 Kouku Source에서 Pos 선택, Go To Scene, Start, Save는 한 key를 허용한다.
Duration을 바꿔도 유일한 key는 0ms에 유지하고 static shot도 P1 하나로 표시한다. Capture Pos는
기존대로 새 Pos를 추가하며, P1의 현재 구도를 바꿀 때는 Fine Adjust 또는 Capture / Replace
Selected Scene을 사용한다. 기존 다중 Pos path는 그대로 편집하며 두 개 이상일 때 마지막 key는
durationMs여야 한다. CAS 저장과 실패 시 기존 source/draft 보존 경로는 변경하지 않았다.

Level Area parser와 Composition publisher의 Area track validator를 1..64개 계약으로 맞췄다.
Map publisher는 기존 cameraTrack을 그대로 배포하므로 변경하지 않았다. Valtan cue validator는
별개이고 최소 두 key를 유지한다. 새 Camera runtime·C++ 파일·project/filter 등록은 없다.
source Camera revision71은 기존 camera.scene.auto.1 P1만 남기고 P2를 제거했다. P1 Eye는
(-3.0489308834,25.3147125244,331.9632873535), LookAt은(2.4010531902,18.4604892731,327.1344909668),
FOV60이며 사용자 저장값을 그대로 보존했다. track7400ms, 진입1200ms·기존 유지·복귀1200ms와
나머지 shot은 유지했다. Map Area publish8파일 및 source/runtime 동일성 검증 PASS다.

검증은 기존 test_composition_pipeline.py의 CameraTrackContractTests 3개 PASS다. 단일 P1의
원본 pose/duration 보존, 빈 목록·0이 아닌 시작 시각·잘못된 view 거절, 두 Pos 종료 시각 경계를
확인했다. 기존 test_valtan_camera_tool_contract.py도 PASS다. 변경 소스는 기존 UTF-8/CRLF,
Python은 지정된 LF를 유지했고 scoped git diff --check가 통과했다. 최종 Debug Product compile/link/deploy는
20260908T044302127Z-debug-product.json의 모든 단계 PASS다. 자세한 통합 로그는
out/KoukuPausedScrub20260908과 Kouku Gate Pattern Bundle RESULT G19에 있다.
실제 Pos 한 개 표시·Capture/Save/Reload·Start와 아레나 화면 확인은 사용자 대기다.


## G24. 댄스 장판 +90도·박수14개·PRODUCT 선택 (2026-09-08)

Composition source149의 P6 세이튼_댄스타임은 PRODUCT다. boss.kouku.dance의 presentation.4는
rotationDegrees [0,90,0],0~26800ms이며 GROUP/leaf 원본은 유지했다. 사용자가 저장한 박수
boss.kouku.dance.clap은 presentation.6~19 총14개, 각2000ms이고 기존 시작 시각·transform을
보존했다. brightnessMultiplier는 모두1이다. G19의 clap23300~25300ms 한 개는 과거 연결이며
현재 저장본으로 덮어쓰지 않았다.

Patterns 목록의 Set Pattern to PRODUCT는 선택 Pattern의 검증된 draft 상태를 바꾼다.
Save가 기존 CAS 저장과 ready PRODUCT publish를 실행하고 Server 재시작 뒤 제품에서 사용한다.
버튼 자체를 저장·서버 적용 완료로 표시하지 않는다. 해당 코드와 native 저장 회귀 결과는
[Gate Pattern Bundle RESULT G21](2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md)에 기록했다.

source149 JSON에서 +90도·박수14개·PRODUCT 상태를 확인했으며 Composition publish는
PRODUCT11개/88 stages로 통과했다. out/KoukuJoker20260908/composition-publish.log가 근거다.
최종 Product 통합 빌드와 사용자의 실제 장판/박수 화면 확인은 아직 별도 완료 기록이 필요하다.
에이전트는 Client/UI를 실행·조작하거나 visual PASS를 기록하지 않았다.


## G25. Collider Box Detail의 즉시 geometry Preview (2026-09-08)

Position/Rotation/Scale, Width/Height/Depth와 Radius를 바꾸면 Preview 버튼 없이 geometry 요청을
보낸다. 크기 입력은 기존 resource 치수를 기준으로 occurrence Scale에 환산한다. 요청은 적용된
occurrence의 세 geometry 배열만 바꾸므로 Detail의 미적용 시간·Bone·Logic을 함께 실행하지 않는다.
Apply/Save의 저장 의미와 실패 보존은 유지했다. Revert geometry와 선택 동기화는 현재 적용된
geometry를 복구한다. 원본 JSON/schema와 resource definition은 이 변경에서 수정하지 않았다.

Workbench는 연속 drag를 같은 stable ID의 최신 요청으로 합친다. 재생/정지된 Pattern과 Bundle
child는 clock과 session을 유지하고, inactive는 현재 cursor에서 paused Preview를 한 번 준비한다.
MainApp/PresentationPlayer는 정확한 owner의 해당 Collider wire만 갱신하며 Effect/SFX를 다시
시작하지 않는다. follow=false는 저장한 anchor basis와 scale로 geometry를 계산해 정지 anchor를
유지한다. 이 소비 경로는 실제 코드 읽기 검토이며 화면 검증을 대신하지 않는다.

기존 ValtanPatternAuditionServiceHarness Debug x64를 BuildProjectReferences=false로 빌드하고
--kouku-preview-transport-contract를 실행해 exit0 PASS를 확인했다. running/paused Pattern,
Bundle child의 끝 cursor, geometry 요청 합치기, invalid/NaN/ID 오류에서 이전값 보존, 다른
미적용 필드 격리, Cancel/선택 복구, 계층 Reset 폐기, inactive321ms/끝시각 보존과 기존
Apply/Save/Reload·Stage retarget 검사가 포함된다. source bytes·dirty·generation도 유지했다.
초기 fixture 기대값은 parse된 occurrence와 기존 계층 전환 STOP 계약에 맞춰 수정했다.

로그는 out/KoukuJoker20260908/collider-live-geometry-build.log와 collider-live-geometry-contract.log다.
소유 C++/harness의 scoped git diff --check PASS, UTF-8/CRLF 유지 확인을 완료했다. 최종 Product
compile/link/deploy와 사용자의 실제 Detail drag/정지 pose/wire 확인은 통합 완료 기록을 기다린다.
에이전트는 Client/UI 실행·조작·캡처를 하지 않았다.

## G26. 최종 실행 파일 반영 (2026-09-08)

G24 댄스타임 +90도·clap14개와 PRODUCT 상태, G25 Collider 즉시 geometry Preview가 최종
Debug Product EXE에 반영됐다. Engine/Shared/Server/Client compile·link·deploy 모두 PASS/exit0,
receipt는 `out/BuildPipeline/runs/20260908T062920624Z-debug-product.json`이다.
실제 새EXE의 즉시 geometry Preview 문자열도 확인했다. source149/World402/Camera71과
배포 데이터의 동일성, 변경 JSON/XML parse를 재확인했다. 빌드·맵 입장 오류 진단의 통합 결과는
같은 폴더 `2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md` G23에 있다.
편집기 입력·저장 회귀는 PASS이며 실제 아레나의 시각·음향·drag 결과는 사용자 확인 대상이다.


## G27. V2 Effect의 즉시 geometry 편집과 Save (2026-09-08)

Box Detail P/R/S와 Preview를 실제 Pattern/active Bundle member의 actor·bone·WORLD 및 현재
clock으로 연결했다. 선택별 Effect geometry staging을 Save 후보에 모두 합쳐 CAS 저장하므로
여러 박스를 바꾼 뒤 Apply 없이 Save해도 보존한다. geometry 외 미적용 timing/Bone/Logic은 섞지
않는다. invalid와 CAS 실패는 이전 원본/draft를 유지하며 Revert는 해당 box만 복구한다.

PresentationPlayer는 Effect/Collider의 resolved anchor와 WORLD scale을 저장한다. V2 GROUP/LEAF는
새 geometry sampler를 적용하고 같은 handle·clock·pause·snapshot·기존 child 객체에서 같은 age를
재계산한다. 이미 렌더 큐에 있는 객체를 교체하지 않아 연속 drag 도중 객체가 빠지는 경로를 막았다.
particle world birth·표시 크기와 sprite/trail의 parent scale 소비를 연결했고 Trail replay는 기록된
pivot을 샘플한다. source leaf/group geometry와 다른 Effect/Sound는 변경하지 않는다. 활성 Effect
구간의 Stop/paused seek는 기존 raw history/첫 anchor를 유지하며 역순 history append를 하지 않는다.
과거 anchor를 취득할 수 없는 replay 실패는 해당 Effect만 격리하며 시각 상태 rollback으로 기록하지 않는다.

기존 native --kouku-preview-transport-contract의 Debug refs=false 빌드·실행 PASS다. LEAF/GROUP의
running/paused/Bundle/cold clock, 복수 박스 Save/Reload, invalid/CAS 실패·unrelated field 보존과
기존 Collider/Stage 회귀를 포함한다. 로그는 effect-live-geometry-build.log,
effect-live-geometry-contract.log 및 최종 product-save-launch-contract.log이며 모두
out/KoukuJoker20260908 아래에 있다. 기존 V2 occurrence-runtime Python8개도 PASS다. 이 Python
검사는 source/JSON 계약이며 native V2 particle/trail 시각 동작 PASS로 해석하지 않는다.

최종 Debug Product의 Engine/Shared/Server/Client compile·link·deploy가 exit0로 완료됐다.
receipt는 `out/BuildPipeline/runs/20260908T071602366Z-debug-product.json`, 로그는 `out/KoukuJoker20260908/product160-v2-build.log`다.
변경 source JSON/XML parse와 소유 범위 diff check를 확인했다. Client/UI 실행·조작·캡처는 하지
않았으며, 현재 Effect 활성 시각에서 Stop → Box Detail P/R/S 조절 → Save/Reload의 실제 표시와
회전·크기는 새 EXE에서 사용자가 확인해야 한다.

## G28. Effect Sequencer의 여섯 트랙과 Composition 조작 통일 (2026-09-09)

기존 `CEffectAuthoringSequencer`에 Animation / Effect / Collider / Sound / Camera / Screen Post
여섯 트랙을 연결했다. Action Workbench의 `CompositionTimeline`과 `CompositionResourceTree`를
재사용하며, V1/V2 Effect Tool에서 각 Sequencer의 Resources와 Box Detail을 연다. Effect 박스는
V1 document 또는 V2 GROUP/LEAF 전체의 occurrence이고 내부 element별 트랙으로 펼치지 않는다.

타임라인 상단 눈금 영역 전체를 drag하면 하나의 노란 재생 커서가 여섯 트랙의 공통 시간을
변경한다. 겹치는 박스는 같은 트랙 안에서 아래로 쌓는다. 박스 본체 이동과 양 끝 trim은 drag
중 후보를 표시하고 release 때 검증·적용한다. Box Detail의 시간·Mute 및 트랙별 속성,
Duplicate/Remove, Play/Pause/Restart/Stop/Loop를 기존 실행 owner에 연결했다. 별도의 Effect
실행 경로는 만들지 않았다. 이 UI 연결은 코드 확인이며 실제 mouse 입력 검증은 사용자 몫이다.

### 실제 Resources와 실행 범위

| 트랙 | 현재 목록과 소비 경로 |
|---|---|
| Animation | 현재 Model View에 로드된 실제 `CModel` clip. source start/play 구간, 재생 배율, Loop/Mute를 occurrence로 저장하고 공통 clock으로 sample한다. 단일 모델의 활성 clip 겹침은 거부한다. |
| Effect | 저장 V1 inventory와 기존 V2 catalog의 typed GROUP/LEAF. V1 `CEffectObject`와 V2 runtime의 기존 준비·sample·정리 경로를 사용한다. V1 추가 시 실제 preview duration을 사용한다. |
| Collider | 저장된 쿠크 `presentationResources`의 형상 정의를 복사한다. 현재 GEOMETRY 7개는 BOX/CIRCLE/SECTOR wire를 root 또는 실제 Bone에 배치한다. ROULETTE_CARD_REGION 8개는 Action Workbench gameplay Logic이 필요하다는 이유를 표시하고 추가를 막는다. |
| Sound | `Client/Bin/Resources/Sound`의 WAV/OGG/MP3를 Resources 상대 asset ID로 선택한다. Engine의 기존 SoundCue를 occurrence별 독립 handle로 재생·정지·seek한다. |
| Camera | 현재 camera view capture와 기존 recovery camera/key 편집을 사용한다. 공통 clock과 기존 cinematic controller / authoring camera override를 소비한다. 모든 boss camera 목록을 복제한 기능은 아니다. |
| Screen Post | V2 `ScreenPost` category의 LEAF를 별도 lane으로 표시한다. 준비 시 실제 SCREEN_POST type을 확인하고 기존 V2 presentation 경로를 사용한다. |

Resources는 명시 Refresh와 모델 교체 시 갱신한다. 한 종류의 목록 읽기가 실패하면 해당 종류의
이전 목록과 이유를 유지하며 다른 종류의 정상 항목을 남긴다. Animation 편집은 이 Sequence의
custom occurrence에만 적용하고 기존 skillbindings나 쿠크 Pattern/Bundle 원본을 변경하지 않는다.
Animation 시간이 바뀌면 이전 root/Bone history를 버리고 현재 cursor를 다시 sample한다.

Sound는 일반 tick마다 채널을 재생성하거나 seek하지 않는다. 자연 종료 후 같은 구간에서 계속
재생성하지 않으며 명시 seek/restart/loop가 다시 진입할 때만 준비한다. 정지 상태의 seek는 처음부터
paused 채널을 사용한다. Source보다 긴 박스의 나머지 구간은 침묵이다. Collider는 `CHitAreaWire`
표시만 수행하며 overlap, damage, Server state를 생성하지 않는다. 룰렛 등 gameplay 판정은 기존
Action Workbench → Shared/Server 권위 경로에 남는다.

임시 Effect Preview에서 Resources의 Add / Append를 실행하면 준비된 Preview와 camera rows를
Sequence에 편입한다. 같은 Effect를 추가할 때는 Preview를 한 번 편입하고 중복 occurrence를
생성하지 않는다. 다른 항목은 검증·준비 후 추가한다. Preview를 편입하기 전 Save는 이유를 표시한다.

### 저장과 실패 보존

저장 위치는 `Data/Effects/Sequences/<Sequence ID>.effectsequence.json`이다. schema
`lostark.effect-authoring-sequence`의 formatVersion 4를 쓰고 기존 1~3을 읽는다. 기존 model,
effects, cameras에 `customAnimation`, `animationRows`, `soundRows`, `colliderRows`와
`effects[].screenPost`를 추가했다. stable occurrence/resource ID와 원본 clip 구간, 시간·Mute 및
형상 값을 저장하고 runtime handle이나 history는 저장하지 않는다.

Load는 새 배열을 parse·validate·stage한 뒤 기존 실행을 종료하고 함께 commit한다. 잘못된 값,
중복 ID, 없는 source/필수 모델·Bone 때문에 실패하면 현재 timeline을 보존한다. Effect의 실제 GPU
준비는 기존 Play 경로에서 수행하므로 JSON Load 성공만으로 GPU admission을 주장하지 않는다.
Save는 기존 파일 baseline 충돌 검사를 유지하고 원자 저장한다. 외부 변경이나 같은 ID의 기존 파일을
조용히 덮어쓰지 않는다. 이 Save는 Sequence 배치만 저장하며 Effect 원본, skillbindings,
boss Composition 또는 Server gameplay 데이터를 publish하지 않는다.

Recovery camera의 v4 읽기는 기존 Effect + Camera 추출 용도이며 나머지 트랙을 실행하지 않는다는
상태를 표시한다. 여섯 트랙 전체 복구는 Sequencer의 Load를 사용한다. 새 `_Timeline.cpp`,
`_Resources.cpp`, `_Tracks.cpp`, `_Presentation.cpp`는 Client 프로젝트와 filters에 등록했다.

### 확인한 검사와 남은 확인

- 실제 production Save/Load, 추가 트랙 parser/writer·validator, DataJson 및 atomic writer의
  함수 본문을 재사용한 out-only native probe가 exit 0, 29 checks / 0 failures다. v4 왕복,
  v1/v3 호환, 잘못된 배열·중복 ID·없는 source에서 현재값 보존, 외부 파일 변경 시 저장 거부를
  확인했다. 근거는 `out/EffectSequencerComposition20260909/codec_run_result.json`이다.
- 위 codec 검사의 모델 clip/Bone metadata와 음원 길이는 fixture다. Camera key 실행, Effect GPU
  stage, 실제 FMOD decode, Client/UI 실행 및 시각·음향 결과는 이 검사에 포함하지 않았다.
- 별도의 실제 FMOD `NOSOUND_NRT` 검사는 15 checks / 0 failures이며 200 ms source 길이,
  독립 동시 handle, paused birth/seek, 자연 종료 후 재생성 방지, stale handle 정리와 실패 시
  다른 cue 보존을 확인했다. 근거는
  `out/EffectSequencerActionWorkbench20260909/sound_actual_results.txt`다. 실제 출력 장치의
  음향 청취나 Sequencer 전체 입력 검증은 아니다.

G28 최종 Debug Product compile/link/deploy는 성공했다. 아래 최종 receipt를 기준으로 한다.
새 실행 파일의 Resources 추가, 노란 커서·박스 drag, 여섯 트랙 동시 재생과 Save/Load,
실제 모델/Bone·Effect·camera·sound의 화면/음향 결과는 사용자가 직접 확인해야 한다.
에이전트는 Client/UI를 실행·조작·캡처하지 않았고 visual/audio PASS를 기록하지 않았다.


G28 추가 실패 경계 검증: 최신 실제 `Play`, `Append_Sound`, `Commit_TransientPreview`,
`Sample_Sounds` 본문을 추출한 out-only presentation 검사29개가 통과했다. 채널 준비 실패나
Effect256개 한도·Camera owner 거절 시 기존 Preview/문서/다른 Sound handle을 보존하고,
새로 준비한 paused handle만 해제한다. transient Effect Preview는 실제 사용하지 않는 저장된
Sound/Collider 오류에 막히지 않지만 전체 Play는 해당 검증을 유지한다. 자연 종료의 매 프레임
재생성 방지와 명시 seek 재생성, occurrence 종료 시 자신의 handle만 해제하는 동작도 포함한다.
`presentation_run.log`와 `presentation_probe_source_receipt.json`이 근거이며 audio/model/
camera/GPU는 명시적 fixture다. FMOD 실제15개 검사와 혼동하지 않는다.

첫 최종 Product 시도는 Engine/Shared/Server 성공 뒤 Client의 `Shader_VtxEffectParticle.hlsl`
Debug `/Od` compile에서 X4505(임시 register4096 한도)로 실패했다. 여섯 트랙 C++ 컴파일
실패가 아니다. 이 shader 파일의 Debug x64만 DisableOptimizations=false 및 `/O1`로 바꾸어
native material 분기들의 register 수명을 최적화하도록 한다. C++ Debug 설정과 원본 셰이더 식,
다른 shader 설정은 유지한다. 이 변경의 최종 성공 여부는 다음 Product receipt로 기록한다.

두 번째 Product 시도는 다른 세션의 native material 추가가 합쳐진
`Shader_VtxEffectMeshPreview.hlsl`에서도 같은 X4505로 실패했다. 이에 MeshPreview와 Particle
두 파일의 Debug x64 FXC에만 `/O1`을 적용했다. C++ Debug 옵션과 두 PS의 계산식은 바꾸지 않았다.
사용자 중단 요청 시점에는 세 번째 Product 시도가 진행 중이다. 추가 복원과 실험은 중단했으며,
이 빌드의 최종 상태와 배포 확인만 아래에 기록한다. 앞선 DimensionMaster Product 성공 receipt는
현재 여섯 트랙 Sequencer를 포함하는 최종 빌드 성공 증거로 사용하지 않는다.

### G28 최종 빌드·종료 상태

세 번째 Debug Product는 exit0으로 완료됐다. receipt는
`out/BuildPipeline/runs/20260909T083046383Z-debug-product.json`이며 Engine/Shared/Server/Client
전부 PASS, missingRuntimeInputs0이다. 전체50분14.853초이며 Client compile이 대부분을 차지했다.
MeshPreview와 Particle의 Debug x64 `/O1`로 X4505 없이 두 CSO가 생성됐다. 완료 CSO의
읽기 전용 disassembly에서 두 PS 모두 dcl_temps17, indexableTemp0이다. MeshPreview의
static instruction74702, Particle100917은 정적 코드 크기이며 실제 GPU 실행시간/FPS가 아니다.
근거는 `out/EffectSequencerComposition20260909/shader_pressure/pressure_20260909T083106.json`.

Client.exe는49,826,304B, 최종 수정 시각17:30:46 KST이다. Engine.dll은 Engine과 Client 양쪽
8,722,432B로 배포됐고 SHA256 일치를 확인했다. 기존 FXC X4000, compiler C4819, 외부 PDB
LNK4099 등의 경고는 남지만 최종 컴파일/링크 오류는 없다. 변경 JSON parse 및 프로젝트/filter
등록 XML 확인과 전체 git diff --check가 통과했다. 별도 publisher/광역 runtime diagnostics는
실행하지 않았다. 다른 세션의 변경을 포함한 공유 작업 트리의 빌드이며 새 commit/push는 없다.

마무리 추가 요청의 A cube4행과 기존 crack8행의 실제 데이터 검사는 Round2 RESULT G35에 둔다.
이 데이터는 빌드 종료 전 저장됐으며 최종 A66행의 검증 SHA를 별도로 보존한다. Client/UI를
실행·조작·캡처하지 않았고 시각·음향·FPS·assertion 미발생을 대신 PASS로 기록하지 않았다.
현재 server-host LAN 설정에 따라 사용자는 Debug x64의 Server + Client profile을 Ctrl+F5로
시작하고 Lobby → Character Select → DimensionMaster → F1 → Effect Tool V1에서 확인한다.
Saved Skill Effects의 이펙트_차원술사A_전체를 다시 Load하여 네 검격과 cube/crack을 확인하고,
R_전체와 R_튜닝은 별도로 비교한다. 추가 작업은 여기서 종료한다.


## G29. 선택 element의 Sequencer Solo (2026-09-10)

구현: `.restore` 복구본의 기존 element Solo와 Effect Detail의 `Timeline Solo`를 기존 Sequencer의 임시 행에 연결했다. element 이름/stable ID를 보관하는 행 하나만 표시하며 원본문서의 시작 지연과 source emitter delay를 유지한 등장 시각에서 정지한다. Time 또는 시간 눈금으로 앞뒤 탐색하고 Play/Pause로 확인한다. 전체 문서의 수백 element를 행으로 펼치지 않는다.

현재 draft → 선택 stable ID document projection → 기존 V1 occurrence factory → CEffectObject를 사용한다. 원본문서 timing·seed·source attachment를 유지하며 선택 element가 요구하는 ModelCue만 숨긴 anchor로 보존한다. stage와 최초 focus 샘플 성공 뒤 이전 미리보기를 교체한다. 같은 문서를 검토할 때 model sequence 재선택이 기존 미리보기를 먼저 Stop하지 않도록 했다. 최초 다른 문서/모델을 선택하는 기존 target 전환 계약은 유지한다.

Live Detail/Refresh는 선택 ID와 cursor를 유지하고 변경된 element의 실제 종료 시각으로 행 길이를 갱신한다. Linear Lerp 편집 후 별도 World Preview 시계로 재시작하던 호출을 막았다. 단일 element preview 중 Model View의 중복 timeline 조작 대신 Sequencer 조작 안내를 표시한다. 임시 element 행은 read-only Box Detail을 제공하며 Append/Commit/Save가 전체 문서 저장행으로 잘못 승격시키지 않는다. Stop 후 기존 저장행이 다시 표시된다.

저장 데이터: Effect JSON, animevents, skillbindings, sequence 저장 schema와 product skill 연결은 이 변경에서 수정하지 않았다. 사용자 튜닝 중인 Alt V 파일도 쓰지 않았다. 새 C++ 파일, project/filter 등록, Resources payload는 없다. 기존 8개 C++ 파일의 encoding/BOM/newline을 유지했다.

검증:

- Client Debug|x64 `ClCompile` 성공, 오류 0, 경고 151(C4819/C4244 포함), 33.45초. 호출부가 포함된 Effect_Tool/Workspace와 Sequencer 및 header 의존 소비자를 컴파일했다. 로그: `out/EffectSelectedElementSequencer20260910/client-compile.utf8.log`.
- 관련 기존 source-contract 3개 성공: selected audition의 play/seek 순서, particle audition의 stage 후 restart, Lerp의 staging 성공 후 restart. 이 검사는 새 Sequencer의 GPU 재생/실제 화면 통합 검증을 대신하지 않는다.
- 변경 8개 소스의 시작 snapshot과 before 대비 diff, SHA는 `out/EffectSelectedElementSequencer20260910`에 보존했다. 신규 상설 검사나 Client/UI 자동 실행·화면 캡처는 하지 않았다.
- 필요한 마지막 확인: scoped `git diff --check` 및 현재 프로세스/배포 상태를 아래 마감 기록에 남긴다. JSON/XML을 변경하지 않아 해당 parse 검사는 없다.

실행 준비 상태: 컴파일 시 Client PID 42808과 Server PID 73356이 실행 중이었다. Client의 최종 링크·배포는 하지 않았으며 현재 실행 중인 EXE에는 G29가 없다. CLAUDE의 Client 종료 후 최종 링크 규칙에 따라 사용자에게 튜닝 저장/종료 확인을 요청했다. 사용자 확인 전 Client/Server를 종료하거나 교체하지 않는다.

사용자 확인 경로: F1 → Effect Tool → 차원술사 Alt V full restore Open Editor → 개별 element의 Solo 또는 선택 후 Effect Detail의 Timeline Solo → Effect Sequencer Time/Play/Pause. 원래 4초에 등장하는 요소의 초기 cursor, 앞뒤 scrub, Detail 수정 후 scope/시간 보존, Stop → 전체 Play 복귀는 사용자 화면 확인 대기다.

### G29 마감 기록

변경 8개 C++와 대응 문서 4개의 scoped `git diff --check`가 성공했다. 컴파일 receipt에 기록한 소스 SHA와 최종 소스가 일치하며 기존 source-contract 3개도 최종 코드에서 다시 성공했다. 결과는 `out/EffectSelectedElementSequencer20260910/final-check.json`이다. 사용자는 종료 확인 요청에 “아직 튜닝 중”이라고 답했다. 따라서 실행 중 Client를 보존하고 **최종 링크·EXE 배포만 대기**한다. 사용자 종료 통지 뒤 Client Debug Build를 한 번 실행하고 배포 증거를 여기에 덧붙인다. 현재 화면의 동작을 이 수정의 검증 결과로 기록하지 않았다.
