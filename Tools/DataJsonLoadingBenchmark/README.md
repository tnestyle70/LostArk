# DataJson loading benchmark

실제 `DataJson.h/.cpp`와 Client/EngineSDK 헤더를 사용하는 독립 콘솔 측정 도구다. Client나 UI를 실행하지 않으며 `.vcxproj` 등록은 필요 없다. native probe는 2026-10-04 실측 소스와 동일하다.

## 고정 입력과 빌드

저장소 루트의 PowerShell에서 실행한다. 출력 디렉터리는 기존 파일 보호를 위해 새 경로를 사용한다.

```powershell
# 현재 저장된 쿠크 V1 사용 목록과 원본 DataJson 소스를 한 번만 snapshot한다.
./Tools/DataJsonLoadingBenchmark/Prepare-Corpus.ps1 -OutputDirectory out/DataJsonBench/input

./Tools/DataJsonLoadingBenchmark/Build-Benchmark.ps1 `
  -SourceHeader out/DataJsonBench/input/baseline/DataJson.h `
  -SourceCpp out/DataJsonBench/input/baseline/DataJson.cpp `
  -OutputDirectory out/DataJsonBench/baseline-Debug -Configuration Debug -Variant baseline

# 후보의 실제 header/cpp 경로를 별도로 지정한다. Release도 같은 입력으로 별도 빌드한다.
./Tools/DataJsonLoadingBenchmark/Build-Benchmark.ps1 `
  -SourceHeader out/DebugEffectLoading20261004/candidate-v2/Client/Public/DataJson.h `
  -SourceCpp out/DebugEffectLoading20261004/candidate-v2/Client/Private/DataJson.cpp `
  -OutputDirectory out/DataJsonBench/candidate-v2-Release -Configuration Release -Variant candidate-v2
```

기본값은 `vswhere`로 찾은 VS의 MSVC 14.44 / Windows SDK 10.0.26100.0이다. 필요하면 `-VcVarsPath '.../VC/Auxiliary/Build/vcvars64.bat' -ToolsetVersion 14.44 -WindowsSdkVersion 10.0.26100.0 -RepoRoot '...'`로 지정한다. 실제 헤더와 cpp를 빌드 폴더에 byte 그대로 복사해 조합을 고정하고 SHA256, flags, 명령, 로그를 저장한다. Debug는 `/O2 /MDd /D_DEBUG /D_ITERATOR_DEBUG_LEVEL=2`, Release는 `/O2 /MD /DNDEBUG /D_ITERATOR_DEBUG_LEVEL=0`을 사용한다.

## 실행

```powershell
./Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 `
  -Executable out/DataJsonBench/baseline-Debug/benchmark.exe `
  -Corpus out/DataJsonBench/input/corpus.tsv -Workers 3 `
  -Result out/DataJsonBench/results/baseline-debug3-a1.json

# 할당 계수는 별도 Debug 1-worker pass로 수집하고 시간 비교에서는 제외한다.
./Tools/DataJsonLoadingBenchmark/Run-Benchmark.ps1 `
  -Executable out/DataJsonBench/baseline-Debug/benchmark.exe `
  -Corpus out/DataJsonBench/input/corpus.tsv -Workers 1 -Allocation `
  -Result out/DataJsonBench/results/baseline-allocation.json
```

기존 `out/DebugEffectLoading20261004/corpus.tsv`도 그대로 전달할 수 있다. TSV는 `asset ID<TAB>JSON 경로`이며 절대 경로나 TSV 디렉터리 기준 상대 경로를 지원한다. 기존 입력 snapshot은 재생성하지 않는다. 새 corpus는 현재 저장본을 수집하므로 파일 수를 168 등으로 고정하지 않는다. 수집 범위는 **KoukuSaydon saved V1 closure**로, `Collect_ProductEffectTargets`의 V1 분기다. Valtan 전체 effect 측정이 아니며 해당 소비자가 바뀌면 준비 스크립트도 대조한다. 적용 이후 현재 제품 소스를 측정하려면 `-SourceHeader Client/Public/DataJson.h -SourceCpp Client/Private/DataJson.cpp`를 지정한다. 이전 버전과 비교할 때는 보존된 이전 소스 snapshot을 사용한다.

모든 JSON byte를 **메모리에 먼저 읽은 뒤** parse, semantic traversal, destruction을 별도 phase로 측정한다. worker별 DOM 하나를 유지하고 1명 또는 3명씩 wave로 처리한다. parse wall에는 phase 동기화와 root 생성이 포함되며 semantic digest 순회는 제외된다. process CPU는 세 phase 전체 값이다. 전체 effect load, codec decode, GPU 준비, 첫 화면 시간으로 해석하지 않는다. native 출력의 `unmodified`는 선택한 snapshot 소스 안에 계측을 삽입하지 않는다는 뜻이며 baseline과 candidate가 같은 소스라는 뜻이 아니다.

동일 corpus/flags에서 A/B/A 순서로 제한된 횟수만 실행하고 컴파일 등 큰 작업을 겹치지 않는다. 모든 결과와 범위를 보존하며 환경 변동을 성능 향상으로 확정하지 않는다. allocation은 parse 중 Debug CRT `_HOOK_ALLOC` 요청 수·요청 byte 합계이고 peak memory가 아니다. FNV1a64 semantic digest는 진단용이며 입력 무결성은 SHA256으로 기록한다. 기본 25계약은 malformed/limits/output 보존/copy/move/order를 검사한다. 결과에 실패가 있거나 semantic 필드가 달라지면 속도 비교보다 원인 확인이 먼저다.

## 값 소유권과 실패 보존 계약

`ValueContracts.cpp`는 2026-10-04에 실행한 별도 native 값 계약 소스와 byte 단위로 같다. `Run-ValueContracts.ps1`은 공용 `Build-Benchmark.ps1 -Probe ValueContracts`를 사용해 실제 DataJson과 Client/EngineSDK 헤더를 새 출력 폴더에 고정하여 빌드하고 검사한다. 컴파일 flags와 도구체인 선택은 benchmark와 같으며, 속도 비교를 위한 실행이 아니다.

```powershell
# 경로를 생략하면 현재 제품 DataJson.h/.cpp를 사용한다.
./Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 `
  -OutputDirectory out/DataJsonContracts/current-Debug -Configuration Debug

# 특정 후보의 header/cpp를 반드시 함께 지정한다.
./Tools/DataJsonLoadingBenchmark/Run-ValueContracts.ps1 `
  -SourceHeader out/DebugEffectLoading20261004/candidate-v2/Client/Public/DataJson.h `
  -SourceCpp out/DebugEffectLoading20261004/candidate-v2/Client/Private/DataJson.cpp `
  -OutputDirectory out/DataJsonContracts/candidate-v2-Debug `
  -Configuration Debug -Variant candidate-v2 -RequireStrongCopy
```

기본 검사는 모든 타입 간 copy/move 대입, factory와 잘못된 타입 getter, 입력 순서, 중첩 deep copy의 독립성·수명, moved-from 객체의 읽기와 재사용, self-move 후 유효한 재대입을 확인한다. self-move 뒤 원래 값까지 보존하도록 요구하지 않는다. `-RequireStrongCopy`는 native의 `--require-strong-copy`로 전달되어 소유 자식으로부터의 복사 대입과, Debug에서는 복사 중 각각의 할당 실패 지점에서 이전 목적지 보존을 추가한다. 이 강화된 보장을 제공하지 않는 옛 소스에는 옵션을 자동 적용하지 않는다. Release에서는 Debug CRT 실패주입이 실행되지 않는다.

출력에는 source snapshot과 `build-receipt.json`, `compile.log`, `value-contracts.exe`, `results.txt`, `results.json`이 남는다. 기존 출력 경로를 덮어쓰지 않으며, 결과 JSON은 실행 인자·EXE hash·iterator 수준·검사 수·주입한 할당 실패 수와 종료 코드를 기록한다. Client/UI를 실행하지 않는다. 성능 benchmark가 실행 중일 때 이 도구의 컴파일이나 실패주입 검사를 겹치지 않는다.
