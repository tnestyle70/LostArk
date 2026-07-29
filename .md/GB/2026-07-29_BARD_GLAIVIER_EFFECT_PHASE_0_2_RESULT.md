# 바드·창술사 Effect Tool Phase 0~2 결과

## 1. 이번 작업의 원칙

- 새 Effect Tool은 바드와 창술사부터 제작한다.
- 기존 버서커, 백룸 Tornado/Hell Blade, 수업용 `Snow`/`Explosion` 리소스는 사용하지 않는다.
- 이번 단계에는 Texture, Mesh, Material 같은 실제 리소스를 연결하지 않는다.
- CPU에서 검증 가능한 공용 Effect 데이터와 파티클 생명주기를 먼저 만든다.
- 바드와 창술사는 같은 런타임 구조를 사용하고, 이후 서로 다른 Effect Asset 데이터만 공급한다.

## 2. Phase 0 — 현재 상태 기준점

작업 시작 기준:

```text
Branch: CY
HEAD: a1220cb Effect_Tool
```

기존 새 Effect 영역에는 다음만 존재했다.

```text
Client/Public/Effect_Types.h   기본 데이터 선언
Client/Public/Effect_Tool.h    빈 ImGui 도구 선언
Client/Private/Effect_Tool.cpp 연결 확인용 창
```

실제 파티클 생성, 시간 진행, 소멸, 개수 검증은 없었다.

기존 `CSnow`와 `CExplosion`은 수업용 Texture·Shader·VIBuffer Prototype에 직접
연결된 별도 구현이다. 새 `CEffect_ParticleSimulator`는 이 코드를 호출하거나
리소스를 참조하지 않는다.

## 3. Phase 1 — Effect 데이터 구조

핵심 파일:

```text
Client/Public/Effect_Types.h
```

구조:

```text
EFFECT_ASSET_DESC
 └─ EFFECT_EMITTER_DESC[]
     └─ EFFECT_MODULE_DESC[]
         ├─ Required
         ├─ Spawn
         ├─ Lifetime
         ├─ InitialLocation
         ├─ InitialVelocity
         ├─ InitialSize
         └─ InitialColor
```

### 렌더링·UI·파일 계층을 분리한 이유

`EFFECT_*_DESC`는 D3D 객체, ImGui 상태, 파일 핸들을 갖지 않는 순수 데이터다.

- ImGui는 이 데이터를 편집한다.
- JSON 또는 향후 `.weffect` 로더는 이 데이터를 저장·불러온다.
- CPU/GPU Runtime은 같은 데이터를 읽어 시뮬레이션한다.
- Renderer는 살아 있는 Particle 배열만 받아 그린다.

따라서 UI를 바꾸거나 파일 형식을 추가해도 파티클 Runtime을 다시 만들 필요가
없고, 바드·창술사 데이터도 같은 스키마로 관리할 수 있다.

### 현재 지원하는 Distribution

```text
CONSTANT
UNIFORM_RANGE
```

Curve, Parameter, Color Over Life 등은 다음 단계에서 Module을 추가하는 방식으로
확장한다.

## 4. Phase 2 — 파티클 시간·생성·소멸

추가 파일:

```text
Client/Public/Effect_ParticleSimulator.h
Client/Private/Effect_ParticleSimulator.cpp
```

`CEffect_ParticleSimulator`의 책임:

- 초당 생성률(`fRatePerSecond`) 누적
- 최초 Burst 생성
- 최대 Particle 수 제한
- Particle Age와 Relative Time 갱신
- Velocity에 따른 Position 갱신
- Lifetime에 도달한 Particle 제거
- 생성·소멸·용량 초과 누적 통계
- 고정 Seed 기반 재현 가능한 Uniform Range 샘플링

이 클래스는 CPU 기준 구현이다. Phase 3에서 GPU 렌더러를 연결할 때
`Get_Particles()` 결과를 Instance Buffer로 전달하면 된다.

## 5. 파티클 개수 자동 검증

검증 조건:

```text
Spawn Rate: 4 particles/sec
Lifetime: 1.0 sec
Emitter Duration: 2.0 sec
Max Particles: 100
Update Step: 0.5 sec
```

기대 결과:

```text
0.5 sec: Alive 2
1.0 sec: Alive 4
1.5 sec: 기존 2개 소멸 + 신규 2개 생성 = Alive 4
Total Spawned: 6
Total Killed: 2
```

실제 검증 결과:

```text
PASS
Debug exit code: 0
Release exit code: 0
```

게임 창이나 GPU를 만들지 않고 검증할 수 있도록 다음 실행 모드를 추가했다.

```powershell
Client.exe --effect-phase2-test
```

정상일 때 프로세스 종료 코드는 `0`, 개수 검증 실패 시 `1`이다. Debug ImGui
Effect Tool 창에서도 같은 검증 결과와 숫자를 확인할 수 있다.

## 6. Debug·Release 빌드

검증 결과:

```text
x64 Debug 전체 Framework.sln 빌드: 성공
x64 Release 전체 Framework.sln 빌드: 성공
```

Release 전체 빌드를 막던 기존 `_DEBUG` 범위 문제도 함께 정리했다.

```text
Engine/Private/RenderTarget.cpp
  Ready_DebugDesc 정의를 _DEBUG 내부로 이동

Engine/Private/GameInstance.cpp
  실제 게임에서도 필요한 Picking/Shadow/Frustum 정의를 _DEBUG 밖으로 이동
```

이는 Effect 기능을 추가한 변경이 아니라, 헤더의 선언 범위와 cpp 정의 범위를
일치시켜 Release 링크가 가능하도록 한 프레임워크 수정이다.

기존 DirectXTK/Effects11 PDB 경고와 숫자 변환 경고는 남아 있지만 빌드 실패는
아니다.

## 7. 변경 파일

```text
Client/Public/Effect_Types.h
Client/Public/Effect_ParticleSimulator.h
Client/Private/Effect_ParticleSimulator.cpp
Client/Public/Effect_Tool.h
Client/Private/Effect_Tool.cpp
Client/Default/Client.cpp
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Engine/Private/RenderTarget.cpp
Engine/Private/GameInstance.cpp
```

## 8. 아직 하지 않은 것

- 바드·창술사 실제 Texture/Mesh/Material 연결
- Sprite Instance Buffer와 Shader 연결
- Effect Asset JSON 또는 `.weffect` 저장·불러오기
- Curve/Color Over Life/Size Over Life
- 여러 Emitter 동시 재생
- Cascade 형태의 Emitter/Module 편집 UI

다음 Phase 3에서는 단색 또는 임시 1×1 Texture를 사용하는 Sprite Renderer를
먼저 연결한다. 실제 바드·창술사 리소스는 Resource Manifest 검증 후 Asset ID로
연결하며, 과거 실험 리소스는 fallback으로도 사용하지 않는다.
