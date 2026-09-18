# 보스 돌진의 골격 잔상 구현 계획

## G00. 원본 occurrence와 표시 경계

첨부 네 번째 이미지에는 몸체와 무기 윤곽이 과거 위치에 흰색으로 남는다. 원본 Action4219863의 TrailGhostEffect는 stage000의0.209004초와 stage001의0.198055초에 있다. 현재 P80의 두 번째 stage는 source stage001/22_04다. 발탄420604의4_01에는2.4617초 TrailGhostEffect가 있으며 현재 VALTAN_DASH_CHARGE의 CHARGE는 원본2450~3350ms를0.6배속으로 소비한다. 발생 종류와 source 시각을 보존하되 serialized payload의 전체 재질/수명 ABI는 복구되지 않았다. 흰색 표시와50ms 간격·250ms fade는 사용자 요청에 대한 PROJECT_AUTHORED 표시값이며 원본 exact shader라고 기록하지 않는다.

## G01. 현재 CModel pose의 불변 복사

Engine Model의 Capture_BoneMatrices는 현재 골격으로 mesh palette를 생성하고 유한성 검증 뒤 출력 벡터를 교체한다. live pose, animation cursor, palette cache와 resource를 변경하지 않는다. 기존 CModel mesh를 렌더하며 두 번째 모델 loader/runtime을 만들지 않는다.

Client SkeletalAfterimage는 body/weapon별로 현재 World와 palette를 최대6개 보존한다. 실제 이동 중에만50ms 간격으로 저장하고250ms에 제거한다. 정지, 숨김, 모델 변경, 비유한 입력과 큰 시간/위치 불연속을 처리하며 실패한 표시만 격리한다. Render는 기존 binary animated shader의 마지막 pass14로 BLEND에 제출하고 live World/palette를 복구한다. snapshot pose로 gameplay 판정하거나 transform을 전송하지 않는다.

## G02. 실제 Server presentation 소비자

CNpc의 Set_ChargeAfterimageEnabled는 Kouku presentation owner가 Server P80 시각으로 켠다. Late_Update에서 이미 동기화된 body와 지팡이 pose/world를 저장한다. Body_Valtan은 CValtan이 Server dash-charge action에만 켠 bool을 소비한다. 다른 animation과 기존 Trail에는 전파하지 않는다. 새 C++ 파일 SkeletalAfterimage.h/cpp만 Client project/filter에 등록하며 root 작업이 등록과 제품 빌드를 통합한다.

## G03. 검증

변경 TU의 최소 컴파일, HLSL FXC 및 보존된 기존 shader pass 확인, palette/World 불변성과 queue 수명·유한성·오류 격리의 focused 검증, git diff --check를 실행한다. Client/UI 실행·조작·캡처는 하지 않으며 최종 흰색 형태·밀도·시각은 사용자가 판정한다.

## G04. Effect V1 모델 큐의 잔상 전용 표시

optional modelCue.afterimage가 현재 표면 대신 기존 CSkeletalAfterimage를 그린다. 원본 WModel/clip, CModel Prototype/Clone, 명시 cue clock과 Local*RootWorld 계산을 재사용한다. emissionStartSeconds는 원본 notify 시작이며, emissionEndSeconds는 원본 종료값을 확인하지 못했으므로 원본 clip 끝으로 설정한 저작값이다. sampleIntervalSeconds/sampleLifetimeSeconds/maxSamples와 색도 PROJECT_AUTHORED다. 현재 actor는 기존 source model preview가 표시한다. 원본 skeleton의 root-motion vertical axis와 scale을 선택적으로 저장해 같은 owner root 이동을 중복 적용하지 않는다. 발탄은 몸체와 animation set이 분리되어 있으므로 optional animationSetAssetId를 기존 CModel::Attach_AnimationSet으로 연결하고 골격 불일치 시 stage를 거부한다.

각 occurrence의 MODEL_CUE_RESOURCE만 history를 소유하며 prepared prototype과 다른 occurrence에 공유하지 않는다. 같은 sample clock은 history를 진행시키지 않고 pause를 유지한다. 명시 seek/Reset/숨김/재stage와 역방향·큰 시간/위치 불연속은 이전 history를 제거한다. seek 시 확인하지 않은 과거 world를 임의 보간하여 만들지 않으며 재생을 계속하면 실제 sampled pose부터 다시 쌓는다. 종료는 emission 이후 마지막 sample lifetime까지이며 Play All과 runtime duration이 같은 cue-end helper를 소비한다. 제자리 Model View에서도 실제 골격 변화가 있으면 기록하고 동일 pose 반복은 억제한다.

기존 modelCue JSON parse/write/validate와 Detail draft에 계약을 연결한다. 실행 중 Data는 직접 배포하지 않고 out 후보 문서에서 codec roundtrip, 잘못된 입력 거절, 수명·reset, 설치 WModel source clock/world palette를 검증한다. Client/UI·제품 빌드는 실행하지 않는다. 신규 C++ 파일은 없으므로 project/filter 추가는 필요 없다.
