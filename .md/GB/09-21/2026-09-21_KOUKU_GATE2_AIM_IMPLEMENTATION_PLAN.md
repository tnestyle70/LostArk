# 쿠크 2관문 기준 회전과 레이저 조준 구현 계획

## G00. 이번 요청과 현재 측정

사용자가 대형 세이튼의 이전 패턴 뒤 남는 회전과, 작은 쿠크의 현재 전방으로만 발사되는
레이저를 수정하도록 요청했다. 앞 작업의 P86/P87 위치 복원은 yaw를 유지하는 계약이었다.
이번에는 두 내려치기의 시작 yaw도 실제 배치 값 226.5도로 복원한다.

source revision 2025에서 대형 세이튼의 P13은 stage.3/.8/.22/.16에 retargetOnEnter를
사용하고 P17 잡기에는 해당 요청이 없다. 현재 P21은 retarget이 없으며 일곱 collider와
레이저 효과가 BOSS 기준 +X 전방, 일부 ±45도 오프셋을 사용한다.
기존 Server의 RETARGET_RANDOM_ALIVE는 대형 세이튼에만 +X 보정을 적용한다.

## G01. 변경 파일과 연결

`Server/Private/GameRoom_BossStageActions.cpp`의 기존 RETARGET_RANDOM_ALIVE 분기에서
G2 Kouku도 +X 모델 전방을 사용하도록 atan2(dx,dz)의 yaw에서 90도를 뺀다.
`Client/Private/KoukuSaydonPresentationPlayer.cpp`의 stage retarget preview에 같은
G2 Kouku 보정을 적용한다. 지속 tracking window의 기존 동작을 바꾸지 않는다.

P21 첫 stage.2의 retargetOnEnter=true와 P86/P87 resetBossYawDegrees=226.5는 out의
stable-field 후보로 준비한다. 주 작업자가 최신 저장본에 병합한다. P21은 시작할 때 살아
있는 플레이어 한 명을 기존 Server 선택 규칙으로 고르고 해당 방향을 고정한다. 후속
±45도 발사 모양, 타이밍, 속도, collider 크기, 효과 anchor는 유지한다.

P25의 별도 시계 90도 요청은 주 작업자 소유다. G2 Kouku 배치 yaw216.5를 기준으로
회전하면306.5도이며, 현재 start0의 grounded 중앙 이동과 기존 reset 계약을 함께 검증한다.

새 public interface, enum, 파일 또는 project/filter 등록은 필요 없다.

## G02. 최소 검증

설치된 최신 Gameplay와 실제 CGameRoom의 Gate2 entry 순서에서 대형 yaw 변경 시점과
stage를 기록하고, 독립 catalog 후보의 두 시작 yaw가226.5인지 확인한다.
P21은 실제 첫 stage action을 네 방향의 플레이어 위치로 실행해 모델 +X와 첫 collider가
같은 플레이어 방향을 향하는지 확인한다. 수정 전 소비자의 90도 오차를 대조한다.
기존 대형 retarget과 다른 archetype 동작, stage 내부 root motion은 보존한다.

변경한 실제 Server/Client TU는 scratch 경로로 컴파일하고 필요한 CPU probe만 실행한다.
제품 링크·Server listener·Client/UI 실행 및 live JSON 교체/publish는 하지 않는다.
