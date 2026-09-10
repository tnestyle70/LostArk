# 2026-09-07 쿠크 Complete Play revision 및 맵 스포트라이트 반영

## G00. 현재 저장 상태와 목표

F1 Saved Patterns의 Complete Play가 `KoukuSaydon audition expected Product source revision is not active`로 거절된다. 현재 Composition revision은 82, 생성된 encounter/patternbindings는 81, Server Gameplay.bootstrap의 KOUKUSAYDONPRODUCTREVISION은 77이다. Server의 exact revision 검사는 이 차이를 거절하며 그대로 유지한다.

`KAKULSAYDON_G1_PATTERN_9`(대형세이튼_세이튼등장)는 MN_RPCT_06의 빈 DRAFT다. 기존 Product 6개에 추가하지 않고 사용자의 저작 상태를 보존한다. 새로운 2관문 패턴의 stage 구현은 이번 복구 범위가 아니다.

## G01. 저장 정본과 반영 경로

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`을 수정하지 않고 명시 KoukuSaydon domain publisher로 encounter, patternbindings, Server gameplay bootstrap을 동일 revision으로 생성한다. publisher의 revision 비교와 실패 시 기존 생성물 보존을 사용한다.

`월드_1관문스포트라이트`는 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.maplights.json`에 `light.LV_LUT_MIDNIGHTC_ED.1`로 저장됐다. enabled=true인 맵 고정 SPOT이며 runtime maplights에도 같은 값이 있다. 기존 Map publisher로 저장 구조와 runtime 일치를 확인한다. 패턴 LIGHT occurrence로 중복 추가하지 않는다.

기존 dirty Composition, maplights, worldsequences 및 LightResources runtime을 보존한다. C++와 프로젝트/filter 등록 변경은 제안하지 않는다.

## G02. 실행 검증과 사용자 재생

- 기존 map-light validator와 Map publisher의 Check로 저장 값과 배포 일치를 확인한다.
- `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 82`로 두 Product와 Server gameplay 데이터를 생성한다.
- 기존 Composition projector validate와 JSON parse, source/Product/bootstrap revision 및 Product 목록 일치, `git diff --check`를 확인한다.
- C++ 변경이 없으므로 컴파일은 불필요하다. 실행 중인 Server는 이전 catalog를 소유하므로 새 데이터의 활성화는 Server 재시작 뒤에만 완료된다.
- Client/UI는 실행·조작하지 않는다. 사용자가 Server + Client를 재시작하고 Lobby → KoukuSaydon → F1 → KoukuSaydon Arena에서 1관문 보스를 올린 뒤 Saved Patterns → Reload KoukuSaydon Inventory → Complete Play를 확인한다. 스포트라이트 밝기와 실제 보스 재생 판정은 사용자 확인으로 남긴다.


## G04. PRODUCT 버튼·Save 배포·F1 Complete Play 연결 (2026-09-08)

사용자 저장159의 P12/P13/P14는 PRODUCT이나 조커찾기 Bundle3은 P13 편집 때 DRAFT로 내려갔고,
배포 Encounter/patternbindings/bootstrap은149다. Publish_Product의 CreateProcessW는 상대
application name powershell.exe를 resolve하지 못해 Win32 error2를 반환하며 로그도0byte다.
Windows system directory에서 Windows PowerShell 절대경로를 resolve해 기존 publisher를 실행하고
실패 원문을 표시한다. PRODUCT 버튼은 저장과 배포 요청을 연결하되 실제 성공은 background 종료
결과로 구분한다. 포함된 DRAFT Bundle은 자동 승격하지 않고 그 이름을 알린다.

F1은 inventory에서 고른 stable ID를 복사한 뒤 최신 Product를 재조회한다. Workbench의 source
revision을 검증하는 기존 expected revision API는 유지하고, F1/Selected/All의 cached revision
때문에 첫 재생이 거절되거나 이전 데이터를 제출하는 경로를 정리한다. Workbench dirty/publish
진행 중/배포 revision 불일치를 확인한 뒤 재생하며, 성공한 publish는 F1 inventory refresh로
연결한다. Server 활성 revision 검사를 우회하지 않으며 Server 재시작은 여전히 필요하다.

최신 source159는 사용자 변경 Collider6개와 삭제 상태를 보존하고, P13.presentation.9의 지원되지
않는 Z회전 -0.25도만0도로 정규화한다. 조커찾기 Bundle3을 PRODUCT로 설정하고 revision160으로
CAS 갱신한다. 그 외 위치·크기·Yaw·Logic·playAll은 동일해야 한다. 지면 gameplay Collider Detail은
Yaw만 편집하게 해 같은 publisher 거절을 예방한다. 기존 domain publisher로160을 배포하며
생성물은 직접 편집하지 않는다.

기존 native Workbench 회귀, publisher 실제 실행 및 projection 참조 검증, 변경 Client 컴파일과
Product 빌드·JSON/XML parse·scoped diff check를 수행한다. 신규 C++/project/filter/schema는 없다.
Client/UI와 실제 조커 성공 분기·화면 결과는 사용자가 직접 확인한다.


## G05. 게시한 패턴의 다음 Server 재생과 공·공포 회귀 (2026-09-10)

사용자 첨부 화면의 requested233 / Server active232는 게시 파일과 실행 중 Server catalog가
달라 발생했다. 기존 G04는 파일 게시와 F1 목록 갱신까지만 연결했으며 재시작을 요구했다.
이번 요청은 다음 Complete Play에서 새 게시본을 사용하는 실행 계약까지 연결한다.
현재 작업 브랜치는 `codex/kouku-publish-live-update`다. 사용자가 편집 중인 Composition과
기존 생성물 세 파일을 보존하며 사용자가 최종 저장·종료했다고 알리기 전에는 원본을 수정하지 않는다.

`CGameRoom::Evaluate_KoukuSaydonPatternAudition`은 새 재생 요청에서 정확한 source revision의
게시 bootstrap을 검증한다. 기존 CGameplayCatalog parser를 사용하고, 읽는 동안 기존 domain-owner
게시 lock과 충돌하면 기존 상태를 유지한 채 재시도를 안내한다. 기존 공통 gameplay revision은
유지하며 쿠크 이외 gameplay 행의 변경은 이 경로로 적용하지 않는다. 새 파일의 파싱·revision·
Pattern/Bundle/actor preflight가 전부 성공한 뒤 기존 tick command 경계에서 실행 상태를 commit한다.
기존 재생은 시작 당시 immutable catalog와 source revision을 소유하고 Stop/Restart는 그 pin을 사용한다.
새로운 Pattern을 시작할 때만 새 게시본으로 바뀐다. 두 번째 brain이나 local combat 경로는 만들지 않는다.

Client의 기존 typed audition 요청과 bundle-state source revision을 재사용한다. 별도 packet/새 enum은
필요하지 않다. `CKoukuSaydonPresentationPlayer::Reload_Product`는 기대 source revision을
검사한 뒤 animation/Effect/scene/light 구성을 commit한다. 잘못된 최신 파일은 이전 캐시를 유지한다.
`CKoukuSaydonPresentationAssetService::Reload_ProductBindings`는 같은 revision으로 Restart할 때
메모리에 있는 기존 clip/grip 캐시를 유지한다. F1과 Workbench는 새 Pattern 시작과 기존 run Restart의
차이를 표시하고 무조건 Server 재시작하라는 안내를 교정한다. World placement·일반 balance 변경은
이번 Pattern 재생 교체 범위에 포함하지 않는다.

공은 `CWorldSequenceObject -> CMapAssetRenderUtils::Bind_Material`의 명시 diffuse override가
공유 shader의 이전 character material program reset을 건너뛴다. 정적 draw마다 source program,
dye/hit 상태를 초기화하며 원본 `mn_rhcn_00_d.dds`는 보존한다. 기존 CModel/CMaterial 경로를 유지한다.

파1빨2의 `kakulsaydon.g1.logic.36`만 기존 `insideOutcome=FAIL`을 사용한다. GAZE_REAL_BOSS의
parser/serializer, Workbench, projector, Gameplay publisher, Server validator와 Judge_Gaze를
연결해 시야 안은 Fail→FEAR, 밖은 Success가 된다. 기존 진짜 세이튼 찾기의 default SUCCESS는 유지한다.
사용자가 저장한 위치·lifetime·나머지 Logic은 변경하지 않는다.

새 C++ 파일/프로젝트 등록은 필요하지 않다. 기존 focused projection/Server 검사로 방향 경계,
새 revision admission, 기존 run pin, 실패 rollback과 게시 후 재생을 확인한다. 필요한 Client/Server
Debug compile/link, 변경 JSON/PowerShell parse, git diff --check를 실행한다. Client/UI 실행과
화면 판정은 사용자가 직접 하며, 실제 수행한 결과와 미확인 사항은 같은 RESULT에 분리한다.


## G06. Collider에서 Trigger 선택·연결·저장 (2026-09-10 추가 요청)

사용자는 Trigger의 용도는 이해하며 Collider Detail에서 DURATION과 같은 선택·연결·Apply·Save를
요구했다. 이후 요청된 휠윈드의 넉백과 재진입 정책은 G07에서 연결한다. Set_ColliderLogicValues의 기존 연결 편집은
Collider의 start/lifetime을 소유 Logic 및 함께 연결된 Collider로 전달하고 3D geometry는 해당 row에
보존한다. 새 Shared window를 명시 선택한 경우 기존 window 시간 수용 계약은 유지한다.
Trigger kind는 저장 enum을 바꾸지 않으며 Collider에 연결 가능한 접촉 종류만 선택한다.
실제 사용자 최종 저장의 name-only Trigger와 길이가 다른 Collider의 첫 연결도 확인한다.
기존 Kouku preview-transport headless fixture로 연결·공유 시간·다른 strike 보존·실패 rollback·
Save/Reload를 검증하고, Client UI 조작은 사용자에게 남긴다.


## G07. 휠윈드 하나의 접촉 창과 넉백 종료 후 반복 (2026-09-10)

P24의 여섯 회전 4000~5602ms에 Trigger와 Collider를 하나씩 연결한다. 기존 logic38,
Logic occurrence.1, Collider presentation.2의 ID를 유지하며 반경 3m의 BOSS_CURRENT 영역을 사용한다.
최초 진입에 타격하고, 밀리는 동안 추가 피해와 넉백을 모두 억제한다. 넉백이 끝난 뒤에도
영역 안에 있으면 다시 타격하며, 영역 밖에서는 재타격하지 않는다. 이 처리는 Trigger 창이 끝나면 종료한다.

ENTER_AREA의 optional `repeatAfterKnockback`는 기본 false다. 이전의 이탈·재진입 설정인
`rearmOnExit`와 동시에 true일 수 없으며 Workbench Contact repetition에서 한 정책을 고른다.
휠윈드는 repeatAfterKnockback=true다. 이 모드의 PRODUCT는 양수 push를 가진 단일
MAX_HP_PERCENT_DAMAGE Success를 요구한다. 일반 ENTER_AREA의 기본 플레이어당 한 번 계약은 유지한다.

Result의 optional pushRangeM/pushMs는 둘 다0 또는 양수이며 20m/600000ms 이하다.
휠윈드 전용 logic39는 최대 HP 10%, 발탄 SWEEP과 같은 2m/242ms로 저장한다. 방향은
쿠크→피격자이며 기존 Server hit reaction과 navigation/collision을 그대로 사용한다.
Can_ArmPlayerHitReaction으로 공통 피격 억제 조건을 함께 소비하고 플레이어별 next contact tick은
벽에 막힌 경우에도 설정한 242ms보다 빠른 재타격을 막는다. Collider를 벗어날 필요는 없다.

Client 편집·저장, projector, Encounter, publisher, bootstrap, Server runtime을 연결한다.
기존 native/서버/projector 검사로 Save/Reload, 양수 push 조건, 첫 타격·밀리는 중 억제·내부 재타격·
외부 미타격·벽/이동 경계·window 종료를 확인한다. 표준 Product 빌드와 domain publish를 수행한다.
Client/UI 실행과 애니메이션·피격 화면 판정은 사용자가 한다. 신규 C++/프로젝트 등록은 없다.
