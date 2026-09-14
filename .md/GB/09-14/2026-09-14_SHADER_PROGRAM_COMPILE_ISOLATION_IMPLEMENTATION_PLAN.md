# SourceCharacter와 Effect 셰이더 컴파일 묶음 분리 구현 계획

## G00. 목표와 현재 실측

2026-09-14. 현재 SourceCharacter Base/Light dispatcher는 1~32, 80~88의 41개 프로그램을 한 번에 컴파일한다. 실제 소비자는 CModel → CMaterial → CShader와 Renderer → CMaterial → CShader이며, Deferred의 일반 광원 패스도 source mask 실패 시 native 조명을 소비한다. 기존 재질 수식, 투명도·early-depth·pass index, input layout, clone 공유 FX 상태와 실패 격리를 보존한다.

## G01. Effect 화면 입력과 전용 계산

CubeSample의 화면 color/bloom 읽기와 Slice의 depth texture/sampler를 입력 include로 분리한다. native family는 입력만 include하고, family 0 mesh의 CubeSample과 particle의 Slice 계산만 전용 include를 읽는다. EffectCommon의 실제 공용 계약은 유지한다. 원문 함수 보존과 FX 전처리 의존 범위로 검증한다.

## G02. SourceCharacter 프로그램과 독립 CSO

Engine 정본의 Base/Light 프로그램 함수와 dispatch case를 같은 안정 ID 묶음(1~8, 9~16, 17~24, 25~32, 80~83, 84~88)으로 분리한다. Shader_VtxAnimMeshBinary, Shader_VtxMeshBinary, Shader_Deferred의 기본 FX는 source 계산을 포함하지 않고, 각각 6개 독립 FX/CSO가 지정 묶음만 컴파일한다. 기존 함수의 bytes와 case 순서는 유지한다. 생성기는 기존 분리 입력을 다시 펼친 후 필요한 leaf만 갱신한다.

## G03. CShader 실제 선택과 바인딩 상태

기존 CMaterial이 바인딩하는 g_SourceCharacterProgram을 CShader::Begin이 읽고 해당 CSO를 적용한다. CShader Prototype가 같은 pass·입력 layout 계약을 검증하며 shard를 stage하고 실패 시 생성 전체를 거부한다. Clone은 원래처럼 FX와 binding 상태를 공유한다. 다른 shader와 forward program 33~65는 기존 경로를 유지한다.

원래 FX의 successful setter revision을 변수별로 기록하고, 미리 연결한 동명 변수 mapping으로 선택 shard에서 달라진 raw value와 resource만 동기화한다. pass별 state·VS·PS 적용은 선택된 FX11 pass가 담당한다. 별도 draw runtime이나 CMaterial 우회 경로를 만들지 않는다.

## G04. 프로젝트·배포·검증

새 HLSL/HLSLI를 Engine/Client vcxproj와 filters의 기존 shader 필터에 등록한다. Deferred shard CSO를 Engine 출력에서 Client 실행 디렉터리로 필수 배포한다. 새 C++ 파일은 없다. 기존 Shader.cpp/h의 인코딩을 유지한다.

검증은 program/case 원문 보존, Engine/Client shader source 일치, 프로젝트 XML parse, shader compile, FX11 headless ABI/binding 비교, 변경 leaf/common/no-op의 실제 재생성 범위와 관련 C++ compile이다. root와 MSBuild를 직렬 조율한다. Client/UI 실행·캡처·화면 판정은 하지 않으며 사용자 확인은 RESULT에서 별도로 남긴다.