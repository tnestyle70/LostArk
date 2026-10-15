#include "SourceCharacterMaterialParameters.h"
//프로그램별 생성 packing을 이 CPP에서만 컴파일한다. 공개 함수는 같은 구현으로 연결한다.
#include "SourceCharacterMaterialParameters_Generated.inl"

NS_BEGIN(Client::SourceCharacterMaterial)
//Catalog와 저작 화면이 사용할 이름별 입력을 검증한다.
bool Read(const DATA_JSON_VALUE& parameters, PARAMETER_VALUES& result)
{
    return Detail::Read(parameters, result);
}

//이미 읽은 입력으로 shader 상수를 준비한다.
bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    return Detail::Configure(family, parameters, result);
}

//재생 중인 색 cue가 변경할 슬롯만 생성된 binding 규칙으로 찾는다.
bool Patch_NamedVector(Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& material,
    std::string_view parameter, const float4_t& value)
{
    return Detail::Patch_NamedVector(material, parameter, value);
}

//JSON 입력 경로도 동일한 생성 packing을 사용한다.
bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)
{
    return Detail::Configure(family, parameters, result);
}
NS_END
