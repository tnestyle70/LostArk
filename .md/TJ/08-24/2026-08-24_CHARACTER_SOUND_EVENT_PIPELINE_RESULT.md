# 캐릭터 애니메이션 SOUND 이벤트 실제 재생 파이프라인 — RESULT

PLAN: [2026-08-24_CHARACTER_SOUND_EVENT_PIPELINE_PLAN.md](2026-08-24_CHARACTER_SOUND_EVENT_PIPELINE_PLAN.md)

## 한 줄 요약

`.animevents`에 이미 저작돼 있던 SOUND 이벤트(6클래스, 1741행)를 실제 wav 파일에 매칭해 배포하고,
Character 애니메이션 재생 중 정확한 타이밍에 `CGameInstance::Play_Sound()`로 재생하는 경로를 새로 연결했다.
로비 BGM 재생/정지도 같은 작업에서 함께 추가했다.

## 한 것

1. **로비 BGM**: `CSound_Manager`에 `Play_Music`/`Stop_Music` 추가(FMOD `FMOD_LOOP_NORMAL`, loopCount=-1).
   `CLevel_Lobby::Initialize()`에서 재생 시작, 서버 승인된 레벨 전환이 실제로 시작되는 시점
   (`Consume_EnterAccepted()`의 `Request_Load` 성공 직후)에 정지. `Client/Bin/Resources/Sound/BGM/Lobby/`에
   `bgm_wallpaperin.wav` 배치.
2. **사운드 카탈로그**: `Tools/CharacterAnimationIntake/build_sound_catalog.py` 신규 작성.
   `.animevents`의 SOUND 행 payload(`<뱅크>.<이벤트명>`)를 `D:\로아 리소스\Sound\Character\...`의 실제 wav와
   대소문자 무시 접두사 매칭 → `Client/Bin/Resources/Sound/Character/`에 배포(2662개 wav 파일) →
   `Data/Sound/CharacterSoundCatalog.json` 생성(이벤트명 → 배리에이션 wav 경로 배열, 클래스별로 분리).
3. **런타임 파싱**: `AnimationEffectCueDocument.h/.cpp`에 `ANIMATION_SOUND_CUE` struct와 `Sounds` 벡터,
   `.animevents`의 `SOUND` 토큰 파싱 분기 추가. 이 파서는 EFFECT/HIT와 동일하게
   Character 초기화 시 `.animevents`를 직접 읽는 기존 경로(`CAnimationEffectCueDocument::Load` →
   `CCharacter`가 `m_EffectCueDocument`에 보관)를 그대로 재사용한다.
4. **카탈로그 로더**: `SoundCueCatalog.h/.cpp` 신규(`PlayerSkillCatalog.cpp`와 동일한 `CDataJson::Parse` +
   `CProjectDataRoot::Resolve` 패턴). `MainApp.cpp` 부팅 시퀀스에서 `CItemCatalog::Load`와 같은 지점에
   비치명적으로(`Load` 실패해도 Client 부팅은 계속) 로드.
5. **런타임 트리거**: `CCharacter::Update_SoundCues()` 신규. 기존 `Update_EffectCues()`의 wall-clock/루프
   epoch 타이밍 해석(`CActionPresentationTimeline`)을 그대로 재사용해 EFFECT와 동일한 정확도로 트리거 시각을
   계산하고, 그 시각에 카탈로그에서 이벤트명을 찾아 배리에이션 중 하나를 `std::rand()`로 골라
   `CGameInstance::Get().Play_Sound()` 호출. `.vcxproj`/`.filters`는 `AnimationEffectCueDocument`와 같은
   필터 그룹(`03. Tools\02. Effect`)에 등록.

## 빌드 중 실제로 잡은 버그 2개 (다른 사람이 비슷한 걸 만들 때 참고)

1. **`SoundCueCatalog.h`에 `Engine_Defines.h` include 누락** — `NS_BEGIN`/`bool_t` 같은 매크로/타입이
   거기서 정의되는데 빠뜨려서, 컴파일 시 매크로가 깨지고 완전히 무관한 뒤쪽 헤더(`ID3D11DeviceChild` 등)까지
   줄줄이 파싱 에러로 번짐. `PlayerSkillCatalog.h`처럼 `Client_Defines.h` + `Engine_Defines.h` 둘 다
   include해야 함.
2. **SOUND 파싱을 HIT 파싱 패턴 그대로 복사하면서 생긴 구조적 실수** — HIT는 데이터 불일치를 문서 전체
   로드 실패(`return false`)로 처리하는 게 맞는데(데미지 판정이라 엄격해야 함), 그 패턴을 SOUND에도 그대로
   가져다써서 SOUND 행 하나(clip이 AvailableClips에 없거나, `payload=""`인 정상적인 "아직 사운드 없는 타이밍
   마커" 행)만 있어도 **EFFECT/HIT까지 포함한 전체 큐 문서가 통째로 로드 실패**하는 버그가 났었음
   (`Character Effect cue load isolated: ...` 로그로 실제 확인). SOUND는 Client 전용 연출이라 판정에
   영향이 없으므로, 이제는 행 하나가 이상하면 그 행만 스킵하도록 고침 — 문서 전체는 항상 로드됨.

## 실측 커버리지 (매칭된 이벤트 / 전체 이벤트)

| 클래스 | 매칭 | 전체 | 비고 |
|---|---|---|---|
| DimensionMaster | 160 | 161 | 99.4% |
| Warlord | 149 | 152 | 98.0% |
| LanceMaster | 229 | 238 | 96.2% |
| Artist | 123 | 129 | 95.3% |
| Common(공용) | 17 | 29 | 58.6% — 아래 참고 |
| GunSlinger | 0 | 199 | 0% — 원본 없음 |
| Slayer | 0 | 212 | 0% — 원본 없음 |

## 부족한 것 / 못한 것

- **건슬링어·슬레이어는 사운드가 아예 없다.** `D:\로아 리소스\Sound\Character\`에 `GunSlinger`/`Slayer`
  폴더 자체가 없음(둘 다 미구현 클래스라 원래 리소스팩에 안 들어있었다고 확인됨). 나중에 리소스팩이
  갱신되면 `build_sound_catalog.py`만 다시 돌리면 자동으로 채워짐 — C++ 쪽은 코드 수정 불필요.
- **Common 뱅크의 41% 미매칭은 원본 데이터 자체의 갭으로 보임.** 예: `.animevents`는
  `PC_Common_Dual1_Fighter_F_Vox1_1`을 요구하는데 리소스팩엔 `..._Vox1_2`만 있고 `_1`은 없음(다른 번호로
  강제 대체하지 않고 그냥 스킵 처리).
- **구현된 4클래스의 나머지 4~5% 미매칭**은 대부분 `SK_...` 접두사(스킬 이펙트음)로, 이 리소스팩의
  `Character/` 폴더엔 없음 — 다른 카테고리(원본 게임의 스킬/이펙트 사운드 뱅크)에 있을 수 있는데
  이번 작업 범위에서는 확인 안 함.
- **실제 게임 내 청취 확인 아직 안 됨.** 방금 두 번째 빌드 버그(SOUND 행이 EFFECT/HIT까지 같이 실패시키던
  것)를 고친 뒤 재빌드/재확인이 진행 중 — 이 문서 작성 시점 기준 "타격음이 실제로 들리는지"는 미확인.
- **`Data/Sound/CharacterSoundCatalog.json`을 만드는 파이프라인 자체는 수동 실행.** `.animevents`가
  바뀔 때마다 자동으로 재생성되는 publish 훅은 없음(다른 domain publisher들처럼 pre-build에 물려있지 않음,
  필요하면 별도 작업).
- **umodel로 원본 게임 사운드 뱅크에서 직접 추가 추출은 시도했지만 막힘.** `-sounds` 옵션은 있지만
  CLI `-nameresolve`가 패키지 **파일명**만 매칭해서, 오브젝트 이름으로 특정 사운드를 찾는 건 안 됨(사운드가
  난독화된 이름의 대용량 뱅크 안에 뭉쳐 들어있어서). 팀장이 맡긴 다른 에이전트는 아마 UModel GUI로 직접
  찾았을 것으로 추정 — 필요하면 그쪽에 방법을 물어보는 게 빠를 듯.

## 관련 파일

```text
Tools/CharacterAnimationIntake/build_sound_catalog.py   (신규)
Data/Sound/CharacterSoundCatalog.json                   (신규, 생성물)
Client/Bin/Resources/Sound/BGM/Lobby/                   (신규, 팀장 관리 리소스)
Client/Bin/Resources/Sound/Character/                   (신규, 팀장 관리 리소스, 2662개 wav)
Client/Public/SoundCueCatalog.h                          (신규)
Client/Private/SoundCueCatalog.cpp                       (신규)
Engine/Public/Sound/Sound_Manager.h                       (수정 — Play_Music/Stop_Music)
Engine/Private/Sound/Sound_Manager.cpp                    (수정)
Engine/Public/GameInstance.h                              (수정)
Engine/Private/GameInstance.cpp                           (수정)
Client/Public/AnimationEffectCueDocument.h                (수정 — ANIMATION_SOUND_CUE)
Client/Private/AnimationEffectCueDocument.cpp             (수정 — SOUND 토큰 파싱)
Client/Public/Character.h                                 (수정 — Update_SoundCues)
Client/Private/Character.cpp                              (수정)
Client/Private/Level_Lobby.cpp                            (수정 — BGM 재생/정지)
Client/Private/MainApp.cpp                                (수정 — 카탈로그 부팅 로드)
Client/Default/Client.vcxproj, .vcxproj.filters           (수정)
```

## 다음 단계

1. 재빌드 후 Bern/Valtan 또는 Character Select에서 LanceMaster/Warlord/Artist/DimensionMaster로 스킬 사용,
   타격음/보이스가 애니메이션과 맞물려 들리는지 직접 확인 (사용자 담당 — 화면·사운드 최종 판정).
2. Common 뱅크 41% 갭, 구현 클래스 4~5% 갭을 실제로 메울지, 메운다면 원본 게임에서 어떻게 더 뽑을지는
   리소스 담당(팀장)과 상의 필요.
3. 건슬링어·슬레이어 리소스팩 갱신되면 `build_sound_catalog.py` 재실행만으로 자동 반영됨.

---

## (참고) 오늘 같은 세션의 다른 작업: ItemUpgrade(장비 재련) UI

같은 세션에서 강화창 UI도 계속 다듬었음 — 좌/우 리스트 레벨·아이템명·스탯 텍스트, 게이지 %(실제 프레임과
동기화하는 버그 수정 포함), 재료 아이콘 3종, "성장"/"장비 재련" 버튼 텍스트, 우측 재련 단계 패널을 4→7줄로
확장 등. 이쪽은 별도 PLAN 없이 대화 중 반복 조정으로 진행돼서 이 문서엔 코드 목록을 따로 정리하지 않았음 —
필요하면 `Data/UI/ItemUpgrade/ItemUpgradeUI.json`과 `Client/Private/MainApp.cpp`의
`RenderItemUpgrade*` 계열 함수들이 실제 정본.
