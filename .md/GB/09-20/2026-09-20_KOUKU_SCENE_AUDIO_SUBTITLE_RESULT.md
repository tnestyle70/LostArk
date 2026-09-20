# 쿠크 연출 사운드·원본 자막 연결 결과

## G1. 실제 구현

`KoukuSaydonCompositionDocument`와 Python projector에 `SUBTITLE` presentation kind를 추가했다. `assetId`는 원본 GameMsg ID이며 `subtitleText`는 1~4096 byte의 유효한 UTF-8 plain text다. LF만 제어문자 예외로 허용하며 markup, 잘못된 UTF-8, 다른 kind에 붙인 자막 필드와 잘못된 `subtitlePosition`을 거부한다. 위치는 `NORMAL` 또는 `UPPER`, occurrence는 `MAP`, `followBoss=false`다.

`KoukuSaydonActionWorkbench`는 Subtitle resource 목록과 별도 timeline 행을 표시하고 기존 start/duration 편집·저장 계약을 사용한다. enum 값은 기존 값 뒤에 추가했고 timeline 배열은 명시적인 lane mapping을 사용한다. `KoukuSaydonPresentationPlayer`의 Product reader와 Preview가 같은 텍스트·위치 계약을 읽으며, 기존 active row의 수명으로 Seek/Reset/Stop을 처리한다. `Collect_Subtitles()`는 Preview 또는 Product/child/Mario/tail에서 활성 텍스트만 읽고 중복을 제거한다. MainApp 제품 font 표시는 통합 담당이 같은 API에 연결했다. 새 C++ 파일이나 별도 런타임은 없다.

## G2. 원본 근거와 후보

설치 게임 `data2.lpk`에서 `EFTable_GameMsg.db`를 추출했다. Matinee `efinterptracksubtitle`의 `strmsgid`를 실제 한국어 GameMsg와 결합했다. `<br>`는 LF로, FONT color tag는 표시용 plain text로 변환했으며 전체 원문은 manifest에 보존했다. 임의의 대사는 만들지 않았다.

- Action 후보: G2 진입 P73 2행, G2 clear P74 6행, ending P75 5행, Showtime P76 1행으로 자막 14행.
- Sequence 후보: G2 진입 P3 2행, G1 P4 2행, G2 clear P5 6행, G3 진입 P7 2행, ending P9 5행으로 자막 17행.
- 카드미로 SCENE04A InterpData393에는 원본 subtitle track이 없다. SCENE01A와 SCENE06A에도 없다. SCENE01C는 ending의 별도 variant다.
- SCENE07A Bingo 진입 3행 및 SCENE02A hammer 연출 1행은 원문·위치·시간을 `unmapped-source-subtitles.json`에 보존했다. 현재 작업 범위에 대응 cinematic Pattern/camera가 없어 새 연출을 만들거나 임의 연결하지 않았다.

쇼타임 SCENE02B track80은 79ms에 `scene_midnightc_ed_koukuskill` Play 한 개이며 track81 BGM은 비어 있다. 실제 `SOUND_SCENE_OCEAN1` bank2755353966의 Event→Play→Layer 두 원본 WEM을 wwiser TXTP와 vgmstream으로 합성했다. 첫 layer의 -1dB도 보존했다. 신규 float WAV는 44100Hz stereo 326105frames이며 7395ms로 올림한 수명을 사용한다. 해당 event에 Stop/fadeout은 없다. P76 SOUND는 79/7395ms, Pattern timeline은 7474ms이며 Server stage5000ms는 유지한다. 기존 Product tail 승계와 Preview Pattern duration이 자연 종료를 소비한다.

카드미로는 기존 catalog/WAV에 두 layer의 전체 소리가 이미 있었다. 원본 44100Hz 742056frames와 설치 WAV 48000Hz 807680frames의 길이가 일치한다. 하지만 Action P77과 Sequence P6의 SOUND row가 11500ms로 잘려 있었다. 후보는 두 행을 16827ms로 복구한다. P77의 기존 timeline18307ms는 유지하고 P6 timeline만17277ms로 늘리며 Server stage11950ms는 유지한다. 원본 track528은 450ms Play 한 개, track529 종료 이벤트는 별도 BGM control이다. BGM Play target348853793은 자식 없는 Music container이므로 별도 소리 파일을 만들어 넣지 않았다.

후보·baseline·hash 목록은 `out/KoukuSceneAudioSubtitle20260920/candidate-files.json`이다. 실제 파일은 `subtitles/{baseline,candidate}/Data` 아래 Action, Sequence, CharacterSoundCatalog 세 개다. source 파일 당시 hash는 `subtitle-manifest.json`/`sound-manifest.json`, 원본 bank/event/media 근거는 `audio/event-bank-evidence.json`과 TXTP에 있다. JSON baseline은 정규화 저장본이므로 source byte hash와 다를 수 있다. 통합은 stable ID 및 변경 leaf를 baseline과 비교해야 한다.

## G3. 실행한 검증

- 실제 codec·player·workbench C++ 전체 TU 집중 컴파일 exit0. root의 MainApp 자막 font 코드가 포함된 별도 전체 TU 컴파일 exit0. 기존 EngineSDK 인코딩 경고만 남았다.
- 실제 codec 객체를 링크한 out 전용 probe 56 checks PASS: 두 후보 parse/serialize/reparse, invalid UTF-8·markup·control·position·cross-kind rollback, 4096 byte 경계, `Reload_FromPath → Save_Atomic → Reload` 뒤 원문·개행·편집 시간 보존. 실제 설치 Data는 쓰지 않았다.
- 실제 Product reader helper/Read_Resource/Read_Occurrence 본문을 추출한 Subtitle 전용 probe에서 projector가 만든 자막 31행과 4096 byte 경계 PASS. 관계없는 Effect/Sound filesystem guard는 호출 시 실패하도록 격리했으며 GPU는 실행하지 않았다.
- Python focused unittest 6 PASS: byte 경계, invalid resource/occurrence, 실제 projected 텍스트·위치·시간, 원본 markup 변환, source clock, 기존 저장 시간 보존과 재실행 멱등성.
- 신규 WAV는 FMOD NOSOUND에서 createSound/getFormat/getLength 성공: float32 stereo44100Hz, 길이7394ms(정수 내림), 재생 channel0. 오디오 장치를 사용하거나 실제 청취하지 않았다.
- 후보의 추가 presentation 행을 Python validator로 검사했고 Showtime Product duration7474/stage5000 분리를 확인했다. `git diff --check` 통과.

## G4. 설치·남은 경계

이 담당은 live Data/Resources를 교체하거나 Client를 실행하지 않았다. 신규 Resource는 `sound-manifest.json`의 단일 WAV이며 통합 담당이 최신 저장본 CAS와 백업 후 설치한다. 원본 Source/candidate를 실제 적용으로 혼동하지 않는다.

`CSoundCueCatalog`는 `CProjectDataRoot::Resolve("Sound/CharacterSoundCatalog.json")` 정본을 직접 읽는다. 별도 SoundCatalog runtime 복사본/publisher는 없다. 신규 WAV, catalog, Composition을 반영하고 기존 Composition publisher를 실행해야 한다. Kouku Play/Preview admission의 catalog snapshot이 새 내용을 읽는다. 실행 중 draft Reload와 Server 새 bootstrap 소비, 사용자 화면·청취는 별도 확인 경계다.

빙고 유리파손 추출은 통합 담당이 `out/KoukuBingoGlass20260920`에 처리했다. 이 변경은 camera/animation을 수정하지 않았다. 전체 Product 빌드와 최종 CAS/publish 결과는 통합 RESULT에 기록한다.
