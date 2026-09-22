# 레이드 BGM과 쿠크 패턴 사운드 결과

## G00. 실제 반영 범위

`Client/Private/Level_KakulSaydonArena.cpp`, `Client/Public/Level_KakulSaydonArena.h`의
기존 Level 소유 Music 경로를 확장했다. 별도 C++ 파일이나 프로젝트 등록은 없다.
관문 전투는 Server raid `COMBAT`/`strGateId`, 마리오·미로는 유효한 비-preview local
player의 replicated mechanic 상태를 소비한다. 관문 1·2·3, Bingo, Mario 1~4, Maze가
같은 Music 채널을 교체하며 기믹 종료 뒤 관문 음악으로 돌아간다.

local sequence, 아직 승인되지 않은 combat 활성화, raid `CINEMATIC`, 해당 플레이어가
보는 재생 중인 sequence camera는 BGM을 중지한다. `COMPLETE`/`ABORTED`와 유효한
local player가 없는 경우에도 재생하지 않는다. 다른 플레이어의 camera와 완료 뒤
남아 있는 follow camera는 중지 사유가 아니다. 같은 snapshot, 같은 곡, 누락 파일은
매 프레임 다시 시작하지 않는다. 대기 구역은 기존 M12/2ndcircus 자산을 계속 사용한다.

공용 Composition과 설치 Resources의 최종 교체는 통합 담당이 수행한다. 이 담당은
`out/RaidAudio20260922/prepare_pattern_patch.py`의 `apply_sound_patch(current)`로 최신
문서를 메모리 병합하는 함수와 후보를 전달했다. 이 RESULT의 후보 제작은 live Composition을
쓰지 않았다. 이후 통합 적용·publish·Product 빌드 결과는 통합 RESULT를 따른다.

## G01. 원본 음원과 재생 대상

설치 게임 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC`의 package를
기존 `Tools/SoundPipeline/wwise_audio_package.py`로 sparse 추출했다. 원본 bank hash,
event action/target, media receipt, TXTP와 WAV hash는 `out/RaidAudio20260922`에 있다.

Wwise event의 Play만 보면 다음 구간을 미리 Pause하는 다른 음악까지 섞인다.
이번 관문 음원은 해당 event의 **Resume target**을 찾아 다음 원본 music container를
렌더했다. WEM을 이름만 보고 단독 연결하지 않았다.

| 후보 | 원본 event/target | 렌더 길이 |
|---|---|---:|
| gate1 | M03 battle splendidcircus / 920688328 | 113.498초 |
| gate2 | M07 battle satonsbook / 500458787 | 81.512초 |
| gate3 | M13 battle darkcircus / 278959898 | 125.084초 |
| maze | M09 battle maze / 753997496 | 88.338초 |
| bingo | M19 battle bingobox / 854059280 | 125.105초 |
| mario1~4 | M14~M17 sidescrolling 1~4 | 41.466/33.268/36.720/40.780초 |

원본 TXTP의 gain과 intro/loop 구간을 WAV `smpl`에 보존했다. Maze/Bingo/Mario는
첫 intro를 반복하지 않고 원본 loop 범위로 돌아간다. M13이 사용하는 559227263 media는
기존 ready 2ndcircus 파일과 같은 media ID다. 이름이 다르다는 이유로 다른 음원을
선택하지 않았다. 원본 Wwise의 전환 crossfade/state/RTPC 전체를 재구현한 것은 아니다.

`EFTable_GameMsg.db`의 `SOUND` 참조로 한국어 원본 안내음 `_02`(쏟아져라, 카드 비!),
`_10`(특급 배송 출발!)을 추출했다. WAV 길이는 6.134/7.862초다. `_01`(새로운 막이
올랐다 기대해!) 10.262초도 조사 후보에 포함했지만 패턴에 연결하지 않았다.
따라서 후보 WAV는 총 12개이고 실제 신규 연결은 11개다.

사용자가 말한 독립적인 짧은 ‘빠밤’ sting의 정확한 원본 event는 아직 특정하지 못했다.
M01 popup의 Resume target은 67.223초 음악이므로 이름만 근거로 짧은 sting으로
추정해 연결하지 않았다. 사용자 청취 판정과 정확한 sting 연결은 남아 있다.

## G02. 패턴 타이밍 후보

원본 `MN_RPCT_05`/`MN_RPCZ_00` LOA를 기존 extractor로 읽고, 현재 Animation
occurrence의 source-in/playRate/stage 길이에 맞췄다. 최초 후보(Composition revision
2193)는 SOUND 18개 추가, 기존 필드 12개 변경, 리소스 2개 추가다.

| 패턴 | 변경 |
|---|---|
| P100 정면 바람방구 | source action 4219873의 Cast1/ShotVox1(1ms), Cast2(850ms), Shot1(1900ms) 추가 |
| P103 카드비 | action 4219803의 Cast/ShotVox/Shot 6개와 원본 카드비 안내음 추가; 종료 stage Shot4는 저장된 stage 시작 기준 |
| P79 공먹기 | 원본 특급 배송 안내음 추가; circle/inner/outer impact SFX를 현재 visual보다 30/30/10ms 먼저 배치 |
| P78 Dice | 기존 준비·charge·폭발 SOUND 보존; card 발사 logic 9~12 시작+1ms에 Attack08 Shot1 네 개 추가 |
| P23 팡파레 | 기존 Shot8 녹음을 source 0~499/499~966/966ms 이후로 분리해 현재 세 impact 위치에 배치; 반복 Shot6 네 개를 현재 visual 시점에 정렬 |

P23 split 경계는 원본 particle 2.146537/2.645973/3.112187초 차이를 ms로 반올림한
499/966ms이며, Shot8 시작 2.150000초와 첫 particle 차이는 +3ms다. 마지막 네 Shot6의
visual 정렬은 저장된 사용자 timing을 따른 프로젝트 조정값이다.

P100의 원본 SkillEffect `421992501` row도 read-only 추출하여
`out/RaidAudio20260922/p100-source-skill-effect.json`으로 전달했다. 이동거리·시간·축의
런타임 변환이나 Server knockdown 연결은 통합 담당의 범위다.

## G03. 실행한 검증과 남은 경계

- 현재 소스의 실제 `Resolve_KoukuRaidBgmAsset`와 `Update_RaidBgm` 본문을 추출한
  native C++ probe: 31 checks, 0 failures. 관문/기믹 선택, 유효 player, sequence/camera
  억제, pending combat, 같은 snapshot 100회, 파일 실패 재시도 억제, 대기 run 변경,
  완료/중단 후 stale mechanic을 확인했다. 의존 API는 stub이므로 Client 실행 검증은 아니다.
- 설치 FMOD DLL의 `NOSOUND`에서 제품과 같은 `createSound` mode로 WAV 12개 로드,
  frame 길이와 loop 9개 범위 일치. 채널 재생 0개. WAV 수치 검증을 실제 청취 판정으로
  기록하지 않는다. receipt는 `fmod-resource-validation.json`이다.
- 베른 thecapital, 발탄 M06의 설치 파일 hash가 사용자 Desktop 원본과 각각 일치한다.
  베른은 Level 초기화 성공 뒤 재생하며 발탄 기존 `Update_RaidBgm`은 M05 entrance를
  끝낸 첫 승인 snapshot(IDLE 포함)에서 M06으로 전환한다. 중복 코드를 추가하지 않았다.
- 메모리 Composition 병합은 같은 후보에 두 번 적용해 무변경을 확인했고 P58 및 P100의
  무관 필드를 보존한다. 원본 파일을 교체하거나 편집 중 draft를 Reload하지 않았다.
- 변경 C++의 UTF-8 BOM/CRLF를 유지했고 대상 `git diff --check`에 오류가 없다.

Client/UI 실행·조작·청취 확인은 수행하지 않았다. 실제 Product TU 컴파일, 공용
Composition validation/publish 및 설치 적용은 통합 담당이 이어서 검증한다.
