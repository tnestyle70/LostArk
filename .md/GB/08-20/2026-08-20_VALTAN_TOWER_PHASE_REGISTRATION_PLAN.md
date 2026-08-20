# 발탄 아레나 후방 고딕 탑 source attachment 복구 계획

> 기준일: 2026-08-20 KST  
> 사용자 목표: 현재 원형 전투장 벽 바로 뒤에 참조 사진의 높은 고딕 탑 4개를 복구한다.  
> 수동 판정 경계: 자동 데이터·빌드 검증 뒤 실제 화면 일치는 사용자가 Debug Client에서 판정한다.

## 1. 확인된 원인

- 대상은 `LV_LUT_HEARTRB_ED_SL04`의 반복 고딕 탑 조립체다.
- 원본 tower station은 상부 47개만으로 완성되는 독립 구조물이 아니다. 원본 transform의 상부,
  하부 받침, 체인이 함께 맞물려 하나의 연속 조립체를 이룬다.
- 이전 phase registration은 각 후방 station의 상부 47개만 `+10.6108742m` 이동하고 하부와 체인을
  원본 위치에 남겼다. 사용자 스크린샷에서 이 두 부분 사이에 약 `11.54m`의 수직 seam이 생겨
  상부 첨탑 4개가 공중에 떨어져 보이는 회귀가 확인됐다.
- 새 가짜 박스나 다른 외곽 첨탑을 복제하지 않는다. 현재 catalog와 Resources에 있는 원본
  OCastle/HeartRB WModel을 그대로 재사용한다.
- SL04 source 조립체 자체가 올바른 attachment 정본이므로 후방 4개 station과 전방 우측 다섯째
  station 모두 원본 transform을 유지한다.

## 2. 구현 계약

1. 후방 4 station에서 잘못 이동했던 source 188개를 원본 transform 그대로 `visible=1`로 복원한다.
2. `VALTAN_TOWER_REGISTERED` registration overlay는 0개로 유지한다. 같은 부재를 복제하거나 Y
   offset을 적용하지 않는다.
3. environment visibility override에서 해당 source를 숨기는 항목은 0개로 유지한다. 문서 전체
   override는 기존 atmosphere용 2개만 남긴다.
4. core overlay manifest에는 기존 phase proxy 6개만 남기고 tower overlay를 넣지 않는다.
5. 후방 4개와 전방 우측 `pointlight_11` point light는 모두 source Y `24.734033m`를 유지한다.
   maplight provenance도 `SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED`를 유지한다.
6. authoring/runtime placement 수는 각각 `13,186`으로 고정하고 publisher 동일성을 확인한다.

## 3. 자동 검증

- 선택 source 집합의 station별 동일 count와 asset 분포
- source 188개의 original transform과 `visible=1`
- registration overlay 0개, tower visibility override 0개
- core overlay 6개가 모두 기존 phase proxy이며 tower overlay가 없음
- 후방 네 light와 다섯째 light가 모두 `Y=24.734033m`이고 source provenance를 유지함
- authoring/runtime placement header와 MapCatalog가 모두 `13,186`
- authoring/runtime parity, Map publisher PASS, JSON parse, `git diff --check`
- ClientFrontendHarness Debug/Release focused test와 Client Debug/Release build

## 4. 완료 보고 경계

- 자동 검증 통과는 데이터 등록과 로드 계약 완료를 뜻한다.
- 에이전트는 Client/UI를 실행하거나 visual PASS를 선언하지 않는다.
- 사용자는 Lobby → Valtan → F1 → `Reference Exterior (Arena Towers)`에서 네 상부가 하부 받침과
  체인에 끊김 없이 붙었는지 확인한다. 새 스크린샷 기반 육안 재확인은 아직 대기 상태다.
