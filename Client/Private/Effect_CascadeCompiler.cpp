#include "Effect_CascadeCompiler.h"

#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"

#include <algorithm>
#include <atomic>
#include <bit>
#include <charconv>
#include <cmath>
#include <cctype>
#include <limits>
#include <map>
#include <set>
#include <span>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	std::atomic_uint64_t g_iCompileAttemptCount = 0u;
	std::atomic_uint64_t g_iCompileSuccessCount = 0u;
	std::atomic_uint64_t g_iCompileFailureCount = 0u;
	constexpr std::string_view UNKNOWN_EXACT_CLASS_SCHEMA_ID =
		"ue3.cascade.quarantine.exact-class.v1";

	class STABLE_HASH final
	{
	public:
		void Add_Byte(const uint8_t Value)
		{
			m_iValue ^= Value;
			m_iValue *= 1099511628211ull;
		}

		void Add_String(const std::string_view Value)
		{
			Add_U64(static_cast<uint64_t>(Value.size()));
			for (const unsigned char Byte : Value)
				Add_Byte(Byte);
		}

		void Add_Bool(const bool_t Value)
		{
			Add_Byte(Value ? 1u : 0u);
		}

		void Add_U32(const uint32_t Value)
		{
			for (uint32_t Shift = 0u; Shift < 32u; Shift += 8u)
				Add_Byte(static_cast<uint8_t>(Value >> Shift));
		}

		void Add_U64(const uint64_t Value)
		{
			for (uint32_t Shift = 0u; Shift < 64u; Shift += 8u)
				Add_Byte(static_cast<uint8_t>(Value >> Shift));
		}

		void Add_F32(const f32_t Value)
		{
			Add_U32(std::bit_cast<uint32_t>(Value));
		}

		std::string Finish() const
		{
			std::array<char, 16u> Digits{};
			const auto Result = std::to_chars(
				Digits.data(), Digits.data() + Digits.size(), m_iValue, 16);
			const size_t iWritten =
				static_cast<size_t>(Result.ptr - Digits.data());
			return "fnv1a64:" + std::string(16u - iWritten, '0') +
				std::string(Digits.data(), iWritten);
		}

		uint64_t Value() const
		{
			return m_iValue;
		}

	private:
		uint64_t m_iValue = 1469598103934665603ull;
	};

	struct PROPERTY_RULE final
	{
		std::string_view strCanonicalPath;
		EFFECT_CASCADE_PROPERTY_STORAGE eStorage =
			EFFECT_CASCADE_PROPERTY_STORAGE::END;
		bool_t bRequired = false;
	};

	struct CLASS_SCHEMA final
	{
		std::string_view strReceiptClassKey;
		EFFECT_CASCADE_OPCODE eOpcode = EFFECT_CASCADE_OPCODE::END;
		std::string_view strOpcodeSchemaId;
		std::span<const PROPERTY_RULE> Rules;
	};

#define L(PATH, REQUIRED) PROPERTY_RULE{ PATH, EFFECT_CASCADE_PROPERTY_STORAGE::LITERAL, REQUIRED }
#define D(PATH, REQUIRED) PROPERTY_RULE{ PATH, EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION, REQUIRED }

	constexpr PROPERTY_RULE g_Acceleration[] = {
		D("acceleration", true), L("balwaysinworldspace", true),
		L("bapplyownerscale", false), L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_CameraOffset[] = {
		D("cameraoffset", true), L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_Color[] = {
		L("brequiresloopingnotification", false), L("lodvalidity", true),
		L("randomseedinfo.properties.randomseeds[0]", false),
		L("randomseedinfo.size", false), D("startalpha", true),
		L("startalpha.distribution.objectpath", false), D("startcolor", true),
		L("startcolor.distribution.objectpath", false) };
	constexpr PROPERTY_RULE g_ColorOverLife[] = {
		D("alphaoverlife", true), L("bclampalpha", false),
		D("coloroverlife", true), L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_ColorScaleOverLife[] = {
		D("alphascaleoverlife", true),
		L("alphascaleoverlife.distribution.objectpath", false),
		D("colorscaleoverlife", true),
		L("colorscaleoverlife.distribution.objectpath", false),
		L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_Lifetime[] = {
		D("lifetime", true), L("lifetime.distribution.objectpath", false),
		L("lodvalidity", true),
		L("randomseedinfo.properties.randomseeds[0]", false),
		L("randomseedinfo.size", false) };
	constexpr PROPERTY_RULE g_Location[] = {
		L("benabled", false), L("lodvalidity", true),
		L("randomseedinfo.hex", false), L("randomseedinfo.size", false),
		D("startlocation", true),
		L("startlocation.distribution.objectpath", false) };
	constexpr PROPERTY_RULE g_LocationDirect[] = {
		L("benabled", true), D("direction", true), D("location", true),
		D("locationoffset", true), L("lodvalidity", true),
		D("scalefactor", true) };
	constexpr PROPERTY_RULE g_LocationOnGround[] = {
		D("adjustlocation", true), L("lodvalidity", true),
		D("skiplocation", true),
		L("skiplocation.distribution.objectpath", true) };
	constexpr PROPERTY_RULE g_LocationPrimitiveCylinder[] = {
		L("b3ddrawmode", false), L("badjustforworldspace", true),
		L("heightaxis", false), L("lodvalidity", true),
		L("negative_x", false), L("negative_z", false),
		L("positive_x", false), L("positive_z", false),
		L("randomseedinfo.hex", false), L("randomseedinfo.size", false),
		D("startheight", true), D("startlocation", true),
		D("startradius", true), L("surfaceonly", false),
		L("velocity", false), D("velocityscale", true) };
	constexpr PROPERTY_RULE g_LocationPrimitiveCylinderSpin[] = {
		L("badjustforworldspace", true), L("benabled", false),
		L("lodvalidity", true), L("negative_x", false),
		L("negative_y", false), L("positive_y", false),
		L("randomseedinfo.hex", false),
		L("randomseedinfo.properties.randomseeds[0]", false),
		L("randomseedinfo.size", false), D("spinangle", true),
		L("spinaxis", false), D("startcylinderrot", true),
		D("startheight", true), D("startlocation", true),
		D("startradius", true), L("surfaceonly", true),
		L("velocity", false), D("velocityscale", true) };
	constexpr PROPERTY_RULE g_LocationPrimitiveSphere[] = {
		L("b3ddrawmode", false), L("benabled", false),
		L("lodvalidity", true), D("startlocation", true),
		D("startradius", true), L("surfaceonly", false),
		L("velocity", true), D("velocityscale", true) };
	constexpr PROPERTY_RULE g_MeshRotation[] = {
		L("lodvalidity", true), L("randomseedinfo.hex", false),
		L("randomseedinfo.size", false), D("startrotation", true),
		L("startrotation.distribution.objectpath", false) };
	constexpr PROPERTY_RULE g_MeshRotationRate[] = {
		L("lodvalidity", true), D("startrotationrate", true) };
	constexpr PROPERTY_RULE g_MeshRotationRateMultiplyLife[] = {
		D("lifemultiplier", true), L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_MeshRotationRateOverLife[] = {
		L("benabled", false), L("lodvalidity", true), D("rotrate", true) };
	constexpr PROPERTY_RULE g_OrientationAxisLock[] = {
		L("lockaxisflags", true), L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_ParameterDynamic[] = {
		L("dynamicparams[0].bscalevelocitybyparamvalue", true),
		L("dynamicparams[0].bspawntimeonly", true),
		L("dynamicparams[0].buseemittertime", true),
		L("dynamicparams[0].paramname", true),
		D("dynamicparams[0].paramvalue", true),
		L("dynamicparams[0].valuemethod", true),
		L("dynamicparams[1].bscalevelocitybyparamvalue", true),
		L("dynamicparams[1].bspawntimeonly", true),
		L("dynamicparams[1].buseemittertime", true),
		L("dynamicparams[1].paramname", true),
		D("dynamicparams[1].paramvalue", true),
		L("dynamicparams[1].valuemethod", true),
		L("dynamicparams[2].bscalevelocitybyparamvalue", true),
		L("dynamicparams[2].bspawntimeonly", true),
		L("dynamicparams[2].buseemittertime", true),
		L("dynamicparams[2].paramname", true),
		D("dynamicparams[2].paramvalue", true),
		L("dynamicparams[2].valuemethod", true),
		L("dynamicparams[3].bscalevelocitybyparamvalue", true),
		L("dynamicparams[3].bspawntimeonly", true),
		L("dynamicparams[3].buseemittertime", true),
		L("dynamicparams[3].paramname", true),
		D("dynamicparams[3].paramvalue", true),
		L("dynamicparams[3].valuemethod", true), L("lodvalidity", true),
		L("updateflags", true) };
	constexpr PROPERTY_RULE g_Required[] = {
		L("ballowimageflipping", false), L("bdelayfirstlooponly", false),
		L("bdrawshadowrenderpass", false), L("bkilloncompleted", false),
		L("bkillondeactivate", false), L("boffsetcenter", false),
		L("bsquareimageflipping", false), L("buselegacyemittertime", false),
		L("buselocalspace", false), L("emitterdelay", false),
		L("emitterduration", false), L("emitterloops", true),
		L("interpolationmethod", false), L("lodvalidity", true),
		L("material", true), L("nearculldistance", false),
		L("offsetcenterx", false), L("offsetcentery", false),
		L("randomimagetime", true), L("screenalignment", false),
		L("sortmode", false), D("spawnrate", true),
		L("subimages_horizontal", false), L("subimages_vertical", false) };
	constexpr PROPERTY_RULE g_Rotation[] = {
		L("lodvalidity", true), D("startrotation", true) };
	constexpr PROPERTY_RULE g_RotationRate[] = {
		L("lodvalidity", true), D("startrotationrate", true) };
	constexpr PROPERTY_RULE g_Size[] = {
		L("lodvalidity", true), L("randomseedinfo.hex", false),
		L("randomseedinfo.size", false), D("startsize", true),
		L("startsize.distribution.objectpath", false) };
	constexpr PROPERTY_RULE g_SizeMultiplyLife[] = {
		L("benabled", false), D("lifemultiplier", true),
		L("lodvalidity", true) };
	constexpr PROPERTY_RULE g_Spawn[] = {
		L("burstlist[0].count", false), L("burstlist[0].countlow", false),
		L("burstlist[0].time", false), L("lodvalidity", true),
		D("rate", true), L("rate.distribution.objectpath", false),
		D("ratescale", true) };
	constexpr PROPERTY_RULE g_SpawnPerUnit[] = {
		L("lodvalidity", true), D("spawnperunit", true),
		L("unitscalar", true) };
	constexpr PROPERTY_RULE g_SubUV[] = {
		L("lodvalidity", true), D("subimageindex", true) };
	constexpr PROPERTY_RULE g_TypeDataDecal[] = {
		L("lodvalidity", true), L("nearplane", true) };
	constexpr PROPERTY_RULE g_TypeDataLight[] = {
		L("b3ddrawmode", true), L("lodvalidity", true),
		L("pointlightcomponent", true),
		L("pointlightcomponent.objectpath", true) };
	constexpr PROPERTY_RULE g_TypeDataMesh[] = {
		L("ballowmotionblur", false), L("boverridematerial", true),
		L("lodvalidity", true), L("mesh", true), L("pitch", false),
		L("roll", false), L("yaw", false) };
	constexpr PROPERTY_RULE g_TypeDataRibbon[] = {
		L("btangentrecalculationeveryframe", true),
		L("distancetessellationstepsize", true), L("lodvalidity", true),
		L("tilingdistance", true) };
	constexpr PROPERTY_RULE g_Velocity[] = {
		L("benabled", false), L("lodvalidity", true),
		L("randomseedinfo.hex", false), L("randomseedinfo.size", false),
		D("startvelocity", true), D("startvelocityradial", true) };
	constexpr PROPERTY_RULE g_VelocityOverLife[] = {
		L("lodvalidity", true), D("veloverlife", true) };

#undef L
#undef D

	const std::array<CLASS_SCHEMA, 32u> g_ClassSchemas = {{
		{ "particlemoduleacceleration", EFFECT_CASCADE_OPCODE::ACCELERATION,
			"ue3.cascade.particlemoduleacceleration.v1", g_Acceleration },
		{ "particlemodulecameraoffset", EFFECT_CASCADE_OPCODE::CAMERA_OFFSET,
			"ue3.cascade.particlemodulecameraoffset.v1", g_CameraOffset },
		{ "particlemodulecolor", EFFECT_CASCADE_OPCODE::COLOR,
			"ue3.cascade.particlemodulecolor.v1", g_Color },
		{ "particlemodulecoloroverlife", EFFECT_CASCADE_OPCODE::COLOR_OVER_LIFE,
			"ue3.cascade.particlemodulecoloroverlife.v1", g_ColorOverLife },
		{ "particlemodulecolorscaleoverlife", EFFECT_CASCADE_OPCODE::COLOR_SCALE_OVER_LIFE,
			"ue3.cascade.particlemodulecolorscaleoverlife.v1", g_ColorScaleOverLife },
		{ "particlemodulelifetime", EFFECT_CASCADE_OPCODE::LIFETIME,
			"ue3.cascade.particlemodulelifetime.v1", g_Lifetime },
		{ "particlemodulelocation", EFFECT_CASCADE_OPCODE::LOCATION,
			"ue3.cascade.particlemodulelocation.v1", g_Location },
		{ "particlemodulelocationdirect", EFFECT_CASCADE_OPCODE::LOCATION_DIRECT,
			"ue3.cascade.particlemodulelocationdirect.v1", g_LocationDirect },
		{ "particlemodulelocationonground", EFFECT_CASCADE_OPCODE::LOCATION_ON_GROUND,
			"ue3.cascade.particlemodulelocationonground.v1", g_LocationOnGround },
		{ "particlemodulelocationprimitivecylinder", EFFECT_CASCADE_OPCODE::LOCATION_PRIMITIVE_CYLINDER,
			"ue3.cascade.particlemodulelocationprimitivecylinder.v1", g_LocationPrimitiveCylinder },
		{ "particlemodulelocationprimitivecylinderspin", EFFECT_CASCADE_OPCODE::LOCATION_PRIMITIVE_CYLINDER_SPIN,
			"ue3.cascade.particlemodulelocationprimitivecylinderspin.v1", g_LocationPrimitiveCylinderSpin },
		{ "particlemodulelocationprimitivesphere", EFFECT_CASCADE_OPCODE::LOCATION_PRIMITIVE_SPHERE,
			"ue3.cascade.particlemodulelocationprimitivesphere.v1", g_LocationPrimitiveSphere },
		{ "particlemodulemeshrotation", EFFECT_CASCADE_OPCODE::MESH_ROTATION,
			"ue3.cascade.particlemodulemeshrotation.v1", g_MeshRotation },
		{ "particlemodulemeshrotationrate", EFFECT_CASCADE_OPCODE::MESH_ROTATION_RATE,
			"ue3.cascade.particlemodulemeshrotationrate.v1", g_MeshRotationRate },
		{ "particlemodulemeshrotationratemultiplylife", EFFECT_CASCADE_OPCODE::MESH_ROTATION_RATE_MULTIPLY_LIFE,
			"ue3.cascade.particlemodulemeshrotationratemultiplylife.v1", g_MeshRotationRateMultiplyLife },
		{ "particlemodulemeshrotationrateoverlife", EFFECT_CASCADE_OPCODE::MESH_ROTATION_RATE_OVER_LIFE,
			"ue3.cascade.particlemodulemeshrotationrateoverlife.v1", g_MeshRotationRateOverLife },
		{ "particlemoduleorientationaxislock", EFFECT_CASCADE_OPCODE::ORIENTATION_AXIS_LOCK,
			"ue3.cascade.particlemoduleorientationaxislock.v1", g_OrientationAxisLock },
		{ "particlemoduleparameterdynamic", EFFECT_CASCADE_OPCODE::PARAMETER_DYNAMIC,
			"ue3.cascade.particlemoduleparameterdynamic.v1", g_ParameterDynamic },
		{ "particlemodulerequired", EFFECT_CASCADE_OPCODE::REQUIRED,
			"ue3.cascade.particlemodulerequired.v1", g_Required },
		{ "particlemodulerotation", EFFECT_CASCADE_OPCODE::ROTATION,
			"ue3.cascade.particlemodulerotation.v1", g_Rotation },
		{ "particlemodulerotationrate", EFFECT_CASCADE_OPCODE::ROTATION_RATE,
			"ue3.cascade.particlemodulerotationrate.v1", g_RotationRate },
		{ "particlemodulesize", EFFECT_CASCADE_OPCODE::SIZE,
			"ue3.cascade.particlemodulesize.v1", g_Size },
		{ "particlemodulesizemultiplylife", EFFECT_CASCADE_OPCODE::SIZE_MULTIPLY_LIFE,
			"ue3.cascade.particlemodulesizemultiplylife.v1", g_SizeMultiplyLife },
		{ "particlemodulespawn", EFFECT_CASCADE_OPCODE::SPAWN,
			"ue3.cascade.particlemodulespawn.v1", g_Spawn },
		{ "particlemodulespawnperunit", EFFECT_CASCADE_OPCODE::SPAWN_PER_UNIT,
			"ue3.cascade.particlemodulespawnperunit.v1", g_SpawnPerUnit },
		{ "particlemodulesubuv", EFFECT_CASCADE_OPCODE::SUBUV,
			"ue3.cascade.particlemodulesubuv.v1", g_SubUV },
		{ "particlemoduletypedatadecal", EFFECT_CASCADE_OPCODE::TYPE_DATA_DECAL,
			"ue3.cascade.particlemoduletypedatadecal.v1", g_TypeDataDecal },
		{ "particlemoduletypedatalight", EFFECT_CASCADE_OPCODE::TYPE_DATA_LIGHT,
			"ue3.cascade.particlemoduletypedatalight.v1", g_TypeDataLight },
		{ "particlemoduletypedatamesh", EFFECT_CASCADE_OPCODE::TYPE_DATA_MESH,
			"ue3.cascade.particlemoduletypedatamesh.v1", g_TypeDataMesh },
		{ "particlemoduletypedataribbon", EFFECT_CASCADE_OPCODE::TYPE_DATA_RIBBON,
			"ue3.cascade.particlemoduletypedataribbon.v1", g_TypeDataRibbon },
		{ "particlemodulevelocity", EFFECT_CASCADE_OPCODE::VELOCITY,
			"ue3.cascade.particlemodulevelocity.v1", g_Velocity },
		{ "particlemodulevelocityoverlifetime", EFFECT_CASCADE_OPCODE::VELOCITY_OVER_LIFE,
			"ue3.cascade.particlemodulevelocityoverlifetime.v1", g_VelocityOverLife }
	}};

	const CLASS_SCHEMA* Find_ClassSchema(const std::string_view ReceiptClassKey)
	{
		const auto Iterator = std::find_if(
			g_ClassSchemas.begin(), g_ClassSchemas.end(),
			[ReceiptClassKey](const CLASS_SCHEMA& Schema)
			{
				return Schema.strReceiptClassKey == ReceiptClassKey;
			});
		return Iterator == g_ClassSchemas.end() ? nullptr : &*Iterator;
	}

	bool_t Is_LowerHex(const std::string_view Value)
	{
		return !Value.empty() && std::all_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return std::isdigit(Character) ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool_t Is_Sha256(const std::string_view Value)
	{
		return Value.size() == 64u && Is_LowerHex(Value);
	}

	bool_t Is_InputIdentity(const std::string_view Value)
	{
		return (Value.starts_with("sha256:") && Value.size() == 71u &&
			Is_LowerHex(Value.substr(7u))) ||
			(Value.starts_with("fnv1a64:") && Value.size() == 24u &&
			Is_LowerHex(Value.substr(8u)));
	}

	bool_t Is_CanonicalPropertyPath(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 256u && std::all_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return std::islower(Character) || std::isdigit(Character) ||
					Character == '_' || Character == '.' || Character == '[' ||
					Character == ']';
			});
	}

	bool_t Is_CanonicalClassKey(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 128u && std::all_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return std::islower(Character) || std::isdigit(Character) ||
					Character == '_';
			});
	}

	bool_t Is_CanonicalAliasId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 192u && std::all_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return std::islower(Character) || std::isdigit(Character) ||
					Character == '_' || Character == '.' || Character == '-';
			});
	}

	bool_t Is_ExpectedExternalIdentityToken(const std::string_view Value)
	{
		return Value.starts_with("sha256:") && Value.size() == 71u &&
			Is_LowerHex(Value.substr(7u));
	}

	bool_t Is_ExpectedSourceIdentity(
		const EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY& Identity)
	{
		return Identity.eKind <
				EFFECT_CASCADE_EXTERNAL_SOURCE_IDENTITY_KIND::END &&
			Is_InputIdentity(Identity.strExpectedCanonicalDocumentIdentity) &&
			Is_ExpectedExternalIdentityToken(
				Identity.strExpectedExternalIdentityToken);
	}

	bool_t Is_CanonicalBlockerSet(
		const std::vector<std::string>& Blockers,
		const bool_t bRequired)
	{
		if (bRequired && Blockers.empty())
			return false;
		std::unordered_set<std::string> Unique;
		return std::all_of(Blockers.begin(), Blockers.end(),
			[&Unique](const std::string& Blocker)
			{
				return !Blocker.empty() && Blocker.size() <= 128u &&
					std::all_of(Blocker.begin(), Blocker.end(),
						[](const unsigned char Character)
						{
							return std::isupper(Character) ||
								std::isdigit(Character) || Character == '_';
						}) &&
					Unique.insert(Blocker).second;
			});
	}

	bool_t Validate_ClassLineage(
		const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage,
		const CLASS_SCHEMA* pSchema,
		EFFECT_CASCADE_HANDLER_RECEIPT& Out)
	{
		Out.strReceiptNormalizedClass = Coverage.strNormalizedClass;
		Out.strExactSourceClass = Coverage.strExactSourceClass;
		Out.strAliasId = Coverage.strAliasId;
		if (!Is_CanonicalClassKey(Out.strReceiptNormalizedClass) ||
			(nullptr != pSchema &&
			 Out.strReceiptNormalizedClass != pSchema->strReceiptClassKey))
		{
			return false;
		}
		if (nullptr == pSchema)
		{
			if (!Is_CanonicalClassKey(Out.strExactSourceClass) ||
				!Out.strAliasId.empty() ||
				Out.strExactSourceClass != Out.strReceiptNormalizedClass)
			{
				return false;
			}
			Out.eClassLineageStatus =
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXACT_CLASS_HANDLER_QUARANTINED;
			Out.bExactClassLineagePreserved = true;
			return true;
		}
		if (Out.strExactSourceClass.empty() && Out.strAliasId.empty())
		{
			Out.eClassLineageStatus =
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::RECEIPT_NORMALIZED_ONLY;
			Out.bExactClassLineagePreserved = false;
			return true;
		}
		if (Out.strExactSourceClass.empty() ||
			!Is_CanonicalClassKey(Out.strExactSourceClass))
		{
			return false;
		}
		Out.bExactClassLineagePreserved = true;
		if (Out.strExactSourceClass == Out.strReceiptNormalizedClass)
		{
			if (!Out.strAliasId.empty())
				return false;
			Out.eClassLineageStatus =
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::EXACT_SOURCE_CLASS;
			return true;
		}
		if (Out.strAliasId.empty())
			return false;
		if (!Is_CanonicalAliasId(Out.strAliasId))
			return false;

		/* Alias execution is intentionally closed until a reviewed, explicit
		 * exact-class -> schema alias registry is added by the execution
		 * compiler owner. A well-shaped free-form alias is preserved as
		 * evidence, but never becomes an executable alias in G04. */
		Out.eClassLineageStatus =
			EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
				EXPLICIT_ALIAS_EXECUTION_UNAPPROVED;
		return true;
	}

	bool_t Is_SourceObjectId(const std::string_view Value)
	{
		const size_t Marker = Value.rfind(":export:");
		if (Marker == std::string_view::npos || Marker == 0u)
			return false;
		const std::string_view Index = Value.substr(Marker + 8u);
		return !Index.empty() && std::all_of(
			Index.begin(), Index.end(), [](const unsigned char Character)
			{
				return std::isdigit(Character);
			});
	}

	bool_t Is_SelectedLodPath(
		const std::string_view EmitterPath,
		const std::string_view SelectedPath)
	{
		const std::string Prefix = std::string(EmitterPath) + ".particlelodlevel_";
		if (!SelectedPath.starts_with(Prefix))
			return false;
		const std::string_view Suffix = SelectedPath.substr(Prefix.size());
		if (Suffix.empty() || !std::all_of(
			Suffix.begin(), Suffix.end(), [](const unsigned char Character)
			{
				return std::isdigit(Character);
			}))
		{
			return false;
		}
		if (Suffix.size() > 1u && Suffix.front() == '0')
			return false;
		uint32_t ParsedObjectSuffix = 0u;
		const auto Parsed = std::from_chars(
			Suffix.data(), Suffix.data() + Suffix.size(), ParsedObjectSuffix);
		return Parsed.ec == std::errc{} &&
			Parsed.ptr == Suffix.data() + Suffix.size();
	}

	std::string Lower_Ascii(const std::string_view Value)
	{
		std::string Result(Value);
		std::transform(Result.begin(), Result.end(), Result.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char>(std::tolower(Character));
			});
		return Result;
	}

	bool_t Is_EmitterNodeLodLineage(
		const std::string_view SourceSystemId,
		const std::string_view EmitterPath,
		const std::string_view EmitterNodeId,
		const std::string_view LodNodeId)
	{
		const size_t SystemDot = SourceSystemId.find('.');
		const size_t EmitterMarker = EmitterNodeId.rfind(":export:");
		const size_t LodMarker = LodNodeId.rfind(":export:");
		if (SystemDot == std::string_view::npos || SystemDot == 0u ||
			EmitterMarker == std::string_view::npos || EmitterMarker == 0u ||
			LodMarker == std::string_view::npos || LodMarker == 0u)
		{
			return false;
		}
		const std::string SystemPackage =
			Lower_Ascii(SourceSystemId.substr(0u, SystemDot));
		const std::string EmitterPackage =
			Lower_Ascii(EmitterNodeId.substr(0u, EmitterMarker));
		const std::string LodPackage =
			Lower_Ascii(LodNodeId.substr(0u, LodMarker));
		const std::string LowerSystem = Lower_Ascii(SourceSystemId);
		const std::string LowerEmitterPath = Lower_Ascii(EmitterPath);
		return SystemPackage == EmitterPackage &&
			SystemPackage == LodPackage &&
			EmitterNodeId != LodNodeId &&
			LowerEmitterPath.starts_with(LowerSystem + ".particlespriteemitter_");
	}

	bool_t Is_SafeGeometryAssetId(const std::string_view AssetId)
	{
		if (AssetId.empty() || AssetId.size() > 260u ||
			!AssetId.starts_with("Effect/") || !AssetId.ends_with(".wmodel") ||
			AssetId.find('\\') != std::string_view::npos ||
			AssetId.find(':') != std::string_view::npos ||
			AssetId.front() == '/' || AssetId.back() == '/')
		{
			return false;
		}
		size_t Begin = 0u;
		while (Begin < AssetId.size())
		{
			const size_t End = AssetId.find('/', Begin);
			const std::string_view Component = AssetId.substr(Begin,
				End == std::string_view::npos ? std::string_view::npos : End - Begin);
			if (Component.empty() || Component == "." || Component == "..")
				return false;
			if (End == std::string_view::npos)
				break;
			Begin = End + 1u;
		}
		return true;
	}

	void Append_Blocker(
		std::vector<std::string>& Blockers,
		const std::string_view Blocker)
	{
		if (!Blocker.empty() &&
			std::find(Blockers.begin(), Blockers.end(), Blocker) ==
				Blockers.end())
		{
			Blockers.emplace_back(Blocker);
		}
	}

	bool_t Parse_Storage(
		const std::string_view Storage,
		EFFECT_CASCADE_PROPERTY_STORAGE& Out)
	{
		if (Storage == "literal")
			Out = EFFECT_CASCADE_PROPERTY_STORAGE::LITERAL;
		else if (Storage == "distribution")
			Out = EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION;
		else
			return false;
		return true;
	}

	bool_t Parse_ModuleRole(
		const std::string_view Role,
		EFFECT_CASCADE_MODULE_ROLE& Out)
	{
		if (Role == "REQUIRED")
			Out = EFFECT_CASCADE_MODULE_ROLE::REQUIRED;
		else if (Role == "MODULE")
			Out = EFFECT_CASCADE_MODULE_ROLE::MODULE;
		else if (Role == "SPAWN")
			Out = EFFECT_CASCADE_MODULE_ROLE::SPAWN;
		else if (Role == "TYPE_DATA")
			Out = EFFECT_CASCADE_MODULE_ROLE::TYPE_DATA;
		else
			return false;
		return true;
	}

	bool_t Parse_Provenance(
		const std::string_view Provenance,
		EFFECT_CASCADE_PROPERTY_PROVENANCE& Out)
	{
		if (Provenance == "SOURCE_TAGGED_PRIMITIVE")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::SOURCE_TAGGED_PRIMITIVE;
		else if (Provenance == "RAW_DISTRIBUTION_TO_TYPED_SOURCE_RECIPE")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::RAW_DISTRIBUTION_TO_TYPED_SOURCE_RECIPE;
		else if (Provenance == "DETERMINISTIC_REFERENCE_METADATA_JOIN")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::DETERMINISTIC_REFERENCE_METADATA_JOIN;
		else if (Provenance == "OPAQUE_HEX_METADATA_ONLY")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::OPAQUE_HEX_METADATA_ONLY;
		else if (Provenance == "OBJECT_REFERENCE_TARGET_OR_CLASS_DEFAULT_UNRESOLVED")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::OBJECT_REFERENCE_TARGET_OR_CLASS_DEFAULT_UNRESOLVED;
		else if (Provenance == "SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED;
		else if (Provenance == "MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED;
		else if (Provenance == "PINNED_SOURCE_RECORD_PHYSICAL_ABSENT")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::PINNED_SOURCE_RECORD_PHYSICAL_ABSENT;
		else if (Provenance == "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED;
		else if (Provenance == "SOURCE_EXACT_PHYSICAL_PACKAGE")
			Out = EFFECT_CASCADE_PROPERTY_PROVENANCE::SOURCE_EXACT_PHYSICAL_PACKAGE;
		else
			return false;
		return true;
	}

	bool_t Validate_PropertyEvidenceMatrix(
		const EFFECT_CASCADE_PROPERTY_STORAGE Storage,
		const EFFECT_SOURCE_COVERAGE_STATUS Status,
		const EFFECT_CASCADE_PROPERTY_PROVENANCE Provenance,
		EFFECT_CASCADE_BLOCKER_REQUIREMENT& OutBlockerRequirement)
	{
		OutBlockerRequirement =
			EFFECT_CASCADE_BLOCKER_REQUIREMENT::BLOCKERS_PROHIBITED;
		if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::LITERAL &&
			Status == EFFECT_SOURCE_COVERAGE_STATUS::SOURCE_DECODED &&
			Provenance ==
				EFFECT_CASCADE_PROPERTY_PROVENANCE::SOURCE_TAGGED_PRIMITIVE)
		{
			return true;
		}
		if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::LITERAL &&
			Status == EFFECT_SOURCE_COVERAGE_STATUS::METADATA_ONLY &&
			(Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				DETERMINISTIC_REFERENCE_METADATA_JOIN ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				OPAQUE_HEX_METADATA_ONLY ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				SEED_ARRAY_SOURCE_DECODED_CONSUMPTION_UNRESOLVED))
		{
			return true;
		}
		if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION &&
			Status == EFFECT_SOURCE_COVERAGE_STATUS::DETERMINISTIC_CONVERSION &&
			(Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				RAW_DISTRIBUTION_TO_TYPED_SOURCE_RECIPE ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				PINNED_SOURCE_RECORD_PHYSICAL_ABSENT ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				SOURCE_EXACT_PHYSICAL_PACKAGE))
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::PROPERTY_BLOCKERS_REQUIRED;
			return true;
		}
		if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION &&
			Status == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
			(Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				OBJECT_REFERENCE_TARGET_OR_CLASS_DEFAULT_UNRESOLVED ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED))
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::PROPERTY_BLOCKERS_REQUIRED;
			return true;
		}
		if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::LITERAL &&
			Status == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
			(Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				MODULE_DEFAULT_OR_NATIVE_TAIL_UNRESOLVED ||
			 Provenance == EFFECT_CASCADE_PROPERTY_PROVENANCE::
				SOURCE_EXACT_PHYSICAL_PACKAGE))
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::PROPERTY_BLOCKERS_REQUIRED;
			return true;
		}
		return false;
	}

	bool_t Validate_ModuleEvidenceAggregate(
		const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage,
		EFFECT_CASCADE_BLOCKER_REQUIREMENT& OutBlockerRequirement)
	{
		if (Coverage.Properties.empty() ||
			Coverage.eStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END)
		{
			return false;
		}
		const bool_t HasUnresolved = std::any_of(
			Coverage.Properties.begin(), Coverage.Properties.end(),
			[](const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property)
			{
				return Property.eStatus ==
					EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED;
			});
		const bool_t HasMetadata = std::any_of(
			Coverage.Properties.begin(), Coverage.Properties.end(),
			[](const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property)
			{
				return Property.eStatus ==
					EFFECT_SOURCE_COVERAGE_STATUS::METADATA_ONLY;
			});
		const bool_t HasConversion = std::any_of(
			Coverage.Properties.begin(), Coverage.Properties.end(),
			[](const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property)
			{
				return Property.eStatus ==
					EFFECT_SOURCE_COVERAGE_STATUS::DETERMINISTIC_CONVERSION;
			});
		if (Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED)
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::MODULE_BLOCKERS_REQUIRED;
			// A module can remain unresolved because its handler, selected-LOD
			// defaults, native tail, or class alias is blocked even when every
			// serialized property has source evidence.  Property evidence is
			// validated independently below; the module-level unresolved state is
			// therefore established by its canonical non-empty blocker set.
			return Is_CanonicalBlockerSet(Coverage.Blockers, true);
		}
		if (HasUnresolved)
			return false;
		if (Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::METADATA_ONLY)
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::MODULE_BLOCKERS_REQUIRED;
			return HasMetadata && Is_CanonicalBlockerSet(
				Coverage.Blockers, true);
		}
		if (Coverage.eStatus ==
			EFFECT_SOURCE_COVERAGE_STATUS::DETERMINISTIC_CONVERSION)
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::MODULE_BLOCKERS_REQUIRED;
			return !HasMetadata && HasConversion && Is_CanonicalBlockerSet(
				Coverage.Blockers, true);
		}
		if (Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::SOURCE_DECODED)
		{
			OutBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::BLOCKERS_PROHIBITED;
			return !HasMetadata && !HasConversion && Coverage.Blockers.empty();
		}
		return false;
	}

	bool_t Role_MatchesOpcode(
		const EFFECT_CASCADE_MODULE_ROLE Role,
		const EFFECT_CASCADE_OPCODE Opcode)
	{
		if (Opcode ==
			EFFECT_CASCADE_OPCODE::UNKNOWN_EXACT_CLASS_QUARANTINE)
		{
			return Role < EFFECT_CASCADE_MODULE_ROLE::END;
		}
		if (Role == EFFECT_CASCADE_MODULE_ROLE::REQUIRED)
			return Opcode == EFFECT_CASCADE_OPCODE::REQUIRED;
		if (Role == EFFECT_CASCADE_MODULE_ROLE::SPAWN)
			return Opcode == EFFECT_CASCADE_OPCODE::SPAWN;
		const bool_t bTypeData =
			Opcode == EFFECT_CASCADE_OPCODE::TYPE_DATA_DECAL ||
			Opcode == EFFECT_CASCADE_OPCODE::TYPE_DATA_LIGHT ||
			Opcode == EFFECT_CASCADE_OPCODE::TYPE_DATA_MESH ||
			Opcode == EFFECT_CASCADE_OPCODE::TYPE_DATA_RIBBON;
		if (Role == EFFECT_CASCADE_MODULE_ROLE::TYPE_DATA)
			return bTypeData;
		return Role == EFFECT_CASCADE_MODULE_ROLE::MODULE &&
			Opcode != EFFECT_CASCADE_OPCODE::REQUIRED &&
			Opcode != EFFECT_CASCADE_OPCODE::SPAWN && !bTypeData;
	}

	bool_t Is_SourceRendererIdentity(
		const EFFECT_ELEMENT_KIND ElementKind,
		const EFFECT_CASCADE_RENDERER_EVIDENCE& Renderer)
	{
		if (ElementKind == EFFECT_ELEMENT_KIND::PARTICLE)
		{
			return (Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
				Renderer.eType == EFFECT_RENDERER_TYPE::SPRITE_PARTICLE) &&
				Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		}
		if (ElementKind == EFFECT_ELEMENT_KIND::DECAL)
		{
			return Renderer.eType == EFFECT_RENDERER_TYPE::DECAL_PARTICLE &&
				Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		}
		if (ElementKind == EFFECT_ELEMENT_KIND::TRAIL)
		{
			return Renderer.eType == EFFECT_RENDERER_TYPE::CASCADE_RIBBON &&
				Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		}
		if (ElementKind == EFFECT_ELEMENT_KIND::LIGHT)
		{
			return Renderer.eType == EFFECT_RENDERER_TYPE::LIGHT_PARTICLE &&
				Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		}
		return ElementKind == EFFECT_ELEMENT_KIND::SCREEN_POST &&
			Renderer.eType == EFFECT_RENDERER_TYPE::SCREEN_POST &&
			Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1;
	}

	bool_t Build_EmitterIdentity(
		const EFFECT_ELEMENT_DESC& Element,
		EFFECT_CASCADE_EMITTER_IDENTITY& Out)
	{
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence =
			Element.SourceRecipe.CompilerEvidence;
		if (Evidence.strSourceSystemId.empty() ||
			Evidence.strSourceOccurrenceId.empty() || Element.strElementId.empty())
		{
			return false;
		}
		Out.strSourceSystemId = Evidence.strSourceSystemId;
		Out.strSourceOccurrenceId = Evidence.strSourceOccurrenceId;
		Out.strElementId = Element.strElementId;
		Out.strCanonicalId = Out.strSourceSystemId + "|" +
			Out.strSourceOccurrenceId + "|" + Out.strElementId;
		STABLE_HASH Hash;
		Hash.Add_String(Out.strCanonicalId);
		Out.iStableReference = Hash.Value();
		return 0u != Out.iStableReference;
	}

	bool_t Build_ModuleReference(
		const EFFECT_CASCADE_EMITTER_IDENTITY& EmitterIdentity,
		const EFFECT_SOURCE_MODULE_REFERENCE_DESC& Source,
		const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage,
		EFFECT_CASCADE_MODULE_REFERENCE& Out)
	{
		if (!Parse_ModuleRole(Source.strRole, Out.eRole) ||
			!Is_SourceObjectId(Source.strSourceObjectId) ||
			!Is_Sha256(Source.strSourceRecordSha256))
		{
			return false;
		}
		const std::string ExpectedStableId = Source.strSourceObjectId +
			"@ref:" + std::to_string(Source.iSourceReferenceIndex);
		if (Coverage.strModuleStableId != ExpectedStableId)
			return false;
		Out.iOrder = Source.iOrder;
		Out.iSourceReferenceIndex = Source.iSourceReferenceIndex;
		Out.strReceiptRole = Source.strRole;
		Out.strSourceObjectId = Source.strSourceObjectId;
		Out.strSourceRecordSha256 = Source.strSourceRecordSha256;
		Out.strModuleStableId = Coverage.strModuleStableId;
		Out.strCanonicalId = EmitterIdentity.strCanonicalId + "|" +
			std::to_string(Out.iOrder) + "|" +
			std::to_string(Out.iSourceReferenceIndex) + "|" +
			std::to_string(static_cast<uint32_t>(Out.eRole)) + "|" +
			Out.strReceiptRole + "|" +
			Out.strSourceObjectId + "|" + Out.strSourceRecordSha256;
		STABLE_HASH Hash;
		Hash.Add_String(Out.strCanonicalId);
		Out.iStableReference = Hash.Value();
		return 0u != Out.iStableReference;
	}

	bool_t Build_PropertyKey(
		const EFFECT_CASCADE_EMITTER_IDENTITY& EmitterIdentity,
		const EFFECT_CASCADE_MODULE_REFERENCE& ModuleReference,
		const EFFECT_CASCADE_PROPERTY_STORAGE Storage,
		const std::string_view CanonicalPath,
		EFFECT_CASCADE_PROPERTY_KEY& Out)
	{
		if (!Is_CanonicalPropertyPath(CanonicalPath))
			return false;
		Out.strCanonicalPath = CanonicalPath;
		Out.strCanonicalReferenceId = EmitterIdentity.strCanonicalId + "|" +
			ModuleReference.strCanonicalId + "|" +
			std::to_string(static_cast<uint32_t>(Storage)) + "|" +
			Out.strCanonicalPath;
		STABLE_HASH SemanticHash;
		SemanticHash.Add_String(Out.strCanonicalPath);
		Out.iStableSemantic = SemanticHash.Value();
		STABLE_HASH ReferenceHash;
		ReferenceHash.Add_String(Out.strCanonicalReferenceId);
		Out.iStableReference = ReferenceHash.Value();
		return 0u != Out.iStableSemantic && 0u != Out.iStableReference;
	}

	const PROPERTY_RULE* Find_PropertyRule(
		const CLASS_SCHEMA& Schema,
		const std::string_view CanonicalPath)
	{
		const auto Iterator = std::find_if(
			Schema.Rules.begin(), Schema.Rules.end(),
			[CanonicalPath](const PROPERTY_RULE& Rule)
			{
				return Rule.strCanonicalPath == CanonicalPath;
			});
		return Iterator == Schema.Rules.end() ? nullptr : &*Iterator;
	}

	void Hash_Inspection(
		const EFFECT_CASCADE_INSPECTION_IR& Inspection,
		STABLE_HASH& Hash)
	{
		Hash.Add_String("lostark.effect.cascade-inspection-ir.v3");
		const auto AddStrings = [&Hash](const std::vector<std::string>& Values)
		{
			Hash.Add_U64(static_cast<uint64_t>(Values.size()));
			for (const std::string& Value : Values)
				Hash.Add_String(Value);
		};
		const auto AddPropertyKey = [&Hash](
			const EFFECT_CASCADE_PROPERTY_KEY& Property)
		{
			Hash.Add_String(Property.strCanonicalPath);
			Hash.Add_String(Property.strCanonicalReferenceId);
			Hash.Add_U64(Property.iStableSemantic);
			Hash.Add_U64(Property.iStableReference);
		};
		Hash.Add_U32(Inspection.iCompilerRevision);
		Hash.Add_String(Inspection.strEffectAssetId);
		Hash.Add_String(Inspection.strCanonicalDocumentIdentity);
		Hash.Add_U32(static_cast<uint32_t>(Inspection.eExternalIdentityKind));
		Hash.Add_U32(static_cast<uint32_t>(Inspection.eExternalIdentityStatus));
		Hash.Add_String(Inspection.strExpectedExternalIdentityToken);
		Hash.Add_Bool(Inspection.bExternalIdentityAuthenticated);
		Hash.Add_U64(static_cast<uint64_t>(Inspection.Systems.size()));
		for (const EFFECT_CASCADE_INSPECTION_SYSTEM& System : Inspection.Systems)
		{
			Hash.Add_String(System.strSourceSystemId);
			Hash.Add_U64(System.iStableSemantic);
			Hash.Add_U64(static_cast<uint64_t>(System.Emitters.size()));
			for (const EFFECT_CASCADE_INSPECTION_EMITTER& Emitter : System.Emitters)
			{
				Hash.Add_String(Emitter.Identity.strSourceSystemId);
				Hash.Add_String(Emitter.Identity.strSourceOccurrenceId);
				Hash.Add_String(Emitter.Identity.strElementId);
				Hash.Add_String(Emitter.Identity.strCanonicalId);
				Hash.Add_U64(Emitter.Identity.iStableReference);
				Hash.Add_U32(static_cast<uint32_t>(Emitter.eElementKind));
				Hash.Add_U32(Emitter.SelectedLOD.iArrayIndex);
				Hash.Add_U32(static_cast<uint32_t>(Emitter.SelectedLOD.ePolicy));
				Hash.Add_String(Emitter.SelectedLOD.strEmitterPath);
				Hash.Add_String(Emitter.SelectedLOD.strEmitterNodeId);
				Hash.Add_String(Emitter.SelectedLOD.strSelectedLodPath);
				Hash.Add_String(Emitter.SelectedLOD.strSelectedLodNodeId);
				Hash.Add_String(Emitter.SelectedLOD.strCanonicalLineageId);
				Hash.Add_U64(Emitter.SelectedLOD.iStableReference);
				Hash.Add_U32(static_cast<uint32_t>(
					Emitter.SelectedLOD.eLevelProvenance));
				Hash.Add_U32(static_cast<uint32_t>(
					Emitter.SelectedLOD.eEnabledProvenance));
				Hash.Add_Bool(Emitter.SelectedLOD.bIdentityPreserved);
				Hash.Add_Bool(Emitter.SelectedLOD.bExecutionFidelityProven);
				AddStrings(Emitter.SelectedLOD.Blockers);
				Hash.Add_U32(static_cast<uint32_t>(Emitter.Renderer.eType));
				Hash.Add_U32(static_cast<uint32_t>(Emitter.Renderer.eSourceSpace));
				Hash.Add_U64(static_cast<uint64_t>(Emitter.OrderedOpcodes.size()));
				for (const EFFECT_CASCADE_INSPECTION_OPCODE& Opcode :
					Emitter.OrderedOpcodes)
				{
					Hash.Add_U32(static_cast<uint32_t>(Opcode.eOpcode));
					Hash.Add_U32(Opcode.Reference.iOrder);
					Hash.Add_U32(Opcode.Reference.iSourceReferenceIndex);
					Hash.Add_U32(static_cast<uint32_t>(Opcode.Reference.eRole));
					Hash.Add_String(Opcode.Reference.strReceiptRole);
					Hash.Add_String(Opcode.Reference.strSourceObjectId);
					Hash.Add_String(Opcode.Reference.strSourceRecordSha256);
					Hash.Add_String(Opcode.Reference.strModuleStableId);
					Hash.Add_String(Opcode.Reference.strCanonicalId);
					Hash.Add_U64(Opcode.Reference.iStableReference);
					Hash.Add_U64(static_cast<uint64_t>(Opcode.Properties.size()));
					for (const EFFECT_CASCADE_PROPERTY_EVIDENCE& Property :
						Opcode.Properties)
					{
						AddPropertyKey(Property.Property);
						Hash.Add_U32(static_cast<uint32_t>(Property.eStorage));
						Hash.Add_U32(static_cast<uint32_t>(Property.eCoverageStatus));
						Hash.Add_U32(static_cast<uint32_t>(Property.eProvenance));
						Hash.Add_U32(static_cast<uint32_t>(
							Property.eBlockerRequirement));
						Hash.Add_String(Property.strReceiptProvenanceToken);
						AddStrings(Property.Blockers);
					}
					Hash.Add_U64(static_cast<uint64_t>(
						Opcode.DistributionEvidenceIndices.size()));
					for (const uint32_t Index : Opcode.DistributionEvidenceIndices)
						Hash.Add_U32(Index);
					Hash.Add_U32(static_cast<uint32_t>(
						Opcode.HandlerReceipt.eResult));
					Hash.Add_U32(static_cast<uint32_t>(
						Opcode.HandlerReceipt.eModuleCoverageStatus));
					Hash.Add_U32(static_cast<uint32_t>(
						Opcode.HandlerReceipt.eAggregateBlockerRequirement));
					Hash.Add_String(Opcode.HandlerReceipt.strReceiptNormalizedClass);
					Hash.Add_String(Opcode.HandlerReceipt.strExactSourceClass);
					Hash.Add_String(Opcode.HandlerReceipt.strAliasId);
					Hash.Add_U32(static_cast<uint32_t>(
						Opcode.HandlerReceipt.eClassLineageStatus));
					Hash.Add_String(Opcode.HandlerReceipt.strOpcodeSchemaId);
					Hash.Add_Bool(
						Opcode.HandlerReceipt.bExactClassLineagePreserved);
					Hash.Add_Bool(Opcode.HandlerReceipt.bPayloadAccessAllowed);
					Hash.Add_U64(static_cast<uint64_t>(
						Opcode.HandlerReceipt.PropertyConsumption.size()));
					for (const EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT& Receipt :
						Opcode.HandlerReceipt.PropertyConsumption)
					{
						AddPropertyKey(Receipt.Property);
						Hash.Add_U32(static_cast<uint32_t>(Receipt.eStorage));
						Hash.Add_U32(static_cast<uint32_t>(Receipt.eResult));
						Hash.Add_String(Receipt.strHandlerFieldId);
						Hash.Add_Bool(Receipt.bRequired);
					}
					AddStrings(Opcode.HandlerReceipt.RequiredPropertyReferenceIds);
					AddStrings(Opcode.HandlerReceipt.ConsumedPropertyReferenceIds);
					AddStrings(Opcode.HandlerReceipt.Blockers);
				}
				Hash.Add_U64(static_cast<uint64_t>(Emitter.Distributions.size()));
				for (const EFFECT_CASCADE_DISTRIBUTION_EVIDENCE& Distribution :
					Emitter.Distributions)
				{
					AddPropertyKey(Distribution.Property);
					Hash.Add_U32(static_cast<uint32_t>(
						Distribution.eCoverageStatus));
					Hash.Add_U32(static_cast<uint32_t>(Distribution.eProvenance));
					Hash.Add_Bool(Distribution.bRawPayloadRead);
					Hash.Add_Bool(Distribution.bExecutionAllowed);
					AddStrings(Distribution.Blockers);
				}
				Hash.Add_Bool(Emitter.Geometry.has_value());
				if (Emitter.Geometry.has_value())
				{
					Hash.Add_String(Emitter.Geometry->strAssetId);
					Hash.Add_String(Emitter.Geometry->strReceiptFileSha256);
					Hash.Add_String(Emitter.Geometry->strReceiptSelfSha256);
					Hash.Add_F32(Emitter.Geometry->fGeometryPreScale);
					Hash.Add_String(Emitter.Geometry->strScaleSemantics);
					Hash.Add_String(Emitter.Geometry->strSourceStatus);
					Hash.Add_Bool(Emitter.Geometry->bPayloadIntegrityValid);
					Hash.Add_Bool(Emitter.Geometry->bRuntimeConsumerReady);
					AddStrings(Emitter.Geometry->EvidenceFlags);
					AddStrings(Emitter.Geometry->ChannelConsumptionBlockers);
				}
				Hash.Add_Bool(Emitter.bSourceRecipeEnabled);
				Hash.Add_Bool(Emitter.bSourceExecutionAdmission);
				AddStrings(Emitter.Blockers);
			}
		}
		Hash.Add_U32(Inspection.Consumption.iSystemCount);
		Hash.Add_U32(Inspection.Consumption.iDeclaredSourceElementCount);
		Hash.Add_U32(Inspection.Consumption.iDisabledSourceRecipeCount);
		Hash.Add_U32(Inspection.Consumption.iEmitterCount);
		Hash.Add_U32(Inspection.Consumption.iOrderedOpcodeCount);
		Hash.Add_U32(Inspection.Consumption.iDistributionEvidenceCount);
		Hash.Add_U32(Inspection.Consumption.iRequiredPropertyCount);
		Hash.Add_U32(Inspection.Consumption.iConsumedPropertyCount);
		Hash.Add_U32(Inspection.Consumption.iUnknownClassCount);
		Hash.Add_U32(Inspection.Consumption.iQuarantinedExactClassCount);
		Hash.Add_U32(Inspection.Consumption.iUnconsumedRequiredPropertyCount);
		Hash.Add_U32(Inspection.Consumption.iHandlerPropertyReceiptCount);
		Hash.Add_U32(Inspection.Consumption.iRawPayloadReadCount);
		Hash.Add_U32(Inspection.Consumption.iExecutableOpcodeCount);
		Hash.Add_U32(Inspection.Consumption.iBlockerCount);
		for (const uint32_t Count : Inspection.Consumption.RendererCounts)
			Hash.Add_U32(Count);
		AddStrings(Inspection.Blockers);
		Hash.Add_Bool(Inspection.bExecutable);
		Hash.Add_Bool(Inspection.bProductAdmission);
	}

	bool_t Same_PropertyKey(
		const EFFECT_CASCADE_PROPERTY_KEY& Left,
		const EFFECT_CASCADE_PROPERTY_KEY& Right)
	{
		return Left.strCanonicalPath == Right.strCanonicalPath &&
			Left.strCanonicalReferenceId == Right.strCanonicalReferenceId &&
			Left.iStableSemantic == Right.iStableSemantic &&
			Left.iStableReference == Right.iStableReference;
	}

	bool_t Validate_InspectionStructure(
		const EFFECT_CASCADE_INSPECTION_IR& Inspection)
	{
		if (Inspection.strEffectAssetId.empty() ||
			!Is_InputIdentity(Inspection.strCanonicalDocumentIdentity) ||
			Inspection.eExternalIdentityKind >=
				EFFECT_CASCADE_EXTERNAL_SOURCE_IDENTITY_KIND::END ||
			Inspection.eExternalIdentityStatus !=
				EFFECT_CASCADE_EXTERNAL_IDENTITY_STATUS::
					SELF_CONSISTENT_UNAUTHENTICATED ||
			!Is_ExpectedExternalIdentityToken(
				Inspection.strExpectedExternalIdentityToken) ||
			Inspection.bExternalIdentityAuthenticated ||
			Inspection.bExecutable || Inspection.bProductAdmission ||
			!Is_CanonicalBlockerSet(Inspection.Blockers, true) ||
			std::find(Inspection.Blockers.begin(), Inspection.Blockers.end(),
				"SELF_CONSISTENT_UNAUTHENTICATED") ==
					Inspection.Blockers.end() ||
			std::find(Inspection.Blockers.begin(), Inspection.Blockers.end(),
				"SOURCE_EXTERNAL_IDENTITY_ADAPTER_PENDING") ==
					Inspection.Blockers.end())
		{
			return false;
		}
		std::unordered_set<std::string> SystemIds;
		std::unordered_set<std::string> EmitterIds;
		uint32_t iRecountedDeclaredSourceElementCount = 0u;
		uint32_t iRecountedDisabledSourceRecipeCount = 0u;
		uint32_t iRecountedQuarantinedExactClassCount = 0u;
		for (const EFFECT_CASCADE_INSPECTION_SYSTEM& System : Inspection.Systems)
		{
			STABLE_HASH SystemHash;
			SystemHash.Add_String(System.strSourceSystemId);
			if (System.strSourceSystemId.empty() ||
				System.iStableSemantic != SystemHash.Value() ||
				!SystemIds.insert(System.strSourceSystemId).second ||
				System.Emitters.empty())
			{
				return false;
			}
			for (const EFFECT_CASCADE_INSPECTION_EMITTER& Emitter : System.Emitters)
			{
				++iRecountedDeclaredSourceElementCount;
				if (!Emitter.bSourceRecipeEnabled)
					++iRecountedDisabledSourceRecipeCount;
				const bool_t bHasDisabledRecipeBlocker =
					std::find(Emitter.Blockers.begin(), Emitter.Blockers.end(),
						"SOURCE_RECIPE_DISABLED_QUARANTINED") !=
							Emitter.Blockers.end();
				const std::string ExpectedEmitterId =
					Emitter.Identity.strSourceSystemId + "|" +
					Emitter.Identity.strSourceOccurrenceId + "|" +
					Emitter.Identity.strElementId;
				STABLE_HASH EmitterHash;
				EmitterHash.Add_String(ExpectedEmitterId);
				if (Emitter.Identity.strSourceSystemId != System.strSourceSystemId ||
					Emitter.Identity.strSourceOccurrenceId.empty() ||
					Emitter.Identity.strElementId.empty() ||
					Emitter.Identity.strCanonicalId != ExpectedEmitterId ||
					Emitter.Identity.iStableReference != EmitterHash.Value() ||
					!EmitterIds.insert(ExpectedEmitterId).second ||
					!Is_SourceRendererIdentity(
						Emitter.eElementKind, Emitter.Renderer) ||
					Emitter.bSourceExecutionAdmission ||
					Emitter.bSourceRecipeEnabled == bHasDisabledRecipeBlocker ||
					!Is_CanonicalBlockerSet(Emitter.Blockers, true) ||
					Emitter.OrderedOpcodes.empty())
				{
					return false;
				}

				const std::string ExpectedLodLineage =
					Emitter.Identity.strSourceSystemId + "|" +
					Emitter.SelectedLOD.strEmitterPath + "|" +
					Emitter.SelectedLOD.strEmitterNodeId + "|" +
					Emitter.SelectedLOD.strSelectedLodPath + "|" +
					Emitter.SelectedLOD.strSelectedLodNodeId + "|" +
					std::to_string(Emitter.SelectedLOD.iArrayIndex);
				STABLE_HASH LodHash;
				LodHash.Add_String(ExpectedLodLineage);
				if (Emitter.SelectedLOD.ePolicy !=
						EFFECT_CASCADE_LOD_SELECTION_POLICY::FIRST_LOD_ONLY ||
					Emitter.SelectedLOD.eLevelProvenance !=
						EFFECT_CASCADE_LOD_FIELD_PROVENANCE::
							UNRESOLVED_CLASS_DEFAULT ||
					Emitter.SelectedLOD.eEnabledProvenance !=
						EFFECT_CASCADE_LOD_FIELD_PROVENANCE::
							UNRESOLVED_CLASS_DEFAULT ||
					!Is_SelectedLodPath(Emitter.SelectedLOD.strEmitterPath,
						Emitter.SelectedLOD.strSelectedLodPath) ||
					!Is_EmitterNodeLodLineage(System.strSourceSystemId,
						Emitter.SelectedLOD.strEmitterPath,
						Emitter.SelectedLOD.strEmitterNodeId,
						Emitter.SelectedLOD.strSelectedLodNodeId) ||
					Emitter.SelectedLOD.strCanonicalLineageId !=
						ExpectedLodLineage ||
					Emitter.SelectedLOD.iStableReference != LodHash.Value() ||
					!Emitter.SelectedLOD.bIdentityPreserved ||
					Emitter.SelectedLOD.bExecutionFidelityProven ||
					!Is_CanonicalBlockerSet(
						Emitter.SelectedLOD.Blockers, true))
				{
					return false;
				}

				if (Emitter.Geometry.has_value())
				{
					const EFFECT_CASCADE_GEOMETRY_EVIDENCE& Geometry =
						*Emitter.Geometry;
					const std::vector<std::string> ExpectedFlags = {
						"SOURCE_STATUS:" + Geometry.strSourceStatus };
					if (Emitter.Renderer.eType !=
							EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
						!Is_SafeGeometryAssetId(Geometry.strAssetId) ||
						!Is_Sha256(Geometry.strReceiptFileSha256) ||
						!Is_Sha256(Geometry.strReceiptSelfSha256) ||
						!std::isfinite(Geometry.fGeometryPreScale) ||
						Geometry.fGeometryPreScale <= 0.f ||
						Geometry.strScaleSemantics.empty() ||
						Geometry.strSourceStatus.empty() ||
						Geometry.EvidenceFlags != ExpectedFlags ||
						Geometry.bPayloadIntegrityValid ||
						Geometry.bRuntimeConsumerReady ||
						!Is_CanonicalBlockerSet(
							Geometry.ChannelConsumptionBlockers, true))
					{
						return false;
					}
				}
				else if (Emitter.Renderer.eType ==
					EFFECT_RENDERER_TYPE::MESH_PARTICLE)
				{
					return false;
				}

				std::unordered_set<uint32_t> Orders;
				std::unordered_set<uint32_t> SourceReferenceIndices;
				std::unordered_set<uint32_t> DistributionIndices;
				for (size_t iOpcode = 0u;
					iOpcode < Emitter.OrderedOpcodes.size(); ++iOpcode)
				{
					const EFFECT_CASCADE_INSPECTION_OPCODE& Opcode =
						Emitter.OrderedOpcodes[iOpcode];
					const CLASS_SCHEMA* pReceiptSchema = Find_ClassSchema(
						Opcode.HandlerReceipt.strReceiptNormalizedClass);
					const bool_t bUnknownExactClassQuarantine =
						Opcode.eOpcode == EFFECT_CASCADE_OPCODE::
							UNKNOWN_EXACT_CLASS_QUARANTINE &&
						Opcode.HandlerReceipt.strOpcodeSchemaId ==
							UNKNOWN_EXACT_CLASS_SCHEMA_ID;
					const CLASS_SCHEMA* pSchema =
						bUnknownExactClassQuarantine ? nullptr : pReceiptSchema;
					const bool_t bTypedSchema = nullptr != pSchema &&
						pSchema->eOpcode == Opcode.eOpcode &&
						pSchema->strOpcodeSchemaId ==
							Opcode.HandlerReceipt.strOpcodeSchemaId;
					if (Opcode.HandlerReceipt.eClassLineageStatus ==
						EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
							EXACT_CLASS_HANDLER_QUARANTINED)
					{
						++iRecountedQuarantinedExactClassCount;
					}
					EFFECT_CASCADE_MODULE_ROLE ParsedRole =
						EFFECT_CASCADE_MODULE_ROLE::END;
					if ((!bTypedSchema && !bUnknownExactClassQuarantine) ||
						!Role_MatchesOpcode(Opcode.Reference.eRole, Opcode.eOpcode) ||
						!Parse_ModuleRole(Opcode.Reference.strReceiptRole, ParsedRole) ||
						ParsedRole != Opcode.Reference.eRole ||
						!Is_SourceObjectId(Opcode.Reference.strSourceObjectId) ||
						!Is_Sha256(Opcode.Reference.strSourceRecordSha256) ||
						Opcode.Reference.iOrder != iOpcode ||
						Opcode.Reference.iSourceReferenceIndex != iOpcode ||
						!Orders.insert(Opcode.Reference.iOrder).second ||
						!SourceReferenceIndices.insert(
							Opcode.Reference.iSourceReferenceIndex).second ||
						Opcode.HandlerReceipt.eResult !=
							EFFECT_CASCADE_HANDLER_RESULT::
								STRUCTURE_CONSUMED_EXECUTION_BLOCKED ||
						Opcode.HandlerReceipt.bPayloadAccessAllowed ||
						!Is_CanonicalBlockerSet(
							Opcode.HandlerReceipt.Blockers, true))
					{
						return false;
					}

					const std::string ExpectedModuleId =
						Emitter.Identity.strCanonicalId + "|" +
						std::to_string(Opcode.Reference.iOrder) + "|" +
						std::to_string(Opcode.Reference.iSourceReferenceIndex) + "|" +
						std::to_string(static_cast<uint32_t>(Opcode.Reference.eRole)) +
						"|" + Opcode.Reference.strReceiptRole + "|" +
						Opcode.Reference.strSourceObjectId + "|" +
						Opcode.Reference.strSourceRecordSha256;
					STABLE_HASH ModuleHash;
					ModuleHash.Add_String(ExpectedModuleId);
					if (Opcode.Reference.strCanonicalId != ExpectedModuleId ||
						Opcode.Reference.iStableReference != ModuleHash.Value() ||
						Opcode.Reference.strModuleStableId !=
							Opcode.Reference.strSourceObjectId + "@ref:" +
							std::to_string(Opcode.Reference.iSourceReferenceIndex))
					{
						return false;
					}

					const bool_t bPendingClass =
						Opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
								RECEIPT_NORMALIZED_ONLY &&
						Opcode.HandlerReceipt.strExactSourceClass.empty() &&
						Opcode.HandlerReceipt.strAliasId.empty() &&
						!Opcode.HandlerReceipt.bExactClassLineagePreserved;
					const bool_t bExactClass =
						!bUnknownExactClassQuarantine &&
						Opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::EXACT_SOURCE_CLASS &&
						Is_CanonicalClassKey(
							Opcode.HandlerReceipt.strExactSourceClass) &&
						Opcode.HandlerReceipt.strExactSourceClass ==
							Opcode.HandlerReceipt.strReceiptNormalizedClass &&
						Opcode.HandlerReceipt.strAliasId.empty() &&
						Opcode.HandlerReceipt.bExactClassLineagePreserved;
					const bool_t bExactClassQuarantined =
						bUnknownExactClassQuarantine &&
						Opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
								EXACT_CLASS_HANDLER_QUARANTINED &&
						Is_CanonicalClassKey(
							Opcode.HandlerReceipt.strExactSourceClass) &&
						Opcode.HandlerReceipt.strAliasId.empty() &&
						Opcode.HandlerReceipt.bExactClassLineagePreserved &&
						std::find(Opcode.HandlerReceipt.Blockers.begin(),
							Opcode.HandlerReceipt.Blockers.end(),
							"EXACT_SOURCE_CLASS_HANDLER_UNAVAILABLE") !=
								Opcode.HandlerReceipt.Blockers.end() &&
						std::find(Opcode.HandlerReceipt.Blockers.begin(),
							Opcode.HandlerReceipt.Blockers.end(),
							"UNKNOWN_EXACT_CLASS_OPCODE_QUARANTINED") !=
								Opcode.HandlerReceipt.Blockers.end();
					const bool_t bExplicitAliasUnapproved =
						Opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
								EXPLICIT_ALIAS_EXECUTION_UNAPPROVED &&
						Is_CanonicalClassKey(
							Opcode.HandlerReceipt.strExactSourceClass) &&
						Opcode.HandlerReceipt.strExactSourceClass !=
							Opcode.HandlerReceipt.strReceiptNormalizedClass &&
						Is_CanonicalAliasId(Opcode.HandlerReceipt.strAliasId) &&
						Opcode.HandlerReceipt.bExactClassLineagePreserved &&
						std::find(Opcode.HandlerReceipt.Blockers.begin(),
							Opcode.HandlerReceipt.Blockers.end(),
							"SOURCE_CLASS_ALIAS_UNAPPROVED") !=
								Opcode.HandlerReceipt.Blockers.end();
					if (!bPendingClass && !bExactClass &&
						!bExactClassQuarantined &&
						!bExplicitAliasUnapproved)
						return false;

					if (Opcode.Properties.size() !=
							Opcode.HandlerReceipt.PropertyConsumption.size() ||
						Opcode.Properties.size() !=
							Opcode.HandlerReceipt.ConsumedPropertyReferenceIds.size())
					{
						return false;
					}
					std::vector<std::string> ExpectedRequired;
					std::unordered_set<std::string> PropertyPaths;
					std::unordered_map<std::string, std::string> ReferenceByPath;
					std::unordered_map<std::string,
						const EFFECT_CASCADE_PROPERTY_EVIDENCE*>
						DistributionPropertyByReference;
					for (size_t iProperty = 0u;
						iProperty < Opcode.Properties.size(); ++iProperty)
					{
						const EFFECT_CASCADE_PROPERTY_EVIDENCE& Property =
							Opcode.Properties[iProperty];
						const PROPERTY_RULE* pRule = nullptr == pSchema ? nullptr :
							Find_PropertyRule(
								*pSchema, Property.Property.strCanonicalPath);
						EFFECT_CASCADE_PROPERTY_KEY ExpectedKey;
						EFFECT_CASCADE_BLOCKER_REQUIREMENT ExpectedRequirement =
							EFFECT_CASCADE_BLOCKER_REQUIREMENT::END;
						EFFECT_CASCADE_PROPERTY_PROVENANCE ParsedProvenance =
							EFFECT_CASCADE_PROPERTY_PROVENANCE::END;
						if ((!bUnknownExactClassQuarantine &&
							 (nullptr == pRule ||
							  pRule->eStorage != Property.eStorage)) ||
							(bUnknownExactClassQuarantine &&
							 !Is_CanonicalPropertyPath(
								Property.Property.strCanonicalPath)) ||
							!PropertyPaths.insert(
								Property.Property.strCanonicalPath).second ||
							!Validate_PropertyEvidenceMatrix(Property.eStorage,
								Property.eCoverageStatus, Property.eProvenance,
								ExpectedRequirement) ||
							!Parse_Provenance(Property.strReceiptProvenanceToken,
								ParsedProvenance) ||
							ParsedProvenance != Property.eProvenance ||
							ExpectedRequirement != Property.eBlockerRequirement ||
							!Is_CanonicalBlockerSet(Property.Blockers,
								ExpectedRequirement ==
									EFFECT_CASCADE_BLOCKER_REQUIREMENT::
										PROPERTY_BLOCKERS_REQUIRED) ||
							(ExpectedRequirement ==
								EFFECT_CASCADE_BLOCKER_REQUIREMENT::
									BLOCKERS_PROHIBITED &&
							 !Property.Blockers.empty()) ||
							!Build_PropertyKey(Emitter.Identity, Opcode.Reference,
								Property.eStorage,
								Property.Property.strCanonicalPath, ExpectedKey) ||
							!Same_PropertyKey(Property.Property, ExpectedKey))
						{
							return false;
						}
						const EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT& Receipt =
							Opcode.HandlerReceipt.PropertyConsumption[iProperty];
						const std::string ExpectedFieldId =
							Opcode.HandlerReceipt.strOpcodeSchemaId + ":" +
							ExpectedKey.strCanonicalPath;
						const EFFECT_CASCADE_PROPERTY_HANDLER_RESULT
							ExpectedPropertyResult = bUnknownExactClassQuarantine ?
								EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
									QUARANTINED_FIELD_PRESERVED_EXECUTION_BLOCKED :
								EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
									SCHEMA_FIELD_CONSUMED_EXECUTION_BLOCKED;
						if (!Same_PropertyKey(Receipt.Property, ExpectedKey) ||
							Receipt.eStorage != Property.eStorage ||
							Receipt.eResult != ExpectedPropertyResult ||
							Receipt.strHandlerFieldId != ExpectedFieldId ||
							Receipt.bRequired !=
								(nullptr != pRule && pRule->bRequired) ||
							Opcode.HandlerReceipt.ConsumedPropertyReferenceIds[iProperty] !=
								ExpectedKey.strCanonicalReferenceId)
						{
							return false;
						}
						ReferenceByPath.emplace(
							ExpectedKey.strCanonicalPath,
							ExpectedKey.strCanonicalReferenceId);
						if (Property.eStorage ==
							EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION)
						{
							DistributionPropertyByReference.emplace(
								ExpectedKey.strCanonicalReferenceId, &Property);
						}
					}
					if (nullptr != pSchema)
					{
						for (const PROPERTY_RULE& Rule : pSchema->Rules)
						{
							if (!Rule.bRequired)
								continue;
							const auto Required = ReferenceByPath.find(
								std::string(Rule.strCanonicalPath));
							if (Required == ReferenceByPath.end())
								return false;
							ExpectedRequired.push_back(Required->second);
						}
					}
					if (ExpectedRequired !=
						Opcode.HandlerReceipt.RequiredPropertyReferenceIds)
					{
						return false;
					}
					if (Opcode.DistributionEvidenceIndices.size() !=
						DistributionPropertyByReference.size())
					{
						return false;
					}
					for (const uint32_t DistributionIndex :
						Opcode.DistributionEvidenceIndices)
					{
						if (DistributionIndex >= Emitter.Distributions.size() ||
							!DistributionIndices.insert(DistributionIndex).second)
						{
							return false;
						}
						const EFFECT_CASCADE_DISTRIBUTION_EVIDENCE& Distribution =
							Emitter.Distributions[DistributionIndex];
						const auto Property = DistributionPropertyByReference.find(
							Distribution.Property.strCanonicalReferenceId);
						std::vector<std::string> ExpectedBlockers;
						if (Property != DistributionPropertyByReference.end())
						{
							ExpectedBlockers = Property->second->Blockers;
							Append_Blocker(ExpectedBlockers,
								"SOURCE_TYPED_DISTRIBUTION_ADAPTER_PENDING");
						}
						if (Distribution.bRawPayloadRead ||
							Distribution.bExecutionAllowed ||
							Distribution.Blockers != ExpectedBlockers ||
							!Is_CanonicalBlockerSet(Distribution.Blockers, true) ||
							Property == DistributionPropertyByReference.end() ||
							!Same_PropertyKey(Distribution.Property,
								Property->second->Property) ||
							Distribution.eCoverageStatus !=
								Property->second->eCoverageStatus ||
							Distribution.eProvenance !=
								Property->second->eProvenance)
						{
							return false;
						}
					}
				}
				if (DistributionIndices.size() != Emitter.Distributions.size())
					return false;
			}
		}
		return iRecountedDeclaredSourceElementCount ==
				Inspection.Consumption.iDeclaredSourceElementCount &&
			iRecountedDeclaredSourceElementCount ==
				Inspection.Consumption.iEmitterCount &&
			iRecountedDisabledSourceRecipeCount ==
				Inspection.Consumption.iDisabledSourceRecipeCount &&
			iRecountedQuarantinedExactClassCount ==
				Inspection.Consumption.iQuarantinedExactClassCount;
	}
}

std::string Client::CEffectCascadeCompiler::Build_CanonicalDocumentIdentity(
	const EFFECT_DOCUMENT_DESC& Document)
{
	STABLE_HASH Hash;
	Hash.Add_String("lostark.effect.source-contract.canonical-document.v1");
	Hash.Add_String(CEffectDocumentCodec::Serialize(Document));
	return Hash.Finish();
}

bool_t Client::CEffectCascadeCompiler::Compile_SourceInspection(
	const EFFECT_DOCUMENT_DESC& Document,
	const EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY& ExpectedIdentity,
	std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>& OutInspection,
	std::string& strOutError)
{
	g_iCompileAttemptCount.fetch_add(1u, std::memory_order_relaxed);
	const auto Fail = [&](const std::string& Error)
	{
		strOutError = Error;
		g_iCompileFailureCount.fetch_add(1u, std::memory_order_relaxed);
		return false;
	};
	const std::string CanonicalDocumentIdentity =
		Build_CanonicalDocumentIdentity(Document);
	if (!Document.bSourceContract ||
		Document.iLoadedFormatVersion != EFFECT_SOURCE_CONTRACT_FORMAT_VERSION ||
		Document.strEffectAssetId.empty() ||
		Document.Elements.empty() ||
		Document.Elements.size() >
			(static_cast<size_t>((std::numeric_limits<uint32_t>::max)())) ||
		!Is_ExpectedSourceIdentity(ExpectedIdentity) ||
		ExpectedIdentity.strExpectedCanonicalDocumentIdentity !=
			CanonicalDocumentIdentity)
	{
		return Fail("Cascade inspection requires a fixed canonical document and external source identity expectation.");
	}

	auto Staged = std::make_shared<EFFECT_CASCADE_INSPECTION_IR>();
	Staged->strEffectAssetId = Document.strEffectAssetId;
	Staged->strCanonicalDocumentIdentity = CanonicalDocumentIdentity;
	Staged->eExternalIdentityKind = ExpectedIdentity.eKind;
	Staged->eExternalIdentityStatus =
		EFFECT_CASCADE_EXTERNAL_IDENTITY_STATUS::
			SELF_CONSISTENT_UNAUTHENTICATED;
	Staged->strExpectedExternalIdentityToken =
		ExpectedIdentity.strExpectedExternalIdentityToken;
	Staged->bExternalIdentityAuthenticated = false;
	Staged->Consumption.iDeclaredSourceElementCount =
		static_cast<uint32_t>(Document.Elements.size());
	Append_Blocker(Staged->Blockers, "COMPILER_INSPECTION_ONLY");
	Append_Blocker(Staged->Blockers, "PRODUCT_ADMISSION_DISABLED");
	Append_Blocker(Staged->Blockers, "RAW_OPCODE_EXECUTOR_UNCHANGED");
	Append_Blocker(Staged->Blockers, "SOURCE_TYPED_DISTRIBUTION_ADAPTER_PENDING");
	Append_Blocker(Staged->Blockers, "SIX_RENDERER_CONSUMERS_PENDING");
	Append_Blocker(Staged->Blockers, "GEOMETRY_SCALE_CONSUMER_PENDING");
		Append_Blocker(Staged->Blockers, "PREPARED_REVISION_PENDING");
	Append_Blocker(Staged->Blockers,
		"CANONICAL_DOCUMENT_CHECKSUM_NOT_AUTHENTICATION");
	Append_Blocker(Staged->Blockers, "SELF_CONSISTENT_UNAUTHENTICATED");
	Append_Blocker(Staged->Blockers,
		"SOURCE_EXTERNAL_IDENTITY_ADAPTER_PENDING");

	std::unordered_map<std::string, size_t> SystemIndices;
	std::unordered_set<std::string> EmitterCanonicalIds;
	std::unordered_map<uint64_t, std::string> StableReferences;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& SourceEvidence =
			Recipe.CompilerEvidence;
		EFFECT_CASCADE_INSPECTION_EMITTER Emitter;
		Emitter.bSourceRecipeEnabled = Recipe.bEnabled;
		if (!Emitter.bSourceRecipeEnabled)
		{
			++Staged->Consumption.iDisabledSourceRecipeCount;
			Append_Blocker(Emitter.Blockers,
				"SOURCE_RECIPE_DISABLED_QUARANTINED");
		}
		if (!Build_EmitterIdentity(Element, Emitter.Identity) ||
			!EmitterCanonicalIds.insert(Emitter.Identity.strCanonicalId).second)
		{
			return Fail("Cascade inspection rejected an invalid or duplicate composite emitter identity.");
		}
		const auto [EmitterHash, bNewEmitterHash] = StableReferences.emplace(
			Emitter.Identity.iStableReference, Emitter.Identity.strCanonicalId);
		if (!bNewEmitterHash && EmitterHash->second != Emitter.Identity.strCanonicalId)
			return Fail("Cascade inspection rejected a stable emitter identity collision.");

		auto [SystemIterator, bInsertedSystem] = SystemIndices.emplace(
			Emitter.Identity.strSourceSystemId, Staged->Systems.size());
		if (bInsertedSystem)
		{
			EFFECT_CASCADE_INSPECTION_SYSTEM System;
			System.strSourceSystemId = Emitter.Identity.strSourceSystemId;
			STABLE_HASH SystemHash;
			SystemHash.Add_String(System.strSourceSystemId);
			System.iStableSemantic = SystemHash.Value();
			Staged->Systems.push_back(std::move(System));
		}

		Emitter.eElementKind = Element.eKind;
		Emitter.Renderer.eType = Element.Renderer.eType;
		Emitter.Renderer.eSourceSpace = Element.Renderer.eSourceSpace;
		if (!Is_SourceRendererIdentity(
			Emitter.eElementKind, Emitter.Renderer))
		{
			return Fail("Cascade inspection rejected an invalid typed renderer identity.");
		}
		++Staged->Consumption.RendererCounts[
			static_cast<size_t>(Emitter.Renderer.eType)];

		Emitter.SelectedLOD.iArrayIndex = SourceEvidence.iSelectedLodArrayIndex;
		if (SourceEvidence.strLodSelectionPolicy != "FIRST_LOD_ONLY" ||
			SourceEvidence.iSelectedLodArrayIndex != 0u)
		{
			return Fail("Cascade inspection rejected an unbound LOD selection policy.");
		}
		Emitter.SelectedLOD.ePolicy =
			EFFECT_CASCADE_LOD_SELECTION_POLICY::FIRST_LOD_ONLY;
		Emitter.SelectedLOD.strEmitterPath = SourceEvidence.strSourceEmitterPath;
		Emitter.SelectedLOD.strEmitterNodeId = SourceEvidence.strSourceEmitterNodeId;
		Emitter.SelectedLOD.strSelectedLodPath = SourceEvidence.strSelectedLodPath;
		Emitter.SelectedLOD.strSelectedLodNodeId = SourceEvidence.strSelectedLodNodeId;
		if (Emitter.SelectedLOD.strEmitterPath.empty() ||
			!Is_SourceObjectId(Emitter.SelectedLOD.strEmitterNodeId) ||
			!Is_SelectedLodPath(Emitter.SelectedLOD.strEmitterPath,
				Emitter.SelectedLOD.strSelectedLodPath) ||
			!Is_SourceObjectId(Emitter.SelectedLOD.strSelectedLodNodeId) ||
			!Is_EmitterNodeLodLineage(
				Emitter.Identity.strSourceSystemId,
				Emitter.SelectedLOD.strEmitterPath,
				Emitter.SelectedLOD.strEmitterNodeId,
				Emitter.SelectedLOD.strSelectedLodNodeId))
		{
			return Fail("Cascade inspection rejected a forged selected-LOD path or node identity.");
		}
		if (SourceEvidence.strSelectedLodLevelProvenance !=
				"UNRESOLVED_CLASS_DEFAULT" ||
			SourceEvidence.strSelectedLodEnabledProvenance !=
				"UNRESOLVED_CLASS_DEFAULT")
		{
			return Fail("Cascade inspection rejected free-form LOD provenance promotion.");
		}
		Emitter.SelectedLOD.eLevelProvenance =
			EFFECT_CASCADE_LOD_FIELD_PROVENANCE::UNRESOLVED_CLASS_DEFAULT;
		Emitter.SelectedLOD.eEnabledProvenance =
			EFFECT_CASCADE_LOD_FIELD_PROVENANCE::UNRESOLVED_CLASS_DEFAULT;
		Emitter.SelectedLOD.strCanonicalLineageId =
			Emitter.Identity.strSourceSystemId + "|" +
			Emitter.SelectedLOD.strEmitterPath + "|" +
			Emitter.SelectedLOD.strEmitterNodeId + "|" +
			Emitter.SelectedLOD.strSelectedLodPath + "|" +
			Emitter.SelectedLOD.strSelectedLodNodeId + "|" +
			std::to_string(Emitter.SelectedLOD.iArrayIndex);
		STABLE_HASH LodLineageHash;
		LodLineageHash.Add_String(Emitter.SelectedLOD.strCanonicalLineageId);
		Emitter.SelectedLOD.iStableReference = LodLineageHash.Value();
		if (0u == Emitter.SelectedLOD.iStableReference)
			return Fail("Cascade inspection rejected an invalid selected-LOD lineage identity.");
		Emitter.SelectedLOD.bIdentityPreserved = true;
		Emitter.SelectedLOD.bExecutionFidelityProven = false;
		Append_Blocker(Emitter.SelectedLOD.Blockers,
			"LOD_EXECUTION_FIDELITY_UNPROVEN");

		Emitter.bSourceExecutionAdmission =
			Recipe.CompiledExecutionAdmission.bAllowed;
		if (Emitter.bSourceExecutionAdmission ||
			Recipe.CompiledExecutionAdmission.Blockers.empty())
		{
			return Fail("Cascade inspection rejected execution admission before typed class, payload, and handler closure.");
		}
		for (const std::string& Blocker :
			Recipe.CompiledExecutionAdmission.Blockers)
		{
			Append_Blocker(Emitter.Blockers, Blocker);
		}
		Append_Blocker(Emitter.Blockers, "COMPILER_INSPECTION_ONLY");
		Append_Blocker(Emitter.Blockers,
			"SOURCE_EXECUTION_ADMISSION_REJECTED_BEFORE_PAYLOAD");

		if (Recipe.GeometryBinding.bEnabled)
		{
			const auto MeshBinding = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
			const size_t iMeshBindingCount = static_cast<size_t>(std::count_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				}));
			if (Emitter.Renderer.eType != EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
				iMeshBindingCount != 1u || MeshBinding == Element.ResourceBindings.end() ||
				!Is_SafeGeometryAssetId(Recipe.GeometryBinding.strAssetId) ||
				!Is_SafeGeometryAssetId(MeshBinding->strAssetId) ||
				MeshBinding->strAssetId != Recipe.GeometryBinding.strAssetId ||
				!Is_Sha256(Recipe.GeometryBinding.strReceiptFileSha256) ||
				!Is_Sha256(Recipe.GeometryBinding.strReceiptSelfSha256) ||
				!std::isfinite(Recipe.GeometryBinding.fCarrierGeometryPreScale) ||
				Recipe.GeometryBinding.fCarrierGeometryPreScale <= 0.f)
			{
				return Fail("Cascade inspection rejected nonfinite or unbound geometry evidence.");
			}
			EFFECT_CASCADE_GEOMETRY_EVIDENCE Geometry;
			Geometry.strAssetId = Recipe.GeometryBinding.strAssetId;
			Geometry.strReceiptFileSha256 =
				Recipe.GeometryBinding.strReceiptFileSha256;
			Geometry.strReceiptSelfSha256 =
				Recipe.GeometryBinding.strReceiptSelfSha256;
			Geometry.fGeometryPreScale =
				Recipe.GeometryBinding.fCarrierGeometryPreScale;
			Geometry.strScaleSemantics =
				Recipe.GeometryBinding.strParticleScaleSemantics;
			Geometry.strSourceStatus = Recipe.GeometryBinding.strStatus;
			Geometry.EvidenceFlags.push_back(
				"SOURCE_STATUS:" + Recipe.GeometryBinding.strStatus);
			Geometry.ChannelConsumptionBlockers =
				Recipe.GeometryBinding.Blockers;
			Append_Blocker(Geometry.ChannelConsumptionBlockers,
				"GEOMETRY_PAYLOAD_INTEGRITY_SCHEMA_PENDING");
			Append_Blocker(Geometry.ChannelConsumptionBlockers,
				"GEOMETRY_RUNTIME_PRESCALE_CONSUMER_PENDING");
			Append_Blocker(Geometry.ChannelConsumptionBlockers,
				"GEOMETRY_MODEL_CACHE_IDENTITY_PENDING");
			Append_Blocker(Geometry.ChannelConsumptionBlockers,
				"GEOMETRY_BOUNDS_CONSUMER_PENDING");
			Emitter.Geometry = std::move(Geometry);
		}

		if (Recipe.ModuleCoverage.size() !=
			SourceEvidence.ModuleReferenceOrder.size())
		{
			return Fail("Cascade inspection rejected module coverage/reference cardinality drift.");
		}
		std::unordered_set<uint32_t> ModuleOrders;
		std::unordered_set<uint32_t> SourceReferenceIndices;
		std::unordered_set<std::string> ModuleStableIds;
		std::unordered_set<std::string> ModuleCanonicalIds;
		for (size_t iModule = 0u; iModule < Recipe.ModuleCoverage.size(); ++iModule)
		{
			const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage =
				Recipe.ModuleCoverage[iModule];
			const EFFECT_SOURCE_MODULE_REFERENCE_DESC& SourceReference =
				SourceEvidence.ModuleReferenceOrder[iModule];
			if (SourceReference.iOrder != iModule ||
				SourceReference.iSourceReferenceIndex != iModule ||
				!ModuleOrders.insert(SourceReference.iOrder).second ||
				!SourceReferenceIndices.insert(
					SourceReference.iSourceReferenceIndex).second ||
				!ModuleStableIds.insert(Coverage.strModuleStableId).second)
			{
				return Fail("Cascade inspection rejected duplicate or noncanonical module order/reference identity.");
			}
			const CLASS_SCHEMA* pReceiptSchema =
				Find_ClassSchema(Coverage.strNormalizedClass);
			const bool_t bUnknownExactClassQuarantine =
				nullptr == pReceiptSchema;
			const CLASS_SCHEMA* pSchema = bUnknownExactClassQuarantine ?
				nullptr : pReceiptSchema;
			EFFECT_CASCADE_BLOCKER_REQUIREMENT AggregateBlockerRequirement =
				EFFECT_CASCADE_BLOCKER_REQUIREMENT::END;
			if (!Validate_ModuleEvidenceAggregate(
				Coverage, AggregateBlockerRequirement))
			{
				return Fail("Cascade inspection rejected the typed module status/property/blocker aggregate matrix.");
			}

			EFFECT_CASCADE_INSPECTION_OPCODE Opcode;
			Opcode.eOpcode = bUnknownExactClassQuarantine ?
				EFFECT_CASCADE_OPCODE::UNKNOWN_EXACT_CLASS_QUARANTINE :
				pSchema->eOpcode;
			if (!Build_ModuleReference(Emitter.Identity, SourceReference,
				Coverage, Opcode.Reference) ||
				!Role_MatchesOpcode(Opcode.Reference.eRole, Opcode.eOpcode) ||
				!ModuleCanonicalIds.insert(Opcode.Reference.strCanonicalId).second)
			{
				return Fail("Cascade inspection rejected module alias/reference lineage.");
			}
			const auto [ModuleHash, bNewModuleHash] = StableReferences.emplace(
				Opcode.Reference.iStableReference,
				Opcode.Reference.strCanonicalId);
			if (!bNewModuleHash &&
				ModuleHash->second != Opcode.Reference.strCanonicalId)
			{
				return Fail("Cascade inspection rejected a stable module reference collision.");
			}
			if (!Validate_ClassLineage(
				Coverage, pSchema, Opcode.HandlerReceipt))
			{
				if (bUnknownExactClassQuarantine)
					++Staged->Consumption.iUnknownClassCount;
				return Fail("Cascade inspection rejected an unbound exact-class or alias lineage.");
			}
			if (Opcode.HandlerReceipt.eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXACT_CLASS_HANDLER_QUARANTINED)
			{
				++Staged->Consumption.iQuarantinedExactClassCount;
			}
			Opcode.HandlerReceipt.eModuleCoverageStatus = Coverage.eStatus;
			Opcode.HandlerReceipt.eAggregateBlockerRequirement =
				AggregateBlockerRequirement;
			Opcode.HandlerReceipt.strOpcodeSchemaId =
				bUnknownExactClassQuarantine ?
					std::string(UNKNOWN_EXACT_CLASS_SCHEMA_ID) :
					std::string(pSchema->strOpcodeSchemaId);
			Opcode.HandlerReceipt.eResult =
				EFFECT_CASCADE_HANDLER_RESULT::STRUCTURE_CONSUMED_EXECUTION_BLOCKED;
			Opcode.HandlerReceipt.bPayloadAccessAllowed = false;
			for (const std::string& Blocker : Coverage.Blockers)
				Append_Blocker(Opcode.HandlerReceipt.Blockers, Blocker);
			if (Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED)
				Append_Blocker(Opcode.HandlerReceipt.Blockers,
					"MODULE_SOURCE_UNRESOLVED");
			else if (Coverage.eStatus ==
				EFFECT_SOURCE_COVERAGE_STATUS::METADATA_ONLY)
			{
				Append_Blocker(Opcode.HandlerReceipt.Blockers,
					"MODULE_SOURCE_METADATA_ONLY");
			}
			Append_Blocker(Opcode.HandlerReceipt.Blockers,
				"SOURCE_VALUE_PAYLOAD_NOT_MATERIALIZED");
			if (Opcode.HandlerReceipt.eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::RECEIPT_NORMALIZED_ONLY)
			{
				Append_Blocker(Opcode.HandlerReceipt.Blockers,
					"SOURCE_EXACT_CLASS_LINEAGE_ADAPTER_PENDING");
			}
			else if (Opcode.HandlerReceipt.eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXACT_CLASS_HANDLER_QUARANTINED)
			{
				Append_Blocker(Opcode.HandlerReceipt.Blockers,
					"EXACT_SOURCE_CLASS_HANDLER_UNAVAILABLE");
				if (bUnknownExactClassQuarantine)
				{
					Append_Blocker(Opcode.HandlerReceipt.Blockers,
						"UNKNOWN_EXACT_CLASS_OPCODE_QUARANTINED");
				}
			}
			else if (Opcode.HandlerReceipt.eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXPLICIT_ALIAS_EXECUTION_UNAPPROVED)
			{
				Append_Blocker(Opcode.HandlerReceipt.Blockers,
					"SOURCE_CLASS_ALIAS_UNAPPROVED");
			}

			std::unordered_set<std::string> PropertyPaths;
			std::unordered_set<std::string> PropertyReferenceIds;
			std::unordered_map<std::string, std::string> ReferenceIdByPath;
			for (const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& SourceProperty :
				Coverage.Properties)
			{
				EFFECT_CASCADE_PROPERTY_STORAGE Storage =
					EFFECT_CASCADE_PROPERTY_STORAGE::END;
				if (!Parse_Storage(SourceProperty.strStorage, Storage) ||
					!PropertyPaths.insert(SourceProperty.strPropertyPath).second)
				{
					return Fail("Cascade inspection rejected unknown storage or duplicate property path.");
				}
				const PROPERTY_RULE* pRule = nullptr == pSchema ? nullptr :
					Find_PropertyRule(*pSchema, SourceProperty.strPropertyPath);
				if ((!bUnknownExactClassQuarantine &&
					 (nullptr == pRule || pRule->eStorage != Storage)) ||
					(bUnknownExactClassQuarantine &&
					 !Is_CanonicalPropertyPath(SourceProperty.strPropertyPath)))
				{
					return Fail("Cascade inspection rejected an unconsumed or storage-mutated opcode property: " +
						SourceProperty.strPropertyPath);
				}
				EFFECT_CASCADE_PROPERTY_EVIDENCE Property;
				Property.eStorage = Storage;
				Property.eCoverageStatus = SourceProperty.eStatus;
				Property.strReceiptProvenanceToken =
					SourceProperty.strProvenance;
				if (Property.eCoverageStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END ||
					!Parse_Provenance(SourceProperty.strProvenance,
						Property.eProvenance) ||
					!Validate_PropertyEvidenceMatrix(Storage,
						Property.eCoverageStatus, Property.eProvenance,
						Property.eBlockerRequirement) ||
					!Is_CanonicalBlockerSet(SourceProperty.Blockers,
						Property.eBlockerRequirement ==
							EFFECT_CASCADE_BLOCKER_REQUIREMENT::
								PROPERTY_BLOCKERS_REQUIRED) ||
					(Property.eBlockerRequirement ==
						EFFECT_CASCADE_BLOCKER_REQUIREMENT::BLOCKERS_PROHIBITED &&
					 !SourceProperty.Blockers.empty()) ||
					!Build_PropertyKey(Emitter.Identity, Opcode.Reference, Storage,
						SourceProperty.strPropertyPath, Property.Property) ||
					!PropertyReferenceIds.insert(
						Property.Property.strCanonicalReferenceId).second)
				{
					return Fail("Cascade inspection rejected property provenance, path, or reference identity: " +
						SourceProperty.strPropertyPath + " [" +
						SourceProperty.strStorage + ", " +
						SourceProperty.strProvenance + "].");
				}
				Property.Blockers = SourceProperty.Blockers;
				const auto [PropertyHash, bNewPropertyHash] =
					StableReferences.emplace(Property.Property.iStableReference,
						Property.Property.strCanonicalReferenceId);
				if (!bNewPropertyHash &&
					PropertyHash->second !=
						Property.Property.strCanonicalReferenceId)
				{
					return Fail("Cascade inspection rejected a stable property reference collision.");
				}
				ReferenceIdByPath.emplace(
					Property.Property.strCanonicalPath,
					Property.Property.strCanonicalReferenceId);
				Opcode.HandlerReceipt.ConsumedPropertyReferenceIds.push_back(
					Property.Property.strCanonicalReferenceId);
				EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT Consumption;
				Consumption.Property = Property.Property;
				Consumption.eStorage = Property.eStorage;
				Consumption.eResult = bUnknownExactClassQuarantine ?
					EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
						QUARANTINED_FIELD_PRESERVED_EXECUTION_BLOCKED :
					EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
						SCHEMA_FIELD_CONSUMED_EXECUTION_BLOCKED;
				Consumption.strHandlerFieldId =
					Opcode.HandlerReceipt.strOpcodeSchemaId + ":" +
					Property.Property.strCanonicalPath;
				Consumption.bRequired =
					nullptr != pRule && pRule->bRequired;
				Opcode.HandlerReceipt.PropertyConsumption.push_back(
					std::move(Consumption));
				if (Storage == EFFECT_CASCADE_PROPERTY_STORAGE::DISTRIBUTION)
				{
					EFFECT_CASCADE_DISTRIBUTION_EVIDENCE Distribution;
					Distribution.Property = Property.Property;
					Distribution.eCoverageStatus = Property.eCoverageStatus;
					Distribution.eProvenance = Property.eProvenance;
					Distribution.Blockers = Property.Blockers;
					Append_Blocker(Distribution.Blockers,
						"SOURCE_TYPED_DISTRIBUTION_ADAPTER_PENDING");
					Opcode.DistributionEvidenceIndices.push_back(
						static_cast<uint32_t>(Emitter.Distributions.size()));
					Emitter.Distributions.push_back(std::move(Distribution));
				}
				Opcode.Properties.push_back(std::move(Property));
			}

			if (nullptr != pSchema)
			{
				for (const PROPERTY_RULE& Rule : pSchema->Rules)
				{
					if (!Rule.bRequired)
						continue;
					const auto Required = ReferenceIdByPath.find(
						std::string(Rule.strCanonicalPath));
					if (Required == ReferenceIdByPath.end())
					{
						++Staged->Consumption.iUnconsumedRequiredPropertyCount;
						return Fail("Cascade inspection handler did not consume a required typed property: " +
							std::string(Rule.strCanonicalPath));
					}
					Opcode.HandlerReceipt.RequiredPropertyReferenceIds.push_back(
						Required->second);
				}
			}
			Staged->Consumption.iRequiredPropertyCount +=
				static_cast<uint32_t>(
					Opcode.HandlerReceipt.RequiredPropertyReferenceIds.size());
			Staged->Consumption.iConsumedPropertyCount +=
				static_cast<uint32_t>(
					Opcode.HandlerReceipt.ConsumedPropertyReferenceIds.size());
			Staged->Consumption.iHandlerPropertyReceiptCount +=
				static_cast<uint32_t>(
					Opcode.HandlerReceipt.PropertyConsumption.size());
			for (const std::string& Blocker : Opcode.HandlerReceipt.Blockers)
				Append_Blocker(Emitter.Blockers, Blocker);
			Emitter.OrderedOpcodes.push_back(std::move(Opcode));
		}

		for (const std::string& Blocker : Emitter.Blockers)
			Append_Blocker(Staged->Blockers, Blocker);
		Staged->Systems[SystemIterator->second].Emitters.push_back(
			std::move(Emitter));
	}

	Staged->Consumption.iSystemCount =
		static_cast<uint32_t>(Staged->Systems.size());
	for (const EFFECT_CASCADE_INSPECTION_SYSTEM& System : Staged->Systems)
	{
		Staged->Consumption.iEmitterCount +=
			static_cast<uint32_t>(System.Emitters.size());
		for (const EFFECT_CASCADE_INSPECTION_EMITTER& Emitter : System.Emitters)
		{
			Staged->Consumption.iOrderedOpcodeCount +=
				static_cast<uint32_t>(Emitter.OrderedOpcodes.size());
			Staged->Consumption.iDistributionEvidenceCount +=
				static_cast<uint32_t>(Emitter.Distributions.size());
		}
	}
	if (Staged->Consumption.iEmitterCount == 0u ||
		Staged->Consumption.iDeclaredSourceElementCount !=
			Staged->Consumption.iEmitterCount ||
		Staged->Consumption.iDisabledSourceRecipeCount >
			Staged->Consumption.iDeclaredSourceElementCount ||
		Staged->Consumption.iOrderedOpcodeCount == 0u ||
		Staged->Consumption.iUnknownClassCount != 0u ||
		Staged->Consumption.iUnconsumedRequiredPropertyCount != 0u ||
		Staged->Consumption.iConsumedPropertyCount <
			Staged->Consumption.iRequiredPropertyCount ||
		Staged->Consumption.iHandlerPropertyReceiptCount !=
			Staged->Consumption.iConsumedPropertyCount ||
		Staged->Consumption.iRawPayloadReadCount != 0u ||
		Staged->Consumption.iExecutableOpcodeCount != 0u)
	{
		return Fail("Cascade inspection receipt is structurally incomplete.");
	}
	Staged->Consumption.iBlockerCount =
		static_cast<uint32_t>(Staged->Blockers.size());
	if (!Validate_InspectionStructure(*Staged))
		return Fail("Cascade inspection internal typed receipt validation failed.");
	STABLE_HASH InspectionHash;
	Hash_Inspection(*Staged, InspectionHash);
	Staged->strInspectionHash = InspectionHash.Finish();
	Staged->bExecutable = false;
	Staged->bProductAdmission = false;
	OutInspection = std::move(Staged);
	strOutError.clear();
	g_iCompileSuccessCount.fetch_add(1u, std::memory_order_relaxed);
	return true;
}

bool_t Client::CEffectCascadeCompiler::Matches_InputIdentity(
	const EFFECT_CASCADE_INSPECTION_IR& Inspection,
	const EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY& ExpectedIdentity)
{
	if (!Is_ExpectedSourceIdentity(ExpectedIdentity) ||
		!Is_InputIdentity(Inspection.strInspectionHash) ||
		Inspection.iCompilerRevision !=
			EFFECT_CASCADE_INSPECTION_COMPILER_REVISION ||
		Inspection.strCanonicalDocumentIdentity !=
			ExpectedIdentity.strExpectedCanonicalDocumentIdentity ||
		Inspection.eExternalIdentityKind != ExpectedIdentity.eKind ||
		Inspection.strExpectedExternalIdentityToken !=
			ExpectedIdentity.strExpectedExternalIdentityToken ||
		Inspection.Systems.empty() || Inspection.Blockers.empty() ||
		Inspection.bExecutable || Inspection.bProductAdmission ||
		!Validate_InspectionStructure(Inspection))
	{
		return false;
	}
	EFFECT_CASCADE_CONSUMPTION_RECEIPT Recounted;
	Recounted.iSystemCount = static_cast<uint32_t>(Inspection.Systems.size());
	for (const EFFECT_CASCADE_INSPECTION_SYSTEM& System : Inspection.Systems)
	{
		Recounted.iEmitterCount += static_cast<uint32_t>(System.Emitters.size());
		for (const EFFECT_CASCADE_INSPECTION_EMITTER& Emitter : System.Emitters)
		{
			++Recounted.iDeclaredSourceElementCount;
			if (!Emitter.bSourceRecipeEnabled)
				++Recounted.iDisabledSourceRecipeCount;
			if (Emitter.Renderer.eType >= EFFECT_RENDERER_TYPE::END)
				return false;
			++Recounted.RendererCounts[
				static_cast<size_t>(Emitter.Renderer.eType)];
			Recounted.iOrderedOpcodeCount +=
				static_cast<uint32_t>(Emitter.OrderedOpcodes.size());
			Recounted.iDistributionEvidenceCount +=
				static_cast<uint32_t>(Emitter.Distributions.size());
			for (const EFFECT_CASCADE_INSPECTION_OPCODE& Opcode :
				Emitter.OrderedOpcodes)
			{
				if (Opcode.HandlerReceipt.eClassLineageStatus ==
					EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
						EXACT_CLASS_HANDLER_QUARANTINED)
				{
					++Recounted.iQuarantinedExactClassCount;
				}
				Recounted.iRequiredPropertyCount += static_cast<uint32_t>(
					Opcode.HandlerReceipt.RequiredPropertyReferenceIds.size());
				Recounted.iConsumedPropertyCount += static_cast<uint32_t>(
					Opcode.HandlerReceipt.ConsumedPropertyReferenceIds.size());
				Recounted.iHandlerPropertyReceiptCount +=
					static_cast<uint32_t>(
						Opcode.HandlerReceipt.PropertyConsumption.size());
				if (Opcode.HandlerReceipt.bPayloadAccessAllowed)
					++Recounted.iRawPayloadReadCount;
				if (Opcode.HandlerReceipt.eResult !=
					EFFECT_CASCADE_HANDLER_RESULT::
						STRUCTURE_CONSUMED_EXECUTION_BLOCKED)
				{
					++Recounted.iExecutableOpcodeCount;
				}
			}
			for (const EFFECT_CASCADE_DISTRIBUTION_EVIDENCE& Distribution :
				Emitter.Distributions)
			{
				if (Distribution.bRawPayloadRead)
					++Recounted.iRawPayloadReadCount;
				if (Distribution.bExecutionAllowed)
					++Recounted.iExecutableOpcodeCount;
			}
		}
	}
	Recounted.iBlockerCount =
		static_cast<uint32_t>(Inspection.Blockers.size());
	if (Recounted.iSystemCount != Inspection.Consumption.iSystemCount ||
		Recounted.iDeclaredSourceElementCount !=
			Inspection.Consumption.iDeclaredSourceElementCount ||
		Recounted.iDisabledSourceRecipeCount !=
			Inspection.Consumption.iDisabledSourceRecipeCount ||
		Recounted.iEmitterCount != Inspection.Consumption.iEmitterCount ||
		Recounted.iOrderedOpcodeCount !=
			Inspection.Consumption.iOrderedOpcodeCount ||
		Recounted.iDistributionEvidenceCount !=
			Inspection.Consumption.iDistributionEvidenceCount ||
		Recounted.iRequiredPropertyCount !=
			Inspection.Consumption.iRequiredPropertyCount ||
		Recounted.iConsumedPropertyCount !=
			Inspection.Consumption.iConsumedPropertyCount ||
		Recounted.iQuarantinedExactClassCount !=
			Inspection.Consumption.iQuarantinedExactClassCount ||
		Recounted.iHandlerPropertyReceiptCount !=
			Inspection.Consumption.iHandlerPropertyReceiptCount ||
		Recounted.iRawPayloadReadCount !=
			Inspection.Consumption.iRawPayloadReadCount ||
		Recounted.iExecutableOpcodeCount !=
			Inspection.Consumption.iExecutableOpcodeCount ||
		Recounted.iBlockerCount != Inspection.Consumption.iBlockerCount ||
		Recounted.RendererCounts != Inspection.Consumption.RendererCounts ||
		Inspection.Consumption.iUnknownClassCount != 0u ||
		Inspection.Consumption.iUnconsumedRequiredPropertyCount != 0u ||
		Inspection.Consumption.iRawPayloadReadCount != 0u ||
		Inspection.Consumption.iExecutableOpcodeCount != 0u)
	{
		return false;
	}
	STABLE_HASH Hash;
	Hash_Inspection(Inspection, Hash);
	return Hash.Finish() == Inspection.strInspectionHash;
}

Client::EFFECT_CASCADE_CLASS_REPORT
Client::CEffectCascadeCompiler::Classify_ReceiptClass(
	const std::string_view ReceiptClassKey)
{
	EFFECT_CASCADE_CLASS_REPORT Result;
	Result.strReceiptClassKey = ReceiptClassKey;
	if (const CLASS_SCHEMA* pSchema = Find_ClassSchema(ReceiptClassKey))
	{
		Result.eClassification =
			EFFECT_CASCADE_CLASS_CLASSIFICATION::SUPPORTED_RECEIPT_OPCODE_SCHEMA;
		Result.strOpcodeSchemaId = pSchema->strOpcodeSchemaId;
		Result.strReasonCode = "EXPLICIT_OPCODE_SCHEMA";
		return Result;
	}
	static constexpr std::array<std::string_view, 5u> KnownLegacyGaps = {
		"particlemodulecollision",
		"particlemodulesizemultiplyvelocity",
		"particlemodulesubuvmovie",
		"distributionfloatsoundparameter",
		"distributionvectorconstant"
	};
	if (std::find(KnownLegacyGaps.begin(), KnownLegacyGaps.end(),
		ReceiptClassKey) != KnownLegacyGaps.end())
	{
		Result.eClassification =
			EFFECT_CASCADE_CLASS_CLASSIFICATION::KNOWN_LEGACY_MIGRATION_GAP;
		Result.strReasonCode = "LEGACY_CORPUS_OPCODE_SCHEMA_PENDING";
		return Result;
	}
	Result.eClassification =
		EFFECT_CASCADE_CLASS_CLASSIFICATION::UNKNOWN_REJECTED;
	Result.strReasonCode = "NO_EXPLICIT_ALIAS_OR_EVALUATOR";
	return Result;
}

Client::EFFECT_CASCADE_INSPECTION_COMPILER_PROBE
Client::CEffectCascadeCompiler::Get_Probe()
{
	EFFECT_CASCADE_INSPECTION_COMPILER_PROBE Result;
	Result.iCompileAttemptCount =
		g_iCompileAttemptCount.load(std::memory_order_relaxed);
	Result.iCompileSuccessCount =
		g_iCompileSuccessCount.load(std::memory_order_relaxed);
	Result.iCompileFailureCount =
		g_iCompileFailureCount.load(std::memory_order_relaxed);
	return Result;
}
