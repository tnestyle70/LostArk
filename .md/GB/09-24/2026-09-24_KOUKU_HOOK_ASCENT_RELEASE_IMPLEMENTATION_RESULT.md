# 쿠크 갈고리 마지막 상승 시작점 해제 결과

## G00. 원인과 실제 source

기존 해제는 마지막 XYZ 도착을 사용했고, 기존 room release는 현재 좌표를 보존했다.
P18/P19의 마지막 grip 높이가 약22.94m여서 상승이 끝난 뒤 공중에서 attachment만
해제되는 구조였다. Source의 hook mesh/clip이나 Client transform은 수정하지 않았다.

게시된 P18/P19/P33의66개 carrier를 조사했다. 모두 WORLD anchor이며 grip은
`BOSS_LOGIC_WORLD_TRANSFORM_KEY::GripPosition`의 world metre 계약이다. 기존
`Resolve_LogicRegionTransform`도 이 grip에 boss/root basis를 다시 적용하지 않는다.

| Source | 수 | 실제 float32 경계 local time | 경계 grip Y |
|---|---:|---|---|
| P18/P19 terminal ascent | 36 | start phase0/700:6567ms,175/875:6542ms,350:6550ms,525:6558ms | 약2.240m |
| P33 후반 terminal ascent | 15 | 1701ms | 약2.276m |
| P33 전반 split-track ending | 15 | 마지막 visible8212ms | 약2.435m |

P18/P19 staggered start는 같은 동작도 bake sample 위상이 달라진다. 첫 track의6567ms를
36개 전체 기대값으로 복제하지 않았다. 전체 경계 실측은
`out/KoukuHookRelease20260924/source-grip-boundaries.json`에 보관한다.

## G01. 구현 완료

`KoukuSaydonLogicRuntime.cpp`는 WORLD baked grip의 마지막 엄격한 연속 상승을 찾고,
전체 상승폭이 player 몸체 높이보다 클 때 상승 시작점에서 내려놓는다. 같은 높이의 수평
접근/대기 key로는 역추적하지 않는다. legacy/no-grip 및 stationary deadline 경로는 유지한다.

경계에 도달한 tick에서 높게 올라간 sample을 적용하기 전에 경계XZ를 Server navigation의
`Project_PointOnSameLevel`로 검증한다. 성공한 floor XYZ를 commit하고 기존 attachment
release deadline을 현재 tick으로 당겨 room이 movement/combat을 복귀시킨다. 상승 후에는
해당 carrier가 다시 잡지 못한다. 바닥 검증 실패 시 경계에 attachment를 유지해 다음 tick에
같은 위치를 재검증하며, 다시 위로 따라가거나 임의0m 바닥을 사용하지 않는다. room은
player attachment를 Kouku logic보다 먼저 갱신하므로 retry deadline은2tick 이후로 두어
다음 tick의 재검증보다 먼저 attachment가 해제되지 않도록 한다.

`ServerGameplayContractTests_KoukuOverlap.cpp`의 기존 runner에 실제66개 carrier의
독립적인 경계 기대값, 직전tick 유지/경계tick 바닥해제/이후 상승 미추종, room의 정상 해제,
바닥 누락 후 재시도, flat horizontal approach→vertical exit, 이동·회전한 boss에 대한
WORLD grip 보존 검사를 추가했다. 새 파일/schema/프로젝트 등록은 없다.

## G02. 검증 상태

- Python source audit PASS: float32로66개 track을 확인했고51개 terminal ascent와15개
  split ending의 독립 기대값이 모두 일치했다. 전체 경계Y 범위2.240057~2.434501m.
- 수정한 두 Server C++의 `git diff --check` PASS.
- 최종 Debug/Release Product build PASS. `--kouku-object-overlap-contract-test`도
  Debug 660개, Release 628개 PASS이며 두 구성 모두 failures 0이다.
- Client 실행/입력/화면 검증은 수행하지 않았다. 실제 잡힘→끌림→맵 끝 상승 시작점 하차는
  사용자의 Debug/Release 화면 확인 경계다.

일반1/3관문 fence, Mario 낙사, 부활 목적지와 Shared/Client snapshot 계약은 이 수정에서
변경하지 않았다. source/floor 수치 검증을 실제 화면 표시 성공으로 대신 기록하지 않는다.

## G03. 첫 Native 실행의 fixture 수정

통합 담당자의첫Debug overlap 실행에서기존synthetic runner에넘긴빈 `CGameplayCatalog`를
새source 검사가참조하여실제track0개가되었고66/51 count assertion이실패했다. 새검사는
`room->m_GameplayCatalog.Active()`의실제admitted Product generation을사용하도록수정했다.
source catalog 존재도명시적으로검사한다. 시간/바닥/하차의기존기대값은완화하지않았다.
이 fixture 수정 후 실제66개를 순회한 native 검사는 아래 바닥 투영 보정까지 포함하여
Debug/Release 모두 통과했다. 별도 Python 수치 검증과 native 실행 증거를 구분한다.

## G04. Release source5개 바닥 admission 보정

통합 담당자의Release Product 빌드는성공했고, Release overlap는5개바닥admission과
그에따른66/51 count만실패했다. 이전6개밀침/펜스회귀는모두PASS로복구됐다.
실패5개는P33의비상승split종단2개와마지막상승시작점3개이므로상승경계에대한수정도필수다.

통합 담당자의승인으로현재 `ServerNavigation.cpp` 자체를링크한읽기전용native probe를
`out/KoukuHookRelease20260924/nav_probe.exe`에만만들어실제게시nav를측정했다. 결과는
`native-nav-probe.tsv`와`native-floor-admission.json`에보관한다. source catalog 순서의
5개실패는다음과같다(좌표m).

| index/region suffix | 종류 | grip XYZ | 실제 Project_Point floor XYZ |
|---|---|---|---|
| 56/8ce3c934393b4fa80b3ec6c3e062f641 | split종단 | 1.789942,2.434501,927.486755 | 1.75,1.742586,926.75 |
| 57/a353220570dca7f593cc04a5c71929de | 상승시작 | 1.952196,2.276372,927.327576 | 1.75,1.742586,926.75 |
| 58/3e5dd4088858c6632dba7ac0e47a18db | split종단 | 2.760942,2.434501,928.514771 | 2.75,1.299005,929.25 |
| 59/5f74156d5f4bd60b6dd1de71bb322fb7 | 상승시작 | 2.923196,2.276372,928.355591 | 2.75,1.299005,929.25 |
| 61/f0c8864309559aac5eb03b15a2cb3016 | 상승시작 | 3.895196,2.276372,929.382568 | 3.75,1.299005,929.75 |

이XZ는`Sample_SurfacePosition`이false인미생성셀이다. `Project_PointOnSameLevel`은
그셀의높이를같은층의기준으로사용하므로근처에실제바닥이있어도1m허용차에서거절했다.
일반player-spawn이사용하는`Project_Point`는모두같은stage의실제walkablefloor를찾았다.

`Resolve_HookReleaseGround`는SameLevel을우선하고실패시일반projection을시도한다.
양쪽모두finite,동일navigation grid,정확한walkable 여부와수평거리/높이차각각1.8m
이하를검증한다.1.8m는기존player collider의전체높이이며임의Y를적용하지않는다.
실제66개전체의최대수평거리0.911024m,최대높이차1.421995m를native probe로확인했다.
기존61개SameLevel 성공점은일반projection과정확히일치한다. 비상승split2개도기존
공중grip에서단순attachment만푸는대신같은근처바닥으로하차하도록바뀐범위를포함한다.

테스트는독립적일반projection의실측floor를기대값으로사용하고허용높이를기존3.6m에서
1.8m로강화했다. 높은다른deck으로오인하는것을막기위해synthetic grip을10m높였을때는
하차가 거절되는 검사도 추가했다. 구조 검사와 diff-check, 최종 Debug/Release Product 및
overlap native 재실행은 모두 PASS다. 로그는 `out/RaidRelease20260924/`의
`product-{debug,release}-complete.log`, `{debug,release}-kouku-object-overlap.log`다.
최초 실패 증거는 같은 위치의 `*-kouku-object-overlap-first-failure.log`에 보존했다.
