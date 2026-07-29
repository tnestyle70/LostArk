#include "Effect_AssetIO.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
	using namespace Client;

	constexpr uint32_t WEFFECT_MAGIC = 0x54434645u; // EFCT

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

	vector<uint8_t> Serialize(const EFFECT_ASSET_DESC& Asset)
	{
		BYTE_WRITER Writer;
		Writer.Pod(Asset.iSchemaVersion);
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
				Writer.Pod(Module.Spawn.fRatePerSecond);
				Writer.Pod(Module.Spawn.iBurstCount);
				Writer.Pod(Module.Spawn.iMaxParticles);
				WriteFloatDistribution(Writer,
					Module.Lifetime.Lifetime);
				WriteVectorDistribution(Writer,
					Module.InitialLocation.Location);
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
			}
		}
		return Writer.Bytes;
	}

	bool Deserialize(const vector<uint8_t>& Bytes,
		EFFECT_ASSET_DESC& OutAsset)
	{
		BYTE_READER Reader{ Bytes };
		EFFECT_ASSET_DESC Asset;
		uint32_t iEmitterCount = {};
		if (!Reader.Pod(Asset.iSchemaVersion) ||
			Asset.iSchemaVersion != EFFECT_ASSET_SCHEMA_VERSION ||
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
					!Reader.Pod(Module.Spawn.fRatePerSecond) ||
					!Reader.Pod(Module.Spawn.iBurstCount) ||
					!Reader.Pod(Module.Spawn.iMaxParticles) ||
					!ReadFloatDistribution(Reader,
						Module.Lifetime.Lifetime) ||
					!ReadVectorDistribution(Reader,
						Module.InitialLocation.Location) ||
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

	string ToHex(const vector<uint8_t>& Bytes)
	{
		ostringstream Stream;
		Stream << hex << setfill('0');
		for (const uint8_t iByte : Bytes)
			Stream << setw(2) << static_cast<uint32_t>(iByte);
		return Stream.str();
	}

	bool FromHex(const string& Text, vector<uint8_t>& OutBytes)
	{
		if (0 != Text.size() % 2)
			return false;
		OutBytes.clear();
		OutBytes.reserve(Text.size() / 2);
		for (size_t i = 0; i < Text.size(); i += 2)
		{
			char* pEnd = nullptr;
			const string Pair = Text.substr(i, 2);
			const unsigned long iValue = strtoul(Pair.c_str(), &pEnd, 16);
			if (nullptr == pEnd || '\0' != *pEnd)
				return false;
			OutBytes.push_back(static_cast<uint8_t>(iValue));
		}
		return true;
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

	bool ValidateLimits(const EFFECT_ASSET_DESC& Asset, string* pError)
	{
		if (Asset.Emitters.size() > EFFECT_MAX_EMITTERS)
		{
			if (pError) *pError = "Too many effect emitters.";
			return false;
		}

		for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
		{
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
			}
		}
		return true;
	}
}

bool_t Client::CEffect_AssetIO::Save_Json(
	const filesystem::path& Path, const EFFECT_ASSET_DESC& Asset,
	string* pError)
{
	if (!ValidateLimits(Asset, pError))
		return false;
	if (!PrepareParent(Path, pError))
		return false;
	const vector<uint8_t> Payload = Serialize(Asset);
	ofstream Stream(Path, ios::binary | ios::trunc);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open JSON output.";
		return false;
	}
	Stream << "{\n"
		<< "  \"format\": \"LostArkEffectAuthoring\",\n"
		<< "  \"schemaVersion\": " << Asset.iSchemaVersion << ",\n"
		<< "  \"assetId\": \"" << Asset.strAssetId << "\",\n"
		<< "  \"name\": \"" << Asset.strName << "\",\n"
		<< "  \"emitterCount\": " << Asset.Emitters.size() << ",\n"
		<< "  \"canonicalPayloadHex\": \"" << ToHex(Payload) << "\"\n"
		<< "}\n";
	return Stream.good();
}

bool_t Client::CEffect_AssetIO::Load_Json(
	const filesystem::path& Path, EFFECT_ASSET_DESC& OutAsset,
	string* pError)
{
	ifstream Stream(Path, ios::binary);
	if (!Stream)
	{
		if (pError) *pError = "Cannot open JSON input.";
		return false;
	}
	const string Text((istreambuf_iterator<char>(Stream)),
		istreambuf_iterator<char>());
	const string Key = "\"canonicalPayloadHex\": \"";
	const size_t iBegin = Text.find(Key);
	if (string::npos == iBegin)
	{
		if (pError) *pError = "JSON payload field is missing.";
		return false;
	}
	const size_t iValueBegin = iBegin + Key.size();
	const size_t iEnd = Text.find('"', iValueBegin);
	vector<uint8_t> Payload;
	if (string::npos == iEnd ||
		!FromHex(Text.substr(iValueBegin, iEnd - iValueBegin), Payload) ||
		!Deserialize(Payload, OutAsset))
	{
		if (pError) *pError = "JSON payload is invalid.";
		return false;
	}
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
	uint32_t iMagic = {}, iSchema = {}, iSize = {}, iHash = {};
	if (!Stream.read(reinterpret_cast<char*>(&iMagic), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iSchema), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iSize), 4) ||
		!Stream.read(reinterpret_cast<char*>(&iHash), 4) ||
		iMagic != WEFFECT_MAGIC ||
		iSchema != EFFECT_ASSET_SCHEMA_VERSION ||
		iSize > 64u * 1024u * 1024u)
	{
		if (pError) *pError = "Invalid .weffect header.";
		return false;
	}
	vector<uint8_t> Payload(iSize);
	if (!Stream.read(reinterpret_cast<char*>(Payload.data()), iSize) ||
		Hash(Payload) != iHash || !Deserialize(Payload, OutAsset))
	{
		if (pError) *pError = "Invalid .weffect payload.";
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
