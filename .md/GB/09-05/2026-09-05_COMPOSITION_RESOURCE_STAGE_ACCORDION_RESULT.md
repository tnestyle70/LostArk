# 2026-09-05 Composition Resources Stage 접기 결과

> 대응 계획: [구현 계획서](2026-09-05_COMPOSITION_RESOURCE_STAGE_ACCORDION_IMPLEMENTATION_PLAN.md)
>
> 구현과 최소 자동 검증 완료. 사용자 화면 확인은 미실행.

## G01. 실제 구현

`Client/Public/KoukuSaydonActionWorkbench.h`와
`Client/Private/KoukuSaydonActionWorkbench.cpp`에 22줄을 추가했다.

- 다른 Action/Sequence를 펼치면 이전 리소스의 Stage/clip 목록이 닫힌다.
- 같은 Action의 다른 Stage를 펼치면 이전 Stage의 슬롯 목록이 닫힌다.
- 직접 접기는 유지한다. 현재 Stage의 상위 Action과 Category/Profile 폴더를 함께 닫지 않는다.
- Action/Sequence는 `SEQUENCE/<stableId>` 또는 `ACTION/<profileId>/<sourceActionId>`,
  Stage는 그 Action 경로와 `<stageId>`로 구분한다. 검색 재구성과 보스 세션 전환에서도
  펼침 상태는 이 stable source identity를 사용한다.
- 기존 resource preview, 선택, Append, Save 호출은 변경하지 않았다. 펼침 상태는 문서에 저장하지 않는다.

공용 `CompositionResourceTree`, Valtan 리소스 목록, project/filter, JSON/XML은 변경하지 않았다.
새 파일 등록이나 runtime resource 전달은 필요하지 않다. 두 C++ 파일은 기존 UTF-8 BOM 없음과
CRLF를 유지했다.

## G02. 실행한 자동 검증

| 검사 | 결과 |
|---|---|
| `KoukuSaydonActionWorkbench.cpp` Debug x64 최소 `ClCompile` | exit code 0, 오류 0, 경고 1 |
| 아래 기존 focused resource 검사 2개 | 2개 통과 |
| 변경한 두 C++ 파일 `git diff --check` | 공백 오류 없음 |
| 변경한 JSON/XML parse | 변경 파일이 없어 대상 없음 |

컴파일 로그는 Git 제외 경로 `out/PR316/compile/client-resource-accordion.log`다. 경고 1건은
기존 `EngineSDK/Inc/Engine_Enum.h`의 C4819 코드 페이지 경고이며 해당 파일은 변경하지 않았다.
Client 링크나 Product 전체 빌드를 실행한 증거로 기록하지 않는다.

실행한 컴파일 인자:

```text
MSBuild.exe Client/Default/Client.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64 /p:SelectedFiles=..\Private\KoukuSaydonActionWorkbench.cpp /p:BuildProjectReferences=false /m:1 /nr:false /v:minimal
```

실행한 두 기존 검사의 정확한 이름:

```text
Tools.KoukuSaydonPipeline.test_kouku_saydon_client_product_level_contract.KoukuSaydonSharedEditorContractTests.test_resource_tree_is_one_shared_component_over_per_boss_catalogs
Tools.KoukuSaydonPipeline.test_kouku_saydon_client_product_level_contract.KoukuSaydonSharedEditorContractTests.test_kouku_resources_browse_every_profile_but_bind_only_the_boss_body
```

새 하네스나 검사를 추가하지 않았다. 위 두 검사는 기존 공용 트리·browse 계약을 확인한 것이며,
실제 ImGui 클릭과 Stage 자동 접힘을 실행 검증한 것은 아니다.

## G03. 사용자 화면 확인과 남은 경계

Client/UI 실행·조작, 화면 캡처와 육안 판정은 수행하지 않았다. 사용자 확인 절차는 다음과 같다.

1. F1 → Action Workbench → KoukuSaydon → Composition Resources에서 Action A와 Stage를 연다.
2. Action B를 열고 A의 Stage 목록이 닫히는지 확인한다.
3. B의 Stage 1을 연 뒤 Stage 2를 열어 이전 슬롯 목록만 닫히는지 확인한다.
4. 같은 노드의 직접 접기, 검색 변경, 보스 왕복 뒤 상태와 resource preview/선택이 유지되는지 확인한다.

입력·화면 결과의 최종 확인은 이 절차를 사용자가 실행한 뒤 기록한다.
