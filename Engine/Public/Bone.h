#pragma once

#include "Engine_Defines.h"

/* assimp : 뼈 -> aiNode, aiBone, aiNodeAnim */
/* aiNode : 뼈의 상속 구조 */ 
/* aiBone : 이 뼈가 어떤 정점에게, 얼마나 영향을 주는지? */
/* aiNodeAnim : 채널, 이 뼈가 특정 애니메이션 내에서 재생위치에 따라 어떤 상태들을 취해야하는가 */

NS_BEGIN(Engine)

class CBone final
{
private:
	CBone();
public:
	~CBone();

public:
	matrix_t Get_CombinedTransformationMatrix() const {
		return XMLoadFloat4x4(&m_CombinedTransformationMatrix);
	}
public:
	HRESULT Initialize(const aiNode* pAINode, int32_t iParentBoneIndex);
	void Update_TransformationMatrix(fmatrix_t TransformationMatrix);
	void Update_CombinedTransformationMatrix(const vector<shared_ptr<CBone>>& Bones, fmatrix_t PreTransformMatrix);

public:
	bool_t Compare_Name(const char_t* pName) {
		return !strcmp(pName, m_szName);
	}

private:
	char_t				m_szName[MAX_PATH] = {};	
	int32_t				m_iParentBoneIndex = { -1 };
	float4x4_t			m_TransformationMatrix = {};
	float4x4_t			m_CombinedTransformationMatrix = {};

public:
	static shared_ptr<CBone> Create(const aiNode* pAINode, int32_t iParentBoneIndex);
	shared_ptr<CBone> Clone();
};

NS_END