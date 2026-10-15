#pragma once

// 디스크 ABI의 정수 폭을 host의 기본 int 크기와 분리한다.
#include <cstdint>

// WINT 계열의 디스크 ABI다. reader는 pack(1) 구조체를 memcpy하며 endian 변환을 하지 않는다.
// Windows의 2바이트 wchar_t를 포함한 크기는 아래 static_assert로 고정한다. C++ 메모리 소유 모델은 ModelAssetData.h에 따로 둔다.
namespace Engine::WintersFormat
{
	// 바깥 WINT와 내부 콘텐츠 종류를 4바이트로 식별한다. 각 reader가 자기 종류의 magic을 확인한다.
	constexpr char WINTERS_MAGIC[4] = {'W', 'I', 'N', 'T'};
	constexpr char WMESH_MAGIC[4] = {'W', 'M', 'S', 'H'};
	constexpr char WMAT_MAGIC[4] = {'W', 'M', 'A', 'T'};
	constexpr char WMAT_V2_MAGIC[4] = {'W', 'M', 'A', '2'};
	constexpr char WMAT_V3_MAGIC[4] = {'W', 'M', 'A', '3'};
	constexpr char WSKEL_MAGIC[4] = {'W', 'S', 'K', 'L'};
	constexpr char WANIM_MAGIC[4] = {'W', 'A', 'N', 'M'};
	constexpr char WMODEL_MAGIC[4] = {'W', 'M', 'O', 'D'};
	constexpr char WGEOMETRY_MAGIC[4] = {'W', 'G', 'E', 'O'};

	// mesh reader가 minor별 정점/추가 채널 layout을 선택하는 버전이다. 다른 reader의 허용 minor 정책은 각 구현에 있다.
	constexpr uint16_t WINT_VERSION_MAJOR = 1;
	constexpr uint16_t WINT_LEGACY_VERSION_MINOR = 0;
	constexpr uint16_t WINT_GEOMETRY_VERSION_MINOR = 1;
	constexpr uint16_t WINT_UV1_VERSION_MINOR = 2;
	constexpr uint16_t WINT_SKINNED_UV_VERSION_MINOR = 3;
	constexpr uint16_t WINT_STATIC_UV2_VERSION_MINOR = 4;
	constexpr uint16_t WINT_SKINNED_BASIS_VERSION_MINOR = 5;
	// 1.6은 80바이트 signed basis와 기존 WUVS tail을 함께 사용한다.
	constexpr uint16_t WINT_SKINNED_BASIS_UV_VERSION_MINOR = 6;

	// 정점 채널 bit다. stride와 version을 함께 검증해야 실제 저장 순서를 결정할 수 있다.
	constexpr uint32_t VF_POSITION = 1u << 0;
	constexpr uint32_t VF_NORMAL = 1u << 1;
	constexpr uint32_t VF_TEXCOORD0 = 1u << 2;
	constexpr uint32_t VF_TANGENT = 1u << 3;
	constexpr uint32_t VF_BONE_WEIGHT = 1u << 4;
	constexpr uint32_t VF_TANGENT_HANDEDNESS = 1u << 5;
	constexpr uint32_t VF_COLOR0 = 1u << 6;
	constexpr uint32_t VF_TEXCOORD1 = 1u << 7;
	constexpr uint32_t VF_TEXCOORD2 = 1u << 8;
	constexpr uint32_t VF_STATIC_BASE = VF_POSITION | VF_NORMAL | VF_TEXCOORD0 | VF_TANGENT;

	// packed 기본 정점 한 개의 byte 수다. 정적 추가 채널과 스킨 UV tail은 reader가 별도로 계산한다.
	constexpr uint32_t STRIDE_STATIC = 48;
	constexpr uint32_t STRIDE_STATIC_COLOR0 = 52;
	constexpr uint32_t STRIDE_SKINNED = 76;
	// WMSH 1.5는 기존 76바이트 skin 데이터 뒤에 tangent handedness를 추가한다.
	constexpr uint32_t STRIDE_SKINNED_BASIS = 80;
	constexpr uint32_t SHA256_SIZE = 32;

	// 입력 메타데이터의 개수 상한이다. 각 reader는 이 값과 실제 남은 바이트를 함께 확인한다.
	constexpr uint32_t MAX_SUBMESHES = 2048;
	constexpr uint32_t MAX_BONES = 512;
	constexpr uint32_t MAX_SOCKETS = 256;
	constexpr uint32_t MAX_VERTICES = 10'000'000;
	constexpr uint32_t MAX_MATERIALS = 4096;
	constexpr uint32_t MAX_ANIMATION_CHANNELS = 1024;
	constexpr uint32_t MAX_ANIMATION_KEYS = 1'000'000;
	constexpr uint32_t MAX_ANIMATION_EVENTS = 100'000;
	constexpr uint32_t MAX_MODEL_SECTIONS = 4096;

	// WMOD section 표가 mesh·재질·골격·animation reader를 선택하는 저장값이다.
	enum class MODEL_SECTION_TYPE : uint32_t
	{
		MESH = 1,
		MATERIAL = 2,
		SKELETON = 3,
		ANIMATION = 4,
	};

#pragma pack(push, 1)

	// 공통 파일 header다. contentSize는 이 header 뒤 payload의 byte 수이며 header 자신의 크기는 포함하지 않는다.
	struct FILE_HEADER
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t flags;
		uint32_t contentSize;
	};

	// WMOD의 section 표 크기와 animation 개수를 선언한다. 실제 section 범위와 중복 종류 검증은 decoder가 맡는다.
	struct MODEL_META_HEADER
	{
		char magic[4];
		uint32_t sectionCount;
		uint32_t animationCount;
		uint32_t flags;
		uint32_t reserved[4];
	};

	// WMOD content 시작 기준의 section byte offset/size다. 고정 이름 폭은 40바이트이며 runtime은 중복 이름에 suffix를 붙일 수 있다.
	struct MODEL_SECTION_DESC
	{
		uint32_t type;
		uint32_t index;
		uint64_t offset;
		uint64_t size;
		char name[40];
	};

	// 전체 정점/index 수와 stride·channel mask를 연결한다. hasBounding이 있으면 본 배열 뒤에 submesh별 bounds가 온다.
	struct MESH_META_HEADER
	{
		char magic[4];
		uint32_t submeshCount;
		uint32_t boneCount;
		uint32_t vertexFormatFlags;
		uint32_t vertexStride;
		uint32_t totalVertexCount;
		uint32_t totalIndexCount;
		uint32_t indexStride;
		uint8_t hasBounding;
		uint8_t reserved[3];
	};

	// vertex/index offset은 각 blob 시작 기준 바이트 수다. materialIndex는 slot 번호이며 materialHash를 슬롯 탐색 키로 쓰지 않는다.
	struct SUBMESH_DESC
	{
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[20];
	};

	// mesh 정점이 참조하는 본 순서와 inverse-bind를 저장한다. parent 계층은 별도 WSKL에서 결합한다.
	struct MESH_BONE_ENTRY
	{
		uint64_t nameHash;
		char name[32];
		int32_t parentIndex;
		float offsetMatrix[16];
		uint32_t channelFlag;
		uint8_t reserved[16];
	};

	// WModel 공간의 AABB와 그 중심을 기준으로 한 구다. 정적 geometry reader가 실제 정점으로 값을 다시 대조한다.
	struct MESH_BOUNDS_V1
	{
		float minimum[3];
		float maximum[3];
		float center[3];
		float radius;
	};

	// flag는 파일이 주장하는 보존/변환 증거 종류다. digest 자체 검증과 원본 데이터의 외부 인증을 구분한다.
	enum MESH_GEOMETRY_EVIDENCE_FLAG : uint32_t
	{
		MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF = 1u << 0,
		MGEF_COLOR0_PRESERVED_FROM_GLTF = 1u << 1,
		MGEF_BOUNDS_WMODEL_SPACE = 1u << 2,
		MGEF_SOURCE_GLTF_SHA256 = 1u << 3,
		MGEF_SOURCE_BUFFER_SET_SHA256 = 1u << 4,
		MGEF_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 = 1u << 5,
		MGEF_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED = 1u << 6,
		MGEF_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 = 1u << 7,
		MGEF_GEOMETRY_TOOL_SHA256 = 1u << 8,
		MGEF_SOURCE_EXPORT_RECEIPT_SHA256 = 1u << 9,
		MGEF_LEGACY_COOK_RECEIPT_SHA256 = 1u << 10,
		MGEF_CLEAN_SOURCE_EXPORT = 1u << 11,
		MGEF_UPK_TO_GLTF_EXACT = 1u << 12,
		MGEF_PIVOT_EXACT = 1u << 13,
		MGEF_TEXCOORD1_PRESERVED_FROM_GLTF = 1u << 14,
		MGEF_TANGENT_HANDEDNESS_PROJECT_RECONSTRUCTED = 1u << 15,
		MGEF_NATIVE_PARALLEL_BASIS_PRESERVED = 1u << 16,
		MGEF_TEXCOORD2_PRESERVED_FROM_GLTF = 1u << 17,
	};

	constexpr uint32_t MGEF_REQUIRED_PAYLOAD = MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF |
		MGEF_BOUNDS_WMODEL_SPACE | MGEF_SOURCE_GLTF_SHA256 | MGEF_SOURCE_BUFFER_SET_SHA256 |
		MGEF_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 | MGEF_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED |
		MGEF_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 | MGEF_GEOMETRY_TOOL_SHA256 |
		MGEF_SOURCE_EXPORT_RECEIPT_SHA256 | MGEF_LEGACY_COOK_RECEIPT_SHA256;
	constexpr uint32_t MGEF_PRODUCT_PROVENANCE =
		MGEF_CLEAN_SOURCE_EXPORT | MGEF_UPK_TO_GLTF_EXACT | MGEF_PIVOT_EXACT;
	constexpr uint32_t MGEF_KNOWN = MGEF_REQUIRED_PAYLOAD | MGEF_COLOR0_PRESERVED_FROM_GLTF |
		MGEF_TEXCOORD1_PRESERVED_FROM_GLTF | MGEF_TANGENT_HANDEDNESS_PROJECT_RECONSTRUCTED |
		MGEF_NATIVE_PARALLEL_BASIS_PRESERVED | MGEF_TEXCOORD2_PRESERVED_FROM_GLTF | MGEF_PRODUCT_PROVENANCE;

	// payload digest는 앞선 geometry bytes, metadata digest는 metadataSha256 필드 이전 bytes를 덮는다.
	struct MESH_GEOMETRY_METADATA_V1
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t byteSize;
		uint32_t evidenceFlags;
		uint32_t payloadSize;
		float sourceToWModelScale;
		float geometryPreScale;
		uint8_t payloadSha256[SHA256_SIZE];
		uint8_t sourceGltfSha256[SHA256_SIZE];
		uint8_t sourceBufferSetSha256[SHA256_SIZE];
		uint8_t sourcePackageObservedUnboundSha256[SHA256_SIZE];
		uint8_t sourceObjectPathHash[SHA256_SIZE];
		uint8_t legacyConverterObservedUnboundSha256[SHA256_SIZE];
		uint8_t geometryToolSha256[SHA256_SIZE];
		uint8_t sourceExportReceiptSha256[SHA256_SIZE];
		uint8_t legacyCookReceiptSha256[SHA256_SIZE];
		uint8_t metadataSha256[SHA256_SIZE];
	};

	// WMSH 1.3은 기존 76바이트 skin stream을 유지한다. 마지막 UV payload는
	// submesh별 {vertexCount, channelMask, UV1[], UV2[]} 순서다.
	struct MESH_SKINNED_UV_HEADER
	{
		char magic[4];
		uint32_t version;
		uint32_t submeshCount;
		uint32_t payloadSize;
		uint8_t payloadSha256[SHA256_SIZE];
	};

	// magic이 재질 행 버전을 정하고 materialCount가 뒤따르는 행 수를 정한다. slot 번호는 각 행이 가진다.
	struct MATERIAL_META_HEADER
	{
		char magic[4];
		uint32_t materialCount;
	};

	// 초기 WMAT는 diffuse 경로 하나를 저장한다. 경로 폭은 wchar_t 260개이며 실제 파일 탐색은 material reader가 수행한다.
	struct MATERIAL_ENTRY
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t diffusePath[260];
	};

	// WMA2는 표면 채널별 texture 경로를 분리한다. GPU texture 객체를 이 ABI에 저장하지 않는다.
	struct MATERIAL_ENTRY_V2
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t baseColorPath[260];
		wchar_t normalPath[260];
		wchar_t specularPath[260];
		wchar_t emissivePath[260];
		wchar_t opacityPath[260];
		wchar_t ormPath[260];
		wchar_t metallicPath[260];
		wchar_t roughnessPath[260];
		wchar_t ambientOcclusionPath[260];
	};

	// V2에 원본의 색상 영역 입력을 추가한다. color mask의 채널과 영역별 세 tint, 전체 diffuse 배율을 저장한다.
	// 빈 mask와 모두 1인 tint는 기본값이며 실제 색상 적용은 CMaterial/shader가 담당한다.
	struct MATERIAL_ENTRY_V3
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t baseColorPath[260];
		wchar_t normalPath[260];
		wchar_t specularPath[260];
		wchar_t emissivePath[260];
		wchar_t opacityPath[260];
		wchar_t ormPath[260];
		wchar_t metallicPath[260];
		wchar_t roughnessPath[260];
		wchar_t ambientOcclusionPath[260];
		wchar_t colorMaskPath[260];
		float diffuseTint[4];
		float regionTintA[4];
		float regionTintB[4];
		float regionTintC[4];
	};

	// 본 배열과 socket 배열의 원소 수다. root 행렬은 본 배열과 socket 배열 사이에 한 개 저장한다.
	struct SKELETON_META_HEADER
	{
		char magic[4];
		uint32_t boneCount;
		uint32_t socketCount;
		uint32_t reserved[5];
	};

	// local rest pose와 parent index를 저장한다. 현재 reader는 parent-before-child를 검사하고 childCount/firstChildIndex는 소비하지 않는다.
	struct SKELETON_BONE_NODE
	{
		uint64_t nameHash;
		char name[64];
		int32_t parentIndex;
		float restTransform[16];
		uint32_t childCount;
		uint32_t firstChildIndex;
		uint32_t reserved[27];
	};

	// 골격의 전역 역루트 변환을 값으로 저장한다. reader는 행렬 성분의 유한성을 확인한다.
	struct GLOBAL_ROOT_MATRIX
	{
		float globalInverseRoot[16];
		uint32_t reserved[16];
	};

	// 부모 본 index와 본 기준 local offset이다. 실제 부착 위치는 runtime이 본 변환과 합성한다.
	struct SOCKET_ENTRY
	{
		char name[32];
		uint64_t nameHash;
		int32_t parentBoneIndex;
		float localOffset[16];
		uint32_t reserved[5];
	};

	// durationTicks/key timeTicks는 native tick 단위다. ticksPerSecond는 초 환산용 metadata이며 실제 재생 시계 정책은 CAnimation이 따로 정한다.
	struct ANIMATION_META_HEADER
	{
		char magic[4];
		uint32_t channelCount;
		float durationTicks;
		float ticksPerSecond;
		uint32_t totalKeyCount;
		uint32_t eventCount;
		uint8_t isLoop;
		uint8_t reserved[7];
	};

	// 각 key offset은 channel table 뒤 key block 시작 기준 바이트 수다. cachedBoneIndex는 hash에 대한 보조 검증값이다.
	struct ANIMATION_CHANNEL
	{
		uint64_t boneNameHash;
		uint32_t positionKeyCount;
		uint32_t positionOffset;
		uint32_t rotationKeyCount;
		uint32_t rotationOffset;
		uint32_t scaleKeyCount;
		uint32_t scaleOffset;
		int32_t cachedBoneIndex;
		uint32_t reserved;
	};

	// 위치 또는 크기 채널의 native tick과 세 성분이다. 단위 변환을 이 저장 구조에서 수행하지 않는다.
	struct VECTOR_KEY
	{
		float timeTicks;
		float x;
		float y;
		float z;
	};

	// 회전 채널의 native tick과 quaternion 네 성분이다. animation reader가 길이를 검사하고 정규화한다.
	struct QUATERNION_KEY
	{
		float timeTicks;
		float x;
		float y;
		float z;
		float w;
	};

	// event payload는 그대로 CPU 데이터로 옮긴다. reader가 type별 gameplay 의미를 실행하거나 검증하지 않는다.
	struct ANIMATION_EVENT
	{
		float timeTicks;
		uint16_t type;
		uint16_t reserved0;
		uint32_t skillId;
		uint32_t paramU32;
		float paramF32;
		uint64_t stringHash;
		uint32_t reserved1;
	};

	// 골격 본 nameHash 순서를 축약한 결합값이다. 골격의 모든 행렬과 부모 관계까지 포함하는 내용 digest는 아니다.
	struct ANIMATION_TRAILER
	{
		uint64_t skeletonHash;
	};

#pragma pack(pop)

	// 아래 크기 검사는 레이아웃 변경을 빌드 때 드러낸다. 파일 내용의 범위/의미 검증을 대신하지 않는다.
	static_assert(sizeof(FILE_HEADER) == 16);
	static_assert(sizeof(MODEL_META_HEADER) == 32);
	static_assert(sizeof(MODEL_SECTION_DESC) == 64);
	static_assert(sizeof(MESH_META_HEADER) == 36);
	static_assert(sizeof(SUBMESH_DESC) == 48);
	static_assert(sizeof(MESH_BONE_ENTRY) == 128);
	static_assert(sizeof(MESH_BOUNDS_V1) == 40);
	static_assert(sizeof(MESH_GEOMETRY_METADATA_V1) == 348);
	static_assert(sizeof(MATERIAL_ENTRY) == 596);
	static_assert(sizeof(MATERIAL_ENTRY_V2) == 4756);
	static_assert(sizeof(MATERIAL_ENTRY_V3) == 5340);
	static_assert(sizeof(SKELETON_META_HEADER) == 32);
	static_assert(sizeof(SKELETON_BONE_NODE) == 256);
	static_assert(sizeof(GLOBAL_ROOT_MATRIX) == 128);
	static_assert(sizeof(SOCKET_ENTRY) == 128);
	static_assert(sizeof(ANIMATION_META_HEADER) == 32);
	static_assert(sizeof(ANIMATION_CHANNEL) == 40);
	static_assert(sizeof(VECTOR_KEY) == 16);
	static_assert(sizeof(QUATERNION_KEY) == 20);
	static_assert(sizeof(ANIMATION_EVENT) == 32);
	static_assert(sizeof(ANIMATION_TRAILER) == 8);
}
