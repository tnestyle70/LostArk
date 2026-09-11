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
