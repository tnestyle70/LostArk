# 쿠크 원본 사운드 연결 후보와 재생 소비자

## 구현 상태

원본 Action LOA 4개 profile의 AKEvent 6,039개와 Matinee AkEvent를 조사했다.
Action 현재 저장본 revision 1748에서 79개 Pattern에 SOUND 691개를 추가하는
revision 1749 후보를, Sequence revision 63에서 7개 Pattern에 31개를 추가하는
revision 64 후보를 만들었다. 원본 JSON과 설치 Resources를 교체하거나 publish하지 않았다.

| 항목 | 실측 |
|---|---:|
| 조사 source event identity | 487 |
| 재생 가능한 event library | 480 |
| Action 연결 | Animation 682 + Matinee 9 |
| Sequence 연결 | Animation 17 + Matinee 14 |
| 기존 설치 WAV 직접 재사용 | 1,042 |
| 신규 원본 미디어 디코딩 | 0 |
| 기존 원본 PCM에서 TXTP event 렌더 | 18 |
| 18개 새 event WAV 크기 | 227,088,088 bytes |

후보는 `out/KoukuSoundRestore20260918/`와 그 아래 `sequence/`에 있다.
두 후보가 공유하는 18개 Resources 상대 asset ID는 동일하므로 한 번만 설치한다.
재실행 중 생긴 이전 파일이 있으므로 폴더 전체가 아니라 manifest의
`renderedVariants`와 `copiedMedia`에 명시된 파일만 설치 대상이다.

## 원본 근거와 연결 방식

- `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260829/ActionNameSources/*.action-effects.json`의
  profile/action/stage/slot을 현재 Animation occurrence에 연결했다. Anim notify 시작점,
  Source In, playRate, LOOP_TO_WINDOW를 반영하고 이미 trim된 앞부분의 one-shot은 다시 만들지 않는다.
- Source Action ID가 없는 RAW clip은 해당 profile의 원본 clip 후보가 모두 같은 notify를
  가질 때만 연결했다. Action P36 및 Sequence P8의 각 10개 RAW occurrence는 원본 부재 또는
  서로 다른 notify 목록으로 인해 임의 연결하지 않았다. 구체 ID는 manifest holdouts에 있다.
- `SoundPlayableExtraction-20260830`의 receipt/WAV, 원본 bank와 wwiser TXTP를 사용했다.
  기존 1,088 WAV runtime closure에 컷씬 `S_SCENE_OCEAN3_3` 계층이 빠져 있었다.
  별도 full extraction에 원본 PCM이 있어 추가 네트워크 다운로드나 원본 재추출은 필요 없었다.
- 컷씬 `moveintosatonbook`, `movetocardmaze`는 실제 HIRC Event → Play → Layer → Sound 2개다.
  한 파일을 랜덤으로 고르면 층이 사라지므로 원본 TXTP의 Layer/Sequence/gain/delay를
  하나의 완성 event WAV로 렌더했다. 재생기는 event variant 하나만 고른다.
- `g_bigsatan1_attack01_shotvox1`의 원본 Random playlist는 10000/10000/10000/90000이다.
  마지막 child의 3개 voice까지 포함하면 얕은 child는 각각 1/12, 깊은 child는 각각 1/4다.
  catalog의 가중 반복 asset 목록이 이 확률을 보존한다. bank SHA와 playlist는 manifest에 남겼다.
- wwiser의 중첩 Random TXTP는 외부 rN을 내부 작은 그룹에도 넣어 일부 파일이 실패했다.
  importer는 각 Random 그룹의 유효 선택을 독립적으로 열거하고 실제 HIRC playlist와 대조한다.
  범위를 임의로 clamp하거나 유효한 voice를 누락하지 않는다.

## 시퀀스 시간과 남은 경계

World/Camera와 같은 source clock에 AkEvent 시작을 놓았다. Gate 1 통합 연출은 기존
`build_gate1_popup_lights.py`의 승인된 landmark와 12,258ms 시작 offset을 사용한다.
GlobalSlomo를 사운드 시작 시각에 다시 적분하지 않았다. 원본 전체 soundtrack을 새로운
GlobalSlomo 곡선에 따라 실시간으로 늘이거나 줄이는 기능을 구현했다고 주장하지 않는다.

Sequence P7은 원본 SCENE02A의 16,710ms 이후 구간이다. `soundSourceStartMs`로 이미
시작한 soundtrack의 해당 media age부터 이어 듣는다. 0ms로 되돌려 중복 재생하지 않는다.

남은 7개 event는 미디어 파일 누락으로 분류하지 않는다. 실제 bank 확인 결과 6개는
Play action이 없는 Stop/Resume 계열 control event이며, 카드미로 BGM 시작 event의
Play target 348853793은 type 13 Music container지만 child가 없다.
이 control event를 WAV로 추측해 추가하지 않았다. 2개 loop BGM 전환은 기존 시퀀스의
남은 유한 owner 구간이 첫 source cycle보다 짧음을 확인하여 그 구간만 연결했다.
시퀀스 종료 후 다른 Pattern으로 이어지는 지속 BGM 상태 머신은 별도 소유 계약이다.

import된 SOUND는 기존 저작 가능한 absolute Pattern occurrence다. 원본 Animation/notify
연결 증거는 manifest에 남는다. 이후 Animation 시간을 바꿔도 사운드 행을 자동 재배치하는
새 편집기 소유 규칙을 추가하지 않았다. 후보 재생성 시 최신 source window를 다시 계산한다.

## 코드 소비자

`KoukuSaydonCompositionDocument`와 기존 projector에 SOUND 전용 optional
`soundEvent`, `soundSourceStartMs`를 추가했다. `KoukuSaydonPresentationPlayer`는
기존 `CSoundCueCatalog::Load_ClassSnapshot`을 명시적인 Product/Preview 시작에 사용하고
기존 `Play_SoundCue`의 seek/paused/Stop 경로로 재생한다. 매프레임 JSON을 읽지 않는다.
동일 occurrence의 variant 선택은 scrub 중 유지되고 실제 WAV 길이를 지난 one-shot은
완료 처리하여 0ms에서 다시 소리 나지 않는다.

`build_kouku_sound_candidates.py::merge_additions`는 최종 설치자의 메모리 병합용이다.
무관한 position/box 추가를 보존하고 새 numeric occurrence ID를 재할당한다. Animation 시간
변경은 재생성을 요구한다. 이 함수 자체는 파일을 쓰지 않으며 최종 설치자는 hash 재검사,
백업, 원자 교체와 실패 rollback을 유지해야 한다.

## 실행한 검증

- `test_kouku_sound_candidates.py` 7개 PASS: Layer/Random 분리, gain/delay/Sequence,
  중첩 Random, 실제 bank 확률, clip trim/loop, 미해석 입력 거부, 무관한 편집 보존/idempotent 병합.
- 기존 projector의 resource/occurrence 검증과 실제 projection으로 추가 SOUND 722개 PASS.
  `candidate-validation.json`에 revision과 개수를 기록했다.
- 새 WAV 18개를 프로젝트 FMOD DLL의 NOSOUND output으로 create/getLength 검증 PASS.
  실제 재생 channel 0이며 Client/UI를 실행하지 않았다. `fmod-admission.json`에 기록했다.
- 변경 소비자 `KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonPresentationPlayer.cpp`
  MSVC x64 C++20 개별 컴파일 PASS. `out/KoukuSoundRestore20260918/compile/compile-exit.txt` = 0.
- 변경 경로 `git diff --check` PASS.
- Action 전체 validate는 사운드 후보 전후 모두 기존 P32 `세이튼_쇼타임`의
  presentation lifetime 초과로 실패한다. 이 무관한 행을 사운드 importer가 변경하지 않았다.
- 실제 사용자 청취, live 저장본 교체, publish, Server 재시작은 수행하지 않았다.


## 통합 shadow 검증과 방향 분신 자식 SOUND

통합 후보 최초 projector의 Product 2 / 93 결과를 분리 조사했다. 75개는 SOUND 내용과 무관하게 검증용 Resources의 Character junction을 resolve한 경로가 shadow Resources 밖으로 나가 Bone Collider containment 검사에서 탈락했다. 3개 부모 P52/P65/P66은 자식 패턴의 새 SOUND 행을 기존 Effect-only 검증이 거절했다. 나머지 13개는 기존 빈 Stage·연출 Stage·동적 follow-up 등 사유다.

Containment 검사를 완화하지 않았다. 검증용 root의 BossCatalog/World Object 모델 149개 경로를 해당 shadow 내부의 실제 directory와 읽기 전용 사용 파일 hardlink로 연결했다. 원본 파일 bytes는 변경하지 않았으며 script와 receipt는 `out/KoukuRaidIntegration20260918/materialize_validation_models.py`, `shadow-resource-containment.json`에 있다. Windows 긴 경로용 접두사를 파일 hardlink에 적용했다. 이 모델 파일은 검증 입력으로만 사용하고 쓰지 않는다. 사용자 candidate JSON 갱신은 통합 담당이 이어 수행한다.

Python `_validate_cross_direction`와 C++ `Try_ResolveCrossDirectionWindows`는 Effect와 SOUND를 허용하도록 일치시켰다. 새로 허용되는 SOUND는 BOSS anchor만 사용하고 외부 Logic/World 참조를 허용하지 않는다. Camera/Light와 다른 action owner 제한은 그대로다.

실제 소비 경로도 확인했다. Preview는 선택된 자식의 PresentationOccurrences에 split 시작 시각을 더해 부모 시계에 연결한다(`KoukuSaydonPresentationPlayer.cpp`의 cross direction 처리). Product는 Server가 전송한 `strPresentationPatternId`와 `iPresentationPatternStartTick`으로 별도 child session을 샘플하고, 공통 `Sample`의 SOUND 분기가 occurrence age와 Source In을 합해 기존 SoundCue handle을 재생한다. source soundEvent, variant 선택과 Source In은 복사 과정에서 보존된다. 새 재생 서비스나 Server 오디오 자산 의존을 추가하지 않았다.

focused `cross_direction` Python 검사 4개가 통과했다. 새 검사는 SOUND 추가 뒤 typed child 참조와 원본 문서/시각/Source In 보존, Camera/외부 anchor 거절을 확인한다. CompositionDocument와 PresentationPlayer 2 TU의 최종 MSVC 컴파일은 0이며 `git diff --check`가 통과했다. 최종 통합 Product 개수와 publish 결과는 최신 후보를 다시 투영하는 통합 담당의 결과로 별도 확인한다.
