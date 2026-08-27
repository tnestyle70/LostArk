# Effect Tool 서버 패턴 재생 바로가기 구현 결과

## 0. 결과

`Effect Tool -> All Effects -> Valtan`에서 한국어 Pattern 목록을 보고 같은 행의 `Play Server`로
Valtan Arena Server pattern을 one-shot 재생할 수 있게 연결했다. Pattern 아래의 Runtime Product Effect는
기존 `Open Editor`와 local Model View timeline 재생을 그대로 유지한다. 독립 Effect는 정확한 owner
Pattern을 찾은 경우에만 `Play Server Owner`를 제공한다.

이 변경은 Effect 전용 packet이나 두 번째 replay runtime을 만들지 않았다. Boss Tool과 Effect Tool은
같은 `CValtanPatternAuditionService`에 stable `patternId`를 제출한다. MainApp만 service lifecycle queue를
소비하며, `Repeat / Stop / Revive / live graph / 원인 진단`은 Boss Tool만 소유한다.

Boss Tool의 기존 전체 graph selector도 제거했다. 이제 All Effects와 Boss Tool은
`CValtanPatternTree::Build_ToolAuditionInventory()`가 검증한 Core 6 + animator manual 20을 같은 순서로
소비한다. 전체 graph 53개는 실제 Server live/owner 진단에만 남고, 목록 밖 Pattern은 선택·Play·Repeat로
재진입할 수 없다.

## 1. 구현 계약

- Server identity: `boss.valtan.center + Pattern.strPatternId`
- 실행 범위: Server에 연결된 `LEVEL::VALTAN_ARENA`
- 실행 전 조건: replicated Valtan과 player가 valid, alive, combat-ready
- 동시 실행: 공용 audition이 진행 중이면 현재 owner와 상태를 표시하고 새 제출을 거부
- 상태 격리: `consumerId == "Effect Tool"`인 snapshot만 Effect Tool 행에 표시
- Pattern 목록: Core 6개와 animator manual 20개를 한국어 `displayName` 우선으로 표시
- Boss selector: 같은 공용 26개 inventory만 표시하고 legacy/product 추가 행은 숨김
- live 예외: 목록 밖 실제 Server Pattern은 read-only `live only`로만 표시
- 반복 안전성: Repeat 중 다른 Pattern 선택 시 다음 반복을 해제하고 현재 occurrence만 정상 종료
- 독립 Effect: `ownerPatternId` exact join 성공 시에만 owner Pattern 전체를 Server에서 실행
- unpublished draft: Server 실행 버튼 없음
- local authoring: `Open Editor`, Effect local play, Model View timeline 상태를 Server 제출이 초기화하지 않음
- 저장하지 않은 Effect 편집: 현재 Server Product 재생에는 포함되지 않음을 화면에 명시

## 2. Winters Engine ImGui 적용

첫 화면에는 Pattern, `Play Server`, 연결 Effect, `Open Editor`만 우선 노출한다. 정상 inventory 수와
ownership 검사 성공 메시지는 숨기고 실패 이유만 해당 Pattern 아래에 표시한다. raw JSON, Balance 수치,
Repeat/Revive, 별도 Server tab은 Effect Tool에 추가하지 않았다.

Boss Tool의 본질과 인접 Tool 경계는 `.md/TEAM/보스툴.md`에 정리했다.

## 3. 자동 검증

| 검증 | 상태 |
|---|---|
| Effect Tool All Effects contract | PASS, 10 tests |
| Effect Tool saved rows | PASS, 32 tests, 7 skipped |
| Boss Tool contract | PASS, 12 tests |
| Balance Tool contract | PASS, 25 tests |
| Valtan Pattern Tree contract | PASS, 18 tests |
| Valtan Pattern Master V2 | PASS, 46 tests |
| Valtan animation promotion Validate | PASS, 20 patterns / 99 stages |
| Valtan V2 publisher Validate | PASS, managed 27 / legacy 26 / world 30 / artifacts 9 |
| JSON/XML parse + `git diff --check` | PASS |
| Client x64 Debug C++ compile | PASS, `BossTool.cpp`, `Effect_Tool.cpp`, `ValtanPatternTree.cpp` 포함 |
| Client canonical link | 실행 중인 `Client.exe` 잠금 해제 후 재실행 필요 |
| 사용자 육안 검증 | 미실행, visual PASS 아님 |

격리 OutDir 링크 시도는 `Effect_Tool.cpp` 컴파일까지 성공했지만 project-reference 산출물을 함께 격리하지
않아 `Shared.lib`를 찾지 못했다. 제품 EXE 판정은 이 우회 결과가 아니라 canonical build로만 닫는다.

동시에 진행 중인 target-axe Effect 연결 작업 때문에 `test_valtan_model_view_composition.py` 10건 중 1건은
현재 `BossCatalog.json`의 새 carrier Effect와 테스트의 기존 `effect.valtan.sky-axe.active` 기대값이 달라
실패했다. 이 selector/Server shortcut 변경에서는 해당 파일을 수정하거나 되돌리지 않았다. 연결 작업
세션이 정본과 기대값을 함께 닫은 뒤 전체 회귀를 다시 실행해야 한다.

## 4. 사용자 육안 검증 절차

1. 최신 canonical Client build 뒤 `Server + Client`로 Valtan Arena에 들어간다.
2. `F1 -> Effect Tool -> All Effects -> Valtan`을 연다.
3. Core 6개와 animator 20개 Pattern이 한국어 이름으로 보이는지 확인한다.
   `Boss Tool`에서도 같은 26개만 같은 두 그룹으로 보이고 이전 legacy 목록이 사라져야 한다.
4. `3회 땅 치기 후 돌진`, `2페이즈 4연속 공격`, `점프 후 플레이어 추적 도끼`의 `Play Server`를
   각각 눌러 실제 Server fixed-tick Pattern과 연결 Effect를 확인한다.
5. Pattern을 열어 Runtime Product Effect의 `Open Editor`를 누르고 Model View timeline에서 local Effect와
   owner animation을 튜닝한다.
6. 다시 `Play Server`를 눌러도 현재 Model View 문서와 timeline이 초기화되지 않는지 확인한다.
7. 독립 도넛과 target axe는 `Play Server Owner`로 owner Pattern 전체가 실행되는지 확인한다.
8. player가 사망했다면 `Boss Tool -> Revive Player`를 사용한 뒤 Effect Tool에서 다시 재생한다.

사용자의 서면 관찰 전에는 Effect 크기, timing, camera, decal과 최종 visual fidelity를 PASS로 기록하지 않는다.
