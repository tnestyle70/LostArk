#pragma once

#include "Engine_Defines.h"
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

struct MODEL_ANIMATION_CHANNEL_DATA;

class CChannel
{
private:
	CChannel();
public:
	~CChannel();

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones);
	HRESULT Initialize(const MODEL_ANIMATION_CHANNEL_DATA& channel,
		const vector<shared_ptr<class CBone>>& Bones);
	void Update_TransformationMatrix(f32_t fCurrentTrackPosition, const vector<shared_ptr<class CBone>>& Bones, uint32_t* pLeftKeyFrameIndex);
private:
	char_t					m_szName[MAX_PATH] = {};

	uint32_t				m_iNumKeyFrames = {};
	vector<KEYFRAME>		m_KeyFrames;
	vector<MODEL_VECTOR_KEY_DATA>	m_PositionKeys;
	vector<MODEL_QUAT_KEY_DATA>	m_RotationKeys;
	vector<MODEL_VECTOR_KEY_DATA>	m_ScaleKeys;
	bool_t					m_bUsesSeparateTracks = { false };

	int32_t				m_iBoneIndex = { -1 };
	// uint32_t			m_iLeftKeyFrameIndex = { 0 };

public:
	static shared_ptr<CChannel> Create(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones);
	static shared_ptr<CChannel> Create(const MODEL_ANIMATION_CHANNEL_DATA& channel,
		const vector<shared_ptr<class CBone>>& Bones);
};

NS_END

