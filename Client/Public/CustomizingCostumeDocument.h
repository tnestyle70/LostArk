#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* Data/UI/Customizing/CustomizingCostumes.json: which equipment visual set each of the five
try-on costumes stands for, per class, in the order the retail table's Object_Unit gives -- the
same order as that class's costume icon row in CustomizingIcons.json.

This document only names sets. The parts themselves live in
Data/Actors/EquipmentPresentationCatalog.json and are put on the model by
CEquipmentPresentationService, so a set named here that the catalog does not carry is reported
and skipped rather than faked. */
class CCustomizingCostumeDocument final
{
public:
	/* The costume and hairstyle documents have the same shape -- a class map whose entries hold
	one ordered array of {index, visualSetId} -- so one reader serves both. strDocument is the
	Data-relative path and strArrayName the array key inside each class entry. */
	CCustomizingCostumeDocument(
		std::string strSchema, std::string strDocument, std::string strArrayName);
	/* Reads and validates the whole document; on failure the previous contents are kept and
	Get_Status explains why. */
	bool_t Load();
	/* The class's five visual set ids, indexed by Object_Unit. Null when the class is absent. */
	const std::vector<std::string>* Find(const std::string& strAssetId) const;
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string, std::vector<std::string>> m_Classes;
	std::string m_strSchema;
	std::string m_strDocument;
	std::string m_strArrayName;
	std::string m_strStatus = "Customizing set document is not loaded";
};

NS_END
