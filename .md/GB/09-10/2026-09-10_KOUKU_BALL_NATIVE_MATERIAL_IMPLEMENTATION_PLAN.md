# 쿠크 공 원본 PBR 재질 연결 구현 계획

기준일: 2026-09-10. 기존 World Object → CModel → CMaterial 경로를 확장한다.

## G00. 현재 정본과 원인

공은 `mn_rhcn_00.mat.mn_rhcn_00_mi`이며 부모는 monster_base_msk_high → pbr_base_msk다. 실제 원본 Base/Light 프로그램은 각각 328/300 instruction이고 material-owned 입력은 8 texture다. 현재 World Object는 D/N/S/E만 일반 재질로 읽는다. S를 일반 specular luminance로 해석하여 roughness power 3, metalicness power 0.2, hdr07_1 IBL, brdf_beckmann_spec LUT와 원본 emissive 시간식을 잃었다. 바닥 BG 반사재질을 공에 복사하지 않는다.

## G01. 원본 PS와 material 값

`Shader_SourceCharacterPrograms.hlsli`에 원본 pair를 program 21로 추가하고 기존 renderer의 Base/Light 프레임 입력과 정상 dispatch에 연결한다. `SourceCharacterMaterialParameters.h`는 원본 이름의 parameter를 같은 uniform row로 패킹한다. Engine Model admission은 program 21을 허용한다. 기존 program 1~20과 geometry는 보존한다.

## G02. World Object 저장과 실제 소비자

`WorldSequenceDocument.h/.cpp`의 object resource에 optional materialProfile을 추가한다. materialName/sourceMaterial/family/parameters/texture expression index와 Resources 상대 ID를 저장하며 Load/Validate/Save/equality에 포함한다. 실패하면 기존 document를 유지한다. `Prepare_ObjectResources`는 검증한 override를 CModel load descriptor로 전달한다. static `CMapAssetRenderUtils`가 이 surface를 실제 Bind_SourceCharacter로 제출하여 직접광과 IBL 입력을 사용한다. 공의 pose, scale, motion, anchor는 유지한다. publisher는 같은 optional 구조/경로/필수 parameter와 texture index를 검사한다.

## G03. 쿠크 맵 환경 입력 경계

쿠크 maplights 4행은 이미 활성화되어 있고 mapmaterials는 BG 바닥 2행과 sampler 3행이다. 공의 IBL은 원본 material 소유 2D texture이며 scene ambient를 일괄 증폭하는 기능이 아니다. 기존 source family와 같은 입력이 아니면 원본으로 간주해 덮지 않는다. 전체 map material family 216개 복원은 이번 공 수정 완료와 구분한다.

## G04. 검증

원본 native texture wire/상속, 새 profile parse와 Save/Reopen, 실제 CModel 8 texture admission, 최소 Model/WorldSequenceDocument/MapAssetRenderUtils 컴파일, 관련 Mesh/Deferred FX 컴파일과 git diff --check를 확인한다. 새 C++ 파일은 없어 프로젝트/filter 등록은 추가하지 않는다. Client/UI 실행·화면 캡처·visual PASS는 하지 않으며 사용자 화면 확인을 남긴다.
