# 쿠크세이튼 1관문 두 짤 패턴 V1 연결 결과

## 구현 범위

2026-09-11. 기존 Effect Tool V1, source recipe, CModel/CMaterial, Action Workbench의 Composition Append 경로에 연결한다. 새로운 이펙트 런타임이나 보스 AI를 추가하지 않는다. Client/UI 자율 실행과 화면 캡처는 하지 않았다.

| 패턴 | 원본 애니메이션 | V1 문서 | 재생 창 |
|---|---|---|---|
| 4219877 내려치기 C | RPCT05 `Att_Battle_1_01`, 2000 ms | `effect.kouku.gate1.4219877.full.restore` | 6582 ms |
| 4219801 불뿜기 | RPCT05 `Att_Battle_14_01` 1667 ms + `Att_Battle_14_02` 3167 ms | `effect.kouku.gate1.4219801.full.restore` | 11075 ms |

Composition revision 323의 pattern 29·30 / presentation resource 53·54다. 원본 공격 뒤 남는 이펙트 수명은 실제 `Idle_Battle_1`의 LOOP_TO_WINDOW로 연결한다. 수동 AUDITION_ONLY, selectionWeight 0이고 기존 PRODUCT의 MECHANIC 분류 계약을 유지한다. 원본의 피해·투사체 서버 스킬 XML을 확보한 것은 아니므로 원작 서버 판정 복구나 자동 AI 편입으로 기록하지 않는다.

## 실제 소비자

- All Effects의 KoukuSaydon 분류에서 두 DIRECT_AUTHORED_DOCUMENT에 `Open Editor`와 `Play Effect`를 모두 제공한다. 목록의 Play Effect도 기존 Current Effect 전체 재생 함수를 사용한다. `.restore`가 캐릭터 skill 복원으로 오인되지 않도록 기존 Sequencer의 typed Kouku 선택을 사용한다.
- Action Workbench의 Saydon/1관문 두 패턴은 기존 V1 Append resource binding을 소비한다. V1 Play/Solo도 Composition의 유일 패턴과 실제 CNpc/CModel preview를 선택한다.
- 같은 문서의 Play All을 다시 누르면 모델과 FX를 함께 0ms부터 재시작한다. 문서를 연 직후 첫 Element Solo도 Kouku 모델을 준비한 뒤 root를 조회한다. 잘못된 문서나 모호한 패턴 연결은 기존 선택을 보존한다.
- 원본 53 first-LOD emitter occurrence(내려치기16, 불뿜기37)를 유지했다. FX_Prj_01/FX_Prj_02 복수 소켓 발생을 분리하면 runtime 73 elements(16+57)다. 광원·메시·데칼·baked AnimationTrail을 포함한다.
- 원본 소켓32개를 읽고 `b_wp_1`, `bip001-head` 등 실제 모델 본을 전달한다. 원본 cm→m 변환과 BossCatalog modelPreScale 0.017의 비율을 보존한다. first socket을 잃던 UModel wrapper 파서를 교정했다.
- 파생 patternbindings의 optional sourceAnchorAnimations를 projector와 C++ reader에 함께 연결했다. 60 Hz 과거 샘플은 원본 animation timeline과 기존 CModel pose sampler를 사용한다. 제자리 수동 패턴 두 개는 frozen actor root와 움직이는 원본 bone timeline을 사용한다. 이동하는 모든 보스의 과거 root motion을 해결한 것은 아니다.

## 원본 재질과 모듈

37 material path에 대해 실제 VF별 38 source color program을 회수했다. 기존 native carrier에 2304..2341 범위만 추가했고 원래 character program은 유지한다. 텍스처 없는 analytic glow와 Disto05의 source binding도 원본 shader bytecode에서 확인했다. Disto05의 별도 accumulation PS `56fd4335bcbc2843a7577f32fab7249e`를 추가해 양/음 XYZW 출력을 기존 signed UV distortion target으로 변환한다.

V1 기존 파티클 모듈 evaluator를 재사용하며, 실제 원본에 있는 AccelerationOverLifetime의 `acceloverlife` 분포만 기존 갱신 경로에 추가한다. 궤적은 원본 baked edge22개와 원본 color/alpha/dynamic 및 point lifetime을 사용한다. Root에 직접 연결되는 두 궤적의 local scale은 BossCatalog의 0.017/0.01=1.7을 적용한다. 광원의 초기 색과 밝기, 원본 burst 1개와 particle lifetime 0.5초도 실제 typed light에 전달한다.

외부 패키지 7개에서 빠져 있던 35개 module export를 추가로 해석했다. 53개 base emitter의 raw 첫 LOD 참조 571개를 수와 순서까지 대조했고, 두 소켓 발생을 전개한 문서는 73요소·794모듈·1,168분포다. C++ 검사에서 발견한 외부 참조 누락, baked history 상대 종료 시각, typed light template, decal 전용 literal 계약을 교정했다. 원본 tick/editor 메타데이터는 추출 기록에 보존하며 portable 문서에는 실제 소비되는 속성을 전달한다. 데칼 roll은 실제 Playback 소비자와 함께 정확한 Kouku native profile에 한해 finite NUMBER와 3,600도 경계를 검사한다.

내려치기 문서는 기존 authored v15의 document-owned runtime projection을 사용한다. 두 baked trail은 이 경로의 SourcePresentation 활성 상태·profile·source identity를 연결한다. 일반 Stage_Document만 실행하면 확인되지 않는 이 계약까지 실제 Catalog와 같은 factory 및 Playback stage로 검증한다.

원본 픽셀 계산과 프로젝트의 렌더 입력은 구분한다. lit decal의 UE skylight는 프로젝트 scene owner가 없어 기존 native model과 같은 committed scene ambient를 사용한다. 지면 receiver는 기존 depth projector다. fog/scene grading은 명시적인 프로젝트 adapter이며 원작 전체 광원·후처리를 재현했다는 뜻이 아니다. 원본 비활성 FireWave notify를 켜서 패턴 후보를 만들지 않았다.

두 문서가 실제 참조하는 Resources는 texture 67개와 mesh 3개, 총70개다. 기존 원본 매핑을 재사용하고 누락된 소수 texture/mesh를 `Effect/KoukuSaydon/FullRestore/`에 설치했다. 모든 참조 파일의 존재를 확인했다. 바이너리는 Git 추적하지 않는다. 실행에 필요한 실제 파일은 native material patch/geometry installation의 Resources-relative ID를 따른다.

## 입장 병목

별도 [입장 병목 결과](2026-09-11_KOUKU_ENTRY_STUTTER_IMPLEMENTATION_RESULT.md)에 기록했다. 선택 class의 Product effect cue 준비와 약190.8 MB의 Deploy prototype 준비를 Loader로 이동하고 Level activation의 중복 준비를 제거했다. 사용자 PC의 입장 뒤 10초 프레임 저하가 얼마나 개선됐는지는 아직 계측하지 않았다.

## 검증 상태

- 공식 Kouku publisher revision323: product, map.kakulsaydon, world.gameplay, gameplay.balance 4 domain PASS. 저장30 / 실행26 / 실행 stage200.
- source socket parser 2 tests, decal receiver normal contract 4 tests PASS.
- native shader extraction의 textureless 분기 포함49 tests PASS. 비어 있지 않은 기존 shader 검사와 실제 selectioncolor binding 검증을 유지했다.
- source module/distribution 및 material/attachment Python 구조 검증은 두 문서 PASS.
- mesh/particle/decal/trail FXC syntax compile PASS. 마지막 codec 및 반복 Play/첫 Solo 수정까지 포함한 Debug Product compile/link/deploy PASS. `out/BuildPipeline/runs/20260910T215447751Z-debug-product.json`: Engine/Shared/Server/Client 모두 PASS, 누락 runtime input 없음.
- 실제 Product C++ obj를 사용한 CPU 검사 PASS. 두 원본 문서 Parse, v15 document-owned projection 생성, immutable 준비, Playback Stage, 총103시점 Seek, 되감기, 필수 source slot 누락 거절, 잘못된 Parse에서 이전 출력 보존을 확인했다. 내려치기45시점은 파티클 최대44·원본 잔상2개·edge pair 최대38, 불뿜기58시점은 파티클 최대103·조명2개가 생성됐다. 모든 검사 transform은 유한했다. 사용한 root/source-slot matrix는 명시적인 합성 입력이며 실제 CModel 본이나 화면 fidelity 증거는 아니다.
- CPU 검사 정본은 `out/KoukuSourceAnchors20260911/cpu_probe_result.json`이다. 최종 두 문서 SHA256은 각각 `d6c2d56ccb4647bf25a925ba4bc879257c6b7b3e34d71f3daa00fa3ef2c51e3d`, `d2e861c79c1417082f4df556734fe03db77f77a5ce765f9998243f953ba59ca6`이며 검사 입력과 현재 파일이 일치한다.
- 변경 JSON/XML parse, 두 FxCompile과 project reference 구조, 전체 `git diff --check` PASS. 입장 및 projector 기존 테스트의 baseline 실패 경계는 각 RESULT에 분리해 기록했다.
- 사용자 Client 화면, 최종 색·크기·타이밍과 FPS 확인은 미실행이다. visual PASS가 아니다.

## 사용자 확인 경로

마지막 상태 조회에서 Server/Client 프로세스는 모두 실행 중이 아니었다. Visual Studio의 `Server + Client` profile을 Ctrl+F5로 실행한다. Lobby에서 KoukuSaydon에 입장한다. F1 → Effect Tool V1 → All Effects → KoukuSaydon에서 두 문서의 `Play Effect`로 직접 전체 재생하거나, `Open Editor`로 열어 Current Effect의 Play All/Solo를 확인한다. 모델 공급자는 해당 아레나이므로 Play/Solo는 입장 후 사용한다. F1 → Action Workbench → Boss `Saydon` → `1관문`에서 `세이튼_내려치기C_FullRestore` 또는 `세이튼_불뿜기(원본0-1)_FullRestore`를 선택하고 `Complete Play (Server)`로 실제 연결을 확인한다. 기존 패턴이 선택된 상태와 구분한다.

마지막 확인 항목은 반복 Play All, 문서를 연 직후 Element Solo, 원본 공격 애니메이션에서 Idle로 넘어가는 동안의 잔여 이펙트, 입장 직후10초 프레임 상태다. Client를 에이전트가 실행하거나 화면을 캡처하지 않았다.

## All Effects 목록의 직접 Play Effect 연결

기존 Kouku 목록은 Open Editor만 제공하고 재생은 열린 편집기 안에서 가능했다. 이번 추가는 목록의 각 catalog 행에 Play Effect를 제공한다. 이미 열린 문서는 저장본으로 다시 읽지 않고 현재 draft를 기존 `Try_PlayActiveUnifiedEffect`로 전달한다. 다른 문서는 정확한 catalog 경로·ID와 SYNCHRONIZED_PRODUCT 의도로 열고 같은 재생 함수를 호출한다. 이 함수가 기존 readiness 검사와 typed Kouku model/animation 선택을 소유한다.

다른 문서에 저장하지 않은 변경이 있으면 기존 문서 전환 guard를 유지한다. 정확한 pending path·ID·intent의 요청에만 전체 재생을 예약하고, 전환 완료 후 Kouku 분기는 같은 canonical play 함수를 호출한다. 문서 로드가 성공한 뒤 preview 준비만 실패하면 로드된 문서는 유지하고, 원래 preview 오류를 Document/All Effects 상태에 표시하며 전환 모달은 닫는다. 로드 성공과 preview 실패를 구분하므로 성공 문구가 재생 오류를 숨기지 않는다. 취소 또는 읽기 실패가 기존 문서를 지우지 않는다. 목록 렌더는 계속 metadata만 조회하며 버튼 클릭 전에는 effect 문서를 파싱하지 않는다. Composition Append, Save와 publisher 호출은 추가하지 않았다.

변경은 `Effect_Tool.cpp::Render_KoukuAuthoredEffectSection`과 `Execute_PendingDocumentLoad`의 Kouku 전체 재생 분기로 한정했다. 기존 Open Editor, Current Effect Play All/Solo, character/Valtan 경로를 보존했다. 새 H/CPP·Data·Resources는 추가하지 않았다.

이번 추가의 검증은 기존 `test_effect_tool_lazy_metadata_contract.py` 9개 통과, PLAN 전체 Effect_Tool.cpp와 현재 코드 일치, 해당 파일의 `git diff --check` 통과다. 이번 통합 Product Debug는 Engine/Shared/Server 빌드 후 Client 컴파일 도중 사용자 Visual Studio 빌드와 동일 Debug 경로를 사용하는 것이 확인되어 에이전트 빌드만 중단했다. 이번 버튼 추가 뒤 최종 Client 컴파일·실행은 사용자 빌드 결과 확인 전이며 PASS로 기록하지 않는다. 사용자 Client 조작 및 시각 확인은 수행하지 않았다.

## Authoring occurrence의 v15 잔상 연결 보완

`Effect_Tool_Workspace.cpp::Create_AuthoringOccurrence`가 일반 Stage_Document만 사용하던 경로를 수정했다. 현재 draft의 immutable 사본으로 원본 v15 projection과 renderer resources를 clone 전에 준비하고, 기존 `Stage_PrevalidatedVisualProgramDocument`에 전달한다. anchor 조회 map과 projection이 같은 사본을 보유해 수명을 유지한다. 전체 v15 Play와 baked trail Solo는 이 경로가 필수다.

Element Solo는 원본 전체 문서를 먼저 검증하고 선택 사본에 실제 참조된 history만 남긴다. carrier가 없는 일반 Element Solo와 v13 문서는 기존 Stage_Document를 사용한다. v15 버전은 바꾸지 않고 누락 참조 검증, prepare 실패 반환, clone 후 실패의 Layer rollback을 유지한다. 읽기 전용 독립 검토로 실제 caller와 실패 소비자를 확인했다. 이번 Workspace 변경 이후 CPU/GPU 실행 증거를 추가한 것은 아니며, 위 기존 CPU 검사와 구분한다.

## PR #358 통합 검증과 사용자 실행 인계

Kouku revision323 공식 owner publisher 4 domain PASS, composition validate PASS(저장30/실행26/stage200). 관련 JSON 9개와 project/filter XML 8개 parse PASS, origin/main 대비 PR diff whitespace 검사 PASS. 로그와 구조 검사는 `out/PR358Integration20260911/`에 보관했다. 최신 요청에 따라 PR은 갱신하고 사용자가 자신의 EXE 빌드·화면 검증 후 병합한다. 에이전트는 중단한 빌드를 성공으로 처리하지 않고 별도 Client 실행을 수행하지 않았다.

## Action Benchmark 관찰 뒤 스케일·공간 해석 수정

사용자는 Action Benchmark의 패턴 Play에서 불뿜기의 작은 불과 뒤쪽의 거대한 sprite 사각형, 내려찍기 미표시를 보고했다. 내려찍기는 별도 오류 문구가 없었던 것으로 기억한다고 답했다. 아래 검증은 이 관찰 뒤 수행했으며, 앞 절의 과거 compile/CPU 결과와 구분한다.

`KoukuSaydonPresentationPlayer.cpp::Build_SourceAnchorWorlds`가 bone basis에 추가로 곱하던 100을 제거했다. 설치된 `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`의 세 공격 clip을 각 31시점으로 읽으면 골격 combined basis는 약100이다. CModel의 .017 pre-transform 뒤 source attachment에 필요한 basis는1.7이다. 기존 코드는170을 만들었으므로 socket offset과 particle 크기를100배 키웠다. 본 위치와 애니메이션 회전·스케일은 그대로 보존한다. 앞 절의 단위 비율 설명만으로 이 모델의 cooked root basis를 검증한 것으로 간주하지 않는다.

Kouku builder가 Required/CDO/archetype을 합성한 뒤 `bUseLocalSpace`의 생략값을 false로 전달하도록 수정했다. 미확인 legacy importer 기본 true를 복사하던 경로에서 내려찍기12개, 불뿜기36개의 localSpace 값만 변경했다. source notify의 bone-follow는 출생 위치를 공급하고, world-space 입자는 출생 transform을 유지한다. 원본에 명시된 true와 zero/null spawn distribution은 보존했다.

검증 증거는 `out/KoukuRenderRepair20260911/`에 둔다.

- `model_bone_measurements.json`: 실제 설치 모델의 raw basis, 기존170과 수정1.7 basis, 본 위치를 분리했다. 실제 Client 실행이나 화면 캡처는 아니다.
- `cpu_probe_result.json`: 기존 Product codec/projection/Playback obj를 사용한 60 Hz 전체 수명 검사. 내려찍기397프레임·불뿜기667프레임의 Parse/Stage/Seek·되감기·source slot 누락 거절·유한 행렬 검사가 통과했다. 내려찍기는16개 중13개, 불뿜기는57개 중37개 occurrence가 생성됐다. 나머지 zero-spawn 원본에 임의 burst를 추가하지 않았다. 입력 source slot basis1.7은 합성값이므로 실제 모델 경로 검사와 구분한다.
- 내려찍기 baked trail payload 1,392표본의 RGBA mask는 모두15, 수명 범위는0 이상1 미만이고 누적 거리도 유효했다. CPU 궤적이 만들어진다는 사실만으로 화면 표시를 판정하지 않는다.
- `shader_variable_probe_result.json`, `all_material_variable_probe.json`: 설치 CSO를 WARP로 읽어 native parameter/time와 렌더 경로의 변수 존재를 수치로 점검했다.
- `native_draw_probe_result.json`: 원본 DDS, native parameter packet, 같은 CPU 표본의 color/dynamic을 사용한 창 없는64×64 WARP 검사다. trail2306/2307, sprite2308/2309/2313/2314/2316/2317/2318, mesh2315에서 비영 RGB 픽셀이 확인됐다.2312는 원본 RGB0·alpha 출력이며2310 distortion은 이 고정 입력에서0이었다. 비유한 픽셀은0이다. 실제 장면의 depth·카메라·렌더 제출을 재현한 검사는 아니다.

거미카운터의 `MN_RPCZ_00`도 별도로 읽었다. 네 `rpcz00_att_battle_6_01..04` clip의 head/spine1/root36표본은 raw basis99.999962~100.000034다. 같은 모델이라도 G1 preScale .017과 G2 .012053이 다르므로 결과 basis는 각각1.7과1.2053이다. 같은 helper가 그 값을 보존하며 G2에1.7을 강제하지 않는다. 수치는 `spider_model_bone_measurements.json`에 둔다.

불뿜기의 중복100배 계산과 localSpace 오해석은 소스에 반영했다. 변경한 `KoukuSaydonPresentationPlayer.cpp`의 Debug C++20 최소 컴파일은 통과했다(`kouku_compile.log`, 기존 C4819 경고 포함). `SelectedFiles`를 지정한 MSBuild가 전체 ClCompile로 확장된 시도는 편집 중 파일과의 경합을 피하려 중단했고 성공으로 기록하지 않는다. 뒤이어 수행한 별도 out OBJ 컴파일만 이번 최소 컴파일 증거다. 내려찍기의 사용자 화면 미표시가 이 수정으로 해소됐는지는 미확인이다. 사용자는 우선 진행 중인 작업을 마무리하도록 요청했으며, 최종 visual PASS는 기록하지 않는다. 최종 통합 컴파일·링크와 정확한 재생 경로는 통합 검증 뒤 아래에 기록한다.

## 이번 요청의 연결·데이터 최종 상태

거미카운터 추가 뒤 Composition revision328 공식 owner publisher4개 domain을 통과했다.
생성된 Client patternbindings와 Encounter의 Product 목록에서 내려찍기29·불뿜기30·거미15의
실제 Effect asset과 유효한 재생 창을 확인했다. 새 장판과 리본의 세부 내용은
[거미카운터 결과](2026-09-11_KOUKU_SPIDER_COUNTER_SOURCE_EFFECT_IMPLEMENTATION_RESULT.md)에 둔다.

현재 수정한6개 Effect 문서는 내려찍기16, 불뿜기57, 거미 준비5/돌진9, 워로드AltV230/186요소다.
각 문서의 실제 Resources 참조를 확인했고 변경 JSON/XML15개 parse와 `git diff --check`도
통과했다. 증거는 `out/KoukuRenderRepair20260911/final_validation.json`이다. 모든 class의
광역 source 검사 결과를 이번 좁은 검증의 성공으로 바꾸지 않는다.

Solo 즉시 재생·Shift/Ctrl선택의 Play Group 반복은
[선택 재생 결과](2026-09-11_EFFECT_TOOL_SOLO_AND_SELECTED_GROUP_PLAYBACK_RESULT.md),
F native446을 재사용한 황금 번개18개 묶음은
[워로드 결과](../09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md)에 기록했다.
AGENTS의 복구 문서 탐색 규칙과 V2/gotchas의 반복 결함 항목도 함께 갱신했다.

## 최종 Debug 빌드와 실행 인계

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`의
Engine/Shared/Server/Client compile·link·deploy가 모두 통과했고 missingRuntimeInputs는0개다.
정본 결과는 `out/BuildPipeline/runs/20260911T073907619Z-debug-product.json`, 상세 로그는
`out/KoukuRenderRepair20260911/product_debug_final.log`다. Client.exe는2026-09-11 16:39:07 KST
빌드다. C4819/C4828과 외부 DirectXTK PDB의 LNK4099 경고는 남았고 컴파일 오류는0개다.
앞서 중단돼 최종 결과가 없었던 `product_debug_build.log`는 성공 증거로 사용하지 않는다.
데이터 배포는 별도로 수행한 revision328 owner publisher 결과를 따른다.

마지막 상태 확인에서 Server와 Client는 모두 실행 중이 아니었다. LAN 설정은 server-host,
`192.168.0.14:7777`이고 Visual Studio의 **Server + Client** profile로 Ctrl+F5를 누른다.

1. Lobby → KoukuSaydon → F1 → Action Benchmark에서 기존1관문 내려찍기29·불뿜기30의
   패턴 Play를 확인한다. 내려찍기 미표시 해소와 불뿜기 크기·위치는 사용자 화면 확인 대기다.
2. 같은 도구의 GATE2 `쿠크_거미카운터`에서 세 돌진의 검정·붉은 바닥과 잔상을 확인한다.
3. Character Select → Warlord에서 F와 Alt+V의 번개 형태·황금색·개수를 비교한다.
4. Effect Tool → Current Effect에서 Solo를 누른다. A를 일반 클릭하고 B를 Shift클릭한 뒤
   Play All 오른쪽 Play Group을 눌러 추가 Play 없이 반복되는지 확인한다. Stop/문서 전환도 확인한다.

Client/UI 실행·조작·캡처와 수동 visual PASS는 수행하지 않았다. 같은 작업 폴더에 다른 기능의
변경이 함께 있어 stage/commit/push하지 않았으며, 기존 변경을 되돌리거나 정리하지 않았다.


## 내려찍기 미표시의 v15 renderer 준비 실패 수정

추가 사용자 관찰은 불뿜기는 표시되지만 내려찍기는 표시되지 않는다는 내용이다. 내려찍기 v15와 거미카운터 stage2 v15를 실제 Product codec·projection·renderer로 실행하면 둘 다 `Visual-program adapter denominator did not map to prepared elements.`로 GPU resource 준비가 실패했다. 정상 표시된 불뿜기와 거미카운터 stage1은 v13이며, 단일 shader draw나 CPU playback 검사는 이 준비 실패를 통과하지 않는다.

`Effect_DocumentRenderer.cpp`의 `ADAPTER_PACKET_V1` 검증은 LocalDecal adapter가 반드시 하나 이상 있다고 가정했다. document-owned baked trail/ribbon은 유효한 supplemental element만 소유하므로 LocalDecal 수가 0이다. 유효한 supplemental이 있으면 이 0을 허용하고, 실제로 준비한 LocalDecal adapter 수와 승인된 수의 일치 검사는 유지했다. source 크기·본 단위·shader·저작 데이터는 변경하지 않았다.

`out/KoukuDecalAnchor20260911/renderer_stage_probe_run.log`는 수정 전 두 문서 모두 prepare 실패, exit1을 기록한다. 같은 임시 CLI와 수정한 renderer의 별도 MSVC out OBJ를 사용한 `renderer_stage_probe_fixed.log`는 두 문서 모두 parse/projection/실제 WARP resource prepare/renderer attach/`CEffectObject::Clone` 성공, exit0이다. 실제 설치 texture·model과 Product CSO를 사용했으며 Client, 창, swapchain, draw 또는 캡처를 만들지 않았다. 최소 C++ compile/link와 `git diff --check`를 통과했다. 최종 Client 통합 빌드는 같은 작업의 Pattern Flow 결과에 기록하며, 실제 아레나 표시·형태 판정은 사용자 확인 대기다.
