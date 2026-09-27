# 2026-09-27 플레이어 반복 타격·자릿수 넘친 타격 행 복원 RESULT

## 구현

- `build_hit_repeats.py`: 직업 Action `.loa`의 `CEFActionNotify_Effect`에서 반복 횟수(+32 int32)와 간격(+36 float 초)을 읽어 `Data/Animation/Reference/<Asset>/<Asset>.hitrepeats`에 PK·시작 ms별로 기록한다. 기존 `.animnotify` 추출은 이 두 값을 읽지 않아 N타 notify가 Server에 1타로 들어갔다.
- `build_base_hit_rows.py`: 효과 행이 10개를 넘어 PK가 다음 decade로 넘어간 행(예: 34610 → 346110~346113)을 스킬 툴팁이 가리키는 행과 계수(ValueA/B/F, Key)로 대조해 `<Asset>.basehits`에 기록한다.
- `fill_animevents_hit_shapes.py`가 두 파일을 읽어 HIT 행의 `rep/repms`를 채우고, 같은 시각에 decade 행이 없을 때만 넘친 행을 본 스킬 판정에 추가한다.
- 대상: LanceMaster, Warlord, Artist, DimensionMaster, GuardianKnight의 `.animevents`와 hitshapes 재생성.
- `PlayerSkills.json` 34160(공의연무) 1단계 `comboAdvanceMs` 1350 → 1370. 복원된 4타(980+3×130)가 전환 전에 끝나야 한다. 기존 값도 `PROJECT_TUNED`였으며 receipt는 `Update-BalanceProvenanceReceipt.ps1`로 동기화했다.

## 검증

- `fill_animevents_hit_shapes.py --check`, `build_hitshapes.py --check`: 5직업 unchanged.
- `Publish-GameplayBalance.ps1 -Mode Publish` 성공. hit-shape coverage 95/95.
- 사용자 로컬 Server+Client(Debug) 확인: 정상 동작 확인(사용자 서면 판정 "잘된다").

## 게시물 참고

- `Gameplay.bootstrap` diff에 쿠크 PATTERNLOGICRESULT 21행의 부동소수 끝자리 변경이 섞였다. 이 PC에서 쿠크 projector를 다시 돌린 결과이며 동작 차이는 없다.
- 발탄 presentation generation 해시 갱신은 브랜치의 에스더 커밋이 바꾼 `EffectCatalog.json`, `CharacterSoundCatalog.json` 반영이다.

## 남은 경계 (쿠크 담당 인계)

- 쿠크 projector의 validate는 byte 비교라 이 PC(`core.autocrlf=true`)에서 CRLF 체크아웃 때문에 항상 stale로 실패한다. 수학 함수 결과의 끝자리 차이도 PC마다 난다.
- 로컬에서 재생성한 `KoukuSaydonEncounter.json`, `KoukuSaydon.patternbindings.json`은 커밋하지 않았다.
