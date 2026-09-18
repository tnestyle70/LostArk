# 세이튼 모자 부착·재질 복구 결과

## G00. 구현과 저장 기준

`KAKULSAYDON_G1_PATTERN_83.logic.1`의 저장값을 확인했다. 시작 1573ms,
DURATION 5930ms이므로 오른손 모자 구간은 1573–7503ms다. 사용자의 최신 저장값을
보존하며, 앞선 왼손 언급이나 근사값 7500ms로 덮어쓰지 않는다.

`CNpcPresentationAssetService`에서 머리 모자를 준비·렌더하고 `CNpc`, `CPart_Body`,
`CWorldSequenceObject`가 기존 CModel 수명에 포함한다. 기존 BOSS WORLD anchor의
`b_wp_1`을 오른손 모자가 사용한다. 보이는 hand object와 실제 owner CModel instance를
weak registration으로 연결하여 같은 owner의 머리 모자만 숨긴다. 다른 세이튼 인스턴스는
영향을 받지 않는다. 종료·Stop·backward seek·실패로 hand object가 숨겨지거나 lease가
해제되면 머리 모자를 다시 제출한다.

새 C++ 파일을 만들지 않았으므로 project/filter 등록 변경은 없다. 총 5개 기존 CPP와
5개 기존 헤더를 확장했다. 기존 C++ UTF-8/BOM 없음/CRLF를 유지했다.

## G01. 원본 형상·재질·적용 모델

원본 head socket `wp_3_1`은 `bip001-head`, local TRS identity다. head 전용 모델은
`WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel`, hand 전용 모델은
`WP_MN_RPCT_08/wp_mn_rpct_08_sk.wmodel`이다. 두 모델 모두
`Character/KoukuSaton/` 아래 설치 모델이며 기존 native catalog material을 사용한다.

원본 PSK 각 523점과 설치 WModel 각 2614정점은 UV/normal seam 복제를 고려하여
대조했다. source cm에서 설치 geometry로 `[0.01, 0.01, -0.01]` 변환한 최대 최근접
정점 잔차는 hand `8.02e-9m`, head `1.46e-8m`다. 두 모델은 1mesh/4bone이고 자체
animation이 없다. WModel import basis 100은 모자 preScale 0.01로 한 번 제거한다.
세이튼 body preScale 0.017에 따른 bone basis 1.7은 머리 부착에서 유지한다.
WORLD anchor는 축을 정규화하므로 손 모자 resource scale은 `[1.7,1.7,1.7]`이다.

모자 material slot은 `mn_rpct_05-2_mi`, source는
`mn_rpct_05.mat.mn_rpct_05-2_mi`, family는
`source.character.monster-pbr-masked.v1`이다. 기존 native binding의 8개 texture가
실제 Resources에 존재한다. Npc의 `nativeBinaryBasePass`는 전용 binary shader의
binding 생략 경로를 고르는 값이다. false인 Part/Sequence 경로도
`Bind_DeferredMaterialInputs` 마지막의 `Bind_SourceCharacter`를 사용하므로 native
material을 소비한다.

| 대상 | 실제 판정 근거 | 결과 |
|---|---|---|
| G1/G3/BINGO 세이튼 | BossCatalog의 공통 RPCT05 body, `mn_rpct_05_mi`와 head/right/left bone | 머리 모자 준비 연결 |
| 앵콜을 외친 쿠크세이튼 | BossProfiles displayName → BINGO archetype → RPCT05 body | 같은 연결 사용 |
| 앵콜 시네마틱 group.51 | 원본 MN_RPCT_07, cinematic reference의 runtimeProfile MN_RPCT_05 | 같은 연결 사용 |
| 설치 sequence Saydon 7개 | 실제 WModel material `mn_rpct_05_mi`와 3개 필수 bone | 머리 모자 준비 대상 |
| 설치 sequence Kouku 3개 | bone은 있으나 material이 `mn_rpcz_00_mi` | 모자 대상에서 제외 |

sequence 7개에는 84bone LargeSaydon이 포함된다. 168bone 일반 세이튼과 별개이지만
실제 material과 head/hand bone 조건을 만족한다. 이 수치는 material과 bone 기준
적용 범위이며 모든 모델의 최종 화면을 확인한 결과가 아니다.

## G02. 후보 데이터와 게시 경계

`Tools/KoukuSaydonPipeline/prepare_saydon_hat_attachment.py`는 읽은 최신 저장본에서
WORLD object/template/instance, Composition WORLD definition/occurrence 후보만
만든다. live JSON이나 실행 중 메모리를 덮어쓰지 않는다. 후보는
`out/SaydonHatRestoration20260917/hat.patch.json`과 `candidate/`에 있다.

stable object ID는 `world.object.kouku.saydon_hat_right`다. Composition의 기존
P83 소스 재생 resource들에 별도 모자 particle이 남아 있지 않음을 확인했다.
원본 action4219806의 particle은 B_WP_2 왼손이며 HidePawn notify도 있지만,
그 payload를 현재 head visibility 계약의 원본 확정 근거로 사용하지 않았다.

통합 PRODUCT 검증에서 Logic81의 빈 judgementKind가 게시 불가 원인임을 확인했다.
기존 `ATTACHMENT_HOLD` kind를 사용한다. Publisher는 이 kind에 Collider/결과선을
금지하며 Server는 구간 시작·종료만 처리하고 전투 결과를 생성하지 않는다. 기존
capture 연결이 없는 이 occurrence는 플레이어 attachment도 만들지 않는다. 실제 모자
표현은 위 WORLD와 CModel 경로가 담당한다. 새 native/Server/schema 변경은 없다.
builder는 현재 kind가 없음/빈 문자열 또는 ATTACHMENT_HOLD일 때만 허용하고,
Logic81의 유일한 소비자가 P83.logic.1인지 검사한다. patch의 sourceLogicDefinition과
logicDefinitionBefore/After를 보존하여 상위 병합이 현재 필드를 대조하도록 했다.

기존 WORLD schema에는 Logic occurrence를 직접 참조하는 필드가 없다. 따라서 현재
저장된 DURATION 시작/길이를 WORLD에 복사한 것이다. 이후 Logic 시간만 수정하면
WORLD 시간도 같이 수정해야 한다. 이번 변경은 자동 동기화 schema를 도입하지 않는다.

최종 stable-ID 병합·revision/hash 재확인·backup·원자 교체·publish는 상위 통합 작업
소유다. 이 문서 작성 시점의 모자 agent 결과는 후보 준비이며 설치/게시 완료로
계산하지 않는다.

## G03. 검증과 남은 확인

변경 CPP 5개 `NpcPresentationAssetService`, `Npc`, `Part_Body`,
`WorldSequenceObject`, `WorldSequencePlayer_Objects`는 Debug x64 격리 compile을
통과했다. 기존 Engine header의 C4828 경고는 있었고 compile error는 없다.
Python 후보 builder 실행 및 생성 JSON parse, 변경 파일 `git diff --check`를 통과했다.

`geometry-material.receipt.json`은 위 설치 모델/원본 geometry/native texture 대조와
정확한 대상 material 분류를 보관한다. `hat_model_probe.exe`는 현재 production의
`Prepare_SaydonHat` 함수와 CActorCatalog를 사용하여 설치 CModel·native material·
실제 저장 clip의 bone basis를 확인하도록 compile/link했다. root의 첫 격리 실행은
catalog 준비 단계에서 실패했다. out 위치 EXE는 기본 Resources 경로가 실제 설치
폴더가 아니고, probe의 status 인자 평가 순서 때문에 실패 문구도 훼손됐다. 첫 실행
원본은 `run1/`에 보존했다. 실제 Data/Resources 환경을 고정하고 문구 평가 순서를
고친 v2를 다시 compile/link했다. root가 검토 후 `run_hat_probe_v2.py`를 1회 실행하여
exit 0, 2.108초, 입력 hash 불변으로 통과했다. 실제 head native program은 21,
body는 168bone이었다. 저장 clip 4개의 60Hz pose에서 head/right/left basis 1.7과
finite matrix를 확인했고, 실제 CModel head native material 준비를 통과했다.
42590회 assertion은 이런 수치·입력 확인의 수이지 화면 부착 성공 횟수가 아니다.
receipt는 `hat-execution-v2.json`, stdout은 `hat-model-v2.stdout.log`다. parent SetErrorMode
상속으로 DLL loader의 오류 창도 막고, 45초 제한·입력 SHA256 대조·실행 receipt 재사용
거부를 둔다. window/swapchain/draw/Client 실행은 없다.

모자 준비 실패는 body를 보존하고 모자만 생략한다. 모자 draw/bind 실패는 body draw
이후 발생하며 해당 owner Render가 E_FAIL을 반환한다. 이것을 모든 GPU 오류에서
무조건 몸체 Render 성공을 유지하는 계약으로 표현하지 않는다.

새 Client 실행 파일 설치와 Client/UI 실행은 하지 않았다. 사용자의 최종 확인 항목은
일반/앵콜/시퀀스의 머리 모자 크기·방향·재질, P83의 1573ms 오른손 전환과 7503ms
머리 복귀, seek/종료 후 중복 모자가 없는지다. 구조·수치 검증은 이 화면 확인을
대신하지 않는다.

## G04. 최종 설치·게시와 실제 WORLD 소비

상위 통합은 최종 저장 Composition1304를 다시 읽어1306, WorldSequence2051로 설치했다.
기존 Logic81 occurrence의 이름·1573ms 시작·5930ms duration은 보존하고 정의에
ATTACHMENT_HOLD 한 필드만 추가했다. 별도 새 WORLD resource/template/instance와
P83.world.8을 추가했으며 새 occurrence도1573~7503ms다. 다른 저장 항목과 사용자 피자는
보존했다. backup·fresh SHA·원자 교체·자기 변경 rollback을 가진 설치 절차와 독립 검토를
완료했다.

공식 KoukuSaydon owner의 네 domain 게시가 모두 exit0/PASS다. 게시된 Encounter P83의
worldSequences에 `world.object.instance.kouku.saydon_hat_right` 및 P83.world.8,
startMs1573/durationMs5930/playbackSpeed1이 존재한다. runtime World의 resource/template/
instance는 검증 후보와 동일하다. WORLD는 Server cue가 담당하므로 일반 Effect용
patternbindings.presentationOccurrences에 같은 모자 행이 없어도 누락이 아니다.

독립 끝단 검토에서 실제 Gameplay.bootstrap의 PATTERNWORLDSEQUENCE에도 같은 값이
있음을 확인했다. Server GameplayCatalog/LogicRuntime → S2C_WORLD_SEQUENCE_PLAY →
ClientReplication → Level의 owned WORLD cue → CWorldSequencePlayer Play/Seek/종료로
연결된다. Tool은 Composition WORLD occurrence를 직접 소비한다. 1573/5930ms는 저장·
preview 시간 계약이며 제품 Arena 전송 시점에는 기존30Hz Server tick 양자화가 적용된다.

증거는 `out/PizzaTrailHatIntegration20260917/installed-verification.json`이다. 설치 파일과
게시된 소비 데이터의 검증이며 실제 Client UI/화면 부착 검증은 아니다. Client/Server와
설치 EXE/DLL/CSO는 그대로 유지했다. 새 코드·셰이더를 함께 빌드하고 게시된 gameplay를
Server 재시작으로 읽은 후에 최종 화면을 확인한다.
