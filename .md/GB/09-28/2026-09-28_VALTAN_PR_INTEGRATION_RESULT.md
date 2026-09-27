# 발탄 수정·PR 471~474 통합 결과

## G00. 검토와 현재 경계

원래 open PR #471~474의 HEAD를 고정하여 검토했다. #471은 현재 작업 브랜치의 LAN endpoint·F1 체력바 변경, #472는 마하라카 복원·MapTool, #473은 수리창·내구도 HUD, #474는 에스더 NPC action cue 복원이다. 네 PR을 현재 Codex 작업에 첨부했다. 읽기 전용 검토 결과와 HEAD는 `out/OpenPrReview20260928/heads.json`, `review.json`, `json-review.json`에 있다. 아직 최종 통합 HEAD의 build/merge 성공을 뜻하지 않는다.

#473의 `CDurabilityHudView::Apply_Part`는 직전 적용한 width를 원본 damaged width로 나누므로 DESTROYED 상태에서 매 Update마다 크기가 누적된다. helmet의 원본 layout width 10은 한 번 적용할 때 10.6667이어야 하지만 60 update 뒤 512.569가 된다. 고정 authored scale 보존으로 수정할 후보를 준비 중이다. 다른 원래 HEAD의 검토에서 추가 actionable 결함은 발견하지 못했다. #473은 내구도 Server 상태·실제 수리 처리를 후속 범위로 명시하며 이번 통합에서 구현됐다고 주장하지 않는다.

## G02. 실제 Resources 반영

### G01 복구 돌 폭발 반영

피자 runtime explode, 독립 편집 composite explosion.full, ground-roar explode의 debris 표면을 생성 돌과 같은 native2391로 교체했다. 기존 fragment mesh, 모든 Element ID/transform/timing/색/운동/수명, 16개 burst와 사용자 검은 파동·전조는 보존했다. 기존 dissolve 구간에 맞춘 필수 dynamic 4채널만 연결했다. ground-roar effect를 공유하는 part-break도 같은 표면을 사용한다. 새 Resources나 shader 변경은 없다.

실제 Codec/Playback와 native2391 parameter binding으로 3문서×9시점 27행, 실제 입자 441개의 count/World/velocity/color/age가 이전과 동일함을 검증했다. 끝난 debris는 기존 1.2초 수명 뒤 사라진다. 두 설치 WModel의 실제 GPU buffer admission도 통과했으나 pixel draw나 실제 화면 검증은 하지 않았다. canonical writer admission·입력 hash 재확인·백업·원자 교체를 통해 3문서를 반영했다. 근거는 `out/ValtanRestoredRockExplosion20260928/installation-receipt.json`, `native.log`, `geometry.json`이다.

발탄 변경 리소스 3개 42,602,616 bytes를 `C:/Users/user/Desktop/GBResources2`에 Resources-relative 경로 그대로 복사했다. `Character/Valtan/Ghost/MN_RPBF_02.wmodel`은 기존 모델의 원본 normal/tangent 복원 교체본이며, 에테르의 `fx_a_line_002.dds`, `fx_c_atypical_003.dds` 두 파일은 신규다. 설치본과 복사본의 bytes/SHA-256이 일치한다. 근거는 `out/ValtanIntegration20260928/resource-delivery.json`이다. Data JSON·shader source·EXE는 이 폴더에 섞지 않았다.

#472의 새 리소스 일부가 현재 Client Resources에 없어 다운로드 폴더의 `CY_Resources`를 조사했다. PR 작성자가 기록한 3730 files/1,241,075,852 bytes와 정확히 일치했다. 같은 상대 경로로 624개 249,882,676 bytes를 추가·교체하고 기존 3106개는 bytes 동일하여 유지했다. 교체 전 파일을 `out/ValtanIntegration20260928/resource-before`에 보존했으며 원자 교체 직전 최신 bytes와 교체 후 bytes를 검사했다. 근거는 `maharaka-resource-install.json`이다. 마하라카 리소스를 발탄 전달 폴더 GBResources2에 섞지 않았다.

incoming JSON에서 확인한 712개 물리 참조 중 54개의 신규 누락을 이 전달본으로 해소했다. 별도의 `NPC_SHIP_SHIPWRIGHT`, `NPC_SHIP_HARBORMASTER` 모델 2개는 기존 HEAD의 NpcCatalog에도 같은 경로로 존재하는 설치 누락이며 이 PR의 신규 마하라카 자산이 아니다. 이를 신규 회귀로 기록하지 않으며 전체 게임 Resources 완전성을 보증하지 않는다.

## G04. 최종 통합 상태

콜라이더/돌 수정, 최종 domain publish, 통합 Debug/Release와 main merge의 실제 결과는 완료 후 이 절에 기록한다. Client/UI는 실행하지 않았으며 실제 화면·게임 FPS 검증은 사용자 범위다.
