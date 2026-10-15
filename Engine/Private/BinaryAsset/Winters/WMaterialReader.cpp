// WModelDecoder의 분리 재질과 WMOD 내장 재질 해석 요청을 받는다.
#include "WMaterialReader.h"

// 파일 소유 byte 배열과 범위 검사 커서로 version별 고정 재질 행을 읽는다.
#include "BinaryAsset/BinaryReader.h"
// WMAT/WMA2/WMA3의 magic과 저장 행 크기·개수 상한을 사용한다.
#include "WFormatTypes.h"

// 고정 magic을 비교하고 V3의 tint 네 묶음을 값으로 복사한다.
#include <cstring>

using namespace Engine::WintersFormat;

namespace
{
	// 문자열 경로를 정규화한 상대 관계만 검사한다. 실제 junction/symlink를 해석하는 canonical 경로 검증은 아니다.
	bool_t IsBelowRoot(const filesystem::path& root, const filesystem::path& candidate)
	{
		if (root.empty() || candidate.empty())
			return false;
		const filesystem::path relative =
			candidate.lexically_normal().lexically_relative(root.lexically_normal());
		if (relative.empty() || relative.is_absolute())
			return false;
		const auto first = relative.begin();
		return first != relative.end() && *first != L"..";
	}

	// 읽기가 완료된 header에서 4바이트만 비교한다. 입력 크기 검사는 호출자가 앞서 수행한다.
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	// 고정 폭 이름을 소유 string으로 복사한다. NUL이 없으면 전체 배열을 사용한다.
	template <size_t Size>
	string ReadFixedString(const char_t (&value)[Size])
	{
		size_t length = {};
		while (length < Size && '\0' != value[length])
			++length;
		return string(value, length);
	}

	// 이전 Resource 경로의 접두 구간을 찾아 현재 assetRoot 아래 존재하는 파일로 재배치한다. 외부 절대 경로를 그대로 열지는 않는다.
	filesystem::path ResolveBelowAssetRoot(
		const MODEL_ASSET_LOAD_DESC& desc, const filesystem::path& storedPath)
	{
		if (desc.assetRoot.empty())
			return {};

		const wstring genericPath = storedPath.generic_wstring();
		constexpr const wchar_t* resourcePrefixes[] = {L"Resource/LostArk/", L"Resource/", L"Resources/"};

		for (const wchar_t* pPrefix : resourcePrefixes)
		{
			const wstring prefix = pPrefix;
			const size_t prefixPosition = genericPath.find(prefix);
			if (wstring::npos == prefixPosition)
				continue;

			const filesystem::path relativeToAssetRoot = genericPath.substr(prefixPosition + prefix.size());
			const filesystem::path candidate = (desc.assetRoot / relativeToAssetRoot).lexically_normal();
			if (IsBelowRoot(desc.assetRoot, candidate) && filesystem::exists(candidate))
				return candidate;
		}

		return {};
	}

	// 재질 옆→assetRoot→이전 Resource 접두 경로 순으로 찾는다. 마지막 반환은 존재하지 않는 허용 상대 경로일 수도 있다.
	// 따라서 비어 있지 않은 반환값만으로 texture 설치/GPU 생성 성공을 판단하지 않는다. assetRoot가 비면 루트 제한도 적용하지 않는다.
	filesystem::path ResolveTexturePath(
		const MODEL_ASSET_LOAD_DESC& desc, const filesystem::path& materialPath, const wchar_t* pValue)
	{
		size_t length = {};
		while (length < 260 && L'\0' != pValue[length])
			++length;
		if (0 == length)
			return {};

		const filesystem::path storedPath(wstring(pValue, length));
		if (storedPath.is_absolute())
		{
			return ResolveBelowAssetRoot(desc, storedPath);
		}

		const filesystem::path besideMaterial = (materialPath.parent_path() / storedPath).lexically_normal();
		if ((desc.assetRoot.empty() || IsBelowRoot(desc.assetRoot, besideMaterial)) &&
			filesystem::exists(besideMaterial))
			return besideMaterial;

		if (!desc.assetRoot.empty())
		{
			const filesystem::path belowAssetRoot = (desc.assetRoot / storedPath).lexically_normal();
			if (IsBelowRoot(desc.assetRoot, belowAssetRoot) && filesystem::exists(belowAssetRoot))
				return belowAssetRoot;

			const filesystem::path relocatedPath = ResolveBelowAssetRoot(desc, storedPath);
			if (!relocatedPath.empty())
				return relocatedPath;
		}

		return desc.assetRoot.empty() || IsBelowRoot(desc.assetRoot, besideMaterial)
			? besideMaterial
			: filesystem::path{};
	}
}

// 분리 WMSH의 선택 companion 재질 경로다. 파일 없음/빈 파일/read 실패는 기본 재질로 대체하지만, 읽힌 잘못된 WMAT는 실패한다.
bool_t CWMaterialReader::Read(
	const MODEL_ASSET_LOAD_DESC& desc,
	uint32_t minimumCount,
	vector<MODEL_MATERIAL_DATA>& outMaterials,
	MODEL_DECODE_REPORT& outReport) const
{
	filesystem::path materialPath = desc.materialPath;
	if (materialPath.empty())
	{
		materialPath = desc.meshPath;
		materialPath.replace_extension(L".wmat");
	}

	// 읽힌 재질은 같은 parser로 검증한다. 읽지 못한 경우에만 아래 최소 슬롯과 fallback을 구성한다.
	vector<uint8_t> bytes;
	if (CBinaryReader::LoadFile(materialPath, bytes))
		return ReadMemory(
			bytes.data(), bytes.size(), desc, materialPath, minimumCount, outMaterials, outReport);

	outMaterials.clear();
	outMaterials.resize(minimumCount);
	for (MODEL_MATERIAL_DATA& material : outMaterials)
	{
		if (material.diffusePath.empty() && !desc.fallbackDiffusePath.empty())
			material.diffusePath = desc.fallbackDiffusePath;
	}
	return true;
}

// WMOD의 embedded 재질과 분리 파일이 공유하는 parser다. outMaterials를 즉시 비우고 최소 슬롯부터 잡으며 rollback은 제공하지 않는다.
bool_t CWMaterialReader::ReadMemory(
	const uint8_t* pData,
	size_t dataSize,
	const MODEL_ASSET_LOAD_DESC& desc,
	const filesystem::path& containerPath,
	uint32_t minimumCount,
	vector<MODEL_MATERIAL_DATA>& outMaterials,
	MODEL_DECODE_REPORT& outReport) const
{
	outMaterials.clear();
	outMaterials.resize(minimumCount);
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WMAT section is empty or truncated.";
		return false;
	}

	try
	{
		// major/magic/content 범위를 검사한다. 이 reader는 minor version과 외부 trailing bytes를 엄격히 제한하지 않는다.
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) || 1 != fileHeader.versionMajor ||
			0 != fileHeader.flags || fileHeader.contentSize > fileReader.Remaining())
		{
			outReport.error = "Invalid WINT material file header.";
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const MATERIAL_META_HEADER materialHeader = reader.Read<MATERIAL_META_HEADER>();
		const bool_t isV2 = HasMagic(materialHeader.magic, WMAT_V2_MAGIC);
		const bool_t isV3 = HasMagic(materialHeader.magic, WMAT_V3_MAGIC);
		if ((!HasMagic(materialHeader.magic, WMAT_MAGIC) && !isV2 && !isV3) ||
			materialHeader.materialCount > MAX_MATERIALS)
		{
			outReport.error = "Invalid WMAT metadata.";
			return false;
		}

		// 저장 materialIndex가 runtime 슬롯을 지정한다. 같은 index의 중복 행은 뒤 행이 덮으며 모든 슬롯의 명시 정의를 요구하지 않는다.
		outMaterials.resize((max)(outMaterials.size(), static_cast<size_t>(materialHeader.materialCount)));
		for (uint32_t i = 0; i < materialHeader.materialCount; ++i)
		{
			// WMA3는 V2 texture에 mask와 네 tint를 추가한다. 여기서는 tint의 finite/범위를 검사하지 않고 경로 유무로 enabled를 정한다.
			if (isV3)
			{
				const MATERIAL_ENTRY_V3 entry = reader.Read<MATERIAL_ENTRY_V3>();
				if (entry.materialIndex >= outMaterials.size())
				{
					outReport.error = "WMA3 contains an out-of-range material index.";
					return false;
				}

				MODEL_MATERIAL_DATA& material = outMaterials[entry.materialIndex];
				material.name = ReadFixedString(entry.name);
				material.nameHash = entry.materialHash;
				material.diffusePath = ResolveTexturePath(desc, containerPath, entry.baseColorPath);
				material.normalPath = ResolveTexturePath(desc, containerPath, entry.normalPath);
				material.specularPath = ResolveTexturePath(desc, containerPath, entry.specularPath);
				material.emissivePath = ResolveTexturePath(desc, containerPath, entry.emissivePath);
				material.opacityPath = ResolveTexturePath(desc, containerPath, entry.opacityPath);
				material.ormPath = ResolveTexturePath(desc, containerPath, entry.ormPath);
				material.metallicPath = ResolveTexturePath(desc, containerPath, entry.metallicPath);
				material.roughnessPath = ResolveTexturePath(desc, containerPath, entry.roughnessPath);
				material.ambientOcclusionPath =
					ResolveTexturePath(desc, containerPath, entry.ambientOcclusionPath);
				material.colorMaskPath = ResolveTexturePath(desc, containerPath, entry.colorMaskPath);
				memcpy(&material.colorTint.vDiffuse, entry.diffuseTint, sizeof(material.colorTint.vDiffuse));
				memcpy(&material.colorTint.vRegionA, entry.regionTintA, sizeof(material.colorTint.vRegionA));
				memcpy(&material.colorTint.vRegionB, entry.regionTintB, sizeof(material.colorTint.vRegionB));
				memcpy(&material.colorTint.vRegionC, entry.regionTintC, sizeof(material.colorTint.vRegionC));
				material.colorTint.isEnabled = !material.colorMaskPath.empty();
			}
			// WMA2는 개별 PBR texture 경로를 채우고 color tint 기본값은 유지한다.
			else if (isV2)
			{
				const MATERIAL_ENTRY_V2 entry = reader.Read<MATERIAL_ENTRY_V2>();
				if (entry.materialIndex >= outMaterials.size())
				{
					outReport.error = "WMA2 contains an out-of-range material index.";
					return false;
				}

				MODEL_MATERIAL_DATA& material = outMaterials[entry.materialIndex];
				material.name = ReadFixedString(entry.name);
				material.nameHash = entry.materialHash;
				material.diffusePath = ResolveTexturePath(desc, containerPath, entry.baseColorPath);
				material.normalPath = ResolveTexturePath(desc, containerPath, entry.normalPath);
				material.specularPath = ResolveTexturePath(desc, containerPath, entry.specularPath);
				material.emissivePath = ResolveTexturePath(desc, containerPath, entry.emissivePath);
				material.opacityPath = ResolveTexturePath(desc, containerPath, entry.opacityPath);
				material.ormPath = ResolveTexturePath(desc, containerPath, entry.ormPath);
				material.metallicPath = ResolveTexturePath(desc, containerPath, entry.metallicPath);
				material.roughnessPath = ResolveTexturePath(desc, containerPath, entry.roughnessPath);
				material.ambientOcclusionPath =
					ResolveTexturePath(desc, containerPath, entry.ambientOcclusionPath);
			}
			// 최초 WMAT는 diffuse 경로만 제공한다. 나머지 표면 입력은 MODEL_MATERIAL_DATA의 기본값에서 시작한다.
			else
			{
				const MATERIAL_ENTRY entry = reader.Read<MATERIAL_ENTRY>();
				if (entry.materialIndex >= outMaterials.size())
				{
					outReport.error = "WMAT contains an out-of-range material index.";
					return false;
				}
				MODEL_MATERIAL_DATA& material = outMaterials[entry.materialIndex];
				material.name = ReadFixedString(entry.name);
				material.nameHash = entry.materialHash;
				material.diffusePath = ResolveTexturePath(desc, containerPath, entry.diffusePath);
			}
		}
	}
	// 파일/경로 해석 중 표준 예외는 진단으로 돌리지만 이미 채운 재질을 복원하지 않는다. 진입부 resize는 try 밖이다.
	catch (const exception& exception)
	{
		outReport.error = exception.what();
		return false;
	}

	// 모든 명시 행 해석에 성공한 뒤 빈 diffuse 슬롯에만 fallback을 적용한다. 실패 경로는 이 보완 단계에 도달하지 않는다.
	for (MODEL_MATERIAL_DATA& material : outMaterials)
	{
		if (material.diffusePath.empty() && !desc.fallbackDiffusePath.empty())
			material.diffusePath = desc.fallbackDiffusePath;
	}

	return true;
}
