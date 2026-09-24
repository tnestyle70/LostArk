# 쿠크 Complete Play 준비 버전 실패 표시 결과

## G00. 실제 원인

Client/Bin/Debug/Diagnostics/client-session-33120.jsonl의 runEpoch 1은 08:30:14에 PREPARING 상태였고, 08:33:32.835의 kouku.raid.prepare가 resources.final_revision에서 실패했다. 저장 Action은 2238, Server가 고정한 Action은 2237이었다. Sequence는 양쪽 171, composition ID와 gameplay revision은 양쪽 동일했다. 따라서 서로 다른 저장본을 섞어 실행하지 않도록 실패한 것이 맞다.

같은 로그의 `Loaded Boss Patterns tree: 119 patterns / 26 parents / 11 bundles; 6 patterns and 2 bundles unavailable`는 CKoukuSaydonBossTool::Reload의 성공 안내였다. Load_ProductIndex는 executable 113개와 unavailable 저작 항목을 구분해 목록을 commit한 뒤 true를 반환한다. 뒤의 revision 비교가 false가 되어도 성공 문자열을 지우지 않았으므로 화면과 FAILED acknowledgement에 잘못된 실패 이유가 남았다. 미지원 6패턴 때문에 전체 raid 준비가 거부된 것은 아니다.

다음 request 3은 08:34:54.440에 제출돼 08:34:54.512에 epoch 0 / `Exact published raid Product could not be admitted`로 거절됐다. 이때 첫 요청과 달리 Server 입장 검사까지 도달했다. 기존 코드는 여러 admission 실패를 같은 문자열로 합쳤으므로 해당 로그만으로 내부 분기를 확정할 수 없다.

## G01. 게시 작업과의 시각 대조

out/KoukuHealthFlow20260924/applied/apply-validation.json과 publish.log 및 publisher receipt를 대조했다.

| 시각(KST) | 상태 |
|---|---|
| 08:30:52 | 저장 Action 2237 → 2238 원자 교체 |
| 08:30:58 | Kouku owner 공식 게시 시작 |
| 08:32:19 | Client용 Encounter/patternbindings 2238 생성 |
| 08:33:32 | 첫 준비의 Action 2238 / pin 2237 불일치 |
| 08:34:54 | 다음 START 거절, 전체 publisher transaction 진행 중 |
| 08:35:35 | Server Gameplay.bootstrap 2238 교체 |
| 08:35:37 | owner 전체 PASS, 278.875초 |

Server Load_PublishedKoukuProduct는 먼저 runtime-owner.lock을 획득한다. 따라서 진행 중 게시를 읽는 요청은 이 경계에서 거부될 수 있고, 당시 디스크 bootstrap도 아직 2237이었다. lock 우선 거절 가능성과 실제 게시 중 구버전 상태는 확인했지만 예전 로그에 없는 구체 분기를 추정 결과로 확정하지 않는다. 현재 게시본의 원본/Client projection/Server bootstrap은 2238로 일치하며, 이 사실은 실행 중 프로세스가 자동 Reload됐다는 의미가 아니다.

## G02. 구현

Client/Private/MainApp.cpp의 UpdateKoukuGateCompletePlay에서 initiator의 resource 준비 완료 검사와 participant의 최초 pin·published resource·최종 revision 검사를 수정했다. Action, Sequence, published Action은 mismatch 항목과 expected/current 숫자를 표시한다. composition/gameplay 불일치도 정확한 항목을 표시한다. 파일 Reload 실패는 해당 실제 실패 이유를 유지하고, 성공 Reload의 목록 안내는 revision 실패 이유를 대체하지 않는다. 기존 diagnostic stage, immutable pin, resource 준비 완료 전 READY 금지와 FAILED acknowledgement는 유지한다.

Server/Private/GameRoom_KoukuRaidFlow.cpp의 admission은 published load status, non-Kouku baseline, requested/published Action, revision 역행, 관문별 Sequence 불일치를 구분한다. 거절 메시지는 기존 192-byte wire 제한과 UTF-8 경계를 지켜 전송한다. 자료를 다시 요청하거나 publish를 자동 실행하지 않고 기존 admission 조건과 선택 closure 검증을 그대로 유지했다.

## G03. 검증 상태

ServerGameplayContractTests_KoukuRaid.cpp의 Debug/Release 공통 실제 두 명 파티 fixture에 unpublished Action·Sequence 요청 거절 검사를 추가했다. requested/published 숫자와 원인을 확인하고, room epoch·catalog·보스 수·player 위치·HP를 보존하는지 확인한다. 이어지는 기존 실제 entry trigger 재시도는 현재 정확한 source로 PREPARING/CINEMATIC까지 진행하는 검사다. 별도 게시 파일이나 새 합성 실행기를 만들지 않았다.

- 진단 및 fixture 변경 C++ 네 파일: UTF-8 BOM 없음 / 기존 working CRLF 보존.
- git diff --check: PASS.
- source와 로그 정적 대조: PASS. runtime-owner 보호와 unavailable inventory의 true 반환 경로를 확인했다.
- root 통합 Debug/Release Product 빌드: PASS. 로그는 out/F1BalanceGate1-Debug-product.log와 F1BalanceGate1-Release-product.log다.
- `--kouku-raid-contract-test`: Debug/Release 모두 failures 0. 새 Action/Sequence 실제 revision 거절 및 상태 보존 검사와 후속 정상 재시도도 PASS다. 로그는 out/F1BalanceGate1-{Debug,Release}-raid-contract.log다.
- Debug `--kouku-bundle-contract-test` 최초 실행은 unrelated damage reload fixture 한 건만 실패했다. 아래 fixture 수정 뒤 최종 95 PASS / 0 FAIL이다. 최초 로그는 `out/F1BalanceGate1-Debug-bundle-contract.log`, 최종 로그는 `out/F1BalanceGate1-Debug-bundle-contract-final.log`다.
- Client 실행, UI 조작과 프로세스 종료는 수행하지 않았다. 실제 화면의 Complete Play 성공은 사용자 확인 대상이다. 데이터 게시와 전체 제품 빌드 증거는 RELEASE_F1_RAID_TEST RESULT를 따른다.

Debug bundle의 실패는 제품 reload 결함이 아니라 Retail 이전의 필드 위치 가정이었다. ServerGameplayContractTests_KoukuBundles.cpp는 DAMAGE 행의 마지막 필드를 99999로 교체했다. 실제 첫 행은 `DAMAGE damage.player.17000 100 0 0 10`이며 마지막 10은 spread다. 99999로 교체하면 GameplayCatalog의 spread < 100 검사를 어겨 published.Load가 정상 실패한다. 따라서 이 fixture는 유효한 별도 balance 게시를 더 이상 표현하지 않았다.

fixture는 stable damage ID를 읽고 세 번째 rate 필드만 기존과 다른 99999 또는 99998로 변경하도록 고쳤다. 게시 후보 catalog가 실제로 유효하며 rate만 달라졌는지 먼저 확인한다. 이어 실제 Kouku admission 결과의 rate, coefficient, addend, spread가 원래 active generation과 모두 같은지 검증한다. admission 실패 시 실제 status를 남긴다. 제품 runtime, 수치 허용 범위 및 reload 검증은 변경하지 않았다.

변경은 Run_KoukuBundles의 전체 `_DEBUG` 구간 내부이며 Release의 전처리 산출물은 바뀌지 않는다. Debug Server 증분 빌드는 PASS이며 로그는 `out/F1BalanceGate1-Debug-server-bundle-fix-build.log`다. 최종 bundle 재실행에서 유효한 rate 변경 및 active damage 보존, 게시 잠금, 이전 run pin 유지, 다음 run 새 Product 사용을 모두 확인했다. 검사 뒤 제품 Gameplay bootstrap hash도 기존 게시 값과 일치한다. C++ UTF-8 BOM 없음 / CRLF와 git diff --check는 PASS다.
