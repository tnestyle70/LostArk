# 2026-08-27 베른 북쪽 출입 계단 내비게이션 수정 결과

## 결론

스크린샷의 북쪽 아치 계단은 collisionBox나 플레이어 충돌 때문에 막힌 것이 아니었다.
실제 계단 배치는 `Z=-3.30000488`까지 이어지지만 Server 내비게이션의 북쪽 경계가
`Z=-9.438004`에서 끝나, 보이는 계단의 마지막 네 구간이 격자 밖으로 잘려 있었다.

## 적용

- `LV_BER_BERNCASTLE.navsource`와 `.navpaint`를 `50×333`에서
  `50×347`로 확장했다.
- 새 북쪽 경계는 `Z=-2.438004`이며 마지막 계단 중심과 상단을 포함한다.
- 기존 마지막 행에서 같은 높이로 연결된 `x=21..29` 통행 성분만 연장했다.
- 옆 지붕과 장식물은 새 통행 영역에 포함하지 않았다.
- Server와 Client의 runtime navigation을 publisher로 함께 재생성했다.
- Server 계약 테스트에 마지막 계단의 실제 월드 좌표
  `(136.86001, -3.30000488)`를 고정했다.

## 검증

- Navigation publisher Validate: PASS
- Navigation publisher Publish: PASS
- Navigation publisher ContractTest: PASS
- Server x64 Debug build: PASS, 경고 0, 오류 0
- Server `--navigation-contract-test`: PASS, failures 0
- Server `--contract-test`: PASS, failures 0
- 정확한 북쪽 계단 경로 회귀:
  `Reach the visible Bern north-entry stair through authoritative navigation`: PASS
- 이번 변경 파일 대상 `git diff --check`: PASS
- 저장소 전체 `git diff --check`: 기존의 별도 작업 파일에 남아 있는 conflict marker로
  실패했다. 이 작업의 파일에는 conflict marker나 whitespace 오류가 없다.

## 사용자 확인

실행 중인 Server와 Client를 완전히 종료한 뒤 새 Debug 빌드로 다시 실행한다.
로비에서 캐릭터를 생성해 베른으로 들어가고, 스크린샷의 아치 중앙 계단을 북쪽 끝까지
우클릭 이동한다. 최종 화면 이동 여부는 사용자가 직접 판정한다.
