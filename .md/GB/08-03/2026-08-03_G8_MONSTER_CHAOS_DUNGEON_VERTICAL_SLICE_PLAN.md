# G8 Monster/Chaos 계획 — 폐기됨

상태: **SUPERSEDED / DO NOT IMPLEMENT**
폐기일: 2026-08-03

이 파일에 있던 수업용 `CMonster` 전제와 Monster/Chaos 수직 구현안은 현재 통합 정본에서 제외됐다. 빈 Monster catalog, placeholder enum, 임시 presentation adapter도 만들지 않는다.

현재 World Gameplay가 지원하는 actor kind는 다음뿐이다.

- `playerSpawn`
- `npc`
- `boss`

향후 Monster 기능이 실제로 필요하면 이 문서를 재개하지 않는다. 새 요구를 바탕으로 서버 상태 소유권, stable archetype ID, Client presentation, MapTool schema, protocol, rollback, 전용 하네스를 다시 조사한 뒤 별도 계획을 작성한다.

통합 정본은 다음 문서를 따른다.

- `AGENTS.md`
- `CLAUDE.md`
- `.md/GB/08-02/2026-08-02_LOSTARK_UNIFIED_WORLD_DATA_ASSET_IMGUI_SERVER_PLAN.md`의 0절
- `.md/GB/08-03/2026-08-03_LOSTARK_UNIFIED_FRAMEWORK_HARNESS_RESULT.md`
