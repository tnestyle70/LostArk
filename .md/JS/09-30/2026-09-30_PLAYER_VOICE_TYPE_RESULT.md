# 2026-09-30 플레이어 음성 타입(성우) 선택 RESULT

## 문제

스킬 대사 cue가 `CharacterSoundCatalog.json`의 변형 목록에서 매번 `std::rand()`로 골라
같은 스킬에도 성우가 바뀌어 들렸다. 원본은 직업당 캐릭터 생성 음성 타입 4종(Type1~4)을
Wwise Switch 컨테이너로 분기하고, 카탈로그 빌드는 그 분기를 한 목록으로 평탄화했다.

## 확인한 원본 구조

- wav 파일명 `__<숫자>`는 Wwise 미디어 ID. `Tools/SoundPipeline/wwise_audio_package.py`로
  설치본 package를 다시 걸으면 Vox 이벤트 → Switch(`Type1..Type8`, fnv1_32) → RandomSeq → 미디어가 나온다.
- 창술사·도화가·차원술사·워로드·가디언나이트·Common의 모든 Vox 파일이 한 타입 이상에 배정됐다
  (일부 take는 원본에서도 여러 타입이 공유).
- `EFTable_CharacterCustomizing` ClassifyType 17이 음성 타입, 문자열은 `sys.pccreat.voicetype1~4`.

## 구현

| 계층 | 변경 |
|---|---|
| Tools | `Tools/SoundPipeline/build_character_voice_types.py` → `Data/Sound/CharacterVoiceTypes.json`(formatVersion 1, 직업별 `selectedVoiceType` 기본값 + 이벤트별 `Type1~4 → wav 목록`). 재생성 시 `selectedVoiceType` 보존 |
| Shared | protocol 126 → 127. `MIN/MAX_VOICE_TYPE`, `Is_Valid_VoiceType`. `C2S_ENTER_WORLD`·`S2C_PLAYER_SPAWNED` 끝에 U8 voice type 추가 |
| Server | `SERVER_PLAYER.iVoiceType`, `SERVER_WORLD_TRANSFER_REQUEST.iVoiceType`. 입장 검증·저장, 월드/파티/Debug 이동, spawn 송신 5곳에 복사. 직업 변경은 `staged = player` 복사로 유지. Guide AI는 기본 1 |
| Client 복제 | `NET_PLAYER_RECORD`·`CHARACTER_DESC`·`CCharacter::m_iVoiceType`. `Create_Character`에 voice 인자 추가(spawn·직업 변경 재생성) |
| Client 사운드 | `CSoundCueCatalog::Find_VoiceVariants(class, event, voiceType)`, `Collect_VoiceTypes`, `Collect_VoiceEventNames`. `Update_SoundCues`는 자기 voice type의 take 중에서만 랜덤 |
| Client 생성창 | `CCustomizingView` 음성 탭 활성화. `CustomizingUI.json`에 `CC_VoiceDivision`, `CC_VoiceType0~7_Bg/_Selected` 추가. 행 클릭 → 선택 + 미리듣기. 외형 JSON `voiceType` 저장·복원, `Read_SavedVoiceType` |
| Client 입장 | `CHARACTER_ENTRY_IDENTITY.iVoiceType`(pending 생성/생성된 캐릭터의 외형 JSON에서 읽음, audition은 1) → `CLevel_Lobby` → `Send_EnterWorld(..., voiceType)` |
| 하네스 | `NetworkProtocolHarness`: spawn payload 35바이트 pin, voice 1..8 왕복·0/9 거부, protocol 127 pin |

## 검증

- Debug Product 빌드 PASS (Shared/Server/Client).
- `NetworkProtocolHarness.exe` Debug: `[FAIL]` 없음, exit 0.
- 사용자 화면 확인(2026-09-30): 음성 탭 미리듣기·입장 후 스킬 대사가 고른 타입으로 재생됨("잘 나와").
- 다인(Client 2개) 상대 목소리 확인은 아직 하지 않았다.

## 남은 경계

- 음성 탭 행은 `v2_tab2_normal/over.png`(82×22)를 240×22로 늘려 쓴 임시 배치다.
- 건슬링어/슬레이어는 카탈로그에 wav가 없어 voice 문서에 없다(기존 동작 유지).
- `Data/Sound/CharacterVoiceTypes.json`은 설치본 Wwise package가 있는 PC에서만 재생성된다.
- protocol 127이라 팀 LAN Server도 같이 재빌드·재시작해야 한다.
