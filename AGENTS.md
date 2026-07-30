# AGENTS.md

LostArk 팀 저장소에서 사용하는 공통 작업 규칙이다.
프로젝트 구조와 빌드 설명은 `CLAUDE.md`를 따른다. 구현 계획서의 형식과 문체는
아래 `계획서 규칙`의 선택적 규칙 파일 탐색 순서로 결정한다.

## 작업 시작 시 읽을 문서

| 작업 | 추가로 읽을 문서 |
|---|---|
| 빌드, Prototype/Clone, Level/Layer, Binary Asset, 리소스 배포 | `CLAUDE.md` |
| 계획서, 설계서, 구현 가이드 | 아래 `계획서 규칙`에서 발견한 첫 번째 규칙 파일 |
| 기존 작업 재개 | `.md/GB/<MM-DD>/`의 대응 `*_PLAN.md`, `*_RESULT.md` |
| LostArk 맵 에셋 검색, UModel 추출, ModelAssetConverter, MapTool 적용 | `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md` |
| 맵 에셋이 생성됐지만 안 보임, diffuse 누락, 스케일 오류, 레거시 런타임 혼선 | `.md/GB/07-29/gotchas.md` |

## 계획서 규칙

### 규칙 파일 탐색 순서

계획서, 설계서, 구현 가이드를 작성할 때 다음 순서로 규칙 파일을 찾는다.

1. `.md/계획서작성규칙.local.md`: 개인 작업 스타일. Git에 커밋하지 않는다.
2. `.md/계획서작성규칙.md`: 팀이 합의했을 때만 두는 선택적 공유 규칙.

- 먼저 발견한 파일 하나를 처음부터 끝까지 읽고 그 형식과 문체를 따른다.
- 두 파일이 모두 없으면 누락을 오류로 처리하거나 작업을 멈추지 않는다. 현재 코드와
  요청에 맞는 합리적인 형식으로 계획서를 작성한다.
- 규칙 파일이 없다는 이유만으로 새 규칙 파일을 자동 생성하거나 사용자에게 질문하지 않는다.
- 개인/공유 규칙은 문서의 형식과 문체만 재정의한다. 이 `AGENTS.md`의 팀 작업 경계,
  구현 원칙, 빌드·검증 규칙은 항상 우선한다.

### 공통 최소 계약

- 기본 위치는 `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_PLAN.md`이다.
- 같은 작업의 계획서가 있으면 새 파일을 만들지 않고 기존 문서를 갱신한다.
- 문서의 섹션 순서, 상세도, 전체 코드 포함 여부는 발견한 규칙 파일을 따른다. 규칙
  파일이 없으면 작업 규모와 독자가 검증하기 쉬운 형태를 작성자가 선택한다.
- 새 C++ 파일을 제안하면 `.vcxproj`와 `.vcxproj.filters` 등록 필요 여부와 검증 방법을
  빠뜨리지 않는다.
- 불확실한 내용은 계획서 안에 표식으로 미루지 않는다. 먼저 실제 코드와 데이터를 조사하고, 그래도 사용자 결정이 필요하면 계획서 작성 전에 질문한다.

## 팀 작업 경계

- `main`은 정본이며 기능 작업은 별도 브랜치와 PR을 사용한다.
- 다른 팀원의 미커밋 변경과 무관한 파일을 되돌리거나 정리하지 않는다.
- 변경한 모든 줄은 현재 요청과 직접 연결되어야 한다.
- `Engine/`, `Client/` C++ 파일은 기존 인코딩을 유지하고 Markdown 문서는 UTF-8로 저장한다.
- 물리 폴더가 소스 구조의 정본이다. `.vcxproj`와 `.filters`는 필요한 항목만 추가하고 기존 필터를 재배치하지 않는다.
- 빌드 산출물, `EngineSDK`, `.vs`, `imgui.ini`는 소스 커밋에 섞지 않는다.

## 구현 원칙

- 추측보다 현재 코드와 데이터 실측을 우선한다.
- 기존 Prototype/Clone/Layer/CModel 경로를 확장하고 같은 역할의 두 번째 런타임 경로를 만들지 않는다.
- `CCookedModel`과 `CBinaryAssetObject`는 레거시 검증 경로다. 신규 기능과 MapTool 에셋은 반드시 `CModel -> CMaterial` 통합 경로를 사용한다.
- Engine은 범용 기능, Client는 LostArk의 Level, GameObject, 에디터 흐름과 Scene 데이터를 소유한다.
- ImGui는 선택과 명령을 전달하는 UI다. 매 프레임 파일을 읽거나 모델을 다시 디코드하지 않는다.
- Catalog는 생성 가능한 정의, placement 문서는 배치 인스턴스의 ID와 Transform을 소유한다.
- 안정적인 asset ID와 placement ID를 저장 계약으로 사용한다. Prototype tag, 포인터, vector index는 저장 ID가 아니다.
- 로드는 `parse -> validate -> stage -> commit` 순서로 처리하고 실패하면 생성 중인 객체를 전부 rollback한다.

## 빌드·검증

```text
1. Engine x64 Debug/Release
2. UpdateLib.bat Debug/Release
3. Client x64 Debug/Release
```

- Engine public header를 바꿨다면 `UpdateLib.bat` 뒤 Client까지 검증한다.
- 실행 중인 `Client.exe`가 출력물을 점유하면 종료한 뒤 다시 링크한다.
- 에디터 기능은 빌드만으로 끝내지 않고 실행 레벨, 단축키, 생성, 저장, 재로드, 실패 시 기존 상태 보존까지 확인한다.
