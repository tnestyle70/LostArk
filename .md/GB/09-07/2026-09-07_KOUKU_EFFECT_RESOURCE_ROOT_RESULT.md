# 2026-09-07 KoukuSaydon Effect 리소스 경로 통합 결과

## G00. 반영 상태

사용자가 이동한 `Client/Bin/Resources/Effect/KoukuSaydon`을 Effect 정본으로 연결했다.
이 작업에서 물리 리소스 이동·수정·추출은 하지 않았다. Git에는 참조와 코드·문서만 반영한다.

- V2 Effect leaf 31개의 슬롯 경로 58곳을 새 root로 변경했다. effectId와 튜닝 값은 보존했다.
- WorldSequence의 공 WModel·diffuse, 칼날 WModel 3곳을 변경하고 revision147→148로 저장했다.
  Object10개/templates75개/instances79개의 ID·동작·배치는 유지한다.
- ResourceIntake의 물리 root 2곳, MainApp의 쿠크 Resource Files 수집 prefix를 변경했다.
- 기존 naming 경계 검사에서 이전 Effect 폴더 예외를 제거했다. Character/UI/Sound와 원본 package는 유지한다.
- 현재 HUD/World Object 인계 문서와 CLAUDE에 통합된 위치를 기록했다.

Effect Resource Library와 World Object Tool은 기존 공용 물리 scanner로 하위 WModel/DDS를 검색한다.
기존 V2 direct-authored 읽기 경로와 Map publisher를 사용하며 추가 alias나 fallback을 만들지 않았다.

## G01. 물리 리소스와 참조 확인

`Screen/fx_d_symbol_100_ycl.dds`는 09-06 커튼 ScreenPost 작업에서 추가 설치한 원본 texture다.
원본 `fx_tex_high_00.fx_d_symbol_100_ycl` 추출 파일과 현재 DDS가 바이트 단위로 일치한다.
1024×1024, 1,048,704바이트이며 `boss.kouku.curtain_1`이 새 상대 경로로 소비한다.

현재 통합 폴더는 `Meshes`, `Textures`, `Screen`, `WorldObjects`이고 총703파일이다.
WModel102개/DDS595개가 기존 물리 목록 검색 대상이며 TGA6개는 기존 scanner 대상이 아니다.
모든 WModel을 파싱했고 내장 구 Effect 경로·절대 texture 경로는 없었다.
칼날의 내장 `textures/mn_cngn_00_{d,n,s,e}.dds` 4개도 정상 연결된다.
수정한 파일 참조61회/고유 asset ID29개는 모두 실제 파일에 연결된다.
변경 문서의 Effect/Map/Character/Deploy 전체 경로91회/고유54개도 누락이 없다.

## G02. 자동 검증

| 검증 | 실제 결과 |
|---|---|
| 추적 파일 전수 검사 | 4,031파일을 대소문자·슬래시·역슬래시·UTF-16 표기로 조사. 코드/Data/Tools의 이전 Effect root 0개 |
| 배포본 검사 | Client/Server DataFiles 전체에서 이전 Effect root 0개 |
| 변경 JSON | source33개/runtime1개 parse 및 경로 변경 외 JSON 값 보존 확인. 예외는 WorldSequence revision 증가뿐 |
| Map Publish / Check | 3,231placements/7files, WorldSequence v3/revision148, 배포본 일치 PASS |
| 기존 Map / V2 검사 | WorldSequence와 Effect V2 binding 46개 PASS |
| 실제 Kouku V2 문서 | 기존 leaf/group 검증 함수로 leaf32개/group12개와 참조 연결 PASS |
| 변경 naming 경계 검사 | Effect의 이전 root를 허용하지 않는 기존 검사 PASS |
| Composition | 기존 publish 후 validate PASS, revision76/Product pattern6/stage66/output2 |
| Debug Product | Engine→Shared→Server→Client 컴파일·링크·정상 배포 PASS, missingRuntimeInputs 없음 |
| 공백 | git diff --check PASS |

빌드 receipt는 `out/BuildPipeline/runs/20260907T021221114Z-debug-product.json`이며
경로 조사·배포·검사 로그는 `out/WorldObjectMerge/effect-root-*`에 보관했다.
Composition 최초 검사는 main checkout의 CRLF 때문에 exact-byte 검사가 실패했다.
두 출력 JSON의 의미와 LF 정규화 내용이 모두 같음을 확인하고 기존 publisher로 재생성했다.
패턴 값이나 Git 추적 Product 내용 변경은 없다.

기존 naming 검사 전체에서는 무관한 `KakulSaydon` 표기 검사1개가 실패한다.
MainApp.h:204와 Level_KakulSaydonArena.cpp:41/1399/1625에서 이전 HEAD에도 같은 실패를 확인했다.
이번 경로 변경 검사는 통과했으며 무관한 표기를 함께 수정하지 않았다.
08-29/08-30 추출 결과와 09-05 조사 등 과거 문서4개의 당시 경로는 역사 기록으로 유지한다.

## G03. 실행과 전달 경계

Client/UI 실행·저장 버튼 왕복·화면 판정은 수행하지 않았다. 확인 시 Server/Client process는 실행 중이 아니다.
사용자는 `Server + Client` profile에서 Ctrl+F5로 시작하고 F1의 Effect Resource Library에서
Rescan → Domain의 KoukuSaydon 하위 폴더를 선택한다. Screen texture와 기존 저장 leaf를 열 수 있다.
World Object Tool의 공·칼날도 새 위치의 모델·texture를 읽는다.

팀원에게 전달할 물리 폴더는 `Client/Bin/Resources/Effect/KoukuSaydon` 전체다.
Drive 전달은 별도이며 이 작업에서 업로드하지 않았다. 사용자 요청에 따라 PR 생성·merge와 로컬 main 동기화를 진행한다.
