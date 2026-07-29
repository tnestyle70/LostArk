# Effect Tool Phase 3~12 구현 결과

## 적용된 편집 화면

F1로 Debug Tool Workspace를 열면 `LostArk Effect Tool` 창에서 다음 기능을
확인할 수 있다.

- 실시간 Particle Preview
- Play/Pause, Restart, Loop, Time Scale
- Emitter 추가·삭제·활성화
- Sprite, Mesh, Beam, Ribbon, Anim Trail 타입 선택
- Module 목록과 활성화
- Spawn Rate, Burst, Max Particle
- Lifetime, Initial Location/Velocity/Size/Color
- Size/Alpha/Velocity Over Life Curve
- Collision Bounds와 Restitution
- Normalized Event
- LOD 거리와 Far Spawn Scale
- JSON Save/Load
- `.weffect` Cook/Load
- Alive, Collision, Event 통계

## Phase별 연결

### Phase 3 — Sprite Preview

`CEffect_ParticleSimulator`의 Particle 배열을 ImGui DrawList로 그린다. 별도
Texture가 없어도 기본 원형 Sprite가 화면에 출력되므로 바드·창술사 리소스가
완전히 준비되기 전에도 Module 동작을 확인할 수 있다.

### Phase 4 — 기본 Module

Spawn, Lifetime, Initial Location, Initial Velocity, Initial Size,
Initial Color가 실제 CPU 시뮬레이션에 반영된다. `Apply / Restart`로 편집값을
Runtime에 다시 적용한다.

### Phase 5 — 메모리 편집 UI

Asset → Emitter → Module 구조를 직접 편집한다. Emitter 복제형 추가, 삭제,
활성화, 타입 전환과 Module별 Property 편집을 지원한다.

### Phase 6 — JSON Round Trip

JSON 파일은 사람이 확인할 수 있는 Asset 요약과 손실 없는 canonical payload를
함께 저장한다.

```text
../Bin/Resources/LostArk/Effect/Editor/working.effect.json
```

저장한 payload를 다시 읽고 동일한 Asset 데이터로 복원한다.

### Phase 7 — Curve Editor

Size, Alpha, Velocity Over Life Module에 Time/Value 키를 추가·삭제·수정할 수
있다. 키는 시간순으로 정렬되고 Runtime에서 선형 보간된다.

### Phase 8 — World Preview

`World Preview`를 켜면 Preview와 같은 Simulator 데이터를 게임 화면 배경
DrawList에도 출력한다. 현재 구현은 Debug용 2D World Overlay이며, 카메라
깊이와 실제 월드 좌표를 사용하는 D3D GameObject 배치는 바드·창술사 실제
Material/Texture가 확정된 뒤 교체해야 한다.

### Phase 9 — `.weffect`

`.weffect`는 다음 Header를 갖는다.

```text
Magic + Schema Version + Payload Size + FNV-1a Hash + Payload
```

Editor의 `Cook .weffect`와 `Load .weffect`로 변환·로드한다.

```text
../Bin/Resources/LostArk/Effect/Editor/working.weffect
```

Release 실행 파일에서도 Binary Load Round Trip을 검증했다.

### Phase 10~11 — 타입별 Preview

- Sprite: 원형 Billboard 대체 표시
- Mesh: 삼각형 Proxy
- Beam: Origin과 Particle을 연결하는 선
- Ribbon: Particle 위치 Polyline
- Anim Trail: 굵은 Polyline

현재 Mesh는 실제 `.wmesh`가 아닌 Proxy다. 실제 Tornado 변환 검증은 과거
백룸 Tornado를 사용하지 않는다는 이번 원칙에 따라 수행하지 않았다. 새로운
바드·창술사 Mesh Resource가 준비되면 `Required.strMeshAssetId`와
`CookedModel`을 연결해야 완료된다.

### Phase 12 — Collision·Event·LOD

- Collision: AABB Bounds 충돌, 위치 보정, 속도 반사, 충돌 횟수
- Event: 지정 Normalized Time에서 Particle당 한 번 발생, Event 횟수
- LOD: Preview Distance에 따라 Spawn Rate를 1.0에서 Far Spawn Scale까지 보간

## 자동 검증

`Client.exe --effect-phase2-test`는 다음을 창 없이 검증한다.

1. Particle 생성·소멸 개수
2. 메모리 직렬화 Round Trip
3. JSON Save/Load
4. `.weffect` Cook/Load와 Hash

최종 결과:

```text
x64 Debug 전체 빌드: 성공
x64 Release 전체 빌드: 성공
Debug Round Trip exit code: 0
Release Round Trip exit code: 0
```

## 중요한 범위 구분

편집 기능과 타입별 독립 Preview는 추가되었다. 하지만 실제 바드·창술사
Texture/Material/Mesh가 아직 프로젝트용 포맷으로 확정되지 않았기 때문에,
다음 항목은 리소스 입고 후 연결해야 한다.

- 실제 Texture Sprite와 Material Blend
- 실제 `.wmesh` Mesh Emitter
- 카메라 View/Projection을 사용하는 3D World GameObject
- Bone Socket 기반 Anim Trail

과거 버서커·백룸·Snow·Explosion 리소스는 이번 Preview의 입력으로 사용하지
않았다.
