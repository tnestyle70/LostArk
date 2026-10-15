# LostArk 코드 작성 스니펫

작성 기준은 [LostArk Engine 컨벤션](../../.md/TEAM/LOSTARK_ENGINE_CONVENTIONS.md)을 따른다.
이 폴더의 Visual Studio C++ 스니펫과 [VS Code 스니펫](../../.vscode/cpp.code-snippets)은
짧은 한국어 주석과 Allman 중괄호를 같은 형태로 제공한다. 새 Manager·singleton·익명 namespace를 생성하지 않는다.

| 단축어 | 사용 위치 |
|---|---|
| `la_guard` | 입력 검사와 이른 반환. 반환 타입에 맞는 실패 값을 직접 지정한다. |
| `la_step` | 목적이 달라지는 처리 묶음 앞. 주석과 실제 처리를 직접 채운다. |
| `la_function` | 호출 목적과 함수 본문. 현재 타입·이름·입력 계약을 사용한다. |
| `la_local` | 클래스와 무관한 CPP 전용 자유 함수. 헤더나 클래스 내부에는 사용하지 않는다. |
| `la_namespace` | Engine 또는 Client namespace 범위. 매크로 제공 헤더는 대상 파일이 직접 포함한다. |
| `la_vector` | `std::vector<원소형>` 선언. 기본 원소형은 `std::string`이며 초기값 위치에 마지막 커서를 둔다. |
| `la_array` | `std::array<원소형, 크기>` 선언. 기본 원소형은 `std::string`, 크기는 `3`이다. |

Visual Studio의 `도구 → 코드 조각 관리자`에서 언어 `Visual C++`를 골라 이 폴더를 추가하거나
`.snippet` 파일을 가져온다. 코드 편집기의 `코드 조각 삽입`에서 선택한다. 사용자 전역 스니펫을
덮어쓰거나 IDE를 자동 조작하지 않는다. VS Code는 이 저장소 폴더를 열면 프로젝트 스니펫을 읽는다.

스니펫의 기본 식별자는 작성 위치에 맞게 채워야 한다. 입력 검사 전에 상태를 이미 바꾸었다면
`기존 상태가 보존된다`고 쓰지 않는다. 반환값·정리·복구는 실제 호출 계약에 맞게 작성한다.
하나의 객체 상태를 다루는 함수는 해당 클래스의 private 멤버를 먼저 검토하며 la_local을 자동 적용하지 않는다.

`la_vector`·`la_array`는 CTAD로 원소형을 생략하지 않는다. 타입·변수 이름과 array의 크기를
작성 위치에 맞게 바꾸고 중괄호 안에 초기 원소를 넣는다. 초기값을 비워 두면 vector는 비어 있고,
array는 지정한 개수의 원소를 값 초기화한다. 기본 `std::string` 원소는 빈 문자열이 된다.
vector에는 `<vector>`, array에는 `<array>`, 기본 원소형에는 `<string>`을 직접 포함한다.
스니펫이 include를 자동 추가하지는 않는다. 같은 명시 규칙을 `std::pair<T, U>`·`std::optional<T>`
등에도 적용하며, 람다·반복자·구조적 바인딩의 `auto`까지 금지하지 않는다.

줄 배치는 루트 [.clang-format](../../.clang-format)이 담당한다. `InsertBraces: false`이므로
기존 한 줄 분기에 없는 중괄호를 자동으로 만들어 주지 않는다. 새 코드에는 스니펫처럼 직접 중괄호를 쓰고,
기존 코드의 중괄호 추가는 전처리·매크로·분기 범위를 확인한 변경으로 검토한다.

formatter는 include·using 순서를 재배열하거나 주석을 다시 쓰거나 문자열을 나누지 않도록 설정했다.
그렇더라도 사용자 메모, 의미 단위의 빈 줄, 전처리 분기와 매크로를 이해하지 못하므로 diff 확인이 필요하다.
저장 시 자동 formatter 실행이나 새 확장 설치를 이 스니펫이 요구하지 않는다.

기존 Engine/Client C++은 파일별 CP949·UTF-8·BOM·줄바꿈을 보존한다. 인코딩을 확신하지 못하면
전체 파일을 formatter로 덮어쓰지 않는다. 새 C++은 UTF-8 BOM 없이, Markdown은 UTF-8로 저장한다.
원본을 읽어 출력만 미리 보고 지정한 파일에 적용하며, 외부 소스·EngineSDK·Resources·생성물에는 일괄 적용하지 않는다.
설정 추가만으로 기존 모든 파일이 이 규격을 만족한다고 보거나 성능이 개선됐다고 기록하지 않는다.
