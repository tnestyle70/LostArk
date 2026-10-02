# Bone.cpp Debug 컴파일 최적화 계획

## G01. 현재 상태와 이번 변경의 경계

대상은 `C:/Users/tnest/Desktop/LostArk/Engine/Default/Engine.vcxproj`의 기존
`..\private\Bone.cpp` 항목 하나다. C++ 구현, 헤더, 저장 데이터, shader 및 런타임 품질 설정은
바꾸지 않는다. 기존 Animation/Channel/Model/Profiler의 Debug 최적화와 같은 컴파일 옵션을
Bone.cpp에 적용하되 모든 새 metadata를 `Debug|x64`로 한정한다.

현재 도화가 Debug 캡처의 Animation.Channels.Update는 평균 11.516ms/frame이며,
channel마다 `Engine/Private/Bone.cpp`의 `CBone::Update_TransformationMatrix`를 호출한다.
combined pose 계산도 같은 파일의 `CBone::Update_CombinedTransformationMatrix`를 호출한다.
현재 Engine Debug 명령 기록에서 Animation.cpp와 Channel.cpp는 `/O2`이고 Bone.cpp는
`/Od /RTC1 /JMC /ZI`다. 이 차이가 몇 ms를 차지하는지는 아직 측정하지 않았다.

이 계획의 소스 반영은 FPS 개선 완료를 뜻하지 않는다. 현재 실행 중인 Client/Server는 유지하며,
이 변경을 포함한 컴파일·링크·동일 조건 캡처는 실행 파일 교체가 가능한 시점에 별도로 수행한다.

## G02. 프로젝트 항목의 정확한 교체 블록

변경 종류는 기존 `ClCompile` 항목 교체다. `..\private\Animation.cpp`의 닫는 `ClCompile`
다음, `..\Private\Bounding_OBB.cpp` 항목 바로 앞의 아래 한 줄이 기준점이다.

```xml
    <ClCompile Include="..\private\Bone.cpp" />
```

이를 다음 블록 전체로 교체한다.

```xml
    <ClCompile Include="..\private\Bone.cpp">
      <Optimization Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">MaxSpeed</Optimization>
      <BasicRuntimeChecks Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Default</BasicRuntimeChecks>
      <DebugInformationFormat Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">ProgramDatabase</DebugInformationFormat>
      <SupportJustMyCode Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">false</SupportJustMyCode>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">NotUsing</PrecompiledHeader>
      <ForcedIncludeFiles Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" />
    </ClCompile>
```

| 설정 | 이 항목에서 바뀌는 의미 |
|---|---|
| MaxSpeed | Bone.cpp의 Debug 코드를 `/O2`로 최적화한다. |
| Default runtime checks | `/O2`와 충돌하는 `/RTC1` 기본 검사를 이 파일에서 해제한다. |
| ProgramDatabase | 최적화 코드의 PDB 정보를 `/Zi`로 남긴다. `/ZI` Edit and Continue는 사용하지 않는다. |
| SupportJustMyCode=false | 이 번역 단위에서 `/JMC` 진입 계측을 사용하지 않는다. |
| NotUsing PCH | 다른 최적화 옵션으로 생성된 공용 Debug PCH를 재사용하지 않는다. |
| 빈 ForcedIncludeFiles | PCH 강제 include를 이 파일에서 해제한다. Bone.cpp의 명시적 Bone.h → Engine_Defines.h include를 사용한다. |

기존 주변 항목의 PCH 설정은 무조건부지만 이를 그대로 복사하지 않는다. Bone의 PCH와
ForcedIncludeFiles에도 Debug 조건을 붙여 Release의 기존 `Use` 및 강제 include를 보존한다.
기존 파일은 UTF-8 BOM 없음, CRLF 726개다. XML 전체를 재직렬화하지 않고 위 한 항목만 바꾼다.

## G03. 불변식과 디버깅상의 비용

- `_DEBUG`, `/MDd`, 기본 iterator debug level 2, `/fp:precise`, 타입·함수 선언과 메모리 배치는 유지한다.
- 전역 compiler 옵션, CRT/iterator ABI, Toolset v143, SDK 선택은 바꾸지 않는다. Release와 Win32 설정도 유지한다.
- Bone.cpp의 최적화로 local 변수가 사라지거나 breakpoint/step 순서가 소스와 다르게 보일 수 있다.
  이 파일의 Edit and Continue, Just My Code 계측 및 `/RTC1` 검사를 포기하는 선택이다.
  PDB와 `/MDd`는 유지되지만 같은 Debug 검사 범위를 그대로 제공한다고 설명하지 않는다.
- C++ 계산식과 호출 순서는 소스상 동일하다. 최적화된 바이너리의 포즈 동등성과 성능은 별도 실행 검증 대상이다.
- 새 C++ 파일이 없으므로 `.vcxproj.filters`, public interface 문서, 데이터 publisher 변경은 필요 없다.

## G04. 적용 순서와 검증

1. 이 PLAN의 블록을 검토하고 적용 전 Engine.vcxproj bytes 및 MSBuild 평가 metadata를 저장소 밖에 보존한다.
2. Bone.cpp 항목만 교체하고 XML parse, 정확한 항목 수, `git diff --check`를 확인한다.
3. MSBuild `-getItem:ClCompile -p:Configuration=Debug -p:Platform=x64`로 여섯 설정과
   RuntimeLibrary/PreprocessorDefinitions를 확인한다. target을 지정하지 않고 build를 실행하지 않는다.
4. 같은 명령의 Release 평가 결과에서 적용 전후 Bone.cpp의 전체 metadata가 동일한지 비교한다.
5. 기존 `Tools/Build/test_native_intermediate_contract.py`와 이번 project 항목에 직접 관련된
   build contract 검사를 실행한다. XML을 다시 읽는 것만으로 의미 없는 새 테스트를 만들지 않는다.
6. 사용자의 실행 검토가 끝난 뒤 정본 Product Debug 빌드에서 Bone.cpp의 실제 CL 옵션과
   Engine DLL → Client 복사 결과를 확인한다. 상위 작업이 이 빌드를 소유한다.

후속 정본 명령은 저장소 루트에서 실행한다. 현재 캡처 중에는 실행하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

후속 실행 검증은 같은 직업·Movie 구간·viewport·포커스·Detailed 설정으로 전후 capture를 비교한다.
Animation.Channels.Update와 Animation.Bones.Combine, frame interval, GPU 유효 표본을 구분하고
inclusive 시간을 더하지 않는다. 일반 재생, pause/resume, 앞뒤 seek, loop에서 visible pose와
attachment가 유지되는지도 확인한다. 변경 전후 조건이 다르거나 실행하지 않은 검사는 PASS로 적지 않는다.

## G05. 별도로 남기는 반복 포즈 조사

도화가 animated 38개 WModel의 skeleton 총합은 7,718 bones다. 이 중 237 bones를 가진 모델
32개는 재질별로 분리된 9개 그룹이며, 그룹 안의 skeleton/animation bytes와 sampling 계약이
동일하다. 반복 평가량은 최적화 후보의 구조적 근거이며 이번 Bone 컴파일 설정 변경과는 별개다.
포즈 공유·숨긴 actor 생략·CModel 상태나 데이터를 변경하지 않는다.

실제 입력 경로, 섹션 hash, 그룹, sampling 계약과 추론 한계는 저장소 밖
`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/Movie-Artist-PoseDuplication.json`
에 보존한다. 이 파일의 section hash는 조사를 재현하기 위한 증거이며 Resources 배포 완료 조건이 아니다.

연결 기록: [취업준비 1일차 PLAN](2026-10-02_JOB_PREPARATION_DAY01_PLAN.md),
[취업준비 1일차 RESULT](2026-10-02_JOB_PREPARATION_DAY01_RESULT.md).
