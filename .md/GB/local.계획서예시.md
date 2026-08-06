# LostArk G별 직접 구현 설명 예시

이 문서는 사용자가 코드를 직접 이해하고 작성할 때 사용할 대화 설명 형식의 개인 예시다.
실제 구현 정본은 날짜별 `*_PLAN.md`, 실제 완료 증거는 `*_RESULT.md`에 둔다.

## 1. 대화 설명과 PLAN의 역할

```text
대화 설명
  G의 목표와 종료 증거
  파일이 존재하는 이유
  H의 include, enum, struct, 함수, 멤버 변수와 불변식
  CPP 함수별 한 줄 책임과 호출 흐름
  사용자가 직접 작성할 순서
  작성 뒤 이해 확인과 검증

PLAN
  사용자가 그대로 대조할 수 있는 생략 없는 전체 반영 코드
  새 파일 전체
  기존 파일의 교체 가능한 함수·블록 전체
  vcxproj와 filters 등록
  Audit/harness와 실행 명령

RESULT
  실제 diff
  실행한 자동 검증
  실행한 수동 검증
  아직 검증하지 않은 항목
```

대화에서 전체 코드를 먼저 복사하지 않는다. 먼저 H/CPP/변수/호출 흐름을 이해하고 사용자가 작성한다.
단, 대응 PLAN에는 항상 전체 코드를 싣는다. 사용자가 `전체 코드`, `정답 코드`, `코드 전문`을 명시하면
대화에서도 전체 코드를 보여준다.

## 2. G 설명 고정 순서

매 G는 다음 순서로 설명한다.

1. 이번 G의 목표와 종료 증거
2. 수정할 파일과 파일이 존재하는 이유
3. H 계약과 include 이유
4. enum과 struct의 의미
5. 멤버 변수의 owner, 수명, 저장 여부, 불변식
6. CPP 함수별 한 줄 책임
7. 함수의 호출자, 입력 검증, 읽는 상태, 변경하는 상태, 성공·실패 결과
8. 사용자가 작성할 순서
9. diff, build, audit, runtime 검증
10. 사용자의 말로 이해 확인

## 3. G1 예시 — 메모리 Effect Document

### 3.1 목표

안정적인 EffectAssetId와 표시 이름을 가진 Effect authoring document 한 건을 메모리에 생성하고
명시적으로 폐기한다.

```text
입력
  Effect Asset ID = dimensionmaster.altv.portal_open
  Display Name    = DimensionMaster Alt-V Portal

Create Document
  -> 메모리 Active Document 생성
  -> Format Version 1
  -> Elements 0

Discard Active Document
  -> 메모리 document 제거
  -> 파일/GPU/Animation 변화 없음
```

### 3.2 용어

`EffectDocument`는 ImGui panel이 아니다. `CEffect_Tool`이 직접 소유하는 C++ 데이터 객체다.
ImGui panel은 매 프레임 그 객체를 읽어 텍스트와 버튼으로 보여준다.

```text
std::nullopt
  Active Document 없음
  New Document 입력 panel 표시

EFFECT_DOCUMENT_DESC 존재
  Active Document 있음
  version/id/name/element count panel 표시
```

EffectDocument 한 건의 단위는 재사용 가능한 시각 EffectAsset 하나다.

```text
dimensionmaster.altv.portal_open
  이후 G에서 Mesh/Sprite/Particle/Decal/Trail Element 여러 개를 소유
```

Animation clip, cue time, Character weapon anchor는 이 문서가 소유하지 않는다.

### 3.3 Effect_AuthoringDocument.h

파일 존재 이유:

> ImGui, D3D, Character와 분리된 Effect 저작 데이터의 언어를 선언한다.

주요 선언:

```text
EFFECT_AUTHORING_FORMAT_VERSION
  schema version. 변경 불가능한 inline constexpr 값.

EFFECT_ELEMENT_KIND
  MESH, SPRITE, PARTICLE, DECAL, TRAIL.
  END는 미초기화/반복 종료용이며 실제 저장 kind가 아니다.

EFFECT_ELEMENT_DESC
  strElementId: document 내부 stable ID.
  eKind: element 종류. 기본 END이며 commit 전에 실제 종류를 대입한다.
  strResourceId: Resources-relative ID. 절대 물리 경로가 아니다.

EFFECT_DOCUMENT_DESC
  iFormatVersion: document schema version.
  strEffectAssetId: catalog/cue가 참조할 stable ID.
  strDisplayName: 사람이 읽는 표시 이름.
  Elements: document가 강하게 소유하는 element vector.
```

이 헤더에는 ImGui, GPU pointer, RenderTarget, Character pointer, Preview particle을 넣지 않는다.

### 3.4 Effect_Tool.h

파일 존재 이유:

> ImGui session state와 현재 Active EffectDocument의 수명을 소유하고 외부에는 `Render()`만 공개한다.

멤버 변수:

```text
m_eSelectedEffectType
  다음에 추가할 Element 종류인 session state.
  radio를 눌러도 document는 변하지 않는다.

m_NewAssetId[129]
  ImGui InputText가 직접 편집하는 mutable char buffer.
  stable ID 최대 128 byte + null 종료 문자.

m_NewDisplayName[65]
  Display Name 입력 buffer. 최대 64 byte + null 종료 문자.

m_ActiveDocument
  optional 값으로 no-document/active-document 수명을 명시한다.
  pointer나 vector index가 아니며 CEffect_Tool이 직접 소유한다.

m_strDocumentStatus
  마지막 성공/실패 메시지. 저장 데이터가 아닌 session feedback.
```

private 함수:

```text
Render_NewDocumentPanel
  active document가 없을 때 입력창과 Create 버튼 표시.

Render_ActiveDocumentPanel
  active document의 version/id/name/element count를 읽기 전용 표시.

Render_EffectTypeSelector
  다음 Element 종류만 선택. document 변경 없음.

Try_CreateDocument
  입력 buffer 검증 -> local staging document 작성 -> optional에 commit.

Discard_ActiveDocument
  memory document만 제거. 파일과 GPU resource는 변경하지 않음.
```

### 3.5 Effect_Tool.cpp 호출 흐름

```text
CMainApp
-> CEffect_Tool::Render
-> ImGui::Begin
-> m_ActiveDocument 확인
   -> 없음: Render_NewDocumentPanel
   -> 있음: Render_ActiveDocumentPanel
-> Render_EffectTypeSelector
-> ImGui::End
```

`Try_CreateDocument()` 흐름:

```text
호출자
  Render_NewDocumentPanel의 Create Document 버튼

입력
  m_NewAssetId
  m_NewDisplayName

검증
  active document 중복 생성 금지
  EffectAssetId 1~128 ASCII stable ID
  Display Name 공백-only 금지

stage
  local EFFECT_DOCUMENT_DESC StagedDocument 생성
  ID와 Display Name 대입

commit
  모든 검증 성공 뒤 m_ActiveDocument에 한 번 대입

성공
  input buffer 초기화
  성공 status 표시

실패
  staging 값 폐기
  기존 active document와 입력 buffer 보존
  실패 status 표시
```

이것이 `validate -> stage -> commit`의 가장 작은 예다. Load에서는 앞에 parse가 추가되고,
GPU Preview에서는 stage에 임시 resource 생성이 추가된다.

### 3.6 사용자가 작성할 순서

1. data-only authoring header
2. Tool H의 session state와 optional document
3. CPP의 stable ID/display-name validator
4. New Document panel
5. Active Document panel
6. selector
7. staged create와 discard
8. project/filter 등록
9. Audit
10. Debug build와 실제 F1 runtime 확인

### 3.7 종료 증거

```text
빈 ID, slash 포함 ID, 129자 ID 거부
공백-only Display Name 거부
정상 ID/name 생성 성공
Active Document에 version=1, Elements=0 표시
radio 선택으로 document가 변하지 않음
Discard 뒤 New Document panel 복귀
Data/Resources/Animation 파일 변화 없음
```

## 4. PLAN 전체 코드 규칙

각 G를 설명하기 전에 대응 `*_PLAN.md`를 갱신한다.

- 새 H/CPP는 파일 처음부터 끝까지 전체 코드를 싣는다.
- C++ 학습 주석은 한국어로 작성하고 선언이나 문장 위의 독립된 줄에 둔다. `// 설명  실제 코드`처럼 코드와 같은 줄에 합치지 않는다.
- 새 C++ 파일은 UTF-8 BOM 없음 규칙을 유지하고, 한글 주석을 넣은 뒤에는 실제 Client compile/link로 주석이 코드를 삼키지 않는지 확인한다.
- 기존 파일은 바뀌는 함수나 블록 전체를 싣는다.
- include, enum, struct, 함수 선언, 멤버 변수를 생략하지 않는다.
- `...`, 의사 코드, 나중에 구현 표식을 사용하지 않는다.
- 새 C++ 파일은 `.vcxproj`와 `.vcxproj.filters`의 정확한 등록 XML을 싣는다.
- Audit/harness가 바뀌면 교체 가능한 전체 블록을 싣는다.
- build와 runtime 실행 절차, 성공 화면과 실패 입력을 기록한다.

현재 Effect Tool G1의 전체 코드 정본은 다음 문서에 있다.

```text
.md/GB/08-03/2026-08-03_EFFECT_TOOL_ADVANCEMENT_PLAN.md
```

대화 설명과 PLAN의 전체 코드는 같은 계약을 가리켜야 한다. 실제 코드가 달라지면 코드를 먼저 확인하고
PLAN을 같은 변경에서 교정한다.
