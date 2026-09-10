# Artist D/T/V/Alt V 원본 복원 결과

2026-09-09. `codex/dimensionmaster-tool-round3`, 시작 HEAD `591012dbebf7eeab0b660baec42852b9396e77d4`. 기존 dirty 변경, unified 문서, 사용자 튜닝과 Resources를 보존했다. Client/UI 실행·조작·캡처와 육안 판정을 하지 않았다.

## G00. 실제 반영 범위

원본 활성 첫 LOD 후보396개에서341개 element와 별도 skeletal modelCue2개를 새 full.restore5문서에 넣었다. 모델 cue는 element 통계에 중복 포함하지 않는다. 원본34개 live sibling 의존,11개 source VF 부재,7개 ribbon geometry,2개 EngineDefaultParticle 재질,1개 별도 engine prefix 미연결 요소는 full.restore에서 실제 제외했다. 모든 제외 ID·원본 emitter·material·참조 입력은 `2026-09-09_ARTIST_CORE_FULL_RESTORE_EXCLUSIONS.json`에 남겼다.

| 슬롯 | 문서 | elements | modelCues | 핵심 연결 |
|---|---|---:|---:|---|
| D31490 | effect.artist.skill.31490.full.restore |26|1|TIG skeletal mesh7clips, 원본 MIC/native460, 추가 MakeFlow893|
| T31950 | effect.artist.skill.31950.full.restore |23|1|DRA UV plane2clips, 원본 MIC/native461, helix3개, FilmNoise894|
| V31910 | effect.artist.skill.31910.full.restore |69|0|활성 world/camera particles, mesh13개, light3개|
| ALT_V31930 clip1 | effect.artist.skill.31930.clip1.full.restore |52|0|camera/front particles, light1개, Sky_Mirror1개|
| ALT_V31930 clip2 | effect.artist.skill.31930.clip2.full.restore |171|0|집결/회전 mesh54개, camera particles, light3개, ZoomBlur, Sky_Mirror1개|

Native174개 프로그램을 `Effect_ArtistMaterial.h`와 `Shader_EffectArtistNative.hlsli`에 생성했다. 기존460~559/820~892 번호를 유지하고893 MakeFlow,894 FilmNoise만 추가했다. native shader map과 PS/VS를 고른 뒤 material parameter, static switch, sampler addressing, texture color space, particle color/dynamic/time, source cm 좌표를 연결했다. 현341개 document가 직접 참조하는 고유 Resources244개를 확인했고 누락0개다. 회수 대상 원본 texture240개는 모두 source-qualified texture mapping을 확보했다. 프로그램 수와 문서별 occurrence 수는 서로 다른 통계다.

## G01. 호랑이·용과 geometry

`Data/Animation/Reference/Artist/Artist.projectiles:244`의 D314900은 TIG 별도 projectile model을, :286의 T319500은 DRA FIXAREA model을 지정한다. 선택한 원본 Action에는 추가 PlaySkeletalMesh 호출이 없으며, 이 두 모델이 원본 particle graph 밖에서 누락돼 있었다.

| 모델 | 원본 geometry | 애니메이션 | Resources 상대 경로 |
|---|---|---|---|
| SK_SDM_TIG_00 |10189vertices /47412indices /37joints /1material /UV0|sk_cloudtiger 및 변형7clips|Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel|
| SK_SDM_DRA_00 |1386vertices /7296indices /19joints /1material /UV0|sk_dragonrising_01/02, 각2초61frames|Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel|

UModel glTF geometry와 원본 PSA를 기존 ActorXAssetCooker 경로로 합쳤다. BYTE normalized weights는 float/255로 보존한 뒤 CModel 형식으로 쿠킹했다. DRA는 원본부터 얇은 animated UV plane이다. 이 평면을 입체 용으로 임의 교체하지 않았다. TIG는1.1466초 출생,6.5m/s,11m 거리로1.6923077초 재생하며 짧은 animation 끝은 holdLastFrame이다. DRA는0.6초 출생, 원본 projectile5초 수명과 animation2초를 분리해 마지막 pose를 유지한다.

TIG native460은 MIC `sk_sdm_tig_00.mat.sk_sdm_tig_00aa_mi`, PS `e95dd62780e50244ba30af26a3184188`, textures8개다. DRA native461은 `sk_sdm_dra_00.mat.sk_sdm_dra_00_mi`, PS `2621fdf5b9a2b34a992dda44d574fff8`, textures7개다. 공통 GPU skin VS `474c00ee07e0114598836e68037bbd72`의 입력을 기존 animated CModel shader로 전달한다. 모델별 shader uniform은32 float4 이내다.

별도 static source mesh31종은 기존 CModel 경로로 쿠킹했고 UV0/UV1/COLOR0/tangent와 topology를 보존했다. `Effect/Artist/Meshes/Native/<SOURCE_PACKAGE>/<mesh>.wmodel`에 저장하고 native mesh carrier의 modelPreScale0.01을 사용한다. Sky_Mirror는 `LV_Matte.Mesh.Sky_Mirror_SM`와 `FX_M_MI_M_00.FX_M_Me_FlowerGarden_Sky_01`의 원본 opaque PS/native892를 기존 ordinary MESH로 연결했다. root snapshot yaw -90와 source cm 좌표변환을 명시하며 해석되지 않은 raw flag의 의미까지 원본과 동일하다고 주장하지 않는다.

## G02. 활성 notify·분포·material 누락 수정

현재 설치판 YINYANGSHI.loa1,188,519bytes의 raw enabled를 별도 확인했다. SHA256 `54e197d0e511ec5d71eb60269136d4e83100516950cd8373fcbc84b4f9586bc9`. archived imported graph와 설치판 Action은 다른 입력으로 보존하고 clip namespace 차이만 명시적으로 연결했다. asset/time 중복 notify는 FIFO1:1로 대응해 비활성 stage를 활성 stage로 덮어쓰지 않았다. world PPE327개, 활성 CameraPPE67개, StaticMesh2개가396개 후보의 구성이다.

기존 `build_action_cue_recipe.py`의 빈 FString 처리가 길이0의4byte까지 소비하도록 수정했다. 빈 anchor는 anchorNames에서 제외하되 뒤의 scale/Alpha offset을 보존한다. 이 결함이 V B_WP_2 notify와 Alt Floor01을 음수 scale로 오독했던 원인이다. scale을 abs로 바꾸거나 signed scale 검사를 느슨하게 하지 않았다.

Source module2397개는 실제 class CDO/archetype/source override를 합쳤다. 원본 null distribution은 빈 source 경로/빈 lookup table 그대로 보존했다. 동일 module object가 LOD Modules 배열에 반복되면 source ordinal을 stableId에 포함해 두 번의 원본 호출을 유지한다. OrbitOptions는 실제 bool3개로 복원했고 vector ParticleParameter는 parammodes, parammodes[1], parammodes[2]를 사용한다.

Light는 Engine LightComponent→PointLightComponent→EF child→source instance 순으로 상속을 합쳐8개를 typed source_exact light로 연결했다. Local vector field는 원본10×10×10 grid를 `Effect/Artist/VectorFields/fx_cm_05.fx_n_vector_field1.wvectorfield`에 쿠킹했다.

MakeFlow는 같은 object path의 duplicate export 때문에 잘못 고른 parent를 실제 parent export54로 교정했다. FilmNoise는 native uniform constant type15를 해석해 원본 shader map과 PS를 회수했다. 모르는 기본값을0으로 주입해 성공 처리하지 않았다.

## G03. Solo 제외와 시각 입력 경계

34개는 EFParticleModuleLocationEmitter가 다른 살아 있는 emitter의 위치/속도/회전을 요구한다. 현재 공용 Solo runtime에는 그 provider가 없다.7개 Ribbon은 원본 history geometry를 그리는 carrier가 없으며 sprite로 바꾸지 않았다.11개는 Required가 선택하는 VF와 일치하는 compiled material VF가 없다.2개는 EngineDefaultParticle 원본 map을 회수하지 못했다.1개 gprotection PS/VS는 회수했지만 원본 engine CB prefix와 별도 vertex 입력 소비자가 닫히지 않았다. 원본 ID와 required inputs를 남겼고 full.restore에는 포함하지 않았다.

Alt core occurrence 기준 Field02는25개 유지/2개 sibling 제외, Shine03은21개, Cam05는18개, Cam01은17개, SunFlare01은12개, Floor01은10개, Cam03은8개를 유지한다. 따라서 노랑·붉은 원형 회전과 집결의 self-contained source branch는 남아 있다. 실제 색·모양·밀도 일치는 아직 사용자 화면 판단 전이다.

원본 masked6개 프로그램은 discard 연산을 유지하고 원본 RT0 alpha가 opaque pass에서 쓰이지 않는 사실을 carrier alpha1 adapter로 명시했다. 현재 공용 render profile의 masked depth write 동일성은 별도 확인 대상이다. 모델 lighting은 현재 엔진에 원본 SkyLight 계약이 없으므로 실제 scene LIGHT_DESC.vAmbient를 ambient로, sky upper/lower/intensity를0으로 전달한다. 이 값은 현재 engine 입력 해석이며 원본 scene lighting과 동일하다는 주장이 아니다. 원본 VS fog 입력은 현재 native adapter에서 float4(0,0,0,1)의 중립값을 사용한다. 원본 PS 산술을 유지한 것과 원본 scene fog를 되살린 것을 구분한다.

사용자 최신 지시에 따라 camera movement/FOV/shake와 Sequencer는 수정하지 않았다. 원본 UltimateSkillCameraControl2개/관련 ViewShake cue는 분석 자료로만 보존했다. CameraPPE와 Sky_Mirror 같은 실제 시각 effect는 범위에 포함한다.

## G04. 검증 상태와 다음 실행

- 완료: 변경 JSON5개 parse, stableId 중복0, Resources244개 실물/상대 경로 검증, source texture240개 mapping 누락0.
- 완료: 새 generator3개와 parser/test py_compile, Action parser 회귀11개 PASS, 소유 파일 git diff --check PASS(기존 test 파일 CRLF→LF 안내만 발생).
- 완료: 원본→cooked TIG weighted bind 최대오차4.1728e-6/pose2.505e-7 이하, DRA bind9.5367e-7/pose8.74e-8 이하. 기존 source bind/pose 비교 reader 사용.
- root 확인: native animated shader pass7 fx_5_0 컴파일 PASS. 이후893/894 추가본 전체 shader 및 제품 최소 컴파일은 root 최종 검증에 따른다.
- 최신 root 실제 Codec: D26/V69/Alt clip1 52/T23의 Load/Save/Solo PASS. Alt clip2는 source Orbit update 옵션 때문에 미통과이며 전체341개 PASS로 기록하지 않는다.
- source emitterDuration500 수용은 root가 source metadata 상한600으로 확장하여 해결했다. 실제 element/document lifetime과 particle 수 상한은 유지한다.
- 진행 중: Alt clip2 Shine03 꽃잎7개는 원본 Orbit offset Spawn=false/Update=true로600→40cm를 수렴시킨다. 원본 UE3는 매틱 Offset=BaseOffset/RotationRate=BaseRotationRate로 복원한 뒤 Update하며 rotation 위상은 누적한다. 기존 공용 Spawn옵션 무시/offset 누적 경계를 root가 수정한 뒤 최신341개를 다시 확인한다.
- 미실행: 사용자 Character Select 실제 키 입력/화면, Solo/Play All의 visual fidelity 판정. 기존 animevents5행은 이 문서 작성 시점 unified를 참조한다. full.restore 선택과 실제 키 입력 연결 여부는 root 최종 인계에서 구분한다.

Random SubUV38개는20sprite/18mesh이며 mesh18개는 모두 bScaleUV=true,2×2, RandomImageTime1이다. 다만 mesh18개가 사용하는 native536/825/826의 원본 VS는 TEXCOORD0에 geometry UV를 그대로 전달하고 PS uniform에도 texture offset/scale/SubUV 입력이 없다. 이 세 material에 atlas 변환을 무조건 적용하면 원본과 다르므로 원본 UV를 유지해야 한다. Required/SubUV payload의 존재와 실제 shader 소비를 분리한다. 근거는 `out/ArtistCoreRestore20260909/mesh_subuv_material_consumption.json`이다. 반대로 random sprite20개는 모두 실제 SubUV VF이며 원본 CPU가 만든 current.xy/next.zw UV를 VS가 PS로 전달한다. PS9종 모두 current UV를 읽고828/829/867은 next UV도 읽으므로 sprite atlas payload 전달을 유지한다. regular VF로 잘못 분류된 sprite는0개다. 근거는 `sprite_random_subuv_consumption.json`과 원본 `UE3_UnParticleSystemRender.cpp:1377`이다.37개는 RandomImageChanges0으로 출생 이미지가 유지되고, clip2 `a3f8fabb4e63fbd57a41416b`만 changes4/time0.198이다. UE3 Spawn/Update 구현은 changes0에서 Update를 반환하고, 나머지는 이전 상대시간과의 차이가 RandomImageTime보다 클 때 새 난수를 선택한다. emitter 초기화는0.99/(changes+1)을 계산한다. [UE3 원본 C++ 미러](https://github.com/CodeRedModding/UnrealEngine3/blob/main/Development/Src/Engine/Src/UnParticleModules.cpp), [emitter 초기화](https://github.com/CodeRedModding/UnrealEngine3/blob/main/Development/Src/Engine/Src/ParticleEmitterInstances.cpp). 공식 UE3 문서는 Random 무보간과 mesh 전용 ScaleUV를 명시한다. [Epic UDK ParticleSystemReference](https://docs.unrealengine.com/udk/Three/ParticleSystemReference.html).

검증 상세는 `out/ArtistCoreRestore20260909/final_data_resource_validation.json`, `TIG/source_cook_verification.json`, `DRA/source_cook_verification.json`, `random_subuv_live_rows.json`에 있다. root의 공용 통합 검증은 `out/ThreeClassFullRestore20260909/codec-solo.log`와 shader/build 로그를 따른다. Resources는 물리 `Client/Bin/Resources/Effect/Artist`에 준비했으며 Git index에 추가하지 않았다. Drive 전달은 팀장 수행 전이다.

## G05. 전체 슬롯의 조사 시작 상태

조사 시작 시 실제 authored/unified 기준이다. 요소가 visible이거나 hardHold가 없다는 사실은 실제 Solo 성공 증거가 아니다. D/T/V/AltV만 이번 후속 구현으로 새 full.restore를 만들었다.

|슬롯|skillId|문서/실제cue|원본effect cue|요소/visible|hardHold/visible hold|modelCue|
|---|---:|---:|---:|---:|---:|---:|
|LMB|31000|4/4|22|11/10|0/0|0|
|Q|31200|1/1|11|4/4|0/0|0|
|W|31430|1/1|6|1/1|0/0|0|
|E|31480|1/1|11|7/7|0/0|1|
|R|31210|2/2|16|2/2|0/0|0|
|A|31460|1/1|15|2/2|0/0|0|
|S|31420|1/1|5|2/2|0/0|0|
|D|31490|1/1|8|68/68|0/0|0|
|F|31470|1/1|17|17/17|0/0|0|
|T|31950|1/1|6|23/11|15/3|0|
|V|31910|1/1|15|46/46|0/0|0|
|ALT_V|31930|2/2|20|97/97|0/0|0|
|X|31110|0/0|11|0/0|0/0|0|
|Z|31050|2/2|9|3/3|0/0|0|
|SPACE|31020|0/0|5|0/0|0/0|0|
|SPACE|31030|0/0|7|0/0|0/0|0|

원본 cue 수에는 비활성 stage가 섞일 수 있으므로 현재 authored 요소와 바로 빼서 누락 건수로 계산하지 않는다. 핵심4개 스킬은 현재 설치판 raw enabled를 재해석하여396개 first-LOD 후보를 확정했다.

T31950은 기존15개 hardHold(visible3개 helix 포함), 모델cue0이 확정 결함이다. D31490은 원본 projectile TIG 모델이 별도로 있는데 modelCue0인 것이 확정 결함이다. V/AltV의 비활성stage/CameraPPE/source MIC·VF·CDO 누락은 후속 전체 source 조사로 교정했다. 나머지 슬롯은 이 조사에서 현재 authored 구조/참조/resource 실물까지만 대조했고, source shader 계산식과 실제 Solo 전수 판정은 하지 않았다. X31110/SPACE31020/31030은 현재 authored effect document와 제품effectcue0이며 이는 데이터 연결 공백이다. gameplay command 지원 범위와 구분한다.

현재 상세 구현 결과: .md/GB/09-09/2026-09-09_ARTIST_CORE_FULL_RESTORE_IMPLEMENTATION_RESULT.md. sourceID별 실제 제외55개: .md/GB/09-09/2026-09-09_ARTIST_CORE_FULL_RESTORE_EXCLUSIONS.json.

실제 입력 파일: Data/Animation/Authored/Artist/Artist.skillbindings.json (D55/T67/V73/Alt79행), Artist.animevents (D1023/V1029/T1032/Alt1035~1036행). 별도 모델 원본: Data/Animation/Reference/Artist/Artist.projectiles244/286행. 현재 Artist native 계약은 Client/Public/Effect_ArtistMaterial.h6557/6597행, shader는 Client/Bin/ShaderFiles/Shader_EffectArtistNative.hlsli.


## G06. 09-10 전체 슬롯 full restore와 소환 재생속도 교정

사용자 후속 요청을 반영하여19개 full.restore 문서의540개 element와 modelCue3개를 준비했다. 이전 D/T/V/Alt V의 정본 native 번호와 unified/손튜닝 문서는 보존했다. Artist 전체 native는 기존174개에93개를 추가한267개이며 신규 구간은1600~1694 중1683/1692를 제외한 번호다. 새 HLSLI group은1600과1664이며 기존 group과 중복 정의하지 않는다. Native1678~1694를 추가하기 전에 기존1600~1677의 `(sourceMaterial, sourceVF)` 순서를 보존했다.

|슬롯|full.restore suffix|elements|modelCue|
|---|---|---:|---:|
|LMB31000|ba1 / ba2 / ba3 / ba4|8 /4 /7 /12|0|
|Q31200|full.restore|29|0|
|W31430|full.restore|20|0|
|E31480|full.restore|18|1|
|R31210|ba1 / ba4|10 /10|0|
|A31460|full.restore|26|0|
|S31420|full.restore|2|0|
|D31490|full.restore|26|1|
|F31470|full.restore|33|0|
|T31950|full.restore|23|1|
|V31910|full.restore|69|0|
|Z31050|clip1 / clip2|2 /19|0|
|Alt V31930|clip1 / clip2|52 /170|0|

일반9개 스킬(LMB/Q/W/E/R/A/S/F/Z)의 활성 source230개를 현재 설치 Action/첫 LOD에서 다시 조인해200개를 연결했다. LMB의 원본 clip 순서와 기존 ba1~ba4를 유지했고, R의 동일 clip 이름을 쓰는 다른 source stage가 event ID dict에서 덮어쓰이지 않도록 정확한 cueId로 조인했다. W/F의 imported one-layer 이름 잘림은 실제 native emitter와 TypeData 부재를 확인해 sprite branch로 다시 읽었다. F Light의 같은 이름 `Size` scalar와 vector parameter는 typed lookup으로 분리하여 vector lookup이 다른 type의 값을 소비하지 않게 했다.

F는 이전17개만 이름을 바꾼 것이 아니다. 원본35개 중 native29개와 기존 RuntimeMaterialV2의 정상 Ribbon/weapon/LocalDecal2개를 정확한 renderer/emitter digest로 이어33개가 됐다. 기존4개는 새 native branch와 중복 생성하지 않는다. 나머지 decal1개는 원본 decal VF, 돌mesh1개는 source VS `0c1413bd3ee54d449ce7fdac8c7f1542`의 TEXCOORD7 및 별도 engine CB0 입력이 현재 carrier에 없어 제외했다. 핵심 무기 mesh의 같은 새 native 입력 제약은 이미 소비자가 있는 기존 RuntimeMaterialV2 carrier를 유지해 해결했다.

`particleModuleLocationEmitter`와 `EFParticleModuleLocationEmitter`는 모두 다른 살아 있는 emitter의 위치/속도 공급자가 필요한 같은 Solo 경계다. S의 `authored.source-particle.artist-full.217b0c54761568407c67881a`가 전자를 사용하여 문서 전체 Load를 막던 요소였으며 해당 행만 제외했다. 신규 일반 source 제외30개는 ribbon16, VF8, default material2, live sibling3, 별도 native input1이다. 이전55개와 Alt V의 복수 Orbit LINK 꽃잎1개를 합친 전체 제외86개는 대응 EXCLUSIONS JSON에 source ID·material·module 이유를 유지했다. 전체 원본후보626개와 실제element540개를 같은 단위로 기록하며 modelCue3개는 별도다.

### G06-01. 용과 호랑이가 정지해 보이던 실제 원인

D의 모델은 요청한 `SK_SDM_TIG_00`이며 원본 `sk_cloudtiger`와 변형7개 clip이 존재한다. T는 미르 새김 projectile 정본이 지정하는 `SK_SDM_DRA_00`과 `sk_dragonrising_01/02`가 맞다. DRA는 원본부터19개 joint의 얇은 animated plane이며 skin pose의 Y범위는0.0131~0.0359m로 바닥을 따라 움직이는 형상이다. screenshot에서 관찰한 얇은 검은 선만으로 모델 identity를 다른 용으로 교체하지 않았다.

Cooked 파일은 실제1000ticks/s와 TIG733.333ticks, DRA2000ticks를 저장했다. `WAnimationReader`는 그 값을 그대로 읽지만 제품 `CAnimation`의 고정 cooked clock은30ticks/s이므로 TIG는24.444초, DRA는66.667초 속도로 늘어나 실제cue수명 동안 거의 처음 pose만 보였다. 기존 `retime_wmodel_ticks.py`로 duration과 모든 key time을30/1000으로 정규화하고 stored ticksPerSecond30으로 바꿨다. TIG의 실제0.733333초와 DRA2초는 보존했으며 geometry/skin weights/key 값은 바꾸지 않았다. 전체 Character/전투 속도를 바꾸는 전역 `CAnimation` 수정은 하지 않았다.

원본 T projectile은 FIXAREA·speed0·수명5초이며 용 body가2초 동안 ground plane 안에서 변형된다. 고정영역 projectile을 임의 속도 missile로 바꾸지 않았다. 이번 복원은 원본 ground animation의 올바른 clock을 회복하며 실제 방향/거리·색·노출 시각 판정은 사용자 확인 전이다. 기존 cue의5초생명/마지막pose 유지와 D의6.5m/s·11m carrier 이동 계약은 보존했다. Generator도 재생성 시 TIG/DRA clock을30으로 정규화하므로1000ticks 재쿠킹으로 다시 퇴행하지 않는다.

### G06-02. 파일·리소스·자동 검증

수정된 generator3개는 추가 native 번호 할당, 기존 Artist contract/group 보존 확장, typed Solo 제외, E crane/기존 F 정상 carrier 유지, TIG/DRA tick 정규화를 소비한다. 신규 Authored14개, 기존 Alt V clip2의 꽃잎1개 제거, `Effect_ArtistMaterial.h`, Artist main HLSLI 및 group1600/1664가 이번 소유 산출물이다. 공용 Catalog/ResourceTree/animevents/project/renderer와 screen.motion-blur contract는 root 통합 담당이 연결했다.

- 완료:19 JSON parse·finite·stableId중복0, Resources상대경로340개 실물 및 누락0. `out/ArtistAllRestore20260910/final_data_resource_validation.json`.
- 완료: 신규 source geometry13개를 추가쿠킹해 이 작업이 사용하는 cached native mesh44종 실물 확보. 모델 runtime 경로는 기존 CModel이다.
- 완료: 새 source texture43개 추가회수. 이번 일반/F source가 요구하는168개 texture mapping 누락0이며 파일은 `Effect/Artist/Textures/<SOURCE_PACKAGE>/...`에 있다.
- 완료: TIG7/DRA2 clip 모두1000→30tick 변환. `tiger_tick_retime.json`, `dragon_tick_retime.json`; 기존 source/cooked weighted pose 대조 두 개 PASS. 최대weighted bind오차는 TIG4.17283e-6, DRA9.53675e-7이며 정규화 후에도 source pose 일치를 유지했다.
- root 완료 보고: 실제 CModel SetTrack/Play(0) probe의6소환모델/21clip/189pose가 finite이고21clip모두pose변화를 보였다. 근거 `out/ThreeClassFullRestore20260910/summon-models.log`.
- 완료: generator3개 py_compile 및 `git diff --check`. 이번 소유29개 파일의 trailing whitespace를 별도 확인해0건이다.
- root 완료 보고: Client C++/FxCompile PASS. `out/ThreeClassFullRestore20260910/client-compile-shaders.log`. shader warning은 존재하며 성공을 visual PASS로 해석하지 않는다.
- root 통합 완료: 실제 Codec/Save/Solo19문서540요소 통과. 최초18문서507요소는 `out/ThreeClassFullRestore20260910/codec-final.log`, 수정한 F33요소는 `codec-final-fixed.log`에서 통과했으며 hidden/locked/unexpected0이다. F는native-v14 source candidate의purpose/renderer/recipe evidence가ordinary-v13에 남아있어 거부됐던 원인을 기존 Artist F materializer의SOURCE_ONLY필드 계약으로 교정했다. source evidence는 입력 receipt에 보존하고 runtime consumed 필드만ordinary-v13으로 출력했다. F33개 ID·material·transform·module/distribution 값은 유지했다.
- 완료: 실제 Artist.animevents의19개 EFFECT payload를full.restore로 교체하고 EffectCatalog에 등록했다. LMB _01=ba3/_02=ba2/_03=ba1/_04=ba4, R _01=ba1/_02=ba4를 유지한다. A만 기존31460.linear-reveal.unified를31460.full.restore로 치환하고 나머지는.unified를.full.restore로 치환했다. `out/ArtistAllRestore20260910/final_event_mapping.json`과 `out/ThreeClassFullRestore20260910/runtime-binding-result.json`이 대응 근거다.
- 미실행: Client/UI 실행·조작·캡처와 사용자 실제스킬/화면 판정. source프로그램/수치검증은 visual PASS가 아니다.

원본 모델의 정확한 runtime asset ID와 물리 위치는 `Client/Bin/Resources/Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel`, `Client/Bin/Resources/Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel`이다. E crane의 `Effect/Artist/Meshes/SK_SDM_RCC_00_SK_FX_01/SK_SDM_RCC_00_SK_FX_01.wmodel`도full.restore에 유지했다. Native mesh는 `Effect/Artist/Meshes/Native/<SOURCE_PACKAGE>/<mesh>.wmodel`, texture는 `Effect/Artist/Textures/<SOURCE_PACKAGE>/<texture>.dds`에서 소비한다. Resources실물은 준비됐으며 Git추적/force-add하지 않았다. Drive전달은 팀장수행전이다.

## G07. 사용자 재검토 후 미르새김 몸통 횡폭 조정

사용자가 새 실행에서도 원작보다 얇은 몸통을 보고했다. 현재 설치본은 어제 PSA non-root 회전을 교정한 `e548f69…` 모델과 byte exact였고 회전 수정의 퇴행은 없었다. 원본 projectile319500의 scale1/FIXAREA speed0, GPU skin VS의 unpack→skin→world/projection 경로를 다시 확인했으며 추가 폭 scale/WPO 누락 근거는 찾지 못했다. 어제 변경은 뒤집힌 면을 고친 것이고 원본 bind 폭0.190895m를 유지한 상태였다.

이번 사용자 요청의 두꺼운 몸통은 PROJECT_AUTHORED 횡폭5배 조정으로 설치했다. bind 폭은0.954474m, 길이는1.80681m로 기존과 같다.20bone·2clip·1386정점의 가중치와 원본 animation payload, 사용자31950 문서23Element/1ModelCue 및 별도 helix3개를 보존했다. global cue Scale 대신 bind 횡폭만 조정하므로 골격 중심 경로를 늘리지 않는다.

기존 `Tools/ActorXAssetCooker/rebuild_artist_dragon.py`에 `--body-width-scale`을 추가했고 기본값은1이다. 위치 횡폭과 normal inverse-transpose/tangent 정규화를 함께 처리한다. CLI5로 다시 만든 mesh/material/skeleton은 검증된 후보와 byte exact였으며 quaternion q/-q의 물리 회전 차이는0이었다. 기존 clip bytes를 완전히 보존하는 검증 후보 `b729893…`를 원본 hash 확인 후 설치했다. 바로 전 모델은 `out/ArtistWarlordVisualFollowup20260910/DRA/width-resume-before/`에 보존했다.

122개 CPU pose가 모두 finite이며 면적은 기존 대비4.787~4.833배다. 최대 뒷면 면적 비율0.027868%는 남아 있고 전체 면을 강제로 양면 렌더링하지 않았다. Python 문법, JSON/glTF parse와 scoped diff check는 통과했다. 근거는 `out/ArtistWarlordVisualFollowup20260910/DRA/width_resume_install.json`이다. 현재 설치본은 기존 실제 CModel probe에서 소환6모델·21clip·189bone palette가 finite이고21clip 모두 움직임을 확인했다(`out/ArtistWarlordVisualFollowup20260910/DRA/width-installed-cmodel.log`, errors0). 사용자의 두께 화면 확인은 아직 미실시다.


## G08. Alt V 단일 restore와 꽃밭 drawable 복구

사용자는 기존 clip1의 컷신 카메라가 정상이라고 관찰했고 clip2가 invalid로 재생되지 않는다고 보고했다. 기존 clip2의 꽃밭32행은 숨김 위치 provider2개와 실제 표시30개로 존재했다. 일반 Load/Save와 Stage의 provider graph는 유효했지만 `CEffectDocumentCodec::Validate_Drawable`가 숨김 provider에도 Base texture를 요구했다. 기존 루프에 simulation-only 제외 조건을 추가해 graph·순서·참조 검사는 유지하고 실제 drawable resource 검사만 제외했다.

새 `effect.artist.skill.31930.full.restore`에 clip1의52행과 clip2의202행을 합쳤다. 원본 Artist animation의52tick/30Hz를 기준으로 clip2 Element 시작만1.733333초 뒤로 옮겼다. 카메라는 기존 clip1 끝1733ms에 clip2를 붙여 공백0ms이며, animation2의 시작은 기존 row 길이1734ms를 유지한다. 이 정수 반올림 차이는 각각 -0.333ms/+0.667ms다. 카메라4행·1065key, 첫 clip의458key·animation, 전체254Element의 시작 지연 외 필드와 provider 참조를 보존했다.

Catalog/ResourceTree에서 `이펙트_도화가AltV_전체` 하나를 선택하고 첫 `sdm_sk_super_pungnyudo_01` animevent에서 NATURAL로 한 번 생성한다. 두 번째 clip의 중복 full notify는 제거했다. 새 full과 sequence를 Client 프로젝트의96.DataFiles None 항목에 연결하고 기존 clip별 full/sequence4개는 정확한 byte 백업 뒤 저작 목록에서 제거했다. 이전 unified 비교본과 sourceNode의 원본 출처 표기는 보존한다.

| 검사 | 실제 결과 |
|---|---|
| 실제 Codec/Playback | full254행 Drawable, Save/Load 동일 직렬화, Stage 성공 |
| 꽃밭 실제 계산 | 표시30행 모두 양수 alpha·변하는 위치, provider draw0·nonfinite0 |
| provider를 포함한 개별 재생 |30개 모두 Drawable/Stage와 시간 이동 성공 |
| 시간 이동·실패 보존 |5시점 Seek 결과 delta0, 잘못된13입력 거절과 기존 상태 보존 |
| 통합 카메라 실제 소비 | actual Load4행·1065key,2root/3aspect에서12,138sample의 eye/look/up/FOV 차이0,36,450check·failures0. 경계 공백0ms·종료3933ms |
| 데이터·프로젝트 | JSON4/XML2 parse 및17개 보존 비교 모두 성공, 전체 변경 JSON/XML17개 parse와 git diff --check 성공 |
| 최종 제품 EXE | Debug Product compile/link·DLL/CSO 배포 PASS (`out/BuildPipeline/runs/20260910T063843956Z-debug-product.json`) |
| 새 실행의 화면 | 사용자의 카메라→꽃밭·미르 몸통 두께 최종 관찰 대기 |

근거는 `out/ArtistAltVUnified20260910/merge-receipt.json`, `final-data-checks.json`, `provider-merged.log`, `camera-merged.log`, `final-parse-diff.json`과 `handoff.md`다. 백업 정본은 `before-exact/`에 있다. 복사한 옛 generic Solo probe는 provider를 잘라내는 자체 구성 때문에32행을 거절했으나, 실제 provider closure를 포함한 위 검사30개는 모두 통과했다. 그 옛 probe의 실패를 full 문서 invalid로 기록하지 않는다.

원본의 emitterDuration500초인 burst-only6행 때문에 Playback의 종료 하한은506.633초다. 해당행은 rate0·1회 burst·입자 수명2~2.25초여서8분 동안 입자가 계속 표시된다는 뜻은 아니며, NATURAL 빈 객체가 오래 남는 기존 경계다. 시간 분포를 임의 변경하지 않았고, 전체 source emitter의 안전한 종료시간 계산 최적화는 이번 단일 restore 연결과 분리해 미구현으로 남겼다.

## G09. 도화가 전체 스킬 이펙트 미발생 긴급 수정

Alt V 두 EFFECT 행을 단일 restore로 합친 변경에서 Artist.animevents 헤더의 이벤트 수가1048로 남았다. 실제는1047행이라 CAnimationEffectCueDocument::Load_FromText가 `Animation event row count does not match the header.`로 문서 전체를 거부했다. 그 결과 Product prewarm과 Character::Load_EffectCues에 도화가 제품 이펙트가 등록되지 않았다.

헤더1048→1047 한 줄을 수정했다. 기존18개 제품 EFFECT, 원본 이벤트, 저작 이펙트 및 C++ 검증은 그대로다. 실제 제품 Debug 객체에 연결한 기존 console 진단을 사용해 수정 전 헤더의 거부와 수정 후 로드를 대조했다. Client/UI는 실행·조작하지 않았으며 실행 중인 Client/Server와 제품 EXE를 교체하지 않았다.

| 검사 | 실제 결과 |
|---|---|
| 수정 범위 | Git 기준 헤더 한 줄만 변경, 기존 이벤트 내용 보존 |
| 데이터 검사 | declared1047=actual1047, 제품 cue18개 Catalog 등록·저작 JSON parse 성공 |
| 실제 Product prewarm | Load_ForProductPrewarm 성공, admitted cue18개 |
| 실제 캐릭터 로더 | Artist wmodel의 실제99개 clip으로 Load 성공, cue18/unavailable0/sound163/hit14 |
| 수정 전 오류 대조 | 메모리에서 이전1048 헤더를 넣으면 행 수 오류로 거부, 이미 로드된18개 cue 보존 |
| 최소 컴파일 | 기존 제품 Debug 객체를 사용한 console 진단 컴파일·링크·실행 성공. C++ 변경이 없는 데이터 수정이므로 제품 재빌드 불필요 |
| diff 검사 | git diff --check 성공 |
| 사용자 화면 | 미확인. 실행 중인 Character Select에서 다른 class→Artist로 재선택하거나 재입장하여 새 Character가 정본을 다시 읽은 뒤 확인 |

근거는 `out/ArtistEventHeaderFix20260910/data-checks.json`, `out/ArtistCueRestore20260910/artist-cue-validation.log`다. 에이전트는 화면 확인을 대신하지 않았다. 사용자 우선순위에 따라 도화가 오류만 처리했고, 앞서 진행하던 쿠크 변경과 차원술사 BA 검토는 보류했다.

## G10. 기존 Product 선택과 Alt V GPU 발생 수 수정

사용자 지정 Q31200/W31430/R31210의ba1·ba4/S31420/F31470을 기존 unified로, A31460을 기존 linear-reveal.unified로 연결했다. 다른11개 제품 cue와1047행 헤더는 유지한다. 실제99개 model clip의 제품 loader는18 cues/unavailable0/sounds163/hits14이며 prewarm도18개가 등록됐다.

Alt V254행 중 simulation-only 위치 provider2개는 Playback::Rebuild_Frame에서 GPU 대상에서 제외하지만 Renderer::Resolve_GpuRenderFamily에서 sprite로 집계됐다. helper가 동일한 provider를 END로 분류하도록 고쳐 전체 발생 수·순서 검사를 유지한다. 실제 Renderer TU 컴파일과 현 Debug 객체에 연결한 Load/Stage/Render 진단으로16시점 모두247 GPU occurrence를 일치시켰고, 잘못된 frame의 한 occurrence 삭제는 계속 거부했다. 진단은 실제 평가된 frame을 소비하되 submission element set을 비워 GPU draw/UI 없이 Render admission 오류를 검증했다. 이는 화면 fidelity 판정이 아니다.

근거: out/ArtistAltVRenderFix20260910/{compile.log,render-validation.log,product-cues.log}. 최종 제품 빌드와 사용자의 Alt V 재생은 후속 통합 검증 전이다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.


## G11. 실행 후 꽃밭·하늘 합성 범위 재확인

사용자는 새 EXE 실행 중 Alt V의 꽃밭/스카이박스가 연결됐는지와 배경을 강제로 맨 앞에 그린 것인지
질문했다. 읽기 전용 조사에서 full254행의 Sky_Mirror_SM2개 및 Field01 provider2+표시30개를 다시
확인했다. Sky의 native892 재질은 opaque_back_depth_write이며 Select_Pass0이
Shader_EffectMeshFamilyCarrier의 OpaqueBackDepthWrite → DSS_Default를 사용한다.
Engine_Shader_Defines의 깊이 검사는 true, 쓰기는 all, 비교는 less_equal이다. clip2 구체는 반경71.68m,
위치[0,-1,-5]/yaw229.998779/scale1.75, 원본 하늘 texture와 player-relative 생성 기준을 사용한다.
따라서 현재 구현에 깊이 검사를 끄고 모든 맵 위에 덮는 sky 전용 처리가 있는 것은 아니다.

원본 action/UPK Kismet에는 Sky spawn·local-only flag·player-attached camera와 Matinee 곡선이 있다.
반면 shader bytecode 자체가 전체 장면의 draw order나 월드 visibility 정책을 소유한다고 볼 수는 없다.
현재 generator는 원본 nativeBlend/nativeTwoSided로 renderProfile을 선택한다. 이는 원작 custom native의
모든 D3D 상태·장면 차폐 처리까지 해독·재현했다는 근거가 아니다. HidePawn Parts9 및 조명 control
rollback 경계는 기존 조사와 같이 미확정이다. 깊이 검사가 있으므로 가까운 기존 맵 geometry가 남는
현상이 보인다면 우선 원본 장면 visibility/native 명령의 미복구 범위를 추가 조사해야 한다.

이번 확인은 코드/실행 데이터 변경이나 UI 조작 없이 수행했다. 렌더링 복원 정본의 오래된 꽃밭222행/
Field01 제외 표기만 현재254행·provider 지원 상태로 정정했다. 사용자의 최종 화면 판정은 대기한다.
