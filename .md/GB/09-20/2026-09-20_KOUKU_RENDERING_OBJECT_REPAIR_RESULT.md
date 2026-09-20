# 쿠크 렌더링과 Object 앵커·충돌 재생 수정 결과

기준일: 2026-09-20. 기존 `GB/koukubugfix-bingo`의 다른 세션 변경을 보존하고 아래 consumer만 수정했다. 전체 Product 빌드·게시와 공통 문서 갱신은 통합 작업에서 취합한다. Client/UI를 실행하지 않았다.

## G01. RenderingProfileService와 Scene Bloom

`RenderingProfileService.h/.cpp`의 `Apply_CameraEnvironment`가 기존 region 평가에 연출용 임시 fog/light 입력을 받는다. 매 프레임 이전 임시 상태를 먼저 복원하고 현재 region을 계산한 뒤 필요한 override만 적용한다. 컷씬 종료와 다른 rendering profile commit 시 이전 override가 새 profile을 덮지 않는다.

`MainApp.cpp`는 쿠크 cinematic camera 소유 또는 발탄 `Is_CinematicCameraActive()` 동안 fog를 끈다. 발탄 getter는 발탄 작업에서 추가했다. `Level_KakulSaydonArena::Needs_Gate2IntroCharacterLight()`는 `kouku.gate2.intro.camera.3`이 실제 카메라를 소유하고 복귀 중이 아닌 경우에만 참이다. 해당 shot의 13950~19490ms에 15048ms가 포함된다. 이때만 before-restoration directional light를 SOURCE_CHARACTER receiver로 제한해 복구한다.

G1의 실제 과광 경로는 baseExposure 2 × book-open/popup exposureMultiplier 1이었다. Source LUT02의 중성 입력 약0.533은 약(0.761, 0.741, 0.678)로 올라간다. Source tone mapping 분기는 먼저 반환하므로 Hable gamma를 다시 통과한다는 증거는 없다. 활성 source map light84개는 UNBAKED receiver이며 baked flag 거부와 baked ambient 배제 경로가 이미 존재한다. popup 복사본도 receiver를 유지하고 현재 project popup spotlight(source.light.1)의 brightness는0이다. 따라서 prebaked light 중복 또는 spotlight 중복이 직접 원인이라고 확정하지 않았다.

`Data/Rendering/Authored/RenderingProfiles.json` revision66→67에서 `scene.kakulsaydon.g1.book-open.v1`, `scene.kakulsaydon.g1.popup.v1` 두 alias의 exposureMultiplier만0.5로 바꿨다. G1 유효 exposure가1이 된다. G3와 source LUT, 전역 bloom은 보존했다. rendering runtime 게시와 필드 검증을 완료했다.

카드9개 V2 leaf는 emissive slot이 비어 있어 기존 bloomIntensity가 scene bright-pass를 제어하지 못했다. `EffectV2_Object.h/.cpp`, `EffectV2_Document.cpp`, `Effect_Tool_V2.cpp`, `Shader_EffectV2_Common.hlsli`, V2 Python validator에 optional `sceneBloomScale`을 추가했다. 기본1, 범위0~1이며 ScreenPost/Multiply는1만 허용한다. scene bloom RGB만 곱하고 alpha coverage와 본체색은 유지한다. leaf는 white_1 및 heart/spade/clober/dia 각각 red/black의9개이며 이 occurrence 계열만0을 저장했다.

데이터 교체는 준비 후보의 최신 disk/hash 재확인, 원본 backup, 영향 필드 병합과 atomic replacement를 사용했다. 경로·hash·필드 목록은 `out/RenderingRepair20260920/install.receipt.json`과 `field-patches.json`에 있다. 실행 중 draft나 Server 메모리가 바뀐 것으로 취급하지 않는다.

자동 검증:

- RenderingProfileService, EffectV2_Document, EffectV2_Object, Effect_Tool_V2, MainApp, Level_KakulSaydonArena의 /Zs 통과.
- V2 shader7개 fxc fx_5_0 /O1 통과.
- V2 focused Python tests21개 통과. 전체 suite의 기존 repository group-count expectation은 현재 별도 bingo 추가와 불일치하여 별도 보고했다.
- 실제 환경 override 메서드를 추출한 headless probe에서 반복 적용, region 전환, 종료, light 실패 rollback, fog-only, profile handoff 통과.
- 변경 JSON parse 및 `git diff --check` 통과.

## G02. WorldObjectTool의 중앙 Map 앵커와 즉시 표시

`WorldObjectTool.h/.cpp`에 `Render_MapAnchor`와 `Use_AuthoredMapPreview`를 추가했다. parent Object와 combined group의 `Parent Map Position`은 연결된 WORLD instance position들의 중심을 표시하고 모든 해당 instance를 동일한 XYZ delta로 옮긴다. 개별 state의 template, emission 간격, local motion key, collider/effect 수치는 바꾸지 않는다. 후보 Document Validate가 실패하면 기존 draft를 보존한다.

명시적 parent/child Map Position 편집이나 Move-to-Character 명령은 `m_PreviewAtCharacter`와 `m_CompositionPreviewPlacement`를 해제하고 현재 clock으로 Seek한다. 기존 override가 저장된 좌표를 가리던 문제를 해소한다. parent의 Preview Default도 더 이상 Preview-at-Character를 강제로 켜지 않는다. 일반/즉사 칼날과 갈고리의 별도 상태를 유지하고 Y도 같은 앵커 입력으로 조절한다.

## G03. WorldSequencePlayer의 finite emitter 반복

`WorldSequencePlayer_Objects.cpp::Apply_ObjectEffects`의 V1 `loopEffectToDuration`만 수정했다. 준비된 전체 duration에는 particle/after-image tail이 포함된다. 이를 단일 cycle 교체 간격으로 사용하면 emission 종료 후 tail만 남는 구간이 생긴다.

finite source에서는 visible element의 start delay + emitter delay + emitter duration × loop count와 model cue의 end를 계산해 재발생 주기로 사용한다. tail-inclusive prepared duration은 각 cycle의 종료 시점에 사용한다. 현재 clock에서 필요한 모든 source-cycle handle을 구하고 이전 cycle tail과 다음 emission을 동시에 유지한다. 직접 Seek/뒤로 Seek도 같은 식을 사용하며 overlap1024와 정수 identity 범위를 검사한다. native infinite emitter와 V2, 일반 one-shot 재생은 기존 경로를 유지한다. 원본 emitter loopCount나 SourceRecipe를 변경하지 않는다.

일반/즉사 source의 원본 emission 주기는1초다. source field로 계산한 tail-inclusive duration은 각각4초이고 element 수8/9, bloomIntensity 약1.3은 보존되어 있다. 새 계산은1초마다 emission을 시작하고 최대4개 tail cycle을 겹친다. 실제 prepared projection duration은 기존 서비스가 공급하므로 source field 산출값을 GPU 실측으로 기록하지 않았다. Motion/effect track의 명시적 종료 시점은 기존대로 유지한다.

## G04. Object Play와 서버 collider 소비자

`WorldObjectTool`은 collider가 있는 선택 Motion의 저장된 Composition 연결 패턴을 찾아 `Saved Pattern` 목록과 `Play (with Collisions)`를 표시한다. 기존 local transport는 `Visual Play`로 표시한다. MainApp은 stable pattern ID와 source revision만 받아 기존 `KoukuSaydonBossTool::Play_PatternById`에 전달한다. 기존 gate/boss 준비, effect prewarm, gameplay/source revision admission과 `KoukuSaydonPatternAuditionService` lifecycle을 재사용한다. Stop도 같은 서비스에 전달한다.

Object draft가 dirty이거나 게시 중이면 저장본 서버 재생만 비활성화한다. 클릭 직전 Object baseline과 Composition 최신 revision을 재확인한다. 서버 준비/재생 중 local draft preview를 시작하지 않아 중복 visual을 만들지 않는다. UI는 서버 lifecycle과 거절 사유를 표시한다. Object나 ImGui가 플레이어 Transform/HP를 직접 바꾸지 않는다.

`DEBUG_WORLD_PLAYBACK::PLAY_SEQUENCE`는 visual broadcast만 하므로 이 기능의 collision 실행 경로로 사용하지 않았다. 실제 collider는 기존 publisher의 `world_object_collider.py`가 WorldTrack으로 굽고 Server `KoukuSaydonLogicRuntime`이 피해·즉사·HOOK_CAPTURE 및 매 tick 끌림을 평가한다.

현재 저장 데이터의 연결 실측:

| Motion | 연결 패턴 |
|---|---|
| 일반 칼날 state.1 | P31, P33 |
| 원본 즉사 칼날 state.instant_death | P95 |
| original_preview 갈고리 | P33 |

P95는 통합 작업에서 추가한 `즉사 칼날 | Object 판정 테스트`다. 이 Object 수정에서 authoring JSON을 바꾸지 않았다. 서버 재생은 선택된 저장 패턴의 World box placement 및 모든 연결 actor를 사용한다. Object 창의 unsaved 위치를 Server에 보내거나 Composition 박스 위치를 몰래 덮지 않는다. 새 위치를 실제 전투에 쓸 때는 해당 Composition placement 및 저장·게시 계약을 사용한다.

## G05. 검증과 남은 화면 확인

- 최신 WorldObjectTool, WorldSequencePlayer_Objects, MainApp의 /Zs 통과. MainApp에는 통합 작업의 `Render_KoukuEncoreRotation`도 포함됐다.
- 실제 `Render_MapAnchor`, `Use_AuthoredMapPreview`, `StateIds`, finite-cycle 계산 블록을 추출하고 현재 WorldSequenceDocument/DataJson codec을 링크한 headless probe29개 통과. 모든 칼날 state의 동일 XYZ 이동, 무관한 hook 보존, NaN 후보 rollback, 실행 clock 보존, codec Save/Load roundtrip, cycle overlap/퇴역/역방향 Seek/상한 거부를 검사했다.
- 기존 `Tools.KoukuSaydonPipeline.test_world_object_collider`16개 통과. hook capture/carry 구분, 기간 종료, bone grip, finite motion loop와 emission delay를 포함한다.
- 변경 C++ `git diff --check` 통과. 기존 파일의 UTF-8/CRLF와 MainApp 기존 인코딩을 유지했다. 신규 C++ 파일과 프로젝트/filter 등록은 없다.
- 증거: `out/RenderingRepair20260920/compile-object-final.log`, `object-probe.cpp`, `object-probe.log`, `object-data-metrics.json`. 임시 파일은 커밋 대상이 아니다.

Client/UI를 실행하지 않았으므로 GPU 밝기, 실제 프레임 사이 연결 품질, 사용자의 Player에 대한 Server 판정 화면은 미검증이다. 통합 Product/게시 후 사용자가 Parent Map Position의 Y 이동, 칼날 장시간 재생, Saved Pattern의 Server Play와 갈고리 잡힘/종료를 확인해야 한다.

## G06. 사용자 1관문 전투 화면의 세이튼 암부 재검토

사용자는 과광 문제가 해소됐다고 보고했으나 세이튼 몸체가 어두워진 화면을 첨부했고, 해당 구간을 **쿠크 1관문 전투**로 확인했다. 첨부 `codex-clipboard-babd65a2-14d3-4921-9a2c-be899fdc2477.png`를 열어 밝은 바닥에 비해 몸체·머리의 암부가 강한 현상을 확인했다. 이 관찰은 다른 요구사항의 화면 PASS나 원본과의 픽셀 일치 증거가 아니다.

현재 authoring/runtime RenderingProfiles revision67에서 G1의 base/book-open/popup 노출 배율은0.5다. alias의 실효 exposure2→1은 맵뿐 아니라 세이튼의 HDR 입력에도 적용된다. 최종 화면은 source tone/LUT를 통과하므로 픽셀 밝기가 정확히 절반이라는 뜻은 아니다.

추가로 `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl::Resolve_SourceCharacterLight`의 ambient adapter가 `light.diffuse * light.ambient * g_vMtrlAmbient * attenuation * AO`로 결합되어 있다. G1 region47의 directional diffuse가0이므로 ambient RGB(.203921571,.225490198,.235294119)의 실제 기여도0이다. native21/26 직접광도 같은 diffuse0을 받고, 몸체에는 재질의 hdr07_1 IBL·발광과 다른 point/spot 입력이 남는다. 단순히 legacy specular를 올려도 native 직접광 입력을 복구하지 못한다.

원본 PS package의 export46이 가리키는 environment export48을 다시 파싱하여 `wle_override=true`, `WLE_CharacterLitIndirectBrightness=2.5`, `WLE_CharacterShadowedIndirectBrightness=2.5`를 확인했다. 09-19에 이 계수를 uniform map ambient에서 제거했지만 캐릭터 전용 간접광/SH 소비자는 아직 연결되지 않았다. 따라서 과광 교정 후 기존 캐릭터 조명 부족이 더 드러날 수 있는 코드 경로가 확인됐다. G2 camera3 전용 light override는 G1 전투에서 조건이 false이므로 이 현상의 원인이 아니다.

수정 방향은 **G1의 독립 캐릭터 간접광 입력**이다. 기본값0의 입력을 profile/region에서 native character pass까지 전달하고 albedo에 한 번 적용해야 한다. 전역 exposure를 되돌리거나 모든 native 재질의 ambient 수식을 바꾸면 정상 맵·다른 관문까지 영향을 받는다. world baseline×원본 계수2.5의 RGB(.5098,.5637,.5882)는 검토 후보일 뿐이며 PROJECT_TUNED uniform 근사다. 원본 SH/probe 복원값으로 부르지 않는다. 구현 시 Light descriptor, 단일/배치 GPU 전달, profile parse·blend·save·publisher의 소비 계약과 맵 비영향을 함께 확인해야 한다.

이번 후속 요청은 진단으로 마무리했다. **위 간접광 수정은 아직 구현·게시하지 않았고 새 화면 확인도 받지 않았다.** Engine/Client Deferred HLSL SHA256 일치, 기존 Product의 SourceGroup017 컴파일 로그와 배포 CSO hash 일치를 확인했다. C++/HLSL/JSON을 바꾸지 않아 새 빌드는 실행하지 않았다. 사용자는 스킬·이펙트 누락/끊김 조사를 보류하고 개별 패턴을 직접 확인하기로 했다. Release의 추가 선행 준비는 Debug의 첫 재생 지연을 줄일 수 있으나 누락이나 끊김 해결을 보장하지 않는다.

## G07. 후속 승인: 독립 캐릭터 간접광·Mario fog·G3 노출 회귀

G06의 진단 이후 사용자가 캐릭터 간접광과 효과 색상 회귀까지 수정을 요청했다. Mario 전체가 뿌연 `ea594eb1...png`, 고채도 원형 효과의 `be46d1d2...png`를 직접 확인했다. 두 번째 사진의 정확한 패턴명은 아직 확정되지 않았으며, 사용자가 별도로 지목한 무지개댄스 P38의 실제 소비자를 조사했다. 아래는 새 소스 구현·후보 검증 상태이고, 이전의 사용자 화면 확인이나 Product 배포 완료를 의미하지 않는다.

### 독립 간접광 입력

`LIGHT_DESC::vSourceCharacterAmbient`를 기본0으로 추가했다. RGB는 유한한0~64, w는0이며 nonzero는 directional scene 입력만 허용한다. `CLight::Render_Desc`와 `CLight_Manager` 배치 record 모두 GPU까지 같은 값을 전달한다. LIGHT_DESC는124byte, shader light record는128byte다. Engine과 Client를 함께 다시 빌드해야 한다.

`Resolve_SourceCharacterLight`의 기존 native 직접광·IBL·ambient 식은 유지하고 독립 항만 더한다. source map program80~83은 baked 여부와 관계없이 새 항을 받지 않는다. 일반 map/effect pass도 새 항을 읽지 않는다. native geometry의 MRT3 albedo는 Combined에서 한 번 곱해지고, native IBL/emissive RT4는 기존 별도 가산을 유지한다. 기본0인 다른 profile의 기존 결과는 변경하지 않는다.

RenderingProfileService의 profile light와 camera region에 optional `sourceCharacterAmbient`를 연결했다. parse/validate/save, region 우선값·시간 보간·이탈 복귀, publisher가 같은 계약을 사용한다. region의 명시적0은 profile 값 상속과 구별하여 저장한다. 후보는 G1 region47에만 RGB(104/255×.5×2.5,115/255×.5×2.5,120/255×.5×2.5,0)를 넣는다. base/book-open/popup/source-rendering 네 profile의 같은 G1 영역이 대상이다. 이는 원본의 character indirect 계수2.5를 사용하는 **PROJECT_TUNED uniform adapter**이며 원본 SH/probe 복원이 아니다.

### Mario와 품질 상속

Mario1~4의 로컬 Server HUD 상태는 원래부터 `scene.kakulsaydon.source-rendering.v1`의 own quality exposure1을 소비한다. 따라서 G1 alias의 raw base exposure2 상속이 Mario 노출을2로 만든다는 설명은 틀리다. MainApp 기존 presentation fog override에 `Is_LocalMarioStageActive()`를 추가했다. 단계1~4에 있는 로컬 플레이어만 fog를 끄며, stage0 복귀·실패·reset은 기존 restore→region evaluate 경로로 fog를 돌린다. Mario의 bloom·tone·LUT·노출은 임의 변경하지 않았다. source fog 최대 opacity3~6%가 뿌연 화면 전체의 유일한 원인이라고 확정하지 않는다.

`Get_ProfileQuality`는 Level의 raw quality baseline을 반환하고 활성 profile multiplier를 한 번 적용하는 현재 계약을 유지했다. 여기에 base multiplier까지 미리 적용하면 alias에서 중복 배율이 생긴다. 현재 저장본 revision68 기준 실효 exposure는 G1=1, G2=1, Mario=1, G3=2다. 새 후보는 G3 dark의 multiplier1→.5 한 필드로 G3=1을 만든다. 이는 기존 source comparison exposure1과 맞추는 **G3 전체 look 정규화 튜닝**으로, G3의 맵·모델·모든 HDR 효과에 영향을 준다. 원본 카메라 exposure 전체 복원으로 부르지 않는다. G1 base의 사용자 bloomMultiplier0과 alias1은 유지했다.

### 무지개댄스와 색 변환

P38의 현재 EFFECT 소비자는 `effect.kouku.gate3.rainbow.grid.full.restore` 두 occurrence다. 이 문서는342행(340particle+2light), native2400~2409, occurrence brightnessMultiplier1, 문서 bloomIntensity1.3이다. 이름이 비슷한 dance.full.root는 이 P38의 소비자가 아니다. 원본 HDR/material/particle 색과 문서 bloom1.3은 변경하지 않았다.

`Write_SceneBloom`은 명시적인 effect intensity가 있으면 scene intensity 대신 선택한다. 따라서 P38 자체 RT2 bloom에 scene multiplier0↔1을 다시 곱하지 않는다. 장면 threshold/knee와 최종 bloomEnabled는 여전히 공통 입력이다. 현재 source tone 경로는 HDR+bloom→exposure→source tone→graded LUT를 한 번 통과하고 즉시 반환하므로, 그 뒤의 Hable/gamma/legacy scene-desaturation 분기가 중복 실행된다는 증거는 없다.

현재 shader/CPU pack과 실제 LUT01 DDS를 사용한 통제 HDR 수치 대조에서 (4,.2,4)의 최종RGB는 exposure2=(.9028,.8220,.9841), exposure1=(.7640,.7207,.9637)이다. 채널 범위가.1621→.2430으로 커진다. 반면 (8,0,0)은 양쪽 tone이 포화되어 같은 결과다. 노출 정규화는 일부 높은 채도의 차이를 회복하지만 모든 포화색·원형 효과 사진의 가장자리까지 고친다는 증거는 아니다. `g3-source-color-sweep.json`은 수식 대조이며 GPU 화면 캡처가 아니다.

### LUT 경로와 검증 경계

사용자가 LUT 두 DDS를 `Map/Lighting/KoukuSaydon`으로 직접 이동했다. 이동된 파일은 09-19 원본 candidate와 바이트 동일하다(01 SHA256 `572a4f5d43d3387a2963b01bbbfbb2c1821e7ce09fc2cc668f67e3248f5a19a7`, 02 `2fae2ecf4871c05caf14128c62a9860295fc7be0db4beA4284438df6332d8f33`). 리소스 재생성·복사·덮어쓰기와 새 Kouku 폴더 생성은 하지 않았다. 후보의 LUT9필드와 이전 생성 스크립트의 출력 경로만 KoukuSaydon으로 바꿨다.

- `out/RaidRegression20260920/rendering/stable-field-patch.json`: revision68 저장본 hash에 기반한14필드. 간접광4, G3 노출1, LUT 경로9. 최종 설치는 root의 최신 디스크 stable-field merge와 정상 publisher가 담당한다. 이 절은 후보 검증 시점 기록이다.
- 실제 C++ parse/validate/serialize/quality 함수와 UserSettings video 함수를 추출한 headless 실행: 네 G1/source profile exposure1, roundtrip, 잘못된 RGB/범위/w 입력3개 거절 및 이전 catalog 보존 PASS. renderer는 실행하지 않았다.
- sourceCharacterAmbient publisher roundtrip, profile/region invalid4개에서 runtime 보존 테스트 PASS. 최종 LUT 이동 후 candidate C++ 검사와 정상 publisher→out runtimecandidate 재검사 PASS.
- Engine Light/Light_Manager, Client RenderingProfileService/Level_KakulSaydonArena/MainApp, PointLightFalloffContractHarness의6TU `/Zs` PASS. Deferred 기본 및 SourceGroup017/025 FXC `/O1` PASS. 기존 native 함수의 potentially-uninitialized warning은 유지된다.
- 신규 Light 경계 검사는 기존 PointLightFalloffContractHarness에 추가하고 컴파일했다. 전체 Engine ABI 재빌드가 필요한 해당 실행 파일을 오래된 DLL로 실행하지 않았다. 실행 PASS로 기록하지 않는다.
- Engine/Client Deferred 소스 동일, 변경 파일 `git diff --check` PASS. 새 C++ 파일·프로젝트 등록은 없다. Client/UI 및 Product 빌드는 실행하지 않았다. G1 암부·Mario1~4·G3 불과 무지개댄스의 최종 화면 비교는 사용자 검증이 필요하다.


### G07 후속 통합 반영

root가 최종14필드를 최신저장본에병합하고 RenderingProfiles 정상 Publish를 완료했다. authoring/runtime모두 `Map/Lighting/Kouku/` 참조0건이며 사용자가이전한 KoukuSaydon DDS를소비한다. source revision69. 관련추가10파일 설치와 최종컴파일검사 결과는 RAID_PRESENTATION_REPAIR_IMPLEMENTATION_RESULT G09에 기록한다. 전체 Product빌드·화면은사용자진행으로 분리한다.

## G08. 관문 기본 방향광 복구와 baked 맵 수신 경계

사용자는 노출2 당시 모델 표현을 선호하되 LUT/노출 과광 수정은 유지하고, 후속 답변에서 directional 복구를 선택했다. 최신 RenderingProfiles69의 base/G1 book-open/G1 popup/G3 dark/source-rendering 다섯 profile에서 기본 diffuse와 specular를 before-restoration의 RGB(.8,.8,.8)/(.5,.5,.5)로 복구했다. 실제 G1/G3/start 관문 영역47/48/52/53/54의 입력도 함께 바꿨다. 기존 노출 배율, LUT, bloom, map ambient 및 G1 독립 character ambient는 유지했다. 의도된 blackout/spotlight와 비교용 profile은 바꾸지 않았다.

profile.light.receiver는 기존 Engine LIGHT_RECEIVER를 사용하는 optional ALL/SOURCE_CHARACTER/UNBAKED다. 기본ALL이며 region.receiver는 생략 시 profile상속, 명시ALL을 저장해 유지한다. 복구 기본광은 UNBAKED로 적용해 RNM/native baked pixel의 중복 직접광을 차단한다. base/source의 기존 다른12영역에는 ALL을 명시하여 카드미로와 Mario의 원래 수광 경계를 보존했다. MainApp의 G2 camera3 전용 character-only override를 제거해 시퀀스와 전투가 같은 profile/region 경로를 소비한다. shader와 LUT DDS는 수정하지 않았다.

이 후보는 G1 캐릭터 직접광을 복구하고 기존 독립 간접광을 보존한다. directional×ambient 항이 다시 켜지므로 uniform ambient 총량은 약1.32배다. 과거 exposure2처럼 IBL·발광·local light 전체를2배 하는 것과 같지 않으며 이전 모델 픽셀의 정확한 복원으로 기록하지 않는다. 원본 DDL receiver exclusion 완성 대신 사용자가 요청한 프로젝트 조명 복구다. 최종 선호도·바닥 연결은 사용자 화면 확인 대상이다.

53필드를 최신 저장본에서 stable profile/region ID로 병합하고 hash 재확인·backup·원자 교체했다. Rendering 정상 Publish 완료, source/runtime revision70 및 JSON 동일. 증거는 out/KoukuPlaybackRepair20260920/rendering-install.receipt.json과 rendering-publish.log다. 현재 Client/Server 프로세스가 없는 상태에서 적용했으며 미저장 메모리 Reload나 Client/UI 실행은 수행하지 않았다.

RenderingProfileService 전체 TU 컴파일, 실제 C++ codec의 세 receiver roundtrip/생략상속/명시ALL/invalid rollback 및 정상 publisher profile7+region8개 검사를 통과했다. 같은 후보에서 카드미로55와 Mario49/50/59의 ALL 유지, 노출1, sourceCharacterAmbient와 무관한 profile 보존을 확인했다. 검사 증거는 out/KoukuPlaybackRepair20260920/receiver에 있다. 통합 Product 결과는 RAID_PRESENTATION_REPAIR_IMPLEMENTATION_RESULT G12에 별도로 기록한다.

## G09. Mario FXAA와 사용자 비교 조작

로컬 Mario 참가자만 활성화하는 `scene.kakulsaydon.source-rendering.v1.qualityOverride.fxaaEnabled`를 false로 바꾸고 Rendering revision70→71을 정식 게시했다. 다른 노출/LUT/Bloom/region 필드는 그대로임을 backup과 대조했다. receipt는 `out/KoukuPlaybackRepair20260920/mario-fxaa/rendering-install.receipt.json`이다. 빛번짐의 원인을 FXAA로 확정한 것은 아니며 사용자 비교를 위한 설정 변경이다.

추가 사용자 요청에 따라 Recovered map materials 위에 방향광, Exposure0.5/1/2배, LUT grading, FXAA, Bloom의 실시간 비교와 Reset을 추가했다. 실제 동작은 노출 배율이며 LUT는 한 번 평가한다. 기존 renderer의 품질 적용 경로와 presentation 환경 복원을 확장하고 Save 대상 catalog와 분리했다. region/Mario override 뒤 적용하고 다음 프레임 이전 baseline으로 돌아가므로 exposure가 누적되지 않는다. 닫기와 Level 전환은 비교 상태를 해제한다.

실제 `Set_ComparisonOptions`, `Restore_PresentationEnvironment`, `Apply_CameraEnvironment` 코드를 renderer adapter stub에 연결한 native검사4025건 PASS. 1000프레임2배 고정, 세 preset, LUT/FXAA/Bloom 및 diffuse/specular만의 전환, ambient 보존, reset, 새region으로 이동, 적용/복원 실패의 기존 상태 보존을 확인했다. `out/KoukuPlaybackRepair20260920/comparison/{probe.cpp,probe.log}`. 이는 CPU 제어 검증이며 Client/UI 또는 GPU 화면 검증은 아니다. 최종 Product 빌드 기록은 통합 RESULT에 둔다.
