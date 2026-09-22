# 쿠크 1관문 패턴·카드비·오디오 구현 계획

## G00. 최신 저장본과 이전 변경 설치

사용자가 실제 반영을 승인했다. 이전 Clown·MAZE·Ctrl 핑·Effect 삭제·14그룹 화염파동의 검증된 후보23파일을 최신 저장본에 stable ID/field로 병합했다. 사용자 추가 패턴100~103과 무관한 편집을 보존하며, 이후 후보도 기존값 검사·백업·원자 교체를 사용한다. Client/UI를 자율 실행·조작하지 않는다.

## G01. 관문 오디오와 원본 패턴 리소스

Bern·Valtan은 이미 지정된 원본 WAV 경로를 소비하므로 실제 설치와 진입/연출 종료 시점을 대조한다. 쿠크는 기존 ready terrace BGM owner를 관문·Mario·MAZE·Bingo 상태까지 확장하고 시퀀스/컷씬 재생과 겹치지 않게 한다. 원본 Action/SoundCue/GameMsg의 경로·시간으로 바람방구, 카드비, 배송, 도넛, Dice 사운드를 연결한다.

카드비·회전카드·Dice·무력화는 설치된 원본 Cascade 문서와 native material을 재사용한다. UI는 기존 screen image Effect 경로를 소비하고 원본 portrait/frame과 텍스트를 결합한다. 백스텝은 원본 TrailGhost/기존 NPC afterimage 경로를 확장하며, 7개 MAP 화염링에 G1 중앙−G3 중앙의 실제 좌표 차이를 적용한다.

## G02. 카드비 duration과 서버 크기 권위

`project_kouku_saydon_composition.py`, `KoukuSaydonCompositionDocument`, `KoukuSaydonActionWorkbench`의 기존 SHOWTIME random volley 계약을 확장한다. fixed/tracking 표식 없이 random set만 가진 duration도 허용한다. 선택한 Effect group의 장판과 카드 낙하를 함께 투하하고, 원점은 현재 세이튼 또는 기존 spawn 기준으로 명시한다. 기존 쇼타임 기본값은 유지한다.

Server `GameRoom_BossSimulation`이 fixed tick에서 결정적 위치·크기를 결정하고 기존 CombatObject transaction으로 생성한다. 1~2배 배율은 최초 spawn에서 확정해 late join/retry에도 같은 값을 전달하며, 타격 shape도 같은 배율로 계산한다. Shared spawn message와 Client targeted presentation이 이 불변 배율을 전달·소비한다. protocol version과 reader/writer 검증을 함께 갱신한다. nav 밖 샘플은 거부하며 빈 방/실패에서 밀린 투하를 한 번에 발생시키지 않는다.

## G03. 정면바람방구·병정·추적·1관문 흐름

P100의 원본 공격 시점/범위에 collider와 기존 ballistic knockdown 결과를 연결한다. 카드비 병정은 원본 NPC와 기존 카드미로 archetype의 일치를 확인한 뒤 기존 Server spawn 계약을 사용한다. P101에 BOSS_TRACK_TARGET의 이동을 연결하고 별도의1초 추적 패턴을 추가한다.

1관문 flow는 요청한 순서를 그대로 저장한다: 화염파동→추적→백스텝 화염링→추적→공굴리기→추적→비둘기→추적→트럼펫→추적→방구→추적→돌진→카드비→추적→회전카드→추적→공먹기→Dice→1초추적→무력화→1초추적→진짜세이튼→1초추적→댄스→1초추적→룰렛→1초추적. 기존 flow 반복 소비자를 사용하며 다른 관문 flow는 보존한다.

## G04. 게시·빌드·전달

구조·source dependency·동적 투하와 피격의 실제 소비자를 검증하고 해당 Kouku domain publisher로 실행 데이터를 생성한다. 수정 C++/Shared/셰이더와 Product Debug 빌드를 확인한다. 새 C++ 파일을 만들 경우에만 project/filter 항목을 추가한다. 신규 Effect Data는 기존96.DataFiles None 항목으로 등록한다.

새로 설치한 리소스와 추가 필수 의존성은 Desktop/GBResources의 기존 최상위 Character/Effect/Sound/UI 상대 폴더 양식을 유지해 병합하고 README를 갱신한다. 기존 전달 파일은 삭제하지 않는다. Client 화면·실제 청취는 사용자 확인으로 남기며 설치·publish·build와 구분한다.
