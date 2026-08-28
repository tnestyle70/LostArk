# 2026-08-28 Effect 다중 Duplicate 및 모아치기 손 파티클 재사용 결과

작성일: 2026-08-28.

## G00. 완료 상태와 작업 경계

- 기준은 `origin/main@1a9bce42`, 작업 브랜치는 `codex/valtan-flow-reload-next-fix`다.
- Effect Tool의 선택 집합 Duplicate, PR #255 손 Sprite Particle의 native 저작 표현,
  `VALTAN_CHARGE` 모아치기 적용, 왼손 `owner_yaw` 부착과 3-clip preview를 현재 작업 트리에 구현했다.
- 다른 세션의 Valtan authoring, pipeline, sound projection 변경은 수정하거나 되돌리지 않았다.
- 이 작업에서 commit, push, 전체 stage를 하지 않았다.
- 두 DDS는 사용자가 팀원 모두 보유한다고 확인한 로컬 입력이다. Git/LFS에 추가하거나 업로드하지 않았다.
- Client/UI를 실행하거나 화면을 캡처하지 않았다. 최종 불꽃·연기 모양과 타이밍은 사용자 육안 확인 대기다.

## G01. 선택 집합 Duplicate

`Shift`/`Ctrl` 클릭으로 marked Element가 하나 이상 있으면 marked 집합 전체가 Duplicate 대상이다.
marked Element가 없으면 현재 열린 Element 하나를 사용한다. 성공한 복제본은 다시 marked 상태가
되므로 같은 쌍을 연속으로 복제할 수 있다.

- 두 Element 한 쌍에서 `Duplicate 2 Marked`를 9번 누르면 원본 1쌍과 복제 9쌍, 총 10쌍이 된다.
- 각 복제본은 원본 바로 뒤에 안정적인 순서로 삽입한다.
- Element ID는 문서 안에서 고유하게 다시 발급한다.
- 선택 집합 내부의 transform inheritance는 새 ID로 remap하고, 집합 밖 master 참조는 그대로 둔다.
- timing, material, resource, attachment와 실행 payload를 보존한다.
- 후보 문서 전체를 먼저 validate한 뒤 한 번만 commit한다. 실패하면 문서, 열린 행, marked 집합과
  preview를 바꾸지 않는다.
- Start Delay를 자동 증가시키지는 않는다. 반복 타격 간격은 복제 후 각 행에서 저작한다.

구현 중심은 `Client/Private/Effect_DocumentCodec.cpp`의
`Build_DuplicatedAuthoredElements`와 `Client/Private/Effect_Tool.cpp`의 단일 stage/commit 경로다.

## G02. PR #255 손 Particle 분석과 native 이식

PR #255의 손 preset은 다음 V2 경로를 사용한다.

`Shader_EffectParticleV2.hlsl -> Shader_EffectV2_Common.hlsli`

`boss.valtan.hand_1/2`, `hand_3/4`, `hand_5/6`은 모두 왼손 `bip001-l-hand`에 붙는
불꽃/연기 쌍이며, 저장된 기존 binding은 발악이 아니라 rush/trash 계열이다. 이번 요청의
모아치기 대상은 `VALTAN_CHARGE -> effect.valtan.sequence.charge`로 확정했다.

기존 Data Files가 `.effectv2.json`을 직접 로드하는 두 번째 런타임은 만들지 않았다. 이 preset에서
활성인 base/mask/emissive, blend, color offset, atlas와 particle motion을 기존 native 경로에 옮겼다.

`Shader_VtxEffectParticle.hlsl -> Shader_EffectCommon.hlsli`

원 preset에서 꺼진 noise, dissolve, rim, distortion, soft fade는 추가하지 않았다. 따라서 수치와
저작 의도는 보존하지만 V2 RNG와 픽셀 출력의 byte-identical 복제라고 판정하지 않는다.

## G03. native Particle와 부착 계약 확장

기존 `Effect_AuthoringDocument -> Effect_DocumentCodec -> Effect_Playback ->
Effect_DocumentRenderer` 경로에 다음 optional 필드를 추가했다.

| 필드 | 기본값 | 소비 |
|---|---:|---|
| `drag` | `0` | acceleration 뒤 속도 감쇠 |
| `rotationRangeDegrees` | `[0,0]` | 생성 roll |
| `spinRangeDegreesPerSecond` | `[0,0]` | 입자별 회전 속도 |
| `subUVOverLife` | `false` | 개별 수명 기준 atlas frame |
| `initialVelocity.uniformSolidAngle` | `false` | cone solid-angle 균등 표본화 |

기본값 문서는 기존 RNG draw와 birth integration을 유지한다. 새 V2 표현을 명시한 native Particle은
V2와 같은 update-then-birth 순서를 사용해 생성 tick에서 한 번 더 적분하지 않는다. 잘못된 범위,
source-owned recipe와의 의미 충돌, emitter UV sequence와 per-particle SubUV 혼합은 codec이 거부한다.

부착 orientation에 기본 `bone`과 새 `owner_yaw`를 두었다. `owner_yaw`는 실제 bone의 presentation
위치를 사용하되 방향과 크기는 검증된 owner world basis를 사용한다. 계산은
`CEffectPlayback::Build_OwnerYawBoneAnchorWorld`를 Tool과 Product가 공유한다. 같은 anchor ID에
서로 다른 bone/socket/orientation이 들어오면 fail-close한다.

모아치기 3개 animation clip의 과거 pose를 같은 `CModel`에서 이름으로 읽을 수 있게 했다.
명시 clip sampling은 현재 clip, track, blend와 bone state를 바꾸지 않는다. Tool은 세 clip을 모두
preflight한 뒤에만 상태를 commit하고, 잘못된 source window나 start 실패는 이전 preview 상태를
보존한다.

`material.colorTexturesSRGB`는 optional false다. true일 때 base/base2/emissive만 sRGB SRV로 읽고
mask/noise/dissolve는 linear를 유지한다. Saved Element copy는 direct native owner-yaw Sprite
Particle에 한해 emitter와 material을 복사하되 attachment를 초기화하며, 사용자가 대상 본을 다시
선택해야 한다. 기존 source-owned FOLLOW와 Trail 거부 경계는 유지한다.

## G04. 모아치기 데이터

`Data/Effects/Authored/effect.valtan.sequence.charge.effect.json`의 기존 두 Element는 보존했고 다음
두 Element를 append했다.

| Element ID | 역할 | 주요 값 |
|---|---|---|
| `authored.valtan.charge.hand-fire` | 왼손 불꽃 | additive, base/mask/emissive, emissive 3, 4x4 atlas, cap 256 |
| `authored.valtan.charge.hand-smoke` | 왼손 연기 | alpha, base, emissive 1, 6x6 atlas, spin 55~65 deg/s, cap 64 |

공통값은 시작 0초, emitter 2.7초, 수명 0.6~1.2초, 20 particles/s, burst 0,
sphere 반경 0.15, cone 25도, 속력 0.2~0.5, acceleration `[0,0.03,0]`, drag 1,
world-space billboard, SubUV over life, clip 0.001, 왼손 follow/owner_yaw다. 최대 1.2초 tail을
포함해 3.9초 모아치기 안에서 끝나도록 잡았다.

로컬 전용 DDS는 아래 두 파일이다.

- `Client/Bin/Resources/Effect/Esther/Wei/Textures/FX_TEX_00/fx_a_fire_023.dds`
- `Client/Bin/Resources/Effect/Artist/Textures/fx_m_smokesq_01.dds`

`-AllowLocalResources` 검증 모드는 미추적 파일의 존재, 안전한 Resources 상대 경로와 DDS 내용을
검사한다. 이미 추적된 리소스의 Git/LFS identity 검사는 완화하지 않으며 이 결과를 Git 배포 PASS로
해석하지 않는다.

## G05. 자동 검증 결과

| 검증 | 결과 |
|---|---|
| 관련 Effect Python 회귀 7파일 | PASS, 178 tests, skipped 7 |
| Effect source validation `-AllowLocalResources` | PASS, direct 197, resources 1031, local untracked 2 |
| EffectRenderContractHarness Debug 빌드/실행 | PASS, exit 0 |
| Client Debug 빌드 | PASS, 최신 Effect/Tool 코드 링크 완료 |
| EffectRenderContractHarness Release 최신 소스 빌드 | PASS |
| Release 하네스의 이번 범위 단계 | PASS: color-space, multi Duplicate, Valtan document metadata, particle dynamics/life/SubUV |
| 대상 JSON / vcxproj XML parse | PASS |
| `git diff --check` | PASS, 기존 line-ending 경고만 출력 |
| DDS Git 상태 | 두 파일 모두 untracked, staged Resources 0 |
| Debug 정본 전체 회귀 | 다른 세션의 stale sound projection에서 중단: 511 source 중 497 admit, 14 NONE skip |
| Release 정본 전체 회귀 | 다른 세션의 갱신 중인 `VALTAN_WARP/STEP_02` V2 join에서 Client pre-build 중단 |

Debug 전체 회귀의 sound 실패는 작업 시작 전 보존한 `before.patch`에도 있던 rush action
`playbackMode: NONE` 변경과 갱신 전 sound 문서의 불일치다. Release 중단은 다른 세션이 작업 중인
Valtan pipeline이 `PORTAL_TARGET_RUSH`를 요구하지만 현재 Product가 `PORTAL_CROSS_ARENA`인 상태다.
두 경우 모두 Effect 변경 파일을 통과한 뒤의 외부 계약이며, 사용자 요청에 따라 해당 세션 파일을
덮어쓰지 않았다.

자동 검증은 시각 품질 PASS가 아니다.

## G06. 사용자 실행 확인

이 PC는 LAN sync 결과 `server-host`다. Visual Studio에서 `Server + Client` profile을 `Ctrl+F5`로
실행한 뒤 다음 순서로 확인한다.

1. `F1 -> Effect Tool -> All Effects -> Valtan -> 모아치기`를 연다.
2. `Open Editor` 또는 authoring timeline 재생으로 세 clip 동안 왼손 불꽃과 연기 follow를 확인한다.
3. 두 손 Particle 행을 `Shift` 또는 `Ctrl` 클릭해 marked로 만든다.
4. `Duplicate 2 Marked`를 9번 눌러 총 10쌍이 되는지 확인한다.
5. 반복 타격 시간에 맞춰 복제본의 Start Delay를 조정한다.
6. 다른 Effect에 쓸 때는 Saved Element로 불러온 뒤 대상 bone을 선택하고 `Attach Selected Element`를
   다시 적용한다.

최종 occurrence, 크기, 색, 손에서의 위치와 tail은 사용자 관찰로 승인한다.
