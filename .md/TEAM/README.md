# LostArk 팀 통합 문서 입구

이 폴더는 팀원이 pull 후 가장 먼저 보는 살아 있는 public 계약의 정본이다. 날짜별 PLAN/RESULT는 구현 당시의 증거이므로 `.md/GB/<MM-DD>/`에 그대로 보존하고, 여기서 현재 유효한 문서만 연결한다.

## 2026-08-20까지 팀 LAN 세션 시작

모든 팀원과 에이전트는 pull 후 다른 작업보다 먼저 아래 명령을 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

endpoint와 만료일 정본은 `../../Tools/Network/TeamLanEndpoint.json`, 실행·실패 진단 정본은
`TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 `서로 다른 장소에서 Server와 Client 연결`이다.
현재 Server가 꺼져 있으면 `not-listening`이 정상일 수 있으며, 스크립트가 로컬 debugger 설정을
동기화한 뒤 출력이 `server-host`이면 Visual Studio의 `Server + Client` profile, `client`이면
Client project만 `Ctrl+F5`로 시작한다.

## 읽는 순서

1. 저장소 금지 경계와 완료 조건: `../../AGENTS.md`
2. 최초 세팅, 빌드, 런타임 구조: `../../CLAUDE.md`
3. 통합 데이터 수명·stable ID·publish 구조: `UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`
4. 담당별 입력·출력 계약: `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
5. F1 Balance Tool과 공식 provenance 작업법: `BALANCE_TOOL_OWNER_HANDOFF.md`
6. Animation/Effect/Character Preview Tool 경계: `ANIMATION_TOOL_OWNER_HANDOFF.md`
7. Area별 데이터 레이어와 확장 경계: `AREA_DATA_LAYER_GUIDE.md`
8. Map Destruction PhysX·Mesh Debris 작업법: `MAP_DESTRUCTION_PHYSX_HANDOFF.md`
9. 밸런스 Hot Reload 경계: `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`
10. 최근 통합 검증 증거: `../GB/08-07/2026-08-07_VALTAN_WORLD_DESTRUCTION_RESULT.md`

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

담당 표는 배타적 파일 소유권 목록이 아니다. 팀원이나 AI agent는 자기 기능의 시작 인터페이스를
찾은 뒤, 그 기능에 필요한 Data/Shared/Server/Client/UI/harness를 수직으로 연결해 완료한다.
세부 금지 경계는 계층 우회를 막기 위한 것이며, Server 반영이 필요한 기능에서 Server 파일 수정을
막기 위한 규칙이 아니다.

## 문서 소유권

| 문서 | 바꿀 때 |
|---|---|
| `AGENTS.md` | 팀 전체 금지 규칙이나 완료 조건이 바뀔 때 |
| `CLAUDE.md` | 경로, 빌드, 최초 세팅, 런타임 사용법이 바뀔 때 |
| `UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md` | 데이터 domain 소유권, stable ID graph, authoring/publish/runtime 수명이나 확장 순서가 바뀔 때 |
| `BALANCE_TOOL_OWNER_HANDOFF.md` | 공식 receipt, F1 Balance Tool, Server 적용·검증 절차가 바뀔 때 |
| `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 담당자가 소비하는 C++/데이터 public 계약이 바뀔 때 |
| `ANIMATION_TOOL_OWNER_HANDOFF.md` | Animation/Effect/Character Preview Tool의 authoring 소유권이 바뀔 때 |
| `AREA_DATA_LAYER_GUIDE.md` | Area 등록, optional layer, publisher, MapTool 지원 범위가 바뀔 때 |
| `MAP_DESTRUCTION_PHYSX_HANDOFF.md` | MapTool 파괴 preview의 Mesh Emitter, PhysX, trigger/effect 연결 절차가 바뀔 때 |
| `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md` | 수치 정본, 튜닝 절차, runtime reload 정책이 바뀔 때 |
| `Tools/Network/TeamLanEndpoint.json` | 임시 팀 Server 주소나 만료일이 바뀔 때 |
| 날짜별 `*_RESULT.md` | 실행한 검증과 당시 완료/미완료 증거를 남길 때 |

같은 설명을 여러 문서에 복제하지 않는다. 이 README는 입구와 문서 역할만 소유하고, 세부 계약은 연결된 정본에서 읽는다.
