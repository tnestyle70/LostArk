# Valtan Effect corpus 전수조사와 ProjectAudit 퇴역 구현 계획

- 작성일: 2026-08-13
- 작업 브랜치: `codex/artist-f-complete-visual-runtime`
- 사용자 결정: 전역 `ProjectAudit`과 그 JSON report 계약은 제거한다.
- 보존 경계: 실제 runtime/publisher가 소비하는 JSON schema/version, Effect 원본 증거 receipt, 실행형 harness는 제거 대상이 아니다.

## 1. 목표

1. `ValtanEncounter.json`의 31개 pattern과 63개 source Action이 참조하는 원본 Effect 재료를 전수 매핑한다.
2. ParticleSystem graph, material, mesh, texture가 실제로 존재하는 경우와 pattern별 완제품으로 조립된 경우를 구분한다.
3. `PlayDecalEffect`, generic `Effect`, Trail/TrailGhost, 돌 생성과 arena mutation처럼 아직 별도 연결이 필요한 항목을 분리한다.
4. 빌드 뒤 저장소 전체를 다시 훑는 `ProjectAudit` aggregate runner와 JSON report를 제거한다.
5. 빌드·publisher validation·Network/Client harness·Server contract test처럼 실제 소비자를 검증하는 경로만 남긴다.

## 2. 현재 실측

- Valtan Server encounter: 31 pattern, 115 stage, 63 source Action.
- 원본 Action 문서: Action 170, stage 2,464, animation clip occurrence 2,378, notify 21,931.
- 명시 ParticleSystem: 193/193 graph 해소.
- resource closure: material binding 335, parent 123, effect mesh 52, texture 346.
- runtime resource: `Client/Bin/Resources/Effect/Valtan`에 WModel 52, DDS 346 존재.
- 아직 완제품 EffectCatalog entry: 0.
- 미해결 notify: generic Effect 3,787, PlayDecalEffect 536, DefaultParticle 133.
- ProjectAudit: 39 files, 12,820 lines, aggregate 121 checks. 실제 build caller는 `Invoke-BuildAndRegression.ps1` 한 곳이다.

## 3. 구현 단계

### G00. Valtan source corpus matrix

- pattern별 `sourceActionIds`를 원본 Action 문서와 join한다.
- 명시 ParticleSystem, clip, material/mesh/texture, renderer feature를 기록한다.
- `EXACT_RESOURCE_PRESENT`, `SOURCE_NOTIFY_UNRESOLVED`, `PATTERN_STAGE_MAPPING_OPEN`, `WORLD_MECHANIC_REQUIRED`를 구분한다.

종료 증거:

- 31/31 pattern에 source Action 및 명시 ParticleSystem 후보가 집계된다.
- 돌 mesh와 stone ParticleSystem의 실제 Resources 경로가 확인된다.

### G01. 조립 및 mechanic 경계

- Particle 재료 존재를 Product Effect 완성으로 오인하지 않는다.
- pattern stage/branch → clip/notify → EffectAsset 연결이 없는 항목을 OPEN으로 남긴다.
- 돌 생성 particle mesh와 충돌 가능한 gameplay/world stone actor를 구분한다.
- arena 80/33 파괴, ghost actor, grab/counter/stagger branch는 별도 Server-authoritative slice로 분리한다.

종료 증거:

- 사용자가 요청한 점프·도끼·다방향 돌·회전 계열의 실제 source system과 남은 blocker가 표로 정리된다.

### G02. ProjectAudit aggregate 퇴역

- `Invoke-ProjectAudit.ps1`과 ProjectAudit JSON report 생성을 제거한다.
- `Invoke-BuildAndRegression.ps1`에서 aggregate 호출과 report directory 관리를 제거한다.
- stale tracked ProjectAudit report를 제거한다.
- 정본 문서에서 ProjectAudit 필수 계약을 실제 publisher/harness 검증으로 교체한다.

종료 증거:

- build/regression이 더 이상 `Invoke-ProjectAudit.ps1` 또는 `ProjectAudit.json`을 생성·호출하지 않는다.
- Solution/project/runtime에는 ProjectAudit 의존성이 없다.

### G03. 실제 소비자 검증 보존

- `Publish-BalanceRuntimeSet.ps1 -Mode Validate`
- `Publish-ServerNavigation.ps1 -Mode Validate`
- `Publish-Effects.ps1 -Mode Validate`
- `Sync-EffectDataProject.ps1 -Check`
- `Publish-RenderingProfiles.ps1 -Mode Validate`
- NetworkProtocolHarness, ClientFrontendHarness, Artist WARP first draw, Server contract test

JSON의 `schema`와 `formatVersion`은 Git resource version 관리가 아니라 runtime parser의 입력 형식 계약이므로 보존한다. Artist의 MIC/ShaderMap/DXBC/texture SHA receipt도 원본 일치 증거이므로 보존한다.

## 4. 실패 및 rollback

- 다른 G 세션이 수정 중인 ProjectAudit wrapper는 underlying Python/unit/receipt 결과를 먼저 보존한다.
- 사용자의 Warlord animevents 및 Artist F dirty 변경은 이 작업에서 되돌리거나 stage하지 않는다.
- ProjectAudit 제거 뒤 publisher 또는 executable harness가 실패하면 해당 domain 실패로 보고하며 전역 audit를 다시 만들지 않는다.
- historical PLAN/RESULT의 과거 ProjectAudit PASS 기록은 역사적 사실이므로 일괄 수정하지 않는다.

## 5. 최종 검증

1. 변경된 PowerShell parse
2. JSON/XML parse
3. 위 다섯 domain validator
4. 관련 focused Effect unit/harness
5. `Invoke-BuildAndRegression.ps1 -Configuration Debug` 또는 `-SkipBuild` 재검증
6. `git diff --check`

## 6. 실행 결과

- G00/G01 Valtan corpus matrix와 mechanic 경계: 완료
- G02 ProjectAudit aggregate/report 퇴역: 완료
- G03 domain publisher 직접 검증: PASS
- Debug `-SkipBuild` 통합 실행: ClientFrontendHarness가 244초 제한 안에 종료되지 않아 미완료
- 상세 증거: `2026-08-13_VALTAN_EFFECT_CORPUS_AND_PROJECT_AUDIT_RETIREMENT_RESULT.md`
