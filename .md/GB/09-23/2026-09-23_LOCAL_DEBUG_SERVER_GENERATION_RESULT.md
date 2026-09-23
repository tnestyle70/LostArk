# 로컬 F5 서버 시작 복구

## 원인과 반영

기존 Debug Server는 최신 게시본의 PATTERNPARENTCHILD 행을 읽지 못해
`Unknown gameplay bootstrap row kind`로 listener 생성 전에 종료했다.
주소 변경이나 VS 재시작만으로 해결되지 않는 코드·데이터 세대 불일치였다.

기존 브랜치를 main 26e5fc72(PR #451 포함)로 fast-forward했다.
겹치는 로컬 파일은 stash 70791f88b759af28ab1b06a15540460f6f3014f6에 보존했다.
복원 중 생성물 Encounter/Gameplay.bootstrap만 충돌했으며 로컬본을 복원하고
공식 Kouku projector와 Gameplay publisher로 재생성했다. 중복 untracked 5개는
incoming 파일과 Git 내용 비교가 동일했다. 백업 stash는 삭제하지 않았다.

Client/Server 로컬 debugger는 127.0.0.1을 사용한다. 공유 팀 endpoint는 변경하지 않았다.

## 실행한 검증

- Debug Product Engine/Shared/Server/Client 빌드 성공.
  증거: out/BuildPipeline/runs/20260922T214657685Z-debug-product.json.
- Kouku publish: revision2222, 저작119개/실행113개 유지.
- Gameplay Publish 성공. 첫 시도는 병행 서버 계약 검사의 파일 잠금으로 실패했고,
  검사 종료 뒤 단독 재실행 성공. 데이터 행을 삭제하거나 reader 검사를 우회하지 않았다.
- Debug Server --kouku-bundle-contract-test: failures 0.
- 발탄 컷신 연결 구조 검사 및 publisher landing-frame 10개 사례 통과.
- test_valtan_presentation_generation.py: 13개 통과.
- 최종 게시본의 발탄 패턴70개와 전환 대기69개를 원본과 대조하여 모두 일치.
- XML parse 및 git diff --check 통과. Git 미해결 충돌 없음.
- 최종 게시 후 Debug Server --headless --bind-address 127.0.0.1 --port 7777
  --smoke-timeout-ms 1500: 실제 Listening 출력, exit0.

## 사용자가 확인할 항목

VS에서 Debug/x64 및 Server + Client 시작 프로필을 선택하고 F5 실행한다.
Client Only는 별도로 Server가 실행 중일 때만 사용한다.
서버 테스트는 제한 시간 뒤 종료했으며 상주 서버나 Client UI를 실행하지 않았다.
실제 맵 입장·화면·사운드 확인은 사용자 검증으로 남긴다.
기존 인코딩 컴파일 경고는 남아 있으나 이번 빌드 오류는 없었다.
추가 commit/push는 수행하지 않았다.
