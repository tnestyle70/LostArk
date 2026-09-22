# 쿠크·세이튼 원본 액션 이펙트 full restore 결과

## 적용 범위

사용자 재지정에 따라 2관문 대형 세이튼의 불어날리기 바람과 노란 시선만 설치했다.
1관문 무력화와 파1빨2 광선은 조사만 하고 변경하지 않았다.

## 설치 결과

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
