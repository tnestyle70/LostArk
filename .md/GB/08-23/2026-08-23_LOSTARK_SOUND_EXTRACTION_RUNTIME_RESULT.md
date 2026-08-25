# LostArk 사운드 추출·런타임 WAV 결과

## 1. 완료 상태

한국 LostArk 설치본의 Wwise wrapper를 복호화하고 요청 범위의 event/media를 식별해
`Client/Bin/Resources/Sound`에 Sound Manager가 읽을 수 있는 WAV로 배치했다.

- 출력 WAV: 11,644개
- 출력 WAV bytes: 8,077,905,210
- 총 재생시간: 44,968.437초
- mono/stereo: 3,927 / 7,717
- 형식: PCM signed 16-bit little-endian, 48 kHz, mono 또는 stereo
- 원본 WEM/BNK, decrypt key, 변환 중간물의 runtime 출력 잔존: 0건

요청 단위별 출력 수는 다음과 같다.

| 요청 범위 | WAV | 실제 폴더 |
|---|---:|---|
| Lobby BGM | 71 | `Sound/BGM/Lobby` |
| Character Select + Trision BGM | 53 | `Sound/BGM/CharacterSelect` |
| Valtan phase BGM + finite event mix | 16 | `Sound/BGM/Valtan` |
| Bern Castle BGM + state track | 35 | `Sound/BGM/BernCastle` |
| Bern Castle level ambience | 23 | `Sound/Ambience/BernCastle` |
| Valtan action/voice/boss SFX | 728 | `Sound/Boss/Valtan` |
| Lance Master | 915 | `Sound/Character/LanceMaster` |
| Artist | 745 | `Sound/Character/Artist` |
| Warlord | 511 | `Sound/Character/Warlord` |
| DimensionMaster | 1,270 | `Sound/Character/DimensionMaster` |
| PC common/hitted/global | 5,570 | `Sound/Character/Common` |
| UI enhancement/equipment/select/system | 1,707 | `Sound/UI` |

Bern Castle ambience는 level sound stream의 37 AkEvent reference를 HIRC와 결합했다. 재생
media가 없는 `_stop` control event는 WAV를 만들지 않았고, 실제 playable media 23개만
Air, Crowd, Objects, Water로 나눴다. Valtan BGM과 Bern Castle BGM은 Wwise state 전환을
지원하지 않는 현재 Sound Manager에서도 직접 고를 수 있도록 phase/state media와 유한 event
mix를 함께 둔다.

## 2. 구현

다음 재현 도구를 추가했다.

- `Tools/Audio/LostArkSoundSelection.json`: 24개 logical package와 category admission
- `Tools/Audio/Extract-LostArkSound.ps1`: filename 복원, 854-byte key 검증, PCK/BNK/WEM stage
- `Tools/Audio/Resolve-LostArkSoundEvents.ps1`: 16개 bank group의 HIRC/TXTP event graph 생성
- `Tools/Audio/Publish-LostArkSound.ps1`: media dedupe, TXTP 경로 교정, WAV 변환, transactional publish
- `Tools/Audio/Test-LostArkSound.ps1`: JSON/path/raw 잔존, ffprobe, 선택적 ffmpeg full decode 검증

wwiser가 다른 bank의 embedded media를 `?ID.wem`으로 남기는 경우 vgmstream이 오류 대신
1초 Silence를 만들 수 있다. publisher는 모든 `?ID.wem`을 검증된 stage media 절대 입력으로
재작성하고 잔존 0건을 확인한 뒤 BGM event mix를 렌더한다. 효과음은 event random/state 조합을
중복 렌더하지 않고 실제 media ID별 WAV 한 개와 모든 event alias를 catalog에 기록한다.

`Resources` 최상위를 기존 여섯 폴더로 고정하고 `Sound`를 금지하던 문구는 `AGENTS.md`,
`CLAUDE.md`, 팀 interface handbook에서 제거했다. Resources-relative path 탈출 금지와 다른
안전 계약은 유지했다.

## 3. Runtime 인계 파일

- `Client/Bin/Resources/Sound/SoundCatalog.json`
  - runtime용 `Sound/.../*.wav` asset ID
  - category, event alias, Wwise media ID
- `Client/Bin/Resources/Sound/ExtractionManifest.json`
  - source logical package, event/media ID, bytes, duration, channel, SHA-256
  - category별 파일 수·bytes·duration

두 JSON은 절대 설치 경로와 decrypt key를 저장하지 않는다. `Client/Bin/Resources`는 팀장이
관리하는 물리 runtime 입력이므로 WAV corpus는 Git commit 대상이 아니다.

## 4. 자동 검증 증거

### Extraction

- selected PCK: 24개
- staged BNK: 186개
- staged unique media: 35,436개
- wrapper key length/prefix/SHA와 RIFF/AKPK/BKHD magic 검증: PASS
- source logical package 중복·누락: 0건

### Event resolution

- group: 16개
- TXTP graph: 19,192개
- selected bank ID 누락: 0건
- publish 전 unresolved `?ID.wem` 잔존: 0건

### Publish와 catalog

- 실제 WAV / catalog entry / manifest file entry: 11,644 / 11,644 / 11,644
- case-insensitive asset ID 중복: 0건
- catalog ID 중복: 0건
- WAV 이외 raw audio/key 파일: 0건
- JSON parse와 모든 WAV reference 존재 검사: PASS

### 전 파일 audio 검사

다음 명령을 실행했다.

```powershell
pwsh -File Tools/Audio/Test-LostArkSound.ps1 `
  -Root Client/Bin/Resources/Sound `
  -ThrottleLimit 12 `
  -FullDecode
```

- ffprobe `pcm_s16le`, 48 kHz, 1~2 channels, positive duration: 11,644/11,644 PASS
- ffmpeg decode-to-null로 파일 끝까지 decode: 11,644/11,644 PASS
- validation failures: 0

Engine/Client C++와 public header는 바꾸지 않았으므로 Engine/Client build는 적용 대상이 아니다.
Client와 UI는 실행하거나 조작하지 않았다.

## 5. 남은 수동 경계

- 음원의 의미, 체감 음량, 대표곡 선택과 loop 지점의 최종 청취 판정은 사용자 확인이 필요하다.
- 현재 `CSound_Manager::Play_Sound(path, volume)`은 2D one-shot lazy cache다. 이번 결과는 해당
  API가 로드 가능한 WAV corpus까지이며, BGM loop/stop/switch/crossfade channel과 animation
  SOUND cue runtime 결선은 별도 구현 범위다.
- 검증이 끝난 생성형 staging 약 14.47GB와 복호화 key 파일은 `%TEMP%` 아래에만 남아 있다.
  host의 destructive cleanup policy가 명시 경로 삭제도 차단해 자동 정리는 실행되지 않았다.
  runtime 출력은 이 경로를 참조하지 않으며 사용자가 확인 후 `LostArkSound*` stage와
  `LostArk_KR_Wwise_20260823.key`를 수동 삭제할 수 있다.
