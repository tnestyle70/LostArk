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

1관문 flow는 요청한 순서를 그대로 저장한다: 화염파동→추적→백스텝 화염링→추적→공굴리기→추적→비둘기→추적→트럼펫→추적→방구→추적→돌진→카드비→추적→회전카드→추적→공먹기→Dice→1초추적→무력화→1초추적→진짜세이튼→1초추적→댄스→1초추적→룰렛→1초추적. 마지막 1초추적이 완료되면 다시 화염파동으로 돌아가야 하며, 다른 관문 flow는 보존한다.

## G04. 게시·빌드·전달

구조·source dependency·동적 투하와 피격의 실제 소비자를 검증하고 해당 Kouku domain publisher로 실행 데이터를 생성한다. 수정 C++/Shared/셰이더와 Product Debug 빌드를 확인한다. 새 C++ 파일을 만들 경우에만 project/filter 항목을 추가한다. 신규 Effect Data는 기존96.DataFiles None 항목으로 등록한다.

새로 설치한 리소스와 추가 필수 의존성은 Desktop/GBResources의 기존 최상위 Character/Effect/Sound/UI 상대 폴더 양식을 유지해 병합하고 README를 갱신한다. 기존 전달 파일은 삭제하지 않는다. Client 화면·실제 청취는 사용자 확인으로 남기며 설치·publish·build와 구분한다.

## G05. Gate 1 마지막 항목 이후 반복 보완

후속 설명을 위해 실제 소비자를 다시 확인한 결과, 기존 Server wrap은 BINGO만 지원하고 Client Play_Flow는 한 번 완료 후 종료했다. G03의 순서 설치만으로 Gate 1 반복이 구현된 것은 아니므로 이 누락을 보완한다.

`Server/Private/GameRoom_KoukuRaidFlow.cpp`의 기존 관문별 반복 조건에 GATE1을 추가한다. 완료 lifecycle과 epoch가 일치하고 마지막 wait가 지난 뒤 첫 entry를 시작한다. `Client/Private/KoukuSaydonPatternAuditionService.cpp`도 GATE1만 마지막 entry 완료·wait 후 index 0을 재요청한다. Stop·거절·중단·world/revision 변경은 기존 종료 경로를 유지한다. 다른 관문의 기존 동작과 gate clear 처리는 보존한다.

저장된 28개 entry와 revision 2194, Resources, packet, 프로젝트 등록은 변경하지 않는다. 두 기존 CPP의 최소 컴파일과 두 주기 순서·마지막 wait·종료 경로·다른 Gate 보존을 검사한다. 최종 제품 실행 파일 교체는 앞서 보고한 출력 점유 경계를 따른다.

## G06. 설치 이펙트의 실제 소비자 재검토

사용자가 요청한 전체 이펙트를 현재 설치된 stable asset ID, animation/Pattern occurrence, source material 및 GPU carrier까지 다시 대조한다. 이전 CPU finite 검사와 리소스 존재 검사만으로 실제 표시를 통과했다고 기록하지 않는다. Clown·MAZE·핑·삭제, 화염파동·Dice·무력화·백스텝, 카드비·회전카드·DJ를 독립 검토하고 발견한 누락은 기존 경로 안에서 보완한다.

무력화 별 선의 native3008은 C++ sprite admission과 달리 셰이더 생성 guard가 particle carrier를 배제했다. `install_kouku_gate1_native_shaders.py`에서 해당 material/PS identity에만 sprite와 ribbon을 함께 허용하고 동일한 세 generated leaf를 갱신한다. 원본 PS 식·상수·텍스처와 ribbon 경로를 보존하며 실제 particle 셰이더의 수정 전후 headless draw를 비교한다.

회전카드 P48의 6개 occurrence와 서버 projectile 표현, 이펙트 내부 원본 타임라인의 잘린 후반부를 대조한다. 변경이 필요하면 사용자 저작 행과 현재 저장본을 보존하는 필드 단위 병합으로 처리하고, 기존 Gameplay/Product publisher로 소비자를 다시 게시한다. 최종 Product 빌드와 관련 Server 계약 검사를 수행하되 사용자 Client 화면 판정은 별도로 남긴다.

카드비 Projectile421980301의 원본 개별 폭발음은 각 투하 birth+1350ms이다. 기존 random volley template에 유한 MAP SOUND를 함께 저장·게시·소비할 수 있게 projector, Composition validator, Workbench 선택항목과 targeted presentation reader를 연결한다. 최소 한 개 MAP EFFECT가 위치 원점을 소유해야 하며 SOUND 단독·BOSS 부착 SOUND·추적 루프 SOUND는 허용하지 않는다. 기존 `Sample`의 SoundCue 시작/seek/종료를 그대로 사용해 늦은 입장도 현재 나이에서 한 번만 재생한다.

사용자가 지정한 1관문 음원 `Sound/KoukuSaton/S_BGM_COMMANDERRAID/midnightc_ed__398225682.wav`로 관문 selector를 교체한다. 이전에 선택한 M03와 구분해 전달 manifest도 갱신한다. 화염파동은 1관문 14그룹의 전체 중심·방향을 유지하면서 가로3.5m/행간3.031m로 줄이고, 5352ms에서 잘리던 불바닥을 기존 independent tail 경로로 끝까지 유지한다. 백스텝 Tool preview도 기존 실제 모델 afterimage 경로에 연결하며 pause/seek/stop과 Product 시계를 구분한다.
