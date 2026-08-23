# 2026-08-23 Valtan Whirlwind SPIN Effect 연결 결과

## 완료 상태

- `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01`의 stable ID와 authored Effect 문서는 유지했다.
- Product cue owner만 `VALTAN_WHIRLWIND / RECOVERY`에서 `VALTAN_WHIRLWIND / SPIN`으로 옮겼다.
- 실제 재생 animation occurrence는 `valtan.attack.whirlwind.active.clip.01`, clip은
  `mesh_att_battle_20_03`, loop는 `true`다.
- RECOVERY animation의 `mesh_att_battle_20_04` binding은 삭제하지 않았다. 다만 carrier V1 Effect cue는
  더 이상 RECOVERY가 아니라 SPIN에서 재생된다.
- SPIN의 기존 `effect.valtan.pattern.420633.active`와 carrier V1 Effect는 같은 SPIN 구간에서 함께 재생된다.
- 사용자가 Effect Tool에서 저장 중인 authored elements와 timing 값은 수정하거나 재생성하지 않았다.
- carrier materializer `--write`와 역사 hash receipt 재생성은 실행하지 않았다.

## 물리 정본 동기화

`C:\Users\user\Desktop\LostArk`는 `origin/main`의 발탄 3연격/회전공격 분리 변경으로 동기화했다.
동기화 뒤 clip-01 authored/runtime 문서가 모두 8 elements이고,
`VALTAN_TRIPLE_SLASH`와 `VALTAN_ROTATION_SLASH`가 존재하며 이전 `VALTAN_FOUR_SLASH`는 없는 것을 확인했다.
동기화 전 사용자/동시 작업은 named safety stash로 보존했고, 사용자의 Whirlwind authored 문서는 다시 적용했다.

## 자동 검증

- JSON parse: PASS
- `python -B -m unittest Tools.EffectPipeline.test_effect_tool_valtan_saved_rows`: PASS, 18 tests
- `Publish-Effects.ps1 -Mode Validate`: PASS, 156 Effect catalog entries / 171 material-program bindings
- Client x64 Debug compile: PASS. 실행 중인 물리 `Client.exe` 잠금 때문에 기본 출력 링크만 `LNK1104`였고,
  같은 정본 오브젝트를 별도 `OutDir`로 링크해 `Client.exe` 생성까지 PASS했다.
- `git diff --check`: PASS

## 수동 검증

사용자 화면 검증은 남아 있다. Client 새 process에서
Valtan → F1 → All Effects → `VALTAN_WHIRLWIND / SPIN`을 열고 carrier V1 row가
Stage `SPIN`, Clip `mesh_att_battle_20_03`으로 표시되는지 확인한다.
