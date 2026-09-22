# 쿠크·세이튼 원본 액션 이펙트 full restore 결과

## 09-22 병합 후 현재 연결 재확인

PR #440의 feature `583e4b5bb`, 충돌 해결 `fb9501f5c`, 최종 main `1b5361a2d`와 현재
Composition revision 2166에는 노란 시선 full restore의 P11 `.presentation.34`가 없다.
`kakulsaydon.effect.ab269d3dda5ae03d6ae7` 정의는 남아 있지만 전체 Pattern occurrence
참조는 0개다. 아래 표의 P11 연결은 최초 설치 당시 기록이며 현재 재생 연결이 아니다.
이번 병합은 삭제된 occurrence를 복구하지 않았다.

EffectCatalog의 main 쪽 기존 1,286개 항목은 모두 동일하게 보존됐고, feature에서 추가한
wind/yellow-gaze full restore 2개를 포함해 최종 1,288개다. 중복·기존 ID 삭제·기존 필드 변경은
없다. 카탈로그 줄 끝의 이름만으로 패턴 재생 복구나 삭제 실패라고 판단하지 않는다.

## 적용 범위

사용자 재지정에 따라 2관문 대형 세이튼의 불어날리기 바람과 노란 시선만 설치했다.
1관문 무력화와 파1빨2 광선은 조사만 하고 변경하지 않았다.

## 최초 설치 결과

| 대상 | 설치 문서 | stable resource | 연결 pattern |
|---|---|---|---|
| 대형 세이튼 바람 | `Data/Effects/Authored/effect.kouku.gate2.bigsaydon.wind.full.restore.effect.json` | `kakulsaydon.effect.7c4ae296cdc86bc637e8` | P99 `.presentation.1` |
| 대형 세이튼 노란 시선 | `Data/Effects/Authored/effect.kouku.gate2.bigsaydon.yellow-gaze.full.restore.effect.json` | `kakulsaydon.effect.ab269d3dda5ae03d6ae7` | P11 `.presentation.34` |

두 full restore 문서는 후보 byte를 그대로 설치했다. EffectCatalog, EffectResourceTree,
`Client/Default/Client.vcxproj`, `.filters`, Composition presentation resources를 함께
갱신했다. P11의 사용자가 저장한 partial occurrences `.29-.33`은 삭제하지 않았다.
P99의 기존 MAP rectangle presentation은 바람 effect로 교체했고, 실제 충돌은 별도의 stable
collider resource와 logic occurrence로 연결했다.

## 검증

- 후보 effect JSON parse, effect asset/group/resource 중복 검증: 통과.
- Composition publish/validate: 통과, revision 2043.
- generated Encounter/Gameplay bootstrap: 통과, P99 Product 포함.
- `git diff --check`와 변경 JSON 전체 parse: 최종 점검 대상.
- 원본 EFTable SkillEffect geometry가 없어 P99 sector collider는 6m/35도의
  `PROJECT_TUNED` 값으로 기록했다. 실제 화면에서 크기·전방·밝기를 확인하는 일은 사용자 몫이다.
- Client 실행·GPU 화면 판정은 수행하지 않았다.
