# 쿠크 원본 시퀀스 저작 연결 결과

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
