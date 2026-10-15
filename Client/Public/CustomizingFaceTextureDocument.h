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

// Data/UI/Customizing/CustomizingFaceTextures.json의 class·category별 texture ID를 아이콘 index에 맞춰 소유한다.
// CustomizingView는 iris를 눈 diffuse로, 화장 stamp를 얼굴 재질 입력으로 Character에 전달한다. 이 문서는 GPU texture를 만들지 않는다.
class CCustomizingFaceTextureDocument final
{
public:
	// 전체 문서를 임시 목록에 읽는다. 명시적 실패는 기존 목록을 유지하고 상태 문구만 바꾼다.
	bool_t Load();
	// class/category가 없으면 nullptr다. 반환 vector는 문서가 소유하며 다음 Load 성공이나 문서 소멸 전까지만 차용한다.
	const std::vector<std::string>* Find(
		const std::string& strAssetId, const std::string& strCategory) const;
	// 마지막 Load 결과를 차용한다. 다음 Load가 같은 문자열을 바꿀 수 있다.
	const std::string& Get_Status() const { return m_strStatus; }

private:
	// class → category → 아이콘 index의 texture ID다. 제공되지 않은 texture 자리는 빈 문자열로 남는다.
	std::unordered_map<std::string,
		std::unordered_map<std::string, std::vector<std::string>>> m_Classes;
	std::string m_strStatus = "Customizing face texture document is not loaded";
};

NS_END
