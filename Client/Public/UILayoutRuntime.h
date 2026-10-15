#pragma once

// Client의 LEVEL·이름 공간과 Engine의 수치·COM 타입을 사용한다.
#include "Client_Defines.h"
#include "Engine_AnimationTypes.h"
#include "Engine_Defines.h"

// slot은 vector로 순서를 보존하고 map은 문서·label, unordered_map은 ID별 index를 찾는다.
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

NS_BEGIN(Client)
// smart pointer 멤버의 실제 생성·파괴는 CPP에서 정의하므로 여기서는 이름만 선언한다.
class CUI_Sprite;
class CUITextureCache;

// Data 루트의 UI JSON 슬롯을 CUI_Sprite로 만들어 지정 Level/Layer에 등록한다.
// 화면별 소유자가 이 객체를 보유하고, Renderer는 등록된 sprite를 UI 그룹에서 그린다.
class CUILayoutRuntime final
{
public:
	// view 소유자가 생성한다. 장치 COM 참조와 문서 경로를 보관한 뒤 Load를 호출한다.
	CUILayoutRuntime(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex, const wstring_t& strLayerTag,
		const wstring_t& strDocumentPath);
	// 멤버 참조는 해제하지만 Layer의 sprite까지 자동 제거하지 않는다. 필요하면 Release_Sprites를 먼저 호출한다.
	~CUILayoutRuntime();
	// 화면 소유자가 sprite를 숨기고 Layer에서 제거할 때 호출한다. 공용 Texture 원형은 남긴다.
	void Release_Sprites();

public:
	// ID의 문서 기준 사각형을 돌려준다. 이미지가 없어도 위치용 슬롯은 조회할 수 있다.
	bool_t Get_SlotRect(const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	// 문서 전체의 UI 정렬 layer를 지정한다. 현재 sprite와 이후 만드는 sprite에 함께 적용한다.
	void Set_UISortLayer(int32_t iLayer);
	// 표시 변경은 색·텍스처를 보존한다. keyframe의 실제 표시 여부는 Update가 다시 결정한다.
	void Set_SlotVisible(const string& strId, bool_t bVisible);
	// 컷신 중 표시 허용 여부와 렌더 그룹을 해당 슬롯의 sprite 전체에 전달한다.
	void Set_SlotCinematicOverlay(const string& strId, bool_t overlay);
	void Set_SlotScenePresentation(const string& strId, bool_t enabled);
	// 외부 연출 시계의 label·초 위치를 즉시 샘플링한다. 잘못된 시간·label이면 false다.
	bool_t Sample_KeyframeAnimation(const string& strId, const string& label, f32_t seconds);
    // 이미지가 없는 위치 슬롯에 caption sprite를 만든다. 이미 sprite가 있거나 글꼴 측정 실패면 false다.
    bool_t Set_SlotCaption(const string& strId, const wstring_t& caption, const wstring_t& font);
	// 기본·추가 정적 이미지의 RGBA를 바꾼다. keyframe의 원래 색은 별도로 갱신된다.
	void Set_SlotTint(const string& strId, const float4_t& vTint);
	// 정적·keyframe 이미지의 원래 색에 곱한다. 흰색이면 원래 색을 유지한다.
	void Set_SlotTintMultiplier(const string& strId, const float4_t& vTintMultiplier);
	// 정적 이미지의 RGB를 흰색으로 지정하면서 alpha를 바꾼다.
	void Set_SlotAlpha(const string& strId, f32_t fAlpha);
	// Resources 상대 경로를 캐시에서 읽어 기본 이미지에 덮어쓴다. 빈 경로는 원래 Component 텍스처로 복귀한다.
	void Set_SlotTexture(const string& strId, const string& strAssetPath);
	// 초상화 Render Target 등 호출자가 만든 SRV를 기본 이미지에 보유한다. nullptr는 원래 이미지로 복귀한다.
	void Set_SlotTextureSRV(const string& strId, ComPtr<ID3D11ShaderResourceView> pSRV);
	// 추가된 순서의 슬롯 ID 복사본이다. 창 전체 이동·숨김 목록으로 사용한다.
	vector<string> Get_SlotIds() const;
	// ID의 좌상단 위치를 바꾸고 정적 이미지 중심을 다시 계산한다. 알 수 없는 ID는 무시한다.
	void Set_SlotPosition(const string& strId, f32_t fX, f32_t fY);
	// 문서 기준 좌상단과 크기를 함께 바꾼다. Get_SlotRect에도 변경값이 반영된다.
	void Set_SlotRect(const string& strId, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	// 기본 이미지의 가로 표시 비율이다. 크기 변경 없이 셰이더에서 잘라낸다.
	void Set_SlotFillRatio(const string& strId, f32_t fFillRatio);
	// 기본 이미지의 12시부터 시계 방향 부채꼴 표시 비율이다.
	void Set_SlotArcRatio(const string& strId, f32_t fArcRatio);
	// 기본 이미지에서 읽을 UV 영역을 바꿔 미니맵을 이동·확대한다.
	void Set_SlotUVWindow(const string& strId, f32_t fOffsetU, f32_t fOffsetV,
		f32_t fScaleU, f32_t fScaleV);
	// 기본 이미지의 UV 영역을 중심 기준으로 회전한다. 단위는 radian이다.
	void Set_SlotUVRotation(const string& strId, f32_t fRadians, f32_t fAspect);
	// 기본·추가 정적 이미지를 중심 기준 시계 방향 degree로 회전한다.
	void Set_SlotRotation(const string& strId, f32_t fDegrees);
	// ownerClass가 있는 슬롯만 선택 class와 일치할 때 표시한다. 공용 슬롯은 건드리지 않는다.
	void Set_ActiveOwnerClass(const string& strOwnerClass);
	// 화면 소유자가 중지될 때 남아 있는 모든 sprite를 함께 숨기거나 다시 표시한다.
	void Set_AllSlotsVisible(bool_t bVisible);
	// 없는 ID만 문서 기준 사각형과 이미지로 보충한다. 이미 등록된 슬롯은 유지한다.
	// 같은 UI 정렬 layer에서는 제출 순서가 앞뒤를 결정하므로, 덮을 이미지는 뒤에 생성해야 한다.
	void Ensure_RuntimeSlot(const string& strId, f32_t fX, f32_t fY,
		f32_t fWidth, f32_t fHeight, const string& strTexturePath);
	// label부터 다음 label 전까지 재생한다. 반복 문서는 구간을 돌고 단발 문서는 끝 프레임을 유지한다.
	bool_t Play_KeyframeAnimation(const string& strId, const string& strLabel);
	// 화면 소유자가 초 단위 delta를 전달한다. flipbook 텍스처와 keyframe별 이미지 상태를 갱신한다.
	void Update(f32_t fTimeDelta);
	// 이벤트 발생 시 flipbook의 시간과 이미지를 0번으로 되감는다.
	void Restart_Animation(const string& strId);
	// flipbook의 현재 프레임을 범위 안으로 맞춘다. 이후 Update를 호출하면 다시 진행한다.
	void Set_Animation_Frame(const string& strId, int32_t iFrame);
	// flipbook 목록과 재생 상태를 교체한다. 빈 목록이면 현재 텍스처를 그대로 두고 재생만 멈춘다.
	void Set_SlotAnimation(const string& strId, const vector<string>& Frames,
		f32_t fFps, bool_t bLoop);
	// 이미 로드한 SRV를 경로별 캐시에 보관해 연출 재생 때 재사용한다.
	void Adopt_Texture(const string& strPath, ComPtr<ID3D11ShaderResourceView> pSRV);

	f32_t Get_ResolutionWidth() const { return m_fResolutionWidth; }
	f32_t Get_ResolutionHeight() const { return m_fResolutionHeight; }

private:
	struct KEYFRAME_ANIM_KEY
	{
		int32_t	iFrame = 0;
		// 빈 asset은 이 프레임부터 해당 레이어를 숨기라는 뜻이다.
		string	strAsset;
		f32_t	fX = 0.f, fY = 0.f;
		f32_t	fScaleX = 1.f, fScaleY = 1.f;
		f32_t	fRotationDeg = 0.f;
		f32_t	fAlpha = 1.f;
		// 원본 colorTransform의 RGB 배율이다. alpha와 함께 sprite tint에 전달한다.
		f32_t	fTintR = 1.f, fTintG = 1.f, fTintB = 1.f;
		bool_t	bAdditive = false;
		bool_t	bFlipX = false;
	};

	struct KEYFRAME_ANIM_LAYER
	{
		vector<KEYFRAME_ANIM_KEY> Keys;
	};

	// 문서별 frame rate·label 시작 frame·레이어 key 목록을 캐시에 보관한다.
	struct KEYFRAME_ANIM_DOCUMENT
	{
		bool_t				isLoaded = false;
		bool_t				bLoop = false;
		f32_t				fFrameRate = 40.f;
		int32_t				iFrameCount = 0;
		map<string, int32_t>			Labels;
		vector<KEYFRAME_ANIM_LAYER>		Layers;
	};

	// JSON의 ID·문서 좌표와 실행 중 표시·애니메이션 상태를 소유한다. sprite는 Layer와 함께 참조한다.
	struct RUNTIME_SLOT
	{
		string				strId;
		string				strOwnerClass;
		f32_t				fX = 0.f, fY = 0.f, fSizeX = 0.f, fSizeY = 0.f;
		bool_t				bVisible = true;
		shared_ptr<CUI_Sprite>	pSprite;
		// 같은 사각형 위에 생성 순서대로 겹치는 추가 이미지들이다.
		vector<shared_ptr<CUI_Sprite>>	ExtraLayerSprites;

		vector<string>		AnimationFramePaths;
		f32_t				fAnimationFPS = 10.f;
		bool_t				bAnimationLoop = true;
		f32_t				fAnimationElapsed = 0.f;
		int32_t				iAnimationFrame = -1;

		// keyframe 문서의 재생 구간과 경과 시간이다. 음수 시간은 아직 재생하지 않은 상태다.
		string				strKeyframeAnimationPath;
		f32_t				fKeyframeAnimationScale = 1.f;
		int32_t				iKeyframeWindowStart = 0;
		int32_t				iKeyframeWindowEnd = 0;
		f64_t				fKeyframeElapsedSeconds = -1.0;
		vector<shared_ptr<CUI_Sprite>>	KeyframeSprites;
	};

private:
	// 중복 ID 후보 중 일반 슬롯·flipbook·keyframe에 맞는 항목을 고르는 조건이다.
	enum class SLOT_REQUIREMENT { ANY, FLIPBOOK, KEYFRAME };
	// slot 값을 순서대로 보관하고 ID 검색용 index 목록을 함께 갱신한다.
	void Append_Slot(RUNTIME_SLOT Slot);
	RUNTIME_SLOT* Find_Slot(const string& strId,
		SLOT_REQUIREMENT eRequirement = SLOT_REQUIREMENT::ANY);
	const RUNTIME_SLOT* Find_Slot(const string& strId,
		SLOT_REQUIREMENT eRequirement = SLOT_REQUIREMENT::ANY) const;
	// Data 루트의 JSON을 읽으며 유효한 슬롯을 바로 추가한다. 파일·파싱 실패도 S_OK를 반환한다.
	HRESULT Load();
	// 이미지 경로를 STATIC Texture 원형 tag로 등록한다. 이 인스턴스는 같은 경로를 다시 시도하지 않는다.
	void Ensure_TexturePrototype(const wstring_t& strPath);
	// UI 원형을 복제해 지정 Layer에 넣고 같은 sprite의 shared_ptr를 반환한다.
	shared_ptr<CUI_Sprite> Create_Sprite(
		f32_t fRectX, f32_t fRectY, f32_t fRectWidth, f32_t fRectHeight,
		const string& strTexturePath);
	// Data 상대 경로의 애니메이션 문서를 캐시한다. 실패도 캐시해 같은 인스턴스에서 재로드하지 않는다.
	const KEYFRAME_ANIM_DOCUMENT* Get_Or_Load_KeyframeAnimation(const string& strPath);
	// 재생 frame의 key를 골라 이미지·위치·색을 적용한다. 숨긴 슬롯도 경과 시간은 증가한다.
	void Update_KeyframeSlot(RUNTIME_SLOT& Slot, f32_t fTimeDelta);

private:
	// 장치 COM 참조와 sprite를 등록할 대상 Level/Layer를 보관한다.
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	uint32_t						m_iGameObjectLevelIndex = 0;
	wstring_t						m_strLayerTag;
	wstring_t						m_strDocumentPath;
	f32_t							m_fResolutionWidth = 1280.f;
	f32_t							m_fResolutionHeight = 720.f;

	int32_t							m_iUISortLayer = 0;
	vector<RUNTIME_SLOT>			m_Slots;
	// 슬롯은 m_Slots가 소유하고 이 map은 ID별 index만 보관한다. 중복 ID는 조건에 맞는 첫 슬롯을 찾는다.
	std::unordered_map<string, vector<size_t>> m_SlotIndices;
	vector<wstring_t>				m_RegisteredTexturePrototypes;
	map<string, KEYFRAME_ANIM_DOCUMENT>	m_KeyframeAnimationCache;
	// 실행 중 이미지 교체와 애니메이션 frame의 SRV 캐시를 단독 소유한다.
	unique_ptr<CUITextureCache>	m_pTextureCache;
};

NS_END
