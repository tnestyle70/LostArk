# LostArk 발탄 보스 튜닝 ImGui·JSON 방향 계획서

- 작성일: 2026-07-31
- 문서 유형: 시스템 이해·구현 전 방향 문서
- 우선순위: NavGrid와 Character 이동 검증 뒤 진행
- 이번 범위: 본질과 소유권만 고정한다. 구체 필드·JSON·코드는 보스/이펙트 작업이 안정된 뒤 작성한다.

## 1. C1~C8 관점

| 관점 | 확인한 사실 | 중요도 |
|---|---|---:|
| C1 기준계 | 이동 속도, 거리, 시간, 각도 단위를 JSON과 runtime에서 동일하게 고정해야 한다. | ★★★ |
| C2 이동>계산 | JSON parse/validate는 load/apply 때만 하고 매 프레임은 검증된 숫자만 읽는다. | ★★★ |
| C3 공유는 비싸다 | 기본 tuning 정의와 Valtan clone의 현재 runtime 상태를 분리한다. | ★★★ |
| C4 수명은 선언된다 | Loader가 기본 tuning Prototype, Valtan clone이 적용 사본, Debug Tool이 working copy를 가진다. | ★★★ |
| C5 이산화와 오차 | 재탐색 주기와 거리 임계값은 frame count가 아니라 초·미터로 저장한다. | ★★☆ |
| C6 가지치기 | 유효 범위 밖 값은 Apply 전에 거부하고 기존 runtime 값을 보존한다. | ★★★ |
| C7 권위와 정합성 | JSON은 authoring 정본, 검증된 tuning component는 runtime 사본, ImGui는 working copy다. | ★★★ |
| C8 검증이 병목 | Apply·Save·Reload·실패 rollback과 실제 chase 수치를 함께 확인해야 한다. | ★★★ |

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 발탄 수치는 `Valtan.h/.cpp`와 `Level_AssetTest.cpp`에 3, 5, 2.5, 0.35, 0.25처럼 흩어져 있다.

② 단순 해법의 문제: ImGui가 Valtan 멤버를 직접 수정하면 저장 정본과 runtime 상태가 섞인다. Effect JSON의 `canonicalPayloadHex` 패턴은 사람이 수치 diff를 읽기 어렵다.

③ 해결 방식: 검증 가능한 순수 tuning data, JSON document, runtime tuning component, F1 editor working copy를 분리한다.

④ 비교: Profiler JSON은 write-only capture이고 Effect JSON은 binary payload wrapper다. 발탄 tuning은 사람이 읽고 merge해야 하므로 명시적 이름의 숫자 필드가 필요하다.

⑤ 대가: data/document/component/tool 네 책임이 생긴다. 실제 보스 상태와 이펙트 목록이 확정되기 전에는 구체 필드를 만들지 않는다.

## 3. 자료구조·알고리즘 핵심

후속 구조는 아래 방향만 고정한다.

```text
Valtan.tuning.json
 -> CValtanTuningDocument parse/validate/stage
 -> CValtanTuning Component Prototype
 -> CValtan clone의 적용 사본
 -> CValtan::Update가 moveSpeed/stopDistance/repathSeconds 등을 읽음

F1 ImGui
 -> working copy 수정
 -> Validate
 -> Apply: runtime component 교체
 -> Save: 임시 JSON 작성 후 원자적 교체
```

ImGui는 파일을 매 프레임 읽지 않는다. `Apply`와 `Save`는 분리한다. 잘못된 값은 runtime과 파일 어느 쪽도 바꾸지 않는다.

## 4. 현재 발탄 코드에서 확인한 튜닝 후보

- move speed: 기본 3, AssetTest 전달 5.
- rotation speed: 180 degree/sec.
- stop distance: 2.5m.
- repath interval: 0.35sec.
- goal change threshold: 현재 squared distance 0.25, 즉 0.5m.
- navigation max step/max expanded nodes: follower 기본값 0.6m/16,384.
- idle/run animation clip 이름.

이 목록은 현재 하드코딩 위치를 보여 주는 조사 결과일 뿐 최종 JSON schema가 아니다. 공격, phase, stagger, effect timing은 다른 세션의 구현이 끝난 뒤 실제 소비자가 생긴 항목만 추가한다.

## 5. 권장 소유권

- `CValtanTuningDocument`: JSON I/O와 검증만 담당.
- `CValtanTuning` Client component: 검증된 runtime 수치 사본을 보관.
- `CValtan`: component를 읽어 상태와 이동을 수행.
- `CValtanTuningTool`: F1 UI와 working copy, Apply/Save 명령만 담당.
- `CLoader`: JSON을 읽은 tuning component Prototype과 Valtan Prototype을 등록.
- `CLevel_AssetTest`: Valtan spawn과 검증 배치만 담당.

Tool은 기존 `CGameInstance::Get_Component(level, layer, tag, index)`로 Valtan tuning component를 찾을 수 있어야 한다. Debug 편의를 위해 Engine에 보스 전용 API를 추가하지 않는다.

## 6. 후속 단계

1. NavGrid 수동 편집과 A*를 먼저 완료한다.
2. Character spawn·click 이동을 완료한다.
3. 다른 세션의 Valtan animation/effect/state 결과를 동기화한다.
4. 실제 소비되는 tuning 필드를 목록화한다.
5. 그때 같은 문서를 구현 계획서로 갱신하고 전체 코드·프로젝트 XML·검증 절차를 추가한다.

## 7. 현재 하지 않는 것

- 최종 JSON schema 확정.
- attack/phase/effect field 선점.
- live Apply 코드.
- 별도 singleton 또는 범용 boss framework.
- Release 빌드와 실행 검증.

