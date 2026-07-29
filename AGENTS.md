# AGENTS.md

LostArk 팀 저장소에서 사용하는 공통 작업 규칙이다.
프로젝트 구조와 빌드 설명은 `CLAUDE.md`, 구현 계획서 형식은 `.md/계획서작성규칙.md`를 따른다.

## 작업 시작 시 읽을 문서

| 작업 | 추가로 읽을 문서 |
|---|---|
| 빌드, Prototype/Clone, Level/Layer, Binary Asset, 리소스 배포 | `CLAUDE.md` |
| 계획서, 설계서, 구현 가이드 | `.md/계획서작성규칙.md` |
| 기존 작업 재개 | `.md/GB/<MM-DD>/`의 대응 `*_PLAN.md`, `*_RESULT.md` |
| LostArk 맵 에셋 검색, UModel 추출, ModelAssetConverter, MapTool 적용 | `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md` |
| 맵 에셋이 생성됐지만 안 보임, diffuse 누락, 스케일 오류, 레거시 런타임 혼선 | `.md/GB/07-29/gotchas.md` |

## 계획서 규칙

- 기본 위치는 `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_PLAN.md`이다.
- 같은 작업의 계획서가 있으면 새 파일을 만들지 않고 기존 문서를 갱신한다.
- 계획서는 다음 순서만 사용한다.
  1. C1~C8 관점
  2. 문제 해결 ①~⑤
  3. 자료구조·알고리즘 핵심
  4. 추가·수정·삭제 파일 목록
  5. 파일별 전체 구현 코드
  6. 프로젝트 등록과 검증
- 새 파일은 처음부터 끝까지 전체 내용을 쓴다.
- 기존 파일은 변경되는 선언, 함수, 데이터 블록의 최종 교체 코드를 전부 쓴다. 일부 문장이나 의사 코드로 대체하지 않는다.
- 새 C++ 파일마다 `.vcxproj`와 `.vcxproj.filters`에 추가할 정확한 XML을 함께 쓴다.
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
