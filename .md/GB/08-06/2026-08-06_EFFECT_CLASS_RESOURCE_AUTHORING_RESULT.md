# Effect Tool 클래스별 리소스 저작 구현 결과

## 완료 상태

`Client/Bin/Resources/Effect/<Domain>`의 실제 물리 폴더를 저작 category의 정본으로 사용하도록
Effect Tool을 확장했다. 별도 manifest나 두 번째 Effect runtime은 만들지 않았으며 기존
`Effect Document -> Playback -> Renderer -> CEffectObject`와 thumbnail cache를 그대로 사용한다.

완료한 계약은 다음과 같다.

- Resource Catalog entry가 첫 상대 경로 component인 `Artist`, `DimensionMaster`,
  `LanceMaster`, `Valtan`, `Warlord`를 `strDomainId`로 소유한다.
- 도메인 아래 parent path를 `Meshes/...`, `Textures/...` 하위 category로 분리하고 file kind별
  category와 개수를 한 번의 catalog refresh에서 stage한 뒤 commit한다.
- Resource Browser에 `Authoring Category`와 `Resource Folder`를 분리했다. domain, file kind,
  하위 category, search가 모두 맞는 항목만 visible-index cache에 들어간다.
- Mesh Shape 슬롯은 선택 도메인의 모든 WModel을 후보로 표시한다. Base/Noise/Mask/Emissive/
  Dissolve 슬롯은 선택 도메인의 모든 DDS를 후보로 표시하며 이름 기반 의미 추측으로 제외하지 않는다.
- Bind Selected에서도 선택 도메인과 file kind를 다시 검사하므로, category 전환 전에 남은 stale
  selection이나 다른 도메인 asset을 문서에 넣을 수 없다.
- All Effects의 playable class 선택은 실제 리소스 폴더가 있는 LanceMaster, Artist,
  DimensionMaster domain과 동기화된다. Gunslinger와 Slayer에는 물리 Effect 폴더가 없으므로
  다른 class 리소스로 fallback하지 않고 기존 선택을 보존한다.
- Data Files에도 같은 `Authoring Category` 필터를 연결했다. Imported 경로, PlayerSkills 소유 class,
  stable effect ID token 순으로 domain을 판정하고 근거가 없으면 `Uncategorized`로 격리한다.
- Warlord와 Valtan은 저작 resource/Data Files domain으로 보이지만 playable roster나 runtime
  onboarding은 추가하지 않았다.
- Save/Save As의 물리 경로는 기존 flat `Data/Effects/Authored/<effectId>.effect.json`을 유지한다.
  category는 UI 분류일 뿐 새 저장 정본이나 schema field가 아니다.

## 실제 리소스 팔레트

| 도메인 | Mesh Shape 후보 | Texture 슬롯 후보 | 합계 |
|---|---:|---:|---:|
| Artist | 35 WModel | 237 DDS | 272 |
| DimensionMaster | 140 WModel | 696 DDS | 836 |
| LanceMaster | 69 WModel | 286 DDS | 355 |
| Valtan | 52 WModel | 346 DDS | 398 |
| Warlord | 149 WModel | 652 DDS | 801 |
| 전체 | 445 WModel | 2,217 DDS | 2,662 |

## 계획서 규칙 반영

Git 제외 개인 규칙 `.md/GB/계획서하네스규칙.local.md`에 다음 세 축을 추가했다.

- 구현 계획서: 범위, 현재 실측, 변경 파일, 데이터/호출 흐름, G 순서, 검증을 소유한다.
- 디테일 계획서: include, enum/struct, member, 함수 계약과 필요한 전체 전문 코드를 소유한다.
- RESULT: 실제 구현 상태, 자동 검증, 수동 검증, 남은 경계를 분리한다.

단순히 `계획서 작성`이라고 요청하면 구현 계획서를 기본값으로 사용하고, 파일명은
`*_IMPLEMENTATION_PLAN.md`, `*_DETAIL_PLAN.md`, `*_RESULT.md`로 구분한다.

## 변경 파일

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `.md/GB/계획서하네스규칙.local.md`
- `.md/GB/08-06/2026-08-06_EFFECT_CLASS_RESOURCE_AUTHORING_IMPLEMENTATION_PLAN.md`
- `.md/GB/08-06/2026-08-06_EFFECT_CLASS_RESOURCE_AUTHORING_RESULT.md`

`Client.vcxproj/.filters`에는 병행 추출 세션이 생성한
`Data/Effects/Imported/DimensionMaster/Modules/skill.2050500.external-module-closure.json`을
`96.DataFiles/Effects/Imported`의 `None` 항목으로만 등록했다. 해당 추출 데이터의 내용은 수정하지
않았으며 ProjectAudit의 모든 Git Data 원본 노출 계약을 닫기 위한 project metadata 변경이다.

## 실행한 자동 검증

### Effect 전용 하네스

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/ProjectAudit/Test-EffectToolFinal.ps1 `
  -ResourceRoot Client/Bin/Resources
```

PASS:

```text
PASS: final Effect Tool bundle; code=50, documents=11, resources=4, palette=2662, cues=14.
```

하네스는 다섯 물리 domain이 모두 존재하며 각 domain에 WModel과 DDS가 하나 이상 존재하는지,
다섯 domain 아래 모든 WModel/DDS 2,662개가 catalog 대상인지, domain/category UI와 binding guard가
코드에 연결됐는지를 검사한다.

### ProjectAudit

첫 실행은 병행 추출 세션의 신규 Data 파일 1개가 project/filter에 미등록되어
`expected=213 project=212 filters=212`로 실패했다. 파일을 수정하지 않고 `None` 항목으로 등록한 뒤
다시 실행했다.

```text
Project audit passed: 69 checks.
```

### 정본 Debug 빌드와 회귀

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Debug
```

PASS:

- Engine x64 Debug build
- UpdateLib Debug와 runtime dependency 배포
- Shared, NetworkProtocolHarness, ClientFrontendHarness x64 Debug build와 실행
- Server x64 Debug build와 contract test, failures 0
- Client x64 Debug compile/link와 `Client.exe` 생성
- Effect final harness, `palette=2662`
- ProjectAudit 69 checks
- 최종 `Regression completed: Debug`

추가 정합성 검사:

- PowerShell parser: `Test-EffectToolFinal.ps1` PASS
- Client project/filter XML parse: 정본 Debug build와 ProjectAudit에서 PASS
- `git diff --check`: whitespace 오류 없음. 기존 파일의 LF/CRLF 변환 경고만 존재한다.

## 수동 검증으로 남긴 항목

이번 실행에서는 GUI를 자동 조작하지 않았다. 다음은 F1 Effect Tool에서 사용자가 눈으로 확인할
항목이며 자동 검증 PASS와 구분한다.

1. `Authoring Category`를 다섯 domain으로 전환할 때 다른 domain asset이 섞이지 않는지 확인한다.
2. Mesh Shape에서 WModel thumbnail, 다섯 Texture 슬롯에서 DDS thumbnail이 실제로 보이는지 확인한다.
3. 각 slot의 Bind Selected/Clear Slot 후 preview와 dirty 상태가 갱신되는지 확인한다.
4. Data Files category 전환, Authored load, Save/Reload 뒤 동일 binding이 유지되는지 확인한다.

렌더링 품질 고도화, 원본 material parameter 의미 자동 판정, Cascade 파싱값의 시각 튜닝은 이번
변경에 포함하지 않았다.
