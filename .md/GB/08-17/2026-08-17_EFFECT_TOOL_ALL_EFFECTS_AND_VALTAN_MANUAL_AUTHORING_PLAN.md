# 2026-08-17 All Effects 표시와 발탄 수동 저작 계획

branch: `feature/effect-tool-texture-kind-filter`

## 목표

1. `All Effects`의 Q/W/E/R/T/A/S/D/F 기본 행은 Product 부가 검증이 실패해도 항상 표시한다.
2. 기존 saved authored Effect 목록은 각 스킬 아래에서 계속 선택할 수 있게 유지한다.
3. 발탄은 420633 canary 하나만 허용하던 UI 게이트를 풀고, `effect.valtan.*` 수동 저장본을 Product 연결 전에도 열 수 있게 한다.
4. All Effects가 같은 `.animevents`를 두 번 읽지 않고, 이미 읽은 텍스트를 공통 파서에 전달한다.

## 구현 범위

```text
AnimationEffectCueDocument  텍스트 입력 파서와 스킬 클립 필터 추가
AnimationSkillBindingDocument  발탄 pattern Effect tree를 1행 고정에서 N행으로 일반화
Effect_Tool  기본 스킬 행 선반영, saved authored 목록 유지,
             발탄 standalone 목록/Model View 진입,
             420633 전용 transform-history 검증 격리
```

## 이번에 하지 않는 것

- 발탄 패턴을 Product runtime occurrence에 연결하는 작업
- 휠윈드·3연격의 시각 품질 판정
- Track A 하네스 파일의 물리 삭제
- Product prewarm 전체 구조와 runtime payload publisher 재구현

성능 쪽 정본은 다음 결과를 그대로 사용한다.

- `2026-08-17_EFFECT_RUNTIME_PAYLOAD_COMPACTION_RESULT.md`
- `main`의 `f97653fa` (`Effect loading prewarm and All Effects indexing`)

## 검증

요청에 따라 별도 하네스·publisher는 실행하지 않는다.

```text
자동  Client x64 Debug 컴파일/링크, git diff --check
수동  사용자가 All Effects 기본 행/하위 Effect와 Valtan Model View를 직접 확인
```

