# G05 Waterpang entry full implementation
PLAN/RESULT implementation appendix. Generated from the saved files; new Client H/CPP and Shared H are registered in their vcxproj and filters. Data source/region use Client 96.DataFiles/Navigation None entries. No autonomous visual PASS.

## Shared/Public/Gameplay/MaharakaWaterpangContract.h

```cpp
#pragma once
#include <cstdint>

namespace LostArk::Shared
{
    // The existing stage instance identifies the entire admitted Waterpang intro.
    // S2C_WORLD_SEQUENCE_PLAY carries its future start on the 30 Hz Server clock.
    inline constexpr const char* MAHARAKA_WATERPANG_INTRO_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.intro15.stage";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TICK_HZ = 30u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COUNTDOWN_TICKS = 300u;
}
```

## Client/Public/MaharakaWaterpangPresentation.h

```cpp
#pragma once
#include "Client_Defines.h"
#include "WorldSequencePlayer.h"
#include "ValtanCinematicCameraController.h"
#include "Network/PacketMessages.h"

namespace Client
{
class CCamera_Free;
// Level-owned presentation only. A Server reservation is the sole start authority.
class CMaharakaWaterpangPresentation final
{
public:
    bool Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera);
    void Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play);
    void Update(float delta, uint32_t serverTick, bool editing);
    void Render() const;
    void Stop();
    void Suspend_ForAuthoring();
    ~CMaharakaWaterpangPresentation();
    const std::string& Get_Status() const { return m_Status; }
private:
    struct CUT { uint32_t startMs = 0; VALTAN_CINEMATIC_CAMERA_CUE cue; };
    bool Load_Camera();
    bool Start_World(float elapsedMs);
    CWorldSequencePlayer::TARGET_SET m_Targets;
    CWorldSequencePlayer m_World;
    std::weak_ptr<CCamera_Free> m_Camera;
    std::vector<CUT> m_Cuts;
    std::vector<std::string> m_Instances;
    uint32_t m_DurationMs = 0, m_StartTick = 0, m_LastTick = 0;
    float m_ClockMs = 0.f;
    float m_ReadinessWait = 0.f;
    float m_WorldClockMs = 0.f;
    int m_Countdown = 0;
    bool m_Ready = false, m_Scheduled = false, m_Started = false, m_Failed = false;
    std::string m_Status;
};
}
```

## Client/Private/MaharakaWaterpangPresentation.cpp

```cpp
#include "MaharakaWaterpangPresentation.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "Camera_Free.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "UILabelFont.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <map>

using namespace Client;
namespace
{
constexpr uint64_t CAMERA_OWNER = 0x4d48505741544552ull;
constexpr const char* AREA = "LV_OCN_EVENTIS_MHP";
constexpr const char* INTRO = "cutscene.maharaka.waterpang.source.intro15";
const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing camera field: ") + key);
    return *value;
}
std::string Text(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_String() || value.Get_String().empty()) throw std::runtime_error("Invalid camera string");
    return value.Get_String();
}
double Number(const DATA_JSON_VALUE& row, const char* key, double lo, double hi)
{
    const auto& value = Field(row,key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number()<lo || value.Get_Number()>hi)
        throw std::runtime_error(std::string("Invalid camera number: ") + key);
    return value.Get_Number();
}
uint32_t Milliseconds(const DATA_JSON_VALUE& row, const char* key)
{
    const double value = Number(row,key,0,600000);
    if (value != std::floor(value)) throw std::runtime_error("Non-integral camera clock");
    return static_cast<uint32_t>(value);
}
const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_Array()) throw std::runtime_error("Invalid camera array");
    return value.Get_Array();
}
float3_t Vector(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& a = Array(row,key);
    if (a.size()!=3) throw std::runtime_error("Invalid camera vector");
    for (const auto& v:a) if (!v.Is_Number() || !std::isfinite(v.Get_Number()) || std::abs(v.Get_Number())>100000)
        throw std::runtime_error("Invalid camera coordinate");
    return {static_cast<float>(a[0].Get_Number()),static_cast<float>(a[1].Get_Number()),static_cast<float>(a[2].Get_Number())};
}
}

bool CMaharakaWaterpangPresentation::Load_Camera()
{
    try
    {
        const auto path = CMapAssetCatalog::Get_MapDataRoot()/L"LV_OCN_EVENTIS_MHP.camerashots.json";
        if (std::filesystem::file_size(path)>2097152u) throw std::runtime_error("Camera document exceeds limit");
        std::ifstream input(path, std::ios::binary); std::ostringstream bytes; bytes<<input.rdbuf();
        DATA_JSON_VALUE root; std::string error;
        if (!input || !CDataJson::Parse(bytes.str(),root,error)) throw std::runtime_error("Camera parse failed: "+error);
        if (Text(root,"schema")!="lostark.camera-shots" || Number(root,"formatVersion",1,1)!=1 || Text(root,"areaId")!=AREA)
            throw std::runtime_error("Wrong Waterpang camera document");
        const DATA_JSON_VALUE* scene = nullptr;
        std::set<std::string> seen;
        for (const auto& row:Array(root,"cutscenes"))
        {
            const auto id = Text(row,"cutsceneId");
            if (!seen.insert(id).second) throw std::runtime_error("Duplicate cutscene");
            if (id==INTRO) scene=&row;
        }
        if (!scene) throw std::runtime_error("Waterpang intro15 is missing");
        const uint32_t duration = Milliseconds(*scene,"durationMs");
        if (!duration) throw std::runtime_error("Empty Waterpang intro");
        std::vector<std::string> instances; seen.clear();
        for (const auto& id:Array(*scene,"worldInstanceIds"))
        {
            if (!id.Is_String() || !seen.insert(id.Get_String()).second || !m_World.Get_Document().Find_Instance(id.Get_String()))
                throw std::runtime_error("Missing/duplicate Waterpang world instance");
            instances.push_back(id.Get_String());
        }
        if (instances.empty()) throw std::runtime_error("Empty Waterpang cast");
        std::map<std::string,const DATA_JSON_VALUE*> shots;
        for (const auto& row:Array(root,"shots"))
            if (!shots.emplace(Text(row,"shotId"), &row).second) throw std::runtime_error("Duplicate camera shot");
        std::vector<CUT> cuts;
        for (const auto& row:Array(*scene,"cameraCuts"))
        {
            CUT cut; cut.startMs=Milliseconds(row,"startMs");
            if (cut.startMs>=duration || (!cuts.empty() && cut.startMs<=cuts.back().startMs)) throw std::runtime_error("Camera cut order invalid");
            auto shot=shots.find(Text(row,"shotId"));
            if (shot==shots.end()) throw std::runtime_error("Camera shot reference missing");
            const auto& track=Field(*shot->second,"cameraTrack");
            auto& cue=cut.cue; cue.strCueId=shot->first; cue.iDurationMs=Milliseconds(track,"durationMs");
            if (!cue.iDurationMs) throw std::runtime_error("Empty camera track");
            const auto interpolation=Text(track,"interpolation"), easing=Text(track,"easing");
            if (interpolation=="LINEAR") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
            else if (interpolation=="CATMULL_ROM") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
            else throw std::runtime_error("Unknown camera interpolation");
            if (easing=="LINEAR") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
            else if (easing=="SMOOTHSTEP") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
            else if (easing=="HOLD") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
            else throw std::runtime_error("Unknown camera easing");
            for (const auto& k:Array(track,"keyframes"))
            {
                VALTAN_CINEMATIC_CAMERA_KEYFRAME key; key.strSceneId=Text(k,"sceneId"); key.iTimeMs=Milliseconds(k,"timeMs");
                if (key.iTimeMs>cue.iDurationMs || (!cue.Keyframes.empty() && key.iTimeMs<=cue.Keyframes.back().iTimeMs))
                    throw std::runtime_error("Invalid camera key clock");
                key.vEye=Vector(k,"eye"); key.vLookAt=Vector(k,"lookAt"); key.fFovYDegrees=static_cast<float>(Number(k,"fovYDegrees",1,179));
                if (k.Find("up")) { key.vUp=Vector(k,"up"); key.hasUp=true; }
                cue.Keyframes.push_back(key);
            }
            if (cue.Keyframes.empty() || cue.Keyframes.size()>512) throw std::runtime_error("Camera key count invalid");
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (!CValtanCinematicCameraController::Sample_Cue(cue,0,pose)) throw std::runtime_error("Camera sampler rejected cue");
            cuts.push_back(std::move(cut));
        }
        if (cuts.empty() || cuts.front().startMs!=0) throw std::runtime_error("Missing initial camera cut");
        m_Cuts=std::move(cuts); m_Instances=std::move(instances); m_DurationMs=duration;
        return true;
    }
    catch (const std::exception& error) { m_Status=error.what(); return false; }
}

bool CMaharakaWaterpangPresentation::Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera)
{
    m_Targets=targets; m_Camera=camera; m_Targets.objectPreparationOwner=&m_World;
    if (!m_World.Load_PreparedArea(AREA,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    if (!Load_Camera()) return false;
    for (const auto& id:m_Instances)
        if (!m_World.Prepare_InstanceResources(id,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    m_Ready=true; return true;
}

void CMaharakaWaterpangPresentation::Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play)
{
    using namespace LostArk::Shared;
    if (!m_Ready || play.strSequenceInstanceId!=MAHARAKA_WATERPANG_INTRO_INSTANCE || play.eOperation!=WORLD_SEQUENCE_OPERATION::PLAY || !play.iStartTick) return;
    if (m_Scheduled) return; // Same room reservation must not restart on duplicate delivery.
    m_StartTick=play.iStartTick; m_LastTick=play.iServerTick;
    m_ClockMs=static_cast<float>(static_cast<int32_t>(play.iServerTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    m_Scheduled=true;
}

bool CMaharakaWaterpangPresentation::Start_World(float elapsedMs)
{
    // Admission can arrive before async NPC presentation is ready. Do not start
    // half a cast; wait for every exact replacement's owner before taking poses.
    for (const auto& id:m_Instances)
        for (const auto& binding:m_World.Get_Document().Find_Instance(id)->bindings)
            if (!binding.previewNpcPlacementId.empty() && (!m_Targets.previewNpc || !m_Targets.previewNpc(binding.previewNpcPlacementId))) return false;
    for (const auto& id:m_Instances)
    {
        if (!m_World.Play(id,m_Targets) || !m_World.Seek_InstanceToMs(id,elapsedMs,m_Targets,true))
        {
            m_Status=m_World.Get_Status(); m_World.Stop_All(m_Targets,true); m_Failed=true;
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str()); return false;
        }
    }
    m_WorldClockMs=elapsedMs; m_Started=true; return true;
}

void CMaharakaWaterpangPresentation::Update(float delta, uint32_t serverTick, bool editing)
{
    using namespace LostArk::Shared;
    if (!m_Ready || !m_Scheduled || m_Failed) return;
    if (static_cast<int32_t>(serverTick-m_LastTick)>0)
    {
        m_LastTick=serverTick;
        m_ClockMs=static_cast<float>(static_cast<int32_t>(serverTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    }
    // A frozen/disconnected Server cannot locally advance the match to its start.
    m_Countdown=m_ClockMs<0 ? static_cast<int>(std::ceil(-m_ClockMs/1000.f)) : 0;
    const auto camera=m_Camera.lock();
    if (editing) { Suspend_ForAuthoring(); return; }
    if (m_ClockMs<0) return;
    if (!m_Started && !Start_World(m_ClockMs))
    {
        m_ReadinessWait += delta;
        if (!m_Failed && m_ReadinessWait >= 5.f)
        {
            m_Failed=true; m_Status="Waterpang NPC presentation readiness timed out";
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
        }
        return;
    }
    m_World.Update((std::max)(0.f,m_ClockMs-m_WorldClockMs)*.001f,m_Targets);
    m_WorldClockMs=m_ClockMs;
    if (!camera) return;
    if (m_ClockMs>=m_DurationMs) { camera->End_PresentationOverride(CAMERA_OWNER); return; }
    const CUT* cut=&m_Cuts.front();
    for (const auto& row:m_Cuts) if (row.startMs<=m_ClockMs) cut=&row;
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    if (CValtanCinematicCameraController::Sample_Cue(cut->cue,(m_ClockMs-cut->startMs)*.001f,pose) &&
        camera->Begin_PresentationOverride(CAMERA_OWNER,Engine::CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
    {
        if (pose.hasUp) camera->Apply_PresentationPoseWithUp(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.vUp,pose.fFovYDegrees);
        else camera->Apply_PresentationPose(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.fFovYDegrees);
    }
}

void CMaharakaWaterpangPresentation::Render() const
{
    if (!m_Countdown || m_Failed) return;
    const auto viewport=Engine::CGameInstance::Get().Get_ViewportSize();
    const float scale=(std::min)(viewport.x/1280.f,viewport.y/720.f);
    const std::wstring text=L"\uC6CC\uD130\uD321 \uC544\uB808\uB098\uAC00 "+std::to_wstring(m_Countdown)+L"\uCD08 \uB4A4\uC5D0 \uC2DC\uC791\uD569\uB2C8\uB2E4";
    UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"),text.c_str(),viewport.x*.5f,viewport.y*.25f,26.f*scale,DirectX::Colors::Yellow);
}

void CMaharakaWaterpangPresentation::Stop()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    m_World.Stop_All(m_Targets,true); m_Scheduled=false; m_Started=false; m_Countdown=0;
}
void CMaharakaWaterpangPresentation::Suspend_ForAuthoring()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    if (m_Started) m_World.Stop_All(m_Targets,true);
    m_Started=false; m_Countdown=0; m_ReadinessWait=0;
    // Keep the Server reservation. Closing the tool seeks to the current room
    // time (including HOLD after the intro), rather than replaying the event.
}
CMaharakaWaterpangPresentation::~CMaharakaWaterpangPresentation() { Stop(); }
```

## Client/Public/Level_Development.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class IPlayerCommandSink;
class CMaharakaWaterpangPresentation;
class CInteractKeyPromptView;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	// The current Development/Training or Maharaka owner exposes its existing camera.
	static CLevel_Development* Get_Active(LEVEL level)
	{
		return s_pActiveInstance && s_pActiveInstance->m_eLevel == level ?
			s_pActiveInstance : nullptr;
	}
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera.lock(); }
#ifdef _DEBUG
	// Borrow the existing Maharaka map; the Level remains its owner.
	CMapPlacementRuntime& Get_MapAuthoringRuntime() { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_WaterpangDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	void Set_MapAuthoringActive(bool_t active);
	std::shared_ptr<CNpc> Find_MapAuthoringNpc(const std::string& placementId) const
	{ return m_Replication.Find_NpcPlacement(placementId); }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records)
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
#endif

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_WaterpangDeploy;
#ifdef _DEBUG
	bool_t m_bMapAuthoringActive = false;
#endif
	// Maharaka only: the published source lights of the island, submitted every frame.
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	bool_t m_isMapEditorWorkspace = false;
	weak_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	std::unique_ptr<CMaharakaWaterpangPresentation> m_Waterpang;
	std::unique_ptr<CInteractKeyPromptView> m_InteractPrompt;
	static CLevel_Development* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel = LEVEL::DEVELOPMENT);
};

NS_END
```

## Client/Private/Level_Development.cpp

```cpp
#include "Level_Development.h"

#include "Camera_Free.h"
#include "ArenaCameraProfile.h"
#include "Character.h"
#include "CharacterSelectionState.h"
#include "CombatHUDViewModel.h"
#include "EffectFailureDiagnostic.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "MapLightPresentationRuntime.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "Transform.h"
#include "MaharakaWaterpangPresentation.h"
#include "UILayoutRuntime.h"

#ifdef _DEBUG
void Client::CLevel_Development::Set_MapAuthoringActive(bool_t active)
{
    if (active && m_Waterpang) m_Waterpang->Suspend_ForAuthoring();
    m_bMapAuthoringActive = active;
}
#endif
#include "InteractKeyPromptView.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

CLevel_Development* CLevel_Development::s_pActiveInstance = nullptr;

CLevel_Development::CLevel_Development(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
	: CLevel{ pDevice, pContext }
	, m_eLevel{ eLevel }
{
	s_pActiveInstance = this;
}

CLevel_Development::~CLevel_Development()
{
	m_Waterpang.reset(); // Return poses while map and replication still exist.
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	if (m_isMapEditorWorkspace)
	{
		CCombatHUDViewModel::Get().Reset_RuntimeState();
		CMapEditorWorkspaceService::Cancel();
	}
#endif
}

HRESULT CLevel_Development::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

#ifdef _DEBUG
	m_isMapEditorWorkspace =
		LEVEL::DEVELOPMENT == m_eLevel &&
		CMapEditorWorkspaceService::Is_Requested();
	if (m_isMapEditorWorkspace)
	{
		auto previewClass =
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;
		CCharacterSelectionState::Try_Get_SelectedClass(previewClass);

		if (FAILED(Ready_Lights()) ||
			FAILED(Ready_Camera(TEXT("Layer_Camera"))) ||
			!CCombatHUDViewModel::Get().Initialize_Definitions() ||
			!CCombatHUDViewModel::Get().Apply_CharacterPreview(previewClass))
		{
			CCombatHUDViewModel::Get().Reset_RuntimeState();
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		CMapEditorWorkspaceService::Set_Active(true);
		return S_OK;
	}
#endif

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(m_eLevel);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(m_eLevel),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_Development] " + m_MapRuntime.Get_Status() + "\n").c_str());
		Write_EffectFailureDiagnostic("map.area.load-failed",
			m_MapRuntime.Get_Status());
		return E_FAIL;
	}

	// Report what the Maharaka documents delivered: WATER assets and their water rows,
	// and assets that carry source material rows. Written once per level entry.
	if (LEVEL::MAHARAKA == m_eLevel)
	{
		const CMapAssetCatalog& catalog = m_MapRuntime.Get_Catalog();
		size_t waterAssets = 0u;
		size_t waterRows = 0u;
		size_t materialAssets = 0u;
		for (const MAP_ASSET_ENTRY& entry : catalog.Get_Entries())
		{
			if (MAP_ASSET_RENDER_MODE::WATER == entry.renderProfile.renderMode)
			{
				++waterAssets;
				if (nullptr != catalog.Find_Water(entry.id))
					++waterRows;
			}
			if (!entry.materialOverrides.empty())
				++materialAssets;
		}
		Write_EffectFailureDiagnostic("map.water.loaded",
			"area=" + catalog.Get_AreaId() +
			" assets=" + std::to_string(catalog.Get_Entries().size()) +
			" waterAssets=" + std::to_string(waterAssets) +
			" waterRows=" + std::to_string(waterRows) +
			" materialAssets=" + std::to_string(materialAssets));
	}

	// The island's EFActorMotion rows (turning and rocking props). An absent
	// document is not an error; a rejected one is reported and the island stays static.
	if (LEVEL::MAHARAKA == m_eLevel &&
		!m_MapRuntime.Load_SelfMotions(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Development] Self-motion document was rejected.\n");
	}

	if (FAILED(Ready_Lights()) ||
		FAILED(Ready_Camera(TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex = ETOUI(m_eLevel);
	replicationDesc.iLayerLevelIndex = ETOUI(m_eLevel);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	if (!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	if (m_eLevel == LEVEL::MAHARAKA)
	{
		m_InteractPrompt = std::make_unique<CInteractKeyPromptView>();
		m_InteractPrompt->Initialize(m_pDevice,m_pContext,ETOUI(m_eLevel),pEntry->pMapAreaId);
		m_Waterpang = std::make_unique<CMaharakaWaterpangPresentation>();
		CWorldSequencePlayer::TARGET_SET targets;
		targets.levelIndex=ETOUI(m_eLevel); targets.pCatalog=&m_MapRuntime.Get_Catalog();
		targets.pPlacements=&m_MapRuntime.Get_MutablePlacements(); targets.pDeployRuntime=&m_WaterpangDeploy;
		targets.device=m_pDevice; targets.context=m_pContext;
		targets.previewNpc=[this](const std::string& id) { return m_Replication.Find_NpcPlacement(id); };
		if (!m_Waterpang->Initialize(targets,m_pCamera.lock()))
		{
			Write_EffectFailureDiagnostic("maharaka.waterpang.prepare",m_Waterpang->Get_Status());
			m_Waterpang.reset();
		}
	}
	return S_OK;
}

void CLevel_Development::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_isMapEditorWorkspace)
		return;

	if (LEVEL::MAHARAKA == m_eLevel)
	{
#ifdef _DEBUG
		// MapTool owns sampled poses while editing; keep replication and lights live.
		if (!m_bMapAuthoringActive)
#endif
		m_MapRuntime.Update_SelfMotions(fTimeDelta);
		if (m_pMapLightPresentation &&
			!m_pMapLightPresentation->Submit_Frame() &&
			!m_bMapLightSubmissionFailureReported)
		{
			m_bMapLightSubmissionFailureReported = true;
			OutputDebugStringA(("[Level_Development][MapLight] " +
				m_pMapLightPresentation->Get_Status() + "\n").c_str());
		}
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Development] Failed to apply training replication.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-development.network-connection-lost",
			"Training replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_Development] Lobby recovery request was rejected; retrying.\n");
	}

	Bind_CameraToLocalCharacter();
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	bool editing = false;
#ifdef _DEBUG
	editing = m_bMapAuthoringActive;
#endif
	if (m_Waterpang)
	{
		for (const auto& play:m_Replication.Consume_WorldSequencePlays()) m_Waterpang->Accept(play);
		m_Waterpang->Update(fTimeDelta,m_Replication.Get_LastServerTick(),editing);
	}
	if (m_InteractPrompt)
		m_InteractPrompt->Update(fTimeDelta,localCharacter,CCombatHUDViewModel::Get().Get_InteractPromptTriggerId(),
			!editing && camera && !camera->Is_PresentationOverrideActive());
	m_PlayerController.Update(
		nullptr != camera && camera->Is_FollowEnabled() && !camera->Is_PresentationOverrideActive());
}

HRESULT CLevel_Development::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	if (m_InteractPrompt) m_InteractPrompt->Render_Text();
	if (m_Waterpang) m_Waterpang->Render();

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(m_isMapEditorWorkspace ?
		TEXT("LostArk Map Editor Workspace") :
		LEVEL::MAHARAKA == m_eLevel ?
		TEXT("LostArk Maharaka Paradise") :
		TEXT("LostArk Test Training Ground"));
#endif
	return S_OK;
}

HRESULT CLevel_Development::Ready_Lights()
{
	if (LEVEL::MAHARAKA != m_eLevel)
		return S_OK;

	// The island's Area declares a light pair, so a missing or rejected document
	// fails the load like Character Select does instead of showing an unlit island.
	const CLIENT_LEVEL_DESCRIPTOR* pEntry = CLevelRegistry::Find(m_eLevel);
	auto staged = make_shared<CMapLightPresentationRuntime>();
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!staged->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_Development][MapLight] " +
			staged->Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	m_pMapLightPresentation = std::move(staged);
	m_bMapLightSubmissionFailureReported = false;
	return S_OK;
}

HRESULT CLevel_Development::Ready_Camera(
	const wstring_t& strLayerTag)
{
	// Training/Maharaka share the saved Character Select sizes, independent
	// of whichever arena last wrote the process presentation profile.
	ARENA_CAMERA_PROFILE sizeProfile = CArenaCameraProfile::Default(ARENA_CAMERA_MAP::CHARACTER_SELECT);
	std::string sizeStatus;
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::CHARACTER_SELECT, sizeProfile, sizeStatus))
		OutputDebugStringA(("[Level_Development][CharacterSize] " + sizeStatus + "\n").c_str());
	CCharacter::Set_MapPresentationSizeProfile(sizeProfile);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	cameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 4000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(m_eLevel),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(m_eLevel),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(m_eLevel),
			strLayerTag,
			gameObject);
		return E_FAIL;
	}

	m_pCamera = camera;
	return S_OK;
}

bool_t CLevel_Development::Bind_CameraToLocalCharacter()
{
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr == camera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		camera->Set_FollowTarget(nullptr);
		camera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;
	camera->Set_PositionOffset(float3_t(0.4f, 7.5f, 4.5f));
	camera->Set_FollowTarget(transform);
	camera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_Development> CLevel_Development::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
{
	auto instance = unique_ptr<CLevel_Development>(
		new CLevel_Development(pDevice, pContext, eLevel));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
```

## Client/Private/Loader.cpp

```cpp
#include "Loader.h"
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "Profiler.h"

#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "Camera_Free.h"
#include "Body_Valtan.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "ClassSelectionPresentation.h"
#include "Collider.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "Effect_Catalog.h"
#include "Effect_LoadPreparationJob.h"
#include "Effect_PresentationService.h"
#include "EstherActionSoundCueDocument.h"
#include "EstherCutinPresentationService.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapAssetPreview.h"
#include "MapNavigationContract.h"
#include "MapPlacementRuntime.h"
#include "MapStaticBatchObject.h"
#include "WorldSequencePlayer.h"
#include "WorldSequenceObject.h"
#include "Navigation.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "Part_Vehicle.h"
#include "VehiclePresentationAssetService.h"
#include "PlayableCharacterAssetService.h"
#include "RuntimeAssetRoot.h"
#include "Trigger_Box.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <unordered_set>
#include <unordered_map>

namespace
{
	std::mutex g_ActiveStatusMutex;
	std::string g_ActiveStatus = "Loader has not started.";

	std::string ToUtf8(const tchar_t* pText)
	{
		if (nullptr == pText || L'\0' == *pText)
			return {};
		const int characterCount = static_cast<int>(wcslen(pText));
		const int byteCount = WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, nullptr, 0, nullptr, nullptr);
		if (byteCount <= 0)
			return {};
		std::string result(static_cast<size_t>(byteCount), '\0');
		WideCharToMultiByte(
			CP_UTF8, 0, pText, characterCount, result.data(), byteCount,
			nullptr, nullptr);
		return result;
	}

	class CLevelResourceRollbackScope final
	{
	public:
		explicit CLevelResourceRollbackScope(const uint32_t levelIndex)
			: m_iLevelIndex { levelIndex }
		{
		}

		~CLevelResourceRollbackScope()
		{
			if (m_isCommitted)
				return;

			if (FAILED(CGameInstance::Get().Clear_Resources(m_iLevelIndex)))
			{
				OutputDebugStringA(
					"[Loader] Failed to roll back level resources.\n");
			}
		}

		CLevelResourceRollbackScope(
			const CLevelResourceRollbackScope&) = delete;
		CLevelResourceRollbackScope& operator=(
			const CLevelResourceRollbackScope&) = delete;

		void Commit()
		{
			m_isCommitted = true;
		}

	private:
		uint32_t m_iLevelIndex = {};
		bool_t m_isCommitted = false;
	};

	std::string ResolveAssetPath(const std::filesystem::path& relativePath)
	{
		return CRuntimeAssetRoot::Resolve(relativePath).string();
	}

	const tchar_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return TEXT("Lance Master");
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return TEXT("Gunslinger");
		case CHARACTER_CLASS_ID::SLAYER:
			return TEXT("Slayer");
		case CHARACTER_CLASS_ID::ARTIST:
			return TEXT("Artist");
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return TEXT("DimensionMaster");
		case CHARACTER_CLASS_ID::WARLORD:
			return TEXT("Warlord");
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return TEXT("GuardianKnight");
		default:
			return TEXT("Unknown");
		}
	}
}

CLoader::CLoader(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CLoader::~CLoader()
{
	Free();
}

uint32_t APIENTRY ThreadMain(void* pArgument)
{
	const HRESULT comResult =
		CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	CLoader* pLoader = static_cast<CLoader*>(pArgument);
	const HRESULT loadResult =
		nullptr == pLoader ? E_POINTER : pLoader->Start_Loading();

	if (SUCCEEDED(comResult))
		CoUninitialize();

	return FAILED(loadResult) ? 1u : 0u;
}

HRESULT CLoader::Initialize(const LEVEL eNextLevelID)

{
	return Initialize(eNextLevelID, 0u, 0u);
}

HRESULT CLoader::Initialize(
	const LEVEL eNextLevelID,
	const uint64_t iEffectLoadJobEpoch,
	const uint64_t iEffectCatalogRevision)
{
	const auto reject = [](const HRESULT result, const std::string_view source,
		const std::string_view detail)
	{
		// Record before Create destroys this Loader and MainApp reports the generic null.
		// The transition service preserves the first recovery for this load attempt.
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED,
			source, detail, result);
		return result;
	};
	if (nullptr == CLevelRegistry::Find(eNextLevelID))
		return reject(E_INVALIDARG, "loader.initialize.target", "Target Level is not registered.");
	if ((0u == iEffectLoadJobEpoch) != (0u == iEffectCatalogRevision))
		return reject(E_INVALIDARG, "loader.initialize.effect-input",
			"Effect preparation epoch and catalog revision must both be present or absent.");
	if (0u != iEffectLoadJobEpoch)
	{
		m_pEffectLoadJob = std::make_shared<CEffectLoadPreparationJob>();
		std::string JobStatus;
		if (nullptr == m_pEffectLoadJob || !m_pEffectLoadJob->Open(
			iEffectLoadJobEpoch, iEffectCatalogRevision, JobStatus))
		{
			OutputDebugStringA(("[Loader][Effect] " + JobStatus + "\n").c_str());
			m_pEffectLoadJob.reset();
			return reject(E_FAIL, "loader.initialize.effect-job", JobStatus);
		}
	}

	// Stage mutable catalog membership/skills on the owner thread. The loader
	// worker reads this immutable selection even if an explicit F1 reload occurs.
	if (eNextLevelID != LEVEL::LOBBY)
	{
		// ActorCatalog's lazy initialization mutates shared vectors. Finish it
		// on this owner before level and Effect preparation can read them.
		if (!CActorCatalog::Initialize())
		{
			OutputDebugStringA(("[Loader] " + CActorCatalog::Get_Status() + "\n").c_str());
			return reject(E_FAIL, "loader.initialize.actor-catalog", CActorCatalog::Get_Status());
		}
		m_ePreparedCharacterClass = CNetworkManager::Get().Get_LocalCharacterClass();
		// Lobby's accepted enter-world generation preserves this exact class.
		// The optional character-selection UI cache is not admission authority.
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(m_ePreparedCharacterClass))
		{
			Set_Status(TEXT("Character preparation rejected: Server admission has no supported class."));
			OutputDebugStringA("[Loader] Missing/invalid Server-approved character class.\n");
			return reject(E_INVALIDARG, "loader.initialize.character-class",
				"Server admission has no supported character class.");
		}
		const std::array selectedClass = { m_ePreparedCharacterClass };
		std::span<const LostArk::Shared::CHARACTER_CLASS_ID> preparationClasses = selectedClass;
#ifndef _DEBUG
		if (eNextLevelID == LEVEL::CHARACTER_SELECT)
			preparationClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
#endif
		for (const auto characterClass : preparationClasses)
		{
			auto input = CPlayableCharacterAssetService::Capture_AuthoringInput(characterClass);
			if (!input)
				return reject(E_FAIL, "loader.initialize.character-authoring",
					"Character authoring snapshot could not be captured; player skills are unavailable.");
			m_CharacterAuthoringInputs.emplace(characterClass, std::move(input));
		}
	}
	m_eNextLevelID = eNextLevelID;
	m_iResult.store(S_FALSE, std::memory_order_release);
	m_eState.store(STATE::RUNNING, std::memory_order_release);

	// Effect requests are captured and posted by the Loading Level owner.
	// This producer can wait for that mailbox while map/character work starts;
	// it never mutates the target level's prototype registry.
	if (m_pEffectLoadJob &&
		(m_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) == 0u)
	{
		m_hEffectThread = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr, 0u, &CLoader::EffectThreadMain, this, 0u, nullptr));
		if (!m_hEffectThread)
			OutputDebugStringA("[Loader] Effect producer creation failed; using serial preparation.\n");
	}
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
		nullptr,
		0,
		ThreadMain,
		this,
		0,
		nullptr));
	if (nullptr == m_hThread)
	{
		Request_Cancellation();
		m_iResult.store(E_FAIL, std::memory_order_release);
		m_eState.store(STATE::FAILED, std::memory_order_release);
		return reject(E_FAIL, "loader.initialize.worker", "Could not create the Level loading worker.");
	}
	return S_OK;
}

HRESULT CLoader::Start_Loading()
{
	/* Worker-thread scopes: the profiler attributes them to the frame in
	   which they end, and long ones surface in its Long operations list. */
	Engine::CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	HRESULT result = S_OK;
	try
	{
		Engine::CProfilerScope levelScope(pProfiler, "Loader.LevelLoad");
		result = CLevelRegistry::Execute_Load(m_eNextLevelID, *this);
	}
	catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
	catch (...) { result = E_FAIL; }
	if (FAILED(result))
		Request_Cancellation();
	if (nullptr != m_hEffectThread)
	{
		// Main keeps consuming the Effect queue while this worker joins. Free
		// owns the bounded shutdown deadline and both producer handles.
		const DWORD waited = WaitForSingleObject(m_hEffectThread, INFINITE);
		if (WAIT_OBJECT_0 != waited)
		{
			OutputDebugStringA("[Loader] Effect producer join failed; terminating the process.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_INVALID_HANDLE))
				std::terminate();
			__assume(0);
		}
		const HRESULT effectResult = static_cast<HRESULT>(
			m_iEffectResult.load(std::memory_order_acquire));
		if (SUCCEEDED(result) || result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
			result = effectResult;
	}
	else if (SUCCEEDED(result) && nullptr != m_pEffectLoadJob)
	{
		try
		{
			Engine::CProfilerScope effectScope(pProfiler, "Loader.EffectPreparation");
			result = Run_EffectLoadPreparation();
		}
		catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
		catch (...) { result = E_FAIL; }
	}
	if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
		result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (FAILED(result))
		Request_Cancellation();
	m_iResult.store(result, std::memory_order_release);
	m_eState.store(
		SUCCEEDED(result) ? STATE::SUCCEEDED : STATE::FAILED,
		std::memory_order_release);
	if (FAILED(result))
		OutputDebugStringW(L"[Loader] Level load failed.\n");
	return result;
}

HRESULT CLoader::Run_EffectLoadPreparation()
{
	return CEffectPresentationService::Run_ProductPreparationWorker(
		m_pDevice, m_pContext, m_pEffectLoadJob, &m_isCancellationRequested,
		&m_EffectPreparationBatch);
}

unsigned __stdcall CLoader::EffectThreadMain(void* pArgument)
{
	auto* loader = static_cast<CLoader*>(pArgument);
	const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	HRESULT result = apartment;
	if (SUCCEEDED(apartment))
	{
		try
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.EffectPreparation");
			result = loader->Run_EffectLoadPreparation();
		}
		catch (const std::bad_alloc&) { result = E_OUTOFMEMORY; }
		catch (...) { result = E_FAIL; }
		CoUninitialize();
	}
	loader->m_iEffectResult.store(result, std::memory_order_release);
	if (FAILED(result))
		loader->Request_Cancellation();
	return FAILED(result) ? 1u : 0u;
}

void CLoader::Request_Cancellation() noexcept
{
	m_isCancellationRequested.store(true, std::memory_order_release);
	try
	{
		if (m_pEffectLoadJob && !m_pEffectLoadJob->Is_Cancelled() &&
			!m_pEffectLoadJob->Is_Closed())
		{
			std::optional<EFFECT_LOAD_JOB_COMMAND> displaced;
			std::string status;
			(void)m_pEffectLoadJob->Post_Command(EFFECT_LOAD_JOB_COMMAND::Cancel(
				m_pEffectLoadJob->Get_CurrentEpoch(),
				m_pEffectLoadJob->Get_CurrentCatalogRevision()), displaced, status);
		}
	}
	catch (...)
	{
		OutputDebugStringA("[Loader] Could not post Effect cancellation; bounded shutdown remains active.\n");
	}
}

void CLoader::Set_Status(const tchar_t* pStatus)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged = nullptr == pStatus ?
			L'\0' != m_szLoadingText[0] :
			0 != wcscmp(m_szLoadingText, pStatus);
		if (nullptr == pStatus)
			m_szLoadingText[0] = L'\0';
		else
			wcsncpy_s(m_szLoadingText, pStatus, _TRUNCATE);
		if (nullptr != pStatus)
			++m_iPhaseIndex;
		m_bProgressDeterminate = false;
		m_iProgressCompleted = 0u;
		m_iProgressTotal = 0u;
		if (bChanged)
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
	}

	lock_guard<mutex> activeLock(g_ActiveStatusMutex);
	g_ActiveStatus = ToUtf8(pStatus);
}

void CLoader::Set_DeterminateStatus(
	const tchar_t* pStatus,
	const size_t iCompleted,
	const size_t iTotal)
{
	{
		lock_guard<mutex> lock(m_StatusMutex);
		const bool_t bChanged = nullptr == pStatus ?
			L'\0' != m_szLoadingText[0] :
			0 != wcscmp(m_szLoadingText, pStatus);
		if (nullptr == pStatus)
			m_szLoadingText[0] = L'\0';
		else
			wcsncpy_s(m_szLoadingText, pStatus, _TRUNCATE);
		m_bProgressDeterminate = 0u != iTotal;
		m_iProgressCompleted = (min)(iCompleted, iTotal);
		m_iProgressTotal = iTotal;
		if (bChanged)
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
	}

	lock_guard<mutex> activeLock(g_ActiveStatusMutex);
	g_ActiveStatus = ToUtf8(pStatus);
}

void CLoader::Declare_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseIndex = 0u;
	m_iPhaseCount = iCount;
}

void CLoader::Add_Phases(const size_t iCount)
{
	lock_guard<mutex> lock(m_StatusMutex);
	m_iPhaseCount += iCount;
}

std::string CLoader::Get_ActiveStatus()
{
	lock_guard<mutex> lock(g_ActiveStatusMutex);
	return g_ActiveStatus;
}

CLoader::PROGRESS_SNAPSHOT CLoader::Get_ProgressSnapshot() const
{
	PROGRESS_SNAPSHOT Snapshot;
	tchar_t Status[MAX_PATH]{};
	{
		lock_guard<mutex> lock(m_StatusMutex);
		wcsncpy_s(Status, m_szLoadingText, _TRUNCATE);
		Snapshot.bDeterminate = m_bProgressDeterminate;
		Snapshot.iCompleted = m_iProgressCompleted;
		Snapshot.iTotal = m_iProgressTotal;
		Snapshot.iPhaseIndex = m_iPhaseIndex;
		Snapshot.iPhaseCount = m_iPhaseCount;
		Snapshot.iElapsedMs = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() -
				m_ProgressPhaseStarted).count());
	}
	Snapshot.strStatus = ToUtf8(Status);
	return Snapshot;
}

void CLoader::Copy_Status(
	tchar_t* pOutput,
	const size_t outputCount) const
{
	if (nullptr == pOutput || 0u == outputCount)
		return;
	lock_guard<mutex> lock(m_StatusMutex);
	wcsncpy_s(pOutput, outputCount, m_szLoadingText, _TRUNCATE);
}

#ifdef _DEBUG
void CLoader::Print_Text()
{
	tchar_t status[MAX_PATH]{};
	Copy_Status(status, size(status));
	SetWindowText(g_hWnd, status);
}
#endif

HRESULT CLoader::Ready_For_Lobby()
{
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::LOBBY));
	Declare_Phases(2u);
	Set_Status(TEXT("LOBBY: stage selection UI"));
	Set_Status(TEXT("Lobby loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_CharacterSelect()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::CHARACTER_SELECT));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::CHARACTER_SELECT), "CHARACTER_SELECT_ARENA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
#ifdef _DEBUG
	const std::array characterClasses = { m_ePreparedCharacterClass };
#else
	// Move first-use class model preparation into the entry Loading phase.
	constexpr auto characterClasses = CCharacterCatalog::CHARACTER_SELECT_CLASSES;
#endif

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::CHARACTER_SELECT));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);

	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Declare_Phases(9u);
	Set_Status(TEXT("CHARACTER SELECT: visual map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("CHARACTER SELECT: playable classes"));
	if (FAILED(Ready_Character_Rendering(
			ETOUI(LEVEL::CHARACTER_SELECT),
			characterClasses)))
	{
		return E_FAIL;
	}
	/* The Server enables the Esther gauge in this test arena too, so the same
	summon roster as VALTAN is admitted here: a lazy first-spawn admission stalls
	the frame the caster presses the key. Missing payload isolates only the summon. */
	Set_Status(TEXT("CHARACTER SELECT: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::CHARACTER_SELECT));
	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::CHARACTER_SELECT));
#ifndef _DEBUG
	Set_Status(TEXT("CHARACTER SELECT: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::CHARACTER_SELECT), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] CHARACTER SELECT vehicle " +
				std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n").c_str());
		}
	}
#endif
	Set_Status(TEXT("CHARACTER SELECT: class selection cinematics"));
	if (CClassSelectionPresentation::Is_Configured())
	{
		std::vector<std::string> backgroundAreas;
		std::string backgroundStatus;
		if (!CClassSelectionPresentation::Load_BackgroundAreas(pEntry->pMapAreaId,
			pEntry->pPresentationMapAreaId ? pEntry->pPresentationMapAreaId : "",
			backgroundAreas, backgroundStatus))
			OutputDebugStringA(("[Loader][ClassSelectionCinema] " + backgroundStatus + "\n").c_str());
		else for (const auto& backgroundArea : backgroundAreas)
		{
			// The primary Area is already prepared; never register its prototypes twice.
			if (backgroundArea == pEntry->pMapAreaId) continue;
			if (FAILED(Ready_MapArea(ETOUI(LEVEL::CHARACTER_SELECT), backgroundArea,
				pEntry->PresentationMapLoadScope, false)))
			{
				if (m_isCancellationRequested.load(std::memory_order_acquire))
					return HRESULT_FROM_WIN32(ERROR_CANCELLED);
				OutputDebugStringA(("[Loader][ClassSelectionCinema] Optional background unavailable: " +
					backgroundArea + "\n").c_str());
			}
		}
	}
	std::string selectionSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::CHARACTER_SELECT),
		pEntry->pMapAreaId, pEntry->MapLoadScope, selectionSequenceStatus,
		[this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][ClassSelectionCinema] " + selectionSequenceStatus + "\n").c_str());
	}
	Set_Status(TEXT("Character Select loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Bern()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::BERN), "BERN")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::BERN));
	Declare_Phases(7u);
	Set_Status(TEXT("BERN: world catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::BERN);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::BERN),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("BERN: session character bundle"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::BERN),
		selectedClass)))
		return E_FAIL;

	CVehiclePresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::BERN));
#ifndef _DEBUG
	Set_Status(TEXT("BERN: vehicle presentation"));
	for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
	{
		if (FAILED(CVehiclePresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, ETOUI(LEVEL::BERN), vehicle.vehicleId)))
		{
			OutputDebugStringA(("[Loader][VehiclePresentation] BERN vehicle " +
				std::to_string(vehicle.vehicleId) + " is unavailable; the level loads on foot.\n").c_str());
		}
	}
#endif
	Set_Status(TEXT("Bern loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_ValtanArena()
{
	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::VALTAN_ARENA), "VALTAN_ARENA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CMonsterPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::VALTAN_ARENA));
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::VALTAN_ARENA));
	Declare_Phases(10u);
	Set_Status(TEXT("VALTAN: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: network player rendering"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::VALTAN_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: server-authoritative boss presentation"));
	if (FAILED(Ready_ValtanPresentation(
		ETOUI(LEVEL::VALTAN_ARENA))))
	{
		return E_FAIL;
	}
	Set_Status(TEXT("VALTAN: monster presentation warmup"));
	for (const MONSTER_ACTOR_ENTRY& monster : CActorCatalog::Get_Monsters())
	{
		if (monster.runtimeStatus != "supported")
			continue;
		if (FAILED(CMonsterPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::VALTAN_ARENA),
			monster.archetypeId)))
		{
			OutputDebugStringA((
				"[Loader][MonsterPresentation] unavailable archetype: " +
				monster.archetypeId + "\n").c_str());
		}
	}
	/* The raid Esther summons spawn mid-fight, so their models must already be
	prototypes when the arena opens: the lazy admission on first spawn stalls
	the frame the caster presses the key. This list mirrors the server's
	CEstherSkillSystem roster and moves into a data contract with it. */
	Set_Status(TEXT("VALTAN: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::VALTAN_ARENA));
	Set_Status(TEXT("VALTAN: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

    Set_Status(TEXT("VALTAN: source cinematic world sequences"));
    // Source actors are used by Server presentation and Workbench before Map
    // Tool is ever opened. Register their clone factory in the Level load,
    // under the same rollback scope as the other Valtan prototypes.
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::VALTAN_ARENA), CWorldSequenceObject::PROTOTYPE_TAG,
        CWorldSequenceObject::Create(m_pDevice, m_pContext))))
    {
        Set_Status(TEXT("VALTAN: source cinematic object prototype failed"));
        return E_FAIL;
    }
    std::string worldSequenceStatus;
    if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::VALTAN_ARENA),
        pEntry->pMapAreaId, pEntry->MapLoadScope, worldSequenceStatus,
        [this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
    {
        if (m_isCancellationRequested.load(std::memory_order_acquire))
            return HRESULT_FROM_WIN32(ERROR_CANCELLED);
        OutputDebugStringA(("[Loader][ValtanSourceCinema] " + worldSequenceStatus + "\n").c_str());
    }
	Set_Status(TEXT("Valtan arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_KakulSaydonArena()
{
	CKoukuSaydonPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	CNpcPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::KAKULSAYDON_ARENA), "KAKULSAYDON_ARENA")))
	{
		/* The world remains enterable when it has no authored NPC placement
		   presentation yet. Reliable Server entities retain their catalog idle
		   fallback, while the missing optional binding is reported explicitly. */
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CMonsterPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));

	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::KAKULSAYDON_ARENA));
	Declare_Phases(9u);
	Set_Status(TEXT("KoukuSaydon: arena map"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::KAKULSAYDON_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: server-approved character rendering"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		selectedClass)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: Server boss presentation"));
	if (FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		"BOSS_KAKULSAYDON_G1_KOUKU")))
	{
		OutputDebugStringA(("[Loader][KoukuSaydonPresentation] " +
			CKoukuSaydonPresentationAssetService::Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	/* Same Esther roster as Valtan (the Server enables the gauge in this raid too); the
	   summons spawn mid-fight, so their models are prototypes before the arena opens. */
	Set_Status(TEXT("KoukuSaydon: esther summon presentation"));
	Ready_EstherSummonPresentation(ETOUI(LEVEL::KAKULSAYDON_ARENA));
	Set_Status(TEXT("KoukuSaydon: deploy environment prototypes"));
	if (FAILED(Ready_DeployPropArea(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("KoukuSaydon: world sequence document"));
	std::string worldSequenceStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId, pEntry->MapLoadScope, worldSequenceStatus,
		[this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		// Preserve the Level's optional-presentation failure boundary. Activation
		// consumes this exact failure and never reparses on the game frame.
		OutputDebugStringA(("[Loader][WorldSequence] " + worldSequenceStatus + "\n").c_str());
	}

	Set_Status(TEXT("KoukuSaydon arena loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Development()
{
	CLevelResourceRollbackScope rollback(
		ETOUI(LEVEL::DEVELOPMENT));

#ifdef _DEBUG
	if (CMapEditorWorkspaceService::Is_Requested())
	{
		CNpcPresentationAssetService::Begin_LevelLoad(
			ETOUI(LEVEL::DEVELOPMENT));
		Declare_Phases(3u);
		Set_Status(TEXT("MAP EDITOR: core rendering resources"));
		if (FAILED(Ready_MapAuthoringCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		if (FAILED(Ready_AnimatedMeshShader(ETOUI(LEVEL::DEVELOPMENT))) ||
			FAILED(Ready_DeployPropCore(ETOUI(LEVEL::DEVELOPMENT))))
		{
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}

		Set_Status(TEXT("Map Editor workspace loading complete"));
		rollback.Commit();
		return S_OK;
	}
#endif

	CValtanPresentationAssetService::Begin_LevelLoad(
		ETOUI(LEVEL::DEVELOPMENT));
	CNpcPlacementPresentationService::Begin_LevelLoad(
		ETOUI(LEVEL::DEVELOPMENT));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::DEVELOPMENT), "TRAINING_GROUND")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::DEVELOPMENT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId)
		return E_INVALIDARG;

	Declare_Phases(6u);
	Set_Status(TEXT("TEST: training map"));
	if (FAILED(Ready_MapArea(
		ETOUI(LEVEL::DEVELOPMENT),
		pEntry->pMapAreaId,
		pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	Set_Status(TEXT("TEST: server-approved character rendering"));
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::DEVELOPMENT),
		selectedClass)))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_AnimationPreviewModels(
		ETOUI(LEVEL::DEVELOPMENT))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Development scenario loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_For_Maharaka()
{
	CNpcPresentationAssetService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	CNpcPlacementPresentationService::Begin_LevelLoad(ETOUI(LEVEL::MAHARAKA));
	if (FAILED(CNpcPlacementPresentationService::Load(
		ETOUI(LEVEL::MAHARAKA), "MAHARAKA")))
	{
		OutputDebugStringA(("[Loader][NpcPresentation] " +
			CNpcPlacementPresentationService::Get_Status() + "\n").c_str());
	}
	CLevelResourceRollbackScope rollback(ETOUI(LEVEL::MAHARAKA));
	Declare_Phases(6u);
	Set_Status(TEXT("MAHARAKA: island catalog and placements"));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::MAHARAKA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		FAILED(Ready_MapArea(
			ETOUI(LEVEL::MAHARAKA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope)))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("MAHARAKA: session character bundle"));
	const std::array selectedClass =
	{
		m_ePreparedCharacterClass
	};
	if (FAILED(Ready_Character_Rendering(
		ETOUI(LEVEL::MAHARAKA),
		selectedClass)))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::MAHARAKA),
		CWorldSequenceObject::PROTOTYPE_TAG,CWorldSequenceObject::Create(m_pDevice,m_pContext)))) return E_FAIL;
	std::string waterpangStatus;
	if (!CWorldSequencePlayer::Prepare_AreaLoad(ETOUI(LEVEL::MAHARAKA),pEntry->pMapAreaId,
		pEntry->MapLoadScope,waterpangStatus,[this]() { return m_isCancellationRequested.load(std::memory_order_acquire); }))
	{
		if (m_isCancellationRequested.load(std::memory_order_acquire)) return HRESULT_FROM_WIN32(ERROR_CANCELLED);
		OutputDebugStringA(("[Loader][Waterpang] "+waterpangStatus+"\n").c_str());
	}
	Set_Status(TEXT("Maharaka loading complete"));
	rollback.Commit();
	return S_OK;
}

HRESULT CLoader::Ready_MapArea(
	const uint32_t iLevelIndex,
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope, const bool_t prepareMapCore)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	// A failed optional preparation must not leave a previous visit's stage available.
	CMapPlacementRuntime::Discard_LoadStage(areaId);
	try
	{
		if (prepareMapCore && FAILED(Ready_MapAuthoringCore(iLevelIndex)))
			return E_FAIL;

		CMapAssetCatalog mapCatalog;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Catalog");
			Set_Status(TEXT("Map: explicit area catalog"));
			if (!mapCatalog.Load_Area(areaId))
			{
				{
					lock_guard<mutex> activeLock(g_ActiveStatusMutex);
					g_ActiveStatus = "Map " + areaId + ": " + mapCatalog.Get_Status();
				}
				OutputDebugStringA(("[Loader][Map] " + mapCatalog.Get_Status() + "\n").c_str());
				return E_FAIL;
			}
		}

		std::unordered_set<std::string> requiredAssetIds;
		std::vector<MAP_PLACEMENT_RECORD> scopedPlacements;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Scope");
			if (loadScope.isEnabled)
			{
				std::string placementStatus;
				Add_Phases(1u);
				Set_Status(TEXT("Map: product load scope"));
				if (!CMapPlacementRuntime::Read_Placements(
					mapCatalog, scopedPlacements, placementStatus))
				{
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": product load scope: " + placementStatus;
					}
					OutputDebugStringA(("[Loader][Map] " + placementStatus + "\n").c_str());
					return E_FAIL;
				}
				CMapPlacementRuntime::Apply_LoadScope(mapCatalog, loadScope, scopedPlacements);
				for (const MAP_PLACEMENT_RECORD& record : scopedPlacements)
					requiredAssetIds.insert(record.assetId);
				if (requiredAssetIds.empty())
				{
					lock_guard<mutex> activeLock(g_ActiveStatusMutex);
					g_ActiveStatus = "Map " + areaId + ": product load scope selected no placements";
					return E_FAIL;
				}
			}
		}

		const size_t requiredModelCount = loadScope.isEnabled ?
			requiredAssetIds.size() : mapCatalog.Get_Entries().size();
		std::vector<const MAP_ASSET_ENTRY*> entries;
		std::vector<std::vector<size_t>> groups;
		std::unordered_map<std::wstring, size_t> geometryGroups;
		entries.reserve(requiredModelCount);
		// Catalog order selects the same physical base and material/LOD inputs
		// as the serial loader; scheduling only changes independent groups.
		for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
		{
			if (loadScope.isEnabled && !requiredAssetIds.contains(entry.id))
				continue;
			const size_t index = entries.size();
			entries.push_back(&entry);
			const std::wstring key = entry.resolvedModelPath.lexically_normal().wstring();
			const auto [found, inserted] = geometryGroups.emplace(key, groups.size());
			if (inserted)
				groups.emplace_back();
			groups[found->second].push_back(index);
		}
		if (entries.size() != requiredModelCount)
			return E_FAIL;

		std::vector<std::pair<wstring_t, unique_ptr<CPrototype>>> staged(requiredModelCount);
		for (size_t index = 0u; index < entries.size(); ++index)
			staged[index].first = entries[index]->prototypeTag;
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Models");
			const auto started = std::chrono::steady_clock::now();
			const auto assetRoot = CRuntimeAssetRoot::Get();
			std::vector<size_t> failedModels(groups.size(), (std::numeric_limits<size_t>::max)());
			std::atomic_bool failed = false;
			std::mutex progressMutex;
			size_t completed = 0u;
			Set_DeterminateStatus(TEXT("Map: model prototypes"), 0u, requiredModelCount);
			const auto preparation = m_AssetPreparationBatch.Run(m_pDevice.Get(), groups.size(),
				&m_isCancellationRequested, [&](const size_t groupIndex) -> HRESULT
				{
					size_t activeIndex = (std::numeric_limits<size_t>::max)();
					try
					{
						Engine::CProfilerScope workerScope(CGameInstance::Get().Get_Profiler(),
							"Loader.Map.ModelWorker");
						const matrix_t transform = XMMatrixScaling(0.01f, 0.01f, 0.01f);
						const CModel* geometry = nullptr;
						for (const size_t index : groups[groupIndex])
						{
							if (failed.load(std::memory_order_acquire))
								return S_OK; // The failing group's result owns the actual error.
							if (m_isCancellationRequested.load(std::memory_order_acquire))
								return HRESULT_FROM_WIN32(ERROR_CANCELLED);
							activeIndex = index;
							const MAP_ASSET_ENTRY& entry = *entries[index];
							Engine::MODEL_ASSET_LOAD_DESC loadDesc;
							loadDesc.assetRoot = assetRoot;
							loadDesc.meshPath = entry.resolvedModelPath;
							loadDesc.materialOverrides = entry.materialOverrides;
							auto model = geometry ?
								CModel::Create_MaterialVariant(*geometry, loadDesc) :
								CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, loadDesc, transform);
							if (!model)
							{
								failedModels[groupIndex] = index;
								failed.store(true, std::memory_order_release);
								return E_FAIL;
							}
							if (!geometry)
								geometry = model.get();
							// The group's first staged model keeps geometry alive. No worker
							// changes the container or another group's catalog slots.
							staged[index].second = std::move(model);
							{
								std::scoped_lock progressLock(progressMutex);
								++completed;
								Set_DeterminateStatus(TEXT("Map: model prototypes"), completed, requiredModelCount);
							}
						}
						return S_OK;
					}
					catch (...)
					{
						failedModels[groupIndex] = activeIndex;
						failed.store(true, std::memory_order_release);
						throw; // The common worker boundary preserves E_OUTOFMEMORY/E_FAIL.
					}
				});
			HRESULT result = preparation.result;
			if (SUCCEEDED(result) && m_isCancellationRequested.load(std::memory_order_acquire))
				result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			if (SUCCEEDED(result) && completed != requiredModelCount)
				result = E_FAIL;
			const double elapsedMs = std::chrono::duration<double, std::milli>(
				std::chrono::steady_clock::now() - started).count();
			char diagnostic[512]{};
			sprintf_s(diagnostic,
				"[Loader][Map] area=%s models=%zu geometryGroups=%zu variants=%zu workers=%u prepared=%zu elapsedMs=%.3f result=0x%08lX\n",
				areaId.c_str(), requiredModelCount, groups.size(),
				requiredModelCount - groups.size(), preparation.workerCount, completed, elapsedMs,
				static_cast<unsigned long>(result));
			OutputDebugStringA(diagnostic);
			if (FAILED(result))
			{
				const size_t failedIndex = preparation.failedTask < failedModels.size() ?
					failedModels[preparation.failedTask] : (std::numeric_limits<size_t>::max)();
				if (failedIndex < entries.size())
				{
					const MAP_ASSET_ENTRY& entry = *entries[failedIndex];
					{
						lock_guard<mutex> activeLock(g_ActiveStatusMutex);
						g_ActiveStatus = "Map " + areaId + ": model prototype failed " +
							entry.id + " / " + entry.modelRelativePath.generic_string() +
							" (" + std::to_string(completed) + "/" +
							std::to_string(requiredModelCount) + ")";
					}
					OutputDebugStringW((L"[Loader][Map] Model failed: " + entry.prototypeTag +
						L" / " + entry.resolvedModelPath.wstring() + L"\n").c_str());
				}
				return result;
			}
		}

		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Navigation");
			MAP_NAVIGATION_CONTRACT navigationContract;
			std::string navigationStatus;
			Set_Status(TEXT("Map: navigation contract"));
			if (!CMapNavigationContract::Resolve_Area(areaId, navigationContract, navigationStatus))
			{
				OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
				return E_FAIL;
			}
			if (navigationContract.runtimeGridAvailable)
			{
				f32_t maximumStepHeight = 0.f;
				if (!CMapNavigationContract::Read_RuntimeStepHeight(
					navigationContract, maximumStepHeight, navigationStatus))
				{
					OutputDebugStringA(("[Loader][Map] " + navigationStatus + "\n").c_str());
					return E_FAIL;
				}
				auto navigation = CNavigation::Create_NavGrid(m_pDevice, m_pContext,
					navigationContract.runtimePath.c_str(), maximumStepHeight);
				if (!navigation)
					return E_FAIL;
				staged.emplace_back(navigationContract.prototypeTag, std::move(navigation));
			}
		}
		if (m_isCancellationRequested.load(std::memory_order_acquire))
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Loader.Map.Commit");
			if (!staged.empty() &&
				FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
				return E_FAIL;
			// Failed model/navigation preparation never publishes a placement stage.
			if (loadScope.isEnabled)
				CMapPlacementRuntime::Cache_LoadStage(areaId, loadScope, mapCatalog, scopedPlacements);
		}
		return S_OK;
	}
	catch (const std::bad_alloc&)
	{
		OutputDebugStringA("[Loader][Map] Map preparation allocation failed.\n");
		return E_OUTOFMEMORY;
	}
	catch (...)
	{
		OutputDebugStringA("[Loader][Map] Map preparation failed with an exception.\n");
		return E_FAIL;
	}
}

HRESULT CLoader::Ready_MapAuthoringCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) ||
		FAILED(Ready_StaticMeshShader(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			CMapAssetPreview::SHADER_PROTOTYPE_TAG,
			CShader::Create(
				m_pDevice,
				m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements,
				VTXMESH::iNumElements))))
	{
		return E_FAIL;
	}

	Set_Status(TEXT("Map: editor core prototypes"));
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
			VTXMESHINSTANCE::Elements,
			VTXMESHINSTANCE::iNumElements))) ||
		FAILED(Ready_Camera_Prototype(iLevelIndex)) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapAsset"),
			CMapAssetObject::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_MapStaticBatch"),
			CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

#ifdef _DEBUG
	/* Every level the Map Tool can author stages its trigger, collision and
	   spawn anchor boxes as TriggerBox clones under that level's index. */
	if ((iLevelIndex == ETOUI(LEVEL::DEVELOPMENT) ||
		iLevelIndex == ETOUI(LEVEL::MAHARAKA) ||
		iLevelIndex == ETOUI(LEVEL::CHARACTER_SELECT) ||
		iLevelIndex == ETOUI(LEVEL::BERN) ||
		iLevelIndex == ETOUI(LEVEL::VALTAN_ARENA) ||
		iLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA)) &&
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			CTrigger_Box::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
#endif

	return S_OK;
}

HRESULT CLoader::Ready_Camera_Prototype(
	const uint32_t iLevelIndex)
{
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext));
}

HRESULT CLoader::Ready_StaticMeshShader(
	const uint32_t iLevelIndex)
{
	auto shader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
		VTXMESH::Elements,
		VTXMESH::iNumElements);
	if (!shader)
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary creation failed (compiled shader or program variants)"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary creation failed before prototype registration.\n");
		return E_FAIL;
	}
	const HRESULT result = CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		std::move(shader));
	if (FAILED(result))
	{
		Set_Status(TEXT("Map: Shader_VtxMeshBinary prototype registration failed"));
		OutputDebugStringA("[Loader][Shader] Shader_VtxMeshBinary prototype registration failed.\n");
	}
	return result;
}

HRESULT CLoader::Ready_AnimatedMeshShader(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))))
	{
		return E_FAIL;
	}
	/* Esther summon NPCs and their screen cutin pin their own copy of the
	   deferred material path so shared-shader pass churn cannot shift them. */
	return CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Shader_VtxEstherNpc"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEstherNpc.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements));
}

HRESULT CLoader::Ready_DeployPropCore(const uint32_t iLevelIndex)
{
	if (iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	std::string status;
	if (!CDeployPropRuntime::Ensure_ObjectPrototype(
		m_pDevice, m_pContext, iLevelIndex, status))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status +
			"\n").c_str());
		return E_FAIL;
	}
	return S_OK;
}

/* The deploy admission itself lives with CDeployPropRuntime so an Area whose
   loader contract excludes deploy can stage the same prototypes from its own
   Level without a second implementation. */
HRESULT CLoader::Ready_DeployPropArea(
	const uint32_t iLevelIndex,
	const std::string& areaId)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || areaId.empty())
		return E_INVALIDARG;

	std::string status;
	if (!CDeployPropRuntime::Ensure_AreaPrototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex,
		areaId,
		status,
		[this]() -> bool_t
		{
			return m_isCancellationRequested.load(
				std::memory_order_acquire);
		}))
	{
		OutputDebugStringA(("[Loader][DeployProp] " + status +
			"\n").c_str());
		return m_isCancellationRequested.load(std::memory_order_acquire) ?
			HRESULT_FROM_WIN32(ERROR_CANCELLED) : E_FAIL;
	}
	return S_OK;
}
HRESULT CLoader::Ready_Character_Rendering(
	const uint32_t iLevelIndex,
	const std::span<const LostArk::Shared::CHARACTER_CLASS_ID>
		characterClasses)
{
	if (iLevelIndex >= ETOUI(LEVEL::END) || characterClasses.empty())
	{
		return E_INVALIDARG;
	}
	for (const auto characterClass : characterClasses)
	{
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass))
		{
			return E_INVALIDARG;
		}
	}

	CPlayableCharacterAssetService::Begin_LevelLoad(iLevelIndex);

	if (FAILED(Ready_AnimatedMeshShader(iLevelIndex)) ||
		FAILED(Ready_Character_Shared_Prototypes(iLevelIndex)))
	{
		return E_FAIL;
	}

	for (size_t classIndex = 0;
		classIndex < characterClasses.size();
		++classIndex)
	{
		const auto characterClass = characterClasses[classIndex];
		const auto authoring = m_CharacterAuthoringInputs.find(characterClass);
		if (authoring == m_CharacterAuthoringInputs.end() || !authoring->second) return E_INVALIDARG;
		const auto progress = [
			this,
			classIndex,
			classCount = characterClasses.size(),
			characterClass](
			const size_t completedModelCount,
			const size_t totalModelCount,
			const std::string&)
		{
			tchar_t status[MAX_PATH]{};
			_snwprintf_s(
				status,
				std::size(status),
				_TRUNCATE,
				TEXT("Character %zu/%zu | %s models"),
				classIndex + 1u,
				classCount,
				Get_CharacterClassName(characterClass));
			Set_DeterminateStatus(
				status, completedModelCount, totalModelCount);
		};
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			characterClass,
			&m_isCancellationRequested,
			progress,
			authoring->second,
			&m_AssetPreparationBatch)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLoader::Ready_Character_Shared_Prototypes(
	const uint32_t iLevelIndex)
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		CPart_Equipment::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Part_Body"),
			CPart_Body::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			iLevelIndex,
			TEXT("Prototype_GameObject_Part_Vehicle"),
			CPart_Vehicle::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_GameObject_Character"),
		CCharacter::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Collider_Player"),
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex,
		TEXT("Prototype_Component_Collider_WorldEntity"),
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::SPHERE))))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLoader::Ready_AnimationPreviewModels(
	const uint32_t iLevelIndex)
{
#ifdef _DEBUG
	if (iLevelIndex != ETOUI(LEVEL::CHARACTER_SELECT) &&
		iLevelIndex != ETOUI(LEVEL::DEVELOPMENT))
	{
		return E_INVALIDARG;
	}

	constexpr std::array playablePrototypeTags =
	{
		TEXT("Prototype_Component_Model_LanceMaster"),
		TEXT("Prototype_Component_Model_GunSlinger"),
		TEXT("Prototype_Component_Model_Slayer"),
		TEXT("Prototype_Component_Model_Artist"),
		TEXT("Prototype_Component_Model_DimensionMaster"),
		TEXT("Prototype_Component_Model_Warlord"),
		TEXT("Prototype_Component_Model_GuardianKnight")
	};

	for (const ANIMATION_PREVIEW_ASSET& asset :
		ANIMATION_PREVIEW_ASSETS)
	{
		bool_t bPlayablePrototype = false;
		for (const wchar_t* pPlayableTag : playablePrototypeTags)
		{
			if (0 == wcscmp(asset.pPrototypeTag, pPlayableTag))
			{
				bPlayablePrototype = true;
				break;
			}
		}
		// Playable classes are loaded on demand by Character Select. This direct
		// path is only for optional non-playable authoring previews.
		if (bPlayablePrototype)
		{
			continue;
		}
		if (nullptr == asset.pModelAssetId ||
			nullptr == asset.pPrototypeTag ||
			!std::isfinite(asset.fPreviewScale) ||
			asset.fPreviewScale <= 0.f ||
			!std::isfinite(asset.fPreviewYawDegrees))
		{
			return E_FAIL;
		}
		const BOSS_ACTOR_ENTRY* pBoss = nullptr;
		if (nullptr != asset.pBossArchetypeId)
		{
			pBoss = CActorCatalog::Find_Boss(asset.pBossArchetypeId);
			if (nullptr == pBoss || pBoss->bodyModel != asset.pModelAssetId)
				return E_FAIL;

			/* Product Valtan is optional Model View content. It is prepared on
			   selection (or before an actual Server spawn), not on every Character
			   Select entry. */
			continue;
		}
		const filesystem::path path =
			CRuntimeAssetRoot::Resolve(asset.pModelAssetId);
		if (path.empty() || !filesystem::is_regular_file(path))
			continue;
		const matrix_t previewTransform =
			XMMatrixScaling(
				asset.fPreviewScale,
				asset.fPreviewScale,
				asset.fPreviewScale) *
			XMMatrixRotationY(
				XMConvertToRadians(asset.fPreviewYawDegrees));

		unique_ptr<CModel> model = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::ANIM,
			path.string().c_str(),
			previewTransform);
		if (nullptr == model)
			continue;
		/* A preview-only body may declare its own donor. The attach is the same
		   contract the product boss uses, so a donor built against another rig
		   fails here instead of previewing a torn mesh. */
		if (nullptr != asset.pAnimationSetAssetId)
		{
			const filesystem::path donorPath =
				CRuntimeAssetRoot::Resolve(asset.pAnimationSetAssetId);
			if (donorPath.empty() || !filesystem::is_regular_file(donorPath))
				return E_FAIL;
			const unique_ptr<CModel> donor = CModel::Create(
				m_pDevice,
				m_pContext,
				MODEL::ANIM,
				donorPath.string().c_str(),
				previewTransform);
			if (nullptr == donor ||
				FAILED(model->Attach_AnimationSet(*donor)))
			{
				return E_FAIL;
			}
		}
		if (nullptr != pBoss)
		{
			const filesystem::path animSetPath =
				CRuntimeAssetRoot::Resolve(pBoss->animationSetId);
			if (!animSetPath.empty() &&
				filesystem::is_regular_file(animSetPath))
			{
				const unique_ptr<CModel> animSet = CModel::Create(
					m_pDevice,
					m_pContext,
					MODEL::ANIM,
					animSetPath.string().c_str(),
					previewTransform);
				if (nullptr == animSet ||
					FAILED(model->Attach_AnimationSet(*animSet)))
				{
					return E_FAIL;
				}
			}
		}
		if (FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.pPrototypeTag,
				move(model))))
		{
			return E_FAIL;
		}

		/* An optional socketed weapon is prepared with its body so Development
		   and Character Select stage the same composition. It is a static mesh
		   riding one bone, so only the unit ratio to the body is applied: the
		   socket bone matrix already carries the preview scale and yaw. */
		const bool_t bDeclaresWeapon =
			nullptr != asset.pWeaponModelAssetId &&
			nullptr != asset.pWeaponPrototypeTag &&
			nullptr != asset.pWeaponSocketBone;
		if (!bDeclaresWeapon)
			continue;
		if (!std::isfinite(asset.fWeaponScale) || asset.fWeaponScale <= 0.f)
			return E_FAIL;
		const filesystem::path weaponPath =
			CRuntimeAssetRoot::Resolve(asset.pWeaponModelAssetId);
		if (weaponPath.empty() || !filesystem::is_regular_file(weaponPath))
			continue;
		unique_ptr<CModel> weaponModel = CModel::Create(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			weaponPath.string().c_str(),
			XMMatrixScaling(
				asset.fWeaponScale,
				asset.fWeaponScale,
				asset.fWeaponScale));
		if (nullptr == weaponModel ||
			FAILED(CGameInstance::Get().Add_Prototype(
				iLevelIndex,
				asset.pWeaponPrototypeTag,
				move(weaponModel))))
		{
			return E_FAIL;
		}
	}
#endif
	return S_OK;
}

HRESULT CLoader::Ready_ValtanPresentation(const uint32_t iLevelIndex)
{
	if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			"BOSS_VALTAN")))
	{
		return E_FAIL;
	}
	/* The finale repeatedly checks out the ghost rig.  Register its body and
	animation-set prototypes while the arena loader still owns the loading
	window, never on the first visible dependent-boss spawn. */
	return CValtanPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		iLevelIndex,
		"BOSS_VALTAN_GHOST");
}

void CLoader::Ready_EstherSummonPresentation(const uint32_t iLevelIndex)
{
	for (const char* pEstherArchetypeId :
		{ "NPC_59030", "NPC_58700", "NPC_59060", "NPC_59504", "NPC_59620" })
	{
		if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			iLevelIndex,
			pEstherArchetypeId)))
		{
			OutputDebugStringA(
				(std::string("[Loader][NpcPresentation] esther summon "
					"presentation is unavailable (") + pEstherArchetypeId +
					"); the arena loads without it.\n").c_str());
		}
		(void)CEstherCutinPresentationService::Preload_Frames(
			m_pDevice.Get(), pEstherArchetypeId);
	}
	(void)CEstherActionSoundCueDocument::Preload_Sounds();
}

unique_ptr<CLoader> CLoader::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const uint64_t iEffectLoadJobEpoch,
	const uint64_t iEffectCatalogRevision)
{
	auto pInstance = unique_ptr<CLoader>(
		new CLoader(pDevice, pContext));
	if (FAILED(pInstance->Initialize(
		eNextLevelID, iEffectLoadJobEpoch, iEffectCatalogRevision)))
		return nullptr;
	return pInstance;
}

void CLoader::Free()
{
	Request_Cancellation();
	HANDLE workers[2]{};
	DWORD workerCount = 0u;
	if (m_hThread)
		workers[workerCount++] = m_hThread;
	if (m_hEffectThread)
		workers[workerCount++] = m_hEffectThread;
	if (workerCount != 0u)
	{
		const auto FailFastOnWaitFailure = [](const DWORD waitResult) noexcept
		{
			if (WAIT_FAILED != waitResult)
				return;
			const DWORD errorCode = GetLastError();
			OutputDebugStringA(
				"[Loader] Worker wait failed; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(
					GetCurrentProcess(),
					ERROR_SUCCESS == errorCode ? ERROR_INVALID_HANDLE : errorCode))
			{
				std::terminate();
			}
			__assume(0);
		};
		DWORD waitResult = WaitForMultipleObjects(workerCount, workers, TRUE, 5000);
		FailFastOnWaitFailure(waitResult);
		if (WAIT_TIMEOUT == waitResult)
		{
			for (DWORD index = 0u; index < workerCount; ++index)
				CancelSynchronousIo(workers[index]);
			m_AssetPreparationBatch.Cancel_SynchronousIo();
			m_EffectPreparationBatch.Cancel_SynchronousIo();
			waitResult = WaitForMultipleObjects(workerCount, workers, TRUE, 5000);
			FailFastOnWaitFailure(waitResult);
		}
		if (WAIT_TIMEOUT == waitResult)
		{
			OutputDebugStringA(
				"[Loader] Worker exceeded shutdown deadline; terminating the process to preserve loader invariants.\n");
			if (!TerminateProcess(GetCurrentProcess(), ERROR_TIMEOUT))
				std::terminate();
			__assume(0);
		}
		for (DWORD index = 0u; index < workerCount; ++index)
			CloseHandle(workers[index]);
		m_hThread = nullptr;
		m_hEffectThread = nullptr;
	}
}
```

## Client/Private/WorldSequenceDocument.cpp

```cpp
#include "WorldSequenceDocument.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "SourceCharacterMaterialParameters.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-sequences";
	constexpr uint32_t FORMAT_VERSION = 3;
	constexpr uint32_t LEGACY_FORMAT_VERSION = 1;
	constexpr f32_t MIN_SCALE = 0.000001f;
	constexpr f32_t MIN_RUNTIME_SCALE_DETERMINANT = 0.000001f;
	constexpr f32_t MAX_COMPONENT = 100000.f;
	constexpr uintmax_t MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u;

	bool_t Is_ValidUtf8DisplayText(const std::string& value, const bool_t allowLineFeed = false)
	{
		for (size_t offset = 0u; offset < value.size();)
		{
			const uint8_t first = static_cast<uint8_t>(value[offset]);
			if (first < 0x80u)
			{
				if ((first < 0x20u && !(allowLineFeed && first == 0x0au)) || 0x7fu == first)
					return false;
				++offset;
				continue;
			}
			size_t length = 0u;
			uint32_t codePoint = 0u;
			uint32_t minimum = 0u;
			if (first >= 0xc2u && first <= 0xdfu)
			{
				length = 2u;
				codePoint = first & 0x1fu;
				minimum = 0x80u;
			}
			else if (first >= 0xe0u && first <= 0xefu)
			{
				length = 3u;
				codePoint = first & 0x0fu;
				minimum = 0x800u;
			}
			else if (first >= 0xf0u && first <= 0xf4u)
			{
				length = 4u;
				codePoint = first & 0x07u;
				minimum = 0x10000u;
			}
			else
			{
				return false;
			}
			if (offset + length > value.size())
				return false;
			for (size_t index = 1u; index < length; ++index)
			{
				const uint8_t next = static_cast<uint8_t>(value[offset + index]);
				if ((next & 0xc0u) != 0x80u)
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if (codePoint < minimum || codePoint > 0x10ffffu ||
				(codePoint >= 0xd800u && codePoint <= 0xdfffu))
			{
				return false;
			}
			offset += length;
		}
		return true;
	}

	bool_t Is_IntegerNumber(const DATA_JSON_VALUE* value)
	{
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number()) &&
			std::floor(value->Get_Number()) == value->Get_Number();
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Is_ObjectShape(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> required,
		const std::initializer_list<const char_t*> optional)
	{
		if (!value.Is_Object()) return false;
		for (const char_t* key : required)
			if (nullptr == value.Find(key)) return false;
		for (const auto& entry : value.Get_Object())
		{
			const auto matches = [&entry](const char_t* key) { return entry.first == key; };
			if (std::none_of(required.begin(), required.end(), matches) &&
				std::none_of(optional.begin(), optional.end(), matches)) return false;
		}
		return true;
	}

	bool_t Is_BoundedFloat3(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
			std::abs(value.x) <= MAX_COMPONENT && std::abs(value.y) <= MAX_COMPONENT &&
			std::abs(value.z) <= MAX_COMPONENT;
	}

	bool_t Is_ResourcePath(const std::string& value, const bool_t model)
	{
		if (value.empty() || value.size() > 1024u || value.front() == '/' ||
			value.find(':') != std::string::npos || value.find('\\') != std::string::npos ||
			!Is_ValidUtf8DisplayText(value)) return false;
		std::istringstream parts(value);
		std::string part;
		while (std::getline(parts, part, '/'))
			if (part.empty() || part == "." || part == "..") return false;
		std::string extension = value.size() > 7u ? value.substr(value.size() - 7u) : std::string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](const unsigned char character) { return static_cast<char_t>(std::tolower(character)); });
		return value.back() != '/' && (!model || extension == ".wmodel");
	}

    bool_t Validate_MaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS* out = nullptr)
    {
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS packed;
        if (profile.materialName.empty() || profile.materialName.size() > 63u ||
            !Is_ValidUtf8DisplayText(profile.materialName) || profile.sourceMaterial.empty() ||
            profile.sourceMaterial.size() > 512u || !Is_ValidUtf8DisplayText(profile.sourceMaterial) ||
            !SourceCharacterMaterial::Configure(profile.family, profile.parameters, packed)) return false;
        for (const auto& [name, values] : profile.parameters)
            for (const auto value : values)
                if (!std::isfinite(value) || std::abs(value) > 1000000.f) return false;
        const uint32_t required = packed.baseTextureMask | packed.lightTextureMask;
        uint32_t supplied = 0u;
        for (const auto& texture : profile.textures)
        {
            if (texture.expressionIndex >= Engine::SOURCE_CHARACTER_TEXTURE_COUNT ||
                !Is_ResourcePath(texture.assetId, false)) return false;
            const auto bit = 1u << texture.expressionIndex;
            if ((supplied & bit) != 0u || (required & bit) == 0u) return false;
            supplied |= bit;
        }
        if (supplied != required) return false;
        if (out) *out = packed;
        return true;
    }

	/* Authored rows must agree with the seeded fields they replace, so a
	   document never carries two different answers for the emission count. */
	bool_t Is_ValidEmissionList(const WORLD_SEQUENCE_OBJECT_MOTION& motion)
	{
		if (motion.emissions.empty()) return true;
		if (motion.emissions.size() > 128u || motion.count != motion.emissions.size() ||
			0u != motion.intervalMs || 0.f != motion.spreadDegrees) return false;
		for (const auto& emission : motion.emissions)
		{
			if (!Is_BoundedFloat3(emission.positionOffset) || !std::isfinite(emission.yawDegrees) ||
				emission.yawDegrees < -36000.f || emission.yawDegrees > 36000.f ||
				emission.startDelayMs > CWorldSequenceDocument::MAX_DURATION_MS) return false;
		}
		return true;
	}

	bool_t Read_Uint32(
		const DATA_JSON_VALUE* value,
		uint32_t& outValue,
		const uint32_t maximum = UINT32_MAX)
	{
		if (!Is_IntegerNumber(value) || value->Get_Number() < 0.0 ||
			value->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(value->Get_Number());
		return true;
	}

	bool_t Read_FiniteFloat(const DATA_JSON_VALUE* value, f32_t& outValue)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Read_Float3(const DATA_JSON_VALUE* value, float3_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		return Read_FiniteFloat(&values[0], outValue.x) &&
			Read_FiniteFloat(&values[1], outValue.y) &&
			Read_FiniteFloat(&values[2], outValue.z);
	}

	bool_t Read_Quaternion(const DATA_JSON_VALUE* value, float4_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			4u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		if (!Read_FiniteFloat(&values[0], outValue.x) ||
			!Read_FiniteFloat(&values[1], outValue.y) ||
			!Read_FiniteFloat(&values[2], outValue.z) ||
			!Read_FiniteFloat(&values[3], outValue.w))
		{
			return false;
		}
		const vector_t raw = XMLoadFloat4(&outValue);
		const f32_t length = XMVectorGetX(XMVector4Length(raw));
		if (!std::isfinite(length) ||
			std::abs(length - 1.f) > 0.001f)
			return false;
		if (outValue.w < 0.f)
		{
			outValue.x = -outValue.x;
			outValue.y = -outValue.y;
			outValue.z = -outValue.z;
			outValue.w = -outValue.w;
		}
		return true;
	}

	bool_t Parse_Uint64String(const DATA_JSON_VALUE* value, uint64_t& outValue)
	{
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		const std::string& text = value->Get_String();
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Parse_Uint64Text(const std::string& text, uint64_t& outValue)
	{
		if (text.empty())
			return false;
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Is_FiniteTransform(const WORLD_SEQUENCE_TRANSFORM_KEY& key)
	{
		const auto finiteBounded = [](const f32_t value)
		{
			return std::isfinite(value) && std::abs(value) <= MAX_COMPONENT;
		};
		if (!finiteBounded(key.positionOffset.x) ||
			!finiteBounded(key.positionOffset.y) ||
			!finiteBounded(key.positionOffset.z) ||
			!finiteBounded(key.scaleMultiplier.x) ||
			!finiteBounded(key.scaleMultiplier.y) ||
			!finiteBounded(key.scaleMultiplier.z) ||
			std::abs(key.scaleMultiplier.x) < MIN_SCALE ||
			std::abs(key.scaleMultiplier.y) < MIN_SCALE ||
			std::abs(key.scaleMultiplier.z) < MIN_SCALE)
		{
			return false;
		}
		const vector_t quaternion = XMLoadFloat4(&key.rotationQuaternion);
		const f32_t length = XMVectorGetX(XMVector4Length(quaternion));
		return std::isfinite(length) && std::abs(length - 1.f) <= 0.001f &&
			key.rotationQuaternion.w >= 0.f;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) && !existsError &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CWorldSequenceDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus)
{
	CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Load");
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		if (existsError)
		{
			outStatus = "Could not inspect world sequence document";
			return false;
		}
		Reset_Empty(expectedAreaId);
		outStatus = "No world sequence document; starting empty";
		return true;
	}
	if (!std::filesystem::is_regular_file(path, existsError) || existsError)
	{
		outStatus = "World sequence document is not a regular file";
		return false;
	}
	const uintmax_t fileBytes = std::filesystem::file_size(path, existsError);
	if (existsError || fileBytes > MAX_DOCUMENT_BYTES)
	{
		outStatus = existsError ?
			"Could not inspect world sequence document size" :
			"World sequence document exceeds the 16 MiB parse limit";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open world sequence document file";
		return false;
	}
	std::string text;
	try
	{
		text.resize(static_cast<size_t>(fileBytes));
	}
	catch (const std::bad_alloc&)
	{
		outStatus = "Could not allocate bounded world sequence input";
		return false;
	}
	if (!text.empty())
		input.read(text.data(), static_cast<std::streamsize>(text.size()));
	if (input.bad() || input.gcount() != static_cast<std::streamsize>(text.size()) ||
		std::char_traits<char_t>::eof() != input.peek())
	{
		outStatus = "World sequence document changed or failed while reading";
		return false;
	}
	return Load_Text(text, expectedAreaId, availablePlacements, availableDeployPlacements, outStatus);
}

bool_t Client::CWorldSequenceDocument::Load_Text(const std::string_view text,
	const std::string& expectedAreaId, const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements, std::string& outStatus)
{
	if (text.size() > MAX_DOCUMENT_BYTES)
	{ outStatus = "World sequence document exceeds the 16 MiB parse limit"; return false; }
	DATA_JSON_VALUE root;
	std::string parseError;
	bool_t parsed = false;
	{
		CProfilerScope parseScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Parse");
		parsed = CDataJson::Parse(text, root, parseError);
	}
	if (!parsed ||
		!Is_ObjectShape(root,
			{ "schema", "formatVersion", "areaId", "revision",
			  "templates", "instances" }, { "objectResources", "objectFolders" }))
	{
		outStatus = "World sequence JSON root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areaId = root.Find("areaId");
	const DATA_JSON_VALUE* revision = root.Find("revision");
	const DATA_JSON_VALUE* templates = root.Find("templates");
	const DATA_JSON_VALUE* instances = root.Find("instances");
	uint32_t parsedFormatVersion = 0;
	uint32_t parsedRevision = 0;
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != SCHEMA ||
		!Read_Uint32(version, parsedFormatVersion) ||
		(parsedFormatVersion < LEGACY_FORMAT_VERSION || parsedFormatVersion > FORMAT_VERSION) ||
		nullptr == areaId || !areaId->Is_String() ||
		areaId->Get_String() != expectedAreaId ||
		!Read_Uint32(revision, parsedRevision) || 0u == parsedRevision ||
		nullptr == templates || !templates->Is_Array() ||
		templates->Get_Array().size() > MAX_TEMPLATE_COUNT ||
		nullptr == instances || !instances->Is_Array() ||
		instances->Get_Array().size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence header is invalid or belongs to another Area";
		return false;
	}

	CWorldSequenceDocument staged;
	staged.m_AreaId = expectedAreaId;
	staged.m_iRevision = parsedRevision;
    if (const auto* folders = root.Find("objectFolders"))
    {
        if (parsedFormatVersion != 3u || !folders->Is_Array() || folders->Get_Array().size() > MAX_INSTANCE_COUNT)
        { outStatus = "World object folder list is invalid"; return false; }
        for (const auto& row : folders->Get_Array())
        {
            if (!Is_ObjectShape(row, {"folderId", "displayName"}, {"anchorKind", "parentId"}) ||
                !row.Find("folderId")->Is_String() || !row.Find("displayName")->Is_String())
            { outStatus = "World object folder fields are invalid"; return false; }
            WORLD_SEQUENCE_OBJECT_FOLDER folder;
            folder.folderId = row.Find("folderId")->Get_String();
            folder.displayName = row.Find("displayName")->Get_String();
            for (const char_t* key : {"anchorKind", "parentId"})
                if (const auto* value = row.Find(key))
                {
                    if (!value->Is_String()) { outStatus = "World object folder anchor and parent must be text"; return false; }
                    (std::string(key) == "anchorKind" ? folder.anchorKind : folder.parentId) = value->Get_String();
                }
            staged.m_ObjectFolders.push_back(std::move(folder));
        }
    }
	const DATA_JSON_VALUE* objects = root.Find("objectResources");
	if ((parsedFormatVersion < 3u && nullptr != objects) ||
		(parsedFormatVersion == 3u && (nullptr == objects || !objects->Is_Array() ||
			objects->Get_Array().size() > MAX_INSTANCE_COUNT)))
	{
		outStatus = "World object resource list is invalid";
		return false;
	}
	if (nullptr != objects)
	{
		for (const DATA_JSON_VALUE& row : objects->Get_Array())
		{
			WORLD_SEQUENCE_OBJECT_RESOURCE object;
			if (!Is_ObjectShape(row, { "objectId", "displayName", "modelAssetId", "modelPreScale",
				"animated", "scale" }, { "diffuseTextureAssetId", "sequenceInstanceId", "anchorKind", "defaultMotionInstanceId", "anchorBossArchetypeId", "anchorBone", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings", "motionInstanceIds", "combatBody", "animationSetAssetId", "presentationBossArchetypeId", "parentId" }) ||
				!row.Find("objectId")->Is_String() || !row.Find("displayName")->Is_String() ||
				!row.Find("modelAssetId")->Is_String() || !row.Find("animated")->Is_Boolean() ||
				!Read_FiniteFloat(row.Find("modelPreScale"), object.modelPreScale) ||
				!Read_Float3(row.Find("scale"), object.scale))
			{
				outStatus = "World object resource fields are invalid";
				return false;
			}
			object.objectId = row.Find("objectId")->Get_String();
			object.displayName = row.Find("displayName")->Get_String();
            if (const auto* parent = row.Find("parentId"))
            {
                if (!parent->Is_String()) { outStatus = "World object parent ID must be text"; return false; }
                object.parentId = parent->Get_String();
            }
			object.modelAssetId = row.Find("modelAssetId")->Get_String();
			object.animated = row.Find("animated")->Get_Boolean();
			if (const auto* value = row.Find("combatBody"))
			{
				WORLD_SEQUENCE_COMBAT_BODY body;
				if (!Is_ObjectShape(*value, {"maxHp", "localCenterM", "halfExtentsM", "lifetimePolicy"}, {"shape"}) ||
					!Read_Uint32(value->Find("maxHp"), body.maxHp, 1000000000u) ||
					!Read_Float3(value->Find("localCenterM"), body.localCenterM) ||
					!Read_Float3(value->Find("halfExtentsM"), body.halfExtentsM) ||
					!value->Find("lifetimePolicy")->Is_String())
				{ outStatus = "Invalid World Object combat body"; return false; }
				if (const auto* shape = value->Find("shape"))
				{
					if (!shape->Is_String()) { outStatus = "World Object combat shape must be text"; return false; }
					body.shape = shape->Get_String();
				}
				body.lifetimePolicy = value->Find("lifetimePolicy")->Get_String();
				object.combatBody = std::move(body);
			}
			if (const auto* members = row.Find("motionInstanceIds"))
			{
				if (!members->Is_Array() || members->Get_Array().empty() || members->Get_Array().size() > 32u)
				{ outStatus = "Object group requires 1..32 motion instance IDs"; return false; }
				for (const auto& member : members->Get_Array())
				{
					if (!member.Is_String()) { outStatus = "Object group member ID must be text"; return false; }
					object.motionInstanceIds.push_back(member.Get_String());
				}
			}
			if (const auto* motion = row.Find("defaultMotionInstanceId"))
			{
				if (!motion->Is_String()) { outStatus = "Default Motion instance ID must be text"; return false; }
				object.defaultMotionInstanceId = motion->Get_String();
			}
			if (const auto* anchor = row.Find("anchorKind"))
			{
				if (!anchor->Is_String()) { outStatus = "World object resource anchor must be WORLD, PLAYER or BOSS"; return false; }
				object.anchorKind = anchor->Get_String();
			}
			for (const char_t* key : { "anchorBossArchetypeId", "anchorBone" })
			{
				const auto* field = row.Find(key);
				if (!field) continue;
				if (!field->Is_String()) { outStatus = "World object boss anchor fields must be text"; return false; }
				(std::string(key) == "anchorBone" ? object.anchorBone : object.anchorBossArchetypeId) = field->Get_String();
			}
			for (const char_t* key : { "diffuseTextureAssetId", "sequenceInstanceId" })
			{
				const auto* field = row.Find(key);
				if (nullptr == field) continue;
				if (!field->Is_String()) { outStatus = "Invalid world object optional path/reference"; return false; }
				(std::string(key) == "sequenceInstanceId" ? object.sequenceInstanceId :
					object.diffuseTextureAssetId) = field->Get_String();
			}
            if (const auto* source = row.Find("materialSourceModelAssetId"))
            {
                if (!source->Is_String() || !Is_ResourcePath(source->Get_String(), true))
                { outStatus = "Invalid world object material source model: " + object.objectId; return false; }
                object.materialSourceModelAssetId = source->Get_String();
            }
            if (const auto* animationSet = row.Find("animationSetAssetId"))
            {
                if (!animationSet->Is_String() || !Is_ResourcePath(animationSet->Get_String(), true))
                { outStatus = "Invalid world object animation set: " + object.objectId; return false; }
                object.animationSetAssetId = animationSet->Get_String();
            }
            if (const auto* presentation = row.Find("presentationBossArchetypeId"))
            {
                if (!presentation->Is_String())
                { outStatus = "Invalid world object presentation boss: " + object.objectId; return false; }
                object.presentationBossArchetypeId = presentation->Get_String();
            }
            if (const auto* bindings = row.Find("mapMaterialBindings"))
            {
                if (!bindings->Is_Array() || bindings->Get_Array().size() > 64u)
                { outStatus = "Invalid world object map material bindings"; return false; }
                for (const auto& binding : bindings->Get_Array())
                {
                    if (!Is_ObjectShape(binding, { "materialName", "sourceAssetId", "sourceMaterialName" }, { "diffuseTextureAssetId", "unlit" }) ||
                        !binding.Find("materialName")->Is_String() || !binding.Find("sourceAssetId")->Is_String() ||
                        !binding.Find("sourceMaterialName")->Is_String())
                    { outStatus = "Invalid world object map material binding"; return false; }
                    WORLD_SEQUENCE_MAP_MATERIAL_BINDING material;
                    material.materialName = binding.Find("materialName")->Get_String();
                    material.sourceAssetId = binding.Find("sourceAssetId")->Get_String();
                    material.sourceMaterialName = binding.Find("sourceMaterialName")->Get_String();
                    if (const auto* diffuse = binding.Find("diffuseTextureAssetId"))
                    {
                        if (!diffuse->Is_String() || !Is_ResourcePath(diffuse->Get_String(), false))
                        { outStatus = "Invalid map material diffuse texture"; return false; }
                        material.diffuseTextureAssetId = diffuse->Get_String();
                    }
                    if (const auto* unlit = binding.Find("unlit"))
                    {
                        if (!unlit->Is_Boolean())
                        { outStatus = "Invalid map material unlit flag"; return false; }
                        material.unlit = unlit->Get_Boolean();
                    }
                    object.mapMaterialBindings.push_back(std::move(material));
                }
            }
            if (const auto* value = row.Find("materialProfile"))
            {
                WORLD_SEQUENCE_MATERIAL_PROFILE profile;
                if (!Is_ExactObject(*value, { "materialName", "sourceMaterial", "family", "parameters", "textures" }) ||
                    !value->Find("materialName")->Is_String() || !value->Find("sourceMaterial")->Is_String() ||
                    !value->Find("family")->Is_String() || !value->Find("textures")->Is_Array() ||
                    !SourceCharacterMaterial::Read(*value->Find("parameters"), profile.parameters))
                { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
                profile.materialName = value->Find("materialName")->Get_String();
                profile.sourceMaterial = value->Find("sourceMaterial")->Get_String();
                profile.family = value->Find("family")->Get_String();
                for (const auto& texture : value->Find("textures")->Get_Array())
                {
                    WORLD_SEQUENCE_MATERIAL_TEXTURE input;
                    if (!Is_ExactObject(texture, { "expressionIndex", "assetId", "colorSpace" }) ||
                        !Read_Uint32(texture.Find("expressionIndex"), input.expressionIndex) ||
                        !texture.Find("assetId")->Is_String() || !texture.Find("colorSpace")->Is_String() ||
                        (texture.Find("colorSpace")->Get_String() != "srgb" && texture.Find("colorSpace")->Get_String() != "linear"))
                    { outStatus = "Invalid world object material texture: " + object.objectId; return false; }
                    input.assetId = texture.Find("assetId")->Get_String();
                    input.srgb = texture.Find("colorSpace")->Get_String() == "srgb";
                    profile.textures.push_back(std::move(input));
                }
                if (!Validate_MaterialProfile(profile))
                { outStatus = "World object material input contract failed: " + object.objectId; return false; }
                object.materialProfile = std::move(profile);
            }
			staged.m_ObjectResources.push_back(std::move(object));
		}
	}
	for (const DATA_JSON_VALUE& templateValue : templates->Get_Array())
	{
		const bool_t validTemplateShape =
			LEGACY_FORMAT_VERSION == parsedFormatVersion ?
			Is_ExactObject(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks" }) :
			Is_ObjectShape(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks", "animationTracks" }, { "objectMotion", "effectTracks", "colliderTracks", "soundTracks", "subtitleTracks", "materialTracks" });
		if (!validTemplateShape)
		{
			outStatus = "World sequence template shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* sequenceId = templateValue.Find("sequenceId");
		const DATA_JSON_VALUE* displayName = templateValue.Find("displayName");
		const DATA_JSON_VALUE* category = templateValue.Find("category");
		const DATA_JSON_VALUE* interpolation = templateValue.Find("interpolation");
		const DATA_JSON_VALUE* tracks = templateValue.Find("tracks");
		const DATA_JSON_VALUE* animationTracks =
			templateValue.Find("animationTracks");
		WORLD_SEQUENCE_TEMPLATE parsedTemplate;
		if (nullptr == sequenceId || !sequenceId->Is_String() ||
			nullptr == displayName || !displayName->Is_String() ||
			nullptr == category || !category->Is_String() ||
			!Read_Uint32(templateValue.Find("durationMs"),
				parsedTemplate.durationMs, MAX_DURATION_MS) ||
			nullptr == interpolation || !interpolation->Is_String() ||
			!Try_ParseInterpolation(interpolation->Get_String(),
				parsedTemplate.interpolation) ||
			nullptr == tracks || !tracks->Is_Array() ||
			tracks->Get_Array().size() > MAX_TRACK_COUNT ||
			(2u <= parsedFormatVersion &&
				(nullptr == animationTracks || !animationTracks->Is_Array() ||
					animationTracks->Get_Array().size() > MAX_TRACK_COUNT ||
					tracks->Get_Array().size() +
						animationTracks->Get_Array().size() > MAX_TRACK_COUNT)))
		{
			outStatus = "World sequence template fields are invalid";
			return false;
		}
		parsedTemplate.sequenceId = sequenceId->Get_String();
		parsedTemplate.displayName = displayName->Get_String();
		parsedTemplate.category = category->Get_String();
		if (const DATA_JSON_VALUE* motion = templateValue.Find("objectMotion"))
		{
			auto& value = parsedTemplate.objectMotion;
			if (parsedFormatVersion < 3u || !Is_ObjectShape(*motion,
				{ "velocity", "acceleration", "angularVelocityDegrees", "revolutionDegreesPerSecond",
				  "revolutionOffset", "count", "intervalMs", "spreadDegrees", "seed" }, { "spawnHalfExtents", "emissions" }) ||
				!Read_Float3(motion->Find("velocity"), value.velocity) ||
				!Read_Float3(motion->Find("acceleration"), value.acceleration) ||
				!Read_Float3(motion->Find("angularVelocityDegrees"), value.angularVelocityDegrees) ||
				!Read_Float3(motion->Find("revolutionDegreesPerSecond"), value.revolutionDegreesPerSecond) ||
				!Read_Float3(motion->Find("revolutionOffset"), value.revolutionOffset) ||
				(motion->Find("spawnHalfExtents") && !Read_Float3(motion->Find("spawnHalfExtents"), value.spawnHalfExtents)) ||
				!Read_Uint32(motion->Find("count"), value.count, 128u) ||
				!Read_Uint32(motion->Find("intervalMs"), value.intervalMs, MAX_DURATION_MS) ||
				!Read_FiniteFloat(motion->Find("spreadDegrees"), value.spreadDegrees) ||
				!Read_Uint32(motion->Find("seed"), value.seed))
			{
				outStatus = "World object motion is invalid";
				return false;
			}
			if (const DATA_JSON_VALUE* emissions = motion->Find("emissions"))
			{
				if (!emissions->Is_Array() || emissions->Get_Array().size() > 128u)
				{
					outStatus = "World object emissions are invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& emissionValue : emissions->Get_Array())
				{
					WORLD_SEQUENCE_OBJECT_EMISSION emission;
					if (!Is_ExactObject(emissionValue, { "positionOffset", "yawDegrees", "startDelayMs" }) ||
						!Read_Float3(emissionValue.Find("positionOffset"), emission.positionOffset) ||
						!Read_FiniteFloat(emissionValue.Find("yawDegrees"), emission.yawDegrees) ||
						!Read_Uint32(emissionValue.Find("startDelayMs"), emission.startDelayMs, MAX_DURATION_MS))
					{
						outStatus = "World object emission row is invalid";
						return false;
					}
					value.emissions.push_back(emission);
				}
			}
		}

		for (const DATA_JSON_VALUE& trackValue : tracks->Get_Array())
		{
			if (!Is_ExactObject(trackValue, { "slotId", "keys" }))
			{
				outStatus = "World sequence track shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
			const DATA_JSON_VALUE* keys = trackValue.Find("keys");
			WORLD_SEQUENCE_TRACK parsedTrack;
			if (nullptr == slotId || !slotId->Is_String() ||
				nullptr == keys || !keys->Is_Array() ||
				keys->Get_Array().size() > MAX_KEY_COUNT)
			{
				outStatus = "World sequence track fields are invalid";
				return false;
			}
			parsedTrack.slotId = slotId->Get_String();
			for (const DATA_JSON_VALUE& keyValue : keys->Get_Array())
			{
				if (!Is_ExactObject(keyValue,
					{ "timeMs", "positionOffset", "rotationQuaternion",
					  "scaleMultiplier", "visible" }))
				{
					outStatus = "World sequence key shape is invalid";
					return false;
				}
				WORLD_SEQUENCE_TRANSFORM_KEY parsedKey;
				const DATA_JSON_VALUE* visible = keyValue.Find("visible");
				if (!Read_Uint32(keyValue.Find("timeMs"), parsedKey.timeMs,
						MAX_DURATION_MS) ||
					!Read_Float3(keyValue.Find("positionOffset"),
						parsedKey.positionOffset) ||
					!Read_Quaternion(keyValue.Find("rotationQuaternion"),
						parsedKey.rotationQuaternion) ||
					!Read_Float3(keyValue.Find("scaleMultiplier"),
						parsedKey.scaleMultiplier) ||
					nullptr == visible || !visible->Is_Boolean())
				{
					outStatus = "World sequence key fields are invalid";
					return false;
				}
				parsedKey.visible = visible->Get_Boolean();
				parsedTrack.keys.push_back(parsedKey);
			}
			parsedTemplate.tracks.push_back(std::move(parsedTrack));
		}
		if (2u <= parsedFormatVersion)
		{
			for (const DATA_JSON_VALUE& trackValue :
				animationTracks->Get_Array())
			{
				if (!Is_ObjectShape(trackValue,
						{ "slotId", "clipName", "playbackRate", "loop",
						  "holdLastFrame" }, { "startMs", "displayName", "sourceStartMs", "sourceEndMs" }))
				{
					outStatus = "World sequence animation track shape is invalid";
					return false;
				}
				const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
				const DATA_JSON_VALUE* clipName = trackValue.Find("clipName");
				const DATA_JSON_VALUE* trackDisplayName = trackValue.Find("displayName");
				const DATA_JSON_VALUE* loop = trackValue.Find("loop");
				const DATA_JSON_VALUE* holdLastFrame =
					trackValue.Find("holdLastFrame");
				WORLD_SEQUENCE_ANIMATION_TRACK parsedTrack;
				if (nullptr == slotId || !slotId->Is_String() ||
					nullptr == clipName || !clipName->Is_String() ||
					(nullptr != trackDisplayName && !trackDisplayName->Is_String()) ||
					!Read_FiniteFloat(trackValue.Find("playbackRate"),
						parsedTrack.playbackRate) ||
					nullptr == loop || !loop->Is_Boolean() ||
					nullptr == holdLastFrame || !holdLastFrame->Is_Boolean())
				{
					outStatus = "World sequence animation track fields are invalid";
					return false;
				}
				const DATA_JSON_VALUE* startMs = trackValue.Find("startMs");
				if (nullptr != startMs &&
					!Read_Uint32(startMs, parsedTrack.startMs, MAX_DURATION_MS))
				{
					outStatus = "World sequence animation track start is invalid";
					return false;
				}
				if (const auto* sourceStart = trackValue.Find("sourceStartMs"))
				{
					if (!Read_Uint32(sourceStart, parsedTrack.sourceStartMs, MAX_DURATION_MS))
					{ outStatus = "World sequence animation source start is invalid"; return false; }
				}
				if (const auto* sourceEnd = trackValue.Find("sourceEndMs"))
				{
					if (!Read_Uint32(sourceEnd, parsedTrack.sourceEndMs, MAX_DURATION_MS))
					{ outStatus = "World sequence animation source end is invalid"; return false; }
				}
				parsedTrack.slotId = slotId->Get_String();
				parsedTrack.clipName = clipName->Get_String();
				if (nullptr != trackDisplayName)
					parsedTrack.displayName = trackDisplayName->Get_String();
				parsedTrack.loop = loop->Get_Boolean();
				parsedTrack.holdLastFrame = holdLastFrame->Get_Boolean();
				parsedTemplate.animationTracks.push_back(std::move(parsedTrack));
			}
		}
        if (const auto* materials = templateValue.Find("materialTracks"))
        {
            if (parsedFormatVersion < 3u || !materials->Is_Array() || materials->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World materialTracks must be a bounded v3 array"; return false; }
            for (const auto& row : materials->Get_Array())
            {
                WORLD_SEQUENCE_MATERIAL_TRACK track;
                if (!Is_ObjectShape(row, {"slotId", "materialName", "curves"}, {}) ||
                    !row.Find("slotId")->Is_String() || !row.Find("materialName")->Is_String() ||
                    !row.Find("curves")->Is_Array() || row.Find("curves")->Get_Array().size() > 64u)
                { outStatus = "World material track fields are invalid"; return false; }
                track.slotId = row.Find("slotId")->Get_String();
                track.materialName = row.Find("materialName")->Get_String();
                for (const auto& curve : row.Find("curves")->Get_Array())
                {
                    WORLD_SEQUENCE_MATERIAL_CURVE parsed;
                    if (!Is_ObjectShape(curve, {"parameter", "keys"}, {}) ||
                        !curve.Find("parameter")->Is_String() || !curve.Find("keys")->Is_Array() ||
                        curve.Find("keys")->Get_Array().size() > 4096u)
                    { outStatus = "World material curve fields are invalid"; return false; }
                    parsed.parameter = curve.Find("parameter")->Get_String();
                    for (const auto& key : curve.Find("keys")->Get_Array())
                    {
                        WORLD_SEQUENCE_MATERIAL_KEY sample;
                        if (!Is_ObjectShape(key, {"timeMs", "value", "interpolation"}, {}) ||
                            !Read_Uint32(key.Find("timeMs"), sample.timeMs, MAX_DURATION_MS) ||
                            !key.Find("interpolation")->Is_String() || !key.Find("value")->Is_Array() ||
                            key.Find("value")->Get_Array().size() != 4u)
                        { outStatus = "World material key fields are invalid"; return false; }
                        const auto& mode = key.Find("interpolation")->Get_String();
                        if (mode != "LINEAR" && mode != "CONSTANT")
                        { outStatus = "World material interpolation is invalid"; return false; }
                        sample.constant = mode == "CONSTANT";
                        for (size_t axis = 0u; axis < 4u; ++axis)
                            if (!Read_FiniteFloat(&key.Find("value")->Get_Array()[axis], sample.value[axis]))
                            { outStatus = "World material value is not finite"; return false; }
                        parsed.keys.push_back(sample);
                    }
                    track.curves.push_back(std::move(parsed));
                }
                parsedTemplate.materialTracks.push_back(std::move(track));
            }
        }
		if (const auto* effects = templateValue.Find("effectTracks"))
		{
			if (parsedFormatVersion < 3u || !effects->Is_Array() || effects->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object effectTracks must be a bounded v3 array"; return false; }
			for (const auto& row : effects->Get_Array())
			{
				WORLD_SEQUENCE_EFFECT_TRACK effect;
				if (!Is_ObjectShape(row, { "effectTrackId", "slotId", "resourceKind", "resourceId",
					"timing", "startMs", "durationMs", "positionOffset", "rotationDegrees", "scale" }, { "followObject", "inheritObjectRotation", "bone", "fitEffectToDuration", "loopEffectToDuration" }))
				{ outStatus = "World Object effect track shape is invalid"; return false; }
				for (const char* key : { "effectTrackId", "slotId", "resourceKind", "resourceId", "timing" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object effect identity must be text"; return false; }
				effect.effectTrackId = row.Find("effectTrackId")->Get_String();
				effect.slotId = row.Find("slotId")->Get_String();
				effect.resourceKind = row.Find("resourceKind")->Get_String();
				effect.resourceId = row.Find("resourceId")->Get_String();
				effect.timing = row.Find("timing")->Get_String();
				if (const auto* fit = row.Find("fitEffectToDuration"))
				{
					if (!fit->Is_Boolean()) { outStatus = "World Object effect fit must be boolean"; return false; }
					effect.fitEffectToDuration = fit->Get_Boolean();
				}
				if (const auto* loop = row.Find("loopEffectToDuration"))
				{
					if (!loop->Is_Boolean()) { outStatus = "World Object effect loop must be boolean"; return false; }
					effect.loopEffectToDuration = loop->Get_Boolean();
				}
				if (const auto* follow = row.Find("followObject"))
				{
					if (!follow->Is_Boolean()) { outStatus = "World Object effect followObject must be boolean"; return false; }
					effect.followObject = follow->Get_Boolean();
				}
				if (const auto* inherit = row.Find("inheritObjectRotation"))
				{
					if (!inherit->Is_Boolean()) { outStatus = "World Object effect inheritObjectRotation must be boolean"; return false; }
					effect.inheritObjectRotation = inherit->Get_Boolean();
				}
				if (const auto* bone = row.Find("bone"))
				{
					if (!bone->Is_String()) { outStatus = "World Object effect bone must be text"; return false; }
					effect.bone = bone->Get_String();
				}
				if (!Read_Uint32(row.Find("startMs"), effect.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), effect.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), effect.positionOffset) ||
					!Read_Float3(row.Find("rotationDegrees"), effect.rotationDegrees) ||
					!Read_Float3(row.Find("scale"), effect.scale))
				{ outStatus = "World Object effect timing or transform is invalid"; return false; }
				parsedTemplate.effectTracks.push_back(std::move(effect));
			}
		}

		if (const auto* colliders = templateValue.Find("colliderTracks"))
		{
			if (parsedFormatVersion < 3u || !colliders->Is_Array() || colliders->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object colliderTracks must be a bounded v3 array"; return false; }
			for (const auto& row : colliders->Get_Array())
			{
				WORLD_SEQUENCE_COLLIDER_TRACK collider;
				if (!Is_ObjectShape(row, { "colliderTrackId", "slotId", "startMs", "durationMs", "positionOffset",
					"halfExtents", "yawDegrees", "behavior", "damagePercent", "gripLocalOffset" }, { "attachmentBone", "shape" }))
				{ outStatus = "World Object collider track shape is invalid"; return false; }
				for (const char* key : { "colliderTrackId", "slotId", "behavior" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object collider identity must be text"; return false; }
				collider.colliderTrackId = row.Find("colliderTrackId")->Get_String();
				collider.slotId = row.Find("slotId")->Get_String();
				collider.behavior = row.Find("behavior")->Get_String();
				if (const auto* shape = row.Find("shape"))
				{
					if (!shape->Is_String()) { outStatus = "World Object collider shape must be text"; return false; }
					collider.shape = shape->Get_String();
				}
				if (const auto* bone = row.Find("attachmentBone"))
				{
					if (!bone->Is_String()) { outStatus = "World Object collider attachmentBone must be text"; return false; }
					collider.attachmentBone = bone->Get_String();
				}
				if (!Read_Uint32(row.Find("startMs"), collider.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), collider.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), collider.positionOffset) ||
					!Read_Float3(row.Find("halfExtents"), collider.halfExtents) ||
					!Read_Float3(row.Find("gripLocalOffset"), collider.gripLocalOffset) ||
					!row.Find("yawDegrees")->Is_Number() || !row.Find("damagePercent")->Is_Number())
				{ outStatus = "World Object collider timing or values are invalid"; return false; }
				collider.yawDegrees = static_cast<f32_t>(row.Find("yawDegrees")->Get_Number());
				collider.damagePercent = static_cast<f32_t>(row.Find("damagePercent")->Get_Number());
				parsedTemplate.colliderTracks.push_back(std::move(collider));
			}
		}

        if (const auto* sounds = templateValue.Find("soundTracks"))
        {
            if (parsedFormatVersion < 3u || !sounds->Is_Array() || sounds->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World soundTracks must be a bounded v3 array"; return false; }
            for (const auto& row : sounds->Get_Array())
            {
                WORLD_SEQUENCE_SOUND_TRACK sound;
                if (!Is_ObjectShape(row, { "soundTrackId", "assetId", "startMs", "durationMs", "volume" }, { "loopToDuration" }) ||
                    !row.Find("soundTrackId")->Is_String() || !row.Find("assetId")->Is_String() || !row.Find("volume")->Is_Number() ||
                    !Read_Uint32(row.Find("startMs"), sound.startMs, MAX_DURATION_MS) ||
                    !Read_Uint32(row.Find("durationMs"), sound.durationMs, MAX_DURATION_MS))
                { outStatus = "World sound track fields are invalid"; return false; }
                sound.soundTrackId = row.Find("soundTrackId")->Get_String();
                sound.assetId = row.Find("assetId")->Get_String();
                sound.volume = static_cast<f32_t>(row.Find("volume")->Get_Number());
                if (const auto* loop = row.Find("loopToDuration"))
                {
                    if (!loop->Is_Boolean()) { outStatus = "World sound loopToDuration must be boolean"; return false; }
                    sound.loopToDuration = loop->Get_Boolean();
                }
                parsedTemplate.soundTracks.push_back(std::move(sound));
            }
        }
        if (const auto* subtitles = templateValue.Find("subtitleTracks"))
        {
            if (parsedFormatVersion < 3u || !subtitles->Is_Array() || subtitles->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World subtitleTracks must be a bounded v3 array"; return false; }
            for (const auto& row : subtitles->Get_Array())
            {
                WORLD_SEQUENCE_SUBTITLE_TRACK subtitle;
                if (!Is_ExactObject(row, { "subtitleTrackId", "stringId", "text", "position", "slotId", "startMs", "durationMs" }))
                { outStatus = "World subtitle track shape is invalid"; return false; }
                for (const auto* key : { "subtitleTrackId", "stringId", "text", "position", "slotId" })
                    if (!row.Find(key)->Is_String())
                    { outStatus = "World subtitle identity and text must be strings"; return false; }
                if (!Read_Uint32(row.Find("startMs"), subtitle.startMs, MAX_DURATION_MS) ||
                    !Read_Uint32(row.Find("durationMs"), subtitle.durationMs, MAX_DURATION_MS))
                { outStatus = "World subtitle timing is invalid"; return false; }
                subtitle.subtitleTrackId = row.Find("subtitleTrackId")->Get_String();
                subtitle.stringId = row.Find("stringId")->Get_String();
                subtitle.text = row.Find("text")->Get_String();
                subtitle.position = row.Find("position")->Get_String();
                subtitle.slotId = row.Find("slotId")->Get_String();
                parsedTemplate.subtitleTracks.push_back(std::move(subtitle));
            }
        }

		staged.m_Templates.push_back(std::move(parsedTemplate));
	}

	for (const DATA_JSON_VALUE& instanceValue : instances->Get_Array())
	{
		if (!Is_ObjectShape(instanceValue,
			{ "instanceId", "templateId", "enabled", "startDelayMs",
			  "playbackSpeed", "bindings" }, { "anchorKind", "position", "motionEnd", "nextMotionId", "walkableSurface", "loopFullPresentation" }))
		{
			outStatus = "World sequence instance shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* instanceId = instanceValue.Find("instanceId");
		const DATA_JSON_VALUE* templateId = instanceValue.Find("templateId");
		const DATA_JSON_VALUE* enabled = instanceValue.Find("enabled");
		const DATA_JSON_VALUE* bindings = instanceValue.Find("bindings");
		WORLD_SEQUENCE_INSTANCE parsedInstance;
		if (nullptr == instanceId || !instanceId->Is_String() ||
			nullptr == templateId || !templateId->Is_String() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!Read_Uint32(instanceValue.Find("startDelayMs"),
				parsedInstance.startDelayMs, MAX_DURATION_MS) ||
			!Read_FiniteFloat(instanceValue.Find("playbackSpeed"),
				parsedInstance.playbackSpeed) ||
			nullptr == bindings || !bindings->Is_Array() ||
			bindings->Get_Array().size() > MAX_TRACK_COUNT)
		{
			outStatus = "World sequence instance fields are invalid";
			return false;
		}
		parsedInstance.instanceId = instanceId->Get_String();
		parsedInstance.templateId = templateId->Get_String();
		parsedInstance.enabled = enabled->Get_Boolean();
		if (const auto* loop = instanceValue.Find("loopFullPresentation"))
		{
			if (parsedFormatVersion < 3u || !loop->Is_Boolean())
			{ outStatus = "World Object full presentation loop must be a v3 boolean"; return false; }
			parsedInstance.loopFullPresentation = loop->Get_Boolean();
		}
		const DATA_JSON_VALUE* anchor = instanceValue.Find("anchorKind");
		const DATA_JSON_VALUE* position = instanceValue.Find("position");
		if ((parsedFormatVersion < 3u && (anchor || position)) ||
			(anchor && !anchor->Is_String()) ||
			(position && !Read_Float3(position, parsedInstance.position)))
		{ outStatus = "World object instance anchor is invalid"; return false; }
		if (anchor) parsedInstance.anchorKind = anchor->Get_String();
		const DATA_JSON_VALUE* motionEnd = instanceValue.Find("motionEnd");
		const DATA_JSON_VALUE* nextMotionId = instanceValue.Find("nextMotionId");
		if ((parsedFormatVersion < 3u && (motionEnd || nextMotionId)) ||
			(motionEnd && (!motionEnd->Is_String() ||
				!Try_ParseMotionEnd(motionEnd->Get_String(), parsedInstance.motionEnd))) ||
			(nextMotionId && !nextMotionId->Is_String()))
		{ outStatus = "World object motion completion is invalid"; return false; }
		if (nextMotionId) parsedInstance.nextMotionId = nextMotionId->Get_String();
		if (const DATA_JSON_VALUE* surface = instanceValue.Find("walkableSurface"))
		{
			WORLD_SEQUENCE_WALKABLE_SURFACE parsed;
			if (parsedFormatVersion < 3u || !Is_ExactObject(*surface, { "radiusM", "localHeightM" }) ||
				!Read_FiniteFloat(surface->Find("radiusM"), parsed.radiusM) ||
				!Read_FiniteFloat(surface->Find("localHeightM"), parsed.localHeightM))
			{ outStatus = "Invalid walkable surface fields: " + parsedInstance.instanceId; return false; }
			parsedInstance.walkableSurface = parsed;
		}

		for (const DATA_JSON_VALUE& bindingValue : bindings->Get_Array())
		{
			const bool_t validBindingShape =
				LEGACY_FORMAT_VERSION == parsedFormatVersion ?
				Is_ExactObject(bindingValue, { "slotId", "placementId" }) :
				Is_ObjectShape(bindingValue,
					{ "slotId", "targetKind", "targetId" }, { "previewNpcPlacementId" });
			if (!validBindingShape)
			{
				outStatus = "World sequence binding shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = bindingValue.Find("slotId");
			WORLD_SEQUENCE_BINDING parsedBinding;
			if (nullptr == slotId || !slotId->Is_String())
			{
				outStatus = "World sequence binding fields are invalid";
				return false;
			}
			parsedBinding.slotId = slotId->Get_String();
			if (LEGACY_FORMAT_VERSION == parsedFormatVersion)
			{
				uint64_t placementId = 0;
				if (!Parse_Uint64String(bindingValue.Find("placementId"),
					placementId))
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetKind =
					WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
				parsedBinding.targetId = std::to_string(placementId);
			}
			else
			{
				const DATA_JSON_VALUE* targetKind =
					bindingValue.Find("targetKind");
				const DATA_JSON_VALUE* targetId = bindingValue.Find("targetId");
				if (nullptr == targetKind || !targetKind->Is_String() ||
					!Try_ParseTargetKind(targetKind->Get_String(),
						parsedBinding.targetKind) ||
					nullptr == targetId || !targetId->Is_String())
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetId = targetId->Get_String();
				if (parsedFormatVersion < 3u && parsedBinding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object resource binding requires formatVersion 3"; return false; }
			}
			if (const auto* placement = bindingValue.Find("previewNpcPlacementId"))
			{
				if (!placement->Is_String() || !Is_ValidStableId(placement->Get_String()))
				{ outStatus = "Invalid NPC preview placement"; return false; }
				parsedBinding.previewNpcPlacementId = placement->Get_String();
			}
			parsedInstance.bindings.push_back(std::move(parsedBinding));
		}
		staged.m_Instances.push_back(std::move(parsedInstance));
	}

	if (!staged.Validate(availablePlacements, availableDeployPlacements,
		outStatus))
		return false;
	*this = std::move(staged);
	outStatus = "Loaded world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Save(
	const std::filesystem::path& path,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	if (!Validate(availablePlacements, availableDeployPlacements, outStatus))
		return false;
	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create world sequence authoring directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create world sequence temporary file";
		return false;
	}
	output << std::setprecision(9)
		<< "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(m_AreaId) << "\",\n"
		<< "  \"revision\": " << m_iRevision << ",\n";
    if (!m_ObjectFolders.empty())
    {
        output << "  \"objectFolders\": [";
        for (size_t index = 0; index < m_ObjectFolders.size(); ++index)
        {
            const auto& folder = m_ObjectFolders[index];
            output << (index ? ",\n" : "\n") << "    {\n"
                << "      \"folderId\": \"" << CDataJson::Escape(folder.folderId) << "\",\n"
                << "      \"displayName\": \"" << CDataJson::Escape(folder.displayName) << "\",\n"
                << "      \"anchorKind\": \"" << CDataJson::Escape(folder.anchorKind) << "\"";
            if (!folder.parentId.empty())
                output << ",\n      \"parentId\": \"" << CDataJson::Escape(folder.parentId) << "\"";
            output << "\n    }";
        }
        output << "\n  ],\n";
    }
    output << "  \"objectResources\": [";
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& object = m_ObjectResources[index];
		output << (index == 0u ? "\n" : ",\n") << "    {\n"
			<< "      \"objectId\": \"" << CDataJson::Escape(object.objectId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(object.displayName) << "\",\n"
			<< "      \"modelAssetId\": \"" << CDataJson::Escape(object.modelAssetId) << "\",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(object.anchorKind) << "\",\n"
			<< "      \"diffuseTextureAssetId\": \"" << CDataJson::Escape(object.diffuseTextureAssetId) << "\",\n"
			<< "      \"modelPreScale\": " << object.modelPreScale << ",\n"
			<< "      \"animated\": " << (object.animated ? "true" : "false") << ",\n"
			<< "      \"scale\": [" << object.scale.x << ", " << object.scale.y << ", " << object.scale.z << "],\n"
			<< "      \"sequenceInstanceId\": \"" << CDataJson::Escape(object.sequenceInstanceId) << "\"";
        if (!object.parentId.empty())
            output << ",\n      \"parentId\": \"" << CDataJson::Escape(object.parentId) << "\"";
		if (object.anchorKind == "BOSS")
			output << ",\n      \"anchorBossArchetypeId\": \"" << CDataJson::Escape(object.anchorBossArchetypeId)
				<< "\",\n      \"anchorBone\": \"" << CDataJson::Escape(object.anchorBone) << "\"";
		if (!object.defaultMotionInstanceId.empty())
			output << ",\n      \"defaultMotionInstanceId\": \"" << CDataJson::Escape(object.defaultMotionInstanceId) << "\"";
		if (!object.motionInstanceIds.empty())
		{
			output << ",\n      \"motionInstanceIds\": [";
			for (size_t i = 0; i < object.motionInstanceIds.size(); ++i)
				output << (i ? ", " : "") << "\"" << CDataJson::Escape(object.motionInstanceIds[i]) << "\"";
			output << "]";
		}
        if (object.combatBody)
        {
            const auto& body = *object.combatBody;
            output << ",\n      \"combatBody\": { \"maxHp\": " << body.maxHp
                << ", \"localCenterM\": [" << body.localCenterM.x << ", " << body.localCenterM.y << ", " << body.localCenterM.z
                << "], \"halfExtentsM\": [" << body.halfExtentsM.x << ", " << body.halfExtentsM.y << ", " << body.halfExtentsM.z
                << "], \"lifetimePolicy\": \"UNTIL_DESTROYED\"";
            if (body.shape != "BOX") output << ", \"shape\": \"" << CDataJson::Escape(body.shape) << "\"";
            output << " }";
        }
        if (!object.materialSourceModelAssetId.empty())
            output << ",\n      \"materialSourceModelAssetId\": \"" << CDataJson::Escape(object.materialSourceModelAssetId) << "\"";
        if (!object.animationSetAssetId.empty())
            output << ",\n      \"animationSetAssetId\": \"" << CDataJson::Escape(object.animationSetAssetId) << "\"";
        if (!object.presentationBossArchetypeId.empty())
            output << ",\n      \"presentationBossArchetypeId\": \"" << CDataJson::Escape(object.presentationBossArchetypeId) << "\"";
        if (!object.mapMaterialBindings.empty())
        {
            output << ",\n      \"mapMaterialBindings\": [";
            for (size_t i = 0; i < object.mapMaterialBindings.size(); ++i)
            {
                const auto& binding = object.mapMaterialBindings[i];
                output << (i ? "," : "") << "\n        {\"materialName\": \"" << CDataJson::Escape(binding.materialName)
                    << "\", \"sourceAssetId\": \"" << CDataJson::Escape(binding.sourceAssetId)
                    << "\", \"sourceMaterialName\": \"" << CDataJson::Escape(binding.sourceMaterialName) << "\"";
                if (!binding.diffuseTextureAssetId.empty()) output << ", \"diffuseTextureAssetId\": \"" << CDataJson::Escape(binding.diffuseTextureAssetId) << "\"";
                if (binding.unlit) output << ", \"unlit\": true";
                output << "}";
            }
            output << "\n      ]";
        }
        if (object.materialProfile)
        {
            const auto& profile = *object.materialProfile;
            output << ",\n      \"materialProfile\": {\n        \"materialName\": \"" << CDataJson::Escape(profile.materialName)
                << "\",\n        \"sourceMaterial\": \"" << CDataJson::Escape(profile.sourceMaterial)
                << "\",\n        \"family\": \"" << CDataJson::Escape(profile.family) << "\",\n        \"parameters\": {";
            bool first = true;
            for (const auto& [name, value] : profile.parameters)
            {
                output << (first ? "" : ",") << "\n          \"" << CDataJson::Escape(name) << "\": ["
                    << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << "]";
                first = false;
            }
            output << "\n        },\n        \"textures\": [";
            for (size_t i = 0u; i < profile.textures.size(); ++i)
            {
                const auto& texture = profile.textures[i];
                output << (i ? "," : "") << "\n          {\"expressionIndex\": " << texture.expressionIndex
                    << ", \"assetId\": \"" << CDataJson::Escape(texture.assetId)
                    << "\", \"colorSpace\": \"" << (texture.srgb ? "srgb" : "linear") << "\"}";
            }
            output << "\n        ]\n      }";
        }
		output << "\n    }";
	}
	output << (m_ObjectResources.empty() ? "],\n" : "\n  ],\n")
		<< "  \"templates\": [";
	for (size_t templateIndex = 0; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& value = m_Templates[templateIndex];
		output << (0u == templateIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"sequenceId\": \"" << CDataJson::Escape(value.sequenceId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(value.displayName) << "\",\n"
			<< "      \"category\": \"" << CDataJson::Escape(value.category) << "\",\n"
			<< "      \"durationMs\": " << value.durationMs << ",\n"
			<< "      \"interpolation\": \"" << Interpolation_ToString(value.interpolation) << "\",\n"
			<< "      \"objectMotion\": {\n";
		const auto& motion = value.objectMotion;
		const auto writeVector = [&output](const char_t* name, const float3_t& vector)
		{
			output << "        \"" << name << "\": [" << vector.x << ", " << vector.y << ", " << vector.z << "],\n";
		};
		writeVector("velocity", motion.velocity);
		writeVector("acceleration", motion.acceleration);
		writeVector("angularVelocityDegrees", motion.angularVelocityDegrees);
		writeVector("revolutionDegreesPerSecond", motion.revolutionDegreesPerSecond);
		writeVector("revolutionOffset", motion.revolutionOffset);
		if (motion.spawnHalfExtents.x != 0.f || motion.spawnHalfExtents.y != 0.f || motion.spawnHalfExtents.z != 0.f)
			writeVector("spawnHalfExtents", motion.spawnHalfExtents);
		output << "        \"count\": " << motion.count << ", \"intervalMs\": " << motion.intervalMs
			<< ", \"spreadDegrees\": " << motion.spreadDegrees << ", \"seed\": " << motion.seed;
		if (!motion.emissions.empty())
		{
			output << ",\n        \"emissions\": [";
			for (size_t emissionIndex = 0; emissionIndex < motion.emissions.size(); ++emissionIndex)
			{
				const auto& emission = motion.emissions[emissionIndex];
				output << (0u == emissionIndex ? "\n" : ",\n")
					<< "          {\"positionOffset\": [" << emission.positionOffset.x << ", " << emission.positionOffset.y
					<< ", " << emission.positionOffset.z << "], \"yawDegrees\": " << emission.yawDegrees
					<< ", \"startDelayMs\": " << emission.startDelayMs << "}";
			}
			output << "\n        ]";
		}
		output << "\n      },\n"
			<< "      \"tracks\": [";
		for (size_t trackIndex = 0; trackIndex < value.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& track = value.tracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        {\n"
				<< "          \"slotId\": \"" << CDataJson::Escape(track.slotId) << "\",\n"
				<< "          \"keys\": [";
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				output << (0u == keyIndex ? "\n" : ",\n")
					<< "            {\n"
					<< "              \"timeMs\": " << key.timeMs << ",\n"
					<< "              \"positionOffset\": [" << key.positionOffset.x << ", "
					<< key.positionOffset.y << ", " << key.positionOffset.z << "],\n"
					<< "              \"rotationQuaternion\": [" << key.rotationQuaternion.x << ", "
					<< key.rotationQuaternion.y << ", " << key.rotationQuaternion.z << ", "
					<< key.rotationQuaternion.w << "],\n"
					<< "              \"scaleMultiplier\": [" << key.scaleMultiplier.x << ", "
					<< key.scaleMultiplier.y << ", " << key.scaleMultiplier.z << "],\n"
					<< "              \"visible\": " << (key.visible ? "true" : "false") << "\n"
					<< "            }";
			}
			output << (track.keys.empty() ? "]\n" : "\n          ]\n")
				<< "        }";
		}
		output << (value.tracks.empty() ? "],\n" : "\n      ],\n")
			<< "      \"animationTracks\": [";
		for (size_t trackIndex = 0;
			trackIndex < value.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& track =
				value.animationTracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \""
				<< CDataJson::Escape(track.slotId)
				<< "\", \"clipName\": \""
				<< CDataJson::Escape(track.clipName)
				<< "\"";
			if (!track.displayName.empty())
				output << ", \"displayName\": \"" << CDataJson::Escape(track.displayName) << "\"";
			if (track.sourceStartMs != 0u)
				output << ", \"sourceStartMs\": " << track.sourceStartMs;
			if (track.sourceEndMs != 0u)
				output << ", \"sourceEndMs\": " << track.sourceEndMs;
			output << ", \"startMs\": " << track.startMs
					<< ", \"playbackRate\": " << track.playbackRate
				<< ", \"loop\": " << (track.loop ? "true" : "false")
				<< ", \"holdLastFrame\": "
				<< (track.holdLastFrame ? "true" : "false") << " }";
		}
		output << (value.animationTracks.empty() ? "]" : "\n      ]");
        if (!value.materialTracks.empty())
        {
            output << ",\n      \"materialTracks\": [";
            for (size_t index = 0u; index < value.materialTracks.size(); ++index)
            {
                const auto& track = value.materialTracks[index];
                output << (index ? "," : "") << "{\"slotId\":\"" << CDataJson::Escape(track.slotId)
                    << "\",\"materialName\":\"" << CDataJson::Escape(track.materialName) << "\",\"curves\":[";
                for (size_t curveIndex = 0u; curveIndex < track.curves.size(); ++curveIndex)
                {
                    const auto& curve = track.curves[curveIndex];
                    output << (curveIndex ? "," : "") << "{\"parameter\":\"" << CDataJson::Escape(curve.parameter) << "\",\"keys\":[";
                    for (size_t keyIndex = 0u; keyIndex < curve.keys.size(); ++keyIndex)
                    {
                        const auto& key = curve.keys[keyIndex];
                        output << (keyIndex ? "," : "") << "{\"timeMs\":" << key.timeMs << ",\"value\":[";
                        for (size_t axis = 0u; axis < 4u; ++axis) output << (axis ? "," : "") << key.value[axis];
                        output << "],\"interpolation\":\"" << (key.constant ? "CONSTANT" : "LINEAR") << "\"}";
                    }
                    output << "]}";
                }
                output << "]}";
            }
            output << "]";
        }
		if (!value.effectTracks.empty())
		{
			output << ",\n      \"effectTracks\": [";
			for (size_t index = 0; index < value.effectTracks.size(); ++index)
			{
				const auto& effect = value.effectTracks[index];
				output << (index ? ",\n" : "\n") << "        { \"effectTrackId\": \"" << CDataJson::Escape(effect.effectTrackId)
					<< "\", \"slotId\": \"" << CDataJson::Escape(effect.slotId)
					<< "\", \"resourceKind\": \"" << effect.resourceKind
					<< "\", \"resourceId\": \"" << CDataJson::Escape(effect.resourceId)
					<< "\", \"timing\": \"" << effect.timing
					<< "\", \"followObject\": " << (effect.followObject ? "true" : "false")
					<< ", \"bone\": \"" << CDataJson::Escape(effect.bone)
					<< "\", \"startMs\": " << effect.startMs << ", \"durationMs\": " << effect.durationMs
					<< ", \"positionOffset\": [" << effect.positionOffset.x << ", " << effect.positionOffset.y << ", " << effect.positionOffset.z
					<< "], \"rotationDegrees\": [" << effect.rotationDegrees.x << ", " << effect.rotationDegrees.y << ", " << effect.rotationDegrees.z
					<< "], \"scale\": [" << effect.scale.x << ", " << effect.scale.y << ", " << effect.scale.z << "]";
				if (!effect.inheritObjectRotation) output << ", \"inheritObjectRotation\": false";
				if (effect.fitEffectToDuration) output << ", \"fitEffectToDuration\": true";
				if (effect.loopEffectToDuration) output << ", \"loopEffectToDuration\": true";
				output << " }";
			}
			output << "\n      ]";
		}
		if (!value.colliderTracks.empty())
		{
			output << ",\n      \"colliderTracks\": [";
			for (size_t index = 0; index < value.colliderTracks.size(); ++index)
			{
				const auto& collider = value.colliderTracks[index];
				output << (index ? ",\n" : "\n") << "        { \"colliderTrackId\": \"" << CDataJson::Escape(collider.colliderTrackId)
					<< "\", \"slotId\": \"" << CDataJson::Escape(collider.slotId)
					<< "\", \"startMs\": " << collider.startMs << ", \"durationMs\": " << collider.durationMs
					<< ", \"positionOffset\": [" << collider.positionOffset.x << ", " << collider.positionOffset.y << ", " << collider.positionOffset.z
					<< "], \"halfExtents\": [" << collider.halfExtents.x << ", " << collider.halfExtents.y << ", " << collider.halfExtents.z
					<< "], \"yawDegrees\": " << collider.yawDegrees << ", \"behavior\": \"" << collider.behavior
					<< "\", \"damagePercent\": " << collider.damagePercent << ", \"gripLocalOffset\": ["
					<< collider.gripLocalOffset.x << ", " << collider.gripLocalOffset.y << ", " << collider.gripLocalOffset.z << "]";
				if (collider.shape != "BOX") output << ", \"shape\": \"" << CDataJson::Escape(collider.shape) << "\"";
				if (!collider.attachmentBone.empty()) output << ", \"attachmentBone\": \"" << CDataJson::Escape(collider.attachmentBone) << "\"";
				output << " }";
			}
			output << "\n      ]";
		}
        if (!value.soundTracks.empty())
        {
            output << ",\n      \"soundTracks\": [";
            for (size_t index = 0; index < value.soundTracks.size(); ++index)
            {
                const auto& sound = value.soundTracks[index];
                output << (index ? ",\n" : "\n") << "        { \"soundTrackId\": \"" << CDataJson::Escape(sound.soundTrackId)
                    << "\", \"assetId\": \"" << CDataJson::Escape(sound.assetId)
                    << "\", \"startMs\": " << sound.startMs << ", \"durationMs\": " << sound.durationMs
                    << ", \"volume\": " << sound.volume;
                if (sound.loopToDuration) output << ", \"loopToDuration\": true";
                output << " }";
            }
            output << "\n      ]";
        }
        if (!value.subtitleTracks.empty())
        {
            output << ",\n      \"subtitleTracks\": [";
            for (size_t index = 0; index < value.subtitleTracks.size(); ++index)
            {
                const auto& subtitle = value.subtitleTracks[index];
                output << (index ? ",\n" : "\n") << "        { \"subtitleTrackId\": \"" << CDataJson::Escape(subtitle.subtitleTrackId)
                    << "\", \"stringId\": \"" << CDataJson::Escape(subtitle.stringId)
                    << "\", \"text\": \"" << CDataJson::Escape(subtitle.text)
                    << "\", \"position\": \"" << subtitle.position << "\", \"slotId\": \"" << CDataJson::Escape(subtitle.slotId)
                    << "\", \"startMs\": " << subtitle.startMs << ", \"durationMs\": " << subtitle.durationMs << " }";
            }
            output << "\n      ]";
        }
		output << "\n    }";
	}
	output << (m_Templates.empty() ? "],\n" : "\n  ],\n")
		<< "  \"instances\": [";
	for (size_t instanceIndex = 0; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& value = m_Instances[instanceIndex];
		output << (0u == instanceIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"instanceId\": \"" << CDataJson::Escape(value.instanceId) << "\",\n"
			<< "      \"templateId\": \"" << CDataJson::Escape(value.templateId) << "\",\n"
			<< "      \"enabled\": " << (value.enabled ? "true" : "false") << ",\n"
			<< "      \"startDelayMs\": " << value.startDelayMs << ",\n"
			<< "      \"playbackSpeed\": " << value.playbackSpeed << ",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(value.anchorKind) << "\",\n"
			<< "      \"position\": [" << value.position.x << ", " << value.position.y << ", " << value.position.z << "],\n"
			<< "      \"motionEnd\": \"" << MotionEnd_ToString(value.motionEnd) << "\",\n"
			<< "      \"nextMotionId\": \"" << CDataJson::Escape(value.nextMotionId) << "\",\n"
			;
		if (value.loopFullPresentation) output << "      \"loopFullPresentation\": true,\n";
		if (value.walkableSurface)
			output << "      \"walkableSurface\": { \"radiusM\": " << value.walkableSurface->radiusM
				<< ", \"localHeightM\": " << value.walkableSurface->localHeightM << " },\n";
		output << "      \"bindings\": [";
		for (size_t bindingIndex = 0; bindingIndex < value.bindings.size();
			++bindingIndex)
		{
			const WORLD_SEQUENCE_BINDING& binding = value.bindings[bindingIndex];
			output << (0u == bindingIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \"" << CDataJson::Escape(binding.slotId)
				<< "\", \"targetKind\": \""
				<< TargetKind_ToString(binding.targetKind)
				<< "\", \"targetId\": \""
				<< CDataJson::Escape(binding.targetId) << "\"";
			if (!binding.previewNpcPlacementId.empty())
				output << ", \"previewNpcPlacementId\": \"" << CDataJson::Escape(binding.previewNpcPlacementId) << "\"";
			output << " }";
		}
		output << (value.bindings.empty() ? "]\n" : "\n      ]\n")
			<< "    }";
	}
	output << (m_Instances.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	bool_t writeSucceeded = output.good();
	output.close();
	writeSucceeded = writeSucceeded && !output.fail();
	if (!writeSucceeded || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit world sequence document atomically";
		return false;
	}
	outStatus = "Saved world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Validate_ObjectHierarchy(std::string& outStatus) const
{
    if (m_ObjectFolders.size() > MAX_INSTANCE_COUNT || m_ObjectResources.size() > MAX_INSTANCE_COUNT)
    { outStatus = "World object hierarchy exceeds its limits"; return false; }
    struct NODE { const std::string* parent; const std::string* anchor; };
    std::unordered_map<std::string, NODE> nodes;
    const auto add = [&](const std::string& id, const std::string& name,
        const std::string& anchor, const std::string& parent) {
        return Is_ValidStableId(id) && !name.empty() && name.size() <= 128u &&
            Is_ValidUtf8DisplayText(name) &&
            (anchor == "WORLD" || anchor == "PLAYER" || anchor == "BOSS") &&
            (parent.empty() || Is_ValidStableId(parent)) && nodes.emplace(id, NODE{&parent, &anchor}).second;
    };
    for (const auto& folder : m_ObjectFolders)
        if (!add(folder.folderId, folder.displayName, folder.anchorKind, folder.parentId))
        { outStatus = "Invalid or duplicate Object folder: " + folder.folderId; return false; }
    for (const auto& object : m_ObjectResources)
        if (!add(object.objectId, object.displayName, object.anchorKind, object.parentId))
        { outStatus = "Invalid or duplicate Object hierarchy entry: " + object.objectId; return false; }
    for (const auto& [id, node] : nodes)
    {
        std::unordered_set<std::string> visited{id};
        const NODE* current = &node;
        size_t depth = 0;
        while (!current->parent->empty())
        {
            const auto found = nodes.find(*current->parent);
            if (found == nodes.end() || *found->second.anchor != *node.anchor)
            { outStatus = "Object parent must exist in the same anchor category: " + id; return false; }
            if (!visited.insert(found->first).second || ++depth > 64u)
            { outStatus = "Object hierarchy contains a cycle or exceeds 64 parents: " + id; return false; }
            current = &found->second;
        }
    }
    return true;
}

bool_t Client::CWorldSequenceDocument::Build_PlaybackSubset(
    const std::vector<std::string>& roots, CWorldSequenceDocument& out, std::string& status) const
{
    CWorldSequenceDocument staged;
    staged.m_AreaId = m_AreaId;
    staged.m_iRevision = m_iRevision;
    std::unordered_set<std::string> instances, objects, templates;
    std::vector<std::string> pending = roots;
    if (pending.empty()) { status = "World playback selection is empty."; return false; }
    for (size_t index = 0u; index < pending.size(); ++index)
    {
        const std::string id = pending[index];
        if (const auto* object = Find_ObjectResource(id))
        {
            if (!objects.insert(id).second) continue;
            staged.m_ObjectResources.push_back(*object);
            // Folder ancestry is authoring organization, not a playback dependency.
            staged.m_ObjectResources.back().parentId.clear();
            if (!object->sequenceInstanceId.empty()) pending.push_back(object->sequenceInstanceId);
            if (!object->defaultMotionInstanceId.empty()) pending.push_back(object->defaultMotionInstanceId);
            pending.insert(pending.end(), object->motionInstanceIds.begin(), object->motionInstanceIds.end());
            if (!object->modelAssetId.empty())
                for (const auto& motion : m_Instances)
                    if (std::any_of(motion.bindings.begin(), motion.bindings.end(), [&](const auto& binding) {
                        return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == id;
                    })) pending.push_back(motion.instanceId);
            continue;
        }
        if (!instances.insert(id).second) continue;
        const auto* instance = Find_Instance(id);
        const auto* sequence = instance ? Find_Template(instance->templateId) : nullptr;
        if (!instance || !sequence)
        { status = "World playback dependency is unavailable: " + id; return false; }
        staged.m_Instances.push_back(*instance);
        if (templates.insert(sequence->sequenceId).second) staged.m_Templates.push_back(*sequence);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
        for (const auto& binding : instance->bindings)
            if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) pending.push_back(binding.targetId);
        if (instances.size() > MAX_INSTANCE_COUNT || objects.size() > MAX_INSTANCE_COUNT || templates.size() > MAX_TEMPLATE_COUNT)
        { status = "World playback dependency closure exceeds document capacity."; return false; }
    }
    out = std::move(staged);
    status.clear();
    return true;
}

bool_t Client::CWorldSequenceDocument::Validate(
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Validate");
	if (m_AreaId.empty() || m_AreaId.size() > 128u || 0u == m_iRevision ||
		m_Templates.size() > MAX_TEMPLATE_COUNT ||
		m_Instances.size() > MAX_INSTANCE_COUNT || m_ObjectResources.size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence document header is invalid";
		return false;
	}
	if (!Validate_ObjectHierarchy(outStatus)) return false;
	std::unordered_set<std::string> objectIds;
	for (const WORLD_SEQUENCE_OBJECT_RESOURCE& object : m_ObjectResources)
	{
		if (!object.motionInstanceIds.empty())
		{
			if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
				object.displayName.empty() || object.displayName.size() > 128u || !Is_ValidUtf8DisplayText(object.displayName) ||
				object.motionInstanceIds.size() > 32u || object.anchorKind != "WORLD" ||
				!object.modelAssetId.empty() || !object.sequenceInstanceId.empty() || !object.defaultMotionInstanceId.empty() ||
				object.animated || !object.diffuseTextureAssetId.empty() || object.materialProfile || object.combatBody ||
				!object.materialSourceModelAssetId.empty() || !object.mapMaterialBindings.empty() ||
				!object.animationSetAssetId.empty() ||
				!object.presentationBossArchetypeId.empty() ||
				!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty() ||
				!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE || object.modelPreScale > MAX_COMPONENT ||
				!Is_BoundedFloat3(object.scale) || object.scale.x != 1.f || object.scale.y != 1.f || object.scale.z != 1.f)
			{ outStatus = "Invalid model-less Object group: " + object.objectId; return false; }
			std::unordered_set<std::string> members;
			for (const auto& id : object.motionInstanceIds)
			{
				const auto* instance = Find_Instance(id);
				if (!Is_ValidStableId(id) || !members.insert(id).second || !instance || instance->anchorKind != "WORLD" ||
					(instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && instance->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP) || instance->bindings.size() != 1u ||
					instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object group needs unique existing Map Object motions ending with Stop or Loop: " + id; return false; }
				const auto* model = Find_ObjectResource(instance->bindings.front().targetId);
				if (!model || model->modelAssetId.empty() || !model->motionInstanceIds.empty())
				{ outStatus = "Object group member must bind a model, not another group: " + id; return false; }
			}
			continue;
		}
		const bool_t alias = !object.sequenceInstanceId.empty();
		if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
			object.displayName.empty() || object.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(object.displayName) ||
			(object.anchorKind != "WORLD" && object.anchorKind != "PLAYER" && object.anchorKind != "BOSS") ||
			(object.anchorKind == "BOSS" ? !Is_ValidStableId(object.anchorBossArchetypeId) :
				(!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty())) ||
			object.anchorBone.size() > 128u || !Is_ValidUtf8DisplayText(object.anchorBone) ||
			(alias && object.anchorKind != "WORLD") ||
			!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE ||
			object.modelPreScale > MAX_COMPONENT || !Is_BoundedFloat3(object.scale) ||
			object.scale.x < MIN_SCALE || object.scale.y < MIN_SCALE || object.scale.z < MIN_SCALE ||
			(alias ? (!object.modelAssetId.empty() || !Is_ValidStableId(object.sequenceInstanceId) ||
				nullptr == Find_Instance(object.sequenceInstanceId) || !object.diffuseTextureAssetId.empty() || object.animated) :
				(!Is_ResourcePath(object.modelAssetId, true) ||
					(!object.diffuseTextureAssetId.empty() && !Is_ResourcePath(object.diffuseTextureAssetId, false)))))
		{
			outStatus = "Invalid or duplicate world object resource: " + object.objectId;
			return false;
		}
        if (object.combatBody)
        {
            const auto& body = *object.combatBody;
            if (alias || object.anchorKind != "WORLD" || body.maxHp == 0u || body.maxHp > 1000000000u ||
                (body.shape != "BOX" && body.shape != "ELLIPSOID") ||
                body.lifetimePolicy != "UNTIL_DESTROYED" || !Is_BoundedFloat3(body.localCenterM) ||
                !Is_BoundedFloat3(body.halfExtentsM) || body.halfExtentsM.x < .001f || body.halfExtentsM.y < .001f ||
                body.halfExtentsM.z < .001f || body.halfExtentsM.x > 1000.f || body.halfExtentsM.y > 1000.f || body.halfExtentsM.z > 1000.f)
            { outStatus = "Combat body requires a WORLD model with bounded HP and local bounds: " + object.objectId; return false; }
        }
        if ((!object.materialSourceModelAssetId.empty() && (alias || !Is_ResourcePath(object.materialSourceModelAssetId, true))) ||
            object.mapMaterialBindings.size() > 64u || (alias && !object.mapMaterialBindings.empty()))
        { outStatus = "Invalid world object material source: " + object.objectId; return false; }
        /* A clip donor only makes sense for a skinned body that plays clips. */
        if (!object.animationSetAssetId.empty() &&
            (alias || !object.animated || !Is_ResourcePath(object.animationSetAssetId, true)))
        { outStatus = "Invalid world object animation set: " + object.objectId; return false; }
        /* The product boss assembly borrows this skinned body's bone palette. */
        if (!object.presentationBossArchetypeId.empty() &&
            (alias || !object.animated || !Is_ValidStableId(object.presentationBossArchetypeId)))
        { outStatus = "Invalid world object presentation boss: " + object.objectId; return false; }
        std::unordered_set<std::string> materialNames;
        if (object.materialProfile) materialNames.insert(object.materialProfile->materialName);
        for (const auto& binding : object.mapMaterialBindings)
            if (binding.materialName.empty() || binding.materialName.size() > 63u || !Is_ValidUtf8DisplayText(binding.materialName) ||
                !Is_ValidStableId(binding.sourceAssetId) || binding.sourceMaterialName.empty() || binding.sourceMaterialName.size() > 63u ||
                !Is_ValidUtf8DisplayText(binding.sourceMaterialName) || !materialNames.insert(binding.materialName).second ||
                (!binding.diffuseTextureAssetId.empty() && !Is_ResourcePath(binding.diffuseTextureAssetId, false)))
            { outStatus = "Invalid or duplicate world object map material binding: " + object.objectId; return false; }
        if (object.materialProfile && (alias || !Validate_MaterialProfile(*object.materialProfile)))
        { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
		if (!object.defaultMotionInstanceId.empty())
		{
			const auto* motion = Find_Instance(object.defaultMotionInstanceId);
			if (!Is_ValidStableId(object.defaultMotionInstanceId) || nullptr == motion || !motion->enabled ||
				(alias ? object.defaultMotionInstanceId != object.sequenceInstanceId :
					(motion->bindings.size() != 1u || motion->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
					 motion->bindings.front().targetId != object.objectId)))
			{
				outStatus = "Default Motion must be an enabled instance of the same Object: " + object.objectId;
				return false;
			}
		}
	}
	std::unordered_set<std::string> templateIds;
	for (const WORLD_SEQUENCE_TEMPLATE& value : m_Templates)
	{
		if (!Is_ValidStableId(value.sequenceId) ||
			!templateIds.insert(value.sequenceId).second ||
			value.displayName.empty() || value.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(value.displayName) ||
			value.category.empty() || value.category.size() > 64u ||
			!Is_ValidUtf8DisplayText(value.category) ||
			0u == value.durationMs || value.durationMs > MAX_DURATION_MS ||
			(WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation &&
				WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP != value.interpolation) ||
			(value.tracks.empty() && value.animationTracks.empty()) ||
			value.tracks.size() + value.animationTracks.size() + value.effectTracks.size() + value.colliderTracks.size() + value.materialTracks.size() +
                value.soundTracks.size() + value.subtitleTracks.size() > MAX_TRACK_COUNT)
		{
			outStatus = "Invalid or duplicate world sequence template: " +
				value.sequenceId;
			return false;
		}
        std::unordered_set<std::string> soundIds, subtitleIds;
        for (const auto& sound : value.soundTracks)
            if (!Is_ValidStableId(sound.soundTrackId) || !soundIds.insert(sound.soundTrackId).second ||
                !Is_ResourcePath(sound.assetId, false) || !sound.assetId.starts_with("Sound/") || !sound.assetId.ends_with(".wav") ||
                sound.startMs > value.durationMs || sound.durationMs == 0u ||
                uint64_t(sound.startMs) + sound.durationMs > MAX_DURATION_MS ||
                !std::isfinite(sound.volume) || sound.volume < 0.f || sound.volume > 4.f)
            { outStatus = "Invalid World sound track: " + value.sequenceId + "/" + sound.soundTrackId; return false; }
        for (const auto& subtitle : value.subtitleTracks)
        {
            const bool balloon = subtitle.position == "BALLOON";
            const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
                [&](const auto& track) { return track.slotId == subtitle.slotId; });
            if (!Is_ValidStableId(subtitle.subtitleTrackId) || !subtitleIds.insert(subtitle.subtitleTrackId).second ||
                !Is_ValidStableId(subtitle.stringId) || subtitle.text.empty() || subtitle.text.size() > 4096u ||
                !Is_ValidUtf8DisplayText(subtitle.text, true) || subtitle.text.find_first_of("<>") != std::string::npos ||
                (!balloon && subtitle.position != "NORMAL" && subtitle.position != "UPPER") ||
                (balloon ? !Is_ValidStableId(subtitle.slotId) || !slotExists : !subtitle.slotId.empty()) ||
                !subtitle.durationMs || uint64_t(subtitle.startMs) + subtitle.durationMs > value.durationMs)
            { outStatus = "Invalid World subtitle track: " + value.sequenceId + "/" + subtitle.subtitleTrackId; return false; }
        }
		const auto& motion = value.objectMotion;
		if (!Is_BoundedFloat3(motion.velocity) || !Is_BoundedFloat3(motion.acceleration) ||
			!Is_BoundedFloat3(motion.angularVelocityDegrees) ||
			!Is_BoundedFloat3(motion.revolutionDegreesPerSecond) || !Is_BoundedFloat3(motion.revolutionOffset) ||
			!Is_BoundedFloat3(motion.spawnHalfExtents) || motion.spawnHalfExtents.x < 0.f ||
			motion.spawnHalfExtents.y < 0.f || motion.spawnHalfExtents.z < 0.f ||
			motion.count < 1u || motion.count > 128u || motion.intervalMs > MAX_DURATION_MS ||
			(value.effectTracks.empty() && motion.LastEmissionDelayMs() >= value.durationMs) ||
			!Is_ValidEmissionList(motion) ||
			!std::isfinite(motion.spreadDegrees) || motion.spreadDegrees < 0.f || motion.spreadDegrees > (value.effectTracks.empty() ? 180.f : 360.f))
		{ outStatus = "Invalid object motion in template: " + value.sequenceId; return false; }
		std::unordered_set<std::string> effectIds;
		for (const auto& effect : value.effectTracks)
		{
			const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
				[&](const auto& track) { return track.slotId == effect.slotId; }) ||
				std::any_of(value.animationTracks.begin(), value.animationTracks.end(),
					[&](const auto& track) { return track.slotId == effect.slotId; });
			if (!Is_ValidStableId(effect.effectTrackId) || !effectIds.insert(effect.effectTrackId).second ||
				!Is_ValidStableId(effect.slotId) || !slotExists || !Is_ValidStableId(effect.resourceId) ||
				(effect.resourceKind != "LEAF" && effect.resourceKind != "GROUP" && effect.resourceKind != "V1_EFFECT") ||
				((effect.fitEffectToDuration || effect.loopEffectToDuration) && effect.resourceKind != "V1_EFFECT") ||
				(effect.fitEffectToDuration && effect.loopEffectToDuration) ||
				effect.bone.size() > 256u || !Is_ValidUtf8DisplayText(effect.bone) ||
				(effect.timing != "TIME" && effect.timing != "MOTION_END") ||
				(effect.timing == "MOTION_END" && effect.startMs != 0u) ||
				effect.startMs > value.durationMs || effect.durationMs == 0u || effect.durationMs > MAX_DURATION_MS ||
				!Is_BoundedFloat3(effect.positionOffset) || !Is_BoundedFloat3(effect.rotationDegrees) ||
				!Is_BoundedFloat3(effect.scale) || effect.scale.x < MIN_SCALE || effect.scale.y < MIN_SCALE || effect.scale.z < MIN_SCALE ||
				value.PresentationSpanMs() > MAX_DURATION_MS)
			{ outStatus = "Invalid World Object effect track: " + value.sequenceId + "/" + effect.effectTrackId; return false; }
		}
		if (!value.colliderTracks.empty() && (motion.spawnHalfExtents.x != 0.f || motion.spawnHalfExtents.y != 0.f ||
			motion.spawnHalfExtents.z != 0.f || motion.spreadDegrees != 0.f))
		{ outStatus = "Collider tracks require deterministic Motion emission positions: " + value.sequenceId; return false; }
		std::unordered_set<std::string> colliderIds;
		for (const auto& collider : value.colliderTracks)
		{
			const bool hook = collider.behavior == "HOOK_CAPTURE";
			const bool damage = collider.behavior == "DAMAGE";
			const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
				[&](const auto& track) { return track.slotId == collider.slotId; });
			if (!Is_ValidStableId(collider.colliderTrackId) || !colliderIds.insert(collider.colliderTrackId).second ||
				!Is_ValidStableId(collider.slotId) || !slotExists ||
				(collider.shape != "BOX" && collider.shape != "CYLINDER") ||
				(collider.shape == "CYLINDER" && (hook || std::abs(collider.halfExtents.x - collider.halfExtents.z) > .0001f)) ||
				collider.durationMs == 0u || uint64_t(collider.startMs) + collider.durationMs > value.durationMs ||
				!Is_BoundedFloat3(collider.positionOffset) || !Is_BoundedFloat3(collider.halfExtents) ||
				collider.halfExtents.x <= .001f || collider.halfExtents.y <= .001f || collider.halfExtents.z <= .001f ||
				collider.halfExtents.x > 1000.f || collider.halfExtents.y > 1000.f || collider.halfExtents.z > 1000.f ||
				!std::isfinite(collider.yawDegrees) || std::abs(collider.yawDegrees) > 36000.f ||
				(!hook && !damage && collider.behavior != "INSTANT_DEATH") || !std::isfinite(collider.damagePercent) ||
				(damage ? collider.damagePercent < 1.f || collider.damagePercent > 100.f || std::floor(collider.damagePercent) != collider.damagePercent : collider.damagePercent != 0.f) ||
				!Is_BoundedFloat3(collider.gripLocalOffset) || collider.attachmentBone.size() > 256u ||
				!Is_ValidUtf8DisplayText(collider.attachmentBone) ||
				(!hook && (!collider.attachmentBone.empty() || collider.gripLocalOffset.x != 0.f ||
					collider.gripLocalOffset.y != 0.f || collider.gripLocalOffset.z != 0.f)))
			{ outStatus = "Invalid World Object collider track: " + value.sequenceId + "/" + collider.colliderTrackId; return false; }
		}
		std::unordered_set<std::string> slotIds;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
		{
			if (!Is_ValidStableId(track.slotId) ||
				!slotIds.insert(track.slotId).second || track.keys.size() < 2u ||
				track.keys.size() > MAX_KEY_COUNT || 0u != track.keys.front().timeMs ||
				value.durationMs != track.keys.back().timeMs)
			{
				outStatus = "Invalid track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				const auto& first = track.keys.front().scaleMultiplier;
				// A reflected source actor is valid. Each axis must retain its sign
				// so interpolation never crosses a singular, zero-scale transform.
				if (!Is_FiniteTransform(key) ||
					key.timeMs > value.durationMs ||
					(0u != keyIndex &&
						track.keys[keyIndex - 1u].timeMs >= key.timeMs) ||
					std::signbit(first.x) != std::signbit(key.scaleMultiplier.x) ||
					std::signbit(first.y) != std::signbit(key.scaleMultiplier.y) ||
					std::signbit(first.z) != std::signbit(key.scaleMultiplier.z))
				{
					outStatus = "Invalid keyframe in world sequence template: " +
						value.sequenceId + "/" + track.slotId;
					return false;
				}
			}
		}
        std::unordered_set<std::string> materialTargets;
        for (const auto& track : value.materialTracks)
        {
            if (!Is_ValidStableId(track.slotId) || track.materialName.empty() || track.materialName.size() > 256u ||
                !Is_ValidUtf8DisplayText(track.materialName) || !materialTargets.insert(track.slotId + ":" + track.materialName).second ||
                track.curves.empty() || track.curves.size() > 64u ||
                (std::none_of(value.tracks.begin(), value.tracks.end(), [&](const auto& row) { return row.slotId == track.slotId; }) &&
                 std::none_of(value.animationTracks.begin(), value.animationTracks.end(), [&](const auto& row) { return row.slotId == track.slotId; })))
            { outStatus = "Invalid World material track target: " + value.sequenceId; return false; }
            std::unordered_set<std::string> parameters;
            for (const auto& curve : track.curves)
            {
                if (curve.parameter.empty() || curve.parameter.size() > 128u || !Is_ValidUtf8DisplayText(curve.parameter) ||
                    !parameters.insert(curve.parameter).second || curve.keys.empty() || curve.keys.size() > 4096u ||
                    curve.keys.front().timeMs != 0u || curve.keys.back().timeMs != value.durationMs)
                { outStatus = "World material curve must cover its motion: " + value.sequenceId; return false; }
                for (size_t index = 0u; index < curve.keys.size(); ++index)
                {
                    const auto& key = curve.keys[index];
                    if (key.timeMs > value.durationMs || (index && key.timeMs <= curve.keys[index - 1u].timeMs) ||
                        std::any_of(key.value.begin(), key.value.end(), [](float v) { return !std::isfinite(v) || std::abs(v) > 1000000.f; }))
                    { outStatus = "Invalid World material key: " + value.sequenceId; return false; }
                }
            }
        }
		/* An animation slot may carry an ordered clip chain, so its rows are
		   checked against the slot's previous start instead of a plain unique
		   set. A slot still may not be both a transform and an animation slot. */
		std::unordered_map<std::string, uint32_t> animationSlotStarts;
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
			value.animationTracks)
		{
			const auto chained = animationSlotStarts.find(track.slotId);
			const bool_t firstOfSlot = animationSlotStarts.end() == chained;
			/* A slot may carry both a transform track and a clip chain so one
			   binding can walk an animated prop while it plays. Only a second
			   animation chain on the same slot is a conflict. */
			if (!Is_ValidStableId(track.slotId) || track.clipName.empty() ||
				track.clipName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.clipName) ||
				track.displayName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.displayName) ||
				!std::isfinite(track.playbackRate) || track.playbackRate < 0.05f ||
				track.playbackRate > 8.f ||
				track.startMs >= value.durationMs || track.sourceStartMs > MAX_DURATION_MS ||
				track.sourceEndMs > MAX_DURATION_MS ||
				(track.sourceEndMs != 0u && track.sourceEndMs <= track.sourceStartMs) ||
				(!firstOfSlot && track.startMs <= chained->second))
			{
				outStatus = "Invalid animation track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			animationSlotStarts[track.slotId] = track.startMs;
		}
	}

	/* One binding drives one slot, so a slot whose clips are chained still
	   needs exactly one. */
	const auto Count_BoundSlots =
		[](const WORLD_SEQUENCE_TEMPLATE& value) -> size_t
	{
		std::unordered_set<std::string> slots;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
			slots.insert(track.slotId);
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track : value.animationTracks)
			slots.insert(track.slotId);
		return slots.size();
	};

	std::unordered_set<std::string> instanceIds;
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const WORLD_SEQUENCE_TEMPLATE* targetTemplate = Find_Template(value.templateId);
		if (!Is_ValidStableId(value.instanceId) ||
			!instanceIds.insert(value.instanceId).second || nullptr == targetTemplate ||
			value.startDelayMs > MAX_DURATION_MS ||
			!std::isfinite(value.playbackSpeed) || value.playbackSpeed < 0.05f ||
			value.playbackSpeed > 8.f ||
			(value.anchorKind != "WORLD" && value.anchorKind != "PLAYER" && value.anchorKind != "BOSS") ||
			!Is_BoundedFloat3(value.position) ||
			value.bindings.size() != Count_BoundSlots(*targetTemplate))
		{
			outStatus = "Invalid world sequence instance: " + value.instanceId;
			return false;
		}
		if (value.loopFullPresentation && (value.motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP ||
			value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Full presentation loop requires one looping Object Resource: " + value.instanceId; return false; }
		if (!targetTemplate->colliderTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || value.anchorKind != "WORLD"))
		{ outStatus = "Collider tracks require one WORLD Object Resource binding: " + value.instanceId; return false; }
		if (!targetTemplate->effectTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Effect lanes require one Object Resource binding: " + value.instanceId; return false; }
        for (const auto& subtitle : targetTemplate->subtitleTracks)
            if (subtitle.position == "BALLOON" && std::none_of(value.bindings.begin(), value.bindings.end(), [&](const auto& binding) {
                return binding.slotId == subtitle.slotId && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; }))
            { outStatus = "World balloon subtitle requires its Object Resource binding: " + value.instanceId; return false; }
		if (value.walkableSurface)
		{
			const auto& surface = *value.walkableSurface;
			if (!std::isfinite(surface.radiusM) || surface.radiusM < 0.001f || surface.radiusM > 1000.f ||
				!std::isfinite(surface.localHeightM) || std::abs(surface.localHeightM) > 10000.f ||
				value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT ||
				value.anchorKind != "WORLD" || value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP ||
				targetTemplate->tracks.size() != 1u || !targetTemplate->animationTracks.empty())
			{ outStatus = "Walkable surface requires one static Map placement: " + value.instanceId; return false; }
			const auto& keys = targetTemplate->tracks.front().keys;
			const auto& first = keys.front();
			for (const auto& key : keys)
			{
				if (std::abs(key.rotationQuaternion.x) > 0.00001f || std::abs(key.rotationQuaternion.z) > 0.00001f ||
					key.positionOffset.x != first.positionOffset.x || key.positionOffset.y != first.positionOffset.y ||
					key.positionOffset.z != first.positionOffset.z || key.scaleMultiplier.x != first.scaleMultiplier.x ||
					key.scaleMultiplier.y != first.scaleMultiplier.y || key.scaleMultiplier.z != first.scaleMultiplier.z ||
					key.scaleMultiplier.x <= 0.f || key.scaleMultiplier.y <= 0.f ||
					std::abs(key.scaleMultiplier.x - key.scaleMultiplier.z) > 0.00001f)
				{ outStatus = "Walkable surface needs fixed position/scale and Y rotation only: " + value.instanceId; return false; }
			}
		}
		std::unordered_set<std::string> boundSlots;
		std::unordered_set<std::string> boundTargets;
		for (const WORLD_SEQUENCE_BINDING& binding : value.bindings)
		{
			if (!binding.previewNpcPlacementId.empty())
			{
				const auto* object = Find_ObjectResource(binding.targetId);
				if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || !object ||
					!object->animated || !object->sequenceInstanceId.empty() || object->combatBody ||
					!object->presentationBossArchetypeId.empty() || value.anchorKind != "WORLD" ||
					(value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && value.motionEnd != WORLD_SEQUENCE_MOTION_END::HOLD) ||
					value.bindings.size() != 1u || targetTemplate->objectMotion.EmissionCount() != 1u ||
					!targetTemplate->colliderTracks.empty() || !Is_ValidStableId(binding.previewNpcPlacementId))
				{ outStatus = "NPC preview requires one animated WORLD visual without combat"; return false; }
			}
			const auto transformSlot = std::find_if(targetTemplate->tracks.begin(),
				targetTemplate->tracks.end(),
				[&binding](const WORLD_SEQUENCE_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
            for (const auto& material : targetTemplate->materialTracks)
                if (material.slotId == binding.slotId)
                {
                    const auto* resource = binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
                        Find_ObjectResource(binding.targetId) : nullptr;
                    if (!resource || !resource->materialProfile || resource->materialProfile->materialName != material.materialName)
                    { outStatus = "World material track requires its exact Object material profile: " + value.instanceId; return false; }
                    for (const auto& curve : material.curves)
                    {
                        if (!resource->materialProfile->parameters.contains(curve.parameter))
                        { outStatus = "World material curve parameter is absent from its profile: " + curve.parameter; return false; }
                        for (const auto& key : curve.keys)
                        {
                            auto profile = *resource->materialProfile;
                            profile.parameters[curve.parameter] = key.value;
                            if (!Validate_MaterialProfile(profile))
                            { outStatus = "World material curve key is outside its native profile: " + curve.parameter; return false; }
                        }
                    }
                }
			const bool colliderSlot = std::any_of(targetTemplate->colliderTracks.begin(), targetTemplate->colliderTracks.end(),
				[&](const auto& collider) { return collider.slotId == binding.slotId; });
			if (colliderSlot && binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
			{ outStatus = "Collider tracks require an Object Resource binding: " + value.instanceId + "/" + binding.slotId; return false; }
			const auto animationSlot = std::find_if(
				targetTemplate->animationTracks.begin(),
				targetTemplate->animationTracks.end(),
				[&binding](const WORLD_SEQUENCE_ANIMATION_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
			uint64_t targetId = 0;
			const std::string uniqueTarget =
				std::string(TargetKind_ToString(binding.targetKind)) + ":" +
				binding.targetId;
			const bool_t hasTransformSlot =
				targetTemplate->tracks.end() != transformSlot;
			if (hasTransformSlot && binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
			{
				const auto& scale = transformSlot->keys.front().scaleMultiplier;
				if (scale.x < 0.f || scale.y < 0.f || scale.z < 0.f)
				{ outStatus = "Signed scale requires an Object Resource binding: " + value.instanceId; return false; }
			}
			const bool_t hasAnimationSlot =
				targetTemplate->animationTracks.end() != animationSlot;
			/* A Deploy target may carry a transform track alongside its clip
			   chain so one binding can walk an animated prop while it plays.
			   A map placement has no clips, so an animation slot there is
			   still a mistake. */
			const bool_t bindingShapeIsValid =
				WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT ==
					binding.targetKind ?
				hasAnimationSlot : (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
				(hasTransformSlot || hasAnimationSlot) : (hasTransformSlot && !hasAnimationSlot));
			if (!boundSlots.insert(binding.slotId).second ||
				(binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
					!Is_ValidStableId(binding.targetId) : !Parse_Uint64Text(binding.targetId, targetId)) ||
				!boundTargets.insert(uniqueTarget).second ||
				!bindingShapeIsValid)
			{
				outStatus = "Invalid binding in world sequence instance: " +
					value.instanceId;
				return false;
			}
			/* The binding's own kind decides which target table admits it. A
			   Deploy slot that also carries a transform track is still a
			   Deploy binding. */
			if (WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE == binding.targetKind)
			{
				const auto* object = Find_ObjectResource(binding.targetId);
				const bool colliderBone = std::any_of(targetTemplate->colliderTracks.begin(), targetTemplate->colliderTracks.end(),
					[&](const auto& collider) { return collider.slotId == binding.slotId && !collider.attachmentBone.empty(); });
				if (!object || object->modelAssetId.empty() || !object->sequenceInstanceId.empty() ||
					((hasAnimationSlot || colliderBone) && !object->animated) ||
					(colliderSlot && object->anchorKind != "WORLD") ||
					((value.anchorKind == "BOSS" || object->anchorKind == "BOSS") &&
					 (value.anchorKind != "BOSS" || object->anchorKind != "BOSS" || value.bindings.size() != 1u)))
				{ outStatus = "Invalid object resource binding: " + value.instanceId + "/" + binding.slotId; return false; }
				continue;
			}
			if (value.anchorKind != "WORLD" || value.position.x != 0.f || value.position.y != 0.f || value.position.z != 0.f)
			{ outStatus = "Placed sequences cannot use object instance anchors: " + value.instanceId; return false; }
			if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
			{
				const auto deploy = availableDeployPlacements.find(targetId);
				if (availableDeployPlacements.end() == deploy ||
					!deploy->second.animationTargetSupported)
				{
					outStatus = "Invalid animated Deploy binding in world sequence instance: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
				/* Every clip of the chain must exist on the prop, not just the
				   first, so a mistyped later beat fails here instead of part
				   way through the cutscene. */
				for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
					targetTemplate->animationTracks)
				{
					if (track.slotId != binding.slotId)
						continue;
					if (deploy->second.animationClips.end() == std::find(
						deploy->second.animationClips.begin(),
						deploy->second.animationClips.end(), track.clipName))
					{
						outStatus = "Invalid animated Deploy clip in world sequence instance: " +
							value.instanceId + "/" + track.clipName;
						return false;
					}
				}
				continue;
			}
			const auto placement = availablePlacements.find(targetId);
			if (WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind ||
				availablePlacements.end() == placement ||
				!placement->second.sequenceTargetSupported)
			{
				outStatus = "Invalid map binding in world sequence instance: " +
					value.instanceId + "/" + binding.slotId;
				return false;
			}
			const float3_t& baselineScale =
				placement->second.signedScale;
			if (value.walkableSurface && (baselineScale.x <= 0.f || baselineScale.y <= 0.f ||
				std::abs(baselineScale.x - baselineScale.z) > 0.00001f))
			{ outStatus = "Walkable surface placement scale must be positive and uniform in X/Z: " + value.instanceId; return false; }

			for (const WORLD_SEQUENCE_TRANSFORM_KEY& key : transformSlot->keys)
			{
				const double scaleX = static_cast<double>(baselineScale.x) *
					static_cast<double>(key.scaleMultiplier.x);
				const double scaleY = static_cast<double>(baselineScale.y) *
					static_cast<double>(key.scaleMultiplier.y);
				const double scaleZ = static_cast<double>(baselineScale.z) *
					static_cast<double>(key.scaleMultiplier.z);
				const f32_t composedX = static_cast<f32_t>(scaleX);
				const f32_t composedY = static_cast<f32_t>(scaleY);
				const f32_t composedZ = static_cast<f32_t>(scaleZ);
				const double determinant = static_cast<double>(composedX) *
					static_cast<double>(composedY) *
					static_cast<double>(composedZ);
				const f32_t runtimeDeterminant =
					static_cast<f32_t>(determinant);
				if (!std::isfinite(composedX) || !std::isfinite(composedY) ||
					!std::isfinite(composedZ) || !std::isfinite(determinant) ||
					!std::isfinite(runtimeDeterminant) ||
					std::abs(runtimeDeterminant) < MIN_RUNTIME_SCALE_DETERMINANT)
				{
					outStatus = "Sequence scale would create a singular map transform: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
			}
		}
	}
	/* Resolve completion links only after every instance and binding is valid.
	   A motion changes the existing object, so it cannot switch resource or slot. */
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const bool_t next = value.motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT;
		if (std::string_view(MotionEnd_ToString(value.motionEnd)) == "INVALID" ||
			(next ? !Is_ValidStableId(value.nextMotionId) : !value.nextMotionId.empty()) ||
			(value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP &&
				(value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)))
		{ outStatus = "Invalid world object motion completion: " + value.instanceId; return false; }
		if (!next) continue;
		const auto* target = Find_Instance(value.nextMotionId);
		if (!target || !target->enabled || target->bindings.size() != 1u ||
			target->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
			target->bindings.front().targetId != value.bindings.front().targetId ||
			target->bindings.front().slotId != value.bindings.front().slotId ||
			Find_Template(value.templateId)->objectMotion.count != 1u ||
			Find_Template(target->templateId)->objectMotion.count != 1u)
		{ outStatus = "NEXT motion must target an enabled single object state with the same resource and slot: " + value.instanceId; return false; }
	}
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		std::unordered_set<std::string> visited;
		const WORLD_SEQUENCE_INSTANCE* current = &value;
		uint32_t depth = 0u;
		while (current->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
		{
			if (!visited.insert(current->instanceId).second || ++depth > 32u)
			{ outStatus = "World object NEXT motion chain contains a cycle or exceeds 32 links: " + value.instanceId; return false; }
			current = Find_Instance(current->nextMotionId);
		}
	}
	outStatus = "World sequence document is valid";
	return true;
}

void Client::CWorldSequenceDocument::Reset_Empty(const std::string& areaId)
{
	m_AreaId = areaId;
	m_iRevision = 1;
	m_Templates.clear();
	m_Instances.clear();
	m_ObjectResources.clear();
	m_ObjectFolders.clear();
}

void Client::CWorldSequenceDocument::Touch()
{
	if (m_iRevision < (std::numeric_limits<uint32_t>::max)())
		++m_iRevision;
}

Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(const std::string& sequenceId)
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(
	const std::string& sequenceId) const
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(const std::string& instanceId)
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(
	const std::string& instanceId) const
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_OBJECT_FOLDER* Client::CWorldSequenceDocument::Find_ObjectFolder(const std::string& folderId)
{
    const auto found = std::find_if(m_ObjectFolders.begin(), m_ObjectFolders.end(),
        [&folderId](const auto& value) { return value.folderId == folderId; });
    return found == m_ObjectFolders.end() ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_OBJECT_FOLDER* Client::CWorldSequenceDocument::Find_ObjectFolder(const std::string& folderId) const
{
    const auto found = std::find_if(m_ObjectFolders.begin(), m_ObjectFolders.end(),
        [&folderId](const auto& value) { return value.folderId == folderId; });
    return found == m_ObjectFolders.end() ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId)
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId) const
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

bool_t Client::CWorldSequenceDocument::Is_Equivalent(
	const CWorldSequenceDocument& other) const
{
	const auto sameFloat = [](const f32_t left, const f32_t right)
	{
		return left == right;
	};
	const auto sameFloat3 = [&sameFloat](
		const float3_t& left, const float3_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z);
	};
	const auto sameFloat4 = [&sameFloat](
		const float4_t& left, const float4_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z) && sameFloat(left.w, right.w);
	};
	if (m_AreaId != other.m_AreaId || m_iRevision != other.m_iRevision ||
		m_Templates.size() != other.m_Templates.size() ||
		m_Instances.size() != other.m_Instances.size() ||
		m_ObjectResources.size() != other.m_ObjectResources.size() ||
        m_ObjectFolders != other.m_ObjectFolders)
	{
		return false;
	}
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& left = m_ObjectResources[index];
		const auto& right = other.m_ObjectResources[index];
		if (left.objectId != right.objectId || left.displayName != right.displayName ||
            left.parentId != right.parentId ||
			left.anchorKind != right.anchorKind || left.anchorBossArchetypeId != right.anchorBossArchetypeId ||
			left.anchorBone != right.anchorBone ||
			left.modelAssetId != right.modelAssetId || left.diffuseTextureAssetId != right.diffuseTextureAssetId ||
            left.materialProfile != right.materialProfile ||
            left.materialSourceModelAssetId != right.materialSourceModelAssetId || left.mapMaterialBindings != right.mapMaterialBindings ||
			left.animationSetAssetId != right.animationSetAssetId ||
			left.presentationBossArchetypeId != right.presentationBossArchetypeId ||
			left.modelPreScale != right.modelPreScale || left.animated != right.animated ||
			!sameFloat3(left.scale, right.scale) || left.sequenceInstanceId != right.sequenceInstanceId ||
			left.motionInstanceIds != right.motionInstanceIds ||
			left.defaultMotionInstanceId != right.defaultMotionInstanceId) return false;
		if (left.combatBody.has_value() != right.combatBody.has_value() || (left.combatBody &&
			(left.combatBody->maxHp != right.combatBody->maxHp || left.combatBody->shape != right.combatBody->shape || left.combatBody->lifetimePolicy != right.combatBody->lifetimePolicy ||
			!sameFloat3(left.combatBody->localCenterM, right.combatBody->localCenterM) ||
			!sameFloat3(left.combatBody->halfExtentsM, right.combatBody->halfExtentsM)))) return false;
	}
	for (size_t templateIndex = 0u; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& left = m_Templates[templateIndex];
		const WORLD_SEQUENCE_TEMPLATE& right = other.m_Templates[templateIndex];
		if (left.sequenceId != right.sequenceId ||
			left.displayName != right.displayName ||
			left.category != right.category || left.durationMs != right.durationMs ||
			left.interpolation != right.interpolation ||
			left.tracks.size() != right.tracks.size() ||
			left.animationTracks.size() != right.animationTracks.size() ||
			left.effectTracks.size() != right.effectTracks.size() ||
			left.colliderTracks.size() != right.colliderTracks.size() ||
            left.soundTracks != right.soundTracks || left.subtitleTracks != right.subtitleTracks ||
            left.materialTracks != right.materialTracks ||
			!sameFloat3(left.objectMotion.velocity, right.objectMotion.velocity) ||
			!sameFloat3(left.objectMotion.acceleration, right.objectMotion.acceleration) ||
			!sameFloat3(left.objectMotion.angularVelocityDegrees, right.objectMotion.angularVelocityDegrees) ||
			!sameFloat3(left.objectMotion.revolutionDegreesPerSecond, right.objectMotion.revolutionDegreesPerSecond) ||
			!sameFloat3(left.objectMotion.revolutionOffset, right.objectMotion.revolutionOffset) ||
			!sameFloat3(left.objectMotion.spawnHalfExtents, right.objectMotion.spawnHalfExtents) ||
			left.objectMotion.count != right.objectMotion.count || left.objectMotion.intervalMs != right.objectMotion.intervalMs ||
			left.objectMotion.spreadDegrees != right.objectMotion.spreadDegrees || left.objectMotion.seed != right.objectMotion.seed ||
			left.objectMotion.emissions.size() != right.objectMotion.emissions.size())
		{
			return false;
		}
		for (size_t index = 0; index < left.objectMotion.emissions.size(); ++index)
		{
			const auto& a = left.objectMotion.emissions[index]; const auto& b = right.objectMotion.emissions[index];
			if (!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat(a.yawDegrees, b.yawDegrees) ||
				a.startDelayMs != b.startDelayMs) return false;
		}
		for (size_t index = 0; index < left.effectTracks.size(); ++index)
		{
			const auto& a = left.effectTracks[index]; const auto& b = right.effectTracks[index];
			if (a.effectTrackId != b.effectTrackId || a.slotId != b.slotId || a.resourceKind != b.resourceKind ||
				a.resourceId != b.resourceId || a.fitEffectToDuration != b.fitEffectToDuration || a.loopEffectToDuration != b.loopEffectToDuration || a.followObject != b.followObject || a.inheritObjectRotation != b.inheritObjectRotation || a.bone != b.bone || a.timing != b.timing || a.startMs != b.startMs || a.durationMs != b.durationMs ||
				!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat3(a.rotationDegrees, b.rotationDegrees) ||
				!sameFloat3(a.scale, b.scale)) return false;
		}
		for (size_t index = 0; index < left.colliderTracks.size(); ++index)
		{
			const auto& a = left.colliderTracks[index]; const auto& b = right.colliderTracks[index];
			if (a.colliderTrackId != b.colliderTrackId || a.slotId != b.slotId || a.startMs != b.startMs ||
				a.durationMs != b.durationMs || !sameFloat3(a.positionOffset, b.positionOffset) ||
				!sameFloat3(a.halfExtents, b.halfExtents) || !sameFloat(a.yawDegrees, b.yawDegrees) ||
				a.shape != b.shape || a.behavior != b.behavior || !sameFloat(a.damagePercent, b.damagePercent) ||
				!sameFloat3(a.gripLocalOffset, b.gripLocalOffset) || a.attachmentBone != b.attachmentBone) return false;
		}
		for (size_t trackIndex = 0u; trackIndex < left.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& leftTrack = left.tracks[trackIndex];
			const WORLD_SEQUENCE_TRACK& rightTrack = right.tracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.keys.size() != rightTrack.keys.size())
			{
				return false;
			}
			for (size_t keyIndex = 0u; keyIndex < leftTrack.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& leftKey = leftTrack.keys[keyIndex];
				const WORLD_SEQUENCE_TRANSFORM_KEY& rightKey = rightTrack.keys[keyIndex];
				if (leftKey.timeMs != rightKey.timeMs ||
					!sameFloat3(leftKey.positionOffset, rightKey.positionOffset) ||
					!sameFloat4(leftKey.rotationQuaternion,
						rightKey.rotationQuaternion) ||
					!sameFloat3(leftKey.scaleMultiplier,
						rightKey.scaleMultiplier) ||
					leftKey.visible != rightKey.visible)
				{
					return false;
				}
			}
		}
		for (size_t trackIndex = 0u;
			trackIndex < left.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& leftTrack =
				left.animationTracks[trackIndex];
			const WORLD_SEQUENCE_ANIMATION_TRACK& rightTrack =
				right.animationTracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.startMs != rightTrack.startMs ||
				leftTrack.sourceStartMs != rightTrack.sourceStartMs ||
				leftTrack.sourceEndMs != rightTrack.sourceEndMs ||
				leftTrack.clipName != rightTrack.clipName ||
				leftTrack.displayName != rightTrack.displayName ||
				!sameFloat(leftTrack.playbackRate, rightTrack.playbackRate) ||
				leftTrack.loop != rightTrack.loop ||
				leftTrack.holdLastFrame != rightTrack.holdLastFrame)
			{
				return false;
			}
		}
	}
	for (size_t instanceIndex = 0u; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& left = m_Instances[instanceIndex];
		const WORLD_SEQUENCE_INSTANCE& right = other.m_Instances[instanceIndex];
		if (left.instanceId != right.instanceId ||
			left.templateId != right.templateId || left.enabled != right.enabled ||
			left.startDelayMs != right.startDelayMs ||
			!sameFloat(left.playbackSpeed, right.playbackSpeed) ||
			left.bindings.size() != right.bindings.size() || left.anchorKind != right.anchorKind ||
			!sameFloat3(left.position, right.position) ||
			left.motionEnd != right.motionEnd || left.loopFullPresentation != right.loopFullPresentation || left.nextMotionId != right.nextMotionId)
		{
			return false;
		}
		if (left.walkableSurface.has_value() != right.walkableSurface.has_value() ||
			(left.walkableSurface && (!sameFloat(left.walkableSurface->radiusM, right.walkableSurface->radiusM) ||
				!sameFloat(left.walkableSurface->localHeightM, right.walkableSurface->localHeightM)))) return false;
		for (size_t bindingIndex = 0u; bindingIndex < left.bindings.size();
			++bindingIndex)
		{
			if (left.bindings[bindingIndex].slotId !=
				right.bindings[bindingIndex].slotId ||
				left.bindings[bindingIndex].targetKind !=
					right.bindings[bindingIndex].targetKind ||
				left.bindings[bindingIndex].targetId !=
					right.bindings[bindingIndex].targetId ||
				left.bindings[bindingIndex].previewNpcPlacementId !=
					right.bindings[bindingIndex].previewNpcPlacementId)
			{
				return false;
			}
		}
	}
	return true;
}

bool_t Client::CWorldSequenceDocument::Try_EffectTimeScale(
    const WORLD_SEQUENCE_EFFECT_TRACK& effect, const f32_t sourceDurationSeconds, f32_t& outScale)
{
    if (!effect.fitEffectToDuration) { outScale = 1.f; return true; }
    if (effect.resourceKind != "V1_EFFECT" || !effect.durationMs ||
        !std::isfinite(sourceDurationSeconds) || sourceDurationSeconds <= 0.f) return false;
    const double rate = double(sourceDurationSeconds) * 1000. / effect.durationMs;
    if (!std::isfinite(rate) || rate <= 0. || rate > (std::numeric_limits<f32_t>::max)()) return false;
    const f32_t scale = static_cast<f32_t>(rate);
    if (!std::isfinite(scale) || scale <= 0.f) return false;
    outScale = scale;
    return true;
}

bool_t Client::CWorldSequenceDocument::Resize_TimelineDuration(const std::string& sequenceId,
    const uint32_t durationMs, const uint32_t requiredAnimationEndMs,
    const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, std::string& outStatus)
{
    const auto reject = [&](const char* reason) { outStatus = reason; return false; };
    auto* current = Find_Template(sequenceId);
    if (!current || !durationMs || durationMs > MAX_DURATION_MS || durationMs < requiredAnimationEndMs)
        return reject("Stage duration would cut an Animation or exceed the timeline limit. Existing rows preserved.");
    if (durationMs == current->durationMs) return true;
    auto candidate = *this;
    auto* edited = candidate.Find_Template(sequenceId);
    const auto samePose = [](const auto& a, const auto& b) {
        return a.positionOffset.x == b.positionOffset.x && a.positionOffset.y == b.positionOffset.y && a.positionOffset.z == b.positionOffset.z &&
            a.rotationQuaternion.x == b.rotationQuaternion.x && a.rotationQuaternion.y == b.rotationQuaternion.y &&
            a.rotationQuaternion.z == b.rotationQuaternion.z && a.rotationQuaternion.w == b.rotationQuaternion.w &&
            a.scaleMultiplier.x == b.scaleMultiplier.x && a.scaleMultiplier.y == b.scaleMultiplier.y && a.scaleMultiplier.z == b.scaleMultiplier.z &&
            a.visible == b.visible;
    };
    for (auto& track : edited->tracks)
    {
        if (track.keys.size() < 2u) return reject("Stage requires valid Transform endpoints.");
        if (durationMs > edited->durationMs)
        {
            if (track.keys.size() >= MAX_KEY_COUNT) return reject("Stage extension exceeds the Transform key limit.");
            auto endpoint = track.keys.back(); endpoint.timeMs = durationMs;
            track.keys.push_back(endpoint); // Never retime an authored key.
        }
        else
        {
            // Only trim a constant held tail. A changed pose/visibility is authored content.
            while (track.keys.size() > 1u && track.keys.back().timeMs > durationMs)
            {
                if (!samePose(track.keys.back(), track.keys[track.keys.size() - 2u]))
                    return reject("Stage shortening would cut a Transform or visibility change. Existing rows preserved.");
                auto endpoint = track.keys.back(); track.keys.pop_back();
                if (track.keys.back().timeMs < durationMs)
                { endpoint.timeMs = durationMs; track.keys.push_back(endpoint); break; }
            }
            if (track.keys.size() < 2u) return reject("Stage shortening would remove a required Transform endpoint.");
        }
    }
    for (auto& material : edited->materialTracks)
        for (auto& curve : material.curves)
        {
            if (curve.keys.size() < 2u) return reject("Stage requires valid material endpoints.");
            if (durationMs > edited->durationMs)
            {
                if (curve.keys.size() >= MAX_KEY_COUNT) return reject("Stage extension exceeds the material key limit.");
                auto endpoint = curve.keys.back(); endpoint.timeMs = durationMs;
                curve.keys.push_back(endpoint);
            }
            else
            {
                while (curve.keys.size() > 1u && curve.keys.back().timeMs > durationMs)
                {
                    if (curve.keys.back().value != curve.keys[curve.keys.size() - 2u].value)
                        return reject("Stage shortening would cut a material change. Existing rows preserved.");
                    auto endpoint = curve.keys.back(); curve.keys.pop_back();
                    if (curve.keys.back().timeMs < durationMs)
                    { endpoint.timeMs = durationMs; curve.keys.push_back(endpoint); break; }
                }
                if (curve.keys.size() < 2u) return reject("Stage shortening would remove a required material endpoint.");
            }
        }
    bool pinnedMotionEnd = false;
    for (auto& effect : edited->effectTracks)
        if (effect.timing == "MOTION_END")
        { effect.timing = "TIME"; effect.startMs = edited->durationMs; pinnedMotionEnd = true; }
    edited->durationMs = durationMs;
    // Validation rejects clipped starts, Collider windows and emission limits, without moving them.
    if (!candidate.Validate(mapPlacements, deployPlacements, outStatus))
    { outStatus = "Stage resize refused: " + outStatus + ". Existing rows preserved."; return false; }
    *current = std::move(*edited); // Keep UI references to this template valid.
    outStatus = "Stage duration updated; Animation, Effect and Collider timings are preserved.";
    if (pinnedMotionEnd) outStatus += " Motion End Effects now use At Time to keep their previous start.";
    return true;
}

bool_t Client::CWorldSequenceDocument::Try_SampleAnimationTicks(
    const WORLD_SEQUENCE_ANIMATION_TRACK& track, const f32_t localMs, const f32_t windowEndMs,
    const f32_t ticksPerSecond, const f32_t durationTicks, f32_t& outTicks)
{
    if (!std::isfinite(localMs) || !std::isfinite(windowEndMs) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f ||
        !std::isfinite(durationTicks) || durationTicks <= 0.f ||
        !std::isfinite(track.playbackRate) || track.playbackRate <= 0.f)
        return false;
    const f32_t sourceTicks = static_cast<f32_t>(static_cast<double>(track.sourceStartMs) *
        .001 * static_cast<double>(ticksPerSecond));
    // Authored milliseconds may round a native float duration by less than one ms.
    const f32_t authoredEnd = track.sourceEndMs == 0u ? durationTicks :
        static_cast<f32_t>(static_cast<double>(track.sourceEndMs) * .001 * static_cast<double>(ticksPerSecond));
    if (!std::isfinite(sourceTicks) || sourceTicks > durationTicks || !std::isfinite(authoredEnd) ||
        (track.sourceEndMs != 0u && (track.sourceEndMs <= track.sourceStartMs ||
            static_cast<double>(track.sourceEndMs) > static_cast<double>(durationTicks) * 1000.0 / ticksPerSecond + 1.0))) return false;
    const f32_t endTicks = (std::min)(authoredEnd, durationTicks);
    if ((track.loop && sourceTicks >= endTicks) || sourceTicks > endTicks) return false;
    const f32_t elapsedTicks = (std::max)(0.f, localMs - track.startMs) * .001f *
        track.playbackRate * ticksPerSecond;
    if (!std::isfinite(elapsedTicks)) return false;
    f32_t ticks = sourceTicks + elapsedTicks;
    // Hold wins at the timeline end, including looped clips, as before.
    if (localMs >= windowEndMs && track.holdLastFrame) ticks = endTicks;
    else if (track.loop) ticks = sourceTicks + std::fmod(elapsedTicks, endTicks - sourceTicks);
    else if (ticks > endTicks) ticks = track.holdLastFrame ? endTicks : sourceTicks;
    if (!std::isfinite(ticks)) return false;
    outTicks = ticks;
    return true;
}

const char_t* Client::CWorldSequenceDocument::Interpolation_ToString(
	const WORLD_SEQUENCE_INTERPOLATION interpolation)
{
	switch (interpolation)
	{
	case WORLD_SEQUENCE_INTERPOLATION::LINEAR:
		return "LINEAR";
	case WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP:
		return "SMOOTH_STEP";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseInterpolation(
	const std::string& value,
	WORLD_SEQUENCE_INTERPOLATION& outInterpolation)
{
	if ("LINEAR" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR;
	else if ("SMOOTH_STEP" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::TargetKind_ToString(
	const WORLD_SEQUENCE_TARGET_KIND targetKind)
{
	switch (targetKind)
	{
	case WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT:
		return "MAP_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT:
		return "DEPLOY_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE:
		return "OBJECT_RESOURCE";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseTargetKind(
	const std::string& value,
	WORLD_SEQUENCE_TARGET_KIND& outTargetKind)
{
	if ("MAP_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	else if ("DEPLOY_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT;
	else if ("OBJECT_RESOURCE" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::MotionEnd_ToString(
	const WORLD_SEQUENCE_MOTION_END motionEnd)
{
	switch (motionEnd)
	{
	case WORLD_SEQUENCE_MOTION_END::STOP: return "STOP";
	case WORLD_SEQUENCE_MOTION_END::HOLD: return "HOLD";
	case WORLD_SEQUENCE_MOTION_END::LOOP: return "LOOP";
	case WORLD_SEQUENCE_MOTION_END::NEXT: return "NEXT";
	default: return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseMotionEnd(
	const std::string& value, WORLD_SEQUENCE_MOTION_END& outMotionEnd)
{
	if (value == "STOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::STOP;
	else if (value == "HOLD") outMotionEnd = WORLD_SEQUENCE_MOTION_END::HOLD;
	else if (value == "LOOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::LOOP;
	else if (value == "NEXT") outMotionEnd = WORLD_SEQUENCE_MOTION_END::NEXT;
	else return false;
	return true;
}

bool_t Client::CWorldSequenceDocument::Is_ValidStableId(
	const std::string& value)
{
	return !value.empty() && value.size() <= 128u &&
		std::all_of(value.begin(), value.end(), [](const unsigned char character)
		{
			return 0 != std::isalnum(character) || character == '_' ||
				character == '-' || character == '.';
		});
}

bool_t Client::CWorldSequenceDocument::Try_SampleMaterialParameters(
    const WORLD_SEQUENCE_MATERIAL_PROFILE& profile, const WORLD_SEQUENCE_MATERIAL_TRACK& track,
    const f32_t timeMs, Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& out)
{
    if (!std::isfinite(timeMs) || track.materialName != profile.materialName) return false;
    auto values = profile.parameters;
    for (const auto& curve : track.curves)
    {
        if (curve.keys.empty() || !values.contains(curve.parameter)) return false;
        auto right = std::upper_bound(curve.keys.begin(), curve.keys.end(), timeMs,
            [](float time, const auto& key) { return time < key.timeMs; });
        const auto left = right == curve.keys.begin() ? right : right - 1;
        const float fraction = right == curve.keys.end() || left == right || left->constant ? 0.f :
            (timeMs - left->timeMs) / (right->timeMs - left->timeMs);
        auto& value = values.at(curve.parameter);
        for (size_t axis = 0u; axis < 4u; ++axis)
            value[axis] = left->value[axis] + (fraction ? (right->value[axis] - left->value[axis]) * fraction : 0.f);
    }
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged;
    if (!SourceCharacterMaterial::Configure(profile.family, values, staged)) return false;
    out = std::move(staged);
    return true;
}

bool_t Client::CWorldSequenceDocument::Is_ValidMaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile)
{
    return Validate_MaterialProfile(profile);
}

bool_t Client::CWorldSequenceDocument::Build_MaterialOverride(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
    const std::filesystem::path& resourceRoot, Engine::MODEL_MATERIAL_OVERRIDE& out)
{
    Engine::MODEL_MATERIAL_OVERRIDE staged;
    if (!resourceRoot.is_absolute() || !Validate_MaterialProfile(profile, &staged.surface.sourceCharacter)) return false;
    staged.materialName = profile.materialName;
    staged.surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
    for (const auto& texture : profile.textures)
    {
        auto& input = staged.sourceCharacterTextures[texture.expressionIndex];
        input.path = (resourceRoot / texture.assetId).lexically_normal();
        input.srgb = texture.srgb;
    }
    out = std::move(staged);
    return true;
}

bool_t CWorldSequenceDocument::Duplicate_TimelineBox(const std::string& sequenceId,
    const bool animation, const size_t index, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus)
{
    auto* current = Find_Template(sequenceId);
    if (!current) { outStatus = "Motion is unavailable: " + sequenceId; return false; }
    auto& sequence = *current;
    if ((animation && index >= sequence.animationTracks.size()) ||
        (!animation && index >= sequence.effectTracks.size())) return false;
    auto candidate = *this;
    auto* staged = candidate.Find_Template(sequence.sequenceId);
    if (!staged) return false;
    if (staged->tracks.size() + staged->animationTracks.size() + staged->effectTracks.size() + staged->colliderTracks.size() + staged->materialTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { outStatus = "Duplicate refused: Motion track limit reached. Existing draft preserved."; return false; }
    const uint32_t oldDuration = staged->durationMs;
    uint32_t duration = oldDuration;
    size_t selected = 0u;
    if (animation)
    {
        auto duplicate = staged->animationTracks[index];
        uint32_t end = oldDuration;
        for (const auto& next : staged->animationTracks)
            if (next.slotId == duplicate.slotId && next.startMs > duplicate.startMs) end = (std::min)(end, next.startMs);
        const uint32_t span = end - duplicate.startMs;
        if (oldDuration > CWorldSequenceDocument::MAX_DURATION_MS - span)
        { outStatus = "Duplicate refused: Animation exceeds the 600-second Motion limit."; return false; }
        for (auto& next : staged->animationTracks)
            if (next.slotId == duplicate.slotId && next.startMs >= end) next.startMs += span;
        duplicate.startMs = end;
        staged->animationTracks.insert(staged->animationTracks.begin() + index + 1u, duplicate);
        selected = index + 1u;
        duration += span;
    }
    else
    {
        auto duplicate = staged->effectTracks[index];
        const uint64_t start = uint64_t(staged->EffectStartMs(duplicate)) + duplicate.durationMs;
        if (start + duplicate.durationMs > CWorldSequenceDocument::MAX_DURATION_MS)
        { outStatus = "Duplicate refused: Effect exceeds the 600-second presentation limit."; return false; }
        uint32_t serial = 1u;
        do { duplicate.effectTrackId = "effect." + std::to_string(serial++); }
        while (std::any_of(staged->effectTracks.begin(), staged->effectTracks.end(),
            [&](const auto& row) { return row.effectTrackId == duplicate.effectTrackId; }));
        duplicate.timing = "TIME";
        duplicate.startMs = static_cast<uint32_t>(start);
        duration = (std::max)(duration, duplicate.startMs);
        staged->effectTracks.insert(staged->effectTracks.begin() + index + 1u, duplicate);
        selected = index + 1u;
    }
    if (duration > oldDuration)
    {
        for (auto& track : staged->tracks)
        {
            if (track.keys.empty() || track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { outStatus = "Duplicate refused: Motion endpoint cannot be extended. Existing draft preserved."; return false; }
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
        staged->durationMs = duration;
    }
    std::string status;
    if (!candidate.Validate(mapPlacements, deployPlacements, status))
    { outStatus = "Duplicate refused: " + status + ". Existing draft preserved."; return false; }
    // Preserve references held by the open Detail/Sequencer pane.
    sequence = std::move(*staged);
    outIndex = selected;
    outStatus = "Duplicated the selected box after its window.";
    return true;
}

bool_t CWorldSequenceDocument::Duplicate_ColliderTrack(const std::string& sequenceId,
    const size_t index, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus)
{
    auto* current = Find_Template(sequenceId);
    if (!current || index >= current->colliderTracks.size())
    { outStatus = "Collider row is unavailable: " + sequenceId; return false; }
    auto candidate = *this;
    auto* staged = candidate.Find_Template(sequenceId);
    if (staged->tracks.size() + staged->animationTracks.size() + staged->effectTracks.size() + staged->colliderTracks.size() + staged->materialTracks.size() >= MAX_TRACK_COUNT)
    { outStatus = "Duplicate refused: Motion track limit reached. Existing draft preserved."; return false; }
    auto duplicate = staged->colliderTracks[index];
    uint32_t serial = 1u;
    do { duplicate.colliderTrackId = "collider." + std::to_string(serial++); }
    while (std::any_of(staged->colliderTracks.begin(), staged->colliderTracks.end(),
        [&](const auto& row) { return row.colliderTrackId == duplicate.colliderTrackId; }));
    staged->colliderTracks.insert(staged->colliderTracks.begin() + index + 1u, duplicate);
    std::string status;
    if (!candidate.Validate(mapPlacements, deployPlacements, status))
    { outStatus = "Duplicate refused: " + status + ". Existing draft preserved."; return false; }
    *current = std::move(*staged);
    outIndex = index + 1u;
    outStatus = "Duplicated the collider with its original time window.";
    return true;
}

namespace
{
    bool Validate_ObjectBundle(const WORLD_SEQUENCE_OBJECT_BUNDLE& bundle, std::string& status)
    {
        const auto& resource = bundle.resource;
        if (resource.modelAssetId.empty() || !resource.sequenceInstanceId.empty() || !resource.motionInstanceIds.empty())
        { status = "Copy requires a model Object, not a placed alias or combined Motion group."; return false; }
        CWorldSequenceDocument projection;
        projection.Reset_Empty("clipboard.world.object");
        projection.Get_ObjectResources().push_back(resource);
        projection.Get_ObjectResources().front().parentId.clear();
        projection.Get_Templates() = bundle.templates;
        projection.Get_Instances() = bundle.instances;
        if (!projection.Validate({}, {}, status)) return false;
        std::unordered_set<std::string> rootIds, visited, templateIds;
        std::vector<std::string> pending = bundle.rootMotionIds;
        for (const auto& id : pending)
            if (!rootIds.insert(id).second || !projection.Find_Instance(id))
            { status = "Copied Motion roots must be unique and present in the bundle."; return false; }
        for (size_t index = 0; index < pending.size(); ++index)
        {
            const auto& id = pending[index];
            if (!visited.insert(id).second) continue;
            const auto* instance = projection.Find_Instance(id);
            if (!instance || instance->bindings.size() != 1u ||
                instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
                instance->bindings.front().targetId != resource.objectId || instance->anchorKind != resource.anchorKind)
            { status = "Every copied Motion must bind only the copied Object in its anchor category."; return false; }
            templateIds.insert(instance->templateId);
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
        }
        if (visited.size() != bundle.instances.size() || templateIds.size() != bundle.templates.size())
        { status = "Copied Motion bundle contains unreferenced instances or templates."; return false; }
        return true;
    }

    bool Is_UnboundObjectDraft(const CWorldSequenceDocument& document,
        const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
    {
        // Only the unfinished model selection produced by Create Object is exempt.
        // A malformed assigned model or a referenced resource still gets full validation.
        return resource.modelAssetId.empty() && resource.sequenceInstanceId.empty() &&
            resource.motionInstanceIds.empty() && resource.defaultMotionInstanceId.empty() &&
            resource.animationSetAssetId.empty() && resource.presentationBossArchetypeId.empty() &&
            resource.diffuseTextureAssetId.empty() && !resource.materialProfile && !resource.combatBody &&
            resource.materialSourceModelAssetId.empty() && resource.mapMaterialBindings.empty() && !resource.animated &&
            std::isfinite(resource.modelPreScale) && resource.modelPreScale >= MIN_SCALE && resource.modelPreScale <= MAX_COMPONENT &&
            Is_BoundedFloat3(resource.scale) && resource.scale.x >= MIN_SCALE && resource.scale.y >= MIN_SCALE && resource.scale.z >= MIN_SCALE &&
            std::none_of(document.Get_Instances().begin(), document.Get_Instances().end(), [&](const auto& instance) {
                return std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
                    return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
                });
            });
    }
}

bool_t CWorldSequenceDocument::Capture_ObjectBundle(const std::string& objectId,
    const std::vector<std::string>& selectedMotionIds, WORLD_SEQUENCE_OBJECT_BUNDLE& outBundle,
    std::string& outStatus) const
{
    const auto* resource = Find_ObjectResource(objectId);
    if (!resource || resource->modelAssetId.empty() || !resource->sequenceInstanceId.empty() || !resource->motionInstanceIds.empty())
    { outStatus = "Copy requires a model Object. Placed aliases and combined Motion groups keep their existing bindings."; return false; }
    WORLD_SEQUENCE_OBJECT_BUNDLE staged;
    staged.resource = *resource;
    staged.rootMotionIds = selectedMotionIds;
    if (selectedMotionIds.empty())
        for (const auto& instance : m_Instances)
            if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
                return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == objectId;
            })) staged.rootMotionIds.push_back(instance.instanceId);
    std::unordered_set<std::string> instanceIds, templateIds;
    std::vector<std::string> pending = staged.rootMotionIds;
    for (size_t index = 0; index < pending.size(); ++index)
    {
        const auto id = pending[index];
        if (!instanceIds.insert(id).second) continue;
        const auto* instance = Find_Instance(id);
        const auto* sequence = instance ? Find_Template(instance->templateId) : nullptr;
        if (!instance || !sequence || instance->bindings.size() != 1u ||
            instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
            instance->bindings.front().targetId != objectId || instance->anchorKind != resource->anchorKind)
        { outStatus = "Copy refused: every selected and NEXT Motion must bind this model Object only."; return false; }
        if (instanceIds.size() > MAX_INSTANCE_COUNT)
        { outStatus = "Copy refused: Motion bundle capacity reached."; return false; }
        staged.instances.push_back(*instance);
        if (templateIds.insert(sequence->sequenceId).second) staged.templates.push_back(*sequence);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
    }
    if (!selectedMotionIds.empty() && !instanceIds.contains(staged.resource.defaultMotionInstanceId))
    {
        staged.resource.defaultMotionInstanceId.clear();
        for (const auto& id : staged.rootMotionIds)
            if (const auto* instance = Find_Instance(id); instance && instance->enabled)
            { staged.resource.defaultMotionInstanceId = id; break; }
    }
    if (!Validate_ObjectBundle(staged, outStatus))
    { outStatus = "Copy refused: " + outStatus + " Existing clipboard preserved."; return false; }
    outBundle = std::move(staged);
    outStatus = "Copied the Object resource and " + std::to_string(outBundle.instances.size()) + " independent Motion values.";
    return true;
}

bool_t CWorldSequenceDocument::Paste_ObjectBundle(const WORLD_SEQUENCE_OBJECT_BUNDLE& bundle,
    const std::string& destinationObjectId, const std::string& newObjectName, const std::string& parentId,
    const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements, const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements,
    WORLD_SEQUENCE_PASTE_RESULT& outResult, std::string& outStatus)
{
    if (!Validate_ObjectBundle(bundle, outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    const bool createObject = destinationObjectId.empty();
    const auto* destination = createObject ? nullptr : Find_ObjectResource(destinationObjectId);
    if (!createObject && !destination)
    { outStatus = "Paste refused: destination Object is unavailable. Existing draft preserved."; return false; }
    if (m_Templates.size() + bundle.templates.size() > MAX_TEMPLATE_COUNT ||
        m_Instances.size() + bundle.instances.size() > MAX_INSTANCE_COUNT ||
        m_ObjectResources.size() + (createObject ? 1u : 0u) > MAX_INSTANCE_COUNT)
    { outStatus = "Paste refused: World sequence document capacity reached. Existing draft preserved."; return false; }
    const bool fillDraft = destination && Is_UnboundObjectDraft(*this, *destination);
    if (destination && (destination->anchorKind != bundle.resource.anchorKind ||
        !destination->sequenceInstanceId.empty() || !destination->motionInstanceIds.empty() ||
        (!fillDraft && (destination->modelAssetId != bundle.resource.modelAssetId ||
            destination->animationSetAssetId != bundle.resource.animationSetAssetId ||
            destination->animated != bundle.resource.animated ||
            destination->presentationBossArchetypeId != bundle.resource.presentationBossArchetypeId))))
    { outStatus = "Paste refused: destination requires the same model, animation set and anchor category. Existing draft preserved."; return false; }
    auto candidate = *this;
    WORLD_SEQUENCE_PASTE_RESULT result;
    result.objectId = destinationObjectId;
    if (createObject)
    {
        for (uint32_t serial = 1u; ; ++serial)
        {
            result.objectId = "world.object.copy." + std::to_string(serial);
            if (!candidate.Find_ObjectResource(result.objectId) && !candidate.Find_ObjectFolder(result.objectId)) break;
        }
        auto resource = bundle.resource;
        resource.objectId = result.objectId;
        resource.displayName = newObjectName;
        resource.parentId = parentId;
        candidate.Get_ObjectResources().push_back(std::move(resource));
    }
    else if (fillDraft)
    {
        auto resource = bundle.resource;
        resource.objectId = destination->objectId;
        resource.displayName = destination->displayName;
        resource.parentId = destination->parentId;
        *candidate.Find_ObjectResource(result.objectId) = std::move(resource);
    }
    std::unordered_map<std::string, std::string> templateIds, instanceIds;
    for (const auto& original : bundle.templates)
    {
        auto sequence = original;
        for (uint32_t serial = 1u; ; ++serial)
        {
            sequence.sequenceId = "world.object.motion.copy." + std::to_string(serial);
            if (!candidate.Find_Template(sequence.sequenceId)) break;
        }
        templateIds.emplace(original.sequenceId, sequence.sequenceId);
        candidate.Get_Templates().push_back(std::move(sequence));
    }
    for (const auto& original : bundle.instances)
    {
        auto instance = original;
        for (uint32_t serial = 1u; ; ++serial)
        {
            instance.instanceId = "world.object.instance.copy." + std::to_string(serial);
            if (!candidate.Find_Instance(instance.instanceId)) break;
        }
        instance.templateId = templateIds.at(original.templateId);
        instance.bindings.front().targetId = result.objectId;
        instanceIds.emplace(original.instanceId, instance.instanceId);
        result.instanceIds.push_back(instance.instanceId);
        candidate.Get_Instances().push_back(std::move(instance));
    }
    for (const auto& id : result.instanceIds)
    {
        auto* instance = candidate.Find_Instance(id);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            instance->nextMotionId = instanceIds.at(instance->nextMotionId);
    }
    for (const auto& id : bundle.rootMotionIds) result.rootMotionIds.push_back(instanceIds.at(id));
    auto* resource = candidate.Find_ObjectResource(result.objectId);
    if (createObject || fillDraft || resource->defaultMotionInstanceId.empty())
        resource->defaultMotionInstanceId = bundle.resource.defaultMotionInstanceId.empty() ? std::string{} :
            instanceIds.at(bundle.resource.defaultMotionInstanceId);
    if (!candidate.Validate_ObjectHierarchy(outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    // Other Create Object drafts remain visible and unchanged. Remove only those
    // unbound placeholders from a validation copy; Save still validates everything.
    auto validation = candidate;
    auto& resources = validation.Get_ObjectResources();
    resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const auto& row) {
        return row.objectId != result.objectId && Is_UnboundObjectDraft(candidate, row);
    }), resources.end());
    validation.Get_ObjectFolders().clear();
    for (auto& row : resources) row.parentId.clear();
    if (!validation.Validate(mapPlacements, deployPlacements, outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    *this = std::move(candidate);
    outResult = std::move(result);
    outStatus = "Pasted an independent Object/Motion copy. Save to keep the resource.";
    return true;
}
```

## Server/Public/GameRoom.h

```cpp
#pragma once

#include "RoomCommand.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "HonorTitleCatalog.h"
#include "VehicleCatalog.h"
#include "GuideCatalog.h"
#include "ValtanClearRewards.h"
#include "PlayerSkillSystem.h"
#include "CombatObjectRuntime.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "NpcBehaviorRuntime.h"
#include "ValtanBrain.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "Gameplay/EstherStrikeContract.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	/* A room never mutates an admitted gameplay catalog. The facade preserves
	   the established lookup surface while retaining immutable old generations
	   until every replicated occurrence releases its revision pin. */
	class CGameplayCatalogGenerations final
	{
	public:
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		CGameplayCatalogGenerations();
		bool Load();
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit(std::uint32_t transactionSequence) noexcept;
		void Abort(std::uint32_t transactionSequence) noexcept;
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept { return m_pActiveGeneration; }
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;
		[[nodiscard]] const std::string& Get_Status() const noexcept;
		operator const CGameplayCatalog&() const noexcept { return Active(); }

	private:
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;
		std::string m_strStatus;
	};

	// The last completed outer room-loop iteration, shared by all rooms on the next tick.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	/* Receive-thread admission is not a bool: a full reliable queue, a room
	   runtime failure, a sealed private arena, and cleanup already in flight
	   require different session policy and diagnostics.  Keep success values
	   explicit too so best-effort shedding remains non-terminal. */
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {});

		bool Enqueue(ROOM_COMMAND command);
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		// Room-thread only. At most one sampled line is retained until ServerApp writes it.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		/* Room-thread only. Stage is allowed to fail before publication; Commit
		   is a bounded pointer swap after every process room has staged. */
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}
		/* Room-thread only. Decision observability reads the same authoritative
		   brain and immutable selector generation as the Valtan simulation. */
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);
		// Room-thread only, while ServerApp holds its session-binding mutex.
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		void Mark_RuntimeFailure(std::string_view source);
		std::size_t Count_HumanPlayers() const;
		void Initialize_Guide();
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point) const;
		bool Invite_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);
		void Update_Guides(float seconds);
        float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor);
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);
		void Remove_Guide(std::uint32_t partyId, bool publish = true);
		void Queue_GuidePrompt(std::uint32_t partyId, const std::string& promptId);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);
		struct STAGED_PLAYER_ENTRY final
		{
			std::shared_ptr<CClientSession> pSession;
			SERVER_PLAYER Player;
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {});
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {});
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		/* A move goal inside the running skill's authored move-cancel window
		ends the action now instead of waiting out the recovery pose. */
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		/* Debug F1 Esther summon by name: same caster lock and summon timeline as
		the slot path, without the gauge or the world roster. Release ignores it. */
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		/* The caster the session owns, if it may start an Esther call right now:
		bound, idle, on its feet and not riding. */
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		/* Queues the summon forward along the aim and locks the caster into
		ESTHER_CAST. The gauge decision is the caller's. */
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		/* The landing of a square hole: the world's disabled "squarehole.<id>" movePlayer
		   row, admitted with the debug-teleport ground/height/collision rules. False when
		   the world has no such row or the landing is not standable for this player. */
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		/* The song lock ended: land the player at the destination, or leave them in place. */
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		/* Debug/Development-build test aid only -- zeroes the caster's own HP and
		sets PLAYER_ACTION_STATE::DEAD so a death-screen tester does not have to
		survive a real hit. Real body is compiled out in Release, matching
		Evaluate_ValtanAudition's convention. */
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		/* Debug-only Character Select audition entry. This stages the ordinary
		Server world-transfer transaction; it never changes a Client level directly. */
		void Handle_DebugEnterKakulSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& request);
		/* Debug-only authored waypoint audition. The placement must be a Kakul
		playerSpawn waypoint and Server navigation remains the position authority. */
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		/* Debug F1 clown/player avatar toggle: swaps only the replicated
		madness form of this session's player; Release answers REJECTED_DISABLED. */
		/* Debug F1 bingo check: paints cells and promotes completed lines. The
		board replicates on the world snapshot, so there is no result message. */
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		/* Debug F1 bingo bomb: marks this session's own player. The bomb
		rides the world snapshot, so there is no result message. */
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		/* Debug F1 bingo hammer: rolls one of the twenty row/column anchors
		and starts the sweep. The hammer rides the world snapshot. */
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		/* Debug F1 "Normal Monster 1/2" (Kouku Book1/Book2, Valtan Stage_1/Stage_2):
		removes the mapped wave group's live monsters, resets the group and starts it
		over at its authored anchors. Release ignores it; the monsters ride the world
		snapshot, so there is no result message. */
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		/* H key riding toggle for this session's player. The verdict is sent
		back; the ridden vehicle itself rides the world snapshot. */
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		/* True while nothing the player is doing forbids a vehicle underneath. */
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		/* Title window change for this session's player. The verdict is sent back; the
		worn title itself rides the world snapshot. */
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		/* Metres per second the player walks at: the ridden vehicle's speed, or
		the class speed scaled by its held stance. */
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		/* Dismounts every player the world, catalog or current state no longer
		lets ride. Runs once per tick before the snapshot is committed. */
		void Enforce_VehicleRidingState();
		/* Bern voyage ships (EFTable_VoyageShip 8200..8208) sail on the BernSea navigation region. Boarding
		   moves the player to the nearest open sea cell and keeps the pier position; leaving the ship, or any
		   forced dismount, brings the player back to that pier position. Begin returns false when no sea cell
		   lies within reach (the player is not at a harbour). */
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		/* A skill press while mounted. Only a skill of the ridden vehicle starts,
		from an idle mount, off cooldown and with a newer sequence; it faces the
		player's current yaw. */
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		/* Advances a running vehicle skill one fixed tick: authored root motion is
		clamped to walkable ground and collision, and the action ends at its length. */
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		/* One quick-slot press while this session's player shows an interaction
		HUD. DANCE answers the open pose window; the other modes only record the
		press until their skills own a Server judgement. */
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		/* Debug F1 "HUD Mode: MARIO / MAZE / Clear": forces one of the two
		modes whose gimmick has no Server trigger yet. Release answers
		REJECTED_DISABLED. */
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		/* Every tick: madness maximum from the encounter policy, clown hold
		expiry, and the interaction HUD mode plus slot layout per player. */
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		/* Debug-only. Moves a live Valtan onto an authored health-bar threshold
		so CValtanBrain judges the crossing itself on a later fixed tick. The
		room never starts a pattern, breaks a wall or plays a cue directly.
		Evaluate owns the decision and the boss mutation and is what the contract
		tests drive; Handle only resolves the session and answers it. */
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		/* The live arena boss a Debug audition scope names: the Gate 1 Kouku or
		a gate boss raised from a disabled placement. Null when that placement
		is not currently spawned. */
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		/* Broadcasts the cues a Logic tick produced, inserts follow-up patterns
		after the running audition slot, and returns true when a window asked
		the running pattern to end now (the brain commits it as COMPLETED). */
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// The entry root owns its portal across the completion-driven children.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// Pin the successful entrant, not whichever player is present later.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// Stage completion releases the actor clock, while these occurrence rows retain theirs.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			/* Global gameplay authority remains PinnedGameplayRevision. The
			   separate Kouku Product source owns pattern/logic rows for this run. */
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// Survives natural Pattern completion; reset/cancel and HP zero own removal.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// A live predecessor has no Client Play request to report a lifecycle for.
			bool bAdoptedLivePredecessor = false;
			// Keep only the current Flow occurrence on its existing ordered Brain path.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		/* A page start differs from a one-row timeline audition: it stages the
		already-destroyed arena, releases the real Brain at that page boundary,
		and then leaves the encounter running normally. */
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Validates itemId against the loaded catalog and stacks quantity into
		// player.Inventory, capped at maxStack and MAX_INVENTORY_ITEMS distinct
		// stacks. Returns false (no-op) for an unknown item or a full inventory
		// that would need a new stack. Shared by Handle_DebugGiveItem and the
		// Valtan clear-reward grant in the world entity tick loop.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// Debug-only. Validates the item against the loaded catalog and
		// stacks it into the player's inventory, capped at maxStack.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// Validates the item is owned, is a consumable (iHealPercent > 0), and
		// the player is alive; heals iMaximumHp * iHealPercent / 100, then
		// decrements/removes the stack. HP reaches the Client through the next
		// S2C_WORLD_SNAPSHOT tick like any other HP change; no separate result.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		/* Right-click equip / unequip. Checks the slot kind, the class and bag room,
		   then answers with the whole inventory whether or not anything moved. */
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		/* After a class change: items bound to another class go back to the bag. */
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// debug spawn buttons created in this room (Broadcast_WorldEntityDespawned per
		// entity) and resets the spawn group runtime so the same groups can be
		// re-activated. CHARACTER_SELECT_ARENA only; no-op reply for anything else.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		/* KoukuSaydon arena form of the Debug revert: removes only the entities
		raised from disabled bootstrap placements (the F1 gate buttons) and
		their dependents, keeping the statically enabled Gate 1 Kouku. */
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern's Valtan-entry confirm window (right-click a guide NPC). Replaces the
		// old automatic changeLevel triggerBox OBB fire: validates the requesting
		// player is still near the named guide NPC world entity, alive, and idle,
		// then stages the same SERVER_WORLD_TRANSFER_REQUEST the trigger used to
		// build. BERN only; no-op for anything else.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// The player pressed the key an interact-gated trigger box offered.
		// Names only the box; the trigger system re-tests that this player is
		// still standing in it before anything runs, so a stale or forged
		// request changes nothing.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// or party-leader gating (unlike Handle_ConfirmNpcEntry): any player in
		// a cleared Valtan/Kouku raid can return independently to its recorded entry guide.
		// Direct Lobby entries retain the default guide; NPC entries keep their source ID.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		/* Same-room-only: request.iTargetNetEntityId must resolve to a real
		   player currently in this room's m_PlayerIdByEntityId. There is no
		   cross-room player identity yet (nickname is display text only, see
		   CLAUDE.md), so an invite naming a player in a different room or a
		   stale/unknown NetEntityId is rejected, not queued. */
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		/* Commander raid gate progress (KoukuSaydon gates). The room marks a gate cleared
		   when its last primary boss dies (Notify_GateBossDeath from the world update),
		   the leader / solo player proposes to move on, members answer, and on
		   ALL_ACCEPTED Advance_Gate despawns the arena, raises the next gate's disabled
		   placements and moves every player to the gate position -- the product path of
		   what the Debug gate buttons do by hand. Implemented in GameRoom_GateProgress.cpp. */
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1-based, 0 = no gate raised yet
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // Nonzero pins a vote to its immutable raid run
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		// A primary boss is about to be removed DEAD: clears its gate when it was the last one.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// A gate placement came up (Debug button or Advance_Gate): that gate is now current.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		/* Raid-clear award input. Every fought primary boss advances its players' fight
		   clock each tick; a dying gate boss hands its ledger to the room, and the clear
		   sends the room ledger to every player and empties it. */
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId);
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		/* Tells every session in this room that an authored world sequence
		   instance started. Presentation only: the Server keeps no sequence
		   state, so a session that joins later simply misses a played edge.
		   False rejects the action without consuming its trigger or moving players. */
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		/* Offers or withdraws one interact-gated box for the one player it
		   concerns. Unlike the sequence broadcast this is never room-wide. */
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		/* The spawn-group activation every trigger path shares: starts a dormant
		   group, restarts a finished one once its monsters are gone, and never
		   stacks a wave on a group that is still running. */
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		/* What one trigger action does in this room, whether the box fired because
		   the player stepped in (a scripted flow) or pressed G inside it. */
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		/* Leave() calls this so a disconnecting player does not linger as a
		   ghost roster entry for whoever they partied with. */
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		/* Same room-scoped broadcast Broadcast_PartyRoster already uses --
		   every current session in this room receives the relayed line,
		   including the sender (its own head bubble is driven off the same
		   S2C_CHAT the rest of the room gets, not a second local-only path). */
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// Single-session send: inventory is per-player state, not room-shared
		// like S2C_WORLD_SNAPSHOT, so it never broadcasts.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				inventory);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// Server-owned collision/navigation counters carried by every
		// destruction message so the Debug audition panel never has to infer
		// passage from the replicated wall states.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		/* Runs only after every stage-action preflight transaction commits. These
		actions own player/target state and therefore cannot be staged inside the
		boss-combat or combat-object value transactions above. */
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		/* Commit the 69 ordinary contact walls and the 30 outer ring walls in one
		transaction, leaving every floor sector INTACT. A floor-collapse bar only
		arrives after the fight has already taken those walls down, so the
		audition for such a bar has to clear them inside the same atomic request
		instead of a second one the boss could start a pattern between. */
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		/* The navigation grid is the ground a boss pattern stride may cross.
		The collision sweep owns wall contact, while the furthest sample the grid
		still owns is what any stride is allowed to reach, so a charge cannot
		leave the floor before its wall contact is evaluated. A start the grid
		already refuses passes through
		unchanged, because refusing it there would strand the boss for good. */
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever a non-impact boss body physically reached between its
		previous and current position. A charge-impact stage bypasses this generic
		pass and owns one exact swept wall transaction: impact receiver first,
		then the co-located ordinary contact binding. */
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		/* Card maze. The telescope claim deals the suits and raises the
		targets; the MAZE hammer press judges its swing once, at the runtime's
		hit tick, against those targets. */
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		/* Before the run: raises the clown box for players inside the maze and
		latches its destruction. Clear forgets it and removes a living box. */
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		/* Advances the bingo bomb clock: a mark whose deadline passed is
		planted where its carrier stands, and a carrier that left the room
		takes its mark with it. */
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		/* Hands the living monster and boss bodies to the collision system so this
		tick's player walks and root motion stop at them. */
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		/* Product boss-pattern adapters call these with replicated identities. The
		room owns interruption, fixed-tick fallback motion and release reaction so
		no pattern can leave half of a grabbed player state behind. */
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		// Best-effort traffic leaves room for gameplay/control commands. LEAVE
		// never shares this bounded queue: disconnect cleanup has its own
		// session-deduplicated priority queue below.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		struct GUIDE_RUNTIME
		{
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;
			std::string ComboId, PendingComboId, Reason;
			std::size_t ComboStep = 0;
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f, FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;
			std::uint32_t Sequence = 0, EventSequence = 0;
            std::map<std::string, std::uint32_t> CommandTicks;
			std::uint8_t Action = 0;
            float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;
			std::deque<std::pair<std::string, std::size_t>> PromptQueue;
			std::map<std::string, std::uint32_t> TriggerTicks;
			std::unordered_set<std::string> InsideBoxes;
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};
		CGuideCatalog m_GuideCatalog;
		std::map<std::uint32_t, GUIDE_RUNTIME> m_Guides;
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;
        LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		/* Grants what a started skill buffs, to the caster, the party in this room
		or the entities it targets. */
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		/* Same-room party state -- PLAYER_ID is room-local (freshly allocated
		   per room on Join), so this map does not by itself survive a member
		   moving to a different room. 0 means "no party" -- never a real party
		   ID. Invite/accept/join only; leave/kick/leader promotion is a
		   separate follow-up.
		   A party-leader-triggered group Valtan entry (Handle_ConfirmNpcEntry
		   -> Transfer_PartyTo) is the one case
		   that does survive a room change: every member transfers together in
		   one batch and gets re-grouped into a fresh room-local party in the
		   target room, so the party itself is never actually split across two
		   rooms at once. There is still no general cross-room party identity
		   (e.g. inviting or chatting with someone in a different room). */
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// One pending invite per target at a time; a new invite silently
		// replaces whatever that target's last unanswered invite was.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// At most one latest failure per present player. A full reliable queue
		// delays the notice instead of disconnecting a rejected source party.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		GATE_PROGRESS_STATE m_GateProgress;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
		// One room-wide scheduled intro, retained for late join until the room empties.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		// Popped source-ball slots per Mario stage (index 1..4), bit = bootstrap slot.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		/* The four pillars come back four times in one fight, so they live in a
		reversible prop runtime instead of a one-way destruction group. */
		CEncounterPropRuntime m_EncounterPropRuntime;
		/* Room-authoritative completion latch. The primary Product Valtan death
		   raises it before that entity is reliably despawned; the last-player reset
		   clears it for the next party. */
		bool m_bValtanRaidCleared = false;
		/* Debug audition only: the tick a whole pillar cycle shatters on, and
		the flag the next raise turns into that tick. No product trigger for the
		shatter is identified yet, so nothing else writes these. */
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		/* Damage-text events raised while draining room commands, which happens before
		m_TickDamageEvents is cleared for the tick. Moved in right after that clear so a
		potion heal reaches the same broadcast as a combat hit. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. Stable-ID pattern requests have an independent ledger because
		the Effect Tool and the Valtan level own independent sequence counters. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			/* A QUEUED receipt must remain reconcilable after its occurrence is no
			   longer the room's current audition. Keep the last authoritative edge
			   so an exact retry cannot loop on a verdict without lifecycle. */
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		/* Stable-ID Play/Restart keeps the exact payload and verdict. A retry of
		   one identity replays that verdict; an altered tuple never inherits it. */
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			/* Exact Start retries replay the latest authoritative edge for that
			   admitted program. This settles an unconfirmed Client even when the
			   Flow already reached COMPLETED_HOLD; it never starts a second run. */
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
```

## Server/Private/GameRoom_PartyWorld.cpp

```cpp
#include "GameRoom.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_ReturnToBern(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RETURN_TO_BERN& request)
{
	using namespace LostArk::Shared;
	// Direct Lobby/debug entries have no source NPC; retain their established exit.
	constexpr const char* BERN_RETURN_PLACEMENT_ID = "npc.bern.beda.guide";

	/* Valtan after its clear; KoukuSaydon after its last gate cleared (the gate progress
	   widget's exit button). */
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_bValtanRaidCleared && !Is_RaidClearTestModeEnabled())
			return;
	}
	else if (WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId)
	{
		if (0u == Gate_Count() || 0u == (m_GateProgress.iClearedMask & (1u << (Gate_Count() - 1u))))
			return;
	}
	else
		return;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& player = playerIter->second;
	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return;
	}

	// Solo only -- unlike Handle_ConfirmNpcEntry, returning is never batched
	// across a party. Each player presses their own button independently.
	const bool isAlreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId;
		});
	if (isAlreadyStaged)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = WORLD_ID::BERN;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iHonorTitleId = player.iHonorTitleId;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	transfer.strSpawnPlacementOverrideId = player.strRaidReturnNpcPlacementId.empty() ?
		BERN_RETURN_PLACEMENT_ID : player.strRaidReturnNpcPlacementId;
	// Carries Valtan clear rewards (and anything else still held) across the
	// trip -- without this, Stage_PlayerEntry's default fresh-entry grant would
	// silently reset the player back to just 3 starting potions.
	transfer.CarriedInventory = player.Inventory;
    if (const auto party = m_PartyIdByPlayerId.find(player.iPlayerId); party != m_PartyIdByPlayerId.end())
        if (const auto companion = m_Guides.find(party->second); companion != m_Guides.end() &&
            (companion->second.AnchorId == player.iPlayerId || m_PartyMembersByPartyId.at(party->second).size() == 1u))
            transfer.PartyBatchSessionIds.push_back(sessionId);
	m_PendingWorldTransfers.push_back(std::move(transfer));
}

void LostArk::Server::CGameRoom::Handle_PartyInvite(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID inviterId = sessionIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	const SERVER_PLAYER& inviter = inviterIter->second;
	if (Invite_Guide(inviter, request.iTargetNetEntityId)) return;

	const auto targetPlayerIdIter =
		m_PlayerIdByEntityId.find(request.iTargetNetEntityId);
	if (targetPlayerIdIter == m_PlayerIdByEntityId.end() ||
		targetPlayerIdIter->second == inviterId)
	{
		return;
	}
	const PLAYER_ID targetId = targetPlayerIdIter->second;
	const auto targetIter = m_Players.find(targetId);
	if (targetIter == m_Players.end())
		return;
	const SERVER_PLAYER& target = targetIter->second;

	const auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	const std::uint32_t inviterPartyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	const auto targetPartyIter = m_PartyIdByPlayerId.find(targetId);
	if (targetPartyIter != m_PartyIdByPlayerId.end())
	{
		// Already partied together, or target belongs to a different party --
		// merging two existing parties is not supported yet either way.
		return;
	}
	if (0u != inviterPartyId)
	{
		const auto membersIter = m_PartyMembersByPartyId.find(inviterPartyId);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() >= MAX_PARTY_MEMBERS)
		{
			return;
		}
	}

	// A new invite silently replaces whatever this target's last unanswered
	// invite was -- only one can ever be outstanding per target.
	m_PendingPartyInviteByTargetPlayerId[targetId] = inviterId;

	const std::shared_ptr<CClientSession> targetSession =
		Find_Session(target.iSessionId);
	if (nullptr == targetSession)
		return;
	S2C_PARTY_INVITE_RECEIVED message{};
	message.iFromNetEntityId = inviter.iNetEntityId;
	message.strFromNickname = inviter.strNickName;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!targetSession->Send_Frame(
			PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED, writer.Get_Buffer()))
	{
		targetSession->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_PartyInviteRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto pendingIter =
		m_PendingPartyInviteByTargetPlayerId.find(responderId);
	if (pendingIter == m_PendingPartyInviteByTargetPlayerId.end())
		return;
	const PLAYER_ID inviterId = pendingIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	if (inviterIter->second.iNetEntityId != request.iFromNetEntityId)
		return;
	// A response to a replaced invite must not consume the current invite,
	// regardless of whether that stale response accepts or declines.
	m_PendingPartyInviteByTargetPlayerId.erase(pendingIter);
	if (!request.bAccepted)
		return;
	if (m_Players.find(responderId) == m_Players.end())
		return;
	// Re-check both invariants Handle_PartyInvite validated -- state may have
	// changed while this invite was outstanding.
	if (m_PartyIdByPlayerId.find(responderId) != m_PartyIdByPlayerId.end())
		return;

	auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	std::uint32_t partyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	if (0u == partyId)
	{
		partyId = m_iNextPartyId++;
		m_PartyMembersByPartyId[partyId] = { inviterId };
		m_PartyIdByPlayerId[inviterId] = partyId;
	}
	else if (m_PartyMembersByPartyId[partyId].size() >= MAX_PARTY_MEMBERS)
	{
		return;
	}
	m_PartyMembersByPartyId[partyId].push_back(responderId);
	m_PartyIdByPlayerId[responderId] = partyId;

	Broadcast_PartyRoster(partyId);
}

void LostArk::Server::CGameRoom::Broadcast_PartyRoster(
	const std::uint32_t partyId)
{
	using namespace LostArk::Shared;

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;

	S2C_PARTY_ROSTER message{};
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		PARTY_ROSTER_MEMBER member{};
		member.iNetEntityId = playerIter->second.iNetEntityId;
		member.strNickname = playerIter->second.strNickName;
		member.eCharacterClass = playerIter->second.eCharacterClass;
		message.Members.push_back(std::move(member));
	}
	if (const auto companion = m_Guides.find(partyId); companion != m_Guides.end())
	{
		const auto actor = m_Players.find(companion->second.PlayerId);
		if (actor != m_Players.end()) message.GuideCompanion = PARTY_ROSTER_MEMBER{actor->second.iNetEntityId, actor->second.strNickName, actor->second.eCharacterClass, actor->second.eControlKind};
	}
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_PARTY_ROSTER, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Send_InteractPrompt(
	const SERVER_INTERACT_PROMPT_EDGE& edge)
{
	using namespace LostArk::Shared;

	const auto player = m_Players.find(edge.iPlayerId);
	if (m_Players.end() == player)
		return;
	const std::shared_ptr<CClientSession> session =
		Find_Session(player->second.iSessionId);
	if (nullptr == session)
		return;
	S2C_INTERACT_PROMPT message{};
	message.strTriggerPlacementId = edge.strTriggerPlacementId;
	message.bAvailable = edge.bAvailable;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!session->Send_Frame(
		PACKET_TYPE::S2C_INTERACT_PROMPT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Activate_SpawnGroupFromTrigger(
	const std::string& spawnGroupId)
{
	return m_SpawnGroupRuntime.Activate_Repeat(
		spawnGroupId,
		[this](const std::string& id)
		{
			return Count_SpawnGroupEntities(id);
		});
}

void LostArk::Server::CGameRoom::Handle_DebugResummonWaveMonsters(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	const auto report = [this](std::string line)
	{
		m_strStatus = std::move(line);
		std::cout << "[WaveMonsters] " << m_strStatus << '\n';
	};
	/* The request names this room's own world, that world has a button for it, and
	the session owns a player here. The Valtan pattern audition owns its arena while
	it runs (the trigger boxes are not evaluated then either), so the wave waits. */
	const WAVE_MONSTER_BUTTON_ROW* row =
		CServerTriggerSystem::Find_WaveMonsterButton(m_eWorldId, request.eButton);
	if (request.eWorldId != m_eWorldId || nullptr == row ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		report("Wave monster re-summon refused: wrong world, no such button or no player in this room");
		return;
	}
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase)
	{
		report("Wave monster re-summon refused: the Valtan pattern audition is running");
		return;
	}
	const std::string groupId = row->pSpawnGroupId;
	const auto& groups = m_SpawnGroupBootstrap.Get_Groups();
	if (std::none_of(groups.begin(), groups.end(),
		[&groupId](const SPAWN_GROUP_DEFINITION& definition)
		{
			return definition.strSpawnGroupId == groupId;
		}))
	{
		report("Wave monster re-summon refused: spawn group is missing: " + groupId);
		return;
	}
	/* Remove what is still alive, then start the group over from its first wave. The
	monsters appear on the next spawn-group update at the group's authored anchors,
	wherever the player stands. */
	for (auto entity = m_WorldEntities.begin(); entity != m_WorldEntities.end();)
	{
		if (WORLD_BOOTSTRAP_KIND::MONSTER != entity->eKind ||
			entity->strSpawnGroupId != groupId)
		{
			++entity;
			continue;
		}
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		entity = m_WorldEntities.erase(entity);
	}
	if (!Broadcast_CombatObjectLifecycle())
	{
		Mark_RuntimeFailure("wave-resummon.combat-object-lifecycle");
		return;
	}
	if (!m_SpawnGroupRuntime.Reset_Group(groupId) ||
		!m_SpawnGroupRuntime.Activate(groupId))
	{
		report("Wave monster re-summon failed to restart the group: " + groupId);
		return;
	}
	report("Wave monsters re-summoned: " + groupId);
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Handle_InteractTrigger(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_INTERACT_TRIGGER& request)
{
	using namespace LostArk::Shared;

	const auto playerId = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == playerId)
		return;
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	const std::uint32_t actionTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	const auto activateTarget = [this](const WORLD_TRIGGER_ACTION_KIND kind,
		const std::string& targetId)
	{
		return Activate_TriggerTarget(kind, targetId);
	};
	/* Mario crossings and exits wait for G now, so the request must reach the same
	   room-owned admission the tick uses for a stepped-in entry (stage, authority
	   locks, contact interruption, the terminal exit's return destination). */
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER moveEntry =
		[this](const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			const std::uint32_t actionStartTick)
	{
		return Begin_MarioTriggerMove(trigger, player, actionStartTick);
	};
	/* G with no box on offer names none (INTERACT_TRIGGER_HERE_ID): the Server
	   decides which boxes the player is standing in and runs those. */
	const bool answersHere =
		LostArk::Shared::INTERACT_TRIGGER_HERE_ID == request.strTriggerPlacementId;
	if (answersHere
		? 0u == m_ServerTriggerSystem.Activate_Here(
			playerId->second, m_Players, actionTick, transfers, activateTarget, moveEntry)
		: !m_ServerTriggerSystem.Activate_Interact(
			playerId->second,
			request.strTriggerPlacementId,
			m_Players,
			actionTick,
			transfers,
			activateTarget,
			moveEntry))
	{
		return;
	}
	/* A gated box can move worlds like any other, so its transfer is staged
	   through the same pending list the tick uses. */
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[staged = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == staged;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	/* The offer stays: every box is repeatable and the player is still inside
	   it. Walking out withdraws it (Evaluate_Entries). */
}

void LostArk::Server::CGameRoom::Handle_DebugWorldPlayback(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_WORLD_PLAYBACK_RESULT result{ request.iRequestSequence, request.eWorldId,
		request.eOperation, DEBUG_WORLD_PLAYBACK_RESULT::DISABLED, request.strTargetId };
#ifdef _DEBUG
	const auto execute = [&]() -> DEBUG_WORLD_PLAYBACK_RESULT
	{
		using Result = DEBUG_WORLD_PLAYBACK_RESULT;
		using Op = DEBUG_WORLD_PLAYBACK_OPERATION;
		if (request.eWorldId != m_eWorldId ||
			(m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA && m_eWorldId != WORLD_ID::VALTAN_ARENA))
			return Result::WRONG_WORLD;
		const auto playerId = m_PlayerIdBySessionId.find(sessionId);
		if (playerId == m_PlayerIdBySessionId.end()) return Result::INVALID_PLAYER;
		const auto player = m_Players.find(playerId->second);
		if (player == m_Players.end() || !player->second.iCurrentHp) return Result::INVALID_PLAYER;
		auto& last = m_WorldPlaybackRequestSequences[sessionId];
		if (request.iRequestSequence <= last) return Result::STALE_REQUEST;
		last = request.iRequestSequence;
		if (request.eOperation == Op::PLACE_ROOM_PLAYER) return Apply_DebugRoomPlayerArrival(sessionId, request);
		const bool replay = request.eOperation == Op::REPLAY_TRIGGER || request.eOperation == Op::REPLAY_SEQUENCE;
		const auto play = [&](const std::string& id)
		{
			const auto& ids = m_WorldBootstrap.Get_SequenceInstanceIds();
			if (std::find(ids.begin(), ids.end(), id) == ids.end()) return false;
			return Broadcast_WorldSequencePlay(id, 1.f, 0.f, 0.f, 0.f, 0u, {},
				replay ? WORLD_SEQUENCE_OPERATION::REPLAY : WORLD_SEQUENCE_OPERATION::PLAY);
		};
		if (request.eOperation == Op::PLAY_TRIGGER || request.eOperation == Op::REPLAY_TRIGGER)
		{
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			const auto verdict = m_ServerTriggerSystem.Debug_Activate(playerId->second, request.strTargetId,
				replay, m_Players, m_iServerTick ? m_iServerTick : 1u, transfers,
				[&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
				{
					if (kind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE) return play(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP) return Activate_SpawnGroupFromTrigger(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER) return Activate_Encounter(id);
					if (kind == WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE) return Begin_CardMaze(playerId->second);
					return false;
				});
			for (auto& transfer : transfers)
			{
				if (std::none_of(m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
					[&](const auto& pending) { return pending.iSessionId == transfer.iSessionId; }))
					m_PendingWorldTransfers.push_back(std::move(transfer));
			}
			return verdict;
		}
		const auto& ids = m_WorldBootstrap.Get_SequenceInstanceIds();
		if (std::find(ids.begin(), ids.end(), request.strTargetId) == ids.end()) return Result::INVALID_TARGET;
		if (request.eOperation == Op::STOP_SEQUENCE)
			Broadcast_WorldSequencePlay(request.strTargetId, 1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
		else if (!play(request.strTargetId)) return Result::INVALID_TARGET;
		return Result::ACCEPTED;
	};
	result.eResult = execute();
#endif
	const auto session = Find_Session(sessionId);
	CPacketWriter writer;
	if (session && Write_Message(writer, result) &&
		!session->Send_Frame(PACKET_TYPE::S2C_DEBUG_WORLD_PLAYBACK_RESULT, writer.Get_Buffer()))
		session->Request_Close();
}

LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT LostArk::Server::CGameRoom::Apply_DebugRoomPlayerArrival(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request)
{
	using namespace LostArk::Shared;
	using Result = DEBUG_WORLD_PLAYBACK_RESULT;
#ifndef _DEBUG
	(void)sessionId; (void)request;
	return Result::DISABLED;
#else
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || request.eWorldId != m_eWorldId)
		return Result::WRONG_WORLD;
	CPacketWriter validation;
	if (request.eOperation != DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER || !Write_Message(validation, request) ||
		!request.strTargetId.starts_with("KAKULSAYDON_G1_PATTERN_") ||
		!request.strOccurrenceId.starts_with(request.strTargetId + ".")) return Result::INVALID_TARGET;
	const auto requesterId = m_PlayerIdBySessionId.find(sessionId);
	const auto requester = requesterId == m_PlayerIdBySessionId.end() ? m_Players.end() : m_Players.find(requesterId->second);
	const auto requesterSession = Find_Session(sessionId);
	if (requester == m_Players.end() || requester->second.iSessionId != sessionId || !requester->second.iCurrentHp ||
		!requesterSession || !requesterSession->Is_Open() || requesterSession->Is_Closing()) return Result::INVALID_PLAYER;
	auto& run = m_RoomPlayerArrivalRuns[sessionId];
	if (request.iRunEpoch < run.iEpoch) return Result::STALE_REQUEST;
	if (request.iRunEpoch == run.iEpoch && request.strTargetId != run.strRootPatternId) return Result::INVALID_TARGET;
	if (request.iRunEpoch > run.iEpoch)
	{
		ROOM_PLAYER_ARRIVAL_RUN staged;
		staged.iEpoch = request.iRunEpoch; staged.strRootPatternId = request.strTargetId;
		// m_Players is ordered by stable PlayerId. Never resolve a slot again after leave/join.
		for (const auto& [id, player] : m_Players)
		{
			const auto binding = m_PlayerIdBySessionId.find(player.iSessionId);
			const auto connection = Find_Session(player.iSessionId);
			if (binding != m_PlayerIdBySessionId.end() && binding->second == id && connection && connection->Is_Open() && !connection->Is_Closing())
				staged.Players.emplace_back(id, player.iSessionId);
			if (staged.Players.size() == 4u) break;
		}
		run = std::move(staged);
	}
	if (const auto done = run.Occurrences.find(request.strOccurrenceId); done != run.Occurrences.end())
		return done->second == Result::ACCEPTED ? Result::ALREADY_USED : done->second;
	if (run.Occurrences.size() >= 128u) return Result::INVALID_TARGET;
	const auto finish = [&](Result result) { run.Occurrences.emplace(request.strOccurrenceId, result); return result; };
	if (request.iRoomPlayerSlot >= run.Players.size()) return finish(Result::SKIPPED_PLAYER);
	const auto [targetId, targetSessionId] = run.Players[request.iRoomPlayerSlot];
	const auto target = m_Players.find(targetId);
	const auto binding = m_PlayerIdBySessionId.find(targetSessionId);
	const auto connection = Find_Session(targetSessionId);
	if (target == m_Players.end() || target->second.iSessionId != targetSessionId ||
		binding == m_PlayerIdBySessionId.end() || binding->second != targetId || !connection || !connection->Is_Open() || connection->Is_Closing())
		return finish(Result::SKIPPED_PLAYER);
	C2S_DEBUG_TELEPORT_TO_POSITION position{};
	position.eWorldId = m_eWorldId;
	position.fPositionX = request.fPositionX; position.fPositionY = request.fPositionY; position.fPositionZ = request.fPositionZ;
	SERVER_NAV_POINT ground{};
	const auto verdict = Validate_DebugTeleportDestination(target->second, position, ground);
	if (verdict != DEBUG_TELEPORT_RESULT::ACCEPTED)
	{
		m_strStatus = "Sequence room player arrival rejected by teleport validation: " + std::to_string(static_cast<unsigned>(verdict));
		return finish(verdict == DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE ? Result::INVALID_PLAYER : Result::ACTION_REJECTED);
	}
	// The same teleport validator accepted this participant and destination before live state changes.
	Reset_PlayerForDebugTeleport(target->second);
	target->second.fPositionX = ground.x; target->second.fPositionY = ground.y; target->second.fPositionZ = ground.z;
	Update_MarioControlState(target->second);
	m_strStatus = "Sequence room player arrival committed for slot " + std::to_string(request.iRoomPlayerSlot + 1u);
	return finish(Result::ACCEPTED);
#endif
}

bool LostArk::Server::CGameRoom::Broadcast_WorldSequencePlay(
	const std::string& instanceId,
	const float playbackSpeed, const float positionOffsetX,
	const float positionOffsetY, const float positionOffsetZ, const std::uint32_t durationMs,
	const std::string& targetSequenceInstanceId,
	const LostArk::Shared::WORLD_SEQUENCE_OPERATION operation)
{
	using namespace LostArk::Shared;

	// A published raid entry replaces its first World-only cutscene with the
	// complete authored Sequence. All other world playback keeps its own path.
	if (m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA &&
		(operation == WORLD_SEQUENCE_OPERATION::PLAY || operation == WORLD_SEQUENCE_OPERATION::REPLAY))
	{
		const auto* gate = m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1");
		if (gate && !gate->strEntrySequenceInstanceId.empty() && gate->strEntrySequenceInstanceId == instanceId)
		{
			if (Is_KoukuRaidRunning() || m_Players.empty()) return false;
			const SESSION_ID owner = m_Players.begin()->second.iSessionId;
			C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
			request.eWorldId = m_eWorldId; request.eOperation = KOUKUSAYDON_RAID_OPERATION::START;
			const auto previous = m_KoukuRaidReceipts.find(owner);
			const auto priorSequence = previous == m_KoukuRaidReceipts.end() ? 0u : previous->second.first.iRequestSequence;
			if (priorSequence == (std::numeric_limits<std::uint32_t>::max)()) return false;
			request.iRequestSequence = priorSequence + 1u;
			request.strStartGateId = gate->strGateId;
			request.ExpectedGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
			request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(m_GameplayCatalog.Active());
			request.iSequenceSourceRevision = gate->iSequenceRevision;
			std::string reason;
			if (!Begin_KoukuRaidPreparation(owner, request, reason)) { m_strStatus = reason; return false; }
			m_KoukuRaid.strEntryTriggerSequenceId = instanceId;
			return true;
		}
	}

	// The authored Pattern owns Saydon. Reject the retired trigger before any
	// player mutation or broadcast; STOP remains valid for stale-client cleanup.
	if ((operation == WORLD_SEQUENCE_OPERATION::PLAY || operation == WORLD_SEQUENCE_OPERATION::REPLAY) &&
		(instanceId == "world.sequence.instance.original_kouku" ||
		 targetSequenceInstanceId == "world.sequence.instance.original_kouku"))
	{
		m_strStatus = "Legacy Saydon cutscene is retired; use the authored Sequence Pattern";
		return false;
	}

	S2C_WORLD_SEQUENCE_PLAY message{};
	const bool waterpang = m_eWorldId == WORLD_ID::MAHARAKA && instanceId == MAHARAKA_WATERPANG_INTRO_INSTANCE;
	if (waterpang)
	{
		// Re-entry and debug replay cannot restart an in-progress arena session.
		if (operation != WORLD_SEQUENCE_OPERATION::PLAY) return false;
		if (m_MaharakaWaterpangIntro) return true;
		message.iServerTick = m_iServerTick;
		message.iStartTick = m_iServerTick + MAHARAKA_WATERPANG_COUNTDOWN_TICKS;
		if (!message.iStartTick) ++message.iStartTick;
	}
	message.eOperation = operation;
	message.strSequenceInstanceId = instanceId;
	message.strTargetSequenceInstanceId = targetSequenceInstanceId;
	message.iDurationMs = durationMs;
	message.fPlaybackSpeed = playbackSpeed;
	message.fPositionOffsetX = positionOffsetX;
	message.fPositionOffsetY = positionOffsetY;
	message.fPositionOffsetZ = positionOffsetZ;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	if (waterpang) m_MaharakaWaterpangIntro = message;
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Remove_FromParty(
	const LostArk::Shared::PLAYER_ID playerId)
{
	const auto partyIdIter = m_PartyIdByPlayerId.find(playerId);
	if (partyIdIter == m_PartyIdByPlayerId.end())
		return;
	const std::uint32_t partyId = partyIdIter->second;
	m_PartyIdByPlayerId.erase(partyIdIter);

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;
	std::vector<LostArk::Shared::PLAYER_ID>& members = membersIter->second;
	members.erase(
		std::remove(members.begin(), members.end(), playerId),
		members.end());
	if (members.empty())
	{
		Remove_Guide(partyId);
		m_PartyMembersByPartyId.erase(membersIter);
		return;
	}
	Broadcast_PartyRoster(partyId);
}

bool LostArk::Server::CGameRoom::Is_PlayerNearValtanEntryNpc(
	const SERVER_PLAYER& player, const std::string& npcPlacementId) const
{
	using namespace LostArk::Shared;
	// Handle_ConfirmNpcEntry의 VALTAN_ENTRY_GUIDE_NPCS와 같은 placement 집합. 여기서는
	// proximity만 검증하고, target world는 NPC가 아니라 propose의 eTarget이 소유한다.
	static constexpr const char* GUIDE_NPC_PLACEMENT_IDS[] = {
		"npc.bern.beda.guide", "npc.bern.aylara" };
	constexpr float INTERACTION_RADIUS = 3.f;
	const bool isGuide = std::any_of(
		std::begin(GUIDE_NPC_PLACEMENT_IDS), std::end(GUIDE_NPC_PLACEMENT_IDS),
		[&npcPlacementId](const char* id) { return npcPlacementId == id; });
	if (!isGuide)
		return false;
	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&npcPlacementId](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == npcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
		return false;
	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	return deltaX * deltaX + deltaZ * deltaZ <=
		INTERACTION_RADIUS * INTERACTION_RADIUS;
}

bool LostArk::Server::CGameRoom::Stage_PartyWorldTransfer(
	const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId)
{
	using namespace LostArk::Shared;
	if (batchMemberIds.empty())
		return false;
	const auto leaderIter = m_Players.find(batchMemberIds.front());
	if (leaderIter == m_Players.end())
		return false;
	const SERVER_PLAYER& leader = leaderIter->second;

	const auto isAlreadyStaged = [this](SESSION_ID sid)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sid](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sid ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sid) !=
						pending.PartyBatchSessionIds.end();
			});
	};

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = leader.iSessionId;
	transfer.eTargetWorldId = targetWorldId;
	transfer.strRaidReturnNpcPlacementId = raidReturnNpcPlacementId;
	transfer.eCharacterClass = leader.eCharacterClass;
	transfer.strNickName = leader.strNickName;
	transfer.iHonorTitleId = leader.iHonorTitleId;
	transfer.iPartyRequestSequence = requestSequence;
	for (const PLAYER_ID memberId : batchMemberIds)
	{
		const auto memberIter = m_Players.find(memberId);
		if (memberIter == m_Players.end() ||
			CHARACTER_CLASS_ID::END == memberIter->second.eCharacterClass ||
			memberIter->second.strNickName.empty() ||
			isAlreadyStaged(memberIter->second.iSessionId))
		{
			return false;
		}
		if (batchMemberIds.size() > 1u || (m_PartyIdByPlayerId.contains(leader.iPlayerId) && m_Guides.contains(m_PartyIdByPlayerId.at(leader.iPlayerId))))
			transfer.PartyBatchSessionIds.push_back(memberIter->second.iSessionId);
	}
	m_PendingWorldTransfers.push_back(std::move(transfer));
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_RaidEntryVote(
	const RAID_ENTRY_PROPOSAL& proposal, const bool bClosed,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	S2C_RAID_ENTRY_VOTE message{};
	message.iProposalId = proposal.iProposalId;
	message.iAccepted = static_cast<std::uint8_t>(proposal.Accepted.size());
	message.iTotal = static_cast<std::uint8_t>(proposal.Voters.size());
	message.bClosed = bClosed;
	message.eResult = bClosed ? result : RAID_ENTRY_VOTE_RESULT::END;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_VOTE, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_RaidEntryPropose(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request)
{
	using namespace LostArk::Shared;
	// 30Hz 기준 30초 미응답이면 tick 루프가 TIMEOUT으로 닫는다.
	constexpr std::uint32_t VOTE_TIMEOUT_TICKS = 30u * 30u;

	if (WORLD_ID::BERN != m_eWorldId || request.eTarget >= RAID_ENTRY_TARGET::END)
		return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID proposerId = sessionIter->second;
	const auto playerIter = m_Players.find(proposerId);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& proposer = playerIter->second;
	if (0u == proposer.iCurrentHp || PLAYER_ACTION_STATE::NONE != proposer.eAction ||
		INVALID_SESSION_ID == proposer.iSessionId ||
		CHARACTER_CLASS_ID::END == proposer.eCharacterClass ||
		proposer.strNickName.empty())
	{
		return;
	}
	if (!Is_PlayerNearValtanEntryNpc(proposer, request.strNpcPlacementId))
		return;

	// 한 플레이어는 동시에 하나의 열린 proposal에만 속한다.
	const bool alreadyInVote = std::any_of(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[proposerId](const RAID_ENTRY_PROPOSAL& p)
		{
			return std::find(p.Voters.begin(), p.Voters.end(), proposerId) !=
				p.Voters.end();
		});
	if (alreadyInVote)
		return;

	std::uint32_t partyId = 0u;
	std::vector<PLAYER_ID> voters{ proposerId };
	const auto partyIdIter = m_PartyIdByPlayerId.find(proposerId);
	if (partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter = m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() > 1u)
		{
			// 파티 발의는 리더(members.front())만 가능. 비리더는 조용히 거절한다
			// (Client UI가 입장하기를 리더에게만 노출하므로 정상 경로에서 오지 않는다).
			if (membersIter->second.front() != proposerId)
				return;
			partyId = partyIdIter->second;
			voters = membersIter->second;
		}
	}

	RAID_ENTRY_PROPOSAL proposal{};
	proposal.iProposalId = m_iNextRaidEntryProposalId++;
	if (0u == m_iNextRaidEntryProposalId)
		m_iNextRaidEntryProposalId = 1u;
	proposal.iPartyId = partyId;
	proposal.iRequestSequence = request.iRequestSequence;
	proposal.eTarget = request.eTarget;
	proposal.strNpcPlacementId = request.strNpcPlacementId;
	proposal.Voters = voters;
	proposal.iDeadlineTick = m_iServerTick + VOTE_TIMEOUT_TICKS;

	S2C_RAID_ENTRY_PROMPT prompt{};
	prompt.iProposalId = proposal.iProposalId;
	prompt.iProposerNetEntityId = proposer.iNetEntityId;
	prompt.eTarget = proposal.eTarget;
	prompt.strProposerNickname = proposer.strNickName;
	CPacketWriter promptWriter;
	if (!Write_Message(promptWriter, prompt))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto memberPlayerIter = m_Players.find(memberId);
		if (memberPlayerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(memberPlayerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_PROMPT, promptWriter.Get_Buffer()))
		{
			session->Request_Close();
		}
	}

	m_RaidEntryProposals.push_back(std::move(proposal));
	Broadcast_RaidEntryVote(
		m_RaidEntryProposals.back(), false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Handle_RaidEntryRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto proposalIter = std::find_if(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[&request](const RAID_ENTRY_PROPOSAL& p)
		{
			return p.iProposalId == request.iProposalId;
		});
	if (proposalIter == m_RaidEntryProposals.end())
		return;
	if (std::find(proposalIter->Voters.begin(), proposalIter->Voters.end(),
			responderId) == proposalIter->Voters.end())
	{
		return;
	}
	if (!request.bAccepted)
	{
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::DECLINED);
		return;
	}
	if (std::find(proposalIter->Accepted.begin(), proposalIter->Accepted.end(),
			responderId) == proposalIter->Accepted.end())
	{
		proposalIter->Accepted.push_back(responderId);
	}
	if (proposalIter->Accepted.size() >= proposalIter->Voters.size())
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED);
	else
		Broadcast_RaidEntryVote(*proposalIter, false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Close_RaidEntryVote(
	RAID_ENTRY_PROPOSAL& proposal,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	RAID_ENTRY_VOTE_RESULT finalResult = result;
	if (RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED == result)
	{
		const WORLD_ID targetWorld =
			(RAID_ENTRY_TARGET::KAKULSAYDON == proposal.eTarget)
			? WORLD_ID::KAKULSAYDON_ARENA : WORLD_ID::VALTAN_ARENA;
		// 수락 완료와 실제 stage 사이에 멤버가 unavailable해졌으면 전송하지 않고
		// CANCELLED로 낮춰 전원이 Bern에 남게 한다(부분 이동 금지).
		if (!Stage_PartyWorldTransfer(
				proposal.Voters, targetWorld, proposal.iRequestSequence,
				proposal.strNpcPlacementId))
		{
			finalResult = RAID_ENTRY_VOTE_RESULT::CANCELLED;
		}
	}
	Broadcast_RaidEntryVote(proposal, true, finalResult);
	const std::uint32_t closedId = proposal.iProposalId;
	m_RaidEntryProposals.erase(
		std::remove_if(m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[closedId](const RAID_ENTRY_PROPOSAL& p)
			{
				return p.iProposalId == closedId;
			}),
		m_RaidEntryProposals.end());
}

void LostArk::Server::CGameRoom::Expire_RaidEntryProposals()
{
	using namespace LostArk::Shared;
	// Close_RaidEntryVote가 벡터를 수정하므로 만료 id를 먼저 모은 뒤 닫는다.
	std::vector<std::uint32_t> expiredIds;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (m_iServerTick >= p.iDeadlineTick)
			expiredIds.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : expiredIds)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::TIMEOUT);
	}
}

void LostArk::Server::CGameRoom::Cancel_RaidEntryProposalsInvolving(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	std::vector<std::uint32_t> ids;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (std::find(p.Voters.begin(), p.Voters.end(), playerId) != p.Voters.end())
			ids.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : ids)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::CANCELLED);
	}
}

bool LostArk::Server::CGameRoom::Transfer_PartyTo(
	CGameRoom& target, const std::vector<SESSION_ID>& leaderFirstSessionIds,
	LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
	const std::string& raidReturnNpcPlacementId, const std::string& spawnPlacementOverrideId)
{
	using namespace LostArk::Shared;
	outResult = PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
	const auto reject = [&outResult, &status](const PARTY_TRANSFER_RESULT reason, const char* detail)
	{
		outResult = reason;
		status = detail;
		return false;
	};
    const bool returning = target.m_eWorldId == WORLD_ID::BERN &&
        (m_eWorldId == WORLD_ID::VALTAN_ARENA || m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA);
    const bool entering = m_eWorldId == WORLD_ID::BERN &&
        (target.m_eWorldId == WORLD_ID::VALTAN_ARENA || target.m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA);
	if (!m_isReady || !target.m_isReady || (!entering && !returning) ||
		leaderFirstSessionIds.empty() || leaderFirstSessionIds.size() > MAX_PARTY_MEMBERS || (returning && leaderFirstSessionIds.size() != 1u))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "invalid party transfer world/batch");
	const auto leader = m_PlayerIdBySessionId.find(leaderFirstSessionIds.front());
	if (leader == m_PlayerIdBySessionId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party leader is no longer present");
	const auto sourceParty = m_PartyIdByPlayerId.find(leader->second);
	if (sourceParty == m_PartyIdByPlayerId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party no longer exists");
	const auto sourceMembers = m_PartyMembersByPartyId.find(sourceParty->second);
	if (sourceMembers == m_PartyMembersByPartyId.end() ||
		(!returning && (sourceMembers->second.size() != leaderFirstSessionIds.size() ||
		sourceMembers->second.front() != leader->second)))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party changed before transfer");
	if (0u == target.m_iNextPartyId || target.m_PartyMembersByPartyId.contains(target.m_iNextPartyId))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party identity is exhausted");

    const std::uint32_t sourcePartyId = sourceParty->second;
    const std::vector<PLAYER_ID> departingMembers = returning ? std::vector<PLAYER_ID>{leader->second} : sourceMembers->second;
    const auto sourceGuide = m_Guides.find(sourcePartyId);
    const bool carryGuide = sourceGuide != m_Guides.end() &&
        (!returning || sourceGuide->second.AnchorId == leader->second || sourceMembers->second.size() == 1u);
    std::optional<SERVER_PLAYER> guideEntry;
    GUIDE_RUNTIME guideRuntime;
    std::vector<PACKET_FRAME> guideFrames;
	std::vector<STAGED_PLAYER_ENTRY> entries;
	std::vector<NET_ENTITY_ID> departingEntities;
	entries.reserve(leaderFirstSessionIds.size());
	departingEntities.reserve(leaderFirstSessionIds.size());
	for (std::size_t index = 0; index < leaderFirstSessionIds.size(); ++index)
	{
		const auto member = m_Players.find(departingMembers[index]);
		if (member == m_Players.end() ||
			member->second.iSessionId != leaderFirstSessionIds[index] ||
			std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.begin() + index,
				leaderFirstSessionIds[index]) != leaderFirstSessionIds.begin() + index)
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party member identity changed");
		const auto session = Find_Session(member->second.iSessionId);
		if (nullptr == session || session->Is_Closing())
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party member session is terminal");
		C2S_ENTER_WORLD enter{};
		enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enter.eWorldId = target.m_eWorldId;
		enter.eCharacterClass = member->second.eCharacterClass;
		enter.strNickName = member->second.strNickName;
		STAGED_PLAYER_ENTRY entry{};
		SESSION_DIAGNOSTIC_REASON reason{};
		if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status,
			spawnPlacementOverrideId, member->second.Inventory, member->second.iHonorTitleId, raidReturnNpcPlacementId))
		{
			outResult = SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL == reason ?
				PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL : PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entries.push_back(std::move(entry));
		departingEntities.push_back(member->second.iNetEntityId);
	}
    if (carryGuide)
    {
        if (!target.m_GuideCatalog.Loaded || target.m_GuideCatalog.Revision != m_GuideCatalog.Revision ||
            target.m_Players.size() + entries.size() + 1u > MAX_WORLD_SNAPSHOT_PLAYERS)
            return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "guide generation or destination capacity is unavailable");
        const auto anchor = std::find(departingMembers.begin(), departingMembers.end(), sourceGuide->second.AnchorId);
        const auto index = anchor == departingMembers.end() ? 0u : static_cast<std::size_t>(anchor - departingMembers.begin());
        const auto& owner = entries[index].Player;
        SERVER_PLAYER companion;
        bool guideAdmitted = false;
        for (unsigned candidate = 0; candidate < 16 && !guideAdmitted; ++candidate)
        {
            const float angle = static_cast<float>(candidate) * 3.14159265359f / 8.f;
            if (!target.Build_GuidePlayer(target.m_iNextGuidePlayerId,
                target.m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(entries.size()),
                owner.fPositionX + std::sin(angle) * target.m_GuideCatalog.DesiredDistance,
                owner.fPositionY,
                owner.fPositionZ + std::cos(angle) * target.m_GuideCatalog.DesiredDistance, companion)) continue;
            guideAdmitted = std::none_of(entries.begin(), entries.end(), [&](const auto& entry) {
                return std::abs(entry.Player.fPositionY - companion.fPositionY) < 1.5f &&
                    std::hypot(entry.Player.fPositionX - companion.fPositionX,
                        entry.Player.fPositionZ - companion.fPositionZ) < .75f;
            });
        }
        if (!guideAdmitted)
            return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "guide destination failed navigation or collision admission");
        guideRuntime.PlayerId = companion.iPlayerId; guideRuntime.AnchorId = owner.iPlayerId;
        guideRuntime.EventSequence = sourceGuide->second.EventSequence;
        guideEntry = companion;
        const auto oldActor = m_Players.find(sourceGuide->second.PlayerId);
        if (oldActor == m_Players.end()) return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "guide actor disappeared before transfer");
        departingEntities.push_back(oldActor->second.iNetEntityId);
        S2C_PLAYER_SPAWNED spawn;
        spawn.iPlayerId=companion.iPlayerId;spawn.iNetEntityId=companion.iNetEntityId;spawn.eCharacterClass=companion.eCharacterClass;
        spawn.eControlKind=companion.eControlKind;spawn.strNickName=companion.strNickName;
        spawn.fPositionX=companion.fPositionX;spawn.fPositionY=companion.fPositionY;spawn.fPositionZ=companion.fPositionZ;spawn.fYawDegrees=companion.fYawDegrees;
        CPacketWriter writer;if(!Write_Message(writer,spawn))return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED,"guide spawn encoding failed");
        guideFrames.push_back({PACKET_TYPE::S2C_PLAYER_SPAWNED,writer.Get_Buffer()});
    }
	std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
	S2C_PARTY_ROSTER roster{};
	for (const auto& entry : entries)
		roster.Members.push_back({ entry.Player.iNetEntityId, entry.Player.strNickName,
			entry.Player.eCharacterClass });
    if (guideEntry) roster.GuideCompanion = PARTY_ROSTER_MEMBER{guideEntry->iNetEntityId,guideEntry->strNickName,guideEntry->eCharacterClass,guideEntry->eControlKind};
	CPacketWriter rosterWriter;
	if (!Write_Message(rosterWriter, roster))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party roster failed encoding");
	for (auto& entry : entries)
	{
		if (!target.Build_PlayerEntryFrames(entry, entries, status))
		{
			outResult = PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entry.Frames.insert(entry.Frames.end(),guideFrames.begin(),guideFrames.end());
		entry.Frames.push_back({ PACKET_TYPE::S2C_PARTY_ROSTER, rosterWriter.Get_Buffer() });
		outboundBatches.push_back({ entry.pSession, entry.Frames });
	}
	// Include observer notifications in the same bounded FIFO reservation;
	// neither a slow member nor a slow spectator can cause a partial commit.
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (player.Is_Guide()) continue;
		if (std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.end(),
			player.iSessionId) != leaderFirstSessionIds.end()) continue;
		CLIENT_SESSION_RELIABLE_BATCH observer{ Find_Session(player.iSessionId), {} };
		for (const NET_ENTITY_ID entityId : departingEntities)
		{
			S2C_PLAYER_DESPAWNED message{};
			message.iNetEntityId = entityId;
			message.eReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "source departure payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer() });
		}
		outboundBatches.push_back(std::move(observer));
	}
	for (const auto& [id, player] : target.m_Players)
	{
		(void)id;
		if (player.Is_Guide()) continue;
		CLIENT_SESSION_RELIABLE_BATCH observer{ target.Find_Session(player.iSessionId), {} };
		for (const auto& entry : entries)
		{
			S2C_PLAYER_SPAWNED message{};
			message.iPlayerId = entry.Player.iPlayerId;
			message.iNetEntityId = entry.Player.iNetEntityId;
			message.eCharacterClass = entry.Player.eCharacterClass;
			message.strNickName = entry.Player.strNickName;
			message.fPositionX = entry.Player.fPositionX;
			message.fPositionY = entry.Player.fPositionY;
			message.fPositionZ = entry.Player.fPositionZ;
			message.fYawDegrees = entry.Player.fYawDegrees;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target spawn payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer() });
		}
        observer.Frames.insert(observer.Frames.end(),guideFrames.begin(),guideFrames.end());
		outboundBatches.push_back(std::move(observer));
	}

	// All allocating membership work is staged before taking outbound locks.
	// Commit below contains only erases, swaps and atomic player-id stores.
	auto targetPlayers = target.m_Players;
	auto targetSessionPlayers = target.m_PlayerIdBySessionId;
	auto targetEntityPlayers = target.m_PlayerIdByEntityId;
	auto targetSessions = target.m_Sessions;
	auto targetPartyIds = target.m_PartyIdByPlayerId;
	auto targetParties = target.m_PartyMembersByPartyId;
    auto targetGuides = target.m_Guides;
	std::vector<PLAYER_ID> targetMembers;
	targetMembers.reserve(entries.size());
	for (const auto& entry : entries)
	{
		const SERVER_PLAYER& player = entry.Player;
		targetPlayers.emplace(player.iPlayerId, player);
		targetSessionPlayers.emplace(player.iSessionId, player.iPlayerId);
		targetEntityPlayers.emplace(player.iNetEntityId, player.iPlayerId);
		targetSessions.insert_or_assign(player.iSessionId, entry.pSession);
		targetPartyIds.emplace(player.iPlayerId, target.m_iNextPartyId);
		targetMembers.push_back(player.iPlayerId);
	}
    if(guideEntry){targetPlayers.emplace(guideEntry->iPlayerId,*guideEntry);targetEntityPlayers.emplace(guideEntry->iNetEntityId,guideEntry->iPlayerId);targetGuides.emplace(target.m_iNextPartyId,guideRuntime);}
	targetParties.emplace(target.m_iNextPartyId, std::move(targetMembers));
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare(outboundBatches, status))
	{
		outResult = PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY;
		return false;
	}
	// No callback here may send to the locked queues. Whole-party removal has
	// no intermediate roster; all departures/arrivals were staged above.
    for (const PLAYER_ID memberId : departingMembers) m_PartyIdByPlayerId.erase(memberId);
    for (const auto id : departingMembers) std::erase(sourceMembers->second,id);
    const bool sourcePartyRemains = !sourceMembers->second.empty();
    if(!sourcePartyRemains) m_PartyMembersByPartyId.erase(sourceMembers);
    if(carryGuide) Remove_Guide(sourcePartyId,false);
	for (const SESSION_ID sessionId : leaderFirstSessionIds)
		Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
	target.m_Players.swap(targetPlayers);
	target.m_PlayerIdBySessionId.swap(targetSessionPlayers);
	target.m_PlayerIdByEntityId.swap(targetEntityPlayers);
	target.m_Sessions.swap(targetSessions);
	target.m_PartyIdByPlayerId.swap(targetPartyIds);
	target.m_PartyMembersByPartyId.swap(targetParties);
    target.m_Guides.swap(targetGuides);
	target.m_iNextPlayerId += static_cast<PLAYER_ID>(entries.size());
    if(guideEntry) ++target.m_iNextGuidePlayerId;
	target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(entries.size() + (guideEntry ? 1u : 0u));
	++target.m_iNextPartyId;
	for (const auto& entry : entries)
		entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
	outbound.Commit();
    if(sourcePartyRemains) Broadcast_PartyRoster(sourcePartyId);
	status = "party transfer committed";
	return true;
}

void LostArk::Server::CGameRoom::Notify_PartyTransferFailure(
	const SESSION_ID sessionId, const std::uint32_t requestSequence,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const LostArk::Shared::PARTY_TRANSFER_RESULT result)
{
	if (0u == requestSequence || !m_PlayerIdBySessionId.contains(sessionId)) return;
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT message{};
	message.iRequestSequence = requestSequence;
	message.eTargetWorldId = targetWorldId;
	message.eResult = result;
	m_PendingPartyTransferResults.insert_or_assign(sessionId, message);
	Flush_PartyTransferResults();
}

void LostArk::Server::CGameRoom::Flush_PartyTransferResults()
{
	using namespace LostArk::Shared;
	for (auto iter = m_PendingPartyTransferResults.begin(); iter != m_PendingPartyTransferResults.end();)
	{
		const auto session = Find_Session(iter->first);
		if (nullptr == session || session->Is_Closing())
		{
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CPacketWriter writer;
		if (!Write_Message(writer, iter->second))
		{
			m_strStatus = "party transfer failure notice failed validation";
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
		std::string status;
		if (!outbound.Prepare({ { session, {
			{ PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT, writer.Get_Buffer() } } } }, status))
		{
			++iter;
			continue;
		}
		outbound.Commit();
		iter = m_PendingPartyTransferResults.erase(iter);
	}
}

void LostArk::Server::CGameRoom::Handle_RoomPing(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	if (binding == m_PlayerIdBySessionId.end() || request.eWorldId != m_eWorldId) return;
	const auto sender = m_Players.find(binding->second);
	if (sender == m_Players.end() || sender->second.iSessionId != sessionId) return;
	auto& player = sender->second;
	CPacketWriter validated;
	if (!Write_Message(validated, request) || !Is_NewerSequence(request.iClientSequence, player.iLastRoomPingSequence) ||
		!player.iCurrentHp || !player.isCombatReady || player.eAction == PLAYER_ACTION_STATE::DEAD) return;
	player.iLastRoomPingSequence = request.iClientSequence;
	const auto tick = m_iServerTick ? m_iServerTick : 1u;
	if (player.iLastRoomPingTick && Elapsed_ServerTicksSkippingReservedZero(player.iLastRoomPingTick, tick) < 8u) return;
	SERVER_NAV_POINT ground;
	if (!m_ServerNavigation.Sample_Position(request.fPositionX, request.fPositionZ, ground, request.fPositionY)) return;
	S2C_ROOM_PING message;
	message.eWorldId = m_eWorldId; message.iFromNetEntityId = player.iNetEntityId;
	message.iClientSequence = request.iClientSequence;
	message.fPositionX = ground.x; message.fPositionY = ground.y; message.fPositionZ = ground.z;
	CPacketWriter writer;
	if (!Write_Message(writer, message)) return;
	player.iLastRoomPingTick = tick;
	for (const auto& [id, recipient] : m_Players)
		if (const auto session = Find_Session(recipient.iSessionId); session &&
			!session->Send_Frame(PACKET_TYPE::S2C_ROOM_PING, writer.Get_Buffer())) session->Request_Close();
}

void LostArk::Server::CGameRoom::Handle_Chat(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHAT& request)
{
	using namespace LostArk::Shared;

	const auto senderPlayerIdIter = m_PlayerIdBySessionId.find(sessionId);
	if (senderPlayerIdIter == m_PlayerIdBySessionId.end())
		return;
	const auto senderIter = m_Players.find(senderPlayerIdIter->second);
	if (senderIter == m_Players.end())
		return;

	S2C_CHAT message{};
	message.iFromNetEntityId = senderIter->second.iNetEntityId;
	message.strFromNickname = senderIter->second.strNickName;
	message.strText = request.strText;
	Guide_ChatCommand(senderIter->second, request.strText);

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;

	// Every current room member, sender included -- see Handle_Chat's own
	// header comment for why the sender reads its own bubble off this same
	// broadcast instead of a second local-only path.
	for (const auto& [playerId, player] : m_Players)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(PACKET_TYPE::S2C_CHAT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_SpawnWorldEntity(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	/* Arena F1 controls share the same disabled boss placement allowlist in
	Debug and Release. Character Select keeps its private-room admission. */
	if (Is_KoukuRaidRunning())
	{
		Send_WorldEntitySpawnResult(session, request.strPlacementId, WORLD_ENTITY_SPAWN_RESULT::REJECTED, INVALID_NET_ENTITY_ID);
		return;
	}
	const bool koukuGateWorld = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId;
    const bool valtanWorld = WORLD_ID::VALTAN_ARENA == m_eWorldId;
	const bool debugSpawnWorld =
		WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId || koukuGateWorld || valtanWorld;
	if (!debugSpawnWorld ||
		!m_PlayerIdBySessionId.contains(sessionId) || nullptr == session)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(request.strPlacementId);
	if (nullptr == placement)
	{
		if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		const auto group = std::find_if(
			m_SpawnGroupBootstrap.Get_Groups().begin(),
			m_SpawnGroupBootstrap.Get_Groups().end(),
			[&request](const SPAWN_GROUP_DEFINITION& definition)
			{
				return definition.strSpawnGroupId == request.strPlacementId;
			});
		if (m_SpawnGroupBootstrap.Get_Groups().end() == group)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		if (!m_SpawnGroupRuntime.Is_ActiveOrCompleted(request.strPlacementId) &&
			!m_SpawnGroupRuntime.Activate_Immediate(
				request.strPlacementId,
				m_SpawnGroupBootstrap,
				[this](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				}))
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}

		if (!Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::ACTIVATED,
			INVALID_NET_ENTITY_ID))
		{
			session->Request_Close();
		}
		return;
	}

	const bool admittedPlacement = valtanWorld ?
        (!placement->isEnabled && placement->strPlacementId == "boss.valtan.center" &&
            placement->eKind == WORLD_BOOTSTRAP_KIND::BOSS && placement->strArchetypeId == "BOSS_VALTAN" &&
            placement->strEncounterId == "ENCOUNTER_VALTAN") :
		WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId ?
			(!placement->isEnabled &&
			 WORLD_BOOTSTRAP_KIND::BOSS == placement->eKind &&
			 placement->strArchetypeId == "BOSS_VALTAN") :
			CKoukuSaydonBrain::Is_ArenaBossPlacement(m_eWorldId, *placement);
	if (!admittedPlacement)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}
	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == request.strPlacementId;
		});
	if (m_WorldEntities.end() != existing)
	{
		if (!Send_WorldEntitySpawned(session, *existing) ||
			!Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS,
				existing->iNetEntityId))
		{
			session->Request_Close();
		}
		if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
			Apply_KoukuGateEntryCard(player->second, *existing);
		return;
	}
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

#ifdef _DEBUG
    if (valtanWorld)
    {
        staged.bIntroPatternConsumed = true;
        staged.bAutomaticPatternSequenceAuditionOverride = true;
        staged.bAutomaticPatternSequenceAuditionHold = true;
    }
#endif
	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
		Apply_KoukuGateEntryCard(player->second, m_WorldEntities.back());
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	Note_GatePlacementRaised(m_WorldEntities.back().strPlacementId);
	if (!Send_WorldEntitySpawnResult(
		session,
		request.strPlacementId,
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED,
		m_WorldEntities.back().iNetEntityId))
	{
		session->Request_Close();
	}
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_KoukuSaydonAuditionBoss()
{
	const auto found = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this](const SERVER_WORLD_ENTITY& entity)
		{
			return CKoukuSaydonBrain::Is_GateOneBoss(m_eWorldId, entity);
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}
```

## Server/Private/GameRoom_Admission.cpp

```cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;
	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

bool LostArk::Server::CGameRoom::Stage_PlayerEntry(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
	STAGED_PLAYER_ENTRY& staged,
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory,
	const LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId,
	const std::string& raidReturnNpcPlacementId)
{
	using namespace LostArk::Shared;
	outReason = SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED;
	status.clear();
	const auto reject = [&outReason, &status](
		const SESSION_DIAGNOSTIC_REASON reason, const char* detail)
	{
		outReason = reason;
		status = detail;
		return false;
	};
	const std::size_t offset = precedingEntries.size();
	if (!raidReturnNpcPlacementId.empty() &&
		((WORLD_ID::VALTAN_ARENA != m_eWorldId && WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId) ||
		 (raidReturnNpcPlacementId != "npc.bern.beda.guide" && raidReturnNpcPlacementId != "npc.bern.aylara")))
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"invalid raid return guide or destination world");
	if (!m_isReady || nullptr == session || session->Is_Closing() ||
		INVALID_SESSION_ID == session->Get_SessionId() ||
		!Is_Valid_EnterWorld(enterWorld) || enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(session->Get_SessionId()) ||
		m_Players.size() + offset >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		INVALID_PLAYER_ID == m_iNextPlayerId ||
		INVALID_NET_ENTITY_ID == m_iNextNetEntityId ||
		offset > (std::numeric_limits<PLAYER_ID>::max)() - m_iNextPlayerId ||
		offset > (std::numeric_limits<NET_ENTITY_ID>::max)() - m_iNextNetEntityId)
	{
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"player entry room/session/identity validation failed");
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = nullptr;
	if (!spawnPlacementOverrideId.empty())
	{
		/* Not restricted to PLAYER_SPAWN kind or exclusivity -- an override names
		one specific placement (e.g. a guide NPC) directly, and several returning
		players landing at the same NPC concurrently is fine (unlike normal
		PLAYER_SPAWN slots, which are one-player-at-a-time). */
		spawn = Find_Placement(spawnPlacementOverrideId);
		if (nullptr == spawn || !spawn->isEnabled)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				"spawn placement override id does not exist in this world's bootstrap");
	}
	else
	{
		for (const auto& candidate : m_WorldBootstrap.Get_Placements())
		{
			if (!candidate.isEnabled || WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != candidate.eKind)
				continue;
			const bool occupied = std::any_of(m_Players.begin(), m_Players.end(),
				[&candidate](const auto& value)
				{ return value.second.strSpawnPlacementId == candidate.strPlacementId; });
			const bool reserved = std::any_of(precedingEntries.begin(), precedingEntries.end(),
				[&candidate](const auto& value)
				{ return value.Player.strSpawnPlacementId == candidate.strPlacementId; });
			if (!occupied && !reserved) { spawn = &candidate; break; }
		}
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				"target room has fewer free player spawns than the transfer batch");
	}
	STAGED_PLAYER_ENTRY candidate{};
	candidate.pSession = session;
	SERVER_PLAYER& player = candidate.Player;
	player.iSessionId = session->Get_SessionId();
	player.iPlayerId = m_iNextPlayerId + static_cast<PLAYER_ID>(offset);
	player.iNetEntityId = m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(offset);
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	/* A transfer keeps the title it wore; the target room's own bootstrap still has the
	last word, so an id it does not list arrives bare. */
	player.iHonorTitleId = m_HonorTitleCatalog.Has_Title(carriedHonorTitleId) ?
		carriedHonorTitleId : INVALID_HONOR_TITLE_ID;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.strRaidReturnNpcPlacementId = raidReturnNpcPlacementId;
	player.fPositionY = spawn->fPositionY;
	if (!spawnPlacementOverrideId.empty())
	{
		/* An override names an NPC's own placement, not an authored player-standing
		spot -- its exact point is often flush against a wall or counter (the NPC's
		back), so landing there directly can navigation-project to the wrong side
		of that geometry. Stand where a player who walked up to talk to it would:
		NPC_APPROACH_OFFSET_M out along its own forward direction, facing back
		toward it (matches this codebase's yaw convention, forward = (sin, cos),
		e.g. MonsterBrain.cpp's own movement step). */
		constexpr float NPC_APPROACH_OFFSET_M = 2.5f;
		const float yawRadians = spawn->fYawDegrees * DEGREES_TO_RADIANS;
		player.fPositionX = spawn->fPositionX + std::sin(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fPositionZ = spawn->fPositionZ + std::cos(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fYawDegrees = std::fmod(spawn->fYawDegrees + 180.f, 360.f);
	}
	else
	{
		player.fPositionX = spawn->fPositionX;
		player.fPositionZ = spawn->fPositionZ;
		player.fYawDegrees = spawn->fYawDegrees;
	}
	const PLAYER_RUNTIME_PROFILE* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_PROFILE_MISSING,
			"selected character class has no runtime profile");
	player.eStance = profile->eDefaultStance;
	player.iCurrentHp = player.iMaximumHp = profile->iMaximumHp;
	player.iCurrentResource = player.iMaximumResource = profile->iMaximumResource;
	player.fMoveSpeed = profile->fMoveSpeed;
	player.iMaximumIdentity = profile->iMaximumIdentity;
	CPlayerSkillSystem::Reset_Gauges(player, m_GameplayCatalog);
	player.iCurrentMadness = 0u; player.dMadnessRemainder = 0.;
	player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	player.Clear_KoukuInteractionState();
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, projected,
			spawnPlacementOverrideId.empty() ? NAVIGATION_HEIGHT_UNKNOWN : player.fPositionY))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED,
				"player spawn could not project onto Server navigation");
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}
	if (!carriedInventory.empty())
	{
		// A world transfer carrying the departing player's own live inventory
		// (e.g. Handle_ReturnToBern) replaces the default fresh-entry grant
		// entirely -- Valtan clear rewards must survive the trip back to Bern.
		player.Inventory = carriedInventory;
	}
	else
	{
		for (const char* potionId : { "POTION_HP_SMALL", "POTION_HP_MEDIUM", "POTION_HP_LARGE" })
		{
			const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(potionId);
			if (nullptr == definition) continue;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = potionId;
			item.iQuantity = (std::min)(500u, definition->iMaxStack);
			player.Inventory.push_back(std::move(item));
		}
		/* A fresh character already wears the catalog's starting accessories. */
		for (const auto& [itemId, slot] : m_ItemCatalog.Get_StartingEquipment())
		{
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = itemId;
			item.iQuantity = 1u;
			item.eEquippedSlot = slot;
			player.Inventory.push_back(std::move(item));
		}
	}
	staged = std::move(candidate);
	return true;
}

bool LostArk::Server::CGameRoom::Build_PlayerEntryFrames(
	STAGED_PLAYER_ENTRY& entry, const std::span<const STAGED_PLAYER_ENTRY> batch,
	std::string& status)
{
	using namespace LostArk::Shared;
	std::vector<PACKET_FRAME> frames;
	const auto append = [&frames, &status](const PACKET_TYPE type, const auto& message)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
		{
			status = "initial entry payload failed validation, packet=" +
				std::to_string(static_cast<std::uint16_t>(type));
			return false;
		}
		frames.push_back({ type, writer.Get_Buffer() });
		return true;
	};
	S2C_ENTER_ACCEPTED accepted{};
	accepted.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	accepted.eWorldId = m_eWorldId;
	accepted.iPlayerId = entry.Player.iPlayerId;
	accepted.iNetEntityId = entry.Player.iNetEntityId;
	accepted.ActiveGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(accepted.RequiredPinnedGameplayRevisions))
	{
		status = "initial entry pinned gameplay revisions failed validation";
		return false;
	}
	if (!append(PACKET_TYPE::S2C_ENTER_ACCEPTED, accepted)) return false;
	S2C_INVENTORY_SNAPSHOT inventory{};
	inventory.Items = entry.Player.Inventory;
	if (!append(PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, inventory)) return false;
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_WorldDestructionRuntime.Is_Initialized())
		{
			status = "initial world destruction runtime is not initialized";
			return false;
		}
		S2C_WORLD_DESTRUCTION_FULL_SYNC fullSync{};
		fullSync.strCombatRuntimeRevision = m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
		fullSync.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		fullSync.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
		for (const auto& state : m_WorldDestructionRuntime.Get_GroupStates())
			fullSync.GroupStates.push_back(To_NetworkDestructionState(state));
		fullSync.Diagnostics = Build_WorldDestructionDiagnostics();
		if (!append(PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, fullSync)) return false;
	}
	if (m_EncounterPropRuntime.Is_Initialized())
	{
		S2C_ENCOUNTER_PROP_SYNC props{};
		props.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
		props.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		props.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
		for (const auto& slot : m_EncounterPropRuntime.Get_SlotStates())
		{
			ENCOUNTER_PROP_SLOT_WIRE wire{};
			wire.strSlotId = slot.strSlotId;
			wire.eState = slot.eState;
			wire.iStateVersion = slot.iStateVersion;
			wire.iStateStartTick = slot.iStateStartTick;
			wire.iOccurrenceSequence = slot.iOccurrenceSequence;
			props.Slots.push_back(std::move(wire));
		}
		if (!append(PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, props)) return false;
	}
	std::unordered_set<NET_ENTITY_ID> admittedWorldEntityIds;
	for (const bool dependentPass : { false, true })
	{
		for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
		{
			if (entity.eKind == WORLD_BOOTSTRAP_KIND::WORLD_OBJECT) continue;
			const bool isDependent = INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId;
			if (isDependent != dependentPass)
				continue;
			if (!admittedWorldEntityIds.insert(entity.iNetEntityId).second)
			{
				status = "Initial world entity ID is duplicated";
				return false;
			}
			if (isDependent)
			{
				const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
					[&entity](const SERVER_WORLD_ENTITY& candidate)
					{ return candidate.iNetEntityId == entity.iOwnerBossNetEntityId; });
				if (!admittedWorldEntityIds.contains(entity.iOwnerBossNetEntityId) ||
					owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
					INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
					owner->strEncounterId != entity.strEncounterId ||
					owner->PinnedDefinitionRevision !=
						entity.PinnedDefinitionRevision)
				{
					status = "Initial dependent boss has no preceding primary owner";
					return false;
				}
			}
			std::vector<std::uint8_t> payload;
			if (!Build_WorldEntitySpawnedPayload(entity, payload))
			{
				status = "World entity spawn payload preflight failed: " + entity.strPlacementId;
				return false;
			}
			frames.push_back({ PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, std::move(payload) });
		}
	}
	S2C_KOUKUSAYDON_RAID_STATE raidState;
	if (m_MaharakaWaterpangIntro)
	{
		auto intro = *m_MaharakaWaterpangIntro;
		intro.iServerTick = m_iServerTick;
		if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, intro)) return false;
	}
	if (Build_KoukuRaidState(raidState) && !append(PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE, raidState)) return false;
	S2C_GATE_PROGRESS_STATE gateState;
	if (Build_GateProgressState(gateState, false, GATE_PROGRESS_VOTE_RESULT::NONE) &&
		!append(PACKET_TYPE::S2C_GATE_PROGRESS_STATE, gateState)) return false;
	S2C_KOUKUSAYDON_BUNDLE_STATE bundleState;
	if (Build_KoukuBundleState(bundleState) && !append(PACKET_TYPE::S2C_KOUKUSAYDON_BUNDLE_STATE, bundleState)) return false;
	{
		// Natural Stage/run completion retains independent World rows for late join.
		for (auto play : m_KoukuSaydonPatternAudition.WorldPlays)
		{
			if (play.bUntilDestroyed || play.iCombatBodyNetEntityId) continue; // Live combat bodies below own late join replay.
			if (play.iDurationMs && Has_ReachedServerTick(m_iServerTick, Add_ServerTicksSkippingReservedZero(play.iStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(play.iDurationMs)))) continue;
			play.iServerTick = m_iServerTick;
			if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, play)) return false;
		}
	}
	for (const auto& cue : m_KoukuDamageableWorldCues)
	{
		if (cue.bCancelled) continue;
		const auto live = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& row) { return row.iNetEntityId == cue.iBodyId && row.iCurrentHp; });
		const auto pending = std::find_if(m_PendingKoukuWorldBodies.begin(), m_PendingKoukuWorldBodies.end(), [&](const auto& row) { return row.iNetEntityId == cue.iBodyId && row.iCurrentHp; });
		if (live == m_WorldEntities.end() && pending == m_PendingKoukuWorldBodies.end()) continue;
		auto play = cue.Play; play.iServerTick = m_iServerTick;
		if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, play)) return false;
	}
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> combatObjects;
	m_CombatObjectRuntime.Build_LiveSpawnMessages(0u == m_iServerTick ? 1u : m_iServerTick, combatObjects);
	for (const auto& object : combatObjects)
		if (!append(PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, object)) return false;
	const auto appendPlayer = [&append](const SERVER_PLAYER& player)
	{
		S2C_PLAYER_SPAWNED message{};
		message.iPlayerId = player.iPlayerId;
		message.iNetEntityId = player.iNetEntityId;
		message.eCharacterClass = player.eCharacterClass;
		message.eControlKind = player.eControlKind;
		message.strNickName = player.strNickName;
		message.fPositionX = player.fPositionX;
		message.fPositionY = player.fPositionY;
		message.fPositionZ = player.fPositionZ;
		message.fYawDegrees = player.fYawDegrees;
		return append(PACKET_TYPE::S2C_PLAYER_SPAWNED, message);
	};
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (!appendPlayer(player)) return false;
	}
	for (const auto& staged : batch)
		if (!appendPlayer(staged.Player)) return false;
	entry.Frames = std::move(frames);
	return true;
}

void LostArk::Server::CGameRoom::Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry)
{
	const SERVER_PLAYER& player = entry.Player;
	m_Sessions.insert_or_assign(player.iSessionId, entry.pSession);
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	entry.pSession->Bind_PlayerId(player.iPlayerId);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory,
	const LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId,
	const std::string& raidReturnNpcPlacementId)
{
	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				WSAEINVAL,
				"ENTER_WORLD failed room/session/id validation");
		}
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		const std::size_t enabledPlayerSpawnCount =
			static_cast<std::size_t>(std::count_if(
				m_WorldBootstrap.Get_Placements().begin(),
				m_WorldBootstrap.Get_Placements().end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
				}));
		const std::string roomFullCounts =
			"activePlayers=" + std::to_string(m_Players.size()) +
			", registeredSessionsIncludingCandidate=" +
			std::to_string(m_Sessions.size()) +
			", candidateSessionId=" + std::to_string(sessionId) +
			", candidateRegistered=" +
			(m_Sessions.contains(sessionId) ? "true" : "false") +
			", enabledPlayerSpawns=" +
			std::to_string(enabledPlayerSpawnCount);
		/* Leave room for the terminal-action prefix so the copied close context
		   remains bounded to roughly one KiB. */
		constexpr std::size_t MAX_ROOM_FULL_CONTEXT_BYTES = 960u;
		const std::uint64_t observedUnixMilliseconds =
			Current_UnixMilliseconds();
		std::string roomFullContext = roomFullCounts + ", activeRoster=[";
		bool isFirstRosterEntry = true;
		for (const auto& [playerId, player] : m_Players)
		{
			const std::shared_ptr<CClientSession> incumbent =
				Find_Session(player.iSessionId);
			std::string peer = "unavailable";
			std::uint64_t lastInboundUnixMilliseconds = 0u;
			if (nullptr != incumbent)
			{
				const CLIENT_SESSION_PEER_ENDPOINT& endpoint =
					incumbent->Get_PeerEndpoint();
				peer = endpoint.strAddress + ':' +
					std::to_string(endpoint.iPort);
				lastInboundUnixMilliseconds =
					incumbent->Get_LastInboundUnixMilliseconds();
			}
			const std::uint64_t lastInboundAgeMilliseconds =
				0u != lastInboundUnixMilliseconds &&
				observedUnixMilliseconds >= lastInboundUnixMilliseconds ?
				observedUnixMilliseconds - lastInboundUnixMilliseconds : 0u;
			const std::string rosterEntry =
				(isFirstRosterEntry ? "" : ", ") +
				std::string{ "{sessionId=" } +
				std::to_string(player.iSessionId) +
				", playerId=" + std::to_string(playerId) +
				", spawn=" + player.strSpawnPlacementId +
				", peer=" + peer +
				", lastInboundUnixMs=" +
				std::to_string(lastInboundUnixMilliseconds) +
				", lastInboundAgeMs=" +
				std::to_string(lastInboundAgeMilliseconds) + '}';
			if (roomFullContext.size() + rosterEntry.size() + 1u >
				MAX_ROOM_FULL_CONTEXT_BYTES)
			{
				constexpr std::string_view TRUNCATED =
					", {truncated=true}]";
				roomFullContext.resize((std::min)(
					roomFullContext.size(),
					MAX_ROOM_FULL_CONTEXT_BYTES - TRUNCATED.size()));
				roomFullContext.append(TRUNCATED);
				break;
			}
			roomFullContext += rosterEntry;
			isFirstRosterEntry = false;
		}
		if (roomFullContext.empty() || ']' != roomFullContext.back())
			roomFullContext += ']';
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"typed ROOM_FULL rejection flushed before close; " +
					roomFullContext);
		}
		else
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"ROOM_FULL rejection could not be queued; " +
					roomFullContext);
		}
		return false;
	}
	STAGED_PLAYER_ENTRY entry{};
	SESSION_DIAGNOSTIC_REASON reason{};
	std::string status;
	if (!Stage_PlayerEntry(session, enterWorld, {}, entry, reason, status,
			spawnPlacementOverrideId, carriedInventory, carriedHonorTitleId,
			raidReturnNpcPlacementId))
	{
		session->Request_Close(reason, WSAEINVAL, status);
		return false;
	}
	if (!Build_PlayerEntryFrames(entry, std::span<const STAGED_PLAYER_ENTRY>{ &entry, 1u }, status))
	{
		m_strStatus = status;
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, 0, status);
		return false;
	}
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare({ { session, entry.Frames } }, status))
	{
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_INITIAL_SYNC_ENQUEUE_FAILED, 0, status);
		return false;
	}
	Commit_PlayerEntry(entry);
	outbound.Commit();
	Broadcast_Spawned(entry.Player, sessionId);
	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId << ", PlayerId=" << entry.Player.iPlayerId
		<< ", Spawn=" << entry.Player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason, const bool publishDeparture)
{
	using namespace LostArk::Shared;

	if (Is_KoukuRaidRunning())
	{
		const auto departing = m_PlayerIdBySessionId.find(sessionId);
		if (departing != m_PlayerIdBySessionId.end() && std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), departing->second) != m_KoukuRaid.PlayerIds.end())
			Stop_KoukuRaid("A raid participant left the room");
	}
	m_KoukuRaidReceipts.erase(sessionId);
	if (sessionId == m_ValtanPatternIdAudition.iOwnerSessionId)
	{
		Cancel_ValtanNextPatternReservation("owner left the room");
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase)
		{
			if (SERVER_WORLD_ENTITY* boss =
				Find_AuditionBoss(m_ValtanPatternIdAudition.strBossPlacementId))
			{
				std::erase(boss->PendingPatternIds, m_ValtanPatternIdAudition.strPatternId);
				boss->bAutomaticPatternSequenceAuditionOverride = true;
				boss->bAutomaticPatternSequenceAuditionHold = true;
			}
			Cancel_ValtanPatternIdAudition("owner left before the occurrence started");
		}
		else
		{
			// A already running with other players may finish normally; its
			// departed owner can no longer append to that terminal anchor.
			m_ValtanPatternIdAudition.iOwnerSessionId = INVALID_SESSION_ID;
		}
	}
	m_ValtanNextPatternReceiptBySessionId.erase(sessionId);
	if (sessionId == m_ValtanPatternFlowAudition.iOwnerSessionId &&
		Is_ValtanPatternFlowRunning())
	{
		Abort_ValtanPatternFlowForOwner(
			sessionId, "Valtan pattern-flow owner left the room");
		(void)Flush_ValtanPatternFlowLifecycle();
	}
	if (sessionId == m_ValtanTimelineAudition.iOwnerSessionId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}
	Cancel_KoukuWorldBodies({}, sessionId);
	const bool koukuOwnerLeft = sessionId == m_KoukuSaydonPatternAudition.iOwnerSessionId;
	const auto soloMarioDeparture = std::find_if(
		m_KoukuSaydonPatternAudition.Members.begin(), m_KoukuSaydonPatternAudition.Members.end(),
		[sessionId, koukuOwnerLeft](const auto& member) {
			return member.bMarioSoloReturnRequired && !member.bCompletionChainSuccessQueued &&
				(koukuOwnerLeft || member.iMarioEntrantSessionId == sessionId);
		});
	if (soloMarioDeparture != m_KoukuSaydonPatternAudition.Members.end())
	{
		m_strStatus = std::string("Mario solo ") +
			(soloMarioDeparture->iMarioEntrantSessionId == sessionId ? "entrant " : "run owner ") +
			(reason == PLAYER_DESPAWN_REASON::DISCONNECTED ? "disconnected" : "left the room") + " before phase 2";
		// The departing owner cannot receive its receipt; keep the cause in the Server log.
		std::cout << "[MarioSoloAbort] session=" << sessionId << " entrant=" << soloMarioDeparture->iMarioEntrantPlayerId
			<< " reason=" << m_strStatus << '\n';
		Clear_KoukuSaydonPatternAudition(false, m_strStatus);
	}
	else if (koukuOwnerLeft) Clear_KoukuSaydonPatternAudition();

	std::erase_if(m_PendingKoukuSaydonPatternAuditionLifecycle,
		[sessionId](const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& edge)
		{
			return edge.iSessionId == sessionId;
		});
	m_KoukuSaydonPatternAuditionReceiptBySessionId.erase(sessionId);
	m_KoukuDraftUploads.erase(sessionId);
	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
	m_KillGateBossesRequestSequences.erase(sessionId);
	m_CooldownModeRequestSequences.erase(sessionId);
	m_WorldPlaybackRequestSequences.erase(sessionId);
	m_RoomPlayerArrivalRuns.erase(sessionId);
	m_ValtanPatternIdAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowStartSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowControlSequenceBySessionId.erase(sessionId);
	m_PendingPartyTransferResults.erase(sessionId);
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;
	m_CombatObjectRuntime.Cancel_Source(netEntityId);
	if (m_KoukuCardMaze.Remove_Player(playerId))
		Reset_CardMaze();
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_PendingPartyInviteByTargetPlayerId.erase(playerId);
	std::erase_if(m_PendingPartyInviteByTargetPlayerId,
		[playerId](const auto& invite) { return invite.second == playerId; });
	Cancel_RaidEntryProposalsInvolving(playerId);
	Remove_FromParty(playerId);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	if (publishDeparture)
		Broadcast_Despawned(netEntityId, reason);

	std::cout << "Player left. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", RoomPlayers=" << m_Players.size() << '\n';

	if (!Reset_ReplayableArenaWhenEmpty())
	{
		std::cerr << "Replayable arena reset failed. World="
			<< static_cast<unsigned>(m_eWorldId) << ", Status="
			<< m_strStatus << '\n';
	}
	if (!Reset_ValtanArenaWhenEmpty())
	{
		std::cerr << "Valtan arena reset failed: "
			<< m_strStatus << '\n';
	}
}

void LostArk::Server::CGameRoom::Close_SessionForBindingFailure(
	const SESSION_ID sessionId,
	const std::string_view packetName,
	const std::string_view validation)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	session->Request_Close(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		WSAENOTCONN,
		"packet=" + std::string{ packetName } + " validation=" +
			std::string{ validation });
}
```

## Server/Private/GameRoom_WorldEntities.cpp

```cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Rollback_Join(const SESSION_ID sessionId)
{
	using namespace LostArk::Shared;
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
}

bool LostArk::Server::CGameRoom::Is_PlayerAdmissionFull() const
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		Count_HumanPlayers() >= LostArk::Shared::MAX_VALTAN_RAID_PLAYERS)
	{
		return true;
	}
	return nullptr == Find_AvailablePlayerSpawn();
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_AvailablePlayerSpawn() const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN)
		{
			continue;
		}
		const bool isUsed = std::any_of(
			m_Players.begin(), m_Players.end(),
			[&placement](const auto& playerEntry)
			{
				return playerEntry.second.strSpawnPlacementId ==
					placement.strPlacementId;
			});
		if (!isUsed)
			return &placement;
	}
	return nullptr;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_Placement(
	const std::string& placementId) const
{
	const auto& placements = m_WorldBootstrap.Get_Placements();
	const auto iter = std::find_if(
		placements.begin(),
		placements.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == placementId;
		});
	return placements.end() != iter ? &*iter : nullptr;
}

bool LostArk::Server::CGameRoom::Build_WorldEntity(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	SERVER_WORLD_ENTITY& outEntity,
	const CGameplayCatalog* definitionCatalog,
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId, const std::uint32_t ownerPatternSequence)
{
	const CGameplayCatalog& catalog = nullptr == definitionCatalog ?
		m_GameplayCatalog.Active() : *definitionCatalog;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::WORLD_OBJECT == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind ||
		(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId &&
			(WORLD_BOOTSTRAP_KIND::BOSS != placement.eKind ||
			 ownerBossNetEntityId == netEntityId)))
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.iOwnerBossNetEntityId = ownerBossNetEntityId;
	staged.PinnedDefinitionRevision =
		catalog.Get_ActiveRevision();
	if (!staged.PinnedDefinitionRevision.Is_Valid())
	{
		m_strStatus = "Active gameplay revision is unavailable";
		return false;
	}
	staged.strPlacementId = placement.strPlacementId;
	staged.strArchetypeId = placement.strArchetypeId;
	staged.strEncounterId = placement.strEncounterId;
	staged.eKind = placement.eKind;
	staged.fPositionX = placement.fPositionX;
	staged.fPositionY = placement.fPositionY;
	staged.fPositionZ = placement.fPositionZ;
	staged.fYawDegrees = placement.fYawDegrees;
	if (WORLD_BOOTSTRAP_KIND::NPC == staged.eKind)
	{
		/* NPCs are non-combat living bodies. Keep their liveness explicit so
		the body rebuild cannot silently drop them through a zero HP default.
		Their wire collision radius stays zero by Shared contract; the Server's
		blocking-body rebuild owns the separate player-sized NPC body. */
		staged.iCurrentHp = 1u;
		staged.iMaximumHp = 1u;
		staged.fCollisionRadius = 0.f;
		staged.strActionId = CNpcBehaviorRuntime::IDLE_ACTION_ID;
		staged.iActionStartTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		if (placement.bHasNpcBehavior &&
			!m_NpcBehaviorRuntime.Initialize(
				placement, m_ServerNavigation,
				staged.iActionStartTick, staged, m_strStatus))
		{
			return false;
		}
	}
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			catalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			catalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		const bool isDependentArchetype = std::any_of(patterns->begin(), patterns->end(),
			[&staged](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
					pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
			});
		const bool isKoukuClone = LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId &&
			LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
			staged.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
		if ((isDependentArchetype || isKoukuClone) !=
			(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId))
		{
			m_strStatus = "Boss archetype requires its declared primary/dependent spawn role";
			return false;
		}
		if (isDependentArchetype || isKoukuClone)
		{
			const auto liveOwner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
				[ownerBossNetEntityId](const SERVER_WORLD_ENTITY& candidate)
				{ return candidate.iNetEntityId == ownerBossNetEntityId; });
			const SERVER_WORLD_ENTITY* owner = isKoukuClone && ownerPatternSequence ?
				Find_KoukuOccurrenceOwner(ownerBossNetEntityId, ownerPatternSequence) :
				(liveOwner == m_WorldEntities.end() ? nullptr : &*liveOwner);
			if (!owner || liveOwner == m_WorldEntities.end() || !liveOwner->iCurrentHp ||
				liveOwner->eAction == SERVER_ENTITY_ACTION::DEAD || liveOwner->bMechanicLedgerRequiresReset ||
				WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
				LostArk::Shared::INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
				0u == owner->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
				owner->bMechanicLedgerRequiresReset || owner->strEncounterId != staged.strEncounterId ||
				owner->PinnedDefinitionRevision != catalog.Get_ActiveRevision())
			{
				m_strStatus = "Dependent boss requires a live primary in its pinned encounter";
				return false;
			}
			const auto* ownerPatterns = patterns;
			// A live run can pin a newer Product than the base combat catalog.
			if (isKoukuClone)
				if (const auto* product = Resolve_KoukuProductCatalog())
					ownerPatterns = product->Find_BossPatterns(owner->strEncounterId);
			const bool ownerRunsFinale = ownerPatterns && std::any_of(ownerPatterns->begin(), ownerPatterns->end(),
				[&owner, &staged, isKoukuClone](const BOSS_PATTERN_DEFINITION& pattern)
				{
					if (isKoukuClone)
						return pattern.strPatternId == owner->strPatternId &&
							owner->strArchetypeId == staged.strArchetypeId &&
							std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
								[](const BOSS_PATTERN_MECHANIC_TRIGGER& trigger)
								{ return BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT == trigger.eKind ||
									(BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES == trigger.eKind && trigger.DirectionPatternIds.size() == 4u) ||
									(BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS == trigger.eKind && !trigger.PatternSpawns.empty()); });
					const bool directFinaleOccurrence =
						pattern.strPatternId == owner->strPatternId;
					const bool phaseThreeFinaleController =
						owner->bGhostPhasePatternLoopActive && 3u == owner->iPhase &&
						"BOSS_VALTAN" == owner->strArchetypeId &&
						"boss.valtan.center" == owner->strPlacementId &&
						"VALTAN_GHOST_FINALE" == pattern.strPatternId;
					return (directFinaleOccurrence || phaseThreeFinaleController) &&
						BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
						pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
				});
			if (!ownerRunsFinale)
			{
				m_strStatus = "Dependent boss owner is not running its declared finale";
				return false;
			}
		}
		const std::vector<BOSS_PART_DEFINITION>* bossParts =
			catalog.Find_BossParts(staged.strArchetypeId);
		const std::vector<BOSS_PART_DEFINITION> noBossParts;
		std::string combatStatus;
		if (!CBossCombatRuntime::Initialize(
			staged.BossCombat,
			nullptr == bossParts ? noBossParts : *bossParts,
			combatStatus))
		{
			m_strStatus = std::move(combatStatus);
			return false;
		}
		staged.iCurrentHp = profile->iMaximumHp;
		staged.iMaximumHp = profile->iMaximumHp;
		staged.iMaximumHealthBars = profile->iMaximumHealthBars;
		staged.iLastEvaluatedHealthBar = profile->iMaximumHealthBars;
		staged.iAttackPower = profile->iAttackPower;
		staged.fCollisionRadius = profile->fCollisionRadius;
		staged.fEngageDistance = profile->fEngageDistance;
		staged.fMoveSpeed = profile->fMoveSpeed;
		staged.PhasePolicy = profile->PhasePolicy;
		staged.iPhaseTwoHpPercent = profile->PhasePolicy.iThresholdPercent;
		/* Every plate starts intact. A boss with no authored plates keeps an
		empty list and therefore no mitigation, which is the pre-armour rule. */
		staged.ArmorPlates.clear();
		staged.ArmorPlates.reserve(profile->ArmorPlates.size());
		for (const BOSS_ARMOR_PLATE& plate : profile->ArmorPlates)
		{
			SERVER_BOSS_ARMOR_PLATE_STATE state{};
			state.iPlateIndex = plate.iPlateIndex;
			state.iRemainingDurability = plate.iDurability;
			state.iDefense = plate.iDefense;
			staged.ArmorPlates.push_back(state);
		}
		if (m_ServerNavigation.Is_Loaded())
		{
			/* An authored placement on a walkable cell keeps its exact XZ and
			only takes the grid height. Snapping it to the cell centre moved the
			KoukuSaydon gate bosses up to 2.8 m on that area's 4 m grid, onto
			the fixed player position the gate teleports to. Off-grid or
			blocked placements still project to the nearest walkable cell. */
			/* Big Saydon stands above the G2 floor. Its saved transform owns
			the height while navigation still admits its XZ footprint. */
			const bool preserveAuthoredHeight =
				LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
				staged.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
			SERVER_NAV_POINT projected{};
			if (m_ServerNavigation.Is_PointWalkableExact(
					staged.fPositionX, staged.fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					staged.fPositionX, staged.fPositionZ, projected))
			{
				if (!preserveAuthoredHeight)
					staged.fPositionY = projected.y;
			}
			else if (m_ServerNavigation.Project_Point(
				staged.fPositionX,
				staged.fPositionZ,
				projected))
			{
				staged.fPositionX = projected.x;
				if (!preserveAuthoredHeight)
					staged.fPositionY = projected.y;
				staged.fPositionZ = projected.z;
			}
			else
			{
				m_strStatus = "Boss placement is outside server navigation";
				return false;
			}
		}
	}
	staged.fSpawnPositionX = staged.fPositionX;
	staged.fSpawnPositionY = staged.fPositionY;
	staged.fSpawnPositionZ = staged.fPositionZ;
	outEntity = std::move(staged);
	return true;
}

bool LostArk::Server::CGameRoom::Initialize_WorldEntities()
{
	m_WorldEntities.clear();
	m_KoukuCardRainSoldiers.clear();
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::COLLISION_BOX)
		{
			continue;
		}
		if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		{
			m_strStatus = "World entity ID space exhausted";
			return false;
		}
		SERVER_WORLD_ENTITY entity{};
		if (!Build_WorldEntity(placement, m_iNextNetEntityId, entity))
			return false;
		++m_iNextNetEntityId;
		m_WorldEntities.push_back(std::move(entity));
	}
	{
		/* The Bern3 ship NPCs were once spawned here yet never seen on screen: name each one the room
		   really holds so a missing NPC can be told apart from a Client-side presentation failure. */
		std::size_t shipNpcCount = 0u;
		for (const SERVER_WORLD_ENTITY& spawned : m_WorldEntities)
		{
			if (WORLD_BOOTSTRAP_KIND::NPC != spawned.eKind ||
				0 != spawned.strArchetypeId.rfind("NPC_SHIP_", 0))
				continue;
			++shipNpcCount;
			std::cout << "[ShipNpc] spawned placement=" << spawned.strPlacementId
				<< " archetype=" << spawned.strArchetypeId << " pos=(" << spawned.fPositionX << ", "
				<< spawned.fPositionY << ", " << spawned.fPositionZ << ")\n";
		}
		if (0u != shipNpcCount)
			std::cout << "[ShipNpc] " << shipNpcCount << " ship NPC(s) live in this world\n";
	}
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ReplayableArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (m_eWorldId == WORLD_ID::MAHARAKA && Count_HumanPlayers() == 0u)
		m_MaharakaWaterpangIntro.reset();
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId &&
		WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId) || Count_HumanPlayers() != 0u)
		return true;

	Clear_KoukuSaydonPatternAudition();
	Update_KoukuWorldBodies(m_iServerTick);
	// A fresh room must not inherit the departed raid's gate vote or clear state.
	// The empty-player guard above preserves an encounter still owned by its party.
	m_GateProgress = {};
	if (WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId)
	{
		Stop_KoukuBingoDuration(true);
		Reset_CardMaze();
		m_KoukuRaid = {};
		m_KoukuRaidReceipts.clear();
		m_PendingKoukuMechanicTriggers.clear();
		m_PendingKoukuMarioEntries.clear();
		std::fill(std::begin(m_MarioPoppedBalls), std::end(m_MarioPoppedBalls), 0u);
	}
	m_iNextMarioEntryStage = 1u;
	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.trigger-system");
		return false;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.spawn-groups");
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("empty-arena-reset.world-entities");
		return false;
	}
	m_CombatObjectRuntime.Reset();
	m_CombatObjectRuntime.Discard_PendingLifecycle();
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	m_iNextBossCombatEventSequence = 1u;
	m_ValtanPatternIdAuditionSequenceBySessionId.clear();
	m_ValtanPatternFlowStartSequenceBySessionId.clear();
	m_ValtanPatternFlowControlSequenceBySessionId.clear();
	m_KoukuSaydonPatternAuditionReceiptBySessionId.clear();
	m_PendingKoukuSaydonPatternAuditionLifecycle.clear();
	Cancel_ValtanPatternIdAudition("room reset after the last player left");
	m_ValtanNextPatternReceiptBySessionId.clear();
	m_ValtanPatternFlowAudition = {};
	m_ValtanFightPageStart = {};
	m_strStatus = "Replayable arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId || Count_HumanPlayers() != 0u)
		return true;

	std::string resetStatus;
	const std::uint32_t resetTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.spawn-groups");
		return false;
	}
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
	m_ValtanTimelineAudition = {};
	m_ValtanFightPageStart = {};
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!m_EncounterPropRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.encounter-props");
		return false;
	}
	if (!m_WorldDestructionRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.world-destruction");
		return false;
	}
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iNextBossCombatEventSequence = 1u;
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("valtan-empty-reset.world-entities");
		return false;
	}
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	/* The encounter is fresh, so a bar armed by the previous occupants no
	longer describes any live boss. Leave already dropped their sequences. */
	m_iValtanAuditionArmedHealthBar = 0u;
	// The next party charges its Esther gauge from zero; any live summon was
	// already discarded with the entity rebuild above, and a summon still in
	// its landing delay has no party left to land for.
	m_PendingEstherSummons.clear();
	m_EstherZones.clear();
	m_EstherSkillSystem.Reset();
	m_bValtanRaidCleared = false;
	m_strStatus = "Valtan arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Apply_EncounterPropStageEntry(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_EncounterPropRuntime.Is_Initialized() ||
		0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strPatternStageId.empty())
	{
		return true;
	}
	/* A later pattern owns each shatter, because the stele set outlives the
	   pattern that raised it. The stage edge names the pair it breaks, so the
	   pair that goes leaves the opposite pair standing as the cover the raid
	   moves to. */
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Encounter prop pinned gameplay generation is missing";
		return false;
	}
	if (const std::vector<BOSS_PATTERN_DEFINITION>* propBreakPatterns =
		occurrenceCatalog->Find_BossPatterns(boss.strEncounterId))
	{
		const auto propBreakPattern = std::find_if(
			propBreakPatterns->begin(), propBreakPatterns->end(),
			[&boss](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == boss.strPatternId; });
		if (propBreakPatterns->end() != propBreakPattern &&
			boss.iPatternStageIndex < propBreakPattern->Stages.size())
		{
			const BOSS_PATTERN_STAGE_DEFINITION& propBreakStage =
				propBreakPattern->Stages[boss.iPatternStageIndex];
			const auto& propSlots = m_EncounterPropRuntime.Get_SlotStates();
			bool allowScriptedPropBreak = false;
			allowScriptedPropBreak = boss.bScriptedPatternPlayback &&
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
					m_ValtanTimelineAudition.ePhase &&
				boss.iNetEntityId == m_ValtanTimelineAudition.iBossEntityId &&
				m_ValtanTimelineAudition.bAllowProductPropBreak;
			/* Only a raised pair can shatter. The wave is an ordinary rotation
			   pattern that also runs when no stele stands, and asking to break a
			   hidden slot is a rejection, not a no-op. */
			const bool everyNamedSlotRaised =
				!propBreakStage.PropBreakSlotIds.empty() &&
				/* Generic scripted playback suppresses incidental prop breaks. A
				   timeline row that explicitly prepared four intact pillars is the
				   one exception: its real red-blade stages own the two pairs. */
				(!boss.bScriptedPatternPlayback || allowScriptedPropBreak) &&
				propBreakStage.strPropBreakSetId ==
					m_EncounterPropRuntime.Get_PropSetId() &&
				propBreakStage.strActionId == boss.strActionId &&
				std::all_of(
					propBreakStage.PropBreakSlotIds.begin(),
					propBreakStage.PropBreakSlotIds.end(),
					[&propSlots](const std::string& slotId)
					{
						const auto found = std::find_if(
							propSlots.begin(), propSlots.end(),
							[&slotId](const ENCOUNTER_PROP_SLOT_STATE& candidate)
							{ return candidate.strSlotId == slotId; });
						return propSlots.end() != found &&
							LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT ==
								found->eState;
					});
			if (everyNamedSlotRaised)
			{
				ENCOUNTER_PROP_TRANSACTION breakTransaction{};
				std::string breakStatus;
				const ENCOUNTER_PROP_PREPARE_RESULT breakResult =
					m_EncounterPropRuntime.Prepare_BreakSlots(
						propBreakStage.PropBreakSlotIds,
						m_EncounterPropRuntime.Get_OccurrenceSequence(),
						serverTick, breakTransaction, breakStatus);
				if (ENCOUNTER_PROP_PREPARE_RESULT::READY == breakResult)
				{
					if (!m_EncounterPropRuntime.Commit(breakTransaction, breakStatus))
					{
						m_strStatus = std::move(breakStatus);
						return false;
					}
					Broadcast_EncounterPropSync();
				}
				else if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE != breakResult)
				{
					m_strStatus = std::move(breakStatus);
					return false;
				}
			}
		}
	}
	if (PILLAR_PATTERN_ID != boss.strPatternId ||
		PILLAR_SPAWN_STAGE_ID != boss.strPatternStageId)
	{
		return true;
	}

	ENCOUNTER_PROP_TRANSACTION transaction{};
	std::string status;
	const ENCOUNTER_PROP_PREPARE_RESULT result =
		m_EncounterPropRuntime.Prepare_Spawn(
			boss.iPatternSequence, serverTick, transaction, status);
	if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE == result)
		return true;
	if (ENCOUNTER_PROP_PREPARE_RESULT::READY != result ||
		!m_EncounterPropRuntime.Commit(transaction, status))
	{
		/* A new occurrence cannot claim slots that the previous one still owns.
		   Preserve the prop failure so this tick cannot publish completion or
		   promote Next after an uncommitted stage entry. */
		m_strStatus = std::move(status);
		return false;
	}
	/* The Debug audition asked for a whole cycle, so the raise it just observed
	   schedules the shatter the product path has no owner for yet. */
	if (m_bPillarAuditionCycleArmed)
	{
		m_iPillarAuditionBreakTick =
			Add_ServerTicksSkippingReservedZero(
				serverTick, PILLAR_AUDITION_DWELL_TICKS);
		m_bPillarAuditionCycleArmed = false;
	}
	Broadcast_EncounterPropSync();
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageActions(
	SERVER_WORLD_ENTITY& boss,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	const std::uint32_t spawnWaveOrdinal,
	const bool scheduledSpawnWave)
{
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Boss stage action pinned gameplay generation is missing";
		return false;
	}
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	if (!Stage_BossPatternStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger, serverTick,
		stagedCombat, stagedGameplayPhase, transaction, spawnWaveOrdinal,
		scheduledSpawnWave))
	{
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	if (!scheduledSpawnWave && !Commit_BossPatternPlayerStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger,
		serverTick, spawnWaveOrdinal))
	{
		m_strStatus = "Boss player stage action commit failed";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Resolve_ArenaRandomVolleyOrigins(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const BOSS_COMBAT_OBJECT_DEFINITION& definition,
	const std::uint32_t spawnWaveOrdinal,
	std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
	const float explicitMinimumSpacingM, const SERVER_NAV_POINT* anchorOverride)
{
	outOrigins.clear();
	const BOSS_COMBAT_OBJECT_VOLLEY& volley = action.Volley;
	const SERVER_NAV_POINT anchor = anchorOverride ? *anchorOverride : SERVER_NAV_POINT{boss.fSpawnPositionX, boss.fSpawnPositionY, boss.fSpawnPositionZ};
	if (0u == volley.iArenaRandomCount)
		return true;
	if (!m_ServerNavigation.Is_Loaded() ||
		BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION !=
			volley.eArenaAnchorPolicy ||
		0u == boss.iNetEntityId || 0u == boss.iPatternSequence ||
		!std::isfinite(anchor.x) ||
		!std::isfinite(anchor.y) ||
		!std::isfinite(anchor.z) ||
		!std::isfinite(volley.fArenaRandomRadiusM) ||
		volley.fArenaRandomRadiusM <= 0.f ||
		!std::isfinite(volley.fArenaHeightToleranceM) ||
		volley.fArenaHeightToleranceM <= 0.f)
	{
		m_strStatus = "Boss arena-random volley contract is invalid";
		return false;
	}

	// Presentation-only volleys have no damage radius from which to derive spacing.
	const float minimumSpacing = explicitMinimumSpacingM == 0.f ?
		Resolve_VolleyMinimumSpacing(definition) : explicitMinimumSpacingM;
	if (!std::isfinite(minimumSpacing) || minimumSpacing <= 0.f)
	{
		m_strStatus = "Boss arena-random volley spacing is invalid";
		return false;
	}
	const float minimumSpacingSquared = minimumSpacing * minimumSpacing;
	/* Arena-random authoring defines a valid origin, not a guarantee that every
	   cell under the 3.5m damage circle is walkable. The Valtan nav paint has
	   intentional seams inside the arena, so admission pins the exact centre to
	   authoritative navigation/height and separates centres by the authored
	   spacing or the existing damage diameter. */
	const auto IsSpawnPointWalkable =
		[this, &anchor, &volley](
			const float centerX, const float centerZ, float& outY)
		{
			SERVER_NAV_POINT center{};
			if (!m_ServerNavigation.Is_PointWalkableExact(centerX, centerZ) ||
				!m_ServerNavigation.Sample_Position(centerX, centerZ, center) ||
				!std::isfinite(center.y) ||
				std::abs(center.y - anchor.y) >
					volley.fArenaHeightToleranceM)
			{
				return false;
			}
			outY = center.y;
			return true;
		};

	try
	{
		outOrigins.reserve(volley.iArenaRandomCount);
	}
	catch (const std::bad_alloc&)
	{
		m_strStatus = "Boss arena-random volley allocation failed";
		return false;
	}
	const std::uint64_t baseSeed = Mix_DeterministicRandom(
		Hash_StableId(action.strTargetId) ^
		(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
		static_cast<std::uint64_t>(boss.iPatternSequence) ^
		(static_cast<std::uint64_t>(spawnWaveOrdinal) << 48u));
	for (std::uint32_t attempt = 0u;
		attempt < ARENA_RANDOM_MAXIMUM_ATTEMPTS &&
		outOrigins.size() < volley.iArenaRandomCount;
		++attempt)
	{
		const std::uint64_t attemptSeed = baseSeed ^
			(static_cast<std::uint64_t>(attempt + 1u) *
				0xd6e8feb86659fd93ull);
		const float radius = std::sqrt(
			DeterministicUnitFloat(attemptSeed)) *
			volley.fArenaRandomRadiusM;
		const float angle = DeterministicUnitFloat(
			attemptSeed ^ 0xa0761d6478bd642full) * TWO_PI;
		const float x = anchor.x + std::cos(angle) * radius;
		const float z = anchor.z + std::sin(angle) * radius;
		float y = 0.f;
		if (!IsSpawnPointWalkable(x, z, y))
			continue;
		const bool overlaps = std::any_of(
			outOrigins.begin(), outOrigins.end(),
			[x, z, minimumSpacingSquared](
				const SERVER_COMBAT_OBJECT_LOCKED_TARGET& existing)
			{
				const float deltaX = x - existing.fPositionX;
				const float deltaZ = z - existing.fPositionZ;
				return deltaX * deltaX + deltaZ * deltaZ +
					VOLLEY_SPACING_EPSILON < minimumSpacingSquared;
			});
		if (overlaps)
			continue;
		SERVER_COMBAT_OBJECT_LOCKED_TARGET origin{};
		origin.fPositionX = x;
		origin.fPositionY = y;
		origin.fPositionZ = z;
		origin.bTrackUntilFirstPulse = false;
		outOrigins.push_back(origin);
	}
	if (outOrigins.size() != volley.iArenaRandomCount)
	{
		outOrigins.clear();
		m_strStatus = "Boss arena-random volley has no valid point set";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternScheduledSpawnWave(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (boss.strPatternId.empty() || boss.strActionId.empty() ||
		0u == boss.iActionStartTick)
	{
		return true;
	}
	const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == catalog)
	{
		m_strStatus = "Boss scheduled volley pinned gameplay generation is missing";
		return false;
	}
	const auto* patterns = catalog->Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Boss scheduled volley encounter is missing";
		return false;
	}
	const auto pattern = std::find_if(
		patterns->begin(), patterns->end(),
		[&boss](const BOSS_PATTERN_DEFINITION& candidate)
		{ return candidate.strPatternId == boss.strPatternId; });
	if (patterns->end() == pattern)
	{
		m_strStatus = "Boss scheduled volley pattern is missing";
		return false;
	}
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&boss](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{ return candidate.strActionId == boss.strActionId; });
	if (pattern->Stages.end() == stage)
	{
		m_strStatus = "Boss scheduled volley stage is missing";
		return false;
	}
	const BOSS_PATTERN_STAGE_ACTION* scheduledAction = nullptr;
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
				action.eKind ||
			(0u == action.Volley.iFirstSpawnOffsetMs &&
			 action.Volley.iSpawnCount <= 1u))
		{
			continue;
		}
		if (nullptr != scheduledAction &&
			(scheduledAction->Volley.iFirstSpawnOffsetMs !=
				action.Volley.iFirstSpawnOffsetMs ||
			 scheduledAction->Volley.iSpawnCount != action.Volley.iSpawnCount ||
			 scheduledAction->Volley.iSpawnIntervalMs !=
				action.Volley.iSpawnIntervalMs))
		{
			m_strStatus = "Boss scheduled volleys do not share one clock";
			return false;
		}
		scheduledAction = &action;
	}
	if (nullptr == scheduledAction)
		return true;
	if (0u == boss.iAppliedPatternStageSpawnWaveCount &&
		0u == scheduledAction->Volley.iFirstSpawnOffsetMs)
	{
		m_strStatus = "Boss scheduled volley ENTER wave was not committed";
		return false;
	}
	if (boss.iAppliedPatternStageSpawnWaveCount >=
		scheduledAction->Volley.iSpawnCount)
	{
		return true;
	}
	const std::uint32_t waveOrdinal =
		boss.iAppliedPatternStageSpawnWaveCount;
	const std::uint64_t dueMilliseconds =
		static_cast<std::uint64_t>(scheduledAction->Volley.iFirstSpawnOffsetMs) +
		static_cast<std::uint64_t>(waveOrdinal) *
			scheduledAction->Volley.iSpawnIntervalMs;
	const std::uint64_t elapsedTicks =
		Elapsed_ServerTicksSkippingReservedZero(
			boss.iActionStartTick, serverTick);
	if (elapsedTicks * 1000ull <
		dueMilliseconds * static_cast<std::uint64_t>(SERVER_TICK_HZ))
	{
		return true;
	}
	if (!Apply_BossPatternStageActions(
		boss, boss.strPatternId, boss.strActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
		serverTick, waveOrdinal, true))
	{
		return false;
	}
	++boss.iAppliedPatternStageSpawnWaveCount;
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageTransition(
	SERVER_WORLD_ENTITY& boss,
	const std::string& previousPatternId,
	const std::string& previousActionId,
	const std::string& nextPatternId,
	const std::string& nextActionId,
	const LostArk::Shared::GameplayDataRevision& previousDefinitionRevision,
	const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
	const std::uint32_t serverTick)
{
	const std::string& failurePatternId =
		nextPatternId.empty() ? previousPatternId : nextPatternId;
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	const CGameplayCatalog* previousCatalog = previousPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(previousDefinitionRevision);
	const CGameplayCatalog* nextCatalog = nextPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(nextDefinitionRevision);
	if (nullptr == previousCatalog || nullptr == nextCatalog)
	{
		m_strStatus = "Boss stage transition pinned gameplay generation is missing";
		return false;
	}
	if (!Stage_BossPatternStageActions(
		boss, *previousCatalog, previousPatternId, previousActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick,
		stagedCombat, stagedGameplayPhase, transaction) ||
		!Stage_BossPatternStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick,
			stagedCombat, stagedGameplayPhase, transaction))
	{
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	std::uint32_t nextStageAppliedSpawnWaveCount =
		nextActionId.empty() ? 0u : 1u;
	if (!nextActionId.empty())
	{
		const auto* nextPatterns = nextCatalog->Find_BossPatterns(
			boss.strEncounterId);
		if (nullptr == nextPatterns)
		{
			m_strStatus = "Boss next-stage scheduled volley encounter is missing";
			return false;
		}
		const auto nextPattern = std::find_if(
			nextPatterns->begin(), nextPatterns->end(),
			[&nextPatternId](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == nextPatternId; });
		if (nextPatterns->end() == nextPattern)
		{
			m_strStatus = "Boss next-stage scheduled volley pattern is missing";
			return false;
		}
		const auto nextStage = std::find_if(
			nextPattern->Stages.begin(), nextPattern->Stages.end(),
			[&nextActionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
			{ return candidate.strActionId == nextActionId; });
		if (nextPattern->Stages.end() == nextStage)
		{
			m_strStatus = "Boss next-stage scheduled volley owner is missing";
			return false;
		}
		for (const BOSS_PATTERN_STAGE_ACTION& action : nextStage->Actions)
		{
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
					action.eKind ||
				(0u == action.Volley.iFirstSpawnOffsetMs &&
				 action.Volley.iSpawnCount <= 1u))
			{
				continue;
			}
			nextStageAppliedSpawnWaveCount =
				0u == action.Volley.iFirstSpawnOffsetMs ? 1u : 0u;
			break;
		}
	}
	WORLD_DESTRUCTION_TRANSACTION worldTransaction{};
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> worldEvents;
	bool hasWorldTransaction = false;
	if (m_WorldDestructionRuntime.Is_Initialized() &&
		0u != boss.iNetEntityId && 0u != boss.iPatternSequence &&
		!boss.strPatternId.empty() && !boss.strPatternStageId.empty() &&
		!boss.strActionId.empty())
	{
		WORLD_DESTRUCTION_ACTION_TUPLE worldAction{};
		worldAction.strPatternId = boss.strPatternId;
		worldAction.strStageId = boss.strPatternStageId;
		worldAction.strActionId = boss.strActionId;
		worldAction.iStageIndex = boss.iPatternStageIndex;
		std::string worldStatus;
		const WORLD_DESTRUCTION_PREPARE_RESULT worldResult =
			m_WorldDestructionRuntime.Prepare_StageTrigger(
				worldAction, boss.iNetEntityId, boss.iPatternSequence,
				serverTick, worldTransaction, worldStatus);
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == worldResult)
		{
			std::vector<SERVER_COLLISION_STATE_CHANGE> collisionChanges;
			std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> navigationChanges;
			SERVER_COLLISION_STATE_STAGE collisionStage{};
			SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
			Build_WorldDestructionStateChanges(
				worldTransaction, collisionChanges, navigationChanges);
			if (!Build_WorldDestructionLiveEvents(
					worldTransaction, boss, worldEvents, worldStatus) ||
				!m_ServerCollisionSystem.Prepare_StateChanges(
					collisionChanges, collisionStage, worldStatus) ||
				!m_ServerNavigation.Prepare_ConditionChanges(
					navigationChanges, navigationStage, worldStatus))
			{
				m_strStatus = std::move(worldStatus);
				CValtanBrain::Fail_Mechanic(
					boss, failurePatternId,
					SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
					serverTick);
				return false;
			}
			hasWorldTransaction = true;
		}
		else if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE != worldResult)
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
				serverTick);
			return false;
		}
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage transition combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	/* A delayed first wave is intentionally absent from the ENTER transaction;
	   ordinal zero remains pending until its exact fixed-tick due time. */
	boss.iAppliedPatternStageSpawnWaveCount = nextStageAppliedSpawnWaveCount;
	if (hasWorldTransaction)
	{
		std::string worldStatus;
		if (!Commit_WorldDestructionTransaction(
			worldTransaction, worldEvents, serverTick, worldStatus))
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT,
				serverTick);
			return false;
		}
		if (!worldEvents.empty())
		{
			const std::uint64_t lastSequence =
				worldEvents.back().iEventSequence;
			m_iNextWorldDestructionEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
					0u : lastSequence + 1u;
		}
	}
	if (!Commit_BossPatternPlayerStageActions(
			boss, *previousCatalog, previousPatternId, previousActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick) ||
		!Commit_BossPatternPlayerStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick))
	{
		m_strStatus = "Boss player stage action commit failed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	return true;
}
```

## Server/Private/ServerTriggerSystem.cpp

```cpp
#include "ServerTriggerSystem.h"

#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace
{
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;

	/* Which trigger actions still fire the moment a player steps into the box.
	   EVERYTHING ELSE waits for the player to press G inside the box: the Server
	   offers the prompt on entry and the G request fires it (Activate_Interact /
	   Activate_Here), re-checking the volume. A row is one (world, action kind,
	   optional id prefix); WORLD_ID::END matches every world, and a null prefix
	   matches every placement id of that kind. These are scripted flows whose entry IS
	   the flow, so making them wait for G would stall the raid or the cutscene:
		PLAY_SEQUENCE, any world   cutscene / mechanic beats (Kouku Mario intro, paper, showtime)
		CLAIM_CARD_MAZE_TELESCOPE  card maze strike volume (never fired by entry or G anyway)
		VALTAN_ARENA spawn group   corridor waves (Stage_1, Stage_2, Stage_MiniBoss_Spawn)
		VALTAN_ARENA encounter     boss start (Stage_Boss)
		KAKULSAYDON_ARENA spawn group  start-area book waves (ids Book1_Monsters, Book2_Monsters)
		BERN movePlayer            castle / library travel boxes (ids castle, castle.2, library, library.2)
	   Everything the PLAYER does to move -- Mario crossings (jump down, climb, cross a gap),
	   the Mario terminal exits, jump.*, the Valtan start box and the other Valtan movePlayer
	   boxes -- is deliberately NOT here: it waits for G. A Mario lane box still goes through
	   the room's Mario admission (SERVER_TRIGGER_MOVE_ENTRY_HANDLER) when G runs it.
	   Delete a row to make that kind G-only. Add a row to make one fire on entry.
	   An authored requiresInteract flag always wins over this table. */
	struct AUTO_ENTRY_RULE final
	{
		LostArk::Shared::WORLD_ID eWorld;
		LostArk::Server::WORLD_TRIGGER_ACTION_KIND eKind;
		const char* pIdPrefix;
	};
	constexpr AUTO_ENTRY_RULE AUTO_ENTRY_RULES[] =
	{
		{ LostArk::Shared::WORLD_ID::END, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE, nullptr },
		{ LostArk::Shared::WORLD_ID::END, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE, nullptr },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP, nullptr },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER, nullptr },
		{ LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP, "Book" },
		{ LostArk::Shared::WORLD_ID::BERN, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER, "castle" },
		{ LostArk::Shared::WORLD_ID::BERN, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER, "library" },
	};

	/* Valtan boss start. The Stage_Boss box starts the boss encounter and also sends
	   the player who fired it to the Stage_Boss_ArenaEntry box, at that box's centre.
	   Every player who fires it is sent, including ones who arrive after the boss is
	   already up, so a raid does not have to walk in one by one. */
	constexpr const char* VALTAN_BOSS_START_TRIGGER_ID = "Stage_Boss";
	constexpr const char* VALTAN_ARENA_ENTRY_TRIGGER_ID = "Stage_Boss_ArenaEntry";

	/* Debug F1 "Normal Monster 1/2". Only these four boxes are handed to the buttons;
	   Stage_MiniBoss_Spawn, Stage_3 (a movePlayer, unrelated to Stage_2's group
	   spawn.valtan.stage03), Stage_Boss and every other box keep firing. */
	constexpr LostArk::Server::WAVE_MONSTER_BUTTON_ROW WAVE_MONSTER_BUTTON_ROWS[] =
	{
		{ LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA, LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Book1_Monsters", "spawn.kouku.book1" },
		{ LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA, LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Book2_Monsters", "spawn.kouku.book2" },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Stage_1", "spawn.valtan.stage01" },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Stage_2", "spawn.valtan.stage03" },
	};
}

const LostArk::Server::WAVE_MONSTER_BUTTON_ROW*
LostArk::Server::CServerTriggerSystem::Find_WaveMonsterButton(
	const LostArk::Shared::WORLD_ID worldId,
	const LostArk::Shared::WAVE_MONSTER_BUTTON button)
{
	const auto found = std::find_if(
		std::begin(WAVE_MONSTER_BUTTON_ROWS), std::end(WAVE_MONSTER_BUTTON_ROWS),
		[worldId, button](const WAVE_MONSTER_BUTTON_ROW& row)
		{
			return row.eWorld == worldId && row.eButton == button;
		});
	return std::end(WAVE_MONSTER_BUTTON_ROWS) == found ? nullptr : &*found;
}

bool LostArk::Server::CServerTriggerSystem::Is_WaveMonsterTrigger(
	const LostArk::Shared::WORLD_ID worldId,
	const WORLD_BOOTSTRAP_PLACEMENT& placement)
{
	if (WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != placement.eKind ||
		1u != placement.TriggerActions.size() ||
		WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP !=
			placement.TriggerActions.front().eKind)
	{
		return false;
	}
	return std::any_of(
		std::begin(WAVE_MONSTER_BUTTON_ROWS), std::end(WAVE_MONSTER_BUTTON_ROWS),
		[worldId, &placement](const WAVE_MONSTER_BUTTON_ROW& row)
		{
			return row.eWorld == worldId &&
				placement.strPlacementId == row.pTriggerPlacementId &&
				placement.TriggerActions.front().strTargetId == row.pSpawnGroupId;
		});
}

bool LostArk::Server::CServerTriggerSystem::Is_WaveMonsterSuppressed(
	const RUNTIME_TRIGGER& trigger) const
{
	return m_bSuppressWaveMonsterTriggers &&
		Is_WaveMonsterTrigger(m_eWorldId, trigger.Definition);
}

bool LostArk::Server::CServerTriggerSystem::Initialize(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	std::string& outStatus,
	const bool enableDebugValtanStageBypass)
{
	std::vector<RUNTIME_TRIGGER> staged;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != placement.eKind ||
			!placement.isEnabled)
		{
			continue;
		}
		if (1u != placement.TriggerActions.size() ||
			(WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE !=
				placement.TriggerActions.front().eKind))
		{
			outStatus = "Enabled trigger requires one supported action: " +
				placement.strPlacementId;
			return false;
		}
		RUNTIME_TRIGGER runtime{ placement };
		/* Product decision (2026-09-19): every authored trigger fires again for
		   every player, any number of times. The authored triggerOnce flag stays
		   in the data and in MapTool, but it used to become a room-wide latch: the
		   first player who fired the box spent it for everyone after them, and it
		   only cleared when the room emptied (Character Select, Valtan, Kouku; Bern
		   and the training ground never reset). Only a test opts back in. */
		if (!m_bHonourTriggerOnce)
			runtime.Definition.isTriggerOnce = false;
		staged.push_back(std::move(runtime));
	}
	m_Triggers = std::move(staged);
#ifdef _DEBUG
	m_bDebugValtanStageBypass = enableDebugValtanStageBypass;
#else
	(void)enableDebugValtanStageBypass;
	m_bDebugValtanStageBypass = false;
#endif
	outStatus = "Initialized server triggers: " +
		std::to_string(m_Triggers.size()) +
		(m_bDebugValtanStageBypass ? ", ValtanStageBypass=1" : "");
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Update_PlayerMotion(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
		PLAYER_ACTION_STATE::WALL_CLIMB != player.eAction)
	{
		return false;
	}
	if (!player.TriggerMove.isActive ||
		player.TriggerMove.fDurationSeconds <= 0.f)
	{
		player.TriggerMove = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
		player.PendingCommand.Clear();
		return false;
	}
	if (0u == player.iCurrentHp)
	{
		player.TriggerMove = {};
		return false;
	}

	SERVER_TRIGGER_MOVE& move = player.TriggerMove;
	if (move.fHeldSeconds < move.fHoldSeconds)
	{
		/* Bern travel: the player stays where they stand while the Client darkens the screen;
		   the displacement starts on the tick after the hold is used up. */
		move.fHeldSeconds = (std::min)(
			move.fHoldSeconds, move.fHeldSeconds + fixedDeltaSeconds);
		return true;
	}
	move.fElapsedSeconds = (std::min)(
		move.fDurationSeconds,
		move.fElapsedSeconds + fixedDeltaSeconds);
	const float ratio = move.fElapsedSeconds / move.fDurationSeconds;
	if (move.TrackSamples.empty())
	{
		player.fPositionX = move.fStartX +
			(move.fTargetX - move.fStartX) * ratio;
		player.fPositionY = move.fStartY +
			(move.fTargetY - move.fStartY) * ratio +
			4.f * move.fArcHeight * ratio * (1.f - ratio);
		player.fPositionZ = move.fStartZ +
			(move.fTargetZ - move.fStartZ) * ratio;
	}
	else
	{
		/* TrackMove samples are absolute original positions.  The Server owns
		interpolation between them so every recipient sees its replicated truth,
		rather than a Client-side spline approximation. */
		const float elapsedMs = move.fElapsedSeconds * 1000.f;
		const SERVER_TRIGGER_MOVE_SAMPLE* before = &move.TrackSamples.front();
		const SERVER_TRIGGER_MOVE_SAMPLE* after = &move.TrackSamples.back();
		for (std::size_t index = 1u; index < move.TrackSamples.size(); ++index)
		{
			if (elapsedMs <= static_cast<float>(move.TrackSamples[index].iTimeMs))
			{
				after = &move.TrackSamples[index];
				before = &move.TrackSamples[index - 1u];
				break;
			}
		}
		const float spanMs = static_cast<float>(after->iTimeMs - before->iTimeMs);
		const float local = spanMs > 0.f ? (elapsedMs - static_cast<float>(before->iTimeMs)) / spanMs : 1.f;
		player.fPositionX = before->fPositionX + (after->fPositionX - before->fPositionX) * local;
		player.fPositionY = before->fPositionY + (after->fPositionY - before->fPositionY) * local;
		player.fPositionZ = before->fPositionZ + (after->fPositionZ - before->fPositionZ) * local;
	}

	if (ratio >= 1.f)
	{
		player.fPositionX = move.fTargetX;
		player.fPositionY = move.fTargetY;
		player.fPositionZ = move.fTargetZ;
		if (KOUKU_HUD_MODE::END != move.eKoukuHudModeOnArrival)
		{
			player.Clear_KoukuInteractionState();
			player.eKoukuAreaHudMode = move.eKoukuHudModeOnArrival;
			player.eMadnessForm = KOUKU_HUD_MODE::NONE == move.eKoukuHudModeOnArrival ?
				PLAYER_MADNESS_FORM::NORMAL : PLAYER_MADNESS_FORM::CLOWN;
		}
		move = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
		player.PendingCommand.Clear();
	}
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Run_Action(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget) const
{
	const WORLD_TRIGGER_ACTION& action =
		trigger.Definition.TriggerActions.front();
	bool fired = false;
	if (WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == action.eKind)
	{
		fired = Begin_MovePlayer(player, action, actionStartTick);
		if (fired)
		{
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			/* Every Bern movement box travels behind a blackout: hold the player in place until
			   the Client screen is dark (Shared BERN_TRAVEL_HOLD_MS). Other worlds move at once. */
			if (LostArk::Shared::WORLD_ID::BERN == m_eWorldId)
			{
				player.TriggerMove.fHoldSeconds =
					static_cast<float>(LostArk::Shared::BERN_TRAVEL_HOLD_MS) / 1000.f;
			}
		}
	}
	else if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL == action.eKind)
	{
		SERVER_WORLD_TRANSFER_REQUEST transfer{};
		fired = Build_WorldTransfer(player, action, transfer);
		if (fired)
			outTransfers.push_back(std::move(transfer));
	}
	else if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == action.eKind &&
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_BOSS_START_TRIGGER_ID == trigger.Definition.strPlacementId &&
		activateTarget)
	{
		/* Start the boss, then send this player to the arena entrance. The start is
		   refused once the boss is up, which is fine: a later player is still sent. The
		   trigger counts as fired when the player was sent. A player who is busy (skill,
		   hit reaction) is not sent; the boss start is unaffected and stepping out and
		   back in sends them. Without an entrance box only the start runs. */
		fired = activateTarget(action.eKind, action.strTargetId);
		if (Place_PlayerAtValtanArenaEntry(player, actionStartTick))
		{
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			fired = true;
		}
	}
	else if ((WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE == action.eKind) &&
		activateTarget)
	{
		fired = activateTarget(action.eKind, action.strTargetId);
	}
	if (fired && trigger.Definition.isTriggerOnce)
		trigger.hasFired = true;
	return fired;
}

bool LostArk::Server::CServerTriggerSystem::Place_PlayerAtValtanArenaEntry(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick) const
{
	const auto entry = std::find_if(m_Triggers.begin(), m_Triggers.end(),
		[](const RUNTIME_TRIGGER& trigger)
		{
			return VALTAN_ARENA_ENTRY_TRIGGER_ID == trigger.Definition.strPlacementId;
		});
	if (m_Triggers.end() == entry)
		return false;
	const WORLD_BOOTSTRAP_PLACEMENT& box = entry->Definition;

	WORLD_TRIGGER_ACTION move{};
	move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
	move.fTargetX = box.fPositionX;
	move.fTargetY = box.fPositionY;
	move.fTargetZ = box.fPositionZ;
	/* Same hop the entrance box itself authors, so both moves feel alike. */
	move.fDurationSeconds = 0.8f;
	move.fArcHeight = 0.f;
	if (!box.TriggerActions.empty() &&
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == box.TriggerActions.front().eKind &&
		box.TriggerActions.front().fDurationSeconds > 0.f)
	{
		move.fDurationSeconds = box.TriggerActions.front().fDurationSeconds;
		move.fArcHeight = box.TriggerActions.front().fArcHeight;
	}
	/* The box height is authored by hand and can sit off the floor; land on the
	   floor the room's navigation reports under the box centre when it can. */
	float groundY = 0.f;
	if (m_GroundSampler &&
		m_GroundSampler(box.fPositionX, box.fPositionZ, groundY))
	{
		move.fTargetY = groundY;
	}
	return Begin_MovePlayer(player, move, actionStartTick);
}

void LostArk::Server::CServerTriggerSystem::Reset_SequenceActivation(const std::string& instanceId)
{
	for (auto& trigger : m_Triggers)
		if (std::any_of(trigger.Definition.TriggerActions.begin(), trigger.Definition.TriggerActions.end(),
			[&](const auto& action) { return action.eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE && action.strTargetId == instanceId; }))
			trigger.hasFired = false;
}

bool LostArk::Server::CServerTriggerSystem::Activate_Interact(
	const LostArk::Shared::PLAYER_ID playerId,
	const std::string& triggerPlacementId,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	const auto found = players.find(playerId);
	if (players.end() == found || 0u == found->second.iCurrentHp ||
		Is_KeyDebounced(playerId, actionStartTick))
	{
		return false;
	}
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		if (trigger.Definition.strPlacementId != triggerPlacementId ||
			trigger.Definition.TriggerActions.empty() ||
			Fires_OnEntry(trigger) ||
			Is_WaveMonsterSuppressed(trigger) ||
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind ||
			(trigger.Definition.isTriggerOnce && trigger.hasFired))
		{
			continue;
		}
		/* The offer was sent when they entered; they may have walked out since,
		   so membership is tested again rather than trusted. */
		if (!Contains(trigger, found->second))
			return false;
		const bool fired = Run_KeyTrigger(trigger, found->second, actionStartTick,
			outTransfers, activateTarget, moveEntry);
		if (fired)
		{
			Log_Fire(trigger, playerId, "KEY");
			m_LastKeyActivationTick[playerId] = actionStartTick;
		}
		return fired;
	}
	return false;
}

std::uint32_t LostArk::Server::CServerTriggerSystem::Activate_Here(
	const LostArk::Shared::PLAYER_ID playerId,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	const auto found = players.find(playerId);
	if (players.end() == found || 0u == found->second.iCurrentHp ||
		Is_KeyDebounced(playerId, actionStartTick))
	{
		return 0u;
	}
	std::uint32_t used = 0u;
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		/* The telescope box is a strike volume the room measures the hammer
		   against; G inside it runs nothing, exactly like walking in. A box that
		   fires on entry is not G's to run either. */
		if (trigger.Definition.TriggerActions.empty() ||
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind ||
			Fires_OnEntry(trigger) ||
			Is_WaveMonsterSuppressed(trigger) ||
			(trigger.Definition.isTriggerOnce && trigger.hasFired) ||
			!Contains(trigger, found->second))
		{
			continue;
		}
		if (Run_KeyTrigger(trigger, found->second, actionStartTick,
			outTransfers, activateTarget, moveEntry))
		{
			Log_Fire(trigger, playerId, "KEY");
			++used;
		}
	}
	if (0u != used)
		m_LastKeyActivationTick[playerId] = actionStartTick;
	return used;
}

void LostArk::Server::CServerTriggerSystem::Log_Fire(
	const RUNTIME_TRIGGER& trigger,
	const LostArk::Shared::PLAYER_ID playerId,
	const char* const pSource) const
{
	if (!m_FireLog)
		return;
	m_FireLog("[Trigger] Fire Trigger=" + trigger.Definition.strPlacementId +
		" Player=" + std::to_string(playerId) + " Source=" + pSource);
}

bool LostArk::Server::CServerTriggerSystem::Fires_OnEntry(
	const RUNTIME_TRIGGER& trigger) const
{
	if (trigger.Definition.requiresInteract ||
		trigger.Definition.TriggerActions.empty())
	{
		return false;
	}
#ifdef _DEBUG
	/* The Debug Valtan corridor shortcut hops the player toward the next stage, so
	   it waits for G like any other movement trigger. */
	WORLD_TRIGGER_ACTION shortcut{};
	if (m_bDebugValtanStageBypass &&
		Build_ValtanStageBypassMove(trigger.Definition.strPlacementId, shortcut))
	{
		return false;
	}
#endif
	const WORLD_TRIGGER_ACTION_KIND kind =
		trigger.Definition.TriggerActions.front().eKind;
	const std::string& placementId = trigger.Definition.strPlacementId;
	return std::any_of(std::begin(AUTO_ENTRY_RULES), std::end(AUTO_ENTRY_RULES),
		[this, kind, &placementId](const AUTO_ENTRY_RULE& rule)
		{
			return rule.eKind == kind &&
				(LostArk::Shared::WORLD_ID::END == rule.eWorld ||
					m_eWorldId == rule.eWorld) &&
				(nullptr == rule.pIdPrefix ||
					placementId.starts_with(rule.pIdPrefix));
		});
}

bool LostArk::Server::CServerTriggerSystem::Run_Trigger(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget) const
{
#ifdef _DEBUG
	/* Debug Valtan shortcut: the trigger's own action is replaced by a short hop
	   toward the next stage. Stage_Boss is exempt: it keeps its real activateEncounter
	   action, and Run_Action then sends the player to the arena entrance box exactly as
	   Release does. Both run only on G. */
	WORLD_TRIGGER_ACTION bypassMove{};
	if (m_bDebugValtanStageBypass &&
		"Stage_Boss" != trigger.Definition.strPlacementId &&
		Build_ValtanStageBypassMove(
			trigger.Definition.strPlacementId, bypassMove))
	{
		const bool moved = Begin_MovePlayer(player, bypassMove, actionStartTick);
		if (moved && trigger.Definition.isTriggerOnce)
			trigger.hasFired = true;
		return moved;
	}
#endif
	/* Stage_Boss's entrance placement lives in Run_Action, so Debug and Release move
	   the player the same way. Place_PlayerAtValtanAuditionBait is now only used by
	   the Debug pattern audition. */
	return Run_Action(
		trigger, player, actionStartTick, outTransfers, activateTarget);
}

bool LostArk::Server::CServerTriggerSystem::Run_KeyTrigger(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry) const
{
	/* A movement box the room owns (a Kouku Mario lane) is admitted by the same room
	   handler whether the player stepped in or pressed G: the stage match, authority
	   locks, contact-action interruption and the terminal exit's return destination
	   all live there, so a G press can neither bypass them nor land the player on the
	   raw authored point. Anything else runs the authored action unchanged. */
	if (moveEntry && !trigger.Definition.TriggerActions.empty() &&
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER ==
			trigger.Definition.TriggerActions.front().eKind)
	{
		const SERVER_TRIGGER_MOVE_ENTRY_RESULT owned =
			moveEntry(trigger.Definition, player, actionStartTick);
		if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE == owned)
			return false;
		if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED == owned)
		{
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			if (trigger.Definition.isTriggerOnce)
				trigger.hasFired = true;
			return true;
		}
	}
	return Run_Trigger(trigger, player, actionStartTick, outTransfers, activateTarget);
}

bool LostArk::Server::CServerTriggerSystem::Is_KeyDebounced(
	const LostArk::Shared::PLAYER_ID playerId,
	const std::uint32_t tick) const
{
	const auto last = m_LastKeyActivationTick.find(playerId);
	return m_LastKeyActivationTick.end() != last &&
		tick - last->second < KEY_ACTIVATION_DEBOUNCE_TICKS;
}

LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT LostArk::Server::CServerTriggerSystem::Debug_Activate(
	const LostArk::Shared::PLAYER_ID playerId, const std::string& triggerId, const bool replay,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players, const std::uint32_t tick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& transfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND, const std::string&)>& activateTarget)
{
	using Result = LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT;
#ifndef _DEBUG
	(void)playerId; (void)triggerId; (void)replay; (void)players; (void)tick; (void)transfers; (void)activateTarget;
	return Result::DISABLED;
#else
	const auto player = players.find(playerId);
	if (player == players.end() || !player->second.iCurrentHp || player->second.TriggerMove.isActive)
		return Result::INVALID_PLAYER;
	const auto trigger = std::find_if(m_Triggers.begin(), m_Triggers.end(),
		[&](const RUNTIME_TRIGGER& value) { return value.Definition.strPlacementId == triggerId; });
	if (trigger == m_Triggers.end()) return Result::INVALID_TARGET;
	if (!replay && trigger->Definition.isTriggerOnce && trigger->hasFired) return Result::ALREADY_USED;
	// Debug bypasses only entry/G-key/one-shot admission. The authored action is shared.
	return Run_Action(*trigger, player->second, tick, transfers, activateTarget) ?
		Result::ACCEPTED : Result::ACTION_REJECTED;
#endif
}

void LostArk::Server::CServerTriggerSystem::Evaluate_Entries(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	std::vector<SERVER_INTERACT_PROMPT_EDGE>& outPromptEdges,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	outTransfers.clear();
	outPromptEdges.clear();
	/* Bern travel: a player who has just landed inside another travel box does not fire it.
	   A box only fires on a fresh step in, so the castle and library pairs cannot bounce the
	   player back and forth; they have to walk out and in again. */
	std::unordered_set<LostArk::Shared::PLAYER_ID> landed;
	if (LostArk::Shared::WORLD_ID::BERN == m_eWorldId)
	{
		for (const auto& [playerId, player] : players)
		{
			if (player.Is_Guide()) continue;
				if (LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction ||
					LostArk::Shared::PLAYER_ACTION_STATE::WALL_CLIMB == player.eAction)
				m_TriggerMoveInFlight.insert(playerId);
			else if (0u != m_TriggerMoveInFlight.erase(playerId))
				landed.insert(playerId);
		}
	}
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		/* The telescope box is a strike volume the room measures the hammer
		   against; walking into it runs nothing and offers nothing. */
		if (!trigger.Definition.TriggerActions.empty() &&
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind)
		{
			continue;
		}
		/* Debug rooms hand the four wave-monster boxes to the F1 buttons: walking
		   in neither raises the wave nor offers G. Release never sets the flag. */
		if (Is_WaveMonsterSuppressed(trigger))
			continue;
		/* A box that does not fire on entry only offers itself: the player presses
		   G inside it and the Server checks the volume again. */
		const bool firesOnEntry = Fires_OnEntry(trigger);
		std::unordered_set<LostArk::Shared::PLAYER_ID> currentInside;
		for (auto& [playerId, player] : players)
		{
			if (player.Is_Guide()) continue;
			if (0u == player.iCurrentHp || !Contains(trigger, player))
				continue;
			// Keep membership unset during the jump: landing creates the entry edge.
			if (m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && player.TriggerMove.isActive &&
				!trigger.Definition.TriggerActions.empty() &&
				trigger.Definition.TriggerActions.front().eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE &&
				trigger.Definition.TriggerActions.front().strTargetId == LostArk::Shared::MAHARAKA_WATERPANG_INTRO_INSTANCE)
				continue;
			currentInside.insert(playerId);
			const bool wasInside = trigger.PlayersInside.contains(playerId);
			if (!firesOnEntry)
			{
				/* It offers, once, on the edge. A spent one-shot (a test that kept
				   the latch, see Set_HonourTriggerOnce) has nothing left to offer. */
				if (!wasInside && !(trigger.Definition.isTriggerOnce &&
					trigger.hasFired))
				{
					outPromptEdges.push_back({ playerId,
						trigger.Definition.strPlacementId, true });
				}
				continue;
			}
			if (wasInside || landed.contains(playerId) ||
				(trigger.Definition.isTriggerOnce && trigger.hasFired))
			{
				continue;
			}
			bool fired = false;
			const auto ownedEntry = moveEntry &&
				WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == trigger.Definition.TriggerActions.front().eKind ?
				moveEntry(trigger.Definition, player, actionStartTick) :
				SERVER_TRIGGER_MOVE_ENTRY_RESULT::USE_DEFAULT;
			if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE == ownedEntry)
			{
				/* Contact was detected but not admitted. Do not turn a
				   temporary authority lock into a consumed entry edge. */
				currentInside.erase(playerId);
				continue;
			}
			if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED == ownedEntry)
			{
				fired = true;
				player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			}
			else
			{
				fired = Run_Action(trigger, player, actionStartTick,
					outTransfers, activateTarget);
			}
			if (fired)
			{
				Log_Fire(trigger, playerId, "ENTER");
				/* A Bern move can finish before the next evaluation sees it in flight, so the
				   landing is recorded the moment it starts. */
				if (LostArk::Shared::WORLD_ID::BERN == m_eWorldId &&
					(LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction ||
					 LostArk::Shared::PLAYER_ACTION_STATE::WALL_CLIMB == player.eAction))
				{
					m_TriggerMoveInFlight.insert(playerId);
				}
				/* An owned entry fires without going through Run_Action, so the
				   one-shot latch is still set here for that path. It is a no-op
				   unless Set_HonourTriggerOnce kept isTriggerOnce. */
				if (trigger.Definition.isTriggerOnce)
				{
					trigger.hasFired = true;
				}
			}
            else if (LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
                WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == trigger.Definition.TriggerActions.front().eKind)
            {
                // Raid admission may wait for another player's G-key transfer. A
                // rejected sequence must not consume this player's only entry edge.
                currentInside.erase(playerId);
            }
			else if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::WALL_CLIMB != player.eAction &&
				(WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER ==
					trigger.Definition.TriggerActions.front().eKind ||
				WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL ==
					trigger.Definition.TriggerActions.front().eKind))
			{
				/* Only a box that fires on entry gets here. Contact was made while
				   the player was busy (skill, hit reaction), so the entry edge is
				   not spent: the next tick tries again until the action ends or they
				   leave. A trigger move in flight is excluded so a box crossed
				   mid-arc never chains into another teleport. */
				currentInside.erase(playerId);
			}
		}
		/* Leaving withdraws the offer, so a Client never keeps a prompt for a
		   box it has walked out of. */
		if (!firesOnEntry)
		{
			for (const LostArk::Shared::PLAYER_ID previous : trigger.PlayersInside)
			{
				if (!currentInside.contains(previous))
				{
					outPromptEdges.push_back({ previous,
						trigger.Definition.strPlacementId, false });
				}
			}
		}
		trigger.PlayersInside = std::move(currentInside);
	}
}

bool LostArk::Server::CServerTriggerSystem::Place_PlayerAtValtanAuditionBait(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick) const
{
	WORLD_TRIGGER_ACTION move{};
	if (!Build_ValtanStageBypassMove("Stage_Boss", move) ||
		!Begin_MovePlayer(player, move, actionStartTick))
	{
		return false;
	}
	return Update_PlayerMotion(player, move.fDurationSeconds) &&
		LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction;
}

bool LostArk::Server::CServerTriggerSystem::Build_ValtanStageBypassMove(
	const std::string& triggerPlacementId,
	WORLD_TRIGGER_ACTION& outAction)
{
	/* These destinations stop just before the next authored trigger. The player
	still walks into every next stage deliberately, while the long blocked route
	and its unkillable audition monsters no longer prevent reaching Valtan. */
	struct BYPASS_DESTINATION final
	{
		const char* pTriggerPlacementId;
		float x;
		float y;
		float z;
		float duration;
		float arcHeight;
	};
	/* Stage_1, Stage_2 and Stage_MiniBoss are deliberately absent. Stage_1 and
	Stage_2 build their spawn-group waves and Stage_MiniBoss now authors the
	Lugaru entrance move that places the party under the entrance camera shot,
	so all three have to run their real actions. Stage_2 used to hop 7 m toward
	Stage_3 instead of spawning spawn.valtan.stage03. Stage_3 is absent as well:
	it authors the cliff move in Gameplay.world.json, so that authored action is
	the only truth. The remaining rows keep the shortcut, which is still how boss
	work reaches Valtan without clearing the whole corridor. */
	static constexpr BYPASS_DESTINATION DESTINATIONS[] =
	{
		{ "Stage_Boss", 154.296f, 22.970f, -125.219f, 0.01f, 0.f }
	};
	const auto found = std::find_if(
		std::begin(DESTINATIONS), std::end(DESTINATIONS),
		[&triggerPlacementId](const BYPASS_DESTINATION& destination)
		{
			return triggerPlacementId == destination.pTriggerPlacementId;
		});
	if (std::end(DESTINATIONS) == found)
		return false;

	WORLD_TRIGGER_ACTION staged{};
	staged.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
	staged.fTargetX = found->x;
	staged.fTargetY = found->y;
	staged.fTargetZ = found->z;
	staged.fDurationSeconds = found->duration;
	staged.fArcHeight = found->arcHeight;
	outAction = staged;
	return true;
}

void LostArk::Server::CServerTriggerSystem::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		trigger.PlayersInside.erase(playerId);
	}
	m_LastKeyActivationTick.erase(playerId);
	m_TriggerMoveInFlight.erase(playerId);
}

bool LostArk::Server::CServerTriggerSystem::Contains(
	const RUNTIME_TRIGGER& trigger,
	const SERVER_PLAYER& player)
{
	return Contains_Placement(trigger.Definition, player);
}

bool LostArk::Server::CServerTriggerSystem::Contains_Placement(
	const WORLD_BOOTSTRAP_PLACEMENT& box,
	const SERVER_PLAYER& player)
{
	const float deltaX = player.fPositionX - box.fPositionX;
	const float deltaZ = player.fPositionZ - box.fPositionZ;
	const float yaw = box.fYawDegrees * DEGREES_TO_RADIANS;
	const float cosine = std::cos(yaw);
	const float sine = std::sin(yaw);
	const float localX = cosine * deltaX - sine * deltaZ;
	const float localZ = sine * deltaX + cosine * deltaZ;
	const float playerCenterY = player.fPositionY +
		LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
	return std::abs(localX) <= box.fHalfExtentX +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X &&
		std::abs(playerCenterY - box.fPositionY) <= box.fHalfExtentY +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y &&
		std::abs(localZ) <= box.fHalfExtentZ +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Z;
}

bool LostArk::Server::CServerTriggerSystem::Begin_MovePlayer(
	SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	const std::uint32_t actionStartTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp || 0u == actionStartTick ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != action.eKind)
	{
		return false;
	}

	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.TriggerMove = {};
	const bool wallClimb = WORLD_TRIGGER_MOVE_STYLE::WALL_CLIMB == action.eMoveStyle;
	if (wallClimb && action.TrackSamples.size() < 2u)
		return false;
	player.TriggerMove.fStartX = wallClimb ? action.TrackSamples.front().fPositionX : player.fPositionX;
	player.TriggerMove.fStartY = wallClimb ? action.TrackSamples.front().fPositionY : player.fPositionY;
	player.TriggerMove.fStartZ = wallClimb ? action.TrackSamples.front().fPositionZ : player.fPositionZ;
	player.TriggerMove.fTargetX = wallClimb ? action.TrackSamples.back().fPositionX : action.fTargetX;
	player.TriggerMove.fTargetY = wallClimb ? action.TrackSamples.back().fPositionY : action.fTargetY;
	player.TriggerMove.fTargetZ = wallClimb ? action.TrackSamples.back().fPositionZ : action.fTargetZ;
	player.TriggerMove.fDurationSeconds = wallClimb ?
		static_cast<float>(action.TrackSamples.back().iTimeMs) / 1000.f : action.fDurationSeconds;
	player.TriggerMove.fElapsedSeconds = 0.f;
	player.TriggerMove.fArcHeight = wallClimb ? 0.f : action.fArcHeight;
	if (wallClimb)
	{
		player.TriggerMove.TrackSamples.reserve(action.TrackSamples.size());
		for (const WORLD_TRIGGER_MOVE_SAMPLE& sample : action.TrackSamples)
			player.TriggerMove.TrackSamples.push_back({ sample.iTimeMs,
				sample.fPositionX, sample.fPositionY, sample.fPositionZ });
		player.fPositionX = player.TriggerMove.fStartX;
		player.fPositionY = player.TriggerMove.fStartY;
		player.fPositionZ = player.TriggerMove.fStartZ;
		player.fYawDegrees = action.fFacingYawDegrees;
	}
	player.TriggerMove.eKoukuHudModeOnArrival = action.eKoukuHudModeOnArrival;
	player.TriggerMove.isActive = true;
	const float deltaX = action.fTargetX - player.fPositionX;
	const float deltaZ = action.fTargetZ - player.fPositionZ;
	if (!wallClimb && deltaX * deltaX + deltaZ * deltaZ > 0.000001f)
		player.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	player.eAction = wallClimb ? PLAYER_ACTION_STATE::WALL_CLIMB :
		PLAYER_ACTION_STATE::TRIGGER_MOVE;
	player.iActionStartTick = actionStartTick;
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Build_WorldTransfer(
	const SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	using namespace LostArk::Shared;
	if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL != action.eKind ||
		(WORLD_ID::BERN != action.eTargetWorldId &&
			WORLD_ID::VALTAN_ARENA != action.eTargetWorldId) ||
		INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty() || 0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		return false;
	}

	outTransfer.iSessionId = player.iSessionId;
	outTransfer.eTargetWorldId = action.eTargetWorldId;
	outTransfer.eCharacterClass = player.eCharacterClass;
	outTransfer.strNickName = player.strNickName;
	return true;
}
```

## Server/Private/ServerGameplayContractTests_WorldPlayback.cpp

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerTriggerSystem.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

int LostArk::Server::CServerGameplayContractRunner::Run_WorldPlayback(TESTS& tests)
{
        {
            auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
            tests.Require(room->Is_Ready(), "Waterpang published room loads");
            room->m_iServerTick=100u;
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                0.f,0.f,0.f,0.f,0u,{}) && !room->m_MaharakaWaterpangIntro,
                "Invalid Waterpang packet cannot consume room reservation");
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro &&
                room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "Waterpang reserves exactly ten seconds on the Server clock");
            room->m_iServerTick=250u;
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "A second arena entry cannot reset countdown");
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{},WORLD_SEQUENCE_OPERATION::REPLAY),
                "Replay cannot restart an active Waterpang reservation");
            auto session=std::make_shared<CClientSession>(99001u,INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});
            C2S_ENTER_WORLD enter{}; enter.iProtocolVersion=NETWORK_PROTOCOL_VERSION;
            enter.eWorldId=WORLD_ID::MAHARAKA; enter.eCharacterClass=CHARACTER_CLASS_ID::ARTIST;
            enter.strNickName="WaterpangLateJoin";
            CGameRoom::STAGED_PLAYER_ENTRY admission{}; SESSION_DIAGNOSTIC_REASON reason{}; std::string admissionStatus;
            const bool admitted=room->Stage_PlayerEntry(session,enter,{},admission,reason,admissionStatus) &&
                room->Build_PlayerEntryFrames(admission,std::span<const CGameRoom::STAGED_PLAYER_ENTRY>{&admission,1u},admissionStatus);
            if (!admitted) std::cout << "Waterpang admission diagnostic: " << admissionStatus << '\n';
            bool sameReservation=false;
            for (const auto& frame:admission.Frames)
                if (frame.ePacketType==PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
                {
                    CPacketReader reader(frame.Payload); S2C_WORLD_SEQUENCE_PLAY play;
                    if (Read_Message(reader,play) && play.strSequenceInstanceId==MAHARAKA_WATERPANG_INTRO_INSTANCE)
                        sameReservation=play.iStartTick==400u && play.iServerTick==250u;
                }
            tests.Require(admitted && sameReservation,"Late Maharaka admission receives original start and current Server tick");
            tests.Require(room->Reset_ReplayableArenaWhenEmpty() && !room->m_MaharakaWaterpangIntro,
                "An empty Maharaka room releases Waterpang reservation");

            CServerNavigation navigation; SERVER_NAV_POINT point;
            tests.Require(navigation.Load("LV_OCN_EVENTIS_MHP") &&
                navigation.Sample_Position(73.041f,-979.223022f,point) && point.y>22.3f && point.y<22.5f &&
                navigation.Resolve_TraversalStep(73.041f,-979.223022f,73.1f,-979.4f,point,23.1289997f) && point.y>22.3f,
                "Waterpang landing and next walking step remain on mesh-baked stage floor");

            CServerTriggerSystem entry; entry.Set_WorldId(WORLD_ID::MAHARAKA);
            WORLD_BOOTSTRAP_PLACEMENT start{}; start.strPlacementId="waterpang.arena.start";
            start.eKind=WORLD_BOOTSTRAP_KIND::TRIGGER_BOX; start.isEnabled=true;
            start.fHalfExtentX=start.fHalfExtentY=start.fHalfExtentZ=1.f;
            WORLD_TRIGGER_ACTION action{}; action.eKind=WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
            action.strTargetId=MAHARAKA_WATERPANG_INTRO_INSTANCE; start.TriggerActions.push_back(action);
            std::string status; tests.Require(entry.Initialize({start},status),"Waterpang landing trigger initializes");
            std::map<PLAYER_ID,SERVER_PLAYER> players;
            auto& player=players[1u]; player.iPlayerId=1u; player.iCurrentHp=player.iMaximumHp=100u;
            player.TriggerMove.isActive=true;
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers; std::vector<SERVER_INTERACT_PROMPT_EDGE> edges;
            int fired=0; const auto activate=[&](WORLD_TRIGGER_ACTION_KIND,const std::string&){++fired;return true;};
            entry.Evaluate_Entries(players,1u,transfers,activate,edges);
            tests.Require(fired==0,"Flying through arena does not start countdown");
            player.TriggerMove.isActive=false;
            entry.Evaluate_Entries(players,2u,transfers,activate,edges);
            tests.Require(fired==1,"Landing inside arena starts countdown without an extra re-entry");
            entry.Evaluate_Entries(players,3u,transfers,activate,edges);
            tests.Require(fired==1,"Standing on arena does not repeatedly start countdown");
            CWorldBootstrap authored;
            tests.Require(authored.Load(WORLD_ID::MAHARAKA),"Published Waterpang G jumps load");
            for (const char* name:{"jump1","jump2","jump3"})
            {
                const auto box=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [&](const auto& row){return row.strPlacementId==name;});
                if (box==authored.Get_Placements().end()) { tests.Require(false,"Waterpang jump source missing"); continue; }
                CServerTriggerSystem jump; jump.Set_WorldId(WORLD_ID::MAHARAKA);
                tests.Require(jump.Initialize({*box},status),"Waterpang authored jump initializes");
                player.fPositionX=box->fPositionX; player.fPositionY=box->fPositionY; player.fPositionZ=box->fPositionZ;
                jump.Evaluate_Entries(players,10u,transfers,activate,edges);
                tests.Require(!player.TriggerMove.isActive && !edges.empty(),"Waterpang jump offers G without automatic movement");
                const auto activated=jump.Activate_Here(1u,players,11u,transfers,activate);
                const auto target=player.TriggerMove;
                for (unsigned tick=0;tick<40;++tick) jump.Update_PlayerMotion(player,1.f/30.f);
                tests.Require(activated==1u && !player.TriggerMove.isActive &&
                    std::abs(player.fPositionX-target.fTargetX)<.001f &&
                    std::abs(player.fPositionY-target.fTargetY)<.001f &&
                    std::abs(player.fPositionZ-target.fTargetZ)<.001f,
                    "Waterpang G travels to the exact authored destination");
            }
        }
		CWorldBootstrap bootstrap;
		tests.Require(bootstrap.Load(WORLD_ID::KAKULSAYDON_ARENA) && !bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer loads published Kouku sequence IDs with the world");
		tests.Require(bootstrap.Load(WORLD_ID::VALTAN_ARENA) && bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer switching to Valtan clears the previous world's sequence IDs");
		CServerTriggerSystem triggers;
		triggers.Set_HonourTriggerOnce(true);
		WORLD_BOOTSTRAP_PLACEMENT box{};
		box.strPlacementId = "viewer.trigger"; box.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		box.fHalfExtentX = box.fHalfExtentY = box.fHalfExtentZ = 1.f;
		box.isTriggerOnce = true; box.requiresInteract = true;
		WORLD_TRIGGER_ACTION action{}; action.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
		action.strTargetId = "viewer.sequence"; box.TriggerActions.push_back(action);
		std::string status;
		tests.Require(triggers.Initialize({ box }, status), "Viewer test initializes the real trigger system");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players[1u].iPlayerId = 1u; players[1u].iCurrentHp = players[1u].iMaximumHp = 100u;
		players[1u].fPositionX = 50.f;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		int fired = 0;
		const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{ if (kind != WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE || id != "viewer.sequence") return false; ++fired; return true; };
		using R = DEBUG_WORLD_PLAYBACK_RESULT;
		tests.Require(triggers.Debug_Activate(2u, box.strPlacementId, false, players, 1u, transfers, activate) ==
#ifdef _DEBUG
			R::INVALID_PLAYER,
#else
			R::DISABLED,
#endif
			"Viewer rejects missing player without activating a trigger");
#ifdef _DEBUG
		tests.Require(triggers.Debug_Activate(1u, "missing", false, players, 1u, transfers, activate) == R::INVALID_TARGET && fired == 0,
			"Viewer rejects unknown targets without effects");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 1u, transfers, activate) == R::ACCEPTED && fired == 1,
			"Debug viewer uses the authored action outside the G-key box");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 2u, transfers, activate) == R::ALREADY_USED && fired == 1,
			"Play preserves the one-shot latch");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 3u, transfers, activate) == R::ACCEPTED && fired == 2,
			"Replay reuses the same authored action");
		players[1u].iCurrentHp = 0;
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 4u, transfers, activate) == R::INVALID_PLAYER && fired == 2,
			"Dead viewer cannot activate world actions");
		players[1u].iCurrentHp = 100;
		const auto reject = [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return false; };
		tests.Require(triggers.Initialize({ box }, status) &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 5u, transfers, reject) == R::ACTION_REJECTED &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 6u, transfers, activate) == R::ACCEPTED,
			"Failed action does not consume the one-shot trigger");
#endif
		{
			// Exercise the real broadcast boundary: stale bootstrap rows cannot revive
			// the old actor or move the party before the Client rejects the cue.
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			auto& player = room->m_Players[1u];
			player.iPlayerId = 1u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			bool allPreserve = room->Is_Ready();
			bool admissionMatches = room->Is_Ready();
			const WORLD_SEQUENCE_OPERATION operations[] = { WORLD_SEQUENCE_OPERATION::PLAY,
				WORLD_SEQUENCE_OPERATION::REPLAY, WORLD_SEQUENCE_OPERATION::STOP,
				WORLD_SEQUENCE_OPERATION::PLAY, WORLD_SEQUENCE_OPERATION::PLAY };
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				player.fPositionX = 12.f; player.fPositionY = 34.f; player.fPositionZ = 56.f;
				player.hasMoveGoal = true; player.TriggerMove.isActive = true;
				player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
				player.iMarioStage = 1u; player.ePreMarioForm = PLAYER_MADNESS_FORM::NORMAL;
				player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
				const bool accepted = room->Broadcast_WorldSequencePlay(
					scenario == 4u ? "world.sequence.instance.circusfinale" : "world.sequence.instance.original_kouku",
					1.f, 0.f, 0.f, 0.f, 0u, scenario == 3u ? "world.existing.target" : "", operations[scenario]);
				admissionMatches = admissionMatches && accepted == (scenario == 2u || scenario == 4u);
				allPreserve = allPreserve &&
					player.fPositionX == 12.f && player.fPositionY == 34.f && player.fPositionZ == 56.f &&
					player.hasMoveGoal && player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
					player.iMarioStage == 1u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && player.iCurrentHp == 100u;
			}
			tests.Require(admissionMatches, "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted");
			tests.Require(allPreserve, "Legacy rejection and ordinary sequence cues preserve all player movement and form state");
		}
		{
			C2S_DEBUG_WORLD_PLAYBACK request{};
			request.iRequestSequence = 17u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
			request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
			request.strOccurrenceId = request.strTargetId + ".logic.51";
			request.iRunEpoch = 9u; request.iRoomPlayerSlot = 3u;
			request.fPositionX = -1.156042f; request.fPositionY = 1.3176255f; request.fPositionZ = 742.512031f;
			CPacketWriter writer;
			tests.Require(Write_Message(writer, request), "Arrival command encodes its run, occurrence, slot and destination");
			CPacketReader reader(writer.Get_Buffer()); C2S_DEBUG_WORLD_PLAYBACK decoded;
			tests.Require(Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 9u && decoded.iRoomPlayerSlot == 3u && decoded.strOccurrenceId == request.strOccurrenceId &&
				decoded.fPositionX == request.fPositionX && decoded.fPositionY == request.fPositionY && decoded.fPositionZ == request.fPositionZ,
				"Arrival packet preserves exact slot coordinates and replay identity");
			auto bytes = writer.Get_Buffer(); bytes.pop_back();
			CPacketReader truncated(bytes); decoded.strOccurrenceId = "sentinel";
			tests.Require(!Read_Message(truncated, decoded) && decoded.strOccurrenceId == "sentinel",
				"Truncated arrival packet does not partially commit decoded intent");
			bool rejects = true;
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				auto bad = request;
				if (scenario == 0u) bad.iRoomPlayerSlot = 4u;
				if (scenario == 1u) bad.iRunEpoch = 0u;
				if (scenario == 2u) bad.fPositionX = std::numeric_limits<float>::quiet_NaN();
				if (scenario == 3u) bad.eWorldId = WORLD_ID::BERN;
				if (scenario == 4u) bad.fPositionZ = 100001.f;
				CPacketWriter invalid; rejects = rejects && !Write_Message(invalid, bad) && invalid.Get_Buffer().empty();
			}
			tests.Require(rejects, "Arrival rejects wrong world, invalid epoch, slot and coordinates before writing");
			S2C_DEBUG_WORLD_PLAYBACK_RESULT receipt{};
			receipt.iRequestSequence = request.iRequestSequence; receipt.eWorldId = request.eWorldId;
			receipt.eOperation = request.eOperation; receipt.strTargetId = request.strTargetId;
			receipt.eResult = R::SKIPPED_PLAYER;
			CPacketWriter replyWriter; const bool wroteReply = Write_Message(replyWriter, receipt);
			CPacketReader replyReader(replyWriter.Get_Buffer()); S2C_DEBUG_WORLD_PLAYBACK_RESULT reply;
			tests.Require(wroteReply && Read_Message(replyReader, reply) && replyReader.Get_RemainingSize() == 0u &&
				reply.iRequestSequence == 17u && reply.eOperation == request.eOperation && reply.eResult == R::SKIPPED_PLAYER,
				"Arrival skip reply uses the existing request-correlated result envelope");
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLAY_SEQUENCE;
			CPacketWriter legacyWriter; const bool wroteLegacy = Write_Message(legacyWriter, request);
			CPacketReader legacyReader(legacyWriter.Get_Buffer());
			tests.Require(wroteLegacy && Read_Message(legacyReader, decoded) && legacyReader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 0u && decoded.strOccurrenceId.empty(), "Ordinary world playback keeps its original payload shape");
		}
#ifdef _DEBUG
		{
			WSADATA winsock{};
			const bool socketReady = WSAStartup(MAKEWORD(2, 2), &winsock) == 0;
			tests.Require(socketReady, "Arrival fixture prepares unconnected session sockets without a listener");
			if (socketReady)
			{
				const std::array<std::array<float, 2>, 4> locations{{ {-3.913588f,739.883125f},
					{-3.290587f,742.070391f}, {-5.324063f,738.528281f}, {-1.156042f,742.512031f} }};
				for (unsigned count = 1u; count <= 4u; ++count)
				{
					auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
					std::vector<std::shared_ptr<CClientSession>> sessions;
					const auto join = [&](PLAYER_ID id)
					{
						const SESSION_ID sessionId = id + 1000u;
						auto connection = std::make_shared<CClientSession>(sessionId, ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
							CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
						sessions.push_back(connection); room->m_Sessions[sessionId] = connection;
						room->m_PlayerIdBySessionId[sessionId] = id;
						auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = sessionId;
						player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
						player.fPositionX = -100.f - static_cast<float>(id); player.fPositionY = 1.3f; player.fPositionZ = 740.f;
						player.hasMoveGoal = true; player.iCurrentSkillId = 34010u; player.eAction = PLAYER_ACTION_STATE::SKILL;
					};
					// Reverse insertion proves that stable PlayerId order, not joins or session order, chooses slots.
					for (unsigned n = count; n > 0u; --n) join(n * 10u);
					if (count == 4u) join(50u);
					bool nativeGround = room->Is_Ready();
					std::array<SERVER_NAV_POINT, 4> ground{};
					for (unsigned slot = 0u; slot < 4u; ++slot)
						nativeGround = nativeGround && room->m_ServerNavigation.Sample_Position(locations[slot][0], locations[slot][1], ground[slot]);
					tests.Require(nativeGround, "Arrival samples all four authored fireworks XZ on actual Server navigation");
					if (!nativeGround) continue;
					C2S_DEBUG_WORLD_PLAYBACK request{};
					request.iRequestSequence = 1u; request.iRunEpoch = 1u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
					request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
					const auto position = [&](unsigned slot)
					{
						request.iRoomPlayerSlot = static_cast<std::uint8_t>(slot);
						request.strOccurrenceId = request.strTargetId + ".logic." + std::to_string(slot + 1u);
						request.fPositionX = ground[slot].x; request.fPositionY = ground[slot].y; request.fPositionZ = ground[slot].z;
					};
					bool movedInOrder = true;
					for (unsigned slot = 0u; slot < 4u; ++slot)
					{
						position(slot); const auto verdict = room->Apply_DebugRoomPlayerArrival(1010u, request);
						movedInOrder = movedInOrder && verdict == (slot < count ? R::ACCEPTED : R::SKIPPED_PLAYER);
						if (slot < count)
						{
							const auto& player = room->m_Players.at((slot + 1u) * 10u);
							movedInOrder = movedInOrder && player.fPositionX == ground[slot].x && player.fPositionY == ground[slot].y &&
								player.fPositionZ == ground[slot].z && !player.hasMoveGoal && player.iCurrentSkillId == INVALID_SKILL_ID && player.iCurrentHp == 100u;
						}
					}
					tests.Require(movedInOrder, "One through four connected players arrive by PlayerId; missing slots are successful skips");
					if (count == 4u)
						tests.Require(room->m_RoomPlayerArrivalRuns.at(1010u).Players.size() == 4u && room->m_Players.at(50u).hasMoveGoal,
							"Arrival roster caps at four and preserves any later connected player");
					position(0u); auto& first = room->m_Players.at(10u); first.fPositionX -= 20.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ALREADY_USED && first.fPositionX == ground[0].x - 20.f,
						"Duplicate arrival occurrence never teleports an already consumed slot twice");
					request.iRunEpoch = 2u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACCEPTED && first.fPositionX == ground[0].x,
						"Explicit new playback epoch permits the same occurrence again");
					first.fPositionX -= 20.f; first.hasMoveGoal = true; first.iCurrentSkillId = 34010u;
					request.iRunEpoch = 1u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::STALE_REQUEST && first.hasMoveGoal && first.iCurrentSkillId == 34010u,
						"An older playback cannot mutate the current run");
					request.iRunEpoch = 2u; request.strOccurrenceId += ".wrongheight"; request.fPositionY += 100.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACTION_REJECTED && first.fPositionX == ground[0].x - 20.f &&
						first.hasMoveGoal && first.iCurrentSkillId == 34010u && first.iCurrentHp == 100u,
						"Rejected destination preserves position, action, movement and health");
					request.eWorldId = WORLD_ID::BERN;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::WRONG_WORLD && first.hasMoveGoal,
						"Arrival cannot cross the requesting room's world boundary");
					request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					if (count > 1u)
					{
						// Epoch 2 already captured PlayerId 20; replacing its room binding cannot retarget that slot.
						room->m_PlayerIdBySessionId.erase(1020u); room->m_Players.erase(20u); join(21u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(21u).hasMoveGoal,
							"Departed roster member is skipped without teleporting its newly joined replacement");
					}
					else
					{
						join(20u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(20u).hasMoveGoal,
							"Joining midway does not fill a slot absent from the playback's fixed roster");
					}
				}
				WSACleanup();
			}
		}
#endif
		std::cout << "World playback contract failures: " << tests.failures << '\n';
		return tests.failures == 0 ? 0 : 1;
	}
```

## Tools/MapPipeline/configure_maharaka_waterpang_entry.py

```python
"""Configure saved G jumps, room intro trigger and mesh-baked arena support.

Does not move user markers, NPCs, permanent map placements or camera keys.
CAS install with a before/candidate backup; publishers own runtime outputs.
The focused navigation region preserves the existing flat base outside the
exact 18 stage tiles, centre disc and two top barrel meshes.
"""
import argparse
import copy
import json
import math
from pathlib import Path
import sys
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'Tools/WorldPipeline'))
sys.path.insert(0, str(ROOT/'Tools/ModelAssetConverter'))
sys.path.insert(0, str(ROOT/'Tools/EffectPipeline'))
from test_maharaka_npc_population import rows, mesh_triangles, transform
from source_character_registration import commit_staged_files

AREA = 'LV_OCN_EVENTIS_MHP'
STAGE = 'world.sequence.instance.maharaka.waterpang.source.intro15.stage'
OUT = ROOT/'out/MaharakaWaterpangEntry20260928'


def encode(value):
    return (json.dumps(value, ensure_ascii=False, indent=2)+'\n').encode('utf-8')


def prepare():
    world_path = ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json'
    sequence_path = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json'
    placement_path = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'
    catalog_path = ROOT/f'Data/Maps/Imported/{AREA}/{AREA}.mapassets'
    paths = [world_path, sequence_path, placement_path, catalog_path]
    expected = {p:p.read_bytes() for p in paths}
    world = json.loads(expected[world_path].decode('utf-8-sig'))
    sequence = json.loads(expected[sequence_path].decode('utf-8-sig'))
    before_world = copy.deepcopy(world)
    by_id = {p['placementId']:p for p in world['placements']}
    assert len(by_id) == len(world['placements'])
    for n in range(1,4):
        start, end = by_id[f'jump{n}'], by_id[f'jump{n}_1']
        assert start['kind'] == end['kind'] == 'triggerBox'
        event = {'type':'movePlayer', 'targetPosition':end['position'],
                 'durationSeconds':1.2, 'arcHeight':2.5}
        assert not start['events'] or start['events'] == [event], 'Existing jump changed; review before replacing'
        start.update(enabled=True, triggerOnce=False, requiresInteract=True,
                     interactAction='climb' if n == 3 else 'tightrope', events=[event])
    start_trigger = {'placementId':'waterpang.arena.start', 'kind':'triggerBox',
                     'position':[75.05,22.65,-984.32], 'yawDegrees':0,
                     'enabled':True, 'halfExtents':[5.5,0.7,5.5],
                     'triggerOnce':False, 'requiresInteract':False,
                     'events':[{'type':'playSequence','sequenceInstanceId':STAGE}]}
    if start_trigger['placementId'] in by_id:
        assert by_id[start_trigger['placementId']] == start_trigger
    else:
        world['placements'].append(start_trigger)
    if world != before_world:
        world['revision'] += 1
    old_sequence = copy.deepcopy(sequence)
    for suffix in ('mokomoko','cannon'):
        inst, = [i for i in sequence['instances'] if i['instanceId'] == STAGE.rsplit('.',1)[0]+'.'+suffix]
        assert inst['motionEnd'] in ('STOP','HOLD')
        inst['motionEnd'] = 'HOLD'
    if sequence != old_sequence:
        sequence['revision'] += 1
    placements = {p[0]:p for p in rows(placement_path)}
    catalog = {p[0]:p for p in rows(catalog_path)}
    stage, = [i for i in sequence['instances'] if i['instanceId'] == STAGE]
    ids = [b['targetId'] for b in stage['bindings']]
    # Exact scene export identities, verified against installed mesh bounds.
    ids += ['11877804709865735844','11550822667285194357','10829680542090774286']
    assert len(ids)==21 and len(set(ids))==21
    width=88; size=.25; ox=64.; oz=-995.
    xx,zz=np.meshgrid(ox+(np.arange(width)+.5)*size,oz+(np.arange(width)+.5)*size)
    heights=np.full(xx.shape,20.48); count=0
    for pid in ids:
        placement=placements[pid]
        model=ROOT/'Client/Bin/Resources'/catalog[placement[4]][2]
        expected[model]=model.read_bytes()
        for tri in mesh_triangles(model):
            a,b,c=[np.array(transform(v,placement)) for v in tri]
            normal=np.cross(b-a,c-a)
            if abs(normal[1])<1e-8 or abs(normal[1])/np.linalg.norm(normal)<math.cos(math.radians(50)):
                continue
            den=(b[2]-c[2])*(a[0]-c[0])+(c[0]-b[0])*(a[2]-c[2])
            u=((b[2]-c[2])*(xx-c[0])+(c[0]-b[0])*(zz-c[2]))/den
            v=((c[2]-a[2])*(xx-c[0])+(a[0]-c[0])*(zz-c[2]))/den
            y=u*a[1]+v*b[1]+(1-u-v)*c[1]
            mask=(u>=-1e-6)&(v>=-1e-6)&(u+v<=1.000001)&(y>heights)&(y<22.8)
            heights[mask]=y[mask]; count+=1
    assert np.max(heights)>22.39
    for n in range(1,4):
        x,y,z=by_id[f'jump{n}_1']['position']
        support=float(heights[int((z-oz)/size),int((x-ox)/size)])
        assert 22.3<support<22.5, (n,support)
        assert all(abs(by_id[f'jump{n}_1']['position'][j]-start_trigger['position'][j])<=start_trigger['halfExtents'][j] for j in (0,2))
    region='WaterpangEntry'
    lines=[f'LOSTARK_NAVGRID_SOURCE 1 "{AREA}.{region}" {width} {width} {size} {ox} {oz} {width*width}']
    lines += [f'{x} {z} 1 {heights[z,x]:.8f}' for z in range(width) for x in range(width)]
    nav_path=ROOT/f'Data/Navigation/{AREA}.{region}.navsource'
    manifest=ROOT/f'Data/Navigation/{AREA}.navregions'
    manifest_bytes=f'LOSTARK_NAVGRID_REGIONS 1 "{AREA}" 1\nREGION "{region}" 0.6\n'.encode()
    assert not manifest.exists() or manifest.read_bytes()==manifest_bytes, 'Existing regions must be merged explicitly'
    staged={world_path:(expected[world_path],encode(world)),sequence_path:(expected[sequence_path],encode(sequence)),
            nav_path:(nav_path.read_bytes() if nav_path.exists() else None, ('\n'.join(lines)+'\n').encode()),
            manifest:(manifest.read_bytes() if manifest.exists() else None,manifest_bytes)}
    # Every unrequested actor/marker stays byte-value identical.
    after={p['placementId']:p for p in world['placements']}
    for row in before_world['placements']:
        if row['placementId'] not in ('jump1','jump2','jump3'):
            assert after[row['placementId']]==row
    report={'placementCount':len(world['placements']),'jumpPairs':3,'holdActors':2,
            'navigationMeshPlacementIds':ids,'navigationRaisedCells':int(np.sum(heights>20.49)),
            'worldRevision':world['revision'],'sequenceRevision':sequence['revision']}
    return staged,expected,report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply',action='store_true')
    args=parser.parse_args()
    staged,expected,report=prepare()
    for path,(before,after) in staged.items():
        candidate=OUT/'candidate'/path.relative_to(ROOT)
        candidate.parent.mkdir(parents=True,exist_ok=True); candidate.write_bytes(after)
        backup=OUT/'before'/path.relative_to(ROOT)
        if before is not None and not backup.exists():
            backup.parent.mkdir(parents=True,exist_ok=True); backup.write_bytes(before)
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'report.json').write_bytes(encode(report))
    if args.apply: commit_staged_files(staged,expected=expected)
    print(json.dumps(dict(applied=args.apply,**report),ensure_ascii=False))
```

## Tools/MapPipeline/test_maharaka_waterpang_entry.py

```python
"""Published Waterpang entry data, preservation and exact mesh bake guards."""
import json
from pathlib import Path
import struct
import unittest
from configure_maharaka_waterpang_entry import ROOT, AREA, STAGE, OUT, prepare


class EntryContract(unittest.TestCase):
    def test_installer_is_idempotent_and_preserves_unrelated_rows(self):
        staged,_,report=prepare()
        self.assertTrue(all(before==after for before,after in staged.values()))
        current=json.loads((ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        before=json.loads((OUT/f'before/Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        after={p['placementId']:p for p in current['placements']}
        for row in before['placements']:
            if row['placementId'] not in ('jump1','jump2','jump3'):
                self.assertEqual(row,after[row['placementId']])
            else:
                self.assertEqual(row['position'],after[row['placementId']]['position'])
        self.assertEqual(3,report['jumpPairs'])

    def test_published_holds_and_unmodified_source_camera(self):
        author=ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json'
        runtime=ROOT/f'Client/Bin/DataFiles/Map/{AREA}.worldsequences.json'
        self.assertEqual(json.loads(author.read_text()),json.loads(runtime.read_text()))
        seq=json.loads(author.read_text())
        for suffix in ('mokomoko','cannon'):
            instance=next(i for i in seq['instances'] if i['instanceId']==STAGE.rsplit('.',1)[0]+'.'+suffix)
            self.assertEqual('HOLD',instance['motionEnd'])
            template=next(t for t in seq['templates'] if t['sequenceId']==instance['templateId'])
            self.assertTrue(template['tracks'][0]['keys'][-1]['visible'])
        path=f'{AREA}.camerashots.json'
        self.assertEqual(json.loads((ROOT/f'Data/Maps/Authoring/{AREA}'/path).read_text()),
                         json.loads((ROOT/'Client/Bin/DataFiles/Map'/path).read_text()))

    def test_client_server_navigation_and_landings(self):
        name=f'{AREA}.WaterpangEntry.navgrid'
        data=(ROOT/'Server/Bin/DataFiles/Navigation'/name).read_bytes()
        self.assertEqual(data,(ROOT/'Client/Bin/DataFiles/Navigation'/name).read_bytes())
        w,h,size,ox,oz=struct.unpack_from('<IIfff',data)
        world=json.loads((ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        rows={p['placementId']:p for p in world['placements']}
        for n in range(1,4):
            start=rows[f'jump{n}']; end=rows[f'jump{n}_1']
            self.assertTrue(start['enabled'] and start['requiresInteract'])
            self.assertFalse(end['enabled'])
            self.assertEqual(end['position'],start['events'][0]['targetPosition'])
            x,_,z=end['position']; index=int((z-oz)/size)*w+int((x-ox)/size)
            ground=struct.unpack_from('<f',data,20+w*h+4*index)[0]
            self.assertGreater(ground,22.3); self.assertLess(ground,22.5)


if __name__=='__main__': unittest.main(verbosity=2)
```
