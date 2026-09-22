# 차원술사 ALT V 액자와 캡처 큐브 연결 결과

## G0. 실제 문제와 보존한 경계

화면 캡처 자체의 누락이 아니라, 현재 캡처가 큐브 내부 이미지로 이어지는 연출과 액자 표시가 요청 범위다. `Stage_PreparedInternal`은 시작 장면의 `Target_SceneHDR`와 `Target_SceneBloom`을 같은 시점에 복사한다. `Build_NativeScreenPost`와 `Bind_ModelCueNativeMaterial`은 같은 frozen pair를 사용하고, native178의 texture lane 2도 이 캡처를 읽는다. 이 기존 생성·소유권 경로는 변경하지 않았다.

실제 `sk_swp_cub_00_sk.wmodel`은 정점 600개이고 UV 범위는 약 0~1이다. 기존 임시 world-position adapter `uv.x*360, (1-uv.y)*360`은 원본 material의 `capture_centeruvtile=1.15`, U/V offset .5와 중첩됐다. 정규화 면 중앙 (.5,.5)이 약 (1.075,.925)로 가고 일부 영역이 texture clamp에 걸리는 것을 수식과 실제 shader로 확인했다. 같은 texture를 바인딩한다는 사실만으로 정상 구도가 보장되지 않았다.

## G1. 반영 내용

- native178의 프로젝트 UV adapter만 중앙과 기본 배율에 맞게 재기준화했다. 원본 116 RT0 연산, RGB split, native aura/edge Add, translucent carrier와 큐브 opacity/TRS는 보존했다. 이것은 **PROJECT_TUNED 좌표계 adapter 수정**이며, 미해독 원본 capture camera CB의 복원으로 기록하지 않는다.
- `g_ALTVCaptureUVTransform`은 실제 frozen texture의 가로/세로 비율과 기존 `captureSquare`로 center crop을 계산한다. native Color와 Bloom을 같은 함수에서 같은 UV로 읽는다. 2D capture의 끝 crop과 같은 값이며, square=false는 identity이다. Mesh178와 animated cue178 양쪽에 동일 crop을 바인딩한다.
- 액자 원본은 camera group의 17/12 `boxlinelight`, 58 `boxedge`, 61/62 후반 `boxlinelight` mesh다. 이 정확한 다섯 stable ID에만 기존 ScreenPost의 네 방향 edge speed, shrink time, destination offset, square, rotation을 적용한다. 원본 `fm_h_box_01_1` geometry와 native material을 기존 mesh carrier로 그리며, 화면 평면 배치만 해당 저작값에 맞춘다. 이것도 **PROJECT_TUNED 연출 제어**다. 다른 sprite·crack·카메라 행의 원본 transform은 변경하지 않았다.
- tuning 문서는 기존 27개 행을 그대로 보존하고, 현재 full 문서의 원본 액자 12/58 두 행만 추가했다. 결과는 29행이다. full의 모든 데이터 행, 사용자의 기존 tint/transform/visibility/45도 설정은 변경하지 않았다. 두 원본 capture mesh 18/31은 계속 숨긴다. 기존 2D capture 위에 별도 capture mesh를 이중 표시하지 않는다.
- effect JSON은 최신 디스크 재독, stable ID 충돌 검사, 기존 27행 동등 비교, 백업, hash 재확인, 원자 교체로 설치했다. `out/AltVCubeRepair20260920/install.receipt.json`.

## G2. 검증

- 현재 소스의 Renderer Geometry/MaterialHelpers/Particles/Rendering 네 TU를 별도 out 경로에 Debug compile: PASS. 기존 C4805 경고 1개는 남아 있다.
- 실제 ALTV128 mesh carrier 및 animated model shader를 `fxc /T fx_5_0`로 컴파일: PASS. 기존 potentially-uninitialized 경고는 보존된다.
- 실제 native178 함수와 Color/Bloom sampling helper를 그대로 추출하여 D3D11 WARP에서 실행했다. synthetic gradient texture의 16:9, 9:16, 1:1에서 Color/Bloom 각각 총 6 draw, 21,678 checks PASS. 최대 UV/색 오차 0.00024417. cube image/alpha가 존재하고 두 채널에 동일 crop이 적용되는 것을 readback으로 검사했다. 45도 화면 사각형의 회전 역변환 후 정규화 면 UV와 끝 crop이 일치함도 검사했다. `gpu.log`.
- 설치된 원본 `fm_h_box_01_1.wmodel`을 실제 CModel로 로드했다. preScale .01 적용 bounds는 [-.25,.25]m이다. production frame warp 본문을 추출해 화면 비율 3종, 각도 0/45/-90도, 대칭·비대칭 속도, 시간 4개를 조합한 72조건에서 실제 ScreenPost 역변환의 네 모서리와 대조했다. 유한·비특이 matrix 및 좌표 일치 1,514 checks PASS, 최대 UV 오차 4.77e-7. `frame.log`.
- 설치 후보 실제 C++ Load/Validate_Drawable/안정 roundtrip/두 번 rewind playback: 164,942 checks PASS. particle 26 emitters가 발생했다(ScreenPost 1개 및 숨김 capture 2개는 particle 수에 포함되지 않는다). `candidate-codec.log`.
- JSON parse와 변경 코드/셰이더/JSON `git diff --check`: PASS. C++ 파일의 기존 인코딩과 줄바꿈은 유지했다. 새 런타임 파일이나 vcxproj 항목은 없다.

## G3. 남은 화면 판정

headless 검사는 실제 native 함수, 설치 mesh, production frame 배치 수식의 제한된 검증이다. 게임 전체 composition 순서와 실제 카메라에서의 액자 두께·밝기, 2D 평면에서 변형되는 3D cube silhouette로 넘어가는 최종 화면은 사용자 확인이 남는다. 정규화 UV의 연속성 검사를 모든 3D 면의 화면상 픽셀 대응이나 원본 capture camera 복원으로 확대하지 않는다. Client/UI를 실행하지 않았다. Product 링크·배포는 root가 취합한다.

## G4. 사용자 후속 회귀: 캡처가 발밑으로 내려가는 현상

### G4.1 원인과 실제 좌표 근거

앞선 구현에서 활성화한 `captureUseModelCenter=true`가 직접 원인이다. `Try_ProjectCaptureTargetBounds`는 cube cue의 Local TRS와 character RootWorld로 첫 pose의 geometry bounds를 투영한다. 현재 cue Local Position은 0이고 실제 cube 첫 pose 중심도 원점 부근이라 character의 발밑을 끝점으로 선택했다. ScreenPost의 일반 Transform으로 계산한 위치를 이 bounds 계산이 다시 덮어써 위치·크기 튜닝도 캡처 끝점에 전달되지 않았다. 예전 액자 검사는 고정 UV 중심 (.57,.43)을 공급했으므로 이 실제 모델·캐릭터 기준점 문제를 검출하지 못했다.

설치 `sk_swp_cub_00_sk.wmodel`의 hash는 `21d0e0fb9b983b6ff3e9e10badee5a39bc14f752cb41e20b8a0a909b0c8869ad`로 기존 실제 geometry 분석과 같다. CModel의 정점 600개/본 52개, reference bounds는 [-.0401189,-.04643,-.0423673]~[.0456156,.0444816,.0422998]이다. 과거 첫 skin pose 측정과 reference bounds 차이도 약 .00005m 이내다. 원본 notify 036 `PlaySkeletalMesh` recipe는 `transformDecoded=false` 상태이며, 원본 capture shader의 camera CB0[0..5]도 완전히 해독하지 못했다. 임의 높이를 원본 복원값으로 추가하지 않았다.

### G4.2 코드 반영과 튜닝 경로

정확한 `altv.source.notify036.cube`를 목표로 하는 가시 ScreenPost에만 공통 camera-space rig를 적용했다. `Use Model Center`를 끄면 기존 깊이에서 화면 중심을 사용하고, 켜면 기존 월드 모델 기준을 사용한다. capture 끝 시각의 기존 Transform position/rotation/scale, linear lerp·velocity/revolution을 평가하여 같은 끝점을 유지한다. 2D 캡처 끝점, 다섯 원본 액자, 실제 animated cube draw 및 그 본에 부착된 FX가 이 값을 공유한다. UV offset도 여기서 한 번만 적용한다. 다른 Model Cue나 다른 ScreenPost profile에는 전파하지 않는다.

`Effect Detail`의 `Presentation Screen Post`에 다음 세 label을 추가했다. 기존 `Transform`의 동일 필드를 편집하며 별도 schema나 저장본을 만들지 않는다.

| label | 실제 소비 |
|---|---|
| `Capture / Cube Position` | 카메라 X 오른쪽, Y 위, Z 깊이. 캡처 끝점·액자·큐브·큐브 부착 FX에 전달한다. |
| `Capture / Cube Rotation (Degrees)` | Z는 화면·액자·큐브를 함께 회전한다. X/Y는 3D cube를 기울이고 2D 끝점의 투영 외곽 크기를 바꾼다. 2D 캡처를 완전한 3D 원근 평면으로 바꾸는 기능은 아니다. |
| `Capture / Cube Scale` | 실제 cube와 투영 끝점 크기를 함께 바꾼다. `Square Capture`를 켜면 2D 이미지는 계속 정사각형이며, 가로·세로 독립 튜닝은 이 옵션을 끈다. |

기존 `Use Model Center`, `Destination Offset (UV)`, `Capture Rotation (deg)`, `Shrink Duration (s)`, `Left Edge Speed`, `Right Edge Speed`, `Top Edge Speed`, `Bottom Edge Speed`를 유지했다. `45 degree capture into cube`는 이제 model center를 끄며, model foot root를 다시 선택하지 않는다. `Model Cue`의 `Local Position`/`Local Rotation (Degrees)`/`Local Scale`은 추가 모델 조절로 계속 적용된다. 편집 후 기존 `Apply`와 `Save`가 필요하다. Color/Bloom frozen pair, native178 UV/crop adapter, native 원본 연산과 translucent blending은 변경하지 않았다.

### G4.3 후보와 검증 상태

- full/tuning 최신 원본을 out에 보존하고 stable ScreenPost ID의 `detail.screenPost.captureUseModelCenter` true→false 한 필드만 바꾼 후보를 준비했다. 원본과 후보의 해당 필드를 복원한 전체 JSON 동등 검사가 통과했다. 실제 `Data`나 실행 중 draft는 이 하위 작업에서 변경하지 않았다. 설치는 root가 최신 hash·사용자 저장 기준을 확인하여 취합한다. manifest: `out/RaidRegression20260920/dimension/candidate.receipt.json`.
- Renderer MaterialHelpers/Particles/Rendering/Geometry와 Effect Tool MaterialDetail 다섯 TU의 격리 Debug `/Zs` 검사 PASS. 기존 C4805 경고 1개 유지. 새 source 파일·프로젝트 등록·저장 schema·wire 변경 없음. Product build·설치는 root 담당이다.
- 후보 두 문서를 실제 C++ `Load`/`Validate_Drawable`/`CEffectPlayback::Stage_Document`로 검사하여 2개 모두 PASS. `candidate-admission.log`.
- 실제 설치 CModel과 최신 production rig/projection/ModelCue sample/anchor 본문을 추출한 headless 하네스가 화면비 3종×root yaw 2종×root scale 3종, 총 18조건에서 2,282 checks PASS. world-center 기준의 발밑 UV Y=.608919가 screen-center 기준 .500299로 교정됐다. 실제 model origin은 (.5,.5)다. 양수 Y 위치가 위로 이동하고, scale이 끝점 크기를 바꾸며, UV offset이 cube에도 한 번만 전달되는 것을 검사했다. bone attachment는 설치된 실제 본으로 792 samples를 대조하여 draw world와 최대 오차 0이었다. 추가 Model Cue Local Position, 무관한 cue의 원래 transform 보존, explicit motion hold, 잘못된 camera 거절도 검사했다. `rig_probe.log`.
- Client/UI는 에이전트가 실행하지 않았다. 이번 사용자 화면 회귀는 수치·소비 경로까지 수정한 상태이며, 복구 후 실제 게임 화면 판정은 아직 미수신이다. 기존 G2의 GPU crop 검사를 이번 실제 화면 확인으로 대체하여 기록하지 않는다.

## G05. 큐브25개 전체에 퍼진 화면 캡처의 중앙 큐브 한정

### G05.1 원인

09-22 사용자가 화면 축소 뒤 주변 큐브 여러 개에도 동일 캡처가 매핑되는 현상을 보고했다.
설치 모델을 다시 읽은 결과600정점/900인덱스/52본이 한 submesh를 이루고, 그 안에는25개의
rigid cube가 있다. 각 cube24정점이 각각 한 본에 weight1로 연결된다. 중앙 `b_cube_1_02`는
현재 bone3이며 정점 중심은 원점, 각 축 bounds는±.0371621m다. 주변은 bone5,7…51이다.

기존 `altv.source.notify036.cube` 선택은 이 모델 전체를 선택했다. renderer가 native178의
texture lane2에 frozen 장면을 바인딩하고 전체 submesh를 그리므로25개 cube 모두 같은
캡처를 받았다. 앞선 UV crop·2D handoff 검사는 이 skin island별 표면 선택을 검사하지 않았다.

### G05.2 수정

`Effect_DocumentRenderer_Rendering.cpp`는 정확한 모델과 cue에서 실제 CModel의
`b_cube_1_02` index를 찾아 `g_ALTVCaptureBoneIndex`로 바인딩한다. 본이 없거나 해당 모델이
아니면 그 draw를 거절한다. 다른 native ModelCue에는 sentinel index를 매번 공급한다.

`Shader_VtxAnimMeshBinary.hlsl`의 skinned VS가 실제 vertex blend indices/weights에서
중앙 본의 가중치를 계산하고 pixel shader로 전달한다. native178의 새 두 인자 호출은
capture RGB항 `source[2]`에만 이 가중치를 곱한다. frozen Color와 Bloom 평가가 같은 값을
소비하고, 주변24개 cube의 animation·geometry·aura·edge·alpha는 기존 계산을 유지한다.
한 인자 native178 호출은 weight1로 이어져 static capture mesh의 기존 동작을 보존한다.

수정은 Client 정본 C++1개/HLSL2개다. source 원본 재질의 새 해석이 아니라 요청에 따른
표면 선택 adapter다. authored JSON, 설치 WModel, capture crop/축소 시간/TRS는 변경하지
않았다. 다른 세션의 Renderer shadow 및 source-character forward shader 변경은 보존했다.

### G05.3 검증 상태

실제 설치 모델25개 skin island, 중앙24정점과 주변576정점의 bone/weight를 확인했다.
중앙 정점의 active weight가 네 blend lane 중 세 번째에 있는 경우도 포함하여 모든 lane을
가중 합산한다. `out/AltVCentralCube20260922/topology.json`에 설치 모델 hash와 본별 정점
분포를 기록했다.

현재 native178 함수·Color/Bloom sampling helper·VS weight 계산을 추출하여 D3D11 WARP에서
실행했다. 실제25개 skin group × 화면비3종 × Color/Bloom2종에서 중앙6 draw는 기존 캡처
출력과 일치하고, 주변144 draw는 캡처항만 빠지며 native aura·edge·alpha를 유지했다.
static 한 인자 wrapper의 기존 출력과 잘못된 본 index의 캡처 차단도 포함하여 총171 draw,
584,444 checks PASS, 최대 오차1.01328e-6이다. 이 검사는 설치 정점과 실제 shader 연산을
사용한 통제된 headless 검증이며 게임 전체 화면 판정은 아니다. 로그는
`out/AltVCentralCube20260922/gpu.log`, 하네스는 같은 폴더의 `gpu.cpp`와
`native178_actual.hlsl`이다.

수정 Renderer TU의 격리 Debug `/Zs` 검사와 C++/HLSL `git diff --check`가 통과했다.
실제 animated shader와 static ALTV128 shader의 전체 `fxc /Od /T fx_5_0` 컴파일도 각각
exit0으로 통과했다. X4000 경고가 남는다. 로그는 같은 폴더의 `compile.log`,
`animated-shader.log`, `static-shader.log`이고 종합 근거는 `verification.receipt.json`이다.
격리 최적화 compile은 정상 Product 컴파일과 중복되어 중단한 뒤 `/Od` 검증으로 교체했다.
정상 최적화 Product Debug 빌드 결과는 통합 빌드 결과에 별도로 기록한다. Client/UI 실행·
화면 캡처는 하지 않았으며 최종 게임 화면 판정은 사용자 확인 단계다.

정상 Product의 `/O1`로 새로 생성된 animated base와 SourceGroup10개를 모두 확인했다.
첫 컴파일 중 source가 바뀌어 이전 입력으로 만들어진 CSO를 mtime만으로 최신이라
판정하지 않았다. 마지막 SourceGroup176도18:54:16 새 산출물이 생성됐다.
18:46:39에 링크된 실제 `Engine/Bin/Debug/Engine.dll`과 이11개 CSO를 out sandbox에
고정하여 FX11 변수 reflection11/11 valid 및 Set/Get S_OK, 실제
`CShader::Create → Stage_ProgramVariants`, native178 Bind/Begin, sentinel/중앙 mask별
20개 variant copy/Begin을 통과했다. source3개·Engine shader source·DLL·11개 CSO의
검증 전후 hash도 일치했다. 이것은 windows0/draws0의 실제 엔진 로더 검증이다.
`out/AltVCentralCube20260922/independent-review/admission/admission.receipt.json`과
`admission.log`, `reflection-fresh.log`에 근거를 남겼다.

19:04:54 최종 정상 Product Debug 빌드에서 Engine/Shared/Server/Client 모두 PASS다.
설치된 `Client/Bin/Debug/Engine.dll`과 animated CSO11개가 위 실제 로더 검증 입력과
모두 byte hash가 일치하며 Client.exe도 새로 링크됐다. 근거는
`out/BuildPipeline/runs/20260922T100454659Z-debug-product.json`과
`out/Gate2FlowAudio20260922/product-deployed-admission-hash.json`이다. 소스·개별 컴파일·
GPU 연산·실제 엔진 로더·제품 빌드 및 배포 확인까지 완료했다. 실제 게임 화면은 사용자가
확인하며, authored JSON과 WModel은 이번 중앙 큐브 수정에서 변경하지 않았다.

19:41 상태 재확인에서 세 수정 소스와 설치 Engine.dll/Client.exe는 위 검증본과 같았고,
animated CSO11개는19:26~19:29 후속 재생성으로 hash가 바뀌어 있었다. 현재 설치 파일을
다시 고정하여 FX11 reflection11개와 실제 CShader admission·20개 variant copy/Begin을
재실행해 모두 통과했다. source·DLL·CSO의 검사 중 동시 변경은 없었다. 19:04 검증의
receipt/log3개는 `admission/verified-1904/`에 보존했고, 최신 설치본 근거는 기존
`admission/admission.receipt.json`·`admission.log`·`reflection-fresh.log`다.
