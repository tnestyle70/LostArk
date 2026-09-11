# 쿠크 revision 254 생성 데이터 보존

작성일: 2026-09-11. JS의 마지막 로컬 작업 상태 보존.
브랜치: `codex/js-effect-checkpoint-0911`.

## 커밋 범위와 사용자 결정

- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`.
- `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`.
- 두 파일의 sourceRevision은 243에서 254로 변경되어 있다. 저작 정본
  `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` revision 254는 이미 HEAD에 커밋되어 있다.
- P25 animation occurrence 추가, 길이 14667 → 16167ms 및 `boss.kouku.pizza.aura` presentation 연결을 포함한다.
- 사용자 결정: "현재 파일 그대로 보존해 커밋". publisher 재생성이나 생성물 수동 편집 없이 현재 bytes를 보존한다.

## 검증과 남은 차이

- 두 JSON은 기존 reader의 중복 key·숫자 검사 포함 parse 성공.
- 공식 projector `project_kouku_saydon_composition.py --check`는 Encounter의 바이트 불일치로 실패했다.
- 같은 projector로 메모리에서 기대 결과를 생성해 두 파일과 재귀 비교했다.
  소수점 오차 1251개(최대 절대 차이 `7.105427357601002e-15`) 외에는 구조·값 차이가 없었다.
  비교 허용 범위는 상대·절대 `1e-12`이며 바이트 일치 검사 PASS를 대신하지 않는다.
- Debug Product 증분 Build·배포 성공. Server 재시작과 Client 실행은 수행하지 않았다.
- `git diff --check` 성공. 변경 XML 없음.
- 함께 modified로 표시된 맵 출력 6개는 Git clean filter를 적용한 object hash가 HEAD와 같았다.
  실질 변경이 없어 새 맵 내용은 커밋하지 않는다. 원래 물리 파일은 보존한다.

## 다음 단계

현재 상태를 복원할 수 있도록 보존한 커밋이다. projector 바이트 검사가 필요한 후속 배포에서는
공식 publisher로 재생성 후 다시 검증해야 한다. 이번 커밋을 Server 적용·사용자 화면 PASS로 기록하지 않는다.
