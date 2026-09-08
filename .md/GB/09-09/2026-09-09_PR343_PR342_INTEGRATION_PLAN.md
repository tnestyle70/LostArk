# PR #343·#342 병합 계획

사용자 요청: 두 PR을 main의 카드미로 변경과 함께 충돌 없이 병합하고, Effect Tool V1과 V2는 각각 분리해서 사용한다.

## G1. #343과 main 통합

- 별도 worktree에서 사용자 작업 브랜치와 실행 중인 Client/Server를 보존한다.
- 카메라·World Sequence는 stable ID와 실제 변경 필드로 병합한다. patternbindings/encounter는 정본 KoukuSaydonComposition revision 176에서 재생성한다. 생성 데이터는 기존 publisher로 재생성한다.
- main의 카드미로 snapshot과 #343의 FINISH_OWNER를 모두 보존하고, 양쪽 기존 peer와 구분되는 protocol 72로 통합한다.
- 원본 리소스, 복원 데이터, 셰이더 및 열기/저장 성능 개선을 보존한다.

## G2. #342 UI와 V2 저작 기능 통합

- #343 병합 뒤 #342를 갱신한다. V1/V2 편집 화면과 live boss attach를 각각 유지하고 레벨 종료 프레임 정리를 중복 없이 연결한다.
- V2 authored/group/binding 변경과 현재 쿠크 배치·카메라 변경을 필드별로 확인한다.

## G3. 검증 및 병합

- 변경 JSON/XML parse, 충돌 표식·unmerged 항목 없음, diff 공백 검사 및 해당 publisher/계약 검사를 수행한다.
- 별도 빌드 경로에서 변경된 Client/Server/Shared를 컴파일하고 필요한 프로토콜 검사를 실행한다. Client/UI는 실행하지 않는다.
- 검증한 커밋을 각 PR 기존 head에 non-force push하고 해당 head를 확인하여 순서대로 병합한다. 실제 결과와 남은 사용자 화면 확인은 RESULT에 기록한다.
