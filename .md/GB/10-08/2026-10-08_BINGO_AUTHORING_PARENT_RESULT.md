# 빙고 전체 Parent와 공통 Logic 편집 결과

## 구현 상태

Composition의 `Bingo / Encore Saydon`에서 `Bingo Complete [Parent]`를 선택하면 저장된 빙고 Flow 전체를 편집한다. 공통 보드 Logic과 폭탄의 첫 대기·머리 표식·투하 대기·설치 fuse, 특수 Parent와 자식 구간, 처음 한 번 재생하는 Opening 및 반복 구간을 각각 표시한다. 전체 Parent는 기존 Server Raid Flow의 집합 뷰이며 별도의 중첩 반복 런타임을 추가하지 않았다.

공통 Logic의 Box Detail은 활성 모드, 첫 폭탄 지연, 표식 간격, 표식/투하/fuse 시간과 초기 검은 칸 수를 편집한다. ENCOUNTER 기본값은 첫30초·이후20초·표식6초·투하2초·fuse4초·초기2칸이다. WINDOW는 기존 Logic start/lifetime을 사용한다. 구간이 닫혀도 기존 폭탄은 캡처한 설정으로 완료되고, 보드 소유권을 남겨 일반/특수 패턴 전환 및 종료 구간에서 자동 재시작하지 않는다.

일반 box는 Box Detail에서 대기, Earlier/Later, Loop Start, Flow 제거를 편집한다. Append Pattern으로 같은 Bingo 관문의 패턴을 추가한다. 자식 Pattern/Bundle과 특수 Parent를 열면 기존 타임라인·Logic/Effect/World 등의 상세 편집을 사용한다. Back to Bingo Parent로 전체 흐름으로 돌아간다. 변경은 candidate validation, Undo/Redo, Save 경로를 사용하며 실패 시 기존 draft를 유지한다.

F1의 `KoukuSaydon Complete Play` → `Gate: 빙고` → `Saved Pattern Flow`는 전체 Parent, Common Logic, Special Patterns, Opening Patterns, Repeating Patterns를 구분한다. 전체 Parent 선택 후 Complete Play는 기존 Server admission·게시 revision 검사를 거친다. `Open Bingo Parent`는 기존 dirty draft를 보존하며 처음 열 때만 초기 문서를 준비한다.

## 실행 연결과 경계

Client codec → projector → 게시/초안 공용 bootstrap writer → Server catalog → 실제 보드/폭탄 state 순서로 일곱 설정을 연결했다. `PATTERNBINGOBOARD`는 기존 mechanic trigger에 설정을 연결하며 생략된 이전 데이터는 기본값으로 읽는다. `PATTERNBINGOCONTROL`은 publisher가 보드 전용 구조를 확인한 Pattern만 짧게 소비하게 한다. WINDOW를 길게 늘려도 일반 공격을 같은 시간 동안 멈추지 않는다.

긴 보드를 Enabled=false로 바꾸었을 때 trigger만 제거하면 긴 빈 Pattern 대기가 남는 결함도 수정했다. 저작 구간은 유지하고 Encounter와 animation binding 양쪽에서34ms 빈 carrier로 투영한다. 다시 활성화하면 원래 구간을 사용한다. 일반 빈 Pattern을 이름이나 형태로 Server가 임의 추론하여 생략하지 않는다.

초기 칸은0..25, fuse는250..80000ms다. 네 폭탄 slot 한도는 ms 합뿐 아니라 각 phase를30Hz tick으로 올림한 합도 검증한다. planted WORLD는 기존 playback speed로 fuse에 맞춘다. 새 protocol, Client 판정, 렌더링 옵션 변경은 없다. 칸 반전·빨간 줄 승격·보상과 특수 패턴 인터럽트는 기존 Server 로직을 사용한다.

## 자동 검증

- Debug Product 정상 runner PASS. 최종 보고서: `out/BuildPipeline/runs/20261008T101724460Z-debug-product.json`. 앞선 전체 변경 빌드는 `20261008T101231698Z-debug-product.json`에서64.048초·Client OBJ94·CSO0, 마지막 UI 버퍼 보완은 Client OBJ1·CSO0으로 증분 빌드했다. 기존 인코딩/숫자 변환 warning은 남아 있으며 컴파일·링크 오류는 없다.
- native `--kouku-fixed-damage-contract` 최종 PASS: 기본값 누락 호환, 비기본 WINDOW 왕복, 잘못된 type/mode/range/ms 및 tick 용량 거절, 공유 설정 Apply, Parent 선택, Flow 순서/대기/반복, Undo/Redo, Save/Reopen, 활성·비활성 보드 구간 확장 및 일반 Pattern 시간 보존을 검사했다. 로그: `out/bingo-native-build.log`, `out/bingo-native-contract.log`.
- Server `--bingo-contract-test` 최종176 PASS / 0 FAIL. 실제 게시 bootstrap을 사용하여 커스텀 시계, WINDOW 종료와 pending 폭탄, 초기 칸 수, 새 row 파싱·중복·잘못된 수치의 rollback, 긴 control의 일반 Flow 비차단을 확인했다. 로그: `out/bingo-authoring-server-contract-final.log`.
- Server `--kouku-raid-contract-test` 최종2071 PASS / 0 FAIL, exit0. 기존1~3관문, 빙고 준비·반복·특수 인터럽트·사망/엔딩, 참가자 승인과 전환 계약을 회귀 검사했다. 로그: `out/bingo-authoring-raid-contract-final.log`. 모든 CPU 검사가 종료되어 Server 프로세스는 남지 않았다.
- `Tools/KoukuSaydonPipeline/test_bingo_board_settings.py` 5개 PASS. 기존 tracking-bomb writer 1개, raid projection 16개 PASS. 실제 전체 저작본의 투영도 입력 불변 검사와 함께 통과했다.
- 정상 Kouku owner 최종 재게시 PASS,217.8초. `out/KoukuBingoPublish20261008/owner-republish.log`, `final-verification.json`에 기록했다. 저작 revision2500 및 SHA-256 `6c7ae0cb9e4ceb4ec663bb4de7352fd8f2d2e014beb9824b028f912681a5f3bf`를 유지했다. 설치된 Encounter/bindings/bootstrap도2500이며 새 board/control 행 각1개를 확인했다. 렌더링 관련9개 파일 hash는 모두 같다.

게시 결과의 변경은 Kouku Encounter, animation bindings, Gameplay.bootstrap 세 파일이다. bindings의 P1 Effect 시작2263→2257ms는 이미 저장되어 있던2500 정본의 값이며 이번에 저작 값을 바꾼 것이 아니다. source Composition을 덮어쓰지 않았다.

`git diff --check`와 변경/참조 JSON parse는 PASS다. 최신 `origin/main`(0164be5a9)에 대한 `git merge-tree --write-tree`도 충돌 없이 통과했다. 실제 main merge나 Client 실행은 수행하지 않았다. 기존 PR536은 main에 병합되어 이 변경은 빙고 기능만 분리한 PR이다.

## 사용자 화면 확인

Client를 실행하거나 UI를 자동 조작하지 않았다. 촬영 PC에서 F1 전체 Parent 선택, Box Detail 편집 후 Save → Publish All Patterns, 새 Complete Play의 화면·사운드·이펙트·폭탄 수명 확인은 사용자가 수행한다. 설치 파일 및 CPU 계약 검증을 실제 영상 확인으로 대신 기록하지 않는다.
