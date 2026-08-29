# LostArk 팀 통합 문서 입구

이 폴더는 팀원이 pull 후 가장 먼저 보는 살아 있는 public 계약의 정본이다. 날짜별 PLAN/RESULT는 구현 당시의 증거이므로 `.md/GB/<MM-DD>/`에 그대로 보존하고, 여기서 현재 유효한 문서만 연결한다.

## 2026-09-30까지 팀 LAN 세션 시작

모든 팀원과 에이전트는 pull 후 다른 작업보다 먼저 아래 명령을 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

endpoint와 만료일 정본은 `../../Tools/Network/TeamLanEndpoint.json`, 실행·실패 진단 정본은
`TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 `서로 다른 장소에서 Server와 Client 연결`이다.
빠른 IP 교체, 실제 4인 LAN, Character Select loopback 독립 테스트는
[네트워크연결가이드.md](네트워크연결가이드.md)의 실행 순서를 따른다.
현재 공유 Server endpoint는 같은 팀 LAN의 `10.207.18.103:7777`이다.
현재 Server가 꺼져 있으면 `not-listening`이 정상일 수 있으며, 스크립트가 로컬 debugger 설정을
동기화한 뒤 출력이 `server-host`이면 Visual Studio의 `Server + Client` profile, `client`이면
Client project를 사용자가 `Ctrl+F5`로 시작할 대상으로 안내한다.

## 모든 세션의 사용자 전용 화면 검증 경계

- Artist F, Character Select와 모든 Client Effect 시각 결과는 사용자가 직접 조작하고 최종 visual fidelity를 판정한다.
- 에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고 화면 캡처·스크린샷 생성을 하지 않으며, visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지 분석을 요청하면 에이전트는 반드시 열람·분석하고 관찰 결과와 가능한 결함 위치를 보고한다.
- 에이전트는 빌드, 구조화된 로그와 수치 진단, 실행 준비까지만 수행한 뒤 사용자가 직접 누를 경로를 전달하고 멈춘다.
- 사용자의 서면 판정 전에는 first pixel, eye smoke, visual PASS, occurrence 승인을 완료로 기록하지 않는다.
- 완성·복원·시각 검증 요청 자체는 Client/UI 자율 실행·조작이나 화면 캡처 권한이 아니다. 첨부 이미지 분석은 최종 육안 판정이나 단독 완료 증거가 아니다.

## 읽는 순서

1. 저장소 금지 경계와 완료 조건: `../../AGENTS.md`
2. 최초 세팅, 빌드, 런타임 구조: `../../CLAUDE.md`
3. merge·실행·화면 검증 금지 경계: `../GB/gotchas.md`, 있으면 `../GB/gotchas.local.md`
4. 현재 작업의 대응 PLAN/RESULT
5. 통합 데이터 수명·stable ID·publish 구조: `UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`
6. 담당별 입력·출력 계약: `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
7. F1 Balance Tool과 공식 provenance 작업법: `BALANCE_TOOL_OWNER_HANDOFF.md`
8. Animation/Effect/Character Preview Tool 경계: `ANIMATION_TOOL_OWNER_HANDOFF.md`
9. 발탄 gameplay/presentation split 정본과 joined revision: `발탄인수인계서.md`
10. Effect family/runtime ABI 복원 경계: `EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md`
11. Area별 데이터 레이어와 확장 경계: `AREA_DATA_LAYER_GUIDE.md`
12. Map Destruction PhysX·Mesh Debris 작업법: `MAP_DESTRUCTION_PHYSX_HANDOFF.md`
13. NPC 배치·상호작용 작업법: `NPC_OWNER_HANDOFF.md`
14. 밸런스 Hot Reload 경계: `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`
15. 최근 Valtan·Bern·Party 통합 검증 증거: `../GB/08-28/2026-08-28_VALTAN_BERN_PARTY_INTEGRATION_RESULT.md`

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
| `발탄인수인계서.md` | 발탄 gameplay/presentation split, joined revision, animation/Effect invocation 또는 담당별 튜닝 경계가 바뀔 때 |
| `EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md` | Effect element→pixel 구조, family/program/descriptor/adapter 경계와 source-exact admission 공정이 바뀔 때 |
| `AREA_DATA_LAYER_GUIDE.md` | Area 등록, optional layer, publisher, MapTool 지원 범위가 바뀔 때 |
| `MAP_DESTRUCTION_PHYSX_HANDOFF.md` | MapTool 파괴 preview의 Mesh Emitter, PhysX, trigger/effect 연결 절차가 바뀔 때 |
| `NPC_OWNER_HANDOFF.md` | NPC catalog/placement/publish/runtime 계층, 쿠킹 절차, 상호작용 계약이 바뀔 때 |
| `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md` | 수치 정본, 튜닝 절차, runtime reload 정책이 바뀔 때 |
| `네트워크연결가이드.md` | endpoint 교체, 4인 LAN, Character Select loopback 격리 테스트 실행 절차가 바뀔 때 |
| `Tools/Network/TeamLanEndpoint.json` | 임시 팀 Server 주소나 만료일이 바뀔 때 |
| 날짜별 `*_RESULT.md` | 실행한 검증과 당시 완료/미완료 증거를 남길 때 |

같은 설명을 여러 문서에 복제하지 않는다. 이 README는 입구와 문서 역할만 소유하고, 세부 계약은 연결된 정본에서 읽는다.
