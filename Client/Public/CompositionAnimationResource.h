#pragma once

// 대상 종류의 고정 개수와 순서를 보관한다.
#include <array>
// 길이의 ms 값을 uint32_t로 표현한다.
#include <cstdint>
// asset ID와 표시 이름을 snapshot 값으로 소유한다.
#include <string>
// 존재 검사에서 호출자의 문자열을 복사하지 않고 읽는다. 호출 뒤 view를 저장하지 않는다.
#include <string_view>
// Sequence의 clip 개수와 순서는 저작 내용에 따라 달라진다.
#include <vector>
// 원본 clip 이름으로 표시 이름을 찾는 조회 결과다. 재생 순서는 이 map의 순회 순서에 기대지 않는다.
#include <unordered_map>

namespace Client
{

// 물리 clip의 식별에는 모델과 원본 패키지가 함께 필요하다.
// 서로 다른 골격에 같은 clip 이름이 있어도 선택 가능한 자원은 두 개로 유지한다.
struct COMPOSITION_ANIMATION_RESOURCE final
{
	std::string strTargetAssetName;
	std::string strModelAssetId;
	std::string strSourceAssetId;
	std::string strProfileId;
	std::string strRuntimeClip;
	std::uint32_t iDurationMs = 0u;
	std::string strEndPolicy = "EXACT";
	// Sequence가 iDurationMs를 바꿔도 원본 tick과 tick/초를 남겨 재생 길이의 정밀도를 보존한다.
	// 저장 검증에 선택한 preview 모델의 현재 상태를 대신 사용하지 않는다.
	float fDurationTicks = 0.f;
	float fTicksPerSecond = 0.f;
	// 화면에 보이는 이름이다. 삽입할 clip의 식별은 원본 clip과 패키지 정보가 담당한다.
	std::string strDisplayName;
    // World 목록의 identity다. native clip과 물리/Transform Motion은 서로 다른 소비자가 처리한다.
    std::string strWorldObjectId, strWorldMotionId, strAnimationSetAssetId;
    std::vector<std::string> CategoryPath;
    float fModelPreScale = 0.01f;
    bool Is_WorldMotion() const { return !strWorldObjectId.empty() && strRuntimeClip.empty(); }
};

// 한 Sequence의 clip을 순서대로 소유한다. vector는 순회·직렬화와 맞지만 삽입/교체 뒤 예전 원소 포인터를 보관하면 안 된다.
struct COMPOSITION_ANIMATION_SEQUENCE_RESOURCE final
{
	std::string strStableId;
	std::string strDisplayName;
	std::string strProfileId;
	std::string strTargetAssetName;
	std::vector<COMPOSITION_ANIMATION_RESOURCE> Clips;
};

// 고정 whitelist를 헤더에서 공유한다. inline은 여러 CPP의 동일 정의를 허용하며 함수의 강제 인라인화나 속도 보장이 아니다.
inline constexpr std::array<const char*, 28u>
	COMPOSITION_ANIMATION_TARGET_ASSET_NAMES = {
		"LanceMaster", "GunSlinger", "Slayer", "Artist", "DimensionMaster", "Warlord", "GuardianKnight",
		"Valtan", "Valtan_Ghost_MN_RPBF_02",
		"MN_RPCT_00", "MN_RPCT_03", "MN_RPCT_05", "MN_RPCT_06", "MN_RPCZ_00", "MN_RPCZ_00-1",
		"Monster_480001_MN_PADD_01",
		"Monster_480002_MN_SJFC_00_4",
		"Monster_480003_MN_0019_05",
		"Monster_480005_Lugaru_MN_RPRS_02",
		"Monster_Valtan_SJFC_Elite",
		"Monster_Kouku_CMDUP_02",
		"Monster_Kouku_REUP_04",
		"Monster_Kouku_RHKP_06",
		"Monster_Kouku_CMDGR_03",
		"Monster_Card_Heart",
		"Monster_Card_Diamond",
		"Monster_Card_Club",
		"Monster_Card_Spade" };

// 선택적인 원본 표시 이름만 조회한다. clip의 존재나 재생 가능 여부를 이 이름 목록으로 결정하지 않는다.
std::unordered_map<std::string, std::string> Read_CompositionAnimationDisplayNames(
    const std::string& assetName);

// 설치 모델의 전체 clip과 저장 World Object Motion을 읽는다. 실패한 source는 이전 snapshot을 유지한다.
bool Read_CompositionAnimationResources(std::vector<COMPOSITION_ANIMATION_RESOURCE>& resources,
    std::string& status);

// 목록 UI가 사용할 category 경로를 값으로 반환한다. 몇 단계의 작은 경로이며 문서나 자원의 소유권은 변경하지 않는다.
inline std::vector<std::string> CompositionAnimationCategory(const std::string& assetName)
{
    for (std::size_t i = 0u; i < 7u; ++i)
        if (assetName == COMPOSITION_ANIMATION_TARGET_ASSET_NAMES[i])
            return {"Character", assetName == "GunSlinger" ? "Gunslinger" : assetName};
    if (assetName == "MN_RPCZ_00-1") return {"Character Transform", "Mario / Clown"};
    if (assetName == "Valtan") return {"Boss", "Valtan"};
    if (assetName == "Valtan_Ghost_MN_RPBF_02") return {"Boss", "Ghost Valtan"};
    if (assetName == "MN_RPCT_00") return {"Boss", "Saydon"};
    if (assetName == "MN_RPCT_03") return {"Boss", "Saydon (colorless)"};
    if (assetName == "MN_RPCT_05") return {"Boss", "Saydon (Gate 3 / Encore)"};
    if (assetName == "MN_RPCT_06") return {"Boss", "Large Saydon"};
    if (assetName == "MN_RPCZ_00") return {"Boss", "Kouku"};
    return {"Other models", assetName};
}

inline std::vector<std::string> CompositionAnimationCategory(const COMPOSITION_ANIMATION_RESOURCE& resource)
{
    return resource.CategoryPath.empty() ? CompositionAnimationCategory(resource.strTargetAssetName) : resource.CategoryPath;
}

// 28개 고정 항목을 선형 대조한다. 추가 hash 저장소가 없지만 조회량이 커져도 최적이라는 측정은 아니다.
inline bool Is_CompositionAnimationTargetAsset(const std::string_view name)
{
    if (name.starts_with("WorldObject:")) return true; // 실제 model은 preview panel이 저장 inventory로 검증한다.
	for (const char* candidate : COMPOSITION_ANIMATION_TARGET_ASSET_NAMES)
	{
		if (name == candidate)
			return true;
	}
	return false;
}

}
