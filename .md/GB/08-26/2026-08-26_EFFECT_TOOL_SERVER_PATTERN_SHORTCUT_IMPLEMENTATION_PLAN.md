# Effect Tool 서버 패턴 재생 바로가기 구현 계획

## 0. 작업 결론

Effect 작업자가 연결 Effect를 찾은 뒤 `Boss Tool`로 이동하지 않고도 같은 Server 제품 Pattern을 즉시
검증할 수 있도록 `All Effects -> Valtan`에 `Play Server` 바로가기를 둔다. 이 기능은 두 번째 재생기나
Effect 전용 Server command를 만들지 않는다. `Boss Tool`과 같은 `CValtanPatternAuditionService`에 stable
`patternId`를 one-shot으로 제출하는 얇은 UI다.

```text
All Effects / Valtan
  -> 한국어 Pattern 이름 + 연결 Effect 수
  -> Play Server
  -> 연결된 Runtime Product Effects
       -> Open Editor
       -> Play Effect (+ Owner Animation)  // 기존 Model View authoring timeline
```

`Boss Tool`은 반복, 중단, 부활, live Stage와 연결 원인 진단을 계속 소유한다. `Effect Tool`은 선택 Pattern
한 번 재생과 현재 연결 Effect 편집에만 집중한다.

Boss Tool의 selector도 All Effects와 같은 공용 inventory인 Core 6 + animator manual 20만 소비한다.
전체 graph는 live 진단에 유지하되 legacy/product 추가 행은 선택·Play·Repeat 목록에 노출하지 않는다.

## 1. 권위와 실패 경계

- Server 실행 identity는 `boss.valtan.center + Pattern.strPatternId`뿐이다.
- `LEVEL::VALTAN_ARENA`와 연결된 Debug Server에서만 버튼을 활성화한다.
- Character Select/local boss/animation 직접 재생을 Server 검증처럼 사용하지 않는다.
- 공용 서비스가 이미 다른 Tool 요청을 수행 중이면 owner와 상태를 표시하고 새 요청을 거부한다.
- Effect Tool 상태는 service snapshot의 `consumerId == "Effect Tool"`인 경우에만 갱신한다.
- 독립 Effect는 `Effect.strOwnerPatternId`를 exact join하여 owner Pattern 전체를 재생한다.
- unpublished Pattern Draft에는 Server Play를 제공하지 않는다.
- Repeat, Stop, Revive는 추가하지 않는다. 사망한 player는 Boss Tool의 Revive를 사용한다.

## 2. Winters Engine ImGui 계약

첫 화면에는 다음 질문의 답만 남긴다.

```text
어떤 Pattern인가?
Server에서 그대로 재생하려면 무엇을 누르는가?
어떤 Effect가 연결되어 있고 어디서 편집하는가?
```

- Pattern 행은 한국어 `displayName`을 먼저 보여 주고 stable ID는 보조 정보로 둔다.
- `Play Server`는 Pattern 행 바로 옆 한 번만 둔다.
- 연결 Effect 아래의 `Open Editor`와 local Play는 기존 Model View timeline을 유지한다.
- 독립 Effect에는 owner Pattern 실행이라는 의미를 드러내는 `Play Server Owner`만 둔다.
- 실행 실패 이유와 lifecycle 상태는 해당 Pattern 바로 아래에 표시한다.
- raw JSON, gameplay 수치 편집, Repeat/Revive, 별도 탭은 추가하지 않는다.

## 3. 구현 단위

1. `Effect_Tool`에 Arena-only resolver와 shared audition Submit/status filter를 복구한다.
2. Pattern 행과 independent Effect 행에 최소 Server action을 배치한다.
3. 한국어 이름 우선 label과 간결한 authoring/Server 역할 안내를 적용한다.
4. Boss/Balance/Effect static contract를 `공용 실행 서비스 + Boss 전용 제어` 계약으로 갱신한다.
5. `CValtanPatternTree`에서 All Effects와 Boss Tool이 함께 소비하는 exact 26 inventory를 stage한다.
6. `보스툴.md`에 Boss Tool의 본질, exact 목록, 확장 원칙, 인접 Tool shortcut 규칙을 기록한다.

## 4. 검증

- Effect Tool All Effects contract
- Boss Tool contract
- Balance Tool contract
- Valtan Pattern Tree / Pattern Master V2 contract
- Valtan V2 publisher validation
- Client x64 Debug canonical build
- `git diff --check`와 JSON/XML parse
- 사용자 Valtan Arena 육안 검증: Pattern Play Server, 연결 Effect Open Editor, Model View local timeline

Client와 UI는 에이전트가 실행하거나 조작하지 않는다. 자동 검증 이후 사용자가 직접 Server + Client에서
육안 판정한다.
