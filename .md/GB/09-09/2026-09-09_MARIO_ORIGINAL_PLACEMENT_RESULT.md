# 마리오 1–4 원본 배치 교체 결과

## 반영 상태

일반 진입 ZoneLevel 0의 원본 37081 DeployData와 TriggerMapData를 함께 해석해
수동 MAP_MARIO 배치 50개를 원본 배치로 교체했다. 특수 ZoneLevel 2 분기는 제외했다.
원본 색 공은 진입마다 서버가 Case 1/2/3을 34/33/33 가중치로 선택한다.
같은 stage에 이미 참여자가 있으면 그 패턴을 공유한다. 재입장에서도 같은 패턴이
연속 선택될 수 있다. 후보 117개를 한꺼번에 보이는 방식이 아니다.

| 구간 | 선택된 색 공 | 줄무늬 공 | 고정 해골 폭탄 | 서버 몬스터 |
|---|---:|---:|---:|---:|
| 1마리오 | 9 | 3 | 3 | 3 |
| 2마리오 | 9 | 0 | 0 | 7 |
| 3마리오 | 9 | 3 | 4 | 14 |
| 4마리오 | 12 | 7 | 0 | 16 |

저장된 prop 후보는 총 137개이고 전체 mapplacements는 3,368개다.
원본 actor ID는 `source:37081:npc:<actorId>`로 남는다.
좌표는 UE cm(X,Y,Z)에서 제품 m(X,Z,-Y)로 변환했다.
몬스터 40개 anchor는 기존 서버 네비 위의 원본 XZ를 유지하며 높이 허용 오차를 검사한다.
새 네비 bake는 하지 않았다.

마지막 참여자가 퇴장하면 해당 stage의 몬스터 그룹을 despawn/reset한다.
클라이언트 색 공 시퀀스도 숨김 baseline으로 복귀한다.
종료 트리거의 목적지가 기존 입구가 아닌 아레나여도 Mario lane graph의 terminal exit로
인식해 stage/layout을 해제하도록 수정했다. 기존 마리오 이동과 종료 검사도 통과했다.

## 보존과 남은 범위

- 기존의 다른 맵 배치 3,231개는 변경하지 않았다.
- Gameplay.world.json은 작업 전 백업과 바이트 단위로 같다. 이동·컷신·카메라와
  사용자가 설치한 7개 비행 폭탄 marker를 유지했다.
- 기존 줄무늬 공 시퀀스의 대상만 원본 13개로 바꾸고 높이 4m, 주기 1,200ms,
  엇갈림 곡선과 공 배율 1.5를 유지했다. 플레이어 크기는 이번 작업에서 바꾸지 않았다.
- 다른 world sequence와 object resource는 보존했다.
- 원본 모델/대기 애니메이션과 배치는 연결했지만 원본 전투 AI, 공 파괴 목표,
  폭탄 피해 판정까지 복원한 상태는 아니다. 몬스터 프로필의 추적 범위는 최소값이며
  엄밀한 공격 금지 정책은 아니다.
- 기존 비행 폭탄 marker를 원본 emitter/AI 배치로 대체하지 않았다.

삭제된 수동 배치는 아래 로컬 백업에서 복구할 수 있다. 폴더를 자동 삭제하지 않았다.

`out/MarioOriginalReplacement/backup-before-replacement/`

## 리소스 인계

새 리소스의 물리 위치는
`Client/Bin/Resources/Character/Monster/MarioOriginal/`이다.
REUP, RHKP, CDMD 세 폴더와 각각의 textures 하위 폴더를 함께 Drive로 전달해야 한다.

| Resources 상대 asset ID | 구성 |
|---|---|
| Character/Monster/MarioOriginal/REUP/REUP.wmodel | 몸체·무기, 2 mesh, 30 animation |
| Character/Monster/MarioOriginal/RHKP/RHKP.wmodel | 몸체·무기, 3 mesh, 47 animation |
| Character/Monster/MarioOriginal/CDMD/CDMD.wmodel | 몸체, 1 mesh, 30 animation |

기존 MarioProps와 카드미로 HEART/DIAMOND/CLUB 리소스도 계속 필요하다.
신규 리소스는 로컬 생성·배치까지 완료했고 Drive 업로드/ZIP 전달은 하지 않았다.
원본 glTF/PSA와 변환 중간 파일은 out에만 두었다. Resources를 Git에 추가하지 않았다.

## 검증 증거

- Client Debug x64 빌드 성공: `out/MarioOriginalReplacement/client-build.log`.
  기존 shader/PDB 경고는 남아 있다.
- Server Debug x64 최종 빌드 성공: `out/MarioOriginalReplacement/server-final-build.log`.
- 기존 Server `--debug-teleport-contract-test`: failures 0.
  패턴 범위, stage별 3/7/14/16 실제 생성, 참여자 공유, 퇴장 정리, 재입장,
  기존 이동/점프/trigger 전이 검사 포함. `server-contract-test.log`에 기록했다.
- 기존 NetworkProtocolHarness `--mario-controls-only`: exit 0.
  variant round-trip, 잘못된 값 거부, 실패 시 출력 보존 검사 포함.
- WorldGameplay Validate/Publish 성공, Kouku SpawnGroup 5개.
- Map Validate/Publish 성공, 3,368 placements.
- 변경 authoring JSON과 runtime worldsequences JSON 파싱 성공.
- Client/Server/Shared/NetworkProtocolHarness vcxproj 및 filters XML 파싱 성공.
- Mario catalog 6개 모델의 실제 WModel과 지정 animation clip 존재 확인.
  신규 CDMD/REUP/RHKP의 내장 texture 참조 4/6/7개가 모두 실제 DDS 파일로 연결된다.
  animation section은 각각 30/30/47개다. 이는 구조 검사이며 화면 판정이 아니다.
- `git diff --check` 성공. Git LFS 임시 캐시 권한 때문에 승인된 범위에서 재실행했다.
- 작업 종료 시 Client/Server 실행 프로세스 없음. 에이전트는 Client/UI를 실행하지 않았다.

World sequence의 MAP_PLACEMENT 트랙은 publisher가 허용하는 STOP 종료를 쓴다.
기존 player가 종료 pose를 유지하고 명시적 Stop_Instance(..., true)가 baseline을 복원한다.
재생 완료 후 매 프레임 재시작하지 않도록 Client 시작 상태를 별도로 보존한다.

## 사용자가 확인할 순서

네트워크 프로토콜은 76이다. 공유 Server PC도 같은 코드와 WorldGameplay 생성물을
갱신하고 서버를 재시작해야 한다. 이전 서버와 새 Client를 섞어서 테스트하지 않는다.
이 PC는 팀 LAN client 역할이므로 Visual Studio에서 Client 프로젝트를 Ctrl+F5로 실행한다.

1. Lobby → KoukuSaydon → F1 → 1/2/3/4마리오를 선택한다.
2. F1을 닫고 컷신 이후 색 공 한 패턴, 구간별 몬스터, 줄무늬 공의 움직임을 확인한다.
3. 마리오 밖으로 나왔다가 다시 들어가 패턴이 재선택되는지 확인한다.
   확률 선택이므로 같은 패턴이 반복되는 것은 정상이다.
4. 몬스터 모델 방향·크기·무기 부착과 실제 통로 동선을 확인한다.

화면 검증과 다중 Client 동시 재생은 아직 사용자가 확인하지 않았다.
자동 검사 성공을 visual PASS로 기록하지 않는다.
여러 이전 작업이 섞인 dirty worktree이므로 자동 stage/commit/push는 하지 않았다.

공유 계약은 TEAM_GAMEPLAY_INTERFACE_HANDBOOK의 4.3과 AREA_DATA_LAYER_GUIDE에 반영했다.
코드 전문과 실제 데이터 변경 블록은 대응 PLAN에 보존한다.
