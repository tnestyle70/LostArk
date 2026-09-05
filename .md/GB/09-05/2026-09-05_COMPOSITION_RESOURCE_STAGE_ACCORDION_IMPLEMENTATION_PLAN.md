# 2026-09-05 Composition Resources Stage 접기 구현 계획서

> 문서 종류: 구현 계획서
>
> 구현 상태와 실행 증거: [대응 RESULT](2026-09-05_COMPOSITION_RESOURCE_STAGE_ACCORDION_RESULT.md)

## G00. 목표와 현재 호출 경로

Composition Resources에서 다른 리소스를 열 때 이전 리소스의 Stage 목록을 자동으로 닫고,
같은 Action 안에서도 Stage 하나의 슬롯 목록만 열어 둔다. 사용자가 열어 둔 항목을 일일이
닫아야 하는 반복 작업을 줄이는 범위다.

실제 Action/Sequence와 Stage 노드는 `CKoukuSaydonActionWorkbench::Render_ResourceTree`가
그린다. `CompositionResourceTree`는 Category/Profile 폴더만 소유하므로 이 공용 부품을
수정하지 않는다. Valtan의 기존 Sequence 리소스 잎은 Selectable이며 이번 변경 대상이 아니다.

## G01. KoukuSaydonActionWorkbench.h와 .cpp

| 파일 | 추가할 책임 |
|---|---|
| `Client/Public/KoukuSaydonActionWorkbench.h` | 이 Workbench 세션에서 펼친 Action/Sequence와 Stage의 stable ID를 각각 문자열 하나로 보관 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | `Render_ResourceTree`의 실제 노드 입력에 단일 펼침 상태를 연결 |

`m_strExpandedResourceActionId`는 `SEQUENCE/<stableId>` 또는
`ACTION/<profileId>/<sourceActionId>`를 보관한다. `m_strExpandedResourceStageId`는
해당 Action 경로에 `<stageId>`를 붙여 보관한다. 같은 Stage 이름이 다른 Action/Profile에
있어도 상태가 섞이지 않는다. 포인터, vector index, 표시 문자열은 펼침 identity로 사용하지 않는다.

각 노드는 `SetNextItemOpen`으로 해당 세션 상태를 소비한다. 사용자가 Action/Sequence를
다르게 펼치면 이전 Action/Sequence가 접히고 Stage 펼침 상태는 비운다. Stage를 다르게
펼치면 이전 Stage만 접힌다. 현재 Stage의 상위 Action과 Category/Profile 폴더는 유지한다.
같은 노드를 다시 눌러 직접 접는 동작도 보존한다.

검색과 catalog 재구성은 문자열 identity를 교체하지 않는다. 보스 선택으로 Workbench를
전환해도 각 세션의 상태를 사용한다. `Queue_SequencePreview`, `Queue_ActionPreview`,
`Queue_SlotPreview`의 기존 호출과 선택/Append/Save 경로는 그대로 둔다. 펼침은 UI 세션
상태이므로 JSON이나 Composition 문서에 저장하지 않는다.

새 C++ 파일이 없으며 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 필요하지 않다.
기존 두 C++ 파일의 UTF-8 BOM 없음과 CRLF를 유지한다.

## G02. 종료 확인

- `KoukuSaydonActionWorkbench.cpp`만 Debug x64 `ClCompile`로 컴파일한다.
- 기존 `KoukuSaydonSharedEditorContractTests`의 공용 트리와 profile browse 검사 두 개를 실행한다.
- 두 소스와 대응 문서의 `git diff --check`를 확인한다. JSON/XML 변경이 없으면 별도 parse 대상도 없다.
- 사용자가 F1 → Action Workbench → KoukuSaydon → Composition Resources에서 Action A → Stage A를
  연 뒤 Action B를 열어 A의 목록이 닫히는지 확인한다. B의 Stage 1 → Stage 2 전환, 직접 접기,
  검색 변경과 보스 왕복 뒤 preview/선택 동작도 사용자가 확인한다.

에이전트는 Client/UI를 실행·조작하거나 화면을 캡처하지 않는다. 최소 컴파일과 구조 검사는
사용자의 실제 화면 판정을 대신하지 않으며, 실행한 결과만 대응 RESULT에 기록한다.
