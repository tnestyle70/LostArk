# 선택형 direct-authored Effect publisher 구현 결과

## 1. 결론

거대한 `EffectCatalog.runtime.json` 전체를 재생성하지 않고, 감사된 direct-authored schema v13
Effect stable ID만 선택해 검증·publish하는 도구를 추가했다. 여러 작업 세션의 authored Effect가
동시에 dirty인 상황에서도 요청한 row와 content-addressed seal만 바꾸므로 기능별 PR을 독립적으로
조립할 수 있다.

## 2. 실행 계약

`Publish-SelectedDirectAuthoredEffects.ps1`은 다음 순서로 동작한다.

1. target ID가 audited allowlist의 stable ID인지 확인한다.
2. source catalog에 정확히 한 direct-authored v13 row가 있고 material/visual program binding이 없는지 확인한다.
3. authored document의 identity, Element ID, visible Element, Resources-relative `.wmodel`/`.dds`를 검증한다.
4. 기존 compactor로 payload를 stage하고 SHA-256 seal 경로를 계산한다.
5. runtime catalog에서 target lexical object만 교체한 뒤 non-target semantic 불변을 다시 확인한다.
6. seal 설치와 catalog 교체를 transaction lock 아래 수행하고 실패 시 catalog와 새 seal을 rollback한다.
7. 이미 같은 seal을 가리키면 파일을 다시 쓰지 않는 no-op으로 종료한다.

`Validate`는 source와 예상 seal을 계산해 현재 runtime과 일치하는지만 확인하고 파일을 바꾸지 않는다.
`Publish`는 staged bundle validation을 통과한 뒤에만 commit한다. test fault injection은 임시 runtime
root에서만 허용한다.

## 3. 실행형 harness

`Test-PublishSelectedDirectAuthoredEffects.ps1`은 격리된 임시 Data/Resources/runtime fixture를 만들고
다음을 검증한다.

- 여러 target 동시 Validate와 Publish
- content-addressed seal 생성과 runtime catalog target row 교체
- non-target catalog row와 기존 seal byte identity 보존
- material-program 및 visual-program bound target 거부
- allowlist 밖 ID와 중복 target 거부
- 두 번째 Publish의 byte-for-byte no-op
- catalog backup 이후 fault injection과 완전 rollback
- transaction lock과 임시 파일 잔류 없음
- 최종 direct-authored runtime catalog validator PASS

clean `origin/main@b8653bfd` 기준 실행 결과는 다음과 같다.

```text
PASS: selected runtime targets already match their compacted authored seals.
PASS: selected direct-authored Effect validate/publish/no-op/binding/rollback contract.
```

PowerShell AST parse와 `git diff --check`도 PASS했다. 이 도구와 harness는 C++ 또는 제품 runtime
코드를 바꾸지 않으므로 Debug/Release build 대상이 아니다.

## 4. 완료 경계

allowlist는 현재 실제 source catalog에서 감사한 unbound direct-authored Product target만 포함한다.
새 stable ID를 자동 허용하거나 material/visual program binding을 우회하지 않는다. 일반적인 전체
Effect publish 정본은 계속 `Publish-Effects.ps1`이며, 이 도구는 dirty corpus에서 독립 PR을 만들기
위한 좁은 transaction 경로다.
