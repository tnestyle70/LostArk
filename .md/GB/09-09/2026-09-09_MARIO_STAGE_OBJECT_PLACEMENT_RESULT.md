# 마리오 배치 기능 조사 결과

작성일: 2026-09-09. 구현이 아닌 설계 요청 처리 결과다.

후속 조사: [공 원본 추적 결과](2026-09-09_MARIO_BALL_RESOURCE_TRACE_RESULT.md)에서 노란/파란 공의 정확한 LookInfo→메시→재질→DDS 연결을 확보했다. 아래는 그 전의 최초 조사 기록이며, 두 공의 미확정 결론은 후속 결과로 대체한다. 제품 코드/Resources 설치 및 MapTool 구현은 여전히 하지 않았다.

설계 정본: [마리오 배치 설계](2026-09-09_MARIO_STAGE_OBJECT_PLACEMENT_DESIGN.md).

## 확인한 것

- HEAD `ffca5286`, WorldSequence v3/revision 412의 모델·기본 Motion 참조를 읽었다.
- WorldSequence의 문서 로드와 실제 재생, 모델 리소스 validator, V2 Group/Leaf 소비 경로를 구분했다.
- MapEffect의 V1 LEVEL_ACTIVE 지원과 쿠크 MapCatalog의 effects pair 부재를 확인했다.
- MonsterCatalog 9종을 파싱했다. 마리오 적으로 등록된 항목은 확인하지 못했다.
- PlayerController의 Mario 일반 입력 early return, 서버의 class skill 거부, 기존 interaction slot 경로를 확인했다.
- Kouku symbol DDS 48개를 오프라인 열람했다. 폭탄 이미지와 삼각형·다이아 마스크 후보는 존재한다. 네 대상 모두의 원본 actor/particle 매핑을 확정하지 않았다.
- 기존 추출 결과서가 지시하는 WorldObjectExtraction-20260907 경로는 현재 없었다. 원본 추적 완료로 기록하지 않았다.

## 변경과 검증 범위

- 제품 코드·Resources·저작 JSON·generated runtime을 변경하지 않았다. 기존 사용자 변경도 보존했다.
- 설계 및 이 조사 결과 문서만 추가했다. `out/mario_asset_contact_sheet.py/.png`는 Git 제외 진단 산출물이다.
- 조사 대상 JSON을 read-only 파싱했다. 변경 JSON/XML은 없다.
- `git diff --check`는 오류 없이 끝났다. 기존 사용자 파일의 LF/CRLF 및 전역 ignore 접근 경고는 별도이며 새 구현의 컴파일 성공을 뜻하지 않는다.
- 빌드, publisher, Client/UI 실행·캡처, 실제 배치·공격·재입장 테스트는 수행하지 않았다.
- 신규 리소스 추출·쿠킹·Drive 업로드·Git stage/commit/push는 수행하지 않았다.
- 독립 설계 검토 후 실제 publisher/서버 코드를 재확인해 두 항목을 보완했다. 적 삐에로의 배치 정본은 기존 SpawnGroups로 유지하고 MapTool에서 통합 표시한다. 개인 stage 종료는 참가자만 제거하며 다른 참가자가 남은 공유 run의 객체를 정리하지 않는다.

## 남은 실제 구현

정확한 외형 연결 확정, 모델 없는 Effect 표현 정의, MapTool stage 배치 편집/저장, 서버 run 수명과 snapshot 연결, 마리오 공격·높이별 판정, 다인/재입장 검증이다. 설계 파일에 적었다는 이유로 이 항목을 완료 처리하지 않는다.
