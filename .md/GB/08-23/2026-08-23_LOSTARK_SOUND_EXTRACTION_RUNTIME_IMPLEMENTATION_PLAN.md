# LostArk 사운드 추출·런타임 WAV 구현 계획서

## 0. 목표와 종료 조건

한국 LostArk 설치본의 Wwise 원본을 실제 데이터와 event ID로 식별해
`Client/Bin/Resources/Sound`에 Sound Manager가 읽을 수 있는 48 kHz PCM16 WAV로 배치한다.

이번 변경 단위의 종료 조건은 다음과 같다.

- Lobby, Character Select, Valtan, Bern BGM과 Bern ambience를 category별 폴더로 분리한다.
- Lance Master, Artist(YINYANGSHI), Warlord(GUNLANCER), DimensionMaster의 전용 media와 PC 공용·피격 media를 보존한다.
- Valtan action/voice/system media와 UI 선택·장비 강화 계열 media를 분리한다.
- 각 출력 WAV가 단일 PCM16 audio stream, 48 kHz, mono 또는 stereo이고 전체 decode에 성공한다.
- 원본 PCK/WEM, 복호화 중간물, 변환 임시 파일은 runtime `Resources/Sound`에 남기지 않는다.
- event 이름, Wwise media ID, 원본 logical package, 출력 asset ID, SHA-256과 WAV 속성을 JSON manifest로 남긴다.
- 자동 검증은 파일 무결성과 decode 가능성을 판정하고, clip 의미·음량·loop 지점의 최종 청취 판정은 사용자가 직접 수행한다.

이번 요청은 asset extraction과 Sound Manager 호환 WAV corpus까지다. Client 화면을 실행하거나 UI를 조작하지 않으며, BGM channel/loop/crossfade와 animation SOUND cue runtime 결선은 별도 runtime vertical slice로 남긴다.

## 1. 현재 실측

- 설치 원본은 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/WwiseAudioPackage`다.
- root와 `Korean`을 합쳐 PCK 184개 약 19.0 GB, standalone WEM 1,189개 약 2.52 GB가 있다.
- 파일명 deobfuscation으로 `SOUND_BGM_BERN`, `SOUND_BGM_COMMANDERRAID`, `SOUND_BGM_TRISION`, 네 직업 package, `SOUND_UI`와 `_NONSTREAM` package를 구분할 수 있다.
- 모든 한국 audio wrapper는 `3E CE A6 74` magic 뒤에 period `0x356` XOR payload를 둔다.
- 184개 PCK의 AKPK alignment padding을 이용해 복원한 854-byte key는 WEM을 `RIFF`, PCK를 `AKPK`로 되돌렸고, probe WEM을 vgmstream으로 9.9초까지 완전 decode했다.
- 현재 `CSound_Manager::Play_Sound(path, volume)`은 FMOD `createSound` 기반 2D one-shot lazy cache다. PCM WAV는 읽지만 BGM loop/stop/switch와 category channel은 없다.
- 현재 Client는 `Play_Sound`를 호출하지 않으며 product animation parser는 SOUND row를 소비하지 않는다.
- `Resources` 최상위 domain 수를 여섯 개로 고정하고 Sound를 금지하던 public 문구는 팀장 지시로 제거한다.
- 기존 Effect 관련 dirty worktree는 이번 작업 범위 밖이며 수정·정리·stage하지 않는다.

## G00. Source identity와 category admission

### 수정·추가 파일

- `Tools/Audio/Extract-LostArkSound.ps1`
- `Tools/Audio/Resolve-LostArkSoundEvents.ps1`
- `Tools/Audio/Publish-LostArkSound.ps1`
- `Tools/Audio/Test-LostArkSound.ps1`
- `Tools/Audio/LostArkSoundSelection.json`

### 구현 범위

추출기는 물리 파일명을 곧바로 저장 ID로 사용하지 않는다. 먼저 LostArk name deobfuscation을 적용하고 선택 문서의 logical package allowlist와 정확히 일치하는 입력만 stage한다. 같은 logical package 중복, 예상 확장자 불일치, wrapper magic 불일치, source root 탈출은 commit 전에 실패시킨다.

초기 category admission은 다음 domain을 소유한다.

```text
BGM/Lobby
BGM/CharacterSelect
BGM/Valtan
BGM/BernCastle
Ambience/BernCastle
Character/LanceMaster
Character/Artist
Character/Warlord
Character/DimensionMaster
Boss/Valtan
UI/Select
UI/Enhancement
UI/System
```

Lobby와 Character Select는 `SOUND_BGM_ALL`과 `SOUND_BGM_TRISION`, Valtan은
`SOUND_BGM_COMMANDERRAID`와 `SOUND_MOB_GLOBAL5`, Bern Castle은 `SOUND_BGM_BERN`과
level sound stream이 참조하는 `SOUND_AMB_AIR1`, `SOUND_AMB_ACT`,
`SOUND_AMB_ACT_NONSTREAM`을 실제 HIRC와 media ID로 결합해 admission한다. package 이름만
보고 임의의 track을 대표곡으로 고르지 않는다.

Valtan은 CommanderRaid의 Heartrb phase media와 Voltan1/2/3 세 bank, Bern은
`berntown` state media 5개와 Bern Castle level이 참조하는 ambience media 23개, UI는
Theme/UI 여섯 bank event를 근거로 분류한다. Artist는 `SOUND_PC_YINYANGSHI`를 사용한다.

### 검증

- 선택 문서 JSON parse
- logical package 중복 0건
- 모든 allowlist package가 설치 source에서 정확히 1개로 resolve
- 잘못된 물리 경로와 unknown logical package가 stage 전에 실패

## G01. 한국 Wwise wrapper 복호화와 AKPK 추출

### 호출 흐름

```text
설치 source scan
→ logical name resolve
→ PCK corpus의 adjusted-byte mode로 0x356 key stage
→ known key SHA-256과 prefix 확인
→ wrapper를 RIFF/AKPK payload로 stream decrypt
→ AKPK language/bank/sound/external table bounds 검증
→ BNK와 WEM을 작업용 staging directory에 추출
→ 실패하면 staging directory만 폐기하고 Resources/Sound 기존 상태 유지
```

key 복원은 PCK의 zero padding 통계에만 의존해 조용히 성공 처리하지 않는다. `A6 90 5F 3A` prefix, 854-byte 길이, 복수 WEM의 RIFF chunk bounds, 복수 PCK의 AKPK section/table bounds를 모두 통과해야 한다.

AKPK entry는 numeric Wwise ID, language, offset, size를 보존한다. offset overflow, duplicate ID, source file bounds 초과, 겹치는 entry는 해당 package 전체를 실패시킨다.

### 검증

- WEM probe 3개 이상 vgmstream metadata parse 및 decode 성공
- stream/nonstream PCK 각각 AKPK parse 성공
- 잘못된 key, truncated wrapper, table overflow, duplicate ID 실패 사례
- source 원본 hash/mtime 불변

## G02. Event와 media ID 결합

### 데이터 정본

- 네 직업: `Data/Animation/Authored/<Asset>/<Asset>.animevents`
- Valtan: `Data/Animation/Reference/Valtan/Valtan.animnotify`
- UI: 추출된 `UISoundTheme.loa`와 UI AkEvent package
- BGM/scene: 실제 LostArk map/scene Wwise event reference와 bank HIRC
- Wwise object graph: 복호화한 BNK의 HIRC, DIDX/DATA와 standalone numeric WEM

### 구현 범위

wwiser가 HIRC의 Event → Action → Sound/Music object graph를 따라 생성한 TXTP와 event name을
media ID 집합에 결합한다. `?ID.wem` unresolved 표기는 stage의 실제 `media/ID.wem`으로
재작성하고 잔존 0건을 확인한 뒤에만 합성한다. 효과음은 random/switch가 참조하는 실제 media를
ID별로 한 번 변환하고 event alias를 catalog에 모두 남긴다. BGM은 유한 event mix와
Sound Manager에서 직접 선택할 수 있는 state/phase media를 함께 보존한다.

같은 media가 여러 event에서 사용되면 WAV payload는 하나만 생성하고 manifest에서 여러 event가 같은 asset ID를 참조한다. 같은 event가 여러 media를 가리키면 deterministic suffix와 media ID를 붙인다.

### 검증

- 네 직업 전용·공용·피격 bank가 참조하는 media ID가 모두 stage에서 resolve
- Valtan BGM 2개와 action/voice event 전부 같은 기준 적용
- UI 선택·강화 event는 이름 근거 없이 추측 분류하지 않음
- event ID, media ID, asset ID의 case-insensitive 중복 검증

## G03. PCM WAV 변환과 category commit

### 변환 계약

```text
Wwise media
→ vgmstream decode
→ ffmpeg pcm_s16le / 48000 Hz / mono 또는 stereo 정규화
→ temp WAV ffprobe
→ SHA-256과 duration 수집
→ category output staging
→ category 전체 성공 후 기존 category를 교체
```

filename은 읽을 수 있는 event stem을 우선하고 Windows 금지 문자와 길이를 정규화한다. 충돌 시 Wwise media ID를 suffix로 붙인다. raw PCK/WEM/BNK와 decrypt key는 output에 복사하지 않는다.

출력은 `Client/Bin/Resources/Sound/<Category>/.../*.wav`만 사용한다. 다른 Resources domain과 Effect 작업물은 건드리지 않는다.

### 검증

- ffprobe: `pcm_s16le`, 48000 Hz, channels 1 또는 2, duration > 0
- ffmpeg decode-to-null 전 파일 성공
- zero-byte, non-WAV, raw intermediate 0건
- case-insensitive asset ID 중복 0건
- category별 expected/resolved/unassigned/output count 일치

## G04. Catalog, 결과 문서와 인계

### 산출물

- `Client/Bin/Resources/Sound/SoundCatalog.json`
- `Client/Bin/Resources/Sound/ExtractionManifest.json`
- `.md/GB/08-23/2026-08-23_LOSTARK_SOUND_EXTRACTION_RUNTIME_RESULT.md`

Catalog에는 runtime 소비자가 사용할 Resources-relative `Sound/...` asset ID만 둔다. Manifest에는 추출 재현과 청취 검수에 필요한 source logical package, event name/ShortID, media ID, category, WAV metadata, hash, resolution 상태를 둔다. 절대 설치 경로와 decrypt key는 저장하지 않는다.

RESULT는 실제 출력 수와 bytes, category별 검증 결과, unresolved event, 자동 검증과 사용자 청취 대기 항목을 분리한다. Client나 UI를 에이전트가 실행하지 않고, 사용자가 직접 들을 정확한 폴더와 대표 후보를 인계한다.

### 최종 검증

```powershell
pwsh -File Tools/Audio/Test-LostArkSound.ps1
git diff --check
```

Engine/Client 코드를 바꾸지 않는 asset extraction만 완료되면 C++ build는 요구하지 않는다. Sound Manager API나 Client consumer를 추가하는 후속 작업에서는 Engine x64 Debug/Release → `UpdateLib.bat` → Client Debug/Release와 전용 FMOD load harness를 같은 변경 단위로 실행한다.
