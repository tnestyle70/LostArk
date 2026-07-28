#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CAnimation final
{
private:
	CAnimation();
public:
	~CAnimation();

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones);
	bool_t	Update_TransformationMatrix(f32_t fTimeDelta, const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop);

private:
	char_t				m_szName[MAX_PATH] = {};
	f32_t				m_fDuration = {}; // 현재 애니메이션의 전체 길이. 
	f32_t				m_fTickPerSecond = {}; // 초당 애니메이션의 재생 속도. 
	f32_t				m_fCurrentTrackPosition = {}; // 현재 재생 위치. 
	
private:
	uint32_t							m_iNumChannels = {};
	vector<shared_ptr<class CChannel>>	m_Channels;
	vector<uint32_t>					m_iLeftKeyFrameIndices;


public:
	static shared_ptr<CAnimation> Create(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones);
	shared_ptr<CAnimation> Clone();
};

NS_END