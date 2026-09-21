# 고대의 바다·가디언 나이트 통합 결과

코드·Data 및 새 Resources를 Desktop 작업 폴더에 반영했다. 기존 쿠크세이튼 작업은 최신 저장본의 변경을 보존해 3-way 병합했고, 그 상태의 Product 빌드도 통과했다. PR #437 충돌 해결·병합·pull은 기능 작업보다 먼저 완료했다.

- Character 7종·광기/Mario/카드미로 광대·탈것의 공통 Composition Resources, stage/clip/effect/sound/collider/logic 편집과 저장 경로를 연결했다.
- Bone & Animation Edit는 실제 본과 native clip을 이용해 새 clip을 저작·저장·재생한다. Sea flight/glide/ascent는 저작 예시이며, Valtan 도끼 본 회전과 Pattern Publish도 연결했다.
- Guardian 최종 효과 44문서와 20스킬, 실제 165clip의 연결을 확인했다. 원본 재질·조명·가시성 제어 40개와 각성 카메라를 기존 소비자에 추가했다.
- Valtan F1 3위치·Despawn·Server Pattern 재생, 유령 preview 모델 선택, 일리아칸 연출의 async 준비 처리를 수정했다.

화면 판정은 별도다. 실제 전투 중 다섯 번째 유령 비표시의 원인은 아직 단일 경로로 확정하지 못했으며, 이 문서는 GPU 수치 검증을 사용자 화면 성공으로 대신 기록하지 않는다.

## Git 기준선

PR [#437](https://github.com/tnestyle70/LostArk/pull/437)은 충돌 해결과 NetworkProtocolHarness/Product 검증 후 병합됐다. main 정본은 `0ebd23cd1f04a6a65a09125e00ef2ac319ce0d97`이며 Desktop 작업 폴더도 이 커밋까지 pull했다. 사용자 쿠크세이튼 미커밋 변경은 보존했다. 복구 stash `3fc32f8da19720333340368966481b4c91f9ca7f`와 `out/Pr437IntegrationBackup`은 유지한다. 별도 작업의 미승인 out 후보는 적용하지 않았다.

## 바다 용의 원본과 설치

원본 Vehicle 9523은 `EFDLVehi_MN_PMSDZ_00.MN_PMSDZ_00-2`다. 기존 신화 9524와 실제 source mesh/AnimSet이 같아 기존 cooked geometry를 재사용하고 Sea의 원본 MIC 5개를 별도로 연결했다. 런타임 경로는 `Character/Vehicle/AncientSea/AncientSea.wmodel`이다. WModel SHA256은 `53bb9ee9ba23971909f8465fa5f3eb649bd17e1db03a9e9acbc1ff2d8c2b59b8`이다.

실제 모델에는 원본 clip 9개와 cooked bone 91개가 있다. source action 14개·stage 22개·notify 91개를 조사했다. 같은 skill ID를 공유하는 탈것 variant를 vehicleId로 구분해 효과·사운드·스킬 게시가 다른 탈것의 항목을 덮지 않게 했다. 실제 Server vehicle catalog 6검사와 publisher variant 회귀 3검사를 통과했다.

원본 particle notify 28개, ParticleSystem 22개, emitter 88개를 효과 문서 7개/기본 element 120개로 연결했다. 원본 재질 49개는 native program 4480~4528로 설치했으며 그중 distortion 18개다. 이후 E 후처리 4529와 dash 잔상, Q shake 종료, 로컬 조명 및 transcolor 변화 소비자를 추가했다. 최종 문서 수와 element/model cue 수는 최종 검증 receipt가 정본이다.

source sound event 15개 중 playable 11개는 기존 정확한 event 9개와 신규 GetOn/GetOff 2개다. 새 WAV는 6개다. 나머지 4개는 원본 Stop-only event여서 재생할 media가 없으며 가짜 WAV를 만들지 않았다. source가 참조하지만 실제 AnimSet에 없는 clip 5개도 새 원본 clip으로 위장하지 않았다.

새 SourceCharacter 100은 Sea slot03의 원본 Base/Light 계산을 연결한다. 다른 4개 slot은 실제 같은 shader와 parameter 계약을 재사용한다. Mount/ambient cue와 탑승·해제 사운드는 기존 Character/Vehicle 소비자에 연결했다. Vehicle UI에는 원본 atlas132에서 추출한 Sea 아이콘과 일곱 번째 행을 추가했다.

본 편집·공통 Workbench·Valtan trim/publish 결과는 [Character/Bone 결과](2026-09-22_CHARACTER_BONE_WORKBENCH_RESULT.md)를 따른다. flight/glide/ascent 3개는 실제 Sea 본을 이용해 만든 **저작 예시**이며 원본 추출 clip이 아니다. 탑승 모델의 애니메이션 표현을 확장했으며 Server의 비행 이동이나 충돌 권위를 추가하지 않았다.

## 원본 shader 계산 검증

가디언 모션 블러 4006/4060의 source PS `b9fc10ac51695c41b71ae1807fe6a47d`와 VS `5825675b4ffbc840ad691ec56973cf7e`를 확인했다. shader object wire의 LocalToWorld와 WorldToView 3행을 실제 emitter/camera에서 공급하고 실제 clip Z/W를 보존했다. camera 방향으로 정확히 겹친 forward와 0 깊이는 해당 pass만 중단하여 non-finite 합성을 막는다.

실제 원본 DXBC와 제품 native HLSL을 같은 WARP에서 비교했다. 단색 1×1 texture 검사만으로 UV 오류를 검출할 수 없어 64×64 비선형 RGB gradient를 사용했다. 6 자세×3 깊이×3 강도×2 재질의 **108조건 PASS**, non-finite 0, 최대 절대 오차 `1.3113021850585938e-06`이다. 증거는 `out/GuardianEffects20260922/native/motionblur/product-warp-receipt.json`이다.

이 비교에서 DXBC `sincos r0.x, r1.x, r0.x`의 두 출력을 순차 대입하며 cosine이 덮어쓴 sine 값을 읽는 변환기 결함을 발견했다. sourceAngle을 먼저 고정해 atomic source-read를 보존했다. 이번 신규 program의 95개 sincos 명령만 같은 generator 출력으로 교정했고 이전 program 함수는 보존했다. 이 결함을 유령 발탄의 미확인 전투 증상의 원인으로 단정하지 않는다.

ALT+V의 PlayStaticMesh 7개는 하늘 1개·발판 6개이며 원본 메시 3개, native 재질 4530/4531 두 개로 추출했다. source TRS 및 bApplyLocalRotation을 보존한다. 기존 SourceTransformTrack의 FRotator 행렬 변환과 root snapshot basis를 사용하며 본 import 회전을 중복 적용하지 않는다. 중심 발판의 parent rotation 비상속은 저장 가능한 optional mesh 필드로 구현했다. 실제 원본 PSK 715점과 glTF의 모든 점이 `(x,z,-y)×0.01`에서 일치했고 cooked WModel의 추가 좌표 변환은 기존 geometry 계약 검증을 통과했다.

## 검증 범위와 남은 확인

- Effect source validator 회귀 45개 PASS. Sea 기본 7문서의 color space/module override/attachment/native sprite/v15 구조 검사와 참조 Resources 94파일(10,038,320byte) 검사 PASS. 이후 추가한 후처리·잔상은 실제 C++ codec 통합 검사에서 다시 확인한다.
- 모션 블러 관련 3TU와 각 담당 C++ syntax 검사를 통과했고, 최종 Product 빌드는 아래 통합 검증 기록과 같이 양쪽 작업 폴더에서 통과했다.
- 전체 source validator는 변경하지 않은 기존 `effect.kouku.gate1.blade-dance.circle.impact.effect.json`의 v15 empty carrier를 발견했다. 이 기존 문서는 수정하지 않았고 전체 통과로 기록하지 않는다.
- TrailGhost의 실제 owner/장비/무기 pose와 source timing은 연결했으나 원본 rim/fade shader ABI는 확인되지 않았다. 해당 표현은 `PROJECT_AUTHORED`로 분리한다.
- source light/material envelope는 원본 serialized field를 소비하는 명시적인 project adapter다. 원본 게임 C++ 실행을 동일하게 재현했다고 표현하지 않는다.
- Client/UI를 자동 실행하지 않았다. 사용자 화면 확인, 특히 전투 중 유령 발탄의 재현 여부와 새 비행 연출의 최종 자세 판정은 남아 있다. 렌더링 조사와 진입/ghost preview 수정 증거는 [Valtan 결과](2026-09-22_VALTAN_EDITOR_VISIBILITY_RESULT.md)를 따른다.

최종 source 반영·Product 결과·GBResources 패키지 검증은 아래 통합 검증 기록을 따른다. 기능 변경은 별도 codex/guardian-workbench-integration 브랜치와 PR로 전달한다.


## 최종 자산·실제 소비자 검증 추가

Owner Controls 추가 전 가디언나이트 기본 43문서는 실제 C++ Codec에서 load → Validate_Drawable → serialize → parse canonical roundtrip을 모두 통과했다. element 1,608개, Model Cue 23개, source PBR parameter sample 66개다. Static actor는 program 4530/4531와 원본 3 WModel, EffectRoot/root snapshot, SourceTransformTrack 조합으로만 승인한다. 다른 모델, particle kind, track 제거의 3개 변조는 모두 거부했다. 신규 Resources 230개/124,457,360 bytes를 두 작업 폴더에 설치했고 기존 501개 참조는 동일 SHA256을 확인했다.

Sea의 최종 7문서도 같은 실제 C++ 경로를 통과했다(121 element, live afterimage 1개). 실제 CModel decoder와 catalog native register에서 모델 포함 37개 의존성을 검사했고 누락 0이다. 처음에는 공통 statefx_default.tga가 통합 worktree에 없어 모델 생성이 실패했다. 정확한 기존 파일을 설치하고 실제 Sea CModel 2 Clone/5 material을 사용해 owner 변경, peer/prototype 불변, Clear 원복까지 29조건을 통과했다. 모델이 로드되지 않은 이전 검사를 성공으로 대체 기록하지 않았다.

현재 실제 CSkeletalAfterimage와 원본 Static 3 CModel의 geometry/texture binding WARP 검사는 115조건 PASS다. Static geometry 출력은 원본 native pixel shader의 동일성 검사와 별개다. native 4530/4531 원본 DXBC 연산 비교 10조건과 함께 증거 범위를 구분한다. source lifetime 0의 Guardian S 두 occurrence는 원래 normalized age 0을 유지하며 각 notify가 끝나면 정리되는 실제 bone/clip 소비자 28조건을 통과했다. 부모 회전 비상속은 실제 Effect_Playback에서 SourceTransformTrack과 함께 8조건 PASS다.

추가 리소스 전달 폴더는 C:/Users/user/Desktop/GBResources/2026-09-22_ActionWorkbench 이다. 373파일/211,660,421 bytes를 상대 경로로 정리했고, manifest.json의 전 파일 SHA256을 원본 설치 파일과 다시 대조했다. 기존 리소스 전체를 복사한 묶음이 아니며 새 리소스와 추가로 확인한 필수 모델 의존성을 포함한다. 애니메이션/효과/본 JSON과 shader 소스는 Git 변경으로 전달한다.


가디언나이트의 WModel 내부 기본 DDS 28개도 actual CModel 생성에서 확인해 추가했다. JSON의 native override texture가 모두 있어도 기본 CMaterial 입장 단계는 먼저 실행된다. 최종 native PBR section 8개는 실제 WARP CModel 생성과 animated material variant를 8/8 통과했다. 기본 43문서의 All Effects join을 먼저 확인했고, Owner Controls 설치 후 아래 최종 44문서/20스킬/165clip으로 전체 연결을 다시 검사했다.

최종 source notify coverage는 PawnMaterialParam·DirectionalLight·UltimateCamera·IdentityParts·HidePawn까지 대조했다. 원본 활성 제어와 source-disabled/empty no-op를 구분하며, 아래 최종 문서/실제 소비자 검증 결과를 따른다.

후처리 material Bind가 S_FALSE(이번 pass 생략)를 반환해도 renderer가 이전 shader state로 draw하던 오류를 수정했다. Render_ScreenPostPass는 반환값을 전달하며 scene replacement는 복사하지 않고, 일반 합성 target 순서는 S_OK일 때만 진행한다. 실제 Renderer 7개 메서드 본문을 그대로 사용하고 생산 CShader/CVIBuffer_Rect/CTarget_Manager를 연결한 8×8 WARP 검사는 33/33 PASS다. 정상→생략→정상과 생략-only, scene replacement 모두 draw primitive 수/최종 target/RGB/이전 화면 보존을 확인했다. 원본 shader 수치 검사와 renderer 소비자 검사를 분리했다.


## 최종 통합 검증 및 전달

가디언 최종 한 실행은 44문서 / 1,608 element / 23 Model Cue / 40 Owner Control / 66 native parameter sample의 actual Codec load·Validate_Drawable·serialize/parse를 통과했다. 실제 All Effects join은 20스킬 / WModel 165clip / animevents 연결44 / 미해결0이다. 재질23·조명5·identity9·pawn3 제어를 24stage에 넣었고, 원본 disabled14개와 empty no-op6개는 실행 제어로 만들지 않았다. 원본 sound114 occurrence와 shake48도 대조해 source sound 시각4행만 정정했다.

각성 원본 camera458key를 0..3,800ms로 연결했다. source3,801시점 비교의 최대 오차는 위치1.152mm/FOV0.01112도이며, 실제 recovery camera와 Character stage camera 소비자의 trim0/500ms × rate0.5/1/2 및 원래 start0 FX cue 제외 시 camera 유지 검사53개를 통과했다. F6와 기존 camera 우선순위를 보존한다. 독립 status/buff FX owner가 없는 원본 숨김 flag는 미구현 경계로 구분한다.

실제 default-stack Character 검사에서 generated SourceCharacterMaterial::Configure의 거대한 Debug frame이 stack overflow를 일으켰다. 61개 native packing을 별도 lambda call frame으로 분리했으며 모든 body 계산식·상수·parameter 이름은 바이트 비교로 동일하다. Guardian101~108의 generator/installed packing도8/8 일치한다. /STACK 증가로 덮지 않았다. 최종 actual Character/EffectObject/material owner 검사 45개를 통과했다. 자세한 frame·재질·원복 증거는 [Owner Control 결과](2026-09-22_GUARDIAN_OWNER_CONTROL_RESULT.md)를 따른다.

현재 native program의 정확한 TransColor/BuffColor register만 수정하며 catalog 이름 추측이나 sentinel 재packing을 사용하지 않는다. 실제 Configure 대비61family/116지원조합의 전체 struct 비교734검사와 generator6회귀가 통과했다. actual Character 검사는 native99 hair와96 weapon의 실물 clone, peer/prototype 및 기존 튜닝값 보존, 종료 원복을 포함한다. Guardian 기본 body의 나머지5 AUTO 슬롯은 기존 #437의 얼굴/눈 parser 및 UV1/UV2 경계로 유지한다. 이 슬롯에 native 색 제어가 적용된 것으로 확대하지 않는다([기존 재질 경계](../../JS/09-21/2026-09-21_GUARDIANKNIGHT_RESTORE_SYNC_RESULT.md)).

최종 Debug Product는 통합 worktree `20260921T213031143Z-debug-product.json`와 Desktop `20260921T213128876Z-debug-product.json` 모두 PASS이며 SkipBuild가 아니다. Engine·Shared·Server·Client의 compile/link/deploy와 Item/Valtan reward runtime 검사를 포함한다. Client/UI 또는 공유 Server를 시작한 기록은 아니다. 새 F1 Server 동작을 공유 LAN에서 쓰려면 Server PC도 이번 코드를 빌드·재시작해야 한다.

Desktop 반영은 current/incoming/candidate SHA를 재확인한 뒤 파일별 백업·원자 교체로 수행했다. 원본 작업 폴더의 index와 사용자 쿠크세이튼 source/draft를 보존했으며, 별도 미승인 out 후보를 적용하지 않았다. 로컬 기록은 `out/ActionWorkbenchIntegrationApply`의 source-applied/applied receipt와 apply-history, backups에 있다.

추가 Resources는 `C:/Users/user/Desktop/GBResources/2026-09-22_ActionWorkbench/Resources`에 정리했다. manifest.json의 373파일 / 211,660,421 bytes를 설치된 원본과 전부 SHA256 대조했다. 기존에 받은 Guardian WAV519개 등 공통 자산 재사용은 신규 파일 수에 합산하지 않았다. Resources와 exe/dll/pdb/cso/out은 기능 소스 커밋에서 제외한다.

남은 사용자 화면 확인과 원본 ABI 경계는 그대로 유지한다. ghost 실제 전투 프레임·조명·suppression 상태, 바다 용의 최종 비행 자세, HUD/툴 조작은 Client를 자동 실행하지 않아 확인하지 않았다. TrailGhost의 원본 rim/fade shader는 PROJECT_AUTHORED 표현이며, source light/material 시간 합성과 brightness는 명시적인 PROJECT_ADAPTER다. 원본 게임의 비행 물리나 새로운 Server 비행 이동을 구현한 것으로 표현하지 않는다.
