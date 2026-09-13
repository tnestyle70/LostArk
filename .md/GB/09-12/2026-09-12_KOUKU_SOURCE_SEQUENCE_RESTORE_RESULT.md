# 쿠크 원본 시퀀스 저작 연결 결과

## G08. 정상 피날레·팝업북 결합 적용

2026-09-12 사용자의 `전부 반영하자` 승인 뒤 PLAN G06의 코드·정본·게시·최종 Debug 빌드를 완료했다. 사용자 화면 판정은 남아 있다. 시작 HEAD는 `5168899d`, 브랜치는 `codex/kouku-authored-finale-popup`이다. 같은 작업공간의 Character/Map 성능 개선, 관문 Effect 그룹·ALBION 작업은 이 G08의 변경으로 집계하지 않는다. 자동 stage/commit/push는 하지 않았다.

### G08-01. 코드와 정본에 적용한 범위

- P4 `KAKULSAYDON_G1_PATTERN_4`: 정상 피날레 23 MAP track → 포탈·흡입 → 정상 팝업북 136 MAP 배치·배우·책·카메라를 58,810ms로 연결했다. WORLD 8개, CAMERA 7개, EFFECT 4개이며 `enterCombatOnFinish`와 원래 target boss 계약은 보존했다.
- P1의 MAP occurrence 수명을 37,800ms로 늘리고, 원래 4,507ms 모션 키는 유지했다. 책은 기존 `evt2_book02` template을 참조하는 전용 HOLD instance다. 기존 196 template·251 instance·83 camera 및 다른 5개 Pattern은 보존했다. revision은 Sequence 7 / World 676 / Camera 81이다.
- 신규 V1 3개·V2 1개를 추가했다. 기존 효과 원점·회전·크기·source track·내부 event·particle lifetime을 보존하면서 254개 V1 요소의 28 activation 그룹과 V2 fade 47키만 새 시각에 맞췄다. 원본 4개 문서는 변경하지 않았다. V1 3개는 EffectCatalog의 DIRECT_AUTHORED_DOCUMENT까지 연결하고 프로젝트 96.DataFiles에 4개 파일을 등록했다.
- placement 43/44, 45/46, 114/115의 assetId만 동일 mesh의 F1 material variant로 교체했다. 이 G08에서 Resources 물리 파일과 shader를 추가하거나 수정하지 않았다.
- RenderingProfiles revision 30에 `scene.kakulsaydon.g1.popup.v1`을 추가했다. F1 region 48의 환경 입력을 전용 profile로 옮기고 optional `environmentRegions`는 생략했다. 현재 parser는 명시적 빈 배열을 거부하므로 PLAN의 초기 빈 배열 표현을 교정했다.
- Level은 기존 MapLight 문서/provider를 재사용하여 32개 F1 광원을 Z -204.8m로 복사하고 14개 책 공간 광원을 해당 연출 동안 비활성화한다. 실제 HOLD occurrence 수명으로 provider를 선택하고 MainApp WORLD 샘플 뒤 한 번 제출한다. source provider는 보존하며 Stop은 이미 등록된 provider를 Clear하지 않는다. 같은 포인터의 authoring 문서 변경도 감지해 preview 실패 소비자까지 전달한다.
- 기존 Deploy 5·7을 초기 숨김 상태로 두고 정상 연출의 대여·복구 경로를 사용한다. P4에서 별도 source Saydon 배우 참조를 제거했다.
- RenderingProfiles publisher를 C++ float32 변환·범위와 맞췄다. 9자리 저장값을 다시 정밀 문자열로 고치는 우회 없이 통과시키며 원문 벡터 범위·유한성·near/far와 실패 시 기존 출력 보존을 유지한다.

### G08-02. 실행한 검증과 게시

| 확인 | 실제 결과 |
|---|---|
| Rendering profile 공식 Validate / Publish | PASS, runtime revision 30 |
| Map Area 공식 Validate / Publish / Check | PASS, placement 3,368개·출력 8개. 변경 출력은 mapplacements/worldsequences/camerashots |
| Rendering publisher 기존·추가 unittest | 13개 PASS, 28.488초. 0.1/0.0001/0.0312의 9자리 왕복, 범위 밖 float32·overflow·NaN·Inf·타입·벡터 원문 범위·near/far·실패 출력 보존 포함 |
| 실제 CModel/CMaterial 준비·Clone·clip 샘플 | P1/P4 모델 57개 PASS, WORLD 바인딩 299개 확인 |
| 실제 WorldSequenceDocument reader | source/runtime 모두 196 template·252 instance 수락 |
| 실제 CKoukuSaydonCompositionDocument reader | Sequence revision 7, Pattern 7개, quarantine 0 |
| 실제 MapLightDocument/MapLightPresentationRuntime 및 Level helper CPU 실행 | 119→151개, 29 POINT+3 SPOT 복사, 14 disable. 원본 불변·누락/충돌/nonfinite/unready 거절·same-pointer 변경 감지·owner 해제 후 retained provider ready 확인 |
| 시간/소유권 자료 검사 | P1 [0,37800), P4 [21010,58810), 종료와 역방향 재진입, Deploy 5/7 초기 숨김 확인 |
| 현재 Product V1 codec | 새 V1 3개 Load → Serialize → Parse → 재직렬화 일치, source matrix 28표본 유한값, failures 0 |
| 효과 시간 검사 | source clock의 float32 최대 차이 3.8147μs, V2 키 엄격 증가·수명 범위, 신규 ID 유일성 확인 |

자료는 `out/KoukuPopupFinale20260912/implementation/`의 `map-*.log`, `rendering-tests-final.log`, `authored-effect-retime-result.json`, `effect-parser-contract-result.json`, `actor-cpu/VERIFICATION.md`와 관련 JSON이다. Sequence 보존 검사는 상위 `sequence-implementation-validation.json`에 있다. 광원 helper와 모델 probe는 현재 소스를 out에서 새로 컴파일했으며 제품 파일을 변경하지 않았다. 이전 probe의 DLL ABI 불일치는 새 컴파일·현재 Debug DLL 경로로 해소한 검사와 구분했다. V1 codec 구형 probe의 최초 DLL 진입 실패도 현재 Product object·헤더로 새 console probe를 링크하고 신규 Engine DLL과 명시적 Resources root를 사용해 해소했다. 최종 실제 검사 결과는 `current-effect-probe/result.json`이다.

### G08-03. 실행 파일과 사용자 확인 경계

추가 요청인 Parent 편집 진입과 Rename IME 수정은 각 RESULT에서 기록한다. `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`가 18:00:06에 367,010ms로 완료됐으며 Engine·Shared·Server·Client 모두 PASS다. 새 Client는 18:00:05에 링크됐고 Engine 원본/Client 배포 DLL의 SHA가 동일하다. EXE/DLL PE 형식, 수정 CPP보다 최신인 object, 새 Parent 진입 버튼 문자열 포함도 확인했다. 산출물 확인은 `implementation/final-build-verification.json`, 공식 기록은 `out/BuildPipeline/runs/20260912T090006253Z-debug-product.json`이다. 기존 외부 PDB 누락·인코딩·FX 초기화 경고는 있었지만 빌드 오류는 없었다. 이 후속 작업에서는 배포 ZIP을 갱신하지 않았으며 새 실행본은 원래 Bin/Debug 경로에 있다. 현재 켜진 out 복사본은 이전 빌드로 계속 실행되고 있다.

자동 검증은 Client/UI 실행·조작·화면 캡처·visual PASS가 아니다. WorldSequencePlayer 인스턴스 전체 Stop/Seek와 Level의 실제 렌더 제출은 실행하지 않았다. 사용자가 직접 `KoukuSaydon → F1 Developer Tools → Sequencer Benchmark / Composition Sequencer → 1관문_통합_시퀀스`에서 문 열림, 포탈·흡입, 암전, 책 펼침 유지, 커튼/배경, 세이튼 한 명과 종료 후 F1 전환을 확인한다. 독립 `연출_팝업북`과 `연출_1관문 피날레`도 비교한다. F1의 구운 RNM을 움직이는 책 부품에 복사하지 않았으므로 최종 간접광·시각 일치는 사용자 관찰로 판정한다.

사용자의 명시적인 후속 요청에 따라 빌드 잠금 분리를 위한 기존 실행본을 `out/InteractiveRuntime/20260912_174839`에 준비했다. 363개 EXE/DLL/CSO/DataFiles의 복사 전후 SHA가 일치하며 새 코드의 빌드본이 아니다. 저작 Data와 Resources는 정본 경로를 연결하고 Bin/DataFiles는 복사 시점의 게시 결과를 사용한다. `Prepare-Snapshot.ps1`은 다음 복사본 준비, `Start-Snapshot.ps1`은 기존 앱 종료 후 명시적 실행을 맡는다. `-VerifyOnly`를 통과했다. 사용자가 직접 기존 앱을 종료하고 실행을 요청한 뒤 17:53:46에 복사본 Server PID28752와 Client PID85052를 시작했고 0.0.0.0:7777의 listener owner가 Server PID임을 확인했다. 최초 서버 실행은 redirected stdin EOF 때문에 정상 시작 직후 종료되어, 자체 숨김 콘솔을 주는 실행으로 교정했다. 앱 UI를 조작하거나 화면을 캡처하지 않았으며 복사본 실행은 새 기능 검증이 아니다. 이 편의 산출물은 out에만 두고 제품 runtime에 두 번째 Level/모델 경로를 추가하지 않는다.

## G07. 팝업북·피날레 사용자 이미지 후속 조사 — 구현 미실행

2026-09-12 후속 요청에서 사용자가 코드 수정 대신 원인 조사와 수정 계획만 요청했다. 이 후속 작업은 PLAN의 G06을 갱신했고, 제품 코드·Data·Resources·runtime·ZIP을 변경하지 않았다. 다른 작업이 같은 작업공간에서 수정한 렌더링 성능 파일은 이 조사 변경이 아니다. 아래 G00~G06의 과거 생성·설치·검증 기록과 이번 미적용 계획을 구분한다.

사용자가 첨부한 세 이미지를 열람하고 정상 P1/P2와 P4의 참조, 실제 WorldSequencePlayer와 Level의 수명·복구 경로, WModel section/재질/광원 데이터를 읽어 대조했다. 정상 피날레23개 MAP track이 P4에 없는 점, 정상 책과 다른 clip/좌표/카메라, MAP box4507ms 종료의 standing arena 복귀, STOP 책의 종료, Deploy5와 source SaydonStage의 겹치는 좌표를 확인했다. 정상 book과 source book의 mesh/material/skeleton section은 동일하다.

popup40개 slot의 texture88개 누락은0이다. F1과 동일 mesh의 기존 material asset으로 교체할 placement6행을 특정했다.136개 배치 전체가 F1과 같은 것은 아니며 최종pose 비교에서24개만 기준 내에 대응됐다. 책과F1은204.8m 떨어져 local light가 다르고 region47 ambient는 region48보다25% 밝다. 효과174개 WORLD track과80개 축포의 위치는 이미 정상 광장·책 공간에 있으므로 book 위치 차이를 더하는 수정은 계획에서 배제했다.

현재 계획은 정상 피날레 0~21.010초, 후반 포탈/흡입 12.258~21.010초, 정상 팝업북 21.010~58.810초다. 정확한 WORLD/카메라 참조, 전용 효과 4문서의 시간 변환, 재질 6행, scoped 조명 32개 복사/14개 제외, profile과 publisher float32 검증 변경을 [구현 계획의 G06](2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md)에 기록했다. runtime 검토에서는 광원 Submit을 WORLD 샘플 이후로 이동하고 등록 provider의 Clear를 금지해야 한다는 결함을 계획에 반영했다.

근거는 `out/KoukuPopupFinale20260912/sequence-transition-diagnosis.json`, `sequence-plan-snippets.json`, `material-candidate/`의 JSON/메모다. 이번 검증은 읽기 전용 데이터/코드 비교와 계획 문서 검토이며, 새 구현의 컴파일·publisher·Client/UI 재생·시각 PASS를 실행한 결과가 아니다. 수정 범위와 적용 명세를 준비한 상태다.

마무리 검사에서 Sequence/WorldSequence/CameraShots 세 정본의 SHA256이 조사 baseline과 동일함을 다시 확인했다. 변경한 PLAN/RESULT에 한정한 `git diff --check`는 exit0이었다.

## G00. 현재 상태와 적용 경계

원본 Matinee를 기존 Sequence Composition, CameraShot, WorldSequence, Effect 문서로 연결한
후보를 `out/KoukuSourceSequenceRestore20260912/candidate`에 작성했다. 실행 중인 기존 exe는
신규 `enterCombatOnFinish`를 모르므로 저작 Data, 프로젝트 등록과 기존 Resources는 아직
교체하지 않았다. 새 배우 WModel 9개와 반사 정적 모델 26개만 기존 파일이 없는 Resource 경로에 생성했다.
사용자 Client/UI 실행·조작·캡처와 화면 판정은 수행하지 않았다.

`authoring_install_manifest.json`은 Data 20개와 프로젝트 2개의 기존 baseline SHA 및
후보 SHA를 기록한다. 설치 전 모든 baseline을 함께 비교해야 한다. 별도 검사에서 복사한
기존 Pattern Flow 문서는 이 설치 목록에 포함하지 않는다. 기존 제품 map publisher 출력도
이 하위 작업에서 생성하거나 변경하지 않았다.

## G01. 원본 시간축과 전투 종료 계약

| Pattern ID 끝자리 | 표시명 | 원본 Matinee | 후보 길이 | 전투 연결 |
|---|---|---|---:|---|
| 4 | 1관문_통합_시퀀스 | SCENE03A Matinee0 | 41,488ms | G1 |
| 3 | 기존 2관문 진입 이름 보존 | SCENE04A Matinee2 | 27,000ms | G2 |
| 5 | 2관문_클리어 | SCENE02A Matinee10 전체 | 35,368ms | preview |
| 6 | 2관문_카드미로 | SCENE04A Matinee1 | 11,950ms | preview |
| 7 | 3관문_진입 | SCENE02A Matinee10의 16,710ms 도착 cut부터 | 18,658ms | G3 |

모든 ID는 `KAKULSAYDON_G1_PATTERN_` 접두사를 그대로 사용한다. 3·4·7만
`enterCombatOnFinish=true`다. 기존 팝업북과 1관문 피날레 두 Sequence는 보존했다.
신규 World 정의는 공식 `kakulsaydon.g1.world.15`~`.32`을 사용하고 source 의미는
각 `sequenceInstanceId`에 보존했다. 기본 환경 정의는 `kakulsaydon.g1.sceneprofile.2`다.

G1은 포탈 2.344762초, 흡입 4.354424초, 암전 7.459291초, 책 펼침 10.218567초를
원본 공통 시간축으로 연결했다. 두 기존 연출의 길이를 합산하지 않았다. G2 클리어의
SL05 도착 cut은 원본의 완전 암전 16.13896~17.19185초 안에 있다.
G1 책 세트와 열린 책 전투 아레나는 다른 위치다. 마지막 Server 관문 활성화·player 이동·
보스 HUD·follow camera 복귀는 부모 작업의 기존 typed command 계약이 소유한다.

## G02. 카메라, 암전과 환경

신규 shot은 G1 18개, G2 클리어 18개, 카드미로 7개, G3 진입 8개다.
기존 32개와 합쳐 83개이며 한 shot의 key는 최대 64개다.
실제 reader Save 왕복 뒤 카메라 후보는 1,311,326bytes, JSON value 36,874개다.
2MiB/128shot/64key/128Ki value 경계 안이다.

원본 Director의 활성 cut과 movement/FOV Hermite 곡선을 표본화했다. source horizontal
FOV는 16:9 vertical FOV로 변환했다. 원본 0도/180도는 perspective 특이점이므로
1.1~178.9도로 유한 투영했다. 원본 수치는 source cache에 남아 있고
`fov_finite_projection.json`에 G2 카드미로에서 하한에 걸린 4개 key를 기록했다.

원본 Fade 곡선은 기존 V2 black overlay의 intensity key로 투영했다. G1과 G3의
`입장_기본환경`은 기존 `scene.kakulsaydon.g1.base.v1`의 명시적 재사용이다.
원본 환경 전체 복원으로 판정하지 않는다. G2는 기존 원본 light/fade를 유지한다.
임의 DOF, MotionBlur, 다른 패턴의 find-true-dark 프로필을 추가하지 않았다.

## G03. 모델과 좌표

신규 배우 WModel 9개는 `Map/KakulSaydon/SourceSequences/<prefix>/<actor>/<actor>.wmodel`에
생성했다. 합계 140,416,484bytes이며 mesh·skeleton·material을 유지하고 원본 A/B animation
slot blend를 30Hz 단일 WANM으로 표본화했다. Material은 실제 BossCatalog 모델에서 읽는다.
G1/G3 Saydon은 RPCT05, preScale .017, object scale 1을 사용한다. 원본 연출의 drawScale
1.2보다 사용자 요청의 실제 전투 크기를 우선했다. SCENE02A의 look 정보는 RPCT07이지만
실제 skeletal component는 RPCT05이며 필요한 원본 clip이 모두 설치 모델에 존재한다.

| 대상 | 확보한 위치 근거 | 현재 투영 |
|---|---|---|
| G1 포탈·흡입·축포 | 원본 emitter actor 월드 위치, 회전, ON/OFF | sourceTransformTrack + MAP occurrence |
| G1 책과 부착 배경 | 원본 Move, parent와 book bone | 실제 baked CModel bone을 거쳐 월드 좌표 |
| G2·G3 이동/터널/포탈 | 원본 actor Move와 attach-relative frame | 원본 부모 체인과 source 시간 원점 |
| G2 뿅망치·나팔·손 이펙트 5개 | 실제 부모 CModel의 b_wp_1 또는 bip001-l-hand | 30Hz bone WORLD 표본, 곡선 축소 |
| 원본 4명 player 배우 | playerlocationinfo의 matplayer_partymember와 낙하/착지 key | 파티 배우 치환은 미구현; 고정 더미를 추가하지 않음 |

UE3 cm의 `(X,Y,Z)`는 현재 맵 `(X,Z,-Y)*.01`이다. Attach Move key를 절대 월드로
오인하던 부분은 parent/bone frame으로 합성하도록 수정했다. 회전은 같은 basis를 좌우로
적용하고 Effect의 source origin/time origin은 occurrence의 MAP 위치/시작 시간과 일치시켰다.

G1 source static actor 280개 중 266개는 현재 정확한 mesh/material로 연결됐다.
14개는 커튼 curtain01c 9개와 black/white plane·lightbeam 등 5개로, 정확한 원본 material
identity가 현재 catalog에 없어 임의 재질로 대체하지 않았다. 원본 signed scale의 94개 배우는
별도 reflected WModel 26조합으로 처리하며 양수 abs만 적용해 반사 방향을 잃는 방식은 쓰지 않는다.
266개 배우는 `.actor.<sourceExport>`를 갖는 독립 Object Resource ID로 연결했다.
같은 모델을 쓰는 두 배우가 하나의 runtime target을 공유하지 않도록 했으며 mesh/재질 입력은 재사용한다.
마지막 33.583초에 actor374가 내려가고 actor375가 올라오는 원본 교체도 SaydonFinale 배우로 연결했다.

## G04. 원본 이펙트와 카드 분출

기존 최종 native leaf library를 재사용해 5개 원본 구간을 11문서로 분할했다.
분할은 source occurrence 경계에서만 하므로 내부 emitter event 참조가 다른 문서로 나뉘지 않는다.
전체 1,252개 native 요소를 보존하며 문서별 particle 8,192/trail point 2,048 경계를 실제
Codec으로 검사했다. G1 축포 6개 source occurrence는 기존 festival 문서를 재사용한다.
Coloredpaper 4개 요소는 같은 sourceMaterialPath·rendererShape를 가진 설치된 native material로
복원했다. 원본 occurrence 누락을 임의 개수 제한으로 숨기지 않았다.

G2 cardfly의 6개 actor × 3개 mesh emitter는 기존 CardEruption CModel WorldOccurrence가
이미 재생하므로 새 native document에서 중복 18요소만 제거했다. 같은 cardfly의 나머지 native
요소는 유지했다. 현재 CardEruption은 BG map material binding을 사용하므로 원본 translucent
MeshMaterial override와 완전히 같은 재질이라고 판정하지 않는다.

원본 lifetime=0인 13개 요소는 SourceRecipe의 0을 유지했다. 현재 particle runtime은
0 결과에 Detail fallback을 적용하므로 이 fallback만 각 원본 ON/OFF 활성 구간
4.075~11.54초로 한정했다. 범용 무한 수명 지원이나 원본과 동일한 over-life 결과로 판정하지 않는다.

카드 분출 WModel은 14,000 ticks/1,000Hz였지만 CModel이 30Hz로 재생하여 실제로는
466.667초가 걸리는 시간 단위 오류였다. 기존 retime 도구로 420 ticks/30Hz, 14초 후보를
`CardEruption.30hz.wmodel`에 만들고 verify를 통과했다. builder도 앞으로 30Hz로 생성한다.
이후 사용자가 Client/Server 종료를 확인한 뒤 기존 Resource를 교체했다(G06-3 설치 receipt).
G2 진입 배우의 기존 WANM은 810 ticks/30Hz로
정상이었다. 맵에 고정된 카드는 분출의 끝 프레임이 아니다. 현재 정적 card placement 951개 중
SL02에 549개가 별도로 있으며 SCENE04A/SL04에는 정적 card placement가 없다.

## G05. 검증 증거와 남은 경계

- 실제 `CEffectDocumentCodec` Load → Validate_Drawable → Save_Atomic → reload → Serialize
  동일성: 11/11, 1,252요소. `Codec/run.log`의 첫 10문서와 `Codec/remaining.log`의 수정 G2 문서다.
- 부모 작업의 실제 Sequence reader/metadata/Complete 선택/Stop/실패/CAS/구문 격리 검사 통과.
  `out/KoukuSequenceCombat20260912/sequence-contract.log`.
- 실제 카메라 reader 83shot, reload, 실패 cache, 1,000회 Ensure와 CAS 보존 검사 통과.
  `out/SequencerOpen20260912/camera_full_probe/cache_probe.run.log`.
- Map publisher의 기존 Read-WorldSequenceDocument/Read-CameraShotDocument 함수를 out에서
  호출한다. 제품 publisher나 runtime 출력 쓰기는 실행하지 않는다. 최종 `map_validate.log`에서 `SOURCE_SEQUENCE_MAP_DOMAIN_PASS`를 확인했다.
  WorldSequence 후보는 14,351,816bytes/434,349 JSON values로 16MiB/1Mi value 경계 안이다.
- 반사 모델은 실제 Engine decoder로 26개 source/candidate basis, material 경로 120개,
  정점·UV·winding과 설치 SHA를 검증했다. 오차 0, 원본 24개 SHA 보존을 확인했다.
  `out/Reflect/verification.json`, `basis_verification.json`.
- 변경한 Python 5개 AST parse와 프로젝트 후보 XML parse, 변경 경로 git diff --check 통과.
- 프로젝트 후보에 신규 15개 Effect JSON의 `None`/`96.DataFiles`만 추가하고 XML parse를 확인했다.
  현재 실행 프로젝트에는 아직 설치하지 않았다.

원본 4명 player 슬롯은 `player_actor_review.json`에서 실제 package의 playerlocationinfo를
해석해 `matplayer_partymember`, `bbacktostartlocation=false`, `bfinishidleanimation=true`를
확인했다. 0~5초 animation은 idle뿐이며, 명시된 낙하/착지는 29.72~32.07초다.
집결/흡입 전용 clip이 누락됐다고 단정하지 않는다.

원본 전역 Slomo는 G1 25.688~26.885초(최소 .1), G2 클리어/G3 진입 28.938~29.514초(최소 .2)에
있지만 현재 Sequencer 공통 clock으로 소비하지 않았다. 표의 길이는 source Matinee clock이다.
G1/G3 일부 face SkelControl, 원본 플레이어 집결 배우, 위 14개 배경 재질도 미완료다.
원본 전체 visual fidelity, 실제 화면 프레임과 전투 종료 핸드오프는 사용자 확인 전이다.

## G06. 7개 시퀀스의 실제 모델 준비와 Table admission 재검증

사용자가 보고한 WORLD preview 실패를 대상으로 기존 p1/p2와 신규 p3~p7을 함께 검사했다.
검사 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` revision 6과
MIDNIGHTC_ED WorldSequence revision 673이다. 현재 설치 파일을 읽은 시점에는 runtime도
revision 673이었다. 앞서 확인한 runtime revision 672의 18 instance/275 object 누락은
이전 파일 상태이며, 14:55:07 갱신 뒤 검사에서는 해당 누락이 없다. 이 조사와 probe는
publisher를 실행하지 않았으며 runtime 문서 갱신의 실행 주체를 판정하지 않는다.

| 대상 | 실제 검사 결과 |
|---|---|
| WORLD occurrence / binding | 7패턴, 43 occurrence, 453 binding. ID·활성 instance·동일 패턴의 공유 placement 중복 검사 통과 |
| binding 종류 | OBJECT_RESOURCE 293, MAP_PLACEMENT 159, DEPLOY_PLACEMENT 1. 고유 Object Resource 288개 |
| 모델 준비 입력 | 재질·preScale까지 구분한 146개: Object 90, Map 52, Deploy 4. 물리 model ID 115개 |
| 실제 CModel / CMaterial / Clone | 146/146 준비 및 Clone 통과. 요청 clip 29개를 시작·끝에서 CPU 샘플 |
| 실제 C++ World 문서 reader | source/runtime 모두 196 template, 251 instance Load/Validate 통과 |
| 실제 placement reader / binding 대응 | Map 3,368개, Deploy 6개 읽기. 453 binding의 정확한 placement/model identity 확인 |
| camera/effect/profile 참조 | source/runtime camera 58개, effect 문서 19개, scene profile 1개. 참조 물리 자원 249개 누락 0 |

기존 `out/KoukuActualModel20260912/actual_model_probe.cpp` 경로를 out에서 좁게 확장했다.
제품의 ActorCatalog, MapAssetCatalog, MapPlacementDocument, DeployPropCatalog,
WorldSequenceDocument, DataJson, RuntimeAssetRoot, ProjectDataRoot OBJ와 기존 Engine DLL을
사용한다. 새 C++ 한 TU만 별도 OBJ/PDB로 컴파일했고 제품·shader 빌드는 하지 않았다.
Map은 Loader의 NONANIM/preScale .01/전체 materialOverrides를, Deploy는 실제
ActorCatalog/preScale .01을 그대로 사용한다. 전체 World 문서가 참조하는 기존 Deploy
clip metadata 검증 때문에 7패턴 밖의 Deploy 모델 3개도 준비했다.

최종 실행은 exit 0, 19.8807초다. 실행 전후 source 두 문서, runtime Map 8파일과
실제 Table의 SHA가 모두 같았다. 입력 binary SHA도 검증 당시 같았다.
증거는 `out/KoukuSequenceAdmission20260912/model_probe_seven/verification_receipt.json`,
`result.json`, `run_receipt.json`과 `reference_audit_7patterns.json`이다.

### G06-1. Table의 확인된 실패 원인과 후보 경계

실제 Table 파일은 누락되지 않았다. 기존 468,144bytes WModel을 실제 Engine decoder로
읽으면 `Legacy WMSH contains unsupported trailing payload.`로 거부된다.
`build_gate2_intro_composition.py::visible_table_mesh`가 원본 submesh 4개 중 1/2만
남기면서 bounds 4개를 그대로 붙였기 때문이다. 남은 submesh 2개에 필요한 bounds보다
80bytes가 많다. 신규 5패턴이 참조한 92개 물리 WModel의 실제 decoder 검사에서는
91개가 통과하고 이 Table만 실패했다.

원본 builder를 교정한 out 후보는 468,064bytes이며 WMAT/WSKL/WANM 3section과
mesh의 정점·index·bone 입력을 보존했다. 후보의 실제 decoder 및 CModel 준비 결과는
mesh 2개, material override 2개, animation 1개, Clone 및 해당 clip의 시작·끝 샘플 통과다.

- 검사 당시 실제 Table SHA: `e0d878c392700b958e4465ba805a1c7b734777fdfbe91c5618184062c91ebe3b`.
- out 후보 SHA: `3e4f2af46bd9c45f2af719bf254ca50c71b6f9c48eb69ad64ea49df23a332ce2`.
- 후보: `out/KoukuSequenceAdmission20260912/candidate/Resources/Map/KakulSaydon/Gate2Intro/Table/Table.wmodel`.

7패턴 fixture는 Table만 물리 out 후보와 동일 DDS 복사본을 사용했다. 그 외 모델은
실제 Resources 입력을 읽었다. 이 G06 검사는 기존 Resources를 교체하지 않았다.
후속 설치 여부는 부모 작업의 실제 설치 receipt로 구분한다.

### G06-2. 준비 성공과 화면 재생 성공의 구분

위 검사는 실제 CModel/CMaterial 준비, C++ 문서 admission과 clip CPU 샘플의 증거다.
D3D11 WARP device는 사용했지만 draw, 실제 Layer 설치, Effect 재생, Camera 이동,
Complete Play와 Client 화면을 실행하지 않았다. camera/effect 표는 현재 참조의 가용성
검사이며 원본 Scene Profile이나 전체 Effect의 시각 복원 성공을 뜻하지 않는다.
원본 4명 player 배우, 14개 배경 재질, 전역 Slomo, 일부 face SkelControl,
CardEruption의 원본 translucent material 차이 및 사용자 화면 판단 경계는 G03~G05에
기록한 대로 남는다. 첫 프레임 준비 성공을 원본 전체 연출의 재생 완료로 기록하지 않는다.

기존 Level 오류는 missing instance, disabled instance, duration/speed, stable occurrence ID,
중복 occurrence를 하나의 문구로 합쳤다. 특히 열린 Object Tool의 saved document는 디스크가
갱신돼도 자동 교체되지 않으므로 현재 파일의 PASS가 실행 중인 이전 snapshot의 PASS를
증명하지 않는다. occurrence/instance ID와 선택된 문서 revision을 함께 표시하는 최소
소스안과, CModel 실패 시 동일 mesh에 대한 기존 decoder report를 보존하는 소스안을
부모 작업에 전달했다. 이 조사에서 제품 오류 처리 코드를 수정하지 않았다.

### G06-3. 사용자 종료 후 실제 설치 입력으로 재검증

사용자가 Client/Server 종료를 확인한 뒤 부모 작업에서 Table과 기존 대기 리소스를
설치했다. Table은 `out/KoukuSequenceAdmission20260912/table_install_receipt.json`,
CardEruption 등은 `out/KoukuSequenceCombat20260912/resource_install_receipt.json`에
원본·후보 SHA 및 실제 설치 상태가 기록돼 있다. Table은 15:06:44 KST에 설치됐다.

설치 뒤 동일 out 실행 파일로 146개 준비를 한 번 더 수행했다. 이번에는 Table도 포함해
모든 모델과 재질 입력을 실제 `Client/Bin/Resources`에서 읽었다. 후보 root를 대신 쓰지 않았다.
21.762초, exit 0, CModel/Clone 146개와 요청 clip 29개, source/runtime World 문서,
453 binding identity가 다시 통과했다. source/runtime Map 문서와 실제 모델 115파일의
실행 전후 SHA가 같았다. Table SHA는 `3e4f2af46bd9c45f2af719bf254ca50c71b6f9c48eb69ad64ea49df23a332ce2`,
CardEruption SHA는 `3865b1110911be80fd7de96b97ddb69095923ec4905194f65b06a8af69ee0bae`다.
증거는 `model_probe_seven/result-installed.json`, `run_receipt-installed.json`,
`verification_receipt-installed.json`이다. 최초 후보 검증 기록도 별도로 보존했다.

부모 작업의 Level 실패 원인 분리와 기존 decoder report 연결 diff도 읽기 검토했다.
기존 admission 조건·시점과 rollback을 유지하며, 실패한 occurrence/instance/revision과
현재 mesh에 해당하는 decoder 원인을 구분한다. 설치·CPU 준비 성공 이후의 제품 빌드와
사용자 화면 재생 결과는 별도이며 G06-2의 시각 검증 경계를 유지한다.


## G09. Bloom 1.3 게시와 Rendering Benchmark 경계 조사

쿠크 레벨의 scene.kakulsaydon.g1.base.v1 qualityOverride.bloomIntensity를 1.3으로 저장하고
공식 RenderingProfiles Validate/Publish를 완료했다. authored/runtime 모두 revision 31이며
두 JSON의 전체 의미가 같다. 전역 quality와 다른 Level은 보존했다. g1.popup의 18개 quality
값은 변경 전 g1.base와 전부 같았으므로 중복 qualityOverride를 제거했다. 기존 popup의
light/fog/environment와 multiplier는 그대로이며 활성 쿠크 Level의 Bloom 1.3을 상속한다.

현재 저장본 revision 30은 변경 전에도 공식 Validate를 통과했다. 현재 Client70972는 정상
Client/Bin/Debug에서 실행하므로 과거 out 복사본 경로 혼선을 이번 원인으로 단정할 수 없다.
Save는 메모리 catalog를 Authored에 저장하고, Publish는 디스크 Authored를 검증·게시하며,
Reload Runtime은 실행 파일의 DataFiles를 다시 읽어 메모리에 commit한다. 게시 성공만으로
이미 실행 중인 Client의 catalog가 교체되지는 않는다. 사용자가 Reload Runtime을 누르면
이번 revision 31이 적용된다. Client/UI 조작, 프로세스 종료·재시작은 수행하지 않았다.

구조상 확인한 결함은 RenderingProfileService::Publish_Runtime이 CREATE_NO_WINDOW로
publisher를 실행하면서 stdout/stderr를 수집하지 않아 종료 코드만 보여 주는 점과, UI 호출
안에서 최대 120초 동안 WaitForSingleObject로 동기 대기하는 점이다. 게시기는 strict JSON
검사용 Python을 PATH에서 찾으므로 환경 의존성도 있으나 실제 실패 문구가 없어 이번 실패
원인으로 확정하지 않았다. 기존 float32 왕복 수정은 이미 적용돼 있다. 이 후속은 요청한 값의
게시와 조사이며 C++ 비동기 job·오류 출력 수집 기능의 구현 완료를 의미하지 않는다.

검증: 공식 Validate/Publish PASS, 기존 publisher의 runtime 왕복·Workbench float32 경계·
qualityOverride 왕복/실패 출력 보존 3개 테스트 PASS(3.168초), 원본 변경 필드 제한과
source/runtime 동등성 확인. 증거는 out/RenderingBloom20260912/result.json과 변경 전
RenderingProfiles.before.json이다. 데이터만 바꿨으므로 새 EXE 빌드는 필요하지 않다.


사용자가 제공한 실제 상태 문구는 `Rendering runtime profile published`였으며 성공 분기다.
이번 증상은 publisher 실패로 분류하지 않는다. 후속 19:37 Save/Publish에서 authored/runtime이
다시 revision31로 저장된 것을 확인했다. base의1.29999995는 C++ float32의1.3과 동일하다.
반면 메모리의 이전 popup qualityOverride가 다시 저장됐다. Save_Authored는 현재 디스크
revision과 비교하지 않고 메모리 catalog 전체를 저장하므로, 외부 편집과 같은 revision을
충돌 없이 덮어쓸 수 있는 구조다. 실제 이번 데이터에서도 이 덮어쓰기를 확인했다.

사용자의 최신 저장값을 유지하고 popup의 나머지17개 quality값이 base와 같음을 확인한 뒤
그 중복 override만 다시 제거했다. 최종 게시 revision은32이며 source/runtime 의미가 같고
쿠크 base와 popup이float32 Bloom1.3을 사용한다. 사용자는 다음 Save 전에 Reload Runtime으로
최신 catalog를 받아야 이 외부 정리를 유지한다. 이전 저장본은
out/RenderingBloom20260912/RenderingProfiles.user-saved-31.json에 보존했다.
C++ stale-save 검출·비동기 publisher·상세 로그 개선은 조사 결과이며 구현한 것으로 기록하지 않는다.

## G10. 2026-09-13 포탈·폭죽·연출 배우·재질 후속

기존 미커밋 변경을 유지한 채 `kouku-pattern3-sequence`를 만들고 전환했다. 기준 HEAD는
`3fc23750761107fee9c6933df9f926d93165de05`다. 실행 중 미저장 편집이 있다는 사용자 답변에 따라
처음에는 out 후보만 만들었고 Client/Server를 종료하거나 UI를 조작하지 않았다. 이후 두 프로세스가
없는 상태를 확인하고 최신 저장 Sequence20, Action420, World677의 SHA를 다시 비교해 병합했다.
원본15파일의 backup/CAS/설치 결과는 `out/KoukuPattern3Sequence20260913/installation_receipt.json`과
`installation_backup/`에 있다. 최종 저작 revision은 Sequence21, Action421, World678이다.
사용자의 암전 occurrence, 폭죽 XZ·박스 창, 기존 animation instance와 무관한 Pattern은 보존했다.
신규 Effect9개는 기존 catalog와 Client `96.DataFiles`의 None/project filter에만 등록했다.

### G10-1. 폭죽은 동시 생성 예산, 포탈은 원점과 sprite 축 결함

`Effect Tool → Play All`의 단독 성공은 P4 동시 생성의 성공을 보장하지 않았다. 기존 포탈 두 문서의
mesh 예약량1995에 폭죽238을 더하면2233으로 캐릭터 owner 한도2048을 넘었다. 맵의 모든
LevelPlacement를 하나의 캐릭터 예산으로 합친 원인이다. `Can_AdmitBudget`에서 Level만 기존
scene hard 한도를 사용하게 했다. Character/Boss owner, remote soft, scene 상한은 유지했다.
실제 비용 함수의 원본 실패 재현 및 경계·pending·overflow26검사는 통과했다.

폭죽은40요소(visible36/simulation4), 실제 tail 포함 수명12.2385초다. 방출 구간6.2385초와 다르다.
resource 기본 수명을12239ms로 맞추고 저장한 가로 위치·24785ms 박스는 유지했다. source 발사
높이4.1899975586m를 복구했다. 낮은 배치만으로 전체 비표시를 설명하지 않았으며 예산 거부를
주원인으로 분리했다. Box Detail은 현재 플레이어 좌표·MAP 원점까지 거리를 표시하고
`F1 Move Player → Use Player Position → Apply → Save`의 입력 순서를 안내한다.

포탈 원본1의 공통 원점은 오망성 중심에서19.694m 떨어졌다. 오망성25요소와 흡입5요소를
`effect.kouku.gate1.intro.portal-suction.centered`로 분리했다. 주변 쥐·금빛120요소는 별도 context로
그대로 유지하고 원본2의 금빛24요소도 바꾸지 않았다. 새 포탈 중심은 MAP
`[72.8571191406,1.76162734985,-99.7693652344]m`이다. 기존 임시 회전을 제거하고 local-space
fixed-axis sprite의 축에 source emitter basis를 한 번 적용했다. 4704표본 중 해당44표본만 바뀌고
나머지4660표본은 byte 동일했다. 법선Y는1에서0.0010719로 바뀌었고 중심의 yaw0/90 이동 오차는0이다.

문서 내부14.6027618초 대기는 제거하고 P4 박스 시작14603ms가 출현 시각을 소유한다.
새 문서 수명은15511ms, 흡입 시작은 local2.00966초다. Box Start를 옮겨 일찍 생성할 수 있다.
박스를 길게 늘리는 것과 원본 입자 수명·흡입 시점을 늘리는 것은 별개다.

### G10-2. 실제 연출 세이튼과 조명 대상

전투 보스와 P4의 연출 배우는 별개다. P4 world.13/world.21은 실제 Deploy5의
`world.sequence.instance.original_kouku`를 재생한다. `세이튼_1관문_연출` 이름의 alias와 표시명을
추가하고 기존 World Object의 Animation Clips 목록을 기본 펼침·번호·구간 표시로 연결했다.
12개 clip은 삭제하지 않았다. `Sequencer → 1관문_통합_시퀀스 → World 세이튼_1관문_연출 →
Edit This Motion → Animation Clips`에서 현재 항목을 제거하고 Save할 수 있다. 이 instance는
기존 P1과 P4가 공유하므로 해당 모션 편집은 양쪽에 반영된다.

원래 MAP Spotlight의 절대 위치를 BOSS 상대 offset으로 다시 더해 대상에서 멀어지는 결함을
수정했다. MAP 단독 사용은 기존 결과를 유지하고 BOSS/PLAYER/WORLD는 실제 대상 높이에서
source ray가 닿는 중심을 pivot에 맞춘다. 리소스 Preview도 선택한 관문·actor context를 전달한다.
사용자가 추가한 P4 조명 occurrence31은 WORLD world.13/world.21로 연결했다. start26525ms,
duration11243ms와 사용자 값은 보존했다. 실제 이 시점의 배우 중심은
`[-.288301,1.3253,737.629028]m`, 조명 높이는35.470001m이며 range59.8m 안에서 중심에 도달한다.
공용 resource 기본값은 MAP으로 유지한다. 아직 준비되지 않은 WORLD를 전투 보스나 identity로
대체하지 않고 실제 샘플된 Deploy pivot을 사용한다.

### G10-3. 팝업북 누락6개와 동적 광원7개

원본 SCENE03A에 연결된 동적 광원5 POINT/1 SPOT/1 DIRECTIONAL을
`effect.kouku.gate1.popup.movable-lights`로 등록했다. 같은 Matinee의 move/rotation/brightness/color
시계를 사용하며 관련 없는 l08/float track1269는 추가하지 않았다. 기존 POINT의 기본 specular0을
보존하고 신규7개만 source specular1을 사용한다. 17,759원본 표본의 최대 위치 오차0.122076mm,
방향2.98e-8, RGB0.00179757이며 기존 Gate2 POINT325표본의 최종 LIGHT_DESC는 byte 동일하다.
기존32개 팝업 맵 조명 사본과 사용자 Scene Profile은 별도 소유권으로 유지했다.

`effect.kouku.gate1.popup.source-material-carriers`에는 기존 배치에 없던 actor
672/691/692/811/812/848의 안개·평면2개·흰 섬광·빛기둥·부착 커튼을 추가했다. Native3616~3620의
정확한 material/VF/VS/PS, 원본 texture와 기존 CModel/MESH 재생 경로를 사용한다. white_t의
op/color, shine의32.noisestr/31.fresnal_power 네 곡선은 기존 native parameter 이름과 row/lane으로
연결한다. vector W와 다른 packet은 보존한다. 실제 codec 저장 왕복과4402시간 표본을 통과했다.

EngineMaterials.DefaultMaterial 평면2개의 cooked graph는 삭제돼 있었지만 shader cache에는
정확한 material map이 있었다. global shader 참조를0개로 가정한 추출기 결함을 수정해3개 참조를
소비한 뒤 LocalVF를 찾았다. GUID/static set/repeated set과 물리 cache SHA를 일치시켰고,
global RadialBlur 등록을 별도 화면 블러 실행으로 해석하지 않았다. 실제 zero-global map은
HEAD parser의 전체 JSON과 동일하고 focused55검사는 통과했다.

커튼848의 parent는 actor819→camera4→848이다. camera4는 transform carrier이며 director의
활성 카메라가 아니다. P4 약22.71~26.99초에 가림 전 frustum 기여54표본이 확인됐다.
기존 child356은 이미 등록돼 있어 중복 생성하지 않았다. 원본 quaternion node4개를 포함한
실제 transform9753표본에서 기존 false 경로 행렬은 HEAD와 byte 동일했다. 최대 위치 오차는
0.138mm, occurrence root 한 번 적용 후0.155mm다. native3620은 실제 vertex alpha·tangent
view/up·masked discard를 소비한다. 신규 Resources9개는 원본/후보 SHA와 크기를 대조해 설치했다.

두 신규 팝업 Effect 박스는 P4 start12258ms/duration46552ms이며 기존 승인된 비선형 시간표를
사용한다. MAP root는 identity이고 실제 source 좌표는 요소별 SourceTransformTrack에 있다.
기존136개 MAP 배치, 정상인 책·F1 배경과 기존 floor/decor6행 보정은 바꾸지 않았다.
기존 MAP 커튼8개는 현재 저작 배치를 보존했으며, 이번 신규6개와 원본 재질 복원 수에 합산하지 않는다.

### G10-4. 쇼타임 총구와 표적의 현재 연결

P35 `세이튼_쇼타임`의 기존7개 창을 유지하며 양쪽 WORLD 총에 발사광·signature를 연결했다.
설치 총 모델5265정점 중 실제 앞 격자72정점의 중심을 총구 기준으로 사용했다. local 중심은
`[.01913988,-.02469903,.98251975]m`, 전방은+Z다. WORLD의1.5×사용자1.4 배율을 그대로 소비한다.
실제 CModel loop A382표본의 중심 오차는6.7435e-7m다. 원본에 없는 socket 이름을 만들지 않았다.

바닥/노란 표적의 원본 BOSS+절대 MAP 좌표 중복을 제거하고 fixed/tracking/end를 저장된
MAP `[-.07,1.32,942.33]`에 배치했다. 기존 내부-0.1m 때문에 지면 아래에 있던 표적은 별도
ground variant에서 높이를 교정했다. 원본20개 라이브러리는 보존하고 총구2개·표적3개 variant만
추가했다. source와 variant25문서의185요소/139물리 DDS·WModel 참조를 확인했다.

Source Projectile421991205는 지면 표적·먼지·착탄을 소유한다. 원본 CEFSequenceSummonsProjectileTrace의
Key33 target 의미·callback과 총구→목표 비행 경로는 해독 완료가 아니다. 현재 MAP 표적 배치를
실제 PLAYER 추적 복구라고 기록하지 않는다. 탄피의 Bip002-R-Hand는 총의 B_WP1과 다른 sub-rig여서
원본 손 부착을 보존했다. 임의 총 측면 socket으로 치환하지 않았다.

### G11. 게시 실패 원인과 최종 실행 경계

기존 실패 로그는 Save420/Product382를 보였다. Product 변환은 성공했지만 World publisher가
Kouku Parent의 optional boolean fixedTimeline을 unknown field로 거부하여 전체 rollback했다.
Kouku의 해당 field만 허용하고 다른 encounter·unknown field·잘못된 타입 거절은 유지했다.
`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 421` 실행으로
Product/Map/World/GameplayBalance 네 domain이 모두 PASS했다. 게시된 Product는421이고 Map World는678이다.
현재 저장47개 Pattern 중 게시 가능한33개가 projection됐다. 기존 미완성 Pattern을 임의로 삭제하거나
revision freshness 검사를 제거하지 않았다.

Client/Server 실행·UI 조작·화면 캡처는 수행하지 않았다. 코드·게시·수치 검증은 사용자 화면
판정과 별개다. 사용자 확인 경로는 직접 Server/Client 실행 후 `KoukuSaydon → F1 → Sequencer →
1관문_통합_시퀀스`의 포탈/폭죽/조명/책 펼침 및 Action Composition의 `세이튼_쇼타임`이다.
최종 제품 빌드와 실제 최신 OBJ 검증 결과는 아래에 기록한다.

첫 Product 빌드는 PASS(179.262초, Client172OBJ/4CSO/1EXE)했다. 마지막 review에서 source
static masked3616/3620이 particle용 depth-read profile을 소비한 결함을 확인해 두 프로그램만
기존 OPAQUE_BACK_DEPTH_WRITE로 교정했다. 원본 masked discard는 유지하고 나머지1139개
program row와 기존 particle masked profile은 동일하다. 실제 설치 Effect의 두 renderProfile만
CAS로 교체한 뒤6 carrier actual codec572348검사가 다시 PASS했다. 수정 설치 SHA는
`03b21102513b6f93f7ec5e0547e7dabbf163188616a0c7f1bf844c6738d5bdb3`이며 근거는
`masked_installation_receipt.json`, `candidate/popup_carriers/masked-depth-validation.json`이다.

남은 범위는 기존 MAP 커튼79~86의 재질8개와 쇼타임의 실제 target callback/추적·비행이다.
기존 커튼은 모두 LV curtain01_mi를 사용하지만 SCENE03A 대응은 BG01 네 개와 BG01c 네 개다.
사용자 TRS를 보존하며 해당8개만 고치는 CModel용 source material variant는 아직 구현하지 않았다.
Effect용 native3620의 ID만 Map material에 넣으면 동일 ABI가 아니므로 연결 완료로 기록하지 않는다.
따라서 이번 결과는 모든 팝업 재질·쇼타임 gameplay의 완전 복원 또는 사용자 visual PASS가 아니다.

최종 증분 Product 빌드 PASS: `out/BuildPipeline/runs/20260913T051107655Z-debug-product.json`.
Client의 변경된 native table1OBJ와 EXE를 갱신했고 shader closure도 통과했다. 첫 빌드의 전체
소비자172OBJ/4CSO와 함께 최종 상태다. 기존 shader/PDB/charset 경고는 존재하지만 빌드 오류는0이다.
최종 JSON/XML18파일 parse, 신규 Resources9개 SHA/크기, World source/runtime678 의미 동일성도 PASS다.
검증 중 Client/Server 프로세스는 없었고 실행·종료·화면 캡처는 하지 않았다.

최종 Product37OBJ와 새 fixture2개를 연결한 Showtime CPU probe는34문서 전부 Load/재생 PASS,
실패0이다. 원본20개와 신규5개 및 비교9개를 포함하고 Load 실패 또는 입력 수 불일치를 실패로
처리한다. 실제 CModel 본·총구 수치 검사이며 GPU draw/PLAYER 추적 증거는 아니다.
근거는 `showtime_probe/final_receipt.json`, `playback_final.json`, `muzzle.json`이다.
같은 최신 Product codec으로 P4의7 Effect 문서를 검사한 전체 박스 동시 예약 상한은
particles9310/16384, mesh3783/4096, lights7/32, draw4041/6144이며6개 시간 경계에서 모두 통과했다.
실제 이펙트가 먼저 종료해도 박스 전체를 예약하는 보수적 검사다. `budget/p4_schedule.json`에 있다.

이후 사용자는 Server와 `Client/Bin/Debug/Client.exe`를 직접 실행해 P4와 쇼타임을 확인한다.
별도 재빌드는 필요하지 않으며 F5/Ctrl+F5를 무빌드 실행으로 안내하지 않는다. 현재 branch는
`kouku-pattern3-sequence`이며 사용자 요청이 없었던 commit/push는 수행하지 않았다.


## G12. Box Preview와 포탈 context의17초 앞 대기

Box Detail의 Effect Preview는 geometry 드래그의 정지 커서 경로를 잘못 재사용했다.
명시 Preview는 기존 `Request_PatternPreview`를 박스의 시작 시각에서 재생하도록 수정했다.
Effect local age는0이고 BOSS 애니메이션/WORLD 시계는 기존 Pattern 시각을 유지한다.
일반 TRS 드래그의 paused/cursor 상태는 그대로다. 실제 Workbench CPU probe에서
0/14603/17000/23000ms 즉시 시작, 미적용 Detail override, 다른 박스·WORLD·Animation 보존,
실패 시 이전 요청 보존과4500ms geometry cursor 유지가 통과했다.
`out/KoukuBoxPreviewTiming20260913/workbench-probe/run.log`가 실제 요청 경로의 근거다.

사용자가 작업 중 저장한 Sequence revision22의 centered 포탈 박스는 이미 start0/duration36541이다.
오망성30개 centered 문서의 내부 시작도0이다. 별도 `1관문_전후 쥐·금빛 연결` context120개에는
원본 장면의17.1347671585083초 대기가 남아 있었다. 해당 context의 모든 StartDelay에서
이 값을 빼고 SourceTimeOrigin에 더한 후보를 `out/KoukuBoxPreviewTiming20260913/leading-delay`에
준비했다. 원본 source SHA는 `1f484aea4961c33cd516f379bd988131937e8fdb81a19d30c8cbff399c0014cf`,
후보 SHA는 `9d09ef9e111913872210c74e48ac599911536a14e33520058a28e19e55c25acb`다.
240개 timing field만 바뀌었고 이 기록 시점에는 실행 중 사용자 draft를 보호하기 위해 설치하지 않았다.
원본150개와 금빛 원본2의34초 시각은 변경하지 않았다.

Effect Tool의 Current Effect에 공통 대기 초와 `Remove Leading Delay`를 추가했다.
독립 SourceTransformTrack 문서만 지원하며 model cue/actor/baked history/attachment/inheritance/
enabled SourcePresentation 등 다른 시계 의존성은 이유를 표시하고 거절한다. 미적용 Detail은
Apply한 뒤 처리한다. 검증한 candidate는 기존 `Try_CommitDocument`로 교체하고 `Save Changes`만
파일을 쓴다. 현재의 요소 간 간격, 수명, native emitter delay와 원본 transform/alpha/material 곡선을 보존한다.
기존 portal 분리 generator도 context에 같은0초 기준을 적용해 다시17초를 넣지 않게 수정했다.

변경된 Effect Tool 두 CPP의 격리 컴파일은 통과했다. 실제 두 helper 함수의 원문을 그대로
분리한 CPU probe는 실제 codec, `CEffectPlayback::Sample_SourceTransformTrack`, native material
packing을 연결해2300검사를 통과했다. context120개×501시점에서 C++ 편집본과 설치 후보를
각각 원본과 비교했고 실제 popup6개/four material curve도 scratch pre-roll로 검사했다.
최대 source clock 오차3.8147e-6초, transform matrix component 오차2.95639e-5, native packet
lane 오차0이다. 지원하지 않는12입력의 거절과 source 보존도 확인했다.
`leading-delay/probe-receipt.json`, `full_compile_receipt.json`, `generator-receipt.json`에 기록했다.
Generator 재실행은 결정적이며 원본150개 무변경, centered30개 byte 동일, context 후보 의미 동일이다.
Client/Server 종료·UI 실행·화면 검증이나 최신 Product EXE 교체는 수행하지 않았다.

후속 범위 감사에서 Effect Tool의 저장 목록에는 전체150개 `portal-arrival.1`도 남아 있음을
확인했다. 현재 Action427/Sequence22가 직접 참조하는 resource는0개지만 Catalog/EffectResourceTree에서
Play All할 수 있다. 전체 문서의 공통 대기는14.602761848449706초이고 그 안의 context 요소가
17.134767초부터 시작한다. 전체150개에도 공통 대기를 제거한 별도 out 후보를 준비했다.
새 시각은 포탈0/흡입2.00966/context2.532005초이며 서로의 간격은 보존한다.
`leading-delay/original1/candidate-receipt.json`의 source SHA는
`637f3b431fa9532de83cd919fcd6e8fcea11b8508900fdef8579d1ae75e8b22c`, 후보 SHA는
`e867a88b065052b6ebc71f29f4be232a7ee43f57cac1e24563587d355936dbd3`이다. 아직 Data에는 설치하지 않았다.

동일 실제 C++ helper/codec/SourceTransform/native material probe의150개 검사2360회도 통과했다.
최대 source clock 오차3.8147e-6초, matrix component2.00272e-5, native packet lane0이다.
정규화된 전체150개를 generator 입력으로 두 번 재생성해 centered30/context120의 시작0과
모든 파생 field·배치 원점이 종전 생성 결과와 동일함을 확인했다(시간 오차0).
공통 대기가 이미0인 포탈30개에는 다시 offset을 적용하지 않는다. 전체150개 안에 남은
context 상대2.532005초는 context 독립 파생본 생성 때만 제거한다. 원본2/금빛34초는 변경하지 않는다.

## G10. 통합 암전 미표시: BC1 A coverage 거부 교정 (2026-09-13)

사용자 요청대로 포탈과 카메라는 수정하지 않았다. 실제 작업 위치는 C:/Users/USER/source/졸업팀폴/LostArk, 시작 HEAD 461224f9, 적용 브랜치 codex/kouku-g1-fade-visible이다. 기존 암전 timing 16개(+약0.3초)와 Framework.sln의 다른 변경을 보존했으며 자동 stage/commit/push는 하지 않았다.

원인: CEffectV2Object::Submit_Presentation은 기본 A coverage를 사용한다. 기존 fx_a_blankwhite_01.dds는 DXT1/BC1이며 CPresentation_Manager::HasOverlayCoverageChannel의 A 허용 목록에는 BC1이 없어 Add_ScreenOverlay가 거부한다. 흰색/불투명 픽셀 디코딩 결과만으로 runtime format admission을 검증했다고 볼 수 없다. GPU draw 이전에 발생하는 계약 결함을 소스와 실제 DDS header로 확인했다.

적용: 기존 RGBA8 흰색 Map/LV_BER_BERNCASTLE/SourceMaterials/50c22a2692ed_flat_white.dds를 Effect/KoukuSaydon/Textures/Cutscene/fade_opaque_white.dds로 복사하고 kouku.gate1.authored.fade.black의 slots.base 한 필드만 교체했다. 원본 DDS는 보존했다. 새 파일은 Client/Bin/Resources 아래 168 bytes이며 Git 제외 runtime 입력이다. 다른 PC에 전달할 때 새 상대 asset ID를 포함해야 한다. Drive 업로드는 하지 않았다.

2026-09-13 23:30 사용자 결정으로 이 적용을 되돌렸다: `slots.base`를 원래 `Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_blankwhite_01.dds`로 복구하고 `fade_opaque_white.dds`와 빈 `Cutscene` 폴더를 삭제했다. 전달할 새 Resources 파일은 없다. 같은 JSON의 암전 키 +0.299963초 이동(흡입 종료 시점 정렬)은 남아 있다.

검증: 변경 직전 백업과 JSON deep comparison에서 base 외 전부 동일, 47개 intensity/time 키 보존. 새 DDS는 DXGI 28 RGBA8, 2x2+1x1 mip, 모든 texel RGBA=255이며 SRGB loader 선택 후 RGBA8 SRGB도 현재 A coverage 허용 목록에 들어간다. Effect V2 _validate_authored 단계 253문서 PASS. 전체 binding validator PASS나 사용자 화면 PASS로 확대하지 않는다. 독립 검토도 동일 header/기본 coverage/loader/shader 관계를 확인했다.

보존된 밝기 곡선의 예상 alpha: 0초 1.0, 20.7초 약0.999604, 21.01초 약0.998945, 23.2초 약0.000710. 불투명 흰색과 black tint가 이 값을 그대로 검정 합성에 사용한다. C++/HLSL/프로젝트 등록 변경이 없으므로 빌드하지 않았다.

사용자 확인: 새 Client를 실행하거나 기존 Play를 Stop한 뒤 다시 시작하여 leaf snapshot/texture 객체를 새로 생성한다. F1 -> Sequencer Benchmark -> Gate 1 -> 1관문_통합_시퀀스, 0초와20.7초 암전, 23.2초 복귀를 확인한다. Play Sequence의 전투 연결 동작은 기존 그대로다. 실제 Client/UI 조작·캡처/GPU 표시 판정은 수행하지 않았다. 카메라 FOV 전환 결함과 포탈 정렬은 이번 수정 범위 밖이다.

## G12. 세 컷신 제작 계획 검증 — 구현 전 (2026-09-13)

### 확정 범위와 이번 결과

사용자가 계획 대상으로 `2관문_진입`, `3관문_진입`, `빙고_최종엔딩씬`만 지정했다. 추가 확인에서 3관문 진입은 현재 도착 부분만 있는 18,658ms가 아니라 **영상처럼 출발·비행부터 전체**를 포함하도록 확정했다. 대응 PLAN의 G12에 원본 연결 → P3 보완 → P7 전체 확장 → 신규 빙고 엔딩 → 수동 배치 보정 순서를 작성했다. 기존 P1/P2/P4/P5/P6는 수정 대상이 아니다.

전등·암전·재질 복구는 팀장 담당으로 제외했다. P7 앞부분 추가에 따른 기존 EFFECT/profile occurrence 네 박스의 +16,710ms 이동은 내부 문서 변경과 구분해 명시했다. 이번 결과는 **조사와 계획 작성**이며 C++/셰이더/제품 JSON/Resources 변경, publisher 실행, 빌드, Client 실행·조작·화면 캡처는 하지 않았다. G11 포탈/흡입 카메라 후보도 실제 설치하지 않고 보류한 상태다.

### 직접 확인한 근거

- 사용자 제공 영상 세 개를 각각 24개 시점, 총 72개 추출 프레임으로 읽었다. 실제 녹화 길이는 2관문 27.3333초, 3관문 36.4666초, 최종 엔딩 47.3333초이며 모두 30fps / 2160×1440이다. 녹화 컨테이너와 실제 게임 viewport를 구분했다. 이 프레임들은 원본 영상 분석 자료이며 구현 화면의 visual PASS 증거가 아니다.
- 현재 Sequence revision 14 / 7 Pattern을 확인했다. P3는 WORLD 17 / CAMERA 5 / EFFECT 3, P7은 WORLD 1 / CAMERA 8 / EFFECT 3 / profile 1이다. P5는 같은 SCENE02A 원본 전체의 WORLD 3 / CAMERA 18을 포함하는 읽기 전용 재사용 기준이다. 원본 Director의 13컷과 생성 후 18개 segment를 구분했다.
- SCENE04A Matinee2는 27,000ms, SCENE02A Matinee10은 35,368ms다. 빙고 공간 SCENE01B Matinee0의 InterpLength는 49.083335876초다. 녹화 길이에 맞추는 일괄 속도 변경은 계획하지 않았다.
- 원본 UPK에서 `37081_342 → SCENE01B Matinee export 32 → Data 45 → Completed/EndRemoteEvent 31`과 다른 공간의 `37081_341 → SCENE01C Matinee 31 → Data 44 → Completed/EndRemoteEvent 30` 연결을 재확인했다. 01B/01C는 각각 23 group, 동일 InterpLength지만 다른 공간이다. TriggerMapData의 52.13291초를 Matinee 길이로 오인하지 않는다.
- 빙고 엔딩의 source visibility 28,542 / 30,933 / 41,067ms, 첫 camera preroll, 숨김 중에도 진행되는 후반 배우 clip을 조사했다. A/B/C 슬롯과 무기 clip을 오프라인에서 해석하는 항목을 구현 선행 단계로 두었다. 아직 새 모델 bake나 슬롯 변환의 정확성 검증을 끝낸 것은 아니다.
- 현재 CameraShot은 up vector / Apply_PresentationPoseWithUp 경로를 지원한다. 이를 지원하지 않는다는 09-10 문서의 옛 설명을 새 계획에 적용하지 않았다. 기존 Object/Camera 편집 경로와 Sequence workspace의 Save 전용 게시 경계도 실제 호출자에서 확인했다.
- 현재 worldsequences.json 14,387,090 bytes / 16MiB, camerashots.json 1,336,647 bytes / 2MiB와 reader의 key/track/shot 제한을 확인했다. 기존 builder의 P4 41,488ms 설정과 사용자가 저장한 P4 58,810ms의 불일치 때문에 전체 빌더 재설치가 기존 편집을 덮어쓸 위험을 명시했다.

원본/코드 확인은 `C:/Users/USER/source/졸업팀폴/LostArk`의 현재 파일을 기준으로 했다. 이전 문서가 가리키는 out 폴더 중 현재 없는 경로는 새 실행 증거로 사용하지 않았다. 독립 검토의 지적을 재확인해 P3 표시명을 `2관문_진입`으로 맞추는 항목과 P7 기존 EFFECT/profile 박스 시각 이동표를 계획에 반영했다.

분석 스크립트와 영상 프레임, 문서 변경 전 사본은 `C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutscenePlan20260913/`에 있다. 검증용 중간 산출물이며 source commit 대상이 아니다. 문서 적용 전 현재 PLAN/RESULT가 시작 사본과 같은지 검사해 다른 세션의 저장값을 덮어쓰지 않는다.

### 구현 상태·다음 단계

계획 작성과 원본/현재 데이터 대조만 완료했다. 세 컷신 후보 생성, 모델/clip bake, 배치 반영, domain publisher/harness, 재생·저장·재로드와 사용자 화면 확인은 아직 수행하지 않았다. 데이터/오프라인 저작으로 먼저 진행하며 실제 runtime 수정 필요성이 확인되지 않는 한 C++/셰이더 재빌드를 선행 작업으로 잡지 않는다.

다음 구현은 P3의 기존 17 WORLD와 원본 대응을 보완하는 것부터 시작한다. 사용자 육안 확인 뒤 P7 출발·비행 전체, 신규 빙고 엔딩 순서로 진행하며 각 후보 설치 직전에 최신 저장값을 다시 확인한다. 제품 리소스 변경 전에 Save/종료를 조율한다. 자동 stage/commit/push는 하지 않았다.
