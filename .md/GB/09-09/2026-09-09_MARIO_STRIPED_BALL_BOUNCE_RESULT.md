# 마리오 줄무늬 공 엇갈린 바운스 적용 결과

## G08. 마리오 광대 약 1.5m로 후속 축소

G07의 목표 높이를 2m에서 1.5m로 변경했다. Character.cpp의 Apply_MarioPresentation 배율 분자만 1.5f로 수정했으며 머리 장식 포함 기본 idle 자세 기준이다. 일반 플레이어와 마리오 밖 광대, 충돌, 공 크기와 바운스 데이터는 이번 변경에서 수정하지 않았다. PLAN 전체 코드와 Area 사용서도 갱신했다.

Client Debug x64 ClCompile 성공(exit0), 변경 파일 git diff --check 성공. 기존 코드페이지 경고가 남는다. 최초 제한 환경 컴파일은 Windows SDK 경로 접근 거부로 실패했으며 승인된 재시도로 성공했다. JSON/XML 변경은 없다. 로그: out/MarioBounce/client-scale-1p5-compile.log. 이후 Client 프로세스 종료를 확인하고 Debug Build(BuildProjectReferences=false) 및 링크도 성공(exit0)했다. 빌드 로그: out/MarioBounce/client-scale-1p5-build.log. Client/Bin/Debug/Client.exe에 반영했으며 사용자 마리오 진입 화면 확인은 미실시다. Server 재빌드와 Resources Drive 배포는 필요하지 않다.

## G07. 마리오 광대 약 2m와 줄무늬 공 1.5배

최종 git diff --check 성공(exit0). publisher가 재기록한 deployassets/deployplacements는 Git 내용 diff가 없고 줄바꿈 경고만 확인했다.

마리오 stage 1~4의 광대 body local scale만 2/2.3730526=약0.8427963으로 설정했다. 2.3730526m는 현재 MN_RPCZ_00-1의 기본 idle 시작 자세에서 머리 장식까지 포함한 스키닝 높이와 기존 admission 배율을 적용한 값이다. 실제 자세에 따른 크기 변화는 유지되므로 매 프레임 상단을 강제로 2m에 맞추는 방식이 아니다. stage 밖이면 같은 광대 body scale을 1로 복구한다. 일반 class는 ClownSpec guard로 무변경이며 모델 prototype/pre-scale과 Character 부모 Transform, Server 이동/충돌은 변경하지 않았다.

- Character.h/.cpp: Apply_MarioPresentation(bool) 추가. 실제 body local Com_Transform에 Scale(절대 축 길이)을 설정하고 누락 시 false를 반환한다.
- ClientReplication.cpp: 일반 snapshot 및 deferred class replacement 두 경로에서 iMarioStage 1~4를 전달하고 기존 실패 처리에 포함했다. 기존 다른 담당의 Bingo snapshot 변경은 보존했다.
- 줄무늬 공 ID 429,430,431,443,449,455,459,461,462,463,468,469의 placement scale만 1 -> 1.5. 다른 배치 행/필드와 worldsequence 바이트 불변 확인. 바운스 최고 4m, 주기1200ms, 상대 위상, scaleMultiplier=1을 유지한다.
- 기존 Map publisher Area Publish/Check 성공(exit0), 3281 placements/8 runtime files. 첫 시도는 Python 검색 실패로 publish 전 중단됐으며, bundled Python을 해당 프로세스 PATH에 추가한 재시도로 성공했다. 다른 optional JSON은 배포 전 source/runtime 구조 동등성을 확인했다.
- 배포된 12개 scale1.5와 source/runtime placement byte 일치, 바운스 높이/주기 불변, Client project/filter XML parse 성공. 기존 세 C++ 파일 UTF-8 BOM 없음 유지, 새 프로젝트 등록 없음.
- 독립 read-only 검토 후 실제 코드 재확인: Part_00_Body/Com_Transform 존재, 반복 snapshot 축소 누적 없음, 일반 class 무변경 및 stage 종료 복구, body/shadow 공통 matrix, 별도 광대 장비 없음, Server collider 무변경.
- Client Debug x64 Build 성공(exit0), 필요한 Engine/Shared 참조도 빌드했다. Client/Bin/Debug/Client.exe: 2026-09-09 16:29:38, 48,176,640 bytes. 기존 코드페이지/셰이더/DirectXTK PDB 경고는 남아 있다. 이번 변경으로 Server source/protocol은 수정하지 않았다.
- 화면/전환 입력 확인은 사용자 전용이다. Client/UI 자율 실행·조작·캡처는 하지 않았다. 새 Client -> Lobby KoukuSaydon -> F1 -> 마리오에서 약2m 광대/1.5배 공을 확인하고, 마리오 밖 광대와 일반 class 크기도 확인한다. Resources Drive 추가 배포 없음.

## G06. 최고 4m 조정 및 폭탄 영상 검토

검증/반영: 기존 publisher WorldSequences 범위 Publish(내부 검증 포함)/Check 성공(exit 0). source/runtime JSON 일치와 관련 git diff --check 성공. 런타임 반영 완료.

사용자 요청에 따라 기존 12개 공의 Y offset만 4/3배 변경해 최고 4m로 조정했다. G05의 빠른 반동 1200ms 주기, 키 시간, 상대 위상은 그대로 유지했다. revision 417 -> 418. 다른 JSON 원소와 배치 파일 byte 불변, 각 track 최저 0m/최고 4m 및 양끝 일치 검사를 통과했다. C++/Resources/Server 변경 없음, 재빌드 불필요. 새 4m 화면은 사용자 확인 대상이다.

폭탄 등장.mp4(7.47초/30fps)의 0.5초 간격 15프레임을 열람했다. 폭탄의 반복 발사는 진입 트리거가 발사 지점의 반복 출현을 활성화하고 생성 간격/비행 경로/지속 시간/소멸 조건으로 제어하는 설계를 안내한다. 비행 표현은 기존 Object World Sequence 경로를 재사용할 수 있지만 Server 생성/피해 계약을 완성했다고 주장하지 않는다. 이번 요청의 질문에 답하는 범위이며 폭탄 데이터나 코드는 변경하지 않았다.

## G05. 공 점프.mp4 참고 반동 수정

배포 검증: 기존 Map publisher WorldSequences 한 파일 범위 Publish(내부 검증 포함)/Check 성공(exit 0). source/runtime JSON 동등성과 관련 git diff --check 성공. Client/Server 프로세스는 확인 당시 실행 중이 아니며 에이전트가 실행하지 않았다.

사용자 첨부 영상(7.03초/30fps)을 로컬 디코딩하여 개요 및 1.5~4.4초의 0.1초 간격 프레임을 열람했다. 정지된 플레이어 화면 구간이 포함돼 있어 영상 전체 길이를 주기로 사용하지 않았다. 위쪽 체류, 급격한 낙하, 접지 직후 빠른 상승을 관찰했다. 이는 원본 애니메이션 데이터 추출이나 정확한 속도/미터 계측이 아니며 아래 값은 재현 느낌을 위한 튜닝이다.

- 최고 높이 3m 유지. 기존 2262ms 대칭 포물선을 1200ms 비대칭 곡선으로 교체했다.
- 180ms 빠른 상승(감속) -> 540ms 최고점 유지 -> 440ms 가속 낙하 -> 40ms 접지 후 반동.
- 메시 크기/회전/XZ는 그대로이며 squash/stretch, 충격 이펙트, 피해 판정은 추가하지 않았다.
- 12개 공의 상대 위상은 유지. 20ms 기본 키에 각 위상의 상승/최고점/낙하/접지 경계 키를 추가했다.
- revision 416 -> 417. 다른 template/instance/objectResource 전부 동일, 배치 파일 byte 불변 검사를 통과했다.
- 12개 곡선의 최고 3m/최저 0m, 시간 단조 증가, 0/1200ms 위치 일치, 빠른 초기 상승과 가속 낙하, 각 위상 경계 포함을 검사했다.
- C++/프로토콜/Resources 변경 없음. 컴파일은 불필요하고 Client 재시작으로 새 데이터를 읽는다. 사용자 3m 이전 버전 확인과 이번 새 움직임의 화면 확인은 별개이며 이번 화면은 아직 사용자 확인 대기다.

## G04. 최고 3m 재조정

기존 Map publisher의 WorldSequences 한 파일 범위 Publish(내부 검증 포함)/Check 성공(exit 0), 관련 JSON git diff --check 성공. 런타임 반영 완료.

사용자 요청으로 기존 12개 공의 최고 상승 높이를 3m, 왕복 주기를 2262ms로 변경했다. 1800*sqrt(3/1.9)=2261.8111ms를 정수 ms로 반올림하여 이전 포물선의 가속감을 유지한다. 상승/하강은 각각 약 1.131초다. 41키 시간과 높이를 비례 조정하며 엇갈리는 상대 위상을 유지했다.

Authoring revision 415 -> 416. 다른 JSON 원소와 배치 파일 바이트가 불변임을 확인했다. 12 track 모두 최저 0m/최고 3m, 시간 단조 증가, 0~2262ms 주기와 양끝 위치 일치 검사를 통과했다. C++/Resources/프로젝트 변경이 없어 재빌드와 Server 재시작은 불필요하며 Client 재시작으로 읽는다. 새 3m의 화면 결과는 사용자 확인 대상이다.

## G03. 사용자 확인 후 바운스 높이 조정

사용자가 런타임 진입·방향 이동·공 바운스가 동작한다고 직접 확인했다. 따라서 아래 G02의 표시/이동 재확인 대기는 이 관찰 범위에서 해소됐다. Mario4 후반 전체 클리어 검증을 대신하는 확인은 아니다.

요청에 따라 기존 12개 줄무늬 공의 최고 상승 높이를 원래 배치 위치 기준 1.5m에서 1.9m로 변경했다. 주기는 1600*sqrt(1.9/1.5)=1800.7406ms를 반올림해 실용적으로 1800ms로 정했다. 상승/하강은 각각 약 0.9초이며 기존 포물선의 가속감은 거의 유지된다. 41키 간격은 45ms, 기존 상대 위상은 그대로라 공끼리 엇갈리는 순서를 유지한다.

- Authoring revision 414 -> 415. 해당 template의 12개 track timeMs/Y와 durationMs만 변경했다.
- 각 track의 최저 0m/최고 1.9m, 주기 양끝 위치 일치, 키 시간 0~1800ms 검사를 통과했다.
- 변경 전후 JSON 구조 비교에서 다른 template/instance/objectResource는 전부 동일하며 배치 파일 바이트도 불변이다.
- Map publisher WorldSequences 범위 Validate/Publish/Check 모두 성공(exit 0), 런타임 시퀀스 파일 1개만 갱신했다. 관련 JSON diff --check도 성공했다.
- C++/프로토콜/프로젝트/XML/Resources 변경 없음. 기존 실행 파일이 데이터 길이를 읽으므로 컴파일 및 Server 재시작은 필요 없다.
- 기존 1.5m 화면 확인은 사용자 완료. 조정한 1.9m 화면은 Client 재시작 후 사용자 확인 대상이다.

## G02. 런타임 이동 불가 재현과 수정 (2026-09-09)

사용자는 Debug 제품 런타임에서 네 마리오 모두 진입은 되지만 방향키 이동이 안 되고 공도 보이지 않는다고 보고했다. 아래 G01의 최초 화면 미실행 기록은 당시 상태이며, 현재 사용자 화면 결과는 실패/재확인 대기다.

원인: 현재 Server bootstrap의 Mario1_go~Mario4_go는 disabled, 각 Intro는 enabled다. Intro로 입장한 뒤 Configure_MarioRail이 비활성 go를 경로 좌표의 근거로도 거부하여 bMarioRailReady=false가 유지됐고, Handle_MarioMove가 방향 입력을 거절했다. 비활성 트리거의 발동 여부와 이미 입장한 플레이어의 경로 메타데이터 사용을 혼동한 조건이다.

GameRoom.cpp에서 arrival의 enabled 검사만 제거했다. exit enabled, 고정 경로 목록, stage, trigger kind, 단일 MOVE_PLAYER 검사와 ServerTriggerSystem의 disabled trigger 제외는 유지한다. 사용자 트리거를 활성화하거나 배치·네비 데이터를 변경하지 않았다. 기존 ServerGameplayContractTests의 네 입장 검사에 rail-ready 및 트리거 enabled 값 불변 검사를 추가했다.

검증:

- 수정 전 기존 --debug-teleport-contract-test: 15 failures, 네 go 모두 invalid authored binding 재현.
- Server Debug x64 빌드 성공. Server/Bin/Debug/Server.exe: 2026-09-09 13:43:29, 12,743,168 bytes. 로그 out/MarioBounce/server-fix-build.log.
- 수정 후 같은 테스트: 네 마리오 입장 직후 경로 준비/방향 이동/점프 검사 및 17개 경로 축 검사 PASS. 로그 out/MarioBounce/server-mario-contract-after.log.
- 전체 테스트는 3 failures가 남아 PASS가 아니다: 현재 Mario4 마지막 귀환 위치와 기존 테스트 기대 불일치에 따른 클리어/복귀 검사 2개, T2의 기존 축 유지 기대 검사 1개. 현재 T13은 (-3.901,1.31762564,733.617004)로 이동한다. 이 사용자 저작 데이터를 예전 go 위치로 되돌리지 않았다.
- 별도 Client 코드/리소스 수정 없이 이번 이동 문제는 Server 실행 파일 갱신이 필요하다. 확인 당시 Client/Server 프로세스는 없었다. UI 실행/조작 및 화면 캡처는 하지 않았다.
- git diff --check 성공(exit 0). 최초 sandbox 검사는 Git LFS 임시 파일 권한으로 실패했고, 동일 검사를 승인된 실행으로 재수행했다. 기존 LF/CRLF 경고는 남아 있다.

공 조사 상태:

- 배포 mapplacements의 MarioProps 50개와 줄무늬 공 12개가 visible=1이고, 12개의 바운스 binding이 모두 실제 배치에 대응한다. 물리 wmodel 및 diffuse DDS가 존재하며 DDS alpha는 255다.
- 인스턴싱의 NONANIM stride, 월드 행렬/바운드, BottomCenter 보정, legacy material 호출에서 재현 가능한 누락 원인은 찾지 못했다. 렌더러를 추측으로 수정하지 않았다.
- 입구에서 가장 가까운 줄무늬 공의 XZ 거리: Mario1 ID429 약65.81m, Mario3 ID443 약10.68m, Mario4 ID462 약18.84m. Mario2에는 줄무늬 공 배치가 없다.
- 이동 차단 상태의 입구 화면만으로 공 렌더링 실패를 확정할 수 없다. Server 재시작 후 3마리오의 ID443 위치(-1894.2323,-11.5270214,-1655.8811) 부근에서 사용자가 바운스와 표시를 확인해야 한다. 공 표시 문제의 해결/visual PASS는 아직 선언하지 않는다.

공통 검증과 인계: 변경 C++ UTF-8 BOM 없음 유지, 신규 파일/프로젝트/프로토콜 변경 없음. Server project/filter XML 및 관련 source/runtime worldsequence JSON parse 성공. Resources Drive 추가 배포는 없다. 실제 접속 대상 Server를 새 바이너리로 재시작해야 하며 Client만 다시 실행해서는 서버 이동 수정이 적용되지 않는다.

## 구현 상태

사용자가 저장한 줄무늬 공 12개를 기존 CWorldSequencePlayer의 MAP_PLACEMENT track으로 연결했다. 새 공을 중복 생성하지 않고 기존 배치의 렌더 Transform만 움직인다. 마리오 진입은 Server-replicated iMarioStage 1~4로 판단하며, 마리오 밖에서는 기존 Active/Held owner를 Stop(true)로 정리해 원래 위치를 복원한다. 기존 컷신/카메라/플레이어 변신/입장 명령은 변경하지 않았다.

- 대상 ID: 429,430,431,443,449,455,459,461,462,463,468,469.
- 기본 높이: 1.5m. 주기: 1600ms. 수평 이동 및 회전/크기 변화 없음.
- phase: 위 순서로 0,800,400,1200,200,1000,600,1400,0,800,400,1200ms.
- 모션 정의: sequence.mario.striped_ball.bounce.
- 재생 ID: world.sequence.instance.mario.striped_ball.bounce.
- 표시 이름: Mario Striped Ball Bounce / 줄무늬 공 엇갈려 튀기.

포물선 y=6*u*(1-u)를 40ms 간격 41키로 표현한다. 서로 다른 위상 키를 가진 12track이 같은 Server tick 기반 시계를 소비한다. 큰 tick 값은 double로 계산하고 snapshot 사이 프레임 시간은 100ms까지 보간한다. 반복은 Level이 같은 instance를 순환 Seek하는 방식이다. 기존 일반 MAP_PLACEMENT의 STOP parser/publisher 계약을 바꾸거나 생성형 OBJECT_RESOURCE 전용 LOOP를 맵 binding에 억지로 허용하지 않았다.

Apply_RuntimeRecord는 entry.record 원본 TRS를 덮어쓰지 않는다. 기존 Update에서 한 주기 종료 후 Held로 옮겨져도 다음 Play가 같은 ID의 held owner를 복원한 뒤 baseline을 확보하므로 높이를 누적하지 않는다. Seek 실패는 명시 Stop(true) 후 로그를 남기고 마리오를 나가기 전까지 자동 재시도하지 않는다.

## 검증

- 사용자 placement 원본 파일은 byte 불변. 총 3281개, SHA-256 81a180d1436b46e06d9d2b9dfd8f8e76b3d2a0e2963f3dcf4ece8d87b75219b0.
- 12 binding이 실제 MAP_MARIO_STRIPED_BALL 배치 전체와 일치. 기존 시퀀스에는 이 12개를 소유하는 binding이 없어 현재 저장본 기준 active owner 충돌 없음.
- 모든 곡선은 높이 0~1.5m, XZ=0, rotation identity, scale=1이며 0/1600ms 높이가 동일. 첫 track은 시작 후 상승, 두 번째는 하강. 1ms 간격 수치 검사에서 선형 키 보간과 포물선 오차 최대 약 0.000938m.
- 기존 worldsequence 원소 전부 불변, template/instance 하나씩만 추가, revision 413 -> 414. 새 Resource 없음. 프로젝트/filters XML parse 성공.
- 기존 Map publisher Validate / Publish / Check 모두 성공. 3281 placements, runtime 8파일. 이는 JSON/배포 검증이며 화면 재생 PASS가 아니다.
- 독립 읽기 전용 검토에서 baseline/held 복원 및 현재 owner 충돌 부재를 확인하고, Seek 실패 rollback과 큰 tick 정밀도 지적을 실제 코드에 반영했다.
- Client Debug x64 Build 성공(exit 0). Client/Bin/Debug/Client.exe 갱신 확인: 2026-09-09 12:55:43, 48,145,408 bytes. 로그: out/MarioBounce/client-debug-build.log. 기존 shader/인코딩 및 DirectXTK PDB 경고는 남아 있으며 오류 없이 링크 완료했다.
- 변경 JSON/XML parse 및 git diff --check 성공. 사용자 기존 배치와 기존 시퀀스 항목은 보존했다.
- 사용자 화면 검증: 미실행. Client/UI를 자율 실행하거나 캡처하지 않았다.

## 사용자 확인

Client를 새 실행 파일로 실행 -> Lobby KoukuSaydon -> F1의 1마리오, 3마리오 또는 4마리오 진입. 이 세 구역에 현재 줄무늬 공이 배치되어 있다. 2마리오에는 현재 대상 공 배치가 없다. 공마다 위아래 타이밍이 엇갈리고 마리오 밖으로 나가면 원래 높이로 돌아와야 한다.

완료 시점 로컬 Client/Server 프로세스는 실행 중이 아니다. 팀 LAN 설정은 client 역할이며 정본 endpoint 192.168.0.14:7777을 유지했다. 팀 Server가 켜진 상태에서 사용자가 Client 프로젝트를 Ctrl+F5로 실행한다. 에이전트는 Client나 Server UI를 시작하지 않았다.

높이와 phase는 MapTool World Sequence 목록의 위 표시 이름, ball_<placementId> track keys에 저장돼 있다. 별도의 Bounce Height/Start Phase 슬라이더는 추가하지 않았다. 마리오에 있는 동안 상시 활성 시퀀스가 있으므로 기존 World Object Preview/Reload guard가 적용된다. 수정/Reload는 마리오 밖이나 Lobby Test에서 수행한 뒤 Map publish하고 재진입한다. 이 12개에 다른 시퀀스를 동시에 연결하지 않는다.

현재 기능은 순수 외형이며 공에 닿았을 때 피해·넉백·튕김이나 공 위 통행은 포함하지 않는다. 앞으로 새로 배치한 공은 이 instance에 binding/track을 추가해야 한다. Resources Drive 추가 배포는 필요 없고 코드와 Map runtime 데이터 갱신이 필요하다. 기존 MarioProps 리소스는 그대로 사용한다.
