# World Movie 머리카락·가디언 눈 재질 구현 계획

## G00. 현재 경계와 정본

사용자 정정에 따라 창술사 무비의 머리카락을 복구한다. 허리·몸체는 변경하지 않는다.
All Effects의 World 목록은 기존 다섯 Movie의 class ID를 사용하며 Open Editor와 Play All은
09-26 World Movie Effect Editor의 Level-owned 재생·편집 경로를 재사용한다.

## G01. 창술사 native600의 primitive 상수

`Shader_SourceCharacterBaseGroup576.hlsli`의 SourceCharacterBase600은 최종 opacity에
cb0[1].w를 곱하지만 Configure는 원본 material 소유 범위2..24만 채운다. 추출한
원본 PS040a63ec2e3e5e42a8f2194c6622723a는0/1을 engine 소유 prefix로 선언한다.
같은 opacity와 environment 배율의 기존 hair99 처리처럼 native600의 필요한 lane만
identity로 연결한다. Light의 대응 opacity lane도 같은 원본 계약으로 확인한다.
`Tools/VehiclePipeline/build_vehicle_source_material.py`가 같은 source ID와 상수 범위를
검증한 뒤 같은 보정을 생성하도록 하고 Client/Engine shader mirror를 함께 갱신한다.

## G02. Guardian eye의 실제 입력 검증

현재 일반 모델과 Movie의 eye는 모두 source.character.eye.v1이다. 현재 설치 WModel의
눈210/380정점, UV1/UV2와 원본 iris texture alpha가 존재한다. 원본 shader의 UV·parameter·
texture 입력, 실제 model bind와 draw 소비를 확인해 누락 지점을 수정한다. 기존 골격과
애니메이션·부착·조명 튜닝을 보존한다. 리소스 변경이 필요하면 원본 연결 증거, 최신 bytes
대조, 백업·원자 교체 후 Runtime Resources와 Desktop/GBResources에 같은 상대 ID로 전달한다.

## G03. 검증과 인계

원본 shader의 engine/material 상수 경계를 대조하고 수치 alpha/iris 검사를 수행한다.
변경 shader를 격리 컴파일하고 Product Debug/Release 빌드는 상위 통합 작업에서 조율한다.
새 C++ 파일·프로젝트 등록은 없다. Client/UI 실행·스크린샷·시각 PASS는 수행하지 않는다.
Movie5의 Open Editor/Play All은 실제 기존 호출 경로와 문서/resource 연결을 재검사하고,
RESULT에 자동 검사와 사용자 최종 화면 확인을 분리한다.
## G02. 가디언 눈 최종 프로젝트 조정

실제 Movie20s camera/pose와7개 Movie light·현재 scene directional을 사용하는 native5
수치 대조에서 원본 shadowfactor0.6은 각 직접광마다 최소0.4 밝기를 남겨 눈의 높은
휘도가 누적된다. 사용자가 반사도 낮춰 눈동자를 보이게 마무리하도록 요청했으므로
가디언 일반 body의 pc_dl_eye_mi와 Movie object a12265.p2의 pc_dk_eye_00_mi에만
shadowfactor1.0, tdspecular_intensity0.25를 적용한다. 이는 원본0.6/2를 프로젝트에 맞춘
조정이며 shader ABI 복원으로 설명하지 않는다. iris RGB·alpha·size·texture와 전역
렌더링 프로필은 보존한다. 최신 JSON의 두 stable row·두 field만 hash 확인·백업·원자
교체로 병합하고 WorldSequences 범위 publisher로 런타임 정본을 갱신한다. Guardian
Movie 재설치기는 현재 저장된 두 lighting field를 유지해 source donor refresh가 튜닝을
되돌리지 않게 한다. 새 schema·C++ 파일·Resources 추가는 없다.

## G04. 창술사 헤어 외형 개선과 가디언 Movie 근거 검사

사용자는 창술사 헤어를 원인 확정과 관계없이 개선하도록 요청했다. 일반·커스터마이징의
FT00은 원본 재질 override가 없고, Movie의 FT06-high는 실제 CModel pose에서 형상과 본
스케일이 유지된다. 따라서 이번 교체는 원본 오류 복원으로 설명하지 않고 요청한 외형
개선으로 기록한다. 같은 창술사 FT 계열의 더 풍성한 헤어를 우선 사용하며 다른 class의
두상과 골격을 추측해 이식하지 않는다.

일반 기본 장착과 커스터마이징의 초기 선택은 같은 asset ID를 사용하고 기존 사용자가
선택한 preset을 보존한다. 기본 장착의 material override는 CharacterCatalog의 해당
소유 경계로 옮기며 중복 소유를 만들지 않는다. Movie는 actor a12230.p0의 stable ID,
Intro·Loop clip과 clock을 보존한다. 일반 234본과 Movie 208본의 차이를 직접 검증하고,
양의 weight가 있는 본을 버리지 않는 호환 파생 모델을 기존 WModel 경로로 만든다.
원본 asset과 shader·전역 rendering option은 보존한다.

후보는 실제 골격·bind·clip과 정점 pose, material 소비를 검사한 후 최신 저장본의 해당
필드에만 병합한다. 설치와 Resources 전달은 같은 상대 ID 및 SHA로 확인하고, JSON parse,
변경 C++ 최소 컴파일, 필요한 Product Debug 빌드와 diff 검사를 수행한다. 사용자 화면
평가와 수치 검증은 분리한다. 새 런타임 C++ 타입이나 프로젝트 등록은 계획하지 않는다.

가디언의 회전·위치는 source root와 TypeData 값에 이어 실제 최종 particle draw matrix를
대조한다. 미연결 crack WORLD는 원본 actor·material·visibility 근거를 조사하며 사용자
표현인 유리와 동일하다고 단정하지 않는다. 근거가 없는 회전·위치·알파 보정은 적용하지 않는다.

## G07. Movie FT43의 반사 tangent basis 보존 (2026-09-29)

설치 FT43 donor와 Movie 파생본은 tangent sign이 없는 76-byte 정점이다. 기존 생성기는
좌표 반사 `(x,-z,-y)`에서 normal·tangent·winding은 바꾸지만, 명시 sign이 있을 때만
sign을 반전한다. `CWMeshReader`는 legacy 정점의 binormal을 `cross(normal,tangent)`로
복원하므로 determinant -1 변환 후 binormal이 donor basis와 반대가 된다. 위치·두피 간격
검사로 검출되지 않는 normal-map 입력 결함이다.

`derive_lance_movie_hair.py`는 donor의 기존 implicit +1을 먼저 WINT의 명시 sign으로
보존한 뒤 반사 시 sign을 뒤집는다. 기존 explicit sign은 그대로 읽어 한 번만 반전한다.
일반 FT43, geometry·UV·재질·Movie clip·head binding을 유지하며 렌더링 옵션은 변경하지 않는다.
오프라인 pose reader는 이미 제품이 사용하는 WINT1.6의 80-byte 정점을 읽도록 확장한다.
별도 C++ 파일이나 project/filter 등록은 없다.

후보는 out에서 생성하고 CWMeshReader와 같은 basis 재구성으로 모든 정점의 donor→Movie
normal/tangent/binormal 일치를 검사한다. 기존 설치본과 정점·index·bone·clip·UV·material을
대조하고 제품 WMesh/WModel reader 소비 검증을 수행한다. Resources 교체는 최종 후보와 최신 저장본 확인
절차를 따른다. 첨부 얼굴의 밝은 점무늬와 최종 사용자 외형 판정은 이 수치 검사와 구분한다.

## G08. 정상 커스터마이징 머리와 Movie 복장의 조합 (2026-09-29)

사용자는 원인 조사에 따른 부분 보정 대신 정상 커스터마이징 모델로의 교체를 선택했다.
일반 LanceMaster.wmodel의 얼굴 material3, 속눈썹4, 눈5(두 submesh)를 각각 Movie
`a12205.p1`, `a12205.p2`, `a12205.p0`의 파생 WModel로 만든다. 눈에 포함된 두 번째
head shell도 보존한다. 이 세 부분의 양의 weight 본은 Movie208본에 전부 존재한다.
일반 face00 geometry·UV·재질과 FT43 머리를 사용하며 기존 Movie의 의상 `a12241.*`,
배우 TRS·Intro/Loop·camera·clock과 무기·배경은 유지한다.

`derive_lance_movie_head.py`는 기존 split_material과 WModel writer를 사용한다. 좌표 반사와
명시 tangent sign을 함께 보존하고 본 이름으로 weight index를 대응시킨다. 가중하는 본의
mesh inverse bind는 donor 값을 좌표 변환하여 이식한다. Movie 원본 mesh에서 미사용 본의
offset은 일반 얼굴을 연결하기에 부적합하므로 재사용하지 않는다. Movie의 208본 skeleton과
클립을 사용하므로 일반 얼굴은 기존 얼굴 동작과 blink를 소비한다. 전정점 neutral pose
오차가 0.01cm 미만인지 생성 시 확인하여 잘못된 bind 후보를 거부한다.
파생 모델은 out에만 생성하고 변경할 stable object3행을 별도 patch로 남긴다. 원본 일반
모델·원본 Movie 모델은 덮어쓰지 않는다. 미저장 사용자 morph/preset을 읽었다고 주장하지 않는다.

일반 속눈썹 native6이 World에서도 기존 Character와 같은 forward pass9를 사용하도록
`WorldSequenceObject.cpp`의 pass selector에 기존 일반 translucent6/7/99를 연결한다.
새 렌더링 경로·shader·project/filter 등록은 없다. donor 기준 head-space shape, UV/texture,
본 index·clip bytes, 실제 reader와 준비 경로, 변경 TU 최소 컴파일을 검증한다. 최종 설치와
사용자의 Movie 외형 판정은 별도 단계로 기록한다.

## G10. 기존 정상 재질의 반사 lookup mip 연결 (2026-09-29)

사용자는 이미 보정한 워로드 기본 의상의 방식을 Movie에도 적용하도록 요청했다. 기존
`CMaterial::BuildRgbaMipChain`을 사용하는 정상 TGA 입력과 Movie의 단일 mip DDS 입력을
대조한다. 같은 512×512 RGBA와 colorSpace가 입증된 `hdr07_1`, `brdf_beckmann_spec`만
기존 `Character/SourceMaterials/efmaster_material_prologue/*.tga`로 연결한다. 정상 TGA는
기존 loader에서 10단계 mip를 생성한다. 밝기·roughness·specular scalar를 새로 조정하지 않는다.

다섯 class Movie의 실제 얼굴·몸·복장·무기에 속한 stable object의
`materialProfile.textures[expressionIndex].assetId`만 후보에서 변경한다. 소환 동물·몬스터·prop,
보스·공유 DDS·CharacterCatalog·normal map·shader는 변경하지 않는다. 가디언 HR00 armor의
native199는 두 lookup을 소비하지 않으므로 이번 연결의 적용 대상이 아니다. 모델·UV·clip·camera,
sourceMaterial·family·parameter·colorSpace와 팀장이 저장한 rendering option은 보존한다.

최신 source hash를 확인한 뒤 out 후보와 stable field patch·RGBA/hash 검증 receipt를 만든다.
서로 다른 texture path는 각각 비교하며 기존 정상 TGA와 mip0 전체가 다르면 거부한다.
publisher Validate/Publish 및 독립 field-diff 검사 후 현재 디스크 저장본에 병합한다. 공유 DDS와
보스 관련 파일은 전후 hash가 같아야 한다. 파일 반영과 사용자의 Movie 화면 판정은 구분한다.

## G11. 정상 얼굴 교체 뒤 Movie 재질 트랙 연결 복구 (2026-09-29)

설치한 `a12205.p1`의 `DefaultFace.wmodel`은 일반 catalog의 `pc_ft_face_mi`,
`source.character.classic-head.v1`(program4)를 소비하지만 Movie Intro/Loop의 두 materialTracks는
이전 `pc_ft_face_mi_high`, program200을 참조한다. 실제 사용 mesh와 전체20개 트랙을 대조하면
이 두 행만 불일치한다. 이름만 교체하면 program 검증이 다시 실패하므로 두 계약을 함께 수정한다.

`Data/Camera/ClassSelection.cinematics.json`에서 창술사 Intro/Loop의 해당 instanceId·slotId를
정확히 찾아 materialName·family·parameters만 정상 donor catalog에 맞춘다. 기존 `ibl_intensity`
곡선·시각·instanceId·slotId와 다른 모든 트랙, 사용자가 저장한 excludedWorldObjectIds는 보존한다.
기존 머리 생성기의 후보 receipt에도 이 두 cinematic patch를 함께 남겨 후속 설치 누락을 막는다.
별도 runtime·schema·C++·shader·Resources 변경은 없다.

후보는 out에서 만들고 실제 제품의 재질 Read/Configure 및 가능한 최소 소비 경로로 기존 거부와
수정 후 수용을 검사한다. JSON parse, 실제 mesh name/program, 두 행 밖 동일성, 삭제 목록 보존과
diff check를 확인한다. 최종 반영은 편집 중 데이터 반영 절차에 따라 저장 기준을 확인하고 최신
디스크의 해당 필드만 병합하며 hash 재검사·백업·원자 교체·실패 시 자기 변경 rollback을 유지한다.
이 manifest는 Data 원본을 직접 읽으므로 재빌드나 WorldSequences publish를 요구하지 않는다.
실행 중 편집기는 자동 Reload하지 않고 사용자에게 저장본 Reload와 Play All 경로를 안내한다.


## G12. 가디언나이트 Movie 갑옷·무기의 원본 표면 mip 복구 (2026-09-30)

얼굴·피부의 hdr07 lookup은 G10에서 이미 full mip TGA로 연결되었고, native199 갑옷의
scene reflection cube도128²/8mip를 보유한다. 갑옷5개와 무기1개의 실제 표면 texture30개는
설치 DDS가1mip인 반면 원본 pc_ddk_hr_00/wp_ddk_hr_00 package에는29개11mip와 무기ORM1개10mip가
남아 있다. 표면 sampler가 사용할 원본 mip를 복구하며 이 사실을 반사 cube 누락으로 설명하지 않는다.

기존 extract_ue3_texture_mips.py로 원본 BC1/BC3/BC5 block을 재압축·필터 생성 없이 복구한다.
현재 DDS와 mip0 payload가 byte-exact이고 형식·크기·colorSpace가 같은 후보만 허용한다.
Movie 전용 Resources-relative 신규 ID30개로 보관하고 WorldSequences objectResources의 해당
6개 stable object materialProfile.texture assetId만 교체한다. 기존 공유 DDS와 일반캐릭터·보스,
shader·roughness/밝기/렌더링 옵션은 보존한다. ORM green의 기존 roughness 소비와 원본분포를
확인하되 사용자 화면을 보지 않고 유리 느낌이 완전히 해결되었다고 기록하지 않는다.

후보 texture의 실제 loader mip와 source/material 계약을 검증한 뒤 최신 디스크 WorldSequences의
대상 필드만 병합한다. hash 재확인·백업·원자 교체와 실패 시 자기 변경 rollback을 유지하며
WorldSequences scope만 publish한다. 카메라 후보와 별개의 문서이므로 저장본의 무관한 사용자
수정은 유지한다. 실행 중 메모리 Reload는 자동 수행하지 않는다. 신규 C++/project 등록은 없다.
