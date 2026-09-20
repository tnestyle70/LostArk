# 발탄 원본 연출 자막 연결 결과

## G1. 실제 완료 상태

설치 GameMsg DB 원문 8개와 원본 SCENE06A·04A·07A Matinee track을 기존 WorldSequence에 연결했다. 새 런타임이나 중복 Valtan stage 자막 시간표를 만들지 않았다. Document의 optional `subtitleTracks` parse/validate/serialize와 publisher 검증은 같은 작업의 sequence_editor가 맡았고, 이 변경은 기존 WorldSequencePlayer의 read-only 자막 sample과 Level·MapTool forwarding, 원본 후보 생성을 맡았다. MainApp 제품 폰트 렌더러 연결은 통합 담당이 구현했다.

| 기존 instance suffix | 위치 | 시작 ms | 길이 ms | 원문 |
|---|---|---:|---:|---|
| entrance | NORMAL | 19289 | 1300 | 나, 는... |
| entrance | NORMAL | 20645 | 3500 | 마수군단장... 발탄...! |
| trash | UPPER | 623 | 1800 | 이 버러지들... |
| trash | UPPER | 3276 | 2500 | 다... 짓뭉개주마! |
| finale | NORMAL | 5460 | 2300 | 안돼, 안돼! |
| finale | NORMAL | 7860 | 6500 | 크으으아아아악! 카마... 인...! |
| gate1-entrance | BALLOON | 3733 | 1800 | 머리는 내 것이다, 루가루! |
| gate1-entrance.black-wolf | BALLOON | 7500 | 2200 | 새로운 사냥감이군. |

`Collect_Subtitles`는 성공한 마지막 WorldSequence sample의 template와 local clock을 사용한다. Pause는 같은 sample을 유지하고 Seek는 성공한 새 sample을 사용하며, 실패한 Apply는 sample을 무효화한다. Stop/clear로 owner가 제거되면 수집 대상에서 사라진다. 행은 시작 포함·끝 제외다. finished HOLD pose는 template 종료 이후 자막을 남기지 않는다.

NORMAL/UPPER는 장면 시간에 연결된다. BALLOON은 원본 white actor25/group62와 black actor26/group63에 대응하는 기존 `actor` ObjectResource slot을 사용한다. 해당 오브젝트가 visible이고 실제 CModel local bounds가 있을 때 sampled world로 변환한 8개 corner의 AABB 상단을 반환한다. 숨김·누락·비유한 위치는 해당 말풍선만 격리한다. 화면 padding은 UI 표현이며 원본 authored pixel offset으로 주장하지 않는다.

## G2. 소스와 저작 후보

- `Client/Public/WorldSequencePlayer.h`, `Client/Private/WorldSequencePlayer.cpp`: typed sample, 성공한 시퀀스 clock cache, 실제 actor bounds collector.
- `Client/Public/Level_ValtanArena.h`, `Client/Public/MapTool.h`: 기존 player를 읽는 forwarding API.
- `Client/Private/MapTool_CameraShots.cpp`: 기존 연출 World 배우 섹션의 자막 행 목록·Add/Apply/Delete/Reset, 시작/길이·문구·Normal/Upper/Balloon·actor slot 편집. 같은 화면의 사운드 행 목록과 시작/길이/volume Apply·Reset도 추가했다. Apply는 기존 전체 World draft 검증 후 commit하고 실패하면 직전 row/template를 보존한다. 성공하면 기존 preview stale와 Save 경로를 사용한다. WorldObjectTool enum이나 별도 도구는 추가하지 않았다.
- `Tools/ValtanPipeline/build_valtan_subtitle_candidates.py`: 설치 GameMsg + 기존 원본 parse + 현재 WorldSequence의 stable instance/template 결합. 이미 가져온 row는 사용자 편집값을 보존한다.
- `Tools/ValtanPipeline/test_valtan_subtitle_candidates.py`: idempotence, 기존 편집 보존, clock·slot·문자열·시간 경계 검사.

후보는 `out/ValtanSubtitle20260920/candidate-files.json`에 baseline/candidate 경로와 SHA-256을 기록했다. baseline revision 19, candidate revision 20이며 5개 기존 template에 8개 행을 추가하고 그 외 필드는 보존한다. 이 작성자가 Data/Resources를 직접 교체하거나 publish하지 않았다. 통합 담당이 sound 후보와 동일 문서의 서로 다른 필드를 stable ID 기준 병합해 설치·publish했다. 최종 read-only 확인에서 저작 revision20과 runtime 모두 subtitle8/sound6이었다. `installed-readonly-validation.json`에 설치 파일 SHA-256과 행 수를 남겼다.

원본 근거는 `subtitle-manifest.json`에 DB·scene SHA-256, track export, 원본 key, row를 기록했다. `source-validation.json`은 원본 시작/길이 오차가 각각 0.5ms 미만이며 무관 필드가 동일함을 확인했다. `balloon-anchor-validation.json`은 원본 movement와 설치 track의 실제 시작 위치를 비교해 white 오차 0.00004136m, black 0.00003736m 이하를 기록했다. 이 수치는 source actor 결합 검사이며 GPU 화면 표시 성공을 대신하지 않는다.

## G3. 실행한 검증

- Python unittest 3개 PASS: 수입 시간/위치, 기존 row 수정 보존 및 재실행 불변, 잘못된 입력 거부.
- 실제 production `Collect_Subtitles`와 `Find_Template const` 함수 본문을 격리한 CPU fixture 14개 PASS: 시작/끝, 되감긴 clock, 정지 clock, 실패 sample, 종료 HOLD, 제거된 owner, actor 없는 balloon. 전체 GPU/Stop/Seek 함수 체인의 실행 검증으로 기록하지 않는다.
- 최종 WorldSequencePlayer.cpp + WorldSequencePlayer_Objects.cpp + MainApp.cpp + MapTool_CameraShots.cpp + MapTool_Cutscenes.cpp focused TU 5개 compile exit0: `out/ValtanSubtitle20260920/compile/compile.log`, `compile-exit.txt`. 공유 sound player와 sound/subtitle UI 변경 뒤 다시 실행했다. 기존 EngineSDK 인코딩 경고만 남았다.
- 변경 파일 `git diff --check` PASS.
- 실제 WorldSequence production codec Save/Reload/equivalence·UTF8/markup/position/slot/time·합계64/65·실패 rollback 30검사 PASS: `out/ValtanWorldTracks20260920/result.log`. 실제 설치 후보를 사용한 sound6/subtitle8 및 null 거부 포함 31검사도 PASS: `actual-result.log`.
- 실제 `Read-WorldSequenceDocument` publisher 함수의 정상 후보/roundtrip 승인과 invalid volume/path/missing/text 4종 거부 PASS: `out/ValtanWorldTracks20260920/publisher-result.log`, `publisher-actual-result.log`.
- UI의 실제 클릭·저장 제스처는 실행하지 않았다. UI 저장 소비자는 기존 document Validate/Save와 동일하며 위 codec 검사는 필드의 저장·재로드 보존을 검증한다.

## G4. 남은 적용·화면 경계

기존 제품 자동 연출인 입장·버러지들·사망에는 6개 문구가 붙는다. 흰/검 늑대 말풍선 2개는 이미 존재하는 MapTool source preview instance에 연결했다. 현재 제품 흐름에 자동 gate1 입장 cinematic이 없어, 이 변경으로 그 연출이나 camera/animation을 새로 만들지 않았다. 따라서 발탄 원본 8개를 모두 추출·소유 경로에 연결한 것과 8개 전부가 자동 raid flow에서 재생되는 것은 구분한다.

Client를 실행하지 않았고 실제 font 크기·balloon head projection·원본 연출과의 화면 판정은 사용자가 확인한다. 원본에 없는 대사나 string ID를 화면 텍스트로 생성하지 않았다.

## G5. 쿠크·발탄 전체 적용 범위

최종 설치 정본을 읽어 Kouku Action14 + Sequence17 = 기존 live 자막31행을 확인했다. `cin.37081_12_01`~`03` 앵콜 첫 진입 자막3개는 양쪽 resource catalog에 준비됐지만 presentation occurrence가 없으므로 재생 연결 완료로 세지 않는다. SCENE07A 첫 진입 연출 자체가 현재 흐름에 없다는 경계다. 발탄은 World8행 중 자동 입장·버러지들·사망6행과 기존 늑대 preview2행으로 구분한다. 원본에서 자막이 발견되지 않은 장면에 새 문구를 만들지 않았다.

`MainApp::RenderCinematicSubtitles`는 기존 `Font_YoonGasiIIM`/UILabelFont 제품 렌더러를 사용한다. 저장된 GameMsg 한국어 원문을 UTF-8→wide로 변환해 정상/상단·개행·그림자·폭 맞춤을 처리하고, balloon은 실제 actor bounds의 world 위치를 view/projection으로 투영한다. string ID는 근거/식별용이며 런타임 번역 lookup이나 새 i18n 체계, 새 폰트 리소스를 만들지 않았다. 화면의 최종 크기·위치·가독성은 사용자 확인 경계다.

Kouku의 codec56검사, projector 실제31행 소비, Python6검사와 원본 GameMsg 추출 증거는 같은 날짜 `2026-09-20_KOUKU_SCENE_AUDIO_SUBTITLE_RESULT.md` 및 `out/KoukuSceneAudioSubtitle20260920`에 기록했다. Valtan의 본 결과·source/anchor/collector 검사와 통합 Product 빌드는 각각 별도 증거로 구분한다.

## G6. 최종 소비자 리뷰에서 수정한 연출 사운드 수명

새 sound handle 경로를 실제 MapTool 소비자까지 대조하니, 기존 `Seek_EditorCutsceneWorld`가 PLAYING에도 매 프레임 `Set_Paused(true)`와 discontinuous seek를 호출해 소리를 계속 정지·재생성하는 불일치가 있었다. PLAYING은 unpaused continuous seek, Pause는 동일 clock 유지, 명시 scrub/되감기는 discontinuous seek로 분리했다. 같은 시각의 명시 scrub도 별도 요청 flag로 tail을 정리한다.

자연 종료에서는 마지막 visual/camera sample을 유지하면서 기존 `Retire_InstanceSoundTails`가 이미 시작한 handle만 분리한다. 이후에는 tail만 tick하며 끝에서 sound row를 다시 생성하지 않는다. 명시 Stop/Area 변경/도구 닫기는 기존 session instance별 stop으로 active와 retired handle을 함께 정리한다. draft refresh는 natural-end flag를 초기화해 현재 시각의 새 draft를 다시 보여준다. 통합 editor session이 활성인 동안 legacy ArenaRise가 같은 player를 두 번째로 Update하지 않는다.

실제 MapTool `Seek_EditorCutsceneWorld`, `Update_EditorCutscene`, `Stop_EditorCutscene` 본문을 사용한 consumer fixture9건 PASS: continuous play, pause, 같은 시각 scrub, rewind, natural end retirement, held end 재생성 없음, draft refresh, explicit stop, STOPPED의 중복 tail tick 방지. 외부 actor/camera/audio adapter는 격리했고 FMOD 청취나 GPU 검증은 아니다. 근거: `out/ValtanSubtitle20260920/compile/map-sound-consumer-probe.cpp`, `.exe`, `map-sound-consumer-build.log`. 최종 5개 TU 컴파일과 diff check도 다시 통과했다.

제품 Level의 explicit stop/clear와 CValtan의 M09 natural tail·BGM generation guard, MainApp의 value-copy 수집/dedupe에서 추가 메모리수명 회귀는 발견하지 못했다. M05/M09 BGM은 서버 권위 CValtan의 Music owner이므로 제품 Complete Play가 소비하며, MapTool World source preview의 6개 soundTracks에 BGM 자체를 복제하지 않았다. 따라서 MapTool preview의 원본 BGM 전체 일치로 범위를 확대해 설명하지 않는다.
