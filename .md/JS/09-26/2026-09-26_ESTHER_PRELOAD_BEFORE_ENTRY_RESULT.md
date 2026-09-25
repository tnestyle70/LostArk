# 에스더 소환 표현 입장 전 선로딩 결과

## 문제

- 소환 NPC 모델은 실리안·웨이·바훈투르 3명만 Loader에서 준비했다. 쿠크 로스터(니나브·웨이·이난나)의
  니나브 `NPC_59504`, 이난나 `NPC_59620`은 첫 소환 때 `CClientReplication`이 모델을 로드했다.
- 컷인 flipbook DDS(5명 합계 575장, 약 500MB)는 첫 발동 때 메인 스레드 `CUITextureCache`가 로드했다.
- 에스더 사운드는 첫 재생 때 FMOD `createSound`(FMOD_DEFAULT, 전체 디코드)가 실행됐다.
- 이펙트 문서는 이미 `CLevel_Loading`이 5명 모두 priority 준비 중이었다.

## 변경

- `CLoader::Ready_EstherSummonPresentation(level)`: Character Select·Valtan·Kouku의 중복 루프 3개를 대체한다.
  5명 모델 prototype, 컷인 프레임, `CEstherActionSoundCueDocument::Preload_Sounds()`를 Loader worker에서 준비한다.
- 컷인: `CEstherCutinPresentationService::Preload_Frames`가 worker에서 SRV를 만들어 mutex 대기열에 넣고,
  메인 스레드 `Update`가 `CUILayoutRuntime::Adopt_Texture`로 캐시에 넣는다. 이미 있는 항목은 교체하지 않는다.
  archetype별 1회만 로드한다(프로세스 수명 캐시, 기존과 동일).
- `CUITextureCache`: `Load_Texture`(static)와 `Adopt`로 분리. `Get_Or_Load` 동작은 동일.
- `CSound_Manager::Find_Or_LoadSound`: `m_Sounds` 조회·삽입에 mutex를 추가해 worker preload와 메인 재생의 경합을 막는다.
  `createSound`는 잠금 밖에서 수행하고 중복 삽입 시 새 Sound를 release한다.

## 검증

- Debug Product 빌드 PASS (Engine → Shared → Server → Client, OBJ 198 재컴파일, CSO 0).
- `git diff --check` 통과.
- 로컬 Server(`127.0.0.1:7777`)와 Client 실행까지 준비. 첫 소환 hitch 제거와 로딩 시간 증가 여부의 화면 확인은 사용자 몫이며 미확인.

## 남은 경계

- 컷인 5명분 약 500MB를 첫 입장에서 읽으므로 로딩 시간이 늘어난다. 월드 로스터 3명으로 줄일지는 사용자 확인 후 결정.
- 에스더 로스터 5개 ID는 Loader와 Level_Loading에 하드코딩되어 Server `ESTHER_DEFINITIONS`와 수동 동기다.
