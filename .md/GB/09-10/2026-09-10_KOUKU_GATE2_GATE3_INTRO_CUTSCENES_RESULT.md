# 2관문·3관문 시작 컷신 원본 조사 및 상세 인계 결과

날짜: 2026-09-10.
대응 PLAN: 2026-09-10_KOUKU_GATE2_GATE3_INTRO_CUTSCENES_PLAN.md.
사용자 요청: 첨부 두 영상을 기준으로 원본 데이터와 현재 프레임워크 적용 방법을 매우 상세한 TXT로 작성. 제품 구현 요청은 아님.

## 수행

- 세션 LAN sync: client, 192.168.0.14:7777 not-listening. 로컬 설정 완료, endpoint 임의 변경 안 함.
- 기존 브랜치 codex/kouku-card-maze-0908 및 대규모 dirty worktree 확인. 자동 stage/commit/push와 사용자 파일 복구 없음.
- 첨부 영상 오프라인 디코딩/프레임 분석. Client/UI 실행·조작·화면 캡처 없음.
- 실제 UPK 806의 SCENE04A/interpdata_2(27초/98group)와 8M6의 SCENE02A/interpdata_10(약35.368초/67group) 재열람.
- 활성 Director 5/13shot, fade, slomo, 주요 animation/reverse/LookInfo 재확인.
- MN_RPCT_05 249clip, MN_RPCT_06 34clip, MN_RPCZ_00 91clip의 실제 WModel 내부 목록 열람.
- table/book/paperstage PSK/PSA 원본 존재 확인, 현재 deploy 모델/clip 연결과 차이 조사.
- 현재 WorldSequence/Composition/Camera/Server gate-entry 및 F1 viewer 소비 경로 검토.
- G00~G18 상세 본문과 원본 165group actor 인벤토리 작성.
- local.md에 따른 독립 비평 후 실제 코드로 재확인하여 GATE2/05 잘못된 combat join 방지, Debug_ActivateGate의 비transactional 한계, 내부 shot blendIn/out=0을 보강.

## 검증과 경계

- 조사 raw JSON 두 개 parse 성공.
- 두 문서 UTF-8/빈 파일 아님/줄 끝 공백 검사 성공. 최초 전달 PLAN은 1,058줄, 86,116bytes였으며 아래 추가 안내로 분량이 증가했다.
- git diff --check 광역 검사는 기존 dirty LFS 파일의 .git/lfs/tmp 쓰기 권한 오류로 실패. 이 문서 범위 diff 검사는 성공했으나 untracked 문서 내용 검증은 별도 수행한다.
- git fetch는 .git/FETCH_HEAD 권한 오류로 실패. main 최신화 성공으로 기록하지 않는다.
- 제품 C++/JSON/Resources 변경 없음. compile/publish/Server 테스트는 해당 없음, 실행 안 함.
- 모든 particle emitter, Kismet 의미, A/B weight, 원본 camera aspect default 완전 해독은 아님. 문서에 사실/해석 한계/프로젝트 적용안을 분리.
- 프레임워크에서 두 컷신 재생 및 4인 동기화·최종 화면 확인은 아직 수행하지 않음.
- 문서는 구현 지침이며 전체 C++ 교체 패치가 아니다. 후속 구현자는 현재 변경을 반영한 G별 최종 코드와 actual consumer/test를 한 단위로 작성해야 함.

## 전달

사용자 지정 C:/Users/USER/OneDrive/바탕 화면/2관문 시작 3관문 시작 컷신.txt에 PLAN 본문을 UTF-8 그대로 복사했으며 SHA-256 동일 확인 성공. 이 SHA는 문서 복사 확인일 뿐 Resource 배포 manifest가 아니다.

사용자 후속 요청에 따라 문서 앞부분에 완전성과 검증 범위 안내를 추가했다. 확인한 사항, 남은 네 가지 항목, 전체 C++ 교체 코드가 아닌 상세 지침이라는 점과 앞선 안내의 한계를 명시했다. 기존 TXT와 PLAN의 동일 여부를 먼저 확인한 뒤 갱신했으며, 갱신 후에도 복사본 SHA-256 일치를 확인했다.
