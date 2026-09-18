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

## G05. Native ModelCue와 SourceGroup의 컴파일 책임 분리 — 2026-09-17

사용자가 요청한 main 동기화 Product 빌드를 먼저 성공시켰다. 기준은 `bddacace2`와 Loading의 누락 선언 보완이며, 2,290,594ms 중 Client가 2,249,173ms였다. SourceGroup 6개가 캐릭터 표면과 무관한 native ModelCue PS까지 반복 컴파일했다. 성공한 CSO는 `out/ShaderBuildOptimization20260917/baseline/Client`에 보존한다.

Engine Shader의 기존 variant 선택에 pass 소유권을 추가한다. native ModelCue pass 7/8/12/13은 FX annotation으로 기본 FX가 소유하며, 이전 SourceCharacter selector 값과 관계없이 기본 FX의 동일 프로그램을 적용한다. Engine에 Client pass 번호를 하드코딩하지 않는다. SourceGroup은 `EFFECT_NATIVE_DECLARATIONS_ONLY`로 Artist/Lance/ALTV의 기존 uniform·resource·기본값을 보존하고 native 함수와 leaf include를 제외한다. 기본 FX의 원본 수식과 SourceGroup의 일반 재질·광원·투명 pass는 유지한다. 기존 pass 이름·순서·input signature 및 변수 타입 검증을 완화하지 않는다.

## G06. Native 그룹 추가의 재컴파일 범위

Artist facade의 입력 ABI와 공통 helper를 설치 프로그램 목록에서 분리한다. Mesh/Particle wrapper는 자기 그룹의 본문과 dispatcher를 선택하고, Decal/Trail/ScreenPost 등 실제 여러 그룹을 소비하는 경로는 전체 목록을 유지한다. 공통 carrier의 계속 바뀌는 쿠크 최대 ID 대신 해당 그룹의 실제 guarded dispatch ID를 판별한다. 지원하지 않는 ID의 거부와 모든 native 함수·case 본문을 보존한다.

`native_shader_dispatch.py`, 쿠크 installer와 직접 model dispatch를 수정하던 Vehicle 생성기를 같은 분리 writer로 연결한다. 재생성은 내용이 같은 파일을 다시 쓰지 않는다. 새 HLSLI와 wrapper 등록은 기존 Client project/filter에 필요한 항목만 추가한다. Resources나 재질 원본 수식은 변경하지 않는다.

## G07. 동일 shader 프로그램의 pass 간 공유

Deferred, map instance, decal/trail/rect, V2, 기본 모델·UI 등 실제 활성 FX의 같은 profile·entry·argument compile 식을 하나의 전역 shader 객체로 공유한다. pass 상태·순서·이름과 함수 본문을 유지한다. 전처리 조건이 다른 식은 무조건 합치지 않는다. 변경한 공유 선언을 원래 compile 식으로 되돌렸을 때 원문과 일치하는지 검사하고, SDK가 배포하는 Engine shader의 정본은 Engine 쪽에서 수정한다.

## G08. 최종 검증과 시간 보고

기존 native 함수·case 및 pass 수식의 보존, 생성기 roundtrip/no-op, 선택 그룹의 include 의존성과 미등록 ID 거부를 확인한다. 기존 SourceCharacter variant probe에서 stale surface selector와 native pass의 기본 FX 선택·상수·resource를 추가 검증한다. 실제 FX11 pass/input layout과 프로그램 bytecode를 비교하고 관련 Product 빌드와 무변경 Build를 직렬 실행한다. leaf 변경이 무관한 그룹을 재생성하지 않는지도 확인한다. 초기 대량 변경 빌드, 필요한 부분만 갱신하는 빌드, 무변경 빌드 시간을 구분한다. Client/UI 실행이나 화면 판정은 사용자 경계로 유지한다.

## G09. 전체 shader owner의 실제 병목과 작업 기준 — 2026-09-17

사용자는 새 프로그램을 추가하는 팀원에게도 같은 병목이 재발하지 않도록 전체 컴파일 단위를 세분화하도록 요청했다. 현재 branch의 모든 기존 미커밋 변경을 보존한다. 시작 diff와 상태는 out/ShaderGranularity20260917/session-baseline.patch와 session-baseline-status.txt에 기록한다. 실행 중 Client/Server는 사용자가 소유하며 에이전트가 종료·UI 조작·화면 캡처하지 않는다.

기존 MSBuild include/command tracking은 실제 필요한 입력을 선택한다. 후속 증분 빌드는131개 중 animated 기본과 SourceGroup6개만 선택했고 선택 준비는68ms였다. 성공한 이전 로그에서는 MeshPreview321초, DimensionMaster Mesh SD216초·WR176초·ALTV128166초 등 큰 FXC가 지배한다. 후속 기본 animated FX의585초는 의도적 중단까지의 경과라 성공 빌드 시간으로 비교하지 않는다.

기본 모델의 native ModelCue, 여러 native 계열을 합치는 Decal/Trail/ScreenPost, 아직 나뉘지 않은 Dimension 계열, 큰64-ID 그룹과 공통 include 변경 전파를 모두 다룬다. 표면·광원·native 수식과 실제 소비자, pass 상태·signature·uniform/resource 기본값을 보존하며 기존 CShader 경로에서 작은 독립 컴파일 결과를 선택한다. HLSLI 파일만 나눈 뒤 다시 같은 FX에 전부 include하는 형태로 완료 처리하지 않는다.

## G10. MSBuild의 단일 출력 소유와 기존 추적 유지

VS와 정본 runner가 같은 project/configuration의 tlog·CSO를 동시에 쓰지 않도록 두 진입점이 모두 소비하는 공통 MSBuild target에서 빌드 단위 출력 잠금을 적용한다. 잠금 수명은 해당 build 종료·실패까지이며 사용자 프로세스를 종료하지 않는다. 다른 출력 경로의 격리 실험은 독립적으로 허용한다. 기존 FxCompile tracking을 사용하고 timestamp 조작·항상 skip·별도 캐시로 최신 여부를 위장하지 않는다.

Engine/Client의 FX worker 설정은 공통 props에서 일치시키고 여러 빌드의 전체 worker 한계를 지원한다. 기존 BuildIncrementalDiagnostics.psm1에 실제 FX별 시간과 재컴파일 사유를 반영한다. 실제 MSBuild 호출자가 소비하는 최소 task/target만 만들고 기존 테스트에 실패·해제·증분 동작을 추가한다. project/filter 등록은 기존 필터를 유지한다.

## G11. CShader의 독립 pass/program 선택

기존 CShader 안에서 logical owner pass를 독립 FX leaf의 동명 pass로 연결한다. DefaultTechnique는 public pass 이름·순서를 유지하고 뒤의 pass0개 technique에 VariantShader·VariantSelector·VariantFirst·VariantLast·VariantPasses annotation을 저장한다. 이 annotation-only technique는 격리 FXC 실험에서 실제 CSO 보존을 확인했다. 새 registry 추가는 가벼운 owner만 변경하며 무관한 native program 본문은 읽지 않는다.

leaf의 caller-controlled 변수 모두에 대해 owner의 동일 이름·Class/Type/Elements/Rows/Columns/Members/UnpackedSize/resource를 검사하고 기존 revision 복사를 사용한다. 명시 pass-name mapping과 IA signature를 비교하며 중복·모호한 selector 범위, 잘못된 selector 타입, 알 수 없는 pass, self/nested registry를 거부한다. owner만 사용하는 입력은 leaf에 강요하지 않는다. 전체 staging 성공 뒤 한 번만 commit하고 실패 시 기존 shader와 clone 공유 상태를 보존한다.

pass별 fallback 허용을 명시하고 필수 leaf 누락 또는 범위 밖 selector를 빈 PS 성공으로 처리하지 않는다. native ModelCue는 g_ArtistModelCueProfile, 표면·광원은 g_SourceCharacterProgram, 독립 잔상은 무조건 route로 선택한다. ALTV178 capture·Lance1360 depth 단위·Vehicle 조명·bloom의 원본 입력/계산 순서를 유지한다. 새 C++ 파일은 최소화하고 기존 public Bind/Begin 호출자는 유지한다.
