# 차원술사 S 원형 공간·관통 광선과 검격 재질 원본 조사 결과

최초 조사 기준: 2026-09-09 01:34 KST, 후속 재확인 01:52~01:55 KST,
`codex/kouku-ball-motion-effects`, HEAD
`f92178f054a759c128a61cebd8e2ff4b8712eb12`의 dirty working tree.
`Review rendering restoration status`는 같은 checkout에서 구현 중이다. 아래의 설치 상태는
각 표시 시점의 관측이며 그 세션의 후속 반영 완료를 대신하지 않는다. 최초 작업은 조사·문서였고,
사용자가 지속 복원을 요청한 뒤 G31에서 아래의 renderer 최소 수정·검증을 수행했다.

## G31 반영 결과: V63 원본 속도 정렬 연결

`Client/Private/Effect_DocumentRenderer.cpp`의 `Make_ParticleSpriteWorld` 안 native velocity
basis 목록에 **profile63 한 행을 추가했다.** source-enabled와 `CAMERA_VELOCITY` 조건을
유지하며, 같은 선택 VS를 쓰는 S4개·V2개의 Shine이 기존 Q의 원본 U/V 계산을 소비한다.
픽셀 셰이더·원본 JSON·pivot literal·UV flip 코드는 바꾸지 않았다. 변경 바깥 bytes와
UTF-8 BOM/기존 개행을 보존했다. 수정 시 compiler process는 관측되지 않았다.

검증은 기존 Q actual CPU probe 방식을 재사용했다. 현재 제품의 world 함수와 scale helper
본문을 그대로 추출하고 실제 DirectXMath SDK로 컴파일해 원본 VS의 U/V 식과 비교했다.
camera/data owner만 최소 stub이며 Client·Engine 초기화·UI 실행은 없다.

| 검사 | 실제 결과와 범위 |
|---|---|
| CPU 축·모서리 수치 | 865 checks, failures0. V63 및 기존 Q, pivot .1/.9, 양/음 evaluated 크기, off-axis/depth motion, roll 포함 |
| 원본 basis 비교 | 최대 모서리 오차9.83025e-7m. 기존63의 동일 검사 최대 오차5.9567m는 선택한 합성 입력 사례의 수치이며 원작 화면 실측 거리가 아님 |
| 회귀·실패 보존 | 다른 profile·비속도 정렬·source disabled·zero/view-parallel fallback 유지. 비정상 velocity는 output mutation 없이 거부 |
| 최소 C++ 컴파일 | 전체 Effect_DocumentRenderer.cpp translation unit을 별도 out OBJ로 컴파일, exit0. 컴파일 중 source hash 불변 |
| diff | 반영 전 snapshot과 한 행 차이, whitespace 오류 없음. 외부 dirty 변경 유지 |
| 제품 빌드 | 다른 작업이 실행한 Debug Product가02:15:05 종료. Engine/Shared/Server/Client PASS, SkipBuild=false, missingRuntimeInputs0. 아래 포함 근거 확인 |
| 실제 화면 | Client/UI 실행·캡처·원작 유사도 판정은 아직 없다. CPU/빌드 성공을 시각 PASS로 기록하지 않음 |

Product 포함 근거는 `out/DimensionMasterSlashFix20260909/product_debug_build.log`의
Effect_DocumentRenderer.cpp 실제 재컴파일과 Client link/deploy 완료, 그리고
`out/BuildPipeline/runs/20260908T171505116Z-debug-product.json`이다. 수정 소스는02:08:13,
제품 renderer OBJ는02:14:38, `Client/Bin/Debug/Client.exe`는02:15:04에 생성됐다.
관측한 MSBuild59044/37124는 실행 중임을 확인한 뒤 종료를 확인했다. source SHA256은
`44b06be4543ab6b2b84e3aafb987038ce88dde3e26a92b66f4615d747b785d78`로 수정·최소 컴파일·
제품 빌드 이후 재확인까지 같았다. 이 조사 세션은 별도 Product 빌드를 시작하지 않았다.

빌드 후 확인 시점에는 Server/Client process와7777 listener가 없었다. 사용자는 Visual Studio
`Server + Client` profile로 실행하고 Character Select→차원술사→F1→Effect Tool→
`이펙트_차원술사S_전체`를 선택해 재생한다. 실제 블랙홀/검격/교차선 관찰은 사용자에게 요청했다.

한 파일 컴파일의 최초 준비에서는 Windows SDK winrt include 경로를 빠뜨려 `wrl/client.h`
찾기 실패가 있었다. 기존 MSBuild의 Unicode/SDK include 설정을 임시 명령에 맞춘 뒤 재실행해
통과했다. 제품 소스를 임시 환경 오류에 맞춰 수정하지 않았다.

부호/pivot의 원본 CPU packing은 여전히 미확정이다. 원본 VS는 geometry corner와 texture UV를
별도 입력으로 받는다. 로컬 별도 Epic UE5.7.4 코드에는 abs geometry·UV flip·원래 pivot을
유지하는 구현이 있어, negative source size라는 사실만으로 pivot을 `1-pivot`으로 바꾸는 것은
근거가 부족하다. 이번 수치 검사는 기존 magnitude-size geometry에서 **basis 연결만** 검증한다.
원본 live vertex payload 전체나 UV flip 자체의 신규 실행 검증으로 확대하지 않는다.

후속 핵심8행 입력 감사에서는 명시 원본 LUT 변경0, native named parameter 누락/유효값 불일치0,
default 분포 보완19개를 확인했다. Dynamic12곡선과 signed size/pivot/중첩 size를 보존한다.
e38의 literal SizeY0은 현재 fixed-axis X→Y 확장이 처리한다. 추가 JSON 수정 근거는 발견하지 않았다.

근거: `out/DimensionMasterSForensic20260909/shine_velocity_edit.json`,
`shine_velocity_actual_cpu.result.json`, `shine_velocity_actual_cpu.receipt.json`,
`shine_renderer_compile.receipt.json`, `shine_renderer_compile.log`,
`v63_basis_minimal_audit.md`, `s_core_inputs_current_audit.json`.

## 후속 진행 상태: 02:00 이후 파일·로그 재확인

사용자는 다른 구현 세션이 반영 후 현재 빌드 중이라고 알렸다. 이 조사 세션은 중복 코드 수정이나
빌드를 실행하지 않고 최신 산출물을 읽었다. 원본에 최대한 가까운 블랙홀·검격·주변 스크류를
지속 복원하는 목표를 유지하며, 빌드 이후 실제 화면 비교는 사용자 관찰로 진행한다.

- 최신 S full은 `이펙트_차원술사S_전체`, 원본30개를 보존하며 `visible=true`24개/false6개다.
  비활성6개는 `_02/_04`의 작은 파편 e3/e4 네 발생, `_05/e48` 공격 빛, ZoomBlur다.
  표시 활성24/30을 화면 복원율80%로 환산하지 않는다.
- `out/DimensionMasterSlashFix20260909/sd_final_codec.txt`의02:00:31 결과는
  `PASS effect.dimensionmaster.skill.2050220.full.restore 30 0`, failures0이다.
  01:46/01:49의 이전 exact-open 실패 로그는 각각 Dynamic semantics/vectorfield 연결 이전이다.
  최신 codec 통과와 과거 실패 로그를 동일 시점으로 혼합하지 않는다. 최신 GPU/화면 성공은 별도다.
- 02:00 상태 확인에서는 V63의 native velocity basis 적용 목록 부재가 남아 있었다.
  이후 G31에서 수정하고 최소 컴파일·CPU 검증을 통과했다. 제품 빌드의 포함 여부는 해당
  최종 산출물로 확인하며 코드 수정만으로 최신 EXE 반영을 주장하지 않는다.
- Product EffectCatalog는 여전히 `2050220.unified`를 가리킨다. 복구 full을 확인할 때에는
  Effect Tool의 `이펙트_차원술사S_전체`를 명시적으로 선택한다. full 파일의 설치가 S키의
  제품 binding 자동 교체를 뜻하지 않는다.
- 원작 비교에 필요한 남은 핵심은 V63 정렬·pivot/부호 결합, 원본 색/alpha·시각의 실제 표현,
  흰 교차 스크류의 발생별 기여 식별이다. 사용자 화면 확인 전이라 시각 복원율은 산정하지 않는다.

## 사용자 첨부 두 장면의 원본 구성과 복원 방향

사용자가 명확히 한 목표는 직접 만든 현재 S를 평가하는 것이 아니라, **첨부한 원작의 검은 원형
공간과 유리 관통 광선을 최대한 충실하게 복원할 수 있도록 원본 구성을 밝히는 것**이다.
현재 편집본 대조는 뒤의 참고 정보로 한정하고, 원본30개의 관계와 실제 재질 입력을 기준으로 한다.

처음 두 첨부와 후속 검격 첨부를 직접 열람했다. 첫 장면에는 바닥에 놓인 넓고 검은 타원형 영역, 흐린 청색 외곽,
푸른 곡선과 화살 끝처럼 밝은 점, 내부 문양과 작은 빛들이 함께 보인다. 두 번째에는 캐릭터에서
옆으로 길게 뻗는 청색/보라 광선, 교차하는 백색 가는 선, 큰 삼각형 유리 조각과 작은 파편이 보인다.
사진의 픽셀 하나와 emitter 하나의 일치까지 확정한 것은 아니며 아래는 원본 재질·축·시각과
함께 좁힌 대응이다. 사진의 실제 capture 시각은 제공되지 않았다.

| 첫 장면의 층 | 원본에 있는 재료 | 복원할 핵심 |
|---|---|---|
| 검은 원형 바탕 | `_05/e41`, MaskControl SD320 | 원형 alpha와 검은 RGB를 따로 계산. 원본 alpha blend와 수평축 유지 |
| 푸른 원형 경계 | `_05/e38`, SpriteWave SD321 | polar UV, trail/noise, `fx_e_ring_039` dissolve, 별도 emissive texture |
| 내부 차원 문양 | `_05/e45`, SpriteWave SD322 | `fx_r_symbol_swp_01_cl`은 dissolve 입력. blue edge와 내부 투명도를 같은 Dynamic 곡선으로 변화 |
| 감기는 푸른 선/소용돌이 | `_05/e35`, Twirl SD323 | 회전·곡률 Dynamic, 음수 size 부호와 깊이 fade |
| 점·짧은 선·움직임 | `_05/e28/e48/e49/e4` | 원형 빛, cylinder 발생, 속도 정렬, 회전 vector field 입력을 각각 유지 |

첫 장면의 핵심4프로그램은 모두 source SceneColor를 직접 샘플하지 않는다. 검은 바닥을
만들기 위해 Alt V용 저장 SceneCapture나 별도 포털 renderer부터 만들 필요가 없다.
원본은 평면 sprite의 마스크·색·UV·움직임을 합성한다. 실제 장면 입력은 후술한 post3ch와
깊이 fade처럼 해당 분기가 요구할 때만 연결한다.

| 두 번째 장면의 층 | 원본에 있는 재료 | 복원할 핵심 |
|---|---|---|
| 길게 뻗는 광선/가는 흐름 | `_04`, `_02`의 e5/e13 `fx_e_pa_ht_18_4_tr` | 양/음 size인 한 쌍. source pivot과 반전 후 중심점 관계, PSA_Velocity와 모든 SizeMultiplyLife 모듈을 보존 |
| 중심 순간광·가는 flare | 두 system의 e11, e19/e21 | 짧은 원형광과 수명.6초의50×250cm flare. 해당 flare는 texture 없는 원본 절차식 |
| 장면이 어긋나는 청색/보라 가장자리 | 두 system의 e2 `Post3Ch`, e23/e22 `RGBSplit` | Post3Ch는 배경 RGB를 서로 다른 screen UV로 읽는 실제 장면 색분리. RGBSplit은 별도 강조층 |
| 큰 유리 조각/균열 | `_02/e0/e1`, `fm_d_crack_037`, LocalCrack V66 | 같은 geometry의 두 발생을 원본 위치·회전·수명으로 재생. normal/reflection/noise/Fresnel 원본 식 |
| 작은 파편 | 두 system의 e3/e4 | 서로 반대쪽으로 발생하는 sprite 파편. 큰 crack과 독립 |

shine 쌍의 원본 StartSize는 `_04`에서 `(50,+75,0)/(50,-75,0)`cm,
`_02`에서 `(30,+75,0)/(30,-75,0)`cm다. 이 값 뒤에 `×10`, `×(.5,1.5,1)`, `×.3`,
수명별 크기 곡선이 중첩된다. 초기 .5×.75m만 적용하거나 마지막 크기 모듈 하나만 채택하면
긴 광선의 형상이 사라진다. 두 입자의 수명은.22초이며 작은 초기 속도도 PSA_Velocity의
정렬축을 정의하므로 정지한 것처럼 보여도 삭제하지 않는다.

특히 **LocalCrack V66은 Q CubeSample과 같은 장면 굴절식이 아니다.** 선택 RT0는 normal,
고정 reflection texture, noise, view/Fresnel을 사용하며 SceneColor sample이 없다.
반면 **Post3Ch ALT195는 진짜 SceneColor RGB 분리식**이다. Dynamic 입력에 따른 서로 다른
UV offset에서 R/G/B를 샘플한다. 재질 이름이나 `distortion=10` 값만으로 둘을 같은 유리식으로
합치면 두 번째 장면의 색·투명감·윤곽이 달라진다.

전수조사의 비교 단위는 첫 장면의 원본8개, 중간 관통8개, 마지막 관통10개다. 화면 후처리3개와
light1까지 조사하면 총30개다. 사용자의 후속 목표는 이 중 **블랙홀·푸른 검격·주변 스크류의
세 시각 묶음을 완성하는 것**으로 좁혀졌다. full30은 원본 기여를 비교하는 조사 자료로 보존하고,
사용자가 편집할 핵심본은 그 세 묶음에 필요한 발생만 남긴다. 시각 묶음3개가 element 정확히3개나
shader 정확히3개를 뜻하지 않는다. 부차 파편·화면 후처리까지 전부 켜야 핵심 복원이 되는 것은 아니다.

## 현재 S의 정체

현재 S는 **2050220 / 일점 관통 / MomentaryRift**다.
`PlayerSkills.json:2292`의 S 슬롯과 `DimensionMaster.skillbindings.json:53`의
`pc_sp_m_00_sk_sk_momentaryrift`가 일치한다. actionDuration은1667ms이고 Server hitTime은1451ms다.
아래 원본 Effect notify 시각은 presentation 시각이므로 Server hitTime과 같은 필드로 취급하지 않는다.

08-06의 `2050550_S_RECONSTRUCTION_RESULT.md`는 당시 S로 배정했던 **찰나 / Super Instance**
기록이다. 그 문서의5초·시계·거대 검·175 layer 설명과 `차원술사_S00~S06.png`를 현재 S의
정본으로 사용하면 안 된다. 현재 slot → skillId → clip → raw notify 순서로 다시 연결해야 한다.

현재 S 원본의 주된 구성은 **수평 원형 마스크·소용돌이·차원 문양, 두 차례의 빛/관통 표현,
마지막 local crack 파편**이다. 활성 첫 LOD에는 cube/swing/slash 메시가 없다. 원본에서
확인되는 메시2개는 모두 LocalCrack이다. 공간에 생기는 구멍/차원문 같은 연출이라는 해석은
마스크·문양·축과 발생 순서를 종합한 추론이며, 원작 화면의 최종 모양을 확인한 판정은 아니다.

## G00. 원본을 element로 흡수한다는 뜻

사용자가 요청한 세 번을 넘는 분석은 서로 다른 질문으로 네 차례 수행했다.

| 분석 | 직접 확인한 질문 | 얻은 사실 |
|---|---|---|
| 원본 발생 재확인 | 무엇이 실제 enabled이며 어느 시각에 호출되는가 | raw44 blob 재검증, ParticleEffect7 활성/30 비활성, first LOD30 |
| 재질/프로그램 재확인 | 각 발생이 어떤 원본 PS와 입력을 고르는가 | MaskControl·polar SpriteWave·Twirl·Shine·LocalCrack·Post3Ch를 분리 |
| 실제 소비 경로 재확인 | 원본 값이 JSON에서 GPU까지 도착하는가 | SD descriptor/codec/renderer/PS 연결, signed size flip과 vector field 소비자 확인 |
| 첨부와 수치 재대조 | 두 화면의 층과 원본 size/pivot/수명·장면 입력이 맞는가 | shine 반대 부호 쌍·5개 size 모듈, 검은 mask식, SceneColor 쓰는 Post3Ch와 안 쓰는 LocalCrack 구분 |

이것은 네 번의 읽기/구조/수식 분석이다. 네 번의 Client 실행이나 원작과 동일한 화면을 검증한
것으로 기록하지 않는다. 최신 full이 조사 중 추가돼 실제 소비 경로도 한 차례 더 재감사한다.

`element`는 같은 역할의 입자를 생성하는 한 발생 단위다. 한 element에서 burst10개를 만들 수
있으며, 같은 material을 쓰더라도 위치·시각·모듈이 다르면 다른 element다. S의30개는 입자
최대30개라는 뜻이 아니다. renderer shape가 sprite이면 quad, mesh이면 CModel geometry가 된다.

각 element에서 복원할 정보는 다음 여섯 묶음이다.

| 묶음 | 이 예제에서 담는 실제 정보 |
|---|---|
| identity와 출처 | skill2050220, source notify ID, system/emitter path, 독립 stable element ID |
| 시계 | source cue 시작, emitter delay/duration/loop, burst 시각, 각 입자의 수명 |
| 공간과 움직임 | root/bone anchor, source cm→m, 축 변환, 초기 위치·속도·크기·회전·정렬 |
| 시간별 변화 | SizeMultiplyLife 전체, 색/alpha 곡선, DynamicParameter4채널, vector field |
| 재질 입력 | 원본 MIC와 parent, 선택 PS/VF, texture의 채널/색공간/sampler, named parameter |
| 그리기와 합성 | shader profile, alpha/additive, cull/depth, SceneColor/SceneDepth 및 최종 draw |

앞의 네 묶음은 주로 CPU의 particle 재생이 맡고, 마지막 두 묶음의 픽셀 계산은 GPU shader가
맡는다. UI의 Add Element는 이 저장 단위를 추가하는 명령이다. **Add Element를 누르는 것만으로
원본 재질 계산이 생기지는 않는다.** 반대로 shader를 추가해도 그 식을 선택할 element와
입력 수송이 없으면 한 픽셀도 바뀌지 않는다.

## G01. 원본 파일에서 실제 입력을 읽는 순서

실제 탐색 순서는 `PlayerSkills S → clip MomentaryRift → notify → ParticleSystem → 첫 LOD emitter
→ Required/Module → MaterialInstance → Parent/StaticSwitch → 선택 ShaderMap/VS/PS`다.
원본 ParticleSystem은 package에 있고, 이미지 파일 한 장에 이런 구조가 들어 있는 것은 아니다.

이번 조사에서는 보존된 `skill.2050220.source-receipt.json`,
`skill.2050220.action-cue-recipe.json`, `skill.2050220.external-module-closure.json`을 읽고 raw
payload의 활성값을 재확인했다. module의 다른 package 참조까지 따라가 실제 수치/곡선을 읽는다.
Required의 class default·archetype·instance를 합치되 명시0/false를 보존해야 한다.
현재 공통 importer의 상속 기본값 자동 복구 전체가 완료됐다고 가정하지 않는다.

재질은 이름이 같은 parent까지 찾는 것으로 끝나지 않는다. MIC의 numeric/texture override와
static switch를 합친 선택 키로 ShaderMap을 고르고, 해당 emitter의 vertex factory와 pass에
맞는 VS/PS를 읽는다. 이 작업의 기존 추출 코드는
`out/DimensionMasterSDRestore20260908/read_native_material_programs.py`에서
`decode_mic_target → select_unique_map_context → extract_selected_packed_dxbc → disassemble`로
이어진다. shader의 입력 signature와 cb/texture binding, uniform expression을 함께 읽는다.

검은 마스크의 원본 PS는 `d465be14468f91489ecb9184a42842a4`, 푸른 shine의 원본 PS는
`a14d57c96465f346be3437ad63c9ad10`다. 번호320/63은 이번 프로젝트의 dispatch 번호이며
원작 skill ID나 원작 shader ID가 아니다. 저장 참조에는 stable profile 문자열을 쓴다.

원본 CSO의 cb0나 texture t0 위치를 우리 엔진의 같은 번호에 그대로 꽂지 않는다. 원작이 cb0에
넣던 재질/시간/장면 입력을 이름과 의미로 해석한 뒤 기존 renderer의 parameter packet에
옮긴다. 원작 기계어의 픽셀 계산은 명시적 HLSL로 옮기고, 엔진 소유 depth/fog/scene 입력은
현재 엔진과의 변환 부분을 구분한다. 이 변환 부분의 미해석 값까지 원작과 동일하다고 부르지 않는다.

## G02. 첫 이미지의 검은 바탕 element 하나를 끝까지 따라간다

대상은 `_00_05/particlespriteemitter_41`이다. stable element ID는
`authored.source-particle.full-s.96e0413a5820d45c2ca1`이다. 새 full30에 들어 있는 이 한 행을
기준으로 이해한다. 사용자가 별도 독립 문서로 복사할 때에는 새 stable ID를 부여하고 원본 출처는
유지하며, 원본 문서의 ID를 무단 변경하지 않는다.

**element 쪽에서 결정하는 것.** 시작0초, emitter duration.2초, 입자 수명.45초,
수평축`epal_z`, 초기 크기와 크기/alpha 곡선을 `sourceRecipe`로 보존한다. .2초는 입자가
그때 모두 사라진다는 뜻이 아니다. 생성 창과 생성된 입자의 수명은 서로 다른 시계다.
CPU playback은 각 입자의 생존 여부·크기·색·UV·정렬을 계산해서 draw 입력으로 보낸다.

**재질이 선택하는 것.** `templateId=effect.source_material`, 원본 MIC
`fx_m_mi_01.fx_mi.fx_e_pa_gl_10_1_tr`, native profile
`effect.ue3.sd-320-native.v1`, render profile `alpha_one_sided_depth_read`다.
`textures=[]`가 정확한 입력이다. 존재하지 않는 검은 PNG를 만들어 Base에 넣지 않는다.

**C++ descriptor가 옮기는 것.** `Effect_DimensionMasterSDMaterial.h`의320 descriptor가
MIC/parent/profile/shape/switch를 한 프로그램에 묶고 named parameter의 목적 위치를 지정한다.

| JSON의 원본 이름/값 | 현재 shader packet 위치 |
|---|---|
| `mask_hardness=0` | float4 row0의y |
| `mask_intensity=30` | row0의z |
| `mask_power=4` | row0의w |
| `mask_color=(0,0,0,1)` | row2 전체 |
| `selectioncolor=(0,0,0,1)` | row3 전체 |

`Build_DimensionMasterSDParameters`는 이름의 유일성·개수·finite 값을 검사해 임시 Candidate에
채운다. 성공할 때만 Output을 교체한다. `Has_DimensionMasterSDMaterialContract`는 실제
element가 sprite인지, 같은 원본 MIC인지, texture/switch/renderProfile이 맞는지도 검사한다.
`Effect_DocumentCodec`가 이 검사를 소비하므로 잘못된 ID만 붙인 문서를 정상 native로 취급하지 않는다.

**renderer가 GPU로 보내는 것.** `CEffectDocumentRenderer`는 profile320을 고르고 위 packet을
준비한다. 현재 SD/V 프로그램은 동시에 선택되지 않으므로 기존 V packet을 재사용한다.
`Shader_VtxEffectParticle.hlsl`의320~323 분기가 UV·입자색·Dynamic·basis·projection을
`SD_NATIVE_INPUT`에 넣고 `Shade_EffectDimensionMasterSDNative → SDNative320`을 호출한다.

**pixel shader의 실제 의미.** 현재 원본 유효값인 UV scale1/offset0/rotation0에서
`d=length(UV-.5)`, `m=saturate(1-2d)`, `alpha=saturate(m^4*30)*particleAlpha`, `RGB=0`이다.
실제 함수에는 UV 변환과 engine opacity/fog 입력도 있으며 이 식은 그 유효값을 대입한 설명이다.

particleAlpha를1로 둔 수치 예에서 중심d0은 alpha1, d.30은 alpha.768,
d.45는 alpha.003, d.50 바깥은0이다. 원형 중앙이 검고 가장자리가 부드럽게 사라진다.
일반 alpha 합성 `최종색=검은색*alpha+기존장면색*(1-alpha)`가 바닥을 어둡게 만든다.
additive로 바꾸면 검은 RGB를 더해도 장면이 어두워지지 않아 이 역할을 잃는다.

**첫 이미지 전체를 만드는 것.** 검은 바탕 위에 e38 ring, e45 문양, e35 twirl,
e28/e48/e49/e4의 빛/선을 원본 시각·색·정렬로 합친다. SD321/322의 polar UV는 평면 좌표를
중심에 대한 각도와 반경으로 바꿔 texture가 원을 따라 흐르게 한다. dissolve는 문양을
지우고 드러내며 `edge_color`가 그 문양 경계를 푸르게 만든다. SD320 하나로 전체 그림을
만드는 것이 아니라 각 층이 맡은 일을 유지한다.

## G03. 두 번째 이미지의 푸른 검격 한 쌍을 끝까지 따라간다

첫 관통은 `_00_04` e5/e13, 후반은 `_00_02` e5/e13이다. 같은 원본 MIC
`fx_m_mi_02.fx_mi.fx_e_pa_ht_18_4_tr`와 V native63을 쓴다. 이 경우 **새 shader 함수가 아니라
이미 있는 원본 식에 새 occurrence의 데이터가 들어가는 것**이다.

element의 시작은 각각.394240초/1.093617초이고, 쌍의 source size는 ±Y로 구분한다.
Required 원문을 재확인한 pivot은 e5=(.5,.9), e13=(.5,.1)이며 두 system이 동일하다.
중간 consumer 보고서의 '둘 다 .9'는 부분 출력 누락으로 생긴 조사 오류여서 철회했다.
e13의 .1은 음수 크기로 추론한 값이 아니라 저장된 별도 입력이다. world 크기를 양수로
GPU에 보내더라도 UV 반전 부호는 따로 유지하며, pivot을 음수 부호로 임의 재계산하지 않는다.
현재 playback은 source signed size를 보존하고 atlas 셀 안에서 UV를 한 번 뒤집는 경로가 있다.
원작의 axis/basis와 renderer의 속도 정렬축까지 맞춰야 날이 진행 방향으로 놓인다.

`_04` 쌍을 예로, 초기 `(50,75)`cm에 상수 size 모듈만 먼저 누적하면
`(50,75) *10 *(.5,1.5) *.3 = (75,337.5)`cm다. 그 뒤 별도의 수명 곡선과 cue scale1.2가
작동한다. renderer가 size 모듈을 하나만 사용하거나 생략된 분포를0으로 읽으면 재질이 맞아도
광선이 짧아지거나 사라진다. 여러 모듈은 중복 쓰레기가 아니라 원본 계산의 곱셈 단계다.

V63에서 필요한 texture는 세 개다. `14.map_d=noise009`, `01.map_a=noise014`,
`06.map_b=noise021`로 이름과 샘플 순서/채널을 보존한다. 명칭이 Noise라고 임의로 모두
linear로 바꾸지 않으며 현재 descriptor의 색공간 근거도 따로 확인한다.

원본 shader는 noise009로 UV를 흔들고, 서로 다른 scale/pan의 noise014·021을 곱해 흐름을
만든다. 여기에 UV의 중심/끝 감쇠와 power를 적용해 빛의 실루엣을 좁히고, SceneDepth와
입자 깊이 차이로 접점 alpha를 줄인다. particle RGB의 시작값 `(2,4,20)` 같은 HDR 입력과
수명별 색/alpha가 청색 광선의 변화에 기여한다. PNG alpha만 읽거나 툴 emissive를 중복 곱하면
원본의 빛/투명도 관계가 바뀐다.

### G03-1. 진한 청색·하늘색·파스텔이 같이 생기는 실제 식

`VNative63`의 선택 분기는 `use_multyply_noisecolor=false`다. 원본 texture009의 RG는 UV 왜곡에,
texture014와021의 R은 서로 다른 panning 무늬의 곱에 사용한다. texture 이름만으로 역할을 바꾸지 않는다.
UV가0~1인 범위에서 engine fog를 identity로 둔 현재 adapter와 원본 유효값을 정리하면 다음과 같다.
아래 의미식은 원본 HLSL의 near-zero guard와 좌표 변환 일부를 생략한 설명이며 대체 구현 코드가 아니다.

```text
a, b = 왜곡된 UV에서 각각 pan/scale하여 읽은 noise014.r, noise021.r
n = abs(a*b)^0.7 * 5
width = 1 - abs(2*u - 1)
shape = (width * 0.5 * (1+v) * sqrt(v))^3.4
light = n * shape

RGB = ParticleColor.rgb + light * (1.5,1.5,1.5) * 5
    = ParticleColor.rgb + light * (7.5,7.5,7.5)

bodyAlpha = saturate(shape * 0.3 + light)
endAlpha = saturate((1-v)^2 * 40)
alpha = saturate(bodyAlpha * depthFade * endAlpha * ParticleAlpha)
```

birth의 ParticleColor는 `(2,4,20)`이다. 첫 `_04` 쌍의 ParticleAlpha는.6이며 후반 `_02`
쌍은 보완된 class default1을 사용한다. 두 발생의 투명도를 같은 값으로 통일하지 않는다.
예를 들어 light가0/.2/1인 위치의
합성 전 RGB는 각각 `(2,4,20)`, `(3.5,5.5,21.5)`, `(9.5,11.5,27.5)`다. **짙은 청색 바탕 위에
무늬가 있는 곳만 무채색 빛을 더하므로 공간적으로 채도가 달라진다.** alpha는 RGB와 별도 식이며
가장자리·끝·장면 접점·수명에 따라 변한다. 밝은 배경과 섞이는 양, 이후 exposure/tonemap/Bloom과
다른 발생의 합성까지 더해져 최종 화면의 옅은 하늘색·보라빛 인상에 기여할 수 있다.

이것으로 원작 스크린샷의 모든 흰 선과 파스텔 영역이 V63 하나라고 확정한 것은 아니다.
특히 화면상 앞/뒤 어느 위치에 색이 남는지는 source UV, pivot, signed size, velocity basis까지
함께 결정한다. 근거 없이 색 gradient texture를 새로 그리거나 HDR particle color를[0,1]로
자르는 작업, 밝은 무늬와 base color를 전부 곱셈으로 바꾸는 작업은 이 선택 원본 식을 훼손한다.

주변 흰 교차 스크류는 후속 첨부에서 확인된다. 활성 S에 별도 helix mesh는 없으며, shine의
무늬/반전 쌍, flare, RGBSplit, crack의 밝은 모서리를 분리해 기여를 판별해야 한다. 블랙홀 단계의
Twirl e35를 관통 단계의 흰 스크류와 같은 occurrence라고 단정하지 않는다. 두 단계의 시각이 다르다.

후속 원본 DDS/PS 조사에서 V64 RGBSplit의 `fx_e_atypical_028`은8방향 starburst texture이며
V65는 texture 없는 원형·가느다란 직선 십자 flare였다. 이 둘의 식에 helix geometry나 곡선 경로는
없다. V63은 왜곡된 noise와 길게 늘어진 shine의 흰 세부 무늬라는 점에서 더 유력한 후보지만,
사진의 매끈한 교차선 전체를 이 한 식에 확정하지 않는다.

### G03-2. 새 픽셀 셰이더보다 먼저 확인된 검격 정렬 연결 결함

V63의 선택 offset-center dynamic vertex factory는 원본 VS
`2dd6d96a7e6c974fac82106409a5b9b8`를 참조한다. Q에서 복구한 VS와 ID 및 보존 ASM의
SHA256 `af877d539c68342724954c1afdceab7316427e793585b191a53b186bc5363368`가 같다.
해당 VS의 속도 정렬 분기는 카메라 방향과 motion으로 면의 가로축을 만들고 세로축을
motion에 맞춘다. pivot을 뺀 UV에 source signed size를 곱하는 단계가 뒤따른다.

최초 조사 당시 `Effect_DocumentRenderer.cpp:3729`의 native velocity basis 적용 목록에는
Q/WR의 일부 profile만 있고 **V63은 없었다**. 같은 `PSA_Velocity`인 S shine이 카메라 평면의
atan2로 회전하는 분기 차이를 재현하고, 후속 G31에서63을 기존 native 경로에 연결했다.
새 Shine PS를 추가한 것이 아니라 원본 VS basis의 해당 consumer 연결을 교정한 것이다.

원본 실행 중 CPU selector CB/OldPosition을 회수한 것은 아니므로 입력 동일성 전체와
화면에서의 기여량은 별도 검증이다. 적용은 정확한 V63/PSA_Velocity 범위에서 검토하고,
모든 sprite의 정렬을 한꺼번에 바꾸지 않는다. G31의 실제 수정과 검증 범위는 문서 앞에 기록했다.

## G03-3. S 전체 복원에 새 shader 10개가 필요한가

01:52~01:55 재계수의 기준은 원본 selected pixel shader의 고유 ID다. 같은 shader를 다른
occurrence가 여러 번 쓰면1개로 세며, vertex shader·조명 simulation·HLSLI 파일 개수와 구분한다.

| 범위 | 발생 수 | 고유 원본 PS | 현재 코드에서 함수 확인 |
|---|---:|---:|---:|
| Sprite24 + Mesh2 | 26 | 14 | 14 |
| RGBNoise2 + ZoomBlur1 | 3 | 2 | 2 |
| PointLight | 1 | PS로 계수하지 않음 | 원본 light CDO 입력 미확정 |

particle26개는 MIC/shape14조합, parent 기준 family13개다. SpriteWave321/322처럼 같은
parent가 다른 static switch/PS를 고를 수 있어 family와 PS 수가 다르다. particle PS14개는
기존 V/ALT 재사용10개와 S용으로 소스 연결된 SD320~323의4개로 나뉜다. 현재16개 PS 함수
존재를 다시 확인했고, 이 분모에서 **아직 코드에 없는 새 PS는0개로 식별됐다.**

이는 shader 수량 조사다. 원작 최종 출력과 동일한 계산·모든 입력·빌드·실행을 통과했다는
주장은 아니다. 원본 PS는 있으나 basis/Dynamic/시간/장면 입력을 잘못 공급한 결함은 그대로
남을 수 있다. '새 family가 필요하다'는 판단은 generic 표현만으로 MaskControl·SpriteWave·
Twirl·Shine을 모두 대신할 수 없다는 의미에서는 타당하다. 하지만 이미 다른 구현 세션이
SD4개를 연결한 현재 상태에서 원본 근거 없이10개 PS를 더 만드는 계획은 맞지 않는다.

세 핵심 묶음의 먼저 볼 식은 블랙홀 SD320/321/322/323과 검격 V63이다. 주변 흰 스크류의
기여를 occurrence별로 식별한 뒤 필요한 기존 식을 더한다. 별도 원본 PS가 확인되거나 현재
식의 오역이 재현될 때만 그 분기를 추가·교정한다. shader 수를 완성 목표로 정하지 않는다.

이 쌍을 그린 뒤 flare, RGBSplit/Post3Ch, LocalCrack2개와 작은 sprite 파편을 같은 시계에
합친다. LocalCrack은 `.wmodel` geometry를 기존 CModel로 읽고 cm 모델에 `.01`을 한 번
적용한 뒤 particle scale/회전을 사용한다. source StartSize를 cm라고 한 번 더 `.01`하면 안 된다.
반사처럼 보이는 삼각형 조각과 길게 늘어진 빛은 서로 다른 입력이다.

## G04. 실제 element·shader 추가 작업의 완료 경계

새로 추가할 때는 다음 순서로 기존 경로를 확장한다. 이번 설명에서는 이 순서를 새로 실행하지 않았다.

1. source occurrence를 식별해 기존 authoring JSON에 stable element로 넣는다. 원본 모듈/곡선,
   texture·mesh 상대 ID, MIC와 native profile, 시간·축·단위를 함께 보존한다. 같은 원본 system이
   다른 notify에서 다시 호출되면 별도 occurrence로 만든다.
2. 같은 selected PS/VF/input 계약이 이미 있으면 descriptor/parameter 연결을 재사용한다.
   새 식인 경우에만 SD320처럼 native HLSL 함수와 C++ descriptor를 추가한다. shader를 한 개
   더 만든다는 것과 입자30개를 만든다는 것은 관계가 없다.
3. codec의 계약 검증, renderer의 profile 선택·parameter/texture/scene bind, shader entry의
   dispatch를 모두 연결한다. 어느 한 군데만 추가하면 저장은 되지만 재생에서 거절되거나 다른 식을 고른다.
4. 새 H/CPP/HLSLI가 있으면 `.vcxproj`와 `.filters`에 필요한 항목만 등록한다. 새 JSON은
   `96.DataFiles`의 None 항목이며, Recovery Effect는 기존 SourceIndex/Sequencer로 연결한다.
   native HLSLI는 제품 shader entry에 include되어 FXC 산출 CSO에 포함돼야 한다.
5. 실제 codec의 Load→Save→Reload, source/texture/field 참조, shader 최소 컴파일과 실제
   renderer 입력을 확인한다. missing resource/shader 단계가 실패하면 기존 유효 문서를 보존한다.
6. 사용자 재생으로 e41 단독, e38/e45/e35 각각, 관통 shine 쌍과 스크류 후보를 같은 timeline에서
   비교한다. full30의 발생별 켜기/끄기로 기여를 식별한 뒤 핵심본에 필요한 층만 남긴다.
   이때 재질/축/시계가 맞은 뒤 크기·색·밀도를 튜닝한다.

이렇게 해야 각 element를 어디서 얻었고 무엇을 실행하는지 다시 추적할 수 있다. generic fallback,
지원 안 되는 module 삭제, 모양만 비슷한 cube 교체로 성공처럼 숨기지 않는다.

01:37 이후 다른 세션이 새 S full30을 Data에 설치했다. 이 문서의01:34 snapshot과 혼합하지 않는다.
01:45까지의 읽기 전용 조사에서 `_05/e4`의 local vector field는 원본 object path만 있고 필수
`vectorfield.assetid`가 없어 codec의 `Portable authored particle local vector field asset is missing or unsafe.`
조건에 해당했다. 같은 원본 `fx_o_w_01.fx_o_vectorfield_02`는 F/AltV에서 이미
`Effect/DimensionMaster/VectorFields/fx_o_w_01.b7a4be9b5da97572.wvectorfield`로 연결돼 있다.
새 물리 시뮬레이터를 만드는 작업이 아니라 이 occurrence의 정확한 Resource 연결을 완성할 사례다.
**01:52:16 다른 구현 세션의 갱신 뒤 asset ID가 추가됐고, 후속 실파일 재확인에서도 확인했다.**
이 누락을 현재 미해결로 보고하지 않는다. 최신 codec 실행·빌드는 이번 조사에서 확인하지 않았다.

## 원본의 활성 범위와 시간

원본 raw cue44개 중 ParticleEffect 계열 활성7개, 비활성30개, 기타 활성 여부 미확정7개다.
활성7개가 만드는 첫 LOD 발생30개는 sprite24 + mesh2 + screenPost3 + light1이다.
원본에 남아 있는 비활성 변형을 모두 켜서 전체 복원으로 부르지 않는다.

| 시각 | 원본 system/발생 | 구성 |
|---|---|---|
| 0.000초 | `MomentaryRift_00_05` | sprite8: 원형 마스크·ring·문양·twirl·빛/선 |
| 0.000초 | `Par_J_RGBNoise_01` | screenPost1 |
| 0.394240초 | `MomentaryRift_00_04` | sprite8: 중간 관통/광선/색 분리/잔광 |
| 0.393503초 | `Par_J_RGBNoise_01` | screenPost1, 앞 호출과 다른 occurrence |
| 0.395940초 | `Par_J_ZoomBlur_01`, `Par_MP_Light_01` | screenPost1 + light1 |
| 1.093617초 | `MomentaryRift_00_02` | sprite8 + LocalCrack mesh2 |

첫 system의 emitter38/45/41/28/35에는 원본 `OrientationAxisLock=epal_z`가 있다.
원본 Z축에 수직인 수평면 표현이며, 일반 camera-facing quad로 바꾸면 구도 자체가 달라진다.
시간과 같은 texture의 반복 사용만 보고 occurrence를 합치지 않는다.

## S에서 먼저 식별할 원본 재료

아래 emitter 번호는 `FX_PC_SWP_02.Par_R_SWP_MomentaryRift_00_05` 안의 source emitter 이름이다.
툴에서 삭제/정렬 후 보이는 순번이나 다른 system의 같은 emitter 번호와 혼동하지 않는다.

| 원본 emitter | 정체와 입력 | 선택 native 식 |
|---|---|---|
| 41 | texture 없는 검은 원형 마스크. `MaskControl`, `mask_color=(0,0,0,1)`, power4, intensity30 | SD320 |
| 38 | `SpriteWave_24_02`: polar UV로 감은 ring, noise/dissolve/추가 발광 texture. `fx_e_ring_039` 사용 | SD321 |
| 45 | `SpriteWave_30_01`: trail/noise에 `fx_r_symbol_swp_01_cl`을 dissolve 입력으로 쓰는 차원 문양 | SD322 |
| 35 | `Twirl_03_09`: 회전/곡률/중심 mask/깊이 fade가 있는 소용돌이. 원본 회전율 -2회/초 | SD323 |
| 28·48·49·4 | 원형 빛, 공격 빛, split line, 원형 보조층 | 기존 V/Alt native 후보 재사용 |

SD320은 검은 texture가 누락된 상태가 아니다. 유효 파라미터에서 의미식을 정리하면
`RGB=0`, `alpha≈saturate(saturate(1-2*length(UV-.5))^4*30)*particleAlpha`다.
UV 회전·scale·offset과 engine opacity 입력은 원본 계산 경로에 별도로 존재한다.
검은 중심을 alpha 합성으로 만드는 것이 이 분기의 의도다.

SD321/322는 같은 SpriteWave parent지만 같은 프로그램이 아니다. 둘 다 polar·noise·dissolve를
사용하되321은 추가 emissive texture를 켜고322는 끈다.321은38 named parameter/4 texture,
322는34 parameter/3 texture를 요구한다. 선택 PS도 서로 다르다. 둘 다 SceneColor를 읽지 않는다.
SD323만 이4개 중 실제 SceneDepth sample을 요구한다. texture 없는320도 현재 adapter에서는
viewport 입력을 위해 depth resource가 필요하므로 texture0개와 runtime 입력0개는 다른 뜻이다.

## 참고: 사용자 저장본과 원본 조사 범위의 구분

01:34:59 KST에 다시 읽은 `Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json`은
**12개 편집본**이다.9개는 source sprite이고3개는 `effect.standard`의 손저작 메시다.
현재 native profile은0개이며 circle3/grouped-translucent5/shine1의 이전 표현을 사용한다.

손저작 `manual.hit1`은1초에 원뿔 `fm_d_cone_003` 1개와 나선 `fm_d_helix_036` 2개를 만든다.
이3개는 사용자가 만든 표현이며 원본 S 발생 목록과 구분한다. 원본의 핵심 시작부
ring38·문양45·mask41, 최종 LocalCrack2개가 현재 제품 문서에는 없다. 이 대조는 사용자의
제작 방향을 잘못됐다고 판정하는 것이 아니라, 원본 전수조사의 분모를 구분하기 위한 기록이다.

| 경계 | 조사 시점의 상태 |
|---|---|
| 제품 `.unified` | 위12개. SHA256 `f61bb3a9b91eb2772ea659a3a9724472163a92c42ade51b66c89bf577e575ec9` |
| 원본 full30 | `out/DimensionMasterHandTuning20260908/full_source_composition/` 및 deferred에 존재 |
| core 튜닝3 후보 | LocalCrack2 + SD322 문양1. ring/mask/twirl은 포함하지 않는 축약본 |
| Data/Authored S full/tuning | 01:34:59 KST에는 둘 다 없음 |
| SD320–323 C++/HLSL | 다른 구현 세션이 codec/renderer/particle shader 연결을 소스에 추가한 상태 |
| 최신 EXE 포함·화면 | 이번 조사에서 빌드·실행하지 않았으며 확인하지 않음 |

core3개는 작은 실험의 시작점으로는 쓸 수 있지만, 이3개만 보고 S 전체의 정체를 판정하면
다시 시작부의 원형 경계·검은 중심·소용돌이를 잃는다. 전체30 보존과 핵심 선별은 별도다.

## A 초승달과 R의 검은 중심·보라 경계

Sprite/Mesh는 geometry 구분이다. 같은 sprite라도 MaskControl, SpriteWave, SpriteInvert,
procedural glow는 다른 계산을 실행한다. cube/crack의 유리 계산을 모든 검격에 적용하지 않는다.

| 재료 | 조사된 원본과 현재 차이 |
|---|---|
| A 초승달 메시 | 원본 MakeFlow·Swing·LinearFlow 계열이다. 진행 중인 복구는 WR277/278/279와280 alias를 연결한다. 현재 A `_튜닝`2개는 `effect.standard + fm_h_swing_02 + fx_h_wave_04`의 손조립 호이며 full의 native 복구와 다르다. |
| R 검은 중심/발광 경계 | WR208 `SpriteInvert`는 SceneColor 반전이 아니다. flow와electric mask로 alpha를 만들고 내부 RGB를 깎아 검은 중심과 발광 경계를 분리한다. 경계 색은 particle RGB 입력에도 의존한다. |
| S 검은 원형 | SD320의 texture 없는 radial mask. R SpriteInvert와 별도 식이다. |
| S ring/문양 | SD321/322의 polar SpriteWave. R의 SpriteWave parent와 관계가 있어도 static switch·VF·선택 PS를 대조해야 재사용할 수 있다. |

WR208에서 원본 파라미터를 대입하고 시간항을0으로 둔 예는 다음과 같다.
`z=pow(flow.r,Dynamic.x)*electric.r`, `A=saturate((10*z)^2*3)`,
`rim=saturate(A-9*z)`, `alpha=saturate(A*particleAlpha)`다.
`z=.06`이면 alpha1과 rim.46으로 밝은 영역이 남고, `z=.12`이면 alpha1·rim0으로 검은 내부가 된다.
이는 검은색 자체가 정상일 수 있다는 근거이지, 사각형 실루엣·반복 띠·찢어진 모양까지 정상이라는 뜻은 아니다.

이전 세션의 사용자 첨부 `codex-clipboard-f77498f4-1820-43cb-983f-81163fe69e16.png`와
`codex-clipboard-63e226f6-957a-4579-9c7c-5d2b140153ce.png`를 직접 열람했다. 원작 참고에는
가느다란 어두운 중심과 이어지는 밝은 보라 경계가 보이고, 복구 화면에는 검은 사각 띠가 반복되며
양옆이 끊겨 보인다. material 계산, UV/polar, alpha, DynamicParameter, particle 정렬/크기를
분리해서 점검해야 한다. 한 이미지로 그 픽셀을 만든 source occurrence 하나를 확정하지 않았다.

다른 세션의 R/A 수정은 generic UV pan·emissive의 native 중복 적용 교정과 A 누락 native 식 연결이다.
이것으로 첨부 화면의 결함이 실제 해소됐다는 사용자 판정은 아직 없다.

## 복원할 때의 확인 순서

1. 현재 S2050220 full30을 기존 Recovery Effect로 연결하고 원본12개 편집본과 구분한다.
2. 시작부 `_00_05`의41(mask),38(ring),45(문양),35(twirl)를 각각 같은 시각에 식별한다.
   `epal_z`, polar UV, DynamicParameter 곡선과 alpha blend를 함께 확인한다.
3. 네 요소를 함께 재생해 검은 중심·바깥 ring·문양·소용돌이의 관계를 사용자가 확인한다.
4. 0.394초와1.094초의 shine 쌍, flare·RGBSplit/Post3Ch를 따로 비교해 푸른 본체와 흰 스크류의 기여를 식별한다.
5. 핵심본에는 블랙홀·검격·스크류에 필요한 층을 남긴다. LocalCrack·작은 파편·RGBNoise·ZoomBlur·light는
   full 비교 자료에 보존하며 사용자 핵심 목표에 대한 기여가 있을 때만 핵심본에 포함한다.

위 순서는 실행한 검증이 아니라 조사에서 도출한 후속 복원 순서다. 지금30개를 전부 삭제하거나
cube2개로 바꾸는 작업은 하지 않았다. 기존 공용 renderer와 native descriptor를 확장하면 되며
스킬 전용 두 번째 runtime은 필요하지 않다.

## 조사 증거·검증·공유

아래 항목은 G31 구현 이전의 조사 범위다. 이후 수행한 renderer 수정·최소 컴파일·CPU 검증은
문서 앞 G31 결과가 정본이며, 아래의 초기 미실행 항목을 현재 미완료 상태로 혼용하지 않는다.

- 현재 PlayerSkills/animation binding/authored JSON과 원본 inventory/native contract를 parse하고
  ID·개수·시각·shape·material 연결을 대조했다. source cue 활성은 raw boolean을 가진 기록을 기준으로 했다.
- 원본 추출 자료의 보존 위치는 `C:/Users/user/.codex/worktrees/dm-ba-effect-pr/LostArk/Data/Effects/Imported/DimensionMaster/`다.
  현재 checkout의 같은 Imported 경로에 있다고 가정하지 않는다. 실제 보존 파일 접근을 확인했다.
- native320 mask와 WR208 alpha/RGB 계산을 실제 HLSL/선택 PS 기록으로 대조했다.
- 후속으로 V63의 RGB/alpha 식, Required pivot .9/.1, 선택 원본 VS와 Q ASM 동일성,
  현재 renderer의 profile63 적용 부재를 재확인했다. 최신 vectorfield.assetid 추가도 실파일에서 확인했다.
- `s_shader_counts.json`의16행을 parse하고 각 실제 HLSLI 함수 존재를 재확인했다.
  재질 발생29개와 light1이 full30과 일치한다. 원본 PS 함수 수와 실행 완료 수를 구분한다.
- 조사팀의 Resource 참조 검사에서 현재 S 문서12개 고유 참조와 full 후보20개 고유 참조의 누락은0이다.
  파일 존재는 material 입력·GPU draw·화면 품질의 성공 증거가 아니다.
- 코드·Data·Resources payload 수정, publisher, 컴파일, Client/UI 실행, 화면 캡처는 수행하지 않았다.
  문서 변경만 diff-check한다. 기존 dirty worktree를 stage/commit/push하지 않는다.
- 조사 RESULT와 V2 노트에 `git diff --no-index --check -- NUL <문서>`를 실행해 공백 오류가
  출력되지 않음을 확인했다. 두 파일은 현재 untracked라 no-index 비교를 사용했으며 변경 존재에
  따른 exit1과 Git의 LF→CRLF 안내가 있었다. 컴파일 통과나 Client 실행 성공으로 기록하지 않는다.
- 팀 LAN 준비 결과는 `server-host`, TCP7777 LocalSubnet 방화벽 준비, endpoint `not-listening`이었다.
  사용자가 실행할 때 Visual Studio `Server + Client` profile을 사용한다.
- Resources의 신규 설치·교체와 Drive 업로드는 없다. 기존 상대 ID는 아래 전수 목록에서 확인한다.

세부 전수조사 자료:

- [S 원본 발생 전수 목록](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/s_source_inventory.md)
- [S 원본 구조화 목록](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/s_source_inventory.json)
- [현재 S runtime 연결 차이](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/s_runtime_gap.md)
- [네 번째 소비 경로와 후속 VS 재감사](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/fourth_pass_consumer_trace.md)
- [S 고유 PS·family 계수와 함수 위치](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/s_shader_counts.json)
- [초승달·검은 sprite 재질식](C:/Users/user/Desktop/LostArk/out/DimensionMasterSForensic20260909/crescent_black_material.md)

참조 정본: [렌더링·이펙트 복원 V2](../렌더링이펙트복원V2.md),
[진행 중인 복원 PLAN G29](../09-08/2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_PLAN.md),
`out/DimensionMasterFullRestore20260908/skill.2050220.inventory.json`,
`out/DimensionMasterSDRestore20260908/native_runtime_contract.json`,
`out/DimensionMasterSlashFix20260909/ra_handoff.md`.
