# 쿠크 도넛 확장·공 분열·손 궤적·작은 오망성 결과

## G00. 반영 상태

2026-09-13, `codex/kouku-donut-ball-motion`에서 Effect Tool V1용 Authored 문서 12개를 추가하고 기존 `작은 오망성` 문서 1개를 교체했다. Catalog, ResourceTree, Composition의 전역 presentationResources 및 Client project의 `96.DataFiles` None 등록을 연결했다. 새 C++·HLSL·Resources binary는 없으며 기존 document, playback, material, model 경로를 사용한다.

소스 반영과 아래 수치 검증은 완료했다. Client 화면의 색·두께·속도 및 최종 유사도는 사용자 확인 전이다. 첨부한 네 이미지는 진단 입력으로 열람했으며 자동 visual PASS 증거로 사용하지 않았다.

이 데이터 설치 이후 추가된 고정·추적 조준점 문의에서는 native2484의 blend 결함을 별도로 확인해 shader dispatch를 수정했다. 이 문서의 데이터 설치 완료와 shader의 Product 적용 상태는 구분하며, 추가 수정은 [쇼타임 조준점 결과](2026-09-13_KOUKU_SHOWTIME_TARGET_BLEND_RESULT.md)를 따른다.

계획: [구현 계획](2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_IMPLEMENTATION_PLAN.md).

## G01. 작은 오망성

기존 `effect.kouku.gate3.mario.boss.pentagram.full.restore`의 이름과 asset ID를 유지했다. 처음 별을 그리는 8개 요소의 직렬화 블록은 동일하게 보존하고, 나중에 추가했던 회색 floor 5개(`kouku.mario.small.pentagram.floor.child.authored.00016`~`00020`)를 제거했다. 원본 `star_shot` 7개를 1.536231995초에 연결해 총 15개 요소로 별 그리기 이후 폭발을 재생한다.

원본 Action4219911에서 `star_cast` notify002와 `star_shot` notify008은 disabled다. 따라서 이번 연결은 사용자가 요청한 독립 이펙트 조합이며, 원본 emitter·material·notify 시간을 재사용한 것이다. 원본 action의 활성 cue를 그대로 복원했다고 기록하지 않는다. 다른 패턴의 번개나 임의 완성형 별 장판은 추가하지 않았다.

`build_kouku_gate3_slam_mario_restore.py`도 같은 star-shot helper를 사용하도록 수정해 재생성 시 회색 floor가 돌아오지 않게 했다. 재생성 결과와 설치 후보의 구조 일치, 보존된 drawing 8개 블록 hash 일치를 확인했다.

기존 Composition resource의 기본 길이 3000ms는 실행 중 draft의 append merge를 보존하기 위해 유지했다. 전체 source window는 3737ms다. 해당 리소스를 Effect Box에 배치해 전체 꼬리까지 보려면 box 길이를 **3.74초 이상**으로 설정한다. 설치 당시 이 asset을 사용하는 기존 pattern occurrence는 없었다.

## G02. 제물 의식의 손 궤적

`effect.kouku.gate3.ritual.hand.trail.full.restore`, 표시 이름 `제물 의식_손 궤적`을 추가했다. Action4219911/4219932의 활성 notify001, `Par_V_RPCT_HandSwing_Trail_01_LOC_INT`의 Cascade Ribbon 2개와 별 sprite 1개를 사용한다. AnimationTrail로 대체하지 않았다.

원본 `startcontrol → b_wp_1`, clip `rpct00_att_battle_27_01`, 설치된 `MN_RPCT_05.wmodel`의 CModel preScale 0.017을 사용했다. 미직렬화 Required CDO의 EmitterLoops=0을 복구해 0~3.174864초 notify 동안 입자가 계속 방출되고 이전 손 위치의 Ribbon 이력이 남게 했다. 원본 material 2836/3007/2428 및 기존 texture 8개를 재사용했다.

실제 설치 CModel의 pose와 production source-anchor 경로로 251회 CPU 샘플링했다. anchor 1개, peak CPU rows 27, 두 Ribbon의 최대 point 수 20/29, 폭 1.36/0.338982m, 최대 alpha 1, 마지막 trail 시각 3.25/3.56667초를 확인했다. 카메라 basis는 명시적 수치 fixture이며 화면 두께·길이·유사도의 최종 판정은 하지 않았다.

## G03. 도넛과 노란 예고

실제 발탄 `effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01`은 sprite의 Transform.scale 17→41.3과 Linear Lerp로 커진다. 쿠크 native3601의 inner/thickness는 반경 마스크 입력이고 현재 shader 시간 입력은 caustic UV와 pulse에 쓰인다. 원본 엔진이 inner를 시간에 따라 증가시키는 호출 근거는 찾지 못했다.

기존 칼날댄스·쇼타임 도넛은 보존하고 `바깥확장(크기보간)`이라는 별도 6종을 추가했다. 확장본은 `PROJECT_AUTHORED_SCALE_LERP`다. 시작 XZ 배율은 내경/외경, 끝은 1이며 Y 배율 1로 projector 깊이 6m를 유지한다. 원본 SourceRecipe·native material·fade는 유지한다.

쓰리투원투하의 노란 원형 1개와 도넛 2개도 추가했다. Projectile421980613의 Timer 2초와 GroundEffect 3개, SkillDecal2112/2116, Effect421980629/630/631을 근거로 원본 재질과 범위에 맞췄다. 독립 warning의 시작 시각은 편집용 0초다.

| 계열 | 도넛 1 최종 내경/외경 반경 | 도넛 2 최종 내경/외경 반경 | 확장 시간 |
|---|---|---|---|
| 칼날댄스 | 3 / 4.5m | 4.5 / 6m | 약 0.433초 |
| 쇼타임 | 4 / 8m | 8 / 12m | 약 1.300초 |
| 쓰리투원투하 | 2.5 / 4m | 5.5 / 7m | 약 1.733초 |

쓰리투원투하의 원형 반경은 1m다. Detail Life를 root scale 보간 시간으로 사용해 원본 fade-out 전에 확장을 끝낸다. MeshParticle 전용 `transformMotionDurationSeconds`는 LocalDecal codec에서 거부되므로 추가하지 않았다. shader나 공통 runtime의 지원 범위를 넓혀 우회하지 않았다.

수정 경로는 `Effect Detail → Transform → Scaling`, `Advanced Authoring → Linear Lerp → Lerp Scaling / Scaling End`, `Timing → Life Time`이다. 이 source decal에서 Life Time은 root 보간 시간이고 원본 입자 수명·fade는 SourceRecipe/SourceTransformTrack이 소유한다. `Source Playback Tuning → Size x`는 고정 배율이다.

## G04. 공 낙하·분열

`effect.kouku.common.circus.ball.drop.split`, 표시 이름 `세이튼_서커스공_낙하분열(편집용)`을 추가했다. 원본 Action4219806 → Projectile421980602 → 603~607의 종료 callback에 2자식 연결이 있어 원본 공 mesh/Cascade와 6세대, 총 63개 발생을 재사용했다. 새로운 Server 분열 경로를 추가하지 않았다.

원본 세대 배율 2/1.7/1.4/1.1/0.8/0.5, 낙하·반동·회전 곡선, 속도 15m/s 및 세대별 거리 2.5/2.75/3/3.25/3.5/4m를 사용한다. 최대 수명 1.5초를 세대 교체 시각으로 선택하고 고정 seed 방향을 적용한 것은 편집 정책이다. 원본의 거리 도달과 수명 종료 중 어느 이벤트가 우선하는지, 실제 target 선택·gameplay 판정은 이번 범위에서 복원하지 않았다.

원본 instance의 `StartSize`, `LocationDirect.ScaleFactor`에 `Distribution=0`만 저장된 경우 CDO의 lookup=1을 상속해야 한다. 파생 후보의 두 분포에 누락된 CDO 값만 채워 크기 0.2 fallback과 낙하/반동 곡선의 0배 소거를 교정했다. 기존 raw leaf와 공통 runtime은 유지했다. AlphaOverLife/RateScale의 null 분포는 기존 runtime이 이미 1로 평가하므로 수정하지 않았다. disabled LocationDirect 3개(큰 공), disabled Lifetime 1개(작은 공)는 운영 후보에서 제외하고 원본 증거에 남겼다.

실제 첫 부모와 두 자식의 CPU 366개 샘플에서 source scale 2/1.7의 1회 적용, 부모 worldY 3.4~17m, 자식 3.34~5.51866m, 1.5/3초 종료 후 alpha 0을 확인했다(root translation 17,3,-9). 모든 63개 발생과 반복 seek는 native probe로 확인했다. 각 세대의 가시 종료를 개별 CSV로 기록한 범위는 첫 두 세대다. 편집용 시퀀스는 9초이며 계산된 playback duration 13초에는 투명 입자 꼬리가 포함된다.

## G05. 백스텝 후 세 갈래 감전

`effect.kouku.gate3.backstep.electric.threeway.authored`, 표시 이름 `백스텝_세갈래전격(편집용배치)`를 추가했다. Action4219962의 1.3초 Effect319/321/323 세 발이 enabled이고 Effect313/315 두 발은 disabled임을 raw header로 확인했다. Projectile421990316의 원본 6 emitter를 세 번 배치해 18개 요소로 만들었다.

원본 material 2786/2767/3166/2990, 크기 1, 속도 50m/s, 거리 75m를 재사용한다. 독립 재생의 시작 0초, -30/0/+30도 직선 fan은 `PROJECT_AUTHORED`다. 원본 Grenade arc와 target-origin은 미해독이므로 원작 발사 궤적 전체 복원으로 기록하지 않는다. runtime trail capacity는 0.5초·60Hz에 맞춘 32를 사용하고 raw module의 500은 원본 증거에 보존했다.

기존 P46은4219921 단독 backstep이며 발사 occurrence가 없다. 사용자의 기존 animation과 lane를 변경하지 않고 독립 전격 리소스만 추가했다. backstep animation, 표적 선택, impact 및 Server gameplay 자동 연결은 이번 추가물에 포함되지 않는다.

## G06. 설치와 기존 편집 보존

설치 입력은 `out/KoukuPatternMotion20260913/installation_manifest.json`, 설치 시 보존 증거는 `installation_preservation.json`, 기존 파일 백업은 같은 폴더의 `BeforeInstall/`이다. 대상 Authored 13개 hash를 기록했다. 기존 Catalog/Tree 항목은 유지하고 12개를 추가했다. 새 JSON 12개는 Client.vcxproj와 filters에만 None으로 등록했다.

실행 중 사용자가 편집한 Composition을 설치 직전 기준본으로 삼았다. 설치 시 revision443→444, presentationResources151→163이며 기존 48개 pattern의 모든 lane와 기존151개 resource 내용이 동일했다. 기존 작은 오망성 resource의 3000ms도 바꾸지 않아 live draft의 append merge 조건을 유지했다.

설치 후 읽기 전용 검사에서는 사용자 편집이 계속되어 revision447, resources164가 관찰됐다. 기존151개 resource와 추가12개는 모두 유지됐고, 별도 `Rainbow Grid` resource와 P38 lane 변경이 발생했다. 이를 사용자 사후 편집으로 보존했다. 설치 당시 보존 결과와 사후 worktree 상태를 같은 것으로 기록하지 않는다. Git HEAD와의 Composition diff에는 작업 중 사용자 변경이 섞여 있으므로 파일 전체를 작업 커밋에 임의로 넣지 않았다.

## G07. 실행한 검증

| 검증 | 결과와 범위 | 증거 |
|---|---|---|
| Python syntax | 신규 4개 및 기존 생성기 1개 통과 | `python -m py_compile` |
| 현재 C++ CPU probe 컴파일·링크 | production codec/playback 등의 현재 소스를 격리 out에 빌드. 기존 C4828 및 DirectXTK PDB 경고만 존재 | `out/KoukuPatternMotion20260913/CPU/compile.log`, `link.log` |
| 원형·도넛·전격·작은 별 11개 | Load, drawable validation, SaveAtomic/Load roundtrip, 60Hz CPU, 모든 occurrence, 반복 seek 통과 | `out/KoukuPatternMotion20260913/candidate_validation.json` |
| 확장6개 수치 | root yaw45도·translation17,3,-9에서 중심 고정, Y깊이6m, 기대 XZ 증가 통과 | 같은 폴더 `expansion_samples.csv` |
| 공63개 | native load/roundtrip/playback/seek 및 첫 두 세대 위치·scale·alpha 통과 | `out/KoukuCircusBalls20260913/native/validation.json`, `numeric_checks.json`, `particle_samples.csv` |
| 손 본 부착 | 실제 설치 CModel/clip의 production CPU source-anchor 검사 통과 | `out/KoukuRitualHandTrail20260913/CPU/hand_actual_model.json` |
| 작은 별 보존·재생성 | drawing8 byte block 동일, gray5 제거, shot7 추가, 재생성 구조 동일 | `out/KoukuRitualHandTrail20260913/drawing_preservation.json`, `regeneration_validation.json` |
| native resource staging13개 | 기존 CEffectObject WARP probe로 성공. window/swapchain/draw 없음 | `out/KoukuPatternMotion20260913/resource_stage_validation.json`, `ball_resource_validation.json` |
| 설치·등록 보존 | Authored hash, JSON/XML, Catalog/Tree/Composition 기존 항목과 추가12개 검사 | `out/KoukuPatternMotion20260913/final_registration_review.json` |
| whitespace | `git diff --check` 통과. Git CRLF 변환 안내만 존재 | 작업 종료 검사 |

Resource staging은 기존 native CSO를 사용한 자원 생성 검사이며 GPU 표시나 visual fidelity 검증이 아니다. 위 데이터 설치 단계에서는 제품 C++/shader 변경이 없어 Client·Server EXE, DLL, CSO를 재빌드하거나 교체하지 않았다. 이후 조준점 추가 수정의 빌드 상태는 별도 결과를 따른다. out의 probe와 진단 산출물은 소스 변경에 포함하지 않는다.

## G08. 사용자가 확인할 경로

최종 프로세스 조회 시 Server PID48036은 `Server/Bin/Debug/Server.exe --bind-address 0.0.0.0`, Client PID45900은 `Client/Bin/Debug/Client.exe`로 실행 중이었다. 에이전트는 이들을 실행·종료·조작하거나 화면을 캡처하지 않았다. Server CMD 창의 가시 상태는 확인하지 않았다.

현재 화면의 미저장 편집을 보존한 뒤 **F1 → Effect Tool V1 → All Effects → Refresh → KoukuSaydon**에서 아래 이름을 선택해 `Open Editor`와 `Play All`로 확인한다. 이미 열려 있던 작은 오망성 draft는 목록 Refresh만으로 디스크 변경이 자동 반영된다고 가정하지 말고, 편집을 보존한 뒤 저장된 Effect를 다시 연다.

| 찾을 이름 | 분류 |
|---|---|
| `작은 오망성` | 3관문 / 패턴 / 세이튼 / 마리오 |
| `제물 의식_손 궤적` | 3관문 / 패턴 / 세이튼 / 제물 의식 |
| `쓰리투원투하_원형_예고`, `쓰리투원투하_도넛1/2_예고` | 1관문 / 패턴 / 세이튼 / 쓰리투원투하 / 원형·도넛 |
| `바깥확장(크기보간)` 포함6개 | 각 칼날댄스·쇼타임·쓰리투원투하 / 원형·도넛 |
| `세이튼_서커스공_낙하분열(편집용)` | 1관문 / 패턴 / 세이튼 / 세이튼_서커스공 날리기 |
| `백스텝_세갈래전격(편집용배치)` | 3관문 / 패턴 / 세이튼 / 쿠크세이튼_벡스텝후감전빔 |

작은 별의 회색 도형 제거와 폭발 연결, 손 궤적의 실제 위치, 도넛의 확장 크기·속도, 공의 낙하·분열은 사용자가 직접 화면에서 판정한다. 이 문서는 사용자 visual PASS를 선언하지 않는다.

## G09. 작은 오망성 폭발이 투명한 왜곡만 남는 문제

사용자가 마지막 수정 범위를 Play All 차단과 마리오의 작은 오망성 폭발로 제한했다. 원본 데이터
누락은 발견하지 않았다. cooked source의 star_shot7개와 첫 LOD 모듈66개 순서가 현재 SourceRecipe와
일치하고, material7개도 source leaf와 동일하다. drawing8과 참조 mesh/DDS11개는 보존돼 있다.
실제 CPU에서도 shot7개가 모두 발생하며 native2811의 mesh5개와2812 sprite에 양의 색·alpha가 있다.

native2811/2812 원본은 blend_additive이고, source PS가 opacity를 이미 RGB에 곱한 뒤 A=0을
출력한다. distortion 동반 dispatch가 A=0을 공통 SrcAlpha/One carrier에 그대로 보내 RGB가
최종 합성에서0이 됐다. 별도의2310 왜곡만 남는 사용자 증상과 연결되는 실제 합성 결함이다.

`Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli`에서2811/2812의 SceneColor alpha
전달값만1로 고쳤다. 원본 RGB 수식·fade·별을 그리는8개 요소·왜곡 출력·Data·트리는 바꾸지 않았다.
동일 source program을 사용하는8문서40개 요소도 같은 additive 계약을 사용한다. 재생성기는 이전
G06에서 source nativeBlend에 따른 동일 처리를 이미 사용하므로 전체 shader 재생성을 하지 않았다.

두 carrier의 실제 Debug FXC 컴파일과 두 줄 외 원문 불변, 현재 Effect/Catalog/Tree/Boss/Sequence
SHA 보존 및 JSON/XML parse를 통과했다. 근거는 `out/KoukuSmallPentagramBurst20260913`의
`shader-build.json`, `source-change.json`, `scope-check.json`과
`out/KoukuSmallPentagramSourceAudit20260913/source_audit.json`이다.

실제 Codec/Playback의 Color/Dynamic, material parameter32행과 원본 DDS를 넣은38개 수치 입력을
기존 Product CSO, 기존 CSO의 RT0 One/One 대조, 수정 CSO로 각각 검사했다. 오류와 NaN은0이다.
실제2811 mesh5개·2812 sprite의 모든 샘플은 RGB0에서 양수로 바뀌고 One/One 대조와 RGB hash가
정확히 같다.1.6초 fixture의 RGB합은 mesh0→3.67375944, sprite0→358.183496이다. alpha0 경계6개는
수정 뒤에도 RGB0이고2813/2310 대조의 RT0/RT1은 그대로이며38개 모두 RT1 RGBA hash가 불변이다.
이 수치는 실제 입자 입력을 사용하는 격리 quad 검사이며 실제 아레나 화면의 픽셀 수나 외형 승인이
아니다. 증거는 같은 burst 폴더의 `before.json`, `reference-one-one.json`, `after.json` 및 비교 receipt다.

Play All 수정은 별도로 실제 문서·selector42개 검사와 TU 컴파일·격리 Client 링크를 통과했다.
현재 실행 Client3000/Server29804와 제품 EXE/CSO는 교체하지 않았다. 저장 후 종료하고 정상 Product
빌드·재실행해야 두 수정이 적용된다. Client/UI 실행·캡처와 최종 시각 판정은 수행하지 않았다.

## G10. 원형·도넛의 inner 시간 보간

사용자가 원형과 도넛 모두 원본 lifetime에 따라 채움이 커지도록 구현을 요청했다. 이전 G03의 전체 XZ scale 확장 방식은 이번 변경으로 대체했다. 실제 원본 native3600/3601은 고정 바깥 경계, 도넛의 고정 안쪽 경계와 채움을 한 program에서 합성한다. 발탄처럼 같은 native를 물리3요소로 복제하면 경계가 중복 합성되므로 원본 native1개를 유지하고 기존 `SourceTransformTrack.materialParameterTracks`에 `inner` 곡선만 연결했다.

원형은 `inner=0→1`, 도넛은 `inner=thickness(내경/외경)→1`이다. 고정 thickness, native 수식·텍스처·caustic UV·색·projector 크기·중심·깊이는 유지한다. 원본에서 직렬화된 시간 곡선을 회수한 것은 아니므로 선형 보간 정책은 `PROJECT_AUTHORED`이며, 기존 neutral engine texture 대체 입력도 그대로다.

원본 수명과 fade를 기준으로 fade-out 시작까지1에 도달하고 마지막 fade 구간은1을 유지한다. SourceRecipe의 수명과 원본 fade를 줄이지 않았다.

| 대상 | 원본 예고 수명 | 채움 완료 | 이후 |
|---|---:|---:|---|
| 쇼타임 원형·도넛, 칼날댄스 원형 | 1.5초 | 1.3초 | 원본 fade-out |
| 칼날댄스 도넛 | 0.5초 | 0.433333초 | 원본 fade-out |
| 쓰리투원투하 원형·도넛 | 2초 | 1.733333초 | 원본 fade-out |

기존 원형3·도넛6·확장6·실제 쇼타임 합성2, 총17개 Authored 문서를 기존 asset ID에 설치했다. 확장6개의 전체 Transform scale Lerp는 해제하고 짧아졌던 Detail Life를 SourceRecipe와 같은 원래 수명으로 복구했다. 쇼타임의 실제 `circle.warning.impact`와 `donut.warning.impact`를 직접 수정했으므로 기존 패턴 참조가 이번 보간을 사용한다. 합성 안의 폭발 등24개 비warning 요소는 보존했다.

`build_kouku_showtime_warning_groups.py::animate_radial_fill`을 기존 생성 경로와 `build_kouku_expanding_warning_groups.py`에서 함께 사용한다. 재생성이 고정 inner를 다시 만들지 않는다. 설치는 source/candidate hash를 확인하고 임시 파일 교체·실패 rollback을 사용하며 최초 백업을 덮어쓰지 않는다. 확장6개 Authored와 EffectResourceTree의 표시명은 `채움확장(고정경계)`로 바꿨다. 사용자가 EXE를 실행한 뒤에는 ActionComposition의 이전 resource 표시명과 편집을 유지했다. 해당 리소스의 asset ID와 실제 효과는 동일하다.

현재 소스의 codec/playback/native parameter 함수로 격리 probe를 컴파일·링크했다.17문서의 Load/Drawable/SaveAtomic/Reload, 60Hz 수명 종료와 앞뒤 Seek가 통과했다. 시작·중간·끝의 inner 값, 변하지 않는 다른127개 native parameter lane, 고정 월드 경계·중심·깊이, 끝값 도달 시 양수 alpha를 확인했다. 이는 실제 GPU 화면 판정이 아니다. 후보 재현과 반복 적용17/17, 재생성9+6문서 일치, 참조 리소스99개 존재와 비warning24개 보존을 확인했다. 근거는 `out/KoukuRadialFill20260913/native_validation.json`, `native_samples.csv`, `installation_validation.json`, `CPU/compile.log`, `CPU/link.log`다.

변경 JSON/XML parse와 `git diff --check`는 통과했다. 새 C++/HLSL/Resources binary는 없다. 기존 Product의 material binding/parameter 소비자가 LocalDecal curve를 이미 지원하므로 EXE/CSO를 재빌드하지 않았다. 설치된17개 문서는 `radial_fill_installation.json`의 검증 candidate와 일치한다. 최종 프로세스 확인에서는 사용자가 실행한 `Client/Bin/Debug/Client.exe` PID9404와 TCP7777을 listen하는 Server PID58992가 있었다. 에이전트는 실행·종료·UI 조작·캡처하지 않았다. 이미 열려 있는 Effect draft가 있다면 편집을 보존하고 저장 문서를 다시 열어 확인한다. 최종 원형/도넛 외형·속도와 포탈 위치는 사용자가 직접 판정한다.


## G11. 쇼타임 공 낙하의 원본 CDO 상속 복구 (2026-09-14)

대상은 effect.kouku.gate3.showtime.ball.drop의 기존5요소다. 원본 hidden provider38의
LocationDirect.ScaleFactor와 mesh4의 StartSize는 Distribution=None만 직렬화돼
Engine CDO의 RawDistributionVector lookup=1을 상속해야 했다. 기존 projection의 빈
lookup가 낙하곡선을0배로 만들고 mesh를 generic fallback.2배로 만들었다.
현재 authored 문서의 정확히 두 distribution만 원본 CDO 값으로 복구한 후보를 stage했다.
build_kouku_showtime_restore.py는 재생성에서도 ordinal14에 같은 복구를 적용한다.
--repair-ball-drop-defaults 옵션은 사용자 TRS/ID/material/clock/provider를 보존해 out만 쓴다.

실제 source Projectile421991201 byte3904의 ParticleData.scale1.25와 설치 mesh716정점,
preScale.01을 확인했다. geometry 크기는 .800432×.671151×.671310m다. 실제 Playback의
0.1667초 크기는 기존 .218678×.167788×.167828m에서1.093390×.838938×.839138m로
5배가 된다. 이는 원본 StartSize1 복구이며 임의 확대가 아니다. 기존공은 localY1에 정지했고
복구공은 source curve를 따라 X3.75/Y13.5에서 X.09964/Y1.30684까지 약.5초에 내려온다.
사용자 meshY0/다른4요소Y1, 1.25배와 source X축 이동을 보존했다. 마지막 위치는 provider의
수명 끝 직전 fixed-step sample이며 별도 gameplay 충돌/정확한 지면 도착을 새로 구현하지 않았다.

actual Codec의 drawable/SaveAtomic/reopen 및2초 전체 CPU Playback·finite·반복seek가
before/after 모두 PASS다. 내부 provider1은 숨김이고 draw4는 유지했다. candidate를 두
수정 전 payload로 복원하면 현재 원본 전체와 같으며, 반복실행은 동일하고 사용자 변경된
nonempty StartSize는 거절한다. 원본 archive, 다른Authored, Catalog/Tree/Composition와
Resources를 직접 쓰지 않았다. 독립 actual Codec/Playback의 시간별 연기 birth-history 검증은 1,229개 검사, 실패 0개다.
0.5초에 연기 33개의 첫 위치 Y13.4463, 마지막 Y1.99292, 공 Y1.30684였고,
출생 위치의 경로 폭은 11.9051m다. 출생 후 자체 이동 최대 .0647926m, 현재 공과의
분리 최대 12.7745m로 과거 낙하 경로에 연기가 남았다. mesh와 hidden provider의
오차는 최대 9.53674e-7m, 0.5초 직접 seek의 재현 오차는 0이다. 원본 provider는
0.5초 수명 후 다음 fixed step에서 사망하며, mesh는 마지막 표본 위치를 유지한다.
peer-trail/result.log와 trail.csv가 근거이며 실제 GPU/화면 판정은 아니다.

근거는 out/KoukuShowtimeBallDrop20260914/installation.json, validation.json,
native.json, geometry-validation.json, source_projectile_leaf_parameters.json이다.
Product C++/shader 변경과 Client/UI 실행·캡처·원작 화면 동등성 판정은 없다.


실제 적용: Client/Server 종료 상태에서 위 원본 hash와 후보 hash를 대조한 CAS로 기존 Authored 파일 한 개에 반영했다. `out/KoukuShowtimeBallDrop20260914/installed/receipt.json`의 installed=true와 디스크 after SHA가 근거다. Catalog/Tree/Composition ID나 사용자 박스 값은 변경하지 않았다. Open Editor의 숫자 입력 assertion과 최종 EXE/게시 기록은 [Sequence G24 결과](../09-14/2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md)에 연결한다.


## G12. 기분나빠·폭탄·저주의식·앵콜 블랙홀빔 적용

사용자가 마지막 편집을 Save하고 Client·Server를 종료했다고 알린 뒤 실제 종료 상태와 파일 SHA를 확인했다. `out/KoukuFourEffects20260914/installed/receipt.json`의 CAS로 저작10파일을 적용했다. Gameplay는611→612, WorldSequence는1845→1846이다. 기존59패턴,293 World instance,318 objectResource와 사용자 TRS는 동일하며 독립 Pattern60/61/62를 추가했다. Catalog/Tree/Composition에 새 FX 두 개와 Client project/filter의96.DataFiles None 두 항목을 등록했다. 통합 before/after 및 입력 freshness 독립검토1242검사를 통과했다.

| 새 재생 항목 | 원본 연결 | 저작 길이 |
|---|---|---|
| P60 기분나빠 \| 브레스 | action4219917,31_01 5167ms, notify008 브레스1989~4989ms | 5167ms |
| P61 저주의식 \| 왼손 트레일 | action4219911,27_01,fx_l_hand_01/bip001-l-hand | 4667ms |
| P62 빙고 \| 앵콜세이튼 \| 블랙홀빔 | action4219983,35_01 1000ms→35_04 2500ms,원본5notify | 4509ms |
| 기존 WORLD bingo_bomb 기본 idle/Respawn | 원본 Spark→FX_01/b_body→3 emitter | 각2000ms |

### G12-01. 기분나빠 브레스

원본15요소와 고유 흰 분출·붉은 끝부분을 유지했다. exact module instance에 Distribution=None만 저장돼 nested CDO cooked table을 잃었던10필드(StartRotation, primitive radius/velocityScale, Spawn.Rate)를 복구했다. 원본 재질15개와 root yaw-90, 기존 분신 이동 P50/53/54/55 및 Parent P52는 바꾸지 않았다. 단독 EffectTool은31_01 sourceStart1989ms부터 재생하여 준비 동작과 브레스 clock이 엇갈리지 않는다. 실제226정점×37 mesh pose=8362표본, 실제CModel5 clips/57 bone poses,5582 particle frame 표본과 SourceProfile/리소스27개 SHA 확인을 통과했다. 설치asset은 `effect.kouku.gate3.clone.breath`이며 증거는 `out/KoukuCloneBreath20260914/verification.json`이다.

### G12-02. 쇼타임 해골 폭탄

이미지 모델 MN_RHCN_01은 fm_d_rhcn_00 낙하 공과 다르다. 원본 PSK513점과 설치2127 skinned 정점의 위치 대응 오차2.813e-7m, D/N/S 원본 DDS일치, 실제 CMaterial native program30의7입력을 확인했다. 설치 geometry .671310×.871840×.670530m와 사용자의scale2를 유지했다. 현재 표시 크기계산은1.342621×1.743679×1.341060m이며, 원본 actor DrawScale까지 동일하다는 판정은 아니다.

누락된 심지 PS3요소를 `effect.kouku.gate3.showtime.bomb.fuse`로 연결했다. 실제 source FX_01+[20,0,52.1496]cm와 설치b_body basis를 대조하여 offset[.2,0,-.521496],Rx(-90)을 적용했다. 심지 가장 가까운 실제 정점과4.545mm다. 기존idle/Respawn 두 template만 effectTrack을 받았으며 다른모션/배치는 같다. 실제CModel/CMaterial/Codec/Playback77980검사, yaw0/90/180 및seek오차0을 확인했다. 증거는 `out/KoukuShowtimeBomb20260914/validation.json`이다.

### G12-03. 저주의식 왼손과 Ribbon UV1

원본 WaterRibbon shader2836/3007은 이미 있었지만, source VS가 넘기는 TEXCOORD0.zw를 runtime adapter가0으로 채웠다. PS의 폭 edge mask가항상0이어서 CPU trail점이 존재해도 띠가사라졌다. 원본 PS·texture·scalar 식은 유지하고 exact VS확인후 primary/distortion 양쪽에UV1을 전달했다. 기존 carrier의(distance/tiling,width)를 UV1(width,distance/tiling)으로 연결했다. 이는 원본 PS소비자와현재geometry의축을대조한adapter이며, 원본 CPU vertexbuffer packing 자체를회수했다는판정은아니다.

원본startcontrol/b_wp_1이력을보존하고 사용자지정왼손은 원본fx_l_hand_01 socket의15cmX와 실제bip001-l-hand,Rx(-90)을사용했다. 같은기존asset `effect.kouku.gate3.ritual.hand.trail.full.restore`의세요소재질/recipe/사용자TRS는유지했다. 실제CModel/production sourceAnchor/Playback20226검사, 세yaw간위치오차1.94e-6m,폭·색·age·dynamic오차4.8e-7 및되감기동일성을확인했다. 두Ribbon alpha1/폭.304127·1.36m/길이6.63906·2.07602m를관찰했다. 별도UV/마스크249검사와2primary+2distortion의 generator재생성본문일치도통과했다. 증거는 `out/KoukuRitualLeftTrail20260914/peer-native/validation.json`, `single-flash-peer-review.json`, `peer-regenerated/receipt.json`이다.

### G12-04. 앵콜세이튼 블랙홀빔

새asset `effect.kouku.bingo.encore.blackhole.beam.full.restore`는 준비14,ZoomBlur1,light1,dust2,양눈21씩42의60요소다. actual source action4219983의첫준비·발사만포함하고반복stage2/3은제외했다. 검정원판/빔/무지개flare의native30프로그램과원본DDS46개를유지했다. 설치165bone근거에맞춘socket/basis를사용하고원본눈notify의비등방scale(1,1,1.2)는축교환후(1,1.2,1)이다.

원본nested CDO누락131분포를exact instance근거로상속했다. 원본notify보다긴10emitter는기존Timing소비경로로spawning을0.999675초에끝내고원본2·3초curveclock은보존한다. 꺼져있던native3328 ZoomBlur의기존typed carrier를원본0.3초창에연결했다. 실제Codec/roundtrip/productStage/CModel/productionPlayback636129검사실패0,양눈모든emitter발생,최대particle225/light1/post1,post활성2.01667~2.300초,종료4.508703초/5초잔류0이다. yaw·이동대조오차1.14441e-5m,seek오차0이다. 증거는 `out/KoukuEncoreBlackhole20260914/validation.json`이다.

### G12-05. 빌드와 화면 경계

정규 Debug Product `out/BuildPipeline/runs/20260914T092030409Z-debug-product.json`이PASS다. 변경HLSL을포함한CSO7개가갱신됐고OBJ/PCH/EXE/DLL 재생성은0개다. 이후project변경은새저작JSON의None등록뿐이며C++/HLSL추가변경은없다. Source/후보수치와리소스준비검증은실제화면승인이아니다. Client/UI실행·조작·캡처·GPU Draw 또는시각적PASS는수행하지않았다. 최종외형·텍스처위상·밀도는사용자가확인한다.


## G13. 최종 publish 및 설치 확인

네 이펙트의 저작 설치, 재생 연결, publish와 필요한 셰이더 빌드를 완료했다. 최종 검증 정본은 `out/KoukuFourEffects20260914/final-validation.json`이며 결과는 PASS다.

- Gameplay 612의 새 P60·P61·P62가 게시된 encounter의 playAllPatternIds와 presentation bindings에 모두 존재하고 unavailableReason은 비어 있다. 기존 59개 패턴은 보존했다.
- WorldSequence 1846의 실제 Client runtime JSON이 authoring과 같고, 폭탄 idle/Respawn 두 모션의 심지 effectTrack도 게시됐다.
- KoukuSaydon owner의 product, map, world, gameplay balance 네 domain과 Composition publish가 모두 통과했다. 로그는 `publish-kouku.log`, `publish-composition.log`다.
- 설치한 10개 파일의 SHA, JSON/XML parse, 세이튼 P8/P36의 반시계 90도 회전, 빌드 후 HLSL 원본 SHA를 확인했다. Sequence 58은 유지했다.
- Debug Product 빌드에서 필요한 CSO 7개가 갱신됐다. C++ 변경이 없어 기존 Client.exe와 Server.exe는 다시 링크하지 않았다. 최종 확인 때 두 프로그램은 종료 상태였다.

사용자가 직접 실행할 대상은 `Client/Bin/Debug/Client.exe`와 `Server/Bin/Debug/Server.exe`다. 3관문 패턴 목록에서 P60·P61·P62를 재생하고, 폭탄은 기존 WORLD 오브젝트의 기본 또는 Respawn 모션에서 심지를 확인할 수 있다. 이번 네 이펙트의 최종 시각 판정은 사용자 확인 전이며, 기존 1관문 시퀀스의 완료 확인과 구분한다.
