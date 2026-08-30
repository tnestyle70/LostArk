# Effect Authored 런타임 도달성 정리 구현 계획서

기준일: 2026-08-29

기준 물리 작업 폴더: `C:/Users/user/Desktop/LostArk`

## 0. 현재 실제 상태와 작업 경계

`Data/Effects/Authored`에는 468개 문서가 있고 이 경로 자체의 시작 시점 Git 변경은 0개다.
현재 working `EffectCatalog.json`은 기존 작업으로 298행에서 197개
`DIRECT_AUTHORED_DOCUMENT` 행만 남은 상태지만, catalog 밖 문서가 물리 폴더에 그대로 남아 있다.

이번 작업은 사용자가 승인한 303개 후보에서 발탄 V1 alias 6개를 보류한다.

```text
보존
  실제 캐릭터 입력 cue/fallback 102
  발탄 Product cue/Boss combat visual 63
  발탄 V1 alias 6
  합계 171

삭제
  catalog/draft 밖 기존 orphan 268
  catalog 또는 draft 계약에서 추가 해제할 항목 29
  합계 297
```

정렬한 삭제 상대 경로 집합의 SHA-256은
`54e0e9d980b31cb55beb14249f247f624c52aa3be2c1be7d785a80de347fde01`이다.

## G00. 물리 Authored 297개 제거

### 목표와 종료 증거

`Data/Effects/Authored`를 468개에서 171개로 줄이고 다음 세 집합만 남긴다.

```text
Character Product 102
Valtan Product 63
Valtan debug V1 alias 6
```

삭제 집합은 `C:/w/lostark-structure-cleanup-0829`의 staged Authored 삭제 268개와
아래 추가 29개의 합집합이다. 발탄 V1 alias 6개와의 교집합은 0이어야 한다.

### 추가 29개 계약 해제 대상

```text
Character catalog-only/audition 17
Valtan catalog-only 8
Valtan catalog+draft 1
Valtan draft-only 3
```

파일명은 안정적인 `effectAssetId`에서
`Data/Effects/Authored/<effectAssetId>.effect.json`으로 유도한다.

## G01. EffectCatalog와 PlayerSkills 계약 정리

### `Data/Effects/EffectCatalog.json`

추가 29개 중 catalog에 존재하는 26개 행을 삭제한다. 기존 197행에서 171행이 남아야 하며,
남은 catalog ID와 물리 Authored ID는 정확히 같은 집합이어야 한다.

### `Data/Balance/PlayerSkills.json`

catalog에서 이미 빠진 DimensionMaster 구형 base effect ID 12개를 빈 문자열로 바꾼다.
실제 표현은 각 skillbinding clip의 `.animevents effectref=asset` cue가 소유하므로 정상 입력의
표현은 유지된다. Artist 3개와 Warlord 1개의 유효 fallback만 남긴다.

같은 12개 field의 `sourceValue`와 `resultValue`를
`Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`에서도 빈 값으로
동기화한다. 다른 밸런스 field는 수정하지 않는다.

## G02. 발탄 draft와 V1 보류 경계

### `Data/Effects/ValtanPatternAuthoringEffects.json`

현재 `DRAFT_ATTACHED` 4개를 모두 해제해 `bindings`를 빈 배열로 만든다.
파일과 schema root는 유지해 “활성 draft 없음”을 명시적으로 표현한다.

### 보존 계약

`Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json`과 그 target 6개는 수정하지 않는다.
발탄 Product cue 61개와 `BossCatalog` combat visual을 합친 Product 63개도 수정하지 않는다.

## G03. validator와 하네스

### `Tools/EffectPipeline/validate_effect_sources.py`

다음을 실행 계약으로 추가한다.

```text
PlayerSkills의 non-empty effectId는 모두 Product catalog에 존재
ValtanPatternAuthoringEffects의 declared draft만 catalog 밖 Authored를 허용
물리 Authored = Product catalog + declared draft
실제 character cue/fallback + Valtan cue/Boss visual + Valtan V1 alias = Product catalog
```

잘못된 catalog ID, undeclared Authored 문서, stale PlayerSkills effectId를 fail-closed한다.

### `Tools/EffectPipeline/test_validate_effect_sources.py`

정상 exact join과 다음 실패를 검증한다.

```text
undeclared Authored orphan
catalog 밖 PlayerSkills effectId
runtime reachability 밖 Product catalog 행
```

기존 역사 자료나 materializer가 삭제된 Effect ID를 문자열로 언급하는 것은 Product 소비로
승격하지 않는다. 현재 Client/Server runtime data join만 보존선으로 사용한다.

### 삭제된 corpus를 다시 만드는 구형 경로

삭제된 발탄 requested-element corpus 전용 one-shot generator/test와 이미 world row가 없는
red-vortex 전용 obsolete test를 제거한다. 남은 Effect Tool과 Map lifecycle 테스트는 empty draft와
물리 파일 부재를 정상 계약으로 검사하도록 바꾼다.

`Client/Private/Effect_Tool.cpp`의 호출자 없는 구형 DimensionMaster T unified draft 생성 함수는
이미 삭제되는 baseline/source 문서만 입력으로 사용하므로 선언·정의와 두 retired ID 상수를
함께 제거한다. 현재 입력 cue가 소비하는 `effect.dimensionmaster.skill.2050500.unified` 지원은
보존한다.

## G04. 적용 순서와 검증

1. catalog 26행과 PlayerSkills 12개 stale fallback을 정리한다.
2. 발탄 draft sidecar를 빈 bindings로 교체한다.
3. validator와 focused tests를 갱신한다.
4. 검증된 상대 경로 297개를 물리 삭제한다.
5. JSON parse와 ID 집합을 다시 계산한다.
6. Effect source validator와 focused Python tests를 실행한다.
7. 발탄 cue 61개, Boss visual union 63개, V1 alias 6개 보존을 다시 확인한다.
8. `git diff --check`를 실행한다.

기대 결과:

```text
Authored files 171
EffectCatalog rows 171
Valtan draft bindings 0
PlayerSkills non-empty effectId 4
undeclared Authored 0
missing Product source 0
Valtan V1 alias target missing 0
```

Client 화면 실행과 visual fidelity 판정은 이번 데이터 정리 범위에 포함하지 않는다.
