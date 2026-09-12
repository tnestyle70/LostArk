# 차원술사 Alt V 시작 장면·큐브 구현 결과

## G00. 현재 소스와 후보 범위

현재 두 Authored 문서는 기존 실행 Client와의 호환을 위해 작업 전 사용자 원본으로 복귀했다.
아래 데이터 변경은 out 후보에만 보존되어 있으며 설치되지 않았다. Codec/Renderer/Shader 소스
구현과 out 검증은 유지한다.

실제 입력은 `2050540 -> pc_sp_m_00_sk_sk_super_timewave ->
effect.dimensionmaster.skill.2050540.full.restore`다. 사용자 편집 이후 full 309행은 그대로
보존하고 tuning에 남아 있던 camera capture source emitter18/31만 되살려311행으로 만들었다.
tuning은26행을 유지한다. 과거 문서의318행을 복원 기준으로 사용하지 않았다.

full의 `fm_h_box_01_1.wmodel`47개 occurrence 중 생략된42개의 `detail.mesh.modelPreScale`만
.01로 명시했다. 설치된 정적 메시의 raw bounds는 각 축[-25,25]cm이며, mesh StartSize는
무차원이므로 geometry에서 cm→m 변환이 한 번 필요하다. 기존 명시5개 .1/.123/.5 값,
모든 기존 position/rotation/scale·source distribution·visibility·tint는 보존했다.

두 문서의 `altv.source.notify036.cube`에는 기존 ALT178 native material을 연결했다.
full의 OPAQUE만 해당 재질의 TRANSLUCENT carrier로 바꾸었으며 cue transform, opacity,
colorMultiply, asset pretransform은 유지했다. tuning의 기존 .22 opacity/청색 tint도 유지한다.

## G01. 화면 축소와 기존 렌더러 연결

`Effect_DimensionMasterALTVMaterial.h`는 ALT178과 정확한 `sk_swp_cub_00_sk.wmodel`의
교집합만 animated ModelCue로 허용한다. Codec은 모델·named texture·switch·parameter를
검사하고 Renderer의 기존 ModelCue stage/clone/native surface 경로가 이를 소비한다.
`Shader_VtxAnimMeshBinary.hlsl`의 기존 skinned VS가 ALT178 함수를 선택한다. 별도 모델
런타임이나 Engine public ABI, 저장 필드를 만들지 않았다.

ALT178의 texture2는 해당 effect occurrence가 시작될 때 보존한 SceneHDR snapshot이다.
static camera mesh와 animated central cube가 같은 snapshot을 사용하고, 재질의 DDS
fallback으로 제품 장면을 대신하지 않는다. ModelCue만 capture를 요구하는 경우에도
기존 capture lifetime을 연결했다. 같은 문서의 NORMAL phase에서는 ModelCue/mesh를
먼저, sprite 등 나머지 family를 뒤에 제출한다. 평가 행의 순서와 occurrence 통계는
기존 순서로 검증하며 각 occurrence는 한 번만 draw/count한다. 다른 문서와 world-mark의
제출 순서는 그대로다.

원본 capture-view 행렬은 기존 자료에도 미해석 상태다. 따라서 native RT0식과 기존
cube-face UV adapter를 유지하고, 요청한 시작 화면 전체 프레이밍을 명시적인 project
camera-fit adapter로 구현했다. stable camera emitter18/31 + ALT178만 대상으로 한다.
CModel의 preScale 적용 bounds8개 모서리를 현재 view로 변환하고 projection _11/_22와
front depth로 시작 화면을 채운다. 공통 root elapsed는 현재 .0299999993→2초다.
각 particle 생성 시간으로 확대를 재시작하지 않으며 smoothstep으로 기존 evaluated World에
안착한다. 끝에서는 기존 World를 byte 그대로 반환한다. near plane, 영 bounds, 특이 view,
비유한 matrix를 만나면 기존 World를 유지한다. native instanced mesh와 단일 mesh fallback이
같은 함수를 사용한다.

## G02. 중앙 큐브 시간 단위와 Resources 경계

설치된 `Effect/DimensionMaster/Models/SK_SWP_CUB_00/sk_swp_cub_00_sk.wmodel`은 이미 미터
단위의600 vertex·52 skeleton bone을 가진다. 여기에 .01을 추가하지 않았다. 현재 animation은
3333.333251953125tick/1000Hz로 저장됐지만 CAnimation은30Hz로 소비하므로 약111초로
늘어진다. 기존 `Tools/ActorXAssetCooker/retime_wmodel_ticks.py`로 모든15453 key time을
30Hz로 바꾼 후보를 아래에 만들었다. 실제 clip 길이는3.33333325초로 유지된다.

`out/DimensionMasterALTV20260912/sk_swp_cub_00_sk.wmodel`

사용자 Server/Client가 실행 중이므로 Resources의 원본과 EngineSDK/DLL/제품 CSO는
교체하지 않았다. 이 후보를 실제 Resources에 반영하는 작업과 제품 빌드는 root가 사용자
실행 상태를 확인한 뒤 연결해야 한다. 현재 실행 중인 Client에 적용됐다고 기록하지 않는다.

## G03. 실행한 검증

| 검사 | 실제 결과 |
|---|---|
| JSON 값 비교 | full309→311, tuning26 유지. 기존 각 row 전체 값 일치; full42개 생략 preScale와 cue material/alphaMode만 예외 |
| Debug 개별 C++ 컴파일 | 현재 Codec/Renderer 및 focused 의존 소스 성공. 기존 SDK 인코딩/BOOL 경고만 존재 |
| Shader fxc fx_5_0 /Od | animated ModelCue, ALTV Mesh128, ALTV Particle128 모두 out compile 성공. 기존 source shader 경고는 남음 |
| 실제 Codec | 두 문서 Load, Validate_Drawable, Save_Atomic, reload, canonical 일치. 잘못된 model carrier/누락 capture texture 거부, 실패 Save 시 기존 파일 보존 |
| 실제 camera-fit 함수 | 16:9,1:1,9:16 시작 앞면 NDC±1; 중간 축소; 끝 행렬 byte 일치; invalid bounds/projection 시 기존 행렬 보존 |
| WModel retime 비교 |600 vertex,52 bone, 모든 key pose 값 불변;0/.1/.5/1/1.95/3초6개 시점 bounds delta0 |
| git diff --check | 변경 source/Data/PLAN/RESULT 대상 통과 |

검증 산출물은 `out/DimensionMasterALTV20260912/`의 `compile.log`, `renderer_compile.log`,
`fxc.log`, `fxc_mesh128.log`, `fxc_particle128.log`, `codec_run.log`, `data_check.json`,
`retime.json`, `pose_check.json`에 있다. 기존 tiger focused probe의 CPU Codec 검사를
재사용했다. 처음에는 어제의 obj를 재사용해0xc0000005가 났고, 현재 public ABI로
의존 소스를 다시 컴파일하고 새 ParticleUpdatePool 의존성을 연결한 뒤 통과했다.
이 실패를 제품 효과의 화면 오류로 분류하지 않는다.

Client/UI 실행·조작·GPU draw·화면 캡처는 하지 않았다. 사용자 화면에서 실제 중앙 큐브
안착, 색/가림/카메라 추종, 시작 화면과 주변 큐브의 시각적 일치는 아직 사용자 확인 전이다.


## G04. 기존 실행 Client 보호와 데이터 설치 보류

기존 Client Codec은 ModelCue material을 Artist/Lance 계약에만 허용한다. 이번 ALT178 cue
material은 기존 `Validate`에서 문서 전체를 거부한다. Effect Catalog는 Data/Effects/Authored의
문서를 첫 사용/Loader stage에서 직접 읽고, 이미 로드한 문서만 캐시로 유지한다. 따라서 새
코드와 데이터의 배포 시점을 분리하면 기존 프로그램의 새 로드/Reload가 실패할 수 있다.

두 데이터는 작업 전 `full.before.json`, `tuning.before.json`에 기존 patch를 다시 적용한 결과와
대조했다. full은 삽입된 두 줄의 CRLF 보존까지 반영한 bytes가 일치하고 tuning은 원래 patch
bytes가 일치했다. 추가 사용자 값 편집은 없었다. 완전한 후보를 보존한 뒤 직전 bytes CAS로
두 Authored 파일을 원본과 byte-identical 복귀했다. 원래 사용자 전체309행/26행을 보존한다.

- 후보: `out/DimensionMasterALTV20260912/full.candidate.effect.json`, `tuning.candidate.effect.json`
- 기준·후보 SHA/현재 상태: 같은 경로의 `altv_deferred_data_manifest.json`
- 설치는 기존 Client 종료 뒤 새 Codec/Renderer/Shader와 retimed WModel 준비 시 두 문서를
  함께 수행한다. 두 source가 manifest의 baseline과 달라졌다면 새 사용자 편집을 먼저 합치며
  후보 파일로 덮어쓰지 않는다. 현재 상태는 `DEFERRED_NOT_INSTALLED`다.
