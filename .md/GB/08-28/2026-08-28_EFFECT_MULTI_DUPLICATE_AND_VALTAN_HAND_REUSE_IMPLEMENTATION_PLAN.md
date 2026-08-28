# 2026-08-28 Effect 다중 Duplicate 및 모아치기 손 파티클 재사용 구현 계획

## G00. 기준과 변경 경계

- 기준: `origin/main@1a9bce42`, PR #255가 병합된 현재 checkout.
- 작업 위치: `C:/Users/user/Desktop/LostArk`, 현재 `codex/valtan-flow-reload-next-fix`.
- 다른 작업의 큰 dirty diff는 보존한다. 시작 diff와 수정 대상 원본은 Git 제외 `.codex_tmp/effect-valtan-reuse-0828`에 보관한다. 이 작업에서 전체 stage/commit하지 않는다.
- Client/UI 실행, 화면 캡처, 사용자 시각 판정 대행은 하지 않는다.
- 새 C++ 파일과 두 번째 renderer/object/runtime은 추가하지 않는다. 기존 프로젝트 등록을 유지한다.

PR #255의 `boss.valtan.hand_1/2`, `hand_3/4`, `hand_5/6`은 모두 왼손 `bip001-l-hand`의 불꽃/연기 쌍이다. 저장된 바인딩은 발악이 아니라 버러지 계열이며, 쌍 사이의 차이는 emitter 기간이다. 기존 V2 파일과 바인딩은 수정하지 않는다.

기본 모아치기는 `VALTAN_CHARGE -> effect.valtan.sequence.charge`다. `VALTAN_CHARGE_2`, `VALTAN_ROAR_CHARGE`, dirty 도넛 Effect는 대상이 아니다. 모아치기의 기존 두 Element와 cue는 그대로 두고 두 Element를 append한다.

## G01. 선택 집합 Duplicate

현재 Shift/Ctrl 클릭은 `m_MarkedElementIds`만 바꾸고 Detail의 열린 행은 유지한다. Delete와 같은 대상 규칙을 Duplicate에도 적용한다.

- 표시한 행이 있으면 그 집합 전체, 없으면 열린 Element 한 개가 대상이다.
- `CEffectDocumentCodec::Build_DuplicatedAuthoredElements`가 입력 identity 검증, 고유 ID 발급, 선택 집합 내부 transform inheritance remap과 문서 stage를 소유한다.
- 외부 master 참조, timing, material, resources, attachment와 실행 가능한 source recipe를 보존한다.
- Tool은 후보 문서를 한 번 commit한다. 실패하면 기존 문서, marks, Detail와 preview를 보존한다.
- 성공하면 새 복제본들을 marked 상태로 유지한다. 다시 Duplicate하면 같은 개수의 쌍을 반복 복제할 수 있다.
- 자동 타격 간격 입력은 추가하지 않는다. 각 복제본의 Start Delay는 원본과 같으며 사용자가 조절한다.

## G02. 기존 Particle의 수치 표현 확장

기존 `Effect_AuthoringDocument -> Effect_DocumentCodec -> Effect_Playback -> Effect_DocumentRenderer`를 확장한다. source-owned Particle의 실행 계약은 바꾸지 않는다.

| optional 저작 필드 | 기본값 | 새 소비 의미 |
|---|---|---|
| `detail.particle.drag` | 0 | acceleration 적용 뒤 `max(0, 1-drag*dt)`로 속도를 감쇠 |
| `rotationRangeDegrees` | `[0,0]` | 입자 생성 때 billboard roll을 선택 |
| `spinRangeDegreesPerSecond` | `[0,0]` | 입자마다 생성 때 선택한 회전속도 유지 |
| `subUVOverLife` | false | 입자 나이/개별 수명으로 atlas frame 선택 |
| `initialVelocity.uniformSolidAngle` | false | 명시한 CONE만 cos(theta)를 균등 표본화 |

기본값에서는 기존 RNG와 재생 결과를 보존한다. 새 필드는 native Sprite Particle에만 허용한다. SourceRecipe/Material Execution과의 의미 충돌, 잘못된 범위와 UV sequence clock 혼합은 validate에서 거부한다. 입자별 UV가 활성화되면 renderer의 emitter 공통 atlas 변환을 중복 적용하지 않는다.

불꽃/연기의 `sizeStart`는 난수 범위가 아니라 width/height다. 원본의 white-to-transparent alpha는 기존 native Particle의 수명 fade로 표현하며 alpha lerp를 중복 적용하지 않는다.

## G03. 왼손 위치와 owner 방향

`actionCueAttachment.orientation`의 기본은 `bone`이고 새 `owner_yaw`만 명시적으로 선택할 수 있게 한다.

- 위치: 실제 `Bone * PresentationRoot`.
- 방향/크기: 실제 owner world의 검증된 unit basis. bone 또는 presentation root를 owner world로 추측하지 않는다.
- `CEffectPlayback::Build_OwnerYawBoneAnchorWorld`를 Tool과 Product가 함께 호출한다.
- 같은 runtime anchor ID가 다른 bone/socket/orientation을 가리키면 거부한다.
- 기존 Artist bone scale 정규화와 기본 bone-follow 경로는 유지한다.
- 기존 본 부착 UI를 native Sprite Particle에도 연결해 Saved Element 복사 뒤 본을 다시 선택할 수 있게 한다.
- 모아치기는 세 animation clip을 잇는다. 기존 historical pose sampling이 현재 clip에만 묶인 제한을 해소하고, 동일 scene CModel의 명시한 clip을 변경 없이 샘플링한다. Tool은 기존 synchronized clip 표에서 pattern 시간을 clip-local 시간으로 해석한다. 다른 preview/runtime 경로는 만들지 않는다.
- Engine `Sample_CurrentAnimationBoneCombinedMatrices`는 현재 clip/blend 계약을 유지한다. 새 명시 clip 진입점은 같은 내부 sampler를 재사용하되 bind/rest pose에서 시작하고 live blend를 섞지 않는다. CAnimationTargetService의 명시 clip binding만 현재 clip 변경을 허용하며 model/target generation 변경은 계속 거부한다. 기존 하네스에서 실제 Valtan model의 다른 clip을 샘플링하고 현재 clip/track/blend/bone state 불변 및 잘못된 index/time/bone 실패를 검사한다. Engine public header 변경 때문에 UpdateLib와 Client Debug/Release 검증이 필요하다.

## G04. 색공간과 모아치기 적용

원 V2는 `Shader_EffectParticleV2.hlsl -> Shader_EffectV2_Common.hlsli`를 사용한다. 이 손 preset은 noise, dissolve, rim, distortion, soft fade가 비활성이고 입자 RGB가 white이므로 기존 `Shader_VtxEffectParticle.hlsl -> Shader_EffectCommon.hlsli`의 base/mask/emissive 수식으로 옮긴다. 셰이더 파일을 복제하거나 V2 object를 기존 Element 내부에 넣지 않는다.

`material.colorTexturesSRGB`는 optional false 기본값이다. true인 generic material의 base/base2/emissive만 기존 `Load_SourceTexture`의 sRGB 경로로 읽고 mask/noise/dissolve는 linear로 읽는다. 기존 문서의 기본 texture load 결과는 바꾸지 않는다. 원 V2의 항상 적용되는 alpha 0.001 discard는 native color.clip 0.001로 표현한다.

`Data/Effects/Authored/effect.valtan.sequence.charge.effect.json`에 다음 두 native Sprite Particle을 추가한다.

| 항목 | 불꽃 | 연기 |
|---|---|---|
| 기준 preset | `boss.valtan.hand_1` | `boss.valtan.hand_2` |
| texture | `Effect/Esther/Wei/Textures/FX_TEX_00/fx_a_fire_023.dds` | `Effect/Artist/Textures/fx_m_smokesq_01.dds` |
| 슬롯 | base/mask/emissive | base |
| blend | additive | alpha |
| color offset | `[0,3,3,0]` | `[0,0,0,0]` |
| emissive intensity | 3 | 1 |
| max particles | 256 | 64 |
| width/height | `[1,1.5]` | `[1.5,2]` |
| spin | 0 | 55~65도/초 |
| atlas | 4×4 | 6×6 |

공통: 시작 0초, emitter 2.7초, 입자 수명 0.6~1.2초, 초당 20개, burst 0, Sphere 반경 0.15, Cone 25도, 속력 0.2~0.5, acceleration `[0,0.03,0]`, drag 1, world-space particles, 왼손 follow/owner_yaw. 2.7초는 모아치기의 3.9초 패턴 안에 최대 1.2초 particle tail을 포함하기 위한 초기 저작값이며 최종 모양/타이밍은 사용자가 판정한다.

두 원본 DDS는 현재 물리 Resources에 있으나 Git 추적 대상은 아니다. 사용자는 팀원 모두 보유한 리소스이므로 Git/LFS 추가·업로드를 명시적으로 거절했다. 이 두 파일은 수정하거나 stage하지 않는다. 기본 validator의 Git closure 검사는 유지하고 명시적 `-AllowLocalResources`만 추가한다. 이 모드는 미추적 리소스도 실제 파일·안전 경로·DDS/WModel 내용 검사를 수행하며 이미 추적된 파일의 index/LFS identity 검사는 생략하지 않는다. 결과에 local 미추적 개수를 구분해 기록한다. 정본 빌드 회귀 명령의 `-AllowLocalEffectResources`는 이 옵션만 전달하며 Git 배포 PASS를 뜻하지 않는다.

기존 Data Files는 `.effectv2.json`을 직접 읽지 않는다. 적용 후에는 Saved Skill Effects의 모아치기 아래에 새 Element 두 개가 보이고, 기존 `Load Saved Element for Editing`으로 다른 Effect에 복사할 수 있다. 기존 portable copy는 모든 FOLLOW를 거부하므로, 새 owner_yaw의 direct native Sprite Particle만 emitter 설정을 복사하고 attachment를 초기화하는 좁은 예외를 추가한다. 기존 source-owned FOLLOW와 Trail은 계속 거부한다. UI는 부착이 초기화되었으며 대상 본을 다시 선택해야 함을 명시한다. 원본 문서와 현재 preview는 복사 후보가 모두 준비되기 전에는 변경하지 않는다.

## G05. 검증과 실행 인계

- 기존 EffectRenderContractHarness에 다중 복제/실패 rollback, optional field 왕복/부정 입력, particle drag/spin/SubUV, owner-yaw matrix와 원본 문서 보존을 검증한다.
- 기존 saved Element copy 및 Valtan Effect Tool Python regression을 실행한다.
- `Tools/EffectPipeline/Validate-EffectSources.ps1`, JSON/XML parse, `git diff --check`를 실행한다.
- 정본 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 Release를 실행하되 Client/UI를 시작하는 항목은 사용자 전용 경계와 분리한다. 실제 실행한 단계와 실패/미실행은 RESULT에 그대로 기록한다.
- build/publisher가 다른 작업과 겹치거나 실행 중 EXE를 점유하면 겹쳐 실행하지 않는다.
- 사용자 경로: `Server + Client` profile `Ctrl+F5` -> `F1 -> Effect Tool -> All Effects -> Valtan -> 모아치기 -> Open Editor`. Element 두 개를 Shift/Ctrl로 표시한 뒤 Duplicate하고 Start Delay를 조절한다.

자동 검증의 PASS는 시각 완성 판정이 아니다. Client 실행과 수동 확인은 사용자에게 인계한다.
