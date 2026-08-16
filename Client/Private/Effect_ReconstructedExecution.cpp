#include "Effect_ReconstructedExecution.h"

#include "Effect_DocumentCodec.h"
#include "Effect_Distribution.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct Client::EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA final
{
	enum class MODULE_OPCODE : uint8_t
	{
		REQUIRED_TIMING,
		LIFETIME_TIMING,
		SIZE,
		VELOCITY,
		COLOR_OVER_LIFE,
		ROTATION,
		ACCELERATION,
		ROTATION_RATE,
		SIZE_MULTIPLY_LIFE,
		CYLINDER_Z,
		LOCATION,
		GROUND,
		TYPE_DATA_MESH,
		DYNAMIC_PARAMETER,
		SPAWN_TIMING,
		END
	};

	struct MODULE_OPERATION final
	{
		const EFFECT_RECONSTRUCTED_EXECUTION_MODULE* pPlanModule = nullptr;
		const EFFECT_RUNTIME_PROGRAM_MODULE* pProgramModule = nullptr;
		const EFFECT_RUNTIME_PROGRAM_HANDLER* pHandler = nullptr;
		MODULE_OPCODE eOpcode = MODULE_OPCODE::END;
		std::array<const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION*, 4u>
			Distributions{};
		uint32_t iDistributionCount = 0u;
	};

	struct EMITTER_OPERATION final
	{
		uint32_t iSelectionIndex = 0u;
		const EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE* pPlanSchedule = nullptr;
		const EFFECT_RECONSTRUCTED_EXECUTION_EMITTER* pPlanEmitter = nullptr;
		const EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE* pProgramSchedule = nullptr;
		const EFFECT_RUNTIME_PROGRAM_EMITTER* pProgramEmitter = nullptr;
		std::vector<MODULE_OPERATION> Modules;
		bool_t bCylinderSurfaceOnly = false;
		bool_t bCylinderVelocity = false;
	};

	std::vector<EMITTER_OPERATION> Emitters;
};

namespace
{
	using namespace Client;

	constexpr std::string_view ARTIST_EFFECT_ID =
		"effect.artist.skill.31470";
	constexpr std::string_view ARTIST_ONE_LAYER_RECIPE_ID =
		"material-recipe-3992a9772b12abf2";
	constexpr std::string_view ARTIST_ONE_LAYER_EMISSIVE_SOURCE =
		"fx_tex_03.fx_e_atypical_012";
	constexpr std::string_view ARTIST_ONE_LAYER_NOISE_SOURCE =
		"fx_tex_02.fx_d_noise_009";
	constexpr std::string_view ARTIST_ONE_LAYER_EMISSIVE_ASSET =
		"Effect/Artist/Textures/fx_e_atypical_012.dds";
	constexpr std::string_view ARTIST_ONE_LAYER_NOISE_ASSET =
		"Effect/Artist/Textures/fx_d_noise_009.dds";
	constexpr std::string_view ARTIST_PROGRAM_ID =
		"effect.artist.skill.31470.reconstructed-approved-v1";
	constexpr std::string_view ARTIST_PROGRAM_SHA256 =
		"0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802";
	constexpr std::string_view ARTIST_CANDIDATE_SHA256 =
		"430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6";
	constexpr std::string_view ARTIST_RANDOM_POLICY =
		"DETERMINISTIC_OCCURRENCE_RNG_FROM_SOURCE_CANDIDATE_V1";
	constexpr std::string_view ARTIST_SEED_EVALUATOR =
		"ue3.particle-random-seed-info.current-default.v1";
	constexpr std::string_view FROZEN_DISTRIBUTION_TARGET_PROJECTION_SHA256 =
		"9f99d7a65d6e2a74bc241dd4268751fc876522c9deac387eacfb40be7cc429b1";
	constexpr std::string_view FROZEN_DISTRIBUTION_OWNER_PROJECTION_SHA256 =
		"35b0977b106c003b2542327959dd1ee11ea326dd17324c5dbc242153b086bd88";
	constexpr std::string_view FROZEN_EXECUTION_PLAN_SEMANTIC_PROJECTION_SHA256 =
		"2697c3dc7e929f9e25e4d667dc0d7287dba6e4969b68a8237ce5ec8c6125123e";
	constexpr std::string_view FROZEN_SEEDED_EMITTER_ID =
		"fx_cm_01.distortion_onelayer.par_convatedisol_fsm_pushinghit_01::"
		"action-31470/stage-000/notify-028::FX_CM_01.distortion_onelayer."
		"par_convatedisol_fsm_pushinghit_01.particlespriteemitter_21";
	constexpr std::string_view FROZEN_SEEDED_MODULE_HANDLER_ID =
		"handler-e2090fa9d17f0261c4dd44af";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_HANDLER_ID =
		"handler-bd8a985dfc08db23f1e06bd1";
	constexpr std::string_view FROZEN_INLINE_EVALUATOR_HANDLER_ID =
		"handler-3dbd1f0b4bdedf14ab3e9df0";
	constexpr std::string_view FROZEN_SEEDED_MODULE_HANDLER_CONTRACT_SHA256 =
		"e2090fa9d17f0261c4dd44afcf4f20b0bb679caa4fa15322a4ef925eeb123f55";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_IMPLEMENTATION_ID =
		"source.property.particlemodulelifetime.seeded.lifetime.v1";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_IMPLEMENTATION_SHA256 =
		"43f7adf568c0f021289a92ab7fd1750e665c8437e6aa457e0f39f7390e55fdb5";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_CONTRACT_SHA256 =
		"bd8a985dfc08db23f1e06bd12b6e1bea87dec5aa9e18eb84066ba68d801e8a3c";
	constexpr std::string_view FROZEN_INLINE_EVALUATOR_IMPLEMENTATION_ID =
		"source.distribution.ue3-cooked.v1";
	constexpr std::string_view FROZEN_INLINE_EVALUATOR_IMPLEMENTATION_SHA256 =
		"c9872ce6f39b92b2b0d19e2b593fb74e534ab431a389a9ceba7484844f10f10f";
	constexpr std::string_view FROZEN_INLINE_EVALUATOR_CONTRACT_SHA256 =
		"3dbd1f0b4bdedf14ab3e9df0b8c41e7cce634f656567b257843023a430454edb";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_POLICY_FAMILY_ID =
		"source.reconstructed.seeded.v1";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_ID =
		"source.reconstructed.seeded.v1.implementation";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_SHA256 =
		"ec8c504d703204aa783e8e7271b2b723fbd0e192e28d851ec017946758c79c01";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_FAMILY_SHA256 =
		"720302c18e517a9578d2e7f162bd34e4774326657fd44887980e8a800fad5c9d";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_INPUT_SHA256 =
		"50b290f01ed7fa5f86d2d24940a4a997ea12804e03d7fe83de1afae661c6a58c";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_OUTPUT_SHA256 =
		"aa8e728186d8376aeef859016f816d711bfbba30c3c2e8c27f34cece24aa5bff";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_DEFAULT_SHA256 =
		"37864470fb20da536b2b06b68e99136a6376ba6bfae763b77762b1d8a6d7a913";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_SOURCE_ROW_SHA256 =
		"2f838a77c5a837694974a00b7a07276c7c80fd0baba3e3b7e96bef27252d2712";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_LITERAL_SHA256 =
		"6363595cc429c791d5767a8c06dbfc1c4381313b1f9cab31064e5a10dd796b91";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_DISTRIBUTION_SHA256 =
		"bbd0cb7d1022d6eda8f0d54c1bb7c054afd1a9516b5a1ddcf9ba33979ae0be66";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_PROPERTY_SHA256 =
		"69af836353f5990e6fe6cabb14f4d751426f674d07f59069b34164e7f6c5920f";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_SEED_SHA256 =
		"f0c7afef34462a3c5f600d350a536aa76df684aa99633110afaf874dc70e8802";
	constexpr std::string_view FROZEN_SEEDED_CAPABILITY_ACTION_CUE_SHA256 =
		"4f53cda18c2baa0c0354bb5f9a3ecbe5ed12ab4d8e11ba873c2f11161202b945";
	constexpr std::string_view FROZEN_SEEDED_MODULE_ROW_SHA256 =
		"1eec53947e626922ad5a7f4d8377fe8167bbdbd2fa5618684ddf198160a43bb5";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_ROW_SHA256 =
		"bd9b6e05cf6f055016dcaf4bdc16728f33ffda94d245897383e7b0802b46a21d";
	constexpr std::string_view FROZEN_SEEDED_DISTRIBUTION_ROW_SHA256 =
		"62e3dfc92a0f943bbabee82a1d148a0a8e08a7a04756c00f3cad706c825c5de9";
	constexpr std::string_view FROZEN_SEEDED_POLICY_ROW_SHA256 =
		"1d43e93331e50168277ffbc0ddb7709bdec0bf528541557362feb705cb39ee3a";
	constexpr std::string_view FROZEN_SEEDED_MODULE_HANDLER_ROW_SHA256 =
		"45fadc709610e68ce4894d1ade3c6616c32c80f9529fa2bc3b8f65572d0ce6dd";
	constexpr std::string_view FROZEN_SEEDED_PROPERTY_HANDLER_ROW_SHA256 =
		"19d77a7f214dba5a8a23abc74a3326c33bad4266805e708e2bd5d5a142085030";
	constexpr std::string_view FROZEN_INLINE_EVALUATOR_ROW_SHA256 =
		"d79bb9a07b9e58258186e0305990d190aedaac20650cba685dfa167f13afda42";
	constexpr std::array<uint32_t, 7u> EXPECTED_SCHEDULE_EMITTER_COUNTS{
		4u, 1u, 15u, 12u, 1u, 1u, 1u };
	constexpr std::array<uint32_t, 5u> EXPECTED_DISTRIBUTION_VARIANT_COUNTS{
		612u, 8u, 5u, 1u, 3u };
	constexpr std::array<uint32_t, 2u> EXPECTED_DISTRIBUTION_OWNER_COUNTS{
		509u, 120u };
	constexpr std::array<uint32_t, 6u> EXPECTED_RENDERER_COUNTS{
		16u, 13u, 3u, 1u, 1u, 1u };

	struct COMPILED_PLAN_DATA final
	{
		EFFECT_RECONSTRUCTED_EXECUTION_PLAN_IDENTITY Identity;
		EFFECT_RECONSTRUCTED_EXECUTION_PLAN_SUMMARY Summary;
		EFFECT_RECONSTRUCTED_EXECUTION_SEEDED_LIFETIME_AUTHORITY
			SeededLifetimeAuthority;
		std::vector<std::string> ScheduleOrder;
		std::vector<std::string> EmitterOrder;
		std::vector<std::string> ModuleOrder;
		std::vector<std::string> DistributionOrder;
		std::vector<std::string> SeedPolicyOrder;
		std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE,
			std::less<>> Schedules;
		std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_EMITTER,
			std::less<>> Emitters;
		std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_MODULE,
			std::less<>> Modules;
		std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION,
			std::less<>> Distributions;
		std::map<std::string, EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY,
			std::less<>> SeedPolicies;
	};

	bool_t Reject(std::string& strOutError, const char_t* pError)
	{
		strOutError = pError;
		return false;
	}

	bool_t Is_Finite(const double Value)
	{
		return std::isfinite(Value);
	}

	bool_t Is_Finite(const std::vector<double>& Values)
	{
		return std::all_of(Values.begin(), Values.end(),
			[](const double Value) { return Is_Finite(Value); });
	}

	enum class FROZEN_DISTRIBUTION_OWNER_CLASSIFICATION : uint32_t
	{
		EXACT = 0u,
		BRACKET_DESCENDANT = 1u
	};

	struct FROZEN_DISTRIBUTION_OWNER_PROJECTION final
	{
		std::string strSha256;
		std::array<uint32_t, 2u> ClassificationCounts{};
	};

	std::optional<FROZEN_DISTRIBUTION_OWNER_CLASSIFICATION>
		Classify_FrozenPropertyPathOwner(
		const std::string_view strPropertyPath,
		const std::string_view strDistributionPath)
	{
		if (strPropertyPath == strDistributionPath)
			return FROZEN_DISTRIBUTION_OWNER_CLASSIFICATION::EXACT;
		if (strDistributionPath.size() > strPropertyPath.size() &&
			strDistributionPath.starts_with(strPropertyPath) &&
			strDistributionPath[strPropertyPath.size()] == '[')
		{
			return FROZEN_DISTRIBUTION_OWNER_CLASSIFICATION::BRACKET_DESCENDANT;
		}
		return std::nullopt;
	}

	bool_t Is_Finite(
		const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY& Key)
	{
		const auto Finite4 = [](const std::array<double, 4u>& Values)
		{
			return std::all_of(Values.begin(), Values.end(),
				[](const double Value) { return Is_Finite(Value); });
		};
		return Is_Finite(Key.fTime) && Finite4(Key.vMinimum) &&
			Finite4(Key.vMaximum) && Finite4(Key.vArriveTangentMinimum) &&
			Finite4(Key.vLeaveTangentMinimum) &&
			Finite4(Key.vArriveTangentMaximum) &&
			Finite4(Key.vLeaveTangentMaximum) &&
			(Key.eInterpolation ==
				EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::CUBIC ||
			 Key.eInterpolation ==
				EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::LINEAR);
	}

	void Append_Projection(std::string& Projection, const std::string_view Value)
	{
		Projection += std::to_string(Value.size());
		Projection.push_back(':');
		Projection.append(Value);
		Projection.push_back('|');
	}

	template <typename T>
	void Append_Integer(std::string& Projection, const T Value)
	{
		Append_Projection(Projection, std::to_string(Value));
	}

	void Append_F64(std::string& Projection, const double Value)
	{
		Append_Integer(Projection, std::bit_cast<uint64_t>(Value));
	}

	void Append_OptionalU32(
		std::string& Projection, const std::optional<uint32_t>& Value)
	{
		Append_Integer(Projection, Value.has_value() ? 1u : 0u);
		if (Value.has_value())
			Append_Integer(Projection, *Value);
	}

	void Append_OptionalF64(
		std::string& Projection, const std::optional<double>& Value)
	{
		Append_Integer(Projection, Value.has_value() ? 1u : 0u);
		if (Value.has_value())
			Append_F64(Projection, *Value);
	}

	void Append_F64Vector(
		std::string& Projection, const std::vector<double>& Values)
	{
		Append_Integer(Projection, Values.size());
		for (const double Value : Values)
			Append_F64(Projection, Value);
	}

	void Append_StringVector(
		std::string& Projection, const std::vector<std::string>& Values)
	{
		Append_Integer(Projection, Values.size());
		for (const std::string& Value : Values)
			Append_Projection(Projection, Value);
	}

	const std::string& Frozen_SeededModuleId()
	{
		static const std::string Value =
			std::string(FROZEN_SEEDED_EMITTER_ID) + "::module:001";
		return Value;
	}

	const std::string& Frozen_SeededPropertyId()
	{
		static const std::string Value =
			Frozen_SeededModuleId() + "::property:lifetime";
		return Value;
	}

	const std::string& Frozen_SeededDistributionId()
	{
		static const std::string Value =
			Frozen_SeededModuleId() + "::distribution:lifetime";
		return Value;
	}

	const std::string& Frozen_SeededPolicyId()
	{
		static const std::string Value =
			Frozen_SeededModuleId() + "::seed-policy";
		return Value;
	}

	std::string Build_DistributionTargetProjectionSha256(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program)
	{
		std::string Projection;
		Projection.reserve(350'000u);
		for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Distribution :
			Program.Distributions)
		{
			Append_Projection(Projection, Distribution.Row.strId);
			Append_Projection(Projection, Distribution.strModuleId);
			Append_Projection(Projection, Distribution.strPropertyId);
			Append_Projection(Projection, Distribution.strPropertyPath);
		}
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}

	std::optional<FROZEN_DISTRIBUTION_OWNER_PROJECTION>
		Build_DistributionOwnerProjection(
			const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program)
	{
		std::map<std::string, const EFFECT_RUNTIME_PROGRAM_PROPERTY*, std::less<>>
			Properties;
		for (const EFFECT_RUNTIME_PROGRAM_PROPERTY& Property : Program.Properties)
		{
			if (!Properties.emplace(Property.Row.strId, &Property).second)
				return std::nullopt;
		}

		std::string Projection;
		Projection.reserve(450'000u);
		FROZEN_DISTRIBUTION_OWNER_PROJECTION Result;
		for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Distribution :
			Program.Distributions)
		{
			const auto Property = Properties.find(Distribution.strPropertyId);
			if (Property == Properties.end())
				return std::nullopt;
			const auto Classification = Classify_FrozenPropertyPathOwner(
				Property->second->strPropertyPath, Distribution.strPropertyPath);
			if (!Classification.has_value())
				return std::nullopt;
			const size_t ClassificationIndex = static_cast<size_t>(*Classification);
			if (ClassificationIndex >= Result.ClassificationCounts.size())
				return std::nullopt;

			Append_Projection(Projection, Distribution.Row.strId);
			Append_Projection(Projection, Distribution.strModuleId);
			Append_Projection(Projection, Distribution.strPropertyId);
			Append_Projection(Projection, Distribution.strPropertyPath);
			Append_Projection(Projection, Property->second->strModuleId);
			Append_Projection(Projection, Property->second->strPropertyPath);
			Append_Integer(Projection, ClassificationIndex);
			++Result.ClassificationCounts[ClassificationIndex];
		}
		Result.strSha256 =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
		return Result;
	}

	std::string Build_BurstId(
		const std::string& strSpawnModuleId, const uint32_t iBurstIndex)
	{
		std::ostringstream Stream;
		Stream << strSpawnModuleId << "::burst:" << std::setw(3) <<
			std::setfill('0') << iBurstIndex;
		return Stream.str();
	}

	std::string Build_OccurrenceId(
		const std::string& strEmitterId, const uint64_t iSpawnSerial)
	{
		std::ostringstream Stream;
		Stream << strEmitterId << "::occurrence:" << std::setw(16) <<
			std::setfill('0') << iSpawnSerial;
		return Stream.str();
	}

	bool_t Equals_NoCaseAscii(
		const std::string_view Left, const std::string_view Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t Index = 0u; Index < Left.size(); ++Index)
		{
			const unsigned char A = static_cast<unsigned char>(Left[Index]);
			const unsigned char B = static_cast<unsigned char>(Right[Index]);
			const unsigned char LowerA = A >= 'A' && A <= 'Z' ? A + 32u : A;
			const unsigned char LowerB = B >= 'A' && B <= 'Z' ? B + 32u : B;
			if (LowerA != LowerB)
				return false;
		}
		return true;
	}

	bool_t Validate_CatalogIdentity(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& Identity,
		std::string& strOutError)
	{
		if (0u == Identity.iCatalogRevision ||
			Identity.strEffectAssetId != ARTIST_EFFECT_ID ||
			Identity.iArtifactRevision != 1u ||
			Identity.iProgramVersion != 1u ||
			Identity.iInputArtifactCount != 13u ||
			Identity.iCandidateByteCount != 15'121'873u ||
			Identity.strProgramId != ARTIST_PROGRAM_ID ||
			Identity.strProgramSha256 != ARTIST_PROGRAM_SHA256 ||
			Identity.strCandidateRawSha256 != ARTIST_CANDIDATE_SHA256 ||
			Identity.strCompilerRevision !=
				"artist31470.reconstructed-runtime-program-link-v1" ||
			Identity.strCandidateBuilderCommitId !=
				"ddef21a5314eb8c3db891d36f702cfeda3149f20" ||
			Identity.strCandidateBuilderTreeId !=
				"36a36b889dae7be092e0d2f6f3c3aee2c28bc462")
		{
			return Reject(strOutError,
				"Reconstructed execution plan Catalog identity is not frozen.");
		}
		return true;
	}

	bool_t Validate_ProgramIdentity(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string& strOutError)
	{
		if (Program.Identity.strProgramId != ARTIST_PROGRAM_ID ||
			Program.Identity.iProgramVersion != 1u ||
			Program.Identity.strProgramSha256 != ARTIST_PROGRAM_SHA256 ||
			Program.Identity.strCandidateRawSha256 != ARTIST_CANDIDATE_SHA256 ||
			Program.Identity.strBuilderAuthorityCommitId !=
				"ddef21a5314eb8c3db891d36f702cfeda3149f20" ||
			Program.Identity.strBuilderAuthorityTreeId !=
				"36a36b889dae7be092e0d2f6f3c3aee2c28bc462" ||
			Program.strCharacterClass != "ARTIST" || Program.iSkillId != 31470u ||
			Program.strInputSlot != "F" ||
			Program.strRuntimeCatalogAssetId != ARTIST_EFFECT_ID ||
			Program.Handlers.size() != 385u || Program.Emitters.size() != 35u ||
			Program.ActionSchedules.size() != 7u ||
			Program.Modules.size() != 399u ||
			Program.Properties.size() != 1434u ||
			Program.Distributions.size() != 629u ||
			Program.SeedPolicies.size() != 14u ||
			!Program.Admission.bArtifactBinding ||
			!Program.Admission.bPolicyRoute ||
			!Program.Admission.bSourceHandlerSelection ||
			!Program.Admission.bDistributionEvaluatorSelection ||
			!Program.Admission.bMaterialPolicySelection ||
			!Program.Admission.bGeometryBinding ||
			Program.Admission.bSourceExact ||
			Program.Admission.bRuntimeExecution || Program.Admission.bProduct)
		{
			return Reject(strOutError,
				"Reconstructed execution plan Program identity or admission is invalid.");
		}
		return true;
	}

	bool_t Validate_DistributionShape(
		const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Source,
		std::string& strOutError)
	{
		if (Source.Row.strId.empty() || Source.strModuleId.empty() ||
			Source.strPropertyId.empty() || Source.iComponentCount < 1u ||
			Source.iComponentCount > 4u || !Is_Finite(Source.DefaultMinimum) ||
			!Is_Finite(Source.DefaultMaximum) || !Is_Finite(Source.LookupTable) ||
			!Is_Finite(Source.MinimumInput) || !Is_Finite(Source.MaximumInput) ||
			!Is_Finite(Source.MinimumOutput) || !Is_Finite(Source.MaximumOutput) ||
			!Is_Finite(Source.ConstantValues))
		{
			return Reject(strOutError,
				"Reconstructed execution distribution has invalid identity or numeric data.");
		}
		double PreviousCurveTime = -std::numeric_limits<double>::infinity();
		for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY& Key :
			Source.CurveKeys)
		{
			if (!Is_Finite(Key) || Key.fTime <= PreviousCurveTime)
				return Reject(strOutError,
					"Reconstructed execution distribution curve is invalid or unordered.");
			PreviousCurveTime = Key.fTime;
		}
		for (size_t Index = 0u; Index < Source.ActionCueBindings.size(); ++Index)
		{
			const auto& Binding = Source.ActionCueBindings[Index];
			const bool_t Scalar = Binding.eKind ==
				EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR;
			const bool_t Vector = Binding.eKind ==
				EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR;
			if (Binding.strName.empty() || Binding.iSourceIndex < 0 ||
				Binding.iSourceValueByteOffset < 0 ||
				(!Scalar && !Vector) ||
				(Scalar && (!Binding.fScalarValue.has_value() ||
					!Is_Finite(*Binding.fScalarValue) ||
					!Binding.VectorValue.empty())) ||
				(Vector && (Binding.fScalarValue.has_value() ||
					(Binding.VectorValue.size() != 3u &&
					 Binding.VectorValue.size() != 4u) ||
					!Is_Finite(Binding.VectorValue))))
			{
				return Reject(strOutError,
					"Reconstructed execution ActionCue binding is not strictly typed.");
			}
			for (size_t Prior = 0u; Prior < Index; ++Prior)
			{
				if (Equals_NoCaseAscii(Source.ActionCueBindings[Prior].strName,
					Binding.strName))
				{
					return Reject(strOutError,
						"Reconstructed execution ActionCue binding name is duplicated.");
				}
			}
		}

		const bool_t bParameter =
			Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ||
			Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER ||
			Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
		if (bParameter)
		{
			const size_t Count = Source.iComponentCount;
			if (Source.iOperation.has_value() ||
				Source.iRandomLockAxes.has_value() ||
				Source.iLookupTableChunkSize.has_value() ||
				Source.iLookupTableNumElements.has_value() ||
				Source.fLookupTableTimeScale.has_value() ||
				Source.fLookupTableStartTime.has_value() ||
				!Source.bIsDirty.has_value() ||
				Source.ParamModes.size() != Count ||
				Source.MinimumInput.size() != Count ||
				Source.MaximumInput.size() != Count ||
				Source.MinimumOutput.size() != Count ||
				Source.MaximumOutput.size() != Count ||
				Source.ConstantValues.size() != Count)
			{
				return Reject(strOutError,
					"Reconstructed execution parameter distribution shape is invalid.");
			}
			for (const std::string& Mode : Source.ParamModes)
			{
				if (Mode != "dpm_direct" && Mode != "dpm_normal")
					return Reject(strOutError,
						"Reconstructed execution parameter mode is unsupported.");
			}
			const bool_t Multiply = Source.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
			if ((!Multiply &&
					(!Source.DefaultMinimum.empty() ||
					 !Source.DefaultMaximum.empty() ||
					 !Source.LookupTable.empty() || !Source.CurveKeys.empty())) ||
				(Multiply &&
					(Source.DefaultMinimum.size() != 4u ||
					 Source.DefaultMaximum.size() != 4u ||
					 !Source.LookupTable.empty() || !Source.CurveKeys.empty())))
			{
				return Reject(strOutError,
					"Reconstructed parameter distribution carries an unknown fallback payload.");
			}
			return true;
		}

		if (Source.eVariant != EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
			Source.eVariant != EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE)
		{
			return Reject(strOutError,
				"Reconstructed execution distribution variant is unsupported.");
		}
		if (!Source.iOperation.has_value() || *Source.iOperation > 3u ||
			!Source.iRandomLockAxes.has_value() || *Source.iRandomLockAxes > 4u ||
			!Source.iLookupTableChunkSize.has_value() ||
			!Source.iLookupTableNumElements.has_value() ||
			!Source.fLookupTableTimeScale.has_value() ||
			!Source.fLookupTableStartTime.has_value() ||
			Source.bIsDirty.has_value() || Source.DefaultMinimum.size() != 4u ||
			Source.DefaultMaximum.size() != 4u)
		{
			return Reject(strOutError,
				"Reconstructed execution inline distribution shape is invalid.");
		}
		if (!Source.LookupTable.empty())
		{
			const size_t RequiredChunk = static_cast<size_t>(
				Source.iComponentCount) * (*Source.iOperation >= 2u ? 2u : 1u);
			if (*Source.iLookupTableChunkSize != RequiredChunk ||
				Source.LookupTable.size() < 2u + RequiredChunk ||
				0u != (Source.LookupTable.size() - 2u) % RequiredChunk)
			{
				return Reject(strOutError,
					"Reconstructed execution lookup table shape is invalid.");
			}
		}
		if (!Source.LookupTable.empty() && !Source.CurveKeys.empty())
			return Reject(strOutError,
				"Reconstructed execution distribution has ambiguous table and curve authority.");
		return true;
	}

	std::array<double, 4u> Select_Range(
		const std::array<double, 4u>& Minimum,
		const std::array<double, 4u>& Maximum,
		const uint32_t iOperation,
		const std::array<double, 4u>& RandomUnits)
	{
		std::array<double, 4u> Result = Minimum;
		if (2u == iOperation)
		{
			for (size_t Index = 0u; Index < Result.size(); ++Index)
			{
				Result[Index] = std::lerp(Minimum[Index], Maximum[Index],
					std::clamp(RandomUnits[Index], 0.0, 1.0));
			}
		}
		else if (3u == iOperation && RandomUnits[0u] >= 0.5)
		{
			Result = Maximum;
		}
		return Result;
	}

	std::array<double, 4u> Read_TableValue(
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Distribution,
		const size_t iEntry,
		const bool_t bMaximum)
	{
		std::array<double, 4u> Result{};
		const size_t ComponentCount = Distribution.iComponentCount;
		const size_t ChunkSize = *Distribution.iLookupTableChunkSize;
		const size_t Offset = 2u + iEntry * ChunkSize +
			(bMaximum && ChunkSize >= ComponentCount * 2u ? ComponentCount : 0u);
		for (size_t Index = 0u; Index < ComponentCount; ++Index)
			Result[Index] = Distribution.LookupTable[Offset + Index];
		return Result;
	}

	std::array<double, 4u> Lerp4(
		const std::array<double, 4u>& A,
		const std::array<double, 4u>& B,
		const double T)
	{
		std::array<double, 4u> Result{};
		for (size_t Index = 0u; Index < Result.size(); ++Index)
			Result[Index] = std::lerp(A[Index], B[Index], T);
		return Result;
	}

	std::array<double, 4u> Hermite4(
		const std::array<double, 4u>& Start,
		const std::array<double, 4u>& StartTangent,
		const std::array<double, 4u>& End,
		const std::array<double, 4u>& EndTangent,
		const double T,
		const double Duration)
	{
		const double T2 = T * T;
		const double T3 = T2 * T;
		const double H00 = 2.0 * T3 - 3.0 * T2 + 1.0;
		const double H10 = T3 - 2.0 * T2 + T;
		const double H01 = -2.0 * T3 + 3.0 * T2;
		const double H11 = T3 - T2;
		std::array<double, 4u> Result{};
		for (size_t Index = 0u; Index < Result.size(); ++Index)
		{
			Result[Index] = H00 * Start[Index] +
				H10 * Duration * StartTangent[Index] + H01 * End[Index] +
				H11 * Duration * EndTangent[Index];
		}
		return Result;
	}

	std::optional<std::vector<double>> Resolve_BoundParameter(
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Distribution)
	{
		for (const EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE& Binding :
			Distribution.ActionCueBindings)
		{
			if (!Equals_NoCaseAscii(Binding.strName,
				Distribution.strParameterName))
			{
				continue;
			}
			if (Distribution.iComponentCount == 1u &&
				Binding.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR &&
				Binding.fScalarValue.has_value())
			{
				return std::vector<double>{ *Binding.fScalarValue };
			}
			if (Distribution.iComponentCount > 1u &&
				Binding.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR &&
				Binding.VectorValue.size() >= Distribution.iComponentCount)
			{
				return std::vector<double>(Binding.VectorValue.begin(),
					Binding.VectorValue.begin() + Distribution.iComponentCount);
			}
		}
		return std::nullopt;
	}

	std::array<double, 4u> Evaluate_Parameter(
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Distribution,
		const std::optional<std::vector<double>>& ExplicitInput)
	{
		std::array<double, 4u> Result{};
		const std::optional<std::vector<double>> Input =
			ExplicitInput.has_value() ? ExplicitInput :
			Resolve_BoundParameter(Distribution);
		for (size_t Index = 0u; Index < Distribution.iComponentCount; ++Index)
		{
			if (!Input.has_value() || Input->size() <= Index)
			{
				Result[Index] = Distribution.ConstantValues[Index];
				continue;
			}
			const double Value = (*Input)[Index];
			if (Distribution.ParamModes[Index] == "dpm_direct")
			{
				Result[Index] = Value;
			}
			else
			{
				const double InputRange = Distribution.MaximumInput[Index] -
					Distribution.MinimumInput[Index];
				const double Ratio = InputRange == 0.0 ? 0.0 : std::clamp(
					(Value - Distribution.MinimumInput[Index]) / InputRange,
					0.0, 1.0);
				Result[Index] = std::lerp(Distribution.MinimumOutput[Index],
					Distribution.MaximumOutput[Index], Ratio);
			}
		}
		return Result;
	}

	std::array<double, 4u> Evaluate_Distribution(
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Distribution,
		const double fTime,
		const std::array<double, 4u>& RandomUnits,
		const std::optional<std::vector<double>>& ExplicitParameter = std::nullopt)
	{
		if (Distribution.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ||
			Distribution.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER)
		{
			return Evaluate_Parameter(Distribution, ExplicitParameter);
		}
		if (Distribution.eVariant ==
			EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY)
		{
			std::array<double, 4u> Rejected{};
			Rejected.fill(std::numeric_limits<double>::quiet_NaN());
			return Rejected;
		}

		std::array<double, 4u> Minimum{};
		std::array<double, 4u> Maximum{};
		if (!Distribution.LookupTable.empty())
		{
			const size_t ChunkSize = *Distribution.iLookupTableChunkSize;
			const size_t EntryCount =
				(Distribution.LookupTable.size() - 2u) / ChunkSize;
			const double Lookup = *Distribution.fLookupTableTimeScale > 0.0 ?
				(fTime - *Distribution.fLookupTableStartTime) *
					*Distribution.fLookupTableTimeScale : 0.0;
			const double Clamped = std::clamp(
				Lookup, 0.0, static_cast<double>(EntryCount - 1u));
			const size_t Start = static_cast<size_t>(std::floor(Clamped));
			const size_t End = (std::min)(Start + 1u, EntryCount - 1u);
			const double Ratio = Clamped - static_cast<double>(Start);
			Minimum = Lerp4(Read_TableValue(Distribution, Start, false),
				Read_TableValue(Distribution, End, false), Ratio);
			Maximum = Lerp4(Read_TableValue(Distribution, Start, true),
				Read_TableValue(Distribution, End, true), Ratio);
		}
		else if (!Distribution.CurveKeys.empty())
		{
			const auto Copy4 = [](const std::array<double, 4u>& Value)
			{
				return Value;
			};
			const auto& Keys = Distribution.CurveKeys;
			if (fTime <= Keys.front().fTime)
			{
				Minimum = Copy4(Keys.front().vMinimum);
				Maximum = Copy4(Keys.front().vMaximum);
			}
			else if (fTime >= Keys.back().fTime)
			{
				Minimum = Copy4(Keys.back().vMinimum);
				Maximum = Copy4(Keys.back().vMaximum);
			}
			else
			{
				for (size_t Index = 0u; Index + 1u < Keys.size(); ++Index)
				{
					const auto& Start = Keys[Index];
					const auto& End = Keys[Index + 1u];
					if (fTime > End.fTime)
						continue;
					const double Duration = End.fTime - Start.fTime;
					const double Ratio = Duration <= 0.0 ? 0.0 :
						(fTime - Start.fTime) / Duration;
					if (Start.eInterpolation ==
						EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::CUBIC)
					{
						Minimum = Hermite4(Start.vMinimum,
							Start.vLeaveTangentMinimum, End.vMinimum,
							End.vArriveTangentMinimum, Ratio, Duration);
						Maximum = Hermite4(Start.vMaximum,
							Start.vLeaveTangentMaximum, End.vMaximum,
							End.vArriveTangentMaximum, Ratio, Duration);
					}
					else
					{
						Minimum = Lerp4(Start.vMinimum, End.vMinimum, Ratio);
						Maximum = Lerp4(Start.vMaximum, End.vMaximum, Ratio);
					}
					break;
				}
			}
		}
		else
		{
			std::copy_n(Distribution.DefaultMinimum.begin(), 4u,
				Minimum.begin());
			std::copy_n(Distribution.DefaultMaximum.begin(), 4u,
				Maximum.begin());
		}
		return Select_Range(Minimum, Maximum, *Distribution.iOperation,
			RandomUnits);
	}

	bool_t Numeric_Close(
		const double Actual, const double Expected,
		const double AbsoluteTolerance, const double RelativeTolerance)
	{
		return std::abs(Actual - Expected) <=
			AbsoluteTolerance + RelativeTolerance * std::abs(Expected);
	}

	bool_t Validate_DistributionSamples(
		const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Source,
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Compiled,
		uint32_t& iInOutSampleCount,
		uint32_t& iInOutEvaluatorSampleCount,
		uint32_t& iInOutParameterSampleCount,
		std::string& strOutError)
	{
		for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_SAMPLE& Sample :
			Source.Samples)
		{
			if (!Is_Finite(Sample.fTime) ||
				!Is_Finite(Sample.RandomUnits) ||
				!Is_Finite(Sample.OutputValues) ||
				!Is_Finite(Sample.fAbsoluteTolerance) ||
				!Is_Finite(Sample.fRelativeTolerance) ||
				Sample.fAbsoluteTolerance < 0.0 ||
				Sample.fRelativeTolerance < 0.0)
			{
				return Reject(strOutError,
					"Reconstructed execution distribution sample is not finite.");
			}
			++iInOutSampleCount;
			std::array<double, 4u> RandomUnits{};
			for (size_t Index = 0u;
				Index < Sample.RandomUnits.size() && Index < RandomUnits.size();
				++Index)
			{
				RandomUnits[Index] = Sample.RandomUnits[Index];
			}
			std::optional<std::vector<double>> Parameter;
			if (Sample.eDomain ==
				EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::DISTRIBUTION_EVALUATOR)
			{
				if (Sample.eInputVariant !=
						EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::
							TIME_RANDOM_UNITS ||
					Sample.RandomUnits.size() != 4u ||
					Sample.eBranch.has_value() || Sample.ParameterInput.has_value() ||
					Sample.OutputValues.size() != 4u)
				{
					return Reject(strOutError,
						"Reconstructed execution evaluator sample shape is invalid.");
				}
				++iInOutEvaluatorSampleCount;
			}
			else if (Sample.eDomain ==
				EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::PARTICLE_PARAMETER_BRANCH)
			{
				if (Sample.eInputVariant !=
						EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::
							PARTICLE_PARAMETER_INPUT ||
					!Sample.RandomUnits.empty() || !Sample.eBranch.has_value() ||
					(*Sample.eBranch !=
							EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::PARAMETER_INPUT &&
					 *Sample.eBranch !=
							EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::CONSTANT_FALLBACK) ||
					Sample.OutputValues.size() != Compiled.iComponentCount ||
					(Sample.ParameterInput.has_value() !=
						(*Sample.eBranch ==
						 EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::PARAMETER_INPUT)))
				{
					return Reject(strOutError,
						"Reconstructed execution parameter sample shape is invalid.");
				}
				++iInOutParameterSampleCount;
				if (Sample.ParameterInput.has_value())
				{
					const EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT& Input =
						*Sample.ParameterInput;
					if (Input.strName.empty() ||
						!Equals_NoCaseAscii(
							Input.strName, Compiled.strParameterName) ||
						Input.iSourceIndex < 0 ||
						Input.iSourceValueByteOffset < 0)
					{
						return Reject(strOutError,
							"Reconstructed execution parameter sample identity is invalid.");
					}
					if (Input.eKind ==
						EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR &&
						Input.fScalarValue.has_value() &&
						Is_Finite(*Input.fScalarValue) && Input.VectorValue.empty())
					{
						Parameter = std::vector<double>{ *Input.fScalarValue };
					}
					else if (Input.eKind ==
							EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR &&
						!Input.fScalarValue.has_value() &&
						Input.VectorValue.size() == Compiled.iComponentCount &&
						Is_Finite(Input.VectorValue))
					{
						Parameter = Input.VectorValue;
					}
					else
					{
						return Reject(strOutError,
							"Reconstructed execution parameter sample value is not strictly typed.");
					}
				}
			}
			else
			{
				return Reject(strOutError,
					"Reconstructed execution distribution sample domain is unknown.");
			}
			const std::array<double, 4u> Actual = Evaluate_Distribution(
				Compiled, Sample.fTime, RandomUnits, Parameter);
			if (Sample.OutputValues.size() > Actual.size())
				return Reject(strOutError,
					"Reconstructed execution distribution sample arity is invalid.");
			for (size_t Index = 0u; Index < Sample.OutputValues.size(); ++Index)
			{
				if (!Numeric_Close(Actual[Index], Sample.OutputValues[Index],
					Sample.fAbsoluteTolerance, Sample.fRelativeTolerance))
				{
					return Reject(strOutError,
						"Reconstructed execution distribution numeric oracle mismatch.");
				}
			}
		}
		return true;
	}

	std::string Compute_SemanticProjection(const COMPILED_PLAN_DATA& Data)
	{
		std::string Projection;
		Projection.reserve(1024u * 1024u);
		Append_Integer(Projection, EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION);
		Append_Integer(Projection, EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION);
		Append_Integer(Projection, EFFECT_RECONSTRUCTED_FIXED_STEP_HZ);
		Append_Projection(Projection, Data.Identity.strEffectAssetId);
		Append_Projection(Projection, Data.Identity.strProgramId);
		Append_Integer(Projection, Data.Identity.iProgramVersion);
		Append_Projection(Projection, Data.Identity.strProgramSha256);
		const auto& Seeded = Data.SeededLifetimeAuthority;
		Append_Projection(Projection, Seeded.strEmitterId);
		Append_Projection(Projection, Seeded.strModuleId);
		Append_Projection(Projection, Seeded.strPropertyId);
		Append_Projection(Projection, Seeded.strDistributionId);
		Append_Projection(Projection, Seeded.strSeedPolicyId);
		Append_Projection(Projection, Seeded.strModuleHandlerRegistryId);
		Append_Projection(Projection, Seeded.strPropertyHandlerRegistryId);
		Append_Projection(Projection, Seeded.strEvaluatorRegistryId);
		Append_Projection(Projection, Seeded.strCapabilityPolicyFamilyId);
		Append_Projection(Projection, Seeded.strCapabilityImplementationId);
		Append_Integer(Projection,
			Seeded.iCapabilityImplementationVersion);
		Append_Projection(Projection,
			Seeded.strCapabilityImplementationSha256);
		Append_Projection(Projection,
			Seeded.strCapabilityFamilySemanticSha256);
		Append_Projection(Projection, Seeded.strCapabilityInputSchemaSha256);
		Append_Projection(Projection, Seeded.strCapabilityOutputSchemaSha256);
		Append_Projection(Projection, Seeded.strCapabilityDefaultPolicySha256);
		Append_Projection(Projection, Seeded.strModuleRowSha256);
		Append_Projection(Projection, Seeded.strPropertyRowSha256);
		Append_Projection(Projection, Seeded.strDistributionRowSha256);
		Append_Projection(Projection, Seeded.strSeedPolicyRowSha256);
		Append_Projection(Projection, Seeded.strModuleHandlerRowSha256);
		Append_Projection(Projection, Seeded.strPropertyHandlerRowSha256);
		Append_Projection(Projection, Seeded.strEvaluatorRowSha256);
		for (const std::string& Id : Data.ScheduleOrder)
		{
			const auto& Row = Data.Schedules.at(Id);
			Append_Projection(Projection, Row.strScheduleId);
			Append_Integer(Projection, Row.iOrder);
			Append_Projection(Projection, Row.strSourceCueId);
			Append_Projection(Projection, Row.strSourceOccurrenceId);
			Append_Projection(Projection, Row.strSourceSystemId);
			Append_Integer(Projection, Row.iSourceReceiptEventIndex);
			Append_F64(Projection, Row.fGlobalTimeSeconds);
			Append_F64(Projection, Row.fDurationSeconds);
			Append_StringVector(Projection, Row.EmitterIds);
		}
		for (const std::string& Id : Data.EmitterOrder)
		{
			const auto& Row = Data.Emitters.at(Id);
			Append_Projection(Projection, Row.strEmitterId);
			Append_Integer(Projection, Row.iOrder);
			Append_Projection(Projection, Row.strScheduleId);
			Append_Integer(Projection, static_cast<uint32_t>(Row.eRenderer));
			Append_Integer(Projection, Row.bVisible ? 1u : 0u);
			Append_Integer(Projection, Row.iOperationalMaxParticles);
			Append_Integer(Projection, Row.iRibbonOperationalMaxPoints);
			Append_F64(Projection, Row.fEmitterDelaySeconds);
			Append_F64(Projection, Row.fEmitterDurationSeconds);
			Append_Integer(Projection, Row.iEmitterLoopCount);
			Append_Projection(Projection, Row.strRandomPolicyId);
			Append_Integer(Projection, Row.iEmitterRandomSeed);
			Append_Projection(Projection, Row.strRequiredModuleId);
			Append_Projection(Projection, Row.strSpawnModuleId);
			Append_Projection(Projection, Row.strLifetimeModuleId);
			Append_Projection(Projection, Row.strSpawnRateDistributionId);
			Append_Projection(Projection, Row.strSpawnRateScaleDistributionId);
			Append_Projection(Projection, Row.strLifetimeDistributionId);
			Append_Projection(Projection,
				Row.strLifetimeSeedPolicyId.value_or(std::string{}));
			Append_Integer(Projection, Row.Bursts.size());
			for (const auto& Burst : Row.Bursts)
			{
				Append_Projection(Projection, Burst.strBurstId);
				Append_Integer(Projection, Burst.iBurstIndex);
				Append_F64(Projection, Burst.fTimeSeconds);
				Append_Integer(Projection, Burst.iCountMinimum);
				Append_Integer(Projection, Burst.iCountMaximum);
			}
			Append_StringVector(Projection, Row.ModuleIds);
		}
		for (const std::string& Id : Data.ModuleOrder)
		{
			const auto& Row = Data.Modules.at(Id);
			Append_Projection(Projection, Row.strModuleId);
			Append_Projection(Projection, Row.strEmitterId);
			Append_Integer(Projection, Row.iOrder);
			Append_Projection(Projection, Row.strExactSourceClass);
			Append_Integer(Projection, static_cast<uint32_t>(Row.eSelection));
			Append_Integer(Projection, static_cast<uint32_t>(Row.eRole));
			Append_Projection(Projection, Row.strHandlerRegistryId);
			Append_Projection(Projection, Row.strHandlerVariant);
			Append_Projection(Projection, Row.strHandlerImplementationId);
			Append_Integer(Projection, Row.iHandlerImplementationVersion);
			Append_Projection(Projection, Row.strHandlerImplementationSha256);
			Append_Projection(Projection, Row.strHandlerConsumerContract);
			Append_StringVector(Projection, Row.DistributionIds);
			Append_Projection(Projection,
				Row.strSeedPolicyId.value_or(std::string{}));
		}
		for (const std::string& Id : Data.DistributionOrder)
		{
			const auto& Row = Data.Distributions.at(Id);
			Append_Projection(Projection, Row.strDistributionId);
			Append_Projection(Projection, Row.strModuleId);
			Append_Projection(Projection, Row.strPropertyId);
			Append_Integer(Projection, Row.iOrder);
			Append_Integer(Projection, static_cast<uint32_t>(Row.eVariant));
			Append_Projection(Projection, Row.strSourceClass);
			Append_Projection(Projection, Row.strEvaluatorRegistryId);
			Append_Projection(Projection, Row.strEvaluatorImplementationId);
			Append_Integer(Projection, Row.iEvaluatorImplementationVersion);
			Append_Projection(Projection, Row.strEvaluatorImplementationSha256);
			Append_Projection(Projection, Row.strEvaluatorConsumerContract);
			Append_Integer(Projection, Row.bCpuTimingExecutable ? 1u : 0u);
			Append_Integer(Projection, Row.iComponentCount);
			Append_OptionalU32(Projection, Row.iOperation);
			Append_OptionalU32(Projection, Row.iRandomLockAxes);
			Append_OptionalU32(Projection, Row.iLookupTableChunkSize);
			Append_OptionalU32(Projection, Row.iLookupTableNumElements);
			Append_OptionalF64(Projection, Row.fLookupTableTimeScale);
			Append_OptionalF64(Projection, Row.fLookupTableStartTime);
			Append_F64Vector(Projection, Row.DefaultMinimum);
			Append_F64Vector(Projection, Row.DefaultMaximum);
			Append_F64Vector(Projection, Row.LookupTable);
			Append_Integer(Projection, Row.CurveKeys.size());
			for (const auto& Key : Row.CurveKeys)
			{
				Append_F64(Projection, Key.fTime);
				for (const double Value : Key.vMinimum) Append_F64(Projection, Value);
				for (const double Value : Key.vMaximum) Append_F64(Projection, Value);
				for (const double Value : Key.vArriveTangentMinimum)
					Append_F64(Projection, Value);
				for (const double Value : Key.vLeaveTangentMinimum)
					Append_F64(Projection, Value);
				for (const double Value : Key.vArriveTangentMaximum)
					Append_F64(Projection, Value);
				for (const double Value : Key.vLeaveTangentMaximum)
					Append_F64(Projection, Value);
				Append_Integer(Projection,
					static_cast<uint32_t>(Key.eInterpolation));
			}
			Append_Projection(Projection, Row.strParameterName);
			Append_StringVector(Projection, Row.ParamModes);
			Append_F64Vector(Projection, Row.MinimumInput);
			Append_F64Vector(Projection, Row.MaximumInput);
			Append_F64Vector(Projection, Row.MinimumOutput);
			Append_F64Vector(Projection, Row.MaximumOutput);
			Append_F64Vector(Projection, Row.ConstantValues);
			Append_Integer(Projection,
				Row.bIsDirty.has_value() ? (*Row.bIsDirty ? 2u : 1u) : 0u);
			Append_Integer(Projection, Row.ActionCueBindings.size());
			for (const auto& Binding : Row.ActionCueBindings)
			{
				Append_Projection(Projection, Binding.strName);
				Append_Integer(Projection, static_cast<uint32_t>(Binding.eKind));
				Append_OptionalF64(Projection, Binding.fScalarValue);
				Append_F64Vector(Projection, Binding.VectorValue);
				Append_Integer(Projection, Binding.iSourceIndex);
				Append_Integer(Projection, Binding.iSourceValueByteOffset);
			}
		}
		for (const std::string& Id : Data.SeedPolicyOrder)
		{
			const auto& Row = Data.SeedPolicies.at(Id);
			Append_Projection(Projection, Row.strSeedPolicyId);
			Append_Projection(Projection, Row.strModuleId);
			Append_Integer(Projection, Row.iOrder);
			Append_Projection(Projection, Row.strEvaluatorId);
			Append_Integer(Projection, Row.RandomSeeds.size());
			for (const int32_t Value : Row.RandomSeeds)
				Append_Integer(Projection, Value);
			Append_Projection(Projection,
				Row.strParameterName.value_or(std::string{}));
			Append_Integer(Projection, Row.bGetSeedFromInstance ? 1u : 0u);
			Append_Integer(Projection, Row.bInstanceSeedIsIndex ? 1u : 0u);
			Append_Integer(Projection, Row.bResetSeedOnEmitterLooping ? 1u : 0u);
			Append_Integer(Projection, Row.bRandomlySelectSeedArray ? 1u : 0u);
			Append_Integer(Projection,
				Row.bEmptyArrayUsesOccurrenceRandomStream ? 1u : 0u);
		}
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}

	bool_t Compile_Program(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& CatalogIdentity,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		COMPILED_PLAN_DATA& OutData,
		std::string& strOutError)
	{
		if (!Validate_CatalogIdentity(CatalogIdentity, strOutError) ||
			!Validate_ProgramIdentity(Program, strOutError) ||
			CatalogIdentity.strProgramId != Program.Identity.strProgramId ||
			CatalogIdentity.strProgramSha256 != Program.Identity.strProgramSha256 ||
			CatalogIdentity.strCandidateRawSha256 !=
				Program.Identity.strCandidateRawSha256)
		{
			if (strOutError.empty())
				strOutError =
					"Reconstructed execution Catalog and Program identities disagree.";
			return false;
		}

		COMPILED_PLAN_DATA Data;
		Data.Identity.iPlanVersion = EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION;
		Data.Identity.iOccurrenceRngVersion =
			EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION;
		Data.Identity.iFixedStepHz = EFFECT_RECONSTRUCTED_FIXED_STEP_HZ;
		Data.Identity.iCatalogRevision = CatalogIdentity.iCatalogRevision;
		Data.Identity.strEffectAssetId = CatalogIdentity.strEffectAssetId;
		Data.Identity.strProgramId = CatalogIdentity.strProgramId;
		Data.Identity.iProgramVersion = CatalogIdentity.iProgramVersion;
		Data.Identity.strProgramSha256 = CatalogIdentity.strProgramSha256;
		Data.Identity.strCandidateRawSha256 =
			CatalogIdentity.strCandidateRawSha256;

		std::set<std::string, std::less<>> StableIds;
		const auto RegisterStableId = [&StableIds](const std::string& Id)
		{
			return !Id.empty() && StableIds.emplace(Id).second;
		};
		std::map<std::string, const EFFECT_RUNTIME_PROGRAM_HANDLER*, std::less<>>
			Handlers;
		for (size_t Index = 0u; Index < Program.Handlers.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_HANDLER& Handler =
				Program.Handlers[Index];
			if (Handler.Row.iOrder != Index ||
				!RegisterStableId(Handler.Row.strId) ||
				Handler.strImplementationId.empty() ||
				0u == Handler.iImplementationVersion ||
				Handler.strImplementationSha256.size() != 64u ||
				!Handlers.emplace(Handler.Row.strId, &Handler).second)
			{
				return Reject(strOutError,
					"Reconstructed execution handler identity is invalid or duplicated.");
			}
		}

		double PreviousScheduleTime = -1.0;
		for (size_t Index = 0u; Index < Program.ActionSchedules.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE& Source =
				Program.ActionSchedules[Index];
			if (!RegisterStableId(Source.Row.strId) ||
				Source.Row.iOrder != Index ||
				!Is_Finite(Source.fGlobalTimeSeconds) ||
				!Is_Finite(Source.fDurationSeconds) ||
				Source.fGlobalTimeSeconds < 0.0 || Source.fDurationSeconds < 0.0 ||
				Source.fGlobalTimeSeconds < PreviousScheduleTime)
			{
				return Reject(strOutError,
					"Reconstructed execution schedule identity or order is invalid.");
			}
			PreviousScheduleTime = Source.fGlobalTimeSeconds;
			EFFECT_RECONSTRUCTED_EXECUTION_SCHEDULE Row;
			Row.strScheduleId = Source.Row.strId;
			Row.iOrder = Source.Row.iOrder;
			Row.strSourceCueId = Source.strSourceCueId;
			Row.strSourceOccurrenceId = Source.strSourceOccurrenceId;
			Row.strSourceSystemId = Source.strSourceSystemId;
			Row.iSourceReceiptEventIndex = Source.iSourceReceiptEventIndex;
			Row.fGlobalTimeSeconds = Source.fGlobalTimeSeconds;
			Row.fDurationSeconds = Source.fDurationSeconds;
			if (!Data.Schedules.emplace(Row.strScheduleId, Row).second)
				return Reject(strOutError,
					"Reconstructed execution schedule ID is duplicated.");
			Data.ScheduleOrder.push_back(Row.strScheduleId);
		}

		std::array<uint32_t, 6u> RendererCounts{};
		for (size_t Index = 0u; Index < Program.Emitters.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_EMITTER& Source = Program.Emitters[Index];
			const size_t RendererIndex = static_cast<size_t>(Source.eRenderer);
			const auto Schedule = Data.Schedules.find(Source.strScheduleId);
			if (!RegisterStableId(Source.Row.strId) ||
				Source.Row.iOrder != Index ||
				RendererIndex >= RendererCounts.size() ||
				Schedule == Data.Schedules.end() ||
				Source.strSourceCueId != Schedule->second.strSourceCueId ||
				Source.strSourceOccurrenceId !=
					Schedule->second.strSourceOccurrenceId ||
				Source.strSourceSystemId != Schedule->second.strSourceSystemId ||
				!Is_Finite(Source.Timing.fEmitterDelaySeconds) ||
				!Is_Finite(Source.Timing.fEmitterDurationSeconds) ||
				Source.Timing.fEmitterDelaySeconds < 0.0 ||
				Source.Timing.fEmitterDurationSeconds <= 0.0 ||
				Source.Timing.iEmitterLoopCount != 1u ||
				Source.iOperationalMaxParticles == 0u ||
				Source.Random.strPolicyId != ARTIST_RANDOM_POLICY ||
				Source.Random.iEmitterRandomSeed == 0u ||
				Source.Timing.strRequiredModuleId.empty() ||
				Source.Timing.strSpawnModuleId.empty() ||
				Source.Timing.strLifetimeModuleId.empty())
			{
				return Reject(strOutError,
					"Reconstructed execution emitter timing, cap, or RNG is invalid.");
			}
			EFFECT_RECONSTRUCTED_EXECUTION_EMITTER Row;
			Row.strEmitterId = Source.Row.strId;
			Row.iOrder = Source.Row.iOrder;
			Row.strScheduleId = Source.strScheduleId;
			Row.strSourceCueId = Source.strSourceCueId;
			Row.strSourceOccurrenceId = Source.strSourceOccurrenceId;
			Row.strSourceSystemId = Source.strSourceSystemId;
			Row.eRenderer = Source.eRenderer;
			Row.bVisible = Source.bVisible;
			Row.iOperationalMaxParticles = Source.iOperationalMaxParticles;
			Row.iRibbonOperationalMaxPoints = Source.RibbonAdapter.has_value() ?
				Source.RibbonAdapter->iOperationalMaxPoints : 0u;
			Row.fEmitterDelaySeconds = Source.Timing.fEmitterDelaySeconds;
			Row.fEmitterDurationSeconds = Source.Timing.fEmitterDurationSeconds;
			Row.iEmitterLoopCount = Source.Timing.iEmitterLoopCount;
			Row.strRandomPolicyId = Source.Random.strPolicyId;
			Row.iEmitterRandomSeed = Source.Random.iEmitterRandomSeed;
			Row.strRequiredModuleId = Source.Timing.strRequiredModuleId;
			Row.strSpawnModuleId = Source.Timing.strSpawnModuleId;
			Row.strLifetimeModuleId = Source.Timing.strLifetimeModuleId;
			Row.ModuleIds = Source.ModuleIds;
			for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Source.Timing.Bursts)
			{
				const std::string BurstId = Build_BurstId(
					Burst.strSpawnModuleId, Burst.iBurstIndex);
				if (Burst.strSpawnModuleId != Row.strSpawnModuleId ||
					Burst.iBurstIndex != Row.Bursts.size() ||
					!RegisterStableId(BurstId) ||
					!Is_Finite(Burst.fTimeSeconds) || Burst.fTimeSeconds < 0.0 ||
					Burst.fTimeSeconds > Row.fEmitterDurationSeconds ||
					Burst.iCountMinimum > Burst.iCountMaximum)
				{
					return Reject(strOutError,
						"Reconstructed execution burst identity or range is invalid.");
				}
				Row.Bursts.push_back({
					BurstId,
					Burst.strSpawnModuleId, Burst.iBurstIndex, Burst.fTimeSeconds,
					Burst.iCountMinimum, Burst.iCountMaximum });
			}
			if (!Data.Emitters.emplace(Row.strEmitterId, Row).second)
				return Reject(strOutError,
					"Reconstructed execution emitter ID is duplicated.");
			Data.EmitterOrder.push_back(Row.strEmitterId);
			Data.Schedules.at(Row.strScheduleId).EmitterIds.push_back(Row.strEmitterId);
			++RendererCounts[RendererIndex];
		}
		if (RendererCounts != EXPECTED_RENDERER_COUNTS)
			return Reject(strOutError,
				"Reconstructed execution renderer family counts are not frozen.");
		for (size_t Index = 0u; Index < Data.ScheduleOrder.size(); ++Index)
		{
			if (Data.Schedules.at(Data.ScheduleOrder[Index]).EmitterIds.size() !=
				EXPECTED_SCHEDULE_EMITTER_COUNTS[Index])
			{
				return Reject(strOutError,
					"Reconstructed execution schedule-to-emitter distribution is invalid.");
			}
		}

		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Source : Program.Modules)
		{
			const auto Handler = Handlers.find(Source.strHandlerRegistryId);
			const bool_t Reconstructed = Source.eSelection ==
				EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER;
			if (!RegisterStableId(Source.Row.strId) ||
				Source.strEmitterId.empty() ||
				!Data.Emitters.contains(Source.strEmitterId) ||
				(Source.eSelection !=
					EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER &&
				 Source.eSelection !=
					EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER) ||
				Handler == Handlers.end() ||
				Handler->second->eKind != (Reconstructed ?
					EFFECT_RUNTIME_HANDLER_KIND::RECONSTRUCTED_MODULE :
					EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE) ||
				Handler->second->strExactSourceClass != Source.strExactSourceClass ||
				Handler->second->strVariant != Source.strHandlerVariant ||
				(Reconstructed &&
					(Handler->second->strImplementationId !=
						Source.strCapabilityImplementationId ||
					 Handler->second->iImplementationVersion !=
						Source.iCapabilityImplementationVersion ||
					 Handler->second->strImplementationSha256 !=
						Source.strCapabilityImplementationSha256)))
			{
				return Reject(strOutError,
					"Reconstructed execution module handler identity or selection is invalid.");
			}
			EFFECT_RECONSTRUCTED_EXECUTION_MODULE Row;
			Row.strModuleId = Source.Row.strId;
			Row.strEmitterId = Source.strEmitterId;
			Row.iOrder = Source.Row.iOrder;
			Row.strExactSourceClass = Source.strExactSourceClass;
			Row.eSelection = Source.eSelection;
			Row.strHandlerRegistryId = Source.strHandlerRegistryId;
			Row.strHandlerVariant = Source.strHandlerVariant;
			Row.strHandlerImplementationId =
				Handler->second->strImplementationId;
			Row.iHandlerImplementationVersion =
				Handler->second->iImplementationVersion;
			Row.strHandlerImplementationSha256 =
				Handler->second->strImplementationSha256;
			Row.strHandlerConsumerContract =
				Handler->second->strConsumerContract;
			Row.DistributionIds = Source.DistributionIds;
			if (!Source.strSeedPolicyId.empty())
				Row.strSeedPolicyId = Source.strSeedPolicyId;
			if (!Data.Modules.emplace(Row.strModuleId, Row).second)
				return Reject(strOutError,
					"Reconstructed execution module ID is duplicated.");
			Data.ModuleOrder.push_back(Row.strModuleId);
		}
		std::set<std::string, std::less<>> ReferencedModuleIds;
		for (const std::string& EmitterId : Data.EmitterOrder)
		{
			auto& Emitter = Data.Emitters.at(EmitterId);
			for (size_t Index = 0u; Index < Emitter.ModuleIds.size(); ++Index)
			{
				const auto Iterator = Data.Modules.find(Emitter.ModuleIds[Index]);
				if (Iterator == Data.Modules.end() ||
					!ReferencedModuleIds.emplace(Emitter.ModuleIds[Index]).second ||
					Iterator->second.strEmitterId != EmitterId ||
					Iterator->second.iOrder != Index)
				{
					return Reject(strOutError,
						"Reconstructed execution module reverse owner or order is invalid.");
				}
			}
		}
		if (ReferencedModuleIds.size() != Data.Modules.size())
			return Reject(strOutError,
				"Reconstructed execution module reverse membership is incomplete.");

		std::map<std::string, const EFFECT_RUNTIME_PROGRAM_PROPERTY*, std::less<>>
			Properties;
		std::map<std::string, std::vector<std::string>, std::less<>>
			PropertyIdsByModule;
		for (const EFFECT_RUNTIME_PROGRAM_PROPERTY& Property : Program.Properties)
		{
			const size_t ExpectedOrder =
				PropertyIdsByModule[Property.strModuleId].size();
			if (!RegisterStableId(Property.Row.strId) ||
				!Data.Modules.contains(Property.strModuleId) ||
				Property.Row.iOrder != ExpectedOrder ||
				!Properties.emplace(Property.Row.strId, &Property).second)
			{
				return Reject(strOutError,
					"Reconstructed execution property reverse owner is invalid.");
			}
			PropertyIdsByModule[Property.strModuleId].push_back(Property.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Module : Program.Modules)
		{
			if (PropertyIdsByModule[Module.Row.strId] != Module.PropertyIds)
				return Reject(strOutError,
					"Reconstructed execution module property order is invalid.");
		}

		std::map<std::string, std::vector<std::string>, std::less<>>
			DistributionIdsByModule;
		std::map<std::string, std::vector<std::string>, std::less<>>
			DistributionIdsByProperty;
		if (Build_DistributionTargetProjectionSha256(Program) !=
			FROZEN_DISTRIBUTION_TARGET_PROJECTION_SHA256)
		{
			return Reject(strOutError,
				"Reconstructed execution frozen distribution target projection is invalid.");
		}
		const auto FrozenDistributionOwnerProjection =
			Build_DistributionOwnerProjection(Program);
		if (!FrozenDistributionOwnerProjection.has_value() ||
			FrozenDistributionOwnerProjection->ClassificationCounts !=
				EXPECTED_DISTRIBUTION_OWNER_COUNTS ||
			FrozenDistributionOwnerProjection->strSha256 !=
				FROZEN_DISTRIBUTION_OWNER_PROJECTION_SHA256)
		{
			return Reject(strOutError,
				"Reconstructed execution frozen distribution owner projection is invalid.");
		}
		uint32_t DistributionSampleCount = 0u;
		uint32_t EvaluatorSampleCount = 0u;
		uint32_t ParameterSampleCount = 0u;
		for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Source :
			Program.Distributions)
		{
			const std::vector<std::string> BlockedDefaultTokens{
				"DEFAULT_DEPENDENT_DISTRIBUTION_REQUIRES_TYPED_DEFAULT_POLICY",
				"DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED",
				"DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
				"DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
				"INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
			};
			const bool_t BlockedDefaultDistribution =
				Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
				Source.strPayloadStatus == "UNRESOLVED_SEMANTIC_CLOSURE" &&
				Source.strFidelity == "UNRESOLVED_CROSS_REVISION" &&
				Source.PreservedBlockers == BlockedDefaultTokens;
			if (BlockedDefaultDistribution)
			{
				strOutError =
					"Reconstructed execution contains an unresolved default-dependent distribution: " +
					Source.Row.strId + ".";
				return false;
			}
			const auto Property = Properties.find(Source.strPropertyId);
			const auto Handler = Handlers.find(Source.strEvaluatorRegistryId);
			const bool_t Multiply = Source.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
			const std::string_view ExpectedVariant =
				Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ?
					"INLINE" :
				Source.eVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ?
					"FLOAT_PARAMETER" :
				Source.eVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER ?
					"VECTOR_PARAMETER" :
				Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE ?
					"FLOAT_CURVE" : std::string_view{};
			const size_t ExpectedOrder =
				DistributionIdsByModule[Source.strModuleId].size();
			if (!RegisterStableId(Source.Row.strId) ||
				Source.Row.iOrder != ExpectedOrder ||
				!Validate_DistributionShape(Source, strOutError) ||
				!Data.Modules.contains(Source.strModuleId) ||
				Property == Properties.end() ||
				Property->second->strModuleId != Source.strModuleId ||
				!Classify_FrozenPropertyPathOwner(
					Property->second->strPropertyPath,
					Source.strPropertyPath).has_value() ||
				Handler == Handlers.end() ||
				Handler->second->eKind !=
					EFFECT_RUNTIME_HANDLER_KIND::DISTRIBUTION ||
				Handler->second->strExactSourceClass != Source.strSourceClass ||
				(!Multiply && Handler->second->strVariant != ExpectedVariant) ||
				(Multiply &&
					(Handler->second->strImplementationId !=
						Source.strCapabilityImplementationId ||
					 Handler->second->iImplementationVersion !=
						Source.iCapabilityImplementationVersion ||
					 Handler->second->strImplementationSha256 !=
						Source.strCapabilityImplementationSha256 ||
					 Handler->second->strVariant.empty())) ||
				std::find(Property->second->SemanticDistributionIds.begin(),
					Property->second->SemanticDistributionIds.end(), Source.Row.strId) ==
					Property->second->SemanticDistributionIds.end())
			{
				if (strOutError.empty())
				{
					strOutError =
						"Reconstructed execution distribution reverse owner is invalid: " +
						Source.Row.strId + ".";
				}
				return false;
			}
			EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION Row;
			Row.strDistributionId = Source.Row.strId;
			Row.strModuleId = Source.strModuleId;
			Row.strPropertyId = Source.strPropertyId;
			Row.iOrder = Source.Row.iOrder;
			Row.eVariant = Source.eVariant;
			Row.strSourceClass = Source.strSourceClass;
			Row.strEvaluatorRegistryId = Source.strEvaluatorRegistryId;
			Row.strEvaluatorImplementationId =
				Handler->second->strImplementationId;
			Row.iEvaluatorImplementationVersion =
				Handler->second->iImplementationVersion;
			Row.strEvaluatorImplementationSha256 =
				Handler->second->strImplementationSha256;
			Row.strEvaluatorConsumerContract =
				Handler->second->strConsumerContract;
			Row.iComponentCount = Source.iComponentCount;
			Row.iOperation = Source.iOperation;
			Row.iRandomLockAxes = Source.iRandomLockAxes;
			Row.iLookupTableChunkSize = Source.iLookupTableChunkSize;
			Row.iLookupTableNumElements = Source.iLookupTableNumElements;
			Row.fLookupTableTimeScale = Source.fLookupTableTimeScale;
			Row.fLookupTableStartTime = Source.fLookupTableStartTime;
			Row.DefaultMinimum = Source.DefaultMinimum;
			Row.DefaultMaximum = Source.DefaultMaximum;
			Row.LookupTable = Source.LookupTable;
			Row.CurveKeys = Source.CurveKeys;
			Row.strParameterName = Source.strParameterName;
			Row.ParamModes = Source.ParamModes;
			Row.MinimumInput = Source.MinimumInput;
			Row.MaximumInput = Source.MaximumInput;
			Row.MinimumOutput = Source.MinimumOutput;
			Row.MaximumOutput = Source.MaximumOutput;
			Row.ConstantValues = Source.ConstantValues;
			Row.bIsDirty = Source.bIsDirty;
			Row.ActionCueBindings = Source.ActionCueBindings;
			const size_t VariantIndex = static_cast<size_t>(Row.eVariant);
			if (VariantIndex >= Data.Summary.DistributionVariantCounts.size())
				return Reject(strOutError,
					"Reconstructed execution distribution enum is unknown.");
			if (!Validate_DistributionSamples(Source, Row,
				DistributionSampleCount, EvaluatorSampleCount,
				ParameterSampleCount, strOutError))
			{
				return false;
			}
			if (!Data.Distributions.emplace(Row.strDistributionId, Row).second)
				return Reject(strOutError,
					"Reconstructed execution distribution ID is duplicated.");
			Data.DistributionOrder.push_back(Row.strDistributionId);
			DistributionIdsByModule[Row.strModuleId].push_back(Row.strDistributionId);
			DistributionIdsByProperty[Row.strPropertyId].push_back(
				Row.strDistributionId);
			++Data.Summary.DistributionVariantCounts[VariantIndex];
		}
		if (Data.Summary.DistributionVariantCounts !=
				EXPECTED_DISTRIBUTION_VARIANT_COUNTS ||
			DistributionSampleCount != 1852u || EvaluatorSampleCount != 1839u ||
			ParameterSampleCount != 13u)
		{
			return Reject(strOutError,
				"Reconstructed execution distribution denominators are not frozen.");
		}
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Module : Program.Modules)
		{
			if (DistributionIdsByModule[Module.Row.strId] != Module.DistributionIds)
				return Reject(strOutError,
					"Reconstructed execution module distribution order is invalid.");
		}
		for (const EFFECT_RUNTIME_PROGRAM_PROPERTY& Property : Program.Properties)
		{
			if (DistributionIdsByProperty[Property.Row.strId] !=
				Property.SemanticDistributionIds)
			{
				return Reject(strOutError,
					"Reconstructed execution property distribution reverse membership or order is invalid.");
			}
		}

		for (size_t Index = 0u; Index < Program.SeedPolicies.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_SEED_POLICY& Source =
				Program.SeedPolicies[Index];
			if (!RegisterStableId(Source.Row.strId) ||
				Source.Row.iOrder != Index ||
				!Data.Modules.contains(Source.strModuleId) ||
				Data.Modules.at(Source.strModuleId).strSeedPolicyId != Source.Row.strId ||
				Source.strEvaluatorId != ARTIST_SEED_EVALUATOR)
			{
				return Reject(strOutError,
					"Reconstructed execution seed policy owner is invalid.");
			}
			EFFECT_RECONSTRUCTED_EXECUTION_SEED_POLICY Row;
			Row.strSeedPolicyId = Source.Row.strId;
			Row.strModuleId = Source.strModuleId;
			Row.iOrder = Source.Row.iOrder;
			Row.strEvaluatorId = Source.strEvaluatorId;
			Row.RandomSeeds = Source.RandomSeeds;
			Row.strParameterName = Source.strParameterName;
			Row.bGetSeedFromInstance = Source.bGetSeedFromInstance;
			Row.bInstanceSeedIsIndex = Source.bInstanceSeedIsIndex;
			Row.bResetSeedOnEmitterLooping = Source.bResetSeedOnEmitterLooping;
			Row.bRandomlySelectSeedArray = Source.bRandomlySelectSeedArray;
			Row.bEmptyArrayUsesOccurrenceRandomStream =
				Source.bEmptyArrayUsesOccurrenceRandomStream;
			if (!Data.SeedPolicies.emplace(Row.strSeedPolicyId, Row).second)
				return Reject(strOutError,
					"Reconstructed execution seed policy ID is duplicated.");
			Data.SeedPolicyOrder.push_back(Row.strSeedPolicyId);
		}
		if (Data.SeedPolicies.size() != 14u)
			return Reject(strOutError,
				"Reconstructed execution seed policy denominator is not frozen.");

		const auto FrozenModule = std::find_if(
			Program.Modules.begin(), Program.Modules.end(), [](const auto& Row)
			{ return Row.Row.strId == Frozen_SeededModuleId(); });
		const auto FrozenProperty = Properties.find(Frozen_SeededPropertyId());
		const auto FrozenDistribution = std::find_if(
			Program.Distributions.begin(), Program.Distributions.end(),
			[](const auto& Row)
			{ return Row.Row.strId == Frozen_SeededDistributionId(); });
		const auto FrozenSeed = std::find_if(
			Program.SeedPolicies.begin(), Program.SeedPolicies.end(),
			[](const auto& Row)
			{ return Row.Row.strId == Frozen_SeededPolicyId(); });
		const auto FrozenEmitter = Data.Emitters.find(FROZEN_SEEDED_EMITTER_ID);
		const auto ModuleHandler = Handlers.find(FROZEN_SEEDED_MODULE_HANDLER_ID);
		const auto PropertyHandler =
			Handlers.find(FROZEN_SEEDED_PROPERTY_HANDLER_ID);
		const auto EvaluatorHandler =
			Handlers.find(FROZEN_INLINE_EVALUATOR_HANDLER_ID);
		const std::vector<std::string> FrozenPropertyIds{
			Frozen_SeededModuleId() + "::property:randomseedinfo",
			Frozen_SeededPropertyId(),
			Frozen_SeededModuleId() + "::property:lodvalidity" };
		const std::vector<std::string> FrozenDistributionIds{
			Frozen_SeededDistributionId() };
		const bool_t FrozenTupleValid =
			FrozenModule != Program.Modules.end() &&
			FrozenProperty != Properties.end() &&
			FrozenDistribution != Program.Distributions.end() &&
			FrozenSeed != Program.SeedPolicies.end() &&
			FrozenEmitter != Data.Emitters.end() &&
			ModuleHandler != Handlers.end() &&
			PropertyHandler != Handlers.end() &&
			EvaluatorHandler != Handlers.end() &&
			FrozenEmitter->second.iOrder == 33u &&
			FrozenEmitter->second.strLifetimeModuleId == Frozen_SeededModuleId() &&
			FrozenModule->Row.iOrder == 1u &&
			FrozenModule->Row.strRowSha256 == FROZEN_SEEDED_MODULE_ROW_SHA256 &&
			FrozenModule->strEmitterId == FROZEN_SEEDED_EMITTER_ID &&
			FrozenModule->strExactSourceClass == "particlemodulelifetime_seeded" &&
			FrozenModule->eSelection ==
				EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER &&
			FrozenModule->strHandlerRegistryId ==
				FROZEN_SEEDED_MODULE_HANDLER_ID &&
			FrozenModule->strHandlerVariant == "SEEDED_LIFETIME" &&
			FrozenModule->strCapabilityPolicyFamilyId ==
				FROZEN_SEEDED_CAPABILITY_POLICY_FAMILY_ID &&
			FrozenModule->strCapabilityImplementationId ==
				FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_ID &&
			FrozenModule->iCapabilityImplementationVersion == 1u &&
			FrozenModule->strCapabilityImplementationSha256 ==
				FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_SHA256 &&
			FrozenModule->strCapabilityFamilySemanticSha256 ==
				FROZEN_SEEDED_CAPABILITY_FAMILY_SHA256 &&
			FrozenModule->strCapabilityInputSchemaSha256 ==
				FROZEN_SEEDED_CAPABILITY_INPUT_SHA256 &&
			FrozenModule->strCapabilityOutputSchemaSha256 ==
				FROZEN_SEEDED_CAPABILITY_OUTPUT_SHA256 &&
			FrozenModule->strCapabilityDefaultPolicySha256 ==
				FROZEN_SEEDED_CAPABILITY_DEFAULT_SHA256 &&
			FrozenModule->strCapabilitySourceRowSha256 ==
				FROZEN_SEEDED_CAPABILITY_SOURCE_ROW_SHA256 &&
			FrozenModule->strCapabilityLiteralBindingsSha256 ==
				FROZEN_SEEDED_CAPABILITY_LITERAL_SHA256 &&
			FrozenModule->strCapabilityDistributionBindingsSha256 ==
				FROZEN_SEEDED_CAPABILITY_DISTRIBUTION_SHA256 &&
			FrozenModule->strCapabilityPropertyConsumptionSha256 ==
				FROZEN_SEEDED_CAPABILITY_PROPERTY_SHA256 &&
			FrozenModule->strCapabilitySeedBindingSha256 ==
				FROZEN_SEEDED_CAPABILITY_SEED_SHA256 &&
			FrozenModule->CapabilityActionCueInputNames.empty() &&
			FrozenModule->strCapabilityActionCueInputsSha256 ==
				FROZEN_SEEDED_CAPABILITY_ACTION_CUE_SHA256 &&
			FrozenModule->PropertyIds == FrozenPropertyIds &&
			FrozenModule->DistributionIds == FrozenDistributionIds &&
			FrozenModule->strSeedPolicyId == Frozen_SeededPolicyId() &&
			FrozenProperty->second->Row.iOrder == 1u &&
			FrozenProperty->second->Row.strRowSha256 ==
				FROZEN_SEEDED_PROPERTY_ROW_SHA256 &&
			FrozenProperty->second->strModuleId == Frozen_SeededModuleId() &&
			FrozenProperty->second->strPropertyPath == "lifetime" &&
			FrozenProperty->second->strHandlerRegistryId ==
				FROZEN_SEEDED_PROPERTY_HANDLER_ID &&
			FrozenProperty->second->SemanticDistributionIds ==
				FrozenDistributionIds &&
			FrozenDistribution->Row.iOrder == 0u &&
			FrozenDistribution->Row.strRowSha256 ==
				FROZEN_SEEDED_DISTRIBUTION_ROW_SHA256 &&
			FrozenDistribution->strModuleId == Frozen_SeededModuleId() &&
			FrozenDistribution->strPropertyId == Frozen_SeededPropertyId() &&
			FrozenDistribution->strPropertyPath == "lifetime" &&
			FrozenDistribution->eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
			FrozenDistribution->strEvaluatorRegistryId ==
				FROZEN_INLINE_EVALUATOR_HANDLER_ID &&
			FrozenSeed->Row.iOrder == 11u &&
			FrozenSeed->Row.strRowSha256 == FROZEN_SEEDED_POLICY_ROW_SHA256 &&
			FrozenSeed->strModuleId == Frozen_SeededModuleId() &&
			FrozenSeed->strEvaluatorId == ARTIST_SEED_EVALUATOR &&
			FrozenSeed->RandomSeeds == std::vector<int32_t>{ 3 } &&
			!FrozenSeed->strParameterName.has_value() &&
			!FrozenSeed->bGetSeedFromInstance &&
			!FrozenSeed->bInstanceSeedIsIndex &&
			FrozenSeed->bResetSeedOnEmitterLooping &&
			!FrozenSeed->bRandomlySelectSeedArray &&
			FrozenSeed->bEmptyArrayUsesOccurrenceRandomStream &&
			ModuleHandler->second->Row.iOrder == 329u &&
			ModuleHandler->second->Row.strRowSha256 ==
				FROZEN_SEEDED_MODULE_HANDLER_ROW_SHA256 &&
			ModuleHandler->second->eKind ==
				EFFECT_RUNTIME_HANDLER_KIND::RECONSTRUCTED_MODULE &&
			ModuleHandler->second->strImplementationId ==
				FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_ID &&
			ModuleHandler->second->iImplementationVersion == 1u &&
			ModuleHandler->second->strImplementationSha256 ==
				FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_SHA256 &&
			ModuleHandler->second->strExactSourceClass ==
				"particlemodulelifetime_seeded" &&
			ModuleHandler->second->strVariant == "SEEDED_LIFETIME" &&
			ModuleHandler->second->strConsumerContract.empty() &&
			ModuleHandler->second->strContractSha256 ==
				FROZEN_SEEDED_MODULE_HANDLER_CONTRACT_SHA256 &&
			PropertyHandler->second->Row.iOrder == 331u &&
			PropertyHandler->second->Row.strRowSha256 ==
				FROZEN_SEEDED_PROPERTY_HANDLER_ROW_SHA256 &&
			PropertyHandler->second->eKind ==
				EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PROPERTY &&
			PropertyHandler->second->strImplementationId ==
				FROZEN_SEEDED_PROPERTY_IMPLEMENTATION_ID &&
			PropertyHandler->second->iImplementationVersion == 1u &&
			PropertyHandler->second->strImplementationSha256 ==
				FROZEN_SEEDED_PROPERTY_IMPLEMENTATION_SHA256 &&
			PropertyHandler->second->strExactSourceClass ==
				"particlemodulelifetime_seeded" &&
			PropertyHandler->second->strVariant == "lifetime" &&
			PropertyHandler->second->strConsumerContract.empty() &&
			PropertyHandler->second->strContractSha256 ==
				FROZEN_SEEDED_PROPERTY_CONTRACT_SHA256 &&
			EvaluatorHandler->second->Row.iOrder == 22u &&
			EvaluatorHandler->second->Row.strRowSha256 ==
				FROZEN_INLINE_EVALUATOR_ROW_SHA256 &&
			EvaluatorHandler->second->eKind ==
				EFFECT_RUNTIME_HANDLER_KIND::DISTRIBUTION &&
			EvaluatorHandler->second->strImplementationId ==
				FROZEN_INLINE_EVALUATOR_IMPLEMENTATION_ID &&
			EvaluatorHandler->second->iImplementationVersion == 1u &&
			EvaluatorHandler->second->strImplementationSha256 ==
				FROZEN_INLINE_EVALUATOR_IMPLEMENTATION_SHA256 &&
			EvaluatorHandler->second->strExactSourceClass.empty() &&
			EvaluatorHandler->second->strVariant == "INLINE" &&
			EvaluatorHandler->second->strConsumerContract.empty() &&
			EvaluatorHandler->second->strContractSha256 ==
				FROZEN_INLINE_EVALUATOR_CONTRACT_SHA256;
		if (!FrozenTupleValid)
		{
			return Reject(strOutError,
				"Reconstructed execution frozen seeded lifetime authority tuple is invalid.");
		}

		Data.SeededLifetimeAuthority = {
			std::string(FROZEN_SEEDED_EMITTER_ID),
			Frozen_SeededModuleId(),
			Frozen_SeededPropertyId(),
			Frozen_SeededDistributionId(),
			Frozen_SeededPolicyId(),
			std::string(FROZEN_SEEDED_MODULE_HANDLER_ID),
			std::string(FROZEN_SEEDED_PROPERTY_HANDLER_ID),
			std::string(FROZEN_INLINE_EVALUATOR_HANDLER_ID),
			std::string(FROZEN_SEEDED_CAPABILITY_POLICY_FAMILY_ID),
			std::string(FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_ID),
			1u,
			std::string(FROZEN_SEEDED_CAPABILITY_IMPLEMENTATION_SHA256),
			std::string(FROZEN_SEEDED_CAPABILITY_FAMILY_SHA256),
			std::string(FROZEN_SEEDED_CAPABILITY_INPUT_SHA256),
			std::string(FROZEN_SEEDED_CAPABILITY_OUTPUT_SHA256),
			std::string(FROZEN_SEEDED_CAPABILITY_DEFAULT_SHA256),
			std::string(FROZEN_SEEDED_MODULE_ROW_SHA256),
			std::string(FROZEN_SEEDED_PROPERTY_ROW_SHA256),
			std::string(FROZEN_SEEDED_DISTRIBUTION_ROW_SHA256),
			std::string(FROZEN_SEEDED_POLICY_ROW_SHA256),
			std::string(FROZEN_SEEDED_MODULE_HANDLER_ROW_SHA256),
			std::string(FROZEN_SEEDED_PROPERTY_HANDLER_ROW_SHA256),
			std::string(FROZEN_INLINE_EVALUATOR_ROW_SHA256) };

		for (const std::string& EmitterId : Data.EmitterOrder)
		{
			auto& Emitter = Data.Emitters.at(EmitterId);
			auto Required = Data.Modules.find(Emitter.strRequiredModuleId);
			auto Spawn = Data.Modules.find(Emitter.strSpawnModuleId);
			auto Lifetime = Data.Modules.find(Emitter.strLifetimeModuleId);
			const bool_t LifetimeHandlerKnown =
				Lifetime != Data.Modules.end() &&
				((Lifetime->second.strExactSourceClass ==
						"particlemodulelifetime" &&
				  Lifetime->second.eSelection ==
						EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER) ||
				 (Lifetime->second.strExactSourceClass ==
						"particlemodulelifetime_seeded" &&
				  Lifetime->second.eSelection ==
						EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER));
			if (Required == Data.Modules.end() || Spawn == Data.Modules.end() ||
				Lifetime == Data.Modules.end() ||
				Required->second.strEmitterId != EmitterId ||
				Spawn->second.strEmitterId != EmitterId ||
				Lifetime->second.strEmitterId != EmitterId ||
				Required->second.eSelection !=
					EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER ||
				Spawn->second.eSelection !=
					EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER ||
				Required->second.strExactSourceClass != "particlemodulerequired" ||
				Spawn->second.strExactSourceClass != "particlemodulespawn" ||
				!LifetimeHandlerKnown)
			{
				return Reject(strOutError,
					"Reconstructed execution timing module class or owner is unknown.");
			}
			Required->second.eRole =
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::REQUIRED_TIMING;
			Spawn->second.eRole =
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::SPAWN_TIMING;
			Lifetime->second.eRole = Lifetime->second.strExactSourceClass ==
				"particlemodulelifetime_seeded" ?
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::LIFETIME_SEEDED :
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::LIFETIME;

			const auto FindDistribution = [&](const auto& Module,
				const std::string_view PropertyPath) -> std::optional<std::string>
			{
				std::optional<std::string> Result;
				for (const std::string& DistributionId : Module.DistributionIds)
				{
					const auto& Distribution = Data.Distributions.at(DistributionId);
					const auto Property = Properties.find(Distribution.strPropertyId);
					if (Property == Properties.end() ||
						Property->second->strPropertyPath != PropertyPath)
					{
						continue;
					}
					if (Result.has_value())
						return std::nullopt;
					Result = DistributionId;
				}
				return Result;
			};
			const auto Rate = FindDistribution(Spawn->second, "rate");
			const auto RateScale = FindDistribution(Spawn->second, "ratescale");
			const auto Life = FindDistribution(Lifetime->second, "lifetime");
			if (!Rate.has_value() || !RateScale.has_value() || !Life.has_value())
				return Reject(strOutError,
					"Reconstructed execution timing distribution join is missing or ambiguous.");
			Emitter.strSpawnRateDistributionId = *Rate;
			Emitter.strSpawnRateScaleDistributionId = *RateScale;
			Emitter.strLifetimeDistributionId = *Life;
			for (const std::string& DistributionId : { *Rate, *RateScale, *Life })
			{
				auto& Distribution = Data.Distributions.at(DistributionId);
				if (Distribution.eVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY)
				{
					return Reject(strOutError,
						"Reconstructed execution timing cannot use a deferred EF evaluator.");
				}
				Distribution.bCpuTimingExecutable = true;
			}
			if (Lifetime->second.eRole ==
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::LIFETIME_SEEDED)
			{
				if (!Lifetime->second.strSeedPolicyId.has_value() ||
					!Data.SeedPolicies.contains(*Lifetime->second.strSeedPolicyId))
				{
					return Reject(strOutError,
						"Reconstructed seeded lifetime lacks its exact seed policy.");
				}
				Emitter.strLifetimeSeedPolicyId = Lifetime->second.strSeedPolicyId;
				const auto& Seed = Data.SeedPolicies.at(
					*Emitter.strLifetimeSeedPolicyId);
				if (Seed.strEvaluatorId != ARTIST_SEED_EVALUATOR ||
					Seed.RandomSeeds != std::vector<int32_t>{ 3 } ||
					Seed.strParameterName.has_value() ||
					Seed.bGetSeedFromInstance || Seed.bInstanceSeedIsIndex ||
					!Seed.bResetSeedOnEmitterLooping ||
					Seed.bRandomlySelectSeedArray ||
					!Seed.bEmptyArrayUsesOccurrenceRandomStream)
				{
					return Reject(strOutError,
						"Reconstructed seeded lifetime policy is not executable without fallback.");
				}
			}
		}

		Data.Summary.iScheduleCount =
			static_cast<uint32_t>(Data.Schedules.size());
		Data.Summary.iEmitterCount = static_cast<uint32_t>(Data.Emitters.size());
		Data.Summary.iModuleCount = static_cast<uint32_t>(Data.Modules.size());
		Data.Summary.iDistributionCount =
			static_cast<uint32_t>(Data.Distributions.size());
		for (const auto& [EmitterId, Emitter] : Data.Emitters)
		{
			UNREFERENCED_PARAMETER(EmitterId);
			Data.Summary.iBurstCount +=
				static_cast<uint32_t>(Emitter.Bursts.size());
			Data.Summary.iOperationalCapSum += Emitter.iOperationalMaxParticles;
			Data.Summary.iOperationalCapMaximum = (std::max)(
				Data.Summary.iOperationalCapMaximum,
				Emitter.iOperationalMaxParticles);
			Data.Summary.iRibbonOperationalMaxPoints = (std::max)(
				Data.Summary.iRibbonOperationalMaxPoints,
				Emitter.iRibbonOperationalMaxPoints);
			const auto Role = Data.Modules.at(Emitter.strLifetimeModuleId).eRole;
			if (Role == EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::LIFETIME)
				++Data.Summary.iLifetimeModuleCount;
			else if (Role ==
				EFFECT_RECONSTRUCTED_EXECUTION_MODULE_ROLE::LIFETIME_SEEDED)
				++Data.Summary.iSeededLifetimeModuleCount;
		}
		if (Data.Summary.iBurstCount != 31u ||
			Data.Summary.iLifetimeModuleCount != 34u ||
			Data.Summary.iSeededLifetimeModuleCount != 1u ||
			Data.Summary.iOperationalCapSum != 1291u ||
			Data.Summary.iOperationalCapMaximum != 594u ||
			Data.Summary.iRibbonOperationalMaxPoints != 500u)
		{
			return Reject(strOutError,
				"Reconstructed execution timing, lifetime, or cap denominators are not frozen.");
		}

		Data.Identity.strSemanticProjectionSha256 =
			Compute_SemanticProjection(Data);
		if (Data.Identity.strProgramSha256 != ARTIST_PROGRAM_SHA256 ||
			Data.Identity.strCandidateRawSha256 != ARTIST_CANDIDATE_SHA256 ||
			Data.Identity.strSemanticProjectionSha256 !=
				FROZEN_EXECUTION_PLAN_SEMANTIC_PROJECTION_SHA256)
		{
			const std::string strAuthorityError =
				"Reconstructed execution frozen semantic projection authority is invalid: expected=" +
				std::string(FROZEN_EXECUTION_PLAN_SEMANTIC_PROJECTION_SHA256) +
				" actual=" + Data.Identity.strSemanticProjectionSha256 + ".";
			return Reject(strOutError, strAuthorityError.c_str());
		}
		OutData = std::move(Data);
		strOutError.clear();
		return true;
	}

	uint32_t Next_Random(uint32_t& iInOutState)
	{
		uint32_t Value = iInOutState;
		Value ^= Value << 13u;
		Value ^= Value >> 17u;
		Value ^= Value << 5u;
		iInOutState = 0u == Value ? 1u : Value;
		return iInOutState;
	}

	struct TIMING_PARTICLE final
	{
		std::string strOccurrenceId;
		uint32_t iLoopIndex = 0u;
		uint64_t iSpawnSerial = 0u;
		uint64_t iSpawnStep = 0u;
		uint32_t iOccurrenceRandomValue = 0u;
		uint32_t iLifetimeRandomValue = 0u;
		double fSpawnTimeSeconds = 0.0;
		double fLifetimeSeconds = 0.0;
	};

	struct MUTABLE_EMITTER_STATE final
	{
		EFFECT_RECONSTRUCTED_CPU_EMITTER_STATE Public;
		double fSpawnAccumulator = 0.0;
		size_t iNextBurst = 0u;
		std::vector<TIMING_PARTICLE> Particles;
	};

	bool_t Spawn_TimingParticles(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_EXECUTION_EMITTER& Emitter,
		MUTABLE_EMITTER_STATE& State,
		const uint32_t iLoopIndex,
		const uint32_t iRequestedCount,
		const uint64_t iStep,
		const double fTime,
		const double fEmitterTime,
		std::string& strOutError)
	{
		const uint32_t Available = Emitter.iOperationalMaxParticles >
			State.Particles.size() ? Emitter.iOperationalMaxParticles -
				static_cast<uint32_t>(State.Particles.size()) : 0u;
		const uint32_t SpawnCount = (std::min)(Available, iRequestedCount);
		State.Public.iDroppedByCap += iRequestedCount - SpawnCount;
		const auto& LifetimeDistribution =
			Plan.Get_Distributions().at(Emitter.strLifetimeDistributionId);
		if (!LifetimeDistribution.bCpuTimingExecutable)
			return Reject(strOutError,
				"Reconstructed fixed-step lifetime evaluator is not executable.");
		for (uint32_t Index = 0u; Index < SpawnCount; ++Index)
		{
			const uint32_t OccurrenceRandomValue =
				Next_Random(State.Public.iRandomState);
			const uint32_t LifetimeRandomValue =
				Emitter.strLifetimeSeedPolicyId.has_value() ?
					Next_Random(State.Public.iLifetimeRandomState) :
					OccurrenceRandomValue;
			std::array<double, 4u> RandomUnits{};
			RandomUnits[0u] = static_cast<double>(LifetimeRandomValue) /
				static_cast<double>((std::numeric_limits<uint32_t>::max)());
			const double Lifetime = Evaluate_Distribution(
				LifetimeDistribution, fEmitterTime, RandomUnits)[0u];
			if (!Is_Finite(Lifetime) || Lifetime <= 0.0)
				return Reject(strOutError,
					"Reconstructed fixed-step refuses a non-positive lifetime fallback.");
			TIMING_PARTICLE Particle;
			Particle.iLoopIndex = iLoopIndex;
			Particle.iSpawnSerial = State.Public.iNextSpawnSerial++;
			Particle.strOccurrenceId = Build_OccurrenceId(
				Emitter.strEmitterId, Particle.iSpawnSerial);
			Particle.iSpawnStep = iStep;
			Particle.iOccurrenceRandomValue = OccurrenceRandomValue;
			Particle.iLifetimeRandomValue = LifetimeRandomValue;
			Particle.fSpawnTimeSeconds = fTime;
			Particle.fLifetimeSeconds = Lifetime;
			State.Particles.push_back(std::move(Particle));
			++State.Public.iSpawnedTotal;
		}
		return true;
	}

	template <size_t Count>
	bool_t Is_Finite(const std::array<double, Count>& Values)
	{
		return std::all_of(Values.begin(), Values.end(),
			[](const double Value) { return Is_Finite(Value); });
	}

	struct FIXED_STEP_SIMULATION final
	{
		uint64_t iFixedStepIndex = 0u;
		double fSampleTimeSeconds = 0.0;
		std::map<std::string, MUTABLE_EMITTER_STATE, std::less<>> States;
	};

	bool_t Simulate_FixedSteps(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const uint64_t iTargetSteps,
		FIXED_STEP_SIMULATION& OutSimulation,
		std::string& strOutError)
	{
		if (iTargetSteps >
			static_cast<uint64_t>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ) * 60u)
		{
			return Reject(strOutError,
				"Reconstructed fixed-step target exceeds the bounded window.");
		}
		FIXED_STEP_SIMULATION Staged;
		Staged.iFixedStepIndex = iTargetSteps;
		Staged.fSampleTimeSeconds = static_cast<double>(iTargetSteps) /
			static_cast<double>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ);
		for (const std::string& EmitterId : Plan.Get_EmitterOrder())
		{
			const auto EmitterIt = Plan.Get_Emitters().find(EmitterId);
			if (EmitterIt == Plan.Get_Emitters().end())
				return Reject(strOutError,
					"Reconstructed fixed-step emitter order is invalid.");
			const auto& Emitter = EmitterIt->second;
			MUTABLE_EMITTER_STATE State;
			State.Public.strEmitterId = EmitterId;
			State.Public.strScheduleId = Emitter.strScheduleId;
			State.Public.iRandomState = Emitter.iEmitterRandomSeed;
			if (Emitter.strLifetimeSeedPolicyId.has_value())
			{
				const auto SeedIt = Plan.Get_SeedPolicies().find(
					*Emitter.strLifetimeSeedPolicyId);
				if (SeedIt == Plan.Get_SeedPolicies().end() ||
					SeedIt->second.RandomSeeds.empty())
				{
					return Reject(strOutError,
						"Reconstructed fixed-step lifetime seed is invalid.");
				}
				State.Public.iLifetimeRandomState = static_cast<uint32_t>(
					SeedIt->second.RandomSeeds.front());
				if (0u == State.Public.iLifetimeRandomState)
					State.Public.iLifetimeRandomState = 1u;
			}
			Staged.States.emplace(EmitterId, std::move(State));
		}

		constexpr double FixedStep =
			1.0 / static_cast<double>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ);
		constexpr double StepEpsilon = 1.0e-9;
		for (uint64_t Step = 1u; Step <= iTargetSteps; ++Step)
		{
			const double Time = static_cast<double>(Step) * FixedStep;
			for (const std::string& ScheduleId : Plan.Get_ScheduleOrder())
			{
				const auto ScheduleIt = Plan.Get_Schedules().find(ScheduleId);
				if (ScheduleIt == Plan.Get_Schedules().end())
					return Reject(strOutError,
						"Reconstructed fixed-step schedule order is invalid.");
				const auto& Schedule = ScheduleIt->second;
				for (const std::string& EmitterId : Schedule.EmitterIds)
				{
					const auto EmitterIt = Plan.Get_Emitters().find(EmitterId);
					const auto StateIt = Staged.States.find(EmitterId);
					if (EmitterIt == Plan.Get_Emitters().end() ||
						StateIt == Staged.States.end())
					{
						return Reject(strOutError,
							"Reconstructed fixed-step schedule emitter is invalid.");
					}
					const auto& Emitter = EmitterIt->second;
					auto& State = StateIt->second;
					std::erase_if(State.Particles,
						[Time, StepEpsilon](const TIMING_PARTICLE& Particle)
						{
							return Time - Particle.fSpawnTimeSeconds + StepEpsilon >=
								Particle.fLifetimeSeconds;
						});
					State.Public.iActiveCount =
						static_cast<uint32_t>(State.Particles.size());
					const double ScheduleElapsed =
						Time - Schedule.fGlobalTimeSeconds;
					if (ScheduleElapsed < 0.0)
					{
						State.Public.ePhase =
							EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::WAITING_FOR_SCHEDULE;
						continue;
					}
					const double EmitterElapsed =
						ScheduleElapsed - Emitter.fEmitterDelaySeconds;
					if (EmitterElapsed < 0.0)
					{
						State.Public.ePhase =
							EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::WAITING_FOR_DELAY;
						continue;
					}
					const uint32_t LoopIndex = static_cast<uint32_t>(std::floor(
						EmitterElapsed / Emitter.fEmitterDurationSeconds));
					const bool_t LoopAllowed = Emitter.iEmitterLoopCount == 0u ||
						LoopIndex < Emitter.iEmitterLoopCount;
					if (!LoopAllowed)
					{
						State.Public.ePhase =
							EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::COMPLETE;
					}
					else
					{
						State.Public.ePhase =
							EFFECT_RECONSTRUCTED_CPU_EMITTER_PHASE::EMITTING;
						if (LoopIndex != State.Public.iLoopIndex)
						{
							State.Public.iLoopIndex = LoopIndex;
							State.iNextBurst = 0u;
							State.fSpawnAccumulator = 0.0;
							if (Emitter.strLifetimeSeedPolicyId.has_value())
							{
								const auto& Seed = Plan.Get_SeedPolicies().at(
									*Emitter.strLifetimeSeedPolicyId);
								if (Seed.bResetSeedOnEmitterLooping)
								{
									State.Public.iLifetimeRandomState =
										static_cast<uint32_t>(Seed.RandomSeeds.front());
									if (0u == State.Public.iLifetimeRandomState)
										State.Public.iLifetimeRandomState = 1u;
								}
							}
						}
						const double EmitterTime = EmitterElapsed -
							static_cast<double>(LoopIndex) *
								Emitter.fEmitterDurationSeconds;
						while (State.iNextBurst < Emitter.Bursts.size() &&
							Emitter.Bursts[State.iNextBurst].fTimeSeconds <=
								EmitterTime + 0.5 * FixedStep)
						{
							const auto& Burst = Emitter.Bursts[State.iNextBurst++];
							uint32_t Count = Burst.iCountMinimum;
							if (Burst.iCountMaximum > Burst.iCountMinimum)
							{
								const uint64_t Span =
									static_cast<uint64_t>(Burst.iCountMaximum) -
									Burst.iCountMinimum + 1u;
								Count += static_cast<uint32_t>(
									Next_Random(State.Public.iRandomState) % Span);
							}
							if (!Spawn_TimingParticles(Plan, Emitter, State,
								LoopIndex, Count, Step, Time, EmitterTime,
								strOutError))
							{
								return false;
							}
						}
						const auto& RateDistribution = Plan.Get_Distributions().at(
							Emitter.strSpawnRateDistributionId);
						const auto& RateScaleDistribution =
							Plan.Get_Distributions().at(
								Emitter.strSpawnRateScaleDistributionId);
						if (!RateDistribution.bCpuTimingExecutable ||
							!RateScaleDistribution.bCpuTimingExecutable)
						{
							return Reject(strOutError,
								"Reconstructed fixed-step spawn evaluator is not executable.");
						}
						std::array<double, 4u> RandomUnits{};
						RandomUnits[0u] = static_cast<double>(
							Next_Random(State.Public.iRandomState)) /
							static_cast<double>((std::numeric_limits<uint32_t>::max)());
						const double Rate = Evaluate_Distribution(
							RateDistribution, EmitterTime, RandomUnits)[0u];
						RandomUnits[0u] = static_cast<double>(
							Next_Random(State.Public.iRandomState)) /
							static_cast<double>((std::numeric_limits<uint32_t>::max)());
						const double RateScale = Evaluate_Distribution(
							RateScaleDistribution, EmitterTime, RandomUnits)[0u];
						if (!Is_Finite(Rate) || !Is_Finite(RateScale) ||
							Rate < 0.0 || RateScale < 0.0)
						{
							return Reject(strOutError,
								"Reconstructed fixed-step spawn rate is invalid.");
						}
						State.fSpawnAccumulator += Rate * RateScale * FixedStep;
						if (!Is_Finite(State.fSpawnAccumulator) ||
							State.fSpawnAccumulator > static_cast<double>(
								(std::numeric_limits<uint32_t>::max)()))
						{
							return Reject(strOutError,
								"Reconstructed fixed-step spawn accumulator overflowed.");
						}
						const uint32_t SpawnCount = static_cast<uint32_t>(
							std::floor(State.fSpawnAccumulator));
						State.fSpawnAccumulator -= static_cast<double>(SpawnCount);
						if (!Spawn_TimingParticles(Plan, Emitter, State,
							LoopIndex, SpawnCount, Step, Time, EmitterTime,
							strOutError))
						{
							return false;
						}
					}
					State.Public.iActiveCount =
						static_cast<uint32_t>(State.Particles.size());
					if (State.Public.iActiveCount >
						Emitter.iOperationalMaxParticles)
					{
						return Reject(strOutError,
							"Reconstructed fixed-step exceeded the operational cap.");
					}
				}
			}
		}
		OutSimulation = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Is_ValidRowIdentity(
		const EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY& Identity)
	{
		return !Identity.strId.empty() && Identity.strRowSha256.size() == 64u;
	}

	template <typename T>
	const T* Find_ProgramRow(
		const std::vector<T>& Rows,
		const EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY& Identity)
	{
		if (!Is_ValidRowIdentity(Identity))
			return nullptr;
		const auto It = std::find_if(Rows.begin(), Rows.end(),
			[&Identity](const T& Row)
			{
				return Row.Row.strId == Identity.strId &&
					Row.Row.strRowSha256 == Identity.strRowSha256;
			});
		return It == Rows.end() ? nullptr : &*It;
	}

	template <typename T>
	const T* Find_SidecarRow(
		const std::map<std::string, T, std::less<>>& Rows,
		const EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY& Identity)
	{
		if (!Is_ValidRowIdentity(Identity))
			return nullptr;
		const auto It = Rows.find(Identity.strId);
		return It == Rows.end() || It->second.strRowSha256 !=
			Identity.strRowSha256 ? nullptr : &It->second;
	}

	struct SELECTED_MODULE_HANDLER_ROUTE final
	{
		std::string_view strHandlerRegistryId;
		std::string_view strImplementationId;
		uint32_t iImplementationVersion = 0u;
		std::string_view strImplementationSha256;
		std::string_view strExpectedSourceClass;
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE
			eOpcode = EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
				MODULE_OPCODE::END;
		EFFECT_RUNTIME_HANDLER_KIND eExpectedHandlerKind =
			EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE;
	};

	constexpr std::array<SELECTED_MODULE_HANDLER_ROUTE, 15u>
		SELECTED_MODULE_HANDLER_ROUTES{{
		{ "handler-7197a80cf011dc858e402a52",
		  "source.module.exact.particlemodulerequired.v1", 1u,
		  "7549956b393dd6f0090bbab8d5ec9ec0905e74228243a97269d7f318dada3cc8",
		  "particlemodulerequired",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  REQUIRED_TIMING, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-13a7ed7163d5dfa1114b6b96",
		  "source.module.exact.particlemodulelifetime.v1", 1u,
		  "f23219bf0bade914e82f83b87fd90599ff20dec52db133779567ef9b4c60934e",
		  "particlemodulelifetime",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  LIFETIME_TIMING, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-9c42e464cd66e4181b042c28",
		  "source.module.exact.particlemodulesize.v1", 1u,
		  "48802cd578656989f265b3a9bb60ee29a601972bbe458f8f1f674708289f14da",
		  "particlemodulesize",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  SIZE, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-15647a506337532af1ba133f",
		  "source.module.exact.particlemodulevelocity.v1", 1u,
		  "367119b4649154273f580ab6e30cb62f0026e2044b8e669a2e7b701e14ac9acc",
		  "particlemodulevelocity",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  VELOCITY, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-338e3d2723b94eb720b5d716",
		  "source.module.exact.particlemodulecoloroverlife.v1", 1u,
		  "8806f7df8822465e84b118e75d45b271d8cb4e6f7dcf2d6c88b81f45e5a4e82c",
		  "particlemodulecoloroverlife",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  COLOR_OVER_LIFE, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-ef557e2b5df085f811e910d3",
		  "source.module.exact.particlemodulerotation.v1", 1u,
		  "77d0020b5a71b18474918077f33251cb44a956d5ee6f1adb03b3071df699c3f3",
		  "particlemodulerotation",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  ROTATION, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-a397c8c0eef3abd7aa07e4cb",
		  "source.module.exact.particlemoduleacceleration.v1", 1u,
		  "353c1bae53522ede173f93da8f2c936ddc2a298667b843380a673c7b4616e0d7",
		  "particlemoduleacceleration",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  ACCELERATION, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-c41530ddbfc5f10ed7f3c94d",
		  "source.module.exact.particlemodulerotationrate.v1", 1u,
		  "dda09b31d4e3b7f08d1f912eec57f9f359afc45eedca7b238a71480f6e5800d8",
		  "particlemodulerotationrate",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  ROTATION_RATE, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-8a15dbe6075b17c979aa066a",
		  "source.module.exact.particlemodulesizemultiplylife.v1", 1u,
		  "2892e1d0b0163a0ff74314e6d6c92ee0c0ce4002854efd0f3c96baa3f24f4689",
		  "particlemodulesizemultiplylife",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  SIZE_MULTIPLY_LIFE, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-a90067d9043e62f6e79a5cc4",
		  "source.module.exact.particlemodulelocationprimitivecylinder.v1", 1u,
		  "dbbdcb1c7b4b28726062cc3846d89b38073fd15dd2cf2cf21869c109c757d5d7",
		  "particlemodulelocationprimitivecylinder",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  CYLINDER_Z, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-3639750c28df3302e9effd03",
		  "source.module.exact.particlemodulelocation.v1", 1u,
		  "42e7911080f41f96edf084d1ce2adbc12ed7246a6c632b051b3a650204aca933",
		  "particlemodulelocation",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  LOCATION, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-9864f03184e2a68652252fec",
		  "source.reconstructed.ground.v1.implementation", 1u,
		  "ef488d559203aaef84091e7caf53e5ce9880de37da592f133991687ce2a2164f",
		  "efparticlemodulelocationonground",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  GROUND, EFFECT_RUNTIME_HANDLER_KIND::RECONSTRUCTED_MODULE },
		{ "handler-77746fd3dbe01b88dab89368",
		  "source.module.exact.particlemoduletypedatamesh.v1", 1u,
		  "61d6b904015b25c4cd76f036adbac9cfd5c27fcc4d2742aba83cd4674be8ec63",
		  "particlemoduletypedatamesh",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  TYPE_DATA_MESH, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-738a4e4b7b8c22539ffd2458",
		  "source.module.exact.particlemoduleparameterdynamic.v1", 1u,
		  "ec6543bd649774bddbd004cb91315d3657edf41d30c15e7a69c0dfd930f96ae3",
		  "particlemoduleparameterdynamic",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  DYNAMIC_PARAMETER, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
		{ "handler-fd2ce97f699c5a373f2529cb",
		  "source.module.exact.particlemodulespawn.v1", 1u,
		  "902e8b11ccd6ae323cc7c0fae93e8cc0daf25a143cf4c8aea3e3f6d5f0385392",
		  "particlemodulespawn",
		  EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE::
			  SPAWN_TIMING, EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE }
	}};

	const SELECTED_MODULE_HANDLER_ROUTE* Find_ModuleHandlerRoute(
		const std::string_view strHandlerRegistryId,
		const std::string_view strImplementationId,
		const uint32_t iImplementationVersion,
		const std::string_view strImplementationSha256)
	{
		const auto Route = std::find_if(SELECTED_MODULE_HANDLER_ROUTES.begin(),
			SELECTED_MODULE_HANDLER_ROUTES.end(), [&](const auto& Candidate)
			{
				return strHandlerRegistryId == Candidate.strHandlerRegistryId &&
					strImplementationId == Candidate.strImplementationId &&
					iImplementationVersion == Candidate.iImplementationVersion &&
					strImplementationSha256 == Candidate.strImplementationSha256;
			});
		return Route == SELECTED_MODULE_HANDLER_ROUTES.end() ? nullptr : &*Route;
	}

	std::optional<
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE>
		Resolve_ModuleOpcode(
			const EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY& Selection)
	{
		const auto* Route = Find_ModuleHandlerRoute(Selection.Handler.strId,
			Selection.strImplementationId, Selection.iImplementationVersion,
			Selection.strImplementationSha256);
		if (nullptr == Route ||
			Selection.strExactSourceClass != Route->strExpectedSourceClass)
		{
			return std::nullopt;
		}
		return Route->eOpcode;
	}

	bool_t Has_ModuleHandlerRoute(
		const EFFECT_RUNTIME_PROGRAM_HANDLER& Handler,
		const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			MODULE_OPCODE eExpectedOpcode)
	{
		const auto* Route = Find_ModuleHandlerRoute(Handler.Row.strId,
			Handler.strImplementationId, Handler.iImplementationVersion,
			Handler.strImplementationSha256);
		return nullptr != Route && Route->eOpcode == eExpectedOpcode &&
			Handler.eKind == Route->eExpectedHandlerKind &&
			Handler.strExactSourceClass == Route->strExpectedSourceClass;
	}

	bool_t Same4(
		const std::array<double, 4u>& A,
		const std::array<double, 4u>& B)
	{
		return A == B && Is_Finite(A) && Is_Finite(B);
	}

	bool_t Validate_MaterialConstants(
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE& Recipe,
		const EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_CONSTANTS& Constants)
	{
		if (Recipe.NumericBindingSamples.empty())
			return false;
		const auto& Sample = Recipe.NumericBindingSamples.front();
		return Sample.iOrder == 0u && Sample.fTime == 0.0 &&
			Sample.vUvScale == Constants.vUvScale &&
			Same4(Sample.vPanRotationAux, Constants.vPanRotationAux) &&
			Same4(Sample.vColor, Constants.vColor) &&
			Same4(Sample.vParams0, Constants.vParams0) &&
			Same4(Sample.vParams1, Constants.vParams1) &&
			Is_Finite(Constants.vUvScale);
	}

	bool_t Validate_TextureLane(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence,
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE& Recipe,
		const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER& Provider,
		const EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE& Selection,
		std::string& strOutError)
	{
		const auto* Input = Find_ProgramRow(
			Program.MaterialInputs, Selection.MaterialInput);
		const auto* Binding = Find_ProgramRow(
			Program.MaterialTextureBindings, Selection.MaterialTextureBinding);
		const auto* Policy = Find_ProgramRow(
			Program.MaterialPolicies, Selection.MaterialPolicy);
		const auto* SidecarBinding = Find_SidecarRow(
			Authority.TextureBindingsById, Selection.SidecarTextureBinding);
		const auto* SidecarResource = Find_SidecarRow(
			Authority.TextureResourcesById, Selection.SidecarTextureResource);
		if (nullptr == Input || nullptr == Binding || nullptr == Policy ||
			nullptr == SidecarBinding || nullptr == SidecarResource ||
			Selection.strShaderVariableName.empty() ||
			Selection.strRuntimeAssetId.empty() ||
			Selection.strRawSha256.size() != 64u)
		{
			return Reject(strOutError,
				"Selected evaluator texture lane identity is invalid.");
		}
		if (Input->strRecipeId != Recipe.Row.strId ||
			Binding->strRecipeId != Recipe.Row.strId ||
			Binding->strMaterialInputFieldId != Input->Row.strId ||
			Binding->strSamplerPolicyRowId != Policy->Row.strId ||
			!Binding->strRuntimeAssetId.has_value() ||
			*Binding->strRuntimeAssetId != Selection.strRuntimeAssetId ||
			Binding->eResolutionStatus !=
				EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET ||
			Policy->eDomain !=
				EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::SAMPLER_DESCRIPTOR ||
			!Policy->SamplerDescriptor.has_value() ||
			SidecarBinding->strCandidateBindingId != Binding->Row.strId ||
			SidecarBinding->strCandidateBindingRowSha256 !=
				Binding->Row.strRowSha256 ||
			SidecarBinding->strRecipeId != Recipe.Row.strId ||
			SidecarBinding->strMaterialInputFieldId != Input->Row.strId ||
			SidecarBinding->strSamplerPolicyRowId != Policy->Row.strId ||
			SidecarBinding->strSamplerPolicyRowSha256 !=
				Policy->Row.strRowSha256 ||
			SidecarBinding->strRuntimeAssetId != Selection.strRuntimeAssetId ||
			SidecarBinding->strResourceAuthorityId !=
				Selection.SidecarTextureResource.strId ||
			SidecarBinding->strResourceAuthorityRowSha256 !=
				Selection.SidecarTextureResource.strRowSha256 ||
			SidecarBinding->strActualDdsRawSha256 != Selection.strRawSha256 ||
			SidecarResource->strRuntimeAssetId != Selection.strRuntimeAssetId ||
			SidecarResource->strRawSha256 != Selection.strRawSha256 ||
			Provider.strMaterialInputFieldId != Input->Row.strId ||
			Provider.strMaterialInputRowSha256 != Input->Row.strRowSha256 ||
			Provider.strTextureBindingId != Binding->Row.strId ||
			Provider.strTextureBindingRowSha256 != Binding->Row.strRowSha256 ||
			Provider.strSamplerPolicyRowId != Policy->Row.strId ||
			Provider.strSamplerPolicyRowSha256 != Policy->Row.strRowSha256 ||
			Provider.strRuntimeAssetId != Selection.strRuntimeAssetId)
		{
			return Reject(strOutError,
				"Selected evaluator texture lane join is invalid.");
		}

		if (Selection.RendererTextureResource.has_value() !=
			Selection.SidecarRendererSlotDecision.has_value())
		{
			return Reject(strOutError,
				"Selected evaluator renderer texture presence is invalid.");
		}
		if (Selection.RendererTextureResource.has_value())
		{
			const auto* Renderer = Find_ProgramRow(
				Program.RendererTextureResources,
				*Selection.RendererTextureResource);
			const auto* Decision = Find_SidecarRow(
				Authority.RendererSlotBindingsById,
				*Selection.SidecarRendererSlotDecision);
			if (nullptr == Renderer || nullptr == Decision ||
				Renderer->strEmitterId != Emitter.Row.strId ||
				Renderer->strMaterialOccurrenceId != Occurrence.Row.strId ||
				Renderer->strAssetId != Selection.strRuntimeAssetId ||
				Decision->strRendererResourceRowSha256 !=
					Renderer->Row.strRowSha256 ||
				Decision->strMaterialOccurrenceId != Occurrence.Row.strId ||
				Decision->strMaterialOccurrenceRowSha256 !=
					Occurrence.Row.strRowSha256 ||
				Decision->strRecipeId != Recipe.Row.strId ||
				Decision->strRuntimeAssetId != Selection.strRuntimeAssetId ||
				Decision->strSelectedMaterialInputFieldId != Input->Row.strId ||
				Decision->strSelectedMaterialInputRowSha256 !=
					Input->Row.strRowSha256 ||
				Decision->strSelectedTextureBindingId != Binding->Row.strId ||
				Decision->strSelectedTextureBindingRowSha256 !=
					Binding->Row.strRowSha256 ||
				Decision->strSelectedSamplerPolicyRowId != Policy->Row.strId ||
				Decision->strSelectedSamplerPolicyRowSha256 !=
					Policy->Row.strRowSha256)
			{
				return Reject(strOutError,
					"Selected evaluator renderer texture join is invalid.");
			}
		}
		else
		{
			const bool_t HasUnexpectedRenderer = std::any_of(
				Program.RendererTextureResources.begin(),
				Program.RendererTextureResources.end(),
				[&](const EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE& Row)
				{
					return Row.strEmitterId == Emitter.Row.strId &&
						Row.strAssetId == Selection.strRuntimeAssetId;
				});
			if (HasUnexpectedRenderer)
				return Reject(strOutError,
					"Selected evaluator expected an absent renderer texture row.");
		}
		return true;
	}

	bool_t Validate_StateBinding(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE& Recipe,
		const EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING& Selection,
		const EFFECT_RECONSTRUCTED_RENDER_STATE_KIND eExpectedKind,
		std::string& strOutError)
	{
		const auto* Binding = Find_ProgramRow(
			Program.MaterialRenderBindings, Selection.ProgramBinding);
		const std::string_view ExpectedField =
			eExpectedKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND ?
				"blendmode" :
			eExpectedKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER ?
				"twosided" :
			eExpectedKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL ?
				"bdisabledepthtest" : "";
		if (nullptr == Binding || Binding->strRecipeId != Recipe.Row.strId ||
			ExpectedField.empty() || Binding->strFieldName != ExpectedField)
			return Reject(strOutError,
				"Selected evaluator render binding is invalid.");
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY* Policy = nullptr;
		if (Selection.ProgramPolicy.has_value())
		{
			Policy = Find_ProgramRow(
				Program.MaterialPolicies, *Selection.ProgramPolicy);
			if (nullptr == Policy ||
				Policy->eDomain !=
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE ||
				Policy->strRecipeId != Recipe.Row.strId ||
				Binding->strPolicyRowId != Policy->Row.strId ||
				Policy->strFieldId != Recipe.Row.strId + ":" +
					std::string(ExpectedField) ||
				Policy->strFieldKind != "RENDER_STATE_DEFAULT" ||
				!Policy->D3dDescriptorOracle.has_value() ||
				(eExpectedKind ==
						EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER &&
				 Policy->D3dDescriptorOracle->eKind !=
						EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER) ||
				(eExpectedKind ==
						EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL &&
				 Policy->D3dDescriptorOracle->eKind !=
						EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL) ||
				eExpectedKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND)
			{
				return Reject(strOutError,
					"Selected evaluator render policy is invalid.");
			}
		}
		else if (!Binding->strPolicyRowId.empty())
		{
			return Reject(strOutError,
				"Selected evaluator omitted a bound render policy.");
		}
		if (Selection.SidecarDecision.has_value())
		{
			const auto* Sidecar = Find_SidecarRow(
				Authority.RenderStateDescriptorsById,
				*Selection.SidecarDecision);
			if (nullptr == Sidecar ||
				Sidecar->eKind != eExpectedKind ||
				Sidecar->strRenderBindingId != Binding->Row.strId ||
				Sidecar->strRenderBindingRowSha256 !=
					Binding->Row.strRowSha256 ||
				Sidecar->strRecipeId != Recipe.Row.strId ||
				Sidecar->strRecipeRowSha256 != Recipe.Row.strRowSha256)
			{
				return Reject(strOutError,
					"Selected evaluator sidecar state join is invalid.");
			}
		}
		else
		{
			const bool_t HasUnexpectedDecision = std::any_of(
				Authority.RenderStateDescriptorsById.begin(),
				Authority.RenderStateDescriptorsById.end(),
				[Binding](const auto& Pair)
				{
					return Pair.second.strRenderBindingId == Binding->Row.strId;
				});
			if (HasUnexpectedDecision)
				return Reject(strOutError,
					"Selected evaluator expected an absent sidecar state row.");
		}
		return true;
	}

	bool_t Validate_MaterialBinding(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING& Selection,
		std::string& strOutError)
	{
		const auto* Occurrence = Find_ProgramRow(
			Program.MaterialOccurrences, Selection.Occurrence);
		const auto* Recipe = Find_ProgramRow(
			Program.MaterialRecipes, Selection.Recipe);
		const auto* Family = Find_ProgramRow(
			Program.MaterialFamilies, Selection.Family);
		const auto* TextureDecision = Find_SidecarRow(
			Authority.RecipeTextureBindingsById,
			Selection.RecipeTextureDecision);
		if (nullptr == Occurrence || nullptr == Recipe || nullptr == Family ||
			nullptr == TextureDecision ||
			Occurrence->strEmitterId != Emitter.Row.strId ||
			Occurrence->strRecipeId != Recipe->Row.strId ||
			Occurrence->strFamilyId != Family->Row.strId ||
			Recipe->strFamilyId != Family->Row.strId ||
			Emitter.strMaterialOccurrenceId != Occurrence->Row.strId ||
			Family->strEvaluatorId != Selection.strEvaluatorId ||
			Family->iEvaluatorVersion != Selection.iEvaluatorVersion ||
			Family->strEvaluatorSha256 != Selection.strEvaluatorSha256 ||
			Family->iFeatureMask != Selection.iFeatureMask ||
			TextureDecision->strRecipeId != Recipe->Row.strId ||
			TextureDecision->strRecipeRowSha256 != Recipe->Row.strRowSha256 ||
			TextureDecision->strFamilyId != Family->Row.strId ||
			TextureDecision->strFamilyRowSha256 != Family->Row.strRowSha256 ||
			TextureDecision->iFeatureMask != Family->iFeatureMask)
		{
			return Reject(strOutError,
				"Selected evaluator material authority is invalid.");
		}
		if (Selection.TextureLanes[0u].strShaderVariableName !=
				"g_SourceTexture0" ||
			Selection.TextureLanes[1u].strShaderVariableName !=
				"g_SourceTexture1")
		{
			return Reject(strOutError,
				"Selected evaluator shader texture lane names are invalid.");
		}
		if (!Validate_TextureLane(Program, Authority, Emitter, *Occurrence,
			*Recipe, TextureDecision->Texture0Provider,
			Selection.TextureLanes[0u], strOutError) ||
			!Validate_TextureLane(Program, Authority, Emitter, *Occurrence,
				*Recipe, TextureDecision->Texture1Provider,
				Selection.TextureLanes[1u], strOutError) ||
			!Validate_StateBinding(Program, Authority, *Recipe,
				Selection.BlendState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND, strOutError) ||
			!Validate_StateBinding(Program, Authority, *Recipe,
				Selection.RasterizerState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER,
				strOutError) ||
			!Validate_StateBinding(Program, Authority, *Recipe,
				Selection.DepthStencilState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL,
				strOutError))
		{
			return false;
		}
		const auto& Shader = Selection.Shader;
		const bool_t IsMeshShader = Emitter.eRenderer ==
			EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE &&
			Shader.strShaderAssetId == "Shader_VtxEffectMeshPreview.hlsl" &&
			Shader.iPassIndex == 0u &&
			Shader.strPassName == "OpaqueBackDepthWrite";
		const bool_t IsSpriteShader = Emitter.eRenderer ==
			EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE &&
			Shader.strShaderAssetId == "Shader_VtxEffectParticle.hlsl" &&
			Shader.iPassIndex == 1u &&
			Shader.strPassName == "AlphaTwoSidedDepthRead";
		if (!Validate_MaterialConstants(*Recipe, Selection.Constants) ||
			(!IsMeshShader && !IsSpriteShader) ||
			Shader.strTechniqueName != "DefaultTechnique" ||
			Shader.strEvaluatorEnabledVariable !=
				"g_ReconstructedMaterialEvaluatorEnabled" ||
			Shader.strFeatureMaskVariable !=
				"g_ReconstructedMaterialFeatureMask" ||
			Shader.strUvScaleVariable != "g_ReconstructedUVScale" ||
			Shader.strPanRotationAuxVariable !=
				"g_ReconstructedPanRotationAux" ||
			Shader.strColorVariable != "g_ReconstructedColor" ||
			Shader.strParams0Variable != "g_ReconstructedParams0" ||
			Shader.strParams1Variable != "g_ReconstructedParams1")
		{
			return Reject(strOutError,
				"Selected evaluator material constants or shader binding is invalid.");
		}
		return true;
	}

	bool_t Validate_GeometryBinding(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING& Selection,
		std::string& strOutError)
	{
		const auto* Use = Find_ProgramRow(Program.GeometryUses,
			Selection.GeometryUse);
		const auto* Carrier = Find_ProgramRow(Program.GeometryCarriers,
			Selection.GeometryCarrier);
		if (nullptr == Use || nullptr == Carrier ||
			!Emitter.strGeometryUseId.has_value() ||
			*Emitter.strGeometryUseId != Use->Row.strId ||
			Use->strEmitterId != Emitter.Row.strId ||
			Use->strCarrierId != Carrier->Row.strId ||
			Use->strAssetId != Selection.strRuntimeAssetId ||
			Carrier->strAssetId != Selection.strRuntimeAssetId ||
			Carrier->iCandidateResourceByteSize !=
				Selection.iCandidateResourceByteSize ||
			Carrier->strCandidateResourceSha256 !=
				Selection.strCandidateResourceSha256 ||
			Carrier->strPayloadSha256 != Selection.strPayloadSha256 ||
			Carrier->strMetadataIdentitySha256 !=
				Selection.strMetadataIdentitySha256 ||
			Carrier->strCacheIdentitySha256 != Selection.strCacheIdentitySha256 ||
			Carrier->strExpectedTupleSha256 != Selection.strExpectedTupleSha256 ||
			Carrier->strApprovalGeometryRowSha256 !=
				Selection.strApprovalGeometryRowSha256 ||
			Carrier->strPreparedCacheIdentitySha256 !=
				Selection.strPreparedCacheIdentitySha256 ||
			Carrier->fGeometryPreScale != Selection.fGeometryPreScale ||
			Use->strPreScaleApplication !=
				"VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED" ||
			Use->bPreScaleConsumed || Carrier->bPreScaleConsumed ||
			!Is_Finite(Selection.fGeometryPreScale) ||
			Selection.fGeometryPreScale <= 0.0 ||
			Carrier->Submeshes.size() != Selection.iSubmeshCount)
		{
			return Reject(strOutError,
				"Selected evaluator geometry binding is invalid.");
		}
		uint64_t VertexCount = 0u;
		uint64_t IndexCount = 0u;
		for (const auto& Submesh : Carrier->Submeshes)
		{
			VertexCount += Submesh.iVertexCount;
			IndexCount += Submesh.iIndexCount;
		}
		if (VertexCount != Selection.iVertexCount ||
			IndexCount != Selection.iIndexCount)
		{
			return Reject(strOutError,
				"Selected evaluator geometry counts are invalid.");
		}
		return true;
	}

	bool_t Validate_SpriteSink(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK& Selection,
		std::string& strOutError)
	{
		const auto* Module = Find_ProgramRow(
			Program.Modules, Selection.RequiredModule);
		const auto* AlignmentProperty = Find_ProgramRow(
			Program.Properties, Selection.ScreenAlignmentProperty);
		const auto* AlignmentLiteral = Find_ProgramRow(
			Program.Literals, Selection.ScreenAlignmentLiteral);
		const auto* FlipProperty = Find_ProgramRow(
			Program.Properties, Selection.AllowImageFlippingProperty);
		const auto* FlipLiteral = Find_ProgramRow(
			Program.Literals, Selection.AllowImageFlippingLiteral);
		const auto* OffsetProperty = Find_ProgramRow(
			Program.Properties, Selection.OffsetCenterEnabledProperty);
		const auto* OffsetLiteral = Find_ProgramRow(
			Program.Literals, Selection.OffsetCenterEnabledLiteral);
		const auto* OffsetXProperty = Find_ProgramRow(
			Program.Properties, Selection.OffsetCenterXProperty);
		const auto* OffsetXLiteral = Find_ProgramRow(
			Program.Literals, Selection.OffsetCenterXLiteral);
		const auto* OffsetYProperty = Find_ProgramRow(
			Program.Properties, Selection.OffsetCenterYProperty);
		const auto* OffsetYLiteral = Find_ProgramRow(
			Program.Literals, Selection.OffsetCenterYLiteral);
		if (nullptr == Module || nullptr == AlignmentProperty ||
			nullptr == AlignmentLiteral || nullptr == FlipProperty ||
			nullptr == FlipLiteral || nullptr == OffsetProperty ||
			nullptr == OffsetLiteral || nullptr == OffsetXProperty ||
			nullptr == OffsetXLiteral || nullptr == OffsetYProperty ||
			nullptr == OffsetYLiteral ||
			Module->strEmitterId != Emitter.Row.strId ||
			Module->Row.strId != Emitter.Timing.strRequiredModuleId)
		{
			return Reject(strOutError,
				"Selected evaluator Sprite sink rows are invalid.");
		}
		const auto SameModule = [Module](const auto* Row)
		{
			return Row->strModuleId == Module->Row.strId;
		};
		if (!SameModule(AlignmentProperty) || !SameModule(AlignmentLiteral) ||
			!SameModule(FlipProperty) || !SameModule(FlipLiteral) ||
			!SameModule(OffsetProperty) || !SameModule(OffsetLiteral) ||
			!SameModule(OffsetXProperty) || !SameModule(OffsetXLiteral) ||
			!SameModule(OffsetYProperty) || !SameModule(OffsetYLiteral) ||
			AlignmentProperty->strPropertyPath != "screenalignment" ||
			AlignmentLiteral->strPropertyPath != "screenalignment" ||
			AlignmentLiteral->eVariant !=
				EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING ||
			AlignmentLiteral->strEnumValue != "psa_velocity" ||
			Selection.eAlignment !=
				EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT::VELOCITY ||
			Selection.eOrientation !=
				EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION::
					CAMERA_BILLBOARD_WITH_VELOCITY_ALIGNMENT ||
			FlipProperty->strPropertyPath != "ballowimageflipping" ||
			FlipLiteral->strPropertyPath != "ballowimageflipping" ||
			FlipLiteral->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN ||
			!FlipLiteral->bValue.has_value() ||
			*FlipLiteral->bValue != Selection.bAllowImageFlipping ||
			OffsetProperty->strPropertyPath != "boffsetcenter" ||
			OffsetLiteral->strPropertyPath != "boffsetcenter" ||
			OffsetLiteral->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN ||
			!OffsetLiteral->bValue.has_value() ||
			*OffsetLiteral->bValue != Selection.bOffsetCenter ||
			OffsetXProperty->strPropertyPath != "offsetcenterx" ||
			OffsetXLiteral->strPropertyPath != "offsetcenterx" ||
			OffsetXLiteral->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
			!OffsetXLiteral->fValue.has_value() ||
			*OffsetXLiteral->fValue != Selection.vPivotCenter[0u] ||
			OffsetYProperty->strPropertyPath != "offsetcentery" ||
			OffsetYLiteral->strPropertyPath != "offsetcentery" ||
			OffsetYLiteral->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
			!OffsetYLiteral->fValue.has_value() ||
			*OffsetYLiteral->fValue != Selection.vPivotCenter[1u] ||
			!Is_Finite(Selection.vPivotCenter) || !Selection.bBillboard ||
			Selection.fBillboardRollDegrees != 0.0 ||
			!Is_Finite(Selection.fBillboardRollDegrees))
		{
			return Reject(strOutError,
				"Selected evaluator Sprite sink values are invalid.");
		}
		return true;
	}

	bool_t Validate_CylinderPolicy(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_SELECTED_CYLINDER_POLICY& Selection,
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION& Staged,
		std::string& strOutError)
	{
		const auto* Module = Find_ProgramRow(Program.Modules, Selection.Module);
		const auto* SurfaceOnly = Find_ProgramRow(
			Program.Literals, Selection.SurfaceOnlyLiteral);
		const auto* Velocity = Find_ProgramRow(
			Program.Literals, Selection.VelocityLiteral);
		const auto SelectedCylinder = std::find_if(
			Staged.Modules.begin(), Staged.Modules.end(), [](const auto& Operation)
			{
				return Operation.eOpcode ==
					EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
						MODULE_OPCODE::CYLINDER_Z;
			});
		if (nullptr == Module || nullptr == SurfaceOnly || nullptr == Velocity ||
			SelectedCylinder == Staged.Modules.end() ||
			SelectedCylinder->pProgramModule != Module ||
			nullptr == SelectedCylinder->pHandler ||
			!Has_ModuleHandlerRoute(*SelectedCylinder->pHandler,
				EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
					MODULE_OPCODE::CYLINDER_Z) ||
			Module->strExactSourceClass !=
				"particlemodulelocationprimitivecylinder" ||
			SurfaceOnly->strModuleId != Module->Row.strId ||
			Velocity->strModuleId != Module->Row.strId ||
			SurfaceOnly->strPropertyPath != "surfaceonly" ||
			Velocity->strPropertyPath != "velocity" ||
			SurfaceOnly->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN ||
			Velocity->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN ||
			!SurfaceOnly->bValue.has_value() || !Velocity->bValue.has_value() ||
			Selection.strAbsentHeightAxisDefault != "Z")
		{
			return Reject(strOutError,
				"Selected evaluator cylinder policy is invalid.");
		}
		const bool_t HasHeightAxisProperty = std::any_of(
			Program.Properties.begin(), Program.Properties.end(),
			[Module](const EFFECT_RUNTIME_PROGRAM_PROPERTY& Row)
			{
				return Row.strModuleId == Module->Row.strId &&
					Row.strPropertyPath == "heightaxis";
			});
		const bool_t HasHeightAxisLiteral = std::any_of(
			Program.Literals.begin(), Program.Literals.end(),
			[Module](const EFFECT_RUNTIME_PROGRAM_LITERAL& Row)
			{
				return Row.strModuleId == Module->Row.strId &&
					Row.strPropertyPath == "heightaxis";
			});
		const bool_t HasHeightAxisDefault = std::any_of(
			Program.ImplicitDefaults.begin(), Program.ImplicitDefaults.end(),
			[Module](const EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT& Row)
			{
				return Row.strModuleId == Module->Row.strId &&
					Row.strFieldPath == "heightaxis";
			});
		if (HasHeightAxisProperty || HasHeightAxisLiteral ||
			HasHeightAxisDefault)
		{
			return Reject(strOutError,
				"Selected evaluator cylinder expected an absent Z-axis row.");
		}
		Staged.bCylinderSurfaceOnly = *SurfaceOnly->bValue;
		Staged.bCylinderVelocity = *Velocity->bValue;
		if (!Staged.bCylinderSurfaceOnly || !Staged.bCylinderVelocity)
			return Reject(strOutError,
				"Selected evaluator cylinder flags are outside the closed policy.");
		return true;
	}

	bool_t Bind_ModuleDistributions(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			MODULE_OPERATION& Operation,
		std::string& strOutError);

	bool_t Validate_EmitterSelection(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection,
		const uint32_t iSelectionIndex,
		const bool_t bValidateExpectedResult,
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION& OutOperation,
		std::string& strOutError)
	{
		const auto PlanEmitterIt = Plan.Get_Emitters().find(
			Selection.Emitter.strId);
		const auto PlanScheduleIt = Plan.Get_Schedules().find(
			Selection.Schedule.strId);
		const auto* ProgramEmitter = Find_ProgramRow(
			Program.Emitters, Selection.Emitter);
		const auto* ProgramSchedule = Find_ProgramRow(
			Program.ActionSchedules, Selection.Schedule);
		if (PlanEmitterIt == Plan.Get_Emitters().end() ||
			PlanScheduleIt == Plan.Get_Schedules().end() ||
			nullptr == ProgramEmitter || nullptr == ProgramSchedule)
		{
			return Reject(strOutError,
				"Selected evaluator emitter or schedule row is invalid.");
		}
		const auto& PlanEmitter = PlanEmitterIt->second;
		const auto& PlanSchedule = PlanScheduleIt->second;
		const bool_t IsMesh = Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH;
		const bool_t IsSprite = Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE;
		if ((!IsMesh && !IsSprite) ||
			(IsMesh && Selection.eRenderer !=
				EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE) ||
			(IsSprite && Selection.eRenderer !=
				EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE) ||
			ProgramEmitter->eRenderer != Selection.eRenderer ||
			PlanEmitter.eRenderer != Selection.eRenderer ||
			ProgramEmitter->Row.iOrder != Selection.iEmitterOrder ||
			PlanEmitter.iOrder != Selection.iEmitterOrder ||
			ProgramEmitter->strScheduleId != Selection.Schedule.strId ||
			PlanEmitter.strScheduleId != Selection.Schedule.strId ||
			ProgramSchedule->Row.strId != PlanSchedule.strScheduleId ||
			ProgramEmitter->bLocalSpace != Selection.bLocalSpace ||
			ProgramEmitter->strSizeUnitPolicy != Selection.strSizeUnitPolicy ||
			(bValidateExpectedResult &&
				(Selection.iExpectedVisualRandomDrawCount == 0u ||
				 Selection.iExpectedFinalRandomState == 0u ||
				 Selection.iExpectedOccurrenceRandomValue == 0u ||
				 Selection.iExpectedLifetimeRandomValue == 0u ||
				 !Is_Finite(Selection.fExpectedLifetimeSeconds) ||
				 Selection.fExpectedLifetimeSeconds <= 0.0)) ||
			!ProgramEmitter->bVisible || !PlanEmitter.bVisible ||
			Selection.Handlers.size() != ProgramEmitter->ModuleIds.size() ||
			Selection.Handlers.size() != PlanEmitter.ModuleIds.size() ||
			Selection.Handlers.empty())
		{
			return Reject(strOutError,
				"Selected evaluator emitter shape is invalid.");
		}

		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION Staged;
		Staged.iSelectionIndex = iSelectionIndex;
		Staged.pPlanSchedule = &PlanSchedule;
		Staged.pPlanEmitter = &PlanEmitter;
		Staged.pProgramSchedule = ProgramSchedule;
		Staged.pProgramEmitter = ProgramEmitter;
		for (size_t Index = 0u; Index < Selection.Handlers.size(); ++Index)
		{
			const auto& HandlerSelection = Selection.Handlers[Index];
			const auto* Module = Find_ProgramRow(
				Program.Modules, HandlerSelection.Module);
			const auto* Handler = Find_ProgramRow(
				Program.Handlers, HandlerSelection.Handler);
			const auto PlanModuleIt = Plan.Get_Modules().find(
				HandlerSelection.Module.strId);
			if (nullptr == Module || nullptr == Handler ||
				PlanModuleIt == Plan.Get_Modules().end())
			{
				return Reject(strOutError,
					"Selected evaluator handler authority is invalid.");
			}
			const auto Opcode = Resolve_ModuleOpcode(HandlerSelection);
			if (!Opcode.has_value())
			{
				return Reject(strOutError,
					"Selected evaluator handler dispatch is unsupported.");
			}
			if (
				!Has_ModuleHandlerRoute(*Handler, *Opcode) ||
				ProgramEmitter->ModuleIds[Index] != Module->Row.strId ||
				PlanEmitter.ModuleIds[Index] != Module->Row.strId ||
				Module->Row.iOrder != Index ||
				Module->strEmitterId != ProgramEmitter->Row.strId ||
				Module->strExactSourceClass !=
					HandlerSelection.strExactSourceClass ||
				Handler->Row.strId != Module->strHandlerRegistryId ||
				Handler->strExactSourceClass !=
					HandlerSelection.strExactSourceClass ||
				Handler->strImplementationId !=
					HandlerSelection.strImplementationId ||
				Handler->iImplementationVersion !=
					HandlerSelection.iImplementationVersion ||
				Handler->strImplementationSha256 !=
					HandlerSelection.strImplementationSha256 ||
				PlanModuleIt->second.strHandlerRegistryId != Handler->Row.strId ||
				PlanModuleIt->second.strHandlerImplementationId !=
					Handler->strImplementationId ||
				PlanModuleIt->second.iHandlerImplementationVersion !=
					Handler->iImplementationVersion ||
				PlanModuleIt->second.strHandlerImplementationSha256 !=
					Handler->strImplementationSha256 ||
				PlanModuleIt->second.DistributionIds != Module->DistributionIds ||
				HandlerSelection.ImplicitDefaults.size() !=
					Module->ImplicitDefaultIds.size())
			{
				return Reject(strOutError,
					"Selected evaluator handler authority is invalid.");
			}
			for (size_t DefaultIndex = 0u;
				DefaultIndex < HandlerSelection.ImplicitDefaults.size();
				++DefaultIndex)
			{
				const auto* Default = Find_ProgramRow(Program.ImplicitDefaults,
					HandlerSelection.ImplicitDefaults[DefaultIndex]);
				if (nullptr == Default ||
					Default->Row.strId != Module->ImplicitDefaultIds[DefaultIndex] ||
					Default->strModuleId != Module->Row.strId)
				{
					return Reject(strOutError,
						"Selected evaluator implicit default is invalid.");
				}
			}
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
				MODULE_OPERATION ModuleOperation;
			ModuleOperation.pPlanModule = &PlanModuleIt->second;
			ModuleOperation.pProgramModule = Module;
			ModuleOperation.pHandler = Handler;
			ModuleOperation.eOpcode = *Opcode;
			if (!Bind_ModuleDistributions(Plan, ModuleOperation, strOutError))
				return false;
			Staged.Modules.push_back(std::move(ModuleOperation));
		}

		const size_t CylinderCount = static_cast<size_t>(std::count_if(
			Staged.Modules.begin(), Staged.Modules.end(), [](const auto& Module)
			{
				return Module.eOpcode ==
					EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
						MODULE_OPCODE::CYLINDER_Z;
			}));
		if (CylinderCount != (Selection.Cylinder.has_value() ? 1u : 0u) ||
			(Selection.Cylinder.has_value() &&
			 !Validate_CylinderPolicy(Program, *Selection.Cylinder, Staged,
				 strOutError)))
		{
			if (strOutError.empty())
				Reject(strOutError,
					"Selected evaluator cylinder selection is inconsistent.");
			return false;
		}
		if ((IsMesh && (!Selection.Geometry.has_value() ||
				 Selection.SpriteSink.has_value())) ||
			(IsSprite && (Selection.Geometry.has_value() ||
				 !Selection.SpriteSink.has_value())))
		{
			return Reject(strOutError,
				"Selected evaluator renderer payload shape is invalid.");
		}
		if (IsMesh && !Validate_GeometryBinding(
			Program, *ProgramEmitter, *Selection.Geometry, strOutError))
		{
			return false;
		}
		if (IsSprite && !Validate_SpriteSink(
			Program, *ProgramEmitter, *Selection.SpriteSink, strOutError))
		{
			return false;
		}
		if (!Validate_MaterialBinding(Program, Authority, *ProgramEmitter,
			Selection.Material, strOutError))
		{
			return false;
		}
		OutOperation = std::move(Staged);
		return true;
	}

	const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION*
		Find_ModuleDistribution(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_EXECUTION_MODULE& Module,
		const std::string_view strPropertyPath)
	{
		const std::string Suffix = "::distribution:" +
			std::string(strPropertyPath);
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION* Found = nullptr;
		for (const std::string& DistributionId : Module.DistributionIds)
		{
			if (!DistributionId.ends_with(Suffix))
				continue;
			const auto It = Plan.Get_Distributions().find(DistributionId);
			if (It == Plan.Get_Distributions().end() || nullptr != Found)
				return nullptr;
			Found = &It->second;
		}
		return Found;
	}

	bool_t Bind_ModuleDistributions(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			MODULE_OPERATION& Operation,
		std::string& strOutError)
	{
		using OPCODE =
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE;
		if (nullptr == Operation.pPlanModule)
			return Reject(strOutError,
				"Selected evaluator distribution module is invalid.");
		const auto Bind = [&](const std::initializer_list<std::string_view> Paths)
		{
			if (Paths.size() > Operation.Distributions.size())
				return false;
			uint32_t Index = 0u;
			for (const std::string_view Path : Paths)
			{
				const auto* Distribution = Find_ModuleDistribution(
					Plan, *Operation.pPlanModule, Path);
				if (nullptr == Distribution)
					return false;
				Operation.Distributions[Index++] = Distribution;
			}
			Operation.iDistributionCount = Index;
			return true;
		};
		bool_t Bound = false;
		switch (Operation.eOpcode)
		{
		case OPCODE::REQUIRED_TIMING: Bound = Bind({ "spawnrate" }); break;
		case OPCODE::LIFETIME_TIMING: Bound = Bind({ "lifetime" }); break;
		case OPCODE::SIZE: Bound = Bind({ "startsize" }); break;
		case OPCODE::VELOCITY:
			Bound = Bind({ "startvelocity", "startvelocityradial" });
			break;
		case OPCODE::COLOR_OVER_LIFE:
			Bound = Bind({ "coloroverlife", "alphaoverlife" });
			break;
		case OPCODE::ROTATION: Bound = Bind({ "startrotation" }); break;
		case OPCODE::ACCELERATION: Bound = Bind({ "acceleration" }); break;
		case OPCODE::ROTATION_RATE:
			Bound = Bind({ "startrotationrate" });
			break;
		case OPCODE::SIZE_MULTIPLY_LIFE:
			Bound = Bind({ "lifemultiplier" });
			break;
		case OPCODE::CYLINDER_Z:
			Bound = Bind({ "startradius", "startheight", "velocityscale",
				"startlocation" });
			break;
		case OPCODE::LOCATION: Bound = Bind({ "startlocation" }); break;
		case OPCODE::GROUND:
			Bound = Bind({ "adjustlocation", "skiplocation" });
			break;
		case OPCODE::TYPE_DATA_MESH:
			Bound = Operation.pPlanModule->DistributionIds.empty();
			break;
		case OPCODE::DYNAMIC_PARAMETER:
			if (Operation.pPlanModule->DistributionIds.size() == 4u)
			{
				Bound = true;
				for (uint32_t Index = 0u; Index < 4u; ++Index)
				{
					const std::string ExpectedSuffix = "::distribution:dynamicparams[" +
						std::to_string(Index) + "].paramvalue";
					const std::string& DistributionId =
						Operation.pPlanModule->DistributionIds[Index];
					const auto It = Plan.Get_Distributions().find(DistributionId);
					if (!DistributionId.ends_with(ExpectedSuffix) ||
						It == Plan.Get_Distributions().end())
					{
						Bound = false;
						break;
					}
					Operation.Distributions[Index] = &It->second;
				}
				Operation.iDistributionCount = Bound ? 4u : 0u;
			}
			break;
		case OPCODE::SPAWN_TIMING:
			Bound = Bind({ "rate", "ratescale" });
			break;
		default: break;
		}
		if (!Bound || Operation.iDistributionCount !=
			Operation.pPlanModule->DistributionIds.size())
		{
			return Reject(strOutError,
				"Selected evaluator distribution binding is not closed.");
		}
		return true;
	}

	bool_t Evaluate_VisualDistribution(
		const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Distribution,
		const double fTime,
		uint32_t& iInOutRandomState,
		uint32_t& iInOutDrawCount,
		std::array<double, 4u>& OutValue,
		std::string& strOutError)
	{
		std::array<double, 4u> RandomUnits{};
		if (Distribution.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ||
			Distribution.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER)
		{
			OutValue = Evaluate_Distribution(Distribution, fTime, RandomUnits);
		}
		else
		{
			if ((Distribution.eVariant !=
					 EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
				 Distribution.eVariant !=
					 EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE) ||
				!Distribution.iOperation.has_value() ||
				!Distribution.iRandomLockAxes.has_value() ||
				*Distribution.iRandomLockAxes != 0u ||
				Distribution.iComponentCount == 0u ||
				Distribution.iComponentCount > 4u)
			{
				return Reject(strOutError,
					"Selected evaluator distribution contract is unsupported.");
			}
			if (*Distribution.iOperation == 2u)
			{
				for (uint32_t Index = 0u;
					Index < Distribution.iComponentCount; ++Index)
				{
					RandomUnits[Index] = static_cast<double>(
						Next_Random(iInOutRandomState)) /
						static_cast<double>(
							(std::numeric_limits<uint32_t>::max)());
					++iInOutDrawCount;
				}
			}
			else if (*Distribution.iOperation == 3u)
			{
				RandomUnits[0u] = static_cast<double>(
					Next_Random(iInOutRandomState)) /
					static_cast<double>((std::numeric_limits<uint32_t>::max)());
				++iInOutDrawCount;
			}
			else if (*Distribution.iOperation != 1u)
			{
				return Reject(strOutError,
					"Selected evaluator distribution operation is unsupported.");
			}
			OutValue = Evaluate_Distribution(Distribution, fTime, RandomUnits);
		}
		if (!Is_Finite(OutValue))
			return Reject(strOutError,
				"Selected evaluator distribution produced a nonfinite value.");
		return true;
	}

	bool_t Evaluate_PreparedDistribution(
		const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			MODULE_OPERATION& Operation,
		const uint32_t iDistributionIndex,
		const double fTime,
		uint32_t& iInOutRandomState,
		uint32_t& iInOutDrawCount,
		std::array<double, 4u>& OutValue,
		std::string& strOutError)
	{
		if (iDistributionIndex >= Operation.iDistributionCount ||
			nullptr == Operation.Distributions[iDistributionIndex])
			return Reject(strOutError,
				"Selected evaluator prepared distribution is invalid.");
		return Evaluate_VisualDistribution(
			*Operation.Distributions[iDistributionIndex], fTime,
			iInOutRandomState, iInOutDrawCount, OutValue, strOutError);
	}

	struct SELECTED_VISUAL_RESULT final
	{
		EFFECT_RECONSTRUCTED_SELECTED_PARTICLE_VALUES Values;
		uint32_t iFinalRandomState = 0u;
		uint32_t iRandomDrawCount = 0u;
	};

	std::array<double, 3u> Source_ToClientVector(
		const std::array<double, 3u>& Source,
		const double Scale)
	{
		std::array<double, 3u> Result{ Source[0u] * Scale,
			Source[2u] * Scale, -Source[1u] * Scale };
		for (double& Value : Result)
		{
			if (Value == 0.0)
				Value = 0.0;
		}
		return Result;
	}

	bool_t Evaluate_SelectedVisual(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION& Operation,
		const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection,
		const EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET& Timing,
		const double fSampleTimeSeconds,
		const bool_t bValidateExpectedResult,
		SELECTED_VISUAL_RESULT& OutResult,
		std::string& strOutError)
	{
		using OPCODE =
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::MODULE_OPCODE;
		if (nullptr == Operation.pPlanSchedule ||
			nullptr == Operation.pPlanEmitter || Timing.fAgeSeconds != 0.0 ||
			(bValidateExpectedResult &&
			 (Timing.iOccurrenceRandomValue !=
				 Selection.iExpectedOccurrenceRandomValue ||
			  Timing.iLifetimeRandomValue !=
				 Selection.iExpectedLifetimeRandomValue ||
			  Timing.fLifetimeSeconds != Selection.fExpectedLifetimeSeconds)))
		{
			return Reject(strOutError,
				"Selected evaluator timing packet identity is invalid.");
		}
		const double EmitterElapsed = fSampleTimeSeconds -
			Operation.pPlanSchedule->fGlobalTimeSeconds -
			Operation.pPlanEmitter->fEmitterDelaySeconds;
		const double EmitterTime = EmitterElapsed -
			static_cast<double>(Timing.iLoopIndex) *
				Operation.pPlanEmitter->fEmitterDurationSeconds;
		const double RelativeAge = Timing.fLifetimeSeconds <= 0.0 ?
			std::numeric_limits<double>::quiet_NaN() :
			Timing.fAgeSeconds / Timing.fLifetimeSeconds;
		if (!Is_Finite(EmitterTime) || EmitterTime < 0.0 ||
			!Is_Finite(RelativeAge) || RelativeAge != 0.0)
		{
			return Reject(strOutError,
				"Selected evaluator occurrence time is invalid.");
		}

		std::array<double, 3u> SourceScale{ 1.0, 1.0, 1.0 };
		std::array<double, 3u> SourcePosition{};
		std::array<double, 3u> SourceVelocity{};
		std::array<double, 3u> SourceAcceleration{};
		std::array<double, 4u> Color{};
		std::array<double, 4u> Dynamic{};
		double RotationDegrees = 0.0;
		double RotationRateDegrees = 0.0;
		uint32_t RandomState = Timing.iOccurrenceRandomValue;
		uint32_t DrawCount = 0u;
		for (const auto& ModuleOperation : Operation.Modules)
		{
			if (nullptr == ModuleOperation.pPlanModule ||
				ModuleOperation.eOpcode == OPCODE::END)
			{
				return Reject(strOutError,
					"Selected evaluator prepared opcode is invalid.");
			}
			const auto& Module = *ModuleOperation.pPlanModule;
			std::array<double, 4u> A{};
			std::array<double, 4u> B{};
			switch (ModuleOperation.eOpcode)
			{
			case OPCODE::REQUIRED_TIMING:
			case OPCODE::LIFETIME_TIMING:
			case OPCODE::TYPE_DATA_MESH:
			case OPCODE::SPAWN_TIMING:
				break;
			case OPCODE::SIZE:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				std::copy_n(A.begin(), 3u, SourceScale.begin());
				break;
			case OPCODE::VELOCITY:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError) ||
					!Evaluate_PreparedDistribution(ModuleOperation, 1u, EmitterTime,
						RandomState,
						DrawCount, B, strOutError))
					return false;
				if (B[0u] != 0.0)
					return Reject(strOutError,
						"Selected evaluator radial velocity is unsupported.");
				for (size_t Index = 0u; Index < SourceVelocity.size(); ++Index)
					SourceVelocity[Index] += A[Index];
				break;
			case OPCODE::COLOR_OVER_LIFE:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					RelativeAge, RandomState, DrawCount, A, strOutError) ||
					!Evaluate_PreparedDistribution(ModuleOperation, 1u, RelativeAge,
						RandomState, DrawCount, B, strOutError))
					return false;
				Color = { A[0u], A[1u], A[2u], B[0u] };
				if (const auto* Alpha = ModuleOperation.Distributions[1u];
					nullptr != Alpha && Alpha->LookupTable.empty() &&
					Alpha->CurveKeys.empty())
				{
					const bool_t ExactImplicitIdentity =
						Alpha->iComponentCount == 1u &&
						Alpha->iOperation == 1u &&
						Alpha->DefaultMinimum ==
							std::vector<double>(4u, 0.0) &&
						Alpha->DefaultMaximum ==
							std::vector<double>(4u, 0.0);
					if (!ExactImplicitIdentity)
						return Reject(strOutError,
							"Selected evaluator implicit alpha identity is invalid.");
					Color[3u] = 1.0;
				}
				break;
			case OPCODE::ROTATION:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				RotationDegrees = A[0u] * 360.0;
				break;
			case OPCODE::ACCELERATION:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				std::copy_n(A.begin(), 3u, SourceAcceleration.begin());
				break;
			case OPCODE::ROTATION_RATE:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount,
					A, strOutError))
					return false;
				RotationRateDegrees = A[0u] * 360.0;
				break;
			case OPCODE::SIZE_MULTIPLY_LIFE:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					RelativeAge, RandomState, DrawCount, A, strOutError))
					return false;
				for (size_t Index = 0u; Index < SourceScale.size(); ++Index)
					SourceScale[Index] *= A[Index];
				break;
			case OPCODE::CYLINDER_Z:
			{
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				const double Radius = A[0u];
				if (!Evaluate_PreparedDistribution(ModuleOperation, 1u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				const double Height = A[0u];
				const double AngleUnit = static_cast<double>(
					Next_Random(RandomState)) / static_cast<double>(
						(std::numeric_limits<uint32_t>::max)());
				++DrawCount;
				const double Angle = AngleUnit *
					6.283185307179586476925286766559;
				const double HeightUnit = static_cast<double>(
					Next_Random(RandomState)) / static_cast<double>(
						(std::numeric_limits<uint32_t>::max)());
				++DrawCount;
				const std::array<double, 3u> Offset{
					std::cos(Angle) * Radius,
					std::sin(Angle) * Radius,
					(HeightUnit - 0.5) * Height };
				if (!Evaluate_PreparedDistribution(ModuleOperation, 3u,
					EmitterTime, RandomState, DrawCount, B, strOutError) ||
					!Evaluate_PreparedDistribution(ModuleOperation, 2u, EmitterTime,
						RandomState, DrawCount, A, strOutError))
					return false;
				for (size_t Index = 0u; Index < Offset.size(); ++Index)
				{
					SourcePosition[Index] += Offset[Index] + B[Index];
					SourceVelocity[Index] += Offset[Index] * A[0u];
				}
				break;
			}
			case OPCODE::LOCATION:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					EmitterTime, RandomState, DrawCount, A, strOutError))
					return false;
				for (size_t Index = 0u; Index < SourcePosition.size(); ++Index)
					SourcePosition[Index] += A[Index];
				break;
			case OPCODE::GROUND:
				if (!Evaluate_PreparedDistribution(ModuleOperation, 0u,
					RelativeAge, RandomState, DrawCount, A, strOutError) ||
					!Evaluate_PreparedDistribution(ModuleOperation, 1u, RelativeAge,
						RandomState, DrawCount, B, strOutError))
					return false;
				if (B[0u] < 0.5)
				{
					for (size_t Index = 0u; Index < SourcePosition.size(); ++Index)
						SourcePosition[Index] += A[Index];
				}
				break;
			case OPCODE::DYNAMIC_PARAMETER:
				if (ModuleOperation.iDistributionCount != Dynamic.size())
					return Reject(strOutError,
						"Selected evaluator dynamic parameter count is invalid.");
				for (size_t Index = 0u; Index < Dynamic.size(); ++Index)
				{
					if (!Evaluate_PreparedDistribution(ModuleOperation,
						static_cast<uint32_t>(Index), RelativeAge, RandomState,
						DrawCount, A, strOutError))
						return false;
					Dynamic[Index] = A[0u];
				}
				break;
			default:
				return Reject(strOutError,
					"Selected evaluator encountered an unknown prepared opcode.");
			}
		}

		SELECTED_VISUAL_RESULT Staged;
		Staged.iFinalRandomState = RandomState;
		Staged.iRandomDrawCount = DrawCount;
		if (Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH)
		{
			Staged.Values.vMeshDimensionlessScaleXzy =
				std::array<double, 3u>{ SourceScale[0u], SourceScale[2u],
					SourceScale[1u] };
		}
		else if (Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE)
		{
			Staged.Values.vSpriteSignedWorldSizeXzy =
				std::array<double, 3u>{ SourceScale[0u] * 0.01,
					SourceScale[2u] * 0.01, SourceScale[1u] * 0.01 };
		}
		else
		{
			return Reject(strOutError,
				"Selected evaluator renderer kind is invalid.");
		}
		Staged.Values.vLocalPosition = Source_ToClientVector(SourcePosition, 0.01);
		Staged.Values.vVelocityPerSecond =
			Source_ToClientVector(SourceVelocity, 0.01);
		Staged.Values.vAccelerationPerSecondSquared =
			Source_ToClientVector(SourceAcceleration, 0.01);
		Staged.Values.vColor = Color;
		Staged.Values.vDynamicParameter = Dynamic;
		Staged.Values.fRotationDegrees = RotationDegrees;
		Staged.Values.fRotationRateDegreesPerSecond = RotationRateDegrees;
		if ((bValidateExpectedResult &&
			 (Staged.iRandomDrawCount !=
				  Selection.iExpectedVisualRandomDrawCount ||
			  Staged.iFinalRandomState != Selection.iExpectedFinalRandomState)) ||
			!Is_Finite(Staged.Values.vLocalPosition) ||
			!Is_Finite(Staged.Values.vVelocityPerSecond) ||
			!Is_Finite(Staged.Values.vAccelerationPerSecondSquared) ||
			!Is_Finite(Staged.Values.vColor) ||
			!Is_Finite(Staged.Values.vDynamicParameter) ||
			!Is_Finite(Staged.Values.fRotationDegrees) ||
			!Is_Finite(Staged.Values.fRotationRateDegreesPerSecond) ||
			(Staged.Values.vMeshDimensionlessScaleXzy.has_value() &&
			 !Is_Finite(*Staged.Values.vMeshDimensionlessScaleXzy)) ||
			(Staged.Values.vSpriteSignedWorldSizeXzy.has_value() &&
			 !Is_Finite(*Staged.Values.vSpriteSignedWorldSizeXzy)))
		{
			return Reject(strOutError,
				"Selected evaluator visual result failed its closed contract.");
		}
		OutResult = std::move(Staged);
		return true;
	}

	template <typename T>
	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Program_RowIdentity(
		const T& Row)
	{
		return { Row.Row.strId, Row.Row.strRowSha256 };
	}

	EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY Sidecar_RowIdentity(
		const std::string& strId,
		const std::string& strRowSha256)
	{
		return { strId, strRowSha256 };
	}

	template <typename T, typename Predicate>
	const T* Find_UniqueProgramRow(
		const std::vector<T>& Rows,
		Predicate&& Matches)
	{
		const T* Found = nullptr;
		for (const T& Row : Rows)
		{
			if (!Matches(Row))
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Row;
		}
		return Found;
	}

	template <typename T>
	const T* Find_UniqueProgramRowById(
		const std::vector<T>& Rows,
		const std::string_view strId)
	{
		return Find_UniqueProgramRow(Rows, [strId](const T& Row)
		{
			return Row.Row.strId == strId;
		});
	}

	template <typename T, typename Predicate>
	const std::pair<const std::string, T>* Find_UniqueSidecarRow(
		const std::map<std::string, T, std::less<>>& Rows,
		Predicate&& Matches)
	{
		const std::pair<const std::string, T>* Found = nullptr;
		for (const auto& Row : Rows)
		{
			if (!Matches(Row.second))
				continue;
			if (nullptr != Found)
				return nullptr;
			Found = &Row;
		}
		return Found;
	}

	const EFFECT_RUNTIME_PROGRAM_PROPERTY* Find_OwnedProperty(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const std::string_view strModuleId,
		const std::string_view strPropertyPath)
	{
		return Find_UniqueProgramRow(Program.Properties,
			[strModuleId, strPropertyPath](const auto& Row)
			{
				return Row.strModuleId == strModuleId &&
					Row.strPropertyPath == strPropertyPath;
			});
	}

	const EFFECT_RUNTIME_PROGRAM_LITERAL* Find_OwnedLiteral(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const std::string_view strModuleId,
		const std::string_view strPropertyPath)
	{
		return Find_UniqueProgramRow(Program.Literals,
			[strModuleId, strPropertyPath](const auto& Row)
			{
				return Row.strModuleId == strModuleId &&
					Row.strPropertyPath == strPropertyPath;
			});
	}

	bool_t Build_DiagnosticHandlers(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		std::vector<EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY> Staged;
		Staged.reserve(Emitter.ModuleIds.size());
		for (const std::string& ModuleId : Emitter.ModuleIds)
		{
			const auto* Module = Find_UniqueProgramRowById(
				Program.Modules, ModuleId);
			const auto* Handler = nullptr == Module ? nullptr :
				Find_UniqueProgramRowById(
					Program.Handlers, Module->strHandlerRegistryId);
			if (nullptr == Module || nullptr == Handler ||
				Module->strEmitterId != Emitter.Row.strId ||
				Handler->strExactSourceClass != Module->strExactSourceClass)
			{
				return Reject(strOutError,
					"Diagnostic selector module/handler authority is ambiguous.");
			}
			EFFECT_RECONSTRUCTED_SELECTED_HANDLER_IDENTITY Selection;
			Selection.Module = Program_RowIdentity(*Module);
			Selection.Handler = Program_RowIdentity(*Handler);
			Selection.strExactSourceClass = Module->strExactSourceClass;
			Selection.strImplementationId = Handler->strImplementationId;
			Selection.iImplementationVersion = Handler->iImplementationVersion;
			Selection.strImplementationSha256 =
				Handler->strImplementationSha256;
			for (const std::string& DefaultId : Module->ImplicitDefaultIds)
			{
				const auto* Default = Find_UniqueProgramRowById(
					Program.ImplicitDefaults, DefaultId);
				if (nullptr == Default || Default->strModuleId != ModuleId)
				{
					return Reject(strOutError,
						"Diagnostic selector implicit default join is ambiguous.");
				}
				Selection.ImplicitDefaults.push_back(
					Program_RowIdentity(*Default));
			}
			Staged.push_back(std::move(Selection));
		}
		if (Staged.empty())
			return Reject(strOutError,
				"Diagnostic selector emitter has no handler route.");
		OutSelection.Handlers = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticCylinder(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		const EFFECT_RUNTIME_PROGRAM_MODULE* Cylinder = nullptr;
		for (const std::string& ModuleId : Emitter.ModuleIds)
		{
			const auto* Module = Find_UniqueProgramRowById(
				Program.Modules, ModuleId);
			if (nullptr == Module)
				return Reject(strOutError,
					"Diagnostic selector cylinder module is absent.");
			if (Module->strExactSourceClass !=
				"particlemodulelocationprimitivecylinder")
				continue;
			if (nullptr != Cylinder)
				return Reject(strOutError,
					"Diagnostic selector cylinder module is duplicated.");
			Cylinder = Module;
		}
		if (nullptr == Cylinder)
			return true;
		const auto* SurfaceOnly = Find_OwnedLiteral(
			Program, Cylinder->Row.strId, "surfaceonly");
		const auto* Velocity = Find_OwnedLiteral(
			Program, Cylinder->Row.strId, "velocity");
		if (nullptr == SurfaceOnly || nullptr == Velocity)
			return Reject(strOutError,
				"Diagnostic selector cylinder literals are ambiguous.");
		EFFECT_RECONSTRUCTED_SELECTED_CYLINDER_POLICY Staged;
		Staged.Module = Program_RowIdentity(*Cylinder);
		Staged.SurfaceOnlyLiteral = Program_RowIdentity(*SurfaceOnly);
		Staged.VelocityLiteral = Program_RowIdentity(*Velocity);
		Staged.strAbsentHeightAxisDefault = "Z";
		OutSelection.Cylinder = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticGeometry(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		if (!Emitter.strGeometryUseId.has_value())
			return Reject(strOutError,
				"Diagnostic Mesh selector has no geometry use.");
		const auto* Use = Find_UniqueProgramRowById(
			Program.GeometryUses, *Emitter.strGeometryUseId);
		const auto* Carrier = nullptr == Use ? nullptr :
			Find_UniqueProgramRowById(Program.GeometryCarriers, Use->strCarrierId);
		if (nullptr == Use || nullptr == Carrier ||
			Use->strEmitterId != Emitter.Row.strId)
		{
			return Reject(strOutError,
				"Diagnostic Mesh geometry join is ambiguous.");
		}
		uint64_t VertexCount = 0u;
		uint64_t IndexCount = 0u;
		for (const auto& Submesh : Carrier->Submeshes)
		{
			VertexCount += Submesh.iVertexCount;
			IndexCount += Submesh.iIndexCount;
		}
		if (VertexCount > (std::numeric_limits<uint32_t>::max)() ||
			IndexCount > (std::numeric_limits<uint32_t>::max)())
		{
			return Reject(strOutError,
				"Diagnostic Mesh geometry count overflowed.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING Staged;
		Staged.GeometryUse = Program_RowIdentity(*Use);
		Staged.GeometryCarrier = Program_RowIdentity(*Carrier);
		Staged.strRuntimeAssetId = Carrier->strAssetId;
		Staged.iCandidateResourceByteSize = Carrier->iCandidateResourceByteSize;
		Staged.strCandidateResourceSha256 = Carrier->strCandidateResourceSha256;
		Staged.strPayloadSha256 = Carrier->strPayloadSha256;
		Staged.strMetadataIdentitySha256 = Carrier->strMetadataIdentitySha256;
		Staged.strCacheIdentitySha256 = Carrier->strCacheIdentitySha256;
		Staged.strExpectedTupleSha256 = Carrier->strExpectedTupleSha256;
		Staged.strApprovalGeometryRowSha256 =
			Carrier->strApprovalGeometryRowSha256;
		Staged.strPreparedCacheIdentitySha256 =
			Carrier->strPreparedCacheIdentitySha256;
		Staged.fGeometryPreScale = Carrier->fGeometryPreScale;
		Staged.iSubmeshCount = static_cast<uint32_t>(Carrier->Submeshes.size());
		Staged.iVertexCount = static_cast<uint32_t>(VertexCount);
		Staged.iIndexCount = static_cast<uint32_t>(IndexCount);
		OutSelection.Geometry = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticSpriteSink(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		const auto* Required = Find_UniqueProgramRowById(
			Program.Modules, Emitter.Timing.strRequiredModuleId);
		const auto* AlignmentProperty = nullptr == Required ? nullptr :
			Find_OwnedProperty(Program, Required->Row.strId, "screenalignment");
		const auto* AlignmentLiteral = nullptr == Required ? nullptr :
			Find_OwnedLiteral(Program, Required->Row.strId, "screenalignment");
		const auto* FlipProperty = nullptr == Required ? nullptr :
			Find_OwnedProperty(Program, Required->Row.strId,
				"ballowimageflipping");
		const auto* FlipLiteral = nullptr == Required ? nullptr :
			Find_OwnedLiteral(Program, Required->Row.strId,
				"ballowimageflipping");
		const auto* OffsetProperty = nullptr == Required ? nullptr :
			Find_OwnedProperty(Program, Required->Row.strId, "boffsetcenter");
		const auto* OffsetLiteral = nullptr == Required ? nullptr :
			Find_OwnedLiteral(Program, Required->Row.strId, "boffsetcenter");
		const auto* OffsetXProperty = nullptr == Required ? nullptr :
			Find_OwnedProperty(Program, Required->Row.strId, "offsetcenterx");
		const auto* OffsetXLiteral = nullptr == Required ? nullptr :
			Find_OwnedLiteral(Program, Required->Row.strId, "offsetcenterx");
		const auto* OffsetYProperty = nullptr == Required ? nullptr :
			Find_OwnedProperty(Program, Required->Row.strId, "offsetcentery");
		const auto* OffsetYLiteral = nullptr == Required ? nullptr :
			Find_OwnedLiteral(Program, Required->Row.strId, "offsetcentery");
		if (nullptr == Required || nullptr == AlignmentProperty ||
			nullptr == AlignmentLiteral || nullptr == FlipProperty ||
			nullptr == FlipLiteral || nullptr == OffsetProperty ||
			nullptr == OffsetLiteral || nullptr == OffsetXProperty ||
			nullptr == OffsetXLiteral || nullptr == OffsetYProperty ||
			nullptr == OffsetYLiteral ||
			AlignmentLiteral->strEnumValue != "psa_velocity" ||
			!FlipLiteral->bValue.has_value() ||
			!OffsetLiteral->bValue.has_value() ||
			!OffsetXLiteral->fValue.has_value() ||
			!OffsetYLiteral->fValue.has_value() ||
			!Emitter.RendererRuntimeConfig.Sprite.has_value())
		{
			return Reject(strOutError,
				"Diagnostic Sprite sink authority is incomplete.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK Staged;
		Staged.RequiredModule = Program_RowIdentity(*Required);
		Staged.ScreenAlignmentProperty = Program_RowIdentity(*AlignmentProperty);
		Staged.ScreenAlignmentLiteral = Program_RowIdentity(*AlignmentLiteral);
		Staged.AllowImageFlippingProperty = Program_RowIdentity(*FlipProperty);
		Staged.AllowImageFlippingLiteral = Program_RowIdentity(*FlipLiteral);
		Staged.OffsetCenterEnabledProperty = Program_RowIdentity(*OffsetProperty);
		Staged.OffsetCenterEnabledLiteral = Program_RowIdentity(*OffsetLiteral);
		Staged.OffsetCenterXProperty = Program_RowIdentity(*OffsetXProperty);
		Staged.OffsetCenterXLiteral = Program_RowIdentity(*OffsetXLiteral);
		Staged.OffsetCenterYProperty = Program_RowIdentity(*OffsetYProperty);
		Staged.OffsetCenterYLiteral = Program_RowIdentity(*OffsetYLiteral);
		Staged.eAlignment = EFFECT_RECONSTRUCTED_SPRITE_ALIGNMENT::VELOCITY;
		Staged.eOrientation = EFFECT_RECONSTRUCTED_SPRITE_ORIENTATION::
			CAMERA_BILLBOARD_WITH_VELOCITY_ALIGNMENT;
		Staged.bAllowImageFlipping = *FlipLiteral->bValue;
		Staged.bOffsetCenter = *OffsetLiteral->bValue;
		Staged.vPivotCenter = {
			*OffsetXLiteral->fValue, *OffsetYLiteral->fValue };
		Staged.bBillboard = Emitter.RendererRuntimeConfig.Sprite->bBillboard;
		Staged.fBillboardRollDegrees =
			Emitter.RendererRuntimeConfig.Sprite->fBillboardRollDegrees;
		OutSelection.SpriteSink = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticTextureLane(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence,
		const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER& Provider,
		const std::string_view strShaderVariable,
		EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE& OutLane,
		std::string& strOutError)
	{
		if (Provider.strProviderKind != "MATERIAL_TEXTURE_BINDING")
			return Reject(strOutError,
				"Diagnostic material lane is not texture-backed.");
		const auto* Input = Find_UniqueProgramRowById(
			Program.MaterialInputs, Provider.strMaterialInputFieldId);
		const auto* Binding = Find_UniqueProgramRowById(
			Program.MaterialTextureBindings, Provider.strTextureBindingId);
		const auto* Policy = Find_UniqueProgramRowById(
			Program.MaterialPolicies, Provider.strSamplerPolicyRowId);
		const auto* SidecarBinding = nullptr == Binding ? nullptr :
			Find_UniqueSidecarRow(Authority.TextureBindingsById,
				[Binding](const auto& Row)
				{
					return Row.strCandidateBindingId == Binding->Row.strId &&
						Row.strCandidateBindingRowSha256 ==
							Binding->Row.strRowSha256;
				});
		const auto ResourceIt = nullptr == SidecarBinding ?
			Authority.TextureResourcesById.end() :
			Authority.TextureResourcesById.find(
				SidecarBinding->second.strResourceAuthorityId);
		if (nullptr == Input || nullptr == Binding || nullptr == Policy ||
			nullptr == SidecarBinding ||
			ResourceIt == Authority.TextureResourcesById.end())
		{
			return Reject(strOutError,
				"Diagnostic material texture lane join is ambiguous.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE Staged;
		Staged.strShaderVariableName = std::string(strShaderVariable);
		Staged.MaterialInput = Program_RowIdentity(*Input);
		Staged.MaterialTextureBinding = Program_RowIdentity(*Binding);
		Staged.MaterialPolicy = Program_RowIdentity(*Policy);
		Staged.SidecarTextureBinding = Sidecar_RowIdentity(
			SidecarBinding->first, SidecarBinding->second.strRowSha256);
		Staged.SidecarTextureResource = Sidecar_RowIdentity(
			ResourceIt->first, ResourceIt->second.strRowSha256);
		Staged.strRuntimeAssetId = ResourceIt->second.strRuntimeAssetId;
		Staged.strRawSha256 = ResourceIt->second.strRawSha256;

		const auto* Renderer = Find_UniqueProgramRow(
			Program.RendererTextureResources, [&](const auto& Row)
			{
				return Row.strEmitterId == Emitter.Row.strId &&
					Row.strMaterialOccurrenceId == Occurrence.Row.strId &&
					Row.strAssetId == Staged.strRuntimeAssetId;
			});
		if (nullptr != Renderer)
		{
			const auto* Decision = Find_UniqueSidecarRow(
				Authority.RendererSlotBindingsById, [&](const auto& Row)
				{
					return Row.strTextureResourceId == Renderer->Row.strId &&
						Row.strRendererResourceRowSha256 ==
							Renderer->Row.strRowSha256 &&
						Row.strMaterialOccurrenceId == Occurrence.Row.strId;
				});
			if (nullptr == Decision)
				return Reject(strOutError,
					"Diagnostic renderer texture decision is ambiguous.");
			Staged.RendererTextureResource = Program_RowIdentity(*Renderer);
			Staged.SidecarRendererSlotDecision = Sidecar_RowIdentity(
				Decision->first, Decision->second.strRowSha256);
		}
		OutLane = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticStateBinding(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE& Recipe,
		const std::string_view strFieldName,
		EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING& OutBinding,
		std::string& strOutError)
	{
		const auto* Binding = Find_UniqueProgramRow(
			Program.MaterialRenderBindings, [&](const auto& Row)
			{
				return Row.strRecipeId == Recipe.Row.strId &&
					Row.strFieldName == strFieldName;
			});
		if (nullptr == Binding ||
			std::find(Recipe.RenderBindingIds.begin(),
				Recipe.RenderBindingIds.end(), Binding->Row.strId) ==
				Recipe.RenderBindingIds.end())
		{
			return Reject(strOutError,
				"Diagnostic render-state binding is ambiguous.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING Staged;
		Staged.ProgramBinding = Program_RowIdentity(*Binding);
		if (!Binding->strPolicyRowId.empty())
		{
			const auto* Policy = Find_UniqueProgramRowById(
				Program.MaterialPolicies, Binding->strPolicyRowId);
			if (nullptr == Policy)
				return Reject(strOutError,
					"Diagnostic render-state policy is ambiguous.");
			Staged.ProgramPolicy = Program_RowIdentity(*Policy);
		}
		const auto* Decision = Find_UniqueSidecarRow(
			Authority.RenderStateDescriptorsById, [Binding](const auto& Row)
			{
				return Row.strRenderBindingId == Binding->Row.strId &&
					Row.strRenderBindingRowSha256 == Binding->Row.strRowSha256;
			});
		if (nullptr != Decision)
		{
			Staged.SidecarDecision = Sidecar_RowIdentity(
				Decision->first, Decision->second.strRowSha256);
		}
		OutBinding = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticMaterial(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		if (!Emitter.strMaterialOccurrenceId.has_value())
			return Reject(strOutError,
				"Diagnostic selector has no material occurrence.");
		const auto* Occurrence = Find_UniqueProgramRowById(
			Program.MaterialOccurrences, *Emitter.strMaterialOccurrenceId);
		const auto* Recipe = nullptr == Occurrence ? nullptr :
			Find_UniqueProgramRowById(
				Program.MaterialRecipes, Occurrence->strRecipeId);
		const auto* Family = nullptr == Occurrence ? nullptr :
			Find_UniqueProgramRowById(
				Program.MaterialFamilies, Occurrence->strFamilyId);
		const auto* TextureDecision = nullptr == Recipe || nullptr == Family ?
			nullptr : Find_UniqueSidecarRow(
				Authority.RecipeTextureBindingsById, [&](const auto& Row)
				{
					return Row.strRecipeId == Recipe->Row.strId &&
						Row.strRecipeRowSha256 == Recipe->Row.strRowSha256 &&
						Row.strFamilyId == Family->Row.strId &&
						Row.strFamilyRowSha256 == Family->Row.strRowSha256;
				});
		if (nullptr == Occurrence || nullptr == Recipe || nullptr == Family ||
			nullptr == TextureDecision || Recipe->NumericBindingSamples.empty())
		{
			return Reject(strOutError,
				"Diagnostic material authority is incomplete.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING Staged;
		Staged.Occurrence = Program_RowIdentity(*Occurrence);
		Staged.Recipe = Program_RowIdentity(*Recipe);
		Staged.Family = Program_RowIdentity(*Family);
		Staged.RecipeTextureDecision = Sidecar_RowIdentity(
			TextureDecision->first, TextureDecision->second.strRowSha256);
		Staged.strEvaluatorId = Family->strEvaluatorId;
		Staged.iEvaluatorVersion = Family->iEvaluatorVersion;
		Staged.strEvaluatorSha256 = Family->strEvaluatorSha256;
		Staged.iFeatureMask = Family->iFeatureMask;
		if (!Build_DiagnosticTextureLane(Program, Authority, Emitter,
			*Occurrence, TextureDecision->second.Texture0Provider,
			"g_SourceTexture0", Staged.TextureLanes[0u], strOutError) ||
			!Build_DiagnosticTextureLane(Program, Authority, Emitter,
				*Occurrence, TextureDecision->second.Texture1Provider,
				"g_SourceTexture1", Staged.TextureLanes[1u], strOutError) ||
			!Build_DiagnosticStateBinding(Program, Authority, *Recipe,
				"blendmode", Staged.BlendState, strOutError) ||
			!Build_DiagnosticStateBinding(Program, Authority, *Recipe,
				"twosided", Staged.RasterizerState, strOutError) ||
			!Build_DiagnosticStateBinding(Program, Authority, *Recipe,
				"bdisabledepthtest", Staged.DepthStencilState, strOutError))
		{
			return false;
		}
		const auto& Sample = Recipe->NumericBindingSamples.front();
		Staged.Constants.vUvScale = Sample.vUvScale;
		Staged.Constants.vPanRotationAux = Sample.vPanRotationAux;
		Staged.Constants.vColor = Sample.vColor;
		Staged.Constants.vParams0 = Sample.vParams0;
		Staged.Constants.vParams1 = Sample.vParams1;
		Staged.Shader.strTechniqueName = "DefaultTechnique";
		Staged.Shader.strEvaluatorEnabledVariable =
			"g_ReconstructedMaterialEvaluatorEnabled";
		Staged.Shader.strFeatureMaskVariable =
			"g_ReconstructedMaterialFeatureMask";
		Staged.Shader.strUvScaleVariable = "g_ReconstructedUVScale";
		Staged.Shader.strPanRotationAuxVariable =
			"g_ReconstructedPanRotationAux";
		Staged.Shader.strColorVariable = "g_ReconstructedColor";
		Staged.Shader.strParams0Variable = "g_ReconstructedParams0";
		Staged.Shader.strParams1Variable = "g_ReconstructedParams1";
		if (Emitter.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE)
		{
			Staged.Shader.strShaderAssetId =
				"Shader_VtxEffectMeshPreview.hlsl";
			Staged.Shader.strPassName = "OpaqueBackDepthWrite";
			Staged.Shader.iPassIndex = 0u;
		}
		else if (Emitter.eRenderer ==
			EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE)
		{
			Staged.Shader.strShaderAssetId = "Shader_VtxEffectParticle.hlsl";
			Staged.Shader.strPassName = "AlphaTwoSidedDepthRead";
			Staged.Shader.iPassIndex = 1u;
		}
		else
		{
			return Reject(strOutError,
				"Diagnostic selector renderer has no Mesh/Sprite shader route.");
		}
		OutSelection.Material = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticTiming(
		const FIXED_STEP_SIMULATION& Simulation,
		const EFFECT_RECONSTRUCTED_EXECUTION_EMITTER& PlanEmitter,
		const uint64_t iSpawnSerial,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET& OutTiming,
		std::string& strOutError)
	{
		const auto StateIt = Simulation.States.find(PlanEmitter.strEmitterId);
		if (StateIt == Simulation.States.end())
			return Reject(strOutError,
				"Diagnostic selector timing emitter is absent.");
		const auto Matches = [iSpawnSerial](const TIMING_PARTICLE& Particle)
		{
			return Particle.iSpawnSerial == iSpawnSerial;
		};
		const auto ParticleIt = std::find_if(StateIt->second.Particles.begin(),
			StateIt->second.Particles.end(), Matches);
		if (ParticleIt == StateIt->second.Particles.end() ||
			std::count_if(StateIt->second.Particles.begin(),
				StateIt->second.Particles.end(), Matches) != 1u)
		{
			return Reject(strOutError,
				"Diagnostic selector occurrence is absent or duplicated.");
		}
		EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET Timing;
		Timing.strOccurrenceId = ParticleIt->strOccurrenceId;
		Timing.strScheduleId = PlanEmitter.strScheduleId;
		Timing.strEmitterId = PlanEmitter.strEmitterId;
		Timing.eRenderer = PlanEmitter.eRenderer;
		Timing.iLoopIndex = ParticleIt->iLoopIndex;
		Timing.iSpawnSerial = ParticleIt->iSpawnSerial;
		Timing.iSpawnStep = ParticleIt->iSpawnStep;
		Timing.iOccurrenceRandomValue = ParticleIt->iOccurrenceRandomValue;
		Timing.iLifetimeRandomValue = ParticleIt->iLifetimeRandomValue;
		Timing.fAgeSeconds = (std::max)(0.0,
			Simulation.fSampleTimeSeconds - ParticleIt->fSpawnTimeSeconds);
		Timing.fLifetimeSeconds = ParticleIt->fLifetimeSeconds;
		if (Timing.iSpawnStep != Simulation.iFixedStepIndex ||
			Timing.fAgeSeconds != 0.0)
		{
			return Reject(strOutError,
				"Diagnostic selector must address a just-spawned occurrence.");
		}
		OutSelection.iExpectedOccurrenceRandomValue =
			Timing.iOccurrenceRandomValue;
		OutSelection.iExpectedLifetimeRandomValue = Timing.iLifetimeRandomValue;
		OutSelection.fExpectedLifetimeSeconds = Timing.fLifetimeSeconds;
		OutTiming = std::move(Timing);
		return true;
	}

	bool_t Build_DiagnosticSelection(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const FIXED_STEP_SIMULATION& Simulation,
		const std::string_view strScheduleId,
		const std::string_view strEmitterId,
		const uint64_t iSpawnSerial,
		const EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND eKind,
		const uint32_t iSelectionIndex,
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& OutSelection,
		std::string& strOutError)
	{
		const auto* ProgramSchedule = Find_UniqueProgramRowById(
			Program.ActionSchedules, strScheduleId);
		const auto* ProgramEmitter = Find_UniqueProgramRowById(
			Program.Emitters, strEmitterId);
		const auto PlanEmitterIt = Plan.Get_Emitters().find(strEmitterId);
		if (nullptr == ProgramSchedule || nullptr == ProgramEmitter ||
			PlanEmitterIt == Plan.Get_Emitters().end() ||
			ProgramEmitter->strScheduleId != strScheduleId ||
			PlanEmitterIt->second.strScheduleId != strScheduleId)
		{
			return Reject(strOutError,
				"Diagnostic selector schedule/emitter join is ambiguous.");
		}
		EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION Staged;
		Staged.eKind = eKind;
		Staged.eRenderer = ProgramEmitter->eRenderer;
		Staged.Schedule = Program_RowIdentity(*ProgramSchedule);
		Staged.Emitter = Program_RowIdentity(*ProgramEmitter);
		Staged.iEmitterOrder = ProgramEmitter->Row.iOrder;
		Staged.bLocalSpace = ProgramEmitter->bLocalSpace;
		Staged.strSizeUnitPolicy = ProgramEmitter->strSizeUnitPolicy;
		if (!Build_DiagnosticHandlers(Program, *ProgramEmitter, Staged,
			strOutError) ||
			!Build_DiagnosticCylinder(Program, *ProgramEmitter, Staged,
				strOutError) ||
			(eKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH &&
			 !Build_DiagnosticGeometry(Program, *ProgramEmitter, Staged,
				 strOutError)) ||
			(eKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE &&
			 !Build_DiagnosticSpriteSink(Program, *ProgramEmitter, Staged,
				 strOutError)) ||
			!Build_DiagnosticMaterial(Program, Authority, *ProgramEmitter, Staged,
				strOutError))
		{
			return false;
		}
		EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET Timing;
		if (!Build_DiagnosticTiming(Simulation, PlanEmitterIt->second,
			iSpawnSerial, Staged, Timing, strOutError))
		{
			return false;
		}
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION Operation;
		if (!Validate_EmitterSelection(Plan, Program, Authority, Staged,
			iSelectionIndex, false, Operation, strOutError))
		{
			return false;
		}
		SELECTED_VISUAL_RESULT Visual;
		if (!Evaluate_SelectedVisual(Plan, Operation, Staged, Timing,
			Simulation.fSampleTimeSeconds, false, Visual, strOutError) ||
			0u == Visual.iRandomDrawCount || 0u == Visual.iFinalRandomState)
		{
			if (strOutError.empty())
				Reject(strOutError,
					"Diagnostic selector visual expectation is empty.");
			return false;
		}
		Staged.iExpectedVisualRandomDrawCount = Visual.iRandomDrawCount;
		Staged.iExpectedFinalRandomState = Visual.iFinalRandomState;
		OutSelection = std::move(Staged);
		return true;
	}

	bool_t Build_DiagnosticRequest(
		const EFFECT_RECONSTRUCTED_EXECUTION_PLAN& Plan,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority,
		const EFFECT_RECONSTRUCTED_SELECTED_DIAGNOSTIC_SELECTOR& Selector,
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST& OutRequest,
		std::string& strOutError)
	{
		if (Selector.strScheduleId.empty() ||
			Selector.strMeshEmitterId.empty() ||
			Selector.strSpriteEmitterId.empty() ||
			Selector.strMeshEmitterId == Selector.strSpriteEmitterId ||
			0u == Selector.iFixedStepIndex ||
			Selector.iFixedStepIndex >
				static_cast<uint64_t>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ) * 60u)
		{
			return Reject(strOutError,
				"Diagnostic Mesh/Sprite selector is invalid.");
		}
		FIXED_STEP_SIMULATION Simulation;
		if (!Simulate_FixedSteps(
			Plan, Selector.iFixedStepIndex, Simulation, strOutError))
		{
			return false;
		}
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST Staged;
		Staged.iEvaluatorVersion =
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_VERSION;
		Staged.strOccurrenceRngContract =
			EFFECT_RECONSTRUCTED_SELECTED_OCCURRENCE_RNG_CONTRACT;
		Staged.iOccurrenceRngVersion =
			EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION;
		Staged.iRequiredFixedStepIndex = Selector.iFixedStepIndex;
		Staged.iRequiredSpawnSerial = Selector.iSpawnSerial;
		Staged.Emitters.resize(2u);
		if (!Build_DiagnosticSelection(Plan, Program, Authority, Simulation,
			Selector.strScheduleId, Selector.strMeshEmitterId,
			Selector.iSpawnSerial,
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH, 0u,
			Staged.Emitters[0u], strOutError) ||
			!Build_DiagnosticSelection(Plan, Program, Authority, Simulation,
				Selector.strScheduleId, Selector.strSpriteEmitterId,
				Selector.iSpawnSerial,
				EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE, 1u,
				Staged.Emitters[1u], strOutError))
		{
			return false;
		}
		const uint64_t HandlerCount =
			static_cast<uint64_t>(Staged.Emitters[0u].Handlers.size()) +
			static_cast<uint64_t>(Staged.Emitters[1u].Handlers.size());
		if (0u == HandlerCount ||
			HandlerCount > (std::numeric_limits<uint32_t>::max)())
		{
			return Reject(strOutError,
				"Diagnostic selector handler count overflowed.");
		}
		Staged.iExpectedConsumedHandlerCount =
			static_cast<uint32_t>(HandlerCount);
		OutRequest = std::move(Staged);
		return true;
	}

	void Append_SelectedRow(
		std::string& Projection,
		const EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY& Row)
	{
		Append_Projection(Projection, Row.strId);
		Append_Projection(Projection, Row.strRowSha256);
	}

	void Append_SelectedOptionalRow(
		std::string& Projection,
		const std::optional<EFFECT_RECONSTRUCTED_SELECTED_ROW_IDENTITY>& Row)
	{
		Append_Integer(Projection, Row.has_value() ? 1u : 0u);
		if (Row.has_value())
			Append_SelectedRow(Projection, *Row);
	}

	std::string Compute_SelectedFrameProjection(
		const EFFECT_RECONSTRUCTED_SELECTED_FRAME& Frame)
	{
		const auto Preparation = Frame.Get_Preparation();
		const auto Entry = nullptr == Preparation ? nullptr :
			Preparation->Get_CatalogEntry();
		if (nullptr == Preparation || nullptr == Entry)
			return {};
		const auto& Identity = Entry->Get_Identity();
		std::string Projection;
		Append_Integer(Projection, Identity.iCatalogRevision);
		Append_Projection(Projection, Identity.strEffectAssetId);
		Append_Projection(Projection, Identity.strProgramId);
		Append_Projection(Projection, Identity.strProgramSha256);
		Append_Projection(Projection, Identity.strCandidateRawSha256);
		Append_Projection(Projection,
			Identity.strRenderResourceSidecarRawSha256);
		Append_Projection(Projection,
			Identity.strRenderResourceSidecarDecisionProjectionSha256);
		Append_Projection(Projection,
			Identity.strRenderResourceSidecarReceiptSha256);
		Append_Projection(Projection, Frame.Get_OccurrenceRngContract());
		Append_Integer(Projection, Frame.Get_OccurrenceRngVersion());
		Append_Integer(Projection, Frame.Get_FixedStepIndex());
		Append_F64(Projection, Frame.Get_SampleTimeSeconds());
		Append_Integer(Projection, Frame.Get_ConsumedHandlerCount());
		for (const auto& Packet : Frame.Get_Packets())
		{
			const auto& Selection = Preparation->Get_Request().Emitters.at(
				Packet.Get_SelectionIndex());
			Append_Integer(Projection, Packet.Get_SelectionIndex());
			Append_Integer(Projection, static_cast<uint32_t>(Packet.Get_Kind()));
			Append_SelectedRow(Projection, Selection.Schedule);
			Append_SelectedRow(Projection, Selection.Emitter);
			Append_Projection(Projection, Packet.Get_Timing().strOccurrenceId);
			Append_Integer(Projection, Packet.Get_Timing().iSpawnSerial);
			Append_Integer(Projection, Packet.Get_Timing().iSpawnStep);
			Append_Integer(Projection, Packet.Get_Timing().iOccurrenceRandomValue);
			Append_Integer(Projection, Packet.Get_Timing().iLifetimeRandomValue);
			Append_F64(Projection, Packet.Get_Timing().fLifetimeSeconds);
			Append_Integer(Projection, Packet.Get_FinalRandomState());
			Append_Integer(Projection, Packet.Get_RandomDrawCount());
			for (const auto& Handler : Packet.Get_ConsumedHandlers())
			{
				Append_SelectedRow(Projection, Handler.Module);
				Append_SelectedRow(Projection, Handler.Handler);
				Append_Projection(Projection, Handler.strImplementationId);
				Append_Integer(Projection, Handler.iImplementationVersion);
				Append_Projection(Projection, Handler.strImplementationSha256);
			}
			Append_SelectedRow(Projection, Selection.Material.Occurrence);
			Append_SelectedRow(Projection, Selection.Material.Recipe);
			Append_SelectedRow(Projection, Selection.Material.Family);
			Append_SelectedRow(Projection,
				Selection.Material.RecipeTextureDecision);
			for (const auto& Lane : Selection.Material.TextureLanes)
			{
				Append_SelectedOptionalRow(Projection, Lane.RendererTextureResource);
				Append_SelectedRow(Projection, Lane.MaterialInput);
				Append_SelectedRow(Projection, Lane.MaterialTextureBinding);
				Append_SelectedRow(Projection, Lane.MaterialPolicy);
				Append_SelectedOptionalRow(Projection,
					Lane.SidecarRendererSlotDecision);
				Append_SelectedRow(Projection, Lane.SidecarTextureBinding);
				Append_SelectedRow(Projection, Lane.SidecarTextureResource);
				Append_Projection(Projection, Lane.strRuntimeAssetId);
				Append_Projection(Projection, Lane.strRawSha256);
			}
			if (Selection.Geometry.has_value())
			{
				Append_SelectedRow(Projection, Selection.Geometry->GeometryUse);
				Append_SelectedRow(Projection, Selection.Geometry->GeometryCarrier);
				Append_Projection(Projection,
					Selection.Geometry->strCandidateResourceSha256);
				Append_Projection(Projection, Selection.Geometry->strPayloadSha256);
				Append_Projection(Projection,
					Selection.Geometry->strMetadataIdentitySha256);
			}
			const auto& Values = Packet.Get_Values();
			Append_Integer(Projection,
				Values.vMeshDimensionlessScaleXzy.has_value() ? 1u : 0u);
			if (Values.vMeshDimensionlessScaleXzy.has_value())
				for (double Value : *Values.vMeshDimensionlessScaleXzy)
					Append_F64(Projection, Value);
			Append_Integer(Projection,
				Values.vSpriteSignedWorldSizeXzy.has_value() ? 1u : 0u);
			if (Values.vSpriteSignedWorldSizeXzy.has_value())
				for (double Value : *Values.vSpriteSignedWorldSizeXzy)
					Append_F64(Projection, Value);
			for (double Value : Values.vLocalPosition)
				Append_F64(Projection, Value);
			for (double Value : Values.vVelocityPerSecond)
				Append_F64(Projection, Value);
			for (double Value : Values.vAccelerationPerSecondSquared)
				Append_F64(Projection, Value);
			for (double Value : Values.vColor)
				Append_F64(Projection, Value);
			for (double Value : Values.vDynamicParameter)
				Append_F64(Projection, Value);
			Append_F64(Projection, Values.fRotationDegrees);
			Append_F64(Projection, Values.fRotationRateDegreesPerSecond);
		}
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)

	std::string Compute_StateProjection(
		const EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE& State)
	{
		std::string Projection;
		Append_Projection(Projection,
			State.Get_Plan()->Get_Identity().strSemanticProjectionSha256);
		Append_Integer(Projection, State.Get_FixedStepIndex());
		Append_F64(Projection, State.Get_SampleTimeSeconds());
		for (const auto& Emitter : State.Get_Emitters())
		{
			Append_Projection(Projection, Emitter.strEmitterId);
			Append_Projection(Projection, Emitter.strScheduleId);
			Append_Integer(Projection, static_cast<uint32_t>(Emitter.ePhase));
			Append_Integer(Projection, Emitter.iLoopIndex);
			Append_Integer(Projection, Emitter.iRandomState);
			Append_Integer(Projection, Emitter.iLifetimeRandomState);
			Append_Integer(Projection, Emitter.iNextSpawnSerial);
			Append_Integer(Projection, Emitter.iSpawnedTotal);
			Append_Integer(Projection, Emitter.iDroppedByCap);
			Append_Integer(Projection, Emitter.iActiveCount);
		}
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}

	std::string Compute_FrameProjection(
		const EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME& Frame)
	{
		std::string Projection;
		Append_Projection(Projection,
			Frame.Get_Plan()->Get_Identity().strSemanticProjectionSha256);
		Append_Integer(Projection, Frame.Get_FixedStepIndex());
		Append_F64(Projection, Frame.Get_SampleTimeSeconds());
		for (const auto& Packet : Frame.Get_ActiveOccurrences())
		{
			Append_Projection(Projection, Packet.strOccurrenceId);
			Append_Projection(Projection, Packet.strScheduleId);
			Append_Projection(Projection, Packet.strEmitterId);
			Append_Integer(Projection, static_cast<uint32_t>(Packet.eRenderer));
			Append_Integer(Projection, Packet.iLoopIndex);
			Append_Integer(Projection, Packet.iSpawnSerial);
			Append_Integer(Projection, Packet.iSpawnStep);
			Append_Integer(Projection, Packet.iOccurrenceRandomValue);
			Append_Integer(Projection, Packet.iLifetimeRandomValue);
			Append_F64(Projection, Packet.fAgeSeconds);
			Append_F64(Projection, Packet.fLifetimeSeconds);
		}
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Projection);
	}
#endif
}

bool_t Client::CEffectReconstructedExecutionPlanCompiler::Compile_Preparation(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>& InOutPlan,
	std::string& strOutError)
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> Entry =
		nullptr == pPreparation ? nullptr : pPreparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		nullptr == pPreparation ? nullptr : pPreparation->Get_Program();
	if (nullptr == Entry || nullptr == Program ||
		Entry->Get_Program().get() != Program.get() ||
		pPreparation->Get_AnchorRequests().size() != 5u)
	{
		return Reject(strOutError,
			"Reconstructed execution plan requires one valid Catalog Preparation.");
	}
	COMPILED_PLAN_DATA Data;
	if (!Compile_Program(Entry->Get_Identity(), *Program, Data, strOutError))
		return false;
	std::shared_ptr<EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Staged(
		new EFFECT_RECONSTRUCTED_EXECUTION_PLAN());
	Staged->m_Identity = std::move(Data.Identity);
	Staged->m_Summary = std::move(Data.Summary);
	Staged->m_SeededLifetimeAuthority =
		std::move(Data.SeededLifetimeAuthority);
	Staged->m_ScheduleOrder = std::move(Data.ScheduleOrder);
	Staged->m_EmitterOrder = std::move(Data.EmitterOrder);
	Staged->m_ModuleOrder = std::move(Data.ModuleOrder);
	Staged->m_DistributionOrder = std::move(Data.DistributionOrder);
	Staged->m_SeedPolicyOrder = std::move(Data.SeedPolicyOrder);
	Staged->m_Schedules = std::move(Data.Schedules);
	Staged->m_Emitters = std::move(Data.Emitters);
	Staged->m_Modules = std::move(Data.Modules);
	Staged->m_Distributions = std::move(Data.Distributions);
	Staged->m_SeedPolicies = std::move(Data.SeedPolicies);
	Staged->m_pPreparation = std::move(pPreparation);
	Staged->m_pProgram = Program;
	InOutPlan = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedSourceRuntimeFactory::Build_Document(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError,
	const EFFECT_RECONSTRUCTED_VISUAL_SCOPE eVisualScope)
{
	const auto Fail = [&strOutError](const std::string& strReason) -> bool_t
	{
		strOutError = "Reconstructed source runtime document rejected: " +
			strReason;
		return false;
	};
	CEffectReconstructedRuntimeBoundary Boundary;
	if (!Boundary.Stage(pPreparation, EFFECT_RECONSTRUCTED_RUNTIME_SEAM::OBJECT,
		strOutError))
	{
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		Boundary.Get_Program();
	if (nullptr == Program ||
		(eVisualScope != EFFECT_RECONSTRUCTED_VISUAL_SCOPE::ADMITTED_ONLY &&
		 eVisualScope != EFFECT_RECONSTRUCTED_VISUAL_SCOPE::V3_MAIN_REVIEW &&
		 eVisualScope != EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CONDITIONAL_REVIEW &&
		 eVisualScope != EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS &&
		 eVisualScope != EFFECT_RECONSTRUCTED_VISUAL_SCOPE::ALL_DIAGNOSTIC) ||
		Program->Admission.bRuntimeExecution ||
		Program->Admission.bProduct || Program->Emitters.size() != 35u ||
		Program->ActionSchedules.size() != 7u)
	{
		return Fail("the immutable nonProduct Program identity is invalid.");
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> ExecutionPlan;
	if (!CEffectReconstructedExecutionPlanCompiler::Compile_Preparation(
		pPreparation, ExecutionPlan, strOutError) || nullptr == ExecutionPlan ||
		ExecutionPlan->Get_Program().get() != Program.get())
	{
		if (strOutError.empty())
			strOutError = "Reconstructed source runtime execution plan is invalid.";
		return false;
	}

	const EFFECT_RUNTIME_PROGRAM_INPUT_ARTIFACT* SourceCandidate =
		Find_UniqueProgramRowById(Program->InputArtifacts, "sourceCandidate");
	if (nullptr == SourceCandidate || SourceCandidate->strSchema !=
			"lostark.effect-authoring" ||
		SourceCandidate->strVersionField != "version" ||
		SourceCandidate->iVersionValue !=
			EFFECT_SOURCE_CONTRACT_FORMAT_VERSION ||
		SourceCandidate->strHashDomain != "GIT_OBJECT_CANONICAL_LF_JSON" ||
		SourceCandidate->strPath.rfind("Data/", 0u) != 0u ||
		SourceCandidate->strPath.find('\\') != std::string::npos ||
		SourceCandidate->strTrackedTextSha256.size() != 64u)
	{
		return Fail("the sourceCandidate artifact contract is invalid.");
	}
	const std::filesystem::path CandidatePath = CProjectDataRoot::Resolve(
		std::filesystem::path(SourceCandidate->strPath.substr(5u)));
	std::error_code PathError;
	if (CandidatePath.empty() ||
		!std::filesystem::is_regular_file(CandidatePath, PathError) || PathError)
	{
		return Fail("the tracked native-v14 sourceCandidate file is missing.");
	}
	std::ifstream CandidateStream(CandidatePath, std::ios::binary);
	if (!CandidateStream)
		return Fail("the tracked native-v14 sourceCandidate file cannot be read.");
	const std::string CandidateBytes{
		std::istreambuf_iterator<char>(CandidateStream),
		std::istreambuf_iterator<char>() };
	if (CandidateStream.bad() || CandidateBytes.empty())
	{
		return Fail("the single-read sourceCandidate bytes are empty or invalid.");
	}
	std::string CanonicalCandidateBytes;
	CanonicalCandidateBytes.reserve(CandidateBytes.size());
	for (size_t i = 0u; i < CandidateBytes.size(); ++i)
	{
		if ('\r' != CandidateBytes[i])
		{
			CanonicalCandidateBytes.push_back(CandidateBytes[i]);
			continue;
		}
		if (i + 1u >= CandidateBytes.size() || '\n' != CandidateBytes[i + 1u])
			return Fail("sourceCandidate contains a non-canonical lone CR byte.");
		CanonicalCandidateBytes.push_back('\n');
		++i;
	}
	if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
		CanonicalCandidateBytes) != SourceCandidate->strTrackedTextSha256)
	{
		return Fail("the single-read sourceCandidate canonical-LF bytes do not "
			"match the tracked text SHA-256.");
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	if (!CEffectDocumentCodec::Parse(
		CandidateBytes, StagedDocument, strOutError))
	{
		return false;
	}
	if (!CEffectDocumentCodec::Validate_SourceContract(
		StagedDocument, strOutError))
	{
		return false;
	}
	if (StagedDocument.Elements.size() != Program->Emitters.size())
		return Fail("native-v14 and Program emitter denominators differ.");

	const auto AssignF32 = [&Fail](const double fValue, f32_t& fOut,
		const std::string_view strField) -> bool_t
	{
		if (!std::isfinite(fValue) ||
			std::abs(fValue) >
				static_cast<double>((std::numeric_limits<f32_t>::max)()))
		{
			return Fail(std::string(strField) + " is not a finite f32 value.");
		}
		fOut = static_cast<f32_t>(fValue);
		return std::isfinite(fOut) ? true :
			Fail(std::string(strField) + " did not survive f32 conversion.");
	};
	const auto AssignF2 = [&AssignF32](const std::array<double, 2u>& Value,
		float2_t& Out, const std::string_view strField) -> bool_t
	{
		return AssignF32(Value[0u], Out.x, strField) &&
			AssignF32(Value[1u], Out.y, strField);
	};
	const auto AssignF3 = [&AssignF32](const std::array<double, 3u>& Value,
		float3_t& Out, const std::string_view strField) -> bool_t
	{
		return AssignF32(Value[0u], Out.x, strField) &&
			AssignF32(Value[1u], Out.y, strField) &&
			AssignF32(Value[2u], Out.z, strField);
	};
	const auto AssignF4 = [&AssignF32](const std::array<double, 4u>& Value,
		float4_t& Out, const std::string_view strField) -> bool_t
	{
		return AssignF32(Value[0u], Out.x, strField) &&
			AssignF32(Value[1u], Out.y, strField) &&
			AssignF32(Value[2u], Out.z, strField) &&
			AssignF32(Value[3u], Out.w, strField);
	};
	const auto Lower = [](std::string Value)
	{
		std::transform(Value.begin(), Value.end(), Value.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char>(std::tolower(Character));
			});
		return Value;
	};
	const auto EqualsNoCase = [&Lower](const std::string_view Left,
		const std::string_view Right)
	{
		return Lower(std::string(Left)) == Lower(std::string(Right));
	};
	const auto ResolveParameterDistribution =
		[&Fail, &EqualsNoCase](
			const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Source,
			std::array<double, 4u>& OutValue) -> bool_t
	{
		if (Source.iComponentCount == 0u || Source.iComponentCount > 4u ||
			Source.ParamModes.size() != Source.iComponentCount ||
			Source.MinimumInput.size() != Source.iComponentCount ||
			Source.MaximumInput.size() != Source.iComponentCount ||
			Source.MinimumOutput.size() != Source.iComponentCount ||
			Source.MaximumOutput.size() != Source.iComponentCount ||
			Source.ConstantValues.size() != Source.iComponentCount)
		{
			return Fail("parameter distribution shape is not exhaustive.");
		}
		std::optional<std::vector<double>> BoundInput;
		for (const EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE& Binding :
			Source.ActionCueBindings)
		{
			if (!EqualsNoCase(Binding.strName, Source.strParameterName))
				continue;
			if (Source.iComponentCount == 1u &&
				Binding.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR &&
				Binding.fScalarValue.has_value())
			{
				BoundInput = std::vector<double>{ *Binding.fScalarValue };
				break;
			}
			if (Source.iComponentCount > 1u &&
				Binding.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR &&
				Binding.VectorValue.size() >= Source.iComponentCount)
			{
				BoundInput = std::vector<double>(Binding.VectorValue.begin(),
					Binding.VectorValue.begin() + Source.iComponentCount);
				break;
			}
			/* The frozen source contract has one intentional scalar/vector kind
			   mismatch (PointLight Size scalar -> vector StartSize).  Native
			   ParticleParameter semantics use the distribution constant in this
			   branch; never coerce the scalar into a vector. */
			continue;
		}
		OutValue.fill(0.0);
		for (uint32_t i = 0u; i < Source.iComponentCount; ++i)
		{
			if (!BoundInput.has_value())
			{
				OutValue[i] = Source.ConstantValues[i];
			}
			else if (Source.ParamModes[i] == "dpm_direct")
			{
				OutValue[i] = (*BoundInput)[i];
			}
			else if (Source.ParamModes[i] == "dpm_normal")
			{
				const double fInputRange =
					Source.MaximumInput[i] - Source.MinimumInput[i];
				const double fRatio = 0.0 == fInputRange ? 0.0 : std::clamp(
					((*BoundInput)[i] - Source.MinimumInput[i]) / fInputRange,
					0.0, 1.0);
				OutValue[i] = std::lerp(
					Source.MinimumOutput[i], Source.MaximumOutput[i], fRatio);
			}
			else
			{
				return Fail("parameter distribution mode is unsupported.");
			}
			if (!std::isfinite(OutValue[i]) ||
				std::abs(OutValue[i]) >
					static_cast<double>((std::numeric_limits<f32_t>::max)()))
			{
				return Fail("parameter distribution result is not finite f32.");
			}
		}
		return true;
	};
	const auto RendererMatches = [](const EFFECT_RUNTIME_RENDERER_KIND eRuntime,
		const EFFECT_ELEMENT_DESC& Element)
	{
		switch (eRuntime)
		{
		case EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE;
		case EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::SPRITE_PARTICLE;
		case EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			return Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::DECAL_PARTICLE;
		case EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			return Element.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::CASCADE_RIBBON;
		case EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
			return Element.eKind == EFFECT_ELEMENT_KIND::LIGHT &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::LIGHT_PARTICLE;
		case EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
			return Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST &&
				Element.Renderer.eType == EFFECT_RENDERER_TYPE::SCREEN_POST;
		default:
			return false;
		}
	};
	const auto ResolveSourceSubUVProjection = [&Fail, &Lower](
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_ELEMENT_DESC& Element,
		std::string& strOutMode,
		bool_t& bOutRandom2x2,
		bool_t& bOutLinearBlend6x6) -> bool_t
	{
		strOutMode = "none";
		bOutRandom2x2 = false;
		bOutLinearBlend6x6 = false;
		const EFFECT_SOURCE_MODULE_DESC* Required = nullptr;
		const EFFECT_SOURCE_MODULE_DESC* SubUV = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			const std::string strClass = Lower(Module.strClassName);
			if (strClass == "particlemodulerequired")
			{
				if (nullptr != Required)
					return Fail("a source emitter has duplicate Required modules.");
				Required = &Module;
			}
			else if (strClass == "particlemodulesubuv")
			{
				if (nullptr != SubUV)
					return Fail("a source emitter has duplicate ParticleModuleSubUV modules.");
				SubUV = &Module;
			}
		}
		if (nullptr == SubUV)
			return true;
		if (Emitter.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE ||
			nullptr == Required || !Element.SourceRecipe.bEnabled)
		{
			return Fail("ParticleModuleSubUV is not owned by one Sprite emitter with "
				"one Required module.");
		}

		const auto FindLiteral = [](const EFFECT_SOURCE_MODULE_DESC& Module,
			const std::string_view strPropertyPath,
			size_t& iOutCount) -> const EFFECT_SOURCE_LITERAL_DESC*
		{
			iOutCount = 0u;
			const EFFECT_SOURCE_LITERAL_DESC* Found = nullptr;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath != strPropertyPath)
					continue;
				++iOutCount;
				Found = &Literal;
			}
			return Found;
		};
		size_t iInterpolationCount = 0u;
		size_t iColumnsCount = 0u;
		size_t iRowsCount = 0u;
		size_t iRandomImageTimeCount = 0u;
		size_t iAllowFlipCount = 0u;
		size_t iSquareFlipCount = 0u;
		const EFFECT_SOURCE_LITERAL_DESC* Interpolation = FindLiteral(
			*Required, "interpolationmethod", iInterpolationCount);
		const EFFECT_SOURCE_LITERAL_DESC* Columns = FindLiteral(
			*Required, "subimages_horizontal", iColumnsCount);
		const EFFECT_SOURCE_LITERAL_DESC* Rows = FindLiteral(
			*Required, "subimages_vertical", iRowsCount);
		const EFFECT_SOURCE_LITERAL_DESC* RandomImageTime = FindLiteral(
			*Required, "randomimagetime", iRandomImageTimeCount);
		const EFFECT_SOURCE_LITERAL_DESC* AllowFlip = FindLiteral(
			*Required, "ballowimageflipping", iAllowFlipCount);
		const EFFECT_SOURCE_LITERAL_DESC* SquareFlip = FindLiteral(
			*Required, "bsquareimageflipping", iSquareFlipCount);
		if (iInterpolationCount != 1u || iColumnsCount != 1u ||
			iRowsCount != 1u || iRandomImageTimeCount != 1u ||
			iAllowFlipCount != 1u || iSquareFlipCount > 1u ||
			nullptr == Interpolation || nullptr == Columns || nullptr == Rows ||
			nullptr == RandomImageTime || nullptr == AllowFlip ||
			Interpolation->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING ||
			Columns->eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
			Rows->eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
			RandomImageTime->eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
			AllowFlip->eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN ||
			(nullptr != SquareFlip &&
			 SquareFlip->eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN) ||
			!std::isfinite(Columns->fNumber) ||
			!std::isfinite(Rows->fNumber) ||
			!std::isfinite(RandomImageTime->fNumber))
		{
			return Fail("ParticleModuleSubUV Required literals are incomplete or "
				"not exactly typed.");
		}
		if (SubUV->Distributions.size() != 1u)
			return Fail("ParticleModuleSubUV must own exactly one distribution.");
		const EFFECT_DISTRIBUTION_DESC& SubImageIndex =
			SubUV->Distributions.front();
		if (SubImageIndex.strPropertyPath != "subimageindex" ||
			SubImageIndex.iComponentCount != 1u ||
			SubImageIndex.iOperation != 1u ||
			SubImageIndex.iRandomLockAxes != 0u ||
			SubImageIndex.iLookupTableChunkSize != 1u ||
			SubImageIndex.iLookupTableNumElements != 1u ||
			!std::isfinite(SubImageIndex.fLookupTableTimeScale) ||
			!std::isfinite(SubImageIndex.fLookupTableStartTime) ||
			SubImageIndex.fLookupTableStartTime != 0.f ||
			!SubImageIndex.Keys.empty() || SubImageIndex.LookupTable.empty() ||
			!std::all_of(SubImageIndex.LookupTable.begin(),
				SubImageIndex.LookupTable.end(), [](const f32_t fValue)
				{
					return std::isfinite(fValue);
				}))
		{
			return Fail("ParticleModuleSubUV SubImageIndex distribution is "
				"incomplete or unsupported.");
		}

		const bool_t bSquareFlip = nullptr != SquareFlip &&
			SquareFlip->bBoolean;
		if (Interpolation->strString == "psuvim_random")
		{
			const std::vector<f32_t> ExpectedLookup{ 0.f, 3.f, 0.f, 3.f };
			if (Columns->fNumber != 2.0 || Rows->fNumber != 2.0 ||
				RandomImageTime->fNumber != 1.0 || !AllowFlip->bBoolean ||
				iSquareFlipCount != 0u ||
				SubImageIndex.fLookupTableTimeScale != 1.f ||
				SubImageIndex.LookupTable != ExpectedLookup)
			{
				return Fail("the Artist F random SubUV source is not the exact 2x2 "
					"0..3 recipe.");
			}
			bOutRandom2x2 = true;
			strOutMode = "psuvim_random";
		}
		else if (Interpolation->strString == "psuvim_linear_blend")
		{
			const bool_t bMonotonicSamples = SubImageIndex.LookupTable.size() == 23u &&
				std::is_sorted(SubImageIndex.LookupTable.begin() + 2u,
					SubImageIndex.LookupTable.end());
			if (Columns->fNumber != 6.0 || Rows->fNumber != 6.0 ||
				RandomImageTime->fNumber != 1.0 || !AllowFlip->bBoolean ||
				iSquareFlipCount != 1u || !bSquareFlip ||
				SubImageIndex.fLookupTableTimeScale != 20.f ||
				!bMonotonicSamples || SubImageIndex.LookupTable[0u] != 10.f ||
				SubImageIndex.LookupTable[1u] != 35.f ||
				SubImageIndex.LookupTable[2u] != 10.f ||
				SubImageIndex.LookupTable.back() != 35.f)
			{
				return Fail("the Artist F linear-blend SubUV source is not the exact "
					"6x6 10..35 recipe.");
			}
			bOutLinearBlend6x6 = true;
			strOutMode = "psuvim_linear_blend_random_flip_square";
		}
		else
		{
			return Fail("ParticleModuleSubUV interpolation method is unsupported.");
		}

		return true;
	};

	std::unordered_map<std::string, EFFECT_ELEMENT_DESC*> ElementsById;
	ElementsById.reserve(StagedDocument.Elements.size());
	for (EFFECT_ELEMENT_DESC& Element : StagedDocument.Elements)
	{
		if (!ElementsById.emplace(Element.strElementId, &Element).second)
			return Fail("native-v14 contains a duplicate Element identity.");
	}
	std::unordered_map<std::string, std::string> ElementIdsByEmitterId;
	ElementIdsByEmitterId.reserve(Program->Emitters.size());
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program->Emitters)
	{
		if (!ElementIdsByEmitterId.emplace(
			Emitter.Row.strId, Emitter.strSourceElementId).second)
		{
			return Fail("Program contains a duplicate emitter identity.");
		}
	}

	std::unordered_set<std::string> CoveredElements;
	std::unordered_set<std::string> CoveredOccurrences;
	std::unordered_set<std::string> CoveredTextureResources;
	std::unordered_set<std::string> CoveredGeometryUses;
	uint32_t iOneLayerRuntimeTextureProjectionCount = 0u;
	uint32_t iParameterDistributionOverlayCount = 0u;
	uint32_t iCurveDistributionOverlayCount = 0u;
	uint32_t iClearedDistributionReferenceCount = 0u;
	uint32_t iSpriteEmitterCount = 0u;
	uint32_t iSpriteIdentitySubUVCount = 0u;
	uint32_t iSourceSubUVProjectionCount = 0u;
	uint32_t iRandom2x2SubUVCount = 0u;
	uint32_t iLinearBlend6x6SubUVCount = 0u;
	uint32_t iMaterialSubUVProjectionCount = 0u;
	uint32_t iMainCompositeMaterialProjectionCount = 0u;
	uint32_t iMissileTrailFiniteProfileProjectionCount = 0u;
	uint32_t iReconstructedEvaluatorMaterialProjectionCount = 0u;
	uint32_t iRuntimeMaterialV2ProfileProjectionCount = 0u;
	uint32_t iTypedRibbonGeometryProjectionCount = 0u;
	uint32_t iTypedLightParticleInputProjectionCount = 0u;
	uint32_t iTypedRendererProjectionCount = 0u;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program->Emitters)
	{
		const auto ElementIterator = ElementsById.find(Emitter.strSourceElementId);
		const EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE* Schedule =
			Find_UniqueProgramRowById(
				Program->ActionSchedules, Emitter.strScheduleId);
		if (ElementIterator == ElementsById.end() || nullptr == Schedule ||
			!CoveredElements.emplace(Emitter.strSourceElementId).second)
		{
			return Fail("one Program emitter cannot be joined to exactly one "
				"Element and schedule.");
		}
		EFFECT_ELEMENT_DESC& Element = *ElementIterator->second;
		if (Element.strSourceNode != Emitter.strSourceNode ||
			!RendererMatches(Emitter.eRenderer, Element) ||
			Element.SourceRecipe.CompilerEvidence.strSourceOccurrenceId !=
				Emitter.strSourceOccurrenceId ||
			Element.SourceRecipe.CompilerEvidence.strSourceSystemId !=
				Emitter.strSourceSystemId ||
			Element.SourceRecipe.CompilerEvidence.strSourceEmitterPath !=
				Emitter.strSourceEmitterPath ||
			Schedule->strSourceOccurrenceId != Emitter.strSourceOccurrenceId ||
			Schedule->strSourceSystemId != Emitter.strSourceSystemId)
		{
			return Fail("native-v14 Element provenance differs from its "
				"Program emitter/schedule join.");
		}
		const EFFECT_SOURCE_SPACE eExpectedSourceSpace =
			Emitter.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST ?
				EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
				EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		const std::string_view strExpectedSourceSpace =
			Emitter.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST ?
				"screenSpaceV1" : "ue3CascadeV1";
		if (Emitter.strRendererSourceSpace != strExpectedSourceSpace ||
			Element.Renderer.eSourceSpace != eExpectedSourceSpace)
			return Fail("native-v14 renderer source space differs from Program.");

		/* The immutable Program explicitly remains non-Product and has zero sealed
		   human approvals.  A versioned shader route alone is not restoration:
		   this conditional set also includes the typed finite/special families.
		   Complete therefore stays empty, while V3 main review narrows the audition
		   to the short #9/#10 core and #11 outer carrier stack. */
		const bool_t bConditionalReview =
			Emitter.Row.iOrder == 3u || Emitter.Row.iOrder == 4u ||
			Emitter.Row.iOrder == 9u || Emitter.Row.iOrder == 10u ||
			Emitter.Row.iOrder == 11u || Emitter.Row.iOrder == 16u ||
			Emitter.Row.iOrder == 17u ||
			Emitter.Row.iOrder == 22u || Emitter.Row.iOrder == 23u ||
			Emitter.Row.iOrder == 30u || Emitter.Row.iOrder == 31u ||
			Emitter.Row.iOrder == 32u || Emitter.Row.iOrder == 34u;
		const bool_t bV3MainReview = Emitter.Row.iOrder == 9u ||
			Emitter.Row.iOrder == 10u || Emitter.Row.iOrder == 11u;
		const bool_t bCoreRenderer = [&Emitter]()
		{
			switch (Emitter.eRenderer)
			{
			case EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			case EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			case EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			case EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
				return true;
			case EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
			case EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
			default:
				return false;
			}
		}();
		Element.bVisible = Emitter.bVisible &&
			(eVisualScope == EFFECT_RECONSTRUCTED_VISUAL_SCOPE::ALL_DIAGNOSTIC ||
			 (eVisualScope == EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS &&
			  bCoreRenderer) ||
			 (eVisualScope ==
				 EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CONDITIONAL_REVIEW &&
			  bConditionalReview) ||
			 (eVisualScope == EFFECT_RECONSTRUCTED_VISUAL_SCOPE::V3_MAIN_REVIEW &&
			  bV3MainReview));
		if (!AssignF32(Schedule->fGlobalTimeSeconds,
			Element.Detail.Timing.fStartDelaySeconds, "schedule start") ||
			(Schedule->fDurationSeconds > 0.0 &&
			 !AssignF32(Schedule->fDurationSeconds,
				 Element.Detail.Timing.fLifeTimeSeconds, "schedule duration")) ||
			!AssignF32(Emitter.Timing.fEmitterDelaySeconds,
				Element.SourceRecipe.fEmitterDelaySeconds, "emitter delay") ||
			!AssignF32(Emitter.Timing.fEmitterDurationSeconds,
				Element.SourceRecipe.fEmitterDurationSeconds, "emitter duration"))
		{
			return false;
		}
		Element.SourceRecipe.bEnabled = Emitter.bSourceRecipeEnabled;
		Element.SourceRecipe.iSourcePeakActiveParticles =
			Emitter.iSourcePeakActiveParticles;
		Element.SourceRecipe.iEmitterLoopCount = Emitter.Timing.iEmitterLoopCount;
		Element.SourceRecipe.Bursts.clear();
		Element.SourceRecipe.Bursts.reserve(Emitter.Timing.Bursts.size());
		for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Emitter.Timing.Bursts)
		{
			EFFECT_PARTICLE_BURST_DESC StagedBurst;
			if (Burst.iCountMinimum > Burst.iCountMaximum ||
				!AssignF32(Burst.fTimeSeconds, StagedBurst.fTimeSeconds,
					"emitter burst time"))
			{
				return false;
			}
			StagedBurst.iCountMinimum = Burst.iCountMinimum;
			StagedBurst.iCountMaximum = Burst.iCountMaximum;
			Element.SourceRecipe.Bursts.push_back(std::move(StagedBurst));
		}
		if (Emitter.iOperationalMaxParticles > 0u)
			Element.Detail.Particle.iMaxParticles =
				Emitter.iOperationalMaxParticles;
		if (0u == Emitter.Random.iEmitterRandomSeed)
			return Fail("Program emitter random seed is zero.");
		uint32_t iStableElementHash = 2166136261u;
		for (const unsigned char Character : Element.strElementId)
		{
			iStableElementHash ^= Character;
			iStableElementHash *= 16777619u;
		}
		if (0u == iStableElementHash)
			iStableElementHash = 1u;
		Element.Detail.Particle.iRandomSeed =
			iStableElementHash ^ Emitter.Random.iEmitterRandomSeed;
		if (0u == Element.Detail.Particle.iRandomSeed)
			return Fail("the Playback seed inverse produced an invalid zero seed.");
		Element.Detail.Particle.bLocalSpace = Emitter.bLocalSpace;

		const auto PlanEmitterIterator =
			ExecutionPlan->Get_Emitters().find(Emitter.Row.strId);
		if (PlanEmitterIterator == ExecutionPlan->Get_Emitters().end() ||
			PlanEmitterIterator->second.ModuleIds.size() !=
				Element.SourceRecipe.Modules.size())
		{
			return Fail("source recipe module order differs from the compiled "
				"execution plan.");
		}
		for (size_t iModule = 0u;
			iModule < Element.SourceRecipe.Modules.size(); ++iModule)
		{
			EFFECT_SOURCE_MODULE_DESC& SourceModule =
				Element.SourceRecipe.Modules[iModule];
			const auto PlanModuleIterator = ExecutionPlan->Get_Modules().find(
				PlanEmitterIterator->second.ModuleIds[iModule]);
			if (PlanModuleIterator == ExecutionPlan->Get_Modules().end() ||
				PlanModuleIterator->second.strExactSourceClass !=
					SourceModule.strClassName ||
				PlanModuleIterator->second.DistributionIds.size() !=
					SourceModule.Distributions.size())
			{
				return Fail("source recipe module/distribution membership differs "
					"from the compiled execution plan.");
			}
			std::unordered_map<std::string,
				const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION*>
				PlanDistributionsByPropertyPath;
			PlanDistributionsByPropertyPath.reserve(
				PlanModuleIterator->second.DistributionIds.size());
			for (const std::string& DistributionId :
				PlanModuleIterator->second.DistributionIds)
			{
				const auto DistributionIterator =
					ExecutionPlan->Get_Distributions().find(DistributionId);
				const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION* ProgramDistribution =
					Find_UniqueProgramRowById(
						Program->Distributions, DistributionId);
				if (DistributionIterator ==
						ExecutionPlan->Get_Distributions().end() ||
					nullptr == ProgramDistribution ||
					DistributionIterator->second.strPropertyId.empty() ||
					ProgramDistribution->strPropertyPath.empty() ||
					!PlanDistributionsByPropertyPath.emplace(
						ProgramDistribution->strPropertyPath,
						&DistributionIterator->second).second)
				{
					return Fail("compiled execution plan module has a missing or "
						"duplicate distribution property path.");
				}
			}
			std::unordered_set<std::string> CoveredDistributionPropertyPaths;
			for (size_t iDistribution = 0u;
				iDistribution < SourceModule.Distributions.size(); ++iDistribution)
			{
				EFFECT_DISTRIBUTION_DESC& Destination =
					SourceModule.Distributions[iDistribution];
				const auto PlanDistributionIterator =
					PlanDistributionsByPropertyPath.find(Destination.strPropertyPath);
				if (PlanDistributionIterator ==
						PlanDistributionsByPropertyPath.end() ||
					!CoveredDistributionPropertyPaths.emplace(
						Destination.strPropertyPath).second)
				{
					return Fail("source distribution property path is missing or "
						"duplicated in the compiled execution plan module.");
				}
				const EFFECT_RECONSTRUCTED_EXECUTION_DISTRIBUTION& Source =
					*PlanDistributionIterator->second;
				const bool_t bParameterBacked =
					Source.eVariant ==
						EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ||
					Source.eVariant ==
						EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER ||
					Source.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
				if (bParameterBacked)
				{
					std::array<double, 4u> Resolved{};
					if (!ResolveParameterDistribution(Source, Resolved))
						return false;
					float4_t Value{};
					for (uint32_t i = 0u; i < Source.iComponentCount; ++i)
						(&Value.x)[i] = static_cast<f32_t>(Resolved[i]);
					Destination.iOperation = 1u;
					Destination.iRandomLockAxes = 0u;
					Destination.iLookupTableChunkSize = 0u;
					Destination.iLookupTableNumElements = 0u;
					Destination.fLookupTableTimeScale = 0.f;
					Destination.fLookupTableStartTime = 0.f;
					Destination.vDefaultMinimum = Value;
					Destination.vDefaultMaximum = Value;
					Destination.LookupTable.clear();
					Destination.Keys.clear();
					++iParameterDistributionOverlayCount;
				}
				else if (Source.eVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE)
				{
					if (Source.CurveKeys.empty())
						return Fail("the referenced float curve has no typed keys.");
					Destination.Keys.clear();
					Destination.Keys.reserve(Source.CurveKeys.size());
					for (const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY& Key :
						Source.CurveKeys)
					{
						EFFECT_DISTRIBUTION_KEY_DESC StagedKey;
						if (!AssignF32(Key.fTime, StagedKey.fTime,
								"distribution curve time") ||
							!AssignF4(Key.vMinimum, StagedKey.vMinimum,
								"distribution curve minimum") ||
							!AssignF4(Key.vMaximum, StagedKey.vMaximum,
								"distribution curve maximum") ||
							!AssignF4(Key.vArriveTangentMinimum,
								StagedKey.vArriveTangentMinimum,
								"distribution curve arrive minimum") ||
							!AssignF4(Key.vLeaveTangentMinimum,
								StagedKey.vLeaveTangentMinimum,
								"distribution curve leave minimum") ||
							!AssignF4(Key.vArriveTangentMaximum,
								StagedKey.vArriveTangentMaximum,
								"distribution curve arrive maximum") ||
							!AssignF4(Key.vLeaveTangentMaximum,
								StagedKey.vLeaveTangentMaximum,
								"distribution curve leave maximum"))
						{
							return false;
						}
						StagedKey.eInterpolation =
							Key.eInterpolation ==
								EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::CUBIC ?
								EFFECT_DISTRIBUTION_INTERPOLATION::CUBIC :
								EFFECT_DISTRIBUTION_INTERPOLATION::LINEAR;
						Destination.Keys.push_back(std::move(StagedKey));
					}
					++iCurveDistributionOverlayCount;
				}
				if (!Destination.strReferenceId.empty())
				{
					if (!bParameterBacked && Source.eVariant !=
						EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE)
					{
						return Fail("a referenced distribution lacks a typed Program "
							"overlay.");
					}
					Destination.strReferenceId.clear();
					Destination.strOccurrenceId.clear();
					Destination.strPayloadStatus.clear();
					Destination.strFidelity.clear();
					Destination.ExecutionAdmission = {};
					Destination.eParameterBinding =
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
					Destination.strParameterName.clear();
					++iClearedDistributionReferenceCount;
				}
			}
			if (CoveredDistributionPropertyPaths.size() !=
				PlanDistributionsByPropertyPath.size())
			{
				return Fail("source distribution property membership is not "
					"exhaustive for the compiled execution plan module.");
			}
		}
		std::string strProjectedSubUVMode;
		bool_t bRandom2x2SubUV = false;
		bool_t bLinearBlend6x6SubUV = false;
		if (!ResolveSourceSubUVProjection(Emitter, Element,
			strProjectedSubUVMode, bRandom2x2SubUV,
			bLinearBlend6x6SubUV))
		{
			return false;
		}
		if (Emitter.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE)
		{
			++iSpriteEmitterCount;
			if (!bRandom2x2SubUV && !bLinearBlend6x6SubUV)
				++iSpriteIdentitySubUVCount;
		}
		if (bRandom2x2SubUV || bLinearBlend6x6SubUV)
			++iSourceSubUVProjectionCount;
		if (bRandom2x2SubUV)
			++iRandom2x2SubUVCount;
		if (bLinearBlend6x6SubUV)
			++iLinearBlend6x6SubUVCount;
		Element.SourceRecipe.LocalReferenceBindings.clear();
		Element.SourceRecipe.CompilerEvidence.ParameterOverrides.clear();

		const EFFECT_RUNTIME_PROGRAM_RENDERER_CONFIG& Renderer =
			Emitter.RendererRuntimeConfig;
		if (Renderer.Color.has_value())
		{
			const auto& Source = *Renderer.Color;
			if (!AssignF4(Source.vOffset, Element.Detail.Color.vColorOffset,
					"renderer color offset") ||
				!AssignF4(Source.vMultiply, Element.Detail.Color.vColorMultiply,
					"renderer color multiply") ||
				!AssignF32(Source.fClip, Element.Detail.Color.fColorClip,
					"renderer color clip") ||
				!AssignF32(Source.fEmissiveIntensity,
					Element.Detail.Color.fEmissiveIntensity,
					"renderer emissive intensity") ||
				!AssignF32(Source.fDistortionIntensity,
					Element.Detail.Color.fDistortionIntensity,
					"renderer distortion intensity") ||
				!AssignF32(Source.fRadialTime,
					Element.Detail.Color.fRadialTime, "renderer radial time") ||
				!AssignF32(Source.fRadialIntensity,
					Element.Detail.Color.fRadialIntensity,
					"renderer radial intensity"))
			{
				return false;
			}
			Element.Detail.Color.bDistortionOnBaseMaterial =
				Source.bDistortionOnBaseMaterial;
		}
		if (Renderer.Uv.has_value())
		{
			const auto& Source = *Renderer.Uv;
			if (Source.iTileColumns == 0u || Source.iTileRows == 0u ||
				Source.iTileColumns >
					static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()) ||
				Source.iTileRows >
					static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()) ||
				Source.iTileIndex >
					static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()) ||
				!AssignF2(Source.vStart, Element.Detail.UV.vStart,
					"renderer UV start") ||
				!AssignF2(Source.vSpeed, Element.Detail.UV.vSpeed,
					"renderer UV speed") ||
				!AssignF2(Source.vWaveAmplitude,
					Element.Detail.UV.vWaveAmplitude,
					"renderer UV wave amplitude") ||
				!AssignF32(Source.fWaveFrequency,
					Element.Detail.UV.fWaveFrequency,
					"renderer UV wave frequency") ||
				!AssignF32(Source.fSequenceTerm,
					Element.Detail.UV.fSequenceTerm,
					"renderer UV sequence term"))
			{
				return false;
			}
			Element.Detail.UV.bWave = Source.bWave;
			Element.Detail.UV.bSequence = Source.bSequence;
			Element.Detail.UV.bLoop = Source.bLoop;
			Element.Detail.UV.iTileColumns =
				static_cast<int32_t>(Source.iTileColumns);
			Element.Detail.UV.iTileRows =
				static_cast<int32_t>(Source.iTileRows);
			Element.Detail.UV.iTileIndex =
				static_cast<int32_t>(Source.iTileIndex);
		}
		if (Renderer.LinearLerp.has_value())
		{
			const auto& Source = *Renderer.LinearLerp;
			if (!AssignF3(Source.vEndPosition,
					Element.Detail.LinearLerp.vEndPosition,
					"renderer end position") ||
				!AssignF3(Source.vEndRotationDegrees,
					Element.Detail.LinearLerp.vEndRotationDegrees,
					"renderer end rotation") ||
				!AssignF3(Source.vEndRevolutionDegreesPerSecond,
					Element.Detail.LinearLerp.vEndRevolutionDegreesPerSecond,
					"renderer end revolution") ||
				!AssignF3(Source.vEndScale,
					Element.Detail.LinearLerp.vEndScale,
					"renderer end scale") ||
				!AssignF3(Source.vEndVelocityPerSecond,
					Element.Detail.LinearLerp.vEndVelocityPerSecond,
					"renderer end velocity") ||
				!AssignF4(Source.vEndColorOffset,
					Element.Detail.LinearLerp.vEndColorOffset,
					"renderer end color offset") ||
				!AssignF4(Source.vEndColorMultiply,
					Element.Detail.LinearLerp.vEndColorMultiply,
					"renderer end color multiply") ||
				!AssignF32(Source.fEndEmissiveIntensity,
					Element.Detail.LinearLerp.fEndEmissiveIntensity,
					"renderer end emissive"))
			{
				return false;
			}
			Element.Detail.LinearLerp.bPosition = Source.bPosition;
			Element.Detail.LinearLerp.bRotation = Source.bRotation;
			Element.Detail.LinearLerp.bRevolution = Source.bRevolution;
			Element.Detail.LinearLerp.bScale = Source.bScale;
			Element.Detail.LinearLerp.bVelocity = Source.bVelocity;
			Element.Detail.LinearLerp.bColorOffset = Source.bColorOffset;
			Element.Detail.LinearLerp.bColorMultiply = Source.bColorMultiply;
			Element.Detail.LinearLerp.bEmissiveIntensity =
				Source.bEmissiveIntensity;
		}
		if (Renderer.Mesh.has_value())
		{
			Element.Detail.Mesh.bUseModelMaterial =
				Renderer.Mesh->bUseModelMaterial;
			if (!AssignF3(Renderer.Mesh->vSourceTypeDataRotationDegrees,
				Element.Detail.Mesh.vSourceTypeDataRotationDegrees,
				"renderer source TypeDataMesh rotation"))
			{
				return false;
			}
		}
		if (Renderer.Sprite.has_value())
		{
			Element.Detail.Sprite.bBillboard = Renderer.Sprite->bBillboard;
			if (!AssignF32(Renderer.Sprite->fBillboardRollDegrees,
				Element.Detail.Sprite.fBillboardRollDegrees,
				"renderer billboard roll"))
			{
				return false;
			}
		}
		if (Renderer.Decal.has_value() &&
			(!AssignF2(Renderer.Decal->vSize, Element.Detail.Decal.vSize,
				"renderer decal size") ||
			 !AssignF32(Renderer.Decal->fDepth, Element.Detail.Decal.fDepth,
				 "renderer decal depth")))
		{
			return false;
		}
		if (Renderer.Trail.has_value())
		{
			const auto& Source = *Renderer.Trail;
			Element.Detail.Trail.iMaxPoints = Source.iMaxPoints;
			Element.Detail.Trail.bFaceCamera = Source.bFaceCamera;
			if (!AssignF32(Source.fPointLifeTimeSeconds,
					Element.Detail.Trail.fPointLifeTimeSeconds,
					"renderer trail lifetime") ||
				!AssignF32(Source.fSampleIntervalSeconds,
					Element.Detail.Trail.fSampleIntervalSeconds,
					"renderer trail sample interval") ||
				!AssignF32(Source.fMinimumDistance,
					Element.Detail.Trail.fMinimumDistance,
					"renderer trail minimum distance") ||
				!AssignF32(Source.fStartWidth,
					Element.Detail.Trail.fStartWidth,
					"renderer trail start width") ||
				!AssignF32(Source.fEndWidth,
					Element.Detail.Trail.fEndWidth,
					"renderer trail end width"))
			{
				return false;
			}
		}
		if (Renderer.AfterImage.has_value())
		{
			const auto& Source = *Renderer.AfterImage;
			Element.Detail.AfterImage.iMaxCopies = Source.iMaxCopies;
			if (!AssignF32(Source.fSampleIntervalSeconds,
					Element.Detail.AfterImage.fSampleIntervalSeconds,
					"renderer after-image interval") ||
				!AssignF32(Source.fAlphaExponent,
					Element.Detail.AfterImage.fAlphaExponent,
					"renderer after-image alpha exponent"))
			{
				return false;
			}
		}

		const std::array<double, 3u> ZeroTransform{ 0.0, 0.0, 0.0 };
		const std::array<double, 3u> UnitTransform{ 1.0, 1.0, 1.0 };
		if (Emitter.DetailTransform.vPosition != ZeroTransform ||
			Emitter.DetailTransform.vRotationDegrees != ZeroTransform ||
			Emitter.DetailTransform.vRevolutionDegreesPerSecond != ZeroTransform ||
			Emitter.DetailTransform.vScale != UnitTransform ||
			Emitter.DetailTransform.vVelocityPerSecond != ZeroTransform)
		{
			return Fail("the frozen emitter element transform is not identity; "
				"the format-13 cue projection would require matrix decomposition.");
		}
		if (!AssignF3(Emitter.CueLocalTransform.vSourcePositionUeUnits,
				Element.SourceRecipe.CompilerEvidence.vCueSourcePositionUeUnits,
				"cue source position") ||
			!AssignF3(Emitter.CueLocalTransform.vPosition,
				Element.SourceRecipe.CompilerEvidence.CueLocalTransform.vPosition,
				"cue local position") ||
			!AssignF3(Emitter.CueLocalTransform.vRotationDegrees,
				Element.SourceRecipe.CompilerEvidence.CueLocalTransform.
					vRotationDegrees, "cue local rotation") ||
			!AssignF3(Emitter.CueLocalTransform.vScale,
				Element.SourceRecipe.CompilerEvidence.CueLocalTransform.vScale,
				"cue local scale") ||
			!AssignF3(Emitter.CueLocalTransform.vPosition,
				Element.Detail.Transform.vPosition, "runtime cue position") ||
			!AssignF3(Emitter.CueLocalTransform.vRotationDegrees,
				Element.Detail.Transform.vRotationDegrees, "runtime cue rotation") ||
			!AssignF3(Emitter.DetailTransform.vRevolutionDegreesPerSecond,
				Element.Detail.Transform.vRevolutionDegreesPerSecond,
				"detail revolution") ||
			!AssignF3(Emitter.CueLocalTransform.vScale,
				Element.Detail.Transform.vScale, "runtime cue scale") ||
			!AssignF3(Emitter.DetailTransform.vVelocityPerSecond,
				Element.Detail.Transform.vVelocityPerSecond, "detail velocity"))
		{
			return false;
		}

		Element.ActionCueAttachment.bEnabled =
			Emitter.ActionCueAttachment.bEnabled;
		Element.ActionCueAttachment.bFollow =
			Emitter.ActionCueAttachment.bFollow;
		Element.ActionCueAttachment.strSourceAnchorSlotId =
			Emitter.ActionCueAttachment.strSourceAnchorSlotId;
		Element.ActionCueAttachment.strRuntimeAnchorSlotId =
			Emitter.ActionCueAttachment.strRuntimeAnchorSlotId;
		Element.ActionCueAttachment.strRuntimeBoneName =
			Emitter.ActionCueAttachment.strRuntimeBoneName;
		if (!AssignF32(
				Emitter.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees,
				Element.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees,
				"snapshot root source basis yaw") ||
			!AssignF3(Emitter.ActionCueAttachment.SocketLocalTransform.vPosition,
				Element.ActionCueAttachment.SocketLocalTransform.vPosition,
				"attachment position") ||
			!AssignF3(
				Emitter.ActionCueAttachment.SocketLocalTransform.vRotationDegrees,
				Element.ActionCueAttachment.SocketLocalTransform.vRotationDegrees,
				"attachment rotation") ||
			!AssignF3(Emitter.ActionCueAttachment.SocketLocalTransform.vScale,
				Element.ActionCueAttachment.SocketLocalTransform.vScale,
				"attachment scale"))
		{
			return false;
		}
		Element.TransformInheritance.bEnabled =
			Emitter.TransformInheritance.bEnabled;
		Element.TransformInheritance.strMasterElementId.clear();
		if (Emitter.TransformInheritance.bEnabled)
		{
			const auto Master = ElementIdsByEmitterId.find(
				Emitter.TransformInheritance.strMasterEmitterId);
			if (Master == ElementIdsByEmitterId.end())
				return Fail("transform inheritance master emitter is missing.");
			Element.TransformInheritance.strMasterElementId = Master->second;
		}
		if (Element.SourcePresentation.bEnabled)
		{
			if (!AssignF32(Schedule->fGlobalTimeSeconds,
				Element.SourcePresentation.fSourceTimeSeconds,
				"source presentation time"))
			{
				return false;
			}
		}

		Element.ResourceBindings.clear();
		if (Emitter.strGeometryUseId.has_value())
		{
			const EFFECT_RUNTIME_PROGRAM_GEOMETRY_USE* Use =
				Find_UniqueProgramRowById(
					Program->GeometryUses, *Emitter.strGeometryUseId);
			const EFFECT_RUNTIME_PROGRAM_GEOMETRY_CARRIER* Carrier =
				nullptr == Use ? nullptr : Find_UniqueProgramRowById(
					Program->GeometryCarriers, Use->strCarrierId);
			if (nullptr == Use || nullptr == Carrier ||
				Use->strEmitterId != Emitter.Row.strId ||
				Use->strAssetId != Carrier->strAssetId ||
				!CoveredGeometryUses.emplace(Use->Row.strId).second ||
				Carrier->fGeometryPreScale <= 0.0)
			{
				return Fail("Program geometry use/carrier join is invalid.");
			}
			Element.ResourceBindings.push_back(
				{ "meshModel", Carrier->strAssetId });
			Element.SourceRecipe.GeometryBinding.bEnabled = true;
			Element.SourceRecipe.GeometryBinding.strAssetId = Carrier->strAssetId;
			if (!AssignF32(Carrier->fGeometryPreScale,
				Element.SourceRecipe.GeometryBinding.fCarrierGeometryPreScale,
				"geometry carrier preScale"))
			{
				return false;
			}
		}
		else
		{
			Element.SourceRecipe.GeometryBinding.bEnabled = false;
		}
		std::unordered_set<std::string> ResourceSlots;
		if (!Element.ResourceBindings.empty())
			ResourceSlots.emplace(Element.ResourceBindings.front().strSlotId);
		for (const std::string& strTextureResourceId :
			Emitter.TextureResourceIds)
		{
			const EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE* Resource =
				Find_UniqueProgramRowById(
					Program->RendererTextureResources, strTextureResourceId);
			if (nullptr == Resource ||
				Resource->strEmitterId != Emitter.Row.strId ||
				Resource->strSourceNode != Emitter.strSourceNode ||
				!CoveredTextureResources.emplace(Resource->Row.strId).second ||
				!ResourceSlots.emplace(Resource->strSlotId).second)
			{
				return Fail("Program renderer texture resource join is invalid.");
			}
			Element.ResourceBindings.push_back(
				{ Resource->strSlotId, Resource->strAssetId });
		}

		Element.Material.SourceMaterial = {};
		if (Emitter.strMaterialOccurrenceId.has_value())
		{
			const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE* Occurrence =
				Find_UniqueProgramRowById(Program->MaterialOccurrences,
					*Emitter.strMaterialOccurrenceId);
			const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE* Recipe =
				nullptr == Occurrence ? nullptr : Find_UniqueProgramRowById(
					Program->MaterialRecipes, Occurrence->strRecipeId);
			const EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY* Family =
				nullptr == Occurrence ? nullptr : Find_UniqueProgramRowById(
					Program->MaterialFamilies, Occurrence->strFamilyId);
			if (nullptr == Occurrence || nullptr == Recipe || nullptr == Family ||
				Occurrence->strEmitterId != Emitter.Row.strId ||
				Occurrence->eRenderer != Emitter.eRenderer ||
				Occurrence->strFamilyId != Recipe->strFamilyId ||
				!CoveredOccurrences.emplace(Occurrence->Row.strId).second)
			{
				return Fail("Program material occurrence/recipe/family join is invalid.");
			}
			Element.Material.strSourceMaterialPath = Recipe->strSourceMaterialPath;
			const bool_t bPresentationMaterial =
				Emitter.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE &&
				Emitter.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST;
			if (bPresentationMaterial)
			{
				EFFECT_SOURCE_MATERIAL_DESC& Source =
					Element.Material.SourceMaterial;
				Source.bEnabled = true;
				Source.strProfileId = Recipe->Row.strId;
				Source.strParentMaterialPath = Recipe->strSourceMaterialPath;
				Source.eStatus =
					EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE;
				Source.strSubUVMode = strProjectedSubUVMode;
				if (Source.strSubUVMode != "none")
					++iMaterialSubUVProjectionCount;
				bool_t bOneLayerDistortion = Lower(
					Recipe->strSourceMaterialPath).find("onelayerdistortion") !=
						std::string::npos;
				const bool_t bExactArtistOneLayerRecipe =
					Recipe->Row.strId == ARTIST_ONE_LAYER_RECIPE_ID;
				bool_t bTwoSided = true;
				std::string strBlendMode;
				for (const std::string& strBindingId : Recipe->RenderBindingIds)
				{
					const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Binding =
						Find_UniqueProgramRowById(
							Program->MaterialRenderBindings, strBindingId);
					if (nullptr == Binding || Binding->strRecipeId != Recipe->Row.strId)
						return Fail("Program Material render binding join is invalid.");
					if (Binding->strFieldName == "blendmode")
						strBlendMode = Lower(Binding->strStringValue);
					else if (Binding->strFieldName == "twosided" &&
						Binding->bValue.has_value())
						bTwoSided = *Binding->bValue;
					else if (Binding->strFieldName == "buseonelayerdistortion" &&
						Binding->bValue.value_or(false))
						bOneLayerDistortion = true;
				}
				if (strBlendMode.find("additive") != std::string::npos)
				{
					Element.Material.eRenderProfile = bTwoSided ?
						EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ :
						EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ;
				}
				else if (strBlendMode.find("opaque") != std::string::npos ||
					strBlendMode.find("masked") != std::string::npos)
				{
					Element.Material.eRenderProfile =
						EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
				}
				else
				{
					Element.Material.eRenderProfile = bTwoSided ?
						EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ :
						EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ;
				}

				std::unordered_set<std::string> ScalarNames;
				std::unordered_set<std::string> VectorNames;
				std::unordered_set<std::string> TextureNames;
				const EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING*
					pPreferredBaseBinding = nullptr;
				int32_t iPreferredBaseScore = (std::numeric_limits<int32_t>::max)();
				for (const std::string& strInputId : Recipe->InputIds)
				{
					const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Input =
						Find_UniqueProgramRowById(Program->MaterialInputs, strInputId);
					if (nullptr == Input || Input->strRecipeId != Recipe->Row.strId)
						return Fail("Program Material input join is invalid.");
					const std::string& strName =
						Input->strNormalizedParameterName.empty() ?
							Input->strParameterName :
							Input->strNormalizedParameterName;
					if (strName.empty())
						return Fail("Program Material input has no parameter identity.");
					if (Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64)
					{
						if (!Input->fValue.has_value() ||
							!ScalarNames.emplace(strName).second)
							continue;
						EFFECT_NAMED_FLOAT_DESC Value;
						Value.strName = strName;
						Value.strGroup = Input->strBindingRole;
						if (!AssignF32(*Input->fValue, Value.fValue,
							"Material scalar"))
						{
							return false;
						}
						Source.Scalars.push_back(std::move(Value));
					}
					else if (Input->eVariant ==
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4)
					{
						if (!VectorNames.emplace(strName).second)
							continue;
						EFFECT_NAMED_FLOAT4_DESC Value;
						Value.strName = strName;
						Value.strGroup = Input->strBindingRole;
						if (!AssignF4(Input->vValue, Value.vValue,
							"Material vector"))
						{
							return false;
						}
						Source.Vectors.push_back(std::move(Value));
					}
					else if (Input->eVariant ==
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
					{
						if (!TextureNames.emplace(strName).second)
							continue;
						const EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING* Binding =
							Find_UniqueProgramRow(Program->MaterialTextureBindings,
								[&](const EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING& Row)
								{
									return Row.strRecipeId == Recipe->Row.strId &&
										Row.strMaterialInputFieldId == Input->Row.strId;
								});
						EFFECT_NAMED_TEXTURE_DESC Value;
						Value.strName = strName;
						Value.strGroup = Input->strBindingRole;
						if (nullptr == Binding)
						{
							Value.strSourceObjectPath = Input->strStringValue;
							if (bExactArtistOneLayerRecipe &&
								Input->strStringValue == ARTIST_ONE_LAYER_EMISSIVE_SOURCE)
							{
								Value.strAssetId = ARTIST_ONE_LAYER_EMISSIVE_ASSET;
								Value.strSamplingEvidence =
									"reconstructed-cross-export-byte-identical-v1";
							}
							else if (bExactArtistOneLayerRecipe &&
								Input->strStringValue == ARTIST_ONE_LAYER_NOISE_SOURCE)
							{
								Value.strAssetId = ARTIST_ONE_LAYER_NOISE_ASSET;
								Value.strSamplingEvidence =
									"reconstructed-cross-export-byte-identical-v1";
							}
							else
							{
								Value.strSamplingEvidence = "reconstructed-unbound-v1";
							}
						}
						else
						{
							Value.strSourceObjectPath = Binding->strLogicalTexturePath;
							Value.strAssetId = Binding->strRuntimeAssetId.value_or("");
							Value.strSamplingEvidence = "reconstructed-program-v1";
							const EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY* SamplerPolicy =
								Find_UniqueProgramRowById(
									Program->MaterialPolicies,
									Binding->strSamplerPolicyRowId);
							if (nullptr == SamplerPolicy ||
								!SamplerPolicy->SamplerDescriptor.has_value())
							{
								return Fail("Program Material sampler policy is missing.");
							}
							const auto& Sampler = *SamplerPolicy->SamplerDescriptor;
							Value.eAddressU =
								Lower(Sampler.strAddressUUe3).find("clamp") !=
									std::string::npos ? EFFECT_TEXTURE_ADDRESS_MODE::CLAMP :
									EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
							Value.eAddressV =
								Lower(Sampler.strAddressVUe3).find("clamp") !=
									std::string::npos ? EFFECT_TEXTURE_ADDRESS_MODE::CLAMP :
									EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
							Value.eColorSpace = Sampler.bSrgb ?
								EFFECT_TEXTURE_COLOR_SPACE::SRGB :
								EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
							if (Binding->strRuntimeAssetId.has_value() &&
								!Binding->strRuntimeAssetId->empty())
							{
								const std::string LowerName = Lower(strName);
								const std::string LowerRole =
									Lower(Input->strBindingRole);
								const int32_t iScore =
									LowerRole == "color_texture" ||
									LowerName.find("main") != std::string::npos ||
									LowerName.find("base") != std::string::npos ||
									LowerName.find("diffuse") != std::string::npos ? 0 :
									(LowerName.find("emissive") != std::string::npos ? 1 : 2);
								if (iScore < iPreferredBaseScore)
								{
									iPreferredBaseScore = iScore;
									pPreferredBaseBinding = Binding;
								}
							}
						}
						if (Value.strSourceObjectPath.empty())
							Value.strSourceObjectPath = Input->strStringValue;
						if (Value.strSourceObjectPath.empty())
							continue;
						Source.Textures.push_back(std::move(Value));
					}
				}
				if (bExactArtistOneLayerRecipe)
				{
					constexpr std::array<std::pair<std::string_view, f32_t>, 5u>
						ExpectedScalars{
							std::pair{ "distortion_intensity", -40.f },
							std::pair{ "uv_noise_01_intensity", 0.30000001192092896f },
							std::pair{ "uv_noise_01_tiling_x", 3.f },
							std::pair{ "uv_noise_01_panning_y", 5.f },
							std::pair{ "uv_noise_01_tiling_y", 4.f }
						};
					const auto HasExactTexture = [&Source](
						const std::string_view strSource,
						const std::string_view strAsset)
					{
						return std::ranges::any_of(Source.Textures,
							[strSource, strAsset](const EFFECT_NAMED_TEXTURE_DESC& Value)
							{
								return Value.strSourceObjectPath == strSource &&
									Value.strAssetId == strAsset;
							});
					};
					bool_t bExactScalarLayout =
						Source.Scalars.size() == ExpectedScalars.size();
					for (size_t i = 0u; bExactScalarLayout &&
						i < ExpectedScalars.size(); ++i)
					{
						bExactScalarLayout =
							Source.Scalars[i].strName == ExpectedScalars[i].first &&
							Source.Scalars[i].fValue == ExpectedScalars[i].second;
					}
					if (!bOneLayerDistortion || Source.Textures.size() != 2u ||
						!bExactScalarLayout ||
						!HasExactTexture(ARTIST_ONE_LAYER_EMISSIVE_SOURCE,
							ARTIST_ONE_LAYER_EMISSIVE_ASSET) ||
						!HasExactTexture(ARTIST_ONE_LAYER_NOISE_SOURCE,
							ARTIST_ONE_LAYER_NOISE_ASSET) ||
						ResourceSlots.contains("base") ||
						ResourceSlots.contains("noise"))
					{
						return Fail("the exact Artist one-layer texture projection is invalid.");
					}
					Element.ResourceBindings.push_back(
						{ "base", std::string(ARTIST_ONE_LAYER_EMISSIVE_ASSET) });
					Element.ResourceBindings.push_back(
						{ "noise", std::string(ARTIST_ONE_LAYER_NOISE_ASSET) });
					ResourceSlots.emplace("base");
					ResourceSlots.emplace("noise");
					++iOneLayerRuntimeTextureProjectionCount;
				}
				if (!ResourceSlots.contains("base") &&
					nullptr != pPreferredBaseBinding &&
					pPreferredBaseBinding->strRuntimeAssetId.has_value())
				{
					Element.ResourceBindings.push_back({ "base",
						*pPreferredBaseBinding->strRuntimeAssetId });
					ResourceSlots.emplace("base");
				}
				std::unordered_set<std::string> SwitchNames;
				for (const std::string& strStaticId : Recipe->StaticBindingIds)
				{
					const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Binding =
						Find_UniqueProgramRowById(
							Program->MaterialStaticBindings, strStaticId);
					if (nullptr == Binding || Binding->strRecipeId != Recipe->Row.strId)
						return Fail("Program Material static binding join is invalid.");
					const std::string& strName =
						Binding->strNormalizedParameterName.empty() ?
							Binding->strParameterName :
							Binding->strNormalizedParameterName;
					if (strName.empty() || !SwitchNames.emplace(strName).second)
						continue;
					EFFECT_NAMED_BOOL_DESC Value;
					Value.strName = strName;
					Value.strGroup = Binding->strBindingOrigin;
					Value.bValue = Binding->bSelectedValue.value_or(
						Binding->bValue.value_or(
							Binding->bSourceValue.value_or(false)));
					Source.StaticSwitches.push_back(std::move(Value));
				}
				const auto FindResource = [&Element](const std::string_view strSlotId)
					-> const EFFECT_RESOURCE_BINDING_DESC*
				{
					const auto Iterator = std::find_if(
						Element.ResourceBindings.begin(),
						Element.ResourceBindings.end(),
						[strSlotId](const EFFECT_RESOURCE_BINDING_DESC& Binding)
						{
							return Binding.strSlotId == strSlotId;
						});
					return Iterator == Element.ResourceBindings.end() ?
						nullptr : &*Iterator;
				};
				const EFFECT_RESOURCE_BINDING_DESC* Base = FindResource("base");
				const bool_t bGroupedContract =
					Is_EffectGroupedTranslucentResourceContractSatisfied(
						Source,
						nullptr != Base &&
							!Is_UnsafeEffectBaseTextureAssetId(Base->strAssetId),
						nullptr != FindResource("mask"),
						nullptr != FindResource("emissive"),
						nullptr != FindResource("dissolve"));
				const bool_t bMainCompositeMaterial =
					Occurrence->Row.iOrder == 4u ||
					(Occurrence->Row.iOrder >= 7u &&
						Occurrence->Row.iOrder <= 18u);
				const bool_t bRuntimeMaterialV2 =
					Occurrence->Row.iOrder == 2u ||
					Occurrence->Row.iOrder == 3u ||
					Occurrence->Row.iOrder == 4u ||
					Occurrence->Row.iOrder == 9u ||
					Occurrence->Row.iOrder == 10u ||
					Occurrence->Row.iOrder == 11u ||
					Occurrence->Row.iOrder == 16u ||
					Occurrence->Row.iOrder == 19u ||
					Occurrence->Row.iOrder == 20u ||
					Occurrence->Row.iOrder == 21u ||
					Occurrence->Row.iOrder == 22u ||
					Occurrence->Row.iOrder == 23u ||
					Occurrence->Row.iOrder == 24u ||
					Occurrence->Row.iOrder == 27u ||
					Occurrence->Row.iOrder == 28u ||
					Occurrence->Row.iOrder == 30u ||
					Occurrence->Row.iOrder == 31u;
				const bool_t bArtistVisualV4 =
					Occurrence->Row.iOrder == 0u ||
					Occurrence->Row.iOrder == 25u ||
					Occurrence->Row.iOrder == 29u;
				if (bMainCompositeMaterial)
					++iMainCompositeMaterialProjectionCount;

				if (Occurrence->Row.iOrder == 17u)
				{
					constexpr std::array<std::pair<std::string_view, f32_t>, 16u>
						RequiredScalars{{
							{ "alpha_tex_strength", 1.f },
							{ "alpha_out_falloff", 100.f },
							{ "emissive_tex_strength", 500.f },
							{ "emissive_tex_power", 10.f },
							{ "alpha_tex_r_texcoord", 0.8999999761581421f },
							{ "alpha_tex_g_texcoord", 1.f },
							{ "uv_noise_head_velue", 0.5f },
							{ "dissolve_hardness", 5.f },
							{ "alpha_tex_positon_r", 0.f },
							{ "alpha_tex_positon_g", 0.05000000074505806f },
							{ "uvnoise_tex_01_r_texcoord", 0.30000001192092896f },
							{ "uvnoise_tex_01_g_texcoord", 1.f },
							{ "uvnoise_tex_02_r_texcoord", 1.f },
							{ "uvnoise_tex_02_g_texcoord", 1.f },
							{ "emissive_tex_backvelue", 0.029999999329447746f },
							{ "fresnelalpha_power", 2.f }
						}};
					const auto HasScalar = [&Source](
						const std::string_view strName, const f32_t fValue)
					{
						return std::ranges::count_if(Source.Scalars,
							[strName, fValue](const EFFECT_NAMED_FLOAT_DESC& Value)
							{
								return Value.strName == strName && Value.fValue == fValue;
							}) == 1;
					};
					const auto HasTexture = [&Source](
						const std::string_view strName,
						const std::string_view strAssetId)
					{
						return std::ranges::count_if(Source.Textures,
							[strName, strAssetId](const EFFECT_NAMED_TEXTURE_DESC& Value)
							{
								return Value.strName == strName &&
									Value.strAssetId == strAssetId &&
									Value.strSamplingEvidence ==
										"reconstructed-program-v1";
							}) == 1;
					};
					bool_t bExactScalars = Source.Scalars.size() == 26u;
					for (const auto& [strName, fValue] : RequiredScalars)
						bExactScalars = bExactScalars && HasScalar(strName, fValue);

					const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
					uint32_t iDynamicModuleCount = 0u;
					for (const std::string& strModuleId : Emitter.ModuleIds)
					{
						const EFFECT_RUNTIME_PROGRAM_MODULE* Module =
							Find_UniqueProgramRowById(Program->Modules, strModuleId);
						if (nullptr == Module)
							return Fail("Program Material dynamic module join is invalid.");
						if (Module->strExactSourceClass ==
							"particlemoduleparameterdynamic")
						{
							DynamicModule = Module;
							++iDynamicModuleCount;
						}
					}
					constexpr std::array<std::string_view, 4u> DynamicNames{{
						"alpha_pan", "uv_noise_velue", "uv_noise_pan",
						"alpha_dissolve"
					}};
					bool_t bExactDynamic = iDynamicModuleCount == 1u &&
						nullptr != DynamicModule &&
						DynamicModule->DistributionIds.size() == DynamicNames.size();
					for (size_t iDynamic = 0u;
						bExactDynamic && iDynamic < DynamicNames.size(); ++iDynamic)
					{
						const std::string strPath = "dynamicparams[" +
							std::to_string(iDynamic) + "].paramname";
						const EFFECT_RUNTIME_PROGRAM_LITERAL* Literal =
							Find_UniqueProgramRow(Program->Literals,
								[&](const EFFECT_RUNTIME_PROGRAM_LITERAL& Row)
								{
									return Row.strModuleId == DynamicModule->Row.strId &&
										Row.strPropertyPath == strPath;
								});
						bExactDynamic = nullptr != Literal &&
							Literal->eVariant ==
								EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
							Literal->strEnumValue == DynamicNames[iDynamic];
					}

					const EFFECT_RESOURCE_BINDING_DESC* Mesh =
						FindResource("meshModel");
					const EFFECT_RESOURCE_BINDING_DESC* Noise =
						FindResource("noise");
					const EFFECT_RESOURCE_BINDING_DESC* Mask = FindResource("mask");
					const EFFECT_RESOURCE_BINDING_DESC* Dissolve =
						FindResource("dissolve");
					const bool_t bExactIdentity =
						Occurrence->Row.strId == "source-active-017" &&
						Occurrence->Row.strRowSha256 ==
							"eaa64427a2ad034803b350bb7b5d82f23fa39f7432179d39879c14d5730b874b" &&
						Recipe->Row.strId == "material-recipe-76e78b77b8e6f2f1" &&
						Recipe->Row.strRowSha256 ==
							"df0330451fdecaa7234df290a5166907262dfa85bfeb741c06b8655269b34675" &&
						Recipe->strSourceRecipeCompositionSha256 ==
							"87f953c10c597177b091d6374c386144f687940003ebb22eeea183927aecfc5e" &&
						Recipe->strSourceMaterialPath ==
							"fx_m_mi_s_00.fx_mi.fx_s_me_missiletrail_01_5_ts_tr" &&
						Family->Row.strId == "material-family-4e1ecdcff38a53d1" &&
						Family->Row.strRowSha256 ==
							"7d67b2fb7b4fbc01e2ed61f98d452c095e09689bb9318f49dc2d3a04c65714e1" &&
						Family->strEvaluatorId ==
							"reconstructed-evaluator-47a191250dbffb2d" &&
						Family->iEvaluatorVersion == 1u &&
						Family->strEvaluatorSha256 ==
							"f909d8022f63f13934a60ee5a51b60f0138debc9567b13fcb85fcc197ce8a2dc";
					const bool_t bExactResources =
						Element.ResourceBindings.size() == 5u && nullptr != Mesh &&
						Mesh->strAssetId == "Effect/Artist/Meshes/fm_m_trail_002.wmodel" &&
						nullptr != Base && nullptr != Mask && nullptr != Noise &&
						nullptr != Dissolve && Base->strAssetId == Mask->strAssetId &&
						Mask->strAssetId ==
							"Effect/Artist/Textures/fx_d_trail_002_cl.dds" &&
						Noise->strAssetId ==
							"Effect/Artist/Textures/fx_j_risingforce_01.dds" &&
						Dissolve->strAssetId ==
							"Effect/Artist/Textures/fx_h_atypical_01_1.dds";
					const bool_t bExactFields = bExactScalars &&
						Source.Vectors.empty() && Source.Textures.size() == 3u &&
						HasTexture("alpha_tex",
							"Effect/Artist/Textures/fx_d_trail_002_cl.dds") &&
						HasTexture("uv_dissolve_tex",
							"Effect/Artist/Textures/fx_h_atypical_01_1.dds") &&
						HasTexture("uv_noise_tex",
							"Effect/Artist/Textures/fx_j_risingforce_01.dds") &&
						Source.StaticSwitches.size() == 1u &&
						Source.StaticSwitches.front().strName == "dissolve_rampmap" &&
						Source.StaticSwitches.front().bValue;
					if (!bExactIdentity || !bExactResources || !bExactFields ||
						!bExactDynamic || !Is_EffectFiniteProfileResourceContractSatisfied(
							"effect.ue3.missiletrail-01.v1", true, true, true,
							true, true, true))
					{
						return Fail("the exact Artist missile-trail finite profile contract is invalid.");
					}
					Source.strRuntimeShaderProfileId =
						"effect.ue3.missiletrail-01.v1";
					Source.DynamicParameterSemantics = {
						"missile_alpha_pan", "missile_noise_strength",
						"missile_noise_pan", "missile_dissolve" };
					++iMissileTrailFiniteProfileProjectionCount;
				}
				else if (bMainCompositeMaterial || bRuntimeMaterialV2 ||
					bArtistVisualV4)
				{
					/* Runtime Material v2 or the stable Artist visual registry owns the
					   complete shader packet for these occurrences.  Keep their
					   source-material carrier on the neutral
					   reconstructed-standard profile even when newly admitted texture
					   providers would also satisfy the legacy grouped-translucent
					   heuristic.  Prepared-resource staging then starts from one canonical
					   profile-0/texture-mask-0 seam before it binds the immutable v2 lanes. */
					Source.strRuntimeShaderProfileId =
						"effect.ue3.reconstructed-standard.v1";
					if (bMainCompositeMaterial)
						++iReconstructedEvaluatorMaterialProjectionCount;
					if (bRuntimeMaterialV2)
						++iRuntimeMaterialV2ProfileProjectionCount;
					if (bArtistVisualV4)
						++iReconstructedEvaluatorMaterialProjectionCount;
				}
				else
				{
					Source.strRuntimeShaderProfileId = bOneLayerDistortion ?
						"effect.ue3.one-layer-distortion.v1" :
						(bGroupedContract ?
							"effect.ue3.grouped-translucent.v1" :
							"effect.ue3.reconstructed-standard.v1");
				}
			}
		}

		if (Emitter.DecalAdapter.has_value() &&
			Emitter.DecalAdapter->Common.bEnabled &&
			!AssignF32(Emitter.DecalAdapter->fDepthRuntimeUnits,
				Element.Detail.Decal.fDepth, "decal adapter runtime depth"))
		{
			return false;
		}
		if (Emitter.RibbonAdapter.has_value() &&
			Emitter.RibbonAdapter->Common.bEnabled)
		{
			const auto& Adapter = *Emitter.RibbonAdapter;
			const auto FindUniqueSourceDistribution = [&Element, &Lower](
				const std::string_view strClassName,
				const std::string_view strPropertyPath)
				-> const EFFECT_DISTRIBUTION_DESC*
			{
				const EFFECT_DISTRIBUTION_DESC* pFound = nullptr;
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					Element.SourceRecipe.Modules)
				{
					if (Lower(Module.strClassName) != strClassName)
						continue;
					for (const EFFECT_DISTRIBUTION_DESC& Distribution :
						Module.Distributions)
					{
						if (Distribution.strPropertyPath != strPropertyPath)
							continue;
						if (nullptr != pFound)
							return nullptr;
						pFound = &Distribution;
					}
				}
				return pFound;
			};
			const auto ReadUniformConstant = [](
				const EFFECT_DISTRIBUTION_DESC* pDistribution,
				const uint32_t iComponentCount, float4_t& OutValue)
			{
				if (nullptr == pDistribution ||
					pDistribution->iComponentCount != iComponentCount ||
					pDistribution->iOperation != 1u)
				{
					return false;
				}

				constexpr std::array<f32_t, 5u> SampleTimes{
					-1.f, 0.f, 0.25f, 1.f, 2.f };
				const float4_t First = CEffectDistribution::Evaluate(
					*pDistribution, SampleTimes.front(), float4_t{});
				const auto Matches = [iComponentCount, &First](
					const float4_t& Value)
				{
					const f32_t* const pFirst = &First.x;
					const f32_t* const pValue = &Value.x;
					for (uint32_t i = 0u; i < iComponentCount; ++i)
					{
						if (!std::isfinite(pValue[i]) ||
							pValue[i] != pFirst[i])
						{
							return false;
						}
					}
					return true;
				};
				if (!Matches(First))
					return false;
				for (const f32_t fSampleTime : SampleTimes)
				{
					if (!Matches(CEffectDistribution::Evaluate(
						*pDistribution, fSampleTime, float4_t{})) ||
						!Matches(CEffectDistribution::Evaluate(
							*pDistribution, fSampleTime,
							float4_t(1.f, 1.f, 1.f, 1.f))))
					{
						return false;
					}
				}
				OutValue = First;
				return true;
			};
			const auto ResolveReconstructedSpawnRateScale =
				[&Fail, &Program, &ReadUniformConstant](
					const std::string& strDistributionId,
					const EFFECT_DISTRIBUTION_DESC* pSourceDistribution,
					float4_t& OutValue) -> bool_t
			{
				/* The pinned Program still carries the source null
				   RawDistributionFloat as a fabricated zero.  This nonProduct
				   preview-only bridge admits exactly the separately reconstructed
				   current Engine CDO identity value.  It is deliberately not
				   SOURCE_EXACT and must fail closed when the official Program row
				   is regenerated. */
				constexpr std::string_view PolicyId =
					"CURRENT_ENGINE_CDO_RECONSTRUCTED";
				constexpr std::string_view DescriptorSha256 =
					"45e438c780ff075bb5e485731cd8bf914b34dbac8fdb369d0824926639741826";
				constexpr std::string_view StaleProgramRowSha256 =
					"05ea1c48d12ac4a608a2477c056a264bbab38b2dba87a5893cd81257c695a85c";
				static_assert(DescriptorSha256.size() == 64u);
				static_assert(StaleProgramRowSha256.size() == 64u);

				const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION* ProgramDistribution =
					Find_UniqueProgramRowById(
						Program->Distributions, strDistributionId);
				float4_t SourceValue{};
				const std::vector<double> Zero4(4u, 0.0);
				if (nullptr == ProgramDistribution ||
					ProgramDistribution->Row.strRowSha256 != StaleProgramRowSha256 ||
					ProgramDistribution->Row.bSourceExact ||
					ProgramDistribution->eVariant !=
						EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ||
					ProgramDistribution->strPropertyPath != "ratescale" ||
					!ProgramDistribution->strSourceClass.empty() ||
					!ProgramDistribution->strSourceObjectPath.empty() ||
					ProgramDistribution->strPayloadStatus !=
						"INLINE_SOURCE_PAYLOAD" ||
					ProgramDistribution->strFidelity !=
						"DETERMINISTIC_SOURCE_RECIPE" ||
					ProgramDistribution->iComponentCount != 1u ||
					ProgramDistribution->iOperation != 1u ||
					ProgramDistribution->iRandomLockAxes != 0u ||
					ProgramDistribution->iLookupTableChunkSize != 0u ||
					ProgramDistribution->iLookupTableNumElements != 0u ||
					ProgramDistribution->DefaultMinimum != Zero4 ||
					ProgramDistribution->DefaultMaximum != Zero4 ||
					!ProgramDistribution->LookupTable.empty() ||
					!ProgramDistribution->CurveKeys.empty() ||
					!ReadUniformConstant(pSourceDistribution, 1u, SourceValue) ||
					SourceValue.x != 0.f)
				{
					return Fail("typed Ribbon RateScale " +
						std::string(PolicyId) + " policy " +
						std::string(DescriptorSha256) +
						" no longer matches the pinned null source row.");
				}

				OutValue = float4_t{ 1.f, 0.f, 0.f, 0.f };
				return true;
			};
			float4_t SpawnRate{};
			float4_t SpawnRateScale{};
			float4_t Lifetime{};
			float4_t StartSize{};
			if (Adapter.Common.strAdapterId !=
					"CASCADE_RIBBON_TYPED_ADAPTER_V1" ||
				Adapter.strWidthPolicy != "TYPED_SIZE_DISTRIBUTION_X_AXIS" ||
				Adapter.strGeometryPolicy !=
					"TYPED_RIBBON_DEFAULTS_RENDER_GEOMETRY" ||
				Adapter.strOrientationPolicy !=
					"CAMERA_FACING_SINGLE_SHEET_RECONSTRUCTED_V1" ||
				Adapter.iTypedMaxParticleInTrailCount != 500u ||
				Adapter.iOperationalMaxPoints != 500u ||
				Adapter.SpawnDistributionIds.size() != 2u ||
				!ReadUniformConstant(FindUniqueSourceDistribution(
					"particlemodulespawn", "rate"), 1u, SpawnRate) ||
				!ResolveReconstructedSpawnRateScale(
					Adapter.SpawnDistributionIds[1u],
					FindUniqueSourceDistribution(
						"particlemodulespawn", "ratescale"),
					SpawnRateScale) ||
				!ReadUniformConstant(FindUniqueSourceDistribution(
					"particlemodulelifetime", "lifetime"), 1u, Lifetime) ||
				!ReadUniformConstant(FindUniqueSourceDistribution(
					"particlemodulesize", "startsize"), 3u, StartSize) ||
				SpawnRate.x <= 0.f || SpawnRateScale.x != 1.f ||
				Lifetime.x <= 0.f || StartSize.x <= 0.f)
			{
				return Fail("typed Ribbon lifetime/width projection is invalid.");
			}
			const f32_t fWidthWorldUnits = StartSize.x * 0.01f;
			const f32_t fTilingDistanceWorldUnits =
				static_cast<f32_t>(Adapter.fTilingDistance * 0.01);
			const f32_t fDistanceTessellationStepWorldUnits =
				static_cast<f32_t>(Adapter.fDistanceTessellationStepSize * 0.01);
			if (!std::isfinite(fWidthWorldUnits) || fWidthWorldUnits <= 0.f ||
				!std::isfinite(fTilingDistanceWorldUnits) ||
				fTilingDistanceWorldUnits <= 0.f ||
				!std::isfinite(fDistanceTessellationStepWorldUnits) ||
				fDistanceTessellationStepWorldUnits <= 0.f)
				return Fail("typed Ribbon width conversion is invalid.");

			Element.Detail.Trail.iMaxPoints = Adapter.iOperationalMaxPoints;
			Element.Detail.Trail.fPointLifeTimeSeconds = Lifetime.x;
			Element.Detail.Trail.fSampleIntervalSeconds =
				1.f / (SpawnRate.x * SpawnRateScale.x);
			Element.Detail.Trail.fStartWidth = fWidthWorldUnits;
			Element.Detail.Trail.fEndWidth = fWidthWorldUnits;
			Element.Detail.Trail.fTilingDistanceWorldUnits =
				fTilingDistanceWorldUnits;
			Element.Detail.Trail.fDistanceTessellationStepWorldUnits =
				fDistanceTessellationStepWorldUnits;
			Element.Detail.Trail.bFaceCamera = true;
			++iTypedRibbonGeometryProjectionCount;
		}
		if (Emitter.ScreenPostAdapter.has_value() &&
			Emitter.ScreenPostAdapter->Common.bEnabled)
		{
			const auto& Adapter = *Emitter.ScreenPostAdapter;
			Element.Detail.ScreenPost.bEnabled = true;
			Element.Detail.ScreenPost.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			const std::string AdapterId = Lower(Adapter.Common.strAdapterId);
			if (AdapterId.find("zoom") != std::string::npos)
				Element.Detail.ScreenPost.eProfile =
					EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1;
			else if (AdapterId.find("film") != std::string::npos)
				Element.Detail.ScreenPost.eProfile =
					EFFECT_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED_V1;
			else if (AdapterId.find("rgb") != std::string::npos)
				Element.Detail.ScreenPost.eProfile =
					EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1;
			else
				return Fail("screen-post adapter profile is unsupported.");
			if (!AssignF32(Adapter.fSecondaryIntensity,
					Element.Detail.ScreenPost.fSecondaryIntensity,
					"screen-post secondary intensity") ||
				!AssignF4(Adapter.vTint, Element.Detail.ScreenPost.vTint,
					"screen-post tint"))
			{
				return false;
			}
			Element.Detail.ScreenPost.iRandomSeed =
				Emitter.Random.iEmitterRandomSeed;
		}
		if (Emitter.LightAdapter.has_value() &&
			Emitter.LightAdapter->Common.bEnabled)
		{
			const auto& Adapter = *Emitter.LightAdapter;
			if (Adapter.strPositionSourcePolicy !=
				"EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION" ||
				Adapter.fUeUnitScale != 0.01)
			{
				return Fail("point-light particle input policy is unsupported.");
			}
			Element.Detail.Light.bEnabled = true;
			Element.Detail.Light.eProfile =
				EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1;
			Element.Detail.Light.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			for (const std::string& strFieldId : Adapter.FieldIds)
			{
				const EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD* Field =
					Find_UniqueProgramRowById(Program->PointLightFields, strFieldId);
				if (nullptr == Field || Field->strModuleId != Adapter.strModuleId)
					return Fail("point-light adapter field join is invalid.");
				if (Field->strFieldPath == "brightness" && Field->fValue.has_value())
				{
					if (!AssignF32(*Field->fValue,
						Element.Detail.Light.fIntensity, "point-light intensity"))
						return false;
				}
				else if (Field->strFieldPath == "radius" &&
					Field->fValue.has_value())
				{
					if (!AssignF32(*Field->fValue * Adapter.fUeUnitScale,
						Element.Detail.Light.fRange, "point-light range"))
						return false;
				}
				else if (Field->strFieldPath == "falloffexponent" &&
					Field->fValue.has_value())
				{
					if (!AssignF32(*Field->fValue,
						Element.Detail.Light.fFalloffExponent,
						"point-light falloff"))
						return false;
				}
				else if (Field->strFieldPath == "lightcolor" &&
					Field->ColorRgba8Value.has_value())
				{
					const auto& Color = *Field->ColorRgba8Value;
					Element.Detail.Light.vColor = {
						static_cast<f32_t>(Color[0u]) / 255.f,
						static_cast<f32_t>(Color[1u]) / 255.f,
						static_cast<f32_t>(Color[2u]) / 255.f,
						static_cast<f32_t>(Color[3u]) / 255.f };
				}
			}

			const auto FindUniqueSourceDistribution = [&Element, &Lower](
				const std::string_view strClassName,
				const std::string_view strPropertyPath)
				-> const EFFECT_DISTRIBUTION_DESC*
			{
				const EFFECT_DISTRIBUTION_DESC* pFound = nullptr;
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					Element.SourceRecipe.Modules)
				{
					if (Lower(Module.strClassName) != strClassName)
						continue;
					for (const EFFECT_DISTRIBUTION_DESC& Distribution :
						Module.Distributions)
					{
						if (Distribution.strPropertyPath != strPropertyPath)
							continue;
						if (nullptr != pFound)
							return nullptr;
						pFound = &Distribution;
					}
				}
				return pFound;
			};
			const auto ReadConstant = [](const EFFECT_DISTRIBUTION_DESC* pDistribution,
				const uint32_t iComponentCount, float4_t& OutValue)
			{
				if (nullptr == pDistribution ||
					pDistribution->iComponentCount != iComponentCount ||
					pDistribution->iOperation != 1u ||
					!pDistribution->LookupTable.empty() ||
					!pDistribution->Keys.empty())
				{
					return false;
				}
				const f32_t* const pMinimum =
					&pDistribution->vDefaultMinimum.x;
				const f32_t* const pMaximum =
					&pDistribution->vDefaultMaximum.x;
				for (uint32_t i = 0u; i < iComponentCount; ++i)
				{
					if (!std::isfinite(pMinimum[i]) ||
						pMinimum[i] != pMaximum[i])
					{
						return false;
					}
				}
				OutValue = pDistribution->vDefaultMinimum;
				return true;
			};
			float4_t SourceLocation{};
			float4_t SourceStartColor{};
			float4_t SourceStartAlpha{};
			if (!ReadConstant(FindUniqueSourceDistribution(
					"particlemodulelocation", "startlocation"),
					3u, SourceLocation) ||
				!ReadConstant(FindUniqueSourceDistribution(
					"particlemodulecolor", "startcolor"),
					3u, SourceStartColor) ||
				!ReadConstant(FindUniqueSourceDistribution(
					"particlemodulecolor", "startalpha"),
					1u, SourceStartAlpha))
			{
				return Fail("point-light particle input distribution is not one "
					"typed constant after Program overlay.");
			}
			const std::array<double, 3u> ClientPosition = Source_ToClientVector(
				{ SourceLocation.x, SourceLocation.y, SourceLocation.z },
				Adapter.fUeUnitScale);
			if (!AssignF3(ClientPosition,
					Element.Detail.Particle.vInitialPositionMin,
					"point-light particle location") ||
				!AssignF3(ClientPosition,
					Element.Detail.Particle.vInitialPositionMax,
					"point-light particle location maximum"))
			{
				return false;
			}
			Element.Detail.Color.vColorMultiply = {
				SourceStartColor.x,
				SourceStartColor.y,
				SourceStartColor.z,
				SourceStartAlpha.x
			};
			++iTypedLightParticleInputProjectionCount;
		}

		/* Project the already-verified native-v14 carrier into the ordinary
		   format-13 runtime shape.  The renderer type and source space are part
		   of the drawable runtime contract, so preserve the exact Program-joined
		   values.  Only source-evidence fields that the format-13 evaluator must
		   not consume are cleared below. */
		if (Emitter.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE)
			Element.eKind = EFFECT_ELEMENT_KIND::DECAL;
		Element.SourceRecipe.strSourceContractProfileId.clear();
		Element.SourceRecipe.strSourceContractSha256.clear();
		Element.SourceRecipe.strSourceGraphSha256.clear();
		Element.SourceRecipe.strSourceClosureSha256.clear();
		Element.SourceRecipe.strSourceMaterialClosureSha256.clear();
		Element.SourceRecipe.iSourcePeakActiveParticles = 0u;
		Element.SourceRecipe.LocalReferenceBindings.clear();
		Element.SourceRecipe.ModuleCoverage.clear();
		Element.SourceRecipe.CompilerEvidence = {};
		Element.SourceRecipe.CompiledExecutionAdmission = {};
		Element.SourceRecipe.MaterialAdmission = {};
		Element.SourceRecipe.GeometryBinding = {};
		for (EFFECT_SOURCE_MODULE_DESC& Module : Element.SourceRecipe.Modules)
		{
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			{
				Distribution.strReferenceId.clear();
				Distribution.strOccurrenceId.clear();
				Distribution.strPayloadStatus.clear();
				Distribution.strFidelity.clear();
				Distribution.ExecutionAdmission = {};
				Distribution.eParameterBinding =
					EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
				Distribution.strParameterName.clear();
			}
		}
		if (!RendererMatches(Emitter.eRenderer, Element) ||
			Element.Renderer.eSourceSpace != eExpectedSourceSpace)
		{
			return Fail("the typed renderer identity was lost while projecting the "
				"native-v14 carrier into the format-13 runtime document.");
		}
		++iTypedRendererProjectionCount;
	}

	if (CoveredElements.size() != Program->Emitters.size() ||
		CoveredOccurrences.size() != Program->MaterialOccurrences.size() ||
		CoveredTextureResources.size() !=
			Program->RendererTextureResources.size() ||
		CoveredGeometryUses.size() != Program->GeometryUses.size() ||
		iOneLayerRuntimeTextureProjectionCount != 1u ||
		iParameterDistributionOverlayCount != 16u ||
		iCurveDistributionOverlayCount != 1u ||
		iClearedDistributionReferenceCount != 17u ||
		iSpriteEmitterCount != 16u ||
		iSpriteIdentitySubUVCount != 11u ||
		iSourceSubUVProjectionCount != 5u ||
		iRandom2x2SubUVCount != 3u ||
		iLinearBlend6x6SubUVCount != 2u ||
		iMaterialSubUVProjectionCount != 5u ||
		iMainCompositeMaterialProjectionCount != 13u ||
		iMissileTrailFiniteProfileProjectionCount != 1u ||
		iReconstructedEvaluatorMaterialProjectionCount != 15u ||
		iRuntimeMaterialV2ProfileProjectionCount != 17u ||
		iTypedRibbonGeometryProjectionCount != 1u ||
		iTypedLightParticleInputProjectionCount != 1u ||
		iTypedRendererProjectionCount != Program->Emitters.size())
	{
		return Fail("the 35-emitter material/resource/geometry/SubUV projection is "
			"not exhaustive.");
	}
	StagedDocument.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	StagedDocument.iLoadedFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	StagedDocument.bSourceContract = false;
	StagedDocument.strEffectAssetId = Program->strRuntimeCatalogAssetId;
	StagedDocument.strDisplayName =
		"Artist F 31470 Reconstructed Source Runtime Preview";
	if (!CEffectDocumentCodec::
		Validate_Artist31470ReconstructedRuntimeDrawable(
		StagedDocument, strOutError))
	{
		return false;
	}
	OutDocument = std::move(StagedDocument);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedSelectedDiagnosticFactory::Prepare(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pRuntimePreparation,
	const EFFECT_RECONSTRUCTED_SELECTED_DIAGNOSTIC_SELECTOR& Selector,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>&
		InOutPreparation,
	std::string& strOutError)
{
	const auto Program = nullptr == pRuntimePreparation ? nullptr :
		pRuntimePreparation->Get_Program();
	const auto Authority = nullptr == pRuntimePreparation ? nullptr :
		pRuntimePreparation->Get_RenderResourceAuthority();
	if (nullptr == Program || nullptr == Authority ||
		Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
	{
		return Reject(strOutError,
			"Diagnostic factory requires one immutable nonProduct preparation.");
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Plan;
	if (!CEffectReconstructedExecutionPlanCompiler::Compile_Preparation(
		pRuntimePreparation, Plan, strOutError))
	{
		return false;
	}
	EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST Request;
	if (!Build_DiagnosticRequest(
		*Plan, *Program, *Authority, Selector, Request, strOutError))
	{
		return false;
	}
	return CEffectReconstructedSelectedEvaluator::Prepare(
		std::move(Plan), Request, InOutPreparation, strOutError);
}

bool_t Client::CEffectReconstructedSelectedEvaluator::Prepare(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> pPlan,
	const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_REQUEST& Request,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>&
		InOutPreparation,
	std::string& strOutError)
{
	const auto RuntimePreparation = nullptr == pPlan ? nullptr :
		pPlan->Get_Preparation();
	const auto CatalogEntry = nullptr == RuntimePreparation ? nullptr :
		RuntimePreparation->Get_CatalogEntry();
	const auto Program = nullptr == RuntimePreparation ? nullptr :
		RuntimePreparation->Get_Program();
	const auto Authority = nullptr == RuntimePreparation ? nullptr :
		RuntimePreparation->Get_RenderResourceAuthority();
	if (nullptr == pPlan || nullptr == RuntimePreparation ||
		nullptr == CatalogEntry || nullptr == Program || nullptr == Authority ||
		pPlan->Get_Program().get() != Program.get() ||
		CatalogEntry->Get_Program().get() != Program.get() ||
		CatalogEntry->Get_RenderResourceAuthority().get() != Authority.get() ||
		Request.iEvaluatorVersion !=
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_VERSION ||
		Request.strOccurrenceRngContract !=
			EFFECT_RECONSTRUCTED_SELECTED_OCCURRENCE_RNG_CONTRACT ||
		Request.iOccurrenceRngVersion !=
			EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION ||
		Request.iOccurrenceRngVersion !=
			pPlan->Get_Identity().iOccurrenceRngVersion ||
		Request.iRequiredFixedStepIndex == 0u ||
		Request.iRequiredFixedStepIndex >
			static_cast<uint64_t>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ) * 60u ||
		Request.iRequiredSpawnSerial != 0u || Request.Emitters.size() != 2u ||
		pPlan->Get_Identity().iPlanVersion !=
			EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION ||
		pPlan->Get_Identity().iFixedStepHz !=
			EFFECT_RECONSTRUCTED_FIXED_STEP_HZ ||
		pPlan->Get_Identity().iCatalogRevision !=
			CatalogEntry->Get_Identity().iCatalogRevision ||
		pPlan->Get_Identity().strEffectAssetId !=
			CatalogEntry->Get_Identity().strEffectAssetId ||
		pPlan->Get_Identity().strProgramId != Program->Identity.strProgramId ||
		pPlan->Get_Identity().strProgramSha256 !=
			Program->Identity.strProgramSha256 ||
		pPlan->Get_Identity().strCandidateRawSha256 !=
			Program->Identity.strCandidateRawSha256 ||
		Authority->Identity.strProgramId != Program->Identity.strProgramId ||
		Authority->Identity.strProgramSha256 !=
			Program->Identity.strProgramSha256 ||
		Authority->Identity.strAuthorityId !=
			CatalogEntry->Get_Identity().strRenderResourceAuthorityId ||
		Authority->Identity.strSidecarRawSha256 !=
			CatalogEntry->Get_Identity().strRenderResourceSidecarRawSha256 ||
		Authority->Identity.strSidecarDecisionProjectionSha256 !=
			CatalogEntry->Get_Identity().
				strRenderResourceSidecarDecisionProjectionSha256 ||
		Authority->Identity.strSidecarReceiptSha256 !=
			CatalogEntry->Get_Identity().strRenderResourceSidecarReceiptSha256 ||
		Authority->Identity.iFormatVersion !=
			CatalogEntry->Get_Identity().iRenderResourceSidecarFormatVersion ||
		Authority->Identity.iSidecarByteCount !=
			CatalogEntry->Get_Identity().iRenderResourceSidecarByteCount ||
		Authority->Identity.strSchema !=
			CatalogEntry->Get_Identity().strRenderResourceSidecarSchema ||
		Authority->Identity.strAuthorityLinkSha256 !=
			CatalogEntry->Get_Identity().strRenderResourceAuthorityLinkSha256 ||
		Authority->Identity.strPublishReceiptSha256 !=
			CatalogEntry->Get_Identity().strRenderResourcePublishReceiptSha256 ||
		Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
	{
		return Reject(strOutError,
			"Selected evaluator requires one immutable nonProduct Catalog graph.");
	}

	std::shared_ptr<EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA>
		PreparedData = std::make_shared<
			EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA>();
	std::set<std::string, std::less<>> EmitterIds;
	if (Request.Emitters[0u].Schedule.strId !=
			Request.Emitters[1u].Schedule.strId ||
		Request.Emitters[0u].Schedule.strRowSha256 !=
			Request.Emitters[1u].Schedule.strRowSha256)
	{
		return Reject(strOutError,
			"Selected evaluator requires one shared exact schedule row.");
	}
	uint32_t MeshCount = 0u;
	uint32_t SpriteCount = 0u;
	uint32_t HandlerCount = 0u;
	for (uint32_t Index = 0u; Index < Request.Emitters.size(); ++Index)
	{
		const auto& Selection = Request.Emitters[Index];
		MeshCount += Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ? 1u : 0u;
		SpriteCount += Selection.eKind ==
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE ? 1u : 0u;
		HandlerCount += static_cast<uint32_t>(Selection.Handlers.size());
		if (!EmitterIds.emplace(Selection.Emitter.strId).second)
			return Reject(strOutError,
				"Selected evaluator emitter selection is duplicated.");
		EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARED_DATA::
			EMITTER_OPERATION Operation;
		if (!Validate_EmitterSelection(*pPlan, *Program, *Authority, Selection,
			Index, true, Operation, strOutError))
		{
			return false;
		}
		PreparedData->Emitters.push_back(std::move(Operation));
	}
	if (MeshCount != 1u || SpriteCount != 1u ||
		HandlerCount == 0u ||
		HandlerCount != Request.iExpectedConsumedHandlerCount)
		return Reject(strOutError,
			"Selected evaluator requires exactly one Mesh and one Sprite.");

	std::shared_ptr<EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		Staged(new EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION());
	Staged->m_pPlan = std::move(pPlan);
	Staged->m_pRuntimePreparation = RuntimePreparation;
	Staged->m_pCatalogEntry = CatalogEntry;
	Staged->m_pProgram = Program;
	Staged->m_pRenderResourceAuthority = Authority;
	Staged->m_Request = Request;
	Staged->m_pPreparedData = std::move(PreparedData);
	InOutPreparation = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedSelectedEvaluator::Evaluate(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_EVALUATOR_PREPARATION>
		pPreparation,
	const uint64_t iFixedStepIndex,
	const uint64_t iSpawnSerial,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME>& InOutFrame,
	std::string& strOutError)
{
	const auto Plan = nullptr == pPreparation ? nullptr :
		pPreparation->Get_Plan();
	const auto RuntimePreparation = nullptr == pPreparation ? nullptr :
		pPreparation->Get_RuntimePreparation();
	const auto CatalogEntry = nullptr == pPreparation ? nullptr :
		pPreparation->Get_CatalogEntry();
	const auto Program = nullptr == pPreparation ? nullptr :
		pPreparation->Get_Program();
	const auto Authority = nullptr == pPreparation ? nullptr :
		pPreparation->Get_RenderResourceAuthority();
	const auto PreparedData = nullptr == pPreparation ? nullptr :
		pPreparation->m_pPreparedData;
	if (nullptr == pPreparation || nullptr == Plan ||
		nullptr == RuntimePreparation || nullptr == CatalogEntry ||
		nullptr == Program || nullptr == Authority || nullptr == PreparedData ||
		Plan->Get_Preparation().get() != RuntimePreparation.get() ||
		Plan->Get_Program().get() != Program.get() ||
		RuntimePreparation->Get_CatalogEntry().get() != CatalogEntry.get() ||
		RuntimePreparation->Get_Program().get() != Program.get() ||
		RuntimePreparation->Get_RenderResourceAuthority().get() !=
			Authority.get() ||
		CatalogEntry->Get_Program().get() != Program.get() ||
		CatalogEntry->Get_RenderResourceAuthority().get() != Authority.get() ||
		iFixedStepIndex !=
			pPreparation->Get_Request().iRequiredFixedStepIndex ||
		iSpawnSerial != pPreparation->Get_Request().iRequiredSpawnSerial ||
		PreparedData->Emitters.size() !=
			pPreparation->Get_Request().Emitters.size() ||
		Program->Admission.bRuntimeExecution || Program->Admission.bProduct)
	{
		return Reject(strOutError,
			"Selected evaluator preparation, step, or serial is invalid.");
	}

	FIXED_STEP_SIMULATION Simulation;
	if (!Simulate_FixedSteps(*Plan, iFixedStepIndex, Simulation, strOutError))
		return false;
	std::shared_ptr<EFFECT_RECONSTRUCTED_SELECTED_FRAME> Staged(
		new EFFECT_RECONSTRUCTED_SELECTED_FRAME());
	Staged->m_pPreparation = pPreparation;
	Staged->m_iFixedStepIndex = iFixedStepIndex;
	Staged->m_fSampleTimeSeconds = Simulation.fSampleTimeSeconds;
	Staged->m_strOccurrenceRngContract =
		pPreparation->Get_Request().strOccurrenceRngContract;
	Staged->m_iOccurrenceRngVersion =
		pPreparation->Get_Request().iOccurrenceRngVersion;
	for (const auto& Operation : PreparedData->Emitters)
	{
		if (Operation.iSelectionIndex >=
				pPreparation->Get_Request().Emitters.size() ||
			nullptr == Operation.pPlanEmitter ||
			nullptr == Operation.pPlanSchedule)
		{
			return Reject(strOutError,
				"Selected evaluator prepared emitter is invalid.");
		}
		const auto& Selection = pPreparation->Get_Request().Emitters[
			Operation.iSelectionIndex];
		const auto StateIt = Simulation.States.find(
			Operation.pPlanEmitter->strEmitterId);
		if (StateIt == Simulation.States.end())
			return Reject(strOutError,
				"Selected evaluator timing emitter is absent.");
		const auto ParticleIt = std::find_if(StateIt->second.Particles.begin(),
			StateIt->second.Particles.end(), [iSpawnSerial](const auto& Particle)
			{
				return Particle.iSpawnSerial == iSpawnSerial;
			});
		if (ParticleIt == StateIt->second.Particles.end() ||
			std::count_if(StateIt->second.Particles.begin(),
				StateIt->second.Particles.end(), [iSpawnSerial](const auto& Particle)
				{
					return Particle.iSpawnSerial == iSpawnSerial;
				}) != 1u)
		{
			return Reject(strOutError,
				"Selected evaluator exact active occurrence is absent or duplicated.");
		}
		EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET Timing;
		Timing.strOccurrenceId = ParticleIt->strOccurrenceId;
		Timing.strScheduleId = Operation.pPlanSchedule->strScheduleId;
		Timing.strEmitterId = Operation.pPlanEmitter->strEmitterId;
		Timing.eRenderer = Operation.pPlanEmitter->eRenderer;
		Timing.iLoopIndex = ParticleIt->iLoopIndex;
		Timing.iSpawnSerial = ParticleIt->iSpawnSerial;
		Timing.iSpawnStep = ParticleIt->iSpawnStep;
		Timing.iOccurrenceRandomValue =
			ParticleIt->iOccurrenceRandomValue;
		Timing.iLifetimeRandomValue = ParticleIt->iLifetimeRandomValue;
		Timing.fAgeSeconds = (std::max)(0.0,
			Simulation.fSampleTimeSeconds - ParticleIt->fSpawnTimeSeconds);
		Timing.fLifetimeSeconds = ParticleIt->fLifetimeSeconds;
		if (Timing.strScheduleId != Selection.Schedule.strId ||
			Timing.strEmitterId != Selection.Emitter.strId ||
			Timing.eRenderer != Selection.eRenderer ||
			Timing.iSpawnSerial != iSpawnSerial ||
			Timing.iSpawnStep != iFixedStepIndex ||
			Timing.fAgeSeconds != 0.0)
		{
			return Reject(strOutError,
				"Selected evaluator exact timing packet is invalid.");
		}
		SELECTED_VISUAL_RESULT Visual;
		if (!Evaluate_SelectedVisual(*Plan, Operation, Selection, Timing,
			Simulation.fSampleTimeSeconds, true, Visual, strOutError))
		{
			return false;
		}
		EFFECT_RECONSTRUCTED_SELECTED_PACKET Packet;
		Packet.m_pPreparation = pPreparation;
		Packet.m_iSelectionIndex = Operation.iSelectionIndex;
		Packet.m_eKind = Selection.eKind;
		Packet.m_Timing = std::move(Timing);
		Packet.m_iFinalRandomState = Visual.iFinalRandomState;
		Packet.m_iRandomDrawCount = Visual.iRandomDrawCount;
		Packet.m_Values = std::move(Visual.Values);
		Packet.m_SpriteSink = Selection.SpriteSink;
		for (const auto& Handler : Selection.Handlers)
		{
			EFFECT_RECONSTRUCTED_SELECTED_HANDLER_CONSUMPTION Consumption;
			Consumption.Module = Handler.Module;
			Consumption.Handler = Handler.Handler;
			Consumption.strImplementationId = Handler.strImplementationId;
			Consumption.iImplementationVersion =
				Handler.iImplementationVersion;
			Consumption.strImplementationSha256 =
				Handler.strImplementationSha256;
			Packet.m_ConsumedHandlers.push_back(std::move(Consumption));
		}
		Staged->m_iConsumedHandlerCount += static_cast<uint32_t>(
			Packet.m_ConsumedHandlers.size());
		Staged->m_Packets.push_back(std::move(Packet));
	}
	const uint32_t MeshCount = static_cast<uint32_t>(std::count_if(
		Staged->m_Packets.begin(), Staged->m_Packets.end(), [](const auto& Packet)
		{
			return Packet.Get_Kind() ==
				EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH;
		}));
	const uint32_t SpriteCount = static_cast<uint32_t>(std::count_if(
		Staged->m_Packets.begin(), Staged->m_Packets.end(), [](const auto& Packet)
		{
			return Packet.Get_Kind() ==
				EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE;
		}));
	if (Staged->m_Packets.size() != 2u || MeshCount != 1u ||
		SpriteCount != 1u || Staged->m_iConsumedHandlerCount == 0u ||
		Staged->m_iConsumedHandlerCount !=
			pPreparation->Get_Request().iExpectedConsumedHandlerCount)
	{
		return Reject(strOutError,
			"Selected evaluator frame is empty or incomplete.");
	}
	Staged->m_strProjectionSha256 = Compute_SelectedFrameProjection(*Staged);
	if (Staged->m_strProjectionSha256.size() != 64u)
		return Reject(strOutError,
			"Selected evaluator frame could not be sealed.");
	InOutFrame = std::move(Staged);
	strOutError.clear();
	return true;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
bool_t Client::CEffectReconstructedExecutionPlanCompiler::Compile_ProgramForTests(
	const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& CatalogIdentity,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN>& InOutPlan,
	std::string& strOutError)
{
	if (nullptr == pProgram)
		return Reject(strOutError,
			"Reconstructed execution test compiler requires a typed Program.");
	COMPILED_PLAN_DATA Data;
	if (!Compile_Program(CatalogIdentity, *pProgram, Data, strOutError))
		return false;
	std::shared_ptr<EFFECT_RECONSTRUCTED_EXECUTION_PLAN> Staged(
		new EFFECT_RECONSTRUCTED_EXECUTION_PLAN());
	Staged->m_Identity = std::move(Data.Identity);
	Staged->m_Summary = std::move(Data.Summary);
	Staged->m_SeededLifetimeAuthority =
		std::move(Data.SeededLifetimeAuthority);
	Staged->m_ScheduleOrder = std::move(Data.ScheduleOrder);
	Staged->m_EmitterOrder = std::move(Data.EmitterOrder);
	Staged->m_ModuleOrder = std::move(Data.ModuleOrder);
	Staged->m_DistributionOrder = std::move(Data.DistributionOrder);
	Staged->m_SeedPolicyOrder = std::move(Data.SeedPolicyOrder);
	Staged->m_Schedules = std::move(Data.Schedules);
	Staged->m_Emitters = std::move(Data.Emitters);
	Staged->m_Modules = std::move(Data.Modules);
	Staged->m_Distributions = std::move(Data.Distributions);
	Staged->m_SeedPolicies = std::move(Data.SeedPolicies);
	Staged->m_pProgram = std::move(pProgram);
	InOutPlan = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectReconstructedCpuInspector::Simulate(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_EXECUTION_PLAN> pPlan,
	const double fSampleTimeSeconds,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE>& InOutState,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME>& InOutFrame,
	std::string& strOutError)
{
	if (nullptr == pPlan || !Is_Finite(fSampleTimeSeconds) ||
		fSampleTimeSeconds < 0.0 || fSampleTimeSeconds > 60.0 ||
		pPlan->Get_Identity().iPlanVersion !=
			EFFECT_RECONSTRUCTED_EXECUTION_PLAN_VERSION ||
		pPlan->Get_Identity().iOccurrenceRngVersion !=
			EFFECT_RECONSTRUCTED_OCCURRENCE_RNG_VERSION ||
		pPlan->Get_Identity().iFixedStepHz !=
			EFFECT_RECONSTRUCTED_FIXED_STEP_HZ ||
		pPlan->Get_Summary().iEmitterCount != 35u)
	{
		return Reject(strOutError,
			"Reconstructed CPU inspection input or Plan identity is invalid.");
	}

	constexpr double FixedStep =
		1.0 / static_cast<double>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ);
	constexpr double StepEpsilon = 1.0e-9;
	const uint64_t TargetSteps = static_cast<uint64_t>(std::floor(
		fSampleTimeSeconds / FixedStep + StepEpsilon));
	FIXED_STEP_SIMULATION Simulation;
	if (!Simulate_FixedSteps(*pPlan, TargetSteps, Simulation, strOutError))
		return false;
	const auto& States = Simulation.States;

	std::shared_ptr<EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE> State(
		new EFFECT_RECONSTRUCTED_CPU_INSPECTION_STATE());
	State->m_pPlan = pPlan;
	State->m_iFixedStepIndex = TargetSteps;
	State->m_fSampleTimeSeconds = static_cast<double>(TargetSteps) * FixedStep;
	for (const std::string& EmitterId : pPlan->Get_EmitterOrder())
		State->m_Emitters.push_back(States.at(EmitterId).Public);
	State->m_strProjectionSha256 = Compute_StateProjection(*State);

	std::shared_ptr<EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME> Frame(
		new EFFECT_RECONSTRUCTED_CPU_INSPECTION_FRAME());
	Frame->m_pPlan = pPlan;
	Frame->m_iFixedStepIndex = TargetSteps;
	Frame->m_fSampleTimeSeconds = State->m_fSampleTimeSeconds;
	for (const std::string& ScheduleId : pPlan->Get_ScheduleOrder())
	{
		const auto& Schedule = pPlan->Get_Schedules().at(ScheduleId);
		for (const std::string& EmitterId : Schedule.EmitterIds)
		{
			const auto& Emitter = pPlan->Get_Emitters().at(EmitterId);
			for (const TIMING_PARTICLE& Particle : States.at(EmitterId).Particles)
			{
				EFFECT_RECONSTRUCTED_CPU_OCCURRENCE_PACKET Packet;
				Packet.strOccurrenceId = Particle.strOccurrenceId;
				Packet.strScheduleId = ScheduleId;
				Packet.strEmitterId = EmitterId;
				Packet.eRenderer = Emitter.eRenderer;
				Packet.iLoopIndex = Particle.iLoopIndex;
				Packet.iSpawnSerial = Particle.iSpawnSerial;
				Packet.iSpawnStep = Particle.iSpawnStep;
				Packet.iOccurrenceRandomValue =
					Particle.iOccurrenceRandomValue;
				Packet.iLifetimeRandomValue = Particle.iLifetimeRandomValue;
				Packet.fAgeSeconds = (std::max)(0.0,
					Frame->m_fSampleTimeSeconds - Particle.fSpawnTimeSeconds);
				Packet.fLifetimeSeconds = Particle.fLifetimeSeconds;
				Frame->m_ActiveOccurrences.push_back(std::move(Packet));
			}
		}
	}
	Frame->m_strProjectionSha256 = Compute_FrameProjection(*Frame);
	if (State->m_strProjectionSha256.size() != 64u ||
		Frame->m_strProjectionSha256.size() != 64u)
	{
		return Reject(strOutError,
			"Reconstructed CPU inspection snapshot could not be sealed.");
	}
	InOutState = std::move(State);
	InOutFrame = std::move(Frame);
	strOutError.clear();
	return true;
}
#endif
