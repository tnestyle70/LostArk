# PR #384 — 우리 시퀀스 정본 보존과 main 동기화

사용자 요청: 올라온 PR을 merge하고 pull하며, 완성된 우리 시퀀스와 다른 충돌 항목도 우리 브랜치를 기준으로 병합한다.

## 기준과 충돌 해결

- PR: `https://github.com/tnestyle70/LostArk/pull/384`, `koukusaydon effect, sequencer`.
- 우리 정본: `d970b2c111b022c4fc7f2fb3072922a4c965338f`, `codex/sequence-capture-focus`.
- 병합한 main: `c4dc51b02f8c206fc553f616618734942b6bbc07`.
- 시작 worktree는 clean이었다. 따라서 stash할 미커밋 파일이 없었고 `codex/safety-pr384-before-sync-20260914` 참조로 기존 HEAD를 보존했다.
- 충돌4개는 사용자 지시대로 우리 정본과 Git blob이 완전히 같도록 해결했다: `.md/GB/gotchas.md`, Sequence Composition, WorldSequences authoring, WorldSequences runtime.
- 우리 P3의 WORLD17개/CAMERA5개, P4의19959ms 전환, P8 첫 Stage4997ms를 보존했다. upstream의 추가 무대·카메라 Sequence 변경은 채택하지 않았다.
- 충돌 없는 main 변경인 탈것 UI, 카드미로 Server/Client, Engine 투명 정렬과 관련 데이터를 함께 반영했다. `GameObject::Get_BlendSortPriority`, Renderer 소비와 MapAssetObject override를 같은 계약으로 유지한다.
- upstream G15의 추가 소품 설치 기록은 과거 결과로 보존하고 현재 정본과 다른 점을 해당 RESULT 머리에 명시했다. 생성기 수정은 현재 시퀀스 재생성 지시가 아니며 생성기를 실행하지 않았다.
- 새로 유입된 카드미로 PLAN의 줄 끝 공백3개만 제거해 병합 diff 검사를 정리했다.

## 검증

- 변경 JSON11개 parse, Client 프로젝트/filters XML2개 parse 성공.
- 충돌4개 모두 기준 HEAD와 index blob 동일, 미해결 충돌0개.
- 별도 맵 배치/Deploy 문서는 `LOSTARK_*` 텍스트 형식이며 JSON으로 취급하지 않는다. authoring/runtime의 대응3개 파일은 byte 동일했다.
- `git diff --cached --check` 성공, 작업 파일과 index 사이 미반영 변경0개.
- Debug Product 증분 Build 성공: Engine4.931초(OBJ9), Shared0.379초(OBJ0), Server26.271초(OBJ51), Client75.755초(OBJ147). PCH·CSO 재생성0개이며 Engine/Server/Client 링크와 정상 Debug 배포를 완료했다.
- 실행 파일은 `Server/Bin/Debug/Server.exe`, `Client/Bin/Debug/Client.exe`다. 실제 실행·runtime 진단·Data publisher는 수행하지 않았다.
- 빌드 증거: `out/pr384-sync-product-build.log`, `out/BuildPipeline/runs/20260914T103312225Z-debug-product.json`.
- Client/UI 실행·조작·화면 캡처, 저작 리소스 재생성은 하지 않았다.

원격 PR merge 및 최종 pull 결과는 이 문서가 포함된 커밋 뒤 해당 대화에서 보고한다.
