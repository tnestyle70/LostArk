#include "FaceSliderDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <cmath>
#include <fstream>
#include <set>
#include <sstream>

namespace
{
	constexpr const char_t* SCHEMA = "lostark.face-sliders";
	constexpr int32_t FORMAT_VERSION = 1;

	bool_t Read_FiniteNumbers(
		const Client::DATA_JSON_VALUE* pValue, const size_t iCount, f32_t* pOut)
	{
		if (nullptr == pValue || !pValue->Is_Array() ||
			pValue->Get_Array().size() != iCount)
		{
			return false;
		}
		for (size_t i = 0; i < iCount; ++i)
		{
			const Client::DATA_JSON_VALUE& Element = pValue->Get_Array()[i];
			if (!Element.Is_Number())
				return false;
			const f32_t fValue = static_cast<f32_t>(Element.Get_Number());
			if (!std::isfinite(fValue))
				return false;
			pOut[i] = fValue;
		}
		return true;
	}

	bool_t Read_Bone(
		const Client::DATA_JSON_VALUE& Value, Client::FACE_SLIDER_BONE& outBone,
		std::string& outError)
	{
		const Client::DATA_JSON_VALUE* pName = Value.Find("bone");
		const Client::DATA_JSON_VALUE* pKeys = Value.Find("keys");
		if (nullptr == pName || !pName->Is_String() || pName->Get_String().empty())
		{
			outError = "bone entry has no bone name";
			return false;
		}
		if (nullptr == pKeys || !pKeys->Is_Array() ||
			pKeys->Get_Array().size() != Client::FACE_SLIDER_KEY_COUNT)
		{
			outError = "bone " + pName->Get_String() + " needs exactly 3 keys";
			return false;
		}
		outBone.strBoneName = pName->Get_String();
		for (size_t i = 0; i < Client::FACE_SLIDER_KEY_COUNT; ++i)
		{
			const Client::DATA_JSON_VALUE& Key = pKeys->Get_Array()[i];
			f32_t position[3] = {};
			f32_t rotation[4] = {};
			if (!Key.Is_Object() ||
				!Read_FiniteNumbers(Key.Find("position"), 3, position) ||
				!Read_FiniteNumbers(Key.Find("rotation"), 4, rotation))
			{
				outError = "bone " + pName->Get_String() + " key " +
					std::to_string(i) + " is malformed";
				return false;
			}
			const f32_t fLength = std::sqrt(
				rotation[0] * rotation[0] + rotation[1] * rotation[1] +
				rotation[2] * rotation[2] + rotation[3] * rotation[3]);
			if (fLength < 0.5f || fLength > 1.5f)
			{
				outError = "bone " + pName->Get_String() + " key " +
					std::to_string(i) + " rotation is not a unit quaternion";
				return false;
			}
			outBone.Keys[i].vPosition = float3_t(position[0], position[1], position[2]);
			outBone.Keys[i].vRotation = float4_t(
				rotation[0] / fLength, rotation[1] / fLength,
				rotation[2] / fLength, rotation[3] / fLength);
		}
		return true;
	}
}

bool_t Client::FACE_SLIDER_DOCUMENT::Load(const char_t* pRace, std::string& outError)
{
	if (nullptr == pRace || '\0' == pRace[0])
	{
		outError = "face slider race is empty";
		return false;
	}

	const std::filesystem::path relativePath =
		std::filesystem::path("Customizing/FaceSliders") /
		(std::string(pRace) + ".facesliders.json");
	const std::filesystem::path path = CProjectDataRoot::Resolve(relativePath);
	std::ifstream stream(path, std::ios::binary);
	if (!stream.is_open())
	{
		outError = "cannot open " + path.string();
		return false;
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	const std::string text = buffer.str();

	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, outError) || !root.Is_Object())
	{
		outError = path.string() + ": " + outError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pDocumentRace = root.Find("race");
	const DATA_JSON_VALUE* pNeutral = root.Find("neutralFrame");
	const DATA_JSON_VALUE* pSliders = root.Find("sliders");
	if (nullptr == pSchema || !pSchema->Is_String() || pSchema->Get_String() != SCHEMA ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		static_cast<int32_t>(pVersion->Get_Number()) != FORMAT_VERSION)
	{
		outError = path.string() + ": schema/formatVersion mismatch";
		return false;
	}
	if (nullptr == pDocumentRace || !pDocumentRace->Is_String() ||
		pDocumentRace->Get_String() != pRace)
	{
		outError = path.string() + ": race does not match " + pRace;
		return false;
	}
	if (nullptr == pNeutral || !pNeutral->Is_Number() ||
		static_cast<size_t>(pNeutral->Get_Number()) != FACE_SLIDER_NEUTRAL_KEY)
	{
		outError = path.string() + ": neutralFrame must be 1";
		return false;
	}
	if (nullptr == pSliders || !pSliders->Is_Array() || pSliders->Get_Array().empty())
	{
		outError = path.string() + ": sliders array is missing or empty";
		return false;
	}

	FACE_SLIDER_DOCUMENT staged;
	staged.strRace = pRace;
	std::set<std::string> seenIds;
	for (const DATA_JSON_VALUE& Value : pSliders->Get_Array())
	{
		const DATA_JSON_VALUE* pId = Value.Is_Object() ? Value.Find("id") : nullptr;
		const DATA_JSON_VALUE* pSequence = Value.Is_Object() ? Value.Find("sequence") : nullptr;
		const DATA_JSON_VALUE* pBones = Value.Is_Object() ? Value.Find("bones") : nullptr;
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() ||
			nullptr == pSequence || !pSequence->Is_String() ||
			nullptr == pBones || !pBones->Is_Array() || pBones->Get_Array().empty())
		{
			outError = path.string() + ": a slider entry is malformed";
			return false;
		}
		if (!seenIds.insert(pId->Get_String()).second)
		{
			outError = path.string() + ": duplicate slider id " + pId->Get_String();
			return false;
		}
		FACE_SLIDER slider;
		slider.strId = pId->Get_String();
		slider.strSequence = pSequence->Get_String();
		for (const DATA_JSON_VALUE& BoneValue : pBones->Get_Array())
		{
			FACE_SLIDER_BONE bone;
			std::string boneError;
			if (!BoneValue.Is_Object() || !Read_Bone(BoneValue, bone, boneError))
			{
				outError = path.string() + ": slider " + slider.strId + ": " + boneError;
				return false;
			}
			slider.Bones.push_back(std::move(bone));
		}
		staged.Sliders.push_back(std::move(slider));
	}

	*this = std::move(staged);
	return true;
}
