# 쿠크 카드미로 진행·관전 적용 결과

후속 변경: 같은 날짜 `2026-09-08_KOUKU_CARD_MAZE_FLOOR_MARKERS_RESULT.md`에서 발밑/출구 문양을 실제 등록하고 Debug 망원경 담당 문양 겸임을 제거했다. 아래 이전 단계의 미등록 이펙트·혼자 겸임 설명보다 후속 결과를 우선한다.

2026-09-08. 기존 Claude 작업과 다른 dirty 변경은 보존했고 자동 stage/commit/push는 하지 않았다.

## 구현 상태

| 사용자 항목 | 현재 반영 |
|---|---|
| 망치 한 방·즉시 사라짐 | 자기 문양 등록 목표만 타격 허용. hit adapter가 남은 HP 전체를 소비하고 GameRoom이 즉시 despawn. 3스택 전에는 다음 목표 1마리 보충 |
| 36개 행진 반복 | from3/6/9/12 각각 lane1..9. WorldSequence의 선형 좌표/지연/기간을 worldbootstrap v10에 배포, 서버 시작 tick과 51,722ms 주기를 snapshot으로 복제 |
| 세토 접촉 | 이전/현재 플레이어와 세토 경로의 swept 상대 이동 판정. 초기 합산 반경 1.2m, 높이 차 2m 미만, 재접촉 유예 30tick. 본인 스택·출구 취소 후 목표 교체 |
| 중앙 안전 | 중앙 (0.28, -0.01, 1351.65) XZ 반경 5m에서는 세토 접촉 무시 |
| 3스택 개인 출구 | 중앙과 경로로 연결되는 랜덤 보행 통로에 서버 좌표 확정. 반경 1m, 높이 차 1m 미만의 본인 진입만 허용 |
| 중앙 이동 암전 | 36tick=1.2초. 0..12 fade-in, 12..24 검정, 24..36 fade-out. 18tick 서버 위치 commit, 이동 중 입력 차단 |
| 탈출자 망원경 공유 | 최초 담당과 탈출자가 중앙 상자를 망치로 가격하면 각자의 관전 flag 토글. Debug 재타격 재시작 제거 |
| 전원 완료 | 생존 참가자의 개인 탈출과 중앙 집결 및 암전 종료 확인 후 2관문으로 동일 암전 이동. 전멸 시 미로 상태 정리 |
| 기본 카메라 | `cardmaze.follow`: follow offset (0,14,-10), FoV 50. 첨부 구도를 참고한 초깃값 |
| 망원경 카메라 | `cardmaze.telescope`: eye (0.28,62,1315.65), center look-at, FoV 50, priority 1000. 관전 flag에만 반응 |

## 사용자가 나중에 지정할 항목

- **출구 효과는 아직 없다.** 서버 출구 판정과 Effect GROUP 소비 경로는 구현되어 있고 현재는 HUD `EXIT (x,z)` 좌표만 보인다. 기존 Effect catalog에서 `cardmaze.exit.heart`, `cardmaze.exit.spade`, `cardmaze.exit.club`, `cardmaze.exit.diamond`를 연결·배포 후 다시 시작한다. alias가 없으면 다른 이펙트로 대체하지 않는다.
- 최종 복귀는 사용자 답변대로 기존 2관문 좌표 (3.38,10.56,323.92)를 사용했다. MapTool → World Gameplay → `cardmaze.return`의 movePlayer 목적지를 나중에 수정한다. 설정 전용 행이므로 disabled를 유지한다. 수정 후 WorldGameplay publish 및 Server 재시작.
- 두 카메라는 MapTool → Camera에서 해당 shot ID로 조정하고 Save Shots → MapAuthoring publish. 실제 첨부 화면과의 일치 판정은 사용자에게 남긴다.

## 자동 검증 증거

- Product Debug Engine → Shared → Server → Client 컴파일/링크/SDK·DLL·shader 배포 성공. 마지막 기록 `out/BuildPipeline/runs/20260908T092611809Z-debug-product.json`.
- 이후 중앙 밖으로 나간 탈출자에게도 접촉 초기화가 적용되도록 Server 2개 CPP를 보정하고 Server Debug 최소 재컴파일 및 동일 카드미로 계약 검사를 다시 통과했다. 안전 보장은 중앙 반경이며 탈출 flag 자체가 전역 무적은 아니다.
- 기존 NetworkProtocolHarness를 확장하고 `--mario-controls-only` 실행 exit 0. protocol70 문양·출구·행진/암전 clock 왕복, unknown flags, 불완전 clock, 잘린 snapshot의 기존 상태 보존 통과.
- 기존 Server 계약 검사에 `--card-maze-contract-test` 범위를 추가하고 실행 exit 0, failures 0. 실제 publish된 36lane/bootstrap/nav 로드, 기존 2관문 목적지 보행면, 네 플레이어 3문양/동시 1마리, HP300·방어99999 한 방 처치, 중복 집계 차단, 토글, 출구 취소, 5m 안전 경계, 집결 및 미완료 암전 차단, reset 검사 통과.
- WorldGameplay Validate/Publish 성공. Kouku bootstrap v10 revision8470, placements105, sequences131, lanes36. 생성 bootstrap 직접 편집 안 함.
- MapAuthoring Area Validate/Publish 성공: placements3231, files8. camerashots runtime revision70 확인. Python을 현재 PowerShell PATH에 둔 뒤 publisher 실행. 앞서 별도 powershell 자식에서 PATH를 못 찾은 실패는 최종 성공으로 해소.
- 변경 JSON은 publisher strict validation 및 JSON parse, 프로젝트 XML은 parse, `git diff --check` 통과. 기존 코드 페이지/PDB 경고는 남아 있으며 warning-free라고 판정하지 않는다.
- 독립 read-only 검토의 초기 한 방 처치/Effect consumer 누락 지적은 최신 실제 파일 및 위 실행 검사로 반증했고 검토자가 철회했다. 재검토한 범위에서 추가 확인된 blocker 없음.

## 아직 수행하지 않은 확인

Client/UI를 에이전트가 실행·조작·캡처하지 않았다. 실제 4인 연결, 망치 입력, 세토 시각 위치와 접촉 체감, 반복 경계, 두 카메라 구도, 암전 체감과 지연 네트워크에서의 전원 이동은 사용자 런타임 확인이 남았다. 서버 콘솔 계약 검사 성공은 이 화면 검증을 대신하지 않는다.

테스트 순서: protocol70 Server/Client 재시작 → Lobby KoukuSaydon → F1 카드미로 진입 → MAZE 망치로 중앙 상자 가격 → 담당 관전 토글 → 자기 문양 처치 3회(세토 접촉 시 초기화 확인) → HUD 좌표의 개인 출구 진입 → 중앙 망원경 공유 → 마지막 생존 사냥꾼 탈출 후 2관문 복귀 확인. Debug 혼자서는 담당이 문양도 받으므로 상자를 다시 쳐 관전을 끄고 사냥한다.

이 PC는 팀 LAN client 설정이다. 사용자는 Client 프로젝트 Ctrl+F5로 시작하고, 공유 Server PC도 같은 protocol70 소스/실행 파일과 배포 데이터를 사용해야 한다. 작업 종료 시 Client/Server 상주 프로세스는 실행하지 않은 상태다.

## 리소스 인계

새 binary resource는 추가하지 않았다. 기존 `MONSTER_KOUKU_CARD_*` catalog 모델과 `cardmiro.march.seto` OBJECT_RESOURCE의 Resources-relative 경로를 그대로 사용한다. 이번 기능의 별도 Drive binary 전달물은 없고, 출구 효과는 팀에서 선정·등록 후 그 효과가 참조하는 기존 Resources/Effect 파일을 전달한다. nav 재베이크나 새 nav 셀 수정도 하지 않았다.

물리 루트는 `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/`이며 필요한 기존 modelAssetId는 아래와 같다. 텍스처 등 각 모델 폴더의 기존 의존 파일도 그대로 필요하다.

- `Character/KoukuSaton/CardMiro_Monster_Heart/CardMiro_Monster_Heart.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Diamond/CardMiro_Monster_Diamond.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Clover/CardMiro_Monster_Clover.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Spade/CardMiro_Monster_Spade.wmodel`
- `Character/KoukuSaton/MN_PPCT_00/MN_PPCT_00.wmodel` (세토)
