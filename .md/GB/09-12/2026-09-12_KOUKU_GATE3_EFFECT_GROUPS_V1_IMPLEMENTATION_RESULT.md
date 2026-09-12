# 쿠크 전체 이펙트·Effect Tool V1 진행 상태

이 문서는 2026-09-12 사용자의 구현 현황 정리 요청 시점의 중간 결과다. 전체 복원 완료나
제품 빌드 성공을 의미하지 않는다. Client/UI를 실행하거나 화면을 캡처하지 않았다.

## 소스·데이터에 반영한 범위

- V1에서만 Box Detail, Composition Resources 외부 창을 숨기고 Benchmark 선택 그룹 편집을 연결했다.
  Append 인스턴스의 위치·회전·크기·앵커·재생 시간을 분리했다. 다른 도구의 창은 유지한다.
- 내려찍기·인형·마리오 6개와 쇼타임 20개 authored 문서를 catalog, resource tree,
  전 관문 공용 KoukuSaydonComposition, Client project/filter에 등록했다.
  쇼타임은 3관문/패턴/세이튼/쇼타임 아래에 분류했다. 기존 패턴32에 추가한 occurrence는
  startMs=0인 독립 그룹 튜닝 초안이며 원본 쇼타임 전체 전투 timeline 완성본이 아니다.
- DimensionMaster V skill 2050520의 기존 42개 요소를 유지하고 빠졌던 22개를 더해 64개로
  연결했다. 실제 소비자는 기존 DimensionMaster.animevents와 authored effect 문서다. F는 수정하지 않았다.
- 원본 534 ParticleSystem의 native material 798개(신규 660, 기존 재사용 138), 왜곡 pass 320개를
  추출했다. 기존 설치 그룹과 DMV를 보존하여 Kouku native shader group에는 853개 프로그램을 설치했다.
  DDS 686개와 geometry WModel 120개의 준비 근거는 out/KoukuAllEffects20260912/native 및
  geometry_installation.json에 있다. Resources는 Git 관리 대상이 아니다.
- 전체 개별 원본 문서는 out/KoukuAllEffectsFinal20260912/installation.json 기준 525개를
  Data/Effects/Authored에 설치했다. 이 525개는 아직 전체 tree/catalog 등록을 완료하지 않았다.
- 기존 source particle consumer에 LocationEmitterDirect, 실제 provider 연결, KillHeight,
  PointAttractor, Orbit LINK, 기존 PhysX static scene sweep collision을 연결했다.
  빔·리본과 연출 sourceTransformTrack도 기존 trail/particle/transform 경로에 구현했다.

## 후보까지 준비했고 아직 최종 연결하지 않은 범위

- 화염링은 Projectile 421980401의 지속 8요소와 폭발 12요소를 각각 독립 문서로 구성했다.
  out/KoukuFireRing20260912에 후보와 원본 transform 근거가 있다. 최종 Data/catalog/tree 등록은 남았다.
  첨부 이미지와 해당 원본의 정확한 동일성은 사용자의 화면 비교가 필요하다.
- 패턴·stage별 후보 2,895개 중 원본 clip/bone 조건을 만족한 후보는 2,875개다.
  아직 최종 native library를 소비해 설치하지 않았다. 20개는 실제 원본/설치 모델에 없는
  B_WP_3/4/5 부착점 때문에 완료로 분류하지 않았다.
- 원본 연출 19 Matinee의 173 particle occurrence 중 160개를 이동·부모 곡선과 연결한 후보는
  out/KoukuSequenceMotion20260912에 있다(1,636요소, 완전 후보 15문서).
  나머지 13개는 원본 A/B/C 혼합 애니메이션과 SkelControl을 포함한 실제 모델 owner 연결이 필요하다.

## 실행한 검증

- 실제 RPCT05 보스 및 CDMD00 인형 CModel과 원본 clip으로 본 부착 8건 수치 검사 통과:
  out/KoukuActualModel20260912/result.json.
- 실제 DimensionMaster 모델/timewave 애니메이션으로 V 64요소 검사 통과:
  out/KoukuActualModel20260912/dimensionmaster_v.json. 화면 anchor fixture는 실제 카메라 검증이 아니다.
- Engine Debug 빌드와 PrepareEngineSdk 수행. 실제 PhysX scene의 static box sweep hit/miss 및
  actor 제거 후 no-hit 수치 검사 통과: out/KoukuGate3Effects20260912/physics_sweep_probe.json.
- V1 관련 CPP 및 beam/ribbon 변경의 최소 컴파일을 수행했다. 이 결과를 전체 제품 빌드로 대신하지 않는다.
- 기존 178 native 프로그램 시점의 4개 carrier shader 컴파일은 통과했지만, 전체 853 프로그램을
  설치한 최신 shader 컴파일은 실패했다. 최신 실패는 native group의 원본 CB2[4] 읽기에 대응하는
  passValues 배열 범위 문제다. out/KoukuGate3Effects20260912/Shader_VtxEffectMeshKouku2304.fxc.log.
  앞서 Ice distortion 원본 VS의 TEXCOORD5 world position/projection 연결 누락은 수정했다.

## 남은 작업과 실행 상태

전체 shader compile 및 Product build가 통과하지 않았고 실행파일에 최종 반영되지 않았다.
확인 당시 Client/Bin/Debug/Client.exe 수정 시각은 2026-09-12 01:55:36이며 Client/Server 프로세스는
실행 중이지 않았다. 사용자 최종 시각 검증은 수행 전이다.

남은 원본 문서 실패는 hidden provider의 native material 요구, owner가 필요한 AnimationTrail,
리본 투사체 projection 등을 포함한다. 원본 lifetime=0, 음수 emitter delay, MeshMaterial slot,
EF KillLength, 일부 원본 vertex deformation도 소비 의미 확인·연결이 남았다. 정상처럼 보이도록
값을 임의 clamp하거나 실패 항목을 완료로 바꾸지 않았다.

최종적으로 native library 재투영, 패턴/연출/화염링 설치, 기존 항목의 기획자 이름과 트리 정리,
전체 변경의 JSON/XML parse 및 diff 검사와 제품 빌드를 마쳐야 한다. 사용자 화면 확인 경로는
빌드 완료 뒤 Server + Client profile로 시작하여 F1 → Effect Tool V1 → All Effects → KoukuSaydon이며,
쇼타임 그룹은 3관문 → 패턴 → 세이튼 → 쇼타임에서 Benchmark에 Append한다.

다른 작업의 변경이 섞인 worktree이므로 자동 stage/commit/push는 하지 않았다.

## 사용자 빌드 X3004 수정 경과

사용자가 첨부한 `g_EffectSceneDepthTexture` X3004는 실제 FXC 오류다.
`Shader_VtxAnimMeshBinary.hlsl`로 재현하면 native group 121575행에서 같은 미선언 오류가 난다
(`out/KoukuGate3Effects20260912/model_shader_before.fxc.log`).

installer가 새 왜곡 companion을 생성할 때 `ARTIST_NATIVE_MODEL_ONLY` 제외 조건을 누락했다.
기본 색상 함수와 동일 조건을 companion 320개 모두에 유지하도록 수정하고 생성물을 갱신했다.
수정 후 동일 모델 shader의 FXC fx_5_0 /O1 컴파일은 성공했다
(`model_shader_fixed.fxc.log`, `Shader_VtxAnimMeshBinary.fixed.cso`).
실행 중인 Client/Server는 조작하지 않았으며 독립 out에 결과를 생성했다.

동시에 원본 CB2[4]/CB2[6]를 4행 passValues에 쓰던 generator 범위 오류를 수정했다.
원본 선언대로 5/7행을 생성하고 확인된 override/viewport 값을 공급한다. 설치된 모든
함수의 정적 passValues index 범위 검사와 320개 MODEL_ONLY guard 검사, Python 구문,
project XML parse 및 변경 범위 diff 검사는 통과했다. 다른 carrier의 FXC 결과는 후속
검증란에서 별도로 기록한다.

## 후속 사용자 빌드: projection 및 열 번째 source texture — 2026-09-12

추가 사용자 빌드의 `Client/Default/x64/Debug/Client.log`에서 `ArtistNative2697Distortion`의 미선언 `projection` X3004를 확인했다. 현재 generated group의 동일 결함 12함수와 generator를 교정했다. 원본 PS `2d8c822c88934149bfa9587fd772c1e8` 및 VS 3종은 TEXCOORD5에 world position을 보내고 PS에서 CB1로 투영한다. 함수에는 source projection을 선언하고 world position을 전달하여 clip position의 이중 투영을 없앴다. 새로 생성한 12함수 본문은 설치 함수와 모두 일치했다.

mesh/particle의 기존 source world cm 및 projection 바인딩은 연결돼 있었다. ribbon native2697의 Trail에는 world position 보간과 native input 전달을 추가했고, renderer의 fixed Trail 바인딩에서 같은 source-to-client basis·cm projection을 공급한다. 실패 시 기존 render operation 오류를 보존한다.

뒤이어 `ArtistNative3338`이 실제 열 번째 texture를 sample하지만 helper·SRV·배열이 9개뿐인 오류를 확인했다. 실제 `native_texture_9`는 `fx_i_environment_001.dds`이며 원본 참조를 생략하지 않고 공통/모델 shader 선언, helper와 generator, renderer staging 배열/바인딩, screen-post snapshot/마스크를 10개로 연결했다. 기존 `size()` 기반 상한·필수 texture 검사와 source identity는 유지했다. 같은 수정 TU인 `Effect_NativeScreenPostMaterial.cpp`의 Windows/Winsock 포함 순서 오류는 `Client_Defines.h`를 먼저 포함하여 해소했다.

`Effect_DocumentRenderer.cpp`와 `Effect_NativeScreenPostMaterial.cpp`는 제품 Debug x64의 기존 CL 옵션을 사용하고 OBJ/PDB 경로만 out으로 바꿔 모두 컴파일했다. 오류0, 기존 warning은 각각4/1개이며 renderer 검증에는 위 Trail projection도 포함된다. 원래 실패 로그와 수정 후 로그는 `out/KoukuShaderCompileFix20260912/cpp-texture10/compile/`에 보존했다. 파일별 BOM·개행은 유지했다.

셰이더의 실제 제품 FXC `fx_5_0 /O1` 검증은 후속 결과를 아래에 기록한다. 앞선 `/O3` particle 시도는 불필요한 최적화 비용을 피하기 위해 에이전트 소유 프로세스만 종료했으며 성공으로 계산하지 않는다. 실행 중인 Client·Server와 사용자 Visual Studio 빌드는 조작하지 않았다. 제품 링크와 Client/Bin CSO 교체는 수행하지 않았으므로 전체 제품 빌드나 사용자 visual PASS로 간주하지 않는다.

최종 FXC 결과: particle2304, mesh2304, VtxAnimMeshBinary, Trail, LocalDecal, native screen-post 6개 모두 `fx_5_0 /O1` exit0 및 CSO 생성을 확인했다. 통합 근거는 `out/KoukuShaderCompileFix20260912/fxc.results.json`이다. 마지막 particle은1543.812초가 소요됐으며 제안했던 추가 `/Od` 진단은 실행하지 않았다. 사용자가 직접 결과를 확인하고 문제가 있으면 재요청하겠다고 하여 추가 검증은 중단했다. 전용 FXC/runner는 남아 있지 않으며 수정 소스와 out 검증 산출물은 보존했다. 제품 링크·Client/Bin CSO 반영·사용자 화면 판정은 수행하지 않았다.
