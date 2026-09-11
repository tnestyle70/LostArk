# World 표시 이펙트 연결과 ALT V Profiler 분석 결과

## G00. 범위와 현재 판정

사용자 첨부 두 이미지를 열람하고 원본 resource/module/material을 대조했다. 이번 소비자는
F1 Effect Tool V1이며 gameplay mouse command나 쿠크 패턴의 Server 선택 로직은 바꾸지 않았다.
성능 작업은 최신 캡처 분석과 후속 요청의 CPU/GPU 계측·Composition Profiler 패널 구현이다.
[계측 구현 결과](2026-09-11_PROFILER_CPU_GPU_STAGE_MEASUREMENT_RESULT.md)를 함께 따른다. JobSystem·GPU 최적화를 이번에 구현해
11FPS 문제를 해결했다는 의미가 아니다. 자세한 수치는
[Profiler 분석 보고서](2026-09-11_WARLORD_ALT_V_PROFILER_ANALYSIS_REPORT.md)를 따른다.

조사·구현 기준은 `codex/kouku-gate1-sequence-playback`, HEAD
`2d7b96693fb93c397a26632c221d5d29bf4a2ae1`이다. 시작 시 LAN sync, git status/fetch와
기존 PLAN/RESULT를 확인했다. 기존 대규모 미커밋 변경을 보존했으며 자동 stage/commit/push는
하지 않았다. Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 최종 화면 판정은 USER_PENDING이다.

## G01. 선택한 원본과 남은 식별 경계

| Tool Effect ID | 원본 ParticleSystem | 실제 원본 구성 | Tool 재생 |
|---|---|---|---|
| `effect.world.mouse_click` | `FX_BS_03.mark.par_b_picking_01` | mesh 1, EF local decal 2, glow sprite 1 | 1.2초 한 번 |
| `effect.world.move_destination` | `FX_BS_03.mark.par_i_movetrack_01` | arrow mesh 1, ring/glow/point sprite 11 | 7초 재생창 반복 |

클릭 mesh는 `FX_SM_00.fm_b_picking_001`이고 source geometry 66 vertices다. 원본 VS에는
WPO나 정점 수축 식이 없다. `fx_b_me_pickingarrow_01_ad` PS가 particle alpha를 UV.y에서 빼며
마스크를 이동시킨다. 원본 alpha는 0.4초 동안 -1에서 +1로 변하므로 이를 opacity라고 간주해
0~1로 clamp하거나 alpha<=0 입자를 cull하면 앞 절반이 사라진다. texture green 채널과
원본 TA_CLAMP/linear sampling을 사용한다. mesh 0~0.4초, 첫 바닥 decal 0~1초,
추가 decal 0.2~0.9초, glow 0.2~0.4초가 원본 입력에서 확인된다.

이동 arrow는 `FX_SM_00.fm_i_mark_01`, 21 vertices다. geometry의 tip은 Y=-0.01962m,
양 wing은 Y=+0.02185m로 아래를 향한다. i variant는 위 점과 아래 화살표이며 g variant는
gear가 추가되고 l variant는 화살표 방향과 점 위치가 다르다. 이 구조와 금색 입력을 근거로 i를
선택했다. MIDNIGHTC 계열 실제 map UPK 24개, EFGAME.u와 STARTUP 검사에서는 이 variant를
직접 선택하는 참조를 찾지 못했고 Commander UI package 2개는 기존 reader가 해제하지 못했다.
따라서 첨부와 대응하는 원본 후보 연결이며, 쿠크 1관문의 최종 live variant가 확정됐다는 뜻은 아니다.

arrow의 위아래 움직임은 skeletal animation이나 VS WPO가 아니라 source particle
VelocityOverLife다. emitter 주기는 3.5초, particle 수명은 약 3.52초이고 원본 TypeData 회전은
[-5,0,90]도다. 모델 clip을 임의로 붙이지 않고 모든 component를 같은 effect clock으로 재생한다.
원본 mesh 색 (3,1.2,0.5)에 color scale 0.8을 곱한 기본 RGB는 (2.4,0.96,0.4)다.
shader의 별도 selection color 기본값 0과 이 mesh color 입력을 구분한다. cooked graph node가
없으므로 이 prefix 의미는 PS·원본 기본값·기존 local VF ABI의 교차 분석에 근거한다.

## G02. 실제 연결한 파일과 반복 소유권

두 authored v13 JSON에 원본 4+12 elements를 넣고 `EffectCatalog.json` direct authored 행,
`EffectResourceTree.json`의 World category/두 참조, Client project/filter의 `96.DataFiles\\Effects`
None 항목을 연결했다. 새 C++ 파일이나 별도 runtime 경로는 추가하지 않았다.

`Effect_Tool`의 Character / Boss / World 선택과 saved-authored 목록을 확장했다.
`CEffectAuthoringSequencer::Select_WorldEffect`는 기존 scene-player의 위치·방향을 snapshot해
공통 scene target을 준비한다. 보스 model/Composition은 요구하지 않는다. All Effects의 Play All,
Open Editor, Resources Preview, Element Solo와 Restart는 기존 V1 occurrence owner를 사용한다.
Stop·선택 교체는 기존 occurrence 해제 경로를 따른다.

임시 target의 loopPolicy와 previewDurationMs는 저장된 sequence의 Loop·anchor·dirty 상태와
분리돼 있다. 클릭은 1200ms 한 번, 이동은 7000ms 반복을 기본으로 둔다. 이동 source의
9개 emitter는 생략된 Required emitterLoops=0인 반복이고 3개는 명시 loops=1인 초기 pulse다.
문서의 bounded simulation duration에는 particle tail이 더해져 클릭 약2.2초·이동12초가 된다.
그 끝까지 기다린 뒤 loop하면 arrow가 먼저 사라지므로 Tool만 7초에서 되감는다.
전체 Tool loop마다 초기 pulse도 다시 시작하는 동작이며 무기한 gameplay marker의 수명 계약을
새로 정의한 것은 아니다.

반복은 이미 준비된 occurrence를 `Sample`로 되감는다. 자원을 매번 재stage하지 않고,
일반 `Seek`의 paused=true 부작용으로 두 번째 loop가 멈추지 않도록 했다.

## G03. Resources와 native material 연결

설치 위치는 `Client/Bin/Resources/Effect/World/Meshes`의 WModel 2개와
`Effect/World/Textures/<source-package>`의 DDS 11개다. 원본 mesh는 기존 ModelAssetConverter와
geometry provenance cooker를 사용해 CModel 경로로 구웠고 preScale은 0.01이다.
텍스처는 원본 DDS를 재사용하거나 TGA의 RGBA pixel을 보존해 DDS로 변환했다.
Data에서 참조하는 13개 파일은 모두 Resources 상대 ID이며 실제로 존재한다.

13종 source material 중 4개 native profile 511/850/862/2343을 재사용하고 비어 있던
2351~2359에 9개 exact descriptor를 추가했다. source identity와 실제 texture/uniform 연결은
기존 `Effect_ArtistMaterial.h` 및 material compiler/carrier가 소비한다.
새 `Shader_EffectWorldNative.hlsli`는 기존 ArtistNative의 group2304 dispatcher에 연결하고
Client project/filter의 `97.ShaderFiles` None 항목에 등록했다. 기존 Kouku group 함수를
덮어쓰지 않았다. 새 shader file이 World 전용 evaluator를 제공해도 geometry·particle runtime은
기존 mesh/sprite/decal carrier를 그대로 사용한다.

2358의 ht02_2는 원본 distortion enabled/intensity15와 대응 PS를 따라 기존 MRT2 distortion output에
연결했다. 해당 pass의 원본 discard는 color pass까지 버리는 clip이 아니라 distortion contribution0으로
유지한다. 2358의 depth-sample requirement도 문서·descriptor에 반영했다. mesh module은 Required의
기본 재질과 slot0의 실제 native override를 구분해 기존 strict Codec/runtime이 그대로 소비한다.

이동 arrow의 TypeSpecific/MeshFaceCameraWithLockedAxis/RotateZ 조합은 기존 renderer에 좁게
연결했다. 설치 model의 preRotation을 제거한 basis에서 카메라 수평 방향을 향하게 한 뒤 원본 5도
기울기를 다시 적용한다. CPU scalar/native instanced 경로에 같은 helper를 사용하고 위치·scale과
카메라 극점의 finite fallback을 보존한다.

재현 명령과 source/native evidence는 `Tools/EffectPipeline/build_world_marker_effects.py`,
`Tools/EffectPipeline/install_world_marker_native_materials.py`,
[out/WorldMarkers20260911](C:/Users/user/Desktop/LostArk/out/WorldMarkers20260911)에 있다.
Resources binary는 Git 관리 대상에 추가하지 않았다. 다른 PC에서 보려면 팀장이 이 World 폴더를
기존 Drive resource 배포에 포함해야 한다. 별도 resource pack/lock을 완료 조건으로 만들지 않는다.

## G04. 검증 기록

데이터 조사 단계에서 두 문서 parse, 원본 16 emitter/material mapping, 9개 반복+3개 초기 pulse,
13개 Resources 참조, mesh geometry 및 texture dimension, Python 문법과 변경부 공백 검사를
통과했다. Tool의 루프 경로에서 pause 부작용을 코드 검토로 발견해 수정했다.

최종 통합 Debug Product compile/link/deploy는 2026-09-11 19:20:48 KST에 Engine/Shared/Server/Client
모두 PASS했다. receipt는 `out/BuildPipeline/runs/20260911T102048234Z-debug-product.json`이다.
기존 경고는 남아 있으나 최종 오류는 없다. 실제 JSON/16 emitter/13 resources closure와 Client
project/filter XML 검사를 최종 데이터로 다시 통과했다. native mesh/particle/decal 최종 fxc3종도 PASS다.

원본 DXBC와 lowered World shader의 63개 synthetic RT0 비교는 mismatch0, worst0이다. distortion
7개 수치 비교도 mismatch0, worst1.2666e-5이며 source asm 상수의 인쇄 정밀도 차이를 포함한다.
이 검사는 단순 1×1 texture 입력을 포함하므로 실제 UV addressing이나 최종 화면을 증명하지 않는다.
최종 Product 객체로 링크한 실제 Codec/Playback 검사도 exit0 PASS다. 클릭139개·이동727개
60Hz sample에서 원본4+12 emitter가 모두 출력됐으며 저장/roundtrip/실패 rollback/종료 정리,
되감기와 root translation을 확인했다. 클릭0.1초의 signed alpha는 -0.583333으로 유지되고 이동
arrow는3.5초 source 반복과 Tool7초 직전에도 살아 있으며 rewind payload가 일치했다.

별도 window/swapchain/draw 없는 WARP device에서 실제 CEffectObject Create→Clone→Stage_Document를
실행해 두 문서 모두 resource prepare 성공을 확인했다. 제품 RendererCore가 요구하는65개 최신
CSO를 out 검증 EXE 옆에 복사해 검사했고 source/Data는 바꾸지 않았다. 증거는
`out/WorldMarkers20260911/verification/cpu_probe_run.log`, `renderer_stage_probe_run.log`,
`samples.tsv`, 두 roundtrip JSON이다. 이는 리소스 입장/수치 검사이며 화면 품질 PASS가 아니다.

## G05. 사용자가 확인할 경로

LAN sync 결과 이 PC는 `server-host`이며 endpoint는 `192.168.0.14:7777`, 방화벽 설정은 준비됐다.
Visual Studio의 `Server + Client` profile을 Ctrl+F5로 시작하고 기존 Lobby에서 아레나에 입장한다.

1. F1 → Effect Tool V1 → All Effects → Character / Boss / World에서 **World**를 고른다.
2. `effect.world.mouse_click`의 **Play All**을 누른다. 현재 player 위치에서 청록색 클릭 표시의
   mesh mask·두 바닥 decal·glow가 함께 재생되고 한 번 끝나는지 확인한다.
3. `effect.world.move_destination`의 **Play All**을 누른다. 금빛 원과 아래 화살표/위 점이
   움직이며 반복되는지 확인하고 **Stop**으로 종료한다.
4. **Open Editor**의 Elements에서 각 component를 선택해 Solo를 확인한다. Resources의
   Saved Effects → V1 → World에서도 두 문서를 열거나 Preview할 수 있다.

표시는 Play 시점 player root를 빌린 Tool preview다. 실제 마우스 피킹 위치에 자동 생성하거나
게임플레이 이동 command에 붙인 결과는 아니다. 위치·크기·색·카메라에 대한 방향과 반복 이음새의
최종 fidelity는 사용자의 서면 관찰로 판단한다.
