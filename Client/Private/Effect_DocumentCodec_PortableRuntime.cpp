#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>


using namespace Client::EffectDocumentCodecDetail;

namespace Client::EffectDocumentCodecDetail
{


	constexpr std::array<std::string_view, 51u>
		PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES = {
			"particlemodulecollision",
			"particlemoduleattractorpoint",
			"particlemodulekillheight",
			"particlemoduleacceleration",
			"particlemoduleaccelerationoverlifetime",
			"particlemodulecameraoffset",
			"particlemodulecolor",
			"particlemodulecoloroverlife",
			"particlemodulecolorscaleoverlife",
			"particlemoduleeventgenerator",
			"particlemoduleeventreceiverspawn",
			"particlemodulelifetime",
			"particlemodulelocation",
			"particlemodulelocationcirclesurface",
			"particlemodulelocationdirect",
			"particlemodulelocationemitter",
			"particlemodulelocationemitterdirect",
			"efparticlemodulelocationemitterdirect",
			"particlemodulelocalvectorfield",
			"particlemodulelocationonground",
			"particlemodulelocationprimitivecylinder",
			"particlemodulelocationprimitivecylinderspin",
			"particlemodulelocationprimitivesphere",
			"particlemodulemeshrotation",
			"particlemodulemeshrotationrate",
			"particlemodulemeshrotationratemultiplylife",
			"particlemodulemeshrotationrateoverlife",
			"particlemoduleorientationaxislock",
			"particlemoduleorbit",
			"particlemoduleparameterdynamic",
			"particlemodulerequired",
			"particlemodulerotation",
			"particlemodulerotationrate",
			"particlemodulerotationratemultiplylife",
			"particlemodulesize",
			"particlemodulesizescale",
			"particlemodulesizescalebytime",
			"particlemodulesizemultiplylife",
			"particlemodulesizemultiplyvelocity",
			"particlemodulespawn",
			"particlemodulespawnperunit",
			"particlemodulesubuv",
			"particlemodulesubuvmovie",
			"particlemoduletypedatamesh",
			"particlemodulevectorfieldrotationrate",
			"particlemodulevectorfieldscale",
			"particlemodulevectorfieldscaleoverlife",
			"particlemodulevelocity",
			"particlemodulevelocityinheritparent",
			"particlemodulevelocityoverlifetime",
			"particlemodulevortex"
		};


	constexpr std::array<std::pair<std::string_view, std::string_view>, 80u>
		PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES = {
			std::pair{ "particlemoduleattractorpoint", "position" },
			std::pair{ "particlemoduleattractorpoint", "range" },
			std::pair{ "particlemoduleattractorpoint", "strength" },
			std::pair{ "particlemodulekillheight", "height" },
            std::pair{ "particlemodulecollision", "dampingfactor" },
            std::pair{ "particlemodulecollision", "dampingfactorrotation" },
            std::pair{ "particlemodulecollision", "maxcollisions" },
            std::pair{ "particlemodulecollision", "delayamount" },
            std::pair{ "particlemodulecollision", "particlemass" },
			std::pair{ "efparticlemoduleacceleration", "acceldata" },
			std::pair{ "particlemoduleacceleration", "acceleration" },
			std::pair{ "particlemoduleaccelerationoverlifetime", "acceloverlife" },
			std::pair{ "particlemodulecameraoffset", "cameraoffset" },
			std::pair{ "particlemodulecolor", "startalpha" },
			std::pair{ "particlemodulecolor", "startcolor" },
			std::pair{ "particlemodulecoloroverlife", "alphaoverlife" },
			std::pair{ "particlemodulecoloroverlife", "coloroverlife" },
			std::pair{ "particlemodulecolorscaleoverlife", "alphascaleoverlife" },
			std::pair{ "particlemodulecolorscaleoverlife", "colorscaleoverlife" },
			std::pair{ "particlemoduleeventreceiverspawn", "inheritvelocityscale" },
			std::pair{ "particlemoduleeventreceiverspawn", "spawncount" },
			std::pair{ "particlemodulelifetime", "lifetime" },
			std::pair{ "particlemodulelocation", "startlocation" },
			std::pair{ "particlemodulelocationcirclesurface", "startlocation" },
			std::pair{ "particlemodulelocationcirclesurface", "startradius" },
			std::pair{ "particlemodulelocationcirclesurface", "startrot" },
			std::pair{ "particlemodulelocationcirclesurface", "velocityscale" },
			std::pair{ "particlemodulelocationdirect", "direction" },
			std::pair{ "particlemodulelocationdirect", "location" },
			std::pair{ "particlemodulelocationdirect", "locationoffset" },
			std::pair{ "particlemodulelocationdirect", "scalefactor" },
			std::pair{ "particlemodulelocationonground", "adjustlocation" },
			std::pair{ "particlemodulelocationonground", "skiplocation" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startheight" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startlocation" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startradius" },
			std::pair{ "particlemodulelocationprimitivecylinder", "velocityscale" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "spinangle" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startcylinderrot" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startheight" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startlocation" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startradius" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "velocityscale" },
			std::pair{ "particlemodulelocationprimitivesphere", "startlocation" },
			std::pair{ "particlemodulelocationprimitivesphere", "startradius" },
			std::pair{ "particlemodulelocationprimitivesphere", "velocityscale" },
			std::pair{ "particlemodulemeshrotation", "startrotation" },
			std::pair{ "particlemodulemeshrotationrate", "startrotationrate" },
			std::pair{ "particlemodulemeshrotationratemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulemeshrotationrateoverlife", "rotrate" },
			std::pair{ "particlemoduleorbit", "offsetamount" },
			std::pair{ "particlemoduleorbit", "rotationamount" },
			std::pair{ "particlemoduleorbit", "rotationrateamount" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[0].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[1].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[2].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[3].paramvalue" },
			std::pair{ "particlemodulerequired", "spawnrate" },
			std::pair{ "particlemodulerotation", "startrotation" },
			std::pair{ "particlemodulerotationrate", "startrotationrate" },
			std::pair{ "particlemodulerotationratemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulesize", "startsize" },
			std::pair{ "particlemodulesizescale", "sizescale" },
			std::pair{ "particlemodulesizescalebytime", "sizescalebytime" },
			std::pair{ "particlemodulesizemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulesizemultiplyvelocity", "velocitymultiplier" },
			std::pair{ "particlemodulespawn", "rate" },
			std::pair{ "particlemodulespawn", "ratescale" },
			std::pair{ "particlemodulespawnperunit", "spawnperunit" },
			std::pair{ "particlemodulesubuv", "subimageindex" },
			std::pair{ "particlemodulesubuvmovie", "subimageindex" },
			std::pair{ "particlemodulesubuvmovie", "framerate" },
			std::pair{ "particlemodulevectorfieldscale", "scale" },
			std::pair{ "particlemodulevectorfieldscaleoverlife", "scaleoverlife" },
			std::pair{ "particlemodulevelocity", "startvelocity" },
			std::pair{ "particlemodulevelocity", "startvelocityradial" },
			std::pair{ "particlemodulevelocityinheritparent", "scale" },
			std::pair{ "particlemodulevelocityoverlifetime", "veloverlife" },
			std::pair{ "efparticlemodulevortex", "poweracceleration" }
		};


	constexpr std::array<std::pair<std::string_view, size_t>, 23u>
		PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS = {
			std::pair{ "particlemoduleacceleration", 2u },
			std::pair{ "particlemodulecameraoffset", 2u },
			std::pair{ "particlemodulecolor", 3u },
			std::pair{ "particlemodulecolorscaleoverlife", 5u },
			std::pair{ "particlemoduleeventgenerator", 2u },
			std::pair{ "particlemodulelifetime", 2u },
			std::pair{ "particlemodulelocation", 3u },
			std::pair{ "particlemodulelocationcirclesurface", 2u },
			std::pair{ "particlemodulelocationprimitivecylinder", 2u },
			std::pair{ "particlemodulelocationprimitivecylinderspin", 2u },
			std::pair{ "particlemodulelocationprimitivesphere", 2u },
			std::pair{ "particlemodulemeshrotation", 5u },
			std::pair{ "particlemodulemeshrotationrate", 2u },
			// Ordered multiply modules already compose in the shared update loop.
			std::pair{ "particlemodulemeshrotationratemultiplylife", 2u },
			std::pair{ "particlemoduleorientationaxislock", 2u },
			std::pair{ "particlemoduleorbit", 2u },
			std::pair{ "particlemodulerotation", 3u },
			std::pair{ "particlemodulerotationrate", 2u },
			std::pair{ "particlemodulerotationratemultiplylife", 2u },
			std::pair{ "particlemodulesize", 2u },
			std::pair{ "particlemodulesizemultiplylife", 5u },
			std::pair{ "particlemodulevelocity", 2u },
			std::pair{ "particlemodulevelocityoverlifetime", 3u }
		};


	std::string_view NormalizePortableParticleModuleClass(
		const std::string_view Value)
	{
		std::string_view Result = Value;
		if (Result.starts_with("efparticlemodule"))
			Result.remove_prefix(2u);
		if (Result.ends_with("_seeded"))
			Result.remove_suffix(7u);
		return Result;
	}


	std::string_view PortableParticleDistributionCapabilityClass(
		const std::string_view strSourceClass,
		const std::string_view strNormalizedClass)
	{
		if (strNormalizedClass == "particlemoduleacceleration" &&
			strSourceClass.starts_with("efparticlemoduleacceleration"))
		{
			return "efparticlemoduleacceleration";
		}
		if (strNormalizedClass == "particlemodulevortex" &&
			strSourceClass.starts_with("efparticlemodulevortex"))
		{
			return "efparticlemodulevortex";
		}
		return strNormalizedClass;
	}


	bool_t IsPortableAuthoredParticleDistributionProperty(
		const std::string_view strModuleClass,
		const std::string_view strPropertyPath)
	{
		return std::ranges::find(
			PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES,
			std::pair{ strModuleClass, strPropertyPath }) !=
			PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.end();
	}


	bool_t IsPortableNullCdoDistribution(
		const EFFECT_DISTRIBUTION_DESC& Distribution)
	{
		const auto IsZero4 = [](const float4_t& Value)
		{
			return Value.x == 0.f && Value.y == 0.f &&
				Value.z == 0.f && Value.w == 0.f;
		};
		return Distribution.strSourceClass.empty() &&
			Distribution.strSourceObjectPath.empty() &&
			Distribution.iComponentCount == 1u &&
			Distribution.iOperation == 1u &&
			Distribution.iRandomLockAxes == 0u &&
			Distribution.iLookupTableChunkSize == 0u &&
			Distribution.iLookupTableNumElements == 0u &&
			Distribution.fLookupTableTimeScale == 0.f &&
			Distribution.fLookupTableStartTime == 0.f &&
			IsZero4(Distribution.vDefaultMinimum) &&
			IsZero4(Distribution.vDefaultMaximum) &&
			Distribution.LookupTable.empty() && Distribution.Keys.empty();
	}


	void AppendPortableDistributionProbeTimes(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		const f32_t fDurationSeconds,
		std::vector<f32_t>& InOutTimes)
	{
		const auto Append = [fDurationSeconds, &InOutTimes](const f32_t fTime)
		{
			if (std::isfinite(fTime) && fTime >= 0.f &&
				fTime <= fDurationSeconds)
			{
				InOutTimes.push_back(fTime);
			}
		};
		for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
		{
			Append(Distribution.Keys[iKey].fTime);
			if (iKey + 1u < Distribution.Keys.size())
			{
				Append(0.5f * (Distribution.Keys[iKey].fTime +
					Distribution.Keys[iKey + 1u].fTime));
			}
		}

		if (Distribution.LookupTable.empty() ||
			Distribution.fLookupTableTimeScale <= 0.f)
		{
			return;
		}
		constexpr size_t CookedLookupRangeValueCount = 2u;
		const size_t iChunkSize =
			0u != Distribution.iLookupTableChunkSize ?
				Distribution.iLookupTableChunkSize :
				static_cast<size_t>(Distribution.iComponentCount) *
					(Distribution.iOperation >= 2u ? 2u : 1u);
		const size_t iPayloadCount =
			Distribution.LookupTable.size() >= CookedLookupRangeValueCount ?
				Distribution.LookupTable.size() - CookedLookupRangeValueCount : 0u;
		const size_t iEntryCount = 0u == iChunkSize ? 0u :
			iPayloadCount / iChunkSize;
		for (size_t iEntry = 0u; iEntry < iEntryCount; ++iEntry)
		{
			const f32_t fTime = Distribution.fLookupTableStartTime +
				static_cast<f32_t>(iEntry) /
					Distribution.fLookupTableTimeScale;
			Append(fTime);
			if (iEntry + 1u < iEntryCount)
			{
				Append(fTime + 0.5f /
					Distribution.fLookupTableTimeScale);
			}
		}
	}


	bool_t HasPortableAuthoredAutonomousEmission(
		const EFFECT_ELEMENT_DESC& Element)
	{
		if (std::ranges::any_of(Element.SourceRecipe.Bursts,
			[](const EFFECT_PARTICLE_BURST_DESC& Burst)
			{
				return Burst.iCountMaximum > 0u;
			}))
		{
			return true;
		}

		const EFFECT_SOURCE_MODULE_DESC* pSpawn = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (NormalizePortableParticleModuleClass(Module.strClassName) ==
				"particlemodulespawn")
			{
				pSpawn = &Module;
				break;
			}
		}
		if (nullptr == pSpawn)
			return false;
		const auto FindDistribution = [pSpawn](const std::string_view Property)
			-> const EFFECT_DISTRIBUTION_DESC*
		{
			const auto Iterator = std::ranges::find_if(pSpawn->Distributions,
				[Property](const EFFECT_DISTRIBUTION_DESC& Distribution)
				{
					return Distribution.strPropertyPath == Property;
				});
			return Iterator == pSpawn->Distributions.end() ? nullptr : &*Iterator;
		};
		const EFFECT_DISTRIBUTION_DESC* const pRate =
			FindDistribution("rate");
		const EFFECT_DISTRIBUTION_DESC* const pRateScale =
			FindDistribution("ratescale");
		if (nullptr == pRate || nullptr == pRateScale)
			return false;

		const f32_t fDurationSeconds = (std::max)(0.f,
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
				Element.SourceRecipe.fEmitterDurationSeconds :
				Element.Detail.Timing.fLifeTimeSeconds);
		std::vector<f32_t> ProbeTimes = {
			0.f, 0.5f * fDurationSeconds, fDurationSeconds };
		AppendPortableDistributionProbeTimes(
			*pRate, fDurationSeconds, ProbeTimes);
		if (!IsPortableNullCdoDistribution(*pRateScale))
		{
			AppendPortableDistributionProbeTimes(
				*pRateScale, fDurationSeconds, ProbeTimes);
		}
		std::ranges::sort(ProbeTimes);
		ProbeTimes.erase(std::unique(ProbeTimes.begin(), ProbeTimes.end()),
			ProbeTimes.end());

		constexpr std::array<f32_t, 3u> RandomUnits = { 0.f, 0.5f, 1.f };
		for (const f32_t fTime : ProbeTimes)
		{
			for (const f32_t fRateRandom : RandomUnits)
			{
				const f32_t fRate = CEffectDistribution::Evaluate(
					*pRate, fTime, fRateRandom).x;
				if (!std::isfinite(fRate) || fRate <= 0.f)
					continue;
				if (IsPortableNullCdoDistribution(*pRateScale))
					return true;
				for (const f32_t fScaleRandom : RandomUnits)
				{
					const f32_t fRateScale = CEffectDistribution::Evaluate(
						*pRateScale, fTime, fScaleRandom).x;
					if (std::isfinite(fRateScale) && fRateScale > 0.f)
						return true;
				}
			}
		}
		return false;
	}


	bool_t IsPortableVectorFieldAssetId(const std::string& strAssetId)
	{
		if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
			!strAssetId.starts_with("Effect/") ||
			strAssetId.find('\\') != std::string::npos ||
			strAssetId.find(':') != std::string::npos)
		{
			return false;
		}
		const std::filesystem::path RelativePath(strAssetId);
		if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
			RelativePath.lexically_normal().generic_string() != strAssetId ||
			RelativePath.extension() != ".wvectorfield")
		{
			return false;
		}
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Value = Component.generic_string();
			if (Value.empty() || Value == "." || Value == "..")
				return false;
		}
		const std::filesystem::path Resolved =
			CRuntimeAssetRoot::Resolve(RelativePath);
		std::error_code Error;
		return !Resolved.empty() &&
			std::filesystem::is_regular_file(Resolved, Error) && !Error;
	}


	const EFFECT_SOURCE_LITERAL_DESC* FindPortableSourceLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath)
	{
		const auto Iterator = std::ranges::find_if(Module.Literals,
			[strPropertyPath](const EFFECT_SOURCE_LITERAL_DESC& Literal)
			{
				return Literal.strPropertyPath == strPropertyPath;
			});
		return Iterator == Module.Literals.end() ? nullptr : &*Iterator;
	}


	bool_t ReadPortableBoolLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const bool_t bDefault,
		bool_t& bOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			bOutValue = bDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
			return false;
		bOutValue = pLiteral->bBoolean;
		return true;
	}


	bool_t ReadPortableNumberLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const f64_t fDefault,
		f64_t& fOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			fOutValue = fDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
			!std::isfinite(pLiteral->fNumber))
		{
			return false;
		}
		fOutValue = pLiteral->fNumber;
		return true;
	}


	bool_t ReadPortableStringLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const std::string_view strDefault,
		std::string_view& strOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			strOutValue = strDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING)
			return false;
		strOutValue = pLiteral->strString;
		return true;
	}


	bool_t ValidatePortableParticleModuleSemantics(
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strNormalizedClass,
		std::string& strOutError)
	{
		constexpr std::array<std::string_view, 6u> ExactClasses = {
			"particlemoduleeventgenerator",
			"particlemoduleeventreceiverspawn",
			"particlemoduleorbit",
			"particlemodulesizescale",
			"particlemodulevectorfieldscale",
			"particlemodulevelocityinheritparent"
		};
		if (std::ranges::find(ExactClasses, strNormalizedClass) !=
				ExactClasses.end() &&
			Module.strClassName != strNormalizedClass)
		{
			strOutError =
				"Portable authored particle module requires an exact Playback class identity: " +
				Module.strClassName + ".";
			return false;
		}

        if (strNormalizedClass == "particlemodulecollision")
        {
            bool World = false, ApplyPhysics = false, VerticalOnly = false;
            std::string_view Completion;
            f64_t Scalar = 1.0, Distance = 1000.0;
            if (Module.strClassName != strNormalizedClass ||
                !ReadPortableBoolLiteral(Module, "bcollidewithworld", false, World) || !World ||
                !ReadPortableBoolLiteral(Module, "bapplyphysics", false, ApplyPhysics) || ApplyPhysics ||
                !ReadPortableBoolLiteral(Module, "bonlyverticalnormalsdecrementcount", false, VerticalOnly) || VerticalOnly ||
                !ReadPortableStringLiteral(Module, "collisioncompletionoption", "epcc_kill", Completion) || Completion != "epcc_kill" ||
                !ReadPortableNumberLiteral(Module, "dirscalar", 1.0, Scalar) || Scalar < 0.0 ||
                !ReadPortableNumberLiteral(Module, "maxcollisiondistance", 1000.0, Distance) || Distance < 0.0)
            {
                strOutError = "Source Collision requires static-world bounce followed by Kill.";
                return false;
            }
        }
        else if (strNormalizedClass == "particlemodulekillheight" ||
            strNormalizedClass == "particlemoduleattractorpoint")
        {
            if (Module.strClassName != strNormalizedClass)
            {
                strOutError = "Source force module requires its exact class identity.";
                return false;
            }
            for (const auto& Literal : Module.Literals)
            {
                const auto& P = Literal.strPropertyPath;
                if ((P == "babsolute" || P == "bfloor" || P == "bapplypsysscale" ||
                     P == "baffectbasevelocity" || P == "strengthbydistance" ||
                     P == "buseworldspaceposition" || P == "boverridevelocity") &&
                    Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
                {
                    strOutError = "Source force module option must be a boolean.";
                    return false;
                }
            }
        }
        else if (strNormalizedClass == "particlemodulelocationcirclesurface")
		{
			std::string_view strAxis;
			f64_t fSplit = 0.0;
			bool_t bHalf = false;
			bool_t bNegative = false;
			bool_t bVelocity = false;
			bool_t bEnabled = true;
			if (Module.strClassName != "efparticlemodulelocationcirclesurface" ||
				!ReadPortableStringLiteral(Module, "surfaceaxis",
					"pmlcs_circle_axis_xy", strAxis) ||
				(strAxis != "pmlcs_circle_axis_xy" &&
				 strAxis != "pmlcs_circle_axis_yz" &&
				 strAxis != "pmlcs_circle_axis_zx") ||
				!ReadPortableNumberLiteral(Module, "splitcirclecount", 0.0,
					fSplit) || fSplit < 0.0 || fSplit != std::floor(fSplit) ||
				!ReadPortableBoolLiteral(Module, "bhalfmode", false, bHalf) ||
				!ReadPortableBoolLiteral(Module, "bnegativeaxis", false,
					bNegative) ||
				!ReadPortableBoolLiteral(Module, "velocity", false, bVelocity) ||
				!ReadPortableBoolLiteral(Module, "benabled", true, bEnabled))
			{
				strOutError =
					"Portable authored particle CircleSurface semantics are unsupported: " +
					Module.strClassName + ".";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleeventgenerator")
		{
			constexpr std::array<std::string_view, 9u> EventProperties = {
				"events[0].buseorbitoffset", "events[0].customname",
				"events[0].firsttimeonly", "events[0].frequency",
				"events[0].lasttimeonly", "events[0].lowfreq",
				"events[0].particlefrequency", "events[0].type",
				"events[0].usereflectedimpactvector"
			};
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath.starts_with("events[") &&
					std::ranges::find(EventProperties,
						Literal.strPropertyPath) == EventProperties.end())
				{
					strOutError =
						"Portable authored particle event generator payload is unsupported.";
					return false;
				}
			}
			std::string_view strType;
			std::string_view strName;
			f64_t fFrequency = 0.0;
			f64_t fParticleFrequency = 0.0;
			f64_t fLowFrequency = -1.0;
			bool_t bFirst = false;
			bool_t bLast = false;
			bool_t bReflected = false;
			bool_t bOrbit = false;
			if (!ReadPortableStringLiteral(Module, "events[0].type", "",
					strType) || (strType != "epet_spawn" && strType != "epet_death") ||
				!ReadPortableStringLiteral(Module, "events[0].customname", "",
					strName) || strName.empty() ||
				!ReadPortableNumberLiteral(Module, "events[0].frequency", 0.0,
					fFrequency) || fFrequency < 0.0 ||
				fFrequency != std::floor(fFrequency) ||
				!ReadPortableNumberLiteral(Module,
					"events[0].particlefrequency", 0.0,
					fParticleFrequency) || fParticleFrequency != 0.0 ||
				!ReadPortableNumberLiteral(Module, "events[0].lowfreq", -1.0,
					fLowFrequency) || fLowFrequency != -1.0 ||
				!ReadPortableBoolLiteral(Module, "events[0].firsttimeonly",
					false, bFirst) || bFirst ||
				!ReadPortableBoolLiteral(Module, "events[0].lasttimeonly",
					false, bLast) || bLast ||
				!ReadPortableBoolLiteral(Module,
					"events[0].usereflectedimpactvector", false,
					bReflected) || bReflected ||
				!ReadPortableBoolLiteral(Module, "events[0].buseorbitoffset",
					false, bOrbit) || bOrbit)
			{
				strOutError =
					"Portable authored particle Spawn/Death-event generator semantics are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleeventreceiverspawn")
		{
			std::string_view strType;
			std::string_view strName;
			bool_t bUseParticleTime = false;
			bool_t bInheritVelocity = false;
			bool_t bUseSystemLocation = false;
			if (!ReadPortableStringLiteral(Module, "eventgeneratortype", "",
					strType) || (strType != "epet_spawn" && strType != "epet_death") ||
				!ReadPortableStringLiteral(Module, "eventname", "", strName) ||
				strName.empty() ||
				!ReadPortableBoolLiteral(Module, "buseparticletime", false,
					bUseParticleTime) || bUseParticleTime ||
				!ReadPortableBoolLiteral(Module, "binheritvelocity", false,
					bInheritVelocity) ||
				!ReadPortableBoolLiteral(Module, "busepsyslocation", false,
					bUseSystemLocation) || bUseSystemLocation)
			{
				strOutError =
					"Portable authored particle Spawn/Death-event receiver semantics are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleorbit")
		{
			std::string_view strChainMode;
			if (!ReadPortableStringLiteral(Module, "chainmode",
					"eochainmode_add", strChainMode) ||
				(strChainMode != "eochainmode_add" &&
				 strChainMode != "eochainmode_link") ||
				std::ranges::any_of(Module.Literals,
					[](const EFFECT_SOURCE_LITERAL_DESC& Literal)
					{
						const std::string_view Path = Literal.strPropertyPath;
						if (!Path.starts_with("offsetoptions.") &&
							!Path.starts_with("rotationoptions.") &&
							!Path.starts_with("rotationrateoptions."))
						{
							return false;
						}
						// Offset has a per-tick base reset and can sample over life.
						// Variable rotation/rate updates still need their own phase state.
						if (Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
							return true;
						const std::string_view Option = Path.substr(Path.find('.') + 1u);
						if (Option == "bprocessduringspawn" || Option == "buseemittertime")
							return false;
						if (Option == "bprocessduringupdate")
							return Literal.bBoolean && !Path.starts_with("offsetoptions.");
						return true;
					}))
			{
				strOutError =
					"Portable authored particle Orbit chain/options are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduletypedatadecal")
		{
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				const bool_t bNumber =
					Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::NUMBER;
				const bool_t bBoolean =
					Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::BOOLEAN;
				const bool_t bSupported =
					((Literal.strPropertyPath == "lodvalidity" ||
					  Literal.strPropertyPath == "nearplane" ||
					  Literal.strPropertyPath == "farplane") && bNumber) ||
					(Literal.strPropertyPath == "balwaysdecalupdate" &&
					 bBoolean) ||
					(Literal.strPropertyPath == "rotation.degrees.roll" && bNumber &&
					 Module.strClassName == "efparticlemoduletypedatadecal" &&
					 Element.Material.SourceMaterial.strRuntimeShaderProfileId.starts_with("effect.ue3.kouku-") &&
					 std::isfinite(Literal.fNumber) && std::abs(Literal.fNumber) <= 3600.0);
				if (!bSupported)
				{
					strOutError =
						"Portable authored decal TypeData has an unsupported literal: " +
						Literal.strPropertyPath + ".";
					return false;
				}
			}
		}
		else if (strNormalizedClass == "particlemodulevortex")
		{
			f64_t fPower = 1.0;
			if (Module.strClassName != "efparticlemodulevortex" ||
				!ReadPortableNumberLiteral(Module, "power", 1.0, fPower))
			{
				strOutError =
					"Portable authored particle Vortex semantics are unsupported.";
				return false;
			}
		}
		if (strNormalizedClass == "particlemodulerequired")
		{
			std::string_view Mode;
			if (!ReadPortableStringLiteral(Module, "interpolationmethod", "", Mode))
				return false;
			if (Mode == "psuvim_random")
			{
				double Columns, Rows, Changes, Interval;
				bool_t ScaleUV;
				if (!ReadPortableNumberLiteral(Module, "subimages_horizontal", 1.0, Columns) ||
					!ReadPortableNumberLiteral(Module, "subimages_vertical", 1.0, Rows) ||
					!ReadPortableNumberLiteral(Module, "randomimagechanges", 0.0, Changes) ||
					!ReadPortableNumberLiteral(Module, "randomimagetime", 1.0, Interval) ||
					!ReadPortableBoolLiteral(Module, "bscaleuv", false, ScaleUV) ||
					Columns < 1.0 || Rows < 1.0 || Columns > 256.0 || Rows > 256.0 ||
					std::floor(Columns) != Columns || std::floor(Rows) != Rows ||
					Changes < 0.0 || Changes > 65535.0 || std::floor(Changes) != Changes ||
					Interval < 0.0 || Interval > 1.0)
				{
					strOutError = "Source random SubUV layout or relative-age interval is invalid.";
					return false;
				}
			}
		}
		if (strNormalizedClass == "particlemodulesubuvmovie")
		{
			double StartingFrame = 1.0;
			bool_t UseEmitterTime = false, UseRealTime = false;
			if (!ReadPortableNumberLiteral(Module, "startingframe", 1.0, StartingFrame) ||
				!ReadPortableBoolLiteral(Module, "buseemittertime", false, UseEmitterTime) ||
				!ReadPortableBoolLiteral(Module, "brealtime", false, UseRealTime) ||
				UseRealTime || StartingFrame < 0.0 || StartingFrame > 65536.0 ||
				std::floor(StartingFrame) != StartingFrame)
			{
				strOutError = "Source SubUV Movie requires a valid starting frame and simulation time.";
				return false;
			}
		}
		if (strNormalizedClass == "particlemodulesizemultiplyvelocity")
		{
			bool_t Value = false;
			double Number = 0.0;
			for (const auto Path : { "multiplyx", "multiplyy", "multiplyz",
				"bspawnmodule", "bupdatemodule" })
				if (!ReadPortableBoolLiteral(Module, Path, true, Value))
				{
					strOutError = "SizeMultiplyVelocity has an invalid axis or execution flag.";
					return false;
				}
			for (const auto Path : { "capminsize.x", "capminsize.y", "capminsize.z",
				"capmaxsize.x", "capmaxsize.y", "capmaxsize.z" })
				if (!ReadPortableNumberLiteral(Module, Path, 0.0, Number))
				{
					strOutError = "SizeMultiplyVelocity has an invalid size cap.";
					return false;
				}
			if (Module.Distributions.size() != 1u ||
				Module.Distributions.front().iComponentCount != 3u)
			{
				strOutError = "SizeMultiplyVelocity requires one vector3 multiplier.";
				return false;
			}
		}
		strOutError.clear();
		return true;
	}


	bool_t ValidatePortableAuthoredParticleRuntimeCarrier(
		const EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const size_t iMeshBindingCount = static_cast<size_t>(std::count_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			}));
		const bool_t bMesh = Element.SourceRecipe.strRendererShape == "mesh";
		const bool_t bSprite = Element.SourceRecipe.strRendererShape == "sprite";
		const bool_t bDecal = Element.SourceRecipe.strRendererShape == "decal";
		const bool_t bRibbon = Element.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
            Element.SourceRecipe.strRendererShape == "ribbon" && Element.RuntimeCarrier.eKind ==
                EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1;
        const bool_t bFamilyValid =
			(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				(bMesh || bSprite)) ||
			(Element.eKind == EFFECT_ELEMENT_KIND::DECAL && bDecal) || bRibbon;
		if (!bFamilyValid ||
			Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			!Element.SourceRecipe.bEnabled ||
			(bMesh ? iMeshBindingCount != 1u : iMeshBindingCount != 0u) ||
			!std::isfinite(Element.SourceRecipe.fEmitterDelaySeconds) ||
			Element.SourceRecipe.fEmitterDelaySeconds < 0.f ||
			Element.SourceRecipe.fEmitterDelaySeconds > 300.f ||
			Element.SourceRecipe.Modules.empty())
		{
			strOutError =
				"Portable authored emitter carrier identity, Family, or schedule is invalid.";
			return false;
		}

		std::unordered_set<std::string> ModuleIds;
		std::unordered_map<std::string, size_t> ModuleClassCounts;
		size_t iRequiredCount = 0u;
		size_t iMeshTypeDataCount = 0u;
		size_t iDecalTypeDataCount = 0u;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			const std::string_view NormalizedClass =
				NormalizePortableParticleModuleClass(Module.strClassName);
			const bool_t bAdmittedDecalTypeData = bDecal &&
				NormalizedClass == "particlemoduletypedatadecal";
			// Its static slot array is consumed by the existing CModel material
			// stage; it contributes no second simulation or distribution state.
			const bool_t bAdmittedMeshMaterial = bMesh &&
				!Element.Detail.Mesh.SourceMaterialSlots.empty() &&
				Module.strClassName == "particlemodulemeshmaterial";
			const bool_t bAdmittedRibbonTypeData = bRibbon && NormalizedClass == "particlemoduletypedataribbon";
            if ((!bAdmittedDecalTypeData && !bAdmittedMeshMaterial && !bAdmittedRibbonTypeData && std::ranges::find(
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES,
					NormalizedClass) ==
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES.end()) ||
				Module.strStableId.empty() ||
				!ModuleIds.insert(Module.strStableId).second ||
				(NormalizedClass == "particlemodulerequired" &&
				 Module.strClassName != "particlemodulerequired") ||
				(NormalizedClass == "particlemodulespawn" &&
				 Module.strClassName != "particlemodulespawn") ||
				(NormalizedClass == "particlemoduletypedatamesh" &&
				 Module.strClassName != "particlemoduletypedatamesh") ||
				(NormalizedClass == "particlemoduletypedatadecal" &&
				 Module.strClassName != "efparticlemoduletypedatadecal"))
			{
				strOutError =
					"Portable authored particle carrier has an unsupported or duplicate module: " +
					Module.strClassName + ".";
				return false;
			}
			if (!ValidatePortableParticleModuleSemantics(
					Element, Module, NormalizedClass, strOutError))
			{
				return false;
			}
			iRequiredCount +=
				NormalizedClass == "particlemodulerequired" ? 1u : 0u;
			iMeshTypeDataCount +=
				NormalizedClass == "particlemoduletypedatamesh" ? 1u : 0u;
			iDecalTypeDataCount +=
				NormalizedClass == "particlemoduletypedatadecal" ? 1u : 0u;
			bool_t MissingSourceProvider = false;
			const bool_t IsLocationEmitter = NormalizedClass == "particlemodulelocationemitter" ||
				NormalizedClass == "particlemodulelocationemitterdirect";
			if (IsLocationEmitter && !ReadPortableBoolLiteral(Module, "runtime.sourceprovidermissing", false, MissingSourceProvider))
			{ strOutError = "Missing source-provider marker must be boolean."; return false; }
			if (!MissingSourceProvider) ++ModuleClassCounts[std::string(NormalizedClass)];
			std::unordered_set<std::string> PropertyPaths;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (!PropertyPaths.insert(Literal.strPropertyPath).second)
				{
					strOutError =
						"Portable authored particle carrier has a duplicate module property: " +
						Module.strClassName + "/" + Literal.strPropertyPath + ".";
					return false;
				}
			}
			if (NormalizedClass == "particlemodulelocalvectorfield")
			{
				const auto AssetLiteral = std::find_if(
					Module.Literals.begin(), Module.Literals.end(),
					[](const EFFECT_SOURCE_LITERAL_DESC& Literal)
					{
						return Literal.strPropertyPath == "vectorfield.assetid";
					});
				if (AssetLiteral == Module.Literals.end() ||
					AssetLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING ||
					!IsPortableVectorFieldAssetId(AssetLiteral->strString))
				{
					strOutError =
						"Portable authored particle local vector field asset is missing or unsafe.";
					return false;
				}
			}
			const std::string_view DistributionCapabilityClass =
				PortableParticleDistributionCapabilityClass(
					Module.strClassName, NormalizedClass);
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				const bool_t bNativeEvidence =
					!Distribution.strReferenceId.empty() ||
					!Distribution.strOccurrenceId.empty() ||
					!Distribution.strPayloadStatus.empty() ||
					!Distribution.strFidelity.empty() ||
					Distribution.ExecutionAdmission.bAllowed ||
					!Distribution.ExecutionAdmission.Blockers.empty() ||
					Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					!Distribution.strParameterName.empty();
				const bool_t bIgnoredNullCdo =
					NormalizedClass == "particlemodulerequired" &&
					Distribution.strPropertyPath == "spawnrate";
				if (bNativeEvidence ||
					!IsPortableAuthoredParticleDistributionProperty(
						DistributionCapabilityClass,
						Distribution.strPropertyPath) ||
					(bIgnoredNullCdo &&
					 !IsPortableNullCdoDistribution(Distribution)) ||
					!PropertyPaths.insert(Distribution.strPropertyPath).second)
				{
					strOutError =
						"Portable authored particle carrier has native evidence or a duplicate distribution: " +
						Module.strClassName + "/" +
						Distribution.strPropertyPath + ".";
					return false;
				}
			}
			const size_t iExpectedDistributionCount =
				static_cast<size_t>(std::count_if(
					PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.begin(),
					PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.end(),
					[DistributionCapabilityClass](const auto& Capability)
					{
						return Capability.first ==
							DistributionCapabilityClass;
					}));
			if (Module.Distributions.size() != iExpectedDistributionCount)
			{
				strOutError =
					"Portable authored particle carrier distribution capability is incomplete: " +
					Module.strClassName + ".";
				return false;
			}
		}
		const auto CountClass = [&ModuleClassCounts](
			const std::string_view ClassName)
		{
			const auto Iterator = ModuleClassCounts.find(std::string(ClassName));
			return Iterator == ModuleClassCounts.end() ? 0u : Iterator->second;
		};
        if (CountClass("particlemoduleorbit") > 1u)
        {
            for (const auto& Module : Element.SourceRecipe.Modules)
            {
                if (Module.strClassName != "particlemoduleorbit") continue;
                for (const auto& Literal : Module.Literals)
                    if (Literal.strPropertyPath.ends_with(".bprocessduringupdate") &&
                        Literal.bBoolean)
                    {
                        strOutError = "Multiple source Orbit chains require spawn-owned phases.";
                        return false;
                    }
            }
        }
		for (const auto& [ClassName, Count] : ModuleClassCounts)
		{
			/* UE3 can retain renderer-irrelevant modules in an emitter.  The
			   portable runtime keeps their source order, but Sprite drawing never
			   consumes MeshRotation state and Mesh drawing never consumes
			   OrientationAxisLock presentation.  TypeDataMesh remains the actual
			   renderer-Family discriminator. */
			const bool_t bMeshOnly =
				ClassName == "particlemoduletypedatamesh";
			const bool_t bDecalOnly =
				ClassName == "particlemoduletypedatadecal";
			const bool_t bSpriteOnly =
				ClassName == "particlemodulerotationratemultiplylife" ||
				((ClassName == "particlemodulesubuv" || ClassName == "particlemodulesubuvmovie") && !(bMesh &&
					(Has_ArtistMaterialContract(Element) || Has_WarlordNativeMaterialContract(Element) ||
						Has_LanceMasterVAMaterialContract(Element))));
			const auto Maximum = std::ranges::find_if(
				PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS,
				[&ClassName](const auto& Capability)
				{
					return Capability.first == ClassName;
				});
			const size_t iMaximum =
				Maximum == PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS.end() ?
				1u : Maximum->second;
			if ((bMeshOnly && !bMesh) || (bDecalOnly && !bDecal) ||
				(bSpriteOnly && !bSprite) ||
				Count > iMaximum)
			{
				strOutError =
					"Portable authored particle carrier module Family/cardinality is unsupported: " +
					ClassName + ".";
				return false;
			}
		}
		const bool_t bParticleCardinalityValid =
			(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE || bRibbon) &&
			iRequiredCount == 1u &&
			CountClass("particlemodulelifetime") != 0u &&
			CountClass("particlemodulespawn") == 1u &&
			(bMesh ? iMeshTypeDataCount == 1u : iMeshTypeDataCount == 0u) &&
			iDecalTypeDataCount == 0u && (!bRibbon || CountClass("particlemoduletypedataribbon") == 1u);
		const bool_t bDecalCardinalityValid =
			bDecal && iRequiredCount == 1u &&
			CountClass("particlemodulelifetime") <= 1u &&
			CountClass("particlemodulespawn") <= 1u &&
			iMeshTypeDataCount == 0u && iDecalTypeDataCount == 1u;
		if (!bParticleCardinalityValid && !bDecalCardinalityValid)
		{
			strOutError =
				"Portable authored emitter carrier Required/Lifetime/Spawn/TypeData cardinality is invalid.";
			return false;
		}
		const size_t iLocalVectorFieldCount =
			CountClass("particlemodulelocalvectorfield");
		if ((CountClass("particlemodulevectorfieldrotationrate") != 0u ||
			 CountClass("particlemodulevectorfieldscale") != 0u ||
			 CountClass("particlemodulevectorfieldscaleoverlife") != 0u) &&
			iLocalVectorFieldCount != 1u)
		{
			strOutError =
				"Portable authored particle vector field companion has no unique local field.";
			return false;
		}
		strOutError.clear();
		return true;
	}


	bool_t ValidatePortableAuthoredParticleEventRoutes(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		std::unordered_map<std::string, std::vector<std::string>> Generators;
		std::unordered_map<std::string, std::vector<std::string>> Receivers;
		uint64_t iMaximumQueuedEvents = 0u;
		const auto RouteKey = [](const std::string_view strType,
			const std::string_view strName)
		{
			std::string Result(strType);
			Result.push_back('\0');
			Result.append(strName);
			return Result;
		};
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Element.bVisible ||
				!Is_EffectAuthoringExecutionTarget(
					Element.Material.Execution) ||
				(Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
				 Element.eKind != EFFECT_ELEMENT_KIND::DECAL) ||
				Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
				Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
				!Element.SourceRecipe.bEnabled ||
				((Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				  Element.SourceRecipe.strRendererShape != "mesh" &&
				  Element.SourceRecipe.strRendererShape != "sprite") ||
				 (Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
				  Element.SourceRecipe.strRendererShape != "decal")))
			{
				continue;
			}
			uint32_t iGeneratorCount = 0u;
			for (const EFFECT_SOURCE_MODULE_DESC& Module :
				Element.SourceRecipe.Modules)
			{
				bool_t bEnabled = true;
				if (!ReadPortableBoolLiteral(
						Module, "benabled", true, bEnabled))
				{
					strOutError =
						"Portable authored particle event module enabled state is invalid.";
					return false;
				}
				if (!bEnabled)
					continue;
				const std::string_view strClass =
					NormalizePortableParticleModuleClass(Module.strClassName);
				if (strClass == "particlemoduleeventgenerator")
				{
					std::string_view strType;
					std::string_view strName;
					if (!ReadPortableStringLiteral(Module, "events[0].type",
							"", strType) || (strType != "epet_spawn" && strType != "epet_death") ||
						!ReadPortableStringLiteral(Module,
							"events[0].customname", "", strName) ||
						strName.empty())
					{
						strOutError =
							"Portable authored particle event generator route identity is invalid.";
						return false;
					}
					Generators[RouteKey(strType, strName)].push_back(
						Element.strElementId);
					++iGeneratorCount;
				}
				else if (strClass == "particlemoduleeventreceiverspawn")
				{
					std::string_view strType;
					std::string_view strName;
					if (!ReadPortableStringLiteral(Module,
							"eventgeneratortype", "", strType) ||
						(strType != "epet_spawn" && strType != "epet_death") ||
						!ReadPortableStringLiteral(Module, "eventname", "",
							strName) || strName.empty())
					{
						strOutError =
							"Portable authored particle event receiver route identity is invalid.";
						return false;
					}
					Receivers[RouteKey(strType, strName)].push_back(
						Element.strElementId);
				}
			}
			iMaximumQueuedEvents += SourceScaledParticleCeiling(Element) *
				static_cast<uint64_t>(iGeneratorCount);
		}

		if (iMaximumQueuedEvents > MAX_PORTABLE_SOURCE_EVENTS_PER_STEP)
		{
			strOutError =
				"Portable authored particle event queue has an unbounded per-step upper limit.";
			return false;
		}
		for (const auto& [strRoute, SourceElements] : Generators)
		{
			(void)SourceElements;
			if (!Receivers.contains(strRoute))
			{
				strOutError =
					"Portable authored particle event generator has no same-document receiver.";
				return false;
			}
		}
		for (const auto& [strRoute, TargetElements] : Receivers)
		{
			(void)TargetElements;
			if (!Generators.contains(strRoute))
			{
				strOutError =
					"Portable authored particle event receiver has no same-document generator.";
				return false;
			}
		}

		std::unordered_map<std::string, std::vector<std::string>> Adjacency;
		for (const auto& [strRoute, SourceElements] : Generators)
		{
			const std::vector<std::string>& TargetElements =
				Receivers.at(strRoute);
			for (const std::string& strSourceElement : SourceElements)
			{
				auto& Targets = Adjacency[strSourceElement];
				Targets.insert(Targets.end(), TargetElements.begin(),
					TargetElements.end());
			}
		}
		std::unordered_map<std::string, uint8_t> VisitStates;
		const auto Visit = [&](const auto& Self,
			const std::string& strElementId) -> bool_t
		{
			uint8_t& iState = VisitStates[strElementId];
			if (1u == iState)
			{
				strOutError =
					"Portable authored particle event route cycle is not allowed.";
				return false;
			}
			if (2u == iState)
				return true;
			iState = 1u;
			const auto Iterator = Adjacency.find(strElementId);
			if (Iterator != Adjacency.end())
			{
				for (const std::string& strTarget : Iterator->second)
				{
					if (!Self(Self, strTarget))
						return false;
				}
			}
			iState = 2u;
			return true;
		};
		for (const auto& [strElementId, Targets] : Adjacency)
		{
			(void)Targets;
			if (!Visit(Visit, strElementId))
				return false;
		}
		strOutError.clear();
		return true;
	}

	bool_t ApplyPortableAuthoredEmitterRuntimeCarrier(
		const Client::EFFECT_ELEMENT_DESC& SourceElement,
		Client::EFFECT_ELEMENT_DESC& InOutElement,
		const Client::EFFECT_ELEMENT_KIND eExpectedKind,
		const std::string_view strExpectedShape,
		std::string& strOutError)
	{
		using namespace Client;
		if (SourceElement.eKind != eExpectedKind ||
			InOutElement.eKind != eExpectedKind ||
			!SourceElement.SourceRecipe.bEnabled ||
			SourceElement.SourceRecipe.strRendererShape != strExpectedShape)
		{
			strOutError =
				"Portable authored emitter carrier source/target Family does not match.";
			return false;
		}

		/* Legacy v13 sourceRecipe is the portable, already-interpreted runtime
		   carrier. Constructing a fresh descriptor guarantees that native-v14
		   contract hashes, compiler evidence, authority receipts, geometry
		   admission, and local-reference closure cannot cross this seam. */
		EFFECT_CASCADE_RECIPE_DESC Portable;
		Portable.bEnabled = true;
		Portable.strRendererShape = SourceElement.SourceRecipe.strRendererShape;
		/* Generic occurrence import samples/bakes its starting state before this
		   helper and therefore needs a flattened zero delay. Saved Element reuse
		   restores the source delay explicitly after this carrier is admitted. */
		Portable.fEmitterDelaySeconds = 0.f;
		Portable.fEmitterDurationSeconds =
			SourceElement.SourceRecipe.fEmitterDurationSeconds;
		Portable.iEmitterLoopCount =
			SourceElement.SourceRecipe.iEmitterLoopCount;
		Portable.Bursts = SourceElement.SourceRecipe.Bursts;
		Portable.Modules = SourceElement.SourceRecipe.Modules;
		for (EFFECT_SOURCE_MODULE_DESC& Module : Portable.Modules)
		{
			const std::string_view NormalizedClass =
				NormalizePortableParticleModuleClass(Module.strClassName);
			const bool_t bAdmittedDecalTypeData =
				eExpectedKind == EFFECT_ELEMENT_KIND::DECAL &&
				strExpectedShape == "decal" &&
				NormalizedClass == "particlemoduletypedatadecal";
			const bool_t bAdmittedMeshMaterial =
				eExpectedKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				strExpectedShape == "mesh" &&
				!SourceElement.Detail.Mesh.SourceMaterialSlots.empty() &&
				Module.strClassName == "particlemodulemeshmaterial";
			if ((!bAdmittedDecalTypeData && !bAdmittedMeshMaterial && std::ranges::find(
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES,
					NormalizedClass) ==
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES.end()) ||
				(NormalizedClass == "particlemodulespawn" &&
				 Module.strClassName != "particlemodulespawn"))
			{
				strOutError =
					"Portable authored emitter carrier has an unsupported module class: " +
					Module.strClassName + ".";
				return false;
			}
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			{
				if (Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					!Distribution.strParameterName.empty())
				{
					strOutError =
						"Portable authored emitter carrier cannot erase an ActionCue parameter binding: " +
						Module.strClassName + "/" +
						Distribution.strPropertyPath + ".";
					return false;
				}
				Distribution.strReferenceId.clear();
				Distribution.strOccurrenceId.clear();
				Distribution.strPayloadStatus.clear();
				Distribution.strFidelity.clear();
				Distribution.ExecutionAdmission = {};
				Distribution.strParameterName.clear();
				Distribution.eParameterBinding =
					EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
			}
		}
		EFFECT_ELEMENT_DESC Staged = InOutElement;
		Staged.SourceRecipe = std::move(Portable);
		if (!ValidatePortableAuthoredParticleRuntimeCarrier(
				Staged, strOutError))
		{
			return false;
		}
		if (!HasPortableAuthoredAutonomousEmission(Staged))
		{
			/* SpawnPerUnit and source events are multi-occurrence/history
			   contracts.  A single copied Element must own a positive burst or a
			   concretely evaluable positive Rate; otherwise a valid document can
			   stage successfully while emitting no drawable at a static preview
			   root. */
			strOutError =
				"Portable authored emitter carrier has no autonomous positive Burst or Rate; SpawnPerUnit/event/history-only emitters require the complete Effect.";
			return false;
		}
		InOutElement.SourceRecipe = std::move(Staged.SourceRecipe);
		strOutError.clear();
		return true;
	}


	bool_t Validate_AuthoredRuntimeExtensions(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		using namespace Client;
		const bool_t bRuntimeExtensionDocument =
			Document.iLoadedFormatVersion ==
				EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION;
		if (!bRuntimeExtensionDocument)
		{
			if (Document.RuntimeExtensions.iFormatVersion !=
					EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
				!Document.RuntimeExtensions.Is_Empty() ||
				std::any_of(Document.Elements.begin(), Document.Elements.end(),
					[](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.RuntimeCarrier.iFormatVersion !=
								EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
							!Element.RuntimeCarrier.Is_Empty();
					}))
			{
				strOutError =
					"Only authored-v15 Effect documents may carry runtimeExtensions/runtimeCarrier data.";
				return false;
			}
			return true;
		}

		if (Document.RuntimeExtensions.iFormatVersion !=
				EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
			Document.RuntimeExtensions.BakedEdgeHistories.size() >
				MAX_AUTHORED_RUNTIME_EDGE_HISTORIES)
		{
			strOutError = "Authored runtimeExtensions version/size is invalid.";
			return false;
		}

		std::unordered_map<std::string,
			const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC*> HistoriesById;
		std::string strPreviousHistoryId;
		size_t iTotalSampleCount = 0u;
		for (const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& History :
			Document.RuntimeExtensions.BakedEdgeHistories)
		{
			if (!Is_StableId(History.strHistoryId) ||
				(!strPreviousHistoryId.empty() &&
				 History.strHistoryId <= strPreviousHistoryId) ||
				History.eCoordinateBasis !=
					EFFECT_AUTHORED_RUNTIME_COORDINATE_BASIS::
						UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS ||
				!std::isfinite(History.fSourceEndTimeSeconds) ||
				History.fSourceEndTimeSeconds <= 0.f ||
				History.fSourceEndTimeSeconds >
					MAX_AUTHORED_RUNTIME_EDGE_TIME_SECONDS ||
				!std::isfinite(History.fPlaybackClampSeconds) ||
				History.fPlaybackClampSeconds <= 0.f ||
				History.fPlaybackClampSeconds >
					History.fSourceEndTimeSeconds ||
				History.Samples.size() < 2u ||
				History.Samples.size() >
					MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_PER_HISTORY ||
				!HistoriesById.emplace(
					History.strHistoryId, &History).second)
			{
				strOutError =
					"Authored baked-edge history identity/timing is invalid.";
				return false;
			}
			strPreviousHistoryId = History.strHistoryId;
			iTotalSampleCount += History.Samples.size();
			if (iTotalSampleCount > MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_TOTAL)
			{
				strOutError =
					"Authored baked-edge history sample budget is exceeded.";
				return false;
			}

			f32_t fPreviousTime = -1.f;
			for (const EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC& Sample :
				History.Samples)
			{
				const auto CoordinatesBounded = [](const float3_t& Coordinates)
				{
					return Is_Finite(Coordinates) &&
						std::abs(Coordinates.x) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.y) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.z) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM;
				};
				if (!std::isfinite(Sample.fRelativeTimeSeconds) ||
					Sample.fRelativeTimeSeconds <= fPreviousTime ||
					Sample.fRelativeTimeSeconds < 0.f ||
					Sample.fRelativeTimeSeconds >
						History.fSourceEndTimeSeconds + 5.0e-5f ||
					!CoordinatesBounded(Sample.vFirstEdgeUE3Cm) ||
					!CoordinatesBounded(Sample.vControlPointUE3Cm) ||
					!CoordinatesBounded(Sample.vSecondEdgeUE3Cm))
				{
					strOutError = "Authored baked-edge sample is invalid.";
					return false;
				}
				fPreviousTime = Sample.fRelativeTimeSeconds;
			}
			if (std::abs(History.Samples.front().fRelativeTimeSeconds) >
					1.0e-6f ||
				std::abs(History.Samples.back().fRelativeTimeSeconds -
					History.fSourceEndTimeSeconds) > 5.0e-5f)
			{
				strOutError =
					"Authored baked-edge history does not close at its declared source interval.";
				return false;
			}
		}

		std::unordered_set<std::string> ReferencedHistoryIds;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
				Element.RuntimeCarrier;
			if (Carrier.Is_Empty())
				continue;
			if (Carrier.iFormatVersion !=
					EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
				Carrier.eKind >= EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::END ||
				Carrier.eAdmission !=
					EFFECT_AUTHORED_RUNTIME_CARRIER_ADMISSION::BOUNDED ||
				!Element.bVisible ||
				(!Is_EffectAuthoringExecutionTarget(Element.Material.Execution) &&
				 !Is_EffectPresentationExecutionTarget(Element)))
			{
				strOutError =
					"Authored runtimeCarrier target is not a visible bounded drawable.";
				return false;
			}

			switch (Carrier.eKind)
			{
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1:
			{
				if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
					!Carrier.strHistoryId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
					Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.strTypeDataModuleStableId.size() > 256u ||
					!Has_VisibleCharacter(
						Carrier.strTypeDataModuleStableId) ||
					!Element.SourceRecipe.bEnabled ||
					Element.SourceRecipe.strRendererShape !=
                        (Carrier.eKind == EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1 ? "beam" : "ribbon"))
				{
					strOutError =
						"Authored Cascade runtimeCarrier target/shape is invalid.";
					return false;
				}
				size_t iTypeDataMatchCount = 0u;
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					Element.SourceRecipe.Modules)
				{
					if (Module.strStableId !=
						Carrier.strTypeDataModuleStableId)
					{
						continue;
					}
					++iTypeDataMatchCount;
					if (Normalize_SourceModuleClass(Module.strClassName) !=
						(Carrier.eKind == EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1 ?
                            "particlemoduletypedatabeam2" : "particlemoduletypedataribbon"))
					{
						strOutError =
							"Authored Cascade runtimeCarrier joins a non-Ribbon TypeData module.";
						return false;
					}
				}
                if (Carrier.eKind == EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1)
                {
                    size_t targets = 0u;
                    for (const auto& module : Element.SourceRecipe.Modules)
                    {
                        const auto cls = Normalize_SourceModuleClass(module.strClassName);
                        if (cls == "particlemodulebeamtarget")
                        {
                            ++targets;
                            const auto target = std::ranges::find_if(module.Distributions,
                                [](const auto& d) { return d.strPropertyPath == "target" && d.iComponentCount == 3u; });
                            if (target == module.Distributions.end())
                            { strOutError = "Target Beam2 requires the original target distribution."; return false; }
                            bool_t absolute = false;
                            std::string_view method;
                            if (!ReadPortableBoolLiteral(module,"btargetabsolute",false,absolute) || absolute ||
                                !ReadPortableStringLiteral(module,"targetmethod","peb2stm_default",method) ||
                                method != "peb2stm_default")
                            { strOutError = "Target Beam2 requires emitter-local distribution targets."; return false; }
                        }
                        else if (cls == "particlemoduletypedatabeam2")
                        {
                            double count, sheets, points, speed, textureTile;
                            std::string_view method;
                            if (!ReadPortableStringLiteral(module,"beammethod","",method) || method != "peb2m_target" ||
                                !ReadPortableNumberLiteral(module,"maxbeamcount",1,count) || count != 1 ||
                                !ReadPortableNumberLiteral(module,"sheets",1,sheets) || sheets != 1 ||
                                !ReadPortableNumberLiteral(module,"interpolationpoints",0,points) || points != 0 ||
                                !ReadPortableNumberLiteral(module,"speed",0,speed) || speed < 0 ||
                                !ReadPortableNumberLiteral(module,"texturetile",1,textureTile) || textureTile != 1)
                            { strOutError = "Target Beam2 source count/sheets/interpolation/speed is unsupported."; return false; }
                        }
                        else if (cls == "particlemodulebeamnoise" || cls == "particlemodulebeamsource" ||
                                 cls == "particlemodulebeammodifier")
                        { strOutError = "Target Beam2 source/noise/modifier needs its own source consumer."; return false; }
                    }
                    if (targets != 1u)
                    { strOutError = "Target Beam2 requires one unambiguous target module."; return false; }
                }
				if (1u != iTypeDataMatchCount)
				{
					strOutError =
						"Authored Cascade runtimeCarrier TypeData stable join is missing or ambiguous.";
					return false;
				}
				break;
			}
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				ANIMATION_TRAIL_BAKED_EDGE_V1:
				if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
					!Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
					!Is_StableId(Carrier.strHistoryId) ||
					!HistoriesById.contains(Carrier.strHistoryId))
				{
					strOutError =
						"Authored Animation Trail runtimeCarrier history join is invalid.";
					return false;
				}
				ReferencedHistoryIds.insert(Carrier.strHistoryId);
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				LIGHT_BAKED_EDGE_ATTACHMENT_V1:
				if (Element.eKind != EFFECT_ELEMENT_KIND::LIGHT ||
					!Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::FIRST_EDGE ||
					!Is_StableId(Carrier.strHistoryId) ||
					!HistoriesById.contains(Carrier.strHistoryId) ||
					!Element.Detail.Light.bEnabled)
				{
					strOutError =
						"Authored Light runtimeCarrier history/lane target is invalid.";
					return false;
				}
				ReferencedHistoryIds.insert(Carrier.strHistoryId);
				break;
			default:
				strOutError = "Authored runtimeCarrier kind is invalid.";
				return false;
			}
		}

		if (ReferencedHistoryIds.size() != HistoriesById.size())
		{
			strOutError =
				"Authored runtimeExtensions contain an unreferenced baked-edge history.";
			return false;
		}
		return true;
	}

}


bool_t Client::CEffectDocumentCodec::
	Apply_PortableAuthoredParticleRuntimeCarrier(
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError)
{
	const bool_t bTargetMeshParticle = std::any_of(
		InOutElement.ResourceBindings.begin(),
		InOutElement.ResourceBindings.end(),
		[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
		});
	return ApplyPortableAuthoredEmitterRuntimeCarrier(SourceElement,
		InOutElement, EFFECT_ELEMENT_KIND::PARTICLE,
		bTargetMeshParticle ? "mesh" : "sprite", strOutError);
}


bool_t Client::CEffectDocumentCodec::
	Apply_PortableAuthoredDecalRuntimeCarrier(
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError)
{
	return ApplyPortableAuthoredEmitterRuntimeCarrier(SourceElement,
		InOutElement, EFFECT_ELEMENT_KIND::DECAL, "decal", strOutError);
}
