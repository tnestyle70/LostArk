# 쿠크 전체 이펙트·Effect Tool V1 진행 상태

이 문서는 2026-09-12 초기 중간 결과와 후속 수정의 기록이다. 최신 독립 패턴 그룹의 반영과
검증은 G08을 따른다. 초기 광역 복원 전체의 완료를 의미하지 않으며 Client/UI를 실행하거나
화면을 캡처하지 않았다.

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

## G08. 독립 패턴 그룹과 플레이어별 알비온 — 후속 반영

사용자의 조사 전용 요청 뒤 `전부 반영`, `격자 변형도` 요청을 받아 이 구간을 구현했다.
아래는 앞선 광역 복원 기록과 구분한 이번 요청의 범위다. 기존 Pattern의 사용자 시간·clip·
presentation occurrence는 자동 배치하지 않았고, 각 그룹과 Logic을 직접 Append하도록 등록했다.

### 등록한 그룹

`Data/Effects/Authored`의 신규 37문서, 1,040요소를 기존 catalog/tree와 쿠크 공용
Composition의 presentationResources, Client project/filter의 None 항목에 연결했다.

| 분류 | 그룹 수 | 구성과 원본 경계 |
|---|---:|---|
| 무지개 댄스 | 3 | Aura 7요소, Light sprite 3요소, 둘의 합본. BOSS root 저작 기준 |
| 칼날 댄스 | 6 | 원형·안쪽 도넛·바깥 도넛을 예고/폭발로 분리. 반경 0–3/3–4.5/4.5–6m |
| 쇼타임 원형·도넛 | 6 | 별도 원형/안쪽/바깥 예고·폭발. 반경 0–4/4–8/8–12m |
| 알비온 | 6 | 파란 원 예고·뇌격·런타임 합본, 한쪽 부채꼴 예고·4방향 예고·폭발 |
| 화염 십자 | 8 | 한 줄 예고·폭발, 십자 예고·폭발·조명·폭발+조명, 지팡이 회전·decal |
| 화염격자 | 8 | 한 줄 및 A/B/C 각각의 예고·폭발 |

1관문의 무지개댄스/칼날댄스, 3관문의 쇼타임/알비온/화염 십자/화염격자 아래에 이름으로
묶었다. 기존 쇼타임 합성 circle.warning.impact와 donut.warning.impact는 ID와 payload를
보존하고 `이전 합성`으로 분류했다. 독립 그룹은 각자 0초 기준이며 내부 source delay와
particle tail은 유지한다. 도넛 뒤 원하는 시점에 같은 이름의 폭발을 독립적으로 붙일 수 있다.

### 원본에서 확인한 내용과 반영 방식

무지개 Aura에는 cylinder `fm_b_cylinder_005`와 ring `fm_d_ring_008`이 있으며
native 2315/2869의 mask·emissive를 사용한다. Dance Light의 3요소는 mesh가 아니라 sprite다.
원본 B_WP_3/4/5는 설치 모델에 없으므로 합성 본을 만들지 않고 명시적인 BOSS root 그룹으로
제공했다. `스포트라이트_세이튼댄스`는 기존 LightResources runtime 3과
`kakulsaydon.g1.presentation.28`을 재사용한다. 기존 P7 세이튼_룰렛의 0–33669ms BOSS follow
occurrence가 이미 연결돼 있으므로 중복 생성하지 않았다. 기존 `boss.kouku.dance` 바닥 카드
문서를 이 Aura와 동일한 원본으로 취급하지 않았다.

칼날 댄스 원형 예고는 원본 2122/EX02다. 직접 대응 notify가 없는 도넛 예고는 원본 geometry를
판정 반경에 맞춰 조합한 저작 그룹이며 원작의 직접 추출 occurrence라고 주장하지 않는다.
쇼타임 2112/EX01의 색·크기와 혼합하지 않았다. 실제 소비자에서 발견한 v15 필수
runtimeExtensions 누락 및 disabled attachment에 남은 basis를 수정했으며, basis 회전은
독립 particleSystem yaw로 옮겨 원래 방향을 보존했다.

알비온은 RPCT07 4219903의 파란 Decal 2102 → Projectile 421990319 → ThunderStorm
폭발을 사용한다. 붉은 감전 반복 장판의 Decal 1002는 별개다. 부채꼴은 원본 한쪽 70도·11m를
0/90/180/270도에 네 번 배치한다. 새 그룹은 notify의 실제 int32 FRotator를 읽는다.
기존 전체 action importer의 float3 회전 파서는 이 변경에서 전역 수정하지 않았다.

지팡이 회전은 본체 4219820/4219948의 Att_Battle_10_02이고 실제 십자는 SkillEffect →
NPC 480606/480607 → MN_ISTM_00-4 Action 4222003의 두 방향이다. 원본은 예고 3초,
폭발 3.15초, 판정 3.3초이며 직사각형 16m×2m 두 개를 90도로 교차한다. 본체의 disabled
FireWave를 임의로 켜지 않았다. NPC 예고의 bKillOnDeactivate가 범용 runtime에서 metadata만
소비되는 경계는 이 그룹의 sourceTransformTrack alpha를 3초에 끊어 보존했다.
화염격자 A는 평행 5줄, B는 90도 5줄, C는 45/135도 10줄이며 서로 독립이다.

새 NPC native 3603–3606을 기존 64-ID 구간 installer로 추가했다. 856개 기존 프로그램을
유지하여 총 860개가 됐고 신규 C++ 파일이나 shader carrier는 만들지 않았다.
MIC 고유 cooked texture 배열의 index를 맞추고 기존 native 3060/2804/2438을 재사용했다.
Resources의 필요한 WModel/DDS는 실제 물리 위치에 설치했으며 Git에는 추가하지 않는다.

### 알비온 플레이어별 생성

Composition에 `kakulsaydon.g1.logic.44`, 표시명 `알비온_플레이어장판`을 등록했다.
`ALBION_BLUE_CIRCLE`의 기본은 플레이어당 1개, 반경 0m, 수명 7000ms다. 다수 설정은
플레이어 주변 원주에 2..8개를 놓고 반경은 (0,20]m다. 시작 순간 살아 있는 플레이어 위치를
Server가 확정하고 모든 위치의 navigation을 검사한 뒤 기존 CombatObjectRuntime으로 함께
commit한다. 실패하면 기존 객체를 유지한다. Shared spawn/snapshot/despawn과 기존 Client
CombatObjectProjectionRuntime을 사용하며 별도 Client 위치 선정이나 피해 판정은 없다.

BossCatalog 6개 Kouku body에 같은 stable combatobject/visual ID를 등록했다.
실제 effect는 `effect.kouku.albion.bluecircle.warning.impact.runtime`이며 2초 예고 뒤 폭발한다.
이 Logic은 효과 생성만 담당하고 피해를 추가하지 않는다. Sequence의 그룹 배치와 실제 Server
Logic 실행을 구분하며 Logic 시작 시점은 사용자가 Pattern에 Append해 정한다.

### 검증 증거와 실행 범위

37개 실제 Client codec admission, serialize/parse roundtrip, 60Hz Playback과 deterministic
seek를 확인했다. resource staging은 창·swapchain·draw 없이 WARP와 실제
CEffectObject::Stage_Document를 사용했다. 수치 증거는
`out/KoukuPatternGroups20260912/CPU/codec_playback_final.json`과 `resource_staging.json`에 있다.
이 결과는 최종 화면 색·밀도·위치·타이밍의 사용자 판정을 대신하지 않는다.

신규 native의 particle3584와 모델 Shader_VtxAnimMeshBinary를 실제 FXC fx_5_0 /O1로
컴파일하여 exit 0을 확인했다. 독립 출력과 명령은
`out/KoukuPatternGroups20260912/shader_compile/results.json`에 있다. library installer를
37개 manifest로 재실행한 dry-run은 변경 파일 0이었다. 이후 최종 수명·제품 검증은 아래에
추가 기록한다.

최종 native staging은 새 particle3584/모델 CSO를 probe 옆에 설치한 뒤 37개 모두 통과했다.
최종 근거는 `CPU/resource_staging_final.json`, 입력 hash와 합계는
`CPU/validation_summary.json`이다. `lastVisibleSeconds`라는 기존 probe 필드는 alpha=0인
row도 세므로 실제 가시 시간으로 사용하지 않았다. 십자 예고의 alpha는 2.9초에 양수,
3/3.1/4.9초에는 0인 것을 따로 확인했다.

Playback의 emitterDuration×loops+emitterDelay 및 visible=false 제외 규칙과 manifest의
기본 길이를 대조해 여섯 항목을 동기화했다. 지팡이 바닥은 5551ms, 회전은 2600ms,
십자/한 줄 폭발은 1500ms, 조명/폭발+조명은 1750ms다. 예고 2/1.7/3초는 원본 가시
구간을 유지한다. authored 14개 hash는 바뀌지 않았으며 `albion_cross/duration_validation.json`
및 `duration_installation.json`에 근거가 있다. Composition 최종 source revision은 358이다.

Server의 기존 16개 계약과 알비온 8개 계약, 총 24개가 통과했다. 살아 있는 플레이어의
grounded 생성, no-damage, snapshot/reconnect의 원래 spawn clock·pinned revision,
일부 생성점 실패의 전체 rollback, radial 배치, 7초 만료, 자연 완료 tail 및 수동 Stop 취소를
실제 GameRoom/CombatObjectRuntime으로 확인했다. `albion_contract/run-final.log`를 따른다.
Client의 level-owned V1은 별도 handle kind를 사용해 despawn 때 대기/활성 Effect를 정리한다.
자연 패턴 완료 후 보스의 현재 revision과 이미 생성된 객체의 pinned revision을 혼동하지 않는다.

Python 알비온·gaze/HUD/card-maze 회귀 4건과 수정 CPP 독립 컴파일이 통과했다.
첫 공식 publisher는 기존 Kouku body admission의 combatObjectVisuals=0 제한에서 실패했고
기존 Product/Server 출력·receipt가 rollback됐다. 기존 body·weapon·armor 조건을 유지하면서
빈 목록 또는 정확한 알비온 visual 한 개만 허용하도록 같은 publisher를 수정했다. 실제 본문
검증에서 6개 body/이전 빈 목록 허용과 잘못된 ID·중복·추가 필드·body 거부를 확인했다.

추가 소비자 검토에서 기존 Loader의 combat-object prewarm이 Valtan만 처리하는 누락을 찾았다.
Kouku Arena에서도 ActorCatalog의 6개 body가 참조한 V1 visual을 중복 제거해 한 개 target을
기존 Queue_ProductTargets_Priority → Begin_LoadingProductCuePreparation → activation probe에
넣었다. Spawn의 cache-only admission과 준비 실패의 기존 격리 진단 정책은 유지했다.
ActorCatalog H/CPP와 Level_Loading CPP만 확장했으며 신규 C++ 파일은 없다. 두 CPP의 독립
컴파일도 통과했다. `albion_cross/prewarm_compile/compile.log`에 근거가 있다.

최종 source 358은 공식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
-ExpectedKoukuSaydonSourceRevision 358`로 Product·Map·쿠크 World·Gameplay balance 모두
게시 성공했다. 저장 36패턴 중 제품 조건을 충족한 31패턴, 246 stage와 8 bundle을 게시했다.
미배치 Logic 44는 저작 목록에 남으며 사용자가 Pattern에 Append·Save한 뒤 다시 Publish한다.
실행 기록은 `out/KoukuPatternGroups20260912/product-publish-358.log`이며 기존 player hit-shape
partial coverage 경고는 이 요청의 신규 실패가 아니다.

최종 Client/Server Debug x64 제품 증분 빌드는 모두 exit 0이다. 같은 MSBuild 18 Insiders
amd64와 기존 프로젝트 설정을 사용했고 제품 EXE·CSO·runtime dependency 배포가 완료됐다.
Client.exe는 18:11:36, Server.exe는 18:01:02 산출물이다. 로그는
`client-product-build-final.log`와 `albion_contract/server-product-build.log`다.
Client의 기존 C4819/C4828 인코딩 경고는 남았으며 이 작업에서 무관한 파일 인코딩을 변환하지 않았다.
37개 등록·duration·JSON/XML/Python 구문 검사 및 최종 변경 범위 `git diff --check`도 통과했다.
등록기 재실행은 변경 0이고 authored 최종 hash는 CPU/resource staging 입력과 같다.

18:12 확인 당시 사용자 Server PID 28752와 Client PID 85052는
`out/InteractiveRuntime/20260912_174839`의 이전 복사본을 실행 중이다. 에이전트가 이를
종료·재실행하거나 UI를 조작하지 않았다. 사용자가 이 세션을 닫고 Visual Studio의
Server + Client profile을 Ctrl+F5로 시작하면 위 제품 빌드를 사용한다.

Effect 확인은 F1 → Effect Tool V1 → All Effects → KoukuSaydon → 관문별 패턴 폴더에서
그룹을 선택해 Append한다. 실제 플레이어별 생성은 Action Workbench의 Resources → Logic →
`알비온_플레이어장판` → 원하는 Pattern/커서 → `Append Logic at Cursor`로 배치한다.
개수·반경·수명은 Apply Values로 적용하고 Save → Publish All Patterns 후 Server를 재시작한다.
사용자 최종 화면·부착·밀도·원작 비교는 미실행이며 visual PASS로 기록하지 않는다.

다른 세션의 미커밋 소스·문서·맵 변경과 같은 checkout을 사용하므로 자동 stage/commit/push는
하지 않았다. 사용자 Pattern 타임라인·기존 합성 payload와 다른 세션의 빌드 프로세스를 보존했다.

## G09. 백스탭 불뿜기·수직 화염링·분신 브레스

사용자가 첨부한 두 이미지를 열람했다. 첫 장의 수직 주황색 링·수평 화염포와 둘째 장의
검붉은 브레스를 기준으로 기존 원본 leaf와 설치된 native 재질·DDS·WModel을 재사용했다.
신규 독립 그룹 7개, 총81요소를 실제 Authored·Catalog·EffectResourceTree·Composition
presentationResources와 Client project/filter의 None에 등록했다.

| `effect.kouku.gate3.` 이하 asset ID | 표시명 | 요소 | 실제 Playback 수명 |
|---|---|---:|---:|
| `backstep.flame` | 백스탭불뿜기_확대화염포 | 11 | 2.2초 |
| `backstep.ground` | 백스탭불뿜기_바닥불길 | 3 | 3.1초 |
| `backstep.full` | 백스탭불뿜기_화염포·바닥 | 14 | 3.1초 |
| `backstep.ring` | 화염링_생성·유지 | 10 | 9초 |
| `backstep.ring.flame` | 화염링_동일화염포 | 21 | 9초 |
| `backstep.ring.end` | 화염링_원본종료잔불 | 7 | 4초 |
| `clone.breath` | 분신소환_기분나빠_브레스 | 15 | 3초 |

백스탭 항목은 `KoukuSaydon/3관문/패턴/세이튼/백스탭 불뿜기` 아래 화염포·화염링,
브레스는 같은 세이튼 아래 `분신 소환·기분나빠`에 들어간다. 첫 세 항목의 기본 Resource 길이는
각각2201/3101/3101ms로, 원본 float 합계를 올림하면서 실제 수명보다1ms 여유를 둔다.
나머지는 표의 수명과 같다. 링+화염포는 사격 뒤 링이 유지되며 종료 잔불은 별도 그룹이다.

### G09-01. 원본과 회전·크기 연결

링 원본은 `Par_G_RPCT_05_FireRing_01_LOC_INT`이며 RPCT05 action4219825 /
RPCT07 action4219951 → SkillEffect421982504 → Projectile421982502에서 소비한다.
이름이 비슷한 action4219956의 Sk_04 cone/half-cylinder는 이 hoop와 다른 효과다.
Source TypeDataMesh의 `pitch=-90`이 기존 leaf의 typed detail에서 빠져 있었으므로
새 그룹의 `sourceTypeDataRotationDegrees=[0,-90,0]`에 연결했다. 기존 runtime의
pre-rotation 소비 경로를 사용하며 전체 shader나 다른 emitter를 회전시키지 않았다.

원본 Projectile의 scale.7·높이1.1m를 유지하고 독립 그룹의 pivot만 링 중심에 맞췄다.
raw hoop 정점은 XZ 면이며 preScale.01을 사용한다. 실제 Playback의0.3초 대표 표본에서
822정점을 전부 변환한 root-local bounds는 최소 `[-1.095937,0.003930,-0.084996]`,
최대 `[1.095938,2.196071,0.084994]m`다. 수직 XY 면, 법선 `[0,0,-1]`,
중심 `[0,1.10000014,0]m`, 외경 약2.19m이며 shader 변형 전 수치다.
원본 AABB만 회전하면 폭·높이가 약2.76m로 과대하므로 위 실제 정점 결과를 사용했다.

공용 화염포는 Saydon `Par_L_RPCT_05_Sk_01_LOC_INT`의 sprite11개이며 사용자가 요청한
확대를 uniform scale2.0으로 적용했다. 같은 leaf의 decal3개는 바닥 그룹으로 분리했다.
백스탭과 링 합성의 화염포 설정은 ID를 제외하고 동일하다. 입구 `[0,1.1,0]m`와 +Z 전방을
공유하며 평균 입자 속도 `[-0.526,0.335,4.324]m/s`로 전방 분사를 확인했다. 이는 독립 leaf의
2배 저작값이며 기존 body/socket basis1.7이나 인형 CDMD00의 별도 화염 레시피를 바꾸지 않았다.

붉은 브레스는 `Par_V_RPCT_Breath_01_LOC_INT`와 action4219917의
`rpct00_att_battle_31_01` notify008을 사용했다. 원본 시작1.989097초·notify지속2.98073초,
위치 `[1,.9,0]m`, 배율 `[1,1.1,1.1]`을 보존하고 독립 root의 전방을 yaw-90으로 맞췄다.
색 override `[1,1,1]`과 native 적색·검정·주황 입력을 유지했다. 별도 적색 tint를 곱하지 않았다.
실제 수명은 finite source recipe1초와 particle tail을 포함한3초이므로 기본 Resource도3000ms다.
원본 action 이름은 '광기의 표식'이고, 사용자 장면명 '분신 소환·기분나빠'와의 대응은 첨부
이미지에 따른 후보 매핑이다. 위 원본 사용 경로와 사용자 최종 장면 판정은 구분한다.

### G09-02. 생성·등록과 검증

`Tools/EffectPipeline/build_kouku_backstep_flame_groups.py`와
`build_kouku_clone_breath_group.py`는 기본 실행에서 out 후보·manifest만 만들고
`--install`에서만 새 Authored를 new-or-equal로 설치한다. 현재 설치본은 후보와 바이트 동일하다.
기존 `install_kouku_effect_library.py --library-only`로7개를 등록했고 같은 입력의 최종
dry-run은 변경0이다. 기존 Resources 물리 파일을 재사용하며 binary payload와 새 shader는 없다.

기존 CPU probe를 현재 codec/Playback 등14개 Client translation unit과 함께 격리 재컴파일했다.
7개 모두 실제 codec load·drawable validate·save/load roundtrip·stage·60Hz finite/completion·
deterministic seek가 PASS(exit0, stderr0)다. 컴파일 입력 hash와 실제 probe include closure를
대조했고, 다른 작업의 renderer header 변경은 이 CPU 호출 경로 밖이다. 기존 인코딩 경고는 남는다.
이번 그룹의 GPU resource stage·draw 또는 새 Client 화면 검사는 실행하지 않았다.

증거는 `out/KoukuSaitenGroups20260912/CPU/target_codec_playback.json`,
`target_numeric_summary.json`, `hoop_vertex_bounds.json`과 같은 폴더의 compile/link log다.
원본·후보·설치 기록은 같은 out 루트의 `backstep`, `clone`, `library_installation.json`,
`library_recheck.json`에 있다. 이 out 증거는 소스 커밋 대상이 아니다.

등록 검토에서7개 asset이 Catalog의 DIRECT_AUTHORED_DOCUMENT, Tree V1 경로,
Composition EFFECT/V1_EFFECT/BOSS와 duration, project/filter96.DataFiles에 각각 한 번씩
연결됨을 확인했다. 신규7개와 공용 JSON은 중복 key·parse, project/filter는 XML parse를 통과했다.
등록 직전 baseline의 Catalog384개, Tree28개 node·181개 reference, Composition133개 Resource의
내용과 순서를 보존했다. Composition42개 Pattern은 그대로이며 revision398→399와 Resource7개
추가만 있다. project/filter 기존 항목 삭제·변경도 없고 None7개씩만 추가됐다.

사용자가 요청한 방향·크기 재발 방지 절차를 AGENTS의 '이펙트 복원의 방향·크기 점검',
gotchas의 occurrence별 진단, 렌더링이펙트복원V2의 E05와 현재 연결 범위에 반영했다.
TypeData·particle 회전, 원본 단위·notify 배율·사용자 확대, 실제 geometry·재생 행렬 검증과
화면 판정을 분리하고 새 원인이 확인되면 같은 변경에서 이 문서들을 갱신하도록 했다.

### G09-03. 사용자 확인과 남은 경계

F1 → Effect Tool V1 → All Effects → Refresh → KoukuSaydon → 3관문 → 패턴 → 세이튼에서
새 폴더와 그룹을 선택한다. Composition Resources → Effect에서도 선택 후 원하는 Pattern에
Append할 수 있다. 본 변경은 독립 Effect 라이브러리이며 사용자 타임라인의 패턴·링 배치 수·
백스탭 이동이나 Server 전투 규칙을 임의로 작성하지 않았다.

확인 당시 Client PID15004와 Server PID80532는 각각 제품 Debug 경로에서 실행 중이었다.
에이전트는 종료·재실행·UI 조작·화면 캡처를 하지 않았다. 새 그룹은 직접 읽는 Authored 데이터이며,
새 Client 바이너리를 요구하는 다른 작업의 변경과 구분한다. 사용자 최종 화면의 크기·색·밀도·
분사 방향과 원작 장면 비교는 대기 상태이며 visual PASS를 기록하지 않는다.

### G09-04. Composition Effect의 Created Resources 높이

추가 사용자 요청에 따라 `KoukuSaydonActionWorkbench.cpp`의
`Render_PresentationResources`에서 `##CreatedPresentationResources` child의 Effect 높이를
110→330px로 정확히3배 늘렸다. EFFECT의 V1/V2 선택에 적용되며 다른 Resource family의
140px와 선택·Append·scroll 동작은 유지한다. 새 상태·파일·public interface는 없다.

수정 직전 파일을 다시 읽고 단일 literal만 바꾸어 기존 바이트 인코딩·CRLF10,976개와 다른
세션의 변경을 보존했다. 현재 Engine/Public을 소비하는 Workbench TU의 격리 컴파일은
exit0, error0이며 기존 C4828 경고는 남았다. 컴파일 도중 해당 소스 hash는 동일하다.
근거는 `out/KoukuSaitenGroups20260912/CreatedResourcesCompile/compile.log`와
`change_receipt.json`이다. 제품 EXE 빌드와 실행 중 UI에는 아직 반영하지 않았다.

최종 신규7개 JSON·공용 JSON/XML parse, 두 생성기의 Python 구문과 백스탭 재생성 동일성,
변경 범위 `git diff --check`가 통과했다. `final_data_checks.json`에 설치 문서 hash와 검증을
남겼다. 데이터 Refresh와 C++ 패널 변경의 제품 반영은 다르며, Client/Server 종료를 기다리는
기존 이동 예측의 최종 제품 빌드와 함께 후자를 확인해야 한다.

## G10. V1 MAP 배치의 편집·저장·Preview 연결

### G10-01. 확인한 결함과 수정

첨부 오류는 P37의 `Requested root time has not been recorded`다. 기존 Workbench는 Effect
placement를 stage할 때 TRS만 복사했으므로 Anchor만 바꾼 값이 Dirty, Preview/Play, Save에
전달되지 않을 수 있었다. 현재는 Effect의 anchorKind·followBoss·bone·boneTarget·worldId·
worldOccurrenceId·worldEmissionIndex와 TRS를 함께 보존한다. Collider는 기존 TRS 범위를 유지한다.
Append는 Resource defaultAnchorKind를 소비하며, 독립 Resource Preview는 BOSS/follow=false다.

MainApp은 선택 Pattern의 actor·gate·stable boss placement를 Resource Preview에 전달한다.
원본 SourceModelPreview가 없는 오망성도 기존 single-member Bundle과 CNpc/CModel 준비를
사용한다. Preview actor를 플레이어 위치로 강제 이동하던 처리를 제거하고 기존 Level의 보스
위치 선택을 따른다. 실제 저장·재생 MAP 좌표는 절대 월드 미터이며 보스가 이동해도 고정된다.

PresentationPlayer는 MAP의 bone/world/follow 조합을 검증하며, 배치 편집으로 anchor가 바뀌면
해당 occurrence의 V1/V2 handle과 anchor cache만 갱신한다. 보스 이동의 기록되지 않은 과거를
고정값으로 채우거나 History 오류를 숨기지 않는다. 정상 MAP/follow=false는 수정 전 provider
검사에서도 통과했으므로, V1 renderer 전체가 MAP을 지원하지 않는 구조적 결함으로 단정하지 않는다.

실행본 복구에서 P37의 MAP/follow=false 자체는 보존돼 있었지만 위치는 `[0,0,0]`이었다.
이는 3관문 중앙과 다른 월드 원점이다. 이 사실은 최초 오류 시점의 미저장 상태를 복원한 증거가
아니며, 현재 보이지 않는 배치 문제와 별도로 기록한다. 중앙 위치 보정은 G11 복구 후 적용한다.

### G10-02. 검증

실제 Workbench의 Request/Consume, Dirty/선택 왕복, Pattern/Bundle/부모 확장, Duplicate/Revert,
Save/Reload와 Collider TRS 분리가 기존 native probe에서 PASS다. UI Render와 scene camera
조회만 out 사본에서 비활성화했으며 command·저장 함수는 제품 함수 그대로 실행했다.
원문 Workbench, MainApp, PresentationPlayer translation unit도 각각 격리 컴파일했다.

최종 Workbench 원문을 다시 컴파일하고 같은 native command probe를 실행했다. 실제
Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json을 out fixture로 복사해 기존 편집·저장
검사를 유지했으며, 실제 MAP Append의 보스 spawn 초기값 → Preview → Save/Reload가 PASS다.
World source 누락·손상 시 Append가 오류를 남기고 draft와 저장 파일을 보존하는 것도 확인했다.
기존 보스 spawn helper 검사는 최신 함수·lookup 추출본과 실제 WorldGameplayDocument를
재컴파일해 정상6개·거절11개, 총17개 PASS다. 제품 source 변경 없이 out probe/fixture만 보완했다.


별도 기존 provider probe는 실제 production 함수 추출본과 최신14개 codec/Playback TU로
MAP 첫 비영 시점·rewind·보스 이동 중 고정·frozen BOSS·누락/teleport strict History와 잘못된
MAP/WORLD dependency를 확인했다. 수정 전 유효 MAP7건, 최종11건 PASS, stderr0이다.
이는 Service queue·실제 live actor·GPU·Client UI 실행 검증이 아니다.

근거: `out/KoukuParentTimeline20260912/workbench-probe/run.log`,
`out/KoukuMapAnchorHistory20260912/CPU/anchor_history_validation_summary.json`,
`out/KoukuMapAnchor20260912/{MainAppCompile,PresentationCompile}`.
최종 command/Append 근거는 `out/KoukuParentTimeline20260912/workbench-probe/final-validation-summary.json`,
helper17개 근거는 `out/KoukuMapBossSpawn20260912/CPU/{source_evidence.json,validation.log}`다.
원문 인코딩·CRLF와 다른 세션 변경을 보존했다. 기존 C4828 경고는 남는다.

## G11. 미저장 패턴 복구와 3관 KoukuSaydon 조립

### G11-01. 저장 잠김과 읽기 전용 백업

실행 중 Client15004의 기준본은 revision397, 디스크는399였다. 차이는 revision과
presentationResources129→140의 신규11개 append뿐이고 기존129개의 순서·내용은 같았다.
리소스 등록 뒤 기존 Save의 CAS 검사와 `Is_Fresh=false`가 Save 버튼을 잠근 상황이다.

Client PDB의 `CMainApp::s_pActiveInstance → m_pKoukuSaydonActionWorkbench → m_Draft`
정확한 필드 경로와 Debug STL layout을 사용해 읽기 전용으로 복구했다. 프로세스 정지·쓰기·
함수 실행·전체 memory dump·stack/heap scan·UI 조작은 없었다. 22:27:03, 22:29:28,
22:30:20에 회수한 Composition Draft는 동일했다. 마지막 확장 상태도 연속 두 번 일치했다.

Draft는46개 Pattern/10개 Bundle이다. 새 P43~46 외에 P37의 MAP Effect occurrence 추가와
P42의 첫 idle Stage 삭제를 함께 보존해야 한다. staged geometry는0개였다. 별도 Sequence
Workbench의 Draft7개 Pattern/revision7은 dirty=false이며 함께 백업했다.

out의 typed JSON reader는 실제 헤더의 모든 필드를 소비하고 제품 Serialize/Parse_Text/Validate를
호출했다. Composition/Sequence 각각 Draft·LastGood 총4개 문서의 raw→native→roundtrip이
완전히 일치했다. 객체3,049개·필드47,223개·leaf50,475개, unknown/missing field0이다.
이후 프로세스 목록에서 Client/Server가 종료돼 마지막 백업 뒤의 live 재대조는 불가능했다.

정확한 보존본은 `out/KoukuDraftRecovery20260912/composition.m_Draft.authored.json`,
검증은 같은 폴더의 `recovery-validation-summary.json`, `native_recovery.run.json`,
`composition.structure-diff.json`과 시각별 `.receipt.json`이다. 실제 파일 복구 결과는 아래에 기록한다.

### G11-02. Save 재발 방지와 표시 이름

기존 Save_Atomic은 last-good를 유지하고 외부 변경이 revision 증가와 신규 Resource prefix append
뿐일 때만 사용자 candidate에 신규 Resource를 합친다. stable ID payload 충돌 또는 Pattern 등
다른 외부 변경은 거절한다. 기존 writer lock, 재읽기 CAS, 임시 파일 검증·원자 교체를 유지한다.
Save 버튼은 stale 상태에서도 재시도 가능하며 Publish/Server Play의 freshness는 유지한다.

외부 Resource append+새 사용자 Pattern 동시 보존, 외부 Pattern 변경의 반복 Save 거절,
같은 Resource ID의 다른 payload 충돌 시 양쪽 보존이 실제 native Workbench probe에서 PASS다.
GATE3의 MN_RPCT_05 표시만 KoukuSaydon으로 바꿨다. 저장 stable ID와 다른 관문의 이름은 같다.

### G11-03. 실제 저장과 중앙 좌표

검토한 후보를 실제 Document Reload→Save_Atomic→새 Document Reload로 저장해 revision400,
46개 Pattern/10개 Bundle/140개 Resource를 확인했다. 원본 사용자 Draft 대비 revision과 외부
신규 Resource11개 외 변경0이다. `recovery-commit-receipt.json`과 `composition.recovered-rev400.json`
에 복구 직후 정본·검증을 남겼다.

그 위에서 사용자가 요청한 P37 오망성 occurrence의 MAP 좌표만 정본
`boss.kakulsaydon.g3.saydon`의 `[-0.0700000003,1.32000005,942.330017]`로 설정했다.
start6047ms·duration8953ms·회전·크기·나머지 Pattern은 보존했다. 기존 native Save_Atomic과
재로드 전체 동등 검사를 사용한 revision401이다. `out/KoukuMapAnchor20260912/map-center-change.json`
과 `composition.rev400.before-center.json`에 원래 상태와 변경 근거를 남겼다.

### G11-04. 지팡이와 어깨 쿠크

원본 MN_RPCT_05/G1과 MN_RPCT_07/G3 LookInfo는 모두 Body MN_RPCT_05_SK,
WP_MN_RPCT_05_SK + WP_MN_RPCT_05_Ani, WP_1_20→b_wp_1을 명시한다.
설치된 WP05 모델·native material을 재사용해 BossCatalog의 G1_SAYDON/G3_SAYDON에
weaponModel, preScale0.01, preRotation `[-90,0,0]`을 연결했다. G2/BINGO의 기존 튜닝은 보존했다.

실제 WP05는1개 mesh/5개 bone/15,172개 정점이며, 같은 body .017의 손 본에 붙인6개 pose의
원점 오차0·world 배율1.7(약1e-6 오차)다. 100배가 중복되지 않는다. 무기에는 실제17_01 clip
1개가 있으므로 기존 CNpc 무기 pose 동기화의 normal/transition과 Python projector에 같은
정확 lookup을 추가했다. 없는 clip은 기존 rest pose를 유지한다. 기존06 동작을 포함한 C++10개
lookup·Python 집중 검사와 NpcPresentationAssetService TU 컴파일이 PASS다.

추가로 원본 PSK와 설치 WModel 전체15,172정점·세 축을 비교했다. 원점과 반경이 같아도
rotation0에서는 Y/Z축이90도 어긋났으며 원본 결합 대비 최대 정점 오차0.287559m였다.
DirectX X축 pitch -90도를 적용하면 최대9.65e-7m, RMS6.81e-7m, 축 각도 오차1.8e-5도
이하로 줄어든다. identity 손 소켓에 붙는 원본과 설치 geometry의 변환 차이를 catalog에서
보정했다. 근거는 `out/KoukuGate3BossAssembly20260912/weapon_basis/weapon_basis_evidence.json`과
`catalog-application.json`이다. 원점·반경 검사만으로 방향까지 검증됐다고 해석하지 않는다.

어깨 쿠크는 현재 몸체 slot4의27,276개 정점과47개 bip002 bone에 이미 들어 있다. idle0에서
높이 약0.99m, finite·nonzero이며 별도 쿠크 모델을 중복 추가하지 않았다. 원본 G3 slot4는
MN_RPCZ_00_MI이고 현재 설치 body의 재질과 같다. G1은 MN_RPCZ_00-2_MI로 다른 원본
material variant이며 관문별 숨김 의미는 아직 확정하지 않았으므로 임의 hide를 추가하지 않았다.
WP08_1은 head/WP_3_1에 붙는 별도 부품이며 어깨 쿠크가 아니다. 미해독 표시 flag를 추측해
추가하지 않았다. 최종 부착 방향·표시 판정은 사용자 화면 확인으로 남긴다.

측정·컴파일 근거는 `out/KoukuGate3BossAssembly20260912/model_assembly_cpu_evidence.json`,
`focused_validation.json`, `Compile`이다. 기존 bone-contact 광역 case는 수정 전후 모두 동일한
fixture의 Pattern 수명 초과에서 lookup 전에 중단됐으며 이를 이번 수정 PASS로 기록하지 않았다.

### G11-05. 3방향 불뿜기 라이브러리

`build_kouku_threeway_breath_group.py`가 원본 MN_RPCT_07 Action4219940의 stage0/1 notify를
다시 decode하고, 기존 동일 emitter의 native material과 geometry를 재사용한다.
`effect.kouku.gate3.threeway.breath.full.restore`는77개 element이며 지팡이 FX_Prj_01,
보스 입 FX_Prj_02, 어깨 쪽 FX_Prj_03에 각각20개, MidControl4개·root13개다. 원본 각 socket,
어깨 쪽 local offset, particle parameter의 미지정 기본값과 시간·배율을 보존한다.
이전1관문 두 방향 문서는 수정하지 않았다.

현재 CModel에 실제 MN_RPCT_05를 읽고, 제품 source-anchor 함수와 실제14_01/14_02 animation을
샘플링했다. native codec/drawable validate·정확 roundtrip·60Hz Playback665 samples가 PASS,
anchor4개·peak CPU row172개·error0이다. WARP device는 CModel 준비에만 사용했으며 Client/UI
실행·화면 캡처·visual PASS는 없다. 근거는 `out/KoukuThreewayBreath20260912/CPU/actual_model_result.json`
및 같은 폴더의 compile/link/source span 기록이다.

추가 정렬 검사에서 UE socket 좌표를 설치 PSK/FBX bone basis로 옮기는 변환 누락을 확인했다.
원본 PSK30,470점과 설치169,810정점의 양방향 최대오차는1.16e-7m이고,165개 bone의 bind map은
`.01 * reflectZ`와1.05e-6 이하 오차로 일치한다. UE export mirror와 particle 좌표계를 함께
합성해 이 생성기의 실제 사용4개 socket에만 적용했다. FX01 위치는 `[.32,0,.15]`,
FX02는 `[-.03,.07,0]`, FX03는 `[-.03,-.32,0]`, MidControl은 `[.375,0,0]`이며
회전도 같은 원본 행렬에 맞췄다. 원본 본·FRotator·scale이 달라지면 생성기가 거절한다.
전역 socket extractor와 다른 authored Effect, 원본 bone 선택·notify offset은 변경하지 않았다.

수정 범위는64개 element의 socketLocalTransform164개 leaf뿐이며 root13개와 재질·입자·시간은
그대로다. 원본14_01/14_02 PSA와 실제 CModel의 같은 시점에서 발생 행렬 witness를 비교했다.
정규90도 runtime 회전과 실측 bind 투영의 최대 행렬오차는5.573e-7이다. 올바른 원본 proxy socket도
입 정점과 떨어질 수 있으므로 입 본으로 임의 교체하지 않는다. LookInfo·실제 부착과 두 AnimSet을
대조했고 다른14_01/14_02 clip이나 FX03 override 근거는 발견하지 못했다. 미해석 LookInfo 숫자
필드와 procedural AnimTree, 최종 GPU 화면 판정은 이 source 투영 검증에 포함하지 않는다.

보정한 실제 authored candidate를 기존 native codec·production provider·CModel Playback으로
다시 검사해665개60Hz 표본,4개 anchor, peak row172, error0을 확인했다. 기준 authored bytes를
확인한 뒤 원자 교체했으며 사용자 Composition과 다른 Effect를 변경하지 않았다. 최신 근거는
`out/KoukuThreewayBreath20260912/BasisCorrected/{actual_model_result.json,installation-receipt.json,diff.json}`과
`out/KoukuGate3BossAssembly20260912/{body_socket_basis_evidence.json,projected_body_socket_contract.json}`이다.

별도 첫 입자 검사도 실제 CNpc와 같은 root-motion suppression을 적용한 CModel·제품 provider·
CEffectPlayback으로 실행했다. 전후 각각181개 표본에서18개 최초 입자를 포착했다. 같은 발생시각의
17쌍은 원본 입자→보정 좌표 오차 최대2.076e-7m이며, 원본 PSA/PSK 직접 계산과는 위치0.000724m,
분사 방향0.01553도 이하다. 한 emitter는 이동거리 기반 spawn 때문에 첫 발생이 한 tick 달라
같은 입자 비교에서 제외했다. 처음 나타난 공통 이동 차이는 독립 probe의 NPC root suppression
누락을 교정해 해결했으며 제품 변경이 아니다. 근거는 `birth_projection_evidence.json`이다.

Authored new-or-equal 설치 후 기존 library installer로 Catalog, Tree, Composition Resource와
project/filter None에 등록했다. Composition은 Resource1개 추가만으로 revision402가 됐다.
패턴/세이튼/3방향 불뿜기에서 선택·Append할 수 있으며 원본 model preview는3관 KoukuSaydon이다.
사용자 Pattern에 임의 시점·반복 횟수를 추가하지 않았다. 원본 Action은 여러 stage 반복을 갖지만
이 Resource는 첫 시전·발사 한 묶음이고 입자 잔상을 포함해11,075ms로 등록했다.


## G12. KoukuSaydon Arena 시작지점과 트리거 재시험

### G12-01. 연결한 동작

F1 → KoukuSaydon Arena의 `1관문 - 세이튼` 바로 위에 `시작지점` 버튼을 추가했다.
기존9개 gate 배열과 index는 유지한다. 같은 아레나의 보스와 보스 소유 연출·종속 개체를
정리하고, 요청한 플레이어만 정본 spawn으로 돌려보내며 입장 트리거를 다시 사용할 수 있게 한다.
버튼 옆 설명에 같은 아레나 보스·트리거 초기화와 요청 플레이어만 이동하는 영향을 표시한다.

Level → PlayerController → IPlayerCommandSink → NetworkPlayerCommandSink가 기존
`C2S_DEBUG_TELEPORT_TO_PLACEMENT`로 stable ID `player.spawn.kakul.party01`만 보낸다.
Server는 session/player 소유와 KoukuSaydon world를 확인한 뒤 활성 PLAYER_SPAWN을 찾고,
기존 position teleport의 navigation·높이·collision·생존 상태 검증을 재사용한다. Client에
spawn 좌표를 복제하지 않으며 player Transform을 직접 바꾸지 않는다. 기존 stage marker
요청과 packet codec은 변경하지 않았고 기존 debug movement 결과·sequence를 공유한다.

Server는 새 CServerTriggerSystem을 먼저 준비하고, 이동 검증 후 초기 활성 보스를 포함한
모든 비-Esther boss와 소유 종속 개체를 제거한다. 저장 순서가 owner보다 앞인 자식도 ownership
closure에 포함한다. 관련 없는 NPC·monster·Esther와 다른 플레이어의 위치는 보존한다.
기존 PatternAudition 종료가 owned cue·attachment·projectile을 정리하고, 같은 world의
PLAY_SEQUENCE 트리거 대상에 STOP을 broadcast한다. once/inside trigger 상태와 카드미로·빙고
진행을 초기화해 다음 진입에 기존 sequence PLAY와 Server party placement가 다시 실행된다.

Client는 Server 승인 뒤 gate/HUD target, 로컬 World preview·owned cue·popup cutscene boss,
입장 marker·camera/fade 상태를 정리한다. MainApp은 Engine Update 직후 승인을 소비해
Preview Update/Advance보다 먼저 Complete Play를 취소하므로 같은 프레임의 sequence 완료가
관문 재생성을 예약하지 않는다. 시작지점 pending은 기존 gate spawn의15초 timeout/Retire와
분리했으며, Controller가 늦은 결과를 받을 때까지 요청 sequence와 pending을 보존한다.

입력 검증 거절은 기존 player·boss·trigger 상태를 보존한다. commit 중 runtime failure는
기존 room fail-stop을 사용하고 승인 cache를 거절로 바꾼다. Handler는 해당 room failure가
있으면 성공 reply를 보내지 않고 session을 닫는다. 이미 적용한 부분을 정상 완료로 보고하지 않는다.
같은 승인 sequence의 재요청은 저장된 결과만 돌려주며 위치나 once trigger를 다시 초기화하지 않는다.

### G12-02. 검증과 남은 화면 확인

변경 Client4개 TU(MainApp, Level_KakulSaydonArena, PlayerController,
NetworkPlayerCommandSink)를 격리 컴파일했다. 최신 Server31개·Shared7개 TU를 out으로
컴파일·링크했으며 review 수정 뒤 GameRoom과 Client4개를 다시 컴파일했다. 모두 PASS다.
기존 ServerGameplayContractTests의 debug teleport 구간을 확장했으며 새 test 파일·제품
실행 경로는 추가하지 않았다. `--debug-teleport-contract-test`는 listener를 열기 전에 끝난다.

실제 Server spawn·navigation·초기 boss·once sequence trigger로 신규14개 검사가 PASS다.
다른 world 거절, 사망 상태 실패 시 보존, 요청자 이동 초기화와 다른 플레이어 위치 보존,
초기 boss와 역순 종속 개체 제거, NPC/Esther 보존, 사용한 once trigger 재진입과 duplicate
요청의 무변경을 확인했다. 전체 기존 debug teleport suite는 Mario 관련21개 실패로 exit1이다.
G12 이전 GameRoom/검사 기준본을 같은 최신 의존성과 runtime 입력으로 빌드·실행해 동일21개
실패가 메시지와 순서까지 재현됐으므로 전체 suite PASS로 기록하지 않는다.

근거는 `out/KoukuArenaStart20260912/Compile/client.log`,
`ServerContract/{Server.compile.log,Shared.compile.log,final-room.compile.log,link.log,run.log}`,
`ServerContract/Baseline/run.log`, `ServerContract/validation-summary.json`이다.
이 경로는 모두 같은 out 작업 폴더 아래다. C++의 기존 UTF-8/CRLF와 공유 변경을 보존했고
변경 source와 이 문서의 `git diff --check`를 확인했다. 기존 C4828 경고는 남는다.

Client/UI 실행·조작·화면 캡처와 live packet 교환은 수행하지 않았다. 제품 빌드 반영 뒤 사용자가
F1 → KoukuSaydon Arena → 시작지점을 눌러 초기 위치·보스 제거·입장 트리거와 시퀀스 재생을
확인해야 한다. GPU 표시와 사용자 화면 판정은 이 CPU 검증에 포함하지 않는다.

## G13. 최종 제품 빌드와 실행 준비

다른 Product 빌드의 Client·shader 작업이 끝난 뒤 2026-09-12 23:04 KST에
`Invoke-BuildAndRegression.ps1 -Profile Product -Configuration Debug`를 실행했다.
Engine, Shared, Server, Client 네 단계와 output-lock preflight가 모두 PASS이며 종료 코드는0이다.
최신 source를 대상으로 한 증분 빌드이며 Clean이나 동시 공유 출력 빌드를 수행하지 않았다.

`Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`가 생성돼 있고 Engine.dll 및
shader·필수 DLL 배포가 완료됐다. Engine 출력과 Client 배포 DLL의 SHA256이 같다.
시작지점 검증 당시 source11개와 최종 Workbench 검증 입력5개가 현재 파일과 일치한다.
근거는 `out/KoukuFinal20260912/{product-build.log,build-validation.json,ProductLogs}`와
`out/BuildPipeline/runs/20260912T140418074Z-debug-product.json`이다.

빌드 뒤의 브레스 socket 변경은 생성기와 authored JSON에만 적용해 C++ 재빌드 대상이 없다.
최종 `data-validation.json`은 revision402의46개 Pattern·10개 Bundle·141개 Resource를 확인한다.
복구 revision400과 비교해 Pattern 변경은 P37 MAP 월드 위치 세 성분뿐이며 기존140개 Resource를
보존했다. JSON 중복 key·parse, project/filter XML, 생성기 문법 및 재생성 new-or-equal도 PASS다.

이 빌드는 데이터 전체 게시나 광역 runtime 진단을 실행하지 않는다. Client/Server는 직접
시작하지 않았다. 현재 server-host LAN 설정에서는 Visual Studio의 `Server + Client` profile로
사용자가 Ctrl+F5를 눌러 실행하고, Lobby의 KoukuSaydon 진입 후 해당 화면 동작을 확인한다.
최종 process 조회에서는23:15:53에 시작된 제품 Server46652와 Client70468이 실행 중이었다.
검증한 EXE와 같은 경로·수정 시각이며 에이전트가 실행하거나 UI를 조작한 결과가 아니다.
