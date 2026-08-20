# 2026-08-21 DimensionMaster BA command buffer implementation result

## 1. 이번 검증 단위

- 차원술사 기본 공격 `2050010`의 BA1 4초 회귀를 source timing인 1400ms로 복구했다.
- BA 연계 입력과 명시 MOVE/SKILL의 실행 시점을 분리하고, action 중 폐기되던 다음 command를 Server 권위 pending intent로 보존했다.
- `comboAdvanceMs`를 데이터, publisher/bootstrap, Server runtime, Balance Tool과 harness까지 한 수직 슬라이스로 연결했다.
- Client나 UI는 실행하지 않았으며 실제 조작감과 animation/Effect의 육안 결과는 사용자 판정 경계로 남긴다.

## 2. 구현 결과

### 2.1 BA timing과 presentation 복구

- 차원술사 BA1~BA4를 각각 `1400/1500/1067/1700ms`로 확정하고 BA1 hit/window를 source 계약의 `100ms`, `100..510ms`로 복구했다.
- 차원술사 네 stage의 `comboAdvanceMs`는 각 animation duration과 같게 두어 한 stage animation이 한 번 끝나기 전에 다음 BA로 넘어가지 않도록 했다.
- BA1 skill binding에 `playMs: 1400`을 명시하고 root-motion stage 0을 exact 1400ms sample에서 잘랐다.
- provenance receipt는 공식 update script로 동기화했으며 생성 bootstrap은 직접 편집하지 않았다.

### 2.2 `comboAdvanceMs` 제품 계약

- 모든 COMBO stage에 `comboAdvanceMs`를 필수 필드로 추가했다. parser fallback은 두지 않았다.
- publisher와 Server catalog는 `hitTimeMs <= comboAdvanceMs <= durationMs`와 마지막 caster 반복 hit/마지막 projectile spawn 이후라는 조건을 함께 검증한다.
- Server runtime도 모든 필수 caster hit와 projectile spawn이 끝난 뒤에만 buffered BA continuation을 commit한다.
- 기존 class의 반복 타격을 보존하기 위해 Warlord `17000` stage 1은 600ms, LanceMaster `34140` stage 0은 601ms, `34160` stage 0은 1350ms로 이관했다.

### 2.3 release와 다음 command 보존

- Client는 LMB down에서 catalog로 resolve한 BA skill ID를 보존하고 physical up edge에서 기존 release packet을 전송한다.
- Server는 newer-only release sequence를 소비하며 같은 현재 COMBO skill의 아직 commit되지 않은 continuation만 취소한다. 현재 stage animation은 자르지 않는다.
- COMBO action 중 MOVE/SKILL은 값 기반 tagged pending intent 하나로 보존하며 최신 명시 command가 이전 명시 command를 교체한다.
- pending 명시 command가 있으면 이른 BA continuation을 막고 현재 stage의 `actionDurationMs`에서 재검증한 뒤 실행한다.
- pending MOVE는 그 시점의 최종 Server 위치에서 navigation을 다시 계산하고, pending SKILL은 cooldown/resource/class/stance/aim을 다시 확인한 뒤 비용을 commit한다.
- death, class change, revive, fall/forced movement, hit reaction, knockdown, trigger action replacement, audition reset과 world transfer에서 pending을 제거한다.

### 2.4 LMB resend와 Balance Tool

- accepted MOVE 또는 다른 quick-slot skill 뒤에는 physical LMB release까지 100ms BA resend를 막는다. 첫 BA와 정상 hold continuation은 막지 않으며 up/repress 뒤에는 다시 허용한다.
- Balance Tool은 현재 다섯 authoring 문서의 실제 schema를 parse/validate/save한다. player identity, skill `identityCost`, 모든 skill kind, `comboAdvanceMs`, encounter `introPatternId`와 optional `serverMotion`을 보존한다.
- 편집 실수는 `double`로 소유하고 JSON 숫자는 round-trip 가능한 shortest representation으로 직렬화해 no-op Save의 float 축소를 막았다.
- Collider 종류를 바꾸면 사용하지 않는 geometry를 0으로 만들고, damage/shape가 사라지면 push/knockdown 필드를 함께 정규화한다. push range/pushMs와 knockdown/downMs pair는 publisher와 같은 규칙을 UI에서 유지한다.
- Save와 같은 serializer를 disk write 없이 호출하는 semantic round-trip harness로 `6 players / 94 skills / 108 damage profiles / 1 boss / 33 patterns`의 field coverage와 double 값 보존을 고정했다.

### 2.5 gameplay bootstrap v12

- `SKILLSTAGE`가 `comboAdvanceMs`를 포함한 8-field row가 되었으므로 gameplay bootstrap header를 v11에서 v12로 올렸다.
- publisher는 v12만 생성하고 Server parser는 v12만 수락한다. synthetic fixture도 v12로 맞추고 이전 v11은 stale-format 음성 fixture로 고정했다.

## 3. 자동 검증

- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
  - 6 player profiles, 136 skill/stage rows, 108 damage profiles, 1 boss, 33 patterns/124 stages와 67 preview occurrences
- `Publish-BalanceRuntimeSet.ps1 -Mode Validate`: PASS
- `ClientFrontendHarness.exe --balance-authoring-fast`: PASS, failures 0
- `ClientFrontendHarness.exe --player-command-buffer-fast`: PASS, 4/4, failures 0
- x64 Debug Server isolated-output full compile/link: PASS
- Server `--contract-test`: 이번 BA/DM/pending/multi-hit 검사는 모두 PASS
  - 전체 실행에는 선행 baseline인 `Accept one exact ACTIVE axe wall-contact row` 1건만 남아 exit code 1이었다.
- x64 Debug Client isolated-output full compile/link: PASS
- JSON/PowerShell parse 및 이번 범위 `git diff --check`: PASS

## 4. 남은 수동/통합 경계

- 실행 중인 기존 Client/Server process는 종료하거나 교체하지 않았다. 현재 process에는 이번 binary/data가 아직 반영되지 않았다.
- 사용자는 새 Server/Client를 시작한 뒤 차원술사 BA tap, hold, release 후 RMB, BA 중 quick-slot 입력을 직접 확인한다.
- BA1 1400ms trim에서 root motion 또는 Effect가 튀거나 끊기는지는 사용자 visual/조작감 판정이 필요하다.
- 저장소 전체 회귀의 선행 Server contract, Effect corpus와 protocol version fixture 실패는 이번 BA 계약과 분리해 최종 통합 결과에 기록한다.
