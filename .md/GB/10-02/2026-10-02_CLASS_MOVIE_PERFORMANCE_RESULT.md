# 노트북 Character Select Movie 성능 조사 결과

## G01. 현재 결론과 측정 조건

RTX4050으로 다시 실행한 뒤 사용자가 체감 개선을 보고했다. 현재 Client가 NVIDIA를 사용하는 것은
nvidia-smi와 Profiler JSON의 adapter가 함께 확인한다. 변경 전 GPU 기록은 없으며 GPU 선호와
AC CPU EPP를 같이 바꿨으므로 이전 내장 GPU 사용 여부나 각각의 기여도를 확정하지 않는다.

도화가 Movie의 약20FPS는 캡처로 확인했다. 이 구간은 메인 스레드 갱신·렌더링 호출 비용이 크고
GPU 광원 패스도 무겁다. 따라서 단순히 노트북 GPU 성능 부족이라고 설명하는 것은 부정확하다.
차원술사 첫 카메라의8FPS는 사용자 관찰이며 현재 세 파일에는 해당125ms 구간이 없다.

세 파일은 Debug, D3D11 debug layer ON, debugger 미부착, RTX4050,1920×1080,
export 시점 FPS limit0이다. metadata는 저장 시점 값이며 과거 모든 frame의 class·camera·설정을 보증하지 않는다.
CPU scope는 elapsed time이며 CPU 명령 실행 사이의 대기·스케줄링을 완전히 분리하는 trace는 아니다.

## G02. 실제 JSON과 프레임 범위

폴더: `C:/Users/tnest/Desktop/LostArk/Client/Bin/ProfilerCaptures`.

| 실제 파일 | frame 범위 | 평균 interval / 환산 FPS | p95 / 최대 | Detailed |
|---|---|---|---|---|
| 무비_프레임측정_20261002_151643_262_frame583_29032_0.json | 464~583,120개 | 16.578ms /60.32 | 25.564/41.519ms | OFF |
| 무비_프레임측정_도화가_20261002_152019_331_frame11529_29032_2.json | 11410~11529,120개 | 48.994ms /20.41 | 67.323/83.206ms | ON |
| 무비_프레임측정_차원술사_가디언나이트_20261002_152150_891_frame12236_29032_3.json | 12117~12236,120개 | 30.291ms /33.01 | 52.736/62.284ms | ON |

세 번째 파일의 앞12 frame은 animation22회/frame, 평균55.228ms다.
나머지108 frame은 animation1회/frame, 평균27.520ms로 바뀐다.
서로 다른 부하 상태가 섞여 있으므로 파일 이름만으로 두 class의 성능 평균을 분리하지 않는다.

CPU/GPU scope와 animation drop은 모두0이다. GPU 결과는 첫째·셋째116 valid/4 pending,
둘째120 valid다. pending을0ms 표본으로 평균에 넣지 않았다.
두 Movie 파일의 Detailed ON은 평균적인 실제 플레이 성능과 구분한다. 도화가는 평균3071 CPU scopes/frame이다.
누락이 없다는 사실이 계측 오버헤드가 없다는 뜻은 아니다.

## G03. 도화가의 CPU·GPU 비용

| CPU 구간 | 평균 ms/frame | 관계 |
|---|---:|---|
| Client.Update | 30.438 | 갱신 전체 |
| Client.Render | 18.524 | CPU의 렌더 명령 준비·제출 등 |
| Engine.LevelUpdate | 22.951 | Update에 포함 |
| Animation.Play | 13.255 | LevelUpdate 등에 포함,38회/frame |
| Animation.Channels.Update | 11.516 | Animation.Play에 포함 |
| Engine.LevelUpdate Self | 9.676 | 기록된 하위 scope를 뺀 영역; Movie 전용 세부 계측으로 분해할 대상 |
| Render.NonBlend Self | 6.299 | 하위 scope를 뺀 CPU 영역 |
| Animation.Bones.Combine | 1.682 | 골격 결합 |

Update와 Render의 합은 이 구간의 CPU frame 약48.974ms와 가깝다.
표의 모든 값을 더하면 중첩 구간을 중복 계산한다. 60FPS의 frame budget은 약16.7ms이므로
현재 메인 스레드 비용을 먼저 줄일 근거가 있다. 다른 CPU core가 있어도 이 순차 호출이 자동 분산되지는 않는다.

GPU의 Render.Draw는15.574ms이며 그 안에서 Lights9.520ms, SSAO1.467ms,
NonBlend2.776ms가 기록됐다. GPU 전체 frame interval49.638ms 중 scope 합집합은15.679ms,
미귀속 구간은 약33.960ms다. 전체 interval을 GPU가49.6ms 내내 연산한 것으로 해석하지 않는다.
GPU 공급 공백·드라이버·Present 대기를 정확히 나누려면 별도 타임라인이 필요하다.
광원 패스는 별도 최적화 후보지만 화질 정본의 광원·SSAO·FXAA를 임의로 끄지는 않는다.

## G04. Movie가 일반 Character Select보다 무거운 이유

Movie는 영상 파일이 아니라 기존 world sequence actor들을 외부 시간으로 샘플링하는 실시간 연출이다.
`ClassSelectionPresentation.cpp::Sample_Frame`이 phase의 instance마다
`CWorldSequencePlayer::Seek_InstanceToMs`를 호출한다. 이후 camera, material/light, effect를 샘플링한다.
각 actor의 `WorldSequenceObject.cpp::Sample`은 clip과 시간을 resolve하여
`Set_AnimTrackPosition → Play_Animation(0)`으로 본 행렬을 계산하고 parts도 갱신한다.

실제 `Data/Camera/ClassSelection.cinematics.json` intro 선언은 다음과 같다.

| Movie | instance ID | effect track | light track | material track |
|---|---:|---:|---:|---:|
| 가디언나이트 | 22 | 5 | 7 | 2 |
| 도화가 | 58 | 35 | 8 | 6 |
| 차원술사 | 56 | 10 | 9 | 25 |

이는 phase 전체의 선언 수이지 동시에 화면에 보이는 수가 아니다. 카메라·source clock·가시성·배우·
이펙트의 겹침에 따라 매 frame 비용이 달라진다. 첫 재생은 준비 비용이 겹칠 수 있지만,
차원술사의8FPS 원인이 첫 준비 때문인지 지속적인 연산 때문인지는 아직 포착하지 못했다.

## G05. 재질 분할 모델의 중복 포즈 평가

도화가 저작 문서에는 animated object38개가 있으며 모두 embedded animation을 가진 WModel이다.
실제 Resource의 skeleton·animation 섹션을 읽어 SHA256로 대조한 결과 다음 중복이 확인됐다.

- 전체38 모델의 skeleton 합은7718 bones다.
- 그중32 모델이 각각237 bones/channels를 갖는다.
- 이32 모델은 동일 skeleton·clip bytes를 가진9개 그룹이다. 그룹 크기는10,4,3,3,3,3,2,2,2다.
- 각 그룹의 intro/loop animation track·duration·startDelay·playbackSpeed·motionEnd도 동일하다.
- 모든 animated 모델의 preScale은0.01이다. 가장 큰 a12246.p0~p9는 같은 골격을 열 번 평가한다.

`Tools/CharacterSelectPipeline/split_source_movie_materials.py`는 geometry를 재질별로 나누면서
skeleton/animation bytes를 보존한다. 현재 runtime은 각 분할 모델이 자기 CModel·골격을 소유하여
같은 시간의 같은 포즈도 각각 평가한다. 이 구조는 재질별 표현을 분리하지만 CPU 비용을 반복시킨다.

동일 그룹당 한 번의 계산을 가정하면 평가량은7718→2267 bones가 될 수 있다.
이 수치는 중복 데이터량이며 같은 비율의 frame time 또는 FPS 향상을 뜻하지 않는다.
material과 pose가 mutable인 CModel 자체를 공유하지 않고, 엄격히 같은 입력의 포즈 결과를 재사용하는 설계가 필요하다.
backward seek, loop, pool reuse, parts/attachment, palette invalidation과 포즈 수치 동등성을 검증해야 한다.
이번 작은 변경에는 공유 포즈 최적화를 넣지 않았다.

## G06. 제외한 추측과 먼저 적용할 작은 변경

Animation/Channel/Model/Profiler는 실제 Debug 명령에도 이미 /O2가 있다.
WModel은 compact separate tracks와 cursor를 사용하므로 legacy keyframe cursor reset이 이38 모델의 원인은 아니다.
미제출 모델2개의 animation 비용은0.0694ms라 숨은 모델을 무조건 생략하는 수정의 효과도 작다.
또 prewarm이 Sample(false)를 의도적으로 사용하므로 단순 visible 조건 추가는 예열을 깨뜨릴 수 있다.

반면 Bone.cpp는 /Od /RTC1 /JMC로 컴파일됐고 모든 channel의 행렬 저장·결합 함수가 이 파일을 호출한다.
이 파일만 기존 주변 파일의 Debug 최적화 방식에 맞추도록 프로젝트 항목을 반영했다.
이는 source math·ABI·iterator level·Release 설정을 유지하는 범위이며 실제 절감량은 후속 동일 조건 capture로 확인한다.
ClassMovie.Update/ClassMovie.SampleFrame/ClassMovie.SampleEffects의 새 Debug scope도 추가하여
LevelUpdate의 미분해 비용을 구분한다.

현재 실행 중 EXE에는 새 Movie scope나 Bone 빌드 설정 변경이 아직 적용되지 않았다.
구현·정적 검증·compile/link·화면·성능 결과는 각각 별도로 기록한다.
Bone의 실제 교체 블록과 검증은 [Bone PLAN](2026-10-02_BONE_DEBUG_OPTIMIZATION_PLAN.md)에 연결한다.

## G07. 다음 비교 조건과 증거 위치

Detailed OFF, Frames1200, 같은 viewport·class·카메라·재생 횟수를 사용한다.
Reset 후 Capture를 시작하고 느린 첫 카메라 직후 Capture를 멈춰 앞부분이 rolling window에서 사라지지 않게 한다.
GPU pending 표본은 따로 센다. 현재 authoring/게시 렌더 설정을 변경하지 않은 상태에서 전후 비교한다.

독립 계산 JSON은 외부 복구 디렉터리
`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315`의
`MovieCapture-AgentAnalysis.json`, `MovieCapture-Artist-AgentAnalysis.json`,
`MovieCapture-DimensionGuardian-AgentAnalysis.json`에 있다. 각 파일은 입력 path·SHA·크기와
thread별 inclusive/self, GPU 구간 합집합, 느린 frame 및 수집 한계를 기록한다.
연결 학습 계획은 [Day 1 PLAN](2026-10-02_JOB_PREPARATION_DAY01_PLAN.md),
계측 구현은 [Movie PLAN](2026-10-02_CLASS_MOVIE_PROFILING_IMPLEMENTATION_PLAN.md)이다.
