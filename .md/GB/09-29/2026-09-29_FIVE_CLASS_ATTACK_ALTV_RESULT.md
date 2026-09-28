# 다섯 클래스 기본 공격력과 ALT_V 피해 조정 결과

## G01. 후보에 반영한 수치

기준 commit은 `d6a9cc2240257779c285110bac9b73714349291a`이고 후보 branch는
`codex/movie-visibility-balance`다. `Data/Balance/Profiles/Retail.balanceprofile.json`에서
창술사·도화가·가디언나이트·워로드·차원술사의 `attackPower`를 23,000에서 230,000으로 올렸다.
각 ALT_V의 coefficient/addend를 함께 보정해 일반 공격력 상승이 ALT_V 피해까지 늘리지 않게 했다.
건슬링어와 슬레이어는 그대로다. base JSON과 공식 receipt는 Retail override 소유 범위 밖이므로
변경하지 않았다. 이 조정은 사용자 요청에 따른 프로젝트 튜닝이다.

| 클래스 / ALT_V | 이전 raw 총량 | 후보 raw 총량 |
|---|---:|---:|
| 창술사 / 34630 | 37,534,467 | 37,534,467 |
| 워로드 / 17250 | 26,131,492 | 26,131,492 |
| 도화가 / 31930 | 34,540,861 | 3,454,086 |
| 차원술사 / 2050540 | 33,655,299 | 33,655,299 |
| 가디언나이트 / 49420 | 27,742,080 | 27,742,080 |
| 건슬링어 / 38320 | 23,000 | 23,000 |
| 슬레이어 / 45820 | 21,916,930 | 21,916,930 |

raw 총량은 실제 Server `CGameplayCatalog::Resolve_Damage`의 정수 결과다. 도화가는 1/10 뒤
정수 내림했다. sub-hit 분할·critical·spread·버프와 피격 대상 조건에 따른 실제 한 타격의
화면 숫자는 이 총량과 구분한다. 창술사의 보스 35줄 정책, 도화가 ALT_V의 보호막과 death-deny,
나머지 모든 skillBuff와 쿨타임·무력화·부위 파괴는 보존했다. 일반 스킬의 고정 addend는
확대하지 않고 기본 공격력만 10배로 했다.

## G02. 자동 검증과 기존 owner 게시

- JSON delta 검사: 요청한 15개 숫자 필드만 변경. 다른 모든 profile 필드·배열 보존 PASS.
- 기존 PowerShell publisher의 PLAYER/DAMAGE/SKILLBUFF serializer를 변경 없이 실행해
  147행 생성. 이전 게시본 대비 PLAYER 5행과 DAMAGE 5행만 변경, 나머지 137행 동일 PASS.
- 현 소스의 GameplayCatalog, ServerBalanceNumericStore, KoukuSaydonBrain, Navigation,
  Collision과 Shared 계약으로 격리 실행기를 컴파일·링크했다. 제품 C++ 수정은 없다.
- 기존 `PreparePatch -> PersistPrepared -> CommitPrepared`로 15필드를 격리 candidate에
  원자 저장한 뒤 새 `CGameplayCatalog.Load`로 다시 읽었다. 일곱 ALT_V/AP와 창술사 35줄,
  numeric revision, gameplay parent, nonnumeric hash 보존 PASS.
- 109,339개 runtime 행의 값 대조에서 변경은 요청한 10행뿐이며 PowerShell serializer와
  일치했다. 기존 `Export_BootstrapBytes`의 non-Kouku/Kouku 재배열 때문에 Git diff에는
  많은 행 이동이 나타난다. 행 이동을 새 gameplay 변경으로 집계하지 않았다.
- 기존 native owner가 만든 source/runtime/active receipt의 raw SHA 세 개 일치 PASS.
  이 네 전달 파일은 `.gitattributes`의 `text eol=lf`로 checkout 때도 유지한다.
- 실제 Git `core.autocrlf=true` clean/smudge filter를 거친 네 파일의 bytes와 raw SHA가
  후보 원본과 동일함을 확인했다. `git-checkout-byte-check.json`에 기록했다.
- 담당 파일 JSON parse, `git diff --check` PASS.

검증 증거는 후보 worktree의 `out/FiveClassBalance20260929/`에 있다.
`numeric-check.json`, `publisher-numeric-check.log`, `publisher-row-check.json`,
`native-build-navigation.log`, `native-link.log`, `native-apply-check.log`,
`native-output-check.json`이 각각 수치·직렬화·컴파일·실제 저장과 재로딩을 기록한다.
격리 실행기와 fixture는 out 중간 산출물이며 소스 커밋에 포함하지 않는다.

```text
numericRevision:
d3482e921dfe0a489ad1404185dec4845aaae942a05edd043d795a55fdc6540a
parentGameplayRevision:
fb25318f180b58ee5d4f023f416f6a76772e4496aa9a8c2f14f9ccff4eb0908f
nonNumericRowsSha256:
134ba4b562291d0e332478ba0caa12eecaa0bc2308e8dd51a8a26dbee2ee6124
Gameplay.bootstrap SHA-256:
9ab5b8ed0b3a96424d7bc598dc663984e2b6b89e53ff3bd4a09241b8dfee10c3
```

## G03. 검증 실패와 남은 적용 경계

전체 `Publish-GameplayBalance.ps1 -Mode Validate`는 변경 전부터 Kouku encounter와
patternbindings의 projected Product stale로 실패했다. 원래 Resources를 읽기 junction으로
연결한 뒤에도 LF 정규화만으로 같아지지 않아 semantic drift임을 확인했다.
기존 파일의 실제/예상 길이는 encounter 24,308,558/24,200,387 bytes,
patternbindings 4,723,480/4,467,458 bytes였다. 해당 Kouku 파일은 수정하지 않았다.
전체 publisher 성공으로 보고하지 않고 G02의 기존 공식 numeric owner 검증과 분리했다.

후보 worktree에는 native owner가 생성한 Retail와 Gameplay.bootstrap 및 두 numeric receipt를
함께 반영했다. 원래 Desktop/LostArk의 authoring·설치 파일, 실행 중 Server, 사용자 draft는
변경하지 않았다. 실제 제품 설치·실행 중 Server 반영과 사용자의 전투 화면 확인은 별도다.
Client/UI를 실행하거나 화면 판정을 하지 않았다. 이 데이터 변경을 위해 제품 전체 빌드나
새 영구 테스트를 추가하지 않았다.

## G04. 최종 파일 반영

G03 이후 사용자 요청에 따라 원래 Desktop/LostArk에 네 source/runtime 파일과 LF 계약을
함께 반영했다. 교체 직전 hash를 확인하고 백업·원자 교체한 뒤 네 파일의 최종 SHA가
검증 후보와 일치함을 확인했다. unrelated 편집과 gameplay 비수치 행은 보존했다.
`out/MovieVisibilityBalance20260929/original-install/installed.json`이 설치 기록이다.
사용자가 저장 후 Client/Server를 종료한 뒤 다른 변경과 함께 Product Debug 빌드도 PASS했다.
이 게시를 실행 중 Server hot reload로 설명하지 않으며 새 Server 시작 시 읽는 파일 반영이다.
에이전트가 Server나 Client를 실행하지 않았고 전투 화면 확인은 사용자에게 남는다.
