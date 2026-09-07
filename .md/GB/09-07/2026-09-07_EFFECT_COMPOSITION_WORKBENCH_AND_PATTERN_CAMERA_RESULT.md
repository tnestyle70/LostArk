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
6. Camera와 fake Spotlight는 기존 G03 경로를 유지한다. 룰렛 보행면은 현재 해당 Pattern이 DRAFT라 Product run에 아직 포함되지 않는다. 사용자가 Pattern Status를 PRODUCT로 저장·Publish All PRODUCT 후 Server를 재시작하면 Complete Play에서 확인할 수 있다. 현재 저장 상태는 임의로 바꾸지 않았다.

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
