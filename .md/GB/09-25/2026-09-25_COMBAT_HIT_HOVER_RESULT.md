# 전투 피격·호버 외곽선과 잔상 알파 조정 결과

## G00. 현재 상태

기존 `codex/kouku-timeline-local-preview`의 다른 작업을 보존하며 원본 재감사와 전투 표현을
반영했다. 최종 정상 Debug Product Build는 PASS(36.251초)이고 설치72 CSO 일치,
공통 피격·호버12 draw 및 잔상9 draw의 GPU 검증을 통과했다. 현재 결과의 정본은 G09~G12다.
G01~G08의 파일 점유·빌드 대기는 이전 단계의 이력이며 현재 미빌드 상태를 뜻하지 않는다.

지원 모델의 원본 Hit_Color, 확인된 호버 빨강, 돌진/이동 잔상과 실제 피격 동작의 원본
피해 음성을 연결했다. 원본 PPOutline 전체와 generic 대상의 native 재질까지 모두 복구한 것은
아니다. G11의 native24/generic13 분류 및 미복구 CPU/재질 경계를 유지한다.
Client/UI 실행·사용자 화면 판정·청취는 수행하지 않았다. 자동 stage/commit/push도 하지 않았다.

## G01. 원본 재질 조사

설치 원본 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`를 기존
UE3 decoder로 읽었다. 대상마다 별도의 노란 particle가 있어야 하는 구조가 아니라 공통
몬스터 parent material의 `hit_color` 상태 입력과 rim 연산이 존재한다.

| 대상 | 원본 모델/재질 계열 |
|---|---|
| 세이튼·쿠크세이튼·앵콜세이튼 | MN_RPCT_05 |
| 쿠크 | MN_RPCZ_00 |
| 발탄 | MN_RPBF_01 |
| 괴기스러운 인형 | MN_CDMD_00 |
| 광대 얼굴 공 | MN_RHCN_00 |
| 카드미로 중앙 파괴 상자 | MN_RPPB_01 |
| 발탄 스폰 몬스터·루가루 | MN_PADD_01, MN_SJFC_00/-4, MN_0019_05, MN_RPRS_02 |

고급 재질은 `monster_base_msk_high → pbr_base_msk`, PADD/SJFC는 legacy
`monster_base_msk/opa`를 상속한다. 큰 세이튼 MN_RPCT_06은 MN_RPCT_05의 MIC를 import한다.
원본 parent에 `hit_color`, `1.use_hitcolordirection`, `constantoutline` 계열 입력이 있다.
현재 SourceCharacter program21/26도 1.5-power rim에 Hit_Color RGB를 곱해 더한다.
`selectioncolor`는 최종 RGB tint이므로 빨간 바깥 경계와 같은 기능으로 간주하지 않는다.

현재 원본 자료로 CPU가 설정했던 정확한 황색·수명·발동 조건까지는 확인할 수 없다.
이번 색·강도·시간은 현재 프로젝트의 Server 피해 이벤트와 기존 피격 시계에 연결하는
표현이며, 원작 런타임 전체를 동일하게 복원했다고 기록하지 않는다.

## G02. 일반 WorldEntity와 특수 WORLD 오브젝트

CNpc 기반 쿠크 보스가 기존 BOSS→CValtan 전용 피격 호출에서 빠져 있음을 확인했다.
일반 몬스터와 발탄은 기존 흰색 emissive rim을 가지고 있었다. 피격은 outgoing의 양수
NORMAL/CRITICAL/ABSORB 이벤트만 소비하며 HEAL/MISS/INVINCIBLE·카드 문양 획득 표시는
제외한다. 원본 diffuse/native 재질을 공유 상태로 변경하지 않는다.

공과 괴기스러운 인형은 Server WORLD_OBJECT이며 일반 WorldEntity snapshot에서 제외되고
owned WORLD cue로 표시된다. 따라서 일반 몬스터 분기만 수정해서 전체 적용으로 보고할 수
없다. 기존 sequence PLAY에 Server combat body ID를 추가하고 cue 모델에 연결했다.
protocol은 112이며 Writer/Reader와 late join도 함께 변경했다. 살아 있는 body만 한 번
재전송하고 일반 WorldEntity spawn을 중복 추가하지 않는다. STOP·death·pool reuse에
이전 hit/hover 상태가 남지 않게 정리한다. sequence 샘플링 중 임시 Hide는 피격 시계를
초기화하지 않고, 실제 비가시 상태의 Late_Update와 Reset_ForReuse에서 초기화한다.
카드미로 중앙 상자는 `MONSTER_KOUKU_CLOWN_BOX`로 일반 몬스터 경로를 사용한다.

노란 피격은 RGB(1, 0.72, 0.08), peak intensity 4, 0.12초 감쇠와 1.5-power rim으로
표현한다. 몸체·무기·모자·갑옷의 공통 transient 입력을 사용한다. 대상 WORLD resource는
현재 odd_doll/odd_doll.large/mario_circus_ball이며 각각 OBJECT 모델 binding 하나다.
불꽃·아우라 EFFECT decoration에는 이 강조를 전파하지 않는다.

2관문 대형 세이튼은 Shared의 stable archetype predicate와 Server의 공통 대상 필터에서
제외했다. 최종 피해 적용 및 스킬·전투 오브젝트 후보가 같은 필터를 사용하며 HP·무력화·
카운터·파괴·이벤트·maximumTargets를 소비하지 않는다. spawn·패턴·몸체 충돌은 유지한다.

## G03. 현재 포즈의 호버 경계

raw vertex에 preScale만 곱한 기존 bind bounds는 Valtan 약0.029m, Saydon 약0.045m로
실제 골격 basis를 반영하지 않는다. 반면 MN_RPPB_01은 약1.25×1.85×1.04m이며 실제로
63bones·12animations의 animated WModel이다. static prop으로 간주하지 않는다.
Valtan의 presentation root는 body local −90도 yaw와 actor transform을 올바르게 포함한다.
문제는 root가 아니라 skin palette를 적용하지 않은 raw bounds였다.

Engine의 현재 pose bounds와 실제 presentation root를 사용해 가장 가까운 대상 하나를
선택한다. 빨간 외곽선은 해당 모델의 현재 pose·coverage와 scene depth를 사용한다.
기존 pass index를 보존하고 static 28/29, skinned 18/19를 뒤에 추가했다. 약 2 physical
pixel 폭이며 depth read-only, stencil 미사용이다. reflected root winding과 Valtan ghost의
native opaque coverage도 구분한다. UI 마우스 소비·free camera·연출·모달·연결 종료에는
선택을 해제한다. 피격 시계가 끝나도 호버 상태는 독립적으로 유지한다.
화면 픽셀 단위의 실루엣 picking이 아니라 골격 포즈의 보수적 경계 picking이며,
분리 무기는 body picking 경계에 포함하지 않는다. 실제 화면에서 커서 감도와 외곽선
품질은 사용자 확인 대상으로 남긴다.

## G04. 잔상 알파

사용자 후속 요청은 과거 원본 대비 배율이 아니라 현재값의2배다.
발탄 대시0.38→0.76, 쿠크 대시0.19→0.38, 쿠크 카운터0.35→0.70으로 조정했다.
수명·간격·RGB와 끝점 fade는 유지한다. 별도 backstep과 카드 particle는 변경 대상이 아니다.
현재 발탄에는 별도 counter 모델 잔상 consumer가 없으므로 새 기능을 임의로 추가하지 않는다.

## G05. 검증 기록

- Engine Model/Mesh 2 TU의 격리 Debug 컴파일 성공. `out/CombatHitHover20260925/pose-bounds-compile.json`.
- 수정 Client 14 TU의 격리 Debug 컴파일 성공. 새 Engine public header를 직접 참조했으며
  Product SDK 배포·link 성공을 대신하지 않는다. `out/CombatHitHover20260925/client-compile.json`.
- 수정 Server 8 TU·현재 Shared 7 TU·NetworkProtocolHarness 1 TU 격리 Debug 컴파일 성공,
  경고 0. `out/CombatHitHover20260925/server-wire/compile-results.json` 및
  `harness-recompile-results.json`.
- 현재 Shared 소스 obj를 직접 링크한 NetworkProtocolHarness `--world-motion-only`
  43 PASS, exit 0. body ID roundtrip·truncation 시 기존 출력 보존·owner collision 및 기존
  placement malformed를 확인했다. protocol 고정 assertion도 112로 갱신했다.
  `out/CombatHitHover20260925/server-wire/harness-results.json`, `harness-world-motion.log`.
- 설치 모델 28 archetype, rest/실제 animation 140 pose, GPU와 동일한 skin 식으로 전체
  정점 5,469,330개를 비교했다. bounds 이탈·비유한 값 0, 최악 axis 여유 1.542배.
  `out/CombatHitHover20260925/pose-bounds-numerical.json`.
- 격리 FXC SourceGroup017 static/skinned 및 SourceGroup084 skinned 컴파일 성공.
  기존 원본 함수의 X4000 경고는 남는다. WARP actual draw에서 두 모델
  경로 각각 빨간 border 240 pixel, 내부 0, 앞 물체의 depth에 가려진 draw 0, alpha 0인
  mask draw 0, 노란 hit radiance 양수, override 해제 후 0을 확인했다. 총 12 draw이며
  Client 창·GPU 화면 캡처를 사용하지 않았다. 전체 설치 cohort 검사는 Product Build 뒤 남는다.
  `out/CombatTargetPresentation/combat-draw-probe.result.json`, `combat-draw-probe.log`와
  `commands.txt`에 범위·출력·명령을 보존했다.
- 변경 파일 `git diff --check` 통과. 본 기능은 새 C++/프로젝트 항목·JSON 게시를 만들지 않는다.

## G06. 남은 제품 검증과 사용자 확인

실행 중인 Debug Client/Server의 파일 점유와 소스 변경을 구분한다. 현재 프로세스의 메모리나
설치 EXE/CSO가 새 코드로 갱신됐다고 보고하지 않는다. Client 28640/53208과 Server 276의
저장·종료 후 아래 검사를 정상 경로로 수행해야 한다.

1. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`의 Product 증분 Build로
   Engine→SDK→Shared/Server/Client와 model cohort CSO를 함께 반영한다.
2. Server SkillStages와 KoukuProduct 집중 contract를 실행한다. 대형 세이튼이 더 가까워도
   maxTargets=1이 작은 쿠크를 적중하는 경우 및 body
   PLAY→late join→cancel/death의 수명을 검사한다.
3. `Tools/RenderingPipeline/Test-SourceCharacterShaderVariants.ps1 -Configuration Debug`로
   실제 설치 cohort의 admission·pass state·actual draw를 확인한다.
4. 사용자가 Lobby를 통해 Valtan/Kouku에 진입해 요청 대상별 피격·호버와 잔상 알파를
   확인한다. 큰 세이튼은 피격·호버 대상에서 제외되어야 한다. 렌더링 옵션은 변경하지 않았다.

기존 다수의 미커밋 변경과 산출물을 섞지 않기 위해 자동 stage/commit/push는 하지 않았다.

## G07. 설치 Debug 셰이더 검증

정상 Debug Product Build receipt `out/BuildPipeline/runs/20260925T092422866Z-debug-product.json`
완료 뒤 기존 `Tools/RenderingPipeline/Test-SourceCharacterShaderVariants.ps1 -Configuration Debug
-OutputDirectory out/CombatHitHover20260925/product-shader-variants`를 한 번 실행해 exit 0을 확인했다.
G05/G06에서 남겨 둔 설치 cohort 검사는 이 결과로 완료하며, 사용자 화면 확인은 별도로 남는다.

- static/skinned/deferred 3 family마다 등록된 23개 cohort를 CShader 생성 시 모두 로드했다.
  admission은 pass 수·이름·입력 signature·base/variant pass 정책과 상수·resource 변수의
  타입·배열 길이·크기 ABI 일치를 뜻한다. 69 cohort CSO와 base 3개, 총 72 CSO의
  격리본 SHA256이 설치본과 모두 일치했다.
- 능동 source binding은 program 1–32와 80–89의 42개 × 3 family = 126개다.
  clone 6, 실패 입력 9, light pass 504, native cue 조합 144, afterimage 4,
  unavailable pass 10, combat hover pass 12 검사를 통과했다.
- 공통 slab의 실제 WARP draw는 총 12회, windowsCreated=0이다. skinned pass 18과
  static pass 28의 빨간 border는 각각 240 pixel, 내부 덮어쓰기 0이었다.
  depth 차폐·alpha mask의 출력 0, 유한한 양수 피격 radiance와 override 해제 뒤 출력 0을 확인했다.

모든 native 재질을 실제 draw한 검사는 아니다. Guardian program 200과 Dimension program 902의
512-material binding 및 Movie 검사는 별도
`2026-09-25_FOUR_CLASS_SELECTION_MOVIES_IMPLEMENTATION_RESULT.md`의 G09 이후 근거를 따른다.
최종 게임 화면의 색·감도·실루엣 품질은 이 수치 검사로 대신 판정하지 않는다.
실행 명령·범위·입력 hash·수치는 `out/CombatHitHover20260925/product-shader-variants/result.json`,
`command.txt`, `probe.log`, `compile.log`에 보존했다. 제품 소스 변경과 Client/UI 실행은 없었다.

## G08. 사용자 종료 후 최종 Debug 제품 빌드와 Server 검사

사용자가 실행 파일 종료를 알린 뒤 정상 Product Build를 완료했다. G06의 파일 점유 상태와
미반영 설명은 당시 기록이며, 현재 제품 완료 상태는 이 항목으로 갱신한다.
`out/BuildPipeline/runs/20260925T092422866Z-debug-product.json`은 Engine→Shared→Server→Client
전체 PASS(약 26분 59초), Client model CSO 48개와 제품 바이너리 배포를 기록한다.
마지막 수정분을 반영한 정상 증분 Build도
`out/BuildPipeline/runs/20260925T092644071Z-debug-product.json`에서 PASS(40.025초)다.
이 증분은 Server 7 TU, Client 6 TU를 갱신하고 Engine/Shared 및 CSO는 재생성하지 않았다.
기존 FXC/C4819/수치 변환/DirectXTK PDB 경고는 남으며 경고 0 빌드라고 기록하지 않는다.

새 `Server/Bin/Debug/Server.exe`의 `--skill-stages-contract-test`와
`--kouku-product-contract-test`는 각각 약 6초/327초, exit 0, failures 0으로 통과했다.
대형 세이튼의 직접 피해·shield·stagger·counter·event 거절, caster/projectile/contact/fallback의
대상 수 소비 전 제외, maxTargets=1에서 작은 쿠크 적중을 확인했다. WORLD body ID의 정확한
PLAY 연결, late join의 단일 cue와 중복 spawn 없음, cancel/death 후 재생성 방지도 통과했다.
최초 late join fixture의 4개 실패는 active gameplay revision pin 없이 구성한 시험 입력의
admission 실패였다. 실제 run 계약에 맞는 pin을 fixture에 추가하고 새 Server로 전체 검사를
다시 통과했다. 기존 실패 로그는 `kouku-product-before-fixture-pin.log`에 보존했다.

실행 결과는 `out/CombatHitHover20260925/product-contracts/result.json` 및 두 log,
최종 Client/Server/Engine SHA256은 `out/CombatHitHover20260925/final-product-artifacts.json`에
기록했다. Client 배포 Engine.dll과 Engine 원본 산출물 hash도 같다.
프로젝트/filters XML parse와 전체 `git diff --check`를 통과했다. Client/UI는 자동 실행하지
않았으며 사용자 화면에서 대상별 색·외곽선 감도와 잔상 인상을 확인하는 경계는 유지한다.
실행하려면 새 Debug Client를 사용하고, 공유 Server도 protocol 112가 포함된 새 빌드여야 한다.

## G09. 원본 TrailGhost 재감사와 대시 잔상의 수치·수명 연결

현재 설치 KR `EFGame/data3.lpk`에서 RPBF 3종, RPCT 5종, RPCZ 2종의 Action LOA
10개를 직접 추출했다. 원본 archive entry offset/length와 LOA SHA256은
`out/CombatSourceAudit20260925/afterimages/source-extraction.json`에 있다.
TrailGhost 523개 중 source enabled 439개, 별도 CounterAttack notify 487개를 확인했다.
기존 exact decoder가 읽은 TrailGhost는 514개이며 9개는 trailing ABI mismatch로
격리했다. 추가 payload를 잘라 성공으로 만들거나 source disabled notify를 켜지 않았다.
전체 raw payload/enable/clip/시계는 `all-trailghost-counter-notifies.json`,
EFGame native class reflection은 `efgame-reflection.json`, 요약은 `SOURCE_AUDIT.md`와
`source-audit-summary.json`에 둔다. 설치 원본과 Data authoring은 수정하지 않았다.

발탄 420604/003 Att_Battle_4_01은 source 2.461735964초부터 0.6초 발생,
0.1초 간격·child 0.4초·initialAlpha .5·alpha hold .1초·diffuse intensity .8이다.
현재 Server 승인 dash active의 sourceStart 2.45초/playRate .6에 맞춰 시작과 종료를
모두 제한한다. Body_Valtan은 이 값을 기존 CSkeletalAfterimage에 전달하며
원본 alpha .5와 userAlphaScale 1.52를 분리해 기존 사용자 peak .76을 보존한다.
별도 stage/공격 전체로 activation을 확대하지 않는다.

Saydon 4219863의 22_01은 209.0039998–259.0039998ms, interval .1/life .6,
alpha .8·diffuse .6이고 22_04는 198.0549991–248.0549991ms, interval .005/life .5,
alpha 1·diffuse 1이다. 각각 userAlphaScale .475/.38로 기존 peak .38을 유지한다.
RPCT07 4219951 및 4219922/4219923/4219964의 정확한 source stage·clip 34_03/34_04는
0–50ms, interval .005/life .5, alpha 1·diffuse 1과 기존 peak .19를 사용한다.
원본과 무관한 같은 이름 clip에는 적용하지 않는다. 제품·child·Preview가 같은 helper를
소비하며 sourceStart/playRate/trim과 현재 stage를 함께 검사한다.

CSkeletalAfterimage의 각 child는 태어날 때의 settings를 보관한다. 다음 notify가
기존 child의 색·알파·수명을 다시 쓰지 않는다. interval보다 짧은 50ms emission에서도
첫 실제 pose를 즉시 저장하고, 정상 연속 시계가 150ms 이내에 발생창을 통째로 넘으면
현재 pose 한 장을 전달한다. Seek·pause·rewind·큰 점프의 과거 pose를 만들어내지 않는다.
5ms 원본 간격보다 frame이 길어도 미관측 subframe pose를 복제하지 않는다.
실패한 pose, teleport, 숨김, 명시 reset은 기존 격리 경계를 유지한다.

원본 숫자와 재현 수식은 구분한다. Source 색 RGB와 ambient를 별도 shader 입력으로
보내고 `diffuse*sourceIntensity + ambient + rimColor*viewRim²`를 계산한다.
Alpha hold 뒤 남은 수명에 제곱 fade를 쓰는 정책, 색 선형 보간, 원본 NONE part의 기존
body/weapon coverage는 PROJECT_RECONSTRUCTED다. native material/fade code를 회수한
것이 아니다. 기존 authored counter의 .24초 주기/.16초 수명·peak .70과 다른 class의
legacy model-cue shader 경로는 source adapter와 구분한다.

LOA 색 payload는 packed FColor 4byte가 아니라 little-endian uint32 4개(16byte)다.
Warmup raw components는 [65,147,243,255], RPCZ stage2는 [0,0,255,255]다.
기존 decoder가 이를 RGBA로 부르지만 Core.Color reflection만으로 custom serializer의
R/B 의미 또는 native sRGB→linear 변환까지 증명할 수 없다. 따라서 기존 component 순서와
/255는 adapter 해석임을 명시했고 근거 없는 BGRA swap/gamma 보정은 추가하지 않았다.
`color-packing-receipt.json`에 exact offset·raw hex·payload hash와 미회수 경계가 있다.

최초 5개 TU(SkeletalAfterimage, Body_Valtan, Valtan, Npc,
KoukuSaydonPresentationPlayer)는 실제 project include/Debug 설정으로 out에만 격리
컴파일해 모두 exit 0이다. Crossing 변경 후 presentation TU도 다시 exit 0이다.
`client-compile.json`/`crossing-compile.json`이 명령과 결과를 보존한다.
실제 production helper cpp와 동일 owner window 함수를 직접 컴파일한 CPU fixture의
50 assertions가 통과했다(`native/receipt.json`, `native/run.log`). Engine mock은
shader 입력·수명 계약만 검사하며 GPU/Client 화면을 실행한 결과가 아니다.
8개 원본 파일은 UTF-8·CRLF를 유지했고 수정 전 원문을 out/before에 보존했다.
변경 source의 git diff --check도 통과했다. 최종 제품/GPU 검증은 G12에서 별도 기록한다.

### G09 추가 25개 직접 관련 occurrence — 제품 소스 반영 완료

최종 전수 join 81개는 source adapter 연결 45, source disabled 20,
별개 공격으로 범위 밖 16개다. 최초 미연결 41개 중 원본명이 `쿠크_돌진`인
RPCZ00 4219776/현재 거미카운터 15개와
RPCT07 4219920/4219921 고속이동 전방·후방 10개는 이번 요청에 직접 해당한다.
나머지 16개는 별개의 공격 연출로 범위 밖이다. 분류 근거는 `REMAINING_SCOPE.md`와
`remaining-scope-classification.json`, stable occurrence join은
`current-pattern-afterimage-join.json`이다.

직접 관련 25개는 removal flags false·part NONE·custom material/reference 0·pivot None·
scale 1·viewOffset 0으로 기존 carrier에 호환된다. 추가 source/shader를 만들지 않고
source history가 emission과 마지막 child 수명까지 authored counter보다 우선하도록
Npc의 소유 상태 한 개를 추가했다. Counter flag 변화가 native child를
지우지 않고, child가 끝나면 authored counter가 돌아온다. RPCZ는 원본 alpha .6과
userAlphaScale 1이며, 기존 counter를 대체하는 동안에는 별도 .70/.6 배율로
현재 요청된 최종 counter peak .70을 보존한다. 기존 dash .38/backstep .19는 유지한다.

반영 전 Npc/presentation 2TU compile exit 0과 실제 25개 현재 occurrence·수치·색입력·
counter↔source tail 전환을 포함한 native 197 assertions를 통과했다.
`candidate-related25/client-compile.json`, `candidate-related25/native/receipt.json`에
증거가 있다. Root가 최초 제품 build 종료 뒤 `apply_related25.py --apply`를 실행했다.
`candidate-related25/apply-receipt.json`은 적용 mode와 3개 파일의 전후 SHA를 기록한다.
Npc h/cpp는 준비 시점 이후 audio 변경으로 전체 hash가 달랐으나 소유 block의 exact-match를
확인한 뒤 최신 bytes에 병합했고, 교체 직전 CAS·원자 교체를 통과했다. 현재 3개 제품 파일의
SHA가 적용 receipt와 모두 같고, 검증한 owner window 함수도 제품 최종 함수와 정확히 같다.
반복 격리 컴파일은 실행하지 않았으며, 최종 정상 증분 Product 결과는 G12의 root 검증이다.

마감 시 Kouku composition 전체 SHA는 최초 감사 cb90e006…에서 최신 173d6f49…로
바뀌었다. 저장본을 되돌리지 않고 최신 문서를 다시 읽어 81개 source join을 재생성했고,
해당 stable occurrence·actor profile·source 필드·stage 시계가 모두 동일함을 확인했다.
`current-pattern-afterimage-join.json`, `source-audit-summary.json`,
`candidate-related25/mapping-receipt.json`은 최종 45/16/20 상태로 갱신했다.
`final-source-mapping-verification.json`에 최초/최신 Data hash·관련 field 동일·제품 hash·
197개 검증 함수 동일 여부를 남겼다. 문서 전체 hash 불변으로 보고하지 않는다.

## G10. 대상별 원본 피격 음성·입자 감사와 연결 반영

2026-09-25 원본 조사와 후보 검증 뒤 root가 최종 audio 변경21파일을 실제 반영했다.
새4 event와 원본 WAV18개는 제품 Data/Resources에 설치 완료했다. 이후 afterimage 변경과
병합된 최신 Npc에서도 음성 함수의 정확한 동일성을 확인했다. 정상 Product 빌드의
최종 상태는 root의 G12를 정본으로 참조하며, 사용자 실제 재생·청취는 미실행이다.

### G10-01. 실제 원본 참조와 현재 소비자

설치 lpk의 모델별 LookInfo→공유 Action LOA→Action12 BEHIT의 exact stage/notify를
추적하고, AkEvent 이름의 native Wwise event ID→Play action→하위 Sound media를
원본 bank에서 대조했다. 이름에 Hit/Dash가 있다는 이유만으로 incoming damage 연결을
확정하지 않았다. 원본 자산과 현재 미설치 여부는 아래와 같다.

| 모델 | 실제 Action | Dmg_Idle1/2 원본 이벤트·시각 | 현재 소비 경계 |
|---|---|---|---|
| MN_RPCT_05 | MN_RPCT_05 | 직접 피해 AKEvent 없음 | freeze 음성은 별도 반응이며 every damage 음성으로 차용하지 않음 |
| MN_RPCZ_00 | MN_RPCZ_00 | S_Mob_G_KouKu1.G_KouKu1_Damage1,1ms,5 media | 기존 KoukuSaydon 카탈로그/설치 WAV5 존재; exact rpcz00_dmg_idle_1/2 진입 소비 연결 |
| MN_RPBF_01 | MN_RPBF_00 | 직접 피해 AKEvent 없음 | FreezeStrong의 SYS_MONSTER_GROGGY를 일반 피해음으로 차용하지 않음 |
| MN_CDMD_00 | MN_CDMD_00 | S_Mob_OddDoll1.OddDoll1_Damage1,10ms,5 media | WORLD 인형은 att_battle_2_01 유지·damage flash만; Dmg_Idle sequence가 있어도 gameplay가 시작하지 않아 설치 제외 |
| MN_RHCN_00 | 없음 | LookInfo에 Action/AnimSet/SoundSet 참조 없음 | static WORLD 공에 다른 RHCN 변형의 피격음을 추측해 붙이지 않음 |
| MN_RPPB_01 | MN_RPPB_00 | S_Mob_C01.ClownBox1_Damage1,1ms,5 media | MonsterCatalog hit=dmg_idle_1과 실제 CNpc transient action 소비 있음 |
| MN_PADD_01 | MN_PADD_00 | S_Retch.Retch_Voice_Damage1,10ms,4 media | hit=mn_padd_01_sk.ao_dmg_idle_1과 실제 transient 소비 있음 |
| MN_SJFC_00-4 | MN_SJFC_00-4 | S_Mob_F01.FlameKerberos_Damage1,10ms,4 media | SJFC/ELITE hit=mn_sjfc_00_sk.ao_dmg_idle_1과 실제 transient 소비 있음 |
| MN_0019_05 | MN_0019_00 | S_Mob_Troll1.Troll1_Damage1,1ms,5 media | hit=mn_0019_05_sk.ao_dmg_idle_1과 실제 transient 소비 있음 |
| MN_RPRS_02 | MN_RPRS_00 | 직접 피해 AKEvent 없음 | current hit=critical_start1; 다른 몬스터 피해음 차용 안 함 |

일반 몬스터의 실제 연결은 ClientReplication의 outgoing nonzero NORMAL/CRITICAL/ABSORB
damage에서 IDLE/CHASE일 때 MonsterCatalog.presentationClips.hit→
CNpc::Play_TransientNetworkAction→Play_NetworkAction이다. 보스는 같은 damage에서
rim만 시작하므로 RPCZ 음성을 위해 보스의 행동을 임의로 Dmg_Idle로 바꾸지 않는다.

원본 CEFSoundContainer에는 down/ground 및 hittedWeapon SoundSet 참조가 있고,
CEFGameObjectSoundHittedWeaponSet에는 MOB_Hitted_SwordToFlesh1 등의 실제 AkEvent가
있다. native slot enum과 원본 CPU의 hit category 선택은 미확정이므로 새 범용 dispatch는
만들지 않았다. VoiceSet의 idle/death/freeze도 incoming damage 음성과 구분했다.

원본 reference context 전체6127개에서 AkEvent282종을 선택했고, 조사한 bank에서280개의
Play graph를 해석했다. SYS_MONSTER_FREEZE1/GROGGY2개는 조사 bank에서 미해결이며 원본에
없다고 단정하지 않는다. 일반 event의 reachable media 목록은 switch/layer/RTPC의 실제
동시 재생 결과가 아니다. 위6개 Damage1만 wwiser로 별도 typed parse하여 단일
Random→직접 Sound4/5개, weight50000 균등, loop1,global1,avoid-repeat1,continuous0을
확인했다. Stop-other-voice action, voice limit, bus/RTPC까지 기존 Play_Sound가 재현한다고
주장하지 않는다. wwiser의 missing memory audio/bus 경고도 해당 제어 복원의 한계로 남겼다.

9개 해당 Action12 BEHIT에는 PlayParticleEffect가 없었다. counter/dash/ghost의 실제
notify context와 root의 DevelopOption 기본 참조를 별도로 조사해170개 ParticleSystem이
모두 원본 package export에 존재함을 확인했다.114개는 현재 authored sourceNode가 정확히
일치하고37개는 기존 Kouku/Valtan 등록 cue 자산 참조가 있다. 이 수치는 enabled/현재
선택된 gameplay occurrence나 실제 GPU 표시를 뜻하지 않는다. DevelopOption의
GroggyHit/CounterHit/HitDefaultCri2 등은 노란 material rim과 다른 입자이며 이를 generic
damage마다 추가하지 않았다.

근거는 `out/CombatSourceAudit20260925/audio-fx/target-audit.json`,
`source-occurrences.json`, `event-media-installed-audit.json`, `soundset-events.json`,
`damage-event-typed-graph.json`, `particle-package-evidence.json`,
`particle-native-installed-audit.json`이다. 각 model LookInfo와 Action offset/SHA, 원본
package/export, bank/HIRC SHA, event/media ID, 설치 asset ID를 보존한다. 전체게임의
모든 effect/sound가 완전 복원됐다는 결과가 아니다.

### G10-02. 최종 반영과 실제 검증

최종 반영은 Npc.cpp/h의 성공한 Play_NetworkAction 뒤 exact model tag+clip을 확인하여
원본1/10ms pending notify를 arm하고, 모델 clock이 해당 시점을 지나면 한 번 소비한다.
첫 코드의 EffectV2 owner를 boss archetype으로 비교한 부분은 실제 spawn이 model stem을
넣는다는 소비자 검사에서 불일치를 확인했다. 현재 코드는 spawn과 같은 typed
KoukuSaydon/MonsterPresentationAssetService::Get_ModelPrototypeTag로 대조하며 문자열
추측 fallback을 추가하지 않는다. 실패·숨김·clip 변경·notify 뒤에서 시작하는 crop은
pending을 취소하고, missing sound/play 실패도 매 frame 재시도하지 않는다.

카탈로그는 기존 Valtan에 Retch/FlameKerberos/Troll3 event, 기존 KoukuSaydon에
s_mob_c01.clownbox1_damage1을 추가했다. 이벤트별 전역 직전 asset ID를 제외한 균등선택은
카탈로그 재정렬/축소에도 index를 남기지 않는다.한 variant만 가능하면 재생을 허용한다.
새 loader/schema/runtime은 없고 CSoundCueCatalog→CRuntimeAssetRoot→Play_Sound를 쓴다.
Data/Sound는 CProjectDataRoot가 직접 읽으며 Resources/Sound 안의 원본 WAV18개를
설치했다. unused WORLD 인형5개를 포함한23개는 원본 WEM→Ogg→FMOD NOSOUND PCM WAV
변환까지 실패0으로 준비했다. 자동 재생·청취는 하지 않았다.

- `damage-audio-candidates.json`, `prepare-damage-audio.log`: 원본23 media의 WAV 후보,
  SHA/형식/길이. 인형5개는 out에만 보존한다.
- `candidate-compile.json`, `Npc.candidate.compile.log`: 최종 후보 Npc TU Debug compile exit0.
- `consumer-helper.json`, `consumer-helper.log`: 실제 typed tag 함수 본문+현재 Boss/Monster
  catalog+설치 WModel clip bytes를 대조한7 owner/9 clip 조합,22 case PASS. per-event
  전역 no-repeat 반복과 notify 시각·단발·오대상·crop·숨김을 확인했다. 모델 clock/audio
  backend는 좁은 fake이며 실제 Client/사운드 장치 재생을 대신한 증거가 아니다.
- `hit-sound-norepeat-helper.json`: 별도 no-repeat 후보14 case,카탈로그1개축소·재정렬
  확인. `sound-catalog-merge-test.json`:4 field 병합·idempotence·무관 최신값 보존·동일 field
  충돌 거부4 case PASS.
- `consumer-candidate.json`, `consumer-apply-dryrun.json`: 소유5 source fragment+catalog4
  field+WAV18개,총21파일 dry-run admission PASS.

root가 첫 Product 종료 뒤 `apply_consumer_candidate.py --apply`를 실행해21파일 반영을
마쳤다. `consumer-apply-result.json`의 status=PASS와 backup 경로를 확인했다. 최신
source의 정확 소유 block과 catalog4 event만 병합했고 교체 직전 bytes/hash 재확인·
backup·원자 교체·자기 변경 rollback 경계를 유지했다. 이후 gate2의 afterimage 병합이
완료된 현재 파일을 읽기 전용으로 다시 확인했다.

`out/CombatSourceAudit20260925/audio-fx/installed-audio-verification.json`은 설치 WAV18개
SHA 모두 원본 후보와 일치, catalog4 event의 variant 배열 일치, 최종 Npc.cpp/h의
소유5 fragment(Arm/Update 함수 본문 포함)가 정확히 한 번 존재함을 기록한다. 이 마지막
확인은 추가 컴파일·하네스 반복·Client/UI 실행 없이 수행했다. 소스·카탈로그·음원
반영은 완료이며 정상 Product 빌드 결과는 root G12를 참조한다. 사용자 실제 청취는
미실행이고 원본 Wwise의 Stop-other-voice·voice-limit·bus/RTPC 경계는 그대로 남는다.

## G11. 원본 재질·호버·전역 피격 자료의 재감사와 native 입력 연결

사용자의 원본 기반 재조사 요청에 따라 현재 설치본의 쿠크·세이튼·앵콜·대형 세이튼,
발탄 몸체/장비/유령, 인형·두 종류 공·카드미로 중앙 상자와 발탄/입구 몬스터를 대조했다.
23개 UPK의 live MIC 69개 및 부모를 따라 78개 material 객체를 읽었고 native static set
74개와 surviving parent graph 4개의 해석 실패는 0이다. 전체 게임의 모든 재질을 조사했다는
뜻은 아니다. 자료·원본 payload hash·source package/export는
`out/CombatSourceAudit20260925/materials/original-material-audit.json` 및 MD에 보존한다.

원본 `Hit_Color` 기본값은 RGBA 0이며 actual native static set의
`1.use_hitcolordirection`은 false다. 이 기본값은 노란 피격 CPU 색이나 수명을 증명하지 않는다.
원본 normal/view 기반 rim을 포함한 cooked base 함수와 실제 uniform packing을 이용하도록
`DeferredMaterialRenderUtils`를 바꿨다. native program 21~32/84/92/93/238의 Base 전용
Hit_Color row RGB만 draw 사본에서 수정한다. shared CMaterial의 Base/Light 배열과
authored alpha는 보존하며 native 대상의 기존 generic rim 중복 가산은 끈다.
WORLD의 static/animated/forward 표시도 실제 model/mesh를 받아 같은 경로를 사용한다.
native 입력이 없는 legacy 경로는 기존 프로젝트 강조를 유지한다. 특히 미로 중앙 RPPB 상자와
PADD/SJFC/0019/RPRS(루가루) 및 일부 입구/마리오 몬스터는 현재 catalog native override가
없다. 집중 확인 5개 WModel의 material section은 WMA2 texture-only라 LEGACY/program0을
유지한다. World 인형 MN_CDMD_00의 native23과 별도 MONSTER_MARIO_CDMD generic을
같은 모델로 취급하지 않는다. 현재 분류는 native24행/generic13행(공유 archetype 포함)이며
전부 실제 CModel draw한 개수가 아니다. `materials/current-runtime-hit-coverage.json`이 근거다.
RPPB/상자 REUP/0019의 program26, RPRS 두 번째 slot의 program23 재사용 후보는 원본
shader-map key 일치까지만 확인했다. actual Base/Light DXBC·packed constant·texture ABI가
미폐쇄라 전체 재질을 추측으로 교체하지 않았다. `legacy-five-native-reuse-boundary.json`에
후속 복원 경계를 기록했다. Server의 양수 피해 이벤트,
기존 노란 색·0.12초 시계·세기와 호버 선택 조건은 PROJECT_TUNED/PROJECT_RECONSTRUCTED이며
원본 native CPU 코드를 회수한 것으로 분류하지 않는다.

`ColorOption.loa`는 397개 key/color record 전체를 정확히 소비했고
`OUTLINE_MONSTER_ENEMY=FE0000`을 확인했다. 호버 shader의 RGB 비율을 이 빨강으로 교체했다.
기존 HDR 에너지 2와 약2 physical pixel geometry extrusion은 프로젝트 값이다.
원본에는 PrimitiveComponent/EFPawn의 PPOutline API와 별도의 occluded-outline API가 있다.
global cache에서 이름·GUID·인접 code descriptor로 식별한 원본 DXBC 3개를 추출하고
전체 크기 및 D3DDisassemble 성공을 확인했다. DepthCompare는
`t1.a - t0.a + 0.00001 < 0`에서 discard하며, PPOutlineBlur는 7tap,
OccludedOutlineBlur는 5tap이다. 첫 depth sample의 `t0.wxyz -> r0.x` swizzle도 적용했다.
실제 RT producer·depth convention·CB CPU binding·blur 방향/횟수·최종 blend state는
미복구다. 따라서 현재 외곽선을 원본 후처리와 같은 구현 또는 픽셀 결과로 보고하지 않는다.
`materials/original-ppoutline-shaders.json`과 DXBC/ASM이 이 경계의 근거다.

`DevelopOption.loa`에는 CounterHit/GroggyHit/PartsBreak/HitDefaultCri2와 hit socket,
Counter_Start/Counter_State 참조가 존재한다. 그러나 해당 binary field의 native CPU 소비자와
모든 선택 조건은 회수하지 못했다. 이 입자를 모든 일반 피격에 붙이거나, 현재 파란 counter
모델 잔상을 원본 particle로 바꾸지 않았다. 원본 재질 입력·action notify·global particle
reference·sound set은 다른 계약이며 이름만으로 하나의 트리거에 합치지 않는다.

최신 helper와 ActorCatalog를 직접 컴파일한 out-only 실제 CModel/CShader 검사는
발탄21·인형23·쿠크26·유령84의 4모델/8mesh에서 통과했다(exit0, 약4.99초).
nativeBinary false/true, 실제 CB GetRawValue, 해당 row RGB만 변경, generic flag0,
shared surface Base/Light 불변, 다음 non-hit draw의 원래 값 복구, WORLD helper 동일 동작,
별도 skill glow의 generic flag1과 forward light bind의 원래 상수를 확인했다.
이 검사는 창/Client 실행 및 draw를 만들지 않았으므로 GPU 화면 성공으로 대신 기록하지 않는다.
`out/CombatSourceAudit20260925/native-hit-probe/receipt.json`에 입력·DLL·CSO·EXE hash가 있다.
수정 helper와 WorldSequenceObject의 격리 컴파일도 통과했다. 최종 제품 빌드·새 CSO 검사는
G12에서 별도로 기록한다.

## G12. 원본 재감사 반영 후 최종 Debug 제품·GPU 검증

G09 추가25개 잔상과 G10의 typed tag/피격 notify·4개 catalog event·18개 원본 WAV를
최신 저장본에 병합했다. audio21파일 반영 뒤 Npc h/cpp의 다른 변경을 보존해 afterimage
3파일을 부분 병합했고 각 교체 직전 hash/backup/atomic replace를 사용했다. 적용 receipt는
`audio-fx/consumer-apply-result.json`과 `afterimages/candidate-related25/apply-receipt.json`이다.
기존 알파 튜닝과 원본 initial alpha를 분리했으며 추가 RPCZ counter 겹침은 .70 peak를 보존한다.

정상 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`를 완료했다.
첫 제품 빌드 `20260925T102934852Z-debug-product.json`은 Client132 TU·CSO48개를 갱신했다.
마지막 후보 반영 후 `20260925T103044508Z-debug-product.json`은 PASS(36.251초), Client18 TU와
EXE를 갱신하고 CSO는 재생성하지 않았다. Engine/Shared/Server는 이 마지막 증분에서 변경0이다.
기존 FXC X4000/X3595/X3571 및 C4819/C4828 등 경고는 남으며 경고0 빌드가 아니다.
이번 코드 변경을 이유로 Client/UI나 Server gameplay를 자동 실행하지 않았다.

새 설치 CSO의 기존 `Test-SourceCharacterShaderVariants.ps1`은 exit0이다.
72개 CSO 복사본 SHA가 설치본과 일치하고 source program binding126, clone6, 실패 입력9,
light pass504, native cue144, afterimage4, unavailable pass10, combat pass12를 통과했다.
실제 WARP slab draw12회에서 static/skinned 빨간 border 각각240pixel, 내부/깊이차폐/투명 mask
경계와 generic 피격 해제 복구를 검사했다. 이는 모든 원본 모델의 native hit GPU draw 검사가 아니다.
native hit 실제 CModel/CB 검증 범위는 G11의 대표4모델/8mesh를 따른다.

새 afterimage pass14의 별도 bounded GPU 검사는9draw 모두 통과했다. diffuse·ambient·rim의
분리/합산, .76/.38 alpha의 정면/측면 유지, legacy모드와 후속 draw 복구를 확인했다.
RGBA 최대 오차7.915e-7(허용2e-4), WARP64×64, windowsCreated0이며 원본 화면 동일성을
입증하는 검사가 아니다. `afterimage-gpu-probe/receipt.json`과 run.log에 보존했다.

변경 Sound catalog JSON parse와 `git diff --check`를 통과했다. 설치 Client/Server/Engine 및
Sound catalog hash, 72개 CSO hash와 build/검사 집계는
`out/CombatSourceAudit20260925/final-product-receipt.json`에 기록했다.
Client 배포 Engine.dll은 Engine 산출물과 hash가 같다. 자동 stage/commit/push는 하지 않았다.

원본 PPOutline CPU/RT 합성, G11의 legacy13행 native 재질, 원본 hit CPU 색/시간,
TrailGhost native fade/색 직렬화 의미, Wwise의 stop/voice-limit/mix 제어와 실제 게임 화면·청취는
완료로 분류하지 않는다. 확인된 source data를 기존 runtime에 연결한 부분과 프로젝트 adapter를
구분한다. 대형 세이튼 피격 제외와 현재 렌더링 옵션은 이 변경에서 다시 조정하지 않았다.
