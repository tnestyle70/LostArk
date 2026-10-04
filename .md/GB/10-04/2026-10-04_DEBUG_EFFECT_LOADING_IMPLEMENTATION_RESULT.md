# Debug Effect JSON 로딩 최적화와 Raid Publish 검토 결과

## 반영 범위와 현재 상태

`origin/main`의 `69a4e48f6272eb215d7d4999543f81926b77481d`로 local main을 동기화하고 `codex/debug-effect-loading`에서 작업했다. 이전 브랜치의 미커밋 설명 문서 5개는 byte snapshot과 SHA-256를 보존한 뒤 그대로 옮겼으며 이번 변경에 포함하지 않는다.

제품 변경은 `Client/Public/DataJson.h`, `Client/Private/DataJson.cpp`의 내부 객체 소유·생성 방식이다. Debug `/O2 /MDd`, `_DEBUG`, iterator debug level 2와 기존 JSON 검증을 유지했다. Release의 별도 parser나 두 번째 Effect runtime을 추가하지 않았다. JSON 형식, authored/runtime 데이터, publisher, worker 수, 렌더링 설정은 변경하지 않았다.

최종 후보 v2는 제품 소스에 반영했다. 여기서 v1/v2는 이번 최적화 후보의 순번이며 Effect Tool V1/V2 구분과 다르다. 성능·값 계약·실제 Effect codec 소비자 검증을 완료했으며 Product Debug 빌드 상태는 아래 별도 항목에서 기록한다. Client/UI와 Server를 실행하지 않았다.

전체 최종 코드는 [PLAN](2026-10-04_DEBUG_EFFECT_LOADING_IMPLEMENTATION_PLAN.md), 두 레이드의 실제 저장·게시 호출 경로와 미구현 캐시 제안은 [Publish 검토](2026-10-04_DEBUG_EFFECT_LOADING_PUBLISH_REVIEW.md)에 둔다.

## 같은 parser인데 Debug가 느린 이유

`CEffectDocumentCodec::Parse`는 Debug와 Release 모두 `CDataJson::Parse → Parse_Value`를 호출한다. Debug의 DataJson과 Effect codec에는 이미 `/O2`가 적용되어 있었다. 이번 작업에서 최적화 옵션을 처음 켠 것이 아니다.

`/O2`는 C++ 코드 최적화 옵션이고 `/MDd`는 Debug CRT 선택이다. 설치 MSVC 14.44의 Debug STL은 컨테이너 proxy와 iterator 검사를 사용하며, Debug CRT는 할당 정보를 기록하고 guard bytes 등을 관리한다. `Client.cpp`는 `_CRTDBG_ALLOC_MEM_DF`를 설정한다. 매 할당마다 전체 힙을 검사하는 `_CRTDBG_CHECK_ALWAYS_DF`가 설정되어 있다는 주장은 하지 않는다. 여러 worker가 같은 Debug heap의 잠금을 거치므로 worker 증가가 그대로 처리량 증가로 이어지지 않을 수 있다. Release allocator에도 동기화가 있으므로 lock-free라고 설명하지 않는다.

이 차이는 같은 소스를 서로 다른 검사·런타임 설정으로 컴파일한 결과다. STL 타입을 직접 주고받는 현재 DataJson 경계에서 parser OBJ 하나만 `/MD`와 iterator level 0으로 바꾸어 기존 Debug OBJ에 섞지 않는다. Release helper를 별도 ABI 경계로 분리하는 설계는 가능하지만 이번 변경이 아니다.

근거 위치: `Client/Private/Effect_DocumentCodec.cpp:252`, `Client/Default/Client.vcxproj:706`, `Client/Default/Client.vcxproj:1208`, `Client/Default/Client.cpp:289`, `Engine/Public/Engine_Defines.h:61`. 설치 CRT/STL의 map 이동 생성, proxy 할당, debug heap lock 경로도 대조했다. 과거 32.695/87.466초의 정확한 비율을 이 요인 하나의 기여도로 역산하지 않는다.

## 실제 변경과 보존한 계약

기존 활성 payload variant는 09-21에 이미 반영된 상태였다. 이번에는 OBJECT의 `map + insertionOrder`를 단독 owner인 `unique_ptr<OBJECT_PAYLOAD>`로 보유하도록 바꿨다. JSON 객체가 이동할 때 map 자체의 sentinel/proxy를 다시 준비하지 않고 owner를 넘긴다. 복사할 때는 자식까지 독립적으로 복사하며, 복사 대입은 임시 복사 성공 뒤 commit한다.

또한 private `DATA_JSON_READER`가 최종 OBJECT payload 안에 직접 key/value를 채운다. parser의 임시 map을 Object factory 인자와 payload로 옮기던 비용을 없앴다. 공개 mutable getter는 추가하지 않았다. `CDataJson::Parse`의 외부 출력은 여전히 별도 root를 완성한 뒤에만 교체되므로 문법·제한 검사 실패 시 기존 값을 보존한다. duplicate key와 오류 우선순위도 유지했다.

이동 후 OBJECT owner가 비어 있을 수 있다. 이 상태의 const getter는 빈 컨테이너를 반환하며 Find는 null을 반환한다. 복사·재사용·파괴할 수 있다. variant의 실제 nothrow move trait를 static_assert로 확인하며 map 자체의 이동에 noexcept를 임의로 붙이지 않았다.

## 실측 조건

- 기준/후보 모두 실제 DataJson 소스와 Client/EngineSDK 헤더를 사용했다. parser stub은 없다.
- **현재 저장된 KoukuSaydon V1 사용 범위 168개**, 255,638,203 bytes, JSON 값 7,853,764개를 고정했다. Valtan 전체 corpus, V2, 미저장 draft는 이번 속도 측정에 포함하지 않았다.
- MSVC 19.44.35228, Windows SDK 10.0.26100.0, x64. Debug `/O2 /MDd /D_DEBUG /D_ITERATOR_DEBUG_LEVEL=2`; Release `/O2 /MD /DNDEBUG /D_ITERATOR_DEBUG_LEVEL=0`.
- AMD Ryzen AI 5 340, 6 cores / 12 logical processors, 16 GB RAM, Windows 11 build 26200.
- 모든 파일을 RAM에 먼저 읽었다. 지속 worker 1개 또는 3개가 wave별로 parse → semantic digest → destruction을 처리한다. 각 worker는 한 DOM씩 유지한다.
- parse wall에는 root 생성과 phase 동기화가 포함되고 digest 순회는 제외된다. 파괴 시간은 별도다. process CPU는 세 phase 전체 값이며 parse CPU만의 값이 아니다.
- v2 비교는 구성별 A/B/A/B 각 2회다. 시간 측정 동안 팀의 컴파일을 중단했다. 다른 앱·주파수·시스템 부하는 완전히 통제하지 않았으므로 소수 표본의 관측값이며 통계적 유의성을 주장하지 않는다.
- 할당 hook 측정은 별도 Debug 1-worker pass다. 그 실행 시간은 성능 표에서 제외했다.

## 최종 v2 비교

각 칸은 2회 중앙값, 단위는 초다. 해제 비용도 포함해 개선폭을 판단했다.

| 구성 | 기존 parse | v2 parse | 기존 해제 | v2 해제 | parse+해제 감소 |
|---|---:|---:|---:|---:|---:|
| Debug / 1 worker | 13.806 | 11.513 | 3.721 | 3.869 | 12.24% |
| Debug / 3 workers | 17.327 | 14.082 | 4.970 | 5.117 | 13.89% |
| Release / 1 worker | 2.457 | 2.183 | 0.878 | 0.842 | 9.30% |
| Release / 3 workers | 1.977 | 1.704 | 0.732 | 0.764 | 8.86% |

Debug 3-worker parse만 보면 18.72% 감소했다. 이 구성의 전체 phase process CPU는 43.500초에서 37.766초로 약 13.2% 감소했다. 해제는 조금 느려졌지만 parse+해제 합계도 개선되었다. Release 두 표본에서는 퇴행을 관측하지 않았으나 정확한 개선율이 모든 환경에서 재현된다고 보장하지 않는다.

| Debug parse 할당 계수 | 기존 | v2 | 감소 |
|---|---:|---:|---:|
| `_HOOK_ALLOC` 요청 수 | 65,331,830 | 52,031,170 | 20.36% |
| 요청 bytes 누계 | 3,545,843,878 | 2,503,212,946 | 29.40% |

요청 bytes 누계는 peak memory가 아니다. `sizeof(DATA_JSON_VALUE)`는 Debug 88→72, Release 72→64 bytes다. 이 내부 배치 변경 때문에 헤더 의존 TU 전체를 같은 헤더로 재컴파일한다.

모든 timed/untimed 실행에서 168개 parse 성공, 25개 기본 계약 통과, per-file ID/bytes/성공/오류/nodes/digest 일치를 확인했다. 전체 semantic FNV1a64는 `d1c424a3520c3cb3`다. FNV는 진단용이며 입력 snapshot 무결성에는 SHA-256를 사용했다.

### v2 원자료: parse / 해제 / 전체 phase process CPU

A는 기존 main, B는 v2다. 단위는 초다.

| 구성·실행 | parse | 해제 | process CPU |
|---|---:|---:|---:|
| Debug1 A1 | 13.914 | 3.683 | 17.891 |
| Debug1 B1 | 11.270 | 3.788 | 15.625 |
| Debug1 A2 | 13.698 | 3.758 | 17.734 |
| Debug1 B2 | 11.755 | 3.949 | 16.234 |
| Debug3 A1 | 17.299 | 4.968 | 42.844 |
| Debug3 B1 | 14.070 | 5.077 | 37.375 |
| Debug3 A2 | 17.355 | 4.972 | 44.156 |
| Debug3 B2 | 14.094 | 5.158 | 38.156 |
| Release1 A1 | 2.597 | 0.950 | 4.172 |
| Release1 B1 | 2.135 | 0.874 | 3.609 |
| Release1 A2 | 2.318 | 0.806 | 3.703 |
| Release1 B2 | 2.231 | 0.810 | 3.562 |
| Release3 A1 | 2.092 | 0.809 | 5.547 |
| Release3 B1 | 1.658 | 0.782 | 4.953 |
| Release3 A2 | 1.861 | 0.655 | 4.750 |
| Release3 B2 | 1.751 | 0.746 | 5.094 |

처음의 v1은 OBJECT owner만 분리한 후보였다. 할당은 11.27% 감소했지만 Debug3 parse+해제 중앙값 14.701→15.008초로 속도 개선이 입증되지 않아 그 상태로 제품에 적용하지 않았다. v2는 최종 owner에서 직접 파싱하는 변경까지 포함한다. v1 블록과 v2 블록의 baseline 절대 시간이 다르므로 서로 다른 블록의 최솟값을 골라 개선율을 계산하지 않는다. 이전 09-20/09-21의 파일 수와 측정 단계도 달라 직접 비교하지 않는다.

## 기능 검증과 제품 빌드

별도 값 계약 probe에서 baseline Debug/Release 각 192 checks, v1/v2 Debug 각 260, v1/v2 Release 각 194 checks가 통과했다. Debug 260개에는 복사 중 31개 할당 지점 각각에 bad_alloc을 주입하여 기존 대입 대상이 보존되는 검사, 자식 참조에서 부모로 복사하는 alias 검사, 이동 후 재사용 검사가 포함된다.

보존한 portable `Run-ValueContracts.ps1`로 현재 제품을 다시 빌드·실행하여 Debug 260/Release 194 checks, failures 0을 확인했다. `Prepare-Corpus.ps1`도 Windows PowerShell에서 실제 실행하여 168개/255,638,203 bytes와 기존 고정 입력의 per-file SHA-256가 일치함을 확인했다. 이 과정에서 기본 경로의 초기화 위치와 ordered dictionary를 Measure-Object로 합산하던 호환 오류를 수정했다. 새로 준비한 corpus의 baseline source는 현재 제품이므로 최초 성능 비교의 기준 소스로 소급 사용하지 않는다.

실제 Effect codec CPU 소비자 비교도 통과했다. baseline/후보 각각 168개 모두 `CDataJson::Parse → Parse_Value → Validate → Validate_Drawable`을 승인했다. canonical JSON 168개, resource ID 목록 168개, report를 합친 337개 출력 파일이 SHA-256로 일치했다. 13개 codec TU와 재질 상수표·registry는 실제 소스를 사용했다. 불필요한 GPU 링크를 제거하기 위해 Playback/World의 순수 CPU validator와 helper만 원문 그대로 추출했으며 원본 SHA·범위를 `codec/*/extraction.json`에 기록했다. 승인만 반환하는 validation stub은 없다. 이 probe는 `/MDd _DEBUG /Od`로 기능 동등성을 검증했으며, 성능 표의 `/O2` parser probe와 별개다. `codec/receipt.json`과 `codec/candidate-v2-Debug/comparison.json`이 증거다. renderer 준비·GPU draw·전체 제품 실행 성공으로 확대하지 않는다.

Product Debug 빌드를 먼저 실행하여 Engine·Shared·Server 빌드 완료를 확인했다. Client는 main 동기화로 갱신된 대규모 FxCompile 단계가 계속되어, 이번 코드 검증 범위를 C++ 컴파일·링크로 좁히고 작업 소유가 확인된 MSBuild/FXC 자식 프로세스만 중단했다. Client/Server 제품 프로세스는 실행하거나 종료하지 않았다. `out/BuildPipeline/runs/20261004T060100476Z-debug-product.json`의 Client 실패는 이 명시적 중단 결과이며, **전체 Product 빌드 성공이 아니다**. 실행 Shader/CSO 전체 최신성이나 화면 성공을 인증하지 않는다.

이후 같은 Client 프로젝트와 Debug 설정으로 아래 C++/리소스/링크 target을 실행하여 exit 0을 확인했다. 빌드 전 Debug CL.read tlog에서 기록한 DataJson.h 의존 TU 217개 모두 적용 시각 이후 OBJ로 갱신됐다(217/217). Debug Client.exe 링크와 런타임 의존 DLL 배포도 완료했다. 증거는 `out/DebugEffectLoading20261004/product-cpp-verification.json`과 `client-cpp-diagnostic.log`다. PCH와 tracking 파일을 삭제하거나 단일 DataJson OBJ만 기존 Client에 섞지 않았다. C++ 링크 성공은 중단한 전체 Shader/CSO 빌드 및 사용자 화면 검증을 대신하지 않는다.

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64/MSBuild.exe' `
  Client/Default/Client.vcxproj `
  '/t:PrepareForBuild;ResolveReferences;_ClCompile;_ResourceCompile;_Link;_Manifest;DeployClientRuntimeDependencies' `
  /m:1 /nodeReuse:false /p:Configuration=Debug /p:Platform=x64 `
  /p:PreferredToolArchitecture=x64 /p:WindowsSDKToolArchitecture=Native64Bit `
  /p:BuildProjectReferences=false /p:CL_MPCount=3 /v:minimal `
  '/bl:out/DebugEffectLoading20261004/client-cpp.binlog' `
  '/flp:LogFile=out/DebugEffectLoading20261004/client-cpp-diagnostic.log;Verbosity=diagnostic;Encoding=UTF-8'
```

## 재현과 남은 범위

재현 도구는 `Tools/DataJsonLoadingBenchmark/README.md`의 Build/Run/ValueContracts 명령을 따른다. 독립 콘솔 probe로만 빌드하며 Client 프로젝트와 filters에 등록하지 않는다. 실행한 native benchmark와 값 계약 소스를 보존했다.

현재 PC의 원자료는 `out/DebugEffectLoading20261004`에 있다: `summary.json`은 v1, `summary-v2.json`은 v2, `results/*.json`은 per-file 실측, sidecar는 EXE/source/corpus 식별 증거, `value-contracts-results.json`은 값 계약 결과다. 후보 source와 모든 benchmark build receipt도 보존했다. `out`과 컴파일 산출물은 커밋하지 않는다.

**Debug가 Release와 같은 속도가 된 것은 아니다.** lazy loading은 필요한 패턴의 리소스만 준비하는 범위 정책이고, 이번 변경은 개별 JSON을 처리하는 비용을 줄이는 최적화다. GPU 리소스 생성과 renderer 준비, 실제 Play Preview 첫 화면까지의 시간은 이번 probe에서 측정하지 않았다.

KoukuSaydon·Valtan Publish에 정적 Effect 준비 산출물을 추가하는 방향은 검토했다. 현재 Publish를 다시 실행하는 것만으로 CDataJson parse가 사라지지는 않는다. 원본 hash·cooker/schema version·필요한 의존 hash에 연결된 산출물을 만들고 기존 Client 소비자가 읽도록 연결해야 한다. 이번 작업에는 새로운 cooked 형식·cache reader·Publish 변경을 구현하지 않았다. 상세 차이와 실제 삽입 지점은 연결한 Publish 검토 문서가 정본이다.
