# 가디언 검격·입자·바닥 재질 연결 구현 계획

## G00. 현재 소비 경로와 사용자 증상

가디언 Full Restore 문서는 44개이며 native program 3968 이후를 사용한다. 실제
`Shader_VtxEffectTrail.hlsl`과 `Shader_VtxEffectDecal.hlsl`의 pixel 진입 범위는 3967에서
끝나서 문서와 CPU 검증이 성공해도 가디언 검격·바닥이 해당 native 계산에 들어가지 않는다.
기존 R clip1의 정상 mesh 계산은 유지하며, LMB/Q/D/F 검격과 LMB/A/D 바닥의 실제
carrier 소비 범위를 현재 registry 상한 4543과 맞춘다.

원본 animation-trail PS 3970/3971/4028/4145/4146/4186은 TEXCOORD0.zw를 읽는다.
현재 generator는 이 경로에 0을 넣는다. 정확한 source VS의 `mov o2.xyzw, v3.xyzw`를
확인한 동일 계약에만 기존 runtime uv1을 공급한다. 색·alpha를 상수로 우회하지 않는다.

## G01. shader와 생성기

수정 파일은 `Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl`,
`Shader_VtxEffectDecal.hlsl`, `Tools/EffectPipeline/generate_artist_native_runtime_shader.py`와
그 생성 결과 중 검토한 Guardian native group이다. 새 C++ 파일·project/filter 등록은 없다.
원본 distortion companion PS도 같은 source VS/UV 계약을 확인한다.

Sprite 사각형과 missing mesh는 source material equation, native dispatch, 실제 UV/color/dynamic
입력과 geometry를 순서대로 대조한다. 재현된 추가 결함은 해당 file/field에만 반영한다.
Guardian ALT V의 camera/sequence, Z와 Play All은 병렬 담당자가 소유한다.

## G02. 데이터와 검증

필요한 occurrence field만 latest disk hash 확인·백업 후 병합한다. 원본 generation 후보의 전체
문서로 사용자 저장본을 덮어쓰지 않는다. runtime/CPU/shader focused 검증으로 native 진입,
finite 계산, nonzero drawable, 실패 격리를 확인한다. root 담당자가 Product build를 수행한다.
Client/UI를 실행하거나 화면을 캡처하지 않는다. 사용자 첨부의 형태 분석과 사용자 최종 화면
판정을 분리하며 R clip1 정상값을 임의로 변경하지 않는다.

## G03. 확인한 native 입력과 특정 occurrence

Masked LocalVF 8개는 CB0[0].RGBA, 추가 5개는 원본 world position·WorldToLocal·camera·MacroUV를
요구한다. 원본 PS/VS ID와 CB0 unowned prefix를 검증한 뒤 generator에서만 복원한다.
Trail은 실제 strip world derivative와 UV로 TBN을 복원하고 검증된 UV0.zw에 uv1을 전달한다.
D의 4156..4159 바닥은 기존 floor receiver 분기를 소비하며 Guardian rigid character material
84를 투영 대상에서 제외한다. LMB 마지막 clip의 notify007/008 검격 7개에만 사용자 요청
90도 yaw를 PROJECT_TUNED로 기록한다. 다른 입자와 R clip1은 유지한다.

## G04. ALT V 원본 projectile closure

원본 action49420의 3.79초 notify044는 SkillEffect494200(Key12, ValueA494200)를 호출한다.
원본 Projectile/494200.loa의 fixed-area CreateFX를 추출한다. 즉시 FireBreath01/LocalFire,
0.4초 Timer의 FireBreath02/03/04, 0.6/1.2초 화면 왜곡, 1.4초 DragonDecal과 light의 원본
TRS·parameter·emitters를 기존 sourceRecipe/native 경로로 투사한다. 각 Timer 자식 수와
byte boundary 및 source SHA를 함께 검증하고 fixed-area의 -30cm 전방 offset을 한 번 반영한다.
현재 49420 effect의 stable ID 요소만 추가하며 다른 담당자의 camera/backdrop/model 필드는 보존한다.
DragonDecal의 실제 material/분포를 확인해 원본 바닥 흔적과 skeletal shadow 주장을 구분한다.

## G05. 새 S carrier와 공용 registry

독립 담당자가 복원하는 49290은 신규4532..4552를 사용하므로 native gate/installer 상한을
4607로 동기화한다. 4542의 source beamtrail VS UV0.xyzw 및 4543/4544의 LocalDecal
CB0 prefix·sky suffix를 원본 DXBC로 확인해서 exact ID에만 기존 adapter를 추가한다.
