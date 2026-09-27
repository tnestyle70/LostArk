# World Movie 머리카락·가디언 눈 재질 구현 결과

## G00. 완료한 변경

사용자 정정은 무도가 여/창술사 Movie의 **머리카락**이다. 허리·몸체는 변경하지 않았다.
원본 `bg_pcselect02.mat_ft.pc_ft_06_hair_mi_high`의 native600 Base/Light에서
material uniform 소유 범위 밖 primitive prefix가0으로 남아 최종 opacity가0이 되는 경로를
확인했다. Base source0.x(environment 배율)·source1.w(opacity), Light source1.w만
identity로 연결했다. 동일 Client/Engine shader mirror와 원본 shader ID를 확인하는
`build_vehicle_source_material.py` 생성 규칙을 함께 수정했다.

원본 Base ID는040a63ec2e3e5e42a8f2194c6622723a, Light ID는
e36b84e020d38e408af80c720418b668이다. 생성기는 두 ID의 stage와
leadingUnownedConstantBuffer0Slots=[0,1]을 검증한다. 다른 native material이나
전역 조명·exposure·bloom·AA 값을 수정하지 않았다.

## G01. 실행한 검증

- native600 `verify`: Base EXACT490lines, Light EXACT557lines, Configure EXACT.
- Product Debug는 상위 통합 빌드에서 성공했다. receipt:
  `out/BuildPipeline/runs/20260926T204439420Z-debug-product.json`.
- 현재 Product Debug 객체/DLL/CSO의 독립 복사본으로 기존
  `out/WorldMovieEffectEditor20260926/build_probe.py --run`을 재실행했다.
  compile/link/run0, 실제5category admission,460World instance resource preparation,
  Movie Effect 준비와 실제 Open Movie/Play All/Solo/Edit·sandbox Save/Reload 계약이 통과했다.
  저장 검증은 격리 sandbox에서만 수행했으며 사용자 저작 문서를 저장·Reload하지 않았다.
- 새 C++ 제품 파일·project 등록은 없다. shader 입력 변경은 상위 Debug/Release 빌드가 소비한다.

## G02. 가디언 눈 최종 반영과 근거

일반 GuardianKnight.wmodel의 eye는210정점·WINT1.3·UV1/UV2이며 Movie eye는
380정점·WINT1.3·UV1/UV2다. 일반 Guardian eye와 GunSlinger eye는 position/normal/
UV0/tangent210정점 및 UV1/UV2가 수치상 동일하고 같은 native5·같은 MIC 값·6texture를
사용한다. 같은 head-relative view의 실제 CModel 팔레트/face depth/단위 직접광에서
두 클래스 모두376노출 eye pixel, Base dark22·직접광 dark19로 같았다. 다른 캐릭터의
shader로 교체하는 것은 이미 동일한 경로를 다시 지정하는 일이므로 수행하지 않았다.

사용자가 확인한 흰색 증상은 실제 Movie20초 camera/pose와 WorldSequenceObject::Render,
원래7Movie point light, 현재 scene directional을 사용해 조명 전후로 분리했다. SL00
map light4개는 이 camera에서 frustum 밖이라0개 제출되었다. warm-high-key의 diffuse는
(0.715,0.78,0.98), ambient는(0.28,0.27,0.26), map multiplier1이다. customizing-dark의
(1.3,1.3,1.35)/(0.53,0.53,0.55)도 별도로 대조했다. native5의 shadowfactor0.6은 직접광마다
최소0.4의 diffuse/shadow floor를 남긴다. 이 fixture에서 tdspecular만0으로 낮추는 것은
가장 밝은 pixel 수를 거의 바꾸지 않았고 shadowfactor1이 누적 밝기를 크게 낮췄다.

최종 사용자 요청에 따라 **가디언 일반·Movie eye 두 재질만** shadowfactor1과
**tdspecular_intensity0.25**로 조정했다. 이는 retail 원본0.6/2에 대한 프로젝트 튜닝이다.
shader ABI 오류를 복원했다고 주장하지 않는다. iris RGB·alpha·size·texture·다른 class와
전역 조명·AA·bloom·exposure는 바꾸지 않았다.

| 실제 Movie20s 노출 eye90pixel | 원본0.6/2 | 최종1/0.25 |
|---|---:|---:|
| 합성 HDR 평균 | 0.922698 | 0.545245 |
| source tone 뒤 밝은 pixel(평균RGB>0.9) | 14 | 2 |
| 원본 albedo로 분리한 pupil5pixel의 표시 평균 | 0.284703 | 0.199901 |
| non-finite | 0 | 0 |

source tone 수치는 현재 warm-high-key의 scale1/range8/toe0.5/exposure1과
Shader_Deferred.hlsl의 Tonemap_SourceCustomizable식을 그대로 계산했다. bloom은 현재
warm-high-key에서OFF다. 이 수치는 전체 scene의 FXAA·모든 주변 효과·사용자 최종 화면
판정을 대체하지 않는다. 어두운 pixel의 iris-alpha0 negative control에서는 pupil이0으로
사라져 눈꺼풀 윤곽이나 AO를 pupil로 세지 않았음을 확인했다.

반영 경로:
- Data/Actors/CharacterCatalog.json: GUARDIANKNIGHT/GuardianKnight.wmodel/pc_dl_eye_mi.
- Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.worldsequences.json:
  world.object.classselect.guardianknight.a12265.p2/pc_dk_eye_00_mi.
- Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00
  -Scope WorldSequences -Mode Publish 성공. Runtime 문서 SHA256은
  12ddf5c2df418e5bad7b4ee98236bd137f179b8458716d0f6064918ff5bda9b2.

최신 저장본에서 stable row2개·field4개만 수정하고 나머지 parse tree 동일성을 검사했다.
교체 직전 SHA 확인·백업·원자 교체와 실패 시 자기 변경 rollback을 사용했다. 영수증은
out/WorldMovieHairEyes20260927/eye-tune-receipt.json이다. CharacterCatalog의 별도 avatar
추가 작업은 이후 최신 문서를 읽어 병합하도록 조율했다.

설치 후 실제 CActorCatalog/CWorldSequenceDocument→CModel 재로드에서 일반·Movie 모두
base/light packed shadowfactor1·complement0·tdspec0.25를 소비함을 확인했다.
head-relative native draw도 compile/link/run0, non-finite0이다. 일반 eye는376pixel,
Movie는277pixel이며 최종 native 직접광 dark pixel은 각각23/21이다.
install_guardian_selection.py는 정확한 Guardian Movie eye의 현재 저장된 두 lighting
field를 donor 재설치 때 유지한다. 기존 candidate 입력을 변경하지 않는 것과 다른 row의
통상 교체 동작을 함께 검증했다. 일반 body row를 자동 재생성하는 별도 제품 도구는 없다.

## G03. 검증 산출물·리소스 경계

- out/WorldMovieHairEyes20260927/fullhead-gunslinger-baseline.log: 같은 native5/geometry 대조.
- out/WorldMovieHairEyes20260927/scene-light-iris-controls.log: iris-alpha negative control.
- out/WorldMovieHairEyes20260927/scene-light-tdspec-controls.log: 반사 단독 억제 대조.
- out/WorldMovieHairEyes20260927/scene-light-shadowfactor-controls.log: 두 scene profile 대조.
- out/WorldMovieHairEyes20260927/scene-light-final-combo.log: 최종 조합 native radiance.
- out/WorldMovieHairEyes20260927/run.log: 설치 데이터의 실제 CModel 소비 검증.
- out/WorldMovieHairEyes20260927/movie-eye-final-combo-diagnostic.png: no-window GPU readback의
  eye region 대조. 주변 머리는 위치 식별용 unlit albedo이며 완성된 scene 화면이 아니다.

이 슬라이스는 shader/생성기·JSON 조정이며 새 Resources 파일이 없다. 기존 Resources를
그대로 사용하므로 GBResources에 추가할 바이너리 리소스도 없다. 다른 슬라이스의 신규
avatar Resources 전달은 해당 담당이 별도로 검증한다. Client/UI를 실행하거나 조작하지
않았다. 머리카락 표시와 일반·Movie 눈의 최종 화면은 사용자 확인이 남는다. 수치 개선을
사용자 시각 PASS로 기록하지 않는다.

변경 JSON parse, installer 재적용 회귀, scoped git diff --check 성공. 제품 Debug/Release
최종 통합 빌드는 상위 작업의 영수증을 따른다. 빌드·하네스 산출물은 소스 커밋 대상이 아니다.

## G04. 창술사 기본 FT43 헤어 외형 개선

사용자가 창술사는 원인 확정과 관계없이 수정하도록 요청했다. 일반 FT00은 재질 override가
없지만 Movie FT06-high에서는 7,492정점과 실제 10시점 pose·본 스케일이 유지됐다. 이번 FT43
선택은 원본 결함 복원으로 설명하지 않는 사용자 요청 외형 개선이다.

일반 기본 장착은 `CharacterCatalog` LanceMaster의 equipmentModels[5]를 기존 FT43 asset으로
교체했고, native170 override 한 행을 global lazy 목록에서 해당 character 소유 목록으로 옮겼다.
커스터마이징은 optional defaultVisualSetId로 기존 index32를 초기·Reset 기본값으로 지정한다.
목록 순서와 preset 숫자는 변경하지 않아 명시적인 hair0은 계속 FT00이다. View에서 클래스별
직접 선택도 보존한다. FT00의 재질을 다른 native ID로 추측하여 새로 연결하지 않았다.

실제 CCustomizingCostumeDocument·CActorCatalog·CModel 검사는21개 PASS다. stable default의
정확한 index resolve, legacy0, invalid default 실패 후 이전 document 보존, 일반/preview의
native170 소비, body→hair223본 이름 pose와 finite bounds를 확인했다. 현재 함수 본문의
선택 보존 검사는9개 PASS이며 변경4TU 최소 컴파일도 통과했다. 증거는
`out/LanceDefaultHair20260927/probe-run.log`, `selection-check.cpp`와 각 compile log다.

최신 두 JSON의 SHA를 다시 확인한 뒤 백업·원자 교체했고, 검증한 후보의 SHA와 최종 파일이
일치한다. 무관한 row와 사용자 preset 파일은 변경하지 않았다. 설치 영수증은
`out/LanceDefaultHair20260927/install-receipt.json`이다. 기존 FT43 Resources를 재사용하므로
이 일반 기본값 변경에는 새 리소스 전달이 없다. Movie 헤어의 별도 골격 변경은 아래에서 구분한다.

## G05. 창술사 Movie FT43 파생 헤어와 설치

`Tools/CharacterSelectPipeline/derive_lance_movie_hair.py`는 일반 FT43의 5,865정점과 양의
weight 13,369개를 보존하고 Movie 좌표계로 변환한다. 기존 Movie FT06의 208본과 Intro/Loop
키·시간을 유지하며 FT43 전용 체인 4본을 추가한 212본 WModel을 생성한다. 누락된 weighted
본을 버리거나 모든 정점을 head 한 본에 붙이지 않는다. 일반→Movie 변환은 `(x,-z,-y)`이며
inverse bind·normal·tangent·winding도 함께 변환한다. 추가 체인은 원래 rest offset으로
animated neck을 따라간다. 새로운 물리 hair simulation을 구현한 것은 아니다.

새 asset ID는 `Character/LanceMaster/Cinematics/ClassSelect/Appearance/FT43_Hair.wmodel`이다.
원래 Movie 모델과 일반 FT43은 보존했다. 실제 CWorldSequenceDocument→CWorldSequencePlayer의
Set_Document/Prepare_InstanceResources/Play/Seek를 거쳐 native170과 212본을 소비했다.
Intro/Loop 10시점의 원래 208본 combined matrix 오차는 0이며, 창 없는 D3D11 WARP의 실제
CWorldSequenceObject BLEND draw에서 7,951~8,490 pixel을 확인했다. 독립 bind/rest 검사에서
weighted 정점 오차는 preScale 적용 후 최대 7.021μm였다. 전체 Movie 화면의 최종 외형 판정은
사용자 확인으로 남긴다.

최신 authoring의 `world.object.classselect.lancemaster.a12230.p0`에서 modelAssetId,
materialSourceModelAssetId와 이전 FT06 materialProfile만 교체했다. stable ID·기존 clip·clock·
다른 객체의 JSON 의미는 동일하다. 교체 직전 hash 확인과 백업·원자 교체 후 WorldSequences
범위 Validate/Publish/Check가 모두 PASS다. authoring/runtime SHA256은
`c8a4f19bac94466d3e1769319207efb48199e7e0c6ca32bfdf07dad4a78baf99`로 일치했다.

Runtime Resources와 Desktop/GBResources의 파생 WModel SHA256은
`762896641510c2bc8ece4f928602308300f0d6e896fac9965fb68b4c1c41bd71`로 동일하다.
GBResources에 없던 일반 FT43 모델 및 texture 4개도 같은 상대 ID로 전달했다. 기존에 존재하는
다른 bytes를 덮어쓰지 않았으며 statefx_default는 동일 hash를 확인했다. 증거는
`out/LanceHair20260928/FT43_Hair.receipt.json`, `movie-consumer-final.receipt.json`,
`resource-install-receipt.json`, `world-install-receipt.json`이다.

## G06. 최종 통합 검증과 남은 화면 확인

일반·커스터마이징 변경 4TU와 실제 reader/catalog/model 21개 및 선택 보존 9개 검사를 통과했다.
동시 Guide AI 작업의 오류로 중간 Product/Client 빌드는 실패했으나 해당 작업에서 수정한 뒤
최신 소스를 포함한 전체 Debug Product가 PASS다. 최종 근거는
`out/BuildPipeline/runs/20260927T034610791Z-debug-product.json`이며 Engine/Shared/Server/Client
네 프로젝트가 모두 성공했다. 이 작업의 최종 Release 빌드는 수행하지 않았다.

가디언 나이트 Movie 회전·위치·물방울은 원본 root/TypeData와 실제 particle draw matrix 및
원본 shader map까지 대조했다. 잘못된 변환이나 누락된 distortion pass를 확인하지 못했으므로
추측 보정은 적용하지 않았다. 미연결 WORLD crack과 사용자가 말한 유리를 같은 대상으로
단정하지 않는다. 근거와 수치 검증의 한계는 09-22 Guardian cinematic RESULT G17을 따른다.

Client/UI를 실행·조작하지 않았다. 현재 Debug 실행 파일과 데이터는 설치됐으며 실제 Movie,
커스터마이징 헤어의 선호 외형 및 특정 시점 유리 표시 여부는 사용자의 화면 확인이 남는다.
