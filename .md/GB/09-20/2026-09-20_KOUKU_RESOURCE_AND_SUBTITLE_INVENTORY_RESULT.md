# 쿠크 이번 변경의 리소스·자막 목록

범위는 `2026-09-20_RAID_PRESENTATION_REPAIR_IMPLEMENTATION_PLAN.md` G12~G17의 이번 수정이다. 기존 dirty 작업 전체를 이번에 생성한 리소스로 계산하지 않는다. 설치·게시 완료 증거는 `out/KoukuPlaybackRepair20260920/integration/install.receipt.json`과 encore/valtan 하위 설치 receipt·publisher 로그를 따른다. 마지막 쇼타임 높이·노란 장판·룰렛 보정에서는 새 리소스를 추가하지 않았다. 추출 완료와 실제 재생 연결, 사용자 화면 확인은 별개다.

## 이번에 설치한 파일

Resources에는 WAV 12개를 추가하고 기존 발탄 M09 WAV 1개를 교체했다. PNG, DDS, 모델, 폰트는 새로 설치하지 않았다. 원본 미디어 추출과 설치·재생 연결을 아래처럼 구분한다.

| 종류 | Resources 상대 경로 또는 이름 | 용도·연결 |
|---|---|---|
| 쿠크 WAV 1개 | `Sound/KoukuSaton/Events/scene_midnightc_ed_koukuskill.source.wav` | 쇼타임 원본 2개 layer, 7,395ms. 재생 연결 완료. |
| 쿠크 WAV 3개 | 같은 폴더의 `scene_midnightc_ed_koukustopclearingdungeon.source.wav`, `scene_midnightc_ed_popup1.source.1.wav`, `.source.2.wav` | 앵콜 첫 진입 원본 대사·효과음과 동일 확률 popup 변형 2종. 카탈로그와 인계 데이터 준비, 새 입장 시퀀스 연결은 다른 작업자 소유. |
| 쿠크 WAV 2개 | 같은 폴더의 `bgm_midnightc_ed_m18_scene_fakeclear.source-stop.wav`, `bgm_midnightc_ed_m20_scene_finish.source-stop.wav` | 원본 Stop/fade를 반영한 첫 진입·마지막 엔딩 BGM. 마지막 엔딩은 P75/P9 연결. 첫 진입은 인계 준비. |
| 발탄 WAV 6개 | `Sound/Valtan/Scenes/scene_heartrb_ed_lictusappear_foley.wav`, `scene_heartrb_ed_valtanappear.wav`, `scene_heartrb_ed_valtandeath.wav`, `scene_heartrb_ed_valtanhowl.wav`, `scene_heartrb_ed_valtanskill_lock.wav`, `scene_heartrb_ed_valtanskill_widearea.wav` | 기존 WorldSequence 6개 sound 행으로 연결. 늑대 gate1-entrance는 기존 World 미리보기 범위. |
| 발탄 WAV 교체 1개 | `Sound/BGM/Valtan/EventMixes/bgm_heartrb_ed_m09_mscene_valtandead.wav` | 원본 100ms 시작 지연과 22.82초 Stop+3초 fade. 기존 Music 소유자를 유지. |
| Effect V2 저작 JSON 1개 | `Data/Effects/V2/Authored/bingo.hammer.warning.arrow.effectv2.json` | 기존 DDS로 빨간 망치 경로 화살표, UV 이동·3초 경고. Client `96.DataFiles` 등록. |

기존 카드미로 WAV는 원본 두 layer가 이미 들어 있어 유지했다. 잘렸던 SOUND occurrence를 11,500→16,827ms로 늘렸다. 마지막 앵콜 대사도 기존 WAV에 원본 두 layer와 전체 51,185ms가 있으므로 재추가하지 않고 P75/P9 행을 전체 길이로 늘렸다. 엔딩 전체는 51,285ms이며 원래 animation Stage는 유지한다.

앵콜 fade는 원본 bank의 SineRecip 곡선을 wwiser 근사식으로 렌더한 것이며 Wwise의 bit-exact 출력이라고 주장하지 않는다. 실제 WAV hash·길이·원본 cue는 `out/KoukuEncoreAudio20260920/resource-manifest.json`, `render.receipt.json`, 발탄은 `out/ValtanSceneSound20260920/render-manifest.json`에 있다. 아직 소비자 연결 근거가 확정되지 않은 발탄 collapse 3변형/M02는 추출물로 보존하고 설치 목록에 포함하지 않았다.

## 쿠크 자막

원본 `EFTable_GameMsg.db`의 한국어 16개 문구를 기존 연출 9개에 31행으로 연결한다. Action Composition에 14행, Sequence Composition에 17행이며 중복 연출 문구가 양쪽에 존재한다. 새 PNG/텍스처 없이 JSON `SUBTITLE` resource에 plain UTF-8 `subtitleText`, `subtitlePosition`을 저장하고, occurrence `startMs`와 `durationMs`로 표시 시간을 제어한다. 원본 GameMsg ID를 보존한다.

MainApp은 Presentation Player의 현재 활성 자막을 기존 `Font_YoonGasiIIM`으로 흰색과 그림자를 사용해 표시한다. NORMAL은 아래, UPPER는 위에 놓으며 HUD를 숨기는 연출에서도 표시한다. 현재 한국어 직접 저장 구조이고 언어별 문구 선택이나 다국어 fallback font는 구현하지 않았다. 이 폰트가 원본 연출의 정확한 서체와 동일하다고 판정한 것은 아니다.

연결 대상은 Action P73/P74/P75/P76, Sequence P3/P4/P5/P7/P9다. 원본 시간과 문구별 목록은 `out/KoukuSceneAudioSubtitle20260920/subtitles/subtitle-manifest.json`을 따른다. 빙고 입장 SCENE07A 3문구는 Action/Sequence의 SUBTITLE 리소스로 추가하고 `out/KoukuEncoreAudio20260920/bingo-intro-audio-subtitle-handoff.json`에 정확한 occurrence 후보를 준비했다. 실제 입장 시퀀스에는 아직 연결하지 않았다. SCENE02A 말풍선 연출 1문구도 추출 상태로 보존한다. 다른 작업자의 빙고 카메라/애니메이션에는 임의로 행을 삽입하지 않았다.

## 발탄 자막

원본 일반/상단 6개와 배우 말풍선 2개, 총 8개 문구를 기존 WorldSequence의 `subtitleTracks`에 연결하고 source revision20을 게시했다. 원본 GameMsg 한국어를 사용하며 같은 World instance의 실제 재생/Seek/Pause/Stop 시간을 읽는다. 말풍선은 해당 visible 배우 모델의 실제 bbox 상단을 투영한다. 늑대 두 문구는 기존 gate1 World 연출 미리보기 범위이며 제품의 자동 1관문 입장 연결까지 새로 만들지는 않았다.

기존 MapTool 연출 World 배우 섹션의 Subtitles 행에서 시작/길이 ms, 문구, NORMAL/UPPER/BALLOON, actor slot을 편집한다. Apply는 전체 draft 검증 후 commit하며 기존 World Save 경로를 사용한다. 잘못된 입력은 기존 template를 보존한다. `out/ValtanSubtitle20260920/subtitle-manifest.json`과 기능 RESULT가 원문·시간·배우 검증을 기록한다. 언어 선택과 번역 catalog/fallback font는 아직 구현하지 않았다.

## 빙고에서 확인할 표시

| 항목 | 도구의 stable ID | 이번 변경 |
|---|---|---|
| 빨간 화살표 | `bingo.hammer.warning.arrow` | 신규 Effect V2 leaf. 3초, UV 이동, 3.04×18.24m |
| 머리 폭탄 표식 | `bingo.bomb.mark` | 기존 리소스 재사용, 5초 주기 서버 연결 |
| 폭탄 설치·도화선·폭발 | `sequence.kouku.bingo.bomb.planted` | 기존 Object 원본 재사용, 설치 3초 후 폭발과 원래 잔여 수명 보존 |
| 일반 해골 | `sequence.kouku.bingo.skull.white` | 기존 리소스, 최초 2칸과 폭발 전파 |
| 뒤집히는 해골 | `sequence.kouku.bingo.skull.flip` | 기존 애니메이션, 일반 해골 생성 |
| 빨간 해골 전환 | `world.object.instance.kouku.bingo_skull.red_flip` | 새 Motion instance가 기존 flip 이후 red로 연결 |
| 빨간 해골 | `sequence.kouku.bingo.skull.red` | 기존 리소스, 일반 해골에 재폭발하면 전환 |
| 좌→우 망치 | `sequence.kouku.bingo.hammer.x_plus` | 기존 원본, 경고 3초+하강 1.4초+이동 1.6초 |
| 우→좌 망치 | `sequence.kouku.bingo.hammer.x_minus` | 동일 |
| Z+ 망치 | `sequence.kouku.bingo.hammer.z_plus` | 동일 |
| Z− 망치 | `sequence.kouku.bingo.hammer.z_minus` | 동일 |

전체 Parent는 `빙고_반복전투` (`KAKULSAYDON_G1_PATTERN_96`), duration Logic은 `빙고_바닥_망치생성` (`kakulsaydon.g1.logic.99`) 50,000ms다. Saved Flow `kakulsaydon.flow.bingo`가 기존 3관문 P62/P94/P60/P67/P40을 사용한다. 마지막 공격이 40,477ms에 끝난 뒤 Parent 종료까지 바닥 로직은 계속된다. 다음 반복에서 보드와 남은 폭탄 수명을 보존한다.

## 유리 깨짐 추출물

`out/KoukuBingoGlass20260920/README.md`가 인계 시작점이다. SCENE07A 화면 균열 12.500001~15.433334초, 파편 시작 15.433334초, 파편 12그룹과 spark 2그룹의 원본 설정을 확보했다.

- DDS 4개: `fx_d_environ_072_nomipmap.dds`, `fx_d_environ_073_nomipmap.dds`, `fx_d_normal_097.dds`, `fx_b_atypical_004.dds`.
- 원본 shader DXBC 5개와 disassembly, MIC/parent 재질 입력, 타이밍 JSON, 원본/출력 hash receipt.
- 기존 파편·spark V1 리소스 4종과 참조 파일 존재 확인. 신규 실행 리소스 복사는 하지 않았다.

추출 완료 상태다. 화면 균열을 실제 runtime screen material로 재생하는 연결과 빙고 입장 카메라/애니메이션 연결은 아직 하지 않았다.

## 확인 범위

자막 실제 저장/재로드 56검사, Product 자막 31행과 4096byte 경계, Python 6검사, WAV FMOD 무음 장치 로드 검사, World/EffectV2 실제 codec 및 4방향 transform 31검사를 통과했다. Client 실행·화면 판정은 하지 않았다. 발탄 병합 World 실제 codec 31검사, 실제 publisher 검증, 자막 collector14검사와 말풍선 실제 actor 위치 대조를 통과했다. 발탄 FMOD 무음12검사·World sound lifecycle21검사 및 앵콜 설치 WAV5개 hash/clock/FMOD 무음 검사를 통과했다. 최종 통합 빌드와 서버 실행 검사는 주 RESULT의 최신 G12 이후 기록을 따른다.
