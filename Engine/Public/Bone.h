#pragma once

// 본의 행렬·벡터 타입과 모델이 소유하는 shared_ptr 배열을 사용한다.
#include "Engine_Defines.h"

/* assimp : 뼈 -> aiNode, aiBone, aiNodeAnim */
/* aiNode : 뼈의 상속 구조 */ 
/* aiBone : 이 뼈가 어떤 정점에게, 얼마나 영향을 주는지? */
/* aiNodeAnim : 채널, 이 뼈가 특정 애니메이션 내에서 재생위치에 따라 어떤 상태들을 취해야하는가 */

// Assimp 계층 입력은 이름만 선언하고 실제 구조는 CPP에서 읽는다.
struct aiNode;

NS_BEGIN(Engine)

struct MODEL_BONE_DATA;

// CModel의 한 본에 대한 이름·부모 index·local/combined 행렬을 소유한다.
// clone마다 포즈 값이 독립이며 업데이트 순서와 부모 index의 유효성은 모델이 보장한다.
class CBone final
{
private:
	CBone();
public:
	~CBone();

public:
	// 이름은 본 수명 동안 차용한다. 행렬 getter는 현재 저장된 값을 복사하며 자동으로 계층을 갱신하지 않는다.
	const char_t* Get_Name() const
	{
		return m_szName;
	}
	matrix_t Get_CombinedTransformationMatrix() const
	{
		return XMLoadFloat4x4(&m_CombinedTransformationMatrix);
	}
	matrix_t Get_TransformationMatrix() const
	{
		return XMLoadFloat4x4(&m_TransformationMatrix);
	}
	int32_t Get_ParentBoneIndex() const
	{
		return m_iParentBoneIndex;
	}
public:
	// Create가 새 본을 구성한다. Assimp 입력은 유효한 포인터가 필요하며 부모 index는 여기서 검증하지 않는다.
	HRESULT Initialize(const aiNode* pAINode, int32_t iParentBoneIndex);
	// 변환된 모델의 이름 길이를 검사한 뒤 rest local과 부모 index를 복사한다.
	HRESULT Initialize(const MODEL_BONE_DATA& bone);
	// 채널 샘플이 local 행렬만 바꾼다. combined 갱신은 이후 모델의 계층 순회가 맡는다.
	void Update_TransformationMatrix(fmatrix_t TransformationMatrix);
	// FromMatrix에서 현재 local로 S/R/T를 보간한다. 분해 실패는 기존 local을 보존하며 weight 범위는 호출자가 정한다.
	void Blend_TransformationMatrix(fmatrix_t FromMatrix, f32_t fWeight);
	// 루트는 local×PreTransform, 자식은 local×부모 combined를 저장한다. 부모가 먼저 갱신되어 있어야 한다.
	void Update_CombinedTransformationMatrix(
		const vector<shared_ptr<CBone>>& Bones,
		fmatrix_t PreTransformMatrix);
	/* CModel::Pose_BonesFrom이 착용 파츠의 대응 본에 이미 결합된 몸체 포즈를 복사한다.
	local 행렬은 유지하므로 파츠 자체의 자식 계층 계산에 쓰일 local 값은 바뀌지 않는다. */
	void Set_CombinedTransformationMatrix(fmatrix_t CombinedTransformationMatrix)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, CombinedTransformationMatrix);
	}

public:
	// 유효한 C 문자열을 대소문자 구분으로 비교한다. 이름 포인터를 보관하지 않는다.
	bool_t Compare_Name(const char_t* pName)
	{
		return !strcmp(pName, m_szName);
	}

private:
	// 이름과 포즈는 값 소유다. -1은 루트이며 이외 index는 CModel의 동일한 본 배열을 가리킨다.
	char_t				m_szName[MAX_PATH] = {};	
	int32_t				m_iParentBoneIndex = { -1 };
	float4x4_t			m_TransformationMatrix = {};
	float4x4_t			m_CombinedTransformationMatrix = {};

public:
	// 초기화 실패 시 임시 본을 정리하고 nullptr를 반환한다. 성공한 본만 모델 배열에 등록한다.
	static shared_ptr<CBone> Create(const aiNode* pAINode, int32_t iParentBoneIndex);
	static shared_ptr<CBone> Create(const MODEL_BONE_DATA& bone);
	// 현재 이름·계층 index·포즈를 복사한다. 원본 본 객체를 공유하지 않는다.
	shared_ptr<CBone> Clone();
};

NS_END
