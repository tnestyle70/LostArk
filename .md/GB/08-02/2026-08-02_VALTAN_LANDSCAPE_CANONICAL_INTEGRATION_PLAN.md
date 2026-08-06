# 발탄 Landscape 정본 통합 계획

상태: 구현 및 자동 검증 완료. MapTool Save 버튼의 수동 클릭 재검증만 사용자 확인 항목으로 남는다.

## 목적

발탄 원본 Landscape 6개를 별도 테스트 Area로 남겨두지 않고 기존
`LV_LUT_HEARTRB_ED` MapTool 카탈로그와 배치 문서에 합친다. 통합 후에도 MapTool의
편집, Save, Reload 경로는 기존 단일 문서 방식을 그대로 사용한다.

## 작업 경계

- 맵 데이터와 맵 생성 도구만 수정한다.
- 캐릭터, 몬스터, 이펙트, 전투 코드는 수정하지 않는다.
- 기존 269개 에셋과 13,103개 배치를 손으로 편집하지 않는다.
- Landscape의 원본 `component` Transform과 imported ID를 보존한다.
- UE3 PC 렌더 계약에 따라 hole 때문에 render topology를 삭제하지 않고,
  `__DataLayer__ > 170`을 표시용 opacity mask에 적용한다.

## 구현 순서

1. 기존 정본과 Landscape 문서의 스키마, Area, ID, Prototype, 참조 무결성을 검사한다.
2. Landscape WModel 6개의 경로와 컨테이너 헤더를 검사한다.
3. 재실행해도 중복되지 않는 병합 도구로 275개 에셋과 13,109개 배치를 생성한다.
4. 현재 DDS 디코더와 hole 규칙으로 재생성한 baked diffuse/normal 12개만
   런타임 리소스에 반영한다.
5. 병합 도구 단위 테스트, 문서 재파싱, 해시 재실행 검증을 수행한다.
6. `--map-level=valtan` 실행과 F1 MapTool Save/Reload를 확인한다.

## 실패 처리

ID 충돌, Prototype 충돌, 잘못된 Area, 누락된 WModel, 잘못된 Transform source,
개수 불일치가 하나라도 있으면 출력 문서를 교체하지 않는다. 두 출력은 모두 임시 파일에
완성한 후 교체하며, 두 번째 교체가 실패하면 첫 번째 문서를 복구한다.
