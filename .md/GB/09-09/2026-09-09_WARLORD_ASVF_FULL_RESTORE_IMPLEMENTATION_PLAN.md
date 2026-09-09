# 워로드 A·S·V·F 원본 전체 복원 구현 계획

## G00. 원본 발생과 현재 저작본 분리

현재 브랜치는 `codex/dimensionmaster-tool-round3`, 기준 HEAD는 `591012dbebf7eeab0b660baec42852b9396e77d4`다. 사용자 손튜닝과 다른 작업의 dirty 변경을 보존한다. A17090 사슬, S17040 파란 방패 세 개의 정면 발사, V17170과 F17140 번개가 목표다. S를 D로 바꾸지 않는다.

최신 설치 data3.lpk의 GUNLANCER.loa에서 exact action/stage/clip을 먼저 선택한다. 같은 clip의 여러 stage는 하나의 동시 연출로 합치지 않는다. Particle notify의 enabled 값과 현재 package의 첫 LOD를 대조한 뒤 해당 발생만 복구한다. 보존 Imported JSON은 geometry와 module의 참고 입력이며 원본 활성 근거를 대신하지 않는다.

현재 A14/S11/V9/F4 element와 원본 활성의 수는 다르다. full restore는 별도 `effect.warlord.skill.<id>[.clipN].full.restore` 문서로 저장하고 현재 unified 문서를 수정하지 않는다. 원본을 따라가면서 끝내 실행할 수 없는 요소는 사유와 원본 ID를 RESULT에 남기고 새 full 문서에서 제외한다. 핵심 사슬·방패·번개의 복원이 우선이며 generic shader 교환이나 approximate 허용만으로 복원 완료를 주장하지 않는다.

## G01. 원본 재질·입력 계약 회수

source MIC → parent hierarchy → static parameter set → native shader map → 실제 carrier VF/PS/VS → texture/sampler/uniform을 회수한다. 원본 PS/VS가 같을 때만 차원술사 native 경로를 재사용한다. A는 Masked Chain의 WPO, S는 Shield02의 본체 두 층과 발생 세 회, F는 electric vertexcolor mesh의 sinwave WPO, V는 rectan3cross 전기 geometry와 재질을 먼저 닫는다.

새 `Effect_WarlordNativeMaterial.h`는 source identity와 명시 texture·parameter 계약을 검증한다. 새 `Shader_EffectWarlordNative.hlsli`는 회수한 원본 RT0 식과 필요한 정점 변형을 소유한다. 프로그램 번호 400~459와660~719를 사용하며 기존 native profile과 충돌하지 않는지 확인한다. 새 C++ 번역 단위 Effect_NativeScreenPostMaterial.cpp는 기존 Engine IPresentationScreenPostMaterial 소비자에 native snapshot을 연결한다. header/cpp와 shader의 vcxproj/filter 등록은 root 통합 변경에 포함한다.

## G02. 기존 Renderer와 재생 경로 연결

기존 EffectDocumentRenderer/Playback/Tool, CModel/CMaterial을 사용한다. 새 runtime을 만들지 않는다. 공용 파일과 project/filter 등록은 통합 담당 root가 수정하며, 워로드 담당은 exact include·dispatch·binding patch를 별도로 전달한다. profile의 parameter/texture 실패는 해당 occurrence를 거절하고 기존 문서를 보존한다.

source cm·좌표계·Dynamic/Orbit의 원본 기본값을 유지한다. 외부 emitter state를 필요로 하는 경우 owner 동시 재생이 필요한지를 판별하고, 원본과 동등하지 않은 authored path로 바꾸면 원본 복원과 구분한다.

원본 screen-post도 기존 PREPARED_MATERIAL callback을 통해 실제 scene color/depth, SRV9개와 native parameter32행을 연결한다. immutable frame snapshot이 source color/dynamic, 실제 scene camera projectionW, source local time을 보존한다. Effect_Object의 일반 native refraction도 scene capture를 요청한다.

## G03. 검증과 인계

변경 JSON parse·stable ID·실제 resource 존재, 선택 PS/VS input contract, 필요 FXC 및 focused CPU 검사를 수행한다. 최소 Product 컴파일과 project/filter XML parse·전체 diff check는 통합 root가 한 번 수행한다. Engine.lib를 여는 별도 compile/link는 하지 않는다.

Client/UI는 실행·조작·캡처하지 않는다. 화면의 번개 움직임·사슬 실루엣·방패 세 개·색·타이밍의 최종 판정은 사용자 몫이다. Resources 신규 설치가 필요하면 정확한 상대 asset ID와 물리 위치를 RESULT에 기록하며 Git에 binary를 넣지 않는다.


## G04. 2026-09-10 전체 슬롯과 방패 배치 확대

사용자가 승인한 F17140 full 문서와 해당 native 함수는 바이트 단위로 보존한다. LMB17000부터 Q/W/E/R/A/S/D/F/T/X/V/ALT_V의 현재 skillbindings와 source receipt를 대조해 나머지 full 문서를 생성한다. 원본 활성 occurrence는 현재 raw action의 enabled, 실제 첫 LOD와 stage를 다시 확인한다. 설치 원본의 shader cache layout은 09-09 저장 layout을 재사용하지 않고 현재 package에서 읽는다.

V의 원본 PlaySkeletalMesh notify는 mesh/clip이 None이므로 이 빈 notify를 모델 복원 근거로 쓰지 않는다. 실제 FX_SM의 방패 geometry와 원본 재질을 조사한 뒤 사용자가 요청한 5방향 배치를 별도 저작 요소로 연결한다. ALT_V는 원본 SuperGProtection의 sk_wgl_gdd_01과 fx_w_wgl_gdd_02를 조사하며 외곽 큰 원6개와 캐릭터 주변6개 배치를 명시적으로 구성한다. 요청한 배치 수와 반지름 보강은 project-authored이고 원본 occurrence와 구분한다. 정상 source shield와 번개를 유지하고 실제 미지원 Solo 의존성은 full에서 제외해 정확 ID와 사유를 기록한다.

기존 build_warlord_asvf_full_restore.py를 전체 생성 옵션으로 확장한다. 공용 Runtime/Tool/Catalog/ResourceTree/project 등록과 통합 컴파일은 root가 수행한다. 워로드 전용 shader/header는 기존 native 함수의 변경 없이 새 프로그램1000~1199/2000~2199를 연결한다. 생성 JSON의 stable ID, source anchor, 모든 Resources 존재, 실제 codec Load/Save/Solo를 검증하고 사용자 실행은 대기한다.
