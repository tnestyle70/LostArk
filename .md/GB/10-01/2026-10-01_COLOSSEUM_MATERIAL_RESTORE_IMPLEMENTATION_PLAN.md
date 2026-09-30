# 콜로세움 원본 재질·텍스처·조명 복원 구현 계획

## G00. 기준점과 완료 경계

사용자의 2026-10-01 요청에 따라 기존 조사 전용 작업을 실제 복원으로 이어간다. 기준은
`a1329e0c2ba0274f21bba74e9120eca0de26bac6`, 작업 브랜치는
`codex/colosseum-material-restore-20261001`이다. 기존 조사 결과는
[재질 조사 RESULT](2026-10-01_COLOSSEUM_MATERIAL_INVESTIGATION_RESULT.md)를 따른다.
기존 무관한 untracked 백업과 다른 Area의 렌더링 옵션은 보존한다.

현재 설치본은 모델103종·배치1294개이며 named source material은 없다. 원본67재질의
149slot, 미해결 dummy1slot, RNM1290배치와96texture가 조사됐다. 원본1302배치 중
석상·성 장식2종8배치는 geometry 변환에서 제외됐다. 원본 기하·재질·조명 연결,
파일 설치와 게시, 제품 컴파일, 사용자의 화면 확인을 분리해 기록한다.

## G01. 원본 texture와 mip 후보

`Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py`와 기존 UModel 추출을 사용한다.
원본 texture identity178개를 실제 색공간·크기·포맷·전체 mip과 연결하며, 기존 mip0와
원본 압축 bytes가 같은지 확인한다. TGA로 제공되는 입력은 원본 decode를 보존한다.
단순 확대나 전역 mip bias로 낮은 해상도 입력을 감추지 않는다. 후보는
`out/ColosseumRestore20261001/textures`에 준비한다.

## G02. 실제 model slot과 source material 연결

`CModel -> CMaterial` 및 기존 Area mapmaterials를 사용한다. 누락 모델 복구 후 BG62재질·139slot은
기존 source surface compiler의 실제 texture·render flag·parameter 입력을 연결한다.
특수10재질·15slot은 원본 selected permutation과 기존 native program을 대조해
지원되는 실제 경로에 연결한다. 미지원 branch나 미확정 기본값을 정상 복원으로 표시하지
않는다. 후보는 `out/ColosseumRestore20261001/materials`에 준비한다.

## G03. 원본 scene·component lighting

원본 `LV_PVP_COLOSSEUM_PS`의 WorldInfo·environment·directional/local light·fog·LUT와
배치별 RNM을 추출한다. 기존 neutral scene을 콜로세움 전용 source profile에 연결한다.
RNM에 포함된 광원의 중복 가산을 피하는 기존 수신 계약을 지킨다. atlas pair가 다른
동일 모델은 `build_map_rnm_variant_set.py`의 stable variant를 사용하고 UV1 및
배치별 scale/bias를 보존한다. 1298개 RNM과 585개 static shadow의 atlas·UV를 연결한다.
기존 광원 환경 전달 경로에서 원본 directional lightfunction의 optional 연결을 조사하고,
source shader와 producer를 확인한 범위만 구현한다. 다른 profile의 기본값은 바꾸지 않는다.
후보는 `out/ColosseumRestore20261001/lighting`에 준비한다.

## G04. geometry와 실제 consumer 검증

원본 glTF/WModel의 UV1·COLOR0·normal/tangent와 material slot을 대조한다.
초기 반입에서 제외된 두 source mesh는 원본 basis·정점·삼각형 근거로 복구 가능성을
확인한다. 별도 native color stream이 없는 원본에 UModel이 잘못 내보낸 COLOR0를 제거하고,
실제 color stream이 있는 두 source mesh는 원본 BGRA를 RGBA로 연결한다. 정점 위치·UV·basis·
index·bounds·WMAT는 보존하고 source identity와 stream 근거를 결과에 남긴다.
BSP·decal·volume 등 다른 carrier가 필요한 입력을 material 복원 완료에
포함시키지 않는다. 기하 후보는 `out/ColosseumRestore20261001/geometry`에 둔다.
새 C++ 파일을 추가할 경우에만 프로젝트·filter 등록을 함께 처리한다.

## G05. 설치·게시·전달

최신 디스크 저장본의 stable ID와 변경 필드만 병합한다. 교체 직전 hash 확인과 원본
백업·원자 교체를 사용하며 동시 변경은 덮어쓰지 않는다. installed Resources와
`C:/Users/user/Desktop/GBResources2`에 같은 Resources 상대 경로로 추가·교체분을 준비한다.
기존 Area publisher로 Validate → Publish → Check하며 source와 runtime snapshot을
같이 전달한다. source material·light·scene JSON의 실제 파일과 참조를 검사한다.

C++/HLSL을 변경하면 정상 증분 Product Build를 수행한다. JSON/XML parse,
변경 기능의 focused 검사와 `git diff --check`를 확인하고 RESULT에는 실행한 검증만
기록한다. Client/UI 실행·조작·캡처 및 최종 화면 판정은 사용자가 수행한다.

## G06. 사용자 지정 scene quality 정렬

추가 요청에 따라 콜로세움의 `scene.colosseum.source-day.v1`만 발탄의 현재
`scene.valtan.cool-low-key.v1` quality 값으로 맞춘다. 두 실제 레이드 base profile의
공통 기준은 Bloom OFF, bloom multiplier0, SSAO OFF, gamma2.2다. 서로 다른 노출·FXAA는
발탄을 기준으로 한다. 콜로세움의 sourcePostProcess, directional·point lighting,
fog·shadow·environment는 유지한다. 발탄·쿠크와 globalQuality 자체는 바꾸지 않는다.

최신 디스크의 profile을 다시 읽어 후보를 검증한 뒤 revision을 올리고 원자 교체한다.
공식 Rendering publisher로 게시하고 다른 profile28개 및 콜로세움의 조명·안개·원본
색 보정이 그대로인지 JSON 비교한다. 실행 중 편집 draft의 Reload와 사용자 화면 판정은
파일 게시와 구분한다.
