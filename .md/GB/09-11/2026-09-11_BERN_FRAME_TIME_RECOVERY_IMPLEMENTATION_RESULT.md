# 베른 프레임 시간 회복 구현 결과

## G00. 범위와 사용자 실측

이번 변경은 사용자가 요청한 FPS 하락 수정이다. 추가 환경·폭포 표현 복원은 이후 작업이다. 원본 재질 수식, RNM/SDF, 배치 transform·크기·가시성, 조명과 후처리 데이터는 유지했다.

사용자가 저장한 profiler_20260911_044109_437_frame11498.json의 최근 베른 30프레임에서 CPU 중앙값은 326.282ms, Render.NonBlend 252.491ms, Render.Lights 1.122ms였다. drawCalls 중앙값은 3,459회, runtime mapBatchCount는 16,421이다. 따라서 불투명 맵 제출 경로를 우선 수정했다. 카메라 가시 집합이 변하는 기록이며 GPU 전체 query를 GPU 포화의 단독 근거로 사용하지 않는다. 상세 분모는 [기존 결과 G09](2026-09-11_BERN_NATIVE_FORWARD_IMPLEMENTATION_RESULT.md)를 따른다.

## G01. 동일 인스턴스 GPU 업로드 제거

Client/Public/MapStaticBatchObject.h와 Client/Private/MapStaticBatchObject.cpp에서 후보 visible payload와 마지막 성공한 업로드 payload를 구분했다. 기존 Render 시점의 최종 카메라 culling·hysteresis를 그대로 계산한 뒤, 인스턴스 순서와 192-byte payload 전체가 같으면 WRITE_DISCARD를 생략한다. transform, inverse transpose, RNM 및 SDF 입력 또는 가시 집합이 달라지면 다시 업로드한다.

Map이 실패하면 CPU의 성공 payload와 camera revision을 commit하지 않아 다음 호출에서 재시도한다. 초기화 때 전체 authored 인스턴스 수로 buffer capacity를 확보하며 현재 public mutation은 인스턴스 수를 늘리지 않는다. authored-visible count는 lookup 구성·Update_Instance·Set_InstanceVisible에서 유지하고 Late_Update에서 목록을 다시 검색하지 않는다.

제품의 변경 전후 함수 본문과 실제 frustum 계산을 사용한 headless WARP dynamic-buffer 비교 57단계를 통과했다. CPU payload 순서·GPU staging readback·hysteresis·카메라 부재/bypass·transform/RNM/SDF 갱신이 일치했고, Map 실패 2회에서 기존 commit 보존과 재시도를 확인했다. 같은 단계의 성공 Map은 52회에서 18회로 감소했다. 가시 집합이 같은 카메라 이동 30단계에서는 추가 Map이 30회에서 0회로 줄었다. 이 수치는 함수 검사의 업로드 횟수이며 전체 맵의 FPS가 아니다.

## G02. 인스턴싱 셰이더의 누락된 최적화

Client/Default/Client.vcxproj의 Shader_VtxMeshMapInstance.hlsl 항목에 Debug|x64 /O1과 shader debug 정보 제외를 적용했다. 기존 Binary/Anim/Deferred 셰이더와 같은 최적화 설정이며 C++ Debug/PDB와 D3D debug layer는 유지한다. HLSL 식·패스·정점 입력을 수정하지 않았다.

기존 CSO 9,090,535 bytes와 후보 568,111 bytes를 실제 Effects11·DrawInstanced·8개 제품 G-buffer 형식으로 비교했다. 35개 Bern material row와 기존 program 0~5 fixture를 포함해 656조건, 167,936 pixel positions, 5,373,952 MRT channels의 값이 모두 일치했다. 실제 기록 pixel 124,928개와 alpha로 비는 168조건을 확인했고 discard 불일치·NaN/Inf는 없었다. RNM/SDF, alpha cutoff, UV·shadow channel, mirror/two-sided 변형을 포함한다.

Default PS의 정적 명령 수는 3,332에서 1,713, 임시 register는 31에서 19로 감소했다. 정적 명령 개수와 CSO 크기를 실제 실행 비용 감소율로 환산하지 않는다. 제한된 WARP 시간은 family별 개선·악화가 섞여 실제 FPS 개선 근거로 채택하지 않았다.

## G03. 검토 후 제외한 바인딩 캐시

공유 Effect의 마지막 성공한 Raw/Matrix/Texture 쓰기를 공유 상태로 기록하는 후보를 두 Engine Shader 파일에 구현해 별도 DLL로 비교했다. 실제 CShader의 clone 교차·부분/배열 쓰기·실패 후 상태·정확한 비트 보존·GPU constant buffer·SRV hazard 후 Apply 복구 등 18개 검사를 기존/후보 DLL 모두 통과했다.

그러나 최적화 CSO를 함께 사용한 Hardware+D3D debug draw 검사에서 안정적인 이득을 확인하지 못했다. 5,000 draw의 역순 재검사 중앙값은 같은 값 조건에서 기존 181.747ms와 후보 180.059ms, tint 교대 조건에서 기존 177.642ms와 후보 190.807ms였다. bind만 빠른 것을 프레임 개선으로 판단하지 않았고 이 후보를 최종 제품에서 제외했다. 두 Engine Shader 소스는 작업 시작 bytes와 일치한다.

서로 다른 assetId의 동일 상태 병합도 읽기 전용으로 집계했다. geometry 경로·전체 material 설정·RNM/SDF texture·profile·mirror가 모두 같은 보수적 key에서는 정적 그룹 16,422개가 16,247개가 되지만, 중복 175개는 모두 숨겨진 navmesh 배치였다. authored-visible 그룹은 전후 15,745개로 같다. 이 검사에서 가시 배치를 줄이는 동일 상태 중복을 찾지 못했으므로 원본 조명 texture 차이를 무시하는 병합을 제품에 넣지 않았다. 근거는 batch_compatibility_audit.json이며 실제 draw 수의 측정이 아니다.

추가로 Bern loader가 소비하는 used asset 15,589개와 material row 21,454개의 DDS 참조 8,289개를 header 기준으로 집계했다. 논리 mip payload는 일반 재질 977.535MiB, RNM 48.894MiB, SDF 48.951MiB로 합계 1,075.381MiB다. atlas 개수가 많다는 이유만으로 texel payload를 과대 추정하지 않는다. 실제 adapter budget, driver allocation, SRGB 중복, geometry와 render target은 이 합계에 포함되지 않아 VRAM 포화 여부는 미확정이다. 누락·미지원 DDS header는 0개이며 근거는 texture_memory_audit.json이다.

## G04. 통합 검증

변경 MapStaticBatchObject.cpp 최소 C++ 구문 검사와 실제 제품 컴파일, 두 파일 UTF-8/CRLF 유지, project XML parse 및 전체 git diff --check를 확인했다. 변경된 제품 JSON은 없다. 최종 Engine/Shared/Server/Client Debug Product 빌드와 배포는 모두 PASS이며 runtime 필수 입력 누락은 없다. 정본 실행 기록은 out/BuildPipeline/runs/20260910T202411687Z-debug-product.json, 로그는 아래 Temp의 product-build-4.log다.

앞선 두 요청은 다른 Product 빌드의 exclusive lock 대기 300초 제한으로 끝났으며 컴파일 오류가 아니었다. 이후 빌드는 성공했지만 후보 캐시를 제외할 때 복사된 소스의 옛 timestamp 때문에 중간 DLL이 남은 것을 hash 비교로 발견했다. Shader.cpp를 최종 소스로 다시 컴파일하고 마지막 Product 배포의 Engine/Bin/Debug와 Client/Bin/Debug DLL SHA256이 모두 045F7C8701227F7D099C78034A1D7C995ADE6CCB76271D8111966417420F081A임을 확인했다. 제외 후보 DLL의 hash와 다르다.

최종 MapInstance CSO는 568,111 bytes, SHA256 105C42327DBE2A1AEDEFFEA1F5DF735CC89276B2A83255FBFC1223C2C0C46455로 G02 수치 비교에 사용한 후보와 정확히 같다. 배포된 DLL/CSO를 복사한 실제 CShader WARP+D3D debug 검사에서 생성·pass 적용·DrawInstanced와 상태 검사 18개를 오류 0으로 확인했다. 근거는 FinalProduct/final-probe.log다. 이 검사도 Client 전체 장면 또는 사용자 화면의 FPS 측정은 아니다.

이번 별도 검사의 입력·소스·로그는 C:/Users/user/AppData/Local/Temp/LostArkBernFps20260911에 보존했다. MapInstance/summary.json과 results.json은 shader 비교, BernFpsFix20260911/batch_result.json과 batch_probe.log는 업로드 비교, ShaderBinding/optimized_*는 제외 후보의 측정 근거다. 제품 런타임의 새 하네스나 Resources 입력을 추가하지 않았다.

## G05. 사용자 실행 확인과 남은 경계

수정 후 실제 베른 FPS는 아직 측정하지 않았다. 완료 시 Server/Client process와 TCP 7777 listener는 없었다. Client/UI 실행·조작·캡처와 최종 화면 판정은 사용자가 직접 한다. 이 PC는 LAN sync의 server-host이므로 Server + Client profile의 Ctrl+F5로 실행하고 Lobby → Bern에서 이전에 느려진 위치에 멈춘 뒤 F1 → Profiler → Capture, 약 10초 후 Save JSON으로 비교한다. 후속 기록에서 같은 구간의 frame/NonBlend 시간과 visible/draw 분모를 함께 확인한다. 사용자의 후속 측정 없이 프레임 회복 완료 또는 visual PASS로 기록하지 않는다.
