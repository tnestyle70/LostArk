# 쿠크 아레나 공중 이탈·효과·조준 통합 결과

## G00. 완료

저장본 revision 2040을 CAS 확인한 뒤 revision 2043으로 병합했다. Server ballistic lifecycle과
높은 nav deck 착지 guard, 요청된 Gate2 pattern push rows, 대형 세이튼 P99 wind collider,
P21 laser target tracking, P25 centre yaw를 연결했다.

P25 쿠크 spawn yaw 216.5도 기준 `resetBossYawDegrees`는 306.5도에서 126.5도로 바뀌었다.
P21에는 `kakulsaydon.g1.logic.65`를 0~6532ms로 추가해 cast 중 Server가 선택한 생존자
위치를 따라 보스 yaw를 갱신한다. ENTER_AREA의 플레이어별 overlap 판정은 그대로 유지한다.

두 full restore effect와 stable resource 등록도 완료했다. 파1빨2와 Gate1 무력화 effect는
변경하지 않았다. Gate3Fine navigation은 authored 대기 테라스 조사 결과에 따라 변경하지
않았다.

## G01. 실행한 검증

- Composition publish: PASS (`productPatternCount=89`, `productStageCount=487`).
- Composition validate: PASS.
- Gameplay balance publish: PASS.
- Generated Encounter source revision: 2043; P99 Product pattern 포함.
- Bootstrap 확인: P13/P21/P23/P24/P85/P86/P87/P99 ballistic push rows와 P21
  `BOSS_TRACK_TARGET` row 존재.
- 두 effect candidate SHA: wind `0c66f09a7b054170ef5c06dc4896a5a5de53370af4fd7c5209bd075b17ce8705`,
  yellow `a00b0846ded7d2ef152d8f37e60aa6f6349ea7a256df1a42a7c59bdd37ca0651`.

## G02. 남은 사용자 단계

변경 Server CPP의 targeted compile은 통과했으나 기존 Server.exe가 링크 출력 파일을 잠가
Product link가 완료되지 않았다. 사용자가 EXE를 종료한 뒤 Debug Product Build를 실행하고,
Client에서 실제 중앙 이동 방향, 레이저 추적, 바람/노란 시선 크기와 전방, 낙사 사망을 확인한다.
이 세션에서는 Client를 실행하거나 화면을 판정하지 않았다.
