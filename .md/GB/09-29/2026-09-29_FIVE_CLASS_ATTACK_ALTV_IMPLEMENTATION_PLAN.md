# 다섯 클래스 기본 공격력과 ALT_V 피해 조정 구현 계획

## G00. 실측과 목표

기준은 `codex/movie-visibility-balance`의 `d6a9cc2240257779c285110bac9b73714349291a`다.
사용자가 지정한 창술사, 도화가, 가디언나이트, 워로드, 차원술사만 기본 공격력을 10배 올린다.
네 클래스의 ALT_V 피해는 현재 값으로 유지하고 도화가 ALT_V는 현재 피해의 1/10로 줄인다.
건슬링어와 슬레이어, 버프·보호막·쿨타임·무력화·부위 파괴 수치는 보존한다.

실효 공격력 정본은 `Data/Balance/Profiles/Retail.balanceprofile.json`의 `players`다.
현재 모두 23,000이며 publisher가 base `PlayerProfiles.json`의 1,000을 덮는다.
같은 profile의 ALT_V damage row를 `PlayerSkills.json`의 `serverDamageProfileId`로 연결했다.
`CGameplayCatalog::Resolve_Damage`는 `floor(AP × coefficientBp / 10000) + addend`를 계산하고
`CPlayerSkillSystem`이 이 총량을 hit에 분할한 뒤 spread, critical, buff를 적용한다.
창술사 ALT_V는 보스에 대해서 별도 `bossHealthBarDamage=35` 정책도 소비한다.

## G01. Retail profile의 최소 변경

다섯 player row의 `attackPower`를 230,000으로 바꾼다. ALT_V의 정수 계수는 네 클래스에서
1/10, 도화가에서 1/100로 내림하고, 현재 정본 AP에서 목표 raw 총량과 정확히 일치하도록
`damageAddend`에 정수 나눗셈 잔여분을 보상한다. 별도의 클래스별 runtime 분기나 새 schema는
만들지 않는다. 이 값은 사용자 요청에 따른 프로젝트 튜닝이며 원작 공식 수치로 주장하지 않는다.

| class / skillId | coefficientBp 변경 | addend 변경 | raw 총량 변경 |
|---|---:|---:|---:|
| LANCE_MASTER / 34630 | 16286651 → 1628665 | 75170 → 75172 | 37534467 유지 |
| WARLORD / 17250 | 11338743 → 1133874 | 52384 → 52390 | 26131492 유지 |
| ARTIST / 31930 | 14962143 → 149621 | 127933 → 12803 | 34540861 → 3454086 |
| DIMENSIONMASTER / 2050540 | 14603465 → 1460346 | 67330 → 67341 | 33655299 유지 |
| GUARDIANKNIGHT / 49420 | 12037617 → 1203761 | 55561 → 55577 | 27742080 유지 |

창술사 35줄 정책을 그대로 둔다. 다른 스킬의 coefficient/addend와 모든 확률·분산을 보존한다.
따라서 일반 스킬의 고정 addend까지 10배 하지는 않는다. 요청 대상은 기본 공격력이다.
공식 provenance receipt는 base authored 문서를 검증하고 Retail overlay를 포함하지 않으므로
base JSON과 receipt는 변경하지 않는다. C++와 project/filter 등록 변경은 없다.

## G02. 게시와 검증

격리된 후보 worktree에서 기존 `Publish-GameplayBalance.ps1`의 Validate와 Publish를 실행한다.
게시 bootstrap의 다섯 AP, 일곱 클래스 ALT_V 연결, 네 ALT_V raw 총량 보존과 Artist 1/10,
창술사 35줄, 도화가 shield/death-deny row 보존을 이전 bootstrap과 대조한다.
그 외 gameplay 행은 동일한지 비교하고 JSON parse와 `git diff --check`를 실행한다.
기존 공식 publisher가 생성한 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`만 전달한다.
원래 Desktop 작업 폴더의 설치 데이터, 실행 중 Server와 사용자 draft는 이 후보 단계에서
교체하지 않는다. Client/UI 실행과 최종 화면 확인은 사용자 영역이다.

## G03. 기존 Server numeric owner로 후보 게시

현 기준선의 전체 Gameplay publisher는 요청과 무관한 Kouku generated Product의 semantic stale로
차단된다. 기존 Product를 재생성해 섞지 않고 F1 숫자 저장의 정본인
`CServerBalanceNumericStore::PreparePatch -> PersistPrepared -> CommitPrepared`를 사용한다.
격리된 `out/FiveClassBalance20260929/native-candidate`에 HEAD의 source/runtime을 복사하고
위 15개 필드만 typed patch로 적용한다. 새 publisher나 제품 runtime 경로를 추가하지 않는다.

기존 owner가 내보낸 Retail, Gameplay.bootstrap, NumericBalance.active.json과
BalanceNumeric.save.receipt.json을 같은 후보 단위로 전달한다. 새 CGameplayCatalog.Load가
receipt를 검증하고 부모 gameplay revision과 nonnumeric hash를 보존하는지 확인한다.
저장 owner의 기존 bootstrap 직렬화는 non-Kouku/Kouku 행 순서를 묶으므로 생성물의 행 이동과
실제 수치 변경을 분리해 비교한다. 요청 열 이외의 runtime 행 값은 모두 보존해야 한다.
raw SHA를 보존하기 위해 `.gitattributes`에서 이 네 파일만 `text eol=lf`로 고정한다.
원본 설치와 실행 중 Server 적용은 후보 게시와 구분한다.
