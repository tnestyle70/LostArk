# 마하라카 워터팡 Camera·발판·효과음 등록 결과

## 요청과 현재 경계

요청은 영상처럼 워터팡 전체를 구현하고 MapTool Camera에서 편집 가능하게 만드는 것이다.
이번 결과는 그중 원본 도입 카메라 2종, 기존 발판 18개의 동작 4종, 시작·붕괴 효과음까지다.
**전체 이벤트 구현 완료가 아니다.** 원본 배우·파티클·BGM·게임 진행은 아래 미완료 목록을 따른다.

참고 영상: https://www.youtube.com/watch?v=9dolMRcm_JE&t=135s
브라우저에서 영상의 경기 중 물줄기와 이후 외곽 발판 붕괴를 관찰했다. 이것은 구현된 Client의
육안 검증이 아니다. 영상만으로 정확한 시작 버전·모든 서버 이벤트 시간을 확정하지 않았다.

## 원본 연결과 적용

- SCENE03B 원본: ReleasePC/Packages/A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk.
- SHA256: 804afbb33121d25b43c2d5f5d9690fb88ab923be873b6d24f9fc9037fb50b929.
- source Matinee variableLinks → InterpGroup → actor.staticmeshcomponent → sourcePlacementId
  → 기존 stable placementId로 18개를 정확히 결합했다. 이름·가까운 위치 조인은 사용하지 않았다.
- 원본 cm/축 변환 뒤 기존 배치의 역회전으로 local offset/quaternion을 만들었다.
  배치 transform·모델·재질·물·조명·NPC는 이번 변경에서 수정하지 않았다.
- source Hermite 곡선을 재샘플해 중간 오차 1mm/0.05도 이내로 검증했다.
  무대 복구 +180/-180 Euler 키는 임의 정규화하지 않았다.

| Camera 목록 | 원본 Matinee/Data (1-based) | 길이 | 실제 연결 |
|---|---|---:|---|
| 워터팡 / 원본 도입 카메라 15 (카메라·효과음) | 43/158 | 9507ms | 2 camera cuts, 기본 자세 18개, 효과음 172ms |
| 워터팡 / 원본 도입 카메라 20 (카메라·효과음) | 45/160 | 11735ms | 3 camera cuts, 기본 자세 18개, 효과음 403ms |
| 워터팡 / 원본 바닥 흔들림 (맵 동작 미리보기) | 41/156 | 3004ms | 18개 발판 |
| 워터팡 / 원본 바닥 붕괴 (맵 동작 미리보기) | 42/157 | 5000ms | 18개 발판, 효과음 0ms |
| 워터팡 / 원본 바닥 붕괴 상태 (맵 동작 미리보기) | 44/159 | 1000ms | 18개 발판의 무너진 자세 |
| 워터팡 / 원본 바닥 복구 (맵 동작 미리보기) | 46/161 | 2000ms | 18개 발판 |

붕괴 마지막 키는 약 3.644초지만 InterpLength 생략값은 설치본 Engine.u의
Default__InterpData(packageIndex17330).InterpLength=5.0이다. 5초까지 마지막 자세를 유지한다.
원본 ta_grounddestroy는 loop이며 이번 Camera 항목은 1초 상태 미리보기다. 서버 loop 복원이라고
주장하지 않는다. 네 발판 동작은 같은 배치를 소유하므로 서로 독립된 컷신으로 등록했다.
도입 2개 중 어떤 것이 사용자 영상의 시작인지 아직 사용자의 확인이 없다.

## 효과음과 배포 리소스

SOUND_SCENE_OCEAN1/3의 Event → 단일 Play → Sound → media를 원본에서 추적했다.
기존 WorldSequence soundTracks 경로이며 새 더미 모델이나 C++ 런타임을 만들지 않았다.

| Resources 상대 asset ID | 원본 event / media | 재생 길이 |
|---|---|---:|
| Sound/Maharaka/WaterpangSource/scene_maharakap_waterpangstart.wav | 129050398 / 714113137 | 9331ms |
| Sound/Maharaka/WaterpangSource/scene_maharakap_fallout_foley.wav | 360619994 / 427337176 | 7485ms |

물리 위치는 Client/Bin/Resources/Sound/Maharaka/WaterpangSource/이다. 팀 배포 때 두 WAV가
별도로 필요하다. 원본 WEM payload와 후보를 대조했고 WAV digest도 검증했다.
Wwise bus gain/전체 믹스 복원 완료는 아니다. 붕괴 효과음은 영상 길이 5초를 넘어 자연 tail로
재생되며 명시적 Stop에서는 종료된다. 저속 프레임의 MapTool 시간 보정(100ms cap)과 오디오
실시간 시계 차이 때문에 10FPS 미만 환경의 동기화는 별도 개선·검증이 필요하다.

## 저장·게시 경로

- Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/의 .worldsequences.json과 .camerashots.json이 정본이다.
- MapCatalog에 sourceSequences/sequences/sourceCameraShots/cameraShots를 연결했다.
- Client.vcxproj와 filters의 96.DataFiles None 등록을 추가했다.
- WorldSequence 6 templates / 6 instances, Camera 6 cutscenes / 5 shots, 현재 revision3.
- Publish-MapAuthoring.ps1로 Client/Bin/DataFiles/Map에 게시했다. 생성물 직접 편집 없음.
- Maharaka Level_Development에 자동 시퀀스 재생은 연결하지 않았다. enabled는 입장 시 자동
  실행 보장이 아니다. Camera에서 선택하여 Play하는 편집 프리뷰다.

## 실행한 검증

| 검사 | 결과 |
|---|---|
| test_maharaka_waterpang_sequences.py | 8 tests PASS |
| Publish-MapAuthoring Area Validate | PASS, 4671 placements / 7 files |
| Publish-MapAuthoring Area Publish | PASS |
| Publish-MapAuthoring Area Check | PASS |
| Foley installer dry-run | files=0, 현재 저장본과 후보 일치 |
| JSON/XML parse 및 diff whitespace | PASS, authoring/runtime JSON 2종 byte 일치; 신규 파일 whitespace도 검사 |
| C++ build | 미실행: 이번 변경은 Python·문서·데이터·WAV이며 C++ 변경 없음 |
| Client/UI 실행·화면 캡처 | 미실행, 사용자 전용 |
| 실제 화면·소리·영상 일치 | 사용자 확인 대기, visual PASS 아님 |

추가 광역 검사 test_world_sequence_authoring_contract.py는 42개 중 39개 통과·3개 실패했다.
실패는 MapTool 선언/정의 텍스트 검사, CardMiro prototype 등록의 이전 코드 literal 검사,
walkable reflected-object 기대값 검사다. 관련 기존 C++/해당 검사 파일은 이번 작업에서
수정하지 않았다. 따라서 광역 regression PASS라고 보고하지 않는다.

## 미완료 / 후속 작업

1. 도입 씬의 배우 애니메이션·물 튀김·blur·fade·material parameter 등 원본 비카메라 트랙.
   원본은 source-registration-report.json의 unconsumed 트랙 목록으로 남겼다.
2. 워터캐논·모코코의 action4225601 배우 애니메이션과 얼굴/물줄기/바닥/마무리 파티클.
   source notification 시점은 각각 0.591559 / 2.292587 / 2.390214 / 5.071869초다.
   소켓 FX_01과 root 효과를 구별해야 하며, 효과 경로 이름만으로 연결 완료로 처리하면 안 된다.
3. 회전·공격 AI, 소환/위치 전환, 경기 시작·생존·종료 순서. TriggerMapData57011과
   원본 AI action 연결을 Server 권위 이벤트와 presentation 경계로 구현해야 한다.
4. 붕괴에 따른 서버 support/collision/navigation/낙사. 현재 동작은 시각적 프리뷰뿐이다.
5. BGM start/skipend의 MusicSwitch(type13) 및 Pause/Stop/Resume 상태 전환.
   기존 Sound resolver가 0 media를 반환하는 것은 원본 삭제 근거가 아니다.
6. MapTool은 MAP_PLACEMENT/DEPLOY_PLACEMENT/OBJECT_RESOURCE를 대상으로 한다.
   현재 서버 NPC를 로컬로 복제·이동시켜 제품 이벤트인 것처럼 만들지 않는다.
   NPC 애니메이션 편집 프리뷰와 서버 실행의 소유 경계를 확인한 뒤 기존 typed 경로를 확장한다.

## 사용자가 직접 확인할 경로

마하라카 입장 → F1 → Open Map Tool → Camera → 컷신 목록 → 위 워터팡 항목 → Play.
한 항목씩 확인하고 Stop한 뒤 다음 항목을 선택한다. 끝에 도달하면 편집기는 마지막 자세를
유지하며, Stop이 기존 배치 자세와 카메라를 복구한다. 동작만 있는 항목에는 카메라가 없다.
시간 이동·재생·Stop·Save·재로드를 확인하고, 도입15/20 중 영상에 맞는 버전을 결정해야 한다.
현재 확인 시 Client/Server 프로세스는 꺼져 있었다. 에이전트가 시작하지 않았다.

## 재현 도구

- Tools/MapPipeline/build_maharaka_waterpang_sequences.py: 원본 곡선·정확 배치 결합.
- Tools/SoundPipeline/audit_maharaka_waterpang_audio.py: 원본 Event/media 근거.
- Tools/MapPipeline/install_maharaka_waterpang_foley.py: G01 후보 소유권 검사 후 G02 원자적 반영.
- Tools/MapPipeline/test_maharaka_waterpang_sequences.py: 정상/ID 오류/키 제한/편집 보존/rollback.

G01 최초 후보와 before 백업은 out/MaharakaWaterpang20260927/에 있다. installer는 이 고정
후보가 있어야 실행된다. 사용자 편집 후 무조건 재생성하는 도구가 아니며 충돌 시 거부한다.
source-registration-report.json은 초기 G01의 판독 기록이므로 붕괴 길이는 본 결과의
Engine CDO 기반 5000ms와 최종 authoring을 우선한다. commit/push는 하지 않았다.
