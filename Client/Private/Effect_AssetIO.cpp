#include "Effect_AssetIO.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
	using namespace Client;

	constexpr uint32_t WEFFECT_MAGIC = 0x54434645u; // EFCT
	constexpr uint32_t LEGACY_EFFECT_ASSET_SCHEMA_VERSION = 4;

	// Load_Binary rejects any payload larger than this. Save_Binary uses the
	// same limit so cooking can never produce a file that cannot be reloaded.
	constexpr size_t WEFFECT_MAX_PAYLOAD_BYTES = 64ull * 1024ull * 1024ull;

	bool IsSupportedSchemaVersion(const uint32_t iSchemaVersion)
	{
		return LEGACY_EFFECT_ASSET_SCHEMA_VERSION == iSchemaVersion ||
			EFFECT_ASSET_SCHEMA_VERSION == iSchemaVersion;
	}

	struct BYTE_WRITER
	{
		vector<uint8_t> Bytes;

		template<typename T>
		void Pod(const T& Value)
		{
			const uint8_t* pData =
				reinterpret_cast<const uint8_t*>(&Value);
			Bytes.insert(Bytes.end(), pData, pData + sizeof(T));
		}

		void String(const string& Value)
		{
			const uint32_t iLength =
				static_cast<uint32_t>(Value.size());
			Pod(iLength);
			Bytes.insert(Bytes.end(), Value.begin(), Value.end());
		}
	};

	struct BYTE_READER
	{
		const vector<uint8_t>& Bytes;
		size_t Offset = {};

		template<typename T>
		bool Pod(T& OutValue)
		{
			if (Offset + sizeof(T) > Bytes.size())
				return false;
			memcpy(&OutValue, Bytes.data() + Offset, sizeof(T));
			Offset += sizeof(T);
			return true;
		}

		bool String(string& OutValue)
		{
			uint32_t iLength = {};
			if (!Pod(iLength) || Offset + iLength > Bytes.size())
				return false;
			OutValue.assign(
				reinterpret_cast<const char*>(Bytes.data() + Offset),
				iLength);
			Offset += iLength;
			return true;
		}
	};

	void WriteFloatDistribution(BYTE_WRITER& Writer,
		const EFFECT_DISTRIBUTION_FLOAT_DESC& Desc)
	{
		Writer.Pod(Desc.eType);
		Writer.Pod(Desc.fConstant);
		Writer.Pod(Desc.fMin);
		Writer.Pod(Desc.fMax);
	}

	void WriteVectorDistribution(BYTE_WRITER& Writer,
		const EFFECT_DISTRIBUTION_VECTOR_DESC& Desc)
	{
		Writer.Pod(Desc.eType);
		Writer.Pod(Desc.vConstant);
		Writer.Pod(Desc.vMin);
		Writer.Pod(Desc.vMax);
	}

	void WriteColorDistribution(BYTE_WRITER& Writer,
		const EFFECT_DISTRIBUTION_COLOR_DESC& Desc)
	{
		Writer.Pod(Desc.eType);
		Writer.Pod(Desc.vConstant);
		Writer.Pod(Desc.vMin);
		Writer.Pod(Desc.vMax);
	}

	bool ReadFloatDistribution(BYTE_READER& Reader,
		EFFECT_DISTRIBUTION_FLOAT_DESC& Desc)
	{
		return Reader.Pod(Desc.eType) &&
			Reader.Pod(Desc.fConstant) &&
			Reader.Pod(Desc.fMin) &&
			Reader.Pod(Desc.fMax);
	}

	bool ReadVectorDistribution(BYTE_READER& Reader,
		EFFECT_DISTRIBUTION_VECTOR_DESC& Desc)
	{
		return Reader.Pod(Desc.eType) &&
			Reader.Pod(Desc.vConstant) &&
			Reader.Pod(Desc.vMin) &&
			Reader.Pod(Desc.vMax);
	}

	bool ReadColorDistribution(BYTE_READER& Reader,
		EFFECT_DISTRIBUTION_COLOR_DESC& Desc)
	{
		return Reader.Pod(Desc.eType) &&
			Reader.Pod(Desc.vConstant) &&
			Reader.Pod(Desc.vMin) &&
			Reader.Pod(Desc.vMax);
	}

	void WriteCurve(BYTE_WRITER& Writer,
		const EFFECT_CURVE_FLOAT_DESC& Curve)
	{
		Writer.Pod(static_cast<uint32_t>(Curve.Keys.size()));
		for (const EFFECT_CURVE_KEY& Key : Curve.Keys)
			Writer.Pod(Key);
	}

	bool ReadCurve(BYTE_READER& Reader,
		EFFECT_CURVE_FLOAT_DESC& Curve)
	{
		uint32_t iCount = {};
		if (!Reader.Pod(iCount) || iCount > 4096)
			return false;
		Curve.Keys.resize(iCount);
		for (EFFECT_CURVE_KEY& Key : Curve.Keys)
			if (!Reader.Pod(Key))
				return false;
		return true;
	}

	void WriteMaterial(BYTE_WRITER& Writer,
		const EFFECT_MATERIAL_DESC& Material)
	{
		Writer.String(Material.strOpacityTextureAssetId);
		Writer.String(Material.strDissolveTextureAssetId);
		Writer.String(Material.strDistortionTextureAssetId);
		Writer.Pod(Material.vUVTiling);
		Writer.Pod(Material.vUVOffset);
		Writer.Pod(Material.vUVPanner);
		Writer.Pod(Material.fEmissiveStrength);
		Writer.Pod(Material.fOpacityMaskThreshold);
		Writer.Pod(Material.fDissolveAmount);
		Writer.Pod(Material.fDissolveEdgeWidth);
		Writer.Pod(Material.vDissolveEdgeColor);
		Writer.Pod(Material.fSoftParticleDistance);
		Writer.Pod(Material.fDistortionStrength);
	}

	bool ReadMaterial(BYTE_READER& Reader, EFFECT_MATERIAL_DESC& Material)
	{
		return Reader.String(Material.strOpacityTextureAssetId) &&
			Reader.String(Material.strDissolveTextureAssetId) &&
			Reader.String(Material.strDistortionTextureAssetId) &&
			Reader.Pod(Material.vUVTiling) &&
			Reader.Pod(Material.vUVOffset) &&
			Reader.Pod(Material.vUVPanner) &&
			Reader.Pod(Material.fEmissiveStrength) &&
			Reader.Pod(Material.fOpacityMaskThreshold) &&
			Reader.Pod(Material.fDissolveAmount) &&
			Reader.Pod(Material.fDissolveEdgeWidth) &&
			Reader.Pod(Material.vDissolveEdgeColor) &&
			Reader.Pod(Material.fSoftParticleDistance) &&
			Reader.Pod(Material.fDistortionStrength);
	}

	void WriteDynamicParameter(BYTE_WRITER& Writer,
		const EFFECT_DYNAMIC_PARAMETER_MODULE_DESC& DynamicParameter)
	{
		WriteCurve(Writer, DynamicParameter.X);
		WriteCurve(Writer, DynamicParameter.Y);
		WriteCurve(Writer, DynamicParameter.Z);
		WriteCurve(Writer, DynamicParameter.W);
	}

	bool ReadDynamicParameter(BYTE_READER& Reader,
		EFFECT_DYNAMIC_PARAMETER_MODULE_DESC& DynamicParameter)
	{
		return ReadCurve(Reader, DynamicParameter.X) &&
			ReadCurve(Reader, DynamicParameter.Y) &&
			ReadCurve(Reader, DynamicParameter.Z) &&
			ReadCurve(Reader, DynamicParameter.W);
	}

	vector<uint8_t> Serialize(const EFFECT_ASSET_DESC& Asset)
	{
		BYTE_WRITER Writer;
		Writer.Pod(EFFECT_ASSET_SCHEMA_VERSION);
		Writer.String(Asset.strAssetId);
		Writer.String(Asset.strName);
		Writer.Pod(Asset.eProvenance);
		Writer.Pod(Asset.fDuration);
		Writer.Pod(Asset.fWarmupTime);
		Writer.Pod(static_cast<uint32_t>(Asset.Emitters.size()));

		for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
		{
			Writer.Pod(Emitter.iEmitterId);
			Writer.String(Emitter.strName);
			Writer.Pod(Emitter.eType);
			Writer.Pod(Emitter.isEnabled);
			Writer.Pod(Emitter.eSimulationSpace);
			Writer.Pod(Emitter.eBlendMode);
			Writer.Pod(Emitter.eSortMode);
			Writer.Pod(Emitter.fDelay);
			Writer.Pod(Emitter.fDuration);
			Writer.Pod(Emitter.iLoopCount);
			Writer.Pod(static_cast<uint32_t>(Emitter.Modules.size()));

			for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
			{
				Writer.Pod(Module.iModuleId);
				Writer.String(Module.strName);
				Writer.Pod(Module.eType);
				Writer.Pod(Module.isEnabled);
				Writer.String(Module.Required.strTextureAssetId);
				Writer.String(Module.Required.strMeshAssetId);
				Writer.String(Module.Required.strMaterialAssetId);
				Writer.Pod(Module.Required.eScreenAlignment);
				Writer.Pod(Module.Spawn.fRatePerSecond);
				Writer.Pod(Module.Spawn.iBurstCount);
				Writer.Pod(Module.Spawn.iMaxParticles);
				WriteFloatDistribution(Writer,
					Module.Lifetime.Lifetime);
				WriteVectorDistribution(Writer,
					Module.InitialLocation.Location);
				Writer.Pod(Module.InitialLocation.eShape);
				Writer.Pod(Module.InitialLocation.fRadius);
				Writer.Pod(Module.InitialLocation.fInnerRadius);
				Writer.Pod(Module.InitialLocation.fHeight);
				Writer.Pod(Module.InitialLocation.isSurfaceOnly);
				WriteVectorDistribution(Writer,
					Module.InitialVelocity.Velocity);
				WriteVectorDistribution(Writer,
					Module.InitialSize.Size);
				WriteColorDistribution(Writer,
					Module.InitialColor.Color);
				WriteCurve(Writer, Module.SizeOverLife.Curve);
				WriteCurve(Writer, Module.AlphaOverLife.Curve);
				WriteCurve(Writer, Module.VelocityOverLife.Curve);
				Writer.Pod(Module.Collision);
				Writer.String(Module.Event.strEventName);
				Writer.Pod(Module.Event.fNormalizedTime);
				Writer.Pod(Module.LOD);
				WriteVectorDistribution(Writer,
					Module.InitialRotation.RotationDegrees);
				WriteVectorDistribution(Writer,
					Module.RotationRate.RotationRateDegreesPerSecond);
				WriteCurve(Writer, Module.ColorOverLife.Red);
				WriteCurve(Writer, Module.ColorOverLife.Green);
				WriteCurve(Writer, Module.ColorOverLife.Blue);
				WriteCurve(Writer, Module.ColorOverLife.Alpha);
				Writer.Pod(Module.SubUV.iColumns);
				Writer.Pod(Module.SubUV.iRows);
				Writer.Pod(Module.SubUV.fFramesPerSecond);
				Writer.Pod(Module.SubUV.iStartFrame);
				Writer.Pod(Module.SubUV.iEndFrame);
				Writer.Pod(Module.SubUV.isLoop);
				Writer.Pod(Module.SubUV.useParticleRelativeTime);
				Writer.Pod(Module.MeshAnimation.iAnimationIndex);
				Writer.Pod(Module.MeshAnimation.fPlayRate);
				Writer.Pod(Module.MeshAnimation.isLoop);
				Writer.Pod(Module.Beam.vSourceOffset);
				Writer.Pod(Module.Beam.vTargetOffset);
				Writer.Pod(Module.Beam.fWidth);
				Writer.Pod(Module.Beam.iSegments);
				Writer.Pod(Module.Beam.fNoiseAmplitude);
				Writer.Pod(Module.Trail.fWidth);
				Writer.Pod(Module.Trail.fPointLifetime);
				Writer.Pod(Module.Trail.iMaxPoints);
				Writer.String(Module.Trail.strSourceBone);
				Writer.String(Module.Trail.strTargetBone);
				WriteMaterial(Writer, Module.Required.Material);
				WriteDynamicParameter(Writer, Module.DynamicParameter);
			}
		}
		return Writer.Bytes;
	}

	bool Deserialize(const vector<uint8_t>& Bytes,
		EFFECT_ASSET_DESC& OutAsset,
		const uint32_t iExpectedSchemaVersion = 0)
	{
		BYTE_READER Reader{ Bytes };
		EFFECT_ASSET_DESC Asset;
		uint32_t iSourceSchemaVersion = {};
		uint32_t iEmitterCount = {};
		if (!Reader.Pod(iSourceSchemaVersion) ||
			!IsSupportedSchemaVersion(iSourceSchemaVersion) ||
			(0 != iExpectedSchemaVersion &&
				iSourceSchemaVersion != iExpectedSchemaVersion) ||
			!Reader.String(Asset.strAssetId) ||
			!Reader.String(Asset.strName) ||
			!Reader.Pod(Asset.eProvenance) ||
			!Reader.Pod(Asset.fDuration) ||
			!Reader.Pod(Asset.fWarmupTime) ||
			!Reader.Pod(iEmitterCount) ||
			iEmitterCount > EFFECT_MAX_EMITTERS)
			return false;

		Asset.Emitters.resize(iEmitterCount);
		for (EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
		{
			uint32_t iModuleCount = {};
			if (!Reader.Pod(Emitter.iEmitterId) ||
				!Reader.String(Emitter.strName) ||
				!Reader.Pod(Emitter.eType) ||
				!Reader.Pod(Emitter.isEnabled) ||
				!Reader.Pod(Emitter.eSimulationSpace) ||
				!Reader.Pod(Emitter.eBlendMode) ||
				!Reader.Pod(Emitter.eSortMode) ||
				!Reader.Pod(Emitter.fDelay) ||
				!Reader.Pod(Emitter.fDuration) ||
				!Reader.Pod(Emitter.iLoopCount) ||
				!Reader.Pod(iModuleCount) ||
				iModuleCount > EFFECT_MAX_MODULES_PER_EMITTER)
				return false;

			Emitter.Modules.resize(iModuleCount);
			for (EFFECT_MODULE_DESC& Module : Emitter.Modules)
			{
				if (!Reader.Pod(Module.iModuleId) ||
					!Reader.String(Module.strName) ||
					!Reader.Pod(Module.eType) ||
					!Reader.Pod(Module.isEnabled) ||
					!Reader.String(Module.Required.strTextureAssetId) ||
					!Reader.String(Module.Required.strMeshAssetId) ||
					!Reader.String(Module.Required.strMaterialAssetId) ||
					!Reader.Pod(Module.Required.eScreenAlignment) ||
					!Reader.Pod(Module.Spawn.fRatePerSecond) ||
					!Reader.Pod(Module.Spawn.iBurstCount) ||
					!Reader.Pod(Module.Spawn.iMaxParticles) ||
					!ReadFloatDistribution(Reader,
						Module.Lifetime.Lifetime) ||
					!ReadVectorDistribution(Reader,
						Module.InitialLocation.Location) ||
					!Reader.Pod(Module.InitialLocation.eShape) ||
					!Reader.Pod(Module.InitialLocation.fRadius) ||
					!Reader.Pod(Module.InitialLocation.fInnerRadius) ||
					!Reader.Pod(Module.InitialLocation.fHeight) ||
					!Reader.Pod(Module.InitialLocation.isSurfaceOnly) ||
					!ReadVectorDistribution(Reader,
						Module.InitialVelocity.Velocity) ||
					!ReadVectorDistribution(Reader,
						Module.InitialSize.Size) ||
					!ReadColorDistribution(Reader,
						Module.InitialColor.Color) ||
					!ReadCurve(Reader, Module.SizeOverLife.Curve) ||
					!ReadCurve(Reader, Module.AlphaOverLife.Curve) ||
					!ReadCurve(Reader, Module.VelocityOverLife.Curve) ||
					!Reader.Pod(Module.Collision) ||
					!Reader.String(Module.Event.strEventName) ||
					!Reader.Pod(Module.Event.fNormalizedTime) ||
					!Reader.Pod(Module.LOD) ||
					!ReadVectorDistribution(Reader,
						Module.InitialRotation.RotationDegrees) ||
					!ReadVectorDistribution(Reader,
						Module.RotationRate.RotationRateDegreesPerSecond) ||
					!ReadCurve(Reader,
						Module.ColorOverLife.Red) ||
					!ReadCurve(Reader,
						Module.ColorOverLife.Green) ||
					!ReadCurve(Reader,
						Module.ColorOverLife.Blue) ||
					!ReadCurve(Reader,
						Module.ColorOverLife.Alpha) ||
					!Reader.Pod(Module.SubUV.iColumns) ||
					!Reader.Pod(Module.SubUV.iRows) ||
					!Reader.Pod(Module.SubUV.fFramesPerSecond) ||
					!Reader.Pod(Module.SubUV.iStartFrame) ||
					!Reader.Pod(Module.SubUV.iEndFrame) ||
					!Reader.Pod(Module.SubUV.isLoop) ||
					!Reader.Pod(
						Module.SubUV.useParticleRelativeTime) ||
					!Reader.Pod(
						Module.MeshAnimation.iAnimationIndex) ||
					!Reader.Pod(Module.MeshAnimation.fPlayRate) ||
					!Reader.Pod(Module.MeshAnimation.isLoop) ||
					!Reader.Pod(Module.Beam.vSourceOffset) ||
					!Reader.Pod(Module.Beam.vTargetOffset) ||
					!Reader.Pod(Module.Beam.fWidth) ||
					!Reader.Pod(Module.Beam.iSegments) ||
					!Reader.Pod(Module.Beam.fNoiseAmplitude) ||
					!Reader.Pod(Module.Trail.fWidth) ||
					!Reader.Pod(Module.Trail.fPointLifetime) ||
					!Reader.Pod(Module.Trail.iMaxPoints) ||
					!Reader.String(Module.Trail.strSourceBone) ||
					!Reader.String(Module.Trail.strTargetBone))
					return false;

				if (EFFECT_ASSET_SCHEMA_VERSION == iSourceSchemaVersion &&
					(!ReadMaterial(Reader, Module.Required.Material) ||
						!ReadDynamicParameter(
							Reader, Module.DynamicParameter)))
				{
					return false;
				}

				const uint8_t iModuleType =
					static_cast<uint8_t>(Module.eType);
				const uint8_t iFirstUnsupportedType =
					(LEGACY_EFFECT_ASSET_SCHEMA_VERSION ==
						iSourceSchemaVersion)
					? static_cast<uint8_t>(
						EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER)
					: static_cast<uint8_t>(EFFECT_MODULE_TYPE::END);
				if (iModuleType >= iFirstUnsupportedType)
					return false;

				if (Module.Spawn.iMaxParticles >
						EFFECT_MAX_PARTICLES_PER_EMITTER ||
					Module.Trail.iMaxPoints >
						EFFECT_MAX_TRAIL_POINTS ||
					Module.Beam.iSegments >
						EFFECT_MAX_BEAM_SEGMENTS ||
					Module.SubUV.iColumns >
						EFFECT_MAX_SUBUV_DIMENSION ||
					Module.SubUV.iRows >
						EFFECT_MAX_SUBUV_DIMENSION)
				{
					return false;
				}
			}
		}
		if (Reader.Offset != Bytes.size())
			return false;
		Asset.iSchemaVersion = EFFECT_ASSET_SCHEMA_VERSION;
		OutAsset = move(Asset);
		return true;
	}

	uint32_t Hash(const vector<uint8_t>& Bytes)
	{
		uint32_t iHash = 2166136261u;
		for (const uint8_t iByte : Bytes)
			iHash = (iHash ^ iByte) * 16777619u;
		return iHash;
	}

	bool PrepareParent(const filesystem::path& Path, string* pError)
	{
		error_code Error;
		if (!Path.parent_path().empty())
			filesystem::create_directories(Path.parent_path(), Error);
		if (Error)
		{
			if (pError) *pError = Error.message();
			return false;
		}
		return true;
	}

	/*
	 * Authoring text format.
	 *
	 *   LOSTARK_EFFECT 5
	 *   ASSET id="glaivier-dash-02" name="..." duration=0.3 warmup=0
	 *   EMITTER id=1 type=SPRITE blend=ADDITIVE duration=0.3 loops=1
	 *     MODULE id=2 type=REQUIRED tex="..." align=VELOCITY
	 *     MODULE id=3 type=SPAWN rate=50 max=17
	 *
	 * Every payload field is a key=value token, so adding a field later never
	 * breaks an existing file: unknown keys are ignored and absent keys keep
	 * the struct default. This mirrors how Client/Bin/DataFiles/Map/*.mapassets
	 * and LanceMaster.skilltiming are already read.
	 */
	constexpr char_t AUTHORING_MAGIC[] = "LOSTARK_EFFECT";

	const char_t* const PROVENANCE_NAMES[] = {
		"UNKNOWN", "GAME_ORIGINAL", "DERIVED_CONVERSION",
		"RECONSTRUCTED", "AUTHORED_VARIANT"
	};
	const char_t* const EMITTER_TYPE_NAMES[] = {
		"SPRITE", "MESH", "BEAM", "RIBBON", "ANIM_TRAIL"
	};
	const char_t* const MODULE_TYPE_NAMES[] = {
		"REQUIRED", "SPAWN", "LIFETIME", "INITIAL_LOCATION",
		"INITIAL_VELOCITY", "INITIAL_SIZE", "INITIAL_COLOR",
		"SIZE_OVER_LIFE", "ALPHA_OVER_LIFE", "VELOCITY_OVER_LIFE",
		"COLLISION", "EVENT", "LOD", "INITIAL_ROTATION", "ROTATION_RATE",
		"COLOR_OVER_LIFE", "SUB_UV", "MESH_ANIMATION", "BEAM", "TRAIL",
		"DYNAMIC_PARAMETER"
	};
	const char_t* const DISTRIBUTION_NAMES[] = {
		"CONSTANT", "UNIFORM_RANGE"
	};
	const char_t* const SIMULATION_SPACE_NAMES[] = { "LOCAL", "WORLD" };
	const char_t* const BLEND_MODE_NAMES[] = { "ALPHA", "ADDITIVE" };
	const char_t* const SORT_MODE_NAMES[] = {
		"NONE", "DISTANCE_TO_VIEW", "AGE_OLDEST_FIRST", "AGE_NEWEST_FIRST"
	};
	const char_t* const SCREEN_ALIGNMENT_NAMES[] = {
		"SQUARE", "RECTANGLE", "VELOCITY"
	};
	const char_t* const LOCATION_SHAPE_NAMES[] = {
		"BOX", "SPHERE", "CYLINDER"
	};

	template<typename T, size_t N>
	const char_t* EnumText(T eValue, const char_t* const (&Names)[N])
	{
		const size_t iIndex = static_cast<size_t>(eValue);
		return (iIndex < N) ? Names[iIndex] : Names[0];
	}

	template<typename T, size_t N>
	T TextEnum(const string& Text, const char_t* const (&Names)[N],
		T eFallback)
	{
		for (size_t i = 0; i < N; ++i)
			if (Text == Names[i])
				return static_cast<T>(i);
		return eFallback;
	}

	// Shortest decimal text that still reads back as the same float, so the
	// file stays readable without losing round trip fidelity.
	string FloatText(f32_t fValue)
	{
		for (int32_t iPrecision = 6; iPrecision < 9; ++iPrecision)
		{
			ostringstream Stream;
			Stream << setprecision(iPrecision) << fValue;
			if (strtof(Stream.str().c_str(), nullptr) == fValue)
				return Stream.str();
		}
		ostringstream Stream;
		Stream << setprecision(9) << fValue;
		return Stream.str();
	}

	string Vec3Text(const float3_t& vValue)
	{
		return FloatText(vValue.x) + "," + FloatText(vValue.y) + "," +
			FloatText(vValue.z);
	}

	string Vec2Text(const float2_t& vValue)
	{
		return FloatText(vValue.x) + "," + FloatText(vValue.y);
	}

	string Vec4Text(const float4_t& vValue)
	{
		return FloatText(vValue.x) + "," + FloatText(vValue.y) + "," +
			FloatText(vValue.z) + "," + FloatText(vValue.w);
	}

	string CurveText(const EFFECT_CURVE_FLOAT_DESC& Curve)
	{
		string Text;
		for (size_t i = 0; i < Curve.Keys.size(); ++i)
		{
			if (0 != i)
				Text += ',';
			Text += FloatText(Curve.Keys[i].fTime) + ":" +
				FloatText(Curve.Keys[i].fValue);
		}
		return Text;
	}

	string Quote(const string& Value)
	{
		string Text = "\"";
		for (const char_t Character : Value)
		{
			if ('"' == Character || '\\' == Character)
				Text += '\\';
			Text += Character;
		}
		return Text + "\"";
	}

	vector<string> SplitTokens(const string& Line)
	{
		vector<string> Tokens;
		string Current;
		bool_t isQuoted = false;
		bool_t hasContent = false;
		for (size_t i = 0; i < Line.size(); ++i)
		{
			const char_t Character = Line[i];
			if ('"' == Character)
			{
				isQuoted = !isQuoted;
				hasContent = true;
				continue;
			}
			if ('\\' == Character && isQuoted && i + 1 < Line.size())
			{
				Current += Line[++i];
				continue;
			}
			if (!isQuoted &&
				(' ' == Character || '\t' == Character || '\r' == Character))
			{
				if (hasContent)
				{
					Tokens.push_back(Current);
					Current.clear();
					hasContent = false;
				}
				continue;
			}
			Current += Character;
			hasContent = true;
		}
		if (hasContent)
			Tokens.push_back(Current);
		return Tokens;
	}

	struct TOKEN_LINE
	{
		string Kind;
		vector<pair<string, string>> Fields;

		const string* Find(const char_t* pKey) const
		{
			for (const pair<string, string>& Field : Fields)
				if (Field.first == pKey)
					return &Field.second;
			return nullptr;
		}

		string Text(const char_t* pKey, const string& Fallback = {}) const
		{
			const string* pValue = Find(pKey);
			return (nullptr != pValue) ? *pValue : Fallback;
		}

		f32_t Float(const char_t* pKey, f32_t fFallback) const
		{
			const string* pValue = Find(pKey);
			return (nullptr != pValue)
				? strtof(pValue->c_str(), nullptr)
				: fFallback;
		}

		uint32_t UInt(const char_t* pKey, uint32_t iFallback) const
		{
			const string* pValue = Find(pKey);
			return (nullptr != pValue)
				? static_cast<uint32_t>(strtoul(pValue->c_str(), nullptr, 10))
				: iFallback;
		}

		uint64_t UInt64(const char_t* pKey, uint64_t iFallback) const
		{
			const string* pValue = Find(pKey);
			return (nullptr != pValue)
				? strtoull(pValue->c_str(), nullptr, 10)
				: iFallback;
		}

		bool_t Bool(const char_t* pKey, bool_t isFallback) const
		{
			const string* pValue = Find(pKey);
			if (nullptr == pValue)
				return isFallback;
			return "1" == *pValue || "true" == *pValue;
		}
	};

	bool ParseLine(const string& Line, TOKEN_LINE& Out)
	{
		const vector<string> Tokens = SplitTokens(Line);
		if (Tokens.empty() || '#' == Tokens.front()[0])
			return false;
		Out.Kind = Tokens.front();
		Out.Fields.clear();
		for (size_t i = 1; i < Tokens.size(); ++i)
		{
			const size_t iEqual = Tokens[i].find('=');
			if (string::npos == iEqual)
				continue;
			Out.Fields.emplace_back(
				Tokens[i].substr(0, iEqual),
				Tokens[i].substr(iEqual + 1));
		}
		return true;
	}

	void ParseVec3(const string& Text, float3_t& vOut)
	{
		const char_t* pCursor = Text.c_str();
		char_t* pNext = nullptr;
		f32_t Components[3] = { vOut.x, vOut.y, vOut.z };
		for (int32_t i = 0; i < 3 && '\0' != *pCursor; ++i)
		{
			Components[i] = strtof(pCursor, &pNext);
			if (pNext == pCursor)
				break;
			pCursor = ('\0' != *pNext) ? pNext + 1 : pNext;
		}
		vOut = { Components[0], Components[1], Components[2] };
	}

	void ParseVec2(const string& Text, float2_t& vOut)
	{
		const char_t* pCursor = Text.c_str();
		char_t* pNext = nullptr;
		f32_t Components[2] = { vOut.x, vOut.y };
		for (int32_t i = 0; i < 2 && '\0' != *pCursor; ++i)
		{
			Components[i] = strtof(pCursor, &pNext);
			if (pNext == pCursor)
				break;
			pCursor = ('\0' != *pNext) ? pNext + 1 : pNext;
		}
		vOut = { Components[0], Components[1] };
	}

	void ParseVec4(const string& Text, float4_t& vOut)
	{
		const char_t* pCursor = Text.c_str();
		char_t* pNext = nullptr;
		f32_t Components[4] = { vOut.x, vOut.y, vOut.z, vOut.w };
		for (int32_t i = 0; i < 4 && '\0' != *pCursor; ++i)
		{
			Components[i] = strtof(pCursor, &pNext);
			if (pNext == pCursor)
				break;
			pCursor = ('\0' != *pNext) ? pNext + 1 : pNext;
		}
		vOut = {
			Components[0], Components[1], Components[2], Components[3]
		};
	}

	bool ParseCurve(const string& Text, EFFECT_CURVE_FLOAT_DESC& Curve)
	{
		Curve.Keys.clear();
		if (Text.empty())
			return true;
		size_t iStart = 0;
		while (iStart <= Text.size())
		{
			size_t iComma = Text.find(',', iStart);
			if (string::npos == iComma)
				iComma = Text.size();
			const string Pair = Text.substr(iStart, iComma - iStart);
			const size_t iColon = Pair.find(':');
			if (string::npos == iColon)
				return false;
			if (Curve.Keys.size() >= 4096)
				return false;
			Curve.Keys.push_back({
				strtof(Pair.substr(0, iColon).c_str(), nullptr),
				strtof(Pair.substr(iColon + 1).c_str(), nullptr)
			});
			if (iComma == Text.size())
				break;
			iStart = iComma + 1;
		}
		return true;
	}

	string FloatDistributionText(
		const EFFECT_DISTRIBUTION_FLOAT_DESC& Desc)
	{
		return string(" dist=") + EnumText(Desc.eType, DISTRIBUTION_NAMES) +
			" c=" + FloatText(Desc.fConstant) +
			" min=" + FloatText(Desc.fMin) +
			" max=" + FloatText(Desc.fMax);
	}

	string VectorDistributionText(
		const EFFECT_DISTRIBUTION_VECTOR_DESC& Desc)
	{
		return string(" dist=") + EnumText(Desc.eType, DISTRIBUTION_NAMES) +
			" c=" + Vec3Text(Desc.vConstant) +
			" min=" + Vec3Text(Desc.vMin) +
			" max=" + Vec3Text(Desc.vMax);
	}

	string ColorDistributionText(
		const EFFECT_DISTRIBUTION_COLOR_DESC& Desc)
	{
		return string(" dist=") + EnumText(Desc.eType, DISTRIBUTION_NAMES) +
			" c=" + Vec4Text(Desc.vConstant) +
			" min=" + Vec4Text(Desc.vMin) +
			" max=" + Vec4Text(Desc.vMax);
	}

	void ReadFloatDistribution(const TOKEN_LINE& Line,
		EFFECT_DISTRIBUTION_FLOAT_DESC& Desc)
	{
		Desc.eType = TextEnum(
			Line.Text("dist"), DISTRIBUTION_NAMES, Desc.eType);
		Desc.fConstant = Line.Float("c", Desc.fConstant);
		Desc.fMin = Line.Float("min", Desc.fMin);
		Desc.fMax = Line.Float("max", Desc.fMax);
	}

	void ReadVectorDistribution(const TOKEN_LINE& Line,
		EFFECT_DISTRIBUTION_VECTOR_DESC& Desc)
	{
		Desc.eType = TextEnum(
			Line.Text("dist"), DISTRIBUTION_NAMES, Desc.eType);
		ParseVec3(Line.Text("c"), Desc.vConstant);
		ParseVec3(Line.Text("min"), Desc.vMin);
		ParseVec3(Line.Text("max"), Desc.vMax);
	}

	void ReadColorDistribution(const TOKEN_LINE& Line,
		EFFECT_DISTRIBUTION_COLOR_DESC& Desc)
	{
		Desc.eType = TextEnum(
			Line.Text("dist"), DISTRIBUTION_NAMES, Desc.eType);
		ParseVec4(Line.Text("c"), Desc.vConstant);
		ParseVec4(Line.Text("min"), Desc.vMin);
		ParseVec4(Line.Text("max"), Desc.vMax);
	}

	// Only the payload that eType actually selects is written, so the file
	// shows what the module really does instead of every unused struct.
	string ModulePayloadText(const EFFECT_MODULE_DESC& Module)
	{
		switch (Module.eType)
		{
		case EFFECT_MODULE_TYPE::REQUIRED:
			return " tex=" + Quote(Module.Required.strTextureAssetId) +
				" mesh=" + Quote(Module.Required.strMeshAssetId) +
				" mat=" + Quote(Module.Required.strMaterialAssetId) +
				" align=" + EnumText(
					Module.Required.eScreenAlignment,
					SCREEN_ALIGNMENT_NAMES) +
				" opacitytex=" + Quote(
					Module.Required.Material.strOpacityTextureAssetId) +
				" dissolvetex=" + Quote(
					Module.Required.Material.strDissolveTextureAssetId) +
				" distortiontex=" + Quote(
					Module.Required.Material.strDistortionTextureAssetId) +
				" uvtiling=" + Vec2Text(
					Module.Required.Material.vUVTiling) +
				" uvoffset=" + Vec2Text(
					Module.Required.Material.vUVOffset) +
				" uvpanner=" + Vec2Text(
					Module.Required.Material.vUVPanner) +
				" emissive=" + FloatText(
					Module.Required.Material.fEmissiveStrength) +
				" opacitythreshold=" + FloatText(
					Module.Required.Material.fOpacityMaskThreshold) +
				" dissolveamount=" + FloatText(
					Module.Required.Material.fDissolveAmount) +
				" dissolveedgewidth=" + FloatText(
					Module.Required.Material.fDissolveEdgeWidth) +
				" dissolveedgecolor=" + Vec4Text(
					Module.Required.Material.vDissolveEdgeColor) +
				" softdistance=" + FloatText(
					Module.Required.Material.fSoftParticleDistance) +
				" distortionstrength=" + FloatText(
					Module.Required.Material.fDistortionStrength);

		case EFFECT_MODULE_TYPE::SPAWN:
			return " rate=" + FloatText(Module.Spawn.fRatePerSecond) +
				" burst=" + to_string(Module.Spawn.iBurstCount) +
				" max=" + to_string(Module.Spawn.iMaxParticles);

		case EFFECT_MODULE_TYPE::LIFETIME:
			return FloatDistributionText(Module.Lifetime.Lifetime);

		case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
			return VectorDistributionText(Module.InitialLocation.Location) +
				" shape=" + EnumText(
					Module.InitialLocation.eShape, LOCATION_SHAPE_NAMES) +
				" radius=" + FloatText(Module.InitialLocation.fRadius) +
				" inner=" + FloatText(Module.InitialLocation.fInnerRadius) +
				" height=" + FloatText(Module.InitialLocation.fHeight) +
				" surface=" +
				(Module.InitialLocation.isSurfaceOnly ? "1" : "0");

		case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
			return VectorDistributionText(Module.InitialVelocity.Velocity);

		case EFFECT_MODULE_TYPE::INITIAL_SIZE:
			return VectorDistributionText(Module.InitialSize.Size);

		case EFFECT_MODULE_TYPE::INITIAL_COLOR:
			return ColorDistributionText(Module.InitialColor.Color);

		case EFFECT_MODULE_TYPE::SIZE_OVER_LIFE:
			return " curve=" + CurveText(Module.SizeOverLife.Curve);

		case EFFECT_MODULE_TYPE::ALPHA_OVER_LIFE:
			return " curve=" + CurveText(Module.AlphaOverLife.Curve);

		case EFFECT_MODULE_TYPE::VELOCITY_OVER_LIFE:
			return " curve=" + CurveText(Module.VelocityOverLife.Curve);

		case EFFECT_MODULE_TYPE::COLLISION:
			return " min=" + Vec3Text(Module.Collision.vMinBounds) +
				" max=" + Vec3Text(Module.Collision.vMaxBounds) +
				" restitution=" + FloatText(Module.Collision.fRestitution);

		case EFFECT_MODULE_TYPE::EVENT:
			return " event=" + Quote(Module.Event.strEventName) +
				" time=" + FloatText(Module.Event.fNormalizedTime);

		case EFFECT_MODULE_TYPE::LOD:
			return " near=" + FloatText(Module.LOD.fNearDistance) +
				" far=" + FloatText(Module.LOD.fFarDistance) +
				" farscale=" + FloatText(Module.LOD.fFarSpawnScale);

		case EFFECT_MODULE_TYPE::INITIAL_ROTATION:
			return VectorDistributionText(
				Module.InitialRotation.RotationDegrees);

		case EFFECT_MODULE_TYPE::ROTATION_RATE:
			return VectorDistributionText(
				Module.RotationRate.RotationRateDegreesPerSecond);

		case EFFECT_MODULE_TYPE::COLOR_OVER_LIFE:
			return " r=" + CurveText(Module.ColorOverLife.Red) +
				" g=" + CurveText(Module.ColorOverLife.Green) +
				" b=" + CurveText(Module.ColorOverLife.Blue) +
				" a=" + CurveText(Module.ColorOverLife.Alpha);

		case EFFECT_MODULE_TYPE::SUB_UV:
			return " cols=" + to_string(Module.SubUV.iColumns) +
				" rows=" + to_string(Module.SubUV.iRows) +
				" fps=" + FloatText(Module.SubUV.fFramesPerSecond) +
				" start=" + to_string(Module.SubUV.iStartFrame) +
				" end=" + to_string(Module.SubUV.iEndFrame) +
				" loop=" + (Module.SubUV.isLoop ? "1" : "0") +
				" reltime=" +
				(Module.SubUV.useParticleRelativeTime ? "1" : "0");

		case EFFECT_MODULE_TYPE::MESH_ANIMATION:
			return " anim=" + to_string(Module.MeshAnimation.iAnimationIndex) +
				" rate=" + FloatText(Module.MeshAnimation.fPlayRate) +
				" loop=" + (Module.MeshAnimation.isLoop ? "1" : "0");

		case EFFECT_MODULE_TYPE::BEAM:
			return " src=" + Vec3Text(Module.Beam.vSourceOffset) +
				" dst=" + Vec3Text(Module.Beam.vTargetOffset) +
				" width=" + FloatText(Module.Beam.fWidth) +
				" segments=" + to_string(Module.Beam.iSegments) +
				" noise=" + FloatText(Module.Beam.fNoiseAmplitude);

		case EFFECT_MODULE_TYPE::TRAIL:
			return " width=" + FloatText(Module.Trail.fWidth) +
				" ptlife=" + FloatText(Module.Trail.fPointLifetime) +
				" maxpts=" + to_string(Module.Trail.iMaxPoints) +
				" srcbone=" + Quote(Module.Trail.strSourceBone) +
				" dstbone=" + Quote(Module.Trail.strTargetBone);

		case EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER:
			return " x=" + CurveText(Module.DynamicParameter.X) +
				" y=" + CurveText(Module.DynamicParameter.Y) +
				" z=" + CurveText(Module.DynamicParameter.Z) +
				" w=" + CurveText(Module.DynamicParameter.W);
		}
		return {};
	}

	bool ReadModulePayload(const TOKEN_LINE& Line, EFFECT_MODULE_DESC& Module)
	{
		switch (Module.eType)
		{
		case EFFECT_MODULE_TYPE::REQUIRED:
			Module.Required.strTextureAssetId = Line.Text("tex");
			Module.Required.strMeshAssetId = Line.Text("mesh");
			Module.Required.strMaterialAssetId = Line.Text("mat");
			Module.Required.eScreenAlignment = TextEnum(
				Line.Text("align"), SCREEN_ALIGNMENT_NAMES,
				Module.Required.eScreenAlignment);
			Module.Required.Material.strOpacityTextureAssetId = Line.Text(
				"opacitytex",
				Module.Required.Material.strOpacityTextureAssetId);
			Module.Required.Material.strDissolveTextureAssetId = Line.Text(
				"dissolvetex",
				Module.Required.Material.strDissolveTextureAssetId);
			Module.Required.Material.strDistortionTextureAssetId = Line.Text(
				"distortiontex",
				Module.Required.Material.strDistortionTextureAssetId);
			ParseVec2(Line.Text("uvtiling"),
				Module.Required.Material.vUVTiling);
			ParseVec2(Line.Text("uvoffset"),
				Module.Required.Material.vUVOffset);
			ParseVec2(Line.Text("uvpanner"),
				Module.Required.Material.vUVPanner);
			Module.Required.Material.fEmissiveStrength = Line.Float(
				"emissive",
				Module.Required.Material.fEmissiveStrength);
			Module.Required.Material.fOpacityMaskThreshold = Line.Float(
				"opacitythreshold",
				Module.Required.Material.fOpacityMaskThreshold);
			Module.Required.Material.fDissolveAmount = Line.Float(
				"dissolveamount",
				Module.Required.Material.fDissolveAmount);
			Module.Required.Material.fDissolveEdgeWidth = Line.Float(
				"dissolveedgewidth",
				Module.Required.Material.fDissolveEdgeWidth);
			ParseVec4(Line.Text("dissolveedgecolor"),
				Module.Required.Material.vDissolveEdgeColor);
			Module.Required.Material.fSoftParticleDistance = Line.Float(
				"softdistance",
				Module.Required.Material.fSoftParticleDistance);
			Module.Required.Material.fDistortionStrength = Line.Float(
				"distortionstrength",
				Module.Required.Material.fDistortionStrength);
			break;

		case EFFECT_MODULE_TYPE::SPAWN:
			Module.Spawn.fRatePerSecond =
				Line.Float("rate", Module.Spawn.fRatePerSecond);
			Module.Spawn.iBurstCount =
				Line.UInt("burst", Module.Spawn.iBurstCount);
			Module.Spawn.iMaxParticles =
				Line.UInt("max", Module.Spawn.iMaxParticles);
			break;

		case EFFECT_MODULE_TYPE::LIFETIME:
			ReadFloatDistribution(Line, Module.Lifetime.Lifetime);
			break;

		case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
			ReadVectorDistribution(Line, Module.InitialLocation.Location);
			Module.InitialLocation.eShape = TextEnum(
				Line.Text("shape"), LOCATION_SHAPE_NAMES,
				Module.InitialLocation.eShape);
			Module.InitialLocation.fRadius =
				Line.Float("radius", Module.InitialLocation.fRadius);
			Module.InitialLocation.fInnerRadius =
				Line.Float("inner", Module.InitialLocation.fInnerRadius);
			Module.InitialLocation.fHeight =
				Line.Float("height", Module.InitialLocation.fHeight);
			Module.InitialLocation.isSurfaceOnly =
				Line.Bool("surface", Module.InitialLocation.isSurfaceOnly);
			break;

		case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
			ReadVectorDistribution(Line, Module.InitialVelocity.Velocity);
			break;

		case EFFECT_MODULE_TYPE::INITIAL_SIZE:
			ReadVectorDistribution(Line, Module.InitialSize.Size);
			break;

		case EFFECT_MODULE_TYPE::INITIAL_COLOR:
			ReadColorDistribution(Line, Module.InitialColor.Color);
			break;

		case EFFECT_MODULE_TYPE::SIZE_OVER_LIFE:
			return ParseCurve(Line.Text("curve"), Module.SizeOverLife.Curve);

		case EFFECT_MODULE_TYPE::ALPHA_OVER_LIFE:
			return ParseCurve(Line.Text("curve"), Module.AlphaOverLife.Curve);

		case EFFECT_MODULE_TYPE::VELOCITY_OVER_LIFE:
			return ParseCurve(
				Line.Text("curve"), Module.VelocityOverLife.Curve);

		case EFFECT_MODULE_TYPE::COLLISION:
			ParseVec3(Line.Text("min"), Module.Collision.vMinBounds);
			ParseVec3(Line.Text("max"), Module.Collision.vMaxBounds);
			Module.Collision.fRestitution =
				Line.Float("restitution", Module.Collision.fRestitution);
			break;

		case EFFECT_MODULE_TYPE::EVENT:
			Module.Event.strEventName =
				Line.Text("event", Module.Event.strEventName);
			Module.Event.fNormalizedTime =
				Line.Float("time", Module.Event.fNormalizedTime);
			break;

		case EFFECT_MODULE_TYPE::LOD:
			Module.LOD.fNearDistance =
				Line.Float("near", Module.LOD.fNearDistance);
			Module.LOD.fFarDistance =
				Line.Float("far", Module.LOD.fFarDistance);
			Module.LOD.fFarSpawnScale =
				Line.Float("farscale", Module.LOD.fFarSpawnScale);
			break;

		case EFFECT_MODULE_TYPE::INITIAL_ROTATION:
			ReadVectorDistribution(
				Line, Module.InitialRotation.RotationDegrees);
			break;

		case EFFECT_MODULE_TYPE::ROTATION_RATE:
			ReadVectorDistribution(
				Line, Module.RotationRate.RotationRateDegreesPerSecond);
			break;

		case EFFECT_MODULE_TYPE::COLOR_OVER_LIFE:
			return ParseCurve(Line.Text("r"), Module.ColorOverLife.Red) &&
				ParseCurve(Line.Text("g"), Module.ColorOverLife.Green) &&
				ParseCurve(Line.Text("b"), Module.ColorOverLife.Blue) &&
				ParseCurve(Line.Text("a"), Module.ColorOverLife.Alpha);

		case EFFECT_MODULE_TYPE::SUB_UV:
			Module.SubUV.iColumns =
				Line.UInt("cols", Module.SubUV.iColumns);
			Module.SubUV.iRows = Line.UInt("rows", Module.SubUV.iRows);
			Module.SubUV.fFramesPerSecond =
				Line.Float("fps", Module.SubUV.fFramesPerSecond);
			Module.SubUV.iStartFrame =
				Line.UInt("start", Module.SubUV.iStartFrame);
			Module.SubUV.iEndFrame =
				Line.UInt("end", Module.SubUV.iEndFrame);
			Module.SubUV.isLoop = Line.Bool("loop", Module.SubUV.isLoop);
			Module.SubUV.useParticleRelativeTime =
				Line.Bool("reltime", Module.SubUV.useParticleRelativeTime);
			break;

		case EFFECT_MODULE_TYPE::MESH_ANIMATION:
			Module.MeshAnimation.iAnimationIndex =
				Line.UInt("anim", Module.MeshAnimation.iAnimationIndex);
			Module.MeshAnimation.fPlayRate =
				Line.Float("rate", Module.MeshAnimation.fPlayRate);
			Module.MeshAnimation.isLoop =
				Line.Bool("loop", Module.MeshAnimation.isLoop);
			break;

		case EFFECT_MODULE_TYPE::BEAM:
			ParseVec3(Line.Text("src"), Module.Beam.vSourceOffset);
			ParseVec3(Line.Text("dst"), Module.Beam.vTargetOffset);
			Module.Beam.fWidth = Line.Float("width", Module.Beam.fWidth);
			Module.Beam.iSegments =
				Line.UInt("segments", Module.Beam.iSegments);
			Module.Beam.fNoiseAmplitude =
				Line.Float("noise", Module.Beam.fNoiseAmplitude);
			break;

		case EFFECT_MODULE_TYPE::TRAIL:
			Module.Trail.fWidth = Line.Float("width", Module.Trail.fWidth);
			Module.Trail.fPointLifetime =
				Line.Float("ptlife", Module.Trail.fPointLifetime);
			Module.Trail.iMaxPoints =
				Line.UInt("maxpts", Module.Trail.iMaxPoints);
			Module.Trail.strSourceBone = Line.Text("srcbone");
			Module.Trail.strTargetBone = Line.Text("dstbone");
			break;

		case EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER:
		{
			const string* pX = Line.Find("x");
			const string* pY = Line.Find("y");
			const string* pZ = Line.Find("z");
			const string* pW = Line.Find("w");
			return (nullptr == pX ||
				ParseCurve(*pX, Module.DynamicParameter.X)) &&
				(nullptr == pY ||
					ParseCurve(*pY, Module.DynamicParameter.Y)) &&
				(nullptr == pZ ||
					ParseCurve(*pZ, Module.DynamicParameter.Z)) &&
				(nullptr == pW ||
					ParseCurve(*pW, Module.DynamicParameter.W));
		}

		default:
			return false;
		}
		return true;
	}

	bool ValidateLimits(const EFFECT_ASSET_DESC& Asset, string* pError)
	{
		auto IsFiniteFloat = [](const f32_t fValue)
		{
			return std::isfinite(fValue);
		};
		auto IsFinite2 = [&IsFiniteFloat](const float2_t& vValue)
		{
			return IsFiniteFloat(vValue.x) && IsFiniteFloat(vValue.y);
		};
		auto IsFinite3 = [&IsFiniteFloat](const float3_t& vValue)
		{
			return IsFiniteFloat(vValue.x) && IsFiniteFloat(vValue.y) &&
				IsFiniteFloat(vValue.z);
		};
		auto IsFinite4 = [&IsFiniteFloat](const float4_t& vValue)
		{
			return IsFiniteFloat(vValue.x) && IsFiniteFloat(vValue.y) &&
				IsFiniteFloat(vValue.z) && IsFiniteFloat(vValue.w);
		};
		auto IsFiniteCurve = [&IsFiniteFloat](
			const EFFECT_CURVE_FLOAT_DESC& Curve)
		{
			return all_of(Curve.Keys.begin(), Curve.Keys.end(),
				[&IsFiniteFloat](const EFFECT_CURVE_KEY& Key)
				{
					return IsFiniteFloat(Key.fTime) &&
						IsFiniteFloat(Key.fValue);
				});
		};

		if (!IsFiniteFloat(Asset.fDuration) ||
			!IsFiniteFloat(Asset.fWarmupTime))
		{
			if (pError) *pError = "Effect asset contains a non-finite value.";
			return false;
		}

		if (Asset.Emitters.size() > EFFECT_MAX_EMITTERS)
		{
			if (pError) *pError = "Too many effect emitters.";
			return false;
		}

		for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
		{
			if (!IsFiniteFloat(Emitter.fDelay) ||
				!IsFiniteFloat(Emitter.fDuration))
			{
				if (pError) *pError =
					"Effect emitter contains a non-finite value.";
				return false;
			}

			if (Emitter.Modules.size() >
				EFFECT_MAX_MODULES_PER_EMITTER)
			{
				if (pError) *pError =
					"Too many modules in an effect emitter.";
				return false;
			}

			for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
			{
				if (Module.Spawn.iMaxParticles >
						EFFECT_MAX_PARTICLES_PER_EMITTER ||
					Module.Trail.iMaxPoints >
						EFFECT_MAX_TRAIL_POINTS ||
					Module.Beam.iSegments >
						EFFECT_MAX_BEAM_SEGMENTS ||
					Module.SubUV.iColumns >
						EFFECT_MAX_SUBUV_DIMENSION ||
					Module.SubUV.iRows >
						EFFECT_MAX_SUBUV_DIMENSION)
				{
					if (pError) *pError =
						"Effect module exceeds a runtime safety limit.";
					return false;
				}

				const EFFECT_MATERIAL_DESC& Material =
					Module.Required.Material;
				const bool_t hasFiniteMaterial =
					IsFinite2(Material.vUVTiling) &&
					IsFinite2(Material.vUVOffset) &&
					IsFinite2(Material.vUVPanner) &&
					IsFiniteFloat(Material.fEmissiveStrength) &&
					IsFiniteFloat(Material.fOpacityMaskThreshold) &&
					IsFiniteFloat(Material.fDissolveAmount) &&
					IsFiniteFloat(Material.fDissolveEdgeWidth) &&
					IsFinite4(Material.vDissolveEdgeColor) &&
					IsFiniteFloat(Material.fSoftParticleDistance) &&
					IsFiniteFloat(Material.fDistortionStrength);

				const bool_t hasFiniteCurves =
					IsFiniteCurve(Module.SizeOverLife.Curve) &&
					IsFiniteCurve(Module.AlphaOverLife.Curve) &&
					IsFiniteCurve(Module.VelocityOverLife.Curve) &&
					IsFiniteCurve(Module.ColorOverLife.Red) &&
					IsFiniteCurve(Module.ColorOverLife.Green) &&
					IsFiniteCurve(Module.ColorOverLife.Blue) &&
					IsFiniteCurve(Module.ColorOverLife.Alpha) &&
					IsFiniteCurve(Module.DynamicParameter.X) &&
					IsFiniteCurve(Module.DynamicParameter.Y) &&
					IsFiniteCurve(Module.DynamicParameter.Z) &&
					IsFiniteCurve(Module.DynamicParameter.W);

				const bool_t hasFiniteDistributions =
					IsFiniteFloat(Module.Lifetime.Lifetime.fConstant) &&
					IsFiniteFloat(Module.Lifetime.Lifetime.fMin) &&
					IsFiniteFloat(Module.Lifetime.Lifetime.fMax) &&
					IsFinite3(Module.InitialLocation.Location.vConstant) &&
					IsFinite3(Module.InitialLocation.Location.vMin) &&
					IsFinite3(Module.InitialLocation.Location.vMax) &&
					IsFinite3(Module.InitialVelocity.Velocity.vConstant) &&
					IsFinite3(Module.InitialVelocity.Velocity.vMin) &&
					IsFinite3(Module.InitialVelocity.Velocity.vMax) &&
					IsFinite3(Module.InitialSize.Size.vConstant) &&
					IsFinite3(Module.InitialSize.Size.vMin) &&
					IsFinite3(Module.InitialSize.Size.vMax) &&
					IsFinite4(Module.InitialColor.Color.vConstant) &&
					IsFinite4(Module.InitialColor.Color.vMin) &&
					IsFinite4(Module.InitialColor.Color.vMax);

				if (!hasFiniteMaterial || !hasFiniteCurves ||
					!hasFiniteDistributions ||
					!IsFiniteFloat(Module.Spawn.fRatePerSecond) ||
					!IsFiniteFloat(Module.InitialLocation.fRadius) ||
					!IsFiniteFloat(Module.InitialLocation.fInnerRadius) ||
					!IsFiniteFloat(Module.InitialLocation.fHeight) ||
					!IsFiniteFloat(Module.SubUV.fFramesPerSecond) ||
					!IsFiniteFloat(Module.MeshAnimation.fPlayRate) ||
					!IsFiniteFloat(Module.Beam.fWidth) ||
					!IsFiniteFloat(Module.Beam.fNoiseAmplitude) ||
					!IsFiniteFloat(Module.Trail.fWidth) ||
					!IsFiniteFloat(Module.Trail.fPointLifetime))
				{
					if (pError) *pError =
						"Effect module contains a non-finite value.";
					return false;
				}
			}
		}
		return true;
	}
}

bool_t Client::CEffect_AssetIO::Save_Authoring(
	const filesystem::path& Path, const EFFECT_ASSET_DESC& Asset,
	string* pError)
{
	if (!ValidateLimits(Asset, pError))
		return false;
	if (!PrepareParent(Path, pError))
		return false;
	ofstream Stream(Path, ios::binary | ios::trunc);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open authoring output.";
		return false;
	}

	Stream << AUTHORING_MAGIC << " " << EFFECT_ASSET_SCHEMA_VERSION << "\n"
		<< "ASSET id=" << Quote(Asset.strAssetId)
		<< " name=" << Quote(Asset.strName)
		<< " provenance=" << EnumText(Asset.eProvenance, PROVENANCE_NAMES)
		<< " duration=" << FloatText(Asset.fDuration)
		<< " warmup=" << FloatText(Asset.fWarmupTime) << "\n";

	for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
	{
		Stream << "\nEMITTER id=" << Emitter.iEmitterId
			<< " name=" << Quote(Emitter.strName)
			<< " type=" << EnumText(Emitter.eType, EMITTER_TYPE_NAMES)
			<< " enabled=" << (Emitter.isEnabled ? 1 : 0)
			<< " space=" << EnumText(
				Emitter.eSimulationSpace, SIMULATION_SPACE_NAMES)
			<< " blend=" << EnumText(Emitter.eBlendMode, BLEND_MODE_NAMES)
			<< " sort=" << EnumText(Emitter.eSortMode, SORT_MODE_NAMES)
			<< " delay=" << FloatText(Emitter.fDelay)
			<< " duration=" << FloatText(Emitter.fDuration)
			<< " loops=" << Emitter.iLoopCount << "\n";

		for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
		{
			Stream << "  MODULE id=" << Module.iModuleId
				<< " name=" << Quote(Module.strName)
				<< " type=" << EnumText(Module.eType, MODULE_TYPE_NAMES)
				<< " enabled=" << (Module.isEnabled ? 1 : 0)
				<< ModulePayloadText(Module) << "\n";
		}
	}
	return Stream.good();
}

bool_t Client::CEffect_AssetIO::Load_Authoring(
	const filesystem::path& Path, EFFECT_ASSET_DESC& OutAsset,
	string* pError)
{
	ifstream Stream(Path, ios::binary);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open authoring input.";
		return false;
	}

	string Line;
	const vector<string> Header = getline(Stream, Line)
		? SplitTokens(Line)
		: vector<string>{};
	if (Header.size() < 2 || Header[0] != AUTHORING_MAGIC)
	{
		if (pError) *pError = "Not a LostArk effect authoring file.";
		return false;
	}
	char_t* pSchemaEnd = nullptr;
	const uint32_t iSourceSchemaVersion = static_cast<uint32_t>(
		strtoul(Header[1].c_str(), &pSchemaEnd, 10));
	if (pSchemaEnd == Header[1].c_str() || '\0' != *pSchemaEnd ||
		!IsSupportedSchemaVersion(iSourceSchemaVersion))
	{
		if (pError)
		{
			*pError = "Unsupported authoring schema version '" + Header[1] +
				"'. Supported versions are 4 and " +
				to_string(EFFECT_ASSET_SCHEMA_VERSION) + ".";
		}
		return false;
	}

	EFFECT_ASSET_DESC Asset;
	Asset.Emitters.clear();
	while (getline(Stream, Line))
	{
		TOKEN_LINE Parsed;
		if (!ParseLine(Line, Parsed))
			continue;

		if ("ASSET" == Parsed.Kind)
		{
			Asset.strAssetId = Parsed.Text("id");
			Asset.strName = Parsed.Text("name");
			Asset.eProvenance = TextEnum(
				Parsed.Text("provenance"), PROVENANCE_NAMES,
				Asset.eProvenance);
			Asset.fDuration = Parsed.Float("duration", Asset.fDuration);
			Asset.fWarmupTime = Parsed.Float("warmup", Asset.fWarmupTime);
			continue;
		}

		if ("EMITTER" == Parsed.Kind)
		{
			if (Asset.Emitters.size() >= EFFECT_MAX_EMITTERS)
			{
				if (pError) *pError = "Too many effect emitters.";
				return false;
			}
			EFFECT_EMITTER_DESC Emitter;
			Emitter.iEmitterId = Parsed.UInt64("id", Emitter.iEmitterId);
			Emitter.strName = Parsed.Text("name");
			Emitter.eType = TextEnum(
				Parsed.Text("type"), EMITTER_TYPE_NAMES, Emitter.eType);
			Emitter.isEnabled = Parsed.Bool("enabled", Emitter.isEnabled);
			Emitter.eSimulationSpace = TextEnum(
				Parsed.Text("space"), SIMULATION_SPACE_NAMES,
				Emitter.eSimulationSpace);
			Emitter.eBlendMode = TextEnum(
				Parsed.Text("blend"), BLEND_MODE_NAMES, Emitter.eBlendMode);
			Emitter.eSortMode = TextEnum(
				Parsed.Text("sort"), SORT_MODE_NAMES, Emitter.eSortMode);
			Emitter.fDelay = Parsed.Float("delay", Emitter.fDelay);
			Emitter.fDuration = Parsed.Float("duration", Emitter.fDuration);
			Emitter.iLoopCount = Parsed.UInt("loops", Emitter.iLoopCount);
			Asset.Emitters.push_back(move(Emitter));
			continue;
		}

		if ("MODULE" == Parsed.Kind)
		{
			if (Asset.Emitters.empty())
			{
				if (pError) *pError = "MODULE appears before any EMITTER.";
				return false;
			}
			EFFECT_EMITTER_DESC& Emitter = Asset.Emitters.back();
			if (Emitter.Modules.size() >= EFFECT_MAX_MODULES_PER_EMITTER)
			{
				if (pError) *pError =
					"Too many modules in an effect emitter.";
				return false;
			}
			EFFECT_MODULE_DESC Module;
			Module.iModuleId = Parsed.UInt64("id", Module.iModuleId);
			Module.strName = Parsed.Text("name");
			Module.eType = TextEnum(
				Parsed.Text("type"), MODULE_TYPE_NAMES,
				EFFECT_MODULE_TYPE::END);
			Module.isEnabled = Parsed.Bool("enabled", Module.isEnabled);
			if (!ReadModulePayload(Parsed, Module))
			{
				if (pError) *pError =
					"Unknown or malformed module: " + Parsed.Text("type");
				return false;
			}
			Emitter.Modules.push_back(move(Module));
			continue;
		}
	}

	if (!ValidateLimits(Asset, pError))
		return false;
	Asset.iSchemaVersion = EFFECT_ASSET_SCHEMA_VERSION;
	OutAsset = move(Asset);
	return true;
}

bool_t Client::CEffect_AssetIO::Save_Binary(
	const filesystem::path& Path, const EFFECT_ASSET_DESC& Asset,
	string* pError)
{
	if (!ValidateLimits(Asset, pError))
		return false;
	if (!PrepareParent(Path, pError))
		return false;
	const vector<uint8_t> Payload = Serialize(Asset);
	if (Payload.size() > WEFFECT_MAX_PAYLOAD_BYTES)
	{
		if (pError) *pError = "Effect payload is too large to cook.";
		return false;
	}
	ofstream Stream(Path, ios::binary | ios::trunc);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open .weffect output.";
		return false;
	}
	const uint32_t iSchema = EFFECT_ASSET_SCHEMA_VERSION;
	const uint32_t iSize = static_cast<uint32_t>(Payload.size());
	const uint32_t iHash = Hash(Payload);
	Stream.write(reinterpret_cast<const char*>(&WEFFECT_MAGIC), 4);
	Stream.write(reinterpret_cast<const char*>(&iSchema), 4);
	Stream.write(reinterpret_cast<const char*>(&iSize), 4);
	Stream.write(reinterpret_cast<const char*>(&iHash), 4);
	Stream.write(reinterpret_cast<const char*>(Payload.data()), iSize);
	return Stream.good();
}

bool_t Client::CEffect_AssetIO::Load_Binary(
	const filesystem::path& Path, EFFECT_ASSET_DESC& OutAsset,
	string* pError)
{
	ifstream Stream(Path, ios::binary);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open .weffect input.";
		return false;
	}

	uint32_t iMagic = {}, iSchema = {}, iSize = {}, iHash = {};
	if (!Stream.read(reinterpret_cast<char*>(&iMagic), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iSchema), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iSize), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iHash), 4))
	{
		if (pError) *pError = "Truncated .weffect header.";
		return false;
	}
	if (iMagic != WEFFECT_MAGIC)
	{
		if (pError) *pError = "Invalid .weffect magic.";
		return false;
	}
	if (!IsSupportedSchemaVersion(iSchema))
	{
		if (pError)
		{
			*pError = "Unsupported .weffect schema version " +
				to_string(iSchema) + ". Supported versions are 4 and " +
				to_string(EFFECT_ASSET_SCHEMA_VERSION) + ".";
		}
		return false;
	}
	if (iSize > WEFFECT_MAX_PAYLOAD_BYTES)
	{
		if (pError) *pError = ".weffect payload exceeds the safety limit.";
		return false;
	}
	vector<uint8_t> Payload(iSize);
	if (!Stream.read(reinterpret_cast<char*>(Payload.data()), iSize))
	{
		if (pError) *pError = "Truncated .weffect payload.";
		return false;
	}
	if (Hash(Payload) != iHash)
	{
		if (pError) *pError = ".weffect payload hash mismatch.";
		return false;
	}
	if (!Deserialize(Payload, OutAsset, iSchema))
	{
		if (pError)
		{
			*pError = "Invalid .weffect payload for schema " +
				to_string(iSchema) + ".";
		}
		return false;
	}
	return true;
}

bool_t Client::CEffect_AssetIO::Validate_RoundTrip(
	const EFFECT_ASSET_DESC& Asset, string* pError)
{
	if (!ValidateLimits(Asset, pError))
		return false;
	const vector<uint8_t> Before = Serialize(Asset);
	EFFECT_ASSET_DESC Loaded;
	if (!Deserialize(Before, Loaded))
	{
		if (pError) *pError = "Memory deserialize failed.";
		return false;
	}
	const vector<uint8_t> After = Serialize(Loaded);
	if (Before != After)
	{
		if (pError) *pError = "Round trip bytes differ.";
		return false;
	}
	return true;
}
