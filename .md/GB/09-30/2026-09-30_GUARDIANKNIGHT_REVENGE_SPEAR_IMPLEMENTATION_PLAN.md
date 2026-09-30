# GuardianKnight 리벤지 스피어 방향과 원본 홀드 클립 지속 구현 계획

## G00. 현재 정본과 변경 경계

요청은 리벤지 스피어의 hold 방향을 반시계 90도로 돌리고 한 번의 찌르기 동작 동안 검격을 유지하는 것이다. 사용자가 언급한 775~2169는 대략적인 값이며 원본 클립에 맞추기로 확인했다. 설치된 GuardianKnight.wmodel의 ddk_sk_brutalmarch_01/02/03은 각각 20/30, 40/30, 68/30초다. skill 49210의 HOLD stage1은 clip02, rate1, 1333ms이며 원본 기준 전체 구간 약666.667~2000ms에 해당한다.

Data/Animation/Authored/GuardianKnight/GuardianKnight.skillbindings.json → GuardianKnight.animevents의 clip02 start0 cue → effect.guardianknight.skill.49210.clip.1.full.restore → CCharacter/CEffectPresentationService/CEffectPlayback이 기존 소비 경로다. stage의 기존 HOLD 동작과 Server timing은 바꾸지 않는다. Data 원본과 실행 중 도구를 변경하지 않고 out/GuardianRevengeSpear20260930에 원본 snapshot, 후보와 검증 증거를 준비한다. 사용자 편집 중이므로 최종 반영은 최신 디스크 저장본을 다시 읽어 stable ID와 변경 필드만 병합한다.

## G01. 최소 데이터 후보

clip1의 b_root 방향 그룹만 detail.transform의 위치와 Y 회전을 원점 기준 반시계90도로 바꾼다. 원본 00_01/00_01-2 검격, 00_07 차지, 00_05 주변 입자를 대상으로 한다. 손 본에 붙는 장갑, 척추 고리, screenPost, light는 보존한다. 전역 particleSystem 회전은 사용하지 않는다.

원본 앞 검격 00_01은 start0/emitterDuration0.3, 이어지는 00_01-2는 start0.3/emitterDuration0.2, 모두 유한 loop1이다. 두 번째 그룹의 sourceRecipe.emitterDurationSeconds와 detail.timing.lifeTime을 원본 clip02 끝에서 start와 emitterDelay를 뺀 길이로 늘린다. particle lifetime, velocity, spawn rate와 burst, 원본 material 곡선과 asset은 보존한다. 애니메이션 반복과 무한 owner loop를 새로 만들지 않는다. 실제 재생 결과로 끝 구간 방출과 자연 소멸을 구분해서 확인한다.

## G02. 검증과 반영 조건

기존 실제 Effect_DocumentCodec/Effect_Playback을 Release OBJ로 out 전용 probe에 링크해 baseline과 후보를 같은 고정 anchor basis에서 재생한다. sample별 검격 입자 수, alpha, 방출 구간의 finite transform과 반시계90도 emitter basis를 비교한다. 고정 synthetic anchor 검증은 실제 GPU 화면 검증과 구분한다. 설치 WModel의 clip02와 실제 본 샘플링, 제품 Build_SourceBoneAnchorWorld 정규화 함수를 사용한 무창 CPU geometry 검증을 추가한다. synthetic camera 결과와 GPU 화면 판정은 구분한다. JSON parse와 scoped effect validation, baseline hash 보존을 확인한다. 제품 C++/project/filter 등록 변경은 없다.

Data 설치/publish와 실행 중 도구 Reload는 별도 단계다. 원본 JSON 교체, Client 실행/조작, 외부 draft 폐기는 이 후보 작업에서 수행하지 않는다. 실제 검증한 범위만 RESULT에 기록한다.

## G03. 캐릭터 전방 기준의 독립 검격 그룹과 그룹 미리보기

후속 사용자 요청은 바닥으로 향하는 검격을 GuardianKnight 전방으로 보내고 그룹의 위치와 회전을 함께 편집하는 것이다. 기존 b_root 그룹은 검격25개와 light2개, 비활성 screenPost2개를 함께 포함한다. Play Group이 비활성 screenPost까지 Solo admission에 제출하여 그룹 전체가 거절되는 경로를 확인했다. 기존 live edit는 새 Document와 anchor history를 다시 준비하므로 저장본을 재사용하는 결함으로 단정하지 않는다.

검격25개는 기존 manual group 및 고유 runtime anchor slot 계약으로 분리한다. 실제 b_root 위치와 BONE 부착을 유지하고, 설치 모델에서 측정한 본 회전을 socket 회전으로 상쇄해 캐릭터 전방을 사용한다. 현재 후보는 socket [90,180,0]도다. source recipe가 있는 OWNER_YAW는 기존 codec에서 거절되므로 그 경계를 완화하지 않는다. 각 Element의 현재 위치·회전·시간·재질은 보존하고 source locked-axis sprite는 기존 followEmitterAxisRotation을 연결한다. 최신 디스크 snapshot을 기준으로 후보를 만들며 사용자의 이전 수동 편집을 덮어쓰지 않는다. 정확한 전방·단위는 설치 GuardianKnight.wmodel과 제품 preTransform Scale(.0001)*Ry(-90), clip02의 실제 본으로 측정한다.

그룹 미리보기는 명시적으로 꺼진 Light/ScreenPost만 재생 선택에서 제외하고, 켜진 presentation 및 다른 미지원 요소의 오류 검사는 유지한다. 단일 Solo의 거절과 문서 내용은 보존한다. 기존 Effect_Tool_CatalogPreview.cpp의 선택 진입점만 수정하므로 신규 C++/project/filter 등록은 없다. 실제 문서의 29개 선택→27개 재생 대상, 검격25개 독립 그룹, Save/Load와 그룹 위치·회전 변환, 서로 다른 owner yaw 및 원본 socket/scale 경계를 집중 검증한다.

데이터 후보가 완성되면 저장 기준과 반영 승인을 한 번 확인하고 stable ID/field 병합·hash 재확인·백업·원자 교체를 수행한다. C++ 변경은 정상 Product 빌드와 필요한 최소 컴파일을 수행하되 현재 실행 중인 Debug Client/Server를 임의 종료하지 않는다. 실행 중 메모리 Reload와 최종 화면 판정은 사용자가 한다.
