# 베른 프레임 시간 회복 구현 계획

## G00. 범위와 현재 실측

사용자는 베른 FPS 하락만 먼저 수정하고 추가 표현 복원 방향은 이후에 정하기로 했다. 기존 원본 재질, RNM/SDF, 조명, 가시 배치, 크기와 후처리 설정을 유지한다. 폭포와 새 환경 표현은 이번 변경에 추가하지 않는다.

사용자가 직접 저장한 profiler_20260911_044109_437_frame11498.json에서 베른 최근 30프레임 CPU 중앙값은 326.282ms, Render.NonBlend는 252.491ms, Render.Lights는 1.122ms다. 실제 drawCalls 중앙값은 3,459, mapBatchCount는 16,421이다. 이 구간은 카메라와 가시 배치 수가 변하며 고정 benchmark가 아니다. GPU 전체 프레임 query만으로 GPU 포화를 단정하지 않는다. 근거와 이전 표현 복원 범위는 BERN_NATIVE_FORWARD_RESULT의 G07~G09를 따른다.

## G01. MapStaticBatchObject의 동일 인스턴스 업로드 제거

Client/Public/MapStaticBatchObject.h와 Client/Private/MapStaticBatchObject.cpp가 visible 인스턴스와 D3D11 buffer의 상태를 소유한다. 현재 카메라 revision이 바뀌면 culling 결과가 같아도 WRITE_DISCARD와 동일 payload 복사가 반복된다.

원본 culling과 hysteresis는 그대로 평가하고, 순서와 payload가 이전 업로드와 같으면 GPU buffer 업로드만 생략한다. transform, RNM/SDF scale/bias, visibility 또는 실제 인스턴스 순서가 바뀌면 다시 업로드한다. 실패한 업로드를 성공한 revision으로 기록하지 않는다. Late_Update의 authored-visible 유무는 배치 변경 시 갱신한 상태로 읽어 정적 목록의 반복 탐색을 줄인다. 카메라가 확정되는 Render 시점을 임의로 앞당기지 않는다.

## G02. MapInstance 셰이더의 Debug 최적화

Client/Default/Client.vcxproj의 Shader_VtxMeshMapInstance.hlsl 항목에 Debug x64 O1과 shader debug 정보 비포함 설정을 적용한다. 원본 HLSL 함수와 패스 번호, 입력 형식, 재질 설정은 바꾸지 않는다. 기존 Binary/Anim/Deferred와 같은 설정이며 C++ Debug/PDB는 유지한다.

기존 CSO와 후보 CSO를 실제 Effects11/DrawInstanced 및 G-buffer 형식으로 비교한다. BG/overlay/foliage/special, RNM/SDF, masked alpha, cull 변형의 계산 결과와 discard 여부를 확인한 뒤 제품 빌드로 통합한다. 산출물 크기 감소 자체를 FPS 개선 증거로 사용하지 않는다.

## G03. 공유 Effect의 동일 값 바인딩 비용 조사 — 최종 반영 제외

Engine/Public/Shader.h와 Engine/Private/Shader.cpp의 CShader clone은 Effect와 변수 binding table을 공유한다. 변수 이름 검색은 이미 immutable hash table을 사용하지만 동일 값에도 SetRawValue와 SetMatrix를 호출한다. 실제 Effect 상태와 같은 수명으로 마지막 성공한 쓰기만 기록해 완전히 동일한 작은 값의 중복 쓰기를 생략한다.

캐시는 clone별로 두지 않는다. Raw/Matrix/Array처럼 동일 변수에 다른 방식으로 쓰는 경로는 서로의 기록을 무효화한다. 부분 길이 쓰기, 실패, 큰 배열 경로를 보수적으로 처리한다. texture 상태는 Effect 내부 값과 실제 D3D context를 구분하고 Begin의 Apply를 생략하지 않는다. API 결과와 공유 clone 교차 쓰기·배열·실패 이후 상태를 실제 CShader probe로 비교하고, 이 범위의 호출 비용을 별도로 측정한다.

실제 두 DLL과 기존/최적화 CSO의 비교에서 상태 검사 18개는 통과했지만, draw를 포함한 반복 측정은 개선이 안정적이지 않았고 값 교대 조건에서는 더 느렸다. 따라서 이 후보는 최종 제품 변경에서 제외했다. Engine/Public/Shader.h와 Engine/Private/Shader.cpp는 이번 작업 시작 시점의 bytes로 돌렸으며 G01과 G02만 제품에 남긴다.

## G04. 검증과 종료

변경 C++와 project XML을 검사하고 최종 Product 빌드를 수행한다. 최종 CSO의 실제 CShader 생성·pass 적용, 최적화 전후 shader 출력, 인스턴스 payload 보존 및 후보 공유 Effect 상태 검사를 실행한 결과만 RESULT에 기록한다. 앞선 CModel/CMaterial 연결 검사와 이번 성능 변경 검사를 구분한다. 별도 probe는 제품 런타임의 새 선행조건이나 영구 하네스로 등록하지 않는다. 기존 대규모 미커밋 변경을 자동 stage/commit하지 않는다.

에이전트는 Client/UI를 실행하거나 캡처하지 않는다. 사용자가 같은 베른 위치에서 저장한 후속 Profiler JSON으로 실제 frame 개선과 남은 병목을 확인한다. 빌드와 headless 비교를 사용자 화면 또는 실제 FPS PASS로 대신 기록하지 않는다.
