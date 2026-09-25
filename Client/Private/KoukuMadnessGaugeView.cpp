#include "KoukuMadnessGaugeView.h"

#include "Character.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "Transform.h"
#include "UILayoutRuntime.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace
{
	bool ReadText(const filesystem::path& path, string& text)
	{
		ifstream stream(path, ios::binary);
		if (!stream.is_open()) return false;
		text.assign(istreambuf_iterator<char>(stream), istreambuf_iterator<char>());
		return !stream.bad();
	}

	bool ReadPosition(const Client::DATA_JSON_VALUE& root, float2_t& offset, f32_t& head)
	{
		const auto* madness = root.Find("madness");
		if (!madness || !madness->Is_Object()) return false;
		const auto read = [madness](const char* key, const f32_t fallback, f32_t& out)
		{
			const auto* value = madness->Find(key);
			if (!value) { out = fallback; return true; }
			if (!value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
			out = static_cast<f32_t>(value->Get_Number()); return std::isfinite(out);
		};
		return read("headOffsetMeters", 2.2f, head) && read("screenOffsetX", 0.f, offset.x) &&
			read("screenOffsetY", 0.f, offset.y) && std::abs(head) <= 10.f &&
			std::abs(offset.x) <= 1280.f && std::abs(offset.y) <= 1280.f;
	}

	void WriteJson(std::ostream& out, const Client::DATA_JSON_VALUE& value, const size_t depth = 0u)
	{
		using Client::DATA_JSON_TYPE;
		switch (value.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
		case DATA_JSON_TYPE::BOOLEAN: out << (value.Get_Boolean() ? "true" : "false"); break;
		case DATA_JSON_TYPE::NUMBER: out << std::setprecision(17) << value.Get_Number(); break;
		case DATA_JSON_TYPE::STRING: out << '"' << Client::CDataJson::Escape(value.Get_String()) << '"'; break;
		case DATA_JSON_TYPE::ARRAY:
			out << '[';
			for (size_t i = 0u; i < value.Get_Array().size(); ++i)
			{ if (i) out << ", "; WriteJson(out, value.Get_Array()[i], depth + 1u); }
			out << ']'; break;
		case DATA_JSON_TYPE::OBJECT:
			out << '{';
			{ bool first = true;
			for (const auto& [key, item] : value.Get_Object())
			{
				out << (first ? "\n" : ",\n") << string((depth + 1u) * 2u, ' ') << '"'
					<< Client::CDataJson::Escape(key) << "\": ";
				WriteJson(out, item, depth + 1u); first = false;
			} }
			out << '\n' << string(depth * 2u, ' ') << '}'; break;
		}
	}

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	constexpr f32_t FLASH_DECAY_PER_SECOND = 2.5f;
	const char* const ANCHOR_SLOT = "Madness_Anchor";
	const char* const FILL_SLOT = "Madness_BarFill";
	const char* const FLASH_SLOT = "Madness_BarFlash";
	const char* const STATE_SLOTS[] = { "Madness_State_0", "Madness_State_1", "Madness_State_2" };
	constexpr int32_t STATE_SLOT_COUNT = static_cast<int32_t>(sizeof(STATE_SLOTS) / sizeof(STATE_SLOTS[0]));
	/* Every slot the layout document authors; all of them move with the anchor. */
	const char* const MOVING_SLOTS[] =
	{
		"Madness_BarBg", "Madness_BarBase", FILL_SLOT, FLASH_SLOT,
		"Madness_State_0", "Madness_State_1", "Madness_State_2", "Madness_BarTick",
	};
}

Client::CKoukuMadnessGaugeView::CKoukuMadnessGaugeView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iGameObjectLevelIndex,
		TEXT("Layer_UI"), L"UI/KoukuSaydon/MadnessGauge_Layout.json");
	m_pView->Set_AllSlotsVisible(false);
	Capture_SlotOffsets();
	if (FAILED(Load_Config()))
	{
		OutputDebugStringA(
			"[KoukuMadnessGauge] KoukuHudModes.json madness block missing or invalid -- gauge stays hidden.\n");
	}
}

Client::CKoukuMadnessGaugeView::~CKoukuMadnessGaugeView()
{
}

void Client::CKoukuMadnessGaugeView::Capture_SlotOffsets()
{
	f32_t fW = 0.f, fH = 0.f;
	if (!m_pView->Get_SlotRect(ANCHOR_SLOT, m_fAnchorX, m_fAnchorY, fW, fH))
		return;
	for (const char* pSlotId : MOVING_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			continue;
		SLOT_OFFSET Offset{};
		Offset.strId = pSlotId;
		Offset.fDx = fX - m_fAnchorX;
		Offset.fDy = fY - m_fAnchorY;
		m_SlotOffsets.push_back(std::move(Offset));
	}
}

HRESULT Client::CKoukuMadnessGaugeView::Load_Config()
{
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
		return E_FAIL;
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
		return E_FAIL;
	const DATA_JSON_VALUE* pMadness = Root.Find("madness");
	if (nullptr == pMadness || !pMadness->Is_Object())
		return E_FAIL;

	vector<uint32_t> Thresholds;
	if (const DATA_JSON_VALUE* pThresholds = pMadness->Find("stateThresholds");
		nullptr != pThresholds && pThresholds->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pThresholds->Get_Array())
		{
			if (!Value.Is_Number() || Value.Get_Number() < 0.0)
				return E_FAIL;
			Thresholds.push_back(static_cast<uint32_t>(Value.Get_Number()));
		}
	}
	if (Thresholds.empty() || !std::is_sorted(Thresholds.begin(), Thresholds.end()))
		return E_FAIL;

	vector<string> FillTextures;
	if (const DATA_JSON_VALUE* pFills = pMadness->Find("fillTextureByState");
		nullptr != pFills && pFills->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pFills->Get_Array())
		{
			if (!Value.Is_String() || Value.Get_String().empty())
				return E_FAIL;
			FillTextures.push_back(Value.Get_String());
		}
	}

	f32_t fHeadOffset = 2.2f;
	float2_t screenOffset{};
	if (!ReadPosition(Root, screenOffset, fHeadOffset)) return E_FAIL;

	/* Commit only after every field validated so a bad file leaves the previous
	(empty, hidden) configuration untouched. */
	m_Thresholds = std::move(Thresholds);
	m_FillTextures = std::move(FillTextures);
	m_fHeadOffsetMeters = m_fSavedHeadOffsetMeters = fHeadOffset;
	m_vScreenOffset = m_vSavedScreenOffset = screenOffset;
	m_bConfigLoaded = true;
	return S_OK;
}

int32_t Client::CKoukuMadnessGaugeView::Resolve_State(const uint32_t iGauge) const
{
	int32_t iState = 0;
	for (const uint32_t iThreshold : m_Thresholds)
	{
		if (iGauge >= iThreshold)
			++iState;
	}
	return (std::min)(iState, STATE_SLOT_COUNT - 1);
}

void Client::CKoukuMadnessGaugeView::Hide()
{
	if (!m_bVisible)
		return;
	m_pView->Set_AllSlotsVisible(false);
	m_bVisible = false;
	m_iLastState = -1;
	m_fFlashAlpha = 0.f;
}

void Client::CKoukuMadnessGaugeView::Update(
	const f32_t fTimeDelta,
	const shared_ptr<CCharacter>& pLocalCharacter,
	const HUD_KOUKU_GIMMICK_STATE& State)
{
	if (!m_bConfigLoaded || !State.isValid || 0u == State.iMadnessMaximum ||
		HUD_KOUKU_HUD_MODE::MAZE == State.eHudMode ||
		LostArk::Shared::CARD_MAZE_ROLE::NONE != State.eCardMazeRole ||
		nullptr == pLocalCharacter || m_SlotOffsets.empty())
	{
		Hide();
		return;
	}

	const shared_ptr<CTransform> pTransform = pLocalCharacter->Get_Transform();
	CGameInstance& GameInstance = CGameInstance::Get();
	const float4x4_t* const pView = GameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProj = GameInstance.Get_Transform(D3DTS::PROJ);
	if (nullptr == pTransform || nullptr == pView || nullptr == pProj)
	{
		Hide();
		return;
	}

	float3_t vHead{};
	XMStoreFloat3(&vHead, pTransform->Get_State(STATE::POSITION));
	vHead.y += m_fHeadOffsetMeters;

	const float2_t vViewport = GameInstance.Get_ViewportSize();
	float2_t vScreen{};
	if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
		vHead, *pView, *pProj, vViewport, vScreen) ||
		vViewport.x <= 0.f || vViewport.y <= 0.f)
	{
		Hide();
		return;
	}

	/* CUILayoutRuntime positions are reference-resolution units; the projection is
	viewport pixels. */
	const f32_t fAnchorX = vScreen.x * (REF_WIDTH / vViewport.x) + m_vScreenOffset.x;
	const f32_t fAnchorY = vScreen.y * (REF_HEIGHT / vViewport.y) + m_vScreenOffset.y;
	for (const SLOT_OFFSET& Offset : m_SlotOffsets)
		m_pView->Set_SlotPosition(Offset.strId, fAnchorX + Offset.fDx, fAnchorY + Offset.fDy);

	const uint32_t iGauge = (std::min)(State.iMadnessGauge, State.iMadnessMaximum);
	/* Authored thresholds are percentages; Server madness uses 10000 units.
	Keep the fill ratio precise and normalize only the discrete state lookup. */
	const uint32_t iGaugePercent = static_cast<uint32_t>(
		static_cast<std::uint64_t>(iGauge) * 100u / State.iMadnessMaximum);
	const int32_t iState = Resolve_State(iGaugePercent);
	if (m_bVisible && m_iLastState >= 0 && iState > m_iLastState)
		m_fFlashAlpha = 1.f;
	m_iLastState = iState;

	m_pView->Set_AllSlotsVisible(true);
	m_pView->Set_SlotVisible(ANCHOR_SLOT, false);
	for (int32_t i = 0; i < STATE_SLOT_COUNT; ++i)
		m_pView->Set_SlotVisible(STATE_SLOTS[i], i == iState);

	/* A state past the texture list keeps the last strip (the full red bar at 100). */
	const f32_t fRatio = static_cast<f32_t>(iGauge) / static_cast<f32_t>(State.iMadnessMaximum);
	if (!m_FillTextures.empty())
	{
		const size_t iFillIndex = (std::min)(static_cast<size_t>(iState), m_FillTextures.size() - 1u);
		m_pView->Set_SlotTexture(FILL_SLOT, m_FillTextures[iFillIndex]);
		m_pView->Set_SlotFillRatio(FILL_SLOT, fRatio);
		m_pView->Set_SlotVisible(FILL_SLOT, fRatio > 0.f);
	}
	else
	{
		m_pView->Set_SlotVisible(FILL_SLOT, false);
	}

	m_fFlashAlpha = (std::max)(0.f, m_fFlashAlpha - fTimeDelta * FLASH_DECAY_PER_SECOND);
	m_pView->Set_SlotAlpha(FLASH_SLOT, m_fFlashAlpha);
	m_pView->Set_SlotVisible(FLASH_SLOT, m_fFlashAlpha > 0.f);

	m_bVisible = true;
	m_pView->Update(fTimeDelta);
}

void Client::CKoukuMadnessGaugeView::Get_Position(float2_t& screenOffset, f32_t& headOffsetMeters) const
{
	screenOffset = m_vScreenOffset;
	headOffsetMeters = m_fHeadOffsetMeters;
}

bool_t Client::CKoukuMadnessGaugeView::Set_Position(const float2_t& offset, const f32_t head)
{
	if (!std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(head) ||
		std::abs(offset.x) > 1280.f || std::abs(offset.y) > 1280.f || std::abs(head) > 10.f) return false;
	m_vScreenOffset = offset; m_fHeadOffsetMeters = head;
	return true;
}

bool_t Client::CKoukuMadnessGaugeView::Reload_Position(string& status)
{
	if (FAILED(Load_Config())) { status = "Invalid HUD configuration; current preview preserved."; return false; }
	status = "Reloaded saved madness position.";
	return true;
}

bool_t Client::CKoukuMadnessGaugeView::Save_Position(string& status)
{
	const filesystem::path path = CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json");
	const filesystem::path lockPath = path.wstring() + L".madness-position.lock";
	const HANDLE lock = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0u, nullptr,
		OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	if (lock == INVALID_HANDLE_VALUE) { status = "HUD position save is already in progress."; return false; }
	struct LOCK_GUARD { HANDLE handle; ~LOCK_GUARD() { CloseHandle(handle); } } guard{ lock };
	string before, error;
	DATA_JSON_VALUE root;
	float2_t savedOffset{}; f32_t savedHead = 0.f;
	if (!ReadText(path, before) || !CDataJson::Parse(before, root, error) ||
		!ReadPosition(root, savedOffset, savedHead))
	{ status = "HUD configuration could not be read; disk and preview preserved. " + error; return false; }
	const bool changedX = m_vScreenOffset.x != m_vSavedScreenOffset.x;
	const bool changedY = m_vScreenOffset.y != m_vSavedScreenOffset.y;
	const bool changedHead = m_fHeadOffsetMeters != m_fSavedHeadOffsetMeters;
	if ((changedX && savedOffset.x != m_vSavedScreenOffset.x) ||
		(changedY && savedOffset.y != m_vSavedScreenOffset.y) ||
		(changedHead && savedHead != m_fSavedHeadOffsetMeters))
	{ status = "Edited position field changed on disk. Reload saved position before saving; current preview preserved."; return false; }
	if (!changedX && !changedY && !changedHead)
	{
		m_vScreenOffset = m_vSavedScreenOffset = savedOffset;
		m_fHeadOffsetMeters = m_fSavedHeadOffsetMeters = savedHead;
		status = "No position edits; refreshed position from disk.";
		return true;
	}
	// Merge only edited fields into the latest document, including independent axis edits.
	auto fields = root.Get_Object();
	auto madness = fields.at("madness").Get_Object();
	if (changedX) madness["screenOffsetX"] = DATA_JSON_VALUE::Number(m_vScreenOffset.x);
	if (changedY) madness["screenOffsetY"] = DATA_JSON_VALUE::Number(m_vScreenOffset.y);
	if (changedHead) madness["headOffsetMeters"] = DATA_JSON_VALUE::Number(m_fHeadOffsetMeters);
	fields["madness"] = DATA_JSON_VALUE::Object(std::move(madness));
	std::ostringstream serialized;
	serialized.imbue(std::locale::classic());
	WriteJson(serialized, DATA_JSON_VALUE::Object(std::move(fields)));
	const string after = serialized.str() + "\n";
	DATA_JSON_VALUE verified;
	if (!CDataJson::Parse(after, verified, error) || !ReadPosition(verified, savedOffset, savedHead))
	{ status = "Position serialization rejected; disk and preview preserved."; return false; }
	const auto suffix = std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
	const filesystem::path temporary = path.wstring() + L".tmp." + suffix;
	const filesystem::path backup = path.wstring() + L".backup." + suffix;
	{
		ofstream stream(temporary, ios::binary | ios::trunc);
		stream.write(after.data(), static_cast<std::streamsize>(after.size())); stream.flush();
		if (!stream.good()) { status = "Position temporary write failed; source preserved."; return false; }
	}
	string current;
	if (!ReadText(temporary, current) || current != after || !ReadText(path, current) || current != before)
	{ DeleteFileW(temporary.c_str()); status = "HUD source changed during save; disk and preview preserved."; return false; }
	if (!ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), 0u, nullptr, nullptr))
	{ DeleteFileW(temporary.c_str()); status = "Position atomic replace failed; backup preserved at " + backup.string(); return false; }
	string displaced;
	if (!ReadText(backup, displaced) || displaced != before)
	{
		if (ReadText(path, current) && current == after)
		{
			const filesystem::path recovery = path.wstring() + L".conflict." + suffix;
			(void)ReplaceFileW(path.c_str(), backup.c_str(), recovery.c_str(), 0u, nullptr, nullptr);
		}
		status = "Concurrent HUD save detected; recovery files preserved beside the source. Reload before retrying.";
		return false;
	}
	m_vScreenOffset = m_vSavedScreenOffset = savedOffset;
	m_fHeadOffsetMeters = m_fSavedHeadOffsetMeters = savedHead;
	status = "Saved madness position. Backup: " + backup.string();
	return true;
}
