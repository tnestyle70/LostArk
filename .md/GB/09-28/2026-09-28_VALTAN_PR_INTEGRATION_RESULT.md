# 발탄 수정·PR 471~474 통합 결과

## G00. 검토와 현재 경계

원래 open PR #471~474의 HEAD를 고정하여 검토했다. #471은 현재 작업 브랜치의 LAN endpoint·F1 체력바 변경, #472는 마하라카 복원·MapTool, #473은 수리창·내구도 HUD, #474는 에스더 NPC action cue 복원이다. 네 PR을 현재 Codex 작업에 첨부했다. 원래 HEAD의 읽기 전용 검토 결과는 `out/OpenPrReview20260928/heads.json`, `review.json`, `json-review.json`에 있고, 최종 통합 검증은 아래 G03에 기록한다.

#473의 `CDurabilityHudView::Apply_Part`는 직전 적용한 width를 원본 damaged width로 나누므로 DESTROYED 상태에서 매 Update마다 크기가 누적된다. helmet의 원본 layout width 10은 한 번 적용할 때 10.6667이어야 하지만 60 update 뒤 512.569가 된다. 첫 유효 rect의 authored scale을 보존하도록 수정했다. 실제 production method를 사용한 8개 부위·600 update·400 상태 전환과 실패 후 재시도 검증에서 106,082 assertions가 통과했다. 변경 CPP의 컴파일도 통과했다. 근거는 `out/OpenPrReview20260928/fix-candidate/verification.json`이다. 다른 원래 HEAD의 검토에서 추가 actionable 결함은 발견하지 못했다. #473은 내구도 Server 상태·실제 수리 처리를 후속 범위로 명시하며 이번 통합에서 구현됐다고 주장하지 않는다.

## G01. 복구 돌 폭발 반영

피자 runtime explode, 독립 편집 composite explosion.full, ground-roar explode의 debris 표면을 생성 돌과 같은 native2391로 교체했다. 기존 fragment mesh, 모든 Element ID/transform/timing/색/운동/수명, 16개 burst와 사용자 검은 파동·전조는 보존했다. 기존 dissolve 구간에 맞춘 필수 dynamic 4채널만 연결했다. ground-roar effect를 공유하는 part-break도 같은 표면을 사용한다. 새 Resources나 shader 변경은 없다.

추가로 실제 돌을 생성하는 STRUGGLING의 별도 `effect.valtan.struggling.rock.explode`도 동일하게 반영했다. 사용자의 "버러지"에 해당하는 TRASH 잡기·카운터 sequence는 8개 track을 따라가도 stone mesh나 rock combat-object spawn이 없었다. 대상 확인 질문에 답변이 없는 동안 STRUGGLING을 적용 대상으로 가정한다고 알렸으며 TRASH에 새로운 돌 생성 로직을 넣지 않았다.

실제 Codec/Playback와 native2391 parameter binding으로 최종 4문서×9시점 36행, 실제 입자 535개의 count/World/velocity/color/age가 이전과 동일함을 검증했다. 끝난 debris는 기존 1.2초 수명 뒤 사라진다. 두 설치 WModel의 실제 GPU buffer admission도 통과했으나 pixel draw나 실제 화면 검증은 하지 않았다. canonical writer admission·입력 hash 재확인·백업·원자 교체를 통해 4문서를 반영했다. 근거는 `out/ValtanRestoredRockExplosion20260928/installation-receipt.json`, `installation-struggling-receipt.json`, `native.log`, `geometry.json`이다.

## G02. 실제 Resources 반영

발탄 변경 리소스 3개 42,602,616 bytes를 `C:/Users/user/Desktop/GBResources2`에 Resources-relative 경로 그대로 복사했다. `Character/Valtan/Ghost/MN_RPBF_02.wmodel`은 기존 모델의 원본 normal/tangent 복원 교체본이며, 에테르의 `fx_a_line_002.dds`, `fx_c_atypical_003.dds` 두 파일은 신규다. 설치본과 복사본의 bytes/SHA-256이 일치한다. 근거는 `out/ValtanIntegration20260928/resource-delivery.json`이다. Data JSON·shader source·EXE는 이 폴더에 섞지 않았다.

#472의 새 리소스 일부가 현재 Client Resources에 없어 다운로드 폴더의 `CY_Resources`를 조사했다. PR 작성자가 기록한 3730 files/1,241,075,852 bytes와 정확히 일치했다. 같은 상대 경로로 624개 249,882,676 bytes를 추가·교체하고 기존 3106개는 bytes 동일하여 유지했다. 교체 전 파일을 `out/ValtanIntegration20260928/resource-before`에 보존했으며 원자 교체 직전 최신 bytes와 교체 후 bytes를 검사했다. 근거는 `maharaka-resource-install.json`이다. 마하라카 리소스를 발탄 전달 폴더 GBResources2에 섞지 않았다.

incoming JSON에서 확인한 712개 물리 참조 중 54개의 신규 누락을 이 전달본으로 해소했다. 별도의 `NPC_SHIP_SHIPWRIGHT`, `NPC_SHIP_HARBORMASTER` 모델 2개는 기존 HEAD의 NpcCatalog에도 같은 경로로 존재하는 설치 누락이며 이 PR의 신규 마하라카 자산이 아니다. 이를 신규 회귀로 기록하지 않으며 전체 게임 Resources 완전성을 보증하지 않는다.

## G03. 최종 통합 검증

#474, #473, #472의 검토한 exact HEAD를 현재 브랜치에 순서대로 merge했다. AGENTS의 현재 LAN endpoint와 incoming 명시적 Local/Saved/Team 선택 계약을 함께 유지했고, gotchas의 독립 항목도 보존했다. 나머지는 자동 병합됐다. 수리 HUD 수정은 `1528e9197`로 추가했다.

마하라카 source stand·support·NPC·material/source coverage와 SourceCharacter 등록의 집중 검사 39건이 통과했다. 워터팡의 import 계약 4건도 통과했지만, 원본 curve/audio 검사는 이 PC에 없는 Git 제외 추출 cache 두 개 때문에 재실행하지 못했다. 테스트를 우회하거나 source 검증 성공으로 표시하지 않았다. 현재 게시 데이터는 World MAHARAKA 34 placements Validate, Map 4,671 placements/7 files Publish 및 후속 Check를 통과했다. Map publisher가 5개 JSON의 CRLF를 LF로 정규화했으나 줄바꿈 외 값 변화는 없었다. 근거는 `out/ValtanIntegration20260928/incoming-focused-tests.json`, `maharaka-world-validate.log`, `maharaka-map-final-check.log`다.

통합 Engine Debug/Release 빌드가 통과했다. 4연속·착지 판정은 [별도 결과](2026-09-28_VALTAN_FOUR_SLASH_TRACKING_AXE_COLLIDER_RESULT.md)에 기록한 Server22개·Client542개·Python21개 검증과 독립 리뷰를 통과했다. Composition 회귀74건도 통과했다. ProjectV2는 새 contact가 포함된 두 Product를 게시했고 Composition publish도 통과했다.

초기 저장 Data 중 요청한 판정·돌 변경 및 그 파생 projection/receipt만 변경됐으며, 나머지37개 JSON의 의미가 동일함을 확인했다. presentation, Sound cue, 렌더링 옵션, 쿠크 HUD 정본4개는 초기 raw SHA까지 동일하다. GBResources2 전달3개도 설치본과 여전히 SHA가 일치한다. `out/ValtanIntegration20260928/final-input-verification.json`에 비교한 필드와70개 JSON/XML parse 결과를 남겼다.

공식 Gameplay Publish는108,888행/32,167,636 bytes로 통과했다. 현재 presentation161 artifacts의 generation은 `c0329e7e10ae84fed6d556ea88a529f0ab4502daf844acd642f82af23876c283`이며 Server bootstrap과 동일하다. 게시된 실제 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`을 native Catalog/ValtanBrain으로 다시 읽어22개 판정 검사를 모두 통과했다. 별도 fixture 데이터만으로 제품 게시 성공을 대신하지 않았다. 근거는 `gameplay-publish.log`, `published-native-contacts.log`, `presentation-generation.json`이다.

공식 전체 Product Debug와 Release 모두 PASS했다. 근거는 각각 `out/BuildPipeline/runs/20260928T000044508Z-debug-product.json`, `20260928T000856060Z-release-product.json`이다. Engine/Shared/Server/Client의 일반 Build와 실행 파일·DLL·shader 배포를 수행했고 SkipBuild는 사용하지 않았다. 두 구성의 tracking identity도 유지됐다. 기다리는 동안 별도 Release FxCompile target과 Shared/Server를 먼저 빌드했으며, 이후 공식 Product Build가 필요한 갱신·링크·배포를 수행했다. 기존 source encoding/외부 PDB 및 원본 shader 경고는 남아 있으며 오류는 없었다.

두 구성의 receipt 모두 missingRuntimeInputs/invalidRuntimeInputs가 빈 배열이다. 별도 Product compiled-shader-closure도 Debug/Release 각각 active producers248/Client consumers163, 실행 Effect consumers142와 resource-root8cases를 확인했고 WARP pixel V1/V2 각각1352건을 통과했다(`shader-debug-closure.log`, `shader-release-closure.log`). 이는 headless Effect probe이며 실제 Arena 카메라에서의 유령 피부·컷씬 FPS 확인과는 구분한다. Client/UI를 자동 실행하지 않았으며 실제 화면·FPS 확인은 사용자 범위다.

최종 기능 HEAD `b91fb87d79fb840a4551f6828c805789b0b39898`를 push하고 독립 검토했다. 네 PR ancestry, 수동 충돌 두 문서의 의미 보존, 프로젝트 등록, Native1532/Ghost84 병존, Engine/Client shader mirror7쌍, contact 검토 hash를 확인했으며 추가 actionable 결함은 없었다. 55개 관련 파일의 SHA와 Git blob은 `final-review.json`에 있다. Release guard 확인에서도 실제 Server 판정·유령 재질·발자국·피자 제품 경로의 누락은 없었다(`release-contract-review.md`). 이 구조 검토를 실제 Release 빌드 성공으로 대신하지 않는다.

## G04. Git 반영 기록

main 반영 창구는 [PR #471](https://github.com/tnestyle70/LostArk/pull/471)이며, 검토한 [#472](https://github.com/tnestyle70/LostArk/pull/472), [#473](https://github.com/tnestyle70/LostArk/pull/473), [#474](https://github.com/tnestyle70/LostArk/pull/474)의 원래 HEAD를 모두 포함한다. 제품 검증 기준은 위 기능 HEAD이고 마지막 문서 commit은 이 결과만 갱신한다. 원격 병합의 완료 상태와 merge SHA는 각 PR의 GitHub 병합 기록을 따른다. 로컬의 병합 후 tree/ancestry 확인 기록은 `out/ValtanIntegration20260928/merge-verification.json`에 남긴다.
