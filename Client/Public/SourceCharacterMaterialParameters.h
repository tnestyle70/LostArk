#pragma once

//Client 소유 namespace를 여는 공용 매크로다.
#include "Engine_Macro.h"

//저작 JSON 입력과 shader에 전달할 재질 상수의 값 타입을 사용한다.
#include "DataJson.h"
#include "BinaryAsset/ModelAssetData.h"
//원본 이름별 float 네 값을 소유하고, 수정할 이름은 string_view로 차용한다.
#include <array>
#include <map>
#include <string>
#include <string_view>

NS_BEGIN(Client::SourceCharacterMaterial)
//원본 파라미터 이름을 보존한다. 실제 shader 상수 배치는 CPP의 생성 구현 한 곳에서 계산한다.
using PARAMETER_VALUES = std::map<std::string, std::array<float, 4>>;

//Catalog·외형 도구가 숫자 또는 네 성분 배열을 읽는다. 검증 실패 시 기존 result를 유지한다.
bool Read(const DATA_JSON_VALUE& parameters, PARAMETER_VALUES& result);
//선택 family의 입력을 shader 상수로 변환한다. 호출자가 반환값 확인 뒤 재질에 반영한다.
bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result);
//Character의 transcolor·buffcolor cue를 대응 슬롯에 반영한다. 미지원 이름·program은 false다.
bool Patch_NamedVector(Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& material,
    std::string_view parameter, const float4_t& value);
//JSON 읽기와 상수 변환을 한 번에 수행하는 진입점이다.
bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result);
NS_END
