# LostArk Engine 코드 작성 규칙

이 문서는 LostArk의 Engine 코드와 이를 호출하는 Client 코드의 작성 기준이다.
현재 구현의 소유권과 호출 계약을 유지하면서, 새 코드와 이번에 고치는 코드를 같은 방식으로 읽을 수 있게 한다.
WintersEngine의 문서 구성은 참고하되 다른 프로젝트의 타입·상속·배포 정책은 가져오지 않는다.

작업 절차와 금지 경계는 [AGENTS.md](../../AGENTS.md), 프로젝트 구조·빌드·런타임은
[CLAUDE.md](../../CLAUDE.md), 계획서와 대화 출력 형식은 [GB/local.md](../GB/local.md)가 정본이다.
이 문서는 코드의 이름, 배치, 선언, 의존성과 소유권 선택을 다룬다.
기존 저장소 전체를 자동 변환하거나 외부 라이브러리의 코드를 이 규칙에 맞춰 고치지 않는다.

## 1. 책임을 먼저 정한다

| 위치 | 맡는 일 | 현재 예 |
|---|---|---|
| Engine | 게임 내용을 몰라도 사용할 수 있는 장치·자원·계산·진단 기능 | `CModel`, `CWireBoxRenderer`, `CCrashReport_Manager` |
| Client | LostArk의 Level, GameObject, 화면 정책과 파일 배치 | `CTrigger_Box`, `CClientWindowDisplay`, `Client::PrepareCrashReport` |
| `CGameInstance` | 실제로 소유하는 엔진 서브시스템의 준비·갱신·정리와 호출 중계 | `CPrototype_Manager`, `CObject_Manager`, `CRenderer` |

클래스 이름에 `Manager`가 있다는 이유로 `CGameInstance`에 등록하지 않는다.
누가 만들고, 누가 함께 쓰고, 언제 없어져야 하는지로 소유자를 정한다.
기존 경로가 같은 일을 하면 그 경로를 확장하고 두 번째 관리자나 렌더 제출 경로를 만들지 않는다.

- `CCrashReport_Manager`는 엔진 초기화 전과 정리 중에도 필요한 정적 기능이다.
  `Client::PrepareCrashReport`가 경로를 정하고 `Install`에 전달한다. `CGameInstance` 수명에 묶지 않는다.
- `CWireBoxRenderer`는 `CTrigger_Box` 원형과 복제본이 공유하는 렌더 자원이다.
  Client가 표시 여부와 색을 정하고 월드 OBB·색·view·projection을 전달한다.
  Engine은 DirectXTK 자원과 선 그리기만 맡는다. 별도 singleton이나 GameInstance 서브시스템을 만들지 않는다.

## 2. 파일과 공개 경계를 구분한다

| 경로 | 역할 |
|---|---|
| `Engine/Public/` | Engine 헤더 원본. Client SDK로 복사되므로 외부 소비자를 함께 확인한다. |
| `Engine/Private/` | 구현 CPP와 Engine 내부에서만 쓰는 보조 헤더 |
| `Client/Public/`, `Client/Private/` | Client 선언과 구현 |
| `EngineSDK/inc/` | `Engine/Public/**`에서 복사한 산출물. 직접 편집하지 않는다. |
| `Engine/Default/Engine.vcxproj`, `.filters` | Engine 빌드와 IDE 파일 등록 |

LostArk에는 WintersEngine의 `Include=SDK`, `Public=내부 전용` 구분을 적용하지 않는다.
실제 SDK 갱신자는 `Client/Default/Client.vcxproj`의 `PrepareEngineSdk`다.
이 target은 Engine의 `Public/**`를 하위 폴더 구조를 유지해 SDK에 복사한다.
헤더가 `Public`에 있다는 사실과 그 안의 모든 이름이 DLL export라는 뜻은 다르다.

신규 H/CPP는 실제 물리 폴더에 추가하고 해당 `.vcxproj`와 `.vcxproj.filters`에 필요한 항목만 등록한다.
기존 필터를 재배치하지 않는다. 공개 헤더 변경은 Engine만 확인하지 않고 SDK를 갱신하는 Client 빌드까지 확인한다.
빌드 명령과 배포 순서는 CLAUDE와 [Tools/Build/README.md](../../Tools/Build/README.md)를 따른다.

## 3. 이미 있는 타입과 이름을 사용한다

[Engine_Typedef.h](../../Engine/Public/Engine_Typedef.h)의 공통 별칭을 정본으로 삼는다.
별칭은 새 자료형이나 자동 변환 규칙이 아니라 원래 타입의 다른 이름이다.

| 목적 | LostArk 타입 | 원래 타입·주의점 |
|---|---|---|
| 실수·논리 값 | `f32_t`, `f64_t`, `bool_t` | `float`, `double`, `bool` |
| 문자 | `char_t`, `tchar_t` | `char`, `wchar_t`. `tchar_t`는 빌드 설정에 따라 바뀌는 `TCHAR`가 아니다. |
| 소유 wide 문자열 | `wstring_t` | `std::wstring` |
| 저장용 수학 값 | `float2_t`, `float3_t`, `float4_t`, `float4x4_t` | DirectX의 `XMFLOAT*` |
| SIMD 계산·인자 | `vector_t`, `fvector_t`, `gvector_t`, `hvector_t`, `cvector_t`, `matrix_t`, `fmatrix_t` | DirectXMath의 원래 값 타입과 인자 전달 타입 |
| 정수·길이 | `int32_t`, `uint32_t`, `size_t` 등 | 고정 폭과 컨테이너 크기의 의미에 맞춰 선택 |
| Windows API 계약 | `HWND`, `HANDLE`, `HRESULT`, `DWORD`, `wchar_t` 등 | API가 요구하는 타입을 드러낸다. |

LostArk에 없는 `i32_t`, `u32_t` 같은 별칭을 문서 예시만을 위해 추가하지 않는다.
엔진 도메인 값에는 기존 공통 별칭을 쓰되 표준·Win32 API 원형이나 override 계약을 억지로 바꾸지 않는다.
공개 헤더의 새 선언은 `std::shared_ptr`, `std::unique_ptr`, `std::string`처럼 출처를 명시한다.
기존 `using namespace std`, `DirectX`, `Microsoft::WRL`의 일괄 제거는 소비자 검증이 필요한 별도 변경이다.

### 컨테이너·값·잠금 선언의 템플릿 인자를 명시한다

새 코드와 이번에 정리하는 선언은 원소형과 고정 크기를 코드에서 바로 읽을 수 있게 쓴다.
`std::vector<T>`, `std::array<T, N>`, `std::map<Key, T>`, `std::pair<T, U>`,
`std::optional<T>`처럼 컨테이너·값 타입의 템플릿 인자를 명시한다. 초기값으로 클래스 템플릿의
인자를 추론하는 CTAD에 의존해 `std::vector values = { value };`처럼 생략하지 않는다.
잠금 객체도 `std::lock_guard<std::mutex>`·`std::unique_lock<std::mutex>`처럼 실제 mutex 타입을
명시한다. 다른 mutex를 보호하는 선언에는 그 실제 타입을 적고 잠금 범위와 수명은 유지한다.

```cpp
std::vector<LostArk::Shared::CHARACTER_CLASS_ID> selectedClasses = { m_ePreparedCharacterClass };
std::array<LostArk::Shared::CHARACTER_CLASS_ID, 1> fixedClasses = { m_ePreparedCharacterClass };
```

위 예제의 `{ m_ePreparedCharacterClass }`는 초기 원소 하나를 뜻한다. `vector`의 원소 수는
실행 중 바뀔 수 있고, `array`의 `1`은 타입에 포함된 고정 원소 수다. 기존 선언을 정리할 때는
컴파일러가 추론하던 실제 타입과 초기화 방식을 먼저 확인하며 원소형·개수·동작을 바꾸지 않는다.

이 규칙은 `auto` 전체 금지가 아니다. 람다, 반복자, 구조적 바인딩에는 사용할 수 있다.
기존 반복문·바인딩의 `auto`, `auto&`, `const auto&`를 정리할 때는 복사·차용과 const 의미를
보존한다. 다만 자료의 원소형·크기를 설명해야 하는 선언을 `auto`로 우회해 숨기지 않는다.
기존 저장소의 추론 선언을 일괄 치환하지 않고 현재 수정 범위에서 적용한다.
`la_vector`·`la_array` 스니펫도 타입 인자를 명시하며, 필요한 표준 헤더는 파일에 직접 포함한다.

멤버는 `m_` 뒤에 타입 성격과 역할을 함께 적는다. 접두사는 소유권을 대신 설명하지 않는다.

| 대상 | 이름 예 |
|---|---|
| 창·파일 handle | `m_hWnd`, `m_hLogFile` |
| 참·거짓 상태 | `m_bEngineReady`, `m_bPendingResize` |
| 개수·크기·인덱스 | `m_iPendingWidth`, `iMeshIndex` |
| 실수·벡터 | `m_fSpeed`, `m_vPosition` |
| raw·스마트 포인터 | `m_pContext`, `m_pWireBoxRenderer` |
| 문자열·문자 버퍼 | `m_strName`, `m_szTextBuffer` |
| 관련 값을 묶은 구조체 | `m_AppliedSettings`, `MonitorInfo` |

지역 변수와 매개변수에는 `m_`를 붙이지 않는다. `hMonitor`, `bWindowed`, `hrResult`처럼 역할을 읽게 한다.
`s_`는 C++ 문법이 아니라 일부 코드가 정적 상태를 표시하는 이름 관례다.
이 저장소의 클래스 상태에는 `static` 여부와 관계없이 `m_`를 사용한다.
타입 크기나 signedness는 선언이 정본이므로 이름만 보고 형 변환하지 않는다.

클래스는 기존 `C` 접두사, 파일은 `CrashReport_Manager.h`처럼 클래스 역할 이름을 따른다.
함수는 해당 클래스의 기존 `Initialize_Prototype`, `Set_Window`, `Render` 등 호출 체계를 유지한다.
이름 정리를 이유로 저장 ID, JSON 필드, override, 외부 공개 함수까지 일괄 바꾸지 않는다.

## 4. 한 처리 단위를 읽고 다음으로 넘어갈 수 있게 쓴다

- 함수·조건·반복문의 여는 중괄호는 다음 줄에 둔다. `if`, `else`, `for`, `while`은 한 문장이어도 중괄호를 쓴다.
- 검증, 조회, 계산, 적용처럼 목적이 바뀌면 빈 줄을 둔다. 선언·호출·반환을 한 줄로 압축하지 않는다.
- 처리 단위 바로 앞에 짧고 구체적인 한국어 주석을 둔다. 식별자를 한국어로 번역해 반복하지 않는다.
- 블록 목적은 짧은 구절로 적는다. 불필요한 `~한다` 종결이나 자료형 설명을 매번 반복하지 않는다.
  `//가디언나이트·창술사를 selectedClass에 중복 없이 추가`,
  `//AI 아바타 아이템에서 visual-set ID 수집`처럼 실제 처리 대상을 드러낸다.
  목록에 ID를 추가·수집하는 단계와 파일 로드·객체 스폰을 구분한다.
- 여러 API가 중첩되면 각 결과에 의미 있는 이름을 붙인다. 단순한 식까지 전부 임시 변수로 나누지는 않는다.
- 실패 코드, 차용 수명, 스레드와 재진입처럼 동작을 결정하는 사실은 필요한 위치에 남긴다.
  긴 설계 배경과 학습 설명은 PLAN·RESULT에 둔다.
- 사용자가 쓴 질문·학습 메모를 일괄 삭제하지 않는다. 정리할 때는 확인한 답을 짧은 주석으로 옮기고 취지를 보존한다.

다음은 `CClientWindowDisplay::Set_Window` 안의 처리 블록 형식이다. 함수 전체를 대체하는 코드가 아니다.

```cpp
//창이 없거나 요청 해상도가 허용 범위를 벗어나면 적용하지 않는다.
if (!m_hWnd || settings.width < 640 || settings.height < 480 ||
    settings.width > 16384 || settings.height > 16384)
{
    return Fail(status, "Display dimensions", E_INVALIDARG);
}

//현재 창의 모니터를 고른다. 화면 밖의 창은 가장 가까운 모니터를 사용한다.
const HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

//모니터의 전체 영역과 작업 영역을 받아 창 크기와 위치를 계산한다.
MONITORINFO MonitorInfo{};
MonitorInfo.cbSize = sizeof(MONITORINFO);

if (!GetMonitorInfoW(hMonitor, &MonitorInfo))
{
    const DWORD dwError = GetLastError();
    const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
    return Fail(status, "Read monitor", hrResult);
}
```

`GetLastError`는 실패 직후 저장한다. 읽기 좋게 나누는 과정에서 다른 API를 먼저 호출해 오류를 바꾸지 않는다.
창 변경 중 들어오는 메시지, 부분 적용 뒤 복원, Engine 준비 전후 분기는 줄 수를 줄이기 위해 제거하지 않는다.

## 5. 선언과 정의, 필요한 include를 나눈다

헤더에는 외부에서 사용할 계약과 그 계약을 수행하는 상태를 둔다.
호출자는 `public`, 내부 상태·보조 함수는 `private`, 실제 파생 클래스가 사용해야 하는 경계만 `protected`로 둔다.
단순히 CPP에 옮기기 위해 불필요한 setter·getter를 만들지 않는다.

- 포인터·참조 선언만 필요하고 해당 문맥에서 완전한 타입이 필요 없다면 전방 선언한다.
- 기반 클래스, 값 멤버, 멤버 접근을 하는 inline·템플릿 코드는 실제 정의가 필요하다. 해당 헤더를 직접 포함한다.
- `std::unique_ptr<T>`로 미완성 타입을 보관하면 삭제가 발생하는 소멸자 등을 완전한 타입이 있는 CPP에 정의한다.
- 외부 타입 전방 선언은 실제 namespace와 template 인자, SDK 선언과 일치해야 한다.
  예를 들어 DirectXTK의 `DirectX::DX11` inline namespace를 임의로 생략해 별개 타입을 만들지 않는다.
- 구현에서만 쓰는 Assimp·DirectXTK·FX11 등의 구체 헤더는 사용하는 CPP가 직접 포함한다.
  공개 입력 자체가 SDK 타입을 요구하면 필요한 정의를 헤더에 남긴다. 외부 include를 무조건 모두 없애지 않는다.

`CWireBoxRenderer`는 헤더에 Batch·Effect의 전방 선언과 `unique_ptr` 멤버를 두고,
CPP에 DirectXTK 헤더와 소멸자 정의를 둔다. Client의 `CTrigger_Box`는 Engine 렌더러를 참조하므로
DirectXTK의 생성·해제 API를 알 필요가 없다.

PCH와 다른 헤더의 우연한 include에 기대지 않는다. 프로젝트의 PCH는 안정적인 표준 헤더를 캐시하는 용도다.
`Engine_Defines.h`에 새 기능과 외부 라이브러리를 계속 추가해 의존성을 전체 프로젝트로 퍼뜨리지 않는다.
직접 include가 필요한 공용 타입은 `Engine_InitTypes.h`, `Engine_RenderTypes.h`,
`Engine_VertexTypes.h`, `Engine_AnimationTypes.h` 등 실제 선언 소유자를 선택한다.

### 공용 PCH와 직접 include

공용 입력은 `Tools/Build/PCH.h`다. Engine·Client·Shared·Server는 같은 헤더 목록을 읽지만
각 프로젝트의 `*_Pch.cpp`와 설정으로 별도의 `.pch`를 만든다. `Client_Pch.cpp`는 캐시를 만드는
번역 단위이며 새로운 `Client_Pch.h`에 의존성을 다시 모으지 않는다.

| 위치 | 넣는 것 |
|---|---|
| `PCH.h` | 자주 사용하고 안정적인 표준 라이브러리 헤더 |
| `Client_Defines.h`, `Engine_Defines.h` | 해당 프로젝트가 공유하는 기존 기초 계약과 플랫폼 include 순서 |
| 기능 H | 공개 선언·값 멤버·inline 구현에 실제 필요한 타입의 헤더 |
| 기능 CPP | 함수 본문에서만 필요한 API·구체 구현의 헤더 |

PCH에 들어 있어도 실제로 쓰는 표준 타입의 직접 include는 유지한다. PCH 사용 여부가 공개
헤더의 컴파일 가능성을 바꾸지 않게 하고, 헤더를 읽는 사람이 의존성을 확인할 수 있게 한다.
표준 헤더를 여러 파일에서 include한다고 매번 전체 내용을 재정의하거나 객체를 생성하는 것은 아니다.
PCH의 목적은 컴파일 입력 재사용이며 실행 중 메모리·스레드·게임 FPS를 관리하는 것이 아니다.

`<vector>`는 연속 저장 컨테이너, `<atomic>`은 개별 값의 원자 연산·메모리 순서,
`<mutex>`는 복수 상태를 함께 보호하는 잠금, `<functional>`은 함수 객체 계약을 제공한다.
`<cstdlib>`는 `std::getenv`, 숫자 변환과 C 계열 메모리/종료 함수 등을 선언한다.
`std::bad_alloc`의 직접 선언 헤더는 `<new>`다. atomic 하나를 사용한다고 vector와 callback
출력이 자동으로 스레드 안전해지지 않는다. 실제 잠금과 join 경계를 따로 설명한다.

PCH 확대는 사용 빈도와 선택한 MSVC의 실제 include 의존성을 확인한 뒤 결정한다.
이미 간접 포함된 헤더의 이름을 목록에 추가하는 것을 컴파일 성능 개선으로 기록하지 않는다.
CRT 진단 매크로와 WinSock2/Windows 순서, PCH를 끈 개별 컴파일도 함께 확인한다.

## 6. static, private, namespace, export는 서로 다른 선택이다

| 표기 | 결정하는 것 | 결정하지 않는 것 |
|---|---|---|
| `m_` | 사람이 읽는 이름 규칙 | 수명, 접근 권한, 동기화 |
| `private` | C++ 코드에서 누가 멤버에 접근할 수 있는지 | DLL 기호 숨김, 메모리 구역, 스레드 안전성 |
| 클래스 `static` 데이터 | 각 객체에 속하지 않고 클래스가 공유하는 저장 공간 | 객체 자동 생성, mutex, 내부 연결 |
| 클래스 `static` 함수 | 객체와 `this` 없이 호출 | 함수 본문의 inline 실행이나 스레드 안전성 |
| CPP의 익명 namespace | 그 번역 단위 안에서 사용하는 이름의 내부 연결 | 힙 할당이나 실행 시간 증가 |
| `inline` | 정의를 여러 번역 단위에 둘 때의 C++ 규칙 등 | 호출 지점에 반드시 코드를 펼치는 최적화 |
| `ENGINE_DLL` | Engine 빌드의 export와 소비자 빌드의 import | `public`·`private` 접근 권한 |

함께 관리할 상태는 클래스의 private 멤버로 드러낸다.
프로세스에서 하나만 공유하는 기존 상태라면 private static 멤버를 사용할 수 있다.
이름을 `s_`에서 `m_`로 맞추기 위해 singleton, `new`, 생성·해제 경로를 추가하지 않는다.

다음은 현재 진단 클래스의 선언과 정의를 구분한 예다.

```cpp
//헤더의 클래스 내부: 상태를 선언한다.
private:
    static bool m_bInstalled;
```

```cpp
//CPP의 Engine namespace 내부: 그 상태의 저장 공간을 한 번 정의한다.
bool CCrashReport_Manager::m_bInstalled = false;
```

CPP에서 정의해도 클래스의 private 멤버다. 함수 밖에 있다는 이유로 누구나 쓸 수 있는 전역 변수가 되지 않는다.
정적 저장 기간은 stack·heap의 객체 수명과 다르며, `.data`·`.bss` 같은 실제 배치는 빌드 산출물에서 확인한다.
특정 섹션에 놓였다는 사실을 소스 언어 규칙이나 성능 개선 근거로 일반화하지 않는다.

익명 namespace는 클래스와 무관한 CPP 전용 순수 보조 함수와 상수에 사용할 수 있다.
헤더에는 사용하지 않는다. 기존 익명 namespace를 전부 없애는 규칙도 두지 않는다.
동시 접근은 실제 스레드와 읽기·쓰기 관계를 보고 판단한다. `static`, `private`, `volatile`만으로 동기화되지 않는다.
현재 진단 콜백의 중복 기록 방지는 `Interlocked` 연산이 담당하며 `Install`은 시작 스레드에서 한 번 수행한다.

`ENGINE_DLL`은 [Engine_Macro.h](../../Engine/Public/Engine_Macro.h)에서 `ENGINE_EXPORTS`에 따라 바뀐다.
현재 Engine은 `CGameInstance`뿐 아니라 `CPrototype`, `CComponent`, `CModel` 등도 공개한다.
새 API는 DLL 밖에서 실제로 호출하거나 파생·생성·소멸해야 하는 경계를 확인해 export한다.
`CCrashReport_Manager`처럼 외부 호출이 `Install` 하나인 정적 기능은 그 함수만 export한다.

```cpp
//클래스 전체가 아닌 외부 진입 함수만 DLL로 공개한다.
static ENGINE_DLL bool Install(const wchar_t* logPath) noexcept;
```

private 상태와 콜백의 클래스 소속·정적 수명은 그대로 유지한다.
클래스 전체에 붙이면 private 멤버도 바이너리에 노출될 수 있으므로 접근 제어와 구별한다.
기존 class export를 좁히는 변경은 소비자 링크를 확인하는 별도 API 변경으로 다룬다.

### Engine·Client namespace 표기

새 코드와 이번에 정리하는 Engine·Client 소유 선언은 `NS_BEGIN(Engine)` 또는
`NS_BEGIN(Client)`로 열고 `NS_END`로 닫는다. 매크로 소유자는 `Engine_Macro.h`이며
필요한 기존 공용 헤더나 직접 include로 선언을 확보한다. 이 매크로만 필요하다고
Client의 Level/UI 정의 전체를 Engine에 가져오지 않는다.

CPP 정의를 같은 namespace로 묶을 때 `Client::CType::Function`은 안쪽에서
`CType::Function`으로 쓴다. 기존 namespace를 중첩해 `Client::Client`를 만들거나
extern/Win32 진입점까지 감싸지 않는다. qualified 정의도 C++에서 올바른 표기이며,
매크로 정비는 이름 lookup과 링크를 확인하는 스타일 변경이다.

CPP의 작은 익명 namespace는 그 파일만 사용하는 예산·도우미를 숨길 때 유지한다.
외부 라이브러리 namespace·inline namespace·전방 선언은 실제 선언과 일치시킨다.
검색에서 `namespace`가 발견됐다는 사실만으로 위반 판정하거나 일괄 치환하지 않는다.

### DirectXMath 저장과 계산

객체가 보관하는 위치·방향·행렬은 기존 `float3_t`, `float4_t`, `float4x4_t` 저장 타입을
사용하고, 계산할 때 `XMLoad → vector_t/matrix_t 연산 → XMStore`로 연결한다.
`std::vector<T>`는 컨테이너이고 `vector_t`는 SIMD 계산 값이므로 서로 다른 의미다.
SIMD를 사용한다고 여러 캐릭터의 갱신이 자동으로 한 번에 묶이지 않는다.

행벡터 규약의 RIGHT/UP/LOOK/POSITION, local/world/view/projection의 합성 순서,
방향 w=0·위치 w=1, 초·거리·degree/radian은 값을 만드는 곳과 소비하는 곳에서 확인한다.
XMLoad/XMStore는 자동 전치나 좌표계 변환이 아니다. 기존 shader 전달과 본·부모 합성을
보존하며, 저장 타입을 계산 타입으로 일괄 교체하거나 호출 규약·정렬·패킷 ABI를 바꾸지 않는다.
중복 계산·메모리 배치·일괄 처리 최적화는 비용과 수치 오차를 측정하는 별도 변경으로 다룬다.

## 7. 객체 종류에 맞춰 소유권을 고른다

LostArk의 현재 공통 기반은 `CPrototype`이다. `CComponent`와 `CGameObject`가 이를 상속한다.
`CBase`와 자체 `AddRef`·`Release`를 모든 객체에 붙이는 계층은 현재 Engine에 없다.
학교 코드나 다른 엔진의 수동 참조계수 규칙을 새로 끼워 넣지 않는다.

| 대상 | 현재 소유 방식과 선택 이유 |
|---|---|
| `CGameInstance`의 서브시스템 | `unique_ptr`. GameInstance가 생성·정리 순서를 책임지는 단일 소유자다. |
| 등록된 Prototype 원본 | `Create`의 `unique_ptr`를 `CPrototype_Manager`에 이동한다. 원본을 등록한 저장소가 소유한다. |
| Clone·Layer의 GameObject·GameObject의 Component | `shared_ptr`. 기존 런타임 경계가 같은 객체의 수명을 함께 유지한다. |
| Direct3D 장치·Context·버퍼·view | `ComPtr`. COM의 `AddRef`·`Release` 소유권을 관리한다. |
| 한 객체만 소유하는 내부 구현 자원 | `unique_ptr`. 예: Model의 Assimp Importer, WireBox의 Batch·Effect |
| 수명에 참여하지 않는 관찰 | 참조·raw pointer·차용 handle 또는 필요에 따른 `weak_ptr`. 소유자와 유효 기간을 명시한다. |
| 프로세스 종료까지 필요한 고정 진단 상태 | 클래스의 static 데이터. GameInstance보다 이른 설치·늦은 종료를 지원한다. |

`shared_ptr`는 편의상 모든 포인터에 붙이는 기본값이 아니다. 실제 공유 소유가 필요한지 확인한다.
`ComPtr`의 참조계수와 `shared_ptr`의 제어 블록은 다른 기능이다. 같은 자원을 이중으로 해제하지 않는다.
기존 `Safe_AddRef`·`Safe_Release`가 있다는 이유로 일반 C++ 객체에 COM 수명 규칙을 적용하지 않는다.
빌린 `HWND`·`ID3D11Device*` 등은 차용 자체가 잘못된 것이 아니다. 누가 닫고 해제하는지가 명확해야 한다.

`CModel`의 Clone은 메시·재질 참조를 공유하면서 본·애니메이션의 복제 상태를 따로 만든다.
Prototype 계열의 복사 생성자를 무조건 삭제하면 이 Clone 경로가 깨진다.
반대로 WireBox처럼 장치 자원을 하나의 객체가 소유하고 참조만 공유하면 객체 복사는 금지할 수 있다.
값 구조체는 값 복사가 자연스러울 수 있으므로 모든 클래스에 private 생성자와 factory를 강제하지 않는다.

factory가 필요한 객체는 지역 스마트 포인터에 먼저 만들고 초기화 성공 후 반환한다.
실패하면 임시 객체가 소유한 자원이 함께 해제되도록 구성한다. private 생성자 때문에
`make_unique`·`make_shared`를 사용할 수 없다면 클래스 내부의 `new`를 즉시 스마트 포인터로 감싸는 기존 방식도 유효하다.
다형적 기반을 통해 삭제하는 계층은 가상 소멸자를 유지한다.

## 8. GameInstance 호출과 상태 수명을 보존한다

현재 `DECLARE_SINGLETON(CGameInstance)`의 `Get()`은 포인터가 아닌 참조를 반환한다.

```cpp
//프레임 루프가 실제 client 크기를 렌더 대상으로 전달한다.
const HRESULT hrResult = Engine::CGameInstance::Get().Resize_Viewport(iWidth, iHeight);
```

`GetInstance()->`나 `Get()->`로 바꾸지 않는다. 함수 정적 객체의 생성 보장과
각 서비스의 동시 호출 안전성은 별개다. 새 manager를 모두 singleton으로 만들지 않는다.
중계 호출을 줄일 때는 실제 비용과 수명·스레드 경계를 측정한다. 프레임당 임의의 호출 횟수만으로
GameInstance 경계를 우회하거나 raw pointer를 장기 보관하는 규칙을 만들지 않는다.

한 번 위임하는 처리는 기존 GameInstance 중계 함수나 공개 getter를 호출 지점에서 사용한다.
같은 처리 안에서 같은 서비스를 반복 사용하면 지역 포인터로 한 번 받아 차용 관계를 드러낼 수 있다.
새 getter를 만들거나 모든 호출을 지역 변수로 바꾸는 규칙은 아니다.

```cpp
//이번 로딩 처리에서 사용할 Profiler 차용
Engine::CProfiler* const pProfiler = Engine::CGameInstance::Get().Get_Profiler();
Engine::CProfilerScope levelScope(pProfiler, "Loader.LevelLoad");
```

`CProfiler* const`는 초기화한 포인터 변수에 다른 주소를 다시 대입하지 못하게 한다.
가리키는 Profiler의 상태는 바꿀 수 있으므로 측정 구간을 기록하는 함수를 호출할 수 있다.
`const CProfiler*`는 반대로 주소 재대입은 허용하지만 이 포인터를 통한 대상 변경과 비-const
멤버 함수 호출을 제한한다. 별표는 타입에 붙인 `CProfiler* const pProfiler` 표기를 사용한다.
`CProfiler *const pProfiler`도 문법상 같은 뜻이다. 주소를 유지할 지역 차용에 맞춰 선택하며
모든 포인터에 `const`를 강제하거나 기존 getter의 반환형을 일괄 바꾸지 않는다.

`Get_Profiler()`는 GameInstance가 `unique_ptr`로 소유하는 객체의 `.get()`을 반환한다.
차용한 포인터를 `delete`하거나 소유 스마트 포인터의 생성자·`reset`에 넘기지 않는다.
이 포인터와 `CProfilerScope`는 객체 수명을 늘리지 않으며, `const`도 수명을 보장하지 않는다.
현재 종료 경로는 Loading worker의 합류를 포함한 LevelManager 정리를 먼저 끝내고 Profiler를
해제한다. 멤버·전역·비동기 callback에 주소를 오래 보관하려면 소유자의 해제·교체와 작업 종료를
맞추는 별도 수명 계약이 필요하다.

null 허용 여부는 각 서비스와 소비자의 계약으로 확인한다. 현재 `CProfilerScope`는 `nullptr`을
받으면 측정을 생략하지만, 포인터로 멤버 함수를 직접 호출하려면 유효한 대상이 필요하다.
CPU scope의 시작·종료는 같은 스레드에서 수행한다. 이 경로는 `thread_local` 중첩 상태와 공유
기록의 mutex로 worker 기록을 지원하며, GPU scope는 주 스레드 전용이다. 다른 매니저의 동시
접근 가능 여부는 해당 메서드의 계약을 따로 확인한다.

진단 모듈은 현재 `terminate`와 `SIGABRT` 기록을 맡는다. 모든 Windows 예외를 잡거나 복구하는 기능은 아니다.
고정 버퍼와 미리 연 파일은 치명적 종료 시 추가 자원 준비를 줄이기 위한 선택이다.
static으로 옮겼다는 사실만으로 종료 중 기록 성공을 보장하지 않는다. 예외·버퍼·파일의 실제 실패 경계를 설명한다.
기록 설치 실패는 Client의 시작 자체를 중단시키지 않는 현재 정책을 유지한다.

## 9. 키워드와 디버그 진단을 분리한다

C++ 키워드 `new`는 매크로로 재정의하지 않는다. 공용 헤더에 `#define new DBG_NEW`를 다시 넣거나
이를 피하기 위해 각 CPP에 `push_macro("new")`·`undef new`·`pop_macro("new")`를 반복하지 않는다.
외부 SDK가 자체적으로 가진 보호 코드는 소유 범위가 다르므로 이 규칙으로 일괄 편집하지 않는다.

현재 MSVC Debug CRT의 표준 `new`도 할당 번호·누수 보고·`_CrtSetBreakAlloc` 대상이다.
자동 `__FILE__`·`__LINE__` 주입을 없애면 해당 할당의 파일·행 정보는 줄어든다.
`_CRTDBG_MAP_ALLOC`의 C 할당 함수 매핑과 기존 CRT 진단 호출은 다른 기능이므로 보존한다.
진단을 개선하려고 언어 키워드의 의미나 외부 SDK 헤더의 해석을 전역에서 바꾸지 않는다.

## 10. 정리의 범위와 완료를 확인한다

기존 C++은 파일별 인코딩·BOM·개행을 확인하고 유지한다. 새 C++은 UTF-8 BOM 없음,
Markdown은 UTF-8로 작성한다. 현재 Engine 파일은 인코딩이 혼재하므로 폴더 단위로 추정하지 않는다.

수정 전 직접 호출자·소비자와 기존 PLAN·RESULT를 확인한다. 미사용 함수·변수·include는 검색한 뒤 제거한다.
주석과 중괄호 정리에 실패 검사, rollback, 재진입 방지, 렌더 제출 순서 변경을 섞지 않는다.
동작도 바꿔야 한다면 그 이유와 검증을 별도 변경 항목으로 드러낸다.

변경한 소스의 컴파일, 공개 API의 소비자 링크, 필요한 프로젝트 등록과 `git diff --check`를 확인한다.
빌드 성공과 사용자의 실제 화면 확인은 구분한다. 실행하지 않은 확인을 RESULT에 성공으로 적지 않는다.
이름·namespace·주석을 정리한 것을 성능 병목 제거라고 부르지 않는다. 성능 주장은 측정한 비용과 범위로 설명한다.

## 11. 같은 규칙을 편집 도구에서도 사용한다

줄 배치는 루트 [.clang-format](../../.clang-format), 입력 검사·함수·처리 블록은
[CodeStudy/Snippets](../../CodeStudy/Snippets/README.md)의 Visual Studio 스니펫과
[VS Code 스니펫](../../.vscode/cpp.code-snippets)을 사용한다. 빈 manager·singleton을 자동 생성하지 않는다.
새 코드는 네 칸 들여쓰기와 Allman을 사용한다. 기존 파일 전체의 탭·공백을 같이 바꾸지 않는다.

현재 설정은 설치된 clang-format 22.1.3으로 확인했다. include·using 순서, 문자열과 주석을 다시 쓰지
않도록 설정했으며, InsertBraces는 false다. 없는 중괄호를 자동 삽입하지 않으므로 기존 분기의 괄호 추가는
직접 검토한다. formatter는 인코딩과 동작 보존을 보장하지 않는다. 사본과 diff를 확인하고 지정한 파일만 적용한다.
프로젝트 설정에 저장 시 자동 실행이나 charset 일괄 변환을 추가하지 않는다. 구형 formatter의 옵션 호환성은 별도 확인한다.
## 근거가 되는 현재 코드

- 타입·매크로: [Engine_Typedef.h](../../Engine/Public/Engine_Typedef.h), [Engine_Macro.h](../../Engine/Public/Engine_Macro.h)
- 기반·복제: [Prototype.h](../../Engine/Public/Prototype.h), [Component.h](../../Engine/Public/Component.h), [Prototype_Manager.h](../../Engine/Public/Prototype_Manager.h)
- 객체 소유: [GameInstance.h](../../Engine/Public/GameInstance.h), [GameObject.h](../../Engine/Public/GameObject.h), [Layer.h](../../Engine/Public/Layer.h), [Model.cpp](../../Engine/Private/Model.cpp)
- 독립 진단: [CrashReport_Manager.h](../../Engine/Public/CrashReport_Manager.h), [ClientDiagnostics.cpp](../../Client/Private/ClientDiagnostics.cpp)
- 렌더 자원 공유: [WireBoxRenderer.h](../../Engine/Public/WireBoxRenderer.h), [Trigger_Box.cpp](../../Client/Private/Trigger_Box.cpp)
- SDK 갱신: [Client.vcxproj](../../Client/Default/Client.vcxproj)의 `PrepareEngineSdk`
