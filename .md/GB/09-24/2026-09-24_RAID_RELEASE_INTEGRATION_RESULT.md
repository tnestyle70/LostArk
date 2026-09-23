# 레이드 PR 통합·Release 수정 결과

## G00. PR 기준과 데미지 범위

시작은 PR456 branch `codex/kouku-gate3-bingo-flow-0923`, HEAD7cea970b6 clean이다.
main5dc2adca0을98a220ebc로, PR453을0c5cfaa5f로 통합했다.
이들은 기능 브랜치의 병합이며 GitHub main에 PR을 merge한 것은 아니다.

PR454는 이번 통합에서 제외했다. Retail은 같은 Server catalog를 사용하는 모든 world와
Debug/Release에 영향을 주며 플레이어·스킬·피해·보스 수치와 실제 피해 계산/버프/실드 전송을 바꾼다.
창술사34630 ALT_V의 원시 피해는 현재676,520에서37,534,467로, 쿨타임은3초에서300초로 바뀐다.
후자의 값은 치명타·±10%편차·타격 분할·방어 전이다. Valtan HP는600,000→741,285,439,
G3는1,280,621,510, Bingo는1,868,133,028이므로 기존 ALT_V 즉사 테스트는 유지되지 않는다.

열린 PR454의 미완료 소비 경계는 별도 Balance Test RESULT의 리뷰를 따른다.
Server Retail cooldown과 Client 원본 cooldown 분모/tooltip 불일치, 공용 게시의 profile 미전달,
World Retail snapshot 누락 등을 해결하지 않은 채 적용했다고 기록하지 않는다.

## G01. 공통 표시와 병합

Release FPS는 기존 엔진 폰트로 cinematic/HUD suppression 밖에서 항상 표시한다.
F7 Profiler는 유지하지만 열기만으로 Capture가 시작되지는 않는다. Release의 docking과
외부 viewport 및 Debug imgui.ini 복원을 비활성화했다. 기존 제품 텍스트 입력 context는 유지한다.

PR453의 앵콜 공통 UI와21.322초 타이밍은 최신2235 source에 stable ID 단위로 합쳐2236으로 게시했다.
기존5m 낙사·일반1/3관문 펜스·관문 시작점 부활을 유지하고 Mario dead return을 결합했다.
실제 사용자 화면 공백을 해결했다고 자동 판정하지 않는다. UI 수명/실패 격리, hook,
공용 튜너, navigation, preload/성능은 같은 날짜의 각 기능 RESULT에 상세 증거를 둔다.

## G02. 게시와 검증 상태

Kouku projector, Gameplay Publish, Composition Publish와 CardMiro Navigation Publish를 실행했다.
Gameplay는8 player profile/313 skill row/125 damage profile이며 Retail 값이 아니다.
Composition sourceManifestId는5383868019a5f01b701770165851ebed838101f7c3eeb87ddb7b39bd4757e649다.
새 모델·텍스처·음원 Resources를 추가하지 않았다.

최종 제품 Debug/Release Build 모두 PASS다. 공식 Product runner로 Engine/Shared/Server/Client를
빌드했으며 Client/UI를 실행하지 않았다. 마지막 compile receipt는 각각
`20260923T212447403Z-debug-product.json`, `20260923T212515852Z-release-product.json`이다.

| 실제 검증 | Debug | Release |
|---|---:|---:|
| debug-teleport: Kill Boss, 낙사, Mario 및 내장 raid integration | 8097 PASS | 7527 PASS |
| kouku-object-overlap: 실제66 hook, 밀림·펜스·충돌 | 660 PASS | 628 PASS |
| card-maze | 84 PASS | 57 PASS |
| npc-raid-return | 18 PASS | 18 PASS |
| NetworkProtocolHarness | 1263 PASS | 1263 PASS |
| Client movement primitive: 지연 snapshot/재클릭/실제 teleport | PASS | PASS |

위 native 실행은 모두 failures 0/exit 0이다. PowerShell 저장 transaction6개, Encore trim7개,
변경 JSON/XML24개 parse와 git diff --check도 PASS다. 검증 요약은
`out/RaidRelease20260924/final-verification.json`, 세부 로그는 같은 폴더에 있다.

첫 overlap 실패를 그대로 해결했다. PR453의 무조건 보행 낙사 호출이 넉백의 기존 펜스를
우회하던 회귀를 수정했고, 갈고리 테스트의 빈 catalog 참조를 실제 게시 catalog로 바꿨다.
P33의5개 하차 끝점은 같은 grid의1.8m 이내 실제 바닥으로만 투영하도록 보완했다.
실패 증거는 `*-kouku-object-overlap-first-failure.log`에 별도로 보존했다.

## G03. 수동 확인 경계

Client/UI를 실행하거나 캡처하지 않았다.4인 Release의 앵콜 연속 화면, 캐릭터 교체 지연,
미로 클릭, 갈고리 하차와 실제 Bern FPS는 사용자 확인이 필요하다.
지연 snapshot에서 정상 이동을 순간이동으로 오판하던0.35초 경과 제한을 실제 Server tick
차이로 고쳤다. 빠른 두 클릭 뒤30/53/57tick 정상 이동의 강제 snap을 native Debug/Release에서
재현하고 수정 뒤 pose 보존과 실제 teleport 거부를 검증했다. 사용자4인 더블클릭 현상과
동일 원인인지는 입력/pose 시점 자료가 없어 확정하지 않았다. Bern 실제 FPS도 미측정이다.
