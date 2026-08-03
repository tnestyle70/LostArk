# LostArk 팀 통합 문서 입구

이 폴더는 팀원이 pull 후 가장 먼저 보는 살아 있는 public 계약의 정본이다. 날짜별 PLAN/RESULT는 구현 당시의 증거이므로 `.md/GB/<MM-DD>/`에 그대로 보존하고, 여기서 현재 유효한 문서만 연결한다.

## 읽는 순서

1. 저장소 금지 경계와 완료 조건: `../../AGENTS.md`
2. 최초 세팅, 빌드, 런타임 구조: `../../CLAUDE.md`
3. 담당별 입력·출력 계약: `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
4. Area별 데이터 레이어와 확장 경계: `AREA_DATA_LAYER_GUIDE.md`
5. 밸런스 튜닝과 Hot Reload 경계: `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`
6. 최근 통합 검증 증거: `../GB/08-03/2026-08-03_LOSTARK_UNIFIED_FRAMEWORK_HARNESS_RESULT.md`

## 문서 소유권

| 문서 | 바꿀 때 |
|---|---|
| `AGENTS.md` | 팀 전체 금지 규칙이나 완료 조건이 바뀔 때 |
| `CLAUDE.md` | 경로, 빌드, 최초 세팅, 런타임 사용법이 바뀔 때 |
| `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 담당자가 소비하는 C++/데이터 public 계약이 바뀔 때 |
| `AREA_DATA_LAYER_GUIDE.md` | Area 등록, optional layer, publisher, MapTool 지원 범위가 바뀔 때 |
| `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md` | 수치 정본, 튜닝 절차, runtime reload 정책이 바뀔 때 |
| 날짜별 `*_RESULT.md` | 실행한 검증과 당시 완료/미완료 증거를 남길 때 |

같은 설명을 여러 문서에 복제하지 않는다. 이 README는 입구와 문서 역할만 소유하고, 세부 계약은 연결된 정본에서 읽는다.
