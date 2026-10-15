#pragma once

// 기존 Prototype/Clone component 수명과 device/context 참조를 사용한다.
#include "Component.h"
// NavGrid 소유 관계, 셀 경로 질의/결과와 clone별 A* 작업 공간을 사용한다.
#include "PathFinder.h"

NS_BEGIN(Engine)

// 기존 삼각형 이동과 NavGrid 경로 질의를 같은 component 경계에서 제공한다.
// Prototype/clone은 셀·grid를 공유하되 현재 삼각형 index·대상 Transform·PathFinder 진행 상태는 인스턴스별이다.
// 런타임 blocker는 공유 grid에 반영된다. 내부 잠금이 없으므로 구성/변경/질의를 동시에 수행하지 않는다.
class ENGINE_DLL CNavigation final : public CComponent
{
public:
	// 삼각형 모드는 현재 셀 이웃을 따라 이동을 검사하고, NavGrid 모드는 A*와 지면 샘플을 사용한다.
	enum class MODE : uint8_t
	{
		LEGACY_TRIANGLE,
		NAVGRID_ASTAR,
	};

	// 기존 삼각형 clone의 초기 셀과 배치할 Transform이다. Initialize가 Transform을 공유 소유하며 셀 중심으로 옮긴다.
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
	// Create의 기존 삼각형 파일 로드다. 셀을 차례로 추가하므로 실패 시 이미 추가한 셀을 rollback하지 않는다.
	// Create는 실패한 새 객체 전체를 폐기한다. 기존 인스턴스를 원상 보존하는 재로드 API는 아니다.
	virtual HRESULT Initialize_Prototype(
		const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile);
	// Clone의 모드별 초기화다. NavGrid는 grid/finder 존재만 확인하고 삼각형은 선택적으로 NAVIGATION_DESC를 소비한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// 초기화한 탐색 모드와 이 인스턴스의 기본 최대 단차(월드 거리 단위)를 조회한다.
	MODE Get_Mode() const
	{
		return m_eMode;
	}
	f32_t Get_MaxStepHeight() const
	{
		return m_fMaxStepHeight;
	}

	// 삼각형들의 정확히 일치하는 두 정점을 찾아 이웃 index를 구성한다. 허용 오차/비다양체 해소는 수행하지 않는다.
	HRESULT SetUp_Neighbors();
	// 파일의 셀별 이웃 index 세 개를 검증하고 차례로 저장한다. 중간 실패 시 앞서 저장한 이웃은 복원하지 않는다.
	HRESULT SetUp_Neighbors(const tchar_t* pNeighborDataFile);
	// 삼각형 모드에서 후보 위치를 포함하는 셀로 현재 index를 갱신한다. 실패는 현재 index를 유지하며 Transform은 바꾸지 않는다.
	bool_t isMove(fvector_t vResultPos);
	// 삼각형 현재 셀의 평면 Y로 전달받은 Transform을 맞춘다. XZ를 다른 셀 안으로 이동시키지는 않는다.
	void SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform);

	// 월드 시작/목표를 셀로 바꿔 검색하고 단순화/모서리 보정한 waypoint를 성공 시에만 OutPath로 교체한다.
	// 시작점이 detail region에 속하면 그 region의 grid/단차 정책으로 질의한다. 탐색량 출력은 실패 때도 갱신될 수 있다.
	PATH_RESULT_CODE Find_Path(
		fvector_t vStartPosition,
		fvector_t vGoalPosition,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath,
		uint32_t iMaxExpandedNodes = 16384,
		uint32_t* pOutExpandedNodes = nullptr);
	// 직업 공통 지면 타기팅이 요청한 XZ를 유지한 채 담당 셀의 Y만 읽는다.
	// 격자 밖/차단 셀은 다른 목표로 투영하지 않고 거절한다. 성공 시 outPosition에 지면 위치를 출력한다.
	// 셀을 찾은 뒤 Y 유한성 검사에서 실패하면 출력은 이미 바뀌어 있을 수 있다.
	bool_t Try_SampleWalkablePoint(
		fvector_t vWorldPosition,
		float3_t& outPosition) const;
	// 두 권위 위치가 같은 grid/layer의 실제 지면 위에 있는지 먼저 확인한다.
	// 이어 기존 선분 순회로 해당 grid의 단차/차단 정책을 검사한다. 양 끝의 XZ나 Y를 다른 위치로 보정하지 않는다.
	bool_t Is_GroundedSegmentContinuous(
		fvector_t vFromPosition,
		fvector_t vToPosition) const;
	// base grid 옆에 로드한 "<AreaId>.<regionId>" detail Navigation의 소유권을 넘겨받는다.
	// 해당 XZ의 질의는 선택된 region만 처리하고 막혔다고 base로 재시도하지 않는다. 겹친 층은 높이 hint로 구분한다.
	// 중첩 region은 거절한다. region별 단차를 사용하며 runtime blocker 없이 저작된 region을 연결하는 것이 호출 계약이다.
	// 이 함수 자체가 전달된 grid의 blocker 유무를 검사하지는 않는다. 인수는 실패해도 호출자에게 되돌아가지 않는다.
	HRESULT Attach_Region(unique_ptr<CNavigation> pRegion);
	// 이 인스턴스의 base grid에 차단물을 등록한다. region으로 자동 배분하지 않고 grid의 입력 검증 결과를 반환한다.
	bool_t Register_RuntimeBlocker(
		const std::string& blockerId,
		const vector<uint32_t>& cellIndices,
		bool_t initiallyActive = false);
	// base grid의 같은 ID를 활성/비활성화한다. 공유 grid를 쓰는 다른 clone에도 상태가 보인다.
	bool_t Set_RuntimeBlockerActive(
		const std::string& blockerId,
		bool_t active);
	// base grid의 runtime 등록을 비운다. region이나 원본 보행 데이터를 바꾸지 않는다.
	void Clear_RuntimeBlockers();
	// follower가 base grid의 blocker 변경을 확인한다. 삼각형/미준비 상태는 0이며 region revision을 합산하지 않는다.
	uint64_t Get_NavigationRevision() const;

#ifdef _DEBUG
public:
	// Debug에서 삼각형 셀만 표시한다. NavGrid 모드는 그리지 않고 S_OK를 반환한다.
	virtual HRESULT Render() override;
#endif

private:
	// Create_NavGrid가 새 grid와 finder를 준비해 교체한다. 입력/Load 실패는 기존 멤버를 교체하지 않는다.
	HRESULT Initialize_NavGrid_Prototype(
		const tchar_t* pNavGridFilePath,
		f32_t fMaxStepHeight);
	// 원시 경로에서 통과 가능한 가장 먼 점을 차례로 골라 중간 waypoint를 줄인다. 별도 출력 배열을 사용한다.
	void Simplify_Path(
		const vector<float3_t>& RawPath,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath) const;
	// 각 모서리의 곡선 후보를 지면/선분 검사한 뒤 채택하고 거절되면 원래 모서리를 유지한다.
	void Round_PathCorners(
		const vector<float3_t>& Path,
		f32_t fMaxStepHeight,
		vector<float3_t>& OutPath) const;
	// XZ가 겹치는 region 중 보행 가능한 높이가 hint Y에 가장 가까운 것을 차용한다. 동률은 등록 순서를 유지한다.
	CNavigation* Select_Region(f32_t fX, f32_t fZ, f32_t fHintY) const;

private:
	MODE									m_eMode = { MODE::LEGACY_TRIANGLE };
	vector<shared_ptr<class CCell>>			m_Cells;
	int32_t									m_iCurrentCellIndex = { -1 };
	// grid는 clone 간 공유되지만 변경되는 탐색 작업 공간은 각 인스턴스가 따로 소유한다.
	shared_ptr<CNavGrid>						m_pNavGrid = { nullptr };
	unique_ptr<CPathFinder>					m_pPathFinder = { nullptr };
	f32_t m_fMaxStepHeight = 0.6f;
	// region component는 부모가 소유한다. clone은 region component/finder를 복제하고 region grid는 계속 공유한다.
	vector<shared_ptr<CNavigation>> m_Regions;

private:
	shared_ptr<class CTransform>			m_pTargetTransformCom = {};

#ifdef _DEBUG
private:
	shared_ptr<class CShader>				m_pShader = { nullptr };
#endif

public:
	// 기존 삼각형 Prototype을 만든다. 초기화 실패는 nullptr이며 새 객체의 부분 상태를 함께 폐기한다.
	static unique_ptr<CNavigation> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile = nullptr);

	// 게시된 binary NavGrid를 쓰는 Prototype을 만든다. 반환 unique_ptr의 소유권은 호출자에게 있다.
	static unique_ptr<CNavigation> Create_NavGrid(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pNavGridFilePath,
		f32_t fMaxStepHeight = 0.6f);

	// 공유할 원본 데이터와 독립 탐색 상태를 복제한 뒤 Initialize한다. 실패한 clone은 폐기하고 Prototype은 유지한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
