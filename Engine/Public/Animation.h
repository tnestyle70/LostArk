#pragma once

#include "Engine_Defines.h"

#include <span>

NS_BEGIN(Engine)

struct MODEL_ANIMATION_DATA;

class CAnimation final
{
private:
	CAnimation();
public:
	~CAnimation();

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones);
	HRESULT Initialize(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	bool_t	Update_TransformationMatrix(f32_t fTimeDelta, const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop);
	bool_t Compare_Name(const char_t* pName) const { return !strcmp(pName, m_szName); }
	const char_t* Get_Name() const { return m_szName; }
	f32_t Get_Duration() const { return m_fDuration; }
	f32_t Get_TickPerSecond() const { return m_fTickPerSecond; }
	f32_t Get_CurrentTrackPosition() const { return m_fCurrentTrackPosition; }
	void Set_TrackPosition(f32_t fTrackPosition);

private:
	friend class CModel;
	bool_t Sample_LocalBoneTransforms(
		f32_t fTrackPosition,
		std::span<float4x4_t> InOutLocalTransforms) const;

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
	static shared_ptr<CAnimation> Create(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	shared_ptr<CAnimation> Clone();
};

NS_END
