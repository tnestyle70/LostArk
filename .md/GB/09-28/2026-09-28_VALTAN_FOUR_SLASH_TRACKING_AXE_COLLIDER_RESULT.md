# 발탄 4연속·추적 도끼 타격 판정 결과

## G01. 완료된 Source 및 Server 변경

현재 저장본에 optional `hit.contacts` 7개를 적용했다. 기존 stage 길이, 순서·분기, Animation, Sound, Effect cue와 개별 추적 도끼는 보존했다. Source install 이전 raw byte/hash는 `out/ValtanContacts20260928/before/`와 `baseline.json`, 적용 후 hash는 `data-install.json`에 있다.

| Pattern / Stage | Stage 상대 시각(ms) | 실제 접촉 도형 | 피해·반응 |
|---|---|---|---|
| FOUR_SLASH / SLASHES | 1667 / 2221 / 3008 | CONE 110도, 9m | 최대 HP 10% |
| FOUR_SLASH / SPIN | 600 / 1640 | 기존 전방 접촉 + 네 번째 검격 CONE 110도, 9m | 최대 HP 10% |
| FOUR_SLASH / SPIN | 3000 | RING 내반경4.8m / 외반경10.8m | 최대 HP 10% |
| HIGH_JUMP / LAND | 201 | CIRCLE 반경8.75m | 최대 HP 50% |

모든 새 contact는 쿠크 노란 원·도넛 `KAKULSAYDON_G1_PATTERN_119 / logic137`과 같은 forcePush=true, 수평1m, 상승2m, 1200ms를 사용한다. percentage는 defense 적용 전 최대 HP 기준 raw damage이며 기존 shield, invulnerability, damage-taken buff와 피해 event 경로는 계속 적용된다. 공유 `damage.valtan.high-jump`는 500%로 그대로 두어 개별 추적 도끼를 50%로 바꾸지 않았다.

시각은 현재 저장 Effect element에서 읽었다. LAND는 notify2240ms − playbackOffset2194ms + cue155ms =201ms다. 도넛 치수는 설치 `fm_d_ring_008.wmodel` XZ 반경40..79.88996 × modelPreScale0.01 × mesh StartSize8/9 × cue1.5의 두 ring union을 gameplay 4.8..10.8m로 잡았다. 이 수치는 particle alpha의 정확한 GPU coverage를 뜻하지 않는다. 큰 빨간 원형은 저장 size17.5m의 절반8.75m다. 실측 근거는 `footprint.py`, `footprint.txt`와 기존 `out/ValtanTrackingAxe20260928/REPORT.md`다.

## G02. 저장·게시·실행 계약

Shared `Validate_StageAttackContacts`는 existing pulse schedule과 count/order/atMs, bounded single TIMED contact를 함께 검증한다. ACTIVE_WINDOW·CAPTURE·Stage motion과 조합하면 거절한다. Python source strict join은 기존 Kouku hit validator를 재사용하고 source `contacts`를 Product `attackContacts`로 투영한다.

PowerShell은 기존 `New-KoukuAttackHitRows`로 `PATTERNATTACKHIT/STAGE` row를 만들며 action ID로 Stage에 연결한다. `Get-BootstrapRowSortKey`의 STAGE 역할은 `PATTERNSTAGECONTACT` sort key를 사용해 Stage owner 뒤에 배치한다. emit row 종류는 바꾸지 않는다. 이를 놓치면 공식 정렬 결과가 child를 parent보다 먼저 로드하므로 실제 전체 row sort 후 검증했다.

Server `GameplayCatalog`는 Stage contacts를 읽고 최종 pulse·authority 검증을 거친다. `ValtanBrain`의 기존 hit-count loop가 해당 index의 geometry/damage/push를 `ApplyPatternHit -> ServerCombatHitRuntime`로 보낸다. contact용 별도 timer나 추가 damage loop는 없다. 원점은 기존 Stage hit anchor, contact offset/yaw는 그 basis 뒤에 한 번 적용한다. guide의 contact risk도 개별 geometry를 읽는다.

기존 G14 Sound receipt는 SPIN/LAND의 의도적으로 변경된 hit offset만 갱신했다. SLASHES는 저장 Sound1700/2200/3000ms와 새 contact1667/2221/3008ms가 각각33/21/8ms로 맞아 예외 자체가 불필요해져 한 행을 제거했다. Sound와 Animation payload는 변경하지 않았다. 기존 receipt의 rate/source-start/prefix/extra-occurrence 차단 회귀는 여전히 예외가 필요한 SPIN으로 옮겨 검증했다.

## G03. Client 저장 및 Collider 표시

Client 담당자가 `ValtanPatternTree`, `EncounterPatternReference`, `BalanceTool`, `Valtan`의 header/implementation 8개를 연결했다. 실제 source→master/Product→typed view→Balance draft→SET_STAGE_HIT 저장에서 contact를 보존하고, malformed contact replacement는 기존 vector를 유지한다. 공용 단일 Collider 편집은 contact owner를 읽기 전용으로 두어 timing/shape/damage를 조용히 덮어쓰지 않는다. 재저장 자체는 허용한다.

BOSS_CURRENT mirror는 각 contact의 pulse clock/shape/offset/yaw를 사용한다. STAGE_ORIGIN의 Server 원점은 현재 snapshot에 없으므로 그 contact wire를 추측해 그리지 않는다. 이번7개는 전부 BOSS_CURRENT/default0 offset이라 이 경계의 영향을 받지 않는다. 실제 화면/GPU 확인은 사용자 몫이다.

## G04. 실행한 검증

- source strict join 및 actual Product projection PASS, 7contacts/191legacy stage 보존.
- `test_valtan_stage_contacts.py` 6tests PASS: malformed field, shape, time, repeat, schedule, empty, lifetime, activation/motion, legacy, Product import.
- hit/presentation alignment 기존15tests PASS. read-only validator PASS:474 stage hits,96 individually aligned,4 exact Sound receipts.
- actual PowerShell helper7rows 생성, actual full bootstrap sorter108888rows 정렬 후 actual CGameplayCatalog로 로드 PASS.
- Server fixture는 변경 header ABI를 쓰는15TU를 새로 컴파일했다. fixture는 actual `ValtanBrain.cpp`를 include하여 ApplyPatternHit와 CValtanBrain::Update를 모두 실행했다. 도넛 hole/band/외부와 body-radius 경계,10%/50%, forcePush 및 ballistic, 명시forcePush=false의 grace 보존, invulnerability, offset, legacy, target-axe500%, due clock, 동일/다음 tick 중복0, 실제 SPIN 두 cone→ring 및 guide risk 포함22checks PASS.
- LAND201ms는30Hz server tick에서200ms까지 피해0,233.333ms tick에서한번50%를 적용한다. stage timer의 양자화이며 ms clock을 바꾸지 않는다.
- Client 실제 projection/codec/draft/writer와 최신 Product Load fixture527checks PASS, 실제 Draw_PatternHitAreaDebug fixture15checks PASS,4CPP 최소 컴파일 PASS.
- 새·변경 JSON parse, Python compile, scoped `git diff --check` PASS.

재현 증거 경로는 모두 저장소 상대 경로다.

| 증거 | 경로 |
|---|---|
| Server exact delta / raw baseline | `out/ValtanContacts20260928/actual-delta.diff`, `before/`, `baseline.json` |
| 적용/시각·범위 | `out/ValtanContacts20260928/data-install.json`, `contacts.json`, `footprint.txt` |
| Python21tests/alignment | `out/ValtanContacts20260928/contacts-tests.log`, `alignment-tests.log`, `alignment-validation.log` |
| Publisher inputs/helper/sort | `out/ValtanContacts20260928/contact-row-inputs.json`, `publisher_probe.ps1`, `contact-rows.txt`, `sort_fixture.ps1` |
| Actual Server fixture | `out/ValtanContacts20260928/runtime_probe.cpp`, `build_runtime_probe.py`, `runtime_probe.log`, `Gameplay.bootstrap`, `bootstrap.sha256`, `*.compile.log` |
| Client actual slice | `out/ValtanContactsClient20260928/actual-delta.diff`, `probe.log`, `mirror.log`, `compile.log`, `probe.generated.cpp` |

Client/UI 또는 Server 프로세스는 실행하지 않았다. console native fixture만 실행했다. 최종 공식 domain publish와 Release/Debug 전체 빌드, PR 검토/merge는 root 통합 작업이 소유하므로 이 결과로 런타임 게시까지 완료했다고 주장하지 않는다.
