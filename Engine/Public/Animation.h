#pragma once

// 클립 이름·재생 수치와 공유 채널의 소유 컨테이너를 사용한다.
#include "Engine_Defines.h"

// CModel이 소유한 임시 포즈/경계 배열을 복사 없이 차용한다.
#include <span>
// 본별 translation 세 축과 clone별 분리 track 커서 세 개를 표현한다.
#include <array>

struct aiAnimation;

NS_BEGIN(Engine)

struct MODEL_ANIMATION_DATA;

// CModel이 소유하는 클립 재생 상태다. 채널 키는 공유해도 시계와 탐색 커서는 clone마다 독립적으로 보관한다.
class CAnimation final
{
private:
	CAnimation();
public:
	~CAnimation();

public:
	// 변환된 클립의 런타임 시계다. 패키지에 원본 import 속도가 남아 있어도
	// 메타데이터 소비자는 실제 재생 시간 환산에 같은 속도를 사용해야 한다.
	static constexpr f32_t COOKED_TICK_RATE = 30.f;
	// Create가 유효한 Assimp 입력을 복사한다. 채널 준비 도중 실패하면 factory의 임시 객체 정리로 복구한다.
	HRESULT Initialize(
		const aiAnimation* pAIAnimation,
		const vector<shared_ptr<class CBone>>& Bones);
	// 변환된 클립의 이름/양수 길이/속도를 검사한 뒤 채널을 준비한다. 런타임 속도는 COOKED_TICK_RATE를 사용한다.
	HRESULT Initialize(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	// 모델 재생이 초 단위 delta를 넘긴다. 시계를 진행하고 본 local을 갱신하며 true는 비반복 재생의 끝 도달이다.
	// false는 정상 진행/반복 재생 또는 입력 거부를 포함하므로 일반적인 성공 여부로 사용하지 않는다.
	bool_t	Update_TransformationMatrix(
		f32_t fTimeDelta,
		const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop);
	// 이름은 대소문자를 구분하고 반환 포인터는 객체 수명 동안 차용한다. duration/current 위치는 tick, 속도는 tick/초다.
	bool_t Compare_Name(const char_t* pName) const
	{
		return !strcmp(pName, m_szName);
	}
	const char_t* Get_Name() const
	{
		return m_szName;
	}
	f32_t Get_Duration() const
	{
		return m_fDuration;
	}
	f32_t Get_TickPerSecond() const
	{
		return m_fTickPerSecond;
	}
	f32_t Get_CurrentTrackPosition() const
	{
		return m_fCurrentTrackPosition;
	}
	// 명시 seek의 tick 위치를 비교식으로 범위 제한하고 결합 키 커서를 초기화한다. 입력 NaN 검사는 별도로 하지 않는다.
	void Set_TrackPosition(f32_t fTrackPosition);
    // World Sequence처럼 외부 시점으로 샘플하는 clone의 명시적 선택이다.
    // 불변 compact 채널만 local 표본을 공유하고 각 clone의 시계는 유지한다.
    void Enable_SampleReuse();

private:
	friend class CModel;
    // CModel과 재생 함수가 쓰는 시계 처리다. 음수 delta·반복 wrap 시 결합 키 커서를 되감으며 본 포즈는 직접 바꾸지 않는다.
    bool_t Advance_Clock(f32_t fTimeDelta, bool_t isLoop);
    // WModel compact track만 지원한다. 활성 포즈를 샘플하지 않고 키의 경계를 출력에 누적한다.
    bool_t Accumulate_TransformEnvelope(std::span<std::array<double, 3>> translations,
        std::span<double> scales) const;
	// 지정 본의 키 성분을 1e-7 허용오차로 비교한다. 채널 부재는 true, 같은 본의 중복 채널은 false다.
	bool_t Is_BoneTransformConstant(uint32_t iBoneIndex) const;
	// CModel의 임시 local 배열에 지정 tick을 샘플한다. 키 없는 본은 유지되며 나중 채널 실패 시 앞선 출력 쓰기는 남는다.
	// 현재 시계·활성 본은 바꾸지 않으므로 호출자가 임시 배열을 검증한 뒤 적용한다.
	bool_t Sample_LocalBoneTransforms(
		f32_t fTrackPosition,
		std::span<float4x4_t> InOutLocalTransforms) const;

private:
	char_t				m_szName[MAX_PATH] = {};
	f32_t				m_fDuration = {}; // 현재 애니메이션의 전체 길이. 
	f32_t				m_fTickPerSecond = {}; // 초당 애니메이션의 재생 속도. 
	f32_t				m_fCurrentTrackPosition = {}; // 현재 재생 위치. 
	
private:
	// 채널 참조는 clone과 공유할 수 있지만 아래 커서 vector와 재생 시계는 값으로 복사된다.
	uint32_t							m_iNumChannels = {};
	vector<shared_ptr<class CChannel>>	m_Channels;
	vector<uint32_t>					m_iLeftKeyFrameIndices;
	// 실제 샘플하는 clip에서만 확보한다. clone별 커서는 공유 track을 변경하지 않는다.
	vector<std::array<uint32_t, 3>> m_SeparateTrackIndices;
    // 동일한 불변 채널 묶음의 local 표본 한 개를 잠금 아래 공유한다. 전체 모델의 combined 포즈를 공유하지 않는다.
    struct SAMPLE_REUSE;
    // 표본 mutex는 공유 표본만 보호한다. 같은 clone의 시계/커서와 대상 Bones를 동시에 갱신하지 않도록 모델 호출자가 관리한다.
    shared_ptr<SAMPLE_REUSE> m_pSampleReuse;


public:
	// 초기화 실패한 클립은 준비된 채널과 함께 정리한다. 모델에는 완성된 클립만 반환한다.
	static shared_ptr<CAnimation> Create(
		const aiAnimation* pAIAnimation,
		const vector<shared_ptr<class CBone>>& Bones);
	static shared_ptr<CAnimation> Create(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	// 채널/표본 소유 참조는 공유하고 재생 시계/커서 값은 복사해 독립 재생 상태를 만든다.
	shared_ptr<CAnimation> Clone();
};

NS_END
