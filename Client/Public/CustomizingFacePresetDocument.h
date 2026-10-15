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

// 얼굴 preset 하나의 source ID와 morph 이름/가중치 목록을 소유한다. 0 가중치도 보존한다.
// 이름은 저작 표기 그대로이며 실제 morph 적용기는 대소문자 차이를 무시해 찾는다.
struct CUSTOMIZING_FACE_PRESET
{
	std::string strSourcePresetId;
	std::vector<std::pair<std::string, f32_t>> Morphs;
};

// Data/UI/Customizing/CustomizingFacePresets.json의 class별 얼굴 morph preset 목록을 소유한다.
// CustomizingView가 같은 index의 얼굴 아이콘과 연결해 Character의 morph 가중치로 전달한다.
class CCustomizingFacePresetDocument final
{
public:
	// 전체 목록을 임시로 읽고 성공 시 교체한다. 명시적 실패는 기존 목록을 유지하며 상태 문구에 이유를 남긴다.
	bool_t Load();
	// class가 없으면 nullptr다. 반환 목록은 다음 Load 성공이나 문서 소멸 전까지만 차용한다.
	const std::vector<CUSTOMIZING_FACE_PRESET>* Find(const std::string& strAssetId) const;
	// 마지막 Load 결과를 차용한다. 다음 Load가 같은 문자열을 바꿀 수 있다.
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string, std::vector<CUSTOMIZING_FACE_PRESET>> m_Classes;
	std::string m_strStatus = "Customizing face preset document is not loaded";
};

NS_END
