# 2026-09-28 플레이어 히트 창(durationMs) Server 판정과 적룡질풍격 복원 RESULT

## 증상

- 적룡질풍격(34610) 피니시: 적과 붙어 쓰면 0.6m 후퇴 → 4.2m 돌진(230ms) 중 콜라이더가 보이지만 붙어 있던 적은 맞지 않는다.

## 원인

- Server `CPlayerSkillSystem`은 히트를 `timeMs`가 지난 첫 틱에서 한 번만 판정하고 `iAppliedHitMask`로 닫았다. animevents의 `endms`(원본 notify 지속 시간)는 Client Debug 와이어 표시에만 쓰였다.
- 피니시 히트(346113)는 5404ms 발동·143ms 창인데, 판정 틱(5404~5437ms)에 캐릭터가 이미 2~3m 전진해 박스(-1.0 offset, 5.55m)가 붙어 있던 적 뒤에 놓인다.
- 2타(346101)는 원본 `PushType=1` 당기기 4.5m인데 `.animnotify` 추출이 PushType을 버려 4.5m 밀치기로 들어가 있었다.

## 구현

- `Server/Public/GameplayCatalog.h` `PLAYER_SKILL_HIT::iDurationMs`, `GameplayCatalog.cpp Parse_SkillHits`: 히트 토큰 13/14/15필드 허용(15번째 = durationMs ≤ 10000, 마지막 반복 + duration ≤ 한계).
- `Server/Public/ServerPlayer.h` `HitWindowTargets`(창 index, NetEntityId) 추가, `iAppliedHitMask` 초기화 4곳(`GameRoom_Helpers` 2, `GameRoom_KoukuPlayerCommands`, `GameRoom_BossStageActions`)에서 함께 비운다.
- `Server/Private/PlayerSkillSystem.cpp`: 발동 후 `fireMs + durationMs`까지 매 틱 현재 위치·조준으로 판정, 대상당 창 1회, `maxTargets`는 창 누적, 창이 열려 있으면 `allFired=false`. duration 0은 첫 틱 1회 판정으로 종전과 동일.
- `Publish-GameplayBalance.ps1 Format-HitShapes`: hitshapes 히트의 optional `durationMs`를 검증하고 0보다 클 때만 토큰 15번째 필드로 싣는다.
- `build_hitshapes.py`: animevents `endms - startms`를 `durationMs`로 기록(rate 반영, 한계 clamp).
- `build_base_hit_rows.py` `pull=` 행: base 체인 Effect 판정 중 DB `PushType=1`인 PK. `fill_animevents_hit_shapes.py`가 그 PK의 `pushr` 부호를 뒤집는다. 창술사 2건(34610 346101, 34050 340500), 다른 직업 0건.
- 재생성: 창술사 `basehits`(+2), `animevents`(pushr -450 ×2), `hitshapes.json`(모든 히트에 durationMs). 다른 직업 hitshapes는 재생성하지 않아 종전 한 틱 판정 유지.

## 검증

- `Publish-GameplayBalance.ps1 -Mode Publish` 성공, `SKILLHIT 34610` 피니시 토큰 `5404:…:143`, 2타 `pushRange -4.5`.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug` PASS(Server OBJ 74 재컴파일·링크, Client 변경 없음).
- 새 Server가 15필드 bootstrap을 읽고 `127.0.0.1:7777` 수신 확인. 아레나 재생 확인은 사용자 몫.

## 남은 경계

- 원본 피니시 이동은 8m인데 루트모션은 4.2m다(별도 항목).
- 다른 직업 hitshapes는 `build_hitshapes.py <Asset>`로 재생성해야 창 판정을 받는다.
- 발탄·보스는 push/pull 대상이 아니다(`WORLD_BOOTSTRAP_KIND::MONSTER`만).
