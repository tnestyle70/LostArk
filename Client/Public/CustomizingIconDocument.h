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

// Data/UI/Customizing/CustomizingIcons.json의 class별 Resources 상대 이미지 ID 목록을 소유한다.
// CustomizingView가 배치와 이미지 적용을 맡으며 없는 class/목록은 빈 셀로 처리한다.
class CCustomizingIconDocument final
{
public:
	// 한 class의 각 탭 목록을 값으로 묶는다. GPU texture와 화면 rect는 보관하지 않는다.
	struct CLASS_ICONS
	{
		// 프리셋은 전체 외형(category 0), 기본 얼굴은 얼굴 모양(category 1)의 서로 다른 목록이다.
		std::vector<std::string> Presets;
		std::vector<std::string> FaceShapes;
		// 의상 아이콘 순서는 Costume 문서의 objectUnit과 대응한다. reader가 개수를 5개로 강제하지는 않는다.
		std::vector<std::string> Costumes;
		std::vector<std::string> Actions;
		std::vector<std::string> Backgrounds;
		// 각 보조 탭의 머리·홍채·아이라인·볼·입술 목록이다. 목록이 없는 눈썹은 여기에 저장하지 않는다.
		std::vector<std::string> HairShapes;
		std::vector<std::string> EyeIrises;
		std::vector<std::string> AdornEyeLine;
		std::vector<std::string> AdornTouch;
		std::vector<std::string> AdornLip;
	};

public:
	// 목록의 구조와 이미지 ID 형식을 검사한 뒤 전체를 교체한다. 명시적 실패는 기존 목록을 유지한다.
	bool_t Load();
	// class가 없으면 nullptr다. 반환값은 다음 Load 성공이나 문서 소멸 전까지만 차용한다.
	const CLASS_ICONS* Find(const std::string& strAssetId) const;
	// 마지막 Load 결과를 차용한다. 다음 Load가 같은 문자열을 바꿀 수 있다.
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string, CLASS_ICONS> m_Classes;
	std::string m_strStatus = "Customizing icon document is not loaded";
};

NS_END
