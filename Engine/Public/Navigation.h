#pragma once

#include "Component.h"
#include "PathFinder.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	enum class MODE : uint8_t
	{
		LEGACY_TRIANGLE,
		NAVGRID_ASTAR,
	};

	typedef struct tagNavigationDesc
	{
		int32_t		iStartCellIndex = { -1 };
		shared_ptr<class CTransform> pTransformCom;
	} NAVIGATION_DESC;

private:
	CNavigation(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CNavigation(const CNavigation& Prototype);

public:
	virtual ~CNavigation();

public:
	virtual HRESULT Initialize_Prototype(
		const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile);
	virtual HRESULT Initialize(void* pArg) override;

public:
	MODE Get_Mode() const { return m_eMode; }

	HRESULT SetUp_Neighbors();
	HRESULT SetUp_Neighbors(const tchar_t* pNeighborDataFile);
	bool_t isMove(fvector_t vResultPos);
	void SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform);

	PATH_RESULT_CODE Find_Path(
		fvector_t vStartPosition,
		fvector_t vGoalPosition,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath,
		uint32_t iMaxExpandedNodes = 16384,
		uint32_t* pOutExpandedNodes = nullptr);

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif

private:
	HRESULT Initialize_NavGrid_Prototype(
		const tchar_t* pNavGridFilePath);
	void Simplify_Path(
		const vector<float3_t>& RawPath,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath) const;
	void Round_PathCorners(
		const vector<float3_t>& Path,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath) const;

private:
	MODE									m_eMode = { MODE::LEGACY_TRIANGLE };
	vector<shared_ptr<class CCell>>			m_Cells;
	int32_t									m_iCurrentCellIndex = { -1 };
	shared_ptr<const CNavGrid>				m_pNavGrid = { nullptr };
	unique_ptr<CPathFinder>					m_pPathFinder = { nullptr };

private:
	shared_ptr<class CTransform>			m_pTargetTransformCom = {};

#ifdef _DEBUG
private:
	shared_ptr<class CShader>				m_pShader = { nullptr };
	shared_ptr<PrimitiveBatch<VertexPositionColor>> m_pBatch = { nullptr };
	shared_ptr<BasicEffect>					m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout>				m_pInputLayout = { nullptr };
	vector<f32_t>							m_DebugCellHeights;
	vector<float3_t>						m_DebugPath;
#endif

public:
	static unique_ptr<CNavigation> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile = nullptr);

	static unique_ptr<CNavigation> Create_NavGrid(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pNavGridFilePath);

	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
