# 쿠크 2관문 대형 세이튼 내려치기 기준점 검증 결과

## G00. 확인된 원인

설치 Gameplay revision 1987과 실제 CGameRoom 소비자로 Gate2의 published entry 순서를
P87까지 재생했다. bundle.3의 P13 조커 찾기에서 잘린 root motion을 반복한 끝 위치가
뒤 패턴으로 이어진다. 플레이어를 살아 있게 유지한 실행에서 대형 세이튼 spawn은
`(10.2399998,10,317.75)`, P13 뒤 위치는 `(12.5690441,10.1656637,315.655182)`이며
XZ 차이는 `3.13252473m`다. 이 위치로 P86/P87이 시작한다.

P21 레이저, P23 팡파레, P24, P85 동안 큰 세이튼의 좌표는 매 tick 그대로였다.
큰/작은 세이튼의 entity 배열 순서를 바꿔도 동일했다. 따라서 작은 쿠크의 이동이 다른
보스에 쓰이는 alias 문제로 재현되지 않았다. P13 native root를 불량으로 단정해 바꾸지 않았다.

## G01. 검증한 후보와 기존 동작

독립 메모리 catalog에서 P86/P87의 `resetBossToSpawn` 두 값만 true로 바꿨다.
두 패턴 모두 첫 tick에 자체 spawn XYZ로 돌아와 시작점 오차가 0이다.
`resetBossYawDegrees`는 추가하지 않았고 흐름 중 현재 yaw `-138.031082°`를 유지했다.
이 fixture에는 reset 순간 동적 support가 없어 Y도 10으로 돌아왔다. 실제 WORLD support가
있으면 기존 소비자의 max(spawnY, supportGroundY) 정책이 적용된다.

원래 root 표본과 stage 시간은 보존했다. P86은 시작점 기준 최대 `2.49005079m`,
P87은 `0.850104868m` 움직이는 원래 애니메이션을 그대로 수행했다.
P86은 끝에 시작점으로 돌아오고 P87은 원본 curve의 약 0.0001m 잔차가 남는다.
다음 P87 시작에서는 다시 자체 spawn을 사용한다.

Server/Client 코드 변경은 없다. `Prepare_KoukuAuditionTick`이 own entity의 spawn을
사용하고 root origin은 그 뒤 캡처한다. 따라서 동일 yaw로 독립 재생할 때와 같은 몸체
기준점을 사용한다. 서로 다른 yaw의 독립 재생을 같은 손 위치로 취급하지 않는다.
손/이펙트의 GPU 화면은 이 CPU 검증의 판정 대상이 아니다.

## G02. 실행 증거와 완료 경계

- `out/KoukuBigSaydonMotion20260921/probe.cpp`, `compile.rsp`, `link.rsp`, `build.cmd`
- `full-flow-baseline.log`: 실제 기존 소비자와 원본 catalog, normal/reverse 모두 실패 0.
- `full-flow-candidate.log`: 두 reset flag만 변경, normal/reverse 모두 실패 0.
- `motion-receipt.json`: 현재 읽은 source revision/hash, P13/P86/P87 저작 범위,
  source와 설치 revision 1987의 구분, 실행물 hash와 수치.

이 probe는 published Gate2의 selected/bundle entry를 같은 순서로 직접 요청하고 실제
world update를 실행했다. 네트워크 listener, cinematic, 전체 raid lifecycle, 실제 화면을
실행한 것으로 기록하지 않는다. 플레이어 응답이 없는 mechanic은 timeout 경로로 진행했다.

주 작업자가 최신 사용자 저장본 revision 2023에 두 stable field를 팡파레/레이저 후보와 병합해
revision 2024로 저장하고 publish를 완료했다. 게시 Encounter의 두 reset flag도 true로 확인했다.
배포·빌드 기록은 `2026-09-21_KOUKU_FANFARE_LASER_PUSH_IMPLEMENTATION_RESULT.md`에 있다.
이 문서는 독립 catalog 소비자 검증을 소유하며 사용자 Client 화면 판정은 아직 남아 있다.
