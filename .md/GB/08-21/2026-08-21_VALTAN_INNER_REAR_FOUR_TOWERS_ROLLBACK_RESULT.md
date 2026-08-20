# 발탄 아레나 후방 내벽 타워 추가 롤백 결과

> 검증일: 2026-08-21 KST
>
> 사용자 판정: 추가한 후방 내벽 타워의 시각 결과를 채택하지 않고 전부 제거한다.

## 완료 상태

- `VALTAN_INNER_REAR_TOWERS` presentation placement 188개를 authoring/runtime에서 제거했다.
- `inner-rear-tower:` source ID prefix와 stable placement ID
  `7300000000000000001..7300000000000000188`은 모두 0개다.
- authoring/runtime은 `275 assets / 13,186 placements`이며 placement SHA-256은 양쪽 모두
  `8945e9e7ed4d2641c20c0feaea137a4aa30fce1670f87d479e06563c4b9ad1cb`이다.
- core overlay는 기존 phase proxy 6개만 남는다.
- 기존 외곽 source tower, 하부 구조, chain과 map point light 22개는 위치와 데이터를 유지한다.

## 자동 검증

- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED`: PASS
- `ClientFrontendHarness.exe --valtan-camera-fast`: Debug/Release 각각 `46/46 PASS`,
  `failures 0`

Client/UI는 실행하지 않았으며 화면 결과를 에이전트가 대신 판정하지 않았다.
