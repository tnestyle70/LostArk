# 카드미로 1인 테스트 적용 결과

## 구현

2026-09-08 사용자 요청에 따라 Debug Server의 방에 정확히 한 명일 때만 테스트 겸임을 허용했다. 기존 4인 규칙과 Client 문양·카메라·이펙트 소비 경로는 변경하지 않았다.

- 중앙 상자 망치 타격 → Server가 HUNTER 문양 1개를 랜덤 배정하고 telescope owner identity를 같은 플레이어로 확정한다. snapshot 관전 flag가 즉시 망원경 시점을 켜고 HUNTER 역할이 기존 발밑 문양을 표시한다.
- 같은 문양 목표 1마리를 중앙과 연결된 랜덤 통로에 배치한다. 중앙 5m와 플레이어 4m 배제 조건은 유지한다. 처치 후 3스택까지 목표를 한 마리씩 보충한다.
- 1인 테스트는 관전 중 우클릭 이동을 허용하고 중앙 밖으로 나가도 관전 flag를 유지한다. 중앙 상자 재타격으로 관전을 토글할 수 있다.
- 첫 유효 목표 타격은 기존 서버 행진 시작 시계를 설정한다. 저작된 36개 세토 경로 반복과 접촉 시 스택/출구 초기화는 기존과 동일하다. 빈 바닥을 향한 헛스윙으로 행진을 시작하지 않는다.
- 3스택 출구와 중앙 암전 이동·복귀는 기존 판정을 사용한다. 1인이라고 자동 클리어하지 않는다.
- 다인 플레이의 담당자는 문양이 없다. 다른 팀원이 죽어 한 명만 살아남았다고 1인 테스트로 바꾸지 않는다. Release에는 이 Debug 배정 예외가 없다.

## 검증

- Product Debug Engine/Shared/Server/Client 빌드·정상 배포 성공: `out/BuildPipeline/runs/20260908T101448565Z-debug-product.json`.
- `Server/Bin/Debug/Server.exe --card-maze-contract-test`: failures 0. 1인 문양/관전 동시 배정, 통로 목표, 첫 타격만 행진 시작, 3처치, 초기화, 관전 토글과 다인 비겸임 확인.
- GameRoom의 실제 이동 명령과 중앙 밖 상태 업데이트가 Is_SoloHunter를 소비한다. 카메라는 기존 CardMaze.flags & 1, 바닥 문양은 기존 HUNTER + suit를 소비한다. Shared/프로토콜·JSON·리소스 추가 변경 없음.
- `git diff --check` 성공. 변경 파일은 기존 UTF-8 유지. 기존 dirty 변경 보존, 자동 stage/commit/push 없음.
- 독립 read-only 검토에서 1인 camera/이동/문양 consumer, 세토 접촉 유지, 다인 비겸임 연결을 재확인했다. 추가 blocker 없음.
- Client/UI 실행·조작·캡처 및 화면 PASS는 하지 않았다. Release 빌드는 이번에 실행하지 않았다.

## 혼자 실행 순서

1. 팀 서버 PC에서 이번 **Debug Server**를 다시 실행한다. 접속 endpoint는 `192.168.0.4:7777` 그대로이며, 점검 시 not-listening이었다. 이 PC는 LAN client 역할이므로 Client 프로젝트 Ctrl+F5가 설정돼 있다.
2. 새 Client에서 Lobby → KoukuSaydon → F1 → KoukuSaydon Arena → 카드미로로 진입한다. 서버의 해당 방에 본인 한 명만 있어야 한다.
3. 중앙 상자를 MAZE 망치로 친다. 망원경 시점, 본인 발밑 랜덤 문양, 같은 문양의 통로 병사가 표시되는지 확인한다.
4. 우클릭으로 병사에게 이동해 망치로 타격한다. 첫 적중 뒤 세토 행진이 반복되는지, 접촉 시 스택이 초기화되는지 확인한다. 중앙 반경 5m는 안전하다.
5. 같은 문양 3마리를 처치하고 생성된 문양 출구를 밟아 중앙 암전 이동과 2관문 복귀를 확인한다. 화면 결과는 사용자 확인 대기다.

새 Resource payload나 추가 Drive 전달은 없다. 직전 바닥 문양 작업에서 사용한 네 DDS와 카드 병사·세토 리소스를 그대로 사용한다.
