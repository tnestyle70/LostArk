# 쿠크 플레이어 앵커·무지개·폭죽 구현 결과와 인수인계

작성일: 2026-09-11. 사용자가 이 세션을 종료하고 다른 세션에 직접 인수인계하기로 했다.
대응 계획: [구현 계획](2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_PLAN.md).

## G00. 종료 시점의 상태

코드·원본 데이터·All Effects 등록과 Debug Product 빌드까지 반영했다. 플레이어 앵커의
독립 Play All, Action Workbench/Sequencer Benchmark의 MAP Effect Append 경로를 연결했다.
6개 실제 문서의 CPU 재생과 19개 집중 검사는 통과했다. Client/UI는 실행·조작·캡처하지 않았다.
사용자의 실제 화면 확인과 원본 영상 대비 최종 시각 판정은 남아 있다.

현재 브랜치는 `codex/kouku-gate1-sequence-playback`이다. 같은 작업 폴더에서 다른 세션의
관문 플로우, 월드 마커, 워로드, 카메라 등의 변경이 함께 진행됐다. 대규모 미커밋 변경이
있으므로 자동 stage/commit/push하지 않았다. 파일 단위 되돌리기나 전체 diff를 이 기능으로
묶으면 다른 세션의 변경이 섞인다. 아래 책임과 현재 diff를 대조해서 인수한다.

지연 원인은 두 가지였다. 이 작업에서 새 native 프로그램의 셰이더 분기 상한과 원본 엔진
입력 전달 누락을 찾아 수정하는 데 시간이 들었다. 이후에는 다른 Product 빌드와 공유 출력
폴더 잠금 때문에 순서를 기다렸다. 최종 빌드는 성공했으므로 그 빌드 대기는 해소됐다.

## G01. 실제 추가한 All Effects 항목

모두 `Data/Effects/EffectCatalog.json`의 direct authored 문서이며
`Data/Effects/Authored/<아래 ID>.effect.json`에 있다. Client project/filter의 기존
`96.DataFiles`에 None으로 등록했다. All Effects의 KoukuSaydon 내부에서 선택한다.

| EffectAssetId | 구성 | CPU 문서 재생 길이 |
|---|---|---:|
| `effect.kouku.gate1.intro.festival.full.restore` | 긴 1관문 연출, 80행 중 시각 행 70개 | 24.2005초 |
| `effect.kouku.gate1.intro.fireworks.full.restore` | 짧은 1관문 폭죽, 40행 중 시각 행 36개 | 12.2385초 |
| `effect.kouku.gate3.rainbow.grid.full.restore` | 두 방향 격자 전체, 입자 340행과 light 2행 | 7.1초 |
| `effect.kouku.gate3.rainbow.fire.warning.full.restore` | 바닥 경고와 무지개 띠, 7행 | 4.5초 |
| `effect.kouku.gate3.rainbow.fire.wave.full.restore` | 불꽃 띠·연기·색종이·별, 10행 | 2.5초 |
| `effect.kouku.gate3.rainbow.fire.light.full.restore` | 원본 순간 light, 1행 | 1.75초 |

위 시간은 CPU playback이 산정한 tail 포함 문서 길이다. 모든 행이 그 시간까지 계속
보인다는 뜻은 아니다. 폭죽의 숨은 행은 원본 `ERM_None` event/location provider다.
342행 grid에는 이미 warning/wave/light가 포함되므로 전체 패턴 확인에는 grid 한 개를 쓴다.

### 원본 연결

무지개는 원본 Action `4219916` stage002 → SkillEffect `421991412~421991421` →
고정 영역 Projectile `421991401~421991405`를 실제 설치된 LOA에서 추적했다.
0.6초 45도 5줄, 2.3초 135도 5줄과 각 줄의 위치·회전을 사용한다. Projectile의 경고
timer는 0.3초, 폭발 timer는 줄별 2.0/1.8/1.6/1.4/1.2초다. 각 효과를 양방향으로
호출하는 42개 원본 occurrence를 342개 행으로 풀었다.

`Par_V_RPCT_fire_Decal_T_01_LOC_INT`, `Par_V_RPCT_fire_BIgearthwave_01_LOC_INT`,
`Par_MP_Light`를 사용한다. 6×6 연기 atlas 3개와 원본 교차 mesh, 색종이와 별이 포함된다.
조사 초반의 Ray/Dance/Ready 후보는 최종 복원 항목이 아니다. 보스 FBX 본 정렬에 쓰던
`-90° snapshotRootSourceBasisYawDegrees`를 고정 맵 이펙트에 적용하지 않았다.

폭죽은 SCENE03A/interpdata_0의 festival과 interpdata_7의 fireworks를 별도로 읽었다.
festival은 6개 배치·10개 activation, fireworks는 3개 배치다. 원본 Matinee 첫 on을
0초로 옮기고 상대 toggle 시각을 보존했다. 조사한 actor에는 별도 Move/base track이 없으며,
이동은 원본 Cascade velocity/acceleration/orbit와 live LocationEmitter, spawn/death event가
소유한다. 따라서 actor 이동 경로를 임의로 만들지 않았다.

## G02. H/CPP 호출과 앵커 계약

`Effect_Tool.cpp`, `EffectAuthoringSequencer.cpp/.h`는 Play All 시 실제 SceneCharacter의
위치와 scale을 제거한 yaw를 고정한다. 임시 `m_KoukuEffectPreview`가 이 기준을 소유한다.
저장 sequence/model/dirty 상태를 임시 재생으로 교체하지 않는다. Play All은 0초부터
재시작하고 Solo/Play Group은 현재 draft와 source dependency를 사용한다. 실패 이유와
재생 시각을 표시하며 실패한 준비 때문에 기존 preview를 먼저 지우지 않는다.
`EffectAuthoringSequencer_Tracks.cpp`에는 이 임시 상태를 보호하는 작은 guard만 추가했다.
Resources/Timeline 파일의 별도 수정 전체를 이 기능 소유로 보지 않는다.

본이 없는 문서는 저장된 보스 Composition 없이 기존 V1 factory로 재생한다. 실제 본을
요구하는 문서만 원본 CNpc/CModel/animation 공급자를 준비한다. source bone은 기존
inverse(ownerRoot) × previewRoot 경로로 옮기며 임의 anchor로 본을 대체하지 않는다.

`KoukuSaydonActionWorkbench.cpp`, `KoukuSaydonCompositionDocument.cpp`,
`KoukuSaydonPresentationPlayer.cpp`와 Python composition projector는 EFFECT의 MAP 앵커를
함께 지원한다. V1_EFFECT/V1_ELEMENT/GROUP을 기존 Append 경로로 추가하고 MAP 위치·회전·
크기를 occurrence에 저장한다. MAP은 identity root를 사용하고 follow/bone/world reference를
함께 허용하지 않는다. `Use Player Position`은 현재 실제 플레이어 위치를 복사한다.
같은 Workbench를 사용하는 독립 Sequencer Benchmark에도 이 계약이 적용된다.

`Effect_Playback.cpp/.h`, `Effect_DocumentCodec.cpp`는 기존 bounded event queue에
`EPET_Death`를 연결한다. 수명 만료가 프레임 중간에 일어나면 실제 만료 시각의 위치·속도로
후속 emitter를 생성한다. 이벤트 중복 소비를 막고 seek/rewind를 유지한다. 원본 숨은
provider는 같은 문서에 검증된 소비자가 있을 때만 허용한다. orphan/cycle/과도한 reserve는
거절한다. 원본 burst를 늘리지 않고 receiver의 실제 event tail까지 수신 창을 연장했다.

## G03. 재질·셰이더 연결과 재발 방지

폭죽 native ID는 2360~2371, 무지개는 2400~2409다. 기존 Kouku 프로그램과 합쳐 총 69개를
유지했다. `Effect_ArtistMaterial.h`, `Effect_ShaderFamily.h`, `Effect_DocumentRenderer.cpp`,
Artist/Kouku hlsli와 Mesh/Particle/Decal/Trail carrier가 같은 2304~2495 범위를 사용한다.
프로그램 descriptor를 추가해도 HLSL dispatch가 여전히 2367에서 끝나면 이후 프로그램이
0을 출력한다. 실제로 이 누락을 발견해 6개 shader 분기를 함께 수정했다.

원본 PS의 material 상수 외 엔진 입력도 별도로 연결했다. 2360은 `source[2].x`의 opacity
입력이 0이던 문제를 수정했다. 2409는 원본 PS가 읽는 `source[1]`의 색을 실제 particle
color에 연결했다. 이는 원본 PS 데이터 흐름을 근거로 한 엔진 입력 adapter이며, 추출하지
못한 원본 field 이름까지 복원했다는 뜻은 아니다. 원본의 작은 alpha를 임의로 키우지 않았다.

2360 MacroUV는 원본 ParticleSystem의 위치와 반지름 200cm를 source literal로 보존한다.
`EFFECT_EVALUATED_PARTICLE`의 `bSourceMacroUV`, `vSourceMacroUVWorldCenter`,
`fSourceMacroUVWorldRadius`를 통해 현재 occurrence 중심과 world radius 2m를 전달한다.
world-space 입자의 birth 위치와 시스템 중심을 구분하며 고정 ActionRoot도 보존한다.
`Render_Particles`는 실제 View/Projection으로 화면 중심과 반지름을 계산해
`g_ArtistSourceMacroUV`를 바인딩한다. occurrence scale을 world radius에 다시 곱하지 않는다.
불완전하거나 비유한 source literal은 stage에서 거절한다.

재생성 도구는 아래와 같다. native source의 raw 변환 후 엔진 입력 adapter를 다시 적용하도록
builder wrapper에 포함했다. 임시 aggregate만 손으로 고치고 재생성 시 잃는 구조가 아니다.

- `Tools/EffectPipeline/build_kouku_gate1_fireworks_restore.py`
- `Tools/EffectPipeline/build_kouku_gate1_fireworks_native_sources.py`
- `Tools/EffectPipeline/decode_kouku_gate3_rainbow_grid.py`
- `Tools/EffectPipeline/build_kouku_gate3_rainbow_restore.py`
- `Tools/EffectPipeline/build_kouku_gate3_rainbow_native.py`
- 기존 `install_kouku_gate1_native_materials.py`, `install_kouku_gate1_native_shaders.py`

최종 shader 설치 명령은 다음과 같다. 이전 프로그램을 제거하지 않는 append merge다.

```powershell
python Tools/EffectPipeline/install_kouku_gate1_native_shaders.py --source-dir out/KoukuGate1Restore20260911 --append-source-dir out/KoukuSpiderSource20260911/native --append-source-dir out/KoukuFireworks20260911 --append-source-dir out/KoukuRainbowMatched20260911/native
```

Resources의 DDS/WModel은 기존 `Client/Bin/Resources/Effect/KoukuSaydon` 아래 설치했다.
바이너리는 Drive 관리 대상이며 Git에 force-add하지 않았다. 다른 PC 인수 시 코드/JSON만
전달해서 리소스까지 설치됐다고 판단하면 안 된다. 추출 원본·native 계약·수치 증거는
`out/KoukuFireworks20260911`, `out/KoukuRainbowMatched20260911`에 있다.

## G04. 실제 검증 결과

| 검증 | 결과와 증거 |
|---|---|
| 최종 Debug Product | PASS. Engine/Shared/Server/Client compile/deploy. `out/BuildPipeline/runs/20260911T095348586Z-debug-product.json` |
| 최종 빌드 로그 | `out/KoukuPlayerAnchor20260911/product_build_macro_uv.log` |
| 실제 codec + 6문서 CPU 재생 | PASS. 시각 대상 466행 전부 평가, 숨은 provider draw 0, finite, 종료와 rewind 결정성. `document_probe_result.json` |
| death/event/MacroUV 집중 검사 | 19개 PASS. `death_probe_result.json` |
| 최종 CPU 입력 해시 | `out/KoukuPlayerAnchor20260911/playback_final_receipt.json` |
| 최신 CSO의 WARP 수치 draw | 22개 프로그램 69회, shader 오류 0·비유한 값 0. RGB 양수 21개, 2409는 0으로 추가 구분 필요 |
| PE와 DLL 배포 | Engine/Client/Server의 PE 4개 검사와 배포 Engine.dll 해시 일치. `product_pe_validation.json` |
| MAP projector 집중 검사 | `test_map_effect_anchor_keeps_absolute_placement_and_rejects_follow_dependencies` PASS |
| JSON/XML와 diff | 6문서·catalog·project/filter parse 및 등록 검사 수행. `git diff --check` PASS, 기존 줄바꿈 정규화 경고는 남음 |
| Client 화면과 시각적 일치 | 미실행. 사용자 판정 필요 |

CPU 숫자 검사는 yaw 45도, 위치 (17, 3, -9)의 수치 anchor에서 수행했다. 실제 Client의
화면·GPU 표시를 확인했다는 증거로 사용하지 않는다. festival의 실제 2360 행 10개에서
MacroUV center/radius 전달을 확인했고 다른 문서는 이 필드를 비활성으로 유지했다.

초기 빌드에는 renderer의 section 초과 C1128이 있어 해당 파일에만 `/bigobj`를 추가했다.
컴파일 병렬 자원 오류 뒤 최종 빌드는 일시적인 `CL_MPCount=2`로 수행했다. 영구 환경변수는
바꾸지 않았다. 마지막 Product 명령은 다음이며 추가 publisher나 Client 진입을 실행하지 않았다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

넓은 `KoukuSaydonCompositionProjectionTests` 126개 실행은 4 failures/10 errors로 끝났다.
현재 fixture/저작 데이터와 world object, contact, Gate1 목록, Gate2 draft/Product,
fear presentation/attachment 및 boss placement 계약의 불일치가 포함된다. 새 MAP 집중 검사는
통과했지만 이 전체 suite를 PASS로 기록하지 않는다. 다른 세션이 같은 입력과 계약을 수정 중인
상태에서 기존 실패의 발생 시점을 단정하지 않았고 이 작업에서 일괄 수정하지 않았다.

최종 shader 수치 결과는 `out/KoukuFireworks20260911/native_draw_probe_summary_22_final.json`,
전체 draw는 `native_draw_probe_result_22_final.json`이다. 2360은 MacroUV 중심 이동에 따라
출력이 변했다. normalized age 0.25에서 RGB 절댓값 합은 499.247923 → 498.544439,
0.65에서는 8.65348116 → 8.6673536이었다. age 0의 출력 0은 그대로 기록했다.

2409는 정면 사각형을 사용한 독립 수치 draw에서 RGB 0, alpha 대상 4096픽셀이었다.
원본 Fresnel/dissolve의 시선각·mask 때문인지 Dynamic 입력 전달 때문인지는 아직 구분하지
못했다. 따라서 모든 native 프로그램의 표시 성공으로 확대하지 않는다. material uniform의
mesh-emitter Dynamic default와 실제 sprite VF의 Dynamic stream을 비교하는 것이 다음
진단 지점이다. 강제 양수 입력이나 alpha 보정은 하지 않았다. 사용자 종료 요청 뒤에는
이 항목을 추가 수정하지 않았다. 단일 사각형의 0 출력만으로 Client 실패를 확정하지도 않는다.

## G05. 다음 세션과 사용자가 확인할 순서

1. 현재 diff와 이 문서를 먼저 읽고 다른 세션의 변경을 보존한다. 기능 소스는 이미 반영됐으며
   원본 검색과 전체 복원부터 다시 시작할 필요는 없다. 기술 진단을 이어가면 먼저 G04의
   2409 시선각/mask 대 Dynamic 입력 미확인 사항을 좁힌다.
2. 이 PC는 LAN 설정 시 `server-host`였고 endpoint는 `192.168.0.14:7777`이다. 최종 확인 시
   Client/Server는 실행 중이지 않았다. 사용자가 VS의 Server + Client profile을 Ctrl+F5로 실행한다.
3. 실제 플레이어가 있는 아레나에서 F1 → Effect Tool V1 → All Effects → KoukuSaydon →
   `effect.kouku.gate3.rainbow.grid.full.restore` → Play All을 누른다. 첫 경고까지 원본 지연이
   있으므로 클릭 즉시 0프레임만 보고 실패로 판단하지 않는다. Open Editor의 Solo/Play Group도
   같은 실제 플레이어 기준으로 확인한다.
4. `intro.fireworks`와 `intro.festival`은 서로 다른 연출이므로 각각 Play All을 확인한다.
5. Action Workbench 또는 Open Sequencer Benchmark에서 대상 Pattern/Sequence를 선택하고
   Effect V1의 전체 문서를 `Append Effect at Cursor`로 추가한다. 추가한 box의 Anchor를
   MAP으로 정하고 `Use Player Position` 또는 Map position/회전/크기를 설정해 Apply/저장 후
   Play한다. 선택한 패턴/시퀀스 길이가 문서 tail을 자르지 않는지도 확인한다.
6. 남은 최종 판정은 실제 화면의 무지개 격자·연기·불꽃·폭죽 이동, 원본 영상과의 크기·색·
   시간 차이다. 사용자가 관찰한 결과를 기준으로 occurrence 또는 원본 입력을 좁혀 수정한다.

이 세션은 다른 사용자 작업에 메시지를 보내지 않았다. 사용자가 이 결과 문서를 직접 전달한다.

## G06. 2026-09-13 festival 재생 거절 진단과 scene player 연결

사용자가 전달한 정확한 문구는 `Enter an arena with a scene player before Play All. The player is the Effect anchor.`다.
실제 발생 위치는 `CEffectAuthoringSequencer::Select_SceneEffectTarget`의 scene player/Transform 조회다.
`Resolve_SceneCharacter()`는 `CAnimationTargetService::s_Target`만 읽지만, 조사 시 Bind 호출은
Character Select Level에만 있었다. Kouku 아레나의 복제 player는 카메라·입력에 정상 연결돼도
이 전역 scene target에는 등록되지 않았다. 이 코드 경로는 HEAD에도 있어 최근 Effect Tool 분할에서
처음 생긴 결함으로 단정하지 않는다. reported 문구는 V1 factory·입자 평가·shader 실행 이전의 거절이다.

### 실제 변경

`Client/Private/ClientReplication.cpp`에서 local player의 실제 commit과 scene target 수명을 연결했다.
local spawn과 class replacement 성공 뒤 Bind하며, 실패 rollback·remote player는 기존 target을 보존한다.
local despawn 성공·Reset_World·destructor는 해당 local character만 Unbind한다. destructor가 Layer를
다시 조작하지 않는 기존 종료 계약과 Character Select의 안전한 exact-pointer 호출은 보존한다.
새 C++ 파일·별도 runtime·project 항목은 없다. 기존 클릭 이동 예측 교정도 보존한다.

### 데이터·셰이더·위치의 구분

| 확인 대상 | 현재 근거 |
|---|---|
| original festival | `effect.kouku.gate1.intro.festival.full.restore`, 첫 발생 0초, CPU 전체 tail 24.2005초 |
| authored festival | `effect.kouku.gate1.authored.festival`, 첫 발생 33.408970초, 후반 55.606958/55.608716초, CPU tail 63.1087초 |
| 두 문서의 차이 | asset ID와 80개 startDelay. spatial/attachment/material/source modules는 동일. 09-12 실제 codec 출력과 현재 authored의 의미 차이는 root bloomIntensity 1.3 추가뿐 |
| 입자 구성 | 각 80행 중 시각 행 70개와 simulation-only provider 10개. live-location 참조 30개 모두 문서 내 해소 |
| source attachment | 모두 enabled=true/follow=false, source/runtime slot root, bone 빈값, SourceModelPreview/sourceTransformTrack 없음 |
| 현재 저장 시퀀스 | Sequence Composition의 P4.presentation.22 → kakulsaydon.g1.presentation.51 → authored festival. MAP, follow=false, 위치 `(13.990144,1.040076,735.373359)`, 시간 0~58,810ms |
| 발사 원점 | 6개 원점의 범위 X -6.990~13.990, Y 1.040~1.476, Z 723.398~744.179. 입자 전체 궤적 범위와 구분 |
| 물리 리소스 | DDS 12개와 WModel 1개 존재·헤더 정상. 문서 material과 현재 native descriptor의 shape/profile/texture/scalar/vector/switch 일치 |
| 사용 shader | native 2360~2365, mesh20/sprite50. 현재 Kouku2304 carrier dispatch와 2360 opacity/MacroUV adapter 유지. 직접 관련 소스보다 최신인 09-13 Debug CSO 2개와 필수 binding 이름 확인 |

과거 독립 shader 검사에서 RGB 0이던 2409는 festival에서 사용하지 않는다. 위 정적 검사는 현재 GPU 화면
성공의 증거가 아니다. 이번 실패의 persisted render 로그는 없으며 사용자 문구와 실제 조건을 대조했다.
시퀀스의 발생 지연이나 MAP 좌표를 바꾸지 않았다. resource 기본 anchor WORLD와 현재 실제 occurrence MAP은
별개이며, 기존 P4 재생은 실제 occurrence를 소비한다. 본 등록 오류를 고치려고 이를 임의 변경하지 않았다.

### 실행한 검사와 남은 단계

현재 제품 OBJ와 실제 codec/playback을 사용하는 기존 CPU probe를 `out/FestivalEffect20260913`에서
다시 빌드했다. 최근 codec/material CPP 분할에 맞춰 실제 분할 OBJ와 Client_Pch를 연결했다.
과거 probe EXE를 현재 코드 검증으로 재사용하지 않았다.

- original festival: 시각 행 70개, peak particle 1,356, MacroUV 10행, finite·종료·rewind 결정성 PASS.
- authored festival: 시각 행 70개, peak particle 1,356, MacroUV 10행, 같은 검사 PASS.
- 짧은 fireworks: 시각 행 36개, peak particle 1,607, 같은 검사 PASS.
- 위 고정 pose 검사와 별개로 실제 저장 P4 MAP root를 넣어 세계 위치·finite·재생을 다시 검사했다.
  모델·화면·GPU·UI를 실행한 결과가 아니다. `.run.log`와 `.map.run.log`를 구분한다.

scene target lifecycle 실제 함수 추출 검사 28개 PASS. local/remote spawn, class 교체 성공·실패,
despawn, reset/destructor의 exact ownership과 빈 registry의 generation 보존을 확인했다.
`out/ScenePlayerAnchorLifecycle20260913/validation.json`은 최종 source SHA와 baseline 대비
6개 lifecycle 위치 외의 bytes 보존, UTF-8/CRLF 보존과 격리 컴파일 exit 0을 기록한다.

09-13 00:42 Debug Product 빌드 PASS. Engine → Shared → Server → Client의 현재 dependency 빌드를
수행했고 ClientReplication OBJ 1개와 Client EXE를 새로 생성했다. 기존 클릭 이동·카메라 교정도 현재
소스에 유지되며 같은 제품 빌드 범위다. 기존 인코딩 경고는 남아 있지만 컴파일·링크 오류는 없다.
`out/FestivalEffect20260913/product-build.log`,
`out/BuildPipeline/runs/20260912T154243343Z-debug-product.json`이 근거다.
CPU 문서 검사 6회와 입력 SHA는 `out/FestivalEffect20260913/cpu-receipt.json`이다.

빌드 직전 Client/Server와 다른 MSBuild가 모두 종료된 것을 확인했다. 에이전트는 제품을 실행하지 않았다.
사용자는 `Server + Client` profile → Ctrl+F5 → KoukuSaydon 아레나 → F1 Effect Tool의 해당
`intro.festival`/`intro.fireworks` → Play All에서 확인한다. 저장된 MAP 시퀀스는 원래 시각을 소비한다.
Client/UI 실행과 최종 폭죽 화면 판정은 사용자에게 남아 있다.

### 첨부 이미지로 정정한 대상과 현재 시퀀스 연결 범위

사용자가 추가한 천막·풍선·오른쪽 관람차 이미지의 대상은 여러 색의 방사형 폭죽이다.
앞서 이를 festival로 통칭한 설명은 정정한다. 해당 원본 계열은
`effect.kouku.gate1.intro.fireworks.full.restore`이며, `intro.festival.full.restore`의
`Par_Q_FestiParticle_01`과는 별도다. SCENE03A Matinee7의 `Par_Q_Fireworks_01_Loop` /
`Par_Q_Fireworks_02_02`는 방사형 spawn·velocity와 death/event chain, 따뜻한 색 `(1.3,1,0.7)`과
푸른색 `(0.2,0.5,3)` 입력을 사용한다. 같은 원본 연출은 tent01a/curtain01a와 circuspopup sound를
연결한다. 이미지와 원본 연출의 대응 근거이며 현재 Client 화면의 색·크기 일치 판정은 아니다.

이 fireworks의 시각 행은 sprite 31개·mesh 5개다. DDS 11개와 WModel 1개가 설치돼 있고,
native 2364~2371의 descriptor·dispatch·필수 CSO 연결을 확인했다. 앞의 scene player 수정과
최종 Debug 빌드는 이 문서의 독립 Play All에도 적용된다. CPU tail은 12.2385초이며 첫 발생은
0초, 후속 두 발사는 내부 delay 1.992975235초다.

현재 `KoukuSaydonSequenceComposition.json`에는 이 fireworks를 사용하는 resource/occurrence가 없다.
P4에 연결된 것은 authored festival이다. P1 팝업북은 `original_8T6_00~04`·`2Stage.book`을 사용하며
책 무대 Z는 약 737m다. Matinee7 폭죽 원점은 Z 약 -100m이고, 원본 tent01a/curtain01a도 현재 P1에
없다. 두 배치 사이의 actor 대응·공통 변환을 확인하지 못했으므로 원본 좌표를 P1에 임의 추가하지 않았다.
독립 Play All의 생성 전 거절 수정과 저장 시퀀스의 누락 연결은 별개의 완료 범위다.

원본 Matinee7 자체를 배치할 때의 MAP root는 `(70.051201171875,4.18999755859375,-105.66234375)`,
rotation `(0,0,0)`, scale `(1,1,1)`이다. 원본 최초 ON은 5.9800128937초이며 occurrence로 표현하면
startMs 5980, 전체 tail을 포함한 durationMs 12239다. 내부 element가 이미 yaw 45도와 1/0.8배 크기를
소유하므로 root에 중복 적용하지 않는다. 이 값은 P1의 시간·좌표 정렬값이 아니다. 이번 변경에서는
시퀀스 JSON·폭죽 문서·shader·Resources를 수정하지 않았다.

## G07. 무대 앞 금빛 이동 축포의 원본 곡선·방출 복구

사용자가 새로 첨부한 이미지의 금빛 선단·곡선 잔광은 SCENE03A Matinee0이 사용하는
`FX_Q_W_01.FX_Par_02.Par_Q_Trail_01`이다. `intro.fireworks`의 방사형 폭죽 및 festival 축포와
별도로 움직이는 원본 actor를 사용한다. source emitter는 6개이며, mesh 입자 2개·sprite 3개와
기존 cascadeRibbonV1 1개다. DDS 12개·`fm_e_plan_001.wmodel`은 이미 설치돼 있었다.
native 2461/3109/3334/2457/2426의 재질·shape·필수 texture·shader dispatch 계약을 대조했고,
새 shader·texture·model payload는 추가하지 않았다.

### 실제 원인과 반영

- `build_kouku_sequence_effect_groups.actor_groups`가 원본 `Pc01tr`와 `pc01tr`를 대소문자로
  구분해 실제 Move track을 놓쳤다. UE 이름의 대소문자 비구분 연결로 수정하고, 해당 occurrence의
  actor/group 불일치와 모호한 중복 이름을 거부한다. 무트랙 부모는 기존대로 허용한다.
- source `.matinee_0.1/.2`와 대응 `authored.portal-arrival.1/.2`의 누락된 node 곡선 72개를
  보완했다. 서로 다른 원본 actor는 6개이며 source와 authored의 같은 데이터가 반복된다.
  기존에 정상이던 portal actor21/22의 곡선은 그대로다. 각 source key의 시각·값·접선을 보존했다.
- 6 Required 인스턴스와 `Engine.Default__ParticleModuleRequired → Default__ParticleModule →
  Core.Default__Object`의 원본 체인은 duration 1초, loops 0을 사용한다. 임시 loop 1이 효과를
  1초 방출로 잘랐으므로 선택한 trail recipe 96행을 0으로 교정했다. Matinee의 기존 ON/OFF가
  방출 종료를 소유하며 각 행의 KillOnDeactivate/KillOnCompleted는 바꾸지 않았다.
- `Effect_Playback.h/.cpp`의 기존 ELEMENT_STATE와 prepared 목록에 이동 속도 이력을 연결했다.
  SourceTransformTrack과 VelocityInheritParent를 함께 쓰는 요소만 60Hz로 원점 속도를 계산한다.
  독립 24행 중 4행만 추가 평가하며, 태어난 입자의 simulation basis로 역변환한 뒤 원본 scale을
  한 번 적용한다. source track 없는 root/world/local/bone 경로는 기존 계산을 유지한다.
- 원본 mesh sparkle은 30cm, smoke tail은 20cm 이동마다 생성되는 SpawnPerUnit이다.
  정지 root에서는 생성 거리가 0이므로, 빠진 Move는 위치뿐 아니라 입자 밀도와 꼬리까지 없앴다.

기존 4문서에서는 위 node와 선택한 SourceRecipe.emitterLoopCount 외의 byte를 유지했다.
저작 시작 시각·sourceTimeOrigin 보정·밝기·재질·MAP 위치·기존 저장 pattern/occurrence는 보존했다.
생성 도구는 `Tools/EffectPipeline/build_kouku_gold_trails_restore.py`이며 원본 stage placement와
기존 source 조직·CDO 입력으로 동일 결과를 다시 만든다. 새 C++ 파일은 없다.

### 실제 연결과 독립 재생

새 문서는 `effect.kouku.gate1.intro.gold-trails.full.restore`이며 표시명은
`1관문_금빛 이동 축포_무대 4경로`다. EffectCatalog·EffectResourceTree와 Boss/Sequence Composition의
추가 가능한 Effect resource에 등록했고, Client project/filter의 96.DataFiles None 항목에 연결했다.
resource ID는 `kakulsaydon.effect.8e7eaa218533fd414f00`이다. 기존 시퀀스에 동일 occurrence를
중복 추가하지 않았다. P4의 기존 `.presentation.20/.21`이 교정된 authored 문서를 그대로 소비한다.

| 독립 문서 | 원본을 보존한 값 |
|---|---|
| 구성 | 무대 actor15/16/17/18 × 6 emitter = 24행 |
| 시작 지연 | 0 / 0.640884399 / 0.774541855 / 2.738483429초 |
| source clock origin | 23.076379776초 |
| 위치 기준 | SL04 floor08 원본 placement `LV_LUT_MIDNIGHTC_ED_SL04:export:206` |
| preview origin | UE `[0,-73728,0]` → Client `[0,0,737.28]` |
| 회전·크기 | 원본 node WORLD·scale 2 유지. 바닥 mesh의 yaw45/scale은 effect root에 복사하지 않음 |
| CPU 전체 수명 | 9.41295초, 마지막 시각 요소 관측 9.26667초 |

기존 P4의 MAP offset `[58.0160107422,0.7505390167,-86.8656152344]`는 해당 authored 문서의
기존 preview origin 역변환과 일치한다. 독립 문서의 새 floor 기준을 P4에 덮지 않는다.
시퀀스에서는 무대의 네 경로가 기존 저작 시각 약 34.023/34.726/34.873/37.028초에 시작한다.

### 검증과 남은 화면 판정

| 검증 | 실제 결과 |
|---|---|
| 원본 그룹/곡선 | 480개 PASS. 173 occurrence 연결, 선택 8actor의 위치·회전 72키와 시간·접선 보존, 잘못된 연결 거부 |
| 5문서 실제 codec/playback | PASS. 독립 24/24, authored1 150/150, authored2 24/24, source1 150/150, source2 24/24 요소 관측 |
| 방출·잔광 | 이동 경로와 SpawnPerUnit 생성 확인. world sprite23·ribbon20의 이미 생성된 위치 이력 drift 0 |
| 수명·재시작 | 5문서 finite·종료·rewind 결정성 PASS |
| 속도 상속 | 9경계 PASS. sourceTrack world/local/root/bone 속도 최대 오차 0.00002312m/s, source track 없는 3경로는 이전 CPP와 signature/속도 동일 |
| C++ | 두 파일 UTF-8/CRLF 유지, 격리 번역 단위 compile/link PASS |
| Debug Product | 09-13 01:12 PASS. 현재 Engine→Shared→Server→Client dependency compile/deploy, Client OBJ 61개·EXE 갱신 |
| 설치·재생 입력 일치 | CPU 후보 5문서와 실제 Data의 SHA 일치, 최종 C++ SHA 일치, Catalog/Tree 중복 없음, JSON/XML·project/filter parse PASS |
| 재생성·diff | 설치 뒤 builder 재실행 시 추가 교정 0·동일 standalone 출력. 관련 git diff --check PASS, 기존 줄바꿈 경고만 남음 |
| Client 화면 | 에이전트 미실행. 사용자 최종 색상·크기·움직임 판정 필요 |

원본 actor15/16/17에는 26.9194545746초에 CONSTANT 키 뒤 위치 도약이 있다. 같은 원본 Matinee의
DirectorTrack1263 shot40도 정확히 같은 float 시각에 cm03→cm01로 transitiontime 0인 camera cut을
수행한다. 이 위치 도약을 임의 평활·속도 clamp로 바꾸지 않았다. 독립 Play All은 카메라 연출을
재생하지 않으므로 해당 경계가 더 잘 드러날 수 있다. 현재 60Hz 차분으로 계산한 최대 상속 속도는
106.269m/s이며 원본 PSC의 내부 velocity reset 정책까지 확인한 증거는 아니다.
같은 시각 Toggle 재시작이나 actor reset은 없으며, 직전 SlomoTrack1641에는 0.1배 감속 후
26.884880초에 1배로 돌아오는 키가 있다. 독립 문서는 이 camera/Slomo 문맥까지 포함하는 시퀀스가 아니다.

증거는 `out/KoukuGoldTrails20260913/installation.json`, `registration.json`,
`validation/validation_receipt.json`, `product-build.log`와
`out/BuildPipeline/runs/20260912T161231613Z-debug-product.json`이다. source actor 검사는
`out/KoukuSequenceEffects20260912/trail-motion-review/validation-summary.json`에 있다.
별도 out의 격리 probe에서 발생한 이전 header/OBJ 혼용은 비교용 위치를 분리해 해결했으며,
제품 CPP나 authored 입력의 실패로 기록하지 않는다.
최종 설치 일치 검사는 `final-validation.json`, 기존 3개 경로의 float bit 해시 보존은
`validation/legacy_preservation.json`에 기록했다. 기존 뼈 경계 검사는 수치 anchor 계약이며
실제 모델 부착·GPU 표시 확인으로 확대하지 않는다. 최종 확인 시 Client/Server는 종료 상태였다.

사용자는 최신 Client에서 아레나 진입 후 F1 → Effect Tool V1 → All Effects → KoukuSaydon →
1관문 → 연출 → 금빛 이동 축포 → `1관문_금빛 이동 축포_무대 4경로` → Play All로 확인한다.
기존 시퀀스의 통합 팝업북에서도 저장된 시각·MAP 위치로 재생된다. 에이전트는 Client/Server를
자율 실행하거나 UI를 조작하지 않았다.
