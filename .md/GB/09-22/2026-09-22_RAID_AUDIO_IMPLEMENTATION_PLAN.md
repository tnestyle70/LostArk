# 레이드 BGM과 쿠크 패턴 사운드 구현 계획

## G00. 목표와 현재 연결

베른 입장은 `CLevel_Bern`의 thecapital, 발탄 전투는 `CValtan::Update_RaidBgm`의
원본 M06 EventMixes 경로가 이미 연결되어 있다. 중복 재생 경로를 추가하지 않고 실제
설치 파일과 snapshot 전환 조건을 확인한다. 쿠크는 `CLevel_KakulSaydonArena`의
대기 구역 M12만 연결되어 있어 같은 Level 소유 Music 채널을 관문과 기믹까지 확장한다.

## G01. 원본 음원과 소유 상태

설치 게임의 Wwise package와 기존 `Tools/SoundPipeline`을 사용한다. Event의 Play,
Stop/Pause/Resume, Music playlist와 Layer를 구분한다. 완성 이벤트의 첫 유효 주기를
원본 TXTP로 렌더하며 후보 WAV는 `out/RaidAudio20260922/Resources`에만 둔다.
원본 이름·package/bank/media ID·hash·길이는 같은 폴더의 receipt에 기록한다.

Level은 Server raid phase와 local replicated Mario/maze 상태로 BGM을 선택한다.
Sequence/CINEMATIC 상태가 먼저 Music을 해제하며 같은 snapshot은 곡을 재시작하지 않는다.
기믹 종료에는 해당 관문 음악으로 복귀한다. 누락 음원은 gameplay를 막지 않는다.
새 C++ 파일 없이 기존 Level H/CPP의 상태와 함수만 확장하므로 project 등록은 필요 없다.

## G02. 패턴 SOUND

원본 Action LOA notify를 현재 Animation occurrence의 source-in, playRate, stage 시작에
맞춘다. P100 정면 바람방구와 P103 카드비의 빈 SOUND 행을 보완한다. GameMsg의
`tip.desc.guide_gamenote_raid_kouku_02`, `_10` SOUND 참조로 정확한 대사를 추출한다.
P23 팡파레, P79 공먹기, P78 Dice는 저장된 Effect/Logic 및 원본 notify를 대조해 빠진
출력 소리와 현재 timeline에서 어긋난 timing만 고친다. 추측한 대사를 만들지 않는다.

공용 Composition은 통합 담당이 소유한다. 이 담당은 최신 문서에 대한 stable ID/field
patch와 후보만 전달하며 문서 전체를 덮어쓰지 않는다. 사운드 event catalog도 기존
`CSoundCueCatalog` 소비 경로를 그대로 사용한다.

## G03. 검증과 경계

실제 설치 WAV/후보의 FMOD NOSOUND load/length, JSON parse, 원본 timing 대응,
stable-ID 병합 멱등성과 무관 필드 보존을 확인한다. 변경 BGM selector는 focused native
검사로 반복 snapshot, cinematic 우선, 기믹 입장/복귀를 확인한다. Product 빌드는 통합
담당이 수행한다. Client/UI 자율 실행이나 청취 판정은 하지 않는다.
