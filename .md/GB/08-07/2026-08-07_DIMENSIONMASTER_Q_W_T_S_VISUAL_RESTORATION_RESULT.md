# 2026-08-07 차원술사 Q/W/T/S 시각 복원 결과서

## 1. 현재 상태

구현 진행 중이다. 이 문서는 계획의 PASS를 미리 선언하지 않으며 실제 실행 결과만 누적한다.

## 2. 구현 결과

| G | 상태 | 결과 |
|---|---|---|
| G01 Screen Post | 진행 중 | RGBNoise/FilmNoise 계약 교정 예정 |
| G02 부모 Profile | 진행 중 | Shine/Blackline Aura/Local Crack 예정 |
| G03 재생성·발행 | 대기 | 코드/테스트 후 실행 |
| G04 빌드·회귀 | 대기 | 발행 후 실행 |
| G05 수동 A/B | 대기 | 자동 검증 후 사용자 캡처 |

## 3. 자동 검증 증거

아직 기록할 PASS가 없다.

## 4. 수동 검증 상태

- W/T의 full-screen noise 재검증 필요
- S 2050220 원작 정본 캡처 필요
- Q Blackline/Local Crack emitter Solo 필요
- E 2050160 `fm_h_box_01_1.wmodel` 모델 Material Solo 필요

## 5. 남은 경계

- runtime exact 0이라는 현재 의미는 유지한다.
- 이번 유한 Profile은 원본 Material graph 전체 복원이 아니라 증거가 있는 부모식의 bounded reconstruction이다.
- Camera/Sound/Character Material과 최종 Bloom/색/Scale/Pivot 픽셀 튜닝은 이번 자동 완료 기준에 포함하지 않는다.
