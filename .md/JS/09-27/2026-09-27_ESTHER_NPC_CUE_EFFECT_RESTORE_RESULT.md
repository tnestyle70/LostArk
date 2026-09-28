# 에스더 NPC cue 이펙트 복구 결과

## 문제

- main `8bae8fcf` 이후 에스더 5명(NPC_58700/59030/59060/59504/59620)의 V1 cue 이펙트가 전부 안 나왔다.
  웨이 도철은 Effect V2 바인딩이라 따로 보였다.
- 원인: `20c8be4a`(Bern3 배 항해·마하라카)가 `CNpc::Update_ActionEffectCues`에서 `durationMs != 0`이면
  `bOwnerSustainedSourceLoops=true`와 `fSourceLoopEndSeconds`를 함께 넣었다. `Spawn_LevelPlacement`는
  이 조합을 거부하므로 매 소환마다 `Level-placement Effect spawn descriptor is invalid`로 격리됐다.
- 같은 커밋의 `Release_ActionEffectCues`가 NPC 소멸·다음 동작에서 이펙트까지 멈춰, 이난나 마법진처럼
  NPC despawn 뒤 남아야 하는 이펙트도 함께 지웠다.

## 변경

- cue 이펙트는 이전처럼 level 소유·NATURAL 정책으로 스폰하고 수명은 이펙트 문서가 정한다.
- `NPC_ACTION_EFFECT_LIVE_CUE::iEffectHandle`을 제거했다. `Release_ActionEffectCues`는 cue 사운드만 멈춘다.
- KCY의 cue 사운드 길이 절단(`fSoundStopAtSeconds`)은 유지했다. 현재 cue 문서 5개는 formatVersion 1이라 사운드가 없다.

## 검증

- Debug Product 빌드 PASS (Client OBJ 21 재컴파일).
- `git diff --check` 통과.
- 로컬 Server·Client에서 캐릭터 선택 F1 에스더 소환 시 이펙트가 나오는 것을 사용자가 확인했다.
