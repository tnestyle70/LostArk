# 2026-08-23 Valtan Whirlwind SPIN Effect 연결 결과

## 완료 상태

### 2026-08-24 후속 튜닝

- SPIN 서버 stage를 `1400 ms`, 4 hit를 `350 ms` 간격으로 맞췄다.
- `mesh_att_battle_20_03`의 Product playRate를 `0.761904762`로 맞춰 SPIN 동안 두 회전을 재생한다.
- carrier V1 axe Trail을 `b_wp_r_01` element-local follow로 연결하고 distance UV/tessellation을
  authoring 가능하게 했다.
- Trail의 transform tail을 source window 안에 고정하기 위해 Element life를 `0.916666687 s`,
  point life를 `0.150000006 s`로 두어 합계가 약 `1.066667 s`가 되게 했다.
- Effect Tool Product preview는 Server stage duration을 timeline 상한으로 사용한다.

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
동기화 뒤 3연격 clip-01 authored/runtime 문서가 모두 7 elements이고,
`VALTAN_TRIPLE_SLASH`와 `VALTAN_ROTATION_SLASH`가 존재하며 이전 `VALTAN_FOUR_SLASH`는 없는 것을 확인했다.
동기화 전 사용자/동시 작업은 named safety stash로 보존했고, 사용자의 Whirlwind authored 문서는 다시 적용했다.

## 자동 검증

- JSON parse: PASS
- `python -B -m unittest Tools.EffectPipeline.test_effect_tool_valtan_saved_rows`: PASS, 21 tests
- `Publish-Effects.ps1 -Mode Validate`: PASS, 163 Effect catalog entries / 171 material-program bindings
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS, 34 boss patterns / 131 stages
- Server x64 Debug build와 `Server.exe --contract-test`: PASS, failures 0
- Client x64 Debug build: PASS, `Client/Bin/Debug/Client.exe` link 완료
- `git diff --check`: PASS; 기존 LF→CRLF 경고만 출력

## 수동 검증

사용자 화면 검증은 남아 있다. Client 새 process에서
Valtan → F1 → All Effects → `VALTAN_WHIRLWIND / SPIN`을 열고 carrier V1 row가
Stage `SPIN`, Clip `mesh_att_battle_20_03`으로 표시되는지 확인한다.
