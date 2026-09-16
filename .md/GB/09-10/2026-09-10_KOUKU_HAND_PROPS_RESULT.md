# 쿠크 손 소품 월드오브젝트와 패턴 연결 결과

## G00. 저장한 데이터

다른 진행 작업의 Composition240을 받아 기존 항목을 보존하고 revision241로 저장했다.
Area WorldSequence는 revision423에서424로 올렸다. 기존 Object19/Template149/Instance185와
기존 Composition world13개는 모두 보존했고 각 목록에4개씩 추가했다.
P21/P23은 WORLD occurrence와 next ordinal만 변경했다. 다른 작업의 P24·Logic38/39도 보존했다.
검증 근거와 초기 원본은 `out/KoukuHandProps20260910`에 있다.

| 표시 이름 | 부착 대상 | 실제 본 | 연결 시퀀스 |
|---|---|---|---|
| 월드오브젝트_쿠크트럼펫 | 2관문 Kouku | 오른손 b_wp_1 | P23 쿠크_팡파레, 0~10667ms |
| 월드오브젝트_쿠크레이저대포 | 2관문 Kouku | 오른손 b_wp_1 | P21 쿠크_레이저, 0~5833ms |
| 월드오브젝트_세이튼총_왼손 | 3관문 Saydon | b_wp_2 | Object/기본 상태만 준비 |
| 월드오브젝트_세이튼총_오른손 | 3관문 Saydon | b_wp_1 | Object/기본 상태만 준비 |

쇼타임은 원본 Action4219939의 Gun1/Gun2가 같은 StaticMesh를 양손에 붙인다.
미구현 쇼타임 패턴은 만들지 않았다. 생성 가능한 Object 정의와 미구현 패턴을 구분한다.

## G01. 리소스와 좌표

Resources 기준 새 cooked 모델3개, 총626,364 bytes를 설치했다.

- `Effect/KoukuSaydon/WorldObjects/Trumpet/Trumpet.wmodel`
- `Effect/KoukuSaydon/WorldObjects/LaserCannon/LaserCannon.wmodel`
- `Effect/KoukuSaydon/WorldObjects/SaydonShowtimeGun/SaydonShowtimeGun.wmodel`

기존 `fm_g_reup_01`, `fm_g_rhkp_06`, `wp_mn_rpct_07l_mesh`의 geometry를 보존하고,
원본 material의 D/N, D/N/S/E, D/N/S를 각각 CMaterial에 연결했다. 기존 Effect 모델과
texture9개는 수정하지 않았다. 세 모델은 Git 제외이며 Drive 업로드는 실행하지 않았다.
다른 PC에는 위 세 모델과 기존 texture 입력이 함께 있어야 한다.

prescale은 모두0.01, 소품 scale은 트럼펫2.5/대포2/총1.5다.
트럼펫·대포 local quaternion은 `[-.5,-.5,.5,.5]`, 총은
`[-.4156269364,-.4156269364,.5720614038,.5720614038]`, 총 local offset은 `[0,0,-.1]m`다.
원본 mesh의 UE→cooked 축과 skeletal socket의 local 축을 실측해 합성했다.
본 행렬의 위치는 유지하고 basis를 정규화하여 소품의 모델 단위가 중복 적용되지 않게 한다.
원작 PBR 전체식과 발광 flicker까지 복원했다고 판정하지 않는다.

## G02. 기존 런타임 확장

WorldSequence v3 부모 resource에 `anchorKind=BOSS`, `anchorBossArchetypeId`, `anchorBone`을 저장한다.
상태 instance는 `anchorKind=BOSS`로 부모의 actor/bone을 소비한다. Composition World의
`anchorKind=NONE`은 기존 고정 생성 위치 보정을 쓰지 않는다는 뜻이며 실제 손 추적은 WorldSequence가 소유한다.
`BOSS_SPAWN`은 계속 생성 당시 위치이므로 손 부착에 사용하지 않는다.

기존 CWorldSequencePlayer/CWorldSequenceObject/CModel/CMaterial 경로를 확장한다.
Tool에서 Boss/실제 actor/BODY bone을 편집하고, Product는 살아 있는 복제 보스,
Model View는 선택된 preview actor의 본을 매번 샘플한다. 없는 보스는 숨긴 채 대기하고
없는 본을 root로 대체하지 않는다. 종료·Reset은 기존 occurrence 정리를 사용한다.

## G03. 검증 상태

- 기존 Object/Template/Instance 및 변경 대상 밖 Composition 내용 보존: 확인.
- 변경 JSON parse와 등록 stable ID/실제 리소스 경로: 확인.
- 새3모델 converter info, geometry 원본 유지, material 입력9개 존재: 확인.
- Map publisher 기존 BOSS/비보스 유효·실패 fixture: 통과.
- WorldSequences 범위 publisher Validate: 통과 (`map-validate.log`).
- 실제 C++ Validate/Save/Load: BOSS actor/bone 왕복, 누락 actor·과도한 길이/제어문자 bone·anchor 불일치 거부, legacy 기본값과 실패 시 기존 document 보존 모두 통과 (`codec-build-run.log`, failures0).
- Debug Product Engine/Shared/Server/Client 빌드와 Composition revision243 게시: 완료. 다른 작업의 최종 통합에서도 P21/P23 연결을 보존했다. `out/BuildPipeline/runs/20260910T052159567Z-debug-product.json`, `out/KoukuPublishIntegration20260910/publish243.log` 참조.
- 원본/게시 WorldSequences revision424 JSON deep equality, Composition/Encounter/patternbindings revision243 일치: 확인.
- Client/UI 실행·조작·캡처 및 손 정렬·색·크기의 최종 화면 판정: 미실행, 사용자 확인 대상.

## G04. 사용자 검토 경로

현재 PC는 LAN server-host다. 최종 빌드 이후 Visual Studio `Server + Client` profile을
Ctrl+F5로 시작한다. Lobby → KoukuSaydon → F1 → KoukuSaydon Complete Play에서
Gate 2의 `쿠크_팡파레`, `쿠크_레이저`를 각각 선택해 Complete Play한다.

편집은 F1 → World Object Tool → Object Resources → Boss에서 위 이름을 선택한다.
Action Workbench → Resources → World → Append Object로 같은 정의를 다른 패턴에 추가할 수 있다.
부모 Object의 Boss actor/BODY bone과 기본 상태의 Transform을 확인한다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.

## G06. 09-14 쇼타임 Play와 총 Resource Preview의 actor 연결

사용자는 전날 위치를 맞춘 양손 총이 쇼타임 Play에서 보이지 않고 왼손 Resource Preview에는
`No KoukuSaydon composition preview is playing`이 뜬다고 보고했다. P35의 두 WORLD cue,
World 정의와 좌우 Object/template/instance는 HEAD와 같았다. 왼손 b_wp_2, 오른손 b_wp_1,
사용자 placement와 총구 Effect의 오른손 WORLD anchor/offset도 보존됐다.

실제 회귀는 single actor preview를 external WORLD로 전환한 뒤의 actor 전달 누락이다.
Bundle은 member.actor를 생성해 재생하지만 Level의 WORLD sampler는 별도 전역 Model View를
조회했다. Model View가 없거나 다르면 Apply_Objects가 총을 숨기고 회복 가능한 대기로 반환해
시간은 진행됐다. 이전 무기 교체 작업은 기본 무기 숨김을 다뤘으며 이 외부 WORLD의 actor
연결을 검증하지 못했다. 사용자의 화면 보고와 코드 재현을 구분하며 저장 앵커를 원인으로 바꾸지 않았다.

Level의 Debug_SampleCompositionWorldPreview에 optional 본 resolver를 추가했다. 외부 WORLD는
현재 단일 Bundle actor를 값으로 잡은 resolver를 해당 호출 동안만 사용한다. actor 애니메이션·
무기 pose 다음에 WORLD를 샘플하고 그 뒤 Effect가 현재 총 pivot을 사용한다. member1/offset0
제한과 Level의 팝업북 visibility·조명 정리는 유지한다. cue마다 resolver를 초기화해 이전 cue의
Model View가 새 cue에 누수되지 않으며 명시 override가 없으면 기존 Model View/복제 보스를 쓴다.

WORLD Resource와 박스 Preview는 저장된 BOSS anchor가 있으면 선택 Pattern의 정확한 보스
archetype/actor profile/gate를 검증하고 같은 단일 Bundle+Level WORLD 경로를 사용한다. 맵
WORLD는 기존 동작을 유지한다. 잘못된 보스 선택이나 서로 다른 보스가 섞인 요청은 이유를 남기고
새 미리보기를 시작하지 않는다. 별도 임시 모델 런타임이나 패턴 이름 하드코딩은 없다.

총의 회복 가능한 anchor 대기는 occurrence ID와 실제 이유를 Level→Presentation/MainApp→
Workbench 상태로 전달한다. inactive이며 pending도 없는 Animation Stop은 no-op으로 처리해
다른 Preview의 시작/실패 상태를 `No ... playing`으로 덮지 않는다. 실제 활성 Stop과 pending
취소는 기존 정리를 유지한다. 소스는 기존 UTF-8/BOM/CRLF를 보존했다.

관련 PresentationPlayer, Level, WorldObjects, MainApp, Animation_Tool_KoukuPlayback의 5 TU
Debug 격리 컴파일 PASS, git diff --check PASS. 정규 Product Engine/Shared/Server/Client
Build와 runtime DLL/셰이더 배포 PASS. 최종 근거는
`out/BuildPipeline/runs/20260914T040700948Z-debug-product.json`이다. 직전 동일 입력이 이미
컴파일돼 이 정규 실행은 OBJ/CSO/binary 재작성0으로 현재 제품 출력을 검증했다.
Client/Server 실행·UI 조작·화면 캡처는 하지 않았으며 실제 양손 렌더링/총구 정렬은 사용자 확인 대상이다.

사용자 질문의 총구 앵커는 `세이튼 BODY 본 → 총 WORLD occurrence → 총구 Effect`로 연결한다.
현재 P35.presentation.8/.9는 world.17/P35.world.2의 오른손 총을 follow하며 위치·회전 offset은
총 기준이다. Boss root만 고르면 손 애니메이션을 따라가지 못한다. 총구 섬광의 지속 추적과
발사 뒤 탄환이 월드에 남는 source local/world space 정책도 별개이며 이번에 바꾸지 않았다.

### G06 최종 집중 검사

실제 설치 MN_RPCT_05의 P35 16개 clip 시작/중간/끝과 양손 본 96개 sample에서 1078검사/실패0,
저장 placement 최대 오차2.38419e-7, actor root 이동 오차6.10352e-5를 확인했다. 실제 source
selector/lambda와 native CModel 두 본의 callback 검사1120개도 실패0이며 명시 actor 우선,
잘못된 archetype/null actor/transform/없는 본 거절, no-override legacy와 cue별 초기화를 포함한다.
MainApp actual actor helper 27개와 Stop 수정본32개 및 구 동작 비교29개가 통과했다.
숫자 anchor만으로 실제 본 연결을 대신하지 않았고 실제 Client/UI draw를 수행한 것은 아니다.

`out/ShowtimePreviewActor20260914/`의 `native_result.log`, `callback_result.log`,
`callback-source-order.json`, `agent-verification-receipt.json`을 근거로 사용한다.
최종 새 WorldSequence reader는 authoring/runtime1845 전체를 각각237templates/293instances로
읽고 서로 equivalent임을 확인했다(`latest-load.result.log`). Map scoped 최종 Publish/Check,
JSON/XML parse, 원본 공유 Effect와 대상 외 사용자 필드 보존, git diff --check도 PASS다.

## G07. 09-14 양손 발사 섬광과 한 손 총구 섬광

사용자는 G06 이후 총 Play와 이펙트 연결이 동작함을 확인했다. 이어 원본 발사 섬광을 총 WORLD에
연결하면 양손이 동시에 발사하고 위치 보정 후 회전할 때 총구에서 벗어나는 현상을 보고했다.
현재 저장 revision576의 P35.presentation.48/49/50이 이 조합이며 원본 BOSS용 양손 박스도 별도로 있다.

원본 `effect.kouku.gate3.showtime.gun.signature`는 b_wp_1/2 각각11개, 총22개 element이며
source attachment와 원본 notify 위치 [1.5,.5,0]m가 켜져 있다. WORLD anchor로 바꿔도 이 내부
부착은 제거되지 않는다. 현재 WORLD root 위에 원래 양손 본 변환이 합성되므로 총과 다른 위치가 된다.
사용자가 넣은 [.4,-2.7,-2.65]m는 총 local 좌표이며 실제 WORLD scale2.1을 적용받는다.
원본 내부 위치를 상쇄한 이 값을 한 손용으로 옮겨 유지하는 것도 맞지 않는다.

기존 `effect.kouku.gate3.showtime.gun.signature.world`는 한쪽11개만 포함하며 attachment를 끄고
element 위치·회전을0으로 정리한 총구용 리소스다. 새 asset을 만들지 않고 이 stable ID를 재사용한다.
원본과 WORLD용을 각각 `3관문_세이튼_쇼타임_발사 섬광 양 손`, `3관문_세이튼_쇼타임_발사 섬광 한 손`으로
구분하는 후보를 만들었다. 재생의 짧은 burst 보존 분기도 기존 ID 그대로 사용한다.

WORLD 샘플은 실제 총 객체의 Get_SampledWorld를 소비하며 actor pose → WORLD → Effect 순서다.
Effect offset은 anchor history 밖에서 매 샘플 적용되고 별도의 누적 drift 근거는 찾지 못했다.
한 손의 기존4개 local/7개 world-space 정책은 그대로 둔다. 이미 발생한 world-space 입자가
회전 뒤 남는 것은 총구 발생 원점이 벗어나는 문제와 다르다.

설치 SaydonShowtimeGun.wmodel의 실제 총구 disc72개 정점에서 local position
[.01913988293459018,-.024699029340667443,.9825197505950928]m와 rotation
[0,-89.3295806803671,-.8004327658390026]도를 다시 측정했다. coplanarity 오차는
.000208454cm다. 총 자체 본·placement와 발사 박스 시각·길이는 변경 후보에서도 보존했다.

### G07 검증 및 적용 상태

- 양손/한 손 요소 수, 내부 attachment와 TRS 차이, 재질·recipe·시간·scale 보존51검사 PASS.
- 기존 headless codec/짧은 burst 재생 검사70,191개 PASS. 100/150/200ms step에서 양손12행,
  한 손6행을 유지하며 source 시계와 snapshot을 보존했다. Client/UI나 GPU 화면 검사는 아니다.
- 후보 Composition의 변경 필드 외 deep equality PASS. 정규 prepare_publication 경로에서
 57개 저장 Pattern 중46개가 게시 가능하며 P35의 unavailableReason은 비어 있다.
- 원본 라이브러리의 빈 P32는 직접 validate_document 전체 호출 시 lifetime0으로 거절되므로,
  저장 draft와 게시용 dependency closure의 검증 진입점을 구분했다. 후보와 원본의 동일한 기존 상태다.
- `prepare_showtime_anchor_candidates.py`의 한 손 표시 이름은 요청한 이름으로 갱신했다.
- 사용자가 Save 후 EXE 종료를 알렸고 Server/Client 모두 종료된 것을 확인했다. 마지막 원본은
  revision576이었다. 두 이름과 명시 WORLD 연결3개를 revision577로 설치하고, 추가 요청인
  부채꼴 예고의 신규 박스 기본 길이4000→1500ms만 변경해 최종 revision578로 저장했다.
- 사용자는 프레임 단위 확인 후 실제 발사는 양손 동시가 맞다고 정정했다. 한 손 리소스를
  좌우 총에 각각 연결하고 같은 시각의 두 Effect를 그룹 복제하는 저작 방향을 확인했다.
  현재 오른 총3개를 교정했으며 사용자의 기존 BOSS 양손 박스·시각은 보존했다. 좌우 발사
  시각을 임의 교차시키거나 새로운 발사 박스를 추가하지 않았다.
- 정규 Debug Product Engine/Shared/Server/Client 빌드·링크·배포 PASS. 최종 근거는
  `out/BuildPipeline/runs/20260914T061704289Z-debug-product.json`이다. Effect 그룹 소스가
  집중 검사 때와 동일함을 최종 hash로 확인했다. 이후 부채꼴 작업은 JSON만 변경했다.
- `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 578`
  의 Product/Map/World/Gameplay 네 domain 게시 PASS. `publish578.log`에46개 Product Pattern,
  57개 저장 Pattern,8개 Product Bundle과 최종 sourceRevision578을 기록했다.
- JSON/XML/Python parse, 변경 대상 외 사용자 데이터 deep equality와 git diff 검사 PASS.
  Server/Client는 종료 상태이며 에이전트가 실행·조작·화면 캡처하지 않았다.

근거는 `out/ShowtimeSingleHand20260914/asset-inspection.json`, `staged-native-check.log`,
`measured-muzzle.json`, `integration/receipt.json`, `integration/publication-check.json`이다.
최종 적용 검사는 `out/ShowtimeSingleHand20260914/final-receipt.json`을 따른다. 사용자는 새
Server/Client를 시작하고 P35에서 `발사 섬광 한 손`의 Anchor World/Follow world object를 선택해
왼 총 world.16/P35.world.1 또는 오른 총 world.17/P35.world.2를 연결한다. 위 총구 local position/
rotation과 Follow anchor를 사용하고 두 박스를 같은 시각에 배치한다. Ctrl 선택 → Box Detail의
Set Group → Duplicate로 다음 발사 묶음을 만든다. 그룹 복제 시 총 WORLD 박스 수는 그대로다.
기존 총 Play 확인과 이번 수정 후 최종 총구 위치·실제 드래그·부채꼴 화면 판정은 구분한다.

## G08. WORLD 기본 Effect Append의 필수 ID 누락

한 손 리소스에는 기본 anchorKind=WORLD만 있고 특정 worldId는 없다. Boss Workbench Append가 이를 그대로 occurrence에 복사하여 `World Object anchor requires a worldId`로 거절했다. Sequence 경로에서 사용하던 고정 MAP 배치를 Boss의 WORLD 기본 Effect에도 적용했다. 새 박스는 현재 보스 spawn에서 시작하고 Box Detail에서 실제 총을 연결한다. BOSS/MAP 기본값과 이미 저장한 총 부착은 보존하며 WORLD의 필수 ID 검증은 유지한다.

같은 실제 native Append 테스트를 수정 전 Workbench object에 연결했을 때 해당 오류로 실패했고 수정 후 통과했다. V1_EFFECT/V1_ELEMENT의 추가, 한 번의 candidate commit, Save/Reopen, 명시 총 연결, Effect 복제 시 총 수 보존을 확인했다. WORLD ID를 비운 직접 편집은 계속 거절되며 draft가 유지된다. 테스트의 World Load/Find 대체는 지정된 fixture 경로·Area·placement만 허용하고 다른 접근은 실패한다.

근거는 `out/KoukuEffectAppend20260914/append-contract-receipt.json`, `baseline-native.log`, `candidate-native.log`, `build.log`다. 관련 격리 Debug 컴파일·링크는 통과했다. 사용자 JSON은 변경하지 않았다. 이번 코드의 제품 EXE 반영과 Client 입력 확인은 아직 수행하지 않았으며 G05/G06 Effect Tool 후속 수정과 함께 최종 빌드한다.


### 2026-09-14 후속 실행파일 확인

앞선 제품 빌드 대기는 사용자 Visual Studio 빌드로 해소됐다.16:54:34 Client.exe의 실제
compiler dependency·OBJ·link 입력을 현재9개 관련TU와 대조했고 현재 Shift 선택,
Effect anchor 그룹, saved prop preview, WORLD Append 변경 포함을 확인했다.
17:18:47 사용자 증분 Build 로그도 성공이다. 에이전트의17:19 Product 재빌드는 사용자
재실행으로 output guard가 거절했으므로 그 실행을 PASS로 기록하지 않는다.
증거는 `out/KoukuFlameUnification20260914/user-build-verification.json`과
09-12 KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT의G15-02다.
사용자는 시퀀스 재생을 확인했으며 각 이펙트의 최종 시각 판정은 별도 사용자 확인 범위다.

## G09. 09-15 작은 쿠크 바주카 — 저작본 등록

원본 PS `FX_MN_RPCT_07_V.Par_V_RPCT_Bazooka_mesh_01`은
`Bip002-R-Hand`에 `fm_g_rhkp_06`을 붙인다. 설치 MN_RPCT_05에서 정확한
본은 `bip002-r-hand`이며 큰 세이튼의 b_wp_1과 다르다. 원본 MIC는 기존
2관문 레이저와 같은 `wp_mn_rhkp_06.mat.wp_mn_rhkp_06_mi_dead`다.
FullRestore mesh와 기존 LaserCannon의5485개 XYZ 및21108 indices를 대조해
같은 geometry임을 확인했다. 기존 G2의 모델·재질·calibrated quaternion을
재사용하며, 이 quaternion의 G3 최종 손 정렬은 사용자 화면 확인 대상이다.

`out/KoukuBazookaBombInstall20260915/receipt.json`은 최신 저장 Composition815를
기준으로 816을 설치했고 World1848을 1849로 올렸다. P35 원본 notify12개를
사용자의 최신 애니메이션 편집에 다시 결합하여 아래 5구간을 추가했다.

| WORLD occurrence | 시작–끝(ms) |
|---|---:|
| KAKULSAYDON_G1_PATTERN_35.world.3 | 1024–4100 |
| KAKULSAYDON_G1_PATTERN_35.world.4 | 5500–18031 |
| KAKULSAYDON_G1_PATTERN_35.world.5 | 18122–22864 |
| KAKULSAYDON_G1_PATTERN_35.world.6 | 22955–33263 |
| KAKULSAYDON_G1_PATTERN_35.world.7 | 37672–51004 |

이전 후보의 4구간은 복사하지 않았다. resource는 `kakulsaydon.g1.world.23`,
object는 `world.object.kouku.g3.bazooka`다. 기존 62 Patterns의 편집과
181 presentationResources, 기존 양손 총 placement를 모두 보존했다.
World resource의 G3 actor/bone, 실제 Level의 replicated-boss resolver와
단일/다중 Bundle actor resolver가 같은 기존 런타임을 소비함을 확인했다.

실제 CModel의 P35 16clip/48pose와 현재 WorldSequence 함수602검사에서 실패0,
Bip002 basis1.69999–1.7, pivot 오차0, root 이동 오차4.76837e-7이다.
source4 × notify.3000000119 × owner1.7 =2.040000081을 적용한다.
현재 저장본의 전체 publisher에서 P35 unavailableReason은 비어 있다.

사용자는 저장 완료와 등록 동안 편집 중지를 명시했다. 정확한 Client41664/Server29600
기동 시각을 기록한 task 전용 허용, Composition writer lock, 기준/후보/백업 hash와
각 파일 commit 직전 CAS를 적용했다. 8파일 등록과 Map WorldSequences publish/check,
Kouku Composition publish/check를 완료했다. JSON6/XML2 parse와 최종 설치 31검사에서
실패0이다. 기존 renderer 수치 검사를 불필요하게 재실행하지 않았다.

현재 Client Level은 진입 때 읽은 WorldSequence 문서를 보유하므로 외부 게시 뒤에는
쿠크 아레나에 다시 입장하고 Composition `Reload` 후 P35를 새로 재생한다.
Source Preview와 Complete Play의 실제 사용자 화면 판정은 남아 있다. Client/UI
실행·조작·캡처와 EXE 교체·재빌드는 수행하지 않았다.

기존 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`을 expected revision816으로
완료했다. product/map/world.gameplay/gameplay.balance 4도메인 PASS이며 실제
Gameplay.bootstrap의 `KOUKUSAYDONPRODUCTREVISION`은816이다. 새 Complete Play는
기존 Server의 Kouku generation admission으로 적용되므로 이번 Client 소품 등록에
Server 재시작은 필요 없다. 진행 중 재생은 Stop 후 새 Play해야 하며 Restart는
이전 pinned revision을 유지한다. 게시 후 최종 31검사와 관련 diff--check도 성공했다.
