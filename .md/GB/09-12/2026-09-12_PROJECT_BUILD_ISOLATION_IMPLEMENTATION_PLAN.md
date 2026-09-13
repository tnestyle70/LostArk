# 프로젝트 전체 수정·병합 빌드 범위 축소 구현 계획

## G00. 목표와 기준

기능 담당자의 한 CPP, 도메인 헤더, native 재질 수정이 그 입력을 실제로 사용하는 컴파일 단위에만 전파되도록 한다. 기존 VS/MSBuild 제품 경로와 출력 ABI를 보존한다. 실행 ZIP은 개발용 obj 캐시가 아니며, 캐시 누락이나 변경된 입력을 강제로 건너뛰지 않는다.

기준 기록은 최근 Client 빌드170.562초 중 ClCompile166.830초, Link2.748초, FxCompile0.079초다. Engine_Defines/Engine_Struct는 기존 Client226개 중217개, Engine78개 중71개 TU의 입력이었다. Effect_Tool34,576줄과 Effect_DocumentRenderer23,178줄이 큰 컴파일 단위를 이룬다. 셰이더는 이미64 ID bucket으로 분리돼 있지만 중앙 dispatcher 수정이 여러 재질군 FX에 전파된다. 과거 거대 FX의 수십 분과 모델 FX 약4분 기록은 현재 변경 전후 실측과 구분한다.

## G01. 안정된 PCH와 실제 소비 헤더

Engine/Client/Shared/Server의 x64 Debug/Release에 프로젝트별 PCH를 만든다. PCH에는 표준 라이브러리만 넣고, Windows/vendor, Engine_Defines, gameplay enum/struct, authoring 문서, shader 실행 표는 넣지 않는다. 플랫폼 헤더는 기존 macro와 namespace 포함 순서를 유지하며 실제 소비자가 읽는다. SDK·CRT 옵션별 PCH는 각 프로젝트 IntDir에 생성한다. PCH를 끈 컴파일도 확인해 숨은 include 의존성을 만들지 않는다.

Engine_Defines에서 Assimp, DirectXTK, FX11, DirectInput의 일괄 노출을 제거하고 실제 소비자에 include/전방 선언을 둔다. Engine_Struct의 초기화, 렌더 설정, 애니메이션 keyframe, vertex layout을 개별 헤더로 이동해 필요한 선언만 포함한다. 기존 타입명·배치·값은 유지하며 Engine_Struct는 명시적 호환 aggregate로 남긴다. 신규 헤더와 PCH CPP는 vcxproj와 기존 물리 위치에 대응하는 filters에 등록한다.

## G02. 대형 CPP의 의미별 컴파일 단위

Effect_Tool의 detail 편집, catalog/IO, preview/history, Valtan 저작 흐름과 renderer의 helper, 준비, material, draw 제출을 독립 CPP로 이동한다. 내부 helper는 Private detail 선언과 정의로 공유하며 각 CPP에 큰 구현을 복제하지 않는다. 공개 상태와 저장·rollback·render 동작은 바꾸지 않는다. 파일별 함수 본문 보존과 실제 컴파일/link로 검증한다.

## G03. 셰이더 생성·include 범위

기존 native bucket의 함수 본문뿐 아니라 case dispatch도 대응 물리 include로 나눈다. 생성기는 같은 내용의 파일을 다시 쓰지 않으며 설치된 파일과 생성기가 같은 경계를 유지한다. 공용 carrier, 모델 FX와 각 native family의 실제 include를 조사해 광역 변경 경로를 줄인다. 원본 ID·함수·조건부 guard·technique/pass·입력 ABI를 유지하고 전처리 결과, FXC 및 변경 bucket의 재컴파일 대상을 확인한다.

## G04. 팀 반복 빌드와 증거

IDE와 runner의 명시 toolchain 선택을 보존하고 C++ worker 기본값을 맞춘다. 이미 순차 호출하는 제품 프로젝트에 불필요한 다중 MSBuild node를 늘리지 않는다. 기존 Product 결과에 중간 산출물 변경 수와 toolchain 변화, 선택 diagnostic 로그의 재빌드 원인을 기록하되 일반 빌드에 전체 source/resource hash나 광역 검증을 추가하지 않는다.

최종 검증은 제품 컴파일·link, 같은 인자의 no-change 반복, 대표 CPP/도메인 헤더/native bucket 변경의 실제 obj/CSO 범위, 신규 XML parse 및 diff 검사다. 실행 중 Client/Server는 종료하거나 조작하지 않으며 잠긴 기본 출력은 별도 link 증거와 구분한다. 소유권이 섞인 기존 dirty tree는 자동 stage/commit하지 않는다.

## G05. 전 도메인 실측과 추가 구현

추가 요청에 따라 제품 네 프로젝트의 모든 등록 CPP/HLSL과 헤더 크기, 실제 tlog 의존성을 목록화한다. 조건부 include의 문자 검색과 실제 컴파일 입력을 구분한다. 생성 재질 헤더의 거대 inline 표, Server GameRoom18,035줄, 계약 검사31,302줄, MapTool16,724줄, AnimationTool16,289줄과 document codec15,023줄을 추가 후보로 확인했다.

MapTool과 AnimationTool도 저작 책임별 CPP로 분리한다. GameRoom은 상태 소유권과 tick/command 순서를 유지하며 admission, player command, party/world transfer, boss audition, replication, stage action, world destruction, player/boss simulation 구현을 별도 CPP로 옮긴다. 함수 본문과 조건부 컴파일 guard를 보존하고 공통 helper는 Private 선언과 단일 정의로 공유한다. 기존 CGameRoom public/private 상태를 임의 Pimpl로 바꾸거나 테스트가 검증하는 권위 경계를 제거하지 않는다.

ServerGameplayContractTests는 파일 크기보다 한 worker lambda의 약27,870줄이 문제다. 기존 CLI와 Run_ServerGameplayContractTests 진입점, thread/예외 처리, 호출 순서와 실패 집계를 유지하고 Private runner의 실제 suite 메서드로 나눈다. GameRoom, ClientSession, ServerApp의 기존 테스트 전용 friend 접근만 runner에 연결하며 일반 public gameplay API를 늘리지 않는다. 함수 경계에 걸친 공유 변수와 fixture lifetime을 먼저 확인하고 의미 있는 블록 단위로 이동한다. 새 테스트 CPP도 기존 Server 제품 빌드에 포함하며 테스트를 기본 빌드에서 숨기는 방식으로 시간을 줄이지 않는다.

생성 재질 C++ 헤더는 실제 조회 경로와 생성기를 조사하여 변경되는 테이블·큰 구현을 Private CPP가 소유하게 하는 범위를 검증한다. Effect_DocumentCodec의 JSON 읽기/쓰기, 검증, SourceContract와 Artist 업그레이드 구현도 의미별 CPP로 분리하고 기존 오류 문맥과 round trip을 보존한다. 새로운 CPP와 Private 헤더는 기존 codec의 프로젝트 및 필터와 실제 독립 harness 소비 목록에 등록한다. SourceCharacter HLSLI는 Base/Light를 물리 분리하며 동일 전처리 결과를 유지한다. 한 PS에37개 profile이 들어 있는 현재 모델의 최적화 비용과 일반 include 전파 비용은 별도 병목으로 기록한다. 모든 의존성을 없앴다고 표현하지 않으며 이후 추가할 코드의 owner, 공개 선언, 생성 출력과 빌드 검증 경계를 명시한다.
