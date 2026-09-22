# 탈것 lifetime·원본 제어·후처리 연결 결과

## 구현된 소비 경로

`VehicleCatalog` formatVersion 4에 optional `ambientEffectCues`, `mountEffectCues`, `mountSoundEvent`, `dismountSoundEvent`를 연결했다. 기존 형식과 기존 차량은 새 필드 없이 유지된다. Ambient는 `MOUNT_END`, spawn은 `NATURAL` 정책을 파싱한다. Character의 실제 탈것 본·socket과 기존 EffectPresentationService 준비 큐/handle owner를 사용하며 mount commit 실패는 이전 탈것과 효과를 유지한다. 같은 snapshot은 mount cue와 sound를 반복하지 않는다. 해제·다른 mount·owner 소멸에서 기존 pending/active handle을 정리한다. 원본 무한 emitter만 owner-sustained loop로 유지한다.

스킬의 optional `shakeCues`, `directionalLightCues`, `materialVectorCues`는 Server가 승인한 vehicle skill의 clip window와 action age를 사용한다. 잘못된 배열·clip·정책·시간·parameter·curve는 catalog staging에서 거부하며 기존 catalog를 유지한다. skill generator는 다섯 cue 배열을 보존한다. Workbench의 최신 subtree writer도 새 필드를 보존한다.

Q의 카메라 흔들기는 기존 CameraShakeService payload를 사용한다. 원본 duration 2초와 notify-end 1.8초를 구분해 optional `stop=1.8`에서 샘플만 종료하고 기존 2초 envelope를 재조정하지 않는다. 기존 payload에는 변화가 없다. 현재 서비스는 owner별 cancellation handle을 제공하지 않으므로 동작이 중간 취소되면 이미 시작된 shake는 명시 stop 또는 자연 종료까지 남을 수 있다.

조명은 local Character만 조회하고 MainApp이 기존 RenderingProfileService에 일시 배율을 전달한다. 매 프레임 이전 override를 원래 scene light로 복원한 후 camera region과 새 배율을 적용한다. diffuse/specular를 반복해서 누적 곱하지 않으며 ambient와 저작 파일을 변경하지 않는다. 원본 `Brightness`를 현재 scene의 unit baseline에 대한 배율로, ChangeTime→DirectionGuaranteeTime→ReleaseTime으로 소비하는 **project adapter**다. 원본 실행 코드를 확보하지 않았으므로 동일한 retail scheduler를 검증했다고 표현하지 않는다.

PawnMaterialParam은 실제 EFGame enum DefaultMesh=3, Noncontinuous=1과 8곡선 및 5 LookInfo modifier를 읽었다. Sea 항목의 `TransColor`를 선택해 start curve→life hold→end curve로 합성한다. 실제 CModel clone의 원본 material family 전체를 재구성한 뒤 named parameter를 변경하며 동작 종료·취소 시 원본으로 복원한다. 이 시간 합성도 명시적인 **project adapter**다. 원본 native shader·texture·상수의 나머지 값은 유지한다.

## material owner 분리

실물 clone 검증 준비 중 CModel의 복사 생성자가 CMaterial shared_ptr를 공유하고 source constant 변경은 그 객체를 직접 바꾸는 결함을 확인했다. `CMaterial::Clone_ForOverrides`와 source constant/texture/clear의 copy-on-write를 추가했다. GPU texture와 geometry는 공유하고 변경하는 material 상수·override map만 분리한다. 새 Engine DLL로 실제 AncientSea 모델을 로드하고 두 clone을 만든 검증에서 5개 재질의 TransColor를 변경한 owner만 바뀌었다. prototype/peer의 baseConstants bytes는 같았고 Clear 후 owner도 원본 bytes로 돌아왔다.

## 화면 후처리

`decode_post_process_skill_envelope`는 기존 typed parameter table 뒤의 EFPPMESkillValue를 원본 reflection과 대조해 읽는다. Sea 1개와 Guardian 12개 발생 지점의 FadeIn/Play/FadeOut/MaxOpacity를 보존했다. SceneColor를 샘플하는 원본 PS 세 개는 instruction SHA와 정확한 CB0 prefix/sampler closure를 모두 확인한 경우에만 허용한다. 기존 screenPost carrier가 native 4529, 4304, 4305를 재생한다. Guardian의 보조 MRT write는 기존 RT0 carrier 계약에 따라 생략하되 원본 DXBC/archive는 남긴다.

재질 `Opacity`와 action engine opacity는 별개의 입력이다. `source_effect_opacity`를 기존 material uniform/track에 연결해 CB0[0].x를 구동하며 원본 material Opacity를 덮어쓰지 않는다. 명시 null texture는 버리지 않고 해당 native texture-parameter expression의 정확한 referencedTextureIndex가 가리키는 fallback만 연결한다. helper는 `Tools/EffectPipeline/project_action_post_process.py`다.

원본 DDS 5개가 Resources에 추가됐다. `post-resources.receipt.json`은 source-relative asset ID·bytes·SHA256을 기록한다. Sea E의 element와 Guardian 12개는 stable stage key에 따라 기존 문서에 병합하며 문서 전체를 오래된 후보로 덮어쓰지 않는다.

## 제품 탈것 목록

원본 EFTable Vehicle 9523, IconNewVehicle/index132, IconInfo NewVehicle_1 rect(768,896,128,128)에서 아이콘을 추출했다. 이름은 `고대의 바다`, 설명은 `강력한 고대의 신비가 깃든 용.`이다. 기존 여섯 행의 slot ID와 style을 유지하고 일곱 번째 행을 추가했다. runtime은 hardcoded 6 대신 layout의 완전한 연속 row 수를 검증해 사용한다. catalog는 stage 후 commit하여 실패가 기존 row를 지우지 않는다.

`UI/Vehicle/Icons/vehicle_9523.png`는 19,046byte, SHA256 `da2c34bce0a221ea6fd050807a7024b5564d3d24f80e0e094ea5a6511dc9c600`이다. 7행·54slot, 기존 첫 6행 배치 보존, 창 하단 580<720을 데이터로 검증했다. Client/UI는 실행하지 않았다.

## 실행한 검증

- ActorCatalog, Character, CameraShakeService, RenderingProfileService, MainApp, Part_Vehicle 6TU `/Zs` PASS. 앞선 lifecycle 3TU와 VehicleWindowView도 PASS.
- 실제 ActorCatalog production parser에서 lifetime 정상/잘못된 정책·경로·시간·중복, 기존 차량 호환, 실제 Sea 행, sound commit, 실패 시 기존 catalog 보존을 검증했다. CameraShake의 1.799초 샘플/1.8초 종료와 음수 조명 시간·정렬되지 않은 material key 거부도 PASS.
- 13개 원본 PostProcess tail의 SHA·byte end 일치와 nonfinite 거부 PASS. source native shader 3개 생성 0 deferred. Shader 최종 컴파일은 통합 Product 결과를 따른다.
- 최신 VehicleCatalog 전체를 generator formatter에 넣고 재parse한 값이 완전히 동일함을 확인했다. 새 Python 파일과 변경 pipeline 파일 py_compile PASS.
- 실제 CModel/CMaterial/production parser를 링크한 최종 probe 29개 PASS. Engine.dll SHA256 `9a7ad91f30c801b13f7a3bec000ca0a3fb301391886f4e83c7b59e9079807062`, WARP device를 사용했다. Client/UI 실행은 없다.
- 처음 모델 로드가 실패했던 원인은 후속 검사에서 통합 worktree의 공통 `Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga` 누락으로 확인됐다. 이전 DLL 때문이라고 확정할 근거는 없었다. Desktop에 존재하는 동일 원본 2,097,196B를 설치한 뒤 실제 모델 생성과 clone 격리·원복이 PASS했다.
- WModel decoder의 diffuse/normal 10개와 catalog native register 33개, catalog의 원본 texture expression을 모두 조사했다. 모델 포함 unique 37개 자산 72,860,727B, 누락 0이다. `sea-model-dependency.receipt.json`의 Resources-relative ID/bytes/SHA256 목록을 GBResources 합본에 전달했다.
- Guardian 담당 actual C++ codec 검증에서 Sea 7문서의 121elements+1live-afterimage와 native4529 load/drawable/canonical roundtrip PASS, Guardian 12PostProcess native4304/4305 PASS를 확인했다.

증거 위치는 Git 제외 중간 산출물 `out/SeaLifetime20260922`의 `catalog-contract-cow.log`, `*.controls.log`, `source-stage-controls-contract.json`, `vehicle-controls.source-receipt.json`, `post-projection.receipt.json`, `post-resources.receipt.json`, `vehicle-ui.receipt.json`이다. 최종 제품 빌드·패키지 전달은 [통합 결과](2026-09-22_ANCIENT_SEA_GUARDIAN_INTEGRATION_RESULT.md)를 따른다.
