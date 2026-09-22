# 쿠크 2관문 이펙트·오디오·진행 순서 복원

## G00. 현재 저장본과 적용 범위

작업 시작 저장본은 Composition revision2197이다. P105 나팔과 P106 저글링은 애니메이션만 있고 presentation 행은 비어 있다. P8/P9 등장과 P10/P11 파1빨2는 기존 bundle을 사용한다. 다른 세션의 C++·shader·EffectResourceTree 변경 및 사용자 저작 TRS는 보존한다. 사용자가 후속 지시에서 3갈레바닥장판폭발은 P105 `쿠크_나팔액션`과 같은 패턴이라고 확정하고 전체 이펙트·사운드 연결 및 반영을 요청했다. 별도 3갈레 Pattern을 만들지 않는다.

데이터 후보는 out/Gate2Restoration20260922 아래에 먼저 준비한다. 반영 승인은 받은 상태이므로 재확인하지 않는다. 기존 Effect와 Composition의 stable ID/field를 기준으로 최신 저장본에 병합하며 hash 재확인·백업·원자 교체·자기 변경 rollback을 유지한다. 변경된 project/filter도 최신 파일의 필요한 None 항목만 병합한다.

## G01. 원형·여섯 방향 폭발과 나팔

원본 Action4219713의 Att_Battle_3_06에서 notify011의 B_02 원형과 활성 notify020/021/022의 bilateral B를 독립 문서로 묶는다. 원본 FRotator yaw45/90/135도에 snapshot root basis -90도를 한 번 적용하고 원형0.944024980초→직선1.124565005초의 상대 시차0.180540025초를 보존한다. 비활성 notify019와 별도 도넛 Action4219715의 D_02는 섞지 않는다. 원본 leaf의 개별 TRS·TypeData degree·MeshRotation turn과 P23은 보존한다. P105의 저장된 clip/sourceStart/playRate를 기준으로 기존 노란 경고 리소스·전방 팡파레·원형/6방향 폭발·사운드를 연결한다. 전방 D_Music은 직접 notify가 없는 저작 재사용으로 구분한다.

## G02. 저글링과 세이튼 등장

원본 Action4219716의 세 투척(2.2/3.7/5.2초), 손의 공과 Hit, Projectile421971601의 Jugle_02→Jugle_Exp 연결을 사용한다. 포물선 마지막 위치에서 폭발하도록 기존 event/element 경로를 소비하며 원본 크기와 새 저작 궤적을 구분한다.

등장은 Action4219719의 기존 줄타기 이동과 원본 Circusbomb trail, Projectile421971901의 TimePrj→Circus_Exp를 연결한다. 리소스 트리는 `패턴 | 세이튼등장 | 쿠크줄타기트레일`, `패턴 | 세이튼등장 | 세이튼등장하강폭발공`으로 정리한다. title은 기존 SUBTITLE 경로에 요청한 두 줄을 저장한다.

## G03. 파1빨2의 노란 시선과 파란 반투명 구체

기존 수동 광선·폭발을 중복 추가하지 않고 원본 눈 예고 요소만 분리해 실제 설치 모델의 눈 소켓에 연결한다. 현재 generic additive 파란 돔은 원본 G safezone의 native3270/3271 반투명·SceneColor 경로를 사용하는 후보로 교체한다. 안전존의 서버 판정 반경과 시각 크기를 함께 확인한다.

## G04. 2관문 순서와 후반 반복

요청한 초기 순서와 후반 반복을 기존 patternFlows로 저장한다. 기존 끝 반복은 GATE1/BINGO만 지원하므로 선택적인 stable `loopStartEntryId`를 codec·저장·projector·게시·Server와 Client 진행 소비자에 연결한다. 미지정 문서는 기존 동작을 유지하며 잘못된 entry 참조는 거부한다. 반복 진입은 기존 epoch·완료 receipt·wait·Stop/거절/관문 이탈 계약을 유지한다. 상세 변경은 대응 GATE2_FLOW_AUDIO 문서에 둔다.

첫 행과 등장 뒤의 나팔 행은 같은 P105를 참조한다. 기존 두 저글링 clip은 보존한다.
후반 반복은 사용자 확정대로2번 내려치기→3번 내려치기→휠윈드→팡파레→수퍼바주카→불뿜기다.
전체 선택 패턴과 bundle child의 원본 Action 사운드는 읽기 전용으로 대조한다. 사용자의 후속
정정에 따라 휠윈드·피자·파1빨2·바주카는 이미 사용자가 넣은 사운드를 그대로 유지하며 원본
cue 수나 시각이 다르다는 이유로 보충하지 않는다. 추가 Action 사운드는 원래 요청된 빈
P105/P106과 바람 P99에 한정한다. 등장 공의 발사·폭발음도 기존 대상 지정 소비 경계로 연결한다.
바람불기와 불어날리기는 원본 action/clip을 대조해 같은 동작인지 확인한 뒤 흐름을 연결한다.

## G05. 검증과 전달

기존 Effect Codec/Playback으로 후보의 실제 요소·크기·시계·endpoint·native 재질 admission을 검사한다. JSON/XML parse, 해당 publisher, 필요한 증분 Product Debug Build와 git diff --check를 수행한다. 새 C++ 파일은 예상하지 않으며 새 Data 문서는 Client 96.DataFiles None 항목에 등록한다. Source/Resources/게시 데이터/실행 중 메모리/사용자 화면 판정은 각각 구분한다. Client/UI 실행·조작·캡처는 하지 않는다.

최신 Composition revision을 고정한 원자 설치 뒤 `Invoke-BuildDomainOwner.ps1 -Owner
KoukuSaydon -ExpectedKoukuSaydonSourceRevision <실제 새 revision>`으로 Client/Server 소비
문서를 게시한다. 설치된 모든 Effect/Sound asset과 초기·반복 flow, title을 실제 게시물에서
확인한다. Server 프로세스나 편집 도구의 메모리 Reload는 자동 수행하지 않는다.

사용자의 후속 요청에 따라 Play Pattern의 저장/게시 revision·resource closure와 Complete
Play의1관문→2관문→3관문→빙고를 검증한다. 실제 설치 데이터를 격리한 기존 Server
GameRoom 계약 테스트로1~4인 입장·클리어·다음 관문 승인·앙코르·빙고를 검사하고,
새2관문25개 entry의 초기 순서·후반 반복은 실제 완료 전이 코드로 별도 대조한다.
이 무창 검증을 플레이어의 모든 기믹 성공이나 실제 GPU 화면 확인으로 기록하지 않는다.

검사 중 발견한 P85 STAGE_3의 `retargetTarget: NEAREST_ALIVE` 저장 계약 불일치도 해결한다.
Python publisher가 허용한 필드를 Client codec이 unexpected property로 격리하므로,
기존 Stage struct·검증·Parse/Serialize·실제 타깃 소비자를 같은 optional 계약으로 맞춘다.
사용자가 저장한 P85 stage나 사운드를 제거해 검증을 우회하지 않는다.
