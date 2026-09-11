# 쿠크 V1 원본 소켓과 재생 모델 연결 결과

기준일 2026-09-11. 코드 반영, Python 검증, root 조율 Debug Product 컴파일 및 실제 Product 객체 파일을
사용한 두 문서의 CPU 파싱·재생 검증 완료다.
사용자 Client 실행/화면 검증은 수행하지 않았다.

## G00. 반영한 연결

| 범위 | 실제 소비 경로 |
|---|---|
| All Effects의 Play All / Current Effect Play / Timeline Solo | Effect Tool → Prepare_RecoveryPreviewTarget → Select_KoukuEffect → Composition V1 resource의 유일 pattern → 기존 ModelReferencePreview/CNpc/CModel |
| Effect Sequencer의 원본 follow socket | Resolve_AuthoringSourceAnchors → Resolve_KoukuSourceAnchors → 실제 선택 member의 typed model view → SourceAnchorWorlds |
| Sequencer seek / restart | 기존 60Hz history backfill에 Kouku model sampler 포함, 과거 sample 후 cursor 복구, callback의 useKouku 유지 |
| Composition Preview / Product | Spawn_LevelPlacement → Seek_WorldRoot fixed-step provider → 기존 CModel의 명시 clip/transition pose read-only sample → SourceAnchorWorlds |
| 파생 animation 정본 | projector sourceAnchorAnimations → Product reader의 typed AnimationOccurrences → 같은 pattern clock sampler |
| 반복 idle | LOOP_TO_WINDOW에서 playMs 강제 clamp 제거, source clip 끝을 반복하여 tail 창 동안 실제 pose 변화 유지 |

새 runtime이나 GameObject, .vcxproj/.filters 등록은 없다. source model-cue anchor는 기존 renderer가
소유하며 external source bone만 해당 provider가 채운다. SourceAnchorWorlds provider는 root-only
snapshot element의 시작/끝 시간이나 emitter lifetime을 변경하지 않는다.

## G01. 단위와 수명

원본 `source_socket_contract.json`의 MidControl/FX_Prj_01은 b_wp_1, FX_Prj_02는 bip001-head다.
원본 socket/particle local 값은 cm→m 0.01로 변환돼 있으며 G1 bodyModelPreScale은 0.017이다.
RawBone 3×3에 100을 곱하고 translation은 유지해 보스 mesh의 1.7 배율과 animated scale을 보존한다.
Artist용 rawbone 0.01 강제 검증을 적용하거나 GeometryPreScale metadata를 instance pre-transform으로
오인하지 않는다. SocketLocalTransform과 source orientation은 복원된 Effect document가 소유한다.

P29/P30의 V1 occurrence는 root 승인으로 followBoss=false다. source bone은 원본 clip timeline을
계속 샘플하고 occurrence root만 첫 위치에 고정한다. 이는 first Server snapshot 이전 actor world root를
위조하지 않는 제자리 audition 정책이다. 기존 28개 pattern은 deep equality로 보존했다.

Stop_Session은 각 v1 handle과 row sampler를 해제한다. standalone stop/restart는 기존 Sequencer의
anchor history/occurrence lifetime을 사용한다. read-only CModel 샘플러는 clip cursor, live bone palette와
blend clock을 변경하지 않는다. 실패 시 matrix output은 stage가 성공한 뒤에만 교체한다.

## G02. 검증 증거

- 새 stage boundary/단위 projection test와 기존 animation transition projection test 2개 PASS.
- 실제 rev323 P29 sourceAnchorAnimations 시작 시각 [0, 2000], duration 6582ms.
- 실제 rev323 P30 sourceAnchorAnimations 시작 시각 [0, 1667, 4834], duration 11075ms.
- 두 projection의 followBoss=false 확인, sourceStart/play/rate/endPolicy/blend 필드 보존.
- 기존 generic presentation test는 FEAR fixture가 presentation.17 Light resource를 제거해 실패한다.
  이 작업 전 dirty projector snapshot을 동일 입력으로 실행해 같은 실패를 재확인했다. 신규 회귀로 기록하지 않는다.
- 실제 JSON parse 및 바뀐 C++ 파일의 원래 UTF-8 개행 보존 확인. Effect_Tool.cpp는 다른 담당 최신 LF를
  그대로 유지하고 나머지 해당 C++는 원래 CRLF를 유지한다.
- git diff --check는 변경 소유 범위에서 오류 없음. root 조율 Debug Product compile/link/deploy PASS.
  최종 `out/BuildPipeline/runs/20260910T215447751Z-debug-product.json`의 Engine/Shared/Server/Client 모두
  PASS이며 missingRuntimeInputs는 비어 있다. 별도 CPU 검사는 G05의 같은 Product 객체 파일을 사용했다.

근거는 `out/KoukuSourceAnchors20260911/implementation.diff`, 각 파일의 `.before`,
`projected-animation-contract.json`에 남겼다. 이 diff는 다른 작업의 dirty baseline 이후 본 작업 변화만 나타낸다.

## G03. 남은 경계

사용자 화면 조작, 실제 CModel 본 행렬을 넣은 통합 재생과 fidelity 판정은
아직 이 RESULT에서 PASS로 기록하지 않는다. Product camera-view source attachment는 과거 camera
history가 없어 명시적으로 거절하며 현재 신규 두 패턴에는 해당 orientation이 없다. moving boss의
followBoss=true는 실제 과거 actor root history 범위 밖을 거절한다. 이 작업이 그 일반 이동 계약을
추가했다고 기록하지 않는다. standalone의 Play Family는 별도 기존 single-root flow이며 이번에 검증한
입력은 Play All/Current Effect Play/Timeline Solo다.

최소 새 compile 대상은 Engine Model.cpp, Client EffectAuthoringSequencer.cpp,
KoukuSaydonPresentationPlayer.cpp, Effect_Tool.cpp다. entry stutter 변경의 최소 대상은 별도 RESULT의
Level_Loading.cpp/Loader.cpp/Level_KakulSaydonArena.cpp이며 동일 root build에 포함한다.

## G04. Open Editor intent 보정

Kouku All Effects의 Open Editor는 기존 일반 authored/Sequencer intent인 SYNCHRONIZED_PRODUCT로
로드한다. Try_LoadDocumentPathStaged 진입에서 Kouku의 legacy STANDALONE_EFFECT 요청도 같은
intent로 정규화한다. pending Save/Discard 저장보다 먼저 처리하므로 Valtan static model 준비·reload·
restart 경로에 들어가지 않는다. Open은 문서 CPU 로드만 하며 명시 Play/Solo에서 원본 pattern 모델을 준비한다.

## G05. 실제 Product codec와 CPU 재생 검사

`out/KoukuSourceAnchors20260911/cpu_probe.cpp`와 `run_cpu_probe.ps1`은 Git 제외 임시 진단이다.
Product의 기존 13개 Client 객체 파일과 Engine/DirectXTK 라이브러리를 link했다. Client entry point,
Engine 초기화, window/device 생성, 화면 조작은 실행하지 않았다. provider의 root와 source slot은
명시적인 synthetic 숫자 입력이며 실제 CModel pose나 rendered fidelity의 증거가 아니다.

| 입력 문서 | 전체 C++ Parse/Stage | native material | 샘플 시각 수 | 관찰된 최대 생성 수 |
|---|---|---:|---:|---|
| 내려치기C 4219877 | PASS, authored v15 projection 활성 | 16/16 | 45 | particle 44, trail 2, baked edge pair 합계 38 |
| 불뿜기 4219801 | PASS, authored v13 일반 경로 | 55/55, 광원 2 예외 | 58 | particle 103, light 2 |

두 원본 문서 전체를 실제 `CEffectDocumentCodec::Parse`로 읽고 73개 요소를 검사했다. 0초, notify 시작,
각 emitter의 1/4·1/2·3/4 지속 시점, burst 직후, clip 경계, trail tail 중간·끝, pattern·playback 끝을
순서대로 seek했다. 모든 표본의 Seek와 행렬·입자·trail/light 값의 finite 검사를 통과했다. 마지막에
0초로 되감기, 필수 MidControl source slot 누락 거절, malformed parse의 기존 output 보존도 통과했다.
2개 baked trail 모두 각각 2개 이상의 edge pair를 실제 생성해야 PASS하도록 확인했다.

v15는 Catalog가 사용하는 `CEffectVisualProgramCorpusCodec::Create_DocumentOwnedRuntimeProjection`을
그대로 호출하고 같은 immutable document로 `Prepare_DocumentResources`를 준비한 뒤
`Stage_PrevalidatedVisualProgramDocument`를 호출했다. 제품 연결은 Effect_Catalog.cpp의 authored v15
projection 준비 → Effect_Object.cpp의 visual program stage → 같은 CEffectPlayback stage다.
첫 임시 probe의 `Stage_Document` 단독 호출은 이 projection을 빠뜨려 일반 trail Points만 생성했다.
그 중간 결과는 원본 baked-edge 기하 검증으로 인정하지 않고 `cpu_probe_result.before_product_projection.json`에
남겼다. 최종 결과는 projection과 두 원본 edge history 소비까지 포함한다.

검사에서 원본 baked-edge 절대 종료/상대 interval 혼용, 광원 template 지정, 외부 package의
Required/Lifetime/Spawn 참조 누락, 데칼의 비실행 metadata 혼입, 실제 데칼 roll 소비 계약,
두 trail의 SourcePresentation 활성화 누락을 확인해 각 소유자가 generator/해당 소비 경계에서 수정했다.
모르는 source literal이나 누락된 필수 모듈을 정상값으로 숨기지 않았다. 단요소 derivative 검사는 실패
수집에만 사용했으며 전체 문서 최종 PASS에 합산하지 않았다.

최종 `cpu_probe_result.json`의 원본 SHA256:

- 4219877: `d6c2d56ccb4647bf25a925ba4bc879257c6b7b3e34d71f3daa00fa3ef2c51e3d`
- 4219801: `d2e861c79c1417082f4df556734fe03db77f77a5ce765f9998243f953ba59ca6`

재현 명령은 `powershell -ExecutionPolicy Bypass -File out/KoukuSourceAnchors20260911/run_cpu_probe.ps1`이다.
compile/link/run 로그와 시각별 생성 수는 같은 out 폴더에 있다. native contract 통과는 GPU draw/픽셀 결과나
소스와의 최종 시각적 동등성을 승인하는 검사가 아니다.

임시 probe 컴파일 로그에는 기존 EngineSDK header 문자 인코딩 경고와 DirectXTK 디버그 PDB 누락,
EDITANDCONTINUE link 옵션 경고가 남는다. compile/link/run은 exit 0이며 이 작업에서 SDK/header를
재인코딩하거나 외부 라이브러리를 수정하지 않았다.
