# 발탄·쿠크 패턴과 Movie 수정 통합 결과

## G00. 진행 상태와 사용자 정정

현재 기능 통합과 검증을 진행 중이다. Debug/Release 최종 제품 빌드, PR 생성과 merge는
아직 완료하지 않았다. 최신 사용자 요청의 높이·빨간 장판·중앙 착지 대상은 피자가 아니라
추적 도끼다. 피자에 준비했던 이번 변경은 철회하고 기존 저장 상태를 보존한다.
3회 구르기 후 돌진은 정상 source cue/lifetime을 유지하며 벽 충돌 GROGGY에서 종료한다.

## G01. 창술사와 기존 Movie 저장

사용자 반영 승인 뒤 최신 ClassSelection.cinematics에서 Intro/Loop의 face 트랙 두 곳만
materialName/family/parameters를 정상 donor에 연결했다. 실제 Product parser와
WorldSequencePlayer/CMaterial을 사용하는 창 없는 WARP 검사 27개가 통과했다.
원래 오류와 이름만 수정했을 때의 program mismatch를 재현하고 최종 교정을 확인했다.
Save writer lock·hash 재확인·백업·ReplaceFileW를 사용했고 설치 후 SHA는
`865b6154fb550852414efc3008ecbd8107a5bcb6983241d599cee579711b7258`이다.
워로드25/도화가12/차원술사13개의 Movie 제외 목록과 모든 다른 필드는 보존했다.
상세 근거는 09-27 WORLD_MOVIE_HAIR_GUARDIAN_EYES_IMPLEMENTATION_RESULT G11이다.

## G02. 리소스와 기존 Effect 변경 검증

설치된 창술사 Appearance WModel4개는 GBResources와 이미 SHA가 같았다. 실제 WModel
material section에서 texture 참조를 읽어 donor 모델까지 26개 파일을 대조했고, 전달 폴더에
없던 기존 donor/texture22개(27,109,926 bytes)를 같은 상대 경로에 추가했다. 모델의 geometry나
원본 texture 내용은 바꾸지 않았다. Movie가 참조하도록 변경된 hdr07_1.tga와
brdf_beckmann_spec.tga도 전달본과 SHA 일치를 확인했다.
증거는 `out/RaidMovieIntegration20260929/lance-resource-delivery.json`에 있다.

현재 변경된 Effect8개에 공식 validator의 material color space, native particle,
module override, attachment orientation과 resource closure 검사를 실행했다.
참조244개/27,128,624 bytes를 읽어 PASS했다. 전체 Effect repository validator를
통과한 것으로 기록하지 않는다. 기존 다른 문서의 v15 carrier 문제는 기존 Mario RESULT에
기록돼 있으며 이번 변경 문서는 모두 v13이다.

## G03. 네트워크 검증

마리오 목표 색과 matching-ball 진행도 전달이 포함된 protocol121의
NetworkProtocolHarness를 Debug/Release 각각 증분 Build하고 실행했다. 두 구성 모두
failures0이다. 로그는 `out/RaidMovieIntegration20260929/protocol-*-build.log`와
`protocol-*-run.log`다. 실제 LAN과 Client 화면을 실행한 검사가 아니다.

## G04. 남은 통합 단계

발탄·쿠크 담당 변경과 focused 계약 확인 뒤 공식 projector/publisher, 최종 Product
Debug/Release, PR/merge 결과를 이 문서에 기록한다. 사용자 rendering option과 Retail
수치는 게시 전 현재 저장본을 별도 baseline으로 보존해 대조한다. Client/UI는 실행하지 않는다.
