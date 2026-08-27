# 2026-08-27 베른 북쪽 출입 계단 내비게이션 수정 계획

## 목표

로비에서 캐릭터 생성 후 베른에 진입했을 때, 북쪽 아치 아래의 보이는 계단 끝까지
Server 권위 이동이 이어지게 한다. 화면 모양으로 좌표를 추정하지 않고 맵 배치 원본과
내비게이션 격자 경계를 대조한 실제 좌표를 기준으로 수정한다.

## 확인된 사실

- 계단 중심은 맵 배치 원본에서 `X=136.86001`로 고정되어 있다.
- 반복 계단 `BERNCASTLE_STAIR02C`는 `Z=-16.7400049`부터
  `Z=-5.22000488`까지 이어진다.
- 마지막 계단 `BERNCASTLE_STAIR02A`의 중심은 `Z=-3.30000488`이다.
- 현재 내비게이션은 `originZ=-175.938004`, `height=333`,
  `cellSize=0.5`이므로 북쪽 경계가 `Z=-9.438004`에서 끝난다.
- 따라서 `Z=-9.06` 이후의 실제 계단이 Server 내비게이션 범위 밖이다.
- 이 구간에는 Server 정적 collisionBox가 없으므로 충돌 상자가 원인이 아니다.

## 구현

1. Bern navsource/navpaint 높이를 333행에서 347행으로 확장한다.
2. 기존 마지막 행에서 연결된 통행 성분 `x=21..29`만 새 행에 이어 붙인다.
3. 새 북쪽 경계 `Z=-2.438004`로 마지막 계단 중심과 상단을 포함한다.
4. 실제 마지막 계단 중심 `(136.86001, -3.30000488)`까지 경로가 생성되는
   Server 계약 테스트를 추가한다.
5. publisher로 Server/Client runtime navigation을 함께 재생성한다.

## 검증

- Navigation publisher Validate/Publish/ContractTest
- Server x64 Debug 빌드 및 Server contract test
- `git diff --check`
- 최종 화면 이동은 사용자가 직접 확인
