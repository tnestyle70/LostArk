# 쿠크 거미 Play Pattern 상태와 공포 사운드 구현 계획

## G00. 현재 경계

P15 거미 카운터의 Play Pattern은 Workbench에서 요청을 큐에 넣은 직후 고정 안내를 표시한다. 실제 Gate·리소스 준비는 MainApp과 BossTool이 수행한다. P15는 completion-chain/Mario 전용 `Follows_ServerClock` 조건에 해당하지 않아 Workbench가 서버 상태를 추적하지 않는다. 준비 실패 역시 MainApp의 별도 상태에만 남는다. 게시 성공과 실제 Server admission을 구분해 같은 도구에서 최종 상태를 볼 수 있어야 한다.

작업 기준은 `979601d0f62fc60265457678c192fbb37ccf68b7`, 작업 브랜치는 `codex/spider-pattern-fear-sound`다. 시작 시 clean이었으나 사용자가 실행 중 Composition을 저장하고 있으므로 최신 저장본을 보존한다. Client/UI는 실행하거나 조작하지 않는다.

## G01. Workbench와 MainApp

`Client/Public/KoukuSaydonActionWorkbench.h`의 준비 상태 전달에 실제 결과 문자열을 함께 받는다. `Client/Private/MainApp.cpp`는 클릭 직후 준비 시작/거절과 비동기 준비 중/종료 결과를 Workbench에 전달한다. `Request_SelectedServerPlay`는 모든 명시적 Pattern 재생 요청을 추적하고, 일반 local Preview를 Server 재생으로 바꾸는 `Follows_ServerClock` 정책은 유지한다. 요청 enqueue를 서버 송신 완료로 표현하지 않는다.

실제 게시 P15는 기존 Server GameRoom admission 검증을 out 아래 격리하여 조사한다. 재현된 실행 차단만 수정하며 최신 revision 검사와 Server authority를 유지한다.

## G02. 공포 결과의 SOUND 연결

`KoukuSaydonCompositionDocument.h`의 FEAR result에 optional `soundResourceId`를 추가한다. 기존 codec의 읽기/쓰기/검증, Workbench의 FEAR 편집과 clipboard 참조 수집, Python projector의 FEAR projection을 함께 연결한다. 참조는 SOUND 리소스만 허용하며 다른 result kind에는 허용하지 않는다.

`KoukuSaydonPresentationPlayer.cpp`는 게시된 optional soundResource를 같은 FEAR session의 SOUND occurrence로 만든다. 시작 시각은 얼굴과 같은 `effectDelayMs`, 종료 상한은 Server FEAR 수명이다. 기존 SoundCue 재생·seek·pause·stop과 local player FEAR 식별자를 사용해 snapshot마다 다시 재생하지 않는다. 새로운 오디오 런타임이나 Server asset 경로를 추가하지 않는다.

원작 FEAR buff는 화면 particle과 postprocess를 참조하며 직접 AkEvent는 없다. 실제 설치된 적합한 쿠크 사운드를 확인한 후 얼굴에 연결하는 정책은 사용자 요청에 따른 PROJECT_TUNED로 기록한다. 원본 얼굴 effect, 돌진 방향과 collider 저작값은 유지한다.

## G03. 검증과 최종 반영

기존 publisher focused test에서 optional 부재 호환, 유효 SOUND projection, 잘못된 kind·missing ID·non-FEAR 참조 거절을 검사한다. 변경 C++은 기존 인코딩/줄끝을 유지하고 현재 Product toolchain으로 컴파일한다. 새 제품 C++ 파일은 없으므로 project/filter 등록 변경은 필요하지 않다.

후보가 완성되면 현재 디스크 저장본 기준으로 `kakulsaydon.g1.logic.35`의 사운드 참조만 병합한다. 편집 중 데이터 반영 절차에 따라 저장·반영 의사를 한 번 확인하고 hash 재검사·백업·원자 교체를 유지한다. 해당 domain publisher로 생성하고 변경 JSON parse와 `git diff --check`를 실행한다. 실행 중 EXE의 교체는 파일 점유를 별도로 확인하며, 실제 Play Pattern·피격 얼굴·청취 판정은 사용자 확인으로 남긴다.
