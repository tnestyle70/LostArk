# 쿠크 Pattern 늦은 lifecycle 수신 보존 계획

## G00. 원인과 범위

쿠크_피자는 KAKULSAYDON_G1_PATTERN_25, GATE2 쿠크에 연결된다. 현재 Server는 요청 scope를 결과와 lifecycle에 그대로 돌려주며 첫 고정 tick에서 ACTIVE를 보낸다. Client NetworkManager는 수신 FIFO를 한 Update당 최대 64 frame만 전달한다. Pattern Audition은 5초 verdict 또는 15초 queued deadline을 넘으면 ABORTED로 바꾸고 이후 exact 응답도 버린다. 최근 사용자 session의 최대 main pump 간격은 25,656ms다. 이는 큐 지연으로 실제 승인과 lifecycle을 잃을 수 있는 결함이며, 해당 사용자 클릭의 패킷별 진단은 남아 있지 않아 그 실행을 재현했다고 표현하지 않는다.

## G01. Client 수정

기존 deadline 값과 exact request/scope/epoch/gameplay/source revision 검사를 유지한다. deadline은 대기 안내만 표시하고 요청 소유권을 유지한다. 뒤늦은 거절과 ACTIVE/COMPLETED를 실제 Server 응답대로 소비한다. 승인 전 Stop도 기존 deferred Stop 경로로 연결하여 서버 epoch가 도착한 뒤 정확한 STOP을 보낸다. 같은 배치에서 자연 완료됐어도 남은 투사체의 epoch를 중단한다. 다른 world/session은 기존처럼 종료하며 새 요청은 미해결 요청을 덮지 못한다.

## G02. 검증과 전달

실제 NetworkManager FIFO/packet codec과 실제 AuditionService를 socket/UI 없는 C++ 회귀로 연결한다. 과거 코드의 늦은 응답 소실, 5/15초 경계, 한 배치 ACTIVE부터 COMPLETED까지의 소비, 잘못된 scope/revision/과거 request 거부, pending Stop과 disconnect를 검증한다. 현재 게시 데이터를 읽는 실제 Server GameRoom으로 P25 QUEUED/ACTIVE와 exact scope를 별도로 확인한다. 테스트 clock과 송신만 격리하며 게임의 MainApp이나 UI를 실행하지 않는다.

기존 C++ 파일의 UTF-8/CRLF와 다른 변경을 보존한다. 새 제품 C++ 파일과 vcxproj 등록은 없다. 소스 백업과 격리 검증은 out/KoukuPatternLifecycle20260922에 기록한다. 전체 Product 빌드와 게시 소유자는 root다. 발탄 돌 데이터는 root의 최신 저장본 병합 절차로 따로 반영한다.
