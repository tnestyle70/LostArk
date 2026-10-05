# Unreal 참조 렌더 제출 최적화 결과

## G00. 반영 위치와 범위

작업 브랜치는 `codex/unreal-render-optimization`이다. 변경은 현재 Visual Studio 프로젝트가 있는
`C:/Users/tnest/Desktop/LostArk`에 반영했다. 다른 세션의 PR530 병합 완료와 clean 상태,
다른 제품 빌드·Client/Server 실행 부재를 확인한 뒤 최신 main `e38867de` 위에 적용했다.
초기 개발·독립 검증 폴더 `C:/Users/tnest/.codex/worktrees/unreal-render-optimization/LostArk`는
초기 후보의 detached checkout과 빌드 증거를 보존한다.

최종 제품 변경은 `Model.h`, `Model.cpp`, `MapStaticBatchObject.cpp`의 RNM 중복 재질 검사 축소다.
큰 shader raw 상수 캐시는 검토·검증 후 미반영으로 결정했으며 `Shader.h`와 `Shader.cpp`는
최종 변경 대상에서 제외했다. 기존 작은 값 캐시와 program variant 경로는 유지한다.
LOD index range·instance payload·draw 순서·HLSL 수식·팀장의 rendering option은 변경하지 않았다.
이번 변경은 CPU 제출 준비 비용을 줄이는 작업이다. draw 수·GPU 시간 감소나 게임 FPS 향상을 입증한 결과가 아니다.

전체 적용 코드는 같은 폴더의
`2026-10-05_UNREAL_RENDER_SUBMISSION_OPTIMIZATION_IMPLEMENTATION_PLAN.md`에 보존했다.
새 제품 파일이 없어 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## G01. Shader.h / Shader.cpp — 큰 상수 캐시 검토·검증 후 미반영

기존 `CShader::Bind_RawValue`의 64바이트 이하 캐시를 65~4096바이트까지 확장하는 후보를 검사했다.
후보는 길이와 전체 byte가 같을 때 Effect setter와 revision 증가를 생략하며,
native material의 1024바이트 상수 배열도 비교 대상으로 삼았다. 아래 설계·검증·측정은
이 후보에 대한 기록이다. 최종 제품에는 큰 raw 캐시를 추가하지 않는다.

후보의 `VARIABLE_BINDING::LargeRawValue`는 해당 Effect의 마지막 성공 raw write를 소유했다.
`iLargeRawValueCapacity`는 할당 용량이고, `iLastValueBytes`는 유효 비교 길이다.
`EFFECT_BINDINGS::iLargeRawCacheBytes`는 유지 중인 추가 payload 합계를 최대64KiB로 제한했다.
이는 binding 구조체 전체 메모리나 재할당 중 일시 peak의 엄격한64KiB 제한이 아니다.
성장 중에는 이전 buffer와 새 buffer가 잠시 함께 존재할 수 있다.

Clone은 동일 `EFFECT_BINDINGS`를 공유하므로 다른 clone의 성공 write를 함께 관찰한다.
setter 호출 전에 VALUE_KIND를 무효화하며 실패를 캐시하지 않는다.
Matrix/MatrixArray 전환과 variant 직접 raw write도 기존 무효화를 유지한다.
예산 초과·할당 실패는 성공한 원래 Effect write를 보존하고 cache만 생략한다.
매번 값이 달라지는 입력에는 비교·cache 복사 비용이 추가된다.

후보 단계에서 확인한 검증:

- 실제 production 함수/struct를 추출한 MSVC14.44 `/O2 /W4 /WX` fixture: 644 assertions PASS.
- 반복1024B, Clone 공유, 길이/종류 전환, NaN payload·signed zero, setter 실패·직접 write 무효화,
  변수4KiB/Effect64KiB 한도, 할당 실패·성장 재시도 확인.
- 후보 Shader.cpp의 실제 worktree 헤더를 사용한 `/Zs` 컴파일 PASS.
- 기존 제품 Debug DLL + Effects11 + D3D11 WARP probe: 249 assertions PASS.
  실제 GPU constant buffer에서1024B·Clone/variant 교차·부분 write·실패 후 복구를 읽었다.
- 큰 raw 캐시를 포함한 후보 Debug DLL + Effects11 + D3D11 WARP에서도 동일249 assertions PASS.
  후보 DLL SHA256은 `2C44973CC6A8F15A92C5F6375AB64C0BFBD8B541C85BCEFD5857DB893E8B15CA`다.
- 원래 프로젝트에서 다시 빌드한 캐시 포함 integrated Debug DLL도249 assertions PASS이며 기존 CSO58개와
  SHA256이 모두 같다. DLL SHA256은 `0131066B75B6872CD24FD066D48D67224F5CBAA735D7580CB95EB41BE89F33FB`다.
  이 재검사는 timing loop를 실행하지 않았고 기존 baseline/candidate와 측정 원본을 보존했다.
- 원래 프로젝트의 baseline/캐시 후보 Release DLL도 유효 입력·GPU readback·Clone·부분 write·누락 변수
  실패/복구 검사를 각각261개 통과했다. 기존 CSO58개와 baseline65파일을 그대로 보존했다.
  후보 DLL SHA256은 `AC7810DA4BBF0A03EFC77726736244BDBCF00670800BF44E7434EB77ED118AB8`다.
  기존 Effects11 Release는1024B 변수에1028B를 쓰는 잘못된 입력을 Debug와 달리 거부하지 않았다.
  최초 Release probe의 해당 실패 로그를 보존하고, Release에서는 Debug 전용 초과 크기 검사2개를
  제외했다. 추가 matrix 변경 mode의 초기화14개를 포함한261개이며249개의 단순 재실행이 아니다.
  Release의 초과 크기 입력 안전성을 이번 검사나 캐시로 보장하지 않는다.
- 이 Debug249/Release261 검사는 큰 raw 캐시를 포함한 초기 후보 DLL의 증거다.
  캐시를 제외하고 RNM 세 파일만 남긴 최종 제품 빌드와 검증 대상이 다르다.
  최종 반영 범위의 빌드 증거는 G04에서 별도로 구분한다.

단순 memcpy setter 대역의 microbenchmark에서는 후보가 더 느렸다.
unchanged 입력은13.150→15.231ns, changing 입력은13.242→23.010ns였다.
이 대역은 실제 Effects11 setter·variant 전파 비용을 포함하지 않는다.
unchanged setter 수500,001→1만으로 최종 속도 향상을 단정하지 않는다.

이후 실제 baseline/candidate Debug DLL을 순서 교대하며4쌍 측정했다.
각 mode는7회×20,000 bind이며 양쪽 CSO58개의 SHA256이 일치한다.
GPU 상수 readback과 반복 bind/Begin 초기화 등을 포함한249 assertions를 각 실행에서 통과했다.

| 실제 Effects11 CPU 호출 조건 | Static mesh 원본 → 변경 | Deferred 원본 → 변경 |
|---|---:|---:|
| 같은1024B, setter + Begin | 0.890 → 0.351µs | 0.584 → 0.270µs |
| 바뀌는1024B, setter + Begin | 1.025 → 1.007µs | 0.533 → 0.536µs |
| 바뀌는1024B, setter만 | 0.045 → 0.062µs | 0.042 → 0.051µs |

다른 바인딩도 고정한 분리 측정이며 GPU draw 시간은 아니다. 동시에 다른 빌드가 진행됐다.
world matrix·다른 재질 값까지 바뀌는 실제 장면에 이 감소율을 적용하지 않는다.
변경값의 setter 자체는 비교·복사 때문에 느려졌고, setter+Begin의 작은 차이는 분산과 구분하기 어렵다.
원시 값은 `shader/paired-raw-timing-samples.json`, 요약은 `shader/paired-raw-timing-summary.json`이다.

최종 채택 판단에는 Release 빌드가 끝난 뒤 보존된 baseline/candidate 실행 파일을 다시 사용했다.
시작 시 `cl`, `c1xx`, `link`, `MSBuild` 프로세스가 없음을 확인하고 순서 교대4쌍을 한 번 측정했다.
소스·실행 파일은 바꾸지 않았고 각 조건은7회×20,000회였다. 아래 값은 실행별7회 중앙값을 구한 뒤
각 DLL의4회 실행 중앙값을 비교한 CPU 호출 시간이다.8회 실행 모두261 assertions를 통과했다.
261개는 앞서 구분한 정상 입력·GPU readback·실패 복구·반복 초기화 검사이며 게임 장면261개가 아니다.

| 최종 Release CPU 호출 조건 | Static mesh 원본 → 후보 | Deferred 원본 → 후보 |
|---|---:|---:|
| 같은1024B·다른 바인딩 고정, setter + Begin | 0.682175 → 0.275335µs (−59.6%) | 0.420295 → 0.211018µs (−49.8%) |
| 같은1024B·매회 world matrix 변경, setter + Begin | 0.744808 → 0.695430µs (−6.6%) | 0.450255 → 0.450430µs (+0.04%) |
| 바뀌는1024B, setter + Begin | 0.685338 → 0.694590µs (+1.35%) | 0.424003 → 0.457253µs (+7.84%) |
| 같은1024B, setter만 | 0.043213 → 0.048888µs (+13.1%) | 0.046020 → 0.049260µs (+7.0%) |
| 바뀌는1024B, setter만 | 0.043638 → 0.053163µs (+21.8%) | 0.046338 → 0.056205µs (+21.3%) |

실제 [CMaterial::Bind_SourceCharacterInputs](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:1010)는
time·program·row와1024B 상수를 함께 바인딩한다.
[동일 light 입력 검사](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:1047)는
같은 program·조명 상수·유효 texture 입력의 row를 이미 재사용한다.
[Renderer의 row 반복](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1782)은
각 material light row에서 큰 상수를 한 번 설정하고 `Render_Lights`를 호출한다.
모든 바인딩이 고정된 채 큰 raw setter를 매회 다시 부르는 최선 조건만으로 이 호출 흐름의 이득을
대표할 수 없다. 행렬이 바뀌는 조건은 Static에서 작은 감소가 있었지만 Deferred는 차이가 거의 없었고,
변경값에는 비교·복사 비용이 추가됐다. 실제 장면의 연속 캐시 적중률과 전체 CPU 비용은 측정하지 않았다.
일부 실행 시간 편차도 남아 있으므로 위 증가율을 확정적인 게임 회귀율로 설명하지 않는다.

따라서 모든65~4096B raw 입력에 캐시를 상시 적용할 근거가 부족해 `Shader.h`/`Shader.cpp` 후보를
최종 변경에서 제외했다. 기존64B 캐시·공유 Effect·program variant는 유지한다.
최종 판단 원시 값은 `shader/release-idle-paired-raw-timing-samples.json`, 요약은
`shader/release-idle-paired-raw-timing-summary.json`이다. 동시 빌드 중의 초기 Release2쌍 결과와
최초 초과 크기 입력 실패 로그도 별도 보존했으며 최종 판단 수치로 대체하거나 덮어쓰지 않았다.

## G02. Model.h / Model.cpp / MapStaticBatchObject.cpp — RNM mesh 바인딩 검사

기존 `Bind_StaticLightingBank`는 mesh 하나를 바인딩할 때 모든 모델의 모든 mesh를 다시 검사했다.
Client 후보 수집에서도 전체 검사를 이미 수행하므로 다중 mesh에서 검사가 중복됐다.

새 `Bind_StaticLightingBankMesh`는 전체 모델 admission 이후 현재 mesh의 geometry identity,
NONANIM·device/context·preScale/preTransform·morph·material index와 실제 material 입력을 검사한다.
`CMaterial::Bind_StaticLightingBank`의 surface·override·비조명 SRV·필수 조명 입력 검증은 유지한다.
stack array8을 사용하며 pointer나 호환 결과를 프레임 사이에 저장하지 않는다.
기존 전체 모델 binder의 public 계약은 유지했다.

실제 호출자는 `CMapStaticBatchObject::Render_AdjacentNonBlend`다.
self와 모든 추가 후보가 `Can_BatchStaticLightingWith`를 통과한 뒤에만 새 binder를 호출한다.
LOD·원래 instance payload·lighting SRV 순서·shader pass와 실패 처리 방식은 보존한다.
첫 draw 전 준비 실패는 기존 Render로 복귀하고, 이미 그린 prefix가 있으면 중복 재시작하지 않는다.

실제 CModel/CMaterial 메서드 본문, 실제 material 타입과 D3D11 WARP SRV를 사용한 검증:

- Release 6,920 / Debug 6,920 / 검사 횟수 계측 6,940 assertions PASS.
- bank2~8, mesh1/2/8, geometry·transform·재질·override·입력 누락·setter 실패와 mesh 변경 확인.
- CShader setter는 순서와 값을 기록하는 대역이다. 실제 shader draw·게임 실행 검증은 아니다.

전체 모델 admission과 모든 mesh bind를 포함한 독립 Release fixture:

| mesh / 조명 묶음 | 원본 CPU 시간 | 변경 CPU 시간 | material 검사 횟수 |
|---|---:|---:|---:|
| 1 / 2 | 0.904µs | 0.725µs | 6 → 4 |
| 4 / 8 | 51.568µs | 16.499µs | 192 → 64 |
| 8 / 8 | 183.663µs | 36.351µs | 640 → 128 |

8mesh×8bundle은 synthetic 조건이다. 실제 Bern에서 같은 조합이 재생됐다는 증거는 없다.
기존 `2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md:226`에는 SOURCE_BG 반복 multi-mesh
모델145종·재질 변형1,082개·표시 배치2,273개라는 설치 inventory가 기록돼 있다.
이는 이번 재측정이나 현재 카메라의 bank 호출 수가 아니다.

## G03. 일반 local-light screen clip — 미반영

source-character용 보수적 screen clip을 일반 point/spot에도 켜는 후보를 실제 설치 shader로 검사했다.
창 없는 D3D11 WARP와 하드웨어에서144조건,160×112 출력을 비교했다.
perspective/orthographic, point/spot, marker0/3/4/7/9/14,
near-plane·camera-inside·offscreen·큰 월드 좌표를 포함했다.

제품과 같은 FP16 하드웨어 결과:

| 지표 | 결과 |
|---|---:|
| 비영 조명 조건 | 120 / 144 |
| 엄격 오차1e-5 초과 조건 | 72 |
| 최대 HDR 절대차 | 0.0009765625 |
| PS invocation | 2,580,480 → 933,960 |
| 동일 baseline 반복 오차 | 0 |
| 기존 비영 RGB가0으로 소실된 채널 | 0 |

near-plane·camera-inside·offscreen 출력 차이는0이었다.
clip에 따른 삼각형 UV 보간 정밀도 변화가 원인일 가능성이 있으나 완전히 분리하지 못했다.
엄격한 출력 동등성을 만족하지 않아 Light_Manager.cpp의 flag와 shader는 수정하지 않았다.
과거 Bern의 일반 spot은 near-plane fallback이라 이 후보의 절감이0인 사례도 있었다.
이 PS invocation 감소를 제품 GPU 개선이나 게임 FPS 향상으로 기재하지 않는다.

## G04. 제품 빌드와 검증 산출물

초기 별도 작업 폴더에서 Engine·Shared·Server Debug 컴파일/링크는 성공했다.
첫 빌드의 Git LFS352파일(약489MB)은 공유 local object에서 checkout했다.
Client의226개 shader 초기 생성 중 원래 프로젝트의 다른 작업이 완료되어,
그 초기 빌드는 명시적으로 중단하고 현재 프로젝트에서 정상 Product 증분 빌드로 전환했다.
초기 run의 `fxc.exe` exit -1/FAIL은 이 중단 기록이며 전체 성공으로 집계하지 않는다.
현재 프로젝트의 기존 MSBuild 의존성 추적을 사용하고 최신 main 위의 변경을 빌드한다.

큰 raw 캐시를 제거한 **최종 RNM 변경의 Debug·Release Product compile/deploy 모두 PASS**.
Debug422.216초, Release309.009초였으며 양쪽 모두 runtime 입력 누락·오류 목록이 비어 있다.
최종 증거는
`C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261005T051205479Z-debug-product.json`과
`C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261005T051738415Z-release-product.json`이다.

캐시를 포함했던 초기 후보의 **Debug·Release Product compile/deploy 모두 PASS**.
Debug383.162초, Release400.140초였으며 양쪽 모두 필수 runtime 입력 누락과
검사한 catalog/navigation 오류 목록이 비어 있다.
증거는 `C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261005T044858937Z-debug-product.json`과
`C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261005T045802992Z-release-product.json`이다.

아래 표는 초기 캐시 후보가 아닌 최종 RNM 변경의 빌드 결과다.

| 단계 | 최종 Debug / Release 결과 | 갱신 OBJ(Debug / Release) | 재컴파일 CSO(Debug / Release) |
|---|---|---:|---:|
| Engine | PASS / PASS | 36 / 36 | 0 / 0 |
| Shared | PASS / PASS | 0 / 0 | 0 / 0 |
| Server | PASS / PASS | 0 / 0 | 0 / 0 |
| Client | PASS / PASS | 223 / 214 | 0 / 0 |

모든 단계의 MSBuild tracking identity는 유지됐다. header 변경의 의존성을 따라 C++를 다시
컴파일했고 shader 수식이 바뀌지 않아 기존 CSO를 재사용했다.
VS18.9 x64 MSBuild, MSVC14.44.35207, Windows SDK10.0.26100.0을 사용했다.
기존 C4819/C4828 코드 페이지, 숫자 변환 및 third-party PDB의 LNK4099 경고가 있다.

실행한 정상 빌드 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MSBuildPath 'C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\amd64\MSBuild.exe' -BuildLogDirectory out/UnrealRenderOptimization20261005/FinalDebug -MaxCompilerProcesses 4
```

작업 디렉터리는 `C:/Users/tnest/Desktop/LostArk`다. 같은 명령에서 configuration을
`Release`, 로그 폴더를 `FinalRelease`로 지정한 Product 빌드도 완료했다.
두 빌드 모두 Client나 Server를 실행하지 않았다.

검증 기록은 다음 폴더에 있다.

- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/shader`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/rnm-validation.json`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/rnm-probe`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/light/result.json`
- `C:/Users/tnest/.codex/worktrees/unreal-render-optimization/LostArk/out/UnrealRenderOptimization20261005/Debug`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/IntegratedDebug`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/IntegratedRelease`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/FinalDebug`
- `C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/FinalRelease`

실제 `MapStaticBatchObject.cpp`도 MSVC14.44 Debug|x64 평가 설정과 새 EngineSDK로
`/Zs` 문법 검사를 통과했다(exit0/error0). 소비한 SDK의 Model.h는 수정한 Engine/Public/Model.h와
SHA256이 같다. 기존 C4819 경고3개만 있었고 source/header 전후 hash는 동일했다.
이는 전체 Client 링크와 별도의 최소 소비자 컴파일 증거다.

독립 소스 리뷰에서 correctness blocker는 발견하지 못했다.
새 모델 binder의 전체 admission 선행과 현재 mesh 입력 검증을 확인했다.
캐시 후보의 shared Effect 수명·무효화·실패·예산 검토는 G01의 미반영 후보 검증이다.
최종 PLAN에 포함한3개 파일 전문과 실제 적용 소스가 일치함을 확인했다.
최종3개 소스는 UTF-8 BOM 없음·CRLF를 유지하며 `Shader.h/Shader.cpp`는 기준 main과 같다.
`git diff --check`를 통과했다. 제품 JSON/XML·프로젝트 등록 파일은 변경하지 않았다.
Client/UI 자율 실행·사용자 장면 화면·프레임 시간 측정은 하지 않았다.

## G05. Unreal 비교와 다음 구현의 전제

### G05-1. 기술소개서에서 설명할 장점과 한계

| 현재 구조 | 장점 | 한계와 개선 판단 기준 |
|---|---|---|
| CPU 화면 오차 LOD | 원본 vertex buffer와 재질 채널을 유지하고 별도 per-draw compute/readback 없이 선택 | batch의 보수적 bounds를 사용해 세밀한 instance/cluster별 선택이 제한된다. 더 잘게 나누면 draw가 늘 수 있어 triangle·draw 비용을 함께 측정해야 한다. |
| RNM lighting bank | 원본 texture·UV·scale을 유지하며 호환 배치를 합친다 | 같은 geometry·실제 material 조건과 최대8개 고유 조명 묶음 제한이 있다. 임의 재질 전체를 한 draw로 합치는 구조는 아니다. |
| 명시적 Deferred 패스 | 작은 엔진에서 소유자와 입력·출력을 추적하기 쉽다 | 다양한 native receiver의 GBuffer ABI를 일일이 연결해야 한다. UE RDG의 의존성·수명 관리와 비교해 불필요한 복사·clear를 먼저 조사할 수 있다. |
| 분리된 shader program과 공유 Effect cache | 기존 복원 수식과 Clone 수명을 유지하면서 반복 제출 비용을 줄인다 | 많은 program의 컴파일·유지 비용이 있고 바뀌는 값에는 cache 비용이 추가된다. permutation 수 감소와 runtime 상수 cache는 별개다. |
| 현재 프레임 SSGI/SSR | D3D11의 기존 depth/normal/radiance 입력으로 실험할 수 있다 | 화면 밖 정보와 history가 없어 Lumen의 장면 표현·누적 기능을 대신하지 못한다. 먼저 receiver coverage와 temporal 기반을 연결해야 한다. |
| CPU particle simulation + GPU instancing | 원본 시간·본 부착·재생 history를 기존 CPU 흐름에서 검증하기 쉽다 | 많은 입자는 CPU simulation·upload 비용을 낸다. GPU simulation 전환에는 같은 spawn/lifetime/공간·정렬 계약을 옮기는 작업이 필요하다. |

서로 다른 엔진·장면의 FPS 우열은 측정하지 않았다. 이번 수치는 같은 LostArk 함수의 변경 전후
독립 비교이며, Unreal과의 표는 구현 구조와 적용 전제를 비교한다.

### G05-2. 같은 역할의 소스 연결표

| 역할 | LostArk에서 읽을 곳 | Unreal에서 읽을 곳 |
|---|---|---|
| 프레임 패스 순서 | [Renderer.cpp:870](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:870), `CRenderer::Draw` | [DeferredShadingRenderer.cpp:1823](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/DeferredShadingRenderer.cpp:1823), `Render` |
| 광원 제출 | [Light_Manager.cpp:186](C:/Users/tnest/Desktop/LostArk/Engine/Private/Light_Manager.cpp:186), `Render_Lights` | [LightRendering.cpp:1627](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/LightRendering.cpp:1627), `RenderLights` |
| 일반 정적 LOD 선택 | [StaticMeshLod.cpp:290](C:/Users/tnest/Desktop/LostArk/Engine/Private/StaticMeshLod.cpp:290), `Select_Range` | [StaticMeshSceneProxy.cpp:2887](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/StaticMeshSceneProxy.cpp:2887), `GetLODMask` |
| 배치·draw 제출 | [MapStaticBatchObject.cpp:572](C:/Users/tnest/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp:572), `Render_AdjacentNonBlend` | [MeshPassProcessor.cpp:1248](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/MeshPassProcessor.cpp:1248), `SubmitDrawBegin` |
| BRDF 계산 | [Shader_Deferred.hlsl:373](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:373), `Evaluate_MapSourcePBRDirect` | [ShadingModels.ush:212](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/ShadingModels.ush:212), `DefaultLitBxDF` |
| 현재 세대 재질 평가 | native program·receiver별 LostArk shader | [SubstrateEvaluation.ush:467](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/Substrate/SubstrateEvaluation.ush:467), `SubstrateEvaluateBSDFCommon` |
| 화면 공간 GI | [Shader_ScreenSpaceLighting.hlsl:260](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:260), `PS_SSGI` | [LumenScreenProbeGather.cpp:2169](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/Lumen/LumenScreenProbeGather.cpp:2169), `RenderLumenScreenProbeGather` |
| 입자 draw·업데이트 | [Effect_DocumentRenderer_Particles.cpp:421](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp:421), `Render_Particles` | [NiagaraSystemInstance.cpp:2659](C:/Users/tnest/Desktop/UnrealEngine/Engine/Plugins/FX/Niagara/Source/Niagara/Private/NiagaraSystemInstance.cpp:2659), `Tick_Concurrent` |
| 타임라인 실행 | [WorldSequencePlayer.cpp:1120](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:1120), `Update` | [MovieSceneSequencePlayer.cpp:1084](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/MovieScene/Private/MovieSceneSequencePlayer.cpp:1084), `Update` |
| CPU 계측 시작 | [Profiler.cpp:103](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:103), `Begin_Frame` | [CpuProfilerTrace.cpp:247](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Core/Private/ProfilingDebugging/CpuProfilerTrace.cpp:247), `OutputBeginEvent` |

읽기 순서는 Renderer → Light Manager → BRDF → LOD·배치 → 입자 → 타임라인 → 계측을 권한다.
각 단계에서 C++가 어떤 상수·texture를 shader에 넣고, shader 결과를 다음 패스가 어떻게 읽는지 연결한다.
파일 이름 하나를 옮기는 것보다 입력 소유자·수명·형식·실패 처리까지 추적하는 것이 중요하다.

### G05-3. Deferred와 RDG

LostArk는 Shadow → NonBlend/GBuffer → SSAO → Lights → Combined → 효과·후처리 → Final을 직접 호출한다.
Unreal의 `FDeferredShadingSceneRenderer::Render`는 FRDGBuilder와 여러 장면 하위 시스템으로 패스를 구성한다.
[RenderGraphBuilder.cpp:1327](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RenderCore/Private/RenderGraphBuilder.cpp:1327)의 `Compile`에서 패스 의존성과 culling 처리를 읽을 수 있다.
LostArk에도 비활성 기능 생략과 임시 target 관리가 있으므로 모든 패스가 항상 실행된다고 설명하면 틀린다.
RDG 전체 이식보다 현재 불필요한 복사·clear·반복 bind를 실제 소비 순서와 함께 측정하는 것이 작은 출발점이다.
특히 scene/distortion/bloom의 서로 다른 MRT 의미와 기존 source effect의 scene-color 읽기를 보존해야 한다.

### G05-4. 일반 LOD와 Nanite의 차이

LostArk의 정적 LOD는 미리 생성한 index range에서 화면 오차 조건에 맞는 단계를 선택한다.
`CStaticMeshLod::Create`는 [StaticMeshLod.cpp:173](C:/Users/tnest/Desktop/LostArk/Engine/Private/StaticMeshLod.cpp:173), 선택은 같은 파일 290행이다.
`CMesh::Render_Instanced`는 [Mesh.cpp:375](C:/Users/tnest/Desktop/LostArk/Engine/Private/Mesh.cpp:375)에서 기존 선택 결과와 instance draw를 소비한다.
Unreal에도 일반 StaticMesh LOD가 있으며, 이것과 Nanite를 혼동하면 안 된다.
Nanite는 [NaniteCullRaster.cpp:4442](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/Nanite/NaniteCullRaster.cpp:4442)의 node/cluster cull과 6688행 `DrawGeometry`부터 읽는다.
Nanite 수준의 기능은 cluster 데이터·streaming·GPU culling·raster 경로가 연결된 별도 규모의 작업이다.
기존 LOD admission·가시성·오차 기준을 조사하지 않고 강제로 단순화하면 실루엣이나 원본 채널을 잃을 수 있다.

### G05-5. 재질 묶기와 shader 상수 캐시

같은 texture 이름만으로 재질을 묶을 수 없다. shader program, 실제 SRV, 상수, RNM, 순서·blend 조건을 함께 본다.
LostArk는 동일 instance 상태·ordered geometry·lighting bank를 이미 구분한다.
[Model.cpp:1174](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1174)는 기존 `Bind_StaticLightingBank`의 조사 시작점이다.
Unreal의 [MeshPassProcessor.cpp:1052](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/MeshPassProcessor.cpp:1052)는 shader binding의 동적 인스턴싱 동등성 비교다.
이번 RNM 변경은 배치 기능을 처음 만드는 작업이 아니라 기존 배치 경로의 중복 검증 비용을 줄이는 작업이다.
큰 상수 캐시는 같은 값의 반복 전달 비용을 줄이는 후보로 검사했지만, G01의 실제 호출 조건별
Release 결과를 근거로 이번 제품 변경에서 제외했다. 기존 작은 값 캐시와 program variant는 유지한다.
CPU setter 감소, GPU constant-buffer 갱신 감소, draw 감소, pixel 비용 감소는 각각 별도 계측 대상이다.

### G05-6. PBR·간접광에서 이미 있는 것

LostArk의 MapPBR에는 roughness·metallic·RGB F0·GGX 계열 직접광과 RNM/환경 입력이 이미 있다.
Character Select의 native SH·cube·128×32 BRDF 입력은 181개 material에 연결됐다는 현재 복원 기록이 있다.
정본 근거는 [SOURCE_EVIDENCE_RESULT:156](C:/Users/tnest/Desktop/LostArk/.md/GB/10-04/2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md:156)이다.
이 범위를 모든 BG RNM, native character, dynamic character probe 복구 완료로 확대하면 안 된다.
MapPBR marker3, native character marker5, 기타 legacy·foliage·stone family는 수신 ABI가 다르다.
Unreal 5.8의 `DefaultLitBxDF`에는 Substrate로 대체됐다는 deprecated 주석이 있으므로 비교용 구경로로 읽는다.
화질 개선은 보이는 배치 → 실제 material → texture/UV/parameter → 조명 → 후처리 순서로 원인을 분리한다.
부족한 간접광을 exposure·ambient·bloom 전체 증가로 가리거나 팀장의 FXAA 설정을 바꾸지 않는다.

### G05-7. Lumen과 현재 SSGI·SSR

현재 LostArk SSGI/SSR은 marker3 MapPBR에만 적용되는 선택적 session 실험이다.
[Renderer.cpp:1900](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1900)의 `Render_ScreenSpaceLighting`이 실제 소비자이다.
full SSGI, half gather·depth/normal resolve, SSR depth 교차 보정·roughness 필터는 이미 구현돼 있다.
shader 시작 주석처럼 현재 프레임만 쓰며 history·화면 밖 geometry·hardware RT는 없다.
기존 baked/RNM/IBL 위에 추가 기여를 더하므로 물리적인 environment 교체나 Lumen이라고 부르지 않는다.
Lumen의 화면 추적은 [LumenScreenProbeTracing.usf:57](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/Lumen/LumenScreenProbeTracing.usf:57)이며, 같은 파일 694행에는 mesh SDF 추적이 있다.
Radiance Cache는 [LumenRadianceCache.usf:822](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/Lumen/LumenRadianceCache.usf:822)의 `TraceFromProbesCS`부터 읽는다.
Surface Cache 갱신은 [LumenSceneRendering.cpp:2545](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/Lumen/LumenSceneRendering.cpp:2545)의 `UpdateLumenScene`이다.
반사는 [LumenReflectionTracing.usf:73](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/Lumen/LumenReflectionTracing.usf:73), 시간 누적은 [LumenScreenProbeFiltering.usf:201](C:/Users/tnest/Desktop/UnrealEngine/Engine/Shaders/Private/Lumen/LumenScreenProbeFiltering.usf:201)에서 이어 읽는다.
시간 누적 확장에는 previous camera, motion/velocity, disocclusion·depth/normal 거절, camera-cut·resize·Level 전환 초기화가 먼저 필요하다.
정적 표면의 camera reprojection만으로 움직이는 캐릭터·입자까지 안정화됐다고 주장할 수 없다.
UE5.8 Windows Lumen은 SM6 gate를 사용하며, 현재 D3D11/SM5 shader 복사로 연결할 수 없다.
공식 [Lumen Technical Details](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine)도 software tracing의 DX12/SM6와 별도 distance-field 장면 표현을 설명한다.

### G05-8. Niagara·Sequencer·Profiler 비교의 의미

LostArk 입자에는 기존 renderer와 CPU worker 경로가 있다. [Effect_ParticleUpdatePool.cpp:7](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_ParticleUpdatePool.cpp:7)이 작업 분배 시작점이다.
GPU instanced draw가 있다는 사실은 GPU가 입자 시뮬레이션 전체를 계산한다는 뜻이 아니다.
Niagara는 system/emitter, 데이터 인터페이스, 동시 tick, GPU tick 연결을 함께 읽어야 한다.
GPU 쪽 시작점은 [NiagaraSystemInstance.cpp:2918](C:/Users/tnest/Desktop/UnrealEngine/Engine/Plugins/FX/Niagara/Source/Niagara/Private/NiagaraSystemInstance.cpp:2918)의 `InitGPUTick`이다.
현재 particle 최적화는 source 시간·birth history·본 부착·lifetime·출력 순서를 유지하는 범위에서 먼저 진행한다.
Sequencer 비교는 시간 이동·seek·track evaluation의 책임을 배우는 용도이며 기존 Server 권위 명령을 Client 연출로 바꾸지 않는다.
Profiler 비교는 CPU·GPU 시간을 분리하는 데 사용한다. draw 수가 줄어도 픽셀 병목이면 GPU 시간이 크게 줄지 않을 수 있다.

### G05-9. 현실적인 다음 단계와 보류 근거

1. 같은 camera·해상도·장면·팀장 옵션에서 기존 Profiler capture로 CPU와 GPU의 큰 구간을 먼저 고정한다.
2. 반영한 RNM의 분리 검증과 별개로 실제 장면의 호출 분포·frame time을 기존 capture로 확인한다.
   미반영한 큰 raw 캐시를 다시 검토하려면 먼저 실제 연속 적중률과 다른 바인딩의 변경 분포를 측정한다.
3. GPU는 광원 범위·반복 scene copy·alpha overdraw 등 측정된 패스부터 작은 후보 하나씩 비교한다.
4. 화질은 원본 입력 누락을 먼저 닫고, 이후 temporal 안정화와 화면 밖 probe/반사를 별도 실험으로 진행한다.
5. DX12·Lumen급 장면 표현·Nanite·GPU particle은 별도 설계·데이터 생성·실행 검증이 필요한 장기 범위이다.

일반 local-light clip은 144조건의 실제 shader 비교에서 PS invocation 2,580,480→933,960을 확인했다.
그러나 FP16 최대 절대차 0.0009765625로 엄격 parity 기준을 넘었으므로 이번 제품 반영에서 보류했다.
기존 Bern 일반 spot은 near-plane fallback이라 flag 확장만으로는 비용이 줄지 않았다는 이전 실측도 있다.
[light/result.json](C:/Users/tnest/Desktop/LostArk/out/UnrealRenderOptimization20261005/light/result.json)은 수치 검증이며 최종 게임 화면 판정이나 FPS 보장은 아니다.
팀장의 quality 옵션·Resources·authoring/runtime JSON은 최적화 때문에 임의로 바꾸지 않는다.

### G05-10. Unreal 소스 열기와 빌드

현재 비교는 Visual Studio로 연 로컬 UE5.8.3 소스에 대한 조사이며 UE Editor 빌드 성공을 의미하지 않는다.
release checkout `396c9f059`에서 tracked224,004개 중 missing은0개이며 C++48,260개를 확인했다.
Niagara 관련1773개와 Lumen C++/shader159개도 있다. shallow clone은 과거 Git 이력의 제한이며
현재 checkout의 source 누락을 뜻하지 않는다. sparse/partial checkout 설정은 없었다.
엔진의 Windows 빌드는 Setup → GenerateProjectFiles → UnrealBuildTool의 UnrealEditor Win64 Development 경로다.
소스 폴더의 CMake 감지나 third-party CMakeLists는 이 엔진 Editor 전체 빌드 절차를 대신하지 않는다.
소스 열람은 먼저 가능하지만 프로젝트 생성·정확한 전체 IntelliSense·링크에는 별도 의존성 준비가 필요하다.
Unreal 소스는 Epic 이용 조건으로 제공되는 코드이며 여기서 임의로 라이선스 제한 없는 오픈소스라고 부르지 않는다.

## G06. Perforce와 증분 빌드의 역할

펄어비스의 실제 내부 빌드 명령·엔진·공유 캐시 구성은 확인하지 않았다.
아래는 해당 회사의 내부 운영을 단정한 설명이 아니라, 공개 도구의 역할 구분이다.

Perforce는 workspace가 사용할 파일 revision을 동기화한다. 이미 같은 revision이 있는 파일을
매번 다시 받는 방식이 아니다. 어느 stream·changelist·label을 사용할지는 팀의 정본을 따른다.
[Perforce p4 sync 문서](https://help.perforce.com/helix-core/server-apps/cmdref/current/content/CmdRef/p4_sync.html).

그 뒤 C++의 재컴파일 대상은 빌드 시스템과 compiler의 의존성·설정·산출물 상태가 결정한다.
Unreal은 UBT가 module의 Build.cs 등을 소비하며 Visual Studio 프로젝트 파일은 편집용 표현이다.
[Epic UBT 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine).
엔진의 asset 파생 데이터·material shader를 공유하는 DDC와 C++ object/link 산출물은 구분한다.
[Epic DDC 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-derived-data-cache-in-unreal-engine).

현재 LostArk는 MSBuild 프로젝트와 정상 Product runner를 사용한다. 같은 작업 폴더·toolset·SDK·
configuration을 유지하고 보통 Build를 반복하면 증분 추적을 재사용한다. Clean/Rebuild나
작업 폴더·toolset 교체는 같은 의미가 아니다. 이번 별도 작업 폴더는 기존 `.obj/.tlog/.cso`가 없는
첫 빌드라 전체 shader 생성 비용이 들었다. 이후 원래 폴더가 안전하게 사용 가능한 상태가 되어
같은 toolchain의 기존 산출물을 사용하는 정상 증분 빌드로 전환했다.
팀에서 검증한 prebuilt engine/asset cache 배포가 있다면
입사자는 그 절차를 따라 준비한 뒤 자기 변경분을 빌드하는 방식으로 비용을 줄일 수 있다.
