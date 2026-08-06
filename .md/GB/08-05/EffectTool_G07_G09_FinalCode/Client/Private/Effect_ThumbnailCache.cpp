#include "Effect_ThumbnailCache.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <filesystem>
#include <vector>

namespace
{
	constexpr uint32_t MAX_LOADS_PER_FRAME = 4u;
	constexpr size_t MAX_CACHE_ENTRIES = 192u;
}

Client::CEffectThumbnailCache::CEffectThumbnailCache(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffectThumbnailCache::~CEffectThumbnailCache() = default;

void Client::CEffectThumbnailCache::Begin_Frame(
	const uint64_t iFrameNumber)
{
	m_iFrameNumber = iFrameNumber;
	m_iLoadsThisFrame = 0u;
}

Client::CEffectThumbnailCache::RESULT
Client::CEffectThumbnailCache::Request(const std::string& strAssetId)
{
	auto Iterator = m_Entries.find(strAssetId);
	if (Iterator != m_Entries.end())
	{
		Iterator->second.iLastUsedFrame = m_iFrameNumber;
		return {
			Iterator->second.pTextureView.Get(),
			Iterator->second.strError.empty() ?
				nullptr : &Iterator->second.strError };
	}

	static const std::string BudgetStatus = "Thumbnail load budget reached.";
	if (m_iLoadsThisFrame >= MAX_LOADS_PER_FRAME)
		return { nullptr, &BudgetStatus };
	++m_iLoadsThisFrame;

	ENTRY Staged;
	Staged.iLastUsedFrame = m_iFrameNumber;
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path) ||
		FAILED(DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), Path.c_str(), nullptr, &Staged.pTextureView)))
	{
		Staged.strError = "DDS thumbnail load failed: " + strAssetId;
	}
	const auto Inserted = m_Entries.emplace(strAssetId, std::move(Staged));
	return {
		Inserted.first->second.pTextureView.Get(),
		Inserted.first->second.strError.empty() ?
			nullptr : &Inserted.first->second.strError };
}

void Client::CEffectThumbnailCache::Invalidate(
	const uint64_t iCatalogRevision)
{
	if (m_iCatalogRevision == iCatalogRevision)
		return;
	m_iCatalogRevision = iCatalogRevision;
	Clear();
}

void Client::CEffectThumbnailCache::Trim()
{
	if (m_Entries.size() <= MAX_CACHE_ENTRIES)
		return;
	std::vector<std::pair<std::string, uint64_t>> Ages;
	Ages.reserve(m_Entries.size());
	for (const auto& Pair : m_Entries)
		Ages.emplace_back(Pair.first, Pair.second.iLastUsedFrame);
	std::sort(Ages.begin(), Ages.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.second < Right.second;
		});
	const size_t iRemoveCount = m_Entries.size() - MAX_CACHE_ENTRIES;
	for (size_t iEntry = 0u; iEntry < iRemoveCount; ++iEntry)
		m_Entries.erase(Ages[iEntry].first);
}

void Client::CEffectThumbnailCache::Clear()
{
	m_Entries.clear();
}

