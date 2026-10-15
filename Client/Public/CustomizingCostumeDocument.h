#pragma once

// Client 공통 정의와 이름 공간을 사용한다.
#include "Client_Defines.h"
// bool_t와 숫자 타입·이름 공간 매크로를 사용한다.
#include "Engine_Defines.h"

// asset ID와 상태 문구를 값으로 보관한다.
#include <string>
// class asset ID별로 읽은 목록을 찾는다.
#include <unordered_map>
// 아이콘 순서와 대응하는 목록을 값으로 소유한다.
#include <vector>

NS_BEGIN(Client)

// CustomizingCostumes/CustomizingHairstyles 문서의 class별 visualSetId 순서와 기본 선택을 소유한다.
// 실제 부위 모델의 조회·장착은 EquipmentPresentationService가 맡는다.
class CCustomizingCostumeDocument final
{
public:
	// schema·Data 상대 문서 경로·배열 key를 값으로 받아 의상과 헤어 문서에 같은 reader를 사용한다.
	CCustomizingCostumeDocument(
		std::string strSchema, std::string strDocument, std::string strArrayName);
	// 전체 목록과 기본 선택을 임시로 읽는다. 명시적 실패는 기존 값을 유지하고 상태 문구만 바꾼다.
	bool_t Load();
	// class가 없으면 nullptr다. 목록은 다음 Load 성공이나 문서 소멸 전까지만 차용하며 개수를 5개로 강제하지 않는다.
	const std::vector<std::string>* Find(const std::string& strAssetId) const;
	// defaultVisualSetId를 기존 icon 순서의 index로 찾는다. defaultBodyHair=true는 -1, 기본 지정/해당 class 기록이 없으면 0이다.
	int32_t Get_DefaultIndex(const std::string& strAssetId) const;
	// 마지막 Load 결과를 차용한다. 다음 Load가 같은 문자열을 바꿀 수 있다.
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string, std::vector<std::string>> m_Classes;
	// 아이콘 목록 순서를 바꾸지 않고 class별 최초 선택을 별도로 보관한다.
	std::unordered_map<std::string, int32_t> m_DefaultIndices;
	// 생성 때 받은 문서 계약을 소유하며 이후 Load에도 같은 값으로 검사한다.
	std::string m_strSchema;
	std::string m_strDocument;
	std::string m_strArrayName;
	std::string m_strStatus = "Customizing set document is not loaded";
};

NS_END
