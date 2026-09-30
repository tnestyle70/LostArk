# 현재 브랜치의 최적화·복원 및 PR490·491·492 통합 구현 계획

## G00. 목표와 기준선

사용자가 승인한 통합 범위는 현재 작업 트리의 베른 성능 개선, 베른·캐릭터 선택 지형,
마하라카 환경과 워터팡 효과·AI 및 동시 작업의 완료분, PR490·491·492, 콜로세움 아레나
재질 조사와 기존 리소스의 GBResources 전달이다. 콜로세움 신규 재질 복원은 사용자의
후속 범위 조정에 따라 이번 통합에서 제외한다. 체크아웃은 `codex/valtan-authoring-and-entry-20260930`으로
유지한다. 시작 HEAD는 `4fce79521e7a48f57c6d3351ab272dcbd0e67246`이다.
`origin/main`의 `2019abe4f2b2f90b680f44c908bd6488eedee386`과 파일 내용 차이는 없다.

| PR | 고정 입력 |
|---|---|
| 490 음성 | `bbe576d6552dfd832c41517e2c470586621263e5` |
| 491 팀 테스트 | `bc65b4768b366413b3743c2e1824e1aa69f64029` |
| 492 콜로세움 | `096f653656bf532ec0a304272abba98928e22118` |

코드 기준은 최신 디스크이며 다른 세션의 완료 문서는 구현·검증 범위를 구분하는 입력이다.
Client/UI 실행과 화면 판정은 사용자에게 남긴다.

## G01. 브랜치와 셰이더 증분 상태 보존

현재 변경은 읽기 전용 ZIP으로 먼저 보존하고 기능별 소스 checkpoint를 같은 브랜치에 만든다.
backup/retired와 빌드 산출물은 소스 커밋에 포함하지 않는다. stash push/pop, checkout,
Clean/Rebuild와 기존 OBJ/PCH/tlog 삭제를 하지 않는다. 동일 내용의 shader source/include는
재저장하지 않는다. shader 664개 입력의 디스크 SHA256·mtime를 통합 전후 대조한다.

세 PR에는 HLSL과 프로젝트 빌드 설정 변경이 없다. 워터팡 native effect 변경의 실제
include 범위는 별도로 판단한다. 기존 CSO를 timestamp 조작이나 FxCompile
비활성화로 유지하지 않고 정본 Product Build의 정상 의존성 판단을 사용한다.

## G02. Shared·Server·Client 통합

protocol은 129로 통일한다. 기존 ID 1~112는 보존하고 Colosseum 113~116, Repair 117,
Waterpang tuning 118~119를 명시한다. `PacketType.h`, `PacketMessages.h/.cpp`, 양쪽 dispatch,
기존 protocol harness의 버전·golden payload와 새 필드의 왕복/실패 불변성을 함께 갱신한다.

`C2S_ENTER_WORLD`는 nickname 뒤 voice를, `S2C_PLAYER_SPAWNED`는 yaw 뒤 voice와 Waterpang
NPC ID를 순서대로 기록한다. inventory snapshot 끝에는 슬롯별 durability 6개를 보존한다.
잘못된 tuning 수치를 Server가 typed 오류로 회신하는 현재 Waterpang reader 계약도 유지한다.
콜로세움 match seat와 transfer에 선택 voice가 이어지도록 연결한다.

PR491의 class별 로스터 정렬은 현재 선택 슬롯 유지 계약과 충돌하므로 채택하지 않는다.
Guardian 헤어의 미지원 선택과 실제 리소스 참조, 내구도 세션 보존은 현재 소비자를 조사해
기능을 유지하는 방향으로 통합한다. 신규 Data JSON은 Client `96.DataFiles`에 최소 등록한다.

## G03. 베른 최적화와 복원 데이터 유지

베른의 final-camera map 제출, NPC clock-only/pose 복귀, animation envelope, Layer phase와
profiler 세부 계측을 보존한다. PR492의 관중도 동일한 기존 NPC 경로를 사용해야 한다.
원본 지형 42개가 복원된 현재 베른 배치 50,021개를 기준으로 삼고 PR492의 과거 임시 바닥
434개는 원본 지형과 중복되지 않게 제외한다. 데이터는 stable ID와 필드 기준으로 병합하고
필요한 domain publisher만 실행한다. 렌더링 옵션의 사용자 튜닝을 덮어쓰지 않는다.

## G04. 워터팡과 콜로세움 리소스

워터팡 RESULT의 신규 C++/JSON/Python, 23개 효과와 AI consumer 연결을 빠뜨리지 않는다.
GBResources 전달 626개/828,013,664 bytes는 설치본과 재대조한다. 리소스는 Git에 넣지 않는다.

콜로세움은 설치 103개 모델의 source material과 texture, 기존 material family 및 native
program을 대조해 조사 결과만 남긴다. PR492의 기존 레벨·배치·입장 기능과 실제 의존
리소스를 Resources 상대 경로 그대로 GBResources에 반영한다. 신규 재질·shader 복원은
이번 merge에 포함하지 않는다. 조사 범위와 미지원 원본 기능은 별도 RESULT로 남긴다.
GBResources는 전체 설치 34GB 사본이 아니라 이번 작업분과 팀원 전달분의 추가·변경 리소스를
합친 폴더다. 같은 경로가 다르면 기존 section·clip과 복원 이력을 대조해 양쪽 작업을 보존한다.

## G05. 최종 검증과 병합

현재 Release Product 실행을 완료한 다음 통합 소스를 정상 증분 Product Debug/Release로
빌드한다. 실제 CPP/링크/CSO 변경 목록을 기록하고 통합 때문에 내용이 같은 Mesh/AnimMesh
shader가 재작성되지 않았는지 확인한다. 기존 NetworkProtocolHarness로 통합 layout과
신규 패킷을 검증하고 변경 JSON/XML parse와 diff whitespace를 확인한다.

셰이더 consumer·source group의 기존 closure 검증과 실제 source/include tracking 대응을
확인한다. 기능별 최소 검증과 사용자 화면 미검증을 RESULT에서 분리한다. 같은 브랜치의
통합 커밋을 push하고 원본 PR 이력을 보존하는 merge를 사용한다. 최종 실행파일·게시 데이터·
GBResources 전달 상태를 각각 보고한다.
