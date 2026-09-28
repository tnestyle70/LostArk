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

## G07. Movie FT43 tangent basis 재검증 (2026-09-29)

이 절과 G08은 이전 G05 이후의 현재 후보 상태를 기록한다. 그 사이 FT43은
`2026-09-27_KOUKU_AUDIO_MARIO_HUD_POLISH_IMPLEMENTATION_RESULT.md` G05에서 head-rigid
정책으로 바뀌었다. 이번 기준 설치본 SHA는 `57f4f9cd16b90a579f9ee3e0c64f9702ba6f1a649987dc8f06b65aba316d95c8`다.
G05의 neck-chain 정책과 `762896...` SHA를 현재 상태로 읽으면 안 된다.

일반 FT43과 설치 Movie 파생본은 76-byte legacy 정점이었다. 생성기의 `(x,-z,-y)`는
determinant -1인데 implicit tangent sign을 +1로 남겨 실제 `CWMeshReader`가 복원하는
binormal이 정상 donor의 좌표 변환 결과와 반대였다. 먼저 donor의 implicit +1을 명시
sign으로 보존하고 반사 시 -1로 변환하도록 생성기를 수정했다. WINT1.6을 이미 읽는 제품
reader와 동일하게 오프라인 pose reader도 80-byte 정점을 받아들이도록 확장했다.

실제 제품 `CWMeshReader.cpp`를 컴파일하여 5,865정점을 비교했다. 이전 binormal의 기대
방향 dot은 -1, 후보는 +1이고 최대 성분 오차는 `1.19209e-07`이다. 기존 76-byte 정점의
position/normal/tangent/UV/weights/indices와 index·bone·material·skeleton·clip·extra UV
payload는 동일하다. 이 결함은 normal-map 입력 방향 문제이며 사용자가 지적한 머리
실루엣의 원인이라는 증거는 아니다. 이것만으로 외형 수정을 완료했다고 판단하지 않았다.

증거는 `out/LanceMovieHairBasis20260929/reader-run.log`, `candidate-verification.json` 및
`preservation-receipt.json`이다. Client/UI는 실행하지 않았다.

## G08. 정상 머리 모델과 Movie 복장 교체 후보 (2026-09-29)

사용자가 정상 커스터마이징 모델 교체를 선택하여 `derive_lance_movie_head.py`를 추가했다.
일반 `Character/LanceMaster/LanceMaster.wmodel`의 face material3(1,571정점), eyelashes4
(362정점), eyes5(157+108정점의 두 submesh)를 기존 Movie의 `a12205.p1/p2/p0`에 연결한다.
두 번째 eye-material head shell도 포함한다. 일반 모델의 face00 geometry·UV·catalog 재질과
기본 FT43 머리를 사용하며 의상 `a12241.*`, actor transform·scale·camera·clock은 보존한다.
사용자의 실행 중 morph·색상·preset 메모리를 캡처한 것은 아니며 현재 디스크 기본 모델이 기준이다.

세 파트의 모든 weighted bone은 Movie208본에 이름으로 대응한다. 최초 조사 후보는 Movie
mesh의 inverse bind를 재사용했으나, Movie 원본에서 사용하지 않던 facial 본의 offset이
정상 head용으로 유효하지 않아 실제 전정점 skin 검사에서 최대120.122cm 변형을 검출했다.
그 후보는 설치하지 않았다. 최종 후보는 weighted donor inverse bind를 같은 좌표계로
변환하여 이름별 이식한다. 생성기에 전정점 neutral pose 오차0.01cm 미만 검사를 추가했다.
Movie skeleton 및 Intro/Loop 섹션 bytes는 원본과 동일하며 애니메이션을 새로 만들지 않았다.

각 WModel은 기존 `classselect_intro` 1205.010009765625ticks와 `classselect_loop`
1161.989990234375ticks를 30ticks/s로 포함한다. FT43은 기존 212본(208본 prefix+추가4본)과
208채널 원본키, head-rigid 정책을 유지한다. 머리카락의 별도 secondary motion은 없다.
새 Resources는 앞3개이며 기존 Appearance/FT43_Hair 한 파일은 tangent sign 수정본으로 교체한다.
원본 일반·Movie 모델과 기존 texture를 보존한다.

최종 후보 폴더는 `out/LanceMovieHeadReplacementFinal20260929`다. Resources 아래 공통 경로
`Character/LanceMaster/Cinematics/ClassSelect/Appearance/`의 결과는 다음과 같다.

| 파일 | SHA256 |
|---|---|
| DefaultFace.wmodel | 1bbafc2d5c1570637d7317d87e57eb9e3f0bf6678c83a50fa107afa3796cd1cd |
| DefaultEyelashes.wmodel | 56e3a0aa124d34b916d81ffe30c1e2006ae8d4a24ff8806ac8bbca62200883fe |
| DefaultEyes.wmodel | 46cc13b155221f69a6d6ba898c88c759b5c013ecd6cd2d533d863300155238a3 |
| FT43_Hair.wmodel | b390088076c0287892c980621a09beca1ad574f9fad95fb48bcb1fea46c63cc1 |

`head-replacement-receipt.json`은 source/donor hash, stable object3행의 필드 patch를 기록하고,
`install-baseline-receipt.json`은 최신 디스크 baseline과 후보 hash를 기록한다. 생성한
WorldSequences 후보는 modelAssetId·materialSourceModelAssetId 교체와 기존 materialProfile
제거만 반영한다. templates/instances와 다른 객체는 동일하다. 원본 최신 baseline은
`c8a4f19bac94466d3e1769319207efb48199e7e0c6ca32bfdf07dad4a78baf99`이며 최종 설치 직전 재검사한다.

일반 속눈썹 native6은 World에서 forward9 연결이 빠져 있었으므로 기존 Character/
Part_Equipment와 동일한6/7/99 분기를 `WorldSequenceObject.cpp`에 연결했다. shader와
전역 rendering option은 변경하지 않았다. 현재 Movie authored/runtime exposure·tone scale은1이고
LUT는 한 번 적용됨을 상위 작업에서 확인했다. 'LUT2배' 추정의 근거가 없어 LUT를 조정하지 않았다.
Movie의 face02 재질은 일반 face00 catalog 재질로 교체하며 반짝임에 대한 최종 평가는 화면 확인으로 남긴다.

검증 완료 범위는 다음과 같다.

- 실제 `CWModelDecoder`로 최종4개 파일 decode PASS. face/eyelash/eye208본, hair212본,
  2개 clip과208채널, material slot·정점 count를 확인했다. `native-decoder.log`에 기록했다.
- donor raw geometry 좌표 반사오차0, weights/UV/weighted bone names 보존. 세 파트의 neutral
  skin 최대 오차는 각각0.001215293/0.001300970/0.001406202cm다. 기존 modelPreScale0.01을 유지한다.
- 모든 clip key와 구간 중점 Intro387/Loop3 시점에서 head-relative donor retarget와 후보의
  정점 오차0, finite PASS. `head-geometry-verification.json`에 기록했다.
- 독립 검토자가 인메모리30fps sweep으로 Intro1,207+Loop1,163=2,370시점을 검사했다.
  전체 정점 finite, face head-space extent 약14.309~14.831×10.6318×12.2742cm다.
  최대2.125851cm 이동은 neck-twist 가중 정점의 기존 Movie 동작이다. 120cm 찢김이나 전체
  배율 변경은 없다. 7개 catalog material 이름이 각1슬롯, stored texture25개와 native texture
  closure 존재, UV1/2와 index range 및 최종 후보 bytes 일치도 별도 확인했다.
- 실제 pass selector 함수와 HEAD를 비교한0..1100 프로그램 회귀 검사에서6/7/99만 변경되고
  기존600/170/18/84/88과 null/legacy가 보존됐다. `forward-pass.log`가 근거다.
- 변경 Python3개 py_compile, scoped diff 검사 PASS. `WorldSequenceObject.cpp` Debug 실제 TU
  최소 컴파일은 상위 작업에서 PASS했다. 이 슬라이스는 새 C++ 파일·프로젝트 등록이 없다.

후보 재현은 저장소 루트에서 다음 명령을 사용한다. output은 아직 없는 새 경로여야 한다.

```powershell
python Tools/CharacterSelectPipeline/derive_lance_movie_head.py --resources C:/Users/user/Desktop/LostArk/Client/Bin/Resources --output out/LanceMovieHeadReplacementRebuild20260929
```

이 절 작성 시점은 후보 검증 완료이며 원본 Resources 설치·GBResources 전달·WorldSequences
publish는 상위 작업에서 최종 hash 재확인 후 수행한다. Client/UI 실행이나 GPU 완성 화면 검증은
수행하지 않았다. 사용자 Movie 재생에서 얼굴·머리 외형 확인이 남는다.

## G09. 교체 설치와 Product Debug 빌드

사용자의 교체 승인 후 최신 디스크 source·catalog·donor·target hash를 다시 확인했다.
기존 편집을 3-way로 보존하고 파일별 백업·교체 직전 hash 확인·원자 교체로 원래
Desktop/LostArk에 소스·데이터를 반영했다. 위 WModel4개는 Runtime Resources와
Desktop/GBResources에 같은 상대 경로와 SHA로 설치했다. 새 texture나 animation은 없다.

WorldSequences는 격리 overlay에서 Scope WorldSequences Validate/Publish를 통과하고 실제
설치본 Check도 통과했다. 이후 기존 one-line 형식을 유지한 compact source로 다시 Publish했다.
최종 source/runtime은 bytes가 같고 SHA는
`a2003e335915f04091202bf59e3056b6b3d8edf7cad2dce7bf98786c1d6b7afc`다.
preinstall 대비 a12205.p0/p1/p2 각각 modelAssetId·materialSourceModelAssetId·materialProfile의
세 필드만 달라지고 다른 모든 object·template·instance·배율·조명은 동일하다.
증거는 `out/LanceMovieHeadPublisher20260929/compact-receipt.json`, `compact-publish.log`와
`out/MovieVisibilityBalance20260929/original-install/installed.json`, `compact-installed.json`이다.

사용자가 Client/Server를 저장 후 종료했다고 회신한 뒤 잠금 해제와 파일 hash를 확인하고
정상 Product Debug를 빌드했다. Engine/Shared/Server/Client compile/link/deploy PASS다.
실행 증거는 원래 저장소 `out/BuildPipeline/runs/20260928T220610990Z-debug-product.json`이며
Client OBJ35개와 실행 파일이 갱신됐다. 기존 C4819/외부 PDB 경고는 남고 오류는 없다.
빌드는 데이터를 재게시하지 않았고 에이전트가 Client/Server를 실행하지 않았다.

사용자는 이후 반짝임이 Movie에 국한되지 않고 일반 캐릭터를 가까이 보아도 나타난다고 정정했다.
따라서 이 정상 머리 교체를 전체 피부·재질 광택 문제의 해결로 기록하지 않는다. 그 공통 재질
문제는 별도 조사 대상으로 남고, 이 절의 완료 범위는 머리 교체·게시·빌드다.

## G10. 기존 정상 재질의 반사 lookup mip 재사용 후보

사용자는 워로드 기본 의상의 기존 보정을 Movie에 재사용하도록 요청했다. 기존
`2026-09-08_CHARACTER_MATERIAL_MINIFICATION_RESULT.md`의 TGA full mip 복원과
현재 `CMaterial.cpp`의 `BuildRgbaMipChain`/TGA loader를 대조했다. 정상 기본 의상의
`hdr07_1.tga`와 `brdf_beckmann_spec.tga`는 512×512에서 10단계 mip를 생성하지만,
Movie가 참조하던 동일 lookup의 DDS는 header mip count 0, 실제 mip 1개다.

일반 피부·전체 갑옷의 모든 광택 원인을 확정한 것은 아니다. 이번에 확인한 결함은 아래
Movie 입력에서 기존 정상 경로의 반사 lookup mip가 빠진 것이다. 원본 native shader는
roughness에서 산출한 LOD로 반사 lookup을 `SampleLevel`하므로 한 단계 DDS는 항상
mip0을 샘플한다. 별도 검토자가 현재 Warlord face200·upper2·lower10 및 Guardian face200의
실제 값과 식을 계산했다. 1080p 내부 quad 기준 face 요청 LOD10은 정상 TGA의 유효 mip9와
DDS의 mip0으로 갈리고, armor 요청 LOD2~11은 정상 유효 mip2~9와 DDS mip0으로 갈린다.
증거는 `out/MovieReflectionMip20260929/lod-readonly.json`이다. 화면 개선 정도는 사용자가
판정한다. roughness/specular 강도를 바꾸거나 공통 shader에 광택 감쇠를 추가하지 않았다.

`prepare_movie_lookup_mips.py`는 최신 source SHA
`a2003e335915f04091202bf59e3056b6b3d8edf7cad2dce7bf98786c1d6b7afc`를 확인하고
out 후보만 작성했다. 다섯 class 32 object의 58개
`materialProfile.textures[expressionIndex].assetId`만 기존 정상 TGA 두 파일로 바꾼다.
원본 source·Resources·CharacterCatalog·보스·소환 동물·몬스터·prop은 쓰지 않았다.

| class | 대상 object 수 / texture field 수 | 실제 얼굴·피부 대상 |
|---|---:|---|
| GuardianKnight | 2 / 4 | a12265.p0 얼굴(9,11), a12253.p4 피부(4,6) |
| Artist | 15 / 30 | a12222.p0·a12240.p0·a12247.p0 얼굴(9,11), a12220.p2·a12243.p1·a12244.p2 피부(4,6); 나머지는 복장·붓 |
| DimensionMaster | 3 / 6 | a12269.p0 얼굴(9,11), a12267.p4·a754.p4 피부(4,6) |
| LanceMaster | 6 / 6 | a12241.p0~p3 복장과 a748.p0·a749.p0 무기의 BRDF(8)만; 새 정상 donor 얼굴은 이미 TGA를 사용하므로 제외 |
| Warlord | 6 / 12 | a12206.p5 얼굴(9,11), a12207.p1~p4 복장, a726.p0 무기 |

8개의 서로 다른 DDS 경로 각각을 기존 정상 TGA와 decode하여 512×512 RGBA 전체가
byte-exact equal임을 확인했다. hdr07_1은 기존과 같은 srgb, BRDF는 같은 linear다.
동일 mip0의 RGBA SHA는 hdr07_1
`3f04e367ddab93474f26c0314f1d2381aa848b99bcb1cc1f53dc569f376346b9`, BRDF
`d1cebef3f7bed8ba20b7741fe76b9065fc5e45338dd5474b1abd90eccb6d6a9f`다.
기존 공유 DDS를 덮어쓰지 않고 Movie의 참조 필드만 변경하므로 새 Resources는 없다.
정확한 58행 stable object/expression/전후 asset ID와 실제 파일 hash는
`out/MovieLookupMip20260929/lookup-mip-receipt.json`에 있다.

전체 JSON의 해당 58필드를 역변환하면 원본 JSON과 같다는 검사를 통과했다. 피부 normal·
specular·diffuse·화장·skin color 입력, sourceMaterial·family·모든 scalar, model·geometry·UV·
clip·camera·lighting·LUT 및 다른 object의 값과 순서는 보존했다. Guardian HR00 갑옷 native199는
128×128 8mip scene cube를 사용하고 두 2D lookup을 소비하지 않으므로 이번 수정 대상이 아니다.
창술사 머리 교체를 다른 class의 공통 광택 수정으로 확대하여 기록하지 않는다.

재현 명령은 다음과 같다. source hash가 바뀌었으면 최신 저장본의 필드 충돌을 확인하고
새 기준으로 재검증하며, output은 아직 없는 경로를 사용한다.

```powershell
python -B Tools/CharacterSelectPipeline/prepare_movie_lookup_mips.py --source C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.worldsequences.json --resources C:/Users/user/Desktop/LostArk/Client/Bin/Resources --expected-source-sha256 a2003e335915f04091202bf59e3056b6b3d8edf7cad2dce7bf98786c1d6b7afc --output out/MovieLookupMipRebuild20260929
```

후보는 `out/MovieLookupMip20260929/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/`
`LV_LOBBY_CLASSSELECT_SL00.worldsequences.json`, SHA
`7ee3500856dc485bbd611b6dccd6367cf148db2c16c5960aca40e51b83cf75e1`다.
이 절 작성 시 JSON parse·mip0/colorSpace 동일성·허용 필드 밖 변경 없음·diff check를 통과했다.
독립 scope 검토도 32개 object/58개 assetId 외 모든 JSON 값·순서가 동일함을 확인했다.
Monster/Summon/Prop 변경 0, Guardian199 cube armor 변경 0이며, 보스46개 모델·catalog·
rendering option·공유 lookup·shader를 포함한 730개 기준 파일의 SHA가 모두 동일하다.
증거는 `out/MovieTextureScope20260929/candidate-review.json` 및 `scope-verification.json`이다.
publisher Validate/Publish·원본 반영은 상위 작업에서 이어서 기록한다.
새 C++/shader가 없으므로 새 compile 요구는 없고, Client/UI 실행 및 화면 판정은 수행하지 않았다.


### G10 실제 게시·설치 확인

동일 candidate에 공식 Map publisher의 `Scope WorldSequences / Mode Publish`를 실행해
PASS했다. 생성된 runtime과 source의 SHA는 모두
`7ee3500856dc485bbd611b6dccd6367cf148db2c16c5960aca40e51b83cf75e1`이다.
최신 원본 저장본의 SHA, lookup 여덟 쌍의 실제 파일 SHA, 기존 donor/catalog를 다시
확인한 뒤 원본 작업 폴더에 source/runtime·도구·문서 총 7개 파일을 백업 및 원자적
교체했다. 무관한 원본 문서 변경은 현재 HEAD를 기준으로 3-way 병합해 보존했다.
설치 후 원본 폴더에서 공식 publisher `Mode Check`도 PASS했다.

설치 기록은 `out/MovieVisibilityBalance20260929/reflection-install/installed.json`,
원본 Check 로그는 `out/MovieVisibilityBalance20260929/reflection-installed-check.log`다.
최종 독립 확인에서도 보스·공통 shader·lookup 등 기존 730개 파일의 SHA는 모두
불변이다. 이 단계는 참조 데이터만 변경했으며 추가 Resources와 C++/shader 변경은 없다.
G09의 Product Debug 빌드 성공 이후의 데이터 반영이므로 mip 수정 자체의 재컴파일은
필요하지 않다. 최신 실행 화면의 반짝임과 미저장 편집 draft 반영은 확인하지 않았다.


후속 실행 준비 확인에서 원본 Debug EXE·Engine DLL·Server EXE와 runtime 필수 파일
검증 결과를 확인했다. 추가 Debug Product 재검증 시도
`20260928T222909703Z-debug-product.json`은 다시 실행된 Release Client 4개·Server 1개를
ProductOutputGuard가 감지하여 **컴파일 전에 중단**했다. 기존 G09의 성공 기록과 구분한다.
사용자는 현재 테스트 중이므로 프로세스를 종료하지 말라고 명시했다. 이에 실행 중
프로세스·메모리 상태를 그대로 유지하고 추가 빌드 및 Release EXE 교체는 보류했다.
이 작업에서 완료한 실행본 빌드는 Debug이며, Release 빌드 완료로 보고하지 않는다.
