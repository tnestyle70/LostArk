# 베른·발탄 표면 텍스처의 원본 mip 복원 구현 계획서

## G00. 현재 소비자와 복원 범위

사용자는 베른·발탄 성능 개선과 텍스처 품질 옵션에 이어 원본 데이터로 복구 가능한 mip의
복원을 요청했다. 현재 runtime mapmaterials가 실제 참조하는 DDS를 정본으로 조사한다.
WModel에 남은 fallback 복사본을 실제 typed material의 입력으로 간주하지 않는다.

베른 비조명 표면 DDS1,580개는 full chain1,527·단일11·부분42개이며 부분42개는
Landscape height 입력이므로 원본 native 범위와 대조하기 전에는 누락으로 판정하지 않는다.
발탄 비조명434개는 full10·단일424개다. 단일 mip이라는 사실만으로 원본에도 하위 단계가
있다고 가정하지 않는다. 원본 Texture2D와 실제 사용 재질을 연결한 뒤 복원 대상을 확정한다.

베른 grass24의 현재 SourceMaterials 입력은11단이 있지만 mip0만 원본 압축 bytes와 같고
하위10단은 다르다. 같은 Landscape NativeLayers 입력은11단 모두 원본과 같다. 따라서
단계 부재, 프로젝트 생성 단계, native 복원 완료를 구분한다. 이미 검증된 RNM 조명 복원은
다시 생성하지 않는다. 원본 게임의 품질 등급별 설정을 현재 최소 mip0/1/2/3 정책과 동일하다고
주장하지 않는다. 그 정책은 별도 환경설정 결과의 구현 계약이다.

## G01. 원본 식별과 후보 준비

현재 material의 sourceMaterial·texture field, 기존 추출 manifest·receipt 및 원본 MIC
상속 parameter를 대조해 Texture2D object와 실제 package를 식별한다. 원본 package는
읽기 전용으로 사용하고 기존 extract_ue3_texture_mips의 single/batch 경로를 재사용한다.
원본 native mip의 크기·format·bulk flags·payload를 검사하고 현재 mip0 압축 block과
동일한 경우에만 하위 mip을 복구한다. 이름 유사성만으로 다른 텍스처를 대입하지 않는다.

후보와 원본 hash·소비자·단계별 hash를 out에 보관한다. DX10 DDS는 현재 format/sRGB
metadata를 보존하고 같은 BC 형식의 native blocks를 사용한다. 단일 mip header의 count/caps는
검증된 chain에 맞춘다. 원본에 없는 mip 생성·재압축·resampling·최고 해상도 변경을 하지 않는다.
원본의 특수 lookup/cube/height 또는 지원하지 않는 carrier는 별도 근거로 남기며 범용 표면
텍스처처럼 변환하지 않는다. 필요한 도구 변경은 기존 경로에 추가하고 별도 runtime을 만들지 않는다.

## G02. 검증 후 같은 Resources 경로에 설치

후보 mip0와 기존 입력, native chain의 모든 압축 block 및 DDS 길이/format을 검증한다.
제품과 같은 DirectXTK loader의 화면 없는 GPU 검사로 texture/SRV의 mip 범위와 readback을
확인한다. 원본·decoder·소비자 문서·교체 대상의 hash를 교체 직전에 다시 확인한다.

사용자의 원본 복원 요청 범위 내에서 대상 DDS만 백업하고 같은 디렉터리의 임시 파일을
원자 교체한다. 동시 변경은 덮어쓰지 않으며 실패 rollback은 자기 설치 hash와 같은 파일에만
적용한다. material ID·색 공간·UV·geometry·배치·scene 품질 옵션은 보존한다.
같은 asset 경로의 Resource 교체이며 map publisher나 사용자 메모리 draft를 갱신하지 않는다.
Client/UI 실행·자동 Reload·종료는 수행하지 않는다. 다음 로드와 현재 GPU 메모리 상태를 구분한다.

## G03. 완료 증거와 경계

대응 RESULT에는 실제 소비자 inventory, 원본 join·복원/보존/지원 불가 개수, 설치 전후 hash,
byte 검증·GPU 검증과 백업 경로를 기록한다. texture sampling의 복구를 draw 감소나60FPS
달성으로 환산하지 않는다. Resources payload는 Git에 포함하지 않고 원본 데이터와 무관한
전체 복구 ZIP을 만들지 않는다. 실제 화면·컷신 성능은 사용자가 새 로드 후 확인한다.

## G04. 실재 원본 object 이름의 하이픈 보존

실제 MIC가 참조하는 `wp_fbm_av_002-1_d/n` 원본은 존재하지만 기존 mip extractor의
source-object 정규식이 하이픈을 거부한다. Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py의
split_object에서 각 비어 있지 않은 component의 첫 영숫자/밑줄 뒤에만 하이픈도 허용한다.
점은 package/object 구분자로만 사용하며 slash·backslash·drive·공백·quote·빈 component와
선행 dash 거부는 유지한다. 별도 임의 경로 또는 원본 이름 변경으로 우회하지 않는다.

실제 두 원본의 native 회수와 mip0 exact·완전한 chain을 대조하고 경로성 입력 거부를
검증한다. Python 문법 검사와 README의 원본 object 이름 계약을 갱신한다.
이 도구 변경은 C++/HLSL 제품 재컴파일을 요구하지 않는다.

## G05. 원본 NoMipmaps 정책 복원

후속 원본 대조에서 foliage/normal/noise/sky/state texture5개는 모두
MipGenSettings=TMGS_NoMipmaps를 명시하고 native1단만 가진다. 현재 DDS에는 생성된
하위 단계가 추가되어 있다. 각 mip0의 원본 BC bytes 일치와 실제 소비자를 확인했으므로
이5개는 current DX10 format/sRGB/dimension을 유지한 채 원본1단 범위로 복원한다.
header의 mip count/flags/caps만1단에 맞추고 원본 mip0를 보존한다. 없는 하위 native mip을
생성하거나 다른 texture의 chain을 재사용하지 않는다. state texture의 실제 SampleBias
소비 경로도 원본에 없는 축소 단계를 선택할 수 있으므로 NoMipmaps를 일반 누락과 구분한다.
별도 manifest·GPU loader 검사 후 동일 freshness/backup/atomic 설치 절차를 적용한다.
