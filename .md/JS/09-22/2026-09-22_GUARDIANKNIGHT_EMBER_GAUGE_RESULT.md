# 가디언나이트 엠버레스 오브 게이지·기운·소켓 — RESULT

PLAN: [2026-09-22_GUARDIANKNIGHT_EMBER_GAUGE_PLAN.md](2026-09-22_GUARDIANKNIGHT_EMBER_GAUGE_PLAN.md)

## 1. 구현 완료

| 층 | 파일 | 내용 |
|---|---|---|
| Data | `Data/Balance/GuardianKnightEmber.json` (신규) | orbGauge gainPerHit 4 / dragonDurationMs 15000, ember maximumSockets 10 / damageBonusPercentPerOrb 10, 스킬 12행(Q/W/E/R/D 회복 2/3/3/4/6, 발현·화신 소모 4/4/5/6, 일반형 A/S/F 소켓 잠금) |
| Data | `PlayerProfiles.json` | GUARDIANKNIGHT `maximumIdentity 0 → 100`. receipt는 `Update-BalanceProvenanceReceipt.ps1`로 1필드 PROJECT_TUNED 동기화 |
| Publisher | `Publish-GameplayBalance.ps1` | 문서 strict 검증 후 `SKILLEMBER <id> <gain> <cost> <lock>` 12행, `PLAYEREMBER <class> 4 15000 10 10` 1행. "identity never spends" 검사에서 ember class 제외 |
| Server catalog | `GameplayCatalog.h/.cpp` | `GUARDIAN_EMBER_PROFILE`, `Find_EmberProfile`, skill `iEmberGain/iEmberCost/locksEmberSocket`, 두 행 파서와 post-load 교차 검사 |
| Server state | `ServerPlayer.h` | `iEmberOrbs / iEmberLockedSockets / iEmberSpentOnAction` |
| Server 판정 | `PlayerSkillSystem.h/.cpp` | `Reset_Gauges`(ember class는 게이지 0·기운 10·잠금 0), `Apply_EmberOnStart`(소모 min(보유,비용)·잠금·회복 clamp), `Gain_EmberGauge`(HUMAN 상태 히트당 +4), `Update_EmberGauge`(DRAGON에서 100→0을 정확히 450tick), `Commit_StanceChange`(DRAGON 진입 시 잠금 해제·기운 만충, HUMAN 복귀 시 게이지 0), Z 화신화는 게이지 만충에서만 admission, `resolveRawDamage`에 소모 개수당 +10% |
| Server reset | `GameRoom_Admission/PlayerCommands/GateProgress/KoukuRaidFlow.cpp` | `iCurrentIdentity = max` 두 줄을 `Reset_Gauges` 호출로 교체 (`GameRoom_Helpers.cpp`의 Debug audition `Prepare_TimelineAuditionPlayer`는 catalog가 없어 그대로) |
| Shared | `PacketMessages.h/.cpp`, `PacketType.h` | `PLAYER_SNAPSHOT` U8×3 `iEmberOrbs/iEmberLockedSockets/iEmberMaximumSockets`, validator `orbs+locked ≤ max`, **protocol 98 → 99** |
| Server 복제 | `GameRoom_Replication.cpp` | ember profile이 있는 class만 세 값 채움 |
| Client | `CombatHUDViewModel.h/.cpp`, `MainApp.cpp` | HUD 상태 3필드; ember class는 `ManaBar_Fill` 숨김, 마나 텍스트 자리에 `오브 xx%  기운 n / 열린소켓` readout (GK HUD 아트 전까지 임시) |
| 등록 | `Client.vcxproj`/`.filters` | `96.DataFiles\Balance`에 새 JSON None 항목 |
| 테스트 | `ServerGameplayContractTests_PlayerActions.cpp` | ember 4건 (아래) ; `NetworkProtocolHarness.cpp` round-trip 7/2/10 단언, payload size +3(ember) +4(honor title) |
| 문서 | `CLAUDE.md` 전투 수치 bullet | ember 문서·행·protocol 99 한 문장 |

원본과 다르게 사용자가 확정한 규칙: 게이지는 히트당 고정 +4, 화신 15초 고정, 기운 회복은 적중이 아닌 **사용**, 비전투 소켓 자연 회복 없음, 화신 중 0.8초 기운 회복 없음.

## 2. 빌드 중 잡은 것

- `CGameplayCatalogGenerations`는 forwarding wrapper라 `Find_EmberProfile`이 없다 → Replication에서 `m_GameplayCatalog.Active().Find_EmberProfile()`.
- **bootstrap 행은 natural sort로 정렬된다.** `EMBERSKILL`은 `SKILL` 앞에 와서 owner join이 실패했다(`Player ember skill has no owner`). `SKILLEMBER`/`PLAYEREMBER`로 이름을 바꿔 `SKILL\t…`/`PLAYER\t…` 뒤에 오게 했다. 앞으로 owner join 행은 owner kind를 접두사로 쓴다.
- 하네스 `World Snapshot Payload Size`는 이전부터 honor title(protocol 89) 4바이트 등이 빠져 실패 중이었다. ember 3바이트와 title 4바이트를 더했지만 여전히 실패하며, 나머지 11건도 `84u == NETWORK_PROTOCOL_VERSION` 고정 literal 같은 기존 실패다. 내 변경이 걸린 `World Snapshot Players Round Trip`·`Pattern-Bound Player Snapshot Round Trip`은 PASS.

## 3. 검증

| 항목 | 결과 |
|---|---|
| `Publish-GameplayBalance.ps1 -Mode Publish` | 성공, `Gameplay.bootstrap`에 PLAYEREMBER 1 + SKILLEMBER 12 |
| Debug Product 빌드 | PASS (Shared 3 OBJ, Server 45+37+1, Client 179) |
| 하네스 | ember round-trip PASS, 기존 12건 실패 유지 |
| Server 계약 테스트 | `Load gameplay balance bootstrap` PASS, ember 5건(profile/skill row 로드, 소모·잠금, 회복 clamp, 만충 Z·소켓 해제, 15초 450tick 소진) PASS. 실패 23건은 쿠크 게이트·발탄 collider/preset·Bern·DM T 등 기존 실패로 이번 변경과 무관 |
| 사용자 아레나 확인 | 2026-09-22 사용자 로컬 Server+Client에서 게이지·기운·소켓·화신 동작 확인 |

## 4. 사용자 확인 경로

로컬 Server+Client 재시작(protocol 99라 둘 다 새 빌드). 가디언나이트로 Character Select 입장.
1. HP 바 아래 마나 숫자 대신 `오브 0%  기운 10 / 10`.
2. Q/W/E/R/D 적중 → 오브 % 상승(히트당 4). 사용만 하고 빗나가면 안 오름.
3. S(블레이즈 플래시) → 기운 10→6, 열린 소켓 10→9. A/F도 같음.
4. 100%에서 Z → 날개, 기운 10 / 10 복구. 15초 뒤 자동 해제, 또는 Z로 해제 → 오브 0%.
5. 화신 T/A/S/F는 기운만 소모하고 소켓은 안 줄어듦.

## 5. 남은 것

- GK 전용 HUD 아트(오브 게이지·기운 구슬·잠금 표시). 현재는 텍스트 readout.
- 투사체 히트(`Update_Projectiles`)는 게이지를 채우지 않는다. caster shape 히트만.
- 원작 규칙 중 미적용: 비전투 5초 소켓 회복, 화신 중 0.8초당 기운 회복, 화신 진입 시 6m 피증 디버프.
