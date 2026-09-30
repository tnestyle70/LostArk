# GuardianKnight 리벤지 스피어 반영·검증 결과

## G00. 확정 근거와 현재 상태

사용자는 최초 제시한 775~2169 수치를 대략적인 값으로 설명하고 원본 클립에 맞추기를 요청했다. 설치 GuardianKnight.wmodel의 ddk_sk_brutalmarch_02는 index49, 40tick/30TPS=1.333333초다. skill49210 HOLD의 stage1, rate1, 1333ms와 일치한다. stage0 20/30초를 합한 전체 기본 재생 구간은 약666.667~2000ms다. 애니메이션·Server timing·skillbinding·animevents는 변경하지 않았다.

후보 검증 뒤 사용자의 전체 병합 승인을 받아 Data 원본에 설치했다. 설치 전 source SHA256은 `1aaa2e28d04244ca8f8843de75075aac158c9f0a527291f95743cbd8824432a4`로 snapshot과 교체 직전까지 일치했다. 설치 receipt와 실제 Data 재검증은 G03에 기록했다. 다른 Guardian 수동 편집 데이터와 기존 Valtan/자동입장 소스는 보존했으며 Client 실행·조작은 하지 않았다.

## G01. 반영한 최소 변경

후보: `out/GuardianRevengeSpear20260930/candidate/effect.guardianknight.skill.49210.clip.1.full.restore.effect.json`.

stable ID 기반 변경 목록: `out/GuardianRevengeSpear20260930/candidate-field-patches.json`.

32개 element 중 b_root 방향 그룹 25개의 transform 위치와 Y 회전을 -90도로 변경했다. 원본 00_01/00_01-2 검격, 00_07 차지, 00_05 주변 입자가 해당한다. 손 장갑·척추 고리·screenPost·light의 다른 7개 element는 그대로다. 전역 particleSystem 회전은 변경하지 않았다.

두 번째 검격 00_01-2의 10개 emitter만 sourceRecipe.emitterDurationSeconds와 detail.timing.lifeTimeSeconds를 늘렸다. start0.3000000119초, source delay0인 9개는 duration1.0333333214초, delay0.05인 1개는0.9833333207초로 원본 클립 끝에 방출이 끝난다. 유한 loop1, 원본 spawn rate/burst, 개별 particle 수명·속도·material 곡선은 보존했다. 무한 loop, 반복 공격, 새 애니메이션 재생 경로는 추가하지 않았다. 방출 이후 기존 particle 잔상은 자연 소멸한다.

총 stable field patch는68개다. root가 준비한 `out/GuardianGrandFinale20260930/candidate-revenge-clip1.effect.json`의 원문 span patch 후보와 JSON 의미가 동일함을 비교했다.

## G02. 실제 검증

`build-probe.log`, `probe.log`: 제품 Release Effect_DocumentCodec/Effect_Playback OBJ를 out 전용 probe에 링크하여 actual Codec+Playback PASS(exit0). 원본과 후보의 초기261개 packet에서 회전 대상 emitter basis를 XMMatrixRotationY(-pi/2)로 비교했고 최대 행렬 오차2.38419e-7이다. 기존 소비자 Effect_Playback.cpp의 Evaluate_ElementWorld는 degree→radian 변환 후 XMMatrixRotationRollPitchYaw에 Y값을 전달한다. 이 좌표계의 -90°는 (x,y,z)→(-z,y,x), +X→+Z 변환이며 요청한 반시계 회전으로 적용했다.

원본4개 emitter는 balwaysinworldspace=true 가속도를 가진다. 월드 가속도를 임의 회전하지 않고 원본 모듈을 보존했다. 따라서 이4개 궤적 전체를 단순 quarter-turn한 결과와 동일하다고 주장하지 않는다. 나머지 packet은 실제 위치·속도의 quarter-turn을 검증했고, 모든 packet의 finite matrix와 초기 alpha·count 보존을 확인했다.

`build-geometry.log`, `geometry-original.log`, `geometry-candidate.log`: actual Codec/Playback/Make_ParticleSpriteWorld와 설치 CModel의285개 bones, 실제 clip49 원본40tick을 사용한 무창 검증 PASS(exit0). 제품 Build_SourceBoneAnchorWorld의 원본 import scale 정규화 함수를 verbatim 추출한 기존 helper를 링크했다. source root·실제 named bone history·원본socket을 사용했다. camera는 고정 synthetic camera이며 실제 GPU draw/사용자 화면 판정은 아니다.

실제 모델 본을 사용한 검격 결과는 다음과 같다.

| clip02 로컬 시점 | 원본 입자/alpha>0.01 | 후보 입자/alpha>0.01 |
|---|---:|---:|
|1.0초|6/0|34/24|
|1.2초|0/0|35/31|
|1.333333초, 원본 클립 끝|0/0|37/35|
|2.0초, 자연 소멸 후|0/0|0/0|

변경하지 않은 손 장갑·척추 고리의58개 geometry sample은 byte-equivalent JSON 값으로 동일했다. 원본1086개/후보1266개 geometry sample 행은 `geometry-*.jsonl`, 요약은 `geometry-summary.json`에 있다.

`validation.log`: 기존 scoped effect validator5개, v13 extension 경계, JSON parse,32개 stable element와 원본SHA보존 PASS. 기존 runtime resource closure는46개,5,951,240bytes를 검증했다. 이46개는 shared local Resources이며 이 검증을 Git 전달 완료로 해석하지 않는다. `git diff --check`는 이 PLAN/RESULT 대상으로 PASS.

## G03. 최종 반영 경계

사용자가 최신 저장본 기준 네 문서 전체 병합을 명시적으로 승인한 뒤 리벤지 스피어 후보도 Data 원본에 설치했다. 최신 원본 hash를 다시 확인하고 stable ID/필드별 변경을 병합했으며 백업과 원자 교체를 사용했다. 통합 설치 receipt는 `out/GuardianGrandFinale20260930/installed-20260930T090036807332Z/receipt.json`이다. 설치된 리벤지 스피어 SHA256은 `c825b0eb58629af44a113e7ff0321252090e633d22ff98bb2796c461064ed1d4`이며 검증한 후보 bytes와 일치한다.

설치 후 네 실제 Data 경로의 JSON parse, asset ID와 후보 SHA, 대상 `git diff --check`가 통과했다. 실제 제품 Codec은 설치4문서 failures0/exit0 및 resource boundary12개 PASS였다. 현재 수정된 Guardian 전체8문서도 추가 Load failures0/exit0, 읽기 전후 SHA 일치를 확인했다. 설치 디렉터리의 `installed-validation.json`, `installed-codec.log`, `all-modified-guardian-validation.json`에 증거를 기록했다. 이 단계는 읽기 검증이며 사용자 수동 편집 파일을 변경하지 않았다.

전체 Server/Client owner publish와 Debug/Release Product 빌드가 완료됐다. 빌드 receipt는 `out/BuildPipeline/runs/20260930T091228557Z-debug-product.json`, `20260930T091359049Z-release-product.json`이며 둘 다 PASS다. 게시 후에도 설치4문서 및 수동 편집8문서 SHA가 그대로임을 `out/FinalRaidGuardian20260930/post-publish-effects.json`으로 확인했다.

실행 중 도구의 Reload나 memory draft 갱신은 자동 수행하지 않았다. 실제 화면의 최종 방향·모양은 사용자가 새 실행 파일에서 확인한다. 수치·로더 검증과 별도로 GPU 화면 판정은 남아 있다.

## G04. 실제 본 기준 전방 보정과 독립 그룹 (2026-09-30 후속)

사용자 화면에서 검격이 바닥으로 향하고 Group by anchor의 회전 조절이 기대대로 보이지 않는 증상을 재조사했다. 최초의 숫자 변경/화면 불변 설명은 사용자가 확실하지 않다고 정정했다. 확인된 결함은 두 가지다. b_root29개 그룹에는 꺼진 screenPost2개가 포함되어 Play Group이 첫 항목에서 거절됐다. 또한 이전 geometry 검증의 모델 preTransform에는 제품의 Ry(-90)가 빠져 있었으므로 그 결과를 제품 전방 일치의 근거로 사용할 수 없었다.

실제 제품 Scale(.0001)*Ry(-90)를 사용한 설치 GuardianKnight.wmodel의 clip49(ddk_sk_brutalmarch_02)에서 정규화 b_root의 세 basis는 대략 [-1,0,0], [0,0,-1], [0,-1,0]이다. 기존 Element Y=-90도와 합치면 검격 source +X는 월드 -Y를 향한다. 이 basis는 clip 전체에서 일정하므로 기존 BONE 경로의 socket [90,180,0]도로 상쇄했다. OWNER_YAW는 sourceRecipe 문서에서 금지되어 있으므로 codec·validator 경계를 변경하지 않았다.

검격25개를 `manual.guardianknight.revenge-spear`, 고유 slot `manual.anchor.guardianknight.revenge-spear`로 분리했다. 기존 b_root, 각 Element 위치·Y=-90도 회전·scale·timing·recipe·재질은 보존했다. source locked-axis sprite2개에는 기존 followEmitterAxisRotation=true를 연결했다. 나머지 손·척추·screenPost·light7개는 JSON 원문까지 유지했다. Anchor Position과 Anchor Rotation으로 공통 위치·회전을 편집하며, 새 그룹에 비활성 screenPost는 포함되지 않는다.

`Effect_Tool_CatalogPreview.cpp::Try_PreviewElementsTimeline`은 그룹 재생 선택에서 명시적으로 꺼진 Light/ScreenPost만 제외한다. 단일 Solo, missing ID, hidden carrier, enabled malformed presentation은 기존 오류 처리를 유지하며 원본 문서나 그룹 편집 대상을 삭제하지 않는다. 신규 파일/project/filter 등록은 없다. live draft→immutable occurrence→source anchor 전달과 history 초기화 경로는 이미 연결되어 있었다.

검증 증거는 `out/GuardianRevengeAnchor20260930`에 있다. 실제 모델 81시점×owner yaw4방향=324검사에서 basis 변화 최대2.84217e-14, 전방 오차 최대1.26971e-7이다. actual Playback의 검격1060표본은 기존 worldY=-1에서 수정 후 ownerForward 내적1·worldY 약0으로 바뀌며 비대상145표본이 동일하다. 기존 source world-space 가속도는 보존했다. 실제 Codec Load/Drawable/Stage, 최종 sprite geometry를 수치로 검사했고 camera는 synthetic이며 GPU 화면 판정은 하지 않았다.

그룹 선택은 실제 문서29개→27개(활성 light2개 유지), disabled Solo·all-inactive·missing ID·enabled malformed·hidden particle·inactive light와 원본 문서 보존8개를 검사했다. 수정 TU 전체의 격리 컴파일은 성공했고 기존 include C4819 경고만 남았다. 최초 out 실행의 resource-root 누락은 경로를 명시하여 재검사했다. 소스 선택 부분을 추출한 집중 검증이며 실제 ImGui 조작은 하지 않았다.

사용자가 모든 편집을 저장했다고 명시한 뒤 최신 저장본 기준으로102필드/25요소를 병합했다. 최신 baseline SHA256은7881763abab4048f36e330834fab78cebf0baa4b944f08fe1d746a3b4f826da6이며 최종 설치 SHA256은38b316a4eb58951798c1215bb9d93aa74877a3bcd6cd389cf37d58d2c5264d52다. 실제 Codec 재검사와 scoped validator5개·리소스46개 존재를 확인했고 ReplaceFileW의 실제 교체본 백업과 교체 전후 hash 확인을 사용했다. receipt는 `installed-20260930T123327508759Z/receipt.json`이다. Effect authored는 제품 직접 입력이므로 별도 publish/복사하지 않았다.

추가 그룹 편집 검증은 같은 설치 모델과 제품 preTransform으로 수행했다. Anchor Position [.5,.25,-.75]의 실제 입자1992표본에서 이동 오차 최대6.60e-7, Anchor Rotation [90,180,0]→[120,180,0]의 emitter 축3864비교에서30도 회전 오차 최대1.87e-7이다. 시각 출력23개에 반영되며 나머지2개는 원래 simulationOnly=true로 직접 draw packet이 없다.25개 모두 공통 socket 편집을 받고 비대상7개 JSON과 출력116표본은 보존됐다. Anchor Position은 본 로컬 축으로, owner yaw0에서 [.5,.25,-.75] 입력은 월드[-.5,+.75,-.25] 이동이다. 기존 world-space 가속도는 원래 방향을 유지한다. 증거는 `anchor-edit-verification.json`과 `edit-{base,position,rotation}-geometry.log/jsonl`이다.

현재 완료: 데이터 설치, C++ 소스 수정, 집중 수치 검사와 TU 컴파일, Product Debug 빌드·배포. 사용자는 처음 빌드를 미룬 뒤 배경 검토까지 마치고 Client/Server를 직접 종료했으며, 실제 process 없음과 설치 hash 유지 확인 뒤 빌드했다. `out/BuildPipeline/runs/20260930T125746734Z-debug-product.json`은 PASS다. 새 Debug Client에 그룹 재생 수정이 포함됐고 데이터 hash도 유지됐다. 새 Release EXE 빌드·실행 중 도구 Reload·사용자 화면 확인은 수행하지 않았다.
