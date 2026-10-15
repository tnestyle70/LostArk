// registry가 선택한 WMOD/분리 WMSH를 소유 CPU 모델로 해석한다.
#include "BinaryAsset/WModelDecoder.h"
// 골격이 없는 분리 모델의 bind-pose 경로에서 스킨 정점을 정적 정점으로 복사한다.
#include "Engine_VertexTypes.h"

// 파일 byte 배열을 소유한 뒤 범위를 검사하는 차용 커서로 컨테이너를 읽는다.
#include "BinaryAsset/BinaryReader.h"

// 검증된 골격에 clip 채널과 키를 연결한다.
#include "Winters/WAnimationReader.h"
// WINT/WMOD 헤더와 section 종류·버전·개수 제한을 사용한다.
#include "Winters/WFormatTypes.h"
// mesh가 요구하는 material 슬롯 수에 맞춰 CPU texture 경로를 해석한다.
#include "Winters/WMaterialReader.h"
// 정점·index와 mesh 본 대응 정보 및 geometry metadata를 읽는다.
#include "Winters/WMeshReader.h"
// 본 계층과 socket을 읽고 mesh의 본 순서와 연결한다.
#include "Winters/WSkeletonReader.h"

// 4-byte magic을 byte 단위로 비교한다.
#include <cstring>
// 목록용 clip 길이와 초당 tick의 유한성을 검사한다.
#include <cmath>
// animation section index 순으로 읽을 포인터 목록을 정렬한다.
#include <algorithm>
// 포맷 probe와 목록 metadata의 구간 읽기에 바이너리 stream을 사용한다.
#include <fstream>
// 완성된 clip 이름의 중복을 기록한다. package는 이름을 구분하고 분리 파일은 중복을 거절한다.
#include <unordered_set>

using namespace Engine::WintersFormat;

namespace
{
	// 호출자가 최소 4바이트의 유효 저장소를 제공한 뒤 사용하는 비교다. 이 helper 자체는 버퍼 크기를 받지 않는다.
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	// 분리 WMSH에서 골격/애니메이션을 명시하지 않았고 companion 골격도 없을 때만 사용하는 정적 bind-pose fallback이다.
	// 저장된 위치/기본 basis/UV0를 복사하며 bone skinning을 계산하지 않는다. 추가 UV는 이 복사에서 옮기지 않는다.
	void ConvertToStaticBindPose(vector<MODEL_MESH_DATA>& meshes)
	{
		for (MODEL_MESH_DATA& mesh : meshes)
		{
			mesh.vertices.resize(mesh.skinnedVertices.size());
			for (size_t i = 0; i < mesh.skinnedVertices.size(); ++i)
			{
				const VTXANIMMESH& source = mesh.skinnedVertices[i];
				VTXMESH& destination = mesh.vertices[i];
				destination.vPosition = source.vPosition;
				destination.vNormal = source.vNormal;
				destination.vTangent = source.vTangent;
				destination.vBinormal = source.vBinormal;
				destination.vTexcoord = source.vTexcoord;
			}
			mesh.skinnedVertices.clear();
			mesh.skinnedVertices.shrink_to_fit();
			mesh.vertexKind = MODEL_VERTEX_KIND::STATIC;
		}
	}

	// bytes가 소유한 package 내부를 가리키는 지역 view다. name만 소유하며 pData를 bytes 수명 밖에 보관하지 않는다.
	struct MODEL_SECTION_VIEW
	{
		MODEL_SECTION_TYPE type{};
		uint32_t index = {};
		string name;
		const uint8_t* pData = {nullptr};
		size_t size = {};
	};

	// 고정 폭 이름에서 첫 NUL 또는 capacity까지 복사한다. UTF-8 유효성이나 이름 의미는 이 helper가 검사하지 않는다.
	string ReadFixedName(const char_t* pName, size_t capacity)
	{
		size_t length = {};
		while (length < capacity && '\0' != pName[length])
			++length;
		return string(pName, length);
	}

	// clip 이름 중복은 section index와 추가 번호로 구분한다. 결과 이름 집합도 갱신하며 빈 이름은 상위에서 거절한다.
	string MakeUniqueAnimationName(const MODEL_SECTION_VIEW& section, unordered_set<string>& animationNames)
	{
		if (section.name.empty())
			return {};
		if (animationNames.emplace(section.name).second)
			return section.name;

		// MODEL_SECTION_DESC 이름 폭은 40바이트이므로 긴 ActorX clip 이름은 같은 접두사로 잘릴 수 있다.
		// package의 section index를 suffix로 사용해 충돌하는 이름의 clip도 남긴다.
		const string suffix = "#" + to_string(section.index);
		string candidate = section.name;
		if (candidate.size() + suffix.size() >= MAX_PATH)
			candidate.resize(MAX_PATH - suffix.size() - 1u);
		candidate += suffix;
		if (animationNames.emplace(candidate).second)
			return candidate;

		for (uint32_t discriminator = 1u;; ++discriminator)
		{
			const string fallbackSuffix = suffix + "_" + to_string(discriminator);
			candidate = section.name;
			if (candidate.size() + fallbackSuffix.size() >= MAX_PATH)
				candidate.resize(MAX_PATH - fallbackSuffix.size() - 1u);
			candidate += fallbackSuffix;
			if (animationNames.emplace(candidate).second)
				return candidate;
		}
	}

	// mesh와 skeleton의 본 개수/동일 index의 nameHash를 대조하고 inverse-bind를 옮긴다.
	// 중간 불일치 이전의 본은 이미 갱신될 수 있으므로 바깥 stagedAsset에 적용한다. parent/rest 행렬의 동일성을 비교하지는 않는다.
	bool_t ValidateMeshSkeleton(
		W_MESH_READ_RESULT& meshResult, MODEL_ASSET_DATA& outAsset, MODEL_DECODE_REPORT& outReport)
	{
		if (meshResult.bones.size() != outAsset.skeleton.bones.size())
		{
			outReport.error = "WMSH and WSKL bone counts do not match.";
			return false;
		}

		for (uint32_t i = 0; i < meshResult.bones.size(); ++i)
		{
			if (meshResult.bones[i].nameHash != outAsset.skeleton.bones[i].nameHash)
			{
				outReport.error = "WMSH and WSKL bone order or name hash does not match.";
				return false;
			}
			outAsset.skeleton.bones[i].inverseBind = meshResult.bones[i].inverseBind;
		}
		return true;
	}

	// 컨테이너 안의 mesh/material/skeleton/animation을 순서대로 읽는다. outAsset은 호출자의 stagedAsset이며 실패 시 부분 해석값은 바깥에서 폐기한다.
	bool_t DecodeWModelPackage(
		const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset, MODEL_DECODE_REPORT& outReport)
	{
		// 이 배열이 section view의 저장소를 소유한다. LoadFile의 할당 예외는 아래 try 범위 밖이다.
		vector<uint8_t> bytes;
		if (!CBinaryReader::LoadFile(desc.meshPath, bytes))
		{
			outReport.error = "Could not open the WModel package.";
			return false;
		}

		try
		{
			// 컨테이너 크기/버전을 먼저 확인한 뒤 WMOD metadata와 section 표로 진행한다.
			CBinaryReader fileReader(bytes.data(), bytes.size());
			const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
			if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) || WINT_VERSION_MAJOR != fileHeader.versionMajor ||
				fileHeader.versionMinor > WINT_SKINNED_BASIS_UV_VERSION_MINOR || 0 != fileHeader.flags ||
				fileHeader.contentSize != fileReader.Remaining())
			{
				outReport.error = "Invalid WINT WModel file header.";
				return false;
			}

			const uint8_t* pContent = fileReader.Peek();
			CBinaryReader reader(pContent, fileHeader.contentSize);
			const MODEL_META_HEADER modelHeader = reader.Read<MODEL_META_HEADER>();
			if (!HasMagic(modelHeader.magic, WMODEL_MAGIC) || modelHeader.sectionCount < 2 ||
				modelHeader.sectionCount > MAX_MODEL_SECTIONS ||
				modelHeader.animationCount > modelHeader.sectionCount)
			{
				outReport.error = "Invalid WMOD metadata.";
				return false;
			}

			// WMOD의 연속 배치 강화 검사는 minor 1~2에만 적용된다. WMSH reader가 적용하는 minor 4의 geometry 검사와 범위가 다르다.
			const bool_t geometryContract = WINT_GEOMETRY_VERSION_MINOR <= fileHeader.versionMinor &&
				fileHeader.versionMinor <= WINT_UV1_VERSION_MINOR;
			if (geometryContract &&
				(2 != modelHeader.sectionCount || 0 != modelHeader.animationCount || 0 != modelHeader.flags ||
				 0 != modelHeader.reserved[0] || 0 != modelHeader.reserved[1] || 0 != modelHeader.reserved[2] ||
				 0 != modelHeader.reserved[3]))
			{
				outReport.error = "WModel 1.1 requires canonical static WMOD metadata.";
				return false;
			}

			if (sizeof(MODEL_SECTION_DESC) > reader.Remaining() / modelHeader.sectionCount)
			{
				outReport.error = "WMOD section table is truncated.";
				return false;
			}

			// descriptor의 offset은 FILE_HEADER 다음 content 시작 기준 바이트 수다. sections의 포인터는 bytes를 계속 차용한다.
			vector<MODEL_SECTION_DESC> descriptors(modelHeader.sectionCount);
			reader.ReadBytes(descriptors.data(), descriptors.size() * sizeof(MODEL_SECTION_DESC));
			vector<MODEL_SECTION_VIEW> sections;
			sections.reserve(descriptors.size());
			uint32_t animationCount = {};
			size_t expectedGeometryOffset =
				sizeof(MODEL_META_HEADER) + descriptors.size() * sizeof(MODEL_SECTION_DESC);
			// 모든 버전에서 section 범위를 검사한다. 겹침/descriptor 영역 침범/연속성까지 공통으로 보장하는 검사는 아니다.
			for (const MODEL_SECTION_DESC& source : descriptors)
			{
				if (source.offset > fileHeader.contentSize ||
					source.size > fileHeader.contentSize - source.offset || source.size > SIZE_MAX)
				{
					outReport.error = "A WMOD section points outside the package.";
					return false;
				}

				if (geometryContract && (source.offset != expectedGeometryOffset || 0 == source.size))
				{
					outReport.error = "WModel 1.1 sections must be non-empty and contiguous.";
					return false;
				}
				if (geometryContract)
					expectedGeometryOffset += static_cast<size_t>(source.size);

				const MODEL_SECTION_TYPE type = static_cast<MODEL_SECTION_TYPE>(source.type);
				if (type < MODEL_SECTION_TYPE::MESH || type > MODEL_SECTION_TYPE::ANIMATION)
				{
					outReport.error = "WMOD contains an unknown section type.";
					return false;
				}
				if (MODEL_SECTION_TYPE::ANIMATION == type)
					++animationCount;

				sections.push_back(
					MODEL_SECTION_VIEW{
						type,
						source.index,
						ReadFixedName(source.name, sizeof(source.name)),
						pContent + static_cast<size_t>(source.offset),
						static_cast<size_t>(source.size)});
			}

			if (geometryContract && expectedGeometryOffset != fileHeader.contentSize)
			{
				outReport.error = "WModel 1.1 contains trailing section payload.";
				return false;
			}

			if (animationCount != modelHeader.animationCount)
			{
				outReport.error = "WMOD animation count does not match its section table.";
				return false;
			}

			// 해당 종류가 없거나 두 개 이상이면 nullptr다. 반환 주소는 지역 sections vector가 재할당되지 않는 동안만 유효하다.
			const auto FindSection = [&](MODEL_SECTION_TYPE type) -> const MODEL_SECTION_VIEW*
			{
				const MODEL_SECTION_VIEW* pResult = {nullptr};
				for (const MODEL_SECTION_VIEW& section : sections)
				{
					if (section.type != type)
						continue;
					if (nullptr != pResult)
						return nullptr;
					pResult = &section;
				}
				return pResult;
			};

			const MODEL_SECTION_VIEW* pMesh = FindSection(MODEL_SECTION_TYPE::MESH);
			const MODEL_SECTION_VIEW* pMaterial = FindSection(MODEL_SECTION_TYPE::MATERIAL);
			if (nullptr == pMesh || nullptr == pMaterial)
			{
				outReport.error = "WMOD requires exactly one mesh and one material section.";
				return false;
			}

			// mesh의 버전과 material 최소 슬롯 수를 연결한다. texture GPU 생성이나 source provenance 인증은 여기서 수행하지 않는다.
			W_MESH_READ_RESULT meshResult{};
			if (!CWMeshReader{}.ReadMemory(pMesh->pData, pMesh->size, meshResult, outReport))
				return false;
			if (meshResult.fileVersionMinor != fileHeader.versionMinor)
			{
				outReport.error = "WMOD and WMSH geometry format versions do not match.";
				return false;
			}
			if (!CWMaterialReader{}.ReadMemory(
					pMaterial->pData,
					pMaterial->size,
					desc,
					desc.meshPath,
					meshResult.materialCount,
					outAsset.materials,
					outReport))
				return false;

			outAsset.meshes = move(meshResult.meshes);
			outAsset.geometryMetadata = meshResult.geometryMetadata;

			// 스킨 모델만 골격/clip을 결합한다. 이 시점의 outAsset은 Decode가 최종 전달하기 전의 지역 후보다.
			const MODEL_SECTION_VIEW* pSkeleton = FindSection(MODEL_SECTION_TYPE::SKELETON);
			if (meshResult.skinned)
			{
				if (nullptr == pSkeleton ||
					!CWSkeletonReader{}.ReadMemory(
						pSkeleton->pData, pSkeleton->size, outAsset.skeleton, outReport))
				{
					if (outReport.error.empty())
						outReport.error = "A skinned WMOD requires one skeleton section.";
					return false;
				}
				if (!ValidateMeshSkeleton(meshResult, outAsset, outReport))
					return false;
				outAsset.hasSkeleton = true;

				// animation을 section index 순으로 읽고 이름을 구분한다. 중복 index 자체를 금지하거나 동률의 sort 순서를 고정하는 검사는 없다.
				vector<const MODEL_SECTION_VIEW*> animations;
				for (const MODEL_SECTION_VIEW& section : sections)
				{
					if (MODEL_SECTION_TYPE::ANIMATION == section.type)
						animations.push_back(&section);
				}
				sort(animations.begin(), animations.end(), [](const auto* pLeft, const auto* pRight)
				{
					return pLeft->index < pRight->index;
				});

				unordered_set<string> animationNames;
				outAsset.animations.reserve(animations.size());
				for (const MODEL_SECTION_VIEW* pAnimation : animations)
				{
					const string animationName = MakeUniqueAnimationName(*pAnimation, animationNames);
					if (animationName.empty())
					{
						outReport.error = "WMOD contains an unnamed animation clip.";
						return false;
					}

					MODEL_ANIMATION_DATA animation{};
					if (!CWAnimationReader{}.ReadMemory(
							pAnimation->pData,
							pAnimation->size,
							animationName,
							outAsset.skeleton,
							animation,
							outReport))
						return false;
					outAsset.animations.push_back(move(animation));
				}
			}
			else if (nullptr != pSkeleton || 0 != animationCount)
			{
				outReport.error = "A static WMOD cannot contain skeleton or animation sections.";
				return false;
			}

			// CPU 해석 성공 통계를 기록한다. texture/GPU 생성 성공은 CModel의 후속 준비 단계가 판단한다.
			outReport.usedSkinnedBindPose = meshResult.skinned && outAsset.animations.empty();
			outReport.meshCount = static_cast<uint32_t>(outAsset.meshes.size());
			outReport.materialCount = static_cast<uint32_t>(outAsset.materials.size());
			outReport.boneCount = static_cast<uint32_t>(outAsset.skeleton.bones.size());
			outReport.animationCount = static_cast<uint32_t>(outAsset.animations.size());
			outReport.hasGeometryMetadata = outAsset.geometryMetadata.present;
			outReport.geometryEvidenceFlags = outAsset.geometryMetadata.evidenceFlags;
			outReport.succeeded = true;
			return true;
		}
		// try 안의 표준 예외를 실패 진단으로 바꾼다. 이미 만든 부분 outAsset/report는 여기서 초기화하지 않는다. LoadFile은 try 밖이다.
		catch (const exception& exception)
		{
			outReport.error = exception.what();
			return false;
		}
	}
}

// registry 보고서에 들어갈 정적 리터럴이다. 호출자는 반환 문자열을 해제하지 않는다.
const char_t* CWModelDecoder::Get_Name() const
{
	return "Winters WINT model package v1/v1.1 geometry";
}

// 처음 20바이트의 WINT와 WMSH/WMOD magic만 확인한다. 버전/범위/나머지 payload 검증은 Decode가 수행한다.
bool_t CWModelDecoder::CanDecode(const filesystem::path& meshPath) const
{
	ifstream stream(meshPath, ios::binary);
	if (!stream)
		return false;

	char_t probe[20]{};
	if (!stream.read(probe, sizeof(probe)))
		return false;

	return HasMagic(probe, WINTERS_MAGIC) &&
		(HasMagic(probe + sizeof(FILE_HEADER), WMESH_MAGIC) ||
		 HasMagic(probe + sizeof(FILE_HEADER), WMODEL_MAGIC));
}

// Animation Tool의 목록 표시에 필요한 이름과 native tick 메타데이터만 seek/read한다. geometry·key·skeleton의 재생 가능성 검증은 아니다.
// 지역 목록을 완성한 뒤 outEntries를 교체하므로 명시적 false에서는 이전 목록을 보존하고 outStatus만 바꾼다.
bool_t CWModelDecoder::Read_AnimationCatalog(
	const filesystem::path& modelPath, vector<MODEL_ANIMATION_CATALOG_ENTRY>& outEntries, string& outStatus)
{
	try
	{
		ifstream stream(modelPath, ios::binary | ios::ate);
		const streamoff fileSize = stream ? static_cast<streamoff>(stream.tellg()) : -1;
		if (fileSize < static_cast<streamoff>(sizeof(FILE_HEADER) + sizeof(MODEL_META_HEADER)))
		{
			outStatus = "Could not open the WModel package or its header is truncated.";
			return false;
		}

		// 파일 전체를 메모리에 올리지 않고 파일 시작 기준 offset에서 요청 구간만 읽는다. 범위와 실제 read 성공을 함께 검사한다.
		const auto ReadAt = [&](uint64_t offset, void* pDestination, size_t size) -> bool_t
		{
			if (offset > static_cast<uint64_t>(fileSize) || size > static_cast<uint64_t>(fileSize) - offset)
				return false;
			stream.seekg(static_cast<streamoff>(offset), ios::beg);
			return static_cast<bool_t>(
				stream.read(reinterpret_cast<char*>(pDestination), static_cast<streamsize>(size)));
		};


		// 파일 전체를 읽지 않고 컨테이너 헤더와 section 표의 위치/개수부터 확인한다.
		FILE_HEADER fileHeader{};
		if (!ReadAt(0, &fileHeader, sizeof(fileHeader)) || !HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
			WINT_VERSION_MAJOR != fileHeader.versionMajor ||
			fileHeader.versionMinor > WINT_SKINNED_BASIS_UV_VERSION_MINOR || 0 != fileHeader.flags ||
			fileHeader.contentSize != static_cast<uint64_t>(fileSize) - sizeof(FILE_HEADER))
		{
			outStatus = "Invalid WINT WModel file header.";
			return false;
		}
		MODEL_META_HEADER modelHeader{};
		if (!ReadAt(sizeof(FILE_HEADER), &modelHeader, sizeof(modelHeader)) ||
			!HasMagic(modelHeader.magic, WMODEL_MAGIC) || modelHeader.sectionCount < 2 ||
			modelHeader.sectionCount > MAX_MODEL_SECTIONS ||
			modelHeader.animationCount > modelHeader.sectionCount)
		{
			outStatus = "Invalid WMOD metadata.";
			return false;
		}
		const uint64_t tableBytes =
			static_cast<uint64_t>(modelHeader.sectionCount) * sizeof(MODEL_SECTION_DESC);
		if (tableBytes > fileHeader.contentSize - sizeof(MODEL_META_HEADER))
		{
			outStatus = "WMOD section table is truncated.";
			return false;
		}
		vector<MODEL_SECTION_DESC> descriptors(modelHeader.sectionCount);
		if (!ReadAt(
				sizeof(FILE_HEADER) + sizeof(MODEL_META_HEADER),
				descriptors.data(),
				static_cast<size_t>(tableBytes)))
		{
			outStatus = "Could not read the WMOD section table.";
			return false;
		}

		// section 범위와 종류를 대조하면서 목록에 필요한 animation descriptor만 차용한다.
		vector<const MODEL_SECTION_DESC*> animations;
		animations.reserve(modelHeader.animationCount);
		for (const MODEL_SECTION_DESC& section : descriptors)
		{
			if (section.offset > fileHeader.contentSize ||
				section.size > fileHeader.contentSize - section.offset)
			{
				outStatus = "A WMOD section points outside the package.";
				return false;
			}
			const MODEL_SECTION_TYPE type = static_cast<MODEL_SECTION_TYPE>(section.type);
			if (type < MODEL_SECTION_TYPE::MESH || type > MODEL_SECTION_TYPE::ANIMATION)
			{
				outStatus = "WMOD contains an unknown section type.";
				return false;
			}
			if (MODEL_SECTION_TYPE::ANIMATION == type)
				animations.push_back(&section);
		}
		if (animations.size() != modelHeader.animationCount)
		{
			outStatus = "WMOD animation count does not match its section table.";
			return false;
		}

		// Decode와 같은 section index 정렬 및 잘린 이름의 중복 해소 규칙을 사용한다.
		sort(animations.begin(), animations.end(), [](const auto* pLeft, const auto* pRight)
		{
			return pLeft->index < pRight->index;
		});
		unordered_set<string> animationNames;
		vector<MODEL_ANIMATION_CATALOG_ENTRY> stagedEntries;
		stagedEntries.reserve(animations.size());
		for (const MODEL_SECTION_DESC* pAnimation : animations)
		{
			MODEL_SECTION_VIEW nameSource{};
			nameSource.index = pAnimation->index;
			nameSource.name = ReadFixedName(pAnimation->name, sizeof(pAnimation->name));
			const string animationName = MakeUniqueAnimationName(nameSource, animationNames);
			if (animationName.empty())
			{
				outStatus = "WMOD contains an unnamed animation clip.";
				return false;
			}

			// 이름을 확정한 clip의 헤더·native 시간만 검사한다. 채널과 key payload는 목록 요청에서 읽지 않는다.
			FILE_HEADER animationFileHeader{};
			ANIMATION_META_HEADER animationHeader{};
			const uint64_t animationOffset = sizeof(FILE_HEADER) + pAnimation->offset;
			if (pAnimation->size < sizeof(FILE_HEADER) + sizeof(ANIMATION_META_HEADER) ||
				!ReadAt(animationOffset, &animationFileHeader, sizeof(animationFileHeader)) ||
				!HasMagic(animationFileHeader.magic, WINTERS_MAGIC) ||
				WINT_VERSION_MAJOR != animationFileHeader.versionMajor || 0 != animationFileHeader.flags ||
				animationFileHeader.contentSize < sizeof(ANIMATION_META_HEADER) ||
				animationFileHeader.contentSize > pAnimation->size - sizeof(FILE_HEADER))
			{
				outStatus = "Invalid WINT animation file header: " + animationName;
				return false;
			}
			if (!ReadAt(animationOffset + sizeof(FILE_HEADER), &animationHeader, sizeof(animationHeader)) ||
				!HasMagic(animationHeader.magic, WANIM_MAGIC) ||
				animationHeader.channelCount > MAX_ANIMATION_CHANNELS ||
				animationHeader.totalKeyCount > MAX_ANIMATION_KEYS ||
				animationHeader.eventCount > MAX_ANIMATION_EVENTS || !isfinite(animationHeader.durationTicks) ||
				animationHeader.durationTicks <= 0.f || !isfinite(animationHeader.ticksPerSecond) ||
				animationHeader.ticksPerSecond <= 0.f)
			{
				outStatus = "Invalid WANM metadata: " + animationName;
				return false;
			}
			stagedEntries.push_back(
				MODEL_ANIMATION_CATALOG_ENTRY{
					animationName, animationHeader.durationTicks, animationHeader.ticksPerSecond});
		}

		// 모든 clip metadata가 통과한 목록을 한 번에 넘기고 이전 실패 상태를 지운다.
		outEntries = move(stagedEntries);
		outStatus.clear();
		return true;
	}
	catch (const exception& exception)
	{
		outStatus = exception.what();
		return false;
	}
}

// registry가 선택한 포맷을 완성된 CPU asset으로 전달한다. 시작 시 이전 출력을 비운 뒤 package/분리 파일 경로 모두 지역 후보를 사용한다.
bool_t CWModelDecoder::Decode(
	const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset, MODEL_DECODE_REPORT& outReport) const
{
	outAsset = {};
	MODEL_ASSET_DATA stagedAsset{};
	outReport.meshPath = desc.meshPath;
	outReport.decoderName = Get_Name();

	ifstream stream(desc.meshPath, ios::binary);
	char_t probe[20]{};
	if (!stream.read(probe, sizeof(probe)))
	{
		outReport.error = "Could not probe the Winters model asset.";
		return false;
	}
	// WMOD는 내부 section을 사용한다. 이 경로가 아니면 아래 companion 파일 로드를 진행한다.
	if (HasMagic(probe + sizeof(FILE_HEADER), WMODEL_MAGIC))
	{
		if (!DecodeWModelPackage(desc, stagedAsset, outReport))
			return false;
		outAsset = move(stagedAsset);
		return true;
	}

	// 분리 파일은 mesh의 최대 material index+1을 최소 재질 슬롯 수로 전달한다. 누락 companion 재질은 reader의 기본 재질 정책을 따른다.
	W_MESH_READ_RESULT meshResult{};
	if (!CWMeshReader{}.Read(desc.meshPath, meshResult, outReport))
		return false;

	if (!CWMaterialReader{}.Read(desc, meshResult.materialCount, stagedAsset.materials, outReport))
		return false;

	stagedAsset.meshes = move(meshResult.meshes);
	stagedAsset.geometryMetadata = meshResult.geometryMetadata;
	bool_t usedBindPoseFallback = false;
	if (meshResult.skinned)
	{
		filesystem::path skeletonPath = desc.skeletonPath;
		if (skeletonPath.empty())
		{
			skeletonPath = desc.meshPath;
			skeletonPath.replace_extension(L".wskel");
		}

		// 골격/clip을 명시한 요청은 누락 골격을 정적 모델로 숨기지 않는다. 아무 의존성도 요청하지 않은 경우에만 bind-pose fallback을 허용한다.
		const bool_t skeletonWasRequested = !desc.skeletonPath.empty() || !desc.animationPaths.empty();
		if (!filesystem::exists(skeletonPath) && !skeletonWasRequested)
		{
			ConvertToStaticBindPose(stagedAsset.meshes);
			meshResult.skinned = false;
			usedBindPoseFallback = true;
		}
		else if (!CWSkeletonReader{}.Read(skeletonPath, stagedAsset.skeleton, outReport))
		{
			return false;
		}

		if (!meshResult.skinned)
		{
			outReport.usedSkinnedBindPose = true;
		}
		else
		{
			// 분리 파일도 본 index 순서를 맞춘 뒤 inverse-bind를 결합한다. 모델의 local rest pose를 inverse-bind로 덮어쓰지 않는다.
			if (meshResult.bones.size() != stagedAsset.skeleton.bones.size())
			{
				outReport.error = "WMSH and WSKL bone counts do not match.";
				return false;
			}

			for (uint32_t i = 0; i < meshResult.bones.size(); ++i)
			{
				if (meshResult.bones[i].nameHash != stagedAsset.skeleton.bones[i].nameHash)
				{
					outReport.error = "WMSH and WSKL bone order or name hash does not match.";
					return false;
				}
				stagedAsset.skeleton.bones[i].inverseBind = meshResult.bones[i].inverseBind;
			}
			stagedAsset.hasSkeleton = true;

			// 상대 animation 경로는 mesh 옆 기준이다. 각 clip은 같은 skeletonHash/본 채널을 검사하며 분리 파일의 중복 stem 이름은 거절한다.
			unordered_set<string> animationNames;
			stagedAsset.animations.reserve(desc.animationPaths.size());
			for (filesystem::path animationPath : desc.animationPaths)
			{
				if (animationPath.is_relative())
					animationPath = desc.meshPath.parent_path() / animationPath;

				MODEL_ANIMATION_DATA animation{};
				if (!CWAnimationReader{}.Read(
						animationPath.lexically_normal(), stagedAsset.skeleton, animation, outReport))
					return false;
				if (!animationNames.emplace(animation.name).second)
				{
					outReport.error = "The model package contains duplicate animation clip names.";
					return false;
				}
				stagedAsset.animations.push_back(move(animation));
			}
		}
	}
	else if (!desc.skeletonPath.empty() || !desc.animationPaths.empty())
	{
		outReport.error = "A static WMSH cannot declare skeleton or animation dependencies.";
		return false;
	}

	// 전체 성공 통계와 asset을 최종 반영한다. report의 vertex/index/key 누적값은 하위 reader에서 이미 갱신된다.
	outReport.usedSkinnedBindPose =
		usedBindPoseFallback || (meshResult.skinned && stagedAsset.animations.empty());
	outReport.meshCount = static_cast<uint32_t>(stagedAsset.meshes.size());
	outReport.materialCount = static_cast<uint32_t>(stagedAsset.materials.size());
	outReport.boneCount = static_cast<uint32_t>(stagedAsset.skeleton.bones.size());
	outReport.animationCount = static_cast<uint32_t>(stagedAsset.animations.size());
	outReport.hasGeometryMetadata = stagedAsset.geometryMetadata.present;
	outReport.geometryEvidenceFlags = stagedAsset.geometryMetadata.evidenceFlags;
	outReport.succeeded = true;
	outAsset = move(stagedAsset);
	return true;
}
