# 어떤 코드베이스에서도 길을 잃지 않는 제1원리 핸드북

작성일: 2026-07-30  
기준 저장소: `C:/Users/user/Desktop/LostArk`  
비교 저장소: `C:/Users/user/Desktop/Winters`, `C:/Users/user/Desktop/UnrealEngine/UnrealEngine`

---

## 0. 이 문서의 목표

이 문서는 C++ 문법 목록이 아니다. 처음 보는 회사 프로젝트, 게임 엔진, 서버, 클라이언트,
툴, 백엔드 저장소를 마주쳤을 때 다음 질문에 스스로 답하기 위한 기준서다.

1. 이 프로그램은 무엇을 만드는가?
2. 실행은 어디에서 시작되는가?
3. 권위 있는 데이터는 어디에 있는가?
4. 누가 그 데이터를 소유하고 언제 파괴하는가?
5. 어떤 함수가 그 데이터를 바꾸는가?
6. 반환값과 부작용은 다음 어디로 전달되는가?
7. 어떤 스레드와 주기로 실행되는가?
8. CPU, GPU, 파일, 네트워크 중 어느 경계를 넘는가?
9. 시간과 메모리 비용은 얼마인가?
10. 성공과 실패를 무엇으로 검증하는가?

소스 파일을 전부 외우는 것이 목표가 아니다. 소스는 계속 바뀐다. 대신 어떤 저장소에도 적용되는
**해석 좌표계**를 머릿속에 만든다.

### 0.1 읽는 경로

3천 줄을 처음부터 한 번에 암기하지 않는다. 목적에 맞춰 왕복한다.

```text
오늘 LostArk 코드를 읽어야 한다
-> 1 -> 2 -> 10 -> 22 -> 31 -> 33~36

C++ 기초와 수명이 약하다
-> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 11 -> 12 -> 13 -> 28

자료구조·알고리즘 판단이 약하다
-> 14 -> 15 -> 16 -> 17 -> 18 -> 35

렌더링·시스템이 약하다
-> 19 -> 20 -> 30

회사형 구조를 읽고 싶다
-> 3 -> 21 -> 23 -> 24 -> 25 -> 26 -> 27
```

각 장을 읽은 뒤 반드시 현재 코드에서 한 파일, 한 함수, 한 자료구조를 찾아 분석 양식을 직접
채운다. 설명을 읽고 고개를 끄덕이는 것과 독립적으로 추적하는 것은 다른 능력이다.

---

## 1. 가장 중요한 깨달음: 프로그램은 여섯 가지다

```text
Program
= Data
+ Transformation
+ Ownership/Lifetime
+ Schedule
+ Boundary
+ Evidence
```

### 1.1 Data: 무엇을 기억하는가

- 플레이어 HP, 위치, 상태
- 모델의 정점, 인덱스, 재질, Bone, Animation
- 서버의 세션, 패킷, 월드 상태
- 에디터의 선택 항목과 배치 데이터
- 파일에 저장된 설정, Catalog, Scene

### 1.2 Transformation: 어떻게 바꾸는가

- 입력을 이동 명령으로 바꾼다.
- DeltaTime으로 애니메이션 시간을 증가시킨다.
- KeyFrame을 보간해 Bone 행렬을 만든다.
- 월드 행렬을 Clip 좌표로 바꾼다.
- 네트워크 바이트를 명령 구조체로 역직렬화한다.

### 1.3 Ownership/Lifetime: 누가 가지고 언제 버리는가

- 값 멤버인가?
- `unique_ptr`의 단독 소유인가?
- `shared_ptr`의 공동 소유인가?
- raw pointer/reference로 빌려 보기만 하는가?
- Level 종료 때 사라지는가, 프로세스 종료까지 사는가?

### 1.4 Schedule: 언제 실행되는가

- 프로그램 시작 시 한 번
- Level 로드 시 한 번
- 객체 Clone 시 한 번
- 매 프레임 60회
- 서버 Tick 30회
- 이벤트가 발생했을 때만
- 작업 스레드에서 비동기로

### 1.5 Boundary: 어디를 넘어가는가

- `.h` 공개 API와 `.cpp` 구현 경계
- Engine DLL과 Client EXE 경계
- Client와 Server 네트워크 경계
- CPU와 GPU 경계
- Editor와 Runtime 경계
- Authoring 파일과 Cooked binary 경계
- 프로세스와 Windows Kernel 경계

### 1.6 Evidence: 맞다는 것을 어떻게 아는가

- 컴파일과 링크 성공
- 실행 로그
- 디버거의 Call Stack과 Watch
- 렌더 캡처
- 프로파일러의 CPU/GPU 시간
- 저장 후 재로드 결과
- 서버의 결정론 Replay
- 테스트와 CI

이 여섯 축을 찾으면 언어와 엔진이 달라져도 구조는 보인다.

---

## 2. 처음 보는 저장소의 첫 30분

### 2.1 1단계: 제품과 빌드 그래프를 찾는다

가장 먼저 폴더 안쪽의 임의 클래스부터 읽지 않는다. 루트에서 다음을 찾는다.

```text
README / AGENTS / CLAUDE / CONTRIBUTING
*.sln / *.vcxproj / CMakeLists.txt / *.Build.cs
package.json / pom.xml / build.gradle
*.bat / *.ps1 / *.sh
.github/workflows / Jenkinsfile
```

질문:

- 최종 산출물은 EXE, DLL, 서버, 툴 중 무엇인가?
- 프로젝트 사이 의존 방향은 무엇인가?
- Debug/Release 차이는 무엇인가?
- 생성된 파일과 Git 정본은 무엇인가?

LostArk의 실제 빌드 그래프:

```text
Engine source
-> Engine.dll + Engine.lib
-> UpdateLib.bat
-> EngineSDK/inc + EngineSDK/lib + Client/Bin/Engine.dll
-> Client source
-> Client.exe
```

확인 파일:

- [Framework.sln](C:/Users/user/Desktop/LostArk/Framework.sln)
- [Engine.vcxproj](C:/Users/user/Desktop/LostArk/Engine/Default/Engine.vcxproj)
- [Client.vcxproj](C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj)
- [UpdateLib.bat](C:/Users/user/Desktop/LostArk/UpdateLib.bat)

### 2.2 2단계: 실행 진입점을 찾는다

언어별 흔한 진입점:

```text
C/C++ Windows GUI  wWinMain / WinMain
C/C++ Console      main
C#                 static Main
Unreal             Target/Module + Engine bootstrap
Unity              엔진이 MonoBehaviour 이벤트를 호출
Server             main -> listen -> tick/event loop
Web backend        main/app bootstrap -> route registration -> listen
```

LostArk 진입점은 [Client.cpp](C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp)의
`wWinMain`이다.

```text
wWinMain
-> Win32 Window 생성
-> CMainApp::Create
-> Timer 등록
-> Windows Message 처리
-> CMainApp::Update
-> CMainApp::Render
```

### 2.3 3단계: 메인 루프와 프레임 단계를 찾는다

게임 프로젝트라면 아래 이름을 검색한다.

```text
Run, Tick, Update, FixedUpdate, LateUpdate, Render, Draw, Present
```

LostArk:

```text
Priority_Update
-> Update
-> Late_Update
-> Render group submission
-> Renderer::Draw
-> Present
```

일반 서버:

```text
Network ingress
-> command validation
-> fixed simulation tick
-> state mutation
-> snapshot/event construction
-> network egress
```

### 2.4 4단계: 기능 하나를 수직으로 끝까지 추적한다

전체 클래스를 가로로 읽지 않는다. 기능 하나를 선택해 처음부터 끝까지 읽는다.

예: 발탄 한 프레임

```text
Loader가 Model/GameObject Prototype 등록
-> Level이 CValtan Clone
-> CValtan이 CBody_Valtan PartObject Clone
-> Body Update가 CModel::Play_Animation 호출
-> Animation/Channel이 Bone local matrix 변경
-> Bone이 combined matrix 계산
-> Body Late_Update가 Renderer 큐에 자신을 등록
-> Body Render가 Mesh/Material/Bone을 Shader에 바인딩
-> GPU가 정점을 Skinning하고 화면에 출력
```

관련 파일:

- [Loader.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp)
- [Valtan.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp)
- [Body_Valtan.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Body_Valtan.cpp)
- [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp)
- [Animation.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Animation.cpp)
- [Channel.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Channel.cpp)
- [Bone.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Bone.cpp)
- [Mesh.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Mesh.cpp)

### 2.5 첫 30분에 만들 결과물

```text
[제품]
무엇을 실행하는가:

[빌드 그래프]
A -> B -> C

[진입점]
파일 / 함수:

[메인 루프]
Input -> Simulation -> Render -> Present

[권위 데이터]
누가 원본을 소유하는가:

[수직 기능]
입력 -> 변경 -> 출력:

[검증]
어떤 명령과 화면으로 성공을 판단하는가:
```

---

## 3. 파일 확장자를 보면 무엇을 생각해야 하는가

| 파일 | 본질 | 가장 먼저 볼 것 |
|---|---|---|
| `.h/.hpp` | 다른 번역 단위가 보는 선언과 계약 | public API, 소유권, 멤버, virtual |
| `.cpp` | 계약의 구현과 실제 의존성 | 입력, 상태 변경, 반환, 외부 호출 |
| `.inl` | 헤더에 포함되는 inline/template 구현 | 중복 정의와 컴파일 비용 |
| `.c/.cc` | C 또는 C++ 구현 | ABI와 언어 규칙 |
| `.hlsl/.glsl` | GPU 프로그램 | 입력 레이아웃, 상수, 텍스처, 출력 |
| `.json/.yaml/.xml` | 사람이 편집하는 구조화 데이터 | schema, ID, 정본, 검증 |
| `.bin/.dat/.wmodel` | 기계가 빠르게 읽는 바이너리 | magic, version, endian, bounds 검증 |
| `.bat/.ps1/.sh` | 빌드·복사·배포 자동화 | 작업 디렉터리, 실패 처리, 삭제 범위 |
| `.sln/.vcxproj` | Visual Studio 빌드 그래프와 설정 | project dependency, include/lib path |
| `.vcxproj.filters` | Visual Studio 표시용 가상 폴더 | 실제 소유권 정본으로 착각하지 않기 |
| `.Build.cs` | Unreal 모듈 빌드 계약 | Public/Private dependency |
| `.uproject/.uplugin` | Unreal 제품/플러그인 manifest | module load phase와 target |
| `.asmdef` | Unity C# assembly 경계 | 참조 방향, editor/runtime 분리 |
| `.dll/.so` | 런타임 동적 라이브러리 | ABI, export/import, 배포 위치 |
| `.lib/.a` | 정적 라이브러리 또는 import library | 실제 코드 포함 여부 구분 |
| `.pdb` | 디버그 심볼 | 바이너리와 버전 일치 |
| `.pak/.ucas/.utoc` | cooked 콘텐츠 컨테이너 | source asset과 runtime asset 구분 |
| `.md` | 사람과 에이전트의 의사결정 문서 | 코드 정본과 충돌 시 코드를 재검증 |

### 3.1 Authoring, Intermediate, Runtime을 구분한다

```text
Authoring source
FBX, PSD, WAV, JSON, Blueprint

Intermediate/cooked
wmodel, DDS, shader bytecode, generated code, navmesh

Runtime state
GPU Buffer, Component, Entity, Animation cursor, network session

Persistent runtime data
save, replay, placement scene, database row
```

이 네 종류를 섞으면 다음 문제가 생긴다.

- 실행 중 FBX를 매번 Assimp로 읽는다.
- 빌드 산출물을 Git에 올린다.
- vector index나 포인터를 저장 ID로 기록한다.
- 에디터 임시 상태가 게임 정본이 된다.

---

## 4. 컴파일과 링크의 본질

### 4.1 `.cpp` 하나는 번역 단위다

```text
source.cpp
+ source.cpp가 include한 header 텍스트
-> 전처리된 하나의 거대한 번역 단위
-> compiler
-> object file
```

`#include`는 런타임에 파일을 연결하는 문법이 아니다. 전처리기가 헤더 텍스트를 현재 위치에
붙이는 동작이다.

### 4.2 헤더가 필요한 이유

컴파일러가 함수 호출 코드를 만들려면 최소한 다음을 알아야 한다.

```text
이름
매개변수 타입
반환 타입
호출 규약
클래스 레이아웃이 필요한 경우 크기와 멤버 배치
```

헤더는 여러 `.cpp`가 같은 선언 계약을 공유하게 한다.

```cpp
// Damage.h
float CalculateDamage(float attack, float defense);

// Damage.cpp
#include "Damage.h"

float CalculateDamage(float attack, float defense)
{
    return attack * 100.f / (100.f + defense);
}
```

### 4.3 Forward declaration과 include 선택

Forward declaration으로 충분한 경우:

```cpp
class CModel;

class CBody
{
    std::shared_ptr<CModel> m_pModel;
};
```

완전한 정의가 필요한 대표 경우:

- 해당 타입을 값 멤버로 가진다.
- 해당 타입을 상속한다.
- `sizeof`를 사용한다.
- inline 함수에서 멤버에 접근한다.
- 템플릿 인스턴스화가 완전한 타입을 요구한다.

```cpp
#include "Transform.h"

class CBody
{
    CTransform m_Transform; // 크기와 레이아웃이 필요하다.
};
```

기준:

> 헤더에는 공개 선언을 성립시키는 최소 의존성만 두고, 구현에만 필요한 헤더는 `.cpp`에 둔다.

### 4.4 선언과 정의

```cpp
int Add(int a, int b);              // 선언
int Add(int a, int b) { return a+b; } // 정의
extern int g_Count;                 // 외부 정의가 있다고 선언
```

선언은 여러 곳에 나타날 수 있지만, 비-inline 정의는 프로그램 전체에서 하나여야 한다.
이것이 ODR(One Definition Rule)의 핵심이다.

### 4.5 링크

컴파일러는 `.cpp`별 object를 만든다. 링커는 호출 심볼과 실제 정의를 연결한다.

```text
unresolved external symbol
= 선언을 보고 컴파일은 했지만 링크할 정의를 찾지 못함
```

대표 원인:

- `.cpp`가 프로젝트에 등록되지 않았다.
- 필요한 `.lib`를 링크하지 않았다.
- Debug/Release 라이브러리가 다르다.
- namespace, 매개변수, const가 선언과 정의에서 다르다.
- DLL export/import 매크로가 틀렸다.

### 4.6 DLL, import library, runtime DLL

Windows에서 흔한 구조:

```text
Engine.dll = 실행할 실제 코드
Engine.lib = Client 링커가 DLL 심볼 위치를 알기 위한 import library
Engine.pdb = 디버그 심볼
```

링크가 성공해도 실행 폴더에 DLL이 없으면 프로그램 시작 단계에서 실패한다.
Assimp 오류가 컴파일 오류가 아니었던 이유가 이것이다.

### 4.7 Preprocessor, macro, PCH

전처리기는 C++ 타입 시스템이 동작하기 전에 텍스트를 바꾼다.

```cpp
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _DEBUG
    OutputDebugStringA("debug build\n");
#endif
```

```text
#include     파일 텍스트 포함
#define      token/text 치환
#if/#ifdef   compile할 코드 선택
#pragma once header 중복 포함 방지
```

Macro는 namespace, 타입 검사, debugger 가시성이 약하다. 상수는 `constexpr`, 작은 함수는 `inline`,
타입 계산은 template을 우선하고 DLL export나 platform conditional처럼 전처리 단계가 필요한 곳에
macro를 제한한다.

PCH(Precompiled Header)는 자주 변하지 않는 큰 header 집합을 미리 compile해 빌드 시간을 줄인다.
PCH가 실제 dependency를 숨길 수 있으므로 각 header가 자신의 선언을 성립시키는 include 계약은 계속
지켜야 한다.

---

## 5. 타입, 값, 객체, 메모리의 본질

### 5.1 타입이란 무엇인가

타입은 컴파일러와 프로그래머 사이의 계약이다.

```text
몇 바이트를 읽을 것인가?
그 비트를 어떻게 해석할 것인가?
어떤 연산이 허용되는가?
어떻게 생성·복사·이동·파괴하는가?
```

메모리의 비트 자체에는 의미가 없다. 같은 32비트를 `uint32_t`, `float`, 색상, 핸들 중 무엇으로
해석할지는 타입과 계약이 결정한다.

### 5.2 기본 자료형

| 타입 | 일반적인 Windows x64 크기 | 용도와 주의 |
|---|---:|---|
| `bool` | 1 byte인 경우가 일반적 | raw binary ABI로 고정해 저장하지 않는다 |
| `char` | 1 byte | 문자라기보다 1-byte 정수; encoding은 별도 계약 |
| `wchar_t` | Windows 2 bytes | UTF-16 code unit, 한 글자와 항상 일치하지 않음 |
| `int` | 4 bytes | 범용 signed 정수지만 폭 계약에는 부적합 |
| `unsigned int` | 4 bytes | 음수가 없고 modulo 연산; 크기보다 의미가 중요 |
| `int32_t` | 정확히 32 bits | 파일/네트워크 포맷에 적합 |
| `uint32_t` | 정확히 32 bits | ID, count, flag에 자주 사용 |
| `float` | 4 bytes | 약 7자리 십진 정밀도 |
| `double` | 8 bytes | 약 15~16자리 십진 정밀도 |
| pointer | 8 bytes | 주소; 가리키는 객체 수명은 별도 문제 |

Windows x64는 LLP64 모델이라 `long`은 32비트이고 pointer는 64비트다. 크기가 중요한 계약은
`<cstdint>`의 고정 폭 타입을 쓴다.

### 5.3 `string`은 encoding이 아니다

```text
std::string  = byte sequence
std::wstring = wchar_t sequence
```

UTF-8인지 CP949인지 UTF-16인지 타입만으로 결정되지 않는다. LostArk C++ 소스는 CP949를
유지하고, Markdown은 UTF-8을 사용한다. 이 계약을 무시하면 내용이 같아도 파일 전체 diff가 난다.

### 5.4 값과 객체

```cpp
int count = 3;        // int 타입 객체
CValtan valtan;       // CValtan 타입 객체
CValtan* p = &valtan; // 객체 자체가 아니라 주소를 담는 pointer 객체
```

- **class/struct**: 사용자 정의 타입의 설계도
- **object**: 저장 공간과 수명을 가진 해당 타입의 실체
- **instance**: 보통 object와 같은 의미로 사용
- **value**: 객체가 현재 표현하는 값
- **identity**: 같은 값을 가져도 서로 다른 객체임을 구분하는 정체성

### 5.5 저장 영역과 수명

```text
Static storage   프로그램 시작부터 종료까지
Thread storage   thread 시작부터 종료까지
Automatic        scope 진입부터 scope 종료까지, 흔히 stack
Dynamic          할당부터 해제까지, 흔히 heap
GPU memory       API로 resource 생성부터 release까지
```

Stack/heap은 위치이고, lifetime/ownership은 규칙이다. `std::vector` 객체 자체는 stack에 있어도
원소 저장소는 heap에 있을 수 있다.

### 5.6 정렬과 padding

```cpp
struct Example
{
    char a;   // 1
    int b;    // 보통 4-byte 정렬 필요
    char c;   // 1
};
```

멤버 크기의 합은 6이지만 구조체 크기는 padding 때문에 보통 12가 된다. 이 구조체를 그대로
파일이나 네트워크로 `memcpy`하면 compiler/platform 변경에 취약하다. 명시적 직렬화를 사용한다.

---

## 6. `class`와 `struct`의 근본

### 6.1 언어 차이는 기본 접근 권한뿐이다

```cpp
struct SData     // 기본 public, 기본 public 상속
{
    int value;
};

class CObject    // 기본 private, 기본 private 상속
{
    int value;
};
```

둘 다 생성자, 소멸자, 함수, virtual, template을 가질 수 있다.

### 6.2 설계 관례

```text
struct
= 값/메시지/설정/직렬화 레코드
+ 필드의 의미 자체가 계약
+ 불변식이 단순함

class
= 정체성/수명/행동/불변식을 가진 객체
+ 내부 상태를 함수 뒤에 감춤
+ 유효하지 않은 상태를 만들기 어렵게 함
```

예:

```cpp
struct SpawnDesc
{
    float3 position;
    float yaw;
};

class CMonster
{
public:
    void ApplyDamage(float damage);
    bool IsDead() const;

private:
    float m_hp = 100.f; // 함수가 불변식을 지킨다.
};
```

### 6.3 “왜 이 객체가 존재하는가?”

클래스를 볼 때 다음을 답한다.

```text
책임: 이 객체가 없으면 어떤 기능이 사라지는가?
소유 데이터: 이 객체가 정본으로 가진 상태는 무엇인가?
명령: 어떤 public 함수로 상태를 바꾸는가?
관찰: 어떤 public 함수로 상태를 읽는가?
수명: 누가 생성하고 파괴하는가?
협력자: 어떤 객체를 소유하고 어떤 객체를 빌리는가?
불변식: 항상 참이어야 하는 조건은 무엇인가?
```

`CValtan`과 `CBody_Valtan`:

```text
CValtan
= 발탄 전체의 정체성, 상태, 파트 소유자

CBody_Valtan
= 모델, 애니메이션, 셰이더, 렌더링 표현

CValtan owns Body
Body borrows Valtan state/parent matrix
```

### 6.4 캡슐화

캡슐화는 “멤버를 private으로 숨기는 것”에서 끝나지 않는다.

> 상태를 바꿀 수 있는 경로를 제한해 불변식을 한곳에서 지키는 것.

나쁜 예:

```cpp
struct Player
{
    float hp;
};
```

어디서든 `hp = -999999`가 가능하다.

좋은 방향:

```cpp
class Player
{
public:
    void ApplyDamage(float amount)
    {
        if (amount <= 0.f)
            return;
        m_hp = std::max(0.f, m_hp - amount);
    }

private:
    float m_hp = 100.f;
};
```

---

## 7. 생성자, 소멸자, RAII

### 7.1 생성자의 본질

생성자는 객체의 수명을 시작하면서 불변식을 세운다.

```cpp
class File
{
public:
    explicit File(const wchar_t* path)
    {
        m_handle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
};
```

### 7.2 소멸자의 본질

소멸자는 객체 수명이 끝날 때 소유 자원을 반납한다.

```cpp
~File()
{
    if (m_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_handle);
}
```

### 7.3 RAII

Resource Acquisition Is Initialization:

```text
자원 획득을 객체 수명에 묶는다.
scope를 벗어나면 성공/실패/조기 return과 관계없이 소멸자가 정리한다.
```

RAII 대상:

- heap memory
- file handle
- socket
- mutex lock
- DirectX COM object
- temporary transaction

### 7.4 왜 virtual 소멸자가 필요한가

```cpp
class Base
{
public:
    virtual ~Base() = default;
};

class Derived : public Base
{
    std::vector<int> data;
};

std::unique_ptr<Base> p = std::make_unique<Derived>();
```

Base pointer를 통해 Derived를 삭제할 수 있다면 Base 소멸자는 virtual이어야 Derived 소멸자까지
호출된다. 다형적 base인데 virtual 소멸자가 없으면 undefined behavior가 발생할 수 있다.

### 7.5 Rule of Zero / Five

가장 좋은 기본은 Rule of Zero다.

```text
자원을 vector/string/unique_ptr/ComPtr 같은 RAII 멤버가 소유하게 한다.
그러면 복사/이동/소멸 특수 함수를 직접 만들지 않는다.
```

직접 자원을 소유해 특수 함수를 하나 정의해야 한다면 다음 다섯 개의 의미를 함께 검토한다.

```text
destructor
copy constructor
copy assignment
move constructor
move assignment
```

---

## 8. `public`, `private`, `protected`

| 접근 | 의미 | 기준 |
|---|---|---|
| `public` | 외부 사용자와의 안정적 계약 | 최소화하고 의미를 명확히 한다 |
| `private` | 클래스가 불변식을 지키는 구현 | 기본 선택 |
| `protected` | 파생 클래스와 공유하는 확장 계약 | 결합도를 높이므로 신중히 사용 |

접근 권한은 보안 장벽이 아니라 컴파일 타임 설계 장벽이다.

질문:

- 이 멤버를 외부가 바꾸면 불변식이 깨지는가?
- 이 함수는 제품 전체가 의존해도 되는 안정적 API인가?
- 상속 대신 composition으로 협력자를 넣을 수 있는가?

---

## 9. 상속, virtual, 다형성

### 9.1 상속의 의미

상속은 코드 재사용보다 **대체 가능성**이 핵심이다.

```text
Derived는 Base가 요구되는 곳에서 Base처럼 올바르게 동작해야 한다.
```

LostArk:

```text
CPrototype
├─ CComponent
│  ├─ CTransform
│  ├─ CModel
│  └─ CShader
└─ CGameObject
   ├─ CContainerObject
   │  └─ CValtan
   └─ CPartObject
      └─ CBody_Valtan
```

### 9.2 virtual 호출의 본질

```cpp
class GameObject
{
public:
    virtual void Update(float dt) = 0;
};

void Tick(GameObject& object, float dt)
{
    object.Update(dt);
}
```

실제 타입이 Player이면 Player::Update, Monster이면 Monster::Update가 호출된다. 일반 구현은
vptr/vtable을 사용하지만 표준은 구체 구현까지 강제하지 않는다.

비용보다 중요한 것은 의미다.

- 런타임 대체 가능성
- 호출 대상 추적이 어려워짐
- base contract가 커지면 모든 파생형이 결합됨

### 9.3 `override`와 `final`

```cpp
void Update(float dt) override;
class CValtan final : public CContainerObject {};
```

- `override`: base virtual을 정확히 재정의했는지 compiler가 검사
- `final`: 더 이상의 상속 또는 override를 금지

### 9.4 OOP와 Composition

```text
Inheritance: is-a
Composition: has-a / uses-a
```

`CBody_Valtan has a CModel component`가 자연스럽다면 CBody가 CModel을 상속하지 않는다.

---

## 10. 함수의 본질과 추적법

### 10.1 함수는 상태 변환 단위다

```text
explicit input
+ object/member state
+ external state
-> guard/branch/search/calculation
-> return value
+ member mutation
+ output parameter mutation
+ external side effect
```

### 10.2 모든 함수를 읽는 12개 질문

```text
1. 누가 호출하는가?
2. 언제, 어느 스레드에서 호출하는가?
3. 명시적 입력은 무엇인가?
4. 입력의 소유권과 유효 기간은 무엇인가?
5. 어떤 멤버/전역/싱글턴을 읽는가?
6. 어떤 상태를 변경하는가?
7. 내부에서 어떤 함수를 호출하는가?
8. 반환 타입은 무엇인가?
9. 반환값의 도메인 의미는 무엇인가?
10. 누가 반환값을 소비하고 무시하는가?
11. 실패는 어디까지 전파되는가?
12. 실행 빈도와 시간/공간 비용은 무엇인가?
```

### 10.3 한 문장 압축 공식

```text
[시점/스레드]에 [입력]을 받아,
[읽는 상태]를 바탕으로 [상태/외부 시스템]을 변경하고,
[도메인 의미]를 반환하며,
그 결과는 [다음 소비자]가 사용한다. 비용은 [복잡도/빈도]다.
```

### 10.4 반환형별 의미

| 반환형 | 흔한 의미 | 반드시 볼 것 |
|---|---|---|
| `HRESULT` | 성공/실패 | `FAILED`로 누가 전파하는가 |
| `bool` | 발견/완료/조건 | `false`가 오류인지 정상 상태인지 |
| pointer | 객체 위치 또는 없음 | 소유인지 관찰인지 |
| `unique_ptr` | 단독 소유권 이동 | 다음 owner가 누구인지 |
| `shared_ptr` | 공동 소유권 참여 | cycle과 최종 owner |
| reference | 기존 객체 별칭 | 원본 수명보다 오래 저장하지 않는지 |
| value | 결과 복사/이동 | 크기와 copy/move 비용 |
| `void` | return 없음 | 멤버/외부 시스템 부작용 찾기 |

### 10.5 LostArk 실제 반환 전파

```text
CBody_Valtan::Initialize 실패
-> CBody_Valtan::Clone returns nullptr
-> CContainerObject::Add_PartObject returns E_FAIL
-> CValtan::Ready_PartObjects returns E_FAIL
-> CValtan::Initialize returns E_FAIL
-> CValtan::Clone returns nullptr
-> Prototype_Manager::Clone_Prototype returns nullptr
-> Object_Manager::Add_GameObject_to_Layer returns E_FAIL
-> Level initialization 실패
```

### 10.6 `void`의 숨은 출력

```cpp
void CBody_Valtan::Late_Update(float dt)
{
    CGameInstance::Get().Add_RenderObject(
        RENDERGROUP::NONBLEND,
        shared_from_this());
}
```

명시적 반환값은 없지만 Renderer queue가 바뀐다. 다음 `Renderer::Draw`가 그 큐를 소비한다.

### 10.7 반환값이 버려지는 경우

`CModel::Play_Animation`은 animation 종료 여부를 반환하지만 현재 idle 갱신에서는 반환값을
사용하지 않는다.

```cpp
m_pModelCom->Play_Animation(fTimeDelta); // 반환값 폐기
```

공격 상태 전환에서는 다음처럼 의미가 생긴다.

```cpp
if (m_pModelCom->Play_Animation(fTimeDelta))
{
    m_eState = VALTAN_STATE::IDLE;
}
```

### 10.8 정방향과 역방향 추적

정방향:

```text
입력 함수
-> 반환/부작용
-> 다음 소비자
-> 최종 결과
```

역방향:

```text
화면/오류/파일
-> 바로 생산한 함수
-> 그 함수가 읽은 상태
-> 마지막으로 상태를 쓴 함수
-> 생성/로드/입력 출처
```

검색 예:

```powershell
rg -n "Play_Animation\(" Client Engine
rg -n "Clone_Prototype\(" Client Engine
rg -n "Prototype_GameObject_Valtan" Client Engine
```

---

## 11. Pointer, Reference, Smart Pointer

### 11.1 raw pointer

```cpp
CModel* pModel;
```

pointer는 주소를 저장한다. 다음 의미 중 무엇인지 타입만으로는 알 수 없다.

- nullable observer
- 배열의 시작
- 소유권 없는 빌림
- 레거시 owning pointer

따라서 코드의 생성/삭제/저장 위치를 추적해야 한다.

### 11.2 reference

```cpp
void Update(const FrameData& frame);
```

- null이 아니어야 한다는 의도를 표현
- 기존 객체의 별칭
- 소유권 없음
- 저장한다면 원본 수명 계약을 별도로 확인

### 11.3 `unique_ptr`

```cpp
std::unique_ptr<CPrototype>
```

- owner가 정확히 하나
- 복사 불가, 이동 가능
- scope 종료 시 자동 파괴
- 기본 소유권 표현

LostArk Prototype 원본을 Manager가 단독 소유하는 데 적합하다.

### 11.4 `shared_ptr`

```cpp
std::shared_ptr<CGameObject>
```

- control block의 strong reference count 공유
- 마지막 strong reference가 사라지면 객체 파괴
- 복사/파괴 시 reference count 갱신 비용
- 논리적 owner가 흐려질 수 있음
- 서로를 strong reference하면 cycle 발생

LostArk에서는 Layer와 Renderer queue 등이 frame 동안 GameObject 생존을 공유한다.

### 11.5 `weak_ptr`

```cpp
std::weak_ptr<CGameObject> target;

if (auto locked = target.lock())
    locked->Update();
```

- 수명을 연장하지 않는 관찰자
- cycle을 끊음
- 접근할 때 `lock()`으로 아직 살아 있는지 검사

### 11.6 `ComPtr`

DirectX COM 객체는 `AddRef/Release` reference counting을 사용한다.

```cpp
Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVB;
```

`ComPtr`가 scope와 COM reference count를 묶으므로 직접 `Release()` 호출 실수를 줄인다.

### 11.7 선택 기준

```text
단독 owner          unique_ptr
값 자체             value member
공동 생존 계약      shared_ptr
수명 비연장 관찰    weak_ptr 또는 명시적 raw pointer/reference
COM 자원            ComPtr
```

---

## 12. 복사, 이동, `const`

### 12.1 복사는 두 번째 독립 값을 만든다

```cpp
std::vector<int> a{1,2,3};
std::vector<int> b = a; // 원소 복사
```

### 12.2 이동은 자원 표현을 넘긴다

```cpp
std::vector<int> b = std::move(a);
```

대개 heap buffer 소유권을 넘긴다. 이동된 `a`는 valid하지만 값은 unspecified이므로 다시 값을
가정하지 않는다.

### 12.3 `const`

```cpp
void Render(const RenderSnapshot& snapshot);
bool IsDead() const;
```

- 읽기 전용 의도를 compiler가 검사
- API의 변경 가능 범위를 줄임
- thread-safe를 자동 보장하지는 않음

### 12.4 매개변수 기본 선택

```text
작은 값(int, float, handle)          value
큰 읽기 전용 객체                    const T&
수정할 필수 외부 객체                 T&
nullable observer                    T*
소유권 인수                           unique_ptr<T>
공유 생존 인수                        shared_ptr<T>는 정말 필요할 때
```

---

## 13. Function object, Lambda, Template

### 13.1 함수 객체

```cpp
struct LessByDistance
{
    bool operator()(const Target& a, const Target& b) const
    {
        return a.distance < b.distance;
    }
};
```

상태와 호출 연산을 함께 가진 객체다.

### 13.2 Lambda

```cpp
const float maxDistance = 10.f;
auto isNear = [maxDistance](const Target& target)
{
    return target.distance <= maxDistance;
};
```

lambda는 compiler가 만드는 익명 closure class 객체라고 생각하면 된다.

Capture:

```text
[]       아무것도 capture하지 않음
[x]      x를 값으로 복사
[&x]     x를 reference로 빌림
[=]      사용한 외부 변수를 기본 값 capture
[&]      사용한 외부 변수를 기본 reference capture
```

비동기 저장되는 lambda가 reference capture를 하면 원본 수명이 끝난 뒤 dangling reference가 될 수 있다.

### 13.3 `std::function`

서로 다른 callable을 같은 타입으로 저장하는 type erasure wrapper다.

```cpp
std::function<void(const Event&)> callback;
```

유연하지만 indirect call과 경우에 따른 heap allocation 비용이 있다. hot loop에서는 template,
function pointer, 명시적 interface와 비교한다.

### 13.4 Template

```cpp
template<typename T>
T Clamp(T value, T minValue, T maxValue);
```

- 타입별 코드를 compile time에 생성
- static polymorphism
- header에 정의가 필요한 경우가 많음
- compile time과 binary size가 증가할 수 있음

---

## 14. 연산자와 비트 연산

### 14.1 대입과 비교

```cpp
a = b;   // b의 값을 a에 대입
a == b;  // 같은지 비교해 bool 생성
a != b;  // 다른지 비교
```

### 14.2 논리 연산과 비트 연산

```cpp
conditionA && conditionB // 논리 AND, short-circuit
conditionA || conditionB // 논리 OR, short-circuit
!condition               // 논리 NOT

bitsA & bitsB  // bitwise AND
bitsA | bitsB  // bitwise OR
bitsA ^ bitsB  // bitwise XOR
~bitsA         // bitwise NOT
```

### 14.3 `|=`의 의미

```cpp
flags |= FLAG_VISIBLE;
```

다음과 같다.

```cpp
flags = flags | FLAG_VISIBLE;
```

기존 flag를 유지하면서 특정 bit를 켠다.

```cpp
constexpr uint32_t Visible = 1u << 0; // 0001
constexpr uint32_t Shadow  = 1u << 1; // 0010

uint32_t flags = 0;
flags |= Visible; // 0001
flags |= Shadow;  // 0011

const bool hasShadow = (flags & Shadow) != 0;
flags &= ~Shadow; // Shadow bit 끄기
```

LostArk의 Assimp flag 결합도 같은 원리다.

```cpp
uint32_t flags = aiProcess_ConvertToLeftHanded |
                 aiProcessPreset_TargetRealtime_Fast;

if (MODEL::NONANIM == type)
    flags |= aiProcess_PreTransformVertices;
```

### 14.4 Shift

```cpp
1u << 3 // 0001을 왼쪽으로 3칸 -> 1000 -> 8
8u >> 2 // 1000을 오른쪽으로 2칸 -> 0010 -> 2
```

용도:

- flag bit 위치
- packed format 추출
- power-of-two alignment
- 네트워크/파일 header 조작

주의:

- signed 음수 shift와 폭 이상 shift는 위험하다.
- overflow가 있는 signed 연산은 undefined behavior가 될 수 있다.
- `stream << value`의 `<<`는 bit shift가 아니라 overload된 출력 연산자다.

### 14.5 Enum flag 예시

로컬 Unreal source의 `EUpdateTransformFlags`도 이 방식을 쓴다.

```cpp
enum class EUpdateTransformFlags : int32
{
    None = 0x0,
    SkipPhysicsUpdate = 0x1,
    PropagateFromParent = 0x2,
    OnlyUpdateIfUsingSocket = 0x4
};
```

### 14.6 제어 흐름: `if`, `switch`, `for`, `while`

제어 흐름은 “어떤 문장을 다음에 실행할지” 결정한다.

#### `if/else`

```cpp
if (hp <= 0.f)
{
    EnterDeadState();
}
else if (target != nullptr)
{
    Chase(*target);
}
else
{
    Idle();
}
```

조건식은 `bool`로 해석된다. `&&`와 `||`는 short-circuit한다.

```cpp
if (pModel != nullptr && pModel->IsReady())
```

`pModel == nullptr`이면 뒤 함수는 호출되지 않아 null 역참조를 피한다. 조건 순서가 비용과 안전성에
영향을 준다.

#### `switch`

한 값의 이산 상태를 분기할 때 적합하다.

```cpp
switch (state)
{
case State::Idle:
    UpdateIdle();
    break;
case State::Attack:
    UpdateAttack();
    break;
default:
    assert(false);
    break;
}
```

`break`가 없으면 다음 case로 fallthrough한다. 의도한 경우 명시적으로 `[[fallthrough]]`를 사용한다.

#### 기본 `for`

```cpp
for (size_t i = 0; i < values.size(); ++i)
{
    Process(values[i]);
}
```

의미:

```text
초기화 1회
-> 조건 검사
-> body
-> 증감식
-> 다시 조건 검사
```

Range-for:

```cpp
for (const auto& value : values)
    Inspect(value);
```

원소를 수정하지 않고 큰 타입이면 `const auto&`, 수정하면 `auto&`, 의도적으로 복사하면 `auto`를
사용한다.

#### `while`

반복 횟수보다 종료 조건이 중심일 때 쓴다.

```cpp
while (!openSet.empty())
{
    Node current = openSet.top();
    openSet.pop();
    if (current.id == goal)
        break;
}
```

매 반복마다 종료 조건에 가까워지는 상태가 있어야 한다. 없으면 무한 루프다.

#### `do/while`

body를 최소 한 번 실행한 뒤 조건을 검사한다.

```cpp
do
{
    ReadNextToken();
} while (HasMoreTokens());
```

#### `break`, `continue`, `return`

```text
break     가장 가까운 loop/switch 종료
continue  현재 반복의 나머지를 건너뛰고 다음 반복
return    함수 전체 종료, 선택적으로 결과 전달
```

Guard clause는 중첩을 줄인다.

```cpp
HRESULT Load(const char* path)
{
    if (path == nullptr)
        return E_INVALIDARG;
    if (!Exists(path))
        return E_FAIL;

    return LoadValidated(path);
}
```

#### Loop를 읽는 세 질문

```text
초기 상태는 무엇인가?
매 반복 무엇이 변하는가?
왜 반드시 끝나는가?
```

추가로 유지되는 loop invariant를 찾는다. 예를 들어 binary search에서는 답이 존재한다면 항상
현재 `[left, right)` 구간 안에 있다는 조건이 반복마다 유지되어야 한다.

#### Off-by-one

```cpp
for (size_t i = 0; i < count; ++i)  // 0 .. count-1
```

`i <= count`로 쓰면 `count` index까지 접근해 한 칸 넘는다. 범위를 `[begin, end)` 반열린 구간으로
통일하면 길이가 `end-begin`이고 빈 범위가 `begin==end`라 reasoning이 쉬워진다.

### 14.7 재귀

함수가 더 작은 동일 문제로 자신을 호출한다.

```cpp
void Traverse(Node* node)
{
    if (node == nullptr) // base case
        return;

    Traverse(node->left);
    Visit(*node);
    Traverse(node->right);
}
```

필수:

- 종료 base case
- 매 호출 문제가 작아짐
- 최대 깊이가 thread stack을 넘지 않음

깊이가 외부 입력에 따라 매우 커질 수 있으면 명시적 `std::stack`을 사용하는 반복 구현을 검토한다.

---

## 15. 자료구조의 본질

자료구조 선택 질문은 “Big-O가 무엇인가?”에서 끝나지 않는다.

```text
원소 수는 얼마인가?
가장 잦은 연산은 무엇인가?
순서가 필요한가?
중복을 허용하는가?
주소 안정성이 필요한가?
메모리가 연속이어야 하는가?
할당 횟수와 cache locality는 어떤가?
멀티스레드 공유가 필요한가?
ID가 저장/네트워크 경계를 넘는가?
```

### 15.1 핵심 컨테이너 표

| 구조 | 저장 | 접근/탐색 | 삽입·삭제 | 핵심 주의 |
|---|---|---|---|---|
| C array | 고정 연속 | index `O(1)` | 크기 변경 불가 | 길이 정보가 분리되기 쉬움 |
| `std::array` | 고정 연속 | index `O(1)` | 크기 변경 불가 | 값 semantics, 길이가 타입에 포함 |
| `std::vector` | 동적 연속 | index `O(1)`, 탐색 `O(N)` | 끝 amortized `O(1)`, 중간 `O(N)` | 재할당 시 pointer/iterator 무효화 |
| `std::list` | 양방향 노드 | 접근/탐색 `O(N)` | 위치를 알면 `O(1)` | 할당·pointer·cache miss 비용 큼 |
| `std::deque` | 분할 블록 | index `O(1)` | 양끝 amortized `O(1)`, 중간 `O(N)` | 완전 연속 메모리 아님 |
| `std::map` | 균형 검색 트리 | `O(log N)` | `O(log N)` | key 정렬, 노드 비용 |
| `std::set` | 균형 검색 트리 | `O(log N)` | `O(log N)` | key 자체가 값, unique |
| `unordered_map` | hash table | 평균 `O(1)`, 최악 `O(N)` | 평균 `O(1)` | rehash, hash 품질, 순서 없음 |
| `unordered_set` | hash table | 평균 `O(1)`, 최악 `O(N)` | 평균 `O(1)` | membership에 적합 |
| `stack` | adaptor | top `O(1)` | push/pop `O(1)` | LIFO, 순회 API 없음 |
| `queue` | adaptor | front `O(1)` | push/pop `O(1)` | FIFO |
| `priority_queue` | heap adaptor | top `O(1)` | push/pop `O(log N)` | 전체 정렬 상태는 아님 |

### 15.2 Array와 Vector

공통:

- 원소가 연속적
- 임의 접근 `O(1)`
- cache locality가 좋음

차이:

```text
array      크기가 compile time에 고정, 보통 객체 내부에 원소 직접 저장
vector     size/capacity가 runtime에 변함, 별도 동적 buffer 소유
```

Vector 핵심 상태:

```text
data pointer
size     현재 생성된 원소 수
capacity 재할당 없이 담을 수 있는 수
```

`push_back`이 capacity를 넘으면 더 큰 buffer를 할당하고 기존 원소를 move/copy한다. 개별 호출은
`O(N)`일 수 있지만 여러 번의 평균은 amortized `O(1)`이다.

### 15.3 List의 `O(1)` 함정

```cpp
auto it = FindSomething(list); // 찾기 O(N)
list.erase(it);                // iterator가 있으면 O(1)
```

전체 작업은 탐색 때문에 `O(N)`이다. 각 노드는 별도 할당되고 다음 노드가 메모리상 멀리 있어 CPU
cache miss가 많다. 작은 객체를 순회하는 게임 hot loop에서는 vector가 자주 더 빠르다.

### 15.4 Deque

Deque는 이진 트리가 아니다. 보통 여러 고정 크기 블록과 그 블록들을 가리키는 map을 사용한다.

```text
양끝 push/pop: amortized O(1)
index access: O(1), 단 vector보다 한 단계 더 간접 접근
중간 삽입/삭제: O(N)
전체 contiguous buffer 요구 API에는 부적합
```

### 15.5 Map/Set

`std::map/set`은 보통 red-black tree다.

```text
항상 key 정렬 유지
tree 높이 O(log N)
find/insert/erase O(log N)
iterator 순회는 정렬 순서
```

Red-black tree를 직접 구현할 줄 아는 것보다 왜 균형이 필요한지 이해하는 것이 먼저다.
편향된 이진 검색 트리는 높이가 `N`이 되어 탐색 `O(N)`이 될 수 있다. 색 규칙과 회전으로 높이를
`O(log N)`으로 제한한다.

### 15.6 Hash table

```text
key -> hash -> bucket -> 동일 bucket 후보 비교
```

평균 `O(1)` 조건:

- hash가 key를 고르게 분산
- load factor 관리
- equality가 올바름

최악 `O(N)` 조건:

- 모든 key가 한 bucket에 충돌
- 공격자가 충돌 key를 의도적으로 입력

`reserve`로 예상 수를 미리 잡으면 rehash와 pointer/iterator 무효화를 줄일 수 있다.

### 15.7 Stack, Queue, Priority Queue

```text
Stack: 마지막에 넣은 것을 먼저 꺼냄(LIFO)
호출 stack, Undo, DFS

Queue: 먼저 넣은 것을 먼저 꺼냄(FIFO)
이벤트, BFS, producer-consumer

Priority queue: 우선순위가 가장 높은 원소를 먼저 꺼냄
Dijkstra/A* open set, scheduler
```

### 15.8 Ring Buffer

고정 배열을 원형으로 재사용한다.

```text
head/tail modulo capacity
allocation 없는 고정 메모리
network packet queue, audio buffer, frame history에 적합
```

가득 찼을 때 block/drop/overwrite 중 무엇을 할지는 자료구조가 아니라 제품 정책이다.

### 15.9 Stable handle

pointer나 vector index를 외부 ID로 쓰면 재할당·삭제·프로세스 재실행에서 깨진다.

```cpp
struct EntityHandle
{
    uint32_t index;
    uint32_t generation;
};
```

slot을 재사용할 때 generation을 증가시키면 오래된 handle을 검출할 수 있다. 단, process-local handle과
save/network의 stable asset ID는 서로 다른 계약이다.

---

## 16. 시간 복잡도와 실제 성능

### 16.1 Big-O가 답하는 것

입력 크기 `N`이 커질 때 연산 수가 어떤 비율로 증가하는가.

```text
O(1)       입력 수와 무관한 상수 단계
O(log N)   단계마다 후보가 일정 비율로 줄어듦
O(N)       전체 한 번 순회
O(N log N) 효율적 비교 정렬, divide-and-conquer
O(N^2)     모든 쌍 비교
O(2^N)     선택 조합 폭발
O(N!)      순열 전체 탐색
```

### 16.2 반복문 읽기

```cpp
for (auto& channel : channels) {} // O(C)
for (auto& bone : bones) {}       // O(B)
```

연속이므로 `O(C+B)`다.

```cpp
for (auto& mesh : meshes)
    for (auto& bone : bones) {}
```

중첩이므로 `O(M*B)`다.

### 16.3 실제 성능은 더 많은 항을 가진다

```text
실제 시간
= 연산 수
+ cache miss
+ branch misprediction
+ heap allocation
+ lock contention
+ virtual/indirect call
+ system call
+ page fault
+ CPU-GPU synchronization
+ I/O latency
```

`list erase O(1)`이 `vector erase O(N)`보다 항상 빠르지 않은 이유다. Vector의 `memmove`는 연속
메모리에서 매우 빠르고, list는 탐색과 cache miss와 allocation을 포함할 수 있다.

### 16.4 Amortized complexity

Vector push처럼 가끔 비싼 작업이 있지만 긴 연산열 전체 비용을 나누면 평균 상수인 경우다.

### 16.5 Worst, Average, Expected

- worst case: 보장해야 하는 최대 비용
- average case: 입력 분포를 가정한 평균
- expected: randomized/hash 가정 아래 기대값

게임 실시간 시스템은 평균뿐 아니라 긴 tail spike도 본다. 평균 2ms라도 가끔 30ms면 끊긴다.

### 16.6 측정 원칙

```text
가설 -> 계측 -> 재현 -> 변경 -> 동일 조건 재측정 -> 회귀 검증
```

Container를 느낌으로 고르지 않는다. 예상 원소 수와 연산 패턴을 먼저 쓰고 microbenchmark와 실제
frame capture로 검증한다.

---

## 17. 알고리즘의 본질과 선택 지도

알고리즘은 유명한 이름 모음이 아니다.

> 입력 상태를 원하는 출력 상태로 바꾸는 유한한 절차와 그 비용/정확성 계약.

### 17.1 먼저 문제 형태를 분류한다

```text
정확히 찾기인가?
정렬된 순서가 필요한가?
최단 거리인가?
연결 여부인가?
스케줄 순서인가?
공간에서 가까운 후보인가?
모든 경우가 필요한가, 좋은 근사면 되는가?
데이터가 한 번만 오나 계속 갱신되나?
```

### 17.2 검색

| 알고리즘 | 조건 | 비용 |
|---|---|---:|
| 선형 검색 | 정렬 없음, 작은 N | `O(N)` |
| 이진 검색 | 정렬된 random-access 범위 | `O(log N)` |
| Hash lookup | equality key | 평균 `O(1)` |
| Tree lookup | 정렬/범위 query | `O(log N)` |

작은 N에서는 단순 선형 검색이 cache와 branch 때문에 더 빠를 수 있다.

### 17.3 정렬

```cpp
std::sort(values.begin(), values.end());
```

일반적으로 `O(N log N)`. 구현은 흔히 introsort 계열이다.

선택 질문:

- 안정 정렬이 필요한가? 같은 key 순서를 보존해야 하는가?
- 매 프레임 전체 정렬이 필요한가?
- bucket/radix가 가능한 작은 정수 key인가?
- 이미 거의 정렬되어 있는가?

렌더링에서는 material/shader/depth key로 draw item을 정렬해 state change를 줄일 수 있다.

### 17.4 BFS와 DFS

둘 다 graph에서 `O(V+E)`.

```text
BFS: queue, 가중치 없는 최단 간선 수, 레벨 순회
DFS: stack/recursion, 연결 요소, cycle, backtracking
```

### 17.5 Dijkstra

음수가 아닌 edge cost의 최단 경로를 구한다.

```text
priority queue 사용 시 대략 O((V+E) log V)
```

시작점에서 모든 방향으로 비용이 낮은 후보부터 확장한다.

### 17.6 A*

```text
f(n) = g(n) + h(n)
g = 시작부터 실제 비용
h = 목표까지의 추정 비용
```

- `h=0`이면 Dijkstra처럼 동작
- admissible heuristic은 실제 남은 비용을 과대평가하지 않음
- navigation mesh/grid 위에서 목표 방향의 불필요한 탐색을 줄임

발탄 추적에 필요한 것은 “A*를 안다”보다 다음 계약이다.

```text
walkable graph의 정본은 무엇인가?
동적 파괴물이 graph를 어떻게 막고 여는가?
좌표를 어느 cell/polygon에 투영하는가?
path가 없을 때 어떤 정책을 쓰는가?
재탐색 빈도와 예산은 얼마인가?
```

### 17.7 Topological Sort

방향 비순환 그래프의 의존 순서를 만든다. `O(V+E)`.

용도:

- build dependency
- asset cook dependency
- task graph
- animation/effect graph execution order

cycle이 있으면 올바른 순서를 만들 수 없다.

### 17.8 Union-Find

집합의 연결 여부와 병합을 빠르게 처리한다.

```text
Find + Union
path compression + union by rank
amortized 거의 O(1)
```

용도: 섬 연결, Kruskal MST, offline connectivity.

### 17.9 공간 자료구조

| 구조 | 용도 | 핵심 tradeoff |
|---|---|---|
| Uniform grid | 크기가 비슷한 객체, 단순 월드 | sparse/밀집 불균형 |
| Quadtree | 2D 공간 분할 | 업데이트 비용과 균형 |
| Octree | 3D 공간 분할 | node/메모리 비용 |
| BVH | mesh raycast, collision | build/refit 비용 |
| KD-tree | point nearest query | 동적 갱신 어려움 |
| NavMesh | 걸을 수 있는 polygon graph | bake와 동적 obstacle 처리 |

### 17.10 상태 머신

게임 알고리즘에서 가장 자주 쓰이는 형태다.

```text
State + Event -> Next State + Action
```

```cpp
switch (m_state)
{
case State::Idle:
    if (CanSeeTarget())
        Enter(State::Chase);
    break;
case State::Chase:
    if (InAttackRange())
        Enter(State::Attack);
    break;
}
```

불변식:

- 상태 전환은 한 권위 함수에서만 일어난다.
- Enter/Exit 부작용을 명확히 한다.
- 마지막 `if`가 앞 결정을 조용히 덮지 않게 우선순위를 둔다.

---

## 18. OOP와 DOD/ECS

### 18.1 OOP

```cpp
for (auto& object : objects)
    object->Update(dt);
```

장점:

- 개별 객체 책임이 읽기 쉬움
- polymorphism과 tool-friendly object model
- 규모가 작을 때 빠른 개발

비용:

- 객체별 흩어진 allocation
- pointer chasing
- virtual dispatch
- 데이터가 한 작업에 맞게 모여 있지 않음

### 18.2 DOD

Data-Oriented Design는 계산할 데이터 배열을 연산 순서에 맞춰 배치한다.

AoS:

```cpp
struct Particle
{
    float3 position;
    float3 velocity;
    float life;
};
std::vector<Particle> particles;
```

SoA:

```cpp
struct Particles
{
    std::vector<float3> positions;
    std::vector<float3> velocities;
    std::vector<float> lives;
};
```

위치만 갱신한다면 SoA는 필요 데이터가 촘촘해 cache/SIMD에 유리할 수 있다.

### 18.3 ECS

```text
Entity    identity/handle
Component data
System    같은 component 조합에 대한 bulk operation
```

ECS의 본질은 class를 전부 없애는 것이 아니라 identity, data, behavior를 분리해 대량 처리와 조합을
쉽게 하는 것이다.

### 18.4 무엇을 선택하는가

```text
소수의 복잡한 객체/툴 상호작용       OOP/Composition
대량의 동일 연산 entity/particle     DOD/ECS
외부 API/renderer abstraction         interface/polymorphism
hot inner loop                         contiguous data/static dispatch 검토
```

대부분의 실제 엔진은 혼합형이다.

---

## 19. CPU와 GPU의 본질

### 19.1 CPU

CPU는 복잡한 제어 흐름과 낮은 지연의 직렬 작업에 강하다.

- 입력 처리
- 게임 규칙과 AI
- animation time/pose 계산
- visibility와 draw submission
- 파일/네트워크 orchestration

### 19.2 GPU

GPU는 같은 프로그램을 많은 데이터에 병렬 적용하는 처리량에 강하다.

- vertex transformation
- skinning
- rasterization
- pixel shading
- compute workloads

GPU는 CPU의 함수를 직접 호출하지 않는다. CPU가 API 명령과 resource를 제출하고 GPU가 나중에
command stream을 소비한다.

### 19.3 DirectX 11 그래픽 파이프라인

```text
CPU game/update
-> vertex/index/constant/texture resource 준비
-> Input Assembler
-> Vertex Shader
-> optional Hull/Domain/Geometry Shader
-> Rasterizer
-> Pixel Shader
-> Output Merger
-> Render Target
-> Present
```

### 19.4 좌표 변환

```text
Model local
-> World
-> View
-> Projection
-> Clip
-> NDC
-> Viewport pixel
```

Skinned model은 local position 전에 Bone blend가 추가된다.

```text
vertex indices/weights
-> weighted Bone matrices
-> skinned local position
-> World/View/Projection
```

LostArk에서는 CPU가 animation channel과 Bone combined matrix를 갱신하고, `CMesh::Bind_Resource`가
Bone matrix 배열을 Shader에 전달하며, GPU Vertex Shader가 각 정점의 네 Bone을 blending한다.

### 19.5 CPU-GPU 동기화가 비싼 이유

CPU와 GPU는 서로 다른 timeline에서 실행한다. CPU가 GPU 결과를 즉시 읽으려고 기다리면 pipeline이
멈춘다.

비싼 패턴:

- 매 프레임 staging resource를 즉시 Map해서 GPU 결과 읽기
- query 결과를 같은 frame에 blocking wait
- dynamic buffer를 잘못 갱신해 GPU 사용 완료를 기다림
- resource를 자주 생성/파괴

### 19.6 GPU 최적화 순서

1. 측정해서 CPU-bound/GPU-bound를 구분한다.
2. frame budget과 pass별 시간을 기록한다.
3. 가장 큰 병목 하나를 고친다.
4. 동일 장면과 설정으로 다시 측정한다.

CPU render 병목:

- draw call 감소
- material/state 기준 정렬
- instancing/batching
- culling/LOD
- frame allocation 제거
- command generation 병렬화

GPU 병목:

- overdraw 감소
- resolution/texture bandwidth 감소
- shader instruction/branch 감소
- light/shadow 수와 범위 조정
- texture format/mipmap/streaming
- geometry LOD

### 19.7 Draw call과 triangle 수

항상 triangle만 줄인다고 빨라지지 않는다.

```text
많은 작은 draw -> CPU submission/state overhead
한 개의 거대한 draw -> GPU vertex/pixel/overdraw 비용
```

장면과 병목에 따라 균형이 다르다.

### 19.8 Shader 분기

GPU는 wave/warp 단위로 여러 lane을 함께 실행한다. 같은 wave 안에서 분기가 갈리면 양쪽 경로를
실행하고 mask할 수 있어 효율이 떨어진다. 단, 모든 분기가 항상 느린 것은 아니며 compiler와 분기
coherence를 측정해야 한다.

### 19.9 메모리 대역폭

GPU 연산보다 texture/buffer 읽기가 병목일 수 있다.

- 필요한 channel만 저장
- 적합한 compressed texture format
- mipmap
- 작은 constant와 packed data
- 중복 render target 감소
- 같은 값을 반복 업로드하지 않기

---

## 20. Windows 프로그램의 본질

### 20.1 User mode와 Kernel mode

애플리케이션은 user mode에서 실행되고 파일, socket, window, thread 같은 OS 자원은 Windows API를
통해 kernel service를 요청한다.

### 20.2 Process와 Thread

```text
Process
= virtual address space
+ handles
+ loaded modules
+ security context
+ one or more threads

Thread
= instruction pointer
+ registers
+ stack
+ scheduling state
```

같은 process thread들은 heap/global을 공유한다. 그래서 race가 생긴다.

### 20.3 Virtual Memory

각 process는 독립적인 가상 주소 공간을 본다. OS와 CPU page table이 virtual address를 physical memory
또는 backing storage에 매핑한다.

개념:

- reserve: 주소 범위 확보
- commit: 실제 backing 보장
- page fault: 현재 매핑/상주하지 않은 page 접근 처리
- working set: 현재 RAM에 상주한 page 집합

### 20.4 Win32 Window와 Message Loop

LostArk [Client.cpp](C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp):

```text
RegisterClassExW
-> CreateWindowW
-> PeekMessage
-> TranslateMessage
-> DispatchMessage
-> WndProc
```

`WndProc`는 Windows가 window event를 전달하는 callback이다.

### 20.5 Handle

`HANDLE`, `HWND`, `HINSTANCE`는 OS 자원을 나타내는 opaque handle이다. 정수처럼 보여도 의미 없는
ID가 아니라 OS 계약에 따라 특정 함수로 닫아야 하는 자원일 수 있다.

```text
CreateFile -> CloseHandle
CreateEvent -> CloseHandle
socket -> closesocket
COM -> Release, 보통 ComPtr
```

### 20.6 Thread synchronization

| 도구 | 의미 | 주의 |
|---|---|---|
| mutex/critical section | 한 번에 한 thread만 진입 | contention/deadlock |
| shared mutex | 다수 reader, 단일 writer | writer starvation 가능 |
| condition variable | 조건이 변할 때 sleep/wake | predicate loop 필요 |
| semaphore | 제한된 permit 수 | count 정책 필요 |
| event | 신호 기반 wake | manual/auto reset 구분 |
| atomic | 작은 연산의 원자성/ordering | 복합 불변식 자동 해결 아님 |

### 20.7 Race와 Deadlock

Data race:

```text
둘 이상의 thread가 같은 memory에 동시 접근
+ 최소 하나가 write
+ 올바른 synchronization 없음
```

C++에서는 undefined behavior다.

Deadlock 필요 조건:

```text
mutual exclusion
hold and wait
no preemption
circular wait
```

고정 lock ordering, 짧은 critical section, message passing으로 위험을 줄인다.

### 20.8 IOCP

Windows IO Completion Port는 다수 비동기 I/O 완료를 제한된 worker thread로 처리하는 모델이다.

중요 경계:

```text
IO completion thread
-> owned packet/message queue에 게시
-> simulation owner thread가 tick에서 소비
-> game state 변경
```

완료 callback이 게임 월드를 직접 바꾸면 race와 결정론 문제가 생긴다. Winters는 transport adapter와
30 Hz room owner 경계를 이 원칙으로 분리한다.

### 20.9 Encoding과 파일 경로

```text
Win32 W API: UTF-16 wchar_t
Win32 A API: process code page 의존
std::filesystem::path: platform-native path 표현 지원
```

경로 문자열, 파일 content encoding, source encoding은 서로 다른 계약이다.

---

## 21. Client, Server, Backend, Tool, Engine의 경계

### 21.1 Engine

제품에 독립적인 기반 기능:

- window/platform
- input
- renderer/RHI
- resource
- job system
- physics/navigation primitive
- audio
- generic UI/runtime/editor primitive

Engine이 특정 보스 이름이나 게임 규칙을 알면 dependency 방향이 역전된 것이다.

### 21.2 Client

- 입력 표현
- 카메라
- 시각 상태
- animation/FX/audio playback
- UI
- snapshot 보간과 약한 prediction

멀티플레이 게임에서 Client가 최종 HP/피해를 독단적으로 확정하면 cheating과 divergence 문제가 생긴다.

### 21.3 Game Server

- command 검증
- authoritative simulation
- collision/gameplay 결과
- AI command
- snapshot/event 생성
- match lifecycle

### 21.4 Backend Service

실시간 simulation과 별개인 서비스:

- 인증
- 계정/인벤토리
- matchmaking
- 결제
- leaderboard
- telemetry
- content configuration

일반적으로 HTTP/gRPC/database/message queue를 사용하며 frame 단위 latency보다 안정성, 확장성,
transaction, idempotency가 중요하다.

### 21.5 Shared/Contract

경계를 넘는 최소 데이터 계약:

- command schema
- snapshot/event schema
- stable ID
- serialization format
- deterministic simulation data

Shared가 Renderer, ImGui, DX11을 include하면 서버까지 presentation 의존성이 새어 들어간다.

### 21.6 Tool/Editor

- importer/converter
- content browser
- map/effect/material editor
- validation
- cooking
- profiler/debug visualization

Tool은 runtime이 소비할 검증된 계약을 생산한다. Runtime hot path가 authoring format을 매 프레임
파싱하게 하지 않는다.

### 21.7 Infra/Build/CI

- compiler/toolchain 설치
- dependency fetch
- build/test/package
- artifact/signing/deploy
- crash symbol 관리
- telemetry dashboard

소스가 맞아도 빌드와 배포가 재현되지 않으면 팀 제품은 완성되지 않는다.

---

## 22. LostArk 현재 프로젝트를 읽는 지도

### 22.1 물리 구조

```text
LostArk/
├─ Framework.sln
├─ Engine/
│  ├─ Public/          공개 Engine header
│  ├─ Private/         Engine implementation
│  ├─ Default/         Engine.vcxproj/.filters
│  ├─ External/        ImGui 등 외부 source
│  ├─ ThirdPartyLib/   Assimp/FMOD library와 runtime
│  └─ Bin/             Engine build output/shader
├─ Client/
│  ├─ Public/          Client class 선언
│  ├─ Private/         Client 구현
│  ├─ Default/         Win32 entry/resource/vcxproj
│  └─ Bin/
│     ├─ ShaderFiles/
│     ├─ DataFiles/
│     └─ Resources/
├─ EngineSDK/          UpdateLib이 재생성, Git 정본 아님
├─ Tools/
│  ├─ ModelAssetConverter/
│  ├─ LevelPlacementExtractor/
│  └─ AssetReview/
└─ UpdateLib.bat
```

### 22.2 최고 수준 소유권

```text
CMainApp
├─ CGameInstance singleton facade
│  ├─ Graphic_Device
│  ├─ Input_Device
│  ├─ Renderer
│  ├─ Prototype_Manager
│  ├─ Object_Manager
│  ├─ Level_Manager
│  ├─ Timer/Target/Light/Sound/Font/Picking...
│  └─ 각 subsystem을 unique_ptr로 소유
├─ Debug ImGuiLayer
├─ MapTool
└─ EffectTool
```

### 22.3 Prototype/Clone/Layer 수명

```text
Create() -> unique_ptr Prototype
Prototype_Manager map이 원본 단독 소유

Clone() -> shared_ptr runtime instance
Layer list가 GameObject 생존 소유
Renderer queue가 frame 동안 임시로 shared ownership
Level 종료 -> Layer 제거 -> 참조가 0이면 객체 파괴
```

### 22.4 `CGameInstance`의 본질

모든 기능을 직접 구현하는 God Object라기보다 subsystem을 소유하고 호출을 중계하는 facade다.
새 기능을 볼 때 `CGameInstance` 함수에서 끝내지 말고 실제 manager 구현까지 따라간다.

### 22.5 Binary asset

```text
FBX/glTF
-> ModelAssetConverter에서 Assimp 사용
-> .wmodel cook
-> CModel::Create
-> CWModelDecoder
-> CMesh/CMaterial/CBone/CAnimation
-> 기존 renderer
```

런타임 입구는 `CModel` 하나다. `CCookedModel/CBinaryAssetObject`는 레거시 검증 경로이고 신규
MapTool 기능의 권위 경로로 사용하지 않는다.

### 22.6 현재 구조를 이해할 추천 수직 추적 5개

1. `wWinMain -> CMainApp -> CGameInstance -> Present`
2. `Loader -> Add_Prototype -> Clone -> Layer -> Update`
3. `Valtan -> Body_Valtan -> Model -> Animation -> Shader`
4. `MapTool palette -> Picking -> placement record -> Clone -> Save/Reload`
5. `ModelAssetConverter -> wmodel -> Decoder -> GPU buffer`

---

## 23. Winters Engine을 읽는 지도

Winters는 LostArk보다 제품과 계층이 크게 분리되어 있다.

```text
Winters/
├─ Engine/            범용 runtime, RHI, renderer, resource, jobs
├─ Client/            LoL 제품 presentation/input/UI
├─ EldenRingClient/   별도 action RPG 제품 client
├─ Server/            authoritative game server/network
├─ Shared/GameSim/    deterministic gameplay truth와 schema
├─ Services/          서비스 영역
├─ Tools/             converter, harness, SimLab
├─ LoLEditor/
├─ EldenRingEditor/
├─ Data/
├─ Shaders/
├─ Replay/
├─ infra/
└─ cmake/
```

### 23.1 핵심 의존 방향

```text
Shared/GameSim
    ^
    |
Server -------- produces Snapshot/Event
                    |
                    v
Client ---------- presentation
  |
  v
Engine ---------- RHI/Renderer/Resource
```

정확한 규칙:

```text
Shared/GameSim must not include Engine/Client/Renderer/UI/ImGui/DX
Server must not depend on Client visual code
Engine must not include product-specific Client/Server types
Client may consume Engine and Shared contracts
```

### 23.2 Server authority 흐름

```text
Client Input
-> typed GameCommand
-> transport/session boundary
-> Server room owner tick
-> GameSim validates and mutates truth
-> Snapshot/Event/FX cue
-> Client interpolation/animation/FX/UI
```

### 23.3 LostArk와 Winters의 차이

| 축 | LostArk | Winters |
|---|---|---|
| 규모 | 수업 기반 단일 Client+Engine | 다제품 Client, Server, Shared, Tools |
| 객체 | Prototype/Clone/Layer OOP | OOP + ECS/GameSim/DOD 혼합 |
| 렌더 | DX11 concrete 중심 | DX11 유지 + RHI 다중 backend 방향 |
| 게임 권위 | 현재 local Client 중심 | Server authoritative |
| 에셋 | CModel 통합 wmodel 경로 | cook/registry/streaming/editor 계약 확장 |
| 빌드 | VS vcxproj + UpdateLib | VS/CMake/여러 target/harness |

### 23.4 배워야 할 핵심

Winters의 클래스 수를 외우는 것이 아니라 **dependency direction과 authority**를 읽는다.

```text
이 계산은 gameplay truth인가 presentation인가?
이 데이터는 process-local handle인가 저장 가능한 stable ID인가?
이 callback이 owner thread 밖에서 world를 바꾸는가?
이 JSON은 authoring source인가 runtime hot-path input인가?
```

참조:

- [WINTERS_CODEBASE_COMPASS.md](C:/Users/user/Desktop/Winters/.md/architecture/WINTERS_CODEBASE_COMPASS.md)
- [WINTERS_ENGINE_CONVENTIONS.md](C:/Users/user/Desktop/Winters/.md/architecture/WINTERS_ENGINE_CONVENTIONS.md)

---

## 24. Unreal Engine source를 읽는 지도

로컬 source root:
[Unreal Engine](C:/Users/user/Desktop/UnrealEngine/UnrealEngine)

### 24.1 최고 수준 구조

```text
Engine/Source/
├─ Runtime/     Shipping game에도 들어갈 수 있는 module
├─ Editor/      Unreal Editor 전용 module
├─ Developer/   개발/cook/분석 지원 module
├─ Programs/    ShaderCompileWorker, UnrealBuildTool 관련 도구 등
└─ ThirdParty/  외부 library
```

이것은 단순 폴더 정리가 아니라 build target과 dependency 경계다.

### 24.2 Module이 기본 단위다

Unreal의 대규모 코드는 class보다 module을 먼저 본다.

```text
Module.Build.cs
Public/
Private/
PublicDependencyModuleNames
PrivateDependencyModuleNames
DynamicallyLoadedModuleNames
```

로컬 [Engine.Build.cs](C:/Users/user/Desktop/UnrealEngine/UnrealEngine/Engine/Source/Runtime/Engine/Engine.Build.cs)는
Core, CoreUObject, RHI, RenderCore, NetCore 등 module dependency를 선언한다.

### 24.3 UObject, Actor, Component

```text
UObject
= reflection/serialization/GC ecosystem의 기본 객체

AActor
= World에 spawn되고 identity/transform/network lifetime을 갖는 객체

UActorComponent
= Actor에 붙는 재사용 가능한 기능/데이터

USceneComponent
= transform hierarchy를 가지는 component

UPrimitiveComponent
= render/physics 표현으로 확장되는 scene component
```

LostArk의 GameObject/Component와 겉모양은 비슷하지만 수명 계약이 다르다. Unreal의 `UObject*`를
일반 C++ raw pointer와 똑같이 해석하면 안 된다. Reflection property와 GC reference tracking 계약을
확인해야 한다.

### 24.4 Reflection macro

```text
UCLASS
USTRUCT
UENUM
UPROPERTY
UFUNCTION
GENERATED_BODY
```

이 macro는 단순 문법 장식이 아니다. UnrealHeaderTool이 metadata와 generated code를 만들고
serialization, editor property, Blueprint, replication, GC가 이를 사용한다.

### 24.5 Unreal 수명 질문

```text
일반 C++ object인가 UObject인가?
Outer는 누구인가?
UPROPERTY/GC reference로 추적되는가?
World/Level/Actor가 언제 파괴하는가?
BeginPlay/EndPlay와 constructor 차이는 무엇인가?
CDO(Class Default Object)와 runtime instance 중 무엇인가?
```

### 24.6 Thread와 Rendering

개념적 흐름:

```text
Game Thread world state
-> render proxy/scene data enqueue
-> Render Thread command generation
-> RHI Thread/backend
-> GPU
```

Game Thread UObject를 Render Thread에서 임의로 읽지 않는다. 필요한 immutable/render-owned data를
경계 너머로 전달한다.

로컬 RHI 계약은 [RHI.h](C:/Users/user/Desktop/UnrealEngine/UnrealEngine/Engine/Source/Runtime/RHI/Public/RHI.h),
DX12 구현은 `Runtime/D3D12RHI`에서 볼 수 있다.

### 24.7 Unreal asset 흐름

```text
source asset
-> import factory/interchange
-> UObject asset package
-> Derived Data Cache/cook
-> IoStore/Pak runtime container
-> Asset Registry/Async Loading
-> runtime resource
```

Editor source asset, `.uasset`, derived data, cooked package, GPU resource는 같은 것이 아니다.

### 24.8 Unreal code를 볼 때 첫 질문

```text
어느 module인가?
Runtime/Editor/Developer/Program 중 어디인가?
Build.cs dependency는 무엇인가?
UObject/GC 객체인가 plain C++ 객체인가?
Game Thread/Render Thread/RHI Thread 중 어디인가?
WITH_EDITOR, UE_BUILD_SHIPPING 조건부 경로가 있는가?
Reflection/serialization/replication 대상인가?
```

---

## 25. Unity 프로젝트를 읽는 지도

Unity 엔진 전체 native source는 일반 프로젝트처럼 모두 주어지는 것이 아니다. 회사에서는 보통 Unity
Project의 C# gameplay/tool code와 package/native plugin 경계를 읽는다.

### 25.1 일반 구조

```text
Project/
├─ Assets/
│  ├─ Scripts/
│  ├─ Editor/
│  ├─ Resources/ 또는 Addressable asset
│  ├─ Scenes/
│  └─ Plugins/
├─ Packages/
├─ ProjectSettings/
├─ UserSettings/       보통 개인 설정
└─ Library/            생성 cache, 보통 Git 제외
```

### 25.2 GameObject/Component

```text
GameObject = scene identity/container
Component  = Transform, Renderer, Collider, custom MonoBehaviour
```

Unity가 lifecycle method를 호출한다.

```text
Awake
-> OnEnable
-> Start
-> Update
-> LateUpdate
-> OnDisable
-> OnDestroy
```

물리 고정 시간 갱신은 `FixedUpdate`, 화면 프레임 갱신은 `Update`라는 schedule 차이가 중요하다.

### 25.3 ScriptableObject

Scene object가 아닌 재사용 가능한 serialized data asset/object다. Config와 runtime mutable state를
혼동하지 않아야 한다.

### 25.4 Unity serialization

Unity Inspector에 보이는 C# 필드는 일반 .NET serialization과 동일하지 않다. Unity가 지원하는 field와
reference 규칙을 따르며 domain reload, prefab, scene serialization을 함께 본다.

### 25.5 Assembly Definition

`.asmdef`는 compile unit와 dependency 경계다.

```text
Runtime assembly must not reference Editor assembly
feature module dependency는 단방향
compile time과 재빌드 범위를 줄임
```

### 25.6 Coroutine, Job, Burst, DOTS

- Coroutine은 일반적으로 main thread에서 여러 frame에 걸쳐 실행을 나누는 것; 자동 parallel thread가 아님.
- C# Job System은 dependency를 가진 worker job 실행.
- Burst는 제한된 C# subset을 native optimized code로 compile.
- DOTS/ECS는 component data를 chunk에 배치해 bulk processing.

### 25.7 Unity code를 볼 때 첫 질문

```text
MonoBehaviour lifecycle 중 언제 호출되는가?
Scene/Prefab/ScriptableObject 중 누가 instance를 만든가?
serialized field의 정본은 asset인가 runtime인가?
main thread API인가 job-safe API인가?
Editor folder/assembly가 player build에 포함되는가?
Resources/Addressables/AssetBundle 중 어떤 load contract인가?
```

---

## 26. 회사 규모 저장소를 보는 공통 지도

```text
Repository
├─ apps/ or products/       실행 제품
├─ engine/ or platform/     공용 기반
├─ client/                  사용자-facing application
├─ server/                  실시간/권위 server
├─ services/                backend microservices
├─ shared/ or contracts/    schema와 공용 순수 logic
├─ tools/ or editor/        authoring/debug/cook
├─ data/ or content/        정의 데이터
├─ shaders/                 GPU programs
├─ third_party/             외부 dependency
├─ build/ cmake/ scripts/   build orchestration
├─ infra/ deploy/           배포와 운영
├─ tests/ benchmarks/       검증
├─ docs/                    의사결정과 사용법
└─ generated/ intermediate/ 정본이 아닌 생성물
```

폴더 이름을 믿기 전에 build file과 include/import 방향을 검증한다.

### 26.1 모듈을 읽는 8개 질문

```text
1. 이 module의 제품 책임은 무엇인가?
2. public contract는 어디인가?
3. private implementation은 어디인가?
4. 상위/하위 dependency는 무엇인가?
5. authority와 persistent truth를 소유하는가?
6. thread/process boundary를 넘는가?
7. failure가 어디로 보고되는가?
8. test/benchmark/telemetry는 무엇인가?
```

### 26.2 Dependency 방향

좋은 방향:

```text
high-level policy -> stable abstraction <- low-level implementation plugged in
```

나쁜 신호:

- Engine이 Client boss class를 include
- Shared schema가 Renderer type을 include
- Server가 Client UI를 참조
- Runtime이 Editor singleton을 필수로 참조
- utility가 모든 module을 include
- circular project dependency

---

## 27. 데이터 파일과 직렬화

### 27.1 데이터 파일은 코드 밖의 계약이다

```text
magic
version
counts/offsets
payload
checksum(optional)
```

Reader는 신뢰하지 않는 입력으로 다룬다.

```text
parse
-> bounds/version 검증
-> semantic validation
-> temporary staged object 생성
-> 모두 성공하면 commit
-> 실패하면 rollback
```

### 27.2 Raw struct dump가 위험한 이유

- padding/alignment
- compiler ABI
- endianness
- pointer 포함
- bool/enum width
- version migration 없음

필드별 고정 폭 직렬화를 사용한다.

### 27.3 ID 세 종류를 분리한다

```text
Stable ID
저장/네트워크/팀 협업 경계를 넘어 유지

Dense runtime index
현재 pack/process 안에서 빠른 lookup

Pointer/Handle
현재 process lifetime에서 객체 접근
```

Vector index나 pointer를 save ID로 저장하지 않는다.

### 27.4 Catalog와 Placement

```text
Catalog
= 무엇을 생성할 수 있는가
+ stable asset ID
+ model path
+ kind/prototype definition

Placement
= 어떤 instance가 어디 있는가
+ placement ID
+ asset ID
+ transform
+ scene-specific state
```

Catalog가 scene transform까지 소유하거나 placement가 prototype pointer를 저장하면 경계가 섞인다.

---

## 28. Effective C++ 핵심 원칙

책의 항목을 암기하는 대신 아래 질문으로 압축한다.

### 28.1 Lifetime first

- 누가 자원을 소유하는가?
- 생성 성공 후 어떤 상태가 유효한가?
- 모든 return 경로에서 정리되는가?
- callback보다 capture한 객체가 오래 사는가?

### 28.2 Rule of Zero

- raw resource를 직접 소유하지 않는다.
- vector/string/unique_ptr/ComPtr/lock guard로 수명을 표현한다.

### 28.3 Interface를 작게

- public은 장기 dependency다.
- private 구현을 header에서 줄인다.
- const correctness로 변경 범위를 표현한다.

### 28.4 값 semantics 우선

- 작은 descriptor와 ID는 값으로 전달한다.
- 공유 가변 상태를 기본으로 만들지 않는다.
- pointer는 identity/lifetime이 필요할 때 쓴다.

### 28.5 Undefined behavior 회피

- dangling pointer/reference
- out-of-bounds
- uninitialized read
- signed overflow
- data race
- invalid downcast
- 잘못된 format string
- lifetime이 시작되지 않은 object를 임의 byte로 사용

### 28.6 `override`, `explicit`, `nullptr`

```cpp
explicit CTimer(float interval);
void Update(float dt) override;
CModel* p = nullptr;
```

compiler가 의도치 않은 변환과 signature 오류를 잡게 한다.

### 28.7 `noexcept`

함수가 예외를 던지지 않는 계약이다. 이동 연산의 `noexcept` 여부는 vector가 재할당 때 move를
선택하는 데 영향을 줄 수 있다. 무조건 붙이는 장식이 아니라 실제 보장이어야 한다.

### 28.8 Error contract를 하나로 이해

```text
HRESULT/expected/result   복구 가능한 경계 실패
assert                    programmer invariant 위반
exception                 stack unwinding 기반 오류 전달 정책
log                       관찰 수단, 처리 자체는 아님
```

프로젝트 정책을 섞지 않는다. LostArk는 주로 `HRESULT`, `nullptr`, `FAILED`를 사용한다.

### 28.9 성능 원칙

- 매 프레임 allocation을 피한다.
- reserve/reuse/pool은 측정과 lifetime 계약 뒤에 사용한다.
- `shared_ptr`를 편의상 모든 곳에 쓰지 않는다.
- `std::function`/virtual/hash를 hot path에서 무조건 나쁘다고 가정하지 말고 측정한다.
- layout과 access pattern을 함께 본다.

### 28.10 동시성 원칙

- mutable state owner를 하나 둔다.
- message/command로 owner thread에 전달한다.
- lock을 잡은 채 callback/external call/blocking I/O를 하지 않는다.
- thread shutdown과 작업 수명을 명시한다.

---

## 29. 디버깅의 제1원리

### 29.1 현상과 원인을 분리한다

```text
현상: 발탄이 안 보인다.
가설: Clone 실패 / render queue 미등록 / shader 실패 / camera 밖 / scale 오류.
증거: 각 경계의 pointer, HRESULT, count, matrix, draw call.
```

### 29.2 가장 가까운 관찰 지점부터

화면에 안 보이면 파일 import부터 다시 만들지 않는다.

```text
Render 호출됐나?
-> render queue에 들어갔나?
-> mesh count/index count가 0인가?
-> shader/resource bind가 성공했나?
-> world/view/projection이 유한한가?
-> camera/frustum 안인가?
-> 그다음 load/convert를 본다.
```

### 29.3 실패 전파가 끊긴 곳

하위 함수가 `E_FAIL`을 반환해도 상위 함수가 값을 무시하면 실패가 사라진다.

```cpp
object->Render(); // 반환값 무시
```

이런 boundary를 찾아 log/assert/result 집계를 붙인다.

### 29.4 최소 재현

- 입력 asset 하나
- object 하나
- animation 하나
- level 하나
- 동일 camera/transform
- deterministic command sequence

복잡한 scene 전체보다 원인을 빠르게 분리한다.

### 29.5 디버거에서 보는 순서

```text
Call Stack
-> current arguments
-> this pointer와 member invariants
-> return value/HRESULT
-> last writer/data breakpoint
-> thread list
-> modules와 loaded DLL path
```

---

## 30. 성능 문제를 읽는 제1원리

### 30.1 먼저 예산

```text
60 FPS = frame당 16.67 ms
120 FPS = frame당 8.33 ms
30 Hz server tick = tick당 33.33 ms보다 작아야 하지만 여유 필요
```

CPU와 GPU는 겹쳐 실행될 수 있으므로 단순 합이 아니라 timeline을 본다.

### 30.2 분류

```text
CPU simulation bound
CPU render submission bound
GPU vertex bound
GPU pixel/overdraw bound
memory bandwidth bound
I/O/streaming bound
lock/contention bound
network latency/backpressure
```

### 30.3 흔한 병목 냄새

- Update에서 파일/JSON 파싱
- frame마다 new/delete
- 모든 object가 모든 object를 검색하는 `O(N^2)`
- 동일 Model/Texture 중복 load
- GPU 결과를 CPU가 즉시 readback
- 작은 draw 수천 개
- 필요 없는 shared_ptr copy
- mutex 안에서 파일/network 작업
- 매 tick 전체 snapshot을 무제한 생성

### 30.4 최적화 완료 조건

```text
재현 scene와 hardware
측정 counter/scope
변경 전 수치
변경 후 수치
화질/정확성/메모리 대가
회귀 테스트
```

---

## 31. “천재 사고법”을 실제 절차로 바꾸기

천재는 많이 외운 사람이 아니라 중요한 변수만 남기는 사람처럼 행동한다.

### 31.1 압축

복잡한 코드에서 다음 한 줄만 먼저 만든다.

```text
누가 가진 어떤 데이터를, 어느 시점에, 어떤 규칙으로 바꾸고, 누가 소비하는가?
```

### 31.2 경계 찾기

버그와 설계 문제는 경계에서 자주 난다.

```text
header/cpp
Engine/Client
Client/Server
Editor/Runtime
CPU/GPU
thread/thread
source/cooked
Debug/Release
local/team machine
```

### 31.3 가설을 코드보다 먼저 적기

```text
가설 A: 객체가 Layer에 없다.
증거: layer count/Clone result.

가설 B: Render queue에 없다.
증거: Late_Update breakpoint/queue count.

가설 C: Shader 입력이 잘못됐다.
증거: RenderDoc/PIX/bind HRESULT.
```

### 31.4 불변식 찾기

```text
Model mesh count > 0
Bone index < bone count
Transform 값은 finite
Scale > 0
Stable ID unique
Layer owns every live GameObject
Only room owner mutates GameSim
```

불변식이 깨지는 최초 지점이 원인에 가장 가깝다.

### 31.5 빈도 곱하기

코드 한 줄의 위험도:

```text
cost per call * calls per frame * frames per second * object count
```

로딩 시 `O(N^2)` 한 번과 5,000개 객체의 매 프레임 `O(N)`은 판단이 다르다.

### 31.6 정본 하나

동일 사실을 여러 객체가 독립적으로 수정하면 동기화 문제가 생긴다.

```text
누가 authoritative owner인가?
나머지는 command를 보내는가, snapshot을 읽는가, cache인가?
```

### 31.7 확장보다 삭제 가능성을 본다

새 manager, cache, renderer, decoder를 추가하기 전에 기존 경로를 확장할 수 있는지 확인한다.
두 번째 경로는 두 번째 정본과 두 배의 검증 비용을 만든다.

---

## 32. 코드 리뷰 체크리스트

### 32.1 정확성

- 입력 범위와 null을 검사하는가?
- 실패 시 partial state가 남는가?
- overflow, underflow, NaN 가능성이 있는가?
- index/count/version이 검증되는가?
- 반환값이 무시되는가?

### 32.2 수명

- owner가 분명한가?
- raw pointer의 원본이 더 오래 사는가?
- callback capture가 dangling되지 않는가?
- shared_ptr cycle이 있는가?
- GPU/resource destruction 시점이 안전한가?

### 32.3 복잡도

- hot loop의 container와 순회 수는?
- 중첩 loop가 실제 N에서 안전한가?
- allocation과 string formatting이 frame마다 발생하는가?
- cache locality가 나쁜가?

### 32.4 경계

- public header 의존이 불필요하게 커졌는가?
- product type이 Engine/Shared로 새었는가?
- Editor code가 Release/runtime에 들어가는가?
- source asset을 runtime에서 직접 요구하는가?

### 32.5 동시성

- 어떤 thread가 호출하는가?
- mutable state owner가 하나인가?
- lock order가 일정한가?
- queue가 무한히 증가하는가?
- shutdown 중 pending job/callback 수명이 안전한가?

### 32.6 검증

- 재현 가능한 테스트인가?
- Debug와 Release 차이를 봤는가?
- clean clone/build가 되는가?
- 저장/재로드/실패 rollback을 확인했는가?
- 성능 변화는 수치로 증명했는가?

---

## 33. 클래스 분석 양식

```text
[Class]
이름:
모듈/물리 경로:
존재 이유:
없으면 사라지는 기능:

상속:
구현하는 interface:
소유하는 값:
소유하는 pointer:
빌려 쓰는 pointer/reference:
외부 subsystem:

생성자:
초기화 함수:
매 프레임 함수:
소멸/해제 함수:

public contract:
private invariant:
thread affinity:
lifetime owner:

입력 데이터:
출력/부작용:
저장/네트워크 경계:
CPU/GPU 경계:

가장 비싼 함수:
실패가 전파되는 곳:
한 문장 요약:
```

---

## 34. 함수 분석 양식

```text
[Function]
signature:
정의 파일:
직접 호출자:
호출 시점/스레드/빈도:

명시적 입력:
입력 소유권:
읽는 멤버/전역:
변경하는 멤버:
변경하는 외부 시스템:

guard/precondition:
branch/state transition:
내부 호출:

return type:
도메인 의미:
직접 소비자:
무시되는 경로:
실패 전파:

time complexity:
space/allocation:
불변식:

한 문장 요약:
```

---

## 35. 자료구조 선택 양식

```text
[Container Decision]
원소 타입:
예상 개수:
최대 개수:

가장 잦은 연산:
탐색 key:
정렬 필요:
중복 허용:
삽입/삭제 위치:
주소 안정성:
연속 메모리 필요:
멀티스레드 공유:
저장/네트워크 ID:

후보 1:
후보 2:
복잡도 비교:
allocation/cache 비교:
benchmark 조건:
최종 선택과 이유:
```

---

## 36. 실전 학습 순서: LostArk로 프레임워크를 갈아 마시는 법

한꺼번에 모든 파일을 읽지 않는다. 다음 수직 slice를 직접 설명하고 디버거로 증명한다.

### 단계 1: Process와 Frame

파일:

- [Client.cpp](C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp)
- [MainApp.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp)
- [GameInstance.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp)

완료 조건:

```text
wWinMain부터 Present까지 말로 설명
Update와 Render 호출 횟수를 breakpoint로 확인
WndProc와 polling input 차이 설명
```

### 단계 2: Ownership과 Lifetime

파일:

- `Prototype.h`
- `Prototype_Manager.cpp`
- `Object_Manager.cpp`
- `Layer.cpp`

완료 조건:

```text
Prototype unique_ptr owner 설명
Clone shared_ptr owner 설명
Level 종료 때 객체가 파괴되는 이유 설명
Renderer queue가 임시 수명을 연장하는 이유 설명
```

### 단계 3: Object/Component

파일:

- `GameObject.h/.cpp`
- `Component.h/.cpp`
- `Transform.h/.cpp`
- `Valtan.cpp`
- `Body_Valtan.cpp`

완료 조건:

```text
CValtan과 Body 분리 이유
Part parent matrix가 borrowed pointer인 이유
Component clone과 object clone 차이
```

### 단계 4: Asset과 Serialization

파일:

- `Tools/ModelAssetConverter`
- `Engine/Public/Private/BinaryAsset`
- `Model.cpp`
- `Material.cpp`
- `Mesh.cpp`

완료 조건:

```text
FBX authoring -> wmodel cooked -> runtime GPU buffer 설명
magic/version/count 검증 위치 설명
static/animated model 경로가 CModel에서 합쳐지는 이유 설명
```

### 단계 5: Animation

파일:

- `Animation.cpp`
- `Channel.cpp`
- `Bone.cpp`
- `Mesh.cpp`
- `Shader_VtxAnimMeshBinary.hlsl`

완료 조건:

```text
DeltaTime -> track position -> key interpolation -> Bone matrices -> GPU skinning
각 loop의 C/B/V 복잡도 계산
animation 종료 반환값 소비 위치 설명
```

### 단계 6: Renderer와 GPU

파일:

- `Renderer.cpp`
- `Shader.cpp`
- `VIBuffer.cpp`
- `Graphic_Device.cpp`
- Client shader files

완료 조건:

```text
render group 순서
vertex/index/constant/texture binding
draw call이 GPU work를 예약한다는 사실
Present까지 설명
```

### 단계 7: Tool과 Data

파일:

- `MapTool.cpp`
- `MapAssetCatalog.cpp`
- placement/catalog data
- `RuntimeAssetRoot.cpp`

완료 조건:

```text
Catalog definition과 Placement instance 분리
asset ID/placement ID/prototype tag 차이
parse -> validate -> stage -> commit/rollback 설명
```

### 단계 8: Winters로 확장

```text
LostArk의 local object truth
-> Winters의 Shared/GameSim authority
-> Server command/tick
-> Snapshot/Event
-> Client presentation
```

같은 기능을 두 저장소에서 추적하고 “class 이름”이 아니라 authority/lifetime/boundary 차이를 설명한다.

### 단계 9: Unreal로 확장

```text
LostArk GameObject/Component
vs Unreal Actor/ActorComponent/UObject

LostArk Engine DLL/vcxproj
vs Unreal Module/Build.cs

LostArk Renderer/DX11
vs Unreal Render Thread/RHI backend
```

### 단계 10: 스스로 재구현

다음 작은 실험을 별도 폴더에서 직접 만든다.

1. RAII File wrapper
2. unique/shared/weak lifetime 실험
3. vector/list 10만 원소 순회 benchmark
4. map/unordered_map lookup benchmark
5. binary file magic/version reader
6. BFS/Dijkstra/A* grid visualizer
7. fixed timestep loop
8. producer-consumer queue
9. CPU skinning 작은 예제
10. DX11 triangle -> textured mesh -> instancing

읽기만 하면 지식이고, 독립 구현과 디버깅까지 해야 능력이 된다.

---

## 37. 최종 압축 카드

처음 보는 코드 앞에서 이 순서만 떠올린다.

```text
1. Product       무엇을 실행/배포하는가?
2. Build         어떤 target이 어떤 target에 의존하는가?
3. Entry         실행은 어디서 시작하는가?
4. Schedule      언제, 어느 thread, 얼마나 자주 실행되는가?
5. Authority     최종 진실 데이터는 어디 있는가?
6. Ownership     누가 생성·소유·파괴하는가?
7. Data Flow     입력이 어떤 상태와 출력으로 바뀌는가?
8. Boundary      DLL/process/thread/CPU-GPU/file-network 중 무엇을 넘는가?
9. Cost          N, allocation, cache, sync, I/O 비용은?
10. Evidence     무엇을 측정/로그/테스트하면 맞다고 할 수 있는가?
```

Class를 보면:

```text
왜 존재하는가?
무엇을 소유하는가?
어떤 불변식을 지키는가?
누가 얼마나 오래 소유하는가?
```

Function을 보면:

```text
누가 언제 호출하는가?
무엇을 읽고 무엇을 바꾸는가?
무엇을 반환하는가?
반환과 부작용을 다음 누가 소비하는가?
```

자료구조를 보면:

```text
접근 패턴은 무엇인가?
Big-O뿐 아니라 allocation/cache/address stability는 어떤가?
```

알고리즘을 보면:

```text
어떤 상태를 어떤 정확성과 비용으로 바꾸는가?
입력 크기와 최악 상황은 무엇인가?
```

엔진을 보면:

```text
Data -> Simulation -> Presentation -> GPU/Audio
Authoring -> Validate -> Cook -> Load -> Runtime
Input -> Command -> Authority -> Snapshot/Event -> Client Visual
```

마지막 한 문장:

> 어떤 코드베이스도 결국 데이터, 상태 변환, 소유권, 실행 시간, 경계, 검증의 조합이다. 이름을 외우지 말고 이 여섯 축을 찾아라.
