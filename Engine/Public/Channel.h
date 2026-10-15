#pragma once

// 본 포즈의 수학 타입과 키 배열의 소유 컨테이너를 사용한다.
#include "Engine_Defines.h"
// Assimp 경로가 사용하는 결합 KEYFRAME의 S/R/T 구조다.
#include "Engine_AnimationTypes.h"
// WAnimation의 위치·회전·스케일 분리 키 타입을 값으로 보관한다.
#include "BinaryAsset/ModelAssetData.h"

// clone별 scale/rotation/position 탐색 커서 세 개를 전달한다.
#include <array>

struct aiNodeAnim;

NS_BEGIN(Engine)

struct MODEL_ANIMATION_CHANNEL_DATA;

// 본 하나의 애니메이션 키를 소유한다. 초기화 후 키를 공유하고 재생 위치·커서는 CAnimation clone이 소유한다.
class CChannel
{
private:
	CChannel();
public:
	~CChannel();

public:
	// Assimp 키를 같은 index의 결합 KEYFRAME으로 복사하고 이름으로 본을 찾는다. Create 실패 시 준비 중인 채널을 폐기한다.
	HRESULT Initialize(
		const aiNodeAnim* pAIChannel,
		const vector<shared_ptr<class CBone>>& Bones);
	// 이미 해석된 본 index를 검사하고 compact track을 복사한다. 키 시간·값의 전체 검증은 입력 로드 계층의 책임이다.
	HRESULT Initialize(const MODEL_ANIMATION_CHANNEL_DATA& channel,
		const vector<shared_ptr<class CBone>>& Bones);
	// 애니메이션 재생이 tick 위치와 clone별 커서를 넘긴다. 대상 본의 local만 갱신하며 유효한 본 배열은 호출자가 보장한다.
	void Update_TransformationMatrix(
		f32_t fCurrentTrackPosition,
		const vector<shared_ptr<class CBone>>& Bones,
		uint32_t* pLeftKeyFrameIndex,
		std::array<uint32_t, 3>* pSeparateTrackIndices = nullptr);

private:
	friend class CAnimation;
	// CAnimation의 명시 시점 샘플용이다. 재생 커서나 본을 바꾸지 않고 성공할 때만 두 출력 값을 저장한다.
	bool_t Sample_TransformationMatrix(
		f32_t fTrackPosition,
		uint32_t& iOutBoneIndex,
		float4x4_t& OutTransformationMatrix) const;

private:
	char_t					m_szName[MAX_PATH] = {};

	// 결합 키와 분리 키 중 초기화 경로에 해당하는 저장소를 소비한다. 키 데이터는 채널이 소유한다.
	uint32_t				m_iNumKeyFrames = {};
	vector<KEYFRAME>		m_KeyFrames;
	vector<MODEL_VECTOR_KEY_DATA>	m_PositionKeys;
	vector<MODEL_QUAT_KEY_DATA>	m_RotationKeys;
	vector<MODEL_VECTOR_KEY_DATA>	m_ScaleKeys;
	bool_t					m_bUsesSeparateTracks = { false };

	// 소유 모델의 본 배열에 대한 해석 결과다. 포인터를 저장하지 않아 clone 본 배열에도 같은 index를 적용한다.
	int32_t				m_iBoneIndex = { -1 };
	// uint32_t			m_iLeftKeyFrameIndex = { 0 };

public:
	// 초기화 성공한 채널만 애니메이션에 넘긴다. 실패 시 복사 중인 키 배열도 함께 정리한다.
	static shared_ptr<CChannel> Create(
		const aiNodeAnim* pAIChannel,
		const vector<shared_ptr<class CBone>>& Bones);
	static shared_ptr<CChannel> Create(const MODEL_ANIMATION_CHANNEL_DATA& channel,
		const vector<shared_ptr<class CBone>>& Bones);
};

NS_END

