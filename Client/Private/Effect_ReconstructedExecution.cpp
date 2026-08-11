#include "Effect_ReconstructedExecution.h"

#include "Effect_RuntimeAuthority.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string_view>
#include <utility>

namespace
{
	using namespace Client;

	constexpr std::string_view ARTIST_EFFECT_ID =
		"effect.artist.skill.31470";
	constexpr std::string_view ARTIST_PROGRAM_ID =
		"effect.artist.skill.31470.reconstructed-approved-v1";
	constexpr std::string_view ARTIST_PROGRAM_SHA256 =
		"618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b";
	constexpr std::string_view ARTIST_CANDIDATE_SHA256 =
		"72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849";
	constexpr std::string_view ARTIST_RANDOM_POLICY =
		"DETERMINISTIC_OCCURRENCE_RNG_FROM_SOURCE_CANDIDATE_V1";
	constexpr std::string_view ARTIST_SEED_EVALUATOR =
		"ue3.particle-random-seed-info.current-default.v1";
	constexpr std::string_view FROZEN_DISTRIBUTION_TARGET_PROJECTION_SHA256 =
		"9f99d7a65d6e2a74bc241dd4268751fc876522c9deac387eacfb40be7cc429b1";
	constexpr std::string_view FROZEN_DISTRIBUTION_OWNER_PROJECTION_SHA256 =
		"35b0977b106c003b2542327959dd1ee11ea326dd17324c5dbc242153b086bd88";
	constexpr std::string_view FROZEN_EXECUTION_PLAN_SEMANTIC_PROJECTION_SHA256 =
		"e05c09542624522d20bfdcb0e27913c4aeeec7f45e0e397b11072ac5859bd8df";
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
		"5fba6a5d4df33c509efe4ae08f30db0644bd78aa245e434c4fad33c3a082bc3f";
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
			Identity.iCandidateByteCount != 15'072'141u ||
			Identity.strProgramId != ARTIST_PROGRAM_ID ||
			Identity.strProgramSha256 != ARTIST_PROGRAM_SHA256 ||
			Identity.strCandidateRawSha256 != ARTIST_CANDIDATE_SHA256 ||
			Identity.strCompilerRevision !=
				"artist31470.reconstructed-runtime-program-link-v1" ||
			Identity.strCandidateBuilderCommitId !=
				"a85b8b41afb2f2a51bceafa55d06bf0937b1a245" ||
			Identity.strCandidateBuilderTreeId !=
				"384ed35ca808ab9a71a4edb703ca4d9121b48c18")
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
				"a85b8b41afb2f2a51bceafa55d06bf0937b1a245" ||
			Program.Identity.strBuilderAuthorityTreeId !=
				"384ed35ca808ab9a71a4edb703ca4d9121b48c18" ||
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
			return Reject(strOutError,
				"Reconstructed execution frozen semantic projection authority is invalid.");
		}
		OutData = std::move(Data);
		strOutError.clear();
		return true;
	}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	uint32_t Next_Random(uint32_t& iInOutState)
	{
		uint32_t Value = iInOutState;
		Value ^= Value << 13u;
		Value ^= Value >> 17u;
		Value ^= Value << 5u;
		iInOutState = 0u == Value ? 1u : Value;
		return iInOutState;
	}

	struct SIMULATED_PARTICLE final
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
		std::vector<SIMULATED_PARTICLE> Particles;
	};

	bool_t Spawn_InspectionParticles(
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
				"Reconstructed CPU inspection lifetime evaluator is not executable.");
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
					"Reconstructed CPU inspection refuses a non-positive lifetime fallback.");
			SIMULATED_PARTICLE Particle;
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

	std::map<std::string, MUTABLE_EMITTER_STATE, std::less<>> States;
	for (const std::string& EmitterId : pPlan->Get_EmitterOrder())
	{
		const auto& Emitter = pPlan->Get_Emitters().at(EmitterId);
		MUTABLE_EMITTER_STATE State;
		State.Public.strEmitterId = EmitterId;
		State.Public.strScheduleId = Emitter.strScheduleId;
		State.Public.iRandomState = Emitter.iEmitterRandomSeed;
		if (Emitter.strLifetimeSeedPolicyId.has_value())
		{
			const auto& Seed = pPlan->Get_SeedPolicies().at(
				*Emitter.strLifetimeSeedPolicyId);
			State.Public.iLifetimeRandomState = static_cast<uint32_t>(
				Seed.RandomSeeds.front());
			if (0u == State.Public.iLifetimeRandomState)
				State.Public.iLifetimeRandomState = 1u;
		}
		States.emplace(EmitterId, std::move(State));
	}

	constexpr double FixedStep =
		1.0 / static_cast<double>(EFFECT_RECONSTRUCTED_FIXED_STEP_HZ);
	constexpr double StepEpsilon = 1.0e-9;
	const uint64_t TargetSteps = static_cast<uint64_t>(std::floor(
		fSampleTimeSeconds / FixedStep + StepEpsilon));
	for (uint64_t Step = 1u; Step <= TargetSteps; ++Step)
	{
		const double Time = static_cast<double>(Step) * FixedStep;
		for (const std::string& ScheduleId : pPlan->Get_ScheduleOrder())
		{
			const auto& Schedule = pPlan->Get_Schedules().at(ScheduleId);
			for (const std::string& EmitterId : Schedule.EmitterIds)
			{
				const auto& Emitter = pPlan->Get_Emitters().at(EmitterId);
				auto& State = States.at(EmitterId);
				std::erase_if(State.Particles,
					[Time, StepEpsilon](const SIMULATED_PARTICLE& Particle)
					{
						return Time - Particle.fSpawnTimeSeconds + StepEpsilon >=
							Particle.fLifetimeSeconds;
					});
				State.Public.iActiveCount =
					static_cast<uint32_t>(State.Particles.size());
				const double ScheduleElapsed = Time - Schedule.fGlobalTimeSeconds;
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
							const auto& Seed = pPlan->Get_SeedPolicies().at(
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
						if (!Spawn_InspectionParticles(*pPlan, Emitter, State,
							LoopIndex, Count, Step, Time, EmitterTime, strOutError))
						{
							return false;
						}
					}
					const auto& RateDistribution = pPlan->Get_Distributions().at(
						Emitter.strSpawnRateDistributionId);
					const auto& RateScaleDistribution =
						pPlan->Get_Distributions().at(
							Emitter.strSpawnRateScaleDistributionId);
					if (!RateDistribution.bCpuTimingExecutable ||
						!RateScaleDistribution.bCpuTimingExecutable)
					{
						return Reject(strOutError,
							"Reconstructed CPU inspection spawn evaluator is not executable.");
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
						return Reject(strOutError,
							"Reconstructed CPU inspection spawn rate is invalid.");
					State.fSpawnAccumulator += Rate * RateScale * FixedStep;
					if (!Is_Finite(State.fSpawnAccumulator) ||
						State.fSpawnAccumulator >
							static_cast<double>((std::numeric_limits<uint32_t>::max)()))
					{
						return Reject(strOutError,
							"Reconstructed CPU inspection spawn accumulator overflowed.");
					}
					const uint32_t SpawnCount = static_cast<uint32_t>(
						std::floor(State.fSpawnAccumulator));
					State.fSpawnAccumulator -= static_cast<double>(SpawnCount);
					if (!Spawn_InspectionParticles(*pPlan, Emitter, State,
						LoopIndex, SpawnCount, Step, Time, EmitterTime, strOutError))
					{
						return false;
					}
				}
				State.Public.iActiveCount =
					static_cast<uint32_t>(State.Particles.size());
				if (State.Public.iActiveCount > Emitter.iOperationalMaxParticles)
					return Reject(strOutError,
						"Reconstructed CPU inspection exceeded the operational cap.");
			}
		}
	}

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
			for (const SIMULATED_PARTICLE& Particle : States.at(EmitterId).Particles)
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
