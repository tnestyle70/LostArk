# 보스 돌진 골격 잔상 구현 결과

## G00. 원작 근거와 소스 반영

첨부 이미지4를 열람했다. 몸체·무기 윤곽이 과거 위치에 흰색으로 남는 형태를 관찰했으며 화면 판정이나 원본 재질의 단독 증거로 사용하지 않았다. 원본 Action4219863의 stage000/22_01 TrailGhostEffect는0.2090039998초, stage001/22_04는0.1980549991초에 있다. 발탄 Action420604 stage003/4_01의 TrailGhostEffect는2.4617359638초다. 원본 payload와 source path/hash는 out/BossChargeAfterimage20260917/source-notifies.json에 보존했다.

CModel::Capture_BoneMatrices는 mesh offset × 현재 combined bone을 별도 벡터로 계산하며 finite 확인 뒤 출력만 교체한다. animation cursor, live pose, palette cache를 변경하지 않는다. 새 CSkeletalAfterimage는 같은 CModel의 immutable World/palette 복사를 최대6개 보관하고50ms마다 실제 제시된 pose 하나를 저장하며250ms에 제거한다. 멈춘 위치에 다중 밝은 복사본을 축적하지 않는다. pause, model 교체, teleport,250ms를 넘는 시간 불연속과 비유한 입력을 처리한다. 캡처/렌더 오류는 해당 표시만 격리하며 다음 enable 주기에서 재시도한다.

기존 binary animated shader에 pass14를 append했다. pass0~13 및 기존 dirty shader 분할 변경을 보존했다. pass14는 기존 BINARY_ANIMATED_NATIVE_PASS_POLICY를 따라 기본 Effect에서 ProgramVariantPass=1(BASE), SourceGroup 파생 Effect에서2(UNAVAILABLE)를 선언한다. afterimage PS는 기본 Effect에서만 컴파일하며 파생 Effect는 NULL을 보관한다. 따라서 stale source character program이 남아 있어도 기본 PS를 사용하고 파생 Effect의 직접 Begin(14)는 거부한다. 기존 VS의 bone palette 입력과 BLEND scene/bloom 출력을 재사용한다. Render 종료와 실패에서 live World와 palette를 다시 bind하며 실제 모델 pose는 처음부터 수정하지 않는다.

CNpc는 Kouku presentation owner의 Set_ChargeAfterimageEnabled를 받고 실제 body와 skinned 지팡이의 world/palette를 Late_Update에서 저장한다. Body_Valtan은 CValtan의 Server-approved VALTAN_DASH_CHARGE / valtan.attack.dash-charge.active에서만 활성화한다. 원본2450ms부터0.6배속으로 재생하는 현행 stage에 맞춰(2.461735964-2.45)/0.6초부터 켠다. hidden/dormant는 history를 즉시 비운다. 최초·교체 Body descriptor 모두 동일 bool owner를 소비한다. 신규 header/CPP의 project/filter 등록과 Kouku caller 연결은 root 통합 작업에서 완료했다.

## G01. 실행한 검증

Model, SkeletalAfterimage, Npc, Body_Valtan, Valtan의 실제 변경 TU 격리 Debug 컴파일5개가 성공했다. 발탄의 정확한 원본 시각 반영 후 해당 TU를 다시 컴파일했다. 기존 포함 헤더 C4828 경고는 재인코딩하지 않았다. 명령과 로그는 out/BossChargeAfterimage20260917/compile.ps1 및 각 compile.log에 있다.

초기 Shader_VtxAnimMeshBinary.hlsl의 SOURCE_CHARACTER_PROGRAM_GROUP=84를 FXC fx_5_0/O1로 컴파일했다. 기존 source 함수의 X4000/X4008 경고가 있으며 컴파일은 성공했다. 기본 group0 전체 컴파일은 pass annotation 수정으로 이전 입력이 되어 중단했고 성공으로 세지 않았다. 이 초기 컴파일은 단일 Effect의 shader 문법만 확인했으며 아래 G03의 기본·파생 런타임 admission 오류를 검출하지 못했다.

실제 CSkeletalAfterimage 선언/구현을 사용한 focused native probe는317 checks, failures=0이다. queue cap, deep pose copy, pause, 모든 mesh 제출 요청, live binding 복원, GPU draw 실패 뒤 복원/격리, 다음 주기 재시도, tail 만료, capture 실패, 모델 교체, teleport, suspend와 비유한 World를 검사했다. CModel palette 공급자·Shader·GameInstance는 경계 stub이며 실제 설치 WModel/GPU 제출 증거가 아니다. probe source, SHA와 범위는 queue_probe_scope.json, 결과는 queue_probe.run.log에 있다.

해당 변경의 git diff --check를 통과했다. 소스 인코딩을 UTF-8로 유지하고 기존 CRLF를 보존했다. 편집 전 각 파일은 같은 out 디렉터리의 .baseline에 남겼다.

## G02. 남은 복원과 사용자 확인

원본 TrailGhostEffect의 occurrence와 시각은 확인했지만 serialized 수명·색·shader ABI 전체는 복구하지 않았다. 흰색(1.8,1.8,1.8), 최대 alpha0.38,50ms 간격·250ms 제곱 fade와 rim 강조는 사용자 요청을 구현하는 PROJECT_AUTHORED 표시값이다. source-exact Full Restore 또는 원작 native shader 완료라고 기록하지 않는다. 발탄의 별도 static 도끼·armor까지 복사하는 기능은 이번 skinned body echo에 포함하지 않았다.

제품 통합 빌드와 최종 shader 배포는 root 작업이 소유한다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 사용자 확인 경로는 Kouku Boss Tool의 세이튼_돌진_카운터 Server Play와 Valtan Boss Tool의3회 땅 치기 후 돌진 Play다. 뒤쪽 몸체/지팡이 흰 윤곽의 밀도·형태와 종료 fade는 사용자가 직접 판정해야 한다.

## G03. 캐릭터 입장 실패 원인과 admission 수정

사용자의 KoukuSaydon 입장 실패 뒤 설치된 Debug Engine.dll과 CSO를 읽기 전용으로 조사했다. 기존 headless SourceCharacterShaderVariantProbe는 실제 CShader::Create에서 cohort ABI verification 실패를 재현했다. 별도 FX11 WARP reflection은 기본 Effect와 SourceGroup001/009/017/025/080/084 모두15 passes, pass14의 동일 IA signature336 bytes를 확인했으며, 유일한 policy 불일치는 기본과 모든 파생의 ChargeAfterimage가 모두 BASE=1이라는6건이었다. CShader::Stage_ProgramVariants는 기본 BASE pass의 파생 policy를 UNAVAILABLE=2로 요구한다. 초기 잔상 구현의 annotation이 이 계약을 위반한 것이며 LAN·Server 승인 실패가 원인은 아니다.

수정은 pass14의 ownership annotation과 기본·파생 PS 선택으로 제한했다. validator를 완화하거나 색·fade·mesh 계산을 변경하지 않았다. 기존 SourceCharacterShaderVariantProbe의 미커밋 검사를 보존하면서 직접 파생 Begin(14) 거절과 stale program1/9/18/88에서 shared clone의 기본 afterimage PS·color/fade·bone constant 유지 검사를 추가했다. 실패 당시 근거는 out/EntryCharacterShader20260917/probe.log와 reflect.before.log이며, source와 probe의 편집 직전 byte snapshot도 같은 폴더에 있다.

중간 설치 상태에서 실제 CShader headless WARP 검사를 통과했다. 기본 CSO는02:52 생성본이고, 수정된6개 SourceGroup은03:19 생성본이다. 이 파일들을 별도 out 복사본으로 고정해 실행한 결과 programsChecked126, clonesChecked6, failureCases9, lightPassesChecked504, nativePassesChecked144, afterimagePassesChecked4, unavailablePassesChecked10, windowsCreated0, draws0이었다. 즉 기본 afterimage의 PS·색·bone 동작을 유지하면서 파생 pass14 admission 정책을 교정하면 로드가 회복됨을 실제 Engine 경로로 확인했다. 로그는 out/EntryCharacterShader20260917/after-group-fix/probe.log다. 새 기본 CSO까지 포함한 최종 Product 검증으로 기록하지 않는다.

이후 사용자가 빌드 오류와 실제 진입을 직접 검증하겠다고 명시했다. root는 자신이 시작한 중복 FXC만 부모 chain 확인 뒤 중단했고 사용자 VS의 FXC와 Tracker는 유지했다. 최종 Product·새 기본 CSO probe 대기는 취소했으며 추가 shader 입력 변경은 하지 않는다. 사용자의 VS 빌드 완료 뒤 Lobby → KoukuSaydon 진입과 세이튼/발탄 잔상 화면 확인을 사용자가 수행한다. 현재 기록은 원인 확정, 최소 소스 수정, 중간 설치 조합의 실제 CShader WARP 통과까지이며 최종 빌드·진입·시각 결과는 사용자 확인 전이다.

## G04. Effect V1의 편집 가능한 잔상 모델 큐

앞선 구현은 Server가 재생한 actor의 pose를 받는 runtime helper였으며 V1 문서·카탈로그 항목이 아니었다. 이번 변경은 기존 modelCue에 optional afterimage를 추가하고 CEffectDocumentRenderer가 기존 CModel과 CSkeletalAfterimage를 재사용하도록 연결했다. source preview가 현재 몸체를 표시하고 잔상 cue는 이전 World와 골격 palette만 표시한다. 저장 값은 발생 시작·끝, 간격, 수명, 최대 개수와 PROJECT_AUTHORED appearance basis이며 색·opacity는 기존 modelCue 값을 사용한다. Detail의 Model / Summon에서 Skeletal Afterimage Only와 해당 값들을 편집한다. material program과 afterimage를 동시에 지정하면 거부한다.

원본 root basis에 맞춰 rootMotionVerticalAxis/Scale을 저장하며, 발탄의 별도 animationSetAssetId는 기존 CModel::Attach_AnimationSet으로 연결한다. donor resource도 수집·검증하고 cache key에 포함한다. 실제 body WModel에는 발탄 돌진 clip이 없고 animation donor에 있으므로 body만으로 성공 처리하지 않았다. Same-skeleton/clip/duration 검사를 기존 stage에서 유지한다. 기존 문서는 optional 값이 없으면 이전 동작을 유지한다.

명시 seek, Reset, 숨김, 재stage는 잔상 history를 제거한다. pause는 유지하고 clock 역행·큰 불연속은 초기화한다. 제자리 preview는 실제 골격이 바뀔 때 기록하되 동일 pose를 반복 적층하지 않는다. frame sampling이므로 탐색한 과거 world를 임의로 복원하지 않으며 다시 재생하는 pose부터 history가 생긴다. 수명 꼬리는 Effect_ModelCueEndSeconds를 통해 Playback·Tool duration에 포함한다.

최신 header로 helper, renderer Rendering/ResourceStaging, Effect_Object, Effect_Tool/Detail/CatalogPreview 7 TU 격리 Debug 컴파일을 통과했다. 이후 root의 Detail stable group ID 두 줄까지 포함하여 Detail을 다시 컴파일했다. 공통 codec/Playback/registry 34 TU는 effect_data가 최신 animationSetAssetId layout으로 새로 컴파일했다. 후보 2개 Load·Validate_Drawable·Serialize/Parse·Stage tail과 후보별 9종 잘못된 설정 거절·기존 문서 보존 검사는 24 checks, failures=0이다. 계산된 cue duration은 세이튼 3.083333초, 발탄 4.983333초다. 세이튼 source preview는 별도로 회복 clip까지 4.366초의 기존 P80 순서를 보관한다. 로그는 out/EffectV1Review20260917/cpu/afterimage-codec-probe.log다.

현재 production helper 선언/구현을 사용한 out queue probe는 333 checks, failures=0이다. 기존 317검사에 V1 제자리 pose 변화, 동일 pose 억제, 설정 변경 reset, 잘못된 설정의 기존 history 보존, 사용자 설정 max/lifetime을 추가했다. 모델·렌더 경계는 stub이므로 실제 표시 증거로 사용하지 않는다.

별도 headless WARP probe는 설치 CModel로 세이튼 몸체 5 mesh의 22_01/22_04, 발탄 몸체 3 mesh와 실제 animation donor 연결 뒤 4_01을 원본 notify 시각에 평가했다. 총 1,941 skin palette matrices가 유한했다. 설치 Debug CShader와 out으로 복사한 설치 CSO로 afterimage pass14 및 stale source selector89의 기본 pass admission/binding도 통과했다. draw/window 생성은 0이며 전체 Effect Renderer 실행·최종 화면 판정이 아니다. out/EffectV1Afterimage20260917/model_probe.run.log와 queue_probe.run.log에 기록했다. 제품 빌드·CSO 배포·Client/UI 실행은 하지 않았다.

out/EffectV1Afterimage20260917/candidates에 세이튼/발탄 문서, EffectCatalog.entries.json, EffectResourceTree.entries.json, ValtanFullRestoreAnimations.entry.json과 afterimage.provenance.json을 준비했다. 실제 Data와 실행 중 draft는 변경하지 않았다. 합성할 V1 경로는 `KoukuSaydon / 1관문 / 패턴 / 세이튼 / 돌진 카운터 / 흰 잔상` 및 `Valtan / 3연 구르기 후 돌진 / 흰 잔상`이다. 발탄 source animation sidecar는 기존 허용값 SOURCE_ANIMATION_WINDOW/4733ms를 유지한다. 사용자 저장 뒤 freshness를 재확인한 병합, 최신 Client 빌드, 전체 CEffectDocumentRenderer 실행과 최종 화면 검증은 남아 있다. V1 후보는 body만 포함하며 지팡이·도끼·별도 armor를 포함한다고 주장하지 않는다. 원본 notify 시작 외의 종료·fade·색은 PROJECT_AUTHORED이며 native Full Restore 완료로 기록하지 않는다.
